/*--------------------------------------------------------------------
 *	$Id$
 *
 *   Copyright (c) 1999-2017 by P. Wessel
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
 *   Contact info: www.soest.hawaii.edu/pwessel
 *--------------------------------------------------------------------*/
/*
 */

#include "gmt_dev.h"
#include "spotter.h"

#define THIS_MODULE_NAME	"polespotter"
#define THIS_MODULE_LIB		"spotter"
#define THIS_MODULE_PURPOSE	"Find stage poles given fracture zones and abyssal hills"
#define THIS_MODULE_KEYS	"AD(,FD(,GG},LD)"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>RVbdefghios" GMT_OPT("HMm")

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
	struct F {	/* -F<fzfile> */
		bool active;
		char *file;
		double weight;
	} F;
	struct G {	/* -Goutfile */
		bool active;
		char *file;
	} G;
	struct L {	/* -L */
		bool active;
	} L;
	struct W {	/* -Wa|f<weight> */
		bool active;
	} W;
};

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
	gmt_M_str_free (C->G.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: polespotter [%s] [-G<polegrid>] [%s]\n", GMT_Id_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-A<abyssalhills>] [-D<step>] [-F<FZfile] [-L] [%s]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Wa|f<weight> ][%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_bi_OPT, GMT_d_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Give multisegment file with abyssal hill lineaments [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Give step-length along great circles in km [5].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Give multisegment file with fracture zone lineaments [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for polesearch grid [no gridding].  Requires -R -I [-r]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Specify grid interval(s); Append m [or s] to <dx> and/or <dy> for minutes [or seconds].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Dump all great circle lines to stdout [no lines are written].\n");
	GMT_Option (API, "Rg");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Specify relative weights for (a)byssal hills or (f)racture zones [1].\n");
	GMT_Option (API, "bi2,d,e,h,i,r,s,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct POLESPOTTER_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to polespotter and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
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
			case 'L':	/* Circle dump */
				Ctrl->L.active = true;
				break;
			case 'W':	/* Weights */
				switch (opt->arg[0]) {
					case 'a': Ctrl->A.weight = atof (&opt->arg[1]);	break;
					case 'f': Ctrl->F.weight = atof (&opt->arg[1]);	break;
				}
				Ctrl->W.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

        if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 2, "Syntax error: Binary input data (-bi) must have at least 3 columns\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->A.active && !Ctrl->F.active, "Syntax error: At least one of -A or -F are required.\n");
	if (Ctrl->G.active) {
		n_errors += gmt_M_check_condition (GMT, Ctrl->G.file == NULL, "Syntax error option -G: Must specify output file\n");
		n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->D.length <= 0.0, "Syntax error -D: Must specify a positive length step.\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->L.active && !Ctrl->G.active, "Syntax error: Must specify at least one of -G, -L.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define POLESPOTTER_AH	0
#define POLESPOTTER_FZ	1

EXTERN_MSC void gmtlib_load_rot_matrix (double w, double R[3][3], double E[]);
EXTERN_MSC void gmtlib_init_rot_matrix (double R[3][3], double E[]);

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_polespotter (void *V_API, int mode, void *args) {
	int error;
	unsigned int d, n_steps, grow, gcol, k;
	uint64_t node, tbl, seg, row;
	char header[GMT_LEN128] = {""};
	gmt_grdfloat *layer = NULL;
	double weight, angle_radians, d_angle_radians, mlon, mlat, glon, glat, in[2];
	double P1[3], P2[3], M[3], G[3], B[3], X[3], Rot0[3][3], Rot[3][3];
	
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

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
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
		layer = gmt_M_memory_aligned (GMT, NULL, Grid->header->size, gmt_grdfloat);
	}

	/* Determine how often to sample the great circle given -D */
	n_steps = urint (360.0 * GMT->current.proj.DIST_KM_PR_DEG / Ctrl->D.length);
	d_angle_radians = TWO_PI / (n_steps - 1);

	if (Ctrl->L.active) {
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
	
	/* Loop over all abyssal hill and fracture zone lines and consider each subsecutive pair of points to define a great circle that intersects the pole */
	
	for (d = POLESPOTTER_AH; d <= POLESPOTTER_FZ; d++) {
		if (In[d] == NULL) continue;	/* Don't have this data set */
		weight = (d == POLESPOTTER_AH) ? Ctrl->A.weight : Ctrl->F.weight;
		for (tbl = 0; tbl < In[d]->n_tables; tbl++) {
			for (seg = 0; seg < In[d]->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
				S = In[d]->table[tbl]->segment[seg];	/* Set shortcut to current segment */
				/* Convert the entire segment to geocentric latitude */
				for (row = 0; row < S->n_rows; row++) S->data[GMT_Y][row] = gmt_lat_swap (GMT, S->data[GMT_Y][row], GMT_LATSWAP_G2O);
				gmt_geo_to_cart (GMT, S->data[GMT_Y][0], S->data[GMT_X][0], P1, true);	/* get x/y/z of first point P1 */
				for (row = 1; row < S->n_rows; row++) {
					if (Ctrl->G.active) gmt_M_memset (layer, Grid->header->size, gmt_grdfloat);
					gmt_geo_to_cart (GMT, S->data[GMT_Y][row], S->data[GMT_X][row], P2, true);	/* get x/y/z of 2nd point P2 */
					for (k = 0; k < 3; k++) M[k] = 0.5 * (P1[k] + P2[k]);	/* Mid-point M */
					gmt_normalize3v (GMT, M);
					gmt_cart_to_geo (GMT, &mlat, &mlon, M, true);	/* Get lon/lat of the mid point */
					gmt_cross3v (GMT, P1, P2, G);	/* This is pole of great circle through P1 & P2 */
					gmt_normalize3v (GMT, G);
					if (d == POLESPOTTER_FZ) {	/* Must get the bisector pole instead */
						gmt_cross3v (GMT, M, G, B);
						gmt_normalize3v (GMT, B);
						gmt_M_memcpy (G, B, 3, double);	/* Put bisector pole into G so code below is the same for AH and FZ */
					}
					gmt_cart_to_geo (GMT, &glat, &glon, G, true);	/* Get lon/lat of the mid point */
					gmtlib_init_rot_matrix (Rot0, G);	/* Get partial rotation matrix since no actual angle is applied yet */
					sprintf (header, "Great circle: Center = %g/%g and pole is %g/%g\n", mlon, mlat, glon, glat);
					if (Ctrl->L.active) GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, header);
					for (k = 0; k < n_steps; k++) {
						angle_radians = k * d_angle_radians;
						gmt_M_memcpy (Rot, Rot0, 9, double);			/* Get a copy of the "0-angle" rotation matrix */
						gmtlib_load_rot_matrix (angle_radians, Rot, G);		/* Build the actual rotation matrix for this angle */
						gmt_matrix_vect_mult (GMT, 3U, Rot, M, X);		/* Rotate the mid point along the great circle */
						gmt_cart_to_geo (GMT, &in[GMT_Y], &in[GMT_X], X, true);		/* Get lon/lat of this point along crossing profile */
						in[GMT_Y] = gmt_lat_swap (GMT, in[GMT_Y], GMT_LATSWAP_G2O + 1);	/* Convert to geodetic */
						if (Ctrl->L.active) GMT_Put_Record (API, GMT_WRITE_DATA, Out);
						if (!Ctrl->G.active) continue;
						if (gmt_M_y_is_outside (GMT, in[GMT_Y], Grid->header->wesn[YLO], Grid->header->wesn[YHI])) continue;		/* Outside y-range */
						if (gmt_x_is_outside (GMT, &in[GMT_X], Grid->header->wesn[XLO], Grid->header->wesn[XHI])) continue;		/* Outside x-range (or periodic longitude) */
						if (gmt_row_col_out_of_bounds (GMT, in, Grid->header, &grow, &gcol)) continue;	/* Sorry, outside after all */
						node = gmt_M_ijp (Grid->header, grow, gcol);			/* Bin node */
						layer[node] = cosd (in[GMT_Y]) * weight;			/* Any bin intersected will have this single value */
					}
					if (Ctrl->G.active) {	/* Add density of this great circle to the total */
						for (node = 0; node < Grid->header->size; node++) Grid->data[node] += layer[node];
					}
					gmt_M_memcpy (P1, P2, 3, double);	/* Let old P2 be next P1 */
				}
			}
		}
	}

	if (Ctrl->L.active) {	/* Disables further line output */
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further line output */
			Return (API->error);
		}
		gmt_M_free (GMT, Out);
	}

	if (Ctrl->G.active) {	/* Write the spotting grid */
		gmt_M_free_aligned (GMT, layer);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	
	Return (GMT_NOERROR);
}
