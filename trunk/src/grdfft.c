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
 *  Brief synopsis: grdfft.c is a program to do various operations on
 *  grid files in the frequency domain.
 *
 * Author:	W.H.F. Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 */

#define THIS_MODULE k_mod_grdfft /* I am grdfft */

#include "gmt.h"

struct GRDFFT_CTRL {
	unsigned int n_op_count, n_par;
	int *operation;
	double *par;

	struct In {
		bool active;
		unsigned int n_grids;	/* 1 or 2 */
		char *file[2];
	} In;
	struct A {	/* -A<azimuth> */
		bool active;
		double value;
	} A;
	struct C {	/* -C<zlevel> */
		bool active;
		double value;
	} C;
	struct D {	/* -D[<scale>|g] */
		bool active;
		double value;
	} D;
	struct E {	/* -E[r|x|y][w[k]] */
		bool active;
		bool give_wavelength;
		bool km;
		int mode;	/*-1/0/+1 */
	} E;
	struct F {	/* -F[x_or_y]<lc>/<lp>/<hp>/<hc> or -F[x_or_y]<lo>/<hi> */
		bool active;
		double lc, lp, hp, hc;
	} F;
	struct G {	/* -G<outfile> */
		bool active;
		char *file;
	} G;
	struct I {	/* -I[<scale>|g] */
		bool active;
		double value;
	} I;
	struct L {	/* -L[m|h][+s] */
		bool active;
		bool debug;
		unsigned int mode;
	} L;
	struct N {	/* -N<stuff> */
		bool active;
		bool force_narray, suggest_narray, n_user_set;
		unsigned int nx2, ny2;
		unsigned int mode;	/* 0 is edge point symmetry [Default], 1 is mirroring, 2 is no extension */
		double width;	/* Tapering width in percent of margin between FFT grid and data grid [100] */
	} N;
	struct Q {	/* -Q[<prefix>] */
		bool active;
		char *prefix;
	} Q;
	struct S {	/* -S<scale> */
		bool active;
		double scale;
	} S;
	struct T {	/* -T<te/rl/rm/rw/ri> */
		bool active;
		double te, rhol, rhom, rhow, rhoi;
	} T;
	struct Z {	/* -Z[p] */
		bool active;
		unsigned int mode;
	} Z;
};

#ifndef M_LN2
#define M_LN2			0.69314718055994530942  /* log_e 2 */
#endif
#ifndef FSIGNIF
#define FSIGNIF			24
#endif

#define UP_DOWN_CONTINUE	0
#define AZIMUTHAL_DERIVATIVE	1
#define DIFFERENTIATE		2
#define INTEGRATE		3
#define ISOSTASY		4
#define FILTER_EXP		5
#define FILTER_BW		6
#define FILTER_COS		7
#define SPECTRUM		8

#define	MGAL_AT_45	980619.9203 	/* Moritz's 1980 IGF value for gravity in mGal at 45 degrees latitude */
#define	YOUNGS_MODULUS	1.0e11		/* Pascal = Nt/m**2  */
#define	NORMAL_GRAVITY	9.806199203	/* m/s**2  */
#define	POISSONS_RATIO	0.25

#define GRDFFT_EXTEND_POINT_SYMMETRY	0
#define GRDFFT_EXTEND_MIRROR_SYMMETRY	1
#define GRDFFT_EXTEND_NONE	2

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

	bool do_this[3];	/* T/F this filter wanted for r, x, and y	*/
	bool set_already;	/* true if we already filled in the structure */
	unsigned int kind;	/* FILTER_EXP, FILTER_BW, FILTER_COS  */
	unsigned int arg;	/* 0 = Gaussian, 1 = Butterworth, 2 = cosine taper,  */
};

struct FFT_SUGGESTION {
	unsigned int nx;
	unsigned int ny;
	size_t worksize;	/* # single-complex elements needed in work array  */
	size_t totalbytes;	/* (8*(nx*ny + worksize))  */
	double run_time;
	double rms_rel_err;
}; /* [0] holds fastest, [1] most accurate, [2] least storage  */

struct K_XY {	/* Holds parameters needed to calculate kx, ky, kr */
	int nx2, ny2;
	double delta_kx, delta_ky;
};

void *New_grdfft_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDFFT_CTRL *C = NULL;

	C = GMT_memory (GMT, NULL, 1, struct GRDFFT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->S.scale = 1.0;
	C->N.width = 100.0;				/* Taper over entire margin strip by default */
	C->N.mode = GRDFFT_EXTEND_POINT_SYMMETRY;	/* Default action is edge-point symmetry */
	C->Q.prefix = strdup ("tapered");		/* Default prefix */
	return (C);
}

void Free_grdfft_Ctrl (struct GMT_CTRL *GMT, struct GRDFFT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->operation) GMT_free (GMT, C->operation);	
	if (C->par) GMT_free (GMT, C->par);	
	if (C->In.file[0]) free (C->In.file[0]);	
	if (C->In.file[1]) free (C->In.file[1]);	
	if (C->G.file) free (C->G.file);	
	if (C->Q.prefix) free (C->Q.prefix);	
	GMT_free (GMT, C);	
}

void remove_plane (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, unsigned mode)
{
	/* mode = 0: Remove the best-fitting plane by least squares.
	   mode = 1: Remove the mean value.
	   mode = 2: Remove the middle value.

	Let plane be z = a0 + a1 * x + a2 * y.  Choose the
	center of x,y coordinate system at the center of 
	the array.  This will make the Normal equations 
	matrix G'G diagonal, so solution is trivial.  Also,
	spend some multiplications on normalizing the 
	range of x,y into [-1,1], to avoid roundoff error.  */

	unsigned int i, j, one_or_zero;
	uint64_t ij;
	double x_half_length, one_on_xhl, y_half_length, one_on_yhl;
	double sumx2, sumy2, data_var_orig = 0.0, data_var = 0.0, var_redux, x, y, z, a[3];
	float *datac = Grid->data;

	if (mode == 1) {	/* Remove mean */
		a[0] = 0.0;
		for (j = 0; j < Grid->header->ny; j++) for (i = 0; i < Grid->header->nx; i++) {
			z = Grid->data[GMT_IJPR(Grid->header,j,i)];
			a[0] += z;
			data_var_orig += z * z;
		}
		a[0] /= Grid->header->nm;
		for (j = 0; j < Grid->header->ny; j++) for (i = 0; i < Grid->header->nx; i++) {
			ij = GMT_IJPR(Grid->header,j,i);
			Grid->data[ij] -= a[0];
			z = Grid->data[ij];
			data_var += z * z;
		}
		var_redux = 100.0 * (data_var_orig - data_var) / data_var_orig;
		GMT_report (GMT, GMT_MSG_VERBOSE, "Mean value removed: %.8g Variance reduction: %.2f\n", a[0], var_redux);
		return;
	}
	if (mode == 2) {	/* Remove mid value */
		double zmin = DBL_MAX, zmax = -DBL_MAX;
		for (j = 0; j < Grid->header->ny; j++) for (i = 0; i < Grid->header->nx; i++) {
			ij = GMT_IJPR(Grid->header,j,i);
			z = Grid->data[ij];
			data_var_orig += z * z;
			if (z < zmin) zmin = z;
			if (z > zmax) zmax = z;
		}
		a[0] = 0.5 * (zmin + zmax);
		for (j = 0; j < Grid->header->ny; j++) for (i = 0; i < Grid->header->nx; i++) {
			ij = GMT_IJPR(Grid->header,j,i);
			Grid->data[ij] -= a[0];
			z = Grid->data[ij];
			data_var += z * z;
		}
		var_redux = 100.0 * (data_var_orig - data_var) / data_var_orig;
		GMT_report (GMT, GMT_MSG_VERBOSE, "Mid value removed %.8g Variance reduction: %.2f\n", a[0], var_redux);
		return;
	}
	one_or_zero = (Grid->header->registration == GMT_PIXEL_REG) ? 0 : 1;
	x_half_length = 0.5 * (Grid->header->nx - one_or_zero);
	one_on_xhl = 1.0 / x_half_length;
	y_half_length = 0.5 * (Grid->header->ny - one_or_zero);
	one_on_yhl = 1.0 / y_half_length;

	sumx2 = sumy2 = data_var = a[2] = a[1] = a[0] = 0.0;

	for (j = 0; j < Grid->header->ny; j++) {
		y = one_on_yhl * (j - y_half_length);
		for (i = 0; i < Grid->header->nx; i++) {
			x = one_on_xhl * (i - x_half_length);
			z = datac[GMT_IJPR(Grid->header,j,i)];
			data_var_orig += z * z;
			a[0] += z;
			a[1] += z*x;
			a[2] += z*y;
			sumx2 += x*x;
			sumy2 += y*y;
		}
	}
	a[0] /= (Grid->header->nx*Grid->header->ny);
	a[1] /= sumx2;
	a[2] /= sumy2;
	for (j = 0; j < Grid->header->ny; j++) {
		y = one_on_yhl * (j - y_half_length);
		for (i = 0; i < Grid->header->nx; i++) {
			ij = GMT_IJPR (Grid->header, j, i);
			x = one_on_xhl * (i - x_half_length);
			datac[ij] -= (float)(a[0] + a[1]*x + a[2]*y);
			data_var += (datac[ij] * datac[ij]);
		}
	}
	var_redux = 100.0 * (data_var_orig - data_var) / data_var_orig;
	data_var = sqrt(data_var / (Grid->header->nx*Grid->header->ny - 1));
	/* Rescale a1,a2 into user's units, in case useful later */
	a[1] *= (2.0/(Grid->header->wesn[XHI] - Grid->header->wesn[XLO]));
	a[2] *= (2.0/(Grid->header->wesn[YHI] - Grid->header->wesn[YLO]));
	GMT_report (GMT, GMT_MSG_VERBOSE, "Plane removed.  Mean, S.D., Dx, Dy: %.8g\t%.8g\t%.8g\t%.8g Variance reduction: %.2f\n", a[0], data_var, a[1], a[2], var_redux);
}

