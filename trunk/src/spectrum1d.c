/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
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

#include "gmt.h"

#define SPECTRUM1D_N_OUTPUT_CHOICES 8

struct SPECTRUM1D_CTRL {
	struct C {	/* -C[<xycnpago>] */
		GMT_LONG active;
		char col[SPECTRUM1D_N_OUTPUT_CHOICES];	/* Character codes for desired output in the right order */
	} C;
	struct D {	/* -D<inc> */
		GMT_LONG active;
		double inc;
	} D;
	struct N {	/* -N[+]<namestem> */
		GMT_LONG active;
		GMT_LONG mode;
		char *name;
	} N;
	struct S {	/* -S<segment_size> */
		GMT_LONG active;
		GMT_LONG size;
	} S;
	struct W {	/* -W */
		GMT_LONG active;
	} W;
};

struct SPECTRUM1D_INFO {	/* Control structure for spectrum1d */
	GMT_LONG y_given, n_spec, window, window_2;
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

void detrend_and_hanning (struct SPECTRUM1D_INFO *C)
{
	GMT_LONG i, t;
	double sumx, sumtx, sumy, sumty, sumt2, x_slope, x_mean, y_slope, y_mean;
	double t_factor, h_period, h_scale, hc, hw, tt;
	sumx = sumtx = sumy = sumty = sumt2 = 0.0;
	C->x_variance = C->y_variance = 0.0;
	t_factor = 2.0 / (C->window - 1);
	h_period = M_PI / (double)C->window;	/* For Hanning window  */
	h_scale = sqrt (8.0/3.0);		/* For Hanning window  */

	if (C->y_given) {
		for (i = 0, t = 0; i < C->window_2; i+=2, t++) {
			tt = t * t_factor - 1.0;
			sumt2 += (tt * tt);
			sumx += C->datac[i];
			sumtx += (tt * C->datac[i]);
			sumy += C->datac[i+1];
			sumty += (tt * C->datac[i+1]);
		}
	}
	else {
		for (i = 0, t = 0; i < C->window_2; i+=2, t++) {
			tt = t * t_factor - 1.0;
			sumt2 += (tt * tt);
			sumx += C->datac[i];
			sumtx += (tt * C->datac[i]);
		}
	}
	x_slope = sumtx / sumt2;
	x_mean = sumx / C->window;
	if (C->y_given) {
		y_slope = sumty / sumt2;
		y_mean = sumy / C->window;
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

void compute_spectra (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C, double *x, double *y, GMT_LONG n_data)
{
	GMT_LONG n_windows, w, i, t_start, t_stop, t, f;
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
		t_start = (GMT_LONG)floor (0.5 + w * dw);
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
				C->datac[i+1] = 0.0;
			}
		}

		detrend_and_hanning (C);

		GMT_fft_1d (GMT, C->datac, C->window, GMT_FFT_FWD, GMT_FFT_COMPLEX);
		
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

		if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
			C->y_pow = (C->y_given) ? C->y_variance/y_varp : 0.0;
			GMT_message (GMT, "Window %ld from %ld to %ld\n", w, t_start, t_stop);
			sprintf(format, "X var: %s  X pow: %s  ratio: %s  Y var: %s  Y pow: %s  ratio: %s\n",
				GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			GMT_message (GMT, format, C->x_variance, x_varp, (C->x_variance/x_varp), C->y_variance, y_varp, C->y_pow);
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

GMT_LONG write_output_separate (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C, char *col, GMT_LONG n_outputs, GMT_LONG write_wavelength, char *namestem)
{	/* Writes separate files for each output type.  Does NOT use GMT_Put_* functions */
	GMT_LONG i, j;
	double delta_f, eps_pow, out[3], *f_or_w = NULL;
	char fname[GMT_TEXT_LEN256];
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
				GMT_report (GMT, GMT_MSG_FATAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_report (GMT, GMT_MSG_NORMAL, " Writing %s\n", fname);
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
				GMT_report (GMT, GMT_MSG_FATAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_report (GMT, GMT_MSG_NORMAL, " Writing %s\n", fname);
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
				GMT_report (GMT, GMT_MSG_FATAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_report (GMT, GMT_MSG_NORMAL, " Writing %s\n", fname);
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
				GMT_report (GMT, GMT_MSG_FATAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_report (GMT, GMT_MSG_NORMAL, " Writing %s\n", fname);
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
				GMT_report (GMT, GMT_MSG_FATAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_report (GMT, GMT_MSG_NORMAL, " Writing %s\n", fname);
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
				GMT_report (GMT, GMT_MSG_FATAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_report (GMT, GMT_MSG_NORMAL, " Writing %s\n", fname);
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
				GMT_report (GMT, GMT_MSG_FATAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_report (GMT, GMT_MSG_NORMAL, " Writing %s\n", fname);
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
				GMT_report (GMT, GMT_MSG_FATAL, " Cannot open w %s\n", fname);
				return (EXIT_FAILURE);
			}
			GMT_report (GMT, GMT_MSG_NORMAL, " Writing %s\n", fname);
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

void write_output_spectrum1d (struct GMT_CTRL *GMT, struct SPECTRUM1D_INFO *C, char *col, GMT_LONG n_outputs, GMT_LONG write_wavelength, double *out[])
{	/* Fills out the 2-D table array given */
	GMT_LONG i, j, k;
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
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
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
	return ((void *)C);
}

void Free_spectrum1d_Ctrl (struct GMT_CTRL *GMT, struct SPECTRUM1D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->N.name) free ((void *)C->N.name);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_spectrum1d_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "spectrum1d %s [API] - Compute auto- [and cross- ] spectra from one [or two] timeseries\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: spectrum1d [<table>] -S<segment_size> [-C[<xycnpago>]] [-D<dt>] [-N[+]<name_stem>]\n");
	GMT_message (GMT, "\t[%s] [-W] [%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-S Use data subsets of <segment_size> elements.\n");
	GMT_message (GMT, "\t   <segment_size> must be radix 2;\n");
	GMT_message (GMT, "\t   std. err. = 1/sqrt(n_data/segment_size).\n");
	GMT_message (GMT, "\tOptions:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-C[<xycnpago>] 2 column X(t),Y(t) input; estimate Cross-spectra\n\t   [Default 1 col, X power only].\n");
	GMT_message (GMT, "\t   Optionally specify cross-spectra output(s)  [Default is all].\n");
	GMT_message (GMT, "\t   x = xpower, y = ypower, c = coherent power, n = noise power,\n");
	GMT_message (GMT, "\t   p = phase, a = admittance, g = gain, o = squared coherency.\n\n");
	GMT_message (GMT, "\t-D Set delta_time sampling interval of data [Default = 1.0].\n");
	GMT_message (GMT, "\t-N Supply name stem for files [Default = 'spectrum'].\n");
	GMT_message (GMT, "\t   Output files will be named <name_stem>.xpower, etc.\n");
	GMT_message (GMT, "\t   To save all selected spectra in a single table, use -N+<file>.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Write Wavelength of spectral estimate in col 1 [Default = frequency].\n");
	GMT_explain_options (GMT, "C2D0fghi.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_spectrum1d_parse (struct GMTAPI_CTRL *C, struct SPECTRUM1D_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to spectrum1d and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, j, window_test = 2;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = TRUE;
				if (!opt->arg[0]) break;	/* Stay with the default order of output */
				GMT_memset (Ctrl->C.col, SPECTRUM1D_N_OUTPUT_CHOICES, char);	/* Reset and read options */
				for (j = 0; opt->arg[j]; j++) {
					if (j < SPECTRUM1D_N_OUTPUT_CHOICES) {
						Ctrl->C.col[j] = opt->arg[j];
						if (!strchr ("xycnpago", Ctrl->C.col[j])) {
							GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -C option: Unrecognized output choice %c\n", Ctrl->C.col[j]);
							n_errors++;
						}
					}
					else {
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -C option: Too many output columns selected: Choose from -Cxycnpago\n");
						n_errors++;
					}
				}
				break;
			case 'D':
				Ctrl->D.active = TRUE;
				Ctrl->D.inc = atof (opt->arg);
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				if (opt->arg[0]) {
					free ((void *)Ctrl->N.name);
					if (opt->arg[0] == '+') Ctrl->N.mode = 1;
					Ctrl->N.name = strdup (&opt->arg[Ctrl->N.mode]);
				}
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				Ctrl->S.size = atoi (opt->arg);
				while (window_test < Ctrl->S.size) {
					window_test += window_test;
				}
				break;
			case 'W':
				Ctrl->W.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->S.size <= 0, "Syntax error -S option: segment size must be positive\n");
	n_errors += GMT_check_condition (GMT, window_test != Ctrl->S.size, "Syntax error -S option: Segment size not radix 2.  Try %ld or %ld\n", (window_test/2), window_test);
	n_errors += GMT_check_condition (GMT, Ctrl->D.inc <= 0.0, "Syntax error -D option: Sampling interval must be positive\n");
	n_errors += GMT_check_binary_io (GMT, Ctrl->C.active + 1);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_spectrum1d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_spectrum1d (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = FALSE, one_table, tbl, seg, k, n_outputs;

	struct SPECTRUM1D_INFO C;
	struct GMT_DATASET *Din = NULL, *Dout = NULL;
	struct GMT_TABLE *Tout = NULL;
	struct GMT_LINE_SEGMENT *S = NULL, *Sout = NULL;
	struct SPECTRUM1D_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_spectrum1d_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_spectrum1d_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_spectrum1d", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vbf", "ghis", options))) Return (error);
	Ctrl = (struct SPECTRUM1D_CTRL *) New_spectrum1d_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_spectrum1d_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the spectrum1d main code ----------------------------*/

	GMT_memset (&C, 1, struct SPECTRUM1D_INFO);
	
	C.dt = Ctrl->D.inc;
	C.y_given = Ctrl->C.active;
	C.window = Ctrl->S.size;
	one_table = (Ctrl->N.mode == 1);
	for (k = n_outputs = 0; k < SPECTRUM1D_N_OUTPUT_CHOICES && Ctrl->C.col[k]; k++) n_outputs++;

	if (!Ctrl->C.active) {		/* ensure x-power output */
		Ctrl->C.col[0] = 'x';
		n_outputs = 1;
		Ctrl->C.active = TRUE;
	}

	if ((error = GMT_set_cols (GMT, GMT_IN, 1 + C.y_given))) Return (error);
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, (void **)&Din)) Return ((error = GMT_DATA_READ_ERROR));
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */

	alloc_arrays (GMT, &C);

	if (one_table) {
		Dout = GMT_memory (GMT, NULL, 1, struct GMT_DATASET);				/* Output dataset... */
		Dout->table = GMT_memory (GMT, NULL, Din->n_tables, struct GMT_TABLE *);	/* with table array */
		if ((error = GMT_set_cols (GMT, GMT_OUT, Din->n_columns))) Return (error);
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	}
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		if (one_table) {
			GMT_create_table (GMT, &Tout, Din->table[tbl]->n_segments, Din->n_columns, 0);
			Dout->table[tbl] = Tout;
		}
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			S = Din->table[tbl]->segment[seg];	/* Current segment */
			GMT_report (GMT, GMT_MSG_NORMAL, "Read %ld data points.\n", S->n_rows);

			compute_spectra (GMT, &C, S->coord[GMT_X], S->coord[GMT_Y], S->n_rows);

			if (one_table) {
				Sout = Tout->segment[seg];	/* Current output segment */
				GMT_alloc_segment (GMT, Sout, C.window, Din->n_columns, TRUE);
				write_output_spectrum1d (GMT, &C, Ctrl->C.col, n_outputs, Ctrl->W.active, Sout->coord);
			}
			else {
				if (write_output_separate (GMT, &C, Ctrl->C.col, n_outputs, Ctrl->W.active, Ctrl->N.name)) Return (EXIT_FAILURE);
			}
		}
	}
	
	free_space_spectrum1d (GMT, &C);
	
	if (one_table) {
		if ((error = GMT_Put_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, Dout->io_mode, (void **)&(Ctrl->N.name), (void *)Dout))) Return (error);
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */
	}

	Return (GMT_OK);
}
