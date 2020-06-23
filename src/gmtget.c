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
 * Brief synopsis: gmtget will return the values of selected parameters.
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtget"
#define THIS_MODULE_MODERN_NAME	"gmtget"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Get individual GMT default settings or download data sets"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-V"

/* Control structure for gmtget */

struct GMTGET_CTRL {
	struct GMTGET_D {	/* -Ddir */
		bool active;
		char *dir;
	} D;
	struct GMTGET_G {	/* -Gfilename */
		bool active;
		char *file;
	} G;
	struct GMTGET_I {	/* -I<inc>*/
		bool active;
		double inc;
	} I;
	struct GMTGET_L {	/* -L */
		bool active;
	} L;
	struct GMTGET_N {	/* -N */
		bool active;
	} N;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTGET_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTGET_CTRL);
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTGET_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [-D<download>] [-G<defaultsfile>] [-I<inc>] [-L] [-N] [PARAMETER1 PARAMETER2 PARAMETER3 ...] [%s]\n", name, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\n\tFor available PARAMETERS, see %s man page\n", GMT_SETTINGS_FILE);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Download data from the selected GMT server [%s]\n", API->GMT->session.DATASERVER);
	GMT_Message (API, GMT_TIME_NONE, "\t    Append one of the directories to download:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      cache: The entire contents of the cache directory.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      data: The entire contents of the data directory.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append =<planet> to only download the data/<planet> directory.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append =<dataset1,dataset2...> to only download the stated datasets.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    -Dall downloads both cache and all datasets.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set name of specific %s file to process.\n", GMT_SETTINGS_FILE);
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default looks for file in current directory.  If not found,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   it looks in the home directory, if not found it uses the GMT defaults].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Limit the download of data sets to grid spacings of <inc> or larger [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Write one parameter value per line [Default writes all on one line].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do NOT convert grids downloaded with -D to netCDF but leave as JP2.\n");
	GMT_Option (API, "V,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GMTGET_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtget and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files: Here, arguments like ANNOT_OFFSET = 1i will show up as "files" or numbers */
			case '#':
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Optional defaults file on input and output */
				Ctrl->D.active = true;
				if (opt->arg[0]) Ctrl->D.dir = strdup (opt->arg);
				break;
			case 'G':	/* Optional defaults file on input and output */
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (GMT->parent, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->G.file))) n_errors++;
				break;
			case 'I':	/* Set increment limitation */
				Ctrl->I.active = true;
				if (gmt_getincn (GMT, opt->arg, &Ctrl->I.inc, 1) != 1)
					n_errors++;

				break;
			case 'L':	/* One per line */
				Ctrl->L.active = true;
				break;
			case 'N':	/* Leave JP2 as is */
				Ctrl->N.active = true;
				break;


			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->G.active,
	                                 "Option -D: Cannot be used with -G\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->L.active,
	                                 "Option -D: Cannot be used with -L\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

EXTERN_MSC void gmt_free_list (struct GMT_CTRL *GMT, char **list, uint64_t n);

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gmtget (void *V_API, int mode, void *args) {
	int error = GMT_NOERROR;
	char *datasets = NULL, *c = NULL, file[GMT_LEN64] = {""};

	struct GMTGET_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the gmtget main code ----------------------------*/


	if (Ctrl->D.active) {	/* Data download */
		if (!strncmp (Ctrl->D.dir, "all", 3U) || !strncmp (Ctrl->D.dir, "data", 4U)) {	/* Want data */
			bool found;
			unsigned int n_tiles, k, d = 0, t, n_items = 0;
			char **list = NULL, *string = NULL, *token = NULL, *tofree = NULL;
			double world[4] = {-180.0, +180.0, -90.0, +90.0};

			if (Ctrl->N.active) GMT->current.io.leave_as_jp2 = true;	/* Do not convert to netCDF right away */
			if ((c = strchr (Ctrl->D.dir, '=')) && (datasets = &c[1])) {	/* But only one or more specific planets or datasets */
				n_items = gmt_char_count (datasets, ',') + 1;
				list = gmt_M_memory (GMT, NULL, n_items, char *);
				tofree = string = strdup (datasets);
				while ((token = strsep (&string, ",")) != NULL)
					list[d++] = strdup (token);
				gmt_M_str_free (tofree);
			}
			for (k = 0; k < API->n_remote_info; k++) {
				if (n_items) {	/* Want specific planet or dataset */
					for (d = 0, found = false; !found && d < n_items; d++)
						if (strstr (API->remote_info[k].dir, list[d])) found = true;

					if (!found) continue;	/* Not this planet or dataset */
				}
				if (Ctrl->I.active && Ctrl->I.inc > API->remote_info[k].d_inc) continue;	/* Skip this resolution */
				if (API->remote_info[k].tile_size > 0.0) {	/* Must obtain all tiles */
					char **list = gmt_get_dataset_tiles (API, world, k, &n_tiles);
					for (t = 0; t < n_tiles; t++)
						gmt_download_file_if_not_found (GMT, list[t], GMT_AUTO_DIR);
					gmt_free_list (GMT, list, n_tiles);
				}
				else {
					sprintf (file, "@%s", API->remote_info[k].file);
					gmt_download_file_if_not_found (GMT, file, GMT_AUTO_DIR);
				}
			}
			if (list) gmt_free_list (GMT, list, n_items);
		}
		if (!strncmp (Ctrl->D.dir, "all", 3U) || !strncmp (Ctrl->D.dir, "cache", 5U)) {	/* Want cache */
			char line[GMT_LEN256] = {""}, hashpath[PATH_MAX] = {""};
			FILE *fp = NULL;
			if (access (GMT->session.CACHEDIR, R_OK) && gmt_mkdir (GMT->session.CACHEDIR)) {	/* Have or just made a server subdirectory */
				GMT_Report (API, GMT_MSG_NOTICE, "Unable to create or find your cache directory\n");
				Return (GMT_RUNTIME_ERROR);
			}
			/* Read gmt_hash_server.txt, loop over lines, call gmt_download */
			snprintf (hashpath, PATH_MAX, "%s/server/%s", GMT->session.USERDIR, GMT_HASH_SERVER_FILE);
			if ((fp = fopen (hashpath, "r")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to access or read %s\n", hashpath);
				Return (GMT_RUNTIME_ERROR);
			}
			fgets (line, GMT_LEN256, fp);	/* Skip first record with record count */
			while (fscanf (fp, "%s %*s %*s", line) == 1) {
				sprintf (file, "@%s", line);
				gmt_download_file_if_not_found (GMT, file, GMT_AUTO_DIR);
			}
			fclose (fp);
		}

		Return (GMT_NOERROR);
	}

	/* Read the supplied default file or the users defaults to override system settings */

	if (Ctrl->G.active || API->external) gmt_getdefaults (GMT, Ctrl->G.file);	/* Update defaults if using external API */

	error = gmt_pickdefaults (GMT, Ctrl->L.active, options);		/* Process command line arguments */

	Return (error);
}
