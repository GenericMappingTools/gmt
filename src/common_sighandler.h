/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * common_sighandler.h contains the prototype of a signal-handling function
 *
 * Author:  Florian Wobbe
 * Date:    5-SEP-2013
 * Version: 5
 */

#pragma once
#ifndef _COMMON_SIGHANDLER_H
#define _COMMON_SIGHANDLER_H

#if !(defined WIN32 || defined NO_SIGHANDLER)
void sig_handler(int sig_num, siginfo_t *info, void *ucontext);
#endif /* !(defined WIN32 || defined NO_SIGHANDLER) */

#endif /* !_COMMON_SIGHANDLER_H */
