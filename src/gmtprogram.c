/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-$year by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * The template for all GMT5 programs that are built from their modules.
 * Define MODULE_ID with the module id from enum Gmt_module.
 *
 * Version:	5
 * Created:	14-Sep-2011
 *
 */

#include "pslib.h"
#include "gmt.h"

int main (int argc, char *argv[]) {
	int status = EXIT_SUCCESS;           /* Status code from GMT API */
	struct GMTAPI_CTRL *api_ctrl = NULL; /* GMT API control structure */
	/* Structure containing name, purpose, Api_mode, and function pointer of this module: */
	struct Gmt_moduleinfo this_module = g_module[MODULE_ID];

	/* 1. Initializing new GMT session */
	if ((api_ctrl = GMT_Create_Session (argv[0], this_module.api_mode)) == NULL)
		return EXIT_FAILURE;

	/* 2. Run GMT function, or give usage message if errors arise during parsing */
	status = this_module.p_func (api_ctrl, argc-1, (argv+1));

	/* 3. Destroy GMT session */
	if (GMT_Destroy_Session (&api_ctrl) != GMT_OK)
		return EXIT_FAILURE;

	return status; /* Return the status from FUNC */
}
