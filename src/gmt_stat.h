/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

EXTERN_MSC double GMT_bei (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_ber (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_kei (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_ker (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_plm (struct GMT_CTRL *GMT, int l, int m, double x);
EXTERN_MSC double GMT_plm_bar (struct GMT_CTRL *GMT, int l, int m, double x, bool ortho);
EXTERN_MSC void GMT_plm_bar_all (struct GMT_CTRL *GMT, int lmax, double x, bool ortho, double *plm);
EXTERN_MSC double GMT_factorial (struct GMT_CTRL *GMT, int n);
EXTERN_MSC double GMT_i0 (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_i1 (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_in (struct GMT_CTRL *GMT, unsigned int n, double x);
EXTERN_MSC double GMT_k0 (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_k1 (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_kn (struct GMT_CTRL *GMT, unsigned int n, double x);
EXTERN_MSC double GMT_dilog (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_sinc (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_erfinv (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_rand (struct GMT_CTRL *GMT);
EXTERN_MSC double GMT_nrand (struct GMT_CTRL *GMT);
EXTERN_MSC double GMT_lrand (struct GMT_CTRL *GMT);
EXTERN_MSC int GMT_chebyshev (struct GMT_CTRL *GMT, double x, int n, double *t);
EXTERN_MSC double GMT_corrcoeff (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, unsigned int mode);
EXTERN_MSC double GMT_corrcoeff_f (struct GMT_CTRL *GMT, float *x, float *y, uint64_t n, unsigned int mode);
EXTERN_MSC double GMT_Fcrit (struct GMT_CTRL *GMT, double alpha, double nu1, double nu2);
EXTERN_MSC double GMT_chi2crit (struct GMT_CTRL *GMT, double alpha, double nu);
EXTERN_MSC double GMT_extreme (struct GMT_CTRL *GMT, double *x, uint64_t n, double x_default, int kind, int way);
EXTERN_MSC double GMT_tcrit (struct GMT_CTRL *GMT, double alpha, double nu);
EXTERN_MSC double GMT_zcrit (struct GMT_CTRL *GMT, double alpha);
EXTERN_MSC double GMT_zdist (struct GMT_CTRL *GMT, double x);
EXTERN_MSC int GMT_f_q (struct GMT_CTRL *GMT, double chisq1, uint64_t nu1, double chisq2, uint64_t nu2, double *prob);
EXTERN_MSC int GMT_median (struct GMT_CTRL *GMT, double *x, uint64_t n, double xmin, double xmax, double m_initial, double *med);
EXTERN_MSC int GMT_mode (struct GMT_CTRL *GMT, double *x, uint64_t n, uint64_t j, bool sort, unsigned int mode_selection, unsigned int *n_multiples, double *mode_est);
EXTERN_MSC int GMT_mode_f (struct GMT_CTRL *GMT, float *x, uint64_t n, uint64_t j, bool sort, unsigned int mode_selection, unsigned int *n_multiples, double *mode_est);
EXTERN_MSC double GMT_mean_and_std (struct GMT_CTRL *GMT, double *x, uint64_t n, double *std);

EXTERN_MSC double GMT_median_weighted (struct GMT_CTRL *GMT, struct OBSERVATION *data, uint64_t n, double quantile);
EXTERN_MSC double GMT_mode_weighted (struct GMT_CTRL *GMT, struct OBSERVATION *data, uint64_t n);

EXTERN_MSC int GMT_sig_f (struct GMT_CTRL *GMT, double chi1, uint64_t n1, double chi2, uint64_t n2, double level, double *prob);
EXTERN_MSC int GMT_student_t_a (struct GMT_CTRL *GMT, double t, uint64_t n, double *prob);
EXTERN_MSC void GMT_chi2 (struct GMT_CTRL *GMT, double chi2, double nu, double *prob);
EXTERN_MSC void GMT_cumpoisson (struct GMT_CTRL *GMT, double k, double mu, double *prob);
EXTERN_MSC void GMT_getmad (struct GMT_CTRL *GMT, double *x, uint64_t n, double location, double *scale);
EXTERN_MSC void GMT_getmad_f (struct GMT_CTRL *GMT, float *x, uint64_t n, double location, double *scale);
EXTERN_MSC double GMT_psi (struct GMT_CTRL *GMT, double z[], double p[]);
EXTERN_MSC void GMT_PvQv (struct GMT_CTRL *GMT, double x, double v_ri[], double pq[], unsigned int *iter);
EXTERN_MSC double GMT_quantile (struct GMT_CTRL *GMT, double *x, double q, uint64_t n);
EXTERN_MSC double GMT_quantile_f (struct GMT_CTRL *GMT, float *x, double q, uint64_t n);

#endif /* _GMT_STAT_H */
