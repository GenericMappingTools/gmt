/*--------------------------------------------------------------------
 *	$Id: redpol_func.c,v 1.4 2011-04-12 13:06:44 remko Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * API functions to support the redpol application.
 *
 * Brief synopsis: redpol reads a grid file with the magnetic field anomaly and
 * computes the continuous reduction to the pole (RTP) anomaly by calculating the
 * filter coefficients in the frequency, inverse FT and convolve in space domain.
 * For details on the method see, Luis, J.F and J.M. Miranda (2008), 
 * "Reevaluation of magnetic chrons in the North Atlantic between 35°N and 47°N: 
 * Implications for the formation of the Azores Triple Junction and associated plateau, 
 * J. Geophys. Res., 113, B10105, doi:10.1029/2007JB005573 
 *
 * Author:	Joaquim Luis / Miguel Miranda
 * Date:	18-Feb-2003 (original GMT4 version)
 * Version:	5 API
 */

#include "gmt_potential.h"

struct REDPOL_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct C {	/* -C */
		GMT_LONG use_igrf;
		GMT_LONG const_f;
		double	dec;
		double	dip;
	} C;
	struct E {	/* -E */
		GMT_LONG active;
		GMT_LONG dip_grd_only;
		GMT_LONG dip_dec_grd;
		char *decfile;
		char *dipfile;
	} E;
	struct F {	/* -F */
		GMT_LONG active;
		GMT_LONG	ncoef_row;
		GMT_LONG	ncoef_col;
		GMT_LONG	compute_n;	/* Compute ncoef_col */
		double	width;
	} F;
	struct G {	/* -G<file> */
		GMT_LONG active;
		char	*file;
	} G;
	struct M {	/* -M */
		GMT_LONG pad_zero;
		GMT_LONG mirror;
	} M;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct S {	/* -S, size of working grid */
		GMT_LONG	nx;
		GMT_LONG	ny;
	} S;
	struct T {	/* -T */
		double	year;
	} T;
	struct W {	/* -W */
		double	wid;
	} W;
	struct Z {	/* -Z */
		GMT_LONG active;
		char	*file;
	} Z;
};


#define ij0_data(Ctrl,i,j) ((Ctrl->S.nx+Ctrl->F.ncoef_col-1)*(i)+(j))
#define ij_mn(Ctrl,i,j) (Ctrl->F.ncoef_row*(j)+(i))

void *New_redpol_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct REDPOL_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct REDPOL_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->C.use_igrf = TRUE;
	C->M.pad_zero = TRUE;
	C->N.active = TRUE;
	C->F.ncoef_row = 25;
	C->F.ncoef_col = 25;
	C->T.year = 2000;
	C->W.wid = 5;
	return ((void *)C);
}

void Free_redpol_Ctrl (struct GMT_CTRL *GMT, struct REDPOL_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->G.file) free ((void *)C->G.file);	
	if (C->E.dipfile) free ((void *)C->E.dipfile);	
	if (C->E.decfile) free ((void *)C->E.decfile);	
	if (C->Z.file) free ((void *)C->Z.file);	
	GMT_free (GMT, C);	
}

void rtp_filt_colinear(GMT_LONG i, GMT_LONG j, GMT_LONG n21, double *gxr,double *gxi, double *gxar, 
		double *gxai, double *gxbr, double *gxbi, double *gxgr, double *gxgi, double u, 
		double v, double alfa, double beta, double gama, struct REDPOL_CTRL *Ctrl) {

	GMT_LONG ij = ij_mn(Ctrl,i,j-n21+1);
	double ro, ro2, ro3, ro4, ro5, alfa_u, beta_v, gama_v, alfa_ro, alfa_ro_2, gama_ro, gama_ro_2;
	double alfa_u_beta_v, alfa_u_beta_v_2, rnr, rni, t2, t3;
	ro2 = u * u + v * v;    ro = sqrt(ro2);     ro3 = ro2 * ro;
	ro4 = ro2 * ro2;	ro5 = ro3 * ro2;
	alfa_u = alfa * u;      gama_v = gama * v;
	beta_v = beta * v;
	alfa_ro = alfa * ro;    alfa_ro_2 = alfa_ro * alfa_ro;
	gama_ro = gama * ro;    gama_ro_2 = gama_ro * gama_ro;
	alfa_u_beta_v = alfa_u + beta_v;
	alfa_u_beta_v_2 = alfa_u_beta_v * alfa_u_beta_v;

	t2 = 1 / ((alfa_u_beta_v_2 + gama_ro_2) * (alfa_u_beta_v_2 + gama_ro_2));
	t3 = t2 / (alfa_u_beta_v_2 + gama_ro_2);
          					 
	rnr = (gama_ro_2 - alfa_u_beta_v_2) * ro2;
	rni = 2 * gama_ro * alfa_u_beta_v * ro2;

	gxr[ij] = rnr * t2;
	gxi[ij] = rni * t2;

	if (Ctrl->N.active) {		/* Use the Taylor expansion */
		gxar[ij] = -2*alfa_u_beta_v*u*ro2 * t2 - 4*(gama_ro_2-alfa_u_beta_v_2)*ro2*alfa_u_beta_v*u * t3;
		gxai[ij] = 2*gama*ro3*u * t2 - 8*gama*ro3*alfa_u_beta_v_2*u * t3;
		gxbr[ij] = -2*alfa_u_beta_v*v*ro2 * t2 - 4*(gama_ro_2-alfa_u_beta_v_2)*ro2*alfa_u_beta_v*v * t3;
		gxbi[ij] = 2*gama*ro3*v * t2 - 8*gama*ro3*alfa_u_beta_v_2*v * t3;
		gxgr[ij] = 2*gama*ro4 * t2 - 4*(gama_ro_2-alfa_u_beta_v_2)*ro4*gama * t3;
		gxgi[ij] = 2*ro3*alfa_u_beta_v * t2 - 8*gama*gama*ro5*alfa_u_beta_v * t3;
	}
}


void rtp_filt_NOTcolinear(GMT_LONG i, GMT_LONG j, GMT_LONG n21, double *gxr, double *gxi, double *gxar, 
		double *gxai, double *gxbr, double *gxbi, double *gxgr, double *gxgi, double *gxtr, 
		double *gxti, double *gxmr, double *gxmi, double *gxnr, double *gxni, double u, double v, double alfa, 
		double beta, double gama, double tau, double mu, double nu, struct REDPOL_CTRL *Ctrl) {

	GMT_LONG ij = ij_mn(Ctrl,i,j-n21+1);
	double ro, ro2, ro3, ro4, ro5, alfa_u, beta_v, gama_v, alfa_ro, alfa_ro_2, gama_ro, gama_ro_2;
	double alfa_u_beta_v, alfa_u_beta_v_2, rnr, rni, den_r, den_i1, den_i2;
	double tau_u, mu_v, nu_ro, nu_ro_2, tau_u_mu_v, tau_u_mu_v_2, tau_u_mu_v_gama;
	double alfa_u_beta_v_nu, alfa_u_beta_v_tau_u_mu_v;

	ro2 = u * u + v * v;    ro = sqrt(ro2);     ro3 = ro2 * ro;
	ro4 = ro2 * ro2;	ro5 = ro3 * ro2;
	alfa_u = alfa * u;      gama_v = gama * v;
	beta_v = beta * v;
	alfa_ro = alfa * ro;    alfa_ro_2 = alfa_ro * alfa_ro;
	gama_ro = gama * ro;    gama_ro_2 = gama_ro * gama_ro;
	alfa_u_beta_v = alfa_u + beta_v;
	alfa_u_beta_v_2 = alfa_u_beta_v * alfa_u_beta_v;

	tau_u = tau * u;	mu_v = mu * v;
	nu_ro = nu * ro;	nu_ro_2 = nu_ro * nu_ro;
	tau_u_mu_v = tau_u + mu_v;
	tau_u_mu_v_2 = tau_u_mu_v * tau_u_mu_v;

	tau_u_mu_v_gama = tau_u_mu_v * gama;
	alfa_u_beta_v_nu = alfa_u_beta_v * nu;
	alfa_u_beta_v_tau_u_mu_v = alfa_u_beta_v * tau_u_mu_v;

	rnr = (-alfa_u_beta_v_tau_u_mu_v + gama_ro * nu_ro) * ro2;
	rni = (alfa_u_beta_v_nu + tau_u_mu_v_gama) * ro3;

	den_r = (alfa_u_beta_v_2 + gama_ro_2)*(tau_u_mu_v_2 + nu_ro_2);
	den_i1 = den_r * (alfa_u_beta_v_2+gama_ro_2);
	den_i2 = den_r * (tau_u_mu_v_2 + nu_ro_2);

	gxr[ij] = rnr / den_r;
	gxi[ij] = rni / den_r;

	if (Ctrl->N.active) {		/* Use the Taylor expansion */
		gxar[ij] = -u*tau_u_mu_v*ro2/den_r -2*(-alfa_u_beta_v_tau_u_mu_v+gama*ro2*nu)*ro2*alfa_u_beta_v*u/den_i1;
		gxai[ij] = u*nu*ro3/den_r -2*(alfa_u_beta_v_nu+tau_u_mu_v_gama)*ro3*alfa_u_beta_v*u/den_i1;
		gxbr[ij] = -v*tau_u_mu_v*ro2/den_r -2*(-alfa_u_beta_v_tau_u_mu_v+gama*ro2*nu)*ro2*alfa_u_beta_v*v/den_i1;
		gxbi[ij] = v*nu*ro3/den_r -2*(alfa_u_beta_v_nu+tau_u_mu_v_gama)*ro3*alfa_u_beta_v*v/den_i1;
		gxgr[ij] = nu*ro4/den_r -2*(-alfa_u_beta_v_tau_u_mu_v+gama*ro2*nu)*ro4*gama/den_i1;
		gxgi[ij] = tau_u_mu_v*ro3/den_r -2*(alfa_u_beta_v_nu+tau_u_mu_v_gama)*ro5*gama/den_i1;
		gxtr[ij] = -alfa_u_beta_v*u*ro2/den_r -2*(-alfa_u_beta_v_tau_u_mu_v+gama*ro2*nu)*ro2*u*tau_u_mu_v/den_i2;
		gxti[ij] = u*gama*ro3/den_r -2*(alfa_u_beta_v_nu+tau_u_mu_v_gama)*ro3*u*tau_u_mu_v/den_i2;
		gxmr[ij] = -alfa_u_beta_v*v*ro2/den_r -2*(-alfa_u_beta_v_tau_u_mu_v+gama*ro2*nu)*ro2*v*tau_u_mu_v/den_i2;
		gxmi[ij] = v*gama*ro3/den_r -2*(alfa_u_beta_v_nu+tau_u_mu_v_gama)*ro3*v*tau_u_mu_v/den_i2;
		gxnr[ij] = gama*ro4/den_r -2*(-alfa_u_beta_v_tau_u_mu_v+gama*ro2*nu)*ro4*nu/den_i2;
		gxni[ij] = alfa_u_beta_v*ro3/den_r -2*(alfa_u_beta_v_nu+tau_u_mu_v_gama)*ro5*nu/den_i2;
	}
}

