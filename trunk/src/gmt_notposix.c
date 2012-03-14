/*--------------------------------------------------------------------
 *      $Id$
 *
 *      Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 *      Replacement non-POSIX functions. We will use these if your installation does not
 *      have these non-POSIX implementations itself.
 *
 * Author:      Walter H. F. Smith, P. Wessel, R. Scharroo
 * Date:        1-JAN-2010
 * Version:     5.x
 *
 * PUBLIC functions:
 *
 *      sincos: Sine and cosine
 *      j0:             Bessel function 1st kind order 0
 *      j1:             Bessel function 1st kind order 1
 *      jn:             Bessel function 1st kind order N
 *      y0:             Bessel function 2nd kind order 0
 *      y1:             Bessel function 2nd kind order 1
 *      yn:             Bessel function 2nd kind order N
 *      erf:    Error function
 *      erfc:   Complementary error function
 *      strdup: Save copy of a string
 *      strtod: Convert ascii string to floating point
 *  hypot:      sqrt(x^2 + y^2)
 *      log1p:  log(x+1)
 *      atanh:  inverse hyperbolic tangent
 */

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_internals.h"

/*
 * POSIX replacements
 */

#ifndef HAVE_BASENAME
/* basename -- extract the base portion of a pathname */
char* basename (char* path) {
	char *ptr = strrchr (path, '/');
	return ptr ? ptr + 1 : (char*)path;
}
#endif /* HAVE_BASENAME */

#ifndef HAVE_STRDUP
char *strdup (const char *s) {
	size_t n;
	char *p = NULL;

	n = strlen (s) + 1;
	p = malloc (n);
	strncpy (p, s, n);
	return (p);
}
#endif /* HAVE_STRDUP */

#ifndef HAVE_STRTOD
double strtod (const char *s, char **ends) {
/*      Given s, try to scan it to convert an ascii string representation of a
	double, and return the double so found.  If (ends != (char **)NULL),
	return a pointer to the first char of s which cannot be converted.

	This routine is supplied in GMT because it is not in the POSIX standard.  However, it
	is in ANSI standard C, and so most systems running GMT should have it as a library
	routine.  If the library routine exists, it should be used, as this one will probably
	be slower.  Also, the library routine has ways of dealing with LOCALE info on the
	radix character, and error setting for over and underflows.  Here, I rely on atof()
	to do that.

	Note that if s can be converted successfully and a non-null ends was supplied, then on
	return, *ends[0] == 0.
	*/

	char *t = NULL, savechar;
	double x = 0.0;
	GMT_LONG i, nsign[2], nradix[2], nexp, ndigits, error, inside = FALSE;

	t = s;
	i = 0;
	ndigits = 0;
	while (t[i] && isspace( (int)t[i]) ) i++;
	if (t[i] == 0 || isalpha ( (int)t[i]) ) {
		if (ends != (char **)NULL) *ends = s;
		return (x);
	}
	nsign[0] = nsign[1] = nradix[0] = nradix[1] = nexp = error = 0;
	while (t[i]) {
		if (!isdigit((int)t[i])) {
			switch (t[i]) {
				case '+':
				case '-':
					nsign[nexp]++;
					if (inside) error++;
					inside = TRUE;
					break;
				case '.':       /* This hardwires the radix char,
						instead of using LOCALE  */
					nradix[nexp]++;
					inside = TRUE;
					break;
				case 'e':
				case 'E':
					nexp++;
					inside = FALSE;
					break;
				default:
					error++;
					break;
			}
			if (nexp > 1 || nradix[nexp] > 1 || nsign[nexp] > 1) error++;
			if (error) {
				if (ndigits == 0) {
					if (ends != (char **)NULL) *ends = s;
					return (0.0);
				}
				savechar = t[i];
				t[i] = 0;
				x = atof(t);
				t[i] = savechar;
				if (ends != (char **)NULL) *ends = &t[i];
				return (x);
			}
		}
		else {
			ndigits++;
			inside = TRUE;
		}
		i++;
	}
	if (ndigits == 0) {
		if (ends != (char **)NULL) *ends = s;
		return (0.0);
	}
	x = atof(t);
	if (ends != (char **)NULL) *ends = &t[i];
	return (x);
}
#endif /* HAVE_STRTOD */

