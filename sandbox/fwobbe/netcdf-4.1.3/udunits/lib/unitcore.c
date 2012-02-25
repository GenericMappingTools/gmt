/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * Unit creation and manipulation routines for the udunits(3) library.
 *
 * The following data-structures exist in this module:
 *	BasicUnit	Like an ISO "base unit" but also for dimensionless
 *			units (e.g., "radian").
 *	ProductUnit	A unit that, when it is created, contains all the
 *			BasicUnit-s that exist at the time, each raised
 *			to an integral power (that can be zero).
 *	GalileanUnit	A unit whose value is related to another unit by a 
 *			Galilean transformation (y = ax + b).  Examples include
 *			"yard" and "degrees Fahrenheit".
 *	LogUnit		A unit that is related to another unit by a logarithmic
 *			transformation (y = a*log(x)).  The "Bel" is an example.
 *	TimestampUnit	A wrong-headed unit that shouldn't exist but does for
 *			backward compatibility.  It was intended to provide 
 *			similar functionality as the GalileanUnit, but for time
 *			units (e.g., "seconds since the epoch").  Unfortunately,
 *			people try to use it for more than it is capable (e.g.,
 *			days since some time on an imaginary world with only 360
 *			days per year).
 *	ut_unit		A data-structure that encapsulates ProductUnit, 
 *			GalileanUnit, LogUnit, and TimestampUnit.
 *
 * This module is thread-compatible but not thread-safe: multi-thread access to
 * this module must be externally synchronized.
 */

/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <search.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "udunits2.h"		/* this module's API */
#include "converter.h"

typedef enum {
    PRODUCT_EQUAL = 0,		/* The units are equal -- ignoring dimensionless
				 * basic-units */
    PRODUCT_INVERSE,		/* The units are reciprocals of each other */
    PRODUCT_UNCONVERTIBLE,	/* The units have incompatible dimensionality */
    PRODUCT_UNKNOWN		/* The relationship is unknown */
} ProductRelationship;

typedef struct BasicUnit	BasicUnit;
typedef struct ProductUnit	ProductUnit;

struct ut_system {
    ut_unit*		second;
    ut_unit*		one;		/* the dimensionless-unit one */
    BasicUnit**		basicUnits;
    int			basicCount;
};

typedef struct {
    ProductUnit*	(*getProduct)(const ut_unit*);
    ut_unit*		(*clone)(const ut_unit*);
    void		(*free)(ut_unit*);
    /*
     * The following comparison function is called if and only if the two units
     * belong to the same unit system.
     */
    int			(*compare)(const ut_unit*, const ut_unit*);
    ut_unit*		(*multiply)(const ut_unit*, const ut_unit*);
    ut_unit*		(*raise)(const ut_unit*, const int power);
    ut_unit*		(*root)(const ut_unit*, const int root);
    int			(*initConverterToProduct)(ut_unit*);
    int			(*initConverterFromProduct)(ut_unit*);
    ut_status		(*acceptVisitor)(const ut_unit*, const ut_visitor*,
			    void*);
} UnitOps;

typedef enum {
    BASIC,
    PRODUCT,
    GALILEAN,
    LOG,
    TIMESTAMP
} UnitType;

#undef	ABS
#define	ABS(a)		((a) < 0 ? -(a) : (a))
#undef	MIN
#define	MIN(a,b)	((a) < (b) ? (a) : (b))
#undef	MAX
#define	MAX(a,b)	((a) > (b) ? (a) : (b))

#define GET_PRODUCT(unit) \
			((unit)->common.ops->getProduct(unit))
#define CLONE(unit)	((unit)->common.ops->clone(unit))
#define MULTIPLY(unit1, unit2) \
			((unit1)->common.ops->multiply(unit1, unit2))
#define RAISE(unit, power) \
			((unit)->common.ops->raise(unit, power))
#define ROOT(unit, root) \
			((unit)->common.ops->root(unit, root))
#define FREE(unit)	((unit)->common.ops->free(unit))
#define COMPARE(unit1, unit2) \
			((unit1)->common.ops->compare(unit1, unit2))
#define ENSURE_CONVERTER_TO_PRODUCT(unit) \
			((unit)->common.toProduct != NULL || \
			(unit)->common.ops->initConverterToProduct(unit) == 0)
#define ENSURE_CONVERTER_FROM_PRODUCT(unit) \
			((unit)->common.fromProduct != NULL || \
			(unit)->common.ops->initConverterFromProduct(unit) == 0)
#define ACCEPT_VISITOR(unit, visitor, arg) \
			((unit)->common.ops->acceptVisitor(unit, visitor, arg))

typedef struct {
    ut_system*		system;
    const UnitOps*	ops;
    UnitType		type;
    cv_converter*	toProduct;
    cv_converter*	fromProduct;
} Common;

struct BasicUnit {
    Common		common;
    ProductUnit*	product;		/* equivalent product-unit */
    int			index;			/* system->basicUnits index */
    int			isDimensionless;
};

struct ProductUnit {
    Common		common;
    short*		indexes;
    short*		powers;
    int			count;
};

typedef struct {
    Common		common;
    ut_unit*		unit;
    double		scale;
    double		offset;
} GalileanUnit;

typedef struct {
    Common		common;
    ut_unit*		unit;
    double		origin;
} TimestampUnit;

typedef struct {
    Common		common;
    ut_unit*		reference;
    double		base;
} LogUnit;

union ut_unit {
    Common		common;
    BasicUnit		basic;
    ProductUnit		product;
    GalileanUnit	galilean;
    TimestampUnit	timestamp;
    LogUnit		log;
};

#define IS_BASIC(unit)		((unit)->common.type == BASIC)
#define IS_PRODUCT(unit)	((unit)->common.type == PRODUCT)
#define IS_GALILEAN(unit)	((unit)->common.type == GALILEAN)
#define IS_LOG(unit)		((unit)->common.type == LOG)
#define IS_TIMESTAMP(unit)	((unit)->common.type == TIMESTAMP)

/*
 * The following function are declared here because they are used in the
 * basic-unit section  before they are defined in the product-unit section.
 */
static ProductUnit*	productNew(
    ut_system* const		system,
    const short* const		indexes,
    const short* const		powers,
    const int			count);
static void		productFree(
    ut_unit* const		unit);
static ut_unit*		productMultiply(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2);
static ut_unit*		productRaise(
    const ut_unit* const	unit,
    const int			power);
static ut_unit*		productRoot(
    const ut_unit* const	unit,
    const int			root);


/*
 * The following two functions convert between Julian day number and
 * Gregorian/Julian dates (Julian dates are used prior to October 15,
 * 1582; Gregorian dates are used after that).  Julian day number 0 is
 * midday, January 1, 4713 BCE.  The Gregorian calendar was adopted
 * midday, October 15, 1582.
 *
 * Author: Robert Iles, March 1994
 *
 * C Porter: Steve Emmerson, October 1995
 *
 * Original: http://www.nag.co.uk:70/nagware/Examples/calendar.f90
 *
 * There is no warranty on this code.
 */


/*
 * Convert a Julian day number to a Gregorian/Julian date.
 */
void
julianDayToGregorianDate(julday, year, month, day)
    long	julday;		/* Julian day number to convert */
    int		*year;		/* Gregorian year (out) */
    int		*month;		/* Gregorian month (1-12) (out) */
    int		*day;		/* Gregorian day (1-31) (out) */
{
    long	ja, jb, jd;
    int		jc;
    int		je, iday, imonth, iyear;
    double	xc;

    if (julday < 2299161)
	ja = julday;
    else
    {
	int	ia = (int)(((julday - 1867216) - 0.25) / 36524.25);

	ja = julday + 1 + ia - (int)(0.25 * ia);
    }

    jb = ja + 1524;
    xc = ((jb - 2439870) - 122.1) / 365.25;
    jc = (int)(6680.0 + xc);
    jd = 365 * jc + (int)(0.25 * jc);
    je = (int)((jb - jd) / 30.6001);

    iday = (int)(jb - jd - (int)(30.6001 * je));

    imonth = je - 1;
    if (imonth > 12)
	imonth -= 12;

    iyear = jc - 4715;
    if (imonth > 2)
	iyear -= 1;
    if (iyear <= 0)
	iyear -= 1;

    *year = iyear;
    *month = imonth;
    *day = iday;
}


/*
 * Convert a Gregorian/Julian date to a Julian day number.
 *
 * The Gregorian calendar was adopted midday, October 15, 1582.
 */
long
gregorianDateToJulianDay(year, month, day)
    int		year;	/* Gregorian year */
    int		month;	/* Gregorian month (1-12) */
    int		day;	/* Gregorian day (1-31) */
{
    int32_t	igreg = 15 + 31 * (10 + (12 * 1582));
    int32_t	iy;	/* signed, origin 0 year */
    int32_t	ja;	/* Julian century */
    int32_t	jm;	/* Julian month */
    int32_t	jy;	/* Julian year */
    long	julday;	/* returned Julian day number */

    /*
     * Because there is no 0 BC or 0 AD, assume the user wants the start of 
     * the common era if they specify year 0.
     */
    if (year == 0)
	year = 1;

    iy = year;
    if (year < 0)
	iy++;
    if (month > 2)
    {
	jy = iy;
	jm = month + 1;
    }
    else
    {
	jy = iy - 1;
	jm = month + 13;
    }

    /*
     *  Note: SLIGHTLY STRANGE CONSTRUCTIONS REQUIRED TO AVOID PROBLEMS WITH
     *        OPTIMISATION OR GENERAL ERRORS UNDER VMS!
     */
    julday = day + (int)(30.6001 * jm);
    if (jy >= 0)
    {
	julday += 365 * jy;
	julday += 0.25 * jy;
    }
    else
    {
	double	xi = 365.25 * jy;

	if ((int)xi != xi)
	    xi -= 1;
	julday += (int)xi;
    }
    julday += 1720995;

    if (day + (31* (month + (12 * iy))) >= igreg)
    {
	ja = jy/100;
	julday -= ja;
	julday += 2;
	julday += ja/4;
    }

    return julday;
}

/*
 * Returns the Julian day number that is the origin of all things temporal in
 * this module.
 *
 * Returns:
 *      The Julian day number that is the origin for time in this module.
 */
static long
getJuldayOrigin()
{
    static long juldayOrigin;

    if (juldayOrigin == 0)
	juldayOrigin = gregorianDateToJulianDay(2001, 1, 1);

    return juldayOrigin;
}


/*
 * Encodes a time as a double-precision value.
 *
 * Arguments:
 *	hours		The number of hours (0 = midnight).
 *	minutes		The number of minutes.
 *	seconds		The number of seconds.
 * Returns:
 *	The clock-time encoded as a scalar value.
 */
double
ut_encode_clock(
    int		hours,
    int		minutes,
    double	seconds)
{
    return (hours*60 + minutes)*60 + seconds;
}


/*
 * Decompose a value into a set of values accounting for uncertainty.
 */
static void
decompose(value, uncer, nbasis, basis, count)
    double	value;
    double	uncer;		/* >= 0 */
    int		nbasis;
    double	*basis;		/* all values > 0 */
    double	*count;
{
    int		i;

    for (i = 0; i < nbasis; i++)
    {
	double	r = fmod(value, basis[i]);	/* remainder */

	/* Adjust remainder to minimum magnitude. */
	if (ABS(2*r) > basis[i])
	    r += r > 0
		    ? -basis[i]
		    :  basis[i];

	if (ABS(r) <= uncer)
	{
	    /* The value equals a basis multiple within the uncertainty. */
	    double	half = value < 0 ? -basis[i]/2 : basis[i]/2;
	    modf((value+half)/basis[i], count+i);
	    break;
	}

	value = basis[i] * modf(value/basis[i], count+i);
    }

    if (i >= nbasis) {
	count[--i] += value;
    }
    else {
	for (i++; i < nbasis; i++)
	    count[i] = 0;
    }
}


/*
 * Encodes a date as a double-precision value.
 *
 * Arguments:
 *	year		The year.
 *	month		The month.
 *	day		The day (1 = the first of the month).
 * Returns:
 *	The date encoded as a scalar value.
 */
