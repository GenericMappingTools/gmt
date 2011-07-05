/*-----------------------------------------------------------------
 *	$Id: x2sys_init_func.c,v 1.16 2011-07-05 23:45:47 guru Exp $
 *
 *      Copyright (c) 1999-2011 by P. Wessel
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      Contact info: gmt.soest.hawaii.edu
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

#include "x2sys.h"
extern void x2sys_set_home (struct GMT_CTRL *GMT);

struct X2SYS_INIT_CTRL {
	struct In {	/*  */
		GMT_LONG active;
		char *TAG;
	} In;
	struct C {	/* -C */
		GMT_LONG active;
		char *string;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
		char *file;
	} D;
	struct E {	/* -E */
		GMT_LONG active;
		char *string;
	} E;
	struct F {	/* -F */
		GMT_LONG active;
	} F;
	struct G {	/* -G */
		GMT_LONG active;
		char *string;
	} G;
	struct I {	/* -I */
		GMT_LONG active;
		double inc[2];
		char *string;
	} I;
	struct m {	/* -m */
		GMT_LONG active;
		char *string;
	} m;
	struct N {	/* -N */
		GMT_LONG active[2];
		char *string[2];
	} N;
	struct W {	/* -W */
		GMT_LONG active[2];
		char *string[2];
	} W;
};

void *New_x2sys_init_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_INIT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_INIT_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return ((void *)C);
}

void Free_x2sys_init_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_INIT_CTRL *C) {	/* Deallocate control structure */
	if (C->In.TAG) free ((void *)C->In.TAG);
	if (C->C.string) free ((void *)C->C.string);
	if (C->D.file) free ((void *)C->D.file);
	if (C->E.string) free ((void *)C->E.string);
	if (C->G.string) free ((void *)C->G.string);
	if (C->I.string) free ((void *)C->I.string);
	if (C->m.string) free ((void *)C->m.string);
	if (C->N.string[0]) free ((void *)C->N.string[0]);
	if (C->N.string[1]) free ((void *)C->N.string[1]);
	if (C->W.string[0]) free ((void *)C->W.string[0]);
	if (C->W.string[1]) free ((void *)C->W.string[1]);
	GMT_free (GMT, C);
}

