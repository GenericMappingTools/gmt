/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#define THIS_MODULE_NAME	"pscoast"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot continents, countries, shorelines, rivers, and borders on maps"
#define THIS_MODULE_KEYS	"-Xo,>Do"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->BJKOPRUVXYbcdptxy" GMT_OPT("Z")


#define LAKE	0
#define RIVER	1

#define NOT_REALLY_AN_ERROR -999

/* Control structure for pscoast */

struct PSCOAST_CTRL {
	struct A {	/* -A<min_area>[/<min_level>/<max_level>][+ag|i|s][+r|l][+p<percent>] */
		bool active;
		struct GMT_SHORE_SELECT info;
	} A;
	struct C {	/* -C<fill> */
		bool active;
		struct GMT_FILL fill[2];	/* lake and riverlake fill */
	} C;
	struct D {	/* -D<resolution> */
		bool active;
		bool force;	/* if true, select next highest level if current set is not available */
		char set;	/* One of f, h, i, l, c */
	} D;
	struct E {	/* -E<DWC-options> */
		bool active;
		struct GMT_DCW_SELECT info;
	} E;
	struct F {	/* -F[+c<clearance>][+g<fill>][+i[<off>/][<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;
		/* The panel struct is a member of the GMT_MAP_SCALE in -L */
	} F;
	struct G {	/* -G<fill> */
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
	struct L {	/* -L */
		bool active;
		struct GMT_MAP_SCALE scale;
	} L;
	struct M {	/* -M */
		bool active;
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
	struct S {	/* -S<fill> */
		bool active;
		bool clip;
		struct GMT_FILL fill;
	} S;
	struct T {	/* -L */
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

void *New_pscoast_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	unsigned int k;
	struct PSCOAST_CTRL *C = GMT_memory (GMT, NULL, 1, struct PSCOAST_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.info.high = GSHHS_MAX_LEVEL;			/* Include all GSHHS levels */
	C->D.set = 'l';						/* Low-resolution coastline data */
	if (GMT->current.map.frame.paint)	/* Default Ocean color = Frame background color */
		C->S.fill = GMT->current.map.frame.fill;
	else
		GMT_init_fill (GMT, &C->S.fill, 1.0, 1.0, 1.0);		/* Default Ocean color = white */
	C->C.fill[LAKE] = C->C.fill[RIVER] = C->S.fill;		/* Default Lake/Riverlake color = Ocean color */
	GMT_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);		/* Default Land color = black */
	for (k = 0; k < GSHHS_N_RLEVELS; k++) C->I.pen[k] = GMT->current.setting.map_default_pen;		/* Default river pens */
	for (k = 0; k < GSHHS_N_BLEVELS; k++) C->N.pen[k] = GMT->current.setting.map_default_pen;		/* Default border pens */
	for (k = 0; k < GSHHS_MAX_LEVEL; k++) C->W.pen[k] = GMT->current.setting.map_default_pen;	/* Default coastline pens */
	GMT_memset (&C->L.scale, 1, struct GMT_MAP_SCALE);
	GMT_memset (&C->T.rose, 1, struct GMT_MAP_ROSE);

