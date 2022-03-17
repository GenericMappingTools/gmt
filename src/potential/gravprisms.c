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
 * Authord:     Paul Wessel
 * Date:        12-MAR-2022
 *
 *
 * Calculates geopotential anomalies due to any number of individual
 * vertical prisms that may have their own unique dimensions (or all
 * are constant) and individual density contrasts (or all the same).
 * Instead of reading in prisms we can create prisms based on grid
 * limits (e.g., top and base surfaces) and then use these prisms to
 * compute the geopotential anomalies.
 * For vertically varying density we break the prisms into a stack of
 * short sub-prisms with constant density that approximates the actual
 * continuously-varying densities.
 *
 * Accelerated with OpenMP; see -x.
 */

#include "gmt_dev.h"
#include "talwani.h"

#define THIS_MODULE_CLASSIC_NAME	"gravprisms"
#define THIS_MODULE_MODERN_NAME	"gravprisms"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Compute geopotential anomalies over 3-D vertical prisms"
#define THIS_MODULE_KEYS	"<D{,CD)=w,KG),LG(,ND(,SG(,TG(,WG(,ZG(,G?},GDN"
#define THIS_MODULE_NEEDS	"r"
#define THIS_MODULE_OPTIONS "-VRbdefhior" GMT_ADD_x_OPT

/* Note: All calculations assume distances in meters so degrees -> meters and km -> meters first */

#define DX_FROM_DLON(x1, x2, y1, y2) (((x1) - (x2)) * DEG_TO_M * cos (0.5 * ((y1) + (y2)) * D2R))
#define DY_FROM_DLAT(y1, y2) (((y1) - (y2)) * DEG_TO_M)

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
	struct GRAVPRISMS_C {	/* -C[+q][+w<file>][+z<dz>] creates prisms between surfaces set in -L -S -T */
		bool active;
		bool quit;
		bool dump;
		char *file;
		double dz;
	} C;
	struct GRAVPRISMS_D {	/* -D<rho>|<avedens> fixed density or grid with densities to override individual prisms */
		bool active;
		char *file;
		double rho;
	} D;
	struct GRAVPRISMS_E {	/* -E<dx>[/<dy>] fixed prism x/y dimensions [read from file] */
		bool active;
		double dx, dy;
	} E;
	struct GRAVPRISMS_F {	/* -F[f|n[<lat>]|v] */
		bool active, lset;
		unsigned int mode;
		double lat;
	} F;
	struct GRAVPRISMS_G {	/* Output grid or profile */
		bool active;
		char *file;
	} G;
	struct GRAVPRISMS_H {	/* -H<H>/<rho_l>/<rho_h>[+d<densify>][+p<power>] */
		bool active;
		double H, rho_l, rho_h;
		double p, densify;
		double del_rho;	/* Computed as rho_h - rho_l */
		double p1;	/* Will be p + 1 to avoid addition in loops */
	} H;
	struct GRAVPRISMS_I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct GRAVPRISMS_L {	/* Low (base) surface(x,y) file */
		bool active;
		char *file;
	} L;
	struct GRAVPRISMS_M {	/* -Mh|z  */
		bool active[2];	/* True if km, else m */
	} M;
	struct GRAVPRISMS_N {	/* Desired output points */
		bool active;
		char *file;
	} N;
	struct GRAVPRISMS_S {	/* Full surface(x,y) file */
		bool active;
		char *file;
	} S;
	struct GRAVPRISMS_T {	/* Top surface(x,y) file */
		bool active;
		char *file;
	} T;
	struct GRAVPRISMS_W {	/* Variable rho(x,y) output file */
		bool active;
		char *file;
	} W;
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
	C->H.p = 1.0;	/* Linear density increase */
	C->H.densify = 0.0;	/* No water-driven compaction on flanks */

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRAVPRISMS_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->N.file);
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->D.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->S.file);
	gmt_M_str_free (C->T.file);
	gmt_M_str_free (C->W.file);
	gmt_M_str_free (C->Z.file);
	gmt_M_free (GMT, C);
}

