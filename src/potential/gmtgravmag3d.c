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
 *--------------------------------------------------------------------*/
/*
 * API functions to support the gmtgravmag3d application.
 *
 * Brief synopsis:
 *
 *
 * Author:	Joaquim Luis
 * Date:	06-OCT-2000 (original GMT 4)
 *
 */

#define THIS_MODULE_NAME	"gmtgravmag3d"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Compute the gravity/magnetic anomaly of a body by the method of Okabe"
#define THIS_MODULE_KEYS	"<DI,FDi,GGO"

#include "gmt_dev.h"
#include "okbfuns.h"

#define GMT_PROG_OPTIONS "-:RVf"

struct XYZOKB_CTRL {
	struct XYZOKB_C {	/* -C */
		bool active;
		double rho;
	} C;
	struct XYZOKB_D {	/* -D */
		bool active;
		double dir;
	} D;
	struct XYZOKB_I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct XYZOKB_F {	/* -F<grdfile> */
		bool active;
		char *file;
	} F;
	struct XYZOKB_G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct XYZOKB_H {	/* -H */
		bool active;
		double	t_dec, t_dip, m_int, m_dec, m_dip;
	} H;
	struct XYZOKB_L {	/* -L */
		bool active;
		double zobs;
	} L;
	struct XYZOKB_E {	/* -T */
		bool active;
		double dz;
	} E;
	struct XYZOKB_S {	/* -S */
		bool active;
		double radius;
	} S;
	struct XYZOKB_Z {	/* -Z */
		double z0;
	} Z;
	struct XYZOKB_T {	/* -T */
		bool active;
		bool triangulate;
		bool raw;
		bool stl;
		bool m_var, m_var1, m_var2, m_var3, m_var4;
		char *xyz_file;
		char *t_file;
		char *raw_file;
		char *stl_file;
	} T;
	struct XYZOKB_box {	/* No option, just a container */
		bool is_geog;
		double	d_to_m, *mag_int, lon_0, lat_0;
	} box;
};

struct DATA {
	double  x, y;
} *data;

struct TRIANG {
	double  x, y, z;
} *triang;

struct  VERT {
	unsigned int  a, b, c;
} *vert;

struct  TRI_CENTER {
	double  x, y, z;
} *t_center;

struct RAW {
	double  t1[3], t2[3], t3[3];
} *raw_mesh;

struct MAG_VAR2 {
	double	m, m_dip;
} *mag_var2;

struct MAG_VAR3 {
	double	m, m_dec, m_dip;
} *mag_var3;

struct MAG_VAR4 {
	double	t_dec, t_dip, m, m_dec, m_dip;
} *mag_var4;

void *New_gmtgravmag3d_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct XYZOKB_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct XYZOKB_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->L.zobs = 0;
	C->D.dir = -1;
	C->S.radius = 50000;
	return (C);
}

void Free_gmtgravmag3d_Ctrl (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->F.file) free (C->F.file);
	if (C->G.file) free (C->G.file);
	if (C->T.xyz_file) free (C->T.xyz_file);
	if (C->T.t_file) free (C->T.t_file);
	if (C->T.raw_file) free (C->T.raw_file);
	if (C->T.stl_file) free (C->T.stl_file);
	
	GMT_free (GMT, C);
}

int read_xyz (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *Ctrl, char *fname, double *lon_0, double *lat_0);
int read_t (struct GMT_CTRL *GMT, char *fname);
int read_raw (struct GMT_CTRL *GMT, char *fname, double z_dir);
int read_stl (struct GMT_CTRL *GMT, char *fname, double z_dir);
int read_poly (struct GMT_CTRL *GMT, char *fname, bool switch_xy);
void set_center (unsigned int n_triang);
int facet_triangulate (struct XYZOKB_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool bat);
int facet_raw (struct XYZOKB_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool geo);
int check_triang_cw (unsigned int n, unsigned int type);

int GMT_gmtgravmag3d_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtgravmag3d [-C<density>] [-G<outgrid>] [%s]\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-E<thick>] [-F<xy_file>] [-L<z_observation>]\n", GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-H<f_dec>/<f_dip>/<m_int></m_dec>/<m_dip>] [-S<radius>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T<[d]xyz_file>/<vert_file>[/m]|<[r|s]raw_file> [-Z<level>] [%s] [-fg] [%s]\n", 
		GMT_V_OPT, GMT_r_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-H sets parameters for computation of magnetic anomaly\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f_dec/f_dip -> geomagnetic declination/inclination\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m_int/m_dec/m_dip -> body magnetic intensity/declination/inclination\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C sets body density in SI\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F pass locations where anomaly is going to be computed\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G name of the output grdfile.\n");
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t-L sets level of observation [Default = 0]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E give layer thickness in m [Default = 0 m]\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S search radius in km\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Give either names of xyz[m] and vertex files or of a file defining a close surface.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   In the first case append an 'd' imediatly after -T and optionaly a /m after the vertex file name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   In the second case append an 'r' or a 's' imediatly after -T and before the file name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   'r' and 's' stand for files in raw (x1 y1 z1 x2 ... z3) or STL format.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z z level of reference plane [Default = 0]\n");
	GMT_Option (API, "bi");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Convert geographic grids to meters using a \"Flat Earth\" approximation.\n");
	GMT_Option (API, "r,:,.");

	return (EXIT_FAILURE);
}

