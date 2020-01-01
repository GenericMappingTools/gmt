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
 *--------------------------------------------------------------------
 *
 *
 * API functions to support the grdgravmag3d application.
 *
 * Brief synopsis: Compute the gravity anomaly of the volume contained between
 *                 a surface provided by one grid and a plane, or between a top
 *                 and a bottom surface provided by two grids.
 *
 * Author:	Joaquim Luis
 * Date:	1-Apr-2012
 *
 *          Removed #if 0 code that dealt with side faces in rev r12620
 *
 */

#include "gmt_dev.h"
#include "okbfuns.h"
#include "../mgd77/mgd77.h"
#ifdef HAVE_GLIB_GTHREAD
#include <glib.h>
#endif

#define THIS_MODULE_CLASSIC_NAME	"grdgravmag3d"
#define THIS_MODULE_MODERN_NAME	"grdgravmag3d"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Computes the gravity effect of one (or two) grids by the method of Okabe"
#define THIS_MODULE_KEYS	"<G{+,FD(,GG}"
#define THIS_MODULE_NEEDS	"g"
#define THIS_MODULE_OPTIONS "-:RVfx"

typedef void (*PFV) ();		/* pointer to a function returning void */
typedef double (*PFD) ();		/* pointer to a function returning double */

struct GRDOKB_CTRL {

	struct GRDOKB_In {
		bool active;
		char *file[3];
	} In;

	struct GRDOKB_C {	/* -C */
		bool active;
		double rho;
	} C;
	struct GRDOKB_D {	/* -D */
		bool active;
		gmt_grdfloat z_dir;
	} D;
	struct GRDOKB_E {	/* -E */
		bool active;
		double thickness;
	} E;
	struct GRDOKB_I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct GRDOKB_F {	/* -F<xyfile> */
		bool active;
		char *file;
	} F;
	struct GRDOKB_G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct GRDOKB_H {	/* -H */
		bool active;
		bool bhatta;
		bool pirtt;
		bool x_comp, y_comp, z_comp, f_tot, h_comp;
		bool got_incgrid, got_decgrid, got_maggrid, do_igrf;
		char *incfile, *decfile, *magfile;
		double	t_dec, t_dip, m_int, m_dec, m_dip, koningsberg;
	} H;
	struct GRDOKB_L {	/* -L */
		double zobs;
	} L;
	struct GRDOKB_Q {	/* -Q */
		bool active;
		unsigned int n_pad;
		char region[GMT_BUFSIZ];	/* gmt_parse_R_option has this!!!! */
		double pad_dist;
	} Q;
	struct GRDOKB_S {	/* -S */
		bool active;
		double radius;
	} S;
	struct GRDOKB_T {	/* -T<grdfile> */
		double	year;
	} T;
	struct GRDOKB_Z {	/* -Z */
		bool top, bot;
		double z0;
	} Z;
	struct GRDOKB_box {	/* No option, just a container */
		bool is_geog;
		double	d_to_m, *mag_int, lon_0, lat_0;
	} box;
};

struct THREAD_STRUCT {
	unsigned int row, r_start, r_stop, n_pts, thread_num;
	double *x_grd, *x_grd_geo, *y_grd, *y_grd_geo, *x_obs, *y_obs, *cos_vec, *g;
	struct MAG_VAR *okabe_mag_var;
	struct LOC_OR *loc_or;
	struct BODY_DESC *body_desc;
	struct BODY_VERTS *body_verts;
	struct GRDOKB_CTRL *Ctrl;
	struct GMT_GRID *Grid;
	struct GMT_GRID *Gout;
	struct GMT_GRID *Gsource;
	struct GMT_CTRL *GMT;
};

GMT_LOCAL int grdgravmag3d_body_set_tri (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
	struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
	double *cos_vec, unsigned int j, unsigned int i, unsigned int inc_j, unsigned int inc_i);
GMT_LOCAL int grdgravmag3d_body_set_prism(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
	struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
	double *cos_vec, unsigned int j, unsigned int i, unsigned int inc_j, unsigned int inc_i);
GMT_LOCAL int grdgravmag3d_body_desc_tri(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc,
	struct BODY_VERTS **body_verts, unsigned int face);
GMT_LOCAL int grdgravmag3d_body_desc_prism(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc,
	struct BODY_VERTS **body_verts, unsigned int face);
GMT_LOCAL void grdgravmag3d_calc_surf (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid, struct GMT_GRID *Gout,
	struct GMT_GRID *Gsource, double *g, unsigned int n_pts, double *x_grd, double *y_grd, double *x_grd_geo, double *y_grd_geo,
	double *x_obs, double *y_obs, double *cos_vec, struct MAG_VAR *okabe_mag_var, struct LOC_OR *loc_or,
	struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts);
GMT_LOCAL  double mprism (struct GMT_CTRL *GMT, double x_o, double y_o, double z_o, double mag, bool is_grav,
	struct BODY_DESC bd_desc, struct BODY_VERTS *body_verts, unsigned int km, unsigned int i_comp, struct LOC_OR *loc_or);
GMT_LOCAL  double bhatta (struct GMT_CTRL *GMT, double x_o, double y_o, double z_o, double mag, bool is_grav,
	struct BODY_DESC bd_desc, struct BODY_VERTS *body_verts, unsigned int km, unsigned int i_comp, struct LOC_OR *loc_or);
GMT_LOCAL void grdgravmag3d_calc_surf_ (struct THREAD_STRUCT *t);
GMT_LOCAL double nucleox(double u, double v, double w, double rl, double rm, double rn);
GMT_LOCAL double nucleoy(double u, double v, double w, double rl, double rm, double rn);
GMT_LOCAL double nucleoz(double u, double v, double w, double rl, double rm, double rn);
GMT_LOCAL void dircos(double incl, double decl, double azim, double *a, double *b, double *c);

#if 0
GMT_LOCAL double fast_atan(double x) {
	/* http://nghiaho.com/?p=997 */
	/* Efficient approximations for the arctangent function, Rajan, S. Sichun Wang Inkol, R. Joyal, A., May 2006 */
	return M_PI_4*x - x*(fabs(x) - 1)*(0.2447 + 0.0663*fabs(x));
}
#endif

