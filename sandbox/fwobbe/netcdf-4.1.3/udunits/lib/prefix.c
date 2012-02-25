/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * Module for handling unit prefixes -- both names and symbols.
 */

/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <ctype.h>
#include <errno.h>
#include <search.h>
#include <stdlib.h>
#include <string.h>

#include "udunits2.h"
#include "systemMap.h"

typedef struct {
    void*	tree;
    int		(*compare)(const void*, const void*);
} PrefixToValueMap;

typedef struct {
    void*	nextTree;
    double	value;
    size_t	position;	/* origin-0 index of character in prefix */
    int		character;
} PrefixSearchEntry;

static SystemMap*	systemToNameToValue = NULL;
static SystemMap*	systemToSymbolToValue = NULL;


/******************************************************************************
 * Prefix Search Entry:
 ******************************************************************************/


static PrefixSearchEntry*
pseNew(
    int			character,
    size_t		position)
{
    PrefixSearchEntry*	entry = malloc(sizeof(PrefixSearchEntry));

    if (entry == NULL) {
	ut_set_status(UT_OS);
	ut_handle_error_message(strerror(errno));
	ut_handle_error_message(
            "Couldn't allocate %lu-byte prefix-search-entry",
	    sizeof(PrefixSearchEntry));
    }
    else {
	entry->character = character;
	entry->position = position;
	entry->value = 0;
	entry->nextTree = NULL;
    }

    return entry;
}


static void
pseFree(
    PrefixSearchEntry* const	entry)
{
    free(entry);
}


static int
pseSensitiveCompare(
    const void* const	entry1,
    const void* const	entry2)
{
    int	char1 = ((const PrefixSearchEntry*)entry1)->character;
    int	char2 = ((const PrefixSearchEntry*)entry2)->character;

    return char1 < char2 ? -1 : char1 == char2 ? 0 : 1;
}


static int
pseInsensitiveCompare(
    const void* const	entry1,
    const void* const	entry2)
{
    int	char1 = tolower(((const PrefixSearchEntry*)entry1)->character);
    int	char2 = tolower(((const PrefixSearchEntry*)entry2)->character);

    return char1 < char2 ? -1 : char1 == char2 ? 0 : 1;
}


/******************************************************************************
 * Prefix-to-Value Map:
 ******************************************************************************/


static PrefixToValueMap*
ptvmNew(
    int		(*compare)(const void*, const void*))
{
    PrefixToValueMap*	map =
	(PrefixToValueMap*)malloc(sizeof(PrefixToValueMap));

    if (map != NULL) {
	map->tree = NULL;
	map->compare = compare;
    }

    return map;
}


/*
 * Returns the prefix search-entry that matches an identifier.  Inserts a
 * new prefix search-entry if no matching element is found.  Note that the
 * returned entry might have a different prefix value if it was previously
 * inserted.
 *
 * Arguments:
 *	map		Pointer to the prefix-to-value map.
 *	id		The prefix identifier.  May be freed upon return.
 *	value		The prefix value.
 * Returns:
 *	NULL		"map" is NULL.
 *	NULL		"id" is NULL or the empty string.
 *	NULL		"value" is 0.
 *	NULL		Insufficient storage space is available.
 *	else		Pointer to the prefix-search-entry that matches "id".
 */
static const PrefixSearchEntry*
ptvmSearch(
    PrefixToValueMap*	map,
    const char* const	id,
    const double	value)
{
    PrefixSearchEntry*	entry = NULL;	/* failure */

    if (id != NULL && map != NULL && value != 0) {
	size_t	len = strlen(id);

	if (len > 0) {
	    size_t			i;
	    PrefixSearchEntry* const*	treeEntry = NULL;
	    void**			tree = &map->tree;

	    for (i = 0; i < len; i++) {
		PrefixSearchEntry* const	newEntry = pseNew(id[i], i);

		if (newEntry == NULL)
		    break;

		treeEntry = tsearch(newEntry, tree, map->compare);

		if (treeEntry == NULL) {
		    pseFree(newEntry);
		    break;
		}

		tree = &(*treeEntry)->nextTree;	/* next binary-search tree */

		if (newEntry != *treeEntry)
		    pseFree(newEntry);
	    }

	    if (i >= len) {
		entry = *treeEntry;

		if (entry->value == 0)
		    entry->value = value;
	    }
	}
    }

    return entry;
}


/*
 * Returns the prefix search-entry that matches the beginning of a string.
 *
 * Arguments:
 *	map		Pointer to the prefix-to-value map.
 *	string		Pointer to the string to be examined for a prefix.
 * Returns:
 *	NULL		"map" is NULL.
 *	NULL		"string" is NULL or the empty string.
 *	NULL		"value" is 0.
 *	else		Pointer to the prefix-search-entry that matches the
 *			beginning of "string".
 */
