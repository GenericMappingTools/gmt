/*--------------------------------------------------------------------
 *
 *   Copyright (c) 1999-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation; version 3 or any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * grdpmodeler will read an age grid file and a plate motion model and
 * evaluates chosen attributes such as speed, direction, lon/lat origin
 * of the grid location.
 *
 * Author:	Paul Wessel
 * Date:	1-NOV-2010
 * Ver:		5
 */

#include "gmt_dev.h"
#include "spotter.h"

#define THIS_MODULE_CLASSIC_NAME	"grdpmodeler"
#define THIS_MODULE_MODERN_NAME	"grdpmodeler"
#define THIS_MODULE_LIB		"spotter"
#define THIS_MODULE_PURPOSE	"Evaluate a plate motion model on a geographic grid"
#define THIS_MODULE_KEYS	"<G{,FD(,GG),>DG"
#define THIS_MODULE_NEEDS	"g"
#define THIS_MODULE_OPTIONS "-:RVbdhor"

#define N_PM_ITEMS	9
#define PM_AZIM		0
#define PM_DIST		1
#define PM_STAGE	2
#define PM_VEL		3
#define PM_OMEGA	4
#define PM_DLON		5
#define PM_DLAT		6
#define PM_LON		7
#define PM_LAT		8

struct GRDROTATER_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct In {
		bool active;
		char *file;
	} In;
	struct E {	/* -E[+]rotfile, -E[+]<ID1>-<ID2>, or -E<lon/lat/angle> */
		bool active;
		struct SPOTTER_ROT rot;
	} E;
	struct F {	/* -Fpolfile */
		bool active;
		char *file;
	} F;
	struct G {	/* -Goutfile */
		bool active;
		char *file;
	} G;
	struct N {	/* -N */
		bool active;
		double t_upper;
	} N;
	struct S {	/* -Sa|d|s|v|w|x|y|X|Y */
		bool active, center;
		unsigned int mode[N_PM_ITEMS];
		unsigned int n_items;
	} S;
	struct T {	/* -T<fixtime> */
		bool active;
		double value;
	} T;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDROTATER_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDROTATER_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDROTATER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);	
	gmt_M_str_free (C->E.rot.file);	
	gmt_M_str_free (C->F.file);	
	gmt_M_str_free (C->G.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <agegrdfile> %s [-F<polygontable>] [-G<outgrid>]\n", name, SPOTTER_E_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-N<upper_age>] [-SadrswxyXY]\n\t[-T<time>] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_Id_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_h_OPT, GMT_r_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<agegrdfile> is a gridded data file in geographic coordinates with crustal ages.\n");
	spotter_rot_usage (API, 'E');
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify a multi-segment closed polygon file that describes the area\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of the grid to work on [Default works on the entire grid].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set output filename with the model predictions.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Must contain %%s if more than one item is specified in -S.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default writes x,y,<predictions> to standard output\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Extend earliest stage pole back to <upper_age> [no extension].\n");
	GMT_Option (API, "Rg");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select one or more model predictions as a function of crustal age.  Choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a : Plate spreading azimuth.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   d : Distance to origin of crust in km.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r : Plate motion rate in mm/yr or km/Myr.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s : Plate motion stage ID (1 is youngest).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   w : Rotation rate in degrees/Myr.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   x : Change in longitude since formation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   y : Change in latitude since formation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   X : Longitude at origin of crust.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Y : Latitude at origin of crust.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default writes separate grids for adrswxyXY\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set fixed time of reconstruction to override age grid.\n");
	GMT_Option (API, "V,bi2,d,h,r,.");

	return (GMT_MODULE_USAGE);

}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDROTATER_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdpmodeler and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, k;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Supplemental parameters */

			case 'E':	/* File with stage poles */
				Ctrl->E.active = true;
				n_errors += spotter_parse (GMT, opt->option, opt->arg, &(Ctrl->E.rot));
				break;
			case 'F':
				if ((Ctrl->F.active = gmt_check_filearg (GMT, 'F', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->F.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'G':
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'N':	/* Extend oldest stage back to this time [no extension] */
				Ctrl->N.active = true;
				Ctrl->N.t_upper = atof (opt->arg);
				break;
			case 'S':
				Ctrl->S.active = true;
				while (opt->arg[Ctrl->S.n_items]) {
					switch (opt->arg[Ctrl->S.n_items]) {
						case 'a':	/* Plate spreading azimuth */
							Ctrl->S.mode[Ctrl->S.n_items] = PM_AZIM;	 break;
						case 'd':	/* Distance from point to origin at ridge */
							Ctrl->S.mode[Ctrl->S.n_items] = PM_DIST;	 break;
						case 's':	/* Plate motion stage ID */
							Ctrl->S.mode[Ctrl->S.n_items] = PM_STAGE; break;
						case 'v': case 'r':	/* Plate spreading rate [r is backwards compatible] */
							Ctrl->S.mode[Ctrl->S.n_items] = PM_VEL;	 break;
						case 'w':	/* Plate rotation omega */
							Ctrl->S.mode[Ctrl->S.n_items] = PM_OMEGA; break;
						case 'x':	/* Change in longitude since origin */
							Ctrl->S.mode[Ctrl->S.n_items] = PM_DLON;	Ctrl->S.center = true;	 break;
						case 'y':	/* Change in latitude since origin */
							Ctrl->S.mode[Ctrl->S.n_items] = PM_DLAT;	 break;
						case 'X':	/* Plate longitude at crust origin */
							Ctrl->S.mode[Ctrl->S.n_items] = PM_LON;	 break;
						case 'Y':	/* Plate latitude at crust origin */
							Ctrl->S.mode[Ctrl->S.n_items] = PM_LAT;	 break;
						default:
							n_errors++;		 break;
					}
					Ctrl->S.n_items++;
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.value = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->S.n_items == 0) {	/* Set default which are all the items */
		Ctrl->S.active = true;
		Ctrl->S.n_items = N_PM_ITEMS;
		for (k = 0; k < Ctrl->S.n_items; k++) Ctrl->S.mode[k] = k;
	}

	if (!Ctrl->In.file) {	/* Must have -R -I [-r] */
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET] && !GMT->common.R.active[ISET], "Syntax error: Must specify input file or -R -I [-r]\n");
		n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active, "Syntax error: Must specify -T if no age grid is given.\n");
	}
	else {	/* Must not have -I -r */
		n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[ISET] || GMT->common.R.active[GSET], "Syntax error: Cannot specify input file AND -R -r\n");
	}
	if (Ctrl->G.active) {	/* Specified output grid(s) */
		n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.n_items > 1 && !strstr (Ctrl->G.file, "%s"), "Syntax error -G: File name must be a template containing \"%s\"\n");
	}
	else
		n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file when no output grids are created\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->E.active, "Syntax error: Must specify -E\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->S.active, "Syntax error: Must specify -S\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.n_items == 0, "Syntax error: Must specify one or more fields with -S\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.value < 0.0, "Syntax error -T: Must specify positive age.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {gmt_M_free (GMT, p); Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdpmodeler (void *V_API, int mode, void *args) {
	unsigned int col, row, inside, stage, n_stages, registration, k;
	int retval, error = 0;

	bool skip, spotted;

	uint64_t node, seg, n_old = 0, n_outside = 0, n_NaN = 0;

	double lon = 0, lat = 0, d, value = 0.0, age, wesn[4], inc[2], *grd_x = NULL, *grd_y = NULL, *grd_yc = NULL, *out = NULL;

	char *quantity[N_PM_ITEMS] = { "azimuth", "distance displacement", "stage", "velocity", "rotation rate", "longitude displacement", \
		"latitude displacement", "reconstructed longitude", "reconstructed latitude"};
	char *tag[N_PM_ITEMS] = { "az", "dist", "stage", "vel", "omega", "dlon", "dlat", "lon", "lat" };
	struct GMT_DATASET *D = NULL;
	struct GMT_DATATABLE *pol = NULL;
	struct EULER *p = NULL;			/* Pointer to array of stage poles */
	struct GMT_OPTION *ptr = NULL;
	struct GMT_GRID *G_age = NULL, **G_mod = NULL, *G = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GRDROTATER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if ((ptr = GMT_Find_Option (API, 'f', options)) == NULL) gmt_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdpmodeler main code ----------------------------*/

	/* Check limits and get data file */

	if (Ctrl->In.file) {	/* Gave an age grid */
		if ((G_age = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
		gmt_M_memcpy (wesn, (GMT->common.R.active[RSET] ? GMT->common.R.wesn : G_age->header->wesn), 4, double);
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, G_age) == NULL) {
			Return (API->error);	/* Get header only */
		}
		gmt_M_memcpy (inc, G_age->header->inc, 2, double);	/* Use same increment for output grid */
		registration = G_age->header->registration;
		gmt_M_memcpy (GMT->common.R.wesn, G_age->header->wesn, 4, double);
		GMT->common.R.active[RSET] = true;
	}
	else {	/* Use the input options of -R -I [and -r] */
		gmt_M_memcpy (inc, GMT->common.R.inc, 2, double);
		registration = GMT->common.R.registration;
	}

	if (Ctrl->F.active) {	/* Read the user's clip polygon file */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_READ_NORMAL, NULL, Ctrl->F.file, NULL)) == NULL) {
			Return (API->error);
		}
		pol = D->table[0];	/* Since it is a single file */
		gmt_set_inside_mode (GMT, D, GMT_IOO_UNKNOWN);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Restrict evaluation to within polygons in file %s\n", Ctrl->F.file);
	}

	if (Ctrl->E.rot.single) {	/* Got a single rotation, no time, create a rotation table with one entry */
		n_stages = 1;
		p = gmt_M_memory (GMT, NULL, n_stages, struct EULER);
		p[0].lon = Ctrl->E.rot.lon; p[0].lat = Ctrl->E.rot.lat; p[0].omega = Ctrl->E.rot.w;
		if (gmt_M_is_dnan (Ctrl->E.rot.age)) {	/* No age, use fake age = 1 everywhere */
			Ctrl->T.active = true;
			Ctrl->T.value = Ctrl->N.t_upper = p[0].t_start = 1.0;
		}
		spotter_setrot (GMT, &(p[0]));
	}
	else	/* Got a file or Gplates plate pair */
		n_stages = spotter_init (GMT, Ctrl->E.rot.file, &p, 0, false, Ctrl->E.rot.invert, &Ctrl->N.t_upper);
	for (stage = 0; stage < n_stages; stage++) {
		if (p[stage].omega < 0.0) {	/* Ensure all stages have positive rotation angles */
			p[stage].omega = -p[stage].omega;
			p[stage].lat = -p[stage].lat;
			p[stage].lon += 180.0;
			if (p[stage].lon > 360.0) p[stage].lon -= 360.0;
		}
	}
	if (Ctrl->T.active && Ctrl->T.value > Ctrl->N.t_upper) {
		GMT_Report (API, GMT_MSG_NORMAL, "Requested a fixed reconstruction time outside range of rotation table\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->G.active) {	/* Need one or more output grids */
		G_mod = gmt_M_memory (GMT, NULL, Ctrl->S.n_items, struct GMT_GRID *);
		for (k = 0; k < Ctrl->S.n_items; k++) {
			if ((G_mod[k] = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, inc, \
				registration, GMT_NOTSET, NULL)) == NULL) {	/* Free previous grids and bail */
					unsigned int kk;
					for (kk = 0; kk < k; kk++)
						(void)GMT_Destroy_Data (API, &G_mod[kk]);
					gmt_M_free (GMT, G_mod);
					Return (API->error);
			}

			switch (Ctrl->S.mode[k]) {
				case PM_AZIM:	/* Compute plate motion direction at this point in time/space */
					strcpy (G_mod[k]->header->z_units, "degree");	     break;
				case PM_DIST:	/* Distance to origin in km */
					strcpy (G_mod[k]->header->z_units, "km");	     break;
				case PM_STAGE:	/* Compute plate motion stage at this point in time/space */
					strcpy (G_mod[k]->header->z_units, "integer");	     break;
				case PM_VEL:	/* Compute plate motion speed at this point in time/space */
					strcpy (G_mod[k]->header->z_units, "mm/yr");	     break;
				case PM_OMEGA:	/* Compute plate rotation rate omega */
					strcpy (G_mod[k]->header->z_units, "degree/Myr");    break;
				case PM_DLAT:	/* Difference in latitude relative to where this point was formed in the model */
				case PM_LAT:	/* Latitude where this point was formed in the model */
					strcpy (G_mod[k]->header->z_units, "degrees_north"); break;
				case PM_DLON:	/* Difference in longitude relative to where this point was formed in the model */
				case PM_LON:	/* Longitude where this point was formed in the model */
					strcpy (G_mod[k]->header->z_units, "degrees_east");  break;
			}
		}
		/* Just need on common set of x/y arrays; select G_mod[0] as our template */
		G = G_mod[0];
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Evaluate %d model prediction grids based on %s\n", Ctrl->S.n_items, Ctrl->E.rot.file);
	}
	else {	/* No output grids, must have input age grid to rely on */
		G = G_age;
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, Ctrl->S.n_items + 3, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Evaluate %d model predictions based on %s\n", Ctrl->S.n_items, Ctrl->E.rot.file);
		out = gmt_M_memory (GMT, NULL, Ctrl->S.n_items + 3, double);
		Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
	}

	grd_x  = gmt_M_memory (GMT, NULL, G->header->n_columns, double);
	grd_y  = gmt_M_memory (GMT, NULL, G->header->n_rows, double);
	grd_yc = gmt_M_memory (GMT, NULL, G->header->n_rows, double);
	/* Precalculate node coordinates in both degrees and radians */
	for (row = 0; row < G->header->n_rows; row++) {
		grd_y[row]  = gmt_M_grd_row_to_y (GMT, row, G->header);
		grd_yc[row] = gmt_lat_swap (GMT, grd_y[row], GMT_LATSWAP_G2O);
	}
	for (col = 0; col < G->header->n_columns; col++) grd_x[col] = gmt_M_grd_col_to_x (GMT, col, G->header);

	/* Loop over all nodes in the new rotated grid and find those inside the reconstructed polygon */


	gmt_init_distaz (GMT, 'd', GMT_GREATCIRCLE, GMT_MAP_DIST);	/* Great circle distances in degrees */
	if (Ctrl->S.center) GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;	/* Need +- around 0 here */

	gmt_M_grd_loop (GMT, G, row, col, node) {
		skip = false;
		if (Ctrl->F.active) {	/* Use the bounding polygon */
			for (seg = inside = 0; seg < pol->n_segments && !inside; seg++) {	/* Use degrees since function expects it */
				if (gmt_polygon_is_hole (GMT, pol->segment[seg])) continue;	/* Holes are handled within gmt_inonout */
				inside = (gmt_inonout (GMT, grd_x[col], grd_y[row], pol->segment[seg]) > 0);
			}
			if (!inside) skip = true, n_outside++;	/* Outside the polygon(s); set all output grids to NaN for this node */
		}
		/* Get age for this node (or the constant age) */
		if (Ctrl->T.active)
			age = Ctrl->T.value;
		else {
			age = G_age->data[node];
			if (gmt_M_is_dnan (age)) skip = true, n_NaN++;		/* No crustal age  */
			else if (age > Ctrl->N.t_upper) skip = true, n_old++;	/* Outside of model range */
		}
		if (skip) {
			if (Ctrl->G.active) for (k = 0; k < Ctrl->S.n_items; k++) G_mod[k]->data[node] = GMT->session.f_NaN;
			continue;
		}
		/* Here we are inside; get the coordinates and rotate back to original grid coordinates */
		if ((retval = spotter_stage (GMT, age, p, n_stages)) < 0) continue;	/* Outside valid stage rotation range */
		stage = retval;		/* Current rotation stage */
		spotted = false;	/* Not yet called spotter_backtrack at this node */
		if (!Ctrl->G.active) {	/* Need x,y,t for rec-by-rec output */
			out[GMT_X] = grd_x[col];	out[GMT_Y] = grd_y[row];	out[GMT_Z] = age;
		}
		for (k = 0; k < Ctrl->S.n_items; k++) {
			switch (Ctrl->S.mode[k]) {
				case PM_AZIM:	/* Compute plate motion direction at this point in time/space */
					value = gmt_az_backaz (GMT, grd_x[col], grd_yc[row], p[stage].lon, p[stage].lat, false) - 90.0;
					gmt_lon_range_adjust (GMT->current.io.geo.range, &value);
					break;
				case PM_DIST:	/* Compute great-circle distance between node and point of origin at ridge */
					if (!spotted) {
						lon = grd_x[col] * D2R;	lat = grd_yc[row] * D2R;
						(void)spotter_backtrack (GMT, &lon, &lat, &age, 1U, p, n_stages, 0.0, 0.0, 0, NULL, NULL);
						spotted = true;
					}
					value = GMT->current.proj.DIST_KM_PR_DEG * gmt_distance (GMT, grd_x[col], grd_yc[row], lon * R2D, lat * R2D);
					break;
				case PM_STAGE:	/* Compute plate rotation stage */
					value = stage;
					break;
				case PM_VEL:	/* Compute plate motion speed at this point in time/space */
					d = gmt_distance (GMT, grd_x[col], grd_yc[row], p[stage].lon, p[stage].lat);
					value = sind (d) * p[stage].omega * GMT->current.proj.DIST_KM_PR_DEG;	/* km/Myr or mm/yr */
					break;
				case PM_OMEGA:	/* Compute plate rotation rate omega */
					value = p[stage].omega;	/* degree/Myr  */
					break;
					case PM_DLON:	/* Compute latitude where this point was formed in the model */
					if (!spotted) {
						lon = grd_x[col] * D2R;	lat = grd_yc[row] * D2R;
						(void)spotter_backtrack (GMT, &lon, &lat, &age, 1U, p, n_stages, 0.0, 0.0, 0, NULL, NULL);
						spotted = true;
					}
					value = grd_x[col] - lon * R2D;
					if (fabs (value) > 180.0) value = copysign (360.0 - fabs (value), -value);
					break;
				case PM_DLAT:	/* Compute latitude where this point was formed in the model */
					if (!spotted) {
						lon = grd_x[col] * D2R;	lat = grd_yc[row] * D2R;
						(void)spotter_backtrack (GMT, &lon, &lat, &age, 1U, p, n_stages, 0.0, 0.0, 0, NULL, NULL);
						spotted = true;
					}
					value = grd_y[row] - gmt_lat_swap (GMT, lat * R2D, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					break;
				case PM_LON:	/* Compute latitude where this point was formed in the model */
					if (!spotted) {
						lon = grd_x[col] * D2R;	lat = grd_yc[row] * D2R;
						(void)spotter_backtrack (GMT, &lon, &lat, &age, 1U, p, n_stages, 0.0, 0.0, 0, NULL, NULL);
						spotted = true;
					}
					value = lon * R2D;
					break;
				case PM_LAT:	/* Compute latitude where this point was formed in the model */
					if (!spotted) {
						lon = grd_x[col] * D2R;	lat = grd_yc[row] * D2R;
						(void)spotter_backtrack (GMT, &lon, &lat, &age, 1U, p, n_stages, 0.0, 0.0, 0, NULL, NULL);
						spotted = true;
					}
					value = gmt_lat_swap (GMT, lat * R2D, GMT_LATSWAP_O2G);			/* Convert back to geodetic */
					break;
			}
			if (Ctrl->G.active)
				G_mod[k]->data[node] = (gmt_grdfloat)value;
			else
				out[k+3] = value;
		}
		if (!Ctrl->G.active) GMT_Put_Record (API, GMT_WRITE_DATA, Out);
	}

	if (n_outside) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " points fell outside the polygonal boundary\n", n_outside);
	if (n_old) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " points had ages that exceeded the limit of the rotation model\n", n_old);
	if (n_NaN) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " points had ages that were NaN\n", n_NaN);
	if (Ctrl->G.active) {	/* Need one or more output grids */
		/* Now write model prediction grid(s) */
		char file[PATH_MAX] = {""};
		for (k = 0; k < Ctrl->S.n_items; k++) {
			sprintf (file, Ctrl->G.file, tag[Ctrl->S.mode[k]]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Write model prediction grid for %s (%s) to file %s\n", quantity[Ctrl->S.mode[k]], G_mod[k]->header->z_units, file);
			strcpy (G_mod[k]->header->x_units, "degrees_east");
			strcpy (G_mod[k]->header->y_units, "degrees_north");
			snprintf (G_mod[k]->header->remark, GMT_GRID_REMARK_LEN160, "Plate Model predictions of %s for model %s", quantity[Ctrl->S.mode[k]], Ctrl->E.rot.file);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G_mod[k])) Return (API->error);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, G_mod[k]) != GMT_NOERROR) {
				Return (API->error);
			}
		}
		gmt_M_free (GMT, G_mod);
	}
	else {
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		gmt_M_free (GMT, out);
		gmt_M_free (GMT, Out);
	}

	gmt_M_free (GMT, grd_x);
	gmt_M_free (GMT, grd_y);
	gmt_M_free (GMT, grd_yc);

	Return (GMT_NOERROR);
}
