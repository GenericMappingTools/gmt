/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *    Library (perflib) have been removed too because they are not maintained
 *    anymore.
 *
 *  FFTW    : FFTW library (supplied externally)
 *  vDSP    : OSX Accelerate Framework (OSX only)
 *  Kiss FFT: Free FFT, based on the principle "Keep It Simple, Stupid"
 *  Configure the implementation with gmtset GMT_FFT.
 */

#define GMT_WITH_NO_PS
#include "gmt.h"
#include "gmt_internals.h"

static char *GMT_fft_algo[] = {
	"Auto-Select",
	"Accelerate Framework",
	"FFTW",
	"Kiss FFT",
	"Brenner FFT [Legacy]"
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

#define FFTWF_WISDOM_FILENAME "fftwf_wisdom"

char *gmt_fftwf_wisdom_filename (struct GMT_CTRL *C) {
	static char wisdom_file[PATH_MAX+256] = "\0";
	char hostname[257];
	if (*wisdom_file == '\0') { /* wisdom_file has not been set yet */
		if (C->session.USERDIR == NULL || access (C->session.USERDIR, R_OK|W_OK|X_OK))
			/* USERDIR does not exist, or not writable */
			return NULL;
		else {
			/* create wisdom file in USERDIR */
			strncpy (wisdom_file, C->session.USERDIR, PATH_MAX);
			strcat (wisdom_file, "/" FFTWF_WISDOM_FILENAME "_");
			/* cat hostname */
			memset (hostname, '\0', 257); /* in case gethostname does not null-terminate string */
			gethostname (hostname, 256);
			strcat (wisdom_file, hostname);
		}
	}
	return wisdom_file;
}

/* Wrapper around fftwf_import_wisdom_from_filename */
void gmt_fftwf_import_wisdom_from_filename (struct GMT_CTRL *C) {
	static bool already_imported = false;
	char *filenames[3], **filename = filenames;
	int status;

	if (already_imported) /* nothing to do */
		return;

	fftwf_import_system_wisdom (); /* read wisdom from implementation-defined standard file */

	/* Initialize filenames */
	filenames[0] = FFTWF_WISDOM_FILENAME; /* 1st try importing wisdom from file in current dir */
	filenames[1] = gmt_fftwf_wisdom_filename(C); /* 2nd try wisdom file in USERDIR */
	filenames[2] = NULL; /* end of array */

	while (*filename != NULL) {
		if (!access (*filename, R_OK)) {
			status = fftwf_import_wisdom_from_filename (*filename);
			if (status)
				GMT_report (C, GMT_MSG_LONG_VERBOSE, "Imported FFTW Wisdom from file: %s\n", *filename);
			else
				GMT_report (C, GMT_MSG_NORMAL, "Importing FFTW Wisdom from file failed: %s\n", *filename);
		}
		++filename; /* advance to next file in array */
	}

	already_imported = true;
}

/* Wrapper around fftwf_export_wisdom_to_filename */
void gmt_fftwf_export_wisdom_to_filename (struct GMT_CTRL *C) {
	char *filename = gmt_fftwf_wisdom_filename(C);
	int status;

	if (filename == NULL)
		/* USERDIR does not exist, write wisdom to file in current directory */
		filename = FFTWF_WISDOM_FILENAME;

	status = fftwf_export_wisdom_to_filename (filename);
	if (status)
		GMT_report (C, GMT_MSG_LONG_VERBOSE, "Exported FFTW Wisdom to file: %s\n", filename);
	else
		GMT_report (C, GMT_MSG_NORMAL, "Exporting FFTW Wisdom to file failed: %s\n", filename);
}

fftwf_plan gmt_fftwf_plan_dft(struct GMT_CTRL *C, unsigned ny, unsigned nx, fftwf_complex *data, int direction) {
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
	 * on your machine and on the size of the transform.
	 *
	 * FFTW planner flags supported by the planner routines in FFTW
	 * FFTW_ESTIMATE:   pick a (probably sub-optimal) plan quickly
	 * FFTW_MEASURE:    find optimal plan by computing several FFTs and measuring
	 *                  their execution time
	 * FFTW_PATIENT:    like FFTW_MEASURE, but considers a wider range of algorithms
	 * FFTW_EXHAUSTIVE: like FFTW_PATIENT, but considers an even wider range of
	 *                  algorithms
	 *
	 * Important: the planner overwrites the input array during planning unless
	 * a saved plan (see Wisdom) is available for that problem, so you should
	 * initialize your input data after creating the plan. The only exceptions
	 * to this are the FFTW_ESTIMATE and FFTW_WISDOM_ONLY flags. */

	int sign;
	fftwf_complex *cin, *cout;
	fftwf_plan plan = NULL;

	sign = direction == k_fft_fwd ? FFTW_FORWARD : FFTW_BACKWARD;
	cin  = data;
	cout = cin; /* in-place transform */

	if (C->current.setting.fftw_plan != FFTW_ESTIMATE) {
		gmt_fftwf_import_wisdom_from_filename (C);
		if (ny == 0) /* 1d DFT */
			plan = fftwf_plan_dft_1d(nx, cin, cout, sign, FFTW_WISDOM_ONLY | C->current.setting.fftw_plan);
		else /* 2d DFT */
			plan = fftwf_plan_dft_2d(ny, nx, cin, cout, sign, FFTW_WISDOM_ONLY | C->current.setting.fftw_plan);
		if (plan == NULL) {
			/* No Wisdom available
			 * Need extra memory to prevent overwriting data while planning */
			fftwf_complex *in_place_tmp = fftwf_malloc (2 * (ny == 0 ? 1 : ny) * nx * sizeof(float));
			GMT_report (C, GMT_MSG_NORMAL, "Generating new FFTW Wisdom, be patient...\n");
			if (ny == 0) /* 1d DFT */
				plan = fftwf_plan_dft_1d(nx, in_place_tmp, in_place_tmp, sign, C->current.setting.fftw_plan);
			else /* 2d DFT */
				plan = fftwf_plan_dft_2d(ny, nx, in_place_tmp, in_place_tmp, sign, C->current.setting.fftw_plan);
			fftwf_destroy_plan(plan); /* deallocate plan */
			plan = NULL;
			fftwf_free (in_place_tmp);
			/* Save new Wisdom */
			gmt_fftwf_export_wisdom_to_filename (C);
		}
		else
			GMT_report (C, GMT_MSG_LONG_VERBOSE, "Using preexisting FFTW Wisdom.\n");
	} /* C->current.setting.fftw_plan != FFTW_ESTIMATE */
	else
		GMT_report (C, GMT_MSG_LONG_VERBOSE, "Picking a (probably sub-optimal) FFTW plan quickly.\n");

	if (plan == NULL) { /* If either FFTW_ESTIMATE or new Wisdom generated */
		if (ny == 0) /* 1d DFT */
			plan = fftwf_plan_dft_1d(nx, cin, cout, sign, C->current.setting.fftw_plan);
		else /* 2d DFT */
			plan = fftwf_plan_dft_2d(ny, nx, cin, cout, sign, C->current.setting.fftw_plan);
	}

	if (plan == NULL) { /* There was a problem creating a plan */
		GMT_report (C, GMT_MSG_NORMAL, "Error: Could not create FFTW plan.\n");
		GMT_exit (EXIT_FAILURE);
	}

	return plan;
}

int GMT_fft_1d_fftwf (struct GMT_CTRL *C, float *data, unsigned int n, int direction, unsigned int mode) {
	fftwf_plan plan = NULL;

	/* Generate FFTW plan for complex 1d DFT */
	plan = gmt_fftwf_plan_dft(C, 0, n, (fftwf_complex*)data, direction);
	fftwf_execute(plan);      /* do transform */
	fftwf_destroy_plan(plan); /* deallocate plan */

	return GMT_NOERROR;
}

int GMT_fft_2d_fftwf (struct GMT_CTRL *C, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode) {
	fftwf_plan plan = NULL;

	/* Generate FFTW plan for complex 2d DFT */
	plan = gmt_fftwf_plan_dft(C, ny, nx, (fftwf_complex*)data, direction);
	fftwf_execute(plan);      /* do transform */
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


int BRENNER_fourt_ (float *data, int *nn, int *ndim, int *ksign, int *iform, float *work)
{

    /* System generated locals */
    int i__1, i__2, i__3, i__4, i__5, i__6, i__7, i__8, i__9, i__10, i__11, i__12;

    /* Builtin functions */

    double cos(double), sin(double);

    /* Local variables */

    static int j1rg2, idiv, irem, ipar, kmin, imin, jmin, lmax, mmax, imax, jmax;
    static int ntwo, j1cnj, np1hf, np2hf, j1min, i1max, i1rng, j1rng, j2min, j3max;
    static int j1max, j2max, i2max, non2t, j2stp, i, j, k, l, m, n, icase, ifact[32];
    static int nhalf, krang, kconj, kdif, idim, ntot, kstep, k2, k3, k4, nprev, iquot;
    static int i2, i1, i3, j3, k1, j2, j1, if_, np0, np1, np2, ifp1, ifp2, non2;

    static float theta, oldsi, tempi, oldsr, sinth, difi, difr, sumi, sumr, tempr, twopi;
    static float wstpi, wstpr, twowr, wi, wr, u1i, w2i, w3i, u2i, u3i, u4i, t2i, u1r;
    static float u2r, u3r, w2r, w3r, u4r, t2r, t3r, t3i, t4r, t4i;
    static double wrd, wid;

/*---------------------------------------------------------------------------
       ARGUMENTS :
		DATA - COMPLEX ARRAY, LENGTH NN
		NN - ARRAY OF NUMBER OF POINTS IN EACH DIMENSION
		NDIM - NUMBER OF DIMENSIONS (FOR OUR PURPOSES, NDIM=1)
		KSIGN - +1 FOR INVERSE TRANSFORM (FREQ TO GMT_TIME DOMAIN)
			-1 FOR FORWARD TRANSFORM (GMT_TIME TO FREQ DOMAIN)
		IFORM - 0 REAL DATA
			+1 COMPLEX DATA
		WORK - 0 IF ALL DIMENSIONS ARE RADIX 2
		COMPLEX ARRAY, LARGE AS LARGEST NON-RADIX 2 DIMENSI0N

	PROGRAM BY NORMAN BRENNER FROM THE BASIC PROGRAM BY CHARLES
	RADER.  RALPH ALTER SUGGESTED THE IDEA FOR THE DIGIT REVERSAL.
	MIT LINCOLN LABORATORY, AUGUST 1967.

---------------------------------------------------------------------------*/

   /* Parameter adjustments */
    --work;
    --nn;
    --data;

    /* Function Body */
    wr = wi = wstpr = wstpi = (float)0.0;
    twopi = (float)6.283185307;
    if (*ndim - 1 >= 0) {
	goto L1;
    } else {
	goto L920;
    }
L1:
    ntot = 2;
    i__1 = *ndim;
    for (idim = 1; idim <= i__1; ++idim) {
	if (nn[idim] <= 0) {
	    goto L920;
	} else {
	    goto L2;
	}
L2:
	ntot *= nn[idim];
    }
    np1 = 2;
    i__1 = *ndim;
    for (idim = 1; idim <= i__1; ++idim) {
	n = nn[idim];
	np2 = np1 * n;
	if ((i__2 = n - 1) < 0) {
	    goto L920;
	} else if (i__2 == 0) {
	    goto L900;
	} else {
	    goto L5;
	}
L5:
	m = n;
	ntwo = np1;
	if_ = 1;
	idiv = 2;
L10:
	iquot = m / idiv;
	irem = m - idiv * iquot;
	if (iquot - idiv >= 0) {
	    goto L11;
	} else {
	    goto L50;
	}
L11:
	if (irem != 0) {
	    goto L20;
	} else {
	    goto L12;
	}
L12:
	ntwo += ntwo;
	m = iquot;
	goto L10;
L20:
	idiv = 3;
L30:
	iquot = m / idiv;
	irem = m - idiv * iquot;
	if (iquot - idiv >= 0) {
	    goto L31;
	} else {
	    goto L60;
	}
L31:
	if (irem != 0) {
	    goto L40;
	} else {
	    goto L32;
	}
L32:
	ifact[if_ - 1] = idiv;
	++if_;
	m = iquot;
	goto L30;
L40:
	idiv += 2;
	goto L30;
L50:
	if (irem != 0) {
	    goto L60;
	} else {
	    goto L51;
	}
L51:
	ntwo += ntwo;
	goto L70;
L60:
	ifact[if_ - 1] = m;
L70:
	non2 = np1 * (np2 / ntwo);
	icase = 1;
	if (idim - 4 >= 0) {
	    goto L90;
	} else {
	    goto L71;
	}
L71:
	if (*iform <= 0) {
	    goto L72;
	} else {
	    goto L90;
	}
L72:
	icase = 2;
	if (idim - 1 <= 0) {
	    goto L73;
	} else {
	    goto L90;
	}
L73:
	icase = 3;
	if (ntwo - np1 <= 0) {
	    goto L90;
	} else {
	    goto L74;
	}
L74:
	icase = 4;
	ntwo /= 2;
	n /= 2;
	np2 /= 2;
	ntot /= 2;
	i = 3;
	i__2 = ntot;
	for (j = 2; j <= i__2; ++j) {
	    data[j] = data[i];
	    i += 2;
	}
L90:
	i1rng = np1;
	if (icase - 2 != 0) {
	    goto L100;
	} else {
	    goto L95;
	}
L95:
	i1rng = np0 * (nprev / 2 + 1);
L100:
	if (ntwo - np1 <= 0) {
	    goto L600;
	} else {
	    goto L110;
	}
L110:
	np2hf = np2 / 2;
	j = 1;
	i__2 = np2;
	i__3 = non2;
	for (i2 = 1; i__3 < 0 ? i2 >= i__2 : i2 <= i__2; i2 += i__3) {
	    if (j - i2 >= 0) {
		goto L130;
	    } else {
		goto L120;
	    }
L120:
	    i1max = i2 + non2 - 2;
	    i__4 = i1max;
	    for (i1 = i2; i1 <= i__4; i1 += 2) {
		i__5 = ntot;
		i__6 = np2;
		for (i3 = i1; i__6 < 0 ? i3 >= i__5 : i3 <= i__5; i3 += i__6) {
		    j3 = j + i3 - i2;
		    tempr = data[i3];
		    tempi = data[i3 + 1];
		    data[i3] = data[j3];
		    data[i3 + 1] = data[j3 + 1];
		    data[j3] = tempr;
		    data[j3 + 1] = tempi;
		}
	    }
L130:
	    m = np2hf;
L140:
	    if (j - m <= 0) {
		goto L150;
	    } else {
		goto L145;
	    }
L145:
	    j -= m;
	    m /= 2;
	    if (m - non2 >= 0) {
		goto L140;
	    } else {
		goto L150;
	    }
L150:
	    j += m;
	}
	non2t = non2 + non2;
	ipar = ntwo / np1;
L310:
	if ((i__3 = ipar - 2) < 0) {
	    goto L350;
	} else if (i__3 == 0) {
	    goto L330;
	} else {
	    goto L320;
	}
L320:
	ipar /= 4;
	goto L310;
L330:
	i__3 = i1rng;
	for (i1 = 1; i1 <= i__3; i1 += 2) {
	    i__2 = non2;
	    i__6 = np1;
	    for (j3 = i1; i__6 < 0 ? j3 >= i__2 : j3 <= i__2; j3 +=  i__6) {
		i__5 = ntot;
		i__4 = non2t;
		for (k1 = j3; i__4 < 0 ? k1 >= i__5 : k1 <= i__5; k1 += i__4) {
		    k2 = k1 + non2;
		    tempr = data[k2];
		    tempi = data[k2 + 1];
		    data[k2] = data[k1] - tempr;
		    data[k2 + 1] = data[k1 + 1] - tempi;
		    data[k1] += tempr;
		    data[k1 + 1] += tempi;
		}
	    }
	}
L350:
	mmax = non2;
L360:
	if (mmax - np2hf >= 0) {
	    goto L600;
	} else {
	    goto L370;
	}
L370:
/* Computing MAX */
	i__4 = non2t, i__5 = mmax / 2;
	lmax = MAX(i__4,i__5);
	if (mmax - non2 <= 0) {
	    goto L405;
	} else {
	    goto L380;
	}
L380:
	theta = -twopi * (float) non2 / (float) (mmax <<  2);
	if (*ksign >= 0) {
	    goto L390;
	} else {
	    goto L400;
	}
L390:
	theta = -theta;
L400:
	sincos ((double)theta, &wid, &wrd);
	wr = (float)wrd;
	wi = (float)wid;
	wstpr = (float)-2.0 * wi * wi;
	wstpi = (float)2.0 * wr * wi;
L405:
	i__4 = lmax;
	i__5 = non2t;
	for (l = non2; i__5 < 0 ? l >= i__4 : l <= i__4; l += i__5) {
	    m = l;
	    if (mmax - non2 <= 0) {
		goto L420;
	    } else {
		goto L410;
	    }
L410:
	    w2r = wr * wr - wi * wi;
	    w2i = (float)(wr * 2.0 * wi);
	    w3r = w2r * wr - w2i * wi;
	    w3i = w2r * wi + w2i * wr;
L420:
	    i__6 = i1rng;
	    for (i1 = 1; i1 <= i__6; i1 += 2) {
		i__2 = non2;
		i__3 = np1;
		for (j3 = i1; i__3 < 0 ? j3 >= i__2 : j3 <= i__2; j3  += i__3) {
		    kmin = j3 + ipar * m;
		    if (mmax - non2 <= 0) {
			goto L430;
		    } else {
			goto L440;
		    }
L430:
		    kmin = j3;
L440:
		    kdif = ipar * mmax;
L450:
		    kstep = kdif << 2;
		    i__7 = ntot;
		    i__8 = kstep;
		    for (k1 = kmin; i__8 < 0 ? k1 >= i__7 : k1 <=  i__7; k1 += i__8) {
			k2 = k1 + kdif;
			k3 = k2 + kdif;
			k4 = k3 + kdif;
			if (mmax - non2 <= 0) {
			    goto L460;
			} else {
			    goto L480;
			}
L460:
			u1r = data[k1] + data[k2];
			u1i = data[k1 + 1] + data[k2 + 1];
			u2r = data[k3] + data[k4];
			u2i = data[k3 + 1] + data[k4 + 1];
			u3r = data[k1] - data[k2];
			u3i = data[k1 + 1] - data[k2 + 1];
			if (*ksign >= 0) {
			    goto L475;
			} else {
			    goto L470;
			}
L470:
			u4r = data[k3 + 1] - data[k4 + 1];
			u4i = data[k4] - data[k3];
			goto L510;
L475:
			u4r = data[k4 + 1] - data[k3 + 1];
			u4i = data[k3] - data[k4];
			goto L510;
L480:
			t2r = w2r * data[k2] - w2i * data[k2 + 1];
			t2i = w2r * data[k2 + 1] + w2i * data[k2];
			t3r = wr * data[k3] - wi * data[k3 + 1];
			t3i = wr * data[k3 + 1] + wi * data[k3];
			t4r = w3r * data[k4] - w3i * data[k4 + 1];
			t4i = w3r * data[k4 + 1] + w3i * data[k4];
			u1r = data[k1] + t2r;
			u1i = data[k1 + 1] + t2i;
			u2r = t3r + t4r;
			u2i = t3i + t4i;
			u3r = data[k1] - t2r;
			u3i = data[k1 + 1] - t2i;
			if (*ksign >= 0) {
			    goto L500;
			} else {
			    goto L490;
			}
L490:
			u4r = t3i - t4i;
			u4i = t4r - t3r;
			goto L510;
L500:
			u4r = t4i - t3i;
			u4i = t3r - t4r;
L510:
			data[k1] = u1r + u2r;
			data[k1 + 1] = u1i + u2i;
			data[k2] = u3r + u4r;
			data[k2 + 1] = u3i + u4i;
			data[k3] = u1r - u2r;
			data[k3 + 1] = u1i - u2i;
			data[k4] = u3r - u4r;
			data[k4 + 1] = u3i - u4i;
		    }
		    kmin = ((kmin - j3) << 2) + j3;
		    kdif = kstep;
		    if (kdif - np2 >= 0) {
			goto L530;
		    } else {
			goto L450;
		    }
L530:
		    ;
		}
	    }
	    m = mmax - m;
	    if (*ksign >= 0) {
		goto L550;
	    } else {
		goto L540;
	    }
L540:
	    tempr = wr;
	    wr = -wi;
	    wi = -tempr;
	    goto L560;
L550:
	    tempr = wr;
	    wr = wi;
	    wi = tempr;
L560:
	    if (m - lmax <= 0) {
		goto L565;
	    } else {
		goto L410;
	    }
L565:
	    tempr = wr;
	    wr = wr * wstpr - wi * wstpi + wr;
	    wi = wi * wstpr + tempr * wstpi + wi;
	}
	ipar = 3 - ipar;
	mmax += mmax;
	goto L360;
L600:
	if (ntwo - np2 >= 0) {
	    goto L700;
	} else {
	    goto L605;
	}
L605:
	ifp1 = non2;
	if_ = 1;
	np1hf = np1 / 2;
L610:
	ifp2 = ifp1 / ifact[if_ - 1];
	j1rng = np2;
	if (icase - 3 != 0) {
	    goto L612;
	} else {
	    goto L611;
	}
L611:
	j1rng = (np2 + ifp1) / 2;
	j2stp = np2 / ifact[if_ - 1];
	j1rg2 = (j2stp + ifp2) / 2;
L612:
	j2min = ifp2 + 1;
	if (ifp1 - np2 >= 0) {
	    goto L640;
	} else {
	    goto L615;
	}
L615:
	i__5 = ifp1;
	i__4 = ifp2;
	for (j2 = j2min; i__4 < 0 ? j2 >= i__5 : j2 <= i__5; j2 +=  i__4) {
	    theta = -twopi * (float) (j2 - 1) / (float)  np2;
	    if (*ksign >= 0) {
		goto L620;
	    } else {
		goto L625;
	    }
L620:
	    theta = -theta;
L625:
	    sinth = (float)sin((double)(0.5 * theta));
	    wstpr = sinth * (float)(-2. * sinth);
	    wstpi = (float)sin((double)theta);
	    wr = wstpr + (float)1.0;
	    wi = wstpi;
	    j1min = j2 + ifp1;
	    i__3 = j1rng;
	    i__2 = ifp1;
	    for (j1 = j1min; i__2 < 0 ? j1 >= i__3 : j1 <= i__3; j1 += i__2) {

		i1max = j1 + i1rng - 2;
		i__6 = i1max;
		for (i1 = j1; i1 <= i__6; i1 += 2) {
		    i__8 = ntot;
		    i__7 = np2;
		    for (i3 = i1; i__7 < 0 ? i3 >= i__8 : i3 <= i__8; i3 += i__7) {
			j3max = i3 + ifp2 - np1;
			i__9 = j3max;
			i__10 = np1;
			for (j3 = i3; i__10 < 0 ? j3 >= i__9 : j3 <= i__9; j3 += i__10) {
			    tempr = data[j3];
			    data[j3] = data[j3] * wr - data[j3 + 1] *  wi;
			    data[j3 + 1] = tempr * wi + data[j3 + 1]  * wr;
			}
		    }
		}
		tempr = wr;
		wr = wr * wstpr - wi * wstpi + wr;
		wi = tempr * wstpi + wi * wstpr + wi;
	    }
	}
L640:
	theta = -twopi / (float) ifact[if_ - 1];
	if (*ksign >= 0) {
	    goto L645;
	} else {
	    goto L650;
	}
L645:
	theta = -theta;
L650:
	sinth = (float)sin((double)(0.5 * theta));
	wstpr = sinth * (float)(-2. * sinth);
	wstpi = (float)sin((double)theta);
	kstep = (n << 1) / ifact[if_ - 1];
	krang = kstep * (ifact[if_ - 1] / 2) + 1;
	i__2 = i1rng;
	for (i1 = 1; i1 <= i__2; i1 += 2) {
	    i__3 = ntot;
	    i__4 = np2;
	    for (i3 = i1; i__4 < 0 ? i3 >= i__3 : i3 <= i__3; i3 += i__4) {
		i__5 = krang;
		i__10 = kstep;
		for (kmin = 1; i__10 < 0 ? kmin >= i__5 : kmin <= i__5; kmin += i__10) {
		    j1max = i3 + j1rng - ifp1;
		    i__9 = j1max;
		    i__7 = ifp1;
		    for (j1 = i3; i__7 < 0 ? j1 >= i__9 : j1 <= i__9; j1 += i__7) {
			j3max = j1 + ifp2 - np1;
			i__8 = j3max;
			i__6 = np1;
			for (j3 = j1; i__6 < 0 ? j3 >= i__8 : j3 <= i__8; j3 += i__6) {
			    j2max = j3 + ifp1 - ifp2;
			    k = kmin + (j3 - j1 + (j1 - i3) / ifact[if_ - 1]) / np1hf;
			    if (kmin - 1 <= 0) {
				goto L655;
			    } else {
				goto L665;
			    }
L655:
			    sumr = (float)0.0;
			    sumi = (float)0.0;
			    i__11 = j2max;
			    i__12 = ifp2;
			    for (j2 = j3; i__12 < 0 ? j2 >= i__11 : j2 <= i__11; j2 += i__12) {
				sumr += data[j2];
				sumi += data[j2 + 1];
			    }
			    work[k] = sumr;
			    work[k + 1] = sumi;
			    goto L680;
L665:
			    kconj = k + ((n - kmin + 1) << 1);
			    j2 = j2max;
			    sumr = data[j2];
			    sumi = data[j2 + 1];
			    oldsr = (float)0.0;
			    oldsi = (float)0.0;
			    j2 -= ifp2;
L670:
			    tempr = sumr;
			    tempi = sumi;
			    sumr = twowr * sumr - oldsr + data[j2];
			    sumi = twowr * sumi - oldsi + data[j2 + 1];
			    oldsr = tempr;
			    oldsi = tempi;
			    j2 -= ifp2;
			    if (j2 - j3 <= 0) {
				goto L675;
			    } else {
				goto L670;
			    }
L675:
			    tempr = wr * sumr - oldsr + data[j2];
			    tempi = wi * sumi;
			    work[k] = tempr - tempi;
			    work[kconj] = tempr + tempi;
			    tempr = wr * sumi - oldsi + data[j2 + 1];
			    tempi = wi * sumr;
			    work[k + 1] = tempr + tempi;
			    work[kconj + 1] = tempr - tempi;
L680:
			    ;
			}
		    }
		    if (kmin - 1 <= 0) {
			goto L685;
		    } else {
			goto L686;
		    }
L685:
		    wr = wstpr + (float)1.0;
		    wi = wstpi;
		    goto L690;
L686:
		    tempr = wr;
		    wr = wr * wstpr - wi * wstpi + wr;
		    wi = tempr * wstpi + wi * wstpr + wi;
L690:
		    twowr = wr + wr;
		}
		if (icase - 3 != 0) {
		    goto L692;
		} else {
		    goto L691;
		}
L691:
		if (ifp1 - np2 >= 0) {
		    goto L692;
		} else {
		    goto L695;
		}
L692:
		k = 1;
		i2max = i3 + np2 - np1;
		i__10 = i2max;
		i__5 = np1;
		for (i2 = i3; i__5 < 0 ? i2 >= i__10 : i2 <= i__10; i2 += i__5) {
		    data[i2] = work[k];
		    data[i2 + 1] = work[k + 1];
		    k += 2;
		}
		goto L698;
L695:
		j3max = i3 + ifp2 - np1;
		i__5 = j3max;
		i__10 = np1;
		for (j3 = i3; i__10 < 0 ? j3 >= i__5 : j3 <= i__5; j3 += i__10) {
		    j2max = j3 + np2 - j2stp;
		    i__6 = j2max;
		    i__8 = j2stp;
		    for (j2 = j3; i__8 < 0 ? j2 >= i__6 : j2 <= i__6; j2 += i__8) {
			j1max = j2 + j1rg2 - ifp2;
			j1cnj = j3 + j2max + j2stp - j2;
			i__7 = j1max;
			i__9 = ifp2;
			for (j1 = j2; i__9 < 0 ? j1 >= i__7 : j1 <= i__7; j1 += i__9) {
			    k = j1 + 1 - i3;
			    data[j1] = work[k];
			    data[j1 + 1] = work[k + 1];
			    if (j1 - j2 <= 0) {
				goto L697;
			    } else {
				goto L696;
			    }
L696:
			    data[j1cnj] = work[k];
			    data[j1cnj + 1] = -work[k + 1];
L697:
			    j1cnj -= ifp2;
			}
		    }
		}
L698:
		;
	    }
	}
	++if_;
	ifp1 = ifp2;
	if (ifp1 - np1 <= 0) {
	    goto L700;
	} else {
	    goto L610;
	}
L700:
	switch ((int)icase) {
	    case 1:  goto L900;
	    case 2:  goto L800;
	    case 3:  goto L900;
	    case 4:  goto L701;
	}
L701:
	nhalf = n;
	n += n;
	theta = -twopi / (float) n;
	if (*ksign >= 0) {
	    goto L702;
	} else {
	    goto L703;
	}
L702:
	theta = -theta;
L703:
	sinth = (float)sin((double)(0.5 * theta));
	wstpr = sinth * (float)(-2. * sinth);
	wstpi = (float)sin((double)theta);
	wr = wstpr + (float)1.0;
	wi = wstpi;
	imin = 3;
	jmin = (nhalf << 1) - 1;
	goto L725;
L710:
	j = jmin;
	i__4 = ntot;
	i__3 = np2;
	for (i = imin; i__3 < 0 ? i >= i__4 : i <= i__4; i += i__3) {
	    sumr = (float)0.5 * (data[i] + data[j]);
	    sumi = (float)0.5 * (data[i + 1] + data[j + 1]);
	    difr = (float)0.5 * (data[i] - data[j]);
	    difi = (float)0.5 * (data[i + 1] - data[j + 1]);
	    tempr = wr * sumi + wi * difr;
	    tempi = wi * sumi - wr * difr;
	    data[i] = sumr + tempr;
	    data[i + 1] = difi + tempi;
	    data[j] = sumr - tempr;
	    data[j + 1] = -difi + tempi;
	    j += np2;
	}
	imin += 2;
	jmin += -2;
	tempr = wr;
	wr = wr * wstpr - wi * wstpi + wr;
	wi = tempr * wstpi + wi * wstpr + wi;
L725:
	if ((i__3 = imin - jmin) < 0) {
	    goto L710;
	} else if (i__3 == 0) {
	    goto L730;
	} else {
	    goto L740;
	}
L730:
	if (*ksign >= 0) {
	    goto L740;
	} else {
	    goto L731;
	}
L731:
	i__3 = ntot;
	i__4 = np2;
	for (i = imin; i__4 < 0 ? i >= i__3 : i <= i__3; i += i__4) {
	    data[i + 1] = -data[i + 1];
	}
L740:
	np2 += np2;
	ntot += ntot;
	j = ntot + 1;
	imax = ntot / 2 + 1;
L745:
	imin = imax - (nhalf << 1);
	i = imin;
	goto L755;
L750:
	data[j] = data[i];
	data[j + 1] = -data[i + 1];
L755:
	i += 2;
	j += -2;
	if (i - imax >= 0) {
	    goto L760;
	} else {
	    goto L750;
	}
L760:
	data[j] = data[imin] - data[imin + 1];
	data[j + 1] = (float)0.0;
	if (i - j >= 0) {
	    goto L780;
	} else {
	    goto L770;
	}
L765:
	data[j] = data[i];
	data[j + 1] = data[i + 1];
L770:
	i += -2;
	j += -2;
	if (i - imin <= 0) {
	    goto L775;
	} else {
	    goto L765;
	}
L775:
	data[j] = data[imin] + data[imin + 1];
	data[j + 1] = (float)0.0;
	imax = imin;
	goto L745;
L780:
	data[1] += data[2];
	data[2] = (float)0.0;
	goto L900;
L800:
	if (i1rng - np1 >= 0) {
	    goto L900;
	} else {
	    goto L805;
	}
L805:
	i__4 = ntot;
	i__3 = np2;
	for (i3 = 1; i__3 < 0 ? i3 >= i__4 : i3 <= i__4; i3 += i__3) {
	    i2max = i3 + np2 - np1;
	    i__2 = i2max;
	    i__9 = np1;
	    for (i2 = i3; i__9 < 0 ? i2 >= i__2 : i2 <= i__2; i2 += i__9) {
		imin = i2 + i1rng;
		imax = i2 + np1 - 2;
		jmax = (i3 << 1) + np1 - imin;
		if (i2 - i3 <= 0) {
		    goto L820;
		} else {
		    goto L810;
		}
L810:
		jmax += np2;
L820:
		if (idim - 2 <= 0) {
		    goto L850;
		} else {
		    goto L830;
		}
L830:
		j = jmax + np0;
		i__7 = imax;
		for (i = imin; i <= i__7; i += 2) {
		    data[i] = data[j];
		    data[i + 1] = -data[j + 1];
		    j += -2;
		}
L850:
		j = jmax;
		i__7 = imax;
		i__8 = np0;
		for (i = imin; i__8 < 0 ? i >= i__7 : i <= i__7; i += i__8) {
		    data[i] = data[j];
		    data[i + 1] = -data[j + 1];
		    j -= np0;
		}
	    }
	}
L900:
	np0 = np1;
	np1 = np2;
	nprev = n;
    }
L920:
    return 0;
} /* fourt_ */

int gmt_get_non_symmetric_f (unsigned int *f, unsigned int n_in)
{
        /* Return the product of the non-symmetric factors in f[]  */
	
        int i = 0, j = 1, retval = 1, n = n_in;
	
        if (n == 1) return (f[0]);
        while (i < n) {
                while (j < n && f[j] == f[i]) j++;
                if ((j-i)%2) retval *= f[i];
                i = j;
                j = i + 1;
        }
        if (retval == 1) retval = 0;        /* There are no non-sym factors  */
        return (retval);
}

size_t brenner_worksize (struct GMT_CTRL *C, unsigned int nx, unsigned int ny)
{
        /* Find the size of the workspace that will be needed by the transform.
         * To use this routine for a 1-D transform, set ny = 1.
         * 
         * This is all based on the comments in Norman Brenner's code
         * FOURT, from which our C codes are translated.
         * 
         * Let m = largest prime factor in the list of factors.
         * Let p = product of all primes which appear an odd number of
         * times in the list of prime factors.  Then the worksize needed
         * s = max(m,p).  However, we know that if n is radix 2, then no
         * work is required; yet this formula would say we need a worksize
         * of at least 2.  So I will return s = 0 when max(m,p) = 2.
         *
         * W. H. F. Smith, 26 February 1992.
         *  */
        unsigned int f[32], n_factors, nonsymx, nonsymy, nonsym;
        size_t storage, ntotal;
	
        /* Find workspace needed.  First find non_symmetric factors in nx, ny  */
        n_factors = GMT_get_prime_factors (C, nx, f);
        nonsymx = gmt_get_non_symmetric_f (f, n_factors);
        n_factors = GMT_get_prime_factors (C, ny, f);
        nonsymy = gmt_get_non_symmetric_f (f, n_factors);
        nonsym = MAX (nonsymx, nonsymy);
	
        /* Now get factors of ntotal  */
        ntotal = GMT_get_nm (C, nx, ny);
        n_factors = GMT_get_prime_factors (C, ntotal, f);
        storage = MAX (nonsym, f[n_factors-1]);
        if (storage != 2) storage *= 2;
        if (storage < nx) storage = nx;
        if (storage < ny) storage = ny;
        return (2 * storage);
}

/* C-callable wrapper for BRENNER_fourt_ */
int GMT_fft_1d_brenner (struct GMT_CTRL *C, float *data, unsigned int n, int direction, unsigned int mode)
{
        /* void GMT_fourt (struct GMT_CTRL *C, float *data, int *nn, int ndim, int ksign, int iform, float *work) */
        /* Data array */
        /* Dimension array */
        /* Number of dimensions */
        /* Forward(-1) or Inverse(+1) */
        /* Real(0) or complex(1) data */
        /* Work array */
	
        int ksign, ndim = 1, n_signed = n, kmode = mode;
        size_t work_size = 0;
        float *work = NULL;
	
        ksign = (direction == k_fft_inv) ? +1 : -1;
        if ((work_size = brenner_worksize (C, n, 1))) work = GMT_memory (C, NULL, work_size, float);
        (void) BRENNER_fourt_ (data, &n_signed, &ndim, &ksign, &kmode, work);
        if (work_size) GMT_free (C, work);	
        return (GMT_OK);
}
	
int GMT_fft_2d_brenner (struct GMT_CTRL *C, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode)
{
        /* Data array */
        /* Dimension array */
        /* Number of dimensions */
        /* Forward(-1) or Inverse(+1) */
        /* Real(0) or complex(1) data */
        /* Work array */

        int ksign, ndim = 2, nn[2] = {nx, ny}, kmode = mode;
        size_t work_size = 0;
        float *work = NULL;

        ksign = (direction == k_fft_inv) ? +1 : -1;
        if ((work_size = brenner_worksize (C, nx, ny))) work = GMT_memory (C, NULL, work_size, float);
        GMT_report (C, GMT_MSG_LONG_VERBOSE, "Brenner_fourt_ work size = %zu\n", work_size);
        (void) BRENNER_fourt_ (data, nn, &ndim, &ksign, &kmode, work);
        if (work_size) GMT_free (C, work);
        return (GMT_OK);
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
	if (C->session.fft1d[k_fft_fftw])
		return k_fft_fftw;
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
	if (C->session.fft2d[k_fft_fftw])
		return k_fft_fftw;
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

#if defined WIN32
#include <windows.h>
#endif

void GMT_fft_initialization (struct GMT_CTRL *C) {
	/* Called by GMT_begin and sets up pointers to the available FFT calls */
#if defined HAVE_FFTW3F_THREADS
	int n_cpu;
#if defined WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo ( &sysinfo );
	n_cpu = sysinfo.dwNumberOfProcessors;
#else
	n_cpu = (int)sysconf (_SC_NPROCESSORS_CONF);
#endif
	if (n_cpu > 1) {
		/* one-time initialization required to use FFTW3 threads */
		if ( fftwf_init_threads() ) {
			fftwf_plan_with_nthreads(n_cpu);
			GMT_report (C, GMT_MSG_LONG_VERBOSE, "Initialize FFTW with %d threads.\n", n_cpu);
		}
	}

	C->current.setting.fftw_plan = FFTW_ESTIMATE; /* default planner flag */
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
	C->session.fft1d[k_fft_fftw] = &GMT_fft_1d_fftwf;
	C->session.fft2d[k_fft_fftw] = &GMT_fft_2d_fftwf;
#endif /* HAVE_FFTW3F */
	/* Kiss FFT is the integrated fallback */
	C->session.fft1d[k_fft_kiss] = &GMT_fft_1d_kiss;
	C->session.fft2d[k_fft_kiss] = &GMT_fft_2d_kiss;
	/* Brenner FFT is the legacy fallback */
	C->session.fft1d[k_fft_brenner] = &GMT_fft_1d_brenner;
	C->session.fft2d[k_fft_brenner] = &GMT_fft_2d_brenner;
}

void GMT_fft_cleanup (void) {
	/* Called by GMT_end */
#if defined HAVE_FFTW3F_THREADS
	fftwf_cleanup_threads(); /* clean resources allocated internally by FFTW */
#endif
}
