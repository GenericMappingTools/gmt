/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-$year by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Demonstration program for how GMT5 may handle i/o and passing data
 * from one GMT_* module to another.
 *
 * Currently shows how "grdcut t.nc -R2/4/2/4 -Gnew.nc -V" is obtained
 * but calling GMT_grdcut from this main.
 *
 * Version:	5
 * Created:	13-Nov-2009
 *
 */

#include "gmt.h"

int main (int argc, char *argv[]) {

	GMT_LONG status = 0;				/* Status code from GMT API */
	struct GMT_OPTION *head = NULL, *new = NULL;	/* Linked list of options */
	struct GMTAPI_CTRL *API = NULL;			/* GMT API control structure */

	GMT_LONG in_grdcut_ID, out_grdcut_ID;
	char *in_grid = "t.nc", *out_grid = "new.nc", string[GMTAPI_STRLEN];
	double w = 2.0, e = 4.0, s = 1.0, n = 3.0;	/* Hardwired region for test */
	struct GMT_GRID *Gin = NULL, *Gout = NULL;

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session ("TEST", GMTAPI_GMT)) == NULL) exit (EXIT_FAILURE);

	/* 2. READING IN A GRID */
	if ((Gin = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, in_grid, NULL)) == NULL) exit (EXIT_FAILURE);

	/* 3. PREPARING SOURCE AND DESTINATION FOR GMT_grdcut */
	/* 3a. Register the Gin grid to be the source read by grdcut by passing a pointer */
	if ((in_grdcut_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_COPY, GMT_IS_SURFACE, GMT_IN, Gin, NULL)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);
	/* 3b. Register a grid struct Gout to be the destination allocated and written to by grdcut */
	if ((out_grdcut_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REF, GMT_IS_SURFACE, GMT_OUT, NULL, NULL)) == GMTAPI_NOTSET) exit (EXIT_FAILURE);

	/* 4. Create linked options for GMT_grdcut equivalent to "grdcut t.nc -R2/4/2/4 -Gnew.nc -V" */

	if (GMT_Encode_ID (API, string, in_grdcut_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	if ((new = GMT_Make_Option (API, '<', string)) == NULL) exit (EXIT_FAILURE);
	if ((head = GMT_Append_Option (API, new, NULL)) == NULL) exit (EXIT_FAILURE);
	sprintf (string, "%g/%g/%g/%g", w, e, s, n);		/* Create argument for -R option */
	if ((new = GMT_Make_Option (API, 'R', string)) == NULL) exit (EXIT_FAILURE);
	if ((head = GMT_Append_Option (API, new, head)) == NULL) exit (EXIT_FAILURE);
	if (GMT_Encode_ID (API, string, out_grdcut_ID) != GMT_OK) exit (EXIT_FAILURE);	/* Make -Gfilename with embedded object ID */
	if ((new = GMT_Make_Option (API, 'G', string)) == NULL) exit (EXIT_FAILURE);
	if ((head = GMT_Append_Option (API, new, head)) == NULL) exit (EXIT_FAILURE);
	if ((new = GMT_Make_Option (API, 'V', NULL)) == NULL) exit (EXIT_FAILURE);	/* Add -V*/
	if ((head = GMT_Append_Option (API, new, head)) == NULL) exit (EXIT_FAILURE);

	/* 5. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = GMT_grdcut (API, -1, head);	/* This allocates memory for the export grid associated with the -G option */
	if ((Gout = GMT_Retrieve_Data (API, out_grdcut_ID)) == NULL) exit (EXIT_FAILURE);

	/* 6. Destroy local linked option list */
	if (GMT_Destroy_Options (API, &head)) exit (EXIT_FAILURE);

	/* 7. WRITING THE RESULT TO FILE */
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, out_grid, Gout) != GMT_OK) exit (EXIT_FAILURE);

	/* 8. Destroy GMT session */
	if (GMT_Destroy_Session (&API)) exit (EXIT_FAILURE);

	exit (GMT_OK);		/* Return the status from this program */
}
