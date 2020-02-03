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
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: psxyz will read <x,y,z> triplets and plot symbols, lines,
 * or polygons in a 3-D perspective view.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psxyz"
#define THIS_MODULE_MODERN_NAME	"plot3d"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot lines, polygons, and symbols in 3-D"
#define THIS_MODULE_KEYS	"<D{,CC(,T-<,>X},S?(=2"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYabdefghipqtxy" GMT_OPT("EHMmc")

/* Control structure for psxyz */

struct PSXYZ_CTRL {
	struct A {	/* -A[step] {NOT IMPLEMENTED YET} */
		bool active;
		double step;
	} A;
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
	struct I {	/* -I[<intensity>] */
		bool active;
		unsigned int mode;	/* 0 if constant, 1 if read from file (symbols only) */
		double value;
	} I;
	struct L {	/* -L[+xl|r|x0][+yb|t|y0][+e|E][+p<pen>] */
		bool active;
		bool polygon;		/* true when just -L is given */
		int outline;		/* 1 when +p<pen> is given */
		unsigned int mode;	/* Which side for the anchor */
		unsigned int anchor;	/* 0 not used, 1 = x anchors, 2 = y anchors, 3 = +/-dy, 4 = -dy1, +dy2 */
		double value;
		struct GMT_PEN pen;
	} L;
	struct N {	/* -N[r|c] */
		bool active;
		unsigned int mode;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
	struct S {	/* -S */
		bool active;
		char *arg;
	} S;
	struct T {	/* -T */
		bool active;
	} T;
	struct W {	/* -W<pen>[+c[l|f]][+o<offset>][+s][+v[b|e]<size><vecargs>] */
		bool active;
		bool cpt_effect;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z[l|f]<value> */
		bool active;
		unsigned int mode;	/* 1 for line, 2 for fill, 3 for both */
		double value;
	} Z;
};

enum Psxyz_poltype {
	PSXY_POL_X 		= 1,
	PSXY_POL_Y,
	PSXY_POL_SYMM_DEV,
	PSXY_POL_ASYMM_DEV,
	PSXY_POL_ASYMM_ENV};

enum Psxyz_cliptype {
	PSXYZ_CLIP_REPEAT 	= 0,
	PSXYZ_CLIP_NO_REPEAT,
	PSXYZ_NO_CLIP_REPEAT,
	PSXYZ_NO_CLIP_NO_REPEAT};

struct PSXYZ_DATA {
	int symbol, outline;
	unsigned int flag;	/* 1 = convert azimuth, 2 = use geo-functions, 4 = x-base in units, 8 y-base in units */
	double x, y, z, dim[PSL_MAX_DIMS], dist[2], transparency;
	double *zz;	/* For column symbol if +z<n> in effect */
	struct GMT_FILL f;
	struct GMT_PEN p, h;
	struct GMT_VECT_ATTR v;
	char *string;
	struct GMT_CUSTOM_SYMBOL *custom;
};

EXTERN_MSC double gmt_half_map_width (struct GMT_CTRL *GMT, double y);

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSXYZ_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSXYZ_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->W.pen = GMT->current.setting.map_default_pen;
	gmt_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* Default is no fill */
	C->A.step = GMT->current.setting.map_line_step;
	C->N.mode = PSXYZ_CLIP_REPEAT;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSXYZ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->S.arg);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psxyz synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s %s [%s]\n", name, GMT_J_OPT, GMT_Rgeoz_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-C<cpt>] [-D<dx>/<dy>[/<dz>]] [-G<fill>] [-I[<intens>]] %s\n\t[-L[+b|d|D][+xl|r|x0][+yb|t|y0][+p<pen>]] [-N[c|r]] %s\n", GMT_Jz_OPT, API->K_OPT, API->O_OPT);
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC)	/* -T has no purpose in modern mode */
		GMT_Message (API, GMT_TIME_NONE, "\t%s[-Q] [-S[<symbol>][<size>][/size_y]] [-T]\n\t[%s] [%s] [-W[<pen>][<attr>]]\n", API->P_OPT, GMT_U_OPT, GMT_V_OPT);
	else
		GMT_Message (API, GMT_TIME_NONE, "\t%s[-Q] [-S[<symbol>][<size>][/size_y]]\n\t[%s] [%s] [-W[<pen>][<attr>]]\n", API->P_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-Z[l|f]<val>] [%s]\n\t[%s] %s[%s] [%s] [%s]\n\t[%s]\n", GMT_X_OPT, GMT_Y_OPT, GMT_a_OPT, GMT_bi_OPT, API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_tv_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "J-Z,R3");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<,B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Use CPT (or specify -Ccolor1,color2[,color3,...]) to assign symbol\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   colors based on t-value in 4rd column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: requires -S.  Without -S, psxyz excepts lines/polygons\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and looks for -Z<val> options in each segment header.  Then, color is\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   applied for polygon fill (-L) or polygon pen (no -L).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Offset symbol or line positions by <dx>/<dy>[/<dz>] [no offset].\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify color or pattern [Default is no fill].");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -G is specified but not -S, then psxyz draws a filled polygon.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Use the intensity to modulate the fill color (requires -C or -G).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no intensity is given we expect it to follow symbol size in the data record.\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Force closed polygons.  Alternatively, append modifiers to build constant-z polygon from a line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +d to build symmetrical envelope around y(x) using deviations dy(x) from col 4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +D to build asymmetrical envelope around y(x) using deviations dy1(x) and dy2(x) from cols 4-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +b to build asymmetrical envelope around y(x) using bounds yl(x) and yh(x) from cols 4-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +xl|r|x0 to connect 1st and last point to anchor points at xmin, xmax, or x0, or\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   append +yb|t|y0 to connect 1st and last point to anchor points at ymin, ymax, or y0.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Polygon may be painted (-G) and optionally outlined via +p<pen> [no outline].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do not skip or clip symbols that fall outside the map border [clipping is on]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Nr to turn off clipping and plot repeating symbols for periodic maps.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Nc to retain clipping but turn off plotting of repeating symbols for periodic maps.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default will clip or skip symbols that fall outside and plot repeating symbols].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Do NOT sort symbols based on distance to viewer before plotting.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select symbol type and symbol size (in %s).  Choose between\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   -(xdash), +(plus), st(a)r, (b|B)ar, (c)ircle, (d)iamond, (e)llipse,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (f)ront, octa(g)on, (h)exagon (i)nvtriangle, (j)rotated rectangle,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (k)ustom, (l)etter, (m)athangle, pe(n)tagon, c(o)lumn, (p)oint,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (q)uoted line, (r)ectangle, (R)ounded rectangle, (s)quare, (t)riangle,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c(u)be, (v)ector, (w)edge, (x)cross, (y)dash, (z)dash, or\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   =(geovector, i.e., great or small circle vectors).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no size is specified, then the 4th column must have sizes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no symbol is specified, then the last column must have symbol codes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Note: if -C is selected then 4th means 5th column, etc.]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   column and cube are true 3-D objects (give size as xsize/ysize);\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   all other symbols are shown in 2-D perspective only.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   By default, the 3-D symbols column and cube are shaded;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   use upper case O and U to disable this 3-D illumination.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Symbols A, C, D, F, H, I, N, S, T are adjusted to have same area\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of a circle of given diameter.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Bar: Append +b[<base>] to give the y- (or z-) value of the base [Default = 0 (1 for log-scales)]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Use +B instead if heights are measured relative to base [relative to origin].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Use -SB for horizontal bars; then <base> value refers to the x location.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      To read the <base> value from file, specify +b with no trailing argument.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   3-D Column: Append +b[<base>] to give the z-value of the base of the column\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      [Default = 0 (1 for log-scales)]. Use +B if heights are measured relative to base [relative to origin].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      To read the <base> value from file, specify +b with no trailing argument.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      For multi-band columns append +z<nbands>; then <nbands> z-values will\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      be read from file instead of just 1.  Use +Z if dz increments are given instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Multiband columns requires -C with one color per band (0, 1, ...).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Ellipses: Direction, major, and minor axis must be in columns 4-6.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SE rather than -Se is selected, psxy will expect azimuth, and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     axes [in km], and convert azimuths based on map projection.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use -SE- for a degenerate ellipse (circle) with only diameter in km given.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     in column 4, or append a fixed diameter in km to -SE- instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append any of the units in %s to the axes [Default is k].\n", GMT_LEN_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t     For a linear projection we scale the axes by the map scale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Rotatable Rectangle: Direction, x- and y-dimensions in columns 4-6.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SJ rather than -Sj is selected, psxy will expect azimuth, and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     dimensions [in km] and convert azimuths based on map projection.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use -SJ- for a degenerate rectangle (square w/no rotation) with only one dimension given\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     in column 4, or append a fixed dimension to -SJ- instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append any of the units in %s to the dimensions [Default is k].\n", GMT_LEN_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t     For a linear projection we scale dimensions by the map scale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Fronts: Give <tickgap>[/<ticklen>][+l|r][+<type>][+o<offset>][+p[<pen>]].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If <tickgap> is negative it means the number of gaps instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     The <ticklen> defaults to 15%% of <tickgap> if not given.  Append\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +l or +r   : Plot symbol to left or right of front [centered]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +<type>    : +b(ox), +c(ircle), +f(ault), +s|S(lip), +t(riangle) [f]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     	      +s optionally accepts the arrow angle [20].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       box      : square when centered, half-square otherwise.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       circle   : full when centered, half-circle otherwise.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       fault    : centered cross-tick or tick only in specified direction.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       slip     : left-or right-lateral strike-slip arrows.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       Slip     : Same but with curved arrow-heads.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       triangle : diagonal square when centered, directed triangle otherwise.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +o<offset> : Plot first symbol when along-front distance is offset [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +p[<pen>]  : Alternate pen for symbol outline; if no <pen> then no outline [Outline with -W pen].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Kustom: Append <symbolname> immediately after 'k'; this will look for\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     <symbolname>.def in the current directory, in $GMT_USERDIR,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     or in $GMT_SHAREDIR (searched in that order).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use upper case 'K' if your custom symbol refers a variable symbol, ?.\n");
	gmt_list_custom_symbols (API->GMT);
	GMT_Message (API, GMT_TIME_NONE, "\t   Letter: append +t<string> after symbol size, and optionally +f<font> and +j<justify>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Mathangle: start/stop directions of math angle must be in columns 4-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SM rather than -Sm is used, we draw straight angle symbol if 90 degrees.\n");
	gmt_vector_syntax (API->GMT, 0);
	GMT_Message (API, GMT_TIME_NONE, "\t   Quoted line (z must be constant): Give [d|f|n|l|x]<info>[:<labelinfo>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     <code><info> controls placement of labels along lines.  Select\n");
	gmt_cont_syntax (API->GMT, 7, 1);
	GMT_Message (API, GMT_TIME_NONE, "\t     <labelinfo> controls the label attributes.  Choose from\n");
	gmt_label_syntax (API->GMT, 7, 1);
	GMT_Message (API, GMT_TIME_NONE, "\t   Rectangles: x- and y-dimensions must be in columns 4-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Rounded rectangles: x- and y-dimensions and corner radius must be in columns 3-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Vectors: Direction and length must be in columns 4-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SV rather than -Sv is use, psxy will expect azimuth and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     length and convert azimuths based on the chosen map projection.\n");
	gmt_vector_syntax (API->GMT, 19);
	GMT_Message (API, GMT_TIME_NONE, "\t   Wedges: Start and stop directions of wedge must be in columns 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SW rather than -Sw is selected, specify two azimuths instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -SW: Specify <size><unit> with units either from %s or %s [Default is k].\n", GMT_LEN_UNITS_DISPLAY, GMT_DIM_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t     -Sw: Specify <size><unit> with units from %s [Default is %s].\n", GMT_DIM_UNITS_DISPLAY,
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +a[<dr>] to just draw arc(s) or +r[<da>] to just draw radial lines [wedge].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Geovectors: Azimuth and length must be in columns 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append any of the units in %s to length [k].\n", GMT_LEN_UNITS_DISPLAY);
	gmt_vector_syntax (API->GMT, 3);
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC)	/* -T has no purpose in modern mode */
		GMT_Message (API, GMT_TIME_NONE, "\t-T Ignore all input files.\n");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Set pen attributes [Default pen is %s]:", 0);
	GMT_Message (API, GMT_TIME_NONE, "\t   Implicitly draws symbol outline with this pen.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +c Controls how pens and fills are affected if a CPT is specified via -C:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append l to let pen colors follow the CPT setting (requires -C).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append f to let fill/font colors follow the CPT setting.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Default is both effects.\n");
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Use <value> with -C <cpt> to determine <color> instead of via -G<color> or -W<pen>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append l for just pen color or f for fill color [Default sets both].\n");
	GMT_Option (API, "a,bi");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   Default is the required number of columns.\n");
	GMT_Option (API, "c,di,e,f,g,h,i,p,qi,t");
	GMT_Message (API, GMT_TIME_NONE, "\t   For plotting symbols with variable transparency read from file, give no value.\n");
	GMT_Option (API, ":,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int parse_old_W (struct GMTAPI_CTRL *API, struct PSXYZ_CTRL *Ctrl, char *text) {
	unsigned int j = 0, n_errors = 0;
	if (text[j] == '-') {Ctrl->W.pen.cptmode = 1; j++;}
	if (text[j] == '+') {Ctrl->W.pen.cptmode = 3; j++;}
	if (text[j] && gmt_getpen (API->GMT, &text[j], &Ctrl->W.pen)) {
		gmt_pen_syntax (API->GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", 3);
		GMT_Report (API, GMT_MSG_ERROR, "\t   Append +cl to apply cpt color (-C) to the pen only.\n");
		GMT_Report (API, GMT_MSG_ERROR, "\t   Append +cf to apply cpt color (-C) to symbol fill.\n");
		GMT_Report (API, GMT_MSG_ERROR, "\t   Append +c for both effects [none].\n");
		n_errors++;
	}
	return n_errors;
}

GMT_LOCAL unsigned int get_column_bands (struct GMT_SYMBOL *S) {
	/* Report how many bands in the 3-D column */
	unsigned int n_z = S->n_required;	/* z normallly not counted unless +z|Z was used, so this could be 0 */
	if (S->base_set & 2 && n_z) n_z--;	/* Remove the base column item */
	if (n_z == 0) n_z = 1;	/* 1 means singleband column */
	return (n_z);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSXYZ_CTRL *Ctrl, struct GMT_OPTION *options, struct GMT_SYMBOL *S) {
	/* This parses the options provided to psxyz and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, ztype, n_files = 0;
	int n, j;
	bool three_D = false;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	if ((opt = GMT_Find_Option (GMT->parent, 'p', options))) three_D = true;	/* Gave -p */

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Vary symbol color with z */
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':
				if ((n = sscanf (opt->arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c)) < 2) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -D: Give x and y [and z] offsets\n");
					n_errors++;
				}
				else {
					Ctrl->D.dx = gmt_M_to_inch (GMT, txt_a);
					Ctrl->D.dy = gmt_M_to_inch (GMT, txt_b);
					if (n == 3) Ctrl->D.dz = gmt_M_to_inch (GMT, txt_c);
					Ctrl->D.active = true;
				}
				break;
			case 'G':		/* Set color for symbol or polygon */
				Ctrl->G.active = true;
				if (!opt->arg[0] || gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'I':	/* Adjust symbol color via intensity */
				Ctrl->I.active = true;
				if (opt->arg[0])
					Ctrl->I.value = atof (opt->arg);
				else
					Ctrl->I.mode = 1;
				break;
			case 'L':		/* Force closed polygons */
				Ctrl->L.active = true;
				if ((c = strstr (opt->arg, "+b")) != NULL)	/* Build asymmetric polygon from lower and upper bounds */
					Ctrl->L.anchor = PSXY_POL_ASYMM_ENV;
				else if ((c = strstr (opt->arg, "+d")) != NULL)	/* Build symmetric polygon from deviations about y(x) */
					Ctrl->L.anchor = PSXY_POL_SYMM_DEV;
				else if ((c = strstr (opt->arg, "+D")) != NULL)	/* Build asymmetric polygon from deviations about y(x) */
					Ctrl->L.anchor = PSXY_POL_ASYMM_DEV;
				else if ((c = strstr (opt->arg, "+x")) != NULL) {	/* Parse x anchors for a polygon */
					switch (c[2]) {
						case 'l':	Ctrl->L.mode = XLO;	break;	/* Left side anchors */
						case 'r':	Ctrl->L.mode = XHI;	break;	/* Right side anchors */
						default:	Ctrl->L.mode = ZLO;	Ctrl->L.value = atof (&c[2]);	break;	/* Arbitrary x anchor */
					}
					Ctrl->L.anchor = PSXY_POL_X;
				}
				else if ((c = strstr (opt->arg, "+y")) != NULL) {	/* Parse y anchors for a polygon */
					switch (c[2]) {
						case 'b':	Ctrl->L.mode = YLO;	break;	/* Bottom side anchors */
						case 't':	Ctrl->L.mode = YHI;	break;	/* Top side anchors */
						default:	Ctrl->L.mode = ZHI;	Ctrl->L.value = atof (&c[2]);	break;	/* Arbitrary y anchor */
					}
					Ctrl->L.anchor = PSXY_POL_Y;
				}
				else	/* Just force a closed polygon */
					Ctrl->L.polygon = true;
				if ((c = strstr (opt->arg, "+p")) != NULL) {	/* Want outline */
					if (c[2] && gmt_getpen (GMT, &c[2], &Ctrl->L.pen)) {
						gmt_pen_syntax (GMT, 'W', NULL, "sets pen attributes [no outline]", 0);
						n_errors++;
					}
					Ctrl->L.outline = 1;
				}
				break;
			case 'N':	/* Do not clip to map */
				Ctrl->N.active = true;
				if (opt->arg[0] == 'r') Ctrl->N.mode = PSXYZ_NO_CLIP_REPEAT;
				else if (opt->arg[0] == 'c') Ctrl->N.mode = PSXYZ_CLIP_NO_REPEAT;
				else if (opt->arg[0] == '\0') Ctrl->N.mode = PSXYZ_NO_CLIP_NO_REPEAT;
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -N: Unrecognized argument %s\n", opt->arg);
					n_errors++;
				}
				break;
			case 'Q':	/* Do not sort symbols based on distance */
				Ctrl->Q.active = true;
				break;
			case 'S':		/* Get symbol [and size] */
				Ctrl->S.active = true;
				Ctrl->S.arg = strdup (opt->arg);
				break;
			case 'T':		/* Skip all input files */
				Ctrl->T.active = true;
				break;
			case 'W':		/* Set line attributes */
				Ctrl->W.active = true;
				if (opt->arg[0] == '-' || (opt->arg[0] == '+' && opt->arg[1] != 'c')) {	/* Definitively old-style args */
					if (gmt_M_compat_check (API->GMT, 5)) {	/* Sorry */
						GMT_Report (API, GMT_MSG_ERROR, "Your -W syntax is obsolete; see program usage.\n");
						n_errors++;
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Your -W syntax is obsolete; see program usage.\n");
						n_errors += parse_old_W (API, Ctrl, opt->arg);
					}
				}
				else if (opt->arg[0] && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", 11);
					n_errors++;
				}
				if (Ctrl->W.pen.cptmode) Ctrl->W.cpt_effect = true;
				break;

			case 'Z':		/* Get value for CPT lookup */
				if (three_D && strchr ("lf", opt->arg[0]) == NULL && API->GMT->current.setting.run_mode == GMT_CLASSIC) {	/* Could possibly be old-style 3-D -Z<level> setting */
					GMT_Report (API, GMT_MSG_COMPAT, "Cannot tell if your -Z is old-style 3-D zlevel or <level> for CPT lookup.\n");
					GMT_Report (API, GMT_MSG_COMPAT, "If old-style 3-D zlevel, please use -p instead.\n");
				}
				Ctrl->Z.active = true;
				ztype = (strchr (opt->arg, 'T')) ? GMT_IS_ABSTIME : gmt_M_type (GMT, GMT_IN, GMT_Z+1);
				switch (opt->arg[0]) {
					case 'l': Ctrl->Z.mode = 1; j=1; break;
					case 'f': Ctrl->Z.mode = 2; j=1; break;
					default:  Ctrl->Z.mode = 3; j=0; break;
				}
				n_errors += gmt_verify_expectations (GMT, ztype, gmt_scanf_arg (GMT, &opt->arg[j], ztype, false, &Ctrl->Z.value), &opt->arg[j]);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	if (Ctrl->T.active) GMT_Report (API, GMT_MSG_WARNING, "Option -T ignores all input files\n");

	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && !Ctrl->C.active, "Option -Z: No CPT given via -C\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->G.active, "Option -Z: Not compatible with -G\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && (Ctrl->C.file == NULL || Ctrl->C.file[0] == '\0'), "Option -C: No CPT given\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && gmt_parse_symbol_option (GMT, Ctrl->S.arg, S, 1, true), "Option -S: Parsing failure\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && S->symbol == GMT_SYMBOL_NOT_SET, "Binary input data cannot have symbol information\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.pen.cptmode && !Ctrl->C.active, "Option -W modifier +c requires the -C option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.anchor && (!Ctrl->G.active && !Ctrl->Z.active) && !Ctrl->L.outline, "Option -L<modifiers> must include +p<pen> if -G not given\n");
	if (Ctrl->S.active && S->symbol == GMT_SYMBOL_COLUMN) {
		n = get_column_bands (S);
		n_errors += gmt_M_check_condition (GMT, n > 1 && !Ctrl->C.active, "Option -So|O with multiple layers requires -C\n");
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void column3D (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double z, double *dim, double rgb[3][4], int outline) {
	int i, k;
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
				/* Purposefully fall through after flipping the sign */
			case 1:	/* negative side */
				gmt_plane_perspective (GMT, GMT_X, x + sign * x_size);
				PSL_plotbox (PSL, y - y_size, z - z_size, y + y_size, z + z_size);
				break;
			case 2:	/* xz plane positive side */
				sign = 1.0;
				/* Purposefully fall through after flipping the sign */
			case 3:	/* negative side */
				gmt_plane_perspective (GMT, GMT_Y, y + sign * y_size);
				PSL_plotbox (PSL, x - x_size, z - z_size, x + x_size, z + z_size);
				break;
			case 4:	/* xy plane positive side */
				sign = 1.0;
				/* Purposefully fall through after flipping the sign */
			case 5:	/* negative side */
				gmt_plane_perspective (GMT, GMT_Z, z + sign * z_size);
				PSL_plotbox (PSL, x - x_size, y - y_size, x + x_size, y + y_size);
				break;
		}
	}
}

