/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *  Superseded Norman Brenner's ancient Cooley-Tukey FFT implementation
 *    Kiss FFT. Brenner still available as a legacy/compativility choice.
 *  Support for Paul Swarztrauber's ancient FFTPACK and for Sun Performance
 *    Library (perflib) have been removed too because they are not maintained
 *    anymore.
 *
 *  FFTW    : FFTW library (supplied externally)
 *  vDSP    : OSX Accelerate Framework (OSX only)
 *  Kiss FFT: Free FFT, based on the principle "Keep It Simple, Stupid"
 *  Configure the implementation with gmtset GMT_FFT.
 *
 *--------------------------------------------------------------------------
 * Public functions declared in gmt_dev.h ():
 *
 *
 */

#include "gmt_dev.h"
#include "gmt_internals.h"

static char *GMT_fft_algo[] = {
	"Auto-Select",
	"Accelerate Framework",
	"FFTW",
	"Kiss FFT",
	"Brenner FFT [Legacy]"
};

/* Functions that fascilitating setting up FFT operations, like determining optimal dimension,
 * setting wavenumber functions, apply interior or exterior taper to grids, save pre-fft grid
 * to file, save fft grid (real and imag or mag,hypot) to two files.  Programs that wish to
 * operate on data in the wavenumber domain will need to use some of these functions; see
 * grdfft.c and potential/gravfft.c as examples.
 */

static inline uint64_t propose_radix2 (uint64_t n) {
	/* Returns the smallest base 2 exponent, log2n, that satisfies: 2^log2n >= n */
	uint64_t log2n = 1;
	while ( 1ULL<<log2n < n ) ++log2n; /* log2n = 1<<(unsigned)ceil(log2(n)); */
	return log2n;
}

static inline uint64_t radix2 (uint64_t n) {
	/* Returns the base 2 exponent that represents 'n' if 'n' is a power of 2,
	 * 0 otherwise */
	uint64_t log2n = 1ULL;
	while ( 1ULL<<log2n < n ) ++log2n; /* log2n = 1<<(unsigned)ceil(log2(n)); */
	if (n == 1ULL<<log2n)
		return log2n;
	return 0ULL;
}

static inline struct GMT_FFT_WAVENUMBER * fft_get_fftwave_ptr (struct GMT_FFT_WAVENUMBER *ptr) {return (ptr);}
static inline struct GMTAPI_CTRL * fft_get_api_ptr (struct GMTAPI_CTRL *ptr)  {return (ptr);}

GMT_LOCAL uint64_t fft_get_non_symmetric_f (unsigned int *f, unsigned int n) {
	/* Return the product of the non-symmetric factors in f[]  */
	unsigned int i = 0, j = 1, retval = 1;

	if (n == 1) return (f[0]);

	while (i < n) {
		while (j < n && f[j] == f[i]) j++;
		if ((j-i)%2) retval *= f[i];
		i = j;
		j = i + 1;
	}
	if (retval == 1) retval = 0;	/* There are no non-sym factors  */
	return (retval);
}

#ifndef FSIGNIF
#define FSIGNIF			24
#endif

void gmtfft_fourt_stats (struct GMT_CTRL *GMT, unsigned int n_columns, unsigned int n_rows, unsigned int *f, double *r, size_t *s, double *t) {
	/* Find the proportional run time, t, and rms relative error, r,
	 * of a Fourier transform of size n_columns,n_rows.  Also gives s, the size
	 * of the workspace that will be needed by the transform.
	 * To use this routine for a 1-D transform, set n_rows = 1.
	 * 
	 * This is all based on the comments in Norman Brenner's code
	 * FOURT, from which our C codes are translated.
	 * Brenner says:
	 * r = 3 * pow(2, -FSIGNIF) * sum{ pow(prime_factors, 1.5) }
	 * where FSIGNIF is the smallest bit in the floating point fraction.
	 * 
	 * Let m = largest prime factor in the list of factors.
	 * Let p = product of all primes which appear an odd number of
	 * times in the list of prime factors.  Then the worksize needed
	 * s = max(m,p).  However, we know that if n is radix 2, then no
	 * work is required; yet this formula would say we need a worksize
	 * of at least 2.  So I will return s = 0 when max(m,p) = 2.
	 *
	 * I have two different versions of the comments in FOURT, with
	 * different formulae for t.  The simple formula says 
	 * 	t = n * (sum of prime factors of n).
	 * The more complicated formula gives coefficients in microsecs
	 * on a cdc3300 (ancient history, but perhaps proportional):
	 *	t = 3000 + n*(500 + 43*s2 + 68*sf + 320*nf),
	 * where s2 is the sum of all factors of 2, sf is the sum of all
	 * factors greater than 2, and nf is the number of factors != 2.
	 * We know that factors of 2 are very good to have, and indeed,
	 * Brenner's code calls different routines depending on whether
	 * the transform is of size 2 or not, so I think that the second
	 * formula is more correct, using proportions of 43:68 for 2 and
	 * non-2 factors.  So I will use the more complicated formula.
	 * However, I realize that the actual numbers are wrong for today's
	 * architectures, and the relative proportions may be wrong as well.
	 * 
	 * W. H. F. Smith, 26 February 1992.
	 *  */

	unsigned int n_factors, i, sum2, sumnot2, nnot2;
	uint64_t nonsymx, nonsymy, nonsym, storage, ntotal;
	double err_scale;

	/* Find workspace needed.  First find non_symmetric factors in n_columns, n_rows  */
	n_factors = gmt_get_prime_factors (GMT, n_columns, f);
	nonsymx = fft_get_non_symmetric_f (f, n_factors);
	n_factors = gmt_get_prime_factors (GMT, n_rows, f);
	nonsymy = fft_get_non_symmetric_f (f, n_factors);
	nonsym = MAX (nonsymx, nonsymy);

	/* Now get factors of ntotal  */
	ntotal = gmt_M_get_nm (GMT, n_columns, n_rows);
	n_factors = gmt_get_prime_factors (GMT, ntotal, f);
	storage = MAX (nonsym, f[n_factors-1]);
	*s = (storage == 2) ? 0 : storage;

	/* Now find time and error estimates */

	err_scale = 0.0;
	sum2 = sumnot2 = nnot2 = 0;
	for(i = 0; i < n_factors; i++) {
		if (f[i] == 2)
			sum2 += f[i];
		else {
			sumnot2 += f[i];
			nnot2++;
		}
		err_scale += pow ((double)f[i], 1.5);
	}
	*t = 1.0e-06 * (3000.0 + ntotal * (500.0 + 43.0 * sum2 + 68.0 * sumnot2 + 320.0 * nnot2));
	*r = err_scale * 3.0 * pow (2.0, -FSIGNIF);

	return;
}

