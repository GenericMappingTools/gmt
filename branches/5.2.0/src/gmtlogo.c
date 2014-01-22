/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * API functions to support the gmtlogo application.
 *
 * Author:	Paul Wessel
 * Date:	1-FEB-2013
 * Version:	5 API
 *
 * Brief synopsis: gmtlogo plots a GMT logo on a map.
 */

#define THIS_MODULE_NAME	"gmtlogo"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot GMT logo on maps"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->KOPUVXYcptxy" GMT_OPT("EZ")

/* Control structure for gmtlogo */

struct GMTLOGO_CTRL {
	struct G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct S {	/* -S<scale> */
		bool active;
		double scale;
	} S;
	struct W {	/* -W<pen> */
		bool active;
		unsigned int mode;	/* 0 = normal, 1 = -C applies to pen color only, 2 = -C applies to symbol fill & pen color */
		struct GMT_PEN pen;
	} W;
};

void *New_gmtlogo_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTLOGO_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTLOGO_CTRL);
	GMT_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* Default is no fill */
	C->S.scale = 1.0;	/* Default scale is 1 */
	return (C);
}

void Free_gmtlogo_Ctrl (struct GMT_CTRL *GMT, struct GMTLOGO_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);
}

int GMT_gmtlogo_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the gmtlogo synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtlogo [-G<fill>] [-K] [-O] [-P] [-S<scale>] [%s] [%s]\n", GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-W<pen>] [%s] [%s] [%s] [%s] [%s]\n\n", GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_fill_syntax (API->GMT, 'G', "Specify color or pattern for underlying rectangle [no fill].");
	GMT_Option (API, "K,O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Scale the GMT logo size from default 2 by 1 inches (5 by 2.5 cm).\n");
	GMT_Option (API, "U,V");
	GMT_pen_syntax (API->GMT, 'W', "Set pen attributes for underlying rectangle outline [no outline].");
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
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			/* Processes program-specific parameters */

			case 'G':	/* Set color of underlying box */
				if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'S':	/* Scale for the logo */
				Ctrl->S.active = true;
				Ctrl->S.scale = atof (opt->arg);
				break;
			case 'W':
				Ctrl->W.active = true;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', " ");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	n_errors += GMT_check_condition (GMT, Ctrl->S.scale < 0.0, "Syntax error -S option: scale cannot be zero or negative!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtlogo_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout(code);}

#define c_font		"51/51/51"
#define c_grid		"150/160/163"
#define c_land		"125/168/125"
#define c_water		"189/224/223"
#define c_gmt_fill	"238/86/52"
#define c_gmt_outline	"112/112/112"
#define c_gmt_shadow	"92/102/132"
#define NP		86

#if 0
double gmt_x[] = {
#include "x.h"
};
double gmt_y[] = {
#include "y.h"
};
#endif

