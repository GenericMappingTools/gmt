/*--------------------------------------------------------------------
 *	$Id: gmt_stat.c,v 1.18 2004-05-27 23:35:15 pwessel Exp $
 *
 *	Copyright (c) 1991-2004 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
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
 * Version:	3.4
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
 *		09-MAY-2000 P. Wessel: Added GMT_chi2 and GMT_cumpoission
 *		18-AUG-2000 P. Wessel: Moved GMT_mode and GMT_median from gmt_support to here.
 *					Added float versions of these two functions for grd data.
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
 */

#include "gmt.h"

int GMT_inc_beta (double a, double b, double x, double *ibeta);
int GMT_ln_gamma_r (double x, double *lngam);
int GMT_f_q (double chisq1, int nu1, double chisq2, int nu2, double *prob);	
int GMT_f_test_new (double chisq1, int nu1, double chisq2, int nu2, double *prob, int iside);
int GMT_student_t_a (double t, int n, double *prob);
double GMT_ln_gamma (double xx);
double GMT_cf_beta (double a, double b, double x);
double GMT_bei (double x);
double GMT_kei (double x);
double GMT_ber (double x);
double GMT_ker (double x);
double GMT_plm (int l, int m, double x);
double GMT_factorial (int n);
double GMT_i0 (double x);
double GMT_i1 (double x);
double GMT_in (int n, double x);
double GMT_k0 (double x);
double GMT_k1 (double x);
double GMT_kn (int n, double x);
double GMT_dilog (double x);
double GMT_ln_gamma (double xx);		/*	Computes natural log of the gamma function	*/
double GMT_erfinv (double y);
double GMT_gammq (double a, double x);
void GMT_gamma_cf (double *gammcf, double a, double x, double *gln);
void GMT_gamma_ser (double *gamser, double a, double x, double *gln);
void GMT_chi2 (double chi2, double nu, double *prob);
void GMT_cumpoission (double k, double mu, double *prob);


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

int	GMT_f_test_new (double chisq1, int nu1, double chisq2, int nu2, double *prob, int iside)
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
	
	double	q;	/* The probability from GMT_f_q(), which is the prob
				that H0 should be retained even though 
				chisq1/nu1 > chisq2/nu2.  */
	
	if (chisq1 <= 0.0 || chisq2 <= 0.0 || nu1 < 1 || nu2 < 1) {
		*prob = GMT_d_NaN;
		fprintf (stderr, "GMT_f_test_new:  ERROR:  Bad argument(s).\n");
		return (-1);
	}
	
	GMT_f_q (chisq1, nu1, chisq2, nu2, &q);
	
	if (iside > 0) {
		*prob = 1.0 - q;
	}
	else if (iside < 0) {
		*prob = q;
	}
	else if ( (chisq1/nu1) <= (chisq2/nu2) ) {
		*prob = 2.0*q;
	}
	else {
		*prob = 2.0*(1.0 - q);
	}
	
	return (0);
}


int	GMT_f_test (double chisq1, int nu1, double chisq2, int nu2, double *prob)
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

	double	f, df1, df2, p1, p2;

	if ( chisq1 <= 0.0) {
		fprintf(stderr,"GMT_f_test:  Chi-Square One <= 0.0\n");
		return(-1);
	}
	if ( chisq2 <= 0.0) {
		fprintf(stderr,"GMT_f_test:  Chi-Square Two <= 0.0\n");
		return(-1);
	}
	if (chisq1 > chisq2) {
		f = chisq1/chisq2;
		df1 = nu1;
		df2 = nu2;
	}
	else {
		f = chisq2/chisq1;
		df1 = nu2;
		df2 = nu1;
	}
	if (GMT_inc_beta(0.5*df2, 0.5*df1, df2/(df2+df1*f), &p1) ) {
		fprintf(stderr,"GMT_f_test:  Trouble on 1st GMT_inc_beta call.\n");
		return(-1);
	}
	if (GMT_inc_beta(0.5*df1, 0.5*df2, df1/(df1+df2/f), &p2) ) {
		fprintf(stderr,"GMT_f_test:  Trouble on 2nd GMT_inc_beta call.\n");
		return(-1);
	}
	*prob = p1 + (1.0 - p2);
	return(0);
}

int	GMT_sig_f (double chi1, int n1, double chi2, int n2, double level, double *prob)
{
	/* Returns TRUE if chi1/n1 significantly less than chi2/n2
		at the level level.  Returns FALSE if:
			error occurs in GMT_f_test_new();
			chi1/n1 not significantly < chi2/n2 at level.
			
			Changed 12 August 1999 to use GMT_f_test_new()  */

	int	trouble;

	trouble = GMT_f_test_new (chi1, n1, chi2, n2, prob, -1);
	if (trouble) return(0);
	return((*prob) >= level);
}

/* --------- LOWER LEVEL FUNCTIONS ------- */

int	GMT_inc_beta (double a, double b, double x, double *ibeta)
{
	double	bt, gama, gamb, gamab;

	if (a <= 0.0) {
		fprintf(stderr,"GMT_inc_beta:  Bad a (a <= 0).\n");
		return(-1);
	}
	if (b <= 0.0) {
		fprintf(stderr,"GMT_inc_beta:  Bad b (b <= 0).\n");
		return(-1);
	}
	if (x > 0.0 && x < 1.0) {
		GMT_ln_gamma_r(a, &gama);
		GMT_ln_gamma_r(b, &gamb);
		GMT_ln_gamma_r( (a+b), &gamab);
		bt = exp(gamab - gama - gamb
			+ a * d_log(x) + b * d_log(1.0 - x) );

		/* Here there is disagreement on the range of x which
			converges efficiently.  Abramowitz and Stegun
			say to use x < (a - 1) / (a + b - 2).  Editions
			of Numerical Recipes thru mid 1987 say
			x < ( (a + 1) / (a + b + 1), but the code has
			x < ( (a + 1) / (a + b + 2).  Editions printed
			late 1987 and after say x < ( (a + 1) / (a + b + 2)
			in text as well as code.  What to do ? */

		if (x < ( (a + 1) / (a + b + 2) ) ) {
			*ibeta = bt * GMT_cf_beta(a, b, x) / a;
		}
		else {
			*ibeta = 1.0 - bt * GMT_cf_beta(b, a, (1.0 - x) ) / b;
		}
		return(0);
	}
	else if (x == 0.0) {
		*ibeta = 0.0;
		return(0);
	}
	else if (x == 1.0) {
		*ibeta = 1.0;
		return(0);
	}
	else if (x < 0.0) {
		fprintf(stderr,"GMT_inc_beta:  Bad x (x < 0).\n");
		*ibeta = 0.0;
	}
	else if (x > 1.0) {
		fprintf(stderr,"GMT_inc_beta:  Bad x (x > 1).\n");
		*ibeta = 1.0;
	}
	return(-1);
}

