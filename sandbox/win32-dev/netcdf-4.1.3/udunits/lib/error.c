/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * This module is thread-compatible but not thread-safe.  Multi-threaded
 * access must be externally synchronized.
 */

/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <stdarg.h>
#include <stdio.h>

#include "udunits2.h"


/*
 * Writes an error-message to the standard-error stream when received and
 * appends a newline.  This is the initial error-message handler.
 *
 * Arguments:
 *	fmt	The format for the error-message.
 *	args	The arguments of "fmt".
 * Returns:
 *	<0	A output error was encountered.  See "errno".
 *	else	The number of bytes of "fmt" and "arg" written excluding any
 *		terminating NUL.
 */
int
ut_write_to_stderr(
    const char* const	fmt,
    va_list		args)
{
    int	nbytes = vfprintf(stderr, fmt, args);

    (void)fputc('\n', stderr);

    return nbytes;
}


/*
 * Does nothing with an error-message.
 *
 * Arguments:
 *	fmt	The format for the error-message.
 *	args	The arguments of "fmt".
 * Returns:
 *	0	Always.
 */
int
ut_ignore(
    const char* const	fmt,
    va_list		args)
{
    return 0;
}


static ut_error_message_handler	errorMessageHandler = ut_write_to_stderr;


/*
 * Returns the previously-installed error-message handler and optionally
 * installs a new handler.  The initial handler is "ut_write_to_stderr()".
 *
 * Arguments:
 *      handler		NULL or pointer to the error-message handler.  If NULL,
 *			then the handler is not changed.  The 
 *			currently-installed handler can be obtained this way.
 * Returns:
 *	Pointer to the previously-installed error-message handler.
 */
ut_error_message_handler
ut_set_error_message_handler(
    ut_error_message_handler	handler)
{
    ut_error_message_handler	prev = errorMessageHandler;

    if (handler != NULL)
	errorMessageHandler = handler;

    return prev;
}


/*
 * Handles an error-message.
 *
 * Arguments:
 *	fmt	The format for the error-message.
 *	...	The arguments for "fmt".
 * Returns:
 *	<0	An output error was encountered.
 *	else	The number of bytes of "fmt" and "arg" written excluding any
 *		terminating NUL.
 */
int
ut_handle_error_message(
    const char* const	fmt,
    ...)
{
    int			nbytes;
    va_list		args;

    va_start(args, fmt);

    nbytes = errorMessageHandler(fmt, args);

    va_end(args);

    return nbytes;
}
