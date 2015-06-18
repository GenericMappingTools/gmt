/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#define THIS_MODULE_NAME	"grdgravmag3d"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Computes the gravity effect of one (or two) grids by the method of Okabe"
#define THIS_MODULE_KEYS	"<GI,FDi,GGO"

#include "gmt_dev.h"
#include "okbfuns.h"
#include "../mgd77/mgd77.h"

#ifdef HAVE_GLIB_GTHREAD
#include <glib.h>
#endif

#define GMT_PROG_OPTIONS "-:RVfx"

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
		float z_dir;
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
		double z0;
	} Z;
	struct GRDOKB_box {	/* No option, just a container */
		bool is_geog;
		double	d_to_m, *mag_int, lon_0, lat_0;
	} box;
};

struct DATA {
	double  x, y;
} *data;

struct THREAD_STRUCT {
	unsigned int row, r_start, r_stop, n_pts, thread_num;
	double *x_grd, *x_grd_geo, *y_grd, *y_grd_geo, *x_obs, *y_obs, *cos_vec, *g;
	struct MAG_VAR *mag_var;
	struct LOC_OR *loc_or;
	struct BODY_DESC *body_desc;
	struct BODY_VERTS *body_verts;
	struct GRDOKB_CTRL *Ctrl;
	struct GMT_GRID *Grid;
	struct GMT_GRID *Gout;
	struct GMT_GRID *Gsource;
	struct GMT_CTRL *GMT;
};

int read_poly__ (struct GMT_CTRL *GMT, char *fname, bool switch_xy);
void set_center (unsigned int n_triang);
int grdgravmag3d_body_set_tri (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
	struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
	double *cos_vec, unsigned int j, unsigned int i, unsigned int inc_j, unsigned int inc_i);
int grdgravmag3d_body_set_prism(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
	struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
	double *cos_vec, unsigned int j, unsigned int i, unsigned int inc_j, unsigned int inc_i);
int grdgravmag3d_body_desc_tri(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc,
	struct BODY_VERTS **body_verts, unsigned int face);
int grdgravmag3d_body_desc_prism(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc,
	struct BODY_VERTS **body_verts, unsigned int face);
void grdgravmag3d_calc_surf (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid, struct GMT_GRID *Gout,
	struct GMT_GRID *Gsource, double *g, unsigned int n_pts, double *x_grd, double *y_grd, double *x_grd_geo, double *y_grd_geo,
	double *x_obs, double *y_obs, double *cos_vec, struct MAG_VAR *mag_var, struct LOC_OR *loc_or,
	struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts);
double mprism (struct GMT_CTRL *GMT, double x_o, double y_o, double z_o, double mag, bool is_grav,
	struct BODY_DESC bd_desc, struct BODY_VERTS *body_verts, unsigned int km, unsigned int i_comp, struct LOC_OR *loc_or);
double bhatta (struct GMT_CTRL *GMT, double x_o, double y_o, double z_o, double mag, bool is_grav,
	struct BODY_DESC bd_desc, struct BODY_VERTS *body_verts, unsigned int km, unsigned int i_comp, struct LOC_OR *loc_or);
void grdgravmag3d_calc_surf_ (struct THREAD_STRUCT *t);
double nucleox(double u, double v, double w, double rl, double rm, double rn);
double nucleoy(double u, double v, double w, double rl, double rm, double rn);
double nucleoz(double u, double v, double w, double rl, double rm, double rn);
void dircos(double incl, double decl, double azim, double *a, double *b, double *c);

double fast_atan(double x) {
	/* http://nghiaho.com/?p=997 */
	/* Efficient approximations for the arctangent function, Rajan, S. Sichun Wang Inkol, R. Joyal, A., May 2006 */
	return M_PI_4*x - x*(fabs(x) - 1)*(0.2447 + 0.0663*fabs(x));
}

#define FATAN(x) (fabs(x) > 1) ? atan(x) : (M_PI_4*x - x*(fabs(x) - 1)*(0.2447 + 0.0663*fabs(x)))

void *New_grdgravmag3d_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDOKB_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRDOKB_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->E.thickness = 500;
	C->L.zobs = 0;
	C->D.z_dir = -1;		/* -1 because Z was positive down for Okabe */
	C->S.radius = 30000;
	C->T.year = 2000;
	return (C);
}

void Free_grdgravmag3d_Ctrl (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file[0]) free (C->In.file[0]);
	if (C->In.file[1]) free (C->In.file[1]);
	if (C->F.file) free (C->F.file);
	if (C->G.file) free (C->G.file);
	if (C->H.magfile) free (C->H.magfile);
	if (C->H.decfile) free (C->H.decfile);
	if (C->H.incfile) free (C->H.incfile);
	GMT_free (GMT, C);
}

int GMT_grdgravmag3d_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdgravmag3d grdfile_top [grdfile_bot] [-C<density>] [-D] [-E<thick>] [-F<xy_file>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-G<outfile>] [-H<...>] [%s] [-L<z_obs>]\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Q[n<n_pad>]|[pad_dist]|[<w/e/s/n>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-S<radius>] [%s] [-Z<level>] [-fg] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_x_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\tgrdfile_top is the grdfile whose gravity efect is to be computed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If two grids are provided then the gravity/magnetic efect of the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   volume between them is computed\n\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C sets body density in SI\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F pass file with locations where anomaly is going to be computed\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G name of the output grdfile.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D if z is positive down [Default positive up]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E give layer thickness in m [Default = 500 m].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-H sets parameters for computation of magnetic anomaly (Can be used multiple times)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f_dec/f_dip -> geomagnetic declination/inclination\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m_int/m_dec/m_dip -> body magnetic intensity/declination/inclination\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  OR for a grid mode \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +m<magfile> where 'magfile' is the name of the magnetic intensity file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To compute a component, specify any of:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     x|X|e|E  to compute the E-W component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     y|Y|n|N  to compute the N-S component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     z|Z      to compute the Vertical component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     h|H      to compute the Horizontal component.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     t|T|f|F  to compute the total field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     For a variable inclination and declination use IGRF. Set any of -H+i|+g|+r|+f|+n to do that\n");
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t   The new xinc and yinc should be divisible by the old ones (new lattice is subset of old).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L sets level of observation [Default = 0]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Map units true; x,y in degrees, dist units in m [Default dist unit = x,y unit].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Extend the domain of computation with respect to output -R region.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qn<N> artifficially extends the width of the outer rim of cells to have a fake\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   width of N * dx[/dy].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Q<pad_dist> extend the region by west-pad, east+pad, etc\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Q<region> Same sintax as -R.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-R For new Range of output grid; enter <WESN> (xmin, xmax, ymin, ymax) separated by slashes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses the same region as the input grid].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S search radius in km (but only in the two grids mode) [Default = 30 km].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z z level of reference plane [Default = 0]\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Convert geographic grids to meters using a \"Flat Earth\" approximation.\n");
#ifdef HAVE_GLIB_GTHREAD
	GMT_Option (API, "x");
#else
	GMT_Message (API, GMT_TIME_NONE, "\t-x Not available since this binary was not build with multi-threading support.\n");
#endif
	GMT_Option (API, ":,.");

	return (EXIT_FAILURE);
}