int GMT_gmtgravmag3d_parse (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to gmtgravmag3d and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int j, pos = 0, n_errors = 0, n_files = 0;
	char	ptr[GMT_LEN256] = {""};
	struct	GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'B':	/* For backward compat (Undocumented) */
			case 'H':
				if ((sscanf(opt->arg, "%lf/%lf/%lf/%lf/%lf",
					    &Ctrl->H.t_dec, &Ctrl->H.t_dip, &Ctrl->H.m_int, &Ctrl->H.m_dec, &Ctrl->H.m_dip)) != 5) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -H option: Can't dechiper values\n");
					n_errors++;
				}
				Ctrl->H.active = true;
				Ctrl->C.active = false;
				break;
			case 'C':
				Ctrl->C.rho = atof (opt->arg) * 6.674e-6;
				Ctrl->C.active = true;
				Ctrl->H.active = false;
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.dir = 1;
				break;
			case 'F':
				Ctrl->F.active = true;
				Ctrl->F.file = strdup (opt->arg);
				break;
			case 'G':
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':
				Ctrl->L.active = true;
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
	 		case 'P':		/* For backward compat of pre GMT version */
	 		case 'E':
				Ctrl->E.dz = atof (opt->arg);
				Ctrl->E.active = true;
				break;
	 		case 'S':
				Ctrl->S.radius = atof (opt->arg) * 1000;
				Ctrl->S.active = true;
				break;
	 		case 't':		/* For backward compat of pre GMT version */
			case 'T': 		/* Selected input mesh format */
				switch (opt->arg[0]) {
					case 'd':	/* Surface computed by triangulate */
						j = 0;
						while (GMT_strtok (&opt->arg[1], "/", &pos, ptr)) {
							switch (j) {
								case 0:
									Ctrl->T.xyz_file = strdup(ptr);
									break;
								case 1:
									Ctrl->T.t_file = strdup(ptr);
									break;
								case 2:
									Ctrl->T.m_var = true;
									Ctrl->H.active = true;
									Ctrl->C.active = false;
									if (ptr[1] == '2') Ctrl->T.m_var2 = true;
									else if (ptr[1] == '3') Ctrl->T.m_var3 = true;
									else if (ptr[1] == '4') Ctrl->T.m_var4 = true;
									else Ctrl->T.m_var1 = true;
									break;
								default:
									break;
							}
							j++;
						}
						if (j != 2 && j != 3) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Must give names for data points and vertex files\n");
							n_errors++;
						}
						Ctrl->T.triangulate = true;
						break;
					case 'r':	/* Closed volume in RAW format */
 						Ctrl->T.raw_file = strdup(&opt->arg[1]);
						Ctrl->T.raw = true;
						break;
					case 's':	/* Closed volume in STL1format */
 						Ctrl->T.stl_file = strdup(&opt->arg[1]);
						Ctrl->T.stl = true;
						break;
					default:
						n_errors++;
						break;
				}
				break;
			case 'Z':
				Ctrl->Z.z0 = atof(opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->S.active && (Ctrl->S.radius <= 0.0 || GMT_is_dnan (Ctrl->S.radius)),
					 "Syntax error: Radius is NaN or negative\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.active && !Ctrl->F.active , "Error: Must specify either -G or -F options\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.active && !Ctrl->I.active , "Error: Must specify -I option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.active && !GMT->common.R.active, "Error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.rho == 0.0 && !Ctrl->H.active && !Ctrl->T.m_var4 ,
					"Error: Must specify either -Cdensity or -H<stuff>\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.active && !Ctrl->G.file, "Syntax error -G option: Must specify output file\n");
	j = GMT_check_condition (GMT, Ctrl->G.active && Ctrl->F.active, "Warning: -F overrides -G\n");
	if (GMT_check_condition (GMT, Ctrl->T.raw && Ctrl->S.active, "Warning: -Tr overrides -S\n"))
		Ctrl->S.active = false;

	/*n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");*/

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtgravmag3d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtgravmag3d (void *V_API, int mode, void *args) {

	bool bat = true, switch_xy = false, DO = true;
	unsigned int row, col, i, j, k, kk, ndata_r = 0;
	unsigned int ndata_p = 0, ndata_t = 0, nx_p, ny_p, n_vert_max;
	unsigned int z_th = 0, n_triang = 0, ndata_s = 0, n_swap = 0;
	int retval, error = 0;
	uint64_t ij;
	size_t nm;
	int km, pm;		/* index of current body facet (for mag only) */
	float	*g = NULL, one_100;
	double	s_rad2, x_o, y_o, t_mag, a, DX, DY;
	double	*x_obs = NULL, *y_obs = NULL, *z_obs = NULL, *x = NULL, *y = NULL, *cos_vec = NULL;
	double	cc_t, cs_t, s_t, lon_0 = 0, lat_0 = 0;

	struct	LOC_OR *loc_or = NULL;
	struct	BODY_VERTS *body_verts = NULL;
	struct	BODY_DESC body_desc;
	struct	XYZOKB_CTRL *Ctrl = NULL;
	struct	GMT_GRID *Gout = NULL;
	struct	GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct	GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	data = NULL, triang = NULL, vert = NULL, t_center = NULL, raw_mesh = NULL, mag_param = NULL;
	mag_var = NULL, mag_var2 = NULL, mag_var3 = NULL, mag_var4 = NULL;
	body_desc.n_v = NULL, body_desc.ind = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_gmtgravmag3d_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE)
		bailout (GMT_gmtgravmag3d_usage (API, GMT_USAGE));		/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS)
		bailout (GMT_gmtgravmag3d_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_gmtgravmag3d_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtgravmag3d_parse (GMT, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the gmtgravmag3d main code ----------------------------*/
	
	if (GMT_is_geographic (GMT, GMT_IN)) Ctrl->box.is_geog = true;

	if (!Ctrl->box.is_geog)
		Ctrl->box.d_to_m = 1;
	else
		Ctrl->box.d_to_m = 2 * M_PI * 6371008.7714 / 360.0;
		/*Ctrl->box.d_to_m = 2.0 * M_PI * gmtdefs.ellipse[N_ELLIPSOIDS-1].eq_radius / 360.0;*/

	Ctrl->Z.z0 *= Ctrl->D.dir;

	/* ---- Read files section ---------------------------------------------------- */
	if (Ctrl->F.active) { 		/* Read xy file where anomaly is to be computed */
		if ( (retval = read_poly (GMT, Ctrl->F.file, switch_xy)) < 0 ) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open file %s\n", Ctrl->F.file);
			return (EXIT_FAILURE);
		}
		ndata_p = retval;
	}

	if (Ctrl->T.triangulate) { 	/* Read triangle file output from triangulate */
		if ( (retval = read_xyz (GMT, Ctrl, Ctrl->T.xyz_file, &lon_0, &lat_0)) < 0 ) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open file %s\n", Ctrl->T.xyz_file);
			return (EXIT_FAILURE);
		}
		/* read vertex file */
		if ( (retval = read_t (GMT, Ctrl->T.t_file)) < 0 ) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open file %s\n", Ctrl->T.t_file);
			return (EXIT_FAILURE);
		}
		ndata_t = retval;

		t_center = GMT_memory (GMT, NULL, ndata_t, struct TRI_CENTER);
		/* compute aproximate center of each triangle */
		n_swap = check_triang_cw (ndata_t, 0);
		set_center (ndata_t);
	}
	else if (Ctrl->T.stl) { 	/* Read STL file defining a closed volume */
		if ( (retval = read_stl (GMT, Ctrl->T.stl_file, Ctrl->D.dir)) < 0 ) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open file %s\n", Ctrl->T.stl_file);
			return (EXIT_FAILURE);
		}
		ndata_s = retval;
		/*n_swap = check_triang_cw (ndata_s, 1);*/
	}
	else if (Ctrl->T.raw) { 	/* Read RAW file defining a closed volume */
		if ( (retval = read_raw (GMT, Ctrl->T.raw_file, Ctrl->D.dir)) < 0 ) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open file %s\n", Ctrl->T.raw_file);
			return (EXIT_FAILURE);
		}
		ndata_r = retval;
		/*n_swap = check_triang_cw (ndata_r, 1);*/
	}

	if (n_swap > 0)
		GMT_Report (API, GMT_MSG_VERBOSE, "Warning: %d triangles had ccw order\n", n_swap);
