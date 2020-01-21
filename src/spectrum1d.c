/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 *
 * Brief synopsis: Compute auto and cross spectra using Welch's method of
 * multiple overlapped windows.  Find 1 standard error bars
 * following expressions in Bendat & Piersol.
 *
 * Author:	W. H. F. Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 * References:	Julius S. Bendat & Allan G. Piersol,
 *    	"Random Data", 2nd revised edition, 566pp.,
 *    	1986, John Wiley & Sons, New York. [ B&P below]
 *
 *    	Peter D. Welch, "The use of Fast Fourier
 *    	Transform for the estimation of power spectra:
 *    	a method based on time averaging over short,
 *    	modified periodograms", IEEE Transactions on
 *    	Audio and Electroacoustics, Vol AU-15, No 2,
 *    	June, 1967.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"spectrum1d"
#define THIS_MODULE_MODERN_NAME	"spectrum1d"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Compute auto- [and cross-] spectra from one [or two] time series"
#define THIS_MODULE_KEYS	"<D{,>D},T-)"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-Vbdefghiqs"

#define SPECTRUM1D_N_OUTPUT_CHOICES 8

struct SPECTRUM1D_CTRL {
	struct SPECT1D_Out {	/* -><file> */
		bool active;
		char *file;
	} Out;
	struct SPECT1D_C {	/* -C[<xycnpago>] */
		bool active;
		char col[SPECTRUM1D_N_OUTPUT_CHOICES];	/* Character codes for desired output in the right order */
	} C;
	struct SPECT1D_D {	/* -D<inc> */
		bool active;
		double inc;
	} D;
	struct SPECT1D_L {	/* -L[m|h] */
		bool active;
		bool debug;
		unsigned int mode;
	} L;
	struct SPECT1D_N {	/* -N[+]<namestem> */
		bool active;
		unsigned int mode;
		char *name;
	} N;
	struct SPECT1D_S {	/* -S<segment_size> */
		bool active;
		unsigned int size;
	} S;
	struct SPECT1D_T {	/* -T */
		bool active;
	} T;
	struct SPECT1D_W {	/* -W */
		bool active;
	} W;
};

struct SPECTRUM1D_INFO {	/* Control structure for spectrum1d */
	unsigned int y_given;
	int n_spec, window, window_2;
	gmt_grdfloat *datac;
	double dt, x_variance, y_variance, d_n_windows, y_pow;
	struct SPEC {
		double xpow;	/* PSD in X(t)  */
		double ypow;	/* PSD in Y(t)  */
		double gain;	/* Amplitude increase X->Y in Optimal Response Function  */
		double phase;	/* Phase of Y w.r.t. X in Optimal Response Function  */
		double coh;	/* (squared) Coherence of Y and X; SNR of Y = coh/(1-coh)  */
		double radmit;	/* Real part of Admittance; used e.g., for gravity/topography  */
	} *spec;
};

GMT_LOCAL void alloc_arrays (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C) {
	C->n_spec = C->window/2;	/* This means we skip zero frequency; data are detrended  */
	C->window_2 = 2 * C->window;		/* This is for complex array stuff  */

	C->spec = gmt_M_memory (GMT, NULL, C->n_spec, struct SPEC);
	C->datac = gmt_M_memory (GMT, NULL, C->window_2, gmt_grdfloat);
}

