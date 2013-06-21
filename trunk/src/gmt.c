/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Launcher for any GMT5 program via the corresponding function.
 *
 * Version:	5
 * Created:	17-Feb-2010
 *
 */

#include "gmt_dev.h"

int main (int argc, char *argv[]) {
	int status = GMT_NOT_A_VALID_MODULE;     /* Default status code */
	unsigned int modulename_arg_n = 0;      /* argument number that contains module name */
	struct GMTAPI_CTRL *api_ctrl = NULL;    /* GMT API control structure */
	char gmt_module[GMT_TEXT_LEN16] = "gmt";
	char *progname = NULL; /* Last component from the pathname */
	char *module = NULL;			/* Module name */

	/* Initialize new GMT session */
	if ((api_ctrl = GMT_Create_Session (NULL, 2U, 0U, NULL)) == NULL)
		return EXIT_FAILURE;

	progname = strdup (GMT_basename(argv[0])); /* Last component from the pathname */
	/* remove any filename extensions added for example
	 * by the MSYS shell when executing gmt via symlinks */
	GMT_chop_ext (progname);

	/* test if argv[0] contains module name: */
	module = progname;	/* Try this module name */
	if (argc > 1 && (status = GMT_Probe_Module (api_ctrl, module, GMT_MODULE_EXIST) == GMT_NOT_A_VALID_MODULE)) {
		/* argv[0] does not contain a valid module name
		 * argv[1] either holds the name of the module or an option: */
		modulename_arg_n = 1;
		module = argv[1];	/* Try this module name */
		if ((status = GMT_Probe_Module (api_ctrl, module, GMT_MODULE_EXIST) == GMT_NOT_A_VALID_MODULE)) {
			/* argv[1] does not contain a valid module name; try prepending gmt: */
			strncat (gmt_module, argv[1], GMT_TEXT_LEN16-4U);
			module = gmt_module;	/* Try this module name */
			status = GMT_Probe_Module (api_ctrl, module, GMT_MODULE_EXIST); /* either GMT_NOERROR or GMT_NOT_A_VALID_MODULE */
		}
	}
	
	if (status == GMT_NOT_A_VALID_MODULE) {
		/* neither argv[0] nor argv[1] contain a valid module name */

		int arg_n;
		for (arg_n = 1; arg_n < argc; ++arg_n) {
			/* Try all remaining arguments: */

			/* Print module list */
			if (!strcmp (argv[arg_n], "--help")) {
				gmt_coremodule_show_all(api_ctrl);
				gmt_supplmodule_show_all(api_ctrl);
				goto exit;
			}

			/* Print version and exit */
			if (!strcmp (argv[arg_n], "--version")) {
				fprintf (stdout, "%s\n", GMT_PACKAGE_VERSION_WITH_SVN_REVISION);
				goto exit;
			}

			/* Show share directory */
			if (!strcmp (argv[arg_n], "--show-sharedir")) {
				fprintf (stdout, "%s\n", api_ctrl->GMT->session.SHAREDIR);
				goto exit;
			}

			/* Show the directory that contains the 'gmt' executable */
			if (!strcmp (argv[arg_n], "--show-bindir")) {
				fprintf (stdout, "%s\n", api_ctrl->GMT->init.runtime_bindir);
				goto exit;
			}
		} /* for (arg_n = 1; arg_n < argc; ++arg_n) */

		/* If we get here, we were called without a recognized modulename or option
		 *
		 * gmt.c is not a module and hence can use fprintf (stderr, ...). Any API needing a
		 * gmt-like application will write one separately [see mex API] */
		fprintf (stderr, "GMT - The Generic Mapping Tools, Version %s\n", GMT_VERSION);
		fprintf (stderr, "Copyright 1991-%d Paul Wessel, Walter H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe\n\n", GMT_VERSION_YEAR);

		fprintf (stderr, "This program comes with NO WARRANTY, to the extent permitted by law.\n");
		fprintf (stderr, "You may redistribute copies of this program under the terms of the\n");
		fprintf (stderr, "GNU Lesser General Public License.\n");
		fprintf (stderr, "For more information about these matters, see the file named LICENSE.TXT.\n\n");
		fprintf (stderr, "usage: %s [options] <module name> [module options]\n\n", progname);
		fprintf (stderr, "  --help               List and description of GMT modules\n");
		fprintf (stderr, "  --version            Print version and exit\n");
		fprintf (stderr, "  --show-sharedir      Show share directory and exit\n");
		fprintf (stderr, "  --show-bindir        Show directory of executables and exit\n");
		status = EXIT_FAILURE;
		goto exit;
	} /* status == GMT_NOT_A_VALID_MODULE */

	/* OK, here we found a recognized GMT module and the API has been initialized
	 * Now run selected GMT module: */
	status = GMT_Call_Module (api_ctrl, module, argc-1-modulename_arg_n, argv+1+modulename_arg_n);
	

exit:
	if (progname) free (progname);
	/* Destroy GMT session */
	if (GMT_Destroy_Session (api_ctrl))
		return EXIT_FAILURE;

	return status; /* Return the status from the module */
}
