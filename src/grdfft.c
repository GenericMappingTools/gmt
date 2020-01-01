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
 *  Brief synopsis: grdfft.c is a program to do various operations on
 *  grid files in the frequency domain.
 *
 * Author:	W.H.F. Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 * Note:	PW: As of 2/14/2013 the various setup and init functions for FFT use
 *		have been generalized and made available GMT-wide via new functions
 *		in gmt_fft.c, called GMT_fft_*.
 */

#include "gmt_dev.h"
EXTERN_MSC unsigned int gmtlib_count_char (struct GMT_CTRL *GMT, char *txt, char it);

#define THIS_MODULE_CLASSIC_NAME	"grdfft"
#define THIS_MODULE_MODERN_NAME	"grdfft"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Mathematical operations on grids in the spectral domain"
#define THIS_MODULE_KEYS	"<G{+,GG},GDE"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-Vfh" GMT_OPT("T")

#define GMT_FFT_DIM	2	/* Dimension of FFT needed */

#ifdef DEBUG
/* For debugging -E; running this in debug and setting it to true will also output the number of estimates per radial k */
static bool show_n = false;
#endif

struct GRDFFT_CTRL {
	unsigned int n_op_count, n_par;
	int *operation;
	double *par;

	struct GRDFFT_In {
		bool active;
		unsigned int n_grids;	/* 1 or 2 */
		char *file[2];
	} In;
	struct GRDFFT_A {	/* -A<azimuth> */
		bool active;
		double value;
	} A;
	struct GRDFFT_C {	/* -C<zlevel> */
		bool active;
		double value;
	} C;
	struct GRDFFT_D {	/* -D[<scale>|g] */
		bool active;
		double value;
	} D;
	struct GRDFFT_E {	/* -E[r|x|y][w[k]][n] */
		bool active;
		bool give_wavelength;
		bool normalize;
		bool km;
		int mode;	/*-1/0/+1 */
	} E;
	struct GRDFFT_F {	/* -F[r|x|y]<lc>/<lp>/<hp>/<hc> or -F[r|x|y]<lo>/<hi> */
		bool active;
		double lc, lp, hp, hc;
	} F;
	struct GRDFFT_G {	/* -G<outfile> */
		bool active;
		char *file;
	} G;
	struct GRDFFT_I {	/* -I[<scale>|g] */
		bool active;
		double value;
	} I;
	struct GRDFFT_N {	/* -N[f|q|s<n_columns>/<n_rows>][+e|m|n][+t<width>][+w[<suffix>]][+z[p]] */
		bool active;
		struct GMT_FFT_INFO *info;
	} N;
	struct GRDFFT_S {	/* -S<scale> */
		bool active;
		double scale;
	} S;
	/* Now in gravfft in potential supplement; left for backwards compatibility */
	struct GRDFFT_T {	/* -T<te/rl/rm/rw/ri> */
		bool active;
		double te, rhol, rhom, rhow, rhoi;
	} T;
};

enum Grdfft_operators {
	GRDFFT_UP_DOWN_CONTINUE	= 0,
	GRDFFT_AZIMUTHAL_DERIVATIVE,
	GRDFFT_DIFFERENTIATE	   ,
	GRDFFT_INTEGRATE	   ,
	GRDFFT_FILTER_EXP	   ,
	GRDFFT_FILTER_BW	   ,
	GRDFFT_FILTER_COS	   ,
	GRDFFT_SPECTRUM		   ,
	GRDFFT_ISOSTASY		   ,
	GRDFFT_NOTHING		   };

#define	MGAL_AT_45	980619.9203 	/* Moritz's 1980 IGF value for gravity in mGal at 45 degrees latitude */

struct F_INFO {
	double lc[3];		/* Low-cut frequency for r, x, and y	*/
	double lp[3];		/* Low-pass frequency for r, x, and y	*/
	double hp[3];		/* High-pass frequency for r, x, and y	*/
	double hc[3];		/* High-cut frequency for r, x, and y	*/
	double ltaper[3];	/* Low taper width for r, x, and y	*/
	double htaper[3];	/* High taper width for r, x, and y	*/
	double llambda[3];	/* Low full-wavelength where Gauss amp = 0.5 for r, x, and y	*/
	double hlambda[3];	/* High full-wavelength where Gauss amp = 0.5  for r, x, and y	*/
	double bw_order;	/* Order, N, of Butterworth filter	*/
	double (*filter) (struct F_INFO *, double, int);	/* Points to the correct filter function */

	bool set_already;	/* true if we already filled in the structure */
	unsigned int k_type;	/* Which of the r, x, or y wavenumbers we need */
	unsigned int kind;	/* GRDFFT_FILTER_EXP, GRDFFT_FILTER_BW, GRDFFT_FILTER_COS  */
	unsigned int arg;	/* 0 = Gaussian, 1 = Butterworth, 2 = cosine taper,  */
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDFFT_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDFFT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->S.scale = 1.0;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDFFT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C->operation);
	gmt_M_free (GMT, C->par);
	gmt_M_str_free (C->In.file[GMT_IN]);
	gmt_M_str_free (C->In.file[GMT_OUT]);
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C->N.info);
	gmt_M_free (GMT, C);
}

GMT_LOCAL unsigned int do_differentiate (struct GMT_GRID *Grid, double *par, struct GMT_FFT_WAVENUMBER *K) {
	uint64_t k;
	double scale, fact;
	gmt_grdfloat *datac = Grid->data;	/* Shorthand */

	/* Differentiate in frequency domain by multiplying by kr [scale optional] */

	scale = (*par != 0.0) ? *par : 1.0;
	datac[0] = datac[1] = 0.0f;	/* Derivative of the mean is zero */
	for (k = 2; k < Grid->header->size; k += 2) {
		fact = scale * gmt_fft_get_wave (k, K);
		datac[k]   *= (gmt_grdfloat)fact;
		datac[k+1] *= (gmt_grdfloat)fact;
	}
	return (1);	/* Number of parameters used */
}