	return (C);
}

void Free_pscoast_Ctrl (struct GMT_CTRL *GMT, struct PSCOAST_CTRL *C) {	/* Deallocate control structure */
	unsigned int k;
	if (!C) return;
	if (C->E.active && C->E.info.n_items) {
		for (k = 0; k < C->E.info.n_items; k++)
			free ((void *)C->E.info.item[k].codes);
		GMT_free (GMT, C->E.info.item);
	}
	GMT_free (GMT, C);
}

int GMT_pscoast_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the pscoast synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: pscoast %s %s [%s]\n", GMT_B_OPT, GMT_J_OPT, GMT_A_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-C[<feature>/]<fill>]\n\t[-D<resolution>][+] [-E%s] -G[<fill>]]\n", GMT_Rgeoz_OPT, DCW_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s]\n", GMT_PANEL);
	GMT_Message (API, GMT_TIME_NONE, "\t[-I<feature>[/<pen>]] [%s] [-K]\n\t[-L%s]\n", GMT_Jz_OPT, GMT_SCALE);
	GMT_Message (API, GMT_TIME_NONE, "\t[-M] [-N<feature>[/<pen>]] [-O] [-P] [-Q] [-S<fill>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T%s]\n\t[%s] [%s] [-W[<feature>/][<pen>]]\n", GMT_TROSE, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n", GMT_X_OPT, GMT_Y_OPT, GMT_bo_OPT, GMT_do_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s]\n", GMT_c_OPT, GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT);
#ifdef DEBUG
	GMT_Message (API, GMT_TIME_NONE, "\t[-+<bin>]\n");
#endif

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Option (API, "J-Z,G");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_GSHHG_syntax (API->GMT, 'A');
	GMT_Option (API, "B-");
	GMT_fill_syntax (API->GMT, 'C', "Set separate color for lakes and riverlakes [Default is same as ocean].");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, set custom fills below.  Repeat the -C option as needed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      l = Lakes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      r = River-lakes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Choose one of the following resolutions:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a - auto: select best resolution given map scale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f - full resolution (may be very slow for large regions).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   h - high resolution (may be slow for large regions).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i - intermediate resolution.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   l - low resolution [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c - crude resolution, for busy plots that need crude continent outlines only.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append + to use a lower resolution should the chosen one not be available [abort].\n");
	GMT_DCW_option (API, 'E', 1U);
	GMT_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the map scale or rose", 3);
	GMT_Message (API, GMT_TIME_NONE, "\t   If using both -L and -T, you can repeat -F following each option.\n");
	GMT_fill_syntax (API->GMT, 'G', "Paint or clip \"dry\" areas.");
	GMT_Message (API, GMT_TIME_NONE, "\t   6) c to issue clip paths for land areas.\n");
	GMT_pen_syntax (API->GMT, 'I', "Draw rivers.  Specify feature and optionally append pen [Default for all levels: %s].");
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
	GMT_mapscale_syntax (API->GMT, 'L', "Draws a simple map scale centered on <lon0>/<lat0>");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Dump a multisegment ASCII (or binary, see -bo) file to standard output.  No plotting occurs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify one of -E, -I, -N, or -W.\n");
	GMT_pen_syntax (API->GMT, 'N', "Draw boundaries.  Specify feature and optionally append pen [Default for all levels: %s].");
	GMT_Message (API, GMT_TIME_NONE, "\t   Choose from the features below.  Repeat the -N option as many times as needed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     1 = National boundaries.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     2 = State boundaries within the Americas.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     3 = Marine boundaries.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     a = All boundaries (1-3).\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Terminate previously set clip-paths.\n");
	GMT_fill_syntax (API->GMT, 'S', "Paint of clip \"wet\" areas.");
	GMT_Message (API, GMT_TIME_NONE, "\t   6) c to issue clip paths for water areas.\n");
	GMT_maprose_syntax (API->GMT, 'T', "Draw a north-pointing map rose centered on <lon0>/<lat0>.");
	GMT_Option (API, "U,V");
	GMT_pen_syntax (API->GMT, 'W', "Draw shorelines.  Append pen [Default for all levels: %s].");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, set custom pens below.  Repeat the -W option as many times as needed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      1 = Coastline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      2 = Lake shores.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      3 = Island in lakes shores.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      4 = Lake in island in lake shores.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   When feature-specific pens are used, those not set are deactivated.\n");
	GMT_Option (API, "X,bo,do,c,p,t");
#ifdef DEBUG
	GMT_Message (API, GMT_TIME_NONE, "\t-+ Print only a single bin (debug option).\n");
#endif
	GMT_Option (API, ".");

	return (EXIT_FAILURE);
}

int GMT_pscoast_parse (struct GMT_CTRL *GMT, struct PSCOAST_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pscoast and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, k;
	int ks;
	bool clipping;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	struct GMT_PEN pen;
	char *string = NULL;
	struct GMT_MAP_PANEL *this_panel = NULL;	/* Current panel to specify */

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				GMT_set_levels (GMT, opt->arg, &Ctrl->A.info);
				break;
			case 'C':	/* Lake colors */
				Ctrl->C.active = true;
				if ((opt->arg[0] == 'l' || opt->arg[0] == 'r') && opt->arg[1] == '/') {	/* Specific lake or river-lake fill */
					k = (opt->arg[0] == 'l') ? LAKE : RIVER;
					if (opt->arg[2] && GMT_getfill (GMT, &opt->arg[2], &Ctrl->C.fill[k])) {
						GMT_fill_syntax (GMT, 'C', " ");
						n_errors++;
					}
				}
				else if (opt->arg[0]) {
					if (GMT_getfill (GMT, opt->arg, &Ctrl->C.fill[LAKE])) {
						GMT_fill_syntax (GMT, 'C', " ");
						n_errors++;
					}
					Ctrl->C.fill[RIVER] = Ctrl->C.fill[LAKE];
				}
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.set = (opt->arg[0]) ? opt->arg[0] : 'l';
				Ctrl->D.force = (opt->arg[1] == '+');
				break;
			case 'E':	/* Select DCW items; repeatable */
				Ctrl->E.active = true;
				n_errors += GMT_DCW_parse (GMT, opt->option, opt->arg, &Ctrl->E.info);
				break;
			case 'F':
				if (GMT_compat_check (GMT, 5)) {	/* See if we got old -F for DCW stuff (now -E) */
					if (strstr (opt->arg, "+l") || opt->arg[0] == '=' || isupper (opt->arg[0])) {
						GMT_Report (API, GMT_MSG_COMPAT, "Warning: -F option for DCW is deprecated, use -E instead.\n");
						Ctrl->E.active = true;
						n_errors += GMT_DCW_parse (GMT, opt->option, opt->arg, &Ctrl->E.info);
						continue;
					}
				}
				Ctrl->F.active = true;
				if (this_panel == NULL) this_panel = &(Ctrl->L.scale.panel); 
				if (GMT_getpanel (GMT, opt->option, opt->arg, this_panel)) {
					GMT_mappanel_syntax (GMT, 'F', "Specify a rectanglar panel behind the scale", 3);
					n_errors++;
				}
				break;
			case 'G':		/* Set Gray shade, pattern, or clipping */
				Ctrl->G.active = true;
				if (opt->arg[0] == 'c' && !opt->arg[1])
					Ctrl->G.clip = true;
				else if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
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
				pen = GMT->current.setting.map_default_pen;	/* Set default pen */
				if ((string = strchr (opt->arg, '/'))) {	/* Get specified pen */
					if (GMT_getpen (GMT, ++string, &pen)) {	/* Error decoding pen */
						GMT_pen_syntax (GMT, 'I', " ");
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
				n_errors += GMT_getscale (GMT, 'L', opt->arg, &Ctrl->L.scale);
				this_panel = &(Ctrl->L.scale.panel);
				break;
			case 'm':
				if (GMT_compat_check (GMT, 4))	/* Warn and fall through */
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -m option is deprecated and reverted back to -M.\n");
				else {
					n_errors += GMT_default_error (GMT, opt->option);
					break;
				}
			case 'M':
				Ctrl->M.active = true;
				break;
			case 'N':
				Ctrl->N.active = true;
				if (!opt->arg[0]) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: -N option takes at least one argument\n");
					n_errors++;
					continue;
				}
				pen = GMT->current.setting.map_default_pen;	/* Set default pen */
				if ((string = strchr (opt->arg, '/'))) {	/* Get specified pen */
					if (GMT_getpen (GMT, ++string, &pen)) {	/* Error decoding pen */
						GMT_pen_syntax (GMT, 'N', " ");
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
				if (opt->arg[0] == 'c' && opt->arg[1] == '\0')
					Ctrl->S.clip = true;
				else if (GMT_getfill (GMT, opt->arg, &Ctrl->S.fill)) {
					GMT_fill_syntax (GMT, 'S', " ");
					n_errors++;
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				n_errors += GMT_getrose (GMT, 'T', opt->arg, &Ctrl->T.rose);
				this_panel = &(Ctrl->T.rose.panel);
				break;
			case 'W':
				Ctrl->W.active = true;	/* Want to draw shorelines */
				if ((opt->arg[0] >= '1' && opt->arg[0] <= '4') && opt->arg[1] == '/') {	/* Specific pen for this feature */
					k = (int)(opt->arg[0] - '1');
					if (opt->arg[2] && GMT_getpen (GMT, &opt->arg[2], &Ctrl->W.pen[k])) {
						GMT_pen_syntax (GMT, 'W', " ");
						n_errors++;
					}
					Ctrl->W.use[k] = true;
				}
				else if (opt->arg[0]) {	/* Same pen for all features */
					Ctrl->W.use[0] = true;
					if (GMT_getpen (GMT, opt->arg, &Ctrl->W.pen[0])) {
						GMT_pen_syntax (GMT, 'W', " ");
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
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (GMT_DCW_list (GMT, Ctrl->E.info.mode)) return 1;

	if (Ctrl->C.active && !(Ctrl->G.active || Ctrl->S.active || Ctrl->W.active)) {	/* Just lakes, fix -A */
		if (Ctrl->A.info.low < 2) Ctrl->A.info.low = 2;
	}
	if (!GMT->common.R.active && Ctrl->E.active && Ctrl->M.active && !Ctrl->E.info.region) Ctrl->E.info.region = true;	/* For -M and -E with no plotting, get -R from pols */
	if (Ctrl->E.info.region) {	/* Must pick up region from chosen polygons */
		if (GMT->common.R.active)
			GMT_Report (API, GMT_MSG_VERBOSE, "Warning -E option: The -R option overrides the region found via -E.\n");
		else {	/* Pick up region from chosen polygons */
			(void) GMT_DCW_operation (GMT, &Ctrl->E.info, GMT->common.R.wesn, GMT_DCW_REGION);
			GMT->common.R.active = true;
			if (!GMT->common.J.active && !Ctrl->M.active) {	/* No plotting or no dumping means just return the -R string */
				char record[GMT_BUFSIZ] = {"-R"}, text[GMT_LEN64] = {""};
				size_t i, j;
				GMT_ascii_format_col (GMT, text, GMT->common.R.wesn[XLO], GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
				GMT_ascii_format_col (GMT, text, GMT->common.R.wesn[XHI], GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
				GMT_ascii_format_col (GMT, text, GMT->common.R.wesn[YLO], GMT_OUT, GMT_Y);	strcat (record, text);	strcat (record, "/");
				GMT_ascii_format_col (GMT, text, GMT->common.R.wesn[YHI], GMT_OUT, GMT_Y);	strcat (record, text);
				/* Remove any white space due to selected formatting */
				for (i = j = 2; i < strlen (record); i++) {
					if (record[i] == ' ') continue;	/* Skip spaces */
					record[j++] = record[i];
				}
				record[j] = '\0';
				if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
					return (API->error);
				}
				if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT, GMT_HEADER_OFF) != GMT_OK) {
					return (API->error);
				}
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write text record to output destination */
				if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
					return (API->error);
				}
				return NOT_REALLY_AN_ERROR;	/* To return with "error" but then exit with 0 error */
			}
		}
	}

	/* Check that the options selected are mutually consistent */

	clipping = (Ctrl->G.clip || Ctrl->S.clip);
	if (Ctrl->M.active) {	/* Need -R only */
		n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	}
	else if (!Ctrl->Q.active) {	/* Need -R -J */
		n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
		n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	}
	for (k = 0; k < GSHHS_MAX_LEVEL; k++) {
		n_errors += GMT_check_condition (GMT, Ctrl->W.pen[k].width < 0.0, "Syntax error -W option: Pen thickness for feature %d cannot be negative\n", k);
	}
	n_errors += GMT_check_condition (GMT, n_files > 0, "Syntax error: No input files allowed\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->G.active + Ctrl->S.active + Ctrl->C.active) > 1 && clipping, "Syntax error: Cannot combine -C, -G, -S while clipping\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.clip && Ctrl->S.clip, "Syntax error: Must choose between clipping land OR water\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && (Ctrl->G.active || Ctrl->S.active || Ctrl->C.active), "Syntax error: Must choose between dumping and clipping/plotting\n");
	n_errors += GMT_check_condition (GMT, clipping && GMT->current.proj.projection == GMT_AZ_EQDIST && fabs (GMT->common.R.wesn[XLO] - GMT->common.R.wesn[XHI]) == 360.0 && (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) == 180.0, "-JE not implemented for global clipping - I quit\n");
	n_errors += GMT_check_condition (GMT, clipping && (Ctrl->N.active || Ctrl->I.active || Ctrl->W.active), "Cannot do clipping AND draw coastlines, rivers, or borders\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->G.active || Ctrl->S.active || Ctrl->C.active || Ctrl->E.active || Ctrl->W.active || Ctrl->N.active || Ctrl->I.active || Ctrl->Q.active), "Syntax error: Must specify at least one of -C, -G, -S, -I, -N, -Q and -W\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && (Ctrl->E.active + Ctrl->N.active + Ctrl->I.active + Ctrl->W.active) != 1, "Syntax error -M: Must specify one of -E, -I, -N, and -W\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.active && !(Ctrl->L.active || Ctrl->T.active), "Syntax error: -F is only allowed with -L and -T\n");

	if (Ctrl->I.active) {	/* Generate list of desired river features in sorted order */
		for (k = Ctrl->I.n_rlevels = 0; k < GSHHS_N_RLEVELS; k++) {
			if (!Ctrl->I.use[k]) continue;
			n_errors += GMT_check_condition (GMT, Ctrl->I.pen[k].width < 0.0, "Syntax error -I option: Pen thickness cannot be negative\n");
			Ctrl->I.list[Ctrl->I.n_rlevels++] = k;	/* Since goes from 0-10 */
		}
	}

	if (Ctrl->N.active) {	/* Generate list of desired border features in sorted order */
		for (k = Ctrl->N.n_blevels = 0; k < GSHHS_N_BLEVELS; k++) {
			if (!Ctrl->N.use[k]) continue;
			n_errors += GMT_check_condition (GMT, Ctrl->N.pen[k].width < 0.0, "Syntax error -N option: Pen thickness cannot be negative\n");
			Ctrl->N.list[Ctrl->N.n_blevels++] = k + 1;	/* Add one so we get range 1-3 */
		}
	}

	return (n_errors);
}

bool add_this_polygon_to_path (struct GMT_CTRL *GMT, int k0, struct GMT_GSHHS_POL *p, int level, int k)
{
	/* Determines if we should add the current polygon pol[k] to the growing path we are constructing */

	int j;
	
	if (p[k].level != level) return (false);	/* Sorry, this polygon does not have the correct level */
	if (k0 == -1) return (true);			/* The start level is always used */
	/* Must make sure the polygon is inside the mother polygon.  Because numerical noise can trick us we try up to two points */
	if (GMT_non_zero_winding (GMT, p[k].lon[0], p[k].lat[0], p[k0].lon, p[k0].lat, p[k0].n)) return (true);	/* OK, we're in! */
	/* First point was not inside.  Test another point to see if the first failure was just an accident */
	j = p[k].n / 2;	/* We pick the second point from the other half of the polygon */
	if (GMT_non_zero_winding (GMT, p[k].lon[j], p[k].lat[j], p[k0].lon, p[k0].lat, p[k0].n)) return (true);	/* One of those rare occasions when we almost missed a polygon */
	return (false);	/* No, we do not want to use this polygon */
}

void recursive_path (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, int k0, int np, struct GMT_GSHHS_POL p[], int level, struct GMT_FILL fill[]) {
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
				GMT_setfill (GMT, &fill[p[k].fid], false);
				PSL_command (PSL, "FO\n");
			}
		}
	}
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_pscoast_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_pscoast (void *V_API, int mode, void *args)
{	/* High-level function that implements the pscoast task */

	int i, np, ind, bin = 0, base, anti_bin = -1, np_new, k, last_k, err, bin_trouble, error, n;
	int level_to_be_painted = 0, direction, start_direction, stop_direction, last_pen_level;
	
	bool shift = false, need_coast_base, recursive;
	bool greenwich = false, possibly_donut_hell = false, fill_in_use = false;
	bool clobber_background, paint_polygons = false, donut;
	bool donut_hell = false, world_map_save, clipping;

	double bin_x[5], bin_y[5], out[2], *xtmp = NULL, *ytmp = NULL;
	double west_border = 0.0, east_border = 0.0, anti_lon = 0.0, anti_lat = -90.0, edge = 720.0;
	double left, right, anti_x = 0.0, anti_y = 0.0, x_0 = 0.0, y_0 = 0.0, x_c, y_c, dist;

	char *shore_resolution[5] = {"full", "high", "intermediate", "low", "crude"};

	struct GMT_FILL fill[6];	/* Colors for (0) water, (1) land, (2) lakes, (3) islands in lakes, (4) lakes in islands in lakes, (5) riverlakes */
	struct GMT_SHORE c;
	struct GMT_BR b, r;
	struct GMT_GSHHS_POL *p = NULL;
	struct PSCOAST_CTRL *Ctrl = NULL;		/* Control structure specific to program */
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_pscoast_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_pscoast_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_pscoast_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_pscoast_Ctrl (GMT);		/* Allocate and initialize defaults in a new control structure */
	if ((error = GMT_pscoast_parse (GMT, Ctrl, options))) {
		if (error == NOT_REALLY_AN_ERROR) Return (0);
		Return (error);
	}

	/*---------------------------- This is the pscoast main code ----------------------------*/

	/* Check and interpret the command line arguments */

	clipping = (Ctrl->G.clip || Ctrl->S.clip);
	if (Ctrl->D.force) Ctrl->D.set = GMT_shore_adjust_res (GMT, Ctrl->D.set);
	fill[0] = Ctrl->S.fill;
	fill[1] = fill[3] = Ctrl->G.fill;
	fill[2] = fill[4] = (Ctrl->C.active) ? Ctrl->C.fill[LAKE] : Ctrl->S.fill;
	fill[5] = (Ctrl->C.active) ? Ctrl->C.fill[RIVER] : fill[2];
	need_coast_base = (Ctrl->G.active || Ctrl->S.active || Ctrl->C.active || Ctrl->W.active);
	if (Ctrl->Q.active) need_coast_base = false;	/* Since we just end clipping */
	clobber_background = (Ctrl->G.active && Ctrl->S.active);
	recursive = (Ctrl->G.active != (Ctrl->S.active || Ctrl->C.active) || clipping);
	paint_polygons = (Ctrl->G.active || Ctrl->S.active || Ctrl->C.active);

	if (GMT->common.R.wesn[XLO] > 360.0) {
		GMT->common.R.wesn[XLO] -= 360.0;
		GMT->common.R.wesn[XHI] -= 360.0;
	}

	if (Ctrl->Q.active && !GMT->common.J.active) {	/* Fake area and linear projection */
		GMT_parse_common_options (GMT, "J", 'J', "x1d");	/* Fake linear projection */
		GMT->common.R.wesn[XLO] = GMT->common.R.wesn[YLO] = 0.0;
		GMT->common.R.wesn[XHI] = GMT->common.R.wesn[YHI] = 1.0;
	}
	else if (Ctrl->M.active && !GMT->common.J.active)
		GMT_parse_common_options (GMT, "J", 'J', "x1d");	/* Fake linear projection */

	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	base = GMT_set_resolution (GMT, &Ctrl->D.set, 'D');

	if (!GMT_is_geographic (GMT, GMT_IN)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: You must use a map projection or -Jx|X...d[/...d] for geographic data\n");
		Return (EXIT_FAILURE);
	}
	
	world_map_save = GMT->current.map.is_world;

	if (need_coast_base && (err = GMT_init_shore (GMT, Ctrl->D.set, &c, GMT->common.R.wesn, &Ctrl->A.info)))  {
		GMT_Report (API, GMT_MSG_NORMAL, "%s [GSHHG %s resolution shorelines]\n", GMT_strerror(err), shore_resolution[base]);
		need_coast_base = false;
	}

	if (Ctrl->N.active && (err = GMT_init_br (GMT, 'b', Ctrl->D.set, &b, GMT->common.R.wesn))) {
		GMT_Report (API, GMT_MSG_NORMAL, "%s [GSHHG %s resolution political boundaries]\n", GMT_strerror(err), shore_resolution[base]);
		Ctrl->N.active = false;
	}
	if (need_coast_base) GMT_Report (API, GMT_MSG_VERBOSE, "GSHHG version %s\n%s\n%s\n", c.version, c.title, c.source);

	if (Ctrl->I.active && (err = GMT_init_br (GMT, 'r', Ctrl->D.set, &r, GMT->common.R.wesn))) {
		GMT_Report (API, GMT_MSG_NORMAL, "%s [GSHHG %s resolution rivers]\n", GMT_strerror(err), shore_resolution[base]);
		Ctrl->I.active = false;
	}

	if (!(need_coast_base || Ctrl->E.active || Ctrl->N.active || Ctrl->I.active || Ctrl->Q.active)) {
		GMT_Report (API, GMT_MSG_NORMAL, "No GSHHG databases available - must abort\n");
		Return (EXIT_FAILURE);
	}

	if (Ctrl->M.active) {	/* Dump linesegments to stdout; no plotting takes place */
		int id = 0;
		char header[GMT_BUFSIZ] = {""}, *kind[3] = {"Coastlines", "Political boundaries", "Rivers"}, *version = NULL, *title = NULL, *source = NULL;
		GMT_set_geographic (GMT, GMT_OUT);	/* Output lon/lat */
		if (Ctrl->N.active) id = 1;	if (Ctrl->I.active) id = 2;
		GMT_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
		if ((error = GMT_set_cols (GMT, GMT_OUT, 2)) != GMT_OK) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
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
			sprintf (header, "# Country polygons extracted from the DCW database\n");
			GMT_Put_Record (API, GMT_WRITE_TEXT, header);
		}
		else {
			sprintf (header, "# %s extracted from the %s resolution GSHHG version %s database\n", kind[id], shore_resolution[base], version);
			GMT_Put_Record (API, GMT_WRITE_TEXT, header);
			sprintf (header, "# %s\n# %s\n", title, source);
			GMT_Put_Record (API, GMT_WRITE_TEXT, header);
		}
	}
	else {
		if (Ctrl->Q.active)
			GMT->current.ps.nclip = -1;	/* Signal that this program terminates polygon clipping that initiated prior to this process */
		else if (clipping)
			GMT->current.ps.nclip = +1;	/* Signal that this program initiates new clipping that will outlive this process */

		PSL = GMT_plotinit (GMT, options);

		if (Ctrl->Q.active) {  /* Just undo previous clip-path */
			PSL_endclipping (PSL, 1);

			GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
			GMT_plotcanvas (GMT);	/* Fill canvas if requested */
			GMT_map_basemap (GMT); /* Basemap needed */
			GMT_plane_perspective (GMT, -1, 0.0);
	
			GMT_plotend (GMT);

			GMT_Report (API, GMT_MSG_VERBOSE, "Done!\n");

			Return (GMT_OK);
		}

		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT_plotcanvas (GMT);	/* Fill canvas if requested */
	}

	for (i = 0; i < 5; i++) if (fill[i].use_pattern) fill_in_use = true;

	if (fill_in_use && !clobber_background) {	/* Force clobber when fill is used since our routine cannot deal with clipped fills */
		GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Pattern fill requires oceans to be painted first\n");
		clobber_background = true;
		recursive = false;
	}

	if (clipping) GMT_map_basemap (GMT);

	if (GMT->current.proj.projection == GMT_AZ_EQDIST && GMT_360_RANGE (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]) && GMT_180_RANGE (GMT->common.R.wesn[YHI], GMT->common.R.wesn[YLO])) {
		possibly_donut_hell = true;
		anti_lon = GMT->current.proj.central_meridian + 180.0;
		if (anti_lon >= 360.0) anti_lon -= 360.0;
		anti_lat = -GMT->current.proj.pole;
		anti_bin = irint (floor ((90.0 - anti_lat) / c.bsize)) * c.bin_nx + irint (floor (anti_lon / c.bsize));
		GMT_geo_to_xy (GMT, anti_lon, anti_lat, &anti_x, &anti_y);
		GMT_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &x_0, &y_0);
		if (Ctrl->G.active) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Fill/clip continent option (-G) may not work for this projection.\nIf the antipole (%g/%g) is in the ocean then chances are good\nElse: avoid projection center coordinates that are exact multiples of %g degrees\n", anti_lon, anti_lat, c.bsize);
	}

	if (possibly_donut_hell && paint_polygons && !clobber_background) {	/* Force clobber when donuts may be called for now */
		GMT_Report (API, GMT_MSG_VERBOSE, "Warning: -JE requires oceans to be painted first\n");
		clobber_background = true;
		recursive = false;
	}

	if (clobber_background) {	/* Paint entire map as ocean first, then lay land on top */
		n = (int)GMT_map_clip_path (GMT, &xtmp, &ytmp, &donut);
		GMT_setfill (GMT, &Ctrl->S.fill, false);
		if (donut) {
			/* If donut, then the path consists of two path of np points */
			PSL_plotline (PSL, xtmp, ytmp, n, PSL_MOVE + PSL_CLOSE);
			PSL_plotpolygon (PSL, &xtmp[n], &ytmp[n], n);
		}
		else
			PSL_plotpolygon (PSL, xtmp, ytmp, n);
		GMT_free (GMT, xtmp);
		GMT_free (GMT, ytmp);
		level_to_be_painted = 1;
	}
	if (recursive) {
		start_direction = stop_direction = (Ctrl->G.active) ? 1 : -1;
		level_to_be_painted = (Ctrl->G.active) ? 1 : 0;
	}
	else {
		start_direction = -1;
		stop_direction = 1;
		level_to_be_painted = (Ctrl->S.active) ? 0 : 1;
	}

	if (GMT->common.R.wesn[XLO] < 0.0 && GMT->common.R.wesn[XHI] > 0.0 && !GMT_IS_LINEAR (GMT)) greenwich = true;
	if (need_coast_base && (360.0 - fabs (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]) ) < c.bsize) {
		GMT->current.map.is_world = true;
		if (GMT->current.proj.projection == GMT_GNOMONIC || GMT->current.proj.projection == GMT_GENPER) GMT->current.map.is_world = false;
		if (GMT_IS_AZIMUTHAL (GMT)) GMT->current.map.is_world = false;
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

	if (!Ctrl->M.active && Ctrl->W.active) GMT_setpen (GMT, &Ctrl->W.pen[0]);

	last_pen_level = -1;

	if (clipping) PSL_beginclipping (PSL, xtmp, ytmp, 0, GMT->session.no_rgb, 1);	/* Start clippath */

	for (ind = 0; need_coast_base && ind < c.nb; ind++) {	/* Loop over necessary bins only */

		bin = c.bins[ind];
#ifdef DEBUG
		if (Ctrl->debug.active && bin != Ctrl->debug.bin) continue;
#endif
		if ((err = GMT_get_shore_bin (GMT, ind, &c))) {
			GMT_Report (API, GMT_MSG_NORMAL, "%s [%s resolution shoreline]\n", GMT_strerror(err), shore_resolution[base]);
			Return (EXIT_FAILURE);
		}

		GMT_Report (API, GMT_MSG_VERBOSE, "Working on bin # %5d\r", bin);
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
			GMT_geo_to_xy (GMT, c.lon_sw + 0.5 * c.bsize, c.lat_sw + 0.5 * c.bsize, &x_c, &y_c);
			dist = hypot (x_c - x_0, y_c - y_0);	/* Distance from projection center to mid-point of current bin */
			/* State donut_hell if we are close (within 20%) to the horizon or if the bin contains the antipole.
			 * This is simply done with a Cartesian inside/outside test, which is adequate.
			 * the 0.8 factor is arbitrary of course [PW] */
			
			donut_hell = (dist > 0.8 * GMT->current.proj.r || GMT_non_zero_winding (GMT, anti_lon, anti_lat, bin_x, bin_y, 5));
		}

		for (direction = start_direction; paint_polygons && direction <= stop_direction; direction += 2) {

			/* Assemble one or more segments into polygons */

			if ((np = GMT_assemble_shore (GMT, &c, direction, true, west_border, east_border, &p)) == 0) continue;

			/* Get clipped polygons in x,y inches that can be plotted */

			np_new = GMT_prep_shore_polygons (GMT, &p, np, donut_hell, 0.0, bin_trouble);

			if (clipping) {
				for (k = level_to_be_painted; k < GSHHS_MAX_LEVEL - 1; k++) recursive_path (GMT, PSL, -1, np_new, p, k, NULL);

				for (k = 0; k < np_new; k++) {	/* Do any remaining interior polygons */
					if (p[k].n == 0) continue;
					if (p[k].level % 2 == level_to_be_painted || p[k].level > 2) {
						PSL_plotline (PSL, p[k].lon, p[k].lat, p[k].n, PSL_MOVE);
					}
				}
			}
			else if (recursive) {	/* Must avoid pointing anything but the polygons inside */

				for (k = level_to_be_painted; k < GSHHS_MAX_LEVEL - 1; k++) recursive_path (GMT, PSL, -1, np_new, p, k, fill);

				for (k = 0; k < np_new; k++) {	/* Do any remaining interior polygons */
					if (p[k].n == 0) continue;
					if (p[k].level % 2 == level_to_be_painted || p[k].level > 2) {
						GMT_setfill (GMT, &fill[p[k].fid], false);
						PSL_plotpolygon (PSL, p[k].lon, p[k].lat, p[k].n);
					}
				}
			}
			else {	/* Simply paints all polygons as is */
				for (k = 0; k < np_new; k++) {
					if (p[k].n == 0 || p[k].level < level_to_be_painted) continue;
					if (donut_hell && GMT_non_zero_winding (GMT, anti_x, anti_y, p[k].lon, p[k].lat, p[k].n)) {	/* Antipode inside polygon, must do donut */
						n = (int)GMT_map_clip_path (GMT, &xtmp, &ytmp, &donut);
						GMT_setfill (GMT, &fill[p[k].fid], false);
						PSL_plotline (PSL, xtmp, ytmp, n, PSL_MOVE + PSL_CLOSE);
						PSL_plotpolygon (PSL, p[k].lon, p[k].lat, p[k].n);
						GMT_free (GMT, xtmp);
						GMT_free (GMT, ytmp);
					}
					else {
						GMT_setfill (GMT, &fill[p[k].fid], false);
						PSL_plotpolygon (PSL, p[k].lon, p[k].lat, p[k].n);
					}
				}
			}

			GMT_free_shore_polygons (GMT, p, np_new);
			GMT_free (GMT, p);
		}

		if (Ctrl->W.active && c.ns) {	/* Draw or dump shorelines, no need to assemble polygons */
			if ((np = GMT_assemble_shore (GMT, &c, 1, false, west_border, east_border, &p)) == 0) continue;

			for (i = 0; i < np; i++) {
				if (Ctrl->M.active) {
					sprintf (GMT->current.io.segment_header, "Shore Bin # %d, Level %d", bin, p[i].level);
					GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
					for (k = 0; k < p[i].n; k++) {
						out[GMT_X] = p[i].lon[k];
						out[GMT_Y] = p[i].lat[k];
						GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
					}
				}
				else if (Ctrl->W.use[p[i].level-1]) {
					if (donut_hell) p[i].n = (int)GMT_fix_up_path (GMT, &p[i].lon, &p[i].lat, p[i].n, 0.0, 0);
					GMT->current.plot.n = GMT_geo_to_xy_line (GMT, p[i].lon, p[i].lat, p[i].n);
					if (!GMT->current.plot.n) continue;

					k = p[i].level - 1;
					if (k != last_pen_level) {
						GMT_setpen (GMT, &Ctrl->W.pen[k]);
						last_pen_level = k;
					}
					GMT_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n);
				}
			}

			GMT_free_shore_polygons (GMT, p, np);
			GMT_free (GMT, p);
		}

		GMT_free_shore (GMT, &c);

	}
	if (need_coast_base) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Working on bin # %5ld\n", bin);
		GMT_shore_cleanup (GMT, &c);
	}