int	GMT_ln_gamma_r(double x, double *lngam)
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
		*lngam = GMT_ln_gamma(x);
		return(0);
	}
	if (x > 0.0 && x < 1.0) {
		*lngam = GMT_ln_gamma(1.0 + x) - d_log(x);
		return(0);
	}
	if (x == 1.0) {
		*lngam = 0.0;
		return(0);
	}
	fprintf(stderr,"Ln Gamma:  Bad x (x <= 0).\n");
	return(-1);
}

double	GMT_ln_gamma (double xx)
{
	/* Routine to compute natural log of Gamma(x)
		by Lanczos approximation.  Most accurate
		for x > 1; fails for x <= 0.  No error
		checking is done here; it is assumed
		that this is called by GMT_ln_gamma_r()  */

	static double	cof[6] = {
		76.18009173,
		-86.50532033,
		24.01409822,
		-1.231739516,
		0.120858003e-2,
		-0.536382e-5
	};

	static double	stp = 2.50662827465, half = 0.5, one = 1.0, fpf = 5.5;
	double	x, tmp, ser;

	int	i;

	x = xx - one;
	tmp = x + fpf;
	tmp = (x + half) * d_log(tmp) - tmp;
	ser = one;
	for (i = 0; i < 6; i++) {
		x += one;
		ser += (cof[i]/x);
	}
	return(tmp + d_log(stp*ser) );
}

double	GMT_cf_beta (double a, double b, double x)
{
	/* Continued fraction method called by GMT_inc_beta.  */

	static int	itmax = 100;
	static double	eps = 3.0e-7;

	double	am = 1.0, bm = 1.0, az = 1.0;
	double	qab, qap, qam, bz, em, tem, d;
	double	ap, bp, app, bpp, aold;

	int	m = 0;

	qab = a + b;
	qap = a + 1.0;
	qam = a - 1.0;
	bz = 1.0 - qab * x / qap;

	do {
		m++;
		em = m;
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
	} while ( ( (fabs(az-aold) ) >= (eps * fabs(az) ) ) && (m < itmax) );

	if (m == itmax)
		fprintf(stderr,"GMT_cf_beta:  A or B too big, or ITMAX too small.\n");

	return(az);
}

#define ITMAX 100

void	GMT_gamma_ser (double *gamser, double a, double x, double *gln) {
	/* Returns the incomplete gamma function P(a,x) by series rep.
	 * Press et al, gser() */
	 
	int n;
	double sum, del, ap;
	 
	GMT_ln_gamma_r (a, gln);
	 
	if (x < 0.0) {
		fprintf (stderr, "GMT DOMAIN ERROR:  x < 0 in GMT_gamma_ser(x)\n");
		*gamser = GMT_d_NaN;
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
	fprintf (stderr, "GMT DOMAIN ERROR:  a too large, ITMAX too small in GMT_gamma_ser(x)\n");
}

void	GMT_gamma_cf (double *gammcf, double a, double x, double *gln) {
	/* Returns the incomplete gamma function P(a,x) by continued fraction.
	 * Press et al, gcf() */
	int n;
	double gold = 0.0, g, fac = 1.0, b1 = 1.0;
	double b0 = 0.0, anf, ana, an, a1, a0 = 1.0;
	
	GMT_ln_gamma_r (a, gln);

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
	fprintf (stderr, "GMT DOMAIN ERROR:  a too large, ITMAX too small in GMT_gamma_cf(x)\n");
}

double GMT_gammq (double a, double x) {
	/* Returns Q(a,x) = 1 - P(a,x) Inc. Gamma function */
	
	double G, gln;
	
	if (x < 0.0 || a <= 0.0) {
		fprintf (stderr, "GMT DOMAIN ERROR:  Invalid arguments to GMT_gammaq\n");
		return (GMT_d_NaN);
	}
	
	if (x < (a + 1.0)) {
		GMT_gamma_ser (&G, a, x, &gln);
		return (1.0 - G);
	}
	GMT_gamma_cf (&G, a, x, &gln);
	return (G);
}

/*
 * Kelvin-Bessel functions, ber, bei, ker, kei.  ber(x) and bei(x) are even;
 * ker(x) and kei(x) are defined only for x > 0 and x >=0, respectively.  
 * For x <= 8 we use polynomial approximations in Abramowitz & Stegun.
 * For x > 8 we use asymptotic series of Russell, quoted in Watson (Theory
 * of Bessel Functions).
 */
 
double GMT_ber (double x)
{
	double t, rxsq, alpha, beta;

	if (x == 0.0) return (1.0);
	
	/* ber is an even function of x:  */
	x = fabs(x);
	
	if (x <= 8.0) {
		/* Telescoped power series from Abramowitz & Stegun  */
		t = x * 0.125;
		t *= t;
		t *= t;	/* t = pow(x/8, 4)  */
		return (1.0 + t*(-64.0 + t*(
			113.77777774 + t*(
			-32.36345652 + t*(
			2.64191397 + t*(
			-0.08349609 + t*(
			0.00122552 - 0.00000901 * t)))))));
	}
	else {
		/* Russell's asymptotic approximation,
			from Watson, p. 204  */
		
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

		return ( exp(alpha) * cos(beta) / sqrt(2.0 * M_PI * x) );
	}
}

double GMT_bei (double x)
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
		return (rxsq * (16.0 + t*(
			-113.77777774 + t*(
			72.81777742 + t*(
			-10.56765779 + t*(
			0.52185615 + t*(
			-0.01103667 + t*(
			0.00011346))))))));
	}
	else {
		/* Russell's asymptotic approximation,
			from Watson, p. 204  */

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

		return ( exp(alpha) * sin(beta) / sqrt(2.0 * M_PI * x) );
	}
}