static int parse (struct GMT_CTRL *GMT, struct GRAVPRISMS_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int k, n_errors = 0;
	int ns;
	char *c = NULL;
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
			case 'C':	/* Create prisms from layer between two surfaces */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				if ((c = gmt_first_modifier (GMT, opt->arg, "qwz"))) {
					unsigned int pos = 0;
					char txt[GMT_LEN256] = {""};
					while (gmt_getmodopt (GMT, 'C', c, "qwz", &pos, txt, &n_errors) && n_errors == 0) {
						switch (txt[0]) {
							case 'q':	/* Quit once prism file has been written */
								Ctrl->C.quit = true;
								break;
							case 'w':	/* Write created prisms to given file */
								Ctrl->C.dump = true;
								if (txt[1]) Ctrl->C.file = strdup (&txt[1]);
								break;
							case 'z':	/* Set vertical increment*/
								Ctrl->C.dz = atof (&txt[1]);
								break;
							default:
								n_errors++;
								break;
						}
					}
				}
				break;
			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				if (!gmt_access (GMT, opt->arg, F_OK)) {	/* Gave grid with densities */
					Ctrl->D.file = strdup (opt->arg);
					if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->D.file))) n_errors++;
				}
				else
					Ctrl->D.rho = atof (opt->arg);
				break;
			case 'E':	/* Set fixed prism dx, dy parameters instead of reading from prismfile */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				Ctrl->E.active = true;
				ns = sscanf (opt->arg, "%lg/%lg", &Ctrl->E.dx, &Ctrl->E.dy);
				if (ns == 1) Ctrl->E.dy = Ctrl->E.dx;
				break;
			case 'F':	/* Select geopotential type */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'v': Ctrl->F.mode = GRAVPRISMS_VGG; 	break;
					case 'n': Ctrl->F.mode = GRAVPRISMS_GEOID;
						if (opt->arg[1]) Ctrl->F.lat = atof (&opt->arg[1]), Ctrl->F.lset = true;
						break;
					case 'f':  Ctrl->F.mode = GRAVPRISMS_FAA; 	break;
					default:
						GMT_Report (API, GMT_MSG_WARNING, "Option -F: Unrecognized field %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'G':	/* Output file given */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'H':	/* Reference seamount density parameters for rho(z) */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->H.active);
				Ctrl->H.active = true;
				if ((c = gmt_first_modifier (GMT, opt->arg, "dp"))) {
					unsigned int pos = 0;
					char txt[GMT_LEN256] = {""};
					while (gmt_getmodopt (GMT, 'H', c, "dp", &pos, txt, &n_errors) && n_errors == 0) {
						switch (txt[0]) {
							case 'd':	/* Get densify rate over reference water depth H */
								Ctrl->H.densify = atof (&txt[1]);
								if (Ctrl->H.densify < 10.0) Ctrl->H.densify *= 1000;	/* Gave units of g/cm^3 */
								break;
							case 'p':	/* Get power coefficient */
								Ctrl->H.p = atof (&txt[1]);
								break;
							default:
								n_errors++;
								break;
						}
					}
					c[0] = '\0';	/* Chop off all modifiers so range can be determined */
				}
				if (sscanf (opt->arg, "%lg/%lg/%lg", &Ctrl->H.H, &Ctrl->H.rho_l, &Ctrl->H.rho_h) != 3) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -H: Unable to parse the three values\n");
					n_errors++;
				}
				if (Ctrl->H.rho_l < 10.0) Ctrl->H.rho_l *= 1000;	/* Gave units of g/cm^3 */
				if (Ctrl->H.rho_h < 10.0) Ctrl->H.rho_h *= 1000;	/* Gave units of g/cm^3 */
				Ctrl->H.del_rho = Ctrl->H.rho_h - Ctrl->H.rho_l;
				Ctrl->H.p1 = Ctrl->H.p + 1;
				if (c) c[0] = '+';	/* Restore modifiers */
				break;
			case 'I':	/* Grid increments */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'L':	/* Low (base) file (or constant) given */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.active = true;
				Ctrl->L.file = strdup (opt->arg);
				break;
			case 'M':	/* Horizontal and vertical length units */
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
			case 'N':	/* Got profile for output locations */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				Ctrl->N.file = strdup (opt->arg);
				break;
			case 'S':	/* Full surface grid file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.active = true;
				Ctrl->S.file = strdup (opt->arg);
				break;
			case 'T':	/* top of layer file (or constant) given */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				Ctrl->T.file = strdup (opt->arg);
				break;
			case 'W':	/* Out grid with vertically-averaged density contrasts created via -C -H */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				Ctrl->W.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->W.file))) n_errors++;
				break;
			case 'Z':	/* Observation level(s) */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
				if (opt->arg[0] && !gmt_access (GMT, opt->arg, F_OK)) {	/* File with z-levels exists */
					Ctrl->Z.file = strdup (opt->arg);
					Ctrl->Z.mode = 1;
				}
				else {	/* Got a constant z-level */
					Ctrl->Z.mode = 0;
					Ctrl->Z.level = (opt->arg[0]) ? atof (opt->arg) : 0.0;
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
	                                 "Option -Z: Cannot also specify -R -I if a level grid has been supplied\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && Ctrl->Z.mode == 1,
	                                 "Option -Z: Cannot also specify -N if a level grid has been supplied\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->N.active && !Ctrl->G.active && !Ctrl->C.quit,
	                                 "Option -G: Must specify output gridfile name.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && !Ctrl->G.file,
	                                 "Option -G: Must specify output gridfile name.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && !Ctrl->N.file,
	                                 "Option -N: Must specify output gridfile name.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->H.active,
	                                 "Option -H: Cannot be used with -D.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && !Ctrl->D.active && !Ctrl->H.active,
	                                 "option -C: Need to select either -D or -H.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->H.active && Ctrl->C.dz == 0.0,
	                                 "Option -C: Requires +z<dz> when -H is selected\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->H.active && !Ctrl->S.active,
	                                 "Option -C: Requires -S when -H is set\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && !Ctrl->C.active && !Ctrl->H.active,
	                                 "Option -W: Requires -C and -H\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->L.active && !Ctrl->S.active && !Ctrl->T.active,
	                                 "Option -L: Requires -T (or -S)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.quit && !Ctrl->C.dump,
	                                 "Option -C: Modifier +q requires +w\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.dz > 0.0 && !Ctrl->H.active,
	                                 "Option -C: Modifier +z set without -H is disallowed\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s <prismfile> [-A] [-C[+q][+w<file>][+z<dz>]] [-D<density>] [-E<dx>[/<dy>]] [-Ff|n[<lat>]|v] "
		"[-G<outfile>] [-H<H>/<rho_l>/<rho_h>[+d<densify>][+p<power>]] [%s] [-L<base>] [-M[hz]] [-N<trktable>] [%s] "
		"[-S<shapegrd>] [-T<top>] [%s] [-W<avedens>] [-Z<level>] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]%s [%s]\n",
		name, GMT_I_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_bo_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT,
		GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_x_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<prismfile>");
	GMT_Usage (API, -2, "One or more multiple-segment ASCII data files. If no files are given, standard "
		"input is read. Contains (x,y,z_lo,z_hi), i.e., center coordinates of prisms and z-range, with optional [ dx dy] [rho] if not set via -E and -D.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A The z-axis is positive upwards [Default is positive down].");
	GMT_Usage (API, 1, "\n-C[+q][+w<file>][+z<dz>]");
	GMT_Usage (API, -2, "Create prisms from the <base> (-L) level to <top> (-T) level, or for the full seamount <height> (-S).  No <prismfile> will be read. Modifiers are:");
	GMT_Usage (API, 3, "+q Quit execution once prism file has been written (see +w).  No geopotential calculations are preformed.");
	GMT_Usage (API, 3, "+w Write the created prisms to <file>.");
	GMT_Usage (API, 3, "+z Set increment <dz> for discretization of rho(r,z) when -H is used.");
	GMT_Usage (API, 1, "\n-D<density>");
	GMT_Usage (API, -2, "Set fixed density contrast (in kg/m^3) [Default reads it from last numerical column or computes it via -H]. "
		"Alternatively, read grid with vertically-averaged spatially varying densities from file <density>.\n");
	GMT_Usage (API, 1, "\n-E<dx>/<dy>");
	GMT_Usage (API, -2, "Set fixed x- and y-dimensions for all prisms [Default reads it from columns 4-5].");
	GMT_Usage (API, 1, "\n-Ff|n[<lat>]|v]");
	GMT_Usage (API, -2, "Specify desired geopotential field component:");
	GMT_Usage (API, 3, "f: Free-air anomalies (mGal) [Default].");
	GMT_Usage (API, 3, "n: Geoid anomalies (meter).  Optionally append latitude for evaluation of normal gravity "
		"[Default latitude is mid-grid for grid output or mid-latitude if -N is used].");
	GMT_Usage (API, 3, "v: Vertical Gravity Gradient anomalies (Eotvos = 0.1 mGal/km).");
	GMT_Usage (API, 1, "\n-G<outfile>");
	GMT_Usage (API, -2, "Set name of output file. Grid file name is requires unless -N is used.");
	GMT_Usage (API, 1, "\n-H<H>/<rho_l>/<rho_h>[+d<densify>][+p<power>]");
	GMT_Usage (API, -2, "Set parameters for the reference seamount height (in m), flank and deep core densities (kg/m^3 or g/cm^3). "
		"Control the density function by these modifiers:");
	GMT_Usage (API, 3, "+d Density increase (kg/m^3 or g/cm^3) due to water pressure over full depth implied by <H> [0].");
	GMT_Usage (API, 3, "+p Exponential <power> coefficient (> 0) for density change with burial depth [1 (linear)].");
	GMT_Option (API, "I");
	GMT_Usage (API, 1, "\n-L<base>");
	GMT_Usage (API, -2, "Set the lower (base) surface grid or constant of a layer to create prisms for; requires -C and -T [0]");
	GMT_Usage (API, 1, "\n-M[hz]");
	GMT_Usage (API, -2, "Change distance units used, via one or two directives:");
	GMT_Usage (API, 3, "h: All x- and y-distances are given in km [meters].");
	GMT_Usage (API, 3, "z: All z-distances are given in km [meters].");
	GMT_Usage (API, -2, "Note: All horizontal and/or vertical quantities will be affected by a factor of 1000");
	GMT_Usage (API, 1, "\n-N<trktable>");
	GMT_Usage (API, -2, "File with output locations (x,y) where a calculation is requested.  Optionally, z may be read as well if -Z not set. No grid "
		"is produced and output (x,y,z,g) is written to standard output (but see -G; also see -bo for binary output).");
	GMT_Option (API, "R");
	GMT_Usage (API, 1, "\n-S<shapegrd>");
	GMT_Usage (API, -2, "Set the full surface grid height for making prisms of the entire feature (or if -H is set) (or use -L and -T to select a layer); requires -C");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-T<top>");
	GMT_Usage (API, -2, "Set the top surface grid or constant of a layer to create prisms for; requires -C and -L");
	GMT_Usage (API, 1, "\n-W<avedens>");
	GMT_Usage (API, -2, "Write grid with vertically-averaged spatially varying densities created via -C -H to file <avedens>.");
	GMT_Usage (API, 1, "\n-Z<level>");
	GMT_Usage (API, -2, "Set observation level for output locations [0]. "
		"Append either a constant or the name of a grid file with variable levels. "
		"If given a grid then it also defines the output grid.");
	GMT_Usage (API, -2, "Note: Cannot use both -Z<grid> and -R -I [-r].");
	GMT_Option (API, "bo,d,e");
	GMT_Usage (API, 1, "\n-fg Map units (lon, lat in degree, else in m [but see -Mh]).");
	GMT_Option (API, "h,i,o,r,x,.");
	return (GMT_MODULE_USAGE);
}

