/*--------------------------------------------------------------------
 *	$Id: gmt_stat.c,v 1.76 2011-03-15 02:06:36 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 *  some stuff for significance tests.
 *
 * Author:	Walter H. F. Smith
 * Date:	12-JUN-1995
 * Version:	4.1.x
 * Revised:	12-AUG-1998
 *		06-JAN-1999 P Wessel: Added GMT_ber, GMT_bei, GMT_ker, GMT_kei
 *		12-JAN-1999 P Wessel: Added GMT_plm
 *		14-JAN-1999 P Wessel: Added GMT_erf, GMT_erfc
 *		20-JAN-1999 P Wessel: Added GMT_i0, GMT_k0
 *		26-JAN-1999 P Wessel: Added GMT_i1, GMT_k1, GMT_in, GMT_kn
 *		06-MAY-1999 P Wessel: Added GMT_dilog
 *		18-JUN-1999 W Smith:  Changed GMT_ber, GMT_bei, GMT_ker, GMT_kei
 *					to speed up telescoped power series, and
 *					include asymptotic series for x > 8.
 *		21-JUN-1999 W Smith:  Revised GMT_plm and GMT_factorial.
 *		12-AUG-1999 W Smith:  Revised GMT_sig_f() to use new routines;
 *					added GMT_f_q() and GMT_student_t_a();
 *					added GMT_f_test_new().
 *		10-SEP-1999 P Wessel: Added GMT_erfinv
 *		18-OCT-1999 P Wessel: Added sincos when not already defined
 *					Added GMT_j0, GMT_j1, GMT_jn, GMT_y0,
 *					GMT_y1, GMT_yn for platforms that do no
 *					have the bessel functions in the math lib.
 *		07-DEC-1999 P. Wessel: Added GMT_rand and GMT_nrand
 *		29-MAR-2000 W Smith: Added GMT_hypot, GMT_log1p, and GMT_atanh for
 *					platforms without native library support.
 *		09-MAY-2000 P. Wessel: Added GMT_chi2 and GMT_cumpoisson
 *		18-AUG-2000 P. Wessel: Moved GMT_mode and GMT_median from gmt_support to here.
 *					Added float versions of GMT_mode for grd data.
 *		27-JUL-2005 P. Wessel: Added Chebyshev polynomials Tn(x)
 *		07-SEP-2005 P. Wessel: Added GMT_corrcoeff (x,y)
 *		24-MAR-2006 P. Wessel: Added GMT_zdist (x)
 *		06-JUL-2007 P. Wessel: Added GMT_psi () and GMT_PvQv()
 *		28-SEP-2007 R. Scharroo: Added GMT_plm_bar and made GMT_factorial public
 *
 * PUBLIC functions:
 *
 *	GMT_f_test :	Routine to compute the probability that two variances are the same
 *	GMT_f_test_new:	As above, but allows choosing 1- or 2-sided, and which side
 *	GMT_f_q:	Returns the probability integral Q(F,nu1,nu2) of the F-distribution.
 *	GMT_student_t_a:	Returns the prob integral A(t,nu) of the student-t distrib.
 *	GMT_sig_f :	Returns TRUE if reduction in model misfit was significant
 *	GMT_bei:	Kelvin-Bessel function bei(x)
 *	GMT_ber:	Kelvin-Bessel function ber(x)
 *	GMT_kei:	Kelvin-Bessel function kei(x)
 *	GMT_ker:	Kelvin-Bessel function ker(x)
 *	GMT_plm:	Legendre polynomial of degree L order M
 *	GMT_i0:		Modified Bessel function 1st kind order 0
 *	GMT_i1:		Modified Bessel function 1st kind order 1
 *	GMT_i2:		Modified Bessel function 1st kind order N
 *	GMT_k0:		Modified Kelvin function 2nd kind order 0
 *	GMT_k1:		Modified Kelvin function 2nd kind order 1
 *	GMT_kn:		Modified Kelvin function 2nd kind order N
 *	GMT_j0:		Bessel function 1st kind order 0
 *	GMT_j1:		Bessel function 1st kind order 1
 *	GMT_jn:		Bessel function 1st kind order N
 *	GMT_y0:		Bessel function 2nd kind order 0
 *	GMT_y1:		Bessel function 2nd kind order 1
 *	GMT_yn:		Bessel function 2nd kind order N
 *	GMT_dilog:	The dilog function
 *	GMT_erfinv:	The inverse error function
 *	GMT_rand:	Uniformly distributed random numbers 0 < x < 1
 *	GMT_nrand:	Normally distributed random numbers from N(0,1)
 *	GMT_corrcoeff:	Correlation coefficient.
 *	GMT_psi:	Digamma (psi) function.
 *	GMT_PvQv:	Legendre functions Pv and Qv for imaginary v and real x (-1/+1).
 *	GMT_factorial:	Factorials.
 */

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_internals.h"

GMT_LONG GMT_inc_beta (struct GMT_CTRL *C, double a, double b, double x, double *ibeta);
GMT_LONG GMT_ln_gamma_r (struct GMT_CTRL *C, double x, double *lngam);
GMT_LONG GMT_f_test_new (struct GMT_CTRL *C, double chisq1, GMT_LONG nu1, double chisq2, GMT_LONG nu2, double *prob, GMT_LONG iside);
GMT_LONG GMT_student_t_a (struct GMT_CTRL *C, double t, GMT_LONG n, double *prob);
double GMT_ln_gamma (struct GMT_CTRL *C, double xx);
double GMT_cf_beta (struct GMT_CTRL *C, double a, double b, double x);
double GMT_plm (struct GMT_CTRL *C, GMT_LONG l, GMT_LONG m, double x);
double GMT_factorial (struct GMT_CTRL *C, GMT_LONG n);
double GMT_i0 (struct GMT_CTRL *C, double x);
double GMT_i1 (struct GMT_CTRL *C, double x);
double GMT_in (struct GMT_CTRL *C, GMT_LONG n, double x);
double GMT_k0 (struct GMT_CTRL *C, double x);
double GMT_k1 (struct GMT_CTRL *C, double x);
double GMT_kn (struct GMT_CTRL *C, GMT_LONG n, double x);
double GMT_dilog (struct GMT_CTRL *C, double x);
double GMT_ln_gamma (struct GMT_CTRL *C, double xx);		/*	Computes natural log of the gamma function	*/
double GMT_erfinv (struct GMT_CTRL *C, double y);
double GMT_gammq (struct GMT_CTRL *C, double a, double x);
void GMT_gamma_cf (struct GMT_CTRL *C, double *gammcf, double a, double x, double *gln);
void GMT_gamma_ser (struct GMT_CTRL *C, double *gamser, double a, double x, double *gln);
void GMT_cumpoisson (struct GMT_CTRL *C, double k, double mu, double *prob);

#if HAVE_J0 == 0
double GMT_j0 (double x);
#endif
#if HAVE_J1 == 0
double GMT_j1 (double x);
#endif
#if HAVE_JN == 0
double GMT_jn (int n, double x);
#endif
#if HAVE_Y0 == 0
double GMT_y0 (double x);
#endif
#if HAVE_Y1 == 0
double GMT_y1 (double x);
#endif
#if HAVE_YN == 0
double GMT_yn (int n, double x);
#endif

#if HAVE_SINCOS == 0 && HAVE_ALPHASINCOS == 0
void sincos (double a, double *s, double *c);
#endif

GMT_LONG GMT_f_test_new (struct GMT_CTRL *C, double chisq1, GMT_LONG nu1, double chisq2, GMT_LONG nu2, double *prob, GMT_LONG iside)
{
	/* Given chisq1 and chisq2, random variables distributed as chi-square
		with nu1 and nu2 degrees of freedom, respectively, except that
		chisq1 is scaled by var1, and chisq2 is scaled by var2, let
		the null hypothesis, H0, be that var1 = var2.  This routine
		assigns prob, the probability that we can reject H0 in favor
		of a new hypothesis, H1, according to iside:
			iside=+1 means H1 is that var1 > var2
			iside=-1 means H1 is that var1 < var2
			iside=0  means H1 is that var1 != var2.
		This routine differs from the old GMT_f_test() by adding the
		argument iside and allowing one to choose the test.  The old
		routine in effect always set iside=0.
		This routine also differs from GMT_f_test() in that the former
		used the incomplete beta function and this one uses GMT_f_q().

		Returns 0 on success, -1 on failure.

		WHF Smith, 12 August 1999.
	*/

	double q;	/* The probability from GMT_f_q(), which is the prob
				that H0 should be retained even though
				chisq1/nu1 > chisq2/nu2.  */

	if (chisq1 <= 0.0 || chisq2 <= 0.0 || nu1 < 1 || nu2 < 1) {
		*prob = C->session.d_NaN;
		GMT_report (C, GMT_MSG_FATAL, "GMT_f_test_new:  ERROR:  Bad argument(s).\n");
		return (-1);
	}

	GMT_f_q (C, chisq1, nu1, chisq2, nu2, &q);

	if (iside > 0)
		*prob = 1.0 - q;
	else if (iside < 0)
		*prob = q;
	else if ((chisq1/nu1) <= (chisq2/nu2))
		*prob = 2.0*q;
	else
		*prob = 2.0*(1.0 - q);

	return (0);
}


GMT_LONG GMT_f_test (struct GMT_CTRL *C, double chisq1, GMT_LONG nu1, double chisq2, GMT_LONG nu2, double *prob)
{
	/* Routine to compute the probability that
		two variances are the same.
		chisq1 is distributed as chisq with
		nu1 degrees of freedom; ditto for
		chisq2 and nu2.  If these are independent
		and we form the ratio
		F = max(chisq1,chisq2)/min(chisq1,chisq2)
		then we can ask what is the probability
		that an F greater than this would occur
		by chance.  It is this probability that
		is returned in prob.  When prob is small,
		it is likely that the two chisq represent
		two different populations; the confidence
		that the two do not represent the same pop
		is 1.0 - prob.  This is a two-sided test.
	This follows some ideas in Numerical Recipes, CRC Handbook,
	and Abramowitz and Stegun.  */

	double f, df1, df2, p1, p2;

	if (chisq1 <= 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_f_test:  Chi-Square One <= 0.0\n");
		return(-1);
	}
	if (chisq2 <= 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_f_test:  Chi-Square Two <= 0.0\n");
		return(-1);
	}
	if (chisq1 > chisq2) {
		f = chisq1/chisq2;
		df1 = (double)nu1;
		df2 = (double)nu2;
	}
	else {
		f = chisq2/chisq1;
		df1 = (double)nu2;
		df2 = (double)nu1;
	}
	if (GMT_inc_beta(C, 0.5*df2, 0.5*df1, df2/(df2+df1*f), &p1) ) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_f_test:  Trouble on 1st GMT_inc_beta call.\n");
		return(-1);
	}
	if (GMT_inc_beta(C, 0.5*df1, 0.5*df2, df1/(df1+df2/f), &p2) ) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_f_test:  Trouble on 2nd GMT_inc_beta call.\n");
		return(-1);
	}
	*prob = p1 + (1.0 - p2);
	return (0);
}

GMT_LONG GMT_sig_f (struct GMT_CTRL *C, double chi1, GMT_LONG n1, double chi2, GMT_LONG n2, double level, double *prob)
{
	/* Returns TRUE if chi1/n1 significantly less than chi2/n2
		at the level level.  Returns FALSE if:
			error occurs in GMT_f_test_new();
			chi1/n1 not significantly < chi2/n2 at level.

			Changed 12 August 1999 to use GMT_f_test_new()  */

	GMT_LONG trouble;

	trouble = GMT_f_test_new (C, chi1, n1, chi2, n2, prob, -1);
	if (trouble) return (0);
	return ((*prob) >= level);
}

/* --------- LOWER LEVEL FUNCTIONS ------- */

