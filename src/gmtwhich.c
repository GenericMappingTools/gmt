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
 * Brief synopsis: gmtwhich.c will read a list of data files and return the
 * full path by searching the DATADIR list of directories.
 *
 * Author:	Paul Wessel
 * Date:	15-OCT-2009
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtwhich"
#define THIS_MODULE_MODERN_NAME	"gmtwhich"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Find full path to specified files"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-V"

struct GMTWHICH_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -A */
		bool active;
	} A;
	struct C {	/* -C */
		bool active;
	} C;
	struct D {	/* -D */
		bool active;
	} D;
	struct G {	/* -G[c|l|u] */
		bool active;
		unsigned int mode;
	} G;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTWHICH_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTWHICH_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTWHICH_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [files] [-A] [-C] [-D] [-G[c|l|u]] [%s] [%s]\n\n", name, GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Only consider files you have permission to read [all files].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Print Y if found and N if not found.  No path is returned.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Print the directory where a file is found [full path to file].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Download file if possible and not found locally.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append c to place it in the cache directory.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append l to place it in the current local directory [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append u to place it in the user\'s data directory.\n");
	GMT_Option (API, "V,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTWHICH_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtwhich and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Only consider readable files */
				Ctrl->A.active = true;
				break;
			case 'C':	/* Print Y or N instead of names */
				Ctrl->C.active = true;
				break;
			case 'D':	/* Want directory instead */
				Ctrl->D.active = true;
				break;
			case 'G':	/* Download file first, if required */
				Ctrl->G.active = true;
				switch (opt->arg[0]) {
					case 'c': Ctrl->G.mode = GMT_CACHE_DIR;		break;
					case 'u': Ctrl->G.mode = GMT_DATA_DIR;		break;
					case '\0': case 'l': Ctrl->G.mode = GMT_LOCAL_DIR;	break;
					default:
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Download mode %s not recognized\n", opt->arg);
						n_errors++;
						break;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files == 0, "No files specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->D.active, "Cannot use -D if -C is set\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtwhich (void *V_API, int mode, void *args) {
	int error = 0, fmode;
	unsigned int first = 0;	/* Real start of filename */

	char path[PATH_MAX] = {""}, file[PATH_MAX] = {""}, *Yes = "Y", *No = "N", cwd[PATH_MAX] = {""}, *p = NULL;

	struct GMTWHICH_CTRL *Ctrl = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmtwhich main code ----------------------------*/

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_TEXT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}

	if (Ctrl->D.active && (getcwd (cwd, PATH_MAX) == NULL)) {	/* Get full path, even for current dir */
		GMT_Report (API, GMT_MSG_WARNING, "Unable to determine current working directory!\n");
	}
	fmode = (Ctrl->A.active) ? R_OK : F_OK;	/* Either readable or existing files */
	Out = gmt_new_record (GMT, NULL, path);	/* Place coordinates in data and message in text */

	for (opt = options; opt; opt = opt->next) {
		if (opt->option != '<') continue;	/* Skip anything but filenames */
		if (!opt->arg[0]) continue;		/* Skip empty arguments */

		if (Ctrl->G.active)
			first = gmt_download_file_if_not_found (GMT, opt->arg, Ctrl->G.mode);
		else if (opt->arg[0] == '@') /* Gave @ without -G is likely a user mistake; remove it */
			first = 1;
		if (gmt_M_file_is_remotedata (opt->arg) && !strstr (opt->arg, ".grd"))
			sprintf (file, "%s.grd", opt->arg);	/* Append the implicit .grd for remote earth_relief grids */
		else
			strcpy (file, opt->arg);
		if (gmt_getdatapath (GMT, &file[first], path, fmode)) {	/* Found the file */
			char *L = NULL;
			if (Ctrl->D.active) {
				p = strstr (path, &file[first]);	/* Start of filename */
				if (!strcmp (p, path)) /* Found file in current directory */
					strcpy (path, cwd);
				else
					*(--p) = 0;	/* Chop off file, report directory */
			}
			else if (Ctrl->C.active)	/* Just want a Yes */
				strcpy (path, Yes);
			if (Ctrl->G.active && Ctrl->G.mode == GMT_LOCAL_DIR && (path[0] == '/' || path[1] == ':') && (L = strrchr(path, '/'))) {
				/* File found on system but we want a copy in the current directory */
				if (gmt_rename_file (GMT, path, &L[1], GMT_COPY_FILE))
					Return (GMT_RUNTIME_ERROR);
				memmove (path, &L[1], strlen (&L[1])+1);	/* Report the file in the local directory now */
			}
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		else {	/* Did not find.  Report no or be quiet */
			if (Ctrl->C.active) {
				strcpy (path, No);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			GMT_Report (API, GMT_MSG_ERROR, "File %s not found!\n", &file[first]);
		}
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	gmt_M_free (GMT, Out);
	Return (GMT_NOERROR);
}
