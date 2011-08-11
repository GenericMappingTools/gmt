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
 *  Extended regular expression matching function.
 *
 * Author:	Florian Wobbe
 * Date:	11-AUG-2010
 * Version:	5.x
 *
 * PUBLIC functions:
 *
 * gmt_regexp_match:	Match a string against an extended regular expression
 *
 */

#include "gmt.h"
#include "gmt_internals.h"

/*
 * ERE pattern matching with PCRE or POSIX
 */
#ifdef HAVE_PCRE
#include <pcre.h>
#define OVECCOUNT 30    /* should be a multiple of 3 */
#else
#include <regex.h>
#endif

GMT_LONG gmt_regexp_match (struct GMT_CTRL *C, const char *subject, const char *pattern)
{
/* Match string against the extended regular expression in pattern. Return 1 for match, 0 for no match. */

#ifdef HAVE_PCRE

	/* Use PCRE for matching
	 * Based on PCRE DEMONSTRATION PROGRAM pcredemo.c
	 */
	pcre *re;
	const char *error;
	int erroffset;
	int ovector[OVECCOUNT];
	int rc;
	
	/*************************************************************************
	 * Now we are going to compile the regular expression pattern, and handle *
	 * and errors that are detected.                                          *
	 *************************************************************************/
	
	re = pcre_compile(
	                  pattern,              /* the pattern */
	                  0,                    /* default options */
	                  &error,               /* for error message */
	                  &erroffset,           /* for error offset */
	                  NULL);                /* use default character tables */
	
	/* Compilation failed: print the error message and exit */
	
	if (re == NULL)
	  GMT_report (C, GMT_MSG_FATAL, "gmt_regexp_match: PCRE compilation failed at offset %d: %s.\n", erroffset, error);
	
	/*************************************************************************
	 * If the compilation succeeded, we call PCRE again, in order to do a     *
	 * pattern match against the subject string. This does just ONE match. If *
	 * further matching is needed, it will be done below.                     *
	 *************************************************************************/
	
	rc = pcre_exec(
	               re,                   /* the compiled pattern */
	               NULL,                 /* no extra data - we didn't study the pattern */
	               subject,              /* the subject string */
	               (int)strlen(subject), /* the length of the subject */
	               0,                    /* start at offset 0 in the subject */
	               0,                    /* default options */
	               ovector,              /* output vector for substring information */
	               OVECCOUNT);           /* number of elements in the output vector */
	
	/* Matching failed: handle error cases */
	
	if (rc < 0) {
		switch(rc) {
			case PCRE_ERROR_NOMATCH: break;
			/* Handle other special cases if you like */
			default: GMT_report (C, GMT_MSG_FATAL, "gmt_regexp_match: PCRE matching error %d.\n", rc); break;
		}
		pcre_free(re);     /* Release memory used for the compiled pattern */
		return (0);	/* Match failed */
	}
	
	return (1); /* Match succeded */

#else

	/* Based on the regcomp documentation */
	int    status;
	regex_t    re;
	
	if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0)
		GMT_report (C, GMT_MSG_FATAL, "gmt_regexp_match: POSIX ERE compilation failed.\n"); /* Report error. */
	status = regexec(&re, subject, (size_t) 0, NULL, 0);
	regfree(&re);
	if (status != 0) return (0); /* No match */
	return (1); /* Match succeded */

#endif
}
