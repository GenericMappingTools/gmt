/*--------------------------------------------------------------------
 *	$Id$
 *
 *   Copyright (c) 1999-2014 by P. Wessel
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
 * grdrotater will read a grid file, apply a finite rotation to the grid
 * coordinates, and then interpolate the old grid at the new coordinates.
 *
 * Author:	Paul Wessel
 * Date:	27-JAN-2006
 * Ver:		1
 */

#define THIS_MODULE_NAME	"grdrotater"
#define THIS_MODULE_LIB		"spotter"
#define THIS_MODULE_PURPOSE	"Finite rotation reconstruction of geographic grid"

#include "spotter.h"

#define GMT_PROG_OPTIONS "-:>RVbdfghino" GMT_OPT("HMmQ")

#define PAD 3	/* Used to polish up a rotated grid by checking near neighbor nodes */

struct GRDROTATER_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct In {
		bool active;
		char *file;
	} In;
	struct D {	/* -Drotpolfile */
		bool active;
		char *file;
	} D;
	struct E {	/* -E[+]rotfile or -E<lon/lat/angle> */
		bool active;
		bool single;
		bool mode;
		char *file;
		double lon, lat, w;
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
	} N;
	struct S {	/* -S */
		bool active;
	} S;
	struct T {	/* -T */
		bool active;
		double value;
	} T;
};

void *New_grdrotater_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDROTATER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDROTATER_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	return (C);
}

void Free_grdrotater_Ctrl (struct GMT_CTRL *GMT, struct GRDROTATER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->D.file) free (C->D.file);	
	if (C->E.file) free (C->E.file);	
	if (C->F.file) free (C->F.file);	
	if (C->G.file) free (C->G.file);	
	GMT_free (GMT, C);	
}

int GMT_grdrotater_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdrotater <grid> -E[+]<rottable>|<plon>/<plat>/<prot> -G<outgrid> [-F<polygontable>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-D<rotoutline>] [-N] [%s] [-S] [-T<time>] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] > projpol\n\n", GMT_b_OPT, GMT_d_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_n_OPT, GMT_o_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<grid> is a gridded data file in geographic coordinates to be rotated.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set output filename of the new, rotated grid.  The boundary of the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   original grid (or a subset; see -F) after rotation is written to stdout\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   unless the grid is global.\n");
	spotter_rot_usage (API, 'E');
	GMT_Message (API, GMT_TIME_NONE, "\t   This option requires you to specify the age of the reconstruction with -T.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, specify a single finite rotation (in degrees) to be applied.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Write the rotated polygon or grid outline to <rotoutline> [stdout].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify a multi-segment closed polygon table that describes the area of the grid\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   that should be projected [Default projects entire grid].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do NOT output the rotated polygon or grid outline.\n");
	GMT_Option (API, "Rg");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Do NOT rotate the grid - just produce the rotated outline (requires -D).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set the time of reconstruction, if -E is used.\n");
	GMT_Option (API, "V,bi2,bo,d,g,h,i,n,:,.");
	
	return (EXIT_FAILURE);

}