GMT_LONG GMT_inc_beta (struct GMT_CTRL *C, double a, double b, double x, double *ibeta)
{
	double bt, gama, gamb, gamab;

	if (a <= 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_inc_beta:  Bad a (a <= 0).\n");
		return(-1);
	}
	if (b <= 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_inc_beta:  Bad b (b <= 0).\n");
		return(-1);
	}
	if (x > 0.0 && x < 1.0) {
		GMT_ln_gamma_r(C, a, &gama);
		GMT_ln_gamma_r(C, b, &gamb);
		GMT_ln_gamma_r(C, (a+b), &gamab);
		bt = exp(gamab - gama - gamb
			+ a * d_log (C, x) + b * d_log (C, 1.0 - x) );

		/* Here there is disagreement on the range of x which
			converges efficiently.  Abramowitz and Stegun
			say to use x < (a - 1) / (a + b - 2).  Editions
			of Numerical Recipes thru mid 1987 say
			x < ( (a + 1) / (a + b + 1), but the code has
			x < ( (a + 1) / (a + b + 2).  Editions printed
			late 1987 and after say x < ( (a + 1) / (a + b + 2)
			in text as well as code.  What to do ? */

		if (x < ( (a + 1) / (a + b + 2) ) )
			*ibeta = bt * GMT_cf_beta (C, a, b, x) / a;
		else
			*ibeta = 1.0 - bt * GMT_cf_beta (C, b, a, (1.0 - x) ) / b;
		return(0);
	}
	else if (x == 0.0) {
		*ibeta = 0.0;
		return (0);
	}
	else if (x == 1.0) {
		*ibeta = 1.0;
		return (0);
	}
	else if (x < 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_inc_beta:  Bad x (x < 0).\n");
		*ibeta = 0.0;
	}
	else if (x > 1.0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_inc_beta:  Bad x (x > 1).\n");
		*ibeta = 1.0;
	}
	return (-1);
}

GMT_LONG GMT_ln_gamma_r (struct GMT_CTRL *C, double x, double *lngam)
{
	/* Get natural logrithm of Gamma(x), x > 0.
		To maintain full accuracy, this
		routine uses Gamma(1 + x) / x when
		x < 1.  This routine in turn calls
		GMT_ln_gamma(x), which computes the
		actual function value.  GMT_ln_gamma
		assumes it is being called in a
		smart way, and does not check the
		range of x.  */

	if (x > 1.0) {
		*lngam = GMT_ln_gamma (C, x);
		return (0);
	}
	if (x > 0.0 && x < 1.0) {
		*lngam = GMT_ln_gamma (C, 1.0 + x) - d_log (C,x);
		return (0);
	}
	if (x == 1.0) {
		*lngam = 0.0;
		return (0);
	}
	GMT_report (C, GMT_MSG_FATAL, "Ln Gamma:  Bad x (x <= 0).\n");
	return (-1);
}

double GMT_ln_gamma (struct GMT_CTRL *C, double xx)
{
	/* Routine to compute natural log of Gamma(x)
		by Lanczos approximation.  Most accurate
		for x > 1; fails for x <= 0.  No error
		checking is done here; it is assumed
		that this is called by GMT_ln_gamma_r()  */

	static double cof[6] = {
		 76.18009173,
		-86.50532033,
		 24.01409822,
		 -1.231739516,
		0.120858003e-2,
		-0.536382e-5
	};

	static double stp = 2.50662827465, half = 0.5, one = 1.0, fpf = 5.5;
	double x, tmp, ser;

	GMT_LONG i;

	x = xx - one;
	tmp = x + fpf;
	tmp = (x + half) * d_log (C,tmp) - tmp;
	ser = one;
	for (i = 0; i < 6; i++) {
		x += one;
		ser += (cof[i]/x);
	}
	return (tmp + d_log (C,stp*ser) );
}

double GMT_cf_beta (struct GMT_CTRL *C, double a, double b, double x)
{
	/* Continued fraction method called by GMT_inc_beta.  */

	static GMT_LONG	itmax = 100;
	static double eps = 3.0e-7;

	double am = 1.0, bm = 1.0, az = 1.0;
	double qab, qap, qam, bz, em, tem, d;
	double ap, bp, app, bpp, aold;

	GMT_LONG m = 0;

	qab = a + b;
	qap = a + 1.0;
	qam = a - 1.0;
	bz = 1.0 - qab * x / qap;

	do {
		m++;
		em = (double)m;
		tem = em + em;
		d = em*(b-m)*x/((qam+tem)*(a+tem));
		ap = az+d*am;
		bp = bz+d*bm;
		d = -(a+m)*(qab+em)*x/((a+tem)*(qap+tem));
		app = ap+d*az;
		bpp = bp+d*bz;
		aold = az;
		am = ap/bpp;
		bm = bp/bpp;
		az = app/bpp;
		bz = 1.0;
	} while (((fabs (az-aold) ) >= (eps * fabs (az))) && (m < itmax));

	if (m == itmax) GMT_report (C, GMT_MSG_FATAL, "GMT_cf_beta:  A or B too big, or ITMAX too small.\n");

	return (az);
}

#define ITMAX 100

void GMT_gamma_ser (struct GMT_CTRL *C, double *gamser, double a, double x, double *gln) {
	/* Returns the incomplete gamma function P(a,x) by series rep.
	 * Press et al, gser() */

	GMT_LONG n;
	double sum, del, ap;

	GMT_ln_gamma_r (C, a, gln);

	if (x < 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT DOMAIN ERROR:  x < 0 in GMT_gamma_ser(x)\n");
		*gamser = C->session.d_NaN;
		return;
	}
	if (x == 0.0) {
		*gamser = 0.0;
		return;
	}
	ap = a;
	del = sum = 1.0 / a;
	for (n = 1; n <= ITMAX; n++) {
	 	ap += 1.0;
	 	del *= x / ap;
	 	sum += del;
	 	if (fabs (del) < fabs (sum) * DBL_EPSILON) {
	 		*gamser = sum * exp (-x  + a * log (x) - (*gln));
	 		return;
	 	}
	}
	GMT_report (C, GMT_MSG_FATAL, "GMT DOMAIN ERROR:  a too large, ITMAX too small in GMT_gamma_ser(x)\n");
}

void GMT_gamma_cf (struct GMT_CTRL *C, double *gammcf, double a, double x, double *gln) {
	/* Returns the incomplete gamma function P(a,x) by continued fraction.
	 * Press et al, gcf() */
	GMT_LONG n;
	double gold = 0.0, g, fac = 1.0, b1 = 1.0;
	double b0 = 0.0, anf, ana, an, a1, a0 = 1.0;

	GMT_ln_gamma_r (C, a, gln);

	a1 = x;
	for (n = 1; n <= ITMAX; n++) {
		an = (double) n;
		ana = an - a;
		a0 = (a1 + a0 * ana) * fac;
		b0 = (b1 + b0 * ana) * fac;
		anf = an * fac;
		a1 = x * a0 + anf * a1;
		b1 = x * b0 + anf * b1;
		if (a1 != 0.0) {
			fac = 1.0 / a1;
			g = b1 * fac;
			if (fabs ((g - gold) / g) < DBL_EPSILON) {
				*gammcf = exp (-x + a * log (x) - (*gln)) * g;
				return;
			}
			gold = g;
		}
	}
	GMT_report (C, GMT_MSG_FATAL, "GMT DOMAIN ERROR:  a too large, ITMAX too small in GMT_gamma_cf(x)\n");
}

double GMT_gammq (struct GMT_CTRL *C, double a, double x) {
	/* Returns Q(a,x) = 1 - P(a,x) Inc. Gamma function */

	double G, gln;

	if (x < 0.0 || a <= 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT DOMAIN ERROR:  Invalid arguments to GMT_gammaq\n");
		return (C->session.d_NaN);
	}

	if (x < (a + 1.0)) {
		GMT_gamma_ser (C, &G, a, x, &gln);
		return (1.0 - G);
	}
	GMT_gamma_cf (C, &G, a, x, &gln);
	return (G);
}

/*
 * Kelvin-Bessel functions, ber, bei, ker, kei.  ber(x) and bei(x) are even;
 * ker(x) and kei(x) are defined only for x > 0 and x >=0, respectively.
 * For x <= 8 we use polynomial approximations in Abramowitz & Stegun.
 * For x > 8 we use asymptotic series of Russell, quoted in Watson (Theory
 * of Bessel Functions).
 */

double GMT_ber (struct GMT_CTRL *C, double x)
{
	double t, rxsq, alpha, beta;

	if (x == 0.0) return (1.0);

	/* ber is an even function of x:  */
	x = fabs (x);

	if (x <= 8.0) {
		/* Telescoped power series from Abramowitz & Stegun  */
		t = x * 0.125;
		t *= t;
		t *= t;	/* t = pow(x/8, 4)  */
		return (1.0 + t*(-64.0 + t*(113.77777774 + t*(-32.36345652 + t*(2.64191397 + t*(-0.08349609 + t*(0.00122552 - 0.00000901 * t)))))));
	}
	else {
		/* Russell's asymptotic approximation, from Watson, p. 204  */

		rxsq = 1.0 / (x * x);
		t = x / M_SQRT2;

		alpha = t;
		beta  = t - 0.125 * M_PI;
		t *= 0.125 * rxsq;
		alpha += t;
		beta  -= t;
		beta  -= 0.0625*rxsq;
		t *= (25.0/48.0)*rxsq;
		alpha -= t;
		beta  -= t;
		alpha -= (13.0/128.0)*(rxsq*rxsq);

		return (exp (alpha) * cos (beta) / sqrt (2.0 * M_PI * x) );
	}
}

double GMT_bei (struct GMT_CTRL *C, double x)
{
	double t, rxsq, alpha, beta;

	if (x == 0.0) return (0.0);

	/* bei is an even function of x:  */
	x = fabs(x);

	if (x <= 8.0) {
		/* Telescoped power series from Abramowitz & Stegun  */
		t = x * 0.125;
		rxsq = t*t;
		t = rxsq * rxsq;	/* t = pow(x/8, 4)  */
		return (rxsq * (16.0 + t*(-113.77777774 + t*(72.81777742 + t*(-10.56765779 + t*(0.52185615 + t*(-0.01103667 + t*(0.00011346))))))));
	}
	else {
		/* Russell's asymptotic approximation, from Watson, p. 204  */

		rxsq = 1.0 / (x * x);
		t = x / M_SQRT2;

		alpha = t;
		beta  = t - 0.125 * M_PI;
		t *= 0.125 * rxsq;
		alpha += t;
		beta  -= t;
		beta  -= 0.0625*rxsq;
		t *= (25.0/48.0)*rxsq;
		alpha -= t;
		beta  -= t;
		alpha -= (13.0/128.0)*(rxsq*rxsq);

		return (exp (alpha) * sin (beta) / sqrt (2.0 * M_PI * x));
	}
}

double GMT_ker (struct GMT_CTRL *C, double x)
{
	double t, rxsq, alpha, beta;

	if (x <= 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT DOMAIN ERROR:  x <= 0 in GMT_ker(x)\n");
		return (C->session.d_NaN);
	}

	if (x <= 8.0) {
		/* Telescoped power series from Abramowitz & Stegun  */
		t = 0.125 * x;
		t *= t;
		t *= t;  /* t = pow(x/8, 4)  */
		return (-log (0.5 * x) * GMT_ber (C, x) + 0.25 * M_PI * GMT_bei (C, x) -M_EULER + \
			t * (-59.05819744 + t * (171.36272133 + t * (-60.60977451 + t * (5.65539121 + t * (-0.199636347 + t * (0.00309699 + t * (-0.00002458 * t))))))));
	}
	else {
		/* Russell's asymptotic approximation, from Watson, p. 204  */

		rxsq = 1.0 / (x * x);
		t = -x / M_SQRT2;

		alpha = t;
		beta  = t - 0.125 * M_PI;
		t *= 0.125 * rxsq;
		alpha += t;
		beta  -= t;
		beta  -= 0.0625*rxsq;
		t *= (25.0/48.0)*rxsq;
		alpha -= t;
		beta  -= t;
		alpha -= (13.0/128.0)*(rxsq*rxsq);

		return (exp (alpha) * cos (beta) / sqrt (2.0 * x / M_PI));
	}
}