double
ut_encode_date(
    int		year,
    int		month,
    int		day)
{
    return 86400.0 *
	(gregorianDateToJulianDay(year, month, day) - getJuldayOrigin());
}


/*
 * Encodes a time as a double-precision value.  The convenience function is
 * equivalent to "ut_encode_date(year,month,day) + 
 * ut_encode_clock(hour,minute,second)"
 *
 * Arguments:
 *	year	The year.
 *	month	The month.
 *	day	The day.
 *	hour	The hour.
 *	minute	The minute.
 *	second	The second.
 * Returns:
 *	The time encoded as a scalar value.
 */
double
ut_encode_time(
    const int		year,
    const int		month,
    const int		day,
    const int		hour,
    const int		minute,
    const double	second)
{
    return ut_encode_date(year, month, day) + ut_encode_clock(hour, minute, second);
}


/*
 * Decodes a time from a double-precision value.
 *
 * Arguments:
 *      value           The value to be decoded.
 *      year            Pointer to the variable to be set to the year.
 *      month           Pointer to the variable to be set to the month.
 *      day             Pointer to the variable to be set to the day.
 *      hour            Pointer to the variable to be set to the hour.
 *      minute          Pointer to the variable to be set to the minute.
 *      second          Pointer to the variable to be set to the second.
 *      resolution      Pointer to the variable to be set to the resolution
 *                      of the decoded time in seconds.
 */
void
ut_decode_time(
    double	value,
    int		*year,
    int		*month,
    int		*day,
    int		*hour,
    int		*minute,
    double	*second,
    double	*resolution)
{
    int			days;
    int			hours;
    int			minutes;
    double		seconds;
    double		uncer;		/* uncertainty of input value */
    typedef union
    {
	double	    vec[7];
	struct
	{
	    double	days;
	    double	hours12;
	    double	hours;
	    double	minutes10;
	    double	minutes;
	    double	seconds10;
	    double	seconds;
	}	    ind;
    } Basis;
    Basis		counts;
    static const Basis	basis = {86400, 43200, 3600, 600, 60, 10, 1};

    uncer = ldexp(value < 0 ? -value : value, -DBL_MANT_DIG);

    days = (int)floor(value/basis.ind.days);
    value -= days * basis.ind.days;		/* make positive excess */

    decompose(value, uncer, (int)(sizeof(basis.vec)/sizeof(basis.vec[0])),
	   basis.vec, counts.vec);

    days += counts.ind.days;
    hours = (int)counts.ind.hours12 * 12 + (int)counts.ind.hours;
    minutes = (int)counts.ind.minutes10 * 10 + (int)counts.ind.minutes;
    seconds = (int)counts.ind.seconds10 * 10 + counts.ind.seconds;

    if (seconds >= 60) {
	seconds -= 60;
	if (++minutes >= 60) {
	    minutes -= 60;
	    if (++hours >= 24) {
		hours -= 24;
		days++;
	    }
	}
    }

    *second = seconds;
    *minute = minutes;
    *hour = hours;
    *resolution = uncer;

    julianDayToGregorianDate(getJuldayOrigin() + days, year, month, day);
}


/******************************************************************************
 * Parameters common to all types of units:
 ******************************************************************************/


/*
 * Arguments:
 *	common	Pointer to unit common-area.
 *	ops	Pointer to unit-specific function-structure.
 *	system	Pointer to unit-system.
 *	type	The type of unit.
 * Returns:
 *	 0	Success.
 */
static int
commonInit(
    Common* const		common,
    const UnitOps* const	ops,
    const ut_system* const	system,
    const UnitType		type)
{
    assert(system != NULL);
    assert(common != NULL);
    assert(ops != NULL);

    common->system = (ut_system*)system;
    common->ops = ops;
    common->type = type;
    common->toProduct = NULL;
    common->fromProduct = NULL;

    return 0;
}


/******************************************************************************
 * Basic-Unit:
 ******************************************************************************/


static UnitOps	basicOps;


/*
 * Returns a new instance of a basic-unit.
 *
 * Arguments:
 *	system		The unit-system to be associated with the new instance.
 *	isDimensionless	Whether or not the unit is dimensionless (e.g., 
 *			"radian").
 *	index		The index of the basic-unit in "system".
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_OS	Operating-system error.  See "errno".
 *	else	Pointer to newly-allocated basic-unit.
 */
static BasicUnit*
basicNew(
    ut_system* const	system,
    const int		isDimensionless,
    const int		index)
{
    BasicUnit*		basicUnit = NULL;	/* failure */
    int			error = 1;
    short		power = 1;
    short		shortIndex = (short)index;
    ProductUnit*	product;

    assert(system != NULL);

    product = productNew(system, &shortIndex, &power, 1);

    if (product == NULL) {
	ut_set_status(UT_OS);
	ut_handle_error_message(
	    "basicNew(): Couldn't create new product-unit");
    }
    else {
	basicUnit = malloc(sizeof(BasicUnit));

	if (basicUnit == NULL) {
	    ut_set_status(UT_OS);
	    ut_handle_error_message(strerror(errno));
	    ut_handle_error_message(
		"basicNew(): Couldn't allocate %lu-byte basic-unit",
		sizeof(BasicUnit));
	}
	else if (commonInit(&basicUnit->common, &basicOps, system,
		BASIC) == 0) {
	    basicUnit->index = index;
	    basicUnit->isDimensionless = isDimensionless;
	    basicUnit->product = product;
	    error = 0;
	}				/* "basicUnit" allocated */

	if (error)
	    productFree((ut_unit*)product);
    }				/* "product" allocated */

    return basicUnit;
}


static ProductUnit*
basicGetProduct(
    const ut_unit* const	unit)
{
    assert(IS_BASIC(unit));

    return unit->basic.product;
}


static ut_unit*
basicClone(
    const ut_unit* const	unit)
{
    assert(IS_BASIC(unit));

    return (ut_unit*)basicNew(unit->common.system, unit->basic.isDimensionless,
	unit->basic.index);
}


static void
basicFree(
    ut_unit* const	unit)
{
    if (unit != NULL) {
	assert(IS_BASIC(unit));
	productFree((ut_unit*)unit->basic.product);
	unit->basic.product = NULL;
	free(unit);
    }
}


static int
basicCompare(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    int	cmp;

    assert(unit1 != NULL);
    assert(IS_BASIC(unit1));
    assert(unit2 != NULL);

    if (IS_PRODUCT(unit2)) {
	cmp = -COMPARE(unit2, unit1);
    }
    else if (!IS_BASIC(unit2)) {
	int	diff = unit1->common.type - unit2->common.type;

	cmp = diff < 0 ? -1 : diff == 0 ? 0 : 1;
    }
    else {
	int	index1 = unit1->basic.index;
	int	index2 = unit2->basic.index;

	cmp = index1 < index2 ? -1 : index1 == index2 ? 0 : 1;
    }

    return cmp;
}


/*
 * Multiplies a basic-unit by another unit.
 *
 * Arguments:
 *	unit1	The basic-unit.
 *	unit2	The other unit.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given units is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
basicMultiply(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    assert(unit1 != NULL);
    assert(unit2 != NULL);
    assert(IS_BASIC(unit1));

    return productMultiply((const ut_unit*)unit1->basic.product, unit2);
}


/*
 * Returns the result of raising a basic-unit to a power.
 *
 * Arguments:
 *	unit	The basic-unit.
 *	power	The power.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given unit is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
basicRaise(
    const ut_unit* const	unit,
    const int			power)
{
    assert(unit != NULL);
    assert(IS_BASIC(unit));
    assert(power != 0);
    assert(power != 1);

    return productRaise((ut_unit*)unit->basic.product, power);
}


/*
 * Returns the result of taking a root of a basic-unit.
 *
 * Arguments:
 *	unit	The basic-unit.
 *	root	The root to take.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given unit is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
basicRoot(
    const ut_unit* const	unit,
    const int			root)
{
    assert(unit != NULL);
    assert(IS_BASIC(unit));
    assert(root > 1);

    return productRoot((ut_unit*)unit->basic.product, root);
}


/*
 * Initializes the converter of numeric from the given product-unit to the
 * underlying product-unit (i.e., to itself).
 *
 * Arguments:
 *	unit	The product unit.
 * Returns:
 *	 0	Success.
 */
static int
basicInitConverterToProduct(
    ut_unit* const	unit)
{
    assert(unit != NULL);
    assert(IS_BASIC(unit));

    if (unit->common.toProduct == NULL)
	unit->common.toProduct = cv_get_trivial();

    return 0;
}


/*
 * Initializes the converter of numeric to the given product-unit from the
 * underlying product-unit (i.e., to itself).
 *
 * Arguments:
 *	unit	The product unit.
 * Returns:
 *	 0	Success.
 */
static int
basicInitConverterFromProduct(
    ut_unit* const	unit)
{
    assert(unit != NULL);
    assert(IS_BASIC(unit));

    if (unit->common.fromProduct == NULL)
	unit->common.fromProduct = cv_get_trivial();

    return 0;
}


static ut_status
basicAcceptVisitor(
    const ut_unit* const		unit,
    const ut_visitor* const	visitor,
    void* const			arg)
{
    assert(unit != NULL);
    assert(IS_BASIC(unit));
    assert(visitor != NULL);

    return visitor->visit_basic(unit, arg);
}


static UnitOps	basicOps = {
    basicGetProduct,
    basicClone,
    basicFree,
    basicCompare,
    basicMultiply,
    basicRaise,
    basicRoot,
    basicInitConverterToProduct,
    basicInitConverterFromProduct,
    basicAcceptVisitor
};


/******************************************************************************
 * Product Unit:
 ******************************************************************************/

static UnitOps	productOps;


/*
 * Arguments:
 *	system	The unit-system for the new unit.
 *	indexes	Pointer to array of indexes of basic-units.  May be freed upon
 *		return.
 *	powers	Pointer to array of powers.  Client may free upon return.
 *	count	The number of elements in "indexes" and "powers".  May be zero.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_OS	Operating-system error.  See "errno".
 *	else	The newly-allocated, product-unit.
 */
static ProductUnit*
productNew(
    ut_system* const	system,
    const short* const	indexes,
    const short* const	powers,
    const int		count)
{
    ProductUnit*	productUnit;

    assert(system != NULL);
    assert(count >= 0);
    assert(count == 0 || (indexes != NULL && powers != NULL));

    productUnit = malloc(sizeof(ProductUnit));

    if (productUnit == NULL) {
	ut_set_status(UT_OS);
	ut_handle_error_message(strerror(errno));
	ut_handle_error_message(
	    "productNew(): Couldn't allocate %d-byte product-unit",
	    sizeof(ProductUnit));
    }
    else {
	int	error = 1;

	if (commonInit(&productUnit->common, &productOps, system, PRODUCT)
		== 0) {
            if (count == 0) {
                productUnit->count = count;
                productUnit->indexes = NULL;
                productUnit->powers = NULL;
                error = 0;
            }
            else {
                size_t	nbytes = sizeof(short)*count;
                short*	newIndexes = malloc(nbytes*2);

                if (count > 0 && newIndexes == NULL) {
                    ut_set_status(UT_OS);
                    ut_handle_error_message(strerror(errno));
                    ut_handle_error_message("productNew(): "
                        "Couldn't allocate %d-element index array", count);
                }
                else {
                    short*	newPowers = newIndexes + count;

                    productUnit->count = count;
                    productUnit->indexes = memcpy(newIndexes, indexes, nbytes);
                    productUnit->powers = memcpy(newPowers, powers, nbytes);
                    error = 0;
                }
	    }                           /* "count > 0" */
	}                               /* "productUnit->common" initialized */

	if (error) {
	    free(productUnit);
	    productUnit = NULL;
	}
    }				        /* "productUnit" allocated */

    return productUnit;
}


static ProductUnit*
productGetProduct(
    const ut_unit* const	unit)
{
    assert(unit != NULL);
    assert(IS_PRODUCT(unit));

    return (ProductUnit*)&unit->product;
}


