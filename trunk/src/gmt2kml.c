/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * API functions to support the gmt2kml application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: gmt2kml reads GMT tables and plots symbols, lines,
 * and polygons as KML files for Google Earth.
 */
 
#include "gmt.h"

EXTERN_MSC GMT_LONG gmt_parse_R_option (struct GMT_CTRL *C, char *item);
EXTERN_MSC void GMT_get_rgb_lookup (struct GMT_CTRL *C, struct GMT_PALETTE *P, GMT_LONG index, double value, double *rgb);

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

#define GET_LABEL		1
#define NO_LABEL		2
#define FMT_LABEL		3

/* Need unsigned int BGR triplets */
#define GMT_3u255(t) GMT_u255(t[2]),GMT_u255(t[1]),GMT_u255(t[0])

struct EXT_COL {
	int col;			/* Column in input record */
	char name[GMT_TEXT_LEN64];	/* Name of this column */
};

#define F_ID	0	/* Indices into arrays */
#define N_ID	1
#define ALT	0
#define LOD	1
#define FADE	2

struct GMT2KML_CTRL {
	double t_transp;
	struct A {	/* -A */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG get_alt;
		double scale;
		double altitude;
	} A;
	struct C {	/* -C<cpt> */
		GMT_LONG active;
		char *file;
	} C;
	struct D {	/* -D<descriptfile> */
		GMT_LONG active;
		char *file;
	} D;
	struct E {	/* -E */
		GMT_LONG active;
	} E;
	struct F {	/* -F */
		GMT_LONG active;
		GMT_LONG mode;
	} F;
	struct G {	/* -G<fill> */
		GMT_LONG active[2];
		struct GMT_FILL fill[2];
	} G;
	struct I {	/* -I<icon> */
		GMT_LONG active;
		char *file;
	} I;
	struct L {	/* -L */
		GMT_LONG active;
		GMT_LONG n_cols;
		struct EXT_COL *ext;
	} L;
	struct N {	/* -N */
		GMT_LONG active;
		GMT_LONG mode;
		char *fmt;
	} N;
	struct R2 {	/* -R */
		GMT_LONG active;
		GMT_LONG automatic;
	} R2;
	struct S {	/* -S */
		GMT_LONG active;
		double scale[2];
	} S;
	struct T {	/* -T */
		GMT_LONG active;
		char *title;
		char *folder;
	} T;
	struct W {	/* -W<pen> */
		GMT_LONG active;
		GMT_LONG mode;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z */
		GMT_LONG active;
		GMT_LONG invisible;
		GMT_LONG open;
		double min[3], max[3];
	} Z;
};

void *New_gmt2kml_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMT2KML_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMT2KML_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->A.mode = KML_GROUND;
	C->A.scale = 1.0;
	C->F.mode = POINT;
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
	if (C->C.file) free (C->C.file);
	if (C->D.file) free (C->D.file);
	if (C->I.file) free (C->I.file);
	if (C->N.fmt) free (C->N.fmt);
	if (C->T.title) free (C->T.title);
	if (C->T.folder) free (C->T.folder);
	GMT_free (GMT, C->L.ext);
	GMT_free (GMT, C);
}

