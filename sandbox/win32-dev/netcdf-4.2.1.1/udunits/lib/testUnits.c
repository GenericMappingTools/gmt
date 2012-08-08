/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif


#include <float.h>
#include <glob.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "udunits2.h"


static char		xmlPath[80];
static ut_system*	unitSystem;
static ut_unit*		kilogram;
static ut_unit*		meter;
static ut_unit*		radian;
static ut_unit*		kelvin;
static ut_unit*		second;
static ut_unit*		minute;
static ut_unit*		kilometer;
static ut_unit*		micron;
static ut_unit*		rankine;
static ut_unit*		celsius;
static ut_unit*		fahrenheit;
static ut_unit*		meterPerSecondSquared;
static ut_unit*		meterSquaredPerSecondSquared;
static ut_unit*		joulePerKilogram;
static ut_unit*		watt;
static ut_unit*		wattSquared;
static ut_unit*		cubicMeter;
static ut_unit*		cubicMicron;
static ut_unit*		BZ;
static ut_unit*		dBZ;
static ut_unit*		secondsSinceTheEpoch;
static ut_unit*		minutesSinceTheMillenium;
static ut_unit*		hertz;
static ut_unit*		megahertz;

static unsigned		asciiName = UT_ASCII | UT_NAMES;
static unsigned		asciiNameDef = UT_ASCII | UT_NAMES | UT_DEFINITION;
static unsigned		asciiSymbolDef = UT_ASCII | UT_DEFINITION;
static unsigned		latin1SymbolDef = UT_LATIN1 | UT_DEFINITION;
static unsigned		utf8SymbolDef = UT_UTF8 | UT_DEFINITION;


static int
setup(
    void)
{
    int		status = -1;	/* failure */
    const char*	path = getenv("UDUNITS2_XML_PATH");

    if (path != NULL) {
	(void)strcpy(xmlPath, path);
	unitSystem = ut_new_system();
	if (unitSystem != NULL)
	    status = 0;
    }

    return status;
}


static int
teardown(
    void)
{
    ut_free_system(unitSystem);

    return 0;
}


static void
test_unitSystem(void)
{
    ut_system*	system = ut_new_system();
    ut_unit*	unit;
    char	buf[80];

    CU_ASSERT_PTR_NOT_NULL(system);
    ut_set_status(UT_SUCCESS);
    unit = ut_new_base_unit(system);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_map_unit_to_name(unit, "name", UT_ASCII), UT_SUCCESS);
    ut_free(unit);
    unit = ut_get_dimensionless_unit_one(system);
    CU_ASSERT_PTR_NOT_NULL_FATAL(unit);
    CU_ASSERT_EQUAL(ut_format(unit, buf, sizeof(buf)-1, asciiSymbolDef), 1);
    CU_ASSERT_STRING_EQUAL(buf, "1");
    ut_free(unit);
    ut_free_system(system);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
}


