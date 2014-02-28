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
/*
 *
 * Brief synopsis: Compute auto and cross spectra using Welch's method of
 * multiple overlapped windows.  Find 1 standard error bars
 * following expressions in Bendat & Piersol.
 *
 * Author:	W. H. F. Smith
 * Date:	1-JAN-2010
 * Version:	5 API
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

#define THIS_MODULE_NAME	"spectrum1d"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Compute auto- [and cross-] spectra from one [or two] timeseries"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-Vbfghis"

#define SPECTRUM1D_N_OUTPUT_CHOICES 8

struct SPECTRUM1D_CTRL {
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
	struct SPECT1D_W {	/* -W */
		bool active;
	} W;
};

struct SPECTRUM1D_INFO {	/* Control structure for spectrum1d */
	int y_given, n_spec, window, window_2;
	float *datac;
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

void alloc_arrays (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C)
{
	C->n_spec = C->window/2;	/* This means we skip zero frequency; data are detrended  */
	C->window_2 = 2 * C->window;		/* This is for complex array stuff  */

	C->spec = GMT_memory (GMT, NULL, C->n_spec, struct SPEC);
	C->datac = GMT_memory (GMT, NULL, C->window_2, float);
}

void detrend_and_hanning (struct SPECTRUM1D_INFO *C, bool leave_trend, unsigned int mode)
{
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
			C->datac[i] -= (float)(x_mean + tt * x_slope);
			C->datac[i] *= (float)hw;
			C->x_variance += (C->datac[i] * C->datac[i]);
			C->datac[i+1] -= (float)(y_mean + tt * y_slope);
			C->datac[i+1] *= (float)hw;
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
			C->datac[i] -= (float)(x_mean + tt * x_slope);
			C->datac[i] *= (float)hw;
			C->x_variance += (C->datac[i] * C->datac[i]);
		}
		C->x_variance /= C->window;
	}
}

void compute_spectra (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C, double *x, double *y, uint64_t n_data, bool leave_trend, unsigned int mode)
{
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
				C->datac[i] = (float)x[t];
				C->datac[i+1] = (float)y[t];
			}
		}
		else {
			for (t = t_start, i = 0; t < t_stop; t++, i+=2) {
				C->datac[i] = (float)x[t];
				C->datac[i+1] = 0.0f;
			}
		}

		detrend_and_hanning (C, leave_trend, mode);

		if (GMT_FFT_1D (GMT->parent, C->datac, C->window, GMT_FFT_FWD, GMT_FFT_COMPLEX))
			exit (EXIT_FAILURE);

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

		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
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
}