static ut_unit*
productClone(
    const ut_unit* const	unit)
{
    ut_unit*		clone;

    assert(unit != NULL);
    assert(IS_PRODUCT(unit));

    if (unit == unit->common.system->one) {
	clone = unit->common.system->one;
    }
    else {
	clone = (ut_unit*)productNew(unit->common.system,
	    unit->product.indexes, unit->product.powers,
	    unit->product.count);
    }

    return clone;
}


static int
productCompare(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    int	cmp;

    assert(unit1 != NULL);
    assert(IS_PRODUCT(unit1));
    assert(unit2 != NULL);

    if (IS_BASIC(unit2)) {
	cmp = productCompare(unit1, (ut_unit*)unit2->basic.product);
    }
    else if (!IS_PRODUCT(unit2)) {
	int	diff = unit1->common.type - unit2->common.type;

	cmp = diff < 0 ? -1 : diff == 0 ? 0 : 1;
    }
    else {
	const ProductUnit* const	product1 = &unit1->product;
	const ProductUnit* const	product2 = &unit2->product;

	cmp = product1->count - product2->count;

	if (cmp == 0) {
	    const short* const	indexes1 = product1->indexes;
	    const short* const	indexes2 = product2->indexes;
	    const short* const	powers1	= product1->powers;
	    const short* const	powers2	= product2->powers;
	    int			i;

	    for (i = 0; i < product1->count; ++i) {
		cmp = indexes1[i] - indexes2[i];

		if (cmp == 0)
		    cmp = powers1[i] - powers2[i];

		if (cmp != 0)
		    break;
	    }
	}
    }

    return cmp;
}


static void
productReallyFree(
    ut_unit* const	unit)
{
    if (unit != NULL) {
	assert(IS_PRODUCT(unit));
	free(unit->product.indexes);
	unit->product.indexes = NULL;
	cv_free(unit->common.toProduct);
	unit->common.toProduct = NULL;
	cv_free(unit->common.fromProduct);
	unit->common.fromProduct = NULL;
	free(unit);
    }
}


static void
productFree(
    ut_unit* const	unit)
{
    if (unit != unit->common.system->one)
	productReallyFree(unit);
}


/*
 * Multiplies a product-unit by another unit.
 *
 * Arguments:
 *	unit1	The product unit.
 *	unit2	The other unit.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given units is
 *					meaningless.
 *		    UT_OS		Operating-system failure.  See "errno".
 *	else	The resulting unit.
 */
static ut_unit*
productMultiply(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    ut_unit*		result = NULL;	/* failure */

    assert(unit1 != NULL);
    assert(unit2 != NULL);
    assert(IS_PRODUCT(unit1));

    if (!IS_PRODUCT(unit2)) {
	result = MULTIPLY(unit2, unit1);
    }
    else {
	const ProductUnit* const	product1 = &unit1->product;
	const ProductUnit* const	product2 = &unit2->product;
	short*				indexes1 = product1->indexes;
	short*				indexes2 = product2->indexes;
	short*				powers1 = product1->powers;
	short*				powers2 = product2->powers;
	int				count1 = product1->count;
	int				count2 = product2->count;
	int				sumCount = count1 + count2;

	if (sumCount == 0) {
	    result = unit1->common.system->one;
	}
	else {
	    static short*		indexes = NULL;

	    indexes = realloc(indexes, sizeof(short)*sumCount);

	    if (indexes == NULL) {
		ut_set_status(UT_OS);
		ut_handle_error_message(strerror(errno));
		ut_handle_error_message("productMultiply(): "
		    "Couldn't allocate %d-element index array", sumCount);
	    }
	    else {
		static short*	powers = NULL;
		
		powers = realloc(powers, sizeof(short)*sumCount);

		if (powers == NULL) {
		    ut_set_status(UT_OS);
		    ut_handle_error_message(strerror(errno));
		    ut_handle_error_message("productMultiply(): "
			"Couldn't allocate %d-element power array",
			sumCount);
		}
		else {
		    int				count = 0;
		    int				i1 = 0;
		    int				i2 = 0;

		    while (i1 < count1 || i2 < count2) {
			if (i1 >= count1) {
			    indexes[count] = indexes2[i2];
			    powers[count++] = powers2[i2++];
			}
			else if (i2 >= count2) {
			    indexes[count] = indexes1[i1];
			    powers[count++] = powers1[i1++];
			}
			else if (indexes1[i1] > indexes2[i2]) {
			    indexes[count] = indexes2[i2];
			    powers[count++] = powers2[i2++];
			}
			else if (indexes1[i1] < indexes2[i2]) {
			    indexes[count] = indexes1[i1];
			    powers[count++] = powers1[i1++];
			}
			else {
			    if (powers1[i1] != -powers2[i2]) {
				indexes[count] = indexes1[i1];
				powers[count++] = powers1[i1] + powers2[i2];
			    }

			    i1++;
			    i2++;
			}
		    }

		    result = (ut_unit*)productNew(unit1->common.system,
			indexes, powers, count);
		}			/* "powers" re-allocated */
	    }				/* "indexes" re-allocated */
	}				/* "sumCount > 0" */
    }					/* "unit2" is a product-unit */

    return result;
}


/*
 * Returns the result of raising a product unit to a power.
 *
 * Arguments:
 *	unit	The product unit.
 *	power	The power.  Must be greater than -256 and less than 256.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given unit is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
productRaise(
    const ut_unit* const	unit,
    const int			power)
{
    ut_unit*		result = NULL;	/* failure */
    const ProductUnit*	product;
    int			count;
    short*		newPowers;

    assert(unit != NULL);
    assert(IS_PRODUCT(unit));
    assert(power >= -255 && power <= 255);
    assert(power != 0);
    assert(power != 1);

    product = &unit->product;
    count = product->count;

    if (count == 0) {
        result = unit->common.system->one;
    }
    else {
        newPowers = malloc(sizeof(short)*count);

        if (newPowers == NULL) {
            ut_set_status(UT_OS);
            ut_handle_error_message(strerror(errno));
            ut_handle_error_message("productRaise(): "
                "Couldn't allocate %d-element powers-buffer", count);
        }
        else {
            const short* const	oldPowers = product->powers;
            int			i;

            for (i = 0; i < count; i++)
                newPowers[i] = (short)(oldPowers[i] * power);

            result = (ut_unit*)productNew(unit->common.system,
                product->indexes, newPowers, count);

            free(newPowers);
        }				/* "newPowers" allocated */
    }				        /* "count > 0" */

    return result;
}


/*
 * Returns the result of taking a root of a unit.
 *
 * Arguments:
 *	unit	The product unit.
 *	root	The root.  Must be greater than 1 and less than 256.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given unit is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
productRoot(
    const ut_unit* const	unit,
    const int			root)
{
    ut_unit*		result = NULL;	/* failure */
    const ProductUnit*	product;
    int			count;
    short*		newPowers;

    assert(unit != NULL);
    assert(IS_PRODUCT(unit));
    assert(root > 1 && root <= 255);

    product = &unit->product;
    count = product->count;

    if (count == 0) {
        result = unit->common.system->one;
    }
    else {
        newPowers = malloc(sizeof(short)*count);

        if (newPowers == NULL) {
            ut_set_status(UT_OS);
            ut_handle_error_message(strerror(errno));
            ut_handle_error_message("productRoot(): "
                "Couldn't allocate %d-element powers-buffer", count);
        }
        else {
            const short* const	oldPowers = product->powers;
            int			i;

            for (i = 0; i < count; i++) {
                if ((oldPowers[i] % root) != 0) {
                    break;
                }
                newPowers[i] = (short)(oldPowers[i] / root);
            }

            if (i < count) {
                char buf[80];

                if (ut_format(unit, buf, sizeof(buf), UT_ASCII) == -1) {
                    ut_set_status(UT_MEANINGLESS);
                    ut_handle_error_message("productRoot(): "
                        "Can't take root of unit");
                }
                else {
                    ut_set_status(UT_MEANINGLESS);
                    buf[sizeof(buf)-1] = 0;
                    ut_handle_error_message("productRoot(): "
                        "It's meaningless to take the %d%s root of \"%s\"",
                        root, root == 2 ? "nd" : root == 3 ? "rd" : "th",
                        buf);
                }
            }
            else {
                result = (ut_unit*)productNew(unit->common.system,
                    product->indexes, newPowers, count);
            }

            free(newPowers);
        }				/* "newPowers" allocated */
    }				        /* "count > 0" */

    return result;
}


/*
 * Initializes a converter of numeric values between the given product-unit and
 * the underlying product-unit (i.e., to itself).
 *
 * Arguments:
 *	converter	Pointer to pointer to the converter to be initialized.
 * Returns:
 *	 0	Success.
 */
static int
productInitConverter(
    cv_converter** const	converter)
{
    assert(converter != NULL);

    *converter = cv_get_trivial();

    return 0;
}


/*
 * Initializes the converter of numeric values from the given product-unit to
 * the underlying product-unit (i.e., to itself).
 *
 * Arguments:
 *	unit	The product unit.
 * Returns:
 *	 0	Success.
 */
static int
productInitConverterToProduct(
    ut_unit* const	unit)
{
    assert(unit != NULL);
    assert(IS_PRODUCT(unit));

    return productInitConverter(&unit->common.toProduct);
}


/*
 * Initializes the converter of numeric values to the given product-unit from
 * the underlying product-unit (i.e., to itself).
 *
 * Arguments:
 *	unit	The product unit.
 * Returns:
 *	 0	Success.
 */
static int
productInitConverterFromProduct(
    ut_unit* const	unit)
{
    assert(unit != NULL);
    assert(IS_PRODUCT(unit));

    return productInitConverter(&unit->common.fromProduct);
}


/*
 * Returns the relationship between two product-units.  In determining the
 * relationship, dimensionless basic-units are ignored.
 *
 * Arguments:
 *	unit1	The first product unit.
 *	unit2	The second product unit.
 * Returns:
 *	PRODUCT_EQUAL		The units are equal -- ignoring dimensionless
 *				basic-units.
 *	PRODUCT_INVERSE		The units are reciprocals of each other.
 *	PRODUCT_UNCONVERTIBLE	The dimensionalities of the units are
 *				unconvertible.
 */
static ProductRelationship
productRelationship(
    const ProductUnit* const	unit1,
    const ProductUnit* const	unit2)
{
    ProductRelationship		relationship = PRODUCT_UNKNOWN;

    assert(unit1 != NULL);
    assert(unit2 != NULL);

    {
	const short* const	indexes1 = unit1->indexes;
	const short* const	indexes2 = unit2->indexes;
	const short* const	powers1 = unit1->powers;
	const short* const	powers2 = unit2->powers;
	const int		count1 = unit1->count;
	const int		count2 = unit2->count;
	const ut_system* const	system = unit1->common.system;
	int			i1 = 0;
	int			i2 = 0;

	while (i1 < count1 || i2 < count2) {
	    int	iBasic = -1;

	    if (i1 >= count1) {
		iBasic = indexes2[i2++];
	    }
	    else if (i2 >= count2) {
		iBasic = indexes1[i1++];
	    }
	    else if (indexes1[i1] > indexes2[i2]) {
		iBasic = indexes2[i2++];
	    }
	    else if (indexes1[i1] < indexes2[i2]) {
		iBasic = indexes1[i1++];
	    }

	    if (iBasic != -1) {
		if (!system->basicUnits[iBasic]->isDimensionless) {
		    relationship = PRODUCT_UNCONVERTIBLE;
		    break;
		}
	    }
	    else {
		iBasic = indexes1[i1];

		if (!system->basicUnits[iBasic]->isDimensionless) {
		    if (powers1[i1] == powers2[i2]) {
			if (relationship == PRODUCT_INVERSE) {
			    relationship = PRODUCT_UNCONVERTIBLE;
			    break;
			}

			relationship = PRODUCT_EQUAL;
		    }
		    else if (powers1[i1] == -powers2[i2]) {
			if (relationship == PRODUCT_EQUAL) {
			    relationship = PRODUCT_UNCONVERTIBLE;
			    break;
			}

			relationship = PRODUCT_INVERSE;
		    }
		    else {
			relationship = PRODUCT_UNCONVERTIBLE;
			break;
		    }
		}

		i1++;
		i2++;
	    }
	}
    }

    if (relationship == PRODUCT_UNKNOWN) {
	/*
	 * Both units are dimensionless.
	 */
	relationship = PRODUCT_EQUAL;
    }

    return relationship;
}


