/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: psxyz will read <x,y,z> triplets and plot symbols, lines,
 * or polygons in a 3-D perspective view.
 */

#include "pslib.h"
#include "gmt.h"

EXTERN_MSC GMT_LONG GMT_parse_symbol_option (struct GMT_CTRL *C, char *text, struct GMT_SYMBOL *p, GMT_LONG mode, GMT_LONG cmd);

/* Control structure for psxyz */

struct PSXYZ_CTRL {
	struct A {	/* -A[step] {NOT IMPLEMENTED YET} */
		GMT_LONG active;
		double step;
	} A;
	struct C {	/* -C<cpt> */
		GMT_LONG active;
		char *file;
	} C;
	struct D {	/* -D<dx>/<dy>[/<dz>] */
		GMT_LONG active;
		double dx, dy, dz;
	} D;
	struct G {	/* -G<fill> */
		GMT_LONG active;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I<intensity> */
		GMT_LONG active;
		double value;
	} I;
	struct L {	/* -L */
		GMT_LONG active;
	} L;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct Q {	/* -Q */
		GMT_LONG active;
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
		char *arg;
	} S;
	struct W {	/* -W<pen> */
		GMT_LONG active;
		GMT_LONG mode;	/* 0 = normal, 1 = -C applies to pen color only, 2 = -C applies to symbol fill & pen color */
		struct GMT_PEN pen;
	} W;
};

struct PSXYZ_DATA {
	GMT_LONG symbol, flag, outline;
	double x, y, z, dim[PSL_MAX_DIMS], dist[2];
	struct GMT_FILL f;
	struct GMT_PEN p;
	struct GMT_VECT_ATTR v;
	char *string;
	struct GMT_CUSTOM_SYMBOL *custom;
};

void *New_psxyz_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSXYZ_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSXYZ_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->W.pen = GMT->current.setting.map_default_pen;
	GMT_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* Default is no fill */
	C->A.step = GMT->current.setting.map_line_step;
	return (C);
}

void Free_psxyz_Ctrl (struct GMT_CTRL *GMT, struct PSXYZ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->C.file) free (C->C.file);
	if (C && C->S.arg) free (C->S.arg);
	GMT_free (GMT, C);
}