/*
 * Custom math functions
 */

#ifndef HAVE_ATANH
double atanh (double x) {
/*      Return hyperbolic arctangent of x.
	This should be used only if there isn't a better library routine available.
	WHFS 30 March 2000  */

	if (GMT_is_dnan (x)) return (x);
	if (fabs (x) >= 1.0) {
		GMT_make_dnan (x);
		return (x);
	}

	return (0.5 * (log1p (x) - log1p (-x)));
}
#endif /* HAVE_ATANH */

#if !defined(HAVE_ERF) || !defined(HAVE_ERFC)
/* Need to include the GMT error functions erf & erfc
 * since they are not in the user's local library. */

#ifndef SQRT_PI
#define SQRT_PI 0.5641895835477563
#endif

static double p[5] = {113.8641541510502, 377.4852376853020,
	3209.377589138469, 0.1857777061846032, 3.161123743870566};
static double q[4] = {244.0246379344442, 1282.616526077372,
	2844.236833439171, 23.60129095234412};
static double p1[9] = {8.883149794388376, 66.11919063714163,
	298.6351381974001, 881.9522212417691, 1712.047612634071,
	2051.078377826071, 1230.339354797997, 2.153115354744038e-8,
	0.5641884969886701};
static double q1[8] = {117.6939508913125, 537.1811018620099,
	1621.389574566690, 3290.799235733460, 4362.619090143247,
	3439.367674143722, 1230.339354803749, 15.74492611070983};
static double p2[6] = {-3.603448999498044e-01, -1.257817261112292e-01,
	-1.608378514874228e-02, -6.587491615298378e-04,
	-1.631538713730210e-02, -3.053266349612323e-01};
static double q2[5] = {1.872952849923460, 5.279051029514284e-01,
	6.051834131244132e-02, 2.335204976268692e-03,
	2.568520192289822};
#endif /* !defined(HAVE_ERF) || !defined(HAVE_ERFC) */

#ifndef HAVE_ERF
double erf (double y)
{
	GMT_LONG i, sign = 1;
	double x, res, xsq, xnum, xden, xi;

	x = y;
	if (x < 0.0) {
		sign = -1;
		x = -x;
	}
	if (x < 1.0e-10)
		res = x * p[2] / q[2];
	else if (x < 0.477) {
		xsq = x * x;
		xnum = p[3] * xsq + p[4];
		xden = xsq + q[3];
		for (i = 0; i < 3; i++) {
			xnum = xnum * xsq + p[i];
			xden = xden * xsq + q[i];
		}
		res = x * xnum / xden;
	}
	else if (x <= 4.0)  {
		xsq = x * x;
		xnum = p1[7] * x + p1[8];
		xden = x + q1[7];
		for (i = 0; i < 7; i++) {
			xnum = xnum * x + p1[i];
			xden = xden * x + q1[i];
		}
		res = 1.0 - ((xnum / xden) * exp (-xsq));
	}
	else if (x < 6.375) {
		xsq = x * x;
		xi = 1.0 / xsq;
		xnum = p2[4] * xi + p2[5];
		xden = xi + q2[4];
		for (i = 0; i < 4; i++) {
			xnum = xnum * xi + p2[i];
			xden = xden * xi + q2[i];
		}
		res = 1.0 - (((SQRT_PI + xi * xnum / xden) / x) * exp (-xsq));
	}
	else
		res = 1.0;

	if (sign == -1) res = -res;

	return (res);
}
#endif /* HAVE_ERF */

