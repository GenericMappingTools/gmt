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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "expat.h"
#include "udunits2.h"

#define NAME_SIZE 128
#define ACCUMULATE_TEXT \
    XML_SetCharacterDataHandler(currFile->parser, accumulateText)
#define IGNORE_TEXT \
    XML_SetCharacterDataHandler(currFile->parser, NULL)

typedef enum {
    START,
    UNIT_SYSTEM,
    PREFIX,
    UNIT,
    UNIT_NAME,
    ALIASES,
    ALIAS_NAME
} ElementType;

typedef struct {
    const char* path;
    char	singular[NAME_SIZE];
    char	plural[NAME_SIZE];
    char        symbol[NAME_SIZE];
    double      value;
    XML_Parser  parser;
    ut_unit*	unit;
    ElementType context;
    ut_encoding xmlEncoding;
    ut_encoding textEncoding;
    int         fd;
    int         skipDepth;
    int		prefixAdded;
    int         haveValue;
    int		isBase;
    int         isDimensionless;
    int         noPLural;
    int         nameSeen;
    int         symbolSeen;
} File;

typedef struct {
    char       ascii[NAME_SIZE];
    char       latin1[NAME_SIZE];
    char       latin1Nbsp[NAME_SIZE];
    char       utf8[NAME_SIZE];
    char       utf8Nbsp[NAME_SIZE];
} Identifiers;

static ut_status readXml(
    const char* const   path);

static File*            currFile = NULL;
static ut_system*	unitSystem = NULL;
static char*            text = NULL;
static size_t           nbytes = 0;


/*
 * Returns the plural form of a name.
 *
 * Arguments:
 *      singular        Pointer to the singular form of a name.
 * Returns:
 *      Pointer to the plural form of "singular".  Client must not free.  May be
 *      overwritten by subsequent calls.
 */
const char*
ut_form_plural(
    const char*	singular)
{
    static char	buf[NAME_SIZE];
    const char*	plural = NULL;		/* failure */

    if (singular != NULL) {
        int     length = strlen(singular);

	if (length + 3 >= sizeof(buf)) {
            ut_set_status(UT_SYNTAX);
	    ut_handle_error_message("Singular form is too long");
	    XML_StopParser(currFile->parser, 0);
	}
	else if (length > 0) {
	    (void)strcpy(buf, singular);

	    if (length == 1) {
		(void)strcpy(buf+length, "s");
	    }
	    else {
		char    lastChar = singular[length-1];

		if (lastChar == 'y') {
		    char	penultimateChar = singular[length-2];

		    if (penultimateChar == 'a' || penultimateChar == 'e' ||
			    penultimateChar == 'i' || penultimateChar == 'o' ||
			    penultimateChar == 'u') {
			(void)strcpy(buf+length, "s");
		    }
		    else {
			(void)strcpy(buf+length-1, "ies");
		    }
		}
		else {
		    if (lastChar == 's' || lastChar == 'x' || lastChar == 'z' ||
			    (length >= 2 && (
				strcmp(singular+length-2, "ch") == 0 ||
				strcmp(singular+length-2, "sh") == 0))) {
			(void)strcpy(buf+length, "es");
		    }
		    else {
			(void)strcpy(buf+length, "s");
		    }
		}
	    }

	    plural = buf;
        }
    }

    return plural;
}


/*
 * Substitutes one substring for all occurrences another in a string.
 *
 * Arguments:
 *      inString        Pointer to the input string.
 *      str             Pointer to the substring to be replaced.
 *      outString       Pointer to the output string buffer.
 *      repl            Pointer to the replacement substring.
 *      size            Size of the output string buffer, including the
 *                      terminating NUL.
 * Returns:
 *      0               Failure.  The output string buffer is too small.
 *      else            Success.
 */
static int
substitute(
    const char* const   inString,
    const char*         str,
    char* const         outString,
    const char*         repl,
    const size_t        size)
{
    const char* in = inString;
    char*       out = outString;
    char*       beyond = outString + size;
    size_t      strLen = strlen(str);
    size_t      replLen = strlen(repl);

    while (*in) {
        size_t      nbytes;
        char*       cp = strstr(in, str);

        if (cp == NULL) {
            nbytes = strlen(in);

            if (out + nbytes >= beyond) {
                ut_set_status(UT_SYNTAX);
                ut_handle_error_message("String \"%s\" is too long", inString);
                return 0;
            }

            (void)strncpy(out, in, nbytes);

            out += nbytes;

            break;
        }
        else {
            nbytes = (size_t)(cp - in);

            if (out + nbytes + replLen >= beyond) {
                ut_set_status(UT_SYNTAX);
                ut_handle_error_message("String \"%s\" is too long", inString);
                return 0;
            }

            (void)strncpy(out, in, nbytes);

            out += nbytes;

            (void)strncpy(out, repl, replLen);

            out += replLen;
            in += nbytes + strLen;
        }
    }

    *out = 0;

    return 1;
}


#define IS_ASCII(c)     (((c) & '\x80') == 0)


/*
 * Returns the UTF-8 string equivalent to a Latin-1 string.
 *
 * Arguments:
 *      inString        Pointer to the input, Latin-1 string.
 *      outString       Pointer to the output, UTF-8, string buffer.
 *      size            Size of "outString".
 * Returns:
 *      0               Failure.  "outString" is too small.
 *      1               Success.
 */
static int
latin1_to_utf8(
    const char* inString,
    char*       outString,
    size_t      size)
{
    size_t                      numSpecial = 0;
    const unsigned char*        in;
    unsigned char*              out;

    assert(inString != NULL);
    assert(outString != NULL);

    /*
     * Compute the number of non-ASCII characters.
     */
    for (in = (const unsigned char*)inString; *in; in++)
        if (!IS_ASCII(*in))
            numSpecial++;

    if (size < ((size_t)((char*)in - (char*)inString) + numSpecial + 1)) {
        ut_set_status(UT_SYNTAX);
        ut_handle_error_message("Identifier \"%s\" is too long", inString);
        return 0;
    }

    /*
     * Convert the string.
     */
    for (in = (const unsigned char*)inString, out = (unsigned char*)outString;
            *in; ++in) {
        if (IS_ASCII(*in)) {
            *out++ = *in;
        }
        else {
            *out++ = 0xC0 | ((0xC0 & *in) >> 6);
            *out++ = 0x80 | (0x3F & *in);
        }
    }

    *out = 0;

    return 1;
}


/*
 * Returns the Latin-1 string equivalent to a UTF-8 string, if possible.
 *
 * Arguments:
 *      inString        Pointer to the input, UTF-8 string.
 *      outString       Pointer to the output, Latin-1, string buffer.
 *      size            Size of "outString".
 * Returns:
 *     -1               Failure.  "outString" is too small.
 *      0               "inString" can't be represented in Latin-1.
 *      1               Success.
 */