GMT_LOCAL void detrend_and_hanning (struct SPECTRUM1D_INFO *C, bool leave_trend, unsigned int mode) {
	/* If leave_trend is true we do not remove best-fitting LS trend.  Otherwise
	 * we do, modulated by mode: 0: remove trend, 1: remove mean, 2: remove mid-value.
	 * In all cases we apply the Hanning windowing */
	int i, t;
	double sumx, sumtx, sumy, sumty, sumt2, x_slope, x_mean, y_slope, y_mean;
	double t_factor, h_period, h_scale, hc, hw, tt, x_min, x_max, y_min, y_max;
	sumx = sumtx = sumy = sumty = sumt2 = 0.0;
	x_slope = x_mean = y_slope = y_mean = 0.0;
	x_min = y_min = DBL_MAX; x_max = y_max = -DBL_MAX;
	C->x_variance = C->y_variance = 0.0;
	t_factor = 2.0 / (C->window - 1);
	h_period = M_PI / (double)C->window;	/* For Hanning window  */
	h_scale = sqrt (8.0/3.0);		/* For Hanning window  */

	if (!leave_trend) {
		if (C->y_given) {
			for (i = 0, t = 0; i < C->window_2; i+=2, t++) {
				tt = t * t_factor - 1.0;
				sumt2 += (tt * tt);
				sumx += C->datac[i];
				sumtx += (tt * C->datac[i]);
				sumy += C->datac[i+1];
				sumty += (tt * C->datac[i+1]);
				if (C->datac[i] < x_min) x_min = C->datac[i];
				if (C->datac[i] > x_max) x_max = C->datac[i];
				if (C->datac[i+1] < y_min) y_min = C->datac[i+1];
				if (C->datac[i+1] > y_max) y_max = C->datac[i+1];
			}
			y_slope = (mode) ? 0.0 : sumty / sumt2;
			y_mean = (mode == 2) ? 0.5 * (y_min + y_max) : sumy / C->window;
		}
		else {
			for (i = 0, t = 0; i < C->window_2; i+=2, t++) {
				tt = t * t_factor - 1.0;
				sumt2 += (tt * tt);
				sumx += C->datac[i];
				sumtx += (tt * C->datac[i]);
				if (C->datac[i] < x_min) x_min = C->datac[i];
				if (C->datac[i] > x_max) x_max = C->datac[i];
			}
		}
		x_slope = (mode) ? 0.0 :sumtx / sumt2;
		x_mean = (mode == 2) ? 0.5 * (x_min + x_max) : sumx / C->window;
	}
	if (C->y_given) {
		for (i = 0, t = 0; i < C->window_2; i+=2, t++) {
			hc = cos(t * h_period);
			hw = h_scale * (1.0 - hc * hc);
			tt = t * t_factor - 1.0;
			C->datac[i] -= (gmt_grdfloat)(x_mean + tt * x_slope);
			C->datac[i] *= (gmt_grdfloat)hw;
			C->x_variance += (C->datac[i] * C->datac[i]);
			C->datac[i+1] -= (gmt_grdfloat)(y_mean + tt * y_slope);
			C->datac[i+1] *= (gmt_grdfloat)hw;
			C->y_variance += (C->datac[i+1] * C->datac[i+1]);
		}
		C->x_variance /= C->window;
		C->y_variance /= C->window;
	}
	else {
		for (i = 0, t = 0; i < C->window_2; i+=2, t++) {
			hc = cos(t * h_period);
			hw = h_scale * (1.0 - hc * hc);
			tt = t * t_factor - 1.0;
			C->datac[i] -= (gmt_grdfloat)(x_mean + tt * x_slope);
			C->datac[i] *= (gmt_grdfloat)hw;
			C->x_variance += (C->datac[i] * C->datac[i]);
		}
		C->x_variance /= C->window;
	}
}

