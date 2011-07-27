/*--------------------------------------------------------------------
 *	$Id$
 *
 *    Copyright (c) 2004-2011 by P. Wessel
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mgd77path accepts MGD77 cruise names and returns the full system
 * path to the file(s).
 *
 * Author:	Paul Wessel
 * Date:	26-AUG-2004
 * Version:	1.0 Based on the old gmtpath.c
 *
 *
 */
 
#include "gmt_mgd77.h"
#include "mgd77.h"

struct MGD77PATH_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct A {	/* -A */
		GMT_LONG active;
		GMT_LONG mode;
	} A;
	struct D {	/* -D */
		GMT_LONG active;
	} D;
	struct I {	/* -I */
		GMT_LONG active;
		GMT_LONG n;
		char code[3];
	} I;
};

void *New_mgd77path_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77PATH_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct MGD77PATH_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	return ((void *)C);
}

void Free_mgd77path_Ctrl (struct GMT_CTRL *GMT, struct MGD77PATH_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);	
}

GMT_LONG GMT_mgd77path_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT,"mgd77path %s [API] - Return paths to MGD77 cruises and directories\n\n", GMT_VERSION);
	GMT_message (GMT,"usage: mgd77path <cruise(s)> A[-] -D [-I<code>] [%s]\n\n", GMT_V_OPT);
        
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);
             
	MGD77_Cruise_Explain (GMT);
	GMT_message (GMT, "\t-A List full cruise pAths [Default].  Append - to only get cruise names.\n");
	GMT_message (GMT, "\t-D List all directories with MGD77 files instead.\n");
	GMT_message (GMT, "\tOPTIONS:\n\n");
	GMT_message (GMT, "\t-I Ignore certain data file formats from consideration. Append combination of act to ignore\n");
	GMT_message (GMT, "\t   (a) MGD77 ASCII, (c) MGD77+ netCDF, or (t) plain table files. [Default ignores none].\n");
	GMT_explain_options (GMT, "V.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_mgd77path_parse (struct GMTAPI_CTRL *C, struct MGD77PATH_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to mgd77path and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
			case '#':	/* Skip input files confused as numbers (e.g. 123456) */
				break;

			/* Processes program-specific parameters */

#ifdef GMT_COMPAT
			case 'P':
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -P is deprecated; use -A instead.\n");
#endif
			case 'A':	/* Show list of paths to MGD77 files */
				Ctrl->A.active = TRUE;
				if (opt->arg[0] == '-') Ctrl->A.mode = TRUE;
				break;
				
			case 'D':	/* Show list of directories with MGD77 files */
				Ctrl->D.active = TRUE;
				break;

			case 'I':
				Ctrl->I.active = TRUE;
				if (Ctrl->I.n < 3) {
					if (strchr ("act", (int)opt->arg[0]))
						Ctrl->I.code[Ctrl->I.n++] = opt->arg[0];
					else {
						GMT_report (GMT, GMT_MSG_FATAL, "Option -I Bad modifier (%c). Use -Ia|c|t!\n", opt->arg[0]);
						n_errors++;
					}
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Option -I: Can only be applied 0-2 times\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && Ctrl->D.active, "Syntax error: Only one of -A -D may be used\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_mgd77path_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); GMT_exit (code);}

GMT_LONG GMT_mgd77path (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG i, n_cruises = 0, n_paths, error = FALSE;
	
	char path[GMT_BUFSIZ], **list = NULL;
	
	struct MGD77_CONTROL M;
	struct MGD77PATH_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (options && options->option == '?') return (GMT_mgd77path_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options && options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_mgd77path_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_mgd77path", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-V", "", options))) Return ((int)error);
	Ctrl = (struct MGD77PATH_CTRL *) New_mgd77path_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_mgd77path_parse (API, Ctrl, options))) Return ((int)error);
	
	/*---------------------------- This is the mgd77path main code ----------------------------*/

	MGD77_Init (GMT, &M);			/* Initialize MGD77 Machinery */

	if (Ctrl->I.active) MGD77_Process_Ignore (GMT, 'I', Ctrl->I.code);

	if (Ctrl->D.active) {	/* Just list the current active MGD77 data directories and exit */
		printf ("# Currently, your $MGD77_HOME is set to: %s\n", M.MGD77_HOME);
		printf ("# $MGD77_HOME/mgd77_paths.txt contains these directories:\n");
		for (i = 0; i < M.n_MGD77_paths; i++) printf ("%s\n", M.MGD77_datadir[i]);
		Return (EXIT_SUCCESS);
	}

	n_paths = MGD77_Path_Expand (GMT, &M, options, &list);	/* Get list of requested IDs */

	if (n_paths == 0) {
		GMT_report (GMT, GMT_MSG_FATAL, "No cruises found\n");
		Return (EXIT_FAILURE);
	}
	
	for (i = 0; i < n_paths; i++) {		/* Process each ID */
 		if (MGD77_Get_Path (GMT, path, list[i], &M))
			GMT_report (GMT, GMT_MSG_FATAL, "Cannot find cruise %s\n", list[i]);
		else if (Ctrl->A.mode) {
			printf ("%s\n", list[i]);
			n_cruises++;
		}
		else {
			printf ("%s\n", path);
			n_cruises++;
		}
	}
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Found %ld cruises\n", n_cruises);
	
	MGD77_Path_Free (GMT, (int)n_paths, list);
	MGD77_end (GMT, &M);
	
	Return (GMT_OK);
}
