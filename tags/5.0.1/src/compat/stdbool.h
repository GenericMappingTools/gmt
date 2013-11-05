/*--------------------------------------------------------------------
 * $Id$
 *
 * Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 * See LICENSE.TXT file for copying and redistribution conditions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * ISO C99 stdbool.h standard header as defined by
 * http://www.opengroup.org/onlinepubs/007904975/basedefs/stdbool.h.html
 *
 * The <stdbool.h> header shall define the following macros:
 * bool                          - Expands to _Bool.
 * true                          - Expands to the integer constant 1.
 * false                         - Expands to the integer constant 0.
 * __bool_true_false_are_defined - Expands to the integer constant 1.
 *
 * An application may undefine and then possibly redefine the macros bool,
 * true, and false.
 *
 * Author:  Florian Wobbe
 * Date:    09-MAR-2012
 * Version: 5 API
 */

#ifndef _STDBOOL_H
#define _STDBOOL_H

#ifndef __bool_true_false_are_defined

#include "gmt_config.h"

#ifndef SIZEOF__BOOL
#	if defined SIZEOF_BOOL
		/* A compiler known to have 'bool'. */
		typedef bool _Bool;
#	else
#		if defined _MSC_VER || defined __GNUC__
			/* For symbolic names in gdb, define true and false as enum constants, not as macros. */
			typedef enum { _Bool_must_promote_to_int = -1, false = 0, true = 1 } _Bool;
#		else
#			define _Bool signed char
#		endif /* defined _MSC_VER || defined __GNUC__ */
#	endif /* defined __cplusplus || defined SIZEOF_BOOL */
#endif /* !SIZEOF__BOOL */

#define bool _Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1

#endif /* !__bool_true_false_are_defined */

#endif /* !_STDBOOL_H */
