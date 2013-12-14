/*--------------------------------------------------------------------
 *	$Id: gmt_compat.h 12478 2013-11-08 19:55:37Z pwessel $
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Provides compatibility with module names no longer used.
 *
 * Author:	Paul Wessel
 * Date:	24-JUN-2013
 * Version:	5.x
 */

#ifndef _GMT_COMPAT_H
#define _GMT_COMPAT_H

EXTERN_MSC int GMT_gmtdp (void *V_API, int mode, void *args);
EXTERN_MSC int GMT_minmax (void *V_API, int mode, void *args);
EXTERN_MSC int GMT_gmtstitch (void *V_API, int mode, void *args);

#endif  /* !_GMT_COMPAT_H */
