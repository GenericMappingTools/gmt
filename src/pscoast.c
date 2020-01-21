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
 * pscoast can draw shorelines, paint the landward and/or seaward side of
 * the shoreline, paint lakes, draw political borders, and rivers.  Three
 * data sets are considered: shores, borders, and rivers.  Each of these
 * data sets come in 5 different resolutions.  The lower resolution files
 * are derived from the full resolution data using the Douglas-Peucker (DP)
 * line reduction algorithm.  By giving a tolerance in km, the algorithm
 * will remove points that do not depart more than the tolerance from the
 * straight line that would connect the points if the point in question was
 * removed.  The resolutions are:
 *
 * full		- The complete World Vector Shoreline + CIA data base
 * high		- DP reduced with tolerance = 0.2 km
 * intermediate	- DP reduced with tolerance = 1 km
 * low		- DP reduced with tolerance = 5 km
 * crude	- DP reduced with tolerance = 25 km
 *
 * If selected, pscoast will open the desired binned shoreline file and fill
 * the landmasses and/or oceans/lakes as shaded/colored/textured polygons. The
 * shoreline may also be drawn.  Political boundaries and rivers may be plotted,
 * too.  If only land is filled, then the 'wet' areas remain transparent.
 * If only oceans are filled, then the 'dry' areas remain transparent.  This
 * allows the user to overlay land or ocean without wiping out the plot.
 * For more information about the binned polygon file, see the GMT Technical
 * Reference and Cookbook.
 * Optionally, the user may choose to issue clip paths rather than paint the
 * polygons.  That way one may clip subsequent images to be visible only
 * inside or outside the coastline.
 *
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"pscoast"
#define THIS_MODULE_MODERN_NAME	"coast"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot continents, countries, shorelines, rivers, and borders"
#define THIS_MODULE_KEYS	">?}"
#define THIS_MODULE_NEEDS	"JR"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYbdptxy" GMT_OPT("Zc")

#define LAKE	0
#define RIVER	1

#define NOT_REALLY_AN_ERROR -999

/* Control structure for pscoast */

struct PSCOAST_CTRL {
	struct A {	/* -A<min_area>[/<min_level>/<max_level>][+ag|i|s][+r|l][+p<percent>] */
		bool active;
		struct GMT_SHORE_SELECT info;
	} A;
	struct C {	/* -C<fill>[+l|r] */
		bool active;
		struct GMT_FILL fill[2];	/* lake and riverlake fill */
	} C;
	struct D {	/* -D<resolution>[+f] */
		bool active;
		bool force;	/* if true, select next highest level if current set is not available */
		char set;	/* One of f, h, i, l, c, or a for auto */
	} D;
	struct E {	/* -E<DWC-options> */
		bool active;
		struct GMT_DCW_SELECT info;
	} E;
	struct F {	/* -F[l|t][+c<clearance>][+g<fill>][+i[<off>/][<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;
		/* The panels are members of GMT_MAP_SCALE and GMT_MAP_ROSE */
	} F;
	struct G {	/* -G[<fill>] */
		bool active;
		bool clip;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I<feature>[/<pen>] */
		bool active;
		unsigned int use[GSHHS_N_RLEVELS];
		unsigned int list[GSHHS_N_RLEVELS];
		unsigned int n_rlevels;
		struct GMT_PEN pen[GSHHS_N_RLEVELS];
	} I;
	struct L {	/* -L[g|j|n|x]<refpoint>+c[<slon>/]<slat>+w<length>[e|f|M|n|k|u][+a<align>][+f][+l[<label>]][+u] */
		bool active;
		struct GMT_MAP_SCALE scale;
	} L;
	struct M {	/* -M */
		bool active;
		bool single;
	} M;
	struct N {	/* -N<feature>[/<pen>] */
		bool active;
		unsigned int use[GSHHS_N_BLEVELS];
		unsigned int list[GSHHS_N_BLEVELS];
		unsigned int n_blevels;
		struct GMT_PEN pen[GSHHS_N_BLEVELS];
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
	struct S {	/* -S[<fill>] */
		bool active;
		bool clip;
		struct GMT_FILL fill;
	} S;
	struct T {	/* -Td|m<params> */
		bool active;
		struct GMT_MAP_ROSE rose;
	} T;
	struct W {	/* -W[<feature>/]<pen> */
		bool active;
		bool use[GSHHS_MAX_LEVEL];
		struct GMT_PEN pen[GSHHS_MAX_LEVEL];
	} W;
#ifdef DEBUG
	struct DBG {	/* -+<bin> */
		bool active;
		int bin;
	} debug;
#endif
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	unsigned int k;
	struct PSCOAST_CTRL *C = gmt_M_memory (GMT, NULL, 1, struct PSCOAST_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.info.high = GSHHS_MAX_LEVEL;			/* Include all GSHHS levels */
	C->D.set = 'l';						/* Low-resolution coastline data */
	if (GMT->current.map.frame.paint)	/* Default Ocean color = Frame background color */
		C->S.fill = GMT->current.map.frame.fill;
	else
		gmt_init_fill (GMT, &C->S.fill, 1.0, 1.0, 1.0);		/* Default Ocean color = white */
	C->C.fill[LAKE] = C->C.fill[RIVER] = C->S.fill;		/* Default Lake/Riverlake color = Ocean color */
	gmt_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);		/* Default Land color = black */
	for (k = 0; k < GSHHS_N_RLEVELS; k++) C->I.pen[k] = GMT->current.setting.map_default_pen;		/* Default river pens */
	for (k = 0; k < GSHHS_N_BLEVELS; k++) C->N.pen[k] = GMT->current.setting.map_default_pen;		/* Default border pens */
	for (k = 0; k < GSHHS_MAX_LEVEL; k++) C->W.pen[k] = GMT->current.setting.map_default_pen;	/* Default coastline pens */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSCOAST_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_DCW_free (GMT, &(C->E.info));
	if (C->L.scale.refpoint) gmt_free_refpoint (GMT, &C->L.scale.refpoint);
	gmt_M_free (GMT, C->L.scale.panel);
	if (C->T.rose.refpoint) gmt_free_refpoint (GMT, &C->T.rose.refpoint);
	gmt_M_free (GMT, C->T.rose.panel);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the pscoast synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s %s [%s] [%s]\n", name, GMT_J_OPT, GMT_A_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-C<fill>[+l|r]]\n\t[-D<resolution>][+f] [-E%s][-G[<fill>]]\n", GMT_Rgeoz_OPT, DCW_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-F%s]\n", GMT_PANEL);
	GMT_Message (API, GMT_TIME_NONE, "\t[-I<feature>[/<pen>]] %s\n", API->K_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-L%s]\n", GMT_SCALE);
	GMT_Message (API, GMT_TIME_NONE, "\t[-M] [-N<feature>[/<pen>]] %s%s[-Q] [-S[<fill>]]\n", API->O_OPT, API->P_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Td%s]\n\t[-Tm%s]\n", GMT_TROSE_DIR, GMT_TROSE_MAG);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-W[<feature>/][<pen>]]\n", GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] %s[%s]\n", GMT_X_OPT, GMT_Y_OPT, GMT_bo_OPT, API->c_OPT, GMT_do_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n", GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT, GMT_PAR_OPT);
#ifdef DEBUG
	GMT_Message (API, GMT_TIME_NONE, "\t[-+<bin>]\n");
