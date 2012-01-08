/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Produce version number for GMT
 *
 * Author:	Remko Scharroo
 * Date:	24-AUG-2011
 * Version:	5
 */

#include "gmt.h"

#if HAVE_SVN_VERSION
#define SVN_SUFFIX "_r" SVN_VERSION_STRING
#else
#define SVN_SUFFIX ""
#endif

char *GMT_version ()
{
	static char text[GMT_TEXT_LEN256];
	sprintf (text, PACKAGE_VERSION SVN_SUFFIX GMT_VER_64 GMT_VER_COMPAT);
	return (text);
}
