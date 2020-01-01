/*--------------------------------------------------------------------
 *
 *   Copyright (c) 1999-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * gmtpmodeler will read an lon ,at[, age] table and a plate motion model and
 * evaluates chosen attributes such as speed, direction, lon/lat origin
 * for each input point.
 *
 * Author:	Paul Wessel
 * Date:	30-NOV-2015
 * Ver:		5.3
 */

#include "gmt_dev.h"
#include "spotter.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtpmodeler"
#define THIS_MODULE_MODERN_NAME	"gmtpmodeler"
#define THIS_MODULE_LIB		"spotter"
#define THIS_MODULE_PURPOSE	"Evaluate a plate motion model at given locations"
#define THIS_MODULE_KEYS	"<D{,FD(,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vbdefghios"

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

struct GMTPMODELER_CTRL {	/* All control options for this program (except common args) */
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
	struct GMTPMODELER_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct GMTPMODELER_CTRL);
		
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTPMODELER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);	
	gmt_M_str_free (C->E.rot.file);	
	gmt_M_str_free (C->F.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <table> %s [-F<polygontable>]\n", name, SPOTTER_E_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-N<upper_age>] [-SadrswxyXY] [-T<time>] [%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<table> is a data table with geographic coordinates and optionally crustal ages.\n");
	spotter_rot_usage (API, 'E');
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify a multi-segment closed polygon file that describes the area\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of the data table to work on [Default works on the entire table].\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t   Default writes lon,lat,age,<adrswxyXY> to standard output\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set fixed time of reconstruction to override any input ages.\n");
	GMT_Option (API, "bi3,bo,d,e,h,i,o,s,:,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTPMODELER_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtpmodeler and sets parameters in CTRL.
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

	n_errors += gmt_M_check_condition (GMT, !Ctrl->E.active, "Syntax error: Must specify -E\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->S.active, "Syntax error: Must specify -S\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.n_items == 0, "Syntax error: Must specify one or more fields with -S\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.value < 0.0, "Syntax error -T: Must specify positive age.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int signum (double x) {
	if (x < 0.0) return -1;
	if (x > 0.0) return +1;
	return 0;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {if (p) gmt_M_free (GMT, p); Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtpmodeler (void *V_API, int mode, void *args) {
	unsigned int inside, stage, n_stages, k;
	int retval, error = 0, n_fields;
	
	bool spotted;
	
	uint64_t seg, n_old = 0, n_outside = 0, n_NaN = 0, n_read = 0;
	
	double lon = 0, lat = 0, lat_c, d, value = 0.0, age, *in = NULL, *out = NULL;
	
	char *tag[N_PM_ITEMS] = { "az", "dist", "stage", "vel", "omega", "dlon", "dlat", "lon", "lat" };
	struct GMT_DATASET *D = NULL;
	struct GMT_DATATABLE *pol = NULL;
	struct EULER *p = NULL;			/* Pointer to array of stage poles */
	struct GMT_RECORD *In = NULL, *Out = NULL;
	struct GMT_OPTION *ptr = NULL;
	struct GMTPMODELER_CTRL *Ctrl = NULL;
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
	
	/*---------------------------- This is the gmtpmodeler main code ----------------------------*/

	if (Ctrl->F.active) {	/* Read the user's clip polygon file */
		gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -C,-F,-L files */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_READ_NORMAL, NULL, Ctrl->F.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (D->n_columns < 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)D->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
		pol = D->table[0];	/* Since it is a single file */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Restrict evaluation to within polygons in file %s\n", Ctrl->F.file);
		gmt_set_inside_mode (GMT, D, GMT_IOO_UNKNOWN);
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
	/* Set up input */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if ((error = GMT_Set_Columns (API, GMT_IN, 2 + !Ctrl->T.active, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET,  GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}
	/* Set up output */
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
	
	gmt_init_distaz (GMT, 'd', GMT_GREATCIRCLE, GMT_MAP_DIST);	/* Great circle distances in degrees */
	if (Ctrl->S.center) GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;	/* Need +- around 0 here */

	out = gmt_M_memory (GMT, NULL, Ctrl->S.n_items + 3, double);
	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
	if (GMT->current.setting.io_header[GMT_OUT]) {
		char header[GMT_BUFSIZ] = {""};
		for (k = 0; k < Ctrl->S.n_items; k++) {
			strncat (header, tag[k], GMT_BUFSIZ-1);
			if (k < (Ctrl->S.n_items-1)) strncat (header, GMT->current.setting.io_col_separator, GMT_BUFSIZ-1);
		}
		GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, header);	/* Write a header record */
	}
	
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Evaluate %d model predictions based on %s\n", Ctrl->S.n_items, Ctrl->E.rot.file);

	/* Read the location data from file or stdin */

	do {	/* Keep returning records until we reach EOF */
		n_read++;
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (gmt_M_rec_is_table_header (GMT)) {	/* Skip all table headers */
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, NULL);
				continue;
			}
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			else if (gmt_M_rec_is_new_segment (GMT)) {	/* Parse segment headers */
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
				continue;
			}
		}

		/* Data record to process */
		in = In->data;
		Out->text = In->text;

		if (Ctrl->F.active) {	/* Use the bounding polygon */
			for (seg = inside = 0; seg < pol->n_segments && !inside; seg++) {	/* Use degrees since function expects it */
				if (gmt_polygon_is_hole (GMT, pol->segment[seg])) continue;	/* Holes are handled within gmt_inonout */
				inside = (gmt_inonout (GMT, in[GMT_X], in[GMT_Y], pol->segment[seg]) > 0);
			}
			if (!inside) {
				n_outside++;	/* Outside the polygon(s), continue */
				continue;
			}
		}
		/* Get age for this node (or use the constant age) */
		if (Ctrl->T.active)
			age = Ctrl->T.value;
		else if (n_fields == 3) {
			age = in[GMT_Z];
			if (gmt_M_is_dnan (age)) {	/* No crustal age  */
				n_NaN++;
				continue;
			}
			else if (age > Ctrl->N.t_upper) {	/* Outside of model range */
				n_old++;
				continue;
			}
		}
		else {
			GMT_Report (API, GMT_MSG_VERBOSE, "Point %" PRIu64 " has no age (%g) (skipped)\n", n_read);
			continue;
		}

		/* Here we are inside; get the coordinates and rotate back to original grid coordinates */
		if ((retval = spotter_stage (GMT, age, p, n_stages)) < 0) continue;	/* Outside valid stage rotation range */
		stage = retval;		/* Current rotation stage */
		spotted = false;	/* Not yet called spotter_backtrack at this node */
		out[GMT_X] = in[GMT_X];	out[GMT_Y] = in[GMT_Y];	out[GMT_Z] = age;	/* Set the first 3 output coordinates */
		lat_c = gmt_lat_swap (GMT, in[GMT_Y], GMT_LATSWAP_G2O);			/* Get concentric latitude */

		for (k = 0; k < Ctrl->S.n_items; k++) {	/* Loop over desired output components */
			switch (Ctrl->S.mode[k]) {
				case PM_AZIM:	/* Compute plate motion direction at this point in time/space */
					value = gmt_az_backaz (GMT, in[GMT_X], lat_c, p[stage].lon, p[stage].lat, false) + 90.0 * signum (p[stage].omega);
					gmt_lon_range_adjust (GMT->current.io.geo.range, &value);
					break;
				case PM_DIST:	/* Compute great-circle distance between node and point of origin at ridge */
					if (!spotted) {
						lon = in[GMT_X] * D2R;	lat = lat_c * D2R;
						(void)spotter_backtrack (GMT, &lon, &lat, &age, 1U, p, n_stages, 0.0, 0.0, 0, NULL, NULL);
						spotted = true;
					}
					value = GMT->current.proj.DIST_KM_PR_DEG * gmt_distance (GMT, in[GMT_X], lat_c, lon * R2D, lat * R2D);
					break;
				case PM_STAGE:	/* Compute plate rotation stage */
					value = stage;
					break;
				case PM_VEL:	/* Compute plate motion speed at this point in time/space */
					d = gmt_distance (GMT, in[GMT_X], lat_c, p[stage].lon, p[stage].lat);
					value = sind (d) * p[stage].omega * GMT->current.proj.DIST_KM_PR_DEG;	/* km/Myr or mm/yr */
					break;
				case PM_OMEGA:	/* Compute plate rotation rate omega */
					value = p[stage].omega;	/* degree/Myr  */
					break;
					case PM_DLON:	/* Compute latitude where this point was formed in the model */
					if (!spotted) {
						lon = in[GMT_X] * D2R;	lat = lat_c * D2R;
						(void)spotter_backtrack (GMT, &lon, &lat, &age, 1U, p, n_stages, 0.0, 0.0, 0, NULL, NULL);
						spotted = true;
					}
					value = in[GMT_X] - lon * R2D;
					if (fabs (value) > 180.0) value = copysign (360.0 - fabs (value), -value);
					break;
				case PM_DLAT:	/* Compute latitude where this point was formed in the model */
					if (!spotted) {
						lon = in[GMT_X] * D2R;	lat = lat_c * D2R;
						(void)spotter_backtrack (GMT, &lon, &lat, &age, 1U, p, n_stages, 0.0, 0.0, 0, NULL, NULL);
						spotted = true;
					}
					value = in[GMT_Y] - gmt_lat_swap (GMT, lat * R2D, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					break;
				case PM_LON:	/* Compute latitude where this point was formed in the model */
					if (!spotted) {
						lon = in[GMT_X] * D2R;	lat = lat_c * D2R;
						(void)spotter_backtrack (GMT, &lon, &lat, &age, 1U, p, n_stages, 0.0, 0.0, 0, NULL, NULL);
						spotted = true;
					}
					value = lon * R2D;
					break;
				case PM_LAT:	/* Compute latitude where this point was formed in the model */
					if (!spotted) {
						lon = in[GMT_X] * D2R;	lat = lat_c * D2R;
						(void)spotter_backtrack (GMT, &lon, &lat, &age, 1U, p, n_stages, 0.0, 0.0, 0, NULL, NULL);
						spotted = true;
					}
					value = gmt_lat_swap (GMT, lat * R2D, GMT_LATSWAP_O2G);			/* Convert back to geodetic */
					break;
			}
			out[k+3] = value;
		}
		GMT_Put_Record (API, GMT_WRITE_DATA, Out);
	} while (true);

	if (GMT_End_IO (API, GMT_IN,  0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}
	
	if (n_outside) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " points fell outside the polygonal boundary\n", n_outside);
	if (n_old) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " points had ages that exceeded the limit of the rotation model\n", n_old);
	if (n_NaN) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " points had ages that were NaN\n", n_NaN);

	gmt_M_free (GMT, out);
	gmt_M_free (GMT, Out);
	
	Return (GMT_NOERROR);
}
