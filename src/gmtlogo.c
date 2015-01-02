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
#define THIS_MODULE_PURPOSE	"Plot the GMT logo"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->KJOPRUVXYcptxy"

/* Specific colors for fonts, land, water, text etc */

#define c_font		"51/51/51"
#define c_grid		"150/160/163"
#define c_land		"125/168/125"
#define c_water		"189/224/223"
#define c_gmt_fill	"238/86/52"
#define c_gmt_outline	"112/112/112"
#define c_gmt_shadow	"92/102/132"

#define FRAME_CLEARANCE	4.0	/* In points */
#define FRAME_GAP	2.0	/* In points */
#define FRAME_RADIUS	6.0	/* In points */

/* Control structure for gmtlogo */

struct GMTLOGO_CTRL {
	struct D {	/* -D[g|j|n|x]<anchor>[/<justify>][/<dx>/<dy>] */
		bool active;
		struct GMT_ANCHOR *anchor;
		double dx, dy;
		int justify;
	} D;
	struct W {	/* -W<width> */
		bool active;
		double width;
	} W;
	struct F {	/* -F[+r[<radius>]][+g<fill>][+p[<pen>]][+i[<off>/][<pen>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;
		struct GMT_MAP_PANEL panel;
	} F;
};

void *New_gmtlogo_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTLOGO_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTLOGO_CTRL);
	C->D.justify = PSL_BL;
	C->F.panel.radius = GMT->session.u2u[GMT_PT][GMT_INCH] * FRAME_RADIUS;		/* 6 pt */
	GMT_init_fill (GMT, &C->F.panel.fill, -1.0, -1.0, -1.0);		/* Default is no fill */
	C->F.panel.pen1 = GMT->current.setting.map_frame_pen;
	C->F.panel.pen2 = GMT->current.setting.map_default_pen;
	C->F.panel.gap = GMT->session.u2u[GMT_PT][GMT_INCH] * FRAME_GAP;	/* Default is 2p */
	C->F.panel.dx = GMT->session.u2u[GMT_PT][GMT_INCH] * FRAME_CLEARANCE;
	C->F.panel.dy = -C->F.panel.dx;			/* Default is (4p, -4p) */
	GMT_init_fill (GMT, &C->F.panel.sfill, 0.5, 0.5, 0.5);		/* Default is gray shade if used */
	C->W.width = 2.0;	/* Default width is 2" */
	return (C);
}

void Free_gmtlogo_Ctrl (struct GMT_CTRL *GMT, struct GMTLOGO_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free_anchorpoint (GMT, &C->D.anchor);
	GMT_free (GMT, C);
}

int GMT_gmtlogo_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the gmtlogo synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtlogo  [-D[g|j|n|x]<anchor>[/<justify>][/<dx>/<dy>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "[-F[+i[[<gap>/]<pen>]][+g<fill>][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<fill>]][+d]] [%s] [%s] [-K]\n", GMT_J_OPT, GMT_Jz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t [-O] [-P] [-W<width>] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n\n", GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set the lower left (anchor) position x0,y0 on the map for the logo [0/0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Dg to specify <anchor> with map coordinates.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Dj to specify <anchor> with 2-char justification code (LB, CM, etc).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Dn to specify <anchor> with normalized coordinates in 0-1 range.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Dx to specify <anchor> with plot coordinates.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   All except -Dx requires the -R and -J options to be set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append 2-char <justify> code to associate that point on the logo with <x0>/<y0> [LB].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note for -Dj: If <justify> is not given then it inherits the code use to set <x0>/<y0>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append <dx>/<dy> to shift the logo from the selected anchor in the direction implied by <justify> [0/0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Without further options: draw rectangular border behind the logo (using MAP_FRAME_PEN)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      [Default is no border].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +d to draw guide lines for debugging.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +g<fill> to set the fill for the logo box [Default is no fill].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +i[[<gap>/]<pen>] to add a secondary inner frame boundary [Default gap is %gp].\n", FRAME_GAP);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p[<pen>] to draw the border and optionally change the border pen [%s].\n",
		GMT_putpen (API->GMT, API->GMT->current.setting.map_frame_pen));
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +r[<radius>] to plot rounded rectangles instead [Default radius is %gp].\n", FRAME_RADIUS);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +s[<dx>/<dy>/]<fill> to plot a shadow behind the logo box [Default offset is %gp/%g].\n", FRAME_CLEARANCE, -FRAME_CLEARANCE);
	GMT_Option (API, "J-Z,K,O,P,R");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set width of the GMT logo [2 inches].\n");
	GMT_Option (API, "U,V");
	GMT_Option (API, "X,c,f,p,t,.");

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
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			/* Processes program-specific parameters */

			case 'D':
				Ctrl->D.active = true;
				if ((Ctrl->D.anchor = GMT_get_anchorpoint (GMT, opt->arg)) == NULL) n_errors++;	/* Failed basic parsing */
				else {	/* args are [/<justify>][/<dx>/<dy>] (0-3) */
					n = sscanf (Ctrl->D.anchor->args, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
					if (Ctrl->D.anchor->mode == GMT_ANCHOR_JUST)	/* For -Dj with no 2nd justification, use same code as anchor coordinate as default */
						Ctrl->D.justify = Ctrl->D.anchor->justify;
					switch (n) {
						case 1: Ctrl->D.justify = GMT_just_decode (GMT, txt_a, 12);	break;
						case 2: Ctrl->D.dx = GMT_to_inch (GMT, txt_a); 	Ctrl->D.dy = GMT_to_inch (GMT, txt_b);
							break;
						case 3: Ctrl->D.justify = GMT_just_decode (GMT, txt_a, 12);	Ctrl->D.dx = GMT_to_inch (GMT, txt_b); 	Ctrl->D.dy = GMT_to_inch (GMT, txt_c); break;
					}
				}
				break;
			case 'F':
				Ctrl->F.active = true;
				if (GMT_getpanel (GMT, opt->option, opt->arg, &Ctrl->F.panel)) {
					//GMT_panel_syntax (GMT, 'FW', " ");
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
		Ctrl->D.anchor = GMT_get_anchorpoint (GMT, "x0/0");	/* Default if no -D given */
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
	
	char text[GMT_LEN64] = {""}, cmd[GMT_BUFSIZ] = {""}, file[GMT_BUFSIZ] = {""};
	
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
		Ctrl->D.anchor->x -= 0.5 * ((Ctrl->D.justify-1)%4) * Ctrl->W.width;
		Ctrl->D.anchor->y -= 0.25 * (Ctrl->D.justify/4) * Ctrl->W.width;	/* 0.25 because height = 0.5 * width */
		/* Also deal with any justified offsets if given */
		Ctrl->D.anchor->x -= ((Ctrl->D.justify%4)-2) * Ctrl->D.dx;
		Ctrl->D.anchor->y -= ((Ctrl->D.justify/4)-1) * Ctrl->D.dy;
		wesn[XHI] = Ctrl->D.anchor->x + Ctrl->W.width;	wesn[YHI] = Ctrl->D.anchor->y + 0.5 * Ctrl->W.width;
		GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");
		PSL = GMT_plotinit (GMT, options);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	}
	else {	/* First use current projection, project, then use fake projection */
		if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);
		GMT_set_anchorpoint (GMT, Ctrl->D.anchor);	/* Finalize anchor point plot coordinates, if needed */
		Ctrl->D.anchor->x -= 0.5 * ((Ctrl->D.justify-1)%4) * Ctrl->W.width;
		Ctrl->D.anchor->y -= 0.25 * (Ctrl->D.justify/4) * Ctrl->W.width;	/* 0.25 because height = 0.5 * width */
		/* Also deal with any justified offsets if given */
		Ctrl->D.anchor->x -= ((Ctrl->D.justify%4)-2) * Ctrl->D.dx;
		Ctrl->D.anchor->y -= ((Ctrl->D.justify/4)-1) * Ctrl->D.dy;
		PSL = GMT_plotinit (GMT, options);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT->common.J.active = false;
		GMT_parse_common_options (GMT, "J", 'J', "X1i");
		wesn[XHI] = Ctrl->D.anchor->x + Ctrl->W.width;	wesn[YHI] = Ctrl->D.anchor->y + 0.5 * Ctrl->W.width;
		GMT->common.R.active = GMT->common.J.active = true;
		GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");
	}

	PSL_setorigin (PSL, Ctrl->D.anchor->x, Ctrl->D.anchor->y, 0.0, PSL_FWD);

	/* Set up linear projection with logo domain and user width */
	scale = Ctrl->W.width / 2.0;	/* Scale relative to default size 2 inches */
	plot_x = 0.5 * Ctrl->W.width;	plot_y = 0.25 * Ctrl->W.width;	/* Center of logo box */

	if (Ctrl->F.active && (Ctrl->F.panel.mode & 8)) {	/* First place legend frame fill */
		double dim[3];
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Draw back panel\n");
		dim[0] = Ctrl->W.width;
		dim[1] = 0.5 * Ctrl->W.width;
		dim[2] = Ctrl->F.panel.radius;
		if (Ctrl->F.panel.mode & 4) {	/* Draw offset background shade first */
			GMT_setfill (GMT, &Ctrl->F.panel.sfill, false);
			PSL_plotsymbol (PSL, plot_x + Ctrl->F.panel.dx, plot_y + Ctrl->F.panel.dy, dim, (Ctrl->F.panel.mode & 2) ? PSL_RNDRECT : PSL_RECT);
		}
		GMT_setpen (GMT, &Ctrl->F.panel.pen1);	/* Draw frame outline, without fill */
		GMT_setfill (GMT, &Ctrl->F.panel.fill, (Ctrl->F.panel.mode & 16) == 16);
		PSL_plotsymbol (PSL, plot_x, plot_y, dim, (Ctrl->F.panel.mode & 2) ? PSL_RNDRECT : PSL_RECT);
		if (Ctrl->F.panel.mode & 1) {	/* Also draw secondary frame on the inside */
			dim[0] = Ctrl->W.width - 2.0 * Ctrl->F.panel.gap;
			dim[1] = 0.5 * Ctrl->W.width - 2.0 * Ctrl->F.panel.gap;
			GMT_setpen (GMT, &Ctrl->F.panel.pen2);
			GMT_setfill (GMT, NULL, true);	/* No fill for inner frame */
			PSL_plotsymbol (PSL, plot_x, plot_y, dim, (Ctrl->F.panel.mode & 2) ? PSL_RNDRECT : PSL_RECT);
		}
		/* Reset color */
		PSL_setcolor (PSL, GMT->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);
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

	sprintf (cmd, "-T -Rd -JI0/%gi -N -O -K -X%gi -Y%gi", scale * 1.55, scale * 0.225, scale * 0.220);
	GMT_Call_Module (API, "psclip", GMT_MODULE_CMD, cmd);
	sprintf (cmd, "-Rd -JI0/%gi -S%s -G%s -A35000+l -Dc -O -K", scale * 1.55, c_water, c_land);
	GMT_Call_Module (API, "pscoast", GMT_MODULE_CMD, cmd);
	sprintf (cmd, "-Rd -JI0/%gi -C -O -K -Bxg45 -Byg30 --MAP_FRAME_PEN=0.5p,gray20 --MAP_POLAR_CAP=none", scale * 1.55);
	GMT_Call_Module (API, "psclip", GMT_MODULE_CMD, cmd);
	
	/* Plot the GMT letters as shadows, then full size, using GMT_psxy */

	GMT_getsharepath (GMT, "conf", "gmtlogo_letters", ".txt", file, R_OK);

	sprintf (cmd, "-<%s -R167/527/-90/90 -JI-13/%gi -O -K -G%s@40",
		file, scale * 1.55, c_gmt_shadow);   
	GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd);
	sprintf (cmd, "-<%s -R167/527/-90/90 -JI-13/%gi -O -K -G%s -W%gp,%s -X-%gi -Y-%gi",
		file, scale * 2.47, c_gmt_fill, scale * 0.3, c_gmt_outline, scale * 0.483, scale * 0.230);   
	GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd);
	
	PSL_setorigin (PSL, -Ctrl->D.anchor->x, -Ctrl->D.anchor->y, 0.0, PSL_INV);
	GMT_plane_perspective (GMT, -1, 0.0);

	GMT_plotend (GMT);

	Return (GMT_OK);
}