static int
utf8_to_latin1(
    const char* inString,
    char*       outString,
    size_t      size)
{
    size_t                      numSpecial = 0;
    const unsigned char*        in;
    unsigned char*              out;

    assert(inString != NULL);
    assert(outString != NULL);

    /*
     * Compute the number of non-ASCII characters.
     */
    for (in = (const unsigned char*)inString; *in; in++) {
        if (*in > 0xC3)
            return 0;

        if (!IS_ASCII(*in)) {
            numSpecial++;
            in++;
        }
    }

    if (size < ((size_t)((char*)in - (char*)inString) - numSpecial + 1)) {
        ut_set_status(UT_SYNTAX);
        ut_handle_error_message("Identifier \"%s\" is too long", inString);
        return -1;
    }

    /*
     * Convert the string.
     */
    for (in = (const unsigned char*)inString, out = (unsigned char*)outString;
            *in; ++out) {
        if (IS_ASCII(*in)) {
            *out = *in++;
        }
        else {
            *out = (*in++ & 0x3) << 6;
            *out |= (*in++ & 0x3F);
        }
    }

    *out = 0;

    return 1;
}


#define LATIN1_NBSP "\xA0"
#define UTF8_NBSP "\xC2\xA0"


/*
 * Creates the regular and NBSP forms of a Latin-1 string.
 *
 * Arguments:
 *      latin1          Pointer to the input Latin-1 string.
 *      nonNbsp         Pointer to an output string buffer of size NAME_SIZE
 *                      that will be identical to "latin1" but with all
 *                      non-breaking spaces replaced with underscores.
 *      nbsp            Pointer to an output string buffer of size NAME_SIZE
 *                      that will be identical to "latin1" but with all
 *                      underscores replaced with non-breaking spaces.
 */
static void
make_latin1_forms(
    const char* latin1,
    char*       nonNbsp,
    char*       nbsp)
{
    if (strchr(latin1, '_')) {
        substitute(latin1, "_", nbsp, LATIN1_NBSP, NAME_SIZE);
        substitute(nbsp, LATIN1_NBSP, nonNbsp, "_", NAME_SIZE);
    }
    else if (strstr(latin1, LATIN1_NBSP)) {
        substitute(latin1, LATIN1_NBSP, nonNbsp, "_", NAME_SIZE);
        substitute(nonNbsp, "_", nbsp, LATIN1_NBSP, NAME_SIZE);
    }
    else {
        (void)strcpy(nonNbsp, latin1);
        *nbsp = 0;
    }
}


/*
 * Creates the regular and NBSP forms of a UTF-8 string.
 *
 * Arguments:
 *      utf8            Pointer to the input UTF-8 string.
 *      nonNbsp         Pointer to an output string buffer of size NAME_SIZE
 *                      that will be identical to "utf8" but with all
 *                      non-breaking spaces replaced with underscores.
 *      nbsp            Pointer to an output string buffer of size NAME_SIZE
 *                      that will be identical to "utf8" but with all
 *                      underscores replaced with non-breaking spaces.
 */
static int
make_utf8_forms(
    const char* utf8,
    char*       nonNbsp,
    char*       nbsp)
{
    int         success;

    if (strchr(utf8, '_')) {
        success = substitute(utf8, "_", nbsp, UTF8_NBSP, NAME_SIZE);

        if (success)
            success = substitute(nbsp, UTF8_NBSP, nonNbsp, "_", NAME_SIZE);
    }
    else if (strstr(utf8, UTF8_NBSP)) {
        success = substitute(utf8, UTF8_NBSP, nonNbsp, "_", NAME_SIZE);

        if (success)
            success = substitute(nonNbsp, "_", nbsp, UTF8_NBSP, NAME_SIZE);
    }
    else {
        (void)strcpy(nonNbsp, utf8);
        *nbsp = 0;
        success = 1;
    }

    return success;
}


/*
 * Creates derivatives of an identifier, which are all legitimate combinations
 * of ASCII, Latin-1, and UTF-8, on the one hand, and underscore vs.
 * non-breaking spaces on the other.
 *
 * Arguments:
 *      id              Pointer to the identifier.
 *      encoding        The encoding of "id".
 *      ids             Pointer to an "Identifier" structure.
 * Returns:
 *      0               Failure.
 *      else            Success.  If a combination is not possible or is
 *                      identical to a simpler combination, then the first
 *                      character of the relevant member of "ids" will be NUL.
 */
static int
makeDerivatives(
    const char* const   id,
    const ut_encoding   encoding,
    Identifiers*        ids)
{
    int                 success = 1;

    assert(id != NULL);
    assert(ids != NULL);

    if (strlen(id) > NAME_SIZE - 1) {
        ut_set_status(UT_SYNTAX);
        ut_handle_error_message("Identifier \"%s\" is too long", id);
        success = 0;
    }
    else {
        ids->ascii[0] = 0;
        ids->latin1[0] = 0;
        ids->latin1Nbsp[0] = 0;
        ids->utf8[0] = 0;
        ids->utf8Nbsp[0] = 0;

        if (encoding == UT_ASCII) {
            (void)strcpy(ids->ascii, id);

            if (strchr(id, '_')) {
                (void)substitute(id, "_", ids->latin1Nbsp, LATIN1_NBSP,
                    NAME_SIZE);
                success =
                    latin1_to_utf8(ids->latin1Nbsp, ids->utf8Nbsp, NAME_SIZE);
            }
        }
        else if (encoding == UT_LATIN1) {
            make_latin1_forms(id, ids->latin1, ids->latin1Nbsp);

            success = latin1_to_utf8(ids->latin1, ids->utf8, NAME_SIZE) &&
                latin1_to_utf8(ids->latin1Nbsp, ids->utf8Nbsp, NAME_SIZE);
        }
        else {
            success = make_utf8_forms(id, ids->utf8, ids->utf8Nbsp) &&
                utf8_to_latin1(ids->utf8, ids->latin1, NAME_SIZE) != -1 &&
                utf8_to_latin1(ids->utf8Nbsp, ids->latin1Nbsp, NAME_SIZE) != -1;
        }

        if (success) {
            if (strcmp(ids->ascii, ids->latin1) == 0)
                ids->latin1[0] = 0;
            if (strcmp(ids->ascii, ids->latin1Nbsp) == 0)
                ids->latin1Nbsp[0] = 0;
            if (strcmp(ids->ascii, ids->utf8) == 0)
                ids->utf8[0] = 0;
            if (strcmp(ids->ascii, ids->utf8Nbsp) == 0)
                ids->utf8Nbsp[0] = 0;
        }
    }

    return success;
}