void gmtlib_suggest_fft_dim (struct GMT_CTRL *GMT, unsigned int n_columns, unsigned int n_rows, struct GMT_FFT_SUGGESTION *fft_sug, bool do_print) {
	unsigned int f[32], xstop, ystop;
	unsigned int nx_best_t, ny_best_t;
	unsigned int nx_best_e, ny_best_e;
	unsigned int nx_best_s, ny_best_s;
	unsigned int nxg, nyg;       /* Guessed by this routine  */
	unsigned int nx2, ny2, nx3, ny3, nx5, ny5;   /* For powers  */
	size_t current_space, best_space, e_space, t_space, given_space;
	double current_time, best_time, given_time, s_time, e_time;
	double current_err, best_err, given_err, s_err, t_err;

	gmtfft_fourt_stats (GMT, n_columns, n_rows, f, &given_err, &given_space, &given_time);
	given_space += n_columns * n_rows;
	given_space *= 8;
	if (do_print)
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Data dimension\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n", n_columns, n_rows, given_time, given_err, given_space);

	best_err = s_err = t_err = given_err;
	best_time = s_time = e_time = given_time;
	best_space = t_space = e_space = given_space;
	nx_best_e = nx_best_t = nx_best_s = n_columns;
	ny_best_e = ny_best_t = ny_best_s = n_rows;

	xstop = 2 * n_columns;
	ystop = 2 * n_rows;

	for (nx2 = 2; nx2 <= xstop; nx2 *= 2) {
	  	for (nx3 = 1; nx3 <= xstop; nx3 *= 3) {
		    for (nx5 = 1; nx5 <= xstop; nx5 *= 5) {
		        nxg = nx2 * nx3 * nx5;
		        if (nxg < n_columns || nxg > xstop) continue;

		        for (ny2 = 2; ny2 <= ystop; ny2 *= 2) {
		          for (ny3 = 1; ny3 <= ystop; ny3 *= 3) {
		            for (ny5 = 1; ny5 <= ystop; ny5 *= 5) {
		                nyg = ny2 * ny3 * ny5;
		                if (nyg < n_rows || nyg > ystop) continue;

			gmtfft_fourt_stats (GMT, nxg, nyg, f, &current_err, &current_space, &current_time);
			current_space += nxg*nyg;
			current_space *= 8;
			if (current_err < best_err) {
				best_err = current_err;
				nx_best_e = nxg;
				ny_best_e = nyg;
				e_time = current_time;
				e_space = current_space;
			}
			if (current_time < best_time) {
				best_time = current_time;
				nx_best_t = nxg;
				ny_best_t = nyg;
				t_err = current_err;
				t_space = current_space;
			}
			if (current_space < best_space) {
				best_space = current_space;
				nx_best_s = nxg;
				ny_best_s = nyg;
				s_time = current_time;
				s_err = current_err;
			}

		    }
		  }
		}

	    }
	  }
	}

	if (do_print) {
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Highest speed\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n",
			nx_best_t, ny_best_t, best_time, t_err, t_space);
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Most accurate\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n",
			nx_best_e, ny_best_e, e_time, best_err, e_space);
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, " Least storage\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n",
			nx_best_s, ny_best_s, s_time, s_err, best_space);
	}
	/* Fastest solution */
	fft_sug[GMT_FFT_FAST].n_columns = nx_best_t;
	fft_sug[GMT_FFT_FAST].n_rows = ny_best_t;
	fft_sug[GMT_FFT_FAST].worksize = (t_space/8) - (nx_best_t * ny_best_t);
	fft_sug[GMT_FFT_FAST].totalbytes = t_space;
	fft_sug[GMT_FFT_FAST].run_time = best_time;
	fft_sug[GMT_FFT_FAST].rms_rel_err = t_err;
	/* Most accurate solution */
	fft_sug[GMT_FFT_ACCURATE].n_columns = nx_best_e;
	fft_sug[GMT_FFT_ACCURATE].n_rows = ny_best_e;
	fft_sug[GMT_FFT_ACCURATE].worksize = (e_space/8) - (nx_best_e * ny_best_e);
	fft_sug[GMT_FFT_ACCURATE].totalbytes = e_space;
	fft_sug[GMT_FFT_ACCURATE].run_time = e_time;
	fft_sug[GMT_FFT_ACCURATE].rms_rel_err = best_err;
	/* Least storage solution */
	fft_sug[GMT_FFT_STORAGE].n_columns = nx_best_s;
	fft_sug[GMT_FFT_STORAGE].n_rows = ny_best_s;
	fft_sug[GMT_FFT_STORAGE].worksize = (best_space/8) - (nx_best_s * ny_best_s);
	fft_sug[GMT_FFT_STORAGE].totalbytes = best_space;
	fft_sug[GMT_FFT_STORAGE].run_time = s_time;
	fft_sug[GMT_FFT_STORAGE].rms_rel_err = s_err;

	return;
}

