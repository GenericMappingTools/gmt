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
 * Demonstration program for how GMT may handle i/o and passing data
 * from one GMT_* module to another.
 *
 * Currently shows how "grdcut t.nc -R2/4/2/4 -Gnew.nc -V" is obtained
 * but calling GMT_grdcut from this main.
 *
 * Version:	5
 * Created:	13-Nov-2009
 *
 */

#include "gmt_dev.h"

int main () {

	int status = 0;				/* Status code from GMT API */
	struct GMT_OPTION *head = NULL, *new_opt = NULL;	/* Linked list of options */
	struct GMTAPI_CTRL *API = NULL;			/* GMT API control structure */

	int in_grdcut_ID, out_grdcut_ID;
	char *in_grid = "t.nc", *out_grid = "new.nc", string[GMT_STR16] = {""}, arg[GMT_LEN256] = {""};
	double w = 2.0, e = 4.0, s = 1.0, n = 3.0;	/* Hardwired region for test */
	struct GMT_GRID *Gin = NULL, *Gout = NULL;

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session ("TEST", GMT_PAD_DEFAULT, GMT_SESSION_NORMAL, NULL)) == NULL) exit (EXIT_FAILURE);

	/* 2. READING IN A GRID */
	if ((Gin = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, in_grid, NULL)) == NULL) exit (EXIT_FAILURE);

	/* 3. PREPARING SOURCE AND DESTINATION FOR GMT_grdcut */
	/* 3a. Register the Gin grid to be the source read by grdcut by passing a pointer */
	if ((in_grdcut_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_DUPLICATE, GMT_IS_SURFACE, GMT_IN, NULL, Gin)) == GMT_NOTSET) exit (EXIT_FAILURE);
	/* 3b. Register a grid struct Gout to be the destination allocated and written to by grdcut */
	if ((out_grdcut_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REFERENCE, GMT_IS_SURFACE, GMT_OUT, NULL, NULL)) == GMT_NOTSET) exit (EXIT_FAILURE);

	/* 4. Create linked options for GMT_grdcut equivalent to "grdcut t.nc -R2/4/2/4 -Gnew.nc -V" */

	if (GMT_Encode_ID (API, string, in_grdcut_ID) != GMT_NOERROR) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if ((new_opt = GMT_Make_Option (API, '<', string)) == NULL) exit (EXIT_FAILURE);
	if ((head = GMT_Append_Option (API, new_opt, NULL)) == NULL) exit (EXIT_FAILURE);
	sprintf (arg, "%g/%g/%g/%g", w, e, s, n);		/* Create argument for -R option */
	if ((new_opt = GMT_Make_Option (API, 'R', arg)) == NULL) exit (EXIT_FAILURE);
	if ((head = GMT_Append_Option (API, new_opt, head)) == NULL) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, string, out_grdcut_ID) != GMT_NOERROR) exit (EXIT_FAILURE);	/* Make -Gfilename with embedded object ID */
	if ((new_opt = GMT_Make_Option (API, 'G', string)) == NULL) exit (EXIT_FAILURE);
	if ((head = GMT_Append_Option (API, new_opt, head)) == NULL) exit (EXIT_FAILURE);
	if ((new_opt = GMT_Make_Option (API, 'V', NULL)) == NULL) exit (EXIT_FAILURE);	/* Add -V*/
	if ((head = GMT_Append_Option (API, new_opt, head)) == NULL) exit (EXIT_FAILURE);

	/* 5. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = GMT_Call_Module (API, "grdcut", GMT_MODULE_OPT, head);	/* This allocates memory for the export grid associated with the -G option */
	if (status) {
		GMT_Report (API, GMT_MSG_ERROR, "GMT_grdcut returned error %d\n", status);
		exit (EXIT_FAILURE);
	}
	if ((Gout = GMT_Retrieve_Data (API, out_grdcut_ID)) == NULL) exit (EXIT_FAILURE);

	/* 6. Destroy local linked option list */
	if (GMT_Destroy_Options (API, &head)) exit (EXIT_FAILURE);

	/* 7. WRITING THE RESULT TO FILE */
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, out_grid, Gout) != GMT_NOERROR) exit (EXIT_FAILURE);

	/* 8. Destroy GMT session */
	if (GMT_Destroy_Session (API)) exit (EXIT_FAILURE);

	exit (GMT_NOERROR);		/* Return the status from this program */
}