/*
 * Maps a unit to an identifier.
 *
 * Arguments:
 *      unit            Pointer to the unit.
 *      id              Pointer to the identifier.
 *      encoding        The encoding of "id".
 *      isName          Whether or not "id" is a name.
 * Returns:
 *      0               Failure.
 *      else            Success.
 */
static int
mapUnitToId(
    ut_unit* const        unit,
    const char* const     id,
    ut_encoding           encoding,
    int                   isName)
{
    int                 success = 0;             /* failure */
    ut_status           (*func)(const ut_unit*, const char*, ut_encoding);
    const char*         desc;

    if (isName) {
        func = ut_map_unit_to_name;
        desc = "name";
    }
    else {
        func = ut_map_unit_to_symbol;
        desc = "symbol";
    }

    if (func(unit, id, encoding) != UT_SUCCESS) {
        ut_set_status(UT_PARSE);
        ut_handle_error_message("Couldn't map unit to %s \"%s\"", desc, id);
    }
    else {
        success = 1;
    }

    return success;
}


/*
 * Maps a unit to identifiers.
 *
 * Arguments:
 *      unit            Pointer to the unit.
 *      id              Pointer to the identifier upon wich to base all
 *                      derived identifiers.
 *      encoding        The encoding of "id".
 *      isName          Whether or not "id" is a name.
 * Returns:
 *      0               Failure.
 *      else            Success.
 */
static int
mapUnitToIds(
    ut_unit* const        unit,
    const char* const     id,
    ut_encoding           encoding,
    int                   isName)
{
    int                 success = 1;             /* success */
    Identifiers         ids;

    if (!makeDerivatives(id, encoding, &ids)) {
        success = 0;
    }
    else {
        if (ids.ascii[0])
            success = mapUnitToId(unit, ids.ascii, UT_ASCII, isName);
        if (success && ids.latin1[0])
            success = mapUnitToId(unit, ids.latin1, UT_LATIN1, isName);
        if (success && ids.latin1Nbsp[0])
            success = mapUnitToId(unit, ids.latin1Nbsp, UT_LATIN1, isName);
        if (success && ids.utf8[0])
            success = mapUnitToId(unit, ids.utf8, UT_UTF8, isName);
        if (success && ids.utf8Nbsp[0])
            success = mapUnitToId(unit, ids.utf8Nbsp, UT_UTF8, isName);
    }

    return success;
}


/*
 * Maps a unit to a name and all derivatives of the name.
 *
 * Arguments:
 *      unit            Pointer to the unit.
 *      name            Pointer to the name upon wich to base all derived names.
 *      encoding        The encoding of "name".
 * Returns:
 *      0               Failure.
 *      else            Success.
 */
static int
mapUnitToNames(
    ut_unit* const        unit,
    const char* const     name,
    ut_encoding           encoding)
{
    return mapUnitToIds(unit, name, encoding, 1);
}


/*
 * Maps a unit to a symbol and all derivatives of the symbol.
 *
 * Arguments:
 *      unit            Pointer to the unit.
 *      symbol          Pointer to the symbol upon wich to base all derived
 *                      symbols.
 *      encoding        The encoding of "symbol".
 * Returns:
 *      0               Failure.
 *      else            Success.
 */
static int
mapUnitToSymbols(
    ut_unit* const        unit,
    const char* const     symbol,
    ut_encoding           encoding)
{
    return mapUnitToIds(unit, symbol, encoding, 0);
}


/*
 * Maps an identifier to a unit.
 *
 * Arguments:
 *      id              Pointer to the identifier.
 *      encoding        The character encoding of "id".
 *      unit            Pointer to the unit.
 *      isName          Whether or not "id" is a name.
 * Returns:
 *      0               Failure.
 *      else            Success.
 */
static int
mapIdToUnit(
    const char*	        id,
    const ut_encoding   encoding,
    ut_unit*	        unit,
    const int	        isName)
{
    int		success = 0;		/* failure */
    ut_unit*	prev = ut_get_unit_by_name(unitSystem, id);

    if (prev == NULL)
	prev = ut_get_unit_by_symbol(unitSystem, id);

    if (prev != NULL) {
	char	buf[128];
	int	nchar = ut_format(prev, buf, sizeof(buf),
	    UT_ASCII | UT_DEFINITION | UT_NAMES);

        ut_set_status(UT_PARSE);
	ut_handle_error_message(
	    "Duplicate definition for \"%s\" at \"%s\":%d", id,
            currFile->path, XML_GetCurrentLineNumber(currFile->parser));

	if (nchar < 0)
	    nchar =
                ut_format(prev, buf, sizeof(buf), UT_ASCII | UT_DEFINITION);

	if (nchar >= 0 && nchar < sizeof(buf)) {
	    buf[nchar] = 0;

            ut_set_status(UT_PARSE);
	    ut_handle_error_message("Previous definition was \"%s\"", buf);
	}

        XML_StopParser(currFile->parser, 0);
    }
    else {
	/*
	 * Take prefixes into account for a prior definition by using
         * ut_parse().
	 */
	prev = ut_parse(unitSystem, id, encoding);

	if ((isName
                    ? ut_map_name_to_unit(id, encoding, unit)
                    : ut_map_symbol_to_unit(id, encoding, unit))
                != UT_SUCCESS) {
            ut_set_status(UT_PARSE);
	    ut_handle_error_message("Couldn't map %s \"%s\" to unit",
		isName ? "name" : "symbol", id);
	    XML_StopParser(currFile->parser, 0);
	}
	else {
	    if (prev != NULL) {
		char	buf[128];
		int	nchar = ut_format(prev, buf, sizeof(buf),
		    UT_ASCII | UT_DEFINITION | UT_NAMES);

		if (nchar < 0)
		    nchar = ut_format(prev, buf, sizeof(buf),
			UT_ASCII | UT_DEFINITION);

		if (nchar < 0 || nchar >= sizeof(buf)) {
                    ut_set_status(UT_PARSE);
		    ut_handle_error_message("Definition of \"%s\" in \"%s\", "
                        "line %d, overrides prefixed-unit", id,
                        currFile->path,
			XML_GetCurrentLineNumber(currFile->parser));
		}
		else {
		    buf[nchar] = 0;

                    ut_set_status(UT_PARSE);
		    ut_handle_error_message("Definition of \"%s\" in \"%s\", "
                        "line %d, overrides prefixed-unit \"%s\"",
                        id, currFile->path,
                        XML_GetCurrentLineNumber(currFile->parser), buf);
		}
	    }

	    success = 1;
	}
    }

    ut_free(prev);                      /* NULL safe */

    return success;
}


/*
 * Maps an identifier and all derivatives to a unit.
 *
 * Arguments:
 *      id              Pointer to the identifier.
 *      encoding        The encoding of "id".
 *      unit            Pointer to the unit.
 *      isName          Whether or not "id" is a name.
 * Returns:
 *      0               Failure.
 *      else            Success.
 */
