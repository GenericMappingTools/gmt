/*--------------------------------------------------------------------
 *	$Id: grdrotater_func.c,v 1.3 2011-03-25 22:17:42 guru Exp $
 *
 *   Copyright (c) 1999-2011 by P. Wessel
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
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

#include "spotter.h"
#include "gmt_proj.h"

#define PAD 3	/* Used to polish up a rotated grid by checking near neighbor nodes */

struct GRDROTATER_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct D {	/* -Drotpolfile */
		GMT_LONG active;
		char *file;
	} D;
	struct E {	/* -E[+]rotfile */
		GMT_LONG active;
		GMT_LONG mode;
		char *file;
	} E;
	struct e {	/* -e<lon/lat/angle> */
		GMT_LONG active;
		double lon, lat, w;
	} e;
	struct F {	/* -Fpolfile */
		GMT_LONG active;
		char *file;
	} F;
	struct G {	/* -Goutfile */
		GMT_LONG active;
		char *file;
	} G;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct Q {	/* -Q[b|c|l|n][[/]<threshold>] */
		GMT_LONG active;
		GMT_LONG interpolant;
		double threshold;
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
		double value;
	} T;
};

void *New_grdrotater_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDROTATER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDROTATER_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->Q.interpolant = BCR_BICUBIC; C->Q.threshold = 1.0;
	
	return ((void *)C);
}

void Free_grdrotater_Ctrl (struct GMT_CTRL *GMT, struct GRDROTATER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->D.file) free ((void *)C->D.file);	
	if (C->E.file) free ((void *)C->E.file);	
	if (C->F.file) free ((void *)C->F.file);	
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}


GMT_LONG GMT_grdrotater_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT,"grdrotater %s - Finite rotation reconstruction of geographic grid\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdrotater <grdfile> -E[+]<euler.d> OR -eplon/plat/prot -G<rotgrid> [-F<polygonfile>]\n");
	GMT_message (GMT, "\t[-D<rotoutline>] [-N] [-Q[b|c|l|n][[/]<threshold>]] [%s] [-S] [-T<time>] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_b_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s] > projpol\n\n", GMT_g_OPT, GMT_h_OPT, GMT_i_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<grdfile> is a gridded data file in geographic coordinates to be rotated\n");
	GMT_message (GMT, "\t-G is the output filename of the new, rotated grid.  The boundary of the\n");
	GMT_message (GMT, "\t   original grid (or a subset; see -F) after rotation is written to stdout\n");
	GMT_message (GMT, "\t   unless the grid is global.\n");
	GMT_message (GMT, "\t-E specifies the rotations to be used (see man page for format)\n\n");
	GMT_message (GMT, "\t   Prepend + if you want to invert the finite rotations prior to used\n\n");
	GMT_message (GMT, "\t   This option requires you to specify the age of the reconstruction with -T\n\n");
	GMT_message (GMT, "\t-e Alternatively, specify a single finite rotation (in degrees) to be applied to all input points\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-D Destination file for rotated polygon or grid outline [stdout]\n");
	GMT_message (GMT, "\t-F specifies a multi-segment closed polygon file that describes the area of the grid\n");
	GMT_message (GMT, "\t   that should be projected [Default projects entire grid]\n");
	GMT_message (GMT, "\t-N Do not output the rotated polygon or grid outline\n");
	GMT_message (GMT, "\t-Q Quick mode, use bilinear rather than bicubic [Default] interpolation.\n");
	GMT_message (GMT, "\t   Alternatively, select interpolation mode by adding b = B-spline, c = bicubic,\n");
	GMT_message (GMT, "\t   l = bilinear, or n = nearest-neighbor.\n");
	GMT_message (GMT, "\t   Optionally, append <threshold> in the range [0,1]. [Default = 1 requires all\n");
	GMT_message (GMT, "\t   4 or 16 nodes to be non-NaN.], <threshold> = 0.5 will interpolate about 1/2 way\n");
	GMT_message (GMT, "\t   from a non-NaN to a NaN node, while 0.1 will go about 90%% of the way, etc.\n");
	GMT_message (GMT, "\t   -Q0 will return the value of the nearest node instead of interpolating (Same as -Qn).\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-S Do NOT rotate the grid - just produce the rotated outline\n");
	GMT_message (GMT, "\t-T sets the time of reconstruction, if -E is used.\n");
	GMT_explain_options (GMT, "VC2D0ghi:.");
	
	return (EXIT_FAILURE);

}