/* ---------------------------------------------------------------------------- */

	if (Ctrl->G.active) {
		if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, NULL, Ctrl->I.inc, \
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, Ctrl->G.file)) == NULL) Return (API->error);
	
		GMT_Report (API, GMT_MSG_VERBOSE, "Grid dimensions are nx = %d, ny = %d\n", Gout->header->nx, Gout->header->ny);

		/* Build observation point vectors */
		x = GMT_memory (GMT, NULL, Gout->header->nx, double);
		y = GMT_memory (GMT, NULL, Gout->header->ny, double);
		for (i = 0; i < Gout->header->nx; i++)
			x[i] = (i == (Gout->header->nx-1)) ? Gout->header->wesn[XHI] :
				(Gout->header->wesn[XLO] + i * Gout->header->inc[GMT_X]);
		for (j = 0; j < Gout->header->ny; j++)
			y[j] = (j == (Gout->header->ny-1)) ? -Gout->header->wesn[YLO] :
				-(Gout->header->wesn[YHI] - j * Gout->header->inc[GMT_Y]);
	}

	nx_p = (!Ctrl->F.active) ? Gout->header->nx : ndata_p;
	ny_p = (!Ctrl->F.active) ? Gout->header->ny : ndata_p;
	nm   = (!Ctrl->F.active) ? Gout->header->nm : ndata_p;
	x_obs = GMT_memory (GMT, NULL, nx_p, double);
	y_obs = GMT_memory (GMT, NULL, ny_p, double);
	z_obs = GMT_memory (GMT, NULL, nx_p, double);
	body_verts = GMT_memory (GMT, NULL, 18, struct BODY_VERTS);

	if (Ctrl->F.active) { /* Need to compute observation coords only once */
		for (i = 0; i < ndata_p; i++) {
			x_obs[i] = (Ctrl->box.is_geog) ? (data[i].x-lon_0)*Ctrl->box.d_to_m*cos(data[i].y*D2R) : data[i].x;
			y_obs[i] = (Ctrl->box.is_geog) ? -(data[i].y-lat_0)*Ctrl->box.d_to_m : data[i].y; /* - because y positive 'south' */
		}
		g = GMT_memory (GMT, NULL, nm, float);
	}

	if (Ctrl->T.triangulate) {
		n_triang = ndata_t;
		body_desc.n_f = 5;		/* Number of prism facets */
		body_desc.n_v = GMT_memory (GMT, NULL, body_desc.n_f, unsigned int);
		body_desc.n_v[0] = 3;	body_desc.n_v[1] = 3;
		body_desc.n_v[2] = 4;	body_desc.n_v[3] = 4;
		body_desc.n_v[4] = 4;
		body_desc.ind = GMT_memory (GMT, NULL, (body_desc.n_v[0] + body_desc.n_v[1] +
			body_desc.n_v[2] + body_desc.n_v[3] + body_desc.n_v[4]), unsigned int);
		body_desc.ind[0] = 0;	body_desc.ind[1] = 1; 	body_desc.ind[2] = 2;	/* top triang */
		body_desc.ind[3] = 3;	body_desc.ind[4] = 5; 	body_desc.ind[5] = 4;	/* bot triang */
		body_desc.ind[6] = 1;	body_desc.ind[7] = 4; 	body_desc.ind[8] = 5;	body_desc.ind[9] = 2;
		body_desc.ind[10] = 0;	body_desc.ind[11] = 3;	body_desc.ind[12] = 4;	body_desc.ind[13] = 1;
	 	body_desc.ind[14] = 0;	body_desc.ind[15] = 2;	body_desc.ind[16] = 5;	body_desc.ind[17] = 3;

	 	/* Actually, for the gravity case we can save lots of computations because the flux
	 	   through the vertical walls does not contibute to gravity anomaly, which is vertical */
		if (Ctrl->C.active)
			body_desc.n_f = 2;		/* Number of prism facets that count */
	}
	else if (Ctrl->T.raw || Ctrl->T.stl) {
		n_triang = (Ctrl->T.raw) ? ndata_r : ndata_s;
		body_desc.n_f = 1;
		body_desc.n_v = GMT_memory (GMT, NULL, body_desc.n_f, unsigned int);
		body_desc.n_v[0] = 3;
		body_desc.ind = GMT_memory (GMT, NULL, body_desc.n_v[0], unsigned int);
		body_desc.ind[0] = 0;	body_desc.ind[1] = 1; 	body_desc.ind[2] = 2;
	}
	else {
		GMT_Report (API, GMT_MSG_NORMAL, "It shouldn't pass here\n");
		return (EXIT_FAILURE); /* should not happen but just in case */
	}

	/* Allocate a structure that will be used inside okabe().
	   We do it here to avoid thousands of alloc/free that would result if done in okabe() */
	n_vert_max = body_desc.n_v[0];
	for (i = 1; i < body_desc.n_f; i++)
		n_vert_max = MAX(body_desc.n_v[i], n_vert_max);

	loc_or = GMT_memory (GMT, NULL, (n_vert_max+1), struct LOC_OR);

	if (Ctrl->H.active) { /* 1e2 is a factor to obtain nT from magnetization in A/m */
		cc_t = cos(Ctrl->H.m_dip*D2R)*cos((Ctrl->H.m_dec - 90.)*D2R);
		cs_t = cos(Ctrl->H.m_dip*D2R)*sin((Ctrl->H.m_dec - 90.)*D2R);
		s_t = sin(Ctrl->H.m_dip*D2R);
		if (!Ctrl->T.m_var4) {		/* In all the other cases the field parameters are constatnt */
			mag_param = GMT_memory (GMT, NULL, 1, struct MAG_PARAM);
			mag_param[0].rim[0] = 1e2*cos(Ctrl->H.t_dip*D2R) * cos((Ctrl->H.t_dec - 90.)*D2R);
			mag_param[0].rim[1] = 1e2*cos(Ctrl->H.t_dip*D2R) * sin((Ctrl->H.t_dec - 90.)*D2R);
			mag_param[0].rim[2] = 1e2*sin(Ctrl->H.t_dip*D2R);
		}
		if (!Ctrl->T.m_var) { /* Case of constant magnetization */
			mag_var = GMT_memory (GMT, NULL, 1, struct MAG_VAR);
			mag_var[0].rk[0] = Ctrl->H.m_int * cc_t;
			mag_var[0].rk[1] = Ctrl->H.m_int * cs_t;
			mag_var[0].rk[2] = Ctrl->H.m_int * s_t;
		}
		else { /* The triangles have a non-constant magnetization */
			mag_var = GMT_memory (GMT, NULL, n_triang, struct MAG_VAR);
			if (Ctrl->T.m_var1) {		/* Only the mag intensity changes. Mag dec & dip are constant */
				for (i = 0; i < n_triang; i++) {
					t_mag = (Ctrl->box.mag_int[vert[i].a] + Ctrl->box.mag_int[vert[i].b] + Ctrl->box.mag_int[vert[i].c])/3.;
					mag_var[i].rk[0] = t_mag * cc_t;
					mag_var[i].rk[1] = t_mag * cs_t;
					mag_var[i].rk[2] = t_mag * s_t;
				}
			}
			else if (Ctrl->T.m_var2) {	/* Both mag intensity & dip varies. Dec is Zero (axial dipole) */
				for (i = 0; i < n_triang; i++) {
					t_mag = (mag_var2[vert[i].a].m + mag_var2[vert[i].b].m + mag_var2[vert[i].c].m)/3.;
					Ctrl->H.t_dip = (mag_var2[vert[i].a].m_dip + mag_var2[vert[i].b].m_dip + mag_var2[vert[i].c].m_dip)/3.;
					mag_var[i].rk[0] = 0.;
					mag_var[i].rk[1] = -t_mag * cos(Ctrl->H.t_dip*D2R);
					mag_var[i].rk[2] = t_mag * sin(Ctrl->H.t_dip*D2R);
				}
			}
			else if (Ctrl->T.m_var3) { 	/* Both mag intensity, mag_dec & mag_dip varies. */
				for (i = 0; i < n_triang; i++) {
					t_mag = (mag_var3[vert[i].a].m + mag_var3[vert[i].b].m + mag_var3[vert[i].c].m)/3.;
					Ctrl->H.t_dec = (mag_var3[vert[i].a].m_dec + mag_var3[vert[i].b].m_dec + mag_var3[vert[i].c].m_dec)/3.;
					Ctrl->H.t_dip = (mag_var3[vert[i].a].m_dip + mag_var3[vert[i].b].m_dip + mag_var3[vert[i].c].m_dip)/3.;
					mag_var[i].rk[0] = t_mag * cos(Ctrl->H.t_dip*D2R) * cos((Ctrl->H.t_dec - 90)*D2R);
					mag_var[i].rk[1] = t_mag * cos(Ctrl->H.t_dip*D2R) * sin((Ctrl->H.t_dec - 90)*D2R);
					mag_var[i].rk[2] = t_mag * sin(Ctrl->H.t_dip*D2R);
				}
			}
			else {			/* Everything varies. */
				mag_param = GMT_memory (GMT, NULL, n_triang, struct MAG_PARAM);
				for (i = 0; i < n_triang; i++) {
					Ctrl->H.t_dec = (mag_var4[vert[i].a].t_dec + mag_var4[vert[i].b].t_dec + mag_var4[vert[i].c].t_dec)/3.;
					Ctrl->H.t_dip = (mag_var4[vert[i].a].t_dip + mag_var4[vert[i].b].t_dip + mag_var4[vert[i].c].t_dip)/3.;
					mag_param[i].rim[0] = 1e2*cos(Ctrl->H.t_dip*D2R) * cos((Ctrl->H.t_dec - 90.)*D2R);
					mag_param[i].rim[1] = 1e2*cos(Ctrl->H.t_dip*D2R) * sin((Ctrl->H.t_dec - 90.)*D2R);
					mag_param[i].rim[2] = 1e2*sin(Ctrl->H.t_dip*D2R);
					t_mag = (mag_var4[vert[i].a].m + mag_var4[vert[i].b].m + mag_var4[vert[i].c].m)/3.;
					Ctrl->H.t_dec = (mag_var4[vert[i].a].m_dec + mag_var4[vert[i].b].m_dec + mag_var4[vert[i].c].m_dec)/3.;
					Ctrl->H.t_dip = (mag_var4[vert[i].a].m_dip + mag_var4[vert[i].b].m_dip + mag_var4[vert[i].c].m_dip)/3.;
					mag_var[i].rk[0] = t_mag * cos(Ctrl->H.t_dip*D2R) * cos((Ctrl->H.t_dec - 90)*D2R);
					mag_var[i].rk[1] = t_mag * cos(Ctrl->H.t_dip*D2R) * sin((Ctrl->H.t_dec - 90)*D2R);
					mag_var[i].rk[2] = t_mag * sin(Ctrl->H.t_dip*D2R);
				}
			}
		}
	}