static int
mapIdsToUnit(
    const char* const   id,
    const ut_encoding   encoding,
    ut_unit* const      unit,
    const int           isName)
{
    Identifiers         ids;
    int                 success = 1;

    if (!makeDerivatives(id, encoding, &ids)) {
        success = 0;
    }
    else {
        if (ids.ascii[0])
            success = mapIdToUnit(ids.ascii, UT_ASCII, unit, isName);
        if (success && ids.latin1[0])
            success = mapIdToUnit(ids.latin1, UT_LATIN1, unit, isName);
        if (success && ids.latin1Nbsp[0])
            success = mapIdToUnit(ids.latin1Nbsp, UT_LATIN1, unit, isName);
        if (success && ids.utf8[0])
            success = mapIdToUnit(ids.utf8, UT_UTF8, unit, isName);
        if (success && ids.utf8Nbsp[0])
            success = mapIdToUnit(ids.utf8Nbsp, UT_UTF8, unit, isName);
    }

    return success;
}


/*
 * Maps a name and all derivatives to a unit.
 *
 * Arguments:
 *      name            Pointer to the name.
 *      encoding        The encoding of "name".
 *      unit            Pointer to the unit.
 * Returns:
 *      0               Failure.
 *      else            Success.
 */
static int
mapNamesToUnit(
    const char* const   name,
    const ut_encoding   encoding,
    ut_unit* const	unit)
{
    return mapIdsToUnit(name, encoding, unit, 1);
}


/*
 * Maps a symbol and all derivatives to a unit.
 *
 * Arguments:
 *      symbol          Pointer to the symbol.
 *      encoding        The encoding of "symbol".
 *      unit            Pointer to the unit.
 * Returns:
 *      0               Failure.
 *      else            Success.
 */
static int
mapSymbolsToUnit(
    const char* const   symbol,
    const ut_encoding   encoding,
    ut_unit* const	unit)
{
    return mapIdsToUnit(symbol, encoding, unit, 0);
}


/*
 * Maps between a unit and a name.
 *
 * Arguments:
 *      unit            Pointer to the unit.
 *      name            Pointer to the name  .
 *      encoding        The encoding of "name".
 * Returns:
 *      0               Failure.
 *      else            Success.
 */
static int
mapUnitAndName(
    ut_unit* const        unit,
    const char* const     name,
    ut_encoding           encoding)
{
    return
        mapNamesToUnit(name, encoding, unit) &&
        mapUnitToNames(unit, name, encoding);
}


/*
 * Maps between a unit and a symbol.
 *
 * Arguments:
 *      unit            Pointer to the unit.
 *      symbol          Pointer to the symbol  .
 *      encoding        The encoding of "symbol".
 * Returns:
 *      0               Failure.
 *      else            Success.
 */
static int
mapUnitAndSymbol(
    ut_unit* const        unit,
    const char* const     symbol,
    ut_encoding           encoding)
{
    return
        mapSymbolsToUnit(symbol, encoding, unit) &&
        mapUnitToSymbols(unit, symbol, encoding);
}


/*
 * Initializes a "File" data-structure to the default state.
 */
static void
fileInit(
    File* const file)
{
    file->context = START;
    file->skipDepth = 0;
    file->value = 0;
    file->xmlEncoding = UT_ASCII;
    file->textEncoding = UT_ASCII;
    file->unit = NULL;
    file->fd = -1;
    file->parser = NULL;
    file->isBase = 0;
    file->isDimensionless = 0;
    file->haveValue = 0;
    file->noPLural = 0;
    file->nameSeen = 0;
    file->symbolSeen = 0;
    file->path = NULL;
    (void)memset(file->singular, 0, sizeof(file->singular));
    (void)memset(file->plural, 0, sizeof(file->plural));
    (void)memset(file->symbol, 0, sizeof(file->symbol));
}


/*
 * Clears the text buffer for elements.
 */
static void
clearText(void)
{
    if (text != NULL)
	*text = 0;

    nbytes = 0;
    currFile->textEncoding = UT_ASCII;
}




/*
 * Accumulates the textual portion of an element.
 */
static void
accumulateText(
    void*		data,
    const char*		string,		/* input text in UTF-8 */
    int			len)
{
    char*	tmp = realloc(text, nbytes + len + 1);

    if (tmp == NULL) {
        ut_set_status(UT_OS);
	ut_handle_error_message(strerror(errno));
	ut_handle_error_message("Couldn't reallocate %lu-byte text buffer",
	    nbytes+len+1);
	XML_StopParser(currFile->parser, 0);
    }
    else {
        int     i;

        text = tmp;

        for (i = 0; i < len; i++) {
            text[nbytes++] = string[i];

            if (!IS_ASCII(string[i]))
                currFile->textEncoding = UT_UTF8;
        }

	text[nbytes] = 0;
    }
}


#if 0
/*
 * Converts the accumulated text from UTF-8 to user-specified encoding.
 *
 * Returns:
 *      0       Failure: the text could not be converted.
 *      else    Success.
 */
static int
convertText(void)
{
    int         success = 1;

    if (currFile->encoding == UT_ASCII) {
        unsigned char*   cp;

        for (cp = text; *cp && IS_ASCII(*cp); cp++)
            /* EMPTY */;

        if (*cp) {
            ut_set_status(UT_SYNTAX);
            ut_handle_error_message("Character isn't US-ASCII: %#x", *cp);
            XML_StopParser(currFile->parser, 0);

            success = 0;
        }
        else {
            textEncoding = UT_ASCII;
        }
    }
    else if (currFile->encoding == UT_LATIN1) {
        unsigned char*       in;
        unsigned char*       out;

        for (in = out = text; *in; ++in, ++out) {
            if (IS_ASCII(*in)) {
                *out = *in;
            }
            else {
                if (*in <= 0xC3) {
                    *out = (unsigned char)((*in++ & 0x3) << 6);
                    *out |= (unsigned char)(*in & 0x3F);
                }
                else {
                    ut_set_status(UT_SYNTAX);
                    ut_handle_error_message(
                        "Character is not representable in ISO-8859-1 "
                        "(Latin-1): %d", 1+(int)((char*)out - text));
                    XML_StopParser(currFile->parser, 0);

                    success = 0;

                    break;
                }
            }
        }

        *out = 0;
        nbytes = out - (unsigned char*)text;
        textEncoding = UT_LATIN1;
    }

    return success;
}
#endif


/*
 * Handles the start of a <unit-system> element.
 */
static void
startUnitSystem(
    void*		data,
    const char**	atts)
{
    if (currFile->context != START) {
	ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <unit-system> element");
	XML_StopParser(currFile->parser, 0);
    }

    currFile->context = UNIT_SYSTEM;
}


