/*--------------------------------------------------------------------
 *	$Id$
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
 * Brief synopsis: project.c reads (x,y,[z]) data and writes some combination of (x,y,z,p,q,u,v),
 * where p,q is the distance along,across the track of the projection of (x,y),
 * and u,v are the un-transformed (x,y) coordinates of the projected position.
 * Can also create (x,y) along track.
 *
 * Author: 	Walter H. F. Smith
 * Date:	1 JAN 2010
 * Version:	5 API
 */

#include "gmt.h"

#define PROJECT_N_FARGS	7

struct PROJECT_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct A {	/* -A<azimuth> */
		GMT_LONG active;
		double azimuth;
	} A;
	struct C {	/* -C<ox>/<oy> */
		GMT_LONG active;
		double x, y;
	} C;
	struct E {	/* -E<bx>/<by> */
		GMT_LONG active;
		double x, y;
	} E;
	struct F {	/* -F<flags> */
		GMT_LONG active;
		char col[PROJECT_N_FARGS];	/* Character codes for desired output in the right order */
	} F;
	struct G {	/* -G<inc>[/<lat>] */
		GMT_LONG active;
		GMT_LONG mode;
		double inc;
		double lat;
	} G;
	struct L {	/* -L[w][<l_min>/<l_max>] */
		GMT_LONG active;
		GMT_LONG constrain;
		double min, max;
	} L;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct Q {	/* -Q */
		GMT_LONG active;
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct T {	/* -T<px>/<py> */
		GMT_LONG active;
		double x, y;
	} T;
	struct W {	/* -W<w_min>/<w_max> */
		GMT_LONG active;
		double min, max;
	} W;
};

struct PROJECT_DATA {
	double a[6];
	double *z;
	char *t;
};

struct PROJECT_INFO {
	GMT_LONG n_used;
	GMT_LONG find_new_point;
	GMT_LONG n_outputs;
	GMT_LONG want_z_output;
	GMT_LONG n_z;
	GMT_LONG first;
	GMT_LONG output_choice[PROJECT_N_FARGS];
	double pole[3];
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

double oblique_setup (struct GMT_CTRL *GMT, double plat, double plon, double *p, double *clat, double *clon, double *c, GMT_LONG c_given, GMT_LONG generate)
{
	/* routine sets up a unit 3-vector p, the pole of an
	   oblique projection, given plat, plon, the position
	   of this pole in the usual coordinate frame.
	   c_given = TRUE means that clat, clon are to be used
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

	GMT_geo_to_cart (GMT, plat, plon, p, TRUE);

	if (c_given) GMT_geo_to_cart (GMT, *clat, *clon, s, TRUE);	/* s points to user's clat, clon  */
	GMT_cross3v (GMT, p, s, x);
	GMT_normalize3v (GMT, x);
	GMT_cross3v (GMT, x, p, c);
	GMT_normalize3v (GMT, c);
	if (!generate) GMT_memcpy (c, x, 3, double);
	if (!c_given) GMT_cart_to_geo (GMT, clat, clon, c, TRUE);	/* return the adjusted center  */
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

	GMT_geo_to_cart (GMT, xlat, xlon, x, TRUE);

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

void sphere_project_setup (struct GMT_CTRL *GMT, double alat, double alon, double *a, double blat, double blon, double *b, double azim, double *p, double *c, GMT_LONG two_pts)
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
		GMT_geo_to_cart (GMT, alat, alon, a, TRUE);
		GMT_geo_to_cart (GMT, blat, blon, b, TRUE);
		GMT_cross3v (GMT, a, b, p);
		GMT_normalize3v (GMT, p);
	}
	else {
		GMT_geo_to_cart (GMT, alat, alon, a, TRUE);
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

void flat_project_setup (double alat, double alon, double blat, double blon, double plat, double plon, double *azim, double *e, GMT_LONG two_pts, GMT_LONG pole_set)
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

	GMT_LONG i;

	/* First replace any commas with spaces */

	for (i = 0; line[i]; i++) if (line[i] == ',') line[i] = ' ';

	sscanf (line, "%*s %*s %[^\n]", z_cols);
}

void *New_project_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PROJECT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PROJECT_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->G.lat = 90.0;	/* Great circle path */
	return (C);
}

