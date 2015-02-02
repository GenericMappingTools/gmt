/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Brief synopsis: grdvector reads 2 grid files that contains the 2 components of a vector
 * field (cartesian or polar) and plots vectors at the grid positions.
 * This is basically a short-hand for using grd2xyz | psxy -SV and is
 * more convenient for such plots on a grid.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"grdvector"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot vector field from two component grids"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->BJKOPRUVXYcfptxy"

struct GRDVECTOR_CTRL {
	struct In {
		bool active;
		char *file[2];
	} In;
	struct A {	/* -A */
		bool active;
	} A;
	struct C {	/* -C<cpt> */
		bool active;
		char *file;
	} C;
	struct G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct N {	/* -N */
		bool active;
	} N;
	struct Q {	/* -Q<size>[+<mods>] */
		bool active;
		struct GMT_SYMBOL S;
	} Q;
	struct S {	/* -S[l]<scale>[<unit>] */
		bool active;
		bool constant;
		char unit;
		double factor;
	} S;
	struct T {	/* -T */
		bool active;
	} T;
	struct W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z */
		bool active;
	} Z;
};

void *New_grdvector_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDVECTOR_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDVECTOR_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	GMT_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);
	C->W.pen = GMT->current.setting.map_default_pen;
	C->S.factor = 1.0;
	return (C);
}

void Free_grdvector_Ctrl (struct GMT_CTRL *GMT, struct GRDVECTOR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file[0]) free (C->In.file[0]);	
	if (C->In.file[1]) free (C->In.file[1]);	
	if (C->C.file) free (C->C.file);	
	GMT_free (GMT, C);	
}

int GMT_grdvector_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdvector <gridx> <gridy> %s %s [-A] [%s]\n", GMT_J_OPT, GMT_Rgeo_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C[<cpt>]] [-G<fill>] [-I<dx>/<dy>] [-K] [-N] [-O] [-P] [-Q<params>] [-S[i|l]<scale>[<unit>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T] [%s] [%s] [-W<pen>] [%s]\n\t[%s] [-Z] [%s] [%s]\n\t[%s] [%s]\n\n", 
		GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_f_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<gridx> <gridy> are grid files with the two vector components.\n");
	GMT_Option (API, "J-");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Grids have polar (r, theta) components [Default is Cartesian (x, y) components].\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Use cpt-file to assign colors based on vector length.  Optionally, instead give name\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of a master cpt to automatically assign 16 continuous colors over the data range [rainbow].\n");
	GMT_fill_syntax (API->GMT, 'G', "Select vector fill [Default is outlines only].");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Plot only those nodes that are <dx>/<dy> apart [Default is all nodes].\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do Not clip vectors that exceed the map boundaries [Default will clip].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Modify vector attributes [Default gives stick-plot].\n");
	GMT_vector_syntax (API->GMT, 15);
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Set scale for Cartesian vector lengths in data units per %s [1].\n", API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append c, i, or p to indicate cm, inch, or points as the distance unit.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, prepend l to indicate a fixed length for all vectors.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For Geographic vectors, set scale in data units per km.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Si<scale> to give the reciprocal scale, i.e., %s/unit or km/unit\n", API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t-T Transform angles for Cartesian grids when x- and y-scales differ [Leave alone].\n");
	GMT_Option (API, "U,V");
	GMT_pen_syntax (API->GMT, 'W', "Set pen attributes.");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default pen attributes [%s].\n", GMT_putpen(API->GMT, API->GMT->current.setting.map_default_pen));
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z The angles provided are azimuths rather than direction (requires -A).\n");
	GMT_Option (API, "c,f,p,t,.");
	
	return (EXIT_FAILURE);
}

