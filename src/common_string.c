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
 * common_string.c contains code shared between GMT and PSL
 *
 * Author:  Florian Wobbe
 * Date:    3-MAR-2012
 * Version: 5
 *
 * Modules in this file:
 *
 *  gmt_chop                Chops off any CR or LF at end of string
 *  gmt_chop_ext            Chops off the trailing .xxx (file extension)
 *  gmt_get_ext             Returns a pointer to the tailing .xxx (file extension)
 *  gmt_strstrip            Strip leading and trailing whitespace from string
 *  gmt_strlshift           Left shift a string by n characters
 *  gmt_strrepc             Replaces all occurrences of a char in the string
 *  gmt_strrep              Replaces all occurrences of a string s2 in the string s1 by s3
 *  gmt_strlcmp             Compares strings (ignoring case) until first reaches null character
 *  gmt_strtok              Reiterant replacement of strtok
 *  gmt_strtok_m            A Matlab style strtok 
 *  gmt_dos_path_fix        Turn /c/dir/... paths into c:/dir/...
 *  str(n)casecmp           Case-insensitive string comparison functions
 *  strtok_r                Reentrant string tokenizer from Gnulib (LGPL)
 *  strsep                  Reentrant string tokenizer that handles empty fields
 *  strsepz                 Like strsep but ignores empty fields
 *  stresep                 Like strsep but takes an additional argument esc in order
 *                          to ignore escaped chars (from NetBSD)
 *  basename                Extract the base portion of a pathname
 */

/* CMake definitions: This must be first! */
#include "gmt_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <limits.h>
#include <errno.h>

#include "gmt_notposix.h"
#include "common_string.h"

#define BUF_SIZE 4096

char *gmt_chop_ext (char *string) {
	/* Chops off the filename extension (e.g., .ps) in the string by replacing the last
	 * '.' with '\0' and returns a pointer to the extension or NULL if not found. */
	char *p;
	assert (string != NULL); /* NULL pointer */
	if ((p = strrchr(string, '.'))) {
		*p = '\0';
		return (p + 1);
	}
	return NULL;
}

void gmt_chop (char *string) {
	/* Chops off any CR or LF and terminates string */
	char *p;
	assert (string != NULL); /* NULL pointer */
  /* if (string == NULL) return; / NULL pointer */
	if ((p = strpbrk (string, "\r\n")))
		/* Overwrite 1st CR or LF with terminate string */
		*p = '\0';
}

char *gmt_get_ext (const char *string) {
	/* Returns a pointer to the filename extension (e.g., .ps)  or NULL if not found. */
	char *p;
	assert (string != NULL); /* NULL pointer */
	if ((p = strrchr(string, '.'))) {
		return (p + 1);
	}
	return NULL;
}