double GMT_kei (struct GMT_CTRL *C, double x)
{
	double t, rxsq, alpha, beta;

	if (x <= 0.0) {
		/* Zero is valid.  If near enough to zero, return kei(0)  */
		if (x > -GMT_CONV_LIMIT) return (-0.25 * M_PI);

		GMT_report (C, GMT_MSG_FATAL, "GMT DOMAIN ERROR:  x < 0 in GMT_kei(x)\n");
		return (C->session.d_NaN);
	}

	if (x <= 8.0) {
		/* Telescoped power series from Abramowitz & Stegun  */
		t = x * 0.125;
		rxsq = t*t;
		t = rxsq * rxsq;	/* t = pow(x/8, 4)  */
		return (-log (0.5 * x) * GMT_bei (C, x) - 0.25 * M_PI * GMT_ber (C, x) +
			rxsq * (6.76454936 + t * (-142.91827687 + t * (124.23569650 + t * (-21.30060904 + t * (1.17509064 + t * (-0.02695875 + t * (0.00029532 * t))))))));
	}
	else {
		/* Russell's asymptotic approximation, from Watson, p. 204  */

		rxsq = 1.0 / (x * x);
		t = -x / M_SQRT2;

		alpha = t;
		beta  = t - 0.125 * M_PI;
		t *= 0.125 * rxsq;
		alpha += t;
		beta  -= t;
		beta  -= 0.0625*rxsq;
		t *= (25.0/48.0)*rxsq;
		alpha -= t;
		beta  -= t;
		alpha -= (13.0/128.0)*(rxsq*rxsq);

		return (exp (alpha) * sin (beta) / sqrt (2.0 * x / M_PI));
	}
}

double GMT_i0 (struct GMT_CTRL *C, double x)
{
/* Modified from code in Press et al. */
	double y, res;

	if (x < 0.0) x = -x;

	if (x < 3.75) {
		y = x * x / 14.0625;
		res = 1.0 + y * (3.5156229 + y * (3.0899424 + y * (1.2067492 + y * (0.2659732 + y * (0.360768e-1 + y * 0.45813e-2)))));
	}
	else {
		y = 3.75 / x;
		res = (exp (x) / sqrt (x)) * (0.39894228 + y * (0.1328592e-1 + y * (0.225319e-2 + y * (-0.157565e-2 + y * (0.916281e-2 + y * (-0.2057706e-1 + y * (0.2635537e-1 + y * (-0.1647633e-1 + y * 0.392377e-2))))))));
	}
	return (res);
}

double GMT_i1 (struct GMT_CTRL *C, double x)
{
	/* Modified Bessel function I1(x) */

	double y, res;

	if (x < 0.0) x = -x;
	if (x < 3.75) {
		y = pow (x / 3.75, 2.0);
		res = x * (0.5 + y * (0.87890594 + y * (0.51498869 + y * (0.15084934 + y * (0.02658733 + y * (0.00301532 + y * 0.00032411))))));
	}
	else {
		y = 3.75 / x;
		res = (exp (x) / sqrt (x)) * (0.39894228 + y * (-0.03988024 + y * (-0.00362018 + y * (0.00163801+ y * (-0.01031555 + y * (0.02282967 + y * (-0.02895312 + y * (0.01787654 -y * 0.00420059))))))));
		if (x < 0.0) res = - res;
	}
	return (res);
}

double GMT_in (struct GMT_CTRL *C, GMT_LONG n, double x)
{
	/* Modified Bessel function In(x) */

	GMT_LONG j, m, IACC = 40;
	double res, tox, bip, bi, bim;
	double BIGNO = 1.0e10, BIGNI = 1.0e-10;

	if (n == 0) return (GMT_i0 (C, x));
	if (n == 1) return (GMT_i1 (C, x));
	if (x == 0.0) return (0.0);

	tox = 2.0 / fabs (x);
	bip = res = 0.0;
	bi = 1.0;
	m = 2 * (n + irint (sqrt ((double)(IACC * n))));
	for (j = m; j >= 1; j--) {
		bim = bip + ((double)j) * tox * bi;
		bip = bi;
		bi = bim;
		if (fabs (bi) > BIGNO) {
			res *= BIGNI;
			bi *= BIGNI;
			bip *= BIGNI;
		}
		if (j == n) res = bip;
	}
	res *= (GMT_i0 (C, x) / bi);
	if (x < 0.0 && (n%2)) res = -res;

	return (res);
}

double GMT_k0 (struct GMT_CTRL *C, double x)
{
/* Modified from code in Press et al. */
	double y, z, res;

	if (x < 0.0) x = -x;

	if (x <= 2.0) {
		y = 0.25 * x * x;
		z = x * x / 14.0625;
		res = (-log(0.5*x) * (1.0 + z * (3.5156229 + z * (3.0899424 + z * (1.2067492 + z * (0.2659732 + z * (0.360768e-1 + z * 0.45813e-2))))))) + (-M_EULER + y * (0.42278420 + y * (0.23069756 + y * (0.3488590e-1 + y * (0.262698e-2 + y * (0.10750e-3 + y * 0.74e-5))))));
	}
	else {
		y = 2.0 / x;
		res = (exp (-x) / sqrt (x)) * (1.25331414 + y * (-0.7832358e-1 + y * (0.2189568e-1 + y * (-0.1062446e-1 + y * (0.587872e-2 + y * (-0.251540e-2 + y * 0.53208e-3))))));
	}
	return (res);
}

double GMT_k1 (struct GMT_CTRL *C, double x)
{
	/* Modified Bessel function K1(x) */

	double y, res;

	if (x < 0.0) x = -x;
	if (x <= 2.0) {
		y = x * x / 4.0;
		res = (log (0.5 * x) * GMT_i1 (C, x)) + (1.0 / x) * (1.0 + y * (0.15443144 + y * (-0.67278579 + y * (-0.18156897 + y * (-0.01919402 + y * (-0.00110404 - y * 0.00004686))))));
	}
	else {
		y = 2.0 / x;
		res = (exp (-x) / sqrt (x)) * (1.25331414 + y * (0.23498619 + y * (-0.03655620 + y * (0.01504268 + y * (-0.00780353 + y * (0.00325614 - y * 0.00068245))))));
	}
	return (res);
}

double GMT_kn (struct GMT_CTRL *C, GMT_LONG n, double x)
{
	/* Modified Bessel function Kn(x) */

	GMT_LONG j;
	double bkm, bk, bkp, tox;

	if (n == 0) return (GMT_k0 (C, x));
	if (n == 1) return (GMT_k1 (C, x));

	tox = 2.0 / x;
	bkm = GMT_k0 (C, x);
	bk = GMT_k1 (C, x);
	for (j = 1; j <= (n-1); j++) {
		bkp = bkm + j * tox * bk;
		bkm = bk;
		bk = bkp;
	}

	return (bk);
}

double GMT_plm (struct GMT_CTRL *C, GMT_LONG l, GMT_LONG m, double x)
{
	/* Unnormalized associated Legendre polynomial of degree l and order m, including
	 * Condon-Shortley phase (-1)^m */
	double fact, pll = 0, pmm, pmmp1, somx2;
	GMT_LONG i, ll;

	/* x is cosine of colatitude and must be -1 <= x <= +1 */
	if (fabs(x) > 1.0) {
		GMT_report (C, GMT_MSG_FATAL, "Error: |x| > 1.0 in GMT_plm_bar\n");
		return (C->session.d_NaN);
	}

	if (m < 0 || m > l) {
		GMT_report (C, GMT_MSG_FATAL, "Error: GMT_plm requires 0 <= m <= l.\n");
		return (C->session.d_NaN);
	}

	pmm = 1.0;
	if (m > 0) {
		somx2 = d_sqrt ((1.0 - x) * (1.0 + x));
		fact = 1.0;
		/* This loop used to go to i < m; corrected to i <= m by WHFS  */
		for (i = 1; i <= m; i++) {
			pmm *= -fact * somx2;
			fact += 2.0;
		}
	}
	if (l == m) return (pmm);

	pmmp1 = x * (2*m + 1) * pmm;
	if (l == (m + 1)) return (pmmp1);

	for (ll = (m+2); ll <= l; ll++) {
		pll = (x * (2*ll - 1) * pmmp1 - (ll + m - 1) * pmm) / (ll - m);
		pmm = pmmp1;
		pmmp1 = pll;
	}
	return (pll);
}

double GMT_plm_bar (struct GMT_CTRL *C, GMT_LONG l, GMT_LONG m, double x, GMT_LONG ortho)
{
	/* This function computes the normalized associated Legendre function of x for degree
	 * l and order m. u must be in the range [-1;1] and 0 <= |m| <= l.
	 * The routine is largely based on the second modified forward column method described in
	 * Holmes and Featherstone (2002). It is stable up to very high degree and order
	 * (at least 3000). This is achieved by individually computing the sectorials to P_m,m
	 * and then iterate up to P_l,m divided by P_m,m and the scale factor 1e280.
	 * Eventually, the result is multiplied again with these to terms.
	 *
	 * When ortho=FALSE, normalization is according to the geophysical convention.
	 * - The Condon-Shortley phase (-1)^m is NOT included.
	 * - The normalization factor is sqrt(k*(2l+1)*(l-m)!/(l+m)!) with k=2 except for m=0: k=1.
	 * - The integral of Plm**2 over [-1;1] is 2*k.
	 * - The integrals of (Plm*cos(m*lon))**2 and (Plm*sin(m*lon))**2 over a sphere are 4 pi.
	 *
	 * When ortho=TRUE, the results are orthonormalized.
	 * - The Condon-Shortley phase (-1)^m is NOT included.
	 * - The normalization factor is sqrt((2l+1)*(l-m)!/(l+m)!/(4*pi)).
	 * - The integral of Plm**2 over [-1;1] is 1/(2*pi).
	 * - The integral of (Plm*exp(i*m*lon))**2 over a sphere is 1.
	 *
	 * When called with -m, the Condon-Shortley phase will be included.
	 *
	 * Note that the orthonormalized form produces an integral of 1 when using imaginary terms
	 * for longitude. In geophysics, the imaginary components are split into cosine and sine terms
	 * EACH of which have an average power of 1 over the sphere (i.e. an integral of 4 pi).
	 *
	 * This routine could be further expanded to produce an array of P_m,m through P_l,m. This
	 * would be practical if Legendre functions of various degrees are to be computed at the same
	 * latitude. Also, differentials could be added.
	 *
	 * Reference:
	 * S. A. Holmes and W. E. Featherstone. A unified approach to the Clenshaw summation and the
	 * recursive computation of very high degree and order normalised associated Legendre functions.
	 * Journal of Geodesy, 76, 279-299, 2002. doi:10.1007/s00190-002-0216-2.
	 */
	GMT_LONG i;
	GMT_LONG csphase = FALSE;
	double scalef=1.0e280, u, r, pmm, pmm0, pmm1, pmm2;

	/* x is cosine of colatitude (sine of latitude) and must be -1 <= x <= +1 */

	if (fabs (x) > 1.0) {
		GMT_report (C, GMT_MSG_FATAL, "Error: |x| > 1.0 in GMT_plm_bar\n");
		return (C->session.d_NaN);
	}

	/* If m is negative, include Condon-Shortley phase */

	if (m < 0) {
		csphase = TRUE;
		m = -m;
	}

	if (m > l) {
		GMT_report (C, GMT_MSG_FATAL, "Error: GMT_plm_bar requires 0 <= m <= l.\n");
		return (C->session.d_NaN);
	}

	/* u is sine of colatitude (cosine of latitude) so that 0 <= s <= 1 */

	u = d_sqrt ((1.0 - x)*(1.0 + x));

	/* Initialize P_00=1 and compute up to P_mm using recurrence relation.
	   The result is a normalisation factor: sqrt((2l+1)(l-m)!/(l+m)!) */

	pmm = 1.0;
	for (i = 1; i <= m; i++) pmm = d_sqrt (1.0 + 0.5/i) * u * pmm;

	/* If orthonormalization is requested: multiply by sqrt(1/4pi)
	   In case of geophysical conversion : multiply by sqrt(2-delta_0m) */

	if (ortho)
		pmm *= 0.5 / d_sqrt(M_PI);
	else if (m != 0)
		pmm *= d_sqrt(2.0);

	/* If C-S phase is requested, apply it now */

	if ((m & 1) && csphase) pmm = -pmm;

	/* If l==m, we are done */

	if (l == m) return (pmm);

	/* In the next section all P_l,m are divided by P_m,m and scaled by factor scalef.
	   First compute P_m+1,m / P_m,m */

	pmm0 = 1.0/scalef;
	pmm1 = pmm0 * x * d_sqrt ((double)(2*m + 3));

	/* Use second modified column forward recurrence relation to compute P_m+2,m / P_m,m */

	for (i = m+2; i <= l; i++) {
		r = (2*i+1.0) / (i+m) / (i-m);
		pmm2 = x * pmm1 * d_sqrt(r*(2*i-1)) - pmm0 * d_sqrt(r*(i-m-1)*(i+m-1)/(2*i-3));
		pmm0 = pmm1;
		pmm1 = pmm2;
	}

	/* Return P_l,m */

	pmm1 *= pmm;
	pmm1 *= scalef;
	return (pmm1);
}

