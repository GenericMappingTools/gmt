/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Author:      Paul Wessel and Seung-Sep Kim
 * Date:        10-JUL-2015
 *
 *
 * Calculates gravity due to 3-D shapes based on contours of
 * horizontal slices through the body [or bodies].  The contour
 * levels are expected to be the same for all pieces of a multi-
 * body feature.
 * This version has been tested on a stack of disks (simulating a
 * sphere) and gave the correct answer.  It expects all distances
 * to be in meters (you can override with the -K option) or to be
 * in degrees lon/lat (will scale for flat earth) and densities
 * to be in kg/m^3.
 *
 * Based on method by M. Talwani and M. Ewing, Rapid Computation
 *   of gravitational attraction of three-dimensional bodies of 
 *   arbitrary shape, Geophysics, 25, 203-225, 1960.
 * Extended to handle VGG by Kim & Wessel, 2015, in prep.
 * Accelerated with OpenMP; see -x.
 */

#define THIS_MODULE_NAME	"talwani3d"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Compute free-air anomalies or vertical gravity gradients over 3-D bodies"
#define THIS_MODULE_KEYS	"<DI,NDi,ZGi,GGo,>DO"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-VRfhior" GMT_ADD_x_OPT

#define TOL		1.0e-7
#define DEG_TO_KM	111.319490793
#define GAMMA 		6.673	/* Gravitational constant for distances in km and mass in kg/m^3 */
#define G0 		9.81	/* Normal gravity */

#define DX_FROM_DLON(x1, x2, y1, y2) (((x1) - (x2)) * DEG_TO_KM * cos (0.5 * ((y1) + (y2)) * D2R))
#define DY_FROM_DLAT(y1, y2) (((y1) - (y2)) * DEG_TO_KM)
#define GMT_IS_ZERO2(x) (fabs (x) < 1e-16)	/* Check for near zero numbers */

#define GMT_TALWANI3D_N_DEPTHS GMT_BUFSIZ	/* Max depths allowed due to OpenMP needing stack array */

bool dump = false;

