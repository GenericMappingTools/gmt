/*
 * $Id$
 *
 * Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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
 *--------------------------------------------------------------------
 */

#ifndef _RMAN_H
#define _RMAN_H

#include "gmt_config.h"

#define VOLLIST             "1:2:3:4:5:6:7:8:9:o:l:n:p"
#define MANTITLEPRINTF      "%s(%s) manual page"
#define MANREFPRINTF        "%s.%s.html"
#define POLYGLOTMANVERSION  "3.2"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wchar-subscripts"
#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wpointer-sign"
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif

#endif /* _RMAN_H */

/* vim: set ft=c: */
