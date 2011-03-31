/*--------------------------------------------------------------------
 *	$Id: pscoast_func.c,v 1.5 2011-03-31 23:03:21 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
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

#include "pslib.h"
#include "gmt.h"

#define LAKE	0
#define RIVER	1

/* Control structure for pscoast */

struct PSCOAST_CTRL {
	struct A {	/* -A<min_area>[/<min_level>/<max_level>] */
		GMT_LONG active;
		struct GMT_SHORE_SELECT info;
	} A;
	struct C {	/* -C<fill> */
		GMT_LONG active;
		struct GMT_FILL fill[2];	/* lake and riverlake fill */
	} C;
	struct D {	/* -D<resolution> */
		GMT_LONG active;
		GMT_LONG force;	/* if TRUE, select next highest level if current set is not avaialble */
		char set;	/* One of f, h, i, l, c */
	} D;
	struct G {	/* -G<fill> */
		GMT_LONG active;
		GMT_LONG clip;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I<feature>[/<pen>] */
		GMT_LONG active;
		GMT_LONG use[GMT_N_RLEVELS], n_rlevels;
		struct GMT_PEN pen[GMT_N_RLEVELS];
	} I;
	struct L {	/* -L */
		GMT_LONG active;
		struct GMT_MAP_SCALE item;
	} L;
	struct M {	/* -M */
		GMT_LONG active;
	} M;
	struct N {	/* -N<feature>[/<pen>] */
		GMT_LONG active;
		GMT_LONG use[GMT_N_BLEVELS], n_blevels;
		struct GMT_PEN pen[GMT_N_BLEVELS];
	} N;
	struct Q {	/* -Q */
		GMT_LONG active;
	} Q;
	struct S {	/* -S<fill> */
		GMT_LONG active;
		GMT_LONG clip;
		struct GMT_FILL fill;
	} S;
	struct T {	/* -L */
		GMT_LONG active;
		struct GMT_MAP_ROSE item;
	} T;
	struct W {	/* -W[<feature>/]<pen> */
		GMT_LONG active;
		GMT_LONG use[GMT_MAX_GSHHS_LEVEL];
		struct GMT_PEN pen[GMT_MAX_GSHHS_LEVEL];
	} W;
#ifdef DEBUG
	struct DBG {	/* -+<bin> */
		GMT_LONG active;
		int bin;
	} debug;
#endif
};

void *New_pscoast_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	GMT_LONG k;
	struct PSCOAST_CTRL *C = GMT_memory (GMT, NULL, 1, struct PSCOAST_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->A.info.high = GMT_MAX_GSHHS_LEVEL;			/* Include all GSHHS levels */
	C->D.set = 'l';						/* Low-resolution coastline data */
	GMT_init_fill (GMT, &C->S.fill, 1.0, 1.0, 1.0);		/* Default Ocean color */
	GMT_init_fill (GMT, &C->G.fill, 0.0, 0.0, 0.0);		/* Default Land color */
	GMT_init_fill (GMT, &C->C.fill[LAKE], 1.0, 1.0, 1.0);	/* Default Lake color */
	GMT_init_fill (GMT, &C->C.fill[RIVER], 1.0, 1.0, 1.0);	/* Default Riverlake color */
	for (k = 0; k < GMT_N_RLEVELS; k++) C->I.pen[k] = GMT->current.setting.map_default_pen;		/* Default river pens */
	for (k = 0; k < GMT_N_BLEVELS; k++) C->N.pen[k] = GMT->current.setting.map_default_pen;		/* Default border pens */
	for (k = 0; k < GMT_MAX_GSHHS_LEVEL; k++) C->W.pen[k] = GMT->current.setting.map_default_pen;	/* Default coastline pens */
	GMT_memset (&C->L.item, 1, struct GMT_MAP_SCALE);
	GMT_memset (&C->T.item, 1, struct GMT_MAP_ROSE);

	return ((void *)C);
}

void Free_pscoast_Ctrl (struct GMT_CTRL *GMT, struct PSCOAST_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);
}

GMT_LONG GMT_pscoast_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the pscoast synopsis and optionally full usage information */

	GMT_message (GMT, "pscoast %s [API] - Plot continents, shorelines, rivers, and borders on maps\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: pscoast %s %s [%s] [%s]\n", GMT_B_OPT, GMT_J_OPT, GMT_A_OPT, GMT_Rgeoz_OPT);
	GMT_message (GMT, "\t[-C[<feature>/]<fill>] [-D<resolution>][+] [-G[<fill>]] [-I<feature>[/<pen>]]\n");
	GMT_message (GMT, "\t[%s] [-K] [%s]\n", GMT_Jz_OPT, GMT_SCALE);
	GMT_message (GMT, "\t[-M] [-N<feature>[/<pen>]] [-O] [-P] [-Q] [-S<fill>]\n");
	GMT_message (GMT, "\t[%s] [%s]\n", GMT_TROSE, GMT_U_OPT);
	GMT_message (GMT, "\t[%s] [-W[<feature>/][<pen>]] [%s] [%s]\n", GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s]\n", GMT_bo_OPT, GMT_c_OPT, GMT_p_OPT, GMT_colon_OPT);
#ifdef DEBUG
	GMT_message (GMT, "\t[-+<bin>]\n");