GMT_LONG GMT_grdrotater_parse (struct GMTAPI_CTRL *C, struct GRDROTATER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdrotater and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j, n_errors = 0, n, n_files = 0;
	char txt_a[GMT_LONG_TEXT], txt_b[GMT_LONG_TEXT], txt_c[GMT_LONG_TEXT];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Supplemental parameters */
			
#ifdef GMT_COMPAT
			case 'C':	/* Now done automatically in spotter_init */
				break;
#endif
			case 'D':
				Ctrl->D.active = TRUE;
				Ctrl->D.file = strdup (opt->arg);
				break;
			case 'E':	/* File with stage poles */
				Ctrl->E.active = TRUE;	n = 0;
				if (opt->arg[0] == '+') { Ctrl->E.mode = TRUE; n = 1;}
				Ctrl->E.file  = strdup (&opt->arg[n]);
				break;
			case 'T':	/* New: -Tage; compat mode: -Tlon/lat/angle Finite rotation parameters */
				n = sscanf (opt->arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
#ifdef GMT_COMPAT
				if (n == 3) {	/* Gave -Tlon/lat/angle */
					Ctrl->e.active  = TRUE;
					Ctrl->e.w = atof (txt_c);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->e.lon), txt_a);
					n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->e.lat), txt_b);
				}
				else {			
#endif
					Ctrl->T.active = TRUE;
					Ctrl->T.value = atof (txt_a);
#ifdef GMT_COMPAT
				}