#define FATAN(x) (fabs(x) > 1) ? atan(x) : (M_PI_4*x - x*(fabs(x) - 1)*(0.2447 + 0.0663*fabs(x)))

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDOKB_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDOKB_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->E.thickness = 500;
	C->L.zobs = 0;
	C->D.z_dir = -1;		/* -1 because Z was positive down for Okabe */
	C->S.radius = 30000;
	C->T.year = 2000;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file[0]);
	gmt_M_str_free (C->In.file[1]);
	gmt_M_str_free (C->F.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->H.magfile);
	gmt_M_str_free (C->H.decfile);
	gmt_M_str_free (C->H.incfile);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s grdfile_top [grdfile_bot] [-C<density>] [-D] [-E<thick>] [-F<xy_file>]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-G<outfile>] [-H<...>] [%s] [-L<z_obs>]\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Q[n<n_pad>]|[pad_dist]|[<w/e/s/n>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-S<radius>] [%s] [-Z[<level>]|[t|p]] [-fg] %s[%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_x_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tgrdfile_top is the grdfile whose gravity effect is to be computed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If two grids are provided then the gravity/magnetic effect of the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   volume between them is computed\n\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Sets body density in SI.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Passes file with locations where anomaly is going to be computed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Sets name of the output grdfile.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Specifies that z is positive down [Default positive up].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Gives layer thickness in m [Default = 500 m].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-H Sets parameters for computation of magnetic anomaly (Can be used multiple times)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <f_dec>/<f_dip> -> geomagnetic declination/inclination.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <m_int>/<m_dec>/<m_dip> -> body magnetic intensity/declination/inclination.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  OR for a grid mode \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +m<magfile> where 'magfile' is the name of the magnetic intensity file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To compute a component, specify any of:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     x|X|e|E  to compute the E-W component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     y|Y|n|N  to compute the N-S component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     z|Z      to compute the Vertical component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     h|H      to compute the Horizontal component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     t|T|f|F  to compute the total field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     For a variable inclination and declination use IGRF. Set any of -H+i|+g|+r|+f|+n to do that.\n");
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t   The new xinc and yinc should be divisible by the old ones (new lattice is subset of old).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Sets level of observation [Default = 0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Map units true; x,y in degrees, dist units in m [Default dist unit = x,y unit].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Extends the domain of computation with respect to output -R region.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qn<N> artificially extends the width of the outer rim of cells to have a fake\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   width of N * dx[/dy].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Q<pad_dist> extend the region by west-pad, east+pad, etc.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Q<region> Same syntax as -R.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-R For new Range of output grid; enter <WESN> (xmin, xmax, ymin, ymax) separated by slashes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses the same region as the input grid].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Sets search radius in km (but only in the two grids mode) [Default = 30 km].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Sets z level of reference plane [Default = 0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, set -Zt or Zb to close the body at its top (bottom) plane.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Converts geographic grids to meters using a \"Flat Earth\" approximation.\n");
#ifdef HAVE_GLIB_GTHREAD
	GMT_Option (API, "x");
#else
	GMT_Message (API, GMT_TIME_NONE, "\t-x Not available since this binary was not build with multi-threading support.\n");
#endif
	GMT_Option (API, ":,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdgravmag3d and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct	GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	int i = 0;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file */
				Ctrl->In.active = true;
				if (n_files == 0)
					Ctrl->In.file[n_files++] = strdup(opt->arg);
				else if (n_files == 1)
					Ctrl->In.file[n_files++] = strdup(opt->arg);
				else {
					n_errors++;
					GMT_Report (API, GMT_MSG_NORMAL, "A maximum of two input grids may be processed\n");
				}
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.rho = atof(opt->arg) * 6.674e-6;
				Ctrl->C.active = true;
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.z_dir = 1;
				break;
			case 'E':
				Ctrl->E.active = true;
				Ctrl->E.thickness = fabs(atof(opt->arg));
				break;
			case 'F':
				Ctrl->F.active = true;
				Ctrl->F.file = strdup(opt->arg);
				break;
 			case 'G':
				Ctrl->G.active = true;
				Ctrl->G.file = strdup(opt->arg);
				break;
			case 'H':
				Ctrl->H.active = true;
				if (opt->arg[0] == '+' && opt->arg[1] == 'm') {
					Ctrl->H.magfile = strdup(&opt->arg[2]);        /* Source (magnetization) grid */
					Ctrl->H.got_maggrid = true;
					break;
				}
				else if (opt->arg[0] == '+' && opt->arg[1] == 'i') {
					if (opt->arg[2]) {
						Ctrl->H.incfile = strdup(&opt->arg[2]);    /* Inclination grid (NOT IMPLEMENTED YET) */
						Ctrl->H.got_incgrid = true;
					}
					else {
						Ctrl->H.do_igrf = true;                     /* Case when -H+i to mean use IGRF */
						if (gmt_M_is_cartesian(GMT, GMT_IN))
							gmt_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
					}
					break;
				}
				else if (opt->arg[0] == '+' && opt->arg[1] == 'd') {
					Ctrl->H.decfile = strdup(&opt->arg[2]);        /* Declination grid (NOT IMPLEMENTED YET) */
					Ctrl->H.got_decgrid = true;
					break;
				}
				else if (opt->arg[0] == '+' && (opt->arg[1] == 'g' || opt->arg[1] == 'r' || opt->arg[1] == 'f' || opt->arg[1] == 'n')) {
					Ctrl->H.do_igrf = true;                         /* Anny of -H+i|+g|+r|+f|+n is allowed to mean use IGRF */
					if (gmt_M_is_cartesian(GMT, GMT_IN))
						gmt_parse_common_options(GMT, "f", 'f', "g"); /* Set -fg unless already set */
					break;
				}

				i = 0;
				if (opt->arg[0] == 'x' || opt->arg[0] == 'X' || opt->arg[0] == 'e' || opt->arg[0] == 'E') {
					Ctrl->H.x_comp = true;
					Ctrl->H.pirtt = true;
				}
				else if (opt->arg[0] == 'y' || opt->arg[0] == 'Y' || opt->arg[0] == 'n' || opt->arg[0] == 'N') {
					Ctrl->H.y_comp = true;
					Ctrl->H.pirtt = true;
				}
				else if (opt->arg[0] == 'z' || opt->arg[0] == 'Z') {
					Ctrl->H.z_comp = true;
					Ctrl->H.pirtt = true;
				}
				else if (opt->arg[0] == 't' || opt->arg[0] == 'T' || opt->arg[0] == 'f' || opt->arg[0] == 'F') {
					Ctrl->H.f_tot  = true;
					Ctrl->H.pirtt = true;
				}
				else if (opt->arg[0] == 'h' || opt->arg[0] == 'H') {
					Ctrl->H.h_comp = true;
					Ctrl->H.pirtt = true;
				}
				if (Ctrl->H.pirtt) i = 1;
				if (opt->arg[i] && (sscanf(&opt->arg[i], "%lf/%lf/%lf/%lf/%lf",
				            &Ctrl->H.t_dec, &Ctrl->H.t_dip, &Ctrl->H.m_int, &Ctrl->H.m_dec, &Ctrl->H.m_dip)) != 5) {
					GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -H option: Can't dechiper values\n");
					n_errors++;
				}
				break;
			case 'I':
				Ctrl->I.active = true;
				if (gmt_getinc(GMT, opt->arg, Ctrl->I.inc)) {
					gmt_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
	 		case 'L':
				Ctrl->L.zobs = atof (opt->arg);
				break;
			case 'M':
				if (gmt_M_compat_check(GMT, 4)) {
					GMT_Report(API, GMT_MSG_COMPAT, "Option -M is deprecated; -fg was set instead, use this in the future.\n");
					if (gmt_M_is_cartesian(GMT, GMT_IN))
						gmt_parse_common_options(GMT, "f", 'f', "g"); /* Set -fg unless already set */
				}
				else
					n_errors += gmt_default_error(GMT, opt->option);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				if (opt->arg[0] == 'n') {
					Ctrl->Q.n_pad = atoi(&opt->arg[1]);
				}
				else {
					int n = 0;
					char *pch = strchr(opt->arg, '/');
					while (pch != NULL) {
						n++;
						pch = strchr(pch+1,'/');
					}
					if (n == 0)
						Ctrl->Q.pad_dist = atof(opt->arg);	/* Pad given as a scalar distance */
					else if (n == 3)
						strncpy(Ctrl->Q.region, opt->arg, GMT_BUFSIZ);	/* Pad given as a -R region */
					else {
						GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -Q option. Either -Q<pad> or -Q<region>\n");
						n_errors++;
					}
				}
				break;
	 		case 'S':
				Ctrl->S.radius = atof(opt->arg) * 1000;
				Ctrl->S.active = true;
				break;
 			case 'T':
				break;
			case 'Z':
				if (opt->arg[0] == 't')      Ctrl->Z.top = true;
				else if (opt->arg[0] == 'b') Ctrl->Z.bot = true;
				else
					Ctrl->Z.z0 = atof(opt->arg);
				break;
			case 'b':
				Ctrl->H.bhatta = true;
				break;
			default:
				n_errors += gmt_default_error(GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition(GMT, !Ctrl->In.file[0], "Syntax error: Must specify input file\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->S.active && (Ctrl->S.radius <= 0.0 || gmt_M_is_dnan(Ctrl->S.radius)),
	                                "Syntax error: -S Radius is NaN or negative\n");
	n_errors += gmt_M_check_condition(GMT, !Ctrl->G.active && !Ctrl->F.active,
	                                "Error: Must specify either -G or -F options\n");
	n_errors += gmt_M_check_condition(GMT, !GMT->common.R.active[RSET] && Ctrl->Q.active && !Ctrl->Q.n_pad,
	                                "Error: Cannot specify -Q<pad>|<region> without -R option\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->C.rho == 0.0 && !Ctrl->H.active,
	                                "Error: Must specify either -Cdensity or -H<stuff>\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->C.active && Ctrl->H.active,
	                                "Syntax error Cannot specify both -C and -H options\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->G.active && !Ctrl->G.file,
	                                "Syntax error -G option: Must specify output file\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->H.got_maggrid && !Ctrl->H.magfile,
	                                "Syntax error -H+m option: Must specify source file\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->F.active && gmt_access(GMT, Ctrl->F.file, R_OK),
	                                "Syntax error -F: Cannot read file %s!\n", Ctrl->F.file);
	i += gmt_M_check_condition(GMT, Ctrl->G.active && Ctrl->F.active, "Warning: -F overrides -G\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdgravmag3d (void *V_API, int mode, void *args) {

	size_t  ij;
	unsigned int km = 0;		/* index of current body facet (for mag only) */
	unsigned int n_vert_max = 0;
	unsigned int nx_p, ny_p, i, j, k, ndata = 0, clockwise_type[] = {0, 5};
	bool    two_grids = false;
	int     error = 0;
	double  a, d, x_o, y_o;
	double *x_obs = NULL, *y_obs = NULL, *x_grd = NULL, *y_grd = NULL, *x_grd_geo = NULL, *y_grd_geo = NULL;
	double *cos_vec = NULL, *g = NULL, *x_grd2 = NULL, *y_grd2 = NULL, *cos_vec2 = NULL;
	double  cc_t, cs_t, s_t, wesn_new[4], wesn_padded[4];

	struct  GMT_GRID *GridA = NULL, *GridB = NULL, *GridS = NULL, *Gout = NULL;
	struct  LOC_OR *loc_or = NULL;
	struct  BODY_VERTS *body_verts = NULL;
	struct  BODY_DESC body_desc;
	struct  GRDOKB_CTRL *Ctrl = NULL;
	struct  GMT_DATASET *Cin = NULL;
	struct  GMT_DATATABLE *point = NULL;
	struct  GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct  GMT_OPTION *options = NULL;
	struct  GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	okabe_mag_var = NULL, okabe_mag_param = NULL;
	body_desc.n_v = NULL, body_desc.ind = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	GMT->common.x.n_threads = 1;        /* Default to use only one core (we may change this to max cores) */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdgravmag3d main code ---------------------*/

	if (gmt_M_is_geographic(GMT, GMT_IN)) Ctrl->box.is_geog = true;

	if (!Ctrl->box.is_geog)
		Ctrl->box.d_to_m = 1.;
	else
		Ctrl->box.d_to_m = 2.0 * M_PI * 6371008.7714 / 360.0;
		/*Ctrl->box.d_to_m = 2.0 * M_PI * gmtdefs.ellipse[N_ELLIPSOIDS-1].eq_radius / 360.0;*/

	if (Ctrl->F.active) { 		/* Read xy file where anomaly is to be computed */
		if ((Cin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IO_ASCII, NULL, Ctrl->F.file, NULL)) == NULL)
			Return (API->error);
		if (Cin->n_columns < 2) {	/* Trouble */
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: %s does not have at least 2 columns with coordinates\n",
			            Ctrl->F.file);
			Return (GMT_PARSE_ERROR);
		}
		point = Cin->table[0];	/* Can only be one table since we read a single file */
		ndata = (unsigned int)point->n_records;
		if (point->n_segments > 1) /* case not dealt (or ignored) and should be tested here */
			GMT_Report(API, GMT_MSG_VERBOSE, "Multi-segment files are not used in grdgravmag3d. Using first segment only\n");
	}

	/* ---------------------------------------------------------------------------- */

	if ((GridA = GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL,
	                           Ctrl->In.file[0], NULL)) == NULL) 	/* Get header only */
		Return(API->error);

	if (Ctrl->G.active) {
		double wesn[4], inc[2];
		/* Use the -R region for output if set; otherwise match grid domain */
		gmt_M_memcpy (wesn, (GMT->common.R.active[RSET] ? GMT->common.R.wesn : GridA->header->wesn), 4, double);
		gmt_M_memcpy (inc, (Ctrl->I.active ? Ctrl->I.inc : GridA->header->inc), 2, double);
		if (wesn[XLO] < GridA->header->wesn[XLO]) error = true;
		if (wesn[XHI] > GridA->header->wesn[XHI]) error = true;

		if (wesn[YLO] < GridA->header->wesn[YLO]) error = true;
		if (wesn[YHI] > GridA->header->wesn[YHI]) error = true;

		if (error) {
			GMT_Report(API, GMT_MSG_NORMAL, "New WESN incompatible with old.\n");
			Return(GMT_RUNTIME_ERROR);
		}

		if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, wesn, inc,
			GridA->header->registration, GMT_NOTSET, NULL)) == NULL) Return (API->error);

		GMT_Report(API, GMT_MSG_LONG_VERBOSE, "Grid dimensions are n_columns = %d, n_rows = %d\n",
		           Gout->header->n_columns, Gout->header->n_rows);
	}

	GMT_Report(API, GMT_MSG_LONG_VERBOSE, "Allocates memory and read data file\n");

	if (!GMT->common.R.active[RSET])
		gmt_M_memcpy(wesn_new, GridA->header->wesn, 4, double);
	else
		gmt_M_memcpy(wesn_new, GMT->common.R.wesn,  4, double);

	/* Process Padding request */
	if (Ctrl->Q.active && !Ctrl->Q.n_pad) {
		if (Ctrl->Q.pad_dist)		/* Convert the scalar pad distance into a -R string type */
			sprintf(Ctrl->Q.region, "%f/%f/%f/%f", wesn_new[XLO] - Ctrl->Q.pad_dist,
			        wesn_new[XHI] + Ctrl->Q.pad_dist, wesn_new[YLO] - Ctrl->Q.pad_dist,
			        wesn_new[YHI] + Ctrl->Q.pad_dist);

		GMT->common.R.active[RSET] = false;
		gmt_parse_common_options(GMT, "R", 'R', Ctrl->Q.region);	/* Use the -R parsing machinery to handle this */
		gmt_M_memcpy(wesn_padded, GMT->common.R.wesn, 4, double);
		gmt_M_memcpy(GMT->common.R.wesn, wesn_new, 4, double);		/* Reset previous WESN */
		GMT->common.R.active[RSET] = true;

		if (wesn_padded[XLO] < GridA->header->wesn[XLO]) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Request padding at the West border exceed grid limit, trimming it\n");
			wesn_padded[XLO] = GridA->header->wesn[XLO];
		}
		if (wesn_padded[XHI] > GridA->header->wesn[XHI]) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Request padding at the East border exceed grid limit, trimming it\n");
			wesn_padded[XHI] = GridA->header->wesn[XHI];
		}
		if (wesn_padded[YLO] < GridA->header->wesn[YLO]) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Request padding at the South border exceed grid limit, trimming it\n");
			wesn_padded[YLO] = GridA->header->wesn[YLO];
		}
		if (wesn_padded[YHI] > GridA->header->wesn[YHI]) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Request padding at the North border exceed grid limit, trimming it\n");
			wesn_padded[YHI] = GridA->header->wesn[YHI];
		}
	}
	else
		gmt_M_memcpy (wesn_padded, GridA->header->wesn, 4, double);

	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn_padded,
	                   Ctrl->In.file[0], GridA) == NULL) {			/* Get subset, or all */
		Return (API->error);
	}

	/* Check that Inner region request does not exceeds input grid limits */
	if (GMT->common.R.active[RSET] && Ctrl->G.active) {
		if (Gout->header->wesn[XLO] < GridA->header->wesn[XLO] ||
		    Gout->header->wesn[XHI] > GridA->header->wesn[XHI]) {
			GMT_Report (API, GMT_MSG_NORMAL, " Selected region exceeds the X-boundaries of the grid file!\n");
			return (GMT_RUNTIME_ERROR);
		}
		else if (Gout->header->wesn[YLO] < GridA->header->wesn[YLO] ||
		         Gout->header->wesn[YHI] > GridA->header->wesn[YHI]) {
			GMT_Report (API, GMT_MSG_NORMAL, " Selected region exceeds the Y-boundaries of the grid file!\n");
			return (GMT_RUNTIME_ERROR);
		}
		gmt_RI_prepare (GMT, Gout->header);	/* Ensure -R -I consistency and set n_columns, n_rows */
	}

	Ctrl->box.lon_0 = (GridA->header->wesn[XLO] + GridA->header->wesn[XHI]) / 2;
	Ctrl->box.lat_0 = (GridA->header->wesn[YLO] + GridA->header->wesn[YHI]) / 2;

	/* See if we are closing the body at the grid's top or bottom */
	if (Ctrl->Z.top) Ctrl->Z.z0 = GridA->header->z_max;
	if (Ctrl->Z.bot) Ctrl->Z.z0 = GridA->header->z_min;

	if (Ctrl->In.file[1] == NULL) { 	/* Possible swapping triang order only when we have a single  grid */
		if (Ctrl->Z.z0 >= GridA->header->z_max && !Ctrl->E.active) {	/* If plane is above max grid height and NOT constant thickness */
			/* Typical when computing the effect of whater shell for Buguer reduction of marine data */
			clockwise_type[0] = 5;		/* Means Top triangs will have a CCW description and CW for bot */
			clockwise_type[1] = 0;
		}
	}

	if (Ctrl->D.z_dir != 1) {
		Ctrl->Z.z0 *= Ctrl->D.z_dir;
		for (j = 0; j < GridA->header->size; j++)
			GridA->data[j] *= Ctrl->D.z_dir;
	}

	/* -------------- In case we have one second grid, for bottom surface -------------- */
	if (Ctrl->In.file[1]) {
		if ((GridB = GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL,
		                           Ctrl->In.file[1], NULL)) == NULL) {	/* Get header only */
			Return(API->error);
		}

		if(GridA->header->registration != GridB->header->registration) {
			GMT_Report(API, GMT_MSG_NORMAL, "Up and bottom grids have different registrations!\n");
			Return (GMT_RUNTIME_ERROR);
		}

		if (GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn_padded,
		                  Ctrl->In.file[1], GridB) == NULL) {			/* Get subset, or all */
			Return(API->error);
		}

		if (Ctrl->D.z_dir != -1) {
			for (j = 0; j < GridB->header->size; j++)
				GridB->data[j] *= Ctrl->D.z_dir;
		}
		two_grids = true;
	}

	/* -------------- In case we have  a source (magnetization) grid -------------------- */
	if (Ctrl->H.got_maggrid) {
		if ((GridS = GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL,
		                           Ctrl->H.magfile, NULL)) == NULL) {	/* Get header only */
			Return(API->error);
		}

		if(GridA->header->registration != GridS->header->registration) {
			GMT_Report(API, GMT_MSG_NORMAL, "Up surface and source grids have different registrations!\n");
			Return (GMT_RUNTIME_ERROR);
		}

		if (fabs (GridA->header->inc[GMT_X] - GridS->header->inc[GMT_X]) > 1.0e-6 ||
		          fabs(GridA->header->inc[GMT_Y] - GridS->header->inc[GMT_Y]) > 1.0e-6) {
			GMT_Report(API, GMT_MSG_NORMAL, "Up surface and source grid increments do not match!\n");
			Return(GMT_RUNTIME_ERROR);
		}

		if (GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn_padded,
		                   Ctrl->H.magfile, GridS) == NULL) {			/* Get subset, or all */
			Return(API->error);
		}
	}

	if (Ctrl->S.active && !two_grids && !Ctrl->H.pirtt && !Ctrl->E.active) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Unset -S option. It can only be used when two grids were provided OR -E.\n");
		Ctrl->S.active = false;
	}
	if (two_grids && Ctrl->E.active) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Two grids override -E option. Unsetting it.\n");
		Ctrl->E.active = false;		/* But in future we may have a second grid with variable magnetization and cte thickness */
	}

	nx_p = !Ctrl->F.active ? Gout->header->n_columns : ndata;
	ny_p = !Ctrl->F.active ? Gout->header->n_rows : ndata;
	x_grd = gmt_M_memory (GMT, NULL, GridA->header->n_columns + Ctrl->H.pirtt, double);
	y_grd = gmt_M_memory (GMT, NULL, GridA->header->n_rows + Ctrl->H.pirtt, double);
	x_obs = gmt_M_memory (GMT, NULL, nx_p, double);
	y_obs = gmt_M_memory (GMT, NULL, ny_p, double);
	if (Ctrl->F.active) g = gmt_M_memory (GMT, NULL, ndata, double);

	d = GridA->header->xy_off;		/*  0.5 : 0.0 */

	/* ----------------------- Build observation point vectors ---------------------------------- */
	if (Ctrl->G.active) {
		for (i = 0; i < Gout->header->n_columns; i++)
			x_obs[i] = (i == (Gout->header->n_columns-1)) ? Gout->header->wesn[XHI] - d * Gout->header->inc[GMT_X] :
					Gout->header->wesn[XLO] + (i + d) * Gout->header->inc[GMT_X];
		if (Ctrl->H.pirtt) {
			for (j = 0; j < Gout->header->n_rows; j++)
				y_obs[j] = (j == (Gout->header->n_rows-1)) ? (Gout->header->wesn[YLO] - d*Gout->header->inc[GMT_Y]) :
						(Gout->header->wesn[YHI] - (j + d) * Gout->header->inc[GMT_Y]);
		}
		else {
			for (j = 0; j < Gout->header->n_rows; j++)
				y_obs[j] = (j == (Gout->header->n_rows-1)) ? -(Gout->header->wesn[YLO] + d * Gout->header->inc[GMT_Y]) :
						-(Gout->header->wesn[YHI] - (j + d) * Gout->header->inc[GMT_Y]);
		}
	}
	/* ------------------------------------------------------------------------------------------ */

	if (Ctrl->H.pirtt) d -= 0.5;		/* because for prisms we want all corner coords to be in pixel registration */

	for (i = 0; i < GridA->header->n_columns; i++)
		x_grd[i] = (i == (GridA->header->n_columns-1)) ? GridA->header->wesn[XHI] + d*GridA->header->inc[GMT_X] :
				GridA->header->wesn[XLO] + (i + d) * GridA->header->inc[GMT_X];
	if (Ctrl->H.pirtt) {
		for (j = 0; j < GridA->header->n_rows; j++)
			y_grd[j] = (j == (GridA->header->n_rows-1)) ? (GridA->header->wesn[YLO] - d*GridA->header->inc[GMT_Y]) :
					(GridA->header->wesn[YHI] - (j + d) * GridA->header->inc[GMT_Y]);

		x_grd[i] = x_grd[i - 1] + GridA->header->inc[GMT_X];		/* We need this extra pt because we went from grid to pix reg */
		y_grd[j] = y_grd[j - 1] - GridA->header->inc[GMT_Y];
	}
	else {
		for (j = 0; j < GridA->header->n_rows; j++)
			y_grd[j] = (j == (GridA->header->n_rows-1)) ? -(GridA->header->wesn[YLO] + d*GridA->header->inc[GMT_Y]) :
					-(GridA->header->wesn[YHI] - (j + d) * GridA->header->inc[GMT_Y]);
	}

	if (two_grids) {
		d = GridB->header->xy_off;		/*  0.5 : 0.0 */
		if (Ctrl->H.pirtt)
			d -= 0.5;			/* because for prisms we want all corner coords to be in pixel registration */
		x_grd2 = gmt_M_memory (GMT, NULL, GridB->header->n_columns + Ctrl->H.pirtt, double);
		y_grd2 = gmt_M_memory (GMT, NULL, GridB->header->n_rows + Ctrl->H.pirtt, double);
		for (i = 0; i < GridB->header->n_columns; i++)
			x_grd2[i] = (i == (GridB->header->n_columns-1)) ? GridB->header->wesn[XHI] + d*GridB->header->inc[GMT_X] :
					GridB->header->wesn[XLO] + (i + d) * GridB->header->inc[GMT_X];
		if (Ctrl->H.pirtt) {
			for (j = 0; j < GridB->header->n_rows; j++)
				y_grd2[j] = (j == (GridB->header->n_rows-1)) ? (GridB->header->wesn[YHI] - d*GridB->header->inc[GMT_Y]) :
						(GridB->header->wesn[YHI] - (j + d) * GridB->header->inc[GMT_Y]);

			x_grd2[i] = x_grd2[i - 1] + GridB->header->inc[GMT_X];		/* We need this extra pt because we went from grid to pix reg */
			y_grd2[j] = y_grd2[j - 1] - GridB->header->inc[GMT_Y];
		}
		else {
			for (j = 0; j < GridB->header->n_rows; j++)
				y_grd2[j] = (j == (GridB->header->n_rows-1)) ? -(GridB->header->wesn[YLO] + d*GridB->header->inc[GMT_Y]) :
						-(GridB->header->wesn[YHI] - (j + d) * GridB->header->inc[GMT_Y]);
		}
	}
	/* --------------------------------------------------------------------------------------------- */

	/* ---------- Pre-compute a vector of cos(lat) to use in the Geog cases ------------------------ */
	cos_vec = gmt_M_memory (GMT, NULL, GridA->header->n_rows, double);
	for (j = 0; j < GridA->header->n_rows; j++)
		cos_vec[j] = (Ctrl->box.is_geog) ? cos(y_grd[j]*D2R): 1;
	if (two_grids) {
		cos_vec2 = gmt_M_memory (GMT, NULL, GridB->header->n_rows, double);
		for (j = 0; j < GridB->header->n_rows; j++)
			cos_vec2[j] = (Ctrl->box.is_geog) ? cos(y_grd2[j]*D2R): 1;
	}
	/* --------------------------------------------------------------------------------------------- */

	/* ------ Convert distances to arc distances at Earth surface *** NEEDS CONFIRMATION *** ------- */
	if (Ctrl->box.is_geog) {
		if (Ctrl->H.do_igrf) {          /* We need a copy in Geogs to use in IGRF */
			x_grd_geo = gmt_M_memory (GMT, NULL, GridA->header->n_columns + Ctrl->H.pirtt, double);
			y_grd_geo = gmt_M_memory (GMT, NULL, GridA->header->n_rows + Ctrl->H.pirtt, double);
			gmt_M_memcpy(x_grd_geo, x_grd, GridA->header->n_columns + Ctrl->H.pirtt, double);
			gmt_M_memcpy(y_grd_geo, y_grd, GridA->header->n_rows + Ctrl->H.pirtt, double);
		}
		for (i = 0; i < GridA->header->n_columns; i++)
			x_grd[i] = (x_grd[i] - Ctrl->box.lon_0) * Ctrl->box.d_to_m;
		for (j = 0; j < GridA->header->n_rows; j++)
			y_grd[j] = (y_grd[j] + Ctrl->box.lat_0) * Ctrl->box.d_to_m;

		if (two_grids) {
			for (i = 0; i < GridB->header->n_columns; i++)
				x_grd2[i] = (x_grd[i] - Ctrl->box.lon_0) * Ctrl->box.d_to_m;
			for (j = 0; j < GridB->header->n_rows; j++)
				y_grd2[j] = (y_grd[j] + Ctrl->box.lat_0) * Ctrl->box.d_to_m;
		}
	}
	/* --------------------------------------------------------------------------------------------- */

	if (Ctrl->Q.n_pad) {
		x_grd[0] = x_grd[1] - Ctrl->Q.n_pad * (x_grd[2] - x_grd[1]);
		y_grd[0] = y_grd[1] - Ctrl->Q.n_pad * fabs(y_grd[2] - y_grd[1]);
		x_grd[GridA->header->n_columns-1] = x_grd[GridA->header->n_columns-1] + Ctrl->Q.n_pad * (x_grd[2] - x_grd[1]);
		y_grd[GridA->header->n_rows-1] = y_grd[GridA->header->n_rows-1] + Ctrl->Q.n_pad * fabs(y_grd[2] - y_grd[1]);
		if (two_grids) {	/* Do the extension using top grid's increments so that top and bot extensions are equal */
			x_grd2[0] = x_grd2[1] - Ctrl->Q.n_pad * (x_grd[2] - x_grd[1]);		/* Why not use the x|y_inc ? */
			y_grd2[0] = y_grd2[1] - Ctrl->Q.n_pad * fabs(y_grd[2] - y_grd[1]);
			x_grd2[GridA->header->n_columns-1] += Ctrl->Q.n_pad * (x_grd[2] - x_grd[1]);
			y_grd2[GridA->header->n_rows-1] += Ctrl->Q.n_pad * fabs(y_grd[2] - y_grd[1]);
		}
	}

	if (Ctrl->F.active) { 	/* Need to compute observation coords only once */
		size_t row;
		for (row = 0; row < ndata; row++) {
			x_obs[row] = (Ctrl->box.is_geog) ? (point->segment[0]->data[GMT_X][row] - Ctrl->box.lon_0) *
				Ctrl->box.d_to_m*cos(point->segment[0]->data[GMT_Y][row] * D2R) : point->segment[0]->data[GMT_X][row];
			y_obs[row] = (Ctrl->box.is_geog) ? -(point->segment[0]->data[GMT_Y][row] - Ctrl->box.lat_0) *
				Ctrl->box.d_to_m : -point->segment[0]->data[GMT_Y][row]; /* - because y positive 'south' */
		}
	}

	if (Ctrl->H.active) { /* 1e2 is a factor to obtain nT from magnetization in A/m */
		okabe_mag_param = gmt_M_memory (GMT, NULL, 1, struct MAG_PARAM);
		okabe_mag_param[0].rim[0] = 1e2 * cos(Ctrl->H.t_dip*D2R) * cos((Ctrl->H.t_dec - 90.)*D2R);
		okabe_mag_param[0].rim[1] = 1e2 * cos(Ctrl->H.t_dip*D2R) * sin((Ctrl->H.t_dec - 90.)*D2R);
		okabe_mag_param[0].rim[2] = 1e2 * sin(Ctrl->H.t_dip*D2R);
		cc_t = cos(Ctrl->H.m_dip*D2R) * cos((Ctrl->H.m_dec - 90.)*D2R);
		cs_t = cos(Ctrl->H.m_dip*D2R) * sin((Ctrl->H.m_dec - 90.)*D2R);
		s_t  = sin(Ctrl->H.m_dip*D2R);
		/* Case of constant magnetization */
		okabe_mag_var = gmt_M_memory (GMT, NULL, 1, struct MAG_VAR);
		okabe_mag_var[0].rk[0] = Ctrl->H.m_int * cc_t;
		okabe_mag_var[0].rk[1] = Ctrl->H.m_int * cs_t;
		okabe_mag_var[0].rk[2] = Ctrl->H.m_int * s_t;
	}

	/* Decompose a paralelogram into two triangular prisms */
	if (!Ctrl->H.pirtt)
		grdgravmag3d_body_desc_tri(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[0]);		/* Set CW or CCW of top triangs */
	else
		grdgravmag3d_body_desc_prism(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[0]);	/* Allocate structs */

	/* Allocate a structure that will be used inside okabe().
		We do it here to avoid thousands of alloc/free that would result if done in okabe() */

	/* Sep 2015. Must check the validity of this because loc_or is no longer used in okabe parallel */
	n_vert_max = body_desc.n_v[0];
	for (i = 1; i < body_desc.n_f; i++)
		n_vert_max = MAX(body_desc.n_v[i], n_vert_max);

	loc_or = gmt_M_memory (GMT, NULL, (n_vert_max+1), struct LOC_OR);

	if (Ctrl->H.bhatta) {
		dircos(Ctrl->H.m_dip, Ctrl->H.m_dec, 0, &loc_or[0].x, &loc_or[0].y, &loc_or[0].z);
		dircos(Ctrl->H.t_dip, Ctrl->H.t_dec, 0, &loc_or[1].x, &loc_or[1].y, &loc_or[1].z);
	}
	else if (Ctrl->H.pirtt) {
		/* This retains several of the original FORTRAN code variable names (easy to tell) */
		double sa, si, ci, si0, sa0, ca0, cisa, cica, h1, h2, h3;

		sa = -Ctrl->H.t_dec * D2R;		/* Earth field declination */
		si =  Ctrl->H.t_dip * D2R;		/* Earth field dip (inclination) */
		ci = cos(si);					/* cos of field INC */
		si0= sin(si);					/* sin of field INC */
		sa0= sin(sa);					/* sin of field DEC */
		ca0= cos(sa);					/* cos of field DEC */
		cisa= ci * sa0;
		cica= ci * ca0;

		h1 = cisa;
		h2 = cica;
		h3 = si0;

		if (Ctrl->H.koningsberg != 0) {		/* This option is not currently implemented */
			sa = (-Ctrl->H.t_dec - Ctrl->H.m_dec) * D2R;	/* Earth field DEC - Mag DEC */
			ci = cos(Ctrl->H.m_dip * D2R);					/* cos of mag INC */
			h1 += Ctrl->H.koningsberg * ci * sin(sa);
			h2 += Ctrl->H.koningsberg * ci * cos(sa);
			h3 += Ctrl->H.koningsberg * sin(Ctrl->H.m_dip * D2R);
		}
		loc_or[0].x = cos(Ctrl->H.m_dip * D2R) * sin(-Ctrl->H.m_dec * D2R);
		loc_or[0].y = cos(Ctrl->H.m_dip * D2R) * cos(-Ctrl->H.m_dec * D2R);
		loc_or[0].z = sin(Ctrl->H.m_dip * D2R);
		loc_or[1].x = h1;		loc_or[1].y = h2;		loc_or[1].z = h3;
		loc_or[2].x = sin(-Ctrl->H.t_dec * D2R);		loc_or[2].y = cos(-Ctrl->H.t_dec * D2R);
	}

	/* --------------------------------> Now start computing <------------------------------------- */

	if (Ctrl->G.active) {               /* grid output */
		grdgravmag3d_calc_surf(GMT, Ctrl, GridA, Gout, GridS, NULL, 0, x_grd, y_grd, x_grd_geo, y_grd_geo, x_obs, y_obs, cos_vec,
		                       okabe_mag_var, loc_or, &body_desc, body_verts);

		if (Ctrl->H.pirtt) goto L1;                            /* Ugly, I know but the 2-grids case is not Bhattacharya implemented */

		if (!two_grids) {                                       /* That is, one grid and a flat base Do the BASE now */
			grdgravmag3d_body_desc_tri(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of BOT triangles */

			if (!Ctrl->E.active) {                              /* NOT constant thickness. That is, a surface and a BASE plane */
				grdgravmag3d_body_set_tri(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 0, 0,
				                          GridA->header->n_rows-1, GridA->header->n_columns-1);

				for (k = 0; k < Gout->header->n_rows; k++) {        /* Loop over input grid rows */
					y_o = (Ctrl->box.is_geog) ? (y_obs[k] + Ctrl->box.lat_0) * Ctrl->box.d_to_m : y_obs[k];	 /* +lat_0 because y was already *= -1 */

					for (i = 0; i < Gout->header->n_columns; i++) {    /* Loop over input grid cols */
						x_o = (Ctrl->box.is_geog) ? (x_obs[i] - Ctrl->box.lon_0) * Ctrl->box.d_to_m * cos(y_obs[k]*D2R) : x_obs[i];
						a = okabe(GMT, x_o, y_o, Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, body_desc, body_verts, km, 0, loc_or);
						Gout->data[gmt_M_ijp(Gout->header, k, i)] += (gmt_grdfloat)a;
					}
				}
			}
			else {      /* A Constant thickness layer */
				for (ij = 0; ij < Gout->header->size; ij++) GridA->data[ij] += (gmt_grdfloat)Ctrl->E.thickness;	/* Shift by thickness */
				grdgravmag3d_calc_surf(GMT, Ctrl, GridA, Gout, GridS, NULL, 0, x_grd, y_grd, x_grd_geo, y_grd_geo, x_obs, y_obs, cos_vec,
				                       okabe_mag_var, loc_or, &body_desc, body_verts);
				for (ij = 0; ij < Gout->header->size; ij++) GridA->data[ij] -= (gmt_grdfloat)Ctrl->E.thickness;	/* Remove because grid may be used outside GMT */
			}
		}
		else {          /* "two_grids". One at the top and the other at the base */
			grdgravmag3d_body_desc_tri(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of top triangles */
			grdgravmag3d_calc_surf(GMT, Ctrl, GridB, Gout, GridS, NULL, 0, x_grd2, y_grd2, x_grd_geo, y_grd_geo, x_obs, y_obs,
			                       cos_vec2, okabe_mag_var, loc_or, &body_desc, body_verts);
		}
	}
	else {              /* polyline output */
		grdgravmag3d_calc_surf(GMT, Ctrl, GridA, GridS, NULL, g, ndata, x_grd, y_grd, x_grd_geo, y_grd_geo, x_obs, y_obs, cos_vec,
		                       okabe_mag_var, loc_or, &body_desc, body_verts);

		if (Ctrl->H.pirtt) goto L1;     /* Ugly,I know but the 2-grids case is not Bhattacharya implemented */

		if (!two_grids) {               /* That is, one grid and a flat base. Do the BASE now */
			grdgravmag3d_body_desc_tri(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of BOT triangles */

			if (!Ctrl->E.active) {      /* NOT constant thickness. That is, a surface and a BASE plane */
				grdgravmag3d_body_set_tri(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 0, 0,
				                          GridA->header->n_rows-1, GridA->header->n_columns-1);
			}
			else {                      /* A Constant thickness layer */
				for (ij = 0; ij < Gout->header->size; ij++) GridA->data[ij] += (gmt_grdfloat)Ctrl->E.thickness;	/* Shift by thickness */
				grdgravmag3d_body_set_tri(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 0, 0, 1, 1);
				for (ij = 0; ij < Gout->header->size; ij++) GridA->data[ij] -= (gmt_grdfloat)Ctrl->E.thickness;	/* Remove because grid may be used outside GMT */
			}

			for (k = 0; k < ndata; k++)
				g[k] += okabe (GMT, x_obs[k], y_obs[k], Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, body_desc, body_verts, km, 0, loc_or);
		}
		else {                          /* "two_grids". One at the top and the other at the base */
			grdgravmag3d_body_desc_tri(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of top triangles */
			grdgravmag3d_calc_surf(GMT, Ctrl, GridB, NULL, GridS, g, ndata, x_grd2, y_grd2, x_grd_geo, y_grd_geo, x_obs, y_obs,
			                        cos_vec2, okabe_mag_var, loc_or, &body_desc, body_verts);
		}
	}
	/*---------------------------------------------------------------------------------------------*/

L1:

	if (Ctrl->G.active) {
		if (Ctrl->C.active) {
			strcpy (Gout->header->title, "Gravity field");
			strcpy (Gout->header->z_units, "mGal");
		}
		else {
			strcpy (Gout->header->title, "Magnetic field");
			strcpy (Gout->header->z_units, "nT");
		}

		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Gout)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Gout) != GMT_NOERROR)
			Return (API->error);
	}
	else {
		double out[3];
		struct GMT_RECORD *Out = gmt_new_record (GMT, out, NULL);
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) 	/* Establishes data output */
			Return (API->error);

		if ((error = GMT_Set_Columns (API, GMT_OUT, 3, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
			Return (API->error);

		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) 	/* Enables data output and sets access mode */
			Return (API->error);
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR)	/* Sets output geometry */
			Return (API->error);

		for (k = 0; k < ndata; k++) {
			out[GMT_X] = point->segment[0]->data[GMT_X][k];
			out[GMT_Y] = point->segment[0]->data[GMT_Y][k];
			out[GMT_Z] = g[k];
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
		}
		gmt_M_free (GMT, Out);

		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) 	/* Disables further data input */
			Return (API->error);
	}

	gmt_M_free (GMT, x_grd);
	gmt_M_free (GMT, y_grd);
	gmt_M_free (GMT, x_grd2);
	gmt_M_free (GMT, y_grd2);
	gmt_M_free (GMT, cos_vec2);
	gmt_M_free (GMT, g);
	gmt_M_free (GMT, x_obs);
	gmt_M_free (GMT, y_obs);
	gmt_M_free (GMT, cos_vec);
	gmt_M_free (GMT, loc_or);
	gmt_M_free (GMT, body_desc.n_v);
	gmt_M_free (GMT, body_desc.ind);
	gmt_M_free (GMT, body_verts);
	gmt_M_free (GMT, okabe_mag_param);
	gmt_M_free (GMT, okabe_mag_var);
	if (Ctrl->H.do_igrf) {
		gmt_M_free (GMT, x_grd_geo);		gmt_M_free (GMT, y_grd_geo);
	}

	Return (GMT_NOERROR);
}


