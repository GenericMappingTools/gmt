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
 * Brief synopsis: gmtlogo plots a GMT logo on a map.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2015
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtlogo"
#define THIS_MODULE_MODERN_NAME	"gmtlogo"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot the GMT logo"
#define THIS_MODULE_KEYS	">X}"
#define THIS_MODULE_NEEDS	"jr"
#define THIS_MODULE_OPTIONS "->KJOPRUVXYtxy" GMT_OPT("c")

#define GMT_N_LETTERS	116
static float gmt_letters[GMT_N_LETTERS][2] = {	/* The G, M, T polygons */
	{ NAN, NAN },
	{ -59.5, -41.8 },
	{ -59.5, 2.2 },
	{ -105, 2.2 },
	{ -105, -7.7 },
	{ -72.8, -7.7 },
	{ -72.94, -11 },
	{ -73.24, -13.75 },
	{ -73.65, -16.5 },
	{ -74.5, -19.5 },
	{ -75.88, -22 },
	{ -77.65, -24.5 },
	{ -80.7, -27.5 },
	{ -84, -29.7 },
	{ -87.1, -31.28 },
	{ -91, -32.78 },
	{ -98, -34.1 },
	{ -105, -34.87 },
	{ -112, -34.3 },
	{ -119, -33 },
	{ -122.5, -31.8 },
	{ -126, -30.25 },
	{ -129.5, -27.5 },
	{ -133, -24.64 },
	{ -135.8, -22 },
	{ -137.55, -19.4 },
	{ -138.46, -16.5 },
	{ -139.5, -11 },
	{ -140, -5.5 },
	{ -140.14, 0 },
	{ -139.86, 5.5 },
	{ -138.88, 11 },
	{ -136.36, 16.5 },
	{ -133, 20.9 },
	{ -126, 27.28 },
	{ -119, 30.36 },
	{ -112, 32.12 },
	{ -105, 32.78 },
	{ -100, 32.45 },
	{ -94.3, 31.15 },
	{ -91, 30.14 },
	{ -87.55, 28.89 },
	{ -84.45, 27.39 },
	{ -82.12, 25.9 },
	{ -80.12, 24.1 },
	{ -78.22, 21.9 },
	{ -77.1, 19.8 },
	{ -76.55, 17.52 },
	{ -76.3, 15.18 },
	{ -61.6, 15.18 },
	{ -62, 18.75 },
	{ -63.1, 21.89 },
	{ -64.75, 24.83 },
	{ -66.8, 27.5 },
	{ -70, 30.58 },
	{ -73, 33 },
	{ -77, 35.65 },
	{ -84, 38.72 },
	{ -91, 40.7 },
	{ -98, 41.8 },
	{ -105, 42.13 },
	{ -112, 41.8 },
	{ -119, 41.14 },
	{ -126, 39.6 },
	{ -133, 35.97 },
	{ -137.2, 33 },
	{ -143.78, 27.5 },
	{ -147, 24.2 },
	{ -151.2, 16.5 },
	{ -153.02, 11 },
	{ -154.28, 5.5 },
	{ -154.8, 0 },
	{ -154.63, -5.5 },
	{ -153.85, -11 },
	{ -152.32, -16.5 },
	{ -150.1, -22 },
	{ -147, -26.95 },
	{ -141.4, -33 },
	{ -133, -38.5 },
	{ -126, -41.58 },
	{ -119, -43.01 },
	{ -112, -44 },
	{ -105, -44.33 },
	{ -98, -43.78 },
	{ -91, -42.35 },
	{ -84.5, -40.37 },
	{ -80, -38.2 },
	{ -76.5, -36 },
	{ -74.65, -34.3 },
	{ -73.5, -33 },
	{ -72.7, -32 },
	{ -71.96, -30.8 },
	{ -68.04, -41.8 },
	{ NAN, NAN },
	{ -37.8, -41.8 },
	{ -23.8, -41.8 },
	{ -23.8, 27.39 },
	{ 5.04, -41.8 },
	{ 19.46, -41.8 },
	{ 49, 27.39 },
	{ 49, -41.8 },
	{ 62.58, -41.8 },
	{ 62.58, 40.48 },
	{ 42, 40.48 },
	{ 12.46, -29.7 },
	{ -17.22, 40.48 },
	{ -37.8, 40.48 },
	{ NAN, NAN },
	{ 124.46, -41.8 },
	{ 124.46, 30.47 },
	{ 160.02, 30.47 },
	{ 160.02, 40.48 },
	{ 74.34, 40.48 },
	{ 74.34, 30.47 },
	{ 110.32, 30.47 },
	{ 110.32, -41.8 }
};

/* Specific colors for fonts, land, water, text etc */

