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
 *  Structures in support of FFT use.
 *
 * Author:	P. Wessel, derived from W.H.F. Smith implementation
 * Date:	12-FEB-2013
 * Version:	5 API
 *
 */

#ifndef GMT_FFT_H
#define GMT_FFT_H

enum GMT_FFT_EXTEND {
	GMT_FFT_EXTEND_POINT_SYMMETRY = 0,
	GMT_FFT_EXTEND_MIRROR_SYMMETRY,
	GMT_FFT_EXTEND_NONE,
	GMT_FFT_EXTEND_NOT_SET};
	
enum GMT_FFT_KCHOICE {
	GMT_FFT_K_IS_KR = 0,
	GMT_FFT_K_IS_KX,
	GMT_FFT_K_IS_KY};
	
enum GMT_FFT_DIMSET {
	GMT_FFT_EXTEND = 0,
	GMT_FFT_FORCE,
	GMT_FFT_SET,
	GMT_FFT_LIST,
	GMT_FFT_QUERY};

struct GMT_FFT_WAVENUMBER {	/* Holds parameters needed to calculate kx, ky, kr */
	int nx2, ny2;
	double delta_kx, delta_ky;
	double (*k_ptr) (uint64_t k, struct GMT_FFT_WAVENUMBER *K);	/* pointer to function returning either kx, ky, or kr */
};

struct GMT_FFT_INFO {
	unsigned int nx;		/* Desired hard FFT nx dimensionl or 0 if free to adjust */
	unsigned int ny;		/* Desired hard FFT ny dimensionl or 0 if free to adjust */
	unsigned int taper_mode;	/* One of the GMT_FFT_EXTEND for extension/mirroring */
	unsigned int info_mode;		/* One of the GMT_FFT_INFO for setting nx/ny or inquire */
	double taper_width;		/* Amount of tapering in percent */
};

#endif
