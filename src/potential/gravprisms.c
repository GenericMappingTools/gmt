/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Date:        12-MAR-2022
 *
 *
 * Calculates gravity due to any number of individual vertical prisms
 * that may have their own unique dimensions (or all are constant) and
 * individual density contrasts (or all the same).
 * Based on methods by
 *
 * Accelerated with OpenMP; see -x.
 */

#include "gmt_dev.h"
#include "talwani.h"

#define THIS_MODULE_CLASSIC_NAME	"gravprisms"
#define THIS_MODULE_MODERN_NAME	"gravprisms"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Compute gravity anomalies over 3-D vertical prisms"
#define THIS_MODULE_KEYS	"<D{,ND(,ZG(,G?},GDN"
#define THIS_MODULE_NEEDS	"r"
#define THIS_MODULE_OPTIONS "-VRbdefhior" GMT_ADD_x_OPT

#define DX_FROM_DLON(x1, x2, y1, y2) (((x1) - (x2)) * DEG_TO_KM * cos (0.5 * ((y1) + (y2)) * D2R))
#define DY_FROM_DLAT(y1, y2) (((y1) - (y2)) * DEG_TO_KM)
#define gmt_M_is_zero2(x) (fabs (x) < 2e-5)	/* Check for near-zero angles [used in geoid integral()]*/

#define GMT_GRAVPRISMS_N_DEPTHS GMT_BUFSIZ	/* Max depths allowed due to OpenMP needing stack array */

#if 0
/* this is for various debug purposes and will eventually be purged */
bool dump = false;
FILE *fp = NULL;
#endif

enum gravprisms_fields {
	GRAVPRISMS_FAA	= 0,
	GRAVPRISMS_VGG,
	GRAVPRISMS_GEOID,
	GRAVPRISMS_HOR=0,
	GRAVPRISMS_VER=1
};


struct GRAVPRISMS_CTRL {
	struct GRAVPRISMS_A {	/* -A Set positive up  */
		bool active;
	} A;
	struct GRAVPRISMS_D {	/* -D<rho> fixed density to override individual prisms */
		bool active;
		double rho;
	} D;
	struct GRAVPRISMS_E {	/* -E<dx><dy>/<dz> fixed prism dimensions [read from file] */
		bool active;
		double dx, dy, dz;
	} E;
	struct GRAVPRISMS_F {	/* -F[f|n[<lat>]|v] */
		bool active, lset;
		unsigned int mode;
		double lat;
	} F;
	struct GRAVPRISMS_G {	/* Output file */
		bool active;
		char *file;
	} G;
	struct GRAVPRISMS_I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct GRAVPRISMS_M {	/* -Mh|z  */
		bool active[2];	/* True if km, else m */
	} M;
	struct GRAVPRISMS_N {	/* Desired output points */
		bool active;
		char *file;
	} N;
	struct GRAVPRISMS_Z {	/* Observation level file or constant */
		bool active;
		double level;
		unsigned int mode;
		char *file;
	} Z;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRAVPRISMS_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GRAVPRISMS_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->F.lat = 45.0;	/* So we compute normal gravity at 45 */

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRAVPRISMS_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->N.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->Z.file);
	gmt_M_free (GMT, C);
}