GMT_LOCAL int compute_spectra (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C, double *x, double *y, uint64_t n_data, bool leave_trend, unsigned int mode) {
	int n_windows, w, i, t_start, t_stop, t, f;
	double dw, spec_scale, x_varp, y_varp = 1.0, one_on_nw, co_quad;
	double xreal, ximag, yreal, yimag, xpower, ypower, co_spec, quad_spec;
	char format[GMT_BUFSIZ];

	/* Scale factor for spectral estimates should be 1/4 of amount given in
		Bendat & Piersol eqn 11-102 because I compute 2 * fft in my
		one-sided code below.  However, tests show that I need 1/8 of
		their equation to match variances approximately */

	/* This used to read: spec_scale = 0.5 / (C->window_2 * dt);  */
	spec_scale = C->dt / (C->window_2);

	C->d_n_windows = (double)n_data / (double)C->window;

	n_windows = irint (2.0 * C->d_n_windows) - 1;
	one_on_nw = 1.0 / (double)n_windows;
	dw = (n_windows > 1) ? (double)(n_data - C->window) / (double)(n_windows - 1) : 1.0;

	for (w = 0; w < n_windows; w++) {
		t_start = irint (floor (0.5 + w * dw));
		t_stop = t_start + C->window;
		if (C->y_given) {
			for (t = t_start, i = 0; t < t_stop; t++, i+=2) {
				C->datac[i] = (gmt_grdfloat)x[t];
				C->datac[i+1] = (gmt_grdfloat)y[t];
			}
		}
		else {
			for (t = t_start, i = 0; t < t_stop; t++, i+=2) {
				C->datac[i] = (gmt_grdfloat)x[t];
				C->datac[i+1] = 0.0;
			}
		}

		detrend_and_hanning (C, leave_trend, mode);

		if (GMT_FFT_1D (GMT->parent, C->datac, C->window, GMT_FFT_FWD, GMT_FFT_COMPLEX))
			return GMT_RUNTIME_ERROR;

		/* Get one-sided estimates */

		x_varp = spec_scale * (C->datac[0] * C->datac[0]);
		if (C->y_given) {
			y_varp = spec_scale * (C->datac[1] * C->datac[1]);
			for (i = 0, f = 2; i < C->n_spec; i++, f+=2) {
				xreal = (i == C->n_spec - 1) ? C->datac[f] : C->datac[f] + C->datac[C->window_2 - f];
				ximag = (i == C->n_spec - 1) ? 0.0 : C->datac[f+1] - C->datac[C->window_2 - f + 1];
				yreal = (i == C->n_spec - 1) ? C->datac[f+1] : C->datac[f+1] + C->datac[C->window_2 - f + 1];
				yimag = (i == C->n_spec - 1) ? 0.0 : C->datac[C->window_2 - f] - C->datac[f];
				xpower = spec_scale * (xreal * xreal + ximag * ximag);
				ypower = spec_scale * (yreal * yreal + yimag * yimag);
				co_spec = spec_scale * (xreal * yreal + ximag * yimag);
				quad_spec = spec_scale * (ximag * yreal - yimag * xreal);

				x_varp += xpower;
				y_varp += ypower;
				C->spec[i].xpow += xpower;
				C->spec[i].ypow += ypower;
				/* Temporarily store co-spec in gain */
				C->spec[i].gain += co_spec;
				/* Temporarily store quad-spec in phase */
				C->spec[i].phase += quad_spec;
			}
			x_varp *= (C->dt/C->n_spec);
			y_varp *= (C->dt/C->n_spec);
		}
		else {
			for (i = 0, f = 2; i < C->n_spec; i++, f+=2) {
				xreal = C->datac[f] + C->datac[C->window_2 - f];
				ximag = C->datac[f+1] - C->datac[C->window_2 - f + 1];
				xpower = spec_scale * (xreal * xreal + ximag * ximag);
				x_varp += xpower;
				C->spec[i].xpow += xpower;
			}
			x_varp *= (C->dt/C->n_spec);
		}

		if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
			C->y_pow = (C->y_given) ? C->y_variance/y_varp : 0.0;
			GMT_Message (GMT->parent, GMT_TIME_NONE, "Window %d from %d to %d\n", w, t_start, t_stop);
			sprintf(format, "X var: %s  X pow: %s  ratio: %s  Y var: %s  Y pow: %s  ratio: %s\n",
				GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			GMT_Message (GMT->parent, GMT_TIME_NONE, format, C->x_variance, x_varp, (C->x_variance/x_varp), C->y_variance, y_varp, C->y_pow);
		}
	}
	/* Now we can divide by n_windows for the ensemble average.
		The cross spectral stuff needs to be computed */

	if (C->y_given ) {
		for (i = 0; i < C->n_spec; i++) {
			C->spec[i].xpow *= one_on_nw;
			C->spec[i].ypow *= one_on_nw;
			co_spec = C->spec[i].gain * one_on_nw;
			quad_spec = C->spec[i].phase * one_on_nw;
			C->spec[i].phase = d_atan2(quad_spec, co_spec);
			co_quad = co_spec * co_spec + quad_spec * quad_spec;
			C->spec[i].coh = co_quad / (C->spec[i].xpow * C->spec[i].ypow);
			C->spec[i].gain = sqrt(co_quad) / C->spec[i].xpow;
			C->spec[i].radmit = co_spec / C->spec[i].xpow;
		}
	}
	else {
		for (i = 0; i < C->n_spec; i++)  C->spec[i].xpow *= one_on_nw;
	}
	return 0;
}

