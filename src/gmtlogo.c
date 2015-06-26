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
 * Brief synopsis: gmtlogo plots a GMT logo on a map.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2015
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"gmtlogo"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot the GMT logo on maps"
#define THIS_MODULE_KEYS	""

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->KJOPRUVXYctxy"

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
	struct D {	/* -D[g|j|n|x]<refpoint>[+j<justify>][+o<off[GMT_X]>[/<dy>]] */
		bool active;
		struct GMT_REFPOINT *refpoint;
		double off[2];
		int justify;
	} D;
	struct W {	/* -W<width> */
		bool active;
		double width;
	} W;
	struct F {	/* -F[+c<clearance>][+g<fill>][+i[<off>/][<pen>]][+p[<pen>]][+r[<radius>]][+s[<off[GMT_X]>/<dy>/][<shade>]][+d] */
		bool active;
		struct GMT_MAP_PANEL *panel;
	} F;
};

void *New_gmtlogo_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTLOGO_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GMTLOGO_CTRL);
	C->D.justify = PSL_BL;
	C->W.width = 2.0;	/* Default width is 2" */
	return (C);
}

void Free_gmtlogo_Ctrl (struct GMT_CTRL *GMT, struct GMTLOGO_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free_refpoint (GMT, &C->D.refpoint);
	if (C->F.panel) GMT_free (GMT, C->F.panel);
	GMT_free (GMT, C);
}

int GMT_gmtlogo_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the gmtlogo synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtlogo [%s%s]\n", GMT_XYANCHOR, GMT_OFFSET);
	GMT_Message (API, GMT_TIME_NONE, "[%s] [%s] [%s] [-K]\n", GMT_PANEL, GMT_J_OPT, GMT_Jz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t [-O] [-P] [%s] [-W<width>] [%s] [%s] [%s] [%s]\n\n", GMT_Rgeoz_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_refpoint_syntax (API->GMT, 'D', "Specify position of the GMT logo [0/0]", GMT_ANCHOR_LOGO, 1);
	GMT_refpoint_syntax (API->GMT, 'D', "BL", GMT_ANCHOR_LOGO, 2);
	GMT_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the GMT logo", 0);
	GMT_Option (API, "J-Z,K,O,P,R");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set width of the GMT logo [2 inches].\n");
	GMT_Option (API, "U,V");
	GMT_Option (API, "X,c,f,t,.");

	return (EXIT_FAILURE);
}