static int parse (struct GMT_CTRL *GMT, struct GRAVPRISMS_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int k, n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {		/* Process all the options given */
		switch (opt->option) {

			case '<':	/* Input file(s) */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			case 'A':	/* Specify z-axis is positive up [Default is down] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				break;
			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				Ctrl->D.rho = atof (opt->arg);
				break;
			case 'E':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				Ctrl->E.active = true;
				sscanf (opt->arg, "%lg/%lg/%lg", &Ctrl->E.dx, &Ctrl->E.dy, &Ctrl->E.dz);
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'v': Ctrl->F.mode = GRAVPRISMS_VGG; 	break;
					case 'n': Ctrl->F.mode = GRAVPRISMS_GEOID;
						if (opt->arg[1]) Ctrl->F.lat = atof (&opt->arg[1]), Ctrl->F.lset = true;
						break;
					case 'g':  Ctrl->F.mode = GRAVPRISMS_FAA; 	break;
					default:
						GMT_Report (API, GMT_MSG_WARNING, "Option -F: Unrecognized field %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'G':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'M':	/* Length units */
				k = 0;
				while (opt->arg[k]) {
					switch (opt->arg[k]) {
						case 'h':
							n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active[GRAVPRISMS_HOR]);
							Ctrl->M.active[GRAVPRISMS_HOR] = true;
							break;
						case 'z':
							n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active[GRAVPRISMS_VER]);
							Ctrl->M.active[GRAVPRISMS_VER] = true;
							break;
						default:
							n_errors++;
							GMT_Report (API, GMT_MSG_WARNING, "Option -M: Unrecognized modifier %c\n", opt->arg[k]);
							break;
					}
					k++;
				}
				break;
			case 'N':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				Ctrl->N.file = strdup (opt->arg);
				break;
			case 'Z':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
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
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}
	if (GMT->common.R.active[RSET]) {
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[ISET],
		                                 "Option -R: Must specify both -R and -I (and optionally -r)\n");
	}
	n_errors += gmt_M_check_condition (GMT, (GMT->common.R.active[RSET] && GMT->common.R.active[ISET]) && Ctrl->Z.mode == 1,
	                                 "Option -Z: Cannot also specify -R -I\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->N.active && !Ctrl->G.active,
	                                 "Option -G: Must specify output gridfile name.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && !Ctrl->G.file,
	                                 "Option -G: Must specify output gridfile name.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && !Ctrl->N.file,
	                                 "Option -N: Must specify output gridfile name.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s <prismfile> [-A] [-D<density>] [-Ff|n[<lat>]|v] [-G<outfile>] [%s] "
		"[-M[hz]] [-N<trktable>] [%s] [%s] [-Z<level>] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]%s [%s]\n",
		name, GMT_I_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_bo_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT,
		GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_x_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<prismfile>");
	GMT_Usage (API, -2, "One or more multiple-segment ASCII data files. If no files are given, standard "
		"input is read. Contains (x,y,z) center coordinates of prisms with optional [ dx dy dz] [rho] if not set via -D and .");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A The z-axis is positive upwards [Default is positive down].");
	GMT_Usage (API, 1, "\n-D<density>");
	GMT_Usage (API, -2, "Set fixed density contrast (in kg/m^3) [Default reads it from last numerical column.");
	GMT_Usage (API, 1, "\n-Ff|n[<lat>]|v]");
	GMT_Usage (API, -2, "Specify desired geopotential field component:");
	GMT_Usage (API, 3, "f: Free-air anomalies (mGal) [Default].");
	GMT_Usage (API, 3, "n: Geoid anomalies (meter).  Optionally append latitude for evaluation of normal gravity "
		"[Default latitude is mid-grid for grid output or mid-latitude if -N is used].");
	GMT_Usage (API, 3, "v: Vertical Gravity Gradient anomalies (Eotvos = 0.1 mGal/km).");
	GMT_Usage (API, 1, "\n-G<outfile>");
	GMT_Usage (API, -2, "Set name of output file. Grid file name is requires unless -N is used.");
	GMT_Option (API, "I");
	GMT_Usage (API, 1, "\n-M[hz]");
	GMT_Usage (API, -2, "Change units used, via one or two directives:");
	GMT_Usage (API, 3, "h: All x- and y-distances are given in km [meters].");
	GMT_Usage (API, 3, "z: All z-distances are given in km [meters].");
	GMT_Usage (API, 1, "\n-N<trktable>");
	GMT_Usage (API, -2, "File with output locations where a calculation is requested.  No grid "
		"is produced and output (x,y,z,g) is written to standard output (see -bo for binary output).");
	GMT_Option (API, "R,V");
	GMT_Usage (API, 1, "\n-Z<level>");
	GMT_Usage (API, -2, "Set observation level for output locations [0]. "
		"Append either a constant or the name of grid file with variable levels. "
		"If given a grid then it also defines the output grid.");
	GMT_Usage (API, -2, "Note: Cannot use both -Z<grid> and -R -I [-r].");
	GMT_Option (API, "bo,d,e");
	GMT_Usage (API, 1, "\n-fg Map units (lon, lat in degree, else in m [but see -Mh]).");
	GMT_Option (API, "h,i,o,r,x,.");
	return (GMT_MODULE_USAGE);
}

