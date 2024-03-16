/*
 * Copyright (c) 2012-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 * See LICENSE.TXT file for copying and redistribution conditions.
 */
/* gmt_gsfml_glue.c populates the external array of this shared lib with
 * module parameters such as name, group, purpose and keys strings.
 * This file also contains the following convenience functions to
 * display all module purposes, list their names, or return keys or group:
 *
 *   int gsfml_module_show_all    (void *API);
 *   int gsfml_module_list_all    (void *API);
 *   int gsfml_module_classic_all (void *API);
 *
 * These functions may be called by gmt --help and gmt --show-modules
 *
 * Developers of external APIs for accessing GMT modules will use this
 * function indirectly via GMT_Encode_Options to retrieve option keys
 * needed for module arg processing:
 *
 *   const char * gsfml_module_keys  (void *API, char *candidate);
 *   const char * gsfml_module_group (void *API, char *candidate);
 *
 * All functions are exported by the shared gsfml library so that gmt can call these
 * functions by name to learn about the contents of the library.
 */

#include "gmt_dev.h"

/* Sorted array with information for all GMT gsfml modules */
static struct GMT_MODULEINFO modules[] = {
	{"fzanalyzer", "fzanalyzer", "gsfml", "Analysis of fracture zones using crossing profiles", "<DI,>DO"},
	{"fzblender", "fzblender", "gsfml", "Produce a smooth blended FZ trace", "<DI,>DO"},
	{"mlconverter", "mlconverter", "gsfml", "Convert chrons to ages using selected magnetic timescale", "<DI,>DO"},
	{NULL, NULL, NULL, NULL, NULL} /* last element == NULL detects end of array */
};

/* Pretty print all shared module names and their purposes for gmt --help */
EXTERN_MSC int gsfml_module_show_all (void *API) {
	return (GMT_Show_ModuleInfo (API, modules, "GMT supplemental modules for GSFML", GMT_MODULE_HELP));
}

/* Produce single list on stdout of all shared module names for gmt --show-modules */
EXTERN_MSC int gsfml_module_list_all (void *API) {
	return (GMT_Show_ModuleInfo (API, modules, NULL, GMT_MODULE_SHOW_MODERN));
}

/* Produce single list on stdout of all shared module names for gmt --show-classic [i.e., classic mode names] */
EXTERN_MSC int gsfml_module_classic_all (void *API) {
	return (GMT_Show_ModuleInfo (API, modules, NULL, GMT_MODULE_SHOW_CLASSIC));
}

/* Lookup module id by name, return option keys pointer (for external API developers) */
EXTERN_MSC const char *gsfml_module_keys (void *API, char *candidate) {
	return (GMT_Get_ModuleInfo (API, modules, candidate, GMT_MODULE_KEYS));
}

/* Lookup module id by name, return group char name (for external API developers) */
EXTERN_MSC const char *gsfml_module_group (void *API, char *candidate) {
	return (GMT_Get_ModuleInfo (API, modules, candidate, GMT_MODULE_GROUP));
}