	(void)GMT_DCW_operation (GMT, &Ctrl->E.info, NULL, Ctrl->M.active ? GMT_DCW_DUMP : GMT_DCW_PLOT);

	if (clipping) PSL_beginclipping (PSL, xtmp, ytmp, 0, GMT->session.no_rgb, 2);	/* End clippath */

	if (Ctrl->I.active) {	/* Read rivers file and plot as lines */

		GMT_Report (API, GMT_MSG_VERBOSE, "Adding Rivers...");
		if (!Ctrl->M.active) PSL_comment (PSL, "Start of River segments\n");
		last_k = -1;

		for (ind = 0; ind < r.nb; ind++) {	/* Loop over necessary bins only */

			bin = r.bins[ind];
			GMT_get_br_bin (GMT, ind, &r, Ctrl->I.list, Ctrl->I.n_rlevels);

			if (r.ns == 0) continue;

			if (GMT->current.map.is_world && greenwich) {
				left = r.bsize * (bin % r.bin_nx);	right = left + r.bsize;
				shift = ((left - edge) <= 180.0 && (right - edge) > 180.0);
			}

			if ((np = GMT_assemble_br (GMT, &r, shift, edge, &p)) == 0) {
				GMT_free_br (GMT, &r);
				continue;
			}

			for (i = 0; i < np; i++) {
				if (Ctrl->M.active) {
					sprintf (GMT->current.io.segment_header, "River Bin # %d, Level %d", bin, p[i].level);
					GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
					for (k = 0; k < p[i].n; k++) {
						out[GMT_X] = p[i].lon[k];
						out[GMT_Y] = p[i].lat[k];
						GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
					}
				}
				else {
					GMT->current.plot.n = GMT_geo_to_xy_line (GMT, p[i].lon, p[i].lat, p[i].n);
					if (!GMT->current.plot.n) continue;

					k = p[i].level;
					if (k != last_k) {
						GMT_setpen (GMT, &Ctrl->I.pen[k]);
						last_k = k;
					}
					GMT_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n);
				}
			}

			/* Free up memory */

			GMT_free_br (GMT, &r);
			GMT_free_shore_polygons (GMT, p, np);
			GMT_free (GMT, p);
		}
		GMT_br_cleanup (GMT, &r);
	}

	if (Ctrl->N.active) {	/* Read borders file and plot as lines */
		double step;
		
		GMT_Report (API, GMT_MSG_VERBOSE, "Adding Borders...");
		if (!Ctrl->M.active) PSL_comment (PSL, "Start of Border segments\n");

		/* Must resample borders because some points may be too far apart and look like 'jumps' */

		step = MAX (fabs(GMT->common.R.wesn[XLO] - GMT->common.R.wesn[XHI]), fabs (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO])) / 4.0;

		last_k = -1;

		for (ind = 0; ind < b.nb; ind++) {	/* Loop over necessary bins only */

			bin = b.bins[ind];
			GMT_get_br_bin (GMT, ind, &b, Ctrl->N.list, Ctrl->N.n_blevels);

			if (b.ns == 0) continue;

			if (GMT->current.map.is_world && greenwich) {
				left = b.bsize * (bin % b.bin_nx);	right = left + b.bsize;
				shift = ((left - edge) <= 180.0 && (right - edge) > 180.0);
			}

			if ((np = GMT_assemble_br (GMT, &b, shift, edge, &p)) == 0) {
				GMT_free_br (GMT, &b);
				continue;
			}

			for (i = 0; i < np; i++) {
				if (Ctrl->M.active) {
					sprintf (GMT->current.io.segment_header, "Border Bin # %d, Level %d", bin, p[i].level);
					GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
					for (k = 0; k < p[i].n; k++) {
						out[GMT_X] = p[i].lon[k];
						out[GMT_Y] = p[i].lat[k];
						GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
					}
				}
				else {
					p[i].n = (int)GMT_fix_up_path (GMT, &p[i].lon, &p[i].lat, p[i].n, step, 0);
					GMT->current.plot.n = GMT_geo_to_xy_line (GMT, p[i].lon, p[i].lat, p[i].n);
					if (!GMT->current.plot.n) continue;

					k = p[i].level - 1;
					if (k != last_k) {
						GMT_setpen (GMT, &Ctrl->N.pen[k]);
						last_k = k;
					}
					GMT_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n);
				}
			}

			/* Free up memory */

			GMT_free_br (GMT, &b);
			GMT_free_shore_polygons (GMT, p, np);
			GMT_free (GMT, p);
		}
		GMT_br_cleanup (GMT, &b);
	}

	if (!Ctrl->M.active) {
		if (GMT->current.map.frame.init) {
			GMT->current.map.is_world = world_map_save;
			GMT_map_basemap (GMT);
		}

		if (Ctrl->L.active) GMT_draw_map_scale (GMT, &Ctrl->L.scale);
		if (Ctrl->T.active) GMT_draw_map_rose (GMT, &Ctrl->T.rose);

		GMT_plane_perspective (GMT, -1, 0.0);
		GMT_plotend (GMT);
	}
	else if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {
		Return (API->error);	/* Disables further data output */
	}
	
	GMT->current.map.coastline = false;

	GMT_Report (API, GMT_MSG_VERBOSE, "Done\n");

	Return (GMT_OK);
}
