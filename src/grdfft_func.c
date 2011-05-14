/*--------------------------------------------------------------------
 *	$Id: grdfft_func.c,v 1.22 2011-05-14 00:04:06 guru Exp $
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
 *  Brief synopsis: grdfft.c is a program to do various operations on
 *  grid files in the frequency domain.
 *
 * Author:	W.H.F. Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 */

#include "gmt.h"

struct GRDFFT_CTRL {
	GMT_LONG n_op_count, n_par;
	GMT_LONG *operation;
	double *par;

	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct A {	/* -A<azimuth> */
		GMT_LONG active;
		double value;
	} A;
	struct C {	/* -C<zlevel> */
		GMT_LONG active;
		double value;
	} C;
	struct D {	/* -D[<scale>|g] */
		GMT_LONG active;
		double value;
	} D;
	struct E {	/* -E[x_or_y][w] */
		GMT_LONG active;
		GMT_LONG give_wavelength;
		GMT_LONG mode;
	} E;
	struct F {	/* -F[x_or_y]<lc>/<lp>/<hp>/<hc> or -F[x_or_y]<lo>/<hi> */
		GMT_LONG active;
		GMT_LONG mode;
		double lc, lp, hp, hc;
	} F;
	struct G {	/* -G<outfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -I[<scale>|g] */
		GMT_LONG active;
		double value;
	} I;
	struct L {	/* -L */
		GMT_LONG active;
	} L;
	struct N {	/* -N<stuff> */
		GMT_LONG active;
		GMT_LONG force_narray, suggest_narray, n_user_set;
		GMT_LONG nx2, ny2;
		double value;
	} N;
	struct S {	/* -S<scale> */
		GMT_LONG active;
		double scale;
	} S;
	struct T {	/* -T<te/rl/rm/rw/ri> */
		GMT_LONG active;
		double te, rhol, rhom, rhow, rhoi;
	} T;
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
#define	NORMAL_GRAVITY	9.80619203	/* m/s**2  */
#define	POISSONS_RATIO	0.25

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
	PFD filter;		/* Points to the correct filter function */
	GMT_LONG do_this[3];	/* T/F this filter wanted for r, x, and y	*/
	GMT_LONG set_already;	/* TRUE if we already filled in the structure */
	GMT_LONG kind;		/* FILTER_EXP, FILTER_BW, FILTER_COS  */
	GMT_LONG arg;		/* 0 = Gaussian, 1 = Butterworth, 2 = cosine taper,  */
};

struct FFT_SUGGESTION {
	GMT_LONG nx;
	GMT_LONG ny;
	GMT_LONG worksize;	/* # single-complex elements needed in work array  */
	GMT_LONG totalbytes;	/* (8*(nx*ny + worksize))  */
	double run_time;
	double rms_rel_err;
}; /* [0] holds fastest, [1] most accurate, [2] least storage  */

struct K_XY {	/* Holds parameters needed to calculate kx, ky, kr */
	GMT_LONG nx2, ny2;
	double delta_kx, delta_ky;
};

void *New_grdfft_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDFFT_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDFFT_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->S.scale = 1.0;
	return ((void *)C);
}

void Free_grdfft_Ctrl (struct GMT_CTRL *GMT, struct GRDFFT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->operation) GMT_free (GMT, C->operation);	
	if (C->par) GMT_free (GMT, C->par);	
	if (C->In.file) free ((void *)C->In.file);	
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}