void taper_edges (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, unsigned int mode, double width, unsigned int FFT_nx, unsigned int FFT_ny)
{
	/* mode sets if and how tapering will be performed [see GRDFFT_EXTEND_* constants].
	 * width is relative width in percent of the margin that will be tapered [100]. */
	int il1, ir1, il2, ir2, jb1, jb2, jt1, jt2, im, jm, j, end_i, end_j, min_i, min_j, one;
	int i, i_data_start, j_data_start, mx, i_width, j_width, width_percent;
	unsigned int ju;
	char *method[2] = {"edge-point", "mirror"};
	float *datac = Grid->data, scale, cos_wt;
	struct GRD_HEADER *h = Grid->header;	/* For shorthand */

	if ((Grid->header->nx == FFT_nx && Grid->header->ny == FFT_ny) || mode == GRDFFT_EXTEND_NONE) {
		GMT_report (GMT, GMT_MSG_VERBOSE, "Data and FFT dimensions are equal - no data extension will take place\n");
		/* But there may still be interior tapering */
		if (mode != GRDFFT_EXTEND_NONE) {	/* Nothing to do since no outside pad */
			GMT_report (GMT, GMT_MSG_VERBOSE, "Data and FFT dimensions are equal - no tapering will be performed\n");
			return;
		}
	}
	
	/* Note that if nx2 = nx+1 and ny2 = ny + 1, then this routine
	 * will do nothing; thus a single row/column of zeros may be
	 * added to the bottom/right of the input array and it cannot
	 * be tapered.  But when (nx2 - nx)%2 == 1 or ditto for y,
	 * this is zero anyway.  */

	i_data_start = GMT->current.io.pad[XLO];	/* Some shorthands for readability */
	j_data_start = GMT->current.io.pad[YHI];
	mx = h->mx;
	one = (mode == GRDFFT_EXTEND_NONE) ? 0 : 1;	/* 0 is the boundry point which we want to taper to 0 for the interior taper */
	
	width_percent = lrint (width);
	if (width_percent == 0) {
		GMT_report (GMT, GMT_MSG_VERBOSE, "Tapering has been disabled via +t0\n");
	}
	width /= 100.0;	/* Was percent, now fraction */
	
	if (mode == GRDFFT_EXTEND_NONE) {	/* No extension, just tapering inside the data grid */
		i_width = lrint (Grid->header->nx * width);	/* Interior columns over which tapering will take place */
		j_width = lrint (Grid->header->ny * width);	/* Extended rows over which tapering will take place */
	}
	else {	/* We wish to extend data into the margin pads between FFT grid and data grid */
		i_width = lrint (i_data_start * width);	/* Extended columns over which tapering will take place */
		j_width = lrint (j_data_start * width);	/* Extended rows over which tapering will take place */
	}

	/* First reflect about xmin and xmax, either point symmetric about edge point OR mirror symmetric */

	if (mode != GRDFFT_EXTEND_NONE) {
		for (im = 1; im <= i_width; im++) {
			il1 = -im;	/* Outside xmin; left of edge 1  */
			ir1 = im;	/* Inside xmin; right of edge 1  */
			il2 = il1 + h->nx - 1;	/* Inside xmax; left of edge 2  */
			ir2 = ir1 + h->nx - 1;	/* Outside xmax; right of edge 2  */
			for (ju = 0; ju < h->ny; ju++) {
				if (mode == GRDFFT_EXTEND_POINT_SYMMETRY) {
					datac[GMT_IJPR(h,ju,il1)] = 2.0f * datac[GMT_IJPR(h,ju,0)]       - datac[GMT_IJPR(h,ju,ir1)];
					datac[GMT_IJPR(h,ju,ir2)] = 2.0f * datac[GMT_IJPR(h,ju,h->nx-1)] - datac[GMT_IJPR(h,ju,il2)];
				}
				else {	/* Mirroring */
					datac[GMT_IJPR(h,ju,il1)] = datac[GMT_IJPR(h,ju,ir1)];
					datac[GMT_IJPR(h,ju,ir2)] = datac[GMT_IJPR(h,ju,il2)];
				}
			}
		}
	}

	/* Next, reflect about ymin and ymax.
	 * At the same time, since x has been reflected,
	 * we can use these vals and taper on y edges */

	scale = M_PI / (j_width + 1);	/* Full 2*pi over y taper range */
	min_i = (mode == GRDFFT_EXTEND_NONE) ? 0 : -i_width;
	end_i = (mode == GRDFFT_EXTEND_NONE) ? (int)Grid->header->nx : mx - i_width;
	for (jm = one; jm <= j_width; jm++) {	/* Loop over width of strip to taper */
		jb1 = -jm;	/* Outside ymin; bottom side of edge 1  */
		jt1 = jm;	/* Inside ymin; top side of edge 1  */
		jb2 = jb1 + h->ny - 1;	/* Inside ymax; bottom side of edge 2  */
		jt2 = jt1 + h->ny - 1;	/* Outside ymax; bottom side of edge 2  */
		cos_wt = 0.5f * (1.0f + cosf (jm * scale));
		if (mode == GRDFFT_EXTEND_NONE) cos_wt = 1.0f - cos_wt;	/* Reverse weights for the interior */
		for (i = min_i; i < end_i; i++) {
			if (mode == GRDFFT_EXTEND_POINT_SYMMETRY) {
				datac[GMT_IJPR(h,jb1,i)] = cos_wt * (2.0f * datac[GMT_IJPR(h,0,i)]       - datac[GMT_IJPR(h,jt1,i)]);
				datac[GMT_IJPR(h,jt2,i)] = cos_wt * (2.0f * datac[GMT_IJPR(h,h->ny-1,i)] - datac[GMT_IJPR(h,jb2,i)]);
			}
			else if (mode == GRDFFT_EXTEND_MIRROR_SYMMETRY) {
				datac[GMT_IJPR(h,jb1,i)] = cos_wt * datac[GMT_IJPR(h,jt1,i)];
				datac[GMT_IJPR(h,jt2,i)] = cos_wt * datac[GMT_IJPR(h,jb2,i)];
			}
			else {	/* Interior tapering only */
				datac[GMT_IJPR(h,jt1,i)] *= cos_wt;
				datac[GMT_IJPR(h,jb2,i)] *= cos_wt;
			}
		}
	}

	/* Now, cos taper the x edges */
	scale = M_PI / (i_width + 1);	/* Full 2*pi over x taper range */
	end_j = (mode == GRDFFT_EXTEND_NONE) ? h->ny : h->my - j_data_start;
	min_j = (mode == GRDFFT_EXTEND_NONE) ? 0 : -j_width;
	for (im = one; im <= i_width; im++) {
		il1 = -im;
		ir1 = im;
		il2 = il1 + h->nx - 1;
		ir2 = ir1 + h->nx - 1;
		cos_wt = 0.5f * (1.0f + cosf (im * scale));
		if (mode == GRDFFT_EXTEND_NONE) cos_wt = 1.0f - cos_wt;	/* Switch to weights for the interior */
		for (j = min_j; j < end_j; j++) {
			if (mode == GRDFFT_EXTEND_NONE) {
				datac[GMT_IJPR(h,j,ir1)] *= cos_wt;
				datac[GMT_IJPR(h,j,il2)] *= cos_wt;
			}
			else {
				datac[GMT_IJPR(h,j,il1)] *= cos_wt;
				datac[GMT_IJPR(h,j,ir2)] *= cos_wt;
			}
		}
	}

	if (mode == GRDFFT_EXTEND_NONE)
		GMT_report (GMT, GMT_MSG_VERBOSE, "Data margin tapered to zero over %d %% of data width and height\n", width_percent);
	else
		GMT_report (GMT, GMT_MSG_VERBOSE, "Data extended via %s symmetry at all edges, then tapered to zero over %d %% of extended area\n", method[mode], width_percent);
	
}

void save_intermediate (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, char *prefix)
{
	/* Write the intermediate grid thatwill be passed to the FFT to file.
	 * This grid may have been a mean, mid-value, or plane removed, may
	 * have data filled into an extended margin, and may have been taperer.
	 * File name is determined by prefix, i.e., prefix_file. */
	int del;
	unsigned int pad[4];
	char file[256];
	struct GRD_HEADER save;
	GMT_memcpy (&save, Grid->header, 1, sizeof (struct GRD_HEADER));	/* Save what we have before messing around */
	GMT_memcpy (pad, Grid->header->pad, 4, unsigned int);			/* Save current pad, then set pad to zero */
	if ((del = Grid->header->pad[XLO]) > 0) Grid->header->wesn[XLO] -= del * Grid->header->inc[GMT_X], Grid->header->pad[XLO] = 0;
	if ((del = Grid->header->pad[XHI]) > 0) Grid->header->wesn[XHI] += del * Grid->header->inc[GMT_X], Grid->header->pad[XHI] = 0;
	if ((del = Grid->header->pad[YLO]) > 0) Grid->header->wesn[YLO] -= del * Grid->header->inc[GMT_Y], Grid->header->pad[YLO] = 0;
	if ((del = Grid->header->pad[YHI]) > 0) Grid->header->wesn[YHI] += del * Grid->header->inc[GMT_Y], Grid->header->pad[YHI] = 0;
	GMT_memcpy (GMT->current.io.pad, Grid->header->pad, 4, unsigned int);	/* set tmp pad */
	GMT_set_grddim (GMT, Grid->header);	/* Recompute all dimensions */
	sprintf (file,"%s_%s", prefix, Grid->header->name);
	if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA | GMT_GRID_COMPLEX_REAL, NULL, file, Grid) != GMT_OK)
		GMT_report (GMT, GMT_MSG_NORMAL, "Intermediate detrended, extended, and tapered grid could not be written to %s\n", file);
	else
		GMT_report (GMT, GMT_MSG_VERBOSE, "Intermediate detrended, extended, and tapered grid written to %s\n", file);
	
	GMT_memcpy (Grid->header, &save, 1, sizeof (struct GRD_HEADER));	/* Restore original */
	GMT_memcpy (GMT->current.io.pad, pad, 4, unsigned int);			/* Restore GMT pad */
}