static ut_status
productAcceptVisitor(
    const ut_unit* const	unit,
    const ut_visitor* const	visitor,
    void* const			arg)
{
    int		count = unit->product.count;
    BasicUnit**	basicUnits = malloc(sizeof(BasicUnit)*count);

    assert(unit != NULL);
    assert(IS_PRODUCT(unit));
    assert(visitor != NULL);

    if (count != 0 && basicUnits == NULL) {
	ut_set_status(UT_OS);
	ut_handle_error_message(strerror(errno));
	ut_handle_error_message("productAcceptVisitor(): "
	    "Couldn't allocate %d-element basic-unit array", count);
    }
    else {
	int*	powers = malloc(sizeof(int)*count);

	if (count != 0 && powers == NULL) {
	    ut_set_status(UT_OS);
	    ut_handle_error_message(strerror(errno));
	    ut_handle_error_message("productAcceptVisitor(): "
		"Couldn't allocate %d-element power array", count);
	}
	else {
	    const ProductUnit*	prodUnit = &unit->product;
	    int			i;

	    for (i = 0; i < count; ++i) {
		basicUnits[i] =
		    unit->common.system->basicUnits[prodUnit->indexes[i]];
		powers[i] = prodUnit->powers[i];
	    }

	    ut_set_status(visitor->visit_product(unit, count,
		(const ut_unit**)basicUnits, powers, arg));

	    free(powers);
	}				/* "powers" allocated */

	free(basicUnits);
    }					/* "basicUnits" allocated */

    return ut_get_status();
}


static UnitOps	productOps = {
    productGetProduct,
    productClone,
    productFree,
    productCompare,
    productMultiply,
    productRaise,
    productRoot,
    productInitConverterToProduct,
    productInitConverterFromProduct,
    productAcceptVisitor
};


/*
 * Indicates if a product-unit is dimensionless or not.
 *
 * Arguments:
 *	unit	The product-unit in question.
 * Returns:
 *	0	"unit" is dimensionfull.
 *	else	"unit" is dimensionless.
 */
static int
productIsDimensionless(
    const ProductUnit* const	unit)
{
    int			isDimensionless = 1;
    int			count;
    const short*	indexes;
    ut_system*		system;
    int			i;

    assert(unit != NULL);
    assert(IS_PRODUCT(unit));

    count = unit->count;
    indexes = unit->indexes;
    system = unit->common.system;

    for (i = 0; i < count; ++i) {
	if (!system->basicUnits[indexes[i]]->isDimensionless) {
	    isDimensionless = 0;
	    break;
	}
    }

    return isDimensionless;
}


/******************************************************************************
 * Galilean Unit:
 ******************************************************************************/


static UnitOps	galileanOps;


/*
 * Returns a new unit instance.  The returned instance is not necessarily a
 * Galilean unit.
 *
 * Arguments:
 *	scale	The scale-factor for the new unit.
 *	unit	The underlying unit.  May be freed upon return.
 *	offset	The offset for the new unit.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_OS	Operating-system error.  See "errno".
 *	else	The newly-allocated, galilean-unit.
 */
static ut_unit*
galileanNew(
    double		scale,
    const ut_unit*	unit,
    double		offset)
{
    ut_unit*	newUnit = NULL;	/* failure */

    assert(scale != 0);
    assert(unit != NULL);

    if (IS_GALILEAN(unit)) {
	scale *= unit->galilean.scale;
	offset += (unit->galilean.scale * unit->galilean.offset) / scale;
	unit = unit->galilean.unit;
    }

    if (scale == 1 && offset == 0) {
	newUnit = CLONE(unit);
    }
    else {
	GalileanUnit*	galileanUnit = malloc(sizeof(GalileanUnit));

	if (galileanUnit == NULL) {
	    ut_set_status(UT_OS);
	    ut_handle_error_message("galileanNew(): "
		"Couldn't allocate %lu-byte Galilean unit",
		sizeof(GalileanUnit));
	}
	else {
	    int	error = 1;

	    if (commonInit(&galileanUnit->common, &galileanOps,
		    unit->common.system, GALILEAN) == 0) {
		galileanUnit->scale = scale;
		galileanUnit->offset = offset;
		galileanUnit->unit = CLONE(unit);
		error = 0;
	    }

	    if (error) {
		free(galileanUnit);
		galileanUnit = NULL;
	    }
	}				/* "galileanUnit" allocated */

	newUnit = (ut_unit*)galileanUnit;
    }					/* Galilean unit necessary */

    return newUnit;
}


static ProductUnit*
galileanGetProduct(
    const ut_unit* const	unit)
{
    assert(unit != NULL);
    assert(IS_GALILEAN(unit));

    return GET_PRODUCT(unit->galilean.unit);
}


static ut_unit*
galileanClone(
    const ut_unit* const	unit)
{
    const GalileanUnit*	galileanUnit;

    assert(unit != NULL);
    assert(IS_GALILEAN(unit));

    galileanUnit = &unit->galilean;

    return galileanNew(galileanUnit->scale, galileanUnit->unit,
	galileanUnit->offset);
}


static int
galileanCompare(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    int	cmp;

    assert(unit1 != NULL);
    assert(IS_GALILEAN(unit1));

    if (!IS_GALILEAN(unit2)) {
	int	diff = unit1->common.type - unit2->common.type;

	cmp = diff < 0 ? -1 : diff == 0 ? 0 : 1;
    }
    else {
	const GalileanUnit* const	galilean1 = &unit1->galilean;
	const GalileanUnit* const	galilean2 = &unit2->galilean;

	cmp =
	    galilean1->offset < galilean2->offset
		? -1
		: galilean1->offset == galilean2->offset
		    ? 0
		    : -1;

	if (cmp == 0) {
	    cmp =
		galilean1->scale < galilean2->scale
		    ? -1
		    : galilean1->scale == galilean2->scale
			? 0
			: -1;

	    if (cmp == 0)
		cmp = COMPARE(galilean1->unit, galilean2->unit);
	}
    }

    return cmp;
}


static void
galileanFree(
    ut_unit* const	unit)
{
    if (unit != NULL) {
	assert(IS_GALILEAN(unit));
	FREE(unit->galilean.unit);
	cv_free(unit->common.toProduct);
	unit->common.toProduct = NULL;
	cv_free(unit->common.fromProduct);
	unit->common.fromProduct = NULL;
	free((void*)unit);
    }
}


/*
 * Multiplies a Galilean-unit by another unit.  Any offset is ignored.
 *
 * Arguments:
 *	unit1	The Galilean-unit.
 *	unit2	The other unit.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given units is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
galileanMultiply(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    ut_unit*		result = NULL;	/* failure */
    const GalileanUnit*	galilean1;

    assert(unit1 != NULL);
    assert(unit2 != NULL);
    assert(IS_GALILEAN(unit1));

    galilean1 = &unit1->galilean;

    if (IS_PRODUCT(unit2)) {
	ut_unit*	tmp = MULTIPLY(galilean1->unit, unit2);

	if (tmp != NULL) {
	    result = galileanNew(galilean1->scale, tmp, 0);

	    FREE(tmp);
	}
    }
    else if (IS_GALILEAN(unit2)) {
	const GalileanUnit* const	galilean2 = &unit2->galilean;
	ut_unit*				tmp =
	    MULTIPLY(galilean1->unit, galilean2->unit);

	if (tmp != NULL) {
	    result = galileanNew(galilean1->scale * galilean2->scale, tmp, 0);

	    FREE(tmp);
	}
    }
    else {
	result = MULTIPLY(unit2, unit1);
    }

    return result;
}


/*
 * Returns the result of raising a Galilean-unit to a power.  Any offset is
 * ignored.
 *
 * Arguments:
 *	unit	The Galilean-unit.
 *	power	The power.  Must be greater than -256 and less than 256.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given units is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
galileanRaise(
    const ut_unit* const	unit,
    const int			power)
{
    const GalileanUnit*	 galilean;
    ut_unit*             tmp;
    ut_unit*             result = NULL;  /* failure */

    assert(unit != NULL);
    assert(IS_GALILEAN(unit));
    assert(power >= -255 && power <= 255);
    assert(power != 0);
    assert(power != 1);

    galilean = &unit->galilean;
    tmp = RAISE(galilean->unit, power);

    if (tmp != NULL) {
        result = galileanNew(pow(galilean->scale, power), tmp, 0);

        ut_free(tmp);
    }

    return result;
}


/*
 * Returns the result of taking a root of a Galilean-unit.  Any offset is
 * ignored.
 *
 * Arguments:
 *	unit	The Galilean-unit.
 *	root	The root.  Must be greater than 1 and less than 256.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given units is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
galileanRoot(
    const ut_unit* const	unit,
    const int			root)
{
    const GalileanUnit*	 galilean;
    ut_unit*             tmp;
    ut_unit*             result = NULL;  /* failure */

    assert(unit != NULL);
    assert(IS_GALILEAN(unit));
    assert(root > 1 && root <= 255);

    galilean = &unit->galilean;
    tmp = ROOT(galilean->unit, root);

    if (tmp != NULL) {
        result = galileanNew(pow(galilean->scale, 1.0/root), tmp, 0);

        ut_free(tmp);
    }

    return result;
}


/*
 * Initializes the converter of numeric values from the given Galilean unit to
 * the underlying product-unit.
 *
 * Arguments:
 *	unit	The Galilean unit.
 * Returns:
 *	-1	Failure.  "ut_get_status()" will be:
 *		    UT_OS	Operating-system fault.  See "errno".
 *	0	Success.
 */
static int
galileanInitConverterToProduct(
    ut_unit* const	unit)
{
    int			retCode = -1;	/* failure */
    cv_converter*	toUnderlying;

    assert(unit != NULL);
    assert(IS_GALILEAN(unit));

    toUnderlying = cv_get_galilean(
	unit->galilean.scale, unit->galilean.offset * unit->galilean.scale);

    if (toUnderlying == NULL) {
	ut_set_status(UT_OS);
	ut_handle_error_message(strerror(errno));
	ut_handle_error_message("galileanInitConverterToProduct(): "
	    "Couldn't get converter to underlying unit");
    }
    else {
	if (ENSURE_CONVERTER_TO_PRODUCT(unit->galilean.unit)) {
	    assert(unit->common.toProduct == NULL);

	    unit->common.toProduct = cv_combine(
		toUnderlying, unit->galilean.unit->common.toProduct);

	    if (unit->common.toProduct == NULL) {
		ut_set_status(UT_OS);
		ut_handle_error_message(strerror(errno));
		ut_handle_error_message("galileanInitConverterToProduct(): "
		    "Couldn't combine converters");
	    }
	    else {
		retCode = 0;
	    }
	}

	cv_free(toUnderlying);
    }				/* "toUnderlying" allocated */

    return retCode;
}


/*
 * Initializes the converter of numeric values to the given Galilean unit from
 * the underlying product-unit.
 *
 * Arguments:
 *	unit	The Galilean unit.
 * Returns:
 *	-1	Failure.  "ut_get_status()" will be:
 *		    UT_OS	Operating-system fault.  See "errno".
 *	0	Success.
 */
