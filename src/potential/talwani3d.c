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
 * Authord:     Paul Wessel and Seung-Sep Kim
 * Date:        10-JUL-2015
 *
 *
 * Calculates gravity due to 3-D shapes based on contours of
 * horizontal slices through the body [or bodies].  The contour
 * levels are expected to be the same for all pieces of a multi-
 * body feature.
 * This version has been tested on a stack of disks (simulating a
 * sphere) and gave the correct answer.  It expects all distances
 * to be in meters (you can override with the -M option) or to be
 * in degrees lon/lat (will scale for a flat earth) and densities
 * is expected to be in kg/m^3.
 *
 * Based on methods by
 *
 * Kim, S.-S., and P. Wessel, New analytic solutions for modeling
 *    vertical gravity gradient anomalies, Geochem. Geophys. Geosyst.,
 *    17, 2016, doi:10.1002/2016GC006263. [for VGG and geoid]
 * M. Talwani and M. Ewing, Rapid Computation of gravitational
 *    attraction of three-dimensional bodies of arbitrary shape,
 *    Geophysics, 25, 203-225, 1960. [for gravity]
 *
 * Accelerated with OpenMP; see -x.
 */

#include "gmt_dev.h"
#include "talwani.h"

#define THIS_MODULE_CLASSIC_NAME	"talwani3d"
#define THIS_MODULE_MODERN_NAME	"talwani3d"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Compute geopotential anomalies over 3-D bodies by the method of Talwani"
#define THIS_MODULE_KEYS	"<D{,ND(,ZG(,G?},GDN"
#define THIS_MODULE_NEEDS	"r"
#define THIS_MODULE_OPTIONS "-VRdefhior" GMT_ADD_x_OPT

#define DX_FROM_DLON(x1, x2, y1, y2) (((x1) - (x2)) * DEG_TO_KM * cos (0.5 * ((y1) + (y2)) * D2R))
#define DY_FROM_DLAT(y1, y2) (((y1) - (y2)) * DEG_TO_KM)
#define gmt_M_is_zero2(x) (fabs (x) < 2e-5)	/* Check for near-zero angles [used in geoid integral()]*/

#define GMT_TALWANI3D_N_DEPTHS GMT_BUFSIZ	/* Max depths allowed due to OpenMP needing stack array */

#if 0
/* this is for various debug purposes and will eventually be purged */
bool dump = false;
FILE *fp = NULL;
#endif

