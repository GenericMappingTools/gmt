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

#define THIS_MODULE_NAME	"psbasemap"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot PostScript base maps"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->BJKOPRUVXYcfptxy" GMT_OPT("EZ")

/* Control structure for psbasemap */

struct PSBASEMAP_CTRL {
	struct D {	/* -D */
		bool active;
		struct GMT_MAP_INSERT item;
	} D;
	struct L {	/* -L */
		bool active;
		struct GMT_MAP_SCALE item;
	} L;
	struct T {	/* -T */
		bool active;
		struct GMT_MAP_ROSE item;
	} T;
};

void *New_psbasemap_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSBASEMAP_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSBASEMAP_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	GMT_memset (&C->D.item, 1, struct GMT_MAP_INSERT);
	GMT_memset (&C->L.item, 1, struct GMT_MAP_SCALE);
	GMT_memset (&C->T.item, 1, struct GMT_MAP_ROSE);

	return (C);
}

void Free_psbasemap_Ctrl (struct GMT_CTRL *GMT, struct PSBASEMAP_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);
}

int GMT_psbasemap_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the psbasemap synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: psbasemap %s %s %s\n", GMT_B_OPT, GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-D%s]\n\t[%s] [-K]\n", GMT_INSERT, GMT_Jz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-L%s]\n", GMT_SCALE);
	GMT_Message (API, GMT_TIME_NONE, "\t[-O] [-P] [-T%s]\n", GMT_TROSE);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n\n", GMT_U_OPT, GMT_V_OPT,
		GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_f_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Option (API, "B,JZ,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_mapinsert_syntax (API->GMT, 'D', "Draw a simple map insert box as specified below:");
	GMT_Option (API, "K");
	GMT_mapscale_syntax (API->GMT, 'L', "Draw a simple map scale centered on <lon0>/<lat0>.");
	GMT_Option (API, "O,P");
	GMT_maprose_syntax (API->GMT, 'T', "Draw a north-pointing map rose centered on <lon0>/<lat0>.");
	GMT_Option (API, "U,V,X,c,f,p,t,.");

	return (EXIT_FAILURE);
}

int GMT_psbasemap_parse (struct GMT_CTRL *GMT, struct PSBASEMAP_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to psbasemap and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			/* Processes program-specific parameters */

			case 'D':	/* Draw map insert */
				Ctrl->D.active = true;
				n_errors += GMT_getinsert (GMT, 'D', opt->arg, &Ctrl->D.item);
				break;
			case 'G':	/* Set canvas color */
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -G is deprecated; -B...+g%s was set instead, use this in the future.\n", opt->arg);
					GMT->current.map.frame.paint = true;
					if (GMT_getfill (GMT, opt->arg, &GMT->current.map.frame.fill)) {
						GMT_fill_syntax (GMT, 'G', " ");
						n_errors++;
					}
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'L':	/* Draw map scale */
				Ctrl->L.active = true;
				n_errors += GMT_getscale (GMT, 'L', opt->arg, &Ctrl->L.item);
				break;
			case 'T':	/* Draw map rose */
				Ctrl->T.active = true;
				n_errors += GMT_getrose (GMT, 'T', opt->arg, &Ctrl->T.item);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !(GMT->current.map.frame.init || Ctrl->D.active || Ctrl->L.active || Ctrl->T.active), "Syntax error: Must specify at least one of -B, -D, -L, -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.active && !GMT_is_geographic (GMT, GMT_IN), "Syntax error: -L applies to geographical data only\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_psbasemap_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout(code);}

int GMT_psbasemap (void *V_API, int mode, void *args)
{	/* High-level function that implements the psbasemap task */
	int error;
	
	struct PSBASEMAP_CTRL *Ctrl = NULL;	/* Control structure specific to program */
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_psbasemap_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_psbasemap_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_psbasemap_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_psbasemap_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psbasemap_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the psbasemap main code ----------------------------*/

	/* Ready to make the plot */

	GMT_Report (API, GMT_MSG_VERBOSE, "Constructing the basemap\n");

	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	GMT_plotinit (GMT, options);

	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	GMT_plotcanvas (GMT);	/* Fill canvas if requested */

	GMT_map_basemap (GMT);	/* Plot base map */

	if (Ctrl->D.active) GMT_draw_map_insert (GMT, &Ctrl->D.item);
	if (Ctrl->L.active) GMT_draw_map_scale (GMT, &Ctrl->L.item);
	if (Ctrl->T.active) GMT_draw_map_rose (GMT, &Ctrl->T.item);

	GMT_plane_perspective (GMT, -1, 0.0);

	GMT_plotend (GMT);

	Return (GMT_OK);
}
