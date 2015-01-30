/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

/*!
 * \file gmt_glib.h
 * \brief Include file for items using glib.
 */

/* Authors:	J. Luis
   Date:	1-OCT-2012
   Version:	5 API

*/

/* Include glib header and define mutex calls that are no-op when not linking against glib
   These are used only GLIB based multi-threading */

#ifndef _GMT_GLIB_H

#ifdef HAVE_GLIB_GTHREAD
#include <glib.h>

#define GMT_declare_gmutex static GMutex mutex;
#define GMT_set_gmutex g_mutex_lock (&mutex);
#define GMT_unset_gmutex g_mutex_unlock (&mutex);

#else

#define GMT_declare_gmutex
#define GMT_set_gmutex
#define GMT_unset_gmutex

#endif

#endif /* HAVE_GLIB_GTHREAD */