#define GRAVITATIONAL_CONST 6.674e-11
#define GRAVITATIONAL_CONST_MGAL 6.674e-6

GMT_LOCAL double geoidprism (double dx1, double dx2, double dy1, double dy2, double dz1, double dz2, double rho) {
	/* Geoid anomaly from a single prism [Nagy et al, 2000] */
	double n, dx1_sq, dx2_sq, dy1_sq, dy2_sq, dz1_sq, dz2_sq;
	double R111, R112, R121, R122, R211, R212, R221, R222;
	double n111, n112, n121, n122, n211, n212, n221, n222;

	/* Square distances */
	dx1_sq = dx1 * dx1;	dx2_sq = dx2 * dx2;
	dy1_sq = dy1 * dy1;	dy2_sq = dy2 * dy2;
	dz1_sq = dz1 * dz1;	dz2_sq = dz2 * dz2;
	/* Get radii */
	R111 = sqrt (dx1_sq + dy1_sq + dz1_sq); 
	R112 = sqrt (dx2_sq + dy1_sq + dz1_sq);
	R121 = sqrt (dx1_sq + dy2_sq + dz1_sq);
	R122 = sqrt (dx2_sq + dy2_sq + dz1_sq);
	R211 = sqrt (dx1_sq + dy1_sq + dz2_sq); 
	R212 = sqrt (dx2_sq + dy1_sq + dz2_sq);
	R221 = sqrt (dx1_sq + dy2_sq + dz2_sq);
	R222 = sqrt (dx2_sq + dy2_sq + dz2_sq);
	/* Evaluate at dz1 */
	n111 = -(0.5 * (dx1_sq * atan ((dy1 * dz1) / (dz1 * R111)) + dy1_sq * atan ((dx1 * dz1) / (dz1 * R111)) + dz1_sq * atan ((dx1 * dy1) / (dz1 * R111))) - dx1 * dz1 * log(R111 + dy1) - dy1 * dz1 * log (R111 + dx1) - dx1 * dy1 * log (R111 + dz1));
	n112 = +(0.5 * (dx2_sq * atan ((dy1 * dz1) / (dz1 * R112)) + dy1_sq * atan ((dx2 * dz1) / (dz1 * R112)) + dz1_sq * atan ((dx2 * dy1) / (dz1 * R112))) - dx2 * dz1 * log(R112 + dy1) - dy1 * dz1 * log (R112 + dx2) - dx2 * dy1 * log (R112 + dz1));
	n121 = +(0.5 * (dx1_sq * atan ((dy2 * dz1) / (dz1 * R121)) + dy2_sq * atan ((dx1 * dz1) / (dz1 * R121)) + dz1_sq * atan ((dx1 * dy2) / (dz1 * R121))) - dx1 * dz1 * log(R121 + dy2) - dy2 * dz1 * log (R121 + dx1) - dx1 * dy2 * log (R121 + dz1));
	n122 = -(0.5 * (dx2_sq * atan ((dy2 * dz1) / (dz1 * R122)) + dy2_sq * atan ((dx2 * dz1) / (dz1 * R122)) + dz1_sq * atan ((dx2 * dy2) / (dz1 * R122))) - dx2 * dz1 * log(R122 + dy2) - dy2 * dz1 * log (R122 + dx2) - dx2 * dy2 * log (R122 + dz1));
	/* Evaluate at dz2 */
	n211 = +(0.5 * (dx1_sq * atan ((dy1 * dz2) / (dz2 * R211)) + dy1_sq * atan ((dx1 * dz2) / (dz2 * R211)) + dz2_sq * atan ((dx1 * dy1) / (dz2 * R211))) - dx1 * dz2 * log (R211 + dy1) - dy1 * dz2 * log (R211 + dx1) - dx1 * dy1 * log (R111 + dz2));
	n212 = -(0.5 * (dx2_sq * atan ((dy1 * dz2) / (dz2 * R212)) + dy1_sq * atan ((dx2 * dz2) / (dz2 * R212)) + dz2_sq * atan ((dx2 * dy1) / (dz2 * R212))) - dx2 * dz2 * log (R212 + dy1) - dy1 * dz2 * log (R212 + dx2) - dx2 * dy1 * log (R112 + dz2));
	n221 = -(0.5 * (dx1_sq * atan ((dy2 * dz2) / (dz2 * R221)) + dy2_sq * atan ((dx1 * dz2) / (dz2 * R221)) + dz2_sq * atan ((dx1 * dy2) / (dz2 * R221))) - dx1 * dz2 * log (R221 + dy2) - dy2 * dz2 * log (R221 + dx1) - dx1 * dy2 * log (R121 + dz2));
	n222 = +(0.5 * (dx2_sq * atan ((dy2 * dz2) / (dz2 * R222)) + dy2_sq * atan ((dx2 * dz2) / (dz2 * R222)) + dz2_sq * atan ((dx2 * dy2) / (dz2 * R222))) - dx2 * dz2 * log (R222 + dy2) - dy2 * dz2 * log (R222 + dx2) - dx2 * dy2 * log (R122 + dz2));

	n = rho * GRAVITATIONAL_CONST_MGAL * (n111 + n112 + n121 + n122 + n211 + n212 + n221 + n222);

	return (n);
}