int GMT_grdrotater_parse (struct GMT_CTRL *GMT, struct GRDROTATER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdrotater and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n_files = 0;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Supplemental parameters */
			
			case 'C':	/* Now done automatically in spotter_init */
				if (GMT_compat_check (GMT, 4))
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -C is no longer needed as total reconstruction vs stage rotation is detected automatically.\n");
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.file = strdup (opt->arg);
				break;
			case 'e':
				GMT_Report (API, GMT_MSG_COMPAT, "Warning: -e is deprecated and will be removed in 5.2.x. Use -E instead.\n");
				/* Fall-through on purpose */
			case 'E':	/* File with stage poles or a single rotation pole */
				Ctrl->E.active = true;
				k = (opt->arg[0] == '+') ? 1 : 0;
				if (!GMT_access (GMT, &opt->arg[k], F_OK) && GMT_check_filearg (GMT, 'E', &opt->arg[k], GMT_IN, GMT_IS_DATASET)) {	/* Was given a file (with possible leading + flag) */
					Ctrl->E.file  = strdup (&opt->arg[k]);
					if (k == 1) Ctrl->E.mode = true;
				}
				else {	/* Apply a fixed total reconstruction rotation to all input points  */
					unsigned int ns = 0;
					size_t kk;
					for (kk = 0; kk < strlen (opt->arg); kk++) if (opt->arg[kk] == '/') ns++;
					if (ns == 2) {	/* Looks like we got lon/lat/omega */
						Ctrl->E.single  = true;
						sscanf (opt->arg, "%[^/]/%[^/]/%lg", txt_a, txt_b, &Ctrl->E.w);
						n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->E.lon), txt_a);
						n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->E.lat), txt_b);
					}
					else	/* Junk of some sort */
						n_errors++;
				}
				break;
			case 'F':
				if ((Ctrl->F.active = GMT_check_filearg (GMT, 'F', opt->arg, GMT_IN, GMT_IS_DATASET)))
					Ctrl->F.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'G':
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'N':
				Ctrl->N.active = true;
				break;
			case 'S':
				Ctrl->S.active = true;
				break;
			case 'T':	/* New: -Tage; compat mode: -Tlon/lat/angle Finite rotation parameters */
				k = sscanf (opt->arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
				if (k == 3) {	/* Gave -Tlon/lat/angle */
					if (GMT_compat_check (GMT, 4)) {
						GMT_Report (API, GMT_MSG_COMPAT, "Warning: -T<lon>/<lat>/<angle> is deprecated; use -E<lon>/<lat>/<angle> instead.\n");
						Ctrl->E.single = Ctrl->E.active = true;
						Ctrl->E.w = atof (txt_c);
						n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->E.lon), txt_a);
						n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->E.lat), txt_b);
					}
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Must specify age of rotation\n");
						n_errors++;
					}
				}
				else {			
					Ctrl->T.active = true;
					Ctrl->T.value = atof (txt_a);
				}
				break;
				
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

        if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->G.active, "Syntax error: No output grid file allowed with -S\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->N.active, "Syntax error: Cannot use -N with -S\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->S.active && !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->S.active && !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->N.active, "Syntax error: -N and -S cannot both be given\n");
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3, "Syntax error: Binary input data (-bi) must have at least 2 columns\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && Ctrl->N.active, "Syntax error: -N and -D cannot both be given\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->E.active, "Syntax error: Option -E is required\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->E.single && !Ctrl->T.active, "Syntax error: Option -E<rottable> requires -T\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

struct GMT_DATASET * get_grid_path (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h)
{
	/* Return a single polygon that encloses this geographic grid exactly.
	 * It is used in the case when no particular clip polygon has been given.
	 * Note that the path is the same for pixel or grid-registered grids.
	 */