GMT_LOCAL unsigned int do_integrate (struct GMT_GRID *Grid, double *par, struct GMT_FFT_WAVENUMBER *K) {
	/* Integrate in frequency domain by dividing by kr [scale optional] */
	uint64_t k;
	double fact, scale;
	gmt_grdfloat *datac = Grid->data;	/* Shorthand */

	scale = (*par != 0.0) ? *par : 1.0;
	datac[0] = datac[1] = 0.0f;
	for (k = 2; k < Grid->header->size; k += 2) {
		fact = 1.0 / (scale * gmt_fft_get_wave (k, K));
		datac[k]   *= (gmt_grdfloat)fact;
		datac[k+1] *= (gmt_grdfloat)fact;
	}
	return (1);	/* Number of parameters used */
}

GMT_LOCAL unsigned int do_continuation (struct GMT_GRID *Grid, double *zlevel, struct GMT_FFT_WAVENUMBER *K) {
	uint64_t k;
	gmt_grdfloat tmp, *datac = Grid->data;	/* Shorthand */

	/* If z is positive, the field will be upward continued using exp[- k z].  */

	for (k = 2; k < Grid->header->size; k += 2) {
		tmp = (gmt_grdfloat)exp (-(*zlevel) * gmt_fft_get_wave (k, K));
		datac[k]   *= tmp;
		datac[k+1] *= tmp;
	}
	return (1);	/* Number of parameters used */
}

GMT_LOCAL unsigned int do_azimuthal_derivative (struct GMT_GRID *Grid, double *azim, struct GMT_FFT_WAVENUMBER *K) {
	uint64_t k;
	gmt_grdfloat tempr, tempi, fact, *datac = Grid->data;	/* Shorthand */
	double cos_azim, sin_azim;

	sincosd (*azim, &sin_azim, &cos_azim);

	datac[0] = datac[1] = 0.0f;
	for (k = 2; k < Grid->header->size; k += 2) {
		fact = (gmt_grdfloat)(sin_azim * gmt_fft_any_wave (k, GMT_FFT_K_IS_KX, K) + cos_azim * gmt_fft_any_wave (k, GMT_FFT_K_IS_KY, K));
		tempr = -(datac[k+1] * fact);
		tempi =  (datac[k]   * fact);
		datac[k]   = tempr;
		datac[k+1] = tempi;
	}
	return (1);	/* Number of parameters used */
}

/* Now obsolete but left for backwards compatibility.  Users are encouraged to use gravfft */
#define	POISSONS_RATIO	0.25
#define	YOUNGS_MODULUS	1.0e11		/* Pascal = Nt/m**2  */
#define	NORMAL_GRAVITY	9.806199203	/* m/s**2  */

GMT_LOCAL unsigned int do_isostasy (struct GMT_GRID *Grid, struct GRDFFT_CTRL *Ctrl, double *par, struct GMT_FFT_WAVENUMBER *K) {
	/* Do the isostatic response function convolution in the Freq domain.
	All units assumed to be in SI (that is kx, ky, modk wavenumbers in m**-1,
	densities in kg/m**3, Te in m, etc.
	rw, the water density, is used to set the Airy ratio and the restoring
	force on the plate (rm - ri)*gravity if ri = rw; so use zero for topo in air.  */
	uint64_t k;
	double airy_ratio, rigidity_d, d_over_restoring_force, mk, k2, k4, transfer_fn;

	double te;	/* Elastic thickness, SI units (m)  */
	double rl;	/* Load density, SI units  */
	double rm;	/* Mantle density, SI units  */
	double rw;	/* Water density, SI units  */
	double ri;	/* Infill density, SI units  */

	gmt_grdfloat *datac = Grid->data;	/* Shorthand */

	te = par[0];	rl = par[1];	rm = par[2];	rw = par[3];	ri = par[4];
	airy_ratio = -(rl - rw)/(rm - ri);

	if (te == 0.0) {	/* Airy isostasy; scale variable Ctrl->S.scale and return */
		Ctrl->S.scale *= airy_ratio;
		return (5);	/* Number of parameters used */
	}

	rigidity_d = (YOUNGS_MODULUS * pow (te, 3.0)) / (12.0 * (1.0 - POISSONS_RATIO * POISSONS_RATIO));
	d_over_restoring_force = rigidity_d / ((rm - ri) * NORMAL_GRAVITY);

	for (k = 0; k < Grid->header->size; k += 2) {
		mk = gmt_fft_get_wave (k, K);
		k2 = mk * mk;
		k4 = k2 * k2;
		transfer_fn = airy_ratio / ((d_over_restoring_force * k4) + 1.0);
		datac[k]   *= (gmt_grdfloat)transfer_fn;
		datac[k+1] *= (gmt_grdfloat)transfer_fn;
	}
	return (5);	/* Number of parameters used */
}

#ifndef M_LN2
#define M_LN2			0.69314718055994530942  /* log_e 2 */
#endif

GMT_LOCAL double gauss_weight (struct F_INFO *f_info, double freq, int j) {
	double hi, lo;
	lo = (f_info->llambda[j] == -1.0) ? 0.0 : exp (-M_LN2 * pow (freq * f_info->llambda[j], 2.0));	/* Low-pass part */
	hi = (f_info->hlambda[j] == -1.0) ? 1.0 : exp (-M_LN2 * pow (freq * f_info->hlambda[j], 2.0));	/* Hi-pass given by its complementary low-pass */
	return (hi - lo);
}

