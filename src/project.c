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
/*
 * Brief synopsis: project.c reads (x,y,[z]) data and writes some combination of (x,y,z,p,q,u,v),
 * where p,q is the distance along,across the track of the projection of (x,y),
 * and u,v are the un-transformed (x,y) coordinates of the projected position.
 * Can also create (x,y) along track.
 *
 * Author: 	Walter H. F. Smith
 * Date:	1 JAN 2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"project"
#define THIS_MODULE_MODERN_NAME	"project"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Project data onto lines or great circles, or generate tracks"
#define THIS_MODULE_KEYS	"<D{,>D},G-("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vbdefghios" GMT_OPT("HMm")

#define PROJECT_N_FARGS	7

struct PROJECT_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -A<azimuth> */
		bool active;
		double azimuth;
	} A;
	struct C {	/* -C<ox>/<oy> */
		bool active;
		double x, y;
	} C;
	struct E {	/* -E<bx>/<by> */
		bool active;
		double x, y;
	} E;
	struct F {	/* -F<flags> */
		bool active;
		char col[PROJECT_N_FARGS];	/* Character codes for desired output in the right order */
	} F;
	struct G {	/* -G<inc>[/<colat>][+h] */
		bool active;
		bool header;
		unsigned int mode;
		double inc;
		double colat;
	} G;
	struct L {	/* -L[w][<l_min>/<l_max>] */
		bool active;
		bool constrain;
		double min, max;
	} L;
	struct N {	/* -N */
		bool active;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
	struct S {	/* -S */
		bool active;
	} S;
	struct T {	/* -T<px>/<py> */
		bool active;
		double x, y;
	} T;
	struct W {	/* -W<w_min>/<w_max> */
		bool active;
		double min, max;
	} W;
	struct Z {	/* -Z<major/minor/azimuth>[+e] */
		bool active;
		bool exact;
		double major, minor, azimuth;
	} Z;
};

struct PROJECT_DATA {
	double a[6];	/* xypqrs */
	double *z;	/* all z's */
	char *t;	/* Text endings of each record */
};

struct PROJECT_INFO {
	uint64_t n_used;
	uint64_t n_outputs;
	uint64_t n_z;
	int output_choice[PROJECT_N_FARGS];
	bool find_new_point;
	bool want_z_output;
	bool first;
	double pole[3];
	double plon, plat;	/* Pole location */
};

GMT_LOCAL int compare_distances (const void *point_1, const void *point_2) {
	double d_1, d_2;

	d_1 = ((struct PROJECT_DATA *)point_1)->a[2];
	d_2 = ((struct PROJECT_DATA *)point_2)->a[2];

	if (d_1 < d_2) return (-1);
	if (d_1 > d_2) return (1);
	return (0);
}

GMT_LOCAL double oblique_setup (struct GMT_CTRL *GMT, double plat, double plon, double *p, double *clat, double *clon, double *c, bool c_given, bool generate) {
	/* routine sets up a unit 3-vector p, the pole of an
	   oblique projection, given plat, plon, the position
	   of this pole in the usual coordinate frame.
	   c_given = true means that clat, clon are to be used
	   as the usual coordinates of a point through which the
	   user wants the central meridian of the oblique
	   projection to go.  If such a point is not given, then
	   the central meridian will go through p and the usual
	   N pole.  In either case, a unit 3-vector c is created
	   which is the directed normal to the plane of the central
	   meridian, pointing in the positive normal (east) sense.
	   Latitudes and longitudes are in degrees. */

	double s[3];  /* s points to the south pole  */
	double	x[3];  /* tmp vector  */
	double cp, sin_lat_to_pole;

	s[0] = s[1] = 0.0;	s[2] = -1.0;

	gmt_geo_to_cart (GMT, plat, plon, p, true);

	if (c_given) gmt_geo_to_cart (GMT, *clat, *clon, s, true);	/* s points to user's clat, clon  */
	gmt_cross3v (GMT, p, s, x);
	gmt_normalize3v (GMT, x);
	gmt_cross3v (GMT, x, p, c);
	gmt_normalize3v (GMT, c);
	if (!generate) gmt_M_memcpy (c, x, 3, double);
	if (!c_given) gmt_cart_to_geo (GMT, clat, clon, c, true);	/* return the adjusted center  */
	cp = gmt_dot3v (GMT, p, c);
	sin_lat_to_pole = d_sqrt (1.0 - cp * cp);
	return (sin_lat_to_pole);
}

GMT_LOCAL void oblique_transform (struct GMT_CTRL *GMT, double xlat, double xlon, double *x_t_lat, double *x_t_lon, double *p, double *c) {
	/* routine takes the point x at conventional (xlat, xlon) and
	   computes the transformed coordinates (x_t_lat, x_t_lon) in
	   an oblique reference frame specified by the unit 3-vectors
	   p (the pole) and c (the directed normal to the oblique
	   central meridian).  p and c have been computed earlier by
	   the routine oblique_setup().
	   Latitudes and longitudes are in degrees. */

	double x[3], p_cross_x[3], temp1, temp2;

	gmt_geo_to_cart (GMT, xlat, xlon, x, true);

	temp1 = gmt_dot3v (GMT, x,p);
	*x_t_lat = d_asind(temp1);

	gmt_cross3v (GMT, p,x,p_cross_x);
	gmt_normalize3v (GMT, p_cross_x);

	temp1 = gmt_dot3v (GMT, p_cross_x, c);
	temp2 = gmt_dot3v (GMT, x, c);
	*x_t_lon = copysign(d_acosd(temp1), temp2);
}

