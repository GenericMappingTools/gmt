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
/*void *V_API, int mode
 * API functions to support the gmt2kml application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: gmt2kml reads GMT tables and plots symbols, lines,
 * and polygons as KML files for Google Earth.
 */

#include "gmt_dev.h"
#include <stdarg.h>

#define THIS_MODULE_CLASSIC_NAME	"gmt2kml"
#define THIS_MODULE_MODERN_NAME	"gmt2kml"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Convert GMT data table to Google Earth KML file"
#define THIS_MODULE_KEYS	"<D{,>D},CC("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>KOVabdefghiq" GMT_OPT("HMm")

EXTERN_MSC int gmt_parse_R_option (struct GMT_CTRL *GMT, char *item);
EXTERN_MSC void gmt_get_rgb_lookup (struct GMT_CTRL *GMT, struct GMT_PALETTE *P, int index, double value, double *rgb);

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
	struct E {	/* -E[+e][+s] */
		bool active;
		bool extrude;
		bool tessellate;
	} E;
	struct F {	/* -F */
		bool active;
		unsigned int mode;
		unsigned int geometry;
	} F;
	struct G {	/* -G<fill>+f|n */
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
		unsigned int col;
		unsigned int mode;
		char *fmt;
	} N;
	struct Q {	/* -Qi|a<az> and -Qs<scale> */
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
	C->E.tessellate = true;	/* This is the default, use -E+s to turn that off (turned off for symbols later) */
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

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] [-Aa|g|s[<altitude>|x<scale>]] [-C<cpt>] [-D<descriptfile>] [-E[+e][+s]]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Fe|s|t|l|p|w] [-G[<color>][+f|n]] [-I<icon>] [-K] [-L<name1>,<name2>,...]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-N<col>|t|<template>|<name>] [-O] [-Q[a|i]<az>] [-Qs<scale>] [-Re|<w>/<e>/<s>/n>] [-Sc|n<scale>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T<title>[/<foldername>] [%s] [-W[<pen>][<attr>]] [-Z<opts>] [%s]\n", GMT_V_OPT, GMT_a_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_qi_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Altitude mode, choose among three modes:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     a Absolute altitude.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     g Altitude relative to sea surface or ground.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     s Altitude relative to seafloor or ground.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append fixed <altitude>, or x<scale> [g0: Clamped to sea surface or ground].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Append color palette name to color symbols by third column z-value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or via -Z<value> lookup for lines and polygons.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D File with HTML snippets to use for data description [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Control parameters of lines and polygons:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +e to extend feature down to the ground [no extrusion].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +s to connect points with straight lines [tessellate onto surface].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Feature type; choose from (e)vent, (s)ymbol, (t)imespan, (l)ine, (p)olygon, or (w)iggle [s].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   All features expect lon, lat in the first two columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Value or altitude is given in the third column (see -A and -C).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Event requires a timestamp in the next column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Timespan requires begin and end ISO timestamps in the next two columns\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (use NaN for unlimited begin and/or end times).\n");
	gmt_rgb_syntax (API->GMT, 'G', "Set color for symbol/polygon fill (-G<color>[+f]) or label font (-G<color>+n).");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default polygon fill is lightorange with 75%% transparency; use -G+f for no fill.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default text label font color is white; use -G+n to turn off labels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I URL to an alternative icon used for the symbol [Google circle].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If URL starts with + we will prepend http://maps.google.com/mapfiles/kml/.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give -I- to not place any icons.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is a local icon with no directory path].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-K Allow for more KML code to be appended later [OFF].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Supply extended named data columns via <name1>,<name2>,... [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Control the feature labels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   By default, -L\"label\" statements in the segment header are used. Alternatively,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     1. Specify -N<col> to use a column from the data record a single-word label (-Fe|s|t only).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     2. Specify -Nt if the trailing record text should be used as label (-Fe|s|t only).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     3. Append a string that may contain the format %%d for a running feature count.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     4. Give no argument to indicate no labels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-O Append the KML code to an existing document [OFF].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Set properties in support of wiggle plots (-Fw):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Qa Set preferred azimuth +|-90 for wiggle direction [0], or\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Qi Instead, set fixed azimuth for wiggle direction [variable].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Qs Set wiggle scale in z-data units per map unit.  Append %s [e].\n", GMT_LEN_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t-R Issue Region tag.  Append w/e/s/n to set a particular region or give -Re to use the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   exact domain of the data (single file only) [no region specified].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Scale for (c)ircle icon size or (n)ame label [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Append KML document title name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally append /<foldername> to name folder when used with\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -O and -K to organize features into groups.\n");
	GMT_Option (API, "V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Specify pen attributes for lines and polygon outlines [Default is %s].\n", 8);
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
	GMT_Option (API, "a,bi2,di,e,f,g,h,i,qi,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int parse_old_W (struct GMTAPI_CTRL *API, struct GMT2KML_CTRL *Ctrl, char *text) {
	unsigned int j = 0, n_errors = 0;
	if (text[j] == '-') {Ctrl->W.pen.cptmode = 1; j++;}
	if (text[j] == '+') {Ctrl->W.pen.cptmode = 3; j++;}
	if (text[j] && gmt_getpen (API->GMT, &text[j], &Ctrl->W.pen)) {
		gmt_pen_syntax (API->GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", 8);
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

	unsigned int n_errors = 0, pos = 0, k, n_files = 0, ind;
	size_t n_alloc = 0;
	char buffer[GMT_LEN256] = {""}, p[GMT_LEN256] = {""}, T[4][GMT_LEN64], *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET))
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
						GMT_Report (API, GMT_MSG_ERROR, "Bad altitude mode. Use a, g or s.\n");
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
			case 'E':	/* Extrude feature down to the ground */
			 	Ctrl->E.active = true;
				if (strstr (opt->arg, "+s"))	/* Straight lines, turn off tessellation */
				 	Ctrl->E.tessellate = false;
				if (strstr (opt->arg, "+e"))	/* Straight lines, turn off tessellation */
				 	Ctrl->E.extrude = true;
				else if (opt->arg[0] == '\0')	/* Old syntax -E means -E+e */
				 	Ctrl->E.extrude = true;
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
						GMT_Report (API, GMT_MSG_ERROR, "Bad feature type. Use s, e, t, l, p or w.\n");
						n_errors++;
						break;
				}
				break;
			case 'G':	/* Set fill for symbols or polygon */
				ind = F_ID;	/* Default is fill if not overridden below */
				k = 0;
				if ((c = strstr (opt->arg, "+f"))) 	/* Polygon fill */
					ind = F_ID, k = 0, c[0] = '\0';
				else if ((c = strstr (opt->arg, "+n"))) {	/* Label color */
					ind = N_ID, k = 0, c[0] = '\0';
					if (opt->arg[0] == '\0')
						Ctrl->N.mode = NO_LABEL;	/* Another way of turning off labels */
				}
				else if (gmt_M_compat_check (GMT, 5)) {	/* Old style -Gf or -Gn OK */
					GMT_Report (API, GMT_MSG_COMPAT, "-Gf or -Gn is deprecated, use -G[<fill>][+f|n] instead\n");
					if (opt->arg[0] == 'f')	/* Polygon fill */
						ind = F_ID, k = 1;
					else if (opt->arg[0] == 'n')	/* Label color */
						ind = N_ID, k = 1;
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Old syntax is -Gf or -Gn (use -G[<fill>][+f|n] instead).\n");
						n_errors++;
					}
				}
				if (opt->arg[0] == '\0')	/* Transparency selected */
		 			Ctrl->G.active[ind] = true;
				else if (opt->arg[1] == '-' && gmt_M_compat_check (GMT, 5)) {
		 			Ctrl->G.active[ind] = true;
					GMT_Report (API, GMT_MSG_COMPAT, "Using - for no fill is deprecated, use -G[+f|n] instead\n");
				}
				else if (gmt_getfill (GMT, &opt->arg[k], &Ctrl->G.fill[ind])) {
					gmt_fill_syntax (GMT, 'G', NULL, "(-G[<fill>]+f|n)");
					n_errors++;
				}
				if (c) c[0] = '+';	/* Restore */
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
				if (opt->arg[0] == '-') {	/* First non-coordinate field as label (Backwards compatible) */
					Ctrl->N.mode = GET_COL_LABEL;
					Ctrl->N.col = GMT_Z;
				}
				else if ((opt->arg[0] == 't' || opt->arg[0] == '+') && opt->arg[1] == '\0')	/* Trailing text (+ is backwards compatible)  */
					Ctrl->N.mode = GET_LABEL;
				else if (strchr (opt->arg, '%')) {	/* Want a format */
					Ctrl->N.fmt = strdup (opt->arg);
					Ctrl->N.mode = FMT_LABEL;
				}
				else if (isdigit (opt->arg[0])) {	/* Assume it is a numerical column */
					Ctrl->N.mode = GET_COL_LABEL;
					Ctrl->N.col = atoi (opt->arg);
				}
				else if (!opt->arg[0]) {	/* Want no label */
					Ctrl->t_transp = 0.0;
					Ctrl->N.mode = NO_LABEL;
				}
				break;
			case 'Q':	/* Wiggle azimuth and scale settings in data units for map distance  */
				Ctrl->Q.active = true;
				switch (opt->arg[0]) {
					case 'i': Ctrl->Q.mode = 1; Ctrl->Q.value[1] = atof (&opt->arg[1]); break;
					case 'q': Ctrl->Q.mode = 0; Ctrl->Q.value[0] = atof (&opt->arg[1]); break;
					case 's': strncpy (p, &opt->arg[1],GMT_LEN256-1);
						k = (unsigned int)strlen (p); if (k > 0) k--; /* was k = (unsigned int)strlen(p) - 1, but Coverity screamed */
						if (!strchr (GMT_LEN_UNITS, p[k])) strcat (p, "e");	/* Force meters as default unit */
						Ctrl->Q.dmode = gmt_get_distance (GMT, p, &(Ctrl->Q.scale), &(Ctrl->Q.unit)); break;
				}
				break;
			case 'R':	/* Region setting */
				Ctrl->R2.active = GMT->common.R.active[RSET] = true;
				if (opt->arg[0] == 'e' || opt->arg[0] == 'a')	/* Get args from data domain (used to be -Ra but in modern mdoe -Re is what is meant) */
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
					GMT_Report (API, GMT_MSG_ERROR, "-S requires c or n, then nondimensional scale\n");
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
				if (opt->arg[0] == '-' || (opt->arg[0] == '+' && opt->arg[1] != 'c')) {	/* Definitively old-style args */
					if (gmt_M_compat_check (API->GMT, 5)) {	/* Sorry */
						GMT_Report (API, GMT_MSG_COMPAT, "Your -W syntax is obsolete; see program usage.\n");
						n_errors++;
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Your -W syntax is obsolete; see program usage.\n");
						n_errors += parse_old_W (API, Ctrl, opt->arg);
					}
				}
				else if (opt->arg[0]) {
					if (gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
						gmt_pen_syntax (GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", 11);
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
								GMT_Report (API, GMT_MSG_ERROR, "-Z+a requires 2 arguments\n");
								n_errors++;
							}
							Ctrl->Z.min[ALT] = atof (T[0]);	Ctrl->Z.max[ALT] = atof (T[1]);
							break;
						case 'l':	/* LOD */
							if (sscanf (&p[1], "%[^/]/%s", T[0], T[1]) != 2) {
								GMT_Report (API, GMT_MSG_ERROR, "-Z+l requires 2 arguments\n");
								n_errors++;
							}
							Ctrl->Z.min[LOD] = atof (T[0]);	Ctrl->Z.max[LOD] = atof (T[1]);
							break;
						case 'f':	/* Fading */
							if (sscanf (&p[1], "%[^/]/%s", T[0], T[1]) != 2) {
								GMT_Report (API, GMT_MSG_ERROR, "-Z+f requires 2 arguments\n");
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
							GMT_Report (API, GMT_MSG_ERROR, "-Z unrecognized modifier +%c\n", p[0]);
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

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && !Ctrl->C.file, "Option -C: Need to supply color palette name\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && access (Ctrl->D.file, R_OK), "Option -D: Cannot open HTML description file %s\n", Ctrl->D.file);
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_OUT], "Cannot produce binary KML output\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->R2.automatic && n_files > 1, "Option -Ra without arguments only accepted for single table\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.scale[F_ID] < 0.0 || Ctrl->S.scale[N_ID] < 0.0, "Option -S takes scales > 0.0\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->t_transp < 0.0 || Ctrl->t_transp > 1.0, "Option -Q takes transparencies in range 0-1\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.mode == GET_LABEL && Ctrl->F.mode >= LINE, "Option -Nt not valid for lines and polygons\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.mode == WIGGLE && Ctrl->Q.scale <= 0.0, "Option -Fw requires -Qs\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.pen.width < 1.0, "Option -W given pen width < 1 pixel.  Use integers and append p as unit.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.pen.cptmode && !Ctrl->C.active, "Option -W+|-<pen> requires the -C option.\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && Ctrl->N.mode == GET_LABEL, "Cannot use -Nt when -b is used.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int kml_print (struct GMTAPI_CTRL *API, struct GMT_RECORD *R, int ntabs, char *format, ...) {
	/* Message whose output depends on verbosity setting */
	int tab;
	va_list args;

	R->text[0] = '\0';
	for (tab = 0; tab < ntabs; tab++) R->text[tab] = '\t';
	va_start (args, format);
	/* append format to the record: */
	vsnprintf (R->text + ntabs, GMT_BUFSIZ - ntabs, format, args);
	va_end (args);
	assert (strlen (R->text) < GMT_BUFSIZ);
	GMT_Put_Record (API, GMT_WRITE_DATA, R);
	return (GMT_NOERROR);
}

GMT_LOCAL int check_lon_lat (struct GMT_CTRL *GMT, double *lon, double *lat) {
	if (*lat < GMT->common.R.wesn[YLO] || *lat > GMT->common.R.wesn[YHI]) return (true);
	if (*lon < GMT->common.R.wesn[XLO]) *lon += 360.0;
	if (*lon > GMT->common.R.wesn[XHI]) *lon -= 360.0;
	if (*lon < GMT->common.R.wesn[XLO]) return (true);
	return (false);
}

GMT_LOCAL void print_altmode (struct GMTAPI_CTRL *API, struct GMT_RECORD *Out, bool extrude, bool tessellate, int altmode, int ntabs) {
	char *RefLevel[5] = {"clampToGround", "relativeToGround", "absolute", "relativeToSeaFloor", "clampToSeaFloor"};
	if (extrude) kml_print (API, Out, ntabs, "<extrude>1</extrude>");
	if (tessellate) kml_print (API, Out, ntabs, "<tessellate>1</tessellate>");
	if (altmode == KML_GROUND_REL || altmode == KML_ABSOLUTE) kml_print (API, Out, ntabs, "<altitudeMode>%s</altitudeMode>", RefLevel[altmode]);
	if (altmode == KML_SEAFLOOR_REL || altmode == KML_SEAFLOOR) kml_print (API, Out, ntabs, "<gx:altitudeMode>%s</gx:altitudeMode>", RefLevel[altmode]);
}

GMT_LOCAL void ascii_output_three (struct GMTAPI_CTRL *API, struct GMT_RECORD *Out, double out[], int ntabs) {
	char X[GMT_LEN256] = {""}, Y[GMT_LEN256] = {""}, Z[GMT_LEN256] = {""};
	gmt_ascii_format_col (API->GMT, X, out[GMT_X], GMT_OUT, GMT_X);
	gmt_ascii_format_col (API->GMT, Y, out[GMT_Y], GMT_OUT, GMT_Y);
	gmt_ascii_format_col (API->GMT, Z, out[GMT_Z], GMT_OUT, GMT_Z);
	kml_print (API, Out, ntabs, "%s,%s,%s", X, Y, Z);
}

GMT_LOCAL void place_region_tag (struct GMTAPI_CTRL *API, struct GMT_RECORD *Out, double wesn[], double min[], double max[], int N) {
	char text[GMT_LEN256] = {""};
	if (gmt_M_360_range (wesn[XLO], wesn[XHI])) { wesn[XLO] = -180.0; wesn[XHI] = +180.0;}
	kml_print (API, Out, N++, "<Region>");
	kml_print (API, Out, N++, "<LatLonAltBox>");
	gmt_ascii_format_col (API->GMT, text, wesn[YHI], GMT_OUT, GMT_Y);
	kml_print (API, Out, N, "<north>%s</north>", text);
	gmt_ascii_format_col (API->GMT, text, wesn[YLO], GMT_OUT, GMT_Y);
	kml_print (API, Out, N, "<south>%s</south>", text);
	gmt_ascii_format_col (API->GMT, text, wesn[XHI], GMT_OUT, GMT_X);
	kml_print (API, Out, N, "<east>%s</east>", text);
	gmt_ascii_format_col (API->GMT, text, wesn[XLO], GMT_OUT, GMT_X);
	kml_print (API, Out, N, "<west>%s</west>", text);
	if (max[ALT] > min[ALT]) {
		kml_print (API, Out, N, "<minAltitude>%g</minAltitude>", min[ALT]);
		kml_print (API, Out, N, "<maxAltitude>%g</maxAltitude>", max[ALT]);
	}
	kml_print (API, Out, --N, "</LatLonAltBox>");
	if (max[LOD] != min[LOD]) {
		kml_print (API, Out, N++, "<Lod>");
		kml_print (API, Out, N, "<minLodPixels>%ld</minLodPixels>", lrint (min[LOD]));
		kml_print (API, Out, N, "<maxLodPixels>%ld</maxLodPixels>", lrint (max[LOD]));
		if (min[FADE] > 0.0 || max[FADE] > 0.0) {
			kml_print (API, Out, N, "<minFadeExtent>%g</minFadeExtent>", min[FADE]);
			kml_print (API, Out, N, "<maxFadeExtent>%g</maxFadeExtent>", max[FADE]);
		}
		kml_print (API, Out, --N, "</Lod>");
	}
	kml_print (API, Out, --N, "</Region>");
}

GMT_LOCAL void set_iconstyle (struct GMTAPI_CTRL *API, struct GMT_RECORD *Out, double *rgb, double scale, char *iconfile, int N) {
	/* No icon = no symbol */
	kml_print (API, Out, N++, "<IconStyle>");
	kml_print (API, Out, N++, "<Icon>");
	if (iconfile[0] != '-') kml_print (API, Out, N, "<href>%s</href>", iconfile);
	kml_print (API, Out, --N, "</Icon>");
	if (iconfile[0] != '-') {
		kml_print (API, Out, N, "<scale>%g</scale>", scale);
		kml_print (API, Out, N, "<color>%02x%02x%02x%02x</color>", gmt_M_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	}
	kml_print (API, Out, --N, "</IconStyle>");
}

GMT_LOCAL void set_linestyle (struct GMTAPI_CTRL *API, struct GMT_RECORD *Out, struct GMT_PEN *pen, double *rgb, int N) {
	kml_print (API, Out, N++, "<LineStyle>");
	kml_print (API, Out, N, "<color>%02x%02x%02x%02x</color>", gmt_M_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	kml_print (API, Out, N, "<width>%g</width>", pen->width);
	kml_print (API, Out, --N, "</LineStyle>");
}

GMT_LOCAL void set_polystyle (struct GMTAPI_CTRL *API, struct GMT_RECORD *Out, double *rgb, int outline, int active, int N) {
	kml_print (API, Out, N++, "<PolyStyle>");
	kml_print (API, Out, N, "<color>%02x%02x%02x%02x</color>", gmt_M_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	kml_print (API, Out, N, "<fill>%d</fill>", !active);
	kml_print (API, Out, N, "<outline>%d</outline>", outline);
	kml_print (API, Out, --N, "</PolyStyle>");
}

GMT_LOCAL void get_rgb_lookup (struct GMT_CTRL *GMT, struct GMT_PALETTE *P, int index, double *rgb) {
	/* Special version of gmt_get_rgb_lookup since no interpolation can take place */
	struct GMT_PALETTE_HIDDEN *PH = gmt_get_C_hidden (P);
	if (index < 0) {	/* NaN, Foreground, Background */
		gmt_M_rgb_copy (rgb, P->bfn[index+3].rgb);
		PH->skip = P->bfn[index+3].skip;
	}
	else if (P->data[index].skip) {		/* Set to page color for now */
		gmt_M_rgb_copy (rgb, GMT->current.setting.ps_page_rgb);
		PH->skip = true;
	}
	else {	/* Return low color */
		gmt_M_memcpy (rgb, P->data[index].rgb_low, 4, double);
		PH->skip = false;
	}
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

void KML_plot_object (struct GMTAPI_CTRL *API, struct GMT_RECORD *Out, double *x, double *y, uint64_t np, int type, int process_id, int alt_mode, int N, double z_level) {
	/* Wiggles: Plots a self-contained polygon or line, depending on type, using
	 * the current fill/line styles */
	static char *name[2] = {"Wiggle Anomaly", "Positive Anomaly"};
	static char *feature[5] = {"Point", "Point", "Point", "LineString", "Polygon"};
	double out[3];
	uint64_t k;
	kml_print (API, Out, N++, "<Placemark>");
	kml_print (API, Out, N, "<name>%s</name>", name[type-LINE]);
	kml_print (API, Out, N, "<styleUrl>#st-%d-%d</styleUrl>", process_id, 0); /* It is always style 0 */
	kml_print (API, Out, N++, "<%s>", feature[type]);
	print_altmode (API, Out, false, true, alt_mode, N);
	if (type == POLYGON) {
		kml_print (API, Out, N++, "<outerBoundaryIs>");
		kml_print (API, Out, N++, "<LinearRing>");
	}
	kml_print (API, Out, N++, "<coordinates>");
	out[GMT_Z] = z_level;
	for (k = 0; k < np; k++) {
		out[GMT_X] = x[k];
		out[GMT_Y] = y[k];
		ascii_output_three (API, Out, out, N);
	}
	kml_print (API, Out, --N, "</coordinates>");
	if (type == POLYGON) {
		kml_print (API, Out, --N, "</LinearRing>");
		kml_print (API, Out, --N, "</outerBoundaryIs>");
	}
	kml_print (API, Out, --N, "</%s>", feature[type]);
	kml_print (API, Out, --N, "</Placemark>");
}

GMT_LOCAL void kml_plot_wiggle (struct GMT_CTRL *GMT, struct GMT_RECORD *Out, struct KML *kml, double zscale, int mode, double azim[], int fill, int outline, int process_id, int amode, int N, double altitude) {
	int64_t i, np = 0;
	double lon_len, lat_len, az = 0.0, s = 0.0, c = 0.0, lon_inc, lat_inc;
	double start_az = 0, stop_az = 0, daz;
	/* Here, kml->{lon, lat, z} are the kml->n_in coordinates of one section of a to-be-designed wiggle.
	 * We need to erect z, properly scaled, normal to the line and build the kml->n_out fake kml->{flon, flat}
	 * points that we can use to plot lines and polygons on Google Earth */
	/* zscale is in degrees per data units. */
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
		np = kml->n_out;	/* Number of points for the outline only */
		if (fill) {
			for (i = kml->n_in - 2; i >= 0; i--, kml->n_out++) {	/* Go back to 1st point along track */
				kml->flon[kml->n_out] = kml->lon[i];
				kml->flat[kml->n_out] = kml->lat[i];
			}
		}
	}

	if (fill) /* First shade wiggles */
		KML_plot_object (GMT->parent, Out, kml->flon, kml->flat, kml->n_out, POLYGON, process_id, amode, N, altitude);

	if (outline) /* Then draw wiggle outline */
		KML_plot_object (GMT->parent, Out, kml->flon, kml->flat, np, LINE, process_id, amode, N, altitude);
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmt2kml (void *V_API, int mode, void *args) {
	bool first = true, get_z = false, use_folder = false, do_description, no_dateline = false, act;
	unsigned int n_coord = 0, t1_col, t2_col, pnt_nr = 0, tbl;

	uint64_t row, seg, n_tables, n_segments, n_rows;
	size_t L = 0;
	int set_nr = 0, index, N = 1, error = 0, process_id;

	char buffer[GMT_BUFSIZ] = {""}, description[GMT_BUFSIZ] = {""}, item[GMT_LEN128] = {""};
	char *feature[5] = {"Point", "Point", "Point", "LineString", "Polygon"}, *Document[2] = {"Document", "Folder"};
	char *name[5] = {"Point", "Event", "Timespan", "Line", "Polygon"};
	char text[GMT_LEN256] = {""}, record[GMT_BUFSIZ] = {""};
	char **file = NULL, *label = NULL, *header = NULL;

	double rgb[4], out[5], last_x = 0;

	struct KML *kml = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATATABLE_HIDDEN *TH = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMT2KML_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);		/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmt2kml main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");

	/* gmt2kml only applies to geographic data so we do a -fg implicitly here */
	gmt_set_geographic (GMT, GMT_IN);
	gmt_set_geographic (GMT, GMT_OUT);
	gmt_M_memset (out, 5, double);	/* Set to zero */

	n_coord = (Ctrl->F.mode < LINE) ? Ctrl->F.mode + 2 : 2;		/* This is a cryptic way to determine if there are 2,3 or 4 columns... */
	if (Ctrl->F.mode == WIGGLE) n_coord = 3;	/* But here we need exactly 3 */
	get_z = (Ctrl->C.active || Ctrl->A.get_alt);
	if (get_z && Ctrl->F.mode < LINE) n_coord++;
	if (Ctrl->F.mode < LINE) Ctrl->E.tessellate = false;	/* No tessellation for symbols */


	if (GMT_Init_IO (API, GMT_IS_DATASET, Ctrl->F.geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (D->n_columns < n_coord) {
		GMT_Report (API, GMT_MSG_ERROR, "Input data have %d column(s) but at least %d are needed\n", (int)D->n_columns, n_coord);
		Return (GMT_DIM_TOO_SMALL);
	}
 //       if (Ctrl->L.active && Ctrl->L.n_cols >= (D->n_columns-2)) {
 //       	GMT_Report (API, GMT_MSG_ERROR, "Input data have %d column(s) but at least %d are needed because of -L\n", (int)D->n_columns, n_coord+Ctrl->L.n_cols);
 //       	Return (GMT_DIM_TOO_SMALL);
 //       }

	n_tables = D->n_tables;

	if (Ctrl->N.mode == GET_LABEL && D->type != GMT_READ_MIXED) {
		GMT_Report (API, GMT_MSG_ERROR, "Data file has no trailing text but -Nt was selected\n");
		Return (GMT_DIM_TOO_SMALL);
	}
	if (Ctrl->N.mode == GET_COL_LABEL && Ctrl->N.col >= D->n_columns ) {
		GMT_Report (API, GMT_MSG_ERROR, "Data file has fewer columns than implied by -N<col>");
		Return (GMT_DIM_TOO_SMALL);
	}

	if (Ctrl->C.active) {	/* Process CPT */
		if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (P->is_continuous) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot use continuous color palette\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {
		Return (API->error);	/* Enables data output and sets access mode */
	}

	Out = gmt_new_record (GMT, NULL, record);	/* Since we only need to worry about text in this module */

	/* Now we are ready to take on some input values */

	out[GMT_Z] = Ctrl->A.altitude;
	strcpy (GMT->current.setting.io_col_separator, ",");		/* Specify comma-separated output */
	strcpy (GMT->current.setting.format_float_out, "%.12g");	/* Make sure we use enough decimals */
	t1_col = 2 + get_z;
	t2_col = 3 + get_z;
	if (Ctrl->F.mode == EVENT || Ctrl->F.mode == SPAN) gmt_set_column (GMT, GMT_IO, t1_col, GMT_IS_ABSTIME);
	if (Ctrl->F.mode == SPAN) gmt_set_column (GMT, GMT_IO, t2_col, GMT_IS_ABSTIME);

	if (Ctrl->F.mode == WIGGLE) {	/* Adjust wiggle scale for units and then take inverse */
		char unit_name[GMT_LEN16] = {""};
		gmt_check_scalingopt (GMT, 'Q', Ctrl->Q.unit, unit_name);
		GMT_Report (API, GMT_MSG_INFORMATION, "Wiggle scale given as %g z-data units per %s.\n", Ctrl->Q.scale, unit_name);
		gmt_init_distaz (GMT, Ctrl->Q.unit, Ctrl->Q.dmode, GMT_MAP_DIST);	/* Initialize distance machinery */
		Ctrl->Q.scale = 1.0 / Ctrl->Q.scale;	/* Now in map-distance units (i.e, unit they appended) per users data units */
		GMT_Report (API, GMT_MSG_INFORMATION, "Wiggle scale inverted as %g %s per z-data units.\n", Ctrl->Q.scale, unit_name);
		/* Convert to degrees per user data unit - this is our scale that converts z-data to degree distance latitude */
		Ctrl->Q.scale = R2D * (Ctrl->Q.scale / GMT->current.map.dist[GMT_MAP_DIST].scale) / GMT->current.proj.mean_radius;
		GMT_Report (API, GMT_MSG_INFORMATION, "Wiggle scale inverted to yield %g degrees per z-data unit\n", Ctrl->Q.scale);
	}
	if (!GMT->common.O.active) {
		/* Create KML header */
		kml_print (API, Out, 0, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
		kml_print (API, Out, 0, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">");
		kml_print (API, Out, N++, "<%s>", Document[KML_DOCUMENT]);
		if (Ctrl->T.title != NULL && *Ctrl->T.title != '\0')
			kml_print (API, Out, N, "<name>%s</name>", Ctrl->T.title);
		if (Ctrl->Z.invisible) kml_print (API, Out, N, "<visibility>0</visibility>");
		if (Ctrl->Z.open) kml_print (API, Out, N, "<open>1</open>");
	}

	/* Apparently, for Google Maps (not Google Earth), styles need to be outside any <Folder></Folder> pair, so we do them first */
	process_id = (int) getpid();
	kml_print (API, Out, N++, "<Style id=\"st-%d-0\">", process_id);	/* Default style unless -C is used, use PID to get unique style ID in case of layer-caking -O -K */

	/* Set icon style (applies to symbols only */
	set_iconstyle (API, Out, Ctrl->G.fill[F_ID].rgb, Ctrl->S.scale[F_ID], Ctrl->I.file, N);

	/* Set shared line and polygon style (also for extrusions) */
	if (Ctrl->F.mode == WIGGLE) {
		if (!Ctrl->G.active[F_ID]) {
			set_polystyle (API, Out, Ctrl->G.fill[F_ID].rgb, 0, 0, N);
		}
		if (Ctrl->W.active) {
			set_linestyle (API, Out, &Ctrl->W.pen, Ctrl->W.pen.rgb, N);
		}
	}
	else {
		set_linestyle (API, Out, &Ctrl->W.pen, Ctrl->W.pen.rgb, N);
		set_polystyle (API, Out, Ctrl->G.fill[F_ID].rgb, !Ctrl->W.pen.cptmode, Ctrl->G.active[F_ID], N);
	}

	/* Set style for labels */
	kml_print (API, Out, N++, "<LabelStyle>");
	kml_print (API, Out, N, "<scale>%g</scale>", Ctrl->S.scale[N_ID]);
	kml_print (API, Out, N, "<color>%02x%02x%02x%02x</color>", gmt_M_u255 (1.0 - Ctrl->G.fill[N_ID].rgb[3]), GMT_3u255 (Ctrl->G.fill[N_ID].rgb));
	kml_print (API, Out, --N, "</LabelStyle>");
	kml_print (API, Out, --N, "</Style>");

	for (index = -3; Ctrl->C.active && index < (int)P->n_colors; index++) {	/* Place styles for each color in CPT */
		get_rgb_lookup (GMT, P, index, rgb);	/* For -3, -2, -1 we get the back, fore, nan colors */
		kml_print (API, Out, N++, "<Style id=\"st-%d-%d\">", process_id, index + 4); /* +4 to make the first style ID = 1 */
		if (Ctrl->F.mode < LINE)	/* Set icon style (applies to symbols only */
			set_iconstyle (API, Out, rgb, Ctrl->S.scale[F_ID], Ctrl->I.file, N);
		else if (Ctrl->F.mode == LINE)	/* Line style only */
			set_linestyle (API, Out, &Ctrl->W.pen, rgb, N);
		else {	/* Polygons */
			if (!Ctrl->W.active) {	/* Only fill, no outline */
				set_polystyle (API, Out, rgb, false, Ctrl->G.active[F_ID], N);
			}
			else if (Ctrl->W.pen.cptmode == 2) { /* Use -C for fill, -W for outline */
				set_polystyle (API, Out, rgb, true, Ctrl->G.active[F_ID], N);
				set_linestyle (API, Out, &Ctrl->W.pen, Ctrl->W.pen.rgb, N);
			}
			else if (Ctrl->W.pen.cptmode == 1) { /* Use -G for fill, -C for outline */
				set_polystyle (API, Out, Ctrl->G.fill[F_ID].rgb, true, Ctrl->G.active[F_ID], N);
				set_linestyle (API, Out, &Ctrl->W.pen, rgb, N);
			}
			else if (Ctrl->W.pen.cptmode == 3) { /* Use -C for fill and outline */
				set_polystyle (API, Out, rgb, true, Ctrl->G.active[F_ID], N);
				set_linestyle (API, Out, &Ctrl->W.pen, rgb, N);
			}
		}
		kml_print (API, Out, --N, "</Style>");
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
		kml_print (API, Out, N++, "<%s>", Document[KML_FOLDER]);
		kml_print (API, Out, N, "<name>%s</name>", Ctrl->T.folder);
		if (Ctrl->Z.invisible) kml_print (API, Out, N, "<visibility>0</visibility>");
		if (Ctrl->Z.open) kml_print (API, Out, N, "<open>1</open>");
	}

	if (Ctrl->D.active) {	/* Add in a description HTML snipped */
		char line[GMT_BUFSIZ];
		FILE *fp = NULL;
		if ((fp = gmt_fopen (GMT, Ctrl->D.file, "r")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Could not open description file %s\n", Ctrl->D.file);
			Return (GMT_RUNTIME_ERROR);
		}
		kml_print (API, Out, N++, "<description>");
		kml_print (API, Out, N++, "<![CDATA[");
		while (gmt_fgets (GMT, line, GMT_BUFSIZ, fp)) kml_print (API, Out, N, "%s", line);
		gmt_fclose (GMT, fp);
		kml_print (API, Out, --N, "]]>");
		kml_print (API, Out, --N, "</description>");
	}

	if (GMT->common.R.active[RSET] && first)	/* Issue Region tag as given on command line */
		place_region_tag (API, Out, GMT->common.R.wesn, Ctrl->Z.min, Ctrl->Z.max, N);
	else if (Ctrl->R2.automatic) {	/* Issue Region tag */
		double wesn[4];
		/* This information is part of the dataset header and obtained during reading */
		wesn[XLO] = D->min[GMT_X];	wesn[XHI] = D->max[GMT_X];
		wesn[YLO] = D->min[GMT_Y];	wesn[YHI] = D->max[GMT_Y];
		place_region_tag (API, Out, wesn, Ctrl->Z.min, Ctrl->Z.max, N);
	}
	set_nr = pnt_nr = 0;

	if (Ctrl->F.mode == WIGGLE) kml = kml_alloc (GMT, D);

	for (tbl = 0; tbl < n_tables; tbl++) {
		n_segments = D->table[tbl]->n_segments;
		TH = gmt_get_DT_hidden (D->table[tbl]);
		file = TH->file;
		if (file[GMT_IN]) {	/* Place all of this file's content in its own named folder */
			kml_print (API, Out, N++, "<Folder>");
			if (!strcmp (file[GMT_IN], "<stdin>"))
				kml_print (API, Out, N, "<name>stdin</name>");
			else
				kml_print (API, Out, N, "<name>%s</name>", basename (file[GMT_IN]));
		}
		for (seg = 0; seg < n_segments; seg++) {	/* Process each segment in this table */
			pnt_nr = 0;
			n_rows = D->table[tbl]->segment[seg]->n_rows;
			label = D->table[tbl]->segment[seg]->label;
			header = D->table[tbl]->segment[seg]->header;

			/* Only point sets will be organized in folders as lines/polygons are single entities */
			if (Ctrl->F.mode < LINE) {	/* Meaning point-types, not lines or polygons */
				kml_print (API, Out, N++, "<Folder>");
				if (Ctrl->N.mode == NO_LABEL) { /* Nothing */ }
				else if (label)
					kml_print (API, Out, N, "<name>%s</name>", label);
				else
					kml_print (API, Out, N, "<name>%s Set %d</name>", name[Ctrl->F.mode], set_nr);
				if (gmt_M_compat_check (GMT, 4))	/* GMT4 LEVEL: Accept either -D or -T */
					act = (gmt_parse_segment_item (GMT, header, "-D", buffer) || gmt_parse_segment_item (GMT, header, "-T", buffer));
				else
					act = (gmt_parse_segment_item (GMT, header, "-T", buffer));
				if (act)
					kml_print (API, Out, N, "<description>%s</description>", description);
			}
			else if (Ctrl->F.mode < WIGGLE) {	/* Line or polygon means we lay down the placemark first*/
				if (Ctrl->C.active && gmt_parse_segment_item (GMT, header, "-Z", description)) {
					double z_val = atof (description);
					index = gmt_get_index (GMT, P, z_val);
				}
				kml_print (API, Out, N++, "<Placemark>");
				if (Ctrl->N.mode == NO_LABEL) { /* Nothing */ }
				else if (Ctrl->N.mode == FMT_LABEL) {
					kml_print (API, Out, N, "<name>");
					kml_print (API, Out, 0, Ctrl->N.fmt, (int)set_nr);
					kml_print (API, Out, 0, "</name>");
				}
				else if (label)
					kml_print (API, Out, N, "<name>%s</name>", label);
				else
					kml_print (API, Out, N, "<name>%s %d</name>", name[Ctrl->F.mode], set_nr);
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
					kml_print (API, Out, N, "<description>%s</description>", description);
				kml_print (API, Out, N, "<styleUrl>#st-%d-%d</styleUrl>", process_id, index + 4); /* +4 to make style ID  >= 0 */
				kml_print (API, Out, N++, "<%s>", feature[Ctrl->F.mode]);
				print_altmode (API, Out, Ctrl->E.extrude, Ctrl->E.tessellate, Ctrl->A.mode, N);
				if (Ctrl->F.mode == POLYGON) {
					kml_print (API, Out, N++, "<outerBoundaryIs>");
					kml_print (API, Out, N++, "<LinearRing>");
				}
				kml_print (API, Out, N++, "<coordinates>");
			}
			if (Ctrl->F.mode == WIGGLE) {
				bool positive;
				struct GMT_DATASEGMENT *S = D->table[tbl]->segment[seg];
				double dz, *lon, *lat, *z;
				/* Separate loop over rows since different than the other cases */
				lon = S->data[GMT_X];
				lat = S->data[GMT_Y];
				z = S->data[GMT_Z];
				if (!Ctrl->G.active[F_ID]) {	/* Wants to fill positive wiggles, must get those polygons */
					kml_print (API, Out, N++, "<Folder>");
					kml_print (API, Out, N, "<name>Positive Wiggles</name>");
					kml->lon[0] = lon[0];
					kml->lat[0] = lat[0];
					kml->z[0]   = z[0];
					for (row = kml->n_in = 1; row < S->n_rows; row++) {	/* Convert to inches/cm and get distance increments */
						if (kml->n_in > 0 && gmt_M_is_dnan (z[row])) {	/* Data gap, plot what we have */
							positive = (z[kml->n_in-1] > 0.0);
							if (positive) kml_plot_wiggle (GMT, Out, kml, Ctrl->Q.scale, Ctrl->Q.mode, Ctrl->Q.value, true, false, process_id, Ctrl->A.mode, N, Ctrl->A.altitude);
							kml->n_in = 0;
						}
						else if (!gmt_M_is_dnan (z[row-1]) && (z[row]*z[row-1] < 0.0 || z[row] == 0.0)) {	/* Crossed 0, add new point and plot */
							dz = z[row] - z[row-1];
							kml->lon[kml->n_in] = (dz == 0.0) ? lon[row] : lon[row-1] + fabs (z[row-1] / dz) * (lon[row] - lon[row-1]);
							kml->lat[kml->n_in] = (dz == 0.0) ? lat[row] : lat[row-1] + fabs (z[row-1] / dz) * (lat[row] - lat[row-1]);
							kml->z[kml->n_in++] = 0.0;
							positive = (kml->z[kml->n_in-2] > 0.0);
							if (positive) kml_plot_wiggle (GMT, Out, kml, Ctrl->Q.scale, Ctrl->Q.mode, Ctrl->Q.value, true, false, process_id, Ctrl->A.mode, N, Ctrl->A.altitude);
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
						if (positive) kml_plot_wiggle (GMT, Out, kml, Ctrl->Q.scale, Ctrl->Q.mode, Ctrl->Q.value, true, false, process_id, Ctrl->A.mode, N, Ctrl->A.altitude);
					}
					kml_print (API, Out, --N, "</Folder>");
				}
				if (Ctrl->W.active) {	/* Draw the entire wiggle */
					gmt_M_memcpy (kml->lon, lon, S->n_rows, double);
					gmt_M_memcpy (kml->lat, lat, S->n_rows, double);
					gmt_M_memcpy (kml->z, z, S->n_rows, double);
					kml->n_in = S->n_rows;
					kml_plot_wiggle (GMT, Out, kml, Ctrl->Q.scale, Ctrl->Q.mode, Ctrl->Q.value, false, true, process_id, Ctrl->A.mode, N, Ctrl->A.altitude);
				}
			}
			else {
				unsigned int col;
				bool all_text = ((Ctrl->L.n_cols + n_coord) - D->n_columns == 1);	/* Use all trailing text as single last -L item */
				char *word = NULL;
				struct GMT_DATASEGMENT *S = D->table[tbl]->segment[seg];
				for (row = 0; row < n_rows; row++) {
					for (col = 0; col < n_coord; col++)
						out[col] = S->data[col][row];
					if (GMT->common.R.active[RSET] && check_lon_lat (GMT, &out[GMT_X], &out[GMT_Y])) continue;
					if (get_z) {	/* For point data we use z to determine color */
						if (Ctrl->C.active) index = gmt_get_index (GMT, P, out[GMT_Z]);
						out[GMT_Z] = Ctrl->A.get_alt ? out[GMT_Z] * Ctrl->A.scale : Ctrl->A.altitude;
					}
					if (Ctrl->F.mode < LINE && gmt_M_is_dnan (out[GMT_Z])) continue;	/* Symbols with NaN height are not plotted anyhow */

					if (Ctrl->F.mode < LINE) {	/* Print the information for this point */
						kml_print (API, Out, N++, "<Placemark>");
						if (Ctrl->N.mode == NO_LABEL) { /* Nothing */ }
						else if (Ctrl->N.mode == GET_COL_LABEL) {
							gmt_ascii_format_one (GMT, item, S->data[Ctrl->N.col][row], gmt_M_type(GMT,GMT_IN,Ctrl->N.col));
							kml_print (API, Out, N, "<name>%s</name>", item);
						}
						else if (Ctrl->N.mode == GET_LABEL)
							kml_print (API, Out, N, "<name>%s</name>", S->text[row]);
						else if (Ctrl->N.mode == FMT_LABEL) {
							kml_print (API, Out, N, "<name>");
							kml_print (API, Out, 0, Ctrl->N.fmt, pnt_nr);
							kml_print (API, Out, 0, "</name>");
						}
						else if (label && n_rows > 1)
							kml_print (API, Out, N, "<name>%s %" PRIu64 "</name>", label, row);
						else if (label)
							kml_print (API, Out, N, "<name>%s</name>", label);
						else
							kml_print (API, Out, N, "<name>%s %d</name>", name[Ctrl->F.mode], pnt_nr);
						if (Ctrl->L.n_cols) {
							size_t P = 0;
							char *trail, *orig = strdup (S->text[row]);
							trail = orig;
							kml_print (API, Out, N++, "<ExtendedData>");
							for (col = 0; col < Ctrl->L.n_cols; col++) {
								kml_print (API, Out, N, "<Data name = \"%s\">", Ctrl->L.name[col]);
								kml_print (API, Out, N++, "<value>");
								if ((n_coord+col) < S->n_columns) {	/* Part of the numerical section */
									gmt_ascii_format_one (GMT, item, S->data[n_coord+col][row], gmt_M_type(GMT,GMT_IN,n_coord+col));
									kml_print (API, Out, N, "%s", item);
								}
								else if (all_text) {	/* Place entire trailing text */
									kml_print (API, Out, N, "%s", orig);
								}
								else {	/* Biting into trailing text */
									word = strsepz (&trail, " \t", &P);
									strcpy (item, word);
									L = strlen (item);
									if (L && item[0] == '\"' && item[L-1] == '\"') {	/* Quoted string on input, remove quotes on output */
										item[L-1] = '\0';
										kml_print (API, Out, N, "%s", &item[1]);
									}
									else
										kml_print (API, Out, N, "%s", item);
								}
								kml_print (API, Out, --N, "</value>");
								kml_print (API, Out, N, "</Data>");
							}
							gmt_M_str_free (orig);
							kml_print (API, Out, --N, "</ExtendedData>");
						}
						if (Ctrl->F.mode == SPAN) {
							kml_print (API, Out, N++, "<TimeSpan>");
							if (!gmt_M_is_dnan (out[t1_col])) {
								gmt_ascii_format_col (GMT, text, out[t1_col], GMT_OUT, t1_col);
								kml_print (API, Out, N, "<begin>%s</begin>", text);
							}
							if (!gmt_M_is_dnan (out[t2_col])) {
								gmt_ascii_format_col (GMT, text, out[t2_col], GMT_OUT, t2_col);
								kml_print (API, Out, N, "<end>%s</end>", text);
							}
							kml_print (API, Out, --N, "</TimeSpan>");
						}
						else if (Ctrl->F.mode == EVENT) {
							kml_print (API, Out, N++, "<TimeStamp>");
							gmt_ascii_format_col (GMT, text, out[t1_col], GMT_OUT, t1_col);
							kml_print (API, Out, N, "<when>%s</when>", text);
							kml_print (API, Out, --N, "</TimeStamp>");
						}
						kml_print (API, Out, N, "<styleUrl>#st-%d-%d</styleUrl>", process_id, index + 4); /* +4 to make index a positive integer */
						kml_print (API, Out, N++, "<%s>", feature[Ctrl->F.mode]);
						print_altmode (API, Out, Ctrl->E.extrude, Ctrl->E.tessellate, Ctrl->A.mode, N);
						kml_print (API, Out, N, "<coordinates>");
						ascii_output_three (API, Out, out, N);
						kml_print (API, Out, N, "</coordinates>");
						kml_print (API, Out, --N, "</%s>", feature[Ctrl->F.mode]);
						kml_print (API, Out, --N, "</Placemark>");
					}
					else {	/* For lines and polygons we just output the coordinates */
						if (gmt_M_is_dnan (out[GMT_Z])) out[GMT_Z] = 0.0;	/* Google Earth can not handle lines at NaN altitude */
						ascii_output_three (API, Out, out, N);
						if (row > 0 && no_dateline && crossed_dateline (out[GMT_X], last_x)) {
							/* GE cannot handle polygons crossing the dateline; warn for now */
							GMT_Report (API, GMT_MSG_WARNING,
							            "At least on polygon is straddling the Dateline. Google Earth will wrap these the wrong way\n");
							GMT_Report (API, GMT_MSG_WARNING,
							            "Split such polygons into East and West parts and plot them as separate polygons.\n");
							GMT_Report (API, GMT_MSG_WARNING, "Use gmtconvert to help in this conversion.\n");
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
				kml_print (API, Out, --N, "</Folder>");
			else {
				kml_print (API, Out, --N, "</coordinates>");
				if (Ctrl->F.mode == POLYGON) {
					kml_print (API, Out, --N, "</LinearRing>");
					kml_print (API, Out, --N, "</outerBoundaryIs>");
				}
				kml_print (API, Out, --N, "</%s>", feature[Ctrl->F.mode]);
				kml_print (API, Out, --N, "</Placemark>");
			}
			set_nr++;
		}
		if (file[GMT_IN]) kml_print (API, Out, --N, "</Folder>");
	}
	if (use_folder) kml_print (API, Out, --N, "</%s>", Document[KML_FOLDER]);
	if (!GMT->common.K.active) {
		kml_print (API, Out, --N, "</%s>", Document[KML_DOCUMENT]);
		kml_print (API, Out, 0, "</kml>");
	}

	if (Ctrl->F.mode == WIGGLE) kml_free (GMT, &kml);

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}
	gmt_M_free (GMT, Out);
	Return (GMT_NOERROR);	/* Garbage collection will free D */
}
