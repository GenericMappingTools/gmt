/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2018 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *--------------------------------------------------------------------
 *
 * Author:	Paul Wessel
 * Date:	23-JUN-2017
 * Version:	6
 */

/*!
 * \file gmt_modern.h
 * \brief Definitions of constants used through GMT for modern mode.
 */

#ifndef _GMT_MODERN_H
#define _GMT_MODERN_H

#define GMT_HISTORY_FILE	"gmt.history"
#define GMT_SESSION_FILE	"gmt.session"

/* Session settings for default plot file prefix and format (extension) */

#define GMT_SESSION_NAME	"gmtsession"	/* Default prefix for single-figure modern sessions */
#define GMT_SESSION_FORMAT	0		/* Default entry into gmt_session_format|code arrays -> PDF */
#define GMT_SESSION_CONVERT	"A,P"		/* Default psconvert options in gmt figure */

#endif  /* _GMT_MODERN_H */