GMT_LOCAL double gravprism (double dx1, double dx2, double dy1, double dy2, double dz1, double dz2, double rho) {
	/* Gravity anomaly from a single prism */
	double g, dx1_sq, dx2_sq, dy1_sq, dy2_sq, dz1_sq, dz2_sq;
	double R111, R112, R121, R122, R211, R212, R221, R222;
	double g111, g112, g121, g122, g211, g212, g221, g222;

	/* Square distances */
	dx1_sq = dx1 * dx1;	dx2_sq = dx2 * dx2;
	dy1_sq = dy1 * dy1;	dy2_sq = dy2 * dy2;
	dz1_sq = dz1 * dz1;	dz2_sq = dz2 * dz2;
	/* Get radii */
	R111 = sqrt (dx1_sq + dy1_sq + dz1_sq); 
	R112 = sqrt (dx2_sq + dy1_sq + dz1_sq);
	R121 = sqrt (dx1_sq + dy2_sq + dz1_sq);
	R122 = sqrt (dx2_sq + dy2_sq + dz1_sq);
	R211 = sqrt (dx1_sq + dy1_sq + dz2_sq); 
	R212 = sqrt (dx2_sq + dy1_sq + dz2_sq);
	R221 = sqrt (dx1_sq + dy2_sq + dz2_sq);
	R222 = sqrt (dx2_sq + dy2_sq + dz2_sq);
	/* Evaluate at dz1 */
	g111 = -(dz1 * atan ((dx1 * dy1) / (dz1 * R111)) - dx1 * log(R111 + dy1) - dy1 * log (R111 + dx1));
	g112 = +(dz1 * atan ((dx2 * dy1) / (dz1 * R112)) - dx2 * log(R112 + dy1) - dy1 * log (R112 + dx2));
	g121 = +(dz1 * atan ((dx1 * dy2) / (dz1 * R121)) - dx1 * log(R121 + dy2) - dy2 * log (R121 + dx1));
	g122 = -(dz1 * atan ((dx2 * dy2) / (dz1 * R122)) - dx2 * log(R122 + dy2) - dy2 * log (R122 + dx2));
	/* Evaluate at dz2 */
	g211 = +(dz2 * atan ((dx1 * dy1) / (dz2 * R211)) - dx1 * log (R211 + dy1) - dy1 * log (R211 + dx1));
	g212 = -(dz2 * atan ((dx2 * dy1) / (dz2 * R212)) - dx2 * log (R212 + dy1) - dy1 * log (R212 + dx2));
	g221 = -(dz2 * atan ((dx1 * dy2) / (dz2 * R221)) - dx1 * log (R221 + dy2) - dy2 * log (R221 + dx1));
	g222 = +(dz2 * atan ((dx2 * dy2) / (dz2 * R222)) - dx2 * log (R222 + dy2) - dy2 * log (R222 + dx2));

	g = rho * GRAVITATIONAL_CONST_MGAL * (g111 + g112 + g121 + g122 + g211 + g212 + g221 + g222);

	return (g);
}