void remove_plane (struct GMT_CTRL *GMT, struct GMT_GRID *Grid)
{
	/* Remove the best-fitting plane by least squares.

	Let plane be z = a0 + a1 * x + a2 * y.  Choose the
	center of x,y coordinate system at the center of 
	the array.  This will make the Normal equations 
	matrix G'G diagonal, so solution is trivial.  Also,
	spend some multiplications on normalizing the 
	range of x,y into [-1,1], to avoid roundoff error.  */

	GMT_LONG i, j, ij, one_or_zero, i_data_start, j_data_start;
	double x_half_length, one_on_xhl, y_half_length, one_on_yhl;
	double sumx2, sumy2, data_var, x, y, z, a[3];
	float *datac = Grid->data;

	one_or_zero = (Grid->header->registration == GMT_PIXEL_REG) ? 0 : 1;
	x_half_length = 0.5 * (Grid->header->nx - one_or_zero);
	one_on_xhl = 1.0 / x_half_length;
	y_half_length = 0.5 * (Grid->header->ny - one_or_zero);
	one_on_yhl = 1.0 / y_half_length;

	sumx2 = sumy2 = data_var = a[2] = a[1] = a[0] = 0.0;
	i_data_start = GMT->current.io.pad[XLO];
	j_data_start = GMT->current.io.pad[YHI];

	for (j = 0; j < Grid->header->ny; j++) {
		y = one_on_yhl * (j - y_half_length);
		for (i = 0; i < Grid->header->nx; i++) {
			x = one_on_xhl * (i - x_half_length);
			z = datac[GMT_IJPR(Grid->header,j,i)];
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
	data_var = sqrt(data_var / (Grid->header->nx*Grid->header->ny - 1));
	/* Rescale a1,a2 into user's units, in case useful later */
	a[1] *= (2.0/(Grid->header->wesn[XHI] - Grid->header->wesn[XLO]));
	a[2] *= (2.0/(Grid->header->wesn[YHI] - Grid->header->wesn[YLO]));
	GMT_report (GMT, GMT_MSG_NORMAL, "Plane removed.  Mean, S.D., Dx, Dy: %.8g\t%.8g\t%.8g\t%.8g\n", a[0], data_var, a[1], a[2]);
}

void taper_edges (struct GMT_CTRL *GMT, struct GMT_GRID *Grid)
{
	GMT_LONG im, jm, il1, ir1, il2, ir2, jb1, jb2, jt1, jt2;
	GMT_LONG i, j, i_data_start, j_data_start;
	float *datac = Grid->data;
	double scale, cos_wt;
	struct GRD_HEADER *h = Grid->header;	/* For shorthand */

	/* Note that if nx2 = nx+1 and ny2 = ny + 1, then this routine
	 * will do nothing; thus a single row/column of zeros may be
	 * added to the bottom/right of the input array and it cannot
	 * be tapered.  But when (nx2 - nx)%2 == 1 or ditto for y,
	 * this is zero anyway.  */

	i_data_start = GMT->current.io.pad[XLO];	/* For readability */
	j_data_start = GMT->current.io.pad[YHI];
	
	/* First reflect about xmin and xmax, point symmetric about edge point */

	for (im = 1; im <= i_data_start; im++) {
		il1 = -im;	/* Outside xmin; left of edge 1  */
		ir1 = im;	/* Inside xmin; right of edge 1  */
		il2 = il1 + h->nx - 1;	/* Inside xmax; left of edge 2  */
		ir2 = ir1 + h->nx - 1;	/* Outside xmax; right of edge 2  */
		for (j = 0; j < h->ny; j++) {
			datac[GMT_IJPR(h,j,il1)] = (float)2.0*datac[GMT_IJPR(h,j,0)] - datac[GMT_IJPR(h,j,ir1)];
			datac[GMT_IJPR(h,j,ir2)] = (float)2.0*datac[GMT_IJPR(h,j,h->nx-1)] - datac[GMT_IJPR(h,j,il2)];
		}
	}

	/* Next, reflect about ymin and ymax.
	 * At the same time, since x has been reflected,
	 * we can use these vals and taper on y edges */

	scale = M_PI / (j_data_start + 1);

	for (jm = 1; jm <= j_data_start; jm++) {
		jb1 = -jm;	/* Outside ymin; bottom side of edge 1  */
		jt1 = jm;	/* Inside ymin; top side of edge 1  */
		jb2 = jb1 + h->ny - 1;	/* Inside ymax; bottom side of edge 2  */
		jt2 = jt1 + h->ny - 1;	/* Outside ymax; bottom side of edge 2  */
		cos_wt = 0.5 * (1.0 + cos(jm * scale) );
		for (i = -i_data_start; i < h->mx - i_data_start; i++) {
			datac[GMT_IJPR(h,jb1,i)] = (float)(cos_wt * (2.0*datac[GMT_IJPR(h,0,i)] - datac[GMT_IJPR(h,jt1,i)]));
			datac[GMT_IJPR(h,jt2,i)] = (float)(cos_wt * (2.0*datac[GMT_IJPR(h,h->ny-1,i)] - datac[GMT_IJPR(h,jb2,i)]));
		}
	}

	/* Now, cos taper the x edges */
	scale = M_PI / (i_data_start + 1);
	for (im = 1; im <= i_data_start; im++) {
		il1 = -im;
		ir1 = im;
		il2 = il1 + h->nx - 1;
		ir2 = ir1 + h->nx - 1;
		cos_wt = 0.5 * (1.0 + cos (im * scale));
		for (j = -j_data_start; j < h->my - j_data_start; j++) {
			datac[GMT_IJPR(h,j,il1)] *= (float)cos_wt;
			datac[GMT_IJPR(h,j,ir2)] *= (float)cos_wt;
		}
	}
	GMT_report (GMT, GMT_MSG_NORMAL, "Data reflected and tapered\n");
}

double kx (GMT_LONG k, struct K_XY *K)
{
	/* Return the value of kx given k,
	 * where kx = 2 pi / lambda x,
	 * and k refers to the position
	 * in the datac array, datac[k].  */

	GMT_LONG ii = (k/2)%(K->nx2);
	if (ii > (K->nx2)/2) ii -= (K->nx2);
	return (ii * K->delta_kx);
}

double ky (GMT_LONG k, struct K_XY *K)
{
	/* Return the value of ky given k,
	 * where ky = 2 pi / lambda y,
	 * and k refers to the position
	 *in the datac array, datac[k].  */

	GMT_LONG jj = (k/2)/(K->nx2);
	if (jj > (K->ny2)/2) jj -= (K->ny2);
	return (jj * K->delta_ky);
}

double modk (GMT_LONG k, struct K_XY *K)
{
	/* Return the value of sqrt(kx*kx + ky*ky),
	 * given k, where k is array position.  */

	return (hypot (kx (k, K), ky (k, K)));
}

GMT_LONG do_differentiate (struct GMT_GRID *Grid, double *par, struct K_XY *K)
{
	GMT_LONG k;
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

GMT_LONG do_integrate (struct GMT_GRID *Grid, double *par, struct K_XY *K)
{
	/* Integrate in frequency domain by dividing by kr [scale optional] */
	GMT_LONG k;
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

GMT_LONG do_continuation (struct GMT_GRID *Grid, double *zlevel, struct K_XY *K)
{
	GMT_LONG k;
	float tmp, *datac = Grid->data;

	/* If z is positive, the field will be upward continued using exp[- k z].  */

	for (k = 2; k < Grid->header->size; k += 2) {
		tmp = (float)exp (-(*zlevel) * modk (k, K));
		datac[k]   *= tmp;
		datac[k+1] *= tmp;
	}
	return (1);	/* Number of parameters used */
}

GMT_LONG do_azimuthal_derivative (struct GMT_GRID *Grid, double *azim, struct K_XY *K)
{
	GMT_LONG k;
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

GMT_LONG do_isostasy (struct GMT_GRID *Grid, struct GRDFFT_CTRL *Ctrl, double *par, struct K_XY *K)
{

	/* Do the isostatic response function convolution in the Freq domain.
	All units assumed to be in SI (that is kx, ky, modk wavenumbers in m**-1,
	densities in kg/m**3, Te in m, etc.
	rw, the water density, is used to set the Airy ratio and the restoring
	force on the plate (rm - ri)*gravity if ri = rw; so use zero for topo in air.  */
	GMT_LONG k;
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

double get_filter_weight (GMT_LONG k, struct F_INFO *f_info, struct K_XY *K)
{
	GMT_LONG j;
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
	GMT_LONG k;
	float weight, *datac = Grid->data;

	for (k = 0; k < Grid->header->size; k += 2) {
		weight = (float) get_filter_weight (k, f_info, K);
		datac[k]   *= weight;
		datac[k+1] *= weight;
	}
}

GMT_LONG count_slashes (char *txt)
{
	GMT_LONG i, n;
	for (i = n = 0; txt[i]; i++) if (txt[i] == '/') n++;
	return (n);
}

GMT_LONG parse_f_string (struct GMT_CTRL *GMT, struct F_INFO *f_info, char *c)
{
	GMT_LONG i, j, n_tokens, pos, descending;
	double fourvals[4];
	char line[GMT_TEXT_LEN256], p[GMT_TEXT_LEN256];
	
	/* Syntax is either -F[x|y]lc/hc/lp/hp (Cosine taper), -F[x|y]lo/hi (Gaussian), or 0F[x|y]lo/hi/order (Butterworth) */
	
	strcpy(line, c);
	i = j = 0;	/* j is Filter type: r=0, x=1, y=2  */
	
	if (line[i] == 'x') {
		j = 1;
		i++;
	}
	else if (line[i] == 'y') {
		j = 2;
		i++;
	}
	
	f_info->do_this[j] = TRUE;
	fourvals[0] = fourvals[1] = fourvals[2] = fourvals[3] = -1.0;
	
	n_tokens = pos = 0;
	while ((GMT_strtok (&line[i], "/", &pos, p))) {
		if (n_tokens > 3) {
			GMT_report (GMT, GMT_MSG_FATAL, "Too many slashes in -F.\n");
			return (TRUE);
		}
		if(p[0] == '-')
			fourvals[n_tokens] = -1.0;
		else {
			if ((sscanf(p, "%lf", &fourvals[n_tokens])) != 1) {
				GMT_report (GMT, GMT_MSG_FATAL, " Cannot read token %ld.\n", n_tokens);
				return (TRUE);
			}
		}
		n_tokens++;
	}
	
	if (!(n_tokens == 2 || n_tokens == 3 || n_tokens == 4)) {
		GMT_report (GMT, GMT_MSG_FATAL, "-F Cannot find 2-4 tokens separated by slashes.\n");
		return (TRUE);
	}
	descending = TRUE;
	if (f_info->kind == FILTER_BW && n_tokens == 3) n_tokens = 2;	/* So we dont check the order as a wavelength */

	for (i = 1; i < n_tokens; i++) {
		if (fourvals[i] == -1.0 || fourvals[i-1] == -1.0) continue;
		if (fourvals[i] > fourvals[i-1]) descending = FALSE;
	}
	if (!(descending)) {
		GMT_report (GMT, GMT_MSG_FATAL, "-F Wavelengths are not in descending order.\n");
		return (TRUE);
	}
	if (f_info->kind == FILTER_COS) {	/* Cosine band-pass specification */
		if ((fourvals[0] * fourvals[1]) < 0.0 || (fourvals[2] * fourvals[3]) < 0.0) {
			GMT_report (GMT, GMT_MSG_FATAL, "-F Pass/Cut specification error.\n");
			return (TRUE);
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
		f_info->filter = (PFD) cosine_weight_grdfft;
	}
	else if (f_info->kind == FILTER_BW) {	/* Butterworth specification */
		f_info->llambda[j] = (fourvals[0] == -1.0) ? -1.0 : fourvals[0] / TWO_PI;	/* TWO_PI is used to counteract the 2*pi in the wavenumber */
		f_info->hlambda[j] = (fourvals[1] == -1.0) ? -1.0 : fourvals[1] / TWO_PI;
		f_info->bw_order = 2.0 * fourvals[2];
		f_info->filter = (PFD) bw_weight;
	}
	else {	/* Gaussian half-amp specifications */
		f_info->llambda[j] = (fourvals[0] == -1.0) ? -1.0 : fourvals[0] / TWO_PI;	/* TWO_PI is used to counteract the 2*pi in the wavenumber */
		f_info->hlambda[j] = (fourvals[1] == -1.0) ? -1.0 : fourvals[1] / TWO_PI;
		f_info->filter = (PFD) gauss_weight;
	}
	f_info->arg = f_info->kind - FILTER_EXP;
	return (FALSE);
}

GMT_LONG do_spectrum (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, double *par, GMT_LONG give_wavelength, char *file, struct K_XY *K)
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

	char format[GMT_TEXT_LEN64];
	GMT_LONG k, nk, nused, ifreq, error, ID, dim[4] = {1, 1, 1, 0};
	double delta_k, r_delta_k, freq, *power = NULL, eps_pow, powfactor;
	PFD get_k;
	float *datac = Grid->data;
	struct GMT_DATASET *D = NULL;
	struct GMT_LINE_SEGMENT *S = NULL;

	if (*par > 0.0) {
		/* X spectrum desired  */
		delta_k = K->delta_kx;
		nk = K->nx2 / 2;
		get_k = (PFD)kx;
	}
	else if (*par < 0.0) {
		/* Y spectrum desired  */
		delta_k = K->delta_ky;
		nk = K->ny2 / 2;
		get_k = (PFD)ky;
	}
	else {
		/* R spectrum desired  */
		if (K->delta_kx < K->delta_ky) {
			delta_k = K->delta_kx;
			nk = K->nx2 / 2;
		}
		else {
			delta_k = K->delta_ky;
			nk = K->ny2 / 2;
		}
		get_k = (PFD)modk;
	}

	/* Get an array for summing stuff */
	power = GMT_memory (GMT, NULL, nk, double);

	/* Loop over it all, summing and storing, checking range for r */

	r_delta_k = 1.0 / delta_k;
	
	for (nused = 0, k = 2; k < Grid->header->size; k += 2) {
		freq = (*get_k)(k, K);
		ifreq = irint (fabs (freq) * r_delta_k) - 1;
		if (ifreq < 0) ifreq = 0;	/* Might happen when doing r spectrum  */
		if (ifreq >= nk) continue;	/* Might happen when doing r spectrum  */
		power[ifreq] += hypot (datac[k], datac[k+1]);
		nused++;
	}

	/* Now get here when array is summed.  */
	eps_pow = 1.0 / sqrt ((double)nused/(double)nk);
	delta_k /= (2.0 * M_PI);	/* Write out frequency, not wavenumber  */
	sprintf (format, "%s\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	powfactor = 4.0 / pow ((double)Grid->header->size, 2.0);
	dim[2] = 3;	dim[3] = nk;
	if ((error = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, dim, (void **)&D, -1, &ID))) {
		GMT_report (GMT, GMT_MSG_FATAL, "Unable to create a data set for spectrum\n");
		return (GMT_RUNTIME_ERROR);
	}
	S = D->table[0]->segment[0];	/* Only one table with one segment here, with 3 cols and nk rows */
	for (k = 0; k < nk; k++) {
		freq = (k + 1) * delta_k;
		if (give_wavelength) freq = 1.0 / freq;
		power[k] *= powfactor;
		S->coord[GMT_X][k] = freq;
		S->coord[GMT_Y][k] = power[k];
		S->coord[GMT_Y][k] = eps_pow * power[k];
	}
	if ((error = GMT_Begin_IO (GMT->parent, GMT_IS_DATASET, GMT_OUT, GMT_BY_SET))) return (error);	/* Enables data output and sets access mode */
	if ((error = GMT_Put_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_STREAM, GMT_IS_POINT, NULL, 0, (void **)&file, (void *)D))) return (error);
	if ((error = GMT_End_IO (GMT->parent, GMT_OUT, 0))) return (error);			/* Disables further data output */
	GMT_Destroy_Data (GMT->parent, GMT_ALLOCATED, (void **)&D);
	GMT_free (GMT, power);
	return (2);	/* Number of parameters used */
}

GMT_LONG get_non_symmetric_f (GMT_LONG *f, GMT_LONG n)
{
	/* Return the product of the non-symmetric factors in f[]  */
	GMT_LONG i = 0, j = 1, retval = 1;

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

void fourt_stats (GMT_LONG nx, GMT_LONG ny, GMT_LONG *f, double *r, GMT_LONG *s, double *t)
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

	GMT_LONG n_factors, i, sum2, sumnot2, nnot2;
	GMT_LONG nonsymx, nonsymy, nonsym, storage, ntotal;
	double err_scale;
	EXTERN_MSC GMT_LONG GMT_get_prime_factors (GMT_LONG n, GMT_LONG *f);

	/* Find workspace needed.  First find non_symmetric factors in nx, ny  */
	n_factors = GMT_get_prime_factors (nx, f);
	nonsymx = get_non_symmetric_f (f, n_factors);
	n_factors = GMT_get_prime_factors (ny, f);
	nonsymy = get_non_symmetric_f (f, n_factors);
	nonsym = MAX (nonsymx, nonsymy);

	/* Now get factors of ntotal  */
	ntotal = GMT_get_nm (nx,ny);
        n_factors = GMT_get_prime_factors (ntotal, f);
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

void suggest_fft (struct GMT_CTRL *GMT, GMT_LONG nx, GMT_LONG ny, struct FFT_SUGGESTION *fft_sug, GMT_LONG do_print)
{
	GMT_LONG f[32], xstop, ystop;
	GMT_LONG nx_best_t, ny_best_t;
	GMT_LONG nx_best_e, ny_best_e;
	GMT_LONG nx_best_s, ny_best_s;
        GMT_LONG nxg, nyg;       /* Guessed by this routine  */
        GMT_LONG nx2, ny2, nx3, ny3, nx5, ny5;   /* For powers  */
	GMT_LONG current_space, best_space, given_space, e_space, t_space;
	double current_time, best_time, given_time, s_time, e_time;
	double current_err, best_err, given_err, s_err, t_err;

	fourt_stats (nx, ny, f, &given_err, &given_space, &given_time);
	given_space += nx * ny;
	given_space *= 8;
	if (do_print) GMT_report (GMT, GMT_MSG_FATAL, " Data dimension\t%ld %ld\ttime factor %.8g\trms error %.8e\tbytes %ld\n",
		nx, ny, given_time, given_err, given_space);

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

			fourt_stats (nxg, nyg, f, &current_err, &current_space, &current_time);
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
		GMT_report (GMT, GMT_MSG_FATAL, " Highest speed\t%ld %ld\ttime factor %.8g\trms error %.8e\tbytes %ld\n",
			nx_best_t, ny_best_t, best_time, t_err, t_space);
		GMT_report (GMT, GMT_MSG_FATAL, " Most accurate\t%ld %ld\ttime factor %.8g\trms error %.8e\tbytes %ld\n",
			nx_best_e, ny_best_e, e_time, best_err, e_space);
		GMT_report (GMT, GMT_MSG_FATAL, " Least storage\t%ld %ld\ttime factor %.8g\trms error %.8e\tbytes %ld\n",
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
	GMT_LONG k, worksize, factors[32];
	double tdummy, edummy;
	struct FFT_SUGGESTION fft_sug[3];
		
	/* Get dimensions as may be appropriate */
	if (Ctrl->N.n_user_set) {
		if (Ctrl->N.nx2 < Gin->header->nx || Ctrl->N.ny2 < Gin->header->ny) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: You specified -Nnx/ny smaller than input grid.  Ignored.\n");
			Ctrl->N.n_user_set = FALSE;
		}
	}
	if (!(Ctrl->N.n_user_set) ) {
		if (Ctrl->N.force_narray) {
			Ctrl->N.nx2 = Gin->header->nx;
			Ctrl->N.ny2 = Gin->header->ny;
			for (k = 0; k < 4; k++) Gin->header->BC[k] = GMT_BC_IS_DATA;	/* This bypasses BC pad checking later since there is no pad */
		}
		else {
			suggest_fft (GMT, (GMT_LONG)Gin->header->nx, (GMT_LONG)Gin->header->ny, fft_sug, (GMT_is_verbose (GMT, GMT_MSG_NORMAL) || Ctrl->N.suggest_narray));
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

	fourt_stats (Ctrl->N.nx2, Ctrl->N.ny2, factors, &edummy, &worksize, &tdummy);
	GMT_report (GMT, GMT_MSG_NORMAL, " Data dimension %d %d\tFFT dimension %ld %ld\n",
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

GMT_LONG GMT_grdfft_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdfft %s [API] - Perform mathematical operations on grid files in the wavenumber (or frequency) domain\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdfft <in_grdfile> [-G<out_grdfile>] [-A<azimuth>] [-C<zlevel>]\n");
	GMT_message (GMT, "\t[-D[<scale>|g]] [-E[x_or_y][w]] [-F[x_or_y]<parameters>] [-I[<scale>|g]] [-L]\n");
	GMT_message (GMT, "\t[-N<stuff>] [-S<scale>] [-T<te/rl/rm/rw/ri>] [%s] [%s]\n\n", GMT_V_OPT, GMT_f_OPT);
	
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);
	
	GMT_message (GMT, "\tin_grdfile is the input netCDF grid file\n");
	GMT_message (GMT, "\tOPTIONS:\n");
	GMT_message (GMT, "\t-G filename for output netCDF grid file OR 1-D spectrum (see -E).\n");
	GMT_message (GMT, "\t-A Take azimuthal derivative along line <azimuth> degrees CW from North.\n");
	GMT_message (GMT, "\t-C Continue field upward (+) or downward (-) to <zlevel> (meters).\n");
	GMT_message (GMT, "\t-D Differentiate, i.e., multiply by kr [ * scale].  Use -Dg to get mGal from m].\n");
	GMT_message (GMT, "\t-E Estimate spEctrum of r [x] [y].  Write f, power[f], 1 std dev(power[f]) to output file\n");
	GMT_message (GMT, "\t   (see -G) or stdout.   Append w to write wavelength instead of frequency.\n");
	GMT_message (GMT, "\t-F Filter r [x] [y] freq according to one of three kinds of filter specifications:\n");
	GMT_message (GMT, "\t   a) Cosine band-pass: Append four wavelengths <lc>/<lp>/<hp>/<hc>.\n");
	GMT_message (GMT, "\t      freq outside <lc>/<hc> are cut; inside <lp>/<hp> are passed, rest are tapered.\n");
	GMT_message (GMT, "\t      Replace wavelength by - to skip, e.g.  -F-/-/500/100 is a low-pass filter.\n");
	GMT_message (GMT, "\t   b) Gaussian band-pass: Append two wavelengths <lo>/<hi> where filter amplitudes = 0.5.\n");
	GMT_message (GMT, "\t      Replace wavelength by - to skip, e.g.  -F300/- is a high-pass Gaussian filter.\n");
	GMT_message (GMT, "\t   c) Butterworth band-pass: Append two wavelengths and order <lo>/<hi>/<order> where filter amplitudes = 0.5.\n");
	GMT_message (GMT, "\t      Replace wavelength by - to skip, e.g.  -F300/-/2 is a high-pass 2nd-order Butterworth filter.\n");
	GMT_message (GMT, "\t-I Integrate, i.e., divide by kr [ * scale].  Use -Ig to get m from mGal].\n");
	GMT_message (GMT, "\t-L Leave trend alone.  Do not remove least squares plane from data [Default removes plane].\n");
	GMT_message (GMT, "\t-N Choose or inquire about suitable grid dimensions for FFT.\n");
	GMT_message (GMT, "\t   -Nf will force the FFT to use the dimensions of the data.\n");
	GMT_message (GMT, "\t   -Nq will inQuire about more suitable dimensions.\n");
	GMT_message (GMT, "\t   -N<nx>/<ny> will do FFT on array size <nx>/<ny> (Must be >= grid size).\n");
	GMT_message (GMT, "\t   Default chooses dimensions >= data which optimize speed, accuracy of FFT.\n");
	GMT_message (GMT, "\t   If FFT dimensions > grid dimensions, data are extended and tapered to zero.\n");
	GMT_message (GMT, "\t-S multiply field by scale after inverse FFT [1.0].\n");
	GMT_message (GMT, "\t   Give -Sd to convert deflection of vertical to micro-radians.\n");
	GMT_message (GMT, "\t-T Compute isostatic response.  Input file is topo load. Append elastic thickness,\n");
	GMT_message (GMT, "\t   and densities of load, mantle, water, and infill, all in SI units.\n");
	GMT_message (GMT, "\t   It also implicitly sets -L.\n");
	GMT_explain_options (GMT, "Vf.");
	GMT_message (GMT, "\tList operations in the order desired for execution.\n");
	
	return (EXIT_FAILURE);
}

void add_operation (struct GMT_CTRL *C, struct GRDFFT_CTRL *Ctrl, GMT_LONG operation, GMT_LONG n_par, double *par)
{
	Ctrl->n_op_count++;
	Ctrl->operation = GMT_memory (C, Ctrl->operation, Ctrl->n_op_count, GMT_LONG);
	Ctrl->operation[Ctrl->n_op_count-1] = operation;
	Ctrl->par = GMT_memory (C, Ctrl->par, Ctrl->n_par + n_par, double);
	GMT_memcpy (&Ctrl->par[Ctrl->n_par], par, n_par, double);
	Ctrl->n_par += n_par;
}

GMT_LONG GMT_grdfft_parse (struct GMTAPI_CTRL *C, struct GRDFFT_CTRL *Ctrl, struct F_INFO *f_info, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdfft and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j, n, k, n_errors = 0, n_files = 0, filter_type = 0;
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
			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Directional derivative */
				Ctrl->A.active = TRUE;
				n_errors += GMT_check_condition (GMT, sscanf(opt->arg, "%lf", &par[0]) != 1, "Syntax error -A option: Cannot read azimuth\n");
				add_operation (GMT, Ctrl, AZIMUTHAL_DERIVATIVE, 1, par);
				break;
			case 'C':	/* Upward/downward continuation */
				Ctrl->C.active = TRUE;
				n_errors += GMT_check_condition (GMT, sscanf(opt->arg, "%lf", &par[0]) != 1, "Syntax error -C option: Cannot read zlevel\n");
				add_operation (GMT, Ctrl, UP_DOWN_CONTINUE, 1, par);
				break;
			case 'D':	/* d/dz */
				Ctrl->D.active = TRUE;
				par[0] = (opt->arg[0]) ? ((opt->arg[0] == 'g' || opt->arg[0] == 'G') ? MGAL_AT_45 : atof (opt->arg)) : 1.0;
				n_errors += GMT_check_condition (GMT, par[0] == 0.0, "Syntax error -D option: scale must be nonzero\n");
				add_operation (GMT, Ctrl, DIFFERENTIATE, 1, par);
				break;
			case 'E':	/* x,y,or radial spectrum */ 
				Ctrl->E.active = TRUE;
				j = 0;
				while (opt->arg[j]) {
					switch (opt->arg[j]) {
						case 'x': case 'X': par[0] = +1.0; break;
						case 'y': case 'Y': par[0] = -1.0; break;
						case 'w': case 'W': Ctrl->E.give_wavelength = TRUE; break;
					}
					j++;
				}
				add_operation (GMT, Ctrl, SPECTRUM, 1, par);
				break;
			case 'F':	/* Filter */
				Ctrl->F.active = TRUE;
				if (!(f_info->set_already)) {
					filter_type = count_slashes (opt->arg);
					f_info->kind = FILTER_EXP + (filter_type - 1);
					f_info->set_already = TRUE;
					add_operation (GMT, Ctrl, f_info->kind, 0, NULL);
				}
				n_errors += GMT_check_condition (GMT, parse_f_string (GMT, f_info, opt->arg), "Syntax error -F option");
				break;
			case 'G':	/* Output file */
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':	/* Integrate */
				Ctrl->I.active = TRUE;
				par[0] = (opt->arg[0] == 'g' || opt->arg[0] == 'G') ? MGAL_AT_45 : atof (opt->arg);
				n_errors += GMT_check_condition (GMT, par[0] == 0.0, "Syntax error -I option: scale must be nonzero\n");
				add_operation (GMT, Ctrl, INTEGRATE, 1, par);
				break;
			case 'L':	/* Leave trend alone */
				Ctrl->L.active = TRUE;
				break;
#ifdef GMT_COMPAT
			case 'M':	/* Geographic data */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -M is deprecated; -fg was set instead, use this in the future.\n");
				if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
				break;
#endif
			case 'N':	/* Grid dimension setting or inquiery */
				Ctrl->N.active = TRUE;
				switch (opt->arg[0]) {
					case 'f': case 'F':
						Ctrl->N.force_narray = TRUE; break;
					case 'q': case 'Q':
						Ctrl->N.suggest_narray = TRUE; break;
					default:
						sscanf (opt->arg, "%" GMT_LL "d/%" GMT_LL "d", &Ctrl->N.nx2, &Ctrl->N.ny2);
						Ctrl->N.n_user_set = TRUE;
				}
				break;
			case 'S':	/* Scale */
				Ctrl->S.active = TRUE;
				Ctrl->S.scale = (opt->arg[0] == 'd' || opt->arg[0] == 'D') ? 1.0e6 : atof (opt->arg);
				break;
			case 'T':	/* Flexural isostasy */
				Ctrl->T.active = Ctrl->L.active = TRUE;
				n = sscanf (opt->arg, "%lf/%lf/%lf/%lf/%lf", &par[0], &par[1], &par[2], &par[3], &par[4]);
				for (j = 1, k = 0; j < 5; j++) if (par[j] < 0.0) k++;
				n_errors += GMT_check_condition (GMT, n != 5 || k > 0, "Syntax error -T option: Correct syntax:\n\t-T<te>/<rhol>/<rhom>/<rhow>/<rhoi>, all densities >= 0\n");
				add_operation (GMT, Ctrl, -1, 0, par);
				break;
#ifdef DEBUG
			case 'Q':	/* Do nothing */
				add_operation (GMT, Ctrl, -1, 1, par);
				break;
#endif
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
               }
	}

	n_errors += GMT_check_condition (GMT, !(Ctrl->n_op_count), "Syntax error: Must specify at least one operation\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.n_user_set && (Ctrl->N.nx2 <= 0 || Ctrl->N.ny2 <= 0), "Syntax error -N option: nx2 and/or ny2 <= 0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.scale == 0.0, "Syntax error -S option: scale must be nonzero\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->E.active && !Ctrl->G.file, "Syntax error -G option: Must specify output file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdfft_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdfft (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, stop, op_count = 0, par_count = 0, status;
	GMT_LONG narray[2], i, j, i_data_start, j_data_start, new_grid;

	struct GMT_GRID *Grid = NULL, *Out = NULL;
	struct F_INFO f_info;
	struct K_XY K;
	struct GRDFFT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdfft_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdfft_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdfft", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vf", "", options))) Return (error);
	Ctrl = (struct GRDFFT_CTRL *) New_grdfft_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdfft_parse (API, Ctrl, &f_info, options))) Return (error);

	/*---------------------------- This is the grdfft main code ----------------------------*/

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
	
	GMT_grd_init (GMT, Grid->header, options, TRUE);
	set_grid_radix_size (GMT, Ctrl, Grid);		/* This also sets the new pads */
	/* Because we taper and reflect below we DO NOT want any BCs set since that code expects 2 BC rows/cols */
	for (j = 0; j < 4; j++) Grid->header->BC[j] = GMT_BC_IS_DATA;

	/* Now read data into the real positions in the padded complex radix grid */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA | GMT_GRID_COMPLEX_REAL, (void **)&(Ctrl->In.file), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);	/* Get subset */
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */

	/* Check that no NaNs are present */
	for (j = stop = 0; !stop && j < Grid->header->size; j++) if (Grid->data[j] == 15.0) stop = j;
	
	i_data_start = GMT->current.io.pad[XLO];
	j_data_start = GMT->current.io.pad[YHI];
	stop = FALSE;
	for (j = 0; !stop && j < Grid->header->ny; j++) for (i = 0; !stop && i < Grid->header->nx; i++) stop = GMT_is_fnan (Grid->data[GMT_IJPR(Grid->header,j,i)]);
	if (stop) {
		GMT_report (GMT, GMT_MSG_FATAL, "Input grid cannot have NaNs!\n");
		Return (EXIT_FAILURE);
	}

	new_grid = GMT_set_outgrid (GMT, Grid, &Out);	/* TRUE if input is a read-only array; otherwise Out just points to Grid */

	if (!(Ctrl->L.active)) remove_plane (GMT, Out);
	if (!(Ctrl->N.force_narray)) taper_edges (GMT, Out);

	/* Load K_XY structure with wavenumbers and dimensions */
	K.delta_kx = 2 * M_PI / (Ctrl->N.nx2 * Out->header->inc[GMT_X]);
	K.delta_ky = 2 * M_PI / (Ctrl->N.ny2 * Out->header->inc[GMT_Y]);
	K.nx2 = Ctrl->N.nx2;	K.ny2 = Ctrl->N.ny2;
	
	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Give delta_kx, delta_ky units of 2pi/meters  */
		K.delta_kx /= (GMT->current.proj.DIST_M_PR_DEG * cosd (0.5 * (Out->header->wesn[YLO] + Out->header->wesn[YHI])) );
		K.delta_ky /= GMT->current.proj.DIST_M_PR_DEG;
	}

#ifdef FTEST
	{
		double f = 0.0;
		while (f < 3.0) {
			printf ("%g\t%g\n", f, f_info.filter (f, 0));	/* Radial filter */
			f += 0.01;
		}
		exit (-1);
	}
#endif
	GMT_report (GMT, GMT_MSG_NORMAL, "forward FFT...");
	
	narray[0] = Ctrl->N.nx2;	narray[1] = Ctrl->N.ny2;
	GMT_fft_2d (GMT, Out->data, Ctrl->N.nx2, Ctrl->N.ny2, GMT_FFT_FWD, GMT_FFT_COMPLEX);

	for (op_count = par_count = 0; op_count < Ctrl->n_op_count; op_count++) {
		switch (Ctrl->operation[op_count]) {
			case UP_DOWN_CONTINUE:
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) ((Ctrl->par[par_count] < 0.0) ? GMT_message (GMT, "downward continuation...") : GMT_message (GMT,  "upward continuation..."));
				par_count += do_continuation (Out, &Ctrl->par[par_count], &K);
				break;
			case AZIMUTHAL_DERIVATIVE:
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "azimuthal derivative...");
				par_count += do_azimuthal_derivative (Out, &Ctrl->par[par_count], &K);
				break;
			case DIFFERENTIATE:
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "differentiate...");
				par_count += do_differentiate (Out, &Ctrl->par[par_count], &K);
				break;
			case INTEGRATE:
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "integrate...");
				par_count += do_integrate (Out, &Ctrl->par[par_count], &K);
				break;
			case ISOSTASY:
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "isostasy...");
				par_count += do_isostasy (Out, Ctrl, &Ctrl->par[par_count], &K);
				break;
			case FILTER_COS:
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "cosine filter...");
				do_filter (Out, &f_info, &K);
				break;
			case FILTER_EXP:
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "Gaussian filter...");
				do_filter (Out, &f_info, &K);
				break;
			case FILTER_BW:
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "Butterworth filter...");
				do_filter (Out, &f_info, &K);
				break;
			case SPECTRUM:	/* This currently writes a table to file or stdout if -G is not used */
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "spectrum...");
				status = do_spectrum (GMT, Out, &Ctrl->par[par_count], Ctrl->E.give_wavelength, Ctrl->G.file, &K);
				if (status < 0) Return (status);
				par_count += status;
				break;
		}
	}

	if (!Ctrl->E.active) {	/* Since -E out was handled separately by do_spectrum */

		if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "inverse FFT...");

		GMT_fft_2d (GMT, Out->data, Ctrl->N.nx2, Ctrl->N.ny2, GMT_FFT_INV, GMT_FFT_COMPLEX);

		Ctrl->S.scale *= (2.0 / Out->header->size);
		for (i = 0; i < Out->header->size; i++) Out->data[i] *= (float)Ctrl->S.scale;

		/* The data are in the middle of the padded array */

		if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
		GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA | GMT_GRID_COMPLEX_REAL, (void **)&Ctrl->G.file, (void *)Out);
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);			/* Disables further data output */
	}

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "Done\n");

	Return (EXIT_SUCCESS);
}
