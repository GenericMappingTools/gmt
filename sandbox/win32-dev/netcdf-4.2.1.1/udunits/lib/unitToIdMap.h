/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
#ifndef UT_UNIT_TO_ID_MAP_H_INCLUDED
#define UT_UNIT_TO_ID_MAP_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Frees resources associated with a unit-system.
 *
 * Arguments:
 *	system		Pointer to the unit-system to have its associated
 *			resources freed.
 */
void
utimFreeSystem(
    ut_system*	system);


#ifdef __cplusplus
}
#endif

#endif
