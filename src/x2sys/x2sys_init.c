/*-----------------------------------------------------------------
 *
 *      Copyright (c) 1999-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *      Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/* x2sys_init will accept command line options to specify track data
 * file formats, region of interest, bin spacing, etc, and a unique
 * system identifier.  It will then initialize the databases associated
 * with this system where information about the tracks and their
 * intersections will be stored.
 *
 * Author:	Paul Wessel
 * Date:	14-JUN-2004
 * Version:	1.1, based on the spirit of the old mgg/s_system code
 *
 */

#include "gmt_dev.h"
#include "mgd77/mgd77.h"
#include "x2sys.h"

#define THIS_MODULE_CLASSIC_NAME	"x2sys_init"
#define THIS_MODULE_MODERN_NAME	"x2sys_init"
#define THIS_MODULE_LIB		"x2sys"
#define THIS_MODULE_PURPOSE	"Initialize a new x2sys track database"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->RVj"

EXTERN_MSC void x2sys_set_home (struct GMT_CTRL *GMT);

struct X2SYS_INIT_CTRL {
	struct In {	/*  */
		bool active;
		char *TAG;
	} In;
	struct C {	/* -C [Deprecated, now use -j] */
		bool active;
		char *string;
	} C;
	struct D {	/* -D */
		bool active;
		char *file;
	} D;
	struct E {	/* -E */
		bool active;
		char *string;
	} E;
	struct F {	/* -F */
		bool active;
	} F;
	struct G {	/* -G */
		bool active;
		char *string;
	} G;
	struct I {	/* -I */
		bool active;
		double inc[2];
		char *string;
	} I;
	struct m {	/* -m */
		bool active;
		char *string;
	} m;
	struct N {	/* -N */
		bool active[2];
		char *string[2];
	} N;
	struct W {	/* -W */
		bool active[2];
		char *string[2];
	} W;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_INIT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct X2SYS_INIT_CTRL);

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_INIT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.TAG);
	gmt_M_str_free (C->C.string);
	gmt_M_str_free (C->D.file);
	gmt_M_str_free (C->E.string);
	gmt_M_str_free (C->G.string);
	gmt_M_str_free (C->I.string);
	gmt_M_str_free (C->m.string);
	gmt_M_str_free (C->N.string[0]);
	gmt_M_str_free (C->N.string[1]);
	gmt_M_str_free (C->W.string[0]);
	gmt_M_str_free (C->W.string[1]);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
#ifdef WIN32
	static char *par = "%X2SYS_HOME%";
#else
	static char *par = "$X2SYS_HOME";
