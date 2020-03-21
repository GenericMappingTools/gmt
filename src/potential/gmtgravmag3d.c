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
 * API functions to support the gmtgravmag3d application.
 *
 * Brief synopsis:
 *
 *
 * Author:	Joaquim Luis
 * Date:	06-OCT-2000 (original GMT 4)
 *
 */

#include "gmt_dev.h"
#include "okbfuns.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtgravmag3d"
#define THIS_MODULE_MODERN_NAME	"gmtgravmag3d"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Compute the gravity/magnetic anomaly of a 3-D body by the method of Okabe"
#define THIS_MODULE_KEYS	"TD{,FD(,GG),>D}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVf"

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

static struct TRIANG {
	double  x, y, z;
} *triang;

static struct  VERT {
	unsigned int  a, b, c;
} *vert;

static struct  TRI_CENTER {
	double  x, y, z;
} *t_center;

static struct RAW {
	double  t1[3], t2[3], t3[3];
} *raw_mesh;

static struct MAG_VAR2 {
	double	m, m_dip;
} *okabe_mag_var2;

static struct MAG_VAR3 {
	double	m, m_dec, m_dip;
} *okabe_mag_var3;

static struct MAG_VAR4 {
	double	t_dec, t_dip, m, m_dec, m_dip;
} *okabe_mag_var4;

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct XYZOKB_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct XYZOKB_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->L.zobs = 0;
	C->D.dir = -1;
	C->S.radius = 50000;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->F.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->T.xyz_file);
	gmt_M_str_free (C->T.t_file);
	gmt_M_str_free (C->T.raw_file);
	gmt_M_str_free (C->T.stl_file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int read_t (struct GMT_CTRL *GMT, char *fname);
GMT_LOCAL int read_raw (struct GMT_CTRL *GMT, char *fname, double z_dir);
GMT_LOCAL int read_stl (struct GMT_CTRL *GMT, char *fname, double z_dir);
GMT_LOCAL void set_center (unsigned int n_triang);
GMT_LOCAL int facet_triangulate (struct XYZOKB_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool bat);
GMT_LOCAL int facet_raw (struct XYZOKB_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool geo);
GMT_LOCAL int check_triang_cw (unsigned int n, unsigned int type);
GMT_LOCAL int read_xyz (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *Ctrl, char *fname, double *lon_0, double *lat_0);

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s -Tp<xyz_file>[+m] -Tv<vert_file> | -Tr|s<raw_file> [-C<density>] [-G<outgrid>]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-E<thick>] [-F<xy_file>] [-L<z_observation>]\n", GMT_I_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-H<f_dec>/<f_dip>/<m_int></m_dec>/<m_dip>] [-S<radius>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-Z<level>] [%s] [-fg] [%s] [%s]\n\n", GMT_V_OPT, GMT_r_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-T Gives names of xyz (-Tp<fname>[+m]) and vertex (-Tv<fname>) files defining a close surface.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The [+m] in -Tp tells the program that file has 4 columns and fourth holds a variable magnetization.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The file formats correspond to the output of the triangulate program.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively use -Tr<file> for file in raw triangle format (x1 y1 z1 x2 ... z3).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or -Ts<file> for file in STL format.\n");

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-H Sets parameters for computation of magnetic anomaly.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <f_dec>/<f_dip> -> geomagnetic declination/inclination.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <m_int></m_dec></m_dip> -> body magnetic intensity/declination/inclination.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Sets body <density> in SI units.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Passes locations where anomaly is going to be computed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Sets name of the output grdfile.\n");
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Sets level of observation [Default = 0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E give layer thickness in m [Default = 0 m].\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Sets search radius in km.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Sets z level of reference plane [Default = 0].\n");
	GMT_Option (API, "bi");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Converts geographic grids to meters using a \"Flat Earth\" approximation.\n");
	GMT_Option (API, "r,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *Ctrl, struct GMT_OPTION *options) {

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
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'B':	/* For backward compat (Undocumented) */
			case 'H':
				if ((sscanf(opt->arg, "%lf/%lf/%lf/%lf/%lf",
					    &Ctrl->H.t_dec, &Ctrl->H.t_dip, &Ctrl->H.m_int, &Ctrl->H.m_dec, &Ctrl->H.m_dip)) != 5) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -H: Can't dechiper values\n");
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
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				Ctrl->I.active = true;
				if (gmt_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					gmt_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':
				Ctrl->L.active = true;
				Ctrl->L.zobs = atof (opt->arg);
				break;
			case 'M':
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -M is deprecated; -fg was set instead, use this in the future.\n");
					if (gmt_M_is_cartesian (GMT, GMT_IN)) gmt_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
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
			case 'T': 		/* Selected input mesh format */
				Ctrl->T.active = true;
				if (opt->arg[0] == 'p') {
					char *pch;
					Ctrl->T.xyz_file = strdup(&opt->arg[1]);
					Ctrl->T.triangulate = true;
					if ((pch = strstr(opt->arg, "+m")) != NULL) {	/* Variable magnetization */
						Ctrl->T.m_var = true;
						Ctrl->H.active = true;
						Ctrl->C.active = false;
						pch += 2;		/* Jump the +m chars. These are not documented and I (JL) don't remember what they do */
						if (pch && pch[0] == '2') Ctrl->T.m_var2 = true;
						else if (pch && pch[0] == '3') Ctrl->T.m_var3 = true;
						else if (pch && pch[0] == '4') Ctrl->T.m_var4 = true;
						else Ctrl->T.m_var1 = true;
						Ctrl->T.xyz_file[strlen(Ctrl->T.xyz_file)-2] = '\0';	/* In any case the "+m" must go out of fname */
					}
				}
				else if (opt->arg[0] == 'v') {
					Ctrl->T.t_file = strdup(&opt->arg[1]);
				}
				else if (opt->arg[0] == 'r') {
 					Ctrl->T.raw_file = strdup(&opt->arg[1]);
					Ctrl->T.raw = true;
				}
				else if (opt->arg[0] == 's') {
 					Ctrl->T.stl_file = strdup(&opt->arg[1]);
					Ctrl->T.stl = true;
				}
				else {			/* For backward compat with old syntax (to be removed some day) */
					switch (opt->arg[0]) {
						case 'd':	/* Surface computed by triangulate */
							j = 0;
							while (gmt_strtok (&opt->arg[1], "/", &pos, ptr)) {
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
								GMT_Report(API, GMT_MSG_ERROR, "Option -T: Must give names for data points and vertex files\n");
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
				}
				break;
			case 'Z':
				Ctrl->Z.z0 = atof(opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition(GMT, Ctrl->S.active && (Ctrl->S.radius <= 0.0 || gmt_M_is_dnan (Ctrl->S.radius)),
	                                  "Option -S: Radius is NaN or negative\n");
	n_errors += gmt_M_check_condition(GMT, !Ctrl->T.active, "Option -T is mandatory\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->T.xyz_file != NULL && Ctrl->T.t_file == NULL,
	                                  "with -Tp must provide also vertex (-Tv) file.\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->T.t_file != NULL && Ctrl->T.xyz_file == NULL,
	                                  "Option -T: vertex file provided (-Tv) but not xyz file (-Tp).\n");
	n_errors += gmt_M_check_condition(GMT, !Ctrl->G.active && !Ctrl->F.active, "Must specify either -G or -F options\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->G.active && !Ctrl->I.active, "Must specify -I option\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->G.active && !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->C.rho == 0.0 && !Ctrl->H.active && !Ctrl->T.m_var4 ,
	                                  "Must specify either -Cdensity or -H<stuff>\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->G.active && !Ctrl->G.file, "Option -G: Must specify output file\n");
	j = gmt_M_check_condition(GMT, Ctrl->G.active && Ctrl->F.active, "Warning: -F overrides -G\n");
	if (gmt_M_check_condition(GMT, Ctrl->T.raw && Ctrl->S.active, "Warning: -Tr overrides -S\n"))
		Ctrl->S.active = false;

	/*n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Must specify input file\n");*/

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtgravmag3d (void *V_API, int mode, void *args) {

	bool bat = true, DO = true;
	unsigned int row, col, i, j, k, kk, ndata_r = 0;
	unsigned int ndata_p = 0, ndata_t = 0, nx_p, ny_p, n_vert_max;
	unsigned int z_th = 0, n_triang = 0, ndata_s = 0, n_swap = 0;
	int retval, error = 0;
	uint64_t ij;
	size_t nm;
	int km, pm;		/* index of current body facet (for mag only) */
	gmt_grdfloat	*g = NULL, one_100;
	double	s_rad2, x_o, y_o, t_mag, a, DX, DY;
	double	*x_obs = NULL, *y_obs = NULL, *z_obs = NULL, *x = NULL, *y = NULL, *cos_vec = NULL;
	double	cc_t, cs_t, s_t, lon_0 = 0, lat_0 = 0;

	struct	LOC_OR *loc_or = NULL;
	struct	BODY_VERTS *body_verts = NULL;
	struct	BODY_DESC body_desc;
	struct	XYZOKB_CTRL *Ctrl = NULL;
	struct	GMT_GRID *Gout = NULL;
	struct  GMT_DATASET *Cin = NULL;
	struct  GMT_DATATABLE *point = NULL;
	struct	GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct	GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	struct MAG_PARAM *okabe_mag_param = NULL;
	struct MAG_VAR *okabe_mag_var = NULL;

	triang = NULL, vert = NULL, t_center = NULL, raw_mesh = NULL;
	okabe_mag_var2 = NULL, okabe_mag_var3 = NULL, okabe_mag_var4 = NULL;
	body_desc.n_v = NULL, body_desc.ind = NULL;

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

	/*---------------------------- This is the gmtgravmag3d main code ----------------------------*/

	if (gmt_M_is_geographic (GMT, GMT_IN)) Ctrl->box.is_geog = true;

	if (!Ctrl->box.is_geog)
		Ctrl->box.d_to_m = 1;
	else
		Ctrl->box.d_to_m = 2 * M_PI * 6371008.7714 / 360.0;
		/*Ctrl->box.d_to_m = 2.0 * M_PI * gmtdefs.ellipse[N_ELLIPSOIDS-1].eq_radius / 360.0;*/

	Ctrl->Z.z0 *= Ctrl->D.dir;

	/* ---- Read files section ---------------------------------------------------- */
	if (Ctrl->F.active) { 		/* Read xy file where anomaly is to be computed */
		if ((Cin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IO_ASCII, NULL, Ctrl->F.file, NULL)) == NULL)
			Return (API->error);
		if (Cin->n_columns < 2) {	/* Trouble */
			GMT_Report (API, GMT_MSG_ERROR, "Option -F: %s does not have at least 2 columns with coordinates\n",
			            Ctrl->F.file);
			Return (GMT_PARSE_ERROR);
		}
		point   = Cin->table[0];	/* Can only be one table since we read a single file */
		ndata_p = (unsigned int)point->n_records;
		if (point->n_segments > 1) /* case not dealt (or ignored) and should be tested here */
			GMT_Report(API, GMT_MSG_WARNING, "Multi-segment files are not used in gmtgravmag3d. Using first segment only\n");
	}

	if (Ctrl->T.triangulate) { 	/* Read triangle file output from triangulate */
		if ((retval = read_xyz (GMT, Ctrl, Ctrl->T.xyz_file, &lon_0, &lat_0)) < 0 ) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot open file %s\n", Ctrl->T.xyz_file);
			Return (GMT_ERROR_ON_FOPEN);
		}
		/* read vertex file */
		if ((retval = read_t (GMT, Ctrl->T.t_file)) < 0 ) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot open file %s\n", Ctrl->T.t_file);
			Return (GMT_ERROR_ON_FOPEN);
		}
		ndata_t = retval;

		t_center = gmt_M_memory (GMT, NULL, ndata_t, struct TRI_CENTER);
		/* compute approximate center of each triangle */
		n_swap = check_triang_cw (ndata_t, 0);
		set_center (ndata_t);
	}
	else if (Ctrl->T.stl) { 	/* Read STL file defining a closed volume */
		if ( (retval = read_stl (GMT, Ctrl->T.stl_file, Ctrl->D.dir)) < 0 ) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot open file %s\n", Ctrl->T.stl_file);
			Return (GMT_ERROR_ON_FOPEN);
		}
		ndata_s = retval;
		/*n_swap = check_triang_cw (ndata_s, 1);*/
	}
	else if (Ctrl->T.raw) { 	/* Read RAW file defining a closed volume */
		if ( (retval = read_raw (GMT, Ctrl->T.raw_file, Ctrl->D.dir)) < 0 ) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot open file %s\n", Ctrl->T.raw_file);
			Return (GMT_ERROR_ON_FOPEN);
		}
		ndata_r = retval;
		/*n_swap = check_triang_cw (ndata_r, 1);*/
	}

	if (n_swap > 0)
		GMT_Report (API, GMT_MSG_INFORMATION, "%d triangles had ccw order\n", n_swap);
	/* --------------------------------------------------------------------------------------- */

	if (Ctrl->G.active) {
		if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, Ctrl->I.inc, \
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);

		GMT_Report (API, GMT_MSG_INFORMATION, "Grid dimensions are n_columns = %d, n_rows = %d\n", Gout->header->n_columns, Gout->header->n_rows);

		/* Build observation point vectors */
		x = gmt_M_memory (GMT, NULL, Gout->header->n_columns, double);
		y = gmt_M_memory (GMT, NULL, Gout->header->n_rows, double);
		for (i = 0; i < Gout->header->n_columns; i++)
			x[i] = (i == (Gout->header->n_columns-1)) ? Gout->header->wesn[XHI] : (Gout->header->wesn[XLO] + i * Gout->header->inc[GMT_X]);
		for (j = 0; j < Gout->header->n_rows; j++)
			y[j] = (j == (Gout->header->n_rows-1)) ? -Gout->header->wesn[YLO] : -(Gout->header->wesn[YHI] - j * Gout->header->inc[GMT_Y]);
	}

	nx_p = (!Ctrl->F.active) ? Gout->header->n_columns : ndata_p;
	ny_p = (!Ctrl->F.active) ? Gout->header->n_rows : ndata_p;
	nm   = (!Ctrl->F.active) ? Gout->header->nm : ndata_p;
	x_obs = gmt_M_memory (GMT, NULL, nx_p, double);
	y_obs = gmt_M_memory (GMT, NULL, ny_p, double);
	z_obs = gmt_M_memory (GMT, NULL, nx_p, double);
	body_verts = gmt_M_memory (GMT, NULL, 18, struct BODY_VERTS);

	if (Ctrl->F.active) { /* Need to compute observation coords only once */
		size_t row;
		for (row = 0; row < ndata_p; row++) {
			x_obs[row] = (Ctrl->box.is_geog) ? (point->segment[0]->data[GMT_X][row] - lon_0) *
				Ctrl->box.d_to_m*cos(point->segment[0]->data[GMT_Y][row] * D2R) : point->segment[0]->data[GMT_X][row];
			y_obs[row] = (Ctrl->box.is_geog) ? -(point->segment[0]->data[GMT_Y][row] - lat_0) *
				Ctrl->box.d_to_m : -point->segment[0]->data[GMT_Y][row]; /* - because y positive 'south' */
		}
		g = gmt_M_memory (GMT, NULL, nm, gmt_grdfloat);
	}

	if (Ctrl->T.triangulate) {
		n_triang = ndata_t;
		body_desc.n_f = 5;		/* Number of prism facets */
		body_desc.n_v = gmt_M_memory(GMT, NULL, body_desc.n_f, unsigned int);
		body_desc.n_v[0] = 3;	body_desc.n_v[1] = 3;
		body_desc.n_v[2] = 4;	body_desc.n_v[3] = 4;
		body_desc.n_v[4] = 4;
		body_desc.ind = gmt_M_memory(GMT, NULL, (body_desc.n_v[0] + body_desc.n_v[1] +
		                           body_desc.n_v[2] + body_desc.n_v[3] + body_desc.n_v[4]), unsigned int);
		body_desc.ind[0] = 0;	body_desc.ind[1] = 1; 	body_desc.ind[2] = 2;	/* top triang */
		body_desc.ind[3] = 3;	body_desc.ind[4] = 5; 	body_desc.ind[5] = 4;	/* bot triang */
		body_desc.ind[6] = 1;	body_desc.ind[7] = 4; 	body_desc.ind[8] = 5;	body_desc.ind[9] = 2;
		body_desc.ind[10] = 0;	body_desc.ind[11] = 3;	body_desc.ind[12] = 4;	body_desc.ind[13] = 1;
	 	body_desc.ind[14] = 0;	body_desc.ind[15] = 2;	body_desc.ind[16] = 5;	body_desc.ind[17] = 3;

	 	/* Actually, for the gravity case we can save lots of computations because the flux
	 	   through the vertical walls does not contribute to gravity anomaly, which is vertical */
		if (Ctrl->C.active)
			body_desc.n_f = 2;		/* Number of prism facets that count */
	}
	else if (Ctrl->T.raw || Ctrl->T.stl) {
		n_triang = (Ctrl->T.raw) ? ndata_r : ndata_s;
		body_desc.n_f = 1;
		body_desc.n_v = gmt_M_memory (GMT, NULL, body_desc.n_f, unsigned int);
		body_desc.n_v[0] = 3;
		body_desc.ind = gmt_M_memory (GMT, NULL, body_desc.n_v[0], unsigned int);
		body_desc.ind[0] = 0;	body_desc.ind[1] = 1; 	body_desc.ind[2] = 2;
	}
	else {
		GMT_Report (API, GMT_MSG_ERROR, "It shouldn't pass here\n");
		error = GMT_RUNTIME_ERROR;
		goto END;
	}

	/* Allocate a structure that will be used inside okabe().
	   We do it here to avoid thousands of alloc/free that would result if done in okabe() */
	n_vert_max = body_desc.n_v[0];
	for (i = 1; i < body_desc.n_f; i++)
		n_vert_max = MAX(body_desc.n_v[i], n_vert_max);

	loc_or = gmt_M_memory (GMT, NULL, (n_vert_max+1), struct LOC_OR);

	if (Ctrl->H.active) { /* 1e2 is a factor to obtain nT from magnetization in A/m */
		cc_t = cos(Ctrl->H.m_dip*D2R)*cos((Ctrl->H.m_dec - 90.)*D2R);
		cs_t = cos(Ctrl->H.m_dip*D2R)*sin((Ctrl->H.m_dec - 90.)*D2R);
		s_t = sin(Ctrl->H.m_dip*D2R);
		if (!Ctrl->T.m_var4) {		/* In all the other cases the field parameters are constatnt */
			okabe_mag_param = gmt_M_memory (GMT, NULL, 1, struct MAG_PARAM);
			okabe_mag_param[0].rim[0] = 1e2*cos(Ctrl->H.t_dip*D2R) * cos((Ctrl->H.t_dec - 90.)*D2R);
			okabe_mag_param[0].rim[1] = 1e2*cos(Ctrl->H.t_dip*D2R) * sin((Ctrl->H.t_dec - 90.)*D2R);
			okabe_mag_param[0].rim[2] = 1e2*sin(Ctrl->H.t_dip*D2R);
		}
		if (!Ctrl->T.m_var) { /* Case of constant magnetization */
			okabe_mag_var = gmt_M_memory (GMT, NULL, 1, struct MAG_VAR);
			okabe_mag_var[0].rk[0] = Ctrl->H.m_int * cc_t;
			okabe_mag_var[0].rk[1] = Ctrl->H.m_int * cs_t;
			okabe_mag_var[0].rk[2] = Ctrl->H.m_int * s_t;
		}
		else { /* The triangles have a non-constant magnetization */
			okabe_mag_var = gmt_M_memory (GMT, NULL, n_triang, struct MAG_VAR);
			if (Ctrl->T.m_var1) {		/* Only the mag intensity changes. Mag dec & dip are constant */
				for (i = 0; i < n_triang; i++) {
					t_mag = (Ctrl->box.mag_int[vert[i].a] + Ctrl->box.mag_int[vert[i].b] + Ctrl->box.mag_int[vert[i].c])/3.;
					okabe_mag_var[i].rk[0] = t_mag * cc_t;
					okabe_mag_var[i].rk[1] = t_mag * cs_t;
					okabe_mag_var[i].rk[2] = t_mag * s_t;
				}
			}
			else if (Ctrl->T.m_var2) {	/* Both mag intensity & dip varies. Dec is Zero (axial dipole) */
				for (i = 0; i < n_triang; i++) {
					t_mag = (okabe_mag_var2[vert[i].a].m + okabe_mag_var2[vert[i].b].m + okabe_mag_var2[vert[i].c].m)/3.;
					Ctrl->H.t_dip = (okabe_mag_var2[vert[i].a].m_dip + okabe_mag_var2[vert[i].b].m_dip + okabe_mag_var2[vert[i].c].m_dip)/3.;
					okabe_mag_var[i].rk[0] = 0.;
					okabe_mag_var[i].rk[1] = -t_mag * cos(Ctrl->H.t_dip*D2R);
					okabe_mag_var[i].rk[2] = t_mag * sin(Ctrl->H.t_dip*D2R);
				}
			}
			else if (Ctrl->T.m_var3) { 	/* Both mag intensity, mag_dec & mag_dip varies. */
				for (i = 0; i < n_triang; i++) {
					t_mag = (okabe_mag_var3[vert[i].a].m + okabe_mag_var3[vert[i].b].m + okabe_mag_var3[vert[i].c].m)/3.;
					Ctrl->H.t_dec = (okabe_mag_var3[vert[i].a].m_dec + okabe_mag_var3[vert[i].b].m_dec + okabe_mag_var3[vert[i].c].m_dec)/3.;
					Ctrl->H.t_dip = (okabe_mag_var3[vert[i].a].m_dip + okabe_mag_var3[vert[i].b].m_dip + okabe_mag_var3[vert[i].c].m_dip)/3.;
					okabe_mag_var[i].rk[0] = t_mag * cos(Ctrl->H.t_dip*D2R) * cos((Ctrl->H.t_dec - 90)*D2R);
					okabe_mag_var[i].rk[1] = t_mag * cos(Ctrl->H.t_dip*D2R) * sin((Ctrl->H.t_dec - 90)*D2R);
					okabe_mag_var[i].rk[2] = t_mag * sin(Ctrl->H.t_dip*D2R);
				}
			}
			else {			/* Everything varies. */
				if (okabe_mag_param == NULL)	/* If not yet allocated */
					okabe_mag_param = gmt_M_memory (GMT, NULL, n_triang, struct MAG_PARAM);
				for (i = 0; i < n_triang; i++) {
					Ctrl->H.t_dec = (okabe_mag_var4[vert[i].a].t_dec + okabe_mag_var4[vert[i].b].t_dec + okabe_mag_var4[vert[i].c].t_dec)/3.;
					Ctrl->H.t_dip = (okabe_mag_var4[vert[i].a].t_dip + okabe_mag_var4[vert[i].b].t_dip + okabe_mag_var4[vert[i].c].t_dip)/3.;
					okabe_mag_param[i].rim[0] = 1e2*cos(Ctrl->H.t_dip*D2R) * cos((Ctrl->H.t_dec - 90.)*D2R);
					okabe_mag_param[i].rim[1] = 1e2*cos(Ctrl->H.t_dip*D2R) * sin((Ctrl->H.t_dec - 90.)*D2R);
					okabe_mag_param[i].rim[2] = 1e2*sin(Ctrl->H.t_dip*D2R);
					t_mag = (okabe_mag_var4[vert[i].a].m + okabe_mag_var4[vert[i].b].m + okabe_mag_var4[vert[i].c].m)/3.;
					Ctrl->H.t_dec = (okabe_mag_var4[vert[i].a].m_dec + okabe_mag_var4[vert[i].b].m_dec + okabe_mag_var4[vert[i].c].m_dec)/3.;
					Ctrl->H.t_dip = (okabe_mag_var4[vert[i].a].m_dip + okabe_mag_var4[vert[i].b].m_dip + okabe_mag_var4[vert[i].c].m_dip)/3.;
					okabe_mag_var[i].rk[0] = t_mag * cos(Ctrl->H.t_dip*D2R) * cos((Ctrl->H.t_dec - 90)*D2R);
					okabe_mag_var[i].rk[1] = t_mag * cos(Ctrl->H.t_dip*D2R) * sin((Ctrl->H.t_dec - 90)*D2R);
					okabe_mag_var[i].rk[2] = t_mag * sin(Ctrl->H.t_dip*D2R);
				}
			}
		}
	}

	/* ---------------> Now start computing <------------------------------------- */
	one_100 = (gmt_grdfloat)(n_triang / 100.);
	s_rad2 = Ctrl->S.radius*Ctrl->S.radius;

	if (Ctrl->G.active) {		/* Compute the cos(lat) vector only once */
		cos_vec = gmt_M_memory (GMT, NULL, Gout->header->n_rows, double);
		for (i = 0; i < Gout->header->n_rows; i++)
			cos_vec[i] = (Ctrl->box.is_geog) ? cos(y[i]*D2R): 1;
	}

	error = GMT_NOERROR;
	for (i = j = 0; i < n_triang; i++) {		/* Main loop over all the triangles */
		if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION) && i > j*one_100) {
			GMT_Message (API, GMT_TIME_NONE, "computed %.2d%s of %d prisms\r", j, "%", n_triang);
			j++;
		}
		km = (int)((Ctrl->T.m_var)  ? i : 0);	/* Variable magnetization (intensity) */
		pm = (int)((Ctrl->T.m_var4) ? i : 0);	/* When al 5 parameters (F, Mag) may be variable (undocumented) */

		/* Don't waste time with zero mag triangles */
		if (Ctrl->H.active && Ctrl->T.m_var && okabe_mag_var[i].rk[0] == 0 && okabe_mag_var[i].rk[1] == 0 && okabe_mag_var[i].rk[2] == 0)
			continue;
		if (Ctrl->T.triangulate)
			z_th = facet_triangulate (Ctrl, body_verts, i, bat);
		else if (Ctrl->T.raw || Ctrl->T.stl)
			z_th = facet_raw (Ctrl, body_verts, i, Ctrl->box.is_geog);
		if (z_th) {
			if (Ctrl->G.active) { /* grid */
				for (row = 0; row < Gout->header->n_rows; row++) {
					y_o = (Ctrl->box.is_geog) ? ((y[row]+lat_0) * Ctrl->box.d_to_m): y[row];
					ij = gmt_M_ijp(Gout->header, row, 0);
					for (col = 0; col < Gout->header->n_columns; col++, ij++) {
						x_o = (Ctrl->box.is_geog) ? ((x[col]-lon_0)*Ctrl->box.d_to_m * cos_vec[row]) : x[col];
						if (Ctrl->S.active) {
							DX = t_center[i].x - x_o;
							DY = t_center[i].y - y_o;
							DO = (DX*DX + DY*DY) < s_rad2;
							if (!DO) continue;
						}

						a = okabe (GMT, x_o, y_o, Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, body_desc, body_verts, km, pm, loc_or, okabe_mag_param, okabe_mag_var);
						Gout->data[ij] += (gmt_grdfloat)a;
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
					a = okabe (GMT, x_obs[kk], y_obs[kk], Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, body_desc, body_verts, km, pm, loc_or, okabe_mag_param, okabe_mag_var);
					g[kk] += (gmt_grdfloat)a;
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

		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Gout)) {
			error = API->error;
			goto END;
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Gout) != GMT_NOERROR) {
			error = API->error;
			goto END;
		}
	}
	else {
		double out[3];
		char save[GMT_LEN64] = {""};
		struct GMT_RECORD *Out = gmt_new_record (GMT, out, NULL);
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
			error = API->error;
			goto END;
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, 3, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			error = API->error;
			goto END;
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			error = API->error;
			goto END;
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			error = API->error;
			goto END;
		}
		strcpy (save, GMT->current.setting.format_float_out);
		strcpy (GMT->current.setting.format_float_out, "%.9g");	/* Make sure we use enough decimals */
		for (k = 0; k < ndata_p; k++) {
			out[GMT_X] = point->segment[0]->data[GMT_X][k];
			out[GMT_Y] = point->segment[0]->data[GMT_Y][k];
			out[GMT_Z] = g[k];
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
		}
		gmt_M_free (GMT, Out);
		strcpy (GMT->current.setting.format_float_out, save);
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data input */
			error = API->error;
			goto END;
		}
	}

