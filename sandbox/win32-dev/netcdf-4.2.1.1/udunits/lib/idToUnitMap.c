/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * Identifier-to-unit map.
 */

/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <assert.h>
#include <search.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "udunits2.h"
#include "unitAndId.h"
#include "systemMap.h"

typedef struct {
    int			(*compare)(const void*, const void*);
    void*		tree;
} IdToUnitMap;

static SystemMap*	systemToNameToUnit;
static SystemMap*	systemToSymbolToUnit;


static int
sensitiveCompare(
    const void* const	node1,
    const void* const	node2)
{
    return strcmp(((const UnitAndId*)node1)->id, 
	((const UnitAndId*)node2)->id);
}


static int
insensitiveCompare(
    const void* const	node1,
    const void* const	node2)
{
    return strcasecmp(((const UnitAndId*)node1)->id, 
	((const UnitAndId*)node2)->id);
}


static IdToUnitMap*
itumNew(
    int		(*compare)(const void*, const void*))
{
    IdToUnitMap*	map = (IdToUnitMap*)malloc(sizeof(IdToUnitMap));

    if (map != NULL) {
	map->tree = NULL;
	map->compare = compare;
    }

    return map;
}


/*
 * Frees an identifier-to-unit map.  All entries are freed.
 *
 * Arguments:
 *	map		Pointer to the identifier-to-unit map.
 * Returns:
 */
static void
itumFree(
    IdToUnitMap*	map)
{
    if (map != NULL) {
	while (map->tree != NULL) {
	    UnitAndId*	uai = *(UnitAndId**)map->tree;

	    (void)tdelete(uai, &map->tree, map->compare);
	    uaiFree(uai);
	}

	free(map);
    }					/* valid arguments */
}


/*
 * Adds an entry to an identifier-to-unit map.
 *
 * Arguments:
 *	map		The database.
 *	id		The identifier.  May be freed upon return.
 *	unit		The unit.  May be freed upon return.
 * Returns:
 *	UT_OS		Operating-system error.  See "errno".
 *	UT_EXISTS	"id" already maps to a different unit.
 *	UT_SUCCESS	Success.
 */
static ut_status
itumAdd(
    IdToUnitMap*		map,
    const char* const		id,
    const ut_unit* const	unit)
{
    ut_status		status;
    UnitAndId*		targetEntry;

    assert(map != NULL);
    assert(id != NULL);
    assert(unit != NULL);

    targetEntry = uaiNew(unit, id);

    if (targetEntry != NULL) {
	UnitAndId**	treeEntry = tsearch(targetEntry, &map->tree,
	    map->compare);

	if (treeEntry == NULL) {
	    uaiFree(targetEntry);
	    status = UT_OS;
	}
	else {
	    if (ut_compare((*treeEntry)->unit, unit) == 0) {
		status = UT_SUCCESS;
	    }
	    else {
		status = UT_EXISTS;
                ut_set_status(status);
		ut_handle_error_message(
		    "\"%s\" already maps to existing but different unit", id);
	    }

            if (targetEntry != *treeEntry)
                uaiFree(targetEntry);
	}				/* found entry */
    }					/* "targetEntry" allocated */

    return status;
}


/*
 * Removes an entry to an identifier-to-unit map.
 *
 * Arguments:
 *	map		The database.
 *	id		The identifier.  May be freed upon return.
 * Returns:
 *	UT_SUCCESS	Success.
 */
static ut_status
itumRemove(
    IdToUnitMap*	map,
    const char* const	id)
{
    UnitAndId		targetEntry;
    UnitAndId**		treeEntry;

    assert(map != NULL);
    assert(id != NULL);
    
    targetEntry.id = (char*)id;
    treeEntry = tfind(&targetEntry, &map->tree, map->compare);

    if (treeEntry != NULL) {
	UnitAndId*	uai = *treeEntry;

	(void)tdelete(uai, &map->tree, map->compare);
	uaiFree(uai);
    }

    return UT_SUCCESS;
}


/*
 * Finds the entry in an identifier-to-unit map that corresponds to an
 * identifer.
 *
 * Arguments:
 *	map	The identifier-to-unit map.
 *	id	The identifier to be used as the key in the search.
 * Returns:
 *	NULL	Failure.  "map" doesn't contain an entry that corresponds
 *		to "id".
 *	else	Pointer to the entry corresponding to "id".
 */
static const UnitAndId*
itumFind(
    IdToUnitMap*	map,
    const char* const	id)
{
    UnitAndId*		entry = NULL;	/* failure */
    UnitAndId		targetEntry;
    UnitAndId**		treeEntry;

    assert(map != NULL);
    assert(id != NULL);

    targetEntry.id = (char*)id;
    treeEntry = tfind(&targetEntry, &map->tree, map->compare);

    if (treeEntry != NULL)
	entry = *treeEntry;

    return entry;
}