/*
 * Handles the end of a <unit-system> element.
 */
static void
endUnitSystem(
    void*		data)
{}


/*
 * Handles the start of a <prefix> element.
 */
static void
startPrefix(
    void*		data,
    const char* const*	atts)
{
    if (currFile->context != UNIT_SYSTEM) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <prefix> element");
    }
    else {
	currFile->prefixAdded = 0;
	currFile->haveValue = 0;
    }

    currFile->context = PREFIX;
}


/*
 * Handles the end of a <prefix> element.
 */
static void
endPrefix(
    void*		data)
{
    if (!currFile->haveValue || !currFile->prefixAdded) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Prefix incompletely specified");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	currFile->haveValue = 0;
    }

    currFile->context = UNIT_SYSTEM;
}


/*
 * Handles the start of a <unit> element.
 */
static void
startUnit(
    void*		data,
    const char**	atts)
{
    if (currFile->context != UNIT_SYSTEM) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <unit> element");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	ut_free(currFile->unit);
	currFile->unit = NULL;
	currFile->isBase = 0;
	currFile->isDimensionless = 0;
        currFile->singular[0] = 0;
        currFile->plural[0] = 0;
        currFile->symbol[0] = 0;
        currFile->nameSeen = 0;
        currFile->symbolSeen = 0;
    }

    currFile->context = UNIT;
}


/*
 * Handles the end of a <unit> element.
 */
static void
endUnit(
    void*		data)
{
    if (currFile->isBase) {
        if (!currFile->nameSeen) {
            ut_set_status(UT_PARSE);
            ut_handle_error_message("Base unit needs a name");
            XML_StopParser(currFile->parser, 0);
        }
        if (!currFile->symbolSeen) {
            ut_set_status(UT_PARSE);
            ut_handle_error_message("Base unit needs a symbol");
            XML_StopParser(currFile->parser, 0);
        }
    }

    ut_free(currFile->unit);
    currFile->unit = NULL;
    currFile->context = UNIT_SYSTEM;
}


/*
 * Handles the start of a <base> element.
 */
static void
startBase(
    void*		data,
    const char**	atts)
{
    if (currFile->context != UNIT) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <base> element");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	if (currFile->isDimensionless) {
            ut_set_status(UT_PARSE);
	    ut_handle_error_message(
		"<dimensionless> and <base> are mutually exclusive");
	    XML_StopParser(currFile->parser, 0);
	}
	else if (currFile->unit != NULL) {
            ut_set_status(UT_PARSE);
	    ut_handle_error_message("<base> and <def> are mutually exclusive");
	    XML_StopParser(currFile->parser, 0);
	}
	else if (currFile->isBase) {
            ut_set_status(UT_PARSE);
	    ut_handle_error_message("<base> element already seen");
	    XML_StopParser(currFile->parser, 0);
	}
    }
}


/*
 * Handles the end of a <base> element.
 */
static void
endBase(
    void*		data)
{
    currFile->unit = ut_new_base_unit(unitSystem);

    if (currFile->unit == NULL) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Couldn't create new base unit");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	currFile->isBase = 1;
    }
}


/*
 * Handles the start of a <dimensionless> element.
 */
static void
startDimensionless(
    void*		data,
    const char**	atts)
{
    if (currFile->context != UNIT) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <dimensionless> element");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	if (currFile->isBase) {
            ut_set_status(UT_PARSE);
	    ut_handle_error_message(
		"<dimensionless> and <base> are mutually exclusive");
	    XML_StopParser(currFile->parser, 0);
	}
	else if (currFile->unit != NULL) {
            ut_set_status(UT_PARSE);
	    ut_handle_error_message(
		"<dimensionless> and <def> are mutually exclusive");
	    XML_StopParser(currFile->parser, 0);
	}
	else if (currFile->isDimensionless) {
            ut_set_status(UT_PARSE);
	    ut_handle_error_message("<dimensionless> element already seen");
	    XML_StopParser(currFile->parser, 0);
	}
    }
}


/*
 * Handles the end of a <dimensionless> element.
 */
static void
endDimensionless(
    void*		data)
{
    currFile->unit = ut_new_dimensionless_unit(unitSystem);

    if (currFile->unit == NULL) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Couldn't create new dimensionless unit");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	currFile->isDimensionless = 1;
    }
}


/*
 * Handles the start of a <def> element.
 */
static void
startDef(
    void*		data,
    const char**	atts)
{
    if (currFile->context != UNIT) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <def> element");
	XML_StopParser(currFile->parser, 0);
    }
    else if (currFile->isBase) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message(
	    "<base> and <def> are mutually exclusive");
	XML_StopParser(currFile->parser, 0);
    }
    else if (currFile->isDimensionless) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message(
	    "<dimensionless> and <def> are mutually exclusive");
	XML_StopParser(currFile->parser, 0);
    }
    else if (currFile->unit != NULL) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("<def> element already seen");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	clearText();
        ACCUMULATE_TEXT;
    }
}


/*
 * Handles the end of a <def> element.
 */
static void
endDef(
    void*		data)
{
    if (nbytes == 0) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Empty unit definition");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	currFile->unit = ut_parse(unitSystem, text, currFile->textEncoding);

	if (currFile->unit == NULL) {
            ut_set_status(UT_PARSE);
	    ut_handle_error_message(
                "Couldn't parse unit specification \"%s\"", text);
	    XML_StopParser(currFile->parser, 0);
	}
    }
}


/*
 * Handles the start of a <name> element.
 */
static void
startName(
    void*		data,
    const char**	atts)
{
    if (currFile->context == PREFIX) {
        if (!currFile->haveValue) {
            ut_set_status(UT_PARSE);
            ut_handle_error_message("No previous <value> element");
            XML_StopParser(currFile->parser, 0);
        }
        else {
            clearText();
            ACCUMULATE_TEXT;
        }
    }
    else if (currFile->context == UNIT || currFile->context == ALIASES) {
        if (currFile->unit == NULL) {
            ut_set_status(UT_PARSE);
            ut_handle_error_message(
                "No previous <base>, <dimensionless>, or <def> element");
            XML_StopParser(currFile->parser, 0);
        }
        else {
            currFile->noPLural = 0;
            currFile->singular[0] = 0;
           currFile->plural[0] = 0;
            currFile->context =
                currFile->context == UNIT ? UNIT_NAME : ALIAS_NAME;
        }
    }
    else {
        ut_set_status(UT_PARSE);
        ut_handle_error_message("Wrong place for <name> element");
        XML_StopParser(currFile->parser, 0);
    }
}


/*
 * Handles the end of a <name> element.
 */
