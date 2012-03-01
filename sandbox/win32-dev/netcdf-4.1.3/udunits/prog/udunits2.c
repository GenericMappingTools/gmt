/*
 * Copyright 2007, 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * This program prints definitions of units of physical qantities and converts
 * values between such units.
 */

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <udunits2.h>

static int		_reveal;
static int		_encodingSet;
static ut_encoding	_encoding;
static const char*	_progname = "udunits2";
static const char*	_xmlPath;
static ut_system*	_unitSystem;
static char		_haveUnitSpec[_POSIX_MAX_INPUT+1];
static char		_wantSpec[_POSIX_MAX_INPUT+1];
static ut_unit*		_haveUnit;
static ut_unit*		_wantUnit;
static int		_wantDefinition;
static int		_formattingOptions = UT_DEFINITION;
static int		_exitStatus = EXIT_FAILURE;


static void
usage(void)
{
    (void)fprintf(stderr,
	"Usage: %s [-A|-L|-U] [-hr] [XML_file]\n"
	"\n"
	"where:\n"
	"    -A         Use ASCII encoding.\n"
	"    -L         Use ISO-8859-1 (ISO Latin-1)  encoding.\n"
	"    -U         Use UTF-8 encoding.\n"
	"    -h         Help.  Print this message.\n"
	"    -r         Reveal any problems in the database.\n"
	"    XML_file   XML database file.\n",
	_progname);
}


static int
decodeCommandLine(
    int			argc,
    char* const*	argv)
{
    int		c;
    int		success = 0;

    while ((c = getopt(argc, argv, "ALUhr")) != -1) {
	switch (c) {
	    case 'A':
		_encoding = UT_ASCII;
		_encodingSet = 1;
		continue;
	    case 'L':
		_encoding = UT_LATIN1;
		_encodingSet = 1;
		continue;
	    case 'U':
		_encoding = UT_UTF8;
		_encodingSet = 1;
		continue;
	    case 'r':
		_reveal = 1;
		continue;
	    case 'h':
		_exitStatus = EXIT_SUCCESS;
		/*FALLTHROUGH*/
	    case '?':
		usage();
		break;
	    default:
		(void)fprintf(stderr, "%s: Unknown option \"%c\"\n",
		    _progname, c);
		usage();
	}

	break;
    }

    if (c == -1) {
	_xmlPath = 
	    optind < argc
		? argv[optind]
		: NULL;

	success = 1;
    }

    return success;
}


static int
ensureXmlPathSet(void)
{
    if (_xmlPath == NULL)
	(void)fprintf(stderr, "%s: Using default XML database\n", _progname);

    return 1;
}


static void
setEncoding(
    char*	value)
{
    if (value != NULL) {
	typedef struct {
	    const char*	pattern;
	    ut_encoding	encoding;
	    regex_t		reg;
	}			Entry;

	static Entry	entries[] = {
	    {"^c$",			UT_ASCII},
	    {"^posix$",			UT_ASCII},
	    {"ascii",			UT_ASCII},
	    {"latin.?1([^0-9]|$)",	UT_LATIN1},
	    {"8859.?1([^0-9]|$)",	UT_LATIN1},
	    {"utf.?8([^0-9]|$)",	UT_UTF8},
	};
	static int		initialized = 0;
	static int		entryCount = sizeof(entries)/sizeof(entries[0]);

	if (!initialized) {
	    int		status = 0;
	    int		i;

	    for (i = 0; i < entryCount; i++) {
		Entry*	entry = entries + i;
		regex_t*	reg = &entry->reg;
		const char*	pattern = entry->pattern;

		status =
		    regcomp(reg, entries[i].pattern,
			REG_EXTENDED | REG_ICASE | REG_NOSUB);

		if (status != 0) {
		    char	buf[132];

		    (void)regerror(status, reg, buf, sizeof(buf));
		    (void)fprintf(stderr, "%s: Unable to compile regular "
			"expression \"%s\": %s\n", _progname, pattern, buf);

		    break;
		}
	    }

	    if (status == 0)
		initialized = 1;
	}

	if (initialized) {
	    int	i;
	    int	status = 0;

	    for (i = 0; i < entryCount; i++) {
		Entry*	entry = entries + i;
		regex_t*	reg = &entry->reg;

		status = regexec(reg, value, 0, NULL, 0);

		if (status == 0)
		    break;

		if (status != REG_NOMATCH) {
		    char	buf[132];

		    (void)regerror(status, reg, buf, sizeof(buf));
		    (void)fprintf(stderr,
			"%s: Unable to execute regular expression \"%s\": %s\n",
			_progname, entry->pattern, buf);

		    break;
		}
	    }

	    if (status == 0 && i < entryCount) {
		_encoding = entries[i].encoding;
		_encodingSet = 1;
	    }
	}
    }
}


static int
ensureEncodingSet()
{
    if (!_encodingSet) {
	setEncoding(getenv("LC_ALL"));

	if (!_encodingSet) {
	    setEncoding(getenv("LC_CTYPE"));

	    if (!_encodingSet) {
		setEncoding(getenv("LANG"));

		if (!_encodingSet) {
		    (void)fprintf(stderr, "%s: Character encoding not "
			"specified and not settable from environment variables "
			"LC_ALL, LC_CTYPE, or LANG.  Assuming ASCII "
			"encoding.\n", _progname);

		    setEncoding("ASCII");
		}
	    }
	}
    }

    if (_encodingSet)
	_formattingOptions |= _encoding;

    return _encodingSet;
}