#define GRAVITATIONAL_CONST_GEOID 6.674e-11	/* To get geoid in meter we divide by g0 in gravprisms_get_one_n_output */
#define GRAVITATIONAL_CONST_FAA   6.674e-6	/* To convert m/s^2 to mGal requires 1e5 */
#define GRAVITATIONAL_CONST_VGG   6.674e-2	/* To convert mGal/m to 0.1 mGal/km requires an additional 1e4 */

/* Geoid: Carefully checking terms to avoid divisions by zero in atan or log (zero) */
#define zatan(a,b) ((fabs(b) < GMT_CONV15_LIMIT) ? 0.0 : atan(a/b))		/* For safe atan (a/b) */
#define zlog(a,b)  ((fabs(b) < GMT_CONV15_LIMIT) ? 0.0 : a * log(b))	/* For safe a * log(b) */

GMT_LOCAL double geoidprism (double dx1, double dx2, double dy1, double dy2, double dz1, double dz2, double rho) {
	/* Geoid anomaly from a single vertical prism [Nagy et al, 2000] */
	double n, dx1_sq, dx2_sq, dy1_sq, dy2_sq, dz1_sq, dz2_sq;
	double R111, R112, R121, R122, R211, R212, R221, R222;
	double n111, n112, n121, n122, n211, n212, n221, n222;
	double dx1dy1, dx2dy1, dx1dy2, dx2dy2;
	double dx1dz1, dx2dz1, dx1dz2, dx2dz2;
	double dy1dz1, dy2dz1, dy1dz2, dy2dz2;

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
	/* Get cross-terms */
	dx1dy1 = dx1 * dy1;	dx2dy1 = dx2 * dy1;	dx1dy2 = dx1 * dy2;	dx2dy2 = dx2 * dy2;
	dx1dz1 = dx1 * dz1;	dx2dz1 = dx2 * dz1;	dx1dz2 = dx1 * dz2;	dx2dz2 = dx2 * dz2;
	dy1dz1 = dy1 * dz1;	dy2dz1 = dy2 * dz1;	dy1dz2 = dy1 * dz2;	dy2dz2 = dy2 * dz2;
	/* Evaluate at dz1 */
	n111 = -(0.5 * (dx1_sq * zatan (dy1dz1, (dx1 * R111)) + dy1_sq * zatan (dx1dz1, (dy1 * R111)) + dz1_sq * zatan (dx1dy1, (dz1 * R111))) - zlog (dx1dz1, R111 + dy1) - zlog (dy1dz1, R111 + dx1) - zlog (dx1dy1, R111 + dz1));
	n112 = +(0.5 * (dx2_sq * zatan (dy1dz1, (dx2 * R112)) + dy1_sq * zatan (dx2dz1, (dy1 * R112)) + dz1_sq * zatan (dx2dy1, (dz1 * R112))) - zlog (dx2dz1, R112 + dy1) - zlog (dy1dz1, R112 + dx2) - zlog (dx2dy1, R112 + dz1));
	n121 = +(0.5 * (dx1_sq * zatan (dy2dz1, (dx1 * R121)) + dy2_sq * zatan (dx1dz1, (dy2 * R121)) + dz1_sq * zatan (dx1dy2, (dz1 * R121))) - zlog (dx1dz1, R121 + dy2) - zlog (dy2dz1, R121 + dx1) - zlog (dx1dy2, R121 + dz1));
	n122 = -(0.5 * (dx2_sq * zatan (dy2dz1, (dx2 * R122)) + dy2_sq * zatan (dx2dz1, (dy2 * R122)) + dz1_sq * zatan (dx2dy2, (dz1 * R122))) - zlog (dx2dz1, R122 + dy2) - zlog (dy2dz1, R122 + dx2) - zlog (dx2dy2, R122 + dz1));
	/* Evaluate at dz2 */
	n211 = +(0.5 * (dx1_sq * zatan (dy1dz2, (dx1 * R211)) + dy1_sq * zatan (dx1dz2, (dy1 * R211)) + dz2_sq * zatan (dx1dy1, (dz2 * R211))) - zlog (dx1dz2, R211 + dy1) - zlog (dy1dz2, R211 + dx1) - zlog (dx1dy1, R211 + dz2));
	n212 = -(0.5 * (dx2_sq * zatan (dy1dz2, (dx2 * R212)) + dy1_sq * zatan (dx2dz2, (dy1 * R212)) + dz2_sq * zatan (dx2dy1, (dz2 * R212))) - zlog (dx2dz2, R212 + dy1) - zlog (dy1dz2, R212 + dx2) - zlog (dx2dy1, R212 + dz2));
	n221 = -(0.5 * (dx1_sq * zatan (dy2dz2, (dx1 * R221)) + dy2_sq * zatan (dx1dz2, (dy2 * R221)) + dz2_sq * zatan (dx1dy2, (dz2 * R221))) - zlog (dx1dz2, R221 + dy2) - zlog (dy2dz2, R221 + dx1) - zlog (dx1dy2, R221 + dz2));
	n222 = +(0.5 * (dx2_sq * zatan (dy2dz2, (dx2 * R222)) + dy2_sq * zatan (dx2dz2, (dy2 * R222)) + dz2_sq * zatan (dx2dy2, (dz2 * R222))) - zlog (dx2dz2, R222 + dy2) - zlog (dy2dz2, R222 + dx2) - zlog (dx2dy2, R222 + dz2));

	n = -rho * GRAVITATIONAL_CONST_GEOID * (n111 + n112 + n121 + n122 + n211 + n212 + n221 + n222);

	return (n);
}

