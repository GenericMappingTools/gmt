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
 * Brief synopsis: project.c reads (x,y,[z]) data and writes some combination of (x,y,z,p,q,u,v),
 * where p,q is the distance along,across the track of the projection of (x,y),
 * and u,v are the un-transformed (x,y) coordinates of the projected position.
 * Can also create (x,y) along track.
 *
 * Author: 	Walter H. F. Smith
 * Date:	1 JAN 2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"project"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Project table data onto lines or great circles, generate tracks, or translate coordinates"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>Vbfghis" GMT_OPT("HMm")

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
	struct G {	/* -G<inc>[/<colat>][+] */
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
};

struct PROJECT_DATA {
	double a[6];
	double *z;
	char *t;
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

int compare_distances (const void *point_1, const void *point_2)
{
	double d_1, d_2;

	d_1 = ((struct PROJECT_DATA *)point_1)->a[2];
	d_2 = ((struct PROJECT_DATA *)point_2)->a[2];

	if (d_1 < d_2) return (-1);
	if (d_1 > d_2) return (1);
	return (0);
}

double oblique_setup (struct GMT_CTRL *GMT, double plat, double plon, double *p, double *clat, double *clon, double *c, bool c_given, bool generate)
{
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

	GMT_geo_to_cart (GMT, plat, plon, p, true);

	if (c_given) GMT_geo_to_cart (GMT, *clat, *clon, s, true);	/* s points to user's clat, clon  */
	GMT_cross3v (GMT, p, s, x);
	GMT_normalize3v (GMT, x);
	GMT_cross3v (GMT, x, p, c);
	GMT_normalize3v (GMT, c);
	if (!generate) GMT_memcpy (c, x, 3, double);
	if (!c_given) GMT_cart_to_geo (GMT, clat, clon, c, true);	/* return the adjusted center  */
	cp = GMT_dot3v (GMT, p, c);
	sin_lat_to_pole = d_sqrt (1.0 - cp * cp);
	return (sin_lat_to_pole);
}

void oblique_transform (struct GMT_CTRL *GMT, double xlat, double xlon, double *x_t_lat, double *x_t_lon, double *p, double *c)
{
	/* routine takes the point x at conventional (xlat, xlon) and
	   computes the transformed coordinates (x_t_lat, x_t_lon) in
	   an oblique reference frame specified by the unit 3-vectors
	   p (the pole) and c (the directed normal to the oblique
	   central meridian).  p and c have been computed earlier by
	   the routine oblique_setup().
	   Latitudes and longitudes are in degrees. */

	double x[3], p_cross_x[3], temp1, temp2;

	GMT_geo_to_cart (GMT, xlat, xlon, x, true);

	temp1 = GMT_dot3v (GMT, x,p);
	*x_t_lat = d_asind(temp1);

	GMT_cross3v (GMT, p,x,p_cross_x);
	GMT_normalize3v (GMT, p_cross_x);

	temp1 = GMT_dot3v (GMT, p_cross_x, c);
	temp2 = GMT_dot3v (GMT, x, c);
	*x_t_lon = copysign(d_acosd(temp1), temp2);
}

void make_euler_matrix (double *p, double *e, double theta)
{
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

void matrix_3v (double *a, double *x, double *b)
{
	/* routine to find b, where Ax = b, A is a 3 by 3 square matrix,
	   and x and b are 3-vectors.  A is stored row wise, that is:

	   A = { a11, a12, a13, a21, a22, a23, a31, a32, a33 }  */

	b[0] = x[0]*a[0] + x[1]*a[1] + x[2]*a[2];
	b[1] = x[0]*a[3] + x[1]*a[4] + x[2]*a[5];
	b[2] = x[0]*a[6] + x[1]*a[7] + x[2]*a[8];
}

void matrix_2v (double *a, double *x, double *b)
{
	/* routine to find b, where Ax = b, A is a 2 by 2 square matrix,
	   and x and b are 2-vectors.  A is stored row wise, that is:

	   A = { a11, a12, a21, a22 }  */

	b[0] = x[0]*a[0] + x[1]*a[1];
	b[1] = x[0]*a[2] + x[1]*a[3];
}

void sphere_project_setup (struct GMT_CTRL *GMT, double alat, double alon, double *a, double blat, double blon, double *b, double azim, double *p, double *c, bool two_pts)
{
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
		GMT_geo_to_cart (GMT, alat, alon, a, true);
		GMT_geo_to_cart (GMT, blat, blon, b, true);
		GMT_cross3v (GMT, a, b, p);
		GMT_normalize3v (GMT, p);
	}
	else {
		GMT_geo_to_cart (GMT, alat, alon, a, true);
		b[0] = b[1] = 0.0;	/* set b to north pole  */
		b[2] = 1.0;
		GMT_cross3v (GMT, a, b, c);	/* use c for p_temp  */
		GMT_normalize3v (GMT, c);
		make_euler_matrix(a, e, -azim);
		matrix_3v(e, c, p);	/* c (p_temp) rotates to p  */
	}

	/* Now set c vector  */

	GMT_cross3v (GMT, p, a, c);
	GMT_normalize3v (GMT, c);
}

void flat_project_setup (double alat, double alon, double blat, double blon, double plat, double plon, double *azim, double *e, bool two_pts, bool pole_set)
{
	/* Sets up stuff for rotation of cartesian 2-vectors, analogous
	   to the spherical three vector stuff above.
	   Output is the negative cartesian azimuth in degrees.
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

void copy_text_from_col3 (char *line, char *z_cols)
{	/* returns the input line starting at the 3rd column */

	unsigned int i;

	/* First replace any commas with spaces */

	for (i = 0; line[i]; i++) if (line[i] == ',') line[i] = ' ';

	sscanf (line, "%*s %*s %[^\n]", z_cols);
}

void *New_project_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PROJECT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1U, struct PROJECT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->G.colat = 90.0;	/* Great circle path */
	return (C);
}