static int
galileanInitConverterFromProduct(
    ut_unit* const	unit)
{
    int			retCode = -1;		/* failure */
    cv_converter*	fromUnderlying;

    assert(unit != NULL);
    assert(IS_GALILEAN(unit));

    fromUnderlying = cv_get_galilean(
	1.0/unit->galilean.scale, -unit->galilean.offset);

    if (fromUnderlying == NULL) {
	ut_set_status(UT_OS);
	ut_handle_error_message(strerror(errno));
	ut_handle_error_message("galileanInitConverterFromProduct(): "
	    "Couldn't get converter from underlying unit");
    }
    else {
	if (ENSURE_CONVERTER_FROM_PRODUCT(unit->galilean.unit)) {
	    assert(unit->common.fromProduct == NULL);

	    unit->common.fromProduct = cv_combine(
		unit->galilean.unit->common.fromProduct, fromUnderlying);

	    if (unit->common.fromProduct == NULL) {
		ut_set_status(UT_OS);
		ut_handle_error_message(strerror(errno));
		ut_handle_error_message("galileanInitConverterFromProduct(): "
		    "Couldn't combine converters");
	    }
	    else {
		retCode = 0;
	    }
	}

	cv_free(fromUnderlying);
    }				/* "fromUnderlying" allocated */

    return retCode;
}


static ut_status
galileanAcceptVisitor(
    const ut_unit* const		unit,
    const ut_visitor* const	visitor,
    void* const			arg)
{
    assert(unit != NULL);
    assert(IS_GALILEAN(unit));
    assert(visitor != NULL);

    return visitor->visit_galilean(unit, unit->galilean.scale,
	unit->galilean.unit, unit->galilean.offset, arg);
}


static UnitOps	galileanOps = {
    galileanGetProduct,
    galileanClone,
    galileanFree,
    galileanCompare,
    galileanMultiply,
    galileanRaise,
    galileanRoot,
    galileanInitConverterToProduct,
    galileanInitConverterFromProduct,
    galileanAcceptVisitor
};


/******************************************************************************
 * Timestamp Unit:
 ******************************************************************************/


static UnitOps	timestampOps;


/*
 * Returns a new unit instance.
 *
 * Arguments:
 *	unit	The underlying unit.  May be freed upon return.
 *	origin	The timestamp origin.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_OS		Operating-system error.  See "errno".
 *		    UT_MEANINGLESS	Creation of a timestamp unit based on
 *					"unit" is not meaningful.
 *		    UT_NO_SECOND	The associated unit-system doesn't
 *					contain a second unit.
 *	else	The newly-allocated, timestamp-unit.
 */
static ut_unit*
timestampNewOrigin(
    const ut_unit*	unit,
    const double	origin)
{
    ut_unit*    newUnit = NULL;         /* failure */
    ut_unit*    secondUnit;

    assert(unit != NULL);
    assert(!IS_TIMESTAMP(unit));

    secondUnit = unit->common.system->second;

    if (secondUnit == NULL) {
	ut_set_status(UT_NO_SECOND);
	ut_handle_error_message("galileanInitConverterFromProduct(): "
	    "No \"second\" unit defined");
    }
    else if (ut_are_convertible(secondUnit, unit)) {
	TimestampUnit*	timestampUnit = malloc(sizeof(TimestampUnit));

	if (timestampUnit == NULL) {
	    ut_set_status(UT_OS);
	    ut_handle_error_message(strerror(errno));
	    ut_handle_error_message("timestampNewOrigin(): "
		"Couldn't allocate %lu-byte timestamp-unit",
		sizeof(TimestampUnit));
	}
	else {
	    if (commonInit(&timestampUnit->common, &timestampOps,
		    unit->common.system, TIMESTAMP) == 0) {
		timestampUnit->origin = origin;
		timestampUnit->unit = CLONE(unit);
	    }
	    else {
		free(timestampUnit);
		timestampUnit = NULL;
	    }
	}			/* "timestampUnit" allocated */

	newUnit = (ut_unit*)timestampUnit;
    }				/* "secondUnit != NULL" && time unit */

    return newUnit;
}


/*
 * Returns a new unit instance.
 *
 * Arguments:
 *	unit	The underlying unit.  May be freed upon return.
 *	year	The year of the origin.
 *	month	The month of the origin.
 *	day	The day of the origin.
 *	hour	The hour of the origin.
 *	minute	The minute of the origin.
 *	second	The second of the origin.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_OS		Operating-system error.  See "errno".
 *		    UT_MEANINGLESS	Creation of a timestamp unit base on
 *					"unit" is not meaningful.
 *		    UT_NO_SECOND	The associated unit-system doesn't
 *					contain a unit named "second".
 *	else	The newly-allocated, timestamp-unit.
 */
static ut_unit*
timestampNew(
    ut_unit*	unit,
    const int	year,
    const int	month,
    const int	day,
    const int	hour,
    const int	minute,
    double	second)
{
    assert(unit != NULL);

    return timestampNewOrigin(
	unit, ut_encode_time(year, month, day, hour, minute, second));
}


static ProductUnit*
timestampGetProduct(
    const ut_unit* const	unit)
{
    assert(unit != NULL);
    assert(IS_TIMESTAMP(unit));

    return GET_PRODUCT(unit->timestamp.unit);
}


static ut_unit*
timestampClone(
    const ut_unit* const	unit)
{
    assert(unit != NULL);
    assert(IS_TIMESTAMP(unit));

    return timestampNewOrigin(unit->timestamp.unit, unit->timestamp.origin);
}


static int
timestampCompare(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    int	cmp;

    assert(unit1 != NULL);
    assert(IS_TIMESTAMP(unit1));
    assert(unit2 != NULL);

    if (!IS_TIMESTAMP(unit2)) {
	int	diff = unit1->common.type - unit2->common.type;

	cmp = diff < 0 ? -1 : diff == 0 ? 0 : 1;
    }
    else {
	const TimestampUnit* const	timestamp1 = &unit1->timestamp;
	const TimestampUnit* const	timestamp2 = &unit2->timestamp;

	cmp =
	    timestamp1->origin < timestamp2->origin
		? -1
		: timestamp1->origin == timestamp2->origin
		    ? 0
		    : -1;

	if (cmp == 0)
	    cmp = COMPARE(timestamp1->unit, timestamp2->unit);
    }

    return cmp;
}


static void
timestampFree(
    ut_unit* const	unit)
{
    if (unit != NULL) {
	assert(IS_TIMESTAMP(unit));
	FREE(unit->timestamp.unit);
	unit->timestamp.unit = NULL;
	cv_free(unit->common.toProduct);
	unit->common.toProduct = NULL;
	cv_free(unit->common.fromProduct);
	unit->common.fromProduct = NULL;
	free((void*)unit);
    }
}


/*
 * Multiplies a timestamp-unit by another unit.  The origin is ignored.
 *
 * Arguments:
 *	unit1	The timestamp-unit.
 *	unit2	The other unit.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given units is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
timestampMultiply(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    assert(unit1 != NULL);
    assert(IS_TIMESTAMP(unit1));
    assert(unit2 != NULL);

    return MULTIPLY(unit1->timestamp.unit, unit2);
}


/*
 * Returns the result of raising a Timestamp-unit to a power.  The origin is
 * ignored.
 *
 * Arguments:
 *	unit	The Timestamp-unit.
 *	power	The power.  Must be greater than -256 and less than 256.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given units is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
timestampRaise(
    const ut_unit* const	unit,
    const int			power)
{
    assert(unit != NULL);
    assert(IS_TIMESTAMP(unit));
    assert(power != 0);
    assert(power != 1);

    return RAISE(unit->timestamp.unit, power);
}


/*
 * Returns the result of taking a root of a Timestamp-unit.  The origin is
 * ignored.
 *
 * Arguments:
 *	unit	The Timestamp-unit.
 *	root	The root.  Must be greater than 1 and less than 256.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given units is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
timestampRoot(
    const ut_unit* const	unit,
    const int			root)
{
    assert(unit != NULL);
    assert(IS_TIMESTAMP(unit));
    assert(root > 1 && root < 256);

    return ROOT(unit->timestamp.unit, root);
}


/*
 * Initializes the converter of numeric values from the given Timestamp unit to
 * the underlying product-unit.
 *
 * Arguments:
 *	unit	The Timestamp unit.
 * Returns:
 *	-1	Failure.
 */
static int
timestampInitConverterToProduct(
    ut_unit* const	unit)
{
    /*
     * This function is never called.
     */
    assert(0);

    return -1;
}


/*
 * Initializes the converter of numeric values to the given Timestamp unit from
 * the underlying product-unit.
 *
 * Arguments:
 *	unit	The Timestamp unit.
 * Returns:
 *	-1	Failure.
 */
static int
timestampInitConverterFromProduct(
    ut_unit* const	unit)
{
    /*
     * This function is never called.
     */
    assert(0);

    return -1;
}


static ut_status
timestampAcceptVisitor(
    const ut_unit* const		unit,
    const ut_visitor* const	visitor,
    void* const			arg)
{
    assert(unit != NULL);
    assert(IS_TIMESTAMP(unit));
    assert(visitor != NULL);

    return visitor->visit_timestamp(unit, unit->timestamp.unit,
        unit->timestamp.origin, arg);
}


static UnitOps	timestampOps = {
    timestampGetProduct,
    timestampClone,
    timestampFree,
    timestampCompare,
    timestampMultiply,
    timestampRaise,
    timestampRoot,
    timestampInitConverterToProduct,
    timestampInitConverterFromProduct,
    timestampAcceptVisitor
};


/******************************************************************************
 * Logarithmic Unit:
 ******************************************************************************/


static UnitOps	logOps;


/*
 * Returns a new instance.
 *
 * Arguments:
 *	base		The logarithmic base (e.g., 2, M_E, 10).  Must be
 *                      greater than one.
 *	reference	The reference value.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_OS	Operating-system error.  See "errno".
 *	else	The newly-allocated, logarithmic-unit.
 */
static ut_unit*
logNew(
    const double		base,
    const ut_unit* const	reference)
{
    LogUnit*	logUnit;

    assert(base > 1);
    assert(reference != NULL);

    logUnit = malloc(sizeof(LogUnit));

    if (logUnit == NULL) {
	ut_set_status(UT_OS);
	ut_handle_error_message(strerror(errno));
	ut_handle_error_message(
	    "logNew(): Couldn't allocate %lu-byte logarithmic-unit",
	    sizeof(LogUnit));
    }
    else {
	if (commonInit(&logUnit->common, &logOps, reference->common.system,
		LOG) != 0) {
	    free(logUnit);
	}
	else {
	    logUnit->reference = CLONE(reference);

	    if (logUnit->reference != NULL) {
		logUnit->base = base;
	    }
	    else {
		free(logUnit);
		logUnit = NULL;
	    }
	}
    }

    return (ut_unit*)logUnit;
}


static ProductUnit*
logGetProduct(
    const ut_unit* const	unit)
{
    assert(unit != NULL);
    assert(IS_LOG(unit));

    return GET_PRODUCT(unit->log.reference);
}


static ut_unit*
logClone(
    const ut_unit* const	unit)
{
    assert(unit != NULL);
    assert(IS_LOG(unit));

    return logNew(unit->log.base, unit->log.reference);
}


static int
logCompare(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    int	cmp;

    assert(unit1 != NULL);
    assert(IS_LOG(unit1));
    assert(unit2 != NULL);

    if (!IS_LOG(unit2)) {
	int	diff = unit1->common.type - unit2->common.type;

	cmp = diff < 0 ? -1 : diff == 0 ? 0 : 1;
    }
    else {
	const LogUnit* const	u1 = &unit1->log;
	const LogUnit* const	u2 = &unit2->log;

	cmp = ut_compare(u1->reference, u2->reference);

	if (cmp == 0)
	    cmp =
		u1->base < u2->base
		    ? -1
		    : u1->base == u2->base
			? 0
			: 1;
    }

    return cmp;
}


static void
logFree(
    ut_unit* const	unit)
{
    if (unit != NULL) {
	assert(IS_LOG(unit));
	FREE(unit->log.reference);
	unit->log.reference = NULL;
	cv_free(unit->common.toProduct);
	unit->common.toProduct = NULL;
	cv_free(unit->common.fromProduct);
	unit->common.fromProduct = NULL;
	free((void*)unit);
    }
}