#define c_font		"51/51/51"
#define c_grid		"150/160/163"
#define c_land		"125/168/125"
#define c_water		"189/224/223"
#define c_gmt_fill	"238/86/52"
#define c_gmt_outline	"112/112/112"
#define c_gmt_shadow	"92/102/132"

/* Control structure for gmtlogo */

struct GMTLOGO_CTRL {
	struct D {	/* -D[g|j|n|x]<refpoint>+w<width>[+j<justify>][+o<dx>[/<dy>]] */
		bool active;
		struct GMT_REFPOINT *refpoint;
		double off[2];
		double width;
		int justify;
	} D;
	struct F {	/* -F[+c<clearance>][+g<fill>][+i[<off>/][<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;
		struct GMT_MAP_PANEL *panel;
	} F;
	struct S {	/* -S means only plot logo and not title below */
		bool active;
		unsigned int mode;	/* 0 = draw label, 1 draw URL, 2 no label */
	} S;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTLOGO_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTLOGO_CTRL);
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTLOGO_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_free_refpoint (GMT, &C->D.refpoint);
	gmt_M_free (GMT, C->F.panel);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the gmtlogo synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [-D%s[+w<width>]%s]\n", name, GMT_XYANCHOR, GMT_OFFSET);
	GMT_Message (API, GMT_TIME_NONE, "\t[-F%s]\n\t[%s] %s%s%s[%s]\n", GMT_PANEL, GMT_J_OPT, API->K_OPT, API->O_OPT, API->P_OPT, GMT_Rgeoz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-S[l|n|u]] [%s] [%s]\n\t[%s] [%s] %s[%s] [%s]\n\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_t_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	gmt_refpoint_syntax (API->GMT, "D", "Specify position of the GMT logo [0/0].", GMT_ANCHOR_LOGO, 1);
	gmt_refpoint_syntax (API->GMT, "D", NULL, GMT_ANCHOR_LOGO, 2);
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +w<width> to set the width of the GMT logo. [144p]\n");
	gmt_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the GMT logo.", 0);
	GMT_Option (API, "J-Z,K,O,P,R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Control text label plotted beneath the logo:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append l to plot text label [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append u to plot URL for GMT.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append n to skip label entirely.\n");
	GMT_Option (API, "U,V");
	GMT_Option (API, "X,c,f,t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTLOGO_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtlogo and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	int n;
	char string[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			/* Processes program-specific parameters */

			case 'D':
				Ctrl->D.active = true;
				if ((Ctrl->D.refpoint = gmt_get_refpoint (GMT, opt->arg, 'D')) == NULL)
					n_errors++;	/* Failed basic parsing */
				else {	/* args are [+w<width>][+j<justify>][+o<dx>[/<dy>]] */
					if (gmt_validate_modifiers (GMT, Ctrl->D.refpoint->args, 'D', "jow")) n_errors++;
					if (gmt_get_modifier (Ctrl->D.refpoint->args, 'j', string))
						Ctrl->D.justify = gmt_just_decode (GMT, string, PSL_NO_DEF);
					else	/* With -Dj or -DJ, set default to reference justify point, else BL */
						Ctrl->D.justify = gmt_M_just_default (GMT, Ctrl->D.refpoint, PSL_BL);
					if (gmt_get_modifier (Ctrl->D.refpoint->args, 'o', string)) {
						if ((n = gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->D.off)) < 0) n_errors++;
					}
					if (gmt_get_modifier (Ctrl->D.refpoint->args, 'w', string))	/* Get logo width */
						Ctrl->D.width = gmt_M_to_inch (GMT, string);
				}
				break;
			case 'F':
				Ctrl->F.active = true;
				if (gmt_getpanel (GMT, opt->option, opt->arg, &(Ctrl->F.panel))) {
					gmt_mappanel_syntax (GMT, 'F', "Specify a rectangular panel behind the logo.", 0);
					n_errors++;
				}
				break;
			case 'S':
				Ctrl->S.active = true;
				switch (opt->arg[0]) {
					case 'l': Ctrl->S.mode = 0;	break;	/* Label */
					case 'u': Ctrl->S.mode = 1;	break;	/* URL */
					case 'n': Ctrl->S.mode = 2;	break;	/* URL */
					default:  Ctrl->S.mode = 0;	break;	/* Label */
				}
				break;
			case 'W':	/* Scale for the logo */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Option -W is deprecated; -D...+w%s was set instead, use this in the future.\n", opt->arg);
				Ctrl->D.width = gmt_M_to_inch (GMT, opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	if (!Ctrl->D.active) {	/* Default to -Dx0/0+w2i if no -D given */
		if ((Ctrl->D.refpoint = gmt_get_refpoint (GMT, "x0/0+w2i", 'D')) == NULL)	/* Cannot happen but Coverity thinks so */
			n_errors++;
		else if (gmt_get_modifier (Ctrl->D.refpoint->args, 'w', string))	/* Get logo width */
			Ctrl->D.width = gmt_M_to_inch (GMT, string);
		Ctrl->D.active = true;
	}
	if (Ctrl->D.width == 0.0) Ctrl->D.width = 2.0;	/* Default width */
	if (Ctrl->D.refpoint && Ctrl->D.refpoint->mode != GMT_REFPOINT_PLOT) {	/* Anything other than -Dx need -R -J; other cases don't */
		static char *kind = "gjJnx";	/* The five types of refpoint specifications */
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Option -D%c requires the -R option\n", kind[Ctrl->D.refpoint->mode]);
		n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Option -D%c requires the -J option\n", kind[Ctrl->D.refpoint->mode]);
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.width < 0.0, "Option -D+w modifier: Width cannot be zero or negative!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout(code);}

int GMT_gmtlogo (void *V_API, int mode, void *args) {
	/* High-level function that implements the gmtlogo task */
	int error, fmode;

	uint64_t par[4] = {0, 0, 0, 0};

	double wesn[4] = {0.0, 0.0, 0.0, 0.0};	/* Dimensions in inches */
	double scale, y, dim[2];

	char cmd[GMT_LEN256] = {""}, pars[GMT_LEN128] = {""}, file[GMT_VF_LEN] = {""};

	struct GMT_FONT F;
	struct GMT_MATRIX *M = NULL;
	struct GMTLOGO_CTRL *Ctrl = NULL;	/* Control structure specific to program */
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

	/*---------------------------- This is the gmtlogo main code ----------------------------*/

	/* Ready to make the plot */

	GMT_Report (API, GMT_MSG_INFORMATION, "Constructing the GMT logo\n");

	/* The following is needed to have gmtlogo work correctly in perspective */

	gmt_M_memset (wesn, 4, double);
	dim[GMT_X] = Ctrl->D.width, dim[GMT_Y] = (Ctrl->S.mode == 2) ? 1.55 * 0.25 * Ctrl->D.width : 0.5 * Ctrl->D.width; /* Height is 0.5 * width unless when text is deactivated */
	if (!(GMT->common.R.active[RSET] && GMT->common.J.active)) {	/* When no projection specified, use fake linear projection */
		GMT->common.R.active[RSET] = true;
		GMT->common.J.active = false;
		gmt_parse_common_options (GMT, "J", 'J', "X1i");
		gmt_set_refpoint (GMT, Ctrl->D.refpoint);	/* Finalize reference point plot coordinates, if needed */
		gmt_adjust_refpoint (GMT, Ctrl->D.refpoint, dim, Ctrl->D.off, Ctrl->D.justify, PSL_BL);	/* Adjust refpoint to BL corner */
		wesn[XHI] = Ctrl->D.refpoint->x + Ctrl->D.width;	wesn[YHI] = Ctrl->D.refpoint->y + 0.5 * Ctrl->D.width;
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, wesn), "")) Return (GMT_PROJECTION_ERROR);
		PSL = gmt_plotinit (GMT, options);
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	}
	else {	/* First use current projection, project, then use fake projection */
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);
		gmt_set_refpoint (GMT, Ctrl->D.refpoint);	/* Finalize reference point plot coordinates, if needed */
		gmt_adjust_refpoint (GMT, Ctrl->D.refpoint, dim, Ctrl->D.off, Ctrl->D.justify, PSL_BL);	/* Adjust to BL corner */
		PSL = gmt_plotinit (GMT, options);
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT->common.J.active = false;
		gmt_parse_common_options (GMT, "J", 'J', "X1i");
		wesn[XLO] = Ctrl->D.refpoint->x;	wesn[YLO] = Ctrl->D.refpoint->y;
		wesn[XHI] = Ctrl->D.refpoint->x + Ctrl->D.width;	wesn[YHI] = Ctrl->D.refpoint->y + 0.5 * Ctrl->D.width;
		GMT->common.R.active[RSET] = GMT->common.J.active = true;
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, wesn), "")) Return (GMT_PROJECTION_ERROR);
	}

	PSL_command (PSL, "V\n");	/* Ensure the entire gmtlogo output after initialization is between gsave/grestore */
	PSL_setorigin (PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->y, 0.0, PSL_FWD);

	/* Set up linear projection with logo domain and user width */
	scale = dim[GMT_X] / 2.0;	/* Scale relative to default size 2 inches */
	if (Ctrl->F.active) {	/* First place legend frame fill */
		Ctrl->F.panel->width = dim[GMT_X];	Ctrl->F.panel->height = dim[GMT_Y];
		gmt_draw_map_panel (GMT, 0.5 * dim[GMT_X], 0.5 * dim[GMT_Y], 3U, Ctrl->F.panel);
	}

	/* Plot the title beneath the map with 1.5 vertical stretching */

	if (Ctrl->S.mode == 2)
		y = 0.0;
	else {
		sprintf (cmd, "%g,AvantGarde-Demi,%s", scale * 9.5, c_font);	/* Create required font */
		gmt_getfont (GMT, cmd, &F);
		fmode = gmt_setfont (GMT, &F);
		PSL_setfont (PSL, F.id);
		PSL_command (PSL, "V 1 1.5 scale\n");
		if (Ctrl->S.mode == 0)
			PSL_plottext (PSL, 0.5 * dim[GMT_X], 0.027 * scale, F.size, "@#THE@# G@#ENERIC@# M@#APPING@# T@#OOLS@#", 0.0, PSL_BC, fmode);
		else
			PSL_plottext (PSL, 0.5 * dim[GMT_X], 0.027 * scale, F.size, "G@#ENERIC@#-M@#APPING@#-T@#OOLS@#.@#ORG@#", 0.0, PSL_BC, fmode);
		PSL_command (PSL, "U\n");
		y = scale * 0.220;
	}

	/* Plot the globe via GMT_psclip & GMT_pscoast */

	snprintf (pars, GMT_LEN128, "--MAP_GRID_PEN=faint,%s --MAP_FRAME_PEN=%gp,%s --GMT_HISTORY=false", c_grid, scale * 0.3, c_grid);
	sprintf (cmd, "-T -Rd -JI0/%gi -N -O -K -X%gi -Y%gi %s", scale * 1.55, scale * 0.225, y, pars);
	GMT_Report (API, GMT_MSG_INFORMATION, "Calling psclip with args %s\n", cmd);
	GMT_Call_Module (API, "psclip", GMT_MODULE_CMD, cmd);
	sprintf (cmd, "-Rd -JI0/%gi -S%s -G%s -A35000+l -Dc -O -K %s --GMT_HISTORY=false", scale * 1.55, c_water, c_land, pars);
	GMT_Report (API, GMT_MSG_INFORMATION, "Calling pscoast with args %s\n", cmd);
	GMT_Call_Module (API, "pscoast", GMT_MODULE_CMD, cmd);
	sprintf (cmd, "-Rd -JI0/%gi -C -O -K -Bxg45 -Byg30  %s --MAP_POLAR_CAP=none --GMT_HISTORY=false", scale * 1.55, pars);
	GMT_Report (API, GMT_MSG_INFORMATION, "Calling psclip with args %s\n", cmd);
	GMT_Call_Module (API, "psclip", GMT_MODULE_CMD, cmd);

	/* Plot the GMT letters as shadows, then full size, using GMT_psxy */

	/* Allocate a matrix container for holding the GMT-matrix coordinates */
	par[0] = 2;	par[1] = GMT_N_LETTERS;
	if ((M = GMT_Create_Data (API, GMT_IS_DATASET|GMT_VIA_MATRIX, GMT_IS_POLY, GMT_CONTAINER_ONLY, par, NULL, NULL, 0, GMT_IS_ROW_FORMAT, NULL)) == NULL)
		exit (EXIT_FAILURE);
	GMT_Put_Matrix (API, M, GMT_FLOAT, 0, gmt_letters);	/* Hook in our static float matrix */
	GMT_Open_VirtualFile (API, GMT_IS_DATASET|GMT_VIA_MATRIX, GMT_IS_POLY, GMT_IN, M, file);	/* Open matrix for reading */
	sprintf (cmd, "-<%s -R167/527/-90/90 -JI-13/%gi -O -K -G%s@40 --GMT_HISTORY=false",
		file, scale * 1.55, c_gmt_shadow);
	GMT_Report (API, GMT_MSG_INFORMATION, "Calling psxy with args %s\n", cmd);
	GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd);
	GMT_Init_VirtualFile (API, 0, file);	/* Reset since we are reading it a 2nd time */
	sprintf (cmd, "-<%s -R167/527/-90/90 -JI-13/%gi -O -K -G%s -W%gp,%s -X-%gi -Y-%gi --GMT_HISTORY=false",
		file, scale * 2.47, c_gmt_fill, scale * 0.3, c_gmt_outline, scale * 0.483, scale * 0.230);
	GMT_Report (API, GMT_MSG_INFORMATION, "Calling psxy with args %s\n", cmd);
	GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd);
	GMT_Close_VirtualFile (API, file);

	PSL_setorigin (PSL, -Ctrl->D.refpoint->x, -Ctrl->D.refpoint->y, 0.0, PSL_INV);
	gmt_plane_perspective (GMT, -1, 0.0);
	PSL_command (PSL, "U\n");	/* Ending the encapsulation for gmtlogo */

	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
