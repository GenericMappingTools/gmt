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
 */

#include "gmt_dev.h"
#include "spotter.h"

#define THIS_MODULE_CLASSIC_NAME	"polespotter"
#define THIS_MODULE_MODERN_NAME	"polespotter"
#define THIS_MODULE_LIB		"spotter"
#define THIS_MODULE_PURPOSE	"Find stage poles given fracture zones and abyssal hills"
#define THIS_MODULE_KEYS	"AD(,CD),FD(,GG},LD)"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>RVbdefghios" GMT_OPT("HMm")

#define RADIAN2KM (R2D * GMT->current.proj.DIST_KM_PR_DEG)

struct POLESPOTTER_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -A<abyssalhilefile> */
		bool active;
		char *file;
		double weight;
	} A;
	struct D {	/* -D<spacing> */
		bool active;
		double length;
	} D;
	struct E {	/* -Ea|f<sigma> */
		bool active;
	} E;
	struct F {	/* -F<fzfile> */
		bool active;
		char *file;
		double weight;
	} F;
	struct G {	/* -Goutfile */
		bool active;
		char *file;
	} G;
	struct N {	/* -N */
		bool active;
	} N;
	struct S {	/* -Ss|l|p[<modifiers>] */
		bool active;
		bool dump_lines;
		bool dump_crossings;
		bool midpoint;
		unsigned int mode;
		char *file;
		double plon, plat;
	} S;
};