double kx (uint64_t k, struct K_XY *K)
{
	/* Return the value of kx given k,
	 * where kx = 2 pi / lambda x,
	 * and k refers to the position
	 * in the datac array, datac[k].  */

	int64_t ii = (k/2)%(K->nx2);
	if (ii > (K->nx2)/2) ii -= (K->nx2);
	return (ii * K->delta_kx);
}

double ky (uint64_t k, struct K_XY *K)
{
	/* Return the value of ky given k,
	 * where ky = 2 pi / lambda y,
	 * and k refers to the position
	 *in the datac array, datac[k].  */

	int64_t jj = (k/2)/(K->nx2);
	if (jj > (K->ny2)/2) jj -= (K->ny2);
	return (jj * K->delta_ky);
}

double modk (uint64_t k, struct K_XY *K)
{
	/* Return the value of sqrt(kx*kx + ky*ky),
	 * given k, where k is array position.  */

	return (hypot (kx (k, K), ky (k, K)));
}

unsigned int do_differentiate (struct GMT_GRID *Grid, double *par, struct K_XY *K)
{
	uint64_t k;
	double scale, fact;
	float *datac = Grid->data;

	/* Differentiate in frequency domain by multiplying by kr [scale optional] */

	scale = (*par != 0.0) ? *par : 1.0;
	datac[0] = datac[1] = 0.0;	/* Derivative of the mean is zero */
	for (k = 2; k < Grid->header->size; k += 2) {
		fact = scale * modk (k, K);
		datac[k]   *= (float)fact;
		datac[k+1] *= (float)fact;
	}
	return (1);	/* Number of parameters used */
}

unsigned int do_integrate (struct GMT_GRID *Grid, double *par, struct K_XY *K)
{
	/* Integrate in frequency domain by dividing by kr [scale optional] */
	uint64_t k;
	double fact, scale;
	float *datac = Grid->data;

	scale = (*par != 0.0) ? *par : 1.0;
	datac[0] = datac[1] = 0.0;
	for (k = 2; k < Grid->header->size; k += 2) {
		fact = 1.0 / (scale * modk (k, K));
		datac[k]   *= (float)fact;
		datac[k+1] *= (float)fact;
	}
	return (1);	/* Number of parameters used */
}

unsigned int do_continuation (struct GMT_GRID *Grid, double *zlevel, struct K_XY *K)
{
	uint64_t k;
	float tmp, *datac = Grid->data;

	/* If z is positive, the field will be upward continued using exp[- k z].  */

	for (k = 2; k < Grid->header->size; k += 2) {
		tmp = (float)exp (-(*zlevel) * modk (k, K));
		datac[k]   *= tmp;
		datac[k+1] *= tmp;
	}
	return (1);	/* Number of parameters used */
}

unsigned int do_azimuthal_derivative (struct GMT_GRID *Grid, double *azim, struct K_XY *K)
{
	uint64_t k;
	float tempr, tempi, fact, *datac = Grid->data;
	double cos_azim, sin_azim;

	sincosd (*azim, &sin_azim, &cos_azim);

	datac[0] = datac[1] = 0.0;
	for (k = 2; k < Grid->header->size; k+= 2) {
		fact = (float)(sin_azim * kx (k, K) + cos_azim * ky (k, K));
		tempr = -(datac[k+1] * fact);
		tempi = (datac[k] * fact);
		datac[k]   = tempr;
		datac[k+1] = tempi;
	}
	return (1);	/* Number of parameters used */
}

unsigned int do_isostasy (struct GMT_GRID *Grid, struct GRDFFT_CTRL *Ctrl, double *par, struct K_XY *K)
{

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

	float *datac = Grid->data;

	te = par[0];	rl = par[1];	rm = par[2];	rw = par[3];	ri = par[4];
	airy_ratio = -(rl - rw)/(rm - ri);

	if (te == 0.0) {	/* Airy isostasy; scale variable Ctrl->S.scale and return */
		Ctrl->S.scale *= airy_ratio;
		return (5);	/* Number of parameters used */
	}

	rigidity_d = (YOUNGS_MODULUS * pow (te, 3.0)) / (12.0 * (1.0 - POISSONS_RATIO * POISSONS_RATIO));
	d_over_restoring_force = rigidity_d / ((rm - ri) * NORMAL_GRAVITY);

	for (k = 0; k < Grid->header->size; k += 2) {
		mk = modk (k, K);
		k2 = mk * mk;
		k4 = k2 * k2;
		transfer_fn = airy_ratio / ((d_over_restoring_force * k4) + 1.0);
		datac[k]   *= (float)transfer_fn;
		datac[k+1] *= (float)transfer_fn;
	}
	return (5);	/* Number of parameters used */
}

double gauss_weight (struct F_INFO *f_info, double freq, int j) {
	double hi, lo;
	lo = (f_info->llambda[j] == -1.0) ? 0.0 : exp (-M_LN2 * pow (freq * f_info->llambda[j], 2.0));	/* Low-pass part */
	hi = (f_info->hlambda[j] == -1.0) ? 1.0 : exp (-M_LN2 * pow (freq * f_info->hlambda[j], 2.0));	/* Hi-pass given by its complementary low-pass */
	return (hi - lo);
}

double bw_weight (struct F_INFO *f_info, double freq, int j) {	/* Butterworth filter */
	double hi, lo;
	lo = (f_info->llambda[j] == -1.0) ? 0.0 : sqrt (1.0 / (1.0 + pow (freq * f_info->llambda[j], f_info->bw_order)));	/* Low-pass part */
	hi = (f_info->hlambda[j] == -1.0) ? 1.0 : sqrt (1.0 / (1.0 + pow (freq * f_info->hlambda[j], f_info->bw_order)));	/* Hi-pass given by its complementary low-pass */
	return (hi - lo);
}

double cosine_weight_grdfft (struct F_INFO *f_info, double freq, int j) {
	if (freq <= f_info->lc[j] || freq >= f_info->hc[j]) return(0.0);	/* In fully cut range.  Weight is zero.  */
	if (freq > f_info->lc[j] && freq < f_info->lp[j]) return (0.5 * (1.0 + cos (M_PI * (freq - f_info->lp[j]) * f_info->ltaper[j])));
	if (freq > f_info->hp[j] && freq < f_info->hc[j]) return (0.5 * (1.0 + cos (M_PI * (freq - f_info->hp[j]) * f_info->htaper[j])));
	return (1.0);	/* Freq is in the fully passed range, so weight is multiplied by 1.0  */
}

double get_filter_weight (uint64_t k, struct F_INFO *f_info, struct K_XY *K)
{
	unsigned int j;
	double freq, return_value = 1.0;

	for (j = 0; j < 3; j++) {
		if (!(f_info->do_this[j])) continue;	/* Only do one of x, y, or r filtering */
		switch (j) {
			case 0:
				freq = modk (k, K);
				break;
			case 1:
				freq = kx (k, K);
				break;
			default:
				freq = ky (k, K);
				break;
		}
		return_value = f_info->filter (f_info, freq, j);
	}

	return (return_value);
}

void do_filter (struct GMT_GRID *Grid, struct F_INFO *f_info, struct K_XY *K)
{
	uint64_t k;
	float weight, *datac = Grid->data;

	for (k = 0; k < Grid->header->size; k += 2) {
		weight = (float) get_filter_weight (k, f_info, K);
		datac[k]   *= weight;
		datac[k+1] *= weight;
	}
}

unsigned int count_slashes (char *txt)
{
	unsigned int i, n;
	for (i = n = 0; txt[i]; i++) if (txt[i] == '/') n++;
	return (n);
}

