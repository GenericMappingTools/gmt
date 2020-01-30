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
 *  Extended regular expression matching function.
 *
 * Author:	Florian Wobbe
 * Date:	11-AUG-2010
 * Version:	5.x
 *
 * PUBLIC functions:
 *
 * gmtlib_regexp_match:	Match a string against an extended regular expression
 *
 */

#include "gmt_dev.h"
#include "gmt_internals.h"

/*
 * ERE pattern matching with PCRE2, PCRE or POSIX
 */
#ifdef HAVE_PCRE
#include <pcre.h>
#define OVECCOUNT 30        /* should be a multiple of 3 */
#elif defined HAVE_PCRE2
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#elif defined HAVE_POSIX_ERE
#include <regex.h>
#define MAX_ERR_LENGTH 80   /* max error message length */
#endif

int gmtlib_regexp_match (struct GMT_CTRL *GMT, const char *subject, const char *pattern, bool caseless) {
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
	 * any errors that are detected.                                          *
	 *************************************************************************/

	if (caseless) options = options|PCRE_CASELESS;      /* caseless matching */

	re = pcre_compile(
			pattern,              /* the pattern */
			options,              /* options */
			&error,               /* for error message */
			&erroffset,           /* for error offset */
			NULL);                /* use default character tables */

	/* Compilation failed: print the error message and exit */

	if (re == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtlib_regexp_match: PCRE compilation failed at offset %d: %s.\n", erroffset, error);
		GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
	}

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

	pcre_free(re);	/* Release memory used for the compiled pattern */
	if (rc < 0) {
		switch(rc) {
			case PCRE_ERROR_NOMATCH: break;
			/* Handle other special cases if you like */
			default:
				 GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtlib_regexp_match: PCRE matching error %d.\n", rc);
				 GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
				 break;
		}
		return (0);	/* Match failed */
	}

	return (1); /* Match succeeded */

#elif defined HAVE_PCRE2

	/* Use PCRE2 for matching
	 * Based on PCRE2 DEMONSTRATION PROGRAM pcre2demo.c
	 */
	pcre2_code *re;
	PCRE2_SIZE erroffset;
	pcre2_match_data *match_data;
	int errornumber;
	int rc;
	int options = 0; /* default options */

	/*************************************************************************
	 * Now we are going to compile the regular expression pattern, and handle *
	 * any errors that are detected.                                          *
	 *************************************************************************/

	if (caseless) options = options|PCRE2_CASELESS;      /* caseless matching */

	re = pcre2_compile(
			(PCRE2_SPTR) pattern, /* the pattern */
			PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
			options,              /* options */
			&errornumber,         /* for error number */
			&erroffset,           /* for error offset */
			NULL);                /* use default compile context */

	/* Compilation failed: print the error message and exit */

	if (re == NULL) {
		PCRE2_UCHAR error[256];
		pcre2_get_error_message(errornumber, error, sizeof(error));
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtlib_regexp_match: PCRE2 compilation failed at offset %d: %s.\n", erroffset, error);
		GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
	}

	/*************************************************************************
 	* If the compilation succeeded, we call PCRE again, in order to do a     *
 	* pattern match against the subject string. This does just ONE match. If *
 	* further matching is needed, it will be done below. Before running the  *
    * match we must set up a match_data block for holding the result.        *
 	*************************************************************************/

	/* Using this function ensures that the block is exactly the right size for
	the number of capturing parentheses in the pattern. */

	match_data = pcre2_match_data_create_from_pattern(re, NULL);

	rc = pcre2_match(
			re,                   /* the compiled pattern */
			(PCRE2_SPTR) subject, /* the subject string */
			(int)strlen(subject), /* the length of the subject */
			0,                    /* start at offset 0 in the subject */
			0,                    /* default options */
			match_data,           /* block for storing the result */
			NULL);                /* use default matching context */

	/* Matching failed: handle error cases */

	pcre2_code_free(re);                /* Release memory used for the compiled pattern */
	pcre2_match_data_free(match_data);  /* release memory for the match data */
	if (rc < 0) {
		switch(rc) {
			case PCRE2_ERROR_NOMATCH: break;
			/* Handle other special cases if you like */
			default:
				 GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtlib_regexp_match: PCRE2 matching error %d.\n", rc);
				 GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
				 break;
		}
		return (0);	/* Match failed */
	}

	return (1); /* Match succeeded */

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
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtlib_regexp_match: POSIX ERE compilation failed: %s\n", err_msg);
		GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
	}

	/* execute the RE against the subject string */
	status = regexec(&re, subject, 0U, NULL, 0);
	regfree(&re);     /* Release memory used for the compiled pattern */
	if ( status == 0 )
		return (1); /* Match succeeded */
	else if ( status != REG_NOMATCH ) {
		/* this is when errors have been encountered */
		regerror(status, &re, err_msg, MAX_ERR_LENGTH);
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtlib_regexp_match: POSIX ERE matching error: %s\n", err_msg); /* Report error. */
		GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
	}
	return (0); /* No match */

#else

	/* disable ERE support */
	GMT_Report (GMT->parent, GMT_MSG_ERROR, "gmtlib_regexp_match: this GMT version was compiled without regular expression support.\n");
	GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;

#endif
}
