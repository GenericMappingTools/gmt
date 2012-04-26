/*--------------------------------------------------------------------
 * $Id$
 *
 * Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 * See LICENSE.TXT file for copying and redistribution conditions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3 or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * common_math.c contains shared math functions
 *
 * Author:  Florian Wobbe
 * Date:    10-MAR-2012
 * Version: 5
 *
 * Modules in this file:
 *
 * int32_abs                   Like abs but for int32_t
 * int64_abs                   Like abs but for int64_t
 * floatAlmostEqualUlps        Compare 2 floats with ULPs (not safe for values
 *                             near zero)
 * doubleAlmostEqualUlps       Same as floatAlmostEqualUlps for double
 * floatAlmostEqualUlpsAndAbs  Compare 2 floats with an absolute epsilon
 *                             check (values near zero), then based on ULPs
 * doubleAlmostEqualUlpsAndAbs Same as floatAlmostEqualUlpsAndAbs for double
 * floatAlmostEqual            Convenience macro for floatAlmostEqualUlps
 * doubleAlmostEqual           Convenience macro for doubleAlmostEqualUlps
 * floatAlmostEqualZero        Convenience macro for floatAlmostEqualUlpsAndAbs
 * doubleAlmostEqualZero       Convenience macro for doubleAlmostEqualUlpsAndAbs
 */

/* CMake definitions: This must be first! */
#include "gmt_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#ifdef HAVE_FLOATINGPOINT_H_
#	include <floatingpoint.h>
#endif

#ifdef HAVE_ASSERT_H_
#	include <assert.h>
#else
#	define assert(e) ((void)0)
#endif

#include "common_math.h"


/* Used for accessing the integer representation of floating-point numbers
 * (aliasing through unions works on most platforms). */
union Float_t {
	int32_t i;
	float f;
};

/* Used for accessing the integer representation of double precision
 * floating-point numbers. */
union Double_t {
	int64_t i;
	double f;
};

bool floatAlmostEqualUlpsAndAbs(float A, float B,
																float maxDiff, int maxUlpsDiff) {
	/* Adapted from AlmostEqualUlpsAndAbs,
	 * http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
	 *
	 * A, B   : two floating-point numbers (single precision) to compare.
	 * maxDiff: epsilon for floating-point absolute epsilon check (should be some
	 *          small multiple of FLT_EPSILON.
	 * maxUlps: maximum spacing between the floating-point numbers A and B.
	 * ULP    = unit in the last place or unit of least precision.
	 */
	float absDiff;
	union Float_t uA, uB;
	bool signedA, signedB;
	int32_t ulpsDiff;

	/* Check if the numbers are really close -- needed when comparing numbers near zero. */
	absDiff = fabsf(A - B);
	if (absDiff <= maxDiff)
		return true;

	/* Initialize unions with floats. */
	uA.f = A;
	uB.f = B;

	/* Extract sign bits. */
	signedA = (uA.i >> 31) != 0;
	signedB = (uB.i >> 31) != 0;

	/* Different signs means they do not match. */
	if (signedA != signedB)
		return false;

	/* Find the difference in ULPs */
	ulpsDiff = int32_abs(uA.i - uB.i);
	if (ulpsDiff <= maxUlpsDiff)
		return true;

	return false;
}

bool doubleAlmostEqualUlpsAndAbs(double A, double B,
																 double maxDiff, int maxUlpsDiff) {
	/* Adapted from AlmostEqualUlpsAndAbs,
	 * http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
	 *
	 * A, B   : two floating-point numbers (double precision) to compare.
	 * maxDiff: epsilon for floating-point absolute epsilon check (should be some
	 *          small multiple of DBL_EPSILON.
	 * maxUlps: maximum spacing between the floating-point numbers A and B.
	 * ULP    = unit in the last place or unit of least precision.
	 */
	double absDiff;
	union Double_t uA, uB;
	bool signedA, signedB;
	int64_t ulpsDiff;

	/* Check if the numbers are really close -- needed when comparing numbers near zero. */
	absDiff = fabs(A - B);
	if (absDiff <= maxDiff)
		return true;

	/* Initialize unions with floats. */
	uA.f = A;
	uB.f = B;

	/* Extract sign bits. */
	signedA = (uA.i >> 63) != 0;
	signedB = (uB.i >> 63) != 0;

	/* Different signs means they do not match. */
	if (signedA != signedB)
		return false;

	/* Find the difference in ULPs */
	ulpsDiff = int64_abs(uA.i - uB.i);
	if (ulpsDiff <= maxUlpsDiff)
		return true;

	return false;
}

bool floatAlmostEqualUlps(float A, float B, int maxUlpsDiff) {
	/* Adapted from AlmostEqualUlps,
	 * http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
	 *
	 * A, B       : two floating-point numbers (single precision) to compare.
	 * maxUlpsDiff: maximum spacing between the floating-point numbers A and B.
	 * ULP        = unit in the last place or unit of least precision.
	 */
	union Float_t uA, uB;
	bool signedA, signedB;
	int32_t ulpsDiff;

	/* Ensure that either A or B are not close to zero. */
	assert ( (fabs(A) > 5 * FLT_EPSILON) || (fabs(B) > 5 * FLT_EPSILON) );

	/* Initialize unions with floats. */
	uA.f = A;
	uB.f = B;

	/* Extract sign bits. */
	signedA = (uA.i >> 31) != 0;
	signedB = (uB.i >> 31) != 0;

	/* Different signs means they do not match. */
	if (signedA != signedB) {
		/* Check for equality to make sure +0==-0 */
		if (A == B)
			/* Might be reached if assert is deactivated (-DNDEBUG) */
			return true;
		return false;
	}

	/* Find the difference in ULPs */
	ulpsDiff = int32_abs(uA.i - uB.i);
	if (ulpsDiff <= maxUlpsDiff)
		return true;

	return false;
}

bool doubleAlmostEqualUlps(double A, double B, int maxUlpsDiff) {
	/* Adapted from AlmostEqualUlps,
	 * http://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
	 *
	 * A, B       : two floating-point numbers (double precision) to compare.
	 * maxUlpsDiff: maximum spacing between the floating-point numbers A and B.
	 * ULP        = unit in the last place or unit of least precision.
	 */
	union Double_t uA, uB;
	bool signedA, signedB;
	int64_t ulpsDiff;

	/* Ensure that either A or B are not close to zero. */
	assert ( (fabs(A) > 5 * DBL_EPSILON) || (fabs(B) > 5 * DBL_EPSILON) );

	/* Initialize unions with floats. */
	uA.f = A;
	uB.f = B;

	/* Extract sign bits. */
	signedA = (uA.i >> 63) != 0;
	signedB = (uB.i >> 63) != 0;

	/* Different signs means they do not match. */
	if (signedA != signedB) {
		/* Check for equality to make sure +0==-0 */
		if (A == B)
			/* Might be reached if assert is deactivated (-DNDEBUG) */
			return true;
		return false;
	}

	/* Find the difference in ULPs */
	ulpsDiff = int64_abs(uA.i - uB.i);
	if (ulpsDiff <= maxUlpsDiff)
		return true;

	return false;
}
