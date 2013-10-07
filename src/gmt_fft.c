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
 *  Superceeded Norman Brenner's ancient Cooley-Tukey FFT implementation
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
 * These prototypes are declared in gmt.h and coded in gmt_api.c
 *
 * GMT_FFT_Option     : Lets user code display FFT options
 * GMT_FFT_Parse      : Allows parsing of user option for the FFT settings
 * GMT_FFT_Create     : Initializes the 1-D or 2-D FFT machinery, preps the table/grid.
 * GMT_FFT            : 1-D or 2-D FFT
 * GMT_FFT_Wavenumber : Return any wavenumber given index
 * GMT_FFT_Destroy    : Frees the FFT machinery.
 *
 */

#include "gmt_dev.h"
#include "gmt_internals.h"

static inline struct GMTAPI_CTRL * gmt_get_api_ptr (struct GMTAPI_CTRL *ptr) {return (ptr);}

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

/* first 2 cols from table III of Singleton's paper on fft.... */
#define N_SINGLETON_LIST	117
int Singleton_list[N_SINGLETON_LIST] = {
	64,72,75,80,81,90,96,100,108,120,125,128,135,144,150,160,162,180,192,200,
	216,225,240,243,250,256,270,288,300,320,324,360,375,384,400,405,432,450,480,
	486,500,512,540,576,600,625,640,648,675,720,729,750,768,800,810,864,900,960,
	972,1000,1024,1080,1125,1152,1200,1215,1250,1280,1296,1350,1440,1458,1500,
	1536,1600,1620,1728,1800,1875,1920,1944,2000,2025,2048,2160,2187,2250,2304,
	2400,2430,2500,2560,2592,2700,2880,2916,3000,3072,3125,3200,3240,3375,3456,
	3600,3645,3750,3840,3888,4000,4096,4320,4374,4500,4608,4800,4860,5000};

void gmt_fft_Singleton_list (struct GMTAPI_CTRL *API) {
	unsigned int k;
	char message[GMT_LEN16] = {""};
	GMT_Message (API, GMT_TIME_NONE, "\t\"Good\" numbers for FFT dimensions [Singleton, 1967]:\n");
	for (k = 0; k < N_SINGLETON_LIST; k++) {
		sprintf (message, "\t%d", Singleton_list[k]);
		if ((k+1) % 10 == 0 || k == (N_SINGLETON_LIST-1)) strcat (message, "\n");
		GMT_Message (API, GMT_TIME_NONE, message);
	}
}

