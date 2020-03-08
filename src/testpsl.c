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
 * Demonstration program for how GMT may receive PS output directly
 * from GMT modules in memory, then write its own output file
 *
 * Version:	5
 * Created:	11-Nov-2015
 *
 */

#include "gmt.h"

int main () {

	int status = 0;				/* Status code from GMT API */
	char cmd[BUFSIZ] = {""};			/* Command string */
	char string[GMT_VF_LEN] = {""};		/* Encoded ID */
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
	struct GMT_POSTSCRIPT *PS = NULL;	/* Holds our plot */

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session ("PSLTEST", GMT_NOTSET, GMT_SESSION_NORMAL, NULL)) == NULL) exit (EXIT_FAILURE);

	/* 2. Create a virtual file to be the destination allocated and written to by pscoast */
	if (GMT_Open_VirtualFile (API, GMT_IS_POSTSCRIPT, GMT_IS_NONE, GMT_OUT, NULL, string) != GMT_NOERROR) exit (EXIT_FAILURE);
	sprintf (cmd, "-R0/20/0/20 -JM6i -P -Gred -K > %s", string);			/* Create command for pscoast */

	/* 3. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = GMT_Call_Module (API, "pscoast", GMT_MODULE_CMD, cmd);	/* This allocates memory for the plot and returns it via ID */
	if (status) {
		GMT_Report (API, GMT_MSG_ERROR, "GMT_pscoast returned error %d\n", status);
		exit (EXIT_FAILURE);
	}

	sprintf (cmd, "-R -J -O -Baf >> %s", string);	/* Create command for psbasemap overlay */
	/* 4. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = GMT_Call_Module (API, "psbasemap", GMT_MODULE_CMD, cmd);	/* This appends to the same plot */
	if (status) {
		GMT_Report (API, GMT_MSG_ERROR, "GMT_psbasemap returned error %d\n", status);
		exit (EXIT_FAILURE);
	}

	/* 5a. Get the plot object into our hands */
	if ((PS = GMT_Read_VirtualFile (API, string)) == NULL) exit (EXIT_FAILURE);
	/* 5b. Close the virtual file */
	if (GMT_Close_VirtualFile (API, string) != GMT_NOERROR) exit (EXIT_FAILURE);

	/* 6. Write the plot to file */
	if (GMT_Write_Data (API, GMT_IS_POSTSCRIPT, GMT_IS_FILE, GMT_IS_NONE, 0, NULL, "newmap.ps", PS) != GMT_NOERROR) exit (EXIT_FAILURE);

	/* 7. Destroy GMT session */
	if (GMT_Destroy_Session (API)) exit (EXIT_FAILURE);

	exit (GMT_NOERROR);	/* Return the status from this program */
}