#ifndef HAVE_ERFC
double erfc (double y)
{
	GMT_LONG i, sign = 1;
	double x, res, xsq, xnum, xden, xi;

	x = y;
	if (x < 0.0) {
		sign = -1;
		x = -x;
	}
	if (x < 1.0e-10) {
		res = x * p[2] / q[2];
		if (sign == -1) res = -res;
		res = 1.0 - res;
	}
	else if (x < 0.477) {
		xsq = x * x;
		xnum = p[3] * xsq + p[4];
		xden = xsq + q[3];
		for (i = 0; i < 3; i++) {
			xnum = xnum * xsq + p[i];
			xden = xden * xsq + q[i];
		}
		res = x * xnum / xden;
		if (sign == -1) res = -res;
		res = 1.0 - res;
	}
	else if (x <= 4.0)  {
		xsq = x * x;
		xnum = p1[7] * x + p1[8];
		xden = x + q1[7];
		for (i = 0; i < 7; i++) {
			xnum = xnum * x + p1[i];
			xden = xden * x + q1[i];
		}
		res = (xnum / xden) * exp (-xsq);
		if (sign == -1) res = 2.0 - res;
	}
	else if (x < 6.375) {
		xsq = x * x;
		xi = 1.0 / xsq;
		xnum = p2[4] * xi + p2[5];
		xden = xi + q2[4];
		for (i = 0; i < 4; i++) {
			xnum = xnum * xi + p2[i];
			xden = xden * xi + q2[i];
		}
		res = ((SQRT_PI + xi * xnum / xden) / x) * exp (-xsq);
		if (sign == -1) res = 2.0 - res;
	}
	else
		res = 0.0;

	return (res);
}
#endif /* HAVE_ERFC */

#ifndef HAVE_HYPOT
double hypot (double x, double y) {
/*      Return sqrt(x*x + y*y), guarding against some overflows where possible.  If a local
	library hypot() is available, it should be used instead; it could be faster and more
	accurate.  WHFS 30 March 2000  */

	double a, b, c, d, r, s, t;

	if (GMT_is_dnan (x)) return (x);
	if (GMT_is_dnan (y)) return (y);

	/* A complete implementation of IEEE exceptional values
	would also return +Inf if either x or y is +/- Inf  */

	if (x == 0.0) return (fabs (y));
	if (y == 0.0) return (fabs (x));

	a = fabs (x);
	b = fabs (y);

	/* If POSIX defined M_SQRT2 (= sqrt(2.0) to machine
	precision) then we could say
		if (a == b) return (a * M_SQRT2);
	*/

	if (a < b) {
		c = b;
		d = a/b;
	}
	else if (a == b) {
		return (a * M_SQRT2);   /* Added 17 Aug 2001  */
	}
	else {
		c = a;
		d = b/a;
	}

	/* DBL_EPSILON is defined in POSIX float.h
	as the smallest positive e such that 1+e > 1
	in floating point.  */

	if (d < DBL_EPSILON) return (c);

	s = d*d;
	if (s > DBL_EPSILON) return (c * sqrt (1.0 + s) );

	t = 1.0 + d;
	s = (2 * (d/t)) / t;

	r = t * sqrt (1.0 - s);

	if (r <= 1.0) return (c);

	return (c * r);
}
#endif /* HAVE_HYPOT */

#ifndef HAVE_J0
/* Alternative j0 coded from Numerical Recipes by Press et al */

double j0 (double x)
{
	double ax, z, xx, y, ans, ans1, ans2, s, c;

	if ((ax = fabs (x)) < 8.0) {
		y = x * x;
		ans1 = 57568490574.0 + y * (-13362590354.0 + y * (651619640.7   + y * (-11214424.18    + y * (77392.33017   + y * (-184.9052456)))));
		ans2 = 57568490411.0 + y * (  1029532985.0 + y * (  9494680.718 + y * (    59272.64853 + y * (  267.8532712 + y * 1.0))));
		ans = ans1 / ans2;
	}
	else {
		z = 8.0 / ax;
		y = z * z;
		xx = ax - 0.785398164;
		ans1 = 1.0              + y * (-0.1098628627e-2 + y * ( 0.2734510407e-4 + y * (-0.2073370639e-5 + y * (0.2093887211e-6))));
		ans2 = -0.1562499995e-1 + y * ( 0.1430488765e-3 + y * (-0.6911147651e-5 + y * ( 0.7621095161e-6 + y * (-0.934935152e-7))));
		sincos (xx, &s, &c);
		ans = d_sqrt (0.636619772 / ax) * (c * ans1 - z * s * ans2);
	}

	return (ans);
}
#endif /* HAVE_J0 */

