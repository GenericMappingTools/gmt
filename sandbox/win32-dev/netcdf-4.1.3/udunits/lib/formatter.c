/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * This module is thread-compatible but not thread-safe.
 */
/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "udunits2.h"
#include "unitToIdMap.h"

typedef const char*	(*IdGetter)(const ut_unit*, ut_encoding);
typedef	int		(*ProductPrinter)(const ut_unit* const*, const int*,
    int, char*, size_t, IdGetter);

/*
 * Formatting parameters:
 */
typedef struct {
    IdGetter		getId;
    ProductPrinter	printProduct;
    char*		buf;
    size_t		size;
    int			getDefinition;
    ut_encoding		encoding;
    int			addParens;
    int			nchar;
} FormatPar;

#undef ABS
#define ABS(x)			((x) < 0 ? -(x) : (x))
#define RETURNS_NAME(getId)	((getId) == getName)

static int
asciiPrintProduct(
    const ut_unit* const* const	basicUnits,
    const int* const		powers,
    const int			count,
    char* const			buf,
    const size_t		max,
    IdGetter			getId);
static int
latin1PrintProduct(
    const ut_unit* const* const	basicUnits,
    const int* const		powers,
    const int			count,
    char* const			buf,
    const size_t		max,
    IdGetter			getId);
static int
utf8PrintProduct(
    const ut_unit* const* const	basicUnits,
    const int* const		powers,
    const int			count,
    char* const			buf,
    const size_t		max,
    IdGetter			getId);

static ut_visitor	formatter;


/*
 * Returns a name for a unit.
 *
 * Arguments:
 *	unit		Pointer to the unit to have it's name returned.
 *	encoding	The encoding of the name to be returned.
 * Returns:
 *	NULL		A name is not available in the desired encoding.
 *	else		Pointer to the name.
 */
static const char*
getName(
    const ut_unit* const	unit,
    const ut_encoding	encoding)
{
    const char*	name;

    name = ut_get_name(unit, encoding);

    if (name == NULL)
	name = ut_get_name(unit, UT_ASCII);

    return name;
}


/*
 * Returns a symbol for a unit.
 *
 * Arguments:
 *	unit		Pointer to the unit to have it's symbol returned.
 *	encoding	The encoding of the symbol to be returned.
 * Returns:
 *	NULL		A symbol is not available in the desired encoding.
 *	else		Pointer to the symbol.
 */
static const char*
getSymbol(
    const ut_unit* const	unit,
    const ut_encoding	encoding)
{
    const char*	symbol;

    symbol = ut_get_symbol(unit, encoding);

    if (symbol == NULL)
	symbol = ut_get_symbol(unit, UT_ASCII);

    return symbol;
}


/*
 * Formats a unit.
 *
 * Arguments:
 *	unit		Pointer to the unit to be formatted.
 *	buf		Pointer to the buffer into which to print the formatted
 *			unit.
 *	size		Size of the buffer.
 *	useNames	Use unit names rather than unit symbols.
 *	getDefinition	Returns the definition of "unit" in terms of basic
 *			units.
 *	encoding	The type of encoding to use.
 *	addParens	Whether or not to add bracketing parentheses if
 *			whitespace is printed.
 * Returns:
 *	-1	Failure:  "utFormStatus()" will be
 *		    UT_BAD_ARG	"unit" is NULL or "buf" is NULL.
 *	else	Number of characters printed in "buf".
 */
static int
format(
    const ut_unit* const	unit,
    char*		        buf,
    size_t		        size,
    const int		        useNames,
    const int		        getDefinition,
    ut_encoding		        encoding,
    const int		        addParens)
{
    int	nchar = -1;	/* failure */

    if (unit == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("format(): NULL unit argument");
    }
    else if (buf == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("format(): NULL buffer argument");
    }
    else {
	FormatPar	formatPar;

	formatPar.buf = buf;
	formatPar.size = size;
	formatPar.getId = useNames ? getName : getSymbol;
	formatPar.getDefinition = getDefinition;
	formatPar.encoding = encoding;
	formatPar.printProduct =
	    encoding == UT_ASCII
		? asciiPrintProduct
		: encoding == UT_LATIN1
		    ? latin1PrintProduct
		    : utf8PrintProduct;
	formatPar.addParens = addParens;
	formatPar.nchar = 0;

	if (ut_accept_visitor(unit, &formatter, &formatPar) == UT_SUCCESS)
	    nchar = formatPar.nchar;
    }

    return nchar;
}