double GMT_ker (double x)
{
	double t, rxsq, alpha, beta;

	if (x <= 0.0) {
		fprintf (stderr, "GMT DOMAIN ERROR:  x <= 0 in GMT_ker(x)\n");
		return (GMT_d_NaN);
	}
	
	if (x <= 8.0) {
		/* Telescoped power series from Abramowitz & Stegun  */
		t = 0.125 * x;
		t *= t;
		t *= t;  /* t = pow(x/8, 4)  */
		return (-log(0.5 * x) * GMT_ber(x) + 0.25 * M_PI * GMT_bei(x)
			-0.57721566  + t * (
			-59.05819744 + t * (
			171.36272133 + t * (
			-60.60977451 + t * (
			5.65539121   + t * (
			-0.199636347 + t * (
			0.00309699   + t * (
			-0.00002458 * t))))))));
	}	
	else {
		/* Russell's asymptotic approximation,
			from Watson, p. 204  */
	
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

		return ( exp(alpha) * cos(beta) / sqrt(2.0 * x / M_PI) );
	}
}

double GMT_kei (double x)
{
	double t, rxsq, alpha, beta;
	
	if (x <= 0.0) {
		/* Zero is valid.  If near enough to zero, return kei(0)  */
		if (x > -GMT_CONV_LIMIT) return (-0.25 * M_PI);
	
		fprintf (stderr, "GMT DOMAIN ERROR:  x < 0 in GMT_kei(x)\n");
		return (GMT_d_NaN);
	}

	if (x <= 8.0) {
		/* Telescoped power series from Abramowitz & Stegun  */
		t = x * 0.125;
		rxsq = t*t;
		t = rxsq * rxsq;	/* t = pow(x/8, 4)  */
		return (-log(0.5 * x) * GMT_bei(x) - 0.25 * M_PI * GMT_ber(x) + rxsq * (
			6.76454936    + t * (
			-142.91827687 + t * (
			124.23569650  + t * (
			-21.30060904  + t * (
			1.17509064    + t * (
			-0.02695875   + t * (
			0.00029532 * t))))))));
	}
	else {
		/* Russell's asymptotic approximation,
			from Watson, p. 204  */

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

		return ( exp(alpha) * sin(beta) / sqrt(2.0 * x / M_PI) );
	}
}



double GMT_i0 (double x)
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

double GMT_i1 (double x)
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

double GMT_in (int n, double x)
{
	/* Modified Bessel function In(x) */

	int j, m, IACC = 40;
	double res, tox, bip, bi, bim;
	double BIGNO = 1.0e10, BIGNI = 1.0e-10;

	if (n == 0) return (GMT_i0 (x));
	if (n == 1) return (GMT_i1 (x));
	if (x == 0.0) return (0.0);

	tox = 2.0 / fabs (x);
	bip = res = 0.0;
	bi = 1.0;
	m = 2 * (n + irint (sqrt (IACC * n)));
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
	res *= (GMT_i0 (x) / bi);
	if (x < 0.0 && (n%2)) res = -res;

	return (res);
}

double GMT_k0 (double x)
{
/* Modified from code in Press et al. */
	double y, z, res;

	if (x < 0.0) x = -x;

	if (x <= 2.0) {
		y = 0.25 * x * x;
		z = x * x / 14.0625;
		res = (-log(0.5*x) * (1.0 + z * (3.5156229 + z * (3.0899424 + z * (1.2067492 + z * (0.2659732 + z * (0.360768e-1 + z * 0.45813e-2))))))) + (-0.5772156 + y * (0.42278420 + y * (0.23069756 + y * (0.3488590e-1 + y * (0.262698e-2 + y * (0.10750e-3 + y * 0.74e-5))))));
	}
	else {
		y = 2.0 / x;
		res = (exp (-x) / sqrt (x)) * (1.25331414 + y * (-0.7832358e-1 + y * (0.2189568e-1 + y * (-0.1062446e-1 + y * (0.587872e-2 + y * (-0.251540e-2 + y * 0.53208e-3))))));
	}
	return (res);
}

double GMT_k1 (double x)
{
	/* Modified Bessel function K1(x) */

	double y, res;

	if (x < 0.0) x = -x;
	if (x <= 2.0) {
		y = x * x / 4.0;
		res = (log (0.5 * x) * GMT_i1 (x)) + (1.0 / x) * (1.0 + y * (0.15443144 + y * (-0.67278579 + y * (-0.18156897 + y * (-0.01919402 + y * (-0.00110404 - y * 0.00004686))))));
	}
	else {
		y = 2.0 / x;
		res = (exp (-x) / sqrt (x)) * (1.25331414 + y * (0.23498619 + y * (-0.03655620 + y * (0.01504268 + y * (-0.00780353 + y * (0.00325614 - y * 0.00068245))))));
	}
	return (res);
}