GMT_LOCAL double vggprism (double dx1, double dx2, double dy1, double dy2, double dz1, double dz2, double rho) {
	/* Vertical gravity gradient from a single prism [Kim & Wessel, 2016] */
	double v, dx1_sq, dx2_sq, dy1_sq, dy2_sq, dz1_sq, dz2_sq;
	double R111, R112, R121, R122, R211, R212, R221, R222;
	double v111, v112, v121, v122, v211, v212, v221, v222;

	/* Square distances */
	dx1_sq = dx1 * dx1;	dx2_sq = dx2 * dx2;
	dy1_sq = dy1 * dy1;	dy2_sq = dy2 * dy2;
	dz1_sq = dz1 * dz1;	dz2_sq = dz2 * dz2;
	/* Get radii */
	R111 = sqrt (dx1_sq + dy1_sq + dz1_sq); 
	R112 = sqrt (dx2_sq + dy1_sq + dz1_sq);
	R121 = sqrt (dx1_sq + dy2_sq + dz1_sq);
	R122 = sqrt (dx2_sq + dy2_sq + dz1_sq);
	R211 = sqrt (dx1_sq + dy1_sq + dz2_sq); 
	R212 = sqrt (dx2_sq + dy1_sq + dz2_sq);
	R221 = sqrt (dx1_sq + dy2_sq + dz2_sq);
	R222 = sqrt (dx2_sq + dy2_sq + dz2_sq);
	/* Evaluate at dz1 */
	v111 = -atan ((dx1 * dy1) / (dz1 * R111));
	v112 = +atan ((dx2 * dy1) / (dz1 * R112));
	v121 = +atan ((dx1 * dy2) / (dz1 * R121));
	v122 = -atan ((dx2 * dy2) / (dz1 * R122));
	/* Evaluate at dz2 */
	v211 = +atan ((dx1 * dy1) / (dz2 * R211));
	v212 = -atan ((dx2 * dy1) / (dz2 * R212));
	v221 = -atan ((dx1 * dy2) / (dz2 * R221));
	v222 = +atan ((dx2 * dy2) / (dz2 * R222));

	v = rho * GRAVITATIONAL_CONST_MGAL * (v111 + v112 + v121 + v122 + v211 + v212 + v221 + v222);

	return (v);
}

GMT_LOCAL double gravprisms_get_one_g_output (double x, double y, double z, uint64_t n_prisms, double **P, double unused) {
	/* (x, y, z) is the observation point */
	double g = 0.0;
	gmt_M_unused (unused);
	for (uint64_t k = 0; k < n_prisms; k++)
		g += gravprism (P[0][k]-x, P[1][k]-x, P[2][k]-y, P[3][k]-y, P[4][k]-z, P[5][k]-z, P[6][k]);
	return (g);
}

GMT_LOCAL double gravprisms_get_one_n_output (double x, double y, double z, uint64_t n_prisms, double **P, double constant) {
	/* (x, y, z) is the observation point */
	double n = 0.0;
	for (uint64_t k = 0; k < n_prisms; k++)
		n += geoidprism (P[0][k]-x, P[1][k]-x, P[2][k]-y, P[3][k]-y, P[4][k]-z, P[5][k]-z, P[6][k]);
	return (n * constant * 0.01);	/* To get geoid in meter */
}

