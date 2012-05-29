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

typedef int GMT_LONG;		/* plain signed integer */
struct GMT_CTRL;		 /* forward declaration of GMT_CTRL */

/* p_to_io_func is used as a pointer to functions such as GMT_read_d in assignments
 * and is used to declare GMT_get_io_ptr in gmt_io.c and gmt_prototypes.h */
typedef GMT_LONG (*p_to_io_func) (struct GMT_CTRL *, FILE *, unsigned, double *);

#endif  /* _GMT_TYPES_H */
