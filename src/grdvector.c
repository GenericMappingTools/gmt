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
 * Brief synopsis: grdvector reads 2 grid files that contains the 2 components of a vector
 * field (cartesian or polar) and plots vectors at the grid positions.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#define THIS_MODULE_CLASSIC_NAME	"grdvector"
#define THIS_MODULE_MODERN_NAME	"grdvector"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot vector field from two component grids"
#define THIS_MODULE_KEYS	"<G{2,CC(,>X}"
#define THIS_MODULE_NEEDS	"Jg"

#include "gmt_dev.h"

#define THIS_MODULE_OPTIONS "->BJKOPRUVXYfptxy" GMT_OPT("c")

struct GRDVECTOR_CTRL {
	struct In {
		bool active;
		char *file[2];
	} In;
	struct A {	/* -A */
		bool active;
	} A;
	struct C {	/* -C<cpt>[+i<dz>] */
		bool active;
		double dz;
		char *file;
	} C;
	struct G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I[x]<dx>[/<dy>] */
		bool active;
		unsigned int mode;
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
		bool cpt_effect;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z */
		bool active;
	} Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDVECTOR_CTRL *C = NULL;
	char unit[5] = "cimp";

	C = gmt_M_memory (GMT, NULL, 1, struct GRDVECTOR_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	gmt_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);
	C->Q.S.symbol = PSL_VECTOR;
	C->W.pen = GMT->current.setting.map_default_pen;
	C->S.factor = 1.0;
	C->S.unit = unit[GMT->current.setting.proj_length_unit];
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDVECTOR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file[GMT_IN]);
	gmt_M_str_free (C->In.file[GMT_OUT]);
	gmt_M_str_free (C->C.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <gridx> <gridy> %s %s [-A] [%s]\n", name, GMT_J_OPT, GMT_Rgeo_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C[<cpt>]] [-G<fill>] [-I[x]<dx>/<dy>] %s[-N] %s%s[-Q<params>] [-S[i|l]<scale>[<unit>]]\n", API->K_OPT, API->O_OPT, API->P_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-T] [%s] [%s] [-W<pen>] [%s]\n\t[%s] [-Z] %s[%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_f_OPT, GMT_p_OPT, GMT_t_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<gridx> <gridy> are grid files with the two vector components.\n");
	GMT_Option (API, "J-");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Grids have polar (r, theta) components [Default is Cartesian (x, y) components].\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Color palette file to convert vector length to colors. Optionally, name a master cpt\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to automatically assign continuous colors over the data range [%s]; if so,\n", GMT_DEFAULT_CPT_NAME);
	GMT_Message (API, GMT_TIME_NONE, "\t   optionally append +i<dz> to quantize the range [the exact grid range].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Another option is to specify -Ccolor1,color2[,color3,...] to build a linear\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   continuous cpt from those colors automatically.\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Select vector fill [Default is outlines only].");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Plot only those nodes that are <dx>/<dy> apart [Default is all nodes].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, use -Ix<fact>[/<yfact>] to give multiples of grid spacing.\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do Not clip vectors that exceed the map boundaries [Default will clip].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Modify vector attributes [Default gives stick-plot].\n");
	gmt_vector_syntax (API->GMT, 15);
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Set scale for vector lengths in data units per plot distance unit.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append c, i, or p to indicate cm, inch, or points as the distance unit  [1%s].\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend l to instead set a fixed length (in given unit) for all vectors.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   These vectors are straight and their plot lengths are independent of the projection.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For Geographic vectors you may alternatively give data units per distance unit\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   by appending any of the units in %s to the scale.\n", GMT_DIM_UNITS);
	GMT_Message (API, GMT_TIME_NONE, "\t   These are geovectors and their plot lengths depend on the projection.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Si<scale>[unit] to give the reciprocal scale, i.e., in %s/unit or km/unit\n",
	             API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Vl to see the min, max, and mean vector length.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Transform angles for Cartesian grids when x- and y-scales differ [Leave alone].\n");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Set pen attributes.", 0);
	GMT_Message (API, GMT_TIME_NONE, "\t   Default pen attributes [%s].\n", gmt_putpen(API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z The theta grid provided has azimuths rather than directions (implies -A).\n");
	GMT_Option (API, "c,f,p,t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDVECTOR_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdvector and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int j;
	size_t len;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, symbol, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	symbol = (gmt_M_is_geographic (GMT, GMT_IN)) ? '=' : 'v';	/* Type of vector */

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = true;
				if (n_files >= 2) break;
				if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
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
				if ((c = strstr (opt->arg, "+i"))) {	/* Gave auto-interval */
					Ctrl->C.dz = atof (&c[2]);
					c[0] = '\0';	/* Temporarily chop off the modifier */
				}
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				if (c) c[0] = '+';	/* Restore */
				break;
			case 'E':	/* Center vectors [OBSOLETE; use modifier +jc in -Q ] */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -E is deprecated; use modifier +jc in -Q instead.\n");
					Ctrl->Q.S.v.status |= PSL_VEC_JUST_C;
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'G':	/* Set fill for vectors */
				Ctrl->G.active = true;
				if (gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'I':	/* Only use gridnodes GMT->common.R.inc[GMT_X],GMT->common.R.inc[GMT_Y] apart */
				Ctrl->I.active = true;
				if (opt->arg[0] == 'x') Ctrl->I.mode = 1;
				n_errors += gmt_parse_inc_option (GMT, 'I', &opt->arg[Ctrl->I.mode]);
				break;
			case 'N':	/* Do not clip at border */
				Ctrl->N.active = true;
				break;
			case 'Q':	/* Vector plots, with parameters */
				Ctrl->Q.active = true;
				if (gmt_M_compat_check (GMT, 4) && (strchr (opt->arg, '/') && !strchr (opt->arg, '+'))) {	/* Old-style args */
					if (gmt_M_is_geographic (GMT, GMT_IN))
						GMT_Report (API, GMT_MSG_COMPAT, "Vector arrowwidth/headlength/headwidth is deprecated for geo-vectors; see -Q documentation.\n");
					Ctrl->Q.S.v.status = PSL_VEC_END + PSL_VEC_FILL + PSL_VEC_OUTLINE;
					Ctrl->Q.S.size_x = VECTOR_HEAD_LENGTH * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 9p */
					Ctrl->Q.S.v.h_length = (float)Ctrl->Q.S.size_x;	/* 9p */
					Ctrl->Q.S.v.v_angle = 60.0f;
					Ctrl->Q.S.v.pen = GMT->current.setting.map_default_pen;
					for (j = 0; opt->arg[j] && opt->arg[j] != 'n'; j++);
					if (opt->arg[j]) {	/* Normalize option used */
						Ctrl->Q.S.v.v_norm = (float)gmt_M_to_inch (GMT, &opt->arg[j+1]);	/* Getting inches directly here */
						n_errors += gmt_M_check_condition (GMT, Ctrl->Q.S.v.v_norm <= 0.0, "Syntax error -Qn option: No reference length given\n");
						opt->arg[j] = '\0';	/* Temporarily chop of the n<norm> string */
					}
					if (opt->arg[0] && opt->arg[1] != 'n') {	/* We specified the three parameters */
						if (sscanf (opt->arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c) != 3) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option: Could not decode arrowwidth/headlength/headwidth\n");
							n_errors++;
						}
						else {	/* Turn the old args into new +a<angle> and pen width */
							Ctrl->Q.S.v.v_width = (float)gmt_M_to_inch (GMT, txt_a);
							Ctrl->Q.S.v.pen.width = gmt_M_to_points (GMT, txt_a);
							Ctrl->Q.S.v.h_length = (float)gmt_M_to_inch (GMT, txt_b);
							Ctrl->Q.S.v.h_width = (float)gmt_M_to_inch (GMT, txt_c);
						}
					}
					if (Ctrl->Q.S.v.v_norm > 0.0) opt->arg[j] = 'n';	/* Restore the n<norm> string */
					Ctrl->Q.S.v.status |= (PSL_VEC_JUST_B + PSL_VEC_FILL);	/* Start filled vector at node location */
					Ctrl->Q.S.symbol = GMT_SYMBOL_VECTOR_V4;
				}
				else {
					if (opt->arg[0] == '+') {	/* No size (use default), just attributes */
						Ctrl->Q.S.size_x = VECTOR_HEAD_LENGTH * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 9p */
						n_errors += gmt_parse_vector (GMT, symbol, opt->arg, &Ctrl->Q.S);
					}
					else {	/* Size, plus possible attributes */
						j = sscanf (opt->arg, "%[^+]%s", txt_a, txt_b);	/* txt_a should be symbols size with any +<modifiers> in txt_b */
						if (j == 1) txt_b[0] = 0;	/* No modifiers present, set txt_b to empty */
						Ctrl->Q.S.size_x = gmt_M_to_inch (GMT, txt_a);	/* Length of vector */
						n_errors += gmt_parse_vector (GMT, symbol, txt_b, &Ctrl->Q.S);
					}
					/* Must possibly change v_norm to inches if given in another Cartesian unit */
					if (Ctrl->Q.S.u_set && Ctrl->Q.S.u != GMT_INCH) {
						Ctrl->Q.S.v.v_norm *= GMT->session.u2u[Ctrl->Q.S.u][GMT_INCH];	/* Since we are not reading this again we change to inches */
						Ctrl->Q.S.u = GMT_INCH;
					}
				}
				break;
			case 'S':	/* Scale */
				Ctrl->S.active = true;
				len = strlen (opt->arg) - 1;
				j = (opt->arg[0] == 'i') ? 1 : 0;
				if (strchr (GMT_DIM_UNITS GMT_LEN_UNITS, (int)opt->arg[len]))	/* Recognized plot length or map distance unit character */
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
				if (gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", 0);
					n_errors++;
				}
				if (Ctrl->W.pen.cptmode) Ctrl->W.cpt_effect = true;
				break;
			case 'Z':	/* Azimuths given */
				Ctrl->A.active = true;
				Ctrl->Z.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[ISET] && (GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0),
	                                 "Syntax error -I option: Must specify positive increments\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.factor == 0.0 && !Ctrl->S.constant, "Syntax error -S option: Scale must be nonzero\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.factor <= 0.0 && Ctrl->S.constant, "Syntax error -Sl option: Length must be positive\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.constant && Ctrl->Q.S.v.v_norm > 0.0,
	                                 "Syntax error -Sl, -Q options: Cannot use -Q..n<size> with -Sl\n");
	n_errors += gmt_M_check_condition (GMT, !(Ctrl->G.active || Ctrl->W.active || Ctrl->C.active),
	                                 "Syntax error: Must specify at least one of -G, -W, -C\n");
	n_errors += gmt_M_check_condition (GMT, n_files != 2, "Syntax error: Must specify two input grid files\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.cpt_effect && !Ctrl->C.active, "Syntax error -W: modifier +c only makes sense if -C is given\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdvector (void *V_API, int mode, void *args) {
	unsigned int justify, row, col, col_0, row_0, d_col, d_row, k, n_warn[3] = {0, 0, 0}, warn;
	int error = 0;
	bool Geographic;

	uint64_t ij;

	double tmp, x, y, plot_x, plot_y, x_off, y_off, f, headpen_width = 0.0;
	double x2, y2, wesn[4], value, vec_length, vec_azim, scaled_vec_length, c, s, dim[PSL_MAX_DIMS];

	struct GMT_GRID *Grid[2] = {NULL, NULL};
	struct GMT_PALETTE *P = NULL;
	struct GMT_PEN last_headpen;
	struct GRDVECTOR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;	/* General PSL internal parameters */
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
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdvector main code ----------------------------*/

	gmt_M_memset (&last_headpen, 1, struct GMT_PEN);		/* Initilaize members to zero */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input grids\n");
	d_col = d_row = 1;
	col_0 = row_0 = 0;

	if (!(strcmp (Ctrl->In.file[0], "=") || strcmp (Ctrl->In.file[1], "="))) {
		GMT_Report (API, GMT_MSG_NORMAL, "Piping of grid files not supported!\n");
		Return (GMT_RUNTIME_ERROR);
	}

	for (k = 0; k < 2; k++) {
		if ((Grid[k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
		if ((API->error = gmt_img_sanitycheck (GMT, Grid[k]->header))) {	/* Used map projection on a Mercator (cartesian) grid */
			Return (API->error);
		}
		gmt_grd_init (GMT, Grid[k]->header, options, true);
	}

	if (!(gmt_M_grd_same_shape (GMT, Grid[0], Grid[1]) && gmt_M_grd_same_region (GMT, Grid[0], Grid[1]) && gmt_M_grd_same_inc (GMT, Grid[0], Grid[1]))) {
		GMT_Report (API, GMT_MSG_NORMAL, "files %s and %s does not match!\n", Ctrl->In.file[0], Ctrl->In.file[1]);
		Return (GMT_RUNTIME_ERROR);
	}

	/* Determine what wesn to pass to map_setup */

	if (!GMT->common.R.active[RSET])	/* -R was not set so we use the grid domain */
		gmt_set_R_from_grd (GMT, Grid[0]->header);

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	/* Determine the wesn to be used to read the grid file */

	if (!gmt_grd_setregion (GMT, Grid[0]->header, wesn, BCR_BILINEAR) || !gmt_grd_setregion (GMT, Grid[1]->header, wesn, BCR_BILINEAR)) {
		/* No grid to plot; just do empty map and return */
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "No data within specified region\n");
		if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		gmt_plotcanvas (GMT);	/* Fill canvas if requested */
		gmt_map_basemap (GMT);
		gmt_plane_perspective (GMT, -1, 0.0);
		gmt_plotend (GMT);
		Return (GMT_NOERROR);
	}

	/* Read data */

	for (k = 0; k < 2; k++) if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file[k], Grid[k]) == NULL) {	/* Get data */
		Return (API->error);
	}

	if (Ctrl->C.active) {
		double v_min, v_max;
		if (Ctrl->A.active) {	/* Polar grid, just use min/max of radius grid */
			v_min = Grid[0]->header->z_min;
			v_max = Grid[0]->header->z_max;
		}
		else {	/* Find min/max vector lengths from the components */
			v_min = DBL_MAX;	v_max = 0.0;
			gmt_M_grd_loop (GMT, Grid[GMT_X], row, col, ij) {
				vec_length = hypot (Grid[GMT_X]->data[ij], Grid[GMT_Y]->data[ij]);
				if (vec_length < v_min) v_min = vec_length;
				if (vec_length > v_max) v_max = vec_length;
			}
		}
		if ((P = gmt_get_palette (GMT, Ctrl->C.file, GMT_CPT_OPTIONAL, v_min, v_max, Ctrl->C.dz, GMT_DEFAULT_CPT)) == NULL) {
			Return (API->error);
		}
	}

	Geographic = (gmt_M_is_geographic (GMT, GMT_IN));	/* Will be overridden if c|i|p units for scaling is selected */

	switch (Ctrl->S.unit) {	/* Adjust for possible unit selection */
		/* First three choices will give straight vectors scaled from user length to plot lengths */
		case 'c':
			Ctrl->S.factor *= GMT->session.u2u[GMT_CM][GMT_INCH];
			Geographic = false;
			break;
		case 'i':
			Ctrl->S.factor *= GMT->session.u2u[GMT_INCH][GMT_INCH];
			Geographic = false;
			break;
		case 'p':
			Ctrl->S.factor *= GMT->session.u2u[GMT_PT][GMT_INCH];
			Geographic = false;
			break;
		/* Remaining choices will give geo vectors scaled from user length to distance lengths [dmsefkMnu] */
		case 'd':
			Ctrl->S.factor *= GMT->current.proj.KM_PR_DEG;
			break;
		case 'm':
			Ctrl->S.factor *= (GMT->current.proj.KM_PR_DEG / GMT_DEG2MIN_F);
			break;
		case 's':
			Ctrl->S.factor *= (GMT->current.proj.KM_PR_DEG / GMT_DEG2SEC_F);
			break;
		case 'e':
			Ctrl->S.factor /= METERS_IN_A_KM;
			break;
		case 'f':
			Ctrl->S.factor *= (METERS_IN_A_FOOT / METERS_IN_A_KM);
			break;
		case 'k':
			/* Already in km */
			break;
		case 'M':
			Ctrl->S.factor /= (METERS_IN_A_MILE / METERS_IN_A_KM);
			break;
		case 'n':
			Ctrl->S.factor /= (METERS_IN_A_NAUTICAL_MILE / METERS_IN_A_KM);
			break;
		case 'u':
			Ctrl->S.factor *= (METERS_IN_A_SURVEY_FOOT / METERS_IN_A_KM);
			break;
		default:
			GMT_Report (API, GMT_MSG_NORMAL, "Bad scale unit %c\n", Ctrl->S.unit);
			Return (GMT_RUNTIME_ERROR);
			break;
	}

	if (!Ctrl->S.constant) Ctrl->S.factor = 1.0 / Ctrl->S.factor;
	if (Geographic) {	/* Now that we know this we make sure -T is disabled if given */
		if (Ctrl->T.active) {	/* This is a mistake */
			Ctrl->T.active = false;
			GMT_Report (API, GMT_MSG_NORMAL, "-T does not apply to geographic grids - ignored\n");
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Great-circle geo-vectors will be drawn. Scale converting user lengths to km is %g\n", Ctrl->S.factor);
	}
	else {
		GMT_Report (API, GMT_MSG_DEBUG, "Cartesian straight vectors will be drawn\n");
		GMT_Report (API, GMT_MSG_DEBUG, "Cartesian straight vectors will be drawn. Scale converting user lengths to inches is %g\n", Ctrl->S.factor);
	}

	if (Ctrl->Q.active) {	/* Prepare vector parameters */
		if (Ctrl->Q.S.symbol == PSL_VECTOR) Ctrl->Q.S.v.v_width = (float)(Ctrl->W.pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
		gmt_init_vector_param (GMT, &Ctrl->Q.S, true, Ctrl->W.active, &Ctrl->W.pen, Ctrl->G.active, &Ctrl->G.fill);
	}
	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */

	gmt_setpen (GMT, &Ctrl->W.pen);
	if (!Ctrl->C.active) gmt_setfill (GMT, &Ctrl->G.fill, Ctrl->W.active);

	if (!Ctrl->N.active) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);
	if (Ctrl->I.mode) {	/* Gave multiplier so get actual strides */
		GMT->common.R.inc[GMT_X] *= Grid[0]->header->inc[GMT_X];
		GMT->common.R.inc[GMT_Y] *= Grid[0]->header->inc[GMT_Y];
	}
	if (GMT->common.R.inc[GMT_X] != 0.0 && GMT->common.R.inc[GMT_Y] != 0.0) {	/* Coarsen the output interval. The new -Idx/dy must be integer multiples of the grid dx/dy */
		struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (Grid[0]->header);
		double val = GMT->common.R.inc[GMT_Y] * HH->r_inc[GMT_Y];	/* Should be ~ an integer within 1 ppm */
		d_row = urint (val);
		if (d_row == 0 || fabs ((d_row - val)/d_row) > GMT_CONV6_LIMIT) {
			GMT_Report (API, GMT_MSG_NORMAL, "New y grid spacing (%.12lg) is not a multiple of actual grid spacing (%.12g) [within %g]\n", GMT->common.R.inc[GMT_Y], Grid[0]->header->inc[GMT_Y], GMT_CONV6_LIMIT);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT->common.R.inc[GMT_Y] = d_row * Grid[0]->header->inc[GMT_Y];	/* Get exact y-increment in case of slop */
		val = GMT->common.R.inc[GMT_X] * HH->r_inc[GMT_X];
		d_col = urint (val);
		if (d_col == 0 || fabs ((d_col - val)/d_col) > GMT_CONV6_LIMIT) {
			GMT_Report (API, GMT_MSG_NORMAL, "New x grid spacing (%.12g) is not a multiple of actual grid spacing (%.12g) [within %g]\n", GMT->common.R.inc[GMT_X], Grid[0]->header->inc[GMT_X], GMT_CONV6_LIMIT);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT->common.R.inc[GMT_X] = d_col * Grid[0]->header->inc[GMT_X];	/* Get exact x-increment in case of slop */
	
		/* Determine starting row/col for straddled access */
		tmp = ceil (Grid[0]->header->wesn[YHI] / GMT->common.R.inc[GMT_Y]) * GMT->common.R.inc[GMT_Y];
		if (tmp > Grid[0]->header->wesn[YHI]) tmp -= GMT->common.R.inc[GMT_Y];
		row_0 = urint ((Grid[0]->header->wesn[YHI] - tmp) * HH->r_inc[GMT_Y]);
		tmp = floor (Grid[0]->header->wesn[XLO] / GMT->common.R.inc[GMT_X]) * GMT->common.R.inc[GMT_X];
		if (tmp < Grid[0]->header->wesn[XLO]) tmp += GMT->common.R.inc[GMT_X];
		col_0 = urint ((tmp - Grid[0]->header->wesn[XLO]) * HH->r_inc[GMT_X]);
	}

	dim[5] = Ctrl->Q.S.v.v_shape;	/* dim[5-8] do not change inside the loop */
	dim[6] = (double)Ctrl->Q.S.v.status;
	dim[7] = (double)Ctrl->Q.S.v.v_kind[0];	dim[8] = (double)Ctrl->Q.S.v.v_kind[1];

	if (Ctrl->Q.S.v.status & PSL_VEC_OUTLINE2) {	/* Vector head outline pen specified separately */
		PSL_defpen (PSL, "PSL_vecheadpen", Ctrl->Q.S.v.pen.width, Ctrl->Q.S.v.pen.style, Ctrl->Q.S.v.pen.offset, Ctrl->Q.S.v.pen.rgb);
		headpen_width = Ctrl->Q.S.v.pen.width;
	}
	else {	/* Reset to default pen */
		if (Ctrl->W.active) {	/* Vector head outline pen default is half that of stem pen */
			PSL_defpen (PSL, "PSL_vecheadpen", Ctrl->W.pen.width, Ctrl->W.pen.style, Ctrl->W.pen.offset, Ctrl->W.pen.rgb);
			headpen_width = 0.5 * Ctrl->W.pen.width;
		}
	}
	if (Ctrl->W.cpt_effect) {	/* Should color apply to pen, fill, or both [fill] */
		if ((Ctrl->W.pen.cptmode & 2) == 0 && !Ctrl->G.active)	/* Turn off CPT fill */
			gmt_M_rgb_copy (Ctrl->G.fill.rgb, GMT->session.no_rgb);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) {	/* Report min/max/mean scaled vector length */
		double v_min = DBL_MAX, v_max = -DBL_MAX, v_mean = 0.0;
		uint64_t v_n = 0;
		char v_unit[GMT_LEN8] = {""};
	
		for (row = row_0; row < Grid[1]->header->n_rows; row += d_row) {
			y = gmt_M_grd_row_to_y (GMT, row, Grid[0]->header);	/* Latitude OR y OR radius */
			for (col = col_0; col < Grid[1]->header->n_columns; col += d_col) {

				ij = gmt_M_ijp (Grid[0]->header, row, col);
				if (gmt_M_is_fnan (Grid[0]->data[ij]) || gmt_M_is_fnan (Grid[1]->data[ij])) continue;	/* Cannot plot NaN-vectors */
				x = gmt_M_grd_col_to_x (GMT, col, Grid[0]->header);	/* Longitude OR x OR theta [or azimuth] */
				if (!Ctrl->N.active) {	/* Throw out vectors whose node is outside */
					gmt_map_outside (GMT, x, y);
					if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
				}

				if (Ctrl->A.active) {	/* Got r,theta grids */
					vec_length = Grid[0]->data[ij];
					if (vec_length == 0.0) continue;	/* No length = no plotting */
					if (vec_length < 0.0)	/* Interpret negative lengths to mean pointing in opposite direction 180-degrees off */
						vec_length = -vec_length;
				}
				else {	/* Cartesian component grids: Convert to polar form of radius, theta */
					vec_length = hypot (Grid[GMT_X]->data[ij], Grid[GMT_Y]->data[ij]);
					if (vec_length == 0.0) continue;	/* No length = no plotting */
				}
				scaled_vec_length = (Ctrl->S.constant) ? Ctrl->S.factor : vec_length * Ctrl->S.factor;
				/* scaled_vec_length is now in inches (Cartesian) or km (Geographic) */
				if (scaled_vec_length < v_min) v_min = scaled_vec_length;
				if (scaled_vec_length > v_max) v_max = scaled_vec_length;
				v_mean += scaled_vec_length;
				v_n++;
			}
		}
		if (v_n) v_mean /= v_n;
		if (Geographic)
			sprintf (v_unit, "km");
		else {	/* Report length in selected unit and scale results to match */
			strcpy (v_unit, API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
			v_min *= GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
			v_max *= GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
			v_mean *= GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
		}
					
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Minimum length of scaled vector in %s  : %g\n", v_unit, v_min);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Maximum length of scaled vector in %s  : %g\n", v_unit, v_max);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Mean length of the scaled vector in %s : %g\n", v_unit, v_mean);
	}

	PSL_command (GMT->PSL, "V\n");
	for (row = row_0; row < Grid[1]->header->n_rows; row += d_row) {
		y = gmt_M_grd_row_to_y (GMT, row, Grid[0]->header);	/* Latitude OR y OR radius */
		for (col = col_0; col < Grid[1]->header->n_columns; col += d_col) {

			ij = gmt_M_ijp (Grid[0]->header, row, col);
			if (gmt_M_is_fnan (Grid[0]->data[ij]) || gmt_M_is_fnan (Grid[1]->data[ij])) continue;	/* Cannot plot NaN-vectors */
			x = gmt_M_grd_col_to_x (GMT, col, Grid[0]->header);	/* Longitude OR x OR theta [or azimuth] */
			if (!Ctrl->N.active) {	/* Throw out vectors whose node is outside */
				gmt_map_outside (GMT, x, y);
				if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
			}

			if (Ctrl->A.active) {	/* Got r,theta grids */
				vec_length = Grid[0]->data[ij];
				if (vec_length == 0.0) continue;	/* No length = no plotting */
				vec_azim   = Grid[1]->data[ij];
				value = vec_length;
				if (vec_length < 0.0) {	/* Interpret negative lengths to mean pointing in opposite direction 180-degrees off */
					vec_length = -vec_length;
					vec_azim += 180.0;
				}
				if (!Ctrl->Z.active) vec_azim = 90.0 - vec_azim;	/* Convert theta to azimuth */
			}
			else {	/* Cartesian component grids: Convert to polar form of radius, theta */
				vec_length = hypot (Grid[GMT_X]->data[ij], Grid[GMT_Y]->data[ij]);
				if (vec_length == 0.0) continue;	/* No length = no plotting */
				vec_azim = 90.0 - atan2d (Grid[GMT_Y]->data[ij], Grid[GMT_X]->data[ij]);	/* Convert dy,dx to azimuth */
				value = vec_length;
			}
		
			if (Ctrl->C.active) {	/* Get color based on the vector length */
				gmt_get_fill_from_z (GMT, P, value, &Ctrl->G.fill);
			}
		
			if (Ctrl->W.cpt_effect) {	/* Should color apply to pen, fill, or both [fill] */
				if (Ctrl->W.pen.cptmode & 1) {	/* Change pen color via CPT */
					gmt_M_rgb_copy (Ctrl->W.pen.rgb, Ctrl->G.fill.rgb);
					if (!gmt_M_same_pen (Ctrl->W.pen, last_headpen)) {	/* Since color may have changed */
						PSL_defpen (PSL, "PSL_vecheadpen", Ctrl->W.pen.width, Ctrl->W.pen.style, Ctrl->W.pen.offset, Ctrl->W.pen.rgb);
						last_headpen = Ctrl->W.pen;
					}
				}
			}
			if (Ctrl->C.active) {	/* Update pen and fill color settings */
				if (!Ctrl->Q.active)	/* Must update stick pen */
					gmt_M_rgb_copy (Ctrl->W.pen.rgb, Ctrl->G.fill.rgb);
				gmt_setpen (GMT, &Ctrl->W.pen);
				if (Ctrl->Q.active) gmt_setfill (GMT, &Ctrl->G.fill, Ctrl->W.active);
				gmt_init_vector_param (GMT, &Ctrl->Q.S, true, Ctrl->W.active, &Ctrl->W.pen, true, &Ctrl->G.fill);
			}

			scaled_vec_length = (Ctrl->S.constant) ? Ctrl->S.factor : vec_length * Ctrl->S.factor;
			/* scaled_vec_length is now in inches (Cartesian) or km (Geographic) */
		
			if (Geographic) {	/* Draw great-circle geo-vectors */
				warn = gmt_geo_vector (GMT, x, y, vec_azim, scaled_vec_length, &Ctrl->W.pen, &Ctrl->Q.S);
				n_warn[warn]++;
			}
			else {	/* Draw straight Cartesian vectors */
				gmt_geo_to_xy (GMT, x, y, &plot_x, &plot_y);
				if (Ctrl->T.active)	/* Deal with negative scales in x and/or y which affect the azimuths */
					gmt_flip_azim_d (GMT, &vec_azim);
				vec_azim = 90.0 - vec_azim;	/* Transform azimuths to plot angle */
				if (GMT->current.proj.projection_GMT == GMT_POLAR) {	/* Must rotate azimuth since circular projection */
					double x_orient;
					x_orient = (GMT->current.proj.got_azimuths) ? -(x + GMT->current.proj.p_base_angle) : x - GMT->current.proj.p_base_angle - 90.0;
					vec_azim += x_orient;
				}
				vec_azim *= D2R;		/* vec_azim is now in radians */
				sincos (vec_azim, &s, &c);
				x2 = plot_x + scaled_vec_length * c;
				y2 = plot_y + scaled_vec_length * s;

				justify = PSL_vec_justify (Ctrl->Q.S.v.status);	/* Return justification as 0-2 */
				if (justify) {	/* Justify vector at center, or tip [beginning] */
					x_off = justify * 0.5 * (x2 - plot_x);	y_off = justify * 0.5 * (y2 - plot_y);
					plot_x -= x_off;	plot_y -= y_off;
					x2 -= x_off;		y2 -= y_off;
				}
				n_warn[0]++;
				if (!Ctrl->Q.active) {	/* Just a vector stem: line segment */
					PSL_plotsegment (PSL, plot_x, plot_y, x2, y2);
					continue;
				}
				/* Must plot a vector head */
				dim[0] = x2; dim[1] = y2;
				dim[2] = Ctrl->Q.S.v.v_width;	dim[3] = Ctrl->Q.S.v.h_length;	dim[4] = Ctrl->Q.S.v.h_width;
				dim[11] = headpen_width;	/* Possibly shrunk head pen width */
				if (scaled_vec_length < Ctrl->Q.S.v.v_norm) {	/* Scale arrow attributes down with length */
					f = scaled_vec_length / Ctrl->Q.S.v.v_norm;
					for (k = 2; k <= 4; k++) dim[k] *= f;
					dim[11] *= f;
				}
				if (Ctrl->Q.S.symbol == GMT_SYMBOL_VECTOR_V4) {	/* Do the deprecated GMT4 vector polygon instead */
					int v4_outline = Ctrl->W.active;
					double *this_rgb = NULL;
					if (Ctrl->G.active || Ctrl->C.active)
						this_rgb = Ctrl->G.fill.rgb;
					else
						this_rgb = GMT->session.no_rgb;
					if (v4_outline) gmt_setpen (GMT, &Ctrl->W.pen);
					if (Ctrl->Q.S.v.status & PSL_VEC_BEGIN) v4_outline += 8;	/* Double-headed */
					psl_vector_v4 (PSL, plot_x, plot_y, dim, this_rgb, v4_outline);
				}
				else
					PSL_plotsymbol (PSL, plot_x, plot_y, dim, PSL_VECTOR);
			}
		}
	}
	PSL_command (GMT->PSL, "U\n");
	PSL->current.linewidth = 0.0;	/* Since we changed things under clip; this will force it to be set next */

	if (!Ctrl->N.active) gmt_map_clip_off (GMT);

	gmt_map_basemap (GMT);

	gmt_plane_perspective (GMT, -1, 0.0);

	gmt_plotend (GMT);

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%d vectors plotted successfully\n", n_warn[0]);
	if (n_warn[1]) GMT_Report (API, GMT_MSG_VERBOSE, "%d vector heads had length exceeding the vector length and were skipped. Consider the +n<norm> modifier to -Q\n", n_warn[1]);
	if (n_warn[2]) GMT_Report (API, GMT_MSG_VERBOSE, "%d vector heads had to be scaled more than implied by +n<norm> since they were still too long. Consider changing the +n<norm> modifier to -Q\n", n_warn[2]);


	Return (GMT_NOERROR);
}
