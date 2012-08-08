/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */

#include <errno.h>
#include <search.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "udunits.h"

static ut_system*	unitSystem = NULL;
static ut_unit*		encodedTimeUnit = NULL;
static ut_unit*		second = NULL;
static char*		buffer;
static int		buflen = 80;
static void*		unit2s = NULL;

/*
 * Initialize the units(3) package.
 */
int
utInit(
    const char	*path)
{
    int status;

    (void)ut_set_error_message_handler(ut_ignore);
    if (unitSystem != NULL) {
	ut_free_system(unitSystem);
	unitSystem = NULL;
    }
    unitSystem = ut_read_xml(NULL);
    if (unitSystem == NULL) {
	status = ut_get_status() == UT_PARSE
	    ? UT_ESYNTAX
	    : UT_EIO;
    }
    else {
	second = ut_get_unit_by_name(unitSystem, "second");
	encodedTimeUnit =
	    ut_offset_by_time(second, ut_encode_time(2001, 1, 1, 0, 0, 0.0));
	buffer = malloc(buflen);
	if (buffer == NULL) {
	    buflen = 0;
	    status = UT_EALLOC;
	}
	else {
	    status = 0;
	}
    }
    return status;
}

/*
 * Indicate if the units(3) package has been initialized.
 */
int
utIsInit()
{
    return unitSystem != NULL;
}

static int
compare(
    const void*	key1,
    const void*	key2)
{
    return key1 < key2 ? -1 : key1 == key2 ? 0 : 1;
}

static void
freeIfAllocated(
    utUnit* const	unit)
{
    if (tdelete(unit->unit2, &unit2s, compare) != NULL) {
	ut_free(unit->unit2);
    }
    unit->unit2 = NULL;
}

void
utFree(
    utUnit* const	unit)
{
    freeIfAllocated(unit);
}

void
utIni(
    utUnit* const	unit)
{
    if (unit != NULL) {
	unit->unit2 = NULL;
    }
}

static int
setUnit(
    utUnit* const	unit,
    ut_unit* const	unit2)
{
    int	status;

    if (tsearch(unit2, &unit2s, compare) == NULL) {
	status = UT_EALLOC;
    }
    else {
	freeIfAllocated(unit);
	unit->unit2 = unit2;
	status = 0;
    }
    return status;
}

/*
 * Decode a formatted unit specification into a unit-structure.
 */
int
utScan(
    const char	*spec,
    utUnit	*unit)
{
    int		status;

    if (spec == NULL || unit == NULL) {
	status = UT_EINVALID;
    }
    else {
	ut_unit*	ut_unit = ut_parse(unitSystem, spec, UT_ASCII);

	if (ut_unit != NULL) {
	    status = setUnit(unit, ut_unit);
	}
	else {
	    status = ut_get_status();
	    if (status == UT_BAD_ARG) {
		status = unitSystem == NULL
		    ? UT_ENOINIT
		    : UT_EINVALID;
	    }
	    else if (status == UT_SYNTAX) {
		status =  UT_ESYNTAX;
	    }
	    else if (status == UT_UNKNOWN) {
		status =  UT_EUNKNOWN;
	    }
	    else {
		status = UT_EALLOC;
	    }
	}
    }
    return status;
}

/*
 * Convert a temporal value into a UTC Gregorian date and time.
 */
int
utCalendar(
    double		value,
    const utUnit	*unit,
    int			*year,
    int			*month,
    int			*day,
    int			*hour,
    int			*minute,
    float		*second)
{
    int		status = 0;	/* success */

    cv_converter* converter = ut_get_converter(unit->unit2, encodedTimeUnit);
    if (converter == NULL) {
	status = encodedTimeUnit == NULL ? UT_ENOINIT : UT_EINVALID;
    }
    else {
	double	encodedTime = cv_convert_double(converter, value);
	double	sec, res;

	ut_decode_time(encodedTime, year, month, day, hour, minute, &sec, &res);
	*second = (float)sec;
	cv_free(converter);
    }
    return status;
}

/*
 *	Convert a date into a temporal value.  The date is assumed to
 *	use the Gregorian calendar if on or after noon, October 15, 1582;
 *	otherwise, the date is assumed to use the Julian calendar.
 */
int
utInvCalendar(
    int			year,
    int			month,
    int			day,
    int			hour,
    int			minute,
    double		second,
    const utUnit	*unit,
    double		*value)
{
    int		status = 0;	/* success */

    cv_converter* converter = ut_get_converter(encodedTimeUnit, unit->unit2);
    if (converter == NULL) {
	status = encodedTimeUnit == NULL ? UT_ENOINIT : UT_EINVALID;
    }
    else {
	double	encodedTime =
	    ut_encode_time(year, month, day, hour, minute, second);

	*value = cv_convert_double(converter, encodedTime);
	cv_free(converter);
    }
    return status;
}