GMT_LOCAL double fft_kx (uint64_t k, struct GMT_FFT_WAVENUMBER *K) {
	/* Return the value of kx given k,
	 * where kx = 2 pi / lambda x,
	 * and k refers to the position
	 * in the complex data array Grid->data[k].  */

	int64_t ii = (k/2)%(K->nx2);
	if (ii > (K->nx2)/2) ii -= (K->nx2);
	return (ii * K->delta_kx);
}

GMT_LOCAL double fft_ky (uint64_t k, struct GMT_FFT_WAVENUMBER *K) {
	/* Return the value of ky given k,
	 * where ky = 2 pi / lambda y,
	 * and k refers to the position
	 * in the complex data array Grid->data[k].  */

	int64_t jj = (k/2)/(K->nx2);
	if (jj > (K->ny2)/2) jj -= (K->ny2);
	return (jj * K->delta_ky);
}

GMT_LOCAL double fft_kr (uint64_t k, struct GMT_FFT_WAVENUMBER *K) {
	/* Return the value of sqrt(kx*kx + ky*ky),
	 * where k refers to the position
	 * in the complex data array Grid->data[k].  */

	return (hypot (fft_kx (k, K), fft_ky (k, K)));
}

double gmt_fft_get_wave (uint64_t k, struct GMT_FFT_WAVENUMBER *K) {
	/* Return the value of kx, ky. or kr,
	 * where k refers to the position
	 * in the complex data array Grid->data[k].
	 * GMT_fft_init sets the pointer */

	return (K->k_ptr (k, K));
}

double gmt_fft_any_wave (uint64_t k, unsigned int mode, struct GMT_FFT_WAVENUMBER *K) {
	/* Lets you specify which wavenumber you want */
	double wave = 0.0;

	switch (mode) {	/* Select which wavenumber we need */
		case GMT_FFT_K_IS_KX: wave = fft_kx (k, K); break;
		case GMT_FFT_K_IS_KY: wave = fft_ky (k, K); break;
		case GMT_FFT_K_IS_KR: wave = fft_kr (k, K); break;
	}
	return (wave);
}

int gmt_fft_set_wave (struct GMT_CTRL *GMT, unsigned int mode, struct GMT_FFT_WAVENUMBER *K) {
	/* Change wavenumber selection */
	switch (mode) {	/* Select which wavenumber we need */
		case GMT_FFT_K_IS_KX: K->k_ptr = fft_kx; break;
		case GMT_FFT_K_IS_KY: K->k_ptr = fft_ky; break;
		case GMT_FFT_K_IS_KR: K->k_ptr = fft_kr; break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad mode selected (%u) - exit\n", mode);
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
			break;
	}
	return GMT_OK;
}

/*! . */
double GMT_FFT_Wavenumber (void *V_API, uint64_t k, unsigned int mode, void *v_K) {
	/* Lets you specify which 1-D or 2-D wavenumber you want */
	struct GMT_FFT_WAVENUMBER *K = fft_get_fftwave_ptr (v_K);
	gmt_M_unused(V_API);
	if (K->dim == 2) return (gmt_fft_any_wave (k, mode, K));
	else return (fft_kx (k, K));
}

#ifdef HAVE_FFTW3F

#include <fftw3.h>
#ifdef _WIN32
#include <winsock.h>
#endif

#define FFTWF_WISDOM_FILENAME "fftwf_wisdom"

