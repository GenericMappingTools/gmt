/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * Unit-to-identifier map.
 */

/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <assert.h>
#include <errno.h>
#include <search.h>
#include <stdlib.h>
#include <string.h>

#include "udunits2.h"
#include "unitAndId.h"
#include "unitToIdMap.h"		/* this module's API */
#include "systemMap.h"

typedef struct {
    void*		ascii;
    void*		latin1;
    void*		utf8;
} UnitToIdMap;

static SystemMap*	systemToUnitToName = NULL;
static SystemMap*	systemToUnitToSymbol = NULL;


/******************************************************************************
 * Miscellaneous Functions:
 ******************************************************************************/

/*
 * Unconditionally converts an ISO Latin-1 string into a UTF-8 string.
 *
 * Arguments:
 *	latin1String	Pointer to the ISO Latin-1 string.
 * Returns:
 *	NULL		Failure.  See errno.
 *	else		Pointer to the equivalent UTF-8 string.  Should be freed
 *			when no longer needed.
 */
static char*
latin1ToUtf8(
    const char* const	latin1String)
{
    int			nchar;
    const char*		inp;
    char*		outp;
    char*		utf8String;

    for (nchar = 0, inp = latin1String; *inp; ++inp, ++nchar)
	if ((*inp & 0x80U) != 0)
	    nchar++;

    utf8String = malloc(nchar+1);

    if (utf8String != NULL) {
	for (inp = latin1String, outp = utf8String; *inp; ++inp, ++outp) {
	    if ((*inp & 0x80U) == 0) {
		*outp = *inp;
	    }
	    else {
		*outp++ = (char)(0xC0U | ((unsigned)*inp >> 6));
		*outp = (char)(0x80U | (*inp & 0x3FU));
	    }
	}

	*outp = 0;
    }

    return utf8String;
}

/*
 * Adjust a given encoding according to a string.  Because ASCII is a subset of
 * ISO Latin-1 and because a UTF-8 encoded string must follow certain rules,
 * it's possible for strings to be mis-encoded and for an encoding specification
 * to be too restrictive or over-generous.  If the encoding specification is
 * ASCII and the given string contains a character with the high-order bit
 * set, then the encoding will be changed to ISO Latin-1.  If the encoding
 * specification is ISO Latin-1 but the string doesn't contain a character with
 * the high-order bit set, then the encoding will be changed to ASCII.  If the
 * encoding specification is UTF-8 but the string doesn't follow the UTF-8
 * encoding rules, then the function will error-return.
 *
 * Arguments:
 *	encoding	Pointer to the presumptive encoding.  Might be modified
 *			on return to reflect the actual, most restrictive,
 *			encoding of "string".
 *	string		The string to be checked.
 * Returns:
 *	0		Success.  "*encoding" might be modified.
 *	-1		Failure.  "string" doesn't conform to "encoding".
 */
static int
adjustEncoding(
    ut_encoding* const	encoding,
    const char* 	string)
{
    int		status = 0;		/* success */

    if (*encoding == UT_ASCII) {
	for (; *string && ((*string & 0x80U) == 0); string++)
	    ;

	if (*string != 0)
	    *encoding = UT_LATIN1;
    }
    else if (*encoding == UT_LATIN1) {
	for (; *string && ((*string & 0x80U) == 0); string++)
	    ;

	if (*string == 0)
	    *encoding = UT_ASCII;
    }
    else if (*encoding == UT_UTF8) {
	for (; *string; string++) {
	    if (*string & 0x80U) {
		if ((*string & 0xE0U) == 0xC0U) {
		    if ((*++string & 0xC0U) != 0x80U)
			break;
		}
		else if ((*string & 0xF0U) == 0xE0U) {
		    if ((*++string & 0xC0U) != 0x80U)
			break;
		    if ((*++string & 0xC0U) != 0x80U)
			break;
		}
		else if ((*string & 0xF8U) == 0xF0U) {
		    if ((*++string & 0xC0U) != 0x80U)
			break;
		    if ((*++string & 0xC0U) != 0x80U)
			break;
		    if ((*++string & 0xC0U) != 0x80U)
			break;
		}
	    }
	}

	if (*string)
	    status = -1;
    }

    return status;
}


/******************************************************************************
 * Internal Map Functions:
 ******************************************************************************/

static int
compareUnits(
    const void* const	entry1,
    const void* const	entry2)
{
    return ut_compare(((const UnitAndId*)entry1)->unit,
	((const UnitAndId*)entry2)->unit);
}