int write_output_separate (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C, char *col, int n_outputs, int write_wavelength, char *namestem)
{	/* Writes separate files for each output type.  Does NOT use GMT_Put_* functions */
	int i, j;
	double delta_f, eps_pow, out[3], *f_or_w = NULL;
	char fname[GMT_LEN256] = {""};
	FILE *fpout = NULL;

	delta_f = 1.0 / (C->window * C->dt);
	eps_pow = 1.0 / sqrt(C->d_n_windows);	/* Multiplicative error bars for power spectra  */

	f_or_w = GMT_memory (GMT, NULL, C->n_spec, double);
	for (i = 0; i < C->n_spec; i++) f_or_w[i] = (write_wavelength) ? 1.0 / ((i + 1) * delta_f) : (i + 1) * delta_f;

	/* loop through output choices */
	for (j = 0; j < n_outputs; j++) {
		switch (col[j]) {
		case 'x':		/* write x power [ B&P 2nd Ed. eqn. 9.32 ] */
			sprintf (fname, "%s.xpower", namestem);
			if ((fpout = GMT_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].xpow;
				out[2] = eps_pow * C->spec[i].xpow;
				GMT->current.io.output (GMT, fpout, 3, out);
			}
			GMT_fclose (GMT, fpout);
			break;

		case 'y':		/* Write y power [ B&P 2nd Ed. eqn. 9.32 ] */
			sprintf (fname, "%s.ypower", namestem);
			if ((fpout = GMT_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].ypow;
				out[2] = eps_pow * C->spec[i].ypow;
				GMT->current.io.output (GMT, fpout, 3, out);
			}
			GMT_fclose (GMT, fpout);
			break;
		case 'c':		/* Write Coherent Output power [ B&P 2nd Ed. eqn. 9.71 ] */
			sprintf (fname, "%s.cpower", namestem);
			if ((fpout = GMT_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].ypow * C->spec[i].coh;
				out[2] = out[GMT_Y] * eps_pow * sqrt ( (2.0 - C->spec[i].coh) / C->spec[i].coh);
				GMT->current.io.output (GMT, fpout, 3, out);
			}
			GMT_fclose (GMT, fpout);
			break;
		case 'n':		/* Write Noise Output power [ B&P 2nd Ed. eqn. 9.73 & Table 9.6 ] */
			sprintf (fname, "%s.npower", namestem);
			if ((fpout = GMT_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].ypow * (1.0 - C->spec[i].coh);
				out[2] = out[GMT_Y] * eps_pow;
				GMT->current.io.output (GMT, fpout, 3, out);
			}
			GMT_fclose (GMT, fpout);
			break;
		case 'g':		/* Write Gain spectrum [ B&P 2nd Ed. eqn. 9.90 & Table 9.6 ] */
			sprintf (fname, "%s.gain", namestem);
			if ((fpout = GMT_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].gain;
				out[2] = out[GMT_Y] * eps_pow * sqrt( (1.0 - C->spec[i].coh) / (2.0 * C->spec[i].coh) );
				GMT->current.io.output (GMT, fpout, 3, out);
			}
			GMT_fclose (GMT, fpout);
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
			if ((fpout = GMT_fopen (GMT, fname, "w")) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].radmit;
				out[2] = fabs (eps_pow * sqrt( (1.0 - C->spec[i].coh) / (2.0 * C->spec[i].coh) ) * out[GMT_Y]);
				GMT->current.io.output (GMT, fpout, 3, out);
			}
			GMT_fclose (GMT, fpout);
			break;
		case 'p':		/* Write Phase spectrum [ B&P 2nd Ed. eqn. 9.91 & Table 9.6 ] */
			sprintf (fname, "%s.phase", namestem);
			if ((fpout = GMT_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].phase;
				out[2] = eps_pow * sqrt( (1.0 - C->spec[i].coh) / (2.0 * C->spec[i].coh) );
				GMT->current.io.output (GMT, fpout, 3, out);
			}
			GMT_fclose (GMT, fpout);
		case 'o':		/* Write Coherency spectrum [ B&P 2nd Ed. eqn. 9.82 ] */
			sprintf (fname, "%s.coh", namestem);
			if ((fpout = GMT_fopen (GMT, fname, GMT->current.io.w_mode)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, " Writing %s\n", fname);
			for (i = 0; i < C->n_spec; i++) {
				out[GMT_X] = f_or_w[i];
				out[GMT_Y] = C->spec[i].coh;
				out[2] = out[GMT_Y] * eps_pow * (1.0 - C->spec[i].coh) * sqrt(2.0 / C->spec[i].coh);
				GMT->current.io.output (GMT, fpout, 3, out);
			}
			GMT_fclose (GMT, fpout);
			break;
		}
	}

	GMT_free (GMT, f_or_w);
	return (0);
}

void assign_output_spectrum1d (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C, char *col, int n_outputs, int write_wavelength, double *out[])
{	/* Fills out the 2-D table array given */
	int i, j, k;
	double delta_f, eps_pow, tmp, *f_or_w;

	delta_f = 1.0 / (C->window * C->dt);
	eps_pow = 1.0 / sqrt(C->d_n_windows);	/* Multiplicative error bars for power spectra  */

	f_or_w = GMT_memory (GMT, NULL, C->n_spec, double);
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
	
	GMT_free (GMT, f_or_w);
}

void free_space_spectrum1d (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C)
{
	GMT_free (GMT, C->spec);
	GMT_free (GMT, C->datac);
}

void *New_spectrum1d_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPECTRUM1D_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct SPECTRUM1D_CTRL);
	
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

void Free_spectrum1d_Ctrl (struct GMT_CTRL *GMT, struct SPECTRUM1D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->N.name) free (C->N.name);	
	GMT_free (GMT, C);	
}

int GMT_spectrum1d_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: spectrum1d [<table>] -S<segment_size> [-C[<xycnpago>]] [-D<dt>] [-L[m|h]] [-N[+]<name_stem>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-W] [%s] [%s]\n\t[%s]\n\t[%s] [%s]\n\t[%s]\n\n", GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_s_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-S Use data subsets of <segment_size> elements.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <segment_size> must be radix 2;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   std. err. = 1/sqrt(n_data/segment_size).\n");
	GMT_Message (API, GMT_TIME_NONE, "\tOptions:\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t   To save all selected spectra in a single table, use -N+<file>.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Write Wavelength of spectral estimate in col 1 [Default = frequency].\n");
	GMT_Option (API, "bi2,bo,f,g,h,i,s,.");
	
	return (EXIT_FAILURE);
}