/* -----------------------------------------------------------------*/
GMT_LOCAL int grdgravmag3d_body_desc_tri(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc,
                                         struct BODY_VERTS **body_verts, unsigned int face) {
	gmt_M_unused(Ctrl);
/*
		__________________________________________
		|                                        |
		|               _________________        |
		|             /|         X (Nord)        |
		|            / |                         |
		|           /  |     0 ________ 1        |
		|          /   |      /       /|         |
		|       Y /    |     /       / |         |
		|       (Est)  |  3 /_______/2 |         |
		|              |    |       |  |         |
		|              |    |       | / 5        |
		|            Z |    |       |/           |
		|              V    |_______/            |
		|                  7         6           |
		|________________________________________|

*/
	if (face == 0) {			/* Decompose the TOP square surface in 2 triangles using CW order */
		body_desc->n_f = 2;
		if (body_desc->n_v == NULL)
			body_desc->n_v = gmt_M_memory (GMT, NULL, body_desc->n_f, unsigned int);
		body_desc->n_v[0] = body_desc->n_v[1] = 3;
		if (body_desc->ind == NULL)
			body_desc->ind = gmt_M_memory (GMT, NULL, body_desc->n_v[0] + body_desc->n_v[1], unsigned int);
		body_desc->ind[0] = 0;	body_desc->ind[1] = 1; 	body_desc->ind[2] = 2;	/* 1st top triang (0 1 3)*/
		body_desc->ind[3] = 0;	body_desc->ind[4] = 2; 	body_desc->ind[5] = 3;	/* 2nd top triang (1 2 3) */
		if (*body_verts == NULL) *body_verts = gmt_M_memory (GMT, NULL, 4, struct BODY_VERTS);
	}
	else if (face == 5) {			/* Decompose the BOT square surface in 2 triangles using CCW order */
		body_desc->n_f = 2;
		if (body_desc->n_v == NULL)
			body_desc->n_v = gmt_M_memory (GMT, NULL, body_desc->n_f, unsigned int);
		body_desc->n_v[0] = body_desc->n_v[1] = 3;
		if (body_desc->ind == NULL)
			body_desc->ind = gmt_M_memory (GMT, NULL, body_desc->n_v[0] + body_desc->n_v[1], unsigned int);
		body_desc->ind[0] = 0;	body_desc->ind[1] = 2; 	body_desc->ind[2] = 1;	/* 1st bot triang */
		body_desc->ind[3] = 0;	body_desc->ind[4] = 3; 	body_desc->ind[5] = 2;	/* 2nd bot triang */
		if (*body_verts == NULL) *body_verts = gmt_M_memory (GMT, NULL, 4, struct BODY_VERTS);
	}
	/* Other face cases will go here */
	return 0;
}