static void
test_utNewBaseUnit(void)
{
    kilogram = ut_new_base_unit(unitSystem);
    CU_ASSERT_PTR_NOT_NULL(kilogram);
    CU_ASSERT_EQUAL(ut_map_unit_to_name(kilogram, "kilogram", UT_ASCII),
	UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_unit_to_symbol(kilogram, "kg", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("kg", UT_ASCII, kilogram), UT_SUCCESS);

    meter = ut_new_base_unit(unitSystem);
    CU_ASSERT_PTR_NOT_NULL(meter);
    CU_ASSERT_EQUAL(ut_map_name_to_unit("meter", UT_ASCII, meter), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_unit_to_name(meter, "meter", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_unit_to_symbol(meter, "m", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("m", UT_ASCII, meter), UT_SUCCESS);

    kelvin = ut_new_base_unit(unitSystem);
    CU_ASSERT_PTR_NOT_NULL(kelvin);
    CU_ASSERT_EQUAL(ut_map_unit_to_name(kelvin, "kelvin", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_name_to_unit("kelvin", UT_ASCII, kelvin), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_unit_to_symbol(kelvin, "K", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("K", UT_ASCII, kelvin), UT_SUCCESS);

    CU_ASSERT_EQUAL(ut_map_unit_to_name(kilogram, "dummy", UT_ASCII), UT_EXISTS);

    second = ut_new_base_unit(unitSystem);
    CU_ASSERT_PTR_NOT_NULL(second);

    CU_ASSERT_PTR_NULL(ut_offset_by_time(second,
        ut_encode_time(1970, 1, 1, 0, 0, 0)));
    CU_ASSERT_EQUAL(ut_get_status(), UT_NO_SECOND);

    CU_ASSERT_EQUAL(ut_set_second(second), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_set_second(NULL), UT_BAD_ARG);
    CU_ASSERT_EQUAL(ut_map_unit_to_name(second, "second", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_name_to_unit("second", UT_ASCII, second), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_unit_to_symbol(second, "s", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("s", UT_ASCII, second), UT_SUCCESS);

    CU_ASSERT_PTR_NULL(ut_new_base_unit(NULL));
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);

    CU_ASSERT_EQUAL(ut_map_unit_to_name(kilogram, "Ångström", UT_UTF8), UT_BAD_ARG);

    CU_ASSERT_EQUAL(ut_set_second(second), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_set_second(meter), UT_EXISTS);
    CU_ASSERT_EQUAL(ut_set_second(NULL), UT_BAD_ARG);
}


static void
test_utNewDimensionlessUnit(void)
{
    radian = ut_new_dimensionless_unit(unitSystem);
    CU_ASSERT_PTR_NOT_NULL(radian);
    CU_ASSERT_EQUAL(ut_map_unit_to_name(radian, "radian", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_name_to_unit("radian", UT_ASCII, radian), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_unit_to_symbol(radian, "rad", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("rad", UT_ASCII, radian), UT_SUCCESS);

    CU_ASSERT_EQUAL(ut_map_unit_to_name(radian, "dummy", UT_ASCII), UT_EXISTS);

    CU_ASSERT_EQUAL(ut_map_unit_to_symbol(radian, "f", UT_ASCII), UT_EXISTS);
    CU_ASSERT_EQUAL(ut_map_unit_to_symbol(NULL, "f", UT_ASCII), UT_BAD_ARG);

    CU_ASSERT_EQUAL(ut_map_unit_to_name(radian, "Ångström", UT_UTF8), UT_BAD_ARG);
}


static void
test_utGetUnitByName(void)
{
    ut_unit*	altMeter = ut_get_unit_by_name(unitSystem, "meter");

    CU_ASSERT_PTR_NOT_NULL(altMeter);
    CU_ASSERT_EQUAL(ut_compare(altMeter, meter), 0);
    ut_free(altMeter);

    CU_ASSERT_PTR_NULL(ut_get_unit_by_name(unitSystem, NULL));
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);

    CU_ASSERT_PTR_NULL(ut_get_unit_by_name(unitSystem, "foo"));
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
}


static void
test_utGetUnitBySymbol(void)
{
    ut_unit*	altMeter = ut_get_unit_by_symbol(unitSystem, "m");

    CU_ASSERT_PTR_NOT_NULL(altMeter);
    CU_ASSERT_EQUAL(ut_compare(altMeter, meter), 0);
    ut_free(altMeter);

    CU_ASSERT_PTR_NULL(ut_get_unit_by_symbol(unitSystem, NULL));
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);

    CU_ASSERT_PTR_NULL(ut_get_unit_by_symbol(unitSystem, "M"));
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
}


static void
test_utAddNamePrefix(void)
{
    CU_ASSERT_EQUAL(ut_add_name_prefix(unitSystem, "mega", 1e6), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_add_name_prefix(unitSystem, "mega", 1e6), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_add_name_prefix(unitSystem, "MEGA", 1e6), UT_SUCCESS);

    CU_ASSERT_EQUAL(ut_add_name_prefix(unitSystem, "mega", 1e5), UT_EXISTS);
    CU_ASSERT_EQUAL(ut_add_name_prefix(unitSystem, "MEGA", 1e5), UT_EXISTS);
    CU_ASSERT_EQUAL(ut_add_name_prefix(NULL, "foo", 1), UT_BAD_ARG);
    CU_ASSERT_EQUAL(ut_add_name_prefix(unitSystem, "", 2), UT_BAD_ARG);
    CU_ASSERT_EQUAL(ut_add_name_prefix(unitSystem, NULL, 3), UT_BAD_ARG);
    CU_ASSERT_EQUAL(ut_add_name_prefix(unitSystem, "foo", 0), UT_BAD_ARG);
}


static void
test_utAddSymbolPrefix(void)
{
    CU_ASSERT_EQUAL(ut_add_symbol_prefix(unitSystem, "M", 1e6), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_add_symbol_prefix(unitSystem, "M", 1e6), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_add_symbol_prefix(unitSystem, "u", 1e-6), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_add_symbol_prefix(unitSystem, "µ", 1e-6), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_add_symbol_prefix(unitSystem, "\xc2\xb5", 1e-6),
        UT_SUCCESS);    /* "\xc2\xb5" is "mu" character in UTF-8 */
    CU_ASSERT_EQUAL(ut_add_symbol_prefix(unitSystem, "k", 1e3), UT_SUCCESS);

    CU_ASSERT_EQUAL(ut_add_symbol_prefix(unitSystem, "M", 1e5), UT_EXISTS);
    CU_ASSERT_EQUAL(ut_add_symbol_prefix(NULL, "foo", 1), UT_BAD_ARG);
    CU_ASSERT_EQUAL(ut_add_symbol_prefix(unitSystem, "", 2), UT_BAD_ARG);
    CU_ASSERT_EQUAL(ut_add_symbol_prefix(unitSystem, NULL, 3), UT_BAD_ARG);
    CU_ASSERT_EQUAL(ut_add_symbol_prefix(unitSystem, "f", 0), UT_BAD_ARG);
}


static void
test_utMapNameToUnit(void)
{
    ut_unit*	metre;

    CU_ASSERT_PTR_NULL(ut_get_unit_by_name(unitSystem, "metre"));

    CU_ASSERT_EQUAL(ut_map_name_to_unit("metre", UT_ASCII, meter), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_name_to_unit("metre", UT_ASCII, meter), UT_SUCCESS);

    CU_ASSERT_EQUAL(ut_map_name_to_unit("metre", UT_ASCII, second), UT_EXISTS);
    CU_ASSERT_EQUAL(ut_map_name_to_unit("metre", UT_ASCII, NULL), UT_BAD_ARG);

    metre = ut_get_unit_by_name(unitSystem, "metre");
    CU_ASSERT_PTR_NOT_NULL(metre);
    CU_ASSERT_EQUAL(ut_compare(metre, meter), 0);
    ut_free(metre);
}


static void
test_utMapSymbolToUnit(void)
{
    ut_unit*    degK;

    /* "\xb0" is the degree symbol in Latin-1 */
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("\xb0K", UT_LATIN1, kelvin), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("\xb0K", UT_LATIN1, kelvin), UT_SUCCESS);

    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("\xb0K", UT_LATIN1, second), UT_EXISTS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("\xb0K", UT_LATIN1, NULL), UT_BAD_ARG);

    degK = ut_get_unit_by_symbol(unitSystem, "\xb0K");
    CU_ASSERT_PTR_NOT_NULL(degK);
    CU_ASSERT_EQUAL(ut_compare(degK, kelvin), 0);
    ut_free(degK);

    /* "\xc2\xb0" is the degree symbol in UTF-8 */
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("\xc2\xb0K", UT_UTF8, kelvin), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("\xc2\xb0K", UT_UTF8, kelvin), UT_SUCCESS);

    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("\xc2\xb0K", UT_UTF8, second), UT_EXISTS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("\xc2\xb0K", UT_UTF8, NULL), UT_BAD_ARG);

    degK = ut_get_unit_by_symbol(unitSystem, "\xc2\xb0K");
    CU_ASSERT_PTR_NOT_NULL(degK);
    CU_ASSERT_EQUAL(ut_compare(degK, kelvin), 0);
    ut_free(degK);
}


static void
test_utToString(void)
{
    char	buf[80];
    int		nchar = ut_format(meter, buf, sizeof(buf)-1, asciiSymbolDef);
    int		n;
    ut_unit*	unit;

    CU_ASSERT_EQUAL(nchar, 1);
    CU_ASSERT_STRING_EQUAL(buf, "m");

    nchar = ut_format(meter, buf, sizeof(buf)-1, asciiName);
    CU_ASSERT_STRING_EQUAL(buf, "meter");

    n = ut_format(celsius, buf, sizeof(buf)-1, asciiName);
    CU_ASSERT_TRUE(n > 0);
    CU_ASSERT_STRING_EQUAL(buf, "degrees_celsius");

    unit = ut_parse(unitSystem, "second since 1-01-01T00:00:00Z", UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT(ut_format(unit, buf, sizeof(buf), asciiSymbolDef) != -1);
    CU_ASSERT_STRING_EQUAL(buf, "s @ 1-01-01 00:00:00.000000 UTC");
    ut_free(unit);
}


static void
test_utScale(void)
{
    ut_unit*	metre;
    char	buf[80];
    int		nchar;

    kilometer = ut_scale(1000, meter);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(kilometer);
    CU_ASSERT_EQUAL(ut_get_system(meter), ut_get_system(kilometer));
    CU_ASSERT_NOT_EQUAL(ut_compare(meter, kilometer), 0);

    nchar = ut_format(kilometer, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "1000 m");

    nchar = ut_format(kilometer, buf, sizeof(buf)-1, asciiNameDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "1000 meter");

    micron = ut_scale(1e-6, meter);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(micron);
    CU_ASSERT_EQUAL(ut_get_system(meter), ut_get_system(micron));
    CU_ASSERT_NOT_EQUAL(ut_compare(meter, micron), 0);

    metre = ut_scale(1, meter);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(metre);
    CU_ASSERT_EQUAL(ut_get_system(meter), ut_get_system(metre));
    CU_ASSERT_EQUAL(ut_compare(meter, metre), 0);
    ut_free(metre);

    minute = ut_scale(60, second);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(minute);
    nchar = ut_format(minute, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "60 s");

    nchar = ut_format(minute, buf, sizeof(buf)-1, asciiNameDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "60 second");

    metre = ut_scale(1/1000., kilometer);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(metre);
    CU_ASSERT_EQUAL(ut_get_system(meter), ut_get_system(metre));
    ut_free(metre);

    CU_ASSERT_PTR_NULL(ut_scale(0, meter));
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);

    CU_ASSERT_PTR_NULL(ut_scale(0, NULL));
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);

    rankine = ut_scale(1/1.8, kelvin);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(rankine);
}


static void
test_utOffset(void)
{
    ut_unit*	dupKelvin;
    char	buf[80];
    int		nchar;

    celsius = ut_offset(kelvin, 273.15);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(celsius);
    CU_ASSERT_EQUAL(ut_get_system(kelvin), ut_get_system(celsius));
    CU_ASSERT_NOT_EQUAL(ut_compare(kelvin, celsius), 0);

    fahrenheit = ut_offset(rankine, 459.67);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(fahrenheit);
    CU_ASSERT_EQUAL(ut_get_system(rankine), ut_get_system(fahrenheit));
    CU_ASSERT_NOT_EQUAL(ut_compare(rankine, fahrenheit), 0);
    CU_ASSERT_EQUAL(ut_map_name_to_unit("degrees_fahrenheit", UT_ASCII, fahrenheit),
	UT_SUCCESS);

    nchar = ut_format(celsius, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "K @ 273.15");

    nchar = ut_format(celsius, buf, sizeof(buf)-1, asciiNameDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "kelvin from 273.15");

    dupKelvin = ut_offset(kelvin, 0);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(dupKelvin);
    CU_ASSERT_EQUAL(ut_get_system(kelvin), ut_get_system(dupKelvin));
    CU_ASSERT_EQUAL(ut_compare(kelvin, dupKelvin), 0);
    ut_free(dupKelvin);

    dupKelvin = ut_offset(celsius, -273.15);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    CU_ASSERT_PTR_NOT_NULL(dupKelvin);
    CU_ASSERT_EQUAL(ut_get_system(kelvin), ut_get_system(dupKelvin));
    ut_free(dupKelvin);

    (void)ut_offset(NULL, 5);
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);
}


static void
test_utMapUnitToName(void)
{
    ut_unit*	metre;

    CU_ASSERT_EQUAL(ut_map_unit_to_name(meter, "metre", UT_ASCII), UT_EXISTS);

    CU_ASSERT_EQUAL(ut_map_name_to_unit("metre", UT_ASCII, second), UT_EXISTS);
    CU_ASSERT_EQUAL(ut_map_name_to_unit("metre", UT_ASCII, NULL), UT_BAD_ARG);

    metre = ut_get_unit_by_name(unitSystem, "metre");
    CU_ASSERT_PTR_NOT_NULL(metre);
    CU_ASSERT_EQUAL(ut_compare(metre, meter), 0);

    CU_ASSERT_EQUAL(ut_map_unit_to_name(celsius, "degrees_celsius", UT_ASCII),
	UT_SUCCESS);

    ut_free(metre);
}