GMT_LOCAL int write_output_separate (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C, char *col, int n_outputs, int write_wavelength, char *namestem) {
	/* Writes separate hardwired files for each output type.  Does NOT use GMT_Put_* functions */
	int i, j;
	double delta_f, eps_pow, out[3], *f_or_w = NULL;
	char fname[GMT_LEN256] = {""};
	FILE *fpout = NULL;

	delta_f = 1.0 / (C->window * C->dt);
	eps_pow = 1.0 / sqrt(C->d_n_windows);	/* Multiplicative error bars for power spectra  */

	f_or_w = gmt_M_memory (GMT, NULL, C->n_spec, double);
	for (i = 0; i < C->n_spec; i++) f_or_w[i] = (write_wavelength) ? 1.0 / ((i + 1) * delta_f) : (i + 1) * delta_f;

	/* loop through output choices */
	for (j = 0; j < n_outputs; j++) {
		switch (col[j]) {
		case 'x':		/* write x power [ B&P 2nd Ed. eqn. 9.32 ] */
			sprintf (fname, "%s.xpower", namestem);
			if ((fpout = gmt_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				gmt_M_free (GMT, f_or_w);
				return (GMT_ERROR_ON_FOPEN);
			}
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].xpow;
				out[GMT_Z] = eps_pow * C->spec[i].xpow;
				GMT->current.io.output (GMT, fpout, 3, out, NULL);
			}
			gmt_fclose (GMT, fpout);
			break;

		case 'y':		/* Write y power [ B&P 2nd Ed. eqn. 9.32 ] */
			sprintf (fname, "%s.ypower", namestem);
			if ((fpout = gmt_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				gmt_M_free (GMT, f_or_w);
				return (GMT_ERROR_ON_FOPEN);
			}
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].ypow;
				out[GMT_Z] = eps_pow * C->spec[i].ypow;
				GMT->current.io.output (GMT, fpout, 3, out, NULL);
			}
			gmt_fclose (GMT, fpout);
			break;
		case 'c':		/* Write Coherent Output power [ B&P 2nd Ed. eqn. 9.71 ] */
			sprintf (fname, "%s.cpower", namestem);
			if ((fpout = gmt_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				gmt_M_free (GMT, f_or_w);
				return (GMT_ERROR_ON_FOPEN);
			}
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].ypow * C->spec[i].coh;
				out[GMT_Z] = out[GMT_Y] * eps_pow * sqrt ( (2.0 - C->spec[i].coh) / C->spec[i].coh);
				GMT->current.io.output (GMT, fpout, 3, out, NULL);
			}
			gmt_fclose (GMT, fpout);
			break;
		case 'n':		/* Write Noise Output power [ B&P 2nd Ed. eqn. 9.73 & Table 9.6 ] */
			sprintf (fname, "%s.npower", namestem);
			if ((fpout = gmt_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				gmt_M_free (GMT, f_or_w);
				return (GMT_ERROR_ON_FOPEN);
			}
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].ypow * (1.0 - C->spec[i].coh);
				out[GMT_Z] = out[GMT_Y] * eps_pow;
				GMT->current.io.output (GMT, fpout, 3, out, NULL);
			}
			gmt_fclose (GMT, fpout);
			break;
		case 'g':		/* Write Gain spectrum [ B&P 2nd Ed. eqn. 9.90 & Table 9.6 ] */
			sprintf (fname, "%s.gain", namestem);
			if ((fpout = gmt_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				gmt_M_free (GMT, f_or_w);
				return (GMT_ERROR_ON_FOPEN);
			}
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].gain;
				out[GMT_Z] = out[GMT_Y] * eps_pow * sqrt( (1.0 - C->spec[i].coh) / (2.0 * C->spec[i].coh) );
				GMT->current.io.output (GMT, fpout, 3, out, NULL);
			}
			gmt_fclose (GMT, fpout);
			break;
		case 'a':		/* Write Real Admittance spectrum
			We don't know the correct error estimate and it is not
			in Bendat and Piersol, or Priestly, or any standard text.
			Smith needs to derive this, and should make a note to
			check the expression given by Marcia Maia et al in Geophys.
			J. Int, 100, 337-348, 1990, equation 10, page 341.
			Meanwhile we will default to use the expression related to
			that for the gain spectrum:
			*/
			sprintf (fname, "%s.admit", namestem);
			if ((fpout = gmt_fopen (GMT, fname, "w")) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				return (GMT_ERROR_ON_FOPEN);
			}
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].radmit;
				out[GMT_Z] = fabs (eps_pow * sqrt( (1.0 - C->spec[i].coh) / (2.0 * C->spec[i].coh) ) * out[GMT_Y]);
				GMT->current.io.output (GMT, fpout, 3, out, NULL);
			}
			gmt_fclose (GMT, fpout);
			break;
		case 'p':		/* Write Phase spectrum [ B&P 2nd Ed. eqn. 9.91 & Table 9.6 ] */
			sprintf (fname, "%s.phase", namestem);
			if ((fpout = gmt_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				gmt_M_free (GMT, f_or_w);
				return (GMT_ERROR_ON_FOPEN);
			}
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].phase;
				out[GMT_Z] = eps_pow * sqrt( (1.0 - C->spec[i].coh) / (2.0 * C->spec[i].coh) );
				GMT->current.io.output (GMT, fpout, 3, out, NULL);
			}
			gmt_fclose (GMT, fpout);
			break;
		case 'o':		/* Write Coherency spectrum [ B&P 2nd Ed. eqn. 9.82 ] */
			sprintf (fname, "%s.coh", namestem);
			if ((fpout = gmt_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				gmt_M_free (GMT, f_or_w);
				return (GMT_ERROR_ON_FOPEN);
			}
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].coh;
				out[GMT_Z] = out[GMT_Y] * eps_pow * (1.0 - C->spec[i].coh) * sqrt(2.0 / C->spec[i].coh);
				GMT->current.io.output (GMT, fpout, 3, out, NULL);
			}
			gmt_fclose (GMT, fpout);
			break;
		}
	}

	gmt_M_free (GMT, f_or_w);
	return (0);
}