/*
 * Selects a unit-and-identifier tree corresponding to a given encoding.
 *
 * Arguments:
 *	map		The unit-to-id map.
 *	encoding	The encoding.
 * Returns:
 *	Pointer to the root of the unit-and-identifier tree in "map" that
 *	corresponds to "encoding".
 */
static void**
selectTree(
    UnitToIdMap* const	unitToIdMap,
    const ut_encoding	encoding)
{
    return
	encoding == UT_ASCII
	    ? &unitToIdMap->ascii
	    : encoding == UT_LATIN1
		? &unitToIdMap->latin1
		: &unitToIdMap->utf8;
}


/*
 * Returns a new instance of a unit-to-identifier map.
 *
 * Returns:
 *	NULL	Failure.  See "errno".
 *	else	Pointer to a new unit-to-identifier map.
 */
static UnitToIdMap*
utimNew(void)
{
    UnitToIdMap* const	map = malloc(sizeof(UnitToIdMap));

    if (map != NULL) {
	map->ascii = NULL;
	map->latin1 = NULL;
	map->utf8 = NULL;
    }

    return map;
}


/*
 * Frees a unit-to-identifier map.  All entries in all encodings are freed.
 *
 * Arguments:
 *	map		Pointer to the map to be freed.
 */
static void
utimFree(
    UnitToIdMap*	map)
{
    if (map != NULL) {
	ut_encoding	encodings[] = {UT_ASCII, UT_LATIN1, UT_UTF8};
	int		i;

	for (i = 0; i < sizeof(encodings)/sizeof(encodings[0]); ++i) {
	    void**	rootp = selectTree(map, encodings[i]);

	    while (*rootp != NULL) {
		UnitAndId*	uai = **(UnitAndId***)rootp;

		(void)tdelete(uai, rootp, compareUnits);
		uaiFree(uai);
	    }
	}

	free(map);
    }
}


/*
 * Adds an entry to a unit-to-identifier map.
 *
 * Arguments:
 *	map		Pointer to unit-to-identifier map.
 *	unit		The unit.  May be freed upon return.
 *	id		The identifier.  May be freed upon return.
 *	encoding	The ostensible encoding of "id".
 * Returns:
 *	UT_BAD_ARG	"id" is inconsistent with "encoding".
 *	UT_OS		Operating-system error.  See "errno".
 *	UT_EXISTS	"unit" already maps to a different identifier.
 *	UT_SUCCESS	Success.
 */
static ut_status
utimAdd(
    UnitToIdMap* const	map,
    const ut_unit*	unit,
    const char* const	id,
    ut_encoding		encoding)
{
    ut_status		status;

    assert(map != NULL);
    assert(unit != NULL);
    assert(id != NULL);

    if (adjustEncoding(&encoding, id)) {
	status = UT_BAD_ARG;
        ut_set_status(status);
	ut_handle_error_message("Identifier not in given encoding");
    }
    else {
	UnitAndId*	targetEntry = uaiNew(unit, id);

	if (targetEntry != NULL) {
	    void**	rootp = selectTree(map, encoding);

	    UnitAndId**	treeEntry = tsearch(targetEntry, rootp, compareUnits);

	    if (treeEntry == NULL) {
		status = UT_OS;
                ut_set_status(status);
		ut_handle_error_message(strerror(errno));
		ut_handle_error_message("Couldn't add search-tree entry");
		uaiFree(targetEntry);
	    }
	    else {
		if (strcmp((*treeEntry)->id, id) != 0) {
		    status = UT_EXISTS;
                    ut_set_status(status);
		    ut_handle_error_message("Unit already maps to \"%s\"",
			(*treeEntry)->id);
		}
		else {
		    status = UT_SUCCESS;
		}

                if (targetEntry != *treeEntry)
                    uaiFree(targetEntry);
	    }
	}				/* "targetEntry" allocated */
    }					/* valid arguments */

    return status;
}


/*
 * Removes an entry from a unit-to-identifier map.
 *
 * Arguments:
 *	map		Pointer to the unit-to-identifier map.
 *	unit		The unit.  May be freed upon return.
 *	encoding	The encoding to be removed.
 * Returns:
 *	UT_SUCCESS	Success.
 */
static ut_status
utimRemove(
    UnitToIdMap* const	map,
    const ut_unit*	unit,
    ut_encoding		encoding)
{
    ut_status		status;
    UnitAndId		targetEntry;
    UnitAndId**		treeEntry;

    assert(map != NULL);
    assert(unit != NULL);

    targetEntry.unit = (ut_unit*)unit;
    treeEntry = tfind(&targetEntry, selectTree(map, encoding), compareUnits);

    if (treeEntry == NULL || *treeEntry == NULL) {
	status = UT_SUCCESS;
    }
    else {
	UnitAndId*	uai = *treeEntry;

	(void)tdelete(uai, selectTree(map, encoding), compareUnits);
	uaiFree(uai);
    }

    return status;
}


