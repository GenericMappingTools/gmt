/*--------------------------------------------------------------------
 *	$Id: gmtprogram.c,v 1.4 2011-07-13 02:56:13 guru Exp $
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
 * Created:	20-Oct-2009
 *
 */

#include "pslib.h"
#include "gmt.h"

EXTERN_MSC GMT_LONG FUNC (struct GMTAPI_CTRL *API, struct GMT_OPTION *options);

int main (int argc, char *argv[]) {

	int status = 0;				/* Status code from GMT API */
	struct GMT_OPTION *options = NULL;	/* Linked list of options */
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */

	/* 1. Initializing new GMT session */
	if (GMT_Create_Session (&API, argv[0], FUNC_MODE)) exit (EXIT_FAILURE);

	/* 2. Convert command line arguments to local linked option list */
	if (GMT_Create_Options (API, (GMT_LONG)(argc-1), (void *)(argv+1), &options)) exit (EXIT_FAILURE);

	/* 3. Run GMT cmd function, or give usage message if errors arise during parsing */
	status = (int)FUNC (API, options);

	/* 4. Destroy local linked option list */
	if (GMT_Destroy_Options (API, &options)) exit (EXIT_FAILURE);

	/* 5. Destroy GMT session */
	if (GMT_Destroy_Session (&API)) exit (EXIT_FAILURE);

	exit (status);		/* Return the status from FUNC */
}