/*******************************************************************************
 * Basic-Unit Formatting:
 ******************************************************************************/

/*
 * Prints a basic-unit.
 *
 * Arguments:
 *	unit		The basic-unit to be printed.
 *	buf		The buffer into which to print "unit".
 *	max		The size of "buf".
 * Returns:
 *	-1		Failure.  The identifier for "unit" could not be
 *			obtained.
 *	else		Success.  Number of characters printed, excluding any
 *			trailing NUL.
 */
static int
printBasic(
    const ut_unit* const	unit,
    char* const		buf,
    size_t		max,
    IdGetter		getId,
    ut_encoding		encoding)
{
    const char* const	id = getId(unit, encoding);

    return
	id == NULL
	    ? -1
	    : snprintf(buf, max, "%s", id);
}


/*
 * Formats a basic-unit.
 *
 * Arguments:
 *	unit		The basic-unit to be formatted.
 *	arg		The formatting parameters.
 * Returns:
 *	-1		Failure.  The identifier for "unit" could not be
 *			obtained.
 *	else		Success.  Number of characters formatted, excluding any
 *			trailing NUL.
 */
static ut_status
formatBasic(
    const ut_unit* const	unit,
    void*		arg)
{
    FormatPar*	formatPar = (FormatPar*)arg;
    int		nchar = printBasic(unit, formatPar->buf, formatPar->size,
	formatPar->getId, formatPar->encoding);

    formatPar->nchar = nchar < 0 ? nchar : formatPar->nchar + nchar;

    return nchar < 0 ? UT_VISIT_ERROR : UT_SUCCESS;
}


/*******************************************************************************
 * Product-Unit Formatting:
 ******************************************************************************/

/*
 * Prints a product-unit using the ASCII character-set.
 *
 * Arguments:
 *	basicUnits	Pointer to pointers to the basic-units that constitute
 *			the product-unit.
 *	powers		Pointer to the powers associated with each basic-unit.
 *	count		The number of basic-units.
 *	buf		Pointer to the buffer into which to print the basic-
 *			units.
 *	max		The size of "buf" in bytes.
 *	getId		Returns the identifier for a unit.
 * Returns:
 *	-1		Failure.  See errno.
 *	else		Success.  Number of bytes printed.
 */
static int
asciiPrintProduct(
    const ut_unit* const* const	basicUnits,
    const int* const		powers,
    const int			count,
    char* const			buf,
    const size_t		max,
    IdGetter			getId)
{
    int		nchar = snprintf(buf, max, "%s", "");

    if (nchar >= 0) {
	int	i;

	for (i = 0; i < count; i++) {
	    int	n;

	    /*
	     * Append separator if appropriate.
	     */
	    if (nchar > 0) {
		n = RETURNS_NAME(getId)
		    ? snprintf(buf+nchar, max-nchar, "%s", "-")
		    : snprintf(buf+nchar, max-nchar, "%s", ".");

		if (n < 0) {
		    nchar = n;
		    break;
		}

		nchar += n;
	    }

	    /*
	     * Append unit identifier.
	     */
	    n = printBasic(basicUnits[i], buf+nchar, max-nchar, getId,
		UT_ASCII);

	    if (n < 0) {
		nchar = n;
		break;
	    }

	    nchar += n;

	    /*
	     * Append exponent if appropriate.
	     */
	    if (powers[i] != 1) {
		n = RETURNS_NAME(getId)
		    ? snprintf(buf+nchar, max-nchar, "^%d", powers[i])
		    : snprintf(buf+nchar, max-nchar, "%d", powers[i]);

		if (n < 0) {
		    nchar = n;
		    break;
		}

		nchar += n;
	    }
	}				/* loop over basic-units */
    }					/* "buf" initialized */

    return nchar;
}


/*
 * Prints a product of basic-units using the UTF-8 character-set.
 *
 * Arguments:
 *	basicUnits	Pointer to pointers to the basic-units whose product
 *			is to be printed.
 *	powers		Pointer to the powers associated with each basic-unit.
 *	count		The number of basic-units.
 *	buf		Pointer to the buffer into which to print the basic-
 *			units.
 *	max		The size of "buf" in bytes.
 *	getId		Returns the identifier for a unit.
 * Returns:
 *	-1		Failure.  See errno.
 *	else		Success.  Number of bytes printed.
 */