int GMT_spectrum1d_parse (struct GMT_CTRL *GMT, struct SPECTRUM1D_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to spectrum1d and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int sval;
	unsigned int n_errors = 0, j, window_test = 2;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = true;
				if (!opt->arg[0]) break;	/* Stay with the default order of output */
				GMT_memset (Ctrl->C.col, SPECTRUM1D_N_OUTPUT_CHOICES, char);	/* Reset and read options */
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
			case 'N':
				Ctrl->N.active = true;
				if (opt->arg[0]) {
					free (Ctrl->N.name);
					if (opt->arg[0] == '+') Ctrl->N.mode = 1;
					Ctrl->N.name = strdup (&opt->arg[Ctrl->N.mode]);
				}
				break;
			case 'S':
				Ctrl->S.active = true;
				sval = atoi (opt->arg);
				n_errors += GMT_check_condition (GMT, sval <= 0, "Syntax error -S option: segment size must be positive\n");
				Ctrl->S.size = sval;
				while (window_test < Ctrl->S.size) {
					window_test += window_test;
				}
				break;
			case 'W':
				Ctrl->W.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, window_test != Ctrl->S.size, "Syntax error -S option: Segment size not radix 2.  Try %d or %d\n", (window_test/2), window_test);
	n_errors += GMT_check_condition (GMT, Ctrl->D.inc <= 0.0, "Syntax error -D option: Sampling interval must be positive\n");
	n_errors += GMT_check_binary_io (GMT, Ctrl->C.active + 1);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_spectrum1d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_spectrum1d (void *V_API, int mode, void *args)
{
	bool one_table;
	int error = 0;
	unsigned int k, n_outputs;
	
	uint64_t tbl, seg;

	double *y = NULL;	/* Used for cross-spectra only */
	struct SPECTRUM1D_INFO C;
	struct GMT_DATASET *Din = NULL, *Dout = NULL;
	struct GMT_DATATABLE *Tout = NULL;
	struct GMT_DATASEGMENT *S = NULL, *Sout = NULL;
	struct SPECTRUM1D_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_spectrum1d_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_spectrum1d_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_spectrum1d_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_spectrum1d_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_spectrum1d_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the spectrum1d main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	GMT_memset (&C, 1, struct SPECTRUM1D_INFO);
	
	C.dt = Ctrl->D.inc;
	C.y_given = Ctrl->C.active;
	C.window = Ctrl->S.size;
	one_table = (Ctrl->N.mode == 1);
	for (k = n_outputs = 0; k < SPECTRUM1D_N_OUTPUT_CHOICES && Ctrl->C.col[k]; k++) n_outputs++;

	if (!Ctrl->C.active) {		/* ensure x-power output */
		Ctrl->C.col[0] = 'x';
		n_outputs = 1;
		Ctrl->C.active = true;
	}

	if ((error = GMT_set_cols (GMT, GMT_IN, 1 + C.y_given)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}

	alloc_arrays (GMT, &C);

	if (one_table) {
		uint64_t dim[4];
		dim[GMT_TBL] = Din->n_tables;		/* Same number of tables as input */
		dim[GMT_SEG] = 0;			/* Don't know about segments yet */
		dim[GMT_COL] = 1 + 2 * n_outputs;	/* Number of columns needed output file */
		dim[GMT_ROW] = C.n_spec;		/* Number of rows */
		if ((Dout = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, Ctrl->N.name)) == NULL)
			Return (API->error);	/* An empty table for stacked results */
	}
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		if (one_table) {
			GMT_free_table (GMT, Dout->table[tbl], Dout->alloc_mode);	/* Free it, then allocate separately */
			Dout->table[tbl] = Tout = GMT_create_table (GMT, Din->table[tbl]->n_segments, C.n_spec, 3, false);
		}
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			S = Din->table[tbl]->segment[seg];	/* Current segment */
			GMT_Report (API, GMT_MSG_VERBOSE, "Read %" PRIu64 " data points.\n", S->n_rows);

			y = (C.y_given) ? S->coord[GMT_Y] : NULL;
			compute_spectra (GMT, &C, S->coord[GMT_X], y, S->n_rows, Ctrl->L.active, Ctrl->L.mode);

			if (one_table) {
				Sout = Tout->segment[seg];	/* Current output segment */
				GMT_alloc_segment (GMT, Sout, C.n_spec, Tout->n_columns, false);
				assign_output_spectrum1d (GMT, &C, Ctrl->C.col, n_outputs, Ctrl->W.active, Sout->coord);
			}
			else {
				if (write_output_separate (GMT, &C, Ctrl->C.col, n_outputs, Ctrl->W.active, Ctrl->N.name))
					Return (EXIT_FAILURE);
			}
		}
	}
	
	free_space_spectrum1d (GMT, &C);
	
	if (one_table && GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, Dout->io_mode, NULL, Ctrl->N.name, Dout) != GMT_OK) {
		Return (API->error);
	}

	Return (GMT_OK);
}