struct TALWANI3D_CTRL {
	struct A {	/* -A positive up  */
		bool active;
	} A;
	struct D {	/* -D<rho> fixed density to override model files */
		bool active;
		double rho;
	} D;
	struct F {	/* -F[f|n|v] */
		bool active;
		unsigned int mode;
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

struct SLICE {
	struct SLICE *next;
	int n;
	double rho;
	double *x, *y;
};

struct CAKE {
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

void *New_talwani3d_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct TALWANI3D_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct TALWANI3D_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

void Free_talwani3d_Ctrl (struct GMT_CTRL *GMT, struct TALWANI3D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->N.file) free (C->N.file);	
	if (C->G.file) free (C->G.file);	
	if (C->Z.file) free (C->Z.file);	
	GMT_free (GMT, C);	
}

int GMT_talwani3d_parse (struct GMT_CTRL *GMT, struct TALWANI3D_CTRL *Ctrl, struct GMT_OPTION *options)
{
	unsigned int k, n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {		/* Process all the options given */
		switch (opt->option) {

			case '<':	/* Input file(s) */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			case 'A':	/* Specify z-axis is positive up [Default is down] */
				Ctrl->A.active = TRUE;
				break;
			case 'D':
				Ctrl->D.active = TRUE;
				Ctrl->D.rho = atof (opt->arg);
				break;
			case 'F':
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'v': Ctrl->F.mode = TALWANI3D_VGG; 	break;
					case 'n': Ctrl->F.mode = TALWANI3D_GEOID;	break;
					default:  Ctrl->F.mode = TALWANI3D_FAA; 	break;
				}
				break;
			case 'G':
				Ctrl->G.active = true;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
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
				if (!GMT_access (GMT, opt->arg, F_OK)) {	/* file exists */
					Ctrl->Z.file = strdup (opt->arg);
					Ctrl->Z.mode = 1;
				}
				else {
					Ctrl->Z.mode = 0;
					Ctrl->Z.level = atof (opt->arg);
				}
				break;
			default:
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	if (GMT->common.R.active && Ctrl->I.active)
		GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);
	n_errors += GMT_check_condition (GMT, (GMT->common.R.active && Ctrl->I.active) && Ctrl->Z.mode == 1,
	                                 "Syntax error -Z option: Cannot also specify -R -I\n");
	n_errors += GMT_check_condition (GMT, (GMT->common.R.active + Ctrl->I.active) == 1,
	                                 "Syntax error -R option: Must specify both -R and -I (and optionally -r)\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->N.active && !Ctrl->G.active,
	                                 "Syntax error -G option: Must specify output gridfile name.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int GMT_talwani3d_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: talwani3d <modelfile> [-A] [-D<rho>] [-Ff|v] [-G<outfile>] [%s]\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-M[hz]] [-N<trktable>] [%s] [-Z<level>] [%s] \n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);
	GMT_Message (API, GMT_TIME_NONE,"\t[%s]\n\t[%s] [%s] [%s]%s\n\n", GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_x_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<modelfile> is a multiple-segment ASCII file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A The z-axis is positive upwards [Default is down].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Sets fixed density contrast that overrides settings in model file, in kg/m^3.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify desired geopotential field:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f = Free-air anomalies (mGal) [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   v = Vertical Gravity Gradient (VGG; 1 Eovtos = 0.1 mGal/km).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Sets output grid file name (i.e., when -N is not specified).\n");
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t-M sets units used, as follows:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Mh indicates all x/y-distances are in km [meters]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Mz indicates all z-distances are in km [meters]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Sets output locations where a calculation is requested.  No grid\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   is produced and output (x,y,z,g|v) is written to stdout.\n");
	GMT_Option (API, "R,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Set observation level for output locations [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append either a constant or the name of gridfile with levels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If a grid then it also defines the output grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot use both -Z<grid> and -R -I [-r].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Map units (lon, lat in degree, else in m [but see -Mh]).\n");
	GMT_Option (API, "h,i,o,r,x,.");
	return (EXIT_FAILURE);
}

#define INV_3 (1.0/3.0)

double parint (double x[], double y[], int n)
{
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
	 * Programmer:  W.H.F. Smith,  30-AUG-1986.
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

double get_grav3d (double x[], double y[], int n, double x_obs, double y_obs, double z_obs, double rho, bool flat)
{
	int k;
	double gsum, x1, x2, y1, y2, r1, r2, ir1, ir2, xr1, yr1, side, iside;
	double xr2, yr2, dx, dy, p, em, sign2, wsign, value, part1, part2, part3, q, f, psi;
	bool zerog;
	
	gsum = 0.0;
	/* Get x- and y-distances relative to observation point */
	if (flat) {
		x1 = DX_FROM_DLON (x[0], x_obs, y[0], y_obs);
		y1 = DY_FROM_DLAT (y[0], y_obs);
	}
	else {
		x1 = x[0] - x_obs;
		y1 = y[0] - y_obs;
	}
	r1 = hypot (x1, y1);
	if (r1 != 0.0) {
		ir1 = 1.0 / r1;
		xr1 = x1 * ir1;
		yr1 = y1 * ir1;
	}
				
	for (k = 1; k < n; k++) {	/* Loop over vertex */
				
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
					
		/* move this vertex to last vertex : */
					
		x1 = x2;
		y1 = y2;
		r1 = r2;
		xr1 = xr2;
		yr1 = yr2;
	}
				
	/* If z axis is positive down, then gsum should have the same sign as z, */
				 
	gsum = (z_obs > 0.0) ? fabs (gsum) : -fabs (gsum);
				
	return (GAMMA * rho * gsum);
}

double get_vgg3d (double x[], double y[], int n, double x_obs, double y_obs, double z_obs, double rho, bool flat)
{	/* Now works, see talwani_pw.pdf */
	int k;
	double vsum, x1, x2, y1, y2, r1, r2, ir1, ir2, xr1, yr1, side, iside;
	double xr2, yr2, dx, dy, dz, p, em, sign2, part2, part3, q, f, z2, p2;
	double scl, cos_theta_i, sin_theta2_i, cos_phi_i, sin_phi2_i;
	bool zerog;
	
	dz = z_obs;
	vsum = 0.0;
	if (flat) {
		x1 = DX_FROM_DLON (x[0], x_obs, y[0], y_obs);
		y1 = DY_FROM_DLAT (y[0], y_obs);
	}
	else {
		x1 = x[0] - x_obs;
		y1 = y[0] - y_obs;
	}
	r1 = hypot (x1, y1);
	if (r1 != 0.0) {
		ir1 = 1.0 / r1;
		xr1 = x1 * ir1;
		yr1 = y1 * ir1;
	}
    
	for (k = 1; k < n; k++) {	/* Loop over vertex */
        
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
			                        q = (dx*xr1 + dy*yr1) * iside;
			                        f = (dx*xr2 + dy*yr2) * iside;
						cos_theta_i = q * sign2;
						cos_phi_i   = f * sign2;
						sin_theta2_i = 1.0 - cos_theta_i * cos_theta_i;
						sin_phi2_i   = 1.0 - cos_phi_i   * cos_phi_i;
						p2 = p * p;
						z2 = dz * dz;
						scl = p2 / (p2+z2);
						part2 = scl * (cos_theta_i / sqrt (sin_theta2_i * z2 + p2));
						part3 = scl * (cos_phi_i   / sqrt (sin_phi2_i   * z2 + p2));
					}
				}
			}
		}
        
		if (!zerog) vsum += -part2 + part3;
        
		/* move this vertex to last vertex : */
        
		x1 = x2;
		y1 = y2;
		r1 = r2;
		xr1 = xr2;
		yr1 = yr2;
	}
    
	return (10 * GAMMA * rho * vsum);	/* To get Eotvos = 0.1 mGal/km */
}

double definite_integral (double a, double c)
{	/* Return out definite integral n_i (except the factor p_i) */
	double s, c2, u2, k, q, q3, f, v, g;
	/* Deal with special cases */
	if (GMT_IS_ZERO (a - M_PI_2)) return (0.0);
	else if (GMT_IS_ZERO2 (a)) return (0.0);
	else if (GMT_IS_ZERO2 (a - M_PI)) return (0.0);
	s = sin (a);
	c2 = c * c;
	u2 = 1.0 / (s*s);
	k = sqrt (c2 + 1.0);
	q = sqrt (u2 - 1.0);
	q3 = q * (u2 - 1.0);
	f = sqrt (c2 + u2);
	v = f - k;
	g = c * atan (q) - 2.0 * atanh (v/q) + c * (atan2(v, 2.0*c*q) - atan2 (f + u2*(v*(1.0 - 2.0*c2)-k), c*q3));
	if (a > M_PI_2) g = -g;
	if (GMT_is_dnan (g))
		fprintf (stderr, "definite_integral returns NaN!\n");
	return (g);
}

double integral (double a, double b, double c)
{
	return (definite_integral (b, c) - definite_integral (a, c));
}

double get_geoid3d (double x[], double y[], int n, double x_obs, double y_obs, double z_obs, double rho, bool flat)
{	/* Experimental and wrong so far */
	int k;
	double vsum, x1, x2, y1, y2, r1, r2, ir1, ir2, xr1, yr1, side, iside;
	double xr2, yr2, dx, dy, p, theta_i, sign2, part1, part2, fi_i;
	bool zerog;
	/* Coordinates are in km and g/cm^3  - recover SI units */
	vsum = 0.0;
	if (flat) {
		x1 = DX_FROM_DLON (x[0], x_obs, y[0], y_obs);
		y1 = DY_FROM_DLAT (y[0], y_obs);
	}
	else {
		x1 = x[0] - x_obs;
		y1 = y[0] - y_obs;
	}
	x1 *= 1000;	y1 *= 1000;	rho *= 1000;
	r1 = hypot (x1, y1);
	if (r1 != 0.0) {
		ir1 = 1.0 / r1;
		xr1 = x1 * ir1;
		yr1 = y1 * ir1;
	}
	for (k = 1; k < n; k++) {	/* Loop over vertex */
				
		if (flat) {
			x2 = DX_FROM_DLON (x[k], x_obs, y[k], y_obs);
			y2 = DY_FROM_DLAT (y[k], y_obs);
		}
		else {
			x2 = x[k] - x_obs;
			y2 = y[k] - y_obs;
		}
		x2 *= 1000;	y2 *= 1000;
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
				sign2 = copysign (1.0, p);
				p = fabs (p);
	                        fi_i    = d_acos (sign2 * (dx*xr1 + dy*yr1) * iside);
	                        theta_i = d_acos (sign2 * (dx*xr2 + dy*yr2) * iside);
				part1 = integral (fi_i, theta_i, z_obs / p);
				part2 = p * part1;
				if (dump) fprintf (stderr, "I(%g, %g, %g) = %g %g\n", R2D*(fi_i), R2D*(theta_i), z_obs / p, p, part1);
			}
		}
					