/*
 * Returns the unit-and-identifier whose ASCII identifier corresponding to a
 * unit.
 *
 * Arguments:
 *	map	The unit-to-identifier map.
 *	unit	The unit to be used as the key in the search.
 * Returns:
 *	NULL	The map doesn't contain an entry corresponding to "unit" whose
 *		identifier is in ASCII.
 *	else	Pointer to the entry corresponding to "unit" whose identifier is
 *		in ASCII.
 */
static UnitAndId*
utimFindAsciiByUnit(
    UnitToIdMap* const	map,
    const ut_unit* const	unit)
{
    UnitAndId	targetEntry;
    UnitAndId**	treeEntry;

    targetEntry.unit = (ut_unit*)unit;
    treeEntry = tfind(&targetEntry, &map->ascii, compareUnits);

    return treeEntry == NULL ? NULL : *treeEntry;
}


/*
 * Finds a unit-search-entry with a Latin-1 identifier correcponding to a unit.
 *
 * Arguments:
 *	map	The unit-to-identifier map.
 *	unit	The unit to be used as the key in the search.
 * Returns:
 *	NULL	The map doesn't contain an entry corresponding to "unit" whose
 *		identifier is in Latin-1.
 *	else	Pointer to the entry corresponding to "unit" whose identifier is
 *		in Latin-1 (and might, actually, be in ASCII).
 */
static UnitAndId*
utimFindLatin1ByUnit(
    UnitToIdMap* const	map,
    const ut_unit* const	unit)
{
    UnitAndId	targetEntry;
    UnitAndId**	treeEntry;

    targetEntry.unit = (ut_unit*)unit;
    treeEntry = tfind(&targetEntry, &map->latin1, compareUnits);

    if (treeEntry == NULL)
	treeEntry = tfind(&targetEntry, &map->ascii, compareUnits);

    return treeEntry == NULL ? NULL : *treeEntry;
}


/*
 * Finds an entry with a UTF-8 identifier corresponding to a unit.
 *
 * Arguments:
 *	map	The unit-to-identifier map.
 *	unit	The unit to be used as the key in the search.
 * Returns:
 *	NULL	The map doesn't contain an entry corresponding to "unit" whose
 *		identifier is in UTF-8.
 *	else	Pointer to the entry corresponding to "unit" whose identifier is
 *		in UTF-8 (and might, actually, be in ASCII).
 */
static UnitAndId*
utimFindUtf8ByUnit(
    UnitToIdMap* const	        map,
    const ut_unit* const	unit)
{
    UnitAndId	targetEntry;
    UnitAndId**	treeEntry = NULL;	/* failure */

    targetEntry.unit = (ut_unit*)unit;
    treeEntry = tfind(&targetEntry, &map->utf8, compareUnits);

    if (treeEntry == NULL) {
	treeEntry = tfind(&targetEntry, &map->latin1, compareUnits);

	if (treeEntry == NULL) {
	    treeEntry = tfind(&targetEntry, &map->ascii, compareUnits);
	}
	else {
	    /*
	     * Create the UTF-8 version of the Latin-1 identifier and add it to
	     * the UTF-8 unit-to-id map so that it will be found next time.
	     */
	    char* const	id = latin1ToUtf8((*treeEntry)->id);

	    if (id == NULL) {
		ut_set_status(UT_OS);
		ut_handle_error_message(strerror(errno));
		ut_handle_error_message(
		    "Couldn't convert identifier from ISO-8859-1 to UTF-8");
		treeEntry = NULL;
	    }
	    else {
		UnitAndId*	newEntry = uaiNew(unit, id);

		if (newEntry != NULL) {
		    treeEntry = tsearch(newEntry, &map->utf8, compareUnits);

		    if (treeEntry == NULL) {
			uaiFree(newEntry);
			ut_set_status(UT_OS);
			ut_handle_error_message(strerror(errno));
			ut_handle_error_message(
                            "Couldn't add unit-and-identifier to search-tree");
		    }
		}

		free(id);
	    }				/* UTF-8 identifier created */
	}				/* found Latin-1 identifier */
    }					/* no UTF-8 identifier */

    return treeEntry == NULL ? NULL : *treeEntry;
}


