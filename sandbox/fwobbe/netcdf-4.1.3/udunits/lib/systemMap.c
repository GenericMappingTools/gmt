/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * Module for system-to-pointer maps.
 *
 * This module is thread-compatible but not thread-safe.
 */

/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <search.h>
#include <stdlib.h>

#include "systemMap.h"
#include "udunits2.h"

struct SystemMap {
    void*	tree;
};

typedef struct {
    const ut_system*	system;
    void*		ptr;
} Entry;


/*
 * Compares two entries according to their unit-system pointers.
 *
 * Arguments:
 *	entry1	Pointer to the first entry.
 *	entry2	Pointer to the second entry.
 * Returns:
 *	-1	The first entry's key is less than the second's.
 *	 0	The first entry's key is equal to the second's.
 *	 1	The first entry's key is greater than the second's.
 */
static int
compareEntries(
    const void*	entry1,
    const void*	entry2)
{
    const ut_system* const	system1 = ((Entry*)entry1)->system;
    const ut_system* const	system2 = ((Entry*)entry2)->system;

    return system1 < system2 ? -1 : system1 == system2 ? 0 : 1;
}


/*
 * Returns a new instance of a system-map.
 *
 * Arguments:
 *	compare	Function for comparing keys.
 * Returns:
 *	NULL	Operating-system failure.  See "errno".
 *	else	Pointer to the new map.
 */
SystemMap*
smNew()
{
    SystemMap*	map = (SystemMap*)malloc(sizeof(SystemMap));

    if (map != NULL)
	map->tree = NULL;

    return map;
}


/*
 * Returns the address of the pointer to which a unit-system maps.
 *
 * Arguments:
 *	map	Pointer to the system-map.
 *	system	Pointer to the unit-system.
 * Returns:
 *	NULL	There is no pointer associated with "system".
 *	else	Address of the pointer to which "system" maps.
 */
void**
smFind(
    const SystemMap* const	map,
    const void* const		system)
{
    Entry	targetEntry;
    Entry**	treeEntry;

    targetEntry.system = system;
    treeEntry = tfind(&targetEntry, &map->tree, compareEntries);

    return
	treeEntry == NULL
	    ? NULL
	    : &(*treeEntry)->ptr;
}


/*
 * Returns the address of the pointer to which a unit-system maps -- creating a
 * new entry if necessary.  If a new entry is created, then the pointer whose
 * address is returned will be NULL.
 *
 * Arguments:
 *	map	Pointer to the system-map.
 *	system	Pointer to the unit-system.
 * Returns:
 *	NULL	Operating system failure.  See "errno".
 *	else	Address of the pointer to which "system" maps.
 */
void**
smSearch(
    SystemMap* const	map,
    const void*		system)
{
    void**	addr = NULL;		/* failure */
    Entry*	targetEntry = (Entry*)malloc(sizeof(Entry));

    if (targetEntry != NULL) {
	Entry**	treeEntry;

	targetEntry->system = system;
	targetEntry->ptr = NULL;
	treeEntry = tsearch(targetEntry, &map->tree, compareEntries);

	if (treeEntry == NULL) {
	    free(targetEntry);
	}
	else {
	    addr = &(*treeEntry)->ptr;

	    if (targetEntry != *treeEntry)
		free(targetEntry);
	}
    }

    return addr;
}


/*
 * Removes the system-map entry that corresponds to a unit-system.
 *
 * Arguments:
 *	map	Pointer to the map.
 *	system	Pointer to the unit-system.
 */
void
smRemove(
    SystemMap* const	map,
    const void* const	system)
{
    Entry	targetEntry;
    Entry**	treeEntry;

    targetEntry.system = system;
    treeEntry = tfind(&targetEntry, &map->tree, compareEntries);

    if (treeEntry != NULL) {
	Entry*	entry = *treeEntry;

	(void)tdelete(entry, &map->tree, compareEntries);
	free(entry);
    }
}


/*
 * Frees a system-map.  This function should be called when a system-map is no
 * longer needed.
 *
 * Arguments:
 *	map	Pointer to the system-map to be freed or NULL.  Use of "map"
 *		upon return results in undefined behavior.
 */
void
smFree(
    SystemMap* const	map)
{
    if (map != NULL) {
	while (map->tree != NULL) {
	    Entry*	entry = *(Entry**)map->tree;

	    tdelete(entry, &map->tree, compareEntries);
	    free(entry);
	}

	free(map);
    }
}
