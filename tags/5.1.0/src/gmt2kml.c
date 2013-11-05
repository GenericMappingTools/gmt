/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>KOVabfghi" GMT_OPT("HMm")

int gmt_parse_R_option (struct GMT_CTRL *GMT, char *item);
void GMT_get_rgb_lookup (struct GMT_CTRL *GMT, struct GMT_PALETTE *P, int index, double value, double *rgb);

#define POINT			0
#define EVENT			1
#define SPAN			2
#define LINE			3
#define POLYGON			4
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
#define GMT_3u255(t) GMT_u255(t[2]),GMT_u255(t[1]),GMT_u255(t[0])

#define F_ID	0	/* Indices into arrays */
#define N_ID	1
#define ALT	0
#define LOD	1
#define FADE	2

struct GMT2KML_CTRL {
	double t_transp;
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
	struct W {	/* -W<pen> */
		bool active;
		unsigned int mode;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z */
		bool active;
		bool invisible;
		bool open;
		double min[3], max[3];
	} Z;
};

void *New_gmt2kml_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMT2KML_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMT2KML_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.mode = KML_GROUND;
	C->A.scale = 1.0;
	C->F.mode = POINT;
	C->F.geometry = GMT_IS_POINT;
	GMT_init_fill (GMT, &C->G.fill[F_ID], 1.0, 192.0 / 255.0, 128.0 / 255.0);	/* Default fill color */
	GMT_init_fill (GMT, &C->G.fill[N_ID], 1.0, 1.0, 1.0);				/* Default text color */
	C->G.fill[N_ID].rgb[3] = 0.25;	/* Default text transparency */
	C->I.file = strdup ("http://maps.google.com/mapfiles/kml/pal4/icon57.png");	/* Default icon */
	C->t_transp = 1.0;
	C->S.scale[N_ID] = C->S.scale[F_ID] = 1.0;
	C->Z.max[ALT] = -1.0;
	C->W.pen = GMT->current.setting.map_default_pen; C->W.pen.width = 1.0;		/* Default pen width = 1p */

	return (C);
}

void Free_gmt2kml_Ctrl (struct GMT_CTRL *GMT, struct GMT2KML_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->C.file) free (C->C.file);
	if (C->D.file) free (C->D.file);
	if (C->I.file) free (C->I.file);
	if (C->N.fmt) free (C->N.fmt);
	if (C->T.title) free (C->T.title);
	if (C->T.folder) free (C->T.folder);
	if (C->L.active) {
		unsigned int col;
		for (col = 0; col < C->L.n_cols; col++) free ((void *)C->L.name[col]);
		GMT_free (GMT, C->L.name);
	}
	GMT_free (GMT, C);
}

int GMT_gmt2kml_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the gmt2kml synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmt2kml [<table>] [-Aa|g|s[<altitude>|x<scale>]] [-C<cpt>] [-D<descriptfile>] [-E]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-Fe|s|t|l|p] [-Gf|n[-|<fill>] [-I<icon>] [-K] [-L<name1>,<name2>,...]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-N-|+|<template>|<name>] [-O] [-Q[e|s|t|l|p|n]<transp>] [-Ra|<w>/<e>/<s>/n>] [-Sc|n<scale>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T<title>[/<foldername>] [%s] [-W-|<pen>] [-Z<opts>] [%s]\n", GMT_V_OPT, GMT_a_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s]\n\n", GMT_bi_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Altitude mode, choose among three modes:\n");
	GMT_Message (API, GMT_TIME_NONE,"\t      a Absolute altitude.\n");
	GMT_Message (API, GMT_TIME_NONE,"\t      g Altitude relative to sea surface or ground.\n");
	GMT_Message (API, GMT_TIME_NONE,"\t      s Altitude relative to seafloor or ground.\n");
	GMT_Message (API, GMT_TIME_NONE,"\t    Optionally, append fixed <altitude>, or x<scale> [g0: Clamped to sea surface or ground].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Append color palette name to color symbols by third column z-value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D File with HTML snippets to use for data description [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Extend feature down to the ground [no extrusion].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Feature type; choose from (e)vent, (s)symbol, (t)imespan, (l)ine, or (p)olygon [s].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   All features expect lon, lat in the first two columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Value or altitude is given in the third column (see -A and -C).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Event requires a timestamp in the next column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Timespan requires begin and end timestamps in the next two columns\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (use NaN for unlimited begin and/or end times).\n");
	GMT_rgb_syntax (API->GMT, 'G', "Set color for symbol/polygon fill (-Gf<color>) or label (-Gn<color>).");
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
	GMT_Message (API, GMT_TIME_NONE, "\t   1. Specify -N- if the first non-coordinate column of the data record should be used as single-word label (-Fe|s|t only).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   2. Specify -N+ if the rest of the data record should be used as label (-Fe|s|t only).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   3. Append a string that may contain the format %%d for a running feature count.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   4. Give no argument to indicate no labels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-O Append the KML code to an existing document [OFF].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-R Issue Region tag.  Append w/e/s/n to set a particular region or append a to use the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   actual domain of the data (single file only) [no region specified].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Scale for (c)ircle icon size or (n)ame label [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Append KML document title name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally append /<foldername> to name folder when used with\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -O and -K to organize features into groups.\n");
	GMT_Option (API, "V");
	GMT_pen_syntax (API->GMT, 'W', "Specify pen attributes for lines and polygon outlines [Default is %s].");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give width in pixels and append p.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   A leading + applies cpt color (-C) to both polygon fill and outline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   A leading - applies cpt color (-C) to the outline only.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Control visibility of features.  Append one or more modifiers:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +a<alt_min>/<alt_max> inserts altitude limits [no limit].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +l<minLOD>/<maxLOD>] sets Level Of Detail when layer should be active [always active].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     layer goes inactive when there are fewer than minLOD pixels or more\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     than maxLOD pixels visible.  -1 means never invisible.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +f<minfade>/<maxfade>] sets distances over which we fade from opaque.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     to transparent [no fading].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +v turns off visibility [feature is visible].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +o open document or folder when loaded [closed].\n");
	GMT_Option (API, "a,bi2,f,g,h,i,:,.");

	return (EXIT_FAILURE);
}