#endif
	GMT_Message (API, GMT_TIME_NONE, "\n");

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "J-Z,G");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	gmt_GSHHG_syntax (API->GMT, 'A');
	GMT_Option (API, "B-");
	gmt_fill_syntax (API->GMT, 'C', NULL, "Set separate color for lakes and riverlakes [Default is same as ocean].");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, set custom fills below.  Repeat the -C option as needed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Append +l to set lake fill.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Append +r to set river-lake fill.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Choose one of the following resolutions:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a - auto: select best resolution given map scale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f - full resolution (may be very slow for large regions).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   h - high resolution (may be slow for large regions).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i - intermediate resolution.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   l - low resolution [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c - crude resolution, for busy plots that need crude continent outlines only.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +f to use a lower resolution should the chosen one not be available [abort].\n");
	gmt_DCW_option (API, 'E', 1U);
	gmt_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the map scale or rose.", 3);
	GMT_Message (API, GMT_TIME_NONE, "\t   If using both -L and -T, use -Fl and -Ft.\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Paint or clip \"dry\" areas.");
	GMT_Message (API, GMT_TIME_NONE, "\t   6) leave off <fill> to issue clip paths for land areas.\n");
	gmt_pen_syntax (API->GMT, 'I', NULL, "Draw rivers.  Specify feature and optionally append pen [Default for all levels: %s].", 0);
	GMT_Message (API, GMT_TIME_NONE, "\t   Choose from the features below.  Repeat the -I option as many times as needed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      0 = Double-lined rivers (river-lakes).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      1 = Permanent major rivers.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      2 = Additional major rivers.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      3 = Additional rivers.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      4 = Minor rivers.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      5 = Intermittent rivers - major.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      6 = Intermittent rivers - additional.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      7 = Intermittent rivers - minor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      8 = Major canals.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      9 = Minor canals.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     10 = Irrigation canals.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Alternatively, specify preselected river groups:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      a = All rivers and canals (0-10).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      A = All rivers and canals except river-lakes (1-10).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      r = All permanent rivers (0-4).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      R = All permanent rivers except river-lakes (1-4).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      i = All intermittent rivers (5-7).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      c = All canals (8-10).\n");
	GMT_Option (API, "K");
	gmt_mapscale_syntax (API->GMT, 'L', "Draw a map scale at specified reference point.");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Dump a multisegment ASCII (or binary, see -bo) file to standard output.  No plotting occurs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify one of -E, -I, -N, or -W (and optionally specific levels; see -W<level>/<pen>).\n");
	gmt_pen_syntax (API->GMT, 'N', NULL, "Draw boundaries.  Specify feature and optionally append pen [Default for all levels: %s].", 0);
	GMT_Message (API, GMT_TIME_NONE, "\t   Choose from the features below.  Repeat the -N option as many times as needed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     1 = National boundaries.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     2 = State boundaries within the Americas.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     3 = Marine boundaries.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     a = All boundaries (1-3).\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Terminate previously set clip-paths.\n");
	gmt_fill_syntax (API->GMT, 'S', NULL, "Paint of clip \"wet\" areas.");
	GMT_Message (API, GMT_TIME_NONE, "\t   6) Leave off <fill> to issue clip paths for water areas.\n");
	gmt_maprose_syntax (API->GMT, 'T', "Draw a north-pointing map rose at specified reference point.");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Draw shorelines.  Append pen [Default for all levels: %s].", 0);
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, set custom pens below.  Repeat the -W option as many times as needed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      1 = Coastline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      2 = Lake shores.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      3 = Island in lakes shores.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      4 = Lake in island in lake shores.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   When feature-specific pens are used, those not set are deactivated.\n");
	GMT_Option (API, "X,bo,c,do,p,t");
#ifdef DEBUG
	GMT_Message (API, GMT_TIME_NONE, "\t-+ Print only a single bin (debug option).\n");
#endif
	GMT_Option (API, ".");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSCOAST_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pscoast and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, k = 0, j = 0;
	int ks;
	bool clipping, get_panel[2] = {false, false}, one = false;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	struct GMT_PEN pen;
	char *string = NULL, *c = NULL, *kind[2] = {"Specify a rectangular panel behind the map scale", "Specify a rectangular panel behind the map rose"};

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				gmt_set_levels (GMT, opt->arg, &Ctrl->A.info);
				break;
			case 'C':	/* Lake colors */
				Ctrl->C.active = true;
				if ((opt->arg[0] == 'l' || opt->arg[0] == 'r') && opt->arg[1] == '/') {	/* Specific lake or river-lake fill [deprecated syntax] */
					k = (opt->arg[0] == 'l') ? LAKE : RIVER;
					j = 2;	one = true;
				}
				else if ((c = strstr (opt->arg, "+l")) || (c = strstr (opt->arg, "+r"))) {	/* Specific lake or river-lake fill */
					k = (c[1] == 'l') ? LAKE : RIVER;
					j = 0;	one = true;	c[0] = '\0';	/* Cut off */
				}
				if (one && opt->arg[j]) {
					if (gmt_getfill (GMT, &opt->arg[j], &Ctrl->C.fill[k])) {
						gmt_fill_syntax (GMT, 'C', NULL, " ");
						n_errors++;
					}
				}
				else if (opt->arg[0]) {
					if (gmt_getfill (GMT, opt->arg, &Ctrl->C.fill[LAKE])) {
						gmt_fill_syntax (GMT, 'C', NULL, " ");
						n_errors++;
					}
					Ctrl->C.fill[RIVER] = Ctrl->C.fill[LAKE];
				}
				if (c) c[0] = '+';	/* Restore */
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.set = (opt->arg[0]) ? opt->arg[0] : 'l';
				Ctrl->D.force = (opt->arg[1] == '+' && (opt->arg[2] == 'f' || opt->arg[2] == '\0'));
				break;
			case 'E':	/* Select DCW items; repeatable */
				Ctrl->E.active = true;
				n_errors += gmt_DCW_parse (GMT, opt->option, opt->arg, &Ctrl->E.info);
				break;
			case 'F':
				if (gmt_M_compat_check (GMT, 5)) {	/* See if we got old -F for DCW stuff (now -E) */
					if (strstr (opt->arg, "+l") || opt->arg[0] == '=' || isupper (opt->arg[0])) {
						GMT_Report (API, GMT_MSG_COMPAT, "-F option for DCW is deprecated, use -E instead.\n");
						Ctrl->E.active = true;
						n_errors += gmt_DCW_parse (GMT, opt->option, opt->arg, &Ctrl->E.info);
						continue;
					}
				}
				Ctrl->F.active = true;
				k = 1;
				switch (opt->arg[0]) {
					case 'l': get_panel[0] = true; break;
					case 't': get_panel[1] = true; break;
					default : get_panel[0] = get_panel[1] = true; k = 0; break;
				}
				if (get_panel[0] && gmt_getpanel (GMT, opt->option, &opt->arg[k], &(Ctrl->L.scale.panel))) {
					gmt_mappanel_syntax (GMT, 'F', kind[0], 3);
					n_errors++;
				}
				if (get_panel[1] && gmt_getpanel (GMT, opt->option, &opt->arg[k], &(Ctrl->T.rose.panel))) {
					gmt_mappanel_syntax (GMT, 'F', kind[1], 3);
					n_errors++;
				}
				break;
			case 'G':		/* Set Gray shade, pattern, or clipping */
				Ctrl->G.active = true;
				if (opt->arg[0] == '\0' || (opt->arg[0] == 'c' && !opt->arg[1]))
					Ctrl->G.clip = true;
				else if (gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'I':
				Ctrl->I.active = true;
				if (!opt->arg[0]) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: -I option takes at least one argument\n");
					n_errors++;
					continue;
				}
				if (strchr (opt->arg, '/') == NULL && strchr (opt->arg, ',')) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: usage is -I<feature>[/<pen>]\n");
					n_errors++;
					continue;
				}
				pen = GMT->current.setting.map_default_pen;	/* Set default pen */
				if ((string = strchr (opt->arg, '/')) != NULL) {	/* Get specified pen */
					if (gmt_getpen (GMT, ++string, &pen)) {	/* Error decoding pen */
						gmt_pen_syntax (GMT, 'I', NULL, " ", 0);
						n_errors++;
					}
				}
				switch (opt->arg[0]) {
					case 'a':
						for (k = 0; k < GSHHS_N_RLEVELS; k++) Ctrl->I.use[k] = 1, Ctrl->I.pen[k] = pen;
						break;
					case 'A':
						for (k = 1; k < GSHHS_N_RLEVELS; k++) Ctrl->I.use[k] = 1, Ctrl->I.pen[k] = pen;
						break;
					case 'r':
						for (k = 0; k < GSHHS_RIVER_INTERMITTENT; k++) Ctrl->I.use[k] = 1, Ctrl->I.pen[k] = pen;
						break;
					case 'R':
						for (k = 1; k < GSHHS_RIVER_INTERMITTENT; k++) Ctrl->I.use[k] = 1, Ctrl->I.pen[k] = pen;
						break;
					case 'i':
						for (k = GSHHS_RIVER_INTERMITTENT; k < GSHHS_RIVER_CANALS; k++) Ctrl->I.use[k] = 1, Ctrl->I.pen[k] = pen;
						break;
					case 'c':
						for (k = GSHHS_RIVER_CANALS; k < GSHHS_N_RLEVELS; k++) Ctrl->I.use[k] = 1, Ctrl->I.pen[k] = pen;
						break;
					default:
						ks = atoi (opt->arg);
						if (ks < 0 || ks >= GSHHS_N_RLEVELS) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -I option: Feature not in list!\n");
							n_errors++;
						}
						else
							Ctrl->I.use[ks] = 1, Ctrl->I.pen[ks] = pen;
						break;
				}
				break;
			case 'L':
				Ctrl->L.active = true;
				n_errors += gmt_getscale (GMT, 'L', opt->arg, GMT_SCALE_MAP, &Ctrl->L.scale);
				break;
			case 'm':
				if (gmt_M_compat_check (GMT, 4))	/* Warn and fall through on purpose */
					GMT_Report (API, GMT_MSG_COMPAT, "-m option is deprecated and reverted back to -M.\n");
				else {
					n_errors += gmt_default_error (GMT, opt->option);
					break;
				}
			case 'M':
				Ctrl->M.active = true;
				if (opt->arg[0] == 's') 	/* Write a single segment. Afects only external interfaces. */
					Ctrl->M.single = true;
				break;
			case 'N':
				Ctrl->N.active = true;
				if (!opt->arg[0]) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: -N option takes at least one argument\n");
					n_errors++;
					continue;
				}
				if (strchr (opt->arg, '/') == NULL && strchr (opt->arg, ',')) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: usage is -N<feature>[/<pen>]\n");
					n_errors++;
					continue;
				}
				pen = GMT->current.setting.map_default_pen;	/* Set default pen */
				if ((string = strchr (opt->arg, '/')) != NULL) {	/* Get specified pen */
					if (gmt_getpen (GMT, ++string, &pen)) {	/* Error decoding pen */
						gmt_pen_syntax (GMT, 'N', NULL, " ", 0);
						n_errors++;
					}
				}
				switch (opt->arg[0]) {
					case 'a':
						for (k = 0; k < GSHHS_N_BLEVELS; k++) Ctrl->N.use[k] = 1, Ctrl->N.pen[k] = pen;
						break;
					default:
						ks = opt->arg[0] - '1';
						if (ks < 0 || ks >= GSHHS_N_BLEVELS) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: Feature not in list!\n");
							n_errors++;
						}
						else
							Ctrl->N.use[ks] = 1, Ctrl->N.pen[ks] = pen;
						break;
				}
				break;
			case 'Q':
				Ctrl->Q.active = true;
				break;
			case 'S':		/* Set ocean color if needed */
				Ctrl->S.active = true;
				if (opt->arg[0] == '\0' || (opt->arg[0] == 'c' && !opt->arg[1]))
					Ctrl->S.clip = true;
				else if (gmt_getfill (GMT, opt->arg, &Ctrl->S.fill)) {
					gmt_fill_syntax (GMT, 'S', NULL, " ");
					n_errors++;
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				n_errors += gmt_getrose (GMT, 'T', opt->arg, &Ctrl->T.rose);
				break;
			case 'W':
				Ctrl->W.active = true;	/* Want to draw shorelines */
				if ((opt->arg[0] >= '1' && opt->arg[0] <= '4') && opt->arg[1] == '/') {	/* Specific pen for this feature */
					k = (int)(opt->arg[0] - '1');
					if (opt->arg[2] && gmt_getpen (GMT, &opt->arg[2], &Ctrl->W.pen[k])) {
						gmt_pen_syntax (GMT, 'W', NULL, " ", 0);
						n_errors++;
					}
					Ctrl->W.use[k] = true;
				}
				else if (opt->arg[0]) {	/* Same pen for all features */
					Ctrl->W.use[0] = true;
					if (gmt_getpen (GMT, opt->arg, &Ctrl->W.pen[0])) {
						gmt_pen_syntax (GMT, 'W', NULL, " ", 0);
						n_errors++;
					}
					else {
						for (k = 1; k < GSHHS_MAX_LEVEL; k++) Ctrl->W.pen[k] = Ctrl->W.pen[0], Ctrl->W.use[k] = true;
					}
				}
				else {	/* Accept default pen for all features */
					for (k = 0; k < GSHHS_MAX_LEVEL; k++) Ctrl->W.use[k] = true;
				}
				break;
#ifdef DEBUG
			case '+':
				Ctrl->debug.active = true;
				Ctrl->debug.bin = atoi (opt->arg);
				break;
#endif

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (gmt_DCW_list (GMT, Ctrl->E.info.mode)) return 1;	/* If +l|L was given we list countries and return */

	if (!GMT->common.J.active) {	/* So without -J we can only do -M or report region only */
		if (Ctrl->M.active) Ctrl->E.info.mode = GMT_DCW_DUMP;
		else if (GMT->common.B.active[GMT_PRIMARY] || GMT->common.B.active[GMT_SECONDARY] || Ctrl->C.active || Ctrl->G.active || Ctrl->I.active || Ctrl->N.active || GMT->common.P.active || Ctrl->S.active || Ctrl->W.active)
			n_errors++;	/* Tried to make a plot but forgot -J */
		else if (!Ctrl->Q.active && Ctrl->E.active)
			Ctrl->E.info.region = true;
	}

	if (Ctrl->E.info.region) {	/* Must report region from chosen polygons */
		unsigned int range = GMT->current.io.geo.range;	/* Old setting */
		char record[GMT_BUFSIZ] = {"-R"}, text[GMT_LEN64] = {""};
		struct GMT_RECORD *Rec = gmt_new_record (GMT, NULL, record);
		size_t i, j;
		GMT->current.io.geo.range = GMT_IGNORE_RANGE;	/* wesn is set correctly so don't mess with it during formatting */
		gmt_ascii_format_col (GMT, text, GMT->common.R.wesn[XLO], GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
		gmt_ascii_format_col (GMT, text, GMT->common.R.wesn[XHI], GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
		gmt_ascii_format_col (GMT, text, GMT->common.R.wesn[YLO], GMT_OUT, GMT_Y);	strcat (record, text);	strcat (record, "/");
		gmt_ascii_format_col (GMT, text, GMT->common.R.wesn[YHI], GMT_OUT, GMT_Y);	strcat (record, text);
		/* Remove any white space due to selected formatting */
		for (i = j = 2; i < strlen (record); i++) {
			if (record[i] == ' ') continue;	/* Skip spaces */
			record[j++] = record[i];
		}
		record[j] = '\0';
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
			return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {
			return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_TEXT) != GMT_NOERROR) {	/* Sets output geometry */
			return (API->error);
		}
		GMT_Put_Record (API, GMT_WRITE_DATA, Rec);	/* Write text record to output destination */
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			return (API->error);
		}
		GMT->current.io.geo.range = range;	/* Reset to what it was */
		gmt_M_free (GMT, Rec);
		return NOT_REALLY_AN_ERROR;	/* To return with "error" but then exit with 0 error */
	}

	if (Ctrl->C.active && !(Ctrl->G.active || Ctrl->S.active || Ctrl->W.active)) {	/* Just lakes, fix -A */
		if (Ctrl->A.info.low < 2) Ctrl->A.info.low = 2;
	}
	/* Check that the options selected are mutually consistent */

	clipping = (Ctrl->G.clip || Ctrl->S.clip);
	if (Ctrl->M.active) {	/* Need -R only */
		n_errors += gmt_M_check_condition (GMT, !Ctrl->E.active && !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->W.active, "Syntax error: Cannot combine -E -M with -W\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->G.active, "Syntax error: Cannot combine -E -M with -G\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->I.active, "Syntax error: Cannot combine -E -M with -I\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->N.active, "Syntax error: Cannot combine -E -M with -N\n");
		if (Ctrl->E.active) Ctrl->E.info.mode |= GMT_DCW_DUMP;	/* -M -E combo means dump DCW data */
	}
	else if (!Ctrl->Q.active) {	/* Need -R -J */
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
		n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	}
	for (k = 0; k < GSHHS_MAX_LEVEL; k++) {
		n_errors += gmt_M_check_condition (GMT, Ctrl->W.pen[k].width < 0.0,
		                                 "Syntax error -W option: Pen thickness for feature %d cannot be negative\n", k);
	}
	n_errors += gmt_M_check_condition (GMT, n_files > 0, "Syntax error: No input files allowed\n");
	n_errors += gmt_M_check_condition (GMT, (Ctrl->G.active + Ctrl->S.active + Ctrl->C.active) > 1 && clipping,
	                                 "Syntax error: Cannot combine -C, -G, -S while clipping\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.clip && Ctrl->S.clip, "Syntax error: Must choose between clipping land OR water\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && (Ctrl->G.active || Ctrl->S.active || Ctrl->C.active),
	                                 "Syntax error: Must choose between dumping and clipping/plotting\n");
	n_errors += gmt_M_check_condition (GMT, clipping && GMT->current.proj.projection_GMT == GMT_AZ_EQDIST && fabs (GMT->common.R.wesn[XLO] -
	                                 GMT->common.R.wesn[XHI]) == 360.0 && (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) == 180.0,
	                                 "-JE not implemented for global clipping - I quit\n");
	n_errors += gmt_M_check_condition (GMT, clipping && (Ctrl->N.active || Ctrl->I.active || Ctrl->W.active),
	                                 "Cannot do clipping AND draw coastlines, rivers, or borders\n");
	n_errors += gmt_M_check_condition (GMT, !(Ctrl->G.active || Ctrl->S.active || Ctrl->C.active || Ctrl->E.active || Ctrl->W.active ||
	                                 Ctrl->N.active || Ctrl->I.active || Ctrl->Q.active),
	                                 "Syntax error: Must specify at least one of -C, -G, -S, -I, -N, -Q and -W\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && (Ctrl->E.active + Ctrl->N.active + Ctrl->I.active + Ctrl->W.active) != 1,
	                                 "Syntax error -M: Must specify one of -E, -I, -N, and -W\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && !(Ctrl->L.active || Ctrl->T.active),
	                                 "Syntax error: -F is only allowed with -L and -T\n");

	if (Ctrl->I.active) {	/* Generate list of desired river features in sorted order */
		for (k = Ctrl->I.n_rlevels = 0; k < GSHHS_N_RLEVELS; k++) {
			if (!Ctrl->I.use[k]) continue;
			n_errors += gmt_M_check_condition (GMT, Ctrl->I.pen[k].width < 0.0,
			                                 "Syntax error -I option: Pen thickness cannot be negative\n");
			Ctrl->I.list[Ctrl->I.n_rlevels++] = k;	/* Since goes from 0-10 */
		}
	}

	if (Ctrl->N.active) {	/* Generate list of desired border features in sorted order */
		for (k = Ctrl->N.n_blevels = 0; k < GSHHS_N_BLEVELS; k++) {
			if (!Ctrl->N.use[k]) continue;
			n_errors += gmt_M_check_condition (GMT, Ctrl->N.pen[k].width < 0.0,
			                                 "Syntax error -N option: Pen thickness cannot be negative\n");
			Ctrl->N.list[Ctrl->N.n_blevels++] = k + 1;	/* Add one so we get range 1-3 */
		}
	}

	return (n_errors);
}

GMT_LOCAL bool add_this_polygon_to_path (struct GMT_CTRL *GMT, int k0, struct GMT_GSHHS_POL *p, int level, int k) {
	/* Determines if we should add the current polygon pol[k] to the growing path we are constructing */

	int j;

	if (p[k].level != level) return (false);	/* Sorry, this polygon does not have the correct level */
	if (k0 == -1) return (true);			/* The start level is always used */
	/* Must make sure the polygon is inside the mother polygon.  Because numerical noise can trick us we try up to two points */
	if (gmt_non_zero_winding (GMT, p[k].lon[0], p[k].lat[0], p[k0].lon, p[k0].lat, p[k0].n)) return (true);	/* OK, we're in! */
	/* First point was not inside.  Test another point to see if the first failure was just an accident */
	j = p[k].n / 2;	/* We pick the second point from the other half of the polygon */
	if (gmt_non_zero_winding (GMT, p[k].lon[j], p[k].lat[j], p[k0].lon, p[k0].lat, p[k0].n)) return (true);	/* One of those rare occasions when we almost missed a polygon */
	return (false);	/* No, we do not want to use this polygon */
}

GMT_LOCAL void recursive_path (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, int k0, int np, struct GMT_GSHHS_POL p[], int level, struct GMT_FILL fill[]) {
	/* To avoid painting where no paint should go we assemble all the swiss cheese polygons by starting
	 * with the lowest level (0) and looking for polygons inside polygons of the same level.  We do this
	 * using a recursive algorithm */

	int k;

	if (level > GSHHS_MAX_LEVEL) return;
	for (k = k0 + 1; k < np; k++) {
		if (p[k].n == 0 || p[k].level < level) continue;
		if (add_this_polygon_to_path (GMT, k0, p, level, k)) {	/* Add this to the current path */
			PSL_comment (PSL, "Polygon %d\n", k);
			PSL_plotline (PSL, p[k].lon, p[k].lat, p[k].n, PSL_MOVE);
#ifdef DEBUGX
			GMT_Message (API, GMT_TIME_NONE, "#\n");
			for (i = 0; i < p[k].n; i++) GMT_Message (API, GMT_TIME_NONE, "%g\t%g\n", p[k].lon[i], p[k].lat[i]);
#endif
			recursive_path (GMT, PSL, k, np, p, level + 1, fill);
			p[k].n = 0;	/* Mark as used */

			if (k0 == -1 && fill) {	/* At start level: done nesting, time to paint the assembled swiss cheese polygon */
				gmt_setfill (GMT, &fill[p[k].fid], 0);
				PSL_command (PSL, "FO\n");
			}
		}
	}
}

GMT_LOCAL int check_antipode_status (struct GMT_CTRL *GMT, struct GMT_SHORE *c, int inside, double clon, double clat, int status[]) {
	/* For a global -JE map we need to know if the projection center and its antipode are on land, ocean, what,
	 * since it affects how donut-hell will behave */
	char old_J[GMT_LEN128] = {""};
	double alon = clon + 180.0;
	if (alon >= 360.0) alon -= 360.0;
	/* Switch to linear projection */
	strncpy (old_J, GMT->common.J.string, GMT_LEN128-1);
	GMT->common.J.active = false;
	gmt_parse_common_options (GMT, "J", 'J', "x1i");
	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) return (-1);
	GMT->current.map.parallel_straight = GMT->current.map.meridian_straight = 2;	/* No resampling along bin boundaries */

	if ((status[0] = gmt_shore_level_at_point (GMT, c, inside, clon, clat)) < 0) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Projection center outside -R region so cannot determine status\n");
	}
	if ((status[1] = gmt_shore_level_at_point (GMT, c, inside, alon, -clat)) < 0) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Projection antipode outside -R region so cannot determine status\n");
	}
	gmt_shore_level_at_point (GMT, c, -1, 0.0, 0.0);	/* Free memory */
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Determined GSHHG level of projection center at (%g, %g) [%d] and antipode at (%g, %g) [%d].\n",
		clon, clat, status[0], alon, -clat, status[1]);
	/* Back to initial projection */
	GMT->common.J.active = false;
	gmt_parse_common_options (GMT, "J", 'J', old_J);
	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) return (-1);
	return (GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_coast (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {	/* See if -E+l|L was given, which is part of usage */
		struct GMT_OPTION *opt = NULL, *options = GMT_Create_Options (API, mode, args);
		bool list_items = false, dump_data = false;
		if (API->error) return (API->error);	/* Set or get option list */
		list_items = ((opt = GMT_Find_Option (API, 'E', options)) && opt->arg[0] == '+' && strchr ("lL", opt->arg[1]));
		dump_data = (GMT_Find_Option (API, 'M', options) != NULL);
		gmt_M_free_options (mode);
		if (!list_items && !dump_data) {
			GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: coast\n");
			return (GMT_NOT_A_VALID_MODULE);
		}
	}
	return GMT_pscoast (V_API, mode, args);
}

EXTERN_MSC uint64_t map_wesn_clip (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n_orig, double **x, double **y, uint64_t *total_nx);

int GMT_pscoast (void *V_API, int mode, void *args) {
	/* High-level function that implements the pscoast task */

	int i, np, ind, bin = 0, base, anti_bin = -1, np_new, k, last_k, err, bin_trouble, error, n;
	int level_to_be_painted[2] = {0, 0}, lp, direction, start_direction, stop_direction, last_pen_level;

	bool shift = false, need_coast_base, recursive;
	bool greenwich = false, possibly_donut_hell = false, fill_in_use = false;
	bool clobber_background = false, paint_polygons = false, donut, double_recursive = false;
	bool donut_hell = false, world_map_save, clipping;
	bool clip_to_extend_lines = false;

	double bin_x[5], bin_y[5], out[2], *xtmp = NULL, *ytmp = NULL;
	double west_border = 0.0, east_border = 0.0, anti_lon = 0.0, anti_lat = -90.0, edge = 720.0;
	double left, right, x_0 = 0.0, y_0 = 0.0, x_c, y_c, dist, sample_step[2] = {0.0, 0.001};

	char *shore_resolution[5] = {"full", "high", "intermediate", "low", "crude"};

	struct GMT_FILL fill[6], no_fill;	/* Colors for (0) water, (1) land, (2) lakes, (3) islands in lakes, (4) lakes in islands in lakes, (5) riverlakes */
	struct GMT_SHORE c;
	struct GMT_BR b, r;
	struct GMT_GSHHS_POL *p = NULL;
	struct GMT_RECORD *Out = NULL;
	struct PSCOAST_CTRL *Ctrl = NULL;		/* Control structure specific to program */
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);		/* Allocate and initialize defaults in a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) {
		if (error == NOT_REALLY_AN_ERROR) Return (0);
		Return (error);
	}

	/*---------------------------- This is the pscoast main code ----------------------------*/

	/* Check and interpret the command line arguments */

	gmt_init_fill (GMT, &no_fill, -1.0, -1.0, -1.0);
	if (GMT->current.setting.run_mode == GMT_MODERN && !Ctrl->D.active)
		Ctrl->D.set = 'a';	/* Auto-select resolution under modern mode if -D not given */
	clipping = (Ctrl->G.clip || Ctrl->S.clip);
	if (Ctrl->D.force) Ctrl->D.set = gmt_shore_adjust_res (GMT, Ctrl->D.set);
	fill[0] = (Ctrl->S.active) ? Ctrl->S.fill : no_fill;
	fill[1] = fill[3] = (Ctrl->G.active) ? Ctrl->G.fill : no_fill;
	fill[2] = fill[4] = (Ctrl->C.active) ? Ctrl->C.fill[LAKE] : fill[0];
	fill[5] = (Ctrl->C.active) ? Ctrl->C.fill[RIVER] : fill[2];
	need_coast_base = (Ctrl->G.active || Ctrl->S.active || Ctrl->C.active || Ctrl->W.active);
	if (Ctrl->Q.active) need_coast_base = false;	/* Since we just end clipping */
	if (Ctrl->G.active && Ctrl->S.active) {	/* Must check if any of then are transparent */
		if (Ctrl->G.fill.rgb[3] > 0.0 || Ctrl->S.fill.rgb[3] > 0.0) {	/* Transparency requested */
			/* Special case since we cannot overprint so must run recursive twice */
			double_recursive = true;
			clobber_background = false;
			GMT_Report (API, GMT_MSG_DEBUG, "Do double recursive painting due to transparency option for land or ocean\n");
		}
		else	/* OK to paint ocean first then overlay land */
			clobber_background = true;
	}
	recursive = (double_recursive || (Ctrl->G.active != (Ctrl->S.active || Ctrl->C.active)) || clipping);
	paint_polygons = (Ctrl->G.active || Ctrl->S.active || Ctrl->C.active);

	if (GMT->common.R.wesn[XLO] > 360.0) {
		GMT->common.R.wesn[XLO] -= 360.0;
		GMT->common.R.wesn[XHI] -= 360.0;
	}

	if ((Ctrl->Q.active || Ctrl->E.active) && !GMT->common.J.active) {	/* Set fake area and linear projection */
		gmt_parse_common_options (GMT, "J", 'J', "x1d");
		GMT->common.R.wesn[XLO] = GMT->common.R.wesn[YLO] = 0.0;
		GMT->common.R.wesn[XHI] = GMT->common.R.wesn[YHI] = 1.0;
	}
	else if (Ctrl->M.active && !GMT->common.J.active)	/* Set fake linear projection */
		gmt_parse_common_options (GMT, "J", 'J', "x1d");
	else if (GMT->common.J.active && gmt_M_is_cartesian (GMT, GMT_IN)) {	/* Gave -J but forgot the "d" */
		gmt_set_geographic (GMT, GMT_IN);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Switching to -Jx|X...d[/...d] for geographic data\n");
	}

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	base = gmt_set_resolution (GMT, &Ctrl->D.set, 'D');

	world_map_save = GMT->current.map.is_world;

	if (!Ctrl->M.active && (Ctrl->W.active || Ctrl->I.active || Ctrl->N.active))
		clip_to_extend_lines = (!(gmt_M_is_azimuthal (GMT) && !GMT->current.proj.polar) || GMT->common.R.oblique);
	
	if (need_coast_base && (err = gmt_init_shore (GMT, Ctrl->D.set, &c, GMT->common.R.wesn, &Ctrl->A.info)) != 0)  {
		GMT_Report (API, GMT_MSG_NORMAL, "%s [GSHHG %s resolution shorelines]\n", GMT_strerror(err), shore_resolution[base]);
		need_coast_base = false;
	}

	if (Ctrl->N.active && (err = gmt_init_br (GMT, 'b', Ctrl->D.set, &b, GMT->common.R.wesn)) != 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "%s [GSHHG %s resolution political boundaries]\n", GMT_strerror(err), shore_resolution[base]);
		Ctrl->N.active = false;
	}
	if (need_coast_base) {
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "GSHHG version %s\n", c.version);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%s\n", c.title);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%s\n", c.source);
	}

	if (Ctrl->I.active && (err = gmt_init_br (GMT, 'r', Ctrl->D.set, &r, GMT->common.R.wesn)) != 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "%s [GSHHG %s resolution rivers]\n", GMT_strerror(err), shore_resolution[base]);
		Ctrl->I.active = false;
	}

	if (!(need_coast_base || Ctrl->E.active || Ctrl->N.active || Ctrl->I.active || Ctrl->Q.active)) {
		GMT_Report (API, GMT_MSG_NORMAL, "No GSHHG databases available - must abort\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->M.active) {	/* Dump linesegments to stdout; no plotting takes place */
		int id = 0;
		char header[GMT_BUFSIZ] = {""}, *kind[3] = {"Coastlines", "Political boundaries", "Rivers"}, *version = NULL, *title = NULL, *source = NULL;
		gmt_set_geographic (GMT, GMT_OUT);	/* Output lon/lat */
		if (Ctrl->N.active) id = 1;
		else if (Ctrl->I.active) id = 2;
	
		if (!Ctrl->M.single)
			gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */

		if ((error = GMT_Set_Columns (API, GMT_OUT, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_LINE) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
		if (Ctrl->W.active) {
			version = c.version;	title = c.title;	source = c.source;
		}
		else if (Ctrl->N.active) {
			version = b.version;	title = b.title;	source = b.source;
		}
		else {
			version = r.version;	title = r.title;	source = r.source;
		}
		if (Ctrl->E.active) {
			sprintf (header, "Country polygons extracted from the DCW database\n");
			GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, header);
		}
		else {
			sprintf (header, "%s extracted from the %s resolution GSHHG version %s database\n", kind[id], shore_resolution[base], version);
			GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, header);
			GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, title);
			GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, source);
		}
		Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
	}
	else {
		if (Ctrl->Q.active)
			GMT->current.ps.nclip = -1;	/* Signal that this program terminates polygon clipping that initiated prior to this process */
		else if (clipping)
			GMT->current.ps.nclip = +1;	/* Signal that this program initiates new clipping that will outlive this process */

		if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);

		if (Ctrl->Q.active) {  /* Just undo previous clip-path */
			PSL_endclipping (PSL, 1);

			if (GMT->common.B.active[GMT_PRIMARY] || GMT->common.B.active[GMT_SECONDARY]) {
				gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
				gmt_plotcanvas (GMT);	/* Fill canvas if requested */
				gmt_map_basemap (GMT); /* Basemap needed */
				gmt_plane_perspective (GMT, -1, 0.0);
			}

			gmt_plotend (GMT);

			Return (GMT_NOERROR);
		}

		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	}

	for (i = 0; i < 5; i++) if (fill[i].use_pattern) fill_in_use = true;

	if (fill_in_use && !clobber_background) {	/* Force clobber when fill is used since our routine cannot deal with clipped fills */
		GMT_Report (API, GMT_MSG_VERBOSE, "Pattern fill requires oceans to be painted first\n");
		clobber_background = true;
		recursive = false;
	}

	if (clipping) gmt_map_basemap (GMT);

	if (GMT->current.proj.projection_GMT == GMT_AZ_EQDIST && gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]) && gmt_M_180_range (GMT->common.R.wesn[YHI], GMT->common.R.wesn[YLO])) {
		int status[2] = {0, 0};
		if (check_antipode_status (GMT, &c, GMT_INSIDE, GMT->current.proj.central_meridian, GMT->current.proj.pole, status)) {
			GMT_Report (API, GMT_MSG_VERBOSE, "check_antipode_status crashed - not good\n");
		}
		possibly_donut_hell = true;
		anti_lon = GMT->current.proj.central_meridian + 180.0;
		if (anti_lon >= 360.0) anti_lon -= 360.0;
		anti_lat = -GMT->current.proj.pole;
		anti_bin = irint (floor ((90.0 - anti_lat) / c.bsize)) * c.bin_nx + irint (floor (anti_lon / c.bsize));
		gmt_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &x_0, &y_0);
		if (Ctrl->G.active)
			GMT_Report (API, GMT_MSG_VERBOSE, "Fill/clip continent option (-G) may not work for this projection.\n"
			                                  "If the antipole (%g/%g) is in the ocean then chances are good\n"
			                                  "Else: avoid projection center coordinates that are exact multiples of %g degrees\n",
			                                  anti_lon, anti_lat, c.bsize);
	}

	if (possibly_donut_hell && paint_polygons && !clobber_background) {	/* Force clobber when donuts may be called for now */
		GMT_Report (API, GMT_MSG_VERBOSE, "-JE requires oceans to be painted first\n");
		clobber_background = true;
		recursive = false;
		if (!Ctrl->S.active)	/* Since we are painting wet areas we must now reset them to white */
			gmt_init_fill (GMT, &fill[0], 1.0, 1.0, 1.0);		/* Default Ocean color = white */
		fill[2] = fill[4] = (Ctrl->C.active) ? Ctrl->C.fill[LAKE] : fill[0];	/* If lake not set then use ocean */
	}

	if (clobber_background) {	/* Paint entire map as ocean first, then lay land on top */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Painting entire map with ocean color first, then draw land on top later\n");
		n = (int)gmt_map_clip_path (GMT, &xtmp, &ytmp, &donut);
		gmt_setfill (GMT, &Ctrl->S.fill, 0);
		if (donut) {
			/* If donut, then the path consists of two path of np points */
			PSL_plotline (PSL, xtmp, ytmp, n, PSL_MOVE|PSL_CLOSE);
			PSL_plotpolygon (PSL, &xtmp[n], &ytmp[n], n);
		}
		else
			PSL_plotpolygon (PSL, xtmp, ytmp, n);
		gmt_M_free (GMT, xtmp);
		gmt_M_free (GMT, ytmp);
		level_to_be_painted[0] = level_to_be_painted[1] = 1;	/* Land */
		start_direction = stop_direction = 1;	/* Since we are only doing land in the loop below */
	}
	if (double_recursive) {	/* Must recursively paint both land and ocean */
		start_direction = -1;	stop_direction = 1;
		level_to_be_painted[0] = 0;	level_to_be_painted[1] = 1;
	}
	else if (recursive) {	/* Just paint either land or ocean recursively */
		start_direction = stop_direction = (Ctrl->G.active) ? 1 : -1;
		level_to_be_painted[0] = level_to_be_painted[1] = (Ctrl->G.active) ? 1 : 0;
	}
	else if (!clobber_background) {	/* Just paint one of them but not recursively; skip this if clobber_backtround is true */
		start_direction = -1;	stop_direction = 1;
		level_to_be_painted[0] = level_to_be_painted[1] = (Ctrl->S.active) ? 0 : 1;
	}

	if (GMT->common.R.wesn[XLO] < 0.0 && GMT->common.R.wesn[XHI] > 0.0 && !gmt_M_is_linear (GMT)) greenwich = true;
	if (need_coast_base && (360.0 - fabs (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]) ) < c.bsize) {
		GMT->current.map.is_world = true;
		if (GMT->current.proj.projection_GMT == GMT_GNOMONIC || GMT->current.proj.projection_GMT == GMT_GENPER) GMT->current.map.is_world = false;
		if (gmt_M_is_azimuthal (GMT)) GMT->current.map.is_world = false;
	}
	if (GMT->current.map.is_world && greenwich)
		edge = GMT->current.proj.central_meridian;
	else if (!GMT->current.map.is_world && greenwich) {
		shift = true;
		edge = GMT->common.R.wesn[XLO];	if (edge < 0.0) edge += 360.0;
		if (edge > 180.0) edge = 180.0;
	}

	if (GMT->common.R.wesn[XLO] < 0.0 && GMT->common.R.wesn[XHI] <= 0.0) {	/* Temporarily shift boundaries */
		GMT->common.R.wesn[XLO] += 360.0;
		GMT->common.R.wesn[XHI] += 360.0;
		if (GMT->current.proj.central_meridian < 0.0) GMT->current.proj.central_meridian += 360.0;
	}
	if (need_coast_base) {
		west_border = floor (GMT->common.R.wesn[XLO] / c.bsize) * c.bsize;
		east_border = ceil  (GMT->common.R.wesn[XHI] / c.bsize) * c.bsize;
		GMT->current.map.coastline = true;
	}

	if (!Ctrl->M.active && Ctrl->W.active) {
		gmt_setpen (GMT, &Ctrl->W.pen[0]);
	}
	if (clip_to_extend_lines) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);

	last_pen_level = -1;

	if (clipping) PSL_beginclipping (PSL, xtmp, ytmp, 0, GMT->session.no_rgb, 1);	/* Start clippath */

	for (ind = 0; need_coast_base && ind < c.nb; ind++) {	/* Loop over necessary bins only */

		bin = c.bins[ind];
#ifdef DEBUG
		if (Ctrl->debug.active && bin != Ctrl->debug.bin) continue;
#endif
		if ((err = gmt_get_shore_bin (GMT, ind, &c)) != 0) {
			GMT_Report (API, GMT_MSG_NORMAL, "%s [%s resolution shoreline]\n", GMT_strerror(err), shore_resolution[base]);
			Return (GMT_RUNTIME_ERROR);
		}

		GMT_Report (API, GMT_MSG_DEBUG, "Working on bin # %5d\r", bin);
		if (!Ctrl->M.active) PSL_comment (PSL, "Bin # %d\n", bin);

		if (GMT->current.map.is_world && greenwich) {
			left = c.bsize * (bin % c.bin_nx);	right = left + c.bsize;
			shift = ((left - edge) <= 180.0 && (right - edge) > 180.0);
		}

		bin_trouble = (anti_bin == bin) ? anti_bin : -1;

		if (possibly_donut_hell) {
			/* Fill a 5-point polygon with bin corners */
			bin_x[0] = bin_x[3] = bin_x[4] = c.lon_sw;
			bin_x[1] = bin_x[2] = c.lon_sw + c.bsize;
			bin_y[0] = bin_y[1] = bin_y[4] = c.lat_sw;
			bin_y[2] = bin_y[3] = c.lat_sw + c.bsize;
			/* Determine (x,y) coordinate of projected bin center */
			gmt_geo_to_xy (GMT, c.lon_sw + 0.5 * c.bsize, c.lat_sw + 0.5 * c.bsize, &x_c, &y_c);
			dist = hypot (x_c - x_0, y_c - y_0);	/* Distance from projection center to mid-point of current bin */
			/* State donut_hell if we are close (within 20%) to the horizon or if the bin contains the antipole.
			 * This is simply done with a Cartesian inside/outside test, which is adequate.
			 * the 0.8 factor is arbitrary of course [PW] */
		
			donut_hell = (dist > 0.8 * GMT->current.proj.r || gmt_non_zero_winding (GMT, anti_lon, anti_lat, bin_x, bin_y, 5));
			if (donut_hell)
				GMT_Report (API, GMT_MSG_DEBUG, "Donut-hell is true for bin %d\n", bin);
		}

		for (direction = start_direction; paint_polygons && direction <= stop_direction; direction += 2) {
			lp = (direction == -1) ? 0 : 1;
			/* Assemble one or more segments into polygons */

			if ((np = gmt_assemble_shore (GMT, &c, direction, true, west_border, east_border, &p)) == 0) continue;

			/* Get clipped polygons in x,y inches that can be plotted */

			np_new = gmt_prep_shore_polygons (GMT, &p, np, donut_hell, sample_step[donut_hell], bin_trouble);

			if (clipping) {
				for (k = level_to_be_painted[lp]; k < GSHHS_MAX_LEVEL - 1; k++)
					recursive_path (GMT, PSL, -1, np_new, p, k, NULL);

				for (k = 0; k < np_new; k++) {	/* Do any remaining interior polygons */
					if (p[k].n == 0) continue;
					if (p[k].level % 2 == level_to_be_painted[lp] || p[k].level > 2) {
						PSL_plotline (PSL, p[k].lon, p[k].lat, p[k].n, PSL_MOVE);
					}
				}
			}
			else if (recursive) {	/* Must avoid pointing anything but the polygons inside */

				for (k = level_to_be_painted[lp]; k < GSHHS_MAX_LEVEL - 1; k++)
					recursive_path (GMT, PSL, -1, np_new, p, k, fill);

				for (k = 0; k < np_new; k++) {	/* Do any remaining interior polygons */
					if (p[k].n == 0) continue;
					if (p[k].level % 2 == level_to_be_painted[lp] || p[k].level > 2) {
						gmt_setfill (GMT, &fill[p[k].fid], 0);
						PSL_plotpolygon (PSL, p[k].lon, p[k].lat, p[k].n);
					}
				}
			}
			else {	/* Simply paints all polygons as is */
				for (k = 0; k < np_new; k++) {
					if (p[k].n == 0 || p[k].level < level_to_be_painted[lp]) continue;
					if (donut_hell && gmt_non_zero_winding (GMT, x_0, y_0, p[k].lon, p[k].lat, p[k].n)) {	/* Antipode inside polygon, must do donut */
						n = (int)gmt_map_clip_path (GMT, &xtmp, &ytmp, &donut);
						GMT_Report (API, GMT_MSG_DEBUG, "Doing donut filling for bin %d\n", bin);
						gmt_setfill (GMT, &fill[p[k].fid], 0);
						PSL_plotline (PSL, xtmp, ytmp, n, PSL_MOVE|PSL_CLOSE);
						PSL_plotpolygon (PSL, p[k].lon, p[k].lat, p[k].n);
						gmt_M_free (GMT, xtmp);
						gmt_M_free (GMT, ytmp);
					}
					else {
						gmt_setfill (GMT, &fill[p[k].fid], 0);
						PSL_plotpolygon (PSL, p[k].lon, p[k].lat, p[k].n);
					}
				}
			}

			gmt_free_shore_polygons (GMT, p, np_new);
			gmt_M_free (GMT, p);
		}

		if (Ctrl->W.active && c.ns) {	/* Draw or dump shorelines, no need to assemble polygons */
			if ((np = gmt_assemble_shore (GMT, &c, 1, false, west_border, east_border, &p)) == 0) {
				gmt_free_shore (GMT, &c);
				continue;
			}

			for (i = 0; i < np; i++) {
				if (Ctrl->M.active) {	/* Clip to specified region - this gives x,y coordinates in inches */
					uint64_t unused, n_out;
				
					if (!Ctrl->W.use[p[i].level-1]) continue;

					n_out = map_wesn_clip (GMT, p[i].lon, p[i].lat, p[i].n, &xtmp, &ytmp, &unused);
					if (!Ctrl->M.single) {
						sprintf (GMT->current.io.segment_header, "Shore Bin # %d, Level %d", bin, p[i].level);
						GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
					}
					else {
						out[GMT_X] = out[GMT_Y] = GMT->session.d_NaN;
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					}
					for (k = 0; k < (int)n_out; k++) {	/* Convert back to lon,lat before writing output */
						gmt_xy_to_geo (GMT, &out[GMT_X], &out[GMT_Y], xtmp[k], ytmp[k]);
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					}
					gmt_M_free (GMT, xtmp);
					gmt_M_free (GMT, ytmp);
				}
				else if (Ctrl->W.use[p[i].level-1]) {
					if (donut_hell) p[i].n = (int)gmt_fix_up_path (GMT, &p[i].lon, &p[i].lat, p[i].n, 0.0, 0);
					GMT->current.plot.n = gmt_geo_to_xy_line (GMT, p[i].lon, p[i].lat, p[i].n);
					if (!GMT->current.plot.n) continue;

					k = p[i].level - 1;
					if (k != last_pen_level) {
						gmt_setpen (GMT, &Ctrl->W.pen[k]);
						last_pen_level = k;
					}
					gmt_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n, PSL_LINEAR);
				}
			}

			gmt_free_shore_polygons (GMT, p, np);
			gmt_M_free (GMT, p);
		}

		gmt_free_shore (GMT, &c);

	}
	if (need_coast_base) {
		GMT_Report (API, GMT_MSG_DEBUG, "Working on bin # %5ld\n", bin);
		gmt_shore_cleanup (GMT, &c);
		GMT->current.map.coastline = false;
	}

	if (Ctrl->E.info.mode > GMT_DCW_REGION)
		(void)gmt_DCW_operation (GMT, &Ctrl->E.info, NULL, Ctrl->M.active ? GMT_DCW_DUMP : GMT_DCW_PLOT);

	if (clipping) PSL_beginclipping (PSL, xtmp, ytmp, 0, GMT->session.no_rgb, 2);	/* End clippath */

	if (Ctrl->I.active) {	/* Read rivers file and plot as lines */

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Adding Rivers...");
		if (!Ctrl->M.active) PSL_comment (PSL, "Start of River segments\n");
		last_k = -1;

		for (ind = 0; ind < r.nb; ind++) {	/* Loop over necessary bins only */

			bin = r.bins[ind];

#ifdef DEBUG
			if (Ctrl->debug.active && bin != Ctrl->debug.bin) continue;
#endif
			gmt_get_br_bin (GMT, ind, &r, Ctrl->I.list, Ctrl->I.n_rlevels);

			if (r.ns == 0) continue;

			if (GMT->current.map.is_world && greenwich) {
				left = r.bsize * (bin % r.bin_nx);	right = left + r.bsize;
				shift = ((left - edge) <= 180.0 && (right - edge) > 180.0);
			}

			if ((np = gmt_assemble_br (GMT, &r, shift, edge, &p)) == 0) {
				gmt_free_br (GMT, &r);
				continue;
			}

			for (i = 0; i < np; i++) {
				if (Ctrl->M.active) {
					if (!Ctrl->M.single) {
						sprintf (GMT->current.io.segment_header, "River Bin # %d, Level %d", bin, p[i].level);
						GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
					}
					else {
						out[GMT_X] = out[GMT_Y] = GMT->session.d_NaN;
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					}
					for (k = 0; k < p[i].n; k++) {
						out[GMT_X] = p[i].lon[k];
						out[GMT_Y] = p[i].lat[k];
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					}
				}
				else {
					GMT->current.plot.n = gmt_geo_to_xy_line (GMT, p[i].lon, p[i].lat, p[i].n);
					if (!GMT->current.plot.n) continue;

					k = p[i].level;
					if (k != last_k) {
						gmt_setpen (GMT, &Ctrl->I.pen[k]);
						last_k = k;
					}
					gmt_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n, PSL_LINEAR);
				}
			}

			/* Free up memory */

			gmt_free_br (GMT, &r);
			gmt_free_shore_polygons (GMT, p, np);
			gmt_M_free (GMT, p);
		}
		gmt_br_cleanup (GMT, &r);
	}

	if (Ctrl->N.active) {	/* Read borders file and plot as lines */
		double step;
	
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Adding Borders...");
		if (!Ctrl->M.active) PSL_comment (PSL, "Start of Border segments\n");

		/* Must resample borders because some points may be too far apart and look like 'jumps' */

		step = MAX (fabs(GMT->common.R.wesn[XLO] - GMT->common.R.wesn[XHI]), fabs (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO])) / 4.0;

		last_k = -1;

		for (ind = 0; ind < b.nb; ind++) {	/* Loop over necessary bins only */

			bin = b.bins[ind];
#ifdef DEBUG
			if (Ctrl->debug.active && bin != Ctrl->debug.bin) continue;
#endif
			gmt_get_br_bin (GMT, ind, &b, Ctrl->N.list, Ctrl->N.n_blevels);

			if (b.ns == 0) continue;

			if (GMT->current.map.is_world && greenwich) {
				left = b.bsize * (bin % b.bin_nx);	right = left + b.bsize;
				shift = ((left - edge) <= 180.0 && (right - edge) > 180.0);
			}

			if ((np = gmt_assemble_br (GMT, &b, shift, edge, &p)) == 0) {
				gmt_free_br (GMT, &b);
				continue;
			}

			for (i = 0; i < np; i++) {
				if (Ctrl->M.active) {
					if (!Ctrl->M.single) {
						sprintf (GMT->current.io.segment_header, "Border Bin # %d, Level %d", bin, p[i].level);
						GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
					}
					else {
						out[GMT_X] = out[GMT_Y] = GMT->session.d_NaN;
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					}
					for (k = 0; k < p[i].n; k++) {
						out[GMT_X] = p[i].lon[k];
						out[GMT_Y] = p[i].lat[k];
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					}
				}
				else {
					p[i].n = (int)gmt_fix_up_path (GMT, &p[i].lon, &p[i].lat, p[i].n, step, 0);
					GMT->current.plot.n = gmt_geo_to_xy_line (GMT, p[i].lon, p[i].lat, p[i].n);
					if (!GMT->current.plot.n) continue;

					k = p[i].level - 1;
					if (k != last_k) {
						gmt_setpen (GMT, &Ctrl->N.pen[k]);
						last_k = k;
					}
					gmt_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n, PSL_LINEAR);
				}
			}

			/* Free up memory */

			gmt_free_br (GMT, &b);
			gmt_free_shore_polygons (GMT, p, np);
			gmt_M_free (GMT, p);
		}
		gmt_br_cleanup (GMT, &b);
	}

	if (!Ctrl->M.active) {
		if (clip_to_extend_lines) gmt_map_clip_off (GMT);
		if (GMT->current.map.frame.init) {
			GMT->current.map.is_world = world_map_save;
			gmt_map_basemap (GMT);
		}

		if (Ctrl->L.active) gmt_draw_map_scale (GMT, &Ctrl->L.scale);
		if (Ctrl->T.active) gmt_draw_map_rose (GMT, &Ctrl->T.rose);

		gmt_plane_perspective (GMT, -1, 0.0);
		gmt_plotend (GMT);
	}
	else if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {
		Return (API->error);	/* Disables further data output */
	}
	gmt_M_free (GMT, Out);

	GMT->current.map.coastline = false;

	Return (GMT_NOERROR);
}
