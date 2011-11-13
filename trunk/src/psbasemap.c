/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * API functions to support the psbasemap application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: psbasemap plots a basemap for the given area using
 * the specified map projection.
 */

#include "pslib.h"
#include "gmt.h"

/* Control structure for psbasemap */

struct PSBASEMAP_CTRL {
	struct L {	/* -L */
		GMT_LONG active;
		struct GMT_MAP_SCALE item;
	} L;
	struct T {	/* -T */
		GMT_LONG active;
		struct GMT_MAP_ROSE item;
	} T;
};

void *New_psbasemap_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSBASEMAP_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSBASEMAP_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */
	GMT_memset (&C->L.item, 1, struct GMT_MAP_SCALE);
	GMT_memset (&C->T.item, 1, struct GMT_MAP_ROSE);

	return (C);
}

void Free_psbasemap_Ctrl (struct GMT_CTRL *GMT, struct PSBASEMAP_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);
}

GMT_LONG GMT_psbasemap_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the psbasemap synopsis and optionally full usage information */

	GMT_message (GMT, "psbasemap %s [API] - Plot PostScript base maps\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: psbasemap %s %s %s [-K] [%s]\n", GMT_B_OPT, GMT_J_OPT, GMT_Rgeoz_OPT, GMT_Jz_OPT);
	GMT_message (GMT, "\t[%s]\n", GMT_SCALE);
	GMT_message (GMT, "\t[-O] [-P] [%s]\n", GMT_TROSE);
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s]\n\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_f_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_explain_options (GMT, "BJZR");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "K");
	GMT_mapscale_syntax (GMT, 'L', "Draw a simple map scale centered on <lon0>/<lat0>.");
	GMT_explain_options (GMT, "OP");
	GMT_maprose_syntax (GMT, 'T', "Draw a north-pointing map rose centered on <lon0>/<lat0>.");
	GMT_explain_options (GMT, "UVXcfpt.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_psbasemap_parse (struct GMTAPI_CTRL *C, struct PSBASEMAP_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to psbasemap and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			/* Processes program-specific parameters */

#ifdef GMT_COMPAT
			case 'G':	/* Set canvas color */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -G is deprecated; -B...+g%s was set instead, use this in the future.\n", opt->arg);
				GMT->current.map.frame.paint = TRUE;
				if (GMT_getfill (GMT, opt->arg, &GMT->current.map.frame.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
#endif
			case 'L':	/* Draw map scale */
				Ctrl->L.active = TRUE;
				n_errors += GMT_getscale (GMT, opt->arg, &Ctrl->L.item);
				break;
			case 'T':	/* Draw map rose */
				Ctrl->T.active = TRUE;
				n_errors += GMT_getrose (GMT, opt->arg, &Ctrl->T.item);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !(GMT->current.map.frame.init || Ctrl->L.active || Ctrl->T.active), "Syntax error: Must specify at least one of -B, -L, -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.active && !GMT_is_geographic (GMT, GMT_IN), "Syntax error: -L applies to geographical data only\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_psbasemap_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout(code);}

GMT_LONG GMT_psbasemap (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{	/* High-level function that implements the psbasemap task */
	GMT_LONG error;
	
	struct PSBASEMAP_CTRL *Ctrl = NULL;	/* Control structure specific to program */
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	if ((options = GMT_Prep_Options (API, mode, args)) == NULL) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_psbasemap_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_psbasemap_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_psbasemap", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-VJRf", "BKOPUXxYycpt>" GMT_OPT("EZ"), options)) Return (API->error);
	Ctrl = New_psbasemap_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psbasemap_parse (API, Ctrl, options))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the psbasemap main code ----------------------------*/

	/* Ready to make the plot */

	GMT_report (GMT, GMT_MSG_NORMAL, "Constructing the basemap\n");

	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	GMT_plotinit (GMT, options);

	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	GMT_plotcanvas (GMT);	/* Fill canvas if requested */

	GMT_map_basemap (GMT);	/* Plot base map */

	if (Ctrl->L.active) GMT_draw_map_scale (GMT, &Ctrl->L.item);
	if (Ctrl->T.active) GMT_draw_map_rose (GMT, &Ctrl->T.item);

	GMT_plane_perspective (GMT, -1, 0.0);

	GMT_plotend (GMT);

	Return (GMT_OK);
}