	unsigned int np = 0, add, col, row;
	uint64_t dim[4] = {1, 1, 0, 2};
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	
	if ((D = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (NULL);	/* An empty table with one segment, two cols */

	S = D->table[0]->segment[0];	/* Short hand */
		
	/* Add south border w->e */
	if (h->wesn[YLO] == -90.0) {	/* If at the S pole we just add it twice for end longitudes */
		add = 2;
		S->coord[GMT_X] = GMT_memory (GMT, NULL, add, double);
		S->coord[GMT_Y] = GMT_memory (GMT, NULL, add, double);
		S->coord[GMT_X][0] = h->wesn[XLO];	S->coord[GMT_X][1] = h->wesn[XHI];
		S->coord[GMT_Y][0] = S->coord[GMT_Y][1] = h->wesn[YLO];
	}
	else {				/* Loop along south border from west to east */
		add = h->nx - !h->registration;
		S->coord[GMT_X] = GMT_memory (GMT, NULL, add, double);
		S->coord[GMT_Y] = GMT_memory (GMT, NULL, add, double);
		for (col = 0; col < add; col++) {
			S->coord[GMT_X][col] = GMT_col_to_x (GMT, col, h->wesn[XLO], h->wesn[XHI], h->inc[GMT_X], 0.0, h->nx);
			S->coord[GMT_Y][col] = h->wesn[YLO];
		}
	}
	np += add;
	/* Add east border s->n */
	add = h->ny - !h->registration;
	S->coord[GMT_X] = GMT_memory (GMT, S->coord[GMT_X], add + np, double);
	S->coord[GMT_Y] = GMT_memory (GMT, S->coord[GMT_Y], add + np, double);
	for (row = 0; row < add; row++) {	/* Loop along east border from south to north */
		S->coord[GMT_X][np+row] = h->wesn[XHI];
		S->coord[GMT_Y][np+row] = GMT_row_to_y (GMT, h->ny - 1 - row, h->wesn[YLO], h->wesn[YHI], h->inc[GMT_Y], 0.0, h->ny);
	}
	np += add;
	/* Add north border e->w */
	if (h->wesn[YHI] == 90.0) {	/* If at the N pole we just add it twice for end longitudes */
		add = 2;
		S->coord[GMT_X] = GMT_memory (GMT, S->coord[GMT_X], add + np, double);
		S->coord[GMT_Y] = GMT_memory (GMT, S->coord[GMT_Y], add + np, double);
		S->coord[GMT_X][np] = h->wesn[XHI];	S->coord[GMT_X][np+1] = h->wesn[XLO];
		S->coord[GMT_Y][np] = S->coord[GMT_Y][np+1] = h->wesn[YHI];
	}
	else {			/* Loop along north border from east to west */
		add = h->nx - !h->registration;
		S->coord[GMT_X] = GMT_memory (GMT, S->coord[GMT_X], add + np, double);
		S->coord[GMT_Y] = GMT_memory (GMT, S->coord[GMT_Y], add + np, double);
		for (col = 0; col < add; col++) {
			S->coord[GMT_X][np+col] = GMT_col_to_x (GMT, h->nx - 1 - col, h->wesn[XLO], h->wesn[XHI], h->inc[GMT_X], 0.0, h->nx);
			S->coord[GMT_Y][np+col] = h->wesn[YHI];
		}
	}
	np += add;
	/* Add west border n->s */
	add = h->ny - !h->registration;
	S->coord[GMT_X] = GMT_memory (GMT, S->coord[GMT_X], add + np + 1, double);
	S->coord[GMT_Y] = GMT_memory (GMT, S->coord[GMT_Y], add + np + 1, double);
	for (row = 0; row < add; row++) {	/* Loop along west border from north to south */
		S->coord[GMT_X][np+row] = h->wesn[XLO];
		S->coord[GMT_Y][np+row] = GMT_row_to_y (GMT, row, h->wesn[YLO], h->wesn[YHI], h->inc[GMT_Y], 0.0, h->ny);
	}
	np += add;
	S->coord[GMT_X][np] = S->coord[GMT_X][0];	/* Close polygon explicitly */
	S->coord[GMT_Y][np] = S->coord[GMT_Y][0];
	np++;
	S->n_rows = np;
	S->n_columns = 2;
	S->min[GMT_X] = h->wesn[XLO];	S->max[GMT_X] = h->wesn[XHI];
	S->min[GMT_Y] = h->wesn[YLO];	S->max[GMT_Y] = h->wesn[YHI];
	S->pole = 0;
	
	return (D);
}

bool skip_if_outside (struct GMT_CTRL *GMT, struct GMT_DATATABLE *P, double lon, double lat)
{	/* Returns true if the selected point is outside the polygon */
	uint64_t seg;
	unsigned int inside = 0;
	for (seg = 0; seg < P->n_segments && !inside; seg++) {	/* Use degrees since function expects it */
		if (GMT_polygon_is_hole (P->segment[seg])) continue;	/* Holes are handled within GMT_inonout */
		inside = (GMT_inonout (GMT, lon, lat, P->segment[seg]) > 0);
	}
	return ((inside) ? false : true);	/* true if outside */
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdrotater_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdrotater (void *V_API, int mode, void *args)
{
	int scol, srow, error = 0;	/* Signed row, col */
	bool not_global, global = false;
	unsigned int col, row, col_o, row_o, start_row, stop_row, start_col, stop_col;
	
	uint64_t ij, ij_rot, seg, rec;

	double xx, yy, lon, P_original[3], P_rotated[3], R[3][3];
	double *grd_x = NULL, *grd_y = NULL, *grd_yc = NULL;

	struct GMT_DATASET *D = NULL;
	struct GMT_DATATABLE *pol = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_OPTION *ptr = NULL;
	struct GMT_GRID *G = NULL, *G_rot = NULL;
	struct GRDROTATER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdrotater_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdrotater_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdrotater_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if ((ptr = GMT_Find_Option (API, 'f', options)) == NULL) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdrotater_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdrotater_parse (GMT, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the grdrotater main code ----------------------------*/

	GMT_set_pad (GMT, 2U);	/* Ensure space for BCs in case an API passed pad == 0 */

	/* Check limits and get data file */

	if (Ctrl->In.file) {
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}

		if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, G->header->wesn, 4, double);	/* -R was not set so we use the grid domain */

		/* Determine the wesn to be used to read the Ctrl->In.file; or exit if file is outside -R */

		if (!GMT_grd_setregion (GMT, G->header, GMT->common.R.wesn, BCR_BILINEAR)) {
			GMT_Report (API, GMT_MSG_NORMAL, "No grid values inside selected region - aborting\n");
			Return (EXIT_FAILURE);
		}
		global = (doubleAlmostEqual (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO], 360.0)
							&& doubleAlmostEqual (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO], 180.0));
		if (!GMT->common.R.active) global = GMT_grd_is_global (GMT, G->header);
	}
	not_global = !global;
	
