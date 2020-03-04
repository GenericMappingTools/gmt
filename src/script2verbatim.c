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
 * script2verbatim.c removes comments and replaces -ps from example scripts
 *
 * Author:  Florian Wobbe
 * Date:    6-JAN-2015
 * Version: 5
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_LINE_LEN 256
#define FAILURE_PREFIX "script2verbatim: "

/* Copy from common_string.c to avoid Windows link issues */
/*
 * strrep.c - C substring replacement.
 *
 * Written in 2011 by Drew Hess <dhess-src@bothan.net>.
 * https://gist.github.com/dhess/975639
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.
 *
 * For the full statement of the dedication, see the Creative Commons
 * CC0 Public Domain Dedication at
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

 /*
  * This file includes a main() function so that the file can be
  * compiled into an executable, in order to run some simple test cases
  * on the included strrep() function. If you want to use strrep in
  * your own project, make sure you cut or comment out the main()
  * function below.
  *
  * This function returns string s1 if string s2 is an empty string, or
  * if s2 is not found in s1. If s2 is found in s1, the function
  * returns a new null-terminated string whose contents are identical
  * to s1, except that all occurrences of s2 in the original string s1
  * are, in the new string, replaced by the string s3. The caller owns
  * the new string.
  *
  * Strings s1, s2, and s3 must all be null-terminated strings. If any
  * of s1, s2, or s3 are NULL, the function returns NULL, indicating an
  * error condition. If any other error occurs, the function returns NULL.
  *
  * This code is written pedantically, primarily so that asserts can be
  * used liberally. The code could certainly be optimized and/or made
  * less verbose, and I encourage you to do that if you use strstr in
  * your production code, once you're comfortable that it functions as
  * intended. Each assert makes plain an invariant condition that is
  * assumed to be true by the statement(s) that immediately follow the
  * assert.  Some of the asserts are trivially true, as written, but
  * they're included, nonetheless, in case you, in the process of
  * optimizing or adapting the code for your own purposes, make a
  * change that breaks an assumption made downstream by the original code.
  */

 char *gmt_strrep(const char *s1, const char *s2, const char *s3) {
	 size_t s1_len, s2_len, s3_len, count, s1_without_s2_len, newstr_len, i, substr_len, remains;
	 const char *p, *start_substr, *end_substr;
	 char *newstr, *dst;
	 if (!s1 || !s2 || !s3)
		 return 0;
	 s1_len = strlen(s1);
	 if (!s1_len)
		 return (char *)s1;
	 s2_len = strlen(s2);
	 if (!s2_len)
		 return (char *)s1;

	 /*
	  * Two-pass approach: figure out how much space to allocate for
	  * the new string, pre-allocate it, then perform replacement(s).
	  */
	 count = 0;
	 p = s1;
	 assert(s2_len); /* otherwise, strstr(s1,s2) will return s1. */
	 do {
		 p = strstr(p, s2);
		 if (p) {
			 p += s2_len;
			 count++;
		 }
	 } while (p);

	 if (!count)
		 return (char *)s1;

	 /*
	  * The following size arithmetic is extremely cautious, to guard against size_t overflows.
	  */
	 assert(s1_len >= count * s2_len);
	 assert(count);
	 s1_without_s2_len = s1_len - count * s2_len;
	 s3_len = strlen(s3);
	 newstr_len = s1_without_s2_len + count * s3_len;
	 if (s3_len && ((newstr_len <= s1_without_s2_len) || (newstr_len + 1 == 0))) /* Overflow. */
		 return 0;

	 newstr = (char *)calloc(newstr_len + 1, sizeof(char)); /* w/ terminator */
	 if (!newstr)		/* ENOMEM, but no good way to signal it. */
		 return 0;

	 dst = newstr;
	 start_substr = s1;
	 for (i = 0; i != count; ++i) {
		 end_substr = strstr(start_substr, s2);
		 assert(end_substr);
		 substr_len = end_substr - start_substr;
		 memcpy(dst, start_substr, substr_len);
		 dst += substr_len;
		 memcpy(dst, s3, s3_len);
		 dst += s3_len;
		 start_substr = end_substr + s2_len;
	 }

	 /* copy remainder of s1, including trailing '\0' */
	 remains = s1_len - (start_substr - s1) + 1;
	 assert(dst + remains == newstr + newstr_len + 1);
	 memcpy(dst, start_substr, remains);
	 assert(strlen(newstr) == newstr_len);
	 return newstr;
}

int is_comment (char *line) {
	/* return 1 if line is a comment line, 0 otherwise */
	size_t n = strspn (line, " #");  /* span ' ' and '#' */
	while (n > 0) {
		if (line[--n] == '#') /* rewind until '#' found */
			return 1;
	}
	/* not a comment line */
	return 0;
}


int main (int argc, char *argv[]) {
    	int i, nargs = 0, line_num = 0, strip_comments = 0, ps2pdf = 0;
	FILE *fp_in, *fp_out;
	char line[MAX_LINE_LEN];

	for (i = 1; i < argc; i++) {
		if (strcmp (argv[i], "--strip-comments") == 0) strip_comments = 1;
		else if (strcmp (argv[i], "--ps2pdf") == 0) ps2pdf = 1;
		else	nargs++;
	}

	if (nargs != 2) {
		fprintf (stderr, FAILURE_PREFIX "usage: script2verbatim [--strip-comments] [--ps2pdf] input output\n");
		return EXIT_FAILURE;
	}

	if ((fp_in = fopen (argv[argc-2], "r")) == NULL) {
		fprintf (stderr, FAILURE_PREFIX "error opening input file %s.\n", argv[argc-2]);
		return EXIT_FAILURE;
	}

	if ((fp_out = fopen (argv[argc-1], "w")) == NULL) {
		fprintf (stderr, FAILURE_PREFIX "error opening output file %s.\n", argv[argc-1]);
		fclose (fp_in);
		return EXIT_FAILURE;
	}

	/* Opening files succeeded */
	while (fgets (line, MAX_LINE_LEN, fp_in) != NULL) {
		size_t len = strlen (line);
		++line_num;
		if (len > MAX_LINE_LEN) {
			fprintf (stderr, FAILURE_PREFIX "line %d too long: %s\n", line_num, line);
			fclose (fp_in);
			fclose (fp_out);
			return EXIT_FAILURE;
		}
		if (strip_comments && is_comment(line)) continue;
		if (ps2pdf)
			fputs (gmt_strrep(line, " -ps ", " -pdf "), fp_out);
		else
			fputs (line, fp_out);
	}

	/* Check EOF indicator */
	if (!feof (fp_in)) {
		fprintf (stderr, FAILURE_PREFIX "error: did not reach eof.\n");
		fclose (fp_in);
		fclose (fp_out);
		return EXIT_FAILURE;
	}

	fclose (fp_in);
	fclose (fp_out);
	return EXIT_SUCCESS;
}