enum spotter_modes {
	SPOTTER_SCAN_SPOTS = 0,
	SPOTTER_SCAN_LINES = 1,
	SPOTTER_SCAN_POLES = 2};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct POLESPOTTER_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct POLESPOTTER_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->A.weight = C->F.weight = 1.0;
	C->D.length = 5.0;	/* 5 km spacing */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct POLESPOTTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->A.file);	
	gmt_M_str_free (C->F.file);	
	gmt_M_str_free (C->S.file);	
	gmt_M_str_free (C->G.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [%s] [-G<polegrid>] [%s]\n", name, GMT_Id_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-A<abyssalhills>] [-D<step>] [-Ea|f<sigma>] [-F<FZfile] [-N] [%s]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Ss|p|l[<modifiers>]] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_bi_OPT, GMT_d_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_o_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Give multisegment file with abyssal hill lineaments [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Give step-length along great circles in km [5].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Specify typical angular error (in degrees) for (a)byssal hills or (f)racture zones [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Give multisegment file with fracture zone lineaments [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output grid [no grid].  Requires -R -I [-r]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Accumulates weighted great-circle length density grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Specify grid interval(s); Append m [or s] to <dx> and/or <dy> for minutes [or seconds].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Normalize grid to max = 1 [no normalization].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Determines the spotter mode.  Select from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  -Ss scan for spots [default].  This mode offers two optional modifiers:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append +l to dump all great circles to stdout [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append +c<xfile> to save all great circle intersections to <xfile> [no crossings].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  -Sp scan for poles.  Writes a misfit grid to <grid>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  -Sl scan for compatible lines given <plon>/<plat> trial pole.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append +m to report misfit for each midpoint.\n");
	GMT_Option (API, "Rg");
	GMT_Option (API, "V");
	GMT_Option (API, "bi2,d,e,h,i,o,r,s,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct POLESPOTTER_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to polespotter and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
int n;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			/* Supplemental parameters */

			case 'A':	/* File with abyssal hill traces */
				if ((Ctrl->A.active = gmt_check_filearg (GMT, 'A', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->A.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'D':	/* Step length */
				Ctrl->D.active = true;
				Ctrl->D.length = atof (opt->arg);
				break;
			case 'E':	/* Sigma of lines (store 1/sigma here) */
				switch (opt->arg[0]) {
					case 'a': Ctrl->A.weight = 1.0 / atof (&opt->arg[1]);	break;
					case 'f': Ctrl->F.weight = 1.0 / atof (&opt->arg[1]);	break;
				}
				Ctrl->E.active = true;
				break;
			case 'F':	/* File with fracture zone traces */
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
			case 'N':	/* Normalize grid */
				Ctrl->N.active = true;
				break;
			case 'S':	/* modes */
				switch (opt->arg[0]) {
					case 'p': Ctrl->S.mode = SPOTTER_SCAN_POLES;
						break;
					case 'l': Ctrl->S.mode = SPOTTER_SCAN_LINES;
						if (gmt_validate_modifiers (GMT, opt->arg, 'S', "m")) n_errors++;
						if ((c = strstr (opt->arg, "+m"))) {	/* Do midpoint analysis instead */
							Ctrl->S.midpoint = true;
							c[0] = '\0';	/* Chop off modifier */
						}
						if ((n = sscanf (&opt->arg[1], "%[^/]/%s", txt_a, txt_b)) != 2) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -Sp: No pole given\n");
							n_errors++;
						}
						else {
							n_errors += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf_arg (GMT, txt_a, GMT_IS_LON, false, &Ctrl->S.plon), txt_a);
							n_errors += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf_arg (GMT, txt_b, GMT_IS_LAT, false, &Ctrl->S.plat), txt_b);
						}
						if (c) c[0] = '+';	/* Restore modifier */
						break;
					case 's': Ctrl->S.mode = SPOTTER_SCAN_SPOTS;
						if (gmt_validate_modifiers (GMT, opt->arg, 'S', "cl")) n_errors++;
						if (gmt_get_modifier (opt->arg, 'l', txt_a))	/* Dump lines to stdout */
							Ctrl->S.dump_lines = true;
						if (gmt_get_modifier (opt->arg, 'c', txt_a) && txt_a[0]) {	/* Crossing output file */
							Ctrl->S.dump_crossings = true;
							Ctrl->S.file = strdup (txt_a);
							
						}
						break;
					default:
						n_errors++;
				}
				Ctrl->S.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

        if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 2, "Syntax error: Binary input data (-bi) must have at least 3 columns\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->A.active && !Ctrl->F.active, "Syntax error: At least one of -A or -F is required.\n");
	if (Ctrl->G.active) {
		n_errors += gmt_M_check_condition (GMT, Ctrl->G.file == NULL, "Syntax error option -G: Must specify output file\n");
		n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->D.length <= 0.0, "Syntax error -D: Must specify a positive length step.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == SPOTTER_SCAN_SPOTS && !Ctrl->S.dump_lines && !Ctrl->G.active, "Syntax error -Ss: Must specify at least one of -G, -L.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.dump_crossings && !Ctrl->S.file, "Syntax error -Ss: Must specify a file name if +c is used.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define POLESPOTTER_AH	0
#define POLESPOTTER_FZ	1

EXTERN_MSC void gmtlib_load_rot_matrix (double w, double R[3][3], double E[]);
EXTERN_MSC void gmtlib_init_rot_matrix (double R[3][3], double E[]);

GMT_LOCAL void get_cross_normalized (struct GMT_CTRL *GMT, double P1[], double P2[], double G[]) {
	gmt_cross3v (GMT, P1, P2, G);	/* G is the pole of the great circle that passes through P1 & P2 */
	gmt_normalize3v (GMT, G);	/* But we need to normalize it */
}

GMT_LOCAL void get_great_circle_pole (struct GMT_CTRL *GMT, double P1[], double P2[], unsigned int type, double M[], double G[]) {
	/* Input is P1 and P2, the two cartesian points defining a small great circle segment.
	 * Output is M (the mid point of the segment) and G, the great circle defining the bisector (FZ) or segment itself (AH) */
	unsigned int k;
	for (k = 0; k < 3; k++) M[k] = 0.5 * (P1[k] + P2[k]);	/* Mid-point M */
	gmt_normalize3v (GMT, M);	/* To get a unit length vector M */
	get_cross_normalized (GMT, P1, P2, G);	/* G is the pole of the great circle that passes through P1 & P2 */
	if (type == POLESPOTTER_FZ) {	/* Must get the bisector pole instead, so cross it with M */
		double B[3];	/* Temp vector */
		get_cross_normalized (GMT, M, G, B);	/* This gives the normalied bisector pole instead */
		gmt_M_memcpy (G, B, 3, double);		/* Put bisector pole into G which is what we return */
	}
}

GMT_LOCAL double get_angle_between_trends (struct GMT_CTRL *GMT, double P1[], double P2[], unsigned int type, double X[]) {
	/* P1 and P2 are two points on a FZ or AH.  Midpoint of P1 and P2 is M.  X is a trial pole.
	 * If type = FZ: Find difference in orientations between bisector to P1-P2 and great circle from X through M.
	 * If type = AH: Find difference in orientations between great circle through P1-P2 and great circle from X through M.
	 */
	double M[3], G[3], B[3], cos_del_angle, del_angle;
	get_great_circle_pole (GMT, P1, P2, type, M, G);	/* Obtain great circle pole to segment (or bisector if FZ) as well as mid-point M */
	get_cross_normalized (GMT, M, X, B);			/* B is pole for great circle through selected pole and M */
	cos_del_angle = gmt_dot3v (GMT, G, B);			/* Cos of angle between great circles is our other weight */
	del_angle = fabs (d_acos (cos_del_angle));		/* Get |angle| between the two great circles */
	if (del_angle > M_PI_2) del_angle = M_PI - del_angle;	/* Since angles are actually orientation differences */
	return (del_angle);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_polespotter (void *V_API, int mode, void *args) {
	bool create_great_circles;
	int error;
	unsigned int d, n_steps, grow, gcol, k;
	uint64_t node, tbl, seg, row, ng = 0;
	size_t n_alloc = 0;
	char header[GMT_LEN128] = {""}, *code = NULL;
	const char *label[2] = {"AH", "FZ"};
	gmt_grdfloat *layer = NULL;
	double weight, seg_weight, angle_radians, d_angle_radians, mlon, mlat, glon, glat, L, in[2];
	double P1[3], P2[3], M[3], G[3], X[3];
	
	struct GMT_OPTION *ptr = NULL;
	struct GMT_GRID *Grid = NULL;
	struct GMT_DATASET *In[2] = {NULL, NULL};
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_RECORD *Out = NULL;
	struct POLESPOTTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args); if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	if ((ptr = GMT_Find_Option (API, 'f', options)) == NULL) gmt_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the polespotter main code ----------------------------*/

	if (Ctrl->A.active && (In[POLESPOTTER_AH] = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->A.file, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to open file with abyssal hill lineaments: %s", Ctrl->A.file);
		Return (API->error);
	}
	if (Ctrl->F.active && (In[POLESPOTTER_FZ] = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->F.file, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to open file with fracture zone lineaments: %s", Ctrl->F.file);
		Return (API->error);
	}

	/* Initialize the pole search grid and structure */

	if (Ctrl->G.active) {
		if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL, \
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
		if (Ctrl->S.mode == SPOTTER_SCAN_SPOTS)
			layer = gmt_M_memory_aligned (GMT, NULL, Grid->header->size, gmt_grdfloat);
	}

	/* Determine how often to sample the great circle given -D.  Since all great cirlces are sampled this way
	 * we only do a cos(lat) weighting for the line density if -G is used. */
	n_steps = urint (360.0 * GMT->current.proj.DIST_KM_PR_DEG / Ctrl->D.length);
	d_angle_radians = TWO_PI / (n_steps - 1);

	if (Ctrl->S.dump_lines) {
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		Out = gmt_new_record (GMT, in, NULL);	/* Since we only need to worry about numerics in this module */
	}

	if (Ctrl->S.mode == SPOTTER_SCAN_SPOTS) {
		double Rot0[3][3], Rot[3][3], *GG = NULL;
		/* Loop over all abyssal hill and fracture zone lines and consider each subsecutive pair of points to define a great circle that intersects the pole */
	
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Entering scan mode: spots\n");
		create_great_circles = (Ctrl->G.active || Ctrl->S.dump_lines);
		if (Ctrl->S.dump_crossings) {	/* Need temporary storage for all great circle poles and their weights and type */
			n_alloc = GMT_BIG_CHUNK;
			GG = gmt_M_memory (GMT, NULL, n_alloc*4, double);
			code = gmt_M_memory (GMT, NULL, n_alloc, char);
		}
		for (d = POLESPOTTER_AH; d <= POLESPOTTER_FZ; d++) {
			if (In[d] == NULL) continue;	/* Don't have this data set */
			weight = (d == POLESPOTTER_AH) ? Ctrl->A.weight : Ctrl->F.weight;
			for (tbl = 0; tbl < In[d]->n_tables; tbl++) {
				for (seg = 0; seg < In[d]->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
					S = In[d]->table[tbl]->segment[seg];	/* Set shortcut to current segment */
					if (gmt_parse_segment_item (GMT, S->header, "-D", header))	/* Found -D<sigma> */
						seg_weight = 1.0 / atof (header);
					else
						seg_weight = weight;	/* Already got 1/sigma via -E, actually */
					/* Convert the entire segment to geocentric latitude */
					for (row = 0; row < S->n_rows; row++) S->data[GMT_Y][row] = gmt_lat_swap (GMT, S->data[GMT_Y][row], GMT_LATSWAP_G2O);
					gmt_geo_to_cart (GMT, S->data[GMT_Y][0], S->data[GMT_X][0], P1, true);	/* get x/y/z of first point P1 */
					for (row = 1; row < S->n_rows; row++) {
						if (Ctrl->G.active) gmt_M_memset (layer, Grid->header->size, gmt_grdfloat);
						gmt_geo_to_cart (GMT, S->data[GMT_Y][row], S->data[GMT_X][row], P2, true);	/* get x/y/z of 2nd point P2 */
						L = d_acos (gmt_dot3v (GMT, P1, P2)) * RADIAN2KM * seg_weight;	/* Weighted length of this segment */
						get_great_circle_pole (GMT, P1, P2, d, M, G);	/* Obtain great circle pole to segment (or bisector if FZ) */
						if (Ctrl->S.dump_crossings) {	/* Keep track of great circles to each line */
							gmt_M_memcpy (&GG[4*ng], G, 3, double);
							GG[4*ng+3] = L;
							code[ng++] = (char)d;
							if (ng == n_alloc) {
								n_alloc <<= 1;
								GG = gmt_M_memory (GMT, GG, n_alloc*4, double);
								code = gmt_M_memory (GMT, code, n_alloc, char);
							}
						}
						if (create_great_circles) {
							gmt_cart_to_geo (GMT, &mlat, &mlon, M, true);	/* Get lon/lat of the mid point */
							gmt_cart_to_geo (GMT, &glat, &glon, G, true);	/* Get lon/lat of the mid point */
							gmtlib_init_rot_matrix (Rot0, G);	/* Get partial rotation matrix since no actual angle is applied yet */
							sprintf (header, "Great circle: Center = %g/%g and pole is %g/%g -I%s", mlon, mlat, glon, glat, label[d]);
							if (Ctrl->S.dump_lines) GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, header);
							for (k = 0; k < n_steps; k++) {
								angle_radians = k * d_angle_radians;			/* The angle around the great circle (0-360) */
								gmt_M_memcpy (Rot, Rot0, 9, double);			/* Get a copy of the "0-angle" rotation matrix */
								gmtlib_load_rot_matrix (angle_radians, Rot, G);		/* Build the actual rotation matrix for this angle */
								gmt_matrix_vect_mult (GMT, 3U, Rot, M, X);		/* Rotate the mid point along the great circle */
								gmt_cart_to_geo (GMT, &in[GMT_Y], &in[GMT_X], X, true);		/* Get lon/lat of this point along crossing profile */
								in[GMT_Y] = gmt_lat_swap (GMT, in[GMT_Y], GMT_LATSWAP_G2O + 1);	/* Convert back to geodetic */
								if (Ctrl->S.dump_lines) GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* We are writing these circles to output */
								if (!Ctrl->G.active) continue;	/* Not doing density grid here */
								if (gmt_M_y_is_outside (GMT, in[GMT_Y], Grid->header->wesn[YLO], Grid->header->wesn[YHI])) continue;		/* Outside y-range */
								if (gmt_x_is_outside (GMT, &in[GMT_X], Grid->header->wesn[XLO], Grid->header->wesn[XHI])) continue;		/* Outside x-range (or periodic longitude) */
								if (gmt_row_col_out_of_bounds (GMT, in, Grid->header, &grow, &gcol)) continue;	/* Sorry, outside after all */
								node = gmt_M_ijp (Grid->header, grow, gcol);		/* Bin index */
								layer[node] = (gmt_grdfloat)(cosd (in[GMT_Y]) * L);			/* Any bin intersected will have this single value despite perhaps many intersections */
							}
							if (Ctrl->G.active) {	/* Add density layer of this great circle to the total density grid */
								for (node = 0; node < Grid->header->size; node++) Grid->data[node] += layer[node];
							}
						}
						gmt_M_memcpy (P1, P2, 3, double);	/* Let old P2 be next P1 */
					}
				}
			}
		}
		if (Ctrl->G.active) gmt_M_free_aligned (GMT, layer);

		if (Ctrl->S.dump_lines) {	/* Disables further line output */
			if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further line output */
				Return (API->error);
			}
			gmt_M_free (GMT, Out);
		}

		if (Ctrl->S.dump_crossings) {	/* Generate crossings and write them to the crossings file */
			uint64_t dim[4] = {1, 1, 0, 5}, n_cross, g1, g2;
			struct GMT_DATASET *C = NULL;
			struct GMT_DATASEGMENT *S = NULL;
			n_cross = ng * (ng - 1);
			dim[GMT_ROW] = n_cross;
			if ((C = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
				Return (API->error);
			S = C->table[0]->segment[0];	/* Only have a single segment here*/
			for (g1 = k = 0; g1 < ng; g1++) {
				for (g2 = g1+1; g2 < ng; g2++) {	/* Get circle intersections (2) */
					get_cross_normalized (GMT, &GG[4*g1], &GG[4*g2], X);	/* X is great circle intersection */
					gmt_cart_to_geo (GMT, &S->data[GMT_Y][k], &S->data[GMT_X][k], X, true);		/* Get lon/lat of this point along crossing profile */
					S->data[GMT_Y][k] = gmt_lat_swap (GMT, S->data[GMT_Y][k], GMT_LATSWAP_G2O + 1);	/* Convert to geodetic */
					S->data[GMT_Z][k] = S->data[GMT_Z][k+1] = hypot (GG[4*g1+3], GG[4*g2+3]);	/* Combined length in quadrature */
					S->data[3][k] = S->data[3][k+1] = gmt_dot3v (GMT, &GG[4*g1], &GG[4*g2]);	/* Cos of angle between great circles is our other weight */
					S->data[4][k] = S->data[4][k+1] = code[g1] + code[g2];				/* 0 = AH&AH, 1 = AH&FZ, 2 = FZ&FZ */
					S->data[GMT_X][k+1] = S->data[GMT_X][k] + 180.0;
					S->data[GMT_Y][k+1] = -S->data[GMT_Y][k];
					k += 2;
				}
			}
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_WRITE_SET, NULL, Ctrl->S.file, C) != GMT_NOERROR) {
				Return (API->error);
			}
			gmt_M_free (GMT, GG);
			gmt_M_free (GMT, code);
		}
	}
	else if (Ctrl->S.mode == SPOTTER_SCAN_LINES) {	/* Determine which lines are compatible with the selected test pole */
		double out[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, plat, sum_L = 0.0, del_angle, chi2, this_chi2;
		unsigned int n_out = (Ctrl->S.midpoint) ? 6 : 3;
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Entering scan mode: lines [EXPERIMENTAL]\n");
		gmt_set_cartesian (GMT, GMT_OUT);	/* Since x here will be table number and y is segment number */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, n_out, GMT_COL_FIX)) != GMT_NOERROR) {
			Return (error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */

		plat = gmt_lat_swap (GMT, Ctrl->S.plat, GMT_LATSWAP_G2O);	/* Convert latitude to geodetic */
		gmt_geo_to_cart (GMT, plat, Ctrl->S.plon, X, true);	/* Get x/y/z of selected pole X */

		/* Now visit all our segments */
		for (d = POLESPOTTER_AH; d <= POLESPOTTER_FZ; d++) {
			if (In[d] == NULL) continue;	/* Don't have this data set */
			weight = (d == POLESPOTTER_AH) ? Ctrl->A.weight : Ctrl->F.weight;
			Out->text = (char *)label[d];
			for (tbl = 0; tbl < In[d]->n_tables; tbl++) {
				for (seg = 0; seg < In[d]->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
					S = In[d]->table[tbl]->segment[seg];	/* Set shortcut to current segment */
					if (gmt_parse_segment_item (GMT, S->header, "-D", header))	/* Found -D<val> */
						seg_weight = 1.0 / atof (header);
					else
						seg_weight = weight;	/* Already 1/sigma, actually */
					/* Reminder, latitudes in segments are now geocentric latitudes */
					chi2 = sum_L = 0.0;
					if (Ctrl->S.midpoint) GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);	/* Output the segment header first */
					gmt_geo_to_cart (GMT, S->data[GMT_Y][0], S->data[GMT_X][0], P1, true);	/* Get x/y/z of first point P1 */
					for (row = 1; row < S->n_rows; row++) {
						gmt_geo_to_cart (GMT, S->data[GMT_Y][row], S->data[GMT_X][row], P2, true);	/* get x/y/z of 2nd point P2 */
						L = d_acos (gmt_dot3v (GMT, P1, P2)) * RADIAN2KM;	/* Length of this segment */
						del_angle =  get_angle_between_trends (GMT, P1, P2, d, X);
						this_chi2 = pow (del_angle * seg_weight, 2.0);	/* The chi2 increment from the P1-P2 line */
						gmt_M_memcpy (P1, P2, 3, double);		/* Let old P2 be next P1 */
						if (Ctrl->S.midpoint) {	/* Report for this mid-point */
							gmt_cart_to_geo (GMT, &mlat, &mlon, M, true);	/* Get lon/lat of the mid point */
							mlat = gmt_lat_swap (GMT, mlat, GMT_LATSWAP_G2O + 1);	/* Convert back to geodetic */
							out[GMT_X] = mlon;	out[GMT_Y] = mlat;
							out[GMT_Z] = del_angle;
							out[3] = this_chi2;
							out[4] = (double)tbl;
							out[5] = (double)seg;
							GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Report <mlon mlat del_angle chi2 tbl seg type> for this M */
						}
						else {	/* Summarize for this line instead */
							chi2  += L * this_chi2;	/* The weighted chi2 sum from this line */
							sum_L += L;		/* Add up total weight sum */
						}
					}
					if (!Ctrl->S.midpoint) {	/* Write <tbl seg chi2 type> for this segment */
						Out->data[GMT_X] = (double)tbl;	Out->data[GMT_Y] = (double)seg;	Out->data[GMT_Z] = chi2 / sum_L;
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* We are writing these circles to output */
					}
				}
			}
		}
		/* Disables further line output */
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further line output */
			Return (API->error);
		}
		gmt_M_free (GMT, Out);
	}
	else {	/* SPOTTER_SCAN_POLES */
		double *plon = NULL, *plat = NULL, sum_L = 0.0, del_angle, chi2;

		/* Now visit all our segments to convert to geocentric and to get sum of weights once */
		
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Entering scan mode: poles\n");
		for (d = POLESPOTTER_AH; d <= POLESPOTTER_FZ; d++) {
			if (In[d] == NULL) continue;	/* Don't have this data set */
			weight = (d == POLESPOTTER_AH) ? Ctrl->A.weight : Ctrl->F.weight;
			for (tbl = 0; tbl < In[d]->n_tables; tbl++) {
				for (seg = 0; seg < In[d]->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
					S = In[d]->table[tbl]->segment[seg];	/* Set shortcut to current segment */
					/* Convert the entire segment to geocentric latitude as we go through */
					S->data[GMT_Y][0] = gmt_lat_swap (GMT, S->data[GMT_Y][0], GMT_LATSWAP_G2O);
					gmt_geo_to_cart (GMT, S->data[GMT_Y][0], S->data[GMT_X][0], P1, true);	/* get x/y/z of first point P1 */
					for (row = 1; row < S->n_rows; row++) {
						S->data[GMT_Y][row] = gmt_lat_swap (GMT, S->data[GMT_Y][row], GMT_LATSWAP_G2O);
						gmt_geo_to_cart (GMT, S->data[GMT_Y][row], S->data[GMT_X][row], P2, true);	/* get x/y/z of 2nd point P2 */
						L = d_acos (gmt_dot3v (GMT, P1, P2)) * RADIAN2KM;	/* Length of this segment */
						sum_L += L;	/* Add up total weight sum */
						gmt_M_memcpy (P1, P2, 3, double);		/* Let old P2 be next P1 */
					}
				}
			}
		}
		/* Now we know sum_L which we will divide our grid by at the end */
		
		plon = gmt_grd_coord (GMT, Grid->header, GMT_X);
		plat = gmt_grd_coord (GMT, Grid->header, GMT_Y);
		for (grow = 0; grow < Grid->header->n_rows; grow++) {	/* Try all possible pole latitudes in selected region */
			plat[grow] = gmt_lat_swap (GMT, plat[grow], GMT_LATSWAP_G2O);	/* Convert latitude to geodetic */
			for (gcol = 0; gcol < Grid->header->n_columns; gcol++) {	/* Try all possible pole longitudes in selected region */
				node = gmt_M_ijp (Grid->header, grow, gcol);		/* Current grid node */
				gmt_geo_to_cart (GMT, plat[grow], plon[gcol], X, true);	/* Get x/y/z of current pole X */
				/* Now visit all our segments */
				for (d = POLESPOTTER_AH; d <= POLESPOTTER_FZ; d++) {
					if (In[d] == NULL) continue;	/* Don't have this data set */
					weight = (d == POLESPOTTER_AH) ? Ctrl->A.weight : Ctrl->F.weight;
					for (tbl = 0; tbl < In[d]->n_tables; tbl++) {
						for (seg = 0; seg < In[d]->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
							S = In[d]->table[tbl]->segment[seg];	/* Set shortcut to current segment */
							if (gmt_parse_segment_item (GMT, S->header, "-D", header))	/* Found -D<val> */
								seg_weight = 1.0 / atof (header);
							else
								seg_weight = weight;	/* Already 1/sigma, actually */
							/* Reminder, latitudes in segments are now geocentric latitudes */
							gmt_geo_to_cart (GMT, S->data[GMT_Y][0], S->data[GMT_X][0], P1, true);	/* Get x/y/z of first point P1 */
							for (row = 1; row < S->n_rows; row++) {
								gmt_geo_to_cart (GMT, S->data[GMT_Y][row], S->data[GMT_X][row], P2, true);	/* get x/y/z of 2nd point P2 */
								L = d_acos (gmt_dot3v (GMT, P1, P2)) * RADIAN2KM;	/* Length of this segment */
								del_angle =  get_angle_between_trends (GMT, P1, P2, d, X);
								chi2 = L * pow (del_angle * seg_weight, 2.0);	/* The weighted chi2 increment from this line */
								Grid->data[node] += (gmt_grdfloat)chi2;		/* Add to total chi2 misfit for this pole */
								gmt_M_memcpy (P1, P2, 3, double);		/* Let old P2 be next P1 */
							}
						}
					}
				}
			}
		}
		gmt_M_free (GMT, plon);
		gmt_M_free (GMT, plat);
			
		for (node = 0; node < Grid->header->size; node++) Grid->data[node] /= (gmt_grdfloat)sum_L;	/* Correct for weight sum */
	}
	if (Ctrl->G.active) {	/* Write the spotting grid */
		double max = -DBL_MAX;
		if (Ctrl->N.active) {	/* Normalize grid */
			for (node = 0; node < Grid->header->size; node++) if (Grid->data[node] > max) max = Grid->data[node];	/* Find max value */
			max = 1.0 / max;	/* Do division here */
			for (node = 0; node < Grid->header->size; node++) Grid->data[node] *= (gmt_grdfloat)max;	/* Normalize */
		}
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	
	Return (GMT_NOERROR);
}