static int
utf8PrintProduct(
    const ut_unit* const* const	basicUnits,
    const int* const		powers,
    const int			count,
    char* const			buf,
    const size_t		max,
    IdGetter			getId)
{
    int		nchar = snprintf(buf, max, "%s", "");

    if (nchar >= 0) {
	int	iBasic;

	for (iBasic = 0; iBasic < count; iBasic++) {
	    int	power = powers[iBasic];

	    if (power != 0) {
		/*
		 * The current basic-unit must be printed.
		 */
		int	n;

		if (nchar > 0) {
		    /*
		     * Append mid-dot separator.
		     */
		    n = snprintf(buf+nchar, max-nchar, "%s", "\xc2\xb7");

		    if (n < 0) {
			nchar = n;
			break;
		    }

		    nchar += n;
		}

		/*
		 * Append unit identifier.
		 */
		n = printBasic(basicUnits[iBasic], buf+nchar, max-nchar,
		    getId, UT_UTF8);

		if (n < 0) {
		    nchar = n;
		    break;
		}

		nchar += n;

		if (power != 1) {
		    /*
		     * Append exponent.
		     */
		    static const char*	exponentStrings[10] = {
			"\xe2\x81\xb0",	/* 0 */
			"\xc2\xb9",	/* 1 */
			"\xc2\xb2",	/* 2 */
			"\xc2\xb3",	/* 3 */
			"\xe2\x81\xb4",	/* 4 */
			"\xe2\x81\xb5",	/* 5 */
			"\xe2\x81\xb6",	/* 6 */
			"\xe2\x81\xb7",	/* 7 */
			"\xe2\x81\xb8",	/* 8 */
			"\xe2\x81\xb9",	/* 9 */
		    };

		    if (power < 0) {
			/*
			 * Append superscript minus sign.
			 */
			n = snprintf(buf+nchar, max-nchar, "%s",
                            "\xe2\x81\xbb");

			if (n < 0) {
			    nchar = n;
			    break;
			}

			nchar += n;
			power = -power;
		    }

		    /*
		     * Append UTF-8 encoding of exponent magnitude.
		     */
		    {
			static int*	digit = NULL;

			digit = realloc(digit, (size_t)((sizeof(powers[0])*
				    CHAR_BIT*(M_LOG10E/M_LOG2E)) + 1));

			if (digit == NULL) {
			    nchar = -1;
			}
			else {
			    int	idig = 0;

			    for (; power > 0; power /= 10)
				digit[idig++] = power % 10;

			    while (idig-- > 0) {
				n = snprintf(buf+nchar, max-nchar, "%s",
					exponentStrings[digit[idig]]);

				if (n < 0) {
				    nchar = n;
				    break;
				}

				nchar += n;
			    }

			    if (nchar < 0)
				break;
			}
		    }			/* exponent digits block */
		}			/* must print exponent */
	    }				/* must print basic-unit */
	}				/* loop over basic-units */
    }					/* "buf" initialized */

    return nchar;
}


static const int*	globalPowers = NULL;


static int
compareExponents(
    const void*	i,
    const void*	j)
{
    return globalPowers[*(const int*)j] - globalPowers[*(const int*)i];
}


/*
 * Returns the order of basic-unit powers in decreasing order.
 *
 * Arguments:
 *	powers		Pointer to the powers of the basic-units.
 *	count		The number of powers.
 *	positiveCount	Pointer to pointer to the number of positive powers.
 *			Set on and only on success.
 *	negativeCount	Pointer to pointer to the number of negative powers.
 *			Set on and only on success.
 * Returns:
 *	NULL		Failure.  See errno.
 *	else		Success.  Pointer to indexes of "powers" in decreasing
 *			order.
 */
static void
getBasicOrder(
    const int* const	powers,
    const int		count,
    int* const		order,
    int* const		positiveCount,
    int* const		negativeCount)
{
    int		nNeg = 0;
    int		nPos = 0;
    int		n = 0;
    int		i;

    for (i = 0; i < count; i++) {
	if (powers[i] < 0) {
	    ++nNeg;
	    order[n++] = i;
	}
	else if (powers[i] > 0) {
	    ++nPos;
	    order[n++] = i;
	}
    }

    *negativeCount = nNeg;
    *positiveCount = nPos;
    globalPowers = powers;

    qsort(order, n, sizeof(int), compareExponents);
}