uint64_t get_non_symmetric_f (unsigned int *f, unsigned int n)
{
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

void gmt_fourt_stats (struct GMT_CTRL *GMT, unsigned int nx, unsigned int ny, unsigned int *f, double *r, size_t *s, double *t)
{
	/* Find the proportional run time, t, and rms relative error, r,
	 * of a Fourier transform of size nx,ny.  Also gives s, the size
	 * of the workspace that will be needed by the transform.
	 * To use this routine for a 1-D transform, set ny = 1.
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

	/* Find workspace needed.  First find non_symmetric factors in nx, ny  */
	n_factors = GMT_get_prime_factors (GMT, nx, f);
	nonsymx = get_non_symmetric_f (f, n_factors);
	n_factors = GMT_get_prime_factors (GMT, ny, f);
	nonsymy = get_non_symmetric_f (f, n_factors);
	nonsym = MAX (nonsymx, nonsymy);

	/* Now get factors of ntotal  */
	ntotal = GMT_get_nm (GMT, nx, ny);
	n_factors = GMT_get_prime_factors (GMT, ntotal, f);
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

void GMT_suggest_fft_dim (struct GMT_CTRL *GMT, unsigned int nx, unsigned int ny, struct GMT_FFT_SUGGESTION *fft_sug, bool do_print)
{
	unsigned int f[32], xstop, ystop;
	unsigned int nx_best_t, ny_best_t;
	unsigned int nx_best_e, ny_best_e;
	unsigned int nx_best_s, ny_best_s;
	unsigned int nxg, nyg;       /* Guessed by this routine  */
	unsigned int nx2, ny2, nx3, ny3, nx5, ny5;   /* For powers  */
	size_t current_space, best_space, e_space, t_space, given_space;
	double current_time, best_time, given_time, s_time, e_time;
	double current_err, best_err, given_err, s_err, t_err;

	gmt_fourt_stats (GMT, nx, ny, f, &given_err, &given_space, &given_time);
	given_space += nx * ny;
	given_space *= 8;
	if (do_print)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Data dimension\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n", nx, ny, given_time, given_err, given_space);

	best_err = s_err = t_err = given_err;
	best_time = s_time = e_time = given_time;
	best_space = t_space = e_space = given_space;
	nx_best_e = nx_best_t = nx_best_s = nx;
	ny_best_e = ny_best_t = ny_best_s = ny;

	xstop = 2 * nx;
	ystop = 2 * ny;

	for (nx2 = 2; nx2 <= xstop; nx2 *= 2) {
	  	for (nx3 = 1; nx3 <= xstop; nx3 *= 3) {
		    for (nx5 = 1; nx5 <= xstop; nx5 *= 5) {
		        nxg = nx2 * nx3 * nx5;
		        if (nxg < nx || nxg > xstop) continue;

		        for (ny2 = 2; ny2 <= ystop; ny2 *= 2) {
		          for (ny3 = 1; ny3 <= ystop; ny3 *= 3) {
		            for (ny5 = 1; ny5 <= ystop; ny5 *= 5) {
		                nyg = ny2 * ny3 * ny5;
		                if (nyg < ny || nyg > ystop) continue;

			gmt_fourt_stats (GMT, nxg, nyg, f, &current_err, &current_space, &current_time);
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
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Highest speed\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n",
			nx_best_t, ny_best_t, best_time, t_err, t_space);
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Most accurate\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n",
			nx_best_e, ny_best_e, e_time, best_err, e_space);
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Least storage\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n",
			nx_best_s, ny_best_s, s_time, s_err, best_space);
	}
	/* Fastest solution */
	fft_sug[0].nx = nx_best_t;
	fft_sug[0].ny = ny_best_t;
	fft_sug[0].worksize = (t_space/8) - (nx_best_t * ny_best_t);
	fft_sug[0].totalbytes = t_space;
	fft_sug[0].run_time = best_time;
	fft_sug[0].rms_rel_err = t_err;
	/* Most accurate solution */
	fft_sug[1].nx = nx_best_e;
	fft_sug[1].ny = ny_best_e;
	fft_sug[1].worksize = (e_space/8) - (nx_best_e * ny_best_e);
	fft_sug[1].totalbytes = e_space;
	fft_sug[1].run_time = e_time;
	fft_sug[1].rms_rel_err = best_err;
	/* Least storage solution */
	fft_sug[2].nx = nx_best_s;
	fft_sug[2].ny = ny_best_s;
	fft_sug[2].worksize = (best_space/8) - (nx_best_s * ny_best_s);
	fft_sug[2].totalbytes = best_space;
	fft_sug[2].run_time = s_time;
	fft_sug[2].rms_rel_err = s_err;

	return;
}

double GMT_fft_kx (uint64_t k, struct GMT_FFT_WAVENUMBER *K)
{
	/* Return the value of kx given k,
	 * where kx = 2 pi / lambda x,
	 * and k refers to the position
	 * in the complex data array Grid->data[k].  */

	int64_t ii = (k/2)%(K->nx2);
	if (ii > (K->nx2)/2) ii -= (K->nx2);
	return (ii * K->delta_kx);
}

double GMT_fft_ky (uint64_t k, struct GMT_FFT_WAVENUMBER *K)
{
	/* Return the value of ky given k,
	 * where ky = 2 pi / lambda y,
	 * and k refers to the position
	 * in the complex data array Grid->data[k].  */

	int64_t jj = (k/2)/(K->nx2);
	if (jj > (K->ny2)/2) jj -= (K->ny2);
	return (jj * K->delta_ky);
}

double GMT_fft_kr (uint64_t k, struct GMT_FFT_WAVENUMBER *K)
{
	/* Return the value of sqrt(kx*kx + ky*ky),
	 * where k refers to the position
	 * in the complex data array Grid->data[k].  */

	return (hypot (GMT_fft_kx (k, K), GMT_fft_ky (k, K)));
}

double GMT_fft_get_wave (uint64_t k, struct GMT_FFT_WAVENUMBER *K)
{
	/* Return the value of kx, ky. or kr,
	 * where k refers to the position
	 * in the complex data array Grid->data[k].
	 * GMT_fft_init sets the pointer */

	return (K->k_ptr (k, K));
}

double GMT_fft_any_wave (uint64_t k, unsigned int mode, struct GMT_FFT_WAVENUMBER *K)
{	/* Lets you specify which wavenumber you want */
	double wave = 0.0;

	switch (mode) {	/* Select which wavenumber we need */
		case GMT_FFT_K_IS_KX: wave = GMT_fft_kx (k, K); break;
		case GMT_FFT_K_IS_KY: wave = GMT_fft_ky (k, K); break;
		case GMT_FFT_K_IS_KR: wave = GMT_fft_kr (k, K); break;
	}
	return (wave);
}

int GMT_fft_set_wave (struct GMT_CTRL *GMT, unsigned int mode, struct GMT_FFT_WAVENUMBER *K)
{
	/* Change wavenumber selection */
	switch (mode) {	/* Select which wavenumber we need */
		case GMT_FFT_K_IS_KX: K->k_ptr = GMT_fft_kx; break;
		case GMT_FFT_K_IS_KY: K->k_ptr = GMT_fft_ky; break;
		case GMT_FFT_K_IS_KR: K->k_ptr = GMT_fft_kr; break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad mode selected (%u) - exit\n", mode);
			GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
			break;
	}
	return GMT_OK;
}

void gmt_fft_taper (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GMT_FFT_INFO *F)
{
	/* mode sets if and how tapering will be performed [see GMT_FFT_EXTEND_* constants].
	 * width is relative width in percent of the margin that will be tapered [100]. */
	int il1, ir1, il2, ir2, jb1, jb2, jt1, jt2, im, jm, j, end_i, end_j, min_i, min_j, one;
	int i, i_data_start, j_data_start, mx, i_width, j_width, width_percent;
	unsigned int ju, start_component = 0, stop_component = 0, component;
	uint64_t off;
	char *method[2] = {"edge-point", "mirror"}, *comp[2] = {"real", "imaginary"};
	float *datac = Grid->data, scale, cos_wt;
	double width;
	struct GMT_GRID_HEADER *h = Grid->header;	/* For shorthand */

	width_percent = irint (F->taper_width);

	if ((Grid->header->nx == F->nx && Grid->header->ny == F->ny) || F->taper_mode == GMT_FFT_EXTEND_NONE) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Data and FFT dimensions are equal - no data extension will take place\n");
		/* But there may still be interior tapering */
		if (F->taper_mode != GMT_FFT_EXTEND_NONE) {	/* Nothing to do since no outside pad */
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Data and FFT dimensions are equal - no tapering will be performed\n");
			return;
		}
		if (F->taper_mode == GMT_FFT_EXTEND_NONE && width_percent == 100) {	/* No interior taper specified */
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "No interior tapering will be performed\n");
			return;
		}
	}
	
	if (Grid->header->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Demultiplexing complex grid before tapering can take place.\n");
		GMT_grd_mux_demux (GMT, Grid->header, Grid->data, GMT_GRID_IS_SERIAL);
	}
	
	/* Note that if nx2 = nx+1 and ny2 = ny + 1, then this routine
	 * will do nothing; thus a single row/column of zeros may be
	 * added to the bottom/right of the input array and it cannot
	 * be tapered.  But when (nx2 - nx)%2 == 1 or ditto for y,
	 * this is zero anyway.  */

	i_data_start = GMT->current.io.pad[XLO];	/* Some shorthands for readability */
	j_data_start = GMT->current.io.pad[YHI];
	mx = h->mx;
	one = (F->taper_mode == GMT_FFT_EXTEND_NONE) ? 0 : 1;	/* 0 is the boundry point which we want to taper to 0 for the interior taper */
	
	if (width_percent == 0) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Tapering has been disabled via +t0\n");
	}
	if (width_percent == 100 && F->taper_mode == GMT_FFT_EXTEND_NONE) {	/* Means user set +n but did not specify +t<taper> as 100% is unreasonable for interior */
		width_percent = 0;
		width = 0.0;
	}
	else
		width = F->taper_width / 100.0;	/* Was percent, now fraction */
	
	if (F->taper_mode == GMT_FFT_EXTEND_NONE) {	/* No extension, just tapering inside the data grid */
		i_width = irint (Grid->header->nx * width);	/* Interior columns over which tapering will take place */
		j_width = irint (Grid->header->ny * width);	/* Extended rows over which tapering will take place */
	}
	else {	/* We wish to extend data into the margin pads between FFT grid and data grid */
		i_width = irint (i_data_start * width);	/* Extended columns over which tapering will take place */
		j_width = irint (j_data_start * width);	/* Extended rows over which tapering will take place */
	}
	if (i_width == 0 && j_width == 0) one = 1;	/* So we do nothing further down */

	/* Determine how many complex components (1 or 2) to taper, and which one(s) */
	start_component = (Grid->header->complex_mode & GMT_GRID_IS_COMPLEX_REAL) ? 0 : 1;
	stop_component  = (Grid->header->complex_mode & GMT_GRID_IS_COMPLEX_IMAG) ? 1 : 0;

	for (component = start_component; component <= stop_component; component++) {	/* Loop over 1 or 2 components */
		off = component * Grid->header->size / 2;	/* offset to start of this component in grid */

		/* First reflect about xmin and xmax, either point symmetric about edge point OR mirror symmetric */

		if (F->taper_mode != GMT_FFT_EXTEND_NONE) {
			for (im = 1; im <= i_width; im++) {
				il1 = -im;	/* Outside xmin; left of edge 1  */
				ir1 = im;	/* Inside xmin; right of edge 1  */
				il2 = il1 + h->nx - 1;	/* Inside xmax; left of edge 2  */
				ir2 = ir1 + h->nx - 1;	/* Outside xmax; right of edge 2  */
				for (ju = 0; ju < h->ny; ju++) {
					if (F->taper_mode == GMT_FFT_EXTEND_POINT_SYMMETRY) {
						datac[GMT_IJP(h,ju,il1)+off] = 2.0f * datac[GMT_IJP(h,ju,0)+off]       - datac[GMT_IJP(h,ju,ir1)+off];
						datac[GMT_IJP(h,ju,ir2)+off] = 2.0f * datac[GMT_IJP(h,ju,h->nx-1)+off] - datac[GMT_IJP(h,ju,il2)+off];
					}
					else {	/* Mirroring */
						datac[GMT_IJP(h,ju,il1)+off] = datac[GMT_IJP(h,ju,ir1)+off];
						datac[GMT_IJP(h,ju,ir2)+off] = datac[GMT_IJP(h,ju,il2)+off];
					}
				}
			}
		}

		/* Next, reflect about ymin and ymax.
		 * At the same time, since x has been reflected,
		 * we can use these vals and taper on y edges */

		scale = (float)(M_PI / (j_width + 1));	/* Full 2*pi over y taper range */
		min_i = (F->taper_mode == GMT_FFT_EXTEND_NONE) ? 0 : -i_width;
		end_i = (F->taper_mode == GMT_FFT_EXTEND_NONE) ? (int)Grid->header->nx : mx - i_width;
		for (jm = one; jm <= j_width; jm++) {	/* Loop over width of strip to taper */
			jb1 = -jm;	/* Outside ymin; bottom side of edge 1  */
			jt1 = jm;	/* Inside ymin; top side of edge 1  */
			jb2 = jb1 + h->ny - 1;	/* Inside ymax; bottom side of edge 2  */
			jt2 = jt1 + h->ny - 1;	/* Outside ymax; bottom side of edge 2  */
			cos_wt = 0.5f * (1.0f + cosf (jm * scale));
			if (F->taper_mode == GMT_FFT_EXTEND_NONE) cos_wt = 1.0f - cos_wt;	/* Reverse weights for the interior */
			for (i = min_i; i < end_i; i++) {
				if (F->taper_mode == GMT_FFT_EXTEND_POINT_SYMMETRY) {
					datac[GMT_IJP(h,jb1,i)+off] = cos_wt * (2.0f * datac[GMT_IJP(h,0,i)+off]       - datac[GMT_IJP(h,jt1,i)+off]);
					datac[GMT_IJP(h,jt2,i)+off] = cos_wt * (2.0f * datac[GMT_IJP(h,h->ny-1,i)+off] - datac[GMT_IJP(h,jb2,i)+off]);
				}
				else if (F->taper_mode == GMT_FFT_EXTEND_MIRROR_SYMMETRY) {
					datac[GMT_IJP(h,jb1,i)+off] = cos_wt * datac[GMT_IJP(h,jt1,i)+off];
					datac[GMT_IJP(h,jt2,i)+off] = cos_wt * datac[GMT_IJP(h,jb2,i)+off];
				}
				else {	/* Interior tapering only */
					datac[GMT_IJP(h,jt1,i)+off] *= cos_wt;
					datac[GMT_IJP(h,jb2,i)+off] *= cos_wt;
				}
			}
		}
		/* Now, cos taper the x edges */
		scale = (float)(M_PI / (i_width + 1));	/* Full 2*pi over x taper range */
		end_j = (F->taper_mode == GMT_FFT_EXTEND_NONE) ? h->ny : h->my - j_data_start;
		min_j = (F->taper_mode == GMT_FFT_EXTEND_NONE) ? 0 : -j_width;
		for (im = one; im <= i_width; im++) {
			il1 = -im;
			ir1 = im;
			il2 = il1 + h->nx - 1;
			ir2 = ir1 + h->nx - 1;
			cos_wt = (float)(0.5f * (1.0f + cosf (im * scale)));
			if (F->taper_mode == GMT_FFT_EXTEND_NONE) cos_wt = 1.0f - cos_wt;	/* Switch to weights for the interior */
			for (j = min_j; j < end_j; j++) {
				if (F->taper_mode == GMT_FFT_EXTEND_NONE) {
					datac[GMT_IJP(h,j,ir1)+off] *= cos_wt;
					datac[GMT_IJP(h,j,il2)+off] *= cos_wt;
				}
				else {
					datac[GMT_IJP(h,j,il1)+off] *= cos_wt;
					datac[GMT_IJP(h,j,ir2)+off] *= cos_wt;
				}
			}
		}

		if (F->taper_mode == GMT_FFT_EXTEND_NONE)
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Grid margin (%s component) tapered to zero over %d %% of data width and height\n", comp[component], width_percent);
		else
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Grid (%s component) extended via %s symmetry at all edges, then tapered to zero over %d %% of extended area\n", comp[component], method[F->taper_mode], width_percent);
	}
}

char *file_name_with_suffix (struct GMT_CTRL *GMT, char *name, char *suffix)
{
	static char file[GMT_BUFSIZ];
	uint64_t i, j;
	size_t len;
	
	if ((len = strlen (name)) == 0) {	/* Grids that are being created have no filename yet */
		sprintf (file, "tmpgrid_%s.grd", suffix);
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Created grid has no name to derive new names from; choose %s\n", file);
		return (file);
	}
	for (i = len; i > 0 && name[i] != '/'; i--);	/* i points to 1st char in name after slash, or 0 if no leading dirs */
	if (i) i++;	/* Move to 1st char after / */
	for (j = len; j > 0 && name[j] != '.'; j--);	/* j points to period before extension, or it is 0 if no extension */
	strcpy (file, &name[i]);			/* Make a full copy of filename without leading directories */
	len = strlen (file);
	for (i = len; i > 0 && file[i] != '.'; i--);	/* i now points to period before extension in file, or it is 0 if no extension */
	if (i) file[i] = '\0';	/* Truncate at the extension */
	strcat (file, "_");
	strcat (file, suffix);
	if (j) strcat (file, &name[j]);
	return (file);
}

void gmt_grd_save_taper (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, char *suffix)
{
	/* Write the intermediate grid that will be passed to the FFT to file.
	 * This grid may have been a mean, mid-value, or plane removed, may
	 * have data filled into an extended margin, and may have been taperer.
	 * Normally, the complex grid will be in serial layout, but just in case
	 * we check and add a demux step if required.  The FFT will also check
	 * and multiplex the grid (again) if needed.
	 */
	unsigned int pad[4];
	struct GMT_GRID_HEADER save;
	char *file = NULL;
	
	if (Grid->header->arrangement == GMT_GRID_IS_INTERLEAVED) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Demultiplexing complex grid before saving can take place.\n");
		GMT_grd_mux_demux (GMT, Grid->header, Grid->data, GMT_GRID_IS_SERIAL);
	}
	GMT_memcpy (&save, Grid->header, 1U, struct GMT_GRID_HEADER);	/* Save what we have before messing around */
	GMT_memcpy (pad, Grid->header->pad, 4U, unsigned int);		/* Save current pad, then set pad to zero */
	/* Extend w/e/s/n to what it would be if the pad was not present */
	Grid->header->wesn[XLO] -= Grid->header->pad[XLO] * Grid->header->inc[GMT_X];
	Grid->header->wesn[XHI] += Grid->header->pad[XHI] * Grid->header->inc[GMT_X];
	Grid->header->wesn[YLO] -= Grid->header->pad[YLO] * Grid->header->inc[GMT_Y];
	Grid->header->wesn[YHI] += Grid->header->pad[YHI] * Grid->header->inc[GMT_Y];
	GMT_memset (Grid->header->pad,   4U, unsigned int);	/* Set header pad to {0,0,0,0} */
	GMT_memset (GMT->current.io.pad, 4U, unsigned int);	/* set GMT default pad to {0,0,0,0} */
	GMT_set_grddim (GMT, Grid->header);	/* Recompute all dimensions */
	if ((file = file_name_with_suffix (GMT, Grid->header->name, suffix)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to get file name for file %s\n", Grid->header->name);
		return;
	}
	
	if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY | GMT_GRID_IS_COMPLEX_REAL, NULL, file, Grid) != GMT_OK)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Intermediate detrended, extended, and tapered grid could not be written to %s\n", file);
	else
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Intermediate detrended, extended, and tapered grid written to %s\n", file);
	
	GMT_memcpy (Grid->header, &save, 1U, struct GMT_GRID_HEADER);	/* Restore original, including the original pad */
	GMT_memcpy (GMT->current.io.pad, pad, 4U, unsigned int);	/* Restore GMT default pad */
}

