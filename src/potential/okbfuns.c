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

#include "okbfuns.h"

GMT_LOCAL double okb_grv (unsigned int n_vert, struct LOC_OR *loc_or, double c_phi);
GMT_LOCAL double okb_mag (unsigned int n_vert, unsigned int km, unsigned int pm, struct LOC_OR *loc_or,
	double c_tet, double s_tet, double c_phi, double s_phi, struct MAG_PARAM *okabe_mag_param, struct MAG_VAR *okabe_mag_var);
GMT_LOCAL double eq_30 (double c, double s, double x, double y, double z);
GMT_LOCAL double eq_43 (double mz, double c, double tg, double auxil, double x, double y, double z);
GMT_LOCAL void rot_17 (unsigned int n_vert, bool top, struct LOC_OR *loc_or, double *c_tet, double *s_tet,
	double *c_phi, double *s_phi);

/*--------------------------------------------------------------------*/
double okabe (struct GMT_CTRL *GMT, double x_o, double y_o, double z_o, double rho, bool is_grav,
              struct BODY_DESC bd_desc, struct BODY_VERTS *body_verts, unsigned int km, unsigned int pm, struct LOC_OR *loc_or_, struct MAG_PARAM *okabe_mag_param, struct MAG_VAR *okabe_mag_var) {

	double okb = 0, c_tet = 0, s_tet = 0, c_phi = 0, s_phi = 0;
	unsigned int i, l, k, cnt_v = 0, n_vert;
	bool top = true;
	struct LOC_OR loc_or[32];
	gmt_M_unused(loc_or_);
	GMT_declare_gmutex		/* A no-op when no HAVE_GLIB_GTHREAD */

/* x_o, y_o, z_o are the coordinates of the observation point
 * rho is the body density times G constant
 * km is an: index of current body facet (if they have different mags); or 0 if mag=const
 * pm is an: index of current body facet (when all F, Mag may vary); or 0 if mag=const. This an UNDOCUMENTED feature
 * bd_desc is a structure containing the body's description. It contains the following members
 * n_f -> number of facets (int)
 * n_v -> number of vertex of each facet (pointer)
 * ind -> index describing the vertex order of each facet. These index must
 * describe the facet in a clock-wise order when viewed from outside.
 *
 * loc_or_ is now deprecated (replaced) by the heap alloc(ed) (local) loc_or.
 * Though this might be a limitation form the dynamic allocation view point,
 * it is need for multi-threading usage of this function. A 32 size should be
 * enough for all use cases in GMT.

    _________________________________________________________________
    |                                                               |
    |  Reference : Okabe, M., Analytical expressions for gravity    |
    |     anomalies due to polyhedral bodies and translation into   |
    |     magnetic anomalies, Geophysics, 44, (1979), p 730-741.    |
    |_______________________________________________________________|
    _____________________________________________________________________
    |                                                                   |
    |  Ifac describes the body (ATTN : Integer*2) :                     |
    |   - There are Nff facets that are described clockwise when seen   |
    |          from outside. Mxsomf = Max corners / facet               |
    |   - First number indicates the number of facets. Than follow      |
    |      groups of numbers, the first of each group is the number     |
    |      of vertices of the facet, followed by the indices (pointers) |
    |      vertices (stored in Xx, Yy, Zz) corresponding to the facet.  |
    |                                                                   |
    |  For example for a cube                  _________________        |
    |  (Nff=6; 4 corners /face)              /|         X (North)       |
    |  [Ifac] = { 6,  4, 1,2,3,4,           / |                         |
    |                 4, 2,6,7,3,          /  |     1 ________ 2        |
    |                 4, 4,3,7,8,         /   |      /       /|         |
    |                 4, 5,1,4,8,      Y /    |     /       / |         |
    |                 4, 1,5,6,2,      (East) |  4 /_______/3 |         |
    |                 4, 5,8,7,6 }            |    |       |  |         |
    |                                         |    |       | / 6        |
    |                                       Z |    |       |/           |
    |                                         V    |_______/            |
    |                                             8         7           |
    |___________________________________________________________________|
    |                                                                   |
    |  X,Y & Z are the coordinate arrays of the measure points          |
    |___________________________________________________________________| */

	for (i = 0; i < bd_desc.n_f; i++) {	/* Loop over facets */
		n_vert = bd_desc.n_v[i];	/* Number of vertices of each face */
		if (n_vert < 3)
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Facet with less than 3 vertex\n");
		for (l = 0; l < n_vert; l++) {
			k = bd_desc.ind[l+cnt_v];
			loc_or[l].x = body_verts[k].x - x_o;
			loc_or[l].y = body_verts[k].y - y_o;
			loc_or[l].z = body_verts[k].z - z_o;
		}
		rot_17 (n_vert, top, loc_or, &c_tet, &s_tet, &c_phi, &s_phi); /* rotate coords by eq (17) of okb */
		okb += (is_grav) ? okb_grv (n_vert, loc_or, c_phi) :
				okb_mag (n_vert, km, pm, loc_or, c_tet, s_tet, c_phi, s_phi, okabe_mag_param, okabe_mag_var);
		cnt_v += n_vert;
	}
	GMT_set_gmutex		/* A no-op when no HAVE_GLIB_GTHREAD */
	if (is_grav) okb *= rho;
	GMT_unset_gmutex
	return (okb);
}

