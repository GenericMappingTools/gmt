/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * Status of the last operation by the UDUNITS2(3) library.
 */

/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <udunits2.h>

static ut_status		_status = UT_SUCCESS;


/*
 * Returns the status of the last operation by the units module.  This function
 * will not change the status.
 */
ut_status
ut_get_status()
{
    return _status;
}


/*
 * Sets the status of the units module.  This function would not normally be
 * called by the user unless they were doing their own parsing or formatting.
 *
 * Arguments:
 *	status	The status of the units module.
 */
void
ut_set_status(
    const ut_status	status)
{
    _status = status;
}