static void
test_utGetName(void)
{
    CU_ASSERT_STRING_EQUAL(ut_get_name(meter, UT_ASCII), "meter");
    CU_ASSERT_STRING_EQUAL(ut_get_name(celsius, UT_ASCII), "degrees_celsius");
    CU_ASSERT_STRING_EQUAL(ut_get_name(kilogram, UT_ASCII), "kilogram");
    CU_ASSERT_STRING_EQUAL(ut_get_name(kelvin, UT_ASCII), "kelvin");
    CU_ASSERT_STRING_EQUAL(ut_get_name(second, UT_ASCII), "second");
    CU_ASSERT_STRING_EQUAL(ut_get_name(radian, UT_ASCII), "radian");
}


static void
test_utGetSymbol(void)
{
    CU_ASSERT_STRING_EQUAL(ut_get_symbol(kilogram, UT_ASCII), "kg");
    CU_ASSERT_STRING_EQUAL(ut_get_symbol(meter, UT_ASCII), "m");
    CU_ASSERT_STRING_EQUAL(ut_get_symbol(kelvin, UT_ASCII), "K");
    CU_ASSERT_STRING_EQUAL(ut_get_symbol(second, UT_ASCII), "s");
    CU_ASSERT_STRING_EQUAL(ut_get_symbol(radian, UT_ASCII), "rad");
    CU_ASSERT_STRING_EQUAL(ut_get_symbol(hertz, UT_ASCII), "Hz");
}