GMT_LOCAL void assign_output_spectrum1d (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C, char *col, int n_outputs, int write_wavelength, double *out[]) {
	/* Fills out the 2-D table array given */
	int i, j, k;
	double delta_f, eps_pow, tmp, *f_or_w;

	delta_f = 1.0 / (C->window * C->dt);
	eps_pow = 1.0 / sqrt(C->d_n_windows);	/* Multiplicative error bars for power spectra  */

	f_or_w = gmt_M_memory (GMT, NULL, C->n_spec, double);
	for (i = 0; i < C->n_spec; i++) f_or_w[i] = (write_wavelength) ? 1.0 / ((i + 1) * delta_f) : (i + 1) * delta_f;

	for (i = 0; i < C->n_spec; i++) {
		out[GMT_X][i] = f_or_w[i];

		/* loop through output choices */
		for (j = 0, k = 1; j < n_outputs; j++) {
			switch (col[j]) {
				case 'x':		/* return x power [ B&P 2nd Ed. eqn. 9.32 ] */
					out[k++][i] = C->spec[i].xpow;
					out[k++][i] = eps_pow * C->spec[i].xpow;
					break;
				case 'y':		/* Write y power [ B&P 2nd Ed. eqn. 9.32 ] */
					out[k++][i] = C->spec[i].ypow;
					out[k++][i] = eps_pow * C->spec[i].ypow;
					break;
				case 'c':		/* Write Coherent Output power [ B&P 2nd Ed. eqn. 9.71 ] */
					out[k++][i] = tmp = C->spec[i].ypow * C->spec[i].coh;
					out[k++][i] = tmp * eps_pow * sqrt ( (2.0 - C->spec[i].coh) / C->spec[i].coh);
					break;
				case 'n':		/* Write Noise Output power [ B&P 2nd Ed. eqn. 9.73 & Table 9.6 ] */
					out[k++][i] = tmp = C->spec[i].ypow * (1.0 - C->spec[i].coh);
					out[k++][i] = tmp * eps_pow;
					break;
				case 'g':		/* Write Gain spectrum [ B&P 2nd Ed. eqn. 9.90 & Table 9.6 ] */
					out[k++][i] = tmp = C->spec[i].gain;
					out[k++][i] = tmp * eps_pow * sqrt( (1.0 - C->spec[i].coh) / (2.0 * C->spec[i].coh) );
					break;
				case 'a':		/* Write Real Admittance spectrum
					We don't know the correct error estimate and it is not
					in Bendat and Piersol, or Priestly, or any standard text.
					Smith needs to derive this, and should make a note to
					check the expression given by Marcia Maia et al in Geophys.
					J. Int, 100, 337-348, 1990, equation 10, page 341.
					Meanwhile we will default to use the expression related to
					that for the gain spectrum: */
					out[k++][i] = tmp = C->spec[i].radmit;
					out[k++][i] = fabs (eps_pow * sqrt( (1.0 - C->spec[i].coh) / (2.0 * C->spec[i].coh) ) * tmp);
					break;
				case 'p':		/* Write Phase spectrum [ B&P 2nd Ed. eqn. 9.91 & Table 9.6 ] */
					out[k++][i] = C->spec[i].phase;
					out[k++][i] = eps_pow * sqrt( (1.0 - C->spec[i].coh) / (2.0 * C->spec[i].coh) );
					break;
				case 'o':		/* Write Coherency spectrum [ B&P 2nd Ed. eqn. 9.82 ] */
					out[k++][i] = tmp = C->spec[i].coh;
					out[k++][i] = tmp * eps_pow * (1.0 - C->spec[i].coh) * sqrt(2.0 / C->spec[i].coh);
					break;
			}
		}
	}

	gmt_M_free (GMT, f_or_w);
}