GMT_LOCAL double bw_weight (struct F_INFO *f_info, double freq, int j) {	/* Butterworth filter */
	double hi, lo;
	lo = (f_info->llambda[j] == -1.0) ? 0.0 : sqrt (1.0 / (1.0 + pow (freq * f_info->llambda[j], f_info->bw_order)));	/* Low-pass part */
	hi = (f_info->hlambda[j] == -1.0) ? 1.0 : sqrt (1.0 / (1.0 + pow (freq * f_info->hlambda[j], f_info->bw_order)));	/* Hi-pass given by its complementary low-pass */
	return (hi - lo);
}

GMT_LOCAL double cosine_weight_grdfft (struct F_INFO *f_info, double freq, int j) {
	if (freq <= f_info->lc[j] || freq >= f_info->hc[j]) return(0.0);	/* In fully cut range.  Weight is zero.  */
	if (freq > f_info->lc[j] && freq < f_info->lp[j]) return (0.5 * (1.0 + cos (M_PI * (freq - f_info->lp[j]) * f_info->ltaper[j])));
	if (freq > f_info->hp[j] && freq < f_info->hc[j]) return (0.5 * (1.0 + cos (M_PI * (freq - f_info->hp[j]) * f_info->htaper[j])));
	return (1.0);	/* Freq is in the fully passed range, so weight is multiplied by 1.0  */
}

GMT_LOCAL double get_filter_weight (uint64_t k, struct F_INFO *f_info, struct GMT_FFT_WAVENUMBER *K) {
	double freq, return_value;

	freq = gmt_fft_any_wave (k, f_info->k_type, K);
	return_value = f_info->filter (f_info, freq, f_info->k_type);

	return (return_value);
}

GMT_LOCAL void do_filter (struct GMT_GRID *Grid, struct F_INFO *f_info, struct GMT_FFT_WAVENUMBER *K) {
	uint64_t k;
	gmt_grdfloat weight, *datac = Grid->data;	/* Shorthand */

	for (k = 0; k < Grid->header->size; k += 2) {
		weight = (gmt_grdfloat) get_filter_weight (k, f_info, K);
		datac[k]   *= weight;
		datac[k+1] *= weight;
	}
}