/*
 * Multiplies a logarithmic-unit by another unit.
 *
 * Arguments:
 *	unit1	The logarithmic-unit.
 *	unit2	The other unit.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given units is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
logMultiply(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    ut_unit*	result = NULL;		/* failure */

    assert(unit1 != NULL);
    assert(IS_LOG(unit1));
    assert(unit2 != NULL);

    if (!ut_is_dimensionless(unit2)) {
	ut_set_status(UT_MEANINGLESS);
	ut_handle_error_message("logMultiply(): Second unit not dimensionless");
    }
    else if (IS_BASIC(unit2) || IS_PRODUCT(unit2)) {
	result = CLONE(unit1);
    }
    else if (IS_GALILEAN(unit2)) {
	result = galileanNew(unit2->galilean.scale, unit1, 0);
    }
    else {
	ut_set_status(UT_MEANINGLESS);
	ut_handle_error_message("logMultiply(): can't multiply second unit");
    }

    return result;
}


/*
 * Returns the result of raising a logarithmic-unit to a power.
 *
 * Arguments:
 *	unit	The logarithmic-unit.
 *	power	The power.  Must be greater than -256 and less than 256.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given units is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
logRaise(
    const ut_unit* const	unit,
    const int			power)
{
    assert(unit != NULL);
    assert(IS_LOG(unit));
    assert(power != 0);
    assert(power != 1);

    /*
     * We don't know how to raise a logarithmic unit to a non-zero power.
     */
    ut_set_status(UT_MEANINGLESS);
    ut_handle_error_message(
	"logRaise(): Can't raise logarithmic-unit to non-zero power");

    return NULL;
}


/*
 * Returns the result of taking a root of a logarithmic-unit.
 *
 * Arguments:
 *	unit	The logarithmic-unit.
 *	root	The root.  Must be greater than 1 and less than 256.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_MEANINGLESS	The operation on the given units is
 *					meaningless.
 *	else	The resulting unit.
 */
static ut_unit*
logRoot(
    const ut_unit* const	unit,
    const int			root)
{
    assert(unit != NULL);
    assert(IS_LOG(unit));
    assert(root > 1 && root < 256);

    /*
     * We don't know how to take a root of a logarithmic unit.
     */
    ut_set_status(UT_MEANINGLESS);
    ut_handle_error_message(
	"logRoot(): Can't take a non-unity root of a logarithmic-unit");

    return NULL;
}


/*
 * Initializes the converter of numeric values from the given logarithmic unit
 * to the underlying product-unit.
 *
 * Arguments:
 *	unit	The logarithmic unit.
 * Returns:
 *	-1	Failure.  "ut_get_status()" will be:
 *		    UT_OS	Operating-system fault.  See "errno".
 *	0	Success.
 */
static int
logInitConverterToProduct(
    ut_unit* const	unit)
{
    int			retCode = -1;		/* failure */
    cv_converter*	toUnderlying;

    assert(unit != NULL);
    assert(IS_LOG(unit));

    toUnderlying = cv_get_pow(unit->log.base);

    if (toUnderlying == NULL) {
	ut_set_status(UT_OS);
	ut_handle_error_message(strerror(errno));
	ut_handle_error_message("logInitConverterToProduct(): "
	    "Couldn't get converter to underlying unit");
    }
    else {
	if (ENSURE_CONVERTER_TO_PRODUCT(unit->log.reference)) {
	    assert(unit->common.toProduct == NULL);

	    unit->common.toProduct = cv_combine(
		toUnderlying, unit->log.reference->common.toProduct);

	    if (unit->common.toProduct == NULL) {
		ut_set_status(UT_OS);
		ut_handle_error_message(strerror(errno));
		ut_handle_error_message("logInitConverterToProduct(): "
		    "Couldn't combine converters");
	    }
	    else {
		retCode = 0;
	    }
	}

	cv_free(toUnderlying);
    }				/* "toUnderlying" allocated */

    return retCode;
}


/*
 * Initializes the converter of numeric values to the given logarithmic unit
 * from the underlying product-unit.
 *
 * Arguments:
 *	unit	The logarithmic unit.
 * Returns:
 *	-1	Failure.  "ut_get_status()" will be:
 *		    UT_OS		Operating-system fault.  See "errno".
 *	0	Success.
 */
static int
logInitConverterFromProduct(
    ut_unit* const	unit)
{
    int			retCode = -1;		/* failure */
    cv_converter*	fromUnderlying;

    assert(unit != NULL);
    assert(IS_LOG(unit));

    fromUnderlying = cv_get_log(unit->log.base);

    if (fromUnderlying == NULL) {
	ut_set_status(UT_OS);
	ut_handle_error_message(strerror(errno));
	ut_handle_error_message("logInitConverterFromProduct(): "
	    "Couldn't get converter from underlying unit");
    }
    else {
	if (ENSURE_CONVERTER_FROM_PRODUCT(unit->log.reference)) {
	    assert(unit->common.fromProduct == NULL);

	    unit->common.fromProduct = cv_combine(
		unit->log.reference->common.fromProduct, fromUnderlying);

	    if (unit->common.fromProduct == NULL) {
		ut_set_status(UT_OS);
		ut_handle_error_message(strerror(errno));
		ut_handle_error_message("logInitConverterFromProduct(): "
		    "Couldn't combine converters");
	    }
	    else {
		retCode = 0;
	    }
	}

	cv_free(fromUnderlying);
    }				/* "fromUnderlying" allocated */

    return retCode;
}


static ut_status
logAcceptVisitor(
    const ut_unit* const		unit,
    const ut_visitor* const	visitor,
    void* const			arg)
{
    assert(unit != NULL);
    assert(IS_LOG(unit));
    assert(visitor != NULL);

    return visitor->visit_logarithmic(unit, unit->log.base, unit->log.reference,
	arg);
}


static UnitOps	logOps = {
    logGetProduct,
    logClone,
    logFree,
    logCompare,
    logMultiply,
    logRaise,
    logRoot,
    logInitConverterToProduct,
    logInitConverterFromProduct,
    logAcceptVisitor
};


/******************************************************************************
 * Public API:
 ******************************************************************************/


/*
 * Returns a new unit-system.  On success, the unit-system will only contain
 * the dimensionless unit one.  See "ut_get_dimensionless_unit_one()".
 *
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_OS	Operating-system error.  See "errno".
 *	else	Pointer to a new unit system.
 */
ut_system*
ut_new_system(void)
{
    ut_system*	system = malloc(sizeof(ut_system));

    ut_set_status(UT_SUCCESS);

    if (system == NULL) {
	ut_set_status(UT_OS);
	ut_handle_error_message(strerror(errno));
	ut_handle_error_message(
	    "ut_new_system(): Couldn't allocate %lu-byte unit-system",
	    sizeof(ut_system));
    }
    else {
	system->second = NULL;
	system->basicUnits = NULL;
	system->basicCount = 0;

	system->one = (ut_unit*)productNew(system, NULL, NULL, 0);

	if (ut_get_status() != UT_SUCCESS) {
	    ut_handle_error_message(
		"ut_new_system(): Couldn't create dimensionless unit one");
	    free(system);
	}
    }

    return system;
}


/*
 * Frees resources associated with a unit-system by this module.
 *
 * Arguments:
 *	system		Pointer to the unit-system.
 */
void
coreFreeSystem(
    ut_system*	system)
{
    if (system != NULL) {
	int	i;

	for (i = 0; i < system->basicCount; ++i)
	    basicFree((ut_unit*)system->basicUnits[i]);

	free(system->basicUnits);

	if (system->second != NULL)
	    FREE(system->second);

	if (system->one != NULL)
	    productReallyFree(system->one);

	free(system);
    }
}


/*
 * Returns the dimensionless-unit one of a unit-system.
 *
 * Arguments:
 *	system	Pointer to the unit-system for which the dimensionless-unit one
 *		will be returned.
 * Returns:
 *	NULL	Failure.  "utgetStatus()" will be:
 *		    UT_BAD_ARG	"system" is NULL.
 *	else	Pointer to the dimensionless-unit one associated with "system".
 *		While not necessary, the pointer may be passed to ut_free()
 *		when the unit is no longer needed by the client.
 */
ut_unit*
ut_get_dimensionless_unit_one(
    const ut_system* const	system)
{
    ut_unit*	one;

    ut_set_status(UT_SUCCESS);

    if (system == NULL) {
	one = NULL;
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message(
	    "ut_get_dimensionless_unit_one(): NULL unit-system argument");
    }
    else {
	one = system->one;
    }

    return one;
}


/*
 * Returns the unit-system to which a unit belongs.
 *
 * Arguments:
 *	unit	Pointer to the unit in question.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be
 *		    UT_BAD_ARG	"unit" is NULL.
 *	else	Pointer to the unit-system to which "unit" belongs.
 */
ut_system*
ut_get_system(
    const ut_unit* const	unit)
{
    ut_system*	system;

    ut_set_status(UT_SUCCESS);

    if (unit == NULL) {
	system = NULL;
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_get_system(): NULL unit argument");
    }
    else {
	system = unit->common.system;
    }

    return system;
}


/*
 * Indicates if two units belong to the same unit-system.
 *
 * Arguments:
 *	unit1		Pointer to a unit.
 *	unit2		Pointer to another unit.
 * Returns:
 *	0		Failure or the units belong to different unit-systems.
 *			"ut_get_status()" will be
 *	    		    UT_BAD_ARG		"unit1" or "unit2" is NULL.
 *	    		    UT_SUCCESS		The units belong to different
 *						unit-systems.
 *	else		The units belong to the same unit-system.
 */
int
ut_same_system(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    int	sameSystem = 0;

    if (unit1 == NULL || unit2 == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_same_system(): NULL argument");
    }
    else {
	ut_set_status(UT_SUCCESS);
	sameSystem = unit1->common.system == unit2->common.system;
    }

    return sameSystem;
}


/*
 * Returns a new basic-unit that has been added to a unit-system.
 *
 * Arguments:
 *	system		The unit-system to which to add the new basic-unit.
 *	isDimensionless	Whether or not the basic-unit is dimensionless (e.g.,
 *			a radian).
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be
 *		    UT_BAD_ARG		"system" is NULL.
 *		    UT_OS		Operating-system error.  See "errno".
 *	else	Pointer to the new base-unit.
 */
static BasicUnit*
newBasicUnit(
    ut_system* const	system,
    const int		isDimensionless)
{
    BasicUnit*	basicUnit = NULL;	/* failure */

    if (system == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("newBasicUnit(): NULL unit-system argument");
    }
    else {
	basicUnit = basicNew(system, isDimensionless, system->basicCount);

	if (basicUnit != NULL) {
	    int		error = 1;
	    BasicUnit*	save = (BasicUnit*)basicClone((ut_unit*)basicUnit);

	    if (save == NULL) {
		ut_set_status(UT_OS);
		ut_handle_error_message(strerror(errno));
		ut_handle_error_message(
		    "newBasicUnit(): Couldn't clone basic-unit");
	    }
	    else {
		BasicUnit**	basicUnits = realloc(system->basicUnits,
		    (system->basicCount+1)*sizeof(BasicUnit*));

		if (basicUnits == NULL) {
		    ut_set_status(UT_OS);
		    ut_handle_error_message(strerror(errno));
		    ut_handle_error_message("newBasicUnit(): "
			"Couldn't allocate %d-element basic-unit array",
			system->basicCount+1);
		}
		else {
		    basicUnits[system->basicCount++] = save;
		    system->basicUnits = basicUnits;
		    error = 0;
		}			/* "system->basicUnits" re-allocated */

		if (error)
		    basicFree((ut_unit*)save);
	    }				/* "save" allocated */

	    if (error) {
		basicFree((ut_unit*)basicUnit);
		basicUnit = NULL;
	    }
	}				/* "basicUnit" allocated */
    }					/* valid arguments */

    return basicUnit;
}


/*
 * Adds a base-unit to a unit-system.  Clients that use ut_read_xml() should not
 * normally need to call this function.
 *
 * Arguments:
 *	system	Pointer to the unit-system to which to add the new base-unit.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be
 *		    UT_BAD_ARG		"system" or "name" is NULL.
 *		    UT_OS		Operating-system error.  See "errno".
 *	else	Pointer to the new base-unit.  The pointer should be passed to
 *		ut_free() when the unit is no longer needed by the client (the
 *		unit will remain in the unit-system).
 */
