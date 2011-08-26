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
#define OVECCOUNT 30        /* should be a multiple of 3 */
#elif defined HAVE_POSIX_ERE
#include <regex.h>
#define MAX_ERR_LENGTH 80   /* max error message length */
#endif

GMT_LONG gmt_regexp_match (struct GMT_CTRL *C, const char *subject, const char *pattern, GMT_LONG caseless)
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
	int options = 0; /* default options */

	/*************************************************************************
	 * Now we are going to compile the regular expression pattern, and handle *
	 * and errors that are detected.                                          *
	 *************************************************************************/

	if ( caseless )
		options = options|PCRE_CASELESS;      /* caseless matching */

	re = pcre_compile(
			pattern,              /* the pattern */
			options,              /* options */
			&error,               /* for error message */
			&erroffset,           /* for error offset */
			NULL);                /* use default character tables */
	
	/* Compilation failed: print the error message and exit */

	if (re == NULL) {
		GMT_report (C, GMT_MSG_FATAL, "gmt_regexp_match: PCRE compilation failed at offset %d: %s.\n", erroffset, error);
		GMT_exit (EXIT_FAILURE);
	}
	
	/*************************************************************************
 	* If the compilation succeeded, we call PCRE again, in order to do a     *
 	* pattern match against the subject string. This does just ONE match. If *
 	* further matching is needed, it will be done below.                     *
 	*************************************************************************/
	
	rc = pcre_exec( re,                   /* the compiled pattern */
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
			default: 
				 GMT_report (C, GMT_MSG_FATAL, "gmt_regexp_match: PCRE matching error %d.\n", rc);
				 GMT_exit (EXIT_FAILURE);
				 break;
		}
		pcre_free(re);	/* Release memory used for the compiled pattern */
		return (0);	/* Match failed */
	}
	
	return (1); /* Match succeded */

#elif defined HAVE_POSIX_ERE

	/* Use POSIX ERE for matching
	 * Based on the regcomp documentation */
	regex_t re;
	int cflags = REG_EXTENDED|REG_NOSUB;
	int status;
	char err_msg[MAX_ERR_LENGTH];

	if ( caseless )
		cflags = cflags|REG_ICASE; /* caseless matching */

	/* compile the RE */
	if ( (status = regcomp(&re, pattern, cflags)) != 0) {
		regerror(status, &re, err_msg, MAX_ERR_LENGTH);
		GMT_report (C, GMT_MSG_FATAL, "gmt_regexp_match: POSIX ERE compilation failed: %s\n", err_msg);
		GMT_exit (EXIT_FAILURE);
	}

	/* execute the RE against the subject string */
	status = regexec(&re, subject, (size_t) 0, NULL, 0);
	regfree(&re);     /* Release memory used for the compiled pattern */
	if ( status == 0 )
		return (1); /* Match succeded */
	else if ( status != REG_NOMATCH ) {
		/* this is when errors have been encountered */
		regerror(status, &re, err_msg, MAX_ERR_LENGTH);
		GMT_report (C, GMT_MSG_FATAL, "gmt_regexp_match: POSIX ERE matching error: %s\n", err_msg); /* Report error. */
		GMT_exit (EXIT_FAILURE);
	}
	return (0); /* No match */

#else

	/* disable ERE support */
	GMT_report (C, GMT_MSG_FATAL, "gmt_regexp_match: this GMT version was compiled without regular expression support.\n");
	GMT_exit (EXIT_FAILURE);

#endif
}