/*
 * Prints the product of a set of basic-units using the ISO-8859-1 (Latin-1)
 * character-set.
 *
 * Arguments:
 *	buf		Pointer to the buffer into which to print the basic-
 *			units.
 *	max		The size of "buf" in bytes.
 *	basicUnits	Pointer to pointers to the basic-units.
 *	powers		Pointer to the powers associated with each basic-unit.
 *	order		Pointer to indexes of "powers".  "order[i]" is the
 *			index of "basicUnits" and "powers" for the "i"th
 *			position.
 *	count		The number of basic-units.
 *	getId		Returns the identifier for a unit.
 * Returns:
 *	-1		Failure.  See errno.
 *	else		Success.  Number of bytes printed.
 */
static int
latin1PrintBasics(
    char* const			buf,
    size_t			max,
    const ut_unit* const*	basicUnits,
    const int* const		powers,
    const int* const		order,
    const int			count,
    IdGetter			getId)
{
    int	needSeparator = 0;
    int	nchar = 0;
    int	i;

    for (i = 0; i < count; i++) {
	int	n;
	int	j = order[i];
	int	power = ABS(powers[j]);

	if (power != 0) {
	    if (needSeparator) {
		n = snprintf(buf+nchar, max-nchar, "%s", "·");	/* 0xb7 */

		if (n < 0) {
		    nchar = n;
		    break;
		}

		nchar += n;
	    }

	    /*
	     * Append unit identifier.
	     */
	    n = printBasic(basicUnits[j], buf+nchar, max-nchar, getId,
		UT_LATIN1);

	    if (n < 0) {
		nchar = n;
		break;
	    }

	    nchar += n;
	    needSeparator = 1;

	    /*
	     * Append exponent if appropriate.
	     */
	    if (power != 1) {
		n = snprintf(buf+nchar, max-nchar, "%s",
		    power == 2 ? "²" : "³");	/* 0xb2 0xb3 */

		if (n < 0) {
		    nchar = n;
		    break;
		}

		nchar += n;
	    }
	}		/* exponent not zero */
    }			/* loop over positive exponents */

    return nchar;
}


/*
 * Prints a product-unit using the ISO-8859-1 (Latin-1) character-set.
 *
 * Arguments:
 *	basicUnits	Pointer to pointers to the basic-units that constitute
 *			the product-unit.
 *	powers		Pointer to the powers associated with each basic-unit.
 *	count		The number of basic-units.
 *	buf		Pointer to the buffer into which to print the basic-
 *			units.
 *	max		The size of "buf" in bytes.
 *	getId		Returns the identifier for a unit.
 * Returns:
 *	-1		Failure.  See errno.
 *	else		Success.  Number of bytes printed.
 */
static int
latin1PrintProduct(
    const ut_unit* const* const	basicUnits,
    const int* const		powers,
    const int			count,
    char* const			buf,
    const size_t		max,
    IdGetter			getId)
{
    int				nchar;
    int				i;

    for (i = 0; i < count; i++)
	if (powers[i] < -3 || powers[i] > 3)
	    break;

    if (i < count) {
	/*
	 * At least one exponent can't be represented in ISO 8859-1.  Use
	 * the ASCII encoding instead.
	 */
	nchar = asciiPrintProduct(basicUnits, powers, count, buf, max, getId);
    }
    else {
	int		positiveCount;
	int		negativeCount;
	int*		order = malloc(count*sizeof(int));

	if (order == NULL) {
	    nchar = -1;
	}
	else {
	    getBasicOrder(powers, count, order, &positiveCount, &negativeCount);

	    nchar = snprintf(buf, max, "%s", "");

	    if (nchar >= 0 && (positiveCount + negativeCount > 0)) {
		int		n;

		if (positiveCount == 0) {
		    n = snprintf(buf+nchar, max-nchar, "%s", "1");
		    nchar = n < 0 ? n : nchar + n;
		}
		else {
		    n = latin1PrintBasics(buf+nchar, max-nchar, basicUnits,
			powers, order, positiveCount, getId);
		    nchar = n < 0 ? n : nchar + n;
		}

		if (nchar >= 0 && negativeCount > 0) {
		    n = snprintf(buf+nchar, max-nchar, "%s", 
			negativeCount == 1 ? "/" : "/(");
		    nchar = n < 0 ? n : nchar + n;

		    if (nchar >= 0) {
			n = latin1PrintBasics(buf+nchar, max-nchar, basicUnits,
			    powers, order+positiveCount, negativeCount, getId);
			nchar = n < 0 ? n : nchar + n;

			if (nchar >= 0 && negativeCount > 1) {
			    n = snprintf(buf+nchar, max-nchar, "%s", ")");
			    nchar = n < 0 ? n : nchar + n;
			}
		    }			/* solidus appended */
		}			/* positive exponents printed */
	    }				/* "buf" initialized */

	    (void)free(order);
	}				/* "order" allocated */
    }					/* using Latin-1 encoding */

    return nchar;
}