GMT_LOCAL void make_euler_matrix (double *p, double *e, double theta) {
	/* Routine to fill an euler matrix e with the elements
	   needed to rotate a 3-vector about the pole p through
	   an angle theta (in degrees).  p is a unit 3-vector.
	   Latitudes and longitudes are in degrees. */

	double cos_theta, sin_theta, one_minus_cos_theta;
	double pxsin, pysin, pzsin, temp;

	sincosd (theta, &sin_theta, &cos_theta);
	one_minus_cos_theta = 1.0 - cos_theta;

	pxsin = p[0] * sin_theta;
	pysin = p[1] * sin_theta;
	pzsin = p[2] * sin_theta;

	temp = p[0] * one_minus_cos_theta;
	e[0] = temp * p[0] + cos_theta;
	e[1] = temp * p[1] - pzsin;
	e[2] = temp * p[2] + pysin;

	temp = p[1] * one_minus_cos_theta;
	e[3] = temp * p[0] + pzsin;
	e[4] = temp * p[1] + cos_theta;
	e[5] = temp * p[2] - pxsin;

	temp = p[2] * one_minus_cos_theta;
	e[6] = temp * p[0] - pysin;
	e[7] = temp * p[1] + pxsin;
	e[8] = temp * p[2] + cos_theta;
}

GMT_LOCAL void matrix_3v (double *a, double *x, double *b) {
	/* routine to find b, where Ax = b, A is a 3 by 3 square matrix,
	   and x and b are 3-vectors.  A is stored row wise, that is:

	   A = { a11, a12, a13, a21, a22, a23, a31, a32, a33 }  */

	b[0] = x[0]*a[0] + x[1]*a[1] + x[2]*a[2];
	b[1] = x[0]*a[3] + x[1]*a[4] + x[2]*a[5];
	b[2] = x[0]*a[6] + x[1]*a[7] + x[2]*a[8];
}

GMT_LOCAL void matrix_2v (double *a, double *x, double *b) {
	/* routine to find b, where Ax = b, A is a 2 by 2 square matrix,
	   and x and b are 2-vectors.  A is stored row wise, that is:

	   A = { a11, a12, a21, a22 }  */

	b[0] = x[0]*a[0] + x[1]*a[1];
	b[1] = x[0]*a[2] + x[1]*a[3];
}

GMT_LOCAL void sphere_project_setup (struct GMT_CTRL *GMT, double alat, double alon, double *a, double blat, double blon, double *b, double azim, double *p, double *c, bool two_pts) {
	/* routine to initialize a pole vector, p, and a central meridian
	   normal vector, c, for use in projecting points onto a great circle.

	   The great circle is specified in either one of two ways:
	   if (two_pts), then the user has given two points, a and b,
	   which specify the great circle (directed from a to b);
	   if !(two_pts), then the user has given one point, a, and an azimuth,
	   azim, clockwise from north, which defines the projection.

	   The strategy is to use the oblique_transform operations above,
	   in such a way that the great circle of the projection is the
	   equator of an oblique transform, and the central meridian goes
	   through a.  Then the transformed longitude gives the distance
	   along the projection circle, and the transformed latitude gives
	   the distance normal to the projection circle.

	   If (two_pts), then p = normalized(a X b).  If not, we temporarily
	   create p_temp = normalized(a X n), where n is the north pole.
	   p_temp is then rotated about a through the angle azim to give p.
	   After p is found, then c = normalized(p X a).

	   Latitudes and longitudes are in degrees.
	*/

	double e[9];	/* Euler rotation matrix, if needed  */

	/* First find p vector  */

	if (two_pts) {
		gmt_geo_to_cart (GMT, alat, alon, a, true);
		gmt_geo_to_cart (GMT, blat, blon, b, true);
		gmt_cross3v (GMT, a, b, p);
		gmt_normalize3v (GMT, p);
	}
	else {
		gmt_geo_to_cart (GMT, alat, alon, a, true);
		b[0] = b[1] = 0.0;	/* set b to north pole  */
		b[2] = 1.0;
		gmt_cross3v (GMT, a, b, c);	/* use c for p_temp  */
		gmt_normalize3v (GMT, c);
		make_euler_matrix(a, e, -azim);
		matrix_3v(e, c, p);	/* c (p_temp) rotates to p  */
	}

	/* Now set c vector  */

	gmt_cross3v (GMT, p, a, c);
	gmt_normalize3v (GMT, c);
}