bool parse_f_string (struct GMT_CTRL *GMT, struct F_INFO *f_info, char *c)
{
	unsigned int i, j, n_tokens, pos;
	bool descending;
	double fourvals[4];
	char line[GMT_TEXT_LEN256], p[GMT_TEXT_LEN256];

	/* Syntax is either -F[x|y]lc/hc/lp/hp (Cosine taper), -F[x|y]lo/hi (Gaussian), or 0F[x|y]lo/hi/order (Butterworth) */

	strncpy (line, c,  GMT_TEXT_LEN256);
	i = j = 0;	/* j is Filter type: r=0, x=1, y=2  */

	if (line[i] == 'x') {
		j = 1;
		i++;
	}
	else if (line[i] == 'y') {
		j = 2;
		i++;
	}

	f_info->do_this[j] = true;
	fourvals[0] = fourvals[1] = fourvals[2] = fourvals[3] = -1.0;

	n_tokens = pos = 0;
	while ((GMT_strtok (&line[i], "/", &pos, p))) {
		if (n_tokens > 3) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Too many slashes in -F.\n");
			return (true);
		}
		if(p[0] == '-')
			fourvals[n_tokens] = -1.0;
		else {
			if ((sscanf(p, "%lf", &fourvals[n_tokens])) != 1) {
				GMT_report (GMT, GMT_MSG_NORMAL, " Cannot read token %d.\n", n_tokens);
				return (true);
			}
		}
		n_tokens++;
	}

	if (!(n_tokens == 2 || n_tokens == 3 || n_tokens == 4)) {
		GMT_report (GMT, GMT_MSG_NORMAL, "-F Cannot find 2-4 tokens separated by slashes.\n");
		return (true);
	}
	descending = true;
	if (f_info->kind == FILTER_BW && n_tokens == 3) n_tokens = 2;	/* So we dont check the order as a wavelength */

	for (i = 1; i < n_tokens; i++) {
		if (fourvals[i] == -1.0 || fourvals[i-1] == -1.0) continue;
		if (fourvals[i] > fourvals[i-1]) descending = false;
	}
	if (!(descending)) {
		GMT_report (GMT, GMT_MSG_NORMAL, "-F Wavelengths are not in descending order.\n");
		return (true);
	}
	if (f_info->kind == FILTER_COS) {	/* Cosine band-pass specification */
		if ((fourvals[0] * fourvals[1]) < 0.0 || (fourvals[2] * fourvals[3]) < 0.0) {
			GMT_report (GMT, GMT_MSG_NORMAL, "-F Pass/Cut specification error.\n");
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
	else if (f_info->kind == FILTER_BW) {	/* Butterworth specification */
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
	f_info->arg = f_info->kind - FILTER_EXP;
	return (false);
}

int do_spectrum (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, double *par, bool give_wavelength, bool km, char *file, struct K_XY *K)
{
	/* This is modeled on the 1-D case, using the following ideas:
	 *	In 1-D, we ensemble average over samples of length L = 
	 *	n * dt.  This gives n/2 power spectral estimates, at
	 *	frequencies i/L, where i = 1, n/2.  If we have a total
	 *	data set of ndata, we can make nest=ndata/n independent
	 *	estimates of the power.  Our standard error is then
	 *	1/sqrt(nest).
	 *	In making 1-D estimates from 2-D data, we may choose
	 *	n and L from nx2 or ny2 and delta_kx, delta_ky as 
	 *	appropriate.  In this routine, we are giving the sum over
	 * 	all frequencies in the other dimension; that is, an
	 *	approximation of the integral.
	 */

	char header[GMT_BUFSIZ], *name[2] = {"freq", "wlength"};
	uint64_t dim[4] = {1, 1, 3, 0};	/* One table and one segment, with 1 + 2 = 3 columns and yet unknown rows */
	uint64_t k, nk, nused, ifreq;	/* *nused = NULL; */
	double delta_k, r_delta_k, freq, *power = NULL, eps_pow, powfactor;
	double (*get_k) (uint64_t, struct K_XY *);

	float *datac = Grid->data;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;

	if (*par > 0.0) {
		/* X spectrum desired  */
		delta_k = K->delta_kx;	nk = K->nx2 / 2;	get_k = &kx;
	}
	else if (*par < 0.0) {
		/* Y spectrum desired  */
		delta_k = K->delta_ky;	nk = K->ny2 / 2;	get_k = &ky;
	}
	else {
		/* R spectrum desired  */
		if (K->delta_kx < K->delta_ky) {
			delta_k = K->delta_kx;	nk = K->nx2 / 2;
		}
		else {
			delta_k = K->delta_ky;	nk = K->ny2 / 2;
		}
		get_k = &modk;
	}

	/* Get an array for summing stuff */
	power = GMT_memory (GMT, NULL, nk, double);
	//nused = GMT_memory (GMT, NULL, nk, uint64_t);

	/* Loop over it all, summing and storing, checking range for r */

	r_delta_k = 1.0 / delta_k;

	for (nused = 0, k = 2; k < Grid->header->size; k += 2) {
		freq = (*get_k)(k, K);
		ifreq = lrint (fabs (freq) * r_delta_k);	/* Smallest value returned might be 0 when doing r spectrum*/
		if (ifreq > 0) --ifreq;
		if (ifreq >= nk) continue;	/* Might happen when doing r spectrum  */
		power[ifreq] += hypot (datac[k], datac[k+1]);
		nused++;
	}

	/* Now get here when array is summed.  */
	eps_pow = 1.0 / sqrt ((double)nused/(double)nk);
	delta_k /= (2.0 * M_PI);	/* Write out frequency, not wavenumber  */
	powfactor = 4.0 / pow ((double)Grid->header->size, 2.0);
	dim[3] = nk;
	if ((D = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, dim)) == NULL) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Unable to create a data set for spectrum\n");
		return (GMT->parent->error);
	}
	S = D->table[0]->segment[0];	/* Only one table with one segment here, with 3 cols and nk rows */
	if (give_wavelength && km) delta_k *= 1000.0;
	for (k = 0; k < nk; k++) {
		freq = (k + 1) * delta_k;
		if (give_wavelength) freq = 1.0 / freq;
		power[k] *= powfactor;
		S->coord[GMT_X][k] = freq;
		S->coord[GMT_Y][k] = power[k];
		S->coord[GMT_Z][k] = eps_pow * power[k];
	}
	sprintf (header, "#%s[0]\tpow[1]\tstd_pow[2]", name[give_wavelength]);
	D->table[0]->header = GMT_memory (GMT, NULL, 1, char *);
	D->table[0]->header[0] = strdup (header);
	D->table[0]->n_headers = 1;
	if (GMT_Write_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_STREAM, GMT_IS_POINT, GMT_WRITE_SET, NULL, file, D) != GMT_OK) {
		return (GMT->parent->error);
	}
	if (GMT_Destroy_Data (GMT->parent, GMT_ALLOCATED, &D) != GMT_OK) {
		return (GMT->parent->error);
	}
	GMT_free (GMT, power);
	return (1);	/* Number of parameters used */
}