/* ---------------> Now start computing <------------------------------------- */
	one_100 = (float)(n_triang / 100.);
	s_rad2 = Ctrl->S.radius*Ctrl->S.radius;

	if (Ctrl->G.active) {		/* Compute the cos(lat) vector only once */
		cos_vec = GMT_memory (GMT, NULL, Gout->header->ny, double);
		for (i = 0; i < Gout->header->ny; i++)
			cos_vec[i] = (Ctrl->box.is_geog) ? cos(y[i]*D2R): 1;
	}

	for (i = j = 0; i < n_triang; i++) {		/* Main loop over all the triangles */
		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE) && i > j*one_100) {
			GMT_Message (API, GMT_TIME_NONE, "computed %.2d%s of %d prisms\r", j, "%", n_triang);
			j++;
		}
		km = (int)((Ctrl->T.m_var)  ? i : 0);	/* Variable magnetization (intensity) */
		pm = (int)((Ctrl->T.m_var4) ? i : 0);	/* When al 5 paremeters (F, Mag) may be variable (undocumented) */

		/* Don't waste time with zero mag triangles */
		if (Ctrl->H.active && Ctrl->T.m_var && mag_var[i].rk[0] == 0 && mag_var[i].rk[1] == 0 && mag_var[i].rk[2] == 0)
			continue;
		if (Ctrl->T.triangulate)
			z_th = facet_triangulate (Ctrl, body_verts, i, bat);
		else if (Ctrl->T.raw || Ctrl->T.stl)
			z_th = facet_raw (Ctrl, body_verts, i, Ctrl->box.is_geog);
		if (z_th) {
			if (Ctrl->G.active) { /* grid */
				for (row = 0; row < Gout->header->ny; row++) {
					y_o = (Ctrl->box.is_geog) ? ((y[row]+lat_0) * Ctrl->box.d_to_m): y[row];
					ij = GMT_IJP(Gout->header, row, 0);
					for (col = 0; col < Gout->header->nx; col++, ij++) {
						x_o = (Ctrl->box.is_geog) ? ((x[col]-lon_0)*Ctrl->box.d_to_m * cos_vec[row]) : x[col];
						if (Ctrl->S.active) {
							DX = t_center[i].x - x_o;
							DY = t_center[i].y - y_o;
							DO = (DX*DX + DY*DY) < s_rad2;
							if (!DO) continue;
						}

						a = okabe (GMT, x_o, y_o, Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, body_desc, body_verts, km, pm, loc_or);
						Gout->data[ij] += (float)a;
					}
				}
			}
			else {		/* polygon */
				for (kk = 0; kk < ndata_p; kk++){
					if (Ctrl->S.active) {
						DX = t_center[i].x - x_obs[kk];
						DY = t_center[i].y - y_obs[kk];
						DO = (DX*DX + DY*DY) < s_rad2;
						if (!DO) continue;
					}
					a = okabe (GMT, x_obs[kk], y_obs[kk], Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, body_desc, body_verts, km, pm, loc_or);
					g[kk] += (float)a;
				}
			}
		}
	}	

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
		char save[GMT_LEN64] = {""};
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
			Return (API->error);
		}
		if ((error = GMT_set_cols (GMT, GMT_OUT, 3)) != GMT_OK) {
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		strcpy (save, GMT->current.setting.format_float_out);
		strcpy (GMT->current.setting.format_float_out, "%.9g");	/* Make sure we use enough decimals */
		for (k = 0; k < ndata_p; k++) {
			out[GMT_X] = data[k].x;
			out[GMT_Y] = data[k].y;
			out[GMT_Z] = g[k];
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
		}
		strcpy (GMT->current.setting.format_float_out, save);
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
	}

	if (x) GMT_free (GMT, x);
	if (y) GMT_free (GMT, y);
	if (g) GMT_free (GMT, g);
	GMT_free (GMT, z_obs);
	GMT_free (GMT, x_obs);
	GMT_free (GMT, y_obs);
	if (data) GMT_free (GMT, data);
	if (triang) GMT_free (GMT, triang);
	if (raw_mesh) GMT_free (GMT, raw_mesh);
	if (t_center) GMT_free (GMT, t_center);
	if (vert) GMT_free (GMT, vert);
	if (mag_param) GMT_free (GMT, mag_param);
	if (mag_var) GMT_free (GMT, mag_var);
	GMT_free (GMT, body_desc.n_v);
	GMT_free (GMT, body_desc.ind);
	GMT_free (GMT, loc_or);
	GMT_free (GMT, body_verts);
	if (cos_vec) GMT_free (GMT, cos_vec);
	if (Ctrl->T.m_var1) GMT_free (GMT, Ctrl->box.mag_int);
	if (Ctrl->T.m_var2) GMT_free (GMT, mag_var2);
	if (Ctrl->T.m_var3) GMT_free (GMT, mag_var3);
	if (mag_var4) GMT_free (GMT, mag_var4);

	Return (GMT_OK);
}