ut_unit*
ut_new_base_unit(
    ut_system* const	system)
{
    BasicUnit*	basicUnit = newBasicUnit(system, 0);

    return (ut_unit*)basicUnit;
}


/*
 * Adds a dimensionless-unit to a unit-system.  In the SI system of units, the
 * derived-unit radian is a dimensionless-unit.  Clients that use ut_read_xml()
 * should not normally need to call this function.
 *
 * Arguments:
 *	system	Pointer to the unit-system to which to add the new
 *		dimensionless-unit.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be
 *		    UT_BAD_ARG		"system" is NULL.
 *		    UT_OS		Operating-system error.  See "errno".
 *	else	Pointer to the new dimensionless-unit.  The pointer should be
 *		passed to ut_free() when the unit is no longer needed by the
 *		client (the unit will remain in the unit-system).
 */
ut_unit*
ut_new_dimensionless_unit(
    ut_system* const	system)
{
    return (ut_unit*)newBasicUnit(system, 1);
}


/*
 * Sets the "second" unit of a unit-system.  This function must be called before
 * the first call to "ut_offset_by_time()". ut_read_xml() calls this function if the
 * resulting unit-system contains a unit named "second".
 *
 * Arguments:
 *	second		Pointer to the "second" unit.
 * Returns:
 *	UT_BAD_ARG	"second" is NULL.
 *	UT_EXISTS	The second unit of the unit-system to which "second"
 *			belongs is set to a different unit.
 *	UT_SUCCESS	Success.
 */
ut_status
ut_set_second(
    const ut_unit* const	second)
{
    ut_set_status(UT_SUCCESS);

    if (second == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message(
            "ut_set_second(): NULL \"second\" unit argument");
    }
    else {
	ut_system*	system = second->common.system;

	if (system->second == NULL) {
	    system->second = CLONE(second);
	}
	else {
	    if (ut_compare(system->second, second) != 0) {
		ut_set_status(UT_EXISTS);
		ut_handle_error_message(
		    "ut_set_second(): Different \"second\" unit already "
                    "defined");
	    }
	}
    }

    return ut_get_status();
}


/*
 * Compares two units.  Returns a value less than, equal to, or greater than
 * zero as the first unit is considered less than, equal to, or greater than
 * the second unit, respectively.  Units from different unit-systems never
 * compare equal.
 *
 * Arguments:
 *	unit1		Pointer to a unit or NULL.
 *	unit2		Pointer to another unit or NULL.
 * Returns:
 *	<0	The first unit is less than the second unit.
 *	 0	The first and second units are equal or both units are NULL.
 *	>0	The first unit is greater than the second unit.
 */
int
ut_compare(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    int	cmp = 0;

    ut_set_status(UT_SUCCESS);

    if (unit1 == NULL) {
	cmp = unit2 != NULL ? -1 : 0;
    }
    else if (unit2 == NULL) {
	cmp = 1;
    }
    else if (unit1->common.system < unit2->common.system) {
	cmp = -1;
    }
    else if (unit1->common.system > unit2->common.system) {
	cmp = 1;
    }
    else {
	/*
	 * NB: The comparison function is called if and only if the units
	 * belong to the same unit-system.
	 */
	cmp = COMPARE(unit1, unit2);
    }

    return cmp;
}


/*
 * Returns a unit equivalent to another unit scaled by a numeric factor,
 * e.g.,
 *	const ut_unit*	meter = ...
 *	const ut_unit*	kilometer = ut_scale(1000, meter);
 *
 * Arguments:
 *	factor		The numeric scale factor.
 *	unit		Pointer to the unit to be scaled.
 * Returns:
 *	NULL		Failure.  "ut_get_status()" will be
 *			    UT_BAD_ARG  	"factor" is 0 or "unit" is NULL.
 *			    UT_OS		Operating-system error.  See
 *						"errno".
 *	else		Pointer to the resulting unit.  The pointer should be
 *			passed to ut_free() when the unit is no longer needed by
 *			the client.
 */
ut_unit*
ut_scale(
    const double		factor,
    const ut_unit* const	unit)
{
    ut_unit*		result = NULL;	/* failure */

    ut_set_status(UT_SUCCESS);

    if (unit == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_scale(): NULL unit argument");
    }
    else {
	if (factor == 0) {
	    ut_set_status(UT_BAD_ARG);
	    ut_handle_error_message("ut_scale(): NULL factor argument");
	}
	else {
	    result = factor == 1
		? CLONE(unit)
		: galileanNew(factor, unit, 0.0);
	}
    }

    return result;
}


/*
 * Returns a unit equivalent to another unit offset by a numeric amount,
 * e.g.,
 *	const ut_unit*	kelvin = ...
 *	const ut_unit*	celsius = ut_offset(kelvin, 273.15);
 *
 * Arguments:
 *	unit		Pointer to the unit to be offset.
 *	offset		The numeric offset.
 * Returns:
 *	NULL		Failure.  "ut_get_status()" will be
 *			    UT_BAD_ARG		"unit" is NULL.
 *			    UT_OS		Operating-system error.  See
 *						"errno".
 *	else		Pointer to the resulting unit.  The pointer should be
 *			passed to ut_free() when the unit is no longer needed by
 *			the client.
 */
ut_unit*
ut_offset(
    const ut_unit* const	unit,
    const double		offset)
{
    ut_unit*		result = NULL;	/* failure */

    ut_set_status(UT_SUCCESS);

    if (unit == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_offset(): NULL unit argument");
    }
    else {
	result = offset == 0
	    ? CLONE(unit)
	    : galileanNew(1.0, unit, offset);
    }

    return result;
}


/*
 * Returns a unit equivalent to another unit relative to a particular time.
 * e.g.,
 *	const ut_unit*	second = ...
 *	const ut_unit*	secondsSinceTheEpoch =
 *              ut_offset_by_time(second, ut_encode_time(1970, 1, 1, 0, 0, 0.0));
 *
 * Arguments:
 *	unit	Pointer to the time-unit to be made relative to a time-origin.
 *	origin	The origin as returned by ut_encode_time().
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be
 *		    UT_BAD_ARG		"unit" is NULL.
 *		    UT_OS		Operating-system error.  See "errno".
 *		    UT_MEANINGLESS	Creation of a timestamp unit based on
 *					"unit" is not meaningful.
 *		    UT_NO_SECOND	The associated unit-system doesn't
 *					contain a "second" unit.  See
 *					ut_set_second().
 *	else	Pointer to the resulting unit.  The pointer should be passed
 *		to ut_free() when the unit is no longer needed by the client.
 */
ut_unit*
ut_offset_by_time(
    const ut_unit* const	unit,
    const double		origin)
{
    ut_unit*		result = NULL;	/* failure */

    ut_set_status(UT_SUCCESS);

    if (unit == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_offset_by_time(): NULL unit argument");
    }
    else {
	result = timestampNewOrigin(unit, origin);
    }

    return result;
}


/*
 * Returns the result of multiplying one unit by another unit.
 *
 * Arguments:
 *	unit1	Pointer to a unit.
 *	unit2	Pointer to another unit.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_BAD_ARG		"unit1" or "unit2" is NULL.
 *		    UT_NOT_SAME_SYSTEM	"unit1" and "unit2" belong to
 *					different unit-systems.
 *		    UT_OS		Operating-system error. See "errno".
 *	else	Pointer to the resulting unit.  The pointer should be passed
 *		to ut_free() when the unit is no longer needed by the client.
 */
ut_unit*
ut_multiply(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    ut_unit*	result = NULL;	/* failure */

    ut_set_status(UT_SUCCESS);

    if (unit1 == NULL || unit2 == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_multiply(): NULL argument");
    }
    else if (unit1->common.system != unit2->common.system) {
	ut_set_status(UT_NOT_SAME_SYSTEM);
	ut_handle_error_message(
            "ut_multiply(): Units in different unit-systems");
    }
    else {
	result = MULTIPLY(unit1, unit2);
    }

    return result;
}


/*
 * Returns the inverse (i.e., reciprocal) of a unit.  This convenience function
 * is equal to "ut_raise(unit, -1)".
 *
 * Arguments:
 *	unit	Pointer to the unit.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_BAD_ARG		"unit" is NULL.
 *		    UT_OS		Operating-system error. See "errno".
 *	else	Pointer to the resulting unit.  The pointer should be passed to
 *		ut_free() when the unit is no longer needed by the client.
 */
ut_unit*
ut_invert(
    const ut_unit* const	unit)
{
    return ut_raise(unit, -1);
}


/*
 * Returns the result of dividing one unit by another unit.  This convenience
 * function is equivalent to the following sequence:
 *     {
 *         ut_unit* inverse = ut_invert(denom);
 *         ut_multiply(numer, inverse);
 *         ut_free(inverse);
 *     }
 *
 * Arguments:
 *	numer	Pointer to the numerator (top, dividend) unit.
 *	denom	Pointer to the denominator (bottom, divisor) unit.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_BAD_ARG		"numer" or "denom" is NULL.
 *		    UT_NOT_SAME_SYSTEM	"unit1" and "unit2" belong to
 *					different unit-systems.
 *		    UT_OS		Operating-system error. See "errno".
 *	else	Pointer to the resulting unit.  The pointer should be passed to
 *		ut_free() when the unit is no longer needed by the client.
 */
ut_unit*
ut_divide(
    const ut_unit* const	numer,
    const ut_unit* const	denom)
{
    ut_unit*	result = NULL;		/* failure */

    ut_set_status(UT_SUCCESS);

    if (numer == NULL || denom == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_divide(): NULL argument");
    }
    else if (numer->common.system != denom->common.system) {
	ut_set_status(UT_NOT_SAME_SYSTEM);
	ut_handle_error_message("ut_divide(): Units in different unit-systems");
    }
    else {
	ut_unit*	inverse = RAISE(denom, -1);

	if (inverse != NULL) {
	    result = MULTIPLY(numer, inverse);

	    ut_free(inverse);
	}
    }

    return result;
}


/*
 * Returns the result of raising a unit to a power.
 *
 * Arguments:
 *	unit	Pointer to the unit.
 *	power	The power by which to raise "unit".  Must be greater than or 
 *		equal to -255 and less than or equal to 255.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_BAD_ARG		"unit" is NULL, or "power" is invalid.
 *		    UT_OS		Operating-system error. See "errno".
 *	else	Pointer to the resulting unit.  The pointer should be passed to
 *		ut_free() when the unit is no longer needed by the client.
 */
ut_unit*
ut_raise(
    const ut_unit* const	unit,
    const int			power)
{
    ut_unit*		result = NULL;	/* failure */

    ut_set_status(UT_SUCCESS);

    if (unit == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_raise(): NULL unit argument");
    }
    else if (power < -255 || power > 255) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_raise(): Invalid power argument");
    }
    else {
	result = 
	    power == 0
		? unit->common.system->one
		: power == 1
		    ? CLONE(unit)
		    : RAISE(unit, power);
    }

    return result;
}


/*
 * Returns the result of taking the root of a unit.
 *
 * Arguments:
 *	unit	Pointer to the unit.
 *	root	The root to take of "unit".  Must be greater than or 
 *		equal to 1 and less than or equal to 255.
 * Returns:
 *	NULL	Failure.  "ut_get_status()" will be:
 *		    UT_BAD_ARG		"unit" is NULL, or "root" is invalid.
 *		                        In particular, all powers of base units
 *		                        in "unit" must be integral multiples of
 *		                        "root".
 *		    UT_OS		Operating-system error. See "errno".
 *	else	Pointer to the resulting unit.  The pointer should be passed to
 *		ut_free() when the unit is no longer needed by the client.
 */
ut_unit*
ut_root(
    const ut_unit* const	unit,
    const int			root)
{
    ut_unit*		result = NULL;	/* failure */

    ut_set_status(UT_SUCCESS);

    if (unit == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_root(): NULL unit argument");
    }
    else if (root < 1 || root > 255) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_root(): Invalid root argument");
    }
    else {
	result = 
	    root == 1
		? CLONE(unit)
                : ROOT(unit, root);
    }

    return result;
}