static const PrefixSearchEntry*
ptvmFind(
    PrefixToValueMap* const	map,
    const char* const		string)
{
    PrefixSearchEntry*	entry = NULL;	/* failure */

    if (string != NULL && map != NULL && strlen(string) > 0) {
	size_t	len = strlen(string);

	if (len > 0) {
	    size_t			i;
	    PrefixSearchEntry*		lastEntry = NULL;
	    void**			tree = &map->tree;

	    for (i = 0; i < len; i++) {
		PrefixSearchEntry		targetEntry;
		PrefixSearchEntry* const*	treeEntry;
		
		targetEntry.character = string[i];
		treeEntry = tfind(&targetEntry, tree, map->compare);

		if (treeEntry == NULL)
		    break;

		lastEntry = *treeEntry;

		tree = &(*treeEntry)->nextTree;	/* next binary-search tree */
	    }

	    if (lastEntry != NULL && lastEntry->value != 0)
		entry = lastEntry;
	}
    }

    return entry;
}


/******************************************************************************
 * Public API:
 ******************************************************************************/


/*
 * Adds a prefix to a unit-system.
 *
 * Arguments:
 *	system		Pointer to the unit-system.
 *	prefix		Pointer to the prefix (e.g., "mega", "M").  May be freed
 *			upon return.
 *	value		The value of the prefix (e.g., 1e6).
 *	systemMap	Pointer to system-map.
 *	compare		Prefix comparison function.
 * Returns:
 *	UT_SUCCESS	Success.
 *	UT_BAD_ARG	"system" is NULL, "prefix" is NULL or empty, or "value"
 *                      is 0.
 *	UT_EXISTS	"prefix" already maps to a different value.
 *	UT_OS		Operating-system failure.  See "errno".
 */
static ut_status
addPrefix(
    ut_system* const	system,
    const char* const	prefix,
    const double	value,
    SystemMap** const	systemMap,
    int			(*compare)(const void*, const void*))
{
    ut_status		status;

    if (system == NULL) {
	status = UT_BAD_ARG;
    }
    else if (prefix == NULL || strlen(prefix) == 0) {
	status = UT_BAD_ARG;
    }
    else if (value == 0) {
	status = UT_BAD_ARG;
    }
    else {
	if (*systemMap == NULL) {
	    *systemMap = smNew();

	    if (*systemMap == NULL)
		status = UT_OS;
	}

	if (*systemMap != NULL) {
	    PrefixToValueMap** const	prefixToValue =
		(PrefixToValueMap**)smSearch(*systemMap, system);

	    if (prefixToValue == NULL) {
		status = UT_OS;
	    }
	    else {
		if (*prefixToValue == NULL) {
		    *prefixToValue = ptvmNew(compare);

		    if (*prefixToValue == NULL)
			status = UT_OS;
		}

		if (*prefixToValue != NULL) {
		    const PrefixSearchEntry*	entry =
			ptvmSearch(*prefixToValue, prefix, value);

		    status = 
			entry == NULL
			    ? UT_OS
			    : (entry->value == value)
				? UT_SUCCESS
				: UT_EXISTS;
		}
	    }				/* have system-map entry */
	}				/* have system-map */
    }					/* valid arguments */

    return status;
}


/*
 * Adds a name-prefix to a unit-system.  A name-prefix is something like "mega"
 * or "milli".  Comparisons between name-prefixes are case-insensitive.
 *
 * Arguments:
 *	system		Pointer to the unit-system.
 *	name		Pointer to the name-prefix (e.g., "mega").  May be freed
 *			upon return.
 *	value		The value of the prefix (e.g., 1e6).
 * Returns:
 *	UT_SUCCESS	Success.
 *	UT_BAD_ARG	"system" or "name" is NULL, or "value" is 0.
 *	UT_EXISTS	"name" already maps to a different value.
 *	UT_OS		Operating-system failure.  See "errno".
 */
ut_status
ut_add_name_prefix(
    ut_system* const	system,
    const char* const	name,
    const double	value)
{
    ut_set_status(addPrefix(system, name, value, &systemToNameToValue,
	pseInsensitiveCompare));

    return ut_get_status();
}


/*
 * Adds a symbol-prefix to a unit-system.  A symbol-prefix is something like
 * "M" or "y".  Comparisons between symbol-prefixes are case-sensitive.
 *
 * Arguments:
 *	system		Pointer to the unit-system.
 *	symbol		Pointer to the symbol-prefix (e.g., "M").  May be freed
 *			upon return.
 *	value		The value of the prefix (e.g., 1e6).
 * Returns:
 *	UT_SUCCESS	Success.
 *	UT_BADSYSTEM	"system" or "symbol" is NULL.
 *	UT_BAD_ARG	"value" is 0.
 *	UT_EXISTS	"symbol" already maps to a different value.
 *	UT_OS		Operating-system failure.  See "errno".
 */
