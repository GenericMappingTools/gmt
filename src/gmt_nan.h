/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * Machine-dependent macros for generation and testing of NaNs.
 *
 * These routines use the IEEE definition of Silent NaNs to set NaNs and
 * use the available isnan* routines to test NaNs whenever available.
 *
 * Notes:
 *    If your system has no IEEE support, add -DNO_IEEE to CFLAGS
 *    We will then use the max double/float values to signify NaNs.
 *
 * Author:	Remko Scharroo
 * Date:	1-JAN-2010
 * Ver:		6 API
 */

/*!
 * \file gmt_nan.h
 * \brief Machine-dependent macros for generation and testing of NaNs
 */

#ifndef GMT_NAN_H
#define GMT_NAN_H

#include "gmt_notposix.h"

#ifdef NO_IEEE
#	define gmt_M_make_fnan(x) (x = FLT_MAX)
#	define gmt_M_make_dnan(x) (x = DBL_MAX)
#	define gmt_M_is_fnan(x) ((x) == FLT_MAX)
#	define gmt_M_is_dnan(x) ((x) == DBL_MAX)
#	define gmt_M_is_finf(x) ((x) == FLT_MAX)
#	define gmt_M_is_dinf(x) ((x) == DBL_MAX)
#else
#	define gmt_M_make_fnan(x) (x = (float) NAN)
#	define gmt_M_make_dnan(x) (x = NAN)
#	define gmt_M_is_fnan isnan
#	define gmt_M_is_dnan isnan
#	define gmt_M_is_finf isinf
#	define gmt_M_is_dinf isinf
#endif

#endif /* GMT_NAN_H */