int do_cross_spectrum (struct GMT_CTRL *GMT, struct GMT_GRID *GridX, struct GMT_GRID *GridY, double *par, bool give_wavelength, bool km, char *file, struct K_XY *K)
{
	/* Compute cross-spectral estimates from the two grids Xand Y and return frequency f and 8 quantities:
	 * Xpower[f], Ypower[f], coherent power[f], noise power[f], phase[f], admittance[f], gain[f], coherency[f].
	 * Each quantity comes with its own 1-std dev error estimate, hence output is 17 columns.
	 * Equations based on spectrum1d.c */

	uint64_t dim[4] = {1, 1, 17, 0};	/* One table and one segment, with 1 + 2*8 = 17 columns and yet unknown rows */
	uint64_t k, nk, ifreq, *nused = NULL;
	char header[GMT_BUFSIZ], *name[2] = {"freq", "wlength"};
	unsigned int col;
	float *X = GridX->data, *Y = GridY->data;	/* Short-hands */
	double delta_k, r_delta_k, freq, coh_k, sq_norm, tmp, eps_pow;
	double *X_pow = NULL, *Y_pow = NULL, *co_spec = NULL, *quad_spec = NULL;
	double (*get_k) (uint64_t, struct K_XY *);
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;

	if (*par > 0.0) {
		/* X spectrum desired  */
		delta_k = K->delta_kx;	nk = K->nx2 / 2;	get_k = &kx;
	}
	else if (*par < 0.0) {
		/* Y spectrum desired  */
		delta_k = K->delta_ky;	nk = K->ny2 / 2;	get_k = &ky;
	}
	else {
		/* R spectrum desired  */
		if (K->delta_kx < K->delta_ky) {
			delta_k = K->delta_kx;	nk = K->nx2 / 2;
		}
		else {
			delta_k = K->delta_ky;	nk = K->ny2 / 2;
		}
		get_k = &modk;
	}

	/* Get arrays for summing stuff */
	X_pow     = GMT_memory (GMT, NULL, nk, double );
	Y_pow     = GMT_memory (GMT, NULL, nk, double);
	co_spec   = GMT_memory (GMT, NULL, nk, double);
	quad_spec = GMT_memory (GMT, NULL, nk, double);
	nused     = GMT_memory (GMT, NULL, nk, uint64_t);

	/* Loop over it all, summing and storing, checking range for r */

	r_delta_k = 1.0 / delta_k;

	for (k = 2; k < GridX->header->size; k += 2) {
		freq = (*get_k)(k, K);
		ifreq = lrint (fabs (freq) * r_delta_k);	/* Smallest value returned might be 0 when doing r spectrum*/
		if (ifreq > 0) --ifreq;
		if (ifreq >= nk) continue;	/* Might happen when doing r spectrum  */
		X_pow[ifreq]     += (X[k]   * X[k] + X[k+1] * X[k+1]);	/* X x X* = Power of grid X */
		Y_pow[ifreq]     += (Y[k]   * Y[k] + Y[k+1] * Y[k+1]);	/* Y x Y* = Power of grid Y */
		co_spec[ifreq]   += (Y[k]   * X[k] + Y[k+1] * X[k+1]);	/* Real part of Y x X* */
		quad_spec[ifreq] += (X[k+1] * Y[k] - Y[k+1] * X[k]);	/* Imag part of Y x X* */
		nused[ifreq]++;
	}

	/* Now get here when array is summed.  */
	GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_FLOAT;	/* To counter-act any -fg setting */
	
	delta_k /= (2.0 * M_PI);	/* Write out frequency, not wavenumber  */
	dim[3] = nk;
	if ((D = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, dim)) == NULL) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Unable to create a data set for cross-spectral estimates\n");
		return (GMT->parent->error);
	}
	S = D->table[0]->segment[0];	/* Only one table with one segment here, with 17 cols and nk rows */
	if (give_wavelength && km) delta_k *= 1000.0;	/* Wanted distances measured in km */
	for (k = 0; k < nk; k++) {
		eps_pow = 1.0 / sqrt (nused[k]);	/* Multiplicative error bars for power spectra  */
		freq = (k + 1) * delta_k;
		if (give_wavelength) freq = 1.0 / freq;
		/* Compute coherence first since it is needed by many of the other estimates */
		coh_k = (co_spec[k] * co_spec[k] + quad_spec[k] * quad_spec[k]) / (X_pow[k] * Y_pow[k]);
		sq_norm = sqrt ((1.0 - coh_k) / (2.0 * coh_k));	/* Save repetitive expression further down */
		col = 0;
		/* Col 0 is the frequency (or wavelength) */
		S->coord[col++][k] = freq;
		/* Cols 1-2 are xpower and std.err estimate */
		S->coord[col++][k] = X_pow[k];
		S->coord[col++][k] = X_pow[k] * eps_pow;
		/* Cols 3-4 are ypower and std.err estimate */
		S->coord[col++][k] = Y_pow[k];
		S->coord[col++][k] = Y_pow[k] * eps_pow;
		/* Cols 5-6 are coherent power and std.err estimate */
		S->coord[col++][k] = tmp = Y_pow[k] * coh_k;
		S->coord[col++][k] = tmp * eps_pow * sqrt ((2.0 - coh_k) / coh_k);
		/* Cols 7-8 are noise power and std.err estimate */
		S->coord[col++][k] = tmp = Y_pow[k] * (1.0 - coh_k);
		S->coord[col++][k] = tmp * eps_pow;
		/* Cols 9-10 are phase and std.err estimate */
		S->coord[col++][k] = tmp = d_atan2 (quad_spec[k], co_spec[k]);
		S->coord[col++][k] = tmp * eps_pow * sq_norm;
		/* Cols 11-12 are admittance and std.err estimate */
		S->coord[col++][k] = tmp = co_spec[k] / X_pow[k];
		S->coord[col++][k] = tmp * eps_pow * fabs (sq_norm);
		/* Cols 13-14 are gain and std.err estimate */
		S->coord[col++][k] = tmp = sqrt (quad_spec[k]) / X_pow[k];
		S->coord[col++][k] = tmp * eps_pow * sq_norm;
		/* Cols 15-16 are coherency and std.err estimate */
		S->coord[col++][k] = coh_k;
		S->coord[col++][k] = coh_k * eps_pow * (1.0 - coh_k) * sqrt (2.0 / coh_k);
	}
	sprintf (header, "#%s[0]\txpow[1]\tstd_xpow[2]\typow[3]\tstd_ypow[4]\tcpow[5]\tstd_cpow[6]\tnpow[7]\tstd_npow[8]\t" \
		"phase[9]\tstd_phase[10]\tadm[11]\tstd_ad[12]\tgain[13]\tstd_gain[14]\tcoh[15]\tstd_coh[16]", name[give_wavelength]);
	D->table[0]->header = GMT_memory (GMT, NULL, 1, char *);
	D->table[0]->header[0] = strdup (header);
	D->table[0]->n_headers = 1;
	if (GMT_Write_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_STREAM, GMT_IS_POINT, GMT_WRITE_SET, NULL, file, D) != GMT_OK) {
		return (GMT->parent->error);
	}
	if (GMT_Destroy_Data (GMT->parent, GMT_ALLOCATED, &D) != GMT_OK) {
		return (GMT->parent->error);
	}
	GMT_free (GMT, X_pow);
	GMT_free (GMT, Y_pow);
	GMT_free (GMT, co_spec);
	GMT_free (GMT, quad_spec);
	GMT_free (GMT, nused);
	return (1);	/* Number of parameters used */
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

void fourt_stats (struct GMT_CTRL *C, unsigned int nx, unsigned int ny, unsigned int *f, double *r, size_t *s, double *t)
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
	n_factors = GMT_get_prime_factors (C, nx, f);
	nonsymx = get_non_symmetric_f (f, n_factors);
	n_factors = GMT_get_prime_factors (C, ny, f);
	nonsymy = get_non_symmetric_f (f, n_factors);
	nonsym = MAX (nonsymx, nonsymy);

	/* Now get factors of ntotal  */
	ntotal = GMT_get_nm (C, nx, ny);
	n_factors = GMT_get_prime_factors (C, ntotal, f);
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

void suggest_fft (struct GMT_CTRL *GMT, unsigned int nx, unsigned int ny, struct FFT_SUGGESTION *fft_sug, bool do_print)
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

	fourt_stats (GMT, nx, ny, f, &given_err, &given_space, &given_time);
	given_space += nx * ny;
	given_space *= 8;
	if (do_print)
		GMT_report (GMT, GMT_MSG_NORMAL, " Data dimension\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n", nx, ny, given_time, given_err, given_space);

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

			fourt_stats (GMT, nxg, nyg, f, &current_err, &current_space, &current_time);
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
		GMT_report (GMT, GMT_MSG_NORMAL, " Highest speed\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n",
			nx_best_t, ny_best_t, best_time, t_err, t_space);
		GMT_report (GMT, GMT_MSG_NORMAL, " Most accurate\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n",
			nx_best_e, ny_best_e, e_time, best_err, e_space);
		GMT_report (GMT, GMT_MSG_NORMAL, " Least storage\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %" PRIuS "\n",
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

void set_grid_radix_size (struct GMT_CTRL *GMT, struct GRDFFT_CTRL *Ctrl, struct GMT_GRID *Gin)
{
	unsigned int k, factors[32];
	size_t worksize;
	double tdummy, edummy;
	struct FFT_SUGGESTION fft_sug[3];

	/* Get dimensions as may be appropriate */
	if (Ctrl->N.n_user_set) {
		if (Ctrl->N.nx2 < Gin->header->nx || Ctrl->N.ny2 < Gin->header->ny) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Error: You specified -Nnx/ny smaller than input grid.  Ignored.\n");
			Ctrl->N.n_user_set = false;
		}
	}
	if (!(Ctrl->N.n_user_set) ) {
		if (Ctrl->N.force_narray) {
			Ctrl->N.nx2 = Gin->header->nx;
			Ctrl->N.ny2 = Gin->header->ny;
			for (k = 0; k < 4; k++) Gin->header->BC[k] = GMT_BC_IS_DATA;	/* This bypasses BC pad checking later since there is no pad */
		}
		else {
			suggest_fft (GMT, Gin->header->nx, Gin->header->ny, fft_sug, (GMT_is_verbose (GMT, GMT_MSG_VERBOSE) || Ctrl->N.suggest_narray));
			if (fft_sug[1].totalbytes < fft_sug[0].totalbytes) {
				/* The most accurate solution needs same or less storage
				 * as the fastest solution; use the most accurate's dimensions */
				Ctrl->N.nx2 = fft_sug[1].nx;
				Ctrl->N.ny2 = fft_sug[1].ny;
			}
			else {
				/* Use the sizes of the fastest solution  */
				Ctrl->N.nx2 = fft_sug[0].nx;
				Ctrl->N.ny2 = fft_sug[0].ny;
			}
		}
	}

	/* Get here when nx2 and ny2 are set to the vals we will use.  */

	fourt_stats (GMT, Ctrl->N.nx2, Ctrl->N.ny2, factors, &edummy, &worksize, &tdummy);
	GMT_report (GMT, GMT_MSG_VERBOSE, " Data dimension %d %d\tFFT dimension %d %d\n",
		Gin->header->nx, Gin->header->ny, Ctrl->N.nx2, Ctrl->N.ny2);

	if (worksize) {
		if (worksize < Ctrl->N.nx2) worksize = Ctrl->N.nx2;
		if (worksize < Ctrl->N.ny2) worksize = Ctrl->N.ny2;
		worksize *= 2;
	}
	else
		worksize = 4;


	/* Put the data in the middle of the padded array */

	GMT->current.io.pad[XLO] = (Ctrl->N.nx2 - Gin->header->nx)/2;	/* zero if nx2 < Gin->header->nx+1  */
	GMT->current.io.pad[YHI] = (Ctrl->N.ny2 - Gin->header->ny)/2;
	GMT->current.io.pad[XHI] = Ctrl->N.nx2 - Gin->header->nx - GMT->current.io.pad[XLO];
	GMT->current.io.pad[YLO] = Ctrl->N.ny2 - Gin->header->ny - GMT->current.io.pad[YHI];
}