		if (!zerog) vsum += part2;
					
		/* move this vertex to last vertex */
					
		x1 = x2;
		y1 = y2;
		r1 = r2;
		xr1 = xr2;
		yr1 = yr2;
	}
				
	/* If z axis is positive down, then vsum should have the same sign as z */
				 
	vsum = (z_obs > 0.0) ? fabs (vsum) : -fabs (vsum);

	return (1.0e-11 * GAMMA * rho * vsum / G0);
}

double get_one_output3D (double x_obs, double y_obs, double z_obs, struct CAKE *cake, double depths[], unsigned int ndepths, unsigned int mode, bool flat_earth)
{	/* Evaluate output at a single observation point (x,y,z) */
	/* Work array vtry must have at least of length ndepths */
	unsigned int k;
	double z;
	struct SLICE *sl = NULL;
	double vtry[GMT_TALWANI3D_N_DEPTHS];	/* Allocate on stack since trouble with OpenMP otherwise */
	for (k = 0; k < ndepths; k++) {
		vtry[k] = 0.0;
		z = cake[k].depth - z_obs;	/* Vertical distance from observation point to this slice */
		for (sl = cake[k].first_slice; sl; sl = sl->next) {	/* Loop over slices */
			if (mode == TALWANI3D_FAA) /* FAA */
				vtry[k] += get_grav3d  (sl->x, sl->y, sl->n, x_obs, y_obs, z, sl->rho, flat_earth);
			else if (mode == TALWANI3D_GEOID) /* GEOID */
				vtry[k] += get_geoid3d (sl->x, sl->y, sl->n, x_obs, y_obs, z, sl->rho, flat_earth);
			else /* VGG */
				vtry[k] += get_vgg3d   (sl->x, sl->y, sl->n, x_obs, y_obs, z, sl->rho, flat_earth);
		}
	}
	return (parint (depths, vtry, ndepths));	/* Use parabolic integrator and return value */
}