GMT_LOCAL int dist_compare (const void *a, const void *b) {
	if (((struct PSXYZ_DATA *)a)->dist[0] < ((struct PSXYZ_DATA *)b)->dist[0]) return (-1);
	if (((struct PSXYZ_DATA *)a)->dist[0] > ((struct PSXYZ_DATA *)b)->dist[0]) return (1);
	if (((struct PSXYZ_DATA *)a)->dist[1] < ((struct PSXYZ_DATA *)b)->dist[1]) return (-1);
	if (((struct PSXYZ_DATA *)a)->dist[1] > ((struct PSXYZ_DATA *)b)->dist[1]) return (1);
#if defined(WIN32) || defined(__MINGW32__)
	/* MSVC qsort call a quick sorting function when number of elements to sort is small. e.g.

	    * below a certain size, it is faster to use a O(n^2) sorting method *
		if (size <= CUTOFF) {
			__SHORTSORT(lo, hi, width, comp, context);
		}
		and that function damn looks bugged as it imposes

		if (__COMPARE(context, p, max) > 0) { ...

		as condition to NOT change order (instead of >= 0). Se we force the hand here and
		return 1 to circumvent that bad behavior

		Note: Since it uses MSVCRT.DLL MinGW GCC suffers from the same bug
	*/
	return (1);
#else
	return (0);
#endif
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_plot3d (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: plot3d\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psxyz (V_API, mode, args);
}

int GMT_psxyz (void *V_API, int mode, void *args) {
	/* High-level function that implements the psxyz task */
	bool polygon, penset_OK = true, not_line, old_is_world;
	bool get_rgb = false, read_symbol, clip_set = false, fill_active, rgb_from_z = false, QR_symbol = false;
	bool default_outline, outline_active, save_u = false, geovector = false, can_update_headpen = true;
	unsigned int k, j, geometry, tbl, pos2x, pos2y, icol = 0, tcol = 0;
	unsigned int n_cols_start = 3, justify, v4_outline = 0, v4_status = 0;
	unsigned int col, bcol, ex1, ex2, ex3, change, n_needed, n_z = 0;
	int error = GMT_NOERROR;

	uint64_t i, n, n_total_read = 0;
	size_t n_alloc = 0;

	char s_args[GMT_BUFSIZ] = {""};

	double dim[PSL_MAX_DIMS], rgb[3][4] = {{-1.0, -1.0, -1.0, 0.0}, {-1.0, -1.0, -1.0, 0.0}, {-1.0, -1.0, -1.0, 0.0}};
	double DX = 0, DY = 0, *xp = NULL, *yp = NULL, *in = NULL, *v4_rgb = NULL;
	double lux[3] = {0.0, 0.0, 0.0}, tmp, x_1, x_2, y_1, y_2, dx, dy, s, c, zz, length, base;

	struct GMT_PEN default_pen, current_pen, last_headpen, last_spiderpen;
	struct GMT_FILL default_fill, current_fill, black, no_fill;
	struct GMT_SYMBOL S;
	struct GMT_PALETTE *P = NULL;
	struct GMT_PALETTE_HIDDEN *PH = NULL;
	struct GMT_DATASEGMENT *L = NULL;
	struct PSXYZ_DATA *data = NULL;
	struct PSXYZ_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {	/* Classic requires options, while modern does not */
		if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */
	}
	else {
		if (options && options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */
		if (API->usage || (options && options->option == GMT_OPT_USAGE)) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	}

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	/* Initialize GMT_SYMBOL structure */

	gmt_M_memset (&S, 1, struct GMT_SYMBOL);
	gmt_M_memset (&last_headpen, 1, struct GMT_PEN);
	gmt_M_memset (&last_spiderpen, 1, struct GMT_PEN);
	gmt_contlabel_init (GMT, &S.G, 0);

	S.base = GMT->session.d_NaN;
	S.font = GMT->current.setting.font_annot[GMT_PRIMARY];
	S.u = GMT->current.setting.proj_length_unit;

	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options, &S)) != 0) Return (error);

	/*---------------------------- This is the psxyz main code ----------------------------*/

	if (!Ctrl->T.active) GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	GMT->current.plot.mode_3D = 1;	/* Only do background axis first; do foreground at end */

	/* Do we plot actual symbols, or lines */
	not_line = (S.symbol != GMT_SYMBOL_FRONT && S.symbol != GMT_SYMBOL_QUOTED_LINE && S.symbol != GMT_SYMBOL_LINE);

	read_symbol = (S.symbol == GMT_SYMBOL_NOT_SET);
	gmt_init_fill (GMT, &black, 0.0, 0.0, 0.0);	/* Default fill for points, if needed */
	gmt_init_fill (GMT, &no_fill, -1.0, -1.0, -1.0);

	if (Ctrl->C.active) {
		if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		get_rgb = not_line;	/* Need to assign color from either z or text from input data file */
		if (Ctrl->Z.active) {	/* Get color from cpt -Z and store in -G */
			double rgb[4];
			(void)gmt_get_rgb_from_z (GMT, P, Ctrl->Z.value, rgb);
			if (Ctrl->Z.mode & 1) {	/* To be used in polygon or symbol outline */
				gmt_M_rgb_copy (Ctrl->W.pen.rgb, rgb);
				Ctrl->W.active = true;	/* -C plus -Zl = -W */
			}
			if (Ctrl->Z.mode & 2) {	/* To be used in polygon or symbol fill */
				gmt_M_rgb_copy (Ctrl->G.fill.rgb, rgb);
				Ctrl->G.active = Ctrl->L.active = true;	/* -C plus -Zf = -G */
			}
			get_rgb = false;	/* Not reading z from data */
		}
		else if ((P->categorical & 2))	/* Get rgb from trailing text, so read no extra z columns */
			rgb_from_z = false;
		else {	/* Read extra z column for symbols only */
			rgb_from_z = not_line;
			if (rgb_from_z && (P->categorical & 2) == 0) n_cols_start++;
		}
	}

	polygon = (S.symbol == GMT_SYMBOL_LINE && (Ctrl->G.active || Ctrl->L.polygon) && !Ctrl->L.anchor);
	default_pen = current_pen = Ctrl->W.pen;
	current_fill = default_fill = (S.symbol == PSL_DOT && !Ctrl->G.active) ? black : Ctrl->G.fill;
	default_outline = Ctrl->W.active;
	if (Ctrl->I.active && Ctrl->I.mode == 0) {
		gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
		gmt_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
	}

	if (get_rgb && S.symbol == GMT_SYMBOL_COLUMN && (n_z = get_column_bands (&S)) > 1) get_rgb = rgb_from_z = false;	/* Not used in the same way here */
	if (Ctrl->L.anchor == PSXY_POL_SYMM_DEV) n_cols_start += 1;
	if (Ctrl->L.anchor == PSXY_POL_ASYMM_DEV || Ctrl->L.anchor == PSXY_POL_ASYMM_ENV) n_cols_start += 2;

	/* Extra columns 1, 2, and 3 */
	ex1 = (rgb_from_z) ? 4 : 3;
	ex2 = (rgb_from_z) ? 5 : 4;
	ex3 = (rgb_from_z) ? 6 : 5;
	pos2x = ex1 + GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd longitude (for VECTORS with two sets of coordinates) */
	pos2y = ex2 - GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd latitude (for VECTORS with two sets of coordinates) */
	if (S.symbol == GMT_SYMBOL_COLUMN) {
		n_z = get_column_bands (&S);	/* > 0 for multiband, else 0 */
		n_needed = n_cols_start + ((n_z > 1) ? n_z - 1 : S.n_required);
	}
	else
		n_needed = n_cols_start + S.n_required;
	if (not_line) {
		for (j = n_cols_start; j < 7; j++) gmt_set_column (GMT, GMT_IN, j, GMT_IS_DIMENSION);	/* Since these may have units appended */
		for (j = 0; j < S.n_nondim; j++) gmt_set_column (GMT, GMT_IN, S.nondim_col[j]+rgb_from_z, GMT_IS_FLOAT);	/* Since these are angles or km, not dimensions */
	}
	if (Ctrl->I.mode) {
		n_needed++;	/* Read intensity from data file */
		icol = n_needed - 1;
		gmt_set_column (GMT, GMT_IN, icol, GMT_IS_FLOAT);
	}
	if (GMT->common.t.variable) {
		n_needed++;	/* Read transparency from data file */
		tcol = n_needed - 1;
		gmt_set_column (GMT, GMT_IN, tcol, GMT_IS_FLOAT);
	}

	if (gmt_check_binary_io (GMT, n_needed))
		Return (GMT_RUNTIME_ERROR);

	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		if (gmt_contlabel_prep (GMT, &S.G, NULL))
			Return (GMT_RUNTIME_ERROR);
		penset_OK = false;	/* Since it is set in PSL */
	}

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), ""))
		Return (GMT_PROJECTION_ERROR);

	if (S.u_set) {	/* When -Sc<unit> is given we temporarily reset the system unit to these units so conversions will work */
		save_u = GMT->current.setting.proj_length_unit;
		GMT->current.setting.proj_length_unit = S.u;
	}

	lux[0] = fabs (GMT->current.proj.z_project.sin_az * GMT->current.proj.z_project.cos_el);
	lux[1] = fabs (GMT->current.proj.z_project.cos_az * GMT->current.proj.z_project.cos_el);
	lux[2] = fabs (GMT->current.proj.z_project.sin_el);
	tmp = MAX (lux[0], MAX (lux[1], lux[2]));
	for (k = 0; k < 3; k++) lux[k] = (lux[k] / tmp) - 0.5;

	if ((S.symbol == GMT_SYMBOL_COLUMN || S.symbol == GMT_SYMBOL_CUBE) && (!Ctrl->C.active || current_fill.rgb[0] >= 0)) {	/* Modify the color for each facet */
		for (k = 0; k < 3; k++) {
			gmt_M_rgb_copy (rgb[k], current_fill.rgb);
			if (S.shade3D) {
				GMT_Report (API, GMT_MSG_DEBUG, "3-D shading illusion: lux[k] = %g\n", k, lux[k]);
				gmt_illuminate (GMT, lux[k], rgb[k]);
			}
		}
	}

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
		else if (Ctrl->N.mode == PSXYZ_CLIP_REPEAT || Ctrl->N.mode == PSXYZ_CLIP_NO_REPEAT)	/* Only set clip if plotting symbols and -N not used */
			clip_set = true;
	}
	if (clip_set) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);

	if (S.symbol == GMT_SYMBOL_TEXT && Ctrl->G.active && !Ctrl->W.active) PSL_setcolor (PSL, current_fill.rgb, PSL_IS_FILL);
	if (S.symbol == GMT_SYMBOL_TEXT) gmt_setfont (GMT, &S.font);		/* Set the required font */
	if ((S.symbol == PSL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR) && S.v.status & PSL_VEC_JUST_S) {
		/* Reading 2nd coordinate so must set column types */
		gmt_set_column (GMT, GMT_IN, pos2x, gmt_M_type (GMT, GMT_IN, GMT_X));
		gmt_set_column (GMT, GMT_IN, pos2y, gmt_M_type (GMT, GMT_IN, GMT_Y));
	}
	if (S.symbol == PSL_VECTOR && S.v.status & PSL_VEC_COMPONENTS)
		gmt_set_column (GMT, GMT_IN, pos2y, GMT_IS_FLOAT);	/* Just the users dy component, not length */
	if (S.symbol == PSL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR || S.symbol == PSL_MARC ) {	/* One of the vector symbols */
		geovector = (S.symbol == GMT_SYMBOL_GEOVECTOR);
		if (S.v.status & PSL_VEC_FILL2) {	/* Gave +g<fill> to set head fill; odd, but overrides -G (and sets -G true) */
			current_fill = S.v.fill;	/* Override any -G<fill> with specified head fill */
			if (S.v.status & PSL_VEC_FILL) Ctrl->G.active = true;
		}
		else if (S.v.status & PSL_VEC_FILL) {
			current_fill = default_fill, Ctrl->G.active = true;	/* Return to default fill */
		}
		if (S.v.status & PSL_VEC_OUTLINE2) {	/* Vector head outline pen specified separately */
			last_headpen = S.v.pen;
		}
		else {	/* Reset to default pen */
			current_pen = default_pen, Ctrl->W.active = true;	/* Return to default pen */
			if (Ctrl->W.active) {	/* Vector head outline pen default is half that of stem pen */
				last_headpen = current_pen;
				last_headpen.width *= 0.5;
			}
		}
		if (Ctrl->C.active) {	/* Head fill and/or pen will be set via CPT lookup */
			if (!Ctrl->W.cpt_effect || (Ctrl->W.cpt_effect && (Ctrl->W.pen.cptmode & 2)))
				Ctrl->G.active = false;	/* Must turn off -G so that color is not reset to Ctrl->G.fill after the -C effect */
		}
	}
	bcol = (S.read_size) ? ex2 : ex1;
	if (S.symbol == GMT_SYMBOL_BARX && S.base_set & 2) gmt_set_column (GMT, GMT_IN, bcol, gmt_M_type (GMT, GMT_IN, GMT_Y));
	if (S.symbol == GMT_SYMBOL_BARY && S.base_set & 2) gmt_set_column (GMT, GMT_IN, bcol, gmt_M_type (GMT, GMT_IN, GMT_Y));
	if (penset_OK) gmt_setpen (GMT, &current_pen);
	QR_symbol = (S.symbol == GMT_SYMBOL_CUSTOM && (!strcmp (S.custom->name, "QR") || !strcmp (S.custom->name, "QR_transparent")));
	fill_active = Ctrl->G.active;	/* Make copies because we will change the values */
	outline_active =  Ctrl->W.active;
	if (not_line && !outline_active && S.symbol != PSL_WEDGE && !fill_active && !get_rgb && !QR_symbol) outline_active = true;	/* If no fill nor outline for symbols then turn outline on */

	if (Ctrl->D.active) {
		/* Shift the plot a bit. This is a bit frustrating, since the only way to do this
		   easily is to undo the perspective, shift, then redo. */
		gmt_plane_perspective (GMT, -1, 0.0);
		gmt_xyz_to_xy (GMT, Ctrl->D.dx, Ctrl->D.dy, Ctrl->D.dz, &DX, &DY);
		PSL_setorigin (PSL, DX, DY, 0.0, PSL_FWD);
		gmt_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
	}
	GMT->current.io.skip_if_NaN[GMT_Z] = true;	/* Extend GMT NaN-handling to the z-coordinate */
	if (P) PH = gmt_get_C_hidden (P);
	old_is_world = GMT->current.map.is_world;
	geometry = not_line ? GMT_IS_POINT : ((polygon) ? GMT_IS_POLY: GMT_IS_LINE);

	if (not_line) {	/* symbol part (not counting GMT_SYMBOL_FRONT and GMT_SYMBOL_QUOTED_LINE) */
		bool periodic = false, delayed_unit_scaling[2] = {false, false};
		unsigned int n_warn[3] = {0, 0, 0}, warn, item, n_times, last_time;
		double in2[7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, *p_in = GMT->current.io.curr_rec;
		double xpos[2], width, d;
		struct GMT_RECORD *In = NULL;

		if ((error = GMT_Set_Columns (API, GMT_IN, n_needed, GMT_COL_FIX)) != GMT_NOERROR) {
			Return (error);
		}
		/* Determine if we need to worry about repeating periodic symbols */
		if (clip_set && (Ctrl->N.mode == PSXYZ_CLIP_REPEAT || Ctrl->N.mode == PSXYZ_NO_CLIP_REPEAT) && gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]) && gmt_M_is_geographic (GMT, GMT_IN)) {
			/* Only do this for projection where west and east are split into two separate repeating boundaries */
			periodic = (gmt_M_is_cylindrical (GMT) || gmt_M_is_misc (GMT));
			if (S.symbol == GMT_SYMBOL_GEOVECTOR) periodic = false;
		}
		n_times = (periodic) ? 2 : 1;	/* For periodic boundaries we plot each symbol twice to allow for periodic clipping */
		last_time = n_times - 1;
		if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		gmt_set_meminc (GMT, GMT_BIG_CHUNK);	/* Only a sizeable amount of PSXZY_DATA structures when we initially allocate */
		GMT->current.map.is_world = !(S.symbol == PSL_ELLIPSE && S.convert_angles);
		if (S.symbol == GMT_SYMBOL_GEOVECTOR && (S.v.status & PSL_VEC_JUST_S) == 0)
			gmt_set_column (GMT, GMT_IN, ex2, GMT_IS_GEODIMENSION);
		else if ((S.symbol == PSL_ELLIPSE || S.symbol == PSL_ROTRECT) && S.convert_angles) {
			if (S.n_required == 1)  {
				gmt_set_column (GMT, GMT_IN, ex1, GMT_IS_GEODIMENSION);
				p_in = in2;
			}
			else {
				gmt_set_column (GMT, GMT_IN, ex2, GMT_IS_GEODIMENSION);
				gmt_set_column (GMT, GMT_IN, ex3, GMT_IS_GEODIMENSION);
			}
		}
		else if (S.symbol == PSL_WEDGE) {
			if (S.v.status == PSL_VEC_OUTLINE2) {	/* Wedge spider pen specified separately */
				PSL_defpen (PSL, "PSL_spiderpen", S.v.pen.width, S.v.pen.style, S.v.pen.offset, S.v.pen.rgb);
				last_spiderpen = S.v.pen;
			}
			else if (Ctrl->W.active || S.w_type || !(Ctrl->G.active || Ctrl->C.active)) {	/* Use -W as wedge pen as well as outline, and default to this pen if neither -C, -W or -G given */
				current_pen = default_pen, Ctrl->W.active = true;	/* Return to default pen */
				if (Ctrl->W.active) {	/* Vector head outline pen default is half that of stem pen */
					PSL_defpen (PSL, "PSL_spiderpen", current_pen.width, current_pen.style, current_pen.offset, current_pen.rgb);
					last_spiderpen = current_pen;
				}
			}
		}
		else if (QR_symbol) {
			if (!Ctrl->G.active)	/* Default to black */
				PSL_command (PSL, "/QR_fill {0 A} def\n");
			if (!Ctrl->W.active)	/* No outline of QR code */
				PSL_command (PSL, "/QR_outline false def\n");
		}
		if (S.read_size && GMT->current.io.col[GMT_IN][ex1].convert) {	/* Doing math on the size column, must delay unit conversion unless inch */
			gmt_set_column (GMT, GMT_IN, ex1, GMT_IS_FLOAT);
			if (S.u_set)
				delayed_unit_scaling[GMT_X] = (S.u != GMT_INCH);
			else if (GMT->current.setting.proj_length_unit != GMT_INCH) {
				delayed_unit_scaling[GMT_X] = true;
				S.u = GMT->current.setting.proj_length_unit;
			}
		}
		if (S.read_size && GMT->current.io.col[GMT_IN][ex2].convert) {	/* Doing math on the size column, must delay unit conversion unless inch */
			gmt_set_column (GMT, GMT_IN, ex2, GMT_IS_FLOAT);
			delayed_unit_scaling[GMT_Y] = (S.u_set && S.u != GMT_INCH);	/* Since S.u will be set under GMT_X if that else branch kicked in */
		}

		if (!read_symbol) API->object[API->current_item[GMT_IN]]->n_expected_fields = n_needed;
		if (S.read_symbol_cmd)	/* Must prepare for a rough ride */
			GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_VAR);
		else if (GMT->common.l.active && !get_rgb && (!S.read_size || GMT->common.l.item.size > 0.0)) {
			/* For specified symbol, size, color we can do an auto-legend entry under modern mode */
			gmt_add_legend_item (API, &S, Ctrl->G.active, &(Ctrl->G.fill), Ctrl->W.active, &(Ctrl->W.pen), &(GMT->common.l.item));
		}
		n = 0;
		do {	/* Keep returning records until we reach EOF */
			if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				}
				else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
					break;
				else if (gmt_M_rec_is_segment_header (GMT)) {			/* Parse segment headers */
					PSL_comment (PSL, "Segment header: %s\n", GMT->current.io.segment_header);
					(void)gmt_parse_segment_header (GMT, GMT->current.io.segment_header, P, &fill_active, &current_fill, &default_fill, &outline_active, &current_pen, &default_pen, default_outline, NULL);
					if (Ctrl->I.active && Ctrl->I.mode == 0) {
						gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
						gmt_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
					}
					if (read_symbol) API->object[API->current_item[GMT_IN]]->n_expected_fields = GMT_MAX_COLUMNS;
					if (gmt_parse_segment_item (GMT, GMT->current.io.segment_header, "-S", s_args)) {	/* Found -Sargs */
						if (!(s_args[0] == 'q'|| s_args[0] == 'f')) { /* Update parameters */
							gmt_parse_symbol_option (GMT, s_args, &S, 0, false);
						}
						else
							GMT_Report (API, GMT_MSG_ERROR, "Segment header tries to switch to a line symbol like quoted line or fault - ignored\n");
					}
				}
				continue;							/* Go back and read the next record */
			}

			/* Data record to process */

			in = In->data;
			n_total_read++;

			if (read_symbol) {	/* Must do special processing */
				if (S.read_symbol_cmd == 1) {
					gmt_parse_symbol_option (GMT, In->text, &S, 1, false);
					if (S.symbol == GMT_SYMBOL_COLUMN) {
						n_z = get_column_bands (&S);
						if (n_z > 1 && !Ctrl->C.active) {
							GMT_Report (API, GMT_MSG_ERROR, "The -So|O option with multiple layers requires -C - skipping this point\n");
							continue;
						}
					}
					QR_symbol = (S.symbol == GMT_SYMBOL_CUSTOM && (!strcmp (S.custom->name, "QR") || !strcmp (S.custom->name, "QR_transparent")));
				}
				/* Since we only now know if some of the input columns should NOT be considered dimensions we
				 * must visit such columns and if the current length unit is NOT inch then we must undo the scaling */
				if (S.n_nondim && GMT->current.setting.proj_length_unit != GMT_INCH) {	/* Since these are not dimensions but angles or other quantities */
					for (j = 0; j < S.n_nondim; j++) in[S.nondim_col[j]+rgb_from_z] *= GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
				}

				if (S.symbol == PSL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR || S.symbol == PSL_MARC) {	/* One of the vector symbols */
					if (S.v.status & PSL_VEC_OUTLINE2) {	/* Vector head ouline pen specified separately */
						if (!gmt_M_same_pen (S.v.pen, last_headpen)) {
							last_headpen = S.v.pen;
						}
						can_update_headpen = false;
					}
					else {	/* Reset to default pen (or possibly not used) */
						current_pen = default_pen, Ctrl->W.active = true;	/* Return to default pen */
						if (Ctrl->W.active && !gmt_M_same_pen (current_pen, last_headpen)) {	/* Vector head outline pen default is half that of stem pen */
							last_headpen = current_pen;
							last_headpen.width *= 0.5;
						}
					}
					if (S.v.status & PSL_VEC_FILL2) {
						current_fill = S.v.fill;	/* Override -G<fill> with specified head fill */
						if (S.v.status & PSL_VEC_FILL) Ctrl->G.active = true;
					}
					else if (S.v.status & PSL_VEC_FILL) {
						current_fill = default_fill, Ctrl->G.active = true;	/* Return to default fill */
					}
					if (S.v.status & PSL_VEC_JUST_S) {	/* Got coordinates of tip instead of dir/length so need to undo dimension scaling */
						in[pos2x] *= GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
						in[pos2y] *= GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
					}
				}
				else if (S.symbol == PSL_WEDGE) {
					if (S.v.status == PSL_VEC_OUTLINE2) {	/* Wedge spider pen specified separately */
						PSL_defpen (PSL, "PSL_spiderpen", S.v.pen.width, S.v.pen.style, S.v.pen.offset, S.v.pen.rgb);
						last_spiderpen = S.v.pen;
					}
					else if (outline_active && !gmt_M_same_pen (current_pen, last_spiderpen)) {	/* Reset to new pen */
							PSL_defpen (PSL, "PSL_spiderpen", current_pen.width, current_pen.style, current_pen.offset, current_pen.rgb);
							last_spiderpen = current_pen;
					}
				}
				else if (S.symbol == PSL_DOT && !Ctrl->G.active) {	/* Must switch on default black fill */
					current_fill = black;
				}
			}

			/* Here, all in[*] beyond lon, lat, z will have been converted to inch if they had a trailing unit (e.g., 5c) */

			if (!Ctrl->N.active && (in[GMT_Z] < GMT->common.R.wesn[ZLO] || in[GMT_Z] > GMT->common.R.wesn[ZHI])) continue;
			if (!Ctrl->N.active && S.symbol != GMT_SYMBOL_BARX && S.symbol != GMT_SYMBOL_BARY) {
				/* Skip points outside map */
				gmt_map_outside (GMT, in[GMT_X], in[GMT_Y]);
				if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
			}

			if (get_rgb) {	/* Lookup t to get rgb */
				if (P->categorical & 2)
					gmt_get_fill_from_key (GMT, P, In->text, &current_fill);
				else
					gmt_get_fill_from_z (GMT, P, in[3], &current_fill);
				if (PH->skip) continue;	/* Chosen CPT indicates skip for this t */
				if (Ctrl->I.active) {
					if (Ctrl->I.mode == 0)
						gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
					else
						gmt_illuminate (GMT, in[icol], current_fill.rgb);
				}
			}
			else if (Ctrl->I.mode == 1) {	/* Must reset current file and then apply illumination */
				current_fill = default_fill = (S.symbol == PSL_DOT && !Ctrl->G.active) ? black : Ctrl->G.fill;
				gmt_illuminate (GMT, in[icol], current_fill.rgb);
			}

			if (QR_symbol) {
				if (Ctrl->G.active)	/* Change color of QR code */
					PSL_command (PSL, "/QR_fill {%s} def\n", PSL_makecolor (PSL, current_fill.rgb));
				if (outline_active) {	/* Draw outline of QR code */
					PSL_command (PSL, "/QR_outline true def\n");
					PSL_command (PSL, "/QR_pen {%s} def\n",  PSL_makepen (PSL, (1.0/6.0) * Ctrl->W.pen.width, Ctrl->W.pen.rgb, Ctrl->W.pen.style, Ctrl->W.pen.offset));
				}
				else
					PSL_command (PSL, "/QR_outline false def\n");
			}

			if (n == n_alloc) data = gmt_M_malloc (GMT, data, n, &n_alloc, struct PSXYZ_DATA);

			if (S.symbol == GMT_SYMBOL_BARX && (S.base_set & 4))
				in[GMT_X] += S.base;
			else if (S.symbol == GMT_SYMBOL_BARY && (S.base_set & 4))
				in[GMT_Y] += S.base;

			if (gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &data[n].x, &data[n].y) || gmt_M_is_dnan(in[GMT_Z])) continue;	/* NaNs on input */
			data[n].flag = S.convert_angles;
			data[n].z = gmt_z_to_zz (GMT, in[GMT_Z]);
			if (S.symbol == GMT_SYMBOL_COLUMN) {	/* Must allocate space for multiple z-values */
				bool skip = false;
				n_z = get_column_bands (&S);
				data[n].zz = gmt_M_memory (GMT, NULL, n_z, double);
				if (S.accumulate == false) {
					for (col = GMT_Z + 1, k = 1; k < n_z; k++, col++) {
						if (in[col] < in[col-1]) {
							GMT_Report (API, GMT_MSG_ERROR, "The -So|O option requires monotonically increasing z-values - not true near line %d\n", n_total_read);
							skip = true;
						}
					}
				}
				if (skip) continue;
				for (col = GMT_Z, k = 0; k < n_z; k++, col++ )
					data[n].zz[k] = gmt_z_to_zz (GMT, in[col]);
				data[n].flag = S.accumulate;
			}

			if (S.symbol == PSL_ELLIPSE || S.symbol == PSL_ROTRECT) {	/* Ellipses or rectangles */
				if (S.n_required == 0)	/* Degenerate ellipse or rectangle, Got diameter via S.size_x */
					in2[ex2] = in2[ex3] = S.size_x;	/* Duplicate diameter as major and minor axes */
				else if (S.n_required == 1)	/* Degenerate ellipse or rectangle, expect single diameter via input */
					in2[ex2] = in2[ex3] = in[ex1];	/* Duplicate diameter as major and minor axes */
			}

			if (S.base_set & 2) {	/* Got base from input column */
				bcol = (S.read_size) ? ex2 : ex1;
				if (S.symbol == GMT_SYMBOL_COLUMN)
					bcol += S.n_required - 1;	/* Since we have z1 z2 ... z2 base */
				S.base = in[bcol];
			}
			if (S.read_size) {	/* Update sizes from input */
				S.size_x = in[ex1] * S.factor;
				if (delayed_unit_scaling[GMT_X]) S.size_x *= GMT->session.u2u[S.u][GMT_INCH];
				S.size_y = in[ex2];
				if (delayed_unit_scaling[GMT_Y]) S.size_y *= GMT->session.u2u[S.u][GMT_INCH];
			}
			if (S.base_set & 4) data[n].flag |= 32;	/* Flag that base needs to be added to height(s) */

			if (Ctrl->W.cpt_effect) {
				if (Ctrl->W.pen.cptmode & 1) {	/* Change pen color via CPT */
					gmt_M_rgb_copy (Ctrl->W.pen.rgb, current_fill.rgb);
					current_pen = Ctrl->W.pen;
					if (can_update_headpen && !gmt_M_same_pen (current_pen, last_headpen))	/* Since color may have changed */
						last_headpen = current_pen;
				}
				if ((Ctrl->W.pen.cptmode & 2) == 0 && !Ctrl->G.active)	/* Turn off CPT fill */
					gmt_M_rgb_copy (current_fill.rgb, GMT->session.no_rgb);
				else if (Ctrl->G.active)
					current_fill = Ctrl->G.fill;
			}
			data[n].dim[0] = S.size_x;
			data[n].dim[1] = S.size_y;

			data[n].symbol = S.symbol;
			data[n].f = current_fill;
			data[n].p = current_pen;
			data[n].h = last_headpen;
			data[n].outline = outline_active ? 1 : 0;
			if (GMT->common.t.variable) data[n].transparency = 0.01 * in[tcol];	/* Specific transparency for current symbol if -t was given */
			data[n].string = NULL;
			/* Next two are for sorting:
			   dist[0] is layer "height": objects closer to the viewer have higher numbers
			   dist[1] is higher when objects are further above a place viewed from above or below a plane viewed from below */
			data[n].dist[0] = GMT->current.proj.z_project.sin_az * data[n].x + GMT->current.proj.z_project.cos_az * data[n].y;
			data[n].dist[1] = GMT->current.proj.z_project.sin_el * data[n].z;
			GMT_Report (API, GMT_MSG_DEBUG, "dist[0] = %g dist[1] = %g\n", data[n].dist[0], data[n].dist[1]);

			switch (S.symbol) {
				case GMT_SYMBOL_BARX:
					data[n].dim[2] = (gmt_M_is_dnan (S.base)) ? 0.0 : gmt_x_to_xx (GMT, S.base);
					break;
				case GMT_SYMBOL_BARY:
					data[n].dim[2] = (gmt_M_is_dnan (S.base)) ? 0.0 : gmt_y_to_yy (GMT, S.base);
					break;
				case GMT_SYMBOL_COLUMN:
					data[n].dim[2] = (gmt_M_is_dnan (S.base)) ? 0.0 : gmt_z_to_zz (GMT, S.base);
					break;
				case PSL_RNDRECT:
					if (gmt_M_is_dnan (in[ex3])) {
						GMT_Report (API, GMT_MSG_WARNING, "Rounded rectangle corner radius = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					data[n].dim[2] = in[ex3];	/* radius */
					/* Now fall through to do the rest under regular rectangle */
				case PSL_RECT:
					if (gmt_M_is_dnan (in[ex1])) {
						GMT_Report (API, GMT_MSG_WARNING, "Rounded rectangle width = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					if (gmt_M_is_dnan (in[ex2])) {
						GMT_Report (API, GMT_MSG_WARNING, "Rounded rectangle height = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					data[n].dim[0] = in[ex1];	/* x-dim */
					data[n].dim[1] = in[ex2];	/* y-dim */
					break;
				case PSL_ELLIPSE:
				case PSL_ROTRECT:
					if (gmt_M_is_dnan (p_in[ex1])) {
						GMT_Report (API, GMT_MSG_WARNING, "Ellipse/Rectangle angle = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					if (gmt_M_is_dnan (p_in[ex2])) {
						GMT_Report (API, GMT_MSG_WARNING, "Ellipse/Rectangle width or major axis = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					if (gmt_M_is_dnan (p_in[ex3])) {
						GMT_Report (API, GMT_MSG_WARNING, "Ellipse/Rectangle height or minor axis = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					if (!S.convert_angles) {	/* Got axes in current plot units, change to inches */
						data[n].dim[0] = p_in[ex1];	/* direction */
						data[n].dim[1] = p_in[ex2];
						data[n].dim[2] = p_in[ex3];
						gmt_flip_angle_d (GMT, &data[n].dim[0]);
					}
					else if (gmt_M_is_cartesian (GMT, GMT_IN)) {	/* Got axes in user units, change to inches */
						data[n].dim[0] = 90.0 - p_in[ex1];	/* Cartesian azimuth */
						data[n].dim[1] = p_in[ex2] * GMT->current.proj.scale[GMT_X];
						data[n].dim[2] = p_in[ex3] * GMT->current.proj.scale[GMT_Y];
						gmt_flip_angle_d (GMT, &data[n].dim[0]);
					}
					else {				/* Got axis in km */
						data[n].dim[0] = p_in[ex1];	/* Azimuth will be forwarded to gmt_geo_rectangle/ellipse */
						data[n].dim[1] = p_in[ex2];
						data[n].dim[2] = p_in[ex3];
						data[n].x = in[GMT_X];	/* Revert to longitude and latitude */
						data[n].y = in[GMT_Y];
						data[n].flag |= 2;	/* Signals to use GMT_geo_* routine */
					}
					break;
				case GMT_SYMBOL_TEXT:
					data[n].string = strdup (S.string);
					break;
				case PSL_VECTOR:
					gmt_init_vector_param (GMT, &S, false, false, NULL, false, NULL);	/* Update vector head parameters */
					if (S.v.parsed_v4 && gmt_M_compat_check (GMT, 4)) {	/* Got v_width directly from V4 syntax so no messing with it here if under compatibility */
						/* But have to improvise as far as outline|fill goes... */
						if (outline_active) S.v.status |= PSL_VEC_OUTLINE;	/* Choosing to draw head outline */
						if (fill_active) S.v.status |= PSL_VEC_FILL;		/* Choosing to fill head */
						if (!(S.v.status & PSL_VEC_OUTLINE) && !(S.v.status & PSL_VEC_FILL)) S.v.status |= PSL_VEC_OUTLINE;	/* Gotta do something */
					}
					else
						S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);

					if (S.v.status & PSL_VEC_COMPONENTS)	/* Read dx, dy in user units */
						d = d_atan2d (in[ex2+S.read_size], in[ex1+S.read_size]);
					else
						d = in[ex1+S.read_size];	/* Got direction */
					if (!S.convert_angles)	/* Use direction as given */
						data[n].dim[0] = d;	/* direction */
					else if (gmt_M_is_cartesian (GMT, GMT_IN))	/* Cartesian azimuth; change to direction */
						data[n].dim[0] = 90.0 - d;
					else	/* Convert geo azimuth to map direction */
						data[n].dim[0] = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, d);

					if (gmt_M_is_dnan (data[n].dim[0])) {
						GMT_Report (API, GMT_MSG_WARNING, "Vector azimuth = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					if (gmt_M_is_dnan (in[ex2+S.read_size])) {
						GMT_Report (API, GMT_MSG_WARNING, "Vector length = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					if (S.v.status & PSL_VEC_COMPONENTS)	/* Read dx, dy in user units */
						data[n].dim[1] = hypot (in[ex1+S.read_size], in[ex2+S.read_size]) * S.v.comp_scale;
					else
						data[n].dim[1] = in[ex2+S.read_size];	/* Got length */
					if (S.v.status & PSL_VEC_JUST_S) {	/* Got coordinates of tip instead of dir/length */
						gmt_geo_to_xy (GMT, in[pos2x], in[pos2y], &x_2, &y_2);
						if (gmt_M_is_dnan (x_2) || gmt_M_is_dnan (y_2)) {
							GMT_Report (API, GMT_MSG_ERROR, "Vector head coordinates contain NaNs near line %d. Skipped\n", n_total_read);
							continue;
						}
						data[n].dim[1] = hypot (data[n].x - x_2, data[n].y - y_2);	/* Compute vector length in case of shrinking */
					}
					else {
						gmt_flip_angle_d (GMT, &data[n].dim[0]);
						sincosd (data[n].dim[0], &s, &c);
						x_2 = data[n].x + data[n].dim[1] * c;
						y_2 = data[n].y + data[n].dim[1] * s;
						justify = PSL_vec_justify (S.v.status);	/* Return justification as 0-2 */
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
					if (S.v.parsed_v4) {	/* Parsed the old ways so plot the old ways... */
						data[n].dim[4] *= 0.5;	/* Since it was double in the parsing */
						data[n].symbol = GMT_SYMBOL_VECTOR_V4;
						data[n].dim[5] = GMT->current.setting.map_vector_shape;
					}
					else {
						data[n].dim[5] = S.v.v_shape;
						data[n].dim[6] = (double)S.v.status;
						data[n].dim[7] = (double)S.v.v_kind[0];	data[n].dim[8] = (double)S.v.v_kind[1];
						data[n].dim[9] = (double)S.v.v_trim[0];	data[n].dim[10] = (double)S.v.v_trim[1];
						data[n].dim[11] = s * data[n].h.width;	/* Possibly shrunk head pen width */
					}
					break;
				case GMT_SYMBOL_GEOVECTOR:
					gmt_init_vector_param (GMT, &S, true, Ctrl->W.active, &Ctrl->W.pen, Ctrl->G.active, &Ctrl->G.fill);	/* Update vector head parameters */
					if (S.v.status & PSL_VEC_OUTLINE2)
						S.v.v_width = (float)(S.v.pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					else
						S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					if (gmt_M_is_dnan (in[ex1+S.read_size])) {
						GMT_Report (API, GMT_MSG_WARNING, "Geovector azimuth = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					if (gmt_M_is_dnan (in[ex2+S.read_size])) {
						GMT_Report (API, GMT_MSG_WARNING, "Geovector length = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					data[n].dim[0] = in[ex1+S.read_size];	/* direction */
					data[n].dim[1] = in[ex2+S.read_size];	/* length */
					data[n].x = in[GMT_X];			/* Revert to longitude and latitude */
					data[n].y = in[GMT_Y];
					data[n].v = S.v;
					break;
				case PSL_MARC:
					gmt_init_vector_param (GMT, &S, false, false, NULL, false, NULL);	/* Update vector head parameters */
					S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					data[n].dim[0] = in[ex1+S.read_size];	/* Radius */
					data[n].dim[1] = in[ex2+S.read_size];	/* Start direction in degrees */
					data[n].dim[2] = in[ex3+S.read_size];	/* Stop direction in degrees */
					length = fabs (data[n].dim[2]-data[n].dim[1]);	/* Arc length in degrees */
					if (gmt_M_is_dnan (length)) {
						GMT_Report (API, GMT_MSG_WARNING, "Math angle arc length = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					s = (length < S.v.v_norm) ? length / S.v.v_norm : 1.0;
					data[n].dim[3] = s * S.v.h_length;	/* Length of (shrunk) vector head */
					data[n].dim[4] = s * S.v.h_width;	/* Width of (shrunk) vector head */
					data[n].dim[5] = s * S.v.v_width;	/* Thickness of (shrunk) vector */
					data[n].dim[6] = S.v.v_shape;
					data[n].dim[7] = (double)S.v.status;	/* Vector tributes */
					data[n].dim[8] = (double)S.v.v_kind[0];	data[n].dim[9] = (double)S.v.v_kind[1];
					data[n].dim[10] = (double)S.v.v_trim[0];	data[n].dim[11] = (double)S.v.v_trim[1];
					data[n].dim[12] = s * data[n].h.width;	/* Possibly shrunk head pen width */
					break;
				case PSL_WEDGE:
					if (gmt_M_is_dnan (in[ex1+S.read_size])) {
						GMT_Report (API, GMT_MSG_WARNING, "Wedge start angle = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					if (gmt_M_is_dnan (in[ex2+S.read_size])) {
						GMT_Report (API, GMT_MSG_WARNING, "Wedge stop angle = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					if (!S.convert_angles) {
						data[n].dim[1] = in[ex1+S.read_size];			/* Start direction in degrees */
						data[n].dim[2] = in[ex2+S.read_size];			/* Stop direction in degrees */
					}
					else if (gmt_M_is_cartesian (GMT, GMT_IN)) {	/* Got azimuths instead */
						data[n].dim[1] = 90.0 - in[ex1+S.read_size];		/* Start direction in degrees */
						data[n].dim[2] = 90.0 - in[ex2+S.read_size];		/* Stop direction in degrees */
					}
					else {
						data[n].dim[1] = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, in[ex1+S.read_size]);
						data[n].dim[2] = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, in[ex2+S.read_size]);
					}
					data[n].dim[3] = S.w_type;
					data[n].dim[4] = S.w_radius_i;
					data[n].dim[5] = S.w_dr;	/* In case there is a request for radially spaced arcs */
					data[n].dim[6] = S.w_da;	/* In case there is a request for angularly spaced radial lines */
					data[n].dim[7] = 0.0;	/* Reset */
					if (fill_active || get_rgb) data[n].dim[7] = 1;	/* Lay down filled wedge */
					if (outline_active) data[n].dim[7] += 2;	/* Draw wedge outline */
					if (!S.w_active) {	/* Not geowedge so scale to radii */
						data[n].dim[0] *= 0.5;
						data[n].dim[4] *= 0.5;
					}
					break;
				case GMT_SYMBOL_CUSTOM:
					data[n].custom = gmt_M_memory (GMT, NULL, 1, struct GMT_CUSTOM_SYMBOL);
					gmt_M_memcpy (data[n].custom, S.custom, 1, struct GMT_CUSTOM_SYMBOL);
					if (In->text) data[n].string = strdup (In->text);
					break;
			}
			if (S.user_unit[GMT_X]) data[n].flag |= 4;
			if (S.user_unit[GMT_Y]) data[n].flag |= 8;

			n++;
			if (read_symbol) API->object[API->current_item[GMT_IN]]->n_expected_fields = GMT_MAX_COLUMNS;
		} while (true);

		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}

		n_alloc = n;
		data = gmt_M_malloc (GMT, data, 0, &n_alloc, struct PSXYZ_DATA);

		/* Sort according to distance from viewer */

		if (!Ctrl->Q.active) qsort (data, n, sizeof (struct PSXYZ_DATA), dist_compare);

		/* Now plot these symbols one at the time */

		PSL_command (GMT->PSL, "V\n");
		for (i = 0; i < n; i++) {

			if (n_z == 1 || (data[i].symbol == GMT_SYMBOL_CUBE || data[i].symbol == GMT_SYMBOL_CUBE)) {
				for (j = 0; j < 3; j++) {
					gmt_M_rgb_copy (rgb[j], data[i].f.rgb);
					if (S.shade3D) gmt_illuminate (GMT, lux[j], rgb[j]);
				}
			}

			if (!geovector) {	/* Vectors do it separately */
				gmt_setfill (GMT, &data[i].f, data[i].outline);
				gmt_setpen (GMT, &data[i].p);
			}
			if (QR_symbol) {
				if (Ctrl->G.active)	/* Change color of QR code */
					PSL_command (PSL, "/QR_fill {%s} def\n", PSL_makecolor (PSL, data[i].f.rgb));
				if (data[i].outline) {	/* Draw outline of QR code */
					PSL_command (PSL, "/QR_outline true def\n");
					PSL_command (PSL, "/QR_pen {%s} def\n",  PSL_makepen (PSL, (1.0/6.0) * data[i].p.width, data[i].p.rgb, data[i].p.style, data[i].p.offset));
				}
				else
					PSL_command (PSL, "/QR_outline false def\n");
			}
			if (GMT->common.t.variable)	/* Update the transparency for current symbol */
				PSL_settransparency (PSL, data[i].transparency);

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
				switch (data[i].symbol) {
					case GMT_SYMBOL_NONE:
						break;
					case GMT_SYMBOL_BARX:
						if (!Ctrl->N.active) in[GMT_X] = MAX (GMT->common.R.wesn[XLO], MIN (xpos[item], GMT->common.R.wesn[XHI]));
						if (data[i].flag & 4) {
							gmt_geo_to_xy (GMT, xpos[item], data[i].y - 0.5 * data[i].dim[0], &x_1, &y_1);
							gmt_geo_to_xy (GMT, xpos[item], data[i].y + 0.5 * data[i].dim[0], &x_2, &y_2);
							data[i].dim[0] = 0.5 * hypot (x_1 - x_2, y_1 - y_2);
						}
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						PSL_plotbox (PSL, xpos[item], data[i].y - 0.5 * data[i].dim[0], data[i].dim[2], data[i].y + 0.5 * data[i].dim[0]);
						break;
					case GMT_SYMBOL_BARY:
						if (!Ctrl->N.active) in[GMT_Y] = MAX (GMT->common.R.wesn[YLO], MIN (data[i].y, GMT->common.R.wesn[YHI]));
						if (data[i].flag & 4) {
							gmt_geo_to_xy (GMT, xpos[item] - 0.5 * data[i].dim[0], data[i].y, &x_1, &y_1);
							gmt_geo_to_xy (GMT, xpos[item] + 0.5 * data[i].dim[0], data[i].y, &x_2, &y_2);
							data[i].dim[0] = 0.5 * hypot (x_1 - x_2, y_1 - y_2);
						}
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						PSL_plotbox (PSL, xpos[item] - 0.5 * data[i].dim[0], data[i].y, xpos[item] + 0.5 * data[i].dim[0], data[i].dim[2]);
						break;
					case GMT_SYMBOL_COLUMN:
						if (data[i].flag & 4) {
							gmt_geo_to_xy (GMT, xpos[item] - data[i].dim[0], data[i].y, &x_1, &y_1);
							gmt_geo_to_xy (GMT, xpos[item] + data[i].dim[0], data[i].y, &x_2, &y_2);
							dim[0] = 0.5 * hypot (x_1 - x_2, y_1 - y_2);
						}
						else
							dim[0] = data[i].dim[0];
						if (data[i].flag & 8) {
							gmt_geo_to_xy (GMT, xpos[item], data[i].y - data[i].dim[1], &x_1, &y_1);
							gmt_geo_to_xy (GMT, xpos[item], data[i].y + data[i].dim[1], &x_2, &y_2);
							dim[1] = 0.5 * hypot (x_1 - x_2, y_1 - y_2);
						}
						else
							dim[1] = data[i].dim[1];
						base = data[i].dim[2];	zz = 0.0;
						for (k = 0; k < n_z; k++) {	/* For each band in the column */
							if (Ctrl->C.active && n_z > 1) {
								/* Must update band color based on band number k */
								gmt_get_fill_from_z (GMT, P, k+0.5, &current_fill);
								for (j = 0; j < 3; j++) {
									gmt_M_rgb_copy (rgb[j], current_fill.rgb);
									if (S.shade3D) gmt_illuminate (GMT, lux[j], rgb[j]);
								}
							}
							if (data[i].flag & 1)
								zz += data[i].zz[k];	/* Must get cumulate z value from dz increments */
							else {
								zz = data[i].zz[k];	/* Got actual z values */
								if (data[i].flag & 32) zz += base;		/* Must add base to z height */
							}
							dim[2] = fabs (zz - base);	/* band height */
							column3D (GMT, PSL, xpos[item], data[i].y, (zz + base) / 2.0, dim, rgb, data[i].outline);
							base = zz;	/* Next base */
						}
						if (item == last_time) gmt_M_free (GMT, data[i].zz);	/* Free column band array */
						break;
					case GMT_SYMBOL_CUBE:
						if (data[i].flag & 4) {
							gmt_geo_to_xy (GMT, xpos[item] - data[i].dim[0], data[i].y, &x_1, &y_1);
							gmt_geo_to_xy (GMT, xpos[item] + data[i].dim[0], data[i].y, &x_2, &y_2);
							dim[0] = 0.5 * hypot (x_1 - x_2, y_1 - y_2);
						}
						else
							dim[0] = data[i].dim[0];
						dim[1] = dim[2] = dim[0];
						column3D (GMT, PSL, xpos[item], data[i].y, data[i].z, dim, rgb, data[i].outline);
						break;
					case PSL_CROSS:
					case PSL_PLUS:
					case PSL_DOT:
					case PSL_XDASH:
					case PSL_YDASH:
					case PSL_STAR:
					case PSL_CIRCLE:
					case PSL_SQUARE:
					case PSL_HEXAGON:
					case PSL_PENTAGON:
					case PSL_OCTAGON:
					case PSL_TRIANGLE:
					case PSL_INVTRIANGLE:
					case PSL_DIAMOND:
					case PSL_RECT:
					case PSL_RNDRECT:
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						PSL_plotsymbol (PSL, xpos[item], data[i].y, data[i].dim, data[i].symbol);
						break;
					case PSL_ELLIPSE:
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						if (data[i].flag & 2)
							gmt_plot_geo_ellipse (GMT, xpos[item], data[i].y, data[i].dim[1], data[i].dim[2], data[i].dim[0]);
						else
							PSL_plotsymbol (PSL, xpos[item], data[i].y, data[i].dim, PSL_ELLIPSE);
						break;
					case PSL_ROTRECT:
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						if (data[i].flag & 2)
							gmt_geo_rectangle (GMT, xpos[item], data[i].y, data[i].dim[1], data[i].dim[2], data[i].dim[0]);
						else
							PSL_plotsymbol (PSL, xpos[item], data[i].y, data[i].dim, PSL_ROTRECT);
						break;
					case GMT_SYMBOL_TEXT:
						if (fill_active && !data[i].outline)
							PSL_setcolor (PSL, data[i].f.rgb, PSL_IS_FILL);
						else if (!fill_active)
							PSL_setfill (PSL, GMT->session.no_rgb, data[i].outline);
						(void) gmt_setfont (GMT, &S.font);
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						PSL_plottext (PSL, xpos[item], data[i].y, data[i].dim[0] * PSL_POINTS_PER_INCH, data[i].string, 0.0, PSL_MC, data[i].outline);
						gmt_M_str_free (data[i].string);
						break;
					case PSL_VECTOR:
						PSL_defpen (PSL, "PSL_vecheadpen", data[i].h.width, data[i].h.style, data[i].h.offset, data[i].h.rgb);
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						PSL_plotsymbol (PSL, xpos[item], data[i].y, data[i].dim, PSL_VECTOR);
						break;
					case GMT_SYMBOL_VECTOR_V4:
						v4_outline = Ctrl->W.active;
						if (Ctrl->G.active)
							v4_rgb = Ctrl->G.fill.rgb;
						else if (Ctrl->C.active)
							v4_rgb = data[i].f.rgb;
						else
							v4_rgb = GMT->session.no_rgb;
						if (v4_outline) gmt_setpen (GMT, &Ctrl->W.pen);
						v4_status = lrint (data[n].dim[6]);
						if (v4_status & PSL_VEC_BEGIN) v4_outline += 8;	/* Double-headed */
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						psl_vector_v4 (PSL, xpos[item], data[i].y, data[i].dim, v4_rgb, v4_outline);
						break;
					case GMT_SYMBOL_GEOVECTOR:
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						S.v = data[i].v;	/* Update vector attributes from saved values */
						warn = gmt_geo_vector (GMT, xpos[item], data[i].y, data[i].dim[0], data[i].dim[1], &data[i].p, &S);
						n_warn[warn]++;
						break;
					case PSL_MARC:
						PSL_defpen (PSL, "PSL_vecheadpen", data[i].h.width, data[i].h.style, data[i].h.offset, data[i].h.rgb);
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						PSL_plotsymbol (PSL, xpos[item], data[i].y, data[i].dim, PSL_MARC);
						break;
					case PSL_WEDGE:
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						if (S.w_active)	{	/* Geo-wedge */
							unsigned int status = lrint (data[i].dim[3]);
							gmt_xy_to_geo (GMT, &dx, &dy, data[i].y, data[i].y);	/* Just recycle dx, dy here */
							gmt_geo_wedge (GMT, dx, dy, data[n].dim[4], S.w_radius, data[n].dim[5], data[i].dim[1], data[i].dim[2], data[n].dim[6], status, fill_active || get_rgb, outline_active);
						}
						else {
							PSL_plotsymbol (PSL, xpos[item], data[i].y, data[i].dim, PSL_WEDGE);
						}
						break;
					case GMT_SYMBOL_ZDASH:
						gmt_xyz_to_xy (GMT, xpos[item], data[i].y, data[i].z, &x_1, &y_1);
						gmt_plane_perspective (GMT, -1, 0.0);
						PSL_plotsymbol (PSL, x_1, y_1, data[i].dim, PSL_YDASH);
						break;
					case GMT_SYMBOL_CUSTOM:
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						dim[0] = data[i].dim[0];
#if 0
						for (j = 0; S.custom->type && j < S.n_required; j++) {	/* Convert any azimuths to plot angles first */
							if (S.custom->type[j] == GMT_IS_AZIMUTH) {	/* Make sure angles are 0-360 for macro conditionals */
								dim[j+1] = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, data[i].dim[j]);
								if (dim[j+1] < 0.0) dim[j+1] += 360.0;
							}
							else {	/* Angles (enforce 0-360), dimensions or other quantities */
								dim[j+1] = data[i].dim[j];
								if (S.custom->type[j] == GMT_IS_ANGLE && dim[j+1] < 0.0) dim[j+1] += 360.0;
							}
						}
#endif
						for (j = 0; S.custom->type && j < S.n_required; j++) {
							/* Angles (enforce 0-360), dimensions or other quantities */
							dim[j+1] = data[i].dim[j];
							if (S.custom->type[j] == GMT_IS_ANGLE && dim[j+1] < 0.0) dim[j+1] += 360.0;
						}
						if (!S.custom->start) S.custom->start = (get_rgb) ? 4 : 3;
						gmt_draw_custom_symbol (GMT, xpos[item], data[i].y, dim, data[i].string, data[i].custom, &data[i].p, &data[i].f, data[i].outline);
						gmt_M_free (GMT, data[i].custom);
						if (data[i].string) gmt_M_str_free (data[i].string);
						break;
				}
			}
		}
		PSL_command (GMT->PSL, "U\n");
		if (n_warn[1]) GMT_Report (API, GMT_MSG_INFORMATION, "%d vector heads had length exceeding the vector length and were skipped. Consider the +n<norm> modifier to -S\n", n_warn[1]);
		if (n_warn[2]) GMT_Report (API, GMT_MSG_INFORMATION, "%d vector heads had to be scaled more than implied by +n<norm> since they were still too long. Consider changing the +n<norm> modifier to -S\n", n_warn[2]);
		gmt_M_free (GMT, data);
		gmt_reset_meminc (GMT);
	}
	else {	/* Line/polygon part */
		uint64_t seg;
		struct GMT_DATASET *D = NULL;	/* Pointer to GMT segment table(s) */
		struct GMT_DATASEGMENT_HIDDEN *SH = NULL;

		if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
			Return (API->error);
		}
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}
		if (D->n_records && D->n_columns < 3) {
			GMT_Report (API, GMT_MSG_ERROR, "Input data have %d column(s) but at least 3 are needed\n", (int)D->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}

		if (GMT->common.l.active && S.symbol == GMT_SYMBOL_LINE) {
			if (polygon) {	/* Place a rectangle in the legend */
				int symbol = S.symbol;
				S.symbol = PSL_RECT;
				gmt_add_legend_item (API, &S, Ctrl->G.active, &(Ctrl->G.fill), Ctrl->W.active, &(Ctrl->W.pen), &(GMT->common.l.item));
				S.symbol = symbol;
			}
			else	/* For specified line, width, color we can do an auto-legend entry under modern mode */
				gmt_add_legend_item (API, &S, false, NULL, Ctrl->W.active, &(Ctrl->W.pen), &(GMT->common.l.item));
		}

		for (tbl = 0; tbl < D->n_tables; tbl++) {
			if (D->table[tbl]->n_headers && S.G.label_type == GMT_LABEL_IS_HEADER) gmt_extract_label (GMT, &D->table[tbl]->header[0][1], S.G.label, NULL);	/* Set first header as potential label */

			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */

				L = D->table[tbl]->segment[seg];	/* Set shortcut to current segment */
				if (polygon && gmt_polygon_is_hole (GMT, L)) continue;	/* Holes are handled together with perimeters */

				GMT_Report (API, GMT_MSG_INFORMATION, "Plotting table %" PRIu64 " segment %" PRIu64 "\n", tbl, seg);

				n = (int)L->n_rows;				/* Number of points in this segment */

				/* We had here things like:	x = D->table[tbl]->segment[seg]->data[GMT_X];
				 * but reallocating x below lead to disasters.  */

				SH = gmt_get_DS_hidden (L);
				change = gmt_parse_segment_header (GMT, L->header, P, &fill_active, &current_fill, &default_fill, &outline_active, &current_pen, &default_pen, default_outline, SH->ogr);

				if (P && PH->skip) continue;	/* Chosen CPT indicates skip for this z */

				if (L->header && L->header[0]) {
					PSL_comment (PSL, "Segment header: %s\n", L->header);
					if (gmt_parse_segment_item (GMT, L->header, "-S", s_args)) {	/* Found -S */
						if ((S.symbol == GMT_SYMBOL_QUOTED_LINE && s_args[0] == 'q') || (S.symbol == GMT_SYMBOL_FRONT && s_args[0] == 'f')) { /* Update parameters */
							gmt_parse_symbol_option (GMT, s_args, &S, 0, false);
							if (change & 1) change -= 1;	/* Don't want polygon to be true later for these symbols */
						}
						else if (S.symbol == GMT_SYMBOL_QUOTED_LINE || S.symbol == GMT_SYMBOL_FRONT)
							GMT_Report (API, GMT_MSG_ERROR, "Segment header tries to switch from -S%c to another symbol (%s) - ignored\n", S.symbol, s_args);
						else	/* Probably just junk -S in header */
							GMT_Report (API, GMT_MSG_INFORMATION, "Segment header contained -S%s - ignored\n", s_args);
					}
				}
				if (S.fq_parse) { /* Did not supply -Sf or -Sq in the segment header */
					if (S.symbol == GMT_SYMBOL_QUOTED_LINE) /* Did not supply -Sf in the segment header */
						GMT_Report (API, GMT_MSG_ERROR, "Segment header did not supply enough parameters for -Sf; skipping this segment\n");
					else
						GMT_Report (API, GMT_MSG_ERROR, "Segment header did not supply enough parameters for -Sq; skipping this segment\n");
					continue;
				}

				if (Ctrl->I.active) {
					gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
					gmt_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
				}
				if (change & 4 && penset_OK) gmt_setpen (GMT, &current_pen);
				if (change & 1) polygon = true;
				if (change & 2 && !Ctrl->L.polygon) {
					polygon = false;
					PSL_setcolor (PSL, current_fill.rgb, PSL_IS_STROKE);
				}
				if (S.G.label_type == GMT_LABEL_IS_HEADER)	/* Get potential label from segment header */
					gmt_extract_label (GMT, L->header, S.G.label, SH->ogr);

				xp = gmt_M_memory (GMT, NULL, n, double);
				yp = gmt_M_memory (GMT, NULL, n, double);

				if (polygon) {
					gmt_plane_perspective (GMT, -1, 0.0);
					for (i = 0; i < n; i++) gmt_geoz_to_xy (GMT, L->data[GMT_X][i], L->data[GMT_Y][i], L->data[GMT_Z][i], &xp[i], &yp[i]);
					gmt_setfill (GMT, &current_fill, outline_active ? 1 : 0);
					PSL_plotpolygon (PSL, xp, yp, (int)n);
				}
				else if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {	/* Labeled lines are dealt with by the contour machinery */
					bool closed;
					/* Note that this always be plotted in the XY-plane */
					gmt_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
					if ((GMT->current.plot.n = gmt_geo_to_xy_line (GMT, L->data[GMT_X], L->data[GMT_Y], L->n_rows)) == 0) continue;
					S.G.line_pen = current_pen;
					closed = !(gmt_polygon_is_open (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n));
					gmt_hold_contour (GMT, &GMT->current.plot.x, &GMT->current.plot.y, GMT->current.plot.n, 0.0, "N/A", 'A', S.G.label_angle, closed, false, &S.G);
					GMT->current.plot.n_alloc = GMT->current.plot.n;	/* Since gmt_hold_contour reallocates to fit the array */
				}
				else {	/* Plot line */
					uint64_t end;
					bool draw_line = true;
					gmt_plane_perspective (GMT, -1, 0.0);
					if (Ctrl->L.anchor) {	/* Build a polygon in one of several ways */
						if (Ctrl->L.anchor == PSXY_POL_SYMM_DEV || Ctrl->L.anchor == PSXY_POL_ASYMM_DEV) {	/* Build envelope around y(x) from delta y values in 1 or 2 extra columns */
							uint64_t k, m, col = (Ctrl->L.anchor == PSXY_POL_ASYMM_DEV) ? 4 : 3;
							end = 2 * L->n_rows + 1;
							gmt_prep_tmp_arrays (GMT, GMT_IN, end, 3);	/* Init or reallocate 3 tmp vectors */
							/* First go in positive x direction and build part of envelope */
							gmt_M_memcpy (GMT->hidden.mem_coord[GMT_X], L->data[GMT_X], L->n_rows, double);
							gmt_M_memcpy (GMT->hidden.mem_coord[GMT_Z], L->data[GMT_Z], L->n_rows, double);
							for (k = 0; k < L->n_rows; k++)
								GMT->hidden.mem_coord[GMT_Y][k] = L->data[GMT_Y][k] - fabs (L->data[3][k]);
							/* Then go in negative x direction and build rest of envelope */
							for (k = m = L->n_rows; k > 0; k--, m++) {
								GMT->hidden.mem_coord[GMT_X][m] = L->data[GMT_X][k-1];
								GMT->hidden.mem_coord[GMT_Z][m] = L->data[GMT_Z][k-1];
								GMT->hidden.mem_coord[GMT_Y][m] = L->data[GMT_Y][k-1] + fabs (L->data[col][k-1]);
							}
							/* Explicitly close polygon */
							GMT->hidden.mem_coord[GMT_X][end-1] = GMT->hidden.mem_coord[GMT_X][0];
							GMT->hidden.mem_coord[GMT_Y][end-1] = GMT->hidden.mem_coord[GMT_Y][0];
							GMT->hidden.mem_coord[GMT_Z][end-1] = GMT->hidden.mem_coord[GMT_Z][0];
						}
						else if (Ctrl->L.anchor == PSXY_POL_ASYMM_ENV) {	/* Build envelope around y(x) from low and high 2 extra columns */
							uint64_t k, m;
							end = 2 * L->n_rows + 1;
							gmt_prep_tmp_arrays (GMT, GMT_IN, end, 3);	/* Init or reallocate 3 tmp vectors */
							/* First go in positive x direction and build part of envelope */
							gmt_M_memcpy (GMT->hidden.mem_coord[GMT_X], L->data[GMT_X], L->n_rows, double);
							gmt_M_memcpy (GMT->hidden.mem_coord[GMT_Z], L->data[GMT_X], L->n_rows, double);
							for (k = 0; k < L->n_rows; k++)
								GMT->hidden.mem_coord[GMT_Y][k] = L->data[3][k];
							/* Then go in negative x direction and build rest of envelope */
							for (k = m = L->n_rows; k > 0; k--, m++) {
								GMT->hidden.mem_coord[GMT_X][m] = L->data[GMT_X][k-1];
								GMT->hidden.mem_coord[GMT_Z][m] = L->data[GMT_Z][k-1];
								GMT->hidden.mem_coord[GMT_Y][m] = L->data[4][k-1];
							}
							/* Explicitly close polygon */
							GMT->hidden.mem_coord[GMT_X][end-1] = GMT->hidden.mem_coord[GMT_X][0];
							GMT->hidden.mem_coord[GMT_Y][end-1] = GMT->hidden.mem_coord[GMT_Y][0];
							GMT->hidden.mem_coord[GMT_Z][end-1] = GMT->hidden.mem_coord[GMT_Z][0];
						}
						else {	/* First complete polygon via anchor points and paint the area, optionally with outline */
							uint64_t off = 0U;
							double value;
							end = L->n_rows;
							gmt_prep_tmp_arrays (GMT, GMT_IN, end+3, 3);	/* Init or reallocate 3 tmp vectors */
							/* First copy the given line segment */
							gmt_M_memcpy (GMT->hidden.mem_coord[GMT_X], L->data[GMT_X], end, double);
							gmt_M_memcpy (GMT->hidden.mem_coord[GMT_Y], L->data[GMT_Y], end, double);
							gmt_M_memcpy (GMT->hidden.mem_coord[GMT_Z], L->data[GMT_Z], end, double);
							/* Now add 2 anchor points and explicitly close by repeating 1st point */
							switch (Ctrl->L.mode) {
								case XHI:	off = 1;	/* To select the x max entry, then call through */
								case XLO:
								case ZLO:
									value = (Ctrl->L.mode == ZLO) ? Ctrl->L.value : GMT->common.R.wesn[XLO+off];
									GMT->hidden.mem_coord[GMT_X][end] = GMT->hidden.mem_coord[GMT_X][end+1] = value;
									GMT->hidden.mem_coord[GMT_Z][end] = GMT->hidden.mem_coord[GMT_Z][end+1] = L->data[GMT_Z][0];
									GMT->hidden.mem_coord[GMT_Y][end] = L->data[GMT_Y][end-1];
									GMT->hidden.mem_coord[GMT_Y][end+1] = L->data[GMT_Y][0];
									break;
								case YHI:	off = 1;	/* To select the y max entry, then fall through */
								case YLO:
								case ZHI:
									value = (Ctrl->L.mode == ZHI) ? Ctrl->L.value : GMT->common.R.wesn[YLO+off];
									GMT->hidden.mem_coord[GMT_Y][end] = GMT->hidden.mem_coord[GMT_Y][end+1] = value;
									GMT->hidden.mem_coord[GMT_Z][end] = GMT->hidden.mem_coord[GMT_Z][end+1] = L->data[GMT_Z][0];
									GMT->hidden.mem_coord[GMT_X][end] = L->data[GMT_X][end-1];
									GMT->hidden.mem_coord[GMT_X][end+1] = L->data[GMT_X][0];
									break;
							}
							/* Explicitly close polygon */
							GMT->hidden.mem_coord[GMT_X][end+2] = L->data[GMT_X][0];
							GMT->hidden.mem_coord[GMT_Y][end+2] = L->data[GMT_Y][0];
							GMT->hidden.mem_coord[GMT_Z][end+2] = L->data[GMT_Z][0];
							end += 3;
						}
						/* Project and get ready */
						xp = gmt_M_memory (GMT, xp, end, double);	/* Extend these arrays */
						yp = gmt_M_memory (GMT, yp, end, double);
						for (i = 0; i < end; i++)
							gmt_geoz_to_xy (GMT, GMT->hidden.mem_coord[GMT_X][i], GMT->hidden.mem_coord[GMT_Y][i], GMT->hidden.mem_coord[GMT_Z][i], &xp[i], &yp[i]);
						if (Ctrl->L.outline) gmt_setpen (GMT, &Ctrl->L.pen);	/* Select separate pen for polygon outline */
						if (Ctrl->G.active)	/* Specify the fill, possibly set outline */
							gmt_setfill (GMT, &current_fill, Ctrl->L.outline);
						else	/* No fill, just outline */
							gmt_setfill (GMT, NULL, Ctrl->L.outline);
						PSL_plotpolygon (PSL, xp, yp, (int)end);
						if (!Ctrl->W.active) draw_line = false;	/* Did not want to actually draw the main line */
						if (Ctrl->L.outline) gmt_setpen (GMT, &current_pen);	/* Reset the pen to what -W indicates */
					}
					else {
						for (i = 0; i < n; i++) gmt_geoz_to_xy (GMT, L->data[GMT_X][i], L->data[GMT_Y][i], L->data[GMT_Z][i], &xp[i], &yp[i]);
					}
					if (draw_line) {
						PSL_plotline (PSL, xp, yp, (int)n, PSL_MOVE|PSL_STROKE);
					}
				}
				if (S.symbol == GMT_SYMBOL_FRONT) { /* Must draw fault crossbars */
					gmt_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
					if ((GMT->current.plot.n = gmt_geo_to_xy_line (GMT, L->data[GMT_X], L->data[GMT_Y], L->n_rows)) == 0) continue;
					gmt_setfill (GMT, &current_fill, (S.f.f_pen == -1) ? 0 : 1);
					gmt_draw_front (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, &S.f);
					if (S.f.f_pen == 0) gmt_setpen (GMT, &current_pen);	/* Reinstate current pen */
				}

				gmt_M_free (GMT, xp);
				gmt_M_free (GMT, yp);
			}
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	if (S.u_set) GMT->current.setting.proj_length_unit = save_u;	/* Reset unit */

	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		if (S.G.save_labels) {	/* Want to save the Line label locations (lon, lat, angle, label) */
			if ((error = gmt_contlabel_save_begin (GMT, &S.G)) != 0) Return (error);
			if ((error = gmt_contlabel_save_end (GMT, &S.G)) != 0) Return (error);
		}
		gmt_contlabel_plot (GMT, &S.G);
	}

	if (clip_set && !S.G.delay) gmt_map_clip_off (GMT);	/* We delay map clip off if text clipping was chosen via -Sq<args:+e */

	gmt_plane_perspective (GMT, -1, 0.0);

	if (Ctrl->D.active) PSL_setorigin (PSL, -DX, -DY, 0.0, PSL_FWD);	/* Shift plot a bit */

	PSL_setdash (PSL, NULL, 0);
	if (geovector) PSL->current.linewidth = 0.0;	/* Since we changed things under clip; this will force it to be set next */
	gmt_vertical_axis (GMT, 2);	/* Draw foreground axis */
	GMT->current.map.is_world = old_is_world;

	gmt_symbol_free (GMT, &S);

	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