void save_complex_grid (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int mode, struct K_XY *K, char *file)
{
	/* Save the raw spectrum as two files (real,imag) or (mag,phase), depending on mode.
	 * We must first do an "fftshift" operation as in Matlab, to put the 0 frequency
	 * value in the center of the grid. */
	uint64_t row, col, i_ij, o_ij;
	unsigned int nx_2, ny_2, k, pad[4], wmode[2] = {GMT_GRID_COMPLEX_REAL, GMT_GRID_COMPLEX_IMAG};
	double wesn[4], inc[2];
	char outfile[GMT_BUFSIZ], *prefix[2][2] = {{"real", "imag"}, {"mag", "phase"}};
	struct GMT_GRID *Grid = NULL;

	if ((Grid = GMT_Create_Data (GMT->parent, GMT_IS_GRID, NULL)) == NULL) return;

	GMT_report (GMT, GMT_MSG_VERBOSE, "Write components of complex raw spectrum to files %s_%s and %s_%s\n", prefix[mode][0], file, prefix[mode][1], file);

	/* Prepare wavenumber domain limits and increments */
	nx_2 = K->nx2 / 2;	ny_2 = K->ny2 / 2;
	wesn[XLO] = -K->delta_kx * nx_2;	wesn[XHI] =  K->delta_kx * (nx_2 - 1);
	wesn[YLO] = -K->delta_ky * (ny_2 - 1);	wesn[YHI] =  K->delta_ky * ny_2;
	inc[GMT_X] = K->delta_kx;		inc[GMT_Y] = K->delta_ky;
	GMT_memcpy (pad, GMT->current.io.pad, 4, unsigned int);		/* Save current GMT pad */
	for (k = 0; k < 4; k++) GMT->current.io.pad[k] = 0;		/* No pad is what we need for this application */

	/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
	if (GMT_Init_Data (GMT->parent, GMT_IS_GRID, NULL, wesn, inc, G->header->registration | GMT_GRID_COMPLEX_REAL, Grid)) return;
	strcpy (Grid->header->x_units, "m^(-1)");	strcpy (Grid->header->y_units, "m^(-1)");
	strcpy (Grid->header->z_units, G->header->z_units);
	strcpy (Grid->header->remark, "Applied fftshift: kx = 0 at (nx/2 + 1) and ky = 0 at ny/2");

	if (GMT_Alloc_Data (GMT->parent, GMT_IS_GRID, GMTAPI_NOTSET, Grid)) return;
	for (row = 0; row < ny_2; row++) {	/* Copy over values from 1/3, and 2/4 quadrant */
		for (col = 0; col < nx_2; col++) {
			i_ij = 2*GMT_IJ0 (Grid->header, row, col);
			o_ij = 2*GMT_IJ0 (Grid->header, row+ny_2, col+nx_2);
			if (mode) {	/* Want magnitude and phase */
				Grid->data[i_ij]   = hypot (G->data[o_ij], G->data[o_ij+1]);
				Grid->data[i_ij+1] = d_atan2 (G->data[o_ij+1], G->data[o_ij]);
				Grid->data[o_ij]   = hypot (G->data[i_ij], G->data[i_ij+1]);
				Grid->data[o_ij+1] = d_atan2 (G->data[i_ij+1], G->data[i_ij]);
			}
			else {
				Grid->data[i_ij] = G->data[o_ij];	Grid->data[i_ij+1] = G->data[o_ij+1];
				Grid->data[o_ij] = G->data[i_ij];	Grid->data[o_ij+1] = G->data[i_ij+1];
			}
			i_ij = 2*GMT_IJ0 (Grid->header, row+ny_2, col);
			o_ij = 2*GMT_IJ0 (Grid->header, row, col+nx_2);
			if (mode) {	/* Want magnitude and phase */
				Grid->data[i_ij]   = hypot (G->data[o_ij], G->data[o_ij+1]);
				Grid->data[i_ij+1] = d_atan2 (G->data[o_ij+1], G->data[o_ij]);
				Grid->data[o_ij]   = hypot (G->data[i_ij], G->data[i_ij+1]);
				Grid->data[o_ij+1] = d_atan2 (G->data[i_ij+1], G->data[i_ij]);
			}
			else {
				Grid->data[i_ij] = G->data[o_ij];	Grid->data[i_ij+1] = G->data[o_ij+1];
				Grid->data[o_ij] = G->data[i_ij];	Grid->data[o_ij+1] = G->data[i_ij+1];
			}
		}
	}
	for (k = 0; k < 2; k++) {	/* Write the two grids */
		sprintf (outfile, "%s_%s", prefix[mode][k], file);
		sprintf (Grid->header->title, "The %s part of FFT transformed input grid %s", prefix[mode][k], file);
		if (k == 1 && mode) strcpy (Grid->header->z_units, "radians");
		if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA | wmode[k], NULL, outfile, Grid) != GMT_OK) {
			GMT_report (GMT, GMT_MSG_NORMAL, "%s could not be written\n", outfile);
			return;
		}
	}
	if (GMT_Destroy_Data (GMT->parent, GMT_ALLOCATED, &Grid) != GMT_OK) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Error freeing temporary grid\n");
	}
		
	GMT_memcpy (GMT->current.io.pad, pad, 4, unsigned int);	/* Restore GMT pad */
}