int GMT_gmt2kml_parse (struct GMT_CTRL *GMT, struct GMT2KML_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmt2kml and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos = 0, k, n_files = 0;
	size_t n_alloc = 0;
	char buffer[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""}, T[4][GMT_LEN64], *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)) n_errors++;
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Altitude mode */
				Ctrl->A.active = true;
				switch (opt->arg[1]) {
					case 'x':
						Ctrl->A.scale = atof (&opt->arg[2]);
						Ctrl->A.get_alt = true;
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
				if (Ctrl->C.file) free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Description file */
				Ctrl->D.active = true;
				if (Ctrl->D.file) free (Ctrl->D.file);
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
					default:
						GMT_Message (API, GMT_TIME_NONE, "Bad feature type. Use s, e, t, l or p.\n");
						n_errors++;
						break;
				}
				break;
			case 'G':	/* Set fill for symbols or polygon */
				switch (opt->arg[0]) {
					case 'f':	/* Symbol/polygon color fill */
						if (opt->arg[1] == '-')
				 			Ctrl->G.active[F_ID] = true;
						else if (!opt->arg[1] || GMT_getfill (GMT, &opt->arg[1], &Ctrl->G.fill[F_ID])) {
							GMT_fill_syntax (GMT, 'G', "(-Gf or -Gn)");
							n_errors++;
						}
						break;
					case 'n':	/* Label name color */
		 				Ctrl->G.active[N_ID] = true;
						if (opt->arg[1] == '-')
							Ctrl->t_transp = 0.0;
						else if (!opt->arg[1] || GMT_getfill (GMT, &opt->arg[1], &Ctrl->G.fill[N_ID])) {
							GMT_fill_syntax (GMT, 'G', "(-Gf or -Gn)");
							n_errors++;
						}
						break;
					default:
						GMT_fill_syntax (GMT, 'G', "(-Gf or -Gn)");
						n_errors++;
						break;
				}
				break;
			case 'I':	/* Custom icon */
	 			Ctrl->I.active = true;
				free (Ctrl->I.file);
				if (opt->arg[0] == '+')
					sprintf (buffer, "http://maps.google.com/mapfiles/kml/%s", &opt->arg[1]);
				else if (opt->arg[0])
					strncpy (buffer, opt->arg, GMT_BUFSIZ);
				Ctrl->I.file = strdup (buffer);
				break;
			case 'L':	/* Extended data */
 				Ctrl->L.active = true;
				pos = Ctrl->L.n_cols = 0;
				while ((GMT_strtok (opt->arg, ",", &pos, p))) {
					if (Ctrl->L.n_cols == n_alloc) Ctrl->L.name = GMT_memory (GMT, Ctrl->L.name, n_alloc += GMT_TINY_CHUNK, char *);
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
			case 'R':	/* Region setting */
				Ctrl->R2.active = true;
				if (opt->arg[0] == 'a')	/* Get args from data domain */
					Ctrl->R2.automatic = true;
				else if (opt->arg[0])
					n_errors += gmt_parse_R_option (GMT, opt->arg);
				break;
			case 'S':	/* Scale for symbol or text */
				Ctrl->S.active = true;
				if (opt->arg[0] == 'f')
					Ctrl->S.scale[F_ID] = atof (&opt->arg[1]);
				else if (opt->arg[0] == 'n')
					Ctrl->S.scale[N_ID] = atof (&opt->arg[1]);
				else {
					GMT_Message (API, GMT_TIME_NONE, "-S requires f or n, then size\n");
					n_errors++;
				}
				break;
			case 'T':	/* Title [and folder] */
				Ctrl->T.active = true;
				if ((c = strchr (opt->arg, '/'))) {	/* Got both title and folder */
					if (c[1]) Ctrl->T.folder = strdup (&c[1]);
					*c = '\0';
					if (opt->arg[0]) Ctrl->T.title = strdup (opt->arg);
					*c = '/';
				}
				else if (opt->arg[0]) Ctrl->T.title = strdup (opt->arg);
				break;
			case 'W':	/* Pen attributes */
				Ctrl->W.active = true;
				k = 0;
				if (opt->arg[k] == '-') {Ctrl->W.mode = 1; k++;}
				if (opt->arg[k] == '+') {Ctrl->W.mode = 2; k++;}
				if (opt->arg[k] && GMT_getpen (GMT, &opt->arg[k], &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', "sets pen attributes [Default pen is %s]:");
					GMT_Report (API, GMT_MSG_NORMAL, "\t   A leading + applies cpt color (-C) to both symbol fill and pen.\n");
					GMT_Report (API, GMT_MSG_NORMAL, "\t   A leading - applies cpt color (-C) to the pen only.\n");
					n_errors++;
				}
				break;
			case 'Z':	/* Visibility control */
				Ctrl->Z.active = true;
				pos = 0;
				while ((GMT_strtok (&opt->arg[1], "+", &pos, p))) {
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

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && !Ctrl->C.file, "Syntax error -C option: Need to supply color palette name\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && access (Ctrl->D.file, R_OK), "Syntax error -D: Cannot open HTML description file %s\n", Ctrl->D.file);
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_OUT], "Syntax error: Cannot produce binary KML output\n");
	n_errors += GMT_check_condition (GMT, Ctrl->R2.automatic && n_files > 1, "Syntax error: -Ra without arguments only accepted for single table\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.scale[F_ID] < 0.0 || Ctrl->S.scale[N_ID] < 0.0, "Syntax error: -S takes scales > 0.0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->t_transp < 0.0 || Ctrl->t_transp > 1.0, "Syntax error: -Q takes transparencies in range 0-1\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.mode == GET_LABEL && Ctrl->F.mode >= LINE, "Syntax error: -N+ not valid for lines and polygons\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->W.pen.width < 1.0, "Syntax error: -W given pen width < 1 pixel.  Use integers and append p as unit.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->W.mode && !Ctrl->C.active, "Syntax error: -W option +|-<pen> requires the -C option.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int check_lon_lat (struct GMT_CTRL *GMT, double *lon, double *lat)
{
	if (*lat < GMT->common.R.wesn[YLO] || *lat > GMT->common.R.wesn[YHI]) return (true);
	if (*lon < GMT->common.R.wesn[XLO]) *lon += 360.0;
	if (*lon > GMT->common.R.wesn[XHI]) *lon -= 360.0;
	if (*lon < GMT->common.R.wesn[XLO]) return (true);
	return (false);
}

void tabs (int ntabs)
{	/* Outputs the n leading tabs for this KML line */
	while (ntabs--) putchar ('\t');
}

void print_altmode (int extrude, int fmode, int altmode, int ntabs)
{
	char *RefLevel[5] = {"clampToGround", "relativeToGround", "absolute", "relativeToSeaFloor", "clampToSeaFloor"};
	if (extrude) tabs (ntabs), printf ("<extrude>1</extrude>\n");
	if (fmode) tabs (ntabs), printf ("<tessellate>1</tessellate>\n");
	if (altmode == KML_GROUND_REL || altmode == KML_ABSOLUTE) tabs (ntabs), printf ("<altitudeMode>%s</altitudeMode>\n", RefLevel[altmode]);
	if (altmode == KML_SEAFLOOR_REL || altmode == KML_SEAFLOOR) tabs (ntabs), printf ("<gx:altitudeMode>%s</gx:altitudeMode>\n", RefLevel[altmode]);
}

int ascii_output_one (struct GMT_CTRL *GMT, double x, int col)
{	/* Used instead of GMT_ascii_output_col since Windoze has trouble with GMT->session.std[GMT_OUT] and stdout mixing */
	char text[GMT_LEN256] = {""};

	GMT_ascii_format_col (GMT, text, x, GMT_OUT, col);
	return (printf ("%s", text));
}

void place_region_tag (struct GMT_CTRL *GMT, double wesn[], double min[], double max[], int N)
{
	if (GMT_360_RANGE (wesn[XLO], wesn[XHI])) { wesn[XLO] = -180.0; wesn[XHI] = +180.0;}
	tabs (N++); printf ("<Region>\n");
	tabs (N++); printf ("<LatLonAltBox>\n");
	tabs (N);	printf ("<north>");	ascii_output_one (GMT, wesn[YHI], GMT_Y);	printf ("</north>\n");
	tabs (N);	printf ("<south>");	ascii_output_one (GMT, wesn[YLO], GMT_Y);	printf ("</south>\n");
	tabs (N);	printf ("<east>");	ascii_output_one (GMT, wesn[XHI], GMT_X);	printf ("</east>\n");
	tabs (N);	printf ("<west>");	ascii_output_one (GMT, wesn[XLO], GMT_X);	printf ("</west>\n");
	if (max[ALT] > min[ALT]) {
		tabs (N); printf ("<minAltitude>%g</minAltitude>\n", min[ALT]);
		tabs (N); printf ("<maxAltitude>%g</maxAltitude>\n", max[ALT]);
	}
	tabs (--N); printf ("</LatLonAltBox>\n");
	if (max[LOD] != min[LOD]) {
		tabs (N++); printf ("<Lod>\n");
		tabs (N); printf ("<minLodPixels>%ld</minLodPixels>\n", lrint (min[LOD]));
		tabs (N); printf ("<maxLodPixels>%ld</maxLodPixels>\n", lrint (max[LOD]));
		if (min[FADE] > 0.0 || max[FADE] > 0.0) {
			tabs (N); printf ("<minFadeExtent>%g</minFadeExtent>\n", min[FADE]);
			tabs (N); printf ("<maxFadeExtent>%g</maxFadeExtent>\n", max[FADE]);
		}
		tabs (--N); printf ("</Lod>\n");
	}
	tabs (--N); printf ("</Region>\n");
}

void set_iconstyle (double *rgb, double scale, char *iconfile, int N)
{	/* No icon = no symbol */
	tabs (N++); printf ("<IconStyle>\n");
	tabs (N++); printf ("<Icon>\n");
	if (iconfile[0] != '-') { tabs (N); printf ("<href>%s</href>\n", iconfile); }
	tabs (--N); printf ("</Icon>\n");
	if (iconfile[0] != '-') {
		tabs (N); printf ("<scale>%g</scale>\n", scale);
		tabs (N); printf ("<color>%02x%02x%02x%02x</color>\n", GMT_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	}
	tabs (--N); printf ("</IconStyle>\n");
}

void set_linestyle (struct GMT_PEN *pen, double *rgb, int N)
{
	tabs (N++); printf ("<LineStyle>\n");
	tabs (N); printf ("<color>%02x%02x%02x%02x</color>\n", GMT_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	tabs (N); printf ("<width>%ld</width>\n", lrint (pen->width));
	tabs (--N); printf ("</LineStyle>\n");
}

void set_polystyle (double *rgb, int outline, int active, int N)
{
	tabs (N++); printf ("<PolyStyle>\n");
	tabs (N); printf ("<color>%02x%02x%02x%02x</color>\n", GMT_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	tabs (N); printf ("<fill>%d</fill>\n", !active);
	tabs (N); printf ("<outline>%d</outline>\n", outline);
	tabs (--N); printf ("</PolyStyle>\n");
}

void get_rgb_lookup (struct GMT_CTRL *GMT, struct GMT_PALETTE *P, int index, double *rgb)
{	/* Special version of GMT_get_rgb_lookup since no interpolation can take place */

	if (index < 0) {	/* NaN, Foreground, Background */
		GMT_rgb_copy (rgb, P->patch[index+3].rgb);
		P->skip = P->patch[index+3].skip;
	}
	else if (P->range[index].skip) {		/* Set to page color for now */
		GMT_rgb_copy (rgb, GMT->current.setting.ps_page_rgb);
		P->skip = true;
	}
	else {	/* Return low color */
		GMT_memcpy (rgb, P->range[index].rgb_low, 4, double);
		P->skip = false;
	}
}

int get_data_region (struct GMT_CTRL *GMT, struct GMT_TEXTSET *D, double wesn[])
{
	/* Because we read as textset we must determine the data extent the hard way */
	unsigned int tbl, ix, iy, way;
	uint64_t row, seg;
	char T[2][GMT_LEN64];
	double x, y, y_min = 90.0, y_max = -90.0;
	struct GMT_QUAD *Q = GMT_quad_init (GMT, 1);	/* Allocate and initialize one QUAD structure */
	ix = GMT->current.setting.io_lonlat_toggle[GMT_IN];	iy = 1 - ix;

	for (tbl = 0; tbl < D->n_tables; tbl++) {
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
			for (row = 0; row < D->table[tbl]->segment[seg]->n_rows; row++) {
				sscanf (D->table[tbl]->segment[seg]->record[row], "%s %s", T[ix], T[iy]);
				if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, T[GMT_X], GMT->current.io.col_type[GMT_IN][GMT_X], &x), T[GMT_X])) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not decode longitude from %s\n", T[GMT_X]);
					return (EXIT_FAILURE);
				}
				if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, T[GMT_Y], GMT->current.io.col_type[GMT_IN][GMT_Y], &y), T[GMT_Y])) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not decode latitude from %s\n", T[GMT_Y]);
					return (EXIT_FAILURE);
				}
				GMT_quad_add (GMT, Q, x);
				if (y < y_min) y_min = y;
				if (y > y_max) y_max = y;
			}
		}
	}
	way = GMT_quad_finalize (GMT, Q);
	wesn[XLO] = Q->min[way];	wesn[XHI] = Q->max[way];
	wesn[YLO] = y_min;		wesn[YHI] = y_max;
	GMT_free (GMT, Q);
	return (GMT_NOERROR);
}

