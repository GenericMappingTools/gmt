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
 * API functions to support the redpol application.
 *
 * Brief synopsis: 
 *
 *
 * Author:	Joaquim Luis
 * Date:	06-OCT-2000 (original GMT 4)
 * Revised:	07-FEB-2001	
 *
 *	Versao com -S e onde a okabe foi transformada em double
 *	Comecei a ler um fiche em stl 14-12-00
 *
 *	Muita cosmetica e passagem hypot->sqrt 05-01-01
 *	Resolvida(?) a questao da aceleracao para prismas de espessura constante
 *
 *	Mudei DBL_EPSILON para FLT_EPSILON prque senao dava merda 04-02-01
 *	Tirei uma origem orlat orlon  11-02-01
 *
 *	Retirei o orlat e orlon (era estupido)	27-5-01
 *	Added a test for checking (and swaping if needed) the triangle order.	27-5-01
 *
 *	06-02-03
 *	Changed the way computations in geographical coordinates were donne.
 *	Given that the okabe routine requires coordinates in meters, the geographical
 *	coordinates were transformed to meters along paralels (relative to grenwich) and
 *	meridians (relative to equator). This produces a change in the body azimuth as
 *	comparing to coodinates in a map projection (e.g.UTM).
 *	To reduce this effect, I now remove the body's central longitude. However, for bodies
 *	with large longitude span the problem might still persist.
 *
 *	05-10-03
 *	Ta uma grande mixordia com a historia dos z_dir e se "bat" ou "topo". Nao me oriento.
 *	Pus tudo a z positivo up, por isso as opcoes -D e -T nao devem ser usadas
 *
 *	07-11-03
 *	Na modif anterior so tinha retirado o orlon. Agora retiro tambem o orlat.
 *	Nao sei se faz diferenca
 *
 *	17-02-04
 *	No seguimento da mixordice voltei a por o z0 *= z_dir. Isto faz a coisa mais
 *	coerente. Assim, na opcao -Z o <level> deve obedecer ao sinal. Ou seja, -Z-2300
 *	significa que a base está a -2300 m de prof.
 *
 */

#include "gmt_potential.h"

struct XYZOKB_CTRL {
	struct C {	/* -C */
		double rho;
		GMT_LONG active;
	} C;
	struct D {	/* -D */
		double dir;
	} D;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct F {	/* -F<grdfile> */
		GMT_LONG active;
		char *file;
	} F;
	struct G {	/* -G<grdfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct H {	/* -H */
		GMT_LONG active;
		double	t_dec, t_dip, m_int, m_dec, m_dip;
	} H;
	struct L {	/* -L */
		double zobs;
	} L;
	struct M {	/* -M */
		GMT_LONG active;
	} M;
	struct N {	/* No option, just a container */
		double	xx[24], yy[24], zz[24];
		double	d_to_m, *mag_int, central_long, central_lat;
		double	c_tet, s_tet, c_phi, s_phi;
	} N;
	struct E {	/* -T */
		GMT_LONG active;
		double dz;
	} E;
	struct S {	/* -S */
		GMT_LONG active;
		double radius;
	} S;
	struct Z {	/* -Z */
		double z0;
	} Z;
	struct T {	/* -T */
		GMT_LONG active;
		GMT_LONG triangulate;
		GMT_LONG raw;
		GMT_LONG stl;
		GMT_LONG m_var, m_var1, m_var2, m_var3, m_var4;
		char *xyz_file;
		char *t_file;
		char *raw_file;
		char *stl_file;
	} T;
};

struct  DATA    {
        double  x, y;
}       *data;

struct  BODY_DESC {
	GMT_LONG n_f, *n_v, *ind;
}       bd_desc;

struct  LOC_OR    {
        double  x, y, z;
};

struct  TRIANG    {
        double  x, y, z;
}       *triang;

struct  VERT    {
        GMT_LONG  a, b, c;
}       *vert;

struct  TRI_CENTER {
        double  x, y, z;
}       *t_center;

struct  RAW    {
        double  t1[3], t2[3], t3[3];
}       *raw_mesh;

struct MAG_PARAM {
	double	rim[3];
}	*mag_param;

struct MAG_VAR {		/* Used when only the modulus of magnetization varies */
	double	rk[3];
}	*mag_var;

struct MAG_VAR2 {
	double	m, m_dip;
}	*mag_var2;

struct MAG_VAR3 {
	double	m, m_dec, m_dip;
}	*mag_var3;

struct MAG_VAR4 {
	double	t_dec, t_dip, m, m_dec, m_dip;
}	*mag_var4;

void *New_xyzokb_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct XYZOKB_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct XYZOKB_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->L.zobs = 0;
	C->D.dir = -1;
	C->S.radius = 50000;
	C->S.active = TRUE;
	return (C);
}

void Free_xyzokb_Ctrl (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free (C->G.file);
	GMT_free (GMT, C);
}

GMT_LONG read_xyz (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *Ctrl, char *fname, double *central_long, double *central_lat);
GMT_LONG read_t (struct GMT_CTRL *GMT, char *fname);
GMT_LONG read_raw (struct GMT_CTRL *GMT, char *fname, double z_dir);
GMT_LONG read_stl (struct GMT_CTRL *GMT, char *fname, double z_dir);
GMT_LONG read_poly (struct GMT_CTRL *GMT, char *fname, GMT_LONG switch_xy);
void set_center (GMT_LONG n_triang);
double okabe (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *Ctrl, double x, double y, struct BODY_DESC bd_desc, 
	      GMT_LONG km, GMT_LONG pm, struct LOC_OR *loc_or);
double okb_grv (struct XYZOKB_CTRL *Ctrl, GMT_LONG n_vert, struct LOC_OR *loc_or);
double okb_mag (struct XYZOKB_CTRL *Ctrl, GMT_LONG n_vert, GMT_LONG km, GMT_LONG pm, struct LOC_OR *loc_or); 
double eq_30 (double c, double s, double x, double y, double z);
double eq_43 (double mz, double c, double tg, double auxil, double x, double y, double z);
void rot_17 (struct XYZOKB_CTRL *Ctrl, GMT_LONG n_vert, GMT_LONG top, struct LOC_OR *loc_or);
GMT_LONG facet_triangulate (struct XYZOKB_CTRL *Ctrl, GMT_LONG i, GMT_LONG bat);
GMT_LONG facet_raw (struct XYZOKB_CTRL *Ctrl, GMT_LONG i, GMT_LONG geo);
GMT_LONG check_triang_cw (GMT_LONG n, GMT_LONG type);