static void
endName(
    void*		data)
{
    if (currFile->context == PREFIX) {
	if (!currFile->haveValue) {
            ut_set_status(UT_PARSE);
	    ut_handle_error_message("No previous <value> element");
	    XML_StopParser(currFile->parser, 0);
	}
	else {
	    if (ut_add_name_prefix(unitSystem, text, currFile->value) !=
                    UT_SUCCESS) {
                ut_set_status(UT_PARSE);
		ut_handle_error_message(
		    "Couldn't map name-prefix \"%s\" to value %g", text,
                    currFile->value);
		XML_StopParser(currFile->parser, 0);
	    }
	    else {
		currFile->prefixAdded = 1;
	    }
	}
    }
    else if (currFile->context == UNIT_NAME) {
        if (currFile->singular[0] == 0) {
            ut_set_status(UT_PARSE);
            ut_handle_error_message("<name> needs a <singular>");
            XML_StopParser(currFile->parser, 0);
        }
        else {
            if (!mapUnitAndName(currFile->unit, currFile->singular,
                    currFile->textEncoding)) {
                XML_StopParser(currFile->parser, 0);
            }
            else {
                if (!currFile->noPLural) {
                    const char* plural = NULL;

                    if (currFile->plural[0] != 0) {
                        plural = currFile->plural;
                    }
                    else if (currFile->singular[0] != 0) {
                        plural = ut_form_plural(currFile->singular);

                        if (plural == NULL) {
                            ut_set_status(UT_PARSE);
                            ut_handle_error_message("Couldn't form plural of "
                                "\"%s\"", currFile->singular);
                            XML_StopParser(currFile->parser, 0);
                        }
                    }

                    if (plural != NULL) {
                        /*
                         * Because the unit is already mapped to the singular
                         * name, it is not mapped to the plural name.
                         */
                        if (!mapNamesToUnit(plural, currFile->textEncoding,
                                currFile->unit)) {
                            XML_StopParser(currFile->parser, 0);
                        }
                    }
                }                       /* <noplural/> not specified */
                if (strcmp(currFile->singular, "second") == 0) {
                    if (ut_set_second(currFile->unit) != UT_SUCCESS) {
                        ut_handle_error_message(
                            "Couldn't set \"second\" unit in unit-system");
                        XML_StopParser(currFile->parser, 0);
                    }
                }                       /* unit was 'second' unit */
            }                           /* unit mapped to singular name */
        }                               /* singular name specified */

        currFile->nameSeen = 1;
        currFile->context = UNIT;
    }					/* defining name for unit */
    else if (currFile->context == ALIAS_NAME) {
	if (currFile->singular[0] == 0) {
            ut_set_status(UT_PARSE);
            ut_handle_error_message("<name> needs a <singular>");
            XML_StopParser(currFile->parser, 0);
        }
        else {
            if (!mapNamesToUnit(currFile->singular, currFile->textEncoding,
                    currFile->unit)) {
                XML_StopParser(currFile->parser, 0);
            }

            if (!currFile->noPLural) {
                const char* plural = NULL;

                if (currFile->plural[0] != 0) {
                    plural = currFile->plural;
                }
                else if (currFile->singular[0] != 0) {
                    plural = ut_form_plural(currFile->singular);

                    if (plural == NULL) {
                        ut_set_status(UT_PARSE);
                        ut_handle_error_message("Couldn't form plural of "
                            "\"%s\"", currFile->singular);
                        XML_StopParser(currFile->parser, 0);
                    }
                }

                if (plural != NULL) {
                    if (!mapNamesToUnit(plural, currFile->textEncoding,
                            currFile->unit))
                        XML_StopParser(currFile->parser, 0);
                }
            }                           /* <noplural> not specified */
        }                               /* singular name specified */

        currFile->context = ALIASES;
    }                                   /* defining name for alias */
    else {
	assert(0);
    }
}


/*
 * Handles the start of a <singular> element.
 */
static void
startSingular(
    void*		data,
    const char**	atts)
{
    if (currFile->context != UNIT_NAME && currFile->context != ALIAS_NAME) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <singular> element");
	XML_StopParser(currFile->parser, 0);
    }
    else if (currFile->singular[0] != 0) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("<singular> element already seen");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	clearText();
        ACCUMULATE_TEXT;
    }
}


/*
 * Handles the end of a <singular> element.
 */
static void
endSingular(
    void*		data)
{
    if (nbytes >= NAME_SIZE) {
        ut_set_status(UT_PARSE);
        ut_handle_error_message("Name \"%s\" is too long", text);
        XML_StopParser(currFile->parser, 0);
    }
    else {
        (void)strcpy(currFile->singular, text);
    }
}


/*
 * Handles the start of a <plural> element.
 */
static void
startPlural(
    void*		data,
    const char**	atts)
{
    if (currFile->context != UNIT_NAME && currFile->context != ALIAS_NAME ) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <plural> element");
	XML_StopParser(currFile->parser, 0);
    }
    else if (currFile->noPLural || currFile->plural[0] != 0) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("<plural> or <noplural> element already seen");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	clearText();
        ACCUMULATE_TEXT;
    }
}


/*
 * Handles the end of a <plural> element.
 */
static void
endPlural(
    void*		data)
{
    if (nbytes == 0) {
        ut_set_status(UT_PARSE);
        ut_handle_error_message("Empty <plural> element");
        XML_StopParser(currFile->parser, 0);
    }
    else if (nbytes >= NAME_SIZE) {
        ut_set_status(UT_PARSE);
        ut_handle_error_message("Plural name \"%s\" is too long", text);
        XML_StopParser(currFile->parser, 0);
    }
    else {
        (void)strcpy(currFile->plural, text);
    }
}


/*
 * Handles the start of a <noplural> element.
 */
static void
startNoPlural(
    void*		data,
    const char**	atts)
{
    if (currFile->context != UNIT_NAME || currFile->context != ALIAS_NAME) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <noplural> element");
	XML_StopParser(currFile->parser, 0);
    }
    else if (currFile->plural[0] != 0) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("<plural> element already seen");
	XML_StopParser(currFile->parser, 0);
    }
}


/*
 * Handles the end of a <noplural> element.
 */
static void
endNoPlural(
    void*		data)
{
    currFile->noPLural = 1;
}


/*
 * Handles the start of a <symbol> element.
 */
static void
startSymbol(
    void*		data,
    const char**	atts)
{
    if (currFile->context == PREFIX) {
        if (!currFile->haveValue) {
            ut_set_status(UT_PARSE);
            ut_handle_error_message("No previous <value> element");
            XML_StopParser(currFile->parser, 0);
        }
        else {
            clearText();
            ACCUMULATE_TEXT;
        }
    }
    else if (currFile->context == UNIT || currFile->context == ALIASES) {
        if (currFile->unit == NULL) {
            ut_set_status(UT_PARSE);
            ut_handle_error_message(
                "No previous <base>, <dimensionless>, or <def> element");
            XML_StopParser(currFile->parser, 0);
        }
        else {
            clearText();
            ACCUMULATE_TEXT;
        }
    }
    else {
        ut_set_status(UT_PARSE);
        ut_handle_error_message("Wrong place for <symbol> element");
        XML_StopParser(currFile->parser, 0);
    }
}