/* ---------------------------------------------------------------------- */
GMT_LOCAL void rot_17 (unsigned int n_vert, bool top, struct LOC_OR *loc_or,
                       double *c_tet, double *s_tet, double *c_phi, double *s_phi) {
	/* Rotates coordinates by teta and phi according to equation (17) of Okabe */
	/* store the result in external structure loc_or and angles c_tet s_tet c_phi s_phi */
	double xi, xj, xk, yi, yj, yk, zi, zj, zk, v, x, y, z;
	double r, r2, r_3d, Sxy, Szx, Syz;
	unsigned int i = 0, j, k, l;

	loc_or[n_vert].x = loc_or[0].x;		loc_or[n_vert].y = loc_or[0].y;
	loc_or[n_vert].z = loc_or[0].z;		/* Last point = first point */

	if (top) { /* Currently, this is always true */
		j = i + 1;	k = i + 2;
		xi = loc_or[i].x;	xj = loc_or[j].x;	xk = loc_or[k].x;
		yi = loc_or[i].y;	yj = loc_or[j].y;	yk = loc_or[k].y;
		zi = loc_or[i].z;	zj = loc_or[j].z;	zk = loc_or[k].z;
		Sxy = xi * (yj - yk) + xj * (yk - yi) + xk * (yi - yj);
		Syz = yi * (zj - zk) + yj * (zk - zi) + yk * (zi - zj);
		Szx = zi * (xj - xk) + zj * (xk - xi) + zk * (xi - xj);
		r2 = Syz * Syz + Szx * Szx;
		r = sqrt(r2);
		r_3d = sqrt(r2 + Sxy * Sxy);
		*c_phi = - Sxy / r_3d;
		*s_phi = r / r_3d;

		if (Szx == 0.0 && Syz == 0.0) {*c_tet = 1.0;	*s_tet = 0.0;}
		else {*c_tet = - Syz / r;	*s_tet = - Szx / r;}
		}
	else { /* Don't need to recompute angles, only do this */
		*c_tet *= -1;	*s_tet *= -1;	*c_phi *= -1;
	}

	for (l = 0; l < n_vert + 1; l++) {
		x = loc_or[l].x;	y = loc_or[l].y;	z = loc_or[l].z;
		v = x * *c_tet + y * *s_tet;
		loc_or[l].x = v * *c_phi - z * *s_phi;
		loc_or[l].y = y * *c_tet - x * *s_tet;
		loc_or[l].z = v * *s_phi + z * *c_phi;
	}
}

/* ---------------------------------------------------------------------- */
GMT_LOCAL double okb_grv (unsigned int n_vert, struct LOC_OR *loc_or, double c_phi) {
/*  Computes the gravity anomaly due to a facet. */

	unsigned int l;
	double x1, x2, y1, y2, dx, dy, r, c_psi, s_psi, grv = 0, grv_p;

	if (fabs(c_phi) < FLT_EPSILON) return 0.0;
	for (l = 0; l < n_vert; l++) {
		x1 = loc_or[l].x;	x2 = loc_or[l+1].x;
		y1 = loc_or[l].y;	y2 = loc_or[l+1].y;
		dx = x2 - x1;	dy = y2 - y1;
		r = sqrt(dx*dx + dy*dy);
		if (r > FLT_EPSILON) {
			c_psi = dx / r;		s_psi = dy / r;
			grv_p = eq_30(c_psi, s_psi, x2, y2, loc_or[l+1].z) - eq_30(c_psi, s_psi, x1, y1, loc_or[l].z);
		}
		else
			grv_p = 0;
		grv += grv_p;
	}
	return (grv * c_phi);
}