GMT_LONG GMT_xyzokb_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {

	struct GMT_CTRL *GMT = C->GMT;
	GMT_message (GMT, "xyzokb - Compute the gravity/magnetic anomaly of a body by the method of Okabe\n\n");
	GMT_message (GMT, "usage: xyzokb [-C<density>] [-G<outgrid>] [-R<w>/<e>/<s></n>]\n");
	GMT_message (GMT, "\t[-E<thick>] [-F<xy_file>] [-L<z_observation>] [-M]\n");
	GMT_message (GMT, "\t[-H<f_dec>/<f_dip>/<m_int></m_dec>/<m_dip>] [-S<radius>]\n");
	GMT_message (GMT, "\t[-T[<[d]xyz_file>/<vert_file>[/m]]|<[r|s]raw_file>] [-Z<level>] [-V]\n");

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);
	
	GMT_message (GMT, "\t-H sets parameters for computation of magnetic anomaly\n");
	GMT_message (GMT, "\t   f_dec/f_dip -> geomagnetic declination/inclination\n");
	GMT_message (GMT, "\t   m_int/m_dec/m_dip -> body magnetic intensity/declination/inclination\n");
	GMT_message (GMT, "\t-C sets body density in SI\n");
	GMT_message (GMT, "\t-F pass locations where anomaly is going to be computed\n");
	GMT_message (GMT, "\t-G name of the output grdfile.\n");
	GMT_message (GMT, "\t-I sets the grid spacing for the grid.  Append m for minutes, c for seconds\n");
	GMT_message (GMT, "\t-L sets level of observation [Default = 0]\n");
	GMT_message (GMT, "\t-M Map units TRUE; x,y in degrees, dist units in m [Default dist unit = x,y unit].\n");
	GMT_message (GMT, "\t-E give layer thickness in m [Default = 0 m]\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-S search radius in km\n");
	GMT_message (GMT, "\t-T Give either names of xyz[m] and vertex files or of a file defining a close surface.\n");
	GMT_message (GMT, "\t   In the first case append an 'd' imediatly after -T and optionaly a /m after the vertex file name.\n");
	GMT_message (GMT, "\t   In the second case append an 'r' or a 's' imediatly after -T and before the file name.\n");
	GMT_message (GMT, "\t   'r' and 's' stand for files in raw (x1 y1 z1 x2 ... z3) or STL format.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-Z z level of reference plane [Default = 0]\n");
	GMT_explain_options (GMT, "C0:.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_xyzokb_parse (struct GMTAPI_CTRL *C, struct XYZOKB_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to redpol and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j, n_errors = 0, n_files = 0, pos = 0;
	char	ptr[GMT_TEXT_LEN256];
	struct	GMT_OPTION *opt = NULL;
	struct	GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'B':	/* For backward compat (Undocumented) */
			case 'H':
				if ((sscanf(opt->arg, "%lf/%lf/%lf/%lf/%lf", 
					    &Ctrl->H.t_dec, &Ctrl->H.t_dip, &Ctrl->H.m_int, &Ctrl->H.m_dec, &Ctrl->H.m_dip)) != 5) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -H option: Can't dechiper values\n");
					n_errors++;
				}
				Ctrl->H.active = TRUE;
				Ctrl->C.active = FALSE;
				break;
			case 'C':
				Ctrl->C.rho = atof (opt->arg) * 6.674e-6;
				Ctrl->C.active = TRUE;
				Ctrl->H.active = FALSE;
				break;
			case 'D':
				Ctrl->D.dir = 1;
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				Ctrl->F.file = strdup (opt->arg); 
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':
				Ctrl->L.zobs = atof (opt->arg);
				break;

			case 'M':
				Ctrl->M.active = TRUE;
				break;
	 		case 'P':		/* For backward compat of pre GMT version */
	 		case 'E':
				Ctrl->E.dz = atof (opt->arg);
				Ctrl->E.active = TRUE;
				break;
	 		case 'S':
				Ctrl->S.radius = atof (opt->arg) * 1000;
				Ctrl->S.active = FALSE;
				break;
	 		case 't':		/* For backward compat of pre GMT version */
			case 'T': 		/* Selected input mesh format */ 
				switch (opt->arg[0]) {
					case 'd':	/* Surface computed by triangulate */
						j = 0;
						while (GMT_strtok (GMT, &opt->arg[1], "/", &pos, ptr)) {
							switch (j) {
								case 0:
									Ctrl->T.xyz_file = strdup(ptr);
									break;
								case 1:
									Ctrl->T.t_file = strdup(ptr);
									break;
								case 2:
									Ctrl->T.m_var = TRUE;
									Ctrl->H.active = TRUE;
									Ctrl->C.active = FALSE;
									if (ptr[1] == '2') Ctrl->T.m_var2 = TRUE;
									else if (ptr[1] == '3') Ctrl->T.m_var3 = TRUE;
									else if (ptr[1] == '4') Ctrl->T.m_var4 = TRUE;
									else Ctrl->T.m_var1 = TRUE;
									break;
								default:
									break;
							}
							j++;
						}
						if (j != 2 && j != 3) {
							GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -T option: Must give names for data points and vertex files\n");
							n_errors++;
						}
						Ctrl->T.triangulate = TRUE;
						break;
					case 'r':	/* Closed volume in RAW format */
 						Ctrl->T.raw_file = strdup(&opt->arg[1]);
						Ctrl->T.raw = TRUE;
						break;
					case 's':	/* Closed volume in STL1format */
 						Ctrl->T.stl_file = strdup(&opt->arg[1]);
						Ctrl->T.stl = TRUE;
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
	GMT_check_condition (GMT, Ctrl->G.active && Ctrl->F.active, "Warning: -F overrides -G\n");
	if (GMT_check_condition (GMT, Ctrl->T.raw && !Ctrl->S.active, "Warning: -Tr overrides -S\n"))
		Ctrl->S.active = TRUE;

	/*n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");*/

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_xyzokb_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_xyzokb (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args) {

	GMT_LONG error = FALSE;
	GMT_LONG row, col, i, j, ij, k;
	GMT_LONG bat = TRUE, switch_xy = FALSE, DO = TRUE;
	GMT_LONG kk, nm, ndata_r = 0;
	GMT_LONG ndata_p = 0, ndata_xyz = 0, ndata_t = 0, nx_p, ny_p, n_vert_max;
	GMT_LONG z_th = 0, n_triang = 0, ndata_s = 0, n_swap = 0;
	GMT_LONG km, pm;		/* index of current body facet (for mag only) */
	float	*g = NULL, one_100;
	double	s_rad2;
	double	t_mag, a, DX, DY, DZ;
	double	*x_obs = NULL, *y_obs = NULL, *z_obs = NULL, *x = NULL, *y = NULL, *cos_vec = NULL;
	double	cc_t, cs_t, s_t;
	double	central_long = 0, central_lat = 0;

	struct	LOC_OR *loc_or;
	struct	XYZOKB_CTRL *Ctrl = NULL;
	struct	GMT_GRID *Gout = NULL;
	struct	GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	data = NULL, triang = NULL, vert = NULL, t_center = NULL, raw_mesh = NULL, mag_param = NULL;
	mag_var = NULL, mag_var2 = NULL, mag_var3 = NULL, mag_var4 = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) 
		bailout (GMT_xyzokb_usage (API, GMTAPI_USAGE));		/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) 
		bailout (GMT_xyzokb_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_xyzokb", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VR:", "", options))) Return (error);
	Ctrl = New_xyzokb_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_xyzokb_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the redpol main code ----------------------------*/

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) 	/* Enables data input and sets access mode */
		Return (error);

	if (!Ctrl->M.active) 
		Ctrl->N.d_to_m = 1.;
	else
		Ctrl->N.d_to_m = 2.0 * M_PI * 6371008.7714 / 360.0;

	Ctrl->Z.z0 *= Ctrl->D.dir;

	/*d_to_m = 2.0 * M_PI * gmtdefs.ellipse[N_ELLIPSOIDS-1].eq_radius / 360.0;*/
	/* ---- Read files section ---------------------------------------------------- */

	if (Ctrl->F.active) { 		/* Read xy file where anomaly is to be computed */
		if ( (ndata_p = read_poly (GMT, Ctrl->F.file, switch_xy)) < 0 ) {
			GMT_report (GMT, GMT_MSG_FATAL, "Cannot open file %s\n", Ctrl->F.file);
			return (EXIT_FAILURE);
		}
	}

	if (Ctrl->T.triangulate) { 	/* Read triangle file output from triangulate */
		if ( (ndata_xyz = read_xyz (GMT, Ctrl, Ctrl->T.xyz_file, &central_long, &central_lat)) < 0 ) {
			GMT_report (GMT, GMT_MSG_FATAL, "Cannot open file %s\n", Ctrl->T.xyz_file);
			return (EXIT_FAILURE);
		}
		/* read vertex file */
		if ( (ndata_t = read_t (GMT, Ctrl->T.t_file)) < 0 ) {
			GMT_report (GMT, GMT_MSG_FATAL, "Cannot open file %s\n", Ctrl->T.t_file);
			return (EXIT_FAILURE);
		}

		t_center = GMT_memory (GMT, NULL, (size_t) ndata_t, struct TRI_CENTER);
		/* compute aproximate center of each triangle */
		n_swap = check_triang_cw (ndata_t, 0);
		set_center (ndata_t);
	}
	else if (Ctrl->T.stl) { 	/* Read STL file defining a closed volume */
		if ( (ndata_s = read_stl (GMT, Ctrl->T.stl_file, Ctrl->D.dir)) < 0 ) {
			GMT_report (GMT, GMT_MSG_FATAL, "Cannot open file %s\n", Ctrl->T.stl_file);
			return (EXIT_FAILURE);
		}
		/*n_swap = check_triang_cw (ndata_s, 1);*/
	}
	else if (Ctrl->T.raw) { 	/* Read RAW file defining a closed volume */
		if ( (ndata_r = read_raw (GMT, Ctrl->T.raw_file, Ctrl->D.dir)) < 0 ) {
			GMT_report (GMT, GMT_MSG_FATAL, "Cannot open file %s\n", Ctrl->T.raw_file);
			return (EXIT_FAILURE);
		}
		/*n_swap = check_triang_cw (ndata_r, 1);*/
	}

	if (n_swap > 0)
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning: %ld triangles had ccw order\n", n_swap);
/* ---------------------------------------------------------------------------- */

	if (Ctrl->G.active) {
		Gout = GMT_Create_Data (API, GMT_IS_GRID, NULL);
		GMT_grd_init (GMT, Gout->header, options, FALSE);
		/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
		GMT_err_fail (GMT, GMT_init_newgrid (GMT, Gout, GMT->common.R.wesn, Ctrl->I.inc, FALSE), Ctrl->G.file);
	
		GMT_report (GMT, GMT_MSG_NORMAL, "Grid dimensions are nx = %d, ny = %d\n", Gout->header->nx, Gout->header->ny);
		Gout->data = GMT_memory (GMT, NULL, Gout->header->size, float);

		/* Build observation point vectors */
		x = GMT_memory (GMT, NULL, (size_t) Gout->header->nx, double);
		y = GMT_memory (GMT, NULL, (size_t) Gout->header->ny, double);
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
	x_obs = GMT_memory (GMT, NULL, (size_t) nx_p, double);
	y_obs = GMT_memory (GMT, NULL, (size_t) ny_p, double);
	z_obs = GMT_memory (GMT, NULL, (size_t) nx_p, double);

	if (Ctrl->F.active) { /* Need to compute observation coords only once */
		for (i = 0; i < ndata_p; i++) {
			x_obs[i] = (Ctrl->M.active) ? (data[i].x-central_long)*Ctrl->N.d_to_m*cos(data[i].y*D2R) : data[i].x;
			y_obs[i] = (Ctrl->M.active) ? -(data[i].y-central_lat)*Ctrl->N.d_to_m : data[i].y; /* - because y positive 'south' */
		}
		g = GMT_memory (GMT, NULL, (size_t) nm, float); 
	}

	if (Ctrl->T.triangulate) {
		n_triang = ndata_t;
		bd_desc.n_f = 5;		/* Number of prism facets */
		bd_desc.n_v = GMT_memory (GMT, NULL, (size_t) (bd_desc.n_f), GMT_LONG);
		bd_desc.n_v[0] = 3;	bd_desc.n_v[1] = 3;
		bd_desc.n_v[2] = 4;	bd_desc.n_v[3] = 4;
		bd_desc.n_v[4] = 4;
		bd_desc.ind = GMT_memory (GMT, NULL, (size_t) (bd_desc.n_v[0] + bd_desc.n_v[1] +
			bd_desc.n_v[2] + bd_desc.n_v[3] + bd_desc.n_v[4]), GMT_LONG);
		bd_desc.ind[0] = 0;	bd_desc.ind[1] = 1; 	bd_desc.ind[2] = 2;	/* top triang */
		bd_desc.ind[3] = 3;	bd_desc.ind[4] = 5; 	bd_desc.ind[5] = 4;	/* bot triang */
		bd_desc.ind[6] = 1;	bd_desc.ind[7] = 4; 	bd_desc.ind[8] = 5;	bd_desc.ind[9] = 2;
		bd_desc.ind[10] = 0;	bd_desc.ind[11] = 3;	bd_desc.ind[12] = 4;	bd_desc.ind[13] = 1;
	 	bd_desc.ind[14] = 0;	bd_desc.ind[15] = 2;	bd_desc.ind[16] = 5;	bd_desc.ind[17] = 3;
	}
	else if (Ctrl->T.raw || Ctrl->T.stl) {
		n_triang = (Ctrl->T.raw) ? ndata_r : ndata_s;
		bd_desc.n_f = 1;
		bd_desc.n_v = GMT_memory (GMT, NULL, (size_t) (bd_desc.n_f), GMT_LONG);
		bd_desc.n_v[0] = 3;
		bd_desc.ind = GMT_memory (GMT, NULL, (size_t) (bd_desc.n_v[0]), GMT_LONG);
		bd_desc.ind[0] = 0;	bd_desc.ind[1] = 1; 	bd_desc.ind[2] = 2;
	}
	else
		GMT_report (GMT, GMT_MSG_FATAL, "It shouldn't pass here\n");

	/* Allocate a structure that will be used inside okabe().
	   We do it here to avoid thousands of alloc/free that would result if done in okabe() */ 
	n_vert_max = bd_desc.n_v[0];
	for (i = 1; i < bd_desc.n_f; i++) {
		n_vert_max = MAX(bd_desc.n_v[i], n_vert_max);
	}
	loc_or = GMT_memory (GMT, CNULL, (size_t) (n_vert_max+1), struct LOC_OR);

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
			mag_var = GMT_memory (GMT, NULL, (size_t) n_triang, struct MAG_VAR);
			if (Ctrl->T.m_var1) {		/* Only the mag intensity changes, Dec & Dip are the same as the Field */
				for (i = 0; i < n_triang; i++) {
					t_mag = (Ctrl->N.mag_int[vert[i].a] + Ctrl->N.mag_int[vert[i].b] + Ctrl->N.mag_int[vert[i].c])/3.;
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
				mag_param = GMT_memory (GMT, NULL, (size_t) n_triang, struct MAG_PARAM);
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
		cos_vec = GMT_memory (GMT, NULL, (size_t) Gout->header->ny, double);
		for (i = 0; i < Gout->header->ny; i++)
			cos_vec[i] = (Ctrl->M.active) ? cos(y[i]*D2R): 1;
	}

	for (i = j = 0; i < n_triang; i++) {
		if (GMT_is_verbose (GMT, GMT_MSG_NORMAL) && i > j*one_100) {
			GMT_message (GMT, "computed %.2ld%s of %ld prisms\r", j, "%", n_triang);
			j++;
		}
		km = (Ctrl->T.m_var) ? i : 0;
		pm = (Ctrl->T.m_var4) ? i : 0;

		/* Don't wast time with zero mag triangles */
		if (Ctrl->H.active && Ctrl->T.m_var && mag_var[i].rk[0] == 0 && mag_var[i].rk[1] == 0 && mag_var[i].rk[2] == 0)
			continue;
		if (Ctrl->T.triangulate)
			z_th = facet_triangulate (Ctrl, i, bat);
		else if (Ctrl->T.raw || Ctrl->T.stl)
			z_th = facet_raw (Ctrl, i, Ctrl->M.active);
		if (z_th) {
			if (Ctrl->G.active) { /* grid */
				GMT_row_loop (GMT, Gout,row) {
					y_obs[row] = (Ctrl->M.active) ? ((y[row]+central_lat) * Ctrl->N.d_to_m): y[row];
					GMT_col_loop (GMT, Gout,row,col,ij) {
						x_obs[col] = (Ctrl->M.active) ? ((x[col]-central_long)*Ctrl->N.d_to_m*cos_vec[row]): x[col]; 
						if (Ctrl->S.active)
							DO = TRUE;
						else {
							DX = t_center[i].x - x_obs[col];
							DY = t_center[i].y - y_obs[col];
							DZ = t_center[i].z - z_obs[col];
							DO = (DX*DX + DY*DY + DZ*DZ < s_rad2); 
						}
						if (DO) {
							a = okabe (GMT, Ctrl, x_obs[col], y_obs[row], bd_desc, km, pm, loc_or);
							if (!GMT_is_dnan(a))
								Gout->data[ij] += (float)a;
						}
					}
				}
			}
			else {		/* polygon */
				for (kk = 0; kk < ndata_p; kk++){
					if (Ctrl->S.active)
						DO = TRUE;
					else {
						DX = t_center[i].x - x_obs[kk];
						DY = t_center[i].y - y_obs[kk];
						DZ = t_center[i].z - z_obs[kk];
						DO = (DX*DX + DY*DY + DZ*DZ < s_rad2); 
					}
					if (DO) {
						a = okabe (GMT, Ctrl, x_obs[kk], y_obs[kk], bd_desc, km, pm, loc_or);
						if (!GMT_is_dnan(a))
							g[kk] += (float)a;
					}
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

		if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) /* Enables data output and sets access mode */
			Return (error);

		GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, Ctrl->G.file, Gout);
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);			/* Disables further data output */

	}
	else {
		for (k = 0; k < ndata_p; k++)
			fprintf (stdout, "%.9g %.9g %.9g\n", data[k].x, data[k].y, g[k]);
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
	GMT_free (GMT, bd_desc.n_v);
	GMT_free (GMT, bd_desc.ind);
	if (cos_vec) GMT_free (GMT, cos_vec);
	if (loc_or) GMT_free (GMT, loc_or);
	if (Ctrl->T.m_var1) GMT_free (GMT, Ctrl->N.mag_int);
	if (Ctrl->T.m_var2) GMT_free (GMT, mag_var2);
	if (Ctrl->T.m_var3) GMT_free (GMT, mag_var3);
	if (Ctrl->T.m_var4) GMT_free (GMT, mag_var4);

	Return (GMT_OK);
}

/* -------------------------------------------------------------------------*/
GMT_LONG read_xyz (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *Ctrl, char *fname, double *central_long, double *central_lat) {
	/* read xyz[m] file with point data coordinates */

	GMT_LONG n_alloc, ndata_xyz;
	float x_min = FLT_MAX, x_max = -FLT_MAX, y_min = FLT_MAX, y_max = -FLT_MAX;
	double in[8];
	char line[GMT_TEXT_LEN256];
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

       	n_alloc = GMT_CHUNK;
	ndata_xyz = 0;
	*central_long = 0.;	*central_lat = 0.;
        triang = GMT_memory (GMT, NULL, (size_t)n_alloc, struct TRIANG);
	if (Ctrl->T.m_var1)
        	Ctrl->N.mag_int = GMT_memory (GMT, NULL, (size_t)n_alloc, double);
	else if (Ctrl->T.m_var2)
        	mag_var2 = GMT_memory (GMT, NULL, (size_t)n_alloc, struct MAG_VAR2);
	else if (Ctrl->T.m_var3)
        	mag_var3 = GMT_memory (GMT, NULL, (size_t)n_alloc, struct MAG_VAR3);
	else if (Ctrl->T.m_var4)
        	mag_var4 = GMT_memory (GMT, NULL, (size_t)n_alloc, struct MAG_VAR4);
	
	if (Ctrl->M.active) {	/* take a first read just to compute the central logitude */
		while (fgets (line, GMT_TEXT_LEN256, fp)) { 
			sscanf (line, "%lg %lg", &in[0], &in[1]); /* A test on file integrity will be done bellow */
			x_min = (float)MIN(in[0], x_min);	x_max = (float)MAX(in[0], x_max);
			y_min = (float)MIN(in[1], y_min);	y_max = (float)MAX(in[1], y_max);
		}
		*central_long = (x_min + x_max) / 2;
		*central_lat  = (y_min + y_max) / 2;
		rewind(fp);
	}

	while (fgets (line, GMT_TEXT_LEN256, fp)) { 
		if (!Ctrl->T.m_var) {
			if(sscanf (line, "%lg %lg %lg", &in[0], &in[1], &in[2]) !=3)
				GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering line %ld of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else if (Ctrl->T.m_var1) { /* data file has 4 columns and the last contains magnetization */
			if(sscanf (line, "%lg %lg %lg %lg", &in[0], &in[1], &in[2], &in[3]) !=4)
				GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering line %ld of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else if (Ctrl->T.m_var2) { /* data file has 5 columns: x,y,z,mag,dip */
			if(sscanf (line, "%lg %lg %lg %lg %lg", &in[0], &in[1], &in[2], &in[3], &in[4]) !=5)
				GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering line %ld of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else if (Ctrl->T.m_var3) { /* data file has 6 columns: x,y,z,mag_int,mag_dec,mag_dip */
			if(sscanf (line, "%lg %lg %lg %lg %lg %lg", &in[0], &in[1], &in[2], &in[3], &in[4], &in[5]) !=6)
				GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering line %ld of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		else {		/* data file has 8 columns: x,y,z,field_dec,field_dip,mag_int,mag_dec,mag_dip */ 
			if(sscanf (line, "%lg %lg %lg %lg %lg %lg %lg %lg", 
				   &in[0], &in[1], &in[2], &in[3], &in[4], &in[5], &in[6], &in[7]) !=8)
				GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering line %ld of %s\n", ndata_xyz+1, Ctrl->T.xyz_file);
		}
		if (ndata_xyz == n_alloc) {
			n_alloc <<= 1;
			triang = GMT_memory (GMT, triang, (size_t)n_alloc, struct TRIANG);
			if (Ctrl->T.m_var1)
				Ctrl->N.mag_int = GMT_memory (GMT, Ctrl->N.mag_int, (size_t)n_alloc, double);
			else if (Ctrl->T.m_var2)
				mag_var2 = GMT_memory (GMT, mag_var2, (size_t)n_alloc, struct MAG_VAR2);
			else if (Ctrl->T.m_var3)
				mag_var3 = GMT_memory (GMT, mag_var3, (size_t)n_alloc, struct MAG_VAR3);
			else
				mag_var4 = GMT_memory (GMT, mag_var4, (size_t)n_alloc, struct MAG_VAR4);
		}
		triang[ndata_xyz].x = (Ctrl->M.active) ? (in[0] - *central_long) * Ctrl->N.d_to_m * cos(in[1]*D2R) : in[0];
		triang[ndata_xyz].y = (Ctrl->M.active) ? -(in[1] - *central_lat) * Ctrl->N.d_to_m : -in[1]; /* - because y must be positive 'south'*/
		triang[ndata_xyz].z = in[2] * Ctrl->D.dir;
		if (Ctrl->T.m_var1) 
			Ctrl->N.mag_int[ndata_xyz] = in[3];
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
GMT_LONG read_t (struct GMT_CTRL *GMT, char *fname) {
	/* read file with vertex indexes of triangles */
	GMT_LONG n_alloc, ndata_t;
	int in[3];
	char line[GMT_TEXT_LEN256];
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata_t = 0;
	vert = GMT_memory (GMT, NULL, (size_t) n_alloc, struct VERT);
	
	while (fgets (line, GMT_TEXT_LEN256, fp)) { 
		if(sscanf (line, "%d %d %d", &in[0], &in[1], &in[2]) !=3)
			GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering line %ld of %s\n", ndata_t+1, fname);
		if (ndata_t == n_alloc) {
			n_alloc <<= 1;
			vert = GMT_memory (GMT, vert, (size_t)n_alloc, struct VERT);
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
GMT_LONG read_raw (struct GMT_CTRL *GMT, char *fname, double z_dir) {
	/* read a file with triagles in the raw format and returns nb of triangles */
	GMT_LONG n_alloc, ndata_r;
	double in[9];
	char line[GMT_TEXT_LEN256];
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata_r = 0;
	raw_mesh = GMT_memory (GMT, NULL, (size_t) n_alloc, struct RAW);
	
	while (fgets (line, GMT_TEXT_LEN256, fp)) { 
		if(sscanf (line, "%lg %lg %lg %lg %lg %lg %lg %lg %lg", 
			   &in[0], &in[1], &in[2], &in[3], &in[4], &in[5], &in[6], &in[7], &in[8]) !=9)
			GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering line %ld of %s\n", ndata_r+1, fname);
              	if (ndata_r == n_alloc) {
			n_alloc <<= 1;
			raw_mesh = GMT_memory (GMT, raw_mesh, (size_t)n_alloc, struct RAW);
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
GMT_LONG read_stl (struct GMT_CTRL *GMT, char *fname, double z_dir) {
	/* read a file with triagles in the stl format and returns nb of triangles */
	GMT_LONG n_alloc, ndata_s;
	double in[3];
	char line[GMT_TEXT_LEN256], text[128], ver_txt[128];
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata_s = 0;
	raw_mesh = GMT_memory (GMT, NULL, (size_t) n_alloc, struct RAW);
	
	while (fgets (line, GMT_TEXT_LEN256, fp)) { 
		sscanf (line, "%s", text);
		if (strcmp (text, "outer") == 0) {
			fgets (line, GMT_TEXT_LEN256, fp); /* get first vertex */
			if(sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering triangle %ld of %s\n", ndata_s+1, fname);
			raw_mesh[ndata_s].t1[0] = in[0];
			raw_mesh[ndata_s].t1[1] = -in[1];
			raw_mesh[ndata_s].t1[2] = in[2] * z_dir;
			fgets (line, GMT_TEXT_LEN256, fp); /* get second vertex */
			if(sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering triangle %ld of %s\n", ndata_s+1, fname);
			raw_mesh[ndata_s].t2[0] = in[0];
			raw_mesh[ndata_s].t2[1] = -in[1];
			raw_mesh[ndata_s].t2[2] = in[2] * z_dir;
			fgets (line, GMT_TEXT_LEN256, fp); /* get third vertex */
			if(sscanf (line, "%s %lg %lg %lg", ver_txt, &in[0], &in[1], &in[2]) !=4)
				GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering triangle %ld of %s\n", ndata_s+1, fname);
			raw_mesh[ndata_s].t3[0] = in[0];
			raw_mesh[ndata_s].t3[1] = -in[1];
			raw_mesh[ndata_s].t3[2] = in[2] * z_dir;
			ndata_s++;
              		if (ndata_s == n_alloc) { /* with bad luck we have a flaw here */
				n_alloc <<= 1;
				raw_mesh = GMT_memory (GMT, raw_mesh, (size_t)n_alloc, struct RAW);
			}
		}
		else
			continue;
	}
	fclose(fp);
	return (ndata_s);
}

/* -----------------------------------------------------------------*/
GMT_LONG read_poly (struct GMT_CTRL *GMT, char *fname, GMT_LONG switch_xy) {
	/* Read file with xy points where anomaly is going to be computed*/
	GMT_LONG n_alloc, ndata, ix = 0, iy = 1;
	double in[2];
	char line[GMT_TEXT_LEN256];
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

       	n_alloc = GMT_CHUNK;
	ndata = 0;
	if (switch_xy) {iy = 0; ix = 1;}

	data = GMT_memory (GMT, NULL, (size_t) n_alloc, struct DATA);

	while (fgets (line, GMT_TEXT_LEN256, fp)) { 
		if(sscanf (line, "%lg %lg", &in[0], &in[1]) !=2)
			GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering line %ld of polygon file\n", ndata+1);
               	if (ndata == n_alloc) {
			n_alloc <<= 1;
			data = GMT_memory (GMT, data, (size_t)n_alloc, struct DATA);
               	}
		data[ndata].x = in[ix];
		data[ndata].y = in[iy];
		ndata++;
	}
	fclose(fp);
	return (ndata);
}

/* -----------------------------------------------------------------*/
GMT_LONG facet_triangulate (struct XYZOKB_CTRL *Ctrl, GMT_LONG i, GMT_LONG bat) {
	/* Sets coodinates for the facet whose effect is beeing calculated */
	double x_a, x_b, x_c, y_a, y_b, y_c, z_a, z_b, z_c;

	x_a = triang[vert[i].a].x;	x_b = triang[vert[i].b].x;	x_c = triang[vert[i].c].x;
	y_a = triang[vert[i].a].y;	y_b = triang[vert[i].b].y;	y_c = triang[vert[i].c].y;
	z_a = triang[vert[i].a].z;	z_b = triang[vert[i].b].z;	z_c = triang[vert[i].c].z;
	/* top triang */
	Ctrl->N.xx[0] = x_a;	Ctrl->N.yy[0] = y_a;
	Ctrl->N.xx[1] = x_b;	Ctrl->N.yy[1] = y_b;
	Ctrl->N.xx[2] = x_c;	Ctrl->N.yy[2] = y_c;
	/* bot triang */
	Ctrl->N.xx[3] = Ctrl->N.xx[0];	Ctrl->N.yy[3] = Ctrl->N.yy[0];
	Ctrl->N.xx[4] = Ctrl->N.xx[1];	Ctrl->N.yy[4] = Ctrl->N.yy[1];
	Ctrl->N.xx[5] = Ctrl->N.xx[2];	Ctrl->N.yy[5] = Ctrl->N.yy[2];

	Ctrl->N.xx[6] = Ctrl->N.xx[1];	Ctrl->N.yy[6] = Ctrl->N.yy[1];
	Ctrl->N.xx[7] = Ctrl->N.xx[4];	Ctrl->N.yy[7] = Ctrl->N.yy[4];
	Ctrl->N.xx[8] = Ctrl->N.xx[5];	Ctrl->N.yy[8] = Ctrl->N.yy[5];
	Ctrl->N.xx[9] = Ctrl->N.xx[2];	Ctrl->N.yy[9] = Ctrl->N.yy[2];

	Ctrl->N.xx[10] = Ctrl->N.xx[1];	Ctrl->N.yy[10] = Ctrl->N.yy[1];
	Ctrl->N.xx[11] = Ctrl->N.xx[0];	Ctrl->N.yy[11] = Ctrl->N.yy[0];
	Ctrl->N.xx[12] = Ctrl->N.xx[3];	Ctrl->N.yy[12] = Ctrl->N.yy[3];
	Ctrl->N.xx[13] = Ctrl->N.xx[4];	Ctrl->N.yy[13] = Ctrl->N.yy[4];

	Ctrl->N.xx[14] = Ctrl->N.xx[0];	Ctrl->N.yy[14] = Ctrl->N.yy[0];
	Ctrl->N.xx[15] = Ctrl->N.xx[2];	Ctrl->N.yy[15] = Ctrl->N.yy[2];
	Ctrl->N.xx[16] = Ctrl->N.xx[5];	Ctrl->N.yy[16] = Ctrl->N.yy[5];
	Ctrl->N.xx[17] = Ctrl->N.xx[3];	Ctrl->N.yy[17] = Ctrl->N.yy[3];

	if (Ctrl->E.active) { /* Layer of constant thickness */
		Ctrl->N.zz[0] = z_a;			Ctrl->N.zz[1] = z_b;			Ctrl->N.zz[2] = z_c;
		Ctrl->N.zz[3] = z_a + Ctrl->E.dz;	Ctrl->N.zz[4] = z_b + Ctrl->E.dz;	Ctrl->N.zz[5] = z_c + Ctrl->E.dz;
		Ctrl->N.zz[6] = Ctrl->N.zz[1];		Ctrl->N.zz[7] = Ctrl->N.zz[4];		Ctrl->N.zz[8] = Ctrl->N.zz[5];
		Ctrl->N.zz[9] = Ctrl->N.zz[5];		Ctrl->N.zz[10] = Ctrl->N.zz[1];		Ctrl->N.zz[11] = Ctrl->N.zz[0];
		Ctrl->N.zz[12] = Ctrl->N.zz[3];		Ctrl->N.zz[13] = Ctrl->N.zz[4];		Ctrl->N.zz[14] = Ctrl->N.zz[0];
		Ctrl->N.zz[15] = Ctrl->N.zz[2];		Ctrl->N.zz[16] = Ctrl->N.zz[5];		Ctrl->N.zz[17] = Ctrl->N.zz[3];

		return (1);
	}
	if (bat) { /* Triangle mesh defines a bathymetric surface (TA MIXORDADO (== NOS DOIS CASOS)) */
		Ctrl->N.zz[0] = z_a;		Ctrl->N.zz[1] = z_b;		Ctrl->N.zz[2] = z_c;
		Ctrl->N.zz[3] = Ctrl->Z.z0;	Ctrl->N.zz[4] = Ctrl->Z.z0;	Ctrl->N.zz[5] = Ctrl->Z.z0;
		if (fabs(Ctrl->N.zz[0] - Ctrl->N.zz[3]) > Ctrl->E.dz || fabs(Ctrl->N.zz[1] - Ctrl->N.zz[4]) > 
		    Ctrl->E.dz || fabs(Ctrl->N.zz[2] - Ctrl->N.zz[5]) > Ctrl->E.dz) 
			return 1;
		else 
			return (0);
	}
	else { /* Triangle mesh defines a topographic surface */
		Ctrl->N.zz[0] = z_a;		Ctrl->N.zz[1] = z_b;		Ctrl->N.zz[2] = z_c;
		Ctrl->N.zz[3] = Ctrl->Z.z0;	Ctrl->N.zz[4] = Ctrl->Z.z0;	Ctrl->N.zz[5] = Ctrl->Z.z0;
		if (fabs(Ctrl->N.zz[0] - Ctrl->N.zz[3]) > Ctrl->E.dz || fabs(Ctrl->N.zz[1] - Ctrl->N.zz[4]) > 
		    Ctrl->E.dz || fabs(Ctrl->N.zz[2] - Ctrl->N.zz[5]) > Ctrl->E.dz) 
			return 1;
		else 
			return (0);
	}
}

/* -----------------------------------------------------------------*/
GMT_LONG facet_raw (struct XYZOKB_CTRL *Ctrl, GMT_LONG i, GMT_LONG geo) {
	/* Sets coodinates for the facet in the RAW format */
	double cos_a, cos_b, cos_c, x_a, x_b, x_c, y_a, y_b, y_c, z_a, z_b, z_c;

	x_a = raw_mesh[i].t1[0];   x_b = raw_mesh[i].t2[0];   x_c = raw_mesh[i].t3[0];
	y_a = raw_mesh[i].t1[1];   y_b = raw_mesh[i].t2[1];   y_c = raw_mesh[i].t3[1];
	z_a = raw_mesh[i].t1[2];   z_b = raw_mesh[i].t2[2];   z_c = raw_mesh[i].t3[2];
	if (geo) {
		cos_a = cos(y_a*D2R);	cos_b = cos(y_b*D2R);	cos_c = cos(y_c*D2R);
	}
	else {cos_a = cos_b = cos_c = 1.;}
	Ctrl->N.xx[0] = x_a*Ctrl->N.d_to_m*cos_a;	Ctrl->N.yy[0] = y_a*Ctrl->N.d_to_m;
	Ctrl->N.xx[1] = x_b*Ctrl->N.d_to_m*cos_b;	Ctrl->N.yy[1] = y_b*Ctrl->N.d_to_m;
	Ctrl->N.xx[2] = x_c*Ctrl->N.d_to_m*cos_c;	Ctrl->N.yy[2] = y_c*Ctrl->N.d_to_m;
	Ctrl->N.zz[0] = z_a;	Ctrl->N.zz[1] = z_b;	Ctrl->N.zz[2] = z_c;
	return 1; /* Allways return 1 */
}

/* --------------------------------------------------------------------- */
double okabe (struct GMT_CTRL *GMT, struct XYZOKB_CTRL *Ctrl, double x_o, double y_o, 
	      struct BODY_DESC bd_desc, GMT_LONG km, GMT_LONG pm, struct LOC_OR *loc_or) {
	double okb = 0.0, tot = 0.;
	GMT_LONG i, n_vert, l, k, cnt_v = 0;
	GMT_LONG top = TRUE;

/* x_o, y_o, z_o are the coordinates of the observation point
 * rho is the body density times G constant
 * km is an: index of current body facet (if they have different mags); or 0 if mag=const
 * bd_desc is a structure containing the body's description. It contains the following members
 * n_f -> number of facets (int)
 * n_v -> number of vertex of each facet (pointer)
 * ind -> index describing the vertex order of each facet. These index must
 *	  describe the facet in a clock-wise order when viewed from outside. */
	
/*  _________________________________________________________________ 
    |                                                               | 
    |  Reference : Okabe, M., Analytical expressions for gravity    |
    |     anomalies due to polyhedral bodies and translation into   |
    |     magnetic anomalies, Geophysics, 44, (1979), p 730-741.    |
    |_______________________________________________________________|
    _____________________________________________________________________
    |                                                                   |
    |  Ifac decrit le corps (ATTN : Integer*2) :                        |
    |   - Il y a Nff facettes qui sont decrites dans le sens des        |
    |          aiguilles d'une montre si on regarde le corps de         |
    |          l'exterieur. Mxsomf = Max de sommets / face              |
    |   - Le premier nombre indique le nombre de factettes. Suivent     |
    |       alors des groupes de nombres dont le premier de chaque      |
    |       groupe est le nombre de sommets de la facette, suivi par    |
    |       les indices (pointeurs) des sommets (rang dans Xx,Yy,Zz)    |
    |       correspondant a la facette.                                 |
    |                                                                   |
    |  Par exemple pour un cube                _________________        |
    |  (Nff=6; 4 sommets /face)              /|         X (Nord)        |
    |  [Ifac] = { 6,  4, 1,2,3,4,           / |                         |
    |                 4, 2,6,7,3,          /  |     1 ________ 2        |
    |                 4, 4,3,7,8,         /   |      /       /|         |
    |                 4, 5,1,4,8,      Y /    |     /       / |         |
    |                 4, 1,5,6,2,      (Est)  |  4 /_______/3 |         |
    |                 4, 5,8,7,6 }            |    |       |  |         |
    |                                         |    |       | / 6        |
    |                                       Z |    |       |/           |
    |                                         V    |_______/            |
    |                                             8         7           |
    |___________________________________________________________________|
    |                                                                   |
    |  X,Y ET Z sont les tableaux des coordonness des pts de mesure     |
    |___________________________________________________________________| */

	for (i = 0; i < bd_desc.n_f; i++) {	/* Loop over facets */
		n_vert = bd_desc.n_v[i];	/* Number of vertices of each face */
		if (n_vert < 3) 
			GMT_report (GMT, GMT_MSG_NORMAL, "Warning: facet with less than 3 vertex\n");
		for (l = 0; l < n_vert; l++) {
			k = bd_desc.ind[l+cnt_v];
			loc_or[l].x = Ctrl->N.xx[k] - x_o;
			loc_or[l].y = Ctrl->N.yy[k] - y_o;
			loc_or[l].z = Ctrl->N.zz[k] - Ctrl->L.zobs;
		}
		rot_17 (Ctrl, n_vert, top, loc_or); /* rotate coords by eq (17) of okb */
		okb += (Ctrl->C.active) ? okb_grv (Ctrl, n_vert, loc_or): okb_mag (Ctrl, n_vert, km, pm, loc_or);
		cnt_v += n_vert;
	}
	tot = (Ctrl->C.active) ? okb * Ctrl->C.rho: okb;
	return (tot);
}

/* ---------------------------------------------------------------------- */
void rot_17 ( struct XYZOKB_CTRL *Ctrl, GMT_LONG n_vert, GMT_LONG top, struct LOC_OR *loc_or) {
	/* Rotates coordinates by teta and phi acording to equation (17) of Okabe */
	/* store the result in external structure loc_or and angles c_tet s_tet c_phi s_phi */
	double xi, xj, xk, yi, yj, yk, zi, zj, zk, v, x, y, z;
	double r, r2, r_3d, Sxy, Szx, Syz;
	GMT_LONG i = 0, j, k, l;

	loc_or[n_vert].x = loc_or[0].x;		loc_or[n_vert].y = loc_or[0].y;	
	loc_or[n_vert].z = loc_or[0].z;		/* Last point = first point */

	if (top) { /* Currently, this is always true */
		j = i + 1;	k = i + 2;
		xi = loc_or[i].x;	xj = loc_or[j].x;	xk = loc_or[k].x;
		yi = loc_or[i].y;	yj = loc_or[j].y;	yk = loc_or[k].y;
		zi = loc_or[i].z;	zj = loc_or[j].z;	zk = loc_or[k].z;
		Sxy = xi * (yj - yk) + xj * (yk - yi) + xk * (yi - yj);
		Syz = yi * (zj - zk) + yj * (zk - zi) + yk * (zi - zj);
		Szx = zi * (xj - xk) + zj * (xk - xi) + zk * (xi - xj);
		r2 = Syz * Syz + Szx * Szx;	r = sqrt(r2);
		r_3d = sqrt(r2 + Sxy * Sxy);
		Ctrl->N.c_phi = - Sxy / r_3d;
		Ctrl->N.s_phi = r / r_3d;

		if (Szx == 0.0 && Syz == 0.0) { Ctrl->N.c_tet = 1.0;	Ctrl->N.s_tet = 0.0;}
		else { Ctrl->N.c_tet = - Syz / r;	Ctrl->N.s_tet = - Szx / r;}
		}
	else { /* Don't need to recompute angles, only do this */
		Ctrl->N.c_tet *= -1;	Ctrl->N.s_tet *= -1;	Ctrl->N.c_phi *= -1;
	}

	for (l = 0; l < n_vert + 1; l++) {
		x = loc_or[l].x;	y = loc_or[l].y;	z = loc_or[l].z;
		v = x * Ctrl->N.c_tet + y * Ctrl->N.s_tet;
		loc_or[l].x = v * Ctrl->N.c_phi - z * Ctrl->N.s_phi;
		loc_or[l].y = y * Ctrl->N.c_tet - x * Ctrl->N.s_tet;
		loc_or[l].z = v * Ctrl->N.s_phi + z * Ctrl->N.c_phi;
	}
}
/* ---------------------------------------------------------------------- */
double okb_grv ( struct XYZOKB_CTRL *Ctrl, GMT_LONG n_vert, struct LOC_OR *loc_or) {
/*  Computes the gravity anomaly due to a facet. */
 
	double x1, x2, y1, y2, z2, z1, dx, dy, r, r_1, c_psi, s_psi;
	double grv = 0.0, grv_p;
	GMT_LONG l;

	if (fabs(Ctrl->N.c_phi) < FLT_EPSILON) return 0.0;
	for (l = 0; l < n_vert; l++) {
		x1 = loc_or[l].x;	x2 = loc_or[l+1].x;
		y1 = loc_or[l].y;	y2 = loc_or[l+1].y;
		dx = x2 - x1;	dy = y2 - y1;
		r = sqrt(dx*dx + dy*dy);
		r_1 = 1. / r;
		if (r > FLT_EPSILON) {
			c_psi = dx * r_1;	s_psi = dy * r_1;
			z2 = loc_or[l+1].z;	z1 = loc_or[l].z;
			grv_p = eq_30(c_psi, s_psi, x2, y2, z2) - eq_30(c_psi, s_psi, x1, y1, z1);
		}
		else
			grv_p = 0.0;
		grv += grv_p;
	}
	return (grv * Ctrl->N.c_phi);
}
/* ---------------------------------------------------------------------- */
double eq_30 (double c, double s, double x, double y, double z) {
	double r, Ji = 0.0, log_arg;

	r = sqrt(x * x + y * y + z * z);
	if (r > FLT_EPSILON) {
		if (fabs(z) > FLT_EPSILON && fabs(c) > FLT_EPSILON)
			Ji = -2. * z * atan ((x * c + (s + 1) * (y + r)) / (z * c));
		log_arg = x * c + y * s + r;
		if (log_arg > FLT_EPSILON)
			Ji += (x * s - y * c) * log(log_arg);
	}
	return Ji;
}

/* ---------------------------------------------------------------------- */
double okb_mag ( struct XYZOKB_CTRL *Ctrl, GMT_LONG n_vert, GMT_LONG km, GMT_LONG pm, struct LOC_OR *loc_or) {
/*  Computes the total magnetic anomaly due to a facet. */
 
	double qsi1, qsi2, eta1, eta2, z2, z1, dx, dy, kx, ky, kz, v, r, c_psi, s_psi;
	double ano = 0.0, ano_p, mag_fac, xi, xi1, yi, yi1, mx, my, mz, r_1, tg_psi, auxil;
	GMT_LONG i;

	mag_fac = Ctrl->N.s_phi * (mag_param[pm].rim[0] * Ctrl->N.c_tet + mag_param[pm].rim[1] * Ctrl->N.s_tet) + 
				   mag_param[pm].rim[2] * Ctrl->N.c_phi;

	if (fabs(mag_fac) < FLT_EPSILON) return 0.0;

	kx = mag_var[km].rk[0];	ky = mag_var[km].rk[1];	kz = mag_var[km].rk[2];
	v = kx * Ctrl->N.c_tet + ky * Ctrl->N.s_tet;
	mx = v * Ctrl->N.c_phi - kz * Ctrl->N.s_phi;	my = ky * Ctrl->N.c_tet - kx * Ctrl->N.s_tet;
	mz = v * Ctrl->N.s_phi + kz * Ctrl->N.c_phi;

	for (i = 0; i < n_vert; i++) {
		xi = loc_or[i].x;	xi1 = loc_or[i+1].x;
		yi = loc_or[i].y;	yi1 = loc_or[i+1].y;
		dx = xi1 - xi;	dy = yi1 - yi;
		r = sqrt(dx*dx + dy*dy);
		r_1 = 1. / r;
		if (r > FLT_EPSILON) {
			c_psi = dx * r_1;	s_psi = dy * r_1;
			tg_psi = dy / dx;
			auxil = my * c_psi - mx * s_psi;
			qsi1 = yi * s_psi + xi * c_psi;	qsi2 = yi1 * s_psi + xi1 * c_psi;
			eta1 = yi * c_psi - xi * s_psi;	eta2 = yi1 * c_psi - xi1 * s_psi;
			z1 = loc_or[i].z;	z2 = loc_or[i+1].z;
			ano_p = eq_43(mz, c_psi, tg_psi, auxil, qsi2, eta2, z2) - 
			        eq_43(mz, c_psi, tg_psi, auxil, qsi1, eta1, z1);
		}
		else
			ano_p = 0.0;
		ano += ano_p;
	}
	return (ano * mag_fac);
}

/* ---------------------------------------------------------------------- */
double eq_43 (double mz, double c, double tg, double auxil, double x, double y, double z) {
	double r, ez, Li = 0.0, tmp;

	ez = y * y + z * z;
	r = sqrt(x * x + ez);

	if (r > FLT_EPSILON) {
		if (fabs(z) > FLT_EPSILON && fabs(c) > FLT_EPSILON)
			Li = mz * atan((ez * tg - x * y) / (z * r));
		else
			Li = 0.0;
		tmp = x + r;
		if (tmp <= 0.)
			Li -= log(r - x) * auxil;
		else
			Li += log(tmp) * auxil;
	}
	return Li;
}
/* ---------------------------------------------------------------------- */
void set_center (GMT_LONG n_triang) {
	/* Calculates triangle center by an aproximate (iterative) formula */
	GMT_LONG i, j, k = 5;
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
void triang_norm (GMT_LONG n_triang) {
	/* Computes the unit normal to trianglular facet */
	GMT_LONG i;
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
GMT_LONG check_triang_cw (GMT_LONG n, GMT_LONG type) {
	/* Checks that triangles are given in the correct clock-wise order.
	If not swap them. This is a tricky issue. In the case of "classic" 
	trihedron (x positive right; y positive "north" and z positive up),
	positive determinants signify counter clockwise order. However, in
	geomagnetic reference (x positive right; y positive "south" and z 
	positive down (OK, I know it's not exactly like this but instead 
	x->north; y->east; z->down)), counter clockwise order follows if
	determinant is negative. */

	GMT_LONG i, n_swaped = 0, tmp;
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