void gmt_grd_save_fft (struct GMT_CTRL *GMT, struct GMT_GRID *G, struct GMT_FFT_INFO *F)
{
	/* Save the raw spectrum as two files (real,imag) or (mag,phase), depending on mode.
	 * We must first do an "fftshift" operation as in Matlab, to put the 0 frequency
	 * value in the center of the grid. */
	uint64_t row, col, i_ij, o_ij;
	unsigned int nx_2, ny_2, k, pad[4], mode, wmode[2] = {GMT_GRID_IS_COMPLEX_REAL, GMT_GRID_IS_COMPLEX_IMAG};
	double wesn[4], inc[2];
	float re, im;
	char *file = NULL, *suffix[2][2] = {{"real", "imag"}, {"mag", "phase"}};
	struct GMT_GRID *Grid = NULL;
	struct GMT_FFT_WAVENUMBER *K = F->K;

	if (K == NULL) return;
	
	mode = (F->polar) ? 1 : 0;

	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Write components of complex raw spectrum with file suffiz %s and %s\n", suffix[mode][0], suffix[mode][1]);

	if (G->header->arrangement == GMT_GRID_IS_SERIAL) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Cannot save complex grid unless it is interleaved.\n");
		return;
	}
	/* Prepare wavenumber domain limits and increments */
	nx_2 = K->nx2 / 2;	ny_2 = K->ny2 / 2;
	wesn[XLO] = -K->delta_kx * nx_2;	wesn[XHI] =  K->delta_kx * (nx_2 - 1);
	wesn[YLO] = -K->delta_ky * (ny_2 - 1);	wesn[YHI] =  K->delta_ky * ny_2;
	inc[GMT_X] = K->delta_kx;		inc[GMT_Y] = K->delta_ky;
	GMT_memcpy (pad, GMT->current.io.pad, 4U, unsigned int);	/* Save current GMT pad */
	for (k = 0; k < 4; k++) GMT->current.io.pad[k] = 0;		/* No pad is what we need for this application */

	/* Set up and allocate the temporary grid. */
	if ((Grid = GMT_Create_Data (GMT->parent, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, wesn, inc, \
		G->header->registration | GMT_GRID_IS_COMPLEX_REAL, 0, NULL)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to create complex output grid for %s\n", Grid->header->name);
		return;
	}
			
	strcpy (Grid->header->x_units, "m^(-1)");	strcpy (Grid->header->y_units, "m^(-1)");
	strcpy (Grid->header->z_units, G->header->z_units);
	strcpy (Grid->header->remark, "Applied fftshift: kx = 0 at (nx/2 + 1) and ky = 0 at ny/2");

	for (row = 0; row < ny_2; row++) {	/* Swap values from 1/3 and 2/4 quadrants */
		for (col = 0; col < nx_2; col++) {
			i_ij = 2*GMT_IJ0 (Grid->header, row, col);
			o_ij = 2*GMT_IJ0 (Grid->header, row+ny_2, col+nx_2);
			re = Grid->data[i_ij]; im = Grid->data[i_ij+1];
			if (F->polar) {	/* Want magnitude and phase */
				Grid->data[i_ij]   = (float)hypot (G->data[o_ij], G->data[o_ij+1]);
				Grid->data[i_ij+1] = (float)d_atan2 (G->data[o_ij+1], G->data[o_ij]);
				Grid->data[o_ij]   = (float)hypot (re, im);
				Grid->data[o_ij+1] = (float)d_atan2 (im, re);
			}
			else {		/* Retain real and imag components as is */
				Grid->data[i_ij] = G->data[o_ij];	Grid->data[i_ij+1] = G->data[o_ij+1];
				Grid->data[o_ij] = re;	Grid->data[o_ij+1] = im;
			}
			i_ij = 2*GMT_IJ0 (Grid->header, row+ny_2, col);
			o_ij = 2*GMT_IJ0 (Grid->header, row, col+nx_2);
			re = Grid->data[i_ij]; im = Grid->data[i_ij+1];
			if (F->polar) {	/* Want magnitude and phase */
				Grid->data[i_ij]   = (float)hypot (G->data[o_ij], G->data[o_ij+1]);
				Grid->data[i_ij+1] = (float)d_atan2 (G->data[o_ij+1], G->data[o_ij]);
				Grid->data[o_ij]   = (float)hypot (re, im);
				Grid->data[o_ij+1] = (float)d_atan2 (im, re);
			}
			else {		/* Retain real and imag components as is */
				Grid->data[i_ij] = G->data[o_ij];	Grid->data[i_ij+1] = G->data[o_ij+1];
				Grid->data[o_ij] = re;	Grid->data[o_ij+1] = im;
			}
		}
	}
	for (k = 0; k < 2; k++) {	/* Write the two grids */
		if ((file = file_name_with_suffix (GMT, Grid->header->name, suffix[mode][k])) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to get file name for file %s\n", Grid->header->name);
			return;
		}
		sprintf (Grid->header->title, "The %s part of FFT transformed input grid %s", suffix[mode][k], Grid->header->name);
		if (k == 1 && mode) strcpy (Grid->header->z_units, "radians");
		if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY | wmode[k], NULL, file, Grid) != GMT_OK) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "%s could not be written\n", file);
			return;
		}
	}
	if (GMT_Destroy_Data (GMT->parent, &Grid) != GMT_OK) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error freeing temporary grid\n");
	}

	GMT_memcpy (GMT->current.io.pad, pad, 4U, unsigned int);	/* Restore GMT pad */
}