void Free_project_Ctrl (struct GMT_CTRL *GMT, struct PROJECT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);
}

int GMT_project_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: project [<table>] -C<ox>/<oy> [-A<azimuth>] [-E<bx>/<by>] [-F<flags>] [-G<dist>[/<colat>][+]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-L[w][<l_min>/<l_max>]] [-N] [-Q] [-S] [-T<px>/<py>] [%s] [-W<w_min>/<w_max>]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n", GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

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
	GMT_Message (API, GMT_TIME_NONE, "\t   In a cartesian [-N option] projection, p = q = 0 at -O in all cases;\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t   Finally, append + if you want information about the pole in a segment header [no header].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Check the Length along the projected track and use only certain points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Lw will use only those points Within the span from C to E (Must have set -E).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -L<l_min>/<l_max> will only use points whose p is [l_min <= p <= l_max].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default uses all points.  Note p = 0 at C and increases toward E in azim direction.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Flat Earth mode; a cartesian projection is made.  Default is spherical.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Convert to Map units, so x,y,r,s are degrees,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   while p,q,dist,l_min,l_max,w_min,w_max are km.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If not set, then p,q,dist,l_min,l_max,w_min,w_max are assumed to be in same units as x,y,r,s.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Output should be Sorted into increasing p value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set the option (3) location of the roTation pole to the projection to be <px>/<py>.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Check the width across the projected track and use only certain points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This will use only those points whose q is [w_min <= q <= w_max].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note that q is positive to your LEFT as you walk from C toward E in azim direction.\n");
	GMT_Option (API, "bi2,bo,f,g,h,i,s,:,.");

	return (EXIT_FAILURE);
}

