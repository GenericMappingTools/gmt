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
 * The template for all GMT5 programs that are built from their modules.
 * Define MODULE with the module name (e.g., "psxy") via compiler definition.
 * Only used under Windows when links to gmt.exe do not work.
 *
 * Version:	5
 * Created:	21-Jun-2013
 *
 */

#include "gmt_dev.h"

int main (int argc, char *argv[]) {
	int status = GMT_NOERROR;           /* Status code from GMT API */
	struct GMTAPI_CTRL *api_ctrl = NULL; /* GMT API control structure */
	/* Structure containing name, purpose, Api_mode, and function pointer of this module: */

	/* 1. Initializing new GMT session */
	if ((api_ctrl = GMT_Create_Session (NULL, GMT_PAD_DEFAULT, GMT_SESSION_NORMAL, NULL)) == NULL)
		return EXIT_FAILURE;

	/* 2. Run GMT module, or give usage message if errors arise during parsing */
	status = GMT_Call_Module (api_ctrl, MODULE, argc-1, (argv+1));

	/* 3. Destroy GMT session */
	if (GMT_Destroy_Session (api_ctrl) != GMT_NOERROR)
		return EXIT_FAILURE;

	return status; /* Return the status from the module */
}