/* -------------------------------------------------------------------------*/
int read_xyz (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *Ctrl, char *fname, double *lon_0, double *lat_0) {
	/* read xyz[m] file with point data coordinates */

	unsigned int ndata_xyz;
	size_t n_alloc;
	float x_min = FLT_MAX, x_max = -FLT_MAX, y_min = FLT_MAX, y_max = -FLT_MAX;
	double in[8];
	char line[GMT_LEN256] = {""};
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

       	n_alloc = GMT_CHUNK;
	ndata_xyz = 0;
	*lon_0 = 0.;	*lat_0 = 0.;
        triang = GMT_memory (GMT, NULL, n_alloc, struct TRIANG);
	if (Ctrl->T.m_var1)
        	Ctrl->box.mag_int = GMT_memory (GMT, NULL, n_alloc, double);
	else if (Ctrl->T.m_var2)
        	mag_var2 = GMT_memory (GMT, NULL, n_alloc, struct MAG_VAR2);
	else if (Ctrl->T.m_var3)
        	mag_var3 = GMT_memory (GMT, NULL, n_alloc, struct MAG_VAR3);
	else if (Ctrl->T.m_var4)
        	mag_var4 = GMT_memory (GMT, NULL, n_alloc, struct MAG_VAR4);
	
	if (Ctrl->box.is_geog) {	/* take a first read just to compute the central longitude */
		while (fgets (line, GMT_LEN256, fp)) {
			sscanf (line, "%lg %lg", &in[0], &in[1]); /* A test on file integrity will be done bellow */
			x_min = (float)MIN(in[0], x_min);	x_max = (float)MAX(in[0], x_max);
			y_min = (float)MIN(in[1], y_min);	y_max = (float)MAX(in[1], y_max);
		}
		*lon_0 = (x_min + x_max) / 2;
		*lat_0  = (y_min + y_max) / 2;
		rewind(fp);
	}

	while (fgets (line, GMT_LEN256, fp)) {
		if (!Ctrl->T.m_var) {
			if(sscanf (line, "%lg %lg %lg", &in[0], &in[1], &in[2]) !=3)
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering line %d of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else if (Ctrl->T.m_var1) { /* data file has 4 columns and the last contains magnetization */
			if(sscanf (line, "%lg %lg %lg %lg", &in[0], &in[1], &in[2], &in[3]) !=4)
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering line %d of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else if (Ctrl->T.m_var2) { /* data file has 5 columns: x,y,z,mag,dip */
			if(sscanf (line, "%lg %lg %lg %lg %lg", &in[0], &in[1], &in[2], &in[3], &in[4]) !=5)
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering line %d of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else if (Ctrl->T.m_var3) { /* data file has 6 columns: x,y,z,mag_int,mag_dec,mag_dip */
			if(sscanf (line, "%lg %lg %lg %lg %lg %lg", &in[0], &in[1], &in[2], &in[3], &in[4], &in[5]) !=6)
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering line %d of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else {		/* data file has 8 columns: x,y,z,field_dec,field_dip,mag_int,mag_dec,mag_dip */
			if(sscanf (line, "%lg %lg %lg %lg %lg %lg %lg %lg",
				   &in[0], &in[1], &in[2], &in[3], &in[4], &in[5], &in[6], &in[7]) !=8)
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering line %d of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		if (ndata_xyz == n_alloc) {
			n_alloc <<= 1;
			triang = GMT_memory (GMT, triang, n_alloc, struct TRIANG);
			if (Ctrl->T.m_var1)
				Ctrl->box.mag_int = GMT_memory (GMT, Ctrl->box.mag_int, n_alloc, double);
			else if (Ctrl->T.m_var2)
				mag_var2 = GMT_memory (GMT, mag_var2, n_alloc, struct MAG_VAR2);
			else if (Ctrl->T.m_var3)
				mag_var3 = GMT_memory (GMT, mag_var3, n_alloc, struct MAG_VAR3);
			else
				mag_var4 = GMT_memory (GMT, mag_var4, n_alloc, struct MAG_VAR4);
		}
		triang[ndata_xyz].x = (Ctrl->box.is_geog) ? (in[0] - *lon_0) * Ctrl->box.d_to_m * cos(in[1]*D2R) : in[0];
		triang[ndata_xyz].y = (Ctrl->box.is_geog) ? -(in[1] - *lat_0) * Ctrl->box.d_to_m : -in[1]; /* - because y must be positive 'south'*/
		triang[ndata_xyz].z = in[2] * Ctrl->D.dir;
		if (Ctrl->T.m_var1)
			Ctrl->box.mag_int[ndata_xyz] = in[3];
		else if (Ctrl->T.m_var2) {
			mag_var2[ndata_xyz].m = in[3];
			mag_var2[ndata_xyz].m_dip = in[4];
		}
		else if (Ctrl->T.m_var3) {
			mag_var3[ndata_xyz].m = in[3];
			mag_var3[ndata_xyz].m_dec = in[4];
			mag_var3[ndata_xyz].m_dip = in[5];
		}
		else if (Ctrl->T.m_var4) {
			mag_var4[ndata_xyz].t_dec = in[3];
			mag_var4[ndata_xyz].t_dip = in[4];
			mag_var4[ndata_xyz].m = in[5];
			mag_var4[ndata_xyz].m_dec = in[6];
			mag_var4[ndata_xyz].m_dip = in[7];
		}
		ndata_xyz++;
	}
	fclose(fp);
	return (ndata_xyz);
}

/* -----------------------------------------------------------------*/
int read_t (struct GMT_CTRL *GMT, char *fname) {
	/* read file with vertex indexes of triangles */
	unsigned int ndata_t;
	size_t n_alloc;
	int in[3];
	char line[GMT_LEN256] = {""};
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata_t = 0;
	vert = GMT_memory (GMT, NULL, n_alloc, struct VERT);
	
	while (fgets (line, GMT_LEN256, fp)) {
		if (sscanf (line, "%d %d %d", &in[0], &in[1], &in[2]) !=3)
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering line %d of %s\n", ndata_t+1, fname);
		if (ndata_t == n_alloc) {
			n_alloc <<= 1;
			vert = GMT_memory (GMT, vert, n_alloc, struct VERT);
               	}
		if (in[0] < 0 || in[1] < 0 || in[2] < 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Negative indices for line %d of %s\n", ndata_t+1, fname);
	
		}
		vert[ndata_t].a = in[0];
		vert[ndata_t].b = in[1];
		vert[ndata_t].c = in[2];
		ndata_t++;
	}
	fclose(fp);
	return (ndata_t);
}

/* -----------------------------------------------------------------*/
int read_raw (struct GMT_CTRL *GMT, char *fname, double z_dir) {
	/* read a file with triagles in the raw format and returns nb of triangles */
	unsigned int ndata_r;
	size_t n_alloc;
	double in[9];
	char line[GMT_LEN256] = {""};
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata_r = 0;
	raw_mesh = GMT_memory (GMT, NULL, n_alloc, struct RAW);
	
	while (fgets (line, GMT_LEN256, fp)) {
		if(sscanf (line, "%lg %lg %lg %lg %lg %lg %lg %lg %lg",
			   &in[0], &in[1], &in[2], &in[3], &in[4], &in[5], &in[6], &in[7], &in[8]) !=9)
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering line %d of %s\n", ndata_r+1, fname);
              	if (ndata_r == n_alloc) {
			n_alloc <<= 1;
			raw_mesh = GMT_memory (GMT, raw_mesh, n_alloc, struct RAW);
		}
		raw_mesh[ndata_r].t1[0] = in[0];
		raw_mesh[ndata_r].t1[1] = -in[1];
		raw_mesh[ndata_r].t1[2] = in[2] * z_dir;
		raw_mesh[ndata_r].t2[0] = in[3];
		raw_mesh[ndata_r].t2[1] = -in[4];
		raw_mesh[ndata_r].t2[2] = in[5] * z_dir;
		raw_mesh[ndata_r].t3[0] = in[6];
		raw_mesh[ndata_r].t3[1] = -in[7];
		raw_mesh[ndata_r].t3[2] = in[8] * z_dir;
		ndata_r++;
	}
	fclose(fp);
	return (ndata_r);
}

/* -----------------------------------------------------------------*/
int read_stl (struct GMT_CTRL *GMT, char *fname, double z_dir) {
	/* read a file with triagles in the stl format and returns nb of triangles */
	unsigned int ndata_s;
	size_t n_alloc;
	double in[3];
	char line[GMT_LEN256] = {""}, text[128] = {""}, ver_txt[128] = {""};
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata_s = 0;
	raw_mesh = GMT_memory (GMT, NULL, n_alloc, struct RAW);

	while (fgets (line, GMT_LEN256, fp)) {
		sscanf (line, "%s", text);
		if (strcmp (text, "outer") == 0) {
			if (fgets (line, GMT_LEN256, fp) == NULL) /* get first vertex */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR reading outer first vertex of \n", fname);
			if (sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering triangle %d of %s\n", ndata_s+1, fname);
			raw_mesh[ndata_s].t1[0] = in[0];
			raw_mesh[ndata_s].t1[1] = -in[1];
			raw_mesh[ndata_s].t1[2] = in[2] * z_dir;
			if (fgets (line, GMT_LEN256, fp) == NULL) /* get second vertex */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR reading outer second vertex of \n", fname);
			if (sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering triangle %d of %s\n", ndata_s+1, fname);
			raw_mesh[ndata_s].t2[0] = in[0];
			raw_mesh[ndata_s].t2[1] = -in[1];
			raw_mesh[ndata_s].t2[2] = in[2] * z_dir;
			if (fgets (line, GMT_LEN256, fp) == NULL) /* get third vertex */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR reading outer third vertex of \n", fname);
			if (sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering triangle %d of %s\n", ndata_s+1, fname);
			raw_mesh[ndata_s].t3[0] = in[0];
			raw_mesh[ndata_s].t3[1] = -in[1];
			raw_mesh[ndata_s].t3[2] = in[2] * z_dir;
			ndata_s++;
              		if (ndata_s == n_alloc) { /* with bad luck we have a flaw here */
				n_alloc <<= 1;
				raw_mesh = GMT_memory (GMT, raw_mesh, n_alloc, struct RAW);
			}
		}
		else
			continue;
	}
	fclose(fp);
	return (ndata_s);
}

/* -----------------------------------------------------------------*/
int read_poly (struct GMT_CTRL *GMT, char *fname, bool switch_xy) {
	/* Read file with xy points where anomaly is going to be computed*/
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
int facet_triangulate (struct XYZOKB_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool bat) {
	/* Sets coodinates for the facet whose effect is beeing calculated */
	double x_a, x_b, x_c, y_a, y_b, y_c, z_a, z_b, z_c;

	x_a = triang[vert[i].a].x;	x_b = triang[vert[i].b].x;	x_c = triang[vert[i].c].x;
	y_a = triang[vert[i].a].y;	y_b = triang[vert[i].b].y;	y_c = triang[vert[i].c].y;
	z_a = triang[vert[i].a].z;	z_b = triang[vert[i].b].z;	z_c = triang[vert[i].c].z;
	/* top triang */
	body_verts[0].x = x_a;	body_verts[0].y = y_a;
	body_verts[1].x = x_b;	body_verts[1].y = y_b;
	body_verts[2].x = x_c;	body_verts[2].y = y_c;
	/* bot triang */
	body_verts[3].x = body_verts[0].x;	body_verts[3].y = body_verts[0].y;
	body_verts[4].x = body_verts[1].x;	body_verts[4].y = body_verts[1].y;
	body_verts[5].x = body_verts[2].x;	body_verts[5].y = body_verts[2].y;

	body_verts[6].x = body_verts[1].x;	body_verts[6].y = body_verts[1].y;
	body_verts[7].x = body_verts[4].x;	body_verts[7].y = body_verts[4].y;
	body_verts[8].x = body_verts[5].x;	body_verts[8].y = body_verts[5].y;
	body_verts[9].x = body_verts[2].x;	body_verts[9].y = body_verts[2].y;

	body_verts[10].x = body_verts[1].x;	body_verts[10].y = body_verts[1].y;
	body_verts[11].x = body_verts[0].x;	body_verts[11].y = body_verts[0].y;
	body_verts[12].x = body_verts[3].x;	body_verts[12].y = body_verts[3].y;
	body_verts[13].x = body_verts[4].x;	body_verts[13].y = body_verts[4].y;

	body_verts[14].x = body_verts[0].x;	body_verts[14].y = body_verts[0].y;
	body_verts[15].x = body_verts[2].x;	body_verts[15].y = body_verts[2].y;
	body_verts[16].x = body_verts[5].x;	body_verts[16].y = body_verts[5].y;
	body_verts[17].x = body_verts[3].x;	body_verts[17].y = body_verts[3].y;

	if (Ctrl->E.active) { /* Layer of constant thickness */
		body_verts[0].z = z_a;			body_verts[1].z = z_b;
		body_verts[2].z = z_c;			body_verts[3].z = z_a + Ctrl->E.dz;
		body_verts[4].z = z_b + Ctrl->E.dz;	body_verts[5].z = z_c + Ctrl->E.dz;
		body_verts[6].z = body_verts[1].z;	body_verts[7].z = body_verts[4].z;
		body_verts[8].z = body_verts[5].z;	body_verts[9].z = body_verts[5].z;
		body_verts[10].z = body_verts[1].z;	body_verts[11].z = body_verts[0].z;
		body_verts[12].z = body_verts[3].z;	body_verts[13].z = body_verts[4].z;
		body_verts[14].z = body_verts[0].z;	body_verts[15].z = body_verts[2].z;
		body_verts[16].z = body_verts[5].z;	body_verts[17].z = body_verts[3].z;

		return (1);
	}
	if (bat) { /* Triangle mesh defines a bathymetric surface (TA MIXORDADO (== NOS DOIS CASOS)) */
		body_verts[0].z = z_a;		body_verts[1].z = z_b;
		body_verts[2].z = z_c;		body_verts[3].z = Ctrl->Z.z0;
		body_verts[4].z = Ctrl->Z.z0;	body_verts[5].z = Ctrl->Z.z0;
		if (fabs(body_verts[0].z - body_verts[3].z) > Ctrl->E.dz || fabs(body_verts[1].z - body_verts[4].z) >
		    Ctrl->E.dz || fabs(body_verts[2].z - body_verts[5].z) > Ctrl->E.dz)
			return 1;
		else
			return (0);
	}
	else { /* Triangle mesh defines a topographic surface */
		body_verts[0].z = z_a;		body_verts[1].z = z_b;
		body_verts[2].z = z_c;		body_verts[3].z = Ctrl->Z.z0;
		body_verts[4].z = Ctrl->Z.z0;	body_verts[5].z = Ctrl->Z.z0;
		if (fabs(body_verts[0].z - body_verts[3].z) > Ctrl->E.dz || fabs(body_verts[1].z - body_verts[4].z) >
		    Ctrl->E.dz || fabs(body_verts[2].z - body_verts[5].z) > Ctrl->E.dz)
			return 1;
		else
			return (0);
	}
}

/* -----------------------------------------------------------------*/
int facet_raw (struct XYZOKB_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool geo) {
	/* Sets coodinates for the facet in the RAW format */
	double cos_a, cos_b, cos_c, x_a, x_b, x_c, y_a, y_b, y_c, z_a, z_b, z_c;

	x_a = raw_mesh[i].t1[0];   x_b = raw_mesh[i].t2[0];   x_c = raw_mesh[i].t3[0];
	y_a = raw_mesh[i].t1[1];   y_b = raw_mesh[i].t2[1];   y_c = raw_mesh[i].t3[1];
	z_a = raw_mesh[i].t1[2];   z_b = raw_mesh[i].t2[2];   z_c = raw_mesh[i].t3[2];
	if (geo) {
		cos_a = cos(y_a*D2R);	cos_b = cos(y_b*D2R);	cos_c = cos(y_c*D2R);
	}
	else {
		cos_a = cos_b = cos_c = 1;
	}

	body_verts[0].x = x_a*Ctrl->box.d_to_m*cos_a;		body_verts[0].y = y_a*Ctrl->box.d_to_m;
	body_verts[1].x = x_b*Ctrl->box.d_to_m*cos_b;		body_verts[1].y = y_b*Ctrl->box.d_to_m;
	body_verts[2].x = x_c*Ctrl->box.d_to_m*cos_c;		body_verts[2].y = y_c*Ctrl->box.d_to_m;
	body_verts[0].z = z_a;
	body_verts[1].z = z_b;
	body_verts[2].z = z_c;
	return 1; /* Allways return 1 */
}

/* ---------------------------------------------------------------------- */
void set_center (unsigned int n_triang) {
	/* Calculates triangle center by an aproximate (iterative) formula */
	unsigned int i, j, k = 5;
	double x, y, z, xa[6], ya[6], xb[6], yb[6], xc[6], yc[6];

	for (i = 0; i < n_triang; i++) {
		xa[0] = (triang[vert[i].b].x + triang[vert[i].c].x) / 2.;
		ya[0] = (triang[vert[i].b].y + triang[vert[i].c].y) / 2.;
		xb[0] = (triang[vert[i].c].x + triang[vert[i].a].x) / 2.;
		yb[0] = (triang[vert[i].c].y + triang[vert[i].a].y) / 2.;
		xc[0] = (triang[vert[i].a].x + triang[vert[i].b].x) / 2.;
		yc[0] = (triang[vert[i].a].y + triang[vert[i].b].y) / 2.;
		for (j = 1; j <= k; j++) {
			xa[j] = (xb[j-1] + xc[j-1]) / 2.;
			ya[j] = (yb[j-1] + yc[j-1]) / 2.;
			xb[j] = (xc[j-1] + xa[j-1]) / 2.;
			yb[j] = (yc[j-1] + ya[j-1]) / 2.;
			xc[j] = (xa[j-1] + xb[j-1]) / 2.;
			yc[j] = (ya[j-1] + yb[j-1]) / 2.;
		}
		x = (xa[k]+xb[k]+xc[k])/3.;
		y = (ya[k]+yb[k]+yc[k])/3.;
		z = (triang[vert[i].a].z+triang[vert[i].b].z+triang[vert[i].c].z)/3.;
		t_center[i].x = x;
		t_center[i].y = y;
		t_center[i].z = z;
	}
}

#if 0
void triang_norm (int n_triang) {
	/* Computes the unit normal to trianglular facet */
	int i;
	double v1[3], v2[3], v3[3], mod, n[3];

	for (i = 0; i < n_triang; i++) {
		v1[0] = triang[vert[i].a].x - triang[vert[i].b].x;
		v1[1] = triang[vert[i].a].y - triang[vert[i].b].y;
		v1[2] = triang[vert[i].a].z - triang[vert[i].b].z;

		v2[0] = triang[vert[i].b].x - triang[vert[i].c].x;
		v2[1] = triang[vert[i].b].y - triang[vert[i].c].y;
		v2[2] = triang[vert[i].b].z - triang[vert[i].c].z;

		v3[0] = v1[1]*v2[2] - v1[2]*v2[1];
		v3[1] = v1[2]*v2[0] - v1[0]*v2[2];
		v3[2] = v1[0]*v2[1] - v1[1]*v2[0];

		mod = sqrt(v3[0]*v3[0] + v3[1]*v3[1] + v3[2]*v3[2]);
		n[0] = v3[0] / mod;
		n[1] = v3[1] / mod;
		n[2] = v3[2] / mod;
	}
}
#endif

int check_triang_cw (unsigned int n, unsigned int type) {
	/* Checks that triangles are given in the correct clock-wise order.
	If not swap them. This is a tricky issue. In the case of "classic"
	trihedron (x positive right; y positive "north" and z positive up),
	positive determinants signify counter clockwise order. However, in
	geomagnetic reference (x positive right; y positive "south" and z
	positive down (OK, I know it's not exactly like this but instead
	x->north; y->east; z->down)), counter clockwise order follows if
	determinant is negative. */

	unsigned int i, n_swaped = 0, tmp;
	double x1 = 0, x2 = 0, x3 = 0, y1 = 0, y2 = 0, y3 = 0, det, d_tmp[3];

	if (type > 0)	/* Not yet implemented || 28-4-2010. Dont't undersand why but seams true !!!! */
		return (0);
	for (i = 0; i < n; i++) {
		if (type == 0) { /* triangulate */
			x1 = triang[vert[i].a].x;	 y1 = triang[vert[i].a].y;
			x2 = triang[vert[i].b].x;	 y2 = triang[vert[i].b].y;
			x3 = triang[vert[i].c].x;	 y3 = triang[vert[i].c].y;
		}
		else if (type == 1) { /* raw */
			x1 = raw_mesh[i].t1[0];		y1 = raw_mesh[i].t1[1];
			x2 = raw_mesh[i].t2[0];		y2 = raw_mesh[i].t2[1];
			x3 = raw_mesh[i].t3[0];		y3 = raw_mesh[i].t3[1];
		}

		det = (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);

		if (det < 0.0) { /* counter clockwise triangle -> swap vertex order */
			if (type == 0) {
				tmp = vert[i].b;
				vert[i].b = vert[i].c;
				vert[i].c = tmp;
				n_swaped++;
			}
			else if (type == 1) {
				d_tmp[0] = raw_mesh[i].t2[0];
				d_tmp[1] = raw_mesh[i].t2[1];
				d_tmp[2] = raw_mesh[i].t2[2];
				raw_mesh[i].t2[0] = raw_mesh[i].t3[0];
				raw_mesh[i].t2[1] = raw_mesh[i].t3[1];
				raw_mesh[i].t2[2] = raw_mesh[i].t3[2];
				raw_mesh[i].t3[0] = d_tmp[0];
				raw_mesh[i].t3[1] = d_tmp[1];
				raw_mesh[i].t3[2] = d_tmp[2];
				n_swaped++;
			}
		}
	}
	return (n_swaped);
}
