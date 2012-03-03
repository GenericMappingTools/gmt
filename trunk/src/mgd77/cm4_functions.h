/*---------------------------------------------------------------------------
 *  $Id$
 *
 *
 *  File:       cm4_functions.h
 *
 *  Functions required to compute CM4 magnetic components
 *
 *  Authors:    J. Luis translated from original Fortran code
 *              P. Wessel further massaged it into this form
 *
 *  Version:    1.0
 *  Revised:    1-MAY-2009
 *
 *-------------------------------------------------------------------------*/

#ifndef _CM4_FUNCTIONS_H
#define _CM4_FUNCTIONS_H

#include "gmt.h"

struct MGD77_CM4 {
	struct CM4_L {	/*  */
		int curr;
		int curr_components[4];
		int n_curr_components;
		int curr_sources[4];
		int n_curr_sources;
	} CM4_L;
	struct CM4_D {	/*  */
		int active;
		int index; 
		int load;
		double *dst;
		char *path;
	} CM4_D;
	struct CM4_I {	/*  */
		int active;
		int index; 
		int load;
		double F107;
		char *path;
	} CM4_I;
	struct CM4_F {	/* -F<xymrw> */
		int active;
		int field_components[7];
		int n_field_components;
		int field_sources[7];
		int n_field_sources;
	} CM4_F;
	struct CM4_G {	/*  */
		int geodetic;
	} CM4_G;
	struct CM4_M {	/*  */
		char *path;
	} CM4_M;
	struct CM4_DATA {	/* */
		int pred[6];
		int n_pts;
		int n_times;
		int n_altitudes;
		int coef;
		double	gmdl[1];
		double	*out_field;
	} CM4_DATA;
	struct CM4_S {	/*  */
		int nlmf[2];
		int nhmf[2];
	} CM4_S;
};

int MGD77_cm4field (struct GMT_CTRL *C, struct MGD77_CM4 *Ctrl, double *p_lon, double *p_lat, double *p_alt, double *p_date);
EXTERN_MSC void MGD77_CM4_init (struct GMT_CTRL *C, struct MGD77_CONTROL *F, struct MGD77_CM4 *CM4);

#endif /* _CM4_FUNCTIONS_H */