int GMT_grdfft_usage (struct GMTAPI_CTRL *C, int level)
{
	struct GMT_CTRL *GMT = C->GMT;

	gmt_module_show_name_and_purpose (THIS_MODULE);
	GMT_message (GMT, "usage: grdfft <ingrid> [<ingrid2>]  [-G<outgrid>|<table>] [-A<azimuth>] [-C<zlevel>]\n");
	GMT_message (GMT, "\t[-D[<scale>|g]] [-E[r|x|y][w[k]] [-F[x|y]<parameters>] [-I[<scale>|g]] [-L[m|h]]\n");
	GMT_message (GMT, "\t[-N[f|q|<nx>/<ny>][+e|m|n][+t<width>]] [-Q[<prefix>]] [-S<scale>]\n");
	GMT_message (GMT, "\t[-T<te>/<rl>/<rm>/<rw>/<ri>] [-Z[p]] [%s] [%s] [-ho]\n\n", GMT_V_OPT, GMT_f_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<ingrid> is the input grid file.\n");
	GMT_message (GMT, "\tOPTIONS:\n");
	GMT_message (GMT, "\t-G filename for output netCDF grid file OR 1-D spectrum (see -E).\n");
	GMT_message (GMT, "\t-A Take azimuthal derivative along line <azimuth> degrees CW from North.\n");
	GMT_message (GMT, "\t-C Continue field upward (+) or downward (-) to <zlevel> (meters).\n");
	GMT_message (GMT, "\t-D Differentiate, i.e., multiply by kr [ * scale].  Use -Dg to get mGal from m].\n");
	GMT_message (GMT, "\t-E Estimate spEctrum in the radial [Default], x, or y-direction.\n");
	GMT_message (GMT, "\t   Given one grid X, write f, Xpower[f] to output file (see -G) or stdout.\n");
	GMT_message (GMT, "\t   Given two grids X and Y, write f, Xpower[f], Ypower[f], coherent power[f],\n");
	GMT_message (GMT, "\t   noise power[f], phase[f], admittance[f], gain[f], coherency[f].\n");
	GMT_message (GMT, "\t   Each quantity is followed by a column of 1 std dev. error estimates.\n");
	GMT_message (GMT, "\t   Append w to write wavelength instead of frequency; append k to report\n");
	GMT_message (GMT, "\t   wavelength in km (geographic grids only) [m].\n");
	GMT_message (GMT, "\t-F Filter r [x] [y] freq according to one of three kinds of filter specifications:\n");
	GMT_message (GMT, "\t   a) Cosine band-pass: Append four wavelengths <lc>/<lp>/<hp>/<hc>.\n");
	GMT_message (GMT, "\t      freq outside <lc>/<hc> are cut; inside <lp>/<hp> are passed, rest are tapered.\n");
	GMT_message (GMT, "\t      Replace wavelength by - to skip, e.g., -F-/-/500/100 is a low-pass filter.\n");
	GMT_message (GMT, "\t   b) Gaussian band-pass: Append two wavelengths <lo>/<hi> where filter amplitudes = 0.5.\n");
	GMT_message (GMT, "\t      Replace wavelength by - to skip, e.g., -F300/- is a high-pass Gaussian filter.\n");
	GMT_message (GMT, "\t   c) Butterworth band-pass: Append two wavelengths and order <lo>/<hi>/<order>\n");
	GMT_message (GMT, "\t      where filter amplitudes = 0.5.  Replace wavelength by - to skip, e.g.,\n");
	GMT_message (GMT, "\t      try -F300/-/2 for a high-pass 2nd-order Butterworth filter.\n");
	GMT_message (GMT, "\t-I Integrate, i.e., divide by kr [ * scale].  Use -Ig to get m from mGal].\n");
	GMT_message (GMT, "\t-L Leave trend alone:  Do not remove least squares plane from data [Default removes plane].\n");
	GMT_message (GMT, "\t   Append m to just remove mean or h to remove mid-value instead.\n");
	GMT_message (GMT, "\t-N Choose or inquire about suitable grid dimensions for FFT, and set modifiers:\n");
	GMT_message (GMT, "\t   -Nf will force the FFT to use the dimensions of the data.\n");
	GMT_message (GMT, "\t   -Nq will inQuire about more suitable dimensions, report, then exit.\n");
	GMT_message (GMT, "\t   -N<nx>/<ny> will do FFT on array size <nx>/<ny> (Must be >= grid size).\n");
	GMT_message (GMT, "\t   Default chooses dimensions >= data which optimize speed, accuracy of FFT.\n");
	GMT_message (GMT, "\t   If FFT dimensions > grid dimensions, data are extended via edge point symmetry\n");
	GMT_message (GMT, "\t   and tapered to zero.  Several modifers can be set to change this behavior:\n");
	GMT_message (GMT, "\t     +e: Extend data via edge point symmetry [Default].\n");
	GMT_message (GMT, "\t     +m: Extend data via edge mirror symmetry.\n");
	GMT_message (GMT, "\t     +n: Do NOT extend data.\n");
	GMT_message (GMT, "\t     +t<w>: Limit tapering to <w> %% of the extended margin width and height [100].\n");
	GMT_message (GMT, "\t       If +n is set then +t instead sets the boundary width of the interior\n");
	GMT_message (GMT, "\t       grid margin to be tapered [0].\n");
	GMT_message (GMT, "\t-Q Save intermediate grid passed to FFT after detrending/extention/tapering.\n");
	GMT_message (GMT, "\t   Append output file prefix [tapered].  File name will be <prefix>_<ingrid>.\n");
	GMT_message (GMT, "\t-S multiply field by scale after inverse FFT [1.0].\n");
	GMT_message (GMT, "\t   Give -Sd to convert deflection of vertical to micro-radians.\n");
	GMT_message (GMT, "\t-T Compute isostatic response.  Input file is topo load. Append elastic thickness,\n");
	GMT_message (GMT, "\t   and densities of load, mantle, water, and infill, all in SI units.\n");
	GMT_message (GMT, "\t   It also implicitly sets -L.\n");
	GMT_message (GMT, "\t-Z Store raw complex spectrum to files real_<ingrid> and imag_<ingrid>.\n");
	GMT_message (GMT, "\t   Append p to store polar forms instead, i.e., mag_<ingrid> and phase_<ingrid>\n");
	GMT_explain_options (GMT, "Vf.");
	GMT_message (GMT, "\t-ho Write header record for spectral estimates (requires -E) [no header].\n");
	GMT_message (GMT, "\tList operations in the order desired for execution.\n");

	return (EXIT_FAILURE);
}

void add_operation (struct GMT_CTRL *C, struct GRDFFT_CTRL *Ctrl, int operation, unsigned int n_par, double *par)
{
	Ctrl->n_op_count++;
	Ctrl->operation = GMT_memory (C, Ctrl->operation, Ctrl->n_op_count, int);
	Ctrl->operation[Ctrl->n_op_count-1] = operation;
	if (n_par) {
		Ctrl->par = GMT_memory (C, Ctrl->par, Ctrl->n_par + n_par, double);
		GMT_memcpy (&Ctrl->par[Ctrl->n_par], par, n_par, double);
		Ctrl->n_par += n_par;
	}
}

int GMT_grdfft_parse (struct GMTAPI_CTRL *C, struct GRDFFT_CTRL *Ctrl, struct F_INFO *f_info, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdfft and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int j, k, n_errors = 0, filter_type = 0, pos = 0;
	int n_scan;
	char p[GMT_BUFSIZ], *c = NULL;
	double par[5];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	GMT_memset (f_info, 1, struct F_INFO);
	for (j = 0; j < 3; j++) {
		f_info->lc[j] = f_info->lp[j] = -1.0;		/* Set negative, below valid frequency range  */
		f_info->hp[j] = f_info->hc[j] = DBL_MAX;	/* Set huge positive, above valid frequency range  */
	}

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */
		GMT_memset (par, 5, double);

		switch (opt->option) {
			case '<':	/* Input file (only 1 or 2 are accepted) */
				Ctrl->In.active = true;
				if (Ctrl->In.n_grids < 2) 
					Ctrl->In.file[Ctrl->In.n_grids++] = strdup (opt->arg);
				else {
					n_errors++;
					GMT_report (GMT, GMT_MSG_NORMAL, "Syntax error: A maximum of two input grids may be processed\n");
				}
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Directional derivative */
				Ctrl->A.active = true;
				n_errors += GMT_check_condition (GMT, sscanf(opt->arg, "%lf", &par[0]) != 1, 
						"Syntax error -A option: Cannot read azimuth\n");
				add_operation (GMT, Ctrl, AZIMUTHAL_DERIVATIVE, 1, par);
				break;
			case 'C':	/* Upward/downward continuation */
				Ctrl->C.active = true;
				n_errors += GMT_check_condition (GMT, sscanf(opt->arg, "%lf", &par[0]) != 1, 
						"Syntax error -C option: Cannot read zlevel\n");
				add_operation (GMT, Ctrl, UP_DOWN_CONTINUE, 1, par);
				break;
			case 'D':	/* d/dz */
				Ctrl->D.active = true;
				par[0] = (opt->arg[0]) ? ((opt->arg[0] == 'g' || opt->arg[0] == 'G') ? MGAL_AT_45 : atof (opt->arg)) : 1.0;
				n_errors += GMT_check_condition (GMT, par[0] == 0.0, "Syntax error -D option: scale must be nonzero\n");
				add_operation (GMT, Ctrl, DIFFERENTIATE, 1, par);
				break;
			case 'E':	/* x,y,or radial spectrum, w for wavelength; k for km if geographical */ 
				Ctrl->E.active = true;
				j = 0;
				while (opt->arg[j]) {
					switch (opt->arg[j]) {
						case 'r': Ctrl->E.mode =  0; break;
						case 'x': Ctrl->E.mode = +1; break;
						case 'y': Ctrl->E.mode = -1; break;
						case 'w': Ctrl->E.give_wavelength = true; break;
						case 'k': if (Ctrl->E.give_wavelength) Ctrl->E.km = true; break;
					}
					j++;
				}
				par[0] = Ctrl->E.mode;
				add_operation (GMT, Ctrl, SPECTRUM, 1, par);
				break;
			case 'F':	/* Filter */
				Ctrl->F.active = true;
				if (!(f_info->set_already)) {
					filter_type = count_slashes (opt->arg);
					f_info->kind = FILTER_EXP + (filter_type - 1);
					f_info->set_already = true;
					add_operation (GMT, Ctrl, f_info->kind, 0, NULL);
				}
				n_errors += GMT_check_condition (GMT, parse_f_string (GMT, f_info, opt->arg), "Syntax error -F option");
				break;
			case 'G':	/* Output file */
				Ctrl->G.active = true;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':	/* Integrate */
				Ctrl->I.active = true;
				par[0] = (opt->arg[0] == 'g' || opt->arg[0] == 'G') ? MGAL_AT_45 : atof (opt->arg);
				n_errors += GMT_check_condition (GMT, par[0] == 0.0, "Syntax error -I option: scale must be nonzero\n");
				add_operation (GMT, Ctrl, INTEGRATE, 1, par);
				break;
			case 'L':	/* Leave trend alone */
				if (opt->arg[0] == 'm') Ctrl->L.mode = 1;
				else if (opt->arg[0] == 'h') Ctrl->L.mode = 2;
				else Ctrl->L.active = true;
				break;
#ifdef GMT_COMPAT
			case 'M':	/* Geographic data */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -M is deprecated; -fg was set instead, use this in the future.\n");
				if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
				break;
#endif
			case 'N':	/* Grid dimension setting or inquiery */
				Ctrl->N.active = true;
				if ((c = strchr (opt->arg, '+'))) {	/* Handle modifiers */
					while ((GMT_strtok (c, "+", &pos, p))) {
						switch (p[0]) {
							case 'e':  Ctrl->N.mode = GRDFFT_EXTEND_POINT_SYMMETRY; break;
							case 'n':  Ctrl->N.mode = GRDFFT_EXTEND_NONE; break;
							case 'm':  Ctrl->N.mode = GRDFFT_EXTEND_MIRROR_SYMMETRY; break;
							case 't':  Ctrl->N.width = atof (&p[1]); break;
							default: 
								GMT_report (GMT, GMT_MSG_NORMAL, "Error -N: Unrecognized modifier +%s.\n", p);
								n_errors++;
								break;
						}
					}
				}
				switch (opt->arg[0]) {
					case 'f': case 'F':
						Ctrl->N.force_narray = true; break;
					case 'q': case 'Q':
						Ctrl->N.suggest_narray = true; break;
					default:
						sscanf (opt->arg, "%d/%d", &Ctrl->N.nx2, &Ctrl->N.ny2);
						Ctrl->N.n_user_set = true;
				}
				break;
			case 'S':	/* Scale */
				Ctrl->S.active = true;
				Ctrl->S.scale = (opt->arg[0] == 'd' || opt->arg[0] == 'D') ? 1.0e6 : atof (opt->arg);
				break;
			case 'T':	/* Flexural isostasy */
				Ctrl->T.active = Ctrl->L.active = true;
				n_scan = sscanf (opt->arg, "%lf/%lf/%lf/%lf/%lf", &par[0], &par[1], &par[2], &par[3], &par[4]);
				for (j = 1, k = 0; j < 5; j++) if (par[j] < 0.0) k++;
				n_errors += GMT_check_condition (GMT, n_scan != 5 || k > 0, 
					"Syntax error -T option: Correct syntax:\n\t-T<te>/<rhol>/<rhom>/<rhow>/<rhoi>, all densities >= 0\n");
				add_operation (GMT, Ctrl, ISOSTASY, 5, par);
				break;
#ifdef DEBUG
			case '=':	/* Do nothing */
				add_operation (GMT, Ctrl, -1, 1, par);
				break;
#endif
			case 'Q':	/* Output intermediate grid file */
				Ctrl->Q.active = true;
				if (opt->arg[0]) { free (Ctrl->Q.prefix); Ctrl->Q.prefix = strdup (opt->arg);}
				break;
			case 'Z':	/* Output raw complex spectrum */
				Ctrl->Z.active = true;
				if (opt->arg[0] == 'p') Ctrl->Z.mode = 1;	/* Store mag,phase instead of real,imag */
				break;
					break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !(Ctrl->n_op_count), "Syntax error: Must specify at least one operation\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.n_user_set && (Ctrl->N.nx2 <= 0 || Ctrl->N.ny2 <= 0), 
			"Syntax error -N option: nx2 and/or ny2 <= 0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.scale == 0.0, "Syntax error -S option: scale must be nonzero\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.mode == GRDFFT_EXTEND_NONE && Ctrl->N.width == 100.0, "Syntax error -N option: +n requires +t with width << 100!\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->E.active && !Ctrl->G.file, "Syntax error -G option: Must specify output file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdfft_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdfft (void *V_API, int mode, void *args)
{
	bool error = false, stop;
	int status;
	unsigned int op_count = 0, par_count = 0, side, k;
	uint64_t ij;

	struct GMT_GRID *Grid[2] = {NULL,  NULL}, *Orig[2] = {NULL,  NULL};
	struct F_INFO f_info;
	struct K_XY K;
	struct GRDFFT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_grdfft_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_grdfft_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_gmt_module (API, THIS_MODULE, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, "-Vfh", "", options)) Return (API->error);
	Ctrl = New_grdfft_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdfft_parse (API, Ctrl, &f_info, options))) Return (error);

	/*---------------------------- This is the grdfft main code ----------------------------*/

	for (k = 0; k < Ctrl->In.n_grids; k++) {	/* Get the grid header(s) */
		if ((Orig[k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER, NULL, Ctrl->In.file[k], NULL)) == NULL)	/* Get header only */
			Return (API->error);
	}

	if (Ctrl->In.n_grids == 2) {	/* Make sure grids are co-registered and same size, registration, etc. */
		if(Orig[0]->header->registration != Orig[1]->header->registration) {
			GMT_report (GMT, GMT_MSG_NORMAL, "The two grids have different registrations!\n");
			Return (EXIT_FAILURE);
		}
		if (!GMT_grd_same_shape (GMT, Orig[0], Orig[1])) {
			GMT_report (GMT, GMT_MSG_NORMAL, "The two grids have different dimensions\n");
			Return (EXIT_FAILURE);
		}
		if (!GMT_grd_same_region (GMT, Orig[0], Orig[1])) {
			GMT_report (GMT, GMT_MSG_NORMAL, "The two grids have different regions\n");
			Return (EXIT_FAILURE);
		}
		if (!GMT_grd_same_inc (GMT, Orig[0], Orig[1])) {
			GMT_report (GMT, GMT_MSG_NORMAL, "The two grids have different intervals\n");
			Return (EXIT_FAILURE);
		}
	}

	for (k = 0; k < Ctrl->In.n_grids; k++) {	/* Read, and check that no NaNs are present in either grid */
		GMT_grd_init (GMT, Orig[k]->header, options, true);	/* Update the header */
		set_grid_radix_size (GMT, Ctrl, Orig[k]);		/* Determine extended dimension and set the new pads */
		/* Because we taper and reflect below we DO NOT want any BCs set since that code expects 2 BC rows/cols */
		for (side = 0; side < 4; side++) Orig[k]->header->BC[side] = GMT_BC_IS_DATA;

		if ((Orig[k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA | GMT_GRID_COMPLEX_REAL, NULL, Ctrl->In.file[k], Orig[k])) == NULL)	/* Get data only */
			Return (API->error);
		for (ij = 0, stop = false; !stop && ij < Orig[k]->header->size; ij++) stop = GMT_is_fnan (Orig[k]->data[ij]);
		if (stop) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Input grid %s contain NaNs, aborting!\n", Ctrl->In.file[k]);
			Return (EXIT_FAILURE);
		}
	}
	
	/* OK, the data set(s) are clean. Time to read the data and set FFT dimensions */
	
	/* Note: If input grid(s) are read-only then we must duplicate; otherwise Grid[k] points to Orig[k] */
	for (k = 0; k < Ctrl->In.n_grids; k++) {
		(void) GMT_set_outgrid (GMT, Orig[k], &Grid[k]);
	}
	
	/* From here we address the first grid via Grid[0] and the 2nd grid as Grid[1];
	 * we are done with the addresses Orig[k] although they may be the same pointers. */
	
	for (k = 0; k < Ctrl->In.n_grids; k++) {	/* Detrend (if requested), extend/taper (if requested) */
		if (!(Ctrl->L.active)) remove_plane (GMT, Grid[k], Ctrl->L.mode);
		taper_edges (GMT, Grid[k], Ctrl->N.mode, Ctrl->N.width, Ctrl->N.nx2, Ctrl->N.ny2);
		if (Ctrl->Q.active) save_intermediate (GMT, Grid[k], Ctrl->Q.prefix);
	}

	/* Load K_XY structure with wavenumbers and dimensions */
	K.delta_kx = 2.0 * M_PI / (Ctrl->N.nx2 * Grid[0]->header->inc[GMT_X]);
	K.delta_ky = 2.0 * M_PI / (Ctrl->N.ny2 * Grid[0]->header->inc[GMT_Y]);
	K.nx2 = Ctrl->N.nx2;	K.ny2 = Ctrl->N.ny2;

	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Give delta_kx, delta_ky units of 2pi/meters  */
		K.delta_kx /= (GMT->current.proj.DIST_M_PR_DEG * cosd (0.5 * (Grid[0]->header->wesn[YLO] + Grid[0]->header->wesn[YHI])) );
		K.delta_ky /= GMT->current.proj.DIST_M_PR_DEG;
	}

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

	for (k = 0; k < Ctrl->In.n_grids; k++) {	/* Call the forward FFT */
		GMT_report (GMT, GMT_MSG_VERBOSE, "forward FFT...\n");
		if (GMT_fft_2d (GMT, Grid[k]->data, Ctrl->N.nx2, Ctrl->N.ny2, k_fft_fwd, k_fft_complex))
			Return (EXIT_FAILURE);
		if (Ctrl->Z.active) save_complex_grid (GMT, Grid[k], Ctrl->Z.mode, &K, Ctrl->In.file[k]);
	}