/* -----------------------------------------------------------------------------------*/
GMT_LOCAL int grdgravmag3d_body_desc_prism(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc,
                                           struct BODY_VERTS **body_verts, unsigned int face) {
	gmt_M_unused(Ctrl);
	if (face != 0 && face != 5) return(0);

	body_desc->n_f = 1;
	if (body_desc->n_v == NULL)		/* First time this function is called */
		body_desc->n_v = gmt_M_memory (GMT, NULL, body_desc->n_f, unsigned int);
	body_desc->n_v[0] = 2;
	if (body_desc->ind == NULL)
		body_desc->ind = gmt_M_memory (GMT, NULL, body_desc->n_v[0], unsigned int);
	if (*body_verts == NULL) *body_verts = gmt_M_memory (GMT, NULL, 2, struct BODY_VERTS);

	body_desc->ind[0] = 0;	body_desc->ind[1] = 1;	/* NOT USED REALLY AREN'T THEY? */

	return(0);
}

/* -----------------------------------------------------------------------------------*/
GMT_LOCAL int grdgravmag3d_body_set_tri(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
		struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
		double *cos_vec, unsigned int j, unsigned int i, unsigned int inc_j, unsigned int inc_i) {

	/* Allocate and fill the body_desc structure with the description on how to
	   connect the vertex of the polygonal planar surface */

	unsigned int i1, j1, ij;
	bool is_geog = Ctrl->box.is_geog;
	gmt_grdfloat *z = Grid->data;
	double cosj, cosj1;
	struct GMT_GRID_HEADER *h = Grid->header;
	gmt_M_unused(GMT);
	gmt_M_unused(body_desc);

	j1 = j + inc_j;         i1 = i + inc_i;
	cosj = cos_vec[j];      cosj1 = cos_vec[j1];

	body_verts[0].x = (is_geog) ? x[i]  * cosj  : x[i];
	body_verts[1].x = (is_geog) ? x[i1] * cosj  : x[i1];
	body_verts[2].x = (is_geog) ? x[i1] * cosj1 : x[i1];
	body_verts[3].x = (is_geog) ? x[i]  * cosj1 : x[i];
	body_verts[0].y = y[j];
	body_verts[1].y = body_verts[0].y;
	body_verts[2].y = y[j1];
	body_verts[3].y = body_verts[2].y;
	if (inc_i == 1) {
		ij = gmt_M_ijp(h,j,i);
		body_verts[0].z = z[ij];
		body_verts[1].z = z[ij + 1];  /* z[gmt_M_ijp(h,j,i1)];  */
		ij = gmt_M_ijp(h,j1,i1);
		body_verts[2].z = z[ij];      /* z[gmt_M_ijp(h,j1,i1)]; */
		body_verts[3].z = z[ij - 1];  /* z[gmt_M_ijp(h,j1,i)];  */
	}
	else {		/* Base surface */
		body_verts[0].z = body_verts[1].z = body_verts[2].z = body_verts[3].z = Ctrl->Z.z0;
		if (Ctrl->E.active) {		/* Constant thickness */
			body_verts[0].z -= Ctrl->E.thickness;		body_verts[1].z -= Ctrl->E.thickness;
			body_verts[2].z -= Ctrl->E.thickness;		body_verts[3].z -= Ctrl->E.thickness;
		}
		else {
			body_verts[0].z = body_verts[1].z = body_verts[2].z = body_verts[3].z = Ctrl->Z.z0;
		}
	}

	return(0);
}

