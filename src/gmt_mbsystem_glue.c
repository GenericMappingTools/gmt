/*--------------------------------------------------------------------
 *
 *  Copyright (c) 2012-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *  See LICENSE.TXT file for copying and redistribution conditions.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 3 or any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/* gmt_mbsystem_glue.c populates the external array of this shared lib with
 * module parameters such as name, group, purpose and keys strings.
 * This file also contains the following convenience functions to
 * display all module purposes, list their names, or return keys or group:
 *
 *   int mbsystem_module_show_all    (void *API);
 *   int mbsystem_module_list_all    (void *API);
 *   int mbsystem_module_classic_all (void *API);
 *
 * These functions may be called by gmt --help and gmt --show-modules
 *
 * Developers of external APIs for accessing GMT modules will use this
 * function indirectly via GMT_Encode_Options to retrieve option keys
 * needed for module arg processing:
 *
 *   const char * mbsystem_module_keys  (void *API, char *candidate);
 *   const char * mbsystem_module_group (void *API, char *candidate);
 *
 * All functions are exported by the shared mbsystem library so that gmt can call these
 * functions by name to learn about the contents of the library.
 */

#include "gmt.h"

/* Sorted array with information for all GMT mbsystem modules */
static struct GMT_MODULEINFO modules[] = {
	{"mbcontour", "mbcontour", "mbsystem", "Plot swath bathymetry, amplitude, or backscatter", "CC(,>X}"},
	{"mbgrdtiff", "mbgrdtiff", "mbsystem", "Project grids or images and plot them on maps", "<G{+,CC(,IG("},
	{"mbswath", "mbswath", "mbsystem", "Plot swath bathymetry, amplitude, or backscatter", "CC(,NC(,>X}"},
	{NULL, NULL, NULL, NULL, NULL} /* last element == NULL detects end of array */
};

/* Pretty print all shared module names and their purposes for gmt --help */
EXTERN_MSC int mbsystem_module_show_all (void *API) {
	return (GMT_Show_ModuleInfo (API, modules, "MB-System: GMT-compatible modules", GMT_MODULE_HELP));
}

/* Produce single list on stdout of all shared module names for gmt --show-modules */
EXTERN_MSC int mbsystem_module_list_all (void *API) {
	return (GMT_Show_ModuleInfo (API, modules, NULL, GMT_MODULE_SHOW_MODERN));
}

/* Produce single list on stdout of all shared module names for gmt --show-classic [i.e., classic mode names] */
EXTERN_MSC int mbsystem_module_classic_all (void *API) {
	return (GMT_Show_ModuleInfo (API, modules, NULL, GMT_MODULE_SHOW_CLASSIC));
}

/* Lookup module id by name, return option keys pointer (for external API developers) */
EXTERN_MSC const char *mbsystem_module_keys (void *API, char *candidate) {
	return (GMT_Get_ModuleInfo (API, modules, candidate, GMT_MODULE_KEYS));
}

/* Lookup module id by name, return group char name (for external API developers) */
EXTERN_MSC const char *mbsystem_module_group (void *API, char *candidate) {
	return (GMT_Get_ModuleInfo (API, modules, candidate, GMT_MODULE_GROUP));
}