/*
 * Adds to a particular unit-system a mapping from an identifier to a unit.
 *
 * Arguments:
 *	systemMap	Address of the pointer to the system-map.
 *	id		Pointer to the identifier.  May be freed upon return.
 *	unit		Pointer to the unit.  May be freed upon return.
 *	compare		Pointer to comparison function for unit-identifiers.
 * Returns:
 *	UT_BAD_ARG	"id" is NULL or "unit" is NULL.
 *	UT_OS		Operating-sytem failure.  See "errno".
 *	UT_SUCCESS	Success.
 */
static ut_status
mapIdToUnit(
    SystemMap** const		systemMap,
    const char* const		id,
    const ut_unit* const	unit,
    int				(*compare)(const void*, const void*))
{
    ut_status		status = UT_SUCCESS;

    if (id == NULL) {
	status = UT_BAD_ARG;
    }
    else if (unit == NULL) {
	status = UT_BAD_ARG;
    }
    else {
	ut_system*	system = ut_get_system(unit);

	if (*systemMap == NULL) {
	    *systemMap = smNew();

	    if (*systemMap == NULL)
		status = UT_OS;
	}

	if (*systemMap != NULL) {
	    IdToUnitMap** const	idToUnit =
		(IdToUnitMap**)smSearch(*systemMap, system);

	    if (idToUnit == NULL) {
		status = UT_OS;
	    }
	    else {
		if (*idToUnit == NULL) {
		    *idToUnit = itumNew(compare);

		    if (*idToUnit == NULL)
			status = UT_OS;
		}

		if (*idToUnit != NULL)
		    status = itumAdd(*idToUnit, id, unit);
	    }				/* have system-map entry */
	}				/* have system-map */
    }					/* valid arguments */

    return status;
}


/*
 * Removes the mapping from an identifier to a unit.
 *
 * Arguments:
 *	systemMap	Address of the pointer to the system-map.
 *	id		Pointer to the identifier.  May be freed upon return.
 *	system		Pointer to the unit-system associated with the mapping.
 * Returns:
 *	UT_BAD_ARG	"id" is NULL, "system" is NULL, or "compare" is NULL.
 *	UT_SUCCESS	Success.
 */
static ut_status
unmapId(
    SystemMap* const	systemMap,
    const char* const	id,
    ut_system*		system)
{
    ut_status		status;

    if (systemMap == NULL || id == NULL || system == NULL) {
	status = UT_BAD_ARG;
    }
    else {
	IdToUnitMap** const	idToUnit =
	    (IdToUnitMap**)smFind(systemMap, system);

	status = 
	    (idToUnit == NULL || *idToUnit == NULL)
		? UT_SUCCESS
		: itumRemove(*idToUnit, id);
    }					/* valid arguments */

    return status;
}


/*
 * Adds a mapping from a name to a unit.
 *
 * Arguments:
 *	name		Pointer to the name to be mapped to "unit".  May be
 *			freed upon return.
 *      encoding        The character encoding of "name".
 *	unit		Pointer to the unit to be mapped-to by "name".  May be
 *			freed upon return.
 * Returns:
 *	UT_BAD_ARG	"name" or "unit" is NULL.
 *	UT_OS		Operating-system error.  See "errno".
 *	UT_EXISTS	"name" already maps to a different unit.
 *	UT_SUCCESS	Success.
 */
ut_status
ut_map_name_to_unit(
    const char* const		name,
    const ut_encoding		encoding,
    const ut_unit* const	unit)
{
    ut_set_status(
	mapIdToUnit(&systemToNameToUnit, name, unit, insensitiveCompare));

    return ut_get_status();
}


/*
 * Removes a mapping from a name to a unit.  After this function,
 * ut_get_unit_by_name(system,name) will no longer return a unit.
 *
 * Arguments:
 *	system		The unit-system to which the unit belongs.
 *	name		The name of the unit.
 *      encoding        The character encoding of "name".
 * Returns:
 *	UT_SUCCESS	Success.
 *	UT_BAD_ARG	"system" or "name" is NULL.
 */
ut_status
ut_unmap_name_to_unit(
    ut_system*		system,
    const char* const	name,
    const ut_encoding   encoding)
{
    ut_set_status(unmapId(systemToNameToUnit, name, system));

    return ut_get_status();
}


/*
 * Adds a mapping from a symbol to a unit.
 *
 * Arguments:
 *	symbol		Pointer to the symbol to be mapped to "unit".  May be
 *			freed upon return.
 *      encoding        The character encoding of "symbol".
 *	unit		Pointer to the unit to be mapped-to by "symbol".  May
 *			be freed upon return.
 * Returns:
 *	UT_BAD_ARG	"symbol" or "unit" is NULL.
 *	UT_OS		Operating-system error.  See "errno".
 *	UT_EXISTS	"symbol" already maps to a different unit.
 *	UT_SUCCESS	Success.
 */
