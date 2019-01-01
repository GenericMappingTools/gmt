/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Simulates a shell script but with a single API session.  Non-gmt calls
 * are run via system while GMT calls are run via GMT_Call_Module.
 *
 * Version:	6
 * Created:	16-Aug-2018
 *
 */

#include "gmt.h"
#include <string.h>

int main () {

	int status = 0;			/* Status code from GMT API */
	char line[BUFSIZ] = {""};	/* Input line buffer */
	char first[64] = {""}, module[32] = {""}, args[1024] = {""};
	struct GMTAPI_CTRL *API = NULL;	/* GMT API control structure */

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session ("TEST", GMT_PAD_DEFAULT, GMT_SESSION_NORMAL, NULL)) == NULL) exit (EXIT_FAILURE);

	/* 2. Loop over all commands and run them as is */
	
	while (fgets (line, BUFSIZ, stdin)) {
		if (line[0] == '#' || line[0] == '\n') continue;	/* Skip comments and blank lines */
		sscanf (line, "%s %s %[^\n]", first, module, args);
		if (strncmp (first, "gmt", 3U)) {	/* 3a. Make a system call since not GMT module */
			status = system (line);
			if (status) {
				GMT_Report (API, GMT_MSG_NORMAL, "system call returned error %d\n", status);
				exit (EXIT_FAILURE);
			}
		}
		else {	/* 3b. Call the module */
			status = GMT_Call_Module (API, module, GMT_MODULE_CMD, args);	/* This allocates memory for the export grid associated with the -G option */
			if (status) {
				GMT_Report (API, GMT_MSG_NORMAL, "%s returned error %d\n", module, status);
				exit (EXIT_FAILURE);
			}
		}
	}

	/* 4. Destroy GMT session */
	if (GMT_Destroy_Session (API)) exit (EXIT_FAILURE);

	exit (GMT_NOERROR);		/* Return the status from this program */
}