END:
	gmt_M_free (GMT, x);
	gmt_M_free (GMT, y);
	gmt_M_free (GMT, g);
	gmt_M_free (GMT, z_obs);
	gmt_M_free (GMT, x_obs);
	gmt_M_free (GMT, y_obs);
	gmt_M_free (GMT, triang);
	gmt_M_free (GMT, raw_mesh);
	gmt_M_free (GMT, t_center);
	gmt_M_free (GMT, vert);
	gmt_M_free (GMT, okabe_mag_param);
	gmt_M_free (GMT, okabe_mag_var);
	gmt_M_free (GMT, body_desc.n_v);
	gmt_M_free (GMT, body_desc.ind);
	gmt_M_free (GMT, loc_or);
	gmt_M_free (GMT, body_verts);
	gmt_M_free (GMT, cos_vec);
	if (Ctrl->T.m_var1) gmt_M_free (GMT, Ctrl->box.mag_int);
	if (Ctrl->T.m_var2) gmt_M_free (GMT, okabe_mag_var2);
	if (Ctrl->T.m_var3) gmt_M_free (GMT, okabe_mag_var3);
	gmt_M_free (GMT, okabe_mag_var4);

	Return (error);
}

/* -------------------------------------------------------------------------*/
GMT_LOCAL int read_xyz (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *Ctrl, char *fname, double *lon_0, double *lat_0) {
	/* read xyz[m] file with point data coordinates */

	unsigned int ndata_xyz;
	size_t n_alloc;
	gmt_grdfloat x_min = FLT_MAX, x_max = -FLT_MAX, y_min = FLT_MAX, y_max = -FLT_MAX;
	double in[8];
	char line[GMT_LEN256] = {""};
	FILE *fp = NULL;

	if ((fp = gmt_fopen (GMT, fname, "r")) == NULL) return (-1);

       	n_alloc = GMT_CHUNK;
	ndata_xyz = 0;
	*lon_0 = 0.;	*lat_0 = 0.;
        triang = gmt_M_memory (GMT, NULL, n_alloc, struct TRIANG);
	if (Ctrl->T.m_var1)
        	Ctrl->box.mag_int = gmt_M_memory (GMT, NULL, n_alloc, double);
	else if (Ctrl->T.m_var2)
        	okabe_mag_var2 = gmt_M_memory (GMT, NULL, n_alloc, struct MAG_VAR2);
	else if (Ctrl->T.m_var3)
        	okabe_mag_var3 = gmt_M_memory (GMT, NULL, n_alloc, struct MAG_VAR3);
	else if (Ctrl->T.m_var4)
        	okabe_mag_var4 = gmt_M_memory (GMT, NULL, n_alloc, struct MAG_VAR4);

	if (Ctrl->box.is_geog) {	/* take a first read just to compute the central longitude */
		while (fgets (line, GMT_LEN256, fp)) {
			sscanf (line, "%lg %lg", &in[0], &in[1]); /* A test on file integrity will be done below */
			x_min = (gmt_grdfloat)MIN(in[0], x_min);	x_max = (gmt_grdfloat)MAX(in[0], x_max);
			y_min = (gmt_grdfloat)MIN(in[1], y_min);	y_max = (gmt_grdfloat)MAX(in[1], y_max);
		}
		*lon_0 = (x_min + x_max) / 2;
		*lat_0  = (y_min + y_max) / 2;
		rewind(fp);
	}

	while (fgets (line, GMT_LEN256, fp)) {
		if (!Ctrl->T.m_var) {
			if(sscanf (line, "%lg %lg %lg", &in[0], &in[1], &in[2]) !=3)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering line %d of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else if (Ctrl->T.m_var1) { /* data file has 4 columns and the last contains magnetization */
			if(sscanf (line, "%lg %lg %lg %lg", &in[0], &in[1], &in[2], &in[3]) !=4)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering line %d of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else if (Ctrl->T.m_var2) { /* data file has 5 columns: x,y,z,mag,dip */
			if(sscanf (line, "%lg %lg %lg %lg %lg", &in[0], &in[1], &in[2], &in[3], &in[4]) !=5)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering line %d of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else if (Ctrl->T.m_var3) { /* data file has 6 columns: x,y,z,mag_int,mag_dec,mag_dip */
			if(sscanf (line, "%lg %lg %lg %lg %lg %lg", &in[0], &in[1], &in[2], &in[3], &in[4], &in[5]) !=6)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering line %d of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else {		/* data file has 8 columns: x,y,z,field_dec,field_dip,mag_int,mag_dec,mag_dip */
			if(sscanf (line, "%lg %lg %lg %lg %lg %lg %lg %lg",
				   &in[0], &in[1], &in[2], &in[3], &in[4], &in[5], &in[6], &in[7]) !=8)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering line %d of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		if (ndata_xyz == n_alloc) {
			n_alloc <<= 1;
			triang = gmt_M_memory (GMT, triang, n_alloc, struct TRIANG);
			if (Ctrl->T.m_var1)
				Ctrl->box.mag_int = gmt_M_memory (GMT, Ctrl->box.mag_int, n_alloc, double);
			else if (Ctrl->T.m_var2)
				okabe_mag_var2 = gmt_M_memory (GMT, okabe_mag_var2, n_alloc, struct MAG_VAR2);
			else if (Ctrl->T.m_var3)
				okabe_mag_var3 = gmt_M_memory (GMT, okabe_mag_var3, n_alloc, struct MAG_VAR3);
			else
				okabe_mag_var4 = gmt_M_memory (GMT, okabe_mag_var4, n_alloc, struct MAG_VAR4);
		}
		triang[ndata_xyz].x = (Ctrl->box.is_geog) ? (in[0] - *lon_0) * Ctrl->box.d_to_m * cos(in[1]*D2R) : in[0];
		triang[ndata_xyz].y = (Ctrl->box.is_geog) ? -(in[1] - *lat_0) * Ctrl->box.d_to_m : -in[1]; /* - because y must be positive 'south'*/
		triang[ndata_xyz].z = in[2] * Ctrl->D.dir;
		if (Ctrl->T.m_var1)
			Ctrl->box.mag_int[ndata_xyz] = in[3];
		else if (Ctrl->T.m_var2) {
			okabe_mag_var2[ndata_xyz].m = in[3];
			okabe_mag_var2[ndata_xyz].m_dip = in[4];
		}
		else if (Ctrl->T.m_var3) {
			okabe_mag_var3[ndata_xyz].m = in[3];
			okabe_mag_var3[ndata_xyz].m_dec = in[4];
			okabe_mag_var3[ndata_xyz].m_dip = in[5];
		}
		else if (Ctrl->T.m_var4) {
			okabe_mag_var4[ndata_xyz].t_dec = in[3];
			okabe_mag_var4[ndata_xyz].t_dip = in[4];
			okabe_mag_var4[ndata_xyz].m = in[5];
			okabe_mag_var4[ndata_xyz].m_dec = in[6];
			okabe_mag_var4[ndata_xyz].m_dip = in[7];
		}
		ndata_xyz++;
	}
	fclose(fp);
	return (ndata_xyz);
}

/* -----------------------------------------------------------------*/
GMT_LOCAL int read_t (struct GMT_CTRL *GMT, char *fname) {
	/* read file with vertex indexes of triangles */
	unsigned int ndata_t;
	size_t n_alloc;
	int in[3];
	char line[GMT_LEN256] = {""};
	FILE *fp = NULL;

	if ((fp = gmt_fopen (GMT, fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata_t = 0;
	vert = gmt_M_memory (GMT, NULL, n_alloc, struct VERT);

	while (fgets (line, GMT_LEN256, fp)) {
		if (sscanf (line, "%d %d %d", &in[0], &in[1], &in[2]) !=3)
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering line %d of %s\n", ndata_t+1, fname);
		if (ndata_t == n_alloc) {
			n_alloc <<= 1;
			vert = gmt_M_memory (GMT, vert, n_alloc, struct VERT);
               	}
		if (in[0] < 0 || in[1] < 0 || in[2] < 0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Negative indices for line %d of %s\n", ndata_t+1, fname);

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
GMT_LOCAL int read_raw (struct GMT_CTRL *GMT, char *fname, double z_dir) {
	/* read a file with triagles in the raw format and returns nb of triangles */
	unsigned int ndata_r;
	size_t n_alloc;
	double in[9];
	char line[GMT_LEN256] = {""};
	FILE *fp = NULL;

	if ((fp = gmt_fopen (GMT, fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata_r = 0;
	raw_mesh = gmt_M_memory (GMT, NULL, n_alloc, struct RAW);

	while (fgets (line, GMT_LEN256, fp)) {
		if(sscanf (line, "%lg %lg %lg %lg %lg %lg %lg %lg %lg",
			   &in[0], &in[1], &in[2], &in[3], &in[4], &in[5], &in[6], &in[7], &in[8]) !=9)
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering line %d of %s\n", ndata_r+1, fname);
              	if (ndata_r == n_alloc) {
			n_alloc <<= 1;
			raw_mesh = gmt_M_memory (GMT, raw_mesh, n_alloc, struct RAW);
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
GMT_LOCAL int read_stl (struct GMT_CTRL *GMT, char *fname, double z_dir) {
	/* read a file with triagles in the stl format and returns nb of triangles */
	unsigned int ndata_s;
	size_t n_alloc;
	double in[3];
	char line[GMT_LEN256] = {""}, text[128] = {""}, ver_txt[128] = {""};
	FILE *fp = NULL;

	if ((fp = gmt_fopen (GMT, fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata_s = 0;
	raw_mesh = gmt_M_memory (GMT, NULL, n_alloc, struct RAW);

	while (fgets (line, GMT_LEN256, fp)) {
		sscanf (line, "%s", text);
		if (strcmp (text, "outer") == 0) {
			if (fgets (line, GMT_LEN256, fp) == NULL) /* get first vertex */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR reading outer first vertex of \n", fname);
			if (sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering triangle %d of %s\n", ndata_s+1, fname);
			raw_mesh[ndata_s].t1[0] = in[0];
			raw_mesh[ndata_s].t1[1] = -in[1];
			raw_mesh[ndata_s].t1[2] = in[2] * z_dir;
			if (fgets (line, GMT_LEN256, fp) == NULL) /* get second vertex */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR reading outer second vertex of \n", fname);
			if (sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering triangle %d of %s\n", ndata_s+1, fname);
			raw_mesh[ndata_s].t2[0] = in[0];
			raw_mesh[ndata_s].t2[1] = -in[1];
			raw_mesh[ndata_s].t2[2] = in[2] * z_dir;
			if (fgets (line, GMT_LEN256, fp) == NULL) /* get third vertex */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR reading outer third vertex of \n", fname);
			if (sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering triangle %d of %s\n", ndata_s+1, fname);
			raw_mesh[ndata_s].t3[0] = in[0];
			raw_mesh[ndata_s].t3[1] = -in[1];
			raw_mesh[ndata_s].t3[2] = in[2] * z_dir;
			ndata_s++;
              		if (ndata_s == n_alloc) { /* with bad luck we have a flaw here */
				n_alloc <<= 1;
				raw_mesh = gmt_M_memory (GMT, raw_mesh, n_alloc, struct RAW);
			}
		}
		else
			continue;
	}
	fclose(fp);
	return (ndata_s);
}

/* -----------------------------------------------------------------*/
GMT_LOCAL int facet_triangulate (struct XYZOKB_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool bat) {
	/* Sets coordinates for the facet whose effect is being calculated */
	double x_a, x_b, x_c, y_a, y_b, y_c, z_a, z_b, z_c;
	gmt_M_unused (bat);
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

	body_verts[0].z = z_a;		body_verts[1].z = z_b;
	body_verts[2].z = z_c;		body_verts[3].z = Ctrl->Z.z0;
	body_verts[4].z = Ctrl->Z.z0;	body_verts[5].z = Ctrl->Z.z0;
	if (fabs(body_verts[0].z - body_verts[3].z) > Ctrl->E.dz || fabs(body_verts[1].z - body_verts[4].z) >
	    Ctrl->E.dz || fabs(body_verts[2].z - body_verts[5].z) > Ctrl->E.dz)
		return 1;
	else
		return (0);

#if 0
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
#endif
}

/* -----------------------------------------------------------------*/
GMT_LOCAL int facet_raw (struct XYZOKB_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool geo) {
	/* Sets coordinates for the facet in the RAW format */
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
	return 1; /* Always return 1 */
}

/* ---------------------------------------------------------------------- */
GMT_LOCAL void set_center (unsigned int n_triang) {
	/* Calculates triangle center by an approximate (iterative) formula */
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
GMT_LOCAL void triang_norm (int n_triang) {
	/* Computes the unit normal to triangular facet */
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

GMT_LOCAL int check_triang_cw (unsigned int n, unsigned int type) {
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
