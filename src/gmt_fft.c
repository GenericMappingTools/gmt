/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Here are various ways to perform 1-D and 2-D Fourier transforms.
 * Most of these were provided by other people, as indicated below.
 *
 *
 * Author:  Paul Wessel
 * Date:    1-APR-2011
 * Version: 5
 *
 * Major overhaul, Florian Wobbe, 2012-07-09:
 *  Added free Kiss FFT (kissfft) to GMT code base.
 *  Removed Norman Brenner's ancient Cooley-Tukey FFT implementation,
 *    now superseded by Kiss FFT.
 *  Support for Paul Swarztrauber's ancient FFTPACK and for Sun Performance
 *    Library (perflib) have been removed too because they are not mainteined
 *    anymore.
 *
 *  FFTW3   : FFTW3 library (supplied externally)
 *  vDSP    : OSX Accelerate Framework (OSX only)
 *  Kiss FFT: Free FFT, based on the principle "Keep It Simple, Stupid"
 *  Configure the implementation with gmtset GMT_FFT.
 */

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_internals.h"

static char *GMT_fft_algo[k_n_fft_algorithms] = {
	"Auto-Select",
	"Accelerate Framework",
	"FFTW 3",
	"Kiss FFT",
};

static inline unsigned int propose_radix2 (unsigned n) {
	/* Returns the smallest base 2 exponent, log2n, that satisfies: 2^log2n >= n */
	unsigned log2n = 1;
	while ( 1U<<log2n < n ) ++log2n; /* log2n = 1<<(unsigned)ceil(log2(n)); */
	return log2n;
}

static inline unsigned int radix2 (unsigned n) {
	/* Returns the base 2 exponent that represents 'n' if 'n' is a power of 2,
	 * 0 otherwise */
	unsigned log2n = 1;
	while ( 1U<<log2n < n ) ++log2n; /* log2n = 1<<(unsigned)ceil(log2(n)); */
	if (n == 1U<<log2n)
		return log2n;
	return 0;
}

#ifdef HAVE_FFTW3F

#include <fftw3.h>

int GMT_fft_1d_fftwf (struct GMT_CTRL *C, float *data, unsigned int n, int direction, unsigned int mode)
{
	int sign = direction == k_fft_fwd ? FFTW_FORWARD : FFTW_BACKWARD;
	fftwf_complex *cin, *cout;
	fftwf_plan plan;
	cin = (fftwf_complex*)data; /* need fftwf_complex because GMT_GRID is complex float */
	cout = cin;                 /* in-place transform */

	/*
	 * The first argument, n, is the size of the transform you are trying to
	 * compute. The size n can be any positive integer, but sizes that are
	 * products of small factors are transformed most efficiently.
	 *
	 * The next two arguments are pointers to the input and output arrays of the
	 * transform. These pointers can be equal, indicating an in-place transform.
	 *
	 * The fourth argument, sign, can be either FFTW_FORWARD (-1) or
	 * FFTW_BACKWARD (+1), and indicates the direction of the transform you are
	 * interested in; technically, it is the sign of the exponent in the
	 * transform.
	 *
	 * The flags argument is usually either FFTW_MEASURE or FFTW_ESTIMATE.
	 * FFTW_MEASURE instructs FFTW to run and measure the execution time of
	 * several FFTs in order to find the best way to compute the transform of
	 * size n. This process takes some time (usually a few seconds), depending
	 * on your machine and on the size of the transform. */
	plan = fftwf_plan_dft_1d(n, cin, cout, sign, FFTW_ESTIMATE);

	fftwf_execute(plan); /* do transform */
	fftwf_destroy_plan(plan); /* deallocate plan */

	return GMT_NOERROR;
}

