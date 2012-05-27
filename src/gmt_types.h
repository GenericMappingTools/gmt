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
 * gmt_types.h contains definitions of special types used by GMT.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	5 API
 */

#ifndef _GMT_TYPES_H
#define _GMT_TYPES_H
#ifdef HAVE_STDBOOL_H_
#	include <stdbool.h>
#else
#	include "compat/stdbool.h"
#endif
#include <stdint.h>
/* Two types for counting, depending on expected range of unsigned integers we need */
#define COUNTER_LARGE	uint64_t
#define COUNTER_MEDIUM	unsigned
/* Type for TRUE [!0] or FALSE [0] only */
#define GMT_BOOLEAN	bool

/*--------------------------------------------------------------------
 *			GMT TYPEDEF DEFINITIONS
 *--------------------------------------------------------------------*/

typedef int GMT_LONG;			/* plain signed integer */

typedef void (*p_func_v) ();		/* p_func_v declares a pointer to a function returning void */
typedef void* (*p_func_p) ();		/* p_func_p declares a pointer to a function returning void pointer */
typedef GMT_LONG (*p_func_l) ();	/* p_func_l declares a pointer to a function returning an GMT_LONG */
typedef uint64_t (*p_func_u8) ();	/* p_func_u8 declares a pointer to a function returning an COUNTER_LARGE */
typedef unsigned (*p_func_u4) ();	/* p_func_u4 declares a pointer to a function returning an unsigned int */
typedef size_t (*p_func_z) ();		/* p_func_z declares a pointer to a function returning a size_t */
typedef double (*p_func_d) ();		/* p_func_d declares a pointer to a function returning a double */
typedef GMT_BOOLEAN (*p_func_b) ();	/* p_func_b declares a pointer to a function returning a GMT_BOOLEAN */

#endif  /* _GMT_TYPES_H */