GMT_LOCAL double gravprism (double dx1, double dx2, double dy1, double dy2, double dz1, double dz2, double rho) {
	/* Gravity anomaly from a single vertical prism [Grant & West, 1965] */
	double g, dx1_sq, dx2_sq, dy1_sq, dy2_sq, dz1_sq, dz2_sq;
	double R111, R112, R121, R122, R211, R212, R221, R222;
	double g111, g112, g121, g122, g211, g212, g221, g222;
	double dx1dy1, dx2dy1, dx1dy2, dx2dy2;

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
	/* Get cross-terms */
	dx1dy1 = dx1 * dy1;	dx2dy1 = dx2 * dy1;	dx1dy2 = dx1 * dy2;	dx2dy2 = dx2 * dy2;
	/* Evaluate at dz1 */
	g111 = -(dz1 * atan (dx1dy1 / (dz1 * R111)) - dx1 * log (R111 + dy1) - dy1 * log (R111 + dx1));
	g112 = +(dz1 * atan (dx2dy1 / (dz1 * R112)) - dx2 * log (R112 + dy1) - dy1 * log (R112 + dx2));
	g121 = +(dz1 * atan (dx1dy2 / (dz1 * R121)) - dx1 * log (R121 + dy2) - dy2 * log (R121 + dx1));
	g122 = -(dz1 * atan (dx2dy2 / (dz1 * R122)) - dx2 * log (R122 + dy2) - dy2 * log (R122 + dx2));
	/* Evaluate at dz2 */
	g211 = +(dz2 * atan (dx1dy1 / (dz2 * R211)) - dx1 * log (R211 + dy1) - dy1 * log (R211 + dx1));
	g212 = -(dz2 * atan (dx2dy1 / (dz2 * R212)) - dx2 * log (R212 + dy1) - dy1 * log (R212 + dx2));
	g221 = -(dz2 * atan (dx1dy2 / (dz2 * R221)) - dx1 * log (R221 + dy2) - dy2 * log (R221 + dx1));
	g222 = +(dz2 * atan (dx2dy2 / (dz2 * R222)) - dx2 * log (R222 + dy2) - dy2 * log (R222 + dx2));

	g = -rho * GRAVITATIONAL_CONST_FAA * (g111 + g112 + g121 + g122 + g211 + g212 + g221 + g222);

	return (g);
}