	if (!Ctrl->S.active) {	/* Read the input grid */
		GMT_Report (API, GMT_MSG_VERBOSE, "Allocates memory and read grid file\n");
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, GMT->common.R.wesn, Ctrl->In.file, G) == NULL) {
			Return (API->error);
		}
	}
	
	if (Ctrl->F.active) {	/* Read the user's polygon file */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_READ_NORMAL, NULL, Ctrl->F.file, NULL)) == NULL) {
			Return (API->error);
		}
		pol = D->table[0];	/* Since it is a single file */
	}
	else if (not_global) {	/* Make a single grid-outline polygon */
		if ((D = get_grid_path (GMT, G->header)) == NULL) Return (API->error);
		pol = D->table[0];	/* Since it is a single file */
	}

	if (Ctrl->E.single) {	/* Get rotation matrix R */
		GMT_make_rot_matrix (GMT, Ctrl->E.lon, Ctrl->E.lat, Ctrl->E.w, R);	/* Make rotation matrix from rotation parameters */
		GMT_Report (API, GMT_MSG_VERBOSE, "Using rotation (%g, %g, %g)\n", Ctrl->E.lon, Ctrl->E.lat, Ctrl->E.w);
	}
	else {
		int n_stages;
		struct EULER *p = NULL;			/* Pointer to array of stage poles */
		double lon, lat, w, t_max;
		
		n_stages = spotter_init (GMT, Ctrl->E.file, &p, false, true, Ctrl->E.mode, &t_max);
		if (Ctrl->T.value > t_max) {
			GMT_Report (API, GMT_MSG_NORMAL, "Requested a reconstruction time outside range of rotation table\n");
			GMT_free (GMT, p);
			Return (EXIT_FAILURE);
		}
		spotter_get_rotation (GMT, p, n_stages, Ctrl->T.value, &lon, &lat, &w);
		GMT_make_rot_matrix (GMT, lon, lat, w, R);	/* Make rotation matrix from rotation parameters */
		GMT_Report (API, GMT_MSG_VERBOSE, "Using rotation (%g, %g, %g)\n", lon, lat, w);
		GMT_free (GMT, p);
	}
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Reconstruct polygon outline\n");
	
	/* First reconstruct the polygon outline */
	
	for (seg = 0; pol && seg < pol->n_segments; seg++) {
		S = pol->segment[seg];	/* Shorthand for current segment */
		for (rec = 0; rec < pol->segment[seg]->n_rows; rec++) {
			S->coord[GMT_Y][rec] = GMT_lat_swap (GMT, S->coord[GMT_Y][rec], GMT_LATSWAP_G2O);	/* Convert to geocentric */
			GMT_geo_to_cart (GMT, S->coord[GMT_Y][rec], S->coord[GMT_X][rec], P_original, true);	/* Convert to a Cartesian x,y,z vector; true since we have degrees */
			GMT_matrix_vect_mult (GMT, 3U, R, P_original, P_rotated);				/* Rotate the vector */
			GMT_cart_to_geo (GMT, &S->coord[GMT_Y][rec], &S->coord[GMT_X][rec], P_rotated, true);	/* Recover lon lat representation; true to get degrees */
			S->coord[GMT_Y][rec] = GMT_lat_swap (GMT, S->coord[GMT_Y][rec], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
		}
		GMT_set_seg_polar (GMT, S);	/* Determine if it is a polar cap */
	}
	GMT_set_tbl_minmax (GMT, pol);	/* Update table domain */
	if (!Ctrl->N.active && not_global) {
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_WRITE_SET, NULL, Ctrl->D.file, D) != GMT_OK) {
			Return (API->error);
		}
	}
	if (Ctrl->S.active) {
		if (Ctrl->F.active && GMT_Destroy_Data (API, &D) != GMT_OK) {
			Return (API->error);
		}
		else if (not_global)
			GMT_free_dataset (GMT, &D);
	
		GMT_Report (API, GMT_MSG_VERBOSE, "Done!\n");
		Return (GMT_OK);
	}
	
	/* Then, find min/max of reconstructed outline */
	
	if (global)
		GMT_memcpy (GMT->common.R.wesn, G->header->wesn, 4, double);
	else {
		GMT->common.R.wesn[XLO] = floor (pol->min[GMT_X] * G->header->r_inc[GMT_X]) * G->header->inc[GMT_X];
		GMT->common.R.wesn[XHI] = ceil  (pol->max[GMT_X] * G->header->r_inc[GMT_X]) * G->header->inc[GMT_X];
		GMT->common.R.wesn[YLO] = floor (pol->min[GMT_Y] * G->header->r_inc[GMT_Y]) * G->header->inc[GMT_Y];
		GMT->common.R.wesn[YHI] = ceil  (pol->max[GMT_Y] * G->header->r_inc[GMT_Y]) * G->header->inc[GMT_Y];
		/* Adjust longitude range, as indicated by FORMAT_GEO_OUT */
		GMT_lon_range_adjust (GMT->current.io.geo.range, &GMT->common.R.wesn[XLO]);
		GMT_lon_range_adjust (GMT->current.io.geo.range, &GMT->common.R.wesn[XHI]);
		if (GMT->common.R.wesn[XLO] >= GMT->common.R.wesn[XHI]) GMT->common.R.wesn[XHI] += 360.0;
	}
	GMT->common.R.active = true;
	
	if ((G_rot = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, NULL, G->header->inc, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, Ctrl->G.file)) == NULL) Return (API->error);

	/* Precalculate node coordinates in both degrees and radians */
	grd_x = GMT_grd_coord (GMT, G_rot->header, GMT_X);
	grd_y = GMT_grd_coord (GMT, G_rot->header, GMT_Y);
	grd_yc = GMT_memory (GMT, NULL, G_rot->header->ny, double);
	for (row = 0; row < G_rot->header->ny; row++) grd_yc[row] = GMT_lat_swap (GMT, grd_y[row], GMT_LATSWAP_G2O);

	/* Loop over all nodes in the new rotated grid and find those inside the reconstructed polygon */
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Interpolate reconstructed grid\n");

	GMT_make_rot_matrix (GMT, Ctrl->E.lon, Ctrl->E.lat, -Ctrl->E.w, R);	/* Make inverse rotation using negative angle */
	
	GMT_grd_loop (GMT, G_rot, row, col, ij_rot) {
		G_rot->data[ij_rot] = GMT->session.f_NaN;
		if (not_global && skip_if_outside (GMT, pol, grd_x[col], grd_y[row])) continue;	/* Outside polygon */
		
		/* Here we are inside; get the coordinates and rotate back to original grid coordinates */
		
		GMT_geo_to_cart (GMT, grd_yc[row], grd_x[col], P_rotated, true);	/* Convert degree lon,lat to a Cartesian x,y,z vector */
		GMT_matrix_vect_mult (GMT, 3U, R, P_rotated, P_original);	/* Rotate the vector */
		GMT_cart_to_geo (GMT, &yy, &xx, P_original, true);		/* Recover degree lon lat representation */
		yy = GMT_lat_swap (GMT, yy, GMT_LATSWAP_O2G);			/* Convert back to geodetic */
		xx -= 360.0;
		while (xx < G->header->wesn[XLO]) xx += 360.0;	/* Make sure we deal with 360 issues */
		G_rot->data[ij_rot] = (float)GMT_get_bcr_z (GMT, G, xx, yy);
	}	
	
	/* Also loop over original node locations to make sure the nearest nodes are set */

	for (seg = 0; not_global && seg < pol->n_segments; seg++) {
		for (rec = 0; rec < pol->segment[seg]->n_rows; rec++) {
			lon = pol->segment[seg]->coord[GMT_X][rec];
			while (lon < G_rot->header->wesn[XLO]) lon += 360.0;
			scol = (int)GMT_grd_x_to_col (GMT, lon, G_rot->header);
			srow = (int)GMT_grd_y_to_row (GMT, pol->segment[seg]->coord[GMT_Y][rec], G_rot->header);
			/* Visit the PAD * PAD number of cells centered on col, row and make sure they have been set */
			start_row = (srow > PAD) ? srow - PAD : 0;
			stop_row  = srow + PAD;
			start_col = (scol > PAD) ? scol - PAD : 0;
			stop_col  = scol + PAD;
			for (row = start_row; row <= stop_row; row++) {
				if (row >= G_rot->header->ny) continue;
				for (col = start_col; col <= stop_col; col++) {
					if (col >= G_rot->header->nx) continue;
					ij_rot = GMT_IJP (G_rot->header, row, col);
					if (!GMT_is_fnan (G_rot->data[ij_rot])) continue;	/* Already done this */
					if (not_global && skip_if_outside (GMT, pol, grd_x[col], grd_yc[row])) continue;	/* Outside polygon */
					GMT_geo_to_cart (GMT, grd_yc[row], grd_x[col], P_rotated, true);	/* Convert degree lon,lat to a Cartesian x,y,z vector */
					GMT_matrix_vect_mult (GMT, 3U, R, P_rotated, P_original);	/* Rotate the vector */
					GMT_cart_to_geo (GMT, &xx, &yy, P_original, true);	/* Recover degree lon lat representation */
					yy = GMT_lat_swap (GMT, yy, GMT_LATSWAP_O2G);		/* Convert back to geodetic */
					scol = (int)GMT_grd_x_to_col (GMT, xx, G->header);
					if (scol < 0) continue;
					col_o = scol;	if (col_o >= G->header->nx) continue;
					srow = (int)GMT_grd_y_to_row (GMT, yy, G->header);
					if (srow < 0) continue;
					row_o = srow;	if (row_o >= G->header->ny) continue;
					ij = GMT_IJP (G->header, row_o, col_o);
					G_rot->data[ij_rot] = G->data[ij];
				}
			}
		}
	}

	/* Now write rotated grid */
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Write reconstructed grid\n");

	GMT_set_pad (GMT, API->pad);	/* Reset to session default pad before output */

	sprintf (G_rot->header->remark, "Grid rotated using R[lon lat omega] = %g %g %g", Ctrl->E.lon, Ctrl->E.lat, Ctrl->E.w);
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G_rot)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, G_rot) != GMT_OK) {
		Return (API->error);
	}

	GMT_free (GMT, grd_x);
	GMT_free (GMT, grd_y);
	GMT_free (GMT, grd_yc);
	
	if (GMT_Destroy_Data (API, &D) != GMT_OK)
		Return (API->error);

	GMT_Report (API, GMT_MSG_VERBOSE, "Done!\n");

	Return (GMT_OK);
}