int comp_cakes (const void *cake_a, const void *cake_b)
{	/* Used in the sorting of layers on depths */
	const struct CAKE *a = cake_a, *b = cake_b;
	if (a->depth < b->depth) return (-1);
	if (a->depth > b->depth) return (1);
	return (0);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_talwani3d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_talwani3d (void *V_API, int mode, void *args)
{
	int row, col, error = 0;
	unsigned int k, tbl, seg, ndepths = 0, n, dup_node, n_duplicate = 0;
	uint64_t node;
	size_t n_alloc, n_alloc1;
	
	bool flat_earth = false, first_slice = true;
	
	char *uname[2] = {"meter", "km"}, *kind[3] = {"FAA", "VGG", "GEOID"};
	double z_level, depth, rho;
	double *x = NULL, *y = NULL, *in = NULL, *depths = NULL;
					
	struct SLICE *sl = NULL, *slnext = NULL;
	struct CAKE *cake = NULL;
	struct TALWANI3D_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_talwani3d_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE)
		bailout (GMT_talwani3d_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS)
		bailout (GMT_talwani3d_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_talwani3d_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_talwani3d_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the talwani3d main code ----------------------------*/
	
	GMT_enable_threads (GMT);	/* Set number of active threads, if supported */
	/* Specify input expected columns to be at least 2 */
	if ((error = GMT_set_cols (GMT, GMT_IN, 2)) != GMT_OK) {
		Return (error);
	}
	/* Register likely model files unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POLYGON, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	/* Initialize the i/o for doing record-by-record reading */
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	/* Set up cake slice array and pointers */
	
	n_alloc1 = GMT_CHUNK;
	cake = GMT_memory (GMT, NULL, n_alloc1, struct CAKE);
	
	/* Read polygon information from multiple segment file */
	GMT_Report (API, GMT_MSG_VERBOSE, "All x/y-values are assumed to be given in %s\n", uname[Ctrl->M.active[TALWANI3D_HOR]]);
	GMT_Report (API, GMT_MSG_VERBOSE, "All z-values are assumed to be given in %s\n", uname[Ctrl->M.active[TALWANI3D_VER]]);
	
	/* Read the sliced model */
	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_TABLE_HEADER (GMT)) 	/* Skip all table headers */
				continue;
			if (GMT_REC_IS_SEGMENT_HEADER (GMT) || GMT_REC_IS_EOF (GMT)) {	/* Process segment headers or end-of-file */
				/* First close previous segment */
				if (!first_slice) {
					if (!(x[n-1] == x[0] && y[n-1] == y[0])) {	/* Copy first point to last */
						if (n_duplicate == 1 && dup_node == n) n_duplicate = 0;	/* So it was the last == first duplicate; reset count */
						x[n] = x[0];
						y[n] = y[0];
						n++;
					}
					x = GMT_memory (GMT, x, n, double);
					y = GMT_memory (GMT, y, n, double);
					k = 0;
					while (k < ndepths && depth != cake[k].depth) k++;

					if (k == ndepths) {	/* New depth */
						if (ndepths == n_alloc1) {
							n_alloc1 += GMT_CHUNK;
							cake = GMT_memory (GMT, cake, n_alloc1, struct CAKE);
						}
						cake[k].depth = depth;
						cake[k].first_slice = GMT_memory (GMT, NULL, 1, struct SLICE);
						cake[k].first_slice->rho = rho;
						cake[k].first_slice->x = x;
						cake[k].first_slice->y = y;
						cake[k].first_slice->n = n;
						ndepths++;
					}
					else {	/* Hook onto list of slices at same depth */
						sl = cake[k].first_slice;
						while (sl->next) sl = sl->next;
						sl->next = GMT_memory (GMT, NULL, 1, struct SLICE);
						sl->next->rho = rho;
						sl->next->x = x;
						sl->next->y = y;
						sl->next->n = n;
					}
				}
				first_slice = false;
				if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
					break;
				/* Process the next segment header */
				sscanf (GMT->current.io.segment_header, "%lf %lf", &depth, &rho);
				if (Ctrl->D.active) rho = Ctrl->D.rho;
				rho *= 0.001;	/* Change density to g/cm3 */
				if (!Ctrl->M.active[TALWANI3D_VER]) depth *= 0.001;	/* Change depth to km */
				if (Ctrl->A.active) depth = -depth;	/* Make positive down */
				/* Allocate array for this slice */
				n_alloc = GMT_CHUNK;
				x = GMT_memory (GMT, NULL, n_alloc, double);
				y = GMT_memory (GMT, NULL, n_alloc, double);
				n = 0;
				continue;
			}
		}
		/* Clean data record to process */

		if (n && (x[n-1] == x[n] && y[n-1] == y[n])) {	/* Maybe a duplicate point - or it could be the repeated last = first */
			n_duplicate++;
			dup_node = n;
		}
		else {	/* New point for sure */
			x[n] = in[GMT_X];	y[n] = in[GMT_Y];
			if (!flat_earth) {
				if (!Ctrl->M.active[TALWANI3D_HOR]) {	/* Change distances to km */
					x[n] *= 0.001;
					y[n] *= 0.001;
				}
			}
			n++;
			if (n == n_alloc) {
				n_alloc += GMT_CHUNK;
				x = GMT_memory (GMT, x, n_alloc, double);
				y = GMT_memory (GMT, y, n_alloc, double);
			}
		}
	} while (true);
	
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}
	
	if (ndepths >= GMT_TALWANI3D_N_DEPTHS) {
		GMT_Report (API, GMT_MSG_NORMAL, "Model cannot have more than %d depth layer\n", GMT_TALWANI3D_N_DEPTHS);
		GMT_Report (API, GMT_MSG_NORMAL, "You must increase GMT_TALWANI3D_N_DEPTHS and recompile\n");
		Return (EXIT_FAILURE);
	}
	/* Finish allocation and sort on layers */
	
	cake = GMT_memory (GMT, cake, ndepths, struct CAKE);
	qsort (cake, ndepths, sizeof (struct CAKE), comp_cakes);

	if (n_duplicate) GMT_Report (API, GMT_MSG_VERBOSE, "Ignored %u duplicate vertices\n", n_duplicate);

	if (Ctrl->Z.mode == 1) {	/* Got grid with observation levels which also sets output locations */
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->Z.file, G) == NULL) {
			Return (API->error);
		}
	}
	else if (GMT->common.R.active) {	/* Gave -R -I [-r] */
		if ((G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, NULL, Ctrl->I.inc, \
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	}
	else {	/* Got a dataset with output locations */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->N.file, NULL)) == NULL) {
			Return (API->error);
		}
	}
	
	flat_earth = GMT_is_geographic (GMT, GMT_IN);
	
	if (Ctrl->A.active) Ctrl->Z.level = -Ctrl->Z.level;
	
	/* Now we can write to the screen the user's polygon model characteristics. */
	
	GMT_Report (API, GMT_MSG_VERBOSE, "# of depths: %d\n", ndepths);
	if (GMT_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) {
	 	for (k = 0; k < ndepths; k++) {
	 		for (sl = cake[k].first_slice; sl; sl = sl->next)
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Depth: %lg Rho: %lg N-vertx: %4d\n",
	 				cake[k].depth, sl->rho, sl->n);
	 	}
	}
	GMT_Report (API, GMT_MSG_VERBOSE, "Start calculating %s\n", kind[Ctrl->F.mode]);

	/* Set up depths array needed by get_one_output3D */
	depths = GMT_memory (GMT, NULL, ndepths, double);
	for (k = 0; k < ndepths; k++) depths[k] = cake[k].depth;	/* Used by the parabolic integrator */
	
	if (Ctrl->N.active) {	/* Single loop over specified output locations */
		double scl = (!(flat_earth || Ctrl->M.active[TALWANI3D_HOR])) ? 0.001 : 1.0;	/* Perhaps convert to km */
		double out[4];
		if ((error = GMT_set_cols (GMT, GMT_OUT, 4)) != GMT_OK) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (D->n_segments > 1) GMT_set_segmentheader (GMT, GMT_OUT, true);	
		z_level = Ctrl->Z.level;	/* Default observation z level unless provided in input file */
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
				S = D->table[tbl]->segment[seg];	/* Current segment */
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, S->header);
				GMT_prep_tmp_arrays (GMT, S->n_rows, 1);	/* Init or reallocate tmp vector */