void mirror_edges (float *grid, GMT_LONG nc, GMT_LONG i_data_start, GMT_LONG j_data_start, struct REDPOL_CTRL *Ctrl) {
	/* This routine mirrors or replicates the West and East borders j_data_start times
	   and the South and North borders by i_data_start times.
	   nc	is the total number of columns by which the grid is extended
	   Ctrl->S.nx & Ctrl->S.ny are the grid's original number of column/rows before extension */
	GMT_LONG	i, j, ins, isn, iss, jww, jwe, jee, jew;

	/* First reflect about xmin and xmax, point symmetric about edge point */

	for (j = 1; j <= j_data_start; j++) {	/* COLUMNS */
		jww = j_data_start-j;		/* Minimum Outside xmin and aproaching West border  */
		jee = Ctrl->S.nx + j_data_start + j-1;	/* Minimum Outside xmax and aproching East border  */
		if (Ctrl->M.mirror) {
			jwe = j_data_start+j;			/* Minimum Inside xmin and aproaching center  */
			jew = Ctrl->S.nx + j_data_start - j-1;	/* Minimum Inside xmax and aproching center  */
		}
		else {
			jwe = j_data_start;			/* West border */
			jew = Ctrl->S.nx + j_data_start - 1;	/* East border */
		}
		for (i = i_data_start; i < Ctrl->S.ny+i_data_start; i++) {	/* ROWS */
			grid[ij0_data(Ctrl,i,jww)] = grid[ij0_data(Ctrl,i,jwe)];	/* West border */
			grid[ij0_data(Ctrl,i,jee)] = grid[ij0_data(Ctrl,i,jew)];	/* East border */
		}
	}

	/* Next, reflect about ymin and ymax. At the same time, since x has been reflected, we can use these vals */

	for (i = 0; i < i_data_start; i++) {	/* ROWS */
		iss = Ctrl->S.ny+i_data_start+i;	/* Minimum Outside ymin and aproaching South border  */
		if (Ctrl->M.mirror) {
			ins = 2*i_data_start - i;		/* Maximum Inside ymax and aproaching North border */
			isn = Ctrl->S.ny+i_data_start-2-i;	/* Minimum Inside ymin and aproaching center  */
		}
		else {
			ins = i_data_start;			/* North border */
			isn = Ctrl->S.ny+i_data_start-1;	/* South border */
		}
		for (j = 0; j < (Ctrl->S.nx + nc); j++) {	/* COLUMNS */
			grid[ij0_data(Ctrl,i,j)] = grid[ij0_data(Ctrl,ins,j)];		/* North border */
			grid[ij0_data(Ctrl,iss,j)] = grid[ij0_data(Ctrl,isn,j)];	/* South border */
		}
	}
}

void tfpoeq(double *w, GMT_LONG m, GMT_LONG n, double *greel, double *gim, 
	    double *cosphi, double *sinphi, double *cospsi, double *sinpsi) {
    /* Initialized data */

    static GMT_LONG mkeep = -9999;
    static GMT_LONG nkeep = -9999;

    /* System generated locals */
    GMT_LONG w_offset, greel_offset, gim_offset;
    GMT_LONG i, k, l, k1, k2, m1, n1, l1, l2, ir, is, lr, ks, ky, lx, ir1, ir2, lx1, lrm, ksn;
    static double co1, co2, si2, si1, c1c2, c1s2, arg, c2s1, s1s2, xmn, arg1, somi, somr;

/*     THIS SUBROUTINE COMPUTES THE INVERSE FOURIER TRANSFORM OF A FILTER */
/*     WHOSE RESULTANT INDEX IS 2 */

/*     INPUT ARGUMENTS DESCRIPTION: */
/*          - GREL => REAL PART OF THE FOURIER TRANSFORM OF THE FILTER */
/*          - GIM => IMAGINARY PART OF THE FOURIER TRANSFORM */
/*          - M/N => NUMBER OF LINES/COLUMN OF THE FILTER */

/*     OUTPUT ARGUMENTS DESCRIPTION: */
/*          - W => AN ARRAY WHICH CONTAINS THE FILTER COEFICIENTS STORED */
/*               COLUMNWISE */

/*     REMARKS: */
/*       M AND N MUST BE ODD */
/*       GREL AND GIM MUST CONTAIN AT LEAST M*(N+1)/2 REAL*8 LOCATIONS */

/*	Translated to C by f2c (and further massaged) from a routine of A. Galdeano */

    /* Parameter adjustments */
    gim_offset = 1 + m;
    gim -= gim_offset;
    greel_offset = 1 + m;
    greel -= greel_offset;
    w_offset = 1 + m;
    w -= w_offset;

    /* Function Body */
    ky = (n + 1) / 2;
    lx = (m + 1) / 2;
/*  CALCUL DES COEF. DU FILTRE */
    xmn = (double)(m * n);
    if (m == mkeep) goto L2;
    mkeep = m;
    arg = TWO_PI / m;
    for (i = 0; i < m; i++) {
	arg1 = arg * i;
	sinphi[i] = sin(arg1);
	cosphi[i] = cos(arg1);
    }
L2:
    if (n == nkeep) goto L4;
    nkeep = n;
    arg = TWO_PI / n;
    for (i = 0; i < n; i++) {
	arg1 = arg * i;
	sinpsi[i] = sin(arg1);
	cospsi[i] = cos(arg1);
    }
L4:
    lx1 = lx + 1;
    m1 = m + 1;
    n1 = n + 1;
    for (k1 = 1; k1 <= n; ++k1) {
	k = k1 - ky;
	k2 = n1 - k1;
	for (l1 = lx; l1 <= m; ++l1) {
	    l = l1 - lx;
	    l2 = m1 - l1;
	    somr = 0.;
	    somi = 0.;
	    for (ir = lx1; ir <= m; ++ir) {
		lr = ir - lx;
		lrm = lr * l % m + 1;
		somr += greel[ir + m] * cosphi[lrm - 1];
		somi += gim[ir + m] * sinphi[lrm - 1];
	    }
	    for (is = 2; is <= ky; ++is) {
		ks = is - 1;
		ksn = ks * k % n + 1;
		if (ksn <= 0) ksn += n;
		co2 = cospsi[ksn - 1];
		si2 = sinpsi[ksn - 1];
		somr += greel[lx + is * m] * co2;
		somi += gim[lx + is * m] * si2;
		for (ir1 = lx1; ir1 <= m; ++ir1) {
		    lr = ir1 - lx;
		    ir2 = lx - lr;
		    lrm = lr * l % m + 1;
		    co1 = cosphi[lrm - 1];
		    si1 = sinphi[lrm - 1];
		    c1c2 = co1 * co2;
		    s1s2 = si1 * si2;
		    c1s2 = co1 * si2;
		    c2s1 = co2 * si1;
		    somr = somr + greel[ir1 + is * m] * (c1c2 - s1s2) + greel[ir2 + is * m] * (c1c2 + s1s2);
		    somi = somi + gim[ir1 + is * m] * (c1s2 + c2s1) + gim[ir2 + is * m] * (c1s2 - c2s1);
		}
	    }
	    somr = somr + somr + greel[lx + m];
	    somi += somi;
	    w[l1 + k1 * m] = (somr + somi) / xmn;
	    if (l1 != lx)
		w[l2 + k2 * m] = (somr - somi) / xmn;
	}
    }
}


