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

/*--------------------------------------------------------------------
 * Compute Peak Ground Acceleration/Velocity and Intensity
 * Author:	J. Luis, after the FORTRAN version from J M Miranda
 * Date: 	Original from 06-Apr-2010 for Mirone usage. Adapted to GMT in 2020
 *
 */ 

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdshake"
#define THIS_MODULE_MODERN_NAME	"grdshake"
#define THIS_MODULE_LIB		"seis"
#define THIS_MODULE_PURPOSE	"Compute Peak Ground Acceleration/Velocity and Intensity."
#define THIS_MODULE_KEYS	"<G{,LD(=,GG}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:RVhif"

/* Control structure */

struct SHAKE_CTRL {
	struct SHAKE_In {
		bool active;
		char *file;
	} In;
	struct SHAKE_C {	/* -Cx/y */
		bool active;
		bool no_PGV;
		bool selected[3];
		int n_selected;
	} C;
	struct SHAKE_D {	/* -Dx0/y0/x1/y1 */
		bool active;
		char *line;
	} D;
	struct SHAKE_F {	/* -F */
		bool active;
		int imeca;
	} F;
	struct SHAKE_G {	/* -G<file> */
		bool active;
		bool do_PGA, do_PGV, do_INT;
		int  n;			/* Number of output grids specified via -G */
		char *file[3];	/* Only first is used for commandline but API may need many */
	} G;
	struct SHAKE_L {	/* -L<line.xy>[/<d|e|f|k|m|M|n|s|c|C>] */
		bool active;
		unsigned int mode;	/* 0 = dist to nearest point, 1 = also get the point, 2 = instead get seg#, pt# */
		unsigned int sph;	/* 0 = Flat Earth, 1 = spherical [Default], 2 = ellipsoidal */
		char *file;	/* Name of file with lines */
		double line[4];
		char unit;
	} L;
	struct SHAKE_M {
		bool active;
		double rmw;
	} M;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SHAKE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct SHAKE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->F.imeca = 1;
	C->L.mode = 1;                  /* Default returns distance to line */
	C->L.unit = 'k';                /* Default unit is km */
	C->L.sph = GMT_GREATCIRCLE;     /* Default is great-circle distances */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct SHAKE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free(GMT, C);
}

static char set_unit_and_mode (char *arg, unsigned int *mode) {
	unsigned int k = 0;
	*mode = GMT_GREATCIRCLE;	/* Default is great circle distances */
	switch (arg[0]) {
		case '-': *mode = GMT_FLATEARTH;	k = 1; break;
		case '+': *mode = GMT_GEODESIC;		k = 1; break;
	}
	return (arg[k]);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s <grid> -G<outgrid> -L<fault.dat> | -Dx0y0/x1/y1 -M<mag> [-Ca,v,i] [-F<mecatype>] [%s] [%s] [%s]\n", name, GMT_Rgeoz_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Name of grid (or image) to extract a subset from");
	gmt_outgrid_syntax (API, 'G', "Set name of the output grid file");
	GMT_Usage (API, -2, "If more than one component is set via -C then <outgrid> must contain %%s to format component code.\n");
	GMT_Usage (API, 1, "\n-D<x0/y0/x1/y1>");
	GMT_Usage (API, -2, "End points of the fault trace.");
	GMT_Usage (API, 1, "\n-L<fault_file>");
	GMT_Usage (API, -2, "Alternatively provide a name of a file with the coordinates of the fault trace.");
	GMT_Usage (API, 1, "\n-M<mag>");
	GMT_Usage (API, -2, "Select the seism magnitude.");

	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-C[a|v|i]");
	if (API->external)
		GMT_Usage (API, -2, "List of comma-separated components to be written as grids. Choose from:");
	else
		GMT_Usage (API, -2, "List of comma-separated components to be written as grids (requires -G). Choose from:");
	GMT_Usage (API, 3, "a (acceleration)");
	GMT_Usage (API, 3, "v (velocity)");
	GMT_Usage (API, 3, "i (intensity). This is the default.");
	GMT_Usage (API, 1, "\n-F[1|2|3|4]");
	GMT_Usage (API, -2, "Select focal mechanism type (e.g. -F1 or -F2 ...).");
	GMT_Usage (API, 3, "- 1 unknown [Default].");
	GMT_Usage (API, 3, "- 2 strike-slip.");
	GMT_Usage (API, 3, "- 3 normal.");
	GMT_Usage (API, 3, "- 4 thrust.");
	GMT_Option (API, "R,V");
	GMT_Option (API, "f,i,:");

	return (GMT_MODULE_USAGE);
}


