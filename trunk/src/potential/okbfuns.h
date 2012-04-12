/*--------------------------------------------------------------------
 *	$Id: $
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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

#ifndef OKBFUNS_H
#define OKBFUNS_H

#include "gmt.h"		/* Requires GMT to compile and link */

struct BODY_VERTS {
	double  x, y, z;
};

struct BODY_DESC {
	GMT_LONG n_f, *n_v, *ind;
};

struct LOC_OR {
	double  x, y, z;
};

struct MAG_PARAM {
	double	rim[3];
} *mag_param;

struct MAG_VAR {		/* Used when only the modulus of magnetization varies */
	double	rk[3];
} *mag_var;

EXTERN_MSC double okabe (struct GMT_CTRL *GMT, double x_o, double y_o, double z_o, double rho, int is_grav, struct BODY_DESC bd_desc, struct BODY_VERTS *bd_vert, int km, int pm, struct LOC_OR *loc_or);
EXTERN_MSC double okb_grv (int n_vert, struct LOC_OR *loc_or, double *c_phi);
EXTERN_MSC double okb_mag (int n_vert, GMT_LONG km, GMT_LONG pm, struct LOC_OR *loc_or, double *c_tet, double *s_tet, double *c_phi, double *s_phi); 
EXTERN_MSC double eq_30 (double c, double s, double x, double y, double z);
EXTERN_MSC double eq_43 (double mz, double c, double tg, double auxil, double x, double y, double z);
EXTERN_MSC void rot_17 (int n_vert, int top, struct LOC_OR *loc_or, double *c_tet, double *s_tet, double *c_phi, double *s_phi);

#endif /* OKBFUNS_H */
