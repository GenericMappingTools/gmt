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
#include <unistd.h>
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "udunits.h"


static utUnit		kilogram;
static utUnit		watt;


static int
setup(
    void)
{
    return utInit("/foo/bar");
}


static int
teardown(
    void)
{
    utTerm();
    return 0;
}


static void
test_utIsInit(void)
{
    CU_ASSERT_TRUE(utIsInit());
}


static void
test_utScan(void)
{
    utUnit	unit;

    utIni(&kilogram);
    CU_ASSERT_EQUAL(utScan("kg", &kilogram), 0);
    utIni(&watt);
    CU_ASSERT_EQUAL(utScan("WATT", &watt), 0);
    utIni(&unit);
    CU_ASSERT_EQUAL(utScan("seconds since 1994-12-15 12:30:00 10", &unit), 0);
    CU_ASSERT_EQUAL(utScan("Celsius @ 100", &unit), 0);
    CU_ASSERT_EQUAL(utScan("34 quatloos", &unit), UT_EUNKNOWN);
    CU_ASSERT_EQUAL(utScan("$&/^", &unit), UT_EUNKNOWN);
    CU_ASSERT_EQUAL(utScan(NULL, &unit), UT_EINVALID);
    CU_ASSERT_EQUAL(utScan("kg", NULL), UT_EINVALID);
}


static void
test_utCalendar(void)
{
    utUnit	unit;
    int		year, month, day, hour, minute;
    float	second;

    utIni(&unit);
    CU_ASSERT_EQUAL(utScan("seconds since 1994-12-15 2:29:60.0000 UTC", &unit),
	0);
    CU_ASSERT_EQUAL(
	utCalendar(1, &unit, &year, &month, &day, &hour, &minute, &second), 0);
    CU_ASSERT_EQUAL(year, 1994);
    CU_ASSERT_EQUAL(month, 12);
    CU_ASSERT_EQUAL(day, 15);
    CU_ASSERT_EQUAL(hour, 2);
    CU_ASSERT_EQUAL(minute, 30);
    CU_ASSERT_EQUAL(second, 1);
    CU_ASSERT_EQUAL(utScan("seconds since 1925-01-01 00:00:00", &unit), 0);
    CU_ASSERT_EQUAL(
	utCalendar(1, &unit, &year, &month, &day, &hour, &minute, &second), 0);
    CU_ASSERT_EQUAL(year, 1925);
    CU_ASSERT_EQUAL(month, 1);
    CU_ASSERT_EQUAL(day, 1);
    CU_ASSERT_EQUAL(hour, 0);
    CU_ASSERT_EQUAL(minute, 0);
    CU_ASSERT_EQUAL(second, 1);
}


static void
test_utInvCalendar(void)
{
    utUnit	unit;
    double	value;

    utIni(&unit);
    CU_ASSERT_EQUAL(utScan("seconds since 1994-12-15 2:29:60.0000 UTC", &unit),
	0);
    CU_ASSERT_EQUAL(utInvCalendar(1994, 12, 15, 2, 30, 1.0, &unit, &value), 0);
    CU_ASSERT_EQUAL(value, 1);
    CU_ASSERT_EQUAL(utScan("seconds since 1925-01-01 00:00:00", &unit), 0);
    CU_ASSERT_EQUAL(utInvCalendar(1925, 1, 1, 0, 0, 1.0, &unit, &value), 0);
    CU_ASSERT_EQUAL(value, 1);
}


static void
test_utIsTime(void)
{
    utUnit	unit;

    utIni(&unit);
    CU_ASSERT_EQUAL(utScan("seconds", &unit), 0);
    CU_ASSERT_TRUE(utIsTime(&unit));
    CU_ASSERT_EQUAL(utScan("seconds since 1925-01-01 00:00:00", &unit), 0);
    CU_ASSERT_TRUE(utIsTime(&unit));
    CU_ASSERT_EQUAL(utScan("kg", &unit), 0);
    CU_ASSERT_FALSE(utIsTime(&unit));
    CU_ASSERT_EQUAL(utScan("watt", &unit), 0);
    CU_ASSERT_FALSE(utIsTime(&unit));
    CU_ASSERT_EQUAL(utScan("celsius", &unit), 0);
    CU_ASSERT_FALSE(utIsTime(&unit));
}


static void
test_utHasOrigin(void)
{
    utUnit	unit;

    utIni(&unit);
    CU_ASSERT_EQUAL(utScan("seconds", &unit), 0);
    CU_ASSERT_FALSE(utHasOrigin(&unit));
    CU_ASSERT_EQUAL(utScan("seconds since 1925-01-01 00:00:00", &unit), 0);
    CU_ASSERT_TRUE(utHasOrigin(&unit));
    CU_ASSERT_EQUAL(utScan("kg", &unit), 0);
    CU_ASSERT_FALSE(utHasOrigin(&unit));
    CU_ASSERT_EQUAL(utScan("watt", &unit), 0);
    CU_ASSERT_FALSE(utHasOrigin(&unit));
    CU_ASSERT_EQUAL(utScan("celsius", &unit), 0);
    CU_ASSERT_TRUE(utHasOrigin(&unit));
}