GMT_LONG GMT_gmt2kml_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the gmt2kml synopsis and optionally full usage information */

	GMT_message (GMT, "gmt2kml %s [API] - Convert GMT data tables to KML files for Google Earth\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: gmt2kml [<table>] [-Aa|g|s[<altitude>|x<scale>]] [-C<cpt>] [-D<descriptfile>] [-E]\n");
	GMT_message (GMT, "\t[-Fe|s|t|l|p] [-Gf|n[-|<fill>] [-I<icon>] [-K] [-L<col:name>,<col:name>,...]\n");
	GMT_message (GMT, "\t[-N+|<template>|<name>] [-O] [-Q[e|s|t|l|p|n]<transp>] [-Ra|<w>/<e>/<s>/n>] [-Sc|n<scale>]\n");
	GMT_message (GMT, "\t[-T<title>[/<foldername>] [%s] [-W-|<pen>] [-Z<opts>]\n", GMT_V_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_bi_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-A Altitude mode, choose among three modes:\n");
	GMT_message (GMT,"\t      a Absolute altitude.\n");
	GMT_message (GMT,"\t      g Altitude relative to sea surface or ground.\n");
	GMT_message (GMT,"\t      s Altitude relative to seafloor or ground.\n");
	GMT_message (GMT,"\t    Optionally, append fixed <altitude>, or x<scale> [g0: Clamped to sea surface or ground].\n");
	GMT_message (GMT, "\t-C Append color palette name to color symbols by third column z-value.\n");
	GMT_message (GMT, "\t-D File with HTML snippets to use for data description [none].\n");
	GMT_message (GMT, "\t-E Extend feature down to the ground [no extrusion].\n");
	GMT_message (GMT, "\t-F Feature type; choose from (e)vent, (s)symbol, (t)imespan, (l)ine, or (p)olygon [s].\n");
	GMT_message (GMT, "\t   All features expect lon, lat in the first two columns.\n");
	GMT_message (GMT, "\t   Value or altitude is given in the third column (see -A and -C).\n");
	GMT_message (GMT, "\t   Event requires a timestamp in the next column.\n");
	GMT_message (GMT, "\t   Timespan requires begin and end timestamps in the next two columns\n");
	GMT_message (GMT, "\t   (use NaN for unlimited begin and/or end times).\n");
	GMT_rgb_syntax (GMT, 'G', "Set color for symbol/polygon fill (-Gf<color>) or label (-Gn<color>).");
	GMT_message (GMT, "\t   Default polygon fill is lightorange with 75%% transparency.\n");
	GMT_message (GMT, "\t   Default text label color is white.\n");
	GMT_message (GMT, "\t   Use -Gf- to turn off polygon fill.\n");
	GMT_message (GMT, "\t   Use -Gn- to turn off labels.\n");
	GMT_message (GMT, "\t-I URL to an alternative icon used for the symbol [Google circle].\n");
	GMT_message (GMT, "\t   If URL starts with + we will prepend http://maps.google.com/mapfiles/kml/.\n");
	GMT_message (GMT, "\t   Give -I- to not place any icons.\n");
	GMT_message (GMT, "\t   [Default is a local icon with no directory path].\n");
	GMT_message (GMT, "\t-K Allow for more KML code to be appended later [OFF].\n");
	GMT_message (GMT, "\t-L Supply extended data informat via <col>:<name> strings [none].\n");
	GMT_message (GMT, "\t-N Control the feature labels.\n");
	GMT_message (GMT, "\t   By default, -L\"label\" statements in the segment header are used. Alternatively,\n");
	GMT_message (GMT, "\t   1. Specify -N+ if the rest of the data record should be used as label (-Fe|s|t only).\n");
	GMT_message (GMT, "\t   2. Append a string that may contain the format %%d for a running feature count.\n");
	GMT_message (GMT, "\t   3. Give no argument to indicate no labels.\n");
	GMT_message (GMT, "\t-O Append the KML code to an existing document [OFF].\n");
	GMT_message (GMT, "\t-R Issue Region tag.  Append w/e/s/n to set a particular region or append a to use the\n");
	GMT_message (GMT, "\t   actual domain of the data (single file only) [no region specified].\n");
	GMT_message (GMT, "\t-S Scale for (c)ircle icon size or (n)ame label [1].\n");
	GMT_message (GMT, "\t-T Append KML document title name [GMT Data Document].\n");
	GMT_message (GMT, "\t   Optionally append /<foldername> to name folder when used with\n");
	GMT_message (GMT, "\t   -O and -K to organize features into groups.\n");
	GMT_explain_options (GMT, "V");
	GMT_pen_syntax (GMT, 'W', "Specify pen attributes for lines and polygon outlines [Default is %s].");
	GMT_message (GMT, "\t   Give width in pixels and append p.\n");
	GMT_message (GMT, "\t   A leading + applies cpt color (-C) to both polygon fill and outline.\n");
	GMT_message (GMT, "\t   A leading - applies cpt color (-C) to the outline only.\n");
	GMT_message (GMT, "\t-Z Control visibility of features.  Append one or more modifiers:\n");
	GMT_message (GMT, "\t   +a<alt_min>/<alt_max> inserts altitude limits [no limit].\n");
	GMT_message (GMT, "\t   +l<minLOD>/<maxLOD>] sets Level Of Detail when layer should be active [always active].\n");
	GMT_message (GMT, "\t     layer goes inactive when there are fewer than minLOD pixels or more\n");
	GMT_message (GMT, "\t     than maxLOD pixels visible.  -1 means never invisible.\n");
	GMT_message (GMT, "\t   +f<minfade>/<maxfade>] sets distances over which we fade from opaque.\n");
	GMT_message (GMT, "\t     to transparent [no fading].\n");
	GMT_message (GMT, "\t   +v turns off visibility [feature is visible].\n");
	GMT_message (GMT, "\t   +o open document or folder when loaded [closed].\n");
	GMT_explain_options (GMT, "C2fghi:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_gmt2kml_parse (struct GMTAPI_CTRL *C, struct GMT2KML_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmt2kml and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, pos = 0, k, n_files = 0, n_alloc = 0;
	char buffer[GMT_BUFSIZ], p[GMT_BUFSIZ], T[4][GMT_TEXT_LEN64], *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Altitude mode */
				Ctrl->A.active = TRUE;
				switch (opt->arg[1]) {
					case 'x':
						Ctrl->A.scale = atof (&opt->arg[2]);
						Ctrl->A.get_alt = TRUE;
					case '\0':
						Ctrl->A.get_alt = TRUE;
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
						GMT_message (GMT, "Bad altitude mode. Use a, g or s.\n");
						n_errors++;
						break;
				}
				break;
			case 'C':	/* Color table */
				Ctrl->C.active = TRUE;
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Description file */
				Ctrl->D.active = TRUE;
				if (opt->arg[0]) Ctrl->D.file = strdup (opt->arg);
				break;
			case 'E':	/* Extrude feature down to the ground*/
			 	Ctrl->E.active = TRUE;
				break;
			case 'F':	/* Feature type */
		 		Ctrl->F.active = TRUE;
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
						break;
					case 'p':
						Ctrl->F.mode = POLYGON;
						break;
					default:
						GMT_message (GMT, "Bad feature type. Use s, e, t, l or p.\n");
						n_errors++;
						break;
				}
				break;
			case 'G':	/* Set fill for symbols or polygon */
				switch (opt->arg[0]) {
					case 'f':	/* Symbol/polygon color fill */
						if (opt->arg[1] == '-')
				 			Ctrl->G.active[F_ID] = TRUE;
						else if (!opt->arg[1] || GMT_getfill (GMT, &opt->arg[1], &Ctrl->G.fill[F_ID])) {
							GMT_fill_syntax (GMT, 'G', "(-Gf or -Gn)");
							n_errors++;
						}
						break;
					case 'n':	/* Label name color */
		 				Ctrl->G.active[N_ID] = TRUE;
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
	 			Ctrl->I.active = TRUE;
				free (Ctrl->I.file);
				if (opt->arg[0] == '+')
					sprintf (buffer, "http://maps.google.com/mapfiles/kml/%s", &opt->arg[1]);
				else if (opt->arg[0])
					strcpy (buffer, opt->arg);
				Ctrl->I.file = strdup (buffer);
				break;
			case 'L':	/* Extended data */
 				Ctrl->L.active = TRUE;
				pos = Ctrl->L.n_cols = 0;
				while ((GMT_strtok (opt->arg, ",", &pos, p))) {
					for (k = 0; p[k] && p[k] != ':'; k++);	/* Find position of colon */
					p[k] = ' ';
					if (Ctrl->L.n_cols == n_alloc) Ctrl->L.ext = GMT_memory (GMT, Ctrl->L.ext, n_alloc += GMT_TINY_CHUNK, struct EXT_COL);
					sscanf (p, "%d %[^:]", &Ctrl->L.ext[Ctrl->L.n_cols].col, Ctrl->L.ext[Ctrl->L.n_cols].name);
					Ctrl->L.n_cols++;
				}
				break;
			case 'N':	/* Feature label */
				Ctrl->N.active = TRUE;
				if (opt->arg[0] == '+') {	/* Special ASCII labelled input file */
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
				Ctrl->R2.active = TRUE;
				if (opt->arg[0] == 'a')	/* Get args from data domain */
					Ctrl->R2.automatic = TRUE;
				else if (opt->arg[0])
					n_errors += gmt_parse_R_option (GMT, opt->arg);
				break;
			case 'S':	/* Scale for symbol or text */
				Ctrl->S.active = TRUE;
				if (opt->arg[0] == 'f')
					Ctrl->S.scale[F_ID] = atof (&opt->arg[1]);
				else if (opt->arg[0] == 'n')
					Ctrl->S.scale[N_ID] = atof (&opt->arg[1]);
				else {
					GMT_message (GMT, "-S requires f or n, then size\n");
					n_errors++;
				}
				break;
			case 'T':	/* Title [and folder] */
				Ctrl->T.active = TRUE;
				if ((c = strchr (opt->arg, '/'))) {	/* Got both title and folder */
					if (c[1]) Ctrl->T.folder = strdup (&c[1]);
					*c = '\0';
					if (opt->arg[0]) Ctrl->T.title = strdup (opt->arg);
					*c = '/';
				}
				else if (opt->arg[0]) Ctrl->T.title = strdup (opt->arg);
				break;
			case 'W':	/* Pen attributes */
				Ctrl->W.active = TRUE;
				k = 0;
				if (opt->arg[k] == '-') {Ctrl->W.mode = 1; k++;}
				if (opt->arg[k] == '+') {Ctrl->W.mode = 2; k++;}
				if (opt->arg[k] && GMT_getpen (GMT, &opt->arg[k], &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', "sets pen attributes [Default pen is %s]:");
					GMT_report (GMT, GMT_MSG_FATAL, "\t   A leading + applies cpt color (-C) to both symbol fill and pen.\n");
					GMT_report (GMT, GMT_MSG_FATAL, "\t   A leading - applies cpt color (-C) to the pen only.\n");
					n_errors++;
				}
				break;
			case 'Z':	/* Visibility control */
				Ctrl->Z.active = TRUE;
				pos = 0;
				while ((GMT_strtok (&opt->arg[1], "+", &pos, p))) {
				switch (p[0]) {
					case 'a':	/* Altitude range */
						if (sscanf (&p[1], "%[^/]/%s", T[0], T[1]) != 2) {
							GMT_message (GMT, "-Z+a requires 2 arguments\n");
							n_errors++;
						}
						Ctrl->Z.min[ALT] = atof (T[0]);	Ctrl->Z.max[ALT] = atof (T[1]);
						break;
					case 'l':	/* LOD */
						if (sscanf (&p[1], "%[^/]/%s", T[0], T[1]) != 2) {
							GMT_message (GMT, "-Z+l requires 2 arguments\n");
							n_errors++;
						}
						Ctrl->Z.min[LOD] = atof (T[0]);	Ctrl->Z.max[LOD] = atof (T[1]);
						break;
					case 'f':	/* Fading */
						if (sscanf (&p[1], "%[^/]/%s", T[0], T[1]) != 2) {
							GMT_message (GMT, "-Z+f requires 2 arguments\n");
							n_errors++;
						}
						Ctrl->Z.min[FADE] = atof (T[0]);	Ctrl->Z.max[FADE] = atof (T[1]);
						break;
					case 'v':
						Ctrl->Z.invisible = TRUE;
						break;
					case 'o':
						Ctrl->Z.open = TRUE;
						break;
					default:
						GMT_message (GMT, "-Z unrecognized modifier +%c\n", p[0]);
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
	n_errors += GMT_check_condition (GMT, Ctrl->R2.automatic && n_files > 1, "Syntax error: -Ra without arguments only accepted for single table\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.scale[F_ID] < 0.0 || Ctrl->S.scale[N_ID] < 0.0, "Syntax error: -S takes scales > 0.0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->t_transp < 0.0 || Ctrl->t_transp > 1.0, "Syntax error: -Q takes transparencies in range 0-1\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.mode == GET_LABEL && Ctrl->F.mode >= LINE, "Syntax error: -N+ not valid for lines and polygons\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->W.pen.width < 1.0, "Syntax error: -W given pen width < 1 pixel.  Use integers and append p as unit.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->W.mode && !Ctrl->C.active, "Syntax error: -W option +|-<pen> requires the -C option.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LONG check_lon_lat (struct GMT_CTRL *GMT, double *lon, double *lat)
{
	if (*lat < GMT->common.R.wesn[YLO] || *lat > GMT->common.R.wesn[YHI]) return (TRUE);
	if (*lon < GMT->common.R.wesn[XLO]) *lon += 360.0;
	if (*lon > GMT->common.R.wesn[XHI]) *lon -= 360.0;
	if (*lon < GMT->common.R.wesn[XLO]) return (TRUE);
	return (FALSE);
}

void tabs (GMT_LONG ntabs)
{	/* Outputs the n leading tabs for this KML line */
	while (ntabs--) putchar ('\t');
}

void print_altmode (GMT_LONG extrude, GMT_LONG fmode, GMT_LONG altmode, GMT_LONG ntabs)
{
	char *RefLevel[5] = {"clampToGround", "relativeToGround", "absolute", "relativeToSeaFloor", "clampToSeaFloor"};
	if (extrude) tabs (ntabs), printf ("<extrude>1</extrude>\n"); 
	if (fmode) tabs (ntabs), printf ("<tesselate>1</tesselate>\n");
	if (altmode == KML_GROUND_REL || altmode == KML_ABSOLUTE) tabs (ntabs), printf ("<altitudeMode>%s</altitudeMode>\n", RefLevel[altmode]);
	if (altmode == KML_SEAFLOOR_REL || altmode == KML_SEAFLOOR) tabs (ntabs), printf ("<gx:altitudeMode>%s</gx:altitudeMode>\n", RefLevel[altmode]);
}

GMT_LONG ascii_output_one (struct GMT_CTRL *GMT, double x, GMT_LONG col)
{	/* Used instead of GMT_ascii_output_col since Windoze has trouble with GMT->session.std[GMT_OUT] and stdout mixing */
	char text[GMT_TEXT_LEN256];

	GMT_ascii_format_col (GMT, text, x, col);
	return (printf ("%s", text));
}

void place_region_tag (struct GMT_CTRL *GMT, double west, double east, double south, double north, double min[], double max[], GMT_LONG N)
{
	if (GMT_360_RANGE (west, east)) { west = -180.0; east = +180.0;}
	tabs (N++); printf ("<Region>\n");
	tabs (N++); printf ("<LatLonAltBox>\n");
	tabs (N);	printf ("<north>");	ascii_output_one (GMT, north, GMT_Y);	printf ("</north>\n");
	tabs (N);	printf ("<south>");	ascii_output_one (GMT, south, GMT_Y);	printf ("</south>\n");
	tabs (N);	printf ("<east>");	ascii_output_one (GMT, east, GMT_X);	printf ("</east>\n");
	tabs (N);	printf ("<west>");	ascii_output_one (GMT, west, GMT_X);	printf ("</west>\n");
	if (max[ALT] > min[ALT]) {
		tabs (N); printf ("<minAltitude>%g</minAltitude>\n", min[ALT]);
		tabs (N); printf ("<maxAltitude>%g</maxAltitude>\n", max[ALT]);
	}
	tabs (--N); printf ("</LatLonAltBox>\n");
	if (max[LOD] != min[LOD]) {
		tabs (N++); printf ("<Lod>\n");
		tabs (N); printf ("<minLodPixels>%d</minLodPixels>\n", irint (min[LOD]));
		tabs (N); printf ("<maxLodPixels>%d</maxLodPixels>\n", irint (max[LOD]));
		if (min[FADE] > 0.0 || max[FADE] > 0.0) {
			tabs (N); printf ("<minFadeExtent>%g</minFadeExtent>\n", min[FADE]);
			tabs (N); printf ("<maxFadeExtent>%g</maxFadeExtent>\n", max[FADE]);
		}
		tabs (--N); printf ("</Lod>\n");
	}
	tabs (--N); printf ("</Region>\n");
}

void set_iconstyle (double *rgb, double scale, char *iconfile, GMT_LONG N)
{	/* No icon = no symbol */
	tabs (N++); printf ("<IconStyle>\n");
	tabs (N++); printf ("<Icon>\n");
	if (iconfile[0] != '-') { tabs (N); printf ("<href>%s</href>\n", iconfile); }
	tabs (--N); printf ("</Icon>\n");
	if (iconfile[0] != '-') {
		tabs (N); printf ("<scale>%g</scale>\n", scale);
		tabs (N); printf ("<color>%2.2x%2.2x%2.2x%2.2x</color>\n", GMT_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	}
	tabs (--N); printf ("</IconStyle>\n");
}

void set_linestyle (struct GMT_PEN *pen, double *rgb, GMT_LONG N)
{
	tabs (N++); printf ("<LineStyle>\n");
	tabs (N); printf ("<color>%2.2x%2.2x%2.2x%2.2x</color>\n", GMT_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	tabs (N); printf ("<width>%d</width>\n", irint (pen->width));
	tabs (--N); printf ("</LineStyle>\n");
}

void set_polystyle (double *rgb, GMT_LONG outline, GMT_LONG active, GMT_LONG N)
{
	tabs (N++); printf ("<PolyStyle>\n");
	tabs (N); printf ("<color>%2.2x%2.2x%2.2x%2.2x</color>\n", GMT_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	tabs (N); printf ("<fill>%d</fill>\n", !active);
	tabs (N); printf ("<outline>%ld</outline>\n", outline);
	tabs (--N); printf ("</PolyStyle>\n");
}

void get_rgb_lookup (struct GMT_CTRL *C, struct GMT_PALETTE *P, GMT_LONG index, double *rgb)
{	/* Special version of GMT_get_rgb_lookup since no interpolation can take place */

	if (index < 0) {	/* NaN, Foreground, Background */
		GMT_rgb_copy (rgb, P->patch[index+3].rgb);
		P->skip = P->patch[index+3].skip;
	}
	else if (P->range[index].skip) {		/* Set to page color for now */
		GMT_rgb_copy (rgb, C->current.setting.ps_page_rgb);
		P->skip = TRUE;
	}
	else {	/* Return low color */
		GMT_memcpy (rgb, P->range[index].rgb_low, 4, double);
		P->skip = FALSE;
	}
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmt2kml_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_gmt2kml (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG k, n_coord = 0, first = TRUE, get_z = FALSE, t1_col, t2_col,error = FALSE;
	GMT_LONG set_nr = 0, pnt_nr = 0, index = -4, use_folder = FALSE, do_description, N = 1;
	
	char buffer[GMT_BUFSIZ], description[GMT_BUFSIZ], *Document[2] = {"Document", "Folder"};
	char *feature[5] = {"Point", "Point", "Point", "LineString", "Polygon"};
	char label[GMT_BUFSIZ], *name[5] = {"Point", "Event", "Timespan", "Line", "Polygon"};

	double rgb[4], out[5];

	struct GMT_OPTION *options = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT2KML_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_gmt2kml_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_gmt2kml_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gmt2kml", &GMT_cpy);		/* Save current state */
	if (GMT_Parse_Common (API, "-Vbf:", "ghiOK>" GMT_OPT("HMm"), options)) Return (API->error);
	Ctrl = New_gmt2kml_Ctrl (GMT);		/* Allocate and initialize a new control structure */
	if ((error = GMT_gmt2kml_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmt2kml main code ----------------------------*/

	/* gmt2kml only applies to geographic data so we do a -fg implicitly here */
	GMT->current.io.col_type[GMT_IN][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
	GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
	label[0] = '\0';
	
	if (Ctrl->C.active) {	/* Process CPT file */
		if ((P = GMT_Read_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (P->is_continuous) {
			GMT_message (GMT, "Cannot use continuous color palette\n");
			Return (EXIT_FAILURE);
		}
	}

	/* Now we are ready to take on some input values */

	out[GMT_Z] = Ctrl->A.altitude;
	strcpy (GMT->current.setting.io_col_separator, ",");	/* Specify comma-separated output */
	GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;		/* Want -180/+180 longitude output format */
	strcpy (GMT->current.setting.format_float_out, "%.12g");	/* Make sure we use enough decimals */
	n_coord = (Ctrl->F.mode < LINE) ? Ctrl->F.mode + 2 : 2;
	get_z = (Ctrl->C.active || Ctrl->A.get_alt);
	if (get_z) n_coord++;
	t1_col = 2 + get_z;
	t2_col = 3 + get_z;
	if (Ctrl->F.mode == EVENT || Ctrl->F.mode == SPAN) GMT->current.io.col_type[GMT_IN][t1_col] = GMT->current.io.col_type[GMT_OUT][t1_col] = GMT_IS_ABSTIME;
	if (Ctrl->F.mode == SPAN)  GMT->current.io.col_type[GMT_IN][t2_col] = GMT->current.io.col_type[GMT_OUT][t2_col] = GMT_IS_ABSTIME;

	if (!Ctrl->T.title) Ctrl->T.title = strdup ("GMT Data Document");
	if (!Ctrl->T.folder) {
		sprintf (buffer, "%s Features", name[Ctrl->F.mode]);
		Ctrl->T.folder = strdup (buffer);
	}
	if (GMT->common.O.active || GMT->common.K.active) use_folder = TRUE;	/* When at least one or -O, -K is used */
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
		tabs (N); printf ("<name>%s</name>\n", Ctrl->T.title);
		if (Ctrl->Z.invisible) tabs (N), printf ("<visibility>0</visibility>\n");
		if (Ctrl->Z.open) tabs (N), printf ("<open>1</open>\n");
		if (use_folder) {
			tabs (N++); printf ("<%s>\n", Document[KML_FOLDER]);
			tabs (N); printf ("<name>%s</name>\n", Ctrl->T.folder);
		}
	}
		
	tabs (N++); printf ("<Style id=\"GMT%ld\">\n", index);	/* Default style unless -C is used */

	/* Set icon style (applies to symbols only */
	set_iconstyle (Ctrl->G.fill[F_ID].rgb, Ctrl->S.scale[F_ID], Ctrl->I.file, N);

	/* Set shared line and polygon style (also for extrusions) */
	set_linestyle (&Ctrl->W.pen, Ctrl->W.pen.rgb, N);
	set_polystyle (Ctrl->G.fill[F_ID].rgb, !Ctrl->W.mode, Ctrl->G.active[F_ID], N);

	/* Set style for labels */
	tabs (N++); printf ("<LabelStyle>\n");
	tabs (N); printf ("<scale>%g</scale>\n", Ctrl->S.scale[N_ID]);
	tabs (N); printf ("<color>%2.2x%2.2x%2.2x%2.2x</color>\n", GMT_u255 (1.0 - Ctrl->G.fill[N_ID].rgb[3]), GMT_3u255 (Ctrl->G.fill[N_ID].rgb));
	tabs (--N); printf ("</LabelStyle>\n");
	tabs (--N); printf ("</Style>\n");

	for (k = -3; Ctrl->C.active && k < P->n_colors; k++) {	/* Place styles for each color in CPT file */
		get_rgb_lookup (GMT, P, k, rgb);
		tabs (N++); printf ("<Style id=\"GMT%ld\">\n", k);
		if (Ctrl->F.mode < LINE)	/* Set icon style (applies to symbols only */
			set_iconstyle (Ctrl->G.fill[F_ID].rgb, Ctrl->S.scale[F_ID], Ctrl->I.file, N);
		else if (Ctrl->F.mode == LINE)	/* Line style only */
			set_linestyle (&Ctrl->W.pen, rgb, N);
		else {	/* Polygons */
			if (!Ctrl->W.active) {	/* Only fill, no outline */
				set_polystyle (rgb, FALSE, Ctrl->G.active[F_ID], N);
			}
			else if (Ctrl->W.mode == 0) { /* Use -C for fill, -W for outline */
				set_polystyle (rgb, TRUE, Ctrl->G.active[F_ID], N);
				set_linestyle (&Ctrl->W.pen, Ctrl->W.pen.rgb, N);
			}
			else if (Ctrl->W.mode == 1) { /* Use -G for fill, -C for outline */
				set_polystyle (Ctrl->G.fill[F_ID].rgb, TRUE, Ctrl->G.active[F_ID], N);
				set_linestyle (&Ctrl->W.pen, rgb, N);
			}
			else if (Ctrl->W.mode == 2) { /* Use -C for fill and outline */
				set_polystyle (rgb, TRUE, Ctrl->G.active[F_ID], N);
				set_linestyle (&Ctrl->W.pen, rgb, N);
			}
		}
		tabs (--N); printf ("</Style>\n");
	}
	if (Ctrl->D.active) {	/* Add in a description HTML snipped */
		char line[GMT_BUFSIZ];
		FILE *fp = NULL;
		if ((fp = GMT_fopen (GMT, Ctrl->D.file, "r")) == NULL) {
			GMT_message (GMT, "Could not open description file %s\n", Ctrl->D.file);
			Return (EXIT_FAILURE);
		}
		tabs (N++); printf ("<description>\n");
		tabs (N++); printf ("<![CDATA[\n");
		while (GMT_fgets (GMT, line, GMT_BUFSIZ, fp)) tabs (N), printf ("%s", line);
		GMT_fclose (GMT, fp);
		tabs (--N); printf ("]]>\n");
		tabs (--N); printf ("</description>\n");
	}

	if (Ctrl->N.mode == GET_LABEL) { /* Special ASCII table processing */
		GMT_LONG ix, iy, n_rec = 0;
		char *record = NULL, C[5][GMT_TEXT_LEN64];

		ix = GMT->current.setting.io_lonlat_toggle[GMT_IN];	iy = 1 - ix;
		if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_TEXT, GMT_IN, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Establishes data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_IN) != GMT_OK) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		
		do {	/* Keep returning records until we reach EOF */
			if ((record = GMT_Get_Record (API, GMT_READ_TEXT, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all table and segment headers */
					continue;
				if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
					break;
			}

			switch (n_coord) {
				case 2:	/* Just lon, lat, label */
					sscanf (record, "%s %s %[^\n]", C[ix], C[iy], label);
					break;
				case 3:	/* Just lon, lat, a, label */
					sscanf (record, "%s %s %s %[^\n]", C[ix], C[iy], C[2], label);
					break;
				case 4:	/* Just lon, lat, a, b, label */
					sscanf (record, "%s %s %s %s %[^\n]", C[ix], C[iy], C[2], C[3], label);
					break;
				case 5:	/* Just lon, lat, z, t1, t2, label */
					sscanf (record, "%s %s %s %s %s %[^\n]", C[ix], C[iy], C[2], C[3], C[4], label);
					break;
			}
			if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, C[GMT_X], GMT->current.io.col_type[GMT_IN][GMT_X], &out[GMT_X]), C[GMT_X])) {
				GMT_message (GMT, "Error: Could not decode longitude from %s\n", C[GMT_X]);
				Return (EXIT_FAILURE);
			}
			if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, C[GMT_Y], GMT->current.io.col_type[GMT_IN][GMT_Y], &out[GMT_Y]), C[GMT_Y])) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: Could not decode latitude from %s\n", C[GMT_Y]);
				Return (EXIT_FAILURE);
			}
			if (GMT->common.R.active && check_lon_lat (GMT, &out[GMT_X], &out[GMT_Y])) continue;
			if (get_z) {
				if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Z], GMT_scanf_arg (GMT, C[GMT_Z], GMT->current.io.col_type[GMT_IN][GMT_Z], &out[GMT_Z]), C[GMT_Z])) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error: Could not decode altitude from %s\n", C[GMT_Z]);
					Return (EXIT_FAILURE);
				}
				if (Ctrl->C.active) index = GMT_get_index (GMT, P, out[GMT_Z]);
				out[GMT_Z] = Ctrl->A.get_alt ? out[GMT_Z] * Ctrl->A.scale : Ctrl->A.altitude;
			}
			if (Ctrl->F.mode == EVENT) {
				if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][t1_col], GMT_scanf_arg (GMT, C[t1_col], GMT->current.io.col_type[GMT_IN][t1_col], &out[t1_col]), C[t1_col])) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error: Could not decode time event from %s\n", C[t1_col]);
					Return (EXIT_FAILURE);
				}
			}
			else if (Ctrl->F.mode == SPAN) {
				if (!(strcmp (C[t1_col], "NaN")))
					out[t1_col] = GMT->session.d_NaN;
				else if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][t1_col], GMT_scanf_arg (GMT, C[t1_col], GMT->current.io.col_type[GMT_IN][t1_col], &out[t1_col]), C[t1_col])) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error: Could not decode time span beginning from %s\n", C[t1_col]);
					Return (EXIT_FAILURE);
				}
				if (!(strcmp (C[t2_col], "NaN")))
					out[t2_col] = GMT->session.d_NaN;
				else if (GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][t2_col], GMT_scanf_arg (GMT, C[t2_col], GMT->current.io.col_type[GMT_IN][t2_col], &out[t2_col]), C[t2_col])) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error: Could not decode time span end from %s\n", C[t2_col]);
					Return (EXIT_FAILURE);
				}
			}
			if (GMT->common.R.active && first) {	/* Issue Region tag as given on commmand line*/
				place_region_tag (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI], Ctrl->Z.min, Ctrl->Z.max, N);
				first = FALSE;
			}
			tabs (N++); printf ("<Placemark>\n");
			tabs (N); printf ("<name>%s</name>\n", label);
			if (Ctrl->F.mode == SPAN) {
				tabs (N++); printf ("<TimeSpan>\n");
				if (!GMT_is_dnan(out[t1_col])) tabs (N), printf ("<begin>%s</begin>\n", C[t1_col]);
				if (!GMT_is_dnan(out[t2_col])) tabs (N), printf ("<end>%s</end>\n", C[t2_col]);
				tabs (--N); printf ("</TimeSpan>\n");
			}
			else if (Ctrl->F.mode == EVENT) {
				tabs (N++); printf ("<TimeStamp>\n");
				tabs (N); printf ("<when>%s</when>\n", C[t1_col]);
				tabs (--N); printf ("</TimeStamp>\n");
			}
			tabs (N); printf ("<styleUrl>#GMT%ld</styleUrl>\n", index);
			tabs (N++); printf ("<%s>\n", feature[Ctrl->F.mode]);
			print_altmode (Ctrl->E.active, FALSE, Ctrl->A.mode, N);
			tabs (N); printf ("<coordinates>");
			ascii_output_one (GMT, out[GMT_X], GMT_X);	printf (",");
			ascii_output_one (GMT, out[GMT_Y], GMT_Y);	printf (",");
			ascii_output_one (GMT, out[GMT_Z], GMT_Z);
			printf ("</coordinates>\n");
			tabs (--N); printf ("</%s>\n", feature[Ctrl->F.mode]);
			tabs (--N); printf ("</Placemark>\n");
			n_rec++;
			if (!(n_rec%10000)) GMT_report (GMT, GMT_MSG_NORMAL, "Processed %ld points\n", n_rec);
		} while (TRUE);
		
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
	}
	else {	/* Read regular data table */
		GMT_LONG tbl, seg, row;
		struct GMT_DATASET *Din = NULL;
		struct GMT_TABLE *T = NULL;
#ifdef GMT_COMPAT
		char *t_opt = "-D";
#else
		char *t_opt = "-T";
#endif
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Establishes data input */
			Return (API->error);
		}
		if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, NULL)) == NULL) {
			Return (API->error);
		}
		if (GMT->common.R.active && first) {	/* Issue Region tag as given on commmand line*/
			place_region_tag (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI], Ctrl->Z.min, Ctrl->Z.max, N);
			first = FALSE;
		}
		else if (Ctrl->R2.automatic) {	/* Issue Region tag */
			place_region_tag (GMT, Din->min[GMT_X], Din->max[GMT_X], Din->min[GMT_Y], Din->max[GMT_Y], Ctrl->Z.min, Ctrl->Z.max, N);
		}
		set_nr = pnt_nr = 0;

		for (tbl = 0; tbl < Din->n_tables; tbl++) {
			T = Din->table[tbl];	/* Current table */
			if (T->file[GMT_IN]) {	/* Place all of this file's content in its own named folder */
				tabs (N++); printf ("<Folder>\n");
				tabs (N); printf ("<name>%s</name>\n", T->file[GMT_IN]);
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
						printf ("<name>%s Set %ld</name>\n", name[Ctrl->F.mode], set_nr);
					if (GMT_parse_segment_item (GMT, T->segment[seg]->header, t_opt, description)) { tabs (N); printf ("<description>%s</description>\n", description); }
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
						tabs (N), printf ("<name>%s %ld</name>\n", name[Ctrl->F.mode], set_nr);
					description[0] = 0;
					do_description = FALSE;
					if (GMT_parse_segment_item (GMT, T->segment[seg]->header, "-I", buffer)) { 
						do_description = TRUE;
						strcat (description, buffer);
					}
					if (GMT_parse_segment_item (GMT, T->segment[seg]->header, t_opt, buffer)) { 
						if (do_description) strcat (description, " ");
						strcat (description, buffer);
						do_description = TRUE;
					}
					if (do_description) { tabs (N); printf ("<description>%s</description>\n", description); }
					tabs (N); printf ("<styleUrl>#GMT%ld</styleUrl>\n", index);
					tabs (N++); printf ("<%s>\n", feature[Ctrl->F.mode]);
					print_altmode (Ctrl->E.active, Ctrl->F.mode, Ctrl->A.mode, N);
					if (Ctrl->F.mode == POLYGON) {
						tabs (N++); printf ("<outerBoundaryIs>\n");
						tabs (N++); printf ("<LinearRing>\n");
						if (T->segment[seg]->min[GMT_X] < 180.0 && T->segment[seg]->max[GMT_X] > 180.0) {
							/* GE cannot handle polygons crossing the dateline; warn for now */
							GMT_report (GMT, GMT_MSG_FATAL, "Warning: A polygon is straddling the Dateline.  Google Earth will wrap this the wrong way\n");
							GMT_report (GMT, GMT_MSG_FATAL, "Split the polygon into an East and West part and plot them as separate polygons.\n");
							GMT_report (GMT, GMT_MSG_FATAL, "gmtconvert can be used to help in this conversion.\n");
						}
					}
					tabs (N++); printf ("<coordinates>\n");
				}
				for (row = 0; row < T->segment[seg]->n_rows; row++) {
					out[GMT_X] = T->segment[seg]->coord[GMT_X][row];
					out[GMT_Y] = T->segment[seg]->coord[GMT_Y][row];
					if (GMT->common.R.active && check_lon_lat (GMT, &out[GMT_X], &out[GMT_Y])) continue;
					if (get_z && T->n_columns > 2) {
						out[GMT_Z] = T->segment[seg]->coord[GMT_Z][row];
						if (Ctrl->C.active) index = GMT_get_index (GMT, P, out[GMT_Z]);
						out[GMT_Z] = Ctrl->A.get_alt ? out[GMT_Z] * Ctrl->A.scale : Ctrl->A.altitude;
					}
					if (Ctrl->F.mode < LINE && GMT_is_dnan (out[GMT_Z])) continue;	/* Symbols with NaN height are not plotted anyhow */

					if (Ctrl->F.mode < LINE) {	/* Print the information for this point */
						tabs (N++); printf ("<Placemark>\n");
						if (Ctrl->N.mode == NO_LABEL) { /* Nothing */ }
						else if (Ctrl->N.mode == FMT_LABEL) {
							tabs (N); printf ("<name>"); printf (Ctrl->N.fmt, (int)pnt_nr); printf ("</name>\n");
						}
						else if (T->segment[seg]->label && T->segment[seg]->n_rows > 1)
							tabs (N), printf ("<name>%s %ld</name>\n", T->segment[seg]->label, row);
						else if (T->segment[seg]->label)
							tabs (N), printf ("<name>%s</name>\n", T->segment[seg]->label);
						else
							tabs (N), printf ("<name>%s %ld</name>\n", name[Ctrl->F.mode], pnt_nr);
						if (Ctrl->L.n_cols) {
							tabs (N++); printf ("<ExtendedData>\n");
							for (k = 0; k < Ctrl->L.n_cols; k++) {
								tabs (N++); printf ("<Data name = \"%s\">\n", Ctrl->L.ext[k].name);
								tabs (N--); printf ("<value>");
								ascii_output_one (GMT, T->segment[seg]->coord[Ctrl->L.ext[k].col][row], Ctrl->L.ext[k].col);
								printf ("</value>\n");
								tabs (N--); printf ("</Data>\n");
							}
							tabs (N); printf ("</ExtendedData>\n");
						}
						if (Ctrl->F.mode == SPAN) {
							tabs (N++); printf ("<TimeSpan>\n");
							if (!GMT_is_dnan(T->segment[seg]->coord[t1_col][row])) {
								tabs (N); printf ("<begin>");
								ascii_output_one (GMT, T->segment[seg]->coord[t1_col][row], t1_col);
								printf ("</begin>\n");
							}
							if (!GMT_is_dnan(T->segment[seg]->coord[t2_col][row])) {
								tabs (N); printf ("<end>");
								ascii_output_one (GMT, T->segment[seg]->coord[t2_col][row], t2_col);
								printf ("</end>\n");
							}
							tabs (--N); printf ("</TimeSpan>\n");
						}
						else if (Ctrl->F.mode == EVENT) {
							tabs (N++); printf ("<TimeStamp>\n");
							tabs (N); printf ("<when>");
							ascii_output_one (GMT, T->segment[seg]->coord[t1_col][row], t1_col);
							printf ("</when>\n");
							tabs (--N); printf ("</TimeStamp>\n");
						}
						tabs (N); printf ("<styleUrl>#GMT%ld</styleUrl>\n", index);
						tabs (N++); printf ("<%s>\n", feature[Ctrl->F.mode]);
						print_altmode (Ctrl->E.active, FALSE, Ctrl->A.mode, N);
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
	}
	if (use_folder) tabs (--N), printf ("</%s>\n", Document[KML_FOLDER]);
	if (!GMT->common.K.active) {
		tabs (--N), printf ("</%s>\n", Document[KML_DOCUMENT]);
		printf ("</kml>\n");
	}

	Return (GMT_OK);
}