bool crossed_dateline (double this_x, double last_x)
{
	if (this_x > 90.0 && this_x <= 180.0 && last_x > +180.0 && last_x < 270.0) return (true);	/* Positive lons 0-360 */
	if (last_x > 90.0 && last_x <= 180.0 && this_x > +180.0 && this_x < 270.0) return (true);	/* Positive lons 0-360 */
	if (this_x > 90.0 && this_x <= 180.0 && last_x > -180.0 && last_x < -90.0) return (true);	/* Lons in -180/+180 range */
	if (last_x > 90.0 && last_x <= 180.0 && this_x > -180.0 && this_x < -90.0) return (true);	/* Lons in -180/+180 range */
	return (false);
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmt2kml_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmt2kml (void *V_API, int mode, void *args)
{
	bool first = true, get_z = false, use_folder = false, do_description, no_dateline = false, act;
	unsigned int n_coord = 0, t1_col, t2_col, pnt_nr = 0, tbl, col, pos, ix, iy;

	uint64_t row, seg;
	int set_nr = 0, index = 0, N = 1, error = 0;

	char extra[GMT_BUFSIZ] = {""}, buffer[GMT_BUFSIZ] = {""}, description[GMT_BUFSIZ] = {""}, item[GMT_LEN128] = {""}, C[5][GMT_LEN64] = {"","","","",""};
	char *feature[5] = {"Point", "Point", "Point", "LineString", "Polygon"}, *Document[2] = {"Document", "Folder"};
	char *name[5] = {"Point", "Event", "Timespan", "Line", "Polygon"};

	double rgb[4], out[5], last_x;

	struct GMT_OPTION *options = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_TEXTSET *Din = NULL;
	struct GMT_TEXTTABLE *T = NULL;
	struct GMT2KML_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_gmt2kml_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_gmt2kml_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_gmt2kml_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_gmt2kml_Ctrl (GMT);		/* Allocate and initialize a new control structure */
	if ((error = GMT_gmt2kml_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmt2kml main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	if (GMT->common.b.active[GMT_IN]) {	/* Probably also when user uses the -i option to select columns */
		GMT_Message (API, GMT_TIME_NONE, "Cannot yet use binary data files; pipe in via gmtconvert for now\n");
		Return (EXIT_FAILURE);
	}

	/* gmt2kml only applies to geographic data so we do a -fg implicitly here */
	GMT_set_geographic (GMT, GMT_IN);
	GMT_set_geographic (GMT, GMT_OUT);
	extra[0] = '\0';
	GMT_memset (out, 5, double);	/* Set to zero */
	ix = GMT->current.setting.io_lonlat_toggle[GMT_IN];	iy = 1 - ix;

	if (Ctrl->C.active) {	/* Process CPT file */
		if ((P = GMT_Read_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (P->is_continuous) {
			GMT_Message (API, GMT_TIME_NONE, "Cannot use continuous color palette\n");
			Return (EXIT_FAILURE);
		}
	}

	/* Now we are ready to take on some input values */

	out[GMT_Z] = Ctrl->A.altitude;
	strcpy (GMT->current.setting.io_col_separator, ",");		/* Specify comma-separated output */
	GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;		/* Want -180/+180 longitude output format */
	strcpy (GMT->current.setting.format_float_out, "%.12g");	/* Make sure we use enough decimals */
	n_coord = (Ctrl->F.mode < LINE) ? Ctrl->F.mode + 2 : 2;
	get_z = (Ctrl->C.active || Ctrl->A.get_alt);
	if (get_z) n_coord++;
	t1_col = 2 + get_z;
	t2_col = 3 + get_z;
	if (Ctrl->F.mode == EVENT || Ctrl->F.mode == SPAN) GMT->current.io.col_type[GMT_IN][t1_col] = GMT->current.io.col_type[GMT_OUT][t1_col] = GMT_IS_ABSTIME;
	if (Ctrl->F.mode == SPAN)  GMT->current.io.col_type[GMT_IN][t2_col] = GMT->current.io.col_type[GMT_OUT][t2_col] = GMT_IS_ABSTIME;

	if (!Ctrl->T.folder) {
		sprintf (buffer, "%s Features", name[Ctrl->F.mode]);
		Ctrl->T.folder = strdup (buffer);
	}
	if (GMT->common.O.active || GMT->common.K.active) use_folder = true;	/* When at least one or -O, -K is used */
	if (GMT->common.O.active) {
		N++;	/* Due to the extra folder tag */
		tabs (N++); printf ("<%s>\n", Document[KML_FOLDER]);
		tabs (N); printf ("<name>%s</name>\n", Ctrl->T.folder);
	}
	else {
		/* Create KML header */
		printf ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		printf ("<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
		tabs (N++); printf ("<%s>\n", Document[KML_DOCUMENT]);
		if (Ctrl->T.title != NULL && *Ctrl->T.title != '\0')
			tabs (N), printf ("<name>%s</name>\n", Ctrl->T.title);
		if (Ctrl->Z.invisible) tabs (N), printf ("<visibility>0</visibility>\n");
		if (Ctrl->Z.open) tabs (N), printf ("<open>1</open>\n");
		if (use_folder) {
			tabs (N++); printf ("<%s>\n", Document[KML_FOLDER]);
			tabs (N); printf ("<name>%s</name>\n", Ctrl->T.folder);
		}
	}

	tabs (N++); printf ("<Style id=\"st%d\">\n", index);	/* Default style unless -C is used */

	/* Set icon style (applies to symbols only */
	set_iconstyle (Ctrl->G.fill[F_ID].rgb, Ctrl->S.scale[F_ID], Ctrl->I.file, N);

	/* Set shared line and polygon style (also for extrusions) */
	set_linestyle (&Ctrl->W.pen, Ctrl->W.pen.rgb, N);
	set_polystyle (Ctrl->G.fill[F_ID].rgb, !Ctrl->W.mode, Ctrl->G.active[F_ID], N);

	/* Set style for labels */
	tabs (N++); printf ("<LabelStyle>\n");
	tabs (N); printf ("<scale>%g</scale>\n", Ctrl->S.scale[N_ID]);
	tabs (N); printf ("<color>%02x%02x%02x%02x</color>\n", GMT_u255 (1.0 - Ctrl->G.fill[N_ID].rgb[3]), GMT_3u255 (Ctrl->G.fill[N_ID].rgb));
	tabs (--N); printf ("</LabelStyle>\n");
	tabs (--N); printf ("</Style>\n");

	for (index = -3; Ctrl->C.active && index < (int)P->n_colors; index++) {	/* Place styles for each color in CPT file */
		get_rgb_lookup (GMT, P, index, rgb);
		tabs (N++); printf ("<Style id=\"st%d\">\n", index + 4); /* +4 to make index a positive integer */
		if (Ctrl->F.mode < LINE)	/* Set icon style (applies to symbols only */
			set_iconstyle (Ctrl->G.fill[F_ID].rgb, Ctrl->S.scale[F_ID], Ctrl->I.file, N);
		else if (Ctrl->F.mode == LINE)	/* Line style only */
			set_linestyle (&Ctrl->W.pen, rgb, N);
		else {	/* Polygons */
			if (!Ctrl->W.active) {	/* Only fill, no outline */
				set_polystyle (rgb, false, Ctrl->G.active[F_ID], N);
			}
			else if (Ctrl->W.mode == 0) { /* Use -C for fill, -W for outline */
				set_polystyle (rgb, true, Ctrl->G.active[F_ID], N);
				set_linestyle (&Ctrl->W.pen, Ctrl->W.pen.rgb, N);
			}
			else if (Ctrl->W.mode == 1) { /* Use -G for fill, -C for outline */
				set_polystyle (Ctrl->G.fill[F_ID].rgb, true, Ctrl->G.active[F_ID], N);
				set_linestyle (&Ctrl->W.pen, rgb, N);
			}
			else if (Ctrl->W.mode == 2) { /* Use -C for fill and outline */
				set_polystyle (rgb, true, Ctrl->G.active[F_ID], N);
				set_linestyle (&Ctrl->W.pen, rgb, N);
			}
		}
		tabs (--N); printf ("</Style>\n");
	}
	index = -4;	/* Default unless -C changes things */
	if (Ctrl->D.active) {	/* Add in a description HTML snipped */
		char line[GMT_BUFSIZ];
		FILE *fp = NULL;
		if ((fp = GMT_fopen (GMT, Ctrl->D.file, "r")) == NULL) {
			GMT_Message (API, GMT_TIME_NONE, "Could not open description file %s\n", Ctrl->D.file);
			Return (EXIT_FAILURE);
		}
		tabs (N++); printf ("<description>\n");
		tabs (N++); printf ("<![CDATA[\n");
		while (GMT_fgets (GMT, line, GMT_BUFSIZ, fp)) tabs (N), printf ("%s", line);
		GMT_fclose (GMT, fp);
		tabs (--N); printf ("]]>\n");
		tabs (--N); printf ("</description>\n");
	}

	/* If binary input then call GMT_gmtconvert to handle that and read in as ascii, else do as below.
	   Or open as GMT_IS_DATASET if binary and TEXTSET otherwise, then handle the differenes below
	   by setting n_tables = (binary) ? D->n_tables : T->n_tables; same for nsegments, then
	   final ifs on parsing text record or using coord as is. */

	if (GMT_Init_IO (API, GMT_IS_TEXTSET, Ctrl->F.geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if ((Din = GMT_Read_Data (API, GMT_IS_TEXTSET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (GMT->common.R.active && first) {	/* Issue Region tag as given on commmand line*/
		place_region_tag (GMT, GMT->common.R.wesn, Ctrl->Z.min, Ctrl->Z.max, N);
		first = false;
	}
	else if (Ctrl->R2.automatic) {	/* Issue Region tag */
		double wesn[4];
		if (get_data_region (GMT, Din, wesn)) Return (EXIT_FAILURE);
		place_region_tag (GMT, wesn, Ctrl->Z.min, Ctrl->Z.max, N);
	}
	set_nr = pnt_nr = 0;

	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		T = Din->table[tbl];	/* Current table */
		if (T->file[GMT_IN]) {	/* Place all of this file's content in its own named folder */
			tabs (N++); printf ("<Folder>\n");
			tabs (N);
			if (!strcmp (T->file[GMT_IN], "<stdin>"))
				printf ("<name>stdin</name>\n");
			else
				printf ("<name>%s</name>\n", T->file[GMT_IN]);
		}
		for (seg = 0; seg < T->n_segments; seg++) {	/* Process each segment in this table */
			pnt_nr = 0;

			/* Only point sets will be organized in folders as lines/polygons are single entities */
			if (Ctrl->F.mode < LINE) {	/* Meaning point-types, not lines or polygons */
				tabs (N++); printf ("<Folder>\n");
				tabs (N);
				if (T->segment[seg]->label)
					printf ("<name>%s</name>\n", T->segment[seg]->label);
				else
					printf ("<name>%s Set %d</name>\n", name[Ctrl->F.mode], set_nr);
				if (GMT_compat_check (GMT, 4))	/* GMT4 LEVEL: Accept either -D or -T */
					act = (GMT_parse_segment_item (GMT, T->segment[seg]->header, "-D", buffer) || GMT_parse_segment_item (GMT, T->segment[seg]->header, "-T", buffer));
				else
					act = (GMT_parse_segment_item (GMT, T->segment[seg]->header, "-T", buffer));
				if (act) {
					tabs (N); printf ("<description>%s</description>\n", description);
				}
			}
			else {	/* Line or polygon means we lay down the placemark first*/
				if (Ctrl->C.active && GMT_parse_segment_item (GMT, T->segment[seg]->header, "-Z", description)) {
					double z_val = atof (description);
					index = GMT_get_index (GMT, P, z_val);
				}
				tabs (N++); printf ("<Placemark>\n");
				if (Ctrl->N.mode == NO_LABEL) { /* Nothing */ }
				else if (Ctrl->N.mode == FMT_LABEL) {
					tabs (N); printf ("<name>"); printf (Ctrl->N.fmt, (int)set_nr); printf ("</name>\n");
				}
				else if (T->segment[seg]->label)
					tabs (N), printf ("<name>%s</name>\n", T->segment[seg]->label);
				else
					tabs (N), printf ("<name>%s %d</name>\n", name[Ctrl->F.mode], set_nr);
				description[0] = 0;
				do_description = false;
				if (GMT_parse_segment_item (GMT, T->segment[seg]->header, "-I", buffer)) {
					do_description = true;
					strcat (description, buffer);
				}
				if (GMT_compat_check (GMT, 4))	/* GMT4 LEVEL: Accept either -D or -T */
					act = (GMT_parse_segment_item (GMT, T->segment[seg]->header, "-D", buffer) || GMT_parse_segment_item (GMT, T->segment[seg]->header, "-T", buffer));
				else
					act = (GMT_parse_segment_item (GMT, T->segment[seg]->header, "-T", buffer));
				if (act) {
					if (do_description) strcat (description, " ");
					strcat (description, buffer);
					do_description = true;
				}
				if (do_description) { tabs (N); printf ("<description>%s</description>\n", description); }
				tabs (N); printf ("<styleUrl>#st%d</styleUrl>\n", index + 4); /* +4 to make index a positive integer */
				tabs (N++); printf ("<%s>\n", feature[Ctrl->F.mode]);
				print_altmode (Ctrl->E.active, Ctrl->F.mode, Ctrl->A.mode, N);
				if (Ctrl->F.mode == POLYGON) {
					tabs (N++); printf ("<outerBoundaryIs>\n");
					tabs (N++); printf ("<LinearRing>\n");
				}
				tabs (N++); printf ("<coordinates>\n");
			}
			for (row = 0; row < T->segment[seg]->n_rows; row++) {
				switch (n_coord) {	/* Sort out input coordinates from remaining items which may be text */
					case 2:	/* Just lon, lat, label */
						sscanf (T->segment[seg]->record[row], "%s %s %[^\n]", C[ix], C[iy], extra);
						break;
					case 3:	/* Just lon, lat, a, extra */
						sscanf (T->segment[seg]->record[row], "%s %s %s %[^\n]", C[ix], C[iy], C[2], extra);
						break;
					case 4:	/* Just lon, lat, a, b, extra */
						sscanf (T->segment[seg]->record[row], "%s %s %s %s %[^\n]", C[ix], C[iy], C[2], C[3], extra);
						break;
					case 5:	/* Just lon, lat, z, t1, t2, extra */
						sscanf (T->segment[seg]->record[row], "%s %s %s %s %s %[^\n]", C[ix], C[iy], C[2], C[3], C[4], extra);
						break;
				}
				if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, C[GMT_X], GMT->current.io.col_type[GMT_IN][GMT_X], &out[GMT_X]), C[GMT_X])) {
					GMT_Message (API, GMT_TIME_NONE, "Error: Could not decode longitude from %s\n", C[GMT_X]);
					Return (EXIT_FAILURE);
				}
				if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, C[GMT_Y], GMT->current.io.col_type[GMT_IN][GMT_Y], &out[GMT_Y]), C[GMT_Y])) {
					GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not decode latitude from %s\n", C[GMT_Y]);
					Return (EXIT_FAILURE);
				}
				if (GMT->common.R.active && check_lon_lat (GMT, &out[GMT_X], &out[GMT_Y])) continue;
				pos = 0;
				if (get_z) {
					if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Z], GMT_scanf_arg (GMT, C[GMT_Z], GMT->current.io.col_type[GMT_IN][GMT_Z], &out[GMT_Z]), C[GMT_Z])) {
						GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not decode altitude from %s\n", C[GMT_Z]);
						Return (EXIT_FAILURE);
					}
					if (Ctrl->C.active) index = GMT_get_index (GMT, P, out[GMT_Z]);
					out[GMT_Z] = Ctrl->A.get_alt ? out[GMT_Z] * Ctrl->A.scale : Ctrl->A.altitude;
				}
				if (Ctrl->F.mode == EVENT) {
					if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][t1_col], GMT_scanf_arg (GMT, C[t1_col], GMT->current.io.col_type[GMT_IN][t1_col], &out[t1_col]), C[t1_col])) {
						GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not decode time event from %s\n", C[t1_col]);
						Return (EXIT_FAILURE);
					}
				}
				else if (Ctrl->F.mode == SPAN) {
					if (!(strcmp (C[t1_col], "NaN")))
						out[t1_col] = GMT->session.d_NaN;
					else if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][t1_col], GMT_scanf_arg (GMT, C[t1_col], GMT->current.io.col_type[GMT_IN][t1_col], &out[t1_col]), C[t1_col])) {
						GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not decode time span beginning from %s\n", C[t1_col]);
						Return (EXIT_FAILURE);
					}
					if (!(strcmp (C[t2_col], "NaN")))
						out[t2_col] = GMT->session.d_NaN;
					else if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][t2_col], GMT_scanf_arg (GMT, C[t2_col], GMT->current.io.col_type[GMT_IN][t2_col], &out[t2_col]), C[t2_col])) {
						GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not decode time span end from %s\n", C[t2_col]);
						Return (EXIT_FAILURE);
					}
				}
				if (Ctrl->F.mode < LINE && GMT_is_dnan (out[GMT_Z])) continue;	/* Symbols with NaN height are not plotted anyhow */

				if (Ctrl->F.mode < LINE) {	/* Print the information for this point */
					tabs (N++); printf ("<Placemark>\n");
					if (Ctrl->N.mode == NO_LABEL) { /* Nothing */ }
					else if (Ctrl->N.mode == GET_COL_LABEL) {
						GMT_strtok (extra, " \t,", &pos, item);
						tabs (N), printf ("<name>%s</name>\n", item);
					}
					else if (Ctrl->N.mode == GET_LABEL)
						tabs (N), printf ("<name>%s</name>\n", extra);
					else if (Ctrl->N.mode == FMT_LABEL) {
						tabs (N); printf ("<name>"); printf (Ctrl->N.fmt, pnt_nr); printf ("</name>\n");
					}
					else if (T->segment[seg]->label && T->segment[seg]->n_rows > 1)
						tabs (N), printf ("<name>%s %" PRIu64 "</name>\n", T->segment[seg]->label, row);
					else if (T->segment[seg]->label)
						tabs (N), printf ("<name>%s</name>\n", T->segment[seg]->label);
					else
						tabs (N), printf ("<name>%s %d</name>\n", name[Ctrl->F.mode], pnt_nr);
					if (Ctrl->L.n_cols) {
						tabs (N++); printf ("<ExtendedData>\n");
						for (col = 0; col < Ctrl->L.n_cols; col++) {
							tabs (N); printf ("<Data name = \"%s\">\n", Ctrl->L.name[col]);
							tabs (++N); printf ("<value>");
							GMT_strtok (extra, " \t,", &pos, item);
							printf ("%s", item);
							printf ("</value>\n");
							tabs (--N); printf ("</Data>\n");
						}
						tabs (--N); printf ("</ExtendedData>\n");
					}
					if (Ctrl->F.mode == SPAN) {
						tabs (N++); printf ("<TimeSpan>\n");
						if (!GMT_is_dnan (out[t1_col])) {
							tabs (N); printf ("<begin>");
							ascii_output_one (GMT, out[t1_col], t1_col);
							printf ("</begin>\n");
						}
						if (!GMT_is_dnan (out[t2_col])) {
							tabs (N); printf ("<end>");
							ascii_output_one (GMT, out[t2_col], t2_col);
							printf ("</end>\n");
						}
						tabs (--N); printf ("</TimeSpan>\n");
					}
					else if (Ctrl->F.mode == EVENT) {
						tabs (N++); printf ("<TimeStamp>\n");
						tabs (N); printf ("<when>");
						ascii_output_one (GMT, out[t1_col], t1_col);
						printf ("</when>\n");
						tabs (--N); printf ("</TimeStamp>\n");
					}
					tabs (N); printf ("<styleUrl>#st%d</styleUrl>\n", index + 4); /* +4 to make index a positive integer */
					tabs (N++); printf ("<%s>\n", feature[Ctrl->F.mode]);
					print_altmode (Ctrl->E.active, false, Ctrl->A.mode, N);
					tabs (N); printf ("<coordinates>");
					ascii_output_one (GMT, out[GMT_X], GMT_X);	printf (",");
					ascii_output_one (GMT, out[GMT_Y], GMT_Y);	printf (",");
					ascii_output_one (GMT, out[GMT_Z], GMT_Z);
					printf ("</coordinates>\n");
					tabs (--N); printf ("</%s>\n", feature[Ctrl->F.mode]);
					tabs (--N); printf ("</Placemark>\n");
				}
				else {	/* For lines and polygons we just output the coordinates */
					if (GMT_is_dnan (out[GMT_Z])) out[GMT_Z] = 0.0;	/* Google Earth can not handle lines at NaN altitude */
					tabs (N);
					ascii_output_one (GMT, out[GMT_X], GMT_X);	printf (",");
					ascii_output_one (GMT, out[GMT_Y], GMT_Y);	printf (",");
					ascii_output_one (GMT, out[GMT_Z], GMT_Z);	printf ("\n");
					if (row > 0 && no_dateline && crossed_dateline (out[GMT_X], last_x)) {
						/* GE cannot handle polygons crossing the dateline; warn for now */
						GMT_Report (API, GMT_MSG_NORMAL, "Warning: At least on polygon is straddling the Dateline.  Google Earth will wrap these the wrong way\n");
						GMT_Report (API, GMT_MSG_NORMAL, "Split such polygons into East and West parts and plot them as separate polygons.\n");
						GMT_Report (API, GMT_MSG_NORMAL, "Use gmtconvert to help in this conversion.\n");
						no_dateline = true;
					}
					last_x = out[GMT_X];
				}
				pnt_nr++;
			}

			/* End of segment */
			if (pnt_nr == 0)
				set_nr--;
			else if (Ctrl->F.mode < LINE)
				tabs (--N), printf ("</Folder>\n");
			else {
				tabs (--N); printf ("</coordinates>\n");
				if (Ctrl->F.mode == POLYGON) {
					tabs (--N); printf ("</LinearRing>\n");
					tabs (--N); printf ("</outerBoundaryIs>\n");
				}
				tabs (--N); printf ("</%s>\n", feature[Ctrl->F.mode]);
				tabs (--N); printf ("</Placemark>\n");
			}
			set_nr++;
		}
		if (T->file[GMT_IN]) tabs (--N), printf ("</Folder>\n");
	}
	if (use_folder) tabs (--N), printf ("</%s>\n", Document[KML_FOLDER]);
	if (!GMT->common.K.active) {
		tabs (--N), printf ("</%s>\n", Document[KML_DOCUMENT]);
		printf ("</kml>\n");
	}

	Return (GMT_OK);
}