struct TALWANI3D_CTRL {
	struct A {	/* -A Set positive up  */
		bool active;
	} A;
	struct D {	/* -D<rho> fixed density to override model files */
		bool active;
		double rho;
	} D;
	struct F {	/* -F[f|n[<lat>]|v] */
		bool active, lset;
		unsigned int mode;
		double lat;
	} F;
	struct G {	/* Output file */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct M {	/* -Mh|z  */
		bool active[2];	/* True if km, else m */
	} M;
	struct N {	/* Desired output points */
		bool active;
		char *file;
	} N;
	struct Z {	/* Observation level file or constant */
		bool active;
		double level;
		unsigned int mode;
		char *file;
	} Z;
};

struct SLICE {	/* Holds a single contour slice and its density, plus link to next slice at the same depth */
	struct SLICE *next;
	int n;
	double rho;
	double *x, *y;
};

struct CAKE {	/* Holds linked list of slices for same depth */
	struct SLICE *first_slice;
	double depth;
};

enum Talwani3d_fields {
	TALWANI3D_FAA	= 0,
	TALWANI3D_VGG,
	TALWANI3D_GEOID,
	TALWANI3D_HOR=0,
	TALWANI3D_VER=1
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct TALWANI3D_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct TALWANI3D_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->F.lat = 45.0;	/* So we compute normal gravity at 45 */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct TALWANI3D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->N.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->Z.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct TALWANI3D_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int k, n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {		/* Process all the options given */
		switch (opt->option) {

			case '<':	/* Input file(s) */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			case 'A':	/* Specify z-axis is positive up [Default is down] */
				Ctrl->A.active = true;
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.rho = atof (opt->arg);
				break;
			case 'F':
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'v': Ctrl->F.mode = TALWANI3D_VGG; 	break;
					case 'n': Ctrl->F.mode = TALWANI3D_GEOID;
						if (opt->arg[1]) Ctrl->F.lat = atof (&opt->arg[1]), Ctrl->F.lset = true;
						break;
					case 'g':  Ctrl->F.mode = TALWANI3D_FAA; 	break;
					default: 
						GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Syntax error -F: Unrecognized field %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'G':
				Ctrl->G.active = true;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'M':	/* Length units */
				k = 0;
				while (opt->arg[k]) {
					switch (opt->arg[k]) {
						case 'h': Ctrl->M.active[TALWANI3D_HOR] = true; break;
						case 'z': Ctrl->M.active[TALWANI3D_VER] = true; break;
						default:
							n_errors++;
							GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Syntax error -M: Unrecognized modifier %c\n", opt->arg[k]);
							break;
					}
					k++;
				}
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.file = strdup (opt->arg);
				break;
			case 'Z':
				Ctrl->Z.active = true;
				if (!gmt_access (GMT, opt->arg, F_OK)) {	/* file exists */
					Ctrl->Z.file = strdup (opt->arg);
					Ctrl->Z.mode = 1;
				}
				else {
					Ctrl->Z.mode = 0;
					Ctrl->Z.level = atof (opt->arg);
				}
				break;
			default:
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	if (GMT->common.R.active[RSET]) {
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[ISET],
		                                 "Syntax error -R option: Must specify both -R and -I (and optionally -r)\n");
	}
	n_errors += gmt_M_check_condition (GMT, (GMT->common.R.active[RSET] && GMT->common.R.active[ISET]) && Ctrl->Z.mode == 1,
	                                 "Syntax error -Z option: Cannot also specify -R -I\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->N.active && !Ctrl->G.active,
	                                 "Syntax error -G option: Must specify output gridfile name.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && !Ctrl->G.file,
	                                 "Syntax error -G option: Must specify output gridfile name.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && !Ctrl->N.file,
	                                 "Syntax error -N option: Must specify output gridfile name.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <modelfile> [-A] [-D<rho>] [-Ff|n[<lat>]|v] [-G<outfile>] [%s]\n", name, GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-M[hz]] [-N<trktable>] [%s] [-Z<level>]  [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s]%s [%s]\n\n", GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_x_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<modelfile> is a multiple-segment ASCII file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A The z-axis is positive upwards [Default is down].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set a fixed density contrast that overrides settings in model file (in kg/m^3).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify desired geopotential field component:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f = Free-air anomalies (mGal) [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n = Geoid anomalies (meter).  Optionally append <lat> for evaluation of normal gravity\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       [Default lat is mid-grid for grid output or mid-latitue if -N is used].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   v = Vertical Gravity Gradient anomalies (VGG; 1 Eovtos = 0.1 mGal/km).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Output data. Give name of output file.\n");
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Set units used, as follows:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Mh indicates all x/y-distances are in km [meters]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Mz indicates all z-distances are in km [meters]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set output locations where a calculation is requested.  No grid\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   is produced and output (x,y,z,g|n|v) is written to stdout.\n");
	GMT_Option (API, "R,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Set observation level for output locations [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append either a constant or the name of gridfile with levels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If given a grid then it also defines the output grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot use both -Z<grid> and -R -I [-r].\n");
	GMT_Option (API, "d,e");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Map units (lon, lat in degree, else in m [but see -Mh]).\n");
	GMT_Option (API, "h,i,o,r,x,.");
	return (GMT_MODULE_USAGE);
}

#define INV_3 (1.0/3.0)

GMT_LOCAL double parint (double x[], double y[], int n) {
	/* parint is a piecewise parabolic integrator
	 *
	 * Arguments:
	 *       x and y are arrays of size n   (sent)
	 *       area is area under y = f(x) from x(1) to x(n) (returned)
	 *
	 * Method:
	 *       We approximate y = f(x) by a seies of parabolas, each
	 *       an exact fit to three points, at x(i-1), x(i), x(i+1).
	 *       The area under this curve in the region about x(i) is
	 *       summed as we increment i.  The region about x(i) is
	 *       defined as extending from x1 to x2, where
	 *         x1 = (x(i-1) + x(i)) / 2
	 *         x2 = (x(i) + x(i+1)) / 2
	 *       and the area piece at i is the area under the parabola
	 *       in the region  x2 - x1.  If i is 2 or n-1, however, we
	 *       set x1 = x(1) or x2 = x(n) to cover the entire interval of x.
	 *
	 * Programmer:  W.H.F. Smith,  30-AUG-1986 in Fortran.
	 * C-version by Paul Wessel, 3/4/91.
	 *
	 * Remarks:  This replaces PRBINT, which performed the same operation
	 * by a series of subroutine calls to PARAB and EVALCU.
	 * PARAB finds the best fit least squares parabola through n points;
	 * since in the integrator we always fit each parabola through only
	 * three points, an exact analytical solution exists.
	 *
	 *      It seems that the user must watch the sign of area himself.
	 * If, for example, y(x) is a positive function, but the array x
	 * is in descending order, the value of AREA is negative.  However,
	 * the code below is faithful to the sign of y and the order of x
	 * as written, so if the user desires that area should have same
	 * sign as y, regardless of order of x, then user should test x in
	 * calling program and adjust sign of area accordingly.
	 *
	 * If n == 2 then a simple trapezoidal integration is returned.
	 */

	int i, ip1, im1;
	double c2top, c2bot, c2, c1, c0, x1, x2, x1_2, x2_2, area;
	
	if (n == 2) return (0.5 * (x[1] - x[0]) * (y[0] + y[1])); /* Linear integration */
	 
	area = 0.0;
	for (i = 1; i < n-1; i++) {
		im1 = i - 1;	ip1 = i + 1;
		c2top = (y[i-1] - y[i]) * (x[im1] - x[ip1]) - (y[im1] - y[ip1]) * (x[im1] - x[i]);
		c2bot = (x[im1] - x[i]) * (x[i] - x[ip1]) * (x[im1] - x[ip1]);
		c2 = c2top / c2bot;
		c1 = (y[im1] - y[i]) / (x[im1] - x[i]) - c2 * (x[im1] + x[i]);
		c0 = y[im1] - c2 * pow (x[im1], 2.0) - c1 * x[im1];
		x1 = (i == 1) ? x[0] : 0.5 * (x[im1] + x[i]);
		x2 = (i == n-2) ? x[n-1] : 0.5 * (x[ip1] + x[i]);
		x1_2 = x1 * x1;
		x2_2 = x2 * x2;
		c1 *= 0.5;
		c2 *= INV_3;
		area += (c0 * (x2 - x1) + c1 * (x2_2 - x1_2) + c2 * (x2 * x2_2 - x1 * x1_2));
	}
	return (area);
}

GMT_LOCAL double get_grav3d (double x[], double y[], int n, double x_obs, double y_obs, double z_obs, double rho, bool flat) {
	/* Talwani et al., 1959 */
	int k;
	double gsum = 0.0, x1, x2, y1, y2, r1, r2, ir1, ir2, xr1 = 0.0, yr1 = 0.0, side, iside;
	double xr2 = 0.0, yr2 = 0.0, dx, dy, p, em, sign2, wsign, value, part1 = 0.0, part2 = 0.0, part3 = 0.0, q, f, psi;
	bool zerog;

	/* Get x- and y-distances of first point relative to observation point */
	if (flat) {	/* Got lon, lat and must convert to Flat-Earth km */
		x1 = DX_FROM_DLON (x[0], x_obs, y[0], y_obs);
		y1 = DY_FROM_DLAT (y[0], y_obs);
	}
	else {	/* Got km (or m) */
		x1 = x[0] - x_obs;
		y1 = y[0] - y_obs;
	}
	r1 = hypot (x1, y1);
	if (r1 != 0.0) {
		ir1 = 1.0 / r1;
		xr1 = x1 * ir1;
		yr1 = y1 * ir1;
	}
			
	for (k = 1; k < n; k++) {	/* Loop over vertices */
		/* Get coordinates of next vectex */
		if (flat) {
			x2 = DX_FROM_DLON (x[k], x_obs, y[k], y_obs);
			y2 = DY_FROM_DLAT (y[k], y_obs);
		}
		else {
			x2 = x[k] - x_obs;
			y2 = y[k] - y_obs;
		}
		r2 = hypot (x2, y2);
		if (r2 == 0.0)
			zerog = true;
		else {
			zerog = false;
			ir2 = 1.0 / r2;
			xr2 = x2 * ir2;
			yr2 = y2 * ir2;
			if (r1 == 0.0)
				zerog = true;
			else {
				dx = x1 - x2;
				dy = y1 - y2;
				side = hypot (dx, dy);
				iside = 1.0 / side;
				p = (dy * x1 - dx * y1) * iside;
				if (fabs (p) < TOL)
					zerog = true;
				else {
					sign2 = copysign (1.0, p);
					em = (yr1 * xr2) - (yr2 * xr1);
					if (em == 0.0)
						zerog = true;
					else {
						wsign = copysign (1.0, em);
						value = xr1*xr2 + yr1*yr2;
						part1 = wsign * d_acos (value);
						if (z_obs == 0.0)
							part2 = part3 = 0.0;
						else {
							psi = iside * z_obs * sign2 / hypot (p, z_obs);
							q = (dx*xr1 + dy*yr1) * psi;
							f = (dx*xr2 + dy*yr2) * psi;
							part2 = d_asin (q);
							part3 = d_asin (f);
						}
					}
				}
			}
		}
				
		if (!zerog) gsum += part1 - part2 + part3;
				
		/* Move this vertex to last vertex */
				
		x1 = x2;
		y1 = y2;
		r1 = r2;
		xr1 = xr2;
		yr1 = yr2;
	}
			
	/* If z axis is positive down, then gsum should have the same sign as z. */
				
	gsum = (z_obs > 0.0) ? fabs (gsum) : -fabs (gsum);
			
	return (GAMMA * rho * gsum);	/* Return contribution in mGal */
}

GMT_LOCAL double get_vgg3d (double x[], double y[], int n, double x_obs, double y_obs, double z_obs, double rho, bool flat) {
	/* Kim & Wessel, 2016 */
	int k;
	double vsum = 0.0, x1, x2, y1, y2, r1, r2, ir1, ir2, xr1 = 0.0, yr1 = 0.0, side, iside;
	double xr2 = 0.0, yr2 = 0.0, dx, dy, p, em, sign2, part2 = 0.0, part3 = 0.0, q, f, z2, p2;
	double scl, cos_theta_i, sin_theta2_i, cos_phi_i, sin_phi2_i, area = 0.0;
	bool zerog;

	/* Get x- and y-distances relative to observation point */
	if (flat) {	/* Got lon, lat and must convert to Flat-Earth km */
		x1 = DX_FROM_DLON (x[0], x_obs, y[0], y_obs);
		y1 = DY_FROM_DLAT (y[0], y_obs);
	}
	else {	/* Got km (or m) */
		x1 = x[0] - x_obs;
		y1 = y[0] - y_obs;
	}
	r1 = hypot (x1, y1);
	if (r1 != 0.0) {
		ir1 = 1.0 / r1;
		xr1 = x1 * ir1;
		yr1 = y1 * ir1;
	}
   
	for (k = 1; k < n; k++) {	/* Loop over vertices */
        	/* Get coordinates to the next vertex */
		if (flat) {
			x2 = DX_FROM_DLON (x[k], x_obs, y[k], y_obs);
			y2 = DY_FROM_DLAT (y[k], y_obs);
		}
		else {
			x2 = x[k] - x_obs;
			y2 = y[k] - y_obs;
		}
		r2 = hypot (x2, y2);
		if (r2 == 0.0)
			zerog = true;
		else {
			zerog = false;
			ir2 = 1.0 / r2;
			xr2 = x2 * ir2;
			yr2 = y2 * ir2;
			if (r1 == 0.0)
				zerog = true;
			else {
				dx = x1 - x2;
				dy = y1 - y2;
				area += dx * (y1 + y2);	/* Compute area of polygon slice since we need to know handedness */
				side = hypot (dx, dy);
				iside = 1.0 / side;
				p = (dy * x1 - dx * y1) * iside;
				if (fabs (p) < TOL)
					zerog = true;
				else {
					sign2 = copysign (1.0, p);
					em = (yr1 * xr2) - (yr2 * xr1);
					if (em == 0.0)
						zerog = true;
					else {
			                        q = (dx*xr1 + dy*yr1) * iside;
			                        f = (dx*xr2 + dy*yr2) * iside;
						cos_theta_i = q * sign2;
						cos_phi_i   = f * sign2;
						sin_theta2_i = 1.0 - cos_theta_i * cos_theta_i;
						sin_phi2_i   = 1.0 - cos_phi_i   * cos_phi_i;
						p2 = p * p;
						z2 = z_obs * z_obs;
						scl = p2 / (p2+z2);
						part2 = scl * (cos_theta_i / sqrt (sin_theta2_i * z2 + p2));
						part3 = scl * (cos_phi_i   / sqrt (sin_phi2_i   * z2 + p2));
					}
				}
			}
		}
       
		if (!zerog) vsum += -part2 + part3;
       
		/* Move this vertex to last vertex : */
       
		x1 = x2;
		y1 = y2;
		r1 = r2;
		xr1 = xr2;
		yr1 = yr2;
	}
	if (area < 0.0) vsum = -vsum;		/* 2*area will be negative or positive depending on handedness of polygon */
	return (10 * GAMMA * rho * vsum);	/* To get Eotvos = 0.1 mGal/km */
}

GMT_LOCAL double definite_integral (double a, double s, double c) {
	/* Return out definite integral n_ij (except the factor z_j) */
	/* Here, 0 <= a <= TWO_I and c >= 0 */
	double s2, c2, u2, k, k2, q, q2, f, v, n_ij, arg1, arg2, y;
	/* Deal with special cases */
	if (gmt_M_is_zero2 (a - M_PI_2)) return (0.0);
	else if (gmt_M_is_zero2 (a)) return (0.0);
	else if (gmt_M_is_zero2 (a - M_PI)) return (0.0);
	s2 = s * s;
	c2 = c * c;
	u2 = 1.0 / s2;
	k2 = c2 + 1.0;
	k = sqrt (k2);
	q2 = u2 - 1.0;
	q = sqrt (q2);
	f = sqrt (c2 + u2);
	v = f - k;
	y = 1 / (1 - s2);
	arg1 = 2.0*k2*v*y - f;
	arg2 = c*q;
	n_ij = atan2 (v, 2.0*c*q) - atan2 (arg1, arg2) - 2.0 * atanh (v/q) / c;
#if 0
	if (dump) fprintf (fp, "%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\n",
		a, s, c, c2, k, k2, u2, q2, f, v, arg1, arg2, n_ij);
#endif
	if (a > M_PI_2) n_ij = -n_ij;
	if (gmt_M_is_dnan (n_ij))
		fprintf (stderr, "definite_integral returns n_ij = NaN!\n");
	return (n_ij);
}

GMT_LOCAL double integral (double a, double sa, double b, double sb, double c) {
	/* Return integral of geoid function from a to b */
	return (definite_integral (b, sb, c) - definite_integral (a, sa, c));
}

GMT_LOCAL double get_geoid3d (double x[], double y[], int n, double x_obs, double y_obs, double z_obs, double rho, bool flat, double G0) {
	/* Kim & Wessel, 2016 */
	int k;
	double nsum = 0.0, x1, x2, y1, y2, r1, r2, ir1, ir2, xr1 = 0.0, yr1 = 0.0, side, iside, c, z_j = z_obs;
	double xr2 = 0.0, yr2 = 0.0, dx, dy, p_i, theta_i, sign2, part1 = 0.0, part2 = 0.0, fi_i, em, del_alpha, s_fi, s_th;
	bool zerog;
	/* Coordinates are in km and g/cm^3 */
	/* Get x- and y-distances relative to observation point */
	if (flat) {	/* Got lon, lat and must convert to Flat-Earth km */
		x1 = DX_FROM_DLON (x[0], x_obs, y[0], y_obs);
		y1 = DY_FROM_DLAT (y[0], y_obs);
	}
	else {	/* Got km (or m) */
		x1 = x[0] - x_obs;
		y1 = y[0] - y_obs;
	}
	r1 = hypot (x1, y1);
	if (r1 != 0.0) {
		ir1 = 1.0 / r1;
		xr1 = x1 * ir1;
		yr1 = y1 * ir1;
	}
	for (k = 1; k < n; k++) {	/* Loop over vertices */
		/* Get coordinates of next vertex */
		if (flat) {
			x2 = DX_FROM_DLON (x[k], x_obs, y[k], y_obs);
			y2 = DY_FROM_DLAT (y[k], y_obs);
		}
		else {
			x2 = x[k] - x_obs;
			y2 = y[k] - y_obs;
		}
		r2 = hypot (x2, y2);
		if (r2 == 0.0)
			zerog = true;
		else {
			zerog = false;
			ir2 = 1.0 / r2;
			xr2 = x2 * ir2;
			yr2 = y2 * ir2;
			if (r1 == 0.0)
				zerog = true;
			else {
				dx = x1 - x2;
				dy = y1 - y2;
				side = hypot (dx, dy);
				iside = 1.0 / side;
				p_i = (dy * x1 - dx * y1) * iside;
				if (fabs (p_i) < TOL || fabs (side) < TOL)
					zerog = true;
				else {
					em = (yr1 * xr2) - (yr2 * xr1);
					if (em == 0.0)
						zerog = true;
					else {
						/* When observation point is inside the polygon then del_alpha sums to -2*PI, else it is 0 */
						/* We then strip the sign from p_i since it is a physical length. */
						sign2 = copysign (1.0, p_i);
			                        fi_i    = d_acos (sign2 * (dx*xr1 + dy*yr1) * iside);
			                        theta_i = d_acos (sign2 * (dx*xr2 + dy*yr2) * iside);
						s_fi = p_i / r1;
						s_th = p_i / r2;
						del_alpha = theta_i - fi_i;
						c = z_j / fabs (p_i);
						part1 = integral (fi_i, s_fi, theta_i, s_th, c);
						part2 = z_j * (part1 - del_alpha);
#if 0
						if (dump) fprintf (stderr, "I(%g, %g, %g) = %g [z = %g p_i = %g, da = %g dx = %g dy = %g]\n", R2D*(fi_i), R2D*(theta_i), c, part2, z_j, p_i, del_alpha, dx, dy);
#endif
					}
				}
			}
		}
		if (!zerog) nsum += part2;
				
		/* Move this vertex to last vertex */
				
		x1 = x2;
		y1 = y2;
		r1 = r2;
		xr1 = xr2;
		yr1 = yr2;
	}
	/* If z axis is positive down, then nsum should have the same sign as z */
				
	nsum = (z_j > 0.0) ? fabs (nsum) : -fabs (nsum);

	return (1.0e-2 * GAMMA * rho * nsum / G0);	/* To get geoid in meter */
}

GMT_LOCAL double get_one_output3D (double x_obs, double y_obs, double z_obs, struct CAKE *cake, double depths[], unsigned int ndepths, unsigned int mode, bool flat_earth, double G0) {
	/* Evaluate output at a single observation point (x,y,z) */
	/* Work array vtry must have at least of length ndepths */
	unsigned int k;
	double z;
	struct SLICE *sl = NULL;
	double vtry[GMT_TALWANI3D_N_DEPTHS];	/* Allocate on stack since trouble with OpenMP otherwise */
#if 0
	/* Debug stuff that will eventually go away after more testing */
	char file[32] = {""};
	if (fabs (x_obs - 1.96) < 1e-5) {
		dump = true;
		sprintf (file, "dump.%g.txt", x_obs);
		fp = fopen (file, "w");
	}
	else if (fabs (x_obs - 2.0) < 1e-5) {
		dump = true;
		sprintf (file, "dump.%g.txt", x_obs);
		fp = fopen (file, "w");
	}
	else {
		if (fp) {fclose (fp); fp = NULL;}
		dump = false;
	}
	if (dump)
		k = 0;
#endif
	for (k = 0; k < ndepths; k++) {
		vtry[k] = 0.0;
		z = cake[k].depth - z_obs;	/* Vertical distance from observation point to this slice */
		for (sl = cake[k].first_slice; sl; sl = sl->next) {	/* Loop over slices */
			if (mode == TALWANI3D_FAA) /* FAA */
				vtry[k] += get_grav3d  (sl->x, sl->y, sl->n, x_obs, y_obs, z, sl->rho, flat_earth);
			else if (mode == TALWANI3D_GEOID) /* GEOID */
				vtry[k] += get_geoid3d (sl->x, sl->y, sl->n, x_obs, y_obs, z, sl->rho, flat_earth, G0);
			else /* VGG */
				vtry[k] += get_vgg3d   (sl->x, sl->y, sl->n, x_obs, y_obs, z, sl->rho, flat_earth);
		}
	}
	return (parint (depths, vtry, ndepths));	/* Use parabolic integrator and return value */
}

GMT_LOCAL int comp_cakes (const void *cake_a, const void *cake_b) {
	/* Used in the sorting of layers on depths */
	const struct CAKE *a = cake_a, *b = cake_b;
	if (a->depth < b->depth) return (-1);
	if (a->depth > b->depth) return (+1);
	return (0);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_talwani3d (void *V_API, int mode, void *args) {
	int error = 0, ns;
	unsigned int k, tbl, seg, ndepths = 0, n = 0, dup_node = 0, n_duplicate = 0;
	uint64_t node;
	size_t n_alloc = 0, n_alloc1 = 0;

	bool flat_earth = false, first_slice = true;

	char *uname[2] = {"meter", "km"}, *kind[3] = {"FAA", "VGG", "GEOID"}, remark[GMT_LEN64] = {""};
	double z_level, depth = 0.0, rho = 0.0, lat = 45.0, G0;
	double *x = NULL, *y = NULL, *in = NULL, *depths = NULL;
				
	struct SLICE *sl = NULL, *slnext = NULL;
	struct CAKE *cake = NULL;
	struct TALWANI3D_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_RECORD *In = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the talwani3d main code ----------------------------*/

	gmt_enable_threads (GMT);	/* Set number of active threads, if supported */
	/* Specify input expected columns to be at least 2 */
	if ((error = GMT_Set_Columns (API, GMT_IN, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	/* Register likely model files unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POLYGON, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	/* Initialize the i/o for doing record-by-record reading */
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (Ctrl->Z.mode == 1) {	/* Got grid with observation levels which also sets output locations; it could also set -fg so do this first */
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->Z.file, NULL)) == NULL)
			Return (API->error);
		if (gmt_M_is_geographic (GMT, GMT_IN)) lat = 0.5 * (G->header->wesn[YLO] + G->header->wesn[YHI]);
	}
	else if (GMT->common.R.active[RSET]) {	/* Gave -R -I [-r] and possibly -fg indirectly via geographic coordinates in -R */
		if ((G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL,
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL)
			Return (API->error);
		if (gmt_M_is_geographic (GMT, GMT_IN)) lat = 0.5 * (G->header->wesn[YLO] + G->header->wesn[YHI]);
	}
	else {	/* Got a dataset with output locations via -N */
		gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from the -N file */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->N.file, NULL)) == NULL)
			Return (API->error);
		if (D->n_columns < 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input file %s has %d column(s) but at least 2 are needed\n", Ctrl->N.file, (int)D->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
		if (gmt_M_is_geographic (GMT, GMT_IN)) lat = 0.5 * (D->min[GMT_Y] + D->max[GMT_Y]);
	}

	flat_earth = gmt_M_is_geographic (GMT, GMT_IN);		/* If true then input is in degrees and we must convert to km later on */

	if (flat_earth && Ctrl->M.active[TALWANI3D_HOR]) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error -M: Cannot specify both geographic coordinates (degrees) AND -Mh\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->A.active) Ctrl->Z.level = -Ctrl->Z.level;

