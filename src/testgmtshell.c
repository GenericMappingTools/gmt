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
 * Simulates a shell script but with a single API session.  Non-gmt calls
 * are run via system while GMT calls are run via GMT_Call_Module.
 *
 * Version:	6
 * Created:	16-Aug-2018
 *
 */

#include "gmt.h"
#include <string.h>

int main (int argc, char *argv[]) {

	int status = 0;			/* Status code from GMT API */
	int k, debug = 0;
	char line[BUFSIZ] = {""};	/* Input line buffer */
	char first[128] = {""}, module[32] = {""}, args[1024] = {""};
	FILE *fp = NULL;
	struct GMTAPI_CTRL *API = NULL;	/* GMT API control structure */

	for (k = 1; k < argc; k++) {
		if (strstr (argv[k], "-v")) debug = 1;
		if (strstr (argv[k], "-f")) fp = fopen (argv[k+1], "r");
	}

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session ("TEST", GMT_PAD_DEFAULT, GMT_SESSION_NORMAL, NULL)) == NULL) exit (EXIT_FAILURE);

	/* 2. Loop over all commands and run them as is */
	
	if (fp == NULL) fp = stdin;
	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '#' || line[0] == '\n') continue;	/* Skip comments and blank lines */
		k = sscanf (line, "%s %s %[^\n]", first, module, args);
		if (debug) {	/* Echo back the command in brackets */
			line[strlen(line)-1] = '\0';	/* Chop off newline */
			fprintf (stderr, "cmd = [%s]\n", line);
		}
		if (strncmp (first, "gmt", 3U)) {	/* 3a. Make a system call since not GMT module */
			status = system (line);
			if (status) {
				GMT_Report (API, GMT_MSG_NORMAL, "system call returned error %d\n", status);
				exit (EXIT_FAILURE);
			}
		}
		else {	/* 3b. Call the module */
			if (k == 2)	/* No arguments to module */
				status = GMT_Call_Module (API, module, 0, NULL);
			else
				status = GMT_Call_Module (API, module, GMT_MODULE_CMD, args);
			if (status) {
				GMT_Report (API, GMT_MSG_NORMAL, "%s returned error %d\n", module, status);
				exit (EXIT_FAILURE);
			}
		}
	}
	if (fp != stdin) fclose (fp);

	/* 4. Destroy GMT session */
	if (GMT_Destroy_Session (API)) exit (EXIT_FAILURE);

	exit (GMT_NOERROR);		/* Return the status from this program */
}
