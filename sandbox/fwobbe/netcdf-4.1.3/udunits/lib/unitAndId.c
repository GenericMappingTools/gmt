/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * Searchable unit-and-identifier tree.
 */

/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "unitAndId.h"
#include "udunits2.h"


/*
 * Arguments:
 *	unit	The unit.  May be freed upon return.
 *	id	The identifier (name or symbol).  May be freed upon return.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be
 *		    UT_BAD_ARG	"unit" or "id" is NULL.
 *		    UT_OS	Operating-system failure.  See "errno".
 *	else	Pointer to the new unit-and-identifier.
 */
UnitAndId*
uaiNew(
    const ut_unit* const	unit,
    const char* const	        id)
{
    UnitAndId*	entry = NULL;		/* failure */

    if (id == NULL || unit == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("uaiNew(): NULL argument");
    }
    else {
	entry = malloc(sizeof(UnitAndId));

	if (entry == NULL) {
	    ut_set_status(UT_OS);
	    ut_handle_error_message(strerror(errno));
	    ut_handle_error_message("Couldn't allocate %lu-byte data-structure",
		sizeof(UnitAndId));
	}
	else {
	    entry->id = strdup(id);

	    if (entry->id == NULL) {
		ut_set_status(UT_OS);
		ut_handle_error_message(strerror(errno));
		ut_handle_error_message("Couldn't duplicate identifier");
	    }
	    else {
		entry->unit = ut_clone(unit);

		if (entry->unit == NULL) {
		    assert(ut_get_status() != UT_SUCCESS);
		    free(entry->id);
		}
	    }

	    if (ut_get_status() != UT_SUCCESS) {
		free(entry);
		entry = NULL;
	    }
	}
    }

    return entry;
}


/*
 * Frees memory of a unit-and-identifier.
 *
 * Arguments:
 *	entry	Pointer to the unit-and-identifier or NULL.
 */
void
uaiFree(
    UnitAndId* const	entry)
{
    if (entry != NULL) {
	free(entry->id);
	ut_free(entry->unit);
	free(entry);
    }
}