double GMT_kn (int n, double x)
{
	/* Modified Bessel function Kn(x) */

	int j;
	double bkm, bk, bkp, tox;

	if (n == 0) return (GMT_k0 (x));
	if (n == 1) return (GMT_k1 (x));

	tox = 2.0 / x;
	bkm = GMT_k0 (x);
	bk = GMT_k1 (x);
	for (j = 1; j <= (n-1); j++) {
		bkp = bkm + j * tox * bk;
		bkm = bk;
		bk = bkp;
	}

	return (bk);
}

double GMT_plm (int l, int m, double x)
{
	double fact, pll, pmm, pmmp1, somx2;
	int i, ll;

	/* x is cosine of colatitude and must be -1 <= x <= +1 */
	if (fabs(x) > 1.0) {
		fprintf (stderr, "GMT DOMAIN ERROR:  fabs(x) > 1.0 in GMT_plm(x)\n");
		return (GMT_d_NaN);
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

/* GMT_sinc (x) calculates the sinc function */

double GMT_sinc (double x)
{
	if (x == 0.0) return (1.0);
	x *= M_PI;
	return (sin (x) / x);
}

/* GMT_factorial (n) calculates the factorial n! */

double GMT_factorial (int n)
{
	int i;
	double val = 1.0;
	
	if (n < 0) {
		fprintf (stderr, "GMT DOMAIN ERROR:  n < 0 in GMT_factorial(n)\n");
		return (GMT_d_NaN);
		/* This could be set to return 0 without warning, to facilitate
			sums over binomial coefficients, if desired.  -whfs  */
	}
		
	for (i = 1; i <= n; i++) val *= ((double)i);
	return (val);
}

double GMT_dilog (double x)
{
	/* Compute dilog(x) (defined for x >= 0) by the method of Parker's
	   Appendix A of his Geophysical Inverse Theory.  The function
	   is needed for x in the range 0 <= x <= 1 when solving the
	   spherical spline interpolation in section 2.07 of Parker.

	   I tested this for x in range 0 to 1 by reproducting Figure
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

	if (x < -GMT_CONV_LIMIT) return (GMT_d_NaN);	/* Tolerate minor slop before we are outside domain */

	pisqon6 = M_PI * M_PI / 6.0;
	if (x <= 0.0) return (pisqon6);	/* Meaning -GMT_CONV_LIMIT < x <= 0 */

	if (x < 0.5) {
		y = -log (1.0 - x);
		ysq = y * y;
		z = y * (1.0 + y * (-0.25 + y * (0.027777777777213 + 
			ysq * (-2.7777776990e-04 + ysq * (4.724071696e-06 + 
			ysq * (-9.1764954e-08 + 1.798670e-09 * ysq))))));
		return (pisqon6 - z + y * log (x));
	}
	if (x < 2.0) {
		y = -log (x);
		ysq = y * y;
		z = y * (1.0 + y * (-0.25 + y * (0.027777777777213 + 
			ysq * (-2.7777776990e-04 + ysq * (4.724071696e-06 + 
			ysq * (-9.1764954e-08 + 1.798670e-09 * ysq))))));
		return (z);
	}
	y = log (x);
	ysq = y * y;
	z = y * (1.0 + y * (-0.25 + y * (0.027777777777213 + 
		ysq * (-2.7777776990e-04 + ysq * (4.724071696e-06 + 
		ysq * (-9.1764954e-08 + 1.798670e-09 * ysq))))));
	return (-z - 0.5 * ysq);
}

#ifndef M_2_SQRTPI
#define  M_2_SQRTPI      1.12837916709551257390
#endif

double GMT_erfinv (double y)
{
	double x, fy, z;
	
	/*  Misc. efficients for expansion */

	static double a[4] = {0.886226899, -1.645349621,  0.914624893, -0.140543331};
	static double b[4] = {-2.118377725, 1.442710462, -0.329097515, 0.012229801};
	static double c[4] = {-1.970840454, -1.624906493, 3.429567803, 1.641345311};
	static double d[2] = {3.543889200, 1.637067800};

	fy = fabs (y);
	
	if (fy > 1.0) return (GMT_d_NaN);	/* Outside domain */
	
	if (fabs (1.0 - fy) < GMT_CONV_LIMIT) {	/* Close to +- Inf */
		return (copysign (DBL_MAX, y));
	}
	
	if (fy < 0.7) {	/* Central range */
		z = y * y;
		x = y * (((a[3] * z + a[2]) * z + a[1]) * z + a[0]) / ((((b[3] * z + b[2]) * z + b[1]) * z + b[0]) * z + 1.0);
	}
	else if (y > 0.7 && y < 1.0) { /* Near upper range */
		z = sqrt (-log (0.5 * (1.0 - y)));
		x = (((c[3] * z + c[2]) * z + c[1]) * z + c[0]) / ((d[1] * z + d[0]) * z + 1.0);
	}
	else if (y < -0.7 && y > -1.0) {	/* Near lower range */
		z = sqrt (-log (0.5 * (1.0 + y)));
		x = -(((c[3] * z + c[2]) * z + c[1]) * z + c[0]) / ((d[1] * z + d[0]) * z + 1.0);
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
	int i, sign = 1;
	double x, res, xsq, xnum, xden, xi;

	x = y;
	if (x < 0.0) {
		sign = -1;
		x = -x;
	}
	if (x < 1.0e-10) {
		res = x * p[2] / q[2];
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
	int i, sign = 1;
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
char	*GMT_strdup (const char *s) {
	int n;
	char *p;
	
	n = strlen (s) + 1;
	p = (char *)malloc ((size_t)n);
	strncpy (p, s, n);
	return (p);
}
#endif

#if HAVE_STRTOD == 0
double	GMT_strtod (const char *s, char **ends) {

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
	
	char	*t, savechar;
	double	x = 0.0;
	int	i, nsign[2], nradix[2], nexp, ndigits, error;
	BOOLEAN inside = FALSE;
	
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

int     GMT_f_q (double chisq1, int nu1, double chisq2, int nu2, double *prob)
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
		double	t, p, x, y, theta, term, sum, c, s, csq, ssq;
		int	kt, kb, k, kstop;
		
	
	/* Check range of arguments:  */

	if (nu1 <= 0 || nu2 <= 0 || chisq1 < 0.0 || chisq2 < 0.0) {
		fprintf (stderr, "GMT_f_q:  Bad argument(s).\n");
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
	
	if (GMT_inc_beta(0.5*nu2, 0.5*nu1, chisq2/(chisq2+chisq1), prob) ) {
		fprintf(stderr,"GMT_q_p:  Trouble in GMT_inc_beta call.\n");
		return(-1);
	}
	return(0);	/* REVISION Oct 27, 2000:  All code below here is not used.  */

	/* If nu1 == 1, sqrt(F) is distributed as Student's t;
		if nu2 == 1, use reflexive property to swap 1 and 2.  */
	
	if (nu1 == 1) {
		t = sqrt(nu2 * chisq1 / chisq2);
		if (GMT_student_t_a(t, nu2, &p) ) {
			fprintf (stderr, "GMT_f_q:  Error calling GMT_student_t_a()\n");
			return (-1);
		}
		*prob = 1.0 - p;
	}
	else if (nu2 == 1) {
		t = sqrt(nu1 * chisq2 / chisq1);
		if (GMT_student_t_a(t, nu1, &p) ) {
			fprintf (stderr, "GMT_f_q:  Error calling GMT_student_t_a()\n");
			return (-1);
		}
		*prob = p;
	}
	
	/* if nu1 == nu2, this reduces to a case of Student's t by using
		A & S 26.5.14 and swapping chisq1, chisq2 if needed, using
		the reflexive property:  */
	
	else if (nu1 == nu2) {
		t = 0.5 * fabs(chisq1 - chisq2) * sqrt(nu1/(chisq1*chisq2));
		GMT_student_t_a(t, nu1, &p);
		if (chisq1 >= chisq2) {
			*prob = 0.5 * (1.0 - p);
		}
		else {
			*prob = 0.5 * (1.0 + p);
		}
	}

	/* General cases  */
	
	else if (nu1%2 == 1) {
		if (nu2%2 == 1) {
			/* Both odd.  Use Abramowitz and Stegun 26.6.8  */
			theta = atan(sqrt(chisq1/chisq2));
			sincos (theta, &s, &c);
			csq = c * c;
			ssq = s * s;

			term = 1.0;
			sum = 1.0;
			kt = nu2 - 1;
			kb = 1;
			k = 0;	/* k is the power on sin in the sum  */
			while (k < nu1 - 3) {
				k += 2;
				kt += 2;
				kb += 2;
				term *= (kt * ssq)/kb;
				sum += term;
			}

			term = 2.0 * s * c / M_PI;
			kt = 0;
			kb = -1;
			k = 1;
			while (k < nu2) {
				k += 2;
				kt += 2;
				kb += 2;
				term *= (kt * csq)/kb;
			}

			t = sqrt(nu2 * chisq1 / chisq2);
			if (GMT_student_t_a(t, nu2, &p) ) {
				fprintf (stderr, "GMT_f_q:  Error calling GMT_student_t_a()\n");
				return (-1);
			}
			
			*prob = 1.0 - p + term * sum;
		}
		else {

			/* Get here when nu2 is even and both are greater than 1.
				Use Abramowitz and Stegun 26.6.5  */
		
			x = chisq2 / (chisq1 + chisq2);
			y = chisq1 / (chisq1 + chisq2);		/* y = 1 - x  */
			kstop = (nu2 - 2)/2;
			k = 0;
			term = 1.0;
			sum = 1.0;
			kt = nu1 - 2;
			kb = 0;
			while (k < kstop) {
				k++;
				kt += 2;
				kb += 2;
				term *= (kt * x)/kb;
				sum += term;
			}
			
			term = sqrt(y);
			k = 0;
			kstop = (nu1 - 1)/2;
			while (k < kstop) {
				k++;
				term *= y;
			}
			
			*prob = 1.0 - term * sum;
		}
	}
	else {
		/* nu1 is even.  Use Abramowitz & Stegun 26.6.4  */
			
		x = chisq2 / (chisq1 + chisq2);
		y = chisq1 / (chisq1 + chisq2);		/* y = 1 - x  */
		kstop = (nu1 - 2)/2;
		k = 0;
		term = 1.0;
		sum = 1.0;
		kt = nu2 - 2;
		kb = 0;
		while (k < kstop) {
			k++;
			kt += 2;
			kb += 2;
			term *= (kt * y)/kb;
			sum += term;
		}
		
		if (nu2%2) {
			kstop = (nu2 - 1)/2;
			term = sqrt(x);
		}
		else {
			kstop = nu2/2;
			term = 1.0;
		}
		k = 0;
		while (k < kstop) {
			k++;
			term *= x;
		}
		
		*prob = term * sum;
	}
	
	/* Get here when prob has been assigned.  Adjust range
		in case it went slightly out due to roundoff:  */
	
	if (*prob < 0.0) *prob = 0.0;
	if (*prob > 1.0) *prob = 1.0;
	return (0);
}

		
int	GMT_student_t_a(double t, int n, double *prob)
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
	
	This function sets *prob = GMT_d_NaN and returns (-1)
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
	int	k, kstop, kt, kb;
	
	if (t < 0.0 || n <= 0) {
		fprintf (stderr, "GMT_student_t_a:  Bad argument(s).\n");
		*prob = GMT_d_NaN;
		return (-1);
	}
	
	if (t == 0.0) {
		*prob = 0.0;
		return (0);
	}

	theta = atan(t/sqrt((double)n));
	
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
	
	if (n%2 == 1) {
		*prob = 2.0 * (theta + sum) / M_PI;
	}
	else {
		*prob = sum;
	}
	
	/* Adjust in case of roundoff:  */
	
	if (*prob < 0.0) *prob = 0.0;
	if (*prob > 1.0) *prob = 1.0;
	
	return (0);
}

double GMT_zcrit (double alpha)
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
	
	return (sign * M_SQRT2 * GMT_erfinv (1.0 - alpha));
}

double GMT_tcrit (double alpha, double nu)
{
	/* Critical values for Student t-distribution */

	int NU;
	BOOLEAN done = FALSE;
	double t_low, t_high, t_mid, p_high, p_mid, p, sign;
	
	if (alpha > 0.5) {	/* right tail */
		p = 1 - (1 - alpha) * 2.0;
		sign = 1.0;
	}
	else {
		p = 1 - alpha * 2.0;
		sign = -1.0;
	}
	t_low = GMT_zcrit (alpha);
	t_high = 5.0;
	NU = irint(nu);
	GMT_student_t_a (t_high, NU, &p_high);
	while (p_high < p) {	/* Must pick higher starting point */
		t_high *= 2.0;
		GMT_student_t_a (t_high, NU, &p_high);
	}
	
	/* Now, (t_low, p_low) and (t_high, p_high) are bracketing the desired (t,p) */
	
	while (!done) {
		t_mid = 0.5 * (t_low + t_high);
		GMT_student_t_a (t_mid, NU, &p_mid);
		if (fabs (p_mid - p) < GMT_CONV_LIMIT) {
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

double GMT_chi2crit (double alpha, double nu)
{
	/* Critical values for Chi^2-distribution */

	BOOLEAN done = FALSE;
	double chi2_low, chi2_high, chi2_mid, p_high, p_mid, p;
	
	p = 1.0 - alpha;
	chi2_low = 0.0;
	chi2_high = 5.0;
	GMT_chi2 (chi2_high, nu, &p_high);
	while (p_high > p) {	/* Must pick higher starting point */
		chi2_high *= 2.0;
		GMT_chi2 (chi2_high, nu, &p_high);
	}
	
	/* Now, (chi2_low, p_low) and (chi2_high, p_high) are bracketing the desired (chi2,p) */
	
	while (!done) {
		chi2_mid = 0.5 * (chi2_low + chi2_high);
		GMT_chi2 (chi2_mid, nu, &p_mid);
		if (fabs (p_mid - p) < GMT_CONV_LIMIT) {
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

double GMT_Fcrit (double alpha, double nu1, double nu2)
{
	/* Critical values for F-distribution */

	int NU1, NU2;
	BOOLEAN done = FALSE;
	double F_low, F_high, F_mid, p_high, p_mid, p, chisq1, chisq2;
	void F_to_ch1_ch2 (double F, double nu1, double nu2, double *chisq1, double *chisq2);
	
	F_high = 5.0;
	p = 1.0 - alpha;
	F_low = 0.0;
	F_to_ch1_ch2 (F_high, nu1, nu2, &chisq1, &chisq2);
	NU1 = irint (nu1);
	NU2 = irint (nu2);
	GMT_f_q (chisq1, NU1, chisq2, NU2, &p_high);
	while (p_high > p) {	/* Must pick higher starting point */
		F_high *= 2.0;
		F_to_ch1_ch2 (F_high, nu1, nu2, &chisq1, &chisq2);
		GMT_f_q (chisq1, NU1, chisq2, NU2, &p_high);
	}
	
	/* Now, (F_low, p_low) and (F_high, p_high) are bracketing the desired (F,p) */
	
	while (!done) {
		F_mid = 0.5 * (F_low + F_high);
		F_to_ch1_ch2 (F_mid, nu1, nu2, &chisq1, &chisq2);
		GMT_f_q (chisq1, NU1, chisq2, NU2, &p_mid);
		if (fabs (p_mid - p) < GMT_CONV_LIMIT) {
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

void F_to_ch1_ch2 (double F, double nu1, double nu2, double *chisq1, double *chisq2)
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
	if (ax <= GMT_CONV_LIMIT) return (0.0);
	
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
		m = 2 * ((n + (int) d_sqrt(ACC * n)) / 2);
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
	
fprintf (stderr, "GMT_y0 called\n");
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
#define GMT_RAND_IM 2147483647
#define GMT_RAND_AM (1.0 / GMT_RAND_IM)
#define GMT_RAND_IR 2836
#define GMT_RAND_NTAB 32
#define GMT_RAND_NDIV (1 + (GMT_RAND_IM - 1) / GMT_RAND_NTAB)
#define GMT_RAND_EPS 2.2204460492503131E-16

double GMT_rand ()
{
	/* Uniform random number generator based on ran1 of
	 * Press et al, Numerical Recipes, 2nd edition,
	 * converted from Fortran to C.  Will return values
	 * x so that 0.0 < x < 1.0 occurs with equal probability.
	 */
	
	static int GMT_rand_iy = 0;
	static int GMT_rand_seed = 0;
	static int GMT_rand_iv[GMT_RAND_NTAB];
	
	int j, k;
	
	if (GMT_rand_iy == 0) {	/* First time initialization */
		GMT_rand_seed = (int) time (NULL);		/* Seed value is positive sec since 1970 */
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

double GMT_nrand (void) {
	/* Gaussian random number generator based on gasdev of
	 * Press et al, Numerical Recipes, 2nd edition.  Will
	 * return values that zero mean and unit variance.
	 */
	 
	static int iset = 0;
	static double gset;
	double fac, r, v1, v2;
	double GMT_rand ();

	if (iset == 0) {	/* We don't have an extra deviate handy, so */
		do {
			v1 = 2.0 * GMT_rand () - 1.0;	/* Pick two uniform numbers in the -1/1/-1/1 square */
			v2 = 2.0 * GMT_rand () - 1.0;
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

void GMT_chi2 (double chi2, double nu, double *prob) {
	/* Evaluate probability that chi2 will exceed the
	 * theoretical chi2 by chance. */
 
 	*prob = GMT_gammq (0.5 * nu, 0.5 * chi2);
}

void GMT_cumpoission (double k, double mu, double *prob) {
	/* evaluate Cumulative Poisson Distribution */
	
	*prob = GMT_gammq (k, mu);
}

#if HAVE_HYPOT == 0

double	GMT_hypot (double x, double y) {
/* 	Return sqrt(x*x + y*y), guarding against
	some overflows where possible.  If a local
	library hypot() is available, it should be
	used instead; it could be faster and more
	accurate.  WHFS 30 March 2000  */
	
	double	a, b, c, d, r, s, t;

	if (GMT_is_dnan (x) || GMT_is_dnan (y) ) return (GMT_d_NaN);
	
	/* A complete implementation of IEEE exceptional values
	would also return +Inf if either x or y is +/- Inf  */
	
	
	if (x == 0.0) return (fabs(y));
	if (y == 0.0) return (fabs(x));
	
	a = fabs(x);
	b = fabs(y);
	
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

double	GMT_log1p (double x) {
/* 	Approximate log(1 + x) fairly well for small x.
	This should be used only if there isn't a better
	library routine available.  WHFS 30 March 2000  */

	double 	u;
	
	if (GMT_is_dnan(x) || x <= -1.0) return (GMT_d_NaN);
	
	u = 1.0 + x;
	
	if (u == 1.0) 
		return (x);
	else
		return ( log(u) * (x/(u - 1.0) ) );
}

#endif

#if HAVE_ATANH == 0

double	GMT_atanh (double x) {
/* 	Return hyperbolic arctangent of x.
	This should be used only if there isn't a better
	library routine available.  WHFS 30 March 2000  */
	
	if (GMT_is_dnan(x) || fabs(x) >= 1.0) return (GMT_d_NaN);
	
	return (0.5 * ( log1p(x) - log1p(-x) ) );
}

#endif

int GMT_median (double *x, int n, double xmin, double xmax, double m_initial, double *med)
{
	double	lower_bound, upper_bound, m_guess, t_0, t_1, t_middle;
	double	lub, glb, xx, temp;
	int	i, n_above, n_below, n_equal, n_lub, n_glb;
	int	finished = FALSE;
	int	iteration = 0;
	
	if (n == 0) {
		*med = m_initial;
		return (1);
	}
	if (n == 1) {
		*med = x[0];
		return (1);
	}
	if (n == 2) {
		*med = 0.5 * (x[0] + x[1]);
		return (1);
	}
	
	m_guess = m_initial;
	lower_bound = xmin;
	upper_bound = xmax;
	t_0 = 0;
	t_1 = n - 1;
	t_middle = 0.5 * (n - 1);
	
	do {

		n_above = n_below = n_equal = n_lub = n_glb = 0;
		lub = xmax;
		glb = xmin;
		
		for (i = 0; i < n; i++) {
		
			xx = x[i];
			if (xx == m_guess) {
				n_equal++;
			}
			else if (xx > m_guess) {
				n_above++;
				if (xx < lub) {
					lub = xx;
					n_lub = 1;
				}
				else if (xx == lub) {
					n_lub++;
				}
			}
			else {
				n_below++;
				if (xx > glb) {
					glb = xx;
					n_glb = 1;
				}
				else if (xx == glb) {
					n_glb++;
				}
			}
		}
		
		iteration++;

		/* Now test counts, watch multiple roots, think even/odd:  */
		
		if ( (abs(n_above - n_below)) <= n_equal) {
			
			if (n_equal) {
				*med = m_guess;
			}
			else {
				*med = 0.5 * (lub + glb);
			}
			finished = TRUE;
		}
		else if ( (abs( (n_above - n_lub) - (n_below + n_equal) ) ) < n_lub) {
		
			*med = lub;
			finished = TRUE;
		}
		else if ( (abs( (n_below - n_glb) - (n_above + n_equal) ) ) < n_glb) {
		
			*med = glb;
			finished = TRUE;
		}
		/* Those cases found the median; the next two will forecast a new guess:  */
		
		else if ( n_above > (n_below + n_equal) ) {  /* Guess is too low  */
		
			lower_bound = m_guess;
			t_0 = n_below + n_equal - 1;
			temp = lower_bound + (upper_bound - lower_bound) * (t_middle - t_0) / (t_1 - t_0);
			m_guess = (temp > lub) ? temp : lub;	/* Move guess at least to lub  */
		}
		else if ( n_below > (n_above + n_equal) ) {  /* Guess is too high  */
		
			upper_bound = m_guess;
			t_1 = n_below + n_equal - 1;
			temp = lower_bound + (upper_bound - lower_bound) * (t_middle - t_0) / (t_1 - t_0);
			m_guess = (temp < glb) ? temp : glb;	/* Move guess at least to glb  */
		}
		else {	/* If we get here, I made a mistake!  */
			fprintf (stderr,"%s: GMT Fatal Error: Internal goof - please report to developers!\n", GMT_program);
			exit (EXIT_FAILURE);
		}
			
	} while (!finished);
	
	/* That's all, folks!  */
	return (iteration);
}

int GMT_median_f (float *x, int n, double xmin, double xmax, double m_initial, double *med)
{
	double	lower_bound, upper_bound, m_guess, t_0, t_1, t_middle;
	double	lub, glb, xx, temp;
	int	i, n_above, n_below, n_equal, n_lub, n_glb;
	int	finished = FALSE;
	int	iteration = 0;
	
	if (n == 0) {
		*med = m_initial;
		return (1);
	}
	if (n == 1) {
		*med = x[0];
		return (1);
	}
	if (n == 2) {
		*med = 0.5 * (x[0] + x[1]);
		return (1);
	}
	
	m_guess = m_initial;
	lower_bound = xmin;
	upper_bound = xmax;
	t_0 = 0;
	t_1 = n - 1;
	t_middle = 0.5 * (n - 1);
	
	do {

		n_above = n_below = n_equal = n_lub = n_glb = 0;
		lub = xmax;
		glb = xmin;
		
		for (i = 0; i < n; i++) {
		
			xx = x[i];
			if (xx == m_guess) {
				n_equal++;
			}
			else if (xx > m_guess) {
				n_above++;
				if (xx < lub) {
					lub = xx;
					n_lub = 1;
				}
				else if (xx == lub) {
					n_lub++;
				}
			}
			else {
				n_below++;
				if (xx > glb) {
					glb = xx;
					n_glb = 1;
				}
				else if (xx == glb) {
					n_glb++;
				}
			}
		}
		
		iteration++;

		/* Now test counts, watch multiple roots, think even/odd:  */
		
		if ( (abs(n_above - n_below)) <= n_equal) {
			
			if (n_equal) {
				*med = m_guess;
			}
			else {
				*med = 0.5 * (lub + glb);
			}
			finished = TRUE;
		}
		else if ( (abs( (n_above - n_lub) - (n_below + n_equal) ) ) < n_lub) {
		
			*med = lub;
			finished = TRUE;
		}
		else if ( (abs( (n_below - n_glb) - (n_above + n_equal) ) ) < n_glb) {
		
			*med = glb;
			finished = TRUE;
		}
		/* Those cases found the median; the next two will forecast a new guess:  */
		
		else if ( n_above > (n_below + n_equal) ) {  /* Guess is too low  */
		
			lower_bound = m_guess;
			t_0 = n_below + n_equal - 1;
			temp = lower_bound + (upper_bound - lower_bound) * (t_middle - t_0) / (t_1 - t_0);
			m_guess = (temp > lub) ? temp : lub;	/* Move guess at least to lub  */
		}
		else if ( n_below > (n_above + n_equal) ) {  /* Guess is too high  */
		
			upper_bound = m_guess;
			t_1 = n_below + n_equal - 1;
			temp = lower_bound + (upper_bound - lower_bound) * (t_middle - t_0) / (t_1 - t_0);
			m_guess = (temp < glb) ? temp : glb;	/* Move guess at least to glb  */
		}
		else {	/* If we get here, I made a mistake!  */
			fprintf (stderr,"%s: GMT Fatal Error: Internal goof - please report to developers!\n", GMT_program);
			exit (EXIT_FAILURE);
		}
			
	} while (!finished);
	
	/* That's all, folks!  */
	return (iteration);
}

int GMT_mode (double *x, int n, int j, int sort, double *mode_est)
{
	int	i, istop, multiplicity = 0;
	double	mid_point_sum = 0.0, length, short_length = 1.0e+30;

	if (n == 0) return (0);
	if (n == 1) {
		*mode_est = x[0];
		return (0);
	}
	
	if (sort) qsort((void *)x, (size_t)n, sizeof(double), GMT_comp_double_asc);

	istop = n - j;
	
	for (i = 0; i < istop; i++) {
		length = x[i + j] - x[i];
		if (length < 0.0) {
			fprintf(stderr,"GMT_mode: Array not sorted in non-decreasing order.\n");
			return (-1);
		}
		else if (length == short_length) {
			multiplicity++;
			mid_point_sum += (0.5 * (x[i + j] + x[i]));
		}
		else if (length < short_length) {
			multiplicity = 1;
			mid_point_sum = (0.5 * (x[i + j] + x[i]));
			short_length = length;
		}
	}

	if (multiplicity - 1) mid_point_sum /= multiplicity;
	
	*mode_est = mid_point_sum;
	return (0);
}

int GMT_mode_f (float *x, int n, int j, int sort, double *mode_est)
{
	int	i, istop, multiplicity = 0;
	double	mid_point_sum = 0.0, length, short_length = 1.0e+30;

	if (n == 0) return (0);
	if (n == 1) {
		*mode_est = x[0];
		return (0);
	}
	if (sort) qsort((void *)x, (size_t)n, sizeof(float), GMT_comp_float_asc);

	istop = n - j;
	
	for (i = 0; i < istop; i++) {
		length = x[i + j] - x[i];
		if (length < 0.0) {
			fprintf (stderr,"GMT_mode: Array not sorted in non-decreasing order.\n");
			return (-1);
		}
		else if (length == short_length) {
			multiplicity++;
			mid_point_sum += (0.5 * (x[i + j] + x[i]));
		}
		else if (length < short_length) {
			multiplicity = 1;
			mid_point_sum = (0.5 * (x[i + j] + x[i]));
			short_length = length;
		}
	}

	if (multiplicity - 1) mid_point_sum /= multiplicity;
	
	*mode_est = mid_point_sum;
	return (0);
}


void GMT_getmad (double *x, int n, double location, double *scale)
{
	/* Compute MAD (Median Absolute Deviation) for a double data set */
	
	double	e_low, e_high, error, last_error;
	int	i_low, i_high, n_dev, n_dev_stop;
	
	i_low = 0;
	while (i_low < n && x[i_low] <= location) i_low++;
	i_low--;
	
	i_high = n - 1;
	while (i_high >= 0 && x[i_high] >= location) i_high--;
	i_high++;

	n_dev_stop = n / 2;
	error = 0.0;
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

void GMT_getmad_f (float *x, int n, double location, double *scale)
{
	/* Compute MAD (Median Absolute Deviation) for a float data set */
	
	double	e_low, e_high, error, last_error;
	int	i_low, i_high, n_dev, n_dev_stop;
	
	i_low = 0;
	while (i_low < n && x[i_low] <= location) i_low++;
	i_low--;
	
	i_high = n - 1;
	while (i_high >= 0 && x[i_high] >= location) i_high--;
	i_high++;

	n_dev_stop = n / 2;
	error = 0.0;
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

double GMT_extreme (double x[], int n, double x_default, int kind, int way)
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
	
	int i, k;
	double x_select;
	
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