/*
 * Prints a product-unit.
 *
 * Arguments:
 *	unit		Pointer to the product-unit to be formatted.
 *	count		The number of basic-units that constitute the 
 *			product-unit.
 *	basicUnits	Pointer to pointers to the basic-units that constitute
 *			the product-unit.
 *	powers		Pointer to the powers associated with each basic-unit
 *			of "basicUnits".
 *	arg		The formatting parameters.
 * Returns:
 *	-1		Failure.  See errno.
 *	else		Success.  Number of bytes printed.
 */
static ut_status
formatProduct(
    const ut_unit* const	unit,
    const int			count,
    const ut_unit* const* const	basicUnits,
    const int* const		powers,
    void*			arg)
{
    FormatPar*	formatPar = (FormatPar*)arg;
    int		nchar;

    if (ut_compare(unit,
	    ut_get_dimensionless_unit_one(ut_get_system(unit))) == 0) {
	/*
	 * The dimensionless unit one is special.
	 */
	(void)strncpy(formatPar->buf, "1", formatPar->size);
	nchar = formatPar->size > 0 ? 1 : 0;
    }
    else {
	if (formatPar->getDefinition) {
	    nchar = formatPar->printProduct(basicUnits, powers, count,
		formatPar->buf, formatPar->size, formatPar->getId);
	}
	else {
	    const char*	id = formatPar->getId(unit, formatPar->encoding);

	    nchar = 
		id == NULL
		    ? formatPar->printProduct(basicUnits, powers, count,
			formatPar->buf, formatPar->size, formatPar->getId)
		    : snprintf(formatPar->buf, formatPar->size, "%s", id);
	}
    }
    formatPar->nchar = nchar < 0 ? nchar : formatPar->nchar + nchar;

    return nchar < 0 ? UT_VISIT_ERROR : UT_SUCCESS;
}


/*******************************************************************************
 * Galilean-Unit Formatting:
 ******************************************************************************/

/*
 * Prints a Galilean-unit.
 *
 * Arguments:
 *	scale		The number of "unit"s in the Galilean-unit.
 *	unit		Pointer to the unit underlying the Galilean-unit.
 *	offset		The offset of the Galilean-unit in units of "unit".
 *	buf		Pointer to the buffer into which to print the Galilean-
 *			unit.
 *	max		The size of "buf" in bytes.
 *	getId		Returns the identifier for a unit.
 *	getDefinition	Returns the definition of "unit" in terms of basic
 *			units.
 *	encoding	The type of encoding to use.
 *	addParens	Whether or not to add bracketing parentheses if 
 *			whitespace is printed.
 * Returns:
 *	-1		Failure.  See errno.
 *	else		Success.  Number of bytes printed.
 */
static int
printGalilean(
    double		scale,
    const ut_unit* const	unit,
    double		offset,
    char* const		buf,
    const size_t	max,
    IdGetter		getId,
    const int		getDefinition,
    const ut_encoding	encoding,
    const int		addParens)
{
    int			n;
    int			nchar = 0;
    int			needParens = 0;

    if (scale != 1) {
	needParens = addParens;
	n = snprintf(buf, max, needParens ? "(%.*g " : "%.*g ", DBL_DIG, scale);
	nchar = n < 0 ? n : nchar + n;
    }

    if (nchar >= 0) {
	n = format(unit, buf+nchar, max-nchar, RETURNS_NAME(getId),
	    getDefinition, encoding, 1);

	if (n < 0) {
	    nchar = n;
	}
	else {
	    nchar += n;

	    if (offset != 0) {
		needParens = addParens;
		n = RETURNS_NAME(getId)
		    ? snprintf(buf+nchar, max-nchar, " from %.*g", DBL_DIG,
			offset)
		    : snprintf(buf+nchar, max-nchar, " @ %.*g", DBL_DIG,
			offset);
		nchar = n < 0 ? n : nchar + n;
	    }				/* non-zero offset */

	    if (nchar >= 0) {
		if (needParens) {
		    n = snprintf(buf+nchar, max-nchar, "%s", ")");
		    nchar = n < 0 ? n : nchar + n;
		}
	    }				/* printed offset if appropriate */
	}				/* underlying unit printed */
    }					/* scale printed if appropriate */

    return nchar;
}