int igrf10syn (struct GMT_CTRL *C, int isv, double date, int itype, double alt, double elong, double lat, double *out) {
 /*     This is a synthesis routine for the 10th generation IGRF as agreed
  *     in December 2004 by IAGA Working Group V-MOD. It is valid 1900.0 to
  *     2010.0 inclusive. Values for dates from 1945.0 to 2000.0 inclusive are
  *     definitve, otherwise they are non-definitive.
  *   INPUT
  *     isv   = 0 if main-field values are required
  *     isv   = 1 if secular variation values are required
  *     date  = year A.D. Must be greater than or equal to 1900.0 and
  *             less than or equal to 2015.0. Warning message is given
  *             for dates greater than 2010.0. Must be double precision.
  *     itype = 1 if geodetic (spheroid)
  *     itype = 2 if geocentric (sphere)
  *     alt   = height in km above sea level if itype = 1
  *           = distance from centre of Earth in km if itype = 2 (>3485 km)
  *     lat   = latitude (90-90)
  *     elong = east-longitude (0-360) -- it works also in [-180;+180]
  *   OUTPUT
  *     out[0] F  = total intensity (nT) if isv = 0, rubbish if isv = 1
  *     out[1] H  = horizontal intensity (nT)
  *     out[2] X  = north component (nT) if isv = 0, nT/year if isv = 1
  *     out[3] Y  = east component (nT) if isv = 0, nT/year if isv = 1
  *     out[4] Z  = vertical component (nT) if isv = 0, nT/year if isv = 1
  *     out[5] D  = declination
  *     out[6] I  = inclination
  *
  *     To get the other geomagnetic elements (D, I, H and secular
  *     variations dD, dH, dI and dF) use routines ptoc and ptocsv.
  *
  *     Adapted from 8th generation version to include new maximum degree for
  *     main-field models for 2000.0 and onwards and use WGS84 spheroid instead
  *     of International Astronomical Union 1966 spheroid as recommended by IAGA
  *     in July 2003. Reference radius remains as 6371.2 km - it is NOT the mean
  *     radius (= 6371.0 km) but 6371.2 km is what is used in determining the
  *     coefficients. Adaptation by Susan Macmillan, August 2003 (for
  *     9th generation) and December 2004.
  *
  *	Joaquim Luis 1-MARS-2005
  *	Converted to C (with help of f2c, which explains the ugliness)
  *     1995.0 coefficients as published in igrf9coeffs.xls and igrf10coeffs.xls 
  *     used - (Kimmo Korhonen spotted 1 nT difference in 11 coefficients) 
  *     Susan Macmillan July 2005 (PW update Oct 2006) 
  *
  *	Joaquim Luis 21-JAN-2010
  *	Updated for IGRF 11th generation
  */

     struct IGRF {
	double e_1[3255];
     };
     /* Initialized data */
     static struct IGRF equiv_22 = {
       {-31543.,-2298., 5922., -677., 2905.,-1061.,  924., 1121., /* g0 (1900) */
         1022.,-1469., -330., 1256.,    3.,  572.,  523.,  876.,
          628.,  195.,  660.,  -69., -361., -210.,  134.,  -75.,
         -184.,  328., -210.,  264.,   53.,    5.,  -33.,  -86.,
         -124.,  -16.,    3.,   63.,   61.,   -9.,  -11.,   83.,
         -217.,    2.,  -58.,  -35.,   59.,   36.,  -90.,  -69.,
           70.,  -55.,  -45.,    0.,  -13.,   34.,  -10.,  -41.,
           -1.,  -21.,   28.,   18.,  -12.,    6.,  -22.,   11.,
            8.,    8.,   -4.,  -14.,   -9.,    7.,    1.,  -13.,
            2.,    5.,   -9.,   16.,    5.,   -5.,    8.,  -18.,
            8.,   10.,  -20.,    1.,   14.,  -11.,    5.,   12.,
           -3.,    1.,   -2.,   -2.,    8.,    2.,   10.,   -1.,
           -2.,   -1.,    2.,   -3.,   -4.,    2.,    2.,    1.,
           -5.,    2.,   -2.,    6.,    6.,   -4.,    4.,    0.,
            0.,   -2.,    2.,    4.,    2.,    0.,    0.,   -6.,
       -31464.,-2298., 5909., -728., 2928.,-1086., 1041., 1065., /* g1 (1905) */
         1037.,-1494., -357., 1239.,   34.,  635.,  480.,  880.,
          643.,  203.,  653.,  -77., -380., -201.,  146.,  -65.,
         -192.,  328., -193.,  259.,   56.,   -1.,  -32.,  -93.,
         -125.,  -26.,   11.,   62.,   60.,   -7.,  -11.,   86.,
         -221.,    4.,  -57.,  -32.,   57.,   32.,  -92.,  -67.,
           70.,  -54.,  -46.,    0.,  -14.,   33.,  -11.,  -41.,
            0.,  -20.,   28.,   18.,  -12.,    6.,  -22.,   11.,
            8.,    8.,   -4.,  -15.,   -9.,    7.,    1.,  -13.,
            2.,    5.,   -8.,   16.,    5.,   -5.,    8.,  -18.,
            8.,   10.,  -20.,    1.,   14.,  -11.,    5.,   12.,
           -3.,    1.,   -2.,   -2.,    8.,    2.,   10.,    0.,
           -2.,   -1.,    2.,   -3.,   -4.,    2.,    2.,    1.,
           -5.,    2.,   -2.,    6.,    6.,   -4.,    4.,    0.,
            0.,   -2.,    2.,    4.,    2.,    0.,    0.,   -6.,
       -31354.,-2297., 5898., -769., 2948.,-1128., 1176., 1000., /* g2 (1910) */
         1058.,-1524., -389., 1223.,   62.,  705.,  425.,  884.,
          660.,  211.,  644.,  -90., -400., -189.,  160.,  -55.,
         -201.,  327., -172.,  253.,   57.,   -9.,  -33., -102.,
         -126.,  -38.,   21.,   62.,   58.,   -5.,  -11.,   89.,
         -224.,    5.,  -54.,  -29.,   54.,   28.,  -95.,  -65.,
           71.,  -54.,  -47.,    1.,  -14.,   32.,  -12.,  -40.,
            1.,  -19.,   28.,   18.,  -13.,    6.,  -22.,   11.,
            8.,    8.,   -4.,  -15.,   -9.,    6.,    1.,  -13.,
            2.,    5.,   -8.,   16.,    5.,   -5.,    8.,  -18.,
            8.,   10.,  -20.,    1.,   14.,  -11.,    5.,   12.,
           -3.,    1.,   -2.,   -2.,    8.,    2.,   10.,    0.,
           -2.,   -1.,    2.,   -3.,   -4.,    2.,    2.,    1.,
           -5.,    2.,   -2.,    6.,    6.,   -4.,    4.,    0.,
            0.,   -2.,    2.,    4.,    2.,    0.,    0.,   -6.,
       -31212.,-2306., 5875., -802., 2956.,-1191., 1309.,  917., /* g3 (1915) */
         1084.,-1559., -421., 1212.,   84.,  778.,  360.,  887.,
          678.,  218.,  631., -109., -416., -173.,  178.,  -51.,
         -211.,  327., -148.,  245.,   58.,  -16.,  -34., -111.,
         -126.,  -51.,   32.,   61.,   57.,   -2.,  -10.,   93.,
         -228.,    8.,  -51.,  -26.,   49.,   23.,  -98.,  -62.,
           72.,  -54.,  -48.,    2.,  -14.,   31.,  -12.,  -38.,
            2.,  -18.,   28.,   19.,  -15.,    6.,  -22.,   11.,
            8.,    8.,   -4.,  -15.,   -9.,    6.,    2.,  -13.,
            3.,    5.,   -8.,   16.,    6.,   -5.,    8.,  -18.,
            8.,   10.,  -20.,    1.,   14.,  -11.,    5.,   12.,
           -3.,    1.,   -2.,   -2.,    8.,    2.,   10.,    0.,
           -2.,   -1.,    2.,   -3.,   -4.,    2.,    2.,    1.,
           -5.,    2.,   -2.,    6.,    6.,   -4.,    4.,    0.,
            0.,   -2.,    1.,    4.,    2.,    0.,    0.,   -6.,
       -31060.,-2317., 5845., -839., 2959.,-1259., 1407.,  823., /* g4 (1920) */
         1111.,-1600., -445., 1205.,  103.,  839.,  293.,  889.,
          695.,  220.,  616., -134., -424., -153.,  199.,  -57.,
         -221.,  326., -122.,  236.,   58.,  -23.,  -38., -119.,
         -125.,  -62.,   43.,   61.,   55.,    0.,  -10.,   96.,
         -233.,   11.,  -46.,  -22.,   44.,   18., -101.,  -57.,
           73.,  -54.,  -49.,    2.,  -14.,   29.,  -13.,  -37.,
            4.,  -16.,   28.,   19.,  -16.,    6.,  -22.,   11.,
            7.,    8.,   -3.,  -15.,   -9.,    6.,    2.,  -14.,
            4.,    5.,   -7.,   17.,    6.,   -5.,    8.,  -19.,
            8.,   10.,  -20.,    1.,   14.,  -11.,    5.,   12.,
           -3.,    1.,   -2.,   -2.,    9.,    2.,   10.,    0.,
           -2.,   -1.,    2.,   -3.,   -4.,    2.,    2.,    1.,
           -5.,    2.,   -2.,    6.,    6.,   -4.,    4.,    0.,
            0.,   -2.,    1.,    4.,    3.,    0.,    0.,   -6.,
       -30926.,-2318., 5817., -893., 2969.,-1334., 1471.,  728., /* g5 (1925) */
         1140.,-1645., -462., 1202.,  119.,  881.,  229.,  891.,
          711.,  216.,  601., -163., -426., -130.,  217.,  -70.,
         -230.,  326.,  -96.,  226.,   58.,  -28.,  -44., -125.,
         -122.,  -69.,   51.,   61.,   54.,    3.,   -9.,   99.,
         -238.,   14.,  -40.,  -18.,   39.,   13., -103.,  -52.,
           73.,  -54.,  -50.,    3.,  -14.,   27.,  -14.,  -35.,
            5.,  -14.,   29.,   19.,  -17.,    6.,  -21.,   11.,
            7.,    8.,   -3.,  -15.,   -9.,    6.,    2.,  -14.,
            4.,    5.,   -7.,   17.,    7.,   -5.,    8.,  -19.,
            8.,   10.,  -20.,    1.,   14.,  -11.,    5.,   12.,
           -3.,    1.,   -2.,   -2.,    9.,    2.,   10.,    0.,
           -2.,   -1.,    2.,   -3.,   -4.,    2.,    2.,    1.,
           -5.,    2.,   -2.,    6.,    6.,   -4.,    4.,    0.,
            0.,   -2.,    1.,    4.,    3.,    0.,    0.,   -6.,
       -30805.,-2316., 5808., -951., 2980.,-1424., 1517.,  644., /* g6 (1930) */
         1172.,-1692., -480., 1205.,  133.,  907.,  166.,  896.,
          727.,  205.,  584., -195., -422., -109.,  234.,  -90.,
         -237.,  327.,  -72.,  218.,   60.,  -32.,  -53., -131.,
         -118.,  -74.,   58.,   60.,   53.,    4.,   -9.,  102.,
         -242.,   19.,  -32.,  -16.,   32.,    8., -104.,  -46.,
           74.,  -54.,  -51.,    4.,  -15.,   25.,  -14.,  -34.,
            6.,  -12.,   29.,   18.,  -18.,    6.,  -20.,   11.,
            7.,    8.,   -3.,  -15.,   -9.,    5.,    2.,  -14.,
            5.,    5.,   -6.,   18.,    8.,   -5.,    8.,  -19.,
            8.,   10.,  -20.,    1.,   14.,  -12.,    5.,   12.,
           -3.,    1.,   -2.,   -2.,    9.,    3.,   10.,    0.,
           -2.,   -2.,    2.,   -3.,   -4.,    2.,    2.,    1.,
           -5.,    2.,   -2.,    6.,    6.,   -4.,    4.,    0.,
            0.,   -2.,    1.,    4.,    3.,    0.,    0.,   -6.,
       -30715.,-2306., 5812.,-1018., 2984.,-1520., 1550.,  586., /* g7 (1935) */
         1206.,-1740., -494., 1215.,  146.,  918.,  101.,  903.,
          744.,  188.,  565., -226., -415.,  -90.,  249., -114.,
         -241.,  329.,  -51.,  211.,   64.,  -33.,  -64., -136.,
         -115.,  -76.,   64.,   59.,   53.,    4.,   -8.,  104.,
         -246.,   25.,  -25.,  -15.,   25.,    4., -106.,  -40.,
           74.,  -53.,  -52.,    4.,  -17.,   23.,  -14.,  -33.,
            7.,  -11.,   29.,   18.,  -19.,    6.,  -19.,   11.,
            7.,    8.,   -3.,  -15.,   -9.,    5.,    1.,  -15.,
            6.,    5.,   -6.,   18.,    8.,   -5.,    7.,  -19.,
            8.,   10.,  -20.,    1.,   15.,  -12.,    5.,   11.,
           -3.,    1.,   -3.,   -2.,    9.,    3.,   11.,    0.,
           -2.,   -2.,    2.,   -3.,   -4.,    2.,    2.,    1.,
           -5.,    2.,   -2.,    6.,    6.,   -4.,    4.,    0.,
            0.,   -1.,    2.,    4.,    3.,    0.,    0.,   -6.,
       -30654.,-2292., 5821.,-1106., 2981.,-1614., 1566.,  528., /* g8 (1940) */
         1240.,-1790., -499., 1232.,  163.,  916.,   43.,  914.,
          762.,  169.,  550., -252., -405.,  -72.,  265., -141.,
         -241.,  334.,  -33.,  208.,   71.,  -33.,  -75., -141.,
         -113.,  -76.,   69.,   57.,   54.,    4.,   -7.,  105.,
         -249.,   33.,  -18.,  -15.,   18.,    0., -107.,  -33.,
           74.,  -53.,  -52.,    4.,  -18.,   20.,  -14.,  -31.,
            7.,   -9.,   29.,   17.,  -20.,    5.,  -19.,   11.,
            7.,    8.,   -3.,  -14.,  -10.,    5.,    1.,  -15.,
            6.,    5.,   -5.,   19.,    9.,   -5.,    7.,  -19.,
            8.,   10.,  -21.,    1.,   15.,  -12.,    5.,   11.,
           -3.,    1.,   -3.,   -2.,    9.,    3.,   11.,    1.,
           -2.,   -2.,    2.,   -3.,   -4.,    2.,    2.,    1.,
           -5.,    2.,   -2.,    6.,    6.,   -4.,    4.,    0.,
            0.,   -1.,    2.,    4.,    3.,    0.,    0.,   -6.,
       -30594.,-2285., 5810.,-1244., 2990.,-1702., 1578.,  477., /* g9 (1945) */
         1282.,-1834., -499., 1255.,  186.,  913.,  -11.,  944.,
          776.,  144.,  544., -276., -421.,  -55.,  304., -178.,
         -253.,  346.,  -12.,  194.,   95.,  -20.,  -67., -142.,
         -119.,  -82.,   82.,   59.,   57.,    6.,    6.,  100.,
         -246.,   16.,  -25.,   -9.,   21.,  -16., -104.,  -39.,
           70.,  -40.,  -45.,    0.,  -18.,    0.,    2.,  -29.,
            6.,  -10.,   28.,   15.,  -17.,   29.,  -22.,   13.,
            7.,   12.,   -8.,  -21.,   -5.,  -12.,    9.,   -7.,
            7.,    2.,  -10.,   18.,    7.,    3.,    2.,  -11.,
            5.,  -21.,  -27.,    1.,   17.,  -11.,   29.,    3.,
           -9.,   16.,    4.,   -3.,    9.,   -4.,    6.,   -3.,
            1.,   -4.,    8.,   -3.,   11.,    5.,    1.,    1.,
            2.,  -20.,   -5.,   -1.,   -1.,   -6.,    8.,    6.,
           -1.,   -4.,   -3.,   -2.,    5.,    0.,   -2.,   -2.,
       -30554.,-2250., 5815.,-1341., 2998.,-1810., 1576.,  381., /* ga (1950) */
         1297.,-1889., -476., 1274.,  206.,  896.,  -46.,  954.,
          792.,  136.,  528., -278., -408.,  -37.,  303., -210.,
         -240.,  349.,    3.,  211.,  103.,  -20.,  -87., -147.,
         -122.,  -76.,   80.,   54.,   57.,   -1.,    4.,   99.,
         -247.,   33.,  -16.,  -12.,   12.,  -12., -105.,  -30.,
           65.,  -55.,  -35.,    2.,  -17.,    1.,    0.,  -40.,
           10.,   -7.,   36.,    5.,  -18.,   19.,  -16.,   22.,
           15.,    5.,   -4.,  -22.,   -1.,    0.,   11.,  -21.,
           15.,   -8.,  -13.,   17.,    5.,   -4.,   -1.,  -17.,
            3.,   -7.,  -24.,   -1.,   19.,  -25.,   12.,   10.,
            2.,    5.,    2.,   -5.,    8.,   -2.,    8.,    3.,
          -11.,    8.,   -7.,   -8.,    4.,   13.,   -1.,   -2.,
           13.,  -10.,   -4.,    2.,    4.,   -3.,   12.,    6.,
            3.,   -3.,    2.,    6.,   10.,   11.,    3.,    8.,
       -30500.,-2215., 5820.,-1440., 3003.,-1898., 1581.,  291., /* gb (1955) */
         1302.,-1944., -462., 1288.,  216.,  882.,  -83.,  958.,
          796.,  133.,  510., -274., -397.,  -23.,  290., -230.,
         -229.,  360.,   15.,  230.,  110.,  -23.,  -98., -152.,
         -121.,  -69.,   78.,   47.,   57.,   -9.,    3.,   96.,
         -247.,   48.,   -8.,  -16.,    7.,  -12., -107.,  -24.,
           65.,  -56.,  -50.,    2.,  -24.,   10.,   -4.,  -32.,
            8.,  -11.,   28.,    9.,  -20.,   18.,  -18.,   11.,
            9.,   10.,   -6.,  -15.,  -14.,    5.,    6.,  -23.,
           10.,    3.,   -7.,   23.,    6.,   -4.,    9.,  -13.,
            4.,    9.,  -11.,   -4.,   12.,   -5.,    7.,    2.,
            6.,    4.,   -2.,    1.,   10.,    2.,    7.,    2.,
           -6.,    5.,    5.,   -3.,   -5.,   -4.,   -1.,    0.,
            2.,   -8.,   -3.,   -2.,    7.,   -4.,    4.,    1.,
           -2.,   -3.,    6.,    7.,   -2.,   -1.,    0.,   -3.,
       -30421.,-2169., 5791.,-1555., 3002.,-1967., 1590.,  206., /* gc (1960) */
         1302.,-1992., -414., 1289.,  224.,  878., -130.,  957.,
          800.,  135.,  504., -278., -394.,    3.,  269., -255.,
         -222.,  362.,   16.,  242.,  125.,  -26., -117., -156.,
         -114.,  -63.,   81.,   46.,   58.,  -10.,    1.,   99.,
         -237.,   60.,   -1.,  -20.,   -2.,  -11., -113.,  -17.,
           67.,  -56.,  -55.,    5.,  -28.,   15.,   -6.,  -32.,
            7.,   -7.,   23.,   17.,  -18.,    8.,  -17.,   15.,
            6.,   11.,   -4.,  -14.,  -11.,    7.,    2.,  -18.,
           10.,    4.,   -5.,   23.,   10.,    1.,    8.,  -20.,
            4.,    6.,  -18.,    0.,   12.,   -9.,    2.,    1.,
            0.,    4.,   -3.,   -1.,    9.,   -2.,    8.,    3.,
            0.,   -1.,    5.,    1.,   -3.,    4.,    4.,    1.,
            0.,    0.,   -1.,    2.,    4.,   -5.,    6.,    1.,
            1.,   -1.,   -1.,    6.,    2.,    0.,    0.,   -7.,
       -30334.,-2119., 5776.,-1662., 2997.,-2016., 1594.,  114., /* gd (1965) */
         1297.,-2038., -404., 1292.,  240.,  856., -165.,  957.,
          804.,  148.,  479., -269., -390.,   13.,  252., -269.,
         -219.,  358.,   19.,  254.,  128.,  -31., -126., -157.,
          -97.,  -62.,   81.,   45.,   61.,  -11.,    8.,  100.,
         -228.,   68.,    4.,  -32.,    1.,   -8., -111.,   -7.,
           75.,  -57.,  -61.,    4.,  -27.,   13.,   -2.,  -26.,
            6.,   -6.,   26.,   13.,  -23.,    1.,  -12.,   13.,
            5.,    7.,   -4.,  -12.,  -14.,    9.,    0.,  -16.,
            8.,    4.,   -1.,   24.,   11.,   -3.,    4.,  -17.,
            8.,   10.,  -22.,    2.,   15.,  -13.,    7.,   10.,
           -4.,   -1.,   -5.,   -1.,   10.,    5.,   10.,    1.,
           -4.,   -2.,    1.,   -2.,   -3.,    2.,    2.,    1.,
           -5.,    2.,   -2.,    6.,    4.,   -4.,    4.,    0.,
            0.,   -2.,    2.,    3.,    2.,    0.,    0.,   -6.,
       -30220.,-2068., 5737.,-1781., 3000.,-2047., 1611.,   25., /* ge (1970) */
         1287.,-2091., -366., 1278.,  251.,  838., -196.,  952.,
          800.,  167.,  461., -266., -395.,   26.,  234., -279.,
         -216.,  359.,   26.,  262.,  139.,  -42., -139., -160.,
          -91.,  -56.,   83.,   43.,   64.,  -12.,   15.,  100.,
         -212.,   72.,    2.,  -37.,    3.,   -6., -112.,    1.,
           72.,  -57.,  -70.,    1.,  -27.,   14.,   -4.,  -22.,
            8.,   -2.,   23.,   13.,  -23.,   -2.,  -11.,   14.,
            6.,    7.,   -2.,  -15.,  -13.,    6.,   -3.,  -17.,
            5.,    6.,    0.,   21.,   11.,   -6.,    3.,  -16.,
            8.,   10.,  -21.,    2.,   16.,  -12.,    6.,   10.,
           -4.,   -1.,   -5.,    0.,   10.,    3.,   11.,    1.,
           -2.,   -1.,    1.,   -3.,   -3.,    1.,    2.,    1.,
           -5.,    3.,   -1.,    4.,    6.,   -4.,    4.,    0.,
            1.,   -1.,    0.,    3.,    3.,    1.,   -1.,   -4.,
       -30100.,-2013., 5675.,-1902., 3010.,-2067., 1632.,  -68., /* gf (1975) */
         1276.,-2144., -333., 1260.,  262.,  830., -223.,  946.,
          791.,  191.,  438., -265., -405.,   39.,  216., -288.,
         -218.,  356.,   31.,  264.,  148.,  -59., -152., -159.,
          -83.,  -49.,   88.,   45.,   66.,  -13.,   28.,   99.,
         -198.,   75.,    1.,  -41.,    6.,   -4., -111.,   11.,
           71.,  -56.,  -77.,    1.,  -26.,   16.,   -5.,  -14.,
           10.,    0.,   22.,   12.,  -23.,   -5.,  -12.,   14.,
            6.,    6.,   -1.,  -16.,  -12.,    4.,   -8.,  -19.,
            4.,    6.,    0.,   18.,   10.,  -10.,    1.,  -17.,
            7.,   10.,  -21.,    2.,   16.,  -12.,    7.,   10.,
           -4.,   -1.,   -5.,   -1.,   10.,    4.,   11.,    1.,
           -3.,   -2.,    1.,   -3.,   -3.,    1.,    2.,    1.,
           -5.,    3.,   -2.,    4.,    5.,   -4.,    4.,   -1.,
            1.,   -1.,    0.,    3.,    3.,    1.,   -1.,   -5.,
       -29992.,-1956., 5604.,-1997., 3027.,-2129., 1663., -200., /* gg (1980) */
         1281.,-2180., -336., 1251.,  271.,  833., -252.,  938.,
          782.,  212.,  398., -257., -419.,   53.,  199., -297.,
         -218.,  357.,   46.,  261.,  150.,  -74., -151., -162.,
          -78.,  -48.,   92.,   48.,   66.,  -15.,   42.,   93.,
         -192.,   71.,    4.,  -43.,   14.,   -2., -108.,   17.,
           72.,  -59.,  -82.,    2.,  -27.,   21.,   -5.,  -12.,
           16.,    1.,   18.,   11.,  -23.,   -2.,  -10.,   18.,
            6.,    7.,    0.,  -18.,  -11.,    4.,   -7.,  -22.,
            4.,    9.,    3.,   16.,    6.,  -13.,   -1.,  -15.,
            5.,   10.,  -21.,    1.,   16.,  -12.,    9.,    9.,
           -5.,   -3.,   -6.,   -1.,    9.,    7.,   10.,    2.,
           -6.,   -5.,    2.,   -4.,   -4.,    1.,    2.,    0.,
           -5.,    3.,   -2.,    6.,    5.,   -4.,    3.,    0.,
            1.,   -1.,    2.,    4.,    3.,    0.,    0.,   -6.,
       -29873.,-1905., 5500.,-2072., 3044.,-2197., 1687., -306., /* gi (1985) */
         1296.,-2208., -310., 1247.,  284.,  829., -297.,  936.,
          780.,  232.,  361., -249., -424.,   69.,  170., -297.,
         -214.,  355.,   47.,  253.,  150.,  -93., -154., -164.,
          -75.,  -46.,   95.,   53.,   65.,  -16.,   51.,   88.,
         -185.,   69.,    4.,  -48.,   16.,   -1., -102.,   21.,
           74.,  -62.,  -83.,    3.,  -27.,   24.,   -2.,   -6.,
           20.,    4.,   17.,   10.,  -23.,    0.,   -7.,   21.,
            6.,    8.,    0.,  -19.,  -11.,    5.,   -9.,  -23.,
            4.,   11.,    4.,   14.,    4.,  -15.,   -4.,  -11.,
            5.,   10.,  -21.,    1.,   15.,  -12.,    9.,    9.,
           -6.,   -3.,   -6.,   -1.,    9.,    7.,    9.,    1.,
           -7.,   -5.,    2.,   -4.,   -4.,    1.,    3.,    0.,
           -5.,    3.,   -2.,    6.,    5.,   -4.,    3.,    0.,
            1.,   -1.,    2.,    4.,    3.,    0.,    0.,   -6.,
       -29775.,-1848., 5406.,-2131., 3059.,-2279., 1686., -373., /* gj (1990) */
         1314.,-2239., -284., 1248.,  293.,  802., -352.,  939.,
          780.,  247.,  325., -240., -423.,   84.,  141., -299.,
         -214.,  353.,   46.,  245.,  154., -109., -153., -165.,
          -69.,  -36.,   97.,   61.,   65.,  -16.,   59.,   82.,
         -178.,   69.,    3.,  -52.,   18.,    1.,  -96.,   24.,
           77.,  -64.,  -80.,    2.,  -26.,   26.,    0.,   -1.,
           21.,    5.,   17.,    9.,  -23.,    0.,   -4.,   23.,
            5.,   10.,   -1.,  -19.,  -10.,    6.,  -12.,  -22.,
            3.,   12.,    4.,   12.,    2.,  -16.,   -6.,  -10.,
            4.,    9.,  -20.,    1.,   15.,  -12.,   11.,    9.,
           -7.,   -4.,   -7.,   -2.,    9.,    7.,    8.,    1.,
           -7.,   -6.,    2.,   -3.,   -4.,    2.,    2.,    1.,
           -5.,    3.,   -2.,    6.,    4.,   -4.,    3.,    0.,
            1.,   -2.,    3.,    3.,    3.,   -1.,    0.,   -6.,
       -29692.,-1784., 5306.,-2200., 3070.,-2366., 1681., -413., /* gk (1995) */
         1335.,-2267., -262., 1249.,  302.,  759., -427.,  940.,
          780.,  262.,  290., -236., -418.,   97.,  122., -306.,
         -214.,  352.,   46.,  235.,  165., -118., -143., -166.,
          -55.,  -17.,  107.,   68.,   67.,  -17.,   68.,   72.,
         -170.,   67.,   -1.,  -58.,   19.,    1.,  -93.,   36.,
           77.,  -72.,  -69.,    1.,  -25.,   28.,    4.,    5.,
           24.,    4.,   17.,    8.,  -24.,   -2.,   -6.,   25.,
            6.,   11.,   -6.,  -21.,   -9.,    8.,  -14.,  -23.,
            9.,   15.,    6.,   11.,   -5.,  -16.,   -7.,   -4.,
            4.,    9.,  -20.,    3.,   15.,  -10.,   12.,    8.,
           -6.,   -8.,   -8.,   -1.,    8.,   10.,    5.,   -2.,
           -8.,   -8.,    3.,   -3.,   -6.,    1.,    2.,    0.,
           -4.,    4.,   -1.,    5.,    4.,   -5.,    2.,   -1.,
            2.,   -2.,    5.,    1.,    1.,   -2.,    0.,   -7.,
            0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
            0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
            0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
            0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
            0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
            0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
            0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
            0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
            0.,    0.,    0.,    0.,    0.,    0.,    0.,    0.,
            0.,    0.,    0.,
       -29619.4,-1728.2, 5186.1,-2267.7, 3068.4,-2481.6, 1670.9, /* gl (2000) */
         -458.0, 1339.6,-2288.0, -227.6, 1252.1,  293.4,  714.5,
         -491.1,  932.3,  786.8,  272.6,  250.0, -231.9, -403.0,
          119.8,  111.3, -303.8, -218.8,  351.4,   43.8,  222.3,
          171.9, -130.4, -133.1, -168.6,  -39.3,  -12.9,  106.3,
           72.3,   68.2,  -17.4,   74.2,   63.7, -160.9,   65.1,
           -5.9,  -61.2,   16.9,    0.7,  -90.4,   43.8,   79.0,
          -74.0,  -64.6,    0.0,  -24.2,   33.3,    6.2,    9.1,
           24.0,    6.9,   14.8,    7.3,  -25.4,   -1.2,   -5.8,
           24.4,    6.6,   11.9,   -9.2,  -21.5,   -7.9,    8.5,
          -16.6,  -21.5,    9.1,   15.5,    7.0,    8.9,   -7.9,
          -14.9,   -7.0,   -2.1,    5.0,    9.4,  -19.7,    3.0,
           13.4,   -8.4,   12.5,    6.3,   -6.2,   -8.9,   -8.4,
           -1.5,    8.4,    9.3,    3.8,   -4.3,   -8.2,   -8.2,
            4.8,   -2.6,   -6.0,    1.7,    1.7,    0.0,   -3.1,
            4.0,   -0.5,    4.9,    3.7,   -5.9,    1.0,   -1.2,
            2.0,   -2.9,    4.2,    0.2,    0.3,   -2.2,   -1.1,
           -7.4,    2.7,   -1.7,    0.1,   -1.9,    1.3,    1.5,
           -0.9,   -0.1,   -2.6,    0.1,    0.9,   -0.7,   -0.7,
            0.7,   -2.8,    1.7,   -0.9,    0.1,   -1.2,    1.2,
           -1.9,    4.0,   -0.9,   -2.2,   -0.3,   -0.4,    0.2,
            0.3,    0.9,    2.5,   -0.2,   -2.6,    0.9,    0.7,
           -0.5,    0.3,    0.3,    0.0,   -0.3,    0.0,   -0.4,
            0.3,   -0.1,   -0.9,   -0.2,   -0.4,   -0.4,    0.8,
           -0.2,   -0.9,   -0.9,    0.3,    0.2,    0.1,    1.8,
           -0.4,   -0.4,    1.3,   -1.0,   -0.4,   -0.1,    0.7,
            0.7,   -0.4,    0.3,    0.3,    0.6,   -0.1,    0.3,
            0.4,   -0.2,    0.0,   -0.5,    0.1,   -0.9,
       -29554.63, -1669.05,  5077.99, -2337.24, 3047.69, -2594.50, 1657.76, /* gm (2005) */
         -515.43,  1336.30, -2305.83,  -198.86, 1246.39,   269.72,  672.51,
         -524.72,   920.55,  797.96,    282.07,  210.65,  -225.23, -379.86,
          145.15,   100.00, -305.36,   -227.00,  354.41,    42.72,  208.95,
          180.25,  -136.54, -123.45,   -168.05,  -19.57,   -13.55,  103.85,
           73.60,    69.56,  -20.33,     76.74,   54.75,  -151.34,   63.63,
          -14.58,   -63.53,   14.58,      0.24,  -86.36,    50.94,   79.88,
          -74.46,   -61.14,   -1.65,    -22.57,   38.73,     6.82,   12.30,
           25.35,     9.37,   10.93,      5.42,  -26.32,     1.94,   -4.64,
           24.80,     7.62,   11.20,    -11.73,  -20.88,    -6.88,    9.83,
          -18.11,   -19.71,   10.17,     16.22,    9.36,     7.61,  -11.25,
          -12.76,    -4.87,   -0.06,      5.58,    9.76,   -20.11,    3.58,
           12.69,    -6.94,   12.67,      5.01,   -6.72,   -10.76,   -8.16,
           -1.25,     8.10,    8.76,      2.92,   -6.66,    -7.73,   -9.22,
            6.01,    -2.17,   -6.12,      2.19,    1.42,     0.10,   -2.35,
            4.46,    -0.15,    4.76,      3.06,   -6.58,     0.29,   -1.01,
            2.06,    -3.47,    3.77,     -0.86,   -0.21,    -2.31,   -2.09,
           -7.93,     2.95,   -1.60,      0.26,   -1.88,     1.44,    1.44,
           -0.77,    -0.31,   -2.27,      0.29,    0.90,    -0.79,   -0.58,
            0.53,    -2.69,    1.80,     -1.08,    0.16,    -1.58,    0.96,
           -1.90,     3.99,   -1.39,     -2.15,   -0.29,    -0.55,    0.21,
            0.23,     0.89,    2.38,     -0.38,   -2.63,     0.96,    0.61,
           -0.30,     0.40,    0.46,      0.01,   -0.35,     0.02,   -0.36,
            0.28,     0.08,   -0.87,     -0.49,   -0.34,    -0.08,    0.88,
           -0.16,    -0.88,   -0.76,      0.30,    0.33,     0.28,    1.72,
           -0.43,    -0.54,    1.18,     -1.07,   -0.37,    -0.04,    0.75,
            0.63,    -0.26,    0.21,      0.35,    0.53,    -0.05,    0.38,
            0.41,    -0.22,   -0.10,     -0.57,   -0.18,    -0.82,
       -29496.5,  -1585.9,  4945.1,   -2396.6,  3026.0,  -2707.7,  1668.6, /* gm (2010) */
         -575.4,   1339.7, -2326.3,    -160.5,  1231.7,    251.7,   634.2, 
         -536.8,    912.6,   809.0,     286.4,   166.6,   -211.2,  -357.1, 
          164.4,     89.7,  -309.2,    -231.1,   357.2,     44.7,   200.3, 
          188.9,   -141.2,  -118.1,    -163.1,     0.1,     -7.7,   100.9, 
           72.8,     68.6,   -20.8,      76.0,    44.2,   -141.4,    61.5, 
          -22.9,    -66.3,    13.1,       3.1,   -77.9,     54.9,    80.4, 
          -75.0,    -57.8,    -4.7,     -21.2,    45.3,      6.6,    14.0, 
           24.9,     10.4,     7.0,       1.6,   -27.7,      4.9,    -3.4, 
           24.3,      8.2,    10.9,     -14.5,   -20.0,     -5.7,    11.9, 
          -19.3,    -17.4,    11.6,      16.7,    10.9,      7.1,   -14.1, 
          -10.8,     -3.7,     1.7,       5.4,     9.4,    -20.5,     3.4, 
           11.6,     -5.3,    12.8,       3.1,    -7.2,    -12.4,    -7.4, 
           -0.8,      8.0,     8.4,       2.2,    -8.4,     -6.1,   -10.1, 
            7.0,     -2.0,    -6.3,       2.8,     0.9,     -0.1,    -1.1, 
            4.7,     -0.2,     4.4,       2.5,    -7.2,     -0.3,    -1.0, 
            2.2,     -4.0,     3.1,      -2.0,    -1.0,     -2.0,    -2.8, 
           -8.3,      3.0,    -1.5,       0.1,    -2.1,      1.7,     1.6, 
           -0.6,     -0.5,    -1.8,       0.5,     0.9,     -0.8,    -0.4, 
            0.4,     -2.5,     1.8,      -1.3,     0.2,     -2.1,     0.8, 
           -1.9,      3.8,    -1.8,      -2.1,    -0.2,     -0.8,     0.3, 
            0.3,      1.0,     2.2,      -0.7,    -2.5,      0.9,     0.5, 
           -0.1,      0.6,     0.5,       0.0,    -0.4,      0.1,    -0.4, 
            0.3,      0.2,    -0.9,      -0.8,    -0.2,      0.0,     0.8, 
           -0.2,     -0.9,    -0.8,       0.3,     0.3,      0.4,     1.7, 
           -0.4,     -0.6,     1.1,      -1.2,    -0.3,     -0.1,     0.8, 
            0.5,     -0.2,     0.1,       0.4,     0.5,      0.0,     0.4, 
            0.4,     -0.2,    -0.3,      -0.5,    -0.3,     -0.8, 
           11.4,     16.7,   -28.8,     -11.3,    -3.9,    -23.0,     2.7, /* sv (2010) */
          -12.9,      1.3,    -3.9,       8.6,    -2.9,     -2.9,    -8.1, 
           -2.1,     -1.4,     2.0,       0.4,    -8.9,      3.2,     4.4, 
            3.6,     -2.3,    -0.8,      -0.5,     0.5,      0.5,    -1.5, 
            1.5,     -0.7,     0.9,       1.3,     3.7,      1.4,    -0.6, 
           -0.3,     -0.3,    -0.1,      -0.3,    -2.1,      1.9,    -0.4, 
           -1.6,     -0.5,    -0.2,       0.8,     1.8,      0.5,     0.2, 
           -0.1,      0.6,    -0.6,       0.3,     1.4,     -0.2,     0.3, 
           -0.1,      0.1,    -0.8,      -0.8,    -0.3,      0.4,     0.2, 
           -0.1,      0.1,     0.0,      -0.5,     0.2,      0.3,     0.5, 
           -0.3,      0.4,     0.3,       0.1,     0.2,     -0.1,    -0.5, 
            0.4,      0.2,     0.4,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0,     0.0, 
            0.0,      0.0,     0.0,       0.0,     0.0,      0.0}
	 };
#define gh ((double *)&equiv_22)

	int i, j, k, l, m, n, ll, lm, kmx, nmx, nc;
	double cd, cl[13], tc, ct, sd, fn = 0.0, gn = 0.0, fm, sl[13];
	double rr, st, one, gmm, rho, two, three, ratio;
	double p[105], q[105], r, t, a2, b2;
	double H, F, X = 0, Y = 0, Z = 0, dec, dip;
	
	if (date < 1900.0 || date > 2015.0) {
		GMT_message (C, "%s: Your date (%g) is outside valid extrapolated range for IGRF (1900-2015)\n", C->init.progname, date);
		return (TRUE);
	}
	
	if (date < 2010.) {
		t = 0.2 * (date - 1900.);
		ll = (int) t;
		one = (double) ll;
		t -= one;
		if (date < 1995.) {
			nmx = 10;
			nc = nmx * (nmx + 2);
			ll = nc * ll;
			kmx = (nmx + 1) * (nmx + 2) / 2;
		} else {
			nmx = 13;
			nc = nmx * (nmx + 2);
			ll = (int) ((date - 1995.) * .2);
			ll = nc * ll + 2280		/* 2280, position of first coeff of 1995 */;
			kmx = (nmx + 1) * (nmx + 2) / 2;
		}
		tc = 1. - t;
		if (isv == 1) {
			tc = -.2;
			t = .2;
		}
	}
	else {
		t = date - 2010.;
		tc = 1.;
		if (isv == 1) {
			t = 1.;
			tc = 0.;
		}
		ll = 2865;		/* nth position corresponding to first coeff of 2010 */
		nmx = 13;
		nc = nmx * (nmx + 2);
		kmx = (nmx + 1) * (nmx + 2) / 2;
	}
	r = alt;
	sincosd (90.0 - lat, &st, &ct);
	sincosd (elong, &(sl[0]), &(cl[0]));
	cd = 1.;
	sd = 0.;
	l = 1;
	m = 1;
	n = 0;
	if (itype == 1) { /* conversion from geodetic to geocentric coordinates (using the WGS84 spheroid) */
		a2 = 40680631.6;
		b2 = 40408296.0;
		one = a2 * st * st;
		two = b2 * ct * ct;
		three = one + two;
		rho = sqrt(three);
		r = sqrt(alt * (alt + rho * 2.) + (a2 * one + b2 * two) / three);
		cd = (alt + rho) / r;
		sd = (a2 - b2) / rho * ct * st / r;
		one = ct;
		ct = ct * cd - st * sd;
		st = st * cd + one * sd;
	}
	ratio = 6371.2 / r;
	rr = ratio * ratio;

	/* computation of Schmidt quasi-normal coefficients p and x(=q) */

	p[0] = 1.;
	p[2] = st;
	q[0] = 0.;
	q[2] = ct;
	for (k = 2; k <= kmx; ++k) {
		if (n < m) {
			m = 0;
			n++;
			rr *= ratio;
			fn = (double) n;
			gn = (double) (n - 1);
		}
		fm = (double) m;
		if (k != 3) {
			if (m == n) {
				one = sqrt(1. - .5 / fm);
				j = k - n - 1;
				p[k-1] = one * st * p[j-1];
				q[k-1] = one * (st * q[j-1] + ct * p[j-1]);
				cl[m-1] = cl[m-2] * cl[0] - sl[m-2] * sl[0];
				sl[m-1] = sl[m-2] * cl[0] + cl[m-2] * sl[0];
			}
			else {
				gmm = (double) (m * m);
				one = sqrt(fn * fn - gmm);
				two = sqrt(gn * gn - gmm) / one;
				three = (fn + gn) / one;
				i = k - n;
				j = i - n + 1;
				p[k-1] = three * ct * p[i-1] - two * p[j-1];
				q[k-1] = three * (ct * q[i-1] - st * p[i-1]) - two * q[j-1];
			}
		}

		/* synthesis of x, y and z in geocentric coordinates */

		lm = ll + l;
		one = (tc * gh[lm-1] + t * gh[lm+nc-1]) * rr;
		if (m == 0) {
			X += one * q[k-1];
			Z -= (fn + 1.) * one * p[k-1];
			l++;
		}
		else {
			two = (tc * gh[lm] + t * gh[lm+nc]) * rr;
			three = one * cl[m-1] + two * sl[m - 1];
			X += three * q[k-1];
			Z -= (fn + 1.) * three * p[k-1];
			if (st != 0.)
				Y += (one * sl[m-1] - two * cl[m-1]) * fm * p[k-1] / st;
			else
				Y += (one * sl[m-1] - two * cl[m-1]) * q[k-1] * ct;
			l += 2;
		}
		m++;
	}

	/* conversion to coordinate system specified by itype */
	one = X;
	X = X * cd + Z * sd;
	Z = Z * cd - one * sd;
	H = sqrt(X*X + Y*Y);
	F = sqrt(H*H + Z*Z);
	dec = atan2d(Y,X);	dip = atan2d(Z,H);
	out[0] = F;		out[1] = H;
	out[2] = X;		out[3] = Y;
	out[4] = Z;
	out[5] = dec;		out[6] = dip;
	
	return (GMT_OK);
}