GMT_LOCAL double gravprisms_get_one_v_output (double x, double y, double z, uint64_t n_prisms, double **P, double unused) {
	/* (x, y, z) is the observation point */
	double v = 0.0;
	gmt_M_unused (unused);
	for (uint64_t k = 0; k < n_prisms; k++)
		v += vggprism (P[0][k]-x, P[1][k]-x, P[2][k]-y, P[3][k]-y, P[4][k]-z, P[5][k]-z, P[6][k]);
	return (v * 0.0001);	/* From mGal/m to Eotvos = 0.1 mGal/km */
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gravprisms (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int n_expected = 7;
	uint64_t tbl, seg, row, col, k, node, n_prisms;

	bool flat_earth = false;

	char *uname[2] = {"meter", "km"}, *kind[3] = {"FAA", "VGG", "GEOID"}, remark[GMT_LEN64] = {""};
	double z_level, rho, dx, dy, dz, lat, G0;
	double *prism[7];
	double (*eval) (double, double, double, uint64_t, double **, double);

	struct GRAVPRISMS_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_DATASET *D = NULL, *P = NULL;
	struct GMT_DATASEGMENT *S = NULL;
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

	/*---------------------------- This is the gravprisms main code ----------------------------*/

	gmt_enable_threads (GMT);	/* Set number of active threads, if supported */
	/* Specify input expected columns to be at least 3 */
	if (Ctrl->D.active) n_expected--;		/* Not reading density */
	if (Ctrl->E.active) n_expected -= 3;	/* Not reading dx dy dz */
	if ((error = GMT_Set_Columns (API, GMT_IN, n_expected, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	/* Register likely model files unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if ((P = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}

	for (k = 0; k < 7; k++)
		prism[k] = gmt_M_memory (GMT, NULL, P->n_records, double);

	/* Fill in prism arrays from data set, including optional choices for fixed or variable dimension and density, then free */
	dx = Ctrl->E.dx;	dy = Ctrl->E.dy;	dz = Ctrl->E.dz;	rho = Ctrl->D.rho;
	col = (Ctrl->E.active) ? 3 : 6;	/* Input column for density, if present */
	for (tbl = k = 0; tbl < P->n_tables; tbl++) {
		for (seg = 0; seg < P->table[tbl]->n_segments; seg++) {
			S = P->table[tbl]->segment[seg];
			for (row = 0; row < S->n_rows; row++, k++) {
				if (!Ctrl->E.active) {
					dx = S->data[3][row];
					dy = S->data[4][row];
					dz = S->data[5][row];
				}
				if (!Ctrl->D.active) rho = S->data[col][row];
				prism[0][k] = S->data[GMT_X][row] - dx;	/* Set the x-coordinates of the x-edges of prism */
				prism[1][k] = S->data[GMT_X][row] + dx;
				prism[2][k] = S->data[GMT_Y][row] - dy;	/* Set the y-coordinates of the y-edges of prism */
				prism[3][k] = S->data[GMT_Y][row] + dy;
				prism[4][k] = S->data[GMT_Z][row] - dz;	/* Set the z-coordinates of the z-edges of prism */
				prism[5][k] = S->data[GMT_Z][row] + dz;
				prism[6][k] = rho;
			}
		}
	}
	n_prisms = k;
	if (GMT_Destroy_Data (API, &P) != GMT_NOERROR) {
		Return (GMT_MEMORY_ERROR);
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
		gmt_disable_bghio_opts (GMT);	/* Do not want any -b -g -h -i -o to affect the reading from the -N file */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->N.file, NULL)) == NULL)
			Return (API->error);
		if (D->n_columns < 2) {
			GMT_Report (API, GMT_MSG_ERROR, "Input file %s has %d column(s) but at least 2 are needed\n", Ctrl->N.file, (int)D->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_bghio_opts (GMT);	/* Recover settings provided by user (if -b -g -h -i were used at all) */
		if (gmt_M_is_geographic (GMT, GMT_IN)) lat = 0.5 * (D->min[GMT_Y] + D->max[GMT_Y]);
	}

	flat_earth = gmt_M_is_geographic (GMT, GMT_IN);		/* If true then input is in degrees and we must convert to km later on */

	if (flat_earth && Ctrl->M.active[GRAVPRISMS_HOR]) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -M: Cannot specify both geographic coordinates (degrees) AND -Mh\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->A.active) Ctrl->Z.level = -Ctrl->Z.level;

	/* Read polygon information from multiple segment file */
	GMT_Report (API, GMT_MSG_INFORMATION, "All x/y-values are assumed to be given in %s\n", uname[Ctrl->M.active[GRAVPRISMS_HOR]]);
	GMT_Report (API, GMT_MSG_INFORMATION, "All z-values are assumed to be given in %s\n",   uname[Ctrl->M.active[GRAVPRISMS_VER]]);

	/* Now we can write (if -V) to the screen the user's polygon model characteristics. */

	GMT_Report (API, GMT_MSG_INFORMATION, "# of prisms: %" PRIu64 "\n", n_prisms);
	GMT_Report (API, GMT_MSG_INFORMATION, "Start calculating %s\n", kind[Ctrl->F.mode]);

	switch (Ctrl->F.mode) {		/* Set pointer to chosen evaluation function */
		case GRAVPRISMS_VGG:
			eval = &gravprisms_get_one_v_output;
		 	break;
		case GRAVPRISMS_GEOID:
			eval = &gravprisms_get_one_n_output;
			G0 = (Ctrl->F.lset) ? g_normal (Ctrl->F.lat) : g_normal (lat);
			G0 = 1.0 / G0;	/* Since we will be dividing */
			break;
		case GRAVPRISMS_FAA:
			eval = &gravprisms_get_one_g_output;
		 	break;
		default:
			/* Just for Coverity */
			break;
	}

	if (Ctrl->N.active) {	/* Single loop over specified output locations */
		unsigned int wmode = GMT_ADD_DEFAULT;
		double scl = (!(flat_earth || Ctrl->M.active[GRAVPRISMS_HOR])) ? (1.0 / METERS_IN_A_KM) : 1.0;	/* Perhaps convert to km */
		double out[4];
		struct GMT_RECORD *Rec = gmt_new_record (GMT, out, NULL);
		/* Must register Ctrl->G.file first since we are going to writing rec-by-rec */
		if (Ctrl->G.active) {
			int out_ID;
			if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, NULL, Ctrl->G.file)) == GMT_NOTSET) {
				Return (API->error);
			}
			wmode = GMT_ADD_EXISTING;
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, 4, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, wmode, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
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
#pragma omp parallel for private(row,z_level) shared(GMT,Ctrl,S,eval,scl,n_prisms,prism,flat_earth, G0)
#endif
				/* Separate the calculation from the output in two separate row-loops since cannot do rec-by-rec output
				 * with OpenMP due to race condiations that would mess up the output order */
				for (row = 0; row < (int64_t)S->n_rows; row++) {	/* Calculate attraction at all output locations for this segment */
					z_level = (S->n_columns == 3 && !Ctrl->Z.active) ? S->data[GMT_Z][row] : Ctrl->Z.level;	/* Default observation z level unless provided in input file */
					GMT->hidden.mem_coord[GMT_X][row] = eval (S->data[GMT_X][row] * scl, S->data[GMT_Y][row] * scl, z_level, n_prisms, prism, G0);
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
			Return (API->error);
		}
	}
	else {	/* Dealing with a grid */
		openmp_int row, col, n_columns = (openmp_int)G->header->n_columns, n_rows = (openmp_int)G->header->n_rows;	/* To shut up compiler warnings */
		double y_obs, *x_obs = gmt_M_memory (GMT, NULL, G->header->n_columns, double);
		for (col = 0; col < n_columns; col++) {
			x_obs[col] = gmt_M_grd_col_to_x (GMT, col, G->header);
			if (!(flat_earth || Ctrl->M.active[GRAVPRISMS_HOR])) x_obs[col] /= METERS_IN_A_KM;	/* Convert to km */
		}
#ifdef _OPENMP
		/* Spread calculation over selected cores */
#pragma omp parallel for private(row,col,node,y_obs,z_level) shared(API,GMT,Ctrl,G,eval,x_obs,n_prisms,prism,flat_earth, G0)
#endif
		for (row = 0; row < n_rows; row++) {	/* Do row-by-row and report on progress if -V */
			y_obs = gmt_M_grd_row_to_y (GMT, row, G->header);
			if (!(flat_earth || Ctrl->M.active[GRAVPRISMS_HOR])) y_obs /= METERS_IN_A_KM;	/* Convert to km */
#ifndef _OPENMP
			GMT_Report (API, GMT_MSG_INFORMATION, "Finished row %5d\n", row);
#endif
			for (col = 0; col < (openmp_int)G->header->n_columns; col++) {
				/* Loop over cols; always save the next level before we update the array at that col */
				node = gmt_M_ijp (G->header, row, col);
				z_level = (Ctrl->A.active) ? -G->data[node] : G->data[node];	/* Get observation z level and possibly flip direction */
				G->data[node] = (gmt_grdfloat) eval (x_obs[col], y_obs, z_level, n_prisms, prism, G0);
			}
		}
		gmt_M_free (GMT, x_obs);
		GMT_Report (API, GMT_MSG_INFORMATION, "Create %s\n", Ctrl->G.file);
		sprintf (remark, "Calculated 3-D %s", kind[Ctrl->F.mode]);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, remark, G)) {
			Return (API->error);
		}
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) {
			Return (API->error);
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, G) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	for (k = 0; k < 7; k++)
		gmt_M_free (GMT, prism[k]);

	Return (GMT_NOERROR);
}