#ifdef _OPENMP
				/* Spread calculation over selected cores */
#pragma omp parallel for private(row,z_level) shared(GMT,Ctrl,S,scl,cake,depths,ndepths,flat_earth)
#endif
				/* Separate the calculation from the output in two separate row-loops since cannot do rec-by-rec output
				 * with OpenMP active due to race condiations would mess up the output order */
				for (row = 0; row < S->n_rows; row++) {	/* Calculate attraction at all output locations for this segment */
					if (S->n_columns == 3 && !Ctrl->Z.active) z_level = S->coord[GMT_Z][row];
					GMT->hidden.mem_coord[GMT_X][row] = get_one_output3D (S->coord[GMT_X][row] * scl, S->coord[GMT_Y][row] * scl, z_level, cake, depths, ndepths, Ctrl->F.mode, flat_earth);
				}
				/* This loop is not under OpenMP */
				out[GMT_Z] = Ctrl->Z.level;	/* Default observation z level unless provided in input file */
				for (row = 0; row < S->n_rows; row++) {	/* Loop over output locations */
					out[GMT_X] = S->coord[GMT_X][row];
					out[GMT_Y] = S->coord[GMT_Y][row];
					if (S->n_columns == 3 && !Ctrl->Z.active) out[GMT_Z] = S->coord[GMT_Z][row];
					out[3] = GMT->hidden.mem_coord[GMT_X][row];
					GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
				}
			}
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}
	}
	else {	/* Dealing with a grid */
		int    nx = (int)G->header->nx, ny = (int)G->header->ny;	/* To shut up compiler warnings */
		double y_obs, *x_obs = GMT_memory (GMT, NULL, G->header->nx, double);
		for (col = 0; col < nx; col++) {
			x_obs[col] = GMT_grd_col_to_x (GMT, col, G->header);
			if (!(flat_earth || Ctrl->M.active[TALWANI3D_HOR])) x_obs[col] *= 0.001;	/* Convert to km */
		}
#ifdef _OPENMP
				/* Spread calculation over selected cores */
#pragma omp parallel for private(row,col,node,y_obs,z_level) shared(API,GMT,Ctrl,G,x_obs,cake,depths,ndepths,flat_earth)
#endif
		for (row = 0; row < ny; row++) {	/* Do row-by-row and report on progress if -V */
			y_obs = GMT_grd_row_to_y (GMT, row, G->header);
			if (!(flat_earth || Ctrl->M.active[TALWANI3D_HOR])) y_obs *= 0.001;	/* Convert to km */
			//GMT_Report (API, GMT_MSG_VERBOSE, "Finished row %5d\n", row);
			for (col = 0; col < G->header->nx; col++) {
				/* Loop over cols; always save the next level before we update the array at that col */
				node = GMT_IJP (G->header, row, col);
				z_level = (Ctrl->A.active) ? -G->data[node] : G->data[node];	/* Get observation z level and possibly flip direction */
				G->data[node] = (float) get_one_output3D (x_obs[col], y_obs, z_level, cake, depths, ndepths, Ctrl->F.mode, flat_earth);
			}
		}
		GMT_free (GMT, x_obs);
		//sprintf (h.remark, "Calculated 3-D gravity from polygon file %s", pfile);
		GMT_Report (API, GMT_MSG_VERBOSE, "Create %s\n", Ctrl->G.file);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, G) != GMT_OK) {
			Return (API->error);
		}
	}
		
	/* Clean up memory */
	
	for (k = 0; k < ndepths; k++) {
		sl = cake[k].first_slice;
		slnext = cake[k].first_slice->next;
		while (slnext) {
			GMT_free (GMT, sl->x);
			GMT_free (GMT, sl->y);
			GMT_free (GMT, sl);
			sl = slnext;
			slnext = sl->next;
		}
		GMT_free (GMT, sl->x);
		GMT_free (GMT, sl->y);
		GMT_free (GMT, sl);
	}
	GMT_free (GMT, cake);
	GMT_free (GMT, depths);

	Return (EXIT_SUCCESS);
}