static int parse (struct GMT_CTRL *GMT, struct SHAKE_CTRL *Ctrl, struct GMT_Z_IO *io, struct GMT_OPTION *options) {
	/* This parses the options provided to grdshake and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, pos = 0;
	char txt_a[GMT_LEN256] = {""}, p[GMT_LEN16] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	gmt_M_memset (io, 1, struct GMT_Z_IO);

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)) != 0)
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Requires -G and selects which components should be written as grids */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				while ((gmt_strtok (opt->arg, ",", &pos, p)) && Ctrl->C.n_selected < 3) {
					switch (p[0]) {
						case 'a': 		Ctrl->C.selected[0] = Ctrl->G.do_PGA = true;	break;
						case 'v': 		Ctrl->C.selected[1] = Ctrl->G.do_PGV = true;	break;
						case 'i': 		Ctrl->C.selected[2] = Ctrl->G.do_INT = true;	break;
						default:
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognized field argument %s in -C.!\n", p);
							n_errors++;
							break;
					}
					if (!Ctrl->C.selected[1]) Ctrl->C.no_PGV = true;
					if (Ctrl->C.selected[2]) Ctrl->C.selected[1] = true;	/* Intensity needs velocity too */
					Ctrl->C.n_selected++;
				}
				if (Ctrl->C.n_selected == 0) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-C requires comma-separated component arguments.\n");
					n_errors++;
				}
				break;
			case 'D':	/* x0/y0[x1/y1] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.line = strdup (opt->arg);
				break;
			case 'G':	/* Output filename */
				if (!GMT->parent->external && Ctrl->G.n) {	/* Command line interface */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-G can only be set once!\n");
					n_errors++;
				}
				else if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file[Ctrl->G.n++] = strdup (opt->arg);
				else
					n_errors++;

				if (!GMT->parent->external) {		/* Copy the name into the 3 slots to simplify the grid writing algo */
					Ctrl->G.file[1] = strdup (opt->arg);
					Ctrl->G.file[2] = strdup (opt->arg);
				}
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.imeca = atoi (opt->arg);
				if (Ctrl->F.imeca < 1 || Ctrl->F.imeca > 4)
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-F<type> error. 'type' must be in [1 4]\n");
				break;
			case 'L':	/* -L<table>[+u[+|-]<unit>] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.file = gmt_get_filename(API, opt->arg, "u");
				if (!gmt_check_filearg (GMT, 'L', Ctrl->L.file, GMT_IN, GMT_IS_DATASET)) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-L error. Must provide either an existing file name or line coordinates.\n");
					n_errors++;
				}
				break;
			case 'M':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				Ctrl->M.rmw = atof(opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (!Ctrl->C.active) {
		Ctrl->C.selected[1] = Ctrl->C.selected[2] = Ctrl->G.do_INT = Ctrl->C.no_PGV = true;		/* The default */
		Ctrl->C.n_selected = 1;
	}

	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single grid file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->L.active && !Ctrl->D.active,
	                                   "-L or -D option: Must provide a fault file name or coordinates.\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->M.active, "-M option: Must specify magnitude.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

