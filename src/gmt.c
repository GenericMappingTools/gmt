/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
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

#include "pslib.h"
#include "gmt_modules.h"

struct GMT_PROGRAMS {		/* Struct with name and mode for each GMT 4 program */
	char name[GMT_TEXT_LEN64];	/* Program name */
	GMT_LONG mode;			/* Either GMTAPI_GMT or GMTAPI_GMTPSL */
};

PFL lookup_program (char *prog, struct GMT_PROGRAMS *programs, GMT_LONG n_progs, GMT_LONG *mode)
{
	GMT_LONG k = 0, id = -1;
	PFL func = NULL;
	
	for (k = 0; id == -1 && k < n_progs; k++) if (!strcmp (prog, programs[k].name)) id = k;	/* Get program id */
	
	if (id == -1) return NULL;	/* Not a GMT program */
	
	switch (id) {	/* Assign the function pointer */
#include "gmt_progcases.h"
	}
	return (func);
}

int main (int argc, char *argv[]) {

	GMT_LONG status = 0;			/* Status code from GMT API */
	GMT_LONG mode = 0;			/* Mode of the selected function */
	PFL func = NULL;			/* Pointer to the selected function */
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
	struct GMT_PROGRAMS program[GMT_N_PROGRAMS] = {	/* Sorted array with program information */
#include "gmt_prognames.h"
	};

	if (argc < 2) {
		fprintf (stderr, "gmt - The Generic Mapping Tools, Version %s\n", GMT_VERSION);
		fprintf (stderr, "Copyright 1991-%d Paul Wessel, Walter H. F. Smith, R. Scharroo, and J. Luis\n\n", GMT_VERSION_YEAR);

		fprintf (stderr, "This program comes with NO WARRANTY, to the extent permitted by law.\n");
		fprintf (stderr, "You may redistribute copies of this program under the terms of the\n");
		fprintf (stderr, "GNU General Public License.\n");
		fprintf (stderr, "For more information about these matters, see the file named LICENSE.TXT.\n");
		fprintf (stderr, "For a brief description of GMT programs, type gmt --help\n");
		exit (EXIT_FAILURE);
	}
	
	if (argc == 2 && !strcmp (argv[1], "--help")) {
		fprintf (stderr, "Program - Purpose of Program\n\n");
#include "gmt_progpurpose.h"
		exit (EXIT_FAILURE);
	}
	
	if ((func = lookup_program (argv[1], program, GMT_N_PROGRAMS, &mode)) == NULL) {
		fprintf (stderr, "gmt: No such program: %s\n", argv[1]);
		exit (EXIT_FAILURE);
	}
	
	/* OK, here we found a recognized GMT module; do the job */
	
	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session (argv[0], mode)) == NULL) exit (EXIT_FAILURE);

	/* 2. Run selected GMT cmd function, or give usage message if errors arise during parsing */
	status = func (API, argc-2, argv+2);

	/* 3. Destroy GMT session */
	if (GMT_Destroy_Session (&API)) exit (EXIT_FAILURE);

	exit (status);		/* Return the status from FUNC */
}