/*
 * Handles the end of a <symbol> element.
 */
static void
endSymbol(
    void*		data)
{
    if (currFile->context == PREFIX) {
        if (ut_add_symbol_prefix(unitSystem, text, currFile->value) !=
                UT_SUCCESS) {
            ut_set_status(UT_PARSE);
            ut_handle_error_message(
                "Couldn't map symbol-prefix \"%s\" to value %g",
                text, currFile->value);
            XML_StopParser(currFile->parser, 0);
        }
        else {
            currFile->prefixAdded = 1;
        }
    }
    else if (currFile->context == UNIT) {
        if (!mapUnitAndSymbol(currFile->unit, text, currFile->textEncoding))
            XML_StopParser(currFile->parser, 0);

        currFile->symbolSeen = 1;
    }
    else if (currFile->context == ALIASES) {
        if (!mapSymbolsToUnit(text, currFile->textEncoding, currFile->unit))
            XML_StopParser(currFile->parser, 0);
    }
}


/*
 * Handles the start of a <value> element.
 */
static void
startValue(
    void*		data,
    const char**	atts)
{
    if (currFile->context != PREFIX) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <value> element");
	XML_StopParser(currFile->parser, 0);
    }
    else if (currFile->haveValue) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("<value> element already seen");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	clearText();
        ACCUMULATE_TEXT;
    }
}


/*
 * Handles the end of a <value> element.
 */
static void
endValue(
    void*	data)
{
    char*	endPtr;

    errno = 0;
    currFile->value = strtod(text, &endPtr);

    if (errno != 0) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message(strerror(errno));
	ut_handle_error_message("Couldn't decode numeric prefix value \"%s\"",
            text);
	XML_StopParser(currFile->parser, 0);
    }
    else if (*endPtr != 0) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Invalid numeric prefix value \"%s\"", text);
	XML_StopParser(currFile->parser, 0);
    }
    else {
	currFile->haveValue = 1;
    }
}


/*
 * Handles the start of an <alias> element.
 */
static void
startAliases(
    void*		data,
    const char**	atts)
{
    if (currFile->context != UNIT) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <aliases> element");
	XML_StopParser(currFile->parser, 0);
    }

    currFile->context = ALIASES;
}


/*
 * Handles the end of an <alias> element.
 */
static void
endAliases(
    void*		data)
{
    currFile->context = UNIT;
}


/*
 * Handles the start of an <import> element.
 */
static void
startImport(
    void*		data,
    const char**	atts)
{
    if (currFile->context != UNIT_SYSTEM) {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Wrong place for <import> element");
	XML_StopParser(currFile->parser, 0);
    }
    else {
	clearText();
        ACCUMULATE_TEXT;
    }
}


/*
 * Handles the end of an <import> element.
 */
static void
endImport(
    void*		data)
{
    char        buf[_POSIX_PATH_MAX];
    const char* path;

    if (text[0] == '/') {
        path = text;
    }
    else {
        (void)snprintf(buf, sizeof(buf), "%s/%s",
            XML_GetBase(currFile->parser), text);

        buf[sizeof(buf)-1] = 0;
        path = buf;
    }

    ut_set_status(readXml(path));

    if (ut_get_status() != UT_SUCCESS)
        XML_StopParser(currFile->parser, 0);
}


/*
 * Handles the start of an element.
 */
static void
startElement(
    void*		data,
    const XML_Char*	name,
    const XML_Char**	atts)
{
    if (currFile->skipDepth) {
	currFile->skipDepth++;
    }
    else {
	clearText();

	if (strcasecmp(name, "unit-system") == 0) {
	    startUnitSystem(data, atts);
	}
	else if (strcasecmp(name, "prefix") == 0) {
	    startPrefix(data, atts);
	}
	else if (strcasecmp(name, "unit") == 0) {
	    startUnit(data, atts);
	}
	else if (strcasecmp(name, "base") == 0) {
	    startBase(data, atts);
	}
	else if (strcasecmp(name, "dimensionless") == 0) {
	    startDimensionless(data, atts);
	}
	else if (strcasecmp(name, "def") == 0) {
	    startDef(data, atts);
	}
	else if (strcasecmp(name, "value") == 0) {
	    startValue(data, atts);
	}
	else if (strcasecmp(name, "name") == 0) {
	    startName(data, atts);
	}
	else if (strcasecmp(name, "singular") == 0) {
	    startSingular(data, atts);
	}
	else if (strcasecmp(name, "plural") == 0) {
	    startPlural(data, atts);
	}
	else if (strcasecmp(name, "symbol") == 0) {
	    startSymbol(data, atts);
	}
	else if (strcasecmp(name, "aliases") == 0) {
	    startAliases(data, atts);
	}
	else if (strcasecmp(name, "import") == 0) {
	    startImport(data, atts);
	}
	else {
	    currFile->skipDepth = 1;
	}
    }
}


/*
 * Handles the end of an element.
 */
static void
endElement(
    void*		data,
    const XML_Char*	name)
{
    if (currFile->skipDepth != 0) {
	--currFile->skipDepth;
    }
    else {
        if (strcasecmp(name, "unit-system") == 0) {
            endUnitSystem(data);
        }
        else if (strcasecmp(name, "prefix") == 0) {
	    endPrefix(data);
	}
	else if (strcasecmp(name, "unit") == 0) {
	    endUnit(data);
	}
	else if (strcasecmp(name, "base") == 0) {
	    endBase(data);
	}
	else if (strcasecmp(name, "dimensionless") == 0) {
	    endDimensionless(data);
	}
	else if (strcasecmp(name, "def") == 0) {
	    endDef(data);
	}
	else if (strcasecmp(name, "value") == 0) {
	    endValue(data);
	}
	else if (strcasecmp(name, "name") == 0) {
	    endName(data);
	}
	else if (strcasecmp(name, "singular") == 0) {
	    endSingular(data);
	}
	else if (strcasecmp(name, "plural") == 0) {
	    endPlural(data);
	}
	else if (strcasecmp(name, "symbol") == 0) {
	    endSymbol(data);
	}
	else if (strcasecmp(name, "aliases") == 0) {
	    endAliases(data);
	}
	else if (strcasecmp(name, "import") == 0) {
	    endImport(data);
	}
	else {
            ut_set_status(UT_PARSE);
	    ut_handle_error_message("Unknown element \"<%s>\"", name);
	    XML_StopParser(currFile->parser, 0);
	}
    }

    IGNORE_TEXT;
}


