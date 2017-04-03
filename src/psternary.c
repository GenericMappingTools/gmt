/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: psternary reads one or many abc-files and either
 * plots symbols, draws contours, triangulates to an image, or just
 * converts a,b,c to x,y coordinates.
 *
 * Author:	Paul Wessel
 * Date:	1-APR-2017
 * Version:	5 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"psternary"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot data on ternary diagrams"
#define THIS_MODULE_KEYS	"<D{,>X},>DM,C-("
#define THIS_MODULE_NEEDS	"dJ"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYbdfghipstxy"

struct PSTERNARY_CTRL {
	struct PSTERNARY_A {	/* -A[-][labelinfo] */
		bool active;
		char *string;	/* Since we will simply pass this on to pscontour */
	} A;
	struct PSTERNARY_C {	/* -C<cpt> */
		bool active;
		char *string;	/* Since we will simply pass this on to psxy or pscontour */
	} C;
	struct PSTERNARY_G {	/* -G<fill> */
		bool active;
		char *string;	/* Since we will simply pass this on to psxy */
	} G;
	struct PSTERNARY_M {	/* -M */
		bool active;
	} M;
	struct PSTERNARY_N {	/* -N */
		bool active;
	} N;
	struct PSTERNARY_Q {	/* -Q */
		bool active;
		char *string;	/* Since we will simply pass this on to pscontour */
	} Q;
	struct PSTERNARY_S {	/* -S */
		bool active;
		char *string;	/* Since we will simply pass this on to psxy */
	} S;
	struct PSTERNARY_T {	/* -T[+|-][+d<gap>[c|i|p][/<length>[c|i|p]]][+lLH|"low,high"] */
		bool active;
		char *string;	/* Since we will simply pass this on to pscontour */
	} T;
	struct PSTERNARY_W {	/* -W[a|c]<pen>[+c[l|f]] */
		bool active;
		char *string;	/* Since we will simply pass this on to psxy or pscontour */
	} W;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSTERNARY_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSTERNARY_CTRL);

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSTERNARY_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->A.string);
	gmt_M_str_free (C->C.string);
	gmt_M_str_free (C->G.string);
	gmt_M_str_free (C->S.string);
	gmt_M_str_free (C->T.string);
	gmt_M_str_free (C->W.string);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psternary synopsis and optionally full usage information */

	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: psternary <table> -JX<width> -Ramin/amax/bmin/bmax/cmin/cmax [-B<args> or -Ba<args> -Bb<args> -Bc<args>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<cpt>] [-G<fill>] [-K] [-O] [-S[<symbol>][<size>[unit]]] [-W[<pen>][<attr>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-K] [-M] [-N] [-O] [-P] [-Q[<cut>]] [%s] [%s]\n", GMT_Jz_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n", GMT_X_OPT, GMT_Y_OPT, GMT_bi_OPT, GMT_di_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-C Undo existing clip-paths; no file is needed.  -R, -J are not required (unless -B is used).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Terminates all clipping; optionally append how many clip levels to restore [all].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more polygon files.  If none, standard input is read.\n");
	GMT_Option (API, "J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Use CPT to assign symbol colors based on z-value in 3rd column (with -S), or\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   specify contours to be drawn (with -Q).\n");
	gmt_fill_syntax (API->GMT, 'G', "Specify color or pattern [no fill].");
	GMT_Option (API, "<,B-,K");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Dump a multisegment ASCII (or binary, see -bo) file of x,y to standard output.  No plotting occurs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do not skip or clip symbols that fall outside the map border [clipping is on]\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Selects contouring.  Optionally append <cut>.  If given, then we do not draw\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   closed contours with less than <cut> points [Draw all contours].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select symbol type and symbol size (in %s).  See psxy for full list of symbols.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set clip path for the entire map frame.  No input file is required.\n");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', "Set pen attributes [Default pen is %s]:", 15);
	GMT_Option (API, "X,bi2,di,f,g,h,i,p,t,:,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSTERNARY_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psternary and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Turn off draw_arc mode */
				Ctrl->A.active = true;
				gmt_M_str_free (Ctrl->A.string);
				Ctrl->A.string = strdup (opt->arg);
				break;
			case 'C':	/* Turn clipping off */
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->C.string);
				Ctrl->C.string = strdup (opt->arg);
				break;
			case 'G':	/* Fill */
				Ctrl->G.active = true;
				gmt_M_str_free (Ctrl->G.string);
				Ctrl->G.string = strdup (opt->arg);
				break;
			case 'M':	/* Convert a,b,c -> x,y and dump */
				Ctrl->M.active = true;
				break;
			case 'N':	/* Use the outside of the polygons as clip area */
				Ctrl->N.active = true;
				break;
			case 'S':	/* Plot symbols */
				Ctrl->S.active = true;
				gmt_M_str_free (Ctrl->S.string);
				Ctrl->S.string = strdup (opt->arg);
				break;
			case 'W':	/* Pen settings */
				Ctrl->W.active = true;
				gmt_M_str_free (Ctrl->W.string);
				Ctrl->W.string = strdup (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (!Ctrl->M.active) {	/* Need -R -J for anything but dumping */
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
		n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
		n_errors += gmt_M_check_condition (GMT, !(Ctrl->S.active || Ctrl->Q.active), "Syntax error: Must specify either -S or -Q\n");
	}

	n_errors += gmt_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL unsigned int prep_options (struct GMTAPI_CTRL *API, struct GMT_OPTION **options, struct GMT_OPTION *boptions[]) {
	/* Must intercept select common options and change to valid GMT syntax */
	unsigned int k, n_axis = 0;
	struct GMT_OPTION *opt = NULL, *bopt = NULL;
	char string[GMT_LEN256] = {""};
	
	/* First, append -Jz1 so that -Ramin/amax/bmin/bmax/cmin/cmax is accepted */
	
	if ((opt = GMT_Find_Option (API, 'M', *options)) == NULL) {
		if ((opt = GMT_Make_Option (API, 'J', "z1i")) == NULL) return (GMT_PARSE_ERROR);
		if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return GMT_PARSE_ERROR;	/* Failure to append this option */
	}
	
	for (opt = *options; opt; opt = opt->next) {	/* Linearly search for the specified option */
		if (opt->option == 'B') {
			if (strchr ("ABC", opt->arg[0]) || strstr (opt->arg, "+b") || strstr (opt->arg, "+g") || strstr (opt->arg, "+n") || strstr (opt->arg, "+t")) {	/* Frame setting */
				for (k = 0; opt->arg[k] && opt->arg[k] != '+'; k++) {
					if (opt->arg[k] == 'a') opt->arg[0] = 's';		/* -Ba becomes -Bs */
					else if (opt->arg[k] == 'A') opt->arg[k] = 'S';	/* -BA becomes -BS */
					else if (opt->arg[k] == 'b') opt->arg[k] = 'e';	/* -Bb becomes -Be */
					else if (opt->arg[k] == 'B') opt->arg[k] = 'E';	/* -BB becomes -BE */
					else if (opt->arg[k] == 'c') opt->arg[k] = 'w';	/* -Bc becomes -Bw */
					else if (opt->arg[k] == 'C') opt->arg[k] = 'W';	/* -BC becomes -BW */
				}
			}
			else {	/* Count axis settings [0, 1 or 3 OK] */
				n_axis++;
				bopt = opt;
			}
		}
	}
	if (n_axis == 0)
		return GMT_NOERROR;	/* Did not specify any axis annotations */
	else if (n_axis == 1) {	/* Same annotations for each axis */
		sprintf (string, "a%s", bopt->arg);	/* Turn -B<arg> to -Ba<arg> */
		if ((boptions[GMT_X] = GMT_Make_Option (API, 'B', string)) == NULL) return (GMT_PARSE_ERROR);
		string[0] = 'b';	/* Next is -Ba */
		if ((boptions[GMT_Y] = GMT_Make_Option (API, 'B', string)) == NULL) return (GMT_PARSE_ERROR);
		string[0] = 'c';	/* Finally -Bc */
		if ((boptions[GMT_Z] = GMT_Make_Option (API, 'B', string)) == NULL) return (GMT_PARSE_ERROR);
		GMT_Delete_Option (API, bopt);	/* Remove the single -B setting now that we added 3 separate */
		return GMT_NOERROR;
	}
	else if (n_axis != 3) {
		GMT_Report (API, GMT_MSG_NORMAL, "Use -B<args> for all three axis or specify -Ba<args> -Bb<args> -Bc<args>\n");
		return GMT_PARSE_ERROR;
	}
	for (opt = *options; opt; opt = opt->next) {	/* Linearly search for the specified option */
		if (opt->option == 'B') {
			if (opt->arg[0] == 'a') {
				if ((boptions[GMT_X] = GMT_Make_Option (API, 'B', opt->arg)) == NULL) return (GMT_PARSE_ERROR);
				opt->arg[0] = 'x';		/* -Ba<args> becomes -Bx<args> */
			}
			else if (opt->arg[0] == 'b') {
				if ((boptions[GMT_Y] = GMT_Make_Option (API, 'B', opt->arg)) == NULL) return (GMT_PARSE_ERROR);
				opt->arg[0] = 'y';	/* -Bb<args> becomes -By<args> */
			}
			else if (opt->arg[0] == 'c') {
				if ((boptions[GMT_Z] = GMT_Make_Option (API, 'B', opt->arg)) == NULL) return (GMT_PARSE_ERROR);
				opt->arg[0] = 'z';	/* -Bc<args> becomes -Bz<args> */
			}
		}
	}
	return GMT_NOERROR;
}

#define SQRT3 1.73205080756887729352	/* sqrt(3) */

GMT_LOCAL void abc_to_xy (double a, double b, double c, double *x, double *y) {
	double s = (a + b + c);
	*x = 0.5 * (2.0 * b + c) / s;
	*y = 0.5 * SQRT3 * c / s;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC void gmtlib_set_dataset_minmax (struct GMT_CTRL *GMT, struct GMT_DATASET *D);

int GMT_psternary (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int n_sides = 0;
	uint64_t tbl, seg, row, col;
	
	char cmd[GMT_LEN256] = {""};

	double x, y, rect[4] = {0.0, 0.0, 0.0, 0.0}, tri_x[4] = {0.0, 0.0, 0.0, 0.0}, tri_y[4] = {0.0, 0.0, 0.0, 0.0};
	double wesn_orig[6];

	struct PSTERNARY_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL, *boptions[3] = {NULL, NULL, NULL};
	struct GMT_DATASET *D = NULL;	/* Pointer to GMT multisegment table(s) */
	struct GMT_DATASEGMENT *S = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	if ((error = prep_options (API, &options, boptions))) bailout (error);	/* Enforce common option syntax temporarily to get passed GMT_Parse_Common */
	
	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psternary main code ----------------------------*/

	GMT_Report (API, GMT_MSG_NORMAL, "psternary is experimental and not completed!\n");

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR)	/* Register data input */
		Return (API->error);
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL)
		Return (API->error);
	
	/* Convert a,b,c[,z] to to x,y[,z] */
	for (tbl = 0; tbl < D->n_tables; tbl++) {	/* For each table */
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
			S = D->table[tbl]->segment[seg];	/* Set shortcut to current segment */
			for (row = 0; row < S->n_rows; row++) {
				abc_to_xy (S->data[GMT_X][row], S->data[GMT_Y][row], S->data[GMT_Z][row], &x, &y);
				S->data[GMT_X][row] = x;	S->data[GMT_Y][row] = y;
				for (col = GMT_Z + 1; col < D->n_columns; col++)	/* Override c column by moving columns inward */
					S->data[col-1][row] = S->data[col][row];
			}
		}
	}
	gmt_adjust_dataset (GMT, D, D->n_columns-1);	/* Remove all traces of the extra column */
	gmtlib_set_dataset_minmax (GMT, D);				/* Update column stats */
	
	if (Ctrl->M.active) {	/* Just print the converted data and exit */
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, 0, NULL, NULL, D) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to write x,y file to stdout\n");
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}
	
	/* Here we are doing some sort of plotting */
	
	rect[XHI] = gmt_M_to_inch (GMT, &GMT->common.J.string[1]);
	rect[YHI] =  0.5 * SQRT3 * rect[XHI];
	tri_x[1] = rect[XHI];	tri_x[2] = 0.5 * rect[XHI];	tri_y[2] = rect[YHI];
	gmt_M_memcpy (wesn_orig, GMT->common.R.wesn, 6, double);
	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, rect), ""))
		Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL)
		Return (GMT_RUNTIME_ERROR);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	/* First deal with the ternary basemap frame and cancas filling */
	if (GMT->current.map.frame.paint) {	/* Paint the inside of the map with specified fill */
		gmt_setfill (GMT, &GMT->current.map.frame.fill, false);
		PSL_plotpolygon (GMT->PSL, tri_x, tri_y, 4);
		GMT->current.map.frame.paint = false;
	}
	/* Count how many of the three sides will be drawn */
	if (GMT->current.map.frame.side[S_SIDE]) n_sides++;	/* The bottom (a) side */
	if (GMT->current.map.frame.side[E_SIDE]) n_sides++;	/* The right (b) side */
	if (GMT->current.map.frame.side[W_SIDE]) n_sides++;	/* The left (c) side */
	if (n_sides) {	/* Draw some or all of the triangular sides */
		PSL_comment (GMT->PSL, "Draw triangular frame sides\n");
		gmt_setpen (GMT, &GMT->current.setting.map_frame_pen);
		gmt_setfill (GMT, NULL, true);
		if (n_sides == 3)
			PSL_plotpolygon (GMT->PSL, tri_x, tri_y, 4);
		else if (n_sides == 2) {	/* Must find the open jaw */
			if (GMT->current.map.frame.side[S_SIDE] == 0)
				PSL_plotline (GMT->PSL, &tri_x[1], &tri_y[1], 3, PSL_MOVE + PSL_STROKE);
			else if (GMT->current.map.frame.side[W_SIDE] == 0)
				PSL_plotline (GMT->PSL, tri_x, tri_y, 3, PSL_MOVE + PSL_STROKE);
			else {	/* Must order the coordinates for this one */
				double xx[3], yy[3];
				xx[0] = tri_x[2]; xx[1] = tri_x[0]; xx[2] = tri_x[1];
				yy[0] = tri_y[2]; yy[1] = tri_y[0]; yy[2] = tri_y[1];
				PSL_plotline (GMT->PSL, xx, yy, 3, PSL_MOVE + PSL_STROKE);
			}
		}
		else if (GMT->current.map.frame.side[S_SIDE])
			PSL_plotline (GMT->PSL, &tri_x[0], &tri_y[0], 2, PSL_MOVE + PSL_STROKE);
		else if (GMT->current.map.frame.side[E_SIDE])
			PSL_plotline (GMT->PSL, &tri_x[1], &tri_y[1], 2, PSL_MOVE + PSL_STROKE);
		else if (GMT->current.map.frame.side[W_SIDE])
			PSL_plotline (GMT->PSL, &tri_x[2], &tri_y[2], 2, PSL_MOVE + PSL_STROKE);
	}

	/* Now do axis annotations */
	if (GMT->current.map.frame.side[S_SIDE]) {	/* The horizontal axis, called A axis */
		char code = (GMT->current.map.frame.side[S_SIDE] & 2) ? 'S' : 's';
		sprintf (cmd, "-R%g/%g/0/1 -JX%gi -O -K -B%c \"-B%s\"", wesn_orig[XLO], wesn_orig[XHI], GMT->current.map.width, code, &boptions[GMT_X]->arg[1]);
		gmt_init_B (GMT);
		PSL_comment (GMT->PSL, "Draw axis A (south mode positive x) with origin at 0,0 and no rotation\n");
		if ((error = GMT_Call_Module (API, "psbasemap", GMT_MODULE_CMD, cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to plot A axis\n");
			Return (API->error);
		}
		gmt_M_str_free (boptions[GMT_X]->arg);
		gmt_M_free (GMT, boptions[GMT_X]);
	}
	if (GMT->current.map.frame.side[E_SIDE]) {	/* The right axis, called B axis */
		char code = (GMT->current.map.frame.side[E_SIDE] & 2) ? 'N' : 'n';
		x = rect[XHI] * (1.0 - SQRT3) * 0.5;
		y = -x;
		PSL_comment (GMT->PSL, "Draw axis B (north mode, negative x) after translating to %g, %g and rotating by -60 degrees.\n", x,y);
		PSL_setorigin (PSL, x, y, -60.0, PSL_FWD);
		sprintf (cmd, "-R%g/%g/0/1 -JX%gi -O -K -B%c \"-B%s\"", wesn_orig[YLO], wesn_orig[YHI], -GMT->current.map.width, code, &boptions[GMT_Y]->arg[1]);
		gmt_init_B (GMT);
		if ((error = GMT_Call_Module (API, "psbasemap", GMT_MODULE_CMD, cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to plot B axis\n");
			Return (API->error);
		}
		PSL_setorigin (PSL, -x, -y, 60.0, PSL_INV);
		gmt_M_str_free (boptions[GMT_Y]->arg);
		gmt_M_free (GMT, boptions[GMT_Y]);
	}
	if (GMT->current.map.frame.side[W_SIDE]) {	/* The left axis, called C */
		char code = (GMT->current.map.frame.side[W_SIDE] & 2) ? 'N' : 'n';
		x = 0.5 * SQRT3 * rect[XHI];
		y = -0.5 * rect[XHI];
		PSL_comment (GMT->PSL, "Draw axis C (north mode, negative x) after translating to %g, %g and rotating by 60 degrees.\n", x,y);
		PSL_setorigin (PSL, x, y, 60.0, PSL_FWD);
		sprintf (cmd, "-R%g/%g/0/1 -JX%gi -O -K -B%c \"-B%s\"", wesn_orig[ZLO], wesn_orig[ZHI], -GMT->current.map.width, code, &boptions[GMT_Z]->arg[1]);
		gmt_init_B (GMT);
		if ((error = GMT_Call_Module (API, "psbasemap", GMT_MODULE_CMD, cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to plot C axis\n");
			Return (API->error);
		}
		PSL_setorigin (PSL, -x, -y, -60.0, PSL_INV);
		gmt_M_str_free (boptions[GMT_Z]->arg);
		gmt_M_free (GMT, boptions[GMT_Z]);
	}

	if (Ctrl->S.active) {	/* Plot symbols */
		char vfile[GMT_LEN16] = {""};
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, D, vfile) == GMT_NOTSET) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a virtual data set\n");
			Return (API->error);
		}
		sprintf (cmd, "-R0/1/0/1 -JX%gi -O -K -S%s %s", GMT->current.map.width, Ctrl->S.string, vfile);
		if (Ctrl->C.active) {strcat (cmd, " -C"); strcat (cmd, Ctrl->C.string);}
		else if (Ctrl->G.active) {strcat (cmd, " -G"); strcat (cmd, Ctrl->G.string);}
		if (Ctrl->W.active) {strcat (cmd, " -W"); strcat (cmd, Ctrl->W.string);}
		if ((error = GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to plot symbols\n");
			Return (API->error);
		}
		if (GMT_Close_VirtualFile (API, vfile) != GMT_NOERROR)
			return (API->error);
	}
	
	/* Done plotting */
	
	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	GMT_Report (API, GMT_MSG_VERBOSE, "Done!\n");

	Return (GMT_NOERROR);
}