void gmt_fft_save2d (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int direction, struct GMT_FFT_WAVENUMBER *K)
{
	/* Handle the writing of the grid going into the FFT and comping out of the FFT, per F settings */

	if (G == NULL || (K == NULL ||  K->info == NULL)) return;
	if (direction == GMT_IN  && K->info->save[GMT_IN])  gmt_grd_save_taper (GMT, G, K->info->suffix);
	if (direction == GMT_OUT && K->info->save[GMT_OUT]) gmt_grd_save_fft (GMT, G, K->info);
}

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

#ifdef HAVE_FFTW3F

#include <fftw3.h>

#define FFTWF_WISDOM_FILENAME "fftwf_wisdom"

char *gmt_fftwf_wisdom_filename (struct GMT_CTRL *GMT) {
	static char wisdom_file[PATH_MAX+256] = "\0";
	char hostname[257];
	if (*wisdom_file == '\0') { /* wisdom_file has not been set yet */
		if (GMT->session.USERDIR == NULL || access (GMT->session.USERDIR, R_OK|W_OK|X_OK))
			/* USERDIR does not exist, or not writable */
			return NULL;
		else {
			/* create wisdom file in USERDIR */
			strncpy (wisdom_file, GMT->session.USERDIR, PATH_MAX);
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
void gmt_fftwf_import_wisdom_from_filename (struct GMT_CTRL *GMT) {
	static bool already_imported = false;
	char *filenames[3], **filename = filenames;
	int status;

	if (already_imported) /* nothing to do */
		return;

	fftwf_import_system_wisdom (); /* read wisdom from implementation-defined standard file */

	/* Initialize filenames */
	filenames[0] = FFTWF_WISDOM_FILENAME; /* 1st try importing wisdom from file in current dir */
	filenames[1] = gmt_fftwf_wisdom_filename(GMT); /* 2nd try wisdom file in USERDIR */
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
void gmt_fftwf_export_wisdom_to_filename (struct GMT_CTRL *GMT) {
	char *filename = gmt_fftwf_wisdom_filename(GMT);
	int status;

	if (filename == NULL)
		/* USERDIR does not exist, write wisdom to file in current directory */
		filename = FFTWF_WISDOM_FILENAME;

	status = fftwf_export_wisdom_to_filename (filename);
	if (status)
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Exported FFTW Wisdom to file: %s\n", filename);
	else
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Exporting FFTW Wisdom to file failed: %s\n", filename);
}

fftwf_plan gmt_fftwf_plan_dft(struct GMT_CTRL *GMT, unsigned ny, unsigned nx, fftwf_complex *data, int direction) {
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
		gmt_fftwf_import_wisdom_from_filename (GMT);
		if (ny == 0) /* 1d DFT */
			plan = fftwf_plan_dft_1d(nx, cin, cout, sign, FFTW_WISDOM_ONLY | GMT->current.setting.fftw_plan);
		else /* 2d DFT */
			plan = fftwf_plan_dft_2d(ny, nx, cin, cout, sign, FFTW_WISDOM_ONLY | GMT->current.setting.fftw_plan);
		if (plan == NULL) {
			/* No Wisdom available
			 * Need extra memory to prevent overwriting data while planning */
			fftwf_complex *in_place_tmp = fftwf_malloc (2 * (ny == 0 ? 1 : ny) * nx * sizeof(float));
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Generating new FFTW Wisdom, be patient...\n");
			if (ny == 0) /* 1d DFT */
				plan = fftwf_plan_dft_1d(nx, in_place_tmp, in_place_tmp, sign, GMT->current.setting.fftw_plan);
			else /* 2d DFT */
				plan = fftwf_plan_dft_2d(ny, nx, in_place_tmp, in_place_tmp, sign, GMT->current.setting.fftw_plan);
			fftwf_destroy_plan(plan); /* deallocate plan */
			plan = NULL;
			fftwf_free (in_place_tmp);
			/* Save new Wisdom */
			gmt_fftwf_export_wisdom_to_filename (GMT);
		}
		else
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Using preexisting FFTW Wisdom.\n");
	} /* GMT->current.setting.fftw_plan != FFTW_ESTIMATE */
	else
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Picking a (probably sub-optimal) FFTW plan quickly.\n");

	if (plan == NULL) { /* If either FFTW_ESTIMATE or new Wisdom generated */
		if (ny == 0) /* 1d DFT */
			plan = fftwf_plan_dft_1d(nx, cin, cout, sign, GMT->current.setting.fftw_plan);
		else /* 2d DFT */
			plan = fftwf_plan_dft_2d(ny, nx, cin, cout, sign, GMT->current.setting.fftw_plan);
	}

	if (plan == NULL) { /* There was a problem creating a plan */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not create FFTW plan.\n");
		GMT_exit (GMT, EXIT_FAILURE); return NULL;
	}

	return plan;
}

int GMT_fft_1d_fftwf (struct GMT_CTRL *GMT, float *data, unsigned int n, int direction, unsigned int mode) {
	fftwf_plan plan = NULL;

	/* Generate FFTW plan for complex 1d DFT */
	plan = gmt_fftwf_plan_dft(GMT, 0, n, (fftwf_complex*)data, direction);
	fftwf_execute(plan);      /* do transform */
	fftwf_destroy_plan(plan); /* deallocate plan */

	return GMT_NOERROR;
}

int GMT_fft_2d_fftwf (struct GMT_CTRL *GMT, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode) {
	fftwf_plan plan = NULL;

	/* Generate FFTW plan for complex 2d DFT */
	plan = gmt_fftwf_plan_dft(GMT, ny, nx, (fftwf_complex*)data, direction);
	fftwf_execute(plan);      /* do transform */
	fftwf_destroy_plan(plan); /* deallocate plan */

	return GMT_NOERROR;
}

#endif /* HAVE_FFTW3F */

#ifdef __APPLE__ /* Accelerate framework */

void GMT_fft_1d_vDSP_reset (struct GMT_FFT_HIDDEN *Z)
{
	if (Z->setup_1d) {	/* Free single-precision FFT data structure and arrays */
		vDSP_destroy_fftsetup (Z->setup_1d);
		free (Z->dsp_split_complex_1d.realp);
		free (Z->dsp_split_complex_1d.imagp);
	}
}

int GMT_fft_1d_vDSP (struct GMT_CTRL *GMT, float *data, unsigned int n, int direction, unsigned int mode)
{
	FFTDirection fft_direction = direction == GMT_FFT_FWD ?
			kFFTDirection_Forward : kFFTDirection_Inverse;
	DSPComplex *dsp_complex = (DSPComplex *)data;

	/* Base 2 exponent that specifies the largest power of
	 * two that can be processed by fft: */
	vDSP_Length log2n = radix2 (n);

	if (log2n == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Need Radix-2 input try: %u [n]\n", 1U<<propose_radix2 (n));
		return -1;
	}

	if (GMT->current.fft.n_1d != n) {	/* Must update the FFT setup arrays */
		/* Build data structure that contains precalculated data for use by
		 * single-precision FFT functions: */
		GMT_fft_1d_vDSP_reset (&GMT->current.fft);
		GMT->current.fft.setup_1d = vDSP_create_fftsetup (log2n, kFFTRadix2);
		GMT->current.fft.dsp_split_complex_1d.realp = malloc (n * sizeof(float));
		GMT->current.fft.dsp_split_complex_1d.imagp = malloc (n * sizeof(float));
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

void GMT_fft_2d_vDSP_reset (struct GMT_FFT_HIDDEN *Z)
{
	if (Z->setup_2d) {	/* Free single-precision 2D FFT data structure and arrays */
		vDSP_destroy_fftsetup (Z->setup_2d);
		free (Z->dsp_split_complex_2d.realp);
		free (Z->dsp_split_complex_2d.imagp);
	}
}

int GMT_fft_2d_vDSP (struct GMT_CTRL *GMT, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode)
{
	FFTDirection fft_direction = direction == GMT_FFT_FWD ?
			kFFTDirection_Forward : kFFTDirection_Inverse;
	DSPComplex *dsp_complex = (DSPComplex *)data;

	/* Base 2 exponent that specifies the largest power of
	 * two that can be processed by fft: */
	vDSP_Length log2nx = radix2 (nx);
	vDSP_Length log2ny = radix2 (ny);
	unsigned int n_xy = nx * ny;

	if (log2nx == 0 || log2ny == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Need Radix-2 input try: %u/%u [nx/ny]\n",
				1U<<propose_radix2 (nx), 1U<<propose_radix2 (ny));
		return -1;
	}

	if (GMT->current.fft.n_2d != n_xy) {	/* Must update the 2-D FFT setup arrays */
		/* Build data structure that contains precalculated data for use by
	 	* single-precision FFT functions: */
		GMT_fft_2d_vDSP_reset (&GMT->current.fft);
		GMT->current.fft.setup_2d = vDSP_create_fftsetup (MAX (log2nx, log2ny), kFFTRadix2);
		GMT->current.fft.dsp_split_complex_2d.realp = malloc (n_xy * sizeof(float));
		GMT->current.fft.dsp_split_complex_2d.imagp = malloc (n_xy * sizeof(float));
		if (GMT->current.fft.dsp_split_complex_2d.realp == NULL || GMT->current.fft.dsp_split_complex_2d.imagp == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to allocate dsp_split_complex array of length %u\n", n_xy);
			return -1; /* out of memory */
		}
		GMT->current.fft.n_2d = n_xy;
	}
	vDSP_ctoz (dsp_complex, 2, &GMT->current.fft.dsp_split_complex_2d, 1, n_xy);

	/* complex: */
	vDSP_fft2d_zip (GMT->current.fft.setup_2d, &GMT->current.fft.dsp_split_complex_2d, 1, 0, log2ny, log2nx, fft_direction);
	/* real:
	vDSP_fft2d_zrip (setup, &GMT->current.fft.dsp_split_complex_2d, 1, 0, log2ny, log2nx, fft_direction); */

	vDSP_ztoc (&GMT->current.fft.dsp_split_complex_2d, 1, dsp_complex, 2, n_xy);

	return GMT_NOERROR;
}
#endif /* APPLE Accelerate framework */

/* Kiss FFT */

#include "kiss_fft/kiss_fftnd.h"

int GMT_fft_1d_kiss (struct GMT_CTRL *GMT, float *data, unsigned int n, int direction, unsigned int mode)
{
	kiss_fft_cpx *fin, *fout;
	kiss_fft_cfg config;

	/* Initialize a FFT (or IFFT) config/state data structure */
	config = kiss_fft_alloc(n, direction == GMT_FFT_INV, NULL, NULL);
	fin = fout = (kiss_fft_cpx *)data;
	kiss_fft (config, fin, fout); /* do transform */
	free (config); /* Free config data structure */

	return GMT_NOERROR;
}

int GMT_fft_2d_kiss (struct GMT_CTRL *GMT, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode)
{
	const int dim[2] = {ny, nx}; /* dimensions of fft */
	const int dimcount = 2;      /* number of dimensions */
	kiss_fft_cpx *fin, *fout;
	kiss_fftnd_cfg config;

	/* Initialize a FFT (or IFFT) config/state data structure */
	config = kiss_fftnd_alloc (dim, dimcount, direction == GMT_FFT_INV, NULL, NULL);

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

size_t brenner_worksize (struct GMT_CTRL *GMT, unsigned int nx, unsigned int ny)
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
        n_factors = GMT_get_prime_factors (GMT, nx, f);
        nonsymx = gmt_get_non_symmetric_f (f, n_factors);
        n_factors = GMT_get_prime_factors (GMT, ny, f);
        nonsymy = gmt_get_non_symmetric_f (f, n_factors);
        nonsym = MAX (nonsymx, nonsymy);
	
        /* Now get factors of ntotal  */
        ntotal = GMT_get_nm (GMT, nx, ny);
        n_factors = GMT_get_prime_factors (GMT, ntotal, f);
        storage = MAX (nonsym, f[n_factors-1]);
        if (storage != 2) storage *= 2;
        if (storage < nx) storage = nx;
        if (storage < ny) storage = ny;
        return (2 * storage);
}

/* C-callable wrapper for BRENNER_fourt_ */
int GMT_fft_1d_brenner (struct GMT_CTRL *GMT, float *data, unsigned int n, int direction, unsigned int mode)
{
        /* void GMT_fourt (struct GMT_CTRL *GMT, float *data, int *nn, int ndim, int ksign, int iform, float *work) */
        /* Data array */
        /* Dimension array */
        /* Number of dimensions */
        /* Forward(-1) or Inverse(+1) */
        /* Real(0) or complex(1) data */
        /* Work array */
	
        int ksign, ndim = 1, n_signed = n, kmode = mode;
        size_t work_size = 0;
        float *work = NULL;
	
        ksign = (direction == GMT_FFT_INV) ? +1 : -1;
        if ((work_size = brenner_worksize (GMT, n, 1))) work = GMT_memory (GMT, NULL, work_size, float);
        (void) BRENNER_fourt_ (data, &n_signed, &ndim, &ksign, &kmode, work);
        if (work_size) GMT_free (GMT, work);	
        return (GMT_OK);
}
	
int GMT_fft_2d_brenner (struct GMT_CTRL *GMT, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode)
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

        ksign = (direction == GMT_FFT_INV) ? +1 : -1;
        if ((work_size = brenner_worksize (GMT, nx, ny))) work = GMT_memory (GMT, NULL, work_size, float);
        GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Brenner_fourt_ work size = %" PRIuS "\n", work_size);
        (void) BRENNER_fourt_ (data, nn, &ndim, &ksign, &kmode, work);
        if (work_size) GMT_free (GMT, work);
        return (GMT_OK);
}

int gmt_fft_1d_selection (struct GMT_CTRL *GMT, uint64_t n) {
	/* Returns the most suitable 1-D FFT for the job - or the one requested via GMT_FFT */
	if (GMT->current.setting.fft != k_fft_auto) {
		/* Specific selection requested */
		if (GMT->session.fft1d[GMT->current.setting.fft])
			return GMT->current.setting.fft; /* User defined FFT */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Desired FFT Algorithm (%s) not configured - choosing suitable alternative.\n", GMT_fft_algo[GMT->current.setting.fft]);
	}
	/* Here we want automatic selection from available candidates */
	if (GMT->session.fft1d[k_fft_accelerate] && radix2 (n))
		return k_fft_accelerate; /* Use if Radix-2 under OS/X */
	if (GMT->session.fft1d[k_fft_fftw])
		return k_fft_fftw;
	return k_fft_kiss; /* Default/fallback general-purpose FFT */
}

int gmt_fft_2d_selection (struct GMT_CTRL *GMT, unsigned int nx, unsigned int ny) {
	/* Returns the most suitable 2-D FFT for the job - or the one requested via GMT_FFT */
	if (GMT->current.setting.fft != k_fft_auto) {
		/* Specific selection requested */
		if (GMT->session.fft2d[GMT->current.setting.fft])
			return GMT->current.setting.fft; /* User defined FFT */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Desired FFT Algorithm (%s) not configured - choosing suitable alternative.\n", GMT_fft_algo[GMT->current.setting.fft]);
	}
	/* Here we want automatic selection from available candidates */
	if (GMT->session.fft2d[k_fft_accelerate] && radix2 (nx) && radix2 (ny))
		return k_fft_accelerate; /* Use if Radix-2 under OS/X */
	if (GMT->session.fft2d[k_fft_fftw])
		return k_fft_fftw;
	return k_fft_kiss; /* Default/fallback general-purpose FFT */
}

int GMT_FFT_1D (void *V_API, float *data, uint64_t n, int direction, unsigned int mode) {
	/* data is an array of length n (or 2*n for complex) data points
	 * n is the number of data points
	 * direction is either GMT_FFT_FWD (forward) or GMT_FFT_INV (inverse)
	 * mode is either GMT_FFT_REAL or GMT_FFT_COMPLEX
	 */
	int status, use;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);
	struct GMT_CTRL *GMT = API->GMT;
	assert (mode == GMT_FFT_COMPLEX); /* GMT_FFT_REAL not implemented yet */
	use = gmt_fft_1d_selection (GMT, n);
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "1-D FFT using %s\n", GMT_fft_algo[use]);
	status = GMT->session.fft1d[use] (GMT, data, (unsigned int)n, direction, mode);
	if (direction == GMT_FFT_INV) {	/* Undo the 2/nm factor */
		uint64_t nm = 2ULL * n;
		GMT_scale_and_offset_f (GMT, data, nm, 2.0 / nm, 0);
	}
	return status;
}

int GMT_FFT_2D (void *V_API, float *data, unsigned int nx, unsigned int ny, int direction, unsigned int mode) {
	/* data is an array of length nx*ny (or 2*nx*ny for complex) data points
	 * nx, ny is the number of data nodes
	 * direction is either GMT_FFT_FWD (forward) or GMT_FFT_INV (inverse)
	 * mode is either GMT_FFT_REAL or GMT_FFT_COMPLEX
	 */
	int status, use;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);
	struct GMT_CTRL *GMT = API->GMT;
	assert (mode == GMT_FFT_COMPLEX); /* GMT_FFT_REAL not implemented yet */
	use = gmt_fft_2d_selection (GMT, nx, ny);
	
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "2-D FFT using %s\n", GMT_fft_algo[use]);
	status = GMT->session.fft2d[use] (GMT, data, nx, ny, direction, mode);
	if (direction == GMT_FFT_INV) {	/* Undo the 2/nm factor */
		uint64_t nm = 2ULL * nx * ny;
		GMT_scale_and_offset_f (GMT, data, nm, 2.0 / nm, 0);
	}
	return status;
}

