/*--------------------------------------------------------------------
 *
 *    Copyright (c) 2004-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#include "gmt_dev.h"
#include "mgd77.h"

#define THIS_MODULE_CLASSIC_NAME	"mgd77path"
#define THIS_MODULE_MODERN_NAME	"mgd77path"
#define THIS_MODULE_LIB		"mgd77"
#define THIS_MODULE_PURPOSE	"Return paths to MGD77 cruises and directories"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-V"

struct MGD77PATH_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -A */
		bool active;
		bool mode;
	} A;
	struct D {	/* -D */
		bool active;
	} D;
	struct I {	/* -I */
		bool active;
		unsigned int n;
		char code[3];
	} I;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77PATH_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct MGD77PATH_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct MGD77PATH_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <cruise(s)> A[c] -D [-I<code>] [%s] [%s]\n\n", name, GMT_V_OPT, GMT_PAR_OPT);
       
	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);
            
	MGD77_Cruise_Explain (API->GMT);
	GMT_Message (API, GMT_TIME_NONE, "\t-A List full cruise pAths [Default].  Append c to only get cruise names.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D List all directories with MGD77 files instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Ignore certain data file formats from consideration. Append combination of act to ignore\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (a) MGD77 ASCII, (c) MGD77+ netCDF, (m) MGD77T ASCII, or (t) plain table files. [Default ignores none].\n");
	GMT_Option (API, "V,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct MGD77PATH_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to mgd77path and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
			case '#':	/* Skip input files confused as numbers (e.g. 123456) */
				break;

			/* Processes program-specific parameters */

			case 'P':
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "-P is deprecated; use -A instead mext time.\n");
					Ctrl->A.active = true;
					/* Purposfully falling through to catch 'A' instead */
				}
				else {
					n_errors += gmt_default_error (GMT, opt->option);
					break;
				}
			case 'A':	/* Show list of paths to MGD77 files */
				Ctrl->A.active = true;
				if (opt->arg[0] == 'c' || opt->arg[0] == '-') Ctrl->A.mode = true;
				break;
			
			case 'D':	/* Show list of directories with MGD77 files */
				Ctrl->D.active = true;
				break;

			case 'I':
				Ctrl->I.active = true;
				if (Ctrl->I.n < 3) {
					if (strchr ("acmt", (int)opt->arg[0]))
						Ctrl->I.code[Ctrl->I.n++] = opt->arg[0];
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Option -I Bad modifier (%c). Use -Ia|c|m|t!\n", opt->arg[0]);
						n_errors++;
					}
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Option -I: Can only be applied 0-2 times\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && Ctrl->D.active, "Syntax error: Only one of -A -D may be used\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mgd77path (void *V_API, int mode, void *args) {
	uint64_t n_cruises = 0, i, n_paths;
	int error = 0;

	char path[PATH_MAX] = {""}, **list = NULL;

	struct MGD77_CONTROL M;
	struct MGD77PATH_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the mgd77path main code ----------------------------*/

	MGD77_Init (GMT, &M);			/* Initialize MGD77 Machinery */

	if (Ctrl->I.active) MGD77_Process_Ignore (GMT, 'I', Ctrl->I.code);

	if (Ctrl->D.active) {	/* Just list the current active MGD77 data directories and exit */
		printf ("# Currently, your $MGD77_HOME is set to: %s\n", M.MGD77_HOME);
		printf ("# $MGD77_HOME/mgd77_paths.txt contains these directories:\n");
		for (i = 0; i < M.n_MGD77_paths; i++) printf ("%s\n", M.MGD77_datadir[i]);
		Return (GMT_NOERROR);
	}

	error = MGD77_Path_Expand (GMT, &M, options, &list);	/* Get list of requested IDs */

	if (error <= 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "No cruises found\n");
		Return (GMT_NO_INPUT);
	}
	n_paths = (uint64_t)error;

	for (i = 0; i < n_paths; i++) {		/* Process each ID */
 		if (MGD77_Get_Path (GMT, path, list[i], &M))
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot find cruise %s\n", list[i]);
		else if (Ctrl->A.mode) {
			printf ("%s\n", list[i]);
			n_cruises++;
		}
		else {
			printf ("%s\n", path);
			n_cruises++;
		}
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Found %" PRIu64 " cruises\n", n_cruises);

	MGD77_Path_Free (GMT, n_paths, list);
	MGD77_end (GMT, &M);

	Return (GMT_NOERROR);
}