#endif
				break;
			case 'e':
				Ctrl->e.active  = TRUE;
				sscanf (opt->arg, "%[^/]/%[^/]/%lg", txt_a, txt_b, &Ctrl->e.w);
				n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->e.lon), txt_a);
				n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->e.lat), txt_b);
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				Ctrl->F.file = strdup (opt->arg);
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				break;
			case 'Q':
				Ctrl->Q.active = TRUE;
				Ctrl->Q.interpolant = BCR_BILINEAR;
				for (j = 0; j < 3 && opt->arg[j]; j++) {
					switch (opt->arg[j]) {
						case 'n':
							Ctrl->Q.interpolant = BCR_NEARNEIGHBOR; break;
						case 'l':
							Ctrl->Q.interpolant = BCR_BILINEAR; break;
						case 'b':
							Ctrl->Q.interpolant = BCR_BSPLINE; break;
						case 'c':
							Ctrl->Q.interpolant = BCR_BICUBIC; break;
						case '/':
						default:
							Ctrl->Q.threshold = atof (&opt->arg[j]);
							if (j == 0 && Ctrl->Q.threshold < GMT_SMALL) {
								Ctrl->Q.interpolant = BCR_NEARNEIGHBOR;
								GMT_report (GMT, GMT_MSG_FATAL, "Warning: Option -Q0 deprecated. Use -Qn instead.\n");
							}
							j = 3; break;
					}
				}
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				break;
				
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

        if (GMT_native_binary (GMT, GMT_IN) && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->current.io.io_header[GMT_IN], "GMT SYNTAX ERROR:  Binary input data cannot have header -H\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->G.active, "GMT SYNTAX ERROR:  No output grid file allowed with -S\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->S.active && !Ctrl->In.file, "GMT SYNTAX ERROR:  Must specify input file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->S.active && !Ctrl->G.file, "GMT SYNTAX ERROR -G:  Must specify output file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->N.active, "GMT SYNTAX ERROR:  -N and -S cannot both be given\n");
	n_errors += GMT_check_condition (GMT, GMT_native_binary (GMT, GMT_IN) && GMT->common.b.ncol[GMT_IN] < 3, "GMT SYNTAX ERROR:  Binary input data (-bi) must have at least 2 columns\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && Ctrl->N.active, "GMT SYNTAX ERROR:  -N and -D cannot both be given\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && Ctrl->e.active, "GMT SYNTAX ERROR:  -E and -e cannot both be given\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->E.active && !Ctrl->e.active, "GMT SYNTAX ERROR:  Must specify either -E -T or -e\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && !Ctrl->T.active, "GMT SYNTAX ERROR:  Option -E requires -T\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

void get_grid_path (struct GMT_CTRL *GMT, struct GRD_HEADER *h, struct GMT_DATASET **Dout)
{
	/* Return a single polygon that encloses this geographic grid exactly.
	 * It is used in the case when no particular clip polygon has been given.
	 * Note that the path is the same for pixel or grid-registered grids.
	 */

	GMT_LONG np = 0, add, i, j;
	struct GMT_DATASET *D = NULL;
	struct GMT_TABLE *pol = NULL;
	
	D = GMT_create_dataset (GMT, 1, 1, 2, 0);	/* An empty table with one segment, two cols */

	pol = D->table[0];
	pol->segment = GMT_memory (GMT, NULL, 1, struct GMT_LINE_SEGMENT *);
	pol->segment[0] = GMT_memory (GMT, NULL, 1, struct GMT_LINE_SEGMENT);
	pol->segment[0]->coord = GMT_memory (GMT, NULL, 2, double *);
	pol->segment[0]->min = GMT_memory (GMT, NULL, 2, double);
	pol->segment[0]->max = GMT_memory (GMT, NULL, 2, double);
		
	/* Add south border w->e */
	if (h->wesn[YLO] == -90.0) {	/* If at the S pole we just add it twice for end longitudes */
		add = 2;
		pol->segment[0]->coord[GMT_X] = GMT_memory (GMT, NULL, add, double);
		pol->segment[0]->coord[GMT_Y] = GMT_memory (GMT, NULL, add, double);
		pol->segment[0]->coord[GMT_X][0] = h->wesn[XLO];	pol->segment[0]->coord[GMT_X][1] = h->wesn[XHI];
		pol->segment[0]->coord[GMT_Y][0] = pol->segment[0]->coord[GMT_Y][1] = h->wesn[YLO];
	}
	else {				/* Loop along south border from west to east */
		add = h->nx - !h->registration;
		pol->segment[0]->coord[GMT_X] = GMT_memory (GMT, NULL, add, double);
		pol->segment[0]->coord[GMT_Y] = GMT_memory (GMT, NULL, add, double);
		for (i = 0; i < add; i++) {
			pol->segment[0]->coord[GMT_X][i] = GMT_col_to_x (i, h->wesn[XLO], h->wesn[XHI], h->inc[GMT_X], 0.0, h->nx);
			pol->segment[0]->coord[GMT_Y][i] = h->wesn[YLO];
		}
	}
	np += add;
	/* Add east border s->n */
	add = h->ny - !h->registration;
	pol->segment[0]->coord[GMT_X] = GMT_memory (GMT, pol->segment[0]->coord[GMT_X], add + np, double);
	pol->segment[0]->coord[GMT_Y] = GMT_memory (GMT, pol->segment[0]->coord[GMT_Y], add + np, double);
	for (j = 0; j < add; j++) {	/* Loop along east border from south to north */
		pol->segment[0]->coord[GMT_X][np+j] = h->wesn[XHI];
		pol->segment[0]->coord[GMT_Y][np+j] = GMT_row_to_y (h->ny - 1 - j, h->wesn[YLO], h->wesn[YHI], h->inc[GMT_Y], 0.0, h->ny);
	}
	np += add;
	/* Add north border e->w */
	if (h->wesn[YHI] == 90.0) {	/* If at the N pole we just add it twice for end longitudes */
		add = 2;
		pol->segment[0]->coord[GMT_X] = GMT_memory (GMT, pol->segment[0]->coord[GMT_X], add + np, double);
		pol->segment[0]->coord[GMT_Y] = GMT_memory (GMT, pol->segment[0]->coord[GMT_Y], add + np, double);
		pol->segment[0]->coord[GMT_X][np] = h->wesn[XHI];	pol->segment[0]->coord[GMT_X][np+1] = h->wesn[XLO];
		pol->segment[0]->coord[GMT_Y][np] = pol->segment[0]->coord[GMT_Y][np+1] = h->wesn[YHI];
	}
	else {			/* Loop along north border from east to west */
		add = h->nx - !h->registration;
		pol->segment[0]->coord[GMT_X] = GMT_memory (GMT, pol->segment[0]->coord[GMT_X], add + np, double);
		pol->segment[0]->coord[GMT_Y] = GMT_memory (GMT, pol->segment[0]->coord[GMT_Y], add + np, double);
		for (i = 0; i < add; i++) {
			pol->segment[0]->coord[GMT_X][np+i] = GMT_col_to_x (h->nx - 1 - i, h->wesn[XLO], h->wesn[XHI], h->inc[GMT_X], 0.0, h->nx);
			pol->segment[0]->coord[GMT_Y][np+i] = h->wesn[YHI];
		}
	}
	np += add;
	/* Add west border n->s */
	add = h->ny - !h->registration;
	pol->segment[0]->coord[GMT_X] = GMT_memory (GMT, pol->segment[0]->coord[GMT_X], add + np + 1, double);
	pol->segment[0]->coord[GMT_Y] = GMT_memory (GMT, pol->segment[0]->coord[GMT_Y], add + np + 1, double);
	for (j = 0; j < add; j++) {	/* Loop along west border from north to south */
		pol->segment[0]->coord[GMT_X][np+j] = h->wesn[XLO];
		pol->segment[0]->coord[GMT_Y][np+j] = GMT_row_to_y (j, h->wesn[YLO], h->wesn[YHI], h->inc[GMT_Y], 0.0, h->ny);
	}
	np += add;
	pol->segment[0]->coord[GMT_X][np] = pol->segment[0]->coord[GMT_X][0];	/* Close polygon explicitly */
	pol->segment[0]->coord[GMT_Y][np] = pol->segment[0]->coord[GMT_Y][0];
	np++;
	pol->segment[0]->n_rows = np;
	pol->segment[0]->min[GMT_X] = h->wesn[XLO];	pol->segment[0]->max[GMT_X] = h->wesn[XHI];
	pol->segment[0]->min[GMT_Y] = h->wesn[YLO];	pol->segment[0]->max[GMT_Y] = h->wesn[YHI];
	pol->segment[0]->pole = 0;
	pol->n_segments = 1;
	
	*Dout = D;
}

#define Return(code) {Free_grdrotater_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdrotater (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG ij, col, row, ij_rot, inside, seg, rec, not_global;
	GMT_LONG col2, row2, col_o, row_o, error = FALSE, global = FALSE;
	
	double xx, yy, lon, P_original[3], P_rotated[3], R[3][3];
	double w180, w360, e180, e360, *grd_x = NULL, *grd_y = NULL, *grd_yc = NULL;
	double wp180, wp360, ep180, ep360, spol, npol, west, east, south, north;

	struct GMT_DATASET *D = NULL;
	struct GMT_TABLE *pol = NULL;
	struct GMT_LINE_SEGMENT *S = NULL;
	struct GMT_EDGEINFO edgeinfo;
	struct GMT_BCR bcr;
	struct GMT_OPTION *ptr = NULL;
	struct GMT_GRID *G = NULL, *G_rot = NULL;
	struct GRDROTATER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdrotater_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdrotater_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdrotater", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRbf:", "ghios>" GMT_OPT("HMm"), options))) Return (error);
	if (GMT_Find_Option (API, 'f', options, &ptr)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	Ctrl = (struct GRDROTATER_CTRL *) New_grdrotater_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdrotater_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the grdrotater main code ----------------------------*/

	GMT_lat_swap_init (GMT);	/* Initialize auxiliary latitude machinery */

	/* Check limits and get data file */

	if ((error = GMT_Begin_IO (API, 0, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */

	if (Ctrl->In.file) {

		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file), (void **)&G)) Return (GMT_DATA_READ_ERROR);	/* Get header only */

		if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, G->header->wesn, 4, double);	/* -R was not set so we use the grid domain */

		/* Determine the wesn to be used to read the Ctrl->In.file; or exit if file is outside -R */

		if (GMT_grd_setregion (GMT, G->header, GMT->common.R.wesn, BCR_BILINEAR)) {
			GMT_report (GMT, GMT_MSG_FATAL, "No grid values inside selected region - aborting\n");
			Return (EXIT_FAILURE);
		}
		global = (GMT_IS_ZERO (GMT->common.R.wesn[XHI] - GMT->common.R.wesn[XLO] - 360.0) && GMT_IS_ZERO (GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO] - 180.0));
	}
	not_global = !global;
	
	if (!Ctrl->S.active) {
		/* Read the input grid */

		GMT_report (GMT, GMT_MSG_NORMAL, "Allocates memory and read grid file\n");
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT->common.R.wesn, GMT_GRID_DATA, (void **)&(Ctrl->In.file), (void **)&G)) Return (GMT_DATA_READ_ERROR);	/* Get header only */

		GMT_boundcond_init (GMT, &edgeinfo);
		if (GMT_360_RANGE (G->header->wesn[XHI], G->header->wesn[XLO])) GMT_boundcond_parse (GMT, &edgeinfo, "g");
	
		GMT_boundcond_param_prep (GMT, G, &edgeinfo);

		/* Initialize bcr structure:  */

		GMT_bcr_init (GMT, G, Ctrl->Q.interpolant, Ctrl->Q.threshold, &bcr);

		/* Set boundary conditions  */

		GMT_boundcond_set (GMT, G, &edgeinfo);
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);			/* Disables further data input */
	
	if (Ctrl->F.active) {	/* Read the user's polygon file */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);				/* Enables data input and sets access mode */
		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, NULL, 0, (void **)&Ctrl->F.file, (void **)&D)) Return ((error = GMT_DATA_READ_ERROR));
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */
	}
	else if (not_global) {	/* Make a single grid-outline polygon */
		get_grid_path (GMT, G->header, &D);
		pol = D->table[0];	/* Since it is a single file */
	}

	if (Ctrl->e.active) {	/* Get rotation matrix R */
		spotter_make_rot_matrix (GMT, Ctrl->e.lon, Ctrl->e.lat, Ctrl->e.w, R);	/* Make rotation matrix from rotation parameters */
		GMT_report (GMT, GMT_MSG_NORMAL, "Using rotation (%g, %g, %g)\n", Ctrl->e.lon, Ctrl->e.lat, Ctrl->e.w);
	}
	else {
		GMT_LONG n_stages;
		struct EULER *p = NULL;			/* Pointer to array of stage poles */
		double lon, lat, w, t_max;
		
		n_stages = spotter_init (GMT, Ctrl->E.file, &p, FALSE, TRUE, Ctrl->E.mode, &t_max);
		if (Ctrl->T.value > t_max) {
			GMT_report (GMT, GMT_MSG_FATAL, "Requested a reconstruction time outside range of rotation table\n");
			GMT_free (GMT, p);
			Return (EXIT_FAILURE);
		}
		spotter_get_rotation (GMT, p, n_stages, Ctrl->T.value, &lon, &lat, &w);
		spotter_make_rot_matrix (GMT, lon, lat, w, R);	/* Make rotation matrix from rotation parameters */
		GMT_report (GMT, GMT_MSG_NORMAL, "Using rotation (%g, %g, %g)\n", lon, lat, w);
		GMT_free (GMT, p);
	}
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Reconstruct polygon outline\n");
	
	/* First reconstruct the polygon outline */
	
	west = w360 = w180 = DBL_MAX;	east = e360 = e180 = -DBL_MAX;	south = DBL_MAX;	north = -DBL_MAX;
	for (seg = 0; not_global && seg < pol->n_segments; seg++) {
		if (!Ctrl->N.active) GMT_write_segmentheader (GMT, GMT->session.std[GMT_OUT], 2);
		wp360 = wp180 = DBL_MAX;	ep360 = ep180 = -DBL_MAX;	spol = DBL_MAX;	npol = -DBL_MAX;
		for (rec = 0; rec < pol->segment[seg]->n_rows; rec++) {
			S = pol->segment[seg];	/* Shorthand for current segment */
			S->coord[GMT_Y][rec] = GMT_lat_swap (GMT, S->coord[GMT_Y][rec], GMT_LATSWAP_G2O);	/* Convert to geocentric */
			GMT_geo_to_cart (GMT, S->coord[GMT_Y][rec], S->coord[GMT_X][rec], P_original, TRUE);	/* Convert to a Cartesian x,y,z vector; TRUE since we have degrees */
			spotter_matrix_vect_mult (GMT, R, P_original, P_rotated);				/* Rotate the vector */
			GMT_cart_to_geo (GMT, &S->coord[GMT_Y][rec], &S->coord[GMT_X][rec], P_rotated, TRUE);	/* Recover lon lat representation; TRUE to get degrees */
			S->coord[GMT_Y][rec] = GMT_lat_swap (GMT, S->coord[GMT_Y][rec], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
			if (S->coord[GMT_Y][rec] < south) south = S->coord[GMT_Y][rec];
			if (S->coord[GMT_Y][rec] > north) north = S->coord[GMT_Y][rec];
			if (S->coord[GMT_Y][rec] < spol) spol = S->coord[GMT_Y][rec];
			if (S->coord[GMT_Y][rec] > npol) npol = S->coord[GMT_Y][rec];
			lon = S->coord[GMT_X][rec];
			GMT_lon_range_adjust (GMT, 0, &lon);	/* 0 <= lon < 360 */
			if (lon < w360) w360 = lon;
			if (lon > e360) e360 = lon;
			if (lon < wp360) wp360 = lon;
			if (lon > ep360) ep360 = lon;
			GMT_lon_range_adjust (GMT, 2, &lon);	/* -180 <= lon < 180 */
			if (lon < w180) w180 = lon;
			if (lon > e180) e180 = lon;
			if (lon < wp180) wp180 = lon;
			if (lon > ep180) ep180 = lon;
		}
		S->pole = 0;
		if (fabs (ep360 - wp360) < 180.0) {
			S->min[GMT_X] = w360;
			S->max[GMT_X] = e360;
		}
		else if (fabs (ep180 - wp180) < 180.0) {
			S->min[GMT_X] = w180;
			S->max[GMT_X] = e180;
		}
		else {
			S->min[GMT_X] = 0.0;
			S->max[GMT_X] = 360.0;
			S->pole = irint (copysign (1.0, npol));
		}
		S->min[GMT_Y] = spol;	S->max[GMT_Y] = npol;
	}
	if (!Ctrl->N.active && not_global) {
		if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
		if (GMT_Put_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, NULL, 0, (void **)&Ctrl->D.file, (void *)D)) Return (GMT_DATA_WRITE_ERROR);
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */
		
	}
	if (Ctrl->S.active) {
		if (Ctrl->F.active)
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&D);
		else if (not_global)
			GMT_free_dataset (GMT, &D);
	
		GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");
		Return (GMT_OK);
	}
	
	/* Then, find min/max of reconstructed outline */
	
	GMT->common.R.wesn[YLO] = south;	GMT->common.R.wesn[YHI] = north;
	if (global)
		GMT_memcpy (GMT->common.R.wesn, G->header->wesn, 4, double);
	else if (fabs (e360 - w360) < 180.0) {
		GMT->common.R.wesn[XLO] = w360;
		GMT->common.R.wesn[XHI] = e360;
	}
	else if (fabs (e180 - w180) < 180.0) {
		GMT->common.R.wesn[XLO] = w180;
		GMT->common.R.wesn[XHI] = e180;
	}
	else if (G->header->wesn[XLO] < 0.0 && G->header->wesn[XHI] > 0.0) {
		GMT->common.R.wesn[XLO] = -180.0;
		GMT->common.R.wesn[XHI] = 180.0;
	}
	else {
		GMT->common.R.wesn[XLO] = 0.0;
		GMT->common.R.wesn[XHI] = 360.0;
	}
	
	G_rot = GMT_create_grid (GMT);
	GMT_grd_init (GMT, G_rot->header, options, FALSE);
	
	/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, G_rot, GMT->common.R.wesn, G->header->inc, G->header->registration), Ctrl->G.file);
	
	G_rot->data = GMT_memory (GMT, NULL, G_rot->header->size, float);
	grd_x = GMT_memory (GMT, NULL, G_rot->header->nx, double);
	grd_y = GMT_memory (GMT, NULL, G_rot->header->ny, double);
	grd_yc = GMT_memory (GMT, NULL, G_rot->header->ny, double);
	/* Precalculate node coordinates in both degrees and radians */
	for (row = 0; row < G_rot->header->ny; row++) grd_y[row] = GMT_grd_row_to_y (row, G_rot->header);
	for (col = 0; col < G_rot->header->nx; col++) grd_x[col] = GMT_grd_col_to_x (col, G_rot->header);
	for (row = 0; row < G_rot->header->ny; row++) grd_yc[row] = GMT_lat_swap (GMT, grd_y[row], GMT_LATSWAP_G2O);

	/* Loop over all nodes in the new rotated grid and find those inside the reconstructed polygon */
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Interpolate reconstructed grid\n");

	spotter_make_rot_matrix (GMT, Ctrl->e.lon, Ctrl->e.lat, -Ctrl->e.w, R);	/* Make inverse rotation using negative angle */
	
	GMT_grd_loop (G_rot, row, col, ij_rot) {
		G_rot->data[ij_rot] = GMT->session.f_NaN;
		if (not_global) {
			inside = seg = 0;
			while (seg < pol->n_segments && !inside) {	/* Use degrees since function expects it */
				inside = (GMT_inonout_sphpol (GMT, grd_x[col], grd_y[row], pol->segment[seg]) > 0);
				seg++;
			}
			if (!inside) continue;	/* Outside the polygon(s) */
		}
		
		/* Here we are inside; get the coordinates and rotate back to original grid coordinates */
		
		GMT_geo_to_cart (GMT, grd_yc[row], grd_x[col], P_rotated, TRUE);	/* Convert degree lon,lat to a Cartesian x,y,z vector */
		spotter_matrix_vect_mult (GMT, R, P_rotated, P_original);	/* Rotate the vector */
		GMT_cart_to_geo (GMT, &yy, &xx, P_original, TRUE);		/* Recover degree lon lat representation */
		yy = GMT_lat_swap (GMT, yy, GMT_LATSWAP_O2G);			/* Convert back to geodetic */
		xx -= 360.0;
		while (xx < G->header->wesn[XLO]) xx += 360.0;	/* Make sure we deal with 360 issues */
		G_rot->data[ij_rot] = (float)GMT_get_bcr_z (GMT, G, xx, yy, &edgeinfo, &bcr);
	}	
	
	/* Also loop over original node locations to make sure the nearest nodes are set */

	for (seg = 0; not_global && seg < pol->n_segments; seg++) {
		for (rec = 0; rec < pol->segment[seg]->n_rows; rec++) {
			lon = pol->segment[seg]->coord[GMT_X][rec];
			while (lon < G_rot->header->wesn[XLO]) lon += 360.0;
			col = GMT_grd_x_to_col (lon, G_rot->header);
			row = GMT_grd_y_to_row (pol->segment[seg]->coord[GMT_Y][rec], G_rot->header);
			/* Visit the PAD * PAD number of cells centered on col, row and make sure they have been set */
			for (row2 = (row - PAD); row2 <= (row + PAD); row2++) {
				if (row2 < 0 || row2 >= G_rot->header->ny) continue;
				for (col2 = (col - PAD); col2 <= (col + PAD); col2++) {
					if (col2 < 0 || col2 >= G_rot->header->nx) continue;
					ij_rot = GMT_IJP (G_rot->header, row2, col2);
					if (!GMT_is_fnan (G_rot->data[ij_rot])) continue;	/* Already done this */
					GMT_geo_to_cart (GMT, grd_yc[row2], grd_x[col2], P_rotated, TRUE);	/* Convert degree lon,lat to a Cartesian x,y,z vector */
					spotter_matrix_vect_mult (GMT, R, P_rotated, P_original);	/* Rotate the vector */
					GMT_cart_to_geo (GMT, &xx, &yy, P_original, TRUE);	/* Recover degree lon lat representation */
					yy = GMT_lat_swap (GMT, yy, GMT_LATSWAP_O2G);		/* Convert back to geodetic */
					col_o = GMT_grd_x_to_col (xx, G->header);
					if (col_o < 0 || col_o >= G->header->nx) continue;
					row_o = GMT_grd_y_to_row (yy, G->header);
					if (row_o < 0 || row_o >= G->header->ny) continue;
					ij = GMT_IJP (G->header, row_o, col_o);
					G_rot->data[ij_rot] = G->data[ij];
				}
			}
		}
	}

	/* Now write rotated grid */
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Write reconstructed grid\n");

	strcpy (G_rot->header->x_units, "degree");
	strcpy (G_rot->header->y_units, "degree");
	sprintf (G_rot->header->remark, "Grid rotated using lon lat omega = %g %g %g", Ctrl->e.lon, Ctrl->e.lat, Ctrl->e.w);
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&Ctrl->G.file, (void *)G_rot)) Return (GMT_DATA_WRITE_ERROR);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&G);
	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&G_rot);
	GMT_free (GMT, grd_x);
	GMT_free (GMT, grd_y);
	GMT_free (GMT, grd_yc);
	
	if (Ctrl->F.active)
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&D);
	else if (not_global)
		GMT_free_dataset (GMT, &D);

	GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");

	Return (GMT_OK);
}
