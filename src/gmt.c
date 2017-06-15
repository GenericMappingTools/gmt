/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * listed via GMT_CUSTOM_LIBS in gmt.conf.  Note: GMT_Call_Module checks
 * both <module> and gmt<module> in case the user left that part off.
 *
 * Version:	5
 * Created:	17-June-2013
 *
 */

#include "gmt_dev.h"

#if !(defined(WIN32) || defined(NO_SIGHANDLER))
#if	__APPLE__
	/* Apple Xcode expects _Nullable to be defined but it is not if gcc */
#ifndef _Nullable
#	define _Nullable
#	endif
#	endif
#	include <signal.h>
#	include "common_sighandler.h"
#endif

#define PROGRAM_NAME	"gmt"

/* Determine the system environmental parameter that leads to shared libraries */
#if defined _WIN32
#define LIB_PATH "%%PATH%%"
#elif defined __APPLE__
#define LIB_PATH "DYLD_LIBRARY_PATH"
#else
#define LIB_PATH "LD_LIBRARY_PATH"
#endif

int main (int argc, char *argv[]) {
	int k, status = GMT_NOT_A_VALID_MODULE;	/* Default status code */
	bool gmt_main = false;			/* Set to true if no module was specified */
	unsigned int modulename_arg_n = 0;	/* Argument index in argv[] that contains module name */
	unsigned int mode = GMT_SESSION_NORMAL;	/* Default API mode */
	unsigned int v_mode = GMT_MSG_COMPAT;		/* Default verbosity */
	struct GMTAPI_CTRL *api_ctrl = NULL;	/* GMT API control structure */
	char *progname = NULL;			/* Last component from the pathname */
	char *module = NULL;			/* Module name */

#if !(defined(WIN32) || defined(NO_SIGHANDLER))
	/* Install signal handler */
	struct sigaction act;
	sigemptyset(&act.sa_mask); /* Empty mask of signals to be blocked during execution of the signal handler */
	act.sa_sigaction = sig_handler;
#if 0 /* summit 2016 decision: disable CTRL-C interrupt feature */
	act.sa_flags = SA_SIGINFO | SA_NODEFER; /* Do not prevent the signal from being received from within its own signal handler. */
	sigaction (SIGINT,  &act, NULL);
#endif
	act.sa_flags = SA_SIGINFO;
	sigaction (SIGILL,  &act, NULL);
	sigaction (SIGFPE,  &act, NULL);
	sigaction (SIGBUS,  &act, NULL);
	sigaction (SIGSEGV, &act, NULL);
#endif /* !(defined(WIN32) || defined(NO_SIGHANDLER)) */

	/* Look for and process any -V[flag] so we may use GMT_Report_Error early on for debugging.
	 * Note: Because first 16 bits of mode may be used for other things we must left-shift by 16 */
	for (k = 1; k < argc; k++) if (!strncmp (argv[k], "-V", 2U)) v_mode = gmt_get_V (argv[k][2]);
	if (v_mode) mode = (v_mode << 16);	/* Left-shift the mode by 16 */
	/* Initialize new GMT session */
	if ((api_ctrl = GMT_Create_Session (argv[0], GMT_PAD_DEFAULT, mode, NULL)) == NULL)
		return GMT_RUNTIME_ERROR;
	api_ctrl->internal = true;	/* This is a proper GMT commandline session (external programs will default to false) */
	progname = strdup (basename (argv[0])); /* Last component from the pathname */
	/* Remove any filename extensions added for example by the MSYS shell when executing gmt via symlinks */
	gmt_chop_ext (progname);

	/* Test if argv[0] contains a valid module name: */
	module = progname;	/* Try this module name unless it equals PROGRAM_NAME in which case we just enter the test if argc > 1 */
	gmt_main = !strcmp (module, PROGRAM_NAME);	/* true if running the main program, false otherwise */
	if (gmt_main && (argc == 2 || argc == 3) && !strcmp (argv[1], "clear")) {	/* Clear something. */
		char *ptr = (argc == 3 && !strcmp (argv[2], "all")) ? argv[2] : NULL;	/* For all we pass NULL */
		if (GMT_Manage_Session (api_ctrl, GMT_SESSION_CLEAR, ptr))
			return GMT_RUNTIME_ERROR;
		return GMT_NOERROR;
	}
	if (gmt_main && argc == 2 && !strcmp (argv[1], "begin")) {	/* Initiating a GMT Work Flow. */
		if (GMT_Manage_Session (api_ctrl, GMT_SESSION_BEGIN, NULL))
			return GMT_RUNTIME_ERROR;
		if (GMT_Destroy_Session (api_ctrl))	/* Destroy GMT session */
			return GMT_RUNTIME_ERROR;
		return GMT_NOERROR;
	}
	else if (gmt_main && argc == 2 && !strcmp (argv[1], "end")) {	/* Terminating a GMT Work Flow. */
		if (GMT_Manage_Session (api_ctrl, GMT_SESSION_END, NULL))
			return GMT_RUNTIME_ERROR;
		if (GMT_Destroy_Session (api_ctrl))	/* Destroy GMT session */
			return GMT_RUNTIME_ERROR;
		return GMT_NOERROR;
	}
	else if (gmt_main && argc > 1 && !strcmp (argv[1], "figure")) {	/* Adding a figure entry to the queue. */
		char *cmd = gmt_argv2str (api_ctrl->GMT, argc-2, argv+2);	/* Consolidate all args into a string */
		if (GMT_Manage_Session (api_ctrl, GMT_SESSION_FIGURE, cmd))
			return GMT_RUNTIME_ERROR;
		gmt_M_str_free (cmd);
		if (GMT_Destroy_Session (api_ctrl))	/* Destroy GMT session */
			return GMT_RUNTIME_ERROR;
		return GMT_NOERROR;
	}
	if (gmt_main && argc > 1 && (!strcmp (argv[1], "gmtread") || !strcmp (argv[1], "read") || !strcmp (argv[1], "gmtwrite") || !strcmp (argv[1], "write"))) {
		/* Cannot call [gmt]read or [gmt]write module from the command-line - only external APIs can do that. */
		module = argv[1];	/* Name of module that does not exist, but will give reasonable message */
		modulename_arg_n = 1;
	}
	else if ((gmt_main || (status = GMT_Call_Module (api_ctrl, module, GMT_MODULE_EXIST, NULL)) == GMT_NOT_A_VALID_MODULE) && argc > 1) {
		/* argv[0] does not contain a valid module name, and
		 * argv[1] either holds the name of the module or an option: */
		modulename_arg_n = 1;
		module = argv[1];	/* Try this module name (Note: GMT_Call_Module will also check "gmt"<module> if <module> fails) */
		status = GMT_Call_Module (api_ctrl, module, GMT_MODULE_EXIST, NULL);
	}

	if (status == GMT_NOERROR) {
		/* Here we have found a recognized GMT module and the API has been initialized. */
		if (argv[1+modulename_arg_n] && !strcmp (argv[1+modulename_arg_n], "=") && argv[2+modulename_arg_n] == NULL) {
			/* Just wanted to know if module exists - do nothing here */
		}
		else {	/* Now run the specified GMT module: */
			if ((argc-1-modulename_arg_n) == 0)	/* No args, call explicitly with NULL because under Cygwin argv[2] may not be NULL */
				status = GMT_Call_Module (api_ctrl, module, 0, NULL);
			else
				status = GMT_Call_Module (api_ctrl, module, argc-1-modulename_arg_n, argv+1+modulename_arg_n);
		}
	}
	else {	/* status == GMT_NOT_A_VALID_MODULE */
		/* neither argv[0] nor argv[1] contain a valid module name */
		int arg_n;

		if (argv[1+modulename_arg_n] && !strcmp (argv[1+modulename_arg_n], "=") && argv[2+modulename_arg_n] == NULL)
			status = GMT_RUNTIME_ERROR; /* Just wanted to know if module exists */

		for (arg_n = 1; arg_n < argc && status == GMT_NOT_A_VALID_MODULE; ++arg_n) {
			/* Try all remaining arguments: */

			/* Print module list */
			if (!strcmp (argv[arg_n], "--help")) {
				fprintf (stderr, "\n\tGMT - The Generic Mapping Tools, Version %s [%u cores]\n", GMT_VERSION, api_ctrl->n_cores);
				fprintf (stderr, "(c) 1991-%d Paul Wessel, Walter H. F. Smith, Remko Scharroo, Joaquim Luis, and Florian Wobbe\n\n", GMT_VERSION_YEAR);
				fprintf (stderr, "Supported in part by the US National Science Foundation (http://www.nsf.gov/)\n");
				fprintf (stderr, "and volunteers from around the world (see http://gmt.soest.hawaii.edu/).\n\n");

				GMT_Call_Module (api_ctrl, NULL, GMT_MODULE_PURPOSE, NULL);
				status = GMT_NOERROR;
			}

			/* Print all modules and exit */
			else if (!strncmp (argv[arg_n], "--show-modules", 8U)) {
				GMT_Call_Module (api_ctrl, NULL, GMT_MODULE_LIST, NULL);
				status = GMT_NOERROR;
			}

			/* Print version and exit */
			else if (!strncmp (argv[arg_n], "--version", 5U)) {
				fprintf (stdout, "%s\n", GMT_PACKAGE_VERSION_WITH_SVN_REVISION);
				status = GMT_NOERROR;
			}

			/* Show number of cores */
			else if (!strncmp (argv[arg_n], "--show-cores", 11U)) {
				fprintf (stdout, "%u\n", api_ctrl->n_cores);
				status = GMT_NOERROR;
			}

			/* Show share directory */
			else if (!strncmp (argv[arg_n], "--show-datadir", 11U)) {
				if (api_ctrl->GMT->session.DATADIR == NULL)
					fprintf(stdout, "Not set\n");
				else
					fprintf(stdout, "%s\n", api_ctrl->GMT->session.DATADIR);
				status = GMT_NOERROR;
			}

			/* Show the directory that contains the 'gmt' executable */
			else if (!strncmp (argv[arg_n], "--show-bindir", 10U)) {
				fprintf (stdout, "%s\n", api_ctrl->GMT->init.runtime_bindir);
				status = GMT_NOERROR;
			}

			/* Show the directory that contains the shared plugins */
			else if (!strncmp (argv[arg_n], "--show-plugindir", 13U)) {
				fprintf (stdout, "%s\n", api_ctrl->GMT->init.runtime_plugindir);
				status = GMT_NOERROR;
			}

			/* Show the shared library */
			else if (!strncmp (argv[arg_n], "--show-library", 10U)) {
				fprintf (stdout, "%s\n", api_ctrl->GMT->init.runtime_library);
				status = GMT_NOERROR;
			}

			/* Show share directory */
			else if (!strncmp (argv[arg_n], "--show-sharedir", 12U)) {
				fprintf (stdout, "%s\n", api_ctrl->GMT->session.SHAREDIR);
				status = GMT_NOERROR;
			}

		} /* for (arg_n = 1; arg_n < argc; ++arg_n) */
	} /* status == GMT_NOERROR */

	if (status == GMT_NOT_A_VALID_MODULE) {
		/* If we get here, we were called without a recognized modulename or option
		 *
		 * gmt.c is itself not a module and hence can use fprintf (stderr, ...). Any API needing a
		 * gmt-like application will write one separately [see mex API and documentation] */

		fprintf (stderr, "\n\tGMT - The Generic Mapping Tools, Version %s [%u cores]\n", GMT_VERSION, api_ctrl->n_cores);
		fprintf (stderr, "(c) 1991-%d Paul Wessel, Walter H. F. Smith, Remko Scharroo, Joaquim Luis, and Florian Wobbe\n\n", GMT_VERSION_YEAR);
		fprintf (stderr, "Supported in part by the US National Science Foundation (http://www.nsf.gov/)\n");
		fprintf (stderr, "and volunteers from around the world (see http://gmt.soest.hawaii.edu/).\n\n");

		fprintf (stderr, "This program comes with NO WARRANTY, to the extent permitted by law.\n");
		fprintf (stderr, "You may redistribute copies of this program under the terms of the\n");
		fprintf (stderr, "GNU Lesser General Public License (http://www.gnu.org/licenses/lgpl.html).\n");
		fprintf (stderr, "For more information about legal matters, see the file named LICENSE.TXT.\n\n");
		fprintf (stderr, "usage: %s [options]\n", PROGRAM_NAME);
		fprintf (stderr, "       %s <module name> [<module-options>]\n\n", PROGRAM_NAME);
		fprintf (stderr, "Session management:\n");
		fprintf (stderr, "  gmt begin         Initiate a new GMT session using modern mode [classic].\n");
		fprintf (stderr, "  gmt end           Terminate the current GMT modern mode session.\n\n");
		fprintf (stderr, "  gmt figure        Set figure format specifics under a GMT modern mode session.\n\n");
		fprintf (stderr, "  gmt clear history | conf | cache | all\n");
		fprintf (stderr, "                    Deletes gmt.history, gmt.conf, the user cache dir, or all of them\n\n");
		fprintf (stderr, "options:\n");
		fprintf (stderr, "  --help            List descriptions of available GMT modules.\n");
		fprintf (stderr, "  --show-bindir     Show directory with GMT executables.\n");
		fprintf (stderr, "  --show-cores      Print number of available cores.\n");
		fprintf (stderr, "  --show-datadir    Show directory/ies with user data.\n");
		fprintf (stderr, "  --show-modules    List all module names.\n");
		fprintf (stderr, "  --show-library    Show path of the shared GMT library.\n");
		fprintf (stderr, "  --show-plugindir  Show directory for plug-ins.\n");
		fprintf (stderr, "  --show-sharedir   Show directory for shared GMT resources.\n");
		fprintf (stderr, "  --version         Print GMT version number.\n\n");
		fprintf (stderr, "if <module-options> is \'=\' we call exit (0) if module exist and non-zero otherwise.\n\n");
		if (modulename_arg_n == 1 && module[0] != '-') {
			fprintf (stderr, "ERROR: No module named %s was found.  This could mean one of three cases:\n", module);
			fprintf (stderr, "  1. There actually is no such module; please check your spelling.\n");
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
		status = GMT_RUNTIME_ERROR;
	} /* status == GMT_NOT_A_VALID_OPTION */

	gmt_M_str_free (progname); /* Was already dereferenced in gmt_chop_ext, so no NULL check needed */
	/* Destroy GMT session */
	if (GMT_Destroy_Session (api_ctrl))
		return GMT_RUNTIME_ERROR;

	return status; /* Return the status from the module */
}
