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
 *  Structures in support of FFT use.
 *
 * Author:	P. Wessel, derived from W.H.F. Smith implementation
 * Date:	12-FEB-2013
 * Version:	6 API
 *
 */

/*!
 * \file gmt_fft.h
 * \brief Structures in support of FFT use.
 */

#ifndef GMT_FFT_H
#define GMT_FFT_H
#ifdef __APPLE__ /* Accelerate framework */
#include <Accelerate/Accelerate.h>
#endif

enum GMT_FFT_EXTEND {
	GMT_FFT_EXTEND_POINT_SYMMETRY = 0,
	GMT_FFT_EXTEND_MIRROR_SYMMETRY,
	GMT_FFT_EXTEND_NONE,
	GMT_FFT_EXTEND_NOT_SET};
	
enum GMT_FFT_TREND {
	GMT_FFT_REMOVE_NOT_SET = -1,
	GMT_FFT_REMOVE_NOTHING = 0,
	GMT_FFT_REMOVE_MEAN,
	GMT_FFT_REMOVE_MID,
	GMT_FFT_REMOVE_TREND};
	
enum GMT_FFT_KCHOICE {
	GMT_FFT_K_IS_KX = 0,
	GMT_FFT_K_IS_KY,
	GMT_FFT_K_IS_KR};
	
enum GMT_FFT_DIMSET {
	GMT_FFT_UNSPECIFIED = 0,
	GMT_FFT_EXTEND,
	GMT_FFT_FORCE,
	GMT_FFT_SET,
	GMT_FFT_LIST};

enum GMT_FFT_SUGGEST {	/* Indices for struct GMT_FFT_SUGGESTION arrays */
	GMT_FFT_FAST = 0,
	GMT_FFT_ACCURATE,
	GMT_FFT_STORAGE,
	GMT_FFT_N_SUGGEST};

/*! Holds parameters needed to calculate kx, ky, kr */
struct GMT_FFT_WAVENUMBER {
	int nx2, ny2;
	unsigned int dim;	/* FFT dimension as setup by Init */
	double delta_kx, delta_ky;
	double (*k_ptr) (uint64_t k, struct GMT_FFT_WAVENUMBER *K);	/* pointer to function returning either kx, ky, or kr */
	double coeff[3];		/* Detrending coefficients returned, if used */
	struct GMT_FFT_INFO *info;	/* Pointer back to GMT_FFT_INFO */
};

struct GMT_FFT_INFO {
	bool set;			/* true if we parsed options; false we must take default settings */
	bool save[2];			/* save[GMT_IN] means save the input grid just before calling the FFT */
					/* save[GMT_OUT] means save the complex output grid just after calling the FFT */
	bool polar;			/* true if we are to save the complex output grid in polar form */
	bool verbose;			/* true if we are to report FFT suggestions */
	char suffix[GMT_LEN64];		/* Suffix used to form output names if save[GMT_IN] is true [tapered] */
	unsigned int n_columns;		/* Desired hard FFT n_columns dimensionl or 0 if free to adjust */
	unsigned int n_rows;		/* Desired hard FFT n_rows dimensionl or 0 if free to adjust */
	unsigned int taper_mode;	/* One of the GMT_FFT_EXTEND for extension/mirroring */
	unsigned int info_mode;		/* One of the GMT_FFT_INFO for setting n_columns/n_rows or inquire */
	unsigned int suggest;		/* Index into dimension suggestions [0 = pick best] */
	int trend_mode;			/* One of the GMT_FFT_TREND for handling detrending */
	double taper_width;		/* Amount of tapering in percent */
	struct GMT_FFT_WAVENUMBER *K;	/* Pointer to wavenumber structure */
};

struct GMT_FFT_SUGGESTION {
	unsigned int n_columns;
	unsigned int n_rows;
	size_t worksize;	/* # single-complex elements needed in work array  */
	size_t totalbytes;	/* (8*(n_columns*n_rows + worksize))  */
	double run_time;
	double rms_rel_err;
}; /* [0] holds fastest, [1] most accurate, [2] least storage  */


struct GMT_FFT_HIDDEN {	/* Items needed by various FFT packages */
	unsigned int n_1d, n_2d;	/* Bill Gates says: error C2016: C requires that a struct or union has at least one member */
#ifdef __APPLE__ /* Accelerate framework */
	FFTSetup setup_1d, setup_2d;
	DSPSplitComplex dsp_split_complex_1d;
	DSPSplitComplex dsp_split_complex_2d;
#endif
};

#endif