static void
test_utMultiply(void)
{
    ut_unit*	squareMeter;
    ut_unit*	meterSecond;
    ut_unit*	meterCelsius;
    ut_unit*	meterRadian;
    ut_unit*	kilometerMinute;
    ut_unit*	unit;
    char	buf[80];
    int		nchar;

    squareMeter = ut_multiply(meter, meter);
    CU_ASSERT_PTR_NOT_NULL(squareMeter);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(squareMeter, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m2");

    nchar = ut_format(squareMeter, buf, sizeof(buf)-1, asciiNameDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "meter^2");

    meterSecond = ut_multiply(meter, second);
    CU_ASSERT_PTR_NOT_NULL(meterSecond);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(meterSecond, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m.s");

    nchar = ut_format(meterSecond, buf, sizeof(buf)-1, asciiNameDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "meter-second");

    meterCelsius = ut_multiply(meter, celsius);
    CU_ASSERT_PTR_NOT_NULL(meterCelsius);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(meterCelsius, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m.K");

    meterRadian = ut_multiply(meter, radian);
    CU_ASSERT_PTR_NOT_NULL(meterRadian);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(meterRadian, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m.rad");

    kilometerMinute = ut_multiply(kilometer, minute);
    CU_ASSERT_PTR_NOT_NULL(kilometerMinute);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(kilometerMinute, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "60000 m.s");

    nchar = ut_format(kilometerMinute, buf, sizeof(buf)-1, asciiNameDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "60000 meter-second");

    unit = ut_multiply(secondsSinceTheEpoch, meter);
    CU_ASSERT_PTR_NOT_NULL(unit);
    ut_free(unit);

    ut_free(squareMeter);
    ut_free(meterSecond);
    ut_free(meterCelsius);
    ut_free(meterRadian);
    ut_free(kilometerMinute);

    CU_ASSERT_PTR_NULL(ut_multiply(NULL, meter));
    CU_ASSERT_PTR_NULL(ut_multiply(meter, NULL));
}


static void
test_utInvert(void)
{
    ut_unit*	inverseMeter;
    ut_unit*	inverseMinute;
    ut_unit*	inverseCelsius;
    ut_unit*	inverseRadian;
    ut_unit*	inverseMeterSecond;
    ut_unit*	unit;
    char	buf[80];
    int		nchar;

    inverseMeter = ut_invert(meter);
    CU_ASSERT_PTR_NOT_NULL(inverseMeter);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(inverseMeter, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m-1");

    inverseMinute = ut_invert(minute);
    CU_ASSERT_PTR_NOT_NULL(inverseMinute);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(inverseMinute, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "0.0166666666666667 s-1");

    inverseRadian = ut_invert(radian);
    CU_ASSERT_PTR_NOT_NULL(inverseRadian);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(inverseRadian, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "rad-1");

    inverseCelsius = ut_invert(celsius);
    CU_ASSERT_PTR_NOT_NULL(inverseCelsius);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(inverseCelsius, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "K-1");

    unit = ut_multiply(meter, second);
    inverseMeterSecond = ut_invert(unit);
    ut_free(unit);
    CU_ASSERT_PTR_NOT_NULL(inverseMeterSecond);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(inverseMeterSecond, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m-1.s-1");

    ut_free(inverseMeter);
    ut_free(inverseMinute);
    ut_free(inverseCelsius);
    ut_free(inverseRadian);
    ut_free(inverseMeterSecond);

    hertz = ut_invert(second);
    CU_ASSERT_PTR_NOT_NULL(hertz);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);

    CU_ASSERT_EQUAL(ut_map_unit_to_name(hertz, "hertz", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_name_to_unit("hertz", UT_ASCII, hertz), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_unit_to_symbol(hertz, "Hz", UT_ASCII), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("Hz", UT_ASCII, hertz), UT_SUCCESS);

    unit = ut_invert(second);
    megahertz = ut_scale(1e6, unit);
    ut_free(unit);
    CU_ASSERT_PTR_NOT_NULL(megahertz);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
}


static void
test_utDivide(void)
{
    ut_unit*	meterPerSecond;
    ut_unit*	kilometerPerMinute;
    ut_unit*	celsiusPerMeter;
    ut_unit*	meterPerCelsius;
    ut_unit*	unit;
    ut_unit*	unit2;
    ut_unit*	unit3;
    char	buf[80];
    int		nchar;

    meterPerSecond = ut_divide(meter, second);
    CU_ASSERT_PTR_NOT_NULL(meterPerSecond);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(meterPerSecond, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m.s-1");

    kilometerPerMinute = ut_divide(kilometer, minute);
    CU_ASSERT_PTR_NOT_NULL(kilometerPerMinute);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(kilometerPerMinute, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "16.6666666666667 m.s-1");

    celsiusPerMeter = ut_divide(celsius, meter);
    CU_ASSERT_PTR_NOT_NULL(celsiusPerMeter);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(celsiusPerMeter, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m-1.K");

    meterPerCelsius = ut_divide(meter, celsius);
    CU_ASSERT_PTR_NOT_NULL(meterPerCelsius);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(meterPerCelsius, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m.K-1");

    unit = ut_raise(second, 2);
    meterPerSecondSquared = ut_divide(meter, unit);
    ut_free(unit);
    CU_ASSERT_PTR_NOT_NULL(meterPerSecondSquared);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(meterPerSecondSquared, buf, sizeof(buf)-1,
	asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m.s-2");

    unit = ut_divide(meter, second);
    meterSquaredPerSecondSquared = ut_raise(unit, 2);
    ut_free(unit);
    CU_ASSERT_PTR_NOT_NULL(meterSquaredPerSecondSquared);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(meterSquaredPerSecondSquared, buf, sizeof(buf)-1,
	asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m2.s-2");

    unit = ut_divide(meter, second);
    unit2 = ut_raise(unit, 2);
    ut_free(unit);
    unit3 = ut_multiply(kilogram, unit2);
    ut_free(unit2);
    joulePerKilogram = ut_divide(unit3, kilogram);
    ut_free(unit3);
    CU_ASSERT_PTR_NOT_NULL(joulePerKilogram);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(joulePerKilogram, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m2.s-2");

    unit = ut_divide(meter, second);
    unit2 = ut_raise(unit, 2);
    ut_free(unit);
    unit3 = ut_multiply(kilogram, unit2);
    ut_free(unit2);
    watt = ut_divide(unit3, second);
    ut_free(unit3);
    CU_ASSERT_PTR_NOT_NULL(watt);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(watt, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "kg.m2.s-3");

    ut_free(meterPerSecond);
    ut_free(kilometerPerMinute);
    ut_free(celsiusPerMeter);
    ut_free(meterPerCelsius);

    CU_ASSERT_PTR_NULL(ut_divide(NULL, meter));
    CU_ASSERT_PTR_NULL(ut_divide(meter, NULL));
}


static void
test_utRaise(void)
{
    ut_unit*	perCubicMeter;
    ut_unit*	celsiusCubed;
    ut_unit*	minutesPerKilometer;
    ut_unit*	kilometersSquaredPerMinuteSquared;
    ut_unit*	unit;
    char	buf[80];
    int		nchar;

    wattSquared = ut_raise(watt, 2);
    CU_ASSERT_PTR_NOT_NULL(wattSquared);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(wattSquared, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "kg2.m4.s-6");

    perCubicMeter = ut_raise(meter, -3);
    CU_ASSERT_PTR_NOT_NULL(perCubicMeter);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(perCubicMeter, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m-3");

    celsiusCubed = ut_raise(celsius, 3);
    CU_ASSERT_PTR_NOT_NULL(celsiusCubed);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(celsiusCubed, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "K3");

    minutesPerKilometer = ut_divide(minute, kilometer);
    kilometersSquaredPerMinuteSquared = ut_raise(minutesPerKilometer, -2);
    ut_free(minutesPerKilometer);
    CU_ASSERT_PTR_NOT_NULL(kilometersSquaredPerMinuteSquared);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(kilometersSquaredPerMinuteSquared, buf,
	sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "277.777777777778 m2.s-2");

    ut_free(perCubicMeter);
    ut_free(celsiusCubed);
    ut_free(kilometersSquaredPerMinuteSquared);

    unit = ut_raise(meter, 0);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, ut_get_dimensionless_unit_one(unitSystem)), 0);
    ut_free(unit);

    unit = ut_raise(meter, 1);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, meter), 0);
    ut_free(unit);

    unit = ut_raise(secondsSinceTheEpoch, 2);
    CU_ASSERT_PTR_NOT_NULL(unit);
    ut_free(unit);

    CU_ASSERT_PTR_NULL(ut_raise(dBZ, 2));
    CU_ASSERT_PTR_NULL(ut_raise(NULL, 2));
}


static void
test_utRoot(void)
{
    ut_unit*	unit;
    ut_unit*	unit2;

    CU_ASSERT_PTR_NULL(ut_root(watt, -1));
    CU_ASSERT_PTR_NULL(ut_root(watt, 0));
    unit = ut_root(watt, 1);
    CU_ASSERT_EQUAL(ut_compare(watt, unit), 0);
    ut_free(unit);

    unit = ut_get_dimensionless_unit_one(unitSystem);
    CU_ASSERT_EQUAL(ut_compare(unit, ut_root(unit, 1)), 0);
    ut_free(unit);
    unit = ut_root(watt, 1);
    CU_ASSERT_EQUAL(ut_compare(watt, unit), 0);
    ut_free(unit);

    unit = ut_raise(watt, 2);
    unit2 = ut_root(unit, 2);
    ut_free(unit);
    CU_ASSERT_EQUAL(ut_compare(watt, unit2), 0);
    ut_free(unit2);

    unit = ut_raise(celsius, 2);
    unit2 = ut_root(unit, 2);
    ut_free(unit);
    CU_ASSERT_EQUAL(ut_compare(kelvin, unit2), 0);
    ut_free(unit2);

    unit = ut_raise(secondsSinceTheEpoch, 2);
    unit2 = ut_root(unit, 2);
    ut_free(unit);
    CU_ASSERT_EQUAL(ut_compare(second, unit2), 0);
    ut_free(unit2);

    CU_ASSERT_PTR_NULL(ut_root(BZ, 2));
}


static void
test_utLog(void)
{
    ut_unit*	milliwatt = ut_scale(0.001, watt);
    ut_unit*	bel_1_mW = ut_log(10, milliwatt);
    ut_unit*	decibel_1_mW;
    ut_unit*	unit;
    char	buf[80];
    int		nchar;

    CU_ASSERT_PTR_NOT_NULL(bel_1_mW);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(bel_1_mW, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "lg(re 0.001 kg.m2.s-3)");

    decibel_1_mW = ut_scale(0.1, bel_1_mW);
    CU_ASSERT_PTR_NOT_NULL(decibel_1_mW);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(decibel_1_mW, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "0.1 lg(re 0.001 kg.m2.s-3)");

    unit = ut_log(-10, milliwatt);
    CU_ASSERT_PTR_NULL(unit);
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);

    cubicMeter = ut_raise(meter, 3);
    unit = ut_scale(1e-6, meter);
    cubicMicron = ut_raise(unit, 3);
    ut_free(unit);
    CU_ASSERT_PTR_NOT_NULL(cubicMicron);

    BZ = ut_log(10, cubicMicron);
    CU_ASSERT_PTR_NOT_NULL(BZ);
    CU_ASSERT_EQUAL(ut_is_dimensionless(BZ), 1);

    CU_ASSERT_PTR_NULL(ut_raise(BZ, 2));
    CU_ASSERT_EQUAL(ut_get_status(), UT_MEANINGLESS);

    dBZ = ut_scale(0.1, BZ);
    CU_ASSERT_PTR_NOT_NULL(dBZ);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SUCCESS);
    nchar = ut_format(dBZ, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "0.1 lg(re 1e-18 m3)");
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("dBZ", UT_ASCII, dBZ), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("dBZ", UT_ASCII, dBZ), UT_SUCCESS);
    CU_ASSERT_EQUAL(ut_map_symbol_to_unit("dBZ", UT_ASCII, meter), UT_EXISTS);
    {
	ut_unit*	unit = ut_get_unit_by_symbol(unitSystem, "dBZ");

	CU_ASSERT_PTR_NOT_NULL(unit);
	CU_ASSERT_EQUAL(ut_compare(unit, dBZ), 0);

	CU_ASSERT_PTR_NULL(ut_get_unit_by_symbol(unitSystem, "DBZ"));
	ut_free(unit);
    }

    ut_free(milliwatt);
    ut_free(bel_1_mW);
    ut_free(decibel_1_mW);

    CU_ASSERT_PTR_NULL(ut_log(2, NULL));
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);
    CU_ASSERT_PTR_NULL(ut_log(1, meter));
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);

    CU_ASSERT_PTR_NULL(ut_multiply(dBZ, meter));
    unit = ut_multiply(dBZ, radian);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, dBZ), 0);
    ut_free(unit);
    CU_ASSERT_PTR_NULL(ut_multiply(dBZ, dBZ));
}


static int
areCloseFloats(
    float	x,
    float	y)
{
    int		areClose;

    if (x == 0 || y == 0) {
	areClose = fabs(x - y) < 1000*FLT_EPSILON;
    }
    else {
	areClose = fabs(1.0 - x / y) < 1000*FLT_EPSILON;
    }

    return areClose;
}


static int
areCloseDoubles(
    double	x,
    double	y)
{
    int		areClose;

    if (x == 0 || y == 0) {
	areClose = fabs(x - y) < 1000*DBL_EPSILON;
    }
    else {
	areClose = fabs(1.0 - x / y) < 1000*DBL_EPSILON;
    }

    return areClose;
}


static void
test_utGetDimensionlessUnitOne(void)
{
    CU_ASSERT_PTR_NOT_NULL(ut_get_dimensionless_unit_one(unitSystem));
    CU_ASSERT_PTR_NULL(ut_get_dimensionless_unit_one(NULL));
}


static void
test_utGetSystem(void)
{
    CU_ASSERT_PTR_NOT_NULL(ut_get_system(meter));
    CU_ASSERT_PTR_NULL(ut_get_system(NULL));
}


static void
test_utSameSystem(void)
{
    ut_system*	system;

    CU_ASSERT_EQUAL(ut_same_system(meter, kilogram), 1);

    CU_ASSERT_EQUAL(ut_same_system(NULL, kilogram), 0);
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);
    CU_ASSERT_EQUAL(ut_same_system(kilogram, NULL), 0);
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);

    system = ut_new_system();
    CU_ASSERT_PTR_NOT_NULL(system);

    CU_ASSERT_EQUAL(ut_same_system(meter, ut_get_dimensionless_unit_one(system)), 0);

    ut_free_system(system);
}


static void
test_utIsDimensionless(void)
{
    ut_unit*	unit;

    CU_ASSERT_EQUAL(ut_is_dimensionless(meter), 0);
    CU_ASSERT_EQUAL(ut_is_dimensionless(radian), 1);
    CU_ASSERT_EQUAL(ut_is_dimensionless(secondsSinceTheEpoch), 0);
    CU_ASSERT_EQUAL(ut_is_dimensionless(NULL), 0);
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);

    unit = ut_raise(radian, 2);
    CU_ASSERT_EQUAL(ut_is_dimensionless(unit), 1);
    ut_free(unit);
}


