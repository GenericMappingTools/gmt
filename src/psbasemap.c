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
 * API functions to support the psbasemap application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: psbasemap plots a basemap for the given area using
 * the specified map projection.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psbasemap"
#define THIS_MODULE_MODERN_NAME	"basemap"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot base maps and frames"
#define THIS_MODULE_KEYS	">X},>DA@AD)"
#define THIS_MODULE_NEEDS	"JR"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYfptxy" GMT_OPT("EZc")

/* Control structure for psbasemap */

struct PSBASEMAP_CTRL {
	struct A {	/* -A */
		bool active;
		char *file;
	} A;
	struct D {	/* -D[g|j|n|x]<refpoint>+w<width>[<unit>][/<height>[<unit>]][+j<justify>[+o<dx>[/<dy>]][+s<file>][+t] or <xmin>/<xmax>/<ymin>/<ymax>[+r][+s<file>][+t][+u<unit>] */
		bool active;
		struct GMT_MAP_INSET inset;
	} D;
	struct F {	/* -F[d|f|l][+c<clearance>][+g<fill>][+i[<off>/][<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;
		/* The panels are members of GMT_MAP_SCALE, GMT_MAP_ROSE, and GMT_MAP_INSET */
	} F;
	struct L {	/* -L[g|j|n|x]<refpoint>+c[<slon>/]<slat>+w<length>[e|f|M|n|k|u][+a<align>][+f][+l[<label>]][+u] */
		bool active;
		struct GMT_MAP_SCALE scale;
	} L;
	struct T {	/* -Td|m<params> */
		bool active;
		struct GMT_MAP_ROSE rose;
	} T;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSBASEMAP_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSBASEMAP_CTRL);
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSBASEMAP_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->A.file);
	if (C->D.inset.refpoint) gmt_free_refpoint (GMT, &C->D.inset.refpoint);
	gmt_M_free (GMT, C->D.inset.panel);
	if (C->L.scale.refpoint) gmt_free_refpoint (GMT, &C->L.scale.refpoint);
	gmt_M_free (GMT, C->L.scale.panel);
	if (C->T.rose.refpoint) gmt_free_refpoint (GMT, &C->T.rose.refpoint);
	gmt_M_free (GMT, C->T.rose.panel);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psbasemap synopsis and optionally full usage information */
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	bool classic = (API->GMT->current.setting.run_mode == GMT_CLASSIC);
	
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s %s %s [%s]\n", name, GMT_J_OPT, GMT_Rgeoz_OPT, GMT_B_OPT);
	if (classic) {
		GMT_Message (API, GMT_TIME_NONE, "\t[-A[<file>]] [-D%s] |\n\t[-D%s]\n", GMT_INSET_A_CL, GMT_INSET_B_CL);
		GMT_Message (API, GMT_TIME_NONE, "\t[-F[d|l|t]%s] %s\n", GMT_PANEL, API->K_OPT);
	}
	else {
		GMT_Message (API, GMT_TIME_NONE, "\t[-A[<file>]] [-F[l|t]%s]\n", GMT_PANEL);
		GMT_Message (API, GMT_TIME_NONE, "\t%s\n", API->K_OPT);
	}
	GMT_Message (API, GMT_TIME_NONE, "\t[-L%s]\n", GMT_SCALE);
	GMT_Message (API, GMT_TIME_NONE, "\t%s%s[-Td%s]\n", API->O_OPT, API->P_OPT, GMT_TROSE_DIR);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Tm%s]\n", GMT_TROSE_MAG);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s] %s[%s]\n\t[%s] [%s] [%s]\n\n", GMT_U_OPT, GMT_V_OPT,
		GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_f_OPT, GMT_p_OPT, GMT_t_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "JZ,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A No plotting, just write coordinates of the (possibly oblique) rectangular map boundary\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to given file (or stdout).  Requires -R and -J only.  Spacing along border\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   in projected coordinates is controlled by GMT default MAP_LINE_STEP [%gp].\n",
		API->GMT->session.u2u[GMT_INCH][GMT_PT] * API->GMT->current.setting.map_line_step);
	GMT_Option (API, "B");
	if (classic) {
		gmt_mapinset_syntax (API->GMT, 'D', "Draw a simple map inset box as specified below:");
		gmt_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the map inset, scale or rose.", 3);
		GMT_Message (API, GMT_TIME_NONE, "\t   For separate panel attributes, use -Fd, -Fl, -Ft.\n");
	}
	else {
		gmt_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the map scale or rose.", 3);
		GMT_Message (API, GMT_TIME_NONE, "\t   For separate panel attributes, use -Fl, -Ft.\n");
	}
	GMT_Option (API, "K");
	gmt_mapscale_syntax (API->GMT, 'L', "Draw a map scale at specified reference point.");
	GMT_Option (API, "O,P");
	gmt_maprose_syntax (API->GMT, 'T', "Draw a north-pointing map rose at specified reference point.");
	GMT_Option (API, "U,V,X,c,f,p,t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSBASEMAP_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psbasemap and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k = 1;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	char *kind[3] = {"Specify a rectangular panel for the map inset", "Specify a rectangular panel behind the map scale", "Specify a rectangular panel behind the map rose"};
	bool get_panel[3] = {false, false, false}, classic;
	
	classic = (GMT->current.setting.run_mode == GMT_CLASSIC);
	
	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			/* Processes program-specific parameters */

			case 'A':	/* No plot, just output region outline  */
				Ctrl->A.active = true;
				if (opt->arg[0]) Ctrl->A.file = strdup (opt->arg);
				break;
			case 'D':	/* Draw map inset */
				if (classic) {
					Ctrl->D.active = true;
					n_errors += gmt_getinset (GMT, 'D', opt->arg, &Ctrl->D.inset);
				}
				else {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -D is not available in modern mode - see inset instead\n");
					n_errors++;
				}
				break;
			case 'F':
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'd': get_panel[0] = true; break;
					case 'l': get_panel[1] = true; break;
					case 't': get_panel[2] = true; break;
					default : get_panel[1] = get_panel[2] = true; k = 0;
						get_panel[0] = classic; break;
				}
				if (get_panel[0] && classic && gmt_getpanel (GMT, opt->option, &opt->arg[k], &(Ctrl->D.inset.panel))) {
					gmt_mappanel_syntax (GMT, 'F', kind[0], 3);
					n_errors++;
				}
				else if (!classic && get_panel[0]) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -Fd is not available in modern mode - see inset instead\n");
					n_errors++;
				}
				if (get_panel[1] && gmt_getpanel (GMT, opt->option, &opt->arg[k], &(Ctrl->L.scale.panel))) {
					gmt_mappanel_syntax (GMT, 'F', kind[0], 3);
					n_errors++;
				}
				if (get_panel[2] && gmt_getpanel (GMT, opt->option, &opt->arg[k], &(Ctrl->T.rose.panel))) {
					gmt_mappanel_syntax (GMT, 'F', kind[1], 3);
					n_errors++;
				}
				break;
			case 'G':	/* Set canvas color */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -G is deprecated; -B...+g%s was set instead, use this in the future.\n", opt->arg);
					GMT->current.map.frame.paint = true;
					if (gmt_getfill (GMT, opt->arg, &GMT->current.map.frame.fill)) {
						gmt_fill_syntax (GMT, 'G', NULL, " ");
						n_errors++;
					}
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'L':	/* Draw map scale */
				Ctrl->L.active = true;
				n_errors += gmt_getscale (GMT, 'L', opt->arg, GMT_SCALE_MAP, &Ctrl->L.scale);
				break;
			case 'T':	/* Draw map rose */
				Ctrl->T.active = true;
				n_errors += gmt_getrose (GMT, 'T', opt->arg, &Ctrl->T.rose);
				break;