int GMT_project_parse (struct GMT_CTRL *GMT, struct PROJECT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to project and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, j, k;
	size_t len;
	char txt_a[GMT_LEN64] = {""}, txt_b[GMT_LEN64] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) if (opt->option == 'N') Ctrl->N.active = true;	/* Must find -N first, if given */

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files are OK */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)) n_errors++;
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
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->C.x), txt_a);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->C.y), txt_b);
					if (n_errors) GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Undecipherable argument %s\n", opt->arg);
				}
				break;
			case 'D':
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -D is deprecated; use --FORMAT_GEO_OUT instead\n");
					if (opt->arg[0] == 'g') GMT->current.io.geo.range = GMT_IS_0_TO_P360_RANGE;
					if (opt->arg[0] == 'd') GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'E':
				Ctrl->E.active = true;
				if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) != 2) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -E<lon1>/<lat1>\n");
					n_errors++;
				}
				else {
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->E.x), txt_a);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->E.y), txt_b);
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
				len = strlen (opt->arg) - 1;
				if (len > 0 && opt->arg[len] == '+') {
					Ctrl->G.header = true;	/* Wish to place a segment header on output */
					opt->arg[len] = 0;	/* Temporarily remove the trailing + sign */
				}
				if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) == 2) {	/* Got dist/colat */
					Ctrl->G.mode = 1;
					Ctrl->G.inc = atof (txt_a);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->G.colat), txt_b);
				}
				else
					Ctrl->G.inc = atof (opt->arg);
				if (Ctrl->G.header) opt->arg[len] = '+';	/* Restore it */
				break;
			case 'L':
				Ctrl->L.active = true;
				if (opt->arg[0] == 'W' || opt->arg[0] == 'w')
					Ctrl->L.constrain = true;
				else {
					n_errors += GMT_check_condition (GMT, sscanf(opt->arg, "%lf/%lf", &Ctrl->L.min, &Ctrl->L.max) != 2, "Syntax error: Expected -L[w | <min>/<max>]\n");
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
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->T.x), txt_a);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->T.y), txt_b);
					if (n_errors) GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Undecipherable argument %s\n", opt->arg);
				}
				break;
			case 'W':
				Ctrl->W.active = true;
				n_errors += GMT_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->W.min, &Ctrl->W.max) != 2, "Syntax error: Expected -W<min>/<max>\n");
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->L.active && !Ctrl->L.constrain && Ctrl->L.min >= Ctrl->L.max, "Syntax error -L option: w_min must be < w_max\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->W.min >= Ctrl->W.max, "Syntax error -W option: w_min must be < w_max\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->A.active + Ctrl->E.active + Ctrl->T.active) > 1, "Syntax error: Specify only one of -A, -E, and -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && (Ctrl->C.x == Ctrl->E.x) && (Ctrl->C.y == Ctrl->E.y), "Syntax error -E option: Second point must differ from origin!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.active && Ctrl->L.min == Ctrl->L.max && !Ctrl->E.active, "Syntax error -G option: Must also specify -Lmin/max or use -E instead\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.active && Ctrl->F.active, "Syntax error -G option: -F not allowed [Defaults to rsp]\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.active && Ctrl->G.inc <= 0.0, "Syntax error -G option: Must specify a positive increment\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.constrain && !Ctrl->E.active, "Syntax error -L option: Must specify -Lmin/max or use -E instead\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && (GMT_is_geographic (GMT, GMT_IN) || GMT_is_geographic (GMT, GMT_OUT)), "Syntax error -N option: Cannot be used with -fg\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && Ctrl->G.mode, "Syntax error -N option: Cannot be used with -G<dist>/<colat>\n");
	n_errors += GMT_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int write_one_segment (struct GMT_CTRL *GMT, struct PROJECT_CTRL *Ctrl, double theta, struct PROJECT_DATA *p_data, struct PROJECT_INFO *P)
{
	int error;
	bool pure_ascii;
	uint64_t col, n_items, rec, k;
	double sin_theta, cos_theta, e[9], x[3], xt[3], *out = NULL;
	char record[GMT_BUFSIZ] = {""}, text[GMT_BUFSIZ] = {""};

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
			GMT_geo_to_cart (GMT, Ctrl->C.y, Ctrl->C.x, x, true);
			for (rec = 0; rec < P->n_used; rec++) {
				make_euler_matrix (P->pole, e, p_data[rec].a[2]);
				matrix_3v (e,x,xt);
				GMT_cart_to_geo (GMT, &(p_data[rec].a[5]), &(p_data[rec].a[4]), xt, true);
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

	n_items = P->n_outputs + ((P->want_z_output && P->n_z) ? P->n_z - 1 : 0);
	out = GMT_memory (GMT, NULL, n_items, double);

	if (P->first && (error = GMT_set_cols (GMT, GMT_OUT, n_items))) return (error);
	pure_ascii = GMT_is_ascii_record (GMT);

	/* Now output  */

	/* Special case for pure ascii since we may pass text */

	if (P->n_z && pure_ascii) {
		for (rec = 0; rec < P->n_used; rec++) {
			record[0] = 0;
			for (col = 0; col < P->n_outputs; col++) {
				if (P->output_choice[col] == -1)	/* Output all z columns as one string */
					strcat (record, p_data[rec].t);
				else {
					sprintf (text, GMT->current.setting.format_float_out, p_data[rec].a[P->output_choice[col]]);
					strcat (record, text);
				}
				(col == (P->n_outputs - 1)) ? strcat (record, "\n") : strcat (record, GMT->current.setting.io_col_separator);
			}
			GMT_Put_Record (GMT->parent, GMT_WRITE_TEXT, record);	/* Write this to output */
		}
	}
	else {	/* Any other i/o combination */
		for (rec = 0; rec < P->n_used; rec++) {
			for (col = k = 0; col < P->n_outputs; col++) {
				if (P->output_choice[col] == -1) {	/* Copy over all z columns */
					GMT_memcpy (&out[k], p_data[rec].z, P->n_z, double);
					k += P->n_z;
				}
				else
					out[k++] = p_data[rec].a[P->output_choice[col]];
			}
			GMT_Put_Record (GMT->parent, GMT_WRITE_DOUBLE, out);	/* Write this to output */
		}
	}
	GMT_free (GMT, out);
	return (0);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_project_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_project (void *V_API, int mode, void *args)
{
	uint64_t rec, n_total_read, col, n_total_used = 0;
	unsigned int rmode;
	bool pure_ascii, skip, z_first = true;
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
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_project_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_project_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_project_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_project_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_project_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the project main code ----------------------------*/

	GMT_memset (&P, 1, struct PROJECT_INFO);
	GMT_memset (a, 3, double);
	GMT_memset (b, 3, double);
	GMT_memset (x, 3, double);
	GMT_memset (xt, 3, double);
	GMT_memset (center, 3, double);
	GMT_memset (e, 9, double);
	P.first = true;
	if (Ctrl->N.active) {	/* Must undo an optional -fg that was set before */
		GMT_set_cartesian (GMT, GMT_IN);
		GMT_set_cartesian (GMT, GMT_OUT);
	}
	else {	/* Make sure we set -fg */
		GMT_set_geographic (GMT, GMT_IN);
		GMT_set_geographic (GMT, GMT_OUT);
	}

	/* Convert user's -F choices to internal parameters */
	for (col = P.n_outputs = 0; col < PROJECT_N_FARGS && Ctrl->F.col[col]; col++) {
		switch (Ctrl->F.col[col]) {
			case 'z':	/* Special flag, can mean any number of z columns */
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

	pure_ascii = !(GMT->common.b.active[GMT_IN] || GMT->common.b.active[GMT_OUT]);

	if (P.n_outputs == 0 && !Ctrl->G.active) {	/* Generate default -F setting (all) */
		P.n_outputs = PROJECT_N_FARGS;
		for (col = 0; col < 2; col++) P.output_choice[col] = (int)col;
		P.output_choice[2] = -1;
		for (col = 3; col < P.n_outputs; col++) P.output_choice[col] = (int)col - 1;
		P.find_new_point = true;
	}
	if (Ctrl->G.active) P.n_outputs = 3;

	p_data = GMT_memory (GMT, NULL, n_alloc, struct PROJECT_DATA);

	if (Ctrl->G.active && Ctrl->E.active && (Ctrl->L.min == Ctrl->L.max)) Ctrl->L.constrain = true;	/* Default generate from A to B  */

	/* Set up rotation matrix e for flat earth, or pole and center for spherical; get Ctrl->L.min, Ctrl->L.max if stay_within  */

	if (Ctrl->N.active) {
		theta = Ctrl->A.azimuth;
		flat_project_setup (Ctrl->C.y, Ctrl->C.x, Ctrl->E.y, Ctrl->E.x, Ctrl->T.y, Ctrl->T.x, &theta, e, Ctrl->E.active, Ctrl->T.active);
		/* Azimuth (theta) is now cartesian in degrees */
		if (Ctrl->L.constrain) {
			Ctrl->L.min = 0.0;
			xx = Ctrl->E.x - Ctrl->C.x;
			yy = Ctrl->E.y - Ctrl->C.y;
			Ctrl->L.max = hypot (xx, yy);
			if (Ctrl->Q.active) Ctrl->L.max *= GMT->current.proj.DIST_KM_PR_DEG;
		}
	}
	else {
		if (Ctrl->T.active) {
			sin_lat_to_pole = oblique_setup (GMT, Ctrl->T.y, Ctrl->T.x, P.pole, &Ctrl->C.y, &Ctrl->C.x, center, Ctrl->C.active, Ctrl->G.active);
			GMT_cart_to_geo (GMT, &P.plat, &P.plon, x, true);	/* Save lon, lat of the pole */
		}
		else {	/* Using -C -E or -A */
			double s_hi, s_lo, s_mid, radius, m[3], ap[3], bp[3];
			int done, n_iter = 0;
			
			sphere_project_setup (GMT, Ctrl->C.y, Ctrl->C.x, a, Ctrl->E.y, Ctrl->E.x, b, Ctrl->A.azimuth, P.pole, center, Ctrl->E.active);
			GMT_cart_to_geo (GMT, &P.plat, &P.plon, x, true);	/* Save lon, lat of the pole */
			radius = 0.5 * d_acosd (GMT_dot3v (GMT, a, b)); 
			if (radius > fabs (Ctrl->G.colat)) {
				GMT_Report (API, GMT_MSG_NORMAL, "Center [-C] and end point [-E] are too far apart (%g) to define a small-circle with colatitude %g. Revert to great-circle.\n", radius, Ctrl->G.colat);
				Ctrl->G.mode = 0;
			}
			else if (doubleAlmostEqual (Ctrl->G.colat, 90.0)) {	/* Great circle pole needed */
				if (!Ctrl->L.active) Ctrl->L.max = d_acosd (GMT_dot3v (GMT, a, b));
			}
			else {	/* Find small circle pole so C and E are |lat| degrees from it. */
				for (col = 0; col < 3; col++) m[col] = a[col] + b[col];	/* Mid point along A-B */
				GMT_normalize3v (GMT, m);
				sign = copysign (1.0, Ctrl->G.colat);
				s_hi = 90.0;	s_lo = 0.0;
				done = false;
				do {	/* Trial for finding pole S */
					n_iter++;
					s_mid = 0.5 * (s_lo + s_hi);
					sincosd (sign * s_mid, &s, &c);
					for (col = 0; col < 3; col++) x[col] = P.pole[col] * s + m[col] * c;
					GMT_normalize3v (GMT, x);
					radius = d_acosd (GMT_dot3v (GMT, a, x)); 
					if (fabs (radius - fabs (Ctrl->G.colat)) < GMT_CONV_LIMIT)
						done = true;
					else if (radius > fabs (Ctrl->G.colat))
						s_hi = s_mid;
					else
						s_lo = s_mid;
					if (n_iter > 500) done = true;	/* Safety valve */
				} while (!done);
				GMT_cart_to_geo (GMT, &P.plat, &P.plon, x, true);	/* Save lon, lat of the new pole */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Pole for small circle located at %g %g\n", radius, P.plon, P.plat);
				GMT_memcpy (P.pole, x, 3, double);	/* Replace great circle pole with small circle pole */
				sin_lat_to_pole = s;
				GMT_cross3v (GMT, P.pole, a, x);
				GMT_normalize3v (GMT, x);
				GMT_cross3v (GMT, x, P.pole, ap);
				GMT_normalize3v (GMT, ap);
				GMT_cross3v (GMT, P.pole, b, x);
				GMT_normalize3v (GMT, x);
				GMT_cross3v (GMT, x, P.pole, bp);
				GMT_normalize3v (GMT, bp);
				if (!Ctrl->L.active) Ctrl->L.max = d_acosd (GMT_dot3v (GMT, ap, bp));
			}
		}
		if (Ctrl->L.constrain) {
			Ctrl->L.min = 0.0;
			if (Ctrl->Q.active) Ctrl->L.max *= (GMT->current.proj.DIST_KM_PR_DEG * sin_lat_to_pole);
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

	if ((error = GMT_set_cols (GMT, GMT_OUT, P.n_outputs)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}

	P.n_used = n_total_read = 0;

	if (Ctrl->G.active) {	/* Not input data expected, just generate x,y,d track from arguments given */
		double out[3];
		P.output_choice[0] = 4;
		P.output_choice[1] = 5;
		P.output_choice[2] = 2;

		GMT_Report (API, GMT_MSG_VERBOSE, "Generate table data\n");
		d_along = Ctrl->L.min;
		while ((Ctrl->L.max - d_along) > (GMT_CONV_LIMIT*Ctrl->G.inc)) {
			p_data[P.n_used].a[2] = d_along;
			p_data[P.n_used].t = NULL;	/* Initialize since that is not done by realloc */
			p_data[P.n_used].z = NULL;	/* Initialize since that is not done by realloc */
			P.n_used++;
			d_along = Ctrl->L.min + P.n_used * Ctrl->G.inc;
			if (P.n_used == (n_alloc-1)) {
				size_t old_n_alloc = n_alloc;
				n_alloc <<= 1;
				p_data = GMT_memory (GMT, p_data, n_alloc, struct PROJECT_DATA);
				GMT_memset (&(p_data[old_n_alloc]), n_alloc - old_n_alloc, struct PROJECT_DATA);	/* Set to NULL/0 */
			}
		}
		p_data[P.n_used].a[2] = Ctrl->L.max;
		p_data[P.n_used].t = NULL;	/* Initialize since that is not done by realloc */
		p_data[P.n_used].z = NULL;	/* Initialize since that is not done by realloc */
		P.n_used ++;

		/* We need to find r,s  */

		if (Ctrl->N.active) {
			sincosd (90.0 + theta, &sin_theta, &cos_theta);
			for (rec = 0; rec < P.n_used; rec++) {
				p_data[rec].a[4] = Ctrl->C.x + p_data[rec].a[2] * cos_theta;
				p_data[rec].a[5] = Ctrl->C.y + p_data[rec].a[2] * sin_theta;
			}
		}
		else {
			/* Must set generating vector to point along zero-meridian so it is the desired number of degrees [90]
			 * from the pole. */
			double C[3], N[3];
			GMT_geo_to_cart (GMT, Ctrl->C.y, Ctrl->C.x, C, true);	/* User origin C */
			GMT_cross3v (GMT, P.pole, C, N);		/* This is vector normal to meridian plan */
			GMT_normalize3v (GMT, N);			/* Make it a unit vector */
			make_euler_matrix (N, e, Ctrl->G.colat);	/* Rotation matrix about N */
			matrix_3v (e, P.pole, x);			/* This is the generating vector for our circle */
			for (rec = 0; rec < P.n_used; rec++) {
				make_euler_matrix (P.pole, e, sign * p_data[rec].a[2]);
				matrix_3v (e,x,xt);
				GMT_cart_to_geo (GMT, &(p_data[rec].a[5]), &(p_data[rec].a[4]), xt, true);
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

		if (!GMT->common.b.active[GMT_OUT] && Ctrl->G.header) {	/* Want segment header on output */
			int kind = (doubleAlmostEqualZero (Ctrl->G.colat, 90.0)) ? 0 : 1;
			char *type[2] = {"Great", "Small"};
			GMT_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
			sprintf (GMT->current.io.segment_header, "%s-circle Pole at %g %g", type[kind], P.plon, P.plat);
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);	/* Write segment header */
		}
		for (rec = 0; rec < P.n_used; rec++) {
			for (col = 0; col < P.n_outputs; col++) out[col] = p_data[rec].a[P.output_choice[col]];
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
		}
	}
	else {	/* Must read input file */

		GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
		/* Specify input and output expected columns */
		if ((error = GMT_set_cols (GMT, GMT_IN, 0)) != GMT_OK) {
			Return (error);
		}

		/* Initialize the i/o since we are doing record-by-record reading/writing */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		rmode = (pure_ascii && GMT_get_cols (GMT, GMT_IN) >= 2) ? GMT_READ_MIXED : GMT_READ_DOUBLE;

		do {	/* Keep returning records until we reach EOF */
			if ((in = GMT_Get_Record (API, rmode, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (GMT_REC_IS_TABLE_HEADER (GMT)) {	/* Echo table headers */
					GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, NULL);
					continue;
				}
				if (GMT_REC_IS_SEGMENT_HEADER (GMT)) {			/* Echo segment headers */
					if (P.n_used) {	/* Write out previous segment */
						if ((error = write_one_segment (GMT, Ctrl, theta, p_data, &P))) Return (error);
						n_total_used += P.n_used;
						P.n_used = 0;
					}
					GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
					continue;
				}
				if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
					break;
			}

			/* Data record to process */

			if (z_first) {
				uint64_t n_cols = GMT_get_cols (GMT, GMT_IN);
				if (n_cols == 2 && P.want_z_output) {
					GMT_Report (API, GMT_MSG_NORMAL, "No data columns, cannot use z flag in -F\n");
					Return (EXIT_FAILURE);
				}
				else
					P.n_z = n_cols - 2;
				z_first = false;
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
			if (P.n_z) {	/* Copy over z column(s) */
				if (pure_ascii) {	/* Must store all text beyond x,y columns */
					p_data[P.n_used].t = GMT_memory (GMT, NULL, strlen (GMT->current.io.current_record), char);
					copy_text_from_col3 (GMT->current.io.current_record, p_data[P.n_used].t);
				}
				else {
					p_data[P.n_used].z = GMT_memory (GMT, NULL, P.n_z, double);
					GMT_memcpy (p_data[P.n_used].z, &in[GMT_Z], P.n_z, double);
				}
			}
			P.n_used++;
			if (P.n_used == n_alloc) {
				size_t old_n_alloc = n_alloc;
				n_alloc <<= 1;
				p_data = GMT_memory (GMT, p_data, n_alloc, struct PROJECT_DATA);
				GMT_memset (&(p_data[old_n_alloc]), n_alloc - old_n_alloc, struct PROJECT_DATA);	/* Set to NULL/0 */
			}
		} while (true);

		if (P.n_used < n_alloc) p_data = GMT_memory (GMT, p_data, P.n_used, struct PROJECT_DATA);

		if (P.n_used) {	/* Finish last segment output */
			if ((error = write_one_segment (GMT, Ctrl, theta, p_data, &P))) Return (error);
			n_total_used += P.n_used;
		}

		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " read, %" PRIu64 " used\n", n_total_read, n_total_used);

	for (rec = 0; rec < P.n_used; rec++) {
		if (p_data[rec].t) GMT_free (GMT, p_data[rec].t);
		if (p_data[rec].z) GMT_free (GMT, p_data[rec].z);
	}

	GMT_free (GMT, p_data);

	Return (GMT_OK);
}