void Free_project_Ctrl (struct GMT_CTRL *GMT, struct PROJECT_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);
}

GMT_LONG GMT_project_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "project %s [API] - Project table data onto lines or great circles, generate tracks, or translate coordinates\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: project [<table>] -C<ox>/<oy> [-A<azimuth>] [-E<bx>/<by>]\n");
	GMT_message (GMT, "\t[-F<flags>] [-G<dist>[/<lat>]] [-L[w][<l_min>/<l_max>]]\n");
	GMT_message (GMT, "\t[-N] [-Q] [-S] [-T<px>/<py>] [%s] [-W<w_min>/<w_max>]\n", GMT_V_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n", GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\tproject will read stdin or file, and does not want input if -G option.\n");
	GMT_message (GMT, "\tThe projection may be defined in (only) one of three ways:\n");
	GMT_message (GMT, "\t   (1) by a center -C and an azimuth -A,\n");
	GMT_message (GMT, "\t   (2) by a center -C and end point of the path -E,\n");
	GMT_message (GMT, "\t   (3) by a center -C and a roTation pole position -T.\n");
	GMT_message (GMT, "\t   In a spherical projection [default], all cases place the central meridian\n");
	GMT_message (GMT, "\t   of the transformed coordinates (p,q) through -C (p = 0 at -C).  The equator\n");
	GMT_message (GMT, "\t   of the (p,q) system (line q = 0) passes through -C and makes an angle\n");
	GMT_message (GMT, "\t   <azimuth> with North (case 1), or passes through -E (case 2), or is\n");
	GMT_message (GMT, "\t   determined by the pole -T (case 3).  In (3), point -C need not be on equator.\n");
	GMT_message (GMT, "\t   In a cartesian [-N option] projection, p = q = 0 at -O in all cases;\n");
	GMT_message (GMT, "\t   (1) and (2) orient the p axis, while (3) orients the q axis.\n\n");
	GMT_message (GMT, "\t-C Set the location of the center to be <ox>/<oy>.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-A Set the option (1) Azimuth, (<azimuth> in degrees CW from North).\n");
	GMT_message (GMT, "\t-D Force the location of the Discontinuity in the r coordinate;\n");
	GMT_message (GMT, "\t   -Dd (dateline) means [-180 < r < 180], -Dg (greenwich) means [0 < r < 360].\n");
	GMT_message (GMT, "\t   The default does not check; in spherical case this usually results in [-180,180].\n");
	GMT_message (GMT, "\t-E Set the option (2) location of end point E to be <bx>/<by>.\n");
	GMT_message (GMT, "\t-F Indicate what output you want as one or more of xyzpqrs in any order;\n");
	GMT_message (GMT, "\t   where x,y,[z] refer to input data locations and optional values,\n");
	GMT_message (GMT, "\t   p,q are the coordinates of x,y in the projection's coordinate system,\n");
	GMT_message (GMT, "\t   r,s is the projected position of x,y (taking q = 0) in the (x,y) coordinate system.\n");
	GMT_message (GMT, "\t   p,q may be scaled from degrees into kilometers by the -Q option.  See -L, -Q, -W.\n");
	GMT_message (GMT, "\t   Note z refers to all input data columns beyond the required x,y\n");
	GMT_message (GMT, "\t   [Default is all fields, i.e. -Fxyzpqrs].\n");
	GMT_message (GMT, "\t   If -G is set, -F is not available and output defaults to rsp.\n");
	GMT_message (GMT, "\t-G Generate (r,s,p) points along profile every <dist> units. (No input data used.)\n");
	GMT_message (GMT, "\t   If E given, will generate from C to E; else must give -L<l_min>/<l_max> for length.\n");
	GMT_message (GMT, "\t   Optionally, append /<lat> for a small circle path through C and E (requires -C -E).\n");
	GMT_message (GMT, "\t-L Check the Length along the projected track and use only certain points.\n");
	GMT_message (GMT, "\t   -Lw will use only those points Within the span from C to E (Must have set -E).\n");
	GMT_message (GMT, "\t   -L<l_min>/<l_max> will only use points whose p is [l_min <= p <= l_max].\n");
	GMT_message (GMT, "\t   Default uses all points.  Note p = 0 at C and increases toward E in azim direction.\n");
	GMT_message (GMT, "\t-N Flat Earth mode; a cartesian projection is made.  Default is spherical.\n");
	GMT_message (GMT, "\t-Q Convert to Map units, so x,y,r,s are degrees,\n");
	GMT_message (GMT, "\t   while p,q,dist,l_min,l_max,w_min,w_max are km.\n");
	GMT_message (GMT, "\t   If not set, then p,q,dist,l_min,l_max,w_min,w_max are assumed to be in same units as x,y,r,s.\n");
	GMT_message (GMT, "\t-S Output should be Sorted into increasing p value.\n");
	GMT_message (GMT, "\t-T Set the option (3) location of the roTation pole to the projection to be <px>/<py>.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Check the width across the projected track and use only certain points.\n");
	GMT_message (GMT, "\t   This will use only those points whose q is [w_min <= q <= w_max].\n");
	GMT_message (GMT, "\t   Note that q is positive to your LEFT as you walk from C toward E in azim direction.\n");
	GMT_explain_options (GMT, "C2D0fghis:.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_project_parse (struct GMTAPI_CTRL *C, struct PROJECT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to project and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j, k, n_errors = 0;
	char txt_a[GMT_TEXT_LEN64], txt_b[GMT_TEXT_LEN64];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) if (opt->option == 'N') Ctrl->N.active = TRUE;	/* Must find -N first, if given */

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files are OK */
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = TRUE;
				Ctrl->A.azimuth = atof (opt->arg);
				break;
			case 'C':
				Ctrl->C.active = TRUE;
				if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) != 2) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Expected -C<lon0>/<lat0>\n");
					n_errors++;
				}
				else {
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->C.x), txt_a);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->C.y), txt_b);
					if (n_errors) GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -C option: Undecipherable argument %s\n", opt->arg);
				}
				break;