/* --------------------------------------------------------------------------------- */
EXTERN_MSC int GMT_grdshake (void *V_API, int mode, void *args) {
	uint64_t i, j, ij, k, kk, row, seg;
	int error = 0, way, proj_type = 0;		/* Geographic */
	char file[GMT_LEN512] = {""}, *code[3] = {"a", "v", "i"};

	double wesn[4];
	double u = 1, ss = 0, rns = 0, rs = 0, dist = 0.0, xnear = 0.0, ynear = 0.0, xtmp, ytmp;
	double rmh_pga, rmh_pgv, r_pga, r_pgv, rfd_pga, rfd_pgv, rfd_pga4nl, k1, k2, k3;
	double flin_pga, flin_pgv, pga4nl, bnl_pga, bnl_pgv, fnl_pga, fnl_pgv, fs_pga, fs_pgv;
	double rfm_pgv, rfm_pga, c_pga, d_pga, c_pgv, d_pgv, tmp, tmp1, tmp2, tmp3, rlon, rlat;

	struct GMT_DATATABLE *xyline = NULL;
	struct GMT_DATASET *Lin = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_GRID *Grid[3] = {NULL, NULL, NULL};
	struct GMT_RECORD *Out = NULL;
	struct GMT_Z_IO io;
	struct GMT_OPTION *opt = NULL;
	struct SHAKE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, &io, options)) != 0) Return (error);
	
	/*---------------------------- This is the grdshake main code ----------------------------*/

	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */

	gmt_set_pad (GMT, 0);	/* Change the default pad (ignored if input is a memory grid) */
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) 	/* Get header only */
		Return (API->error);

	if (gmt_M_is_subset (GMT, G->header, wesn))		/* If subset requested make sure wesn matches header spacing */
		gmt_M_err_fail (GMT, gmt_adjust_loose_wesn (GMT, wesn, G->header), "");

	/* Read data */
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, G) == NULL)
		Return (API->error);

	gmt_M_memcpy (wesn, G->header->wesn, 4, double);	/* Need the wesn further down */

	gmt_set_geographic (GMT, GMT_OUT);
	for (k = 0; k < 3; k++) {
		if (!Ctrl->C.selected[k]) continue;
		/* Create the empty grid and allocate space */
		if ((Grid[k] = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, wesn,
		                                G->header->inc, GMT->common.R.registration, G->header->pad[0], NULL)) == NULL)
			Return (API->error);
	}

	way = gmt_M_is_geographic (GMT, GMT_IN) ? Ctrl->L.sph: 0;
	proj_type = gmt_init_distaz (GMT, Ctrl->L.unit, way, GMT_MAP_DIST);

	if (Ctrl->D.active) {	/* Gave -Dx0/y0/x1/y1 */
		int n;
		uint64_t par[] = {1,1,2,2};
		double x0, y0, x1, y1;
		Lin = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_CONTAINER_AND_DATA , par, NULL, NULL, 0, 0, NULL);
		n = sscanf (Ctrl->D.line, "%lf/%lf/%lf/%lf", &x0, &y0, &x1, &y1);
		if (n != 4 && n != 2) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -D: must provide either one x/y or two points x0/y0/x1/y1\n");
			bailout(GMT_PARSE_ERROR);
		}
		Lin->table[0]->segment[0]->data[GMT_X][0] = x0;		Lin->table[0]->segment[0]->data[GMT_Y][0] = y0;
		if (n == 4) {
			Lin->table[0]->segment[0]->data[GMT_X][1] = x1;		Lin->table[0]->segment[0]->data[GMT_Y][1] = y1;
		}
		else {		/* Just repeat the x0/y0 point with a noisy shift */
			Lin->table[0]->segment[0]->data[GMT_X][1] = x0+1e-6;	Lin->table[0]->segment[0]->data[GMT_Y][1] = y0+1e-6;
		}
	}
	else {
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR)
			Return (API->error);

		if ((Lin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, Ctrl->L.file, NULL)) == NULL)
			Return (API->error);

		if (Lin->n_columns < 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)Lin->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_set_segmentheader (GMT, GMT_OUT, false);	/* Since processing of -L file might have turned it on [should be determined below] */
	}

	xyline = Lin->table[0];				/* It can only be one table since we read a single file */

	if (proj_type == GMT_GEO2CART) {	/* Must convert the line points first */
		for (seg = 0; seg < xyline->n_segments; seg++) {
			for (row = 0; row < xyline->segment[seg]->n_rows; row++) {
				gmt_geo_to_xy (GMT, xyline->segment[seg]->data[GMT_X][row], xyline->segment[seg]->data[GMT_Y][row], &xtmp, &ytmp);
				xyline->segment[seg]->data[GMT_X][row] = xtmp;
				xyline->segment[seg]->data[GMT_Y][row] = ytmp;
			}
		}
	}
	else if (gmt_M_is_geographic (GMT, GMT_IN) && proj_type == GMT_GEOGRAPHIC && !gmt_M_is_spherical (GMT)) {
		/* Will need spherical trig so convert to geocentric latitudes if on an ellipsoid */
		for (seg = 0; seg < xyline->n_segments; seg++) {
			for (row = 0; row < xyline->segment[seg]->n_rows; row++) {		/* Convert to geocentric */
				xyline->segment[seg]->data[GMT_Y][row] = 
					gmt_lat_swap (GMT, xyline->segment[seg]->data[GMT_Y][row], GMT_LATSWAP_G2O);
			}
		}
	}

	/* ------------------------------------------------------------- */
	if (Ctrl->F.imeca == 2) {
		u = rns = rs = 0;	ss = 1;
	}
	else if (Ctrl->F.imeca == 3) {
		u = ss = rs = 0;	rns = 1;
	}
	else if (Ctrl->F.imeca == 4) {
		u = ss = rns = 0;	rs = 1;
	}

	/* -------------------------------------------------------------
	  compute the term of magnitude scaling for the pga and pgv.
	  It doesn't depend on the distance but only on magnitude.
	------------------------------------------------------------- */
	rmh_pga = 6.75;
	rmh_pgv = 8.50;
	rfm_pga = -0.53804*u - 0.50350*ss - 0.75472*rns - 0.50970*rs;
	if (Ctrl->M.rmw <= rmh_pga)
		rfm_pga += (0.28805*(Ctrl->M.rmw-rmh_pga) - 0.10164*(Ctrl->M.rmw-rmh_pga)*(Ctrl->M.rmw-rmh_pga));

	rfm_pgv = 5.00121*u + 5.04727*ss + 4.63188*rns + 5.08210*rs;
	if (Ctrl->M.rmw <= rmh_pgv)
		rfm_pgv += (0.18322*(Ctrl->M.rmw-rmh_pgv) - 0.12736*(Ctrl->M.rmw-rmh_pgv)*(Ctrl->M.rmw-rmh_pgv));

	/* - ------------------------------------------------------------
	! iterate the positions matrix pga (i,j), pgv (i,j) e pint (i,j)
	! to calculate the scaling distance term
	! ------------------------------------------------------------- */
	/* The k2 & k3 coeff bellow is to simplify the computations of origianl equations such:
	   c_pga = +bnl_pga * (3.*0.40546510 - 1.0986123)/(1.0986123 * 1.0986123); */
	k2 = (3.*0.40546510 - 1.0986123) / (1.0986123 * 1.0986123);
	k3 = (2.*0.40546510 - 1.0986123) / (1.0986123 * 1.0986123 * 1.0986123);
	k1 = log(0.6);

	for (j = k = 0; j < G->header->n_rows; j++) {
		rlat = G->header->wesn[YHI] - j * G->header->inc[GMT_Y];
		for (i = 0; i < G->header->n_columns; i++, k++) {
			rlon = G->header->wesn[XLO] + i * G->header->inc[GMT_X];
			ij = gmt_M_ijp(G->header, j, i);

			(void)gmt_near_lines (GMT, rlon, rlat, xyline, Ctrl->L.mode, &dist, &xnear, &ynear);

			r_pga = sqrt (dist*dist + 1.35*1.35);
			r_pgv = sqrt (dist*dist + 2.54*2.54);
			rfd_pga = (-0.66050+0.11970*(Ctrl->M.rmw-4.5))*log(r_pga/1.0) - 0.01151*(r_pga-1.0);
			rfd_pgv = (-0.87370+0.10060*(Ctrl->M.rmw-4.5))*log(r_pgv/1.0) - 0.00334*(r_pgv-1.0);
			rfd_pga4nl = (-0.66050+0.11970*(Ctrl->M.rmw-4.5))*log(r_pga/5.0) - 0.01151*(r_pga-5.0);

			/* Compute the site effect  */
			tmp = log(G->data[ij] / 760);
			flin_pga = -0.360 * tmp;
			flin_pgv = -0.600 * tmp;

			pga4nl = exp(rfm_pga + rfd_pga4nl);

			if (G->data[ij] <= 180.) {
				bnl_pga = -0.640;
				bnl_pgv = -0.600;
			}
			else if (G->data[ij] <= 300 && G->data[ij] > 180.) {
				tmp = log(G->data[ij]/300.) / log(180./300.);
				bnl_pga = (-0.64+0.14) * tmp - 0.14;
				bnl_pgv = (-0.60+0.06) * tmp - 0.14;
			}
			else if (G->data[ij] <= 760 && G->data[ij] > 300.) {
				tmp = log(G->data[ij]/760.) / log(300./760.);
				bnl_pga = -0.14 * tmp;
				bnl_pgv = -0.06 * tmp;
			}
			else {
				bnl_pga = bnl_pgv = 0.;
			}

			/* ---- check if the conditions are always in pga  */
			c_pga = +bnl_pga * k2;		d_pga = -bnl_pga * k3;
			c_pgv = +bnl_pgv * k2;		d_pgv = -bnl_pgv * k3;

			if (pga4nl <= 0.03) {
				fnl_pga = bnl_pga * k1;
				fnl_pgv = bnl_pgv * k1;
			}
			else if (pga4nl > 0.03 && pga4nl <= 0.09) {
				tmp3 = log(pga4nl/0.03);	tmp1 = tmp3 * tmp3;	tmp2 = tmp1 * tmp3;
				fnl_pga = bnl_pga * k1 + c_pga * tmp1 + d_pga * tmp2;
				fnl_pgv = bnl_pgv * k1 + c_pgv * tmp1 + d_pgv * tmp2;
			}
			else if (pga4nl > 0.09) {
				tmp = log (pga4nl/0.1);
				fnl_pga = bnl_pga * tmp;
				fnl_pgv = bnl_pgv * tmp;
			}

			fs_pga = flin_pga + fnl_pga;
			fs_pgv = flin_pgv + fnl_pgv;

			if (Ctrl->G.do_PGA)
				Grid[0]->data[ij] = (float)(exp (rfm_pga + rfd_pga + fs_pga) * 980.);

			if (Ctrl->G.do_PGV || Ctrl->G.do_INT)
				Grid[1]->data[ij] = (float)exp (rfm_pgv + rfd_pgv + fs_pgv);
	
			if (Ctrl->G.do_INT) {
				tmp = log10(Grid[1]->data[ij]);
				Grid[2]->data[ij] = (float)(4.398 + 1.916 * tmp + 0.280 * tmp*tmp);
			}
		}
	}

	/* Now write the one to three grids */
	if (Ctrl->C.no_PGV) Ctrl->C.selected[1] = false;	/* Don't save PGV, its usage was only temporary */
	for (k = kk = 0; k < 3; k++) {
		if (!Ctrl->C.selected[k]) continue;
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid[k]))
			Return (API->error);

		if (!API->external) kk = k;		/* On command line we pick item k from an array of 3 items */
		if (strstr (Ctrl->G.file[kk], "%s"))
			sprintf (file, Ctrl->G.file[kk], code[k]);
		else
			strncpy (file, Ctrl->G.file[kk], GMT_LEN512-1);

		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, Grid[k]) != GMT_NOERROR) {
			Return (API->error);
		}
		kk++;	/* For the externals interface we take them in the given order */
	}

	if (Ctrl->L.active)
		GMT_End_IO (API, GMT_IN, 0);	/* Disables further data input */

	if (GMT_Destroy_Data (GMT->parent, &G) != GMT_NOERROR)
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to free G\n");

	Return (GMT_NOERROR);
}