GMT_LOCAL int do_spectrum (struct GMT_CTRL *GMT, struct GMT_GRID *GridX, struct GMT_GRID *GridY, double *par, bool give_wavelength, bool km, bool normalize, char *file, struct GMT_FFT_WAVENUMBER *K) {
	/* Compute [cross-]spectral estimates from the two grids X and Y and return frequency f and 8 quantities:
	 * Xpower[f], Ypower[f], coherent power[f], noise power[f], phase[f], admittance[f], gain[f], coherency[f].
	 * Each quantity comes with its own 1-std dev error estimate, hence output is 17 columns.  If GridY == NULL
	 * then just XPower[f] and its 1-std dev error estimate are returned, hence just 3 columns.
	 * Equations based on spectrum1d.c */

	uint64_t dim[GMT_DIM_SIZE] = {1, 1, 0, 0};	/* One table and one segment, with either 1 + 1*2 = 3 or 1 + 8*2 = 17 columns and yet unknown rows */
	uint64_t k, nk, ifreq, *nused = NULL;
	unsigned int col;
	gmt_grdfloat *X = GridX->data, *Y = NULL;	/* Short-hands */
	double delta_k, r_delta_k, freq, coh_k, sq_norm, powfactor, k_pow_factor, tmp, eps_pow;
	double *X_pow = NULL, *Y_pow = NULL, *co_spec = NULL, *quad_spec = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;

	dim[GMT_COL] = (GridY) ? 17 : 3;	/* Either 3 or 17 estimates */
#ifdef DEBUG
	if (show_n) dim[GMT_COL]++;		/* Also write out nused[k] so add one more column */
#endif
	if (*par > 0.0) {	/* X spectrum desired  */
		delta_k = K->delta_kx;	nk = K->nx2 / 2;
		gmt_fft_set_wave (GMT, GMT_FFT_K_IS_KX, K);
	}
	else if (*par < 0.0) {	/* Y spectrum desired  */
		delta_k = K->delta_ky;	nk = K->ny2 / 2;
		gmt_fft_set_wave (GMT, GMT_FFT_K_IS_KY, K);
	}
	else {	/* R spectrum desired  */
		if (K->delta_kx < K->delta_ky) {
			delta_k = K->delta_kx;	nk = K->nx2 / 2;
		}
		else {
			delta_k = K->delta_ky;	nk = K->ny2 / 2;
		}
		gmt_fft_set_wave (GMT, GMT_FFT_K_IS_KR, K);
	}

	/* Get arrays for summing stuff */
	X_pow = gmt_M_memory (GMT, NULL, nk, double );
	nused = gmt_M_memory (GMT, NULL, nk, uint64_t);
	if (GridY) {	/* For cross-spectral estimates */
		Y = GridY->data;	/* Shorthand for Y data */
		Y_pow     = gmt_M_memory (GMT, NULL, nk, double);
		co_spec   = gmt_M_memory (GMT, NULL, nk, double);
		quad_spec = gmt_M_memory (GMT, NULL, nk, double);
	}

	/* Loop over it all, summing and storing, checking range for r */

	r_delta_k = 1.0 / delta_k;

	for (k = 2; k < GridX->header->size; k += 2) {
		freq = gmt_fft_get_wave (k, K);
		ifreq = lrint (fabs (freq) * r_delta_k);	/* Smallest value returned might be 0 when doing r spectrum*/
		if (ifreq > 0) --ifreq;
		if (ifreq >= nk) continue;	/* Might happen when doing r spectrum  */
		X_pow[ifreq]     += (X[k]   * X[k] + X[k+1] * X[k+1]);	/* X x X* = Power of grid X */
		nused[ifreq]++;
		if (GridY) {	/* For cross-spectral estimates */
			Y_pow[ifreq]     += (Y[k]   * Y[k] + Y[k+1] * Y[k+1]);	/* Y x Y* = Power of grid Y */
			co_spec[ifreq]   += (Y[k]   * X[k] + Y[k+1] * X[k+1]);	/* Real part of Y x X* */
			quad_spec[ifreq] += (X[k+1] * Y[k] - Y[k+1] * X[k]);	/* Imag part of Y x X* */
		}
	}

	gmt_set_cartesian (GMT, GMT_OUT);	/* To counter-act any -fg setting */

	delta_k /= (2.0 * M_PI);	/* Write out frequency, not wavenumber  */
	powfactor = 4.0 / pow ((double)GridX->header->size, 2.0);	/* Squared normalization of FFT */
	dim[GMT_ROW] = nk;
	if ((D = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to create a data set for spectral estimates\n");
		gmt_M_free (GMT, X_pow);
		gmt_M_free (GMT, Y_pow);
		gmt_M_free (GMT, nused);
		gmt_M_free (GMT, co_spec);
		gmt_M_free (GMT, quad_spec);
		return (GMT->parent->error);
	}
	S = D->table[0]->segment[0];	/* Only one table with one segment here, with 17 cols and nk rows */
	S->n_rows = nk;
	if (give_wavelength && km) delta_k *= 1000.0;	/* Wanted distances measured in km */

	k_pow_factor = powfactor;	/* Standard normalization */
	for (k = 0; k < nk; k++) {
		eps_pow = 1.0 / sqrt ((double)nused[k]);	/* Multiplicative error bars for power spectra  */
		freq = (k + 1) * delta_k;
		if (give_wavelength) freq = 1.0 / freq;

		if (normalize) k_pow_factor = powfactor / (double)nused[k];
		X_pow[k] *= k_pow_factor;

		col = 0;
		/* Col 0 is the frequency (or wavelength) */
		S->data[col++][k] = freq;
		/* Cols 1-2 are xpower and std.err estimate */
		S->data[col++][k] = X_pow[k];
		S->data[col++][k] = X_pow[k] * eps_pow;

		if (!GridY) {	/* Nothing more to do (except add nused[k] if true and debug) */
#ifdef DEBUG
			if (show_n) S->data[col][k] = (double)nused[k];
#endif
			continue;
		}

		Y_pow[k]     *= k_pow_factor;
		co_spec[k]   *= k_pow_factor;
		quad_spec[k] *= k_pow_factor;
		/* Compute coherence first since it is needed by many of the other estimates */
		coh_k = (co_spec[k] * co_spec[k] + quad_spec[k] * quad_spec[k]) / (X_pow[k] * Y_pow[k]);
		sq_norm = sqrt ((1.0 - coh_k) / (2.0 * coh_k));	/* Save repetitive expression further down */
		/* Cols 3-4 are ypower and std.err estimate */
		S->data[col++][k] = Y_pow[k];
		S->data[col++][k] = Y_pow[k] * eps_pow;
		/* Cols 5-6 are coherent power and std.err estimate */
		S->data[col++][k] = tmp = Y_pow[k] * coh_k;
		S->data[col++][k] = tmp * eps_pow * sqrt ((2.0 - coh_k) / coh_k);
		/* Cols 7-8 are noise power and std.err estimate */
		S->data[col++][k] = tmp = Y_pow[k] * (1.0 - coh_k);
		S->data[col++][k] = tmp * eps_pow;
		/* Cols 9-10 are phase and std.err estimate */
		S->data[col++][k] = tmp = d_atan2 (quad_spec[k], co_spec[k]);
		S->data[col++][k] = tmp * eps_pow * sq_norm;
		/* Cols 11-12 are admittance and std.err estimate */
		S->data[col++][k] = tmp = co_spec[k] / X_pow[k];
		S->data[col++][k] = tmp * eps_pow * fabs (sq_norm);
		/* Cols 13-14 are gain and std.err estimate */
		S->data[col++][k] = tmp = sqrt (quad_spec[k]) / X_pow[k];
		S->data[col++][k] = tmp * eps_pow * sq_norm;
		/* Cols 15-16 are coherency and std.err estimate */
		S->data[col++][k] = coh_k;
		S->data[col++][k] = coh_k * eps_pow * (1.0 - coh_k) * sqrt (2.0 / coh_k);
#ifdef DEBUG
		if (show_n) S->data[col][k] = (double)nused[k];
#endif
	}

	if (GMT->common.h.add_colnames) {
		char header[GMT_BUFSIZ] = {""}, *name[2] = {"freq", "wlength"};
		if (GridY) {	/* Long header record - number in [] is GMT column; useful for -i option */
			sprintf (header, "%s[0]\txpow[1]\tstd_xpow[2]\typow[3]\tstd_ypow[4]\tcpow[5]\tstd_cpow[6]\tnpow[7]\tstd_npow[8]\t" \
			"phase[9]\tstd_phase[10]\tadm[11]\tstd_ad[12]\tgain[13]\tstd_gain[14]\tcoh[15]\tstd_coh[16]", name[give_wavelength]);
		}
		else
			sprintf (header, "%s[0]\tpow[1]\tstd_pow[2]", name[give_wavelength]);
		if (GMT_Set_Comment (GMT->parent, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, D)) return (GMT->parent->error);
	}

	if (GMT_Write_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, file, D) != GMT_NOERROR) {
		return (GMT->parent->error);
	}
	gmt_M_free (GMT, X_pow);
	gmt_M_free (GMT, nused);
	if (GridY) {
		gmt_M_free (GMT, Y_pow);
		gmt_M_free (GMT, co_spec);
		gmt_M_free (GMT, quad_spec);
	}

	return (1);	/* Number of parameters used */
}