int GMT_grdvector_parse (struct GMT_CTRL *GMT, struct GRDVECTOR_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdvector and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int j;
	size_t len;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, symbol;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	symbol = (GMT_is_geographic (GMT, GMT_IN)) ? '=' : 'v';	/* Type of vector */
	
	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = true;
				if (n_files >= 2) break;
				if (GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					Ctrl->In.file[n_files++] = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Polar data grids */
				Ctrl->A.active = true;
				break;
			case 'C':	/* Vary symbol color with z */
				Ctrl->C.active = true;
				if (Ctrl->C.file) free (Ctrl->C.file);
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'E':	/* Center vectors [OBSOLETE; use modifier +jc in -Q ] */
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -E is deprecated; use modifier +jc in -Q instead.\n");
					Ctrl->Q.S.v.status |= GMT_VEC_JUST_C;
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'G':	/* Set fill for vectors */
				Ctrl->G.active = true;
				if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'I':	/* Only use gridnodes Ctrl->I.inc[GMT_X],Ctrl->I.inc[GMT_Y] apart */
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'N':	/* Do not clip at border */
				Ctrl->N.active = true;
				break;
			case 'Q':	/* Vector plots, with parameters */
				Ctrl->Q.active = true;
				if (GMT_compat_check (GMT, 4) && (strchr (opt->arg, '/') && !strchr (opt->arg, '+'))) {	/* Old-style args */
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Vector arrowwidth/headlength/headwidth is deprecated; see -Q documentation.\n");
					for (j = 0; opt->arg[j] && opt->arg[j] != 'n'; j++);
					if (opt->arg[j]) {	/* Normalize option used */
						Ctrl->Q.S.v.v_norm = (float)GMT_to_inch (GMT, &opt->arg[j+1]);
						n_errors += GMT_check_condition (GMT, Ctrl->Q.S.v.v_norm <= 0.0, "Syntax error -Qn option: No reference length given\n");
						opt->arg[j] = '\0';	/* Temporarily chop of the n<norm> string */
					}
					if (opt->arg[0] && opt->arg[1] != 'n') {	/* We specified the three parameters */
						if (sscanf (opt->arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c) != 3) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option: Could not decode arrowwidth/headlength/headwidth\n");
							n_errors++;
						}
						else {	/* Turn the old args into new +a<angle> and pen width */
							Ctrl->Q.S.v.pen.width = GMT_to_points (GMT, txt_a);
							Ctrl->Q.S.v.h_length = (float)GMT_to_inch (GMT, txt_b);
							Ctrl->Q.S.v.h_width = (float)GMT_to_inch (GMT, txt_c);
							Ctrl->Q.S.v.v_angle = (float)atand (0.5 * Ctrl->Q.S.v.h_width / Ctrl->Q.S.v.h_length);
						}
					}
					if (Ctrl->Q.S.v.v_norm > 0.0) opt->arg[j] = 'n';	/* Restore the n<norm> string */
					Ctrl->Q.S.v.status |= (GMT_VEC_JUST_B + GMT_VEC_FILL);	/* Start filled vector at node location */
				}
				else {
					if (opt->arg[0] == '+') {	/* No size (use default), just attributes */
						Ctrl->Q.S.size_x = VECTOR_HEAD_LENGTH * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 9p */
						n_errors += GMT_parse_vector (GMT, symbol, opt->arg, &Ctrl->Q.S);
					}
					else {	/* Size, plus possible attributes */
						j = sscanf (opt->arg, "%[^+]%s", txt_a, txt_b);	/* txt_a should be symbols size with any +<modifiers> in txt_b */
						if (j == 1) txt_b[0] = 0;	/* No modifiers present, set txt_b to empty */
						Ctrl->Q.S.size_x = GMT_to_inch (GMT, txt_a);	/* Length of vector */
						n_errors += GMT_parse_vector (GMT, symbol, txt_b, &Ctrl->Q.S);
					}
				}
				break;
			case 'S':	/* Scale */
				Ctrl->S.active = true;
				len = strlen (opt->arg) - 1;
				j = (opt->arg[0] == 'i') ? 1 : 0;
				if (strchr (GMT_DIM_UNITS, (int)opt->arg[len]))	/* Recognized unit character */
					Ctrl->S.unit = opt->arg[len];
				else if (! (opt->arg[len] == '.' || isdigit ((int)opt->arg[len]))) {	/* Not decimal point or digit means trouble */
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S option: Unrecognized unit %c\n", opt->arg[len]);
					n_errors++;
				}
				if (opt->arg[0] == 'l' || opt->arg[0] == 'L') {
					Ctrl->S.constant = true;
					Ctrl->S.factor = atof (&opt->arg[1]);
				}
				else
					Ctrl->S.factor = atof (&opt->arg[j]);
				if (j == 1) Ctrl->S.factor = 1.0 / Ctrl->S.factor;	/* Got the inverse value */
				break;
			case 'T':	/* Rescale Cartesian angles */
				Ctrl->T.active = true;
				break;
			case 'W':	/* Set line attributes */
				Ctrl->W.active = true;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', " ");
					n_errors++;
				}
				break;
			case 'Z':	/* Azimuths given */
				Ctrl->Z.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, NULL, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0), "Syntax error -I option: Must specify positive increments\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.factor == 0.0 && !Ctrl->S.constant, "Syntax error -S option: Scale must be nonzero\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.factor <= 0.0 && Ctrl->S.constant, "Syntax error -Sl option: Length must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.constant && Ctrl->Q.S.v.v_norm > 0.0, "Syntax error -Sl, -Q options: Cannot use -Q..n<size> with -Sl\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Z.active && !Ctrl->A.active, "Syntax error -Z option: Azimuth adjustment not valid input for Cartesian data\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->G.active || Ctrl->W.active || Ctrl->C.active), "Syntax error: Must specify at least one of -G, -W, -C\n");
	n_errors += GMT_check_condition (GMT, n_files != 2, "Syntax error: Must specify two input grid files\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdvector_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdvector (void *V_API, int mode, void *args)
{
	unsigned int row, col, col_0, row_0, d_col, d_row, k, n_warn[3] = {0, 0, 0}, warn;
	int error = 0;
	bool justify, Geographic;
	
	uint64_t ij;

	double tmp, x, y, plot_x, plot_y, x_off, y_off, f;
	double x2, y2, wesn[4], vec_length, vec_azim, scaled_vec_length, c, s, dim[PSL_MAX_DIMS];

	struct GMT_GRID *Grid[2] = {NULL, NULL};
	struct GMT_PALETTE *P = NULL;
	struct GRDVECTOR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;	/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdvector_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdvector_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdvector_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdvector_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdvector_parse (GMT, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the grdvector main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grids\n");
	d_col = d_row = 1;
	col_0 = row_0 = 0;

	if (!(strcmp (Ctrl->In.file[0], "=") || strcmp (Ctrl->In.file[1], "="))) {
		GMT_Report (API, GMT_MSG_NORMAL, "Piping of grid files not supported!\n");
		Return (EXIT_FAILURE);
	}

	for (k = 0; k < 2; k++) {
		if ((Grid[k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
		GMT_grd_init (GMT, Grid[k]->header, options, true);
	}

	if (!(GMT_grd_same_shape (GMT, Grid[0], Grid[1]) && GMT_grd_same_region (GMT, Grid[0], Grid[1]) && GMT_grd_same_inc (GMT, Grid[0], Grid[1]))) {
		GMT_Report (API, GMT_MSG_NORMAL, "files %s and %s does not match!\n", Ctrl->In.file[0], Ctrl->In.file[1]);
		Return (EXIT_FAILURE);
	}

	/* Determine what wesn to pass to map_setup */

	if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, Grid[0]->header->wesn, 4, double);

	if (GMT_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_RUNTIME_ERROR);

	/* Determine the wesn to be used to read the grid file */

	if (!GMT_grd_setregion (GMT, Grid[0]->header, wesn, BCR_BILINEAR) || !GMT_grd_setregion (GMT, Grid[1]->header, wesn, BCR_BILINEAR)) {
		/* No grid to plot; just do empty map and return */
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Warning: No data within specified region\n");
		PSL = GMT_plotinit (GMT, options);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT_plotcanvas (GMT);	/* Fill canvas if requested */
		GMT_map_basemap (GMT);
		GMT_plane_perspective (GMT, -1, 0.0);
		GMT_plotend (GMT);
		Return (GMT_OK);
	}

	/* Read data */

	for (k = 0; k < 2; k++) if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, Ctrl->In.file[k], Grid[k]) == NULL) {	/* Get data */
		Return (API->error);
	}

	if (Ctrl->C.active) {
		double v_min, v_max;
		if (Ctrl->A.active) {	/* Polar grid, just mins min/max of r */
			v_min = Grid[0]->header->z_min;
			v_max = Grid[0]->header->z_max;
		}
		else {	/* Find min/max vector lengths from components */
			v_min = DBL_MAX;	v_max = 0.0;
			GMT_grd_loop (GMT, Grid[GMT_X], row, col, ij) {
				vec_length = hypot (Grid[GMT_X]->data[ij], Grid[GMT_Y]->data[ij]);
				if (vec_length < v_min) v_min = vec_length;
				if (vec_length > v_max) v_max = vec_length;
			}
		}
		if ((P = GMT_Get_CPT (GMT, Ctrl->C.file, GMT_CPT_OPTIONAL, v_min, v_max)) == NULL) {
			Return (API->error);
		}
	}

	Geographic = (GMT_is_geographic (GMT, GMT_IN));
	if (!Ctrl->S.constant) Ctrl->S.factor = 1.0 / Ctrl->S.factor;

	if (Geographic) {
		if (Ctrl->T.active) {
			Ctrl->T.active = false;
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: -T does not apply to geographic data - ignored\n");
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Great-circle geo-vectors will be drawn\n");
	}
	else {
		GMT_Report (API, GMT_MSG_DEBUG, "Cartesian straight vectors will be drawn\n");
		switch (Ctrl->S.unit) {	/* Adjust for possible unit selection */
			case 'c':
				Ctrl->S.factor *= GMT->session.u2u[GMT_CM][GMT_INCH];
				break;
			case 'i':
				Ctrl->S.factor *= GMT->session.u2u[GMT_INCH][GMT_INCH];
				break;
			case 'p':
				Ctrl->S.factor *= GMT->session.u2u[GMT_PT][GMT_INCH];
				break;
			default:
				Ctrl->S.factor *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
				break;
		}
	}

	Ctrl->Q.S.v.v_width = (float)(Ctrl->W.pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
	if (Ctrl->Q.active) {	/* Prepare vector parameters */
		GMT_init_vector_param (GMT, &Ctrl->Q.S, true, Ctrl->W.active, &Ctrl->W.pen, Ctrl->G.active, &Ctrl->G.fill);
	}
	dim[6] = 0.0;
	PSL = GMT_plotinit (GMT, options);
	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	GMT_plotcanvas (GMT);	/* Fill canvas if requested */

	GMT_setpen (GMT, &Ctrl->W.pen);
	if (!Ctrl->C.active) GMT_setfill (GMT, &Ctrl->G.fill, Ctrl->W.active);
	
	if (!Ctrl->N.active) GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);

	if (Ctrl->I.inc[GMT_X] != 0.0 && Ctrl->I.inc[GMT_Y] != 0.0) {	/* Coarsen the output interval. The new -Idx/dy must be integer multiples of the grid dx/dy */
		double val = Ctrl->I.inc[GMT_Y] * Grid[0]->header->r_inc[GMT_Y];	/* Should be ~ an integer within 1 ppm */
		d_row = urint (val);
		if (d_row == 0 || fabs ((d_row - val)/d_row) > GMT_CONV6_LIMIT) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: New y grid spacing (%.12lg) is not a multiple of actual grid spacing (%.12g) [within %g]\n", Ctrl->I.inc[GMT_Y], Grid[0]->header->inc[GMT_Y], GMT_CONV6_LIMIT);
			Return (EXIT_FAILURE);
		}
		Ctrl->I.inc[GMT_Y] = d_row * Grid[0]->header->inc[GMT_Y];	/* Get exact y-increment in case of slop */
		val = Ctrl->I.inc[GMT_X] * Grid[0]->header->r_inc[GMT_X];
		d_col = urint (val);
		if (d_col == 0 || fabs ((d_col - val)/d_col) > GMT_CONV6_LIMIT) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: New x grid spacing (%.12g) is not a multiple of actual grid spacing (%.12g) [within %g]\n", Ctrl->I.inc[GMT_X], Grid[0]->header->inc[GMT_X], GMT_CONV6_LIMIT);
			Return (EXIT_FAILURE);
		}
		Ctrl->I.inc[GMT_X] = d_col * Grid[0]->header->inc[GMT_X];	/* Get exact x-increment in case of slop */
		
		/* Determine starting row/col for straddled access */
		tmp = ceil (Grid[0]->header->wesn[YHI] / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
		if (tmp > Grid[0]->header->wesn[YHI]) tmp -= Ctrl->I.inc[GMT_Y];
		row_0 = urint ((Grid[0]->header->wesn[YHI] - tmp) * Grid[0]->header->r_inc[GMT_Y]);
		tmp = floor (Grid[0]->header->wesn[XLO] / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
		if (tmp < Grid[0]->header->wesn[XLO]) tmp += Ctrl->I.inc[GMT_X];
		col_0 = urint ((tmp - Grid[0]->header->wesn[XLO]) * Grid[0]->header->r_inc[GMT_X]);
	}

	dim[5] = GMT->current.setting.map_vector_shape;	/* These do not change inside the loop */
	dim[6] = (double)Ctrl->Q.S.v.status;
	PSL_command (GMT->PSL, "V\n");
	for (row = row_0; row < Grid[1]->header->ny; row += d_row) {
		y = GMT_grd_row_to_y (GMT, row, Grid[0]->header);
		for (col = col_0; col < Grid[1]->header->nx; col += d_col) {

			ij = GMT_IJP (Grid[0]->header, row, col);
			if (GMT_is_fnan (Grid[0]->data[ij]) || GMT_is_fnan (Grid[1]->data[ij])) continue;	/* Cannot plot NaN-vectors */
			x = GMT_grd_col_to_x (GMT, col, Grid[0]->header);

			if (y == 45.0 && x == 345.0) {
				vec_length = 0.0;
			}
			if (Ctrl->A.active) {	/* Got polar grids */
				vec_length = Grid[0]->data[ij];
				vec_azim   = Grid[1]->data[ij];
				if (vec_length < 0.0) {	/* Flip negative lengths as 180-degrees off */
					vec_length = -vec_length;
					vec_azim += 180.0;
				}
				else if (vec_length == 0.0) continue;	/* No length = no plotting */
			}
			else {	/* Cartesian grids: Convert to polar form of length and direction */
				vec_length = hypot (Grid[GMT_X]->data[ij], Grid[GMT_Y]->data[ij]);
				if (vec_length == 0.0) continue;	/* No length = no plotting */
				vec_azim = atan2d (Grid[GMT_Y]->data[ij], Grid[GMT_X]->data[ij]);
			}
			if (!Ctrl->N.active) {	/* Throw out vectors whose node is outside */
				GMT_map_outside (GMT, x, y);
				if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
			}
			
			if (Ctrl->C.active) {	/* Update pen and fill color based on the vector length */
				GMT_get_fill_from_z (GMT, P, vec_length, &Ctrl->G.fill);
				GMT_rgb_copy (Ctrl->W.pen.rgb, Ctrl->G.fill.rgb);
				GMT_setpen (GMT, &Ctrl->W.pen);
				if (Ctrl->Q.active) GMT_setfill (GMT, &Ctrl->G.fill, Ctrl->W.active);
				GMT_init_vector_param (GMT, &Ctrl->Q.S, true, Ctrl->W.active, &Ctrl->W.pen, true, &Ctrl->G.fill);
			}
			scaled_vec_length = (Ctrl->S.constant) ? Ctrl->S.factor : vec_length * Ctrl->S.factor;
			/* scaled_vec_length is now in inches (Cartesian) or km (Geographic) */
			
			if (Geographic) {	/* Draw great-circle geo-vectors */
				if (!Ctrl->A.active) vec_azim = 90.0 - vec_azim;	/* We got direction components; convert to azimuth */
				warn = GMT_geo_vector (GMT, x, y, vec_azim, scaled_vec_length, &Ctrl->W.pen, &Ctrl->Q.S);
				n_warn[warn]++;
			}
			else {	/* Draw straight Cartesian vectors */
				GMT_geo_to_xy (GMT, x, y, &plot_x, &plot_y);
				if (Ctrl->T.active) {	/* Transform azimuths to plot angle */
					if (!Ctrl->Z.active) vec_azim = 90.0 - vec_azim;
					vec_azim = GMT_azim_to_angle (GMT, x, y, 0.1, vec_azim);
				}
				GMT_flip_angle_d (GMT, &vec_azim);
				vec_azim *= D2R;	/* vec_azim is now in radians */
				sincos (vec_azim, &s, &c);
				x2 = plot_x + scaled_vec_length * c;
				y2 = plot_y + scaled_vec_length * s;

				justify = GMT_vec_justify (Ctrl->Q.S.v.status);	/* Return justification as 0-2 */
				if (justify) {	/* Justify vector at center, or tip [beginning] */
					x_off = justify * 0.5 * (x2 - plot_x);	y_off = justify * 0.5 * (y2 - plot_y);
					plot_x -= x_off;	plot_y -= y_off;
					x2 -= x_off;		y2 -= y_off;
				}
				n_warn[0]++;
				if (!Ctrl->Q.active) {	/* Just a line segment */
					PSL_plotsegment (PSL, plot_x, plot_y, x2, y2);
					continue;
				}
				/* Must plot a vector */
				dim[0] = x2; dim[1] = y2;
				dim[2] = Ctrl->Q.S.v.v_width;	dim[3] = Ctrl->Q.S.v.h_length;	dim[4] = Ctrl->Q.S.v.h_width;
				if (scaled_vec_length < Ctrl->Q.S.v.v_norm) {	/* Scale arrow attributes down with length */
					f = scaled_vec_length / Ctrl->Q.S.v.v_norm;
					for (k = 2; k <= 4; k++) dim[k] *= f;
				}
				PSL_plotsymbol (PSL, plot_x, plot_y, dim, PSL_VECTOR);
			}
		}
	}
	PSL_command (GMT->PSL, "U\n");
	PSL->current.linewidth = 0.0;	/* Since we changed things under clip; this will force it to be set next */

	if (!Ctrl->N.active) GMT_map_clip_off (GMT);

	GMT_map_basemap (GMT);

	GMT_plane_perspective (GMT, -1, 0.0);

	GMT_plotend (GMT);
	
	GMT_Report (API, GMT_MSG_VERBOSE, "%d vectors plotted successfully\n", n_warn[0]);
	if (n_warn[1]) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: %d vector heads had length exceeding the vector length and were skipped. Consider the +n<norm> modifier to -Q\n", n_warn[1]);
	if (n_warn[2]) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: %d vector heads had to be scaled more than implied by +n<norm> since they were still too long. Consider changing the +n<norm> modifier to -Q\n", n_warn[2]);
	

	Return (EXIT_SUCCESS);
}