GMT_LONG GMT_redpol_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "redpol %s - Compute the\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: redpol <anom_grd> -G<rtp_grdfile> [-C<dec/dip>]\n");
	GMT_message (GMT, "       [-E<dip_grd>/<dec_grd>] [-F<m/n>] [-M<m|r>] [-N] [-W<win_width>]\n");
	GMT_message (GMT, "       [%s] [-T<year>] [-Z<filter>] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);
                
	GMT_message (GMT, "\tanom_grd is the input grdfile with the magnetic anomaly\n");
	GMT_message (GMT, "\t-G filename for output netCDF grdfile with the RTP solution\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-C<dec/dip> uses this constant values in the RTP procedure.\n");
	GMT_message (GMT, "\t-E<dip_grd/dec_grd> Get magnetization DIP & DEC from these grids [default: use IGRF].\n");
	GMT_message (GMT, "\t-F<m/n> filter with [25x25].\n");
	GMT_message (GMT, "\t-M<m|r> Set boundary conditions. m|r stands for mirror or replicate edges (Default is zero padding).\n");
	GMT_message (GMT, "\t-N Do NOT use Taylor expansion.\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-T<year> Year used by the IGRF routine to compute the various DECs & DIPs [default: 2000]\n");
	GMT_message (GMT, "\t-W<wid> window width in degrees [5]\n");
	GMT_message (GMT, "\t-Z<filter> Write filter file on disk\n");
	GMT_explain_options (GMT, "V");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_redpol_parse (struct GMTAPI_CTRL *C, struct REDPOL_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to redpol and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */


	GMT_LONG j, n_errors = 0, n_files = 0, pos = 0;
	char	p[GMT_LONG_TEXT];
	struct	GMT_OPTION *opt = NULL;
	struct	GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'C':
				sscanf (opt->arg, "%lf/%lf", &Ctrl->C.dec, &Ctrl->C.dip);
				Ctrl->C.dec *= D2R;
				Ctrl->C.dip *= D2R;
				Ctrl->C.const_f = TRUE;
				Ctrl->C.use_igrf = FALSE;
				break;
			case 'E':
				j = 0;
				while (GMT_strtok (opt->arg, "/", &pos, p)) {
					switch (j) {
						case 0:
							Ctrl->E.dipfile = strdup (p);
							Ctrl->E.dip_grd_only = TRUE;
							break;
						case 1:
							Ctrl->E.decfile = strdup (p);
							Ctrl->E.dip_grd_only = FALSE;
							Ctrl->E.dip_dec_grd = TRUE;
							break;
						default:
							GMT_message (GMT, "ERROR using option -E\n");
							n_errors++;
							break;
					}
					j++;
				}
				Ctrl->E.active = TRUE;
				Ctrl->C.use_igrf = FALSE;
				break;
			case 'F':
				j = sscanf (opt->arg, "%" GMT_LL "d/%" GMT_LL "d", &Ctrl->F.ncoef_row, &Ctrl->F.ncoef_col);
				if (j == 1) Ctrl->F.compute_n = TRUE;	/* Case of only one filter dimension was given */
				if (Ctrl->F.ncoef_row %2 != 1 || Ctrl->F.ncoef_col %2 != 1) {
					GMT_message (GMT, "Error: number of filter coefficients must be odd\n");
					n_errors++;
				}
				if (Ctrl->F.ncoef_row < 5 || Ctrl->F.ncoef_col < 5) {
					GMT_message (GMT, "That was a ridiculous number of filter coefficients\n");
					n_errors++;
				}
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'M':
				Ctrl->M.pad_zero = FALSE;
				for (j = 0; opt->arg[j]; j++) {
					if (opt->arg[j] == 'm')
						Ctrl->M.mirror = TRUE;
					else if (opt->arg[j] == 'r')
						Ctrl->M.mirror = FALSE;
					else {
						GMT_message (GMT, "Warning: Error using option -M (option ignored)\n");
						Ctrl->M.pad_zero = TRUE;
					}
				}
				break;
			case 'N':
				Ctrl->N.active = FALSE;
				break;
			case 'T':
				sscanf (opt->arg, "%lf", &Ctrl->T.year);
				break;
			case 'W':
				sscanf (opt->arg, "%lf", &Ctrl->W.wid);
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				Ctrl->Z.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output file\n");

	if (Ctrl->C.const_f && Ctrl->C.use_igrf) {	
		GMT_message (GMT, "Warning: -E option overrides -C\n");
		Ctrl->C.const_f = FALSE;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_redpol_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_redpol (struct GMTAPI_CTRL *API, struct GMT_OPTION *options) {

	GMT_LONG error = FALSE, wrote_one = FALSE;
	GMT_LONG row, col, nx_new, ny_new, one_or_zero, m21, n21;
        GMT_LONG i, j, ij, k, l, i2, j2, i3, n_jlon, n_jlat, jj, n_coef;
        double	tmp_d, sloni, slati, slonf, slatf, slonm, slatm;
        double	*ftlon = NULL, *ftlat = NULL, *gxr = NULL, *gxi = NULL, *fxr = NULL;
	double	*gxar = NULL, *gxai = NULL, *gxbr = NULL, *gxbi = NULL, *gxgr = NULL;
	double	*gxgi = NULL, *fxar = NULL, *fxbr = NULL, *fxgr = NULL, *fix = NULL;
	double	*gxtr = NULL, *gxti = NULL, *gxmr = NULL, *gxmi = NULL, *gxnr = NULL;
	double	*gxni = NULL, *fxtr = NULL, *fxmr = NULL, *fxnr = NULL;
	double	*cosphi = NULL, *sinphi = NULL, *cospsi = NULL, *sinpsi = NULL;
	double	fi, psi, alfa = 0, beta = 0, gama = 0, r, s, u, v;
	double	alfa1, beta1, gama1, da = 0, db = 0, dg = 0, aniso;
	double	dec_m, dip_m, tau1, mu1, nu1, dt = 0, dm = 0, dn = 0, tau = 0, mu = 0, nu = 0;
	double	wesn_new[4], out_igrf[7];

	struct	REDPOL_CTRL *Ctrl = NULL;
	struct	GMT_GRID *Gin = NULL, *Gout = NULL, *Gdip = NULL, *Gdec = NULL, *Gfilt = NULL;
	struct	GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct	GMT_EDGEINFO edgeinfo_dec, edgeinfo_dip;
	struct	GMT_BCR bcr_dec, bcr_dip;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) 
		return (GMT_redpol_usage (API, GMTAPI_USAGE));		/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) 
		return (GMT_redpol_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_redpol", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VR", "", options))) Return (error);
	Ctrl = (struct REDPOL_CTRL *) New_redpol_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_redpol_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the redpol main code ----------------------------*/

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) 	/* Enables data input and sets access mode */
		Return (error);

	/* ... */
	if (Ctrl->F.compute_n) {
		aniso = Gin->header->inc[GMT_X] / Gin->header->inc[GMT_Y] * cos(Gin->header->wesn[YHI]*D2R);
		Ctrl->F.ncoef_col = (int) ((double)Ctrl->F.ncoef_row / aniso);
		if (Ctrl->F.ncoef_col %2 != 1) Ctrl->F.ncoef_col++;
	}

	m21 = (Ctrl->F.ncoef_row+1) / 2;	n21 = (Ctrl->F.ncoef_col+1) / 2;
	GMT->current.io.pad[XLO] = GMT->current.io.pad[XHI] = n21-1;
	GMT->current.io.pad[YLO] = GMT->current.io.pad[YHI] = m21-1;

	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, 	/* Get header only */
			 (void **)&(Ctrl->In.file), (void **)&Gin)) 
		Return (GMT_DATA_READ_ERROR);

	if (!GMT->common.R.active) 
		GMT_memcpy (wesn_new, Gin->header->wesn, 4, double);
	else
		GMT_memcpy (wesn_new, GMT->common.R.wesn, 4, double);

	one_or_zero = (Gin->header->registration == GMT_PIXEL_REG) ? 0 : 1;
	nx_new = irint ((wesn_new[XHI] - wesn_new[XLO]) / Gin->header->inc[GMT_X]) + one_or_zero;
	ny_new = irint ((wesn_new[YHI] - wesn_new[YLO]) / Gin->header->inc[GMT_Y]) + one_or_zero;

	Ctrl->S.nx = nx_new;		Ctrl->S.ny = ny_new;

	GMT_grd_init (GMT, Gin->header, options, TRUE);

	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn_new, GMT_GRID_DATA, 
			 (void **)&(Ctrl->In.file), (void **)&Gin)) Return (GMT_DATA_READ_ERROR);	/* Get subset */

	GMT_set_pad (GMT, 2);		/* Reset the default GMT pad */

	Gout = GMT_create_grid (GMT);

	GMT_memcpy (Gout->header->wesn, wesn_new, 4, double);
	GMT_memcpy (Gout->header->inc, Gin->header->inc, 2, double);
	Gout->header->registration = Gin->header->registration;
	GMT_RI_prepare (GMT, Gout->header);	/* Ensure -R -I consistency and set nx, ny */
	Gout->header->pad[XLO] = Gout->header->pad[XHI] = Gout->header->pad[YLO] = Gout->header->pad[YHI] = 2;
	GMT_set_grddim (GMT, Gout->header);	/* SHOULDN'T IT USE GMT->current.io.pad INSTEAD ???? */

	if (GMT->common.R.active) {
		if (Gout->header->wesn[XLO] < Gin->header->wesn[XLO] || Gout->header->wesn[XHI] > Gin->header->wesn[XHI]) {
			GMT_report (GMT, GMT_MSG_FATAL, " Selected region exceeds the X-boundaries of the grid file!\n");
			return (EXIT_FAILURE);
		}
		else if (Gout->header->wesn[YLO] < Gin->header->wesn[YLO] || Gout->header->wesn[YHI] > Gin->header->wesn[YHI]) {
			GMT_report (GMT, GMT_MSG_FATAL, " Selected region exceeds the Y-boundaries of the grid file!\n");
			return (EXIT_FAILURE);
		}
	}

	if (!Ctrl->M.pad_zero)		/* That is, if we want edges reflected or replicated */
		mirror_edges (Gin->data, Ctrl->F.ncoef_col-1, m21-1, n21-1, Ctrl);

	/* Section to deal with possible external grids with dip and dec for interpolation */

	if (Ctrl->E.dip_grd_only || Ctrl->E.dip_dec_grd) {
		GMT_boundcond_init (GMT, &edgeinfo_dip);
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 		/* Get header only */
			 	  GMT_GRID_HEADER, (void **)&(Ctrl->E.dipfile), (void **)&Gdip)) 
			Return (GMT_DATA_READ_ERROR);
	
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn_new, GMT_GRID_DATA, 
			 	(void **)&(Ctrl->E.dipfile), (void **)&Gdip)) 
			Return (GMT_DATA_READ_ERROR);

		GMT_boundcond_param_prep (GMT, Gdip, &edgeinfo_dip);
		GMT_bcr_init (GMT, Gdip, BCR_BILINEAR, 1.0, &bcr_dip);		/* Initialize bcr structure: */
		GMT_boundcond_set (GMT, Gdip, &edgeinfo_dip); 			/* Set boundary conditions */
	}
	if (Ctrl->E.dip_dec_grd) {
		GMT_boundcond_init (GMT, &edgeinfo_dec);
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER,
			 	(void **)&(Ctrl->E.decfile), (void **)&Gdec)) 
			Return (GMT_DATA_READ_ERROR);
	
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn_new, GMT_GRID_DATA, 
			 	(void **)&(Ctrl->E.decfile), (void **)&Gdec)) 
			Return (GMT_DATA_READ_ERROR);

		GMT_boundcond_param_prep (GMT, Gdec, &edgeinfo_dec);
		GMT_bcr_init (GMT, Gdec, BCR_BILINEAR, 1.0, &bcr_dec);		/* Initialize bcr structure: */
		GMT_boundcond_set (GMT, Gdec, &edgeinfo_dec); 			/* Set boundary conditions */
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);		/* Disables further data input */

	n_coef = Ctrl->F.ncoef_row * Ctrl->F.ncoef_col;
       	cosphi = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	sinphi = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	cospsi = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	sinpsi = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	gxr    = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	gxi    = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	gxar   = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	gxai   = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	gxbr   = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	gxbi   = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	gxgr   = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	gxgi   = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	fxr    = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	fix    = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	fxar   = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	fxbr   = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	fxgr   = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       	ftlon  = (double *) GMT_memory (GMT, NULL, (size_t) Gin->header->nx, double);
       	ftlat  = (double *) GMT_memory (GMT, NULL, (size_t) Gin->header->ny, double);

	if ((Ctrl->E.dip_grd_only || Ctrl->E.dip_dec_grd)) { 
       		gxtr = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       		gxti = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       		gxmr = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       		gxmi = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       		gxnr = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       		gxni = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       		fxtr = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       		fxmr = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
       		fxnr = (double *) GMT_memory (GMT, NULL, (size_t)n_coef, double);
	}

	/* Generate vectors of lon & lats */
	for (col = 0; col < Gin->header->nx; col++) ftlon[col] = GMT_grd_col_to_x (col, Gin->header);
	for (row = 0; row < Gin->header->ny; row++) ftlat[row] = GMT_grd_row_to_y (row, Gin->header);

	n_jlon = (int)((Gin->header->wesn[XHI] - Gin->header->wesn[XLO]) / Ctrl->W.wid) + 1;
	n_jlat = (int)((Gin->header->wesn[YHI] - Gin->header->wesn[YLO]) / Ctrl->W.wid) + 1;

	if (Ctrl->C.const_f) {
		alfa = -cos(Ctrl->C.dip) * cos(Ctrl->C.dec);
		beta =  cos(Ctrl->C.dip) * sin(Ctrl->C.dec);
		gama = -sin(Ctrl->C.dip);
	}
	
	fi  = TWO_PI / Ctrl->F.ncoef_row;
	psi = TWO_PI / Ctrl->F.ncoef_col;

	Gout->data = GMT_memory (GMT, NULL, Gout->header->size, float);
					
	if (Ctrl->Z.active) {		/* Create one grid to hold the filter coefficients */
		Gfilt = GMT_create_grid (GMT);
		GMT_grd_init (GMT, Gfilt->header, options, TRUE);
		strcpy (Gfilt->header->title, "Reduction To the Pole filter");
		strcpy (Gfilt->header->x_units, "radians");
		strcpy (Gfilt->header->y_units, "radians");
		Gfilt->header->nx = (int)Ctrl->F.ncoef_col;		Gfilt->header->ny = (int)Ctrl->F.ncoef_row;
		Gfilt->header->inc[GMT_X] = Gfilt->header->inc[GMT_Y] = 1;
		Gfilt->header->wesn[XLO] = 1;		Gfilt->header->wesn[XHI] = (double)Ctrl->F.ncoef_col;
		Gfilt->header->wesn[YLO] = 1;		Gfilt->header->wesn[YHI] = (double)Ctrl->F.ncoef_row;
		Gfilt->header->z_scale_factor = 1;
		Gfilt->header->z_add_offset = 0;
		Gfilt->data = GMT_memory (GMT, NULL, n_coef, float);
	}

	for (l = 0; l < n_jlat; l++) {		/* Main loop over the moving windows */
		for (k = 0; k < n_jlon; k++) {
			sloni = Gin->header->wesn[XLO] + k * Ctrl->W.wid; 
			slati = Gin->header->wesn[YHI] - (l+1) * Ctrl->W.wid; 
			slonf = sloni + Ctrl->W.wid;
			slatf = slati + Ctrl->W.wid;
			slonm = (sloni + slonf) / 2;
			slatm = (slati + slatf) / 2;
			if (Ctrl->F.compute_n) {
				aniso = Gin->header->inc[GMT_X] / Gin->header->inc[GMT_X] * cos(slatm*D2R);
				Ctrl->F.ncoef_col = (int) ((double)Ctrl->F.ncoef_row / aniso);
				if (Ctrl->F.ncoef_col %2 != 1) Ctrl->F.ncoef_row += 1;
				psi  = TWO_PI / Ctrl->F.ncoef_row;
				n21 = (Ctrl->F.ncoef_row+1) / 2;
			}
			/* Compute dec and dip at the central point of the moving window */
			igrf10syn(GMT, 0, Ctrl->T.year, 1, 0, slonm, slatm, out_igrf); 
			if (!Ctrl->C.const_f) {
				Ctrl->C.dec = out_igrf[5] * D2R;
				Ctrl->C.dip = out_igrf[6] * D2R;
				/* Calculo dos cosenos directores */
				alfa = -cos(Ctrl->C.dip) * cos(Ctrl->C.dec);
				beta =  cos(Ctrl->C.dip) * sin(Ctrl->C.dec);
				gama = -sin(Ctrl->C.dip);
			}
			if ((Ctrl->E.dip_grd_only || Ctrl->E.dip_dec_grd)) {	/* */
				if (Ctrl->E.dip_grd_only) {		/* Use mag DEC = 0 */
					dip_m = GMT_get_bcr_z(GMT, Gdip, slonm, slatm, &edgeinfo_dip, &bcr_dip) * D2R;
					dec_m = 0;
					tau = -cos(dip_m);
					mu  =  0;
					nu  = -sin(dip_m);
				}
				else {			/* Get central window mag DEC & DIP from grids */
					dip_m = GMT_get_bcr_z(GMT, Gdip, slonm, slatm, &edgeinfo_dip, &bcr_dip) * D2R;
					dec_m = GMT_get_bcr_z(GMT, Gdec, slonm, slatm, &edgeinfo_dec, &bcr_dec) * D2R;
					tau = -cos(dip_m) * cos(dec_m);
					mu  =  cos(dip_m) * sin(dec_m);
					nu  = -sin(dip_m);
				}
			}
			if (GMT->current.setting.verbose >= GMT_MSG_NORMAL)
				GMT_report (GMT, GMT_MSG_NORMAL, "Dec %5.1f  Dip %5.1f  Bin_lon %6.1f  Bin_lat %5.1f\r", 
					    Ctrl->C.dec/D2R, Ctrl->C.dip/D2R, slonm, slatm);

			/* Compute the filter coefficients in the frequency domain */
			for (i = 0; i < Ctrl->F.ncoef_row; i++) {
				for (j = n21-1; j < Ctrl->F.ncoef_col; j++) {
					r = (double)(i - m21 + 1);	s = (double)(j - n21 + 1);
					if (r == 0 && s == 0) continue;
					u = r * fi;		v = s * psi;

					if (!(Ctrl->E.dip_grd_only || Ctrl->E.dip_dec_grd)) {
						rtp_filt_colinear(i,j,n21,gxr,gxi,gxar,gxai,gxbr,gxbi,gxgr,gxgi,
								  u,v,alfa,beta,gama, Ctrl);
					}
					else {		/* Case more general. alfa, beta, gama, tau, mu e nu */
						rtp_filt_NOTcolinear(i,j,n21,gxr,gxi,gxar,gxai,gxbr,gxbi,gxgr,gxgi,gxtr,
							gxti,gxmr,gxmi,gxnr,gxni,u,v,alfa,beta,gama,tau,mu,nu, Ctrl);
					}
				}
			}
			gxr[ij_mn(Ctrl,m21-1,0)]  = 1;	gxi[ij_mn(Ctrl,m21-1,0)]  = 0; 
			gxar[ij_mn(Ctrl,m21-1,0)] = 0;	gxai[ij_mn(Ctrl,m21-1,0)] = 0; 
			gxbr[ij_mn(Ctrl,m21-1,0)] = 0;	gxbi[ij_mn(Ctrl,m21-1,0)] = 0; 
			gxgr[ij_mn(Ctrl,m21-1,0)] = 0;	gxgi[ij_mn(Ctrl,m21-1,0)] = 0; 

			/* Compute iFT of the filter */
			tfpoeq(fxr,Ctrl->F.ncoef_row,  Ctrl->F.ncoef_col,gxr,gxi,   cosphi,sinphi,cospsi,sinpsi); 
			tfpoeq(fxar,Ctrl->F.ncoef_row, Ctrl->F.ncoef_col,gxar,gxai, cosphi,sinphi,cospsi,sinpsi); 
			tfpoeq(fxbr,Ctrl->F.ncoef_row, Ctrl->F.ncoef_col,gxbr,gxbi, cosphi,sinphi,cospsi,sinpsi); 
			tfpoeq(fxgr,Ctrl->F.ncoef_row, Ctrl->F.ncoef_col,gxgr,gxgi, cosphi,sinphi,cospsi,sinpsi); 

			if ((Ctrl->E.dip_grd_only || Ctrl->E.dip_dec_grd)) { 
				gxtr[ij_mn(Ctrl,m21-1,0)] = 0;	gxti[ij_mn(Ctrl,m21-1,0)] = 0; 
				gxmr[ij_mn(Ctrl,m21-1,0)] = 0;	gxmi[ij_mn(Ctrl,m21-1,0)] = 0; 
				gxnr[ij_mn(Ctrl,m21-1,0)] = 0;	gxni[ij_mn(Ctrl,m21-1,0)] = 0;
				tfpoeq(fxtr,Ctrl->F.ncoef_row, Ctrl->F.ncoef_col,gxtr,gxti, cosphi,sinphi,cospsi,sinpsi); 
				tfpoeq(fxmr,Ctrl->F.ncoef_row, Ctrl->F.ncoef_col,gxmr,gxmi, cosphi,sinphi,cospsi,sinpsi); 
				tfpoeq(fxnr,Ctrl->F.ncoef_row, Ctrl->F.ncoef_col,gxnr,gxni, cosphi,sinphi,cospsi,sinpsi);
			}

			/* Convolve filter with input data that is inside current window (plus what filter width imposes) */
			GMT_row_loop(Gout,row) {
				if (ftlat[row] < slati || ftlat[row] > slatf) continue;		/* Current point outside WOI */
				GMT_col_loop(Gout,row,col,ij) {
					if (ftlon[col] < sloni || ftlon[col] > slonf) continue;	/* Current point outside WOI */
					/* Compute dec and dip at corrent point */
					if (!Ctrl->C.const_f) {		/* It means we need to get F (& M) vector parameters */
						igrf10syn(GMT, 0, Ctrl->T.year, 1, 0, ftlon[col], ftlat[row], out_igrf); 
						Ctrl->C.dec = out_igrf[5] * D2R;
						Ctrl->C.dip = out_igrf[6] * D2R;
						if (Ctrl->E.dip_grd_only) {
							dip_m = GMT_get_bcr_z(GMT, Gdip, ftlon[row], ftlat[row], &edgeinfo_dip, 
									      &bcr_dip) * D2R;
							dec_m = 0;
							tau1 = -cos(dip_m);
							mu1  =  0;
							nu1  = -sin(dip_m);
							dt = tau1 - tau;	dm = mu1 - mu;		dn = nu1 - nu;
						}
						else if (Ctrl->E.dip_dec_grd) {
							dec_m = GMT_get_bcr_z(GMT, Gdec, ftlon[col], ftlat[row], &edgeinfo_dec, 
									      &bcr_dec) * D2R;
							dip_m = GMT_get_bcr_z(GMT, Gdip, ftlon[col], ftlat[row], &edgeinfo_dip, 
									      &bcr_dip) * D2R;
							tau1 = -cos(dip_m) * cos(dec_m);
							mu1  =  cos(dip_m) * sin(dec_m);
							nu1  = -sin(dip_m);
							dt = tau1 - tau;	dm = mu1 - mu;		dn = nu1 - nu;
						}
						/* Compute director cosinus */
						alfa1 = -cos(Ctrl->C.dip) * cos(Ctrl->C.dec);
						beta1 =  cos(Ctrl->C.dip) * sin(Ctrl->C.dec);
						gama1 = -sin(Ctrl->C.dip);
						da = alfa1 - alfa;	db = beta1 - beta;	dg = gama1 - gama;
					}
					if (!Ctrl->N.active)		/* Do not use the Taylor expansion (What's the interest?) */
						da = db = dg = dt = dm = dn = 0;

					/* Rebuild the filter */
					if (!Ctrl->C.const_f) {
						if (Ctrl->C.use_igrf) { 
							for (i2 = 0; i2 < n_coef; i2++)
								fix[i2] = fxr[i2] + da * fxar[i2] + db * fxbr[i2] + dg * fxgr[i2];
						}
						else {
							for (i2 = 0; i2 < n_coef; i2++)
								fix[i2] = fxr[i2] + da * fxar[i2] + db * fxbr[i2] + dg * fxgr[i2] +
									  dt * fxtr[i2] + dm * fxmr[i2] + dn * fxnr[i2];
						}
					}
					else
						memcpy(fix, fxr, (size_t)(n_coef * sizeof(double)));

					if (Ctrl->Z.active && !wrote_one && l == 0 && k == 0) {
						for (i2 = jj = 0; i2 < Ctrl->F.ncoef_row; i2++)		/* Remember, filter is columnwise */
							for (j2 = 0; j2 < Ctrl->F.ncoef_col; j2++, jj++)
								Gfilt->data[jj] = (float)fix[ij_mn(Ctrl,i2,j2)];

						wrote_one = TRUE;
					}

					tmp_d = 0;
					for (i2 = 0; i2 < Ctrl->F.ncoef_row; i2++) {
						i3 = row + i2;
						for (j2 = 0; j2 < Ctrl->F.ncoef_col; j2++)
							tmp_d += fix[ij_mn(Ctrl,i2,j2)] * Gin->data[ij0_data(Ctrl,i3,(col + j2))];
					}

					Gout->data[ij] = (float)tmp_d;
				}
			}
		}
	}

	if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) GMT_report (GMT, GMT_MSG_NORMAL, "\n"); 

	GMT_free(GMT, cosphi);		GMT_free(GMT, sinphi);
	GMT_free(GMT, cospsi);		GMT_free(GMT, sinpsi);
	GMT_free(GMT, gxr);		GMT_free(GMT, gxi);
	GMT_free(GMT, ftlat);		GMT_free(GMT, ftlon);
	GMT_free(GMT, fxr);

	GMT_free(GMT, fxar);		GMT_free(GMT, fxbr);
	GMT_free(GMT, fxgr);		GMT_free(GMT, fix);

	GMT_free(GMT, gxar);		GMT_free(GMT, gxai);
	GMT_free(GMT, gxbr);		GMT_free(GMT, gxbi);
	GMT_free(GMT, gxgr);		GMT_free(GMT, gxgi);
	if ((Ctrl->E.dip_grd_only || Ctrl->E.dip_dec_grd)) {
       		GMT_free(GMT, gxtr);		GMT_free(GMT, gxti);
       		GMT_free(GMT, gxmr);		GMT_free(GMT, gxmi);
       		GMT_free(GMT, gxnr);		GMT_free(GMT, gxni);
       		GMT_free(GMT, fxtr);		GMT_free(GMT, fxmr);
       		GMT_free(GMT, fxnr);
	}

	strcpy (Gout->header->title, "Anomaly reducted to the pole");
	strcpy (Gout->header->z_units, "nT");

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) /* Enables data output and sets access mode */
		Return (error);

	GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->G.file, (void *)Gout);
	if (Ctrl->Z.active)
		GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->Z.file, (void *)Gfilt);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);			/* Disables further data output */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Gin);
	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Gout);
	if (Ctrl->E.dipfile)
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Gdip);
	if (Ctrl->E.decfile)
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Gdec);
	if (Ctrl->Z.active)
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Gfilt);

	Return (GMT_OK);
}