	/* Read polygon information from multiple segment file */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "All x/y-values are assumed to be given in %s\n", uname[Ctrl->M.active[TALWANI3D_HOR]]);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "All z-values are assumed to be given in %s\n",   uname[Ctrl->M.active[TALWANI3D_VER]]);

	/* Set up cake slice array and pointers */

	n_alloc1 = GMT_CHUNK;
	cake = gmt_M_memory (GMT, NULL, n_alloc1, struct CAKE);

	/* Read the sliced model */
	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) { 		/* Bail if there are any read errors */
				gmt_M_free (GMT, cake);
				Return (GMT_RUNTIME_ERROR);
			}
			if (gmt_M_rec_is_table_header (GMT)) 	/* Skip all table headers */
				continue;
			if (gmt_M_rec_is_segment_header (GMT) || gmt_M_rec_is_eof (GMT)) {	/* Process segment headers or end-of-file */
				/* First close previous segment */
				if (!first_slice) {
					if (!(x[n-1] == x[0] && y[n-1] == y[0])) {	/* Copy first point to last */
						if (n_duplicate == 1 && dup_node == n) n_duplicate = 0;	/* So it was the last == first duplicate; reset count */
						x[n] = x[0];	y[n] = y[0];
						n++;
					}
					x = gmt_M_memory (GMT, x, n, double);
					y = gmt_M_memory (GMT, y, n, double);
					k = 0;
					while (k < ndepths && depth != cake[k].depth) k++;	/* Get the cake index for this depth */

					if (k == ndepths) {	/* New depth, must allocate another cake */
						if (ndepths == n_alloc1) {
							n_alloc1 <<= 1;
							cake = gmt_M_memory (GMT, cake, n_alloc1, struct CAKE);
						}
						cake[k].depth = depth;
						cake[k].first_slice = gmt_M_memory (GMT, NULL, 1, struct SLICE);
						cake[k].first_slice->rho = rho;
						cake[k].first_slice->x = x;
						cake[k].first_slice->y = y;
						cake[k].first_slice->n = n;
						ndepths++;
					}
					else {	/* Hook onto existing list of slices at same depth */
						sl = cake[k].first_slice;
						while (sl->next) sl = sl->next;	/* Get to end of the slices */
						sl->next = gmt_M_memory (GMT, NULL, 1, struct SLICE);
						sl->next->rho = rho;
						sl->next->x = x;
						sl->next->y = y;
						sl->next->n = n;
					}
				}
				first_slice = false;
				if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
					break;
				/* Process the next segment header */
				ns = sscanf (GMT->current.io.segment_header, "%lf %lf", &depth, &rho);
				if (ns == 1 && !Ctrl->D.active) {
					GMT_Report (API, GMT_MSG_NORMAL, "Neither segment header nor -D specified density - must quit\n");
					gmt_M_free (GMT, cake);
					Return (API->error);
				}
				if (Ctrl->D.active) rho = Ctrl->D.rho;
				rho *= 0.001;	/* Change density to g/cm3 */
				if (!Ctrl->M.active[TALWANI3D_VER]) depth /= METERS_IN_A_KM;	/* Change depth to km */
				if (Ctrl->A.active) depth = -depth;	/* Make positive down */
				/* Allocate array for this slice */
				n_alloc = GMT_CHUNK;
				x = gmt_M_memory (GMT, NULL, n_alloc, double);
				y = gmt_M_memory (GMT, NULL, n_alloc, double);
				n = 0;
				continue;
			}
			assert (false);						/* Should never get here */
		}
		if (first_slice) {	/* Did not have the required header record */
			GMT_Report (API, GMT_MSG_NORMAL, "No segment header with depth [and optional densithy contrast] - must quit\n");
			gmt_M_free (GMT, cake);
			Return (API->error);
		}
	
		/* Clean data record to process */

		in = In->data;	/* Only need to process numerical part here */
		if (n && (x[n-1] == x[n] && y[n-1] == y[n])) {	/* Maybe a duplicate point - or it could be the repeated last = first */
			n_duplicate++;
			dup_node = n;
		}
		else {	/* New point for sure */
			x[n] = in[GMT_X];	y[n] = in[GMT_Y];
			if (!flat_earth) {
				if (!Ctrl->M.active[TALWANI3D_HOR]) {	/* Change distances to km */
					x[n] /= METERS_IN_A_KM;
					y[n] /= METERS_IN_A_KM;
				}
			}
			n++;
			if (n == n_alloc) {
				n_alloc += GMT_CHUNK;
				x = gmt_M_memory (GMT, x, n_alloc, double);
				y = gmt_M_memory (GMT, y, n_alloc, double);
			}
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		gmt_M_free (GMT, cake);
		Return (API->error);
	}

	if (ndepths >= GMT_TALWANI3D_N_DEPTHS) {
		GMT_Report (API, GMT_MSG_NORMAL, "Model cannot have more than %d depth layer\n", GMT_TALWANI3D_N_DEPTHS);
		GMT_Report (API, GMT_MSG_NORMAL, "You must increase GMT_TALWANI3D_N_DEPTHS and recompile\n");
		gmt_M_free (GMT, cake);
		Return (GMT_RUNTIME_ERROR);
	}

	/* Finish allocation and sort on layers */

	cake = gmt_M_memory (GMT, cake, ndepths, struct CAKE);
	qsort (cake, ndepths, sizeof (struct CAKE), comp_cakes);

	if (n_duplicate) GMT_Report (API, GMT_MSG_VERBOSE, "Ignored %u duplicate vertices\n", n_duplicate);

	/* Now we can write (if -V) to the screen the user's polygon model characteristics. */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "# of depths: %d\n", ndepths);
	if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) {	/* Give a listing of layers found */
	 	for (k = 0; k < ndepths; k++) {
	 		for (sl = cake[k].first_slice; sl; sl = sl->next)
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Depth: %lg Rho: %lg N-vertx: %4d\n",
	 				cake[k].depth, sl->rho, sl->n);
	 	}
	}
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Start calculating %s\n", kind[Ctrl->F.mode]);

	G0 = (Ctrl->F.lset) ? g_normal (Ctrl->F.lat) : g_normal (lat);
	/* Set up depths array needed by get_one_output3D */
	depths = gmt_M_memory (GMT, NULL, ndepths, double);
	for (k = 0; k < ndepths; k++) depths[k] = cake[k].depth;	/* Used by the parabolic integrator */
	if (Ctrl->N.active) {	/* Single loop over specified output locations */
		unsigned int wmode = GMT_ADD_DEFAULT;
		double scl = (!(flat_earth || Ctrl->M.active[TALWANI3D_HOR])) ? (1.0 / METERS_IN_A_KM) : 1.0;	/* Perhaps convert to km */
		double out[4];
		struct GMT_RECORD *Rec = gmt_new_record (GMT, out, NULL);
		/* Must register Ctrl->G.file first since we are going to writing rec-by-rec */
		if (Ctrl->G.active) {
			int out_ID;
			if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, NULL, Ctrl->G.file)) == GMT_NOTSET)
				Return (API->error);
			wmode = GMT_ADD_EXISTING;
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, 4, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			gmt_M_free (GMT, depths);
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, wmode, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			gmt_M_free (GMT, depths);
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			gmt_M_free (GMT, depths);
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			gmt_M_free (GMT, depths);
			Return (API->error);
		}
		if (D->n_segments > 1) gmt_set_segmentheader (GMT, GMT_OUT, true);
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
				int64_t row;
				S = D->table[tbl]->segment[seg];	/* Current segment */
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, S->header);
				gmt_prep_tmp_arrays (GMT, GMT_OUT, S->n_rows, 1);	/* Init or reallocate tmp vector */
