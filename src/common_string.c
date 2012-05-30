/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * common_string.c contains code shared between GMT and PSL
 *
 * Author:  Florian Wobbe
 * Date:    3-MAR-2012
 * Version: 5
 *
 * Modules in this file:
 *
 *  GMT_chop                Chops off any CR or LF at end of string
 *  GMT_chop_ext            Chops off the trailing .xxx (file extension)
 *  GMT_strstrip            Strip leading and trailing whitespace from string
 *  GMT_cr2lf               Replace CR with LF and terminate string
 *  GMT_strlshift           Left shift a string by n characters
 *  GMT_strrepc             Replaces all occurrences of a char in the string
 *  GMT_strlcmp             Compares strings (ignoring case) until first reaches null character
 *  GMT_strtok              Reiterant replacement of strtok
 *  DOS_path_fix            Turn /c/dir/... paths into c:/dir/...
 *  strtok_r                Reentrant string tokenizer from Gnulib (LGPL)
 *  strsep                  Reentrant string tokenizer that handles empty fields
 *  strsepz                 Like strsep but ignores empty fields
 *  stresep                 Like strsep but takes an additional argument esc in order
 *                          to ignore escaped chars (from NetBSD)
 *  match_string_in_file    Return true if a string is found in file
 */

/* CMake definitions: This must be first! */
#include "gmt_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <limits.h>

#include "gmt_notposix.h"
#include "gmt_types.h"
#include "common_string.h"

#define BUF_SIZE 4096

char *GMT_chop_ext (char *string) {
	/* Chops off the filename extension (e.g., .ps) in the string by replacing the last
	 * '.' with '\0' and returns a pointer to the extension or NULL if not found. */
	char *p;
	assert (string != NULL); /* NULL pointer */
	if ((p = strrchr(string, '.'))) {
		*p = '\0';
		return (p + 1);
	}
	return (NULL);
}

void GMT_chop (char *string) {
	/* Chops off any CR or LF and terminates string */
	char *p;
	assert (string != NULL); /* NULL pointer */
  /* if (string == NULL) return; / NULL pointer */
	if ((p = strpbrk (string, "\r\n")))
		/* Overwrite 1st CR or LF with terminate string */
		*p = '\0';
}

void GMT_strstrip(char *string, int strip_leading) {
	/* Strip leading and trailing whitespace from string */
	char *start = string;
	char *end;

	assert (string != NULL); /* NULL pointer */

	if (strip_leading) {
		/* Skip over leading whitespace */
		while ((*start) && isspace(*start))
			++start;
		/* Is string just whitespace? */
		if (!(*start)) {
			*string = '\0'; /* Truncate entire string */
			return;
		}
	}

	/* Find end of string */
	end = start;
	while (*end)
		++end;

	/* Step backward until first non-whitespace */
	while ((--end != start) && isspace(*end));

	/* Chop off trailing whitespace */
	*(end + 1) = '\0';

	/* If leading whitespace, then move entire string back */
	if (string != start)
		memmove(string, start, end-start+2);
}

void GMT_cr2lf (char *string) {
	/* Replace CR with LF and terminate string */
	char *p;
	assert (string != NULL); /* NULL pointer */
	if ((p = strchr (string, '\r')))
		/* Overwrite 1st CR with LF + \0 */
		strcpy(p, "\n");
}

void GMT_strlshift (char *string, size_t n) {
	/* Left shift a string by n characters */
	size_t len;
	assert (string != NULL); /* NULL pointer */

	if ((len = strlen(string)) <= n ) {
		/* String shorter than shift width */
		*string = '\0'; /* Truncate entire string */
		return;
	}

	/* Move entire string back */
	memmove(string, string + n, len + 1);
}

void GMT_strrepc (char *string, int c, int r) {
	/* Replaces all occurrences of c in the string with r */
	assert (string != NULL); /* NULL pointer */
	do {
		if (*string == c)
			*string = r;
	} while (*(++string)); /* repeat until \0 reached */
}

size_t GMT_strlcmp (char *str1, char *str2)
{
	/* Compares str1 with str2 but only until str1 reaches the
	 * null-terminator character while case is ignored.
	 * When the strings match until that point, the routine returns the
	 * length of str1, otherwise it returns 0.
	 */
	size_t i = 0;
	while (str1[i] && tolower((unsigned char) str1[i]) == tolower((unsigned char) str2[i])) ++i;
	if (str1[i]) return 0;
	return i;
}

unsigned int GMT_strtok (const char *string, const char *sep, unsigned int *pos, char *token)
{
	/* Reentrant replacement for strtok that uses no static variables.
	 * Breaks string into tokens separated by one of more separator
	 * characters (in sep).  Set *pos to 0 before first call.  Unlike
	 * strtok, always pass the original string as first argument.
	 * Returns 1 if it finds a token and 0 if no more tokens left.
	 * pos is updated and token is returned.  char *token must point
	 * to memory of length >= strlen (string).
	 * string is not changed by GMT_strtok.
	 */

	size_t i, j, string_len;

	string_len = strlen (string);

	/* Wind up *pos to first non-separating character: */
	while (string[*pos] && strchr (sep, (int)string[*pos])) (*pos)++;

	token[0] = 0;	/* Initialize token to NULL in case we are at end */

	if (*pos >= string_len || string_len == 0) return 0;	/* Got NULL string or no more string left to search */

	/* Search for next non-separating character */
	i = *pos; j = 0;
	while (string[i] && !strchr (sep, (int)string[i])) token[j++] = string[i++];
	token[j] = 0;	/* Add terminating \0 */

	/* Wind up *pos to next non-separating character */
	while (string[i] && strchr (sep, (int)string[i])) i++;
	*pos = i;

	return 1;
}

