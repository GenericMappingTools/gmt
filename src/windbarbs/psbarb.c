/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: psbarb will read <x,y,z> triplets and plot wind barbs
 * in a 3-D perspective view.
 *
 * Author:	HIGAKI, Masakazu
 * Date:	1-JUN-2017
 * Version:	6 API
 */

#include "gmt_dev.h"
#include "windbarb.h"
#include "longopt/psbarb_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"psbarb"
#define THIS_MODULE_MODERN_NAME	"barb"
#define THIS_MODULE_LIB		"windbarbs"
#define THIS_MODULE_PURPOSE	"Plot wind barbs in 3-D"
#define THIS_MODULE_KEYS	"<D{,CC(,T-<,>X},S?(=2"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYabdefhiptxy" GMT_OPT("EZHMmc")

EXTERN_MSC int GMT_psbarb(void *API, int mode, void *args);

/* Control structure for psbarb */

struct PSBARB_CTRL {
	struct C {	/* -C<cpt> or -C<color1>,<color2>[,<color3>,...] */
		bool active;
		char *file;
	} C;
	struct D {	/* -D<dx>/<dy>[/<dz>] */
		bool active;
		double dx, dy, dz;
	} D;
	struct G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I<intensity> */
		bool active;
		double value;
	} I;
	struct N {	/* -N[r|c] */
		bool active;
		unsigned int mode;
	} N;
	struct Q {	/* -Q<size>[+<mods>] */
		struct GMT_BARB_ATTR B;
	} Q;
	struct T {	/* -T [Deprecated] */
		bool active;
	} T;
	struct W {	/* -W<pen>[+c[l|f]] */
		bool active;
		bool cpt_effect;
		struct GMT_PEN pen;
	} W;
};

enum Psbarb_cliptype {
	PSBARB_CLIP_REPEAT 	= 0,
	PSBARB_CLIP_NO_REPEAT,
	PSBARB_NO_CLIP_REPEAT,
	PSBARB_NO_CLIP_NO_REPEAT};

struct PSBARB_DATA {
	int outline;
	double x, y, z, dim[PSL_MAX_DIMS];
	struct GMT_FILL f;
	struct GMT_PEN p;
};