/*
 * Returns the logarithmic unit corresponding to a logarithmic base and a
 * reference level.  For example, the following creates a decibel unit with a
 * one milliwatt reference level:
 *
 *     const ut_unit* watt = ...;
 *     const ut_unit* milliWatt = ut_scale(0.001, watt);
 *
 *     if (milliWatt != NULL) {
 *         const ut_unit* bel_1_mW = ut_log(10.0, milliWatt);
 *
 *         if (bel_1_mW != NULL) {
 *             const ut_unit* decibel_1_mW = ut_scale(0.1, bel_1_mW);
 *
 *             if (decibel_1_mW != NULL) {
 *                 ...
 *                 ut_free(decibel_1_mW);
 *             }			// "decibel_1_mW" allocated
 *
 *             ut_free(bel_1_mW);
 *         }				// "bel_1_mW" allocated
 *
 *         ut_free(milliWatt);
 *     }				// "milliWatt" allocated
 *
 * Arguments:
 *	base		The logarithmic base (e.g., 2, M_E, 10).  Must be
 *                      greater than one.  "M_E" is defined in <math.h>.
 *	reference	Pointer to the reference value as a unit.
 * Returns:
 *	NULL		Failure.  "ut_get_status()" will be:
 *			    UT_BAD_ARG  	"base" is invalid or "reference"
 *                                              is NULL.
 *			    UT_OS		Operating-system error. See
 *						"errno".
 *	else		Pointer to the resulting unit.  The pointer should be
 *			passed to ut_free() when the unit is no longer needed by
 *			the client.
 */
ut_unit*
ut_log(
    const double		base,
    const ut_unit* const	reference)
{
    ut_unit*		result = NULL;	/* failure */

    ut_set_status(UT_SUCCESS);

    if (base <= 1) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_log(): Invalid logarithmic base, %g", base);
    }
    else if (reference == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_log(): NULL reference argument");
    }
    else {
	result = logNew(base, reference);
    }

    return result;
}


/*
 * Indicates if numeric values in one unit are convertible to numeric values in
 * another unit via "ut_get_converter()".  In making this determination, 
 * dimensionless units are ignored.
 *
 * Arguments:
 *	unit1		Pointer to a unit.
 *	unit2		Pointer to another unit.
 * Returns:
 *	0		Failure.  "ut_get_status()" will be
 *	    		    UT_BAD_ARG		"unit1" or "unit2" is NULL.
 *			    UT_NOT_SAME_SYSTEM	"unit1" and "unit2" belong to
 *						different unit-sytems.
 *			    UT_SUCCESS		Conversion between the units is
 *						not possible (e.g., "unit1" is
 *						"meter" and "unit2" is
 *						"kilogram").
 *	else	Numeric values can be converted between the units.
 */
int
ut_are_convertible(
    const ut_unit* const	unit1,
    const ut_unit* const	unit2)
{
    int			areConvertible = 0;

    if (unit1 == NULL || unit2 == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_are_convertible(): NULL unit argument");
    }
    else if (unit1->common.system != unit2->common.system) {
	ut_set_status(UT_NOT_SAME_SYSTEM);
	ut_handle_error_message(
	    "ut_are_convertible(): Units in different unit-systems");
    }
    else {
	ut_set_status(UT_SUCCESS);

	if (IS_TIMESTAMP(unit1) || IS_TIMESTAMP(unit2)) {
	    areConvertible = IS_TIMESTAMP(unit1) && IS_TIMESTAMP(unit2);
	}
	else {
	    ProductRelationship	relationship =
		productRelationship(GET_PRODUCT(unit1), GET_PRODUCT(unit2));

	    areConvertible = relationship == PRODUCT_EQUAL ||
		relationship == PRODUCT_INVERSE;
	}
    }

    return areConvertible;
}


/*
 * Returns a converter of numeric values in one unit to numeric values in
 * another unit.  The returned converter should be passed to cv_free() when it
 * is no longer needed by the client.
 *
 * NOTE:  Leap seconds are not taken into account when converting between
 * timestamp units.
 *
 * Arguments:
 *	from		Pointer to the unit from which to convert values.
 *	to		Pointer to the unit to which to convert values.
 * Returns:
 *	NULL		Failure.  "ut_get_status()" will be:
 *			    UT_BAD_ARG		"from" or "to" is NULL.
 *			    UT_NOT_SAME_SYSTEM	"from" and "to" belong to
 *						different unit-systems.
 *			    UT_MEANINGLESS	Conversion between the units is
 *						not possible.  See
 *						"ut_are_convertible()".
 *	else		Pointer to the appropriate converter.  The pointer
 *			should be passed to cv_free() when no longer needed by
 *			the client.
 */
cv_converter*
ut_get_converter(
    ut_unit* const	from,
    ut_unit* const	to)
{
    cv_converter*	converter = NULL;	/* failure */

    if (from == NULL || to == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_get_converter(): NULL unit argument");
    }
    else if (from->common.system != to->common.system) {
	ut_set_status(UT_NOT_SAME_SYSTEM);
	ut_handle_error_message(
	    "ut_get_converter(): Units in different unit-systems");
    }
    else {
	ut_set_status(UT_SUCCESS);

	if (!IS_TIMESTAMP(from) && !IS_TIMESTAMP(to)) {
	    ProductRelationship	relationship =
		productRelationship(GET_PRODUCT(from), GET_PRODUCT(to));

	    if (relationship == PRODUCT_UNCONVERTIBLE) {
		ut_set_status(UT_MEANINGLESS);
		ut_handle_error_message(
                    "ut_get_converter(): Units not convertible");
	    }
	    else if (ENSURE_CONVERTER_TO_PRODUCT(from) &&
			ENSURE_CONVERTER_FROM_PRODUCT(to)) {
		if (relationship == PRODUCT_EQUAL) {
		    converter = cv_combine(
			from->common.toProduct, to->common.fromProduct);
		}
		else {
		    /*
		     * The underlying product-units are reciprocals of each
		     * other.
		     */
		    cv_converter*	invert = cv_get_inverse();

		    if (invert != NULL) {
			cv_converter*	phase1 =
			    cv_combine(from->common.toProduct, invert);

			if (phase1 != NULL) {
			    converter =
				cv_combine(phase1, to->common.fromProduct);

			    cv_free(phase1);
			}		/* "phase1" allocated */

			cv_free(invert);
		    }			/* "invert" allocated */
		}			/* reciprocal product-units */

		if (converter == NULL) {
		    ut_set_status(UT_OS);
		    ut_handle_error_message(strerror(errno));
		    ut_handle_error_message(
			"ut_get_converter(): Couldn't get converter");
		}
	    }				/* got necessary product converters */
	}				/* neither unit is a timestamp */
	else {
	    cv_converter*	toSeconds =
                ut_get_converter(from->timestamp.unit,
                    from->common.system->second);

	    if (toSeconds == NULL) {
		ut_set_status(UT_OS);
		ut_handle_error_message(strerror(errno));
		ut_handle_error_message(
		    "ut_get_converter(): Couldn't get converter to seconds");
	    }
	    else {
		cv_converter*	shiftOrigin =
		    cv_get_offset(
			from->timestamp.origin - to->timestamp.origin);

		if (shiftOrigin == NULL) {
		    ut_set_status(UT_OS);
		    ut_handle_error_message(strerror(errno));
		    ut_handle_error_message(
			"ut_get_converter(): Couldn't get offset-converter");
		}
		else {
		    cv_converter*	toToUnit =
			cv_combine(toSeconds, shiftOrigin);

		    if (toToUnit == NULL) {
			ut_set_status(UT_OS);
			ut_handle_error_message(strerror(errno));
			ut_handle_error_message(
			    "ut_get_converter(): Couldn't combine converters");
		    }
		    else {
			cv_converter*	fromSeconds = ut_get_converter(
			    to->common.system->second, to->timestamp.unit); 

			if (fromSeconds == NULL) {
			    ut_set_status(UT_OS);
			    ut_handle_error_message(strerror(errno));
			    ut_handle_error_message(
				"ut_get_converter(): Couldn't get converter "
				"from seconds");
			}
			else {
			    converter = cv_combine(toToUnit, fromSeconds);

			    if (converter == NULL) {
				ut_set_status(UT_OS);
				ut_handle_error_message(strerror(errno));
				ut_handle_error_message("ut_get_converter(): "
				    "Couldn't combine converters");
			    }

			    cv_free(fromSeconds);
			}		/* "fromSeconds" allocated */

			cv_free(toToUnit);
		    }			/* "toToUnit" allocated */

		    cv_free(shiftOrigin);
		}			/* "shiftOrigin" allocated */

		cv_free(toSeconds);
	    }				/* "toSeconds" allocated */
	}				/* units are timestamps */
    }					/* valid arguments */

    return converter;
}


/*
 * Indicates if a given unit is dimensionless or not.  Note that logarithmic
 * units are dimensionless by definition.
 *
 * Arguments:
 *	unit	Pointer to the unit in question.
 * Returns:
 *	0	"unit" is dimensionfull or an error occurred.  "ut_get_status()"
 *		 will be
 *		    UT_BAD_ARG		"unit" is NULL.
 *		    UT_SUCCESS		"unit" is dimensionfull.
 *	else	"unit" is dimensionless.
 */
int
ut_is_dimensionless(
    const ut_unit* const	unit)
{
    int		isDimensionless = 0;

    ut_set_status(UT_SUCCESS);

    if (unit == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_is_dimensionless(): NULL unit argument");
    }
    else {
	/*
	 * Special case logarithmic units because logGetProduct() can be
	 * dimensionfull.
	 */
	isDimensionless =
	    IS_LOG(unit)
		? 1
		: productIsDimensionless(GET_PRODUCT(unit));
    }

    return isDimensionless;
}


/*
 * Returns a clone of a unit.
 *
 * Arguments:
 *	unit	Pointer to the unit to be cloned.
 * Returns:
 *	NULL	Failure.  ut_get_status() will be
 *		    UT_OS	Operating-system failure.  See "errno".
 *		    UT_BAD_ARG	"unit" is NULL.
 *	else	Pointer to the clone of "unit".  The pointer should be
 *		passed to ut_free() when the unit is no longer needed by the
 *		client.
 */
ut_unit*
ut_clone(
    const ut_unit* const	unit)
{
    ut_unit*	clone = NULL;		/* failure */

    ut_set_status(UT_SUCCESS);

    if (unit == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_clone(): NULL unit argument");
    }
    else {
	clone =
	    unit == unit->common.system->one
		? (ut_unit*)unit
		: CLONE(unit);
    }

    return clone;
}


/*
 * Frees resources associated with a unit.  This function should be invoked on
 * all units that are no longer needed by the client.  Use of the unit upon
 * return from this function will result in undefined behavior.
 *
 * Arguments:
 *	unit	Pointer to the unit to have its resources freed or NULL.
 */
void
ut_free(
    ut_unit* const	unit)
{
    ut_set_status(UT_SUCCESS);

    if (unit != NULL) {
	if (unit != unit->common.system->one)
	    FREE(unit);
    }
}


/*
 * Accepts a visitor to a unit.
 *
 * Arguments:
 *	unit		Pointer to the unit to accept the visitor.
 *	visitor		Pointer to the visitor of "unit".
 *	arg		An arbitrary pointer that will be passed to "visitor".
 * Returns:
 *	UT_BAD_ARG	"unit" or "visitor" is NULL.
 *	UT_VISIT_ERROR	A error occurred in "visitor" while visiting "unit".
 *	UT_SUCCESS	Success.
 */
ut_status
ut_accept_visitor(
    const ut_unit* const		unit,
    const ut_visitor* const	visitor,
    void* const			arg)
{
    ut_set_status(UT_SUCCESS);

    if (unit == NULL || visitor == NULL) {
	ut_set_status(UT_BAD_ARG);
	ut_handle_error_message("ut_accept_visitor(): NULL argument");
    }
    else {
	ut_set_status(ACCEPT_VISITOR(unit, visitor, arg));
    }

    return ut_get_status();
}
