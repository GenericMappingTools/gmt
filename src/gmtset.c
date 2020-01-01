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
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: gmtset will override specified defaults parameters.
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtset"
#define THIS_MODULE_MODERN_NAME	"gmtset"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Change individual GMT default settings"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-V" GMT_SHORTHAND_OPTIONS

/* Control structure for gmtset */

struct GMTSET_CTRL {
	struct C {	/* -C */
		bool active;
	} C;
	struct D {	/* -D[s|u] */
		bool active;
		char mode;
	} D;
	struct G {	/* -Gfilename */
		bool active;
		char *file;
	} G;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTSET_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTSET_CTRL);
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTSET_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [-C | -D[s|u] | -G<defaultsfile>] [-[" GMT_SHORTHAND_OPTIONS "]<value>] PARAMETER1 [=] value1 PARAMETER2 [=] value2 PARAMETER3 [=] value3 ...\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\n\tFor available PARAMETERS, see gmt.conf man page.\n\n");

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Convert GMT4 .gmtdefaults4 to a gmt.conf file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The original file is retained.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Modify the default settings based on the GMT system defaults.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append s to see the SI version of defaults.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append u to see the US version of defaults.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set name of specific gmt.conf file to modify.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default looks for file in current directory.  If not found,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   it looks in the home directory, if not found it uses GMT defaults.]\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOnly settings that differ from the GMT SI system defaults are written\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to the file gmt.conf in the current directory (under classic mode)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or in the current session directory (under modern mode).\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\t-[" GMT_SHORTHAND_OPTIONS "]<value> (any of these options).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Set the expansion of any of these shorthand options.\n");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTSET_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtset and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files: Here, arguments like ANNOT_OFFSET = -0.1i will show up as "files" or numbers */
			case '#':
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Convert GMT4 .gmtdefaults4 to gmt.conf */
				Ctrl->C.active = true;
				break;
			case 'D':	/* Get GMT system-wide defaults settings */
				Ctrl->D.active = true;
				Ctrl->D.mode = opt->arg[0];
				break;
			case 'G':	/* Optional defaults file on input and output */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC void gmtinit_update_keys (struct GMT_CTRL *GMT, bool arg);

int GMT_gmtset (void *V_API, int mode, void *args) {
	int error = 0;

	struct GMTSET_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmtset main code ----------------------------*/

	/* Read the supplied default file or the users defaults to override system settings */

	if (Ctrl->D.active) {	/* Start with the system defaults settings which were loaded by GMT_Create_Session */
		gmtinit_update_keys (GMT, false);
		if (Ctrl->D.mode == 'u')
			gmt_conf_US (GMT);	/* Change a few to US defaults */
	}
	else if (Ctrl->C.active)
		gmt_getdefaults (GMT, ".gmtdefaults4");
	else if (Ctrl->G.active)
		gmt_getdefaults (GMT, Ctrl->G.file);

	if (gmt_setdefaults (GMT, options)) Return (GMT_PARSE_ERROR);		/* Process command line arguments, return error if failures */

	gmt_putdefaults (GMT, Ctrl->G.file);	/* Write out the revised settings */

	Return (GMT_NOERROR);
}