/*
 * Handles the header of an XML file.
 */
static void
declareXml(
    void*	data,
    const char*	version,
    const char*	encoding,
    int		standalone)
{
    if (strcasecmp(encoding, "US-ASCII") == 0) {
	currFile->xmlEncoding = UT_ASCII;
    }
    else if (strcasecmp(encoding, "ISO-8859-1") == 0) {
	currFile->xmlEncoding = UT_LATIN1;
    }
    else if (strcasecmp(encoding, "UTF-8") == 0) {
	currFile->xmlEncoding = UT_UTF8;
    }
    else {
        ut_set_status(UT_PARSE);
	ut_handle_error_message("Unknown XML encoding \"%s\"", encoding);
	XML_StopParser(currFile->parser, 0);
    }
}


/*
 * Reads an XML file into the unit-system with the given XML parser.
 *
 * Arguments:
 *      parser          Pointer to the XML parser.
 *      path            Pointer to the pathname of the XML file.
 * Returns:
 *      UT_SUCCESS      Success.
 *      UT_OPEN_ARG     File "path" couldn't be opened.  See "errno".
 *      UT_OS           Operating-system error.  See "errno".
 *      UT_PARSE        Parse failure.
 */
static ut_status
readXmlWithParser(
    XML_Parser          parser,
    const char* const   path)
{
    ut_status   status = UT_SUCCESS;    /* success */
    File        file;

    assert(parser != NULL);
    assert(path != NULL);

    fileInit(&file);

    file.fd = open(path, O_RDONLY);

    if (file.fd == -1) {
        status = UT_OPEN_ARG;
        ut_set_status(status);
        ut_handle_error_message(strerror(errno));
        ut_handle_error_message("Couldn't open file \"%s\"", path);
    }
    else {
        int	        nbytes;
        File* const     prevFile = currFile;

        file.path = path;
        file.parser = parser;
        currFile = &file;

        do {
            char	buf[BUFSIZ];	/* from <stdio.h> */

            nbytes = read(file.fd, buf, sizeof(buf));

            if (nbytes < 0) {
                status = UT_OS;
                ut_set_status(status);
                ut_handle_error_message(strerror(errno));
            }
            else {
                if (XML_Parse(file.parser, buf, nbytes, nbytes == 0)
                        != XML_STATUS_OK) {
                    status = UT_PARSE;
                    ut_set_status(status);
                    ut_handle_error_message(
                        XML_ErrorString(XML_GetErrorCode(file.parser)));
                }
            }
        } while (status == UT_SUCCESS && nbytes > 0);

        if (status != UT_SUCCESS) {
            /*
             * Parsing of the XML file terminated prematurely.
             */
            ut_handle_error_message("File \"%s\", line %d, column %d",
                path, XML_GetCurrentLineNumber(file.parser),
                XML_GetCurrentColumnNumber(file.parser));
        }

        currFile = prevFile;

        (void)close(file.fd);
        file.fd = -1;
    }                                   /* "file.fd" open */

    return status;
}


/*
 * Reads an XML file into the unit-system.
 *
 * Arguments:
 *      path            Pointer to the pathname of the XML file.
 * Returns:
 *      UT_SUCCESS      Success.
 *      UT_OPEN_ARG     File "path" couldn't be opened.  See "errno".
 *      UT_OS           Operating-system error.  See "errno".
 *      UT_PARSE        Parse failure.
 */
static ut_status
readXml(
    const char* const   path)
{
    ut_status       status;
    XML_Parser      parser = XML_ParserCreate(NULL);

    if (parser == NULL) {
        status = UT_OS;
        ut_set_status(status);
        ut_handle_error_message(strerror(errno));
        ut_handle_error_message("Couldn't create XML parser");
    }
    else {
        char        base[_POSIX_PATH_MAX];

        (void)strncpy(base, path, sizeof(base));
        base[sizeof(base)-1] = 0;
        (void)memmove(base, dirname(base), sizeof(base));
        base[sizeof(base)-1] = 0;

        if (XML_SetBase(parser, base) != XML_STATUS_OK) {
            status = UT_OS;
            ut_set_status(status);
            ut_handle_error_message(strerror(errno));
            ut_handle_error_message("XML_SetBase(\"%s\") failure", base);
        }
        else {
            XML_SetXmlDeclHandler(parser, declareXml);
            XML_SetElementHandler(parser, startElement, endElement);
            XML_SetCharacterDataHandler(parser, NULL);

            status = readXmlWithParser(parser, path);
        }                           /* parser "base" set */

        XML_ParserFree(parser);
    }                               /* parser != NULL */

    return status;
}


/*
 * Returns the unit-system corresponding to an XML file.  This is the usual way
 * that a client will obtain a unit-system.
 *
 * Arguments:
 *	path	The pathname of the XML file or NULL.  If NULL, then the
 *		pathname specified by the environment variable UDUNITS2_XML_PATH
 *		is used if set; otherwise, the compile-time pathname of the
 *		installed, default, unit database is used.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be
 *		    UT_OPEN_ARG		"path" is non-NULL but file couldn't be
 *					opened.  See "errno" for reason.
 *		    UT_OPEN_ENV		"path" is NULL and environment variable
 *					UDUNITS2_XML_PATH is set but file
 *					couldn't be opened.  See "errno" for
 *					reason.
 *		    UT_OPEN_DEFAULT	"path" is NULL, environment variable
 *					UDUNITS2_XML_PATH is unset, and the
 *					installed, default, unit database
 *					couldn't be opened.  See "errno" for
 *					reason.
 *		    UT_PARSE		Couldn't parse unit database.
 *		    UT_OS		Operating-system error.  See "errno".
 *	else	Pointer to the unit-system defined by "path".
 */
ut_system*
ut_read_xml(
    const char*	path)
{
    ut_set_status(UT_SUCCESS);

    unitSystem = ut_new_system();

    if (unitSystem == NULL) {
        ut_handle_error_message("Couldn't create new unit-system");
    }
    else {
        ut_status       status;
        ut_status       openError;

        if (path != NULL) {
            openError = UT_OPEN_ARG;
        }
        else {
            path = getenv("UDUNITS2_XML_PATH");

            if (path != NULL) {
                openError = UT_OPEN_ENV;
            }
            else {
                path = DEFAULT_UDUNITS2_XML_PATH;
                openError = UT_OPEN_DEFAULT;
            }
        }

        status = readXml(path);

        if (status == UT_OPEN_ARG) {
            status = openError;
        }
        if (status != UT_SUCCESS) {
            ut_free_system(unitSystem);
            unitSystem = NULL;
        }

        ut_set_status(status);
    }				        /* unitSystem != NULL */

    return unitSystem;
}