#ifdef _OPENMP
				/* Spread calculation over selected cores */
#pragma omp parallel for private(row,z_level) shared(GMT,Ctrl,S,scl,cake,depths,ndepths,flat_earth, G0)
#endif
				/* Separate the calculation from the output in two separate row-loops since cannot do rec-by-rec output
				 * with OpenMP due to race condiations that would mess up the output order */
				for (row = 0; row < (int64_t)S->n_rows; row++) {	/* Calculate attraction at all output locations for this segment */
					z_level = (S->n_columns == 3 && !Ctrl->Z.active) ? S->data[GMT_Z][row] : Ctrl->Z.level;	/* Default observation z level unless provided in input file */
					GMT->hidden.mem_coord[GMT_X][row] = get_one_output3D (S->data[GMT_X][row] * scl, S->data[GMT_Y][row] * scl, z_level, cake, depths, ndepths, Ctrl->F.mode, flat_earth, G0);
				}
				/* This loop is not under OpenMP */
				out[GMT_Z] = Ctrl->Z.level;	/* Default observation z level unless provided in input file */
				for (row = 0; row < (int64_t)S->n_rows; row++) {	/* Loop over output locations */
					out[GMT_X] = S->data[GMT_X][row];
					out[GMT_Y] = S->data[GMT_Y][row];
					if (S->n_columns == 3 && !Ctrl->Z.active) out[GMT_Z] = S->data[GMT_Z][row];
					out[3] = GMT->hidden.mem_coord[GMT_X][row];
					GMT_Put_Record (API, GMT_WRITE_DATA, Rec);	/* Write this to output */
				}
			}
		}
		gmt_M_free (GMT, Rec);
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			gmt_M_free (GMT, depths);
			Return (API->error);
		}
	}
	else {	/* Dealing with a grid */
		int64_t row;
		int col, n_columns = (int)G->header->n_columns, n_rows = (int)G->header->n_rows;	/* To shut up compiler warnings */
		double y_obs, *x_obs = gmt_M_memory (GMT, NULL, G->header->n_columns, double);
		for (col = 0; col < n_columns; col++) {
			x_obs[col] = gmt_M_grd_col_to_x (GMT, col, G->header);
			if (!(flat_earth || Ctrl->M.active[TALWANI3D_HOR])) x_obs[col] /= METERS_IN_A_KM;	/* Convert to km */
		}
#ifdef _OPENMP
		/* Spread calculation over selected cores */
#pragma omp parallel for private(row,col,node,y_obs,z_level) shared(API,GMT,Ctrl,G,x_obs,cake,depths,ndepths,flat_earth, G0)
#endif
		for (row = 0; row < n_rows; row++) {	/* Do row-by-row and report on progress if -V */
			y_obs = gmt_M_grd_row_to_y (GMT, row, G->header);
			if (!(flat_earth || Ctrl->M.active[TALWANI3D_HOR])) y_obs /= METERS_IN_A_KM;	/* Convert to km */
#ifndef _OPENMP
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Finished row %5d\n", row);
#endif
			for (col = 0; col < (int)G->header->n_columns; col++) {
				/* Loop over cols; always save the next level before we update the array at that col */
				node = gmt_M_ijp (G->header, row, col);
				z_level = (Ctrl->A.active) ? -G->data[node] : G->data[node];	/* Get observation z level and possibly flip direction */
				G->data[node] = (gmt_grdfloat) get_one_output3D (x_obs[col], y_obs, z_level, cake, depths, ndepths, Ctrl->F.mode, flat_earth, G0);
			}
		}
		gmt_M_free (GMT, x_obs);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Create %s\n", Ctrl->G.file);
		sprintf (remark, "Calculated 3-D %s", kind[Ctrl->F.mode]);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, remark, G)) {
			gmt_M_free (GMT, depths);	gmt_M_free (GMT, cake);
			Return (API->error);
		}
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) {
			gmt_M_free (GMT, depths);	gmt_M_free (GMT, cake);
			Return (API->error);
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, G) != GMT_NOERROR) {
			gmt_M_free (GMT, depths);	gmt_M_free (GMT, cake);
			Return (API->error);
		}
	}
	
	/* Clean up memory */

	for (k = 0; k < ndepths; k++) {	/* Wind through the depthd and free up slices */
		sl = cake[k].first_slice;
		slnext = cake[k].first_slice->next;
		while (slnext) {
			gmt_M_free (GMT, sl->x);
			gmt_M_free (GMT, sl->y);
			gmt_M_free (GMT, sl);
			sl = slnext;
			slnext = sl->next;
		}
		gmt_M_free (GMT, sl->x);
		gmt_M_free (GMT, sl->y);
		gmt_M_free (GMT, sl);
	}
	gmt_M_free (GMT, cake);
	gmt_M_free (GMT, depths);

	Return (GMT_NOERROR);
}