/* GMT_sinc (x) calculates the sinc function */

double GMT_sinc (struct GMT_CTRL *C, double x)
{
	if (x == 0.0) return (1.0);
	x *= M_PI;
	return (sin (x) / x);
}

/* GMT_factorial (n) calculates the factorial n! */

double GMT_factorial (struct GMT_CTRL *C, GMT_LONG n)
{
	GMT_LONG i;
	double val = 1.0;

	if (n < 0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT DOMAIN ERROR:  n < 0 in GMT_factorial(n)\n");
		return (C->session.d_NaN);
		/* This could be set to return 0 without warning, to facilitate
			sums over binomial coefficients, if desired.  -whfs  */
	}

	for (i = 1; i <= n; i++) val *= ((double)i);
	return (val);
}

double GMT_dilog (struct GMT_CTRL *C, double x)
{
	/* Compute dilog(x) (defined for x >= 0) by the method of Parker's
	   Appendix A of his Geophysical Inverse Theory.  The function
	   is needed for x in the range 0 <= x <= 1 when solving the
	   spherical spline interpolation in section 2.07 of Parker.

	   I tested this for x in range 0 to 1 by reproducing Figure
	   2.07c of Parker's book.  I also tested that the result was
	   smooth for x out to 25.  I also checked d[dilog(x)]/dx
	   obtained numerically from this routine against an analytic
	   expression, and I found the smallest angular separation
	   between two points on a sphere such that their Gram
	   matrix expression (see Parker) was different from 1; this
	   is smallest epsilon such that dilog(epsilon) - dilog(0)
	   != 0.0.  This turned out to be very much smaller than I
	   would have guessed from the apparent e-15 accuracy of
	   Parker's polynomial.  So I think this is very accurate.

	   If this is called with x < 0, it returns dilog(0) and
	   prints a warning.  It could also be set to return NaN.
	   I did this because in applications it might happen that
	   an x which should be zero is computed to be slightly off,
	   in which case the desired result is for x = 0.

	   23 Oct 1996 WHFS.   */

	double pisqon6, y, ysq, z;

	if (x < -GMT_CONV_LIMIT) return (C->session.d_NaN);	/* Tolerate minor slop before we are outside domain */

	pisqon6 = M_PI * M_PI / 6.0;
	if (x <= 0.0) return (pisqon6);	/* Meaning -GMT_CONV_LIMIT < x <= 0 */

	if (x < 0.5) {
		y = -log (1.0 - x);
		ysq = y * y;
		z = y * (1.0 + y * (-0.25 + y * (0.027777777777213 + ysq * (-2.7777776990e-04 + ysq * (4.724071696e-06 + ysq * (-9.1764954e-08 + 1.798670e-09 * ysq))))));
		return (pisqon6 - z + y * log (x));
	}
	if (x < 2.0) {
		y = -log (x);
		ysq = y * y;
		z = y * (1.0 + y * (-0.25 + y * (0.027777777777213 + ysq * (-2.7777776990e-04 + ysq * (4.724071696e-06 + ysq * (-9.1764954e-08 + 1.798670e-09 * ysq))))));
		return (z);
	}
	y = log (x);
	ysq = y * y;
	z = y * (1.0 + y * (-0.25 + y * (0.027777777777213 + ysq * (-2.7777776990e-04 + ysq * (4.724071696e-06 + ysq * (-9.1764954e-08 + 1.798670e-09 * ysq))))));
	return (-z - 0.5 * ysq);
}

#ifndef M_2_SQRTPI
#define  M_2_SQRTPI      1.12837916709551257390
#endif

double GMT_erfinv (struct GMT_CTRL *C, double y)
{
	double x = 0.0, fy, z;

	/*  Misc. efficients for expansion */

	static double a[4] = {0.886226899, -1.645349621,  0.914624893, -0.140543331};
	static double b[4] = {-2.118377725, 1.442710462, -0.329097515, 0.012229801};
	static double c[4] = {-1.970840454, -1.624906493, 3.429567803, 1.641345311};
	static double d[2] = {3.543889200, 1.637067800};

	fy = fabs (y);

	if (fy > 1.0) return (C->session.d_NaN);	/* Outside domain */

	if (GMT_IS_ZERO (1.0 - fy)) return (copysign (DBL_MAX, y));	/* Close to +- Inf */

	if (y > 0.7) {		/* Near upper range */
		z = sqrt (-log (0.5 * (1.0 - y)));
		x = (((c[3] * z + c[2]) * z + c[1]) * z + c[0]) / ((d[1] * z + d[0]) * z + 1.0);
	}
	else if (y < -0.7) {	/* Near lower range */
		z = sqrt (-log (0.5 * (1.0 + y)));
		x = -(((c[3] * z + c[2]) * z + c[1]) * z + c[0]) / ((d[1] * z + d[0]) * z + 1.0);
	}
	else {			/* Central range */
		z = y * y;
		x = y * (((a[3] * z + a[2]) * z + a[1]) * z + a[0]) / ((((b[3] * z + b[2]) * z + b[1]) * z + b[0]) * z + 1.0);
	}

	/* Apply two steps of Newton-Raphson correction to improve accuracy */

	x -= (erf (x) - y) / (M_2_SQRTPI * exp (-x * x));
	x -= (erf (x) - y) / (M_2_SQRTPI * exp (-x * x));

	return (x);
}

#if HAVE_ERF == 0 || HAVE_ERFC == 0

/* Need to include the GMT error functions GMT_erf & GMT_erfc
 * since they are not in the user's local library.
 */

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
#endif

#if HAVE_ERF == 0
double GMT_erf (double y)
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
#endif

#if HAVE_ERFC == 0
double GMT_erfc (double y)
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
#endif

#if HAVE_STRDUP == 0
char *GMT_strdup (const char *s) {
	GMT_LONG n;
	char *p = NULL;

	n = strlen (s) + 1;
	p = (char *)malloc ((size_t)n);
	strncpy (p, s, n);
	return (p);
}
#endif

#if HAVE_STRTOD == 0
double GMT_strtod (const char *s, char **ends) {

	/* Given s, try to scan it to convert an
		ascii string representation of a
		double, and return the double so
		found.  If (ends != (char **)NULL),
		return a pointer to the first char
		of s which cannot be converted.

	This routine is supplied in GMT because it
	is not in the POSIX standard.  However, it
	is in ANSI standard C, and so most systems
	running GMT should have it as a library
	routine.  If the library routine exists,
	it should be used, as this one will probably
	be slower.  Also, the library routine has
	ways of dealing with LOCALE info on the
	radix character, and error setting for
	over and underflows.  Here, I rely on atof()
	to do that.

	Note that if s can be converted successfully
	and a non-null ends was supplied, then on
	return, *ends[0] == 0.

	*/

	char *t = NULL, savechar;
	double x = 0.0;
	GMT_LONG i, nsign[2], nradix[2], nexp, ndigits, error, inside = FALSE;

	t = (char *)s;
	i = 0;
	ndigits = 0;
	while (t[i] && isspace( (int)t[i]) ) i++;
	if (t[i] == 0 || isalpha ( (int)t[i]) ) {
		if (ends != (char **)NULL) *ends = (char *)s;
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
				case '.':	/* This hardwires the radix char,
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
					if (ends != (char **)NULL) *ends = (char *)s;
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
		if (ends != (char **)NULL) *ends = (char *)s;
		return (0.0);
	}
	x = atof(t);
	if (ends != (char **)NULL) *ends = &t[i];
	return (x);
}
#endif

GMT_LONG GMT_f_q (struct GMT_CTRL *C, double chisq1, GMT_LONG nu1, double chisq2, GMT_LONG nu2, double *prob)
{
	/* Routine to compute Q(F, nu1, nu2) = 1 - P(F, nu1, nu2), where nu1
		and nu2 are positive integers, chisq1 and chisq2 are random
		variables having chi-square distributions with nu1 and nu2
		degrees of freedom, respectively (chisq1 and chisq2 >= 0.0),
		F = (chisq1/nu1)/(chisq2/nu2) has the F-distribution, and
		P(F, nu1, nu2) is the cumulative F-distribution, that is,
		the integral from 0 to (chisq1/nu1)/(chisq2/nu2) of the F-
		distribution.  Q = 1 - P is small when (chisq1/nu1)/(chisq2/nu2)
		is large with respect to 1.  That is, the value returned by
		this routine is the likelihood that an F >= (chisq1/nu1)/
		(chisq2/nu2) would occur by chance.

		Follows Abramowitz and Stegun.
		This is different from the method in Numerical Recipes, which
		uses the incomplete beta function but makes no use of the fact
		that nu1 and nu2 are known to be integers, and thus there is
		a finite limit on the sum for their expression.

		W H F Smith, August, 1999.

		REVISED by W H F Smith, October 27, 2000 after GMT 3.3.6 release.
		I found that the A&S methods overflowed for large nu1 and nu2, so
		I decided to go back to the GMT_inc_beta way of doing things.

	*/

	/* Check range of arguments:  */

	if (nu1 <= 0 || nu2 <= 0 || chisq1 < 0.0 || chisq2 < 0.0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_f_q:  Bad argument(s).\n");
		return (-1);
	}

	/* Extreme cases evaluate immediately:  */

	if (chisq1 == 0.0) {
		*prob = 1.0;
		return (0);
	}
	if (chisq2 == 0.0) {
		*prob = 0.0;
		return (0);
	}

	/* REVISION of Oct 27, 2000:  This inc beta call here returns
		the value.  All subsequent code is not used.  */

	if (GMT_inc_beta (C, 0.5*nu2, 0.5*nu1, chisq2/(chisq2+chisq1), prob) ) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_q_p:  Trouble in GMT_inc_beta call.\n");
		return (-1);
	}
	return (0);
}

GMT_LONG GMT_student_t_a (struct GMT_CTRL *C, double t, GMT_LONG n, double *prob)
{
	/* Probability integral called A(t,n) by Abramowitz &
	Stegun for the student's t distribution with n degrees
	of freedom.  Uses expressions A&S 26.7.3 and 26.7.4

	If X is distributed N(0,1) and V is distributed chi-
	square with n degrees of freedom, then
	tau = X / sqrt(V/n) is said to have Student's t-
	distribution with n degrees of freedom.  For example,
	tau could be the sample mean divided by the sample
	standard deviation, for a sample of N points; then
	n = N - 1.

	This function sets *prob = C->session.d_NaN and returns (-1)
	if t < 0.  Otherwise it sets *prob = the probability
	fabs(tau) <= t and returns (0).

	As n -> oo, we can replace this function with
	erf (t / M_SQRT2).  However, it isn't clear how large
	n has to be to make this a good approximation.  I
	consulted six books; one of them suggested this
	approximation for n >= 30, but all the others did not
	say when to use this approximation (A&S, in particular,
	does not say).  I tried some numerical experiments
	which suggested that the relative error in this
	approximation would be < 0.01 for n > 30, all t, but
	I also found that the expression here is stable to
	large n and large t, so I decided to leave it as is.

	W H F Smith, August 1999.
*/

	double	theta, s, c, csq, term, sum;
	GMT_LONG	k, kstop, kt, kb;

	if (t < 0.0 || n <= 0) {
		GMT_report (C, GMT_MSG_FATAL, "GMT_student_t_a:  Bad argument(s).\n");
		*prob = C->session.d_NaN;
		return (-1);
	}

	if (t == 0.0) {
		*prob = 0.0;
		return (0);
	}

	theta = atan (t/sqrt ((double)n));

	if (n == 1) {
		*prob = 2.0 * theta / M_PI;
		return (0);
	}

	sincos (theta, &s, &c);

	csq = c * c;

	kstop = n-2;
	if (n%2 == 1) {
		kt = 0;
		kb = 1;
		k = 1;
		term = c;
	}
	else {
		kt = -1;
		kb = 0;
		k = 0;
		term = 1.0;
	}
	sum = term;
	while (k < kstop) {
		k += 2;
		kt += 2;
		kb += 2;
		term *= (kt * csq)/kb;
		sum += term;
	}

	sum *= s;

	if (n%2 == 1)
		*prob = 2.0 * (theta + sum) / M_PI;
	else
		*prob = sum;

	/* Adjust in case of roundoff:  */

	if (*prob < 0.0) *prob = 0.0;
	if (*prob > 1.0) *prob = 1.0;

	return (0);
}

