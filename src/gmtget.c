/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
	struct GMTGET_Q {	/* -Q */
		bool active;
	} Q;
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
	GMT_Usage (API, 0, "usage: %s [-D<download>] [-G<defaultsfile>] [-I<inc>] [-L] [-N] [-Q] [%s] [PARAMETER1 PARAMETER2 PARAMETER3 ...]\n", name, GMT_V_OPT);
	GMT_Usage (API, 1, "Note: For available PARAMETERS, see %s documentation", GMT_SETTINGS_FILE);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-D<download>");
	GMT_Usage (API, -2, "Download data from the selected GMT server [%s]. Append one of the directories to download:", API->GMT->session.DATASERVER);
	GMT_Usage (API, 3, "cache: The entire contents of the cache directory.");
	GMT_Usage (API, 3, "data: The entire contents of the data directory. "
		"Append =<planet> to only download the data/<planet> directory, or "
		"append =<dataset1,dataset2...> to only download the stated datasets.");
	GMT_Usage (API, -2, "Alternatively, -Dall downloads both cache and all datasets. "
		"Note: Run \"gmt docs data\" to learn about available data sets.");
	GMT_Usage (API, 1, "\n-G<defaultsfile>");
	GMT_Usage (API, -2, "Set name of specific %s file to process "
		"Default looks for file in current directory.  If not found, n"
		"it looks in the home directory, if not found it uses the GMT defaults].", GMT_SETTINGS_FILE);
	GMT_Usage (API, 1, "\n-I<inc>");
	GMT_Usage (API, -2, "Limit the download of data sets to grid spacing of <inc> or larger [0].");
	GMT_Usage (API, 1, "\n-L Write one parameter value per line [Default writes all on one line].");
	GMT_Usage (API, 1, "\n-N Do NOT convert grids downloaded with -D to netCDF but leave as JP2.");
	GMT_Usage (API, 1, "\n-Q In conjunction with -D, will list but not download the selected data.");
	GMT_Option (API, "V");

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
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files: Here, arguments like ANNOT_OFFSET = 1i will show up as "files" or numbers */
			case '#':
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Optional defaults file on input and output */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				if (opt->arg[0]) Ctrl->D.dir = strdup (opt->arg);
				break;
			case 'G':	/* Optional defaults file on input and output */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->G.file))) n_errors++;
				break;
			case 'I':	/* Set increment limitation */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				if (gmt_getincn (GMT, opt->arg, &Ctrl->I.inc, 1) != 1)
					n_errors++;

				break;
			case 'L':	/* One per line */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.active = true;
				break;
			case 'N':	/* Leave JP2 as is */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				break;
			case 'Q':	/* Report data sets available */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				Ctrl->Q.active = true;
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
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->D.dir == NULL,
	                                 "Option -D: Requires a selection\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && !Ctrl->D.active,
	                                 "Option -Q: Requires -D\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && Ctrl->N.active,
	                                 "Option -Q: -N will be ignored\n");
	if (Ctrl->D.active && Ctrl->D.dir && !(!strcmp (Ctrl->D.dir, "all") || !strcmp (Ctrl->D.dir, "cache") || !strncmp (Ctrl->D.dir, "data", 4U))) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -D: Requires arguments all, cache, data[=<planet>] or data=<datasetlist>\n");
		n_errors++;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

EXTERN_MSC void gmt_free_list (struct GMT_CTRL *GMT, char **list, uint64_t n);

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gmtget (void *V_API, int mode, void *args) {
	int error = GMT_NOERROR;
	char *datasets = NULL, *c = NULL, file[PATH_MAX] = {""};

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


	if (Ctrl->D.active) {	/* Remote data download */
		gmt_refresh_server (API);	/* Refresh hash and info tables as needed since we need to know what is there */

		if (!strncmp (Ctrl->D.dir, "all", 3U) || !strncmp (Ctrl->D.dir, "data", 4U)) {	/* Want data */
			bool found;
			int k;
			unsigned int n_tiles, d = 0, t, n_items = 0, n;
			char **list = NULL, *string = NULL, *token = NULL, *tofree = NULL;
			char planet[GMT_LEN32] = {""}, group[GMT_LEN32] = {""}, dataset[GMT_LEN64] = {""}, size[GMT_LEN32] = {""}, message[GMT_LEN256] = {""};
			double world[4] = {-180.0, +180.0, -90.0, +90.0};
			struct GMT_RECORD *Out = NULL;

			if (Ctrl->Q.active) {	/* Must activate data output machinery for a DATASET with no numerical columns */
				Out = gmt_new_record (GMT, NULL, message);
				if ((error = GMT_Set_Columns (API, GMT_OUT, 0, GMT_COL_FIX)) != GMT_NOERROR) {
					gmt_M_free (GMT, Out);
					Return (API->error);
				}
				if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
					gmt_M_free (GMT, Out);
					Return (API->error);
				}
				if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {
					gmt_M_free (GMT, Out);
					Return (API->error);
				}
				if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_NONE) != GMT_NOERROR) {	/* Sets output geometry */
					gmt_M_free (GMT, Out);
					Return (API->error);
				}
			}

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
				if (Ctrl->Q.active) {	/* Reporting only */
					strcpy (message, API->remote_info[k].dir);
					gmt_strrepc (message, '/', ' ');	/* Turn slashes to spaces */
					sscanf (message, "%*s %s %s", planet, group);
					strcpy (dataset, API->remote_info[k].file);
					if (dataset[strlen(dataset)-1] == '/') {	/* Tiles */
						dataset[strlen(dataset)-1] = '\0';	/* Chop off slash */
						strcpy (size, "N/A");
						n = (API->remote_info[k].inc[2] == 's' && strchr ("13", API->remote_info[k].inc[1])) ? 14297 : urint (360.0 * 180.0 / (API->remote_info[k].tile_size * API->remote_info[k].tile_size));
					}
					else {
						(void) gmt_chop_ext (dataset);
						strcpy (size, API->remote_info[k].size);
						n = 1;
					}
					message[0] = '\0';
					snprintf (message, GMT_LEN256-1, "%s\t%s\t%s\t%s\t%u\t%s", planet, group, dataset, size, n, API->remote_info[k].remark);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
				else {
					if (API->remote_info[k].tile_size > 0.0) {	/* Must obtain all tiles */
						char **list = gmt_get_dataset_tiles (API, world, k, &n_tiles, NULL);
						for (t = 0; t < n_tiles; t++)
							gmt_download_file_if_not_found (GMT, list[t], GMT_AUTO_DIR);
						gmt_free_list (GMT, list, n_tiles);
					}
					else {
						sprintf (file, "@%s", API->remote_info[k].file);
						gmt_download_file_if_not_found (GMT, file, GMT_AUTO_DIR);
					}
				}
			}
			if (list) gmt_free_list (GMT, list, n_items);
			if (Ctrl->Q.active) {	/* Terminate i/o */
				gmt_M_free (GMT, Out);
				if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {
					Return (API->error);
				}
			}
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

	if (Ctrl->G.active || API->external) {
		if (gmt_getdefaults (GMT, Ctrl->G.file) && Ctrl->G.file) {	/* Update defaults if using external API */
			GMT_Report (API, GMT_MSG_ERROR, "Unable to access or read %s\n", Ctrl->G.file);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	error = gmt_pickdefaults (GMT, Ctrl->L.active, options);		/* Process command line arguments */

	Return (error);
}
