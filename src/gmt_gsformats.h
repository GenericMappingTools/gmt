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
 *--------------------------------------------------------------------
 *
 * Author:	Paul Wessel
 * Date:	26-JUL-2017
 * Version:	6
 */

/*!
 * \file gmt_gsformats.h
 * \brief List of gs formats for modern mode.
 */

#ifndef GMT_GSFORMATS_H
#define GMT_GSFORMATS_H

/* List ps at end since it causes a renaming of ps- to ps only.  Also allow alternative jpeg and tiff spellings */
static char *gmt_session_format[] = {"pdf", "jpg", "jpeg", "png", "PNG", "ppm", "tif", "tiff", "bmp", "eps", "ps", NULL};
static char gmt_session_code[] =    { 'f',   'j',    'j',   'g',   'G',   'm',   't',    't',   'b',   'e',  'p',    0};

#endif  /* GMT_GSFORMATS_H */