ut_status
ut_map_symbol_to_unit(
    const char* const		symbol,
    const ut_encoding		encoding,
    const ut_unit* const	unit)
{
    ut_set_status(
	mapIdToUnit(&systemToSymbolToUnit, symbol, unit, sensitiveCompare));

    return ut_get_status();
}


/*
 * Removes a mapping from a symbol to a unit.  After this function,
 * ut_get_unit_by_symbol(system,symbol) will no longer return a unit.
 *
 * Arguments:
 *	system		The unit-system to which the unit belongs.
 *	symbol		The symbol of the unit.
 *      encoding        The character encoding of "symbol".
 * Returns:
 *	UT_SUCCESS	Success.
 *	UT_BAD_ARG	"system" or "symbol" is NULL.
 */
ut_status
ut_unmap_symbol_to_unit(
    ut_system*		system,
    const char* const	symbol,
    const ut_encoding   encoding)
{
    ut_set_status(unmapId(systemToSymbolToUnit, symbol, system));

    return ut_get_status();
}


/*
 * Returns the unit to which an identifier maps in a particular unit-system.
 *
 * Arguments:
 *	systemMap	NULL or pointer to the system-map.  If NULL, then
 *			NULL will be returned.
 *	system		Pointer to the unit-system.
 *	id		Pointer to the identifier.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_BAD_ARG	        "system" is NULL or "id" is NULL.
 *	else	Pointer to the unit in "system" with the identifier "id".
 *		Should be passed to ut_free() when no longer needed.
 */
static ut_unit*
getUnitById(
    const SystemMap* const	systemMap,
    const ut_system* const	system,
    const char* const		id)
{
    ut_unit*	unit = NULL;		/* failure */

    if (system == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("getUnitById(): NULL unit-system argument");
    }
    else if (id == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("getUnitById(): NULL identifier argument");
    }
    else if (systemMap != NULL) {
	IdToUnitMap** const	idToUnit =
	    (IdToUnitMap**)smFind(systemMap, system);

	if (idToUnit != NULL) {
	    const UnitAndId*	uai = itumFind(*idToUnit, id);

	    if (uai != NULL)
		unit = ut_clone(uai->unit);
	}
    }					/* valid arguments */

    return unit;
}


/*
 * Returns the unit with a given name from a unit-system.  Name comparisons
 * are case-insensitive.
 *
 * Arguments:
 *	system	Pointer to the unit-system.
 *	name	Pointer to the name of the unit to be returned.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be
 *		    UT_SUCCESS		"name" doesn't map to a unit of
 *					"system".
 *		    UT_BAD_ARG		"system" or "name" is NULL.
 *	else	Pointer to the unit of the unit-system with the given name.
 *		The pointer should be passed to ut_free() when the unit is
 *		no longer needed.
 */
ut_unit*
ut_get_unit_by_name(
    const ut_system* const	system,
    const char* const		name)
{
    ut_set_status(UT_SUCCESS);

    return getUnitById(systemToNameToUnit, system, name);
}


/*
 * Returns the unit with a given symbol from a unit-system.  Symbol 
 * comparisons are case-sensitive.
 *
 * Arguments:
 *	system		Pointer to the unit-system.
 *	symbol		Pointer to the symbol associated with the unit to be
 *			returned.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be
 *		    UT_SUCCESS		"symbol" doesn't map to a unit of
 *					"system".
 *		    UT_BAD_ARG		"system" or "symbol" is NULL.
 *	else	Pointer to the unit in the unit-system with the given symbol.
 *		The pointer should be passed to ut_free() when the unit is no
 *		longer needed.
 */
ut_unit*
ut_get_unit_by_symbol(
    const ut_system* const	system,
    const char* const		symbol)
{
    ut_set_status(UT_SUCCESS);

    return getUnitById(systemToSymbolToUnit, system, symbol);
}


/*
 * Frees resources associated with a unit-system.
 *
 * Arguments:
 *	system		Pointer to the unit-system to have its associated
 *			resources freed.
 */
void
itumFreeSystem(
    ut_system*	system)
{
    if (system != NULL) {
	SystemMap*	systemMaps[2];
	int		i;

	systemMaps[0] = systemToNameToUnit;
	systemMaps[1] = systemToSymbolToUnit;

	for (i = 0; i < 2; i++) {
	    if (systemMaps[i] != NULL) {
		IdToUnitMap** const	idToUnit =
		    (IdToUnitMap**)smFind(systemMaps[i], system);

		if (idToUnit != NULL)
		    itumFree(*idToUnit);

		smRemove(systemMaps[i], system);
	    }
	}
    }					/* valid arguments */
}