/* ---------------------------------------------------------------------- */
GMT_LOCAL double eq_30 (double c, double s, double x, double y, double z) {
	double r, Ji = 0, log_arg;

	r = sqrt(x * x + y * y + z * z);
	if (r > FLT_EPSILON) {
		if (fabs(z) > FLT_EPSILON && fabs(c) > FLT_EPSILON)
			Ji = -2. * z * atan ((x * c + (s + 1) * (y + r)) / (z * c));
		log_arg = x * c + y * s + r;
		if (log_arg > FLT_EPSILON)
			Ji += (x * s - y * c) * log(log_arg);
	}
	return Ji;
}

/* ---------------------------------------------------------------------- */
GMT_LOCAL double okb_mag (unsigned int n_vert, unsigned int km, unsigned int pm, struct LOC_OR *loc_or,
	                      double c_tet, double s_tet, double c_phi, double s_phi, struct MAG_PARAM *okabe_mag_param, struct MAG_VAR *okabe_mag_var) {
/*  Computes the total magnetic anomaly due to a facet. */

	unsigned int i;
	double qsi1, qsi2, eta1, eta2, z2, z1, dx, dy, kx, ky, kz, v, r, c_psi, s_psi;
	double ano = 0, ano_p, mag_fac, xi, xi1, yi, yi1, mx, my, mz, tg_psi, auxil;

	mag_fac = s_phi * (okabe_mag_param[pm].rim[0] * c_tet + okabe_mag_param[pm].rim[1] * s_tet) +
	          c_phi * okabe_mag_param[pm].rim[2];

	if (fabs(mag_fac) < FLT_EPSILON) return 0.0;

	kx = okabe_mag_var[km].rk[0];	ky = okabe_mag_var[km].rk[1];	kz = okabe_mag_var[km].rk[2];
	v  = kx * c_tet + ky * s_tet;
	mx = v * c_phi - kz * s_phi;	my = ky * c_tet - kx * s_tet;	mz = v * s_phi + kz * c_phi;

	for (i = 0; i < n_vert; i++) {
		xi = loc_or[i].x;	xi1 = loc_or[i+1].x;
		yi = loc_or[i].y;	yi1 = loc_or[i+1].y;
		dx = xi1 - xi;	dy = yi1 - yi;
		r  = sqrt(dx*dx + dy*dy);
		if (r > FLT_EPSILON) {
			c_psi  = dx / r;		s_psi = dy / r;
			tg_psi = dy / dx;
			auxil  = my * c_psi - mx * s_psi;
			qsi1   = yi * s_psi + xi * c_psi;	qsi2 = yi1 * s_psi + xi1 * c_psi;
			eta1   = yi * c_psi - xi * s_psi;	eta2 = yi1 * c_psi - xi1 * s_psi;
			z1     = loc_or[i].z;	z2 = loc_or[i+1].z;
			ano_p  = eq_43(mz, c_psi, tg_psi, auxil, qsi2, eta2, z2) -
			         eq_43(mz, c_psi, tg_psi, auxil, qsi1, eta1, z1);
		}
		else
			ano_p = 0;
		ano += ano_p;
	}
	return (ano * mag_fac);
}

/* ---------------------------------------------------------------------- */
GMT_LOCAL double eq_43 (double mz, double c, double tg, double auxil, double x, double y, double z) {
	double r, ez, Li = 0, tmp;

	ez = y * y + z * z;
	r = sqrt(x * x + ez);

	if (r > FLT_EPSILON) {
		if (fabs(z) > FLT_EPSILON && fabs(c) > FLT_EPSILON)
			Li = mz * atan((ez * tg - x * y) / (z * r));
		else
			Li = 0.0;
		tmp = x + r;
		if (tmp <= 0.)
			Li -= log(r - x) * auxil;
		else
			Li += log(tmp) * auxil;
	}
	return Li;
}