static ut_status
isTimeVisitBasic(
    const ut_unit*	unit,
    void*		arg)
{
    return (ut_status)ut_are_convertible(unit, second);
}

static ut_status
isTimeVisitProduct(
    const ut_unit*		unit,
    int				count,
    const ut_unit* const*	basicUnits,
    const int*			powers,
    void* 			arg)
{
    int	isTime;
    if (!ut_are_convertible(unit, second)) {
	isTime = 0;
    }
    else {
	int i;

	isTime = 0;
	for (i = 0; i < count; i++) {
	    if (ut_are_convertible(basicUnits[i], second) && powers[i] == 1) {
		isTime = 1;
		break;
	    }
	}
    }
    return (ut_status)isTime;
}

static ut_status
isTimeVisitGalilean(
    const ut_unit*	unit,
    double		scale,
    const ut_unit*	underlyingUnit,
    double		origin,
    void*		arg)
{
    return (ut_status)0;
}

static ut_status
isTimeVisitTimestamp(
    const ut_unit*	unit,
    const ut_unit*	timeUnit,
    double		origin,
    void*		arg)
{
    return (ut_status)1;
}

static ut_status
isTimeVisitLogarithmic(
    const ut_unit*	unit,
    double		base,
    const ut_unit*	reference,
    void*		arg)
{
    return (ut_status)0;
}

/*
 * Indicate if a unit structure refers to a unit of time.
 */
int
utIsTime(
    const utUnit	*up)
{
    ut_visitor	visitor;
    visitor.visit_basic = isTimeVisitBasic;
    visitor.visit_product = isTimeVisitProduct;
    visitor.visit_galilean = isTimeVisitGalilean;
    visitor.visit_timestamp = isTimeVisitTimestamp;
    visitor.visit_logarithmic = isTimeVisitLogarithmic;
    return ut_accept_visitor(up->unit2, &visitor, NULL);
}

static ut_status
hasOriginVisitBasic(
    const ut_unit*	unit,
    void*		arg)
{
    return (ut_status)0;
}

static ut_status
hasOriginVisitProduct(
    const ut_unit*		unit,
    int				count,
    const ut_unit* const*	basicUnits,
    const int*			powers,
    void* 			arg)
{
    return (ut_status)0;
}

static ut_status
hasOriginVisitGalilean(
    const ut_unit*	unit,
    double		scale,
    const ut_unit*	underlyingUnit,
    double		origin,
    void*		arg)
{
    return (ut_status)1;
}

static ut_status
hasOriginVisitTimestamp(
    const ut_unit*	unit,
    const ut_unit*	timeUnit,
    double		origin,
    void*		arg)
{
    return (ut_status)1;
}

static ut_status
hasOriginVisitLogarithmic(
    const ut_unit*	unit,
    double		base,
    const ut_unit*	reference,
    void*		arg)
{
    return (ut_status)0;
}

/*
 * Indicate if a unit structure has an origin.
 */
int
utHasOrigin(
    const utUnit	*up)
{
    ut_visitor	visitor;
    visitor.visit_basic = hasOriginVisitBasic;
    visitor.visit_product = hasOriginVisitProduct;
    visitor.visit_galilean = hasOriginVisitGalilean;
    visitor.visit_timestamp = hasOriginVisitTimestamp;
    visitor.visit_logarithmic = hasOriginVisitLogarithmic;
    return ut_accept_visitor(up->unit2, &visitor, NULL);
}

static utUnit*
resultingUnit(
    utUnit*		result,
    ut_unit* const	unit2)
{
    if (unit2 == NULL) {
	result = NULL;
    }
    else if (result != NULL) {
	if (setUnit(result, unit2) != 0) {
	    result == NULL;
	}
    }
    return result;
}

/*
 * Clear a unit structure.
 */
utUnit*
utClear(
    utUnit	*unit)
{
    return resultingUnit(unit, ut_get_dimensionless_unit_one(unitSystem));
}

/*
 * Copy a unit-structure.
 */
utUnit*
utCopy(
    const utUnit	*source,
    utUnit		*dest)
{
    return source == NULL
	? NULL
	: dest == NULL
	    ? NULL
	    : resultingUnit(dest, ut_clone(source->unit2));
}

/*
 * Multiply one unit-structure by another.
 */
utUnit*
utMultiply(
    const utUnit	*term1,
    const utUnit	*term2, 
    utUnit		*result)
{
    return term1 == NULL || term2 == NULL
	? NULL
	: resultingUnit(result, ut_multiply(term1->unit2, term2->unit2));
}

/*
 * Divide one unit-structure by another.
 */
utUnit*
utDivide(
    const utUnit	*numer,
    const utUnit	*denom, 
    utUnit		*result)
{
    return numer == NULL || denom == NULL
	? NULL
	: resultingUnit(result, ut_divide(numer->unit2, denom->unit2));
}

/*
 * Form the reciprocal of a unit-structure.
 */