/* -----------------------------------------------------------------------------------*/
GMT_LOCAL int grdgravmag3d_body_set_prism(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
		struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
		double *cos_vec, unsigned int j, unsigned int i, unsigned int inc_j, unsigned int inc_i) {

	/* Allocate and fill the body_desc structure with the description on how to
	   connect the vertex of the rectangular planar surface */

	unsigned int i1, j1;
	bool is_geog = Ctrl->box.is_geog;
	gmt_grdfloat *z = Grid->data;
	double cosj, cosj1;
	struct GMT_GRID_HEADER *h = Grid->header;
	gmt_M_unused(GMT);
	gmt_M_unused(body_desc);

	j1 = j + inc_j;         i1 = i + inc_i;
	cosj = cos_vec[j];      cosj1 = cos_vec[j1];

	body_verts[0].x = (is_geog) ? x[i]  * cosj  : x[i];
	body_verts[1].x = (is_geog) ? x[i1] * cosj1 : x[i1];

	body_verts[0].y = y[j1];	/* Reverse order because y[] starts from y_max and decreases */
	body_verts[1].y = y[j];

	body_verts[0].z = z[gmt_M_ijp(h,j,i)];
	body_verts[1].z = (Ctrl->E.active) ? z[gmt_M_ijp(h,j,i)] + Ctrl->E.thickness : Ctrl->Z.z0;
	return(0);
}


