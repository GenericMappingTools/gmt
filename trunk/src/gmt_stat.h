/*--------------------------------------------------------------------
 *	$Id: gmt_stat.h,v 1.7 2007-09-28 17:37:06 remko Exp $
 *
 *	Copyright (c) 1991-2007 by P. Wessel and W. H. F. Smith
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

#ifndef _GMT_STAT_H
#define _GMT_STAT_H

/* These functions used by grdmath and gmtmath are declared in gmt_stat.c.
 * There are no ANSI-C equivalents so these prototypes are always set here */

EXTERN_MSC double GMT_bei(double x);
EXTERN_MSC double GMT_ber(double x);
EXTERN_MSC double GMT_kei(double x);
EXTERN_MSC double GMT_ker(double x);
EXTERN_MSC double GMT_plm(int l, int m, double x);
EXTERN_MSC double GMT_plm_bar(int l, int m, double x, BOOLEAN ortho);
EXTERN_MSC double GMT_factorial(int n);
EXTERN_MSC double GMT_i0(double x);
EXTERN_MSC double GMT_i1(double x);
EXTERN_MSC double GMT_in(int n, double x);
EXTERN_MSC double GMT_k0(double x);
EXTERN_MSC double GMT_k1(double x);
EXTERN_MSC double GMT_kn(int n, double x);
EXTERN_MSC double GMT_dilog(double x);
EXTERN_MSC double GMT_sinc(double x);
EXTERN_MSC double GMT_erfinv(double x);
EXTERN_MSC double GMT_rand(void);
EXTERN_MSC double GMT_nrand(void);
EXTERN_MSC double GMT_lrand(void);
EXTERN_MSC int GMT_chebyshev(double x, int n, double *t);
EXTERN_MSC double GMT_corrcoeff (double *x, double *y, size_t n, int mode);
EXTERN_MSC double GMT_corrcoeff_f (float *x, float *y, size_t n, int mode);
EXTERN_MSC double GMT_Fcrit (double alpha, double nu1, double nu2);
EXTERN_MSC double GMT_chi2crit (double alpha, double nu);
EXTERN_MSC double GMT_extreme (double x[], size_t n, double x_default, int kind, int way);
EXTERN_MSC double GMT_tcrit (double alpha, double nu);
EXTERN_MSC double GMT_zcrit (double alpha);
EXTERN_MSC double GMT_zdist (double x);
EXTERN_MSC int GMT_f_q (double chisq1, int nu1, double chisq2, int nu2, double *prob);
EXTERN_MSC int GMT_median (double *x, size_t n, double xmin, double xmax, double m_initial, double *med);
EXTERN_MSC int GMT_mode (double *x, size_t n, size_t j, int sort, int mode_selection, int *n_multiples, double *mode_est);
EXTERN_MSC int GMT_mode_f (float *x, size_t n, size_t j, int sort, int mode_selection, int *n_multiples, double *mode_est);
EXTERN_MSC int GMT_sig_f (double chi1, int n1, double chi2, int n2, double level, double *prob);
EXTERN_MSC int GMT_student_t_a (double t, int n, double *prob);
EXTERN_MSC void GMT_chi2 (double chi2, double nu, double *prob);
EXTERN_MSC void GMT_cumpoisson (double k, double mu, double *prob);
EXTERN_MSC void GMT_getmad (double *x, size_t n, double location, double *scale);
EXTERN_MSC void GMT_getmad_f (float *x, size_t n, double location, double *scale);
EXTERN_MSC double GMT_psi (double z[], double p[]);
EXTERN_MSC void GMT_PvQv (double x, double v_ri[], double pq[], int *iter);
EXTERN_MSC double GMT_quantile (double *x, double q, size_t n);
EXTERN_MSC double GMT_quantile_f (float *x, double q, size_t n);

#endif /* _GMT_STAT_H */
