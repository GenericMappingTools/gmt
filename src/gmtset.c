/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
#include "longopt/gmtset_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtset"
#define THIS_MODULE_MODERN_NAME	"gmtset"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Change individual GMT default settings"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-V" GMT_SHORTHAND_OPTIONS

#define GMT_SHORTHAND_OPTIONS_DISP	"B|J|R|X|Y|p"	/* All of the shorthand options for display purposes */

/* Control structure for gmtset */

struct GMTSET_CTRL {
	struct GMTSET_C {	/* -C */
		bool active;
	} C;
	struct GMTSET_D {	/* -D[s|u] */
		bool active;
		char mode;
	} D;
	struct GMTSET_G {	/* -Gfilename */
		bool active;
		char *file;
	} G;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTSET_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTSET_CTRL);
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTSET_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [-C | -D[s|u] | -G<defaultsfile>] [-" GMT_SHORTHAND_OPTIONS_DISP "<arg>] [%s] PARAMETER1 value1 PARAMETER2 value2 PARAMETER3 value3 ...\n", name, GMT_V_OPT);
	GMT_Usage (API, 1, "Give pairs of PARAMETER value to change these settings.  For available PARAMETERS, see %s documentation.", GMT_SETTINGS_FILE);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-C Convert GMT4 .gmtdefaults4 to a %s file. "
		"The original file is retained.", GMT_SETTINGS_FILE);
	GMT_Usage (API, 1, "\n-D[s|u]");
	GMT_Usage (API, -2, "Modify the default settings based on the GMT system defaults. Optionally append a directive:");
	GMT_Usage (API, 3, "s: Use the SI version of defaults.");
	GMT_Usage (API, 3, "u: Use the US version of defaults.");
	GMT_Usage (API, 1, "\n-G<defaultsfile>");
	GMT_Usage (API, -2, "Set name of specific %s file to modify "
		"[Default looks for file in current directory.  If not found, "
		"it looks in the home directory, if not found it uses GMT defaults.]", GMT_SETTINGS_FILE);
	GMT_Usage (API, -2, "Note: Only settings that differ from the GMT SI system defaults are written "
		"to the file %s in the current directory (under classic mode) "
		"or in the current session directory (under modern mode).", GMT_SETTINGS_FILE);
	GMT_Usage (API, 1, "\n-%s<arg>", GMT_SHORTHAND_OPTIONS_DISP);
	GMT_Usage (API, -2, "Any of these options can be used to set "
		"the expansion of any of these shorthand options via the appended argument.");
	GMT_Option (API, "V");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GMTSET_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtset and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files: Here, arguments like ANNOT_OFFSET = -0.1i will show up as "files" or numbers */
			case '#':
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Convert GMT4 .gmtdefaults4 to gmt.conf */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'D':	/* Get GMT system-wide defaults settings */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				n_errors += gmt_get_required_char (GMT, opt->arg, opt->option, 0, &Ctrl->D.mode);
				break;
			case 'G':	/* Optional defaults file on input and output */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->G.file))) n_errors++;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gmtset (void *V_API, int mode, void *args) {
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

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmtset main code ----------------------------*/

	/* Read the supplied default file or the users defaults to override system settings */

	if (Ctrl->D.active) {	/* Start with the system defaults settings which were loaded by GMT_Create_Session */
		gmt_update_keys (GMT, false);
		if (Ctrl->D.mode == 'u')
			gmt_conf_US (GMT);	/* Change a few to US defaults */
	}
	else if (Ctrl->C.active) {	/* Convert deprecated .gmtdefaults4 files */
		if (gmt_getdefaults (GMT, ".gmtdefaults4")) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to find or read .gmtdefaults4 settings file\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}
	else if (Ctrl->G.active && gmt_getdefaults (GMT, Ctrl->G.file)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to access or read %s\n", Ctrl->G.file);
		Return (GMT_RUNTIME_ERROR);
	}

	if (gmt_setdefaults (GMT, options)) Return (GMT_PARSE_ERROR);		/* Process command line arguments, return error if failures */

	strcpy (GMT->current.setting.theme, "off");	/* To preserve changes the user may have set */

	gmt_putdefaults (GMT, Ctrl->G.file);	/* Write out the revised settings */

	Return (GMT_NOERROR);
}