GMT_LOCAL char *fft_fftwf_wisdom_filename (struct GMT_CTRL *GMT) {
	static char wisdom_file[PATH_MAX+256] = "\0";
	char hostname[257];
	if (*wisdom_file == '\0') { /* wisdom_file has not been set yet */
		if (GMT->session.CACHEDIR == NULL || access (GMT->session.CACHEDIR, R_OK|W_OK|X_OK))
			/* CACHEDIR does not exist, or not writable */
			return NULL;
		else {
			/* create wisdom file in CACHEDIR */
			strncpy (wisdom_file, GMT->session.CACHEDIR, PATH_MAX);
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
GMT_LOCAL void fft_fftwf_import_wisdom_from_filename (struct GMT_CTRL *GMT) {
	static bool already_imported = false;
	char *filenames[3], **filename = filenames;
	int status;

	if (already_imported) /* nothing to do */
		return;

	fftwf_import_system_wisdom (); /* read wisdom from implementation-defined standard file */

	/* Initialize filenames */
	filenames[0] = FFTWF_WISDOM_FILENAME; /* 1st try importing wisdom from file in current dir */
	filenames[1] = fft_fftwf_wisdom_filename(GMT); /* 2nd try wisdom file in CACHEDIR */
	filenames[2] = NULL; /* end of array */

	while (*filename != NULL) {
		if (!access (*filename, R_OK)) {
			status = fftwf_import_wisdom_from_filename (*filename);
			if (status)
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Imported FFTW Wisdom from file: %s\n", *filename);
			else
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Importing FFTW Wisdom from file failed: %s\n", *filename);
		}
		++filename; /* advance to next file in array */
	}

	already_imported = true;
}

/* Wrapper around fftwf_export_wisdom_to_filename */
GMT_LOCAL void fft_fftwf_export_wisdom_to_filename (struct GMT_CTRL *GMT) {
	char *filename = fft_fftwf_wisdom_filename(GMT);
	int status;

	if (filename == NULL)
		/* CACHEDIR does not exist, write wisdom to file in current directory */
		filename = FFTWF_WISDOM_FILENAME;

	status = fftwf_export_wisdom_to_filename (filename);
	if (status)
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Exported FFTW Wisdom to file: %s\n", filename);
	else
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Exporting FFTW Wisdom to file failed: %s\n", filename);
}

GMT_LOCAL fftwf_plan gmt_fftwf_plan_dft(struct GMT_CTRL *GMT, unsigned n_rows, unsigned n_columns, fftwf_complex *data, int direction) {
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

	sign = direction == GMT_FFT_FWD ? FFTW_FORWARD : FFTW_BACKWARD;
	cin  = data;
	cout = cin; /* in-place transform */

	if (GMT->current.setting.fftw_plan != FFTW_ESTIMATE) {
		fft_fftwf_import_wisdom_from_filename (GMT);
		if (n_rows == 0) /* 1d DFT */
			plan = fftwf_plan_dft_1d(n_columns, cin, cout, sign, FFTW_WISDOM_ONLY | GMT->current.setting.fftw_plan);
		else /* 2d DFT */
			plan = fftwf_plan_dft_2d(n_rows, n_columns, cin, cout, sign, FFTW_WISDOM_ONLY | GMT->current.setting.fftw_plan);
		if (plan == NULL) {
			/* No Wisdom available
			 * Need extra memory to prevent overwriting data while planning */
			fftwf_complex *in_place_tmp = fftwf_malloc (2 * (n_rows == 0 ? 1 : n_rows) * n_columns * sizeof(gmt_grdfloat));
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Generating new FFTW Wisdom, be patient...\n");
			if (n_rows == 0) /* 1d DFT */
				plan = fftwf_plan_dft_1d(n_columns, in_place_tmp, in_place_tmp, sign, GMT->current.setting.fftw_plan);
			else /* 2d DFT */
				plan = fftwf_plan_dft_2d(n_rows, n_columns, in_place_tmp, in_place_tmp, sign, GMT->current.setting.fftw_plan);
			fftwf_destroy_plan(plan); /* deallocate plan */
			plan = NULL;
			fftwf_free (in_place_tmp);
			/* Save new Wisdom */
			fft_fftwf_export_wisdom_to_filename (GMT);
		}
		else
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Using preexisting FFTW Wisdom.\n");
	} /* GMT->current.setting.fftw_plan != FFTW_ESTIMATE */
	else
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Picking a (probably sub-optimal) FFTW plan quickly.\n");

	if (plan == NULL) { /* If either FFTW_ESTIMATE or new Wisdom generated */
		if (n_rows == 0) /* 1d DFT */
			plan = fftwf_plan_dft_1d(n_columns, cin, cout, sign, GMT->current.setting.fftw_plan);
		else /* 2d DFT */
			plan = fftwf_plan_dft_2d(n_rows, n_columns, cin, cout, sign, GMT->current.setting.fftw_plan);
	}

	if (plan == NULL) { /* There was a problem creating a plan */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Could not create FFTW plan.\n");
		GMT_exit (GMT, GMT_ARG_IS_NULL); return NULL;
	}

	return plan;
}

GMT_LOCAL int fft_1d_fftwf (struct GMT_CTRL *GMT, gmt_grdfloat *data, unsigned int n, int direction, unsigned int mode) {
	fftwf_plan plan = NULL;
	gmt_M_unused(mode);

	/* Generate FFTW plan for complex 1d DFT */
	plan = gmt_fftwf_plan_dft(GMT, 0, n, (fftwf_complex*)data, direction);
	fftwf_execute(plan);      /* do transform */
	fftwf_destroy_plan(plan); /* deallocate plan */

	return GMT_NOERROR;
}

GMT_LOCAL int fft_2d_fftwf (struct GMT_CTRL *GMT, gmt_grdfloat *data, unsigned int n_columns, unsigned int n_rows, int direction, unsigned int mode) {
	fftwf_plan plan = NULL;
	gmt_M_unused(mode);

	/* Generate FFTW plan for complex 2d DFT */
	plan = gmt_fftwf_plan_dft(GMT, n_rows, n_columns, (fftwf_complex*)data, direction);
	fftwf_execute(plan);      /* do transform */
	fftwf_destroy_plan(plan); /* deallocate plan */

	return GMT_NOERROR;
}

#endif /* HAVE_FFTW3F */

#ifdef __APPLE__ /* Accelerate framework */

GMT_LOCAL void fft_1d_vDSP_reset (struct GMT_FFT_HIDDEN *Z) {
	if (Z->setup_1d) {	/* Free single-precision FFT data structure and arrays */
		vDSP_destroy_fftsetup (Z->setup_1d);
		gmt_M_str_free (Z->dsp_split_complex_1d.realp);
		gmt_M_str_free (Z->dsp_split_complex_1d.imagp);
	}
}

GMT_LOCAL int fft_1d_vDSP (struct GMT_CTRL *GMT, gmt_grdfloat *data, unsigned int n, int direction, unsigned int mode) {
	FFTDirection fft_direction = direction == GMT_FFT_FWD ?
			kFFTDirection_Forward : kFFTDirection_Inverse;
	DSPComplex *dsp_complex = (DSPComplex *)data;

	/* Base 2 exponent that specifies the largest power of
	 * two that can be processed by fft: */
	vDSP_Length log2n = radix2 (n);
	gmt_M_unused(mode);

	if (log2n == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Need Radix-2 input try: %u [n]\n", 1U<<propose_radix2 (n));
		return -1;
	}

	if (GMT->current.fft.n_1d != n) {	/* Must update the FFT setup arrays */
		/* Build data structure that contains precalculated data for use by
		 * single-precision FFT functions: */
		fft_1d_vDSP_reset (&GMT->current.fft);
		GMT->current.fft.setup_1d = vDSP_create_fftsetup (log2n, kFFTRadix2);
		GMT->current.fft.dsp_split_complex_1d.realp = calloc (n, sizeof(gmt_grdfloat));
		GMT->current.fft.dsp_split_complex_1d.imagp = calloc (n, sizeof(gmt_grdfloat));
		if (GMT->current.fft.dsp_split_complex_1d.realp == NULL || GMT->current.fft.dsp_split_complex_1d.imagp == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to allocate dsp_split_complex array of length %u\n", n);
			return -1; /* out of memory */
		}
		GMT->current.fft.n_1d = n;
	}
	vDSP_ctoz (dsp_complex, 2, &GMT->current.fft.dsp_split_complex_1d, 1, n);

	vDSP_fft_zip (GMT->current.fft.setup_1d, &GMT->current.fft.dsp_split_complex_1d, 1, log2n, fft_direction);

	vDSP_ztoc (&GMT->current.fft.dsp_split_complex_1d, 1, dsp_complex, 2, n);

	return GMT_NOERROR;
}

GMT_LOCAL void fft_2d_vDSP_reset (struct GMT_FFT_HIDDEN *Z) {
	if (Z->setup_2d) {	/* Free single-precision 2D FFT data structure and arrays */
		vDSP_destroy_fftsetup (Z->setup_2d);
		gmt_M_str_free (Z->dsp_split_complex_2d.realp);
		gmt_M_str_free (Z->dsp_split_complex_2d.imagp);
	}
}

GMT_LOCAL int fft_2d_vDSP (struct GMT_CTRL *GMT, gmt_grdfloat *data, unsigned int n_columns, unsigned int n_rows, int direction, unsigned int mode) {
	FFTDirection fft_direction = direction == GMT_FFT_FWD ?
			kFFTDirection_Forward : kFFTDirection_Inverse;
	DSPComplex *dsp_complex = (DSPComplex *)data;

	/* Base 2 exponent that specifies the largest power of
	 * two that can be processed by fft: */
	vDSP_Length log2nx = radix2 (n_columns);
	vDSP_Length log2ny = radix2 (n_rows);
	unsigned int n_xy = n_columns * n_rows;
	gmt_M_unused(mode);

	if (log2nx == 0 || log2ny == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Need Radix-2 input try: %u/%u [n_columns/n_rows]\n",
				1U<<propose_radix2 (n_columns), 1U<<propose_radix2 (n_rows));
		return -1;
	}

	if (GMT->current.fft.n_2d != n_xy) {	/* Must update the 2-D FFT setup arrays */
		/* Build data structure that contains precalculated data for use by
	 	* single-precision FFT functions: */
		fft_2d_vDSP_reset (&GMT->current.fft);
		GMT->current.fft.setup_2d = vDSP_create_fftsetup (MAX (log2nx, log2ny), kFFTRadix2);
		GMT->current.fft.dsp_split_complex_2d.realp = calloc (n_xy, sizeof(gmt_grdfloat));
		GMT->current.fft.dsp_split_complex_2d.imagp = calloc (n_xy, sizeof(gmt_grdfloat));
		if (GMT->current.fft.dsp_split_complex_2d.realp == NULL || GMT->current.fft.dsp_split_complex_2d.imagp == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to allocate dsp_split_complex array of length %u\n", n_xy);
			return -1; /* out of memory */
		}
		GMT->current.fft.n_2d = n_xy;
	}
	vDSP_ctoz (dsp_complex, 2, &GMT->current.fft.dsp_split_complex_2d, 1, n_xy);

	/* complex: */
	/* PW note 10/26/2014: We used to pass log2ny, log2nx to vDSP_fft2d_zipbut that gave bad results for n_columns != n_rows.
	 * I assume this is because Accelerate expects columns but we pass rows. Now matches KISS, FFTW, etc. */
	vDSP_fft2d_zip (GMT->current.fft.setup_2d, &GMT->current.fft.dsp_split_complex_2d, 1, 0, log2nx, log2ny, fft_direction);
	/* real:
	vDSP_fft2d_zrip (setup, &GMT->current.fft.dsp_split_complex_2d, 1, 0, log2nx, log2ny, fft_direction); */

	vDSP_ztoc (&GMT->current.fft.dsp_split_complex_2d, 1, dsp_complex, 2, n_xy);

	return GMT_NOERROR;
}
#endif /* APPLE Accelerate framework */

/* Kiss FFT */

#include "kiss_fft/kiss_fftnd.h"

GMT_LOCAL int fft_1d_kiss (struct GMT_CTRL *GMT, gmt_grdfloat *data, unsigned int n, int direction, unsigned int mode) {
	kiss_fft_cpx *fin, *fout;
	kiss_fft_cfg config;
	gmt_M_unused(GMT); gmt_M_unused(mode);

	/* Initialize a FFT (or IFFT) config/state data structure */
	config = kiss_fft_alloc(n, direction == GMT_FFT_INV, NULL, NULL);
	fin = fout = (kiss_fft_cpx *)data;
	kiss_fft (config, fin, fout); /* do transform */
	gmt_M_str_free (config); /* Free config data structure */

	return GMT_NOERROR;
}

GMT_LOCAL int fft_2d_kiss (struct GMT_CTRL *GMT, gmt_grdfloat *data, unsigned int n_columns, unsigned int n_rows, int direction, unsigned int mode) {
	const int dim[2] = {n_rows, n_columns}; /* dimensions of fft */
	const int dimcount = 2;      /* number of dimensions */
	kiss_fft_cpx *fin, *fout;
	kiss_fftnd_cfg config;
	gmt_M_unused(GMT); gmt_M_unused(mode);

	/* Initialize a FFT (or IFFT) config/state data structure */
	config = kiss_fftnd_alloc (dim, dimcount, direction == GMT_FFT_INV, NULL, NULL);

	fin = fout = (kiss_fft_cpx *)data;
	kiss_fftnd (config, fin, fout); /* do transform */
	gmt_M_str_free (config); /* Free config data structure */

	return GMT_NOERROR;
}


GMT_LOCAL int fft_brenner_fourt_f (gmt_grdfloat *data, int *nn, int *ndim, int *ksign, int *iform, gmt_grdfloat *work) {

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

    static gmt_grdfloat theta, oldsi, tempi, oldsr, sinth, difi, difr, sumi, sumr, tempr, twopi;
    static gmt_grdfloat wstpi, wstpr, twowr, wi, wr, u1i, w2i, w3i, u2i, u3i, u4i, t2i, u1r;
    static gmt_grdfloat u2r, u3r, w2r, w3r, u4r, t2r, t3r, t3i, t4r, t4i;
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
    wr = wi = wstpr = wstpi = (gmt_grdfloat)0.0;
    twopi = (gmt_grdfloat)6.283185307;
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
	theta = -twopi * (gmt_grdfloat) non2 / (gmt_grdfloat) (mmax <<  2);
	if (*ksign >= 0) {
	    goto L390;
	} else {
	    goto L400;
	}
L390:
	theta = -theta;
L400:
	sincos ((double)theta, &wid, &wrd);
	wr = (gmt_grdfloat)wrd;
	wi = (gmt_grdfloat)wid;
	wstpr = (gmt_grdfloat)-2.0 * wi * wi;
	wstpi = (gmt_grdfloat)2.0 * wr * wi;
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
	    w2i = (gmt_grdfloat)(wr * 2.0 * wi);
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
	    theta = -twopi * (gmt_grdfloat) (j2 - 1) / (gmt_grdfloat)  np2;
	    if (*ksign >= 0) {
		goto L620;
	    } else {
		goto L625;
	    }
L620:
	    theta = -theta;
L625:
	    sinth = (gmt_grdfloat)sin((double)(0.5 * theta));
	    wstpr = sinth * (gmt_grdfloat)(-2. * sinth);
	    wstpi = (gmt_grdfloat)sin((double)theta);
	    wr = wstpr + (gmt_grdfloat)1.0;
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
	theta = -twopi / (gmt_grdfloat) ifact[if_ - 1];
	if (*ksign >= 0) {
	    goto L645;
	} else {
	    goto L650;
	}
L645:
	theta = -theta;
L650:
	sinth = (gmt_grdfloat)sin((double)(0.5 * theta));
	wstpr = sinth * (gmt_grdfloat)(-2. * sinth);
	wstpi = (gmt_grdfloat)sin((double)theta);
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
			    sumr = (gmt_grdfloat)0.0;
			    sumi = (gmt_grdfloat)0.0;
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
			    oldsr = (gmt_grdfloat)0.0;
			    oldsi = (gmt_grdfloat)0.0;
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
		    wr = wstpr + (gmt_grdfloat)1.0;
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
	theta = -twopi / (gmt_grdfloat) n;
	if (*ksign >= 0) {
	    goto L702;
	} else {
	    goto L703;
	}
L702:
	theta = -theta;
L703:
	sinth = (gmt_grdfloat)sin((double)(0.5 * theta));
	wstpr = sinth * (gmt_grdfloat)(-2. * sinth);
	wstpi = (gmt_grdfloat)sin((double)theta);
	wr = wstpr + (gmt_grdfloat)1.0;
	wi = wstpi;
	imin = 3;
	jmin = (nhalf << 1) - 1;
	goto L725;
L710:
	j = jmin;
	i__4 = ntot;
	i__3 = np2;
	for (i = imin; i__3 < 0 ? i >= i__4 : i <= i__4; i += i__3) {
	    sumr = (gmt_grdfloat)0.5 * (data[i] + data[j]);
	    sumi = (gmt_grdfloat)0.5 * (data[i + 1] + data[j + 1]);
	    difr = (gmt_grdfloat)0.5 * (data[i] - data[j]);
	    difi = (gmt_grdfloat)0.5 * (data[i + 1] - data[j + 1]);
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
	data[j + 1] = (gmt_grdfloat)0.0;
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
	data[j + 1] = (gmt_grdfloat)0.0;
	imax = imin;
	goto L745;
L780:
	data[1] += data[2];
	data[2] = (gmt_grdfloat)0.0;
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

GMT_LOCAL size_t fft_brenner_worksize (struct GMT_CTRL *GMT, unsigned int n_columns, unsigned int n_rows) {
        /* Find the size of the workspace that will be needed by the transform.
         * To use this routine for a 1-D transform, set n_rows = 1.
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
	
        /* Find workspace needed.  First find non_symmetric factors in n_columns, n_rows  */
        n_factors = gmt_get_prime_factors (GMT, n_columns, f);
        nonsymx = (unsigned int)fft_get_non_symmetric_f (f, n_factors);
        n_factors = gmt_get_prime_factors (GMT, n_rows, f);
        nonsymy = (unsigned int)fft_get_non_symmetric_f (f, n_factors);
        nonsym = MAX (nonsymx, nonsymy);
	
        /* Now get factors of ntotal  */
        ntotal = gmt_M_get_nm (GMT, n_columns, n_rows);
        n_factors = gmt_get_prime_factors (GMT, ntotal, f);
        storage = (n_factors > 0) ? MAX (nonsym, f[n_factors-1]) : nonsym;
        if (storage != 2) storage *= 2;
        if (storage < n_columns) storage = n_columns;
        if (storage < n_rows) storage = n_rows;
        return (2 * storage);
}

/* C-callable wrapper for fft_brenner_fourt_f */
GMT_LOCAL int fft_1d_brenner (struct GMT_CTRL *GMT, gmt_grdfloat *data, unsigned int n, int direction, unsigned int mode) {
        /* void GMT_fourt (struct GMT_CTRL *GMT, gmt_grdfloat *data, int *nn, int ndim, int ksign, int iform, gmt_grdfloat *work) */
        /* Data array */
        /* Dimension array */
        /* Number of dimensions */
        /* Forward(-1) or Inverse(+1) */
        /* Real(0) or complex(1) data */
        /* Work array */
	
        int ksign, ndim = 1, n_signed = n, kmode = mode;
        size_t work_size = 0;
        gmt_grdfloat *work = NULL;
	
        ksign = (direction == GMT_FFT_INV) ? +1 : -1;
        if ((work_size = fft_brenner_worksize (GMT, n, 1))) work = gmt_M_memory (GMT, NULL, work_size, gmt_grdfloat);
        (void) fft_brenner_fourt_f (data, &n_signed, &ndim, &ksign, &kmode, work);
        gmt_M_free (GMT, work);	
        return (GMT_OK);
}
	
GMT_LOCAL int fft_2d_brenner (struct GMT_CTRL *GMT, gmt_grdfloat *data, unsigned int n_columns, unsigned int n_rows, int direction, unsigned int mode) {
        /* Data array */
        /* Dimension array */
        /* Number of dimensions */
        /* Forward(-1) or Inverse(+1) */
        /* Real(0) or complex(1) data */
        /* Work array */

        int ksign, ndim = 2, nn[2] = {n_columns, n_rows}, kmode = mode;
        size_t work_size = 0;
        gmt_grdfloat *work = NULL;

        ksign = (direction == GMT_FFT_INV) ? +1 : -1;
        if ((work_size = fft_brenner_worksize (GMT, n_columns, n_rows))) work = gmt_M_memory (GMT, NULL, work_size, gmt_grdfloat);
        GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Brenner_fourt_ work size = %" PRIuS "\n", work_size);
        (void) fft_brenner_fourt_f (data, nn, &ndim, &ksign, &kmode, work);
        gmt_M_free (GMT, work);
        return (GMT_OK);
}

GMT_LOCAL int fft_1d_selection (struct GMT_CTRL *GMT, uint64_t n) {
	/* Returns the most suitable 1-D FFT for the job - or the one requested via GMT_FFT */
	if (GMT->current.setting.fft != k_fft_auto) {
		/* Specific selection requested */
		if (GMT->session.fft1d[GMT->current.setting.fft])
			return GMT->current.setting.fft; /* User defined FFT */
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Desired FFT Algorithm (%s) not configured - choosing suitable alternative.\n", GMT_fft_algo[GMT->current.setting.fft]);
	}
	/* Here we want automatic selection from available candidates */
	if (GMT->session.fft1d[k_fft_accelerate] && radix2 (n))
		return k_fft_accelerate; /* Use if Radix-2 under OS/X */
	if (GMT->session.fft1d[k_fft_fftw])
		return k_fft_fftw;
	return k_fft_kiss; /* Default/fallback general-purpose FFT */
}

GMT_LOCAL int fft_2d_selection (struct GMT_CTRL *GMT, unsigned int n_columns, unsigned int n_rows) {
	/* Returns the most suitable 2-D FFT for the job - or the one requested via GMT_FFT */
	if (GMT->current.setting.fft != k_fft_auto) {
		/* Specific selection requested */
		if (GMT->session.fft2d[GMT->current.setting.fft])
			return GMT->current.setting.fft; /* User defined FFT */
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Desired FFT Algorithm (%s) not configured - choosing suitable alternative.\n", GMT_fft_algo[GMT->current.setting.fft]);
	}
	/* Here we want automatic selection from available candidates */
	if (GMT->session.fft2d[k_fft_accelerate] && radix2 (n_columns) && radix2 (n_rows))
		return k_fft_accelerate; /* Use if Radix-2 under OS/X */
	if (GMT->session.fft2d[k_fft_fftw])
		return k_fft_fftw;
	return k_fft_kiss; /* Default/fallback general-purpose FFT */
}

int GMT_FFT_1D (void *V_API, gmt_grdfloat *data, uint64_t n, int direction, unsigned int mode) {
	/* data is an array of length n (or 2*n for complex) data points
	 * n is the number of data points
	 * direction is either GMT_FFT_FWD (forward) or GMT_FFT_INV (inverse)
	 * mode is either GMT_FFT_REAL or GMT_FFT_COMPLEX
	 */
	int status, use;
	struct GMTAPI_CTRL *API = fft_get_api_ptr (V_API);
	struct GMT_CTRL *GMT = API->GMT;
	assert (mode == GMT_FFT_COMPLEX); /* GMT_FFT_REAL not implemented yet */
	use = fft_1d_selection (GMT, n);
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "1-D FFT using %s\n", GMT_fft_algo[use]);
	status = GMT->session.fft1d[use] (GMT, data, (unsigned int)n, direction, mode);
	if (direction == GMT_FFT_INV) {	/* Undo the 2/nm factor */
		uint64_t nm = 2ULL * n;
		gmt_scale_and_offset_f (GMT, data, nm, 2.0 / nm, 0);
	}
	return status;
}

int GMT_FFT_2D (void *V_API, gmt_grdfloat *data, unsigned int n_columns, unsigned int n_rows, int direction, unsigned int mode) {
	/* data is an array of length n_columns*n_rows (or 2*n_columns*n_rows for complex) data points
	 * n_columns, n_rows is the number of data nodes
	 * direction is either GMT_FFT_FWD (forward) or GMT_FFT_INV (inverse)
	 * mode is either GMT_FFT_REAL or GMT_FFT_COMPLEX
	 */
	int status, use;
	struct GMTAPI_CTRL *API = fft_get_api_ptr (V_API);
	struct GMT_CTRL *GMT = API->GMT;
	assert (mode == GMT_FFT_COMPLEX); /* GMT_FFT_REAL not implemented yet */
	use = fft_2d_selection (GMT, n_columns, n_rows);
	
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "2-D FFT using %s\n", GMT_fft_algo[use]);
	status = GMT->session.fft2d[use] (GMT, data, n_columns, n_rows, direction, mode);
	if (direction == GMT_FFT_INV) {	/* Undo the 2/nm factor */
		uint64_t nm = 2ULL * n_columns * n_rows;
		gmt_scale_and_offset_f (GMT, data, nm, 2.0 / nm, 0);
	}
	return status;
}

void gmt_fft_initialization (struct GMT_CTRL *GMT) {
	/* Called by gmt_begin and sets up pointers to the available FFT calls */
	GMT->current.setting.fftw_plan = FFTW_ESTIMATE; /* default planner flag [only accessed if FFTW is compiled in] */
#if defined HAVE_FFTW3F_THREADS
	int n_cpu = gmtlib_get_num_processors();

	if (n_cpu > 1 && !GMT->current.setting.fftwf_threads) {
		/* one-time initialization required to use FFTW3 threads */
		if ( fftwf_init_threads() ) {
			fftwf_plan_with_nthreads(n_cpu);
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Initialize FFTW with %d threads.\n", n_cpu);
		}
	}

#endif /* HAVE_FFTW3_THREADS */

	/* Start with nothing */
	memset (GMT->session.fft1d, k_n_fft_algorithms, sizeof(void*));
	memset (GMT->session.fft2d, k_n_fft_algorithms, sizeof(void*));

#ifdef __APPLE__
	/* OS X Accelerate Framework */
	GMT->session.fft1d[k_fft_accelerate] = &fft_1d_vDSP;
	GMT->session.fft2d[k_fft_accelerate] = &fft_2d_vDSP;
#endif
#ifdef HAVE_FFTW3F
	/* single precision FFTW3 */
	GMT->session.fft1d[k_fft_fftw] = &fft_1d_fftwf;
	GMT->session.fft2d[k_fft_fftw] = &fft_2d_fftwf;
#endif /* HAVE_FFTW3F */
	/* Kiss FFT is the integrated fallback */
	GMT->session.fft1d[k_fft_kiss] = &fft_1d_kiss;
	GMT->session.fft2d[k_fft_kiss] = &fft_2d_kiss;
	/* Brenner FFT is the legacy fallback */
	GMT->session.fft1d[k_fft_brenner] = &fft_1d_brenner;
	GMT->session.fft2d[k_fft_brenner] = &fft_2d_brenner;
}

void gmt_fft_cleanup (struct GMT_CTRL *GMT) {
	/* Called by gmt_end */
#ifndef __APPLE__
	gmt_M_unused(GMT);
#endif
#if defined HAVE_FFTW3F_THREADS
	fftwf_cleanup_threads(); /* clean resources allocated internally by FFTW */
#endif
#ifdef __APPLE__ /* Accelerate framework */
	fft_1d_vDSP_reset (&GMT->current.fft);
	fft_2d_vDSP_reset (&GMT->current.fft);
#endif
}
