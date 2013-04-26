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
	int status = EXIT_SUCCESS;           /* Status code from GMT API */
	unsigned int item = 1, k;
	char *module = NULL;
	enum GMT_MODULE_ID module_id = 0;    /* Module ID */
	struct GMTAPI_CTRL *api_ctrl = NULL; /* GMT API control structure */

	for (k = strlen (argv[0]); k > 0 && argv[0][k] != '/'; k--);	/* Find start of program name after any leading dirs */
	if (k) k++;	/* Unless there is no slash, advance one past the last slash we found */
	if (strncmp (&argv[0][k], "gmt", 3)) {	/* Does not match gmt so it is another module via symbolic link */
		item = 0;	/* Argv[0] holds the name of the module */
		module = &argv[0][k];
	}
	else if (argc > 1)
		module = argv[1];

	/* gmt.c is not a module and hence can use fprintf (stderr, ...). Any API needing a
	 * gmt-like application will write one separately [see mex API] */
	
	if (argc < 2 && item == 1) {
		fprintf (stderr, "gmt - The Generic Mapping Tools, Version %s\n", GMT_VERSION);
		fprintf (stderr, "Copyright 1991-%d Paul Wessel, Walter H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe\n\n", GMT_VERSION_YEAR);

		fprintf (stderr, "This program comes with NO WARRANTY, to the extent permitted by law.\n");
		fprintf (stderr, "You may redistribute copies of this program under the terms of the\n");
		fprintf (stderr, "GNU Lesser General Public License.\n");
		fprintf (stderr, "For more information about these matters, see the file named LICENSE.TXT.\n");
		fprintf (stderr, "For a brief description of GMT programs, type gmt --help\n\n");
		fprintf (stderr, "  --version            Print version and exit\n");
		fprintf (stderr, "  --show-sharedir      Show share directory and exit\n");
		fprintf (stderr, "  --show-bindir        Show directory of executables and exit\n");
		return EXIT_FAILURE;
	}

	/* Print version and exit */
	if (argc == 2 && !strcmp (argv[1], "--version")) {
		fprintf (stdout, "%s\n", GMT_PACKAGE_VERSION_WITH_SVN_REVISION);
		return EXIT_SUCCESS;
	}

	/* Show share directory */
	if (argc == 2 && !strcmp (argv[1], "--show-sharedir")) {
		/* Initializing new GMT session */
		if ((api_ctrl = GMT_Create_Session (argv[0], 2U, 0U, NULL)) == NULL)
			return EXIT_FAILURE;
		fprintf (stdout, "%s\n", api_ctrl->GMT->session.SHAREDIR);
		if (GMT_Destroy_Session (api_ctrl))
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}

	/* Show the directory that contains the 'gmt' executable */
	if (argc == 2 && !strcmp (argv[1], "--show-bindir")) {
		/* Initializing new GMT session */
		if ((api_ctrl = GMT_Create_Session (argv[0], 2U, 0U, NULL)) == NULL)
			return EXIT_FAILURE;
		fprintf (stdout, "%s\n", api_ctrl->GMT->init.runtime_bindir);
		if (GMT_Destroy_Session (api_ctrl))
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}

	/* Initializing new GMT session */
	if ((api_ctrl = GMT_Create_Session (argv[0], 2U, 0U, NULL)) == NULL)
		return EXIT_FAILURE;

	if (argc == 2 && !strcmp (argv[1], "--help")) {
		gmt_module_show_all(api_ctrl);
		if (GMT_Destroy_Session (api_ctrl))
			return EXIT_FAILURE;
		return EXIT_FAILURE;
	}

	if ((module_id = gmt_module_lookup (module)) == GMT_ID_NONE) {
		fprintf (stderr, "gmt: No such program: %s\n", module);
		if (GMT_Destroy_Session (api_ctrl))
			return EXIT_FAILURE;
		return EXIT_FAILURE;
	}

	/* OK, here we found a recognized GMT module and the API has been initialized; do the job */

	/* Run selected GMT cmd function, or give usage message if errors arise during parsing */
	status = g_module[module_id].p_func (api_ctrl, argc-1-item, argv+1+item);

	/* Destroy GMT session */
	if (GMT_Destroy_Session (api_ctrl))
		return EXIT_FAILURE;

	return status; /* Return the status from the module */
}