#ifdef GMT_COMPAT
			case 'D':
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -D is deprecated; use --FORMAT_GEO_OUT instead\n");
				if (opt->arg[0] == 'g') GMT->current.io.geo.range = GMT_IS_0_TO_P360_RANGE;
				if (opt->arg[0] == 'd') GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;
				break;
#endif
			case 'E':
				Ctrl->E.active = TRUE;
				if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) != 2) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Expected -E<lon1>/<lat1>\n");
					n_errors++;
				}
				else {
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->E.x), txt_a);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->E.y), txt_b);
					if (n_errors) GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -E option: Undecipherable argument %s\n", opt->arg);
				}
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				for (j = 0, k = 0; opt->arg[j]; j++, k++) {
					if (k < PROJECT_N_FARGS) {
						Ctrl->F.col[k] = opt->arg[j];
						if (!strchr ("xyzpqrs", opt->arg[j])) {
							GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -F option: Choose from -Fxyzpqrs\n");
							n_errors++;
						}
					}
					else {
						n_errors++;
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -F option: Too many output columns selected\n");
					}
				}
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) == 2) {	/* Got dist/lat */
					Ctrl->G.mode = 1;
					Ctrl->G.inc = atof (txt_a);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->G.lat), txt_b);
				}
				else
					Ctrl->G.inc = atof (opt->arg);
				break;
			case 'L':
				Ctrl->L.active = TRUE;
				if (opt->arg[0] == 'W' || opt->arg[0] == 'w')
					Ctrl->L.constrain = TRUE;
				else {
					n_errors += GMT_check_condition (GMT, sscanf(opt->arg, "%lf/%lf", &Ctrl->L.min, &Ctrl->L.max) != 2, "Syntax error: Expected -L[w | <min>/<max>]\n");
				}
				break;
			case 'N': /* Handled above but still in argv */
				Ctrl->N.active = TRUE;
				break;
			case 'Q':
				Ctrl->Q.active = TRUE;
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) != 2) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Expected -T<lonp>/<latp>\n");
					n_errors++;
				}
				else {
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->T.x), txt_a);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->T.y), txt_b);
					if (n_errors) GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -T option: Undecipherable argument %s\n", opt->arg);
				}
				break;
			case 'W':
				Ctrl->W.active = TRUE;
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
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && Ctrl->G.mode, "Syntax error -N option: Cannot be used with -G<dist>/<lat>\n");
	n_errors += GMT_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LONG write_one_segment (struct GMT_CTRL *GMT, struct PROJECT_CTRL *Ctrl, double theta, struct PROJECT_DATA *p_data, struct PROJECT_INFO *P)
{
	GMT_LONG n_items, i, j, k, error, pure_ascii;
	double sin_theta, cos_theta, e[9], x[3], xt[3], *out = NULL;
	char record[GMT_BUFSIZ], text[GMT_BUFSIZ];

	if (Ctrl->S.active) qsort (p_data, (size_t)P->n_used, sizeof (struct PROJECT_DATA), compare_distances);

	/* Get here when all data are loaded with p,q and p is in increasing order if desired. */

	if (P->find_new_point) {	/* We need to find r,s  */

		if (Ctrl->N.active) {
			sincosd (theta, &sin_theta, &cos_theta);
			for (i = 0; i < P->n_used; i++) {
				p_data[i].a[4] = Ctrl->C.x + p_data[i].a[2] * cos_theta;
				p_data[i].a[5] = Ctrl->C.y + p_data[i].a[2] * sin_theta;
			}
		}
		else {
			GMT_geo_to_cart (GMT, Ctrl->C.y, Ctrl->C.x, x, TRUE);
			for (i = 0; i < P->n_used; i++) {
				make_euler_matrix (P->pole, e, p_data[i].a[2]);
				matrix_3v (e,x,xt);
				GMT_cart_to_geo (GMT, &(p_data[i].a[5]), &(p_data[i].a[4]), xt, TRUE);
			}
		}
	}

	/* At this stage, all values are still in degrees.  */

	if (Ctrl->Q.active) {
		for (i = 0; i < P->n_used; i++) {
			p_data[i].a[2] *= GMT->current.proj.DIST_KM_PR_DEG;
			p_data[i].a[3] *= GMT->current.proj.DIST_KM_PR_DEG;
		}
	}

	n_items = P->n_outputs + ((P->want_z_output && P->n_z) ? P->n_z - 1 : 0);
	out = GMT_memory (GMT, NULL, n_items, double);

	if (P->first && (error = GMT_set_cols (GMT, GMT_OUT, n_items))) return (error);
	pure_ascii = !(GMT->common.b.active[GMT_IN] || GMT->common.b.active[GMT_OUT]);

	/* Now output  */

	/* Special case for pure ascii since we may pass text */

	if (P->n_z && pure_ascii) {
		for (i = 0; i < P->n_used; i++) {
			record[0] = 0;
			for (j = 0; j < P->n_outputs; j++) {
				if (P->output_choice[j] == -1)	/* Output all z columns as one string */
					strcat (record, p_data[i].t);
				else {
					sprintf (text, GMT->current.setting.format_float_out, p_data[i].a[P->output_choice[j]]);
					strcat (record, text);
				}
				(j == (P->n_outputs - 1)) ? strcat (record, "\n") : strcat (record, GMT->current.setting.io_col_separator);
			}
			GMT_Put_Record (GMT->parent, GMT_WRITE_TEXT, record);	/* Write this to output */
		}
	}
	else {	/* Any other i/o combination */
		for (i = 0; i < P->n_used; i++) {
			for (j = k = 0; j < P->n_outputs; j++) {
				if (P->output_choice[j] == -1) {	/* Copy over all z columns */
					GMT_memcpy (&out[k], p_data[i].z, P->n_z, double);
					k += P->n_z;
				}
				else
					out[k++] = p_data[i].a[P->output_choice[j]];
			}
			GMT_Put_Record (GMT->parent, GMT_WRITE_DOUBLE, out);	/* Write this to output */
		}
	}
	GMT_free (GMT, out);
	return (0);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_project_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_project (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG i, n_total_read, n_total_used = 0, n_alloc = GMT_CHUNK;
	GMT_LONG j, k, n_fields, rmode, error = FALSE;
	GMT_LONG pure_ascii, skip, z_first = TRUE;

	double xx, yy, cos_theta, sin_theta, sin_lat_to_pole = 1.0;
	double theta = 0.0, d_along, sign = 1.0, s, c, *in = NULL;
	double a[3], b[3], x[3], xt[3], center[3], e[9];

	struct PROJECT_DATA *p_data = NULL;
	struct PROJECT_INFO P;
	struct PROJECT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	if ((options = GMT_Prep_Options (API, mode, args)) == NULL) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_project_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_project_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_project", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-Vbf:", "ghis>" GMT_OPT("HMm"), options)) Return (API->error);
	Ctrl = New_project_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_project_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the project main code ----------------------------*/

	GMT_memset (&P, 1, struct PROJECT_INFO);
	P.first = TRUE;
	if (Ctrl->N.active) {	/* Must undo an optional -fg that was set before */
		GMT->current.io.col_type[GMT_IN][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_FLOAT;
		GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_FLOAT;
	}
	else {	/* Make sure we set -fg */
		GMT->current.io.col_type[GMT_IN][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
		GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
	}

	/* Convert user's -F choices to internal parameters */
	for (k = P.n_outputs = 0; k < PROJECT_N_FARGS && Ctrl->F.col[k]; k++) {
		switch (Ctrl->F.col[k]) {
			case 'z':	/* Special flag, can mean any number of z columns */
				P.output_choice[k] = -1;
				P.want_z_output = TRUE;
				break;
			case 'x':
				P.output_choice[k] = 0;
				break;
			case 'y':
				P.output_choice[k] = 1;
				break;
			case 'p':
				P.output_choice[k] = 2;
				break;
			case 'q':
				P.output_choice[k] = 3;
				break;
			case 'r':
				P.output_choice[k] = 4;
				P.find_new_point = TRUE;
				break;
			case 's':
				P.output_choice[k] = 5;
				P.find_new_point = TRUE;
				break;
			default:	/* Already checked in parser that this cannot happen */
				break;
		}
		P.n_outputs++;
	}

	pure_ascii = !(GMT->common.b.active[GMT_IN] || GMT->common.b.active[GMT_OUT]);

	if (P.n_outputs == 0 && !Ctrl->G.active) {	/* Generate default -F setting (all) */
		P.n_outputs = PROJECT_N_FARGS;
		for (i = 0; i < 2; i++) P.output_choice[i] = i;
		P.output_choice[2] = -1;
		for (i = 3; i < P.n_outputs; i++) P.output_choice[i] = i - 1;
		P.find_new_point = TRUE;
	}
	if (Ctrl->G.active) P.n_outputs = 3;

	p_data = GMT_memory (GMT, NULL, n_alloc, struct PROJECT_DATA);

	if (Ctrl->G.active && Ctrl->E.active && (Ctrl->L.min == Ctrl->L.max)) Ctrl->L.constrain = TRUE;	/* Default generate from A to B  */

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
			if (Ctrl->G.mode) {	/* Want small-circle path about T; must adjust C to be at right lat */
				Ctrl->G.lat = 90 - Ctrl->G.lat;
				sign = copysign (1.0, Ctrl->G.lat);
				sincosd (Ctrl->G.lat, &s, &c);
				GMT_geo_to_cart (GMT, Ctrl->C.y, Ctrl->C.x, a, TRUE);
				for (k = 0; k < 3; k++) x[k] = sign * (P.pole[k] * s + a[k] * c);
				GMT_cart_to_geo (GMT, &(Ctrl->C.y), &(Ctrl->C.x), x, TRUE);
				sin_lat_to_pole = c;
			}
		}
		else {
			sphere_project_setup (GMT, Ctrl->C.y, Ctrl->C.x, a, Ctrl->E.y, Ctrl->E.x, b, Ctrl->A.azimuth, P.pole, center, Ctrl->E.active);
			if (Ctrl->G.mode) {	/* Want small-circle path from C to E */
				double s_hi, s_lo, s_mid, radius, m[3], ap[3], bp[3];
				GMT_LONG done;
				radius = 0.5 * d_acosd (GMT_dot3v (GMT, a, b)); 
				if (radius > fabs (Ctrl->G.lat)) {
					GMT_report (GMT, GMT_MSG_FATAL, "Center [-C] and end point [-E] are too far apart (%g) to define a small-circle with latitude %g. Revert to great-circle.\n", radius, Ctrl->G.lat);
					Ctrl->G.mode = 0;
				}
				else {	/* Find small circle pole so C and E are |lat| degrees from it. */
					for (k = 0; k < 3; k++) m[k] = a[k] + b[k];	/* Mid point along A-B */
					GMT_normalize3v (GMT, m);
					sign = copysign (1.0, Ctrl->G.lat);
					s_hi = 90.0;	s_lo = 0.0;
					done = FALSE;
					do {	/* Trial for finding pole S */
						s_mid = 0.5 * (s_lo + s_hi);
						sincosd (sign * s_mid, &s, &c);
						for (k = 0; k < 3; k++) x[k] = P.pole[k] * s + m[k] * c;
						GMT_normalize3v (GMT, x);
						radius = d_acosd (GMT_dot3v (GMT, a, x)); 
						if (fabs (radius - fabs (Ctrl->G.lat)) < 0.1)
							done = TRUE;
						else if (radius > fabs (Ctrl->G.lat))
							s_hi = s_mid;
						else
							s_lo = s_mid;
					} while (!done);
					GMT_memcpy (P.pole, x, 3, double);	/* Replace great circle pole with small circle pole */
					sin_lat_to_pole = c;
					GMT_cross3v (GMT, P.pole, a, x);
					GMT_normalize3v (GMT, x);
					GMT_cross3v (GMT, x, P.pole, ap);
					GMT_normalize3v (GMT, ap);
					GMT_cross3v (GMT, P.pole, b, x);
					GMT_normalize3v (GMT, x);
					GMT_cross3v (GMT, x, P.pole, bp);
					GMT_normalize3v (GMT, bp);
					Ctrl->L.max = d_acosd (GMT_dot3v (GMT, ap, bp)) * sin_lat_to_pole;
				}
			}
		}
		if (Ctrl->L.constrain) {
			Ctrl->L.min = 0.0;
			if (!Ctrl->G.mode) Ctrl->L.max = d_acosd (GMT_dot3v (GMT, a, b));
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

	if ((error = GMT_set_cols (GMT, GMT_OUT, P.n_outputs)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Registers data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}

	P.n_used = n_total_read = 0;

	if (Ctrl->G.active) {	/* Not input data expected, just generate x,y,d track from arguments given */
		double out[3];
		P.output_choice[0] = 4;
		P.output_choice[1] = 5;
		P.output_choice[2] = 2;

		d_along = Ctrl->L.min;
		while ((Ctrl->L.max - d_along) > (GMT_CONV_LIMIT*Ctrl->G.inc)) {
			p_data[P.n_used].a[2] = d_along;
			p_data[P.n_used].t = NULL;	/* Initialize since that is not done by realloc */
			p_data[P.n_used].z = NULL;	/* Initialize since that is not done by realloc */
			P.n_used++;
			d_along = Ctrl->L.min + P.n_used * Ctrl->G.inc;
			if (P.n_used == (n_alloc-1)) {
				n_alloc <<= 1;
				p_data = GMT_memory (GMT, p_data, n_alloc, struct PROJECT_DATA);
			}
		}
		p_data[P.n_used].a[2] = Ctrl->L.max;
		p_data[P.n_used].t = NULL;	/* Initialize since that is not done by realloc */
		p_data[P.n_used].z = NULL;	/* Initialize since that is not done by realloc */
		P.n_used ++;

		/* We need to find r,s  */

		if (Ctrl->N.active) {
			sincosd (90.0 + theta, &sin_theta, &cos_theta);
			for (i = 0; i < P.n_used; i++) {
				p_data[i].a[4] = Ctrl->C.x + p_data[i].a[2] * cos_theta;
				p_data[i].a[5] = Ctrl->C.y + p_data[i].a[2] * sin_theta;
			}
		}
		else {
			GMT_geo_to_cart (GMT, Ctrl->C.y, Ctrl->C.x, x, TRUE);
			for (i = 0; i < P.n_used; i++) {
				make_euler_matrix (P.pole, e, sign * p_data[i].a[2] / sin_lat_to_pole);
				matrix_3v (e,x,xt);
				GMT_cart_to_geo (GMT, &(p_data[i].a[5]), &(p_data[i].a[4]), xt, TRUE);
			}
		}

		/* At this stage, all values are still in degrees.  */

		if (Ctrl->Q.active) {
			for (i = 0; i < P.n_used; i++) {
				p_data[i].a[2] *= GMT->current.proj.DIST_KM_PR_DEG;
				p_data[i].a[3] *= GMT->current.proj.DIST_KM_PR_DEG;
			}
		}

		/* Now output generated track */

		if (!GMT->common.b.active[GMT_OUT]) {
			if (GMT->current.io.io_header[GMT_OUT]) GMT_fprintf (GMT->session.std[GMT_OUT], "lon\tlat\tdist\n");

			for (i = 0; i < P.n_used; i++) {
				for (j = 0; j < P.n_outputs; j++) out[j] = p_data[i].a[P.output_choice[j]];
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
			}
		}
	}
	else {	/* Must read input file */

		/* Specify input and output expected columns */
		if ((error = GMT_set_cols (GMT, GMT_IN, 0)) != GMT_OK) {
			Return (error);
		}

		/* Initialize the i/o since we are doing record-by-record reading/writing */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Establishes data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC) != GMT_OK) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		rmode = (pure_ascii && GMT_get_cols (GMT, GMT_IN) >= 2) ? GMT_READ_MIXED : GMT_READ_DOUBLE;

		while ((in = GMT_Get_Record (API, rmode, &n_fields))) {	/* Keep returning records until we reach EOF */

			if (GMT_REC_IS_ERROR (GMT) && n_fields < 2) continue;

			if (GMT_REC_IS_TBL_HEADER (GMT)) {	/* Echo out headers */
				GMT_Put_Record (API, GMT_WRITE_HEADER, NULL);
				continue;
			}
			if (GMT_REC_IS_SEG_HEADER (GMT)) {	/* Echo out results from previous segment and next segment header */
				if (P.n_used) {	/* Write out previous segment */
					if ((error = write_one_segment (GMT, Ctrl, theta, p_data, &P))) Return (error);
					n_total_used += P.n_used;
					P.n_used = 0;
				}
				GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
				continue;
			}

			/* We come here if we have data records */

			if (z_first) {
				P.n_z = GMT_get_cols (GMT, GMT_IN) - 2;
				if (P.n_z == 0 && P.want_z_output) {
					GMT_report (GMT, GMT_MSG_FATAL, "No data columns, cannot use z flag in -F\n");
					Return (EXIT_FAILURE);
				}
				z_first = FALSE;
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
#ifdef DEBUG
				GMT_memtrack_off (GMT, GMT_mem_keeper);	/* Because it gives way too many pointers to search through */
#endif
				if (pure_ascii) {	/* Must store all text beyond x,y columns */
					p_data[P.n_used].t = GMT_memory (GMT, NULL, strlen (GMT->current.io.current_record), char);
					copy_text_from_col3 (GMT->current.io.current_record, p_data[P.n_used].t);
				}
				else {
					p_data[P.n_used].z = GMT_memory (GMT, NULL, P.n_z, double);
					GMT_memcpy (p_data[P.n_used].z, &in[GMT_Z], P.n_z, double);
				}
#ifdef DEBUG
				GMT_memtrack_on (GMT, GMT_mem_keeper);
#endif
			}
			P.n_used++;
			if (P.n_used == n_alloc) {
				n_alloc <<= 1;
				p_data = GMT_memory (GMT, p_data, n_alloc, struct PROJECT_DATA);
			}
		}

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

	GMT_report (GMT, GMT_MSG_NORMAL, "%ld read, %ld used\n", n_total_read, n_total_used);

#ifdef DEBUG
	if (P.n_z) GMT_memtrack_off (GMT, GMT_mem_keeper);	/* Free pointers that were allocated when tracking was off */
#endif
	for (i = 0; i < n_alloc; i++) {
		if (p_data[i].t) GMT_free (GMT, p_data[i].t);
		if (p_data[i].z) GMT_free (GMT, p_data[i].z);
	}
#ifdef DEBUG
	if (P.n_z) GMT_memtrack_on (GMT, GMT_mem_keeper);	/* Continue memory tracking */
#endif

	GMT_free (GMT, p_data);

	Return (GMT_OK);
}