static void
test_utClone(void)
{
    ut_unit*	unit;

    unit = ut_clone(secondsSinceTheEpoch);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_are_convertible(unit, secondsSinceTheEpoch), 1);
    ut_free(unit);

    CU_ASSERT_PTR_NULL(ut_clone(NULL));
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);
}


static void
test_utAreConvertible(void)
{
    ut_unit*	unit;

    CU_ASSERT_EQUAL(ut_are_convertible(meter, meter), 1);
    CU_ASSERT_EQUAL(ut_are_convertible(radian, radian), 1);
    CU_ASSERT_EQUAL(ut_are_convertible(radian, meter), 0);
    CU_ASSERT_EQUAL(ut_are_convertible(cubicMicron, cubicMicron), 1);
    CU_ASSERT_EQUAL(ut_are_convertible(cubicMicron, cubicMeter), 1);
    CU_ASSERT_EQUAL(ut_are_convertible(watt, watt), 1);
    CU_ASSERT_EQUAL(ut_are_convertible(watt, cubicMicron), 0);
    CU_ASSERT_EQUAL(ut_are_convertible(cubicMicron, watt), 0);
    CU_ASSERT_EQUAL(ut_are_convertible(secondsSinceTheEpoch,
	secondsSinceTheEpoch), 1);
    CU_ASSERT_EQUAL(ut_are_convertible(secondsSinceTheEpoch, second), 0);
    CU_ASSERT_EQUAL(ut_are_convertible(joulePerKilogram, 
	meterSquaredPerSecondSquared), 1);
    CU_ASSERT_EQUAL(ut_are_convertible(joulePerKilogram, 
	meterPerSecondSquared), 0);

    unit = ut_raise(radian, 2);
    CU_ASSERT_EQUAL(ut_are_convertible(radian, unit), 1);
    ut_free(unit);

    CU_ASSERT_EQUAL(ut_are_convertible(NULL, meter), 0);
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);
}


static void
test_utGetConverter(void)
{
    cv_converter*	converter = ut_get_converter(meter, meter);
    double		doubles[2];
    float		floats[2];

    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_EQUAL(cv_convert_double(converter, 1.0), 1.0);
    floats[0] = 1; floats[1] = 2;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_EQUAL(floats[0], 1);
    CU_ASSERT_EQUAL(floats[1], 2);
    doubles[0] = 1; doubles[1] = 2;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_EQUAL(doubles[0], 1);
    CU_ASSERT_EQUAL(doubles[1], 2);
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_EQUAL(floats[1], 1);
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1),
	doubles+1);
    CU_ASSERT_EQUAL(doubles[1], 1);
    floats[0] = 1; floats[1] = 2;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_EQUAL(floats[0], 2);
    doubles[0] = 1; doubles[1] = 2;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles),
	doubles);
    CU_ASSERT_EQUAL(doubles[0], 2);
    cv_free(converter);

    converter = ut_get_converter(radian, radian);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_EQUAL(cv_convert_double(converter, 1.0), 1.0);
    cv_free(converter);

    converter = ut_get_converter(meter, radian);
    CU_ASSERT_PTR_NULL(converter);
    converter = ut_get_converter(meter, kelvin);
    CU_ASSERT_PTR_NULL(converter);
    converter = ut_get_converter(meter, celsius);
    CU_ASSERT_PTR_NULL(converter);
    converter = ut_get_converter(meter, fahrenheit);
    CU_ASSERT_PTR_NULL(converter);
    converter = ut_get_converter(meter, dBZ);
    CU_ASSERT_PTR_NULL(converter);
    converter = ut_get_converter(dBZ, radian);
    CU_ASSERT_PTR_NULL(converter);

    converter = ut_get_converter(kilometer, kilometer);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_EQUAL(cv_convert_double(converter, 1.0), 1.0);
    cv_free(converter);

    converter = ut_get_converter(meter, kilometer);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 1000.0), 1.0));
    floats[0] = 0; floats[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_EQUAL(floats[0], 0);
    CU_ASSERT_EQUAL(floats[1], 1);
    doubles[0] = 0; doubles[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_EQUAL(doubles[0], 0);
    CU_ASSERT_EQUAL(doubles[1], 1);
    floats[0] = 0; floats[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_EQUAL(floats[1], 0);
    doubles[0] = 0; doubles[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1), 
	doubles+1);
    CU_ASSERT_EQUAL(doubles[1], 0);
    floats[0] = 0; floats[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_EQUAL(floats[0], 1);
    doubles[0] = 0; doubles[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles), 
	doubles);
    CU_ASSERT_EQUAL(doubles[0], 1);
    cv_free(converter);

    converter = ut_get_converter(kilometer, meter);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_EQUAL(cv_convert_double(converter, 1.0), 1000.0);
    floats[0] = 0; floats[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_EQUAL(floats[0], 0);
    CU_ASSERT_EQUAL(floats[1], 1000);
    doubles[0] = 0; doubles[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_EQUAL(doubles[0], 0);
    CU_ASSERT_EQUAL(doubles[1], 1000);
    floats[0] = 0; floats[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_EQUAL(floats[1], 0);
    doubles[0] = 0; doubles[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1), 
	doubles+1);
    CU_ASSERT_EQUAL(doubles[1], 0);
    floats[0] = 0; floats[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_EQUAL(floats[0], 1000);
    doubles[0] = 0; doubles[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles), 
	doubles);
    CU_ASSERT_EQUAL(doubles[0], 1000);
    cv_free(converter);

    converter = ut_get_converter(kelvin, celsius);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_EQUAL(cv_convert_double(converter, 273.15), 0.0);
    floats[0] = 0; floats[1] = 273.15;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], -273.15));
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 0));
    doubles[0] = 0; doubles[1] = 273.15;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_EQUAL(doubles[0], -273.15);
    CU_ASSERT_EQUAL(doubles[1], 0);
    floats[0] = 0; floats[1] = 273.15;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_TRUE(areCloseFloats(floats[1], -273.15));
    doubles[0] = 0; doubles[1] = 273.15;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1), 
	doubles+1);
    CU_ASSERT_EQUAL(doubles[1], -273.15);
    floats[0] = 0; floats[1] = 273.15;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 0));
    doubles[0] = 0; doubles[1] = 273.15;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles), 
	doubles);
    CU_ASSERT_EQUAL(doubles[0], 0);
    cv_free(converter);

    converter = ut_get_converter(celsius, kelvin);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_EQUAL(cv_convert_double(converter, 0.0), 273.15);
    floats[0] = 0; floats[1] = -273.15;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 273.15));
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 0));
    doubles[0] = 0; doubles[1] = -273.15;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_EQUAL(doubles[0], 273.15);
    CU_ASSERT_EQUAL(doubles[1], 0);
    floats[0] = 0; floats[1] = -273.15;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 273.15));
    doubles[0] = 0; doubles[1] = -273.15;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1), 
	doubles+1);
    CU_ASSERT_EQUAL(doubles[1], 273.15);
    floats[0] = 0; floats[1] = -273.15;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 0));
    doubles[0] = 0; doubles[1] = -273.15;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles), 
	doubles);
    CU_ASSERT_EQUAL(doubles[0], 0);
    cv_free(converter);

    converter = ut_get_converter(celsius, fahrenheit);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 0.0), 32));
    floats[0] = 0; floats[1] = 100;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 32));
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 212));
    doubles[0] = 0; doubles[1] = 100;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 32));
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 212));
    floats[0] = 0; floats[1] = 100;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 32));
    doubles[0] = 0; doubles[1] = 100;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1), 
	doubles+1);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 32));
    floats[0] = 0; floats[1] = 100;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 212));
    doubles[0] = 0; doubles[1] = 100;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles), 
	doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 212));
    cv_free(converter);

    converter = ut_get_converter(fahrenheit, celsius);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 32.0), 0.0));
    floats[0] = 32; floats[1] = 212;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 0));
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 100));
    doubles[0] = 32; doubles[1] = 212;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 0));
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 100));
    floats[0] = 32; floats[1] = 212;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 0));
    doubles[0] = 32; doubles[1] = 212;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1), 
	doubles+1);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 0));
    floats[0] = 32; floats[1] = 212;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 100));
    doubles[0] = 32; doubles[1] = 212;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles), 
	doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 100));
    cv_free(converter);

    converter = ut_get_converter(cubicMeter, dBZ);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 1e-18), 0.0));
    cv_free(converter);

    converter = ut_get_converter(cubicMicron, dBZ);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 1000), 30.0));
    floats[0] = 10; floats[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 10));
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 30));
    doubles[0] = 10; doubles[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 10));
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 30));
    floats[0] = 10; floats[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 10));
    doubles[0] = 10; doubles[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1), 
	doubles+1);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 10));
    floats[0] = 10; floats[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 30));
    doubles[0] = 10; doubles[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles), 
	doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 30));
    cv_free(converter);

    converter = ut_get_converter(dBZ, cubicMicron);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_TRUE(areCloseFloats(cv_convert_float(converter, 10), 10.0));
    CU_ASSERT_TRUE(areCloseFloats(cv_convert_float(converter, 30), 1000.0));
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 10), 10.0));
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 30), 1000.0));
    floats[0] = 10; floats[1] = 30;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 10));
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 1000));
    doubles[0] = 10; doubles[1] = 30;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 10));
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 1000));
    floats[0] = 10; floats[1] = 30;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 10));
    doubles[0] = 10; doubles[1] = 30;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1), 
	doubles+1);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 10));
    floats[0] = 10; floats[1] = 30;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 1000));
    doubles[0] = 10; doubles[1] = 30;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles), 
	doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 1000));
    cv_free(converter);

    converter = ut_get_converter(second, hertz);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_TRUE(areCloseFloats(cv_convert_float(converter, 1.0), 1.0));
    CU_ASSERT_TRUE(areCloseFloats(cv_convert_float(converter, 5.0), 1/5.0));
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 1.0), 1.0));
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 5.0), 1/5.0));
    floats[0] = 1; floats[1] = 5;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 1.0));
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 1.0/5));
    doubles[0] = 1; doubles[1] = 5;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 1));
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 1.0/5));
    cv_free(converter);

    converter = ut_get_converter(second, megahertz);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_TRUE(areCloseFloats(cv_convert_float(converter, 1), 1e-6));
    CU_ASSERT_TRUE(areCloseFloats(cv_convert_float(converter, 1e-6), 1.0));
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 1), 1e-6));
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 1e-6), 1.0));
    floats[0] = 1; floats[1] = 1e-6;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 1e-6));
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 1));
    doubles[0] = 1; doubles[1] = 1e-6;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 1e-6));
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 1));
    cv_free(converter);

    converter = ut_get_converter(secondsSinceTheEpoch, minutesSinceTheMillenium);
    CU_ASSERT_PTR_NOT_NULL(converter);
    cv_free(converter);

    CU_ASSERT_PTR_NULL(ut_get_converter(NULL, meter));
    CU_ASSERT_PTR_NULL(ut_get_converter(meter, NULL));
}