/*
 * Formats a Galilean-unit.
 *
 * Arguments:
 *	unit		Pointer to the Galilean-unit to be formatted.
 *	scale		The number of "underlyingUnit"s in "unit".
 *	underlyingUnit	Pointer to the unit that underlies "unit".
 *	offset		The offset of "unit" in units of "underlyingUnit".
 *	arg		Pointer to the formatting parameters.
 * Returns:
 *	-1		Failure.  See errno.
 *	else		Success.  Number of bytes printed.
 */
static ut_status
formatGalilean(
    const ut_unit* const	unit,
    const double	scale,
    const ut_unit* const	underlyingUnit,
    double		offset,
    void*		arg)
{
    FormatPar*	formatPar = (FormatPar*)arg;
    int		nchar;

    if (formatPar->getDefinition) {
	nchar = printGalilean(scale, underlyingUnit, offset, formatPar->buf,
	    formatPar->size, formatPar->getId, formatPar->getDefinition,
	    formatPar->encoding, formatPar->addParens);
    }
    else {
	const char*	id = formatPar->getId(unit, formatPar->encoding);

	nchar = 
	    id == NULL
		? printGalilean(scale, underlyingUnit, offset, formatPar->buf,
		    formatPar->size, formatPar->getId, formatPar->getDefinition,
		    formatPar->encoding, formatPar->addParens)
		: snprintf(formatPar->buf, formatPar->size, "%s", id);
    }

    formatPar->nchar = nchar < 0 ? nchar : formatPar->nchar + nchar;

    return nchar < 0 ? UT_VISIT_ERROR : UT_SUCCESS;
}


/*******************************************************************************
 * Timestamp-Unit Formatting:
 ******************************************************************************/

/*
 * Prints a timestamp-unit.
 *
 * Arguments:
 *	underlyingUnit	Pointer to the unit underlying the timestamp-unit.
 *	year		The UTC year of the origin.
 *	month		The UTC month of the origin (1 through 12).
 *	day		The UTC day of the origin (1 through 32).
 *	hour		The UTC hour of the origin (0 through 23).
 *	minute		The UTC minute of the origin (0 through 59).
 *	second		The UTC second of the origin (0 through 60).
 *	resolution	The resolution of the origin in seconds.
 *	buf		Pointer to the buffer into which to print the
 *			timestamp-unit.
 *	max		The size of "buf" in bytes.
 *	getId		Returns the identifier for a unit.
 *	getDefinition	Returns the definition of "unit" in terms of basic
 *			units.
 *	encoding	The type of encoding to use.
 *	addParens	Whether or not to add bracketing parentheses if 
 *			whitespace is printed.
 * Returns:
 *	-1		Failure.  See errno.
 *	else		Success.  Number of bytes printed.
 */
static int
printTimestamp(
    const ut_unit* const	underlyingUnit,
    const int		year,
    const int		month,
    const int		day,
    const int		hour,
    const int		minute,
    const double	second,
    const double	resolution,
    char* const		buf,
    const size_t	max,
    IdGetter		getId,
    const int		getDefinition,
    const ut_encoding	encoding,
    const int		addParens)
{
    int		n;
    int		nchar = 0;

    if (addParens) {
	n = snprintf(buf, max, "%s", "(");
	nchar = n < 0 ? n : nchar + n;
    }

    if (nchar >= 0) {
	int	useNames = RETURNS_NAME(getId);

	n = format(underlyingUnit, buf+nchar, max-nchar, useNames,
	    getDefinition, encoding, 1);
	nchar = n < 0 ? n : nchar + n;

	if (nchar >= 0) {
	    int	useSeparators = useNames || year < 1000 || year > 9999;

	    n =  snprintf(buf+nchar, max-nchar,
		useSeparators
		    ? " %s %d-%02d-%02d %02d:%02d"
		    : " %s %d%02d%02dT%02d%02d",
		useNames ? "since" : "@",
		year, month, day, hour, minute);
	    nchar = n < 0 ? n : nchar + n;

	    if (nchar >= 0) {
		int	decimalCount = -(int)floor(log10(resolution));

		if (decimalCount > -2) {
		    n = snprintf(buf+nchar, max-nchar, 
			    useSeparators ? ":%0*.*f" : "%0*.*f",
			    decimalCount+3, decimalCount, second);
		    nchar = n < 0 ? n : nchar + n;
		}			/* sufficient precision for seconds */

		if (nchar >= 0) {
		    n = snprintf(buf+nchar, max-nchar, "%s",
			addParens ? " UTC)" : " UTC");
		    nchar = n < 0 ? n : nchar + n;
		}			/* printed seconds if appropriate */
	    }				/* printed year through minute */
	}				/* underlying unit printed */
    }					/* leading "(" printed if appropriate */

    return nchar;
}