GMT_LOCAL void free_space_spectrum1d (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C) {
	gmt_M_free (GMT, C->spec);
	gmt_M_free (GMT, C->datac);
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPECTRUM1D_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct SPECTRUM1D_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->D.inc = 1.0;
	C->C.col[0] = 'x';
	C->C.col[1] = 'y';
	C->C.col[2] = 'c';
	C->C.col[3] = 'n';
	C->C.col[4] = 'p';
	C->C.col[5] = 'a';
	C->C.col[6] = 'g';
	C->C.col[7] = 'o';
	C->N.name = strdup ("spectrum");
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SPECTRUM1D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->N.name);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -S<segment_size> [-C[<xycnpago>]] [-D<dt>] [-L[m|h]] [-N[<name_stem>]] [-T]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-W] [%s] [%s] [%s] [%s]\n\t[%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_qi_OPT, GMT_s_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-S Use data subsets of <segment_size> elements.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <segment_size> must be radix 2;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   std. err. = 1/sqrt(n_data/segment_size).\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-C[<xycnpago>] 2 column X(t),Y(t) input; estimate Cross-spectra\n\t   [Default 1 col, X power only].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally specify cross-spectra output(s)  [Default is all].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   x = xpower, y = ypower, c = coherent power, n = noise power,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p = phase, a = admittance, g = gain, o = squared coherency.\n\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set delta_time sampling interval of data [Default = 1.0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Leave trend alone:  Do not remove least squares trend from data [Default removes trend].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append m to just remove mean or h to remove mid-value instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Supply name stem for files [Default = 'spectrum'].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Output files will be named <name_stem>.xpower, etc.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To disable the writing of individual files, just give -N.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Disable writing the single output to stdout.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Write Wavelength of spectral estimate in col 1 [Default = frequency].\n");
	GMT_Option (API, "bi2,bo,d,e,f,g,h,i,qi,s,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SPECTRUM1D_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to spectrum1d and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int sval;
	unsigned int n_errors = 0, j, window_test = 2, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			case '>':	/* Got named output file */
				if (n_files++ == 0 && gmt_check_filearg (GMT, '>', opt->arg, GMT_OUT, GMT_IS_DATASET))
					Ctrl->Out.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = true;
				if (!opt->arg[0]) break;	/* Stay with the default order of output */
				gmt_M_memset (Ctrl->C.col, SPECTRUM1D_N_OUTPUT_CHOICES, char);	/* Reset and read options */
				for (j = 0; opt->arg[j]; j++) {
					if (j < SPECTRUM1D_N_OUTPUT_CHOICES) {
						Ctrl->C.col[j] = opt->arg[j];
						if (!strchr ("xycnpago", Ctrl->C.col[j])) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Unrecognized output choice %c\n", Ctrl->C.col[j]);
							n_errors++;
						}
					}
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Too many output columns selected: Choose from -Cxycnpago\n");
						n_errors++;
					}
				}
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.inc = atof (opt->arg);
				break;
			case 'L':	/* Leave trend alone */
				if (opt->arg[0] == 'm') Ctrl->L.mode = 1;
				else if (opt->arg[0] == 'h') Ctrl->L.mode = 2;
				else Ctrl->L.active = true;
				break;
			case 'N':	/* Set alternate file stem OR turn off multiple files */
				Ctrl->N.active = true;
				if (opt->arg[0]) {
					if (opt->arg[0] == '+') {	/* Obsolete syntax */
						if (gmt_M_compat_check (GMT, 5)) {
							GMT_Report (API, GMT_MSG_COMPAT, "Modifier -N+<file> is deprecated; use -N in the future to save to stdout.\n");
							if (n_files++ == 0) Ctrl->Out.file = strdup (&opt->arg[1]);
						}
						else {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: Unrecognized modifier +%c\n", opt->arg[1]);
							n_errors++;
						}
					}
					else {	/* Set name stem */
						gmt_M_str_free (Ctrl->N.name);
						Ctrl->N.name = strdup (opt->arg);
					}
				}
				else	/* Turn off multiple file output */
					Ctrl->N.mode = 1;
				break;
			case 'S':
				Ctrl->S.active = true;
				sval = atoi (opt->arg);
				n_errors += gmt_M_check_condition (GMT, sval <= 0, "Syntax error -S option: segment size must be positive\n");
				Ctrl->S.size = sval;
				while (window_test < Ctrl->S.size) {
					window_test += window_test;
				}
				break;
			case 'T':	/* Turn off multicolumn single output file */
				Ctrl->T.active = true;
				break;
			case 'W':
				Ctrl->W.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, window_test != Ctrl->S.size, "Syntax error -S option: Segment size not radix 2.  Try %d or %d\n", (window_test/2), window_test);
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.inc <= 0.0, "Syntax error -D option: Sampling interval must be positive\n");
	n_errors += gmt_check_binary_io (GMT, Ctrl->C.active + 1);
	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.mode == 1 && Ctrl->T.active, "Syntax error: Cannot use both -T and -N as no output would be produced\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_spectrum1d (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int k, n_outputs;

	uint64_t tbl, seg, n_cols_tot = 0;

	double *y = NULL;	/* Used for cross-spectra only */
	struct SPECTRUM1D_INFO C;
	struct GMT_DATASET *Din = NULL, *Dout = NULL;
	struct GMT_DATATABLE *Tout = NULL;
	struct GMT_DATASEGMENT *S = NULL, *Sout = NULL;
	struct SPECTRUM1D_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the spectrum1d main code ----------------------------*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");
	gmt_M_memset (&C, 1, struct SPECTRUM1D_INFO);

	C.dt = Ctrl->D.inc;
	C.y_given = Ctrl->C.active;
	C.window = Ctrl->S.size;
	for (k = n_outputs = 0; k < SPECTRUM1D_N_OUTPUT_CHOICES && Ctrl->C.col[k]; k++) n_outputs++;

	if (!Ctrl->C.active) {		/* ensure x-power output */
		Ctrl->C.col[0] = 'x';
		n_outputs = 1;
		Ctrl->C.active = true;
	}

	if ((error = GMT_Set_Columns (API, GMT_IN, 1 + C.y_given, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (Din->n_columns < (1 + C.y_given)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least %d are needed\n", (int)Din->n_columns, 1 + C.y_given);
		Return (GMT_DIM_TOO_SMALL);
	}

	alloc_arrays (GMT, &C);

	if (!Ctrl->T.active) {	/* Write single data file with 17 columns to stdout (or specified name) */
		uint64_t dim[GMT_DIM_SIZE];
		n_cols_tot = 1 + 2 * n_outputs;
		dim[GMT_TBL] = Din->n_tables;	/* Same number of tables as input */
		dim[GMT_SEG] = 0;		/* Don't know about segments yet */
		dim[GMT_COL] = n_cols_tot;	/* Number of columns needed output file */
		dim[GMT_ROW] = C.n_spec;	/* Number of rows */
		if ((Dout = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
			Return (API->error);	/* An empty table for stacked results */
	}
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		if (!Ctrl->T.active) {
			gmt_free_table (GMT, Dout->table[tbl]);	/* Free it, then allocate separately */
			Dout->table[tbl] = Tout = gmt_create_table (GMT, Din->table[tbl]->n_segments, C.n_spec, n_cols_tot, 0U, false);
		}
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			S = Din->table[tbl]->segment[seg];	/* Current segment */
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Read %" PRIu64 " data points.\n", S->n_rows);

			y = (C.y_given) ? S->data[GMT_Y] : NULL;
			if (compute_spectra (GMT, &C, S->data[GMT_X], y, S->n_rows, Ctrl->L.active, Ctrl->L.mode)) {
				GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
			}

			if (!Ctrl->T.active) {
				unsigned smode = (Tout->segment[seg]->text) ? GMT_WITH_STRINGS : GMT_NO_STRINGS;
				Sout = Tout->segment[seg] = GMT_Alloc_Segment (GMT->parent, smode, C.n_spec, Tout->n_columns, NULL, Tout->segment[seg]);
				assign_output_spectrum1d (GMT, &C, Ctrl->C.col, n_outputs, Ctrl->W.active, Sout->data);
				Sout->n_rows = C.n_spec;
			}
			if (Ctrl->N.mode == 0) {	/* Write separate tables */
				if (write_output_separate (GMT, &C, Ctrl->C.col, n_outputs, Ctrl->W.active, Ctrl->N.name))
					Return (GMT_RUNTIME_ERROR);
			}
		}
	}

	free_space_spectrum1d (GMT, &C);

	if (!Ctrl->T.active && GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, 0, NULL, Ctrl->Out.file, Dout) != GMT_NOERROR) {
		Return (API->error);
	}

	Return (GMT_NOERROR);
}