/* -----------------------------------------------------------------------------------*/
#ifdef HAVE_GLIB_GTHREAD
GMT_LOCAL void *thread_function (void *args) {
	grdgravmag3d_calc_surf_ ((struct THREAD_STRUCT *)args);
	return NULL;
}
#endif

GMT_LOCAL void grdgravmag3d_calc_surf_ (struct THREAD_STRUCT *t) {
	/* Do the actual work. This function is called in either threaded and non-threaded cases. */

	char   tabs[16] = {""}, frmt[64] = {""};
	unsigned int row, col, k, i, km = 0, pm = 0, indf;
	double rho_or_mag, x_o, y_o, tmp = 1, a, DX, DY = 0.0, s_rad2;
	double out_igrf[7], *igrf_dip = NULL, *igrf_dec = NULL;		/* Row vectors for the case where we will need to compute IGRF params */
	struct BODY_VERTS *body_verts = NULL;

    struct GMT_CTRL *GMT        = t->GMT;
    struct GRDOKB_CTRL *Ctrl    = t->Ctrl;
    struct GMT_GRID *Grid       = t->Grid;
    struct GMT_GRID *Gout       = t->Gout;
    struct GMT_GRID *Gsource    = t->Gsource;
    struct BODY_DESC *body_desc = t->body_desc;
    struct LOC_OR *loc_or       = t->loc_or;
    double *x_grd               = t->x_grd;
    double *y_grd               = t->y_grd;
    double *x_obs               = t->x_obs;
    double *y_obs               = t->y_obs;
   	double *g                   = t->g;
    double *cos_vec             = t->cos_vec;
    unsigned int n_pts          = t->n_pts;
    unsigned int r_start        = t->r_start;
    unsigned int r_stop         = t->r_stop;

	int (*v_func[3])(struct GMT_CTRL *, struct GRDOKB_CTRL *, struct GMT_GRID *, struct BODY_DESC *, struct BODY_VERTS *,
	      double *, double *, double *, unsigned int, unsigned int, unsigned int, unsigned int);
	double (*d_func[3])(struct GMT_CTRL *, double, double, double, double, bool, struct BODY_DESC, struct BODY_VERTS *,
	        unsigned int, unsigned int, struct LOC_OR *);

	/* IDEALY THIS SHOULD BE A MUTEX. BUT FIRST: HOW? AND SECOND, WOULDN'T IT BREAK THE WHOLE TREADING MECHANICS? */
	if (body_verts == NULL)
		body_verts = gmt_M_memory (GMT, NULL, 4, struct BODY_VERTS);		/* 4 is good enough for Okabe (tri = 4) and Mprism (prism = 2) cases */

	v_func[0] = grdgravmag3d_body_set_tri;
	v_func[1] = grdgravmag3d_body_set_prism;
	v_func[2] = grdgravmag3d_body_set_prism;
	d_func[0] = okabe;
	d_func[1] = mprism;
	d_func[2] = bhatta;

	indf = (Ctrl->H.pirtt) ? 1 + Ctrl->H.bhatta : 0;
	rho_or_mag = (Ctrl->C.active) ? Ctrl->C.rho : Ctrl->H.m_int;	/* What are we computing? (But it may be overridden below) */

	/* For Bhattacharya, select which component */
	pm = (Ctrl->H.f_tot) ? 0 : ((Ctrl->H.x_comp) ? 1 : ((Ctrl->H.y_comp) ? 2 : ((Ctrl->H.z_comp) ? 3 : ((Ctrl->H.h_comp) ? 4 : 0)) ));
	if (Ctrl->H.bhatta && pm > 0) pm = pm - 1;		/* TEMP TEMP TEMP. Will crash if h_comp */

	s_rad2 = Ctrl->S.radius * Ctrl->S.radius;

	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		for (i = 0; i < MIN((unsigned)(t->thread_num + 1), 5); i++) {
			tabs[i] = '\t';
		}
		sprintf(frmt, "Thread %%d%s Row = %%d\t of = %%.3d\r", tabs);
	}

	if (Ctrl->H.do_igrf) {
		igrf_dip = gmt_M_memory (GMT, NULL, (size_t)(Grid->header->n_columns + 1), double);
		igrf_dec = gmt_M_memory (GMT, NULL, (size_t)(Grid->header->n_columns + 1), double);
	}

	for (row = r_start; row < r_stop; row++) {                     /* Loop over input grid rows */

		if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE))
			GMT_Message(GMT->parent, GMT_TIME_NONE, frmt, t->thread_num + 1, row + 1, r_stop);

		if (Ctrl->H.do_igrf) {                                     /* Compute a row of IGRF dec & dip */
			for (col = 0; col < Grid->header->n_columns - 1 + MIN(indf,1); col++) {
				MGD77_igrf10syn(GMT, 0, 2000, 1, 0, t->x_grd_geo[col], t->y_grd_geo[row], out_igrf);
				igrf_dec[col] = out_igrf[5] * D2R;
				igrf_dip[col] = out_igrf[6] * D2R;
			}
		}

		for (col = 0; col < Grid->header->n_columns - 1 + MIN(indf,1); col++) {   /* Loop over input grid cols */

			v_func[indf](GMT, Ctrl, Grid, body_desc, body_verts, x_grd, y_grd, cos_vec, row, col, 1, 1);	/* Call the body_set function */
			if (Ctrl->H.pirtt && fabs(body_verts[1].z - body_verts[0].z) < 1e-2) continue;

			if (Gsource)                                            /* If we have a variable source grid */
				rho_or_mag = Gsource->data[gmt_M_ijp(Gsource->header, row, col)];

			if (Ctrl->H.do_igrf) {                                  /* Here we have a constantly varying dec and dip */
				loc_or[0].x = cos(igrf_dip[col]) * sin(-igrf_dec[col]);
				loc_or[0].y = cos(igrf_dip[col]) * cos(-igrf_dec[col]);
				loc_or[0].z = sin(igrf_dip[col]);
				/* This is the first attempt. Probably need to be changed if we allow grids of dec/dip and anomaly along normal field */
				loc_or[1].x = loc_or[0].x;		loc_or[1].y = loc_or[0].y;		loc_or[1].z = loc_or[0].z;
				if (pm == 4) {
					loc_or[2].x = sin(-igrf_dec[col]);	loc_or[2].y = cos(-igrf_dec[col]);
				}
			}

			if (Ctrl->G.active) {
				for (k = 0; k < Gout->header->n_rows; k++) {            /* Loop over output grid rows */
					y_o = (Ctrl->box.is_geog) ? (y_obs[k] + Ctrl->box.lat_0) * Ctrl->box.d_to_m : y_obs[k]; /* +lat_0 because y was already *= -1 */
					if (Ctrl->S.active) {
						DY = body_verts[0].y - y_o;
						if (fabs(DY) > Ctrl->S.radius) continue;    /* If outside the circumscribed square no need to continue */
						DY *= DY;                                   /* Square it right away */
					}

					if (Ctrl->box.is_geog) tmp = Ctrl->box.d_to_m * cos(y_obs[k]*D2R);
					for (i = 0; i < Gout->header->n_columns; i++) {        /* Loop over output grid cols */
						x_o = (Ctrl->box.is_geog) ? (x_obs[i] - Ctrl->box.lon_0) * tmp : x_obs[i];
						if (Ctrl->S.active) {
							DX = body_verts[0].x - x_o;             /* Use the first vertex to estimate distance. Approximate but good enough */
							if ((DX*DX + DY) > s_rad2) continue;    /* Remember that DY was already squared above */
						}
						a = d_func[indf](GMT, x_o, y_o, Ctrl->L.zobs, rho_or_mag, Ctrl->C.active, *body_desc, body_verts, km, pm, loc_or);
						Gout->data[gmt_M_ijp(Gout->header, k, i)] += (gmt_grdfloat)a;
					}
				}
			}
			else {                                                  /* Compute on a polyline only */
				for (k = 0; k < n_pts; k++)
					g[k] += d_func[indf](GMT, x_obs[k], y_obs[k], Ctrl->L.zobs, rho_or_mag, Ctrl->C.active,
					                     *body_desc, body_verts, km, pm, loc_or);
			}
		}
	}

	gmt_M_free (GMT, body_verts);
	if (Ctrl->H.do_igrf) {
		gmt_M_free (GMT, igrf_dip);		gmt_M_free (GMT, igrf_dec);
	}
}

