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
 * The template for all GMT5 programs that are built from their modules.
 * The Makefile will replace FUNC with the module name and FUNC_MODE with
 * either GMTAPI_GMT or GMTAPI_GMTPSL (when PSL needs to be initialized).
 *
 * Version:	5
 * Created:	14-Sep-2011
 *
 */

#include "pslib.h"
#include "gmt.h"

EXTERN_MSC GMT_LONG FUNC (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args);

int main (int argc, char *argv[]) {

	int status = 0;				/* Status code from GMT API */
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session (argv[0], FUNC_MODE)) == NULL) exit (EXIT_FAILURE);

	/* 2. Run GMT function, or give usage message if errors arise during parsing */
	status = (int)FUNC (API, argc-1, argv+1);

	/* 3. Destroy GMT session */
	if (GMT_Destroy_Session (&API) != GMT_OK) exit (EXIT_FAILURE);

	exit (status);		/* Return the status from FUNC */
}