EXTERN_MSC double gmt_half_map_width (struct GMT_CTRL *GMT, double y);

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSBARB_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSBARB_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->W.pen = GMT->current.setting.map_default_pen;
	gmt_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* Default is no fill */
	C->N.mode = PSBARB_CLIP_REPEAT;
	C->Q.B.width = 0.1f;	C->Q.B.length = 0.2f;	C->Q.B.angle = 120.0f;	C->Q.B.scale = 5.0f;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSBARB_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psbarb synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] %s %s [%s] [-C<cpt>] [-D<dx>/<dy>] [-G<fill>|+z] "
		"[-I[<intens>]] %s [-N[c|r]] %s%s [-Q[<params>]] [%s] [%s] [-W[<pen>][<attr>]] [%s] [%s] "
		"[-Z<value>|<file>[+t|T]] [%s] [%s] %s[%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_J_OPT, GMT_Rgeoz_OPT, GMT_B_OPT, API->K_OPT, API->O_OPT, API->P_OPT,
		GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_a_OPT, GMT_bi_OPT, API->c_OPT,
		GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_l_OPT, GMT_p_OPT, GMT_q_OPT, GMT_tv_OPT,
		GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<,J-Z,R3");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Option (API, "B-");
	GMT_Usage (API, 1, "\n-C<cpt>|<color1>,<color2>[,<color3>,...]");
	GMT_Usage (API, -2, "Assign symbol colors based on z-value in 3rd column. ");
	GMT_Usage (API, 1, "\n-D<dx>/<dy>.");
	GMT_Usage (API, -2, "Offset symbol or line positions by <dx>/<dy> [no offset].");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify color or pattern [Default is no fill].");
	GMT_Usage (API, -2, "The -G option can be present in all segment headers. "
		"To assign fill color via -Z, give -G+z).");
	GMT_Usage (API, 1, "\n-I[<intens>]");
	GMT_Usage (API, -2, "Use the intensity to modulate the fill color (requires -C or -G). "
		"If no intensity is given we expect it to follow symbol size in the data record.");
	GMT_Option (API, "K");
	GMT_Usage (API, 1, "\n-N[c|r]");
	GMT_Usage (API, -2, "Do Not skip or clip symbols that fall outside the map border [clipping is on]:");
	GMT_Usage (API, 3, "r: Turn off clipping and plot repeating symbols for periodic maps.");
	GMT_Usage (API, 3, "c: Retain clipping but turn off plotting of repeating symbols for periodic maps.");
	GMT_Usage (API, -2, "[Default will clip or skip symbols that fall outside and plot repeating symbols].");
	GMT_Usage (API, -2, "Note: May also be used with lines or polygons but no periodicity will be honored.");
	GMT_Option (API, "O,P");
	gmt_barb_syntax (API->GMT, 'Q', "Modify wind barb attributes.", 1);
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Set pen attributes [Default pen is %s].", NULL, 8);
	GMT_Usage (API, 2, "To assign pen outline color via -Z, append +z.");
	GMT_Option (API, "X");
	GMT_Usage (API, 1, "\n-Z<value>|<file>[+t|T]");
	GMT_Usage (API, -2, "Use <value> with -C<cpt> to determine <color> instead of via -G<color> or -W<pen>. "
		"Use <file> to get per-line or polygon <values>. Two modifiers can also be used: ");
	GMT_Usage (API, 3, "+t Expect transparency (0-100%%) instead of z-value in the last column of <file>.");
	GMT_Usage (API, 3, "+T Expect both transparency (0-100%%) and z-value in last two columns of <file>.");
	GMT_Usage (API, -2, "Control if the outline or fill (for polygons only) are affected: ");
	GMT_Usage (API, 3, "%s To use <color> for fill, also select -G+z. ", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s To use <color> for an outline pen, also select -W<pen>+z.", GMT_LINE_BULLET);
	GMT_Option (API, "a,bi");
	if (gmt_M_showusage (API)) GMT_Usage (API, -2, "Default <ncols> is the required number of columns.");
	GMT_Option (API, "c,di,e,f,g,h,i,l,p,qi,T,w,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int parse_old_W (struct GMTAPI_CTRL *API, struct PSBARB_CTRL *Ctrl, char *text) {
	unsigned int j = 0, n_errors = 0;
	if (text[j] == '-') {Ctrl->W.pen.cptmode = 1; j++;}
	if (text[j] == '+') {Ctrl->W.pen.cptmode = 3; j++;}
	if (text[j] && gmt_getpen (API->GMT, &text[j], &Ctrl->W.pen)) {
		gmt_pen_syntax (API->GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", NULL, 8);
		n_errors++;
	}
	return n_errors;
}
GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSBARB_CTRL *Ctrl, struct GMT_OPTION *options, struct GMT_SYMBOL *S) {
	/* This parses the options provided to psbarb and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	int n;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Vary symbol color with z */
				if (opt->arg[0]) {
					gmt_M_str_free (Ctrl->C.file);
					Ctrl->C.file = strdup (opt->arg);
					Ctrl->C.active = true;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: No CPT given\n");
					n_errors++;
				}
				break;
			case 'D':
				if ((n = sscanf (opt->arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c)) < 2) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -D option: Give x and y [and z] offsets\n");
					n_errors++;
				}
				else {
					Ctrl->D.dx = gmt_M_to_inch (GMT, txt_a);
					Ctrl->D.dy = gmt_M_to_inch (GMT, txt_b);
					if (n == 3) Ctrl->D.dz = gmt_M_to_inch (GMT, txt_c);
					Ctrl->D.active = true;
				}
				break;
			case 'G':		/* Set color for symbol */
				Ctrl->G.active = true;
				if (!opt->arg[0] || gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'I':	/* Adjust symbol color via intensity */
				Ctrl->I.value = atof (opt->arg);
				Ctrl->I.active = true;
				break;
			case 'N':	/* Do not clip to map */
				Ctrl->N.active = true;
				if (opt->arg[0] == 'r') Ctrl->N.mode = PSBARB_NO_CLIP_REPEAT;
				else if (opt->arg[0] == 'c') Ctrl->N.mode = PSBARB_CLIP_NO_REPEAT;
				else if (opt->arg[0] == '\0') Ctrl->N.mode = PSBARB_NO_CLIP_NO_REPEAT;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: Unrecognized argument %s\n", opt->arg);
					n_errors++;
				}
				break;
			case 'Q':	/* Set wind barb parameters */
				n_errors += gmt_parse_barb (GMT, opt->arg, &Ctrl->Q.B);
				break;
			case 'T':		/* Skip all input files */
				Ctrl->T.active = true;
				break;
			case 'W':		/* Set line attributes */
				Ctrl->W.active = true;
				if (opt->arg[0] && strchr ("-+", opt->arg[0]) ) {	/* Definitively old-style args */
					if (gmt_M_compat_check (API->GMT, 5)) {	/* Sorry */
						GMT_Report (API, GMT_MSG_NORMAL, "Your -W syntax is obsolete; see program usage.\n");
						n_errors++;
					}
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Your -W syntax is obsolete; see program usage.\n");
						n_errors += parse_old_W (API, Ctrl, opt->arg);
					}
				}
				else if (opt->arg[0] && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", NULL, 8);
					n_errors++;
				}
				if (Ctrl->W.pen.cptmode) Ctrl->W.cpt_effect = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	S->read_size = (Ctrl->Q.B.length <= 0.0f);
	S->n_required = 2 + 2*S->read_size;	/* [len,width,] azim,speed */
	S->n_nondim = 2;
	S->nondim_col[0] = S->n_required+1;	/* wind azimuth */
	S->nondim_col[1] = S->n_required+2;	/* wind speed */

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && S->symbol == GMT_SYMBOL_NOT_SET, "Syntax error: Binary input data cannot have symbol information\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.pen.cptmode && !Ctrl->C.active, "Syntax error: -W modifier +c requires the -C option\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_psbarb (void *V_API, int mode, void *args) {
	/* High-level function that implements the psbarb task */
	bool penset_OK = true, old_is_world;
	bool get_rgb, clip_set = false, fill_active;
	bool default_outline, outline_active, save_u = false;
	unsigned int k, j, geometry, tbl, pos2x, pos2y, set_type;
	unsigned int n_cols_start = 3, justify, v4_outline = 0, v4_status = 0;
	unsigned int bcol, ex1, ex2, ex3, change, n_needed, read_mode;
	int error = GMT_NOERROR;

	uint64_t i, n, n_total_read = 0;
	size_t n_alloc = 0;

	char *text_rec = NULL, s_args[GMT_BUFSIZ] = {""};

	void *record = NULL;	/* Opaque pointer to either a text or double record */

	double dim[PSL_MAX_DIMS], rgb[3][4] = {{-1.0, -1.0, -1.0, 0.0}, {-1.0, -1.0, -1.0, 0.0}, {-1.0, -1.0, -1.0, 0.0}};
	double DX = 0, DY = 0, *xp = NULL, *yp = NULL, *in = NULL, *v4_rgb = NULL;
	double lux[3] = {0.0, 0.0, 0.0}, tmp, x_1, x_2, y_1, y_2, dx, dy, s, c, length;

	struct GMT_PEN default_pen, current_pen;
	struct GMT_FILL default_fill, current_fill, black, no_fill;
	struct GMT_SYMBOL S;
	struct GMT_PALETTE *P = NULL;
	struct GMT_DATASEGMENT *L = NULL;
	struct PSBARB_DATA *data = NULL;
	struct PSBARB_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	/* Initialize GMT_SYMBOL structure */

	gmt_M_memset (&S, 1, struct GMT_SYMBOL);

	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options, &S)) != 0) Return (error);

	/*---------------------------- This is the psbarb main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	GMT->current.plot.mode_3D = 1;	/* Only do background axis first; do foreground at end */

	get_rgb = Ctrl->C.active;
	gmt_init_fill (GMT, &black, 0.0, 0.0, 0.0);	/* Default fill for points, if needed */
	gmt_init_fill (GMT, &no_fill, -1.0, -1.0, -1.0);

	default_pen = current_pen = Ctrl->W.pen;
	current_fill = default_fill = (!Ctrl->G.active) ? black : Ctrl->G.fill;
	default_outline = Ctrl->W.active;
	if (Ctrl->I.active) {
		gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
		gmt_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
	}
	if (Ctrl->T.active) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Warning: Option -T ignores all input files\n");

	if (get_rgb) n_cols_start++;

	/* Extra columns 1, 2, and 3 */
	ex1 = (get_rgb) ? 4 : 3;
	ex2 = (get_rgb) ? 5 : 4;
	ex3 = (get_rgb) ? 6 : 5;
	if (!GMT->common.J.zactive) {			/* PSBARB */
		ex1--; ex2--; ex3--; n_cols_start--;
		for (j = 0; j < S.n_nondim; j++) S.nondim_col[j]--;
	}
	pos2x = ex1 + GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd longitude (for VECTORS with two sets of coordinates) */
	pos2y = ex2 - GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd latitude (for VECTORS with two sets of coordinates) */
	n_needed = n_cols_start + S.n_required;

	if (gmt_check_binary_io (GMT, n_cols_start + S.n_required))
		Return (GMT_RUNTIME_ERROR);

	for (j = n_cols_start; j < 7; j++) GMT->current.io.col_type[GMT_IN][j] = GMT_IS_DIMENSION;			/* Since these may have units appended */
	for (j = 0; j < S.n_nondim; j++) GMT->current.io.col_type[GMT_IN][S.nondim_col[j]+get_rgb] = GMT_IS_FLOAT;	/* Since these are angles or km, not dimensions */

	if (Ctrl->C.active && (P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
		Return (API->error);
	}

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), ""))
		Return (GMT_PROJECTION_ERROR);

	lux[0] = fabs (GMT->current.proj.z_project.sin_az * GMT->current.proj.z_project.cos_el);
	lux[1] = fabs (GMT->current.proj.z_project.cos_az * GMT->current.proj.z_project.cos_el);
	lux[2] = fabs (GMT->current.proj.z_project.sin_el);
	tmp = MAX (lux[0], MAX (lux[1], lux[2]));
	for (k = 0; k < 3; k++) lux[k] = (lux[k] / tmp) - 0.5;

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	if (Ctrl->T.active) {
		gmt_plotend (GMT);
		Return (GMT_NOERROR);
	}

	gmt_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */

	gmt_map_basemap (GMT);

	if (GMT->current.proj.z_pars[0] == 0.0) {	/* Only consider clipping if there is no z scaling */
		if ((gmt_M_is_conical(GMT) && gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]))) {	/* Must turn clipping on for 360-range conical */
			/* Special case of 360-range conical (which is periodic but do not touch at w=e) so we must clip to ensure nothing is plotted in the gap between west and east border */
			clip_set = true;
		}
		else if (Ctrl->N.mode == PSBARB_CLIP_REPEAT || Ctrl->N.mode == PSBARB_CLIP_NO_REPEAT)	/* Only set clip if plotting symbols and -N not used */
			clip_set = true;
	}
	if (clip_set) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);

	if (Ctrl->Q.B.status & PSL_VEC_COMPONENTS)
		GMT->current.io.col_type[GMT_IN][pos2y] = GMT_IS_FLOAT;	/* Just the users dy component, not length */
	if ((Ctrl->Q.B.status & PSL_VEC_FILL) == 0) Ctrl->G.active = false;	/* Want to fill so override -G*/
	if (Ctrl->Q.B.status & PSL_VEC_FILL2) current_fill = Ctrl->Q.B.fill;	/* Override -G<fill> (if set) with specified fill */
	if (penset_OK) gmt_setpen (GMT, &current_pen);
	fill_active = Ctrl->G.active;	/* Make copies because we will change the values */
	outline_active =  Ctrl->W.active;
	if (!outline_active && !fill_active && !get_rgb) outline_active = true;	/* If no fill nor outline for symbols then turn outline on */

	if (Ctrl->D.active) {
		/* Shift the plot a bit. This is a bit frustrating, since the only way to do this
		   easily is to undo the perspective, shift, then redo. */
		gmt_plane_perspective (GMT, -1, 0.0);
		gmt_xyz_to_xy (GMT, Ctrl->D.dx, Ctrl->D.dy, Ctrl->D.dz, &DX, &DY);
		PSL_setorigin (PSL, DX, DY, 0.0, PSL_FWD);
		gmt_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
	}
	GMT->current.io.skip_if_NaN[GMT_Z] = true;	/* Extend GMT NaN-handling to the z-coordinate */

	old_is_world = GMT->current.map.is_world;
	geometry = GMT_IS_POINT;
	if ((error = gmt_set_cols (GMT, GMT_IN, n_needed)) != GMT_NOERROR) {
		Return (error);
	}

	/* Here we can process data records (ASCII or binary) */
	set_type = GMT_IS_DATASET;
	read_mode = GMT_READ_DATA;
	in = GMT->current.io.curr_rec;

	{	/* symbol part */
		bool periodic = false;
		unsigned int n_warn[3] = {0, 0, 0}, warn, item, n_times;
		double in2[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, *p_in = GMT->current.io.curr_rec;
		double xpos[2], width, d;
		double z;	/* PSBARB */
		
		/* Determine if we need to worry about repeating periodic symbols */
		if (clip_set && (Ctrl->N.mode == PSBARB_CLIP_REPEAT || Ctrl->N.mode == PSBARB_NO_CLIP_REPEAT) && gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]) && gmt_M_is_geographic (GMT, GMT_IN)) {
			/* Only do this for projection where west and east are split into two separate repeating boundaries */
			periodic = (gmt_M_is_cylindrical (GMT) || gmt_M_is_misc (GMT));
		}
		n_times = (periodic) ? 2 : 1;	/* For periodic boundaries we plot each symbol twice to allow for periodic clipping */
		if (GMT_Init_IO (API, set_type, geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, set_type, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		gmt_set_meminc (GMT, GMT_BIG_CHUNK);	/* Only a sizeable amount of PSXZY_DATA structures when we initially allocate */
		if (S.read_size && GMT->current.io.col[GMT_IN][ex1].convert) {	/* Doing math on the size column, must delay unit conversion unless inch */
			GMT->current.io.col_type[GMT_IN][ex1] = GMT_IS_FLOAT;
		}
		if (S.read_size && GMT->current.io.col[GMT_IN][ex2].convert) {	/* Doing math on the size column, must delay unit conversion unless inch */
			GMT->current.io.col_type[GMT_IN][ex2] = GMT_IS_FLOAT;
		}
		
		API->object[API->current_item[GMT_IN]]->n_expected_fields = n_needed;
		n = 0;
		do {	/* Keep returning records until we reach EOF */
			if ((record = GMT_Get_Record (API, read_mode, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				}
				else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
					break;
				else if (gmt_M_rec_is_segment_header (GMT)) {			/* Parse segment headers */
					PSL_comment (PSL, "Segment header: %s\n", GMT->current.io.segment_header);
					(void)gmt_parse_segment_header (GMT, GMT->current.io.segment_header, P, &fill_active, &current_fill, &default_fill, &outline_active, &current_pen, &default_pen, default_outline, NULL);
					if (Ctrl->I.active) {
						gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
						gmt_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
					}
					if (gmt_parse_segment_item (GMT, GMT->current.io.segment_header, "-S", s_args)) {	/* Found -Sargs */
						if (!(s_args[0] == 'q'|| s_args[0] == 'f')) { /* Update parameters */
							gmt_parse_symbol_option (GMT, s_args, &S, 0, false);
						}
						else
							GMT_Report (API, GMT_MSG_NORMAL, "Segment header tries to switch to a line symbol like quoted line or fault - ignored\n");
					}
				}
				continue;							/* Go back and read the next record */
			}

			/* Data record to process */

			n_total_read++;

			/* Here, all in[*] beyond lon, lat, z will have been converted to inch if they had a trailing unit (e.g., 5c) */
			z = (GMT->common.J.zactive) ? in[GMT_Z] : GMT->common.R.wesn[ZLO];	/* PSBARB */
			if (!Ctrl->N.active && (z < GMT->common.R.wesn[ZLO] || z > GMT->common.R.wesn[ZHI])) continue;
			if (!Ctrl->N.active) {
				/* Skip points outside map */
				gmt_map_outside (GMT, in[GMT_X], in[GMT_Y]);
				if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
			}

			if (get_rgb) {	/* Lookup t to get rgb */
				struct GMT_PALETTE_HIDDEN *PH = NULL;
				gmt_get_fill_from_z (GMT, P, in[ex1-1], &current_fill);
				PH = gmt_get_C_hidden (P);
				if (PH->skip) continue;	/* Chosen CPT indicates skip for this t */
				if (Ctrl->I.active) gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
			}

			if (n == n_alloc) data = gmt_M_malloc (GMT, data, n, &n_alloc, struct PSBARB_DATA);

			if (gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &data[n].x, &data[n].y) || gmt_M_is_dnan(z)) continue;	/* NaNs on input */
			data[n].z = gmt_z_to_zz (GMT, z);

			if (S.read_size) {	/* Update sizes from input */
				data[n].dim[3] = in[ex1];
				data[n].dim[4] = in[ex2];
			}

			data[n].f = current_fill;
			data[n].p = current_pen;
			data[n].outline = outline_active;
			data[n].dim[2] = in[GMT_Y];	/* latitude used in gmt_draw_barb to determine the direction of barbs */

			gmt_init_barb_param (GMT, &Ctrl->Q.B, false, false, NULL, false, NULL);	/* Update wind barb parameters */
			if (Ctrl->Q.B.parsed_v4 && gmt_M_compat_check (GMT, 4)) {	/* Got v_width directly from V4 syntax so no messing with it here if under compatibility */
				/* But have to improvise as far as outline|fill goes... */
				if (outline_active) Ctrl->Q.B.status |= PSL_VEC_OUTLINE;	/* Choosing to draw outline */
				if (fill_active) Ctrl->Q.B.status |= PSL_VEC_FILL;		/* Choosing to fill barb */
				if (!(Ctrl->Q.B.status & PSL_VEC_OUTLINE) && !(Ctrl->Q.B.status & PSL_VEC_FILL)) Ctrl->Q.B.status |= PSL_VEC_OUTLINE;	/* Gotta do something */
			}
			else
				Ctrl->Q.B.pen.width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);

			if (Ctrl->Q.B.status & PSL_VEC_COMPONENTS)	/* Read dx, dy in user units */
				d = d_atan2d (in[ex2+2*S.read_size], in[ex1+2*S.read_size]);
			else
				d = in[ex1+2*S.read_size];	/* Got direction */
			if (!gmt_M_is_geographic (GMT, GMT_IN))	/* Cartesian azimuth; change to direction */
				data[n].dim[0] = 90.0 - d;
			else	/* Convert geo azimuth to map direction */
				data[n].dim[0] = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, d);
						
			if (gmt_M_is_dnan (data[n].dim[0])) {
				GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Wind barb azimuth = NaN near line %d\n", n_total_read);
				continue;
			}
			if (gmt_M_is_dnan (in[ex2+2*S.read_size])) {
				GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Wind speed = NaN near line %d\n", n_total_read);
				continue;
			}
			if (Ctrl->Q.B.status & PSL_VEC_COMPONENTS)	/* Read dx, dy in user units */
				data[n].dim[1] = hypot (in[ex1+2*S.read_size], in[ex2+2*S.read_size]);
			else
				data[n].dim[1] = in[ex2+2*S.read_size];	/* Got length */
			gmt_flip_angle_d (GMT, &data[n].dim[0]);

			justify = PSL_vec_justify (Ctrl->Q.B.status);	/* Return justification as 0-2 */
			if (justify) {
				sincosd (data[n].dim[0], &s, &c);
				dy = (S.read_size) ? data[n].dim[3] : Ctrl->Q.B.length;
				x_2 = data[n].x + dy * c;
				y_2 = data[n].y + dy * s;
				dx = justify * 0.5 * (x_2 - data[n].x);	dy = justify * 0.5 * (y_2 - data[n].y);
				data[n].x -= dx;	data[n].y -= dy;
				x_2 -= dx;		y_2 -= dy;
			}

			if (Ctrl->W.cpt_effect) {
				if (Ctrl->W.pen.cptmode & 1) {	/* Change pen color via CPT */
					gmt_M_rgb_copy (Ctrl->W.pen.rgb, current_fill.rgb);
					current_pen = Ctrl->W.pen;
				}
				if ((Ctrl->W.pen.cptmode & 2) == 0 && !Ctrl->G.active)	/* Turn off CPT fill */
					gmt_M_rgb_copy (current_fill.rgb, GMT->session.no_rgb);
				else if (Ctrl->G.active)
					current_fill = Ctrl->G.fill;
			}
			n++;
		} while (true);

		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}

		n_alloc = n;
		data = gmt_M_malloc (GMT, data, 0, &n_alloc, struct PSBARB_DATA);

		/* Now plot these symbols one at the time */

		PSL_command (GMT->PSL, "V\n");
		for (i = 0; i < n; i++) {

			/* For global periodic maps, symbols plotted close to a periodic boundary may be clipped and should appear
			 * at the other periodic boundary.  We try to handle this below */
			
			xpos[0] = data[i].x;
			if (periodic) {
				width = 2.0 * gmt_half_map_width (GMT, data[i].y);	/* Width of map at current latitude (not all projections have straight w/e boundaries */
				if (data[i].x < GMT->current.map.half_width)     /* Might reappear at right edge */
					xpos[1] = xpos[0] + width;	/* Outside the right edge */
				else      /* Might reappear at left edge */
			              xpos[1] = xpos[0] - width;         /* Outside the left edge */
			}
			for (item = 0; item < n_times; item++) {	/* Plot symbols once or twice, depending on periodic (see above) */
				gmt_plane_perspective (GMT, GMT_Z, data[i].z);
				if (S.read_size) {
					Ctrl->Q.B.length = data[i].dim[3];
					Ctrl->Q.B.width  = data[i].dim[4];
				}
				gmt_draw_barb (GMT, xpos[item], data[i].y, data[i].dim[2], data[i].dim[0], data[i].dim[1], Ctrl->Q.B, &data[i].p, &data[i].f, data[i].outline);
			}
		}
		PSL_command (GMT->PSL, "U\n");
		gmt_M_free (GMT, data);
		gmt_reset_meminc (GMT);
	}

	if (clip_set) gmt_map_clip_off (GMT);	/* We delay map clip off if text clipping was chosen via -Sq<args:+e */

	gmt_plane_perspective (GMT, -1, 0.0);

	if (Ctrl->D.active) PSL_setorigin (PSL, -DX, -DY, 0.0, PSL_FWD);	/* Shift plot a bit */

	PSL_setdash (PSL, NULL, 0);
	gmt_vertical_axis (GMT, 2);	/* Draw foreground axis */
	GMT->current.map.is_world = old_is_world;

	gmt_symbol_free (GMT, &S);

	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