int GMT_fft_2d_fftwf (struct GMT_CTRL *C, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode)
{
	int sign = direction == k_fft_fwd ? FFTW_FORWARD : FFTW_BACKWARD;
	fftwf_complex *cin, *cout;
	fftwf_plan plan;
	cin = (fftwf_complex*)data; /* need fftwf_complex because GMT_GRID is complex float */
	cout = cin;                 /* in-place transform */

	/* The first two arguments, n0 and n1, are the size of the two-dimensional
	 * transform you are trying to compute. The size n can be any positive
	 * integer, but sizes that are products of small factors are transformed
	 * most efficiently.
	 *
	 * The next two arguments are pointers to the input and output arrays of the
	 * transform. These pointers can be equal, indicating an in-place transform.
	 *
	 * The fourth argument, sign, can be either FFTW_FORWARD (-1) or
	 * FFTW_BACKWARD (+1), and indicates the direction of the transform you are
	 * interested in; technically, it is the sign of the exponent in the
	 * transform.
	 *
	 * The flags argument is usually either FFTW_MEASURE or FFTW_ESTIMATE.
	 * FFTW_MEASURE instructs FFTW to run and measure the execution time of
	 * several FFTs in order to find the best way to compute the transform of
	 * size n. This process takes some time (usually a few seconds), depending
	 * on your machine and on the size of the transform. */
	plan = fftwf_plan_dft_2d(ny, nx, cin, cout, sign, FFTW_ESTIMATE);

	fftwf_execute(plan); /* do transform */
	fftwf_destroy_plan(plan); /* deallocate plan */

	return GMT_NOERROR;
}

#endif /* HAVE_FFTW3F */

#ifdef __APPLE__ /* Accelerate framework */

#include <Accelerate/Accelerate.h>

int GMT_fft_1d_vDSP (struct GMT_CTRL *C, float *data, unsigned int n, int direction, unsigned int mode)
{
	FFTDirection fft_direction = direction == k_fft_fwd ?
			kFFTDirection_Forward : kFFTDirection_Inverse;
	DSPComplex *dsp_complex = (DSPComplex *)data;
	DSPSplitComplex dsp_split_complex;

	/* Base 2 exponent that specifies the largest power of
	 * two that can be processed by fft: */
	vDSP_Length log2n = radix2 (n);
	FFTSetup setup;

	if (log2n == 0) {
		GMT_report (C, GMT_MSG_NORMAL, "Need Radix-2 input try: %u [n]\n", 1U<<propose_radix2 (n));
		return -1;
	}

	/* Build data structure that contains precalculated data for use by
	 * single-precision FFT functions: */
	setup = vDSP_create_fftsetup (log2n, kFFTRadix2);

	dsp_split_complex.realp = malloc(n * sizeof(float));
	dsp_split_complex.imagp = malloc(n * sizeof(float));
	if (dsp_split_complex.realp == NULL || dsp_split_complex.imagp == NULL)
		return -1; /* out of memory */
	vDSP_ctoz (dsp_complex, 2, &dsp_split_complex, 1, n);

	vDSP_fft_zip (setup, &dsp_split_complex, 1, log2n, fft_direction);

	vDSP_ztoc (&dsp_split_complex, 1, dsp_complex, 2, n);
	free (dsp_split_complex.realp);
	free (dsp_split_complex.imagp);
	vDSP_destroy_fftsetup (setup); /* Free single-precision FFT data structure */

	return GMT_NOERROR;
}

