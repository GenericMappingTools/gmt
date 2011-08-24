/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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

char *GMT_version ()
{
	static char text[GMT_TEXT_LEN256];
#if SVN_VERSION
	int svn_revision =
#include "gmt_svn_revision.h"
	;
	sprintf (text, PACKAGE_VERSION "-%d" GMT_VER_64 GMT_VER_COMPAT, svn_revision);
#else
	sprintf (text, PACKAGE_VERSION GMT_VER_64 GMT_VER_COMPAT);
#endif
	return (text);
}
