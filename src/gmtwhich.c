/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
	struct GMTWHICH_A {	/* -A */
		bool active;
	} A;
	struct GMTWHICH_C {	/* -C */
		bool active;
	} C;
	struct GMTWHICH_D {	/* -D */
		bool active;
	} D;
	struct GMTWHICH_G {	/* -G[c|l|u] */
		bool active;
		unsigned int mode;
	} G;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTWHICH_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTWHICH_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTWHICH_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<files>] [-A] [-C] [-D] [-G[a|c|l|u]] [%s] [%s]\n", name, GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<files> One or more file names of any data type (grids, tables, etc.).");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A Only consider files you have permission to read [all files].");
	GMT_Usage (API, 1, "\n-C Print Y if found and N if not found.  No path is returned.");
	GMT_Usage (API, 1, "\n-D Print the directory where a file is found [full path to file].");
	GMT_Usage (API, 1, "\n-G[a|c|l|u]");
	GMT_Usage (API, -2, "Download file if possible and not found locally. Optionally, append one of the following:");
	GMT_Usage (API, -3, "a: Place under the user directory in the appropriate folder.");
	GMT_Usage (API, -3, "c: Place it in the cache directory.");
	GMT_Usage (API, -3, "l: Place it in the current local directory [Default].");
	GMT_Usage (API, -3, "u: Place it in the user\'s data directory.");
	GMT_Option (API, "V,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GMTWHICH_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtwhich and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Only consider readable files */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				break;
			case 'C':	/* Print Y or N instead of names */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				break;
			case 'D':	/* Want directory instead */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				break;
			case 'G':	/* Download file first, if required */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				switch (opt->arg[0]) {
					case 'a': Ctrl->G.mode = GMT_AUTO_DIR;		break;
					case 'c': Ctrl->G.mode = GMT_CACHE_DIR;		break;
					case 'u': Ctrl->G.mode = GMT_DATA_DIR;		break;
					case '\0': case 'l': Ctrl->G.mode = GMT_LOCAL_DIR;	break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Download mode %s not recognized\n", opt->arg);
						n_errors++;
						break;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files == 0, "No files specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->D.active, "Cannot use -D if -C is set\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}


GMT_LOCAL int gmtwhich_list_tiles (struct GMTAPI_CTRL *API, char *list, bool YN, struct GMT_RECORD *Out) {
	/* List all tiles and whether found/not found */
	int t_data, fail;
	uint64_t n, k, nf = 0;
	char dir[PATH_MAX] = {""}, path[PATH_MAX] = {""};
	char **file = NULL;

	if ((t_data = gmt_get_tile_id (API, list)) == GMT_NOTSET) return GMT_RUNTIME_ERROR;
	if ((n = gmt_read_list (API->GMT, list, &file)) == 0) return GMT_RUNTIME_ERROR;
	snprintf (dir, PATH_MAX, "%s%s%s", API->GMT->session.USERDIR, API->remote_info[t_data].dir, API->remote_info[t_data].file);

	for (k = 0; k < n; k++) {
		sprintf (path, "%s%s", dir, &file[k][1]);
		fail = access (path, R_OK);
		if (fail) {	/* Look for jp2 version instead */
			char *jp2_file = gmt_strrep (path, GMT_TILE_EXTENSION_LOCAL, GMT_TILE_EXTENSION_REMOTE);
			strcpy (path, jp2_file);
			fail = access (path, R_OK);
			gmt_M_str_free (jp2_file);
		}
		if (fail) {
			if (YN) {
				strcpy (path, "N");
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			strcpy (Out->text, path);
			GMT_Report (API, GMT_MSG_ERROR, "Tile %s not found!\n", file[k]);

		}
		else {	/* Found */
			if (YN)	/* Just want a Yes */
				strcpy (path, "Y");
			strcpy (Out->text, path);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			nf++;
		}
	}
	GMT_Report (API, GMT_MSG_INFORMATION, "Tiled dataset %s has %" PRIu64 " tiles; %" PRIu64 " are present in %s\n", API->remote_info[t_data].file, n, nf, dir);

	gmt_free_list (API->GMT, file, n);
	return GMT_NOERROR;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gmtwhich (void *V_API, int mode, void *args) {
	bool list;
	int error = 0, fmode, k_data;
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

		list = gmt_file_is_tiled_list (API, opt->arg, NULL, NULL, NULL);

		if (Ctrl->G.active) {
			if (list) {
				gmt_download_tiles (API, opt->arg, Ctrl->G.mode);
				if ((error = gmtwhich_list_tiles (API, opt->arg, Ctrl->C.active, Out)))
					Return (error);
				continue;
			}
			else
				first = gmt_download_file_if_not_found (GMT, opt->arg, Ctrl->G.mode);
		}
		else if (opt->arg[0] == '@') /* Gave @ without -G is likely a user mistake; remove it */
			first = 1;
		if ((k_data = gmt_remote_no_extension (API, opt->arg)) != GMT_NOTSET)
			sprintf (file, "%s%s", opt->arg, API->remote_info[k_data].ext);	/* Append the implicit extension for remote grids */
		else
			strcpy (file, opt->arg);
		if (list) {
			if ((error = gmtwhich_list_tiles (API, opt->arg, Ctrl->C.active, Out)))
				Return (error);
		}
		else if (gmt_getdatapath (GMT, &file[first], path, fmode)) {	/* Found the file */
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

	gmt_M_free (GMT, Out);
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	Return (GMT_NOERROR);
}