int GMT_fft_2d_vDSP (struct GMT_CTRL *C, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode)
{
	FFTDirection fft_direction = direction == k_fft_fwd ?
			kFFTDirection_Forward : kFFTDirection_Inverse;
	DSPComplex *dsp_complex = (DSPComplex *)data;
	DSPSplitComplex dsp_split_complex;

	/* Base 2 exponent that specifies the largest power of
	 * two that can be processed by fft: */
	vDSP_Length log2nx = radix2 (nx);
	vDSP_Length log2ny = radix2 (ny);
	unsigned int n_xy = nx * ny;
	FFTSetup setup;

	if (log2nx == 0 || log2ny == 0) {
		GMT_report (C, GMT_MSG_NORMAL, "Need Radix-2 input try: %u/%u [nx/ny]\n",
				1U<<propose_radix2 (nx), 1U<<propose_radix2 (ny));
		return -1;
	}

	/* Build data structure that contains precalculated data for use by
	 * single-precision FFT functions: */
	setup = vDSP_create_fftsetup (MAX (log2nx, log2ny), kFFTRadix2);

	dsp_split_complex.realp = malloc(n_xy * sizeof(float));
	dsp_split_complex.imagp = malloc(n_xy * sizeof(float));
	if (dsp_split_complex.realp == NULL || dsp_split_complex.imagp == NULL)
		return -1; /* out of memory */
	vDSP_ctoz (dsp_complex, 2, &dsp_split_complex, 1, n_xy);

	/* complex: */
	vDSP_fft2d_zip (setup, &dsp_split_complex, 1, 0, log2ny, log2nx, fft_direction);
	/* real:
	vDSP_fft2d_zrip (setup, &dsp_split_complex, 1, 0, log2ny, log2nx, fft_direction); */

	vDSP_ztoc (&dsp_split_complex, 1, dsp_complex, 2, n_xy);
	free (dsp_split_complex.realp);
	free (dsp_split_complex.imagp);
	vDSP_destroy_fftsetup (setup); /* Free single-precision FFT data structure */

	return GMT_NOERROR;
}
#endif /* APPLE Accelerate framework */

/* Kiss FFT */

#include "kiss_fft/kiss_fftnd.h"

int GMT_fft_1d_kiss (struct GMT_CTRL *C, float *data, unsigned int n, int direction, unsigned int mode)
{
	kiss_fft_cpx *fin, *fout;
	kiss_fft_cfg config;

	/* Initialize a FFT (or IFFT) config/state data structure */
	config = kiss_fft_alloc(n, direction == k_fft_inv, NULL, NULL);
  fin = fout = (kiss_fft_cpx *)data;
	kiss_fft (config, fin, fout); /* do transform */
	free (config); /* Free config data structure */

	return GMT_NOERROR;
}

int GMT_fft_2d_kiss (struct GMT_CTRL *C, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode)
{
	const int dim[2] = {ny, nx}; /* dimensions of fft */
	const int dimcount = 2;      /* number of dimensions */
	kiss_fft_cpx *fin, *fout;
	kiss_fftnd_cfg config;

	/* Initialize a FFT (or IFFT) config/state data structure */
	config = kiss_fftnd_alloc (dim, dimcount, direction == k_fft_inv, NULL, NULL);

	fin = fout = (kiss_fft_cpx *)data;
	kiss_fftnd (config, fin, fout); /* do transform */
	free (config); /* Free config data structure */

	return GMT_NOERROR;
}

int GMT_fft_1d_selection (struct GMT_CTRL *C, unsigned int n) {
	/* Returns the most suitable 1-D FFT for the job - or the one requested via GMT_FFT */
	if (C->current.setting.fft != k_fft_auto) {
		/* Specific selection requested */
		if (C->session.fft1d[C->current.setting.fft])
			return C->current.setting.fft; /* User defined FFT */
		GMT_report (C, GMT_MSG_NORMAL, "Desired FFT Algorithm (%s) not configured - choosing suitable alternative.\n", GMT_fft_algo[C->current.setting.fft]);
	}
	/* Here we want automatic selection from available candidates */
	if (C->session.fft1d[k_fft_accelerate] && radix2 (n))
		return k_fft_accelerate; /* Use if Radix-2 under OS/X */
	if (C->session.fft1d[k_fft_fftw3])
		return k_fft_fftw3;
	return k_fft_kiss; /* Default/fallback general-purpose FFT */
}

