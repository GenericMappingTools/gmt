/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

#ifndef _GMT_STAT_H
#define _GMT_STAT_H

/* for weighted mean/mode */
struct OBSERVATION {
	float value;
	float weight;
};

EXTERN_MSC double GMT_bei (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_ber (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_kei (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_ker (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_plm (struct GMT_CTRL *C, GMT_LONG l, GMT_LONG m, double x);
EXTERN_MSC double GMT_plm_bar (struct GMT_CTRL *C, GMT_LONG l, GMT_LONG m, double x, GMT_BOOLEAN ortho);
EXTERN_MSC double GMT_factorial (struct GMT_CTRL *C, GMT_LONG n);
EXTERN_MSC double GMT_i0 (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_i1 (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_in (struct GMT_CTRL *C, COUNTER_MEDIUM n, double x);
EXTERN_MSC double GMT_k0 (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_k1 (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_kn (struct GMT_CTRL *C, COUNTER_MEDIUM n, double x);
EXTERN_MSC double GMT_dilog (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_sinc (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_erfinv (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_rand (struct GMT_CTRL *C);
EXTERN_MSC double GMT_nrand (struct GMT_CTRL *C);
EXTERN_MSC double GMT_lrand (struct GMT_CTRL *C);
EXTERN_MSC GMT_LONG GMT_chebyshev (struct GMT_CTRL *C, double x, GMT_LONG n, double *t);
EXTERN_MSC double GMT_corrcoeff (struct GMT_CTRL *C, double *x, double *y, COUNTER_LARGE n, COUNTER_MEDIUM mode);
EXTERN_MSC double GMT_corrcoeff_f (struct GMT_CTRL *C, float *x, float *y, COUNTER_LARGE n, COUNTER_MEDIUM mode);
EXTERN_MSC double GMT_Fcrit (struct GMT_CTRL *C, double alpha, double nu1, double nu2);
EXTERN_MSC double GMT_chi2crit (struct GMT_CTRL *C, double alpha, double nu);
EXTERN_MSC double GMT_extreme (struct GMT_CTRL *C, double *x, COUNTER_LARGE n, double x_default, GMT_LONG kind, GMT_LONG way);
EXTERN_MSC double GMT_tcrit (struct GMT_CTRL *C, double alpha, double nu);
EXTERN_MSC double GMT_zcrit (struct GMT_CTRL *C, double alpha);
EXTERN_MSC double GMT_zdist (struct GMT_CTRL *C, double x);
EXTERN_MSC GMT_LONG GMT_f_q (struct GMT_CTRL *C, double chisq1, GMT_LONG nu1, double chisq2, GMT_LONG nu2, double *prob);
EXTERN_MSC GMT_LONG GMT_median (struct GMT_CTRL *C, double *x, COUNTER_LARGE n, double xmin, double xmax, double m_initial, double *med);
EXTERN_MSC GMT_LONG GMT_mode (struct GMT_CTRL *C, double *x, COUNTER_LARGE n, COUNTER_LARGE j, GMT_BOOLEAN sort, COUNTER_MEDIUM mode_selection, COUNTER_MEDIUM *n_multiples, double *mode_est);
EXTERN_MSC GMT_LONG GMT_mode_f (struct GMT_CTRL *C, float *x, COUNTER_LARGE n, COUNTER_LARGE j, GMT_BOOLEAN sort, COUNTER_MEDIUM mode_selection, COUNTER_MEDIUM *n_multiples, double *mode_est);
EXTERN_MSC double GMT_mean_and_std (struct GMT_CTRL *C, double *x, COUNTER_LARGE n, double *std);

EXTERN_MSC double GMT_median_weighted (struct GMT_CTRL *C, struct OBSERVATION *data, COUNTER_LARGE n, double quantile);
EXTERN_MSC double GMT_mode_weighted (struct GMT_CTRL *C, struct OBSERVATION *data, COUNTER_LARGE n);

EXTERN_MSC GMT_LONG GMT_sig_f (struct GMT_CTRL *C, double chi1, GMT_LONG n1, double chi2, GMT_LONG n2, double level, double *prob);
EXTERN_MSC GMT_LONG GMT_student_t_a (struct GMT_CTRL *C, double t, GMT_LONG n, double *prob);
EXTERN_MSC void GMT_chi2 (struct GMT_CTRL *C, double chi2, double nu, double *prob);
EXTERN_MSC void GMT_cumpoisson (struct GMT_CTRL *C, double k, double mu, double *prob);
EXTERN_MSC void GMT_getmad (struct GMT_CTRL *C, double *x, COUNTER_LARGE n, double location, double *scale);
EXTERN_MSC void GMT_getmad_f (struct GMT_CTRL *C, float *x, COUNTER_LARGE n, double location, double *scale);
EXTERN_MSC double GMT_psi (struct GMT_CTRL *C, double z[], double p[]);
EXTERN_MSC void GMT_PvQv (struct GMT_CTRL *C, double x, double v_ri[], double pq[], COUNTER_MEDIUM *iter);
EXTERN_MSC double GMT_quantile (struct GMT_CTRL *C, double *x, double q, COUNTER_LARGE n);
EXTERN_MSC double GMT_quantile_f (struct GMT_CTRL *C, float *x, double q, COUNTER_LARGE n);

#endif /* _GMT_STAT_H */