static void
test_utOffsetByTime(void)
{
    char		buf[80];
    int			nchar;
    cv_converter*	converter;
    ut_unit*		day;
    ut_unit*		daysSinceTheEpoch;
    double		doubles[2];
    float		floats[2];

    secondsSinceTheEpoch =
        ut_offset_by_time(second, ut_encode_time(1970, 1, 1, 0, 0, 0));
    CU_ASSERT_PTR_NOT_NULL(secondsSinceTheEpoch);
    nchar = ut_format(secondsSinceTheEpoch, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "s @ 19700101T000000.0000000 UTC");

    minutesSinceTheMillenium =
        ut_offset_by_time(minute, ut_encode_time(2001, 1, 1, 0, 0, 0));
    CU_ASSERT_PTR_NOT_NULL(minutesSinceTheMillenium);

    nchar = ut_format(secondsSinceTheEpoch, buf, sizeof(buf)-1, asciiNameDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "second since 1970-01-01 00:00:00.0000000 UTC");

    converter = ut_get_converter(secondsSinceTheEpoch, secondsSinceTheEpoch);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 1000), 1000));
    floats[0] = 0; floats[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 0));
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 1000));
    doubles[0] = 0; doubles[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 0));
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 1000));
    floats[0] = 0; floats[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 0));
    doubles[0] = 0; doubles[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1), 
	doubles+1);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 0));
    floats[0] = 0; floats[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 1000));
    doubles[0] = 0; doubles[1] = 1000;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles), 
	doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 1000));
    cv_free(converter);

    day = ut_scale(86400, second);
    CU_ASSERT_PTR_NOT_NULL_FATAL(day);
    daysSinceTheEpoch = ut_offset_by_time(day, ut_encode_time(1970, 1, 1, 0, 0, 0));
    ut_free(day);
    CU_ASSERT_PTR_NOT_NULL_FATAL(daysSinceTheEpoch);
    nchar = ut_format(daysSinceTheEpoch, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "(86400 s) @ 19700101T000000.0000000 UTC");

    converter = ut_get_converter(secondsSinceTheEpoch, daysSinceTheEpoch);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 86400), 1));
    floats[0] = 0; floats[1] = 86400;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 0));
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 1));
    doubles[0] = 0; doubles[1] = 86400;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 0));
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 1));
    floats[0] = 0; floats[1] = 86400;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 0));
    doubles[0] = 0; doubles[1] = 86400;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1), 
	doubles+1);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 0));
    floats[0] = 0; floats[1] = 86400;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 1));
    doubles[0] = 0; doubles[1] = 86400;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles), 
	doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 1));
    cv_free(converter);

    converter = ut_get_converter(daysSinceTheEpoch, secondsSinceTheEpoch);
    CU_ASSERT_PTR_NOT_NULL(converter);
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 1), 86400));
    floats[0] = 0; floats[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 2, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 0));
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 86400));
    doubles[0] = 0; doubles[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 2, doubles), doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 0));
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 86400));
    floats[0] = 0; floats[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats, 1, floats+1), floats+1);
    CU_ASSERT_TRUE(areCloseFloats(floats[1], 0));
    doubles[0] = 0; doubles[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles, 1, doubles+1), 
	doubles+1);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[1], 0));
    floats[0] = 0; floats[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_floats(converter, floats+1, 1, floats), floats);
    CU_ASSERT_TRUE(areCloseFloats(floats[0], 86400));
    doubles[0] = 0; doubles[1] = 1;
    CU_ASSERT_EQUAL(cv_convert_doubles(converter, doubles+1, 1, doubles), 
	doubles);
    CU_ASSERT_TRUE(areCloseDoubles(doubles[0], 86400));
    cv_free(converter);

    ut_free(daysSinceTheEpoch);

    CU_ASSERT_PTR_NULL(ut_offset_by_time(NULL, ut_encode_time(0, 0, 0, 0, 0, 0)));
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);
}