#endif
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <TAG> [-D<deffile>] [-E<suffix>] [-F] [-G[d|g]] [-I[<binsize>]]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-N[d|s][c|e|f|k|M|n]]] [%s] [%s] [-Wt|d|n<gap>]\n\t[-m] [%s]] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_j_OPT, GMT_PAR_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t<TAG> is the unique system identifier.  Files created will be placed in the directory %s/<TAG>.\n", par);
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: The environmental parameter %s must be defined.\n\n", par);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Definition file for the track data set [<TAG>.%s].\n", X2SYS_FMT_EXT);
	GMT_Message (API, GMT_TIME_NONE, "\t   (Note: deprecated extension .%s will work but consider renaming the file)\n", X2SYS_FMT_EXT_OLD);
	GMT_Message (API, GMT_TIME_NONE, "\t-E Extension (suffix) for these data files\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default equals the prefix for the definition file].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Force creating new files if old ones are present [Default will abort if old files are found].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Geographical coordinates; append g for discontinuity at Greenwich (output 0/360 [Default])\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or append d for discontinuity at Dateline (output -180/+180).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Set bin size for track bin index output [1/1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Append (d)istances or (s)peed, and your choice for unit. Choose among:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c Cartesian distance (user-dist-units, user-dist-units/user-time-units).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   e Metric units I (meters, m/s).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f British/US I (feet, feet/s).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   k Metric units II (km, km/hr).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   M British/US units II (miles, miles/hr).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n Nautical units (nautical miles, knots).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   u Old US units (survey feet, survey feet/s).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   See -j for distance calculation modes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is -Ndk -Nse].\n");
	GMT_Option (API, "R");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   [Default region is 0/360/-90/90].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set maximum gaps allowed at crossover.  Option may be repeated.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wt sets maximum time gap (in user units) [Default is infinite].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wd sets maximum distance gap (in user units) [Default is infinite].\n");
	GMT_Option (API, "j,m,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct X2SYS_INIT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n_tags = 0;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	/* We are just checking the options for syntax here, not parsing is actually needed */

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */
		k = 0;
		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Tags */
				if (n_tags == 0) Ctrl->In.TAG = strdup (opt->arg);
				n_tags++;
				break;

			/* Processes program-specific parameters */
			
			case 'C':	/* Distance calculation flag */
				if (gmt_M_compat_check (API->GMT, 6)) {
					GMT_Report (API, GMT_MSG_COMPAT, "The -C option is deprecated; use the GMT common option -j<mode> instead\n");
					Ctrl->C.active = true;
					if (!strchr ("cefg", (int)opt->arg[0])) {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C: Flag must be c, f, g, or e\n");
						n_errors++;
					}
					if (!n_errors) Ctrl->C.string = strdup (opt->arg);
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Unrecognized option -C\n");
					n_errors++;
				}
				break;
			case 'D':
				Ctrl->D.active = true;
				if ((c = strstr (opt->arg, "." X2SYS_FMT_EXT)) == NULL && (c = strstr (opt->arg, "." X2SYS_FMT_EXT_OLD)) == NULL)
					Ctrl->D.file = strdup (opt->arg);	/* Gave no extension so store everything */
				else {	/* Must avoid the extension */
					c[0] = '\0';	/* Chop off extension */
					Ctrl->D.file = strdup (opt->arg);
					c[0] = '.';	/* Restore extension */
				}
				break;
			case 'E':
				Ctrl->E.active = true;
				Ctrl->E.string = strdup (opt->arg);
				break;
			case 'G':	/* Geographical coordinates, set discontinuity */
				Ctrl->G.active = true;
				Ctrl->G.string = strdup (opt->arg);
				break;
			case 'F':
				Ctrl->F.active = true;
				break;
			case 'I':
				Ctrl->I.active = true;
				if (opt->arg[0] && gmt_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					gmt_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				Ctrl->I.string = strdup (opt->arg);
				break;
			case 'm':
				Ctrl->m.active = true;
				Ctrl->m.string = strdup (opt->arg);
				break;
			case 'N':	/* Distance and speed unit selection */
				switch (opt->arg[0]) {
					case 'd':	/* Distance unit selection */
						k = 1;
						break;
					case 's':	/* Speed unit selection */
						if (!strchr ("c" GMT_LEN_UNITS2, (int)opt->arg[1])) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N%c: Unit must among c|%s\n", opt->arg[0], GMT_LEN_UNITS2_DISPLAY);
							n_errors++;
						}
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N: Choose from -Nd and -Ns\n");
						n_errors++;
						break;
				}
				if (!n_errors) {
					Ctrl->N.active[k] = true;
					Ctrl->N.string[k] = strdup (opt->arg);
				}
				break;
			case 'W':
				switch (opt->arg[0]) {
					case 'd':	/* Get new distgap */
						k = 1;
					case 't':	/* Get new timegap */
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: -Wt|d<width>\n");
						n_errors++;
						break;
				}
				if (!n_errors) {
					Ctrl->W.active[k] = true;
					Ctrl->W.string[k] = strdup (opt->arg);
				}
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_tags == 0, "Syntax error: No system tag given!\n");
	n_errors += gmt_M_check_condition (GMT, n_tags > 1, "Syntax error: Only give one system tag!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0), "Syntax error: -Idx/dy must be positive!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_x2sys_init (void *V_API, int mode, void *args) {

	time_t right_now;
	char tag_file[PATH_MAX] = {""}, track_file[PATH_MAX] = {""}, bin_file[PATH_MAX] = {""}, def_file[PATH_MAX] = {""};
	char path_file[PATH_MAX] = {""}, path[PATH_MAX] = {""}, line[GMT_BUFSIZ] = {""};
	char *name = NULL;

	int error = 0;

	FILE *fp = NULL, *fp_def = NULL;

	int n_found = 0, d_start;

	struct X2SYS_INIT_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the x2sys_init main code ----------------------------*/

	if (!Ctrl->D.active) Ctrl->D.file = strdup (Ctrl->In.TAG);	/* Default */
	sprintf (def_file, "%s.%s", Ctrl->D.file, X2SYS_FMT_EXT);
	if (access (def_file, R_OK)) {	/* No such local *.fmt file */
		if (!gmt_getsharepath (GMT, "x2sys", Ctrl->D.file, "." X2SYS_FMT_EXT, def_file, R_OK)) {	/* Not found in GMT x2sys share path */
			/* Try old deprecated extension */
			sprintf (def_file, "%s.%s", Ctrl->D.file, X2SYS_FMT_EXT_OLD);
			if (access (def_file, R_OK)) {	/* No such local *.def file */
				if (!gmt_getsharepath (GMT, "x2sys", Ctrl->D.file, "." X2SYS_FMT_EXT_OLD, def_file, R_OK)) {	/* Not found in GMT x2sys share path */
					GMT_Report (API, GMT_MSG_NORMAL, "Unable to find definition file : %s\n", def_file);
					Return (GMT_ERROR_ON_FOPEN);
				}
			}
			GMT_Report (API, GMT_MSG_VERBOSE, "Found deprecated file extension .%s, please rename to .%s\n", X2SYS_FMT_EXT_OLD, X2SYS_FMT_EXT);
		}
	}
	if ((fp_def = fopen (def_file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to open local definition file : %s\n", def_file);
		Return (GMT_ERROR_ON_FOPEN);
	}
	for (d_start = (int)strlen (Ctrl->D.file)-1; d_start >= 0 && Ctrl->D.file[d_start] != '/'; d_start--);	/* Find pos of last slash */
	d_start++;		/* Find start of file name */
	
	/* Determine the TAG directory */
	
	x2sys_set_home (GMT);
	x2sys_path (GMT, Ctrl->In.TAG, path);
	if (x2sys_access (GMT, Ctrl->In.TAG, R_OK)) {	/* No such dir */
		if (gmt_mkdir (path)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create TAG directory : %s\n", path);
			fclose (fp_def);	/* Close local def file */
			Return (GMT_RUNTIME_ERROR);
		}
	}
	else if (!Ctrl->F.active) {	/* Directory exists but -F not on */
		GMT_Report (API, GMT_MSG_VERBOSE, "TAG directory already exists: %s\n", path);
		fclose (fp_def);	/* Close local def file */
		Return (GMT_RUNTIME_ERROR);
	}
	
	/* Initialize the system TAG files in X2SYS_HOME/TAG */

	sprintf (tag_file,   "%s/%s.tag",       Ctrl->In.TAG, Ctrl->In.TAG);
	sprintf (def_file,   "%s/%s.%s",       Ctrl->In.TAG, &Ctrl->D.file[d_start], X2SYS_FMT_EXT);
	sprintf (path_file,  "%s/%s_paths.txt", Ctrl->In.TAG, Ctrl->In.TAG);
	sprintf (track_file, "%s/%s_tracks.d",  Ctrl->In.TAG, Ctrl->In.TAG);
	sprintf (bin_file,   "%s/%s_index.b",   Ctrl->In.TAG, Ctrl->In.TAG);

	if (!x2sys_access (GMT, tag_file, R_OK)) {
		GMT_Report (API, GMT_MSG_NORMAL, "File exists: %s\n", tag_file);
		x2sys_path (GMT, tag_file, path);
		if (Ctrl->F.active) {
			if (gmt_remove_file (GMT, path))
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to remove %s\n", path);
			else
				GMT_Report (API, GMT_MSG_VERBOSE, "Removed file %s\n", path);
		}
		else 
			n_found++;
	}
	if (!x2sys_access (GMT, def_file, R_OK)) {
		GMT_Message (API, GMT_TIME_NONE, "File exists: %s\n", def_file);
		x2sys_path (GMT, def_file, path);
		if (Ctrl->F.active) {
			if (gmt_remove_file (GMT, path))
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to remove %s\n", path);
			else
				GMT_Report (API, GMT_MSG_VERBOSE, "Removed file %s\n", path);
		}
		else 
			n_found++;
	}
	if (!x2sys_access (GMT, track_file, R_OK)) {
		GMT_Message (API, GMT_TIME_NONE, "File exists: %s\n", track_file);
		x2sys_path (GMT, track_file, path);
		if (Ctrl->F.active) {
			if (gmt_remove_file (GMT, path))
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to remove %s\n", path);
			else
				GMT_Report (API, GMT_MSG_VERBOSE, "Removed file %s\n", path);
		}
		else 
			n_found++;
	}
	if (!x2sys_access (GMT, path_file, R_OK)) {
		GMT_Message (API, GMT_TIME_NONE, "File exists: %s\n", path_file);
		x2sys_path (GMT, path_file, path);
		if (Ctrl->F.active) {
			if (gmt_remove_file (GMT, path))
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to remove %s\n", path);
			else
				GMT_Report (API, GMT_MSG_VERBOSE, "Removed file %s\n", path);
		}
		else 
			n_found++;
	}
	if (!x2sys_access (GMT, bin_file, R_OK)) {
		GMT_Report (API, GMT_MSG_NORMAL, "File exists: %s\n", bin_file);
		x2sys_path (GMT, bin_file, path);
		if (Ctrl->F.active) {
			if (gmt_remove_file (GMT, path))
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to remove %s\n", path);
			else
				GMT_Report (API, GMT_MSG_VERBOSE, "Removed file %s\n", path);
		}
		else 
			n_found++;
	}
	if (n_found) {
		GMT_Report (API, GMT_MSG_NORMAL, "Remove/rename old files or use -F to overwrite\n");
		fclose (fp_def);	/* Close local def file */
		Return (GMT_RUNTIME_ERROR);
	}


	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Initialize %s\n", tag_file);
	if ((fp = x2sys_fopen (GMT, tag_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Could not create file %s\n", tag_file);
		fclose (fp_def);	/* Close local def file */
		Return (GMT_ERROR_ON_FOPEN);
	}

	name = gmt_putusername(NULL);
	right_now = time ((time_t *)0);
	fprintf (fp, "# TAG file for system: %s\n", Ctrl->In.TAG);
	fprintf (fp, "#\n# Initialized on: %s", ctime (&right_now));
	fprintf (fp, "# Initialized by: %s\n#\n", name);
	gmt_M_str_free (name);
	fprintf (fp, "-D%s", &Ctrl->D.file[d_start]);	/* Now a local *.def file in the TAG directory */
	if (GMT->common.j.active) fprintf (fp, " -j%s", GMT->common.j.string);
	else if (Ctrl->C.active) fprintf (fp, " -j%s", Ctrl->C.string);
	if (Ctrl->E.active) fprintf (fp, " -E%s", Ctrl->E.string);
	if (Ctrl->G.active) fprintf (fp, " -G%s", Ctrl->G.string);
	if (Ctrl->m.active) fprintf (fp, " -m%s", Ctrl->m.string);
	if (Ctrl->N.active[0]) fprintf (fp, " -N%s", Ctrl->N.string[0]);
	if (Ctrl->N.active[1]) fprintf (fp, " -N%s", Ctrl->N.string[1]);
	if (Ctrl->W.active[0]) fprintf (fp, " -W%s", Ctrl->W.string[0]);
	if (Ctrl->W.active[1]) fprintf (fp, " -W%s", Ctrl->W.string[1]);
	(Ctrl->I.active) ? fprintf (fp, " -I%s", Ctrl->I.string) : fprintf (fp, " -I1/1");
	(GMT->common.R.active[RSET]) ? fprintf (fp, " -R%g/%g/%g/%g", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]) : fprintf (fp, " -R0/360/-90/90");
	fprintf (fp, "\n");
	x2sys_err_fail (GMT, x2sys_fclose (GMT, tag_file, fp), tag_file);

	/* Initialize the system's definition file  */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Initialize %s\n", def_file);
	if ((fp = x2sys_fopen (GMT, def_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Could not create %s\n", def_file);
		fclose (fp_def);	/* Close local def file */
		Return (GMT_ERROR_ON_FOPEN);
	}
	while (fgets (line, GMT_BUFSIZ, fp_def)) fprintf (fp, "%s", line);
	x2sys_err_fail (GMT, x2sys_fclose (GMT, def_file, fp), def_file);
	fclose (fp_def);	/* Close local def file */

	/* Initialize the system's tracks data base  */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Initialize %s\n", track_file);
	if ((fp = x2sys_fopen (GMT, track_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Could not create %s\n", track_file);
		Return (GMT_ERROR_ON_FOPEN);
	}
	fprintf (fp,"# %s\n", Ctrl->In.TAG);	/* Write header record to empty track file */
	
	x2sys_err_fail (GMT, x2sys_fclose (GMT, track_file, fp), track_file);

	/* Initialize the system's index data base  */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Initialize %s\n", bin_file);
	if ((fp = x2sys_fopen (GMT, bin_file, "wb")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Could not create %s\n", bin_file);
		Return (GMT_ERROR_ON_FOPEN);
	}
	x2sys_err_fail (GMT, x2sys_fclose (GMT, bin_file, fp), bin_file);

	/* Initialize the system's track path file  */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Initialize %s\n", path_file);
	if ((fp = x2sys_fopen (GMT, path_file, "wb")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Could not create %s\n", path_file);
		Return (GMT_ERROR_ON_FOPEN);
	}
	fprintf (fp, "# Directories with data files for TAG %s\n", Ctrl->In.TAG);
	fprintf (fp, "# The current directory is always searched first.\n");
	fprintf (fp, "# Add full paths to search additional directories\n");
	x2sys_err_fail (GMT, x2sys_fclose (GMT, path_file, fp), path_file);
	
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "completed successfully\n");

	gmt_M_free (GMT, X2SYS_HOME);

	Return (GMT_NOERROR);
}