int GMT_grdgravmag3d_parse (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_OPTION *options) {

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
					Ctrl->In.file[n_files++] = strdup (opt->arg);
				else if (n_files == 1)
					Ctrl->In.file[n_files++] = strdup (opt->arg);
				else {
					n_errors++;
					GMT_Report (API, GMT_MSG_NORMAL, "Error: A maximum of two input grids may be processed\n");
				}
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.rho = atof (opt->arg) * 6.674e-6;
				Ctrl->C.active = true;
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.z_dir = 1;
				break;
			case 'E':
				Ctrl->E.active = true;
				Ctrl->E.thickness = fabs(atof (opt->arg));
				break;
			case 'F':
				Ctrl->F.active = true;
				Ctrl->F.file = strdup (opt->arg);
				break;
 			case 'G':
				Ctrl->G.active = true;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'H':
				Ctrl->H.active = true;
				if (opt->arg[0] == '+' && opt->arg[1] == 'm') {
					Ctrl->H.magfile = strdup (&opt->arg[2]);        /* Source (magnetization) grid */
					Ctrl->H.got_maggrid = true;
					break;
				}
				else if (opt->arg[0] == '+' && opt->arg[1] == 'i') {
					if (opt->arg[2]) {
						Ctrl->H.incfile = strdup (&opt->arg[2]);    /* Inclination grid (NOT IMPLEMENTED YET) */
						Ctrl->H.got_incgrid = true;
					}
					else {
						Ctrl->H.do_igrf = true;                     /* Case when -H+i to mean use IGRF */
						if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
					}
					break;
				}
				else if (opt->arg[0] == '+' && opt->arg[1] == 'd') {
					Ctrl->H.decfile = strdup (&opt->arg[2]);        /* Declination grid (NOT IMPLEMENTED YET) */
					Ctrl->H.got_decgrid = true;
					break;
				}
				else if (opt->arg[0] == '+' && (opt->arg[1] == 'g' || opt->arg[1] == 'r' || opt->arg[1] == 'f' || opt->arg[1] == 'n')) {
					Ctrl->H.do_igrf = true;                         /* Anny of -H+i|+g|+r|+f|+n is allowed to mean use IGRF */
					if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
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
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -H option: Can't dechiper values\n");
					n_errors++;
				}
				break;
			case 'I':
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
	 		case 'L':
				Ctrl->L.zobs = atof (opt->arg);
				break;
			case 'M':
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -M is deprecated; -fg was set instead, use this in the future.\n");
					if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				if (opt->arg[0] == 'n') {
					Ctrl->Q.n_pad = atoi (&opt->arg[1]);
				}
				else {
					int n = 0;
					char *pch = strchr(opt->arg, '/');
					while (pch != NULL) {
						n++;
						pch = strchr(pch+1,'/');
					}
					if (n == 0)
						Ctrl->Q.pad_dist = atof (opt->arg);	/* Pad given as a scalar distance */
					else if (n == 3)
						strncpy(Ctrl->Q.region, opt->arg, GMT_BUFSIZ);	/* Pad given as a -R region */
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option. Either -Q<pad> or -Q<region>\n");
						n_errors++;
					}
				}
				break;
	 		case 'S':
				Ctrl->S.radius = atof (opt->arg) * 1000;
				Ctrl->S.active = true;
				break;
 			case 'T':
				break;
			case 'Z':
				Ctrl->Z.z0 = atof(opt->arg);
				break;
			case 'b':
				Ctrl->H.bhatta = true;
				break;
			default:
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->In.file[0], "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && (Ctrl->S.radius <= 0.0 || GMT_is_dnan (Ctrl->S.radius)),
				"Syntax error: -S Radius is NaN or negative\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.active && !Ctrl->F.active,
				"Error: Must specify either -G or -F options\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active && Ctrl->Q.active && !Ctrl->Q.n_pad,
				"Error: Cannot specify -Q<pad>|<region> without -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.rho == 0.0 && !Ctrl->H.active,
				"Error: Must specify either -Cdensity or -H<stuff>\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && Ctrl->H.active, 
				"Syntax error Cannot specify both -C and -H options\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.active && !Ctrl->G.file,
				"Syntax error -G option: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->H.got_maggrid && !Ctrl->H.magfile,
				"Syntax error -H+m option: Must specify source file\n");
	i += GMT_check_condition (GMT, Ctrl->G.active && Ctrl->F.active, "Warning: -F overrides -G\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdgravmag3d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdgravmag3d (void *V_API, int mode, void *args) {

	unsigned int nx_p, ny_p, i, j, k, ndata = 0, clockwise_type[] = {0, 5};
	bool    two_grids = false, switch_xy = false;
	unsigned int km = 0;		/* index of current body facet (for mag only) */
	int     error = 0, retval;
	unsigned int n_vert_max;
	double  a, d, x_o, y_o;
	double *x_obs = NULL, *y_obs = NULL, *x_grd = NULL, *y_grd = NULL, *x_grd_geo = NULL, *y_grd_geo = NULL;
	double *cos_vec = NULL, *g = NULL, *x_grd2 = NULL, *y_grd2 = NULL, *cos_vec2 = NULL;
	double  cc_t, cs_t, s_t, wesn_new[4], wesn_padded[4];

	struct  GMT_GRID *GridA = NULL, *GridB = NULL, *GridS = NULL, *Gout = NULL;
	struct  LOC_OR *loc_or = NULL;
	struct  BODY_VERTS *body_verts = NULL;
	struct  BODY_DESC body_desc;
	struct  GRDOKB_CTRL *Ctrl = NULL;
	struct  GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct  GMT_OPTION *options = NULL;
	struct  GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	mag_var = NULL, mag_param = NULL, data = NULL;
	body_desc.n_v = NULL, body_desc.ind = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdgravmag3d_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE)
		bailout (GMT_grdgravmag3d_usage (API, GMT_USAGE));		/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS)
		bailout (GMT_grdgravmag3d_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	GMT->common.x.n_threads = 1;        /* Default to use only one core (we may change this to max cores) */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdgravmag3d_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdgravmag3d_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdgravmag3d main code ---------------------*/

	if (GMT_is_geographic (GMT, GMT_IN)) Ctrl->box.is_geog = true;

	if (!Ctrl->box.is_geog)
		Ctrl->box.d_to_m = 1.;
	else
		Ctrl->box.d_to_m = 2.0 * M_PI * 6371008.7714 / 360.0;
		/*Ctrl->box.d_to_m = 2.0 * M_PI * gmtdefs.ellipse[N_ELLIPSOIDS-1].eq_radius / 360.0;*/

	if (Ctrl->F.active) { 		/* Read xy file where anomaly is to be computed */
		if ( (retval = read_poly__ (GMT, Ctrl->F.file, switch_xy)) < 0 ) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open file %s\n", Ctrl->F.file);
			return (EXIT_FAILURE);
		}
		ndata = retval;
	}

	/* ---------------------------------------------------------------------------- */

	if ((GridA = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL,
	                            Ctrl->In.file[0], NULL)) == NULL) 	/* Get header only */
		Return (API->error);

	if (Ctrl->G.active) {
		double wesn[4], inc[2];
		/* Use the -R region for output if set; otherwise match grid domain */
		GMT_memcpy (wesn, (GMT->common.R.active ? GMT->common.R.wesn : GridA->header->wesn), 4, double);
		GMT_memcpy (inc, (Ctrl->I.active ? Ctrl->I.inc : GridA->header->inc), 2, double);
		if (wesn[XLO] < GridA->header->wesn[XLO]) error = true;
		if (wesn[XHI] > GridA->header->wesn[XHI]) error = true;

		if (wesn[YLO] < GridA->header->wesn[YLO]) error = true;
		if (wesn[YHI] > GridA->header->wesn[YHI]) error = true;

		if (error) {
			GMT_Report (API, GMT_MSG_NORMAL, "New WESN incompatible with old.\n");
			Return (EXIT_FAILURE);
		}

		if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, wesn, inc,
			GridA->header->registration, GMT_NOTSET, NULL)) == NULL) Return (API->error);

		GMT_Report (API, GMT_MSG_VERBOSE, "Grid dimensions are nx = %d, ny = %d\n",
					Gout->header->nx, Gout->header->ny);
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "Allocates memory and read data file\n");

	if (!GMT->common.R.active)
		GMT_memcpy (wesn_new, GridA->header->wesn, 4, double);
	else
		GMT_memcpy (wesn_new, GMT->common.R.wesn,  4, double);

	/* Process Padding request */
	if (Ctrl->Q.active && !Ctrl->Q.n_pad) {
		if (Ctrl->Q.pad_dist)		/* Convert the scalar pad distance into a -R string type */
			sprintf (Ctrl->Q.region, "%f/%f/%f/%f", wesn_new[XLO] - Ctrl->Q.pad_dist,
			         wesn_new[XHI] + Ctrl->Q.pad_dist, wesn_new[YLO] - Ctrl->Q.pad_dist,
			         wesn_new[YHI] + Ctrl->Q.pad_dist);

		GMT->common.R.active = false;
		GMT_parse_common_options (GMT, "R", 'R', Ctrl->Q.region);	/* Use the -R parsing machinery to handle this */
		GMT_memcpy (wesn_padded, GMT->common.R.wesn, 4, double);
		GMT_memcpy (GMT->common.R.wesn, wesn_new, 4, double);		/* Reset previous WESN */
		GMT->common.R.active = true;

		if (wesn_padded[XLO] < GridA->header->wesn[XLO]) {
			GMT_Report (API, GMT_MSG_NORMAL, "Request padding at the West border exceed grid limit, trimming it\n");
			wesn_padded[XLO] = GridA->header->wesn[XLO];
		}
		if (wesn_padded[XHI] > GridA->header->wesn[XHI]) {
			GMT_Report (API, GMT_MSG_NORMAL, "Request padding at the East border exceed grid limit, trimming it\n");
			wesn_padded[XHI] = GridA->header->wesn[XHI];
		}
		if (wesn_padded[YLO] < GridA->header->wesn[YLO]) {
			GMT_Report (API, GMT_MSG_NORMAL, "Request padding at the South border exceed grid limit, trimming it\n");
			wesn_padded[YLO] = GridA->header->wesn[YLO];
		}
		if (wesn_padded[YHI] > GridA->header->wesn[YHI]) {
			GMT_Report (API, GMT_MSG_NORMAL, "Request padding at the North border exceed grid limit, trimming it\n");
			wesn_padded[YHI] = GridA->header->wesn[YHI];
		}
	}
	else
		GMT_memcpy (wesn_padded, GridA->header->wesn, 4, double);

	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn_padded,
	                   Ctrl->In.file[0], GridA) == NULL) {			/* Get subset, or all */
		Return (API->error);
	}

	/* Check that Inner region request does not exceeds input grid limits */
	if (GMT->common.R.active && Ctrl->G.active) {
		if (Gout->header->wesn[XLO] < GridA->header->wesn[XLO] ||
		    Gout->header->wesn[XHI] > GridA->header->wesn[XHI]) {
			GMT_Report (API, GMT_MSG_NORMAL, " Selected region exceeds the X-boundaries of the grid file!\n");
			return (EXIT_FAILURE);
		}
		else if (Gout->header->wesn[YLO] < GridA->header->wesn[YLO] ||
		         Gout->header->wesn[YHI] > GridA->header->wesn[YHI]) {
			GMT_Report (API, GMT_MSG_NORMAL, " Selected region exceeds the Y-boundaries of the grid file!\n");
			return (EXIT_FAILURE);
		}
		GMT_RI_prepare (GMT, Gout->header);	/* Ensure -R -I consistency and set nx, ny */
	}

	Ctrl->box.lon_0 = (GridA->header->wesn[XLO] + GridA->header->wesn[XHI]) / 2;
	Ctrl->box.lat_0 = (GridA->header->wesn[YLO] + GridA->header->wesn[YHI]) / 2;

	if (Ctrl->Z.z0 > GridA->header->z_max) {
		/* Typical when computing the effect of whater shell for Buguer reduction of marine data */
		clockwise_type[0] = 5;		/* Means Top triangs will have a CCW description and CW for bot */
		clockwise_type[1] = 0;
	}

	if (Ctrl->D.z_dir != 1) {
		Ctrl->Z.z0 *= Ctrl->D.z_dir;
		for (j = 0; j < GridA->header->size; j++)
			GridA->data[j] *= Ctrl->D.z_dir;
	}

	/* -------------- In case we have one second grid, for bottom surface -------------- */
	if (Ctrl->In.file[1]) {
		if ((GridB = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL,
		                            Ctrl->In.file[1], NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}

		if(GridA->header->registration != GridB->header->registration) {
			GMT_Report (API, GMT_MSG_NORMAL, "Up and bottom grids have different registrations!\n");
			Return (EXIT_FAILURE);
		}
		if ((GridA->header->z_scale_factor != GridB->header->z_scale_factor) ||
				(GridA->header->z_add_offset != GridB->header->z_add_offset)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Up and bottom grid scale/offset not compatible!\n");
			Return (EXIT_FAILURE);
		}

		if (fabs (GridA->header->inc[GMT_X] - GridB->header->inc[GMT_X]) > 1.0e-6 ||
		          fabs (GridA->header->inc[GMT_Y] - GridB->header->inc[GMT_Y]) > 1.0e-6) {
			GMT_Report (API, GMT_MSG_NORMAL, "Up and bottom grid increments do not match!\n");
			Return (EXIT_FAILURE);
		}

		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn_padded,
		                   Ctrl->In.file[1], GridB) == NULL) {			/* Get subset, or all */
			Return (API->error);
		}

		if (Ctrl->D.z_dir != -1) {
			for (j = 0; j < GridB->header->size; j++)
				GridB->data[j] *= Ctrl->D.z_dir;
		}
		two_grids = true;
	}

	/* -------------- In case we have  a source (magnetization) grid -------------------- */
	if (Ctrl->H.got_maggrid) {
		if ((GridS = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL,
		                            Ctrl->H.magfile, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}

		if(GridA->header->registration != GridS->header->registration) {
			GMT_Report (API, GMT_MSG_NORMAL, "Up surface and source grids have different registrations!\n");
			Return (EXIT_FAILURE);
		}
		if ((GridA->header->z_scale_factor != GridS->header->z_scale_factor) ||
		    (GridA->header->z_add_offset != GridS->header->z_add_offset)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Up surface and source grid scale/offset not compatible!\n");
			Return (EXIT_FAILURE);
		}

		if (fabs (GridA->header->inc[GMT_X] - GridS->header->inc[GMT_X]) > 1.0e-6 ||
		          fabs (GridA->header->inc[GMT_Y] - GridS->header->inc[GMT_Y]) > 1.0e-6) {
			GMT_Report (API, GMT_MSG_NORMAL, "Up surface and source grid increments do not match!\n");
			Return (EXIT_FAILURE);
		}

		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn_padded,
		                   Ctrl->H.magfile, GridS) == NULL) {			/* Get subset, or all */
			Return (API->error);
		}
	}

	if (Ctrl->S.active && !two_grids && !Ctrl->H.pirtt && !Ctrl->E.active) {
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: unset -S option. It can only be used when two grids were provided OR -E.\n");
		Ctrl->S.active = false;
	}
	if (two_grids && Ctrl->E.active) {
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: two grids override -E option. Unsetting it.\n");
		Ctrl->E.active = false;		/* But in future we may have a second grid with variable magnetization and cte thickness */
	}

	nx_p = !Ctrl->F.active ? Gout->header->nx : ndata;
	ny_p = !Ctrl->F.active ? Gout->header->ny : ndata;
	x_grd = GMT_memory (GMT, NULL, GridA->header->nx + Ctrl->H.pirtt, double);
	y_grd = GMT_memory (GMT, NULL, GridA->header->ny + Ctrl->H.pirtt, double);
	x_obs = GMT_memory (GMT, NULL, nx_p, double);
	y_obs = GMT_memory (GMT, NULL, ny_p, double);
	if (Ctrl->F.active) g = GMT_memory (GMT, NULL, ndata, double);

	d = GridA->header->xy_off;		/*  0.5 : 0.0 */

	/* ----------------------- Build observation point vectors ---------------------------------- */
	if (Ctrl->G.active) {
		for (i = 0; i < Gout->header->nx; i++)
			x_obs[i] = (i == (Gout->header->nx-1)) ? Gout->header->wesn[XHI] - d * Gout->header->inc[GMT_X] :
					Gout->header->wesn[XLO] + (i + d) * Gout->header->inc[GMT_X];
		if (Ctrl->H.pirtt) {
			for (j = 0; j < Gout->header->ny; j++)
				y_obs[j] = (j == (Gout->header->ny-1)) ? (Gout->header->wesn[YLO] - d*Gout->header->inc[GMT_Y]) :
						(Gout->header->wesn[YHI] - (j + d) * Gout->header->inc[GMT_Y]);
		}
		else {
			for (j = 0; j < Gout->header->ny; j++)
				y_obs[j] = (j == (Gout->header->ny-1)) ? -(Gout->header->wesn[YLO] + d * Gout->header->inc[GMT_Y]) :
						-(Gout->header->wesn[YHI] - (j + d) * Gout->header->inc[GMT_Y]);
		}
	}

	if (Ctrl->H.pirtt)
		d -= 0.5;			/* because for prisms we want all corner coords to be in pixel registration */

	for (i = 0; i < GridA->header->nx; i++)
		x_grd[i] = (i == (GridA->header->nx-1)) ? GridA->header->wesn[XHI] + d*GridA->header->inc[GMT_X] :
				GridA->header->wesn[XLO] + (i + d) * GridA->header->inc[GMT_X];
	if (Ctrl->H.pirtt) {
		for (j = 0; j < GridA->header->ny; j++)
			y_grd[j] = (j == (GridA->header->ny-1)) ? (GridA->header->wesn[YLO] - d*GridA->header->inc[GMT_Y]) :
					(GridA->header->wesn[YHI] - (j + d) * GridA->header->inc[GMT_Y]);

		x_grd[i] = x_grd[i - 1] + GridA->header->inc[GMT_X];		/* We need this extra pt because we went from grid to pix reg */
		y_grd[j] = y_grd[j - 1] - GridA->header->inc[GMT_Y];
	}
	else {
		for (j = 0; j < GridA->header->ny; j++)
			y_grd[j] = (j == (GridA->header->ny-1)) ? -(GridA->header->wesn[YLO] + d*GridA->header->inc[GMT_Y]) :
					-(GridA->header->wesn[YHI] - (j + d) * GridA->header->inc[GMT_Y]);
	}

	if (two_grids) {
		d = GridB->header->xy_off;		/*  0.5 : 0.0 */
		if (Ctrl->H.pirtt)
			d -= 0.5;			/* because for prisms we want all corner coords to be in pixel registration */
		x_grd2 = GMT_memory (GMT, NULL, GridB->header->nx + Ctrl->H.pirtt, double);
		y_grd2 = GMT_memory (GMT, NULL, GridB->header->ny + Ctrl->H.pirtt, double);
		for (i = 0; i < GridB->header->nx; i++)
			x_grd2[i] = (i == (GridB->header->nx-1)) ? GridB->header->wesn[XHI] + d*GridB->header->inc[GMT_X] :
					GridB->header->wesn[XLO] + (i + d) * GridB->header->inc[GMT_X];
		if (Ctrl->H.pirtt) {
			for (j = 0; j < GridB->header->ny; j++)
				y_grd2[j] = (j == (GridB->header->ny-1)) ? (GridB->header->wesn[YHI] - d*GridB->header->inc[GMT_Y]) :
						(GridB->header->wesn[YHI] - (j + d) * GridB->header->inc[GMT_Y]);

			x_grd2[i] = x_grd2[i - 1] + GridB->header->inc[GMT_X];		/* We need this extra pt because we went from grid to pix reg */
			y_grd2[j] = y_grd2[j - 1] - GridB->header->inc[GMT_Y];
		}
		else {
			for (j = 0; j < GridB->header->ny; j++)
				y_grd2[j] = (j == (GridB->header->ny-1)) ? -(GridB->header->wesn[YLO] + d*GridB->header->inc[GMT_Y]) :
						-(GridB->header->wesn[YHI] - (j + d) * GridB->header->inc[GMT_Y]);
		}
	}
	/* --------------------------------------------------------------------------------------------- */

	/* ---------- Pre-compute a vector of cos(lat) to use in the Geog cases ------------------------ */
	cos_vec = GMT_memory (GMT, NULL, GridA->header->ny, double);
	for (j = 0; j < GridA->header->ny; j++)
		cos_vec[j] = (Ctrl->box.is_geog) ? cos(y_grd[j]*D2R): 1;
	if (two_grids) {
		cos_vec2 = GMT_memory (GMT, NULL, GridB->header->ny, double);
		for (j = 0; j < GridB->header->ny; j++)
			cos_vec2[j] = (Ctrl->box.is_geog) ? cos(y_grd2[j]*D2R): 1;
	}
	/* --------------------------------------------------------------------------------------------- */

	/* ------ Convert distances to arc distances at Earth surface *** NEEDS CONFIRMATION *** ------- */
	if (Ctrl->box.is_geog) {
		if (Ctrl->H.do_igrf) {          /* We need a copy in Geogs to use in IGRF */
			x_grd_geo = GMT_memory (GMT, NULL, GridA->header->nx + Ctrl->H.pirtt, double);
			y_grd_geo = GMT_memory (GMT, NULL, GridA->header->ny + Ctrl->H.pirtt, double);
			GMT_memcpy(x_grd_geo, x_grd, GridA->header->nx + Ctrl->H.pirtt, double);
			GMT_memcpy(y_grd_geo, y_grd, GridA->header->ny + Ctrl->H.pirtt, double);
		}
		for (i = 0; i < GridA->header->nx; i++)
			x_grd[i] = (x_grd[i] - Ctrl->box.lon_0) * Ctrl->box.d_to_m;
		for (j = 0; j < GridA->header->ny; j++)
			y_grd[j] = (y_grd[j] + Ctrl->box.lat_0) * Ctrl->box.d_to_m;

		if (two_grids) {
			for (i = 0; i < GridB->header->nx; i++)
				x_grd2[i] = (x_grd[i] - Ctrl->box.lon_0) * Ctrl->box.d_to_m;
			for (j = 0; j < GridB->header->ny; j++)
				y_grd2[j] = (y_grd[j] + Ctrl->box.lat_0) * Ctrl->box.d_to_m;
		}
	}
	/* --------------------------------------------------------------------------------------------- */

	if (Ctrl->Q.n_pad) {
		x_grd[0] = x_grd[1] - Ctrl->Q.n_pad * (x_grd[2] - x_grd[1]);
		y_grd[0] = y_grd[1] - Ctrl->Q.n_pad * fabs(y_grd[2] - y_grd[1]);
		x_grd[GridA->header->nx-1] = x_grd[GridA->header->nx-1] + Ctrl->Q.n_pad * (x_grd[2] - x_grd[1]);
		y_grd[GridA->header->ny-1] = y_grd[GridA->header->ny-1] + Ctrl->Q.n_pad * fabs(y_grd[2] - y_grd[1]);
	}

	if (Ctrl->F.active) { 	/* Need to compute observation coords only once */
		for (i = 0; i < ndata; i++) {
			x_obs[i] = (Ctrl->box.is_geog) ?  (data[i].x - Ctrl->box.lon_0)*Ctrl->box.d_to_m*cos(data[i].y*D2R) : data[i].x;
			y_obs[i] = (Ctrl->box.is_geog) ? -(data[i].y - Ctrl->box.lat_0)*Ctrl->box.d_to_m : -data[i].y; /* - because y positive 'south' */
		}
	}

	if (Ctrl->H.active) { /* 1e2 is a factor to obtain nT from magnetization in A/m */
		mag_param = GMT_memory (GMT, NULL, 1, struct MAG_PARAM);
		mag_param[0].rim[0] = 1e2 * cos(Ctrl->H.t_dip*D2R) * cos((Ctrl->H.t_dec - 90.)*D2R);
		mag_param[0].rim[1] = 1e2 * cos(Ctrl->H.t_dip*D2R) * sin((Ctrl->H.t_dec - 90.)*D2R);
		mag_param[0].rim[2] = 1e2 * sin(Ctrl->H.t_dip*D2R);
		cc_t = cos(Ctrl->H.m_dip*D2R) * cos((Ctrl->H.m_dec - 90.)*D2R);
		cs_t = cos(Ctrl->H.m_dip*D2R) * sin((Ctrl->H.m_dec - 90.)*D2R);
		s_t  = sin(Ctrl->H.m_dip*D2R);
		/* Case of constant magnetization */
		mag_var = GMT_memory (GMT, NULL, 1, struct MAG_VAR);
		mag_var[0].rk[0] = Ctrl->H.m_int * cc_t;
		mag_var[0].rk[1] = Ctrl->H.m_int * cs_t;
		mag_var[0].rk[2] = Ctrl->H.m_int * s_t;
	}

	/* Decompose a paralelogram into two triangular prisms */
	if (!Ctrl->In.file[1] && !Ctrl->In.file[2])	{	/* One grid only - EXPLICAR */
		if (!Ctrl->H.pirtt)
			grdgravmag3d_body_desc_tri(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[0]);		/* Set CW or CCW of top triangs */
		else
			grdgravmag3d_body_desc_prism(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[0]);	/* Allocate structs */
	}

	/* Allocate a structure that will be used inside okabe().
	   We do it here to avoid thousands of alloc/free that would result if done in okabe() */
	n_vert_max = body_desc.n_v[0];
	for (i = 1; i < body_desc.n_f; i++)
		n_vert_max = MAX(body_desc.n_v[i], n_vert_max);

	loc_or = GMT_memory (GMT, NULL, (n_vert_max+1), struct LOC_OR);

	if (Ctrl->H.bhatta) {
		dircos(Ctrl->H.m_dip, Ctrl->H.m_dec, 0, &loc_or[0].x, &loc_or[0].y, &loc_or[0].z);
		dircos(Ctrl->H.t_dip, Ctrl->H.t_dec, 0, &loc_or[1].x, &loc_or[1].y, &loc_or[1].z);
	}
	else if (Ctrl->H.pirtt) {
		/* This retains several of the original FORTRAN code variable names (easy to tell) */
		double sa, si, ci, si0, sa0, ca0, cisa, cica, h1, h2, h3;

		sa = -Ctrl->H.t_dec * D2R;		// Earth field declination
		si =  Ctrl->H.t_dip * D2R;		// Earth field dip (inclination)
		ci = cos(si);					// cos of field INC
		si0= sin(si);					// sin of field INC
		sa0= sin(sa);					// sin of field DEC
		ca0= cos(sa);					// cos of field DEC
		cisa= ci * sa0;
		cica= ci * ca0;

		h1 = cisa;
		h2 = cica;
		h3 = si0;

		if (Ctrl->H.koningsberg != 0) {		/* This option is not currently implemented */
			sa = (-Ctrl->H.t_dec - Ctrl->H.m_dec) * D2R;	// Earth field DEC - Mag DEC 
			ci = cos(Ctrl->H.m_dip * D2R);					// cos of mag INC
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

/* ---------------> Now start computing <------------------------------------- */

	if (Ctrl->G.active) {               /* grid output */
		grdgravmag3d_calc_surf (GMT, Ctrl, GridA, Gout, GridS, NULL, 0, x_grd, y_grd, x_grd_geo, y_grd_geo, x_obs, y_obs, cos_vec,
		                        mag_var, loc_or, &body_desc, body_verts);

		if (Ctrl->H.pirtt) goto L1;                            /* Uggly, I know but the 2-grids case is not Bhattacharya implemented */

		if (!two_grids) {                                       /* That is, one grid and a flat base Do the BASE now */
			grdgravmag3d_body_desc_tri(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of BOT triangs */

			if (!Ctrl->E.active) {                              /* NOT constant thickness. That is, a surface and a BASE plane */
				grdgravmag3d_body_set_tri(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 0, 0,
				                          GridA->header->ny-1, GridA->header->nx-1);

				for (k = 0; k < Gout->header->ny; k++) {        /* Loop over input grid rows */
					y_o = (Ctrl->box.is_geog) ? (y_obs[k] + Ctrl->box.lat_0) * Ctrl->box.d_to_m : y_obs[k];	 /* +lat_0 because y was already *= -1 */

					for (i = 0; i < Gout->header->nx; i++) {    /* Loop over input grid cols */
						x_o = (Ctrl->box.is_geog) ? (x_obs[i] - Ctrl->box.lon_0) * Ctrl->box.d_to_m * cos(y_obs[k]*D2R) : x_obs[i];
						a = okabe (GMT, x_o, y_o, Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, body_desc, body_verts, km, 0, loc_or);
						Gout->data[GMT_IJP(Gout->header, k, i)] += (float)a;
					}
				}
			}
			else {      /* A Constant thickness layer */
				grdgravmag3d_calc_surf (GMT, Ctrl, GridA, Gout, GridS, NULL, 0, x_grd, y_grd, x_grd_geo, y_grd_geo, x_obs, y_obs, cos_vec,
				                        mag_var, loc_or, &body_desc, body_verts);
			}
		}
		else {          /* "two_grids". One at the top and the other at the base */
			grdgravmag3d_body_desc_tri(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of top triangs */
			grdgravmag3d_calc_surf (GMT, Ctrl, GridB, Gout, GridS, NULL, 0, x_grd2, y_grd2, x_grd_geo, y_grd_geo, x_obs, y_obs,
			                        cos_vec2, mag_var, loc_or, &body_desc, body_verts);
		}
	}
	else {              /* polyline output */
		grdgravmag3d_calc_surf (GMT, Ctrl, GridA, GridS, NULL, g, ndata, x_grd, y_grd, x_grd_geo, y_grd_geo, x_obs, y_obs, cos_vec,
		                        mag_var, loc_or, &body_desc, body_verts);

		if (Ctrl->H.pirtt) goto L1;     /* Uggly,I know but the 2-grids case is not Bhattacharya implemented */

		if (!two_grids) {               /* That is, one grid and a flat base. Do the BASE now */
			grdgravmag3d_body_desc_tri(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of BOT triangs */

			if (!Ctrl->E.active) {      /* NOT constant thickness. That is, a surface and a BASE plane */
				grdgravmag3d_body_set_tri(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 0, 0,
				                          GridA->header->ny-1, GridA->header->nx-1);
			}
			else {                      /* A Constant thickness layer */
				grdgravmag3d_body_set_tri(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 0, 0, 1, 1);
			}

			for (k = 0; k < ndata; k++)
				g[k] += okabe (GMT, x_obs[k], y_obs[k], Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, body_desc, body_verts, km, 0, loc_or);
		}
		else {                          /* "two_grids". One at the top and the other at the base */
			grdgravmag3d_body_desc_tri(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of top triangs */
			grdgravmag3d_calc_surf (GMT, Ctrl, GridB, NULL, GridS, g, ndata, x_grd2, y_grd2, x_grd_geo, y_grd_geo, x_obs, y_obs,
			                        cos_vec2, mag_var, loc_or, &body_desc, body_verts);
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
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Gout) != GMT_OK) {
			Return (API->error);
		}
	}
	else {
		double out[3];
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
			Return (API->error);
		}
		if ((error = GMT_set_cols (GMT, GMT_OUT, 3)) != GMT_OK) {
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		for (k = 0; k < ndata; k++) {
			out[GMT_X] = data[k].x;
			out[GMT_Y] = data[k].y;
			out[GMT_Z] = g[k];
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
	}

	GMT_free (GMT, x_grd);
	GMT_free (GMT, y_grd);
	if (x_grd2) GMT_free (GMT, x_grd2);
	if (y_grd2) GMT_free (GMT, y_grd2);
	if (cos_vec2) GMT_free (GMT, cos_vec2);
	if (g) GMT_free (GMT, g);
	if (data) GMT_free (GMT, data);
	GMT_free (GMT, x_obs);
	GMT_free (GMT, y_obs);
	GMT_free (GMT, cos_vec);
	GMT_free (GMT, loc_or);
	GMT_free (GMT, body_desc.n_v);
	GMT_free (GMT, body_desc.ind);
	GMT_free (GMT, body_verts);
	if (mag_param) GMT_free (GMT, mag_param);
	if (mag_var) GMT_free (GMT, mag_var);
	if (Ctrl->H.do_igrf) {
		GMT_free(GMT, x_grd_geo);		GMT_free(GMT, y_grd_geo);
	}

	Return (GMT_OK);
}

/* -----------------------------------------------------------------*/
int read_poly__ (struct GMT_CTRL *GMT, char *fname, bool switch_xy) {
	/* Read file with xy points where anomaly is going to be computed
	   This is a temporary function while we not use the API to do this job. */
	unsigned int ndata, ix = 0, iy = 1;
	size_t n_alloc;
	double in[2];
	char line[GMT_LEN256] = {""};
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata = 0;
	if (switch_xy) {iy = 0; ix = 1;}

	data = GMT_memory (GMT, NULL, n_alloc, struct DATA);

	while (fgets (line, GMT_LEN256, fp)) {
		if (sscanf (line, "%lg %lg", &in[0], &in[1]) !=2)
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering line %d of polygon file\n", ndata+1);
		if (ndata == n_alloc) {
			n_alloc <<= 1;
			data = GMT_memory (GMT, data, n_alloc, struct DATA);
		}
		data[ndata].x = in[ix];
		data[ndata].y = in[iy];
		ndata++;
	}
	fclose(fp);
	return (ndata);
}

/* -----------------------------------------------------------------*/
int grdgravmag3d_body_desc_tri(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc,
                               struct BODY_VERTS **body_verts, unsigned int face) {
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
			body_desc->n_v = GMT_memory (GMT, NULL, body_desc->n_f, unsigned int);
		body_desc->n_v[0] = body_desc->n_v[1] = 3;
		if (body_desc->ind == NULL)
			body_desc->ind = GMT_memory (GMT, NULL, body_desc->n_v[0] + body_desc->n_v[1], unsigned int);
		body_desc->ind[0] = 0;	body_desc->ind[1] = 1; 	body_desc->ind[2] = 2;	/* 1st top triang (0 1 3)*/
		body_desc->ind[3] = 0;	body_desc->ind[4] = 2; 	body_desc->ind[5] = 3;	/* 2nd top triang (1 2 3) */
		if (*body_verts == NULL) *body_verts = GMT_memory (GMT, NULL, 4, struct BODY_VERTS);
		return(0);
	}
	else if (face == 5) {			/* Decompose the BOT square surface in 2 triangles using CCW order */
		body_desc->n_f = 2;
		if (body_desc->n_v == NULL)
			body_desc->n_v = GMT_memory (GMT, NULL, body_desc->n_f, unsigned int);
		body_desc->n_v[0] = body_desc->n_v[1] = 3;
		if (body_desc->ind == NULL)
			body_desc->ind = GMT_memory (GMT, NULL, body_desc->n_v[0] + body_desc->n_v[1], unsigned int);
		body_desc->ind[0] = 0;	body_desc->ind[1] = 2; 	body_desc->ind[2] = 1;	/* 1st bot triang */
		body_desc->ind[3] = 0;	body_desc->ind[4] = 3; 	body_desc->ind[5] = 2;	/* 2nd bot triang */
		if (*body_verts == NULL) *body_verts = GMT_memory (GMT, NULL, 4, struct BODY_VERTS);
		return(0);
	}
	/* Other face cases will go here */
	return(0);
}

/* -----------------------------------------------------------------------------------*/
int grdgravmag3d_body_desc_prism(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc,
                                 struct BODY_VERTS **body_verts, unsigned int face) {
	GMT_UNUSED(Ctrl);
	if (face != 0 && face != 5) return(0);

	body_desc->n_f = 1;
	if (body_desc->n_v == NULL)		/* First time this function is called */
		body_desc->n_v = GMT_memory (GMT, NULL, body_desc->n_f, unsigned int);
	body_desc->n_v[0] = 2;
	if (body_desc->ind == NULL)
		body_desc->ind = GMT_memory (GMT, NULL, body_desc->n_v[0], unsigned int);
	if (*body_verts == NULL) *body_verts = GMT_memory (GMT, NULL, 2, struct BODY_VERTS);

	body_desc->ind[0] = 0;	body_desc->ind[1] = 1;	/* NOT USED REALY AREN'T THEY? */

	return(0);
}

/* -----------------------------------------------------------------------------------*/
int grdgravmag3d_body_set_tri(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
		struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
		double *cos_vec, unsigned int j, unsigned int i, unsigned int inc_j, unsigned int inc_i) {

	/* Allocate and fill the body_desc structure with the description on how to
	   connect the vertex of the polygonal planar surface */

	unsigned int i1, j1, ij;
	bool is_geog = Ctrl->box.is_geog;
	float *z = Grid->data;
	double cosj, cosj1;
	struct GMT_GRID_HEADER *h = Grid->header;

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
		ij = GMT_IJP(h,j,i);
		body_verts[0].z = z[ij];
		body_verts[1].z = z[ij + 1];  /* z[GMT_IJP(h,j,i1)];  */
		ij = GMT_IJP(h,j1,i1);
		body_verts[2].z = z[ij];      /* z[GMT_IJP(h,j1,i1)]; */
		body_verts[3].z = z[ij - 1];  /* z[GMT_IJP(h,j1,i)];  */
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
int grdgravmag3d_body_set_prism(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
		struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
		double *cos_vec, unsigned int j, unsigned int i, unsigned int inc_j, unsigned int inc_i) {

	/* Allocate and fill the body_desc structure with the description on how to
	   connect the vertex of the rectangular planar surface */

	unsigned int i1, j1;
	bool is_geog = Ctrl->box.is_geog;
	float *z = Grid->data;
	double cosj, cosj1;
	struct GMT_GRID_HEADER *h = Grid->header;

	j1 = j + inc_j;         i1 = i + inc_i;
	cosj = cos_vec[j];      cosj1 = cos_vec[j1];

	body_verts[0].x = (is_geog) ? x[i]  * cosj  : x[i];
	body_verts[1].x = (is_geog) ? x[i1] * cosj1 : x[i1];

	body_verts[0].y = y[j1];	/* Reverse order because y[] starts from y_max and decreases */
	body_verts[1].y = y[j];

	body_verts[0].z = z[GMT_IJP(h,j,i)];
	body_verts[1].z = (Ctrl->E.active) ? z[GMT_IJP(h,j,i)] + Ctrl->E.thickness : Ctrl->Z.z0;
	return(0);
}


/* -----------------------------------------------------------------------------------*/
#ifdef HAVE_GLIB_GTHREAD
static void *thread_function (void *args) {
	grdgravmag3d_calc_surf_ ((struct THREAD_STRUCT *)args);
	return NULL;
}
#endif

void grdgravmag3d_calc_surf_ (struct THREAD_STRUCT *t) {
	/* Do the actual work. This function is called in either threaded and non-threaded cases. */

	char   tabs[16] = {""}, frmt[64] = {""};
	unsigned int row, col, k, i, km = 0, pm = 0, indf;
	double rho_or_mag, x_o, y_o, tmp = 1, a, DX, DY, s_rad2;
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
		body_verts = GMT_memory (GMT, NULL, 4, struct BODY_VERTS);		/* 4 is good enough for Okabe (tri = 4) and Mprism (prism = 2) cases */

	v_func[0] = grdgravmag3d_body_set_tri;
	v_func[1] = grdgravmag3d_body_set_prism;
	v_func[2] = grdgravmag3d_body_set_prism;
	d_func[0] = okabe;
	d_func[1] = mprism;
	d_func[2] = bhatta;

	indf = (Ctrl->H.pirtt) ? 1 + Ctrl->H.bhatta : 0;
	rho_or_mag = (Ctrl->C.active) ? Ctrl->C.rho : Ctrl->H.m_int;	/* What are we computing? (But it may be overriden below) */

	/* For Bhattacharya, select which component */
	pm = (Ctrl->H.f_tot) ? 0 : ((Ctrl->H.x_comp) ? 1 : ((Ctrl->H.y_comp) ? 2 : ((Ctrl->H.z_comp) ? 3 : ((Ctrl->H.h_comp) ? 4 : 0)) ));
	if (Ctrl->H.bhatta && pm > 0) pm = pm - 1;		/* TEMP TEMP TEMP. Will crash if h_comp */

	s_rad2 = Ctrl->S.radius * Ctrl->S.radius;

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		for (i = 0; i < MIN((unsigned)(t->thread_num + 1), 5); i++) {
			tabs[i] = '\t';
		}
		sprintf(frmt, "Thread %%d%s Row = %%d\t of = %%.3d\r", tabs);
	}

	if (Ctrl->H.do_igrf) {
		igrf_dip = GMT_memory (GMT, NULL, (size_t)(Grid->header->nx + 1), double);
		igrf_dec = GMT_memory (GMT, NULL, (size_t)(Grid->header->nx + 1), double);
	}

	for (row = r_start; row < r_stop; row++) {                     /* Loop over input grid rows */

		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE))
			GMT_Message(GMT->parent, GMT_TIME_NONE, frmt, t->thread_num + 1, row + 1, r_stop);
			//GMT_Message (GMT->parent, GMT_TIME_NONE, "Thread %d\tRow = %d\t of = %.3d\r", t->thread_num+1, row+1, Grid->header->ny - !indf);

		if (Ctrl->H.do_igrf) {                                     /* Compute a row of IGRF dec & dip */
			for (col = 0; col < Grid->header->nx - 1 + MIN(indf,1); col++) { 
				MGD77_igrf10syn(GMT, 0, 2000, 1, 0, t->x_grd_geo[col], t->y_grd_geo[row], out_igrf); 
				igrf_dec[col] = out_igrf[5] * D2R;
				igrf_dip[col] = out_igrf[6] * D2R;
			}
		}

		for (col = 0; col < Grid->header->nx - 1 + MIN(indf,1); col++) {   /* Loop over input grid cols */

			v_func[indf](GMT, Ctrl, Grid, body_desc, body_verts, x_grd, y_grd, cos_vec, row, col, 1, 1);	/* Call the body_set function */
			if (Ctrl->H.pirtt && fabs(body_verts[1].z - body_verts[0].z) < 1e-2) continue;

			if (Gsource)                                            /* If we have a variable source grid */
				rho_or_mag = Gsource->data[GMT_IJP(Gsource->header, row, col)];

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
				for (k = 0; k < Gout->header->ny; k++) {            /* Loop over output grid rows */
					y_o = (Ctrl->box.is_geog) ? (y_obs[k] + Ctrl->box.lat_0) * Ctrl->box.d_to_m : y_obs[k]; /* +lat_0 because y was already *= -1 */
					if (Ctrl->S.active) {
						DY = body_verts[0].y - y_o;
						if (fabs(DY) > Ctrl->S.radius) continue;    /* If outside the circumscribed square no need to continue */
						DY *= DY;                                   /* Square it right away */
					}

					if (Ctrl->box.is_geog) tmp = Ctrl->box.d_to_m * cos(y_obs[k]*D2R);
					for (i = 0; i < Gout->header->nx; i++) {        /* Loop over output grid cols */
						x_o = (Ctrl->box.is_geog) ? (x_obs[i] - Ctrl->box.lon_0) * tmp : x_obs[i];
						if (Ctrl->S.active) {
							DX = body_verts[0].x - x_o;             /* Use the first vertex to estimate distance. Approximate but good enough */
							if ((DX*DX + DY) > s_rad2) continue;    /* Remember that DY was already squared above */
						}
						a = d_func[indf](GMT, x_o, y_o, Ctrl->L.zobs, rho_or_mag, Ctrl->C.active, *body_desc, body_verts, km, pm, loc_or);
						Gout->data[GMT_IJP(Gout->header, k, i)] += (float)a;
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

	GMT_free (GMT, body_verts);
	if (Ctrl->H.do_igrf) {
		GMT_free (GMT, igrf_dip);		GMT_free (GMT, igrf_dec);
	}
}

/* -----------------------------------------------------------------------------------*/
void grdgravmag3d_calc_surf (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid, struct GMT_GRID *Gout,
		struct GMT_GRID *Gsource, double *g, unsigned int n_pts, double *x_grd, double *y_grd, double *x_grd_geo, double *y_grd_geo,
		double *x_obs, double *y_obs, double *cos_vec, struct MAG_VAR *mag_var, struct LOC_OR *loc_or,
		struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts) {

	/* Send g = NULL for grid computations (e.g. -G) or Gout = NULL otherwise (-F).
	   In case of polyline output (-F) n_pts is the number of output locations (irrelevant otherwise) */

	int i, indf;
	struct THREAD_STRUCT *threadArg;
#ifdef HAVE_GLIB_GTHREAD
	GThread **threads;
	if (GMT->common.x.n_threads > 1)
		threads = GMT_memory (GMT, NULL, GMT->common.x.n_threads, GThread *);
#endif

	GMT_tic(GMT);

	indf = (Ctrl->H.pirtt) ? 1 : 0;

	threadArg = GMT_memory (GMT, NULL, GMT->common.x.n_threads, struct THREAD_STRUCT);

	for (i = 0; i < GMT->common.x.n_threads; i++) {
		threadArg[i].GMT        = GMT;
		threadArg[i].Ctrl       = Ctrl;
		threadArg[i].Grid       = Grid;
		threadArg[i].Gout       = Gout;
		threadArg[i].Gsource    = Gsource;
   		threadArg[i].body_verts = body_verts;
   		threadArg[i].body_desc  = body_desc;
   		threadArg[i].mag_var    = mag_var;
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
   		threadArg[i].r_start    = i * irint((Grid->header->ny - 1 - indf) / GMT->common.x.n_threads);
   		threadArg[i].thread_num = i;

		if (GMT->common.x.n_threads == 1) {		/* Independently of WITH_THREADS, if only one don't call the threading machine */
   			threadArg[i].r_stop = Grid->header->ny - 1 + indf;
			grdgravmag3d_calc_surf_ (&threadArg[0]);
			break;		/* Make sure we don't go through the threads lines below */
		}
#ifndef HAVE_GLIB_GTHREAD
	}
#else
   		threadArg[i].r_stop = (i + 1) * irint((Grid->header->ny - 1 - indf) / GMT->common.x.n_threads);
   		if (i == GMT->common.x.n_threads - 1) threadArg[i].r_stop = Grid->header->ny - 1 + indf;	/* Make sure last row is not left behind */
		threads[i] = g_thread_new(NULL, thread_function, (void*)&(threadArg[i]));
	}

	if (GMT->common.x.n_threads > 1) {		/* Otherwise g_thread_new was never called aand so no need to "join" */
		for (i = 0; i < GMT->common.x.n_threads; i++)
			g_thread_join(threads[i]);
	}

	if (GMT->common.x.n_threads > 1)
		GMT_free (GMT, threads);
#endif

	GMT_free (GMT, threadArg);

	GMT_toc(GMT,"");
}

/* -----------------------------------------------------------------------------------*/
/* This is a modified version of the FORTRAN code that allows also a prism inclination
   and declination and Remanant + Inducec magnetization via a Koninsberg ratio. Those have
   not been ported into this C translation. Second analysis, the FORTRAN code did not actually
   consider different dec/dip for magnetization vector and Earth field, but here we do.
*/

double mprism (struct GMT_CTRL *GMT, double x_o, double y_o, double z_o, double mag, bool is_grav,
		struct BODY_DESC bd_desc, struct BODY_VERTS *body_verts, unsigned int km, unsigned int i_comp, struct LOC_OR *mag_par) {

	/* The MAG_PAR struct is used here to transmit the Ctrl->H members (components actually) */

	int i, j, k, ijk;
	double a[3][2], eps1, eps2, hx, hy, hz, tr, sc, xc, yc; 
	double xn, yn, f11, f12, f13, f21, f22, f23, u, v, w, r, c4, c5, c6;

	eps1 = 1.0e-12;
	eps2 = 5.0e-3;

	a[0][0] = (body_verts[1].y - body_verts[0].y) / 2;		// thickness (arbitrary, could also be the 'length')
	a[0][1] = -a[0][0];
	a[1][0] = (body_verts[1].x - body_verts[0].x) / 2;		// length
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

double bhatta (struct GMT_CTRL *GMT, double x_o, double y_o, double z_o, double mag, bool is_grav,
		struct BODY_DESC bd_desc, struct BODY_VERTS *body_verts, unsigned int km, unsigned int i_comp, struct LOC_OR *loc_or) {

	/* x_o, y_o, z_o are the coordinates of the observation point
 	 * mag is the body magnetization in A/m
 	 * km is not used here.
 	 * i_comp: index to which component function to compute: 0 -> EW comp; 1 -> NS comp; 2 -> Z component
 	 */

	double x111, x011, x101, x001, x110, x010, x100, x000, u0, u1, v0, v1, w0, w1, tx;
	double (*d_func[3])(double, double, double, double, double, double);       /* function pointer */

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

double nucleox(double u, double v, double w, double rl, double rm, double rn) {
	double r, t1, t2, t3, rnum, rden;
	r = sqrt(u*u + v*v + w*w);
	t1 = rn / 2.0 * log((r+v)/(r-v));
	t2 = rm * log(r+w);
	rnum = u*v;
	rden = u*u + w*w + r*w;
	t3 = rl * atan2(rnum,rden);
	return (t1 + t2 + t3);
}

double nucleoy(double u, double v, double w, double rl, double rm, double rn) {
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

double nucleoz(double u, double v, double w, double rl, double rm, double rn) {
	double r, t1, t2, t3, rnum, rden;
	r = sqrt(u*u + v*v + w*w);
	t1 = rm / 2.0 * log((r+u)/(r-u));
	t2 = rl / 2.0 * log((r+v)/(r-v));
	rnum = u*v;
	rden = r*w;
	t3 = -rn * atan2(rnum,rden);
	return (t1 + t2 + t3);
}

void dircos(double incl, double decl, double azim, double *a, double *b, double *c) {
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