/*
 * Formats a timestamp-unit.
 *
 * Arguments:
 *	unit		Pointer to the timestamp-unit to be formatted.
 *	underlyingUnit	Pointer to the unit that underlies "unit".
 *      origin          The encoded origin of the timestamp-unit.
 *	arg		Pointer to the formatting parameters.
 * Returns:
 *	-1		Failure.  See errno.
 *	else		Success.  Number of bytes printed.
 */
static ut_status
formatTimestamp(
    const ut_unit* const	unit,
    const ut_unit* const	underlyingUnit,
    const double		origin,
    void*			arg)
{
    FormatPar*  	formatPar = (FormatPar*)arg;
    int 		nchar;
    int		        year;
    int		        month;
    int		        day;
    int		        hour;
    int		        minute;
    double      	second;
    double              resolution;

    ut_decode_time(origin, &year, &month, &day, &hour, &minute, &second,
        &resolution);

    if (formatPar->getDefinition) {
	nchar = printTimestamp(underlyingUnit, year, month, day, hour, minute,
	    second, resolution, formatPar->buf, formatPar->size,
	    formatPar->getId, formatPar->getDefinition, formatPar->encoding,
	    formatPar->addParens);
    }
    else {
	const char*	id = formatPar->getId(unit, formatPar->encoding);

	nchar = 
	    id == NULL
		? printTimestamp(underlyingUnit, year, month, day, hour, minute,
		    second, resolution, formatPar->buf, formatPar->size,
		    formatPar->getId, formatPar->getDefinition,
		    formatPar->encoding, formatPar->addParens)
		: snprintf(formatPar->buf, formatPar->size, "%s", id);
    }

    formatPar->nchar = nchar < 0 ? nchar : formatPar->nchar + nchar;

    return nchar < 0 ? UT_VISIT_ERROR : UT_SUCCESS;
}


/*******************************************************************************
 * Logarithmic-Unit Formatting:
 ******************************************************************************/

/*
 * Prints a logarithmic-unit.
 *
 * Arguments:
 *      base            The base of the logarithm (e.g., 2, M_E, 10).
 *	reference	Pointer to the reference-level of the logarithmic-unit.
 *	buf		Pointer to the buffer into which to print the
 *			logarithmic-unit.
 *	max		The size of "buf" in bytes.
 *	getId		Returns the identifier for a unit.
 *	getDefinition	Returns the definition of "unit" in terms of basic
 *			units.
 *	encoding	The type of encoding to use.
 *	addParens	Whether or not to add bracketing parentheses if 
 *			whitespace is printed.
 * Returns:
 *	-1		Failure.  See errno.
 *	else		Success.  Number of bytes printed.
 */
static int
printLogarithmic(
    const double	base,
    const ut_unit* const	reference,
    char*		buf,
    const size_t	max,
    IdGetter		getId,
    const int		getDefinition,
    const ut_encoding	encoding,
    const int		addParens)
{
    char	refSpec[512];
    int		nchar = format(reference, refSpec, sizeof(refSpec)-1,
	RETURNS_NAME(getId), getDefinition, encoding, 0);

    if (nchar >= 0) {
	const char*	amount;

	refSpec[nchar] = 0;
	amount = isalpha(refSpec[0]) ? "1 " : "";

	if (base == 2) {
	    nchar = snprintf(buf, max, "lb(re %s%s)", amount, refSpec);
	}
	else if (base == M_E) {
	    nchar = snprintf(buf, max, "ln(re %s%s)", amount, refSpec);
	}
	else if (base == 10) {
	    nchar = snprintf(buf, max, "lg(re %s%s)", amount, refSpec);
	}
	else {
	    nchar = snprintf(buf, max,
		addParens ? "(%.*g ln(re %s%s))" : "%.*g ln(re %s%s)",
		DBL_DIG, 1/log(base), amount, refSpec);
	}
    }					/* printed reference unit */

    return nchar;
}


