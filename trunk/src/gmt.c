/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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

#include "gmt.h"

int main (int argc, char *argv[]) {
	int status = EXIT_SUCCESS;           /* Status code from GMT API */
	enum Gmt_module module_id = 0;       /* Module ID */
	struct Gmt_moduleinfo module;        /* Name, purpose, Api_mode, and function pointer of module */
	struct GMTAPI_CTRL *api_ctrl = NULL; /* GMT API control structure */

	if (argc < 2) {
		fprintf (stderr, "gmt - The Generic Mapping Tools, Version %s\n", GMT_VERSION);
		fprintf (stderr, "Copyright 1991-%d Paul Wessel, Walter H. F. Smith, R. Scharroo, and J. Luis\n\n", GMT_VERSION_YEAR);

		fprintf (stderr, "This program comes with NO WARRANTY, to the extent permitted by law.\n");
		fprintf (stderr, "You may redistribute copies of this program under the terms of the\n");
		fprintf (stderr, "GNU Lesser General Public License.\n");
		fprintf (stderr, "For more information about these matters, see the file named LICENSE.TXT.\n");
		fprintf (stderr, "For a brief description of GMT programs, type gmt --help\n\n");
		fprintf (stderr, "  --version            Print version and exit\n");
		fprintf (stderr, "  --show-sharedir      Show share directory and exit\n");
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
		if ((api_ctrl = GMT_Create_Session (argv[0], k_mode_gmt)) == NULL)
			return EXIT_FAILURE;
		fprintf (stdout, "%s\n", api_ctrl->GMT->session.SHAREDIR);
		if (GMT_Destroy_Session (&api_ctrl))
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
	}

	if (argc == 2 && !strcmp (argv[1], "--help")) {
		fprintf (stderr, "Program - Purpose of Program\n\n");
		gmt_module_show_all();
		return EXIT_FAILURE;
	}

	if ((module_id = gmt_module_lookup (argv[1])) == k_mod_notfound) {
		fprintf (stderr, "gmt: No such program: %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	/* OK, here we found a recognized GMT module; do the job */

	/* 1. Initializing new GMT session */
	if ((api_ctrl = GMT_Create_Session (argv[0], g_module[module_id].api_mode)) == NULL)
		return EXIT_FAILURE;

	/* Copy Gmt_moduleinfo to api_ctrl->GMT */
	api_ctrl->GMT->init.module = g_module[module_id];

	/* 2. Run selected GMT cmd function, or give usage message if errors arise during parsing */
	status = g_module[module_id].p_func (api_ctrl, argc-2, argv+2);

	/* 3. Destroy GMT session */
	if (GMT_Destroy_Session (&api_ctrl))
		return EXIT_FAILURE;

	return status; /* Return the status from FUNC */
}