static void
test_ut_decode_time(void)
{
    double      timeval = ut_encode_time(2001, 1, 1, 0, 0, 0);
    int         year1, month1, day1, hour1, minute1;
    int         year2, month2, day2, hour2, minute2;
    double      second1, resolution1;
    double      second2, resolution2;
    ut_unit*    unit;

    ut_decode_time(timeval, &year1, &month1, &day1, &hour1, &minute1,
        &second1, &resolution1);
    unit = ut_parse(unitSystem, "second since 2010-01-11T09:08:10Z", UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    ut_free(unit);
    ut_decode_time(timeval, &year2, &month2, &day2, &hour2, &minute2,
        &second2, &resolution2);
    CU_ASSERT_EQUAL(year1, year2);
    CU_ASSERT_EQUAL(month1, month2);
    CU_ASSERT_EQUAL(day1, day2);
    CU_ASSERT_EQUAL(hour1, hour2);
    CU_ASSERT_EQUAL(minute1, minute2);
    CU_ASSERT_EQUAL(second1, second2);
    CU_ASSERT_EQUAL(resolution1, resolution2);
}



static void
test_utSetEncoding(void)
{
    char		buf[80];
    int			nchar;

    nchar = ut_format(watt, buf, sizeof(buf)-1, asciiSymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "kg.m2.s-3");

    nchar = ut_format(watt, buf, sizeof(buf)-1, latin1SymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf, "m\xb2\xb7kg/s\xb3");

    nchar = ut_format(watt, buf, sizeof(buf)-1, utf8SymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf,
	"kg\xc2\xb7m\xc2\xb2\xc2\xb7s\xe2\x81\xbb\xc2\xb3");

    nchar = ut_format(wattSquared, buf, sizeof(buf)-1, utf8SymbolDef);
    CU_ASSERT_TRUE_FATAL(nchar > 0);
    CU_ASSERT_TRUE_FATAL(nchar < sizeof(buf));
    buf[nchar] = 0;
    CU_ASSERT_STRING_EQUAL(buf,
	"kg\xc2\xb2\xc2\xb7m\xe2\x81\xb4\xc2\xb7s\xe2\x81\xbb\xe2\x81\xb6");
        /* kg2.m4.s-6 */
}


static void
test_utCompare(void)
{
    CU_ASSERT_NOT_EQUAL(ut_compare(kilogram, meter), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(meter, radian), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(radian, kelvin), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(kelvin, second), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(second, minute), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(minute, kilometer), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(kilometer, rankine), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(rankine, celsius), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(celsius, fahrenheit), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(fahrenheit, watt), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(watt, cubicMicron), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(BZ, cubicMicron), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(cubicMicron, dBZ), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(dBZ, secondsSinceTheEpoch), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(secondsSinceTheEpoch, hertz), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(hertz, megahertz), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(megahertz, kilogram), 0);
    CU_ASSERT_NOT_EQUAL(ut_compare(dBZ, meter), 0);

    CU_ASSERT_EQUAL(ut_compare(NULL, meter), -1);
    CU_ASSERT_EQUAL(ut_compare(NULL, NULL), 0);
    CU_ASSERT_EQUAL(ut_compare(meter, NULL), 1);
}


static void
test_parsing(void)
{
    ut_unit*	unit;
    char*	spec;

    spec = "34 quatloos";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NULL(unit);
    CU_ASSERT_EQUAL(ut_get_status(), UT_UNKNOWN);

    spec = "$&/^";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NULL(unit);
    CU_ASSERT_EQUAL(ut_get_status(), UT_UNKNOWN);

    unit = ut_parse(unitSystem, NULL, UT_ASCII);
    CU_ASSERT_PTR_NULL(unit);
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);

    unit = ut_parse(NULL, "kg", UT_ASCII);
    CU_ASSERT_PTR_NULL(unit);
    CU_ASSERT_EQUAL(ut_get_status(), UT_BAD_ARG);

    spec = "m";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, meter), 0);
    ut_free(unit);

    spec = "kg.m";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    ut_free(unit);

    spec = "kg m";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    ut_free(unit);

    spec = "1/s";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, hertz), 0);
    ut_free(unit);

    spec = "kg.m2.s-3";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, watt), 0);
    ut_free(unit);

    spec = "kg.m2/s3";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, watt), 0);
    ut_free(unit);

    spec = "s-3.m2.kg";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, watt), 0);
    ut_free(unit);

    spec = "(kg.m2/s3)";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, watt), 0);
    ut_free(unit);

    spec = "(kg.m2/s3)^1";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, watt), 0);
    ut_free(unit);

    spec = "kg.(m/s)^2.s-1";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, watt), 0);
    ut_free(unit);

    spec = "1000 m";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, kilometer), 0);
    ut_free(unit);

    spec = "(1000)(m)";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, kilometer), 0);
    ut_free(unit);

    spec = "(K/1.8) @ 459.67";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, fahrenheit), 0);
    ut_free(unit);

    spec = "METER";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, meter), 0);
    ut_free(unit);

    spec = "s @ 19700101T000000";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, secondsSinceTheEpoch), 0);
    ut_free(unit);

    spec = "s@19700101T000000 UTC";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, secondsSinceTheEpoch), 0);
    ut_free(unit);

    spec = "s@19700101T000000Z";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, secondsSinceTheEpoch), 0);
    ut_free(unit);

    spec = "s@19700101T000000 Z";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, secondsSinceTheEpoch), 0);
    ut_free(unit);

    spec = "s @ 1970-01-01 00:00:00";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, secondsSinceTheEpoch), 0);
    ut_free(unit);

    spec = "s @ 1970-01-01T00:00:00";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, secondsSinceTheEpoch), 0);
    ut_free(unit);

    spec = "s@1970-01-01T00:00:00Z";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, secondsSinceTheEpoch), 0);
    ut_free(unit);

    spec = "s @ 1970-01-01T00:00:00Z";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, secondsSinceTheEpoch), 0);
    ut_free(unit);

    spec = "second@1970-01-01T00:00:00Z";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, secondsSinceTheEpoch), 0);
    ut_free(unit);

    spec = "second @ 1970-01-01T00:00:00Z";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, secondsSinceTheEpoch), 0);
    ut_free(unit);

    spec = "second since 1970-01-01T00:00:00Z";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, secondsSinceTheEpoch), 0);
    ut_free(unit);

    spec = "kg·m²/s³";
    unit = ut_parse(unitSystem, spec, UT_LATIN1);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, watt), 0);
    ut_free(unit);

    spec = "(kg)(m)^2/(s)^3";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, watt), 0);
    ut_free(unit);

    spec = "kg\xc2\xb7m\xc2\xb2/s\xc2\xb3";
    unit = ut_parse(unitSystem, spec, UT_UTF8);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, watt), 0);
    ut_free(unit);

    spec = "0.1 lg(re (1e-6 m)^3)";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, dBZ), 0);
    ut_free(unit);

    {
        char    buf[] = " (K/1.8) @ 459.67 ";

        (void)ut_trim(buf, UT_ASCII);

        unit = ut_parse(unitSystem, buf, UT_ASCII);
        CU_ASSERT_PTR_NOT_NULL(unit);
        CU_ASSERT_EQUAL(ut_compare(unit, fahrenheit), 0);
        ut_free(unit);
    }

    spec = "1";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(ut_get_dimensionless_unit_one(unitSystem), unit), 0);
    ut_free(unit);

    spec = "3.141592653589793238462643383279";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    ut_free(unit);

    spec = "";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(ut_get_dimensionless_unit_one(unitSystem), unit), 0);
    ut_free(unit);

    spec = "km";
    unit = ut_parse(unitSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(kilometer, unit), 0);
    ut_free(unit);

    spec = "µm";
    unit = ut_parse(unitSystem, spec, UT_LATIN1);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, micron), 0);
    ut_free(unit);

    spec = "µmegaHz";
    unit = ut_parse(unitSystem, spec, UT_LATIN1);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, hertz), 0);
    ut_free(unit);

    spec = "MeGaµHertz";
    unit = ut_parse(unitSystem, spec, UT_LATIN1);
    CU_ASSERT_PTR_NOT_NULL(unit);
    CU_ASSERT_EQUAL(ut_compare(unit, hertz), 0);
    ut_free(unit);

    spec = "meter @ 100 @ 10";
    unit = ut_parse(unitSystem, spec, UT_LATIN1);
    CU_ASSERT_PTR_NULL(unit);
    CU_ASSERT_EQUAL(ut_get_status(), UT_SYNTAX);
}


static void
test_visitor(void)
{
    CU_ASSERT_EQUAL(ut_accept_visitor(NULL, NULL, NULL), UT_BAD_ARG);
}


