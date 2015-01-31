/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Launcher for any GMT5 module via the corresponding function.
 * Modules are loaded dynamically from the GMT core library, the
 * optional supplemental library, and any number of custom libraries
 * listed via GMT_CUSTOM_LIBS in gmt.conf.
 *
 * Version:	5
 * Created:	17-June-2013
 *
 */

#include "gmt_dev.h"

#if !(defined(WIN32) || defined(NO_SIGHANDLER))
#	include <signal.h>
#	include "common_sighandler.h"
#endif

#define PROGRAM_NAME	"gmt"
#define GMT_PAD_DEFAULT	2U

/* Determine the system environmetal parameter that leads to shared libraries */
#if defined _WIN32
#define LIB_PATH "%%PATH%%"
#elif defined __APPLE__
#define LIB_PATH "DYLD_LIBRARY_PATH"
#else
#define LIB_PATH "LD_LIBRARY_PATH"
#endif

int main (int argc, char *argv[]) {
	int status = GMT_NOT_A_VALID_MODULE;	/* Default status code */
	int k, v_mode = GMT_MSG_COMPAT;		/* Default verbosity */
	bool gmt_main = false;			/* Set to true if no module specified */
	unsigned int modulename_arg_n = 0;	/* Argument number in argv[] that contains module name */
	unsigned int mode = 0;			/* Default API mode */
	struct GMTAPI_CTRL *api_ctrl = NULL;	/* GMT API control structure */
	char gmt_module[GMT_LEN32] = "gmt";
	char *progname = NULL;			/* Last component from the pathname */
	char *module = NULL;			/* Module name */

#if !(defined(WIN32) || defined(NO_SIGHANDLER))
	/* Install signal handler */
	struct sigaction act;
	sigemptyset(&act.sa_mask); /* Empty mask of signals to be blocked during execution of the signal handler */
	act.sa_flags = SA_SIGINFO | SA_NODEFER; /* Do not prevent the signal from being received from within its own signal handler. */
	act.sa_sigaction = sig_handler;
	sigaction (SIGINT,  &act, NULL);
	act.sa_flags = SA_SIGINFO;
	sigaction (SIGILL,  &act, NULL);
	sigaction (SIGFPE,  &act, NULL);
	sigaction (SIGBUS,  &act, NULL);
	sigaction (SIGSEGV, &act, NULL);
#endif /* !(defined(WIN32) || defined(NO_SIGHANDLER)) */

	/* Look for and process any -V[flag] so we may use GMT_Report_Error early on.
	 * Because first 2 bits of mode is used for other things we must left-shift by 2 */
	for (k = 1; k < argc; k++) if (!strncmp (argv[k], "-V", 2U)) v_mode = GMT_get_V (argv[k][2]);
	if (v_mode) mode = ((unsigned int)v_mode) << 2;	/* Left-shift the mode by 2 */
	/* Initialize new GMT session */
	if ((api_ctrl = GMT_Create_Session (argv[0], GMT_PAD_DEFAULT, mode, NULL)) == NULL)
		return EXIT_FAILURE;
	api_ctrl->internal = true;	/* This is a proper GMT internal session (external programs will default to false) */
	progname = strdup (basename (argv[0])); /* Last component from the pathname */
	/* Remove any filename extensions added for example
	 * by the MSYS shell when executing gmt via symlinks */
	GMT_chop_ext (progname);

	/* Test if argv[0] contains a module name: */
	module = progname;	/* Try this module name unless it equals PROGRAM_NAME in which case we just enter the test if argc > 1 */
	gmt_main = !strcmp (module, PROGRAM_NAME);	/* true if running the main program, false otherwise */
	if (gmt_main && argc > 1 && (!strcmp (argv[1], "read") || !strcmp (argv[1], "write"))) {	/* Cannot call read or write module from command-line gmt.c */
		module = argv[1];	/* Name of module that does not exist */
		status = GMT_NOT_A_VALID_MODULE;
		modulename_arg_n = 1;
		goto no_such;
	}

	if ((gmt_main || (status = GMT_Call_Module (api_ctrl, module, GMT_MODULE_EXIST, NULL)) == GMT_NOT_A_VALID_MODULE) && argc > 1) {
		/* argv[0] does not contain a valid module name, and
		 * argv[1] either holds the name of the module or an option: */
		modulename_arg_n = 1;
		module = argv[1];	/* Try this module name */
		if ((status = GMT_Call_Module (api_ctrl, module, GMT_MODULE_EXIST, NULL) == GMT_NOT_A_VALID_MODULE)) {
			/* argv[1] does not contain a valid module name; try prepending gmt: */
			strncat (gmt_module, argv[1], GMT_LEN32-4U);
			status = GMT_Call_Module (api_ctrl, gmt_module, GMT_MODULE_EXIST, NULL); /* either GMT_NOERROR or GMT_NOT_A_VALID_MODULE */
			if (status != GMT_NOT_A_VALID_MODULE) module = gmt_module;
		}
	}

	if (status == GMT_NOT_A_VALID_MODULE) {
		/* neither argv[0] nor argv[1] contain a valid module name */
		int arg_n;
		status = GMT_OK; /* default exit status */

		if (argv[1+modulename_arg_n] && !strcmp (argv[1+modulename_arg_n], "=") && argv[2+modulename_arg_n] == NULL) {
			/* Just wanted to know if module exists */
			status = 1; /* Return nonzero when modules does not exist */
			goto exit;
		}

		for (arg_n = 1; arg_n < argc; ++arg_n) {
			/* Try all remaining arguments: */

			/* Print module list */
			if (!strcmp (argv[arg_n], "--help")) {
				fprintf (stderr, "\n\tGMT - The Generic Mapping Tools, Version %s\n", GMT_VERSION);
				fprintf (stderr, "(c) 1991-%d Paul Wessel, Walter H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe\n\n", GMT_VERSION_YEAR);
				fprintf (stderr, "Supported in part by the US National Science Foundation (http://www.nsf.gov/)\n");
				fprintf (stderr, "and volunteers from around the world (see http://gmt.soest.hawaii.edu/).\n\n");

				GMT_Call_Module (api_ctrl, NULL, GMT_MODULE_PURPOSE, NULL);
				goto exit;
			}

			/* Print version and exit */
			if (!strcmp (argv[arg_n], "--version")) {
				fprintf (stdout, "%s\n", GMT_PACKAGE_VERSION_WITH_SVN_REVISION);
				goto exit;
			}

			/* Show number of cores */
			if (!strcmp (argv[arg_n], "--show-cores")) {
				fprintf (stdout, "%u\n", api_ctrl->n_cores);
				goto exit;
			}

			/* Show share directory */
			if (!strcmp (argv[arg_n], "--show-datadir")) {
				fprintf (stdout, "%s\n", api_ctrl->GMT->session.SHAREDIR);
				goto exit;
			}

			/* Show the directory that contains the 'gmt' executable */
			if (!strcmp (argv[arg_n], "--show-bindir")) {
				fprintf (stdout, "%s\n", api_ctrl->GMT->init.runtime_bindir);
				goto exit;
			}

			/* Show the directory that contains the shared plugins */
			if (!strcmp (argv[arg_n], "--show-plugindir")) {
				fprintf (stdout, "%s\n", api_ctrl->GMT->init.runtime_plugindir);
				goto exit;
			}
		} /* for (arg_n = 1; arg_n < argc; ++arg_n) */

		/* If we get here, we were called without a recognized modulename or option
		 *
		 * gmt.c is itself not a module and hence can use fprintf (stderr, ...). Any API needing a
		 * gmt-like application will write one separately [see mex API] */
no_such:

		fprintf (stderr, "\n\tGMT - The Generic Mapping Tools, Version %s [%u cores]\n", GMT_VERSION, api_ctrl->n_cores);
		fprintf (stderr, "(c) 1991-%d Paul Wessel, Walter H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe\n\n", GMT_VERSION_YEAR);
		fprintf (stderr, "Supported in part by the US National Science Foundation (http://www.nsf.gov/)\n");
		fprintf (stderr, "and volunteers from around the world (see http://gmt.soest.hawaii.edu/).\n\n");

		fprintf (stderr, "This program comes with NO WARRANTY, to the extent permitted by law.\n");
		fprintf (stderr, "You may redistribute copies of this program under the terms of the\n");
		fprintf (stderr, "GNU Lesser General Public License (http://www.gnu.org/licenses/lgpl.html).\n");
		fprintf (stderr, "For more information about these matters, see the file named LICENSE.TXT.\n\n");
		fprintf (stderr, "usage: %s [options]\n", PROGRAM_NAME);
		fprintf (stderr, "       %s <module name> [<module options>]\n\n", PROGRAM_NAME);
		fprintf (stderr, "options:\n");
		fprintf (stderr, "  --help            List and description of GMT modules.\n");
		fprintf (stderr, "  --version         Print version and exit.\n");
		fprintf (stderr, "  --show-cores      Show number of available cores and exit.\n");
		fprintf (stderr, "  --show-datadir    Show data directory and exit.\n");
		fprintf (stderr, "  --show-bindir     Show directory of executables and exit.\n");
		fprintf (stderr, "  --show-plugindir  Show directory of plug-ins and exit.\n\n");
		fprintf (stderr, "if <module options> is \'=\' we call exit (0) if module exist and non-zero otherwise.\n\n");
		if (modulename_arg_n == 1) {
			fprintf (stderr, "ERROR: No module named %s was found.  This could mean:\n", module);
			fprintf (stderr, "  1. There actually is no such module; check your spelling.\n");
			if (strlen (GMT_SUPPL_LIB_NAME))
				fprintf (stderr, "  2. Module exists in the GMT supplemental library, but the library could not be found.\n");
			else
				fprintf (stderr, "  2. Module exists in the GMT supplemental library, but the library was not installed.\n");
			if (api_ctrl->n_shared_libs > 2)
				fprintf (stderr, "  3. Module exists in a GMT custom library, but the library could not be found.\n");
			else
				fprintf (stderr, "  3. Module exists in a GMT custom library, but none was specified via GMT_CUSTOM_LIBS.\n");
			fprintf (stderr, "Shared libraries must be in standard system paths or set via environmental parameter %s.\n\n", LIB_PATH);
		}
		status = EXIT_FAILURE;
		goto exit;
	} /* status == GMT_NOT_A_VALID_MODULE */

	/* Here we have found a recognized GMT module and the API has been initialized. */
	if (argv[1+modulename_arg_n] && !strcmp (argv[1+modulename_arg_n], "=") && argv[2+modulename_arg_n] == NULL)	/* Just want to know if module exists */
		status = GMT_OK;
	else {	/* Now run the specified GMT module: */
		if ((argc-1-modulename_arg_n) == 0)	/* No args, call explicitly with NULL because under Cygwin argv[2] may not be NULL */
			status = GMT_Call_Module (api_ctrl, module, 0, NULL);
		else
			status = GMT_Call_Module (api_ctrl, module, argc-1-modulename_arg_n, argv+1+modulename_arg_n);
	}

exit:
	if (progname) free (progname);
	/* Destroy GMT session */
	if (GMT_Destroy_Session (api_ctrl))
		return EXIT_FAILURE;

	return status; /* Return the status from the module */
}