int GMT_gmtlogo (void *V_API, int mode, void *args)
{	/* High-level function that implements the gmtlogo task */
	int error;
	
	double wesn[4] = {0.0, 0.0, 0.0, 0.0};	/* Dimensions in inches */
	
	char text[GMT_LEN64] = {""}, cmd[GMT_BUFSIZ] = {""};
	char file[GMT_BUFSIZ] = {""};
	
	struct GMTLOGO_CTRL *Ctrl = NULL;	/* Control structure specific to program */
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
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

	GMT_Report (API, GMT_MSG_VERBOSE, "Constructing the logo [NOT IMPLEMENTED YET - USE gmtlogo SCRIPT]\n");

#if 0
 	V = GMT_Create_Data (API, GMT_IS_VECTOR, GMT_IS_POINT, 0, par, NULL, NULL, 0, -1, NULL);
 	S = GMT_Create_Data (API, GMT_IS_VECTOR, GMT_IS_POINT, 0, par, NULL, NULL, 0, -1, NULL);
	for (k = 0; k < NP; k++) {	/* Replace -999 with NaN */
		if (gmt_x[k] < -500.0) gmt_x[k] = GMT->session.d_NaN;
		if (gmt_y[k] < -500.0) gmt_y[k] = GMT->session.d_NaN;
	}
	V->data[GMT_X] = (union GMT_UNIVECTOR)gmt_x;  V->type[GMT_X] = GMT_DOUBLE;
	V->data[GMT_Y] = (union GMT_UNIVECTOR)gmt_y;  V->type[GMT_Y] = GMT_DOUBLE;
 	ID = GMT_Get_ID (API, GMT_IS_DATASET, GMT_IN, V);
	GMT_Encode_ID (API, string, ID);
	S->data[GMT_X] = (union GMT_UNIVECTOR)gmt_x;  S->type[GMT_X] = GMT_DOUBLE;
	S->data[GMT_Y] = (union GMT_UNIVECTOR)gmt_y;  S->type[GMT_Y] = GMT_DOUBLE;
 	ID2 = GMT_Get_ID (API, GMT_IS_DATASET, GMT_IN, S);
	GMT_Encode_ID (API, string2, ID2);
#endif

	/* Set up linear projection with logo domain and user scale */
	if (GMT->current.setting.proj_length_unit == GMT_INCH) {	/* Logo is 2 by 1 inches */
		wesn[XHI] = 2.0;	wesn[YHI] = 1.0;
	}
	else {
		wesn[XHI] = 5.0 / 2.54;	wesn[YHI] = 2.5 / 2.54;		/* Logo is 5 by 2.5 cm */
	}
	GMT->common.R.active = true;	GMT->common.J.active = false;
	sprintf (text, "x%gi", Ctrl->S.scale);
	GMT_parse_common_options (GMT, "J", 'J', text);
	GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");

	GMT_plotinit (GMT, options);

	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	if (Ctrl->G.active || Ctrl->W.active) {	/* Draw and/or fill the background box */
		double dim[2], plot_x, plot_y;
		/* Determine polygon size given clearence, then register input and call GMT_psxy */
		GMT_setfill (GMT, &Ctrl->G.fill, Ctrl->W.active);
		GMT_setpen (GMT, &Ctrl->W.pen);
		dim[GMT_X] = 1.03 * wesn[XHI] * Ctrl->S.scale;
		dim[GMT_Y] = 1.03 * wesn[YHI] * Ctrl->S.scale;
		GMT_geo_to_xy (GMT, 0.5 * wesn[XHI], 0.5 * wesn[YHI], &plot_x, &plot_y);	/* Center of logo box */
		PSL_plotsymbol (GMT->PSL, plot_x, plot_y, dim, GMT_SYMBOL_RECT);
	}
	
	/* Plot the globe via GMT_pscoast */
	
	sprintf (cmd, "-Rd -JI0/%gi -S%s -G%s -A35000+l -Dc -O -K -X%gi -Y%gi -Vd",
		Ctrl->S.scale * 1.55, c_water, c_land, Ctrl->S.scale * 0.225, Ctrl->S.scale * 0.220);
	GMT_Call_Module (API, "pscoast", GMT_MODULE_CMD, cmd);
	
	/* Plot the title beneath the map */
	
	GMT_getsharepath (GMT, "conf", "gmtlogo_title", ".txt", file, R_OK);
	sprintf (cmd, "-<%s -F+f%g,AvantGarde-Demi,%s+jBC -R0/2/0/1 -Jx%gi -O -K -N -X-1i -Y-0.5i",
		file, Ctrl->S.scale * 9.5, c_font, Ctrl->S.scale);   
	PSL_command (GMT->PSL, "V 1 1.5 scale\n");
	GMT_Call_Module (API, "pstext", GMT_MODULE_CMD, cmd);
	PSL_command (GMT->PSL, "U\n");
	
	/* Plot the GMT letters as shadows, then full size via GMT_psxy */

	GMT_getsharepath (GMT, "conf", "gmtlogo_letters", ".txt", file, R_OK);

	sprintf (cmd, "-<%s -R167/527/-90/90 -JI-13/%gi -O -K -G%s@40",
		file, Ctrl->S.scale * 1.55, c_gmt_shadow);   
	GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd);
	sprintf (cmd, "-<%s -R167/527/-90/90 -JI-13/%gi -O -K -G%s -W%gp,%s -X-%gi -Y-%gi",
		file, Ctrl->S.scale * 2.47, c_gmt_fill, Ctrl->S.scale * 0.3, c_gmt_outline, Ctrl->S.scale * 0.483, Ctrl->S.scale * 0.230);   
	GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd);
	
	GMT_plane_perspective (GMT, -1, 0.0);

	GMT_plotend (GMT);

	Return (GMT_OK);
}
