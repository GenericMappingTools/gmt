/*--------------------------------------------------------------------
 *	$Id: gmt_stat.h,v 1.4 2007-07-07 03:29:11 guru Exp $
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

#endif /* _GMT_STAT_H */
