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
 * Command-line launcher for any GMT module via the corresponding function.
 * Modules are loaded dynamically from the GMT core library, the
 * optional supplemental library, and any number of custom libraries
 * listed via GMT_CUSTOM_LIBS in gmt.conf.  Note: GMT_Call_Module checks
 * both <module> and gmt<module> in case the user left that part off.
 *
 * Version:	6
 * Created:	17-June-2013
 *
 */

#include "gmt_dev.h"

#if !(defined(WIN32) || defined(NO_SIGHANDLER))
#ifdef	__APPLE__
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
	unsigned int v_mode = GMT_MSG_WARNING;	/* Default verbosity */
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

	progname = strdup (basename (argv[0])); /* Last component from the pathname */
	/* Remove any filename extensions added for example by the MSYS shell when executing gmt via symlinks */
	gmt_chop_ext (progname);

	/* Test if argv[0] contains a valid module name: */
	module = progname;	/* Try this module name unless it equals PROGRAM_NAME in which case we just enter the test if argc > 1 */
	gmt_main = !strcmp (module, PROGRAM_NAME);	/* true if running the main program, false otherwise */

	/* Initialize new GMT session */
	if ((api_ctrl = GMT_Create_Session (argv[0], GMT_PAD_DEFAULT, mode, NULL)) == NULL)
		return GMT_RUNTIME_ERROR;

	api_ctrl->internal = true;	/* This is a proper GMT commandline session (external programs will default to false) */
	if (gmt_main && argc > 1 && (!strcmp (argv[1], "gmtread") || !strcmp (argv[1], "read") || !strcmp (argv[1], "gmtwrite") || !strcmp (argv[1], "write"))) {
		/* Cannot call [gmt]read or [gmt]write module from the command-line - only external APIs can do that. */
		module = argv[1];	/* Name of module that does not exist, but will give reasonable message */
		modulename_arg_n = 1;
	}
	else if ((gmt_main || (status = GMT_Call_Module (api_ctrl, module, GMT_MODULE_EXIST, NULL)) == GMT_NOT_A_VALID_MODULE) && argc > 1) {
		/* argv[0] does not contain a valid module name, and
		 * argv[1] either holds the name of the module or an option: */
		modulename_arg_n = 1;
		module = argv[1];	/* Try this module name (Note: GMT_Call_Module will also check "gmt" <module> if just <module> fails) */
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
				fprintf (stderr, "\t(c) 1991-%d The GMT Team (https://www.generic-mapping-tools.org/team.html).\n\n", GMT_VERSION_YEAR);
				fprintf (stderr, "Supported in part by the US National Science Foundation (http://www.nsf.gov/)\n");
				fprintf (stderr, "and volunteers from around the world.\n\n");

				GMT_Call_Module (api_ctrl, NULL, GMT_MODULE_PURPOSE, NULL);
				status = GMT_NOERROR;
			}

			/* Print all modern modules and exit */
			else if (!strncmp (argv[arg_n], "--show-modules", 8U)) {
				GMT_Call_Module (api_ctrl, NULL, GMT_MODULE_LIST, NULL);
				status = GMT_NOERROR;
			}

			/* Print all classic modules and exit */
			else if (!strncmp (argv[arg_n], "--show-classic", 9U)) {
				GMT_Call_Module (api_ctrl, NULL, GMT_MODULE_CLASSIC, NULL);
				status = GMT_NOERROR;
			}

			/* Print version and exit */
			else if (!strncmp (argv[arg_n], "--version", 5U)) {
				fprintf (stdout, "%s\n", GMT_PACKAGE_VERSION_WITH_GIT_REVISION);
				status = GMT_NOERROR;
			}

			/* Show citation of the current release */
			else if (!strncmp (argv[arg_n], "--show-citation", 15U)) {
				fprintf(stdout, "%s\n", GMT_VERSION_CITATION);
				status = GMT_NOERROR;
			}

			/* Show number of cores */
			else if (!strncmp (argv[arg_n], "--show-cores", 11U)) {
				fprintf (stdout, "%u\n", api_ctrl->n_cores);
				status = GMT_NOERROR;
			}

			/* Show share directory */
			else if (!strncmp (argv[arg_n], "--show-datadir", 14U)) {
				if (api_ctrl->GMT->session.DATADIR == NULL)
					fprintf(stdout, "Not set\n");
				else
					fprintf(stdout, "%s\n", api_ctrl->GMT->session.DATADIR);
				status = GMT_NOERROR;
			}

			/* Show URL of the remote GMT data server */
			else if (!strncmp (argv[arg_n], "--show-dataserver", 17U)) {
				fprintf(stdout, "%s\n", api_ctrl->GMT->session.DATASERVER);
				status = GMT_NOERROR;
			}

			/* Show DOI of the current release */
			else if (!strncmp (argv[arg_n], "--show-doi", 10U)) {
				fprintf(stdout, "%s\n", GMT_VERSION_DOI);
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

			/* print new shell template */
			else if (!strncmp (argv[arg_n], "--new-script", 12U)) {
				unsigned int type = 0;	/* Default is bash */
				time_t right_now = time (NULL);
				char *s = NULL, *txt = NULL, *shell[3] = {"bash", "csh", "batch"}, stamp[GMT_LEN32] = {""};
				char *comment[3] = {"#", "#", "REM"};
				char *name = gmt_putusername (NULL);

				strftime (stamp, GMT_LEN32, "%FT%T", localtime (&right_now));
				if ((s = strchr (argv[arg_n], '=')) && s[1]) {	/* Gave a specific script language name */
					if ((strstr (&s[1], shell[0]) || strstr (&s[1], shell[1]) || strstr (&s[1], shell[2]) || strstr (&s[1], "dos")))
						txt = &s[1];
					else
						fprintf (stderr, "gmt: ERROR: --new-script language %s not recognized; default to bash\n\n", &s[1]);
				}
				else if ((txt = getenv ("shell")) == NULL) /* Likely not in a csh-type environment, try the Bourne shell environment variable SHELL */
					txt = getenv ("SHELL");	/* Here txt is either a shell path or NULL */
				if (txt && (!strcmp (txt, "batch") || !strcmp (txt, "dos"))) {	/* User asked for batch */
					type = 2;		/* Select batch */
					txt = shell[type];	/* Since user may have typed dos instead of batch */
					printf ("@echo off\n");	/* Turn of the default echo-ing of commands */
				}
#ifdef WIN32
				else if (txt == NULL) {	/* Assume batch if no shell setting exist under Windows */
					type = 2;		/* Select batch */
					printf ("@echo off\n");	/* Turn of the default echo-ing of commands */
				}
#endif
				if (type == 0 && txt && (strstr (txt, "csh") || strstr (txt, "tcsh")))	/* Got csh or tcsh */
					type = 1;	/* Select csh */
				if (type < 2) {	/* Start the shell via env and pass -e to exit script upon error */
					printf ("#!/usr/bin/env -S %s -e\n", shell[type]);
					printf ("%s GMT modern mode %s template\n", comment[type], shell[type]);
				}
				printf ("%s Date:    %s\n%s User:    %s\n%s Purpose: Purpose of this script\n", comment[type], stamp, comment[type], name, comment[type]);
				switch (type) {
					case 0: printf ("export GMT_SESSION_NAME=$$	# Set a unique session name\n"); break;
					case 1: printf ("setenv GMT_SESSION_NAME $$	# Set a unique session name\n"); break;
					case 2: printf ("REM Set a unique session name:\n");	/* Can't use $$ so output the PPID of this process */
						printf ("set GMT_SESSION_NAME=%s\n", api_ctrl->session_name);
						break;
				}
				printf ("gmt begin figurename\n\t%s Place modern session commands here\ngmt end show\n", comment[type]);
				gmt_M_str_free (name);
				status = GMT_NOERROR;
			}

		} /* for (arg_n = 1; arg_n < argc; ++arg_n) */
	} /* status == GMT_NOERROR */

	if (status == GMT_NOT_A_VALID_MODULE) {
		/* If we get here, we were called without a recognized modulename or option.
		 *
		 * gmt.c is itself not a module and hence can use fprintf (stderr, ...). Any API needing a
		 * gmt-like application will write one separately [see mex API and documentation] */

		if (modulename_arg_n == 1 && module[0] != '-') {
			fprintf (stderr, "\nERROR: No module named %s was found.  This could mean one of four cases:\n", module);
			fprintf (stderr, "  1. There actually is no such module; please check your spelling.\n");
			fprintf (stderr, "  2. You used a modern mode module name while running in GMT classic mode.\n");
			if (strlen (GMT_SUPPL_LIB_NAME))
				fprintf (stderr, "  3. Module exists in the GMT supplemental library, but the library could not be found.\n");
			else
				fprintf (stderr, "  3. Module exists in the GMT supplemental library, but the library was not installed.\n");
			if (api_ctrl->n_shared_libs > 2)
				fprintf (stderr, "  4. Module exists in a GMT custom library, but the library could not be found.\n");
			else
				fprintf (stderr, "  4. Module exists in a GMT custom library, but none was specified via GMT_CUSTOM_LIBS.\n");
			fprintf (stderr, "Shared libraries must be in standard system paths or set via environmental parameter %s.\n\n", LIB_PATH);
		}
		else {
			char libraries[GMT_LEN128] = {"netCDF"};	/* Always linked with netCDF */
#ifdef HAVE_GDAL
			strcat (libraries, ", GDAL");
#endif
#ifdef HAVE_PCRE
			strcat (libraries, ", PCRE");
#endif
#ifdef HAVE_FFTW3F
			strcat (libraries, ", FFTW");
#endif
#ifdef HAVE_LAPACK
			strcat (libraries, ", LAPACK");
#endif
#ifdef HAVE_ZLIB
			strcat (libraries, ", ZLIB");
#endif
			fprintf (stderr, "\n\tGMT - The Generic Mapping Tools, Version %s [%u cores]\n", GMT_VERSION, api_ctrl->n_cores);
			fprintf (stderr, "\t(c) 1991-%d The GMT Team (https://www.generic-mapping-tools.org/team.html).\n\n", GMT_VERSION_YEAR);
			fprintf (stderr, "\tSupported in part by the US National Science Foundation (http://www.nsf.gov/)\n");
			fprintf (stderr, "\tand volunteers from around the world.\n\n");

			fprintf (stderr, "\tGMT is distributed under the GNU LGPL License (http://www.gnu.org/licenses/lgpl.html).\n");
			fprintf (stderr, "\tDependencies: %s, Ghostscript, GraphicsMagick, FFmpeg.\n\n", libraries);
			fprintf (stderr, "usage: %s [options]\n", PROGRAM_NAME);
			fprintf (stderr, "       %s <module name> [<module-options>]\n\n", PROGRAM_NAME);
			fprintf (stderr, "options:\n");
			fprintf (stderr, "  --help            List descriptions of available GMT modules.\n");
			fprintf (stderr, "  --new-script[=L]  Write GMT modern mode script template to stdout.\n");
			fprintf (stderr, "                    Optionally specify bash|csh|batch [Default is current shell].\n");
			fprintf (stderr, "  --show-bindir     Show directory with GMT executables.\n");
			fprintf (stderr, "  --show-citation   Show the most recent citation for GMT.\n");
			fprintf (stderr, "  --show-classic    Show all classic module names.\n");
			fprintf (stderr, "  --show-cores      Show number of available cores.\n");
			fprintf (stderr, "  --show-datadir    Show directory/ies with user data.\n");
			fprintf (stderr, "  --show-dataserver Show URL of the remote GMT data server.\n");
			fprintf (stderr, "  --show-doi        Show the DOI for the current release.\n");
			fprintf (stderr, "  --show-modules    Show all modern module names.\n");
			fprintf (stderr, "  --show-library    Show path of the shared GMT library.\n");
			fprintf (stderr, "  --show-plugindir  Show directory for plug-ins.\n");
			fprintf (stderr, "  --show-sharedir   Show directory for shared GMT resources.\n");
			fprintf (stderr, "  --version         Print GMT version number.\n\n");
			fprintf (stderr, "if <module-options> is \'=\' we call exit (0) if module exist and non-zero otherwise.\n\n");
		}
		status = GMT_RUNTIME_ERROR;
	} /* status == GMT_NOT_A_VALID_OPTION */

	gmt_M_str_free (progname); /* Was already dereferenced in gmt_chop_ext, so no NULL check needed */
	/* Destroy GMT session */
	if (GMT_Destroy_Session (api_ctrl))
		return GMT_RUNTIME_ERROR;

	return status; /* Return the status from the module */
}