GMT_LONG GMT_x2sys_init_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "x2sys_init %s - Initialize a new x2sys track database\n\n", X2SYS_VERSION);
	GMT_message (GMT, "usage: x2sys_init <TAG> [-Cc|f|g|e] [-D<deffile>] [-E<suffix>] [-F] [-G[d/g]] [-I[<binsize>]]\n");
	GMT_message (GMT, "\t[-N[d|s][c|e|f|k|M|n]]] [%s] [%s] [-Wt|d|n<gap>] [-m]\n\n", GMT_Rgeo_OPT, GMT_V_OPT);
	GMT_message (GMT, "\t<TAG> is the unique system identifier.  Files created will be placed in\n");
	GMT_message (GMT, "\t   the directory X2SYS_HOME/<TAG>.\n");

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-C Select procedure for along-track distance and azimuth calculations:\n");
	GMT_message (GMT, "\t   c Plain Cartesian.\n");
	GMT_message (GMT, "\t   f Flat Earth.\n");
	GMT_message (GMT, "\t   g Great circle [Default].\n");
	GMT_message (GMT, "\t   e Ellipsoidal (geodesic) using current ellipsoid.\n");
	GMT_message (GMT, "\t-D Definition file for the track data set [<TAG>.def].\n");
	GMT_message (GMT, "\t-E Extension (suffix) for these data files\n");
	GMT_message (GMT, "\t   [Default equals the prefix for the definition file].\n");
	GMT_message (GMT, "\t-F Force creating new files if old ones are present [Default will abort if old files are found].\n");
	GMT_message (GMT, "\t-G Geographical coordinates; append g for discontinuity at Greenwich (output 0/360 [Default])\n");
	GMT_message (GMT, "\t   and append d for discontinuity at Dateline (output -180/+180).\n");
	GMT_message (GMT, "\t-I Set bin size for track bin index output [1/1].\n");
	GMT_message (GMT, "\t-N Append (d)istances or (s)peed, and your choice for unit. Choose among:\n");
	GMT_message (GMT, "\t   c Cartesian distance (user-dist-units, user user-dist-units/user-time-units).\n");
	GMT_message (GMT, "\t   e Metric units I (meters, m/s).\n");
	GMT_message (GMT, "\t   f British/US I (feet, feet/s).\n");
	GMT_message (GMT, "\t   k Metric units II (km, km/hr).\n");
	GMT_message (GMT, "\t   M British/US units II (miles, miles/hr).\n");
	GMT_message (GMT, "\t   n Nautical units (nautical miles, knots).\n");
	GMT_message (GMT, "\t   [Default is -Ndk -Nse].\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t   [Default region is 0/360/-90/90].\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Set maximum gaps allowed at crossover.  Option may be repeated.\n");
	GMT_message (GMT, "\t   -Wt sets maximum time gap (in user units) [Default is infinite].\n");
	GMT_message (GMT, "\t   -Wd sets maximum distance gap (in user units) [Default is infinite].\n");
	GMT_explain_options (GMT, "m");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_x2sys_init_parse (struct GMTAPI_CTRL *C, struct X2SYS_INIT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, k, n_tags = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;
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
				Ctrl->C.active = TRUE;
				if (!strchr ("cefg", (int)opt->arg[0])) {
					GMT_report (GMT, GMT_MSG_FATAL, "ERROR -C: Flag must be c, f, g, or e\n");
					n_errors++;
				}
				if (!n_errors) Ctrl->C.string = strdup (opt->arg);
				break;
			case 'D':
				Ctrl->D.active = TRUE;
				Ctrl->D.file = strdup (opt->arg);
				break;
			case 'E':
				Ctrl->E.active = TRUE;
				Ctrl->E.string = strdup (opt->arg);
				break;
			case 'G':	/* Geographical coordinates, set discontinuity */
				Ctrl->G.active = TRUE;
				Ctrl->G.string = strdup (opt->arg);
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (opt->arg[0]) GMT_getinc (GMT, opt->arg, Ctrl->I.inc);
				Ctrl->I.string = strdup (opt->arg);
				break;
			case 'm':
				Ctrl->m.active = TRUE;
				Ctrl->m.string = strdup (opt->arg);
				break;
			case 'N':	/* Distance and speed unit selection */
				switch (opt->arg[0]) {
					case 'd':	/* Distance unit selection */
						k = 1;
					case 's':	/* Speed unit selection */
						if (!strchr ("cefkMn", (int)opt->arg[1])) {
							GMT_report (GMT, GMT_MSG_FATAL, "ERROR -N%c: Unit must be c, e, f, k, M, or n\n", opt->arg[0]);
							n_errors++;
						}
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "ERROR -N: Choose from -Nd and -Ns\n");
						n_errors++;
						break;
				}
				if (!n_errors) {
					Ctrl->N.active[k] = TRUE;
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
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: -Wt|d<width>\n");
						n_errors++;
						break;
				}
				if (!n_errors) {
					Ctrl->W.active[k] = TRUE;
					Ctrl->W.string[k] = strdup (opt->arg);
				}
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_tags == 0, "Syntax error: No system tag given!\n");
	n_errors += GMT_check_condition (GMT, n_tags > 1, "Syntax error: Only give one system tag!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0), "Syntax error: -Idx/dy must be positive!\n");
	n_errors += GMT_check_condition (GMT, GMT->common.R.active && GMT_check_region (GMT, GMT->common.R.wesn), "Syntax error: -R given inconsistent values!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_x2sys_init_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_x2sys_init (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	time_t right_now;
	char tag_file[GMT_BUFSIZ], track_file[GMT_BUFSIZ], bin_file[GMT_BUFSIZ], def_file[GMT_BUFSIZ];
	char path_file[GMT_BUFSIZ], path[GMT_BUFSIZ], line[GMT_BUFSIZ];

	GMT_LONG error = FALSE;

	FILE *fp = NULL, *fp_def = NULL;

	int n_found = 0, d_start;

	struct X2SYS_INIT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_x2sys_init_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_x2sys_init_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_x2sys_init", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VR", ">", options))) Return (error);
	Ctrl = (struct X2SYS_INIT_CTRL *)New_x2sys_init_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_init_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the x2sys_init main code ----------------------------*/

	if (!Ctrl->D.active) Ctrl->D.file = strdup (Ctrl->In.TAG);	/* Default */
	sprintf (def_file, "%s.def", Ctrl->D.file);
	if (access (def_file, R_OK)) {	/* No such local *.def file */
		GMT_report (GMT, GMT_MSG_FATAL, "Unable to find local definition file : %s\n", def_file);
		Return (EXIT_FAILURE);
	}
	else if ((fp_def = fopen (def_file, "r")) == NULL) {
		GMT_report (GMT, GMT_MSG_FATAL, "Unable to open local definition file : %s\n", def_file);
		Return (EXIT_FAILURE);
	}
	for (d_start = (int)strlen (Ctrl->D.file)-1; d_start >= 0 && Ctrl->D.file[d_start] != '/'; d_start--);	/* Find pos of last slash */
	d_start++;		/* Find start of file name */
	
	/* Determine the TAG directory */
	
	x2sys_set_home (GMT);
	x2sys_path (GMT, Ctrl->In.TAG, path);
	if (x2sys_access (GMT, Ctrl->In.TAG, R_OK)) {	/* No such dir */
		if (mkdir (path, (mode_t)0777)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Unable to create TAG directory : %s\n", path);
			Return (EXIT_FAILURE);
		}
	}
	else if (!Ctrl->F.active) {	/* Directory exists but -F not on */
		GMT_report (GMT, GMT_MSG_FATAL, "TAG directory already exists: %s\n", path);
		Return (EXIT_FAILURE);
	}
	
	/* Initialize the system TAG files in X2SYS_HOME/TAG */

	sprintf (tag_file, "%s/%s.tag", Ctrl->In.TAG, Ctrl->In.TAG);
	sprintf (def_file, "%s/%s.def", Ctrl->In.TAG, &Ctrl->D.file[d_start]);
	sprintf (path_file, "%s/%s_paths.txt", Ctrl->In.TAG, Ctrl->In.TAG);
	sprintf (track_file, "%s/%s_tracks.d", Ctrl->In.TAG, Ctrl->In.TAG);
	sprintf (bin_file, "%s/%s_index.b", Ctrl->In.TAG, Ctrl->In.TAG);

	if (!x2sys_access (GMT, tag_file, R_OK)) {
		GMT_report (GMT, GMT_MSG_FATAL, "File exists: %s\n", tag_file);
		x2sys_path (GMT, tag_file, path);
		if (Ctrl->F.active) {
			if (remove (path))
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to remove %s\n", path);
			else
				GMT_report (GMT, GMT_MSG_FATAL, "Removed file %s\n", path);
		}
		else 
			n_found++;
	}
	if (!x2sys_access (GMT, def_file, R_OK)) {
		GMT_message (GMT, "File exists: %s\n", def_file);
		x2sys_path (GMT, def_file, path);
		if (Ctrl->F.active) {
			if (remove (path))
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to remove %s\n", path);
			else
				GMT_report (GMT, GMT_MSG_FATAL, "Removed file %s\n", path);
		}
		else 
			n_found++;
	}
	if (!x2sys_access (GMT, track_file, R_OK)) {
		GMT_message (GMT, "File exists: %s\n", track_file);
		x2sys_path (GMT, track_file, path);
		if (Ctrl->F.active) {
			if (remove (path))
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to remove %s\n", path);
			else
				GMT_report (GMT, GMT_MSG_FATAL, "Removed file %s\n", path);
		}
		else 
			n_found++;
	}
	if (!x2sys_access (GMT, path_file, R_OK)) {
		GMT_message (GMT, "File exists: %s\n", path_file);
		x2sys_path (GMT, path_file, path);
		if (Ctrl->F.active) {
			if (remove (path))
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to remove %s\n", path);
			else
				GMT_report (GMT, GMT_MSG_FATAL, "Removed file %s\n", path);
		}
		else 
			n_found++;
	}
	if (!x2sys_access (GMT, bin_file, R_OK)) {
		GMT_report (GMT, GMT_MSG_FATAL, "File exists: %s\n", bin_file);
		x2sys_path (GMT, bin_file, path);
		if (Ctrl->F.active) {
			if (remove (path))
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to remove %s\n", path);
			else
				GMT_report (GMT, GMT_MSG_FATAL, "Removed file %s\n", path);
		}
		else 
			n_found++;
	}
	if (n_found) {
		GMT_report (GMT, GMT_MSG_FATAL, "Remove/rename old files or use -F to overwrite\n");
		Return (EXIT_FAILURE);
	}


	GMT_report (GMT, GMT_MSG_NORMAL, "Initialize %s\n", tag_file);
	if ((fp = x2sys_fopen (GMT, tag_file, "w")) == NULL) {
		GMT_report (GMT, GMT_MSG_FATAL, "Could not create file %s\n", tag_file);
		Return (EXIT_FAILURE);
	}

        right_now = time ((time_t *)0);
	fprintf (fp, "# TAG file for system: %s\n", Ctrl->In.TAG);
	fprintf (fp, "#\n# Initialized on: %s", ctime (&right_now));
	fprintf (fp, "# Initialized by: %s\n#\n", GMT_putusername(GMT));
	fprintf (fp, "-D%s", &Ctrl->D.file[d_start]);	/* Now a local *.def file in the TAG directory */
	if (Ctrl->C.active) fprintf (fp, " -C%s", Ctrl->C.string);
	if (Ctrl->E.active) fprintf (fp, " -E%s", Ctrl->E.string);
	if (Ctrl->G.active) fprintf (fp, " -G%s", Ctrl->G.string);
	if (Ctrl->m.active) fprintf (fp, " -m%s", Ctrl->m.string);
	if (Ctrl->N.active[0]) fprintf (fp, " -N%s", Ctrl->N.string[0]);
	if (Ctrl->N.active[1]) fprintf (fp, " -N%s", Ctrl->N.string[1]);
	if (Ctrl->W.active[0]) fprintf (fp, " -W%s", Ctrl->W.string[0]);
	if (Ctrl->W.active[1]) fprintf (fp, " -W%s", Ctrl->W.string[1]);
	(Ctrl->I.active) ? fprintf (fp, " -I%s", Ctrl->I.string) : fprintf (fp, " -I1/1");
	(GMT->common.R.active) ? fprintf (fp, " -R%g/%g/%g/%g", GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]) : fprintf (fp, " -R0/360/-90/90");
	fprintf (fp, "\n");
	x2sys_err_fail (GMT, x2sys_fclose (GMT, tag_file, fp), tag_file);

	/* Initialize the system's definition file  */

	GMT_report (GMT, GMT_MSG_NORMAL, "Initialize %s\n", def_file);
	if ((fp = x2sys_fopen (GMT, def_file, "w")) == NULL) {
		GMT_report (GMT, GMT_MSG_FATAL, "Could not create %s\n", def_file);
		Return (EXIT_FAILURE);
	}
	while (fgets (line, GMT_BUFSIZ, fp_def)) fprintf (fp, "%s", line);
	x2sys_err_fail (GMT, x2sys_fclose (GMT, def_file, fp), def_file);
	fclose (fp_def);	/* Close local def file */

	/* Initialize the system's tracks data base  */

	GMT_report (GMT, GMT_MSG_NORMAL, "Initialize %s\n", track_file);
	if ((fp = x2sys_fopen (GMT, track_file, "w")) == NULL) {
		GMT_report (GMT, GMT_MSG_FATAL, "Could not create %s\n", track_file);
		Return (EXIT_FAILURE);
	}
	x2sys_err_fail (GMT, x2sys_fclose (GMT, track_file, fp), track_file);

	/* Initialize the system's index data base  */

	GMT_report (GMT, GMT_MSG_NORMAL, "Initialize %s\n", bin_file);
	if ((fp = x2sys_fopen (GMT, bin_file, "wb")) == NULL) {
		GMT_report (GMT, GMT_MSG_FATAL, "Could not create %s\n", bin_file);
		Return (EXIT_FAILURE);
	}
	x2sys_err_fail (GMT, x2sys_fclose (GMT, bin_file, fp), bin_file);

	/* Initialize the system's track path file  */

	GMT_report (GMT, GMT_MSG_NORMAL, "Initialize %s\n", path_file);
	if ((fp = x2sys_fopen (GMT, path_file, "wb")) == NULL) {
		GMT_report (GMT, GMT_MSG_FATAL, "Could not create %s\n", path_file);
		Return (EXIT_FAILURE);
	}
	fprintf (fp, "# Directories with data files for TAG %s\n", Ctrl->In.TAG);
	fprintf (fp, "# The current directory is always searched first.\n");
	fprintf (fp, "# Add full paths to search additional directories\n");
	x2sys_err_fail (GMT, x2sys_fclose (GMT, path_file, fp), path_file);
	
	GMT_report (GMT, GMT_MSG_NORMAL, "completed successfully\n");

	GMT_free (GMT, X2SYS_HOME);

	Return (GMT_OK);
}
