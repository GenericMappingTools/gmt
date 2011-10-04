/*--------------------------------------------------------------------
 *
 *  $Id$
 *
 *  Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo,
 *  F. Wobbe, and J. Luis
 *  See LICENSE.TXT file for copying and redistribution conditions.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 or any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------
 */

#ifndef _DECLSPEC_H_
#	define _DECLSPEC_H_

/*
 * When an application links to a DLL in Windows, the symbols that
 * are imported have to be identified as such.
 */

#	ifdef _WIN32
#		ifdef LIBRARY_EXPORTS
#			define LIBSPEC __declspec(dllexport)
#		else
#			define LIBSPEC __declspec(dllimport)
#		endif /* ifdef LIBRARY_EXPORTS */
#	else /* ifdef _WIN32 */
#		define LIBSPEC
#	endif /* ifdef _WIN32 */

/* By default, we use the standard "extern" declarations. */
#		define EXTERN_MSC extern LIBSPEC

#endif /* _DECLSPEC_H_ */