#ifndef HAVE_J1
/* Alternative j1 coded from Numerical Recipes by Press et al */

double j1 (double x)
{
	double ax, z, xx, y, ans, ans1, ans2, s, c;

	if ((ax = fabs (x)) < 8.0) {
		y = x * x;
		ans1 = x * (72362614232.0 + y * (-7895059235.0 + y * (242396853.1   + y * (-2972611.439    + y * (15704.48260   + y * (-30.16036606))))));
		ans2 = 144725228442.0     + y * ( 2300535178.0 + y * ( 18583304.74  + y * (   99447.43394  + y * (  376.9991397 + y * 1.0))));
		ans = ans1 / ans2;
	}
	else {
		z = 8.0 / ax;
		y = z * z;
		xx = ax - 2.356194491;
		ans1 = 1.0           + y * (  0.183105e-2     + y * (-0.3516396496e-4 + y * ( 0.2457520174e-5 + y * (-0.240337019e-6))));
		ans2 = 0.04687499995 + y * ( -0.2002690873e-3 + y * ( 0.8449199096e-5 + y * (-0.88228987e-6   + y * ( 0.105787412e-6))));
		sincos (xx, &s, &c);
		ans = d_sqrt (0.636619772 / ax) * (c * ans1 - z * s * ans2);
	}

	return (ans);
}
#endif /* HAVE_J1 */

#ifndef HAVE_JN

#define ACC 40.0
#define BIGNO 1.0e10
#define BIGNI 1.0e-10
#define IS_ZERO(x) (fabs (x) < GMT_CONV_LIMIT)

/* Alternative jn coded from Numerical Recipes by Press et al */

double jn (int n, double x)
{
	int j, jsum, m;
	double ax, bj, bjm, bjp, sum, tox, ans;

	if (n == 0) return (j0 (x));
	if (n == 1) return (j1 (x));

	ax = fabs (x);
	if (IS_ZERO (ax)) return (0.0);

	if (ax > (double)n) {   /* Upwards recurrence */
		tox = 2.0 / ax;
		bjm = j0 (ax);
		bj = j1 (ax);
		for (j = 1; j < n; j++) {
			bjp = (double)j * tox * bj - bjm;
			bjm = bj;
			bj = bjp;
		}
		ans = bj;
	}
	else {  /* More complicated here */
		tox = 2.0 / ax;
		m = 2 * ((n + (GMT_LONG) d_sqrt(ACC * n)) / 2);
		jsum = 0;
		bjp = ans = sum = 0.0;
		bj = 1.0;
		for (j = m; j > 0; j--) {
			bjm = (double)j * tox * bj - bjp;
			bjp = bj;
			bj = bjm;
			if (fabs (bj) > BIGNO) {
				bj *= BIGNI;
				bjp *= BIGNI;
				ans *= BIGNI;
				sum *= BIGNI;
			}
			if (jsum) sum += bj;
			jsum = !jsum;
			if (j == n) ans = bjp;
		}
		sum = 2.0 * sum - bj;
		ans /= sum;
	}

	return ((x < 0.0 && (n % 2) == 1) ? -ans : ans);
}
#endif /* HAVE_JN */

#ifndef HAVE_LOG1P
double log1p (double x) {
/*      Approximate log(1 + x) fairly well for small x.
	This should be used only if there isn't a better library routine available.
	WHFS 30 March 2000  */

	double u;

	if (GMT_is_dnan (x)) return (x);
	if (x <= -1.0) {
		GMT_make_dnan (u);
		return (u);
	}

	u = 1.0 + x;

	if (u == 1.0)
		return (x);
	else
		return (log(u) * (x/(u - 1.0) ) );
}
#endif /* HAVE_LOG1P */

#ifndef HAVE_RINT
	/* #define rint(x) (floor((x)+0.5f)) does not work reliable.
	 * We use s_rint.c from sun instead. */