/*
 * Adds an entry to the unit-to-identifier map associated with a unit-system.
 *
 * Arguments:
 *	sytemMap	Address of the pointer to the
 *			system-to-unit-to-identifier map.
 *	unit		The unit.  May be freed upon return.
 *	id		The identifier.  May be freed upon return.
 *	encoding	The ostensible encoding of "id".
 * Returns:
 *	UT_BAD_ARG	"unit" or "id" is NULL, or "id" is inconsistent with
 *                      "encoding".
 *	UT_OS		Operating-system error.  See "errno".
 *	UT_EXISTS	"unit" already maps to a different identifier.
 *	UT_SUCCESS	Success.
 */
static ut_status
mapUnitToId(
    SystemMap** const		systemMap,
    const ut_unit* const	unit,
    const char* const		id,
    ut_encoding			encoding)
{
    ut_status		status;

    assert(systemMap != NULL);

    if (unit == NULL || id == NULL) {
	status = UT_BAD_ARG;
    }
    else {
	if (*systemMap == NULL) {
	    *systemMap = smNew();

	    if (*systemMap == NULL)
		status = UT_OS;
	}

	if (*systemMap != NULL) {
	    UnitToIdMap** const	unitToIdMap =
		(UnitToIdMap**)smSearch(*systemMap, ut_get_system(unit));

	    if (unitToIdMap == NULL) {
		status = UT_OS;
	    }
	    else {
		if (*unitToIdMap == NULL) {
		    *unitToIdMap = utimNew();

		    if (*unitToIdMap == NULL)
			status = UT_OS;
		}

		if (*unitToIdMap != NULL)
		    status = utimAdd(*unitToIdMap, unit, id, encoding);
	    }
	}
    }

    return status;
}


/*
 * Removes an entry from the unit-to-identifier map associated with a
 * unit-system.
 *
 * Arguments:
 *	sytemMap	Pointer to the system-to-unit-to-identifier map.
 *	unit		The unit.  May be freed upon return.
 *	encoding	The ostensible encoding of "id".
 * Returns:
 *	UT_BAD_ARG	"systemMap" is NULL.
 *	UT_BAD_ARG	"unit" is NULL.
 *	UT_SUCCESS	Success.
 */
static ut_status
unmapUnitToId(
    SystemMap* const		systemMap,
    const ut_unit* const	unit,
    ut_encoding			encoding)
{
    ut_status		status;

    if (systemMap == NULL || unit == NULL) {
	status = UT_BAD_ARG;
    }
    else {
	UnitToIdMap** const	unitToIdMap =
	    (UnitToIdMap**)smFind(systemMap, ut_get_system(unit));

	status =
	    (unitToIdMap == NULL || *unitToIdMap == NULL)
		? UT_SUCCESS
		: utimRemove(*unitToIdMap, unit, encoding);
    }

    return status;
}


/*
 * Returns the identifier in a given encoding to which a unit associated with
 * a unit-system maps.
 *
 * Arguments:
 *	systemMap	Pointer to the system-to-unit-to-id map.
 *	unit		Pointer to the unit whose identifier should be returned.
 *	encoding	The desired encoding of the identifier.
 * Returns:
 *	NULL		Failure.  "ut_get_status()" will be
 *			    UT_BAD_ARG	"unit" was NULL.
 *	else		Pointer to the identifier in the given encoding
 *			associated with "unit".
 */
static const char*
getId(
    SystemMap* const	systemMap,
    const ut_unit* const	unit,
    const ut_encoding	encoding)
{
    const char*	id = NULL;		/* failure */

    if (unit == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("NULL unit argument");
    }
    else {
	UnitToIdMap** const	unitToId = 
	    (UnitToIdMap**)smFind(systemMap, ut_get_system(unit));

	if (unitToId != NULL) {
	    UnitAndId*	mapEntry = 
		encoding == UT_LATIN1
		    ? utimFindLatin1ByUnit(*unitToId, unit)
		    : encoding == UT_UTF8
			? utimFindUtf8ByUnit(*unitToId, unit)
			: utimFindAsciiByUnit(*unitToId, unit);

	    if (mapEntry != NULL)
		id = mapEntry->id;
	}
    }

    return id;
}


/******************************************************************************
 * Public API:
 ******************************************************************************/


/*
 * Adds a mapping from a unit to a name.
 *
 * Arguments:
 *	unit		Pointer to the unit to be mapped to "name".  May be
 *			freed upon return.
 *	name		Pointer to the name to be mapped-to by "unit".  May be
 *			freed upon return.
 *	encoding	The encoding of "name".
 * Returns:
 *	UT_SUCCESS	Success.
 *	UT_BAD_ARG	"unit" or "name" is NULL, or "name" is not in the
 *                      specified encoding.
 *	UT_OS		Operating-system error.  See "errno".
 *	UT_EXISTS	"unit" already maps to a name.
 */
