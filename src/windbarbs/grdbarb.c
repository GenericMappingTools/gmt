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
 * Brief synopsis: grdbarb reads 2 grid files that contains the 2 components of a wind
 * field (cartesian or polar) and plots wind barbs at the grid positions.
 *
 * Author:	HIGAKI, Masakazu
 * Date:	1-JUN-2017
 * Version:	6 API
 */

#include "gmt_dev.h"
#include "windbarb.h"

#define THIS_MODULE_CLASSIC_NAME	"grdbarb"
#define THIS_MODULE_MODERN_NAME	"grdbarb"
#define THIS_MODULE_NAME	"grdbarb"
#define THIS_MODULE_LIB		"windbarb"
#define THIS_MODULE_PURPOSE	"Plot wind barb field from two component grids"
#define THIS_MODULE_KEYS	"<G{2,CC(,>X}"
#define THIS_MODULE_NEEDS	"gJ"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYfptxy" GMT_OPT("c")

EXTERN_MSC int GMT_grdbarb(void *API, int mode, void *args);

struct GRDBARB_CTRL {
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
		struct GMT_BARB_ATTR B;
	} Q;
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
	struct GRDBARB_CTRL *C = NULL;
	
	C = gmt_M_memory (GMT, NULL, 1, struct GRDBARB_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	gmt_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);
	C->W.pen = GMT->current.setting.map_default_pen;
	C->Q.B.width = 0.1f;	C->Q.B.length = 0.2f;	C->Q.B.angle = 120.0f;	C->Q.B.scale = 5.0f;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDBARB_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file[GMT_IN]);	
	gmt_M_str_free (C->In.file[GMT_OUT]);	
	gmt_M_str_free (C->C.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdbarb <gridx> <gridy> %s %s [-A] [%s]\n", GMT_J_OPT, GMT_Rgeo_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C[<cpt>]] [-G<fill>] [-I[x]<dx>/<dy>] [-K] [-N] [-O] [-P] [-Q[<len>][<attr>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T] [%s] [%s] [-W<pen>] [%s]\n\t[%s] [-Z] [%s]\n\t[%s] [%s]\n\n", 
		GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_f_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<gridx> <gridy> are grid files with the two wind components.\n");
	GMT_Option (API, "J-");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Grids have (speed, theta) wind components [Default is (u, v) components].\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Use CPT to assign colors based on wind speed. Optionally, instead give name\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of a master cpt to automatically assign 16 continuous colors over the data range [rainbow].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Yet another option is to specify -Ccolor1,color2[,color3,...] to build a linear\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   continuous cpt from those colors automatically.\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Select wind barb fill [Default is outlines only].");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Plot only those nodes that are <dx>/<dy> apart [Default is all nodes].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, use -Ix<fact>[/<yfact>] to give multiples of grid spacing.\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do Not clip wind barbs that exceed the map boundaries [Default will clip].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Modify wind barb attributes.\n");
	gmt_barb_syntax (API->GMT, 0);
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Transform angles for Cartesian grids when x- and y-scales differ [Leave alone].\n");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Set pen attributes.", NULL, 0);
	GMT_Message (API, GMT_TIME_NONE, "\t   Default pen attributes [%s].\n", gmt_putpen(API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z The theta grid provided has azimuths rather than directions (implies -A).\n");
	GMT_Option (API, "f,p,t,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDBARB_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdbarb and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	size_t len;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

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
				gmt_M_str_free (Ctrl->C.file);
				Ctrl->C.file = strdup (opt->arg);
				gmt_cpt_interval_modifier (GMT, &(Ctrl->C.file), &(Ctrl->C.dz));
				break;
			case 'E':	/* Center wind barbs [OBSOLETE; use modifier +jc in -Q ] */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -E is deprecated; use modifier +jc in -Q instead.\n");
					Ctrl->Q.B.status |= PSL_VEC_JUST_C;
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'G':	/* Set fill for wind barbs */
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
			case 'Q':	/* Set wind barb parameters */
				n_errors += gmt_parse_barb (GMT, opt->arg, &Ctrl->Q.B);
				break;
			case 'T':	/* Rescale Cartesian angles */
				Ctrl->T.active = true;
				break;
			case 'W':	/* Set line attributes */
				Ctrl->W.active = true;
				if (gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", NULL, 0);
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

	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[ISET] && (GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0),
	                                 "Syntax error -I option: Must specify positive increments\n");
	n_errors += gmt_M_check_condition (GMT, !(Ctrl->G.active || Ctrl->W.active || Ctrl->C.active),
	                                 "Syntax error: Must specify at least one of -G, -W, -C\n");
	n_errors += gmt_M_check_condition (GMT, n_files != 2, "Syntax error: Must specify two input grid files\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.cpt_effect && !Ctrl->C.active, "Syntax error -W: modifier +c only makes sense if -C is given\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdbarb (void *V_API, int mode, void *args) {
	unsigned int justify, row, col, col_0, row_0, d_col, d_row, k, n_warn[1] = {0}, warn;
	int error = 0;
	
	uint64_t ij;

	double tmp, x, y, plot_x, plot_y, x_off, y_off, f;
	double x2, y2, wesn[4], value, vec_length, vec_azim, c, s;

	struct GMT_GRID *Grid[2] = {NULL, NULL};
	struct GMT_PALETTE *P = NULL;
	struct GRDBARB_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;	/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);
	
	/*---------------------------- This is the grdbarb main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grids\n");
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
		GMT_Report (API, GMT_MSG_VERBOSE, "Warning: No data within specified region\n");
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
		else {	/* Find min/max wind barb lengths from the components */
			v_min = DBL_MAX;	v_max = 0.0;
			gmt_M_grd_loop (GMT, Grid[GMT_X], row, col, ij) {
				vec_length = hypot (Grid[GMT_X]->data[ij], Grid[GMT_Y]->data[ij]);
				if (vec_length < v_min) v_min = vec_length;
				if (vec_length > v_max) v_max = vec_length;
			}
		}
		if ((P = gmt_get_palette (GMT, Ctrl->C.file, GMT_CPT_OPTIONAL, v_min, v_max, Ctrl->C.dz)) == NULL) {
			Return (API->error);
		}
	}

	gmt_init_barb_param (GMT, &Ctrl->Q.B, true, Ctrl->W.active, &Ctrl->W.pen, Ctrl->G.active, &Ctrl->G.fill);
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
			GMT_Report (API, GMT_MSG_NORMAL, "Error: New y grid spacing (%.12lg) is not a multiple of actual grid spacing (%.12g) [within %g]\n", GMT->common.R.inc[GMT_Y], Grid[0]->header->inc[GMT_Y], GMT_CONV6_LIMIT);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT->common.R.inc[GMT_Y] = d_row * Grid[0]->header->inc[GMT_Y];	/* Get exact y-increment in case of slop */
		val = GMT->common.R.inc[GMT_X] * HH->r_inc[GMT_X];
		d_col = urint (val);
		if (d_col == 0 || fabs ((d_col - val)/d_col) > GMT_CONV6_LIMIT) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: New x grid spacing (%.12g) is not a multiple of actual grid spacing (%.12g) [within %g]\n", GMT->common.R.inc[GMT_X], Grid[0]->header->inc[GMT_X], GMT_CONV6_LIMIT);
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

	if (Ctrl->W.cpt_effect) {	/* Should color apply to pen, fill, or both [fill] */
		if ((Ctrl->W.pen.cptmode & 2) == 0 && !Ctrl->G.active)	/* Turn off CPT fill */
			gmt_M_rgb_copy (Ctrl->G.fill.rgb, GMT->session.no_rgb);
	}

	PSL_command (GMT->PSL, "V\n");
	for (row = row_0; row < Grid[1]->header->n_rows; row += d_row) {
		y = gmt_M_grd_row_to_y (GMT, row, Grid[0]->header);	/* Latitude OR y OR radius */
		for (col = col_0; col < Grid[1]->header->n_columns; col += d_col) {

			ij = gmt_M_ijp (Grid[0]->header, row, col);
			if (gmt_M_is_fnan (Grid[0]->data[ij]) || gmt_M_is_fnan (Grid[1]->data[ij])) continue;	/* Cannot plot NaN-barbs */
			x = gmt_M_grd_col_to_x (GMT, col, Grid[0]->header);	/* Longitude OR x OR theta [or azimuth] */
			if (!Ctrl->N.active) {	/* Throw out wind barbs whose node is outside */
				gmt_map_outside (GMT, x, y);
				if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
			}

			if (Ctrl->A.active) {	/* Got r,theta grids */
				vec_length = Grid[0]->data[ij];
				if (vec_length == 0.0) continue;	/* No length = no plotting */
				vec_azim = Grid[1]->data[ij];
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
				vec_azim = atan2d (-Grid[GMT_X]->data[ij], -Grid[GMT_Y]->data[ij]);	/* Convert dy,dx to azimuth */
				value = vec_length;
			}
			
			if (Ctrl->C.active) {	/* Get color based on the wind speed */
				gmt_get_fill_from_z (GMT, P, value, &Ctrl->G.fill);
			}
			if (Ctrl->W.cpt_effect) {	/* Should color apply to pen, fill, or both [fill] */
				if (Ctrl->W.pen.cptmode & 1)	/* Change pen color via CPT */
					gmt_M_rgb_copy (Ctrl->W.pen.rgb, Ctrl->G.fill.rgb);
			}
			if (Ctrl->C.active) {	/* Update pen and fill color settings */
				gmt_M_rgb_copy (Ctrl->W.pen.rgb, Ctrl->G.fill.rgb);
				gmt_setpen (GMT, &Ctrl->W.pen);
				gmt_setfill (GMT, &Ctrl->G.fill, Ctrl->W.active);
				gmt_init_barb_param (GMT, &Ctrl->Q.B, true, Ctrl->W.active, &Ctrl->W.pen, true, &Ctrl->G.fill);
			}

			gmt_geo_to_xy (GMT, x, y, &plot_x, &plot_y);
			if (Ctrl->T.active)	/* Deal with negative scales in x and/or y which affect the azimuths */
				gmt_flip_azim_d (GMT, &vec_azim);
			if (!gmt_M_is_geographic (GMT, GMT_IN))	/* Cartesian angle; change to azimuth */
				vec_azim = 90.0 - vec_azim;
			else	/* Convert geo azimuth to map direction */
				vec_azim = gmt_azim_to_angle (GMT, x, y, 0.1, vec_azim);
			if (GMT->current.proj.projection == GMT_POLAR) {	/* Must rotate azimuth since circular projection */
				double x_orient;
				x_orient = (GMT->current.proj.got_azimuths) ? -(x + GMT->current.proj.p_base_angle) : x - GMT->current.proj.p_base_angle - 90.0;
				vec_azim += x_orient;
			}
			sincosd (vec_azim, &s, &c);
			x2 = plot_x + Ctrl->Q.B.length * c;
			y2 = plot_y + Ctrl->Q.B.length * s;

			justify = PSL_vec_justify (Ctrl->Q.B.status);	/* Return justification as 0-2 */
			if (justify) {	/* Justify wind barb at center, or tip [beginning] */
				x_off = justify * 0.5 * (x2 - plot_x);	y_off = justify * 0.5 * (y2 - plot_y);
				plot_x -= x_off;	plot_y -= y_off;
				x2 -= x_off;		y2 -= y_off;
			}
			n_warn[0]++;
			gmt_draw_barb (GMT, plot_x, plot_y, y, vec_azim, vec_length, Ctrl->Q.B, &Ctrl->W.pen, &Ctrl->G.fill, Ctrl->W.active);
		}
	}
	PSL_command (GMT->PSL, "U\n");
	PSL->current.linewidth = 0.0;	/* Since we changed things under clip; this will force it to be set next */

	if (!Ctrl->N.active) gmt_map_clip_off (GMT);

	gmt_map_basemap (GMT);

	gmt_plane_perspective (GMT, -1, 0.0);

	gmt_plotend (GMT);
	
	GMT_Report (API, GMT_MSG_VERBOSE, "%d wind barbs plotted successfully\n", n_warn[0]);

	Return (GMT_NOERROR);
}