static void
test_utClear(void)
{
    utUnit	one;
    utUnit	unit;
    double	slope, intercept;

    utIni(&one);
    utIni(&unit);
    CU_ASSERT_EQUAL(utScan("1", &one), 0);
    CU_ASSERT_EQUAL(utScan("seconds", &unit), 0);
    CU_ASSERT_EQUAL(utConvert(&one, &unit, &slope, &intercept), UT_ECONVERT);
    CU_ASSERT_PTR_NOT_NULL(utClear(&unit));
    CU_ASSERT_EQUAL(utConvert(&one, &unit, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
    CU_ASSERT_PTR_NULL(utClear(NULL));
}


static void
test_utCopy(void)
{
    utUnit	unit1;
    utUnit	unit2;
    double	slope, intercept;

    utIni(&unit1);
    utIni(&unit2);
    CU_ASSERT_EQUAL(utScan("seconds", &unit1), 0);
    CU_ASSERT_PTR_NOT_NULL(utCopy(&unit1, &unit2));
    CU_ASSERT_EQUAL(utConvert(&unit1, &unit2, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
    CU_ASSERT_PTR_NULL(utCopy(NULL, &unit2));
    CU_ASSERT_PTR_NULL(utCopy(&unit1, NULL));
}


static void
test_utMultiply(void)
{
    utUnit	m, s, result, expect;
    double	slope, intercept;

    utIni(&m);
    utIni(&s);
    utIni(&result);
    utIni(&expect);
    CU_ASSERT_EQUAL(utScan("m", &m), 0);
    CU_ASSERT_EQUAL(utScan("s", &s), 0);
    CU_ASSERT_PTR_NOT_NULL(utMultiply(&m, &s, &result));
    CU_ASSERT_EQUAL(utScan("m.s", &expect), 0);
    CU_ASSERT_EQUAL(utConvert(&result, &expect, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
    CU_ASSERT_PTR_NULL(utMultiply(NULL, &s, &result));
    CU_ASSERT_PTR_NULL(utMultiply(&m, NULL, &result));
}


static void
test_utDivide(void)
{
    utUnit	m, s, result, expect;
    double	slope, intercept;

    utIni(&m);
    utIni(&s);
    utIni(&result);
    utIni(&expect);
    CU_ASSERT_EQUAL(utScan("m", &m), 0);
    CU_ASSERT_EQUAL(utScan("s", &s), 0);
    CU_ASSERT_PTR_NOT_NULL(utDivide(&m, &s, &result));
    CU_ASSERT_EQUAL(utScan("m/s", &expect), 0);
    CU_ASSERT_EQUAL(utConvert(&result, &expect, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
    CU_ASSERT_PTR_NULL(utDivide(NULL, &s, &result));
    CU_ASSERT_PTR_NULL(utDivide(&m, NULL, &result));
}


static void
test_utInvert(void)
{
    utUnit	unit, result, expect;
    double	slope, intercept;

    utIni(&unit);
    utIni(&result);
    utIni(&expect);
    CU_ASSERT_EQUAL(utScan("m", &unit), 0);
    CU_ASSERT_PTR_NOT_NULL(utInvert(&unit, &result));
    CU_ASSERT_EQUAL(utScan("1/m", &expect), 0);
    CU_ASSERT_EQUAL(utConvert(&result, &expect, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
    CU_ASSERT_PTR_NULL(utInvert(NULL, &result));
}


static void
test_utRaise(void)
{
    utUnit	unit, result, expect;
    double	slope, intercept;

    utIni(&unit);
    utIni(&result);
    utIni(&expect);
    CU_ASSERT_EQUAL(utScan("watt", &unit), 0);
    CU_ASSERT_PTR_NOT_NULL(utRaise(&unit, 2, &result));
    CU_ASSERT_EQUAL(utScan("watt^2", &expect), 0);
    CU_ASSERT_EQUAL(utConvert(&result, &expect, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
    CU_ASSERT_PTR_NULL(utRaise(NULL, 2, &result));
}


static void
test_utShift(void)
{
    utUnit	unit, result, expect;
    double	slope, intercept;

    utIni(&unit);
    utIni(&result);
    utIni(&expect);
    CU_ASSERT_EQUAL(utScan("kelvin", &unit), 0);
    CU_ASSERT_PTR_NOT_NULL(utShift(&unit, 273.15, &result));
    CU_ASSERT_EQUAL(utScan("celsius", &expect), 0);
    CU_ASSERT_EQUAL(utConvert(&result, &expect, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
    CU_ASSERT_PTR_NULL(utShift(NULL, 2, &result));
}


static void
test_utScale(void)
{
    utUnit	unit, result, expect;
    double	slope, intercept;

    utIni(&unit);
    utIni(&result);
    utIni(&expect);
    CU_ASSERT_EQUAL(utScan("meter", &unit), 0);
    CU_ASSERT_PTR_NOT_NULL(utScale(&unit, 1000, &result));
    CU_ASSERT_EQUAL(utScan("kilometer", &expect), 0);
    CU_ASSERT_EQUAL(utConvert(&result, &expect, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
    CU_ASSERT_PTR_NULL(utScale(NULL, 2, &result));
}


static void
test_utPrint(void)
{
    utUnit	unit1, unit2;
    char*	string;
    double	slope, intercept;

    utIni(&unit1);
    utIni(&unit2);
    CU_ASSERT_EQUAL(utScan("seconds since 1994-12-15 12:30:00 10", &unit1), 0);
    CU_ASSERT_EQUAL(utPrint(&unit1, &string), 0);
    CU_ASSERT_EQUAL(utScan(string, &unit2), 0);
    CU_ASSERT_EQUAL(utConvert(&unit1, &unit2, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);

    CU_ASSERT_EQUAL(utScan("seconds since 1994-12-15 2:29:60.0000 UTC", &unit1),
	0);
    CU_ASSERT_EQUAL(utPrint(&unit1, &string), 0);
    CU_ASSERT_EQUAL(utScan(string, &unit2), 0);
    CU_ASSERT_EQUAL(utConvert(&unit1, &unit2, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);

    CU_ASSERT_EQUAL(utScan("seconds since 1925-01-01 00:00:00", &unit1), 0);
    CU_ASSERT_EQUAL(utPrint(&unit1, &string), 0);
    CU_ASSERT_EQUAL(utScan(string, &unit2), 0);
    CU_ASSERT_EQUAL(utConvert(&unit1, &unit2, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);

    CU_ASSERT_EQUAL(utScan("hours since 1-1-1 00:00:00", &unit1), 0);
    CU_ASSERT_EQUAL(utPrint(&unit1, &string), 0);
    CU_ASSERT_EQUAL(utScan(string, &unit2), 0);
    CU_ASSERT_EQUAL(utConvert(&unit1, &unit2, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);

    CU_ASSERT_EQUAL(utScan("3600 s since 1-1-1 00:00:00", &unit1), 0);
    CU_ASSERT_EQUAL(utPrint(&unit1, &string), 0);
    CU_ASSERT_EQUAL(utScan(string, &unit2), 0);
    CU_ASSERT_EQUAL(utConvert(&unit1, &unit2, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
}


static void
test_utAdd(void)
{
    utUnit	kilogram;
    utUnit	watt;
    utUnit	unit;
    utUnit	actual;
    double	slope, intercept;

    utIni(&kilogram);
    utIni(&watt);
    utIni(&unit);
    utIni(&actual);

    CU_ASSERT_EQUAL_FATAL(utScan("kg", &kilogram), 0);
    CU_ASSERT_EQUAL_FATAL(utScan("watt", &watt), 0);
    CU_ASSERT_PTR_NOT_NULL_FATAL(utMultiply(&kilogram, &watt, &unit));
    CU_ASSERT_EQUAL(utAdd("bof", 1, &unit), 0);
    CU_ASSERT_EQUAL_FATAL(utScan("bof", &actual), 0);
    CU_ASSERT_EQUAL(utConvert(&unit, &actual, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
    CU_ASSERT_EQUAL_FATAL(utScan("bofs", &actual), 0);
    CU_ASSERT_EQUAL(utConvert(&unit, &actual, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
}


static void
test_utFind(void)
{
    utUnit	unit1, unit2;
    double	slope, intercept;

    utIni(&unit1);
    utIni(&unit2);

    CU_ASSERT_EQUAL(utFind("watt", &unit1), 0);
    CU_ASSERT_EQUAL_FATAL(utScan("watt", &unit2), 0);
    CU_ASSERT_EQUAL(utConvert(&unit1, &unit2, &slope, &intercept), 0);
    CU_ASSERT_EQUAL(slope, 1);
    CU_ASSERT_EQUAL(intercept, 0);
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
	    CU_ADD_TEST(testSuite, test_utIsInit);
	    CU_ADD_TEST(testSuite, test_utScan);
	    CU_ADD_TEST(testSuite, test_utCalendar);
	    CU_ADD_TEST(testSuite, test_utInvCalendar);
	    CU_ADD_TEST(testSuite, test_utIsTime);
	    CU_ADD_TEST(testSuite, test_utHasOrigin);
	    CU_ADD_TEST(testSuite, test_utClear);
	    CU_ADD_TEST(testSuite, test_utCopy);
	    CU_ADD_TEST(testSuite, test_utMultiply);
	    CU_ADD_TEST(testSuite, test_utDivide);
	    CU_ADD_TEST(testSuite, test_utInvert);
	    CU_ADD_TEST(testSuite, test_utRaise);
	    CU_ADD_TEST(testSuite, test_utShift);
	    CU_ADD_TEST(testSuite, test_utScale);
	    CU_ADD_TEST(testSuite, test_utPrint);
	    CU_ADD_TEST(testSuite, test_utAdd);
	    CU_ADD_TEST(testSuite, test_utFind);

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