#ifdef WIN32
/* Turn '/c/dir/...' paths into 'c:/dir/...'
 * Must do it in a loop since dir may be several ';'-separated dirs */
void DOS_path_fix (char *dir)
{
	size_t n, k;

	if (!dir || (n = strlen (dir)) < 2U)
		/* Given NULL or too short dir to work */
		return;

	if (!strncmp (dir, "/cygdrive/", 10U))
		/* May happen for example when Cygwin sets GMT_SHAREDIR */
		GMT_strlshift (dir, 9); /* Chop '/cygdrive' */

	/* Replace dumb backslashes with slashes */
	GMT_strrepc (dir, '\\', '/');

	/* If dir begins with '/' and is 2 long, as in '/c', replace with 'c:' */
	if (n == 2U && dir[0] == '/') {
		dir[0] = dir[1];
		dir[1] = ':';
		return;
	}

	/* If dir is longer than 2 and, e.g., '/c/', replace with 'c:/' */
	if (n > 2U && dir[0] == '/' && dir[2] == '/' && isalpha ((int)dir[1])) {
		dir[0] = dir[1];
		dir[1] = ':';
	}

	/* Do same with dirs separated by ';' but do not replace '/c/j/...' with 'c:j:/...' */
	for (k = 4; k < n-2; ++k) {
		if ( (dir[k-1] == ';' && dir[k] == '/' && dir[k+2] == '/' && isalpha ((int)dir[k+1])) ) {
			dir[k] = dir[k+1];
			dir[k+1] = ':';
		}
	}
}
#endif

#if !defined(HAVE_STRTOK_R) && !defined(HAVE_STRTOK_S)
/* Reentrant string tokenizer.  Generic version.
   Copyright (C) 1991, 1996-1999, 2001, 2004, 2007, 2009-2012 Free Software
   Foundation, Inc.
   This file is part of the GNU C Library.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Parse S into tokens separated by characters in DELIM.
   If S is NULL, the saved pointer in SAVE_PTR is used as
   the next starting point.  For example:
        char s[] = "-abc-=-def";
        char *sp;
        x = strtok_r(s, "-", &sp);      // x = "abc", sp = "=-def"
        x = strtok_r(NULL, "-=", &sp);  // x = "def", sp = NULL
        x = strtok_r(NULL, "=", &sp);   // x = NULL
                                        // s = "abc\0-def\0"
*/
char *
strtok_r (char *s, const char *delim, char **save_ptr)
{
  char *token;

  if (s == NULL)
    s = *save_ptr;

  /* Scan leading delimiters.  */
  s += strspn (s, delim);
  if (*s == '\0')
    {
      *save_ptr = s;
      return NULL;
    }

  /* Find the end of the token.  */
  token = s;
  s = strpbrk (token, delim);
  if (s == NULL)
    /* This token finishes the string.  */
    *save_ptr = strchr (token, '\0');
  else
    {
      /* Terminate the token and make *SAVE_PTR point past it.  */
      *s = '\0';
      *save_ptr = s + 1;
    }
  return token;
}
#endif /* !defined(HAVE_STRTOK_R) && !defined(HAVE_STRTOK_S) */

#ifndef HAVE_STRSEP
/* Copyright (C) 2004, 2007, 2009-2012 Free Software Foundation, Inc.

   Written by Yoann Vandoorselaere <yoann@prelude-ids.org>.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

char *
strsep (char **stringp, const char *delim)
{
  char *start = *stringp;
  char *ptr;

  if (start == NULL)
    return NULL;

  /* Optimize the case of no delimiters.  */
  if (delim[0] == '\0')
    {
      *stringp = NULL;
      return start;
    }

  /* Optimize the case of one delimiter.  */
  if (delim[1] == '\0')
    ptr = strchr (start, delim[0]);
  else
    /* The general case.  */
    ptr = strpbrk (start, delim);
  if (ptr == NULL)
    {
      *stringp = NULL;
      return start;
    }

  *ptr = '\0';
  *stringp = ptr + 1;

  return start;
}
#endif /* ifndef HAVE_STRSEP */

/* Like strsep but ignores empty fields */
char *strsepz (char **stringp, const char *delim) {
	char *c;
	while ( (c = strsep(stringp, delim)) != NULL && *c == '\0' );
	return c;
}

/* $NetBSD: stresep.c,v 1.2 2007/12/06 22:07:07 seb Exp $
 *
 * Copyright (c) 1990, 1993
 * The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Get next token from string *stringp, where tokens are possibly-empty
 * strings separated by characters from delim. If esc is not NUL, then
 * the characters followed by esc are ignored and are not taken into account
 * when splitting the string.
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, stresep returns NULL.
 */

char *
stresep(char **stringp, const char *delim, int esc)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;

	assert(delim != NULL);

	if ((s = *stringp) == NULL)
		return NULL;
	for (tok = s;;) {
		c = *s++;
		while (esc != '\0' && c == esc) {
			(void)strcpy(s - 1, s);
			c = *s++;
		}
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return tok;
			}
		} while (sc != 0);
	}
}

/* Return true if a string is found in file */
int match_string_in_file (const char *filename, const char *string) {
	FILE *fp;
	char line[BUF_SIZE+1];

	fp = fopen (filename, "r");
	if ( fp == NULL )
		return false;

	/* make sure string is always \0-terminated */
	line[BUF_SIZE] = '\0';

	/* search for string in each line */
	while ( fgets (line, BUF_SIZE, fp) ) {
		if ( strstr (line, string) )
			/* line matches */
			return true;
	}

	/* string not found in file */
	return false;
}
