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
 * Brief synopsis: psternary reads one or many abc-files and either
 * plots symbols, draws contours, triangulates to an image, or just
 * converts a,b,c to x,y coordinates.
 *
 * Author:	Paul Wessel
 * Date:	1-APR-2017
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psternary"
#define THIS_MODULE_MODERN_NAME	"ternary"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot data on ternary diagrams"
#define THIS_MODULE_KEYS	"<D{,>X},>DM,C-(@<D{,MD),C-("
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYbdefghipqstxy"

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
	struct PSTERNARY_L {	/* -L */
		bool active;
		char vlabel[3][GMT_LEN64];
	} L;
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
	struct PSTERNARY_T {	/* -T[h|l][+d<gap>[c|i|p][/<length>[c|i|p]]][+lLH|"low,high"] */
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

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <table> [-B<args> or -Ba<args> -Bb<args> -Bc<args>] [-C<cpt>] [-G<fill>]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-JX<width>] %s[-L<a>/<b/<c> ] [-M] [-N] %s%s [-S[<symbol>][<size>]]\n", API->K_OPT, API->O_OPT, API->P_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-R<amin>/<amax>/<bmin>/<bmax>/<cmin>/<cmax>] [%s] [%s] [-W[<pen>][<attr>]]\n", GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] %s[%s] [%s]\n", GMT_X_OPT, GMT_Y_OPT, GMT_bi_OPT, API->c_OPT, GMT_di_OPT, GMT_e_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_t_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more data sets.  If none, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-B Specify axis annotations for the three axis a, b, c with separate\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ba<args> -Bb<args> -Bc<args> or a single -B<args> for all axes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Use CPT to assign symbol colors based on z-value in 3rd column (with -S), or\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   specify contours to be drawn (with -Q).\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify color or pattern [no fill].");
	GMT_Message (API, GMT_TIME_NONE, "\t-J Use -JX<width> to set the plot base width.\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Give labels for each of the 3 vertices [no labels].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Convert (a,b,c) to normalized (x,y) and write to standard output.  No plotting occurs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do not skip or clip symbols that fall outside the map border [clipping is on]\n");
	GMT_Option (API, "O,P,R");
	//GMT_Message (API, GMT_TIME_NONE, "\t-Q Selects contouring.  Optionally append <cut>.  If given, then we do not draw\n");
	//GMT_Message (API, GMT_TIME_NONE, "\t   closed contours with less than <cut> points [Draw all contours].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select symbol type and symbol size (in %s).  See psxy for full list of symbols.\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Set pen attributes [Default pen is %s]:", 15);
	GMT_Option (API, "X,bi2,c,di,e,f,g,h,i,p,qi,t,:,.");

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
			case 'C':	/* Use CPT for coloring symbols */
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->C.string);
				if (opt->arg[0]) Ctrl->C.string = strdup (opt->arg);
				break;
			case 'G':	/* Fill */
				Ctrl->G.active = true;
				gmt_M_str_free (Ctrl->G.string);
				Ctrl->G.string = strdup (opt->arg);
				break;
			case 'L':	/* Get the three labels seaprated by slashes */
				Ctrl->L.active = true;
				sscanf (opt->arg, "%[^/]/%[^/]/%s", Ctrl->L.vlabel[GMT_X], Ctrl->L.vlabel[GMT_Y], Ctrl->L.vlabel[GMT_Z]);
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

	gmt_consider_current_cpt (GMT->parent, &Ctrl->C.active, &(Ctrl->C.string));

	if (!Ctrl->M.active) {	/* Need -R -J for anything but dumping */
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
		n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Must specify -JX option\n");
		n_errors += gmt_M_check_condition (GMT, !(Ctrl->S.active || Ctrl->Q.active), "Must specify either -S or -Q\n");
		if (GMT->common.J.active) {	/* Impose our conditions on -JX */
			n_errors += gmt_M_check_condition (GMT, GMT->common.J.string[0] != 'X', "Option -J: Must specify -JX<width>\n");
			n_errors += gmt_M_check_condition (GMT, strchr (GMT->common.J.string, '/'), "Option -J: Must specify -JX<width>\n");
		}
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && !Ctrl->S.string[0], "Option -S: Must specify a symbol\n");

	n_errors += gmt_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL unsigned int prep_options (struct GMTAPI_CTRL *API, struct GMT_OPTION **options, struct GMT_OPTION *boptions[]) {
	/* Must intercept select common options and change to valid GMT syntax */
	unsigned int k, n_axis = 0;
	struct GMT_OPTION *opt = NULL, *bopt = NULL;
	char string[GMT_LEN256] = {""};

	/* Unless -M, append -Jz1 so that -Ramin/amax/bmin/bmax/cmin/cmax is accepted without specifying -Jz on command line */

	if ((opt = GMT_Find_Option (API, 'M', *options)) == NULL) {
		if ((opt = GMT_Make_Option (API, 'J', "z1i")) == NULL) return (GMT_PARSE_ERROR);
		if ((*options = GMT_Append_Option (API, opt, *options)) == NULL) return GMT_PARSE_ERROR;	/* Failure to append this option */
	}
	else if ((opt = GMT_Find_Option (API, 'J', *options)) && (opt->arg[0] != 'X' || strchr (opt->arg, '/'))) {
		GMT_Report (API, GMT_MSG_ERROR, "Only -JX<width> is available for this module\n");
		return (GMT_PARSE_ERROR);
	}

	/* Next, find any references to A|B|C|a|b|c in -B and replace with X|Y|Z|x|y|z to survive the parser */
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
		GMT_Delete_Option (API, bopt, options);	/* Remove the single -B setting now that we added 3 separate */
		return GMT_NOERROR;
	}
	else if (n_axis != 3) {
		GMT_Report (API, GMT_MSG_ERROR, "Use -B<args> for all three axis or specify -Ba<args> -Bb<args> -Bc<args>\n");
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

GMT_LOCAL char * get_gridint (struct GMT_OPTION *B) {
	unsigned int k = 1;
	char *c = NULL, *g = NULL;
	static char gopt[GMT_LEN32] = {""};
	if (B == NULL) return NULL;	/* No option... */
	if ((c = strchr (B->arg, '+')))	/* Temporarily chop off modifiers */
		c[0] = '\0';
	if ((g = strchr (B->arg, 'g')) == NULL)	{	/* No g setting */
		if (c) c[0] = '+';	/* Restore modifiers */
		return NULL;
	}
	/* Here we got a g setting */
	gopt[0] = 'g';
	while (g[k] && isdigit (g[k])) {
		gopt[k] = g[k];
		k++;
	}
	gopt[k] = '\0';
	if (c) c[0] = '+';	/* Restore modifiers */
	return (gopt);
}

GMT_LOCAL char * get_Bsetting (struct GMT_OPTION *B) {
	size_t len, sofar;
	char *c = NULL, *g = NULL;
	static char bopt[GMT_LEN64] = {""};
	if (B == NULL) return NULL;	/* No option... */
	if ((g = get_gridint (B)) == NULL) return (&B->arg[1]);	/* No gridlines requested so use the entire thing */

	c = strstr (B->arg, g);	/* Start of g[<pars>] */
	if (c) c[0] = '\0';	/* Hide g for now */
	strncpy (bopt, &B->arg[1], 63U);	/* Place start of b up to g[<pars>] in bopt, skipping the leading a,b,c */
	if (c) c[0] = 'g';	/* Restore the g */
	sofar = strlen (bopt);
	len = strlen (g);	/* How long is the g thing? */
	if (c && c[len] && (sofar+len) < GMT_LEN64) strncat (bopt, &c[len], GMT_LEN64-1);	/* Append the rest */
	return (bopt);
}

#define SQRT3 1.73205080756887729352	/* sqrt(3) */

GMT_LOCAL void abc_to_xy (double a, double b, double c, double *x, double *y) {
	double s = (a + b + c);
	*x = 0.5 * (2.0 * b + c) / s;
	*y = 0.5 * SQRT3 * c / s;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC void gmt_set_dataset_minmax (struct GMT_CTRL *GMT, struct GMT_DATASET *D);
#define PSL_IZ(PSL,z) ((int)lrint ((z) * PSL->internal.dpu))

int GMT_ternary (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: ternary\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psternary (V_API, mode, args);
}

int GMT_psternary (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int n_sides = 0, side[3];
	uint64_t tbl, seg, row, col, k;

	bool clip_set = false;

	char cmd[GMT_LEN256] = {""}, code, *name = "ABC", cmode[3] = {""}, *g = NULL;

	double x, y, rect[4] = {0.0, 0.0, 0.0, 0.0}, tri_x[4] = {0.0, 0.0, 0.0, 0.0}, tri_y[4] = {0.0, 0.0, 0.0, 0.0};
	double width, height, L_off, T_off, wesn_orig[6], x_origin[3], y_origin[3], rot[3], sign[3];

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

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	if ((error = prep_options (API, &options, boptions))) bailout (error);	/* Enforce common option syntax temporarily to get passed GMT_Parse_Common */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psternary main code ----------------------------*/

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
	gmt_set_dataset_minmax (GMT, D);		/* Update column stats */

	if (Ctrl->M.active) {	/* Just print the converted data and exit */
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, 0, NULL, NULL, D) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to write x,y file to stdout\n");
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
	width = GMT->current.map.width;
	height = 0.5 * SQRT3 * width;

	/* First deal with the ternary "psbasemap" task.  This is complicated since there is no ternary
	 * map projection and region settings in GMT core.  We simulate the ternary diagram using
	 * regular psbasemap calls (one per active axis),  Because gridlines must be clipped to inside
	 * the triangle we must call psbasemap separately for gridlines and everything else, on a
	 * per axis case.  We must therefore separate the gridline arguments (if any) from the remaining
	 * axis arguments.  We also must handle the cancas filling separately.  The three axis are 60 degrees
	 * relative to each other and we do this directly with PSL calls. */

	if (GMT->current.map.frame.paint) {	/* Paint the inside of the map with specified fill */
		gmt_setfill (GMT, &GMT->current.map.frame.fill, 0);
		PSL_plotpolygon (PSL, tri_x, tri_y, 4);
		GMT->current.map.frame.paint = false;
	}
	/* Count how many of the three sides will be drawn */
	if (GMT->current.map.frame.side[S_SIDE]) n_sides++;	/* The bottom (a) side */
	if (GMT->current.map.frame.side[E_SIDE]) n_sides++;	/* The right (b) side */
	if (GMT->current.map.frame.side[W_SIDE]) n_sides++;	/* The left (c) side */
	if (n_sides) {	/* Draw some or all of the triangular sides */
		PSL_comment (PSL, "Draw triangular frame sides\n");
		gmt_setpen (GMT, &GMT->current.setting.map_frame_pen);
		gmt_setfill (GMT, NULL, 1);
		if (n_sides == 3)
			PSL_plotpolygon (PSL, tri_x, tri_y, 4);
		else if (n_sides == 2) {	/* Must find the open jaw */
			if (GMT->current.map.frame.side[S_SIDE] == GMT_AXIS_NONE)
				PSL_plotline (PSL, &tri_x[1], &tri_y[1], 3, PSL_MOVE|PSL_STROKE);
			else if (GMT->current.map.frame.side[W_SIDE] == GMT_AXIS_NONE)
				PSL_plotline (PSL, tri_x, tri_y, 3, PSL_MOVE|PSL_STROKE);
			else {	/* Must order the coordinates for this one */
				double xx[3], yy[3];
				xx[0] = tri_x[2]; xx[1] = tri_x[0]; xx[2] = tri_x[1];
				yy[0] = tri_y[2]; yy[1] = tri_y[0]; yy[2] = tri_y[1];
				PSL_plotline (PSL, xx, yy, 3, PSL_MOVE|PSL_STROKE);
			}
		}
		else if (GMT->current.map.frame.side[S_SIDE] & GMT_AXIS_DRAW)
			PSL_plotline (PSL, &tri_x[0], &tri_y[0], 2, PSL_MOVE|PSL_STROKE);
		else if (GMT->current.map.frame.side[E_SIDE] & GMT_AXIS_DRAW)
			PSL_plotline (PSL, &tri_x[1], &tri_y[1], 2, PSL_MOVE|PSL_STROKE);
		else if (GMT->current.map.frame.side[W_SIDE] & GMT_AXIS_DRAW)
			PSL_plotline (PSL, &tri_x[2], &tri_y[2], 2, PSL_MOVE|PSL_STROKE);
	}

	L_off = 3.0 * GMT->current.setting.map_label_offset;	T_off = 2.0 * GMT->current.setting.map_title_offset;
	if (GMT->current.map.frame.header[0]) {	/* Plot title */
		int form = gmt_setfont (GMT, &GMT->current.setting.font_title);
		PSL_comment (PSL, "Placing plot title\n");
		PSL_plottext (PSL, tri_x[2], tri_y[2]+2.0*T_off, GMT->current.setting.font_title.size, GMT->current.map.frame.header, 0.0, PSL_BC, form);
	}
	if (Ctrl->L.active) {	/* Plot the vertex labels */
		double dx = L_off * cosd (30.0), dy = L_off * sind (30.0);
		int form = gmt_setfont (GMT, &GMT->current.setting.font_label);
		PSL_comment (PSL, "Placing vertices labels\n");
		PSL_plottext (PSL, -dx, -dy, GMT->current.setting.font_label.size, Ctrl->L.vlabel[GMT_X], 0.0, PSL_TR, form);
		PSL_plottext (PSL, tri_x[1]+dx, -dy, GMT->current.setting.font_label.size, Ctrl->L.vlabel[GMT_Y], 0.0, PSL_TL, form);
		PSL_plottext (PSL, tri_x[2], tri_y[2]+L_off, GMT->current.setting.font_label.size, Ctrl->L.vlabel[GMT_Z], 0.0, PSL_BC, form);
	}

	/* Now do axis annotations.  First set array args per axis: */
	x_origin[0] = 0.0;	y_origin[0] = 0.0;	rot[0] = 0.0;	sign[0] = +1;	side[0] = GMT->current.map.frame.side[S_SIDE]; cmode[0] = 'S';	/* S_SIDE settings */
	x_origin[1] = -width * 0.25;	y_origin[1] = 0.5 * height;	rot[1] = -60.0;	sign[1] = -1;	side[1] = GMT->current.map.frame.side[E_SIDE]; cmode[1] = 'N';	/* E_SIDE settings */
	x_origin[2] = 0.75 * width;	y_origin[2] = -0.5 * height;	rot[2] = 60.0;	sign[2] = -1;	side[2] = GMT->current.map.frame.side[W_SIDE]; cmode[2] = 'N';	/* W_SIDE settings */

	for (k = 0; k <= GMT_Z; k++) {	/* Plot the 3 axes for -B settings that have been stripped of gridline requests */
		if (side[k] == 0) continue;	/* Did not want this axis drawn */
		code = (side[k] & 2) ? cmode[k] : (char)tolower (cmode[k]);
		sprintf (cmd, "-R%g/%g/0/1 -JX%gi/%gi -O -K -B%c \"-B%s\"", wesn_orig[2*k], wesn_orig[2*k+1], sign[k]*width, height, code, get_Bsetting (boptions[k]));
		gmt_init_B (GMT);
		PSL_comment (PSL, "Draw axis %c with origin at %g, %g and rotation = %g\n", name[k], x_origin[k], y_origin[k], rot[k]);
		PSL_setorigin (PSL, x_origin[k], y_origin[k], rot[k], PSL_FWD);
		if ((error = GMT_Call_Module (API, "psbasemap", GMT_MODULE_CMD, cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to plot %c axis\n", name[k]);
			Return (API->error);
		}
		PSL_setorigin (PSL, -x_origin[k], -y_origin[k], -rot[k], PSL_INV);
	}

	/* Deal with gridline requests separately */
	for (k = 0; k <= GMT_Z; k++) {
		if (side[k] == 0) continue;	/* Did not want this axis drawn */
		if ((g = get_gridint (boptions[k])) == NULL) continue;	/* No grid interval for this axis */
		if (!clip_set) {	/* OK, so we do need to lay down clip path */
			PSL_comment (PSL, "Activate Map clip path for Ternary diagram\n");
			PSL_beginclipping (PSL, tri_x, tri_y, 4, GMT->session.no_rgb, 3);
			clip_set = true;
		}
		sprintf (cmd, "-R0/1/0/1 -JX%gi/%gi -O -K -B+n -By%s", width, height, g);
		gmt_init_B (GMT);
		PSL_comment (PSL, "Draw gridlines axis %c with origin at %g, %g and rotation = %g\n", name[k], x_origin[k], y_origin[k], rot[k]);
		PSL_setorigin (PSL, x_origin[k], y_origin[k], rot[k], PSL_FWD);
		if ((error = GMT_Call_Module (API, "psbasemap", GMT_MODULE_CMD, cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to plot gridlines for %c axis\n", name[k]);
			Return (API->error);
		}
		PSL_setorigin (PSL, -x_origin[k], -y_origin[k], -rot[k], PSL_INV);
	}
	if (clip_set) PSL_endclipping (PSL, 1);

	for (k = 0; k <= GMT_Z; k++) {
		if (GMT_Free_Option (API, &boptions[k])) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to free -B? option\n");
			Return (API->error);
		}
	}
	if (Ctrl->S.active) {	/* Plot symbols */
		char vfile[GMT_VF_LEN] = {""};
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, D, vfile) == GMT_NOTSET) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create a virtual data set\n");
			Return (API->error);
		}
		sprintf (cmd, "-R0/1/0/1 -JX%gi -O -K -S%s %s", width, Ctrl->S.string, vfile);
		if (Ctrl->C.active) {strcat (cmd, " -C"); if (Ctrl->C.string) strcat (cmd, Ctrl->C.string);}
		else if (Ctrl->G.active) {strcat (cmd, " -G"); strcat (cmd, Ctrl->G.string);}
		if (Ctrl->W.active) {strcat (cmd, " -W"); strcat (cmd, Ctrl->W.string);}
		if ((error = GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to plot symbols\n");
			Return (API->error);
		}
		if (GMT_Close_VirtualFile (API, vfile) != GMT_NOERROR)
			return (API->error);
	}

	/* Done plotting */

	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
