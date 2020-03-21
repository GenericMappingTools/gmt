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
 *--------------------------------------------------------------------*/
/*
 * common_sighandler.h contains the prototype of a signal-handling function
 *
 * Author:  Florian Wobbe
 * Date:    5-SEP-2013
 * Version: 5
 */

/*!
 * \file common_sighandler.h
 * \brief Prototype of a signal-handling function
 */

#pragma once
#ifndef COMMON_SIGHANDLER_H
#define COMMON_SIGHANDLER_H

#if !(defined WIN32 || defined NO_SIGHANDLER)
void sig_handler(int sig_num, siginfo_t *info, void *ucontext);
#endif /* !(defined WIN32 || defined NO_SIGHANDLER) */

#endif /* !COMMON_SIGHANDLER_H */