static void
test_xml(void)
{
    ut_system*          xmlSystem;
    glob_t              files;
    int                 status;
    ut_unit*            unit1;
    ut_unit*            unit2;
    cv_converter*	converter;
    char*		spec;
    ut_unit*		unit;

    ut_set_error_message_handler(ut_write_to_stderr);
    xmlSystem = ut_read_xml(xmlPath);
    CU_ASSERT_PTR_NOT_NULL_FATAL(xmlSystem);

    spec = "seconds@1970-01-01T00:00:00Z";
    unit = ut_parse(xmlSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    ut_free(unit);

    spec = "seconds @ 1970-01-01T00:00:00Z";
    unit = ut_parse(xmlSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    ut_free(unit);

    spec = "seconds since 1970-01-01T00:00:00Z";
    unit = ut_parse(xmlSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    ut_free(unit);

    spec = "seconds since 1970-01-01T00:00:00Z";
    unit = ut_parse(xmlSystem, spec, UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit);
    ut_free(unit);

    unit1 = ut_get_unit_by_symbol(unitSystem, "\xb0K"); /* Latin-1 degree sym */
    CU_ASSERT_PTR_NOT_NULL(unit1);
    CU_ASSERT_EQUAL(ut_compare(unit1, kelvin), 0);
    ut_free(unit1);

    unit1 = ut_parse(xmlSystem, "\xb0K", UT_LATIN1);    /* degree sign */
    CU_ASSERT_PTR_NOT_NULL(unit1);

    unit2 = ut_parse(xmlSystem, "K", UT_LATIN1);
    CU_ASSERT_PTR_NOT_NULL(unit2);

    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);

    ut_free(unit1);

    unit1 = ut_parse(xmlSystem, "\xc2\xb5K", UT_UTF8);  /* "mu" character */
    CU_ASSERT_PTR_NOT_NULL(unit1);
    converter = ut_get_converter(unit1, unit2);
    CU_ASSERT_PTR_NOT_NULL_FATAL(converter);
    CU_ASSERT_TRUE(areCloseDoubles(cv_convert_double(converter, 1e6), 1.0));

    cv_free(converter);
    ut_free(unit1);
    ut_free(unit2);

    unit1 = ut_parse(xmlSystem, "arc_degree", UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit1);
    unit2 = ut_parse(xmlSystem, "arc""\xA0""degree", UT_LATIN1);  /* NBSP */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    unit2 = ut_parse(xmlSystem, "\xB0", UT_LATIN1);  /* degree sign */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    unit2 = ut_parse(xmlSystem, "\xC2\xB0", UT_UTF8);  /* degree sign */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    ut_free(unit1);

    unit1 = ut_parse(xmlSystem, "arc_minute", UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit1);
    unit2 = ut_parse(xmlSystem, "arc""\xA0""minute", UT_LATIN1);  /* NBSP */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    unit2 = ut_parse(xmlSystem, "'", UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    unit2 = ut_parse(xmlSystem, "\xE2\x80\xB2", UT_UTF8);  /* PRIME */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    ut_free(unit1);

    unit1 = ut_parse(xmlSystem, "arc_second", UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit1);
    unit2 = ut_parse(xmlSystem, "arc""\xA0""second", UT_LATIN1);  /* NBSP */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    unit2 = ut_parse(xmlSystem, "\"", UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    unit2 = ut_parse(xmlSystem, "\xE2\x80\xB3", UT_UTF8);  /* DOUBLE PRIME */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    ut_free(unit1);

    unit1 = ut_parse(xmlSystem, "ohm", UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit1);
    unit2 = ut_parse(xmlSystem, "\xCE\xA9", UT_UTF8);  /* UPPER CASE OMEGA */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    unit2 = ut_parse(xmlSystem, "\xE2\x84\xA6", UT_UTF8);  /* OHM SIGN */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    ut_free(unit1);

    unit1 = ut_parse(xmlSystem, "degree_celsius", UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit1);
    unit2 = ut_parse(xmlSystem, "degree""\xA0""celsius", UT_LATIN1);/* NBSP */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    unit2 = ut_parse(xmlSystem, "\xB0""C", UT_LATIN1);  /* degree sign */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    unit2 = ut_parse(xmlSystem, "\xE2\x84\x83", UT_UTF8); /* DEGREE CELSIUS */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
    ut_free(unit1);

    unit1 = ut_parse(xmlSystem, "angstrom", UT_ASCII);
    CU_ASSERT_PTR_NOT_NULL(unit1);
    unit2 = ut_parse(xmlSystem, "\xE5ngstr\xF6m", UT_LATIN1);/* small A with ring */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
#if 0
    unit2 = ut_parse(xmlSystem, "\xC5ngstr\xF6m", UT_LATIN1);/* capital A with ring */
    CU_ASSERT_PTR_NOT_NULL(unit2);
    CU_ASSERT_EQUAL(ut_compare(unit1, unit2), 0);
    ut_free(unit2);
#endif
    ut_free(unit1);

    ut_free_system(xmlSystem);

    ut_set_error_message_handler(ut_ignore);

    xmlSystem = ut_read_xml(NULL);
    if (xmlSystem == NULL) {
        CU_ASSERT_EQUAL(ut_get_status(), UT_OPEN_DEFAULT);
    }
    else {
        ut_free_system(xmlSystem);
    }

    xmlSystem = ut_read_xml("xmlFailure/noExist.xml");
    CU_ASSERT_PTR_NULL(xmlSystem);
    CU_ASSERT_EQUAL(ut_get_status(), UT_OPEN_ARG);

    status = glob("xmlFailure/*.xml", 0, NULL, &files);

    if (status != 0 && status != GLOB_NOMATCH) {
        CU_ASSERT_TRUE(0);
    }
    else {
        size_t  i;

        for (i = 0; i < files.gl_pathc; ++i) {
            xmlSystem = ut_read_xml(files.gl_pathv[i]);

            CU_ASSERT(xmlSystem == NULL);

            if (xmlSystem == NULL) {
                CU_ASSERT(ut_get_status() == UT_PARSE);

                if (ut_get_status() != UT_PARSE)
                    (void)fprintf(stderr, "File didn't fail: \"%s\"\n",
                        files.gl_pathv[i]);
            }
            else {
                (void)fprintf(stderr, "File didn't fail: \"%s\"\n",
                    files.gl_pathv[i]);
                ut_free_system(xmlSystem);
            }
        }
    }

    globfree(&files);

    ut_set_error_message_handler(ut_write_to_stderr);

    status = glob("xmlSuccess/*.xml", 0, NULL, &files);

    if (status != 0 && status != GLOB_NOMATCH) {
        CU_ASSERT_TRUE(0);
    }
    else {
        size_t  i;

        for (i = 0; i < files.gl_pathc; ++i) {
            xmlSystem = ut_read_xml(files.gl_pathv[i]);

            CU_ASSERT_PTR_NOT_NULL(xmlSystem);

            if (xmlSystem != NULL) {
                ut_free_system(xmlSystem);
            }
            else {
                (void)fprintf(stderr, "File failed: \"%s\"\n",
                    files.gl_pathv[i]);
            }
        }
    }

    globfree(&files);

    /*
     * Test again to ensure any persistent state doesn't interfere.
     */
    ut_set_error_message_handler(ut_ignore);
    xmlSystem = ut_read_xml(xmlPath);
    CU_ASSERT_PTR_NOT_NULL(xmlSystem);
    ut_free_system(xmlSystem);
}

int
main(
    const int		argc,
    const char* const*	argv)
{
    int	exitCode = EXIT_FAILURE;

    if (CU_initialize_registry() == CUE_SUCCESS) {
	CU_Suite*	testSuite = CU_add_suite(__FILE__, setup, teardown);

	if (testSuite != NULL) {
	    CU_ADD_TEST(testSuite, test_unitSystem);
	    CU_ADD_TEST(testSuite, test_utNewBaseUnit);
	    CU_ADD_TEST(testSuite, test_utNewDimensionlessUnit);
	    CU_ADD_TEST(testSuite, test_utGetUnitByName);
	    CU_ADD_TEST(testSuite, test_utGetUnitBySymbol);
	    CU_ADD_TEST(testSuite, test_utAddNamePrefix);
	    CU_ADD_TEST(testSuite, test_utAddSymbolPrefix);
	    CU_ADD_TEST(testSuite, test_utMapNameToUnit);
	    CU_ADD_TEST(testSuite, test_utMapSymbolToUnit);
	    CU_ADD_TEST(testSuite, test_utScale);
	    CU_ADD_TEST(testSuite, test_utOffset);
	    CU_ADD_TEST(testSuite, test_utOffsetByTime);
	    CU_ADD_TEST(testSuite, test_ut_decode_time);
	    CU_ADD_TEST(testSuite, test_utMultiply);
	    CU_ADD_TEST(testSuite, test_utInvert);
	    CU_ADD_TEST(testSuite, test_utDivide);
	    CU_ADD_TEST(testSuite, test_utRaise);
	    CU_ADD_TEST(testSuite, test_utRoot);
	    CU_ADD_TEST(testSuite, test_utLog);
	    CU_ADD_TEST(testSuite, test_utMapUnitToName);
	    CU_ADD_TEST(testSuite, test_utGetName);
	    CU_ADD_TEST(testSuite, test_utGetSymbol);
	    CU_ADD_TEST(testSuite, test_utToString);
	    CU_ADD_TEST(testSuite, test_utGetDimensionlessUnitOne);
	    CU_ADD_TEST(testSuite, test_utGetSystem);
	    CU_ADD_TEST(testSuite, test_utSameSystem);
	    CU_ADD_TEST(testSuite, test_utIsDimensionless);
	    CU_ADD_TEST(testSuite, test_utClone);
	    CU_ADD_TEST(testSuite, test_utAreConvertible);
	    CU_ADD_TEST(testSuite, test_utGetConverter);
	    CU_ADD_TEST(testSuite, test_utSetEncoding);
	    CU_ADD_TEST(testSuite, test_utCompare);
	    CU_ADD_TEST(testSuite, test_parsing);
	    CU_ADD_TEST(testSuite, test_visitor);
	    CU_ADD_TEST(testSuite, test_xml);
	    /*
	    */

	    ut_set_error_message_handler(ut_ignore);

	    if (CU_basic_run_tests() == CUE_SUCCESS) {
		if (CU_get_number_of_tests_failed() == 0)
		    exitCode = EXIT_SUCCESS;
	    }
	}

	CU_cleanup_registry();
    }

    return exitCode;
}