#ifdef DEBUG
			case '+':	/* Draw a single gridline only [for debugging]  -+x|y<value>*/
				GMT->hidden.gridline_debug = true;
				GMT->hidden.gridline_kind = opt->arg[0];	/* Get x or y */
				GMT->hidden.gridline_val = atof (&opt->arg[1]);	/* Value of grid line */
				break;
#endif
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && Ctrl->L.scale.old_style, "Syntax error: Cannot specify -F and use old-style -L settings\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !(GMT->current.map.frame.init || Ctrl->A.active || Ctrl->D.active || Ctrl->L.active || Ctrl->T.active), "Syntax error: Must specify at least one of -A, -B, -D, -L, -T\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && (GMT->current.map.frame.init || Ctrl->D.active || Ctrl->L.active || Ctrl->T.active), "Syntax error: Cannot use -B, -D, -L, -T with -A\n");
	//n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && gmt_M_is_cartesian (GMT, GMT_IN), "Syntax error: -L applies to geographical data only\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && !(Ctrl->D.active || Ctrl->L.active || Ctrl->T.active), "Syntax error: -F is only allowed with -L and -T\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout(code);}

int GMT_basemap (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: basemap\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psbasemap (V_API, mode, args);
}

int GMT_psbasemap (void *V_API, int mode, void *args) {
	/* High-level function that implements the psbasemap task */
	int error;
	
	struct PSBASEMAP_CTRL *Ctrl = NULL;	/* Control structure specific to program */
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psbasemap main code ----------------------------*/

	/* Ready to make the plot */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Constructing the basemap\n");

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);
	
	if (Ctrl->A.active) {	/* Just save outline in geographic coordinates */
		/* Loop counter-clockwise around the rectangular projected domain, recovering the lon/lat points */
		uint64_t nx, ny, k = 0, i, dim[GMT_DIM_SIZE] = {1, 1, 0, 2};
		char msg[GMT_BUFSIZ] = {""}, *kind[2] = {"regular", "oblique"};
		struct GMT_DATASET *D = NULL;
		struct GMT_DATASEGMENT *S = NULL;
		
		nx = urint (GMT->current.map.width  / GMT->current.setting.map_line_step);
		ny = urint (GMT->current.map.height / GMT->current.setting.map_line_step);
		dim[GMT_ROW] = 2 * (nx + ny) + 1;
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Constructing coordinates of the plot domain outline polygon using %" PRIu64 " points\n", dim[GMT_ROW]);
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLYGON, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
		S = D->table[0]->segment[0];
		/* March around perimeter in a counter-clockwise sense */
		for (i = 0; i < nx; i++, k++) gmt_xy_to_geo (GMT, &S->data[GMT_X][k], &S->data[GMT_Y][k], i * GMT->current.setting.map_line_step, 0.0);
		for (i = 0; i < ny; i++, k++) gmt_xy_to_geo (GMT, &S->data[GMT_X][k], &S->data[GMT_Y][k], GMT->current.map.width, i * GMT->current.setting.map_line_step);
		for (i = nx; i > 0; i--, k++) gmt_xy_to_geo (GMT, &S->data[GMT_X][k], &S->data[GMT_Y][k], i * GMT->current.setting.map_line_step, GMT->current.map.height);
		for (i = ny; i > 0; i--, k++) gmt_xy_to_geo (GMT, &S->data[GMT_X][k], &S->data[GMT_Y][k], 0.0, i * GMT->current.setting.map_line_step);
		S->data[GMT_X][k] = S->data[GMT_X][0];	S->data[GMT_Y][k++] = S->data[GMT_Y][0];	/* Close polygon */
		S->n_rows = dim[GMT_ROW];
		D->table[0]->n_headers = 1;
		D->table[0]->header = gmt_M_memory (GMT, NULL, 1U, char *);
		sprintf (msg, " Geographical coordinates for a (%s) rectangular plot domain outline polygon", kind[GMT->common.R.oblique]);
		D->table[0]->header[0] = strdup (msg);
		GMT->current.setting.io_header[GMT_OUT] = true;	/* Turn on table headers on output */
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLYGON, GMT_WRITE_SET, NULL, Ctrl->A.file, D) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}

	/* Regular plot behaviour */
	
	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);

	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	gmt_plotcanvas (GMT);	/* Fill canvas if requested */

	gmt_map_basemap (GMT);	/* Plot base map */

	if (Ctrl->L.active) {
		if (Ctrl->L.scale.vertical)
			gmt_draw_vertical_scale (GMT, &Ctrl->L.scale);
		else
			gmt_draw_map_scale (GMT, &Ctrl->L.scale);
	}
	if (Ctrl->T.active) gmt_draw_map_rose (GMT, &Ctrl->T.rose);
	if (Ctrl->D.active) gmt_draw_map_inset (GMT, &Ctrl->D.inset, false);

	gmt_plane_perspective (GMT, -1, 0.0);

	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