GMT_LOCAL void flat_project_setup (double alat, double alon, double blat, double blon, double plat, double plon, double *azim, double *e, bool two_pts, bool pole_set) {
	/* Sets up stuff for rotation of Cartesian 2-vectors, analogous
	   to the spherical three vector stuff above.
	   Output is the negative Cartesian azimuth in degrees.
	   Latitudes and longitudes are in degrees. */

	if (two_pts)
		*azim = 90.0 - d_atan2d (blat - alat, blon - alon);
	else if (pole_set)
		*azim = 180.0 - d_atan2d (plat - alat, plon - alon);

	*azim = -(*azim);
	e[0] = e[3] = cosd (*azim);
	e[1] = sind (*azim);
	e[2] = -e[1];
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PROJECT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1U, struct PROJECT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->G.colat = 90.0;	/* Great circle path */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PROJECT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -C<ox>/<oy> [-A<azimuth>] [-E<bx>/<by>] [-F<flags>] [-G<dist>[/<colat>][+h]]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-L[w|<l_min>/<l_max>]] [-N] [-Q] [-S] [-T<px>/<py>] [%s] [-W<w_min>/<w_max>] [-Z<major>/<minor>/<azimuth>[+e]]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
		GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tproject will read stdin or file, and does not want input if -G option.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tThe projection may be defined in (only) one of three ways:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (1) by a center -C and an azimuth -A,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (2) by a center -C and end point of the path -E,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (3) by a center -C and a roTation pole position -T.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   In a spherical projection [default], all cases place the central meridian\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of the transformed coordinates (p,q) through -C (p = 0 at -C).  The equator\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of the (p,q) system (line q = 0) passes through -C and makes an angle\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <azimuth> with North (case 1), or passes through -E (case 2), or is\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   determined by the pole -T (case 3).  In (3), point -C need not be on equator.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   In a Cartesian [-N option] projection, p = q = 0 at -O in all cases;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (1) and (2) orient the p axis, while (3) orients the q axis.\n\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Set the location of the center to be <ox>/<oy>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Set the option (1) Azimuth, (<azimuth> in degrees CW from North).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Force the location of the Discontinuity in the r coordinate;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Dd (dateline) means [-180 < r < 180], -Dg (greenwich) means [0 < r < 360].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The default does not check; in spherical case this usually results in [-180,180].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set the option (2) location of end point E to be <bx>/<by>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Indicate what output you want as one or more of xyzpqrs in any order;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   where x,y,[z] refer to input data locations and optional values,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p,q are the coordinates of x,y in the projection's coordinate system,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r,s is the projected position of x,y (taking q = 0) in the (x,y) coordinate system.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p,q may be scaled from degrees into kilometers by the -Q option.  See -L, -Q, -W.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note z refers to all input data columns beyond the required x,y\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is all fields, i.e., -Fxyzpqrs].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -G is set, -F is not available and output defaults to rsp.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Generate (r,s,p) points along profile every <dist> units. (No input data used.)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If E given, will generate from C to E; else must give -L<l_min>/<l_max> for length.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append /<colat> for a small circle path through C and E (requires -C -E) [90].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Finally, append +h if you want information about the pole in a segment header [no header].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Check the Length along the projected track and use only certain points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Lw will use only those points Within the span from C to E (Must have set -E).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -L<l_min>/<l_max> will only use points whose p is [l_min <= p <= l_max].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default uses all points.  Note p = 0 at C and increases toward E in <azimuth> direction.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Flat Earth mode; a Cartesian projection is made.  Default is spherical.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Convert to Map units, so x,y,r,s are degrees,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   while p,q,dist,l_min,l_max,w_min,w_max are km.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If not set, then p,q,dist,l_min,l_max,w_min,w_max are assumed to be in same units as x,y,r,s.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Output should be Sorted into increasing p value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set the option (3) location of the roTation pole to the projection to be <px>/<py>.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Check the width across the projected track and use only certain points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This will use only those points whose q is [w_min <= q <= w_max].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note that q is positive to your LEFT as you walk from C toward E in <azimuth> direction.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z With -G and -C, generate an ellipse of given major and minor axes (in km if geographic) and azimuth\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of major axis. Append +e for adjusting increment to fix perimeter exactly [use increment as given in -G].\n");
	GMT_Option (API, "bi2,bo,d,e,f,g,h,i,o,s,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PROJECT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to project and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, j, k;
	size_t len;
	char txt_a[GMT_LEN64] = {""}, txt_b[GMT_LEN64] = {""}, *ce = NULL, *ch = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) if (opt->option == 'N') Ctrl->N.active = true;	/* Must find -N first, if given */

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files are OK */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				Ctrl->A.azimuth = atof (opt->arg);
				break;
			case 'C':
				Ctrl->C.active = true;
				if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) != 2) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -C<lon0>/<lat0>\n");
					n_errors++;
				}
				else {
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X), gmt_scanf_arg (GMT, txt_a, gmt_M_type (GMT, GMT_IN, GMT_X), false, &Ctrl->C.x), txt_a);
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y), gmt_scanf_arg (GMT, txt_b, gmt_M_type (GMT, GMT_IN, GMT_Y), false, &Ctrl->C.y), txt_b);
					if (n_errors) GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Undecipherable argument %s\n", opt->arg);
				}
				break;
			case 'D':
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -D is deprecated; use --FORMAT_GEO_OUT instead\n");
					if (opt->arg[0] == 'g') GMT->current.io.geo.range = GMT_IS_0_TO_P360_RANGE;
					if (opt->arg[0] == 'd') GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'E':
				Ctrl->E.active = true;
				if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) != 2) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -E<lon1>/<lat1>\n");
					n_errors++;
				}
				else {
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X), gmt_scanf_arg (GMT, txt_a, gmt_M_type (GMT, GMT_IN, GMT_X), false, &Ctrl->E.x), txt_a);
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y), gmt_scanf_arg (GMT, txt_b, gmt_M_type (GMT, GMT_IN, GMT_Y), false, &Ctrl->E.y), txt_b);
					if (n_errors) GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -E option: Undecipherable argument %s\n", opt->arg);
				}
				break;
			case 'F':
				Ctrl->F.active = true;
				for (j = 0, k = 0; opt->arg[j]; j++, k++) {
					if (k < PROJECT_N_FARGS) {
						Ctrl->F.col[k] = opt->arg[j];
						if (!strchr ("xyzpqrs", opt->arg[j])) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: Choose from -Fxyzpqrs\n");
							n_errors++;
						}
					}
					else {
						n_errors++;
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: Too many output columns selected\n");
					}
				}
				break;
			case 'G':
				Ctrl->G.active = true;
				if ((ch = strstr (opt->arg, "+h"))) {
					Ctrl->G.header = true;
					ch[0] = '\0';	/* Chop off +h */
				}
				len = strlen (opt->arg) - 1;
				if (len > 0 && opt->arg[len] == '+') {	/* Obsolete way to say +h */
					Ctrl->G.header = true;	/* Wish to place a segment header on output */
					opt->arg[len] = 0;	/* Temporarily remove the trailing + sign */
					ch = &opt->arg[len];
				}
				if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) == 2) {	/* Got dist/colat */
					Ctrl->G.mode = 1;
					Ctrl->G.inc = atof (txt_a);
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y), gmt_scanf_arg (GMT, txt_b, gmt_M_type (GMT, GMT_IN, GMT_Y), false, &Ctrl->G.colat), txt_b);
				}
				else
					Ctrl->G.inc = atof (opt->arg);
				if (ch) ch[0] = '+';	/* Restore the plus-sign */
				break;
			case 'L':
				Ctrl->L.active = true;
				if (opt->arg[0] == 'W' || opt->arg[0] == 'w')
					Ctrl->L.constrain = true;
				else {
					n_errors += gmt_M_check_condition (GMT, sscanf(opt->arg, "%lf/%lf", &Ctrl->L.min, &Ctrl->L.max) != 2, "Syntax error: Expected -L[w | <min>/<max>]\n");
				}
				break;
			case 'N': /* Handled above but still in argv */
				Ctrl->N.active = true;
				break;
			case 'Q':
				Ctrl->Q.active = true;
				break;
			case 'S':
				Ctrl->S.active = true;
				break;
			case 'T':
				Ctrl->T.active = true;
				if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) != 2) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -T<lonp>/<latp>\n");
					n_errors++;
				}
				else {
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X), gmt_scanf_arg (GMT, txt_a, gmt_M_type (GMT, GMT_IN, GMT_X), false, &Ctrl->T.x), txt_a);
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y), gmt_scanf_arg (GMT, txt_b, gmt_M_type (GMT, GMT_IN, GMT_Y), false, &Ctrl->T.y), txt_b);
					if (n_errors) GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Undecipherable argument %s\n", opt->arg);
				}
				break;
			case 'W':
				Ctrl->W.active = true;
				n_errors += gmt_M_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->W.min, &Ctrl->W.max) != 2,
				                                 "Syntax error: Expected -W<min>/<max>\n");
				break;
			case 'Z': /* Parameters of ellipse */
				Ctrl->Z.active = true;
				if ((ce = strstr (opt->arg, "+e"))) {
					Ctrl->Z.exact = true;
					ce[0] = '\0';	/* Chop off +e */
				}
				n_errors += gmt_M_check_condition (GMT, sscanf(opt->arg, "%lf/%lf/%lf", &Ctrl->Z.major, &Ctrl->Z.minor, &Ctrl->Z.azimuth) != 3, "Syntax error: Expected -Z<major/minor/azimuth>\n");
				if (ce) ce[0] = '+';	/* Restore the plus-sign */
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (!Ctrl->N.active && ((Ctrl->C.active && (Ctrl->C.x < -360 || Ctrl->C.x > 360) && (Ctrl->C.y < -90 || Ctrl->C.y > 90)) || (Ctrl->E.active && (Ctrl->E.x < -360 || Ctrl->E.x > 360) && (Ctrl->E.y < -90 || Ctrl->E.y > 90)))) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Your -C or -E options suggest Cartesian coordinates.  Please see -N\n");
		n_errors++;
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && !Ctrl->L.constrain && Ctrl->L.min >= Ctrl->L.max,
	                                 "Syntax error -L option: w_min must be < w_max\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.min >= Ctrl->W.max,
	                                 "Syntax error -W option: w_min must be < w_max\n");
	n_errors += gmt_M_check_condition (GMT, (Ctrl->A.active + Ctrl->E.active + Ctrl->T.active) > 1,
	                                 "Syntax error: Specify only one of -A, -E, and -T\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && (Ctrl->C.x == Ctrl->E.x) && (Ctrl->C.y == Ctrl->E.y),
	                                 "Syntax error -E option: Second point must differ from origin!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && Ctrl->L.min == Ctrl->L.max && !(Ctrl->E.active || Ctrl->Z.active),
	                                 "Syntax error -G option: Must also specify -Lmin/max or use -E instead\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && Ctrl->F.active,
	                                 "Syntax error -G option: -F not allowed [Defaults to rsp]\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && Ctrl->G.inc <= 0.0,
	                                 "Syntax error -G option: Must specify a positive increment\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.constrain && !Ctrl->E.active,
	                                 "Syntax error -L option: Must specify -Lmin/max or use -E instead\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && (gmt_M_is_geographic (GMT, GMT_IN) || gmt_M_is_geographic (GMT, GMT_OUT)),
	                                 "Syntax error -N option: Cannot be used with -fg\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && Ctrl->G.mode,
	                                 "Syntax error -N option: Cannot be used with -G<dist>/<colat>\n");
	n_errors += gmt_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int write_one_segment (struct GMT_CTRL *GMT, struct PROJECT_CTRL *Ctrl, double theta, struct PROJECT_DATA *p_data, struct PROJECT_INFO *P) {
	uint64_t col, n_items, rec, k;
	double sin_theta, cos_theta, e[9], x[3], xt[3], *out = NULL;
	struct GMT_RECORD *Out = NULL;

	if (Ctrl->S.active) qsort (p_data, P->n_used, sizeof (struct PROJECT_DATA), compare_distances);

	/* Get here when all data are loaded with p,q and p is in increasing order if desired. */

	if (P->find_new_point) {	/* We need to find r,s  */

		if (Ctrl->N.active) {
			sincosd (theta, &sin_theta, &cos_theta);
			for (rec = 0; rec < P->n_used; rec++) {
				p_data[rec].a[4] = Ctrl->C.x + p_data[rec].a[2] * cos_theta;
				p_data[rec].a[5] = Ctrl->C.y + p_data[rec].a[2] * sin_theta;
			}
		}
		else {
			gmt_geo_to_cart (GMT, Ctrl->C.y, Ctrl->C.x, x, true);
			for (rec = 0; rec < P->n_used; rec++) {
				make_euler_matrix (P->pole, e, p_data[rec].a[2]);
				matrix_3v (e,x,xt);
				gmt_cart_to_geo (GMT, &(p_data[rec].a[5]), &(p_data[rec].a[4]), xt, true);
			}
		}
	}

	/* At this stage, all values are still in degrees.  */

	if (Ctrl->Q.active) {
		for (rec = 0; rec < P->n_used; rec++) {
			p_data[rec].a[2] *= GMT->current.proj.DIST_KM_PR_DEG;
			p_data[rec].a[3] *= GMT->current.proj.DIST_KM_PR_DEG;
		}
	}

	n_items = gmt_get_cols (GMT, GMT_OUT);
	out = gmt_M_memory (GMT, NULL, n_items, double);
	Out = gmt_new_record (GMT, out, NULL);	/* text will be set below */

	/* Now output this segment */

	for (rec = 0; rec < P->n_used; rec++) {
		for (col = k = 0; col < P->n_outputs; col++) {
			if (P->output_choice[col] == -1) {	/* Copy over all z columns */
				gmt_M_memcpy (&out[k], p_data[rec].z, P->n_z, double);
				k += P->n_z;
			}
			else
				out[k++] = p_data[rec].a[P->output_choice[col]];
		}
		Out->text = p_data[rec].t;	/* The trailing text (may be NULL) */
		GMT_Put_Record (GMT->parent, GMT_WRITE_DATA, Out);	/* Write this to output */
	}
	gmt_M_free (GMT, out);
	gmt_M_free (GMT, Out);
	return (0);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_project (void *V_API, int mode, void *args) {
	uint64_t rec, n_total_read, col, n_total_used = 0;
	bool skip, z_first = true, z_set_auto = false;
	int error = 0;

	size_t n_alloc = GMT_CHUNK;

	double xx, yy, cos_theta, sin_theta, sin_lat_to_pole = 1.0;
	double theta = 0.0, d_along, sign = 1.0, s, c, *in = NULL;
	double a[3], b[3], x[3], xt[3], center[3], e[9];

	struct PROJECT_DATA *p_data = NULL;
	struct PROJECT_INFO P;
	struct PROJECT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the project main code ----------------------------*/

	gmt_M_memset (&P, 1, struct PROJECT_INFO);
	gmt_M_memset (a, 3, double);
	gmt_M_memset (b, 3, double);
	gmt_M_memset (x, 3, double);
	gmt_M_memset (xt, 3, double);
	gmt_M_memset (center, 3, double);
	gmt_M_memset (e, 9, double);
	P.first = true;
	if (Ctrl->N.active) {	/* Must undo an optional -fg that was set before */
		gmt_set_cartesian (GMT, GMT_IN);
		gmt_set_cartesian (GMT, GMT_OUT);
	}
	else {	/* Make sure we set -fg */
		gmt_set_geographic (GMT, GMT_IN);
		gmt_set_geographic (GMT, GMT_OUT);
	}

	/* Convert user's -F choices to internal parameters */
	for (col = P.n_outputs = 0; col < PROJECT_N_FARGS && Ctrl->F.col[col]; col++) {
		switch (Ctrl->F.col[col]) {
			case 'z':	/* Special flag, can mean any number of z columns and trailing text */
				P.output_choice[col] = -1;
				P.want_z_output = true;
				break;
			case 'x':
				P.output_choice[col] = 0;
				break;
			case 'y':
				P.output_choice[col] = 1;
				break;
			case 'p':
				P.output_choice[col] = 2;
				break;
			case 'q':
				P.output_choice[col] = 3;
				break;
			case 'r':
				P.output_choice[col] = 4;
				P.find_new_point = true;
				break;
			case 's':
				P.output_choice[col] = 5;
				P.find_new_point = true;
				break;
			default:	/* Already checked in parser that this cannot happen */
				break;
		}
		P.n_outputs++;
	}

	if (P.n_outputs == 0 && !Ctrl->G.active) {	/* Generate default -F setting (xyzpqrs) */
		P.n_outputs = PROJECT_N_FARGS;
		for (col = 0; col < 2; col++) P.output_choice[col] = (int)col;	/* Do xy */
		P.output_choice[2] = -1;	/* Do z as col 2 */
		P.want_z_output = z_set_auto = true;
		for (col = 3; col < P.n_outputs; col++) P.output_choice[col] = (int)col - 1;	/* Do pqrs */
		P.find_new_point = true;
	}
	if (Ctrl->G.active) {	/* Hardwire 3 output columns and set their types */
		P.n_outputs = 3;
		gmt_set_column (GMT, GMT_OUT, GMT_X, (Ctrl->N.active) ? GMT_IS_FLOAT : GMT_IS_LON);
		gmt_set_column (GMT, GMT_OUT, GMT_Y, (Ctrl->N.active) ? GMT_IS_FLOAT : GMT_IS_LAT);
		gmt_set_column (GMT, GMT_OUT, GMT_Z, GMT_IS_FLOAT);
	}
	else {	/* Decode and set the various output column types */
		for (col = 0; col < P.n_outputs; col++) {
			switch (P.output_choice[col]) {
				case 0: case 4: gmt_set_column (GMT, GMT_OUT, (unsigned int)col, GMT_IS_LON);		break;
				case 1: case 5: gmt_set_column (GMT, GMT_OUT, (unsigned int)col, GMT_IS_LAT);		break;
				default: 	    gmt_set_column (GMT, GMT_OUT, (unsigned int)col, GMT_IS_FLOAT);	break;
			}
		}
	}
	p_data = gmt_M_memory (GMT, NULL, n_alloc, struct PROJECT_DATA);

	if (Ctrl->G.active && Ctrl->E.active && (Ctrl->L.min == Ctrl->L.max)) Ctrl->L.constrain = true;	/* Default generate from A to B  */
	
	/* Set up rotation matrix e for flat earth, or pole and center for spherical; get Ctrl->L.min, Ctrl->L.max if stay_within  */

	if (Ctrl->N.active) {	/* Flat Earth mode */
		theta = Ctrl->A.azimuth;
		flat_project_setup (Ctrl->C.y, Ctrl->C.x, Ctrl->E.y, Ctrl->E.x, Ctrl->T.y, Ctrl->T.x, &theta, e, Ctrl->E.active, Ctrl->T.active);
		/* Azimuth (theta) is now Cartesian in degrees */
		if (Ctrl->L.constrain) {
			Ctrl->L.min = 0.0;
			xx = Ctrl->E.x - Ctrl->C.x;
			yy = Ctrl->E.y - Ctrl->C.y;
			Ctrl->L.max = hypot (xx, yy);
			if (Ctrl->Q.active) Ctrl->L.max *= GMT->current.proj.DIST_KM_PR_DEG;
		}
	}
	else {	/* Spherical Earth mode */
		if (Ctrl->T.active) {	/* Gave the pole */
			sin_lat_to_pole = oblique_setup (GMT, Ctrl->T.y, Ctrl->T.x, P.pole, &Ctrl->C.y, &Ctrl->C.x, center, Ctrl->C.active, Ctrl->G.active);
			gmt_cart_to_geo (GMT, &P.plat, &P.plon, x, true);	/* Save lon, lat of the pole */
		}
		else {	/* Using -C, -E or -A */
			double s_hi, s_lo, s_mid, radius, m[3], ap[3], bp[3];
			int done, n_iter = 0;
			
			sphere_project_setup (GMT, Ctrl->C.y, Ctrl->C.x, a, Ctrl->E.y, Ctrl->E.x, b, Ctrl->A.azimuth, P.pole, center, Ctrl->E.active);
			gmt_cart_to_geo (GMT, &P.plat, &P.plon, x, true);	/* Save lon, lat of the pole */
			radius = 0.5 * d_acosd (gmt_dot3v (GMT, a, b)); 
			if (radius > fabs (Ctrl->G.colat)) {
				GMT_Report (API, GMT_MSG_NORMAL, "Center [-C] and end point [-E] are too far apart (%g) to define a small-circle with colatitude %g. Revert to great-circle.\n", radius, Ctrl->G.colat);
				Ctrl->G.mode = 0;
			}
			else if (doubleAlmostEqual (Ctrl->G.colat, 90.0)) {	/* Great circle pole needed */
				if (Ctrl->L.constrain) Ctrl->L.max = d_acosd (gmt_dot3v (GMT, a, b));
			}
			else {	/* Find small circle pole so C and E are |lat| degrees from it. */
				for (col = 0; col < 3; col++) m[col] = a[col] + b[col];	/* Mid point along A-B */
				gmt_normalize3v (GMT, m);
				sign = copysign (1.0, Ctrl->G.colat);
				s_hi = 90.0;	s_lo = 0.0;
				done = false;
				do {	/* Trial for finding pole S */
					n_iter++;
					s_mid = 0.5 * (s_lo + s_hi);
					sincosd (sign * s_mid, &s, &c);
					for (col = 0; col < 3; col++) x[col] = P.pole[col] * s + m[col] * c;
					gmt_normalize3v (GMT, x);
					radius = d_acosd (gmt_dot3v (GMT, a, x)); 
					if (fabs (radius - fabs (Ctrl->G.colat)) < GMT_CONV8_LIMIT)
						done = true;
					else if (radius > fabs (Ctrl->G.colat))
						s_hi = s_mid;
					else
						s_lo = s_mid;
					if (n_iter > 500) done = true;	/* Safety valve */
				} while (!done);
				gmt_cart_to_geo (GMT, &P.plat, &P.plon, x, true);	/* Save lon, lat of the new pole */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Pole for small circle located at %g %g\n", radius, P.plon, P.plat);
				gmt_M_memcpy (P.pole, x, 3, double);	/* Replace great circle pole with small circle pole */
				sin_lat_to_pole = s;
				gmt_cross3v (GMT, P.pole, a, x);
				gmt_normalize3v (GMT, x);
				gmt_cross3v (GMT, x, P.pole, ap);
				gmt_normalize3v (GMT, ap);
				gmt_cross3v (GMT, P.pole, b, x);
				gmt_normalize3v (GMT, x);
				gmt_cross3v (GMT, x, P.pole, bp);
				gmt_normalize3v (GMT, bp);
				if (Ctrl->L.constrain) Ctrl->L.max = d_acosd (gmt_dot3v (GMT, ap, bp)) * sin_lat_to_pole;
			}
		}
		if (Ctrl->L.constrain) {
			Ctrl->L.min = 0.0;
			if (Ctrl->Q.active) Ctrl->L.max *= GMT->current.proj.DIST_KM_PR_DEG;
		}
	}

	/* Now things are initialized.  We will work in degrees for awhile, so we convert things */

	if (Ctrl->Q.active) {
		Ctrl->G.inc /= GMT->current.proj.DIST_KM_PR_DEG;
		Ctrl->L.min /= GMT->current.proj.DIST_KM_PR_DEG;
		Ctrl->L.max /= GMT->current.proj.DIST_KM_PR_DEG;
		Ctrl->W.min /= GMT->current.proj.DIST_KM_PR_DEG;
		Ctrl->W.max /= GMT->current.proj.DIST_KM_PR_DEG;
	}

	/*  Now we are ready to work  */

	P.n_used = n_total_read = 0;

	if (Ctrl->G.active) {	/* No input data expected, just generate x,y,d track from arguments given */
		double out[3];
		struct GMT_RECORD *Out = gmt_new_record (GMT, out, NULL);
		char *z_header = NULL;
		P.output_choice[0] = 4;
		P.output_choice[1] = 5;
		P.output_choice[2] = 2;
		if (Ctrl->Z.active) {	/* Do the full ellipse */
			double h = pow (Ctrl->Z.major - Ctrl->Z.minor, 2.0) / pow (Ctrl->Z.major + Ctrl->Z.minor, 2.0);
			Ctrl->L.min = 0.0;
			Ctrl->L.max = M_PI * (Ctrl->Z.major + Ctrl->Z.minor) * (1.0 + (3.0 * h)/(10.0 + sqrt (4.0 - 3.0 * h)));	/* Ramanujan approximation of ellipse circumference */
			if (Ctrl->Z.exact) {	/* Adjust inc to fit the ellipse perimeter exactly */
				double f = rint (Ctrl->L.max / Ctrl->G.inc);
				Ctrl->G.inc = Ctrl->L.max / f;
			}
		}

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Generate table data\n");
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Go from min dist = %g to max dist = %g\n", Ctrl->L.min, Ctrl->L.max);
		d_along = Ctrl->L.min;
		while ((Ctrl->L.max - d_along) > (GMT_CONV8_LIMIT*Ctrl->G.inc)) {
			p_data[P.n_used].a[2] = d_along;
			p_data[P.n_used].t = NULL;	/* Initialize since that is not done by realloc */
			p_data[P.n_used].z = NULL;	/* Initialize since that is not done by realloc */
			P.n_used++;
			d_along = Ctrl->L.min + P.n_used * Ctrl->G.inc;
			if (P.n_used == (n_alloc-1)) {
				size_t old_n_alloc = n_alloc;
				n_alloc <<= 1;
				p_data = gmt_M_memory (GMT, p_data, n_alloc, struct PROJECT_DATA);
				gmt_M_memset (&(p_data[old_n_alloc]), n_alloc - old_n_alloc, struct PROJECT_DATA);	/* Set to NULL/0 */
			}
		}
		p_data[P.n_used].a[2] = Ctrl->L.max;
		p_data[P.n_used].t = NULL;	/* Initialize since that is not done by realloc */
		p_data[P.n_used].z = NULL;	/* Initialize since that is not done by realloc */
		P.n_used ++;

		/* We need to find r,s  */

		if (Ctrl->Z.active) {
			uint64_t ne = urint (Ctrl->L.max / Ctrl->G.inc);
			if (Ctrl->N.active) {	/* Cartesian ellipse */
				double r, ca, sa, d_azim = TWO_PI / ne, e2 = 1.0 - pow (Ctrl->Z.minor/Ctrl->Z.major, 2.0);
				sincosd (Ctrl->Z.azimuth - 90.0, &sin_theta, &cos_theta);
				e[0] = e[3] = cos_theta;
				e[1] = sin_theta;
				e[2] = -e[1];
				for (rec = 0; rec < P.n_used; rec++) {
					sincos (d_azim*rec, &sa, &ca);
					r = Ctrl->Z.minor / sqrt (1.0 - e2 * pow (ca, 2.0));
					x[0] = r * ca;	x[1] = r * sa;
					matrix_2v (e, x, xt);
					p_data[rec].a[4] = Ctrl->C.x + xt[GMT_X];
					p_data[rec].a[5] = Ctrl->C.y + xt[GMT_Y];
				}
				z_header = strdup ("Testing Cartesian ellipse");
			}
			else {	/* Geographic ellipse */
				struct GMT_DATASEGMENT *S = gmt_get_geo_ellipse (GMT, Ctrl->C.x, Ctrl->C.y, Ctrl->Z.major, Ctrl->Z.minor, Ctrl->Z.azimuth, ne);
				for (rec = 0; rec < P.n_used; rec++) {
					p_data[rec].a[4] = S->data[GMT_X][rec];
					p_data[rec].a[5] = S->data[GMT_Y][rec];
				}
				z_header = strdup (S->header);
				gmt_free_segment (GMT, &S);
			}
		}
		else if (Ctrl->N.active) {
			sincosd (90.0 + theta, &sin_theta, &cos_theta);
			for (rec = 0; rec < P.n_used; rec++) {
				p_data[rec].a[4] = Ctrl->C.x + p_data[rec].a[2] * cos_theta;
				p_data[rec].a[5] = Ctrl->C.y + p_data[rec].a[2] * sin_theta;
			}
		}
		else {
			/* Must set generating vector to point along zero-meridian so it is the desired number of degrees [Ctrl->G.colat]
			 * from the pole. */
			double C[3], N[3], counteract = 1.0;
			gmt_geo_to_cart (GMT, Ctrl->C.y, Ctrl->C.x, C, true);	/* User origin C */
			gmt_cross3v (GMT, P.pole, C, N);		/* This is vector normal to meridian plan */
			gmt_normalize3v (GMT, N);			/* Make it a unit vector */
			make_euler_matrix (N, e, Ctrl->G.colat);	/* Rotation matrix about N */
			matrix_3v (e, P.pole, x);			/* This is the generating vector for our circle */
			if (Ctrl->L.constrain) counteract = 1.0 / sin_lat_to_pole;	/* Increase angle to counteract effect of small circle settings */
			for (rec = 0; rec < P.n_used; rec++) {
				make_euler_matrix (P.pole, e, sign * p_data[rec].a[2] * counteract);
				matrix_3v (e,x,xt);
				gmt_cart_to_geo (GMT, &(p_data[rec].a[5]), &(p_data[rec].a[4]), xt, true);
			}
		}

		/* At this stage, all values are still in degrees.  */

		if (Ctrl->Q.active) {
			for (rec = 0; rec < P.n_used; rec++) {
				p_data[rec].a[2] *= GMT->current.proj.DIST_KM_PR_DEG;
				p_data[rec].a[3] *= GMT->current.proj.DIST_KM_PR_DEG;
			}
		}

		/* Now output generated track */

		if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)P.n_outputs, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers data output failed */
			for (rec = 0; rec < P.n_used; rec++) {
				gmt_M_str_free (p_data[rec].t);	gmt_M_free (GMT, p_data[rec].z);
			}
			gmt_M_free (GMT, p_data);
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Failed to enable data output and set access mode */
			if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_LINE) != GMT_NOERROR) {	/* Sets output geometry */
				gmt_M_free (GMT, p_data);
				Return (API->error);
			}
			for (rec = 0; rec < P.n_used; rec++) {
				gmt_M_str_free (p_data[rec].t);	gmt_M_free (GMT, p_data[rec].z);
			}
			gmt_M_free (GMT, p_data);
			Return (API->error);
		}

		if (!GMT->common.b.active[GMT_OUT] && Ctrl->G.header) {	/* Want segment header on output */
			int kind = (doubleAlmostEqualZero (Ctrl->G.colat, 90.0)) ? 0 : 1;
			char *type[2] = {"Great", "Small"};
			gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
			if (Ctrl->Z.active)
				sprintf (GMT->current.io.segment_header, "%s", z_header);
			else
				sprintf (GMT->current.io.segment_header, "%s-circle Pole at %g %g", type[kind], P.plon, P.plat);
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);	/* Write segment header */
		}
		if (Ctrl->Z.active) gmt_M_str_free (z_header);
		for (rec = 0; rec < P.n_used; rec++) {
			for (col = 0; col < P.n_outputs; col++) out[col] = p_data[rec].a[P.output_choice[col]];
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		gmt_M_free (GMT, Out);
	}
	else {	/* Must read input file */
		struct GMT_RECORD *In = NULL;

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");
		/* Specify input and output expected columns */
		if ((error = GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_FIX)) != GMT_NOERROR) {
			Return (error);
		}

		/* Initialize the i/o since we are doing record-by-record reading/writing */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers data output */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}

		do {	/* Keep returning records until we reach EOF */
			if ((In = GMT_Get_Record (API, GMT_READ_MIXED, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				}
				else if (gmt_M_rec_is_table_header (GMT))	/* Echo table headers */
					GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, NULL);
				else if (gmt_M_rec_is_segment_header (GMT)) {			/* Echo segment headers */
					if (P.n_used) {	/* Write out previous segment */
						if ((error = write_one_segment (GMT, Ctrl, theta, p_data, &P)) != 0) Return (error);
						n_total_used += P.n_used;
						P.n_used = 0;
					}
					GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
				}
				else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
					break;
				continue;							/* Go back and read the next record */
			}

			/* Data record to process */
			in = In->data;	/* Only need to process numerical part here */

			if (z_first) {
				uint64_t n_cols = gmt_get_cols (GMT, GMT_IN), n_tot_cols;
				if (n_cols == 2 && P.want_z_output && In->text == NULL) {
					if (z_set_auto) {	/* Implicitly set -Fxyzpqrs earlier but input file has no z values so roll back to -Fxypqrs */
						P.want_z_output = z_set_auto = false;
						for (col = 3; col < P.n_outputs; col++) P.output_choice[col-1] = P.output_choice[col];	/* Shuffle pqrs to the left */
						P.n_outputs--;
						for (col = 0; col < P.n_outputs; col++) {
							switch (P.output_choice[col]) {
								case 0: case 4: gmt_set_column (GMT, GMT_OUT, (unsigned int)col, GMT_IS_LON);	break;
								case 1: case 5: gmt_set_column (GMT, GMT_OUT, (unsigned int)col, GMT_IS_LAT);	break;
								default: gmt_set_column (GMT, GMT_OUT, (unsigned int)col, GMT_IS_FLOAT);	break;
							}
						}
					}
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "No data columns or trailing text after leading coordinates, cannot use z flag in -F\n");
						Return (GMT_RUNTIME_ERROR);
					}
				}
				else if (n_cols < 2) {
					GMT_Report (API, GMT_MSG_NORMAL, "Input file must at least have x,y or lon,lat columns\n");
					gmt_M_free (GMT, p_data);
					Return (GMT_RUNTIME_ERROR);
				}
				else {	/* Must update col types since # of z values will need to be considered */
					unsigned int k, kk;
					P.n_z = n_cols - 2;
					for (col = kk = 0; col < P.n_outputs; col++, kk++) {
						switch (P.output_choice[col]) {
							case -1: /* Need to set float type for all the nz columns based on what the input z column types are */
								for (k = 0; k < P.n_z; k++, kk++) gmt_set_column (GMT, GMT_OUT, kk, GMT->current.io.col_type[GMT_IN][k+GMT_Z]);
								kk--;	/* Since we also do this at end of loop */
								break;
							case 0: case 4: gmt_set_column (GMT, GMT_OUT, kk, GMT_IS_LON);		break;
							case 1: case 5: gmt_set_column (GMT, GMT_OUT, kk, GMT_IS_LAT);		break;
							default: 	gmt_set_column (GMT, GMT_OUT, kk, GMT_IS_FLOAT);	break;
						}
					}
				}
				z_first = false;
				n_tot_cols = (P.want_z_output) ? P.n_outputs - 1 + P.n_z : P.n_outputs;
				if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)n_tot_cols, gmt_M_colmode (In->text))) != GMT_NOERROR) {
					Return (error);
				}
			}

			xx = in[GMT_X];
			yy = in[GMT_Y];

			n_total_read ++;

			if (Ctrl->N.active) {
				x[0] = xx - Ctrl->C.x;
				x[1] = yy - Ctrl->C.y;
				matrix_2v (e, x, xt);
			}
			else
				oblique_transform (GMT, yy, xx, &xt[1], &xt[0], P.pole, center);

			skip = ((Ctrl->L.active && (xt[0] < Ctrl->L.min || xt[0] > Ctrl->L.max)) || (Ctrl->W.active && (xt[1] < Ctrl->W.min || xt[1] > Ctrl->W.max)));

			if (skip) continue;

			p_data[P.n_used].a[0] = xx;
			p_data[P.n_used].a[1] = yy;
			p_data[P.n_used].a[2] = xt[0];
			p_data[P.n_used].a[3] = xt[1];
			p_data[P.n_used].t = NULL;	/* Initialize since that is not done by realloc */
			p_data[P.n_used].z = NULL;	/* Initialize since that is not done by realloc */
			if (P.want_z_output && In->text)	/* Must store all trailing text */
				p_data[P.n_used].t = strdup (In->text);
			if (P.n_z) {	/* Copy over z column(s) */
				p_data[P.n_used].z = gmt_M_memory (GMT, NULL, P.n_z, double);
				gmt_M_memcpy (p_data[P.n_used].z, &in[GMT_Z], P.n_z, double);
			}
			P.n_used++;
			if (P.n_used == n_alloc) {
				size_t old_n_alloc = n_alloc;
				n_alloc <<= 1;
				p_data = gmt_M_memory (GMT, p_data, n_alloc, struct PROJECT_DATA);
				gmt_M_memset (&(p_data[old_n_alloc]), n_alloc - old_n_alloc, struct PROJECT_DATA);	/* Set to NULL/0 */
			}
		} while (true);

		if (P.n_used < n_alloc) p_data = gmt_M_memory (GMT, p_data, P.n_used, struct PROJECT_DATA);

		if (P.n_used) {	/* Finish last segment output */
			if ((error = write_one_segment (GMT, Ctrl, theta, p_data, &P)) != 0) Return (error);
			n_total_used += P.n_used;
		}

		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " read, %" PRIu64 " used\n", n_total_read, n_total_used);

	for (rec = 0; rec < P.n_used; rec++) {
		gmt_M_str_free (p_data[rec].t);
		gmt_M_free (GMT, p_data[rec].z);
	}

	gmt_M_free (GMT, p_data);

	Return (GMT_NOERROR);
}