void gmt_strstrip(char *string, bool strip_leading) {
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

void gmt_strlshift (char *string, size_t n) {
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

void gmt_strrepc (char *string, int c, int r) {
	/* Replaces all occurrences of c in the string with r */
	assert (string != NULL); /* NULL pointer */
	do {
		if (*string == c)
			*string = (char)r;
	} while (*(++string)); /* repeat until \0 reached */
}

size_t gmt_strlcmp (char *str1, char *str2) {
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

unsigned int gmt_strtok (const char *string, const char *sep, unsigned int *pos, char *token) {
	/* Reentrant replacement for strtok that uses no static variables.
	 * Breaks string into tokens separated by one of more separator
	 * characters (in sep).  Set *pos to 0 before first call. Unlike
	 * strtok, always pass the original string as first argument.
	 * Returns 1 if it finds a token and 0 if no more tokens left.
	 * pos is updated and token is returned.  char *token must point
	 * to memory of length >= strlen (string).
	 * string is not changed by gmt_strtok.
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
	*pos = (unsigned int)i;

	return 1;
}

void gmt_strtok_m (char *in, char **token, char **remain, char *sep) {
	/* A Matlab style strtok. Note that 'token' and 'remain' must be virgin pointers,
	   otherwise the memory they point to will be leaked because they are allocated here
	   with strdup. For that reason the caller is responsible to free them after being consumed.
	 */
	unsigned int pos = 0;
	char *p, *s;

	if (sep == NULL)
		s = " \t";
	else
		s = sep;

	token[0] = NULL;		remain[0] = NULL;

	p = calloc(strlen(in)+1, sizeof (char));
	if (gmt_strtok (in, s, &pos, p)) {
		token[0] = strdup(p);
		if (gmt_strtok (in, s, &pos, p))
			remain[0] = strdup(p);
	}
	free(p);
}

unsigned int gmt_get_modifier (const char *string, char modifier, char *token) {
	/* Looks for modifier string in the form +<modifier>[arg] and if found
	   returns 1 and places arg in token, else return 0.  Must ignore any
	   +<modifier> found inside quotes as part of text. If token is NULL
	   then we only return 1 or 0 and no string.
	*/
	bool quoted = false;
	size_t k, len, start = 0;

	if (!string || string[0] == 0) return 0;	/* No hope */
	len = strlen (string);
	for (k = 0; start == 0 && k < (len-1); k++) {
		if (string[k] == '\"' || string[k] == '\'') quoted = !quoted;	/* Initially false, becomes true at start of quote, then false when exit quote */
		if (quoted) continue;		/* Not look inside quoted strings */
		if (string[k] == '+' && string[k+1] == modifier)	/* Found the start */
			start = k+2;
	}
	if (start == 0) return 0;	/* Not found */
	for (k = start; k < len; k++) {
		if (string[k] == '\"' || string[k] == '\'') quoted = !quoted;	/* Initially false, becomes true at start of quote, then false when exit quote */
		if (quoted) continue;	/* Not look inside quoted strings */
		if (string[k] == '+')	/* Found the end */
			break;
	}
	len = k - start;
	if (token) {	/* Only pass back when token is not NULL */
		if (len) strncpy (token, &string[start], len);
		token[len] = '\0';
	}
	return 1;
}

#ifdef WIN32
/* Turn '/c/dir/...' paths into 'c:/dir/...'
 * Must do it in a loop since dir may be several ';'-separated dirs */
void gmt_dos_path_fix (char *dir) {
	size_t n, k;

	if (!dir || (n = strlen (dir)) < 2U)
		/* Given NULL or too short dir to work */
		return;

	if (!strncmp (dir, "/cygdrive/", 10U))
		/* May happen for example when Cygwin sets GMT_SHAREDIR */
		gmt_strlshift (dir, 9); /* Chop '/cygdrive' */

	/* Replace dumb backslashes with slashes */
	gmt_strrepc (dir, '\\', '/');

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

	/* Do the same with dirs separated by ';' but do not replace '/c/j/...' with 'c:j:/...' */
	for (k = 4; k < n-2; k++) {
		if ( (dir[k-1] == ';' && dir[k] == '/' && dir[k+2] == '/' && isalpha ((int)dir[k+1])) ) {
			dir[k] = dir[k+1];
			dir[k+1] = ':';
		}
	}

	/* Replace ...:C:/... by ...;C:/... as that was a multi-path set by a e.g. bash shell (msys or cygwin) */
	for (k = 4; k < n-2; k++) {
		if ((dir[k-1] == ':' && dir[k+1] == ':' && dir[k+2] == '/' && isalpha ((int)dir[k])) )
			dir[k-1] = ';';
		else if ((dir[k-1] == ':' && dir[k] == '/' && dir[k+2] == '/' && isalpha ((int)dir[k+1])) ) {
			/* The form ...:/C/... will become ...;C:/... */
			dir[k-1] = ';';
			dir[k] = dir[k+1];
			dir[k+1] = ':';
		}
	}
}
#endif

#if !defined(HAVE_STRCASECMP) && !defined(HAVE_STRICMP)
/* Case-insensitive string comparison function.
   Copyright (C) 1998-1999, 2005-2007, 2009-2012 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.  */

#ifndef TOLOWER
#	define TOLOWER(Ch) (isupper (Ch) ? tolower (Ch) : (Ch))
#endif

/* Compare strings S1 and S2, ignoring case, returning less than, equal to or
   greater than zero if S1 is lexicographically less than, equal to or greater
   than S2.
   Note: This function does not work with multibyte strings!  */

int strcasecmp (const char *s1, const char *s2) {
	const unsigned char *p1 = (const unsigned char *) s1;
	const unsigned char *p2 = (const unsigned char *) s2;
	unsigned char c1, c2;

	if (p1 == p2) return 0;

	do {
		c1 = TOLOWER (*p1);
		c2 = TOLOWER (*p2);

		if (c1 == '\0')
			break;

		++p1;
		++p2;
	}
	while (c1 == c2);

	if (UCHAR_MAX <= INT_MAX)
		return c1 - c2;
	else
		/* On machines where 'char' and 'int' are types of the same size, the
		difference of two 'unsigned char' values - including the sign bit -
		doesn't fit in an 'int'.  */
		return (c1 > c2 ? 1 : c1 < c2 ? -1 : 0);
}
#endif /* !defined(HAVE_STRCASECMP) && !defined(HAVE_STRICMP) */

#if !defined(HAVE_STRNCASECMP) && !defined(HAVE_STRNICMP)
/* strncasecmp.c -- case insensitive string comparator
   Copyright (C) 1998-1999, 2005-2007, 2009-2012 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.  */

#ifndef TOLOWER
#	define TOLOWER(Ch) (isupper (Ch) ? tolower (Ch) : (Ch))
#endif

/* Compare no more than N bytes of strings S1 and S2, ignoring case,
   returning less than, equal to or greater than zero if S1 is
   lexicographically less than, equal to or greater than S2.
   Note: This function cannot work correctly in multibyte locales.  */

int strncasecmp (const char *s1, const char *s2, size_t n) {
  register const unsigned char *p1 = (const unsigned char *) s1;
  register const unsigned char *p2 = (const unsigned char *) s2;
  unsigned char c1, c2;

  if (p1 == p2 || n == 0)
    return 0;

  do
    {
      c1 = TOLOWER (*p1);
      c2 = TOLOWER (*p2);

      if (--n == 0 || c1 == '\0')
        break;

      ++p1;
      ++p2;
    }
  while (c1 == c2);

  if (UCHAR_MAX <= INT_MAX)
    return c1 - c2;
  else
    /* On machines where 'char' and 'int' are types of the same size, the
       difference of two 'unsigned char' values - including the sign bit -
       doesn't fit in an 'int'.  */
    return (c1 > c2 ? 1 : c1 < c2 ? -1 : 0);
}
#endif /* !defined(HAVE_STRNCASECMP) && !defined(HAVE_STRNICMP) */

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

#if 0
/* Parse S into tokens separated by characters in DELIM. */
   If S is NULL, the saved pointer in SAVE_PTR is used as
   the next starting point.  For example:
        char s[] = "-abc-=-def";
        char *sp;
        x = strtok_r(s, "-", &sp);      /* x = "abc", sp = "=-def"	*/
        x = strtok_r(NULL, "-=", &sp);  /* x = "def", sp = NULL		*/
        x = strtok_r(NULL, "=", &sp);   /* x = NULL					*/
                                        /* s = "abc\0-def\0"		*/
#endif
char *strtok_r (char *s, const char *delim, char **save_ptr) {
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

char * strsep (char **stringp, const char *delim) {
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

/* Like strsep but ignores empty fields.
 * PW: Made two near-identical version of strsepz:
 *   strsepz:  As it was, but with an unused 3rd argument.
 *   strsepzp: 3rd argument returns position in original stringp.
 */

char *strsepz (char **stringp, const char *delim, size_t *pos) {
	char *c;
	(void)(pos);
	while ( (c = strsep(stringp, delim)) != NULL && *c == '\0' );
	return c;
}

char *strsepzp (char **stringp, const char *delim, size_t *pos) {
	char *c;
	while ( (c = strsep(stringp, delim)) != NULL && *c == '\0' )
		(*pos)++;
	if (c) (*pos) += strlen (c) + 1;
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

char *stresep(char **stringp, const char *delim, int esc) {
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

/* $OpenBSD: basename.c,v 1.14 2005/08/08 08:05:33 espie Exp $
 *
 * Copyright (c) 1997, 2004 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* The basename() function returns the last component from the pathname
 * pointed to by path, deleting any trailing `/' characters.  If path consists
 * entirely of `/' characters, a pointer to the string "/" is returned.  If
 * path is a null pointer or the empty string, a pointer to the string "." is
 * returned.
 *
 * On successful completion, basename() returns a pointer to the last
 * component of path.
 *
 * If basename() fails, a null pointer is returned and the global variable
 * errno is set to indicate the error.
 *
 * The basename() function returns a pointer to internal static storage
 * space that will be overwritten by subsequent calls (not thread safe).  The
 * function does not modify the string pointed to by path. */

#ifndef HAVE_BASENAME
char *basename(char *path) {
#ifdef WIN32
	static char path_fixed[PATH_MAX];
#endif
	static char bname[PATH_MAX];
	size_t len;
	const char *endp, *startp;

	/* Empty or NULL string gets treated as "." */
	if (path == NULL || *path == '\0') {
		bname[0] = '.';
		bname[1] = '\0';
		return (bname);
	}

#ifdef WIN32
	if (strchr (path, '\\')) {
		char *fixedp = path_fixed;
		if (strlen (path) >= PATH_MAX) {
			errno = ENAMETOOLONG;
			return (NULL);
		}
		/* Replace backslashes with slashes */
		while (*path) { /* repeat until \0 reached */
			if (*path == '\\')
				*fixedp = '/';
			else
				*fixedp = *path;
			++path, ++fixedp;
		}
		*fixedp = '\0'; /* terminate string */
		path = path_fixed;
	}
#endif

	/* Strip any trailing slashes */
	endp = path + strlen(path) - 1;
	while (endp > path && *endp == '/')
		endp--;

	/* All slashes becomes "/" */
	if (endp == path && *endp == '/') {
		bname[0] = '/';
		bname[1] = '\0';
		return (bname);
	}

	/* Find the start of the base */
	startp = endp;
	while (startp > path && *(startp - 1) != '/')
		startp--;

	len = endp - startp + 1;
	if (len >= PATH_MAX) {
		errno = ENAMETOOLONG;
		return (NULL);
	}
	memcpy(bname, startp, len);
	bname[len] = '\0';
	return (bname);
}
#endif

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
