/*
 * Copyright (c) 2012-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 * See LICENSE.TXT file for copying and redistribution conditions.
 */

/* gmt_glue.c populates the external array of this shared lib with
 * module parameters such as name, group, purpose and keys strings.
 * This file also contains the following convenience functions to
 * display all module purposes or just list their names:
 *
 *   void gmt_mbsystem_module_show_all (struct GMTAPI_CTRL *API);
 *   void gmt_mbsystem_module_list_all (void *API);
 *
 * These functions may be called by gmt --help and gmt --show-modules
 *
 * Developers of external APIs for accessing GMT modules will use this
 * function indirectly via GMT_Encode_Options to retrieve option keys
 * needed for module arg processing:
 *
 *   char * gmt_mbsystem_module_keys (void *API, const char *module);
 *
 */
#include "gmt_dev.h"
#include "gmt_internals.h"

#define GMT_PURPOSE_STRING "mbsystem: MB-System modules accessible from gmt"

/* Sorted array with information for all GMT _mbsystem_ modules */

static struct GMT_MODULEINFO modules[] = {
#include "gmt_mbsystem_moduleinfo.h"
	{NULL, NULL, NULL, NULL, NULL} /* last element == NULL detects end of array */
};

/* Pretty print all shared module names and their purposes for gmt --help */
EXTERN_MSC void gmtlib_mbsystem_module_show_all (void *API) {
	gmtlib_module_show_all (API, modules, GMT_PURPOSE_STRING);
}

/* Produce single list on stdout of all shared module names for gmt --show-modules */
EXTERN_MSC void gmtlib_mbsystem_module_list_all (void *API) {
	gmtlib_module_list_all (API, modules);
}

/* Produce single list on stdout of all shared module names for gmt --show-classic [i.e., classic mode names] */
EXTERN_MSC void gmtlib_mbsystem_module_classic_all (void *API) {
	gmtlib_module_classic_all (API, modules);
}

/* Lookup module id by name, return option keys pointer (for external API developers) */
EXTERN_MSC const char *gmtlib_mbsystem_module_keys (void *API, char *candidate) {
	return (gmtlib_module_keys (API, modules, candidate));
}

/* Lookup module id by name, return group char name (for external API developers) */
EXTERN_MSC const char *gmtlib_mbsystem_module_group (void *API, char *candidate) {
	return (gmtlib_module_group (API, modules, candidate));
}
