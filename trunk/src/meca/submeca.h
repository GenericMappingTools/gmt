/*	$Id: submeca.h,v 1.11 2011-03-15 02:06:37 guru Exp $
 *    Copyright (c) 1996-2011 by G. Patau
 *    Distributed under the GNU Public Licence
 *    See README file for copying and redistribution conditions.
 */

void rot_axis(struct AXIS A,struct nodal_plane PREF,struct AXIS *Ar);
void rot_tensor(struct M_TENSOR mt,struct nodal_plane PREF,struct M_TENSOR *mtr);
void rot_meca(st_me meca,struct nodal_plane PREF,st_me *mecar);
void rot_nodal_plane(struct nodal_plane PLAN,struct nodal_plane PREF,struct nodal_plane *PLANR);
GMT_LONG gutm(double lon ,double lat ,double *xutm ,double *yutm,GMT_LONG fuseau);
GMT_LONG dans_coupe(double lon,double lat,double depth,double xlonref,double ylatref,GMT_LONG fuseau,double str,double dip,double p_length,double p_width,double *distance,double *n_dep);
