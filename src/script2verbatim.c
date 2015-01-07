/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * script2verbatim.c strips svn keywords, comments etc. from example scripts
 *
 * Author:  Florian Wobbe
 * Date:    6-JAN-2015
 * Version: 5
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 256
#define FAILURE_PREFIX "script2verbatim: "

int filter (char *line, unsigned strip_comments) {
	/* return 0 if line contains a match, 1 otherwise */

	/* filter lines with '[$]Id:' */
	if ((strstr (line, "$Id:") != 0))
		return 0;

	/* filter lines with 'functions[.]sh'
	 * if ((strstr (line, "functions.sh") != 0))
	 *   return 0; */

	/* filter lines with '^[ ]*#' */
	if (strip_comments) {
		size_t n = strspn (line, " #"); /* span ' ' and '#' */
		while (n > 0) {
			if (line[--n] == '#') /* rewind until '#' found */
				return 0;
		}
	}

	/* line without match */
	return 1;
}

int main (int argc, char *argv[]) {
	int arg = 1, line_num = 0, strip_comments = 0;
	FILE *fp_in, *fp_out;
	char line[MAX_LINE_LEN];

	if ( argc > 1 && (strcmp (argv[arg], "--strip-comments") == 0) ) {
		strip_comments = 1;
		++arg;
	}

	if (argc != arg + 2) {
		fprintf (stderr, FAILURE_PREFIX "usage: script2verbatim [--strip-comments] input output\n");
		return EXIT_FAILURE;
	}

	fp_in = fopen (argv[arg], "r");
	if (fp_in == NULL) {
		fprintf (stderr, FAILURE_PREFIX "error opening input file %s.\n", argv[arg]);
		return EXIT_FAILURE;
	}

	fp_out = fopen (argv[++arg], "w");
	if (fp_out == NULL) {
		fprintf (stderr, FAILURE_PREFIX "error opening output file %s.\n", argv[arg]);
		return EXIT_FAILURE;
	}

	/* Opening files succeeded */

	while (fgets (line, MAX_LINE_LEN, fp_in) != NULL) {
		size_t len = strlen (line);
		++line_num;
		if (len > 0 && line[len-1] != '\n') {
			fprintf (stderr, FAILURE_PREFIX "line %d too long: %s\n", line_num, line);
			return EXIT_FAILURE;
		}
		if (filter (line, strip_comments))
			fputs (line, fp_out);
	}

	/* Check EOF indicator */
	if (!feof (fp_in)) {
		fprintf (stderr, FAILURE_PREFIX "error: did not reach eof.\n");
		return EXIT_FAILURE;
	}

	fclose (fp_in);
	fclose (fp_out);
	return EXIT_SUCCESS;
}