/*
 * Formats a logarithmic-unit.
 *
 * Arguments:
 *	unit		Pointer to the logarithmic-unit to be formatted.
 *      base            The base of the logarithm (e.g., 2, M_E, 10).
 *	reference	Pointer to the reference-level of "unit".
 *	arg		Pointer to the formatting parameters.
 * Returns:
 *	UT_VISIT_ERROR	Failure.
 *	UT_SUCCESS	Success.
 */
static ut_status
formatLogarithmic(
    const ut_unit* const	unit,
    const double	base,
    const ut_unit* const	reference,
    void*		arg)
{
    FormatPar*	formatPar = (FormatPar*)arg;
    int		nchar;

    if (formatPar->getDefinition) {
	nchar = printLogarithmic(base, reference, formatPar->buf,
	    formatPar->size, formatPar->getId, formatPar->getDefinition,
	    formatPar->encoding, formatPar->addParens);
    }
    else {
	const char*	id = formatPar->getId(unit, formatPar->encoding);

	nchar = 
	    id == NULL
		? printLogarithmic(base, reference, formatPar->buf,
		    formatPar->size, formatPar->getId, formatPar->getDefinition,
		    formatPar->encoding, formatPar->addParens)
		: snprintf(formatPar->buf, formatPar->size, "%s", id);
    }

    formatPar->nchar = nchar < 0 ? nchar : formatPar->nchar + nchar;

    return nchar < 0 ? UT_VISIT_ERROR : UT_SUCCESS;
}


/*******************************************************************************
 * This module as a unit-visitor:
 ******************************************************************************/

static ut_visitor	formatter = {
    formatBasic,
    formatProduct,
    formatGalilean,
    formatTimestamp,
    formatLogarithmic
};



/******************************************************************************
 * Public API:
 ******************************************************************************/

/*
 * Formats a unit.
 *
 * Arguments:
 *	unit		Pointer to the unit to be formatted.
 *	buf		Pointer to the buffer into which to format "unit".
 *	size		Size of the buffer in bytes.
 *	opts		Formatting options: bitwise-OR of zero or more of the
 *			following:
 *			    UT_NAMES		Use unit names instead of
 *						symbols
 *                          UT_DEFINITION       The formatted string should be
 *                                              the definition of "unit" in
 *                                              terms of basic-units instead of
 *						stopping any expansion at the
 *						highest level possible.
 *			    UT_ASCII		The string should be formatted
 *						using the ASCII character set
 *						(default).
 *			    UT_LATIN1		The string should be formatted
 *						using the ISO Latin-1 (alias
 *						ISO-8859-1) character set.
 *			    UT_UTF8		The string should be formatted
 *						using the UTF-8 character set.
 *			UT_LATIN1 and UT_UTF8 are mutually exclusive: they may
 *			not both be specified.
 * Returns:
 *	-1		Failure:  "ut_get_status()" will be
 *			    UT_BAD_ARG		"unit" or "buf" is NULL, or both
 *                                              UT_LATIN1 and UT_UTF8 specified.
 *			    UT_CANT_FORMAT	"unit" can't be formatted in
 *						the desired manner.
 *      else		Success.  Number of characters printed in "buf".  If
 *			the number is equal to the size of the buffer, then the
 *			buffer is too small to have a terminating NUL character.
 */
int
ut_format(
    const ut_unit* const	unit,
    char*		        buf,
    size_t		        size,
    unsigned		        opts)
{
    int			nchar = -1;	/* failure */
    const int		useNames = opts & UT_NAMES;
    const int		getDefinition = opts & UT_DEFINITION;
    const ut_encoding	encoding =
        (ut_encoding)(opts & (unsigned)(UT_ASCII | UT_LATIN1 | UT_UTF8));

    if (unit == NULL || buf == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("NULL argument");
    }
    else if ((encoding & UT_LATIN1) && (encoding & UT_UTF8)) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("Both UT_LATIN1 and UT_UTF8 specified");
    }
    else {
	nchar = format(unit, buf, size, useNames, getDefinition, encoding, 0);

	if (nchar < 0) {
	    ut_set_status(UT_CANT_FORMAT);
	    ut_handle_error_message("Couldn't format unit");
	}
	else {
	    ut_set_status(UT_SUCCESS);
	}
    }

    return nchar;
}