GMT_LOCAL double vggprism (double dx1, double dx2, double dy1, double dy2, double dz1, double dz2, double rho) {
	/* Vertical gravity gradient anomaly from a single vertical prism [Kim & Wessel, 2016] */
	double v, dx1_sq, dx2_sq, dy1_sq, dy2_sq, dz1_sq, dz2_sq;
	double R111, R112, R121, R122, R211, R212, R221, R222;
	double v111, v112, v121, v122, v211, v212, v221, v222;
	double dx1dy1, dx2dy1, dx1dy2, dx2dy2;

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
	/* Get cross-terms */
	dx1dy1 = dx1 * dy1;	dx2dy1 = dx2 * dy1;	dx1dy2 = dx1 * dy2;	dx2dy2 = dx2 * dy2;
	/* Evaluate at dz1 */
	v111 = -atan (dx1dy1 / (dz1 * R111));
	v112 = +atan (dx2dy1 / (dz1 * R112));
	v121 = +atan (dx1dy2 / (dz1 * R121));
	v122 = -atan (dx2dy2 / (dz1 * R122));
	/* Evaluate at dz2 */
	v211 = +atan (dx1dy1 / (dz2 * R211));
	v212 = -atan (dx2dy1 / (dz2 * R212));
	v221 = -atan (dx1dy2 / (dz2 * R221));
	v222 = +atan (dx2dy2 / (dz2 * R222));

	v = -rho * GRAVITATIONAL_CONST_VGG * (v111 + v112 + v121 + v122 + v211 + v212 + v221 + v222);

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

GMT_LOCAL double gravprisms_get_one_g_output_geo (double x, double y, double z, uint64_t n_prisms, double **P, double unused) {
	/* (x, y, z) is the observation point */
	double g = 0.0;
	double dx1, dx2, dy1, dy2, ym;
	gmt_M_unused (unused);
	for (uint64_t k = 0; k < n_prisms; k++) {
		/* Got lon, lat and must convert to Flat-Earth km */
		ym = 0.5 * (P[2][k] + P[3][k]);	/* Prism mid-y-value */
		dx1 = DX_FROM_DLON (P[0][k], x, ym, y);
		dx2 = DX_FROM_DLON (P[1][k], x, ym, y);
		dy1 = DY_FROM_DLAT (P[2][k], y);
		dy2 = DY_FROM_DLAT (P[3][k], y);
		g += gravprism (dx1, dx2, dy1, dy2, P[4][k]-z, P[5][k]-z, P[6][k]);
	}
	return (g);
}

GMT_LOCAL double gravprisms_get_one_n_output (double x, double y, double z, uint64_t n_prisms, double **P, double constant) {
	/* (x, y, z) is the observation point */
	double n = 0.0;
	for (uint64_t k = 0; k < n_prisms; k++)
		n += geoidprism (P[0][k]-x, P[1][k]-x, P[2][k]-y, P[3][k]-y, P[4][k]-z, P[5][k]-z, P[6][k]);
	return (n * constant);	/* To get geoid in meter */
}

GMT_LOCAL double gravprisms_get_one_n_output_geo (double x, double y, double z, uint64_t n_prisms, double **P, double constant) {
	/* (x, y, z) is the observation point */
	double n = 0.0;
	double dx1, dx2, dy1, dy2, ym;
	for (uint64_t k = 0; k < n_prisms; k++) {
		/* Got lon, lat and must convert to Flat-Earth km */
		ym = 0.5 * (P[2][k] + P[3][k]);	/* Prism mid-y-value */
		dx1 = DX_FROM_DLON (P[0][k], x, ym, y);
		dx2 = DX_FROM_DLON (P[1][k], x, ym, y);
		dy1 = DY_FROM_DLAT (P[2][k], y);
		dy2 = DY_FROM_DLAT (P[3][k], y);
		n += geoidprism (dx1, dx2, dy1, dy2, P[4][k]-z, P[5][k]-z, P[6][k]);
	}
	return (n * constant);	/* To get geoid in meter */
}

GMT_LOCAL double gravprisms_get_one_v_output (double x, double y, double z, uint64_t n_prisms, double **P, double unused) {
	/* (x, y, z) is the observation point */
	double v = 0.0;
	gmt_M_unused (unused);
	for (uint64_t k = 0; k < n_prisms; k++)
		v += vggprism (P[0][k]-x, P[1][k]-x, P[2][k]-y, P[3][k]-y, P[4][k]-z, P[5][k]-z, P[6][k]);
	return (v);	/* Converted units from mGal/m to Eotvos = 0.1 mGal/km */
}

GMT_LOCAL double gravprisms_get_one_v_output_geo (double x, double y, double z, uint64_t n_prisms, double **P, double unused) {
	/* (x, y, z) is the observation point */
	double v = 0.0;
	double dx1, dx2, dy1, dy2, ym;
	gmt_M_unused (unused);
	for (uint64_t k = 0; k < n_prisms; k++) {
		/* Got lon, lat and must convert to Flat-Earth km */
		ym = 0.5 * (P[2][k] + P[3][k]);	/* Prism mid-y-value */
		dx1 = DX_FROM_DLON (P[0][k], x, ym, y);
		dx2 = DX_FROM_DLON (P[1][k], x, ym, y);
		dy1 = DY_FROM_DLAT (P[2][k], y);
		dy2 = DY_FROM_DLAT (P[3][k], y);
		v += vggprism (dx1, dx2, dy1, dy2, P[4][k]-z, P[5][k]-z, P[6][k]);
	}
	return (v);	/* Converted units from mGal/m to Eotvos = 0.1 mGal/km */
}

GMT_LOCAL double gravprisms_mean_density (struct GRAVPRISMS_CTRL *Ctrl, double h, double z1, double z2) {
	/* Passing in the current seamounts height h(r) and the two depths z1(r) and z2(r) defining a prism.
	 * When doing the whole seamount we pass z2 = 0 and z1 = h(r).
	 * The vertically averaged density for this radial position and range of z is returned */
	double u1 = (h - z1) / Ctrl->H.H;
	double u2 = (h - z2) / Ctrl->H.H;
	double q = (Ctrl->H.H - h) / Ctrl->H.H;
	double dz = z2 - z1;	/* Prism height */

	double rho = Ctrl->H.rho_l + Ctrl->H.densify * q + Ctrl->H.del_rho * Ctrl->H.H * (pow (u1, Ctrl->H.p1) - pow (u2, Ctrl->H.p1)) / (dz * (Ctrl->H.p1));
	return (rho);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gravprisms (void *V_API, int mode, void *args) {
	int error = 0;
	uint64_t tbl, seg, row, col, k, node, n_prisms = 0;

	bool flat_earth = false;

	char *uname[2] = {"meter", "km"}, *kind[3] = {"FAA", "VGG", "GEOID"}, remark[GMT_LEN64] = {""};
	double z_level, rho, dx, dy, lat, G0, scl_xy, scl_z, i_scl_xy, i_scl_z, *prism[7];
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

	/* Handle any -M[hv] settings so that internally we have meters */
	scl_xy = (Ctrl->M.active[GRAVPRISMS_HOR]) ? METERS_IN_A_KM : 1.0;
	scl_z  = (Ctrl->M.active[GRAVPRISMS_VER]) ? METERS_IN_A_KM : 1.0;
	if (Ctrl->A.active) scl_z = -scl_z;
	i_scl_xy = 1.0 / scl_xy;	/* Scale use for output horizontal distances */
	i_scl_z  = 1.0 / scl_z;		/* Scale use for output vertical distances */

	if (Ctrl->C.active) {	/* Need to create prisms from two surfaces first */
		struct GMT_GRID *B = NULL, *T = NULL, *H = NULL, *Rho = NULL;
		double base = 0.0, top = 0.0, z1, z2, z_prev, z_next, z_mid, rs = 0.0, ws = 0.0;
		size_t n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;

		if (Ctrl->L.active) {	/* Specified layer base */
			if (access (Ctrl->L.file, F_OK))	/* No file, just a constant */
				base = atof (Ctrl->L.file);
			else if ((B = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->L.file, NULL)) == NULL)
				Return (API->error);
		}
		if (Ctrl->T.active) {	/* Specified layer top */
			if (access (Ctrl->T.file, F_OK))	/* No file, just a constant */
				top = atof (Ctrl->T.file);
			else if ((T = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->T.file, NULL)) == NULL)
				Return (API->error);
		}
		if (Ctrl->S.active) {	/* Full shape needed (via -H or as the entire grid given) */
			if ((H = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->S.file, NULL)) == NULL)
				Return (API->error);
			if (!Ctrl->T.active) T = H;	/* Top and height are the same if just -S is set */
		}
		else	/* Use H for info for any of the grids */
			H = (B) ? B : T;
		if (Ctrl->W.active) {	/* Write spatially varying average density contrasts */
			if ((Rho = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, H)) == NULL)
				Return (API->error);
			for (node = 0; node < Rho->header->size; node++) Rho->data[node] = GMT->session.f_NaN;	/* Init to NaN */

		}
		else if (Ctrl->D.file) {	/* Read spatially varying average density contrasts */
			if ((Rho = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->D.file, NULL)) == NULL)
				Return (API->error);
		}
		dx = 0.5 * H->header->inc[GMT_X];	dy = 0.5 * H->header->inc[GMT_Y];
		rho = Ctrl->D.rho;	/* Only initialized if -D is set */
		for (k = 0; k < 7; k++)
			prism[k] = gmt_M_memory (GMT, NULL, n_alloc, double);

		gmt_M_grd_loop (GMT, H, row, col, node) {
			z2 = (T) ? T->data[node] : top;
			z1 = (B) ? B->data[node] : base;
			if (z2 <= z1) continue;	/* No layer thickness detected */
			z_prev = z1;	/* We start exactly at z = z1 */
			do {	/* Will at least be one prism */
				if (Ctrl->H.active) {	/* Variable density means stacked prisms, most of thickness dz, at same point */
					/* First and last prisms in the stack are adjusted to have the height needed to match the surface boundaries */
					z_next = (floor (z_prev / Ctrl->C.dz) + 1.0) * Ctrl->C.dz;	/* Presumably next regular z-spacing */
					if (z_next <= z_prev) z_next += Ctrl->C.dz;	/* Can happen if z1 is a multiple of dz */
					else if (z_next > z2) z_next = z2;	/* At the top, clip to limit */
					z_mid = 0.5 * (z_prev + z_next);	/* Middle of prism - used to look up density */
					rho = gravprisms_mean_density (Ctrl, H->data[node], z_prev, z_next);
				}
				else {	/* Constant density rho (set above via -D) or by Rho (via -W), just need a single prism per location */
					z_next = z2;
					if (Ctrl->D.file)
						rho = Rho->data[node];	/* Update constant density for this prism */
				}
				if (n_prisms == n_alloc) {	/* Need to allocate more memory for the prisms */
					n_alloc <<= 1;	/* Double it */
					for (k = 0; k < 7; k++)
						prism[k] = gmt_M_memory (GMT, prism[k], n_alloc, double);
				}
				/* Here we ensure prism is in meters */
				prism[0][n_prisms] = scl_xy * (H->x[col] - dx);	prism[1][n_prisms] = scl_xy * (H->x[col] + dx);
				prism[2][n_prisms] = scl_xy * (H->y[row] - dy);	prism[3][n_prisms] = scl_xy * (H->y[row] + dy);
				prism[4][n_prisms] = scl_z * z_prev;			prism[5][n_prisms] = scl_z * z_next;
				prism[6][n_prisms] = rho;
				n_prisms++;
				z_prev = z_next;	/* The the top of this prism be the bottom of the next */
			} while (z_prev < z2);	/* Until we run out of this stack */
			if (Ctrl->W.active) {	/* Get vertical average density and keep track of means */
				double dz = z2 - z1;
				Rho->data[node] = gravprisms_mean_density (Ctrl, H->data[node], z1, z2);
				rs += Rho->data[node] * dz;
				ws += dz;
			}
		}
		/* Finalize allocation */
		for (k = 0; k < 7; k++)
			prism[k] = gmt_M_memory (GMT, prism[k], n_prisms, double);
		GMT_Report (API, GMT_MSG_INFORMATION, "# of prisms constructed: %" PRIu64 "\n", n_prisms);
		if (!Ctrl->T.active && T) T = NULL;	/* Undo what we set earlier */
		if (B && GMT_Destroy_Data (API, &B) != GMT_NOERROR) {
			error = GMT_MEMORY_ERROR;
			goto end_it_all;
		}
		if (T && GMT_Destroy_Data (API, &T) != GMT_NOERROR) {
			error = GMT_MEMORY_ERROR;
			goto end_it_all;
		}
		if (Ctrl->S.active && GMT_Destroy_Data (API, &H) != GMT_NOERROR) {
			error = GMT_MEMORY_ERROR;
			goto end_it_all;
		}
		if (Ctrl->W.active) {	/* Output the mean density grid */
			char remark[GMT_GRID_REMARK_LEN160] = {""};
			if (ws > 0.0) rs /= ws;	/* Mean load density for this feature */
			sprintf (remark, "Mean Load Density: %lg", rs);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, remark, Rho)) Return (API->error);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->W.file, Rho) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to write average density grid to file %s\n", Ctrl->W.file);
				error = GMT_RUNTIME_ERROR;
				goto end_it_all;
			}
		}
		if (Ctrl->C.file) {	/* Wish to write prisms to a data table */
			uint64_t dim[GMT_DIM_SIZE] = {1, 1, n_prisms, 7};
			if ((P = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
				Return (API->error);
			S = P->table[0]->segment[0];	/* The only segment in the table */
			for (k = 0; k < n_prisms; k++)	{ /* Unscramble edges to coordinates and dimensions and possibly convert Cartesian data back to km */
				S->data[0][k] = 0.5 * (prism[1][k] + prism[0][k]) * i_scl_xy;	/* Get x */
				S->data[1][k] = 0.5 * (prism[3][k] + prism[2][k]) * i_scl_xy;	/* Get y */
				S->data[2][k] = prism[4][k] * i_scl_xy;	/* Get z_low */
				S->data[3][k] = prism[5][k] * i_scl_xy;	/* Get z_high */
				S->data[4][k] = (prism[1][k] - prism[0][k]) * i_scl_xy;	/* Get dx */
				S->data[5][k] = (prism[3][k] - prism[2][k]) * i_scl_xy;	/* Get dy */
			}
			gmt_M_memcpy (S->data[6], prism[6], n_prisms, double);	/* Copy over densities */
			GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, P);
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_WRITE_SET, NULL, Ctrl->C.file, P) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to write prisms to file %s\n", Ctrl->C.file);
				error = GMT_RUNTIME_ERROR;
				goto end_it_all;
			}
			if (Ctrl->C.quit) goto end_it_all;
		}
	}
	else {	/* Read prisms from stdin or input file(s) */
		unsigned int n_expected = 7;	/* Max number of columns */
		/* Specify input expected columns to be at least 3 */
		if (Ctrl->D.active) n_expected--;		/* Not reading density from records */
		if (Ctrl->E.active) n_expected -= 2;	/* Not reading dx dy from records */
		if ((error = GMT_Set_Columns (API, GMT_IN, n_expected, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
		/* Register likely model files unless the caller has already done so */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
			Return (API->error);
		}
		/* Read the entire prism file(s) */
		if ((P = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}
		/* To avoid the need to loop over tables and segments, we extrac the data into a separate array and compute prism edges instead */
		for (k = 0; k < 7; k++) 
			prism[k] = gmt_M_memory (GMT, NULL, P->n_records, double);

		/* Fill in prism arrays from data set, including optional choices for fixed or variable dimension and density, then free the dataset */
		dx = 0.5 * Ctrl->E.dx;	dy = 0.5 * Ctrl->E.dy;	/* Distances from prism center to respective edges */
		rho = Ctrl->D.rho;
		col = (Ctrl->E.active) ? 4 : 6;	/* Input column for density, if -D is not set */
		for (tbl = k = 0; tbl < P->n_tables; tbl++) {
			for (seg = 0; seg < P->table[tbl]->n_segments; seg++) {
				S = P->table[tbl]->segment[seg];
				for (row = 0; row < S->n_rows; row++, k++) {
					/* x y z_lo z_hi [dx dy] [rho] */
					if (!Ctrl->E.active) {	/* Update the half-dimensions of the x/y-sides */
						dx = 0.5 * S->data[4][row];
						dy = 0.5 * S->data[5][row];
					}
					if (!Ctrl->D.active) rho = S->data[col][row];
					/* Here we ensure prism is in meters */
					prism[0][k] = (S->data[GMT_X][row] - dx) * scl_xy;	/* Store the x-coordinates of the x-edges of prism */
					prism[1][k] = (S->data[GMT_X][row] + dx) * scl_xy;
					prism[2][k] = (S->data[GMT_Y][row] - dy) * scl_xy;	/* Store the y-coordinates of the y-edges of prism */
					prism[3][k] = (S->data[GMT_Y][row] + dy) * scl_xy;
					prism[4][k] = S->data[2][row] * scl_z;	/* Store the z-coordinates of the z-edges of prism */
					prism[5][k] = S->data[3][row] * scl_z;
					prism[6][k] = rho;	/* Store the fixed or variable density of prism */
				}
			}
		}
		n_prisms = k;
		/* Free the data set */
		if (GMT_Destroy_Data (API, &P) != GMT_NOERROR) {
			error = GMT_MEMORY_ERROR;
			goto end_it_all;
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "# of prisms read: %" PRIu64 "\n", n_prisms);
	}

	if (n_prisms == 0) {
		GMT_Report (API, GMT_MSG_WARNING, "No prisms found - exiting\n");
		goto end_it_all;
	}

	if (Ctrl->Z.mode == 1) {	/* Got grid with observation levels which also sets output locations; it could also set -fg so do this first */
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->Z.file, NULL)) == NULL) {
			error = API->error;
			goto end_it_all;
		}
		if (gmt_M_is_geographic (GMT, GMT_IN)) lat = 0.5 * (G->header->wesn[YLO] + G->header->wesn[YHI]);	/* Mid-latitude needed if geoid is selected */
	}
	else if (GMT->common.R.active[RSET]) {	/* Gave -R -I [-r] and possibly -fg indirectly via geographic coordinates in -R */
		if ((G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL,
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) {
			error = API->error;
			goto end_it_all;
		}
		if (gmt_M_is_geographic (GMT, GMT_IN)) lat = 0.5 * (G->header->wesn[YLO] + G->header->wesn[YHI]);	/* Mid-latitude needed if geoid is selected */
	}
	else {	/* Got a dataset with output locations via -N */
		unsigned int n_expected = 3;	/* Max number of columns is 3 (x, y, z) but if -Z is set then just (x, y) */
		if (Ctrl->Z.active) n_expected--;
		gmt_disable_bghio_opts (GMT);	/* Do not want any -b -g -h -i -o to affect the reading from the -N file */
		if ((error = GMT_Set_Columns (API, GMT_IN, n_expected, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			error = API->error;
			goto end_it_all;
		}
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->N.file, NULL)) == NULL) {
			error = API->error;
			goto end_it_all;
		}
		if (D->n_columns < n_expected) {
			GMT_Report (API, GMT_MSG_ERROR, "Input file %s has %d column(s) but %d are needed\n", Ctrl->N.file, (int)D->n_columns, n_expected);
			error = GMT_DIM_TOO_SMALL;
			goto end_it_all;
		}
		gmt_reenable_bghio_opts (GMT);	/* Recover settings provided by user (if -b -g -h -i were used at all) */
		if (gmt_M_is_geographic (GMT, GMT_IN)) lat = 0.5 * (D->min[GMT_Y] + D->max[GMT_Y]);	/* Mid-latitude needed if geoid is selected */
	}

	flat_earth = gmt_M_is_geographic (GMT, GMT_IN);		/* If true then input is in degrees and we must convert to km later on */

	if (flat_earth && Ctrl->M.active[GRAVPRISMS_HOR]) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -M: Cannot specify both geographic coordinates (degrees) AND -Mh\n");
		error = GMT_RUNTIME_ERROR;
		goto end_it_all;
	}

	if (Ctrl->A.active) Ctrl->Z.level = -Ctrl->Z.level;

	/* Read polygon information from multiple segment file */
	GMT_Report (API, GMT_MSG_INFORMATION, "All x/y-values are assumed to be given in %s\n", uname[Ctrl->M.active[GRAVPRISMS_HOR]]);
	GMT_Report (API, GMT_MSG_INFORMATION, "All z-values are assumed to be given in %s\n",   uname[Ctrl->M.active[GRAVPRISMS_VER]]);

	/* Now we can write (if -V) to the screen the user's polygon model characteristics. */

	GMT_Report (API, GMT_MSG_INFORMATION, "Start calculating %s\n", kind[Ctrl->F.mode]);

	switch (Ctrl->F.mode) {		/* Set pointer to chosen geopotential evaluation function */
		case GRAVPRISMS_VGG:
			eval = (flat_earth) ? &gravprisms_get_one_v_output_geo : &gravprisms_get_one_v_output;
		 	break;
		case GRAVPRISMS_GEOID:
			eval = (flat_earth) ? &gravprisms_get_one_n_output_geo : &gravprisms_get_one_n_output;
			G0 = (Ctrl->F.lset) ? g_normal (Ctrl->F.lat) : g_normal (lat);
			G0 = 1.0 / G0;	/* To avoid dividing in the loop */
			break;
		case GRAVPRISMS_FAA:
			eval = (flat_earth) ? &gravprisms_get_one_g_output_geo : &gravprisms_get_one_g_output;
		 	break;
		default:
			/* Just for Coverity */
			break;
	}

	if (Ctrl->N.active) {	/* Single loop over specified output locations */
		unsigned int wmode = GMT_ADD_DEFAULT;
		double out[4];	/* x, y, z, g|n|v */
		struct GMT_RECORD *Rec = gmt_new_record (GMT, out, NULL);
		/* Must register Ctrl->G.file first since we are going to writing rec-by-rec */
		if (Ctrl->G.active) {
			int out_ID;
			if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, NULL, Ctrl->G.file)) == GMT_NOTSET) {
				error = API->error;
				goto end_it_all;
			}
			wmode = GMT_ADD_EXISTING;
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, 4, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			goto end_it_all;
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, wmode, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			error = API->error;
			goto end_it_all;
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			error = API->error;
			goto end_it_all;
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			error = API->error;
			goto end_it_all;
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
#pragma omp parallel for private(row,z_level) shared(S,Ctrl,GMT,eval,scl_xy,scl_z,n_prisms,prism,G0)
#endif
				/* Separate the calculation from the output in two separate row-loops since cannot do rec-by-rec output
				 * with OpenMP due to race conditions that would mess up the output order */
				for (row = 0; row < (int64_t)S->n_rows; row++) {	/* Calculate attraction at all output locations for this segment */
					z_level = (S->n_columns == 3 && !Ctrl->Z.active) ? S->data[GMT_Z][row] : Ctrl->Z.level;	/* Default observation z level unless provided in input file */
					GMT->hidden.mem_coord[GMT_X][row] = eval (S->data[GMT_X][row] * scl_xy, S->data[GMT_Y][row] * scl_xy, z_level * scl_z, n_prisms, prism, G0);
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
			error = API->error;
			goto end_it_all;
		}
	}
	else {	/* Dealing with a grid */
		openmp_int row, col, n_columns = (openmp_int)G->header->n_columns, n_rows = (openmp_int)G->header->n_rows;	/* To shut up compiler warnings */
		double y_obs, *x_obs = gmt_M_memory (GMT, NULL, G->header->n_columns, double);
		for (col = 0; col < n_columns; col++) {
			x_obs[col] = scl_xy * gmt_M_grd_col_to_x (GMT, col, G->header);
		}
#ifdef _OPENMP
		/* Spread calculation over selected cores */
#pragma omp parallel for private(row,y_obs,col,node,z_level) shared(n_rows,scl_xy,GMT,G,Ctrl,n_columns,eval,x_obs,scl_z,n_prisms,prism,G0)
#endif
		for (row = 0; row < n_rows; row++) {	/* Do row-by-row and report on progress if -V */
			y_obs = scl_xy * gmt_M_grd_row_to_y (GMT, row, G->header);
#ifndef _OPENMP
			GMT_Report (API, GMT_MSG_INFORMATION, "Finished row %5d\n", row);
#endif
			for (col = 0; col < n_columns; col++) {
				/* Loop over cols; always save the next level before we update the array at that col */
				node = gmt_M_ijp (G->header, row, col);
				z_level = (Ctrl->Z.mode == 1) ? G->data[node] : Ctrl->Z.level;	/* Default observation z level unless provided in input grid */
				G->data[node] = (gmt_grdfloat) eval (x_obs[col], y_obs, z_level * scl_z, n_prisms, prism, G0);
			}
		}
		gmt_M_free (GMT, x_obs);
		GMT_Report (API, GMT_MSG_INFORMATION, "Create %s\n", Ctrl->G.file);
		sprintf (remark, "Calculated 3-D %s", kind[Ctrl->F.mode]);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, remark, G)) {
			error = API->error;
			goto end_it_all;
		}
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) {
			error = API->error;
			goto end_it_all;
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, G) != GMT_NOERROR) {
			error = API->error;
			goto end_it_all;
		}
	}

end_it_all:

	/* Free the prism information */
	for (k = 0; k < 7; k++)
		gmt_M_free (GMT, prism[k]);

	Return (GMT_NOERROR);
}