int GMT_gmtlogo_parse (struct GMT_CTRL *GMT, struct GMTLOGO_CTRL *Ctrl, struct GMT_OPTION *options)
{
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
				if ((Ctrl->D.refpoint = GMT_get_refpoint (GMT, opt->arg)) == NULL) n_errors++;	/* Failed basic parsing */
				else {	/* args are [+j<justify>][+o<dx>[/<dy>]] */
					if (GMT_get_modifier (Ctrl->D.refpoint->args, 'j', string))
						Ctrl->D.justify = GMT_just_decode (GMT, string, PSL_NO_DEF);
					else if (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST)	/* For -Dj with no 2nd justification, use same code as reference point coordinate as default */
						Ctrl->D.justify = Ctrl->D.refpoint->justify;
					if (GMT_get_modifier (Ctrl->D.refpoint->args, 'o', string)) {
						if ((n = GMT_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->D.off)) < 0) n_errors++;
					}
				}
				break;
			case 'F':
				Ctrl->F.active = true;
				if (GMT_getpanel (GMT, opt->option, opt->arg, &(Ctrl->F.panel))) {
					GMT_mappanel_syntax (GMT, 'F', "Specify a rectangular panel behind the logo", 0);
					n_errors++;
				}
				break;
			case 'W':	/* Scale for the logo */
				Ctrl->W.active = true;
				Ctrl->W.width = GMT_to_inch (GMT, opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	if (!Ctrl->D.active) {
		Ctrl->D.refpoint = GMT_get_refpoint (GMT, "x0/0");	/* Default if no -D given */
		Ctrl->D.active = true;
	}
	n_errors += GMT_check_condition (GMT, Ctrl->W.width < 0.0, "Syntax error -W option: Width cannot be zero or negative!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtlogo_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout(code);}

int GMT_gmtlogo (void *V_API, int mode, void *args)
{	/* High-level function that implements the gmtlogo task */
	int error, fmode;
	
	double wesn[4] = {0.0, 0.0, 0.0, 0.0};	/* Dimensions in inches */
	double scale, plot_x, plot_y;
	
	char cmd[GMT_BUFSIZ] = {""}, pars[GMT_LEN128] = {""}, file[GMT_BUFSIZ] = {""};
	
	struct GMT_FONT F;
	struct GMTLOGO_CTRL *Ctrl = NULL;	/* Control structure specific to program */
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_gmtlogo_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_gmtlogo_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_gmtlogo_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_gmtlogo_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtlogo_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the gmtlogo main code ----------------------------*/

	/* Ready to make the plot */

	GMT_Report (API, GMT_MSG_VERBOSE, "Constructing the GMT logo\n");

	/* The following is needed to have gmtlogo work correctly in perspective */

	GMT_memset (wesn, 4, double);
	if (!(GMT->common.R.active && GMT->common.J.active)) {	/* When no projection specified, use fake linear projection */
		GMT->common.R.active = true;
		GMT->common.J.active = false;
		GMT_parse_common_options (GMT, "J", 'J', "X1i");
		Ctrl->D.refpoint->x -= 0.5 * ((Ctrl->D.justify-1)%4) * Ctrl->W.width;
		Ctrl->D.refpoint->y -= 0.25 * (Ctrl->D.justify/4) * Ctrl->W.width;	/* 0.25 because height = 0.5 * width */
		/* Also deal with any justified offsets if given */
		Ctrl->D.refpoint->x -= ((Ctrl->D.justify%4)-2) * Ctrl->D.off[GMT_X];
		Ctrl->D.refpoint->y -= ((Ctrl->D.justify/4)-1) * Ctrl->D.off[GMT_Y];
		wesn[XHI] = Ctrl->D.refpoint->x + Ctrl->W.width;	wesn[YHI] = Ctrl->D.refpoint->y + 0.5 * Ctrl->W.width;
		GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");
		PSL = GMT_plotinit (GMT, options);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	}
	else {	/* First use current projection, project, then use fake projection */
		if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);
		GMT_set_refpoint (GMT, Ctrl->D.refpoint);	/* Finalize reference point plot coordinates, if needed */
		Ctrl->D.refpoint->x -= 0.5 * ((Ctrl->D.justify-1)%4) * Ctrl->W.width;
		Ctrl->D.refpoint->y -= 0.25 * (Ctrl->D.justify/4) * Ctrl->W.width;	/* 0.25 because height = 0.5 * width */
		/* Also deal with any justified offsets if given */
		Ctrl->D.refpoint->x -= ((Ctrl->D.justify%4)-2) * Ctrl->D.off[GMT_X];
		Ctrl->D.refpoint->y -= ((Ctrl->D.justify/4)-1) * Ctrl->D.off[GMT_Y];
		PSL = GMT_plotinit (GMT, options);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT->common.J.active = false;
		GMT_parse_common_options (GMT, "J", 'J', "X1i");
		wesn[XHI] = Ctrl->D.refpoint->x + Ctrl->W.width;	wesn[YHI] = Ctrl->D.refpoint->y + 0.5 * Ctrl->W.width;
		GMT->common.R.active = GMT->common.J.active = true;
		GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");
	}

	PSL_command (PSL, "V\n");	/* Ensure the entire gmtlogo output after initialization is between gsave/grestore */
	PSL_setorigin (PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->y, 0.0, PSL_FWD);

	/* Set up linear projection with logo domain and user width */
	scale = Ctrl->W.width / 2.0;	/* Scale relative to default size 2 inches */
	plot_x = 0.5 * Ctrl->W.width;	plot_y = 0.25 * Ctrl->W.width;	/* Center of logo box */
	if (Ctrl->F.active) {	/* First place legend frame fill */
		Ctrl->F.panel->width = Ctrl->W.width;	Ctrl->F.panel->height = 0.5 * Ctrl->W.width;	
		GMT_draw_map_panel (GMT, plot_x, plot_y, 3U, Ctrl->F.panel);
	}
	
	/* Plot the title beneath the map with 1.5 vertical stretching */

	plot_y = 0.027 * scale;
	sprintf (cmd, "%g,AvantGarde-Demi,%s", scale * 9.5, c_font);	/* Create required font */
	GMT_getfont (GMT, cmd, &F);
	fmode = GMT_setfont (GMT, &F);
	PSL_setfont (PSL, F.id);
	PSL_command (PSL, "V 1 1.5 scale\n");
	PSL_plottext (PSL, plot_x, plot_y, F.size, "@#THE@# G@#ENERIC@# M@#APPING@# T@#OOLS@#", 0.0, PSL_BC, fmode);
	PSL_command (PSL, "U\n");

	/* Plot the globe via GMT_psclip & GMT_pscoast */

	sprintf (pars, "--MAP_GRID_PEN=faint,%s --MAP_FRAME_PEN=%gp,%s", c_grid, scale * 0.3, c_grid);
	sprintf (cmd, "-T -Rd -JI0/%gi -N -O -K -X%gi -Y%gi %s", scale * 1.55, scale * 0.225, scale * 0.220, pars);
	GMT_Call_Module (API, "psclip", GMT_MODULE_CMD, cmd);
	sprintf (cmd, "-Rd -JI0/%gi -S%s -G%s -A35000+l -Dc -O -K %s", scale * 1.55, c_water, c_land, pars);
	GMT_Call_Module (API, "pscoast", GMT_MODULE_CMD, cmd);
	sprintf (cmd, "-Rd -JI0/%gi -C -O -K -Bxg45 -Byg30  %s --MAP_POLAR_CAP=none", scale * 1.55, pars);
	GMT_Call_Module (API, "psclip", GMT_MODULE_CMD, cmd);
	
	/* Plot the GMT letters as shadows, then full size, using GMT_psxy */

	GMT_getsharepath (GMT, "conf", "gmtlogo_letters", ".txt", file, R_OK);

	sprintf (cmd, "-<%s -R167/527/-90/90 -JI-13/%gi -O -K -G%s@40",
		file, scale * 1.55, c_gmt_shadow);   
	GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd);
	sprintf (cmd, "-<%s -R167/527/-90/90 -JI-13/%gi -O -K -G%s -W%gp,%s -X-%gi -Y-%gi",
		file, scale * 2.47, c_gmt_fill, scale * 0.3, c_gmt_outline, scale * 0.483, scale * 0.230);   
	GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd);
	
	PSL_setorigin (PSL, -Ctrl->D.refpoint->x, -Ctrl->D.refpoint->y, 0.0, PSL_INV);
	GMT_plane_perspective (GMT, -1, 0.0);
	PSL_command (PSL, "U\n");	/* Ending the encapsulation for gmtlogo */

	GMT_plotend (GMT);

	Return (GMT_OK);
}
