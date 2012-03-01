/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */

/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include "udunits2.h"
#include "idToUnitMap.h"
#include "unitToIdMap.h"

extern void coreFreeSystem(ut_system* system);


/*
 * Frees a unit-system.  All unit-to-identifier and identifier-to-unit mappings
 * will be removed.
 *
 * Arguments:
 *	system		Pointer to the unit-system to be freed.  Use of "system"
 *			upon return results in undefined behavior.
 */
void
ut_free_system(
    ut_system*	system)
{
    if (system != NULL) {
	itumFreeSystem(system);
	utimFreeSystem(system);
	coreFreeSystem(system);
    }
}