#endif

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_explain_options (GMT, "jZR");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_GSHHS_syntax (GMT, 'A', "Place limits on coastline features from the GSHHS data base.");
	GMT_explain_options (GMT, "b");
	GMT_fill_syntax (GMT, 'C', "Sets separate color for lakes and riverlakes [Default is same as ocean].");
	GMT_message (GMT, "\t   Alternatively, set custom fills below.  Repeat the -C option as needed\n");
	GMT_message (GMT, "\t      l = Lakes\n");
	GMT_message (GMT, "\t      r = River-lakes\n");
	GMT_message (GMT, "\t-D Choose one of the following resolutions:\n");
	GMT_message (GMT, "\t   f - full resolution (may be very slow for large regions)\n");
	GMT_message (GMT, "\t   h - high resolution (may be slow for large regions)\n");
	GMT_message (GMT, "\t   i - intermediate resolution\n");
	GMT_message (GMT, "\t   l - low resolution [Default]\n");
	GMT_message (GMT, "\t   c - crude resolution, for busy plots that need crude continent outlines only\n");
	GMT_message (GMT, "\t   Append + to use a lower resolution should the chosen one not be available [abort].\n");
	GMT_fill_syntax (GMT, 'G', "Paint or clip \"dry\" areas.");
	GMT_message (GMT, "\t   6) c to issue clip paths for land areas.\n");
	GMT_pen_syntax (GMT, 'I', "draws rivers.  Specify feature and optionally append pen [Default for all levels: %s].");
	GMT_message (GMT, "\t   Choose from the features below.  Repeat the -I option as many times as needed\n");
	GMT_message (GMT, "\t      0 = Double-lined rivers (river-lakes)\n");
	GMT_message (GMT, "\t      1 = Permanent major rivers\n");
	GMT_message (GMT, "\t      2 = Additional major rivers\n");
	GMT_message (GMT, "\t      3 = Additional rivers\n");
	GMT_message (GMT, "\t      4 = Minor rivers\n");
	GMT_message (GMT, "\t      5 = Intermittent rivers - major\n");
	GMT_message (GMT, "\t      6 = Intermittent rivers - additional\n");
	GMT_message (GMT, "\t      7 = Intermittent rivers - minor\n");
	GMT_message (GMT, "\t      8 = Major canals\n");
	GMT_message (GMT, "\t      9 = Minor canals\n");
	GMT_message (GMT, "\t     10 = Irrigation canals\n");
	GMT_message (GMT, "\t      a = All rivers and canals (0-10)\n");
	GMT_message (GMT, "\t      r = All permanent rivers (0-4)\n");
	GMT_message (GMT, "\t      i = All intermittent rivers (5-7)\n");
	GMT_message (GMT, "\t      c = All canals (8-10)\n");
	GMT_explain_options (GMT, "K");
	GMT_mapscale_syntax (GMT, 'L', "Draws a simple map scale centered on <lon0>/<lat0>");
	GMT_message (GMT, "\t-M Dump a multisegment ascii (or binary, see -bo) file to standard output.  No plotting occurs\n");
	GMT_message (GMT, "\t   Specify any combination of -W, -I, -N\n");
	GMT_pen_syntax (GMT, 'N', "draws boundaries.  Specify feature and optionally append pen [Default for all levels: %s].");
	GMT_message (GMT, "\t   Choose from the features below.  Repeat the -N option as many times as needed\n");
	GMT_message (GMT, "\t     1 = National boundaries\n");
	GMT_message (GMT, "\t     2 = State boundaries within the Americas\n");
	GMT_message (GMT, "\t     3 = Marine boundaries\n");
	GMT_message (GMT, "\t     a = All boundaries (1-3)\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-Q means terminate previously set clip-paths\n");
	GMT_fill_syntax (GMT, 'S', "Paint of clip \"wet\" areas.");
	GMT_message (GMT, "\t   6) c to issue clip paths for water areas.\n");
	GMT_maprose_syntax (GMT, 'T', "Draws a north-pointing map rose centered on <lon0>/<lat0>");
	GMT_explain_options (GMT, "UV");
	GMT_pen_syntax (GMT, 'W', "draw shorelines.  Append pen [Default for all levels: %s].");
	GMT_message (GMT, "\t   Alternatively, set custom pens below.  Repeat the -W option as many times as needed\n");
	GMT_message (GMT, "\t      1 = Coastline\n");
	GMT_message (GMT, "\t      2 = Lake shores\n");
	GMT_message (GMT, "\t      3 = Island in lakes shores\n");
	GMT_message (GMT, "\t      4 = Lake in island in lake shores\n");
	GMT_message (GMT, "\t   When feature-specific pens are used, those not set are deactivated\n");
	GMT_explain_options (GMT, "XC0cpt");
#ifdef DEBUG
	GMT_message (GMT, "\t-+ Print only a single bin (debug option)\n");
