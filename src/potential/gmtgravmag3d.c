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
#define THIS_MODULE_KEYS	"<D{,TD(,FD(,MD(,GG},>D)"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVf"

struct GMTGRAVMAG3D_CTRL {
	struct GMTGRAVMAG3D_C {	/* -C */
		bool active;
		double rho;
	} C;
	struct GMTGRAVMAG3D_D {	/* -D */
		bool active;
		double dir;
	} D;
	struct GMTGRAVMAG3D_E {	/* -T */
		bool active;
		double dz;
	} E;
	struct GMTGRAVMAG3D_F {	/* -F<grdfile> */
		bool active;
		char *file;
	} F;
	struct GMTGRAVMAG3D_G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct GMTGRAVMAG3D_H {	/* -H */
		bool active;
		double	t_dec, t_dip, m_int, m_dec, m_dip;
	} H;
	struct GMTGRAVMAG3D_I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct GMTGRAVMAG3D_L {	/* -L */
		bool active;
		double zobs;
	} L;
	struct GMTGRAVMAG3D_M {	/* -M for model body(ies) */
		bool active;
		int type[7][10];
		double params[7][10][9];	/* 7 bodies with at most 9 parameters */
	} M;
	struct GMTGRAVMAG3D_S {	/* -S */
		bool active;
		double radius;
	} S;
	struct GMTGRAVMAG3D_Z {	/* -Z */
		bool active;
		double z0;
	} Z;
	struct GMTGRAVMAG3D_T {	/* -T */
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
	struct GMTGRAVMAG3D_box {	/* No option, just a container */
		bool is_geog;
		double	d_to_m, *mag_int, lon_0, lat_0;
	} box;
	int  n_triang, n_vert, n_raw_triang;
	int  npts_circ;		/* Number of points in which a circle is descretized. */
	int  n_slices;		/* Spheres and Ellipsoides are made by 2*n_slices. Bells are made by n_slices. */
	int  n_sigmas;		/* Number of sigmas which will determine the bell's base width */
	struct GMTGRAVMAG3D_XYZ *triang;
	struct GMTGRAVMAG3D_XYZ *t_center;
	struct GMTGRAVMAG3D_RAW *raw_mesh;
	struct MAG_VAR2 *okabe_mag_var2;
	struct MAG_VAR3 *okabe_mag_var3;
	struct MAG_VAR4 *okabe_mag_var4;
	struct GMTGRAVMAG3D_VERT *vert;
};

struct GMTGRAVMAG3D_XYZ {
	double  x, y, z;
};

struct  GMTGRAVMAG3D_VERT {
	unsigned int  a, b, c;
};

struct GMTGRAVMAG3D_RAW {
	double  t1[3], t2[3], t3[3];
};

struct MAG_VAR2 {
	double	m, m_dip;
};

struct MAG_VAR3 {
	double	m, m_dec, m_dip;
};

struct MAG_VAR4 {
	double	t_dec, t_dip, m, m_dec, m_dip;
};

enum GMT_enum_body {
	BELL = 0,
	CYLINDER,
	CONE,
	ELLIPSOID,
	PRISM,
	PYRAMID,
	SPHERE
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTGRAVMAG3D_CTRL *C = gmt_M_memory (GMT, NULL, 1, struct GMTGRAVMAG3D_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->L.zobs = 0;
	C->D.dir  = -1;
	C->S.radius  = 50000;
	C->npts_circ = 24;
	C->n_slices  = 5;
	C->n_sigmas  = 2;
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTGRAVMAG3D_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->F.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->T.xyz_file);
	gmt_M_str_free (C->T.t_file);
	gmt_M_str_free (C->T.raw_file);
	gmt_M_str_free (C->T.stl_file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int read_stl (struct GMT_CTRL *GMT, struct GMTGRAVMAG3D_CTRL *Ctrl);
GMT_LOCAL void set_center (struct GMTGRAVMAG3D_CTRL *Ctrl);
GMT_LOCAL int facet_triangulate (struct GMTGRAVMAG3D_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool bat);
GMT_LOCAL int facet_raw (struct GMTGRAVMAG3D_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool geo);
GMT_LOCAL int check_triang_cw (struct GMTGRAVMAG3D_CTRL *Ctrl, unsigned int n, unsigned int type);

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<xyz_file>] -Tv<vert_file> | -Tr|s<raw_file> | -M+s<body>/<pars> [-C<density>] [-E<thickness>] "
		"[-F<xy_file>] [-G%s] [-H<f_dec>/<f_dip>/<m_int></m_dec>/<m_dip>] [%s] [-L<z_observation>] [%s] "
		"[-S<radius>] [-Z<level>] [%s] [-fg] [%s] [%s]\n",
		name, GMT_OUTGRID, GMT_I_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_r_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<xyz_file>");
	GMT_Usage (API, -2, "One or more data files (in ASCII, binary, netCDF) with data; see -T for format. If no files are given, standard input is read");
	GMT_Usage (API, 1, "\n-Tv<vert_file> | -Tr|s<raw_file>");
	GMT_Usage (API, -2, "Give names of xyz and vertex (-Tv<fname>) files defining a closed surface. "
		"If <xyz_file> has more then 3 columns it means variable magnetization; see docs for more details. "
		"The file formats correspond to the output of the triangulate program. "
		"Alternatively, use directives to indicate specific formats:");
	GMT_Usage (API, 3, "r: Append <file> in raw triangle format (x1 y1 z1 x2 ... z3).");
	GMT_Usage (API, 3, "s: Append <file> in STL format.");
	GMT_Usage (API, 1, "\nOR");
	GMT_Usage (API, 1, "\n-M+s<body>/<pars>");
	GMT_Usage (API, -2, "Select among one or more of the following bodies and append <pars>, where x0 and y0 are the horizontal coordinates "
		"of the body center [default to 0,0], npts is the number of points that a circle is discretized and n_slices "
		"apply when bodies are made by a pile of slices. For example Spheres and Ellipsoids are made of 2*n_slices and "
		"Bells have n_slices [Default 5].");
	GMT_Usage (API, 3, "%s Gaussian: Append bell,height/sx/sy/z0[/x0/y0/n_sig/npts/n_slices] "
		"for a Gaussian of height <height> with characteristic STDs <sx> and <sy>. The base "
		"width (at depth <z0>) is controlled by the number of sigmas (<n_sig>) [Default = 2].", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s Cylinder: Append cylinder,rad/height/z0[/x0/y0/npts/n_slices] for a "
		"cylinder of radius <rad> height <height> and base at depth <z0>.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s Cone: Append cone,semi_x/semi_y/height/z0[/x0/y0/npts] for a "
		"cone of semi axes <semi_x/semi_y> height <height> and base at depth <z0>.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s Ellipsoid: Append ellipsoid,semi_x/semi_y/semi_z/z_center[/x0/y0/npts/n_slices] for an "
		"ellipsoid of semi axes <semi_x/semi_y/semi_z> and center depth <z_center>.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s Prism: Append prism,side_x/side_y/side_z/z0[/x0/y0] for a "
		"prism of sides <x/y/z> and base at depth <z0>.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s Pyramid: Append pyramid,side_x/side_y/height/z0[/x0/y0] for a "
		"pyramid of sides <x/y> height <height> and base at pth <z0>.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s Sphere: Append sphere,rad/z_center[/x0/y0/npts/n_slices] for a "
		"sphere of radius <rad> and center at depth <z_center>.", GMT_LINE_BULLET);
	GMT_Usage (API, -2, "Note: It is possible to select more than one body. For example, "
		"-M+sprism,1/1/1/-5/-10/1+ssphere,1/-5.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-C<density>");
	GMT_Usage (API, -2, "Set body <density> in SI units.");
	GMT_Usage (API, 1, "\n-E<thickness>");
	GMT_Usage (API, -2, "Give layer thickness in m [Default = 0 m].");
	GMT_Usage (API, 1, "\n-F<xy_file>");
	GMT_Usage (API, -2, "Pass locations where anomaly is going to be computed.");
	gmt_outgrid_syntax (API, 'G', "Set name of the output grid file");
	GMT_Usage (API, 1, "\n-H<f_dec>/<f_dip>/<m_int></m_dec>/<m_dip>");
	GMT_Usage (API, -2, "Append parameters for computation of magnetic anomaly:");
	GMT_Usage (API, 3, "%s <f_dec>/<f_dip> -> geomagnetic declination/inclination.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s <m_int></m_dec>/<m_dip> -> body magnetic intensity/declination/inclination.", GMT_LINE_BULLET);
	GMT_Option (API, "I");
	GMT_Usage (API, 1, "\n-L<z_observation>");
	GMT_Usage (API, -2, "Set level of observation [Default = 0].");
	GMT_Option (API, "R");
	GMT_Usage (API, 1, "\n-S<radius>");
	GMT_Usage (API, -2, "Set search radius in km.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-Z<level>");
	GMT_Usage (API, -2, "Set z level of reference plane [Default = 0].");
	GMT_Option (API, "bi");
	GMT_Usage (API, 1, "\n-fg Converts geographic grids to meters using a \"Flat Earth\" approximation.");
	GMT_Option (API, "r,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GMTGRAVMAG3D_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to gmtgravmag3d and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int j, pos = 0, n_errors = 0, n_files = 0;
	int n_par, err_npar = 0, nBELL = 0, nCIL = 0, nPRI = 0, nCONE = 0, nELL = 0, nPIR = 0, nSPHERE = 0;
	char p[GMT_LEN16] = {""}, p2[GMT_LEN16] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				Ctrl->T.xyz_file = strdup(opt->arg);
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'B':	/* For backward compat (Undocumented) */
			case 'H':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->H.active);
				if ((sscanf(opt->arg, "%lf/%lf/%lf/%lf/%lf",
					    &Ctrl->H.t_dec, &Ctrl->H.t_dip, &Ctrl->H.m_int, &Ctrl->H.m_dec, &Ctrl->H.m_dip)) != 5) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -H: Can't dechiper values\n");
					n_errors++;
				}
				Ctrl->H.active = true;
				Ctrl->C.active = false;
				break;
			case 'C':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.rho = atof (opt->arg) * 6.674e-6;
				Ctrl->C.active = true;
				Ctrl->H.active = false;
				break;
			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				Ctrl->D.dir = 1;
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				Ctrl->F.file = strdup (opt->arg);
				break;
			case 'G':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file))) n_errors++;
				break;
			case 'I':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				if (gmt_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					gmt_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.active = true;
				Ctrl->L.zobs = atof (opt->arg);
				break;
			case 'M':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				Ctrl->M.active = true;

				while (gmt_strtok (opt->arg, ",", &pos, p)) {		/* -M+cone,a/b/c+ellipe,a/b/c/d */
					if (p[0] != '+' && p[1] != 's') {
						GMT_Report (API, GMT_MSG_ERROR, "Model option must start with a +s<code> and not %s\n", p);
						return GMT_PARSE_ERROR;
					}
					gmt_strtok(opt->arg, "+", &pos, p2);	/* Get the string with the model parameters */
					if (pos < strlen(opt->arg)) pos--;		/* Need to receed 1 due to the (p[0] != '+') test */
					if (!strcmp(&p[2], "bell")) {
						n_par = sscanf (p2, "%lg/%lg/%lg/%lg/%lg/%lg/%lg/%lg/%lg", &Ctrl->M.params[BELL][nBELL][0], &Ctrl->M.params[BELL][nBELL][1], &Ctrl->M.params[BELL][nBELL][2], &Ctrl->M.params[BELL][nBELL][3], &Ctrl->M.params[BELL][nBELL][4], &Ctrl->M.params[BELL][nBELL][5], &Ctrl->M.params[BELL][nBELL][6], &Ctrl->M.params[BELL][nBELL][7], &Ctrl->M.params[BELL][nBELL][8]);
						if (n_par < 4) err_npar = 1;
						if (n_par < 7)  Ctrl->M.params[BELL][nBELL][6] = Ctrl->n_sigmas;
						if (n_par < 8)  Ctrl->M.params[BELL][nBELL][7] = Ctrl->npts_circ;
						if (n_par < 9)  Ctrl->M.params[BELL][nBELL][8] = Ctrl->n_slices;
						Ctrl->M.type[BELL][nBELL] = BELL;
						nBELL++;
					}
					else if (!strcmp(&p[2], "cylinder")) {
						n_par = sscanf (p2, "%lg/%lg/%lg/%lg/%lg/%lg", &Ctrl->M.params[CYLINDER][nCIL][0], &Ctrl->M.params[CYLINDER][nCIL][1], &Ctrl->M.params[CYLINDER][nCIL][2], &Ctrl->M.params[CYLINDER][nCIL][3], &Ctrl->M.params[CYLINDER][nCIL][4], &Ctrl->M.params[CYLINDER][nCIL][5]);
						if (n_par < 3) err_npar = 1;
						if (n_par < 6)  Ctrl->M.params[CYLINDER][nCIL][5] = Ctrl->npts_circ;
						Ctrl->M.type[CYLINDER][nCIL] = CYLINDER;
						nCIL++;
					}
					else if (!strcmp(&p[2], "cone")) {
						n_par = sscanf (p2, "%lg/%lg/%lg/%lg/%lg", &Ctrl->M.params[CONE][nCONE][0], &Ctrl->M.params[CONE][nCONE][1], &Ctrl->M.params[CONE][nCONE][2], &Ctrl->M.params[CONE][nCONE][3], &Ctrl->M.params[CONE][nCONE][4]);
						if (n_par < 4) err_npar = 1;
						if (n_par == 4)  Ctrl->M.params[CONE][nCONE][4] = Ctrl->npts_circ;
						Ctrl->M.type[CONE][nCONE] = CONE;
						nCONE++;
					}
					else if (!strcmp(&p[2], "ellipsoid")) {
						n_par = sscanf (p2, "%lg/%lg/%lg/%lg/%lg/%lg/%lg/%lg", &Ctrl->M.params[ELLIPSOID][nELL][0], &Ctrl->M.params[ELLIPSOID][nELL][1], &Ctrl->M.params[ELLIPSOID][nELL][2], &Ctrl->M.params[ELLIPSOID][nELL][3], &Ctrl->M.params[ELLIPSOID][nELL][4], &Ctrl->M.params[ELLIPSOID][nELL][5], &Ctrl->M.params[ELLIPSOID][nELL][6], &Ctrl->M.params[ELLIPSOID][nELL][7]);
						if (n_par < 4) err_npar = 1;
						if (n_par < 7)  Ctrl->M.params[ELLIPSOID][nELL][6] = Ctrl->npts_circ;
						if (n_par < 8)  Ctrl->M.params[ELLIPSOID][nELL][7] = Ctrl->n_slices;
						Ctrl->M.type[ELLIPSOID][nELL] = ELLIPSOID;
						nELL++;
					}
					else if (!strcmp(&p[2], "pyramid")) {
						n_par = sscanf (p2, "%lg/%lg/%lg/%lg/%lg/%lg", &Ctrl->M.params[PYRAMID][nPIR][0], &Ctrl->M.params[PYRAMID][nPIR][1], &Ctrl->M.params[PYRAMID][nPIR][2], &Ctrl->M.params[PYRAMID][nPIR][3], &Ctrl->M.params[PYRAMID][nPIR][4], &Ctrl->M.params[PYRAMID][nPIR][5]);
						if (n_par < 4) err_npar = 1;
						Ctrl->M.type[PYRAMID][nPIR] = PYRAMID;
						nPIR++;
					}
					else if (!strcmp(&p[2], "prism")) {
						n_par = sscanf (p2, "%lg/%lg/%lg/%lg/%lg/%lg", &Ctrl->M.params[PRISM][nPRI][0], &Ctrl->M.params[PRISM][nPRI][1], &Ctrl->M.params[PRISM][nPRI][2], &Ctrl->M.params[PRISM][nPRI][3], &Ctrl->M.params[PRISM][nPRI][4], &Ctrl->M.params[PRISM][nPRI][5]);
						if (n_par < 4) err_npar = 1;
						Ctrl->M.type[PRISM][nPRI] = PRISM;
						nPRI++;
					}
					else if (!strcmp(&p[2], "sphere")) {
						n_par = sscanf (p2, "%lg/%lg/%lg/%lg/%lg/%lg", &Ctrl->M.params[SPHERE][nSPHERE][0], &Ctrl->M.params[SPHERE][nSPHERE][1], &Ctrl->M.params[SPHERE][nSPHERE][2], &Ctrl->M.params[SPHERE][nSPHERE][3], &Ctrl->M.params[SPHERE][nSPHERE][4], &Ctrl->M.params[SPHERE][nSPHERE][5]);
						if (n_par < 2) err_npar = 1;
						if (n_par < 5)  Ctrl->M.params[SPHERE][nSPHERE][4] = Ctrl->npts_circ;
						if (n_par < 6)  Ctrl->M.params[SPHERE][nSPHERE][5] = Ctrl->n_slices;
						Ctrl->M.type[SPHERE][nSPHERE] = SPHERE;
						nSPHERE++;
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Unknown model code (%s) in -M option\n", &p[1]);
						return GMT_PARSE_ERROR;
					}
					if (err_npar) {
						GMT_Report (API, GMT_MSG_ERROR, "Model prism option, wrong number of parametrs.\n");
						return GMT_PARSE_ERROR;
					}
				}
				break;
	 		case 'E':
				Ctrl->E.dz = atof (opt->arg);
				Ctrl->E.active = true;
				break;
	 		case 'S':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.active = true;
				Ctrl->S.radius = atof (opt->arg) * 1000;
				break;
			case 'T': 		/* Selected input mesh format */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
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
						if      (pch && pch[0] == '2') Ctrl->T.m_var2 = true;
						else if (pch && pch[0] == '3') Ctrl->T.m_var3 = true;
						else if (pch && pch[0] == '4') Ctrl->T.m_var4 = true;
						else Ctrl->T.m_var1 = true;
						Ctrl->T.xyz_file[strlen(Ctrl->T.xyz_file)-2] = '\0';	/* In any case the "+m" must go out of fname */
					}
				}
				else if (opt->arg[0] == 'v') {
					Ctrl->T.triangulate = true;
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
				break;
			case 'Z':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
				Ctrl->Z.z0 = atof(opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition(GMT, Ctrl->S.active && (Ctrl->S.radius <= 0.0 || gmt_M_is_dnan (Ctrl->S.radius)),
	                                  "Option -S: Radius is NaN or negative\n");
	n_errors += gmt_M_check_condition(GMT, !Ctrl->T.active && !Ctrl->M.active, "Options -T or -M are mandatory\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->T.xyz_file != NULL && Ctrl->T.t_file == NULL,
	                                  "with xyz must provide also vertex (-Tv) file.\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->T.t_file != NULL && Ctrl->T.xyz_file == NULL,
	                                  "Option -T: vertex file provided (-Tv) but not xyz file.\n");
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


/* -------------------------------------------------------------------------*/
GMT_LOCAL int read_xyz(struct GMT_CTRL *GMT, struct GMTGRAVMAG3D_CTRL *Ctrl, struct GMT_OPTION *options, double *lon_0, double *lat_0) {
	/* read xyz[m] file with point data coordinates */
	int n_cols = 0, error;
	unsigned int k, n = 0;
	size_t n_alloc = 10 * GMT_CHUNK;
	char line[GMT_LEN256] = {""};
	double x1, x2, x3, x4, x5, x6, x7, x8;
	struct GMT_RECORD *In = NULL;
	FILE *fp = NULL;

	/* First, count number of columns */
	if ((fp = gmt_fopen (GMT, Ctrl->T.xyz_file, "r")) == NULL) return -1;
	while (fgets (line, GMT_LEN256, fp)) {
		if (line[0] == '#') continue;
		n_cols = sscanf (line, "%lg %lg %lg %lg %lg %lg %lg %lg", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8);
		break;
	}
	fclose(fp);
	if (n_cols < 3 || n_cols == 7 || n_cols > 8) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Wrong number of columns (%d) in file %s\n", n_cols, Ctrl->T.xyz_file);
		return -1;
	}

	if ((error = GMT_Set_Columns (GMT->parent, GMT_IN, (unsigned int)n_cols, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
		return error;
	if (GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR)	/* Establishes data input */
		return GMT->parent->error;

	if (GMT_Begin_IO (GMT->parent, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR)	/* Enables data input and sets access mode */
		return GMT->parent->error;

	Ctrl->triang = gmt_M_memory (GMT, NULL, n_alloc, struct GMTGRAVMAG3D_XYZ);
	Ctrl->T.m_var = (n_cols == 3) ? false : true;		/* x,y,z */
	if (n_cols == 4) {
		Ctrl->T.m_var1 = true;
		Ctrl->box.mag_int = gmt_M_memory (GMT, NULL, n_alloc, double);
	}
	else if (n_cols == 5) {
		Ctrl->T.m_var2 = true;
		Ctrl->okabe_mag_var2 = gmt_M_memory (GMT, NULL, n_alloc, struct MAG_VAR2);
	}
	else if (n_cols == 6) {
		Ctrl->T.m_var3 = true;
		Ctrl->okabe_mag_var3 = gmt_M_memory (GMT, NULL, n_alloc, struct MAG_VAR3);
	}
	else if (n_cols == 8) {
		Ctrl->T.m_var4 = true;
		Ctrl->okabe_mag_var4 = gmt_M_memory (GMT, NULL, n_alloc, struct MAG_VAR4);
	}

	if (n_cols > 3) {				/* A bit ugly doing this here but only now we know enough */
		Ctrl->H.active = true;
		Ctrl->C.active = false;
	}

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (GMT->parent, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
				return (GMT_RUNTIME_ERROR);
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}

		if (In->data == NULL) {
			gmt_quit_bad_record (GMT->parent, In);
			return (GMT->parent->error);
		}

		if (n == n_alloc) {
			n_alloc = (size_t)(n_alloc * 1.7);
			Ctrl->triang = gmt_M_memory (GMT, Ctrl->triang, n_alloc, struct GMTGRAVMAG3D_XYZ);
			if (Ctrl->T.m_var1)
				Ctrl->box.mag_int = gmt_M_memory (GMT, Ctrl->box.mag_int, n_alloc, double);
			else if (Ctrl->T.m_var2)
				Ctrl->okabe_mag_var2 = gmt_M_memory (GMT, Ctrl->okabe_mag_var2, n_alloc, struct MAG_VAR2);
			else if (Ctrl->T.m_var3)
				Ctrl->okabe_mag_var3 = gmt_M_memory (GMT, Ctrl->okabe_mag_var3, n_alloc, struct MAG_VAR3);
			else
				Ctrl->okabe_mag_var4 = gmt_M_memory (GMT, Ctrl->okabe_mag_var4, n_alloc, struct MAG_VAR4);
		}
		Ctrl->triang[n].x = In->data[0];
		Ctrl->triang[n].y = -In->data[1]; /* - because y must be positive 'south'*/
		Ctrl->triang[n].z = In->data[2] * Ctrl->D.dir;
		if (Ctrl->T.m_var1)
			Ctrl->box.mag_int[n] = In->data[3];
		else if (Ctrl->T.m_var2) {
			Ctrl->okabe_mag_var2[n].m     = In->data[3];
			Ctrl->okabe_mag_var2[n].m_dip = In->data[4];
		}
		else if (Ctrl->T.m_var3) {
			Ctrl->okabe_mag_var3[n].m     = In->data[3];
			Ctrl->okabe_mag_var3[n].m_dec = In->data[4];
			Ctrl->okabe_mag_var3[n].m_dip = In->data[5];
		}
		else if (Ctrl->T.m_var4) {
			Ctrl->okabe_mag_var4[n].t_dec = In->data[3];
			Ctrl->okabe_mag_var4[n].t_dip = In->data[4];
			Ctrl->okabe_mag_var4[n].m     = In->data[5];
			Ctrl->okabe_mag_var4[n].m_dec = In->data[6];
			Ctrl->okabe_mag_var4[n].m_dip = In->data[7];
		}
		n++;
	} while (true);

	if (GMT_End_IO (GMT->parent, GMT_IN, 0) != GMT_NOERROR)	/* Disables further data input */
		return GMT->parent->error;

	Ctrl->triang = gmt_M_memory (GMT, Ctrl->triang, (size_t)n, struct GMTGRAVMAG3D_XYZ);
	if      (Ctrl->T.m_var1) Ctrl->box.mag_int = gmt_M_memory (GMT, Ctrl->box.mag_int, (size_t)n, double);
	else if (Ctrl->T.m_var2) Ctrl->okabe_mag_var2 = gmt_M_memory (GMT, Ctrl->okabe_mag_var2, (size_t)n, struct MAG_VAR2);
	else if (Ctrl->T.m_var3) Ctrl->okabe_mag_var3 = gmt_M_memory (GMT, Ctrl->okabe_mag_var3, (size_t)n, struct MAG_VAR3);
	else Ctrl->okabe_mag_var4 = gmt_M_memory (GMT, Ctrl->okabe_mag_var4, (size_t)n, struct MAG_VAR4);

	*lon_0 = 0.;	*lat_0 = 0.;
	if (Ctrl->box.is_geog) {
		double x_min = FLT_MAX, x_max = -FLT_MAX, y_min = FLT_MAX, y_max = -FLT_MAX;
		/* compute the central lat lon For y_min/max we reverse MIN/MAX because we already have multiplied y by -1 above. */
		for (k = 0; k < n; k++) {
			x_min = MIN(Ctrl->triang[k].x, x_min);	x_max = MAX(Ctrl->triang[k].x, x_max);
			y_min = MAX(Ctrl->triang[n].y, y_min);	y_max = MIN(Ctrl->triang[n].y, y_max);
		}
		*lon_0 = (x_min + x_max) / 2;
		*lat_0 = (y_min + y_max) / 2;
		for (k = 0; k < n; k++) {
			Ctrl->triang[k].x =  (Ctrl->triang[k].x - *lon_0) * Ctrl->box.d_to_m * cos(Ctrl->triang[k].y*D2R);
			Ctrl->triang[k].y = (Ctrl->triang[k].y + *lat_0) * Ctrl->box.d_to_m; /* + because we multiplied by -1 above */
		}
	}

	Ctrl->n_triang = n;
	return 0;
}

/* -------------------------------------------------------------------------*/
GMT_LOCAL int read_vertices(struct GMT_CTRL *GMT, struct GMTGRAVMAG3D_CTRL *Ctrl) {
	/* Read precalculated triangulation indices */

	int    n_skipped, error;
	uint64_t seg, row, np, k;
	unsigned int save_col_type[3];
	double d_n = (double)Ctrl->n_triang - 0.5;	/* So we can use > in test near line 806 */
	struct GMT_DATASET *Tin = NULL;
	struct GMT_DATATABLE *T = NULL;

	/* Must switch to Cartesian input and save whatever original input type we have since we are reading integer triplets */
	for (k = 0; k < 3; k++) {
		save_col_type[k] = gmt_M_type (GMT, GMT_IN, k);	/* Remember what we have */
		gmt_set_column_type (GMT, GMT_IN, (unsigned int)k, GMT_IS_FLOAT);	/* And temporarily set to FLOAT */
	}

	if ((error = GMT_Set_Columns (GMT->parent, GMT_IN, 3, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
		return GMT_RUNTIME_ERROR;
	if ((Tin = GMT_Read_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->T.t_file, NULL)) == NULL) {
		return GMT->parent->error;
	}
	for (k = 0; k < 3; k++) gmt_set_column_type (GMT, GMT_IN, (unsigned int)k, save_col_type[k]);	/* Undo the damage above */

	if (Tin->n_columns < 3) {	/* Trouble */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Tv: %s does not have at least 3 columns with indices\n", Ctrl->T.t_file);
		if (GMT_Destroy_Data (GMT->parent, &Tin) != GMT_NOERROR) return GMT->parent->error;
		return GMT_RUNTIME_ERROR;
	}
	T = Tin->table[0];	/* Since we only have one table here */
	Ctrl->vert = gmt_M_memory (GMT, NULL, T->n_records, struct GMTGRAVMAG3D_VERT);	/* Allocate the integer index array */
	for (seg = np = n_skipped = 0; seg < T->n_segments; seg++) {
		for (row = 0; row < T->segment[seg]->n_rows; row++) {
			if (T->segment[seg]->data[0][row] > d_n || T->segment[seg]->data[1][row] > d_n || T->segment[seg]->data[2][row] > d_n)
				n_skipped++;	/* Outside point range */
			else {
				Ctrl->vert[np].a = urint (T->segment[seg]->data[0][row]);
				Ctrl->vert[np].b = urint (T->segment[seg]->data[1][row]);
				Ctrl->vert[np].c = urint (T->segment[seg]->data[2][row]);
				np++;
			}
		}
	}
	if (GMT_Destroy_Data (GMT->parent, &Tin) != GMT_NOERROR)
		return GMT->parent->error;

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Read %d indices triplets from %s.\n", np, Ctrl->T.t_file);
	if (n_skipped)
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Found %d indices triplets exceeding range of known vertices - skipped.\n", n_skipped);

	Ctrl->n_vert = np;
	return 0;
}

/* -----------------------------------------------------------------*/
GMT_LOCAL int read_raw(struct GMT_CTRL *GMT, struct GMTGRAVMAG3D_CTRL *Ctrl) {
	/* read a file with triangles in the raw format and returns nb of triangles */
	unsigned int row, seg, nt;
	struct  GMT_DATASET *In = NULL;

	if ((In = GMT_Read_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IO_ASCII, NULL, Ctrl->T.raw_file, NULL)) == NULL)
		return GMT->parent->error;
	if (In->n_columns < 9) {	/* Trouble */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Ts: %s does not have 9 columns with 3 triang vertices\n", Ctrl->T.raw_file);
		return -1;
	}

	Ctrl->raw_mesh = gmt_M_memory (GMT, NULL, In->n_records, struct GMTGRAVMAG3D_RAW);

	for (seg = nt = 0; seg < In->n_segments; seg++) {
		for (row = 0; row < In->table[0]->n_records; row++) {
			Ctrl->raw_mesh[nt].t1[0] =  In->table[0]->segment[seg]->data[0][row];
			Ctrl->raw_mesh[nt].t1[1] = -In->table[0]->segment[seg]->data[1][row];
			Ctrl->raw_mesh[nt].t1[2] =  In->table[0]->segment[seg]->data[2][row] * Ctrl->D.dir;

			Ctrl->raw_mesh[nt].t2[0] =  In->table[0]->segment[seg]->data[3][row];
			Ctrl->raw_mesh[nt].t2[1] = -In->table[0]->segment[seg]->data[4][row];
			Ctrl->raw_mesh[nt].t2[2] =  In->table[0]->segment[seg]->data[5][row] * Ctrl->D.dir;

			Ctrl->raw_mesh[nt].t3[0] =  In->table[0]->segment[seg]->data[6][row];
			Ctrl->raw_mesh[nt].t3[1] = -In->table[0]->segment[seg]->data[7][row];
			Ctrl->raw_mesh[nt].t3[2] =  In->table[0]->segment[seg]->data[8][row] * Ctrl->D.dir;
			nt++;
		}
	}

	Ctrl->n_raw_triang = In->n_records;
	if (GMT_Destroy_Data (GMT->parent, &In) != GMT_NOERROR) {
		return GMT->parent->error;
	}
	return 0;
}

#include "solids.c"
/* -----------------------------------------------------------------*/
GMT_LOCAL void solids(struct GMT_CTRL *GMT, struct GMTGRAVMAG3D_CTRL *Ctrl) {
	/*  */

	for (int m = 0; m < 7; m++) {
		for (int n = 0; n < 10; n++) {
			if (Ctrl->M.type[m][n]) {
				switch (Ctrl->M.type[m][n]) {
					case BELL:
						five_psoid(GMT, Ctrl, BELL, n, false, false, true, false);
						break;
					case CYLINDER:
						cilindro(GMT, Ctrl, n);
						break;
					case CONE:
						five_psoid(GMT, Ctrl, CONE, n, true, false, false, false);
						break;
					case ELLIPSOID:
						five_psoid(GMT, Ctrl, ELLIPSOID, n, false, false, false, false);
						break;
					case PYRAMID:
						five_psoid(GMT, Ctrl, PYRAMID, n, false, true, false, false);
						break;
					case PRISM:
						prism(GMT, Ctrl, n);
						break;
					case SPHERE:
						five_psoid(GMT, Ctrl, SPHERE, n, false, false, false, false);
						break;
					default:
						break;
				}
			}
		}
	}
}


#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gmtgravmag3d (void *V_API, int mode, void *args) {

	bool bat = true, DO = true;
	unsigned int row, col, i, j, k, kk;
	unsigned int ndata_p = 0, nx_p, ny_p, n_vert_max;
	unsigned int z_th = 0, n_triang = 0, ndata_s = 0, n_swap = 0;
	int error = 0;
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
	struct	GMTGRAVMAG3D_CTRL *Ctrl = NULL;
	struct	GMT_GRID *Gout = NULL;
	struct  GMT_DATASET *Cin = NULL;
	struct  GMT_DATATABLE *point = NULL;
	struct	GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct	GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	struct MAG_PARAM *okabe_mag_param = NULL;
	struct MAG_VAR *okabe_mag_var = NULL;
	struct GMTGRAVMAG3D_VERT *vert = NULL;
	struct MAG_VAR2 *okabe_mag_var2 = NULL;
	struct MAG_VAR3 *okabe_mag_var3 = NULL;
	struct MAG_VAR4 *okabe_mag_var4 = NULL;

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

	okabe_mag_var2 = Ctrl->okabe_mag_var2;		/* Aliases */
	okabe_mag_var3 = Ctrl->okabe_mag_var3;
	okabe_mag_var4 = Ctrl->okabe_mag_var4;

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
			GMT_Report (API, GMT_MSG_ERROR, "Option -F: %s does not have at least 2 columns with coordinates\n", Ctrl->F.file);
			Return (GMT_PARSE_ERROR);
		}
		point   = Cin->table[0];	/* Can only be one table since we read a single file */
		ndata_p = (unsigned int)point->n_records;
		if (point->n_segments > 1) /* case not dealt (or ignored) and should be tested here */
			GMT_Report(API, GMT_MSG_WARNING, "Multi-segment files are not used in gmtgravmag3d. Using first segment only\n");
	}

	if (Ctrl->T.triangulate) { 	/* Read triangle file output from triangulate */
		if ((error = read_xyz(GMT, Ctrl, options, &lon_0, &lat_0)))
			Return (error);

		/* read vertex file */
		if ((error = read_vertices(GMT, Ctrl)))
			Return (error);

		vert = Ctrl->vert;

		Ctrl->t_center = gmt_M_memory (GMT, NULL, Ctrl->n_vert, struct GMTGRAVMAG3D_XYZ);
		/* compute approximate center of each triangle */
		n_swap = check_triang_cw (Ctrl, Ctrl->n_vert, 0);
		set_center (Ctrl);
	}
	else if (Ctrl->T.stl) { 	/* Read STL file defining a closed volume */
		if ((ndata_s = read_stl(GMT, Ctrl)) < 0) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot open file %s\n", Ctrl->T.stl_file);
			Return (GMT_ERROR_ON_FOPEN);
		}
		/*n_swap = check_triang_cw (ndata_s, 1);*/
	}
	else if (Ctrl->T.raw) { 	/* Read RAW file defining a closed volume */
		if ((error = read_raw(GMT, Ctrl)))
			Return (error);

		/*n_swap = check_triang_cw (Ctrl, Ctrl->n_raw_triang, 1);*/
	}
	else if (Ctrl->M.active) {
		solids(GMT, Ctrl);
	}

#if 0
	for (i = 0; i < 24; i++) {
		fprintf(stderr, "%.2f %.2f %.2f  ", Ctrl->raw_mesh[i].t1[0], Ctrl->raw_mesh[i].t1[1], Ctrl->raw_mesh[i].t1[2]);
		fprintf(stderr, "%.2f %.2f %.2f  ", Ctrl->raw_mesh[i].t2[0], Ctrl->raw_mesh[i].t2[1], Ctrl->raw_mesh[i].t2[2]);
		fprintf(stderr, "%.2f %.2f %.2f\n", Ctrl->raw_mesh[i].t3[0], Ctrl->raw_mesh[i].t3[1], Ctrl->raw_mesh[i].t3[2]);
	}
#endif

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
		n_triang = Ctrl->n_vert;
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
	else if (Ctrl->T.raw || Ctrl->T.stl || Ctrl->M.active) {
		n_triang = (Ctrl->T.raw || Ctrl->M.active) ? Ctrl->n_raw_triang : ndata_s;
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
		s_t  = sin(Ctrl->H.m_dip*D2R);
		if (!Ctrl->T.m_var4) {		/* In all the other cases the field parameters are constant */
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
	gmt_M_tic(GMT);
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
		else if (Ctrl->T.raw || Ctrl->T.stl || Ctrl->M.active)
			z_th = facet_raw (Ctrl, body_verts, i, Ctrl->box.is_geog);
		if (z_th) {
			if (Ctrl->G.active) { /* grid */
				for (row = 0; row < Gout->header->n_rows; row++) {
					y_o = (Ctrl->box.is_geog) ? ((y[row]+lat_0) * Ctrl->box.d_to_m): y[row];
					ij = gmt_M_ijp(Gout->header, row, 0);
					for (col = 0; col < Gout->header->n_columns; col++, ij++) {
						x_o = (Ctrl->box.is_geog) ? ((x[col]-lon_0)*Ctrl->box.d_to_m * cos_vec[row]) : x[col];
						if (Ctrl->S.active) {
							DX = Ctrl->t_center[i].x - x_o;
							DY = Ctrl->t_center[i].y - y_o;
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
						DX = Ctrl->t_center[i].x - x_obs[kk];
						DY = Ctrl->t_center[i].y - y_obs[kk];
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
	gmt_M_toc(GMT,"");		/* Print total run time, but only if -Vt was set */
	gmt_M_free (GMT, x);
	gmt_M_free (GMT, y);
	gmt_M_free (GMT, g);
	gmt_M_free (GMT, z_obs);
	gmt_M_free (GMT, x_obs);
	gmt_M_free (GMT, y_obs);
	gmt_M_free (GMT, Ctrl->triang);
	gmt_M_free (GMT, Ctrl->raw_mesh);
	gmt_M_free (GMT, Ctrl->t_center);
	gmt_M_free (GMT, Ctrl->vert);
	gmt_M_free (GMT, okabe_mag_param);
	gmt_M_free (GMT, okabe_mag_var);
	gmt_M_free (GMT, body_desc.n_v);
	gmt_M_free (GMT, body_desc.ind);
	gmt_M_free (GMT, loc_or);
	gmt_M_free (GMT, body_verts);
	gmt_M_free (GMT, cos_vec);
	if (Ctrl->T.m_var1) gmt_M_free (GMT, Ctrl->box.mag_int);
	if (Ctrl->T.m_var2) gmt_M_free (GMT, Ctrl->okabe_mag_var2);
	if (Ctrl->T.m_var3) gmt_M_free (GMT, Ctrl->okabe_mag_var3);
	gmt_M_free (GMT, Ctrl->okabe_mag_var4);

	Return (error);
}

/* -----------------------------------------------------------------*/
GMT_LOCAL int read_stl (struct GMT_CTRL *GMT, struct GMTGRAVMAG3D_CTRL *Ctrl) {
	/* read a file with triagles in the stl format and returns nb of triangles */
	unsigned int ndata_s;
	size_t n_alloc;
	double in[3];
	char line[GMT_LEN256] = {""}, text[128] = {""}, ver_txt[128] = {""};
	FILE *fp = NULL;

	if ((fp = gmt_fopen (GMT, Ctrl->T.stl_file, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata_s = 0;
	Ctrl->raw_mesh = gmt_M_memory (GMT, NULL, n_alloc, struct GMTGRAVMAG3D_RAW);

	while (fgets (line, GMT_LEN256, fp)) {
		sscanf (line, "%s", text);
		if (strcmp (text, "outer") == 0) {
			if (fgets (line, GMT_LEN256, fp) == NULL) /* get first vertex */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR reading outer first vertex of \n", Ctrl->T.stl_file);
			if (sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering triangle %d of %s\n", ndata_s+1, Ctrl->T.stl_file);
			Ctrl->raw_mesh[ndata_s].t1[0] = in[0];
			Ctrl->raw_mesh[ndata_s].t1[1] = -in[1];
			Ctrl->raw_mesh[ndata_s].t1[2] = in[2] * Ctrl->D.dir;
			if (fgets (line, GMT_LEN256, fp) == NULL) /* get second vertex */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR reading outer second vertex of \n", Ctrl->T.stl_file);
			if (sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering triangle %d of %s\n", ndata_s+1, Ctrl->T.stl_file);
			Ctrl->raw_mesh[ndata_s].t2[0] = in[0];
			Ctrl->raw_mesh[ndata_s].t2[1] = -in[1];
			Ctrl->raw_mesh[ndata_s].t2[2] = in[2] * Ctrl->D.dir;
			if (fgets (line, GMT_LEN256, fp) == NULL) /* get third vertex */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR reading outer third vertex of \n", Ctrl->T.stl_file);
			if (sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "ERROR deciphering triangle %d of %s\n", ndata_s+1, Ctrl->T.stl_file);
			Ctrl->raw_mesh[ndata_s].t3[0] = in[0];
			Ctrl->raw_mesh[ndata_s].t3[1] = -in[1];
			Ctrl->raw_mesh[ndata_s].t3[2] = in[2] * Ctrl->D.dir;
			ndata_s++;
			if (ndata_s == n_alloc) { /* with bad luck we have a flaw here */
				n_alloc <<= 1;
				Ctrl->raw_mesh = gmt_M_memory (GMT, Ctrl->raw_mesh, n_alloc, struct GMTGRAVMAG3D_RAW);
			}
		}
		else
			continue;
	}
	fclose(fp);
	return (ndata_s);
}

/* -----------------------------------------------------------------*/
GMT_LOCAL int facet_triangulate (struct GMTGRAVMAG3D_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool bat) {
	/* Sets coordinates for the facet whose effect is being calculated */
	double x_a, x_b, x_c, y_a, y_b, y_c, z_a, z_b, z_c;
	struct GMTGRAVMAG3D_XYZ *triang = Ctrl->triang;
	gmt_M_unused (bat);
	x_a = triang[Ctrl->vert[i].a].x;	x_b = triang[Ctrl->vert[i].b].x;	x_c = triang[Ctrl->vert[i].c].x;
	y_a = triang[Ctrl->vert[i].a].y;	y_b = triang[Ctrl->vert[i].b].y;	y_c = triang[Ctrl->vert[i].c].y;
	z_a = triang[Ctrl->vert[i].a].z;	z_b = triang[Ctrl->vert[i].b].z;	z_c = triang[Ctrl->vert[i].c].z;
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
GMT_LOCAL int facet_raw (struct GMTGRAVMAG3D_CTRL *Ctrl, struct BODY_VERTS *body_verts, unsigned int i, bool geo) {
	/* Sets coordinates for the facet in the RAW format */
	double cos_a, cos_b, cos_c, x_a, x_b, x_c, y_a, y_b, y_c, z_a, z_b, z_c;

	x_a = Ctrl->raw_mesh[i].t1[0];   x_b = Ctrl->raw_mesh[i].t2[0];   x_c = Ctrl->raw_mesh[i].t3[0];
	y_a = Ctrl->raw_mesh[i].t1[1];   y_b = Ctrl->raw_mesh[i].t2[1];   y_c = Ctrl->raw_mesh[i].t3[1];
	z_a = Ctrl->raw_mesh[i].t1[2];   z_b = Ctrl->raw_mesh[i].t2[2];   z_c = Ctrl->raw_mesh[i].t3[2];
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
GMT_LOCAL void set_center (struct GMTGRAVMAG3D_CTRL *Ctrl) {
	/* Calculates triangle center by an approximate (iterative) formula */
	int i, j, k = 5;
	double x, y, z, xa[6], ya[6], xb[6], yb[6], xc[6], yc[6];
	struct GMTGRAVMAG3D_XYZ *triang = Ctrl->triang;

	for (i = 0; i < Ctrl->n_vert; i++) {
		xa[0] = (triang[Ctrl->vert[i].b].x + triang[Ctrl->vert[i].c].x) / 2.;
		ya[0] = (triang[Ctrl->vert[i].b].y + triang[Ctrl->vert[i].c].y) / 2.;
		xb[0] = (triang[Ctrl->vert[i].c].x + triang[Ctrl->vert[i].a].x) / 2.;
		yb[0] = (triang[Ctrl->vert[i].c].y + triang[Ctrl->vert[i].a].y) / 2.;
		xc[0] = (triang[Ctrl->vert[i].a].x + triang[Ctrl->vert[i].b].x) / 2.;
		yc[0] = (triang[Ctrl->vert[i].a].y + triang[Ctrl->vert[i].b].y) / 2.;
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
		z = (triang[Ctrl->vert[i].a].z+triang[Ctrl->vert[i].b].z+triang[Ctrl->vert[i].c].z)/3.;
		Ctrl->t_center[i].x = x;
		Ctrl->t_center[i].y = y;
		Ctrl->t_center[i].z = z;
	}
}

#if 0
GMT_LOCAL void gmtgravmag3d_triang_norm (int n_triang) {
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

GMT_LOCAL int check_triang_cw (struct GMTGRAVMAG3D_CTRL *Ctrl, unsigned int n, unsigned int type) {
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
			x1 = Ctrl->triang[Ctrl->vert[i].a].x;	 y1 = Ctrl->triang[Ctrl->vert[i].a].y;
			x2 = Ctrl->triang[Ctrl->vert[i].b].x;	 y2 = Ctrl->triang[Ctrl->vert[i].b].y;
			x3 = Ctrl->triang[Ctrl->vert[i].c].x;	 y3 = Ctrl->triang[Ctrl->vert[i].c].y;
		}
		else if (type == 1) { /* raw */
			x1 = Ctrl->raw_mesh[i].t1[0];		y1 = Ctrl->raw_mesh[i].t1[1];
			x2 = Ctrl->raw_mesh[i].t2[0];		y2 = Ctrl->raw_mesh[i].t2[1];
			x3 = Ctrl->raw_mesh[i].t3[0];		y3 = Ctrl->raw_mesh[i].t3[1];
		}

		det = (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);

		if (det < 0.0) { /* counter clockwise triangle -> swap vertex order */
			if (type == 0) {
				tmp = Ctrl->vert[i].b;
				Ctrl->vert[i].b = Ctrl->vert[i].c;
				Ctrl->vert[i].c = tmp;
				n_swaped++;
			}
			else if (type == 1) {
				d_tmp[0] = Ctrl->raw_mesh[i].t2[0];
				d_tmp[1] = Ctrl->raw_mesh[i].t2[1];
				d_tmp[2] = Ctrl->raw_mesh[i].t2[2];
				Ctrl->raw_mesh[i].t2[0] = Ctrl->raw_mesh[i].t3[0];
				Ctrl->raw_mesh[i].t2[1] = Ctrl->raw_mesh[i].t3[1];
				Ctrl->raw_mesh[i].t2[2] = Ctrl->raw_mesh[i].t3[2];
				Ctrl->raw_mesh[i].t3[0] = d_tmp[0];
				Ctrl->raw_mesh[i].t3[1] = d_tmp[1];
				Ctrl->raw_mesh[i].t3[2] = d_tmp[2];
				n_swaped++;
			}
		}
	}
	return n_swaped;
}