utUnit*
utInvert(
    const utUnit	*unit,
    utUnit		*result)
{
    return unit == NULL ? NULL : resultingUnit(result, ut_invert(unit->unit2));
}

/*
 * Raise a unit-structure to a power.
 */
utUnit*
utRaise(
    const utUnit	*unit,
    int			power,
    utUnit		*result)
{
    return unit == NULL
	? NULL
	: resultingUnit(result, ut_raise(unit->unit2, power));
}

/*
 * Shift the origin of a unit-structure by an arithmetic amount.
 */
utUnit*
utShift(
    const utUnit	*unit,
    double		amount,
    utUnit		*result)
{
    return unit == NULL
	? NULL
	: resultingUnit(result, ut_offset(unit->unit2, amount));
}

/*
 * Scale a unit-structure.
 */
utUnit*
utScale(
    const utUnit	*unit,
    double		factor,
    utUnit		*result)
{
    return unit == NULL
	? NULL
	: resultingUnit(result, ut_scale(factor, unit->unit2));
}

/*
 * Compute the conversion factor between two unit-structures.
 */
int
utConvert(
    const utUnit	*from,
    const utUnit	*to,
    double		*slope,
    double		*intercept)
{
    int			status;
    cv_converter*	converter = ut_get_converter(from->unit2, to->unit2);

    if (converter == NULL) {
	status = ut_get_status();

	if (status == UT_BAD_ARG) {
	    status = UT_EINVALID;
	}
	else if (status == UT_NOT_SAME_SYSTEM) {
	    status = UT_ENOINIT;
	}
	else if (status == UT_MEANINGLESS) {
	    status = UT_ECONVERT;
	}
	else {
	    status = UT_EALLOC;
	}
    }
    else {
	*intercept = cv_convert_double(converter, 0.0);
	*slope = cv_convert_double(converter, 1.0) - *intercept;
	status = 0;
    }
    return status;
}

/*
 * Encode a unit-structure into a formatted unit-specification.
 */
int
utPrint(
    const utUnit	*unit,
    char		**buf)
{
    int	status;

    for (;;) {
	int	len = ut_format(unit->unit2, buffer, buflen, UT_ASCII);
	if (len == -1) {
	    status = ut_get_status();

	    if (status == UT_BAD_ARG) {
		status = UT_EINVALID;
	    }
	    else {
		status = UT_EALLOC;
	    }
	    break;
	}
	if (len < buflen) {
	    *buf = buffer;
	    status = 0;
	    break;
	}
	else {
	    int		newLen = buflen * 2;
	    char*	newBuf = malloc(newLen);

	    if (newBuf == NULL) {
		status = UT_EALLOC;
		break;
	    }
	    buffer = newBuf;
	    buflen = newLen;
	}
    }
    return status;
}

/*
 * Add a unit to the units database.
 */
int
utAdd(
    char		*name,
    int			hasPlural,
    const utUnit	*unit)
{
    int	status = ut_map_name_to_unit(name, UT_ASCII, unit->unit2);

    if (status == UT_SUCCESS) {
	status = ut_map_unit_to_name(unit->unit2, name, UT_ASCII);
	if (status == UT_SUCCESS) {
	    if (!hasPlural) {
		status = UT_SUCCESS;
	    }
	    else {
		extern const char*	ut_form_plural(const char*);
		const char*	plural = ut_form_plural(name);

		status = ut_map_name_to_unit(plural, UT_ASCII, unit->unit2);
	    }				/* unit has plural name */
	    if (status != UT_SUCCESS) {
		(void)ut_unmap_unit_to_name(unit->unit2, UT_ASCII);
	    }
	}				/* unit mapped to name */
	if (status != UT_SUCCESS) {
	    (void)ut_unmap_name_to_unit(unitSystem, name, UT_ASCII);
	}
    }					/* singular name mapped to unit */
    return status == UT_SUCCESS
	? 0
	: status == UT_EXISTS
	    ? UT_DUP
	    : UT_EALLOC;
}

/*
 * Return the unit corresponding to a unit-specification.
 *
 */
int
utFind(
    char	*spec,
    utUnit	*up)
{
    int		status;
    ut_unit*	unit = ut_parse(unitSystem, spec, UT_ASCII);

    if (unit == NULL) {
	status = ut_get_status();

	if (status == UT_BAD_ARG) {
	    status = UT_EINVALID;
	}
	else if (status == UT_SYNTAX) {
	    status = UT_ESYNTAX;
	}
	else if (status == UT_UNKNOWN) {
	    status = UT_EUNKNOWN;
	}
	else if (status == UT_OS) {
	    status = UT_EALLOC;
	}
    }
    else {
	status = setUnit(up, unit);
    }
    return status;
}

/*
 *	Terminate use of this package.
 */
void
utTerm()
{
    ut_free(second);
    second = NULL;
    ut_free(encodedTimeUnit);
    encodedTimeUnit = NULL;
    ut_free_system(unitSystem);
    unitSystem = NULL;
}
