/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
#ifndef UT_UNIT_SEARCH_NODE_H_INCLUDED
#define UT_UNIT_SEARCH_NODE_H_INCLUDED

#include "udunits2.h"

typedef struct {
    char*	id;
    ut_unit*	unit;
} UnitAndId;

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Arguments:
 *	id	The identifier (name or symbol).  May be freed upon return.
 *	unit	The unit.  Must not be freed upon successful return until the
 *		returned unit-search-node is no longer needed.
 * Returns:
 *	NULL	"id" is NULL.
 *	NULL	"node" is NULL.
 *	NULL	Out of memory.
 *	else	Pointer to the new unit search node.
 */
UnitAndId*
uaiNew(
    const ut_unit* const	unit,
    const char* const	id);


void
uaiFree(
    UnitAndId* const	node);


#ifdef __cplusplus
}
#endif

#endif