int GMT_fft_2d_selection (struct GMT_CTRL *C, unsigned int nx, unsigned int ny) {
	/* Returns the most suitable 2-D FFT for the job - or the one requested via GMT_FFT */
	if (C->current.setting.fft != k_fft_auto) {
		/* Specific selection requested */
		if (C->session.fft2d[C->current.setting.fft])
			return C->current.setting.fft; /* User defined FFT */
		GMT_report (C, GMT_MSG_NORMAL, "Desired FFT Algorithm (%s) not configured - choosing suitable alternative.\n", GMT_fft_algo[C->current.setting.fft]);
	}
	/* Here we want automatic selection from available candidates */
	if (C->session.fft2d[k_fft_accelerate] && radix2 (nx) && radix2 (ny))
		return k_fft_accelerate; /* Use if Radix-2 under OS/X */
	if (C->session.fft2d[k_fft_fftw3])
		return k_fft_fftw3;
	return k_fft_kiss; /* Default/fallback general-purpose FFT */
}

int GMT_fft_1d (struct GMT_CTRL *C, float *data, unsigned int n, int direction, unsigned int mode) {
	/* data is an array of length n (or 2*n for complex) data points
	 * n is the number of data points
	 * direction is either k_fft_fwd (forward) or k_fft_inv (inverse)
	 * mode is either k_fft_real or k_fft_complex
	 */
	int status, use;
	assert (mode == k_fft_complex); /* k_fft_real not implemented yet */
	use = GMT_fft_1d_selection (C, n);
	GMT_report (C, GMT_MSG_LONG_VERBOSE, "1-D FFT using %s\n", GMT_fft_algo[use]);
	status = C->session.fft1d[use] (C, data, n, direction, mode);
	return status;
}

int GMT_fft_2d (struct GMT_CTRL *C, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode) {
	/* data is an array of length nx*ny (or 2*nx*ny for complex) data points
	 * nx, ny is the number of data nodes
	 * direction is either k_fft_fwd (forward) or k_fft_inv (inverse)
	 * mode is either k_fft_real or k_fft_complex
	 */
	int status, use;
	assert (mode == k_fft_complex); /* k_fft_real not implemented yet */
	use = GMT_fft_2d_selection (C, nx, ny);
	GMT_report (C, GMT_MSG_LONG_VERBOSE, "2-D FFT using %s\n", GMT_fft_algo[use]);
	status = C->session.fft2d[use] (C, data, nx, ny, direction, mode);
	return status;
}

void GMT_fft_initialization (struct GMT_CTRL *C) {
	/* Called by GMT_begin and sets up pointers to the available FFT calls */

#if defined HAVE_FFTW3F_THREADS && !defined WIN32
	/* Don't know how to get the number of CPUs on Windows */
	int n_cpu = (int)sysconf(_SC_NPROCESSORS_CONF);

	if (n_cpu > 1) {
		/* one-time initialization required to use FFTW3 threads */
		if ( fftwf_init_threads() ) {
			fftwf_plan_with_nthreads(n_cpu);
			GMT_report (C, GMT_MSG_LONG_VERBOSE, "Initialize FFTW3 with %d threads.\n", n_cpu);
		}
	}
#endif /* HAVE_FFTW3_THREADS */

	/* Start with nothing */
	memset (C->session.fft1d, k_n_fft_algorithms, sizeof(void*));
	memset (C->session.fft2d, k_n_fft_algorithms, sizeof(void*));

#ifdef __APPLE__
	/* OS X Accelerate Framework */
	C->session.fft1d[k_fft_accelerate] = &GMT_fft_1d_vDSP;
	C->session.fft2d[k_fft_accelerate] = &GMT_fft_2d_vDSP;
#endif
#ifdef HAVE_FFTW3F
	/* single precision FFTW3 */
	C->session.fft1d[k_fft_fftw3] = &GMT_fft_1d_fftwf;
	C->session.fft2d[k_fft_fftw3] = &GMT_fft_2d_fftwf;
#endif /* HAVE_FFTW3F */
	/* Kiss FFT is the integrated fallback */
	C->session.fft1d[k_fft_kiss] = &GMT_fft_1d_kiss;
	C->session.fft2d[k_fft_kiss] = &GMT_fft_2d_kiss;
}

void GMT_fft_cleanup (void) {
	/* Called by GMT_end */
#if defined HAVE_FFTW3F_THREADS && !defined WIN32
	fftwf_cleanup_threads(); /* clean resources allocated internally by FFTW3 */
#endif
}
