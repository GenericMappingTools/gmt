/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Machine-dependent macros for generation and testing of NaNs.
 *
 * These routines use the IEEE definition of Silent NaNs to set NaNs and
 * use the avialable isnan* routines to test NaNs whenever available.
 *
 * Notes:
 *    If your system has no IEEE support, add -DNO_IEEE to CFLAGS
 *    We will then use the max double/float values to signify NaNs.
 *
 * Author:	Remko Scharroo
 * Date:	1-JAN-2010
 * Ver:		5 API
 */

#ifndef _GMT_NAN_H
#define _GMT_NAN_H

#include "gmt_notposix.h"

#ifdef NO_IEEE
#	define GMT_make_fnan(x) (x = FLT_MAX)
#	define GMT_make_dnan(x) (x = DBL_MAX)
#	define GMT_is_fnan(x) ((x) == FLT_MAX)
#	define GMT_is_dnan(x) ((x) == DBL_MAX)
#else
#	define GMT_make_fnan(x) (x = (float) NAN)
#	define GMT_make_dnan(x) (x = NAN)
#	define GMT_is_fnan isnan
#	define GMT_is_dnan isnan
#endif

#endif /* _GMT_NAN_H */