#if 1
	for (op_count = par_count = 0; op_count < Ctrl->n_op_count; op_count++) {
		switch (Ctrl->operation[op_count]) {
			case UP_DOWN_CONTINUE:
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) ((Ctrl->par[par_count] < 0.0) ? GMT_message (GMT, "downward continuation...\n") : GMT_message (GMT,  "upward continuation...\n"));
				par_count += do_continuation (Grid[0], &Ctrl->par[par_count], &K);
				break;
			case AZIMUTHAL_DERIVATIVE:
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_message (GMT, "azimuthal derivative...\n");
				par_count += do_azimuthal_derivative (Grid[0], &Ctrl->par[par_count], &K);
				break;
			case DIFFERENTIATE:
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_message (GMT, "differentiate...\n");
				par_count += do_differentiate (Grid[0], &Ctrl->par[par_count], &K);
				break;
			case INTEGRATE:
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_message (GMT, "integrate...\n");
				par_count += do_integrate (Grid[0], &Ctrl->par[par_count], &K);
				break;
			case ISOSTASY:
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_message (GMT, "isostasy...\n");
				par_count += do_isostasy (Grid[0], Ctrl, &Ctrl->par[par_count], &K);
				break;
			case FILTER_COS:
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_message (GMT, "cosine filter...\n");
				do_filter (Grid[0], &f_info, &K);
				break;
			case FILTER_EXP:
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_message (GMT, "Gaussian filter...\n");
				do_filter (Grid[0], &f_info, &K);
				break;
			case FILTER_BW:
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_message (GMT, "Butterworth filter...\n");
				do_filter (Grid[0], &f_info, &K);
				break;
			case SPECTRUM:	/* This operator writes a table to file (or stdout if -G is not used) */
				if (Ctrl->In.n_grids == 2) {	/* Cross-spectral calculations */
					if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_message (GMT, "cross-spectra...\n");
					status = do_cross_spectrum (GMT, Grid[0], Grid[1], &Ctrl->par[par_count], Ctrl->E.give_wavelength, Ctrl->E.km, Ctrl->G.file, &K);
				}
				else {	/* Spectral calculations */
					if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_message (GMT, "spectrum...\n");
					status = do_spectrum (GMT, Grid[0], &Ctrl->par[par_count], Ctrl->E.give_wavelength, Ctrl->E.km, Ctrl->G.file, &K);
				}
				if (status < 0) Return (status);
				par_count += status;
				break;
		}
	}
#endif
	if (!Ctrl->E.active) {	/* Since -E out was handled separately by do_spectrum/do_cross_spectrum */
		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_message (GMT, "inverse FFT...\n");

		if (GMT_fft_2d (GMT, Grid[0]->data, Ctrl->N.nx2, Ctrl->N.ny2, k_fft_inv, k_fft_complex))
			Return (EXIT_FAILURE);

		/* FFT computes an unnormalized transform, in that there is no
		 * coefficient in front of the summation in the FT. In other words,
		 * applying the forward and then the backward transform will multiply the
		 * input by the number of elements (header->size). Here we correct this: */
		Ctrl->S.scale *= (2.0 / Grid[0]->header->size);
		GMT_scale_and_offset_f (GMT, Grid[0]->data, Grid[0]->header->size, Ctrl->S.scale, 0);

		/* The data are in the middle of the padded array */
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA | GMT_GRID_COMPLEX_REAL, NULL, Ctrl->G.file, Grid[0]) != GMT_OK) {
			Return (API->error);
		}
	}

	Return (EXIT_SUCCESS);
}