#endif
	GMT_explain_options (GMT, ".");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_pscoast_parse (struct GMTAPI_CTRL *C, struct PSCOAST_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pscoast and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, k, clipping;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;
	struct GMT_PEN pen;
	char *string = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = TRUE;
				GMT_set_levels (GMT, opt->arg, &Ctrl->A.info);
				break;
			case 'C':	/* Lake colors */
				Ctrl->C.active = TRUE;
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
				Ctrl->D.active = TRUE;
				Ctrl->D.set = (opt->arg[0]) ? opt->arg[0] : 'l';
				Ctrl->D.force = (opt->arg[1] == '+');
				break;
			case 'G':		/* Set Gray shade, pattern, or clipping */
				Ctrl->G.active = TRUE;
				if (opt->arg[0] == 'c' && !opt->arg[1])
					Ctrl->G.clip = TRUE;
				else if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (!opt->arg[0]) {
					GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR:  -I option takes at least one argument\n");
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
						for (k = 0; k < GMT_N_RLEVELS; k++) Ctrl->I.use[k] = TRUE, Ctrl->I.pen[k] = pen;
						break;
					case 'r':
						for (k = 0; k < GMT_RIV_INTERMITTENT; k++) Ctrl->I.use[k] = TRUE, Ctrl->I.pen[k] = pen;
						break;
					case 'i':
						for (k = GMT_RIV_INTERMITTENT; k < GMT_RIV_CANALS; k++) Ctrl->I.use[k] = TRUE, Ctrl->I.pen[k] = pen;
						break;
					case 'c':
						for (k = GMT_RIV_CANALS; k < GMT_N_RLEVELS; k++) Ctrl->I.use[k] = TRUE, Ctrl->I.pen[k] = pen;
						break;
					default:
						k = atoi (opt->arg);
						if (k < 0 || k >= GMT_N_RLEVELS) {
							GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR -I option: Feature not in list!\n");
							n_errors++;
						}
						else
							Ctrl->I.use[k] = TRUE, Ctrl->I.pen[k] = pen;
						break;
				}
				break;
			case 'L':
				Ctrl->L.active = TRUE;
				n_errors += GMT_getscale (GMT, opt->arg, &Ctrl->L.item);
				break;
#ifdef GMT_COMPAT
			case 'm':
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -m option is deprecated and reverted back to -M.\n");
#endif
			case 'M':
				Ctrl->M.active = TRUE;
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				if (!opt->arg[0]) {
					GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR:  -N option takes at least one argument\n");
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
						for (k = 0; k < GMT_N_BLEVELS; k++) Ctrl->N.use[k] = TRUE, Ctrl->N.pen[k] = pen;
						break;
					default:
						k = opt->arg[0] - '1';
						if (k < 0 || k >= GMT_N_BLEVELS) {
							GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR -N option: Feature not in list!\n");
							n_errors++;
						}
						else
							Ctrl->N.use[k] = TRUE, Ctrl->N.pen[k] = pen;
						break;
				}
				break;
			case 'Q':
				Ctrl->Q.active = TRUE;
				break;
			case 'S':		/* Set ocean color if needed */
				Ctrl->S.active = TRUE;
				if (opt->arg[0] == 'c' && opt->arg[1] == '\0')
					Ctrl->S.clip = TRUE;
				else if (GMT_getfill (GMT, opt->arg, &Ctrl->S.fill)) {
					GMT_fill_syntax (GMT, 'S', " ");
					n_errors++;
				}
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				n_errors += GMT_getrose (GMT, opt->arg, &Ctrl->T.item);
				break;
			case 'W':
				Ctrl->W.active = TRUE;	/* Want to draw shorelines */
				if ((opt->arg[0] >= '1' && opt->arg[0] <= '4') && opt->arg[1] == '/') {	/* Specific pen for this feature */
					k = (GMT_LONG)(opt->arg[0] - '1');
					if (opt->arg[2] && GMT_getpen (GMT, &opt->arg[2], &Ctrl->W.pen[k])) {
						GMT_pen_syntax (GMT, 'W', " ");
						n_errors++;
					}
					Ctrl->W.use[k] = TRUE;
				}
				else if (opt->arg[0]) {	/* Same pen for all features */
					Ctrl->W.use[0] = TRUE;
					if (GMT_getpen (GMT, opt->arg, &Ctrl->W.pen[0])) {
						GMT_pen_syntax (GMT, 'W', " ");
						n_errors++;
					}
					else {
						for (k = 1; k < GMT_MAX_GSHHS_LEVEL; k++) Ctrl->W.pen[k] = Ctrl->W.pen[0], Ctrl->W.use[k] = TRUE;
					}
				}
				else {	/* Accept default pen for all features */
					for (k = 0; k < GMT_MAX_GSHHS_LEVEL; k++) Ctrl->W.use[k] = TRUE;
				}
				break;
#ifdef DEBUG
			case '+':
				Ctrl->debug.active = TRUE;
				Ctrl->debug.bin = atoi (opt->arg);
				break;
#endif

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->C.active && !(Ctrl->G.active || Ctrl->S.active || Ctrl->W.active)) {	/* Just lakes, fix -A */
		if (Ctrl->A.info.low < 2) Ctrl->A.info.low = 2;
	}

	/* Check that the options selected are mutually consistent */

	clipping = (Ctrl->G.clip || Ctrl->S.clip);
	if (Ctrl->M.active) {	/* Need -R only */
		n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "GMT SYNTAX ERROR:  Must specify -R option\n");
	}
	else if (!Ctrl->Q.active) {	/* Need -R -J */
		n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "GMT SYNTAX ERROR:  Must specify -R option\n");
		n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "GMT SYNTAX ERROR:  Must specify a map projection with the -J option\n");
	}
	for (k = 0; k < GMT_MAX_GSHHS_LEVEL; k++) {
		n_errors += GMT_check_condition (GMT, Ctrl->W.pen[k].width < 0.0, "GMT SYNTAX ERROR -W option:  Pen thickness for feature %ld cannot be negative\n", k);
	}
	n_errors += GMT_check_condition (GMT, !(Ctrl->G.active || Ctrl->S.active || Ctrl->C.active || Ctrl->W.active || Ctrl->N.active || Ctrl->I.active || Ctrl->Q.active), "GMT SYNTAX ERROR:  Must specify at least one of -C, -G, -S, -I, -N, -Q and -W\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->G.active + Ctrl->S.active + Ctrl->C.active) > 1 && clipping, "GMT SYNTAX ERROR:  Cannot combine -C, -G, -S while clipping\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.clip && Ctrl->S.clip, "GMT SYNTAX ERROR:  Must choose between clipping land OR water\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && (Ctrl->G.active || Ctrl->S.active || Ctrl->C.active), "GMT SYNTAX ERROR:  Must choose between dumping and clipping/plotting\n");
	n_errors += GMT_check_condition (GMT, clipping && GMT->current.proj.projection == GMT_AZ_EQDIST && fabs (GMT->common.R.wesn[XLO] - GMT->common.R.wesn[XHI]) == 360.0 && (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) == 180.0, "-JE not implemented for global clipping - I quit\n");
	n_errors += GMT_check_condition (GMT, clipping && (Ctrl->N.active || Ctrl->I.active || Ctrl->W.active), "Cannot do clipping AND draw coastlines, rivers, or borders\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && (Ctrl->N.active + Ctrl->I.active + Ctrl->W.active) != 1, "GMT SYNTAX ERROR -M:  Must specify one of -I, -N, and -W\n");

	if (Ctrl->I.active) {
		for (k = Ctrl->I.n_rlevels = 0; k < GMT_N_RLEVELS; k++) {
			if (!Ctrl->I.use[k]) continue;
			n_errors += GMT_check_condition (GMT, Ctrl->I.pen[k].width < 0.0, "GMT SYNTAX ERROR -I option:  Pen thickness cannot be negative\n");
			Ctrl->I.use[Ctrl->I.n_rlevels] = k;
			Ctrl->I.pen[Ctrl->I.n_rlevels] = Ctrl->I.pen[k];
			Ctrl->I.n_rlevels++;
		}
	}

	if (Ctrl->N.active) {
		for (k = Ctrl->N.n_blevels = 0; k < GMT_N_BLEVELS; k++) {
			if (!Ctrl->N.use[k]) continue;
			n_errors += GMT_check_condition (GMT, Ctrl->N.pen[k].width < 0.0, "GMT SYNTAX ERROR -N option:  Pen thickness cannot be negative\n");
			Ctrl->N.use[Ctrl->N.n_blevels] = k + 1;
			Ctrl->N.pen[Ctrl->N.n_blevels] = Ctrl->N.pen[k];
			Ctrl->N.n_blevels++;
		}
	}

	return (n_errors);
}

GMT_LONG add_this_polygon_to_path (struct GMT_CTRL *GMT, GMT_LONG k0, struct GMT_GSHHS_POL *p, GMT_LONG level, GMT_LONG k)
{
	/* Determines if we should add the current polygon pol[k] to the growing path we are constructing */

	GMT_LONG j;
	
	if (p[k].level != level) return (FALSE);	/* Sorry, this polygon does not have the correct level */
	if (k0 == -1) return (TRUE);			/* The start level is always used */
	/* Must make sure the polygon is inside the mother polygon.  Because numerical noise can trick us we try up to two points */
	if (GMT_non_zero_winding (GMT, p[k].lon[0], p[k].lat[0], p[k0].lon, p[k0].lat, p[k0].n)) return (TRUE);	/* OK, we're in! */
	/* First point was not inside.  Test another point to see if the first failure was just an accident */
	j = p[k].n / 2;	/* We pick the second point from the other half of the polygon */
	if (GMT_non_zero_winding (GMT, p[k].lon[j], p[k].lat[j], p[k0].lon, p[k0].lat, p[k0].n)) return (TRUE);	/* One of those rare occasions when we almost missed a polygon */
	return (FALSE);	/* No, we do not want to use this polygon */
}

void recursive_path (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, GMT_LONG k0, GMT_LONG np, struct GMT_GSHHS_POL p[], GMT_LONG level, struct GMT_FILL fill[]) {
	/* To avoid painting where no paint should go we assemble all the swiss cheese polygons by starting
	 * with the lowest level (0) and looking for polygons inside polygons of the same level.  We do this
	 * using a recursive algorithm */

	GMT_LONG k;
	
	if (level > GMT_MAX_GSHHS_LEVEL) return;
	for (k = k0 + 1; k < np; k++) {
		if (p[k].n == 0 || p[k].level < level) continue;
		if (add_this_polygon_to_path (GMT, k0, p, level, k)) {	/* Add this to the current path */
			PSL_comment (PSL, "Polygon %ld\n", k);
			PSL_plotline (PSL, p[k].lon, p[k].lat, p[k].n, PSL_MOVE);
#ifdef DEBUGX
			GMT_message (GMT, "#\n");
			for (i = 0; i < p[k].n; i++) GMT_message (GMT, "%g\t%g\n", p[k].lon[i], p[k].lat[i]);
#endif
			recursive_path (GMT, PSL, k, np, p, level + 1, fill);
			p[k].n = 0;	/* Mark as used */

			if (k0 == -1 && fill) {	/* At start level: done nesting, time to paint the assembled swiss cheese polygon */
				GMT_setfill (GMT, PSL, &fill[p[k].fid], FALSE);
				PSL_command (PSL, "fs os\n");
			}
		}
	}
}

#define Return(code) {Free_pscoast_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_pscoast (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{	/* High-level function that implements the pscoast task */

	GMT_LONG i, np, ind, bin, base, anti_bin = -1, np_new, k, last_k, err, bin_trouble, error, n;
	GMT_LONG level_to_be_painted = 0, direction, start_direction, stop_direction, last_pen_level;
	GMT_LONG shift = FALSE, need_coast_base, recursive;
	GMT_LONG greenwich = FALSE, possibly_donut_hell = FALSE, fill_in_use = FALSE;
	GMT_LONG clobber_background, paint_polygons = FALSE, donut;
	GMT_LONG donut_hell = FALSE, world_map_save, clipping;

	double bin_x[5], bin_y[5], out[2];
	double west_border, east_border, anti_lon = 0.0, anti_lat = -90.0, edge = 720.0;
	double *xtmp = NULL, *ytmp = NULL, step, left, right, anti_x, anti_y, x_0, y_0, x_c, y_c, dist;

	char *shore_resolution[5] = {"full", "high", "intermediate", "low", "crude"};

	struct GMT_FILL fill[6];	/* Colors for (0) water, (1) land, (2) lakes, (3) islands in lakes, (4) lakes in islands in lakes, (5) riverlakes */
	struct GMT_SHORE c;
	struct GMT_BR b, r;
	struct GMT_GSHHS_POL *p = NULL;
	struct PSCOAST_CTRL *Ctrl = NULL;		/* Control structure specific to program */
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_pscoast_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_pscoast_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_pscoast", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJRb", "BKOPUXxYycpt>" GMT_OPT("EZ"), options))) Return (error);
	Ctrl = (struct PSCOAST_CTRL *) New_pscoast_Ctrl (GMT);		/* Allocate and initialize defaults in a new control structure */
	if ((error = GMT_pscoast_parse (API, Ctrl, options))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the pscoast main code ----------------------------*/

	/* Check and interpret the command line arguments */

	clipping = (Ctrl->G.clip || Ctrl->S.clip);
	if (Ctrl->D.force) Ctrl->D.set = GMT_shore_adjust_res (GMT, Ctrl->D.set);
	base = GMT_set_resolution (GMT, &Ctrl->D.set, 'D');
	fill[0] = Ctrl->S.fill;
	fill[1] = fill[3] = Ctrl->G.fill;
	fill[2] = fill[4] = (Ctrl->C.active) ? Ctrl->C.fill[LAKE] : Ctrl->S.fill;
	fill[5] = (Ctrl->C.active) ? Ctrl->C.fill[RIVER] : fill[2];
	need_coast_base = (Ctrl->G.active || Ctrl->S.active || Ctrl->C.active || Ctrl->W.active);
	if (Ctrl->Q.active) need_coast_base = FALSE;	/* Since we just end clipping */
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

	world_map_save = GMT->current.map.is_world;

	if (need_coast_base && GMT_init_shore (GMT, Ctrl->D.set, &c, GMT->common.R.wesn, &Ctrl->A.info))  {
		GMT_report (GMT, GMT_MSG_FATAL, "%s resolution shoreline data base not installed\n", shore_resolution[base]);
		need_coast_base = FALSE;
	}

	if (Ctrl->N.active && GMT_init_br (GMT, 'b', Ctrl->D.set, &b, GMT->common.R.wesn)) {
		GMT_report (GMT, GMT_MSG_FATAL, "%s resolution political boundary data base not installed\n", shore_resolution[base]);
		Ctrl->N.active = FALSE;
	}
	if (need_coast_base && GMT->current.setting.verbose >= GMT_MSG_NORMAL) GMT_message (GMT, "GSHHS version %s\n%s\n%s\n", c.version, c.title, c.source);

	if (Ctrl->I.active && GMT_init_br (GMT, 'r', Ctrl->D.set, &r, GMT->common.R.wesn)) {
		GMT_report (GMT, GMT_MSG_FATAL, "%s resolution river data base not installed\n", shore_resolution[base]);
		Ctrl->I.active = FALSE;
	}

	if (!(need_coast_base || Ctrl->N.active || Ctrl->I.active || Ctrl->Q.active)) {
		GMT_report (GMT, GMT_MSG_FATAL, "No databases available - aborts\n");
		Return (EXIT_FAILURE);
	}

	if (Ctrl->M.active) {	/* Dump linesegments to stdout; no plotting takes place */
		GMT_LONG id = 0;
		char header[BUFSIZ], *kind[3] = {"Coastlines", "Political boundaries", "Rivers"};
		if (Ctrl->N.active) id = 1;	if (Ctrl->I.active) id = 2; 
		GMT->current.io.multi_segments[GMT_OUT] = TRUE;	/* Turn on -mo explicitly */
		if ((error = GMT_set_cols (GMT, GMT_OUT, 2))) Return (error);
		if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data output */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);	/* Enables data output and sets access mode */
		sprintf (header, "# %s extracted from the %s resolution GSHHS version %s database\n", kind[id], shore_resolution[base], c.version);
		GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)header);
		sprintf (header, "# %s\n# %s\n", c.title, c.source);
		GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)header);
	}
	else {
		if (Ctrl->Q.active)
			GMT->current.ps.clip = -1;	/* Signal that this program terminates polygon clipping that initiated prior to this process */
		else if (clipping)
			GMT->current.ps.clip = +1;	/* Signal that this program initiates new clipping that wil outlive this process */

		GMT_plotinit (API, PSL, options);

		if (Ctrl->Q.active) {  /* Just undo previous clip-path */
			PSL_endclipping (PSL, 1);

			GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
			GMT_map_basemap (GMT, PSL); /* Basemap needed */
			GMT_plane_perspective (GMT, PSL, -1, 0.0);
	
			GMT_plotend (GMT, PSL);

			GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");

			Return (GMT_OK);
		}

		GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	}

	for (i = 0; i < 5; i++) if (fill[i].use_pattern) fill_in_use = TRUE;

	if (fill_in_use && !clobber_background) {	/* Force clobber when fill is used since our routine cannot deal with clipped fills */
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Pattern fill requires oceans to be painted first\n");
		clobber_background = TRUE;
		recursive = FALSE;
	}

	if (clipping) GMT_map_basemap (GMT, PSL);

	if (GMT->current.proj.projection == GMT_AZ_EQDIST && fabs (GMT->common.R.wesn[XLO] - GMT->common.R.wesn[XHI]) == 360.0 && (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO]) == 180.0) {
		possibly_donut_hell = TRUE;
		anti_lon = GMT->current.proj.central_meridian + 180.0;
		if (anti_lon >= 360.0) anti_lon -= 360.0;
		anti_lat = -GMT->current.proj.pole;
		anti_bin = (GMT_LONG)floor ((90.0 - anti_lat) / c.bsize) * c.bin_nx + (GMT_LONG)floor (anti_lon / c.bsize);
		GMT_geo_to_xy (GMT, anti_lon, anti_lat, &anti_x, &anti_y);
		GMT_geo_to_xy (GMT, GMT->current.proj.central_meridian, GMT->current.proj.pole, &x_0, &y_0);
		if (Ctrl->G.active) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Fill/clip continent option (-G) may not work for this projection.\nIf the antipole (%g/%g) is in the ocean then chances are good\nElse: avoid projection center coordinates that are exact multiples of %g degrees\n", anti_lon, anti_lat, c.bsize);
	}

	if (possibly_donut_hell && paint_polygons && !clobber_background) {	/* Force clobber when donuts may be called for now */
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning: -JE requires oceans to be painted first\n");
		clobber_background = TRUE;
		recursive = FALSE;
	}

	if (clobber_background) {	/* Paint entire map as ocean first, then lay land on top */
		n = GMT_map_clip_path (GMT, &xtmp, &ytmp, &donut);
		GMT_setfill (GMT, PSL, &Ctrl->S.fill, FALSE);
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

	if (GMT->common.R.wesn[XLO] < 0.0 && GMT->common.R.wesn[XHI] > 0.0 && !GMT_IS_LINEAR (GMT)) greenwich = TRUE;
	if ((360.0 - fabs (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO]) ) < c.bsize) {
		GMT->current.map.is_world = TRUE;
		if (GMT->current.proj.projection == GMT_GNOMONIC || GMT->current.proj.projection == GMT_GENPER) GMT->current.map.is_world = FALSE;
		if (GMT_IS_AZIMUTHAL (GMT)) GMT->current.map.is_world = FALSE;
	}
	if (GMT->current.map.is_world && greenwich)
		edge = GMT->current.proj.central_meridian;
	else if (!GMT->current.map.is_world && greenwich) {
		shift = TRUE;
		edge = GMT->common.R.wesn[XLO];	if (edge < 0.0) edge += 360.0;
		if (edge > 180.0) edge = 180.0;
	}

	if (GMT->common.R.wesn[XLO] < 0.0 && GMT->common.R.wesn[XHI] <= 0.0) {	/* Temporarily shift boundaries */
		GMT->common.R.wesn[XLO] += 360.0;
		GMT->common.R.wesn[XHI] += 360.0;
		if (GMT->current.proj.central_meridian < 0.0) GMT->current.proj.central_meridian += 360.0;
	}
	west_border = floor (GMT->common.R.wesn[XLO] / c.bsize) * c.bsize;
	east_border = ceil (GMT->common.R.wesn[XHI] / c.bsize) * c.bsize;

	if (!Ctrl->M.active && Ctrl->W.active) GMT_setpen (GMT, PSL, &Ctrl->W.pen[0]);

	/* Maximum step size (in degrees) used for interpolation of line segments along great circles */
	step = GMT->current.setting.map_line_step / GMT->current.proj.scale[GMT_X] / GMT->current.proj.M_PR_DEG;
	last_pen_level = -1;

	if (clipping) PSL_beginclipping (PSL, xtmp, ytmp, 0, GMT->session.no_rgb, 1);	/* Start clippath */

	for (ind = 0; need_coast_base && ind < c.nb; ind++) {	/* Loop over necessary bins only */

		bin = c.bins[ind];
#ifdef DEBUG
		if (Ctrl->debug.active && bin != Ctrl->debug.bin) continue;
#endif
		if ((err = GMT_get_shore_bin (GMT, ind, &c))) {
			GMT_report (GMT, GMT_MSG_FATAL, "%s [%s resolution shoreline]\n", GMT_strerror(err), shore_resolution[base]);
			Return (EXIT_FAILURE);
		}

		GMT_report (GMT, GMT_MSG_NORMAL, "Working on bin # %5ld\r", bin);
		if (!Ctrl->M.active) PSL_comment (PSL, "Bin # %ld\n", bin);

		if (GMT->current.map.is_world && greenwich) {
			left = c.bsize * (bin % c.bin_nx);	right = left + c.bsize;
			shift = ((left - edge) <= 180.0 && (right - edge) > 180.0);
		}

		bin_trouble = (anti_bin == bin) ? anti_bin : -1;

		if (possibly_donut_hell) {
			bin_x[0] = bin_x[3] = bin_x[4] = c.lon_sw;
			bin_x[1] = bin_x[2] = c.lon_sw + c.bsize;
			bin_y[0] = bin_y[1] = bin_y[4] = c.lat_sw;
			bin_y[2] = bin_y[3] = c.lat_sw + c.bsize;
			GMT_geo_to_xy (GMT, c.lon_sw + 0.5 * c.bsize, c.lat_sw + 0.5 * c.bsize, &x_c, &y_c);
			dist = hypot (x_c - x_0, y_c - y_0);
			donut_hell = (GMT_non_zero_winding (GMT, anti_lon, anti_lat, bin_x, bin_y, 5) || dist > 0.8 * GMT->current.proj.r);
		}

		for (direction = start_direction; paint_polygons && direction <= stop_direction; direction += 2) {

			/* Assemble one or more segments into polygons */

			if ((np = GMT_assemble_shore (GMT, &c, direction, TRUE, shift, west_border, east_border, &p)) == 0) continue;

			/* Get clipped polygons in x,y inches that can be plotted */

			np_new = GMT_prep_polygons (GMT, &p, np, donut_hell, step, bin_trouble);

			if (clipping) {
				for (k = level_to_be_painted; k < GMT_MAX_GSHHS_LEVEL - 1; k++) recursive_path (GMT, PSL, -1, np_new, p, k, NULL);

				for (k = 0; k < np_new; k++) {	/* Do any remaining interior polygons */
					if (p[k].n == 0) continue;
					if (p[k].level % 2 == level_to_be_painted || p[k].level > 2) {
						PSL_plotline (PSL, p[k].lon, p[k].lat, p[k].n, PSL_MOVE);
					}
				}
			}
			else if (recursive) {	/* Must avoid pointing anything but the polygons inside */

				for (k = level_to_be_painted; k < GMT_MAX_GSHHS_LEVEL - 1; k++) recursive_path (GMT, PSL, -1, np_new, p, k, fill);

				for (k = 0; k < np_new; k++) {	/* Do any remaining interior polygons */
					if (p[k].n == 0) continue;
					if (p[k].level % 2 == level_to_be_painted || p[k].level > 2) {
						GMT_setfill (GMT, PSL, &fill[p[k].fid], FALSE);
						PSL_plotpolygon (PSL, p[k].lon, p[k].lat, p[k].n);
					}
				}
			}
			else {	/* Simply paints all polygons as is */
				for (k = 0; k < np_new; k++) {
					if (p[k].n == 0 || p[k].level < level_to_be_painted) continue;
					if (donut_hell && GMT_non_zero_winding (GMT, anti_x, anti_y, p[k].lon, p[k].lat, p[k].n)) {	/* Antipode inside polygon, must do donut */
						n = GMT_map_clip_path (GMT, &xtmp, &ytmp, &donut);
						GMT_setfill (GMT, PSL, &fill[p[k].fid], FALSE);
						PSL_plotline (PSL, xtmp, ytmp, n, PSL_MOVE + PSL_CLOSE);
						PSL_plotpolygon (PSL, p[k].lon, p[k].lat, p[k].n);
						GMT_free (GMT, xtmp);
						GMT_free (GMT, ytmp);
					}
					else {
						GMT_setfill (GMT, PSL, &fill[p[k].fid], FALSE);
						PSL_plotpolygon (PSL, p[k].lon, p[k].lat, p[k].n);
					}
				}
			}

			GMT_free_polygons (GMT, p, np_new);
			GMT_free (GMT, p);
		}

		if (Ctrl->W.active && c.ns) {	/* Draw or dump shorelines, no need to assemble polygons */
			if ((np = GMT_assemble_shore (GMT, &c, 1, FALSE, shift, west_border, east_border, &p)) == 0) continue;

			for (i = 0; i < np; i++) {
				if (Ctrl->M.active) {
					sprintf (GMT->current.io.segment_header, "Shore Bin # %ld, Level %ld", bin, p[i].level);
					GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
					for (k = 0; k < p[i].n; k++) {
						out[GMT_X] = p[i].lon[k];
						out[GMT_Y] = p[i].lat[k];
						GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)out);
					}
				}
				else if (Ctrl->W.use[p[i].level-1]) {
					if (donut_hell) p[i].n = GMT_fix_up_path (GMT, &p[i].lon, &p[i].lat, p[i].n, step, 0);
					GMT->current.plot.n = GMT_geo_to_xy_line (GMT, p[i].lon, p[i].lat, p[i].n);
					if (!GMT->current.plot.n) continue;

					k = p[i].level - 1;
					if (k != last_pen_level) {
						GMT_setpen (GMT, PSL, &Ctrl->W.pen[k]);
						last_pen_level = k;
					}
					GMT_plot_line (GMT, PSL, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n);
				}
			}

			GMT_free_polygons (GMT, p, np);
			GMT_free (GMT, p);
		}

		GMT_free_shore (GMT, &c);

	}
	if (need_coast_base) GMT_shore_cleanup (GMT, &c);

	if (clipping) PSL_beginclipping (PSL, xtmp, ytmp, 0, GMT->session.no_rgb, 2);	/* End clippath */

	if (Ctrl->I.active) {	/* Read rivers file and plot as lines */

		GMT_report (GMT, GMT_MSG_NORMAL, "Adding Rivers...");
		if (!Ctrl->M.active) PSL_comment (PSL, "Start of River segments\n");
		last_k = -1;

		for (ind = 0; ind < r.nb; ind++) {	/* Loop over necessary bins only */

			bin = r.bins[ind];
			GMT_get_br_bin (GMT, ind, &r, Ctrl->I.use, Ctrl->I.n_rlevels);

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
					sprintf (GMT->current.io.segment_header, "River Bin # %ld, Level %ld", bin, p[i].level);
					GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
					for (k = 0; k < p[i].n; k++) {
						out[GMT_X] = p[i].lon[k];
						out[GMT_Y] = p[i].lat[k];
						GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)out);
					}
				}
				else {
					GMT->current.plot.n = GMT_geo_to_xy_line (GMT, p[i].lon, p[i].lat, p[i].n);
					if (!GMT->current.plot.n) continue;

					k = p[i].level;
					if (k != last_k) {
						GMT_setpen (GMT, PSL, &Ctrl->I.pen[k]);
						last_k = k;
					}
					GMT_plot_line (GMT, PSL, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n);
				}
			}

			/* Free up memory */

			GMT_free_br (GMT, &r);
			GMT_free_polygons (GMT, p, np);
			GMT_free (GMT, p);
		}
		GMT_br_cleanup (GMT, &r);
	}

	if (Ctrl->N.active) {	/* Read borders file and plot as lines */

		GMT_report (GMT, GMT_MSG_NORMAL, "Adding Borders...");
		if (!Ctrl->M.active) PSL_comment (PSL, "Start of Border segments\n");

		/* Must resample borders because some points may be too far apart and look like 'jumps' */

		step = MAX (fabs(GMT->common.R.wesn[XLO] - GMT->common.R.wesn[XHI]), fabs (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO])) / 4.0;

		last_k = -1;

		for (ind = 0; ind < b.nb; ind++) {	/* Loop over necessary bins only */

			bin = b.bins[ind];
			GMT_get_br_bin (GMT, ind, &b, Ctrl->N.use, Ctrl->N.n_blevels);

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
					sprintf (GMT->current.io.segment_header, "Border Bin # %ld, Level %ld", bin, p[i].level);
					GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
					for (k = 0; k < p[i].n; k++) {
						out[GMT_X] = p[i].lon[k];
						out[GMT_Y] = p[i].lat[k];
						GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)out);
					}
				}
				else {
					p[i].n = GMT_fix_up_path (GMT, &p[i].lon, &p[i].lat, p[i].n, step, 0);
					GMT->current.plot.n = GMT_geo_to_xy_line (GMT, p[i].lon, p[i].lat, p[i].n);
					if (!GMT->current.plot.n) continue;

					k = p[i].level - 1;
					if (k != last_k) {
						GMT_setpen (GMT, PSL, &Ctrl->N.pen[k]);
						last_k = k;
					}
					GMT_plot_line (GMT, PSL, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n);
				}
			}

			/* Free up memory */

			GMT_free_br (GMT, &b);
			GMT_free_polygons (GMT, p, np);
			GMT_free (GMT, p);
		}
		GMT_br_cleanup (GMT, &b);
	}

	if (!Ctrl->M.active) {
		if (GMT->current.map.frame.plot) {
			GMT->current.map.is_world = world_map_save;
			GMT_map_basemap (GMT, PSL);
		}

		if (Ctrl->L.active) GMT_draw_map_scale (GMT, PSL, &Ctrl->L.item);
		if (Ctrl->T.active) GMT_draw_map_rose (GMT, PSL, &Ctrl->T.item);

		GMT_plane_perspective (GMT, PSL, -1, 0.0);
		GMT_plotend (GMT, PSL);
	}
	else {
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */
	}
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Done\n");

	Return (GMT_OK);
}
