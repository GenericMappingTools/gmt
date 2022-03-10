/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *--------------------------------------------------------------------*/
/*
 * gmt_common_sighandler.h contains the prototype of a signal-handling function
 *
 * Author:  Florian Wobbe
 * Date:    5-SEP-2013
 * Version: 5
 */

/*!
 * \file gmt_common_sighandler.h
 * \brief Prototype of a signal-handling function
 */

#pragma once
#ifndef GMT_COMMON_SIGHANDLER_H
#define GMT_COMMON_SIGHANDLER_H

#ifndef NO_SIGHANDLER
#   ifdef WIN32
#       include <windows.h>
        EXTERN_MSC BOOL sig_handler_win32 (DWORD dwType);
#   else
#       include <signal.h>
        EXTERN_MSC void sig_handler_unix (int sig_num, siginfo_t *info, void *ucontext);
#   endif
#endif /* !defined(NO_SIGHANDLER) */

#endif /* !GMT_COMMON_SIGHANDLER_H */