ut_status
ut_add_symbol_prefix(
    ut_system* const	system,
    const char* const	symbol,
    const double	value)
{
    ut_set_status(addPrefix(system, symbol, value, &systemToSymbolToValue,
	pseSensitiveCompare));

    return ut_get_status();
}


/*
 * Finds a prefix of a unit-system.
 *
 * Arguments:
 *	system		Pointer to the unit-system.
 *	systemMap	Pointer to system-map.
 *	string		Pointer to the string to be examined for a prefix.
 *	compare		Prefix comparison function.
 *	value		NULL or pointer to the memory location to receive the
 *			value of the name-prefix, if one is discovered.
 *	len		NULL or pointer to the memory location to receive the
 *			number of characters in the name-prefix, if one is
 *			discovered.
 *	
 * Returns:
 *	UT_SUCCESS	Success.
 *	UT_BAD_ARG	"system" is NULL, "systemMap" is NULL, "compare" is
 *                      NULL, "string" is NULL or empty, or "value" is 0.
 *	UT_OS		Operating-system failure.  See "errno".
 *	UT_UNKNOWN	No prefix-to-value map is associated with "system".
 *	UT_UNKNOWN	No prefix found in the prefix-to-value map associated
 *			with "system".
 */
static ut_status
findPrefix(
    ut_system* const	system,
    SystemMap* const	systemMap,
    const char* const	string,
    double* const	value,
    size_t* const	len)
{
    ut_status		status;

    if (system == NULL) {
	status = UT_BAD_ARG;
    }
    else if (systemMap == NULL) {
	status = UT_BAD_ARG;
    }
    else if (string == NULL || strlen(string) == 0) {
	status = UT_BAD_ARG;
    }
    else {
	PrefixToValueMap** const	prefixToValue =
	    (PrefixToValueMap**)smFind(systemMap, system);

	if (prefixToValue == NULL) {
	    status = UT_UNKNOWN;
	}
	else {
	    const PrefixSearchEntry*	entry =
		ptvmFind(*prefixToValue, string);

	    if (entry == NULL) {
		status = UT_UNKNOWN;
	    }
	    else {
		if (value != NULL)
		    *value = entry->value;

		if (len != NULL)
		    *len = entry->position + 1;

		status = UT_SUCCESS;
	    }				/* have prefix entry */
	}				/* have system-map entry */
    }					/* valid arguments */

    return status;
}


/*
 * Examines a string for a name-prefix and returns the length of the name-prefix
 * and its value if one is discovered.
 *
 * Arguments:
 *	system	Pointer to the unit-system.
 *	string	Pointer to the string to be examined for a name-prefix.
 *	value	NULL or pointer to the memory location to receive the value of
 *		the name-prefix, if one is discovered.
 *	len	NULL or pointer to the memory location to receive the number of
 *		characters in the name-prefix, if one is discovered.
 * Returns:
 *	UT_BAD_ARG	"string" is NULL.
 *	UT_UNKNOWN	A name-prefix was not discovered.
 *	UT_SUCCESS	Success.  "*value" and "*len" will be set if non-NULL.
 */
ut_status
utGetPrefixByName(
    ut_system* const	system,
    const char* const	string,
    double* const	value,
    size_t* const	len)
{
    return
	string == NULL
	    ? UT_BAD_ARG
	    : findPrefix(system, systemToNameToValue, string, value, len);
}


/*
 * Examines a string for a symbol-prefix and returns the length of the
 * symbol-prefix and its value if one is discovered.
 *
 * Arguments:
 *	system	Pointer to the unit-system.
 *	string	Pointer to the string to be examined for a symbol-prefix.
 *	value	NULL or pointer to the memory location to receive the value of
 *		the symbol-prefix, if one is discovered.
 *	len	NULL or pointer to the memory location to receive the number of
 *		characters in the symbol-prefix, if one is discovered.
 * Returns:
 *	UT_BAD_ARG	"string" is NULL.
 *	UT_UNKNOWN	A symbol-prefix was not discovered.
 *	UT_SUCCESS	Success.  "*value" and "*len" will be set if non-NULL.
 */
ut_status
utGetPrefixBySymbol(
    ut_system* const	system,
    const char* const	string,
    double* const	value,
    size_t* const	len)
{
    return
	string == NULL
	    ? UT_BAD_ARG
	    : findPrefix(system, systemToSymbolToValue, string, value, len);
}