GMT_LONG GMT_psxyz_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the psxyz synopsis and optionally full usage information */

	GMT_message (GMT, "psxyz %s [API] - Plot lines, polygons, and symbols in 3-D\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: psxyz [<table>] %s %s\n", GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_message (GMT, "\t[%s] [%s] [-C<cpt>] [-D<dx>/<dy>[/<dz>]] [-G<fill>] [-I<intens>]\n", GMT_B_OPT, GMT_Jz_OPT);
	GMT_message (GMT, "\t[-K] [-L] [-N] [-O] [-P] [-Q] [-S<symbol><size>|+s<scaling>[/size_y]]\n\t[%s] [%s] [-W[+|-][<pen>]] [%s]\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n", GMT_Y_OPT, GMT_a_OPT, GMT_bi_OPT, GMT_c_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT);
	GMT_message (GMT, "\t[%s] [%s]\n\t[%s] [%s]\n\n", GMT_i_OPT, GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_explain_options (GMT, "jZr");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<b");
	GMT_message (GMT, "\t-C Use cpt-file to assign symbol colors based on t-value in 4th column\n");
	GMT_message (GMT, "\t   Note: requires -S.  Without -S, psxyz excepts lines/polygons\n");
	GMT_message (GMT, "\t   and looks for -Z<val> options in each multiheader.  Then, color is\n");
	GMT_message (GMT, "\t   applied for polygon fill (-L) or polygon pen (no -L).\n");
	GMT_message (GMT, "\t-D Offset symbol or line positions by <dx>/<dy>[/<dz>] [no offset].\n");
	GMT_fill_syntax (GMT, 'G', "Specify color or pattern [Default is no fill].");
	GMT_message (GMT, "\t   If -G is specified but not -S, then psxyz draws a filled polygon.\n");
	GMT_message (GMT, "\t-I Use the intensity to modulate the fill color (requires -C or -G).\n");
	GMT_explain_options (GMT, "ZK");
	GMT_message (GMT, "\t-L Force closed polygons.\n");
	GMT_message (GMT, "\t-N Do not skip or clip symbols that fall outside the map border\n");
	GMT_message (GMT, "\t   [Default will clip or skip symbols that fall outside].\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-Q Do NOT sort symbols based on distance to viewer before plotting.\n");
	GMT_message (GMT, "\t-S Select symbol type and symbol size (in %s).  Choose between\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	GMT_message (GMT, "\t   -(xdash), +(plus), st(a)r, (b|B)ar, (c)ircle, (d)iamond, (e)llipse,\n");
	GMT_message (GMT, "\t   (f)ront, octa(g)on, (h)exagon (i)nvtriangle, (j)rotated rectangle,\n");
	GMT_message (GMT, "\t   (k)ustom, (l)etter, (m)athangle, pe(n)tagon, c(o)lumn, (p)oint,\n");
	GMT_message (GMT, "\t   (q)uoted line, (r)ectangle, (R)ounded rectangle, (s)quare, (t)riangle,\n");
	GMT_message (GMT, "\t   c(u)be, (v)ector, (w)edge, (x)cross, (y)dash, (z)dash, or\n");
	GMT_message (GMT, "\t   =(geovector, i.e., great circle vectors).\n");
	GMT_message (GMT, "\t   If no size is specified, then the 4th column must have sizes and\n");
	GMT_message (GMT, "\t   you may append +s<scale>[unit][/<origin>][l] to convert the given data\n");
	GMT_message (GMT, "\t   as size = (data - origin) * scale, using log10 if l is appended.\n");
	GMT_message (GMT, "\t   [Default scale is 1 and origin is 0 (1 if log10)].\n");
	GMT_message (GMT, "\t   If no symbol is specified, then the last column must have symbol codes.\n");
	GMT_message (GMT, "\t   [Note: if -C is selected then 4th means 5th column, etc.]\n");
	GMT_message (GMT, "\t   column and cube are true 3-D objects (give size as xsize/ysize);\n");
	GMT_message (GMT, "\t   all other symbols are shown in 2-D perspective only.\n");
	GMT_message (GMT, "\t   By default, the 3-D symbols column and cube are shaded;\n");
	GMT_message (GMT, "\t   use upper case O and U to disable this 3-D illumination.\n");
	GMT_message (GMT, "\t   Symbols A, C, D, F, H, I, N, S, T are adjusted to have same area\n");
	GMT_message (GMT, "\t   of a circle of given dimater.\n");
	GMT_message (GMT, "\t   Bar (or Column): Append b<base> to give the y- (or z-) value of the\n");
	GMT_message (GMT, "\t      base [Default = 0 (1 for log-scales)]. Use -SB for horizontal\n");
	GMT_message (GMT, "\t      bars; then base value refers to the x location.\n");
	GMT_message (GMT, "\t   Ellipses: Direction, major, and minor axis must be in columns 4-6.\n");
	GMT_message (GMT, "\t     If -SE rather than -Se is selected, psxy will expect azimuth, and\n");
	GMT_message (GMT, "\t     axes in km, and convert azimuths based on map projection.\n");
	GMT_message (GMT, "\t     If projection is linear then we scale the axes by the map scale.\n");
	GMT_message (GMT, "\t   Rotatable Rectangle: Direction, x- and y-dimensions in columns 4-6.\n");
	GMT_message (GMT, "\t     If -SJ rather than -Sj is selected, psxy will expect azimuth, and\n");
	GMT_message (GMT, "\t     dimensions in km and convert azimuths based on map projection.\n");
	GMT_message (GMT, "\t     For linear projection we scale dimensions by the map scale.\n");
	GMT_message (GMT, "\t   Fronts: Give tickgap/ticklen[+l|+r][+<type>][+o<offset>], where\n");
	GMT_message (GMT, "\t     +l or +r   : Plot symbol to left or right of front [centered]\n");
	GMT_message (GMT, "\t     +<type>    :  +b(ox), +c(ircle), +f(ault), +s(lip), +t(riangle) [f]\n");
	GMT_message (GMT, "\t       box      : square when centered, half-square otherwise.\n");
	GMT_message (GMT, "\t       circle   : full when centered, half-circle otherwise.\n");
	GMT_message (GMT, "\t       fault    : centered cross-tick or tick only in specified direction.\n");
	GMT_message (GMT, "\t       slip     : left-or right-lateral strike-slip arrows.\n");
	GMT_message (GMT, "\t       triangle : diagonal square when centered, directed triangle otherwise.\n");
	GMT_message (GMT, "\t     +<offset>  : Plot first symbol when along-front distance is offset [0].\n");
	GMT_message (GMT, "\t   Kustom: Append <symbolname> immediately after 'k'; this will look for\n");
	GMT_message (GMT, "\t     <symbolname>.def in the current directory, in $GMT_USERDIR,\n");
	GMT_message (GMT, "\t     or in $GMT_SHAREDIR (searched in that order).\n");
	GMT_list_custom_symbols (GMT);
	GMT_message (GMT, "\t   Letter: append /<string> after symbol size, and optionally %%<font>\n");
	GMT_message (GMT, "\t   Mathangle: start/stop directions of math angle must be in columns 4-5.\n");
	GMT_message (GMT, "\t     If -SM rather than -Sm is used, we draw straight angle symbol if 90 degrees.\n");
	GMT_vector_syntax (GMT, 0);
	GMT_message (GMT, "\t   Quoted line (z must be constant): Give [d|f|n|l|x]<info>[:<labelinfo>]\n");
	GMT_message (GMT, "\t     <code><info> controls placement of labels along lines.  Select\n");
	GMT_cont_syntax (GMT, 7, 1);
	GMT_message (GMT, "\t     <labelinfo> controls the label attributes.  Choose from\n");
	GMT_label_syntax (GMT, 7, 1);
	GMT_message (GMT, "\t   Rectangles: x- and y-dimensions must be in columns 4-5.\n");
	GMT_message (GMT, "\t   Rounded rectangles: x- and y-dimensions and corner radius must be in columns 3-5.\n");
	GMT_message (GMT, "\t   Vectors: Direction and length must be in columns 4-5.\n");
	GMT_message (GMT, "\t     If -SV rather than -Sv is use, psxy will expect azimuth and\n");
	GMT_message (GMT, "\t     length and convert azimuths based on the chosen map projection.\n");
	GMT_vector_syntax (GMT, 3);
	GMT_explain_options (GMT, "U");
	GMT_message (GMT, "\t   Wedges: Start and stop directions of wedge must be in columns 3-4.\n");
	GMT_message (GMT, "\t     If -SW rather than -Sw is selected, specify two azimuths instead.\n");
	GMT_message (GMT, "\t   Geovectors: Azimuth and length (in km) must be in columns 3-4.\n");
	GMT_vector_syntax (GMT, 3);
	GMT_explain_options (GMT, "V");
	GMT_pen_syntax (GMT, 'W', "Set pen attributes [Default pen is %s]:");
	GMT_message (GMT, "\t   Implicitly draws symbol outline with this pen.\n");
	GMT_message (GMT, "\t   A leading + applies cpt color (-C) to both symbol fill and pen.\n");
	GMT_message (GMT, "\t   A leading - applies cpt color (-C) to the pen only.\n");
	GMT_explain_options (GMT, "XaC0c");
	GMT_message (GMT, "\t   Default is the required number of columns.\n");
	GMT_explain_options (GMT, "fghipt:.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_psxyz_parse (struct GMTAPI_CTRL *C, struct PSXYZ_CTRL *Ctrl, struct GMT_OPTION *options, struct GMT_SYMBOL *S)
{
	/* This parses the options provided to psxyz and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j, n, n_errors = 0;
	char txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256], txt_c[GMT_TEXT_LEN256];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.file = strdup (opt->arg);
				Ctrl->C.active = TRUE;
				break;
			case 'D':
				if ((n = sscanf (opt->arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c)) < 2) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -D option: Give x and y [and z] offsets\n");
					n_errors++;
				}
				else {
					Ctrl->D.dx = GMT_to_inch (GMT, txt_a);
					Ctrl->D.dy = GMT_to_inch (GMT, txt_b);
					if (n == 3) Ctrl->D.dz = GMT_to_inch (GMT, txt_c);
					Ctrl->D.active = TRUE;
				}
				break;
			case 'G':		/* Set color for symbol or polygon */
				Ctrl->G.active = TRUE;
				if (!opt->arg[0] || GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'I':	/* Adjust symbol color via intensity */
				Ctrl->I.value = atof (opt->arg);
				Ctrl->I.active = TRUE;
				break;
			case 'L':		/* Force closed polygons */
				Ctrl->L.active = TRUE;
				break;
			case 'N':	/* Do not clip to map */
				Ctrl->N.active = TRUE;
				break;
			case 'Q':	/* Do not sort symbols based on distance */
				Ctrl->Q.active = TRUE;
				break;
			case 'S':		/* Get symbol [and size] */
				Ctrl->S.active = TRUE;
				Ctrl->S.arg = strdup (opt->arg);
				break;
			case 'W':		/* Set line attributes */
				Ctrl->W.active = TRUE;
				j = 0;
				if (opt->arg[j] == '-') {Ctrl->W.mode = 1; j++;}
				if (opt->arg[j] == '+') {Ctrl->W.mode = 2; j++;}
				if (opt->arg[j] && GMT_getpen (GMT, &opt->arg[j], &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', "sets pen attributes [Default pen is %s]:");
					GMT_report (GMT, GMT_MSG_FATAL, "\t   A leading + applies cpt color (-C) to both symbol fill and pen.\n");
					GMT_report (GMT, GMT_MSG_FATAL, "\t   A leading - applies cpt color (-C) to the pen only.\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && GMT_parse_symbol_option (GMT, Ctrl->S.arg, S, 1, TRUE), "Syntax error -S option\n");
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && S->symbol == GMT_SYMBOL_NOT_SET, "Syntax error: Binary input data cannot have symbol information\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->W.mode && !Ctrl->C.active, "Syntax error: -W option +|-<pen> requires the -C option\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

void column3D (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double z, double *dim, double rgb[3][4], GMT_LONG outline)
{
	GMT_LONG i, k;
	double x_size, y_size, z_size, sign;

	x_size = 0.5 * dim[0];
	y_size = 0.5 * dim[1];
	z_size = 0.5 * dim[2];

	for (i = 0; i < 3; i++) {
		sign = -1.0;
		k = GMT->current.proj.z_project.face[i] / 2;
		PSL_setfill (PSL, rgb[k], outline);
		switch (GMT->current.proj.z_project.face[i]) {
			case 0:	/* yz plane positive side */
				sign = 1.0;
			case 1:	/* negative side */
				GMT_plane_perspective (GMT, GMT_X, x + sign * x_size);
				PSL_plotbox (PSL, y - y_size, z - z_size, y + y_size, z + z_size);
				break;
			case 2:	/* xz plane positive side */
				sign = 1.0;
			case 3:	/* negative side */
				GMT_plane_perspective (GMT, GMT_Y, y + sign * y_size);
				PSL_plotbox (PSL, x - x_size, z - z_size, x + x_size, z + z_size);
				break;
			case 4:	/* xy plane positive side */
				sign = 1.0;
			case 5:	/* negative side */
				GMT_plane_perspective (GMT, GMT_Z, z + sign * z_size);
				PSL_plotbox (PSL, x - x_size, y - y_size, x + x_size, y + y_size);
				break;
		}
	}
}

int dist_compare (const void *a, const void *b)
{
	if (((struct PSXYZ_DATA *)a)->dist[0] < ((struct PSXYZ_DATA *)b)->dist[0]) return (-1);
	if (((struct PSXYZ_DATA *)a)->dist[0] > ((struct PSXYZ_DATA *)b)->dist[0]) return (1);
	if (((struct PSXYZ_DATA *)a)->dist[1] < ((struct PSXYZ_DATA *)b)->dist[1]) return (-1);
	if (((struct PSXYZ_DATA *)a)->dist[1] > ((struct PSXYZ_DATA *)b)->dist[1]) return (1);
	return (0);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_psxyz_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_psxyz (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{	/* High-level function that implements the psxyz task */
	GMT_LONG polygon, penset_OK = TRUE, not_line, old_is_world;
	GMT_LONG get_rgb, read_symbol, clip_set = FALSE, fill_active;
	GMT_LONG default_outline, outline_active, pos2x, pos2y, set_type;
	GMT_LONG i, n, n_alloc = 0, n_total_read = 0, j, geometry, tbl, seg;
	GMT_LONG n_cols_start = 3, justify, error = GMT_NOERROR;
	GMT_LONG ex1, ex2, ex3, change, n_needed, read_mode, save_u = FALSE;

	char *text_rec = NULL;

	void *record = NULL;	/* Opaque pointer to either a text or double record */

	double dim[PSL_MAX_DIMS], rgb[3][4] = {{-1.0, -1.0, -1.0, 0.0}, {-1.0, -1.0, -1.0, 0.0}, {-1.0, -1.0, -1.0, 0.0}};
	double DX = 0, DY = 0, *xp = NULL, *yp = NULL, *in = NULL;
	double lux[3] = {0.0, 0.0, 0.0}, tmp, x_1, x_2, y_1, y_2, dx, dy, s, c, length;

	struct GMT_PEN default_pen, current_pen;
	struct GMT_FILL default_fill, current_fill;
	struct GMT_SYMBOL S;
	struct GMT_PALETTE *P = NULL;
	struct GMT_LINE_SEGMENT *L = NULL;
	struct PSXYZ_DATA *data = NULL;
	struct PSXYZ_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_psxyz_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_psxyz_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_psxyz", &GMT_cpy);				/* Save current state */
	if (GMT_Parse_Common (API, "-VJRbf:", "BKOPUXxYyacghipst>" GMT_OPT("EZHMm"), options)) Return (API->error);
	/* Initialize GMT_SYMBOL structure */

	GMT_memset (&S, 1, struct GMT_SYMBOL);
	GMT_contlabel_init (GMT, &S.G, 0);

	S.base = GMT->session.d_NaN;
	S.font = GMT->current.setting.font_annot[0];
	S.u = GMT->current.setting.proj_length_unit;

	Ctrl = New_psxyz_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psxyz_parse (API, Ctrl, options, &S))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the psxyz main code ----------------------------*/

	GMT->current.plot.mode_3D = 1;	/* Only do background axis first; do foreground at end */

	/* Do we plot actual symbols, or lines */
	not_line = (S.symbol != GMT_SYMBOL_FRONT && S.symbol != GMT_SYMBOL_QUOTED_LINE && S.symbol != GMT_SYMBOL_LINE);

	get_rgb = (not_line && Ctrl->C.active);
	read_symbol = (S.symbol == GMT_SYMBOL_NOT_SET);
	polygon = (S.symbol == GMT_SYMBOL_LINE && (Ctrl->G.active || Ctrl->L.active));

	default_pen = current_pen = Ctrl->W.pen;
	default_fill = current_fill = Ctrl->G.fill;
	default_outline = Ctrl->W.active;
	if (Ctrl->I.active) {
		GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
		GMT_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
	}

	if (get_rgb) n_cols_start++;

	/* Extra columns 1, 2, and 3 */
	ex1 = (get_rgb) ? 4 : 3;
	ex2 = (get_rgb) ? 5 : 4;
	ex3 = (get_rgb) ? 6 : 5;
	pos2x = ex1 + GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd longitude (for VECTORS with two sets of coordinates) */
	pos2y = ex2 - GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd latitude (for VECTORS with two sets of coordinates) */
	n_needed = n_cols_start + S.n_required;

	error += GMT_check_binary_io (GMT, n_cols_start + S.n_required);

	for (j = n_cols_start; j < 7; j++) GMT->current.io.col_type[GMT_IN][j] = GMT_IS_DIMENSION;			/* Since these may have units appended */
	for (j = 0; j < S.n_nondim; j++) GMT->current.io.col_type[GMT_IN][S.nondim_col[j]+get_rgb] = GMT_IS_FLOAT;	/* Since these are angles or km, not dimensions */

	if (Ctrl->C.active && (P = GMT_Read_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->C.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		if (GMT_contlabel_prep (GMT, &S.G, NULL)) Return (EXIT_FAILURE);
		penset_OK = FALSE;	/* Since it is set in PSL */
	}

	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	if (S.u_set) {	/* When -Sc<unit> is given we temporarily reset the system unit to these units so conversions will work */
		save_u = GMT->current.setting.proj_length_unit;
		GMT->current.setting.proj_length_unit = S.u;
	}

	lux[0] = fabs (GMT->current.proj.z_project.sin_az * GMT->current.proj.z_project.cos_el);
	lux[1] = fabs (GMT->current.proj.z_project.cos_az * GMT->current.proj.z_project.cos_el);
	lux[2] = fabs (GMT->current.proj.z_project.sin_el);
	tmp = MAX (lux[0], MAX (lux[1], lux[2]));
	for (i = 0; i < 3; i++) lux[i] = (lux[i] / tmp) - 0.5;

	if ((Ctrl->C.active || current_fill.rgb[0]) >= 0 && (S.symbol == GMT_SYMBOL_COLUMN || S.symbol == GMT_SYMBOL_CUBE)) {	/* Modify the color for each facet */
		for (i = 0; i < 3; i++) {
			GMT_rgb_copy (rgb[i], current_fill.rgb);
			if (S.shade3D) GMT_illuminate (GMT, lux[i], rgb[i]);
		}
	}

	GMT_plotinit (GMT, options);

	GMT_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
	GMT_plotcanvas (GMT);	/* Fill canvas if requested */

	GMT_map_basemap (GMT);

	if (GMT->current.proj.z_pars[0] == 0.0 && !Ctrl->N.active) {
		GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);
		clip_set = TRUE;
	}
	if (S.symbol == GMT_SYMBOL_ELLIPSE) Ctrl->N.active = TRUE;

	if (S.symbol == GMT_SYMBOL_TEXT && Ctrl->G.active && !Ctrl->W.active) PSL_setcolor (PSL, current_fill.rgb, PSL_IS_FILL);
	if (S.symbol == GMT_SYMBOL_TEXT) GMT_setfont (GMT, &S.font);		/* Set the required font */
	if ((S.symbol == GMT_SYMBOL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR) && S.v.status & GMT_VEC_JUST_S) {
		/* Reading 2nd coordinate so must set column types */
		GMT->current.io.col_type[GMT_IN][pos2x] = GMT->current.io.col_type[GMT_IN][GMT_X];
		GMT->current.io.col_type[GMT_IN][pos2y] = GMT->current.io.col_type[GMT_IN][GMT_Y];
	}
	if (S.symbol == GMT_SYMBOL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR || S.symbol == GMT_SYMBOL_MARC ) {	/* One of the vector symbols */
		if ((S.v.status & GMT_VEC_FILL) == 0) Ctrl->G.active = FALSE;	/* Want to fill so override -G*/
		if (S.v.status & GMT_VEC_FILL2) current_fill = S.v.fill;	/* Override -G<fill> (if set) with specified head fill */
		if (S.v.status & GMT_VEC_OUTLINE2) current_pen = S.v.pen, Ctrl->W.active = TRUE;	/* Override -W (if set) with specified vector pen */
	}
	if (penset_OK) GMT_setpen (GMT, &current_pen);
	fill_active = Ctrl->G.active;	/* Make copies because we will change the values */
	outline_active =  Ctrl->W.active;
	if (not_line && !outline_active && !fill_active && !get_rgb) outline_active = TRUE;	/* If no fill nor outline for symbols then turn outline on */

	if (Ctrl->D.active) {
		/* Shift the plot a bit. This is a bit frustrating, since the only way to do this
		   easily is to undo the perspective, shift, then redo. */
		GMT_plane_perspective (GMT, -1, 0.0);
		GMT_xyz_to_xy (GMT, Ctrl->D.dx, Ctrl->D.dy, Ctrl->D.dz, &DX, &DY);
		PSL_setorigin (PSL, DX, DY, 0.0, PSL_FWD);
		GMT_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
	}
	GMT->current.io.skip_if_NaN[GMT_Z] = TRUE;	/* Extend GMT NaN-handling to the z-coordinate */

	old_is_world = GMT->current.map.is_world;
	geometry = not_line ? GMT_IS_POINT : (GMT_IS_LINE + polygon);
	if ((error = GMT_set_cols (GMT, GMT_IN, n_needed)) != GMT_OK) {
		Return (error);
	}

	if (read_symbol) {	/* If symbol info is given we must process text records */
		set_type = GMT_IS_TEXTSET;
		read_mode = GMT_READ_TEXT;
	}
	else {	/* Here we can process data records (ASCII or binary) */
		set_type = GMT_IS_DATASET;
		read_mode = GMT_READ_DOUBLE;
	}
	in = GMT->current.io.curr_rec;

	if (not_line) {	/* symbol part (not counting GMT_SYMBOL_FRONT and GMT_SYMBOL_QUOTED_LINE) */
		if (GMT_Init_IO (API, set_type, geometry, GMT_IN, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Register data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, set_type, GMT_IN) != GMT_OK) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		GMT->current.map.is_world = !(S.symbol == GMT_SYMBOL_ELLIPSE && S.convert_angles);
		if (!read_symbol) API->object[API->current_item[GMT_IN]]->n_expected_fields = n_needed;
		n = 0;
		do {	/* Keep returning records until we reach EOF */
			if ((record = GMT_Get_Record (API, read_mode, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (GMT_REC_IS_TBL_HEADER (GMT)) {	/* Skip table headers */
					continue;
				}
				if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
					break;
				else if (GMT_REC_IS_SEG_HEADER (GMT)) {			/* Parse segment headers */
					PSL_comment (PSL, "Segment header: %s\n", GMT->current.io.segment_header);
					change = GMT_parse_segment_header (GMT, GMT->current.io.segment_header, P, &fill_active, &current_fill, default_fill, &outline_active, &current_pen, default_pen, default_outline, NULL);
					if (Ctrl->I.active) {
						GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
						GMT_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
					}
					if (read_symbol) API->object[API->current_item[GMT_IN]]->n_expected_fields = GMT_MAX_COLUMNS;
					continue;
				}
			}

			/* Data record to process */

			n_total_read++;

			if (read_symbol) {	/* Must do special processing */
				text_rec = (char *)record;
				/* First establish the symbol type given at the end of the record */
				GMT_chop (text_rec);	/* Get rid of \n \r */
				i = strlen (text_rec) - 1;
				while (text_rec[i] && !strchr (" \t", (int)text_rec[i])) i--;
				GMT_parse_symbol_option (GMT, &text_rec[i+1], &S, 1, FALSE);
				for (j = n_cols_start; j < 7; j++) GMT->current.io.col_type[GMT_IN][j] = GMT_IS_DIMENSION;		/* Since these may have units appended */
				for (j = 0; j < S.n_nondim; j++) GMT->current.io.col_type[GMT_IN][S.nondim_col[j]+get_rgb] = GMT_IS_FLOAT;	/* Since these are angles, not dimensions */
				/* Now convert the leading text items to doubles; col_type[GMT_IN] might have been updated above */
				if (GMT_conv_intext2dbl (GMT, text_rec, 7)) {	/* Max 7 columns needs to be parsed */
					GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad x and/or y coordinates, skipped)\n", n_total_read);
					continue;
				}
				if (S.symbol == GMT_SYMBOL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR || S.symbol == GMT_SYMBOL_MARC) {	/* One of the vector symbols */
					if (S.v.status & GMT_VEC_OUTLINE2) {
						current_pen = S.v.pen, Ctrl->W.active = TRUE;	/* Override -W (if set) with specified pen */
					}
					else if (S.v.status & GMT_VEC_OUTLINE) {
						current_pen = default_pen, Ctrl->W.active = TRUE;	/* Return to default pen */
					}
					if (S.v.status & GMT_VEC_FILL2) {
						current_fill = S.v.fill;	/* Override -G<fill> with specified head fill */
						if (S.v.status & GMT_VEC_FILL) Ctrl->G.active = TRUE;
					}
					else if (S.v.status & GMT_VEC_FILL) {
						current_fill = default_fill, Ctrl->G.active = TRUE;	/* Return to default fill */
					}
				}
			}

			/* Here, all in[*] beyond lon, lat, z will have been converted to inch if they had a trailing unit (e.g., 5c) */

			if (!Ctrl->N.active && (in[GMT_Z] < GMT->common.R.wesn[ZLO] || in[GMT_Z] > GMT->common.R.wesn[ZHI])) continue;
			if (!Ctrl->N.active && S.symbol != GMT_SYMBOL_BARX && S.symbol != GMT_SYMBOL_BARY) {
				/* Skip points outside map */
				GMT_map_outside (GMT, in[GMT_X], in[GMT_Y]);
				if (GMT_abs (GMT->current.map.this_x_status) > 1 || GMT_abs (GMT->current.map.this_y_status) > 1) continue;
			}

			if (get_rgb) {	/* Lookup t to get rgb */
				GMT_get_rgb_from_z (GMT, P, in[3], current_fill.rgb);
				if (Ctrl->I.active) GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
				if (P->skip) continue;	/* Chosen cpt file indicates skip for this t */
			}

			if (n == n_alloc) data = GMT_malloc (GMT, data, n, &n_alloc, struct PSXYZ_DATA);

			if (GMT_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &data[n].x, &data[n].y) || GMT_is_dnan(in[GMT_Z])) continue;	/* NaNs on input */
			data[n].z = GMT_z_to_zz (GMT, in[GMT_Z]);

			if (S.read_size) {	/* Update sizes from input */
				S.size_x = in[ex1];
				S.size_y = in[ex2];
			}
			data[n].dim[0] = S.size_x;
			data[n].dim[1] = S.size_y;

			data[n].flag = S.convert_angles;
			data[n].symbol = S.symbol;
			data[n].f = current_fill;
			data[n].p = current_pen;
			data[n].outline = outline_active;
			data[n].string = NULL;
			/* Next two are for sorting:
			   dist[0] is layer "height": objects closer to the viewer have higher numbers
			   dist[1] is higher when objects are further above a place viewed from above or below a plane viewed from below */
			data[n].dist[0] = GMT->current.proj.z_project.sin_az * data[n].x + GMT->current.proj.z_project.cos_az * data[n].y;
			data[n].dist[1] = GMT->current.proj.z_project.sin_el * data[n].z;

			switch (S.symbol) {
				case GMT_SYMBOL_BARX:
					data[n].dim[2] = (GMT_is_dnan (S.base)) ? 0.0 : GMT_x_to_xx (GMT, S.base);
					break;
				case GMT_SYMBOL_BARY:
					data[n].dim[2] = (GMT_is_dnan (S.base)) ? 0.0 : GMT_y_to_yy (GMT, S.base);
					break;
				case GMT_SYMBOL_COLUMN:
					data[n].dim[2] = (GMT_is_dnan (S.base)) ? 0.0 : GMT_z_to_zz (GMT, S.base);
					break;
				case GMT_SYMBOL_RNDRECT:
					data[n].dim[2] = in[ex3];	/* radius */
				case GMT_SYMBOL_RECT:
					data[n].dim[0] = in[ex1];	/* x-dim */
					data[n].dim[1] = in[ex2];	/* y-dim */
					break;
				case GMT_SYMBOL_ELLIPSE:
				case GMT_SYMBOL_ROTRECT:
					if (!S.convert_angles) {	/* Got axes in current plot units, change to inches */
						data[n].dim[0] = in[ex1];	/* direction */
						data[n].dim[1] = in[ex2];
						data[n].dim[2] = in[ex3];
					}
					else if (!GMT_is_geographic (GMT, GMT_IN)) {	/* Got axes in user units, change to inches */
						data[n].dim[0] = 90.0 - in[ex1];	/* Cartesian azimuth */
						data[n].dim[1] = in[ex2] * GMT->current.proj.scale[GMT_X];
						data[n].dim[2] = in[ex3] * GMT->current.proj.scale[GMT_X];
					}
					else {				/* Got axis in km */
						data[n].dim[0] = in[ex1];	/* Azimuth will be forwarded to GMT_geo_rectangle/ellipse */
						data[n].dim[1] = in[ex2];
						data[n].dim[2] = in[ex3];
						data[n].x = in[GMT_X];	/* Revert to longitude and latitude */
						data[n].y = in[GMT_Y];
						data[n].flag |= 2;	/* Signals to use GMT_geo_* routine */
					}
					break;
				case GMT_SYMBOL_TEXT:
					data[n].string = strdup (S.string);
					break;
				case GMT_SYMBOL_VECTOR:
					GMT_init_vector_param (GMT, &S);	/* Update vector head parameters */
					S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					if (!S.convert_angles)	/* Use direction as given */
						data[n].dim[0] = in[ex1+S.read_size];	/* direction */
					else if (!GMT_is_geographic (GMT, GMT_IN))	/* Cartesian azimuth; change to direction */
						data[n].dim[0] = 90.0 - in[ex1+S.read_size];
					else	/* Convert geo azimuth to map direction */
						data[n].dim[0] = GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, in[ex1+S.read_size]);
					data[n].dim[1] = in[ex2+S.read_size];	/* length */
					if (S.v.status & GMT_VEC_JUST_S) {	/* Got coordinates of tip instead of dir/length */
						GMT_geo_to_xy (GMT, in[pos2x], in[pos2y], &x_2, &y_2);
						if (GMT_is_dnan (x_2) || GMT_is_dnan (y_2)) {
							GMT_report (GMT, GMT_MSG_FATAL, "Warning: Vector head coordinates contain NaNs near line %ld. Skipped\n", n_total_read);
							continue;
						}
						data[n].dim[1] = hypot (data[n].x - x_2, data[n].y - y_2);	/* Compute vector length in case of shrinking */
					}
					else {
						sincosd (data[n].dim[0], &s, &c);
						x_2 = data[n].x + data[n].dim[1] * c;
						y_2 = data[n].y + data[n].dim[1] * s;
						justify = GMT_vec_justify (S.v.status);	/* Return justification as 0-2 */
						if (justify) {
							dx = justify * 0.5 * (x_2 - data[n].x);	dy = justify * 0.5 * (y_2 - data[n].y);
							data[n].x -= dx;	data[n].y -= dy;
							x_2 -= dx;		y_2 -= dy;
						}
					}
					data[n].dim[0] = x_2, data[n].dim[1] = y_2;
					s = (data[n].dim[1] < S.v.v_norm) ? data[n].dim[1] / S.v.v_norm : 1.0;
					data[n].dim[2] = s * S.v.v_width;
					data[n].dim[3] = s * S.v.h_length;
					data[n].dim[4] = s * S.v.h_width;
					data[n].dim[5] = GMT->current.setting.map_vector_shape;
					data[n].dim[6] = (double)S.v.status;
					break;
				case GMT_SYMBOL_GEOVECTOR:
					GMT_init_vector_param (GMT, &S);	/* Update vector head parameters */
					S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					data[n].dim[0] = in[ex2+S.read_size];	/* length */
					data[n].dim[1] = in[ex1+S.read_size];	/* direction */
					data[n].x = in[GMT_X];			/* Revert to longitude and latitude */
					data[n].y = in[GMT_Y];
					data[n].v = S.v;
					break;
				case GMT_SYMBOL_MARC:
					GMT_init_vector_param (GMT, &S);	/* Update vector head parameters */
					S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					data[n].dim[0] = in[ex1+S.read_size];	/* Radius */
					data[n].dim[1] = in[ex2+S.read_size];	/* Start direction in degrees */
					data[n].dim[2] = in[ex3+S.read_size];	/* Stop direction in degrees */
					length = fabs (data[n].dim[2]-data[n].dim[1]);	/* Arc length in degrees */
					s = (length < S.v.v_norm) ? length / S.v.v_norm : 1.0;
					data[n].dim[3] = s * S.v.h_length;	/* Length of (shrunk) vector head */
					data[n].dim[4] = s * S.v.h_width;	/* Width of (shrunk) vector head */
					data[n].dim[5] = s * S.v.v_width;	/* Thickness of (shrunk) vector */
					data[n].dim[6] = GMT->current.setting.map_vector_shape;
					data[n].dim[7] = (double)S.v.status;	/* Vector tributes */
					break;
				case GMT_SYMBOL_WEDGE:
					if (!S.convert_angles) {
						data[n].dim[1] = in[ex1+S.read_size];			/* Start direction in degrees */
						data[n].dim[2] = in[ex2+S.read_size];			/* Stop direction in degrees */
					}
					else if (!GMT_is_geographic (GMT, GMT_IN)) {	/* Got azimuths instead */
						data[n].dim[1] = 90.0 - in[ex1+S.read_size];		/* Start direction in degrees */
						data[n].dim[2] = 90.0 - in[ex2+S.read_size];		/* Stop direction in degrees */
					}
					else {
						data[n].dim[1] = GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, in[ex1+S.read_size]);
						data[n].dim[2] = GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, in[ex2+S.read_size]);
					}
					break;
				case GMT_SYMBOL_CUSTOM:
					data[n].custom = GMT_memory (GMT, NULL, 1, struct GMT_CUSTOM_SYMBOL);
					GMT_memcpy (data[n].custom, S.custom, 1, struct GMT_CUSTOM_SYMBOL);
					break;
			}
			if (S.user_unit) data[n].flag |= 4;

			if (Ctrl->W.mode) {
				GMT_rgb_copy (Ctrl->W.pen.rgb, current_fill.rgb);
				current_pen = Ctrl->W.pen;
			}
			if (Ctrl->W.mode & 1) GMT_rgb_copy (current_fill.rgb, GMT->session.no_rgb);
			n++;
			if (read_symbol) API->object[API->current_item[GMT_IN]]->n_expected_fields = GMT_MAX_COLUMNS;
		} while (TRUE);
		
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}

		data = GMT_malloc (GMT, data, 0, &n, struct PSXYZ_DATA);

		/* Sort according to distance from viewer */

		if (!Ctrl->Q.active) qsort (data, n, sizeof (struct PSXYZ_DATA), dist_compare);

		/* Now plot these symbols one at the time */

		for (i = 0; i < n; i++) {

			if (data[i].symbol == GMT_SYMBOL_COLUMN || data[i].symbol == GMT_SYMBOL_CUBE) {
				for (j = 0; j < 3; j++) {
					GMT_rgb_copy (rgb[j], data[i].f.rgb);
					if (S.shade3D) GMT_illuminate (GMT, lux[j], rgb[j]);
				}
			}

			GMT_setfill (GMT, &data[i].f, data[i].outline);
			GMT_setpen (GMT, &data[i].p);

			switch (data[i].symbol) {
				case GMT_SYMBOL_NONE:
					break;
				case GMT_SYMBOL_BARX:
					if (!Ctrl->N.active) in[GMT_X] = MAX (GMT->common.R.wesn[XLO], MIN (data[i].x, GMT->common.R.wesn[XHI]));
					if (data[i].flag & 4) {
						GMT_geo_to_xy (GMT, data[i].x, data[i].y - 0.5 * data[i].dim[0], &x_1, &y_1);
						GMT_geo_to_xy (GMT, data[i].x, data[i].y + 0.5 * data[i].dim[0], &x_2, &y_2);
						data[i].dim[0] = 0.5 * hypot (x_1 - x_2, y_1 - y_2);
					}
					GMT_plane_perspective (GMT, GMT_Z, data[i].z);
					PSL_plotbox (PSL, data[i].x, data[i].y - 0.5 * data[i].dim[0], data[i].dim[2], data[i].y + 0.5 * data[i].dim[0]);
					break;
				case GMT_SYMBOL_BARY:
					if (!Ctrl->N.active) in[GMT_Y] = MAX (GMT->common.R.wesn[YLO], MIN (data[i].y, GMT->common.R.wesn[YHI]));
					if (data[i].flag & 4) {
						GMT_geo_to_xy (GMT, data[i].x - 0.5 * data[i].dim[0], data[i].y, &x_1, &y_1);
						GMT_geo_to_xy (GMT, data[i].x + 0.5 * data[i].dim[0], data[i].y, &x_2, &y_2);
						data[i].dim[0] = 0.5 * hypot (x_1 - x_2, y_1 - y_2);
					}
					GMT_plane_perspective (GMT, GMT_Z, data[i].z);
					PSL_plotbox (PSL, data[i].x - 0.5 * data[i].dim[0], data[i].y, data[i].x + 0.5 * data[i].dim[0], data[i].dim[2]);
					break;
				case GMT_SYMBOL_COLUMN:
					dim[2] = fabs (data[i].z - data[i].dim[2]);
					if (data[i].flag & 4) {
						GMT_geo_to_xy (GMT, data[i].x - data[i].dim[0], data[i].y, &x_1, &y_1);
						GMT_geo_to_xy (GMT, data[i].x + data[i].dim[0], data[i].y, &x_2, &y_2);
						dim[0] = 0.5 * hypot (x_1 - x_2, y_1 - y_2);
						GMT_geo_to_xy (GMT, data[i].x, data[i].y - data[i].dim[1], &x_1, &y_1);
						GMT_geo_to_xy (GMT, data[i].x, data[i].y + data[i].dim[1], &x_2, &y_2);
						dim[1] = 0.5 * hypot (x_1 - x_2, y_1 - y_2);
					}
					else
						dim[0] = data[i].dim[0], dim[1] = data[i].dim[1];
					column3D (GMT, PSL, data[i].x, data[i].y, (data[i].z + data[i].dim[2]) / 2.0, dim, rgb, data[i].outline);
					break;
				case GMT_SYMBOL_CUBE:
					if (data[i].flag & 4) {
						GMT_geo_to_xy (GMT, data[i].x - data[i].dim[0], data[i].y, &x_1, &y_1);
						GMT_geo_to_xy (GMT, data[i].x + data[i].dim[0], data[i].y, &x_2, &y_2);
						dim[0] = 0.5 * hypot (x_1 - x_2, y_1 - y_2);
					}
					else
						dim[0] = data[i].dim[0];
					dim[1] = dim[2] = dim[0];
					column3D (GMT, PSL, data[i].x, data[i].y, data[i].z, dim, rgb, data[i].outline);
					break;
				case GMT_SYMBOL_CROSS:
				case GMT_SYMBOL_PLUS:
				case GMT_SYMBOL_DOT:
				case GMT_SYMBOL_XDASH:
				case GMT_SYMBOL_YDASH:
				case GMT_SYMBOL_STAR:
				case GMT_SYMBOL_CIRCLE:
				case GMT_SYMBOL_SQUARE:
				case GMT_SYMBOL_HEXAGON:
				case GMT_SYMBOL_PENTAGON:
				case GMT_SYMBOL_OCTAGON:
				case GMT_SYMBOL_TRIANGLE:
				case GMT_SYMBOL_INVTRIANGLE:
				case GMT_SYMBOL_DIAMOND:
				case GMT_SYMBOL_RECT:
				case GMT_SYMBOL_RNDRECT:
					GMT_plane_perspective (GMT, GMT_Z, data[i].z);
					PSL_plotsymbol (PSL, data[i].x, data[i].y, data[i].dim, data[i].symbol);
					break;
				case GMT_SYMBOL_ELLIPSE:
					GMT_plane_perspective (GMT, GMT_Z, data[i].z);
					if (data[i].flag & 2)
						GMT_geo_ellipse (GMT, data[i].x, data[i].y, data[i].dim[1], data[i].dim[2], data[i].dim[0]);
					else
						PSL_plotsymbol (PSL, data[i].x, data[i].y, data[i].dim, PSL_ELLIPSE);
					break;
				case GMT_SYMBOL_ROTRECT:
					GMT_plane_perspective (GMT, GMT_Z, data[i].z);
					if (data[i].flag & 2)
						GMT_geo_rectangle (GMT, data[i].x, data[i].y, data[i].dim[1], data[i].dim[2], data[i].dim[0]);
					else
						PSL_plotsymbol (PSL, data[i].x, data[i].y, data[i].dim, PSL_ROTRECT);
					break;
				case GMT_SYMBOL_TEXT:
					if (fill_active && !data[i].outline)
						PSL_setcolor (PSL, data[i].f.rgb, PSL_IS_FILL);
					else if (!fill_active)
						PSL_setfill (PSL, GMT->session.no_rgb, data[i].outline);
					(void) GMT_setfont (GMT, &S.font);
					GMT_plane_perspective (GMT, GMT_Z, data[i].z);
					PSL_plottext (PSL, data[i].x, data[i].y, data[i].dim[0] * PSL_POINTS_PER_INCH, data[i].string, 0.0, PSL_MC, data[i].outline);
					free ((void*)data[i].string);
					break;
				case GMT_SYMBOL_VECTOR:
					GMT_plane_perspective (GMT, GMT_Z, data[i].z);
					PSL_plotsymbol (PSL, data[i].x, data[i].y, data[i].dim, PSL_VECTOR);
					break;
				case GMT_SYMBOL_GEOVECTOR:
					GMT_plane_perspective (GMT, GMT_Z, data[i].z);
					S.v = data[i].v;	/* Update vector attributes from saved values */
					GMT_geo_vector (GMT, data[i].x, data[i].y, data[i].dim[0], data[i].dim[1], &S);
					break;
				case GMT_SYMBOL_MARC:
					GMT_plane_perspective (GMT, GMT_Z, data[i].z);
					PSL_plotsymbol (PSL, data[i].x, data[i].y, data[i].dim, PSL_MARC);
					break;
				case GMT_SYMBOL_WEDGE:
					data[i].dim[0] *= 0.5;
					GMT_plane_perspective (GMT, GMT_Z, data[i].z);
					PSL_plotsymbol (PSL, data[i].x, data[i].y, data[i].dim, PSL_WEDGE);
					break;
				case GMT_SYMBOL_ZDASH:
					GMT_xyz_to_xy (GMT, data[i].x, data[i].y, data[i].z, &x_1, &y_1);
					GMT_plane_perspective (GMT, -1, 0.0);
					PSL_plotsymbol (PSL, x_1, y_1, data[i].dim, PSL_YDASH);
					break;
				case GMT_SYMBOL_CUSTOM:
					GMT_plane_perspective (GMT, GMT_Z, data[i].z);
					dim[0] = data[i].dim[0];
					for (j = 0; S.custom->type && j < S.n_required; j++) {	/* Deal with any geo-angles first */
						dim[j+1] = (S.custom->type[j] == GMT_IS_GEOANGLE) ? GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, 90.0 - data[i].dim[j]) : data[i].dim[j];
					}
					GMT_draw_custom_symbol (GMT, data[i].x, data[i].y, dim, data[i].custom, &data[i].p, &data[i].f, data[i].outline);
					GMT_free (GMT, data[i].custom);
					break;
			}
		}
		GMT_free (GMT, data);
	}
	else {	/* Line/polygon part */
		struct GMT_DATASET *D = NULL;	/* Pointer to GMT segment table(s) */

		if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_IN, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Establishes data input */
			Return (API->error);
		}
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, NULL)) == NULL) {
			Return (API->error);
		}

		for (tbl = 0; tbl < D->n_tables; tbl++) {
			if (D->table[tbl]->n_headers && S.G.label_type == 2) GMT_extract_label (GMT, &D->table[tbl]->header[0][1], S.G.label);	/* Set first header as potential label */

			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */

				L = D->table[tbl]->segment[seg];	/* Set shortcut to current segment */
				if (polygon && GMT_polygon_is_hole (L)) continue;	/* Holes are handled together with perimeters */

				n = L->n_rows;				/* Number of points in this segment */

				/* We had here things like:	x = D->table[tbl]->segment[seg]->coord[GMT_X];
				 * but reallocating x below lead to disasters.  */

				change = GMT_parse_segment_header (GMT, L->header, P, &fill_active, &current_fill, default_fill, &outline_active, &current_pen, default_pen, default_outline, L->ogr);

				if (P && P->skip) continue;	/* Chosen cpt file indicates skip for this z */

				if (L->header && L->header[0]) {
					char s_args[GMT_BUFSIZ];
					PSL_comment (PSL, "Segment header: %s\n", L->header);
					if (GMT_parse_segment_item (GMT, L->header, "-S", s_args)) {	/* Found -S */
						if ((S.symbol == GMT_SYMBOL_QUOTED_LINE && s_args[0] == 'q') || (S.symbol == GMT_SYMBOL_FRONT && s_args[0] == 'f')) { /* Update parameters */
							GMT_parse_symbol_option (GMT, s_args, &S, 0, FALSE);
							if (change & 1) change -= 1;	/* Don't want polygon to be TRUE later for these symbols */
						}
						else
							GMT_report (GMT, GMT_MSG_FATAL, "Segment header tries to switch from -S%c to another symbol (%s) - ignored\n", S.symbol, s_args);
					}
				}

				if (Ctrl->I.active) {
					GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
					GMT_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
				}
				if (change & 4 && penset_OK) GMT_setpen (GMT, &current_pen);
				if (change & 1) polygon = TRUE;
				if (change & 2 && !Ctrl->L.active) {
					polygon = FALSE;
					PSL_setcolor (PSL, current_fill.rgb, PSL_IS_STROKE);
				}
				if (S.G.label_type == 2)	/* Get potential label from segment header */
					GMT_extract_label (GMT, L->header, S.G.label);

#if 0
				/* This should never happen, GMT_Read_Data should already take care of this */

				if (polygon && GMT_polygon_is_open (GMT, L->coord[GMT_X], L->coord[GMT_Y], L->n_rows)) {
					/* Explicitly close polygon so that arc will work */
					L->n_rows = GMT_malloc3 (GMT, L->coord[GMT_X], L->coord[GMT_Y], L->coord[GMT_Z], 0, L->n_rows+1, double);
					L->coord[GMT_X][L->n_rows-1] = L->coord[GMT_X][0];
					L->coord[GMT_Y][L->n_rows-1] = L->coord[GMT_Y][0];
					L->coord[GMT_Z][L->n_rows-1] = L->coord[GMT_Z][0];
				}

				n = L->n_rows;
#endif

				xp = GMT_memory (GMT, NULL, n, double);
				yp = GMT_memory (GMT, NULL, n, double);

				if (polygon) {
					GMT_plane_perspective (GMT, -1, 0.0);
					for (i = 0; i < n; i++) GMT_geoz_to_xy (GMT, L->coord[GMT_X][i], L->coord[GMT_Y][i], L->coord[GMT_Z][i], &xp[i], &yp[i]);
					GMT_setfill (GMT, &current_fill, outline_active);
					PSL_plotpolygon (PSL, xp, yp, n);
				}
				else if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {	/* Labeled lines are dealt with by the contour machinery */
					/* Note that this always be plotted in the XY-plane */
					GMT_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
					if ((GMT->current.plot.n = GMT_geo_to_xy_line (GMT, L->coord[GMT_X], L->coord[GMT_Y], L->n_rows)) == 0) continue;
					S.G.line_pen = current_pen;
					GMT_hold_contour (GMT, &GMT->current.plot.x, &GMT->current.plot.y, GMT->current.plot.n, 0.0, "N/A", 'A', S.G.label_angle, Ctrl->L.active, &S.G);
					GMT->current.plot.n_alloc = GMT->current.plot.n;	/* Since GMT_hold_contour reallocates to fit the array */
				}
				else {	/* Plot line */
					GMT_plane_perspective (GMT, -1, 0.0);
					for (i = 0; i < n; i++) GMT_geoz_to_xy (GMT, L->coord[GMT_X][i], L->coord[GMT_Y][i], L->coord[GMT_Z][i], &xp[i], &yp[i]);
					PSL_plotline (PSL, xp, yp, n, PSL_MOVE + PSL_STROKE);
				}
				if (S.symbol == GMT_SYMBOL_FRONT) { /* Must draw fault crossbars */
					GMT_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
					if ((GMT->current.plot.n = GMT_geo_to_xy_line (GMT, L->coord[GMT_X], L->coord[GMT_Y], L->n_rows)) == 0) continue;
					GMT_setfill (GMT, &current_fill, outline_active);
					GMT_draw_front (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, &S.f);
				}

				GMT_free (GMT, xp);
				GMT_free (GMT, yp);
			}
		}
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &D) != GMT_OK) {
			Return (API->error);
		}
	}

	if (S.u_set) GMT->current.setting.proj_length_unit = save_u;	/* Reset unit */

	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		if (S.G.save_labels) {	/* Want to save the Line label locations (lon, lat, angle, label) */
			if ((error = GMT_contlabel_save (GMT, &S.G))) Return (error);
		}
		GMT_contlabel_plot (GMT, &S.G);
		GMT_contlabel_free (GMT, &S.G);
	}

	if (clip_set) GMT_map_clip_off (GMT);

	GMT_plane_perspective (GMT, -1, 0.0);

	if (Ctrl->D.active) PSL_setorigin (PSL, -DX, -DY, 0.0, PSL_FWD);	/* Shift plot a bit */

	if (current_pen.style) PSL_setdash (PSL, CNULL, 0);
	GMT_vertical_axis (GMT, 2);	/* Draw foreground axis */
	GMT->current.map.is_world = old_is_world;

	GMT_plotend (GMT);

	Return (GMT_OK);
}
