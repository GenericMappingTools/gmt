/*--------------------------------------------------------------------
 *
 *  Copyright (c) 2013-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *  See LICENSE.TXT file for copying and redistribution conditions.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 3 or any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 *  Copyright (c) 1996-2012 by G. Patau
 *  Donated to the GMT project by G. Patau upon her retirement from IGPG
 *--------------------------------------------------------------------*/

/*!
 * \file utilmeca.h
 */

unsigned int meca_line_parse (struct GMT_CTRL *GMT, struct SEIS_OFFSET_LINE *L, char option, char *arg);
void meca_line_usage (struct GMTAPI_CTRL *API, char option);
void meca_get_trans (struct GMT_CTRL *GMT, double slon, double slat, double *t11, double *t12, double *t21, double *t22);
double meca_ps_mechanism (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, st_me meca, double size, struct GMT_FILL *F, struct GMT_FILL *E, int outline);
double meca_ps_plan (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, st_me meca, double size, int num_of_plane);
double meca_computed_mw(struct SEIS_MOMENT moment, double ms);
double meca_computed_dip2(double str1, double dip1, double str2);
double meca_computed_rake2(double str1, double dip1, double str2, double dip2, double fault);
void meca_define_second_plane(struct SEIS_NODAL_PLANE NP, struct SEIS_NODAL_PLANE *NP2);
double meca_ps_tensor (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, double size, struct SEIS_AXIS T, struct SEIS_AXIS N, struct SEIS_AXIS P, struct GMT_FILL *C, struct GMT_FILL *E, int outline, int plot_zerotrace, int recno);
void meca_axe2dc(struct SEIS_AXIS T, struct SEIS_AXIS P, struct SEIS_NODAL_PLANE *NP1, struct SEIS_NODAL_PLANE *NP2);
void meca_dc2axe (st_me meca, struct SEIS_AXIS *T, struct SEIS_AXIS *N, struct SEIS_AXIS *P);
void ps_pt_axis(double x0, double y0, st_me meca, double size, double *pp, double *dp, double *pt, double *dt, double *xp, double *yp, double *xt, double *yt);
void meca_moment2axe(struct GMT_CTRL *GMT, struct SEIS_M_TENSOR mt, struct SEIS_AXIS *T, struct SEIS_AXIS *N, struct SEIS_AXIS *P);
void meca_axis2xy(double x0, double y0, double size, double pp, double dp, double pt, double dt, double *xp, double *yp, double *xt, double *yt);
double meca_zero_360 (double str);