/* -----------------------------------------------------------------------------------*/
GMT_LOCAL void grdgravmag3d_calc_surf (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid, struct GMT_GRID *Gout,
		struct GMT_GRID *Gsource, double *g, unsigned int n_pts, double *x_grd, double *y_grd, double *x_grd_geo, double *y_grd_geo,
		double *x_obs, double *y_obs, double *cos_vec, struct MAG_VAR *okabe_mag_var, struct LOC_OR *loc_or,
		struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts) {

	/* Send g = NULL for grid computations (e.g. -G) or Gout = NULL otherwise (-F).
	   In case of polyline output (-F) n_pts is the number of output locations (irrelevant otherwise) */

	int i, indf;
	struct THREAD_STRUCT *threadArg = NULL;
#ifdef HAVE_GLIB_GTHREAD
	GThread **threads = NULL;
	if (GMT->common.x.n_threads > 1)
		threads = gmt_M_memory (GMT, NULL, GMT->common.x.n_threads, GThread *);
#endif

	gmt_M_tic(GMT);

	indf = (Ctrl->H.pirtt) ? 1 : 0;

	threadArg = gmt_M_memory (GMT, NULL, GMT->common.x.n_threads, struct THREAD_STRUCT);

	for (i = 0; i < GMT->common.x.n_threads; i++) {
		threadArg[i].GMT        = GMT;
		threadArg[i].Ctrl       = Ctrl;
		threadArg[i].Grid       = Grid;
		threadArg[i].Gout       = Gout;
		threadArg[i].Gsource    = Gsource;
   		threadArg[i].body_verts = body_verts;
   		threadArg[i].body_desc  = body_desc;
   		threadArg[i].okabe_mag_var    = okabe_mag_var;
   		threadArg[i].loc_or     = loc_or;
   		threadArg[i].x_grd      = x_grd;
   		threadArg[i].x_grd_geo  = x_grd_geo;
   		threadArg[i].y_grd      = y_grd;
   		threadArg[i].y_grd_geo  = y_grd_geo;
   		threadArg[i].x_obs      = x_obs;
   		threadArg[i].y_obs      = y_obs;
   		threadArg[i].g          = g;
   		threadArg[i].cos_vec    = cos_vec;
   		threadArg[i].n_pts      = n_pts;
   		threadArg[i].r_start    = i * irint((Grid->header->n_rows - 1 - indf) / GMT->common.x.n_threads);
   		threadArg[i].thread_num = i;

		if (GMT->common.x.n_threads == 1) {		/* Independently of WITH_THREADS, if only one don't call the threading machine */
   			threadArg[i].r_stop = Grid->header->n_rows - 1 + indf;
			grdgravmag3d_calc_surf_ (&threadArg[0]);
			break;		/* Make sure we don't go through the threads lines below */
		}
#ifndef HAVE_GLIB_GTHREAD
	}
#else
   		threadArg[i].r_stop = (i + 1) * irint((Grid->header->n_rows - 1 - indf) / GMT->common.x.n_threads);
   		if (i == GMT->common.x.n_threads - 1) threadArg[i].r_stop = Grid->header->n_rows - 1 + indf;	/* Make sure last row is not left behind */
		threads[i] = g_thread_new(NULL, thread_function, (void*)&(threadArg[i]));
	}

	if (GMT->common.x.n_threads > 1) {		/* Otherwise g_thread_new was never called and so no need to "join" */
		for (i = 0; i < GMT->common.x.n_threads; i++)
			g_thread_join(threads[i]);

		gmt_M_free (GMT, threads);
	}
#endif

	gmt_M_free (GMT, threadArg);

	gmt_M_toc(GMT,"");
}

/* -----------------------------------------------------------------------------------*/
/* This is a modified version of the FORTRAN code that allows also a prism inclination
   and declination and Remanant + Inducec magnetization via a Koninsberg ratio. Those have
   not been ported into this C translation. Second analysis, the FORTRAN code did not actually
   consider different dec/dip for magnetization vector and Earth field, but here we do.
*/

GMT_LOCAL double mprism (struct GMT_CTRL *GMT, double x_o, double y_o, double z_o, double mag, bool is_grav,
		struct BODY_DESC bd_desc, struct BODY_VERTS *body_verts, unsigned int km, unsigned int i_comp, struct LOC_OR *mag_par) {

	/* The MAG_PAR struct is used here to transmit the Ctrl->H members (components actually) */

	int i, j, k, ijk;
	double a[3][2], eps1, eps2, hx, hy, hz, tr, sc, xc, yc;
	double xn, yn, f11, f12, f13, f21, f22, f23, u, v, w, r, c4, c5, c6;
	gmt_M_unused(GMT);
	gmt_M_unused(z_o);
	gmt_M_unused(is_grav);
	gmt_M_unused(bd_desc);
	gmt_M_unused(km);

	eps1 = 1.0e-12;
	eps2 = 5.0e-3;

	a[0][0] = (body_verts[1].y - body_verts[0].y) / 2;		/* thickness (arbitrary, could also be the 'length') */
	a[0][1] = -a[0][0];
	a[1][0] = (body_verts[1].x - body_verts[0].x) / 2;		/* length */
	a[1][1] = -a[1][0];
	a[2][0] =  body_verts[1].z;
	a[2][1] =  body_verts[0].z;

	xc = (body_verts[0].x + body_verts[1].x) * 0.5;
	yc = (body_verts[0].y + body_verts[1].y) * 0.5;

	xn = xc - x_o;
	yn = y_o - yc;
	f11 = 0;	f12 = 0;	f13 = 0;
	f21 = 1;	f22 = 1;	f23 = 1;
	for (i = 0; i < 2; i++) {
		u = xn - a[0][i];
		for (j = 0; j < 2; j++) {
			v = yn - a[1][j];
			for (k = 0; k < 2; k++) {
				w = -a[2][k];
				if (fabs(u) < eps2) u = eps2;
				if (fabs(v) < eps2) v = eps2;
				if (fabs(w) < eps2) w = eps2;
				ijk = i + j + k;
				sc= 1;
				r = sqrt(u*u + v*v + w*w);
				r += eps1;
				c4 = u + r;
				c5 = v + r;
				c6 = w + r;
				tr = eps1 * r;
				if (c4 < tr) c4 = tr;
				if (c5 < tr) c5 = tr;
				if (c6 < tr) c6 = tr;
				if (ijk ==  0 || ijk == 2) {
					f21 *= c4;
					f22 *= c5;
					f23 *= c6;
				}
				else {
					sc  = -sc;
					f21 /= c4;
					f22 /= c5;
					f23 /= c6;
				}
				if (i_comp == 0 || i_comp == 4) {
					f11 += sc * atan(v * w / (u * r));
					f12 += sc * atan((- w * u) / (v * r));
					f13 += sc * atan(v * u / (w * r));
				}
				else if (i_comp == 1)
					f11 += sc * atan(v * w / (u * r));
				else if (i_comp == 2)
					f12 += sc * atan((- w * u) / (v * r));
				else
					f13 += sc * atan(v * u / (w * r));
			}
		}
	}

	if (i_comp == 0) {				/* Total anomaly */
		f21 = log(f21);		f22 = log(f22);		f23 = log(f23);
		hx  = (-mag_par[0].x * f11 + mag_par[0].y * f23 + mag_par[0].z * f22) * mag;
		hy  = ( mag_par[0].x * f23 + mag_par[0].y * f12 + mag_par[0].z * f21) * mag;
		hz  = ( mag_par[0].x * f22 + mag_par[0].y * f21 - mag_par[0].z * f13) * mag;
		return(-(hx * mag_par[1].x + hy * mag_par[1].y + hz * mag_par[1].z) * 100);
	}
	else if (i_comp == 1) {			/* E/W-component anomaly */
		f22 = log(f22);		f23 = log(f23);
		hx  = (-mag_par[0].x * f11 + mag_par[0].y * f23 + mag_par[0].z * f22) * mag;
		return(-hx * 100);
	}
	else if (i_comp == 2) {			/* N/S-component anomaly */
		f21 = log(f21);		f23 = log(f23);
		hy  = (mag_par[0].x * f23 + mag_par[0].y * f12 + mag_par[0].z * f21) * mag;
		return(-hy * 100);
	}
	else if (i_comp == 3) {			/* Z-component anomaly */
		f21 = log(f21);		f22 = log(f22);
		hz  = (mag_par[0].x * f22 + mag_par[0].y * f21 - mag_par[0].z * f13) * mag;
		return(-hz * 100);
	}
	else {							/* Horizontal anomaly */
		f21 = log(f21);		f22 = log(f22);		f23 = log(f23);
		hx  = (-mag_par[0].x * f11 + mag_par[0].y * f23 + mag_par[0].z * f22) * mag;
		hy  = ( mag_par[0].x * f23 + mag_par[0].y * f12 + mag_par[0].z * f21) * mag;
		return(-(hx * mag_par[2].x + hy * mag_par[2].y) * 100);
	}
}