double GMT_zdist (struct GMT_CTRL *C, double x)
{
	/* Cumulative Normal (z) distribution */

	return (0.5 * (erf (x / M_SQRT2) + 1.0));
}

double GMT_zcrit (struct GMT_CTRL *C, double alpha)
{
	double sign;
	/* Critical values for Normal (z) distribution */

	/* Simple since z_a = M_SQRT2 * erf^-1 (1-a) */

	if (alpha > 0.5) {	/* right tail */
		alpha = (1.0 - alpha) * 2.0;
		sign = 1.0;
	}
	else {			/* left tail */
		alpha *= 2.0;
		sign = -1.0;
	}

	return (sign * M_SQRT2 * GMT_erfinv (C, 1.0 - alpha));
}

#if 0
/* double qsnorm(p)
 * double	p;
 *
 * Function to invert the cumulative normal probability
 * function.  If z is a standardized normal random deviate,
 * and Q(z) = p is the cumulative Gaussian probability
 * function, then z = qsnorm(p).
 *
 * Note that 0.0 < p < 1.0.  Data values outside this range
 * will return +/- a large number (1.0e6).
 * To compute p from a sample of data to test for Normalcy,
 * sort the N samples into non-decreasing order, label them
 * i=[1, N], and then compute p = i/(N+1).
 *
 * Author:	Walter H. F. Smith
 * Date:	19 February, 1991.
 *
 * Based on a Fortran subroutine by R. L. Parker.  I had been
 * using IMSL library routine DNORIN(DX) to do what qsnorm(p)
 * does, when I was at the Lamont-Doherty Geological Observatory
 * which had a site license for IMSL.  I now need to invert the
 * gaussian CDF without calling IMSL; hence, this routine.
 *
 */

double qsnorm (double p)
{
	double	t, z;

	if (p <= 0.0) {
		fprintf (stderr, "qsnorm:  Bad probability.\n");
		return (-1.0e6);
	}
	else if (p >= 1.0) {
		fprintf (stderr, "qsnorm:  Bad probability.\n");
		return (1.0e6);
	}
	else if (p == 0.5) {
		return (0.0);
	}
	else if (p > 0.5) {
		t = sqrt(-2.0 * log(1.0 - p) );
		z = t - (2.515517 +t*(0.802853 +t*0.010328))/
			(1.0 + t*(1.432788 + t*(0.189269+ t*0.001308)));
		return (z);
	}
	else {
		t = sqrt(-2.0 * log(p) );
		z = t - (2.515517 +t*(0.802853 +t*0.010328))/
			(1.0 + t*(1.432788 + t*(0.189269+ t*0.001308)));
		return (-z);
	}
}
#endif

double GMT_tcrit (struct GMT_CTRL *C, double alpha, double nu)
{
	/* Critical values for Student t-distribution */

	GMT_LONG NU, done;
	double t_low, t_high, t_mid, p_high, p_mid, p, sign;

	if (alpha > 0.5) {	/* right tail */
		p = 1 - (1 - alpha) * 2.0;
		sign = 1.0;
	}
	else {
		p = 1 - alpha * 2.0;
		sign = -1.0;
	}
	t_low = GMT_zcrit (C, alpha);
	t_high = 5.0;
	NU = (GMT_LONG)irint(nu);
	GMT_student_t_a (C, t_high, NU, &p_high);
	while (p_high < p) {	/* Must pick higher starting point */
		t_high *= 2.0;
		GMT_student_t_a (C, t_high, NU, &p_high);
	}

	/* Now, (t_low, p_low) and (t_high, p_high) are bracketing the desired (t,p) */

	done = FALSE;
	while (!done) {
		t_mid = 0.5 * (t_low + t_high);
		GMT_student_t_a (C, t_mid, NU, &p_mid);
		if (GMT_IS_ZERO (p_mid - p)) {
			done = TRUE;
		}
		else if (p_mid > p) {	/* Too high */
			t_high = t_mid;
		}
		else { /* p_mid < p */
			t_low = t_mid;
		}
	}
	return (sign * t_mid);
}

double GMT_chi2crit (struct GMT_CTRL *C, double alpha, double nu)
{
	/* Critical values for Chi^2-distribution */

	GMT_LONG done;
	double chi2_low, chi2_high, chi2_mid, p_high, p_mid, p;

	p = 1.0 - alpha;
	chi2_low = 0.0;
	chi2_high = 5.0;
	GMT_chi2 (C, chi2_high, nu, &p_high);
	while (p_high > p) {	/* Must pick higher starting point */
		chi2_high *= 2.0;
		GMT_chi2 (C, chi2_high, nu, &p_high);
	}

	/* Now, (chi2_low, p_low) and (chi2_high, p_high) are bracketing the desired (chi2,p) */

	done = FALSE;
	while (!done) {
		chi2_mid = 0.5 * (chi2_low + chi2_high);
		GMT_chi2 (C, chi2_mid, nu, &p_mid);
		if (GMT_IS_ZERO (p_mid - p)) {
			done = TRUE;
		}
		else if (p_mid < p) {	/* Too high */
			chi2_high = chi2_mid;
		}
		else { /* p_mid > p */
			chi2_low = chi2_mid;
		}
	}
	return (chi2_mid);
}

double GMT_Fcrit (struct GMT_CTRL *C, double alpha, double nu1, double nu2)
{
	/* Critical values for F-distribution */

	GMT_LONG NU1, NU2, done;
	double F_low, F_high, F_mid, p_high, p_mid, p, chisq1, chisq2;
	void F_to_ch1_ch2 (struct GMT_CTRL *C, double F, double nu1, double nu2, double *chisq1, double *chisq2);

	F_high = 5.0;
	p = 1.0 - alpha;
	F_low = 0.0;
	F_to_ch1_ch2 (C, F_high, nu1, nu2, &chisq1, &chisq2);
	NU1 = (GMT_LONG)irint (nu1);
	NU2 = (GMT_LONG)irint (nu2);
	GMT_f_q (C, chisq1, NU1, chisq2, NU2, &p_high);
	while (p_high > p) {	/* Must pick higher starting point */
		F_high *= 2.0;
		F_to_ch1_ch2 (C, F_high, nu1, nu2, &chisq1, &chisq2);
		GMT_f_q (C, chisq1, NU1, chisq2, NU2, &p_high);
	}

	/* Now, (F_low, p_low) and (F_high, p_high) are bracketing the desired (F,p) */

	done = FALSE;
	while (!done) {
		F_mid = 0.5 * (F_low + F_high);
		F_to_ch1_ch2 (C, F_mid, nu1, nu2, &chisq1, &chisq2);
		GMT_f_q (C, chisq1, NU1, chisq2, NU2, &p_mid);
		if (GMT_IS_ZERO (p_mid - p)) {
			done = TRUE;
		}
		else if (p_mid < p) {	/* Too high */
			F_high = F_mid;
		}
		else { /* p_mid > p */
			F_low = F_mid;
		}
	}
	return (F_mid);
}

void F_to_ch1_ch2 (struct GMT_CTRL *C, double F, double nu1, double nu2, double *chisq1, double *chisq2)
{	/* Silly routine to break F up into parts needed for GMT_f_q */
	*chisq2 = 1.0;
	*chisq1 = F * nu1 / nu2;
}

#if HAVE_J0 == 0

/* Alternative j0 coded from Numerical Recipes by Press et al */

double GMT_j0 (double x)
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

#endif

#if HAVE_J1 == 0

/* Alternative j1 coded from Numerical Recipes by Press et al */

double GMT_j1 (double x)
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

#endif

#if HAVE_JN == 0

#define ACC 40.0
#define BIGNO 1.0e10
#define BIGNI 1.0e-10

/* Alternative jn coded from Numerical Recipes by Press et al */