GMT_LOCAL bool parse_f_string (struct GMT_CTRL *GMT, struct F_INFO *f_info, char *c) {
	unsigned int i, j, n_tokens, pos;
	bool descending;
	double fourvals[4];
	char line[GMT_LEN256] = {""}, p[GMT_LEN256] = {""};

	/* Syntax is either -F[r|x|y]lc/hc/lp/hp (Cosine taper), -F[r|x|y]lo/hi (Gaussian), or -F[r|x|y]lo/hi/order (Butterworth) */

	strncpy (line, c,  GMT_LEN256-1);
	i =  0;
	f_info->k_type = GMT_FFT_K_IS_KR;	/* j is Filter type: r=2, x=0, y=1  [r] */

	if (line[i] == 'r') {
		i++;	f_info->k_type = GMT_FFT_K_IS_KR;
	}
	else if (line[i] == 'x') {
		i++;	f_info->k_type = GMT_FFT_K_IS_KX;
	}
	else if (line[i] == 'y') {
		i++;	f_info->k_type = GMT_FFT_K_IS_KY;
	}
	fourvals[0] = fourvals[1] = fourvals[2] = fourvals[3] = -1.0;

	n_tokens = pos = 0;
	while ((gmt_strtok (&line[i], "/", &pos, p))) {
		if (n_tokens > 3) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Too many slashes in -F.\n");
			return (true);
		}
		if(p[0] == '-')
			fourvals[n_tokens] = -1.0;
		else {
			if ((sscanf(p, "%lf", &fourvals[n_tokens])) != 1) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, " Cannot read token %d.\n", n_tokens);
				return (true);
			}
		}
		n_tokens++;
	}

	if (!(n_tokens == 2 || n_tokens == 3 || n_tokens == 4)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-F Cannot find 2-4 tokens separated by slashes.\n");
		return (true);
	}
	descending = true;
	if (f_info->kind == GRDFFT_FILTER_BW && n_tokens == 3) n_tokens = 2;	/* So we don't check the order as a wavelength */

	for (i = 1; i < n_tokens; i++) {
		if (fourvals[i] == -1.0 || fourvals[i-1] == -1.0) continue;
		if (fourvals[i] > fourvals[i-1]) descending = false;
	}
	if (!(descending)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-F Wavelengths are not in descending order.\n");
		return (true);
	}
	j = f_info->k_type;
	if (f_info->kind == GRDFFT_FILTER_COS) {	/* Cosine band-pass specification */
		if ((fourvals[0] * fourvals[1]) < 0.0 || (fourvals[2] * fourvals[3]) < 0.0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-F Pass/Cut specification error.\n");
			return (true);
		}

		/* Now everything is OK  */

		if (fourvals[0] >= 0.0 || fourvals[1] >= 0.0) {	/* Lower end values are set  */
			f_info->lc[j] = (2.0 * M_PI)/fourvals[0];
			f_info->lp[j] = (2.0 * M_PI)/fourvals[1];
			if (fourvals[0] != fourvals[1]) f_info->ltaper[j] = 1.0/(f_info->lc[j] - f_info->lp[j]);
		}

		if (fourvals[2] >= 0.0 || fourvals[3] >= 0.0) {	/* Higher end values are set  */
			f_info->hp[j] = (2.0 * M_PI)/fourvals[2];
			f_info->hc[j] = (2.0 * M_PI)/fourvals[3];
			if (fourvals[2] != fourvals[3]) f_info->htaper[j] = 1.0/(f_info->hc[j] - f_info->hp[j]);
		}
		f_info->filter = &cosine_weight_grdfft;
	}
	else if (f_info->kind == GRDFFT_FILTER_BW) {	/* Butterworth specification */
		f_info->llambda[j] = (fourvals[0] == -1.0) ? -1.0 : fourvals[0] / TWO_PI;	/* TWO_PI is used to counteract the 2*pi in the wavenumber */
		f_info->hlambda[j] = (fourvals[1] == -1.0) ? -1.0 : fourvals[1] / TWO_PI;
		f_info->bw_order = 2.0 * fourvals[2];
		f_info->filter = &bw_weight;
	}
	else {	/* Gaussian half-amp specifications */
		f_info->llambda[j] = (fourvals[0] == -1.0) ? -1.0 : fourvals[0] / TWO_PI;	/* TWO_PI is used to counteract the 2*pi in the wavenumber */
		f_info->hlambda[j] = (fourvals[1] == -1.0) ? -1.0 : fourvals[1] / TWO_PI;
		f_info->filter = &gauss_weight;
	}
	f_info->arg = f_info->kind - GRDFFT_FILTER_EXP;
	return (false);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <ingrid> [<ingrid2>] [-G<outgrid>|<table>] [-A<azimuth>] [-C<zlevel>]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-D[<scale>|g]] [-E[r|x|y][+w[k]][+n]] [-F[r|x|y]<parameters>] [-I[<scale>|g]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-N%s] [-S<scale>]\n", GMT_FFT_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-fg] [%s] [%s]\n\n", GMT_V_OPT, GMT_ho_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is the input grid file.  For cross-spectrum also supply <ingrid2>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Take azimuthal derivative along line <azimuth> degrees CW from North.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Continue field upward (+) or downward (-) to <zlevel> (meters).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Differentiate, i.e., multiply by kr [ * scale].  Use -Dg to get mGal from m].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Estimate spEctrum in the radial r [Default], x, or y-direction.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Given one grid X, write f, Xpower[f] to output file (see -G) or stdout.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Given two grids X and Y, write f, Xpower[f], Ypower[f], coherent power[f],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   noise power[f], phase[f], admittance[f], gain[f], coherency[f].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Each quantity is followed by a column of 1 std dev. error estimates.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give +w to write wavelength instead of frequency; append k to report\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   wavelength in km (geographic grids only) [m].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n to yield mean power instead of total power per frequency.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Filter r [x] [y] freq according to one of three kinds of filter specifications:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a) Cosine band-pass: Append four wavelengths <lc>/<lp>/<hp>/<hc>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      frequencies outside <lc>/<hc> are cut; inside <lp>/<hp> are passed, rest are tapered.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Replace wavelength by - to skip, e.g., -F-/-/500/100 is a low-pass filter.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   b) Gaussian band-pass: Append two wavelengths <lo>/<hi> where filter amplitudes = 0.5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Replace wavelength by - to skip, e.g., -F300/- is a high-pass Gaussian filter.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c) Butterworth band-pass: Append two cut-off wavelengths and order, i.e., <lo>/<hi>/<order>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      where filter amplitudes are 0.707.  Replace wavelength by - to skip, e.g.,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      try -F300/-/2 for a high-pass 2nd-order Butterworth filter.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G filename for output netCDF grid file OR 1-D data table (see -E).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optional for -E (spectrum written to stdout); required otherwise.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Integrate, i.e., divide by kr [ * scale].  Use -Ig to get m from mGal].\n");
	GMT_FFT_Option (API, 'N', GMT_FFT_DIM, "Choose or inquire about suitable grid dimensions for FFT, and set modifiers.");
	GMT_Message (API, GMT_TIME_NONE, "\t-S multiply field by scale after inverse FFT [1.0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give -Sd to convert deflection of vertical to micro-radians.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Convert geographic grids to meters using a \"Flat Earth\" approximation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-ho Write header record for spectral estimates (requires -E) [no header].\n");
	GMT_Option (API, ".");
	GMT_Message (API, GMT_TIME_NONE, "\tList operations in the order desired for execution.\n");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL void add_operation (struct GMT_CTRL *GMT, struct GRDFFT_CTRL *Ctrl, int operation, unsigned int n_par, double *par) {
	Ctrl->n_op_count++;
	Ctrl->operation = gmt_M_memory (GMT, Ctrl->operation, Ctrl->n_op_count, int);
	Ctrl->operation[Ctrl->n_op_count-1] = operation;
	if (n_par) {
		Ctrl->par = gmt_M_memory (GMT, Ctrl->par, Ctrl->n_par + n_par, double);
		gmt_M_memcpy (&Ctrl->par[Ctrl->n_par], par, n_par, double);
		Ctrl->n_par += n_par;
	}
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDFFT_CTRL *Ctrl, struct F_INFO *f_info, struct GMT_OPTION *options) {
	/* This parses the options provided to grdfft and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int j, k, pos, n_errors = 0, filter_type = 0;
	int n_scan;
	double par[5];
	char combined[GMT_BUFSIZ] = {""}, argument[GMT_LEN16] = {""}, p[GMT_LEN64] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL, *ptr = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	if (gmt_M_compat_check (GMT, 4)) {
		char *mod = NULL;
		if ((ptr = GMT_Find_Option (API, 'L', options))) {	/* Gave old -L */
			mod = ptr->arg; /* Gave old -L option */
			if (mod[0] == '\0') strcat (argument, "+l");		/* Leave trend alone -L */
			else if (mod[0] == 'm') strcat (argument, "+a");	/* Remove mean -Lm */
			else if (mod[0] == 'h') strcat (argument, "+h");	/* Remove mid-value -Lh */
		}
	}

	gmt_M_memset (f_info, 1, struct F_INFO);
	for (j = 0; j < 3; j++) {
		f_info->lc[j] = f_info->lp[j] = -1.0;		/* Set negative, below valid frequency range  */
		f_info->hp[j] = f_info->hc[j] = DBL_MAX;	/* Set huge positive, above valid frequency range  */
	}

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */
		gmt_M_memset (par, 5, double);

		switch (opt->option) {
			case '<':	/* Input file (only 1 or 2 are accepted) */
				Ctrl->In.active = true;
				if (Ctrl->In.n_grids >= 2) {
					n_errors++;
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: A maximum of two input grids may be processed\n");
				}
				else if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					Ctrl->In.file[Ctrl->In.n_grids++] = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Directional derivative */
				Ctrl->A.active = true;
				n_errors += gmt_M_check_condition (GMT, sscanf(opt->arg, "%lf", &par[0]) != 1,
						"Syntax error -A option: Cannot read azimuth\n");
				add_operation (GMT, Ctrl, GRDFFT_AZIMUTHAL_DERIVATIVE, 1, par);
				break;
			case 'C':	/* Upward/downward continuation */
				Ctrl->C.active = true;
				n_errors += gmt_M_check_condition (GMT, sscanf(opt->arg, "%lf", &par[0]) != 1,
						"Syntax error -C option: Cannot read zlevel\n");
				add_operation (GMT, Ctrl, GRDFFT_UP_DOWN_CONTINUE, 1, par);
				break;
			case 'D':	/* d/dz */
				Ctrl->D.active = true;
				par[0] = (opt->arg[0]) ? ((opt->arg[0] == 'g' || opt->arg[0] == 'G') ? MGAL_AT_45 : atof (opt->arg)) : 1.0;
				n_errors += gmt_M_check_condition (GMT, par[0] == 0.0, "Syntax error -D option: scale must be nonzero\n");
				add_operation (GMT, Ctrl, GRDFFT_DIFFERENTIATE, 1, par);
				break;
			case 'E':	/* x,y,or radial spectrum, w for wavelength; k for km if geographical, n for normalize */
				/* Old syntax: -E[x|y|r][w[k]][n]  new syntax: -E[x|y|r][+w[k]][+n] */
				Ctrl->E.active = true;
				j = 0;
				switch (opt->arg[0]) {	/* Determine direction for spectrum or default to radial */
					case 'r': Ctrl->E.mode =  0; j = 1; break;
					case 'x': Ctrl->E.mode = +1; j = 1;  break;
					case 'y': Ctrl->E.mode = -1; j = 1;  break;
					default :Ctrl->E.mode =  0;  break;
				}
				if ((c = gmt_first_modifier (GMT, opt->arg, "wn"))) {	/* Process new modifiers [+w[k]][+n] */
					pos = 0;	/* Reset to start of new word */
					while (gmt_getmodopt (GMT, 'E', c, "wn", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'w': Ctrl->E.give_wavelength = true; if (p[1] == 'k') Ctrl->E.km = true; break;
							case 'n': Ctrl->E.normalize = true; break;
							default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
				}
				else {	/* Old-style modifiers [w[k]][n] */
					while (opt->arg[j]) {
						switch (opt->arg[j]) {
							case 'w': Ctrl->E.give_wavelength = true; break;
							case 'k': if (Ctrl->E.give_wavelength) Ctrl->E.km = true; break;
							case 'n': Ctrl->E.normalize = true; break;
						}
						j++;
					}
				}
				par[0] = Ctrl->E.mode;
				add_operation (GMT, Ctrl, GRDFFT_SPECTRUM, 1, par);
				break;
			case 'F':	/* Filter */
				Ctrl->F.active = true;
				if (!(f_info->set_already)) {
					filter_type = gmtlib_count_char (GMT, opt->arg, '/');
					f_info->kind = GRDFFT_FILTER_EXP + (filter_type - 1);
					f_info->set_already = true;
					add_operation (GMT, Ctrl, f_info->kind, 0, NULL);
				}
				n_errors += gmt_M_check_condition (GMT, parse_f_string (GMT, f_info, opt->arg), "Syntax error -F option");
				break;
			case 'G':	/* Output file */
				Ctrl->G.active = true;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':	/* Integrate */
				Ctrl->I.active = true;
				par[0] = (opt->arg[0] == 'g' || opt->arg[0] == 'G') ? MGAL_AT_45 : atof (opt->arg);
				n_errors += gmt_M_check_condition (GMT, par[0] == 0.0, "Syntax error -I option: scale must be nonzero\n");
				add_operation (GMT, Ctrl, GRDFFT_INTEGRATE, 1, par);
				break;
			case 'L':	/* Leave trend alone */
				if (gmt_M_compat_check (GMT, 4))
					GMT_Report (API, GMT_MSG_COMPAT, "Option -L is deprecated; use -N modifiers in the future.\n");
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'M':	/* Geographic data */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -M is deprecated; -fg was set instead, use this in the future.\n");
					if (gmt_M_is_cartesian (GMT, GMT_IN))
						gmt_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'N':	/* Grid dimension setting or inquiery */
				Ctrl->N.active = true;
				if (ptr && gmt_M_compat_check (GMT, 4)) {	/* Got both old -L and -N; append */
					sprintf (combined, "%s%s", opt->arg, argument);
					Ctrl->N.info = GMT_FFT_Parse (API, 'N', GMT_FFT_DIM, combined);
				}
				else
					Ctrl->N.info = GMT_FFT_Parse (API, 'N', GMT_FFT_DIM, opt->arg);
				if (Ctrl->N.info == NULL) n_errors++;
				break;
			case 'S':	/* Scale */
				Ctrl->S.active = true;
				Ctrl->S.scale = (opt->arg[0] == 'd' || opt->arg[0] == 'D') ? 1.0e6 : atof (opt->arg);
				break;
			case 'T':	/* Flexural isostasy */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT,
					            "Option -T is deprecated; see gravfft for isostasy and gravity calculations.\n");
					Ctrl->T.active = true;
					n_scan = sscanf (opt->arg, "%lf/%lf/%lf/%lf/%lf", &par[0], &par[1], &par[2], &par[3], &par[4]);
					for (j = 1, k = 0; j < 5; j++) if (par[j] < 0.0) k++;
					n_errors += gmt_M_check_condition (GMT, n_scan != 5 || k > 0,
						"Syntax error -T option: Correct syntax:\n\t-T<te>/<rhol>/<rhom>/<rhow>/<rhoi>, all densities >= 0\n");
					add_operation (GMT, Ctrl, GRDFFT_ISOSTASY, 5, par);
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
#ifdef DEBUG
			case '=':	/* Do nothing */
				add_operation (GMT, Ctrl, GRDFFT_NOTHING, 1, par);
				if (opt->arg[0] == '+') show_n = true;
				break;
#endif
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	if (gmt_M_compat_check (GMT, 4) && !Ctrl->N.active && ptr) {	/* User set -L but no -N so nothing got appended above... Sigh...*/
		Ctrl->N.info = GMT_FFT_Parse (API, 'N', GMT_FFT_DIM, argument);
	}
	if (Ctrl->N.info && Ctrl->N.active && Ctrl->N.info->info_mode == GMT_FFT_LIST)
		return (GMT_PARSE_ERROR);	/* So that we exit the program */

	if (Ctrl->N.info && gmt_M_compat_check (GMT, 4) && Ctrl->T.active)
		Ctrl->N.info->trend_mode = GMT_FFT_REMOVE_NOTHING;

	n_errors += gmt_M_check_condition (GMT, !(Ctrl->n_op_count), "Syntax error: Must specify at least one operation\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.scale == 0.0, "Syntax error -S option: scale must be nonzero\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file[0], "Syntax error: Must specify input file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->E.active && !Ctrl->G.file, "Syntax error -G option: Must specify output grid file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdfft (void *V_API, int mode, void *args) {
	int error = 0, status;
	unsigned int op_count = 0, par_count = 0, k;
	char *spec_msg[2] = {"spectrum", "cross-spectrum"};
	struct GMT_GRID *Grid[2] = {NULL,  NULL}, *Orig[2] = {NULL, NULL};
	struct F_INFO f_info;
	struct GMT_FFT_WAVENUMBER *FFT_info[2] = {NULL, NULL}, *K = NULL;
	struct GRDFFT_CTRL *Ctrl = NULL;
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
	if ((error = parse (GMT, Ctrl, &f_info, options)) != 0) Return (error);

	/*---------------------------- This is the grdfft main code ----------------------------*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input grid(s)\n");
	for (k = 0; k < Ctrl->In.n_grids; k++) {	/* First read the grid header(s) */
		if ((Orig[k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL)
			Return (API->error);
	}

	if (Ctrl->In.n_grids == 2) {	/* If given 2 grids, make sure they are co-registered and has same size, registration, etc. */
		if(Orig[0]->header->registration != Orig[1]->header->registration) {
			GMT_Report (API, GMT_MSG_NORMAL, "The two grids have different registrations!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (!gmt_M_grd_same_shape (GMT, Orig[0], Orig[1])) {
			GMT_Report (API, GMT_MSG_NORMAL, "The two grids have different dimensions\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (!gmt_M_grd_same_region (GMT, Orig[0], Orig[1])) {
			GMT_Report (API, GMT_MSG_NORMAL, "The two grids have different regions\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (!gmt_M_grd_same_inc (GMT, Orig[0], Orig[1])) {
			GMT_Report (API, GMT_MSG_NORMAL, "The two grids have different intervals\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	/* Grids are compatible. Initialize FFT structs, grid headers, read data, and check for NaNs */

	for (k = 0; k < Ctrl->In.n_grids; k++) {	/* Read, and check that no NaNs are present in either grid */
		/* Note: If input grid(s) are read-only then we must duplicate them; otherwise Grid[k] points to Orig[k]
		 * From here we address the first grid via Grid[0] and the 2nd grid (if given) as Grid[1];
	 	 * we are done with using the addresses Orig[k] directly. */
		(void) gmt_set_outgrid (GMT, Ctrl->In.file[k], false, Orig[k], &Grid[k]);
		FFT_info[k] = GMT_FFT_Create (API, Grid[k], GMT_FFT_DIM, GMT_GRID_IS_COMPLEX_REAL, Ctrl->N.info);
	}
	K = FFT_info[0];	/* We only need one of these anyway; K is a shorthand */

#ifdef FTEST
	/* PW: Used with -DFTEST to check that the radial filters compute correctly */
	{
		double f = 0.0;
		while (f < 3.0) {
			printf ("%g\t%g\n", f, f_info.filter (f, 0));	/* Radial filter */
			f += 0.01;
		}
		exit (-1);
	}
#endif

	for (k = 0; k < Ctrl->In.n_grids; k++) {	/* Call the forward FFT, once per grid, optionally save raw FFT output */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "forward FFT...\n");
		if (GMT_FFT (API, Grid[k], GMT_FFT_FWD, GMT_FFT_COMPLEX, FFT_info[k]))
			Return (GMT_RUNTIME_ERROR);
	}

	for (op_count = par_count = 0; op_count < Ctrl->n_op_count; op_count++) {
		switch (Ctrl->operation[op_count]) {
			case GRDFFT_UP_DOWN_CONTINUE:
				if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE))
					((Ctrl->par[par_count] < 0.0) ? GMT_Message (API, GMT_TIME_NONE, "downward continuation...\n") :
					                                GMT_Message (API, GMT_TIME_NONE,  "upward continuation...\n"));
				par_count += do_continuation (Grid[0], &Ctrl->par[par_count], K);
				break;
			case GRDFFT_AZIMUTHAL_DERIVATIVE:
				if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "azimuthal derivative...\n");
				par_count += do_azimuthal_derivative (Grid[0], &Ctrl->par[par_count], K);
				break;
			case GRDFFT_DIFFERENTIATE:
				if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "differentiate...\n");
				par_count += do_differentiate (Grid[0], &Ctrl->par[par_count], K);
				break;
			case GRDFFT_INTEGRATE:
				if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "integrate...\n");
				par_count += do_integrate (Grid[0], &Ctrl->par[par_count], K);
				break;
			case GRDFFT_ISOSTASY:
				if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "isostasy...\n");
				par_count += do_isostasy (Grid[0], Ctrl, &Ctrl->par[par_count], K);
				break;
			case GRDFFT_FILTER_COS:
				if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "cosine filter...\n");
				do_filter (Grid[0], &f_info, K);
				break;
			case GRDFFT_FILTER_EXP:
				if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "Gaussian filter...\n");
				do_filter (Grid[0], &f_info, K);
				break;
			case GRDFFT_FILTER_BW:
				if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "Butterworth filter...\n");
				do_filter (Grid[0], &f_info, K);
				break;
			case GRDFFT_SPECTRUM:	/* This operator writes a table to file (or stdout if -G is not used) */
				if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "%s...\n", spec_msg[Ctrl->In.n_grids-1]);
				status = do_spectrum (GMT, Grid[0], Grid[1], &Ctrl->par[par_count], Ctrl->E.give_wavelength, Ctrl->E.km, Ctrl->E.normalize, Ctrl->G.file, K);
				if (status < 0) Return (status);
				par_count += status;
				break;
			case GRDFFT_NOTHING:
				break;
		}
	}

	if (!Ctrl->E.active) {	/* Since -E output is handled separately by do_spectrum itself */
		if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "inverse FFT...\n");

		if (GMT_FFT (API, Grid[0], GMT_FFT_INV, GMT_FFT_COMPLEX, K))
			Return (GMT_RUNTIME_ERROR);
#ifdef DEBUG
		gmt_grd_dump (Grid[0]->header, Grid[0]->data, false, "After Inv FFT");
#endif

		if (!doubleAlmostEqual (Ctrl->S.scale, 1.0)) gmt_scale_and_offset_f (GMT, Grid[0]->data, Grid[0]->header->size, Ctrl->S.scale, 0);

		/* The data are in the middle of the padded array; only the interior (original dimensions) will be written to file */
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid[0])) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY |
		                    GMT_GRID_IS_COMPLEX_REAL, NULL, Ctrl->G.file, Grid[0]) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	for (k = 0; k < Ctrl->In.n_grids; k++)
		GMT_FFT_Destroy (API, &(FFT_info[k]));

	Return (GMT_NOERROR);
}