ut_status
ut_map_unit_to_name(
    const ut_unit* const	unit,
    const char* const		name,
    ut_encoding			encoding)
{
    ut_set_status(mapUnitToId(&systemToUnitToName, unit, name, encoding));

    return ut_get_status();
}


/*
 * Removes a mapping from a unit to a name.
 *
 * Arguments:
 *	unit		Pointer to the unit.  May be freed upon return.
 *	encoding	The encoding to be removed.  No other encodings will be
 *			removed.
 * Returns:
 *	UT_BAD_ARG	"unit" is NULL.
 *	UT_SUCCESS	Success.
 */
ut_status
ut_unmap_unit_to_name(
    const ut_unit* const	unit,
    ut_encoding			encoding)
{
    ut_set_status(unmapUnitToId(systemToUnitToName, unit, encoding));

    return ut_get_status();
}


/*
 * Adds a mapping from a unit to a symbol.
 *
 * Arguments:
 *	unit		Pointer to the unit to be mapped to "symbol".  May be
 *			freed upon return.
 *	symbol		Pointer to the symbol to be mapped-to by "unit".  May
 *			be freed upon return.
 *	encoding	The encoding of "symbol".
 * Returns:
 *	UT_SUCCESS	Success.
 *	UT_BAD_ARG	"unit" or "symbol" is NULL.
 *	UT_OS		Operating-system error.  See "errno".
 *	UT_EXISTS	"unit" already maps to a symbol.
 */
ut_status
ut_map_unit_to_symbol(
    const ut_unit*		unit,
    const char* const		symbol,
    ut_encoding			encoding)
{
    ut_set_status(mapUnitToId(&systemToUnitToSymbol, unit, symbol, encoding));

    return ut_get_status();
}


/*
 * Removes a mapping from a unit to a symbol.
 *
 * Arguments:
 *	unit		Pointer to the unit to be unmapped to a symbol.  May be
 *			freed upon return.
 *	encoding	The encoding to be removed.  The mappings for "unit" in
 *			other encodings will not be removed.
 * Returns:
 *	UT_SUCCESS	Success.
 *	UT_BAD_ARG	"unit" is NULL.
 */
ut_status
ut_unmap_unit_to_symbol(
    const ut_unit* const	unit,
    ut_encoding			encoding)
{
    ut_set_status(unmapUnitToId(systemToUnitToSymbol, unit, encoding));

    return ut_get_status();
}


/*
 * Returns the name in a given encoding to which a unit maps.
 *
 * Arguments:
 *	unit		Pointer to the unit whose name should be returned.
 *	encoding	The desired encoding of the name.
 * Returns:
 *	NULL		Failure.  "ut_get_status()" will be
 *			    UT_BAD_ARG		"unit" is NULL.
 *			    UT_SUCCESS		"unit" doesn't map to a name in
 *						in the given encoding.
 *	else		Pointer to the name in the given encoding to which
 *			"unit" maps.
 */
const char*
ut_get_name(
    const ut_unit* const	unit,
    const ut_encoding	encoding)
{
    ut_set_status(UT_SUCCESS);

    return getId(systemToUnitToName, unit, encoding);
}


/*
 * Returns the symbol in a given encoding to which a unit maps.
 *
 * Arguments:
 *	unit		Pointer to the unit whose symbol should be returned.
 *	encoding	The desired encoding of the symbol.
 * Returns:
 *	NULL		Failure.  "ut_get_status()" will be
 *			    UT_BAD_ARG		"unit" is NULL.
 *			    UT_SUCCESS		"unit" doesn't map to a symbol
 *						in the given encoding.
 *	else		Pointer to the symbol in the given encoding to which
 *			"unit" maps.
 */
const char*
ut_get_symbol(
    const ut_unit* const	unit,
    const ut_encoding	encoding)
{
    ut_set_status(UT_SUCCESS);

    return getId(systemToUnitToSymbol, unit, encoding);
}


/*
 * Frees resources associated with a unit-system.
 *
 * Arguments:
 *	system		Pointer to the unit-system to have its associated
 *			resources freed.
 */
void
utimFreeSystem(
    ut_system*	system)
{
    if (system != NULL) {
	SystemMap*	systemMaps[2];
	int		i;

	systemMaps[0] = systemToUnitToName;
	systemMaps[1] = systemToUnitToSymbol;

	for (i = 0; i < 2; i++) {
	    if (systemMaps[i] != NULL) {
		UnitToIdMap** const	unitToId =
		    (UnitToIdMap**)smFind(systemMaps[i], system);

		if (unitToId != NULL)
		    utimFree(*unitToId);

		smRemove(systemMaps[i], system);
	    }
	}
    }
}