double GMT_jn (int n, double x)
{
	int j, jsum, m;
	double ax, bj, bjm, bjp, sum, tox, ans;

	if (n == 0) return (GMT_j0 (x));
	if (n == 1) return (GMT_j1 (x));

	ax = fabs (x);
	if (GMT_IS_ZERO (ax)) return (0.0);

	if (ax > (double)n) {	/* Upwards recurrence */
		tox = 2.0 / ax;
		bjm = GMT_j0 (ax);
		bj = GMT_j1 (ax);
		for (j = 1; j < n; j++) {
			bjp = (double)j * tox * bj - bjm;
			bjm = bj;
			bj = bjp;
		}
		ans = bj;
	}
	else {	/* More complicated here */
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

#endif

#if HAVE_Y0 == 0

/* Alternative y0, y1, yn coded from Numerical Recipes by Press et al */

double GMT_y0 (double x)
{
	double z, ax, xx, y, ans, ans1, ans2, s, c;

	if (x < 8.0) {
		y = x * x;
		ans1 = -2957821389.0 + y * (7062834065.0 + y * (-512359803.6   + y * (10879881.29    + y * (-86327.92757   + y * (228.4622733)))));
		ans2 = 40076544269.0 + y * ( 745249964.8 + y * (   7189466.438 + y * (   47447.26470 + y * (   226.1030244 + y * 1.0))));
		ans = (ans1 / ans2) + 0.636619772 * GMT_j0 (x) * d_log (x);
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

#endif

#if HAVE_Y1 == 0

/* Alternative y1 coded from Numerical Recipes by Press et al */

double GMT_y1 (double x)
{
	double z, ax, xx, y, ans, ans1, ans2, s, c;

	if (x < 8.0) {
		y = x * x;
		ans1 = x * (-0.4900604943e13 + y * (0.1275274390e13 + y * (-0.5153438139e11 + y * (0.7349264551e9 + y * (-0.4237922726e7 + y * (0.8511937935e4))))));
		ans2 = 0.2499580570e14 +       y * (0.4244419664e12 + y * ( 0.3733650367e10 + y * (0.2245904002e8 + y * ( 0.1020426050e6 + y * (0.3549632885e3) + y))));
		ans = (ans1 / ans2) + 0.636619772 * (GMT_j1 (x) * d_log (x) - 1.0 / x);
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

#endif

#if HAVE_YN == 0

/* Alternative yn coded from Numerical Recipes by Press et al */

double GMT_yn (int n, double x)
{
	int j;
	double by, bym, byp, tox;

	if (n == 0) return (GMT_y0 (x));
	if (n == 1) return (GMT_y1 (x));

	tox = 2.0 / x;
	by = GMT_y1 (x);
	bym = GMT_y0 (x);
	for (j = 1; j < n; j++) {
		byp = (double)j * tox * by - bym;
		bym = by;
		by = byp;
	}

	return (by);
}

#endif

#if HAVE_SINCOS == 0 && HAVE_ALPHASINCOS == 0

/* Platform does not have sincos - make a dummy one with sin and cos */

void sincos (double a, double *s, double *c)
{
	*s = sin (a);
	*c = cos (a);
}

#endif

#define GMT_RAND_IQ 127773
#define GMT_RAND_IA 16807
#define GMT_RAND_IM INT_MAX
#define GMT_RAND_AM (1.0 / GMT_RAND_IM)
#define GMT_RAND_IR 2836
#define GMT_RAND_NTAB 32
#define GMT_RAND_NDIV (1 + (GMT_RAND_IM - 1) / GMT_RAND_NTAB)
#define GMT_RAND_EPS 2.2204460492503131E-16

double GMT_rand (struct GMT_CTRL *C)
{
	/* Uniform random number generator based on ran1 of
	 * Press et al, Numerical Recipes, 2nd edition,
	 * converted from Fortran to C.  Will return values
	 * x so that 0.0 < x < 1.0 occurs with equal probability.
	 */

	static GMT_LONG GMT_rand_iy = 0;
	static GMT_LONG GMT_rand_seed = 0;
	static GMT_LONG GMT_rand_iv[GMT_RAND_NTAB];

	GMT_LONG j, k;

	if (GMT_rand_iy == 0) {	/* First time initialization */
		GMT_rand_seed = (GMT_LONG) time (NULL);		/* Seed value is positive sec since 1970 */
		if (GMT_rand_seed <= 0) GMT_rand_seed = 1;	/* Make sure to prevent seed = 0 */
		for (j = GMT_RAND_NTAB + 8 - 1; j >= 0; j--) {	/* Load shuffle table after 8 warm-ups */
			k = GMT_rand_seed / GMT_RAND_IQ;
			GMT_rand_seed = GMT_RAND_IA * (GMT_rand_seed - k * GMT_RAND_IQ) - GMT_RAND_IR * k;
			if (GMT_rand_seed < 0) GMT_rand_seed += GMT_RAND_IM;
			if (j < GMT_RAND_NTAB) GMT_rand_iv[j] = GMT_rand_seed;
		}
		GMT_rand_iy = GMT_rand_iv[0];
	}

	/* Starts here when not initializing */

	k = GMT_rand_seed / GMT_RAND_IQ;

	/* Compute GMT_rand_seed = mod (GMT_rand_seed * GMT_RAND_IA, GMT_RAND_IM) without overflowing using Schrage's method */

	GMT_rand_seed = GMT_RAND_IA * (GMT_rand_seed - k * GMT_RAND_IQ) - GMT_RAND_IR * k;
	if (GMT_rand_seed < 0) GMT_rand_seed += GMT_RAND_IM;

	j = GMT_rand_iy / GMT_RAND_NDIV;	/* Will be in the range 0:NTAB-1 */
	GMT_rand_iy = GMT_rand_iv[j];		/* Output previous shuffle value and fill table again */
	GMT_rand_iv[j] = GMT_rand_seed;
	return (MIN (GMT_RAND_AM * GMT_rand_iy, (1.0 - GMT_RAND_EPS)));	/* so we don't return 1.0 */
}

double GMT_nrand (struct GMT_CTRL *C) {
	/* Gaussian random number generator based on gasdev of
	 * Press et al, Numerical Recipes, 2nd edition.  Will
	 * return values that have zero mean and unit variance.
	 */

	static int iset = 0;
	static double gset;
	double fac, r, v1, v2;

	if (iset == 0) {	/* We don't have an extra deviate handy, so */
		do {
			v1 = 2.0 * GMT_rand (C) - 1.0;	/* Pick two uniform numbers in the -1/1/-1/1 square */
			v2 = 2.0 * GMT_rand (C) - 1.0;
			r = v1 * v1 + v2 * v2;
		} while (r >= 1.0 || r == 0.0);	/* Keep trying until v1,v2 is inside unit circle */

		fac = sqrt (-2.0 * log (r) / r);

		/* Now make Box-Muller transformation to get two normal deviates.  Return
		 * one and save the other for the next time GMT_nrand is called */

		gset = v1 * fac;
		iset = 1;	/* Set flag for next time */
		return (v2 * fac);
	}
	else {
		iset = 0;	/* Take old value, reset flag */
		return (gset);
	}
}

double GMT_lrand (struct GMT_CTRL *C) {
	/* Laplace random number generator.  As nrand, it will
	 * return values that have zero mean and unit variance.
	 */

	double rand_0_to_1;

	rand_0_to_1 = GMT_rand (C);	/* Gives uniformly distributed random values in 0-1 range */
	return (((rand_0_to_1 <= 0.5) ? log (2.0 * rand_0_to_1) : -log (2.0 * (1.0 - rand_0_to_1))) / M_SQRT2);
}

void GMT_chi2 (struct GMT_CTRL *C, double chi2, double nu, double *prob) {
	/* Evaluate probability that chi2 will exceed the
	 * theoretical chi2 by chance. */

 	*prob = GMT_gammq (C, 0.5 * nu, 0.5 * chi2);
}

void GMT_cumpoisson (struct GMT_CTRL *C, double k, double mu, double *prob) {
	/* evaluate Cumulative Poisson Distribution */

	*prob = (k == 0.0) ? exp (-mu) : GMT_gammq (C, k, mu);
}

#if HAVE_HYPOT == 0

double GMT_hypot (double x, double y) {
/* 	Return sqrt(x*x + y*y), guarding against
	some overflows where possible.  If a local
	library hypot() is available, it should be
	used instead; it could be faster and more
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
		return (a * M_SQRT2);	/* Added 17 Aug 2001  */
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

#endif

#if HAVE_LOG1P == 0

double GMT_log1p (double x) {
/* 	Approximate log(1 + x) fairly well for small x.
	This should be used only if there isn't a better
	library routine available.  WHFS 30 March 2000  */

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

#endif

#if HAVE_ATANH == 0

double GMT_atanh (double x) {
/* 	Return hyperbolic arctangent of x.
	This should be used only if there isn't a better
	library routine available.  WHFS 30 March 2000  */

	if (GMT_is_dnan (x)) return (x);
	if (fabs (x) >= 1.0) {
		GMT_make_dnan (x);
		return (x);
	}

	return (0.5 * (log1p (x) - log1p (-x)));
}

#endif

GMT_LONG GMT_median (struct GMT_CTRL *C, double *x, GMT_LONG n, double xmin, double xmax, double m_initial, double *med)
{
	double lower_bound, upper_bound, m_guess, t_0, t_1, t_middle;
	double lub, glb, xx, temp;
	GMT_LONG i, n_above, n_below, n_equal, n_lub, n_glb, one;	/* These must be signed integers */
	GMT_LONG iteration = 0, finished = FALSE;

	if (n == (GMT_LONG)0) {
		*med = m_initial;
		return (1);
	}
	if (n == (GMT_LONG)1) {
		*med = x[0];
		return (1);
	}
	if (n == (GMT_LONG)2) {
		*med = 0.5 * (x[0] + x[1]);
		return (1);
	}

	m_guess = m_initial;
	lower_bound = xmin;
	upper_bound = xmax;
	one = (GMT_LONG)1;
	t_0 = 0.0;
	t_1 = (double)(n - one);
	t_middle = 0.5 * t_1;

	do {

		n_above = n_below = n_equal = n_lub = n_glb = (GMT_LONG)0;
		lub = xmax;
		glb = xmin;

		for (i = 0; i < n; i++) {

			xx = x[i];
			if (xx == m_guess)
				n_equal++;
			else if (xx > m_guess) {
				n_above++;
				if (xx < lub) {
					lub = xx;
					n_lub = one;
				}
				else if (xx == lub)
					n_lub++;
			}
			else {
				n_below++;
				if (xx > glb) {
					glb = xx;
					n_glb = one;
				}
				else if (xx == glb)
					n_glb++;
			}
		}

		iteration++;

		/* Now test counts, watch multiple roots, think even/odd:  */

		if ((GMT_abs ((n_above - n_below))) <= n_equal) {
			*med = (n_equal) ? m_guess : 0.5 * (lub + glb);
			finished = TRUE;
		}
		else if ((GMT_abs (((n_above - n_lub) - (n_below + n_equal)))) < n_lub) {
			*med = lub;
			finished = TRUE;
		}
		else if ((GMT_abs (((n_below - n_glb) - (n_above + n_equal)))) < n_glb) {
			*med = glb;
			finished = TRUE;
		}
		/* Those cases found the median; the next two will forecast a new guess:  */

		else if (n_above > (n_below + n_equal)) {  /* Guess is too low  */
			lower_bound = m_guess;
			t_0 = (double)(n_below + n_equal - one);
			temp = lower_bound + (upper_bound - lower_bound) * (t_middle - t_0) / (t_1 - t_0);
			m_guess = (temp > lub) ? temp : lub;	/* Move guess at least to lub  */
		}
		else if (n_below > (n_above + n_equal)) {  /* Guess is too high  */
			upper_bound = m_guess;
			t_1 = (double)(n_below + n_equal - one);
			temp = lower_bound + (upper_bound - lower_bound) * (t_middle - t_0) / (t_1 - t_0);
			m_guess = (temp < glb) ? temp : glb;	/* Move guess at least to glb  */
		}
		else {	/* If we get here, I made a mistake!  */
			GMT_report (C, GMT_MSG_FATAL, "GMT Fatal Error: Internal goof - please report to developers!\n");
			GMT_exit (EXIT_FAILURE);
		}

	} while (!finished);

	/* That's all, folks!  */
	return (iteration);
}

GMT_LONG GMT_mode (struct GMT_CTRL *C, double *x, GMT_LONG n, GMT_LONG j, GMT_LONG sort, GMT_LONG mode_selection, GMT_LONG *n_multiples, double *mode_est)
{
	GMT_LONG i, istop, multiplicity;
	double mid_point_sum = 0.0, length, short_length = DBL_MAX, this_mode;

	if (n == 0) return (0);
	if (n == 1) {
		*mode_est = x[0];
		return (0);
	}

	if (sort) GMT_sort_array ((void *)x, n, GMT_DOUBLE_TYPE);

	istop = n - j;
	multiplicity = 0;

	for (i = 0; i < istop; i++) {
		length = x[i + j] - x[i];
		if (length < 0.0) {
			GMT_report (C, GMT_MSG_FATAL, "GMT_mode: Array not sorted in non-decreasing order.\n");
			return (-1);
		}
		else if (length == short_length) {	/* Possibly multiple mode */
			switch (mode_selection) {
				case -1:	/* Always pick lowest mode */
					this_mode = 0.5 * (x[i + j] + x[i]);
					if (this_mode < mid_point_sum) mid_point_sum = this_mode;
					break;
				case 0:		/* Return average of all modes */
					multiplicity++;
					mid_point_sum += (0.5 * (x[i + j] + x[i]));
					break;
				case +1:	/* Always pick highest mode */
					this_mode = 0.5 * (x[i + j] + x[i]);
					if (this_mode > mid_point_sum) mid_point_sum = this_mode;
					break;
			}
		}
		else if (length < short_length) {	/* Update current best mode estimate */
			multiplicity = 1;
			mid_point_sum = (0.5 * (x[i + j] + x[i]));
			short_length = length;
		}
	}

	if (multiplicity > 1) {	/* Found more than 1 mode; return mean of them all */
		mid_point_sum /= multiplicity;
		(*n_multiples) += multiplicity;
	}

	*mode_est = mid_point_sum;
	return (0);
}

GMT_LONG GMT_mode_f (struct GMT_CTRL *C, float *x, GMT_LONG n, GMT_LONG j, GMT_LONG sort, GMT_LONG mode_selection, GMT_LONG *n_multiples, double *mode_est)
{
	GMT_LONG i, istop, multiplicity;
	double	mid_point_sum = 0.0, length, short_length = FLT_MAX, this_mode;

	if (n == 0) return (0);
	if (n == 1) {
		*mode_est = x[0];
		return (0);
	}
	if (sort) GMT_sort_array ((void *)x, n, GMT_FLOAT_TYPE);

	istop = n - j;
	multiplicity = 0;

	for (i = 0; i < istop; i++) {
		length = x[i + j] - x[i];
		if (length < 0.0) {
			GMT_report (C, GMT_MSG_FATAL, "GMT_mode_f: Array not sorted in non-decreasing order.\n");
			return (-1);
		}
		else if (length == short_length) {	/* Possibly multiple mode */
			switch (mode_selection) {
				case -1:	/* Always pick lowest mode */
					this_mode = 0.5 * (x[i + j] + x[i]);
					if (this_mode < mid_point_sum) mid_point_sum = this_mode;
					break;
				case 0:		/* Return average of all modes */
					multiplicity++;
					mid_point_sum += (0.5 * (x[i + j] + x[i]));
					break;
				case +1:	/* Always pick highest mode */
					this_mode = 0.5 * (x[i + j] + x[i]);
					if (this_mode > mid_point_sum) mid_point_sum = this_mode;
					break;
			}
		}
		else if (length < short_length) {
			multiplicity = 1;
			mid_point_sum = (0.5 * (x[i + j] + x[i]));
			short_length = length;
		}
	}

	if (multiplicity > 1) {	/* Found more than 1 mode; return mean of them all */
		mid_point_sum /= multiplicity;
		(*n_multiples) += multiplicity;
	}

	*mode_est = mid_point_sum;
	return (0);
}

/* Replacement slower functions until we figure out the problem with the algorithm */

void GMT_getmad (struct GMT_CTRL *C, double *x, GMT_LONG n, double location, double *scale)
{
	GMT_LONG i;
	double *dev = NULL, med;

	dev = GMT_memory (C, NULL, n, double);
	for (i = 0; i < n; i++) dev[i] = fabs (x[i] - location);
	GMT_sort_array ((void *)dev, n, GMT_DOUBLE_TYPE);
	for (i = n; GMT_is_dnan (dev[i-1]) && i > 1; i--);
	if (i)
		med = (i%2) ? dev[i/2] : 0.5 * (dev[(i-1)/2] + dev[i/2]);
	else
		med = C->session.d_NaN;
	GMT_free (C, dev);
	*scale = 1.4826 * med;
}

void GMT_getmad_f (struct GMT_CTRL *C, float *x, GMT_LONG n, double location, double *scale)
{
	GMT_LONG i;
	float *dev = NULL;
	double med;

	dev = GMT_memory (C, NULL, n, float);
	for (i = 0; i < n; i++) dev[i] = (float) fabs ((double)(x[i] - location));
	GMT_sort_array ((void *)dev, n, GMT_FLOAT_TYPE);
	for (i = n; GMT_is_fnan (dev[i-1]) && i > 1; i--);
	if (i)
		med = (i%2) ? dev[i/2] : 0.5 * (dev[(i-1)/2] + dev[i/2]);
	else
		med = C->session.d_NaN;
	GMT_free (C, dev);
	*scale = 1.4826 * med;
}

void GMT_getmad_BROKEN (double *x, GMT_LONG n, double location, double *scale)
{
	/* Compute MAD (Median Absolute Deviation) for a double data set */

	double e_low, e_high, error, last_error;
	GMT_LONG i_low, i_high, n_dev, n_dev_stop;

	i_low = 0;
	while (i_low < n && x[i_low] <= location) i_low++;
	i_low--;

	i_high = n - 1;
	while (i_high >= 0 && x[i_high] >= location) i_high--;
	i_high++;

	while (i_high < i_low) i_high++, i_low--;	/* I think this must be added in (P. Wessel, 9/29/04) */

	n_dev_stop = n / 2;
	error = last_error = 0.0;
	n_dev = 0;

	while (n_dev < n_dev_stop) {

		last_error = error;

		if (i_low < 0) {
			error = x[i_high] - location;
			i_high++;
			n_dev++;
		}

		else if (i_high == n) {
			error = location - x[i_low];
			i_low--;
			n_dev++;
		}
		else {
			e_low = location - x[i_low];
			e_high = x[i_high] - location;

			if (e_low < e_high) {
				error = e_low;
				i_low--;
				n_dev++;
			}
			else if (e_high < e_low) {
				error = e_high;
				i_high++;
				n_dev++;
			}
			else {
				error = e_high;
				i_low--;
				i_high++;
				if ( !(n_dev)) n_dev--; /* First time count only once  */
				n_dev += 2;
			}
		}
	}

	*scale = (n%2) ? (1.4826 * error) : (0.7413 * (error + last_error));
}

void GMT_getmad_f_BROKEN (float *x, GMT_LONG n, double location, double *scale)
{
	/* Compute MAD (Median Absolute Deviation) for a float data set */

	double e_low, e_high, error, last_error;
	GMT_LONG i_low, i_high, n_dev, n_dev_stop;

	i_low = 0;
	while (i_low < n && x[i_low] <= location) i_low++;
	i_low--;

	i_high = n - 1;
	while (i_high >= 0 && x[i_high] >= location) i_high--;
	i_high++;

	while (i_high < i_low) i_high++, i_low--;	/* I think this must be added in (P. Wessel, 9/29/04) */

	n_dev_stop = n / 2;
	error = last_error = 0.0;
	n_dev = 0;


	while (n_dev < n_dev_stop) {

		last_error = error;

		if (i_low < 0) {
			error = x[i_high] - location;
			i_high++;
			n_dev++;
		}

		else if (i_high == n) {
			error = location - x[i_low];
			i_low--;
			n_dev++;
		}
		else {
			e_low = location - x[i_low];
			e_high = x[i_high] - location;

			if (e_low < e_high) {
				error = e_low;
				i_low--;
				n_dev++;
			}
			else if (e_high < e_low) {
				error = e_high;
				i_high++;
				n_dev++;
			}
			else {
				error = e_high;
				i_low--;
				i_high++;
				if ( !(n_dev)) n_dev--; /* First time count only once  */
				n_dev += 2;
			}
		}
	}

	*scale = (n%2) ? (1.4826 * error) : (0.7413 * (error + last_error));
}

double GMT_extreme (struct GMT_CTRL *C, double x[], GMT_LONG n, double x_default, GMT_LONG kind, GMT_LONG way)
{
	/* Returns the extreme value in the x array according to:
	*  kind: -1 means only consider negative values.
	*  kind:  0 means consider all values.
	*  kind: +1 means only consider positive values.
	*  way:  -1 means look for mimimum.
	*  way:  +1 means look for maximum.
	* If kind is non-zero we assign x_default is no values are found.
	* If kind == 0 we expect x_default to be set so that x[0] will reset x_select.
	*/

	GMT_LONG i, k;
	double x_select = C->session.d_NaN;

	for (i = k = 0; i < n; i++) {
		if (kind == -1 && x[i] > 0.0) continue;
		if (kind == +1 && x[i] < 0.0) continue;
		if (k == 0) x_select = x[i];
		if (way == -1 && x[i] < x_select) x_select = x[i];
		if (way == +1 && x[i] > x_select) x_select = x[i];
		k++;
	}

	return ((k) ? x_select : x_default);
}

GMT_LONG GMT_chebyshev (struct GMT_CTRL *C, double x, GMT_LONG n, double *t)
{
	/* Calculates the n'th Chebyshev polynomial at x */

	double x2, a, b;

	if (n < 0) GMT_err_pass (C, GMT_CHEBYSHEV_NEG_ORDER, "");
	if (fabs (x) > 1.0) GMT_err_pass (C, GMT_CHEBYSHEV_BAD_DOMAIN, "");

	switch (n) {	/* Testing the order of the polynomial */
		case 0:
			*t = 1.0;
			break;
		case 1:
			*t = x;
			break;
		case 2:
			*t = 2.0 * x * x - 1.0;
			break;
		case 3:
			*t = x * (4.0 * x * x - 3.0);
			break;
		case 4:
			x2 = x * x;
			*t = 8.0 * x2 * (x2 - 1.0) + 1.0;
			break;
		default:	/* For higher degrees we do the recursion */
			GMT_chebyshev (C, x, n-1, &a);
			GMT_chebyshev (C, x, n-2, &b);
			*t = 2.0 * x * a - b;
			break;
	}

	return (GMT_NOERROR);
}

double GMT_corrcoeff (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG mode)
{
	/* Returns plain correlation coefficient, r.
	 * If mode = 1 we assume mean(x) = mean(y) = 0.
	 */

	GMT_LONG i, n_use;
	double xmean = 0.0, ymean = 0.0, dx, dy, vx, vy, vxy, r;

	if (mode == 0) {
		for (i = n_use = 0; i < n; i++) {
			if (GMT_is_dnan (x[i]) || GMT_is_dnan (y[i])) continue;
			xmean += x[i];
			ymean += y[i];
			n_use++;
		}
		if (n_use == 0) return (C->session.d_NaN);
		xmean /= (double)n_use;
		ymean /= (double)n_use;
	}

	vx = vy = vxy = 0.0;
	for (i = n_use = 0; i < n; i++) {
		if (GMT_is_dnan (x[i]) || GMT_is_dnan (y[i])) continue;
		dx = x[i] - xmean;
		dy = y[i] - ymean;
		vx += dx * dx;
		vy += dy * dy;
		vxy += dx * dy;
	}

	r = vxy / sqrt (vx * vy);
	return (r);
}

double GMT_corrcoeff_f (struct GMT_CTRL *C, float *x, float *y, GMT_LONG n, GMT_LONG mode)
{
	/* Returns plain correlation coefficient, r.
	 * If mode = 1 we assume mean(x) = mean(y) = 0.
	 */

	GMT_LONG i, n_use;
	double xmean = 0.0, ymean = 0.0, dx, dy, vx, vy, vxy, r;

	if (mode == 0) {
		for (i = n_use = 0; i < n; i++) {
			if (GMT_is_fnan (x[i]) || GMT_is_fnan (y[i])) continue;
			xmean += (double)x[i];
			ymean += (double)y[i];
			n_use++;
		}
		if (n_use == 0) return (C->session.d_NaN);
		xmean /= (double)n_use;
		ymean /= (double)n_use;
	}

	vx = vy = vxy = 0.0;
	for (i = n_use = 0; i < n; i++) {
		if (GMT_is_fnan (x[i]) || GMT_is_fnan (y[i])) continue;
		dx = (double)x[i] - xmean;
		dy = (double)y[i] - ymean;
		vx += dx * dx;
		vy += dy * dy;
		vxy += dx * dy;
	}

	r = vxy / sqrt (vx * vy);
	return (r);
}


double GMT_quantile (struct GMT_CTRL *C, double *x, double q, GMT_LONG n)
{
	/* Returns the q'th (q in percent) quantile of x (assumed sorted).
	 * q is expected to be 0 < q < 100 */

	GMT_LONG i_f;
	double p, f, df;

	while (n > 1 && GMT_is_dnan (x[n-1])) n--;	/* Skip any NaNs at the end of x */
	if (n < 1) return (C->session.d_NaN);			/* Need at least 1 point to do something */
	if (q == 0.0) return (x[0]);			/* 0% quantile == min(x) */
	if (q == 100.0) return (x[n-1]);		/* 100% quantile == max(x) */
	f = (n - 1) * q / 100.0;
	i_f = (GMT_LONG)floor (f);
	if ((df = (f - (double)i_f)) > 0.0)		/* Must interpolate between the two neighbors */
		p = x[i_f+1] * df + x[i_f] * (1.0 - df);
	else						/* Exactly on a node */
		p = x[i_f];

	return (p);
}

double GMT_quantile_f (struct GMT_CTRL *C, float *x, double q, GMT_LONG n)
{
	/* Returns the q'th (q in percent) quantile of x (assumed sorted).
	 * q is expected to be 0 < q < 100 */

	GMT_LONG i_f;
	double p, f, df;

	while (n > 1 && GMT_is_fnan (x[n-1])) n--;	/* Skip any NaNs at the end of x */
	if (n < 1) return (C->session.d_NaN);			/* Need at least 1 point to do something */
	if (q == 0.0) return ((double)x[0]);		/* 0% quantile == min(x) */
	if (q == 100.0) return ((double)x[n-1]);	/* 100% quantile == max(x) */
	f = (n - 1) * q / 100.0;
	i_f = (GMT_LONG)floor (f);
	if ((df = (f - (double)i_f)) > 0.0)		/* Must interpolate between the two neighbors */
		p = (double)(x[i_f+1] * df + x[i_f] * (1.0 - df));
	else						/* Exactly on a node */
		p = (double)x[i_f];

	return (p);
}

#define RE 0
#define IM 1

static void Cmul (double A[], double B[], double C[])
{	/* Complex multiplication */
	C[RE] = A[RE]*B[RE] - A[IM]*B[IM];
	C[IM] = A[RE]*B[IM] + A[IM]*B[RE];
}

static void Cdiv (double A[], double B[], double C[])
{	/* Complex division */
	double denom;
	denom = B[RE]*B[RE] + B[IM]*B[IM];
	C[RE] = (A[RE]*B[RE] + A[IM]*B[IM])/denom;
	C[IM] = (A[IM]*B[RE] - A[RE]*B[IM])/denom;
}

static double Cabs (double A[])
{
	return (hypot (A[RE], A[IM]));
}

double GMT_psi (struct GMT_CTRL *P, double zz[], double p[])
{
/* Psi     Psi (or Digamma) function for complex arguments z.
*
*                 d
*        Psi(z) = --log(Gamma(z))
*                 dz
*
* zz[RE] is real and zz[IM] is imaginary component; same for p on output (only if p != NULL).
* We also return the real component as function result.
*/
	static double c[15] = { 0.99999999999999709182, 57.156235665862923517, -59.597960355475491248,
			14.136097974741747174, -0.49191381609762019978, 0.33994649984811888699e-4,
			0.46523628927048575665e-4, -0.98374475304879564677e-4, 0.15808870322491248884e-3,
			-0.21026444172410488319e-3, 0.21743961811521264320e-3, -0.16431810653676389022e-3,
			0.84418223983852743293e-4, -0.26190838401581408670e-4, 0.36899182659531622704e-5};
	double z[2], g[2], dx[2], dd[2], d[2], n[2], gg[2], f[2], x0, A[2], B[2], C[2], sx, cx, e;
	GMT_LONG k;

	if (zz[IM] == 0.0 && rint(zz[RE]) == zz[RE] && zz[RE] <= 0.0) {
		if (p) { p[RE] = P->session.d_NaN; p[IM] = 0.0;}
		return (P->session.d_NaN);	/* Singular points */
	}

	z[RE] = zz[RE];	z[IM] = zz[IM];
	if ((x0 = z[RE]) < 0.5) {	/* reflection point */
		z[RE] = 1.0 - z[RE];
		z[IM] = -z[IM];
	}

	/* Lanczos approximation */

	g[RE] = 607.0/128.0;	g[IM] = 0.0; /* best results when 4<=g<=5 */
	n[RE] = d[RE] = n[IM] = d[IM] = 0.0;
	for (k = 14; k > 0; k--) {
		A[RE] = 1.0;	A[IM] = 0.0;
		B[RE] = z[RE] + k - 1.0;	B[IM] = z[IM];
		Cdiv (A, B, dx);
		dd[RE] = c[k] * dx[RE];	dd[IM] = c[k] * dx[IM];
		d[RE] += dd[RE];	d[IM] += dd[IM];
		Cmul (dd, dx, B);
		n[RE] -= B[RE];	n[IM] -= B[IM];
	}
	d[RE] += c[RE];
	gg[RE] = z[RE] + g[RE] - 0.5;	gg[IM] = z[IM];
	Cdiv (n, d, A);
	Cdiv (g, gg, B);
	f[RE] = log (hypot(gg[RE], gg[IM])) + A[RE] - B[RE];
	f[IM] = atan2 (gg[IM], gg[RE])  + A[IM] - B[IM];
	if (x0 < 0.5) {
		C[RE] = M_PI * zz[RE];	C[IM] = M_PI * zz[IM];
		e = exp (-2*C[IM]);	sx = sin (2*C[RE]);	cx = cos (2*C[RE]);
		A[RE] = -e * sx;	A[IM] = e * cx + 1.0;
		B[RE] = e * cx - 1.0;	B[IM] = e * sx;
		Cdiv (A, B, C);
		f[RE] -= M_PI * C[RE];	f[IM] -= M_PI * C[IM];
	}
	if (p) {
		p[RE] = f[RE];
		p[IM] = f[IM];
	}
	return (f[RE]);
}

#ifndef M_SQRT_PI
#define M_SQRT_PI 1.772453850905516
#endif

#define PV_RE 0
#define PV_IM 1
#define QV_RE 2
#define QV_IM 3
void GMT_PvQv (struct GMT_CTRL *C, double x, double v_ri[], double pq[], GMT_LONG *iter)
{
	/* Here, -1 <= x <= +1, v_ri is an imaginary number [r,i], and we return
	 * the real amd imaginary parts of Pv(x) and Qv(x) in the pq array.
	 * Based on recipe in An Atlas of Functions */

	GMT_LONG p_set, q_set;
	double M, L, K, Xn, x2, k, k1, ep, em, sx, cx, fact;
	double a[2], v[2], vp1[2], G[2], g[2], u[2], t[2], f[2];
	double R[2], r[2], z[2], s[2], c[2], w[2], tmp[2], X[2], A[2], B[2];
	
	*iter = 0;
    	pq[PV_RE] = pq[QV_RE] = pq[PV_IM] = pq[QV_IM] = 0.0;	/* Initialize all answers to zero first */
	p_set = q_set = FALSE;
	if (x == -1 && v_ri[IM] == 0.0) {	/* Check special values for real nu when x = -1 */
			/* Special Pv(-1) values */
		if ((v_ri[RE] > 0.0 && fmod (v_ri[RE], 2.0) == 1.0) || (v_ri[RE] < 0.0 && fmod (v_ri[RE], 2.0) == 0.0)) {	/* v = 1,3,5,.. or -2, -4, -6 */
 			pq[PV_RE] = -1.0; p_set = TRUE;
		}
		else if ((v_ri[RE] >= 0.0 && fmod (v_ri[RE], 2.0) == 0.0) || (v_ri[RE] < 0.0 && fmod (v_ri[RE]+1.0, 2.0) == 0.0)) {	/* v = 0,2,4,6 or -1,-3,-5,-7 */
 			pq[PV_RE] = 1.0; p_set = TRUE;
		}
		else if (v_ri[RE] > 0.0 && ((fact = v_ri[RE]-2.0*floor(v_ri[RE]/2.0)) < 1.0 && fact > 0.0)) { /* 0 < v < 1, 2 < v < 3, 4 < v < 5, ... */
   			pq[PV_RE] = C->session.d_NaN; p_set = TRUE;	/* -inf */
		}
		else if (v_ri[RE] < 1.0 && ((fact = v_ri[RE]-2.0*floor(v_ri[RE]/2.0)) < 1.0 && fact > 0.0)) {	/* -2 < v < -1, -4 < v < -3, -6 < v < -5 */
   			pq[PV_RE] = C->session.d_NaN; p_set = TRUE;	/* -inf */
		}
		else if (v_ri[RE] > 1.0 && (v_ri[RE]-2.0*floor(v_ri[RE]/2.0)) > 1.0) {	/* 1 < v < 2, 3 < v < 4, 5 < v < 6, .. */
   			pq[PV_RE] = C->session.d_NaN; p_set = TRUE;	/* +inf */
		}
		else if (v_ri[RE] < 2.0 && ( v_ri[RE]-2.0*floor(v_ri[RE]/2.0)) > 1.0) {	/* -3 < v < -2, -5 < v < -4, -7 < v < -6, .. */
   			pq[PV_RE] = C->session.d_NaN; p_set = TRUE;	/* +inf */
		}
		/* Special Qv(-1) values */
		if (v_ri[RE] > 0.0 && fmod (2.0 * v_ri[RE] - 1.0, 4.0) == 0.0) {	/* v = 1/2, 5/2, 9/2, ... */
   			pq[QV_RE] = -M_PI_2; q_set = TRUE;
		}
		else if (v_ri[RE] > -1.0 && fmod (2.0 * v_ri[RE] + 1.0, 4.0) == 0.0) {	/* v = -1/2, 3/2, 7/2, ... */
   			pq[QV_RE] = M_PI_2; q_set = TRUE;
		}
		else if (v_ri[RE] < -1.0 && fmod (2.0 * v_ri[RE] - 1.0, 4.0) == 0.0) {	/* v = -3/2, -7/2, -11/2, ... */
   			pq[QV_RE] = -M_PI_2; q_set = TRUE;
		}
		else if (v_ri[RE] < -2.0 && fmod (2.0 * v_ri[RE] + 1.0, 4.0) == 0.0) {	/* v = -5/2, -9/2, -13/2, ... */
   			pq[QV_RE] = M_PI_2; q_set = TRUE;
		}
		else {	/* Either -inf or +inf */
   			pq[QV_RE] = C->session.d_NaN; q_set = TRUE;
		}
	}
	else if (x == +1 && v_ri[IM] == 0.0) {	/* Check special values for real nu when x = +1 */
		pq[PV_RE] = 1.0; p_set = TRUE;
		pq[QV_RE] = C->session.d_NaN; q_set = TRUE;
	}
	if (p_set && q_set) return;
	
	/* General case of |x| < 1 */

	a[0] = a[1] = R[RE] = 1.0;	R[IM] = 0.0;
	v[RE] = v_ri[RE];	v[IM] = v_ri[IM];
	Cmul (v, v, z);
	z[RE] = v[RE] - z[RE];	z[IM] = v[IM] - z[IM];
	K = 4.0 * sqrt (Cabs(z));
	vp1[RE] = v[RE] + 1.0;	vp1[IM] = v[IM];
	if ((Cabs(vp1) + floor(vp1[RE])) == 0.0) {
		a[0] = C->session.d_NaN;
		a[1] = 0.0;
		v[RE] = -1 - v[RE];
		v[IM] = -v[IM];
	}
	z[RE] = 0.5 * M_PI * v[RE];	z[IM] = 0.5 * M_PI * v[IM];
	ep = exp (z[IM]);	em = exp (-z[IM]);
	sincos (z[RE], &sx, &cx);
	s[RE] = 0.5 * sx * (em + ep); 
	s[IM] = -0.5 * cx * (em - ep); 
	c[RE] = 0.5 * cx * (em + ep); 
	c[IM] = 0.5 * sx * (em - ep);
	tmp[RE] = 0.5 + v[RE];	tmp[IM] = v[IM];
	Cmul (tmp, tmp, w);
	z[IM] = v[IM];
	while (v[RE] <= 6.0) {
		v[RE] = v[RE] + 2.0;
		z[RE] = v[RE] - 1.0;
		Cdiv (z, v, tmp);
		Cmul (R,tmp,r);
		R[RE] = r[RE];	R[IM] = r[IM];
	}
	z[RE] = v[RE] + 1.0;
	tmp[RE] = 0.25;	tmp[IM] = 0.0;
	Cdiv (tmp, z, X);
	tmp[RE] = 0.35 + 6.1 * X[RE];	tmp[IM] = 6.1*X[IM];
	Cmul (X, tmp, z);
	z[RE] = 1.0 - 3.0*z[RE];	z[IM] = -3.0*z[IM];
	Cmul (X, z, tmp);
	G[RE] = 1.0 + 5.0 * tmp[RE];	G[IM] = 5.0 * tmp[IM];
	z[RE] = 8.0 * X[RE];	z[IM] = 8.0 * X[IM];
	M = sqrt(hypot(z[RE], z[IM]));
	L = 0.5 * atan2 (z[IM], z[RE]);
	tmp[RE] = M * cos(L);	tmp[IM] = M * sin(L);
	Cmul (G, X, z);
	z[RE] = 1.0 - 0.5*z[RE];	z[IM] = -0.5*z[IM];
	Cmul (X, z, r);
	r[RE] = 1.0 - r[RE];	r[IM] = -r[IM];
	Cmul (R, r, z);
	Cdiv (z, tmp, R);
	u[RE] = g[RE] = 2.0 * x;	u[IM] = g[IM] = f[IM] = t[IM] = 0.0;
	f[RE] = t[RE] = 1.0;
	k = 0.5;
	x2 = x * x;
	Xn = 1.0 + (1e8/(1 - x2));
	k1 = k + 1.0;
	fact = x2 / (k1*k1 - 0.25);
	t[RE] = (k*k - w[RE]) * fact;	t[IM] = -w[IM] * fact;
	k += 1.0;
	f[RE] += t[RE];	f[IM] += t[IM];
	k1 = k + 1.0;
	fact = u[RE] * x2 / (k1*k1 - 0.25);
	u[RE] = (k*k - w[RE]) * fact;	u[IM] = -w[IM] * fact;
	k += 1;
	g[RE] += u[RE];	g[IM] += u[IM];
	tmp[RE] = Xn * t[RE];	tmp[IM] = Xn * t[IM];
	while (k < K || Cabs (tmp) > Cabs(f)) {
		(*iter)++;
		k1 = k + 1.0;
		tmp[RE] = k*k - w[RE];	tmp[IM] = -w[IM];	fact = x2 / (k1*k1 - 0.25);
		Cmul (t, tmp, A);
		t[RE] = A[RE] * fact;	t[IM] = A[IM] * fact;
		k += 1.0;
		k1 = k + 1.0;
		f[RE] += t[RE];	f[IM] += t[IM];
		tmp[RE] = k*k - w[RE];	tmp[IM] = -w[IM];	fact = x2 / (k1*k1 - 0.25);
		Cmul (u, tmp, B);
		u[RE] = B[RE] * fact;	u[IM] = B[IM] * fact;
		k += 1.0;
		g[RE] += u[RE];	g[IM] += u[IM];
		tmp[RE] = Xn * t[RE];	tmp[IM] = Xn * t[IM];
	}
	fact = x2 / (1.0 - x2);
	f[RE] += t[RE] * fact;	f[IM] += t[IM] * fact;
	g[RE] += u[RE] * fact;	g[IM] += u[IM] * fact;
	if (!p_set) {
		Cmul(s,R,z);
		Cdiv(c,R,tmp);
		Cmul (g, z, A);	Cmul (f, tmp, B);
		pq[PV_RE] = (A[RE] + B[RE])/M_SQRT_PI;
		pq[PV_IM] = (A[IM] + B[IM])/M_SQRT_PI;
	}
	if (!q_set) {
		Cmul(c,R,z);
		Cdiv(s,R,tmp);
		Cmul (g, z, A);	Cmul (f, tmp, B);
		pq[QV_RE] = a[0]*M_SQRT_PI*(A[RE] - B[RE])/2.0;
		pq[QV_IM] = a[1]*M_SQRT_PI*(A[IM] - B[IM])/2.0;
	}
}
