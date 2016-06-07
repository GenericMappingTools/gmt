/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2016 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Author:	Paul Wessel
 * Date:	7-JUN-2016
 * Version:	5 API
 *
 * Brief synopsis: testapiconv tests GMT_Convert_Data function.
 *
 */

#include "gmt.h"

int main (int argc, char *argv[]) {
	void *API = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_TEXTSET *T = NULL;
	int ID;
	char string[GMT_STR16] = {""}, cmd[128] = {""};

	/*----------------------- Standard module initialization and parsing ----------------------*/

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session ("TEST", 2U, GMT_SESSION_NORMAL, NULL)) == NULL) exit (EXIT_FAILURE);

	/* Read in two data tables */

	if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_PLP, 0, NULL, NULL, NULL, 0, 0, NULL)) == NULL) exit (EXIT_FAILURE);	/* Create output container */
	if ((ID = GMT_Get_ID (API, GMT_IS_DATASET, GMT_OUT, D)) == GMT_NOTSET) exit (EXIT_FAILURE);	/* Get object ID */
	if (GMT_Encode_ID (API, string, ID) != GMT_NOERROR) exit (EXIT_FAILURE);	/* Make filename with embedded object ID */
	sprintf (cmd, "A.txt B.txt ->%s", string);
	if (GMT_Call_Module (API, "gmtconvert", GMT_MODULE_CMD, string)) exit (EXIT_FAILURE);	/* run module */
	if ((D = GMT_Retrieve_Data (API, ID)) == NULL) exit (EXIT_FAILURE);
	if ((T = GMT_Convert_Data (API, D, GMT_IS_DATASET, NULL, GMT_IS_TEXTSET, 0, 0))) exit (EXIT_FAILURE);	/* Convert to text */
	if (GMT_Write_Data (API, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, "AB_txt.txt", T) != GMT_NOERROR)  exit (EXIT_FAILURE);	/* run module */
	/* 8. Destroy GMT session */
	if (GMT_Destroy_Session (API)) exit (EXIT_FAILURE);
}
