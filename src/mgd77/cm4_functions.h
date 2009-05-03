/*---------------------------------------------------------------------------
 *	$Id: cm4_functions.h,v 1.3 2009-05-03 07:57:58 guru Exp $
 *
 *
 *  File:	cm4_functions.h
 *
 *  Functions required to compute CM4 magnetic components
 *
 *  Authors:    J. Luis translated from original Fortran code
 *		P. Wessel further massaged it into this form
 *		
 *  Version:	1.0
 *  Revised:	1-MAY-2009
 * 
 *-------------------------------------------------------------------------*/

struct MGD77_CM4 {
	struct C {	/*  */
		int curr;
	} C;
	struct D {	/*  */
		int active;
		int index; 
		int load;
		double *dst;
		char *path;
	} D;
	struct I {	/*  */
		int active;
		int index; 
		int load;
		double F107;
		char *path;
	} I;
	struct F {	/* -F<xymrw> */
		int active;
		int field_components[7];
		int n_field_components;
		int field_sources[7];
		int n_field_sources;
	} F;
	struct G {	/*  */
		int geodetic;
	} G;
	struct M {	/*  */
		char *path;
	} M;
	struct DATA {	/* */
		int pred[6];
		int n_pts;
		int n_times;
		int n_altitudes;
		int coef;
		double	gmdl[1];
		double	bmdl[21];
		double	jmdl[12];
		double	*out_field;
		double	*out_current;
	} DATA;
	struct S {	/*  */
		int nlmf[2];
		int nhmf[2];
	} S;
};

int MGD77_cm4field (struct MGD77_CM4 *Ctrl, double *p_lon, double *p_lat, double *p_alt, double *p_date);
void MGD77_CM4_init (struct MGD77_CONTROL *F, struct MGD77_CM4 *CM4);