/*

Original FORTRAN code from

https://wiki.oulu.fi/display/~mpi/Magnetic+field+of+a+prism+model

!-------------------------------------------------------------------------------------------
! This subroutine computes the magnetic field components of a dipping thick magnetized prism.
!
!  Created by S.-E. Hjelt, 15.8.1972
!  Modified by M. Pirttij?rvi 1997-2002
!
! Input parameters:
!
!   pri...prism parameters (vector,dim=8):
!
!      1...thickness,  b1-a1 (meters)
!      2...length,  b2-a2 (meters)
!      3...depth,  b3-a3 (meters)
!      4...dip angle (fii/degrees)
!      5...x-coordinate of the center of the top of the prism (meters)
!      6...y-coordinate of the center of the top of the prism (meters)
!      7...z-coordinate (depth) of the center of the top of the prism (meters)
!      8...strike direction of the prism (azi/degrees)
!
!   pmg...magnetic parameters (vector,dim=7):
!
!      1...susceptibility of the prism (khi/SI-units)
!      2...the amplitude of earth's magnetic field (t0/nanoTeslas)
!      3...inclination of earth's magnetic field (i0/degrees)
!      4...declination of earth's magnetic field (d0/degrees)
!         -> strike angle is taken clockwise from magnetic north
!      5...K?ningsbergs ratio (Q)
!      6...inclination of the prism magnetization (ir/degrees)
!      7...azimuth of the prism magnetization (ar/degrees)
!
!   xx....x-coordinate of the field points (m) (vector,dim=np)
!   yy....y-coordinate of the field points (m) (vector,dim=np)
!
! Output parameters:
!
!   zz.....magnetic field components (table,dim=5,np)
!
!      1...total anomaly (t/nanoTeslas)
!      2...z-component, vertical (t/nanoTeslas)
!      3...x-component, north (t/nanoTeslas)
!      4...y-component, east (t/nanoTeslas)
!      5...horizontal anomaly (t/nanoTeslas)
!      6...total field (t/nanoTeslas)
!
! Notes:
!   The strike and the azimuth of prism magnetization are taken
!   clockwise from magnetic north (and the field declination, of course)
!   Dip angle is taken from horizontal plate
!   K?ningsberg's ratio is the ratio between the remanent and induced
!   magnetization (Mr/Mi)
!   Demagnetization coefficients are computed outside this subroutine
!
!----------------------------------------------------------------------

  subroutine mprism(pri,pmg,dmc,xx,yy,zz,np)

    implicit none
    integer :: np
    real, dimension(8) :: pri
    real, dimension(7) :: pmg
    real, dimension(3) :: dmc
    real, dimension(np) :: xx,yy
    real, dimension(6,np) :: zz
    integer :: i,j,k,l,n,ijk
    real, dimension(3,2) :: a
    real :: pii,piip,ca1,sa1,pmg4,sa,si,ci,sa0,ca0,cisa,cica,si0
    real :: cirsa,circa,sir,sf,cf,ctf,dx,si1,si2,si3,h1,h2,h3,zn,xn,yn
    real :: f11,f12,f13,f21,f22,f23,u,v,w,sc,p,q,r,c4,c5,c6,tr,hx,hy,hz
    real :: eps= 1.e-5, eps2= 5.e-3

! set up computation

    pii= 4.*atan(1.)
    piip= pii/180.
    ca1= cos(pri(8)*piip)
    sa1= sin(pri(8)*piip)
! correct so that x-axis is towards east
!      pmg4= (pmg(4)-pri(8))*piip
    pmg4= (90.-pmg(4)-pri(8))*piip
    do j= 1,2
      a(j,1)= pri(j)/2.
      a(j,2)= -pri(j)/2.
    end do
    a(3,1)= pri(7)+pri(3)
    a(3,2)= pri(7)

    sa= pmg4
    si= pmg(3)*piip
    ci= cos(si)
    sa0= sin(sa)
    ca0= cos(sa)
    cisa= ci*sa0
    cica= ci*ca0
    si0= sin(si)
    sa= pmg4-pmg(7)*piip
    si= pmg(6)*piip
    ci= cos(si)
    cirsa= ci*sin(sa)
    circa= ci*cos(sa)
    sir= sin(si)

! correct because x-axis is towards east
!    sf= pri(4)*piip
    sf= (180.-pri(4))*piip
    cf= cos(sf)
    sf= sin(sf)
    ctf= cf/sf
    dx= pri(3)*ctf

    si= pmg(2)*pmg(1)/(4.*pii)
    si1= si*(1./(1.+dmc(2)*pmg(1)))
    si2= si*(1./(1.+dmc(1)*pmg(1)))
    si3= si*(1./(1.+dmc(3)*pmg(1)))

    sa= pmg(5)
    h1= si1*(cisa+sa*cirsa)
    h2= si2*(cica+sa*circa)
    h3= si3*(si0+sa*sir)
    h1= h1*sf-h3*cf
    zn= 0.

! loop for each point: coordinate transform

    do l=1,np
      xn= (yy(l)-pri(6))*ca1-(xx(l)-pri(5))*sa1
      yn= (yy(l)-pri(6))*sa1+(xx(l)-pri(5))*ca1
      f11= 0.
      f12= 0.
      f13= 0.
      f21= 1.
      f22= 1.
      f23= 1.
      do i= 1,2
        do j= 1,2
          do k= 1,2
            u= xn-a(1,i)
            v= yn-a(2,j)
            w= zn-a(3,k)
            if (abs(u) < eps2) u= eps2
            if (abs(v) < eps2) v= eps2
            if (abs(w) < eps2) w= eps2
            if (k ==  1) u= u-dx
            ijk= i+j+k
            sc= 1.
            p= u*cf+w*sf
            q= u*sf-w*cf
            r= sqrt(u*u+v*v+w*w)
            if (r < eps) r= eps
            c4= u+r
            c5= v+r
            c6= p+r
            tr= eps*r
            if (c4 < tr) c4= tr
            if (c5 < tr) c5= tr
            if (c6 < tr) c6= tr
            if (ijk ==  3 .or. ijk ==  5) then
              f21= f21*c4
              f22= f22*c5
              f23= f23*c6
            else
              sc= -sc
              f21= f21/c4
              f22= f22/c5
              f23= f23/c6
            end if
            f11= f11+sc*atan(v*p/(q*r+eps))
            f12= f12+sc*atan((v*v*cf-w*q)/(v*sf*r+eps))
            f13= f13+sc*atan(v*u/(w*r+eps))
          end do
        end do
      end do

      f21= alog(f21)
      f22= alog(f22)
      f23= alog(f23)
      hx= h1*(cf*f22-sf*f11)+h2*sf*f23+h3*f22
      hy= h1*f23+h2*f12+h3*f21
      hz= h1*(cf*f11+sf*f22)+h2*(f21-cf*f23)-h3*f13

      zz(3,l)= -hx
      zz(4,l)= -hy
      zz(2,l)= -hz
      zz(1,l)= -(hx*cisa+hy*cica+hz*si0)
      zz(5,l)= -(hx*sa0+hy*ca0)
      zz(6,l)= pmg(2)+zz(1,l)

    end do

    return
  end subroutine mprism

*/

GMT_LOCAL double bhatta (struct GMT_CTRL *GMT, double x_o, double y_o, double z_o, double mag, bool is_grav,
		struct BODY_DESC bd_desc, struct BODY_VERTS *body_verts, unsigned int km, unsigned int i_comp, struct LOC_OR *loc_or) {

	/* x_o, y_o, z_o are the coordinates of the observation point
 	 * mag is the body magnetization in A/m
 	 * km is not used here.
 	 * i_comp: index to which component function to compute: 0 -> EW comp; 1 -> NS comp; 2 -> Z component
 	 */

	double x111, x011, x101, x001, x110, x010, x100, x000, u0, u1, v0, v1, w0, w1, tx;
	double (*d_func[3])(double, double, double, double, double, double);       /* function pointer */
	gmt_M_unused(GMT);
	gmt_M_unused(is_grav);
	gmt_M_unused(bd_desc);
	gmt_M_unused(km);

	d_func[0] = nucleoy;
	d_func[1] = nucleox;
	d_func[2] = nucleoz;

	w0 = body_verts[0].z - z_o;
	w1 = body_verts[1].z - z_o;
	if (fabs(w0 - w1) < 1e-2) return(0);

	v0 = body_verts[0].x - x_o;
	v1 = body_verts[1].x - x_o;

	u0 = body_verts[0].y - y_o;
	u1 = body_verts[1].y - y_o;

	x111 = d_func[i_comp](u1, v1, w1, loc_or[0].x, loc_or[0].y, loc_or[0].z);
	x011 = d_func[i_comp](u0, v1, w1, loc_or[0].x, loc_or[0].y, loc_or[0].z);
	x101 = d_func[i_comp](u1, v0, w1, loc_or[0].x, loc_or[0].y, loc_or[0].z);
	x001 = d_func[i_comp](u0, v0, w1, loc_or[0].x, loc_or[0].y, loc_or[0].z);
	x110 = d_func[i_comp](u1, v1, w0, loc_or[0].x, loc_or[0].y, loc_or[0].z);
	x010 = d_func[i_comp](u0, v1, w0, loc_or[0].x, loc_or[0].y, loc_or[0].z);
	x100 = d_func[i_comp](u1, v0, w0, loc_or[0].x, loc_or[0].y, loc_or[0].z);
	x000 = d_func[i_comp](u0, v0, w0, loc_or[0].x, loc_or[0].y, loc_or[0].z);
	tx = mag * (x111 - x011 - x101 + x001 - x110 + x010 + x100 - x000) * 100;
	return (tx);
}

GMT_LOCAL double nucleox(double u, double v, double w, double rl, double rm, double rn) {
	double r, t1, t2, t3, rnum, rden;
	r = sqrt(u*u + v*v + w*w);
	t1 = rn / 2.0 * log((r+v)/(r-v));
	t2 = rm * log(r+w);
	rnum = u*v;
	rden = u*u + w*w + r*w;
	t3 = rl * atan2(rnum,rden);
	return (t1 + t2 + t3);
}

GMT_LOCAL double nucleoy(double u, double v, double w, double rl, double rm, double rn) {
	/* Multiply output by -1 because ... not sure but related to the fact that y vector is up-side down */
	double r, t1, t2, t3, rnum, rden;
	r = sqrt(u*u + v*v + w*w);
	t1 = rn / 2.0 * log((r+u)/(r-u));
	t2 = rl * log(r+w);
	rnum = u*v;
	rden = r*r + r*w - u*u;
	t3 = rm * atan2(rnum,rden);
	return (-(t1 + t2 + t3));
}

GMT_LOCAL double nucleoz(double u, double v, double w, double rl, double rm, double rn) {
	double r, t1, t2, t3, rnum, rden;
	r = sqrt(u*u + v*v + w*w);
	t1 = rm / 2.0 * log((r+u)/(r-u));
	t2 = rl / 2.0 * log((r+v)/(r-v));
	rnum = u*v;
	rden = r*w;
	t3 = -rn * atan2(rnum,rden);
	return (t1 + t2 + t3);
}

GMT_LOCAL void dircos(double incl, double decl, double azim, double *a, double *b, double *c) {
/*
c  Subroutine DIRCOS computes direction cosines from inclination
c  and declination.
c
c  Input parameters:
c    incl:  inclination in degrees positive below horizontal.
c    decl:  declination in degrees positive east of true north.
c    azim:  azimuth of x axis in degrees positive east of north.
c
c  Output parameters:
c    a,b,c:  the three direction cosines.
*/
	double xincl, xdecl, xazim;
	xincl = incl * D2R;
	xdecl = decl * D2R;
	xazim = azim * D2R ;
	*a = cos(xincl) * cos(xdecl-xazim);
	*b = cos(xincl) * sin(xdecl-xazim);
	*c = sin(xincl);
}