static int
readXmlDatabase(void)
{
    int		success = 0;

    if (!_reveal)
	ut_set_error_message_handler(ut_ignore);

    _unitSystem = ut_read_xml(_xmlPath);

    ut_set_error_message_handler(ut_write_to_stderr);

    if (_unitSystem != NULL) {
	success = 1;
    }
    else {
	(void)fprintf(stderr, "%s: Couldn't initialize unit-system from "
	    "database \"%s\"\n", _progname, _xmlPath);
    }

    return success;
}


/*
 * Get a specification.
 */
static int
getSpec(
    const char* const	prompt,
    char* const		spec,
    const size_t	size)
{
    int		nbytes = -1;		/* failure */

    if (fputs(prompt, stdout) == EOF) {
	(void)fprintf(stderr, "%s: Couldn't write prompt: %s\n",
	    _progname, strerror(errno));
    } else if (fgets(spec, size, stdin) == NULL) {
	putchar('\n');

	if (feof(stdin)) {
	    _exitStatus = EXIT_SUCCESS;
	} else {
	    (void)fprintf(stderr, "%s: Couldn't read from standard input: %s\n",
		_progname, strerror(errno));
	}
    } else {
	/*
	 * Trim any whitespace from the specification.
	 */
	(void)ut_trim(spec, _encoding);

        nbytes = strlen(spec);
    }

    return nbytes;
}


static int
getInputValue(void)
{
    int		success = 0;

    for (;;) {
        char    buf[sizeof(_haveUnitSpec)];
	int	nbytes = getSpec("You have: ", buf, sizeof(buf));

	if (nbytes < 0)
	    break;

	if (nbytes > 0) {
	    (void)strcpy(_haveUnitSpec, buf);

	    ut_free(_haveUnit);
	    _haveUnit = ut_parse(_unitSystem, _haveUnitSpec, _encoding);

	    if (_haveUnit == NULL) {
		(void)fprintf(stderr, "%s: Don't recognize \"%s\"\n",
		    _progname, _haveUnitSpec);
	    }
	    else {
		success = 1;
		break;
	    }
	}
    }

    return success;
}


static int
getOutputRequest(void)
{
    int		success = 0;

    for (;;) {
	int	nbytes =
	    getSpec("You want: ", _wantSpec, sizeof(_wantSpec));

	if (nbytes < 0)
	    break;

	if (nbytes == 0) {
	    _wantDefinition = 1;
	    success = 1;
	    break;
	}

	_wantDefinition = 0;

	ut_free(_wantUnit);

	_wantUnit = ut_parse(_unitSystem, _wantSpec, _encoding);

	if (_wantUnit == NULL) {
	    (void)fprintf(stderr, "%s: Don't recognize \"%s\"\n",
		_progname, _wantSpec);
	}
	else {
	    success = 1;
	    break;
	}
    }

    return success;
}


static int
handleRequest(void)
{
    int		success = 0;

    if (getInputValue()) {
	if (getOutputRequest()) {
	    if (_wantDefinition) {
                char	buf[256];
                int	nbytes = ut_format(_haveUnit, buf, sizeof(buf),
                    _formattingOptions);

                if (nbytes >= sizeof(buf)) {
                    (void)fprintf(stderr, "%s: Resulting unit "
                        "specification is too long\n", _progname);
                }
                else if (nbytes >= 0) {
                    buf[nbytes] = 0;

                    (void)printf("    %s\n", buf);
                }
	    }
	    else if (!ut_are_convertible(_wantUnit, _haveUnit)) {
		(void)fprintf(stderr, "%s: Units are not convertible\n",
		    _progname);
	    }
	    else {
		cv_converter*	conv = ut_get_converter(_haveUnit, _wantUnit);

		if (conv == NULL) {
		    (void)fprintf(stderr, "%s: Couldn't get unit converter\n",
			_progname);
		}
		else {
                    char        haveExp[_POSIX_MAX_INPUT+1];
                    char        exp[_POSIX_MAX_INPUT+1];
                    char        whiteSpace[] = " \t\n\r\f\v\xa0";
		    int	        needsParens =
                        strpbrk(_wantSpec, whiteSpace) != NULL;
                    int         n;

		    (void)printf(
			needsParens
			    ? "    %s = %g (%s)\n"
			    : "    %s = %g %s\n",
			_haveUnitSpec,
			cv_convert_double(conv, 1.0), _wantSpec);

                    (void)sprintf(haveExp,
                        strpbrk(_haveUnitSpec, whiteSpace) ||
                                strpbrk(_haveUnitSpec, "/")
                            ? "(x/(%s))"
                            : "(x/%s)",
                        _haveUnitSpec);

                    n = cv_get_expression(conv, exp, sizeof(exp), haveExp);

                    if (n >= 0)
                        (void)printf(
                            strpbrk(_wantSpec, whiteSpace) ||
                                    strpbrk(_wantSpec, "/")
                                ?  "    x/(%s) = %*s\n"
                                :  "    x/%s = %*s\n",
                        _wantSpec, n, exp);

                    cv_free(conv);
		}
	    }

	    success = 1;
	}
    }

    return success;
}


int
main(
    const int		argc,
    char* const* const	argv)
{
    if (decodeCommandLine(argc, argv)) {
	if (ensureEncodingSet()) {
	    if (ensureXmlPathSet()) {
		if (readXmlDatabase()) {
		    while (handleRequest())
			;		/* EMPTY */
		}
	    }
	}
    }

    return _exitStatus;
}