#	include "s_rint.c"
#endif /* HAVE_RINT */

#if !defined(HAVE_SINCOS) && !defined(HAVE_ALPHASINCOS)
/* Platform does not have sincos - make a dummy one with sin and cos */
void sincos (double a, double *s, double *c)
{
	*s = sin (a);
	*c = cos (a);
}
#endif /* !defined(HAVE_SINCOS) && !defined(HAVE_ALPHASINCOS) */

#ifndef HAVE_Y0

#ifdef d_log
#undef d_log
#endif

#define d_log(x) ((x) <= 0.0 ? NAN : log (x))

/* Alternative y0, y1, yn coded from Numerical Recipes by Press et al */

double y0 (double x)
{
	double z, ax, xx, y, ans, ans1, ans2, s, c;

	if (x < 8.0) {
		y = x * x;
		ans1 = -2957821389.0 + y * (7062834065.0 + y * (-512359803.6   + y * (10879881.29    + y * (-86327.92757   + y * (228.4622733)))));
		ans2 = 40076544269.0 + y * ( 745249964.8 + y * (   7189466.438 + y * (   47447.26470 + y * (   226.1030244 + y * 1.0))));
		ans = (ans1 / ans2) + 0.636619772 * j0 (x) * d_log (x);
	}
	else {
		z = 8.0 / x;
		ax = fabs (x);
		y = z * z;
		xx = x - 0.785398164;
		ans1 = 1.0              + y * (-0.1098628627e-2 + y * ( 0.2734510407e-4 + y * (-0.2073370639e-5 + y * (0.2093887211e-6))));
		ans2 = -0.1562499995e-1 + y * ( 0.1430488765e-3 + y * (-0.6911147651e-5 + y * ( 0.7621095161e-6 + y * (-0.934935152e-7))));
		sincos (xx, &s, &c);
		ans = d_sqrt (0.636619772 / ax) * (s * ans1 - z * c * ans2);
	}

	return (ans);
}
#endif /* HAVE_Y0 */

#ifndef HAVE_Y1
/* Alternative y1 coded from Numerical Recipes by Press et al */

double y1 (double x)
{
	double z, ax, xx, y, ans, ans1, ans2, s, c;

	if (x < 8.0) {
		y = x * x;
		ans1 = x * (-0.4900604943e13 + y * (0.1275274390e13 + y * (-0.5153438139e11 + y * (0.7349264551e9 + y * (-0.4237922726e7 + y * (0.8511937935e4))))));
		ans2 = 0.2499580570e14 +       y * (0.4244419664e12 + y * ( 0.3733650367e10 + y * (0.2245904002e8 + y * ( 0.1020426050e6 + y * (0.3549632885e3) + y))));
		ans = (ans1 / ans2) + 0.636619772 * (j1 (x) * d_log (x) - 1.0 / x);
	}
	else {
		z = 8.0 / x;
		ax = fabs (x);
		y = z * z;
		xx = x - 2.356194491;
		ans1 = 1.0              + y * (0.183105e-2 + y * ( -0.3516396496e-4 + y * (0.2457520174e-5 + y * (-0.240337019e-6))));
		ans2 = 0.04687499995 + y * ( -0.2002690873e-3 + y * (0.8449199096e-5 + y * ( -0.88228987e-6 + y * (0.105787412e-6))));
		sincos (xx, &s, &c);
		ans = d_sqrt (0.636619772 / ax) * (s * ans1 - z * c * ans2);
	}

	return (ans);
}
#endif /* HAVE_Y1 */

#ifndef HAVE_YN
/* Alternative yn coded from Numerical Recipes by Press et al */

double yn (int n, double x)
{
	int j;
	double by, bym, byp, tox;

	if (n == 0) return (y0 (x));
	if (n == 1) return (y1 (x));

	tox = 2.0 / x;
	by = y1 (x);
	bym = y0 (x);
	for (j = 1; j < n; j++) {
		byp = (double)j * tox * by - bym;
		bym = by;
		by = byp;
	}

	return (by);
}
#endif /* HAVE_YN */