#if defined WIN32
#include <windows.h>
#endif

void GMT_fft_initialization (struct GMT_CTRL *GMT) {
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
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Initialize FFTW with %d threads.\n", n_cpu);
		}
	}

	GMT->current.setting.fftw_plan = FFTW_ESTIMATE; /* default planner flag */
#endif /* HAVE_FFTW3_THREADS */

	/* Start with nothing */
	memset (GMT->session.fft1d, k_n_fft_algorithms, sizeof(void*));
	memset (GMT->session.fft2d, k_n_fft_algorithms, sizeof(void*));

#ifdef __APPLE__
	/* OS X Accelerate Framework */
	GMT->session.fft1d[k_fft_accelerate] = &GMT_fft_1d_vDSP;
	GMT->session.fft2d[k_fft_accelerate] = &GMT_fft_2d_vDSP;
#endif
#ifdef HAVE_FFTW3F
	/* single precision FFTW3 */
	GMT->session.fft1d[k_fft_fftw] = &GMT_fft_1d_fftwf;
	GMT->session.fft2d[k_fft_fftw] = &GMT_fft_2d_fftwf;
#endif /* HAVE_FFTW3F */
	/* Kiss FFT is the integrated fallback */
	GMT->session.fft1d[k_fft_kiss] = &GMT_fft_1d_kiss;
	GMT->session.fft2d[k_fft_kiss] = &GMT_fft_2d_kiss;
	/* Brenner FFT is the legacy fallback */
	GMT->session.fft1d[k_fft_brenner] = &GMT_fft_1d_brenner;
	GMT->session.fft2d[k_fft_brenner] = &GMT_fft_2d_brenner;
}

void GMT_fft_cleanup (struct GMT_CTRL *GMT) {
	/* Called by GMT_end */
#if defined HAVE_FFTW3F_THREADS
	fftwf_cleanup_threads(); /* clean resources allocated internally by FFTW */
#endif
#ifdef __APPLE__ /* Accelerate framework */
	GMT_fft_1d_vDSP_reset (&GMT->current.fft);
	GMT_fft_2d_vDSP_reset (&GMT->current.fft);
#endif
}
