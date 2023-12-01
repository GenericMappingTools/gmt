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
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: psxyz will read <x,y,z> triplets and plot symbols, lines,
 * or polygons in a 3-D perspective view.
 *
 * Note on KEYS: S?(=2 means if -S~|q then we may possibly take optional crossing line file, else the ? is set to ! for skipping it.
 *               The "2" means we must skip two characters (q|~ and f|x) before finding the dataset file name
 */

#include "gmt_dev.h"
#include "longopt/psxyz_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"psxyz"
#define THIS_MODULE_MODERN_NAME	"plot3d"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot lines, polygons, and symbols in 3-D"
#define THIS_MODULE_KEYS	"<D{,CC(,T-<,S?(=2,ZD(,>X}"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYabdefghilpqtwxy" GMT_OPT("EMmc")

/* Control structure for psxyz */

struct PSXYZ_CTRL {
	struct PSXYZ_A {	/* -A[m|y|p|x|r|t|step] */
		bool active;
		unsigned int mode;
		double step;
	} A;
	struct PSXYZ_C {	/* -C<cpt> or -C<color1>,<color2>[,<color3>,...] */
		bool active;
		char *file;
	} C;
	struct PSXYZ_D {	/* -D<dx>/<dy>[/<dz>] */
		bool active;
		double dx, dy, dz;
	} D;
	struct PSXYZ_G {	/* -G<fill>|+z */
		bool active;
		bool set_color;
		unsigned int sequential;
		struct GMT_FILL fill;
	} G;
	struct PSXYZ_H {	/* -H read overall scaling factor for symbol size and pen width */
		bool active;
		unsigned int mode;
		double value;
	} H;
	struct PSXYZ_I {	/* -I[<intensity>] */
		bool active;
		unsigned int mode;	/* 0 if constant, 1 if read from file (symbols only) */
		double value;
	} I;
	struct PSXYZ_L {	/* -L[+xl|r|x0][+yb|t|y0][+e|E][+p<pen>] */
		bool active;
		bool polygon;		/* true when just -L is given */
		int outline;		/* 1 when +p<pen> is given */
		unsigned int mode;	/* Which side for the anchor */
		unsigned int anchor;	/* 0 not used, 1 = x anchors, 2 = y anchors, 3 = +/-dy, 4 = -dy1, +dy2 */
		double value;
		struct GMT_PEN pen;
	} L;
	struct PSXYZ_N {	/* -N[r|c] */
		bool active;
		unsigned int mode;
	} N;
	struct PSXYZ_Q {	/* -Q */
		bool active;
	} Q;
	struct PSXYZ_S {	/* -S */
		bool active;
		char *arg;
	} S;
	struct PSXYZ_T {	/* -T */
		bool active;
	} T;
	struct PSXYZ_W {	/* -W<pen>[+c[l|f]][+o<offset>][+s][+v[b|e]<size><vecargs>][+z] */
		bool active;
		bool cpt_effect;
		bool set_color;
		unsigned int sequential;
		struct GMT_PEN pen;
	} W;
	struct PSXYZ_Z {	/* -Z<value>[+t|T] */
		bool active;
		unsigned set_transp;
		double value;
		char *file;
	} Z;
};

enum Psxyz_poltype {
	PSXYZ_POL_X 		= 1,
	PSXYZ_POL_Y,
	PSXYZ_POL_SYMM_DEV,
	PSXYZ_POL_ASYMM_DEV,
	PSXYZ_POL_ASYMM_ENV};

enum Psxyz_cliptype {
	PSXYZ_CLIP_REPEAT 	= 0,
	PSXYZ_CLIP_NO_REPEAT,
	PSXYZ_NO_CLIP_REPEAT,
	PSXYZ_NO_CLIP_NO_REPEAT};

enum Psxyz_scaletype {
	PSXYZ_READ_SCALE	= 0,
	PSXYZ_CONST_SCALE	= 1};

struct PSXYZ_DATA {
	int symbol, outline;
	unsigned int flag;	/* 1 = convert azimuth, 2 = use geo-functions, 4 = x-base in units, 8 y-base in units */
	double x, y, z, dim[PSL_MAX_DIMS], dist[2], transparency[2];
	double *zz;	/* For column symbol if +z<n> in effect */
	struct GMT_FILL f;
	struct GMT_PEN p, h;
	struct GMT_VECT_ATTR v;
	char *string;
	struct GMT_CUSTOM_SYMBOL *custom;
};

GMT_LOCAL bool psxyz_is_stroke_symbol (int symbol) {
	/* Return true if cross, x, y, - symbols */
	if (symbol == PSL_CROSS) return true;
	if (symbol == PSL_XDASH) return true;
	if (symbol == PSL_YDASH) return true;
	if (symbol == PSL_PLUS)  return true;
	return false;
}

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSXYZ_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSXYZ_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->W.pen = GMT->current.setting.map_default_pen;
	gmt_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* Default is no fill */
	C->A.step = GMT->current.setting.map_line_step;
	C->N.mode = PSXYZ_CLIP_REPEAT;
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct PSXYZ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->S.arg);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psxyz synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	const char *mod_name = &name[4];	/* To skip the leading gmt for usage messages */
	const char *T[2] = {" [-T]", ""};
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] %s %s [%s] [%s] [-A[m|p|r|t|x|y]] [-C<cpt>] [-D<dx>/<dy>[/<dz>]] [-G<fill>] "
		"[-H[<scale>]] [-I[<intens>]] %s [%s] [-N[c|r]] %s %s[-Q] [-S[<symbol>][<size>][/size_y]]%s [%s] [%s] [-W[<pen>][<attr>]] "
		"[%s] [%s] [-Z<value>|<file>[+t|T]] [%s] [%s] %s[%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_J_OPT, GMT_Rgeoz_OPT, GMT_B_OPT, GMT_Jz_OPT, API->K_OPT, PLOT_L_OPT, API->O_OPT, API->P_OPT,
		T[API->GMT->current.setting.run_mode], GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_a_OPT, GMT_bi_OPT,
		API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_l_OPT, GMT_p_OPT,
		GMT_qi_OPT, GMT_tv_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<,J-Z,R3");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Option (API, "B-");
	GMT_Usage (API, 1, "\n-A[m|p|r|t|x|y]");
	GMT_Usage (API, -2, "Suppress drawing geographic line segments as great circle arcs, i.e., draw "
		"straight lines instead.  Six optional directives instead convert paths to staircase curves:");
	GMT_Usage (API, 3, "m: First follow meridians, then parallels when connecting geographic points.");
	GMT_Usage (API, 3, "p: First follow parallels, then meridians when connecting geographic point.");
	GMT_Usage (API, 3, "r: First follow radius, then theta for staircase curves for Polar projection.");
	GMT_Usage (API, 3, "t: First follow theta, then radius for staircase curves for Polar projection.");
	GMT_Usage (API, 3, "x: First follow x, then y for staircase curves for Cartesian projections.");
	GMT_Usage (API, 3, "y: First follow y, then x for staircase curves for Cartesian projections.");
	GMT_Usage (API, 1, "\n-C<cpt>|<color1>,<color2>[,<color3>,...]");
	GMT_Usage (API, -2, "Assign symbol colors based on z-value in 3rd column. "
		"Note: requires -S. Without -S, %s excepts lines/polygons "
		"and looks for -Z<value> options in each segment header. Then, color is "
		"applied for polygon fill (-L) or polygon pen (no -L).", mod_name);
	GMT_Usage (API, 1, "\n-D<dx>/<dy>.");
	GMT_Usage (API, -2, "Offset symbol or line positions by <dx>/<dy> [no offset].");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify color or pattern [Default is no fill].");
	GMT_Usage (API, -2, "The -G option can be present in all segment headers (not with -S). "
		"To assign fill color via -Z, give -G+z).");
	GMT_Usage (API, 1, "\n-H[<scale>]");
	GMT_Usage (API, -2, "Scale symbol sizes (set via -S or input column) by factors read from scale column. "
		"The scale column follows the symbol size column.  Alternatively, append a fixed <scale>.");
	GMT_Usage (API, 1, "\n-I[<intens>]");
	GMT_Usage (API, -2, "Use the intensity to modulate the fill color (requires -C or -G). "
		"If no intensity is given we expect it to follow symbol size in the data record.");
	GMT_Option (API, "K");
	GMT_Usage (API, 1, "\n%s", PLOT_L_OPT);
	GMT_Usage (API, -2, "Force closed polygons, or append modifiers to build polygon from a line:");
	GMT_Usage (API, 3, "+d Symmetrical envelope around y(x) using deviations dy(x) from col 4.");
	GMT_Usage (API, 3, "+D Asymmetrical envelope around y(x) using deviations dy1(x) and dy2(x) from cols 4-5.");
	GMT_Usage (API, 3, "+b Asymmetrical envelope around y(x) using bounds yl(x) and yh(x) from cols 4-5.");
	GMT_Usage (API, 3, "+x Connect 1st and last point to anchor points at l (xmin), r (xmax), or x0.");
	GMT_Usage (API, 3, "+y Connect 1st and last point to anchor points at b (ymin), t (ymax), or y0.");
	GMT_Usage (API, 3, "+p Draw polygon outline with <pen> [no outline].");
	GMT_Usage (API, -2, "The polygon created may be painted via -G.");
	GMT_Usage (API, 1, "\n-N[c|r]");
	GMT_Usage (API, -2, "Do Not skip or clip symbols that fall outside the map border [clipping is on]:");
	GMT_Usage (API, 3, "r: Turn off clipping and plot repeating symbols for periodic maps.");
	GMT_Usage (API, 3, "c: Retain clipping but turn off plotting of repeating symbols for periodic maps.");
	GMT_Usage (API, -2, "[Default will clip or skip symbols that fall outside and plot repeating symbols].");
	GMT_Usage (API, -2, "Note: May also be used with lines or polygons but no periodicity will be honored.");
	GMT_Option (API, "O,P");
	GMT_Usage (API, 1, "\n-Q Do NOT sort symbols based on distance to viewer before plotting.");
	GMT_Usage (API, 1, "\n-S[<symbol>][<size>]");
	GMT_Usage (API, -2, "Select symbol type and symbol size (in %s).  Choose from these symbols:",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);

	GMT_Usage (API, 2, "\n%s Basic geometric symbol. Append one:", GMT_LINE_BULLET);
	GMT_Usage (API, -3, "-(xdash), +(plus), st(a)r, (b|B)ar, (c)ircle, (d)iamond, (e)llipse, "
		"(f)ront, octa(g)on, (h)exagon (i)nvtriangle, (j)rotated rectangle, "
		"(k)ustom, (l)etter, (m)athangle, pe(n)tagon, c(o)lumn, (p)oint, "
		"(q)uoted line, (r)ectangle, (R)ounded rectangle, (s)quare, (t)riangle, "
		"c(u)be, (v)ector, (w)edge, (x)cross, (y)dash, (z)dash, or "
		"=(geovector, i.e., great or small circle vectors).");

	GMT_Usage (API, -3, "If no size is specified, then the 4th column must have sizes. "
		"If no symbol is specified, then the last column must have symbol codes. "
		"[Note: if -C is selected then 4th means 5th column, etc.]. "
		"Both column and cube are true 3-D objects (give size as xsize/ysize); "
		"all other symbols are shown in 2-D perspective only. "
		"By default, the 3-D symbols column and cube are shaded; "
		"use upper case O and U to disable this 3-D illumination. "
		"Symbols A, C, D, F, H, I, N, S, T are adjusted to have same area "
		"of a circle of given diameter.");

	GMT_Usage (API, 2, "\n%s Bar: Append +b[<base>] to give the y- (or z-) value of the base [Default = 0 (1 for log-scales)]. "
		"Use -SB for horizontal bars; then <base> value refers to the x location.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "+B Heights are measured relative to <base> [relative to origin].");
	GMT_Usage (API, 3, "+b Set <base>. Alternatively, leave <base> off to read it from file.");
	GMT_Usage (API, 3, "+i Increments are given instead or values for multiband bars.");
	GMT_Usage (API, 3, "+s Side-by-side placement of multiband bars [stacked multiband bars]. "
		"Optionally, append <gap> between bars in fraction (or percent) of <size> [no gap].");
	GMT_Usage (API, 3, "+v For multi-band bars, append <nbands>; then <nbands> values will "
		"be read from file instead of just one.");
	GMT_Usage (API, -3, "Multiband bars requires -C with one color per band (values 0, 1, ...). "
		"For -SB the input band values are x (or dx) values instead of y (or dy). ");

	GMT_Usage (API, 2, "\n%s 3-D Column: Append +b[<base>] to give the z-value of the base of the column "
		"[Default = 0 (1 for log-scales)].", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "+B Heights are measured relative to <base> [relative to origin].");
	GMT_Usage (API, 3, "+b Set <base>. Alternatively, leave <base> off to read it from file.");
	GMT_Usage (API, 3, "+v For multi-band columns, append <nbands>; then <nbands> z-values will "
		"be read from file instead of just one.");
	GMT_Usage (API, 3, "+i Increments dz are given instead or values for multiband bars.");
	GMT_Usage (API, -3, "Multiband columns requires -C with one color per band (values 0, 1, ...).");

	GMT_Usage (API, 2, "\n%s 3-D Cube: Give <size> as the length of all sides; append q if <size> "
		"is a quantity in x-units.", GMT_LINE_BULLET);

	GMT_Usage (API, 2, "\n%s Ellipse: If not given, we read direction, major, and minor axis from columns 4-6. "
		"If -SE rather than -Se is selected, %s will expect azimuth, and "
		"axes [in km], and convert azimuths based on map projection. "
		"Use -SE- for a degenerate ellipse (circle) with only diameter in km given. "
		"in column 4, or append a fixed diameter in km to -SE instead. "
		"Append any of the units in %s to the axes, and "
		"if reading dimensions from file, just append the unit [Default is k]. "
		"For a linear projection and -SE we scale the axes by the map scale.", GMT_LINE_BULLET, GMT_LEN_UNITS_DISPLAY, mod_name);

	GMT_Usage (API, 2, "\n%s Rotatable Rectangle: If not given, we read direction, width and height from columns 4-6. "
		"If -SJ rather than -Sj is selected, %s will expect azimuth, and "
		"dimensions [in km] and convert azimuths based on map projection. "
		"Use -SJ- for a degenerate rectangle (square w/no rotation) with only one dimension given "
		"in column 4, or append a fixed dimension to -SJ instead. "
		"Append any of the units in %s to the axes, and "
		"if reading dimensions from file, just append the unit [Default is k]. "
		"For a linear projection and -SJ we scale dimensions by the map scale.", GMT_LINE_BULLET, mod_name, GMT_LEN_UNITS_DISPLAY);

	GMT_Usage (API, 2, "\n%s Front: -Sf<spacing>[/<ticklen>][+r+l][+f+t+s+c+b][+o<offset>][+p<pen>]", GMT_LINE_BULLET);
	GMT_Usage (API, -3, "If <spacing> is negative it means the number of gaps instead. "
		"If <spacing> has a leading + then <spacing> is used exactly [adjusted to fit line length]. "
		"If not given, <ticklen> defaults to 15%% of <spacing>.  Append various modifiers:");
	GMT_Usage (API, 3, "+l Plot symbol to the left of the front [centered].");
	GMT_Usage (API, 3, "+r Plot symbol to the right of the front [centered].");
	GMT_Usage (API, 3, "+i Make main front line invisible [drawn using pen settings from -W].");
	GMT_Usage (API, 3, "+b Plot square when centered, half-square otherwise.");
	GMT_Usage (API, 3, "+c Plot full circle when centered, half-circle otherwise.");
	GMT_Usage (API, 3, "+f Plot centered cross-tick or tick only in specified direction [Default].");
	GMT_Usage (API, 3, "+s Plot left-or right-lateral strike-slip arrows. Optionally append the arrow angle [20].");
	GMT_Usage (API, 3, "+S Same as +s but with curved arrow-heads.");
	GMT_Usage (API, 3, "+t diagonal square when centered, directed triangle otherwise.");
	GMT_Usage (API, 3, "+o Plot first symbol when along-front distance is <offset> [0].");
	GMT_Usage (API, 3, "+p Append <pen> for front symbol outline; if no <pen> then no outline [Outline with -W pen].");
	GMT_Usage (API, -3, "Only one of +b|c|f|i|s|S|t may be selected.");

	GMT_Usage (API, 2, "\n%s Kustom: -Sk|K<symbolname>[/<size>]", GMT_LINE_BULLET);
	GMT_Usage (API, -3, "Append <symbolname> immediately after 'k|K'; this will look for "
		"<symbolname>.def or <symbolname>.eps in the current directory, in $GMT_USERDIR, "
		"or in $GMT_SHAREDIR (searched in that order). Give full path if located elsewhere. "
		"Use upper case 'K' if your custom symbol refers a variable symbol, ?.");
	gmt_list_custom_symbols (API->GMT);

	GMT_Usage (API, 2, "\n%s Letter: -Sl[<size>]+t<string>[a|A<angle>][+f<font>][+j<justify]", GMT_LINE_BULLET);
	GMT_Usage (API, -3, "Specify <size> of letter; append required and optional modifiers:");
	GMT_Usage (API, 3, "+t Specify <string> to use (required).");
	GMT_Usage (API, 3, "+a Set text angle relative to horizontal [0].  Use +A if map azimuth.");
	GMT_Usage (API, 3, "+f Set specific <font> for text placement [FONT_ANNOT_PRIMARY].");
	GMT_Usage (API, 3, "+j Change the text justification via <justify> [CM].");

	GMT_Usage (API, 2, "\n%s Mathangle: radius, start, and stop directions of math angle must be in columns 4-5. "
		"If -SM rather than -Sm is used, we draw straight angle symbol if 90 degrees.", GMT_LINE_BULLET);
	gmt_vector_syntax (API->GMT, 0, 3);

	GMT_Usage (API, 2, "\n%s Quoted line (z must be constant): -Sq[d|n|l|s|x]<info>[:<labelinfo>]" ,GMT_LINE_BULLET);
	GMT_Usage (API, -3, "The <code><info> settings control placement of labels along lines.  Select from these codes:");
	gmt_cont_syntax (API->GMT, 3, 1);
	GMT_Usage (API, 3, "<labelinfo> controls the label attributes.  Choose from these choices:");
	gmt_label_syntax (API->GMT, 2, 1);

	GMT_Usage (API, 2, "\n%s Rectangle: If not given, the x- and y-dimensions must be in columns 4-5. "
		"Append +s if instead the diagonal corner coordinates are given in columns 4-5.", GMT_LINE_BULLET);

	GMT_Usage (API, 2, "\n%s Rounded rectangle: If not given, the x- and y-dimensions and corner radius must be in columns 4-6.", GMT_LINE_BULLET);

	GMT_Usage (API, 2, "\n%s Vector: -Sv|V<size>[+a<angle>][+b][+e][+h<shape>][+j<just>][+l][+m][+n[<norm>[/<min>]]][+o<lon>/<lat>][+q][+r][+s][+t[b|e]<trim>][+z]", GMT_LINE_BULLET);
	GMT_Usage (API, -3, "Direction and length must be in columns 4-5. "
		"If -SV rather than -Sv is selected, %s will expect azimuth and "
		"length and convert azimuths based on the chosen map projection.", mod_name);
	gmt_vector_syntax (API->GMT, 19, 3);

	GMT_Usage (API, 2, "\n%s Wedge: -Sw|W[<outerdiameter>[/<startdir>/<stopdir>]][+a[<dr>][+i<inner_diameter>][+r[<da>]]]", GMT_LINE_BULLET);
	GMT_Usage (API, -3, "Append [<outerdiameter>[<startdir><stopdir>]] or we read these parameters from file from column 4. "
		"If -SW rather than -Sw is selected, specify two azimuths instead of directions. "
		"-SW: Specify <outerdiameter><unit> with units either from %s or %s [Default is k]. "
		"-Sw: Specify <outerdiameter><unit> with units from %s [Default is %s].", GMT_LEN_UNITS_DISPLAY, GMT_DIM_UNITS_DISPLAY, GMT_DIM_UNITS_DISPLAY,
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Usage (API, 3, "+a Just draw arc(s), optionally specify <dr> increment [wedge].");
	GMT_Usage (API, 3, "+i Append nonzero <innerdiameter>; we read it from file if not appended.");
	GMT_Usage (API, 3, "+r Just draw radial lines, optionally specify <da> increment [wedge].");

	GMT_Usage (API, 2, "\n%s Geovector: -S=<size>[+a<angle>][+b][+e][+h<shape>][+j<just>][+l][+m][+n[<norm>[/<min>]]][+o<lon>/<lat>][+q][+r][+s][+t[b|e]<trim>][+z]", GMT_LINE_BULLET);
	GMT_Usage (API, -3, "Azimuth and length must be in columns 4-5. "
		"Append any of the units in %s to length [k].", GMT_LEN_UNITS_DISPLAY);
	gmt_vector_syntax (API->GMT, 3+32, 3);

	if (API->GMT->current.setting.run_mode == GMT_CLASSIC)	/* -T has no purpose in modern mode */
		GMT_Usage (API, 1, "\n-T Ignore all input files.");
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

GMT_LOCAL unsigned int gmt_get_z_bands (struct GMTAPI_CTRL *API, struct PSXYZ_CTRL *Ctrl, char *text) {
	unsigned int j = 0, n_errors = 0;
	if (text[j] == '-') {Ctrl->W.pen.cptmode = 1; j++;}
	if (text[j] == '+') {Ctrl->W.pen.cptmode = 3; j++;}
	if (text[j] && gmt_getpen (API->GMT, &text[j], &Ctrl->W.pen)) {
		gmt_pen_syntax (API->GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", NULL, 3);
		GMT_Usage (API, 3, "+c Append l to apply cpt color (-C) to the pen only.");
		GMT_Usage (API, 3, "+c Append f to apply cpt color (-C) to symbol fill.");
		GMT_Usage (API, -2, "Give +c with no arguments for both effects [none].");
		n_errors++;
	}
	return n_errors;
}

static int parse (struct GMT_CTRL *GMT, struct PSXYZ_CTRL *Ctrl, struct GMT_OPTION *options, struct GMT_SYMBOL *S) {
	/* This parses the options provided to psxyz and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, ztype;
	int n;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files after checking they exist */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Turn off draw_arc mode */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				switch (opt->arg[0]) {
					case 'm': case 'y': case 'r': Ctrl->A.mode = GMT_STAIRS_Y; break;
					case 'p': case 'x': case 't': Ctrl->A.mode = GMT_STAIRS_X; break;

#ifdef DEBUG
					default: Ctrl->A.step = atof (opt->arg); break; /* Undocumented test feature */
#endif
				}
				break;
			case 'C':	/* Vary symbol color with z */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				if ((n = sscanf (opt->arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c)) < 2) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -D: Give x and y [and z] offsets\n");
					n_errors++;
				}
				else {
					Ctrl->D.dx = gmt_M_to_inch (GMT, txt_a);
					Ctrl->D.dy = gmt_M_to_inch (GMT, txt_b);
					if (n == 3) Ctrl->D.dz = gmt_M_to_inch (GMT, txt_c);
				}
				break;
			case 'G':		/* Set color for symbol or polygon */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				if (strncmp (opt->arg, "+z", 2U) == 0)
					Ctrl->G.set_color = true;
				else if (!opt->arg[0] || gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				if (Ctrl->G.fill.rgb[0] < -4.0) Ctrl->G.sequential = irint (Ctrl->G.fill.rgb[0] + 7.0);
				break;
			case 'H':		/* Overall symbol/pen scale column provided */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->H.active);
				if (opt->arg[0]) {	/* Gave a fixed scale - no reading from file */
					Ctrl->H.value = atof (opt->arg);
					Ctrl->H.mode = PSXYZ_CONST_SCALE;
				}
				break;
			case 'I':	/* Adjust symbol color via intensity */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				if (opt->arg[0])
					Ctrl->I.value = atof (opt->arg);
				else
					Ctrl->I.mode = 1;
				break;
			case 'L':		/* Force closed polygons */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				if ((c = strstr (opt->arg, "+b")) != NULL)	/* Build asymmetric polygon from lower and upper bounds */
					Ctrl->L.anchor = PSXYZ_POL_ASYMM_ENV;
				else if ((c = strstr (opt->arg, "+d")) != NULL)	/* Build symmetric polygon from deviations about y(x) */
					Ctrl->L.anchor = PSXYZ_POL_SYMM_DEV;
				else if ((c = strstr (opt->arg, "+D")) != NULL)	/* Build asymmetric polygon from deviations about y(x) */
					Ctrl->L.anchor = PSXYZ_POL_ASYMM_DEV;
				else if ((c = strstr (opt->arg, "+x")) != NULL) {	/* Parse x anchors for a polygon */
					switch (c[2]) {
						case 'l':	Ctrl->L.mode = XLO;	break;	/* Left side anchors */
						case 'r':	Ctrl->L.mode = XHI;	break;	/* Right side anchors */
						default:	Ctrl->L.mode = ZLO;	Ctrl->L.value = atof (&c[2]);	break;	/* Arbitrary x anchor */
					}
					Ctrl->L.anchor = PSXYZ_POL_X;
				}
				else if ((c = strstr (opt->arg, "+y")) != NULL) {	/* Parse y anchors for a polygon */
					switch (c[2]) {
						case 'b':	Ctrl->L.mode = YLO;	break;	/* Bottom side anchors */
						case 't':	Ctrl->L.mode = YHI;	break;	/* Top side anchors */
						default:	Ctrl->L.mode = ZHI;	Ctrl->L.value = atof (&c[2]);	break;	/* Arbitrary y anchor */
					}
					Ctrl->L.anchor = PSXYZ_POL_Y;
				}
				else	/* Just force a closed polygon */
					Ctrl->L.polygon = true;
				if ((c = strstr (opt->arg, "+p")) != NULL) {	/* Want outline */
					if (c[2] && gmt_getpen (GMT, &c[2], &Ctrl->L.pen)) {
						gmt_pen_syntax (GMT, 'W', NULL, "sets pen attributes [no outline]", NULL, 0);
						n_errors++;
					}
					Ctrl->L.outline = 1;
				}
				break;
			case 'N':	/* Do not clip to map */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				if (opt->arg[0] == 'r') Ctrl->N.mode = PSXYZ_NO_CLIP_REPEAT;
				else if (opt->arg[0] == 'c') Ctrl->N.mode = PSXYZ_CLIP_NO_REPEAT;
				else if (opt->arg[0] == '\0') Ctrl->N.mode = PSXYZ_NO_CLIP_NO_REPEAT;
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -N: Unrecognized argument %s\n", opt->arg);
					n_errors++;
				}
				break;
			case 'Q':	/* Do not sort symbols based on distance */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'S':		/* Get symbol [and size] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.arg = strdup (opt->arg);
				break;
			case 'T':		/* Skip all input files */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'W':		/* Set line attributes */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				if ((c = strstr (opt->arg, "+z"))) {
					Ctrl->W.set_color = true;
					c[0] = '\0';	/* Chop off this modifier */
				}
				if (opt->arg[0] == '-' || (opt->arg[0] == '+' && opt->arg[1] != 'c')) {	/* Definitively old-style args */
					if (gmt_M_compat_check (API->GMT, 5)) {	/* Sorry */
						GMT_Report (API, GMT_MSG_ERROR, "Your -W syntax is obsolete; see program usage.\n");
						n_errors++;
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Your -W syntax is obsolete; see program usage.\n");
						n_errors += gmt_get_z_bands (API, Ctrl, opt->arg);
					}
				}
				else if (opt->arg[0] && gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", NULL, 11);
					n_errors++;
				}
				if (Ctrl->W.pen.cptmode) Ctrl->W.cpt_effect = true;
				if (c) c[0] = '+';	/* Restore */
				if (Ctrl->W.pen.rgb[0] < -4.0) Ctrl->W.sequential = irint (Ctrl->W.pen.rgb[0] + 7.0);
				break;

			case 'Z':		/* Get value for CPT lookup */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				if ((c = strstr (opt->arg, "+t"))) {
					Ctrl->Z.set_transp = 1;
					c[0] = '\0';	/* Chop off this modifier */
				}
				else if ((c = strstr (opt->arg, "+T"))) {
					Ctrl->Z.set_transp = 2;
					c[0] = '\0';	/* Chop off this modifier */
				}
				if (gmt_not_numeric (GMT, opt->arg) && !gmt_access (GMT, opt->arg, R_OK)) {	/* Got a file */
					Ctrl->Z.file = strdup (opt->arg);
					n_errors += gmt_M_check_condition (GMT, Ctrl->Z.file && gmt_access (GMT, Ctrl->Z.file, R_OK),
					                                   "Option -Z: Cannot read file %s!\n", Ctrl->Z.file);
				}
				else {	/* Got a value */
					ztype = (strchr (opt->arg, 'T')) ? GMT_IS_ABSTIME : gmt_M_type (GMT, GMT_IN, GMT_Z);
					n_errors += gmt_verify_expectations (GMT, ztype, gmt_scanf_arg (GMT, opt->arg, ztype, false, &Ctrl->Z.value), opt->arg);
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	if (Ctrl->T.active) GMT_Report (API, GMT_MSG_WARNING, "Option -T ignores all input files\n");

	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->Z.set_transp != 1 && !Ctrl->C.active, "Option -Z: No CPT given via -C\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && (Ctrl->C.file == NULL || Ctrl->C.file[0] == '\0'), "Option -C: No CPT given\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && gmt_parse_symbol_option (GMT, Ctrl->S.arg, S, 1, true), "Option -S: Parsing failure\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && S->symbol == GMT_SYMBOL_NOT_SET, "Binary input data cannot have symbol information\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.pen.cptmode && !Ctrl->C.active, "Option -W modifier +c requires the -C option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.anchor && (!Ctrl->G.active && !Ctrl->Z.active) && !Ctrl->L.outline, "Option -L<modifiers> must include +p<pen> if -G not given\n");
	if (Ctrl->S.active && gmt_is_barcolumn (GMT, S)) {
		n = gmt_get_columbar_bands (GMT, S);
		n_errors += gmt_M_check_condition (GMT, n > 1 && !Ctrl->C.active, "Options -Sb|B|o|O with multiple layers require -C\n");
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void psxyz_column3D (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double z, double *dim, double rgb[3][4], int outline) {
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
				/* Intentionally fall through - after flipping the sign */
			case 1:	/* negative side */
				gmt_plane_perspective (GMT, GMT_X, x + sign * x_size);
				PSL_plotbox (PSL, y - y_size, z - z_size, y + y_size, z + z_size);
				break;
			case 2:	/* xz plane positive side */
				sign = 1.0;
				/* Intentionally fall through - after flipping the sign */
			case 3:	/* negative side */
				gmt_plane_perspective (GMT, GMT_Y, y + sign * y_size);
				PSL_plotbox (PSL, x - x_size, z - z_size, x + x_size, z + z_size);
				break;
			case 4:	/* xy plane positive side */
				sign = 1.0;
				/* Intentionally fall through - after flipping the sign */
			case 5:	/* negative side */
				gmt_plane_perspective (GMT, GMT_Z, z + sign * z_size);
				PSL_plotbox (PSL, x - x_size, y - y_size, x + x_size, y + y_size);
				break;
		}
	}
}

GMT_LOCAL int psxyz_dist_compare (const void *a, const void *b) {
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

GMT_LOCAL bool psxyz_no_z_variation (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *L) {
	/* Determine if we are on a constant z-level plane */
	unsigned int row;
	gmt_M_unused (GMT);
	for (row = 1; row < L->n_rows; row++) {
		if (!doubleAlmostEqualZero (L->data[GMT_Z][row], L->data[GMT_Z][row-1])) return false;
	}
	return (true);
}

GMT_LOCAL bool psxyz_load_bands (struct GMT_CTRL *GMT, double *in, double *out, unsigned int n, uint64_t rec, struct GMT_SYMBOL *S) {
	/* Accumulate and project to projected values */
	unsigned int k, kk, col;
	double xx, yy;

	for (col = GMT_Z, k = 0; k < n; k++, col++ ) {	/* Place the x, y, z OR dx, dy, or dz increments in the out array */
		if (S->symbol == GMT_SYMBOL_BARX)	/* Must pick first x, then what follows z */
			kk = (k == 0) ? GMT_X : col;
		else if (S->symbol == GMT_SYMBOL_BARY)	/* Must pick first y, then what follows z */
			kk = (k == 0) ? GMT_Y : col;
		else	/* just pick z columns */
			kk = col;
		out[k] = in[kk];
	}
	if (S->accumulate) {	/* Do the accumulation of increments into absolute values */
		for (k = 1; k < n; k++)
			out[k] += out[k-1];
	}
	if (S->base_set & GMT_BASE_ORIGIN) {	/* Add the new origin offset */
		for (k = 0; k < n; k++)
			out[k] += S->base;
	}
	/* Now we have final x1, x2, x3... or y1, y2, y3..., or z1, z2, z3... in the data array in user units */

	for (k = 1; k < n; k++) {	/* Check things are monotonically increasing for single bars or columns */
		if (!S->sidebyside && out[k] < out[k-1]) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "The -Sb|B|o|O options require monotonically increasing band-values - not true near line %d\n", rec);
			return true;
		}
	}
	/* Now project the data values to projected units */
	for (k = 0; k < n; k++) {
		if (S->symbol == GMT_SYMBOL_BARX) {	/* Project and keep the projected x values */
			gmt_geo_to_xy (GMT, out[k], in[GMT_Y], &xx, &yy);
			out[k] = xx;
		}
		else if (S->symbol == GMT_SYMBOL_BARY) {	/* Project and keep the projected y values */
			gmt_geo_to_xy (GMT, in[GMT_X], out[k], &xx, &yy);
			out[k] = yy;
		}
		else	/* Just get the projected z values */
			out[k] = gmt_z_to_zz (GMT, out[k]);
	}
	return false;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_psxyz (void *V_API, int mode, void *args) {
	/* High-level function that implements the psxyz task */
	bool polygon = false, penset_OK = true, not_line = false, old_is_world = false, seq_legend = false, use_z_table = false;
	bool get_rgb = false, read_symbol = false, clip_set = false, fill_active = false, rgb_from_z = false, QR_symbol = false;
	bool default_outline = false, outline_active = false, save_u = false, geovector = false, can_update_headpen = true;
	unsigned int k, j, geometry, tbl, pos2x, pos2y, xcol = 0, icol = 0, tcol_f = 0, tcol_s = 0, grid_order, frame_order, n_z = 0;
	unsigned int n_cols_start = 3, justify, v4_outline = 0, v4_status = 0, bcol, ex1, ex2, ex3, change = 0, n_needed = 0;
	int error = GMT_NOERROR, seq_n_legends = 0, seq_frequency = 0;
	uint64_t i, n, n_total_read = 0, n_z_for_cpt = 0;
	size_t n_alloc = 0;

	char s_args[GMT_BUFSIZ] = {""};

	double dim[PSL_MAX_DIMS], rgb[3][4] = {{-1.0, -1.0, -1.0, 0.0}, {-1.0, -1.0, -1.0, 0.0}, {-1.0, -1.0, -1.0, 0.0}};
	double DX = 0, DY = 0, *xp = NULL, *yp = NULL, *in = NULL, *v4_rgb = NULL, *z_for_cpt = NULL;
	double lux[3] = {0.0, 0.0, 0.0}, tmp, x_1, x_2, y_1, y_2, dx, dy, s, c, zz, zb, length, base;
	double bar_gap, bar_width, bar_step, nominal_size_x, nominal_size_y, *t_for_cpt = NULL;
	double axes[2] = {0.0, 0.0}, Az = 0.0, factor = 1.0;

	struct GMT_PEN default_pen, current_pen, last_headpen, last_spiderpen, save_pen;
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

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	/* Initialize GMT_SYMBOL structure */

	gmt_M_memset (&S, 1, struct GMT_SYMBOL);
	gmt_M_memset (&last_headpen, 1, struct GMT_PEN);
	gmt_M_memset (&last_spiderpen, 1, struct GMT_PEN);
	gmt_contlabel_init (GMT, &S.G, 0);

	S.base = GMT->session.d_NaN;
	S.font = GMT->current.setting.font_annot[GMT_PRIMARY];
	S.u = GMT->current.setting.proj_length_unit;
	S.justify = PSL_MC;

	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options, &S)) != 0) Return (error);

	/*---------------------------- This is the psxyz main code ----------------------------*/

	if (!Ctrl->T.active) GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	GMT->current.plot.mode_3D = 1;	/* Only do background axis first; do foreground at end */
	nominal_size_x = S.size_x;
	nominal_size_y = S.size_y;

	/* Do we plot actual symbols, or lines */
	not_line = (S.symbol != GMT_SYMBOL_FRONT && S.symbol != GMT_SYMBOL_QUOTED_LINE && S.symbol != GMT_SYMBOL_LINE);

	read_symbol = (S.symbol == GMT_SYMBOL_NOT_SET);
	gmt_init_fill (GMT, &black, 0.0, 0.0, 0.0);	/* Default fill for points, if needed */
	gmt_init_fill (GMT, &no_fill, -1.0, -1.0, -1.0);

	if (Ctrl->C.active || Ctrl->Z.file) {
		if (Ctrl->C.active ) {
			if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
				Return (API->error);
			}
			get_rgb = not_line;	/* Need to assign color from either z or text from input data file */
		}
		if (Ctrl->Z.active) {	/* Get color from cpt -Z and store in -G */
			if (Ctrl->Z.file) {
				/* Must temporarily let the x-column contain datavalues for the CPT lookup */
				struct GMT_DATASET *Zin = NULL;
				enum gmt_col_enum x_col_type = gmt_get_column_type (GMT, GMT_IN, GMT_X);
				enum gmt_col_enum v_col_type = gmt_get_column_type (GMT, GMT_IN, 3);
				gmt_disable_bghio_opts (GMT);	/* Do not want any -b -g -h -i -o to affect the reading from -Z file */
				gmt_set_column_type (GMT, GMT_IN, GMT_X, v_col_type);
				if ((Zin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IO_ASCII, NULL, Ctrl->Z.file, NULL)) == NULL) {
					Return (API->error);
				}
				gmt_set_column_type (GMT, GMT_IN, GMT_X, x_col_type);
				gmt_reenable_bghio_opts (GMT);	/* Recover settings provided by user (if -b -g -h -i were used at all) */
				if (Zin->n_segments > 1) {
					GMT_Report (API, GMT_MSG_ERROR, "The file given via -Z must have a single segment with one z-value for each polygon in the input file\n");
					Return (API->error);
				}
				if (Zin->n_columns == 0) {
					GMT_Report (API, GMT_MSG_ERROR, "The file given via -Z must have a at least one data column and we will choose the last column\n");
					Return (API->error);
				}
				n_z_for_cpt = Zin->table[0]->segment[0]->n_rows;	/* Remember length of segment */
				if (Ctrl->Z.set_transp == 2) {	/* This needs two columns */
					if (Zin->n_columns < 2) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -Z: The file must have two columns (transparency and z-value) when modifier +T is used\n");
						Return (API->error);
					}
				}
				if (Ctrl->Z.set_transp == 2) {	/* Got z and transp */
					t_for_cpt = gmt_M_memory (GMT, NULL, n_z_for_cpt, double);
					z_for_cpt = gmt_M_memory (GMT, NULL, n_z_for_cpt, double);
					gmt_M_memcpy (z_for_cpt, Zin->table[0]->segment[0]->data[Zin->n_columns-1], n_z_for_cpt, double);
					gmt_M_memcpy (t_for_cpt, Zin->table[0]->segment[0]->data[Zin->n_columns-2], n_z_for_cpt, double);
				}
				else if (Ctrl->Z.set_transp == 1) {	/* Got just transp */
					t_for_cpt = gmt_M_memory (GMT, NULL, n_z_for_cpt, double);
					gmt_M_memcpy (t_for_cpt, Zin->table[0]->segment[0]->data[Zin->n_columns-1], n_z_for_cpt, double);
				}
				else {	/* Just z-column */
					z_for_cpt = gmt_M_memory (GMT, NULL, n_z_for_cpt, double);
					gmt_M_memcpy (z_for_cpt, Zin->table[0]->segment[0]->data[Zin->n_columns-1], n_z_for_cpt, double);
				}
				use_z_table = (z_for_cpt || t_for_cpt);
				if (GMT_Destroy_Data (API, &Zin) != GMT_NOERROR) {	/* Finished with this file */
					Return (API->error);
				}
			}
			else {
				double rgb[4];
				(void)gmt_get_rgb_from_z (GMT, P, Ctrl->Z.value, rgb);
				if (Ctrl->W.set_color)	/* To be used in polygon or symbol outline */
					gmt_M_rgb_copy (Ctrl->W.pen.rgb, rgb);
				if (Ctrl->G.set_color)	/* To be used in polygon or symbol fill */
					gmt_M_rgb_copy (Ctrl->G.fill.rgb, rgb);
			}
			get_rgb = false;	/* Not reading z from data */
		}
		else if ((P->categorical & GMT_CPT_CATEGORICAL_KEY))	/* Get rgb from trailing text, so read no extra z columns */
			rgb_from_z = false;
		else if (S.v.status & PSL_VEC_MAGCPT)	/* Get rgb via user data magnitude and handle it per symbol */
			rgb_from_z = false;
		else {	/* Read extra z column for symbols only */
			rgb_from_z = not_line;
			if (rgb_from_z && (P->categorical & GMT_CPT_CATEGORICAL_KEY) == 0) n_cols_start++;
		}
	}

	polygon = (S.symbol == GMT_SYMBOL_LINE && (Ctrl->G.active || Ctrl->L.polygon) && !Ctrl->L.anchor);
	if (Ctrl->W.cpt_effect && Ctrl->W.pen.cptmode & 2) polygon = true;
	if (Ctrl->G.set_color) polygon = true;
	default_pen = current_pen = Ctrl->W.pen;
	current_fill = default_fill = (S.symbol == PSL_DOT && !Ctrl->G.active) ? black : Ctrl->G.fill;
	default_outline = Ctrl->W.active;
	if (Ctrl->I.active && Ctrl->I.mode == 0) {
		gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
		gmt_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
	}

	if (get_rgb && gmt_is_barcolumn (GMT, &S) && (n_z = gmt_get_columbar_bands (GMT, &S)) > 1) get_rgb = rgb_from_z = false;	/* Not used in the same way here */
	if (Ctrl->L.anchor == PSXYZ_POL_SYMM_DEV) n_cols_start += 1;
	if (Ctrl->L.anchor == PSXYZ_POL_ASYMM_DEV || Ctrl->L.anchor == PSXYZ_POL_ASYMM_ENV) n_cols_start += 2;

	/* Extra columns 1, 2, and 3 */
	ex1 = (rgb_from_z) ? 4 : 3;
	ex2 = (rgb_from_z) ? 5 : 4;
	ex3 = (rgb_from_z) ? 6 : 5;
	pos2x = ex1 + GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd longitude (for VECTORS with two sets of coordinates) */
	pos2y = ex2 - GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd latitude (for VECTORS with two sets of coordinates) */
	if (gmt_is_barcolumn (GMT, &S)) {
		n_z = gmt_get_columbar_bands (GMT, &S);	/* > 0 for multiband, else 0 */
		n_needed = n_cols_start + ((n_z > 1) ? n_z - 2 : S.n_required);
	}
	else
		n_needed = n_cols_start + S.n_required;
	if (not_line)
		gmt_set_column_types (GMT, n_cols_start, rgb_from_z, 7, &S);	/* Handle the dimensional vs non-dimensional column types including if -i is used */
	if (Ctrl->H.active && Ctrl->H.mode == PSXYZ_READ_SCALE) {
		xcol = n_needed;
		n_needed++;	/* Read scaling from data file */
		gmt_set_column_type (GMT, GMT_IN, xcol, GMT_IS_FLOAT);
	}
	if (Ctrl->I.mode) {
		icol = n_needed;
		n_needed++;	/* Read intensity from data file */
		gmt_set_column_type (GMT, GMT_IN, icol, GMT_IS_FLOAT);
	}
	if (GMT->common.t.variable) {	/* Need one or two transparencies from file */
		if (GMT->common.t.mode & GMT_SET_FILL_TRANSP) {
			tcol_f = n_needed;
			n_needed++;	/* Read fill transparencies from data file */
			gmt_set_column_type (GMT, GMT_IN, tcol_f, GMT_IS_FLOAT);
		}
		if (GMT->common.t.mode & GMT_SET_PEN_TRANSP) {
			tcol_s = n_needed;
			n_needed++;	/* Read stroke transparencies from data file */
			gmt_set_column_type (GMT, GMT_IN, tcol_s, GMT_IS_FLOAT);
		}
	}

	if (gmt_check_binary_io (GMT, n_needed))
		Return (GMT_RUNTIME_ERROR);

	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		if (gmt_contlabel_prep (GMT, &S.G, NULL))
			Return (GMT_RUNTIME_ERROR);
		penset_OK = false;	/* Since it is set in PSL */
	}

	if (gmt_map_setup (GMT, GMT->common.R.wesn))
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

	if (GMT->current.proj.z_pars[0] == 0.0) {	/* Only consider clipping if there is no z scaling */
		if ((gmt_M_is_conical(GMT) && gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]))) {	/* Must turn clipping on for 360-range conical */
			/* Special case of 360-range conical (which is periodic but do not touch at w=e) so we must clip to ensure nothing is plotted in the gap between west and east border */
			clip_set = true;
			frame_order = (Ctrl->N.active) ? GMT_BASEMAP_FRAME_BEFORE : GMT_BASEMAP_FRAME_AFTER;
		}
		else if (Ctrl->N.mode == PSXYZ_CLIP_REPEAT || Ctrl->N.mode == PSXYZ_CLIP_NO_REPEAT) {	/* Only set clip if plotting symbols and -N not used */
			clip_set = true;
			frame_order = GMT_BASEMAP_FRAME_AFTER;
		}
		else
			frame_order = (Ctrl->N.active) ? GMT_BASEMAP_FRAME_BEFORE : GMT_BASEMAP_FRAME_AFTER;
	}
	else
		frame_order = GMT_BASEMAP_FRAME_BEFORE;

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	if (Ctrl->T.active) {
		gmt_plotend (GMT);
		Return (GMT_NOERROR);
	}

	gmt_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
	grid_order = (polygon) ? GMT_BASEMAP_GRID_AFTER : GMT_BASEMAP_GRID_BEFORE;
	gmt_set_basemap_orders (GMT, frame_order, grid_order, GMT_BASEMAP_ANNOT_BEFORE);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
 	gmt_map_basemap (GMT);	/* Lay down gridlines */

	gmt_set_line_resampling (GMT, Ctrl->A.active, Ctrl->A.mode);	/* Possibly change line resampling mode */
#ifdef DEBUG
	/* Change default step size (in degrees) used for interpolation of line segments along great circles (if requested) */
	if (Ctrl->A.active) Ctrl->A.step = Ctrl->A.step / GMT->current.proj.scale[GMT_X] / GMT->current.proj.M_PR_DEG;
#endif

	if (clip_set) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);
	gmt_plane_perspective (GMT, -1, 0.0);

	if (S.symbol == GMT_SYMBOL_TEXT && Ctrl->G.active && !Ctrl->W.active) PSL_setcolor (PSL, current_fill.rgb, PSL_IS_FILL);
	if (S.symbol == GMT_SYMBOL_TEXT) gmt_setfont (GMT, &S.font);		/* Set the required font */
	if ((S.symbol == PSL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR) && S.v.status & PSL_VEC_JUST_S) {
		/* Reading 2nd coordinate so must set column types */
		gmt_set_column_type (GMT, GMT_IN, pos2x, gmt_M_type (GMT, GMT_IN, GMT_X));
		gmt_set_column_type (GMT, GMT_IN, pos2y, gmt_M_type (GMT, GMT_IN, GMT_Y));
	}
	if (S.v.status & PSL_VEC_COMPONENTS) {	/* Giving vector components */
		unsigned int type = (S.symbol == GMT_SYMBOL_GEOVECTOR) ? GMT_IS_GEODIMENSION : GMT_IS_DIMENSION;
		if (S.v.v_norm_d || S.v.v_unit_d) type = GMT_IS_FLOAT;	/* Read user units */
		gmt_set_column_type (GMT, GMT_IN, pos2x, type);	/* Just the users dx component, not azimuth */
		gmt_set_column_type (GMT, GMT_IN, pos2y, type);	/* Just the users dy component, not length */
	}
	else if (S.v.status & PSL_VEC_MAGNIFY)
		gmt_set_column_type (GMT, GMT_IN, pos2y, GMT_IS_FLOAT);	/* Read user units */
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
	if (S.symbol == GMT_SYMBOL_BARX && (S.base_set & GMT_BASE_READ)) gmt_set_column_type (GMT, GMT_IN, bcol, gmt_M_type (GMT, GMT_IN, GMT_Y));
	if (S.symbol == GMT_SYMBOL_BARY && (S.base_set & GMT_BASE_READ)) gmt_set_column_type (GMT, GMT_IN, bcol, gmt_M_type (GMT, GMT_IN, GMT_Y));
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

	PSL_command (GMT->PSL, "V\n");	/* Place all symbols or lines under a gsave/grestore clause */
	if (penset_OK) gmt_setpen (GMT, &current_pen);

	if (not_line) {	/* symbol part (not counting GMT_SYMBOL_FRONT and GMT_SYMBOL_QUOTED_LINE) */
		bool periodic = false, delayed_unit_scaling[2] = {false, false};
		unsigned int n_warn[3] = {0, 0, 0}, warn, item, n_times, last_time, col;
		double xpos[2], width, d, data_magnitude, direction;
		struct GMT_RECORD *In = NULL;

		if ((error = GMT_Set_Columns (API, GMT_IN, n_needed, GMT_COL_FIX)) != GMT_NOERROR) {
			Return (error);
		}
		/* Determine if we need to worry about repeating periodic symbols */
		if (clip_set && (Ctrl->N.mode == PSXYZ_CLIP_REPEAT || Ctrl->N.mode == PSXYZ_NO_CLIP_REPEAT) && gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]) && gmt_M_x_is_lon (GMT, GMT_IN)) {
			/* Only do this for projection where west and east are split into two separate repeating boundaries */
			periodic = gmt_M_is_periodic (GMT);
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
		if (S.symbol == GMT_SYMBOL_GEOVECTOR && (S.v.status & PSL_VEC_JUST_S) == 0 && !(S.v.v_norm_d || S.v.v_unit_d)) {	/* Input is either azim,length or just length for small circle vectors */
			if (S.v.status & PSL_VEC_POLE) {	/* Small circle distance is either map length or start,stop angles */
				if ((S.v.status & PSL_VEC_ANGLES) == 0)	/* Just map length */
					gmt_set_column_type (GMT, GMT_IN, ex1, GMT_IS_GEODIMENSION);
			}
			else	/* Great circle map length */
				gmt_set_column_type (GMT, GMT_IN, ex2, GMT_IS_GEODIMENSION);
		}
		else if ((S.symbol == PSL_ELLIPSE || S.symbol == PSL_ROTRECT) && S.convert_angles && !S.par_set) {
			if (S.n_required == 1)  {
				gmt_set_column_type (GMT, GMT_IN, ex1, GMT_IS_GEODIMENSION);
			}
			else {
				gmt_set_column_type (GMT, GMT_IN, ex2, GMT_IS_GEODIMENSION);
				gmt_set_column_type (GMT, GMT_IN, ex3, GMT_IS_GEODIMENSION);
			}
		}
		else if (S.symbol == PSL_WEDGE) {
			if (S.w_get_do && S.w_active) gmt_set_column_type (GMT, GMT_IN, ex1, GMT_IS_GEODIMENSION);
			if (S.w_get_di && S.w_active) {
				unsigned int col = ex1 + 1;
				if (S.w_get_a) col += 2;
				gmt_set_column_type (GMT, GMT_IN, col, GMT_IS_GEODIMENSION);
			}
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
			gmt_set_column_type (GMT, GMT_IN, ex1, GMT_IS_FLOAT);
			if (S.u_set)
				delayed_unit_scaling[GMT_X] = (S.u != GMT_INCH);
			else if (GMT->current.setting.proj_length_unit != GMT_INCH) {
				delayed_unit_scaling[GMT_X] = true;
				S.u = GMT->current.setting.proj_length_unit;
			}
		}
		if (S.read_size && GMT->current.io.col[GMT_IN][ex2].convert) {	/* Doing math on the size column, must delay unit conversion unless inch */
			gmt_set_column_type (GMT, GMT_IN, ex2, GMT_IS_FLOAT);
			delayed_unit_scaling[GMT_Y] = (S.u_set && S.u != GMT_INCH);	/* Since S.u will be set under GMT_X if that else branch kicked in */
		}

		if (!read_symbol) API->object[API->current_item[GMT_IN]]->n_expected_fields = n_needed;
		if (S.read_symbol_cmd) {	/* Must prepare for a rough ride */
			GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_VAR);
			if (GMT->common.l.active)
				GMT_Report (API, GMT_MSG_WARNING, "Cannot use auto-legend -l for variable symbol types. Option -l ignored.\n");
		}
		else if (GMT->common.l.active) {	/* Can we do auto-legend? */
			if (get_rgb)
				GMT_Report (API, GMT_MSG_WARNING, "Cannot use auto-legend -l for variable symbol color. Option -l ignored.\n");
			else if (S.read_size && gmt_M_is_zero (GMT->common.l.item.size))
				GMT_Report (API, GMT_MSG_WARNING, "Cannot use auto-legend -l for variable symbol size unless +S<size> is used. Option -l ignored.\n");
			else {
				/* For specified symbol, size, color we can do an auto-legend entry under modern mode */
				gmt_add_legend_item (API, &S, Ctrl->G.active, &(Ctrl->G.fill), Ctrl->W.active, &(Ctrl->W.pen), &(GMT->common.l.item), NULL);
			}
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
							if ((error = gmt_parse_symbol_option (GMT, s_args, &S, 0, false))) {
								Return (error);
							}
							nominal_size_x = S.size_x;
							nominal_size_y = S.size_y;
						}
						else
							GMT_Report (API, GMT_MSG_ERROR, "Segment header tries to switch to a line symbol like quoted line or fault - ignored\n");
					}
				}
				continue;							/* Go back and read the next record */
			}

			if (In->data == NULL) {
				gmt_quit_bad_record (API, In);
				Return (API->error);
			}

			/* Data record to process */

			in = In->data;
			n_total_read++;

			if (read_symbol) {	/* Must do special processing */
				if (S.read_symbol_cmd == 1) {
					if ((error = gmt_parse_symbol_option (GMT, In->text, &S, 1, false))) {
						Return (error);
					}
					nominal_size_x = S.size_x;
					nominal_size_y = S.size_y;
					if (gmt_is_barcolumn (GMT, &S)) {
						n_z = gmt_get_columbar_bands (GMT, &S);
						if (n_z > 1 && !Ctrl->C.active) {
							GMT_Report (API, GMT_MSG_ERROR, "The -Sb|B|o|O options with multiple layers require -C - skipping this point\n");
							continue;
						}
					}
					QR_symbol = (S.symbol == GMT_SYMBOL_CUSTOM && (!strcmp (S.custom->name, "QR") || !strcmp (S.custom->name, "QR_transparent")));
				}
				/* Since we only now know if some of the input columns should NOT be considered dimensions we
				 * must visit such columns and if the current length unit is NOT inch then we must undo the scaling */
				if (S.n_nondim && API->is_file && GMT->current.setting.proj_length_unit != GMT_INCH) {	/* Since these are not dimensions but angles or other quantities */
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
				else if (S.symbol == PSL_DOT && !fill_active) {	/* Must switch on default black fill */
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
				double value;
				if (S.v.status & PSL_VEC_MAGNIFY) {	/* Base color on vector magnitude in user units */
					if (S.v.status & PSL_VEC_COMPONENTS)	/* Read dx, dy in user units and compute magnitude */
						value = hypot (in[ex1+S.read_size], in[ex2+S.read_size]);
					else	/* Just get data magnitude as given */
						value = in[ex2+S.read_size];
				}
				else	/* Base color on w-vector */
					value = in[3];
				if (P->categorical & GMT_CPT_CATEGORICAL_KEY)
					gmt_get_fill_from_key (GMT, P, In->text, &current_fill);
				else
					gmt_get_fill_from_z (GMT, P, value, &current_fill);
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

			if (gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &data[n].x, &data[n].y) || gmt_M_is_dnan(in[GMT_Z])) continue;	/* NaNs on input */
			data[n].flag = S.convert_angles;
			data[n].z = gmt_z_to_zz (GMT, in[GMT_Z]);
			if (S.base_set & GMT_BASE_READ) {	/* Got base from input column */
				bcol = (S.read_size) ? ex2 : ex1;
				if (S.symbol == GMT_SYMBOL_COLUMN)
					bcol += S.n_required - 1;	/* Since we have z1 z2 ... z2 base */
				S.base = in[bcol];
			}
			if (gmt_is_barcolumn (GMT, &S)) {	/* Must allocate space for multiple z-values */
				n_z = gmt_get_columbar_bands (GMT, &S);
				data[n].zz = gmt_M_memory (GMT, NULL, n_z + S.sidebyside, double);	/* If sidebyside then zz[nz] has the gap */
				/* Accumulate increments, deal with any origin shifts, then project to final x, yy, zz coordinates depending on BARX, BARY, COLUMN */
				if (psxyz_load_bands (GMT, in, data[n].zz, n_z, n_total_read, &S)) continue;
				if (S.sidebyside) {
					 data[n].flag |= 64;
					 data[n].zz[n_z] = S.gap;
				}
			}

			if (S.read_size) {	/* Update sizes from input */
				S.size_x = in[ex1] * S.factor;
				if (delayed_unit_scaling[GMT_X]) S.size_x *= GMT->session.u2u[S.u][GMT_INCH];
				S.size_y = in[ex2];
				if (delayed_unit_scaling[GMT_Y]) S.size_y *= GMT->session.u2u[S.u][GMT_INCH];
				nominal_size_x = S.size_x;
				nominal_size_y = S.size_y;
			}
			if (Ctrl->H.active) {	/* Variable scaling of symbol size and pen width */
				double scl = (Ctrl->H.mode == PSXYZ_READ_SCALE) ? in[xcol] : Ctrl->H.value;
				S.size_x = nominal_size_x * scl;
				S.size_y = nominal_size_y * scl;
			}

			if ((S.symbol == PSL_ELLIPSE || S.symbol == PSL_ROTRECT) && !S.par_set) {	/* Ellipses or rectangles */
				if (S.n_required == 0)	/* Degenerate ellipse or rectangle, got diameter via S.size_x */
					axes[GMT_X] = axes[GMT_Y] = S.size_x, Az = (gmt_M_is_cartesian (GMT, GMT_IN)) ? 90.0 : 0.0;	/* Duplicate diameter as major and minor axes and set azimuth to zero  */
				else if (S.n_required == 1)	/* Degenerate ellipse or rectangle, expect single diameter via input */
					axes[GMT_X] = axes[GMT_Y] = in[ex1], Az = (gmt_M_is_cartesian (GMT, GMT_IN)) ? 90.0 : 0.0;	/* Duplicate diameter as major and minor axes and set azimuth to zero */
				else 	/* Full ellipse */
					Az = in[ex1], axes[GMT_X] = in[ex2], axes[GMT_Y] = in[ex3];
				if (gmt_M_is_geographic (GMT, GMT_IN)) {
					axes[GMT_X] *= S.geo_scale;
					axes[GMT_Y] *= S.geo_scale;
				}
			}

			if (S.base_set & GMT_BASE_ORIGIN) data[n].flag |= 32;	/* Flag that base needs to be added to height(s) */

			if (psxyz_is_stroke_symbol (S.symbol)) {	/* These are only stroked, not filled */
				/* Unless -W was set, compute pen width from symbol size and get pen color from G or z->CPT */
				if (!Ctrl->W.active && !outline_active)	/* No pen width given, compute from symbol size */
					current_pen.width = (gmt_M_is_zero (GMT->current.setting.map_symbol_pen_scale)) ? GMT->current.setting.map_default_pen.width : GMT->current.setting.map_symbol_pen_scale * S.size_x * PSL_POINTS_PER_INCH;
				if (current_fill.rgb[0] > -0.5) {	/* Color given, use it for the stroke */
					save_pen = current_pen;
					gmt_M_rgb_copy (current_pen.rgb, current_fill.rgb);
				}
				outline_active = true;
			}

			if (Ctrl->W.cpt_effect) {
				if (Ctrl->W.pen.cptmode & 1) {	/* Change pen color via CPT */
					gmt_M_rgb_copy (Ctrl->W.pen.rgb, current_fill.rgb);
					current_pen = Ctrl->W.pen;
					if (Ctrl->H.active) {
						double scl = (Ctrl->H.mode == PSXYZ_READ_SCALE) ? in[xcol] : Ctrl->H.value;
						gmt_scale_pen (GMT, &current_pen, scl);
					}
					if (can_update_headpen && !gmt_M_same_pen (current_pen, last_headpen))	/* Since color may have changed */
						last_headpen = current_pen;
				}
				if ((Ctrl->W.pen.cptmode & 2) == 0 && !Ctrl->G.active)	/* Turn off CPT fill */
					gmt_M_rgb_copy (current_fill.rgb, GMT->session.no_rgb);
				else if (Ctrl->G.active)
					current_fill = Ctrl->G.fill;
			}
			else if (Ctrl->H.active) {
				double scl = (Ctrl->H.mode == PSXYZ_READ_SCALE) ? in[xcol] : Ctrl->H.value;
				gmt_scale_pen (GMT, &current_pen, scl);
			}
			data[n].dim[0] = S.size_x;
			data[n].dim[1] = S.size_y;

			data[n].symbol = S.symbol;
			data[n].f = current_fill;
			data[n].p = current_pen;
			data[n].h = last_headpen;
			data[n].outline = outline_active ? 1 : 0;
			if (GMT->common.t.variable) {
				if (GMT->common.t.n_transparencies == 2) {	/* Requested two separate values to be read from file */
					data[n].transparency[GMT_FILL_TRANSP] = 0.01 * in[tcol_f];
					data[n].transparency[GMT_PEN_TRANSP]  = 0.01 * in[tcol_s];
				}
				else if (GMT->common.t.mode & GMT_SET_FILL_TRANSP) {	/* Gave fill transparency */
					data[n].transparency[GMT_FILL_TRANSP] = 0.01 * in[tcol_f];
					if (GMT->common.t.n_transparencies == 0) data[n].transparency[GMT_PEN_TRANSP] = data[n].transparency[GMT_FILL_TRANSP];	/* Implied to be used for stroke also */
				}
				else {	/* Gave stroke transparency */
					data[n].transparency[GMT_PEN_TRANSP] = 0.01 * in[tcol_s];
					if (GMT->common.t.n_transparencies == 0) data[n].transparency[GMT_FILL_TRANSP] = data[n].transparency[GMT_PEN_TRANSP];	/* Implied to be used for fill also */
				}
			}
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
					if (S.n_required == 3) {	/* Got radius from input file */
						if (gmt_M_is_dnan (in[ex3])) {
							GMT_Report (API, GMT_MSG_WARNING, "Rounded rectangle corner radius = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						data[n].dim[2] = in[ex3];	/* radius */
					}
					else
						data[n].dim[2] = S.factor;	/* radius */
					/* Intentionally fall through - to do the rest under regular rectangle */
				case PSL_RECT:
					if (S.n_required == 2) {	/* Got dimensions from input file */
						if (gmt_M_is_dnan (in[ex1])) {
							GMT_Report (API, GMT_MSG_WARNING, "Rectangle width = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						data[n].dim[0] = in[ex1];
						if (gmt_M_is_dnan (in[ex2])) {
							GMT_Report (API, GMT_MSG_WARNING, "Rectangle height = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						data[n].dim[1] = in[ex2];	/* y-dim */
					}
					break;
				case PSL_ELLIPSE:
				case PSL_ROTRECT:
					if (S.par_set) {	/* Given on command line */
						data[n].dim[0] = S.factor;	/* The angle/azimuth */
						data[n].dim[1] = S.size_x;
						data[n].dim[2] = S.size_y;
					}
					else {	/* Get parameters from file */
						if (gmt_M_is_dnan (Az)) {
							GMT_Report (API, GMT_MSG_WARNING, "Ellipse/Rectangle angle = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						if (gmt_M_is_dnan (axes[GMT_X])) {
							GMT_Report (API, GMT_MSG_WARNING, "Ellipse/Rectangle width or major axis = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						if (gmt_M_is_dnan (axes[GMT_Y])) {
							GMT_Report (API, GMT_MSG_WARNING, "Ellipse/Rectangle height or minor axis = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						data[n].dim[0] = Az;	/* direction */
						data[n].dim[1] = factor * axes[GMT_X];
						data[n].dim[2] = factor * axes[GMT_Y];
					}
					gmt_flip_angle_d (GMT, &data[n].dim[0]);
					if (S.convert_angles) {		/* Got axis in km */
						if (gmt_M_is_cartesian (GMT, GMT_IN)) {	/* Got axes in user units, change to inches via Cartesian scales */
							data[n].dim[1] *= GMT->current.proj.scale[GMT_X];
							data[n].dim[2] *= GMT->current.proj.scale[GMT_Y];
							data[n].dim[0] = 90.0 - data[n].dim[0];
						}
						else {	/* Fully geographic */
							data[n].flag |= 2;	/* Signals to use GMT_geo_* routine */
							data[n].x = in[GMT_X];	/* Revert to longitude and latitude */
							data[n].y = in[GMT_Y];
						}
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

					if (S.v.status & PSL_VEC_COMPONENTS) {	/* Read dx, dy in user units */
						d = d_atan2d (in[ex2+S.read_size], in[ex1+S.read_size]);	/* Compute direction */
						data_magnitude = hypot (in[ex1+S.read_size], in[ex2+S.read_size]);	/* Compute magnitude */
					}
					else {	/* Got direction and magnitude as is */
						d = in[ex1+S.read_size];
						data_magnitude = in[ex2+S.read_size];
					}
					if (S.v.status & PSL_VEC_FIXED) data_magnitude = 1.0;	/* Override with fixed vector length */

					if (gmt_M_is_dnan (data_magnitude)) {
						GMT_Report (API, GMT_MSG_WARNING, "Vector magnitude = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					if (gmt_M_is_dnan (d)) {
						GMT_Report (API, GMT_MSG_WARNING, "Vector azimuth = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}

					data[n].dim[1] = data_magnitude * S.v.comp_scale;
					if (!S.convert_angles)	/* Use direction as given */
						data[n].dim[0] = d;	/* direction */
					else if (gmt_M_is_cartesian (GMT, GMT_IN))	/* Cartesian azimuth; change to direction */
						data[n].dim[0] = 90.0 - d;
					else	/* Convert geo azimuth to map direction */
						data[n].dim[0] = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, d);

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
					data[n].dim[PSL_VEC_XTIP] = x_2;
					data[n].dim[PSL_VEC_YTIP] = y_2;
					s = gmt_get_vector_shrinking (GMT, &(S.v), data_magnitude, data[n].dim[1]);	/* Vector attribute shrinking factor or 1 */
					data[n].dim[PSL_VEC_TAIL_WIDTH]  = s * S.v.v_width;
					data[n].dim[PSL_VEC_HEAD_LENGTH] = s * S.v.h_length;
					data[n].dim[PSL_VEC_HEAD_WIDTH]  = s * S.v.h_width;
					if (S.v.parsed_v4) {	/* Parsed the old ways so plot the old ways... */
						data[n].dim[PSL_VEC_HEAD_WIDTH] *= 0.5;	/* Since it was double in the parsing */
						data[n].symbol = GMT_SYMBOL_VECTOR_V4;
						data[n].dim[PSL_VEC_HEAD_SHAPE] = GMT->current.setting.map_vector_shape;
					}
					else {
						data[n].dim[PSL_VEC_HEAD_SHAPE]      = S.v.v_shape;
						data[n].dim[PSL_VEC_STATUS]          = (double)S.v.status;
						data[n].dim[PSL_VEC_HEAD_TYPE_BEGIN] = (double)S.v.v_kind[0];
						data[n].dim[PSL_VEC_HEAD_TYPE_END]   = (double)S.v.v_kind[1];
						data[n].dim[PSL_VEC_TRIM_BEGIN]      = (double)S.v.v_trim[0];
						data[n].dim[PSL_VEC_TRIM_END]        = (double)S.v.v_trim[1];
						data[n].dim[PSL_VEC_HEAD_PENWIDTH]   = s * data[n].h.width;	/* Possibly shrunk head pen width */
					}
					break;
				case GMT_SYMBOL_GEOVECTOR:
					gmt_init_vector_param (GMT, &S, true, Ctrl->W.active, &Ctrl->W.pen, Ctrl->G.active, &Ctrl->G.fill);	/* Update vector head parameters */
					if (S.v.status & PSL_VEC_OUTLINE2)
						S.v.v_width = (float)(S.v.pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					else
						S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					if (S.v.status & PSL_VEC_COMPONENTS) {	/* Read dx, dy in user units to be scaled to km */
						double dx = in[ex1+S.read_size];
						double dy = in[ex2+S.read_size];
						data_magnitude = gmt_get_az_dist_from_components (GMT, in[GMT_X], in[GMT_Y], dx, dy, S.v.v_unit_d, &data[n].dim[0]);
					}
					else {	/* Got azimuth and length */
						data[n].dim[0] = in[ex1+S.read_size];
						data_magnitude = in[ex2+S.read_size];
					}
					if (S.v.status & PSL_VEC_FIXED) data_magnitude = 1.0;	/* Override with fixed vector length given by comp_scale */
					S.v.value = data_magnitude;
					data[n].dim[1] = data_magnitude * S.v.comp_scale;
					if (gmt_M_is_dnan (data[n].dim[0])) {
						GMT_Report (API, GMT_MSG_WARNING, "Geovector azimuth = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					if (gmt_M_is_dnan (data[n].dim[1])) {
						GMT_Report (API, GMT_MSG_WARNING, "Geovector length = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					data[n].x = in[GMT_X];			/* Revert to longitude and latitude */
					data[n].y = in[GMT_Y];
					data[n].v = S.v;
					break;
				case PSL_MARC:
					gmt_init_vector_param (GMT, &S, false, false, NULL, false, NULL);	/* Update vector head parameters */
					S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					data[n].dim[PSL_MATHARC_RADIUS]      = in[ex1+S.read_size];	/* Radius */
					data[n].dim[PSL_MATHARC_ANGLE_BEGIN] = in[ex2+S.read_size];	/* Start direction in degrees */
					data[n].dim[PSL_MATHARC_ANGLE_END]   = in[ex3+S.read_size];	/* Stop direction in degrees */
					length = fabs (data[n].dim[PSL_MATHARC_ANGLE_END]-data[n].dim[PSL_MATHARC_ANGLE_BEGIN]);	/* Arc length in degrees */
					if (gmt_M_is_dnan (length)) {
						GMT_Report (API, GMT_MSG_WARNING, "Math angle arc length = NaN near line %d. Skipped\n", n_total_read);
						continue;
					}
					s = (length < S.v.v_norm) ? length / S.v.v_norm : 1.0;
					if (s < S.v.v_norm_limit) s = S.v.v_norm_limit;
					data[n].dim[PSL_MATHARC_HEAD_LENGTH]     = s * S.v.h_length;	/* Length of (shrunk) vector head */
					data[n].dim[PSL_MATHARC_HEAD_WIDTH]      = s * S.v.h_width;	/* Width of (shrunk) vector head */
					data[n].dim[PSL_MATHARC_ARC_PENWIDTH]    = s * S.v.v_width;	/* Thickness of (shrunk) vector */
					data[n].dim[PSL_MATHARC_HEAD_SHAPE]      = S.v.v_shape;
					data[n].dim[PSL_MATHARC_STATUS]          = (double)S.v.status;	/* Vector tributes */
					data[n].dim[PSL_MATHARC_HEAD_TYPE_BEGIN] = (double)S.v.v_kind[0];
					data[n].dim[PSL_MATHARC_HEAD_TYPE_END]   = (double)S.v.v_kind[1];
					data[n].dim[PSL_MATHARC_TRIM_BEGIN]      = (double)S.v.v_trim[0];
					data[n].dim[PSL_MATHARC_TRIM_END]        = (double)S.v.v_trim[1];
					data[n].dim[PSL_MATHARC_HEAD_PENWIDTH]   = s * data[n].h.width;	/* Possibly shrunk head pen width */
					break;
				case PSL_WEDGE:
					col = ex1+S.read_size;
					if (S.w_get_do) {	/* Must read from file */
						if (gmt_M_is_dnan (in[col])) {
							GMT_Report (API, GMT_MSG_WARNING, "Wedge outer diameter = NaN near line %d. Skipped\n", n_total_read);
								continue;
						}
						data[n].dim[PSL_WEDGE_RADIUS_O] = in[col++] * S.geo_scale;
					}
					else	/* Set during -S parsing */
						data[n].dim[PSL_WEDGE_RADIUS_O] = S.w_radius;
					if (S.w_get_a) {	/* Must read from file */
						if (gmt_M_is_dnan (in[col])) {
							GMT_Report (API, GMT_MSG_WARNING, "Wedge start angle = NaN near line %d. Skipped\n", n_total_read);
								continue;
						}
						data[n].dim[PSL_WEDGE_ANGLE_BEGIN] = in[col++];
						if (gmt_M_is_dnan (in[col])) {
							GMT_Report (API, GMT_MSG_WARNING, "Wedge stop angle = NaN near line %d. Skipped\n", n_total_read);
								continue;
						}
						data[n].dim[PSL_WEDGE_ANGLE_END] = in[col++];
					}
					else {	/* Angles were set during -S parsing */
						data[n].dim[PSL_WEDGE_ANGLE_BEGIN] = S.size_x;
						data[n].dim[PSL_WEDGE_ANGLE_END]   = S.size_y;
					}
					if (S.w_get_di) {	/* Must read from file else it was set during -S parsing */
						if (gmt_M_is_dnan (in[col])) {
							GMT_Report (API, GMT_MSG_WARNING, "Wedge inner diameter = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						S.w_radius_i = in[col] * S.geo_scale;
					}
					if (S.convert_angles) {
						if (gmt_M_is_cartesian (GMT, GMT_IN)) {
							/* Note that the direction of the arc gets swapped when converting from azimuth */
							data[n].dim[PSL_WEDGE_ANGLE_END]   = 90.0 - data[n].dim[PSL_WEDGE_ANGLE_END];
							data[n].dim[PSL_WEDGE_ANGLE_BEGIN] = 90.0 - data[n].dim[PSL_WEDGE_ANGLE_BEGIN];
						}
						else {
							data[n].dim[PSL_WEDGE_ANGLE_END] = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, data[n].dim[PSL_WEDGE_ANGLE_END]);
							data[n].dim[PSL_WEDGE_ANGLE_BEGIN] = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, data[n].dim[PSL_WEDGE_ANGLE_BEGIN]);
						}
						gmt_M_double_swap (data[n].dim[1], data[n].dim[2]);	/* Must switch the order of the angles */
					}
					/* Load up the rest of the settings */
					data[n].dim[PSL_WEDGE_STATUS]   = S.w_type;
					data[n].dim[PSL_WEDGE_RADIUS_I] = S.w_radius_i;
					data[n].dim[PSL_WEDGE_DR]       = S.w_dr;	/* In case there is a request for radially spaced arcs */
					data[n].dim[PSL_WEDGE_DA]       = S.w_da;	/* In case there is a request for angularly spaced radial lines */
					data[n].dim[PSL_WEDGE_ACTION]   = 0.0;	/* Reset */
					if (fill_active || get_rgb) data[n].dim[PSL_WEDGE_ACTION] = 1;	/* Lay down filled wedge */
					if (outline_active) data[n].dim[PSL_WEDGE_ACTION] += 2;	/* Draw wedge outline */
					if (!S.w_active) {	/* Not geowedge so scale to radii */
						data[n].dim[PSL_WEDGE_RADIUS_O] *= 0.5;
						data[n].dim[PSL_WEDGE_RADIUS_I] *= 0.5;
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
			if (read_symbol) {
				API->object[API->current_item[GMT_IN]]->n_expected_fields = GMT_MAX_COLUMNS;
				if (psxyz_is_stroke_symbol (S.symbol))	/* Reset */
					gmt_M_rgb_copy (current_pen.rgb, save_pen.rgb);
			}
		} while (true);

		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}

		n_alloc = n;
		data = gmt_M_malloc (GMT, data, 0, &n_alloc, struct PSXYZ_DATA);

		/* Sort according to distance from viewer */

		if (!Ctrl->Q.active) qsort (data, n, sizeof (struct PSXYZ_DATA), psxyz_dist_compare);

		/* Now plot these symbols one at the time */

		for (i = 0; i < n; i++) {

			if (n_z == 1 || (data[i].symbol == GMT_SYMBOL_CUBE || data[i].symbol == GMT_SYMBOL_CUBE)) {
				for (j = 0; j < 3; j++) {
					gmt_M_rgb_copy (rgb[j], data[i].f.rgb);
					if (S.shade3D) gmt_illuminate (GMT, lux[j], rgb[j]);
				}
			}
			if (!geovector) {
				gmt_setfill (GMT, &data[i].f, data[i].outline);
				if (data[i].outline) gmt_setpen (GMT, &data[i].p);
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
				PSL_settransparencies (PSL, data[i].transparency);

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
						zz = zb = data[i].dim[2];	/* Projected x-value of start of bar */
						if (data[i].flag & 64) {	/* Compute skinny bar width and gaps */
							bar_gap = data[i].dim[0] * data[i].zz[n_z] * 0.01;	/* Total width of all gaps */
							bar_width = (data[i].dim[0] - bar_gap) / n_z;	/* Width of individual skinny bars */
							bar_gap /= (n_z - 1);	/* Width of individual gap */
							bar_step = bar_width + bar_gap;	/* Spacing between start of each bar */
							y_1 = data[i].y - 0.5 * data[i].dim[0] - 0.5 * data[i].dim[0];
						}
						for (k = 0; k < n_z; k++) {	/* For each band in the column */
							if (Ctrl->C.active && n_z > 1) {	/* Must update band color based on band number k */
								gmt_get_fill_from_z (GMT, P, k+0.5, &data[i].f);
								gmt_setfill (GMT, &data[i].f, data[i].outline);
							}
							if (data[i].flag & 64) {	/* Sidebyside */
								y_2 = y_1 + bar_step * k;	/* Recycle y_2 as bottom point on skinny bar k */
								PSL_plotbox (PSL, zb, y_2, data[i].zz[k], y_2 + bar_width);
							}
							else {
								zb = zz;
								zz = data[i].zz[k];	/* Projected x-values */
								PSL_plotbox (PSL, zb, data[i].y - 0.5 * data[i].dim[0], zz, data[i].y + 0.5 * data[i].dim[0]);
							}
						}
						if (item == last_time) gmt_M_free (GMT, data[i].zz);	/* Free column band array */
						break;
					case GMT_SYMBOL_BARY:
						if (!Ctrl->N.active) in[GMT_Y] = MAX (GMT->common.R.wesn[YLO], MIN (data[i].y, GMT->common.R.wesn[YHI]));
						if (data[i].flag & 4) {
							gmt_geo_to_xy (GMT, xpos[item] - 0.5 * data[i].dim[0], data[i].y, &x_1, &y_1);
							gmt_geo_to_xy (GMT, xpos[item] + 0.5 * data[i].dim[0], data[i].y, &x_2, &y_2);
							data[i].dim[0] = 0.5 * hypot (x_1 - x_2, y_1 - y_2);
						}
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						zz = zb = data[i].dim[2];	/* Projected y-value of start of bar */
						if (data[i].flag & 64) {	/* Compute skinny bar width and gaps */
							bar_gap = data[i].dim[0] * data[i].zz[n_z] * 0.01;	/* Total width of all gaps */
							bar_width = (data[i].dim[0] - bar_gap) / n_z;	/* Width of individual skinny bars */
							bar_gap /= (n_z - 1);	/* Width of individual gap */
							bar_step = bar_width + bar_gap;	/* Spacing between start of each bar */
							x_1 = xpos[item] - 0.5 * data[i].dim[0];
						}
						for (k = 0; k < n_z; k++) {	/* For each band in the column */
							if (Ctrl->C.active && n_z > 1) {	/* Must update band color based on band number k */
								gmt_get_fill_from_z (GMT, P, k+0.5, &data[i].f);
								gmt_setfill (GMT, &data[i].f, data[i].outline);
							}
							if (data[i].flag & 64) {	/* Sidebyside */
								x_2 = x_1 + bar_step * k;	/* Recycle x_2 as left point on skinny bar k */
								PSL_plotbox (PSL, x_2, zb, x_2 + bar_width, data[i].zz[k]);
							}
							else {
								zb = zz;
								zz = data[i].zz[k];	/* Projected y-values */
								PSL_plotbox (PSL, xpos[item] - 0.5 * data[i].dim[0], zb, xpos[item] + 0.5 * data[i].dim[0], zz);
							}
						}
						if (item == last_time) gmt_M_free (GMT, data[i].zz);	/* Free column band array */
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
						base = data[i].dim[2];	/* Projected z-value of start of column */
						for (k = 0; k < n_z; k++) {	/* For each band in the column */
							if (Ctrl->C.active && n_z > 1) {
								/* Must update band color based on band number k */
								gmt_get_fill_from_z (GMT, P, k+0.5, &current_fill);
								for (j = 0; j < 3; j++) {
									gmt_M_rgb_copy (rgb[j], current_fill.rgb);
									if (S.shade3D) gmt_illuminate (GMT, lux[j], rgb[j]);
								}
							}
							zz = data[i].zz[k];	/* Projected z-values */
							dim[2] = fabs (zz - base);	/* band height in projected units */
							psxyz_column3D (GMT, PSL, xpos[item], data[i].y, (zz + base) / 2.0, dim, rgb, data[i].outline);
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
						psxyz_column3D (GMT, PSL, xpos[item], data[i].y, data[i].z, dim, rgb, data[i].outline);
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
						if (S.azim) {	/* Must update angle */
							gmt_xy_to_geo (GMT, &dx, &dy, data[i].y, data[i].y);	/* Just recycle dx, dy here */
							direction = gmt_azim_to_angle (GMT, dx, dy, 0.1, S.angle);
						}
						else
							direction = S.angle;
						PSL_plottext (PSL, xpos[item], data[i].y, data[i].dim[0] * PSL_POINTS_PER_INCH, data[i].string, direction, S.justify, data[i].outline);
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
						v4_status = lrint (data[n].dim[PSL_VEC_STATUS]);
						if (v4_status & PSL_VEC_BEGIN) v4_outline += 8;	/* Double-headed */
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						psl_vector_v4 (PSL, xpos[item], data[i].y, data[i].dim, v4_rgb, v4_outline);
						break;
					case GMT_SYMBOL_GEOVECTOR:
						gmt_plane_perspective (GMT, GMT_Z, data[i].z);
						S.v = data[i].v;	/* Update vector attributes from saved values */
						if (get_rgb) S.v.fill = data[i].f;
						PSL_defpen (PSL, "PSL_vecheadpen", data[i].h.width, data[i].h.style, data[i].h.offset, data[i].h.rgb);
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
							unsigned int status = lrint (data[i].dim[PSL_WEDGE_STATUS]);
							gmt_xy_to_geo (GMT, &dx, &dy, data[i].x, data[i].y);	/* Just recycle dx, dy here */
							gmt_geo_wedge (GMT, dx, dy, data[i].dim[PSL_WEDGE_RADIUS_I], data[i].dim[PSL_WEDGE_RADIUS_O], data[i].dim[PSL_WEDGE_DR], data[i].dim[PSL_WEDGE_ANGLE_BEGIN],
								data[i].dim[PSL_WEDGE_ANGLE_END], data[i].dim[PSL_WEDGE_DA], status, fill_active || get_rgb, outline_active);
						}
						else
							PSL_plotsymbol (PSL, xpos[item], data[i].y, data[i].dim, PSL_WEDGE);
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
						if ((error = gmt_draw_custom_symbol (GMT, xpos[item], data[i].y, dim, data[i].string, data[i].custom, &data[i].p, &data[i].f, data[i].outline)))
							Return (error);
						gmt_M_free (GMT, data[i].custom);
						if (data[i].string) gmt_M_str_free (data[i].string);
						break;
				}
			}
		}
		if (GMT->common.t.variable) {	/* Reset the transparencies */
			double transp[2] = {0.0, 0.0};	/* None selected */
			PSL_settransparencies (PSL, transp);
		}
		if (n_warn[1]) GMT_Report (API, GMT_MSG_INFORMATION, "%d vector heads had length exceeding the vector length and were skipped. Consider the +n<norm> modifier to -S\n", n_warn[1]);
		if (n_warn[2]) GMT_Report (API, GMT_MSG_INFORMATION, "%d vector heads had to be scaled more than implied by +n<norm> since they were still too long. Consider changing the +n<norm> modifier to -S\n", n_warn[2]);
		gmt_M_free (GMT, data);
		gmt_reset_meminc (GMT);
	}
	else {	/* Line/polygon part */
		bool duplicate = false, conf_line = false, no_line_clip = (Ctrl->N.active && S.symbol == GMT_SYMBOL_LINE);
		int outline_setting;
		uint64_t seg;
		struct GMT_PALETTE *A = NULL;
		struct GMT_DATASET *D = NULL;	/* Pointer to GMT segment table(s) */
		struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
		struct GMT_DATASET_HIDDEN *DH = NULL;

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
		DH = gmt_get_DD_hidden (D);

		if (Ctrl->G.sequential || Ctrl->W.sequential) {	/* Load in the color-list as a categorical CPT */
			if ((A = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, GMT->current.setting.color_set, NULL)) == NULL) {
				Return (API->error);
			}
			gmt_init_next_color (GMT);
			if (GMT->common.l.active) {	/* Want auto legend for all the lines or polygons */
				seq_legend = true;
				seq_n_legends = A->n_colors;
				seq_frequency = MAX (Ctrl->G.sequential, Ctrl->W.sequential);
			}
		}

		if (z_for_cpt || Ctrl->Z.set_transp) {	/* Check that the Z length matches our polygon file */
			if (n_z_for_cpt < D->n_segments) {
				GMT_Report (API, GMT_MSG_ERROR, "Number of Z values (%" PRIu64 ") is less then number of polygons (%" PRIu64 ")\n", n_z_for_cpt, D->n_segments);
				gmt_M_free (GMT, z_for_cpt);
				if (Ctrl->Z.set_transp) gmt_M_free (GMT, t_for_cpt);
				Return (API->error);
			}
		}

		conf_line = (Ctrl->L.anchor >= PSXYZ_POL_SYMM_DEV && Ctrl->L.anchor <= PSXYZ_POL_ASYMM_ENV);

		if (!seq_legend && GMT->common.l.active) {
			if (S.symbol == GMT_SYMBOL_LINE) {
				if (polygon || conf_line) {	/* Place a rectangle in the legend */
					int symbol = S.symbol;
					struct GMT_PEN *cpen = (Ctrl->L.outline) ? &(Ctrl->L.pen) : NULL;
					S.symbol = (Ctrl->L.active && Ctrl->G.active && Ctrl->W.active) ? 'L' : PSL_RECT;	/* L means confidence-line */
					gmt_add_legend_item (API, &S, Ctrl->G.active, &(Ctrl->G.fill), Ctrl->W.active, &(Ctrl->W.pen), &(GMT->common.l.item), cpen);
					S.symbol = symbol;
				}
				else	/* For specified line, width, color we can do an auto-legend entry under modern mode */
					gmt_add_legend_item (API, &S, false, NULL, Ctrl->W.active, &(Ctrl->W.pen), &(GMT->common.l.item), NULL);
			}
			else
				GMT_Report (API, GMT_MSG_WARNING, "Cannot use auto-legend -l for selected feature. Option -l ignored.\n");
		}

		for (tbl = 0; tbl < D->n_tables; tbl++) {
			if (D->table[tbl]->n_headers && S.G.label_type == GMT_LABEL_IS_HEADER)
				gmt_extract_label (GMT, &D->table[tbl]->header[0][1], S.G.label, NULL);	/* Set first header as potential label */

			if (Ctrl->G.sequential == GMT_COLOR_AUTO_TABLE) {	/* Update sequential fill color per table */
				gmt_set_next_color (GMT, A, GMT_COLOR_AUTO_TABLE, current_fill.rgb);
				gmt_setfill (GMT, &current_fill, outline_setting);
			}
			else if (Ctrl->W.sequential == GMT_COLOR_AUTO_TABLE) {	/* Update sequential pen color per table */
				gmt_set_next_color (GMT, A, GMT_COLOR_AUTO_TABLE, current_pen.rgb);
				gmt_setpen (GMT, &current_pen);
			}

			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */

				L = D->table[tbl]->segment[seg];	/* Set shortcut to current segment */

				if (gmt_segment_BB_outside_map_BB (GMT, L)) continue;
				if (polygon && gmt_polygon_is_hole (GMT, L)) continue;	/* Holes are handled together with perimeters */

				if (Ctrl->G.sequential == GMT_COLOR_AUTO_SEGMENT) {	/* Update sequential fill color per segment */
					gmt_set_next_color (GMT, A, GMT_COLOR_AUTO_SEGMENT, current_fill.rgb);
					gmt_setfill (GMT, &current_fill, outline_setting);
				}
				else if (Ctrl->W.sequential == GMT_COLOR_AUTO_SEGMENT) {	/* Update sequential pen color per segment */
					gmt_set_next_color (GMT, A, GMT_COLOR_AUTO_SEGMENT, current_pen.rgb);
					gmt_setpen (GMT, &current_pen);
				}

				SH = gmt_get_DS_hidden (L);

				if (seq_legend && seq_n_legends >= 0 && (seq_frequency == GMT_COLOR_AUTO_SEGMENT || seg == 0)) {
					if (GMT->common.l.item.label_type == GMT_LEGEND_LABEL_HEADER && L->header)	/* Use a segment label if found in header */
						gmt_extract_label (GMT, L->header, GMT->common.l.item.label, SH->ogr);
					if (polygon || conf_line) {	/* Place a rectangle in the legend */
						int symbol = S.symbol;
						struct GMT_PEN *cpen = (Ctrl->L.outline) ? &(Ctrl->L.pen) : NULL;
						S.symbol = (Ctrl->L.active && Ctrl->G.active && Ctrl->W.active) ? 'L' : PSL_RECT;	/* L means confidence-line */
						gmt_add_legend_item (API, &S, Ctrl->G.active, &current_fill, Ctrl->W.active, &current_pen, &(GMT->common.l.item), cpen);
						S.symbol = symbol;
					}
					else	/* For specified line, width, color we can do an auto-legend entry under modern mode */
						gmt_add_legend_item (API, &S, false, NULL, Ctrl->W.active, &current_pen, &(GMT->common.l.item), NULL);
					seq_n_legends--;	/* One less to do */
					GMT->common.l.item.ID++;	/* Increment the label counter */
				}

				if (D->n_tables > 1)
					GMT_Report (API, GMT_MSG_INFORMATION, "Plotting table %" PRIu64 " segment %" PRIu64 "\n", tbl, seg);
				else
					GMT_Report (API, GMT_MSG_INFORMATION, "Plotting segment %" PRIu64 "\n", seg);

				duplicate = (DH->alloc_mode == GMT_ALLOC_EXTERNALLY && GMT->current.map.path_mode == GMT_RESAMPLE_PATH && psxyz_no_z_variation (GMT, L));
				if (duplicate) {	/* Must duplicate externally allocated segment since it needs to be resampled below */
					L = gmt_duplicate_segment (GMT, D->table[tbl]->segment[seg]);
					SH = gmt_get_DS_hidden (L);
				}

				/* We had here things like:	x = D->table[tbl]->segment[seg]->data[GMT_X];
				 * but reallocating x below lead to disasters.  */

				outline_setting = outline_active ? 1 : 0;
				if (use_z_table) {	/* Look up line/polygon color and/or transparency via separate z-column and CPT */
					if (z_for_cpt != NULL) {
						double rgb[4];
						(void)gmt_get_rgb_from_z (GMT, P, z_for_cpt[seg], rgb);
						if (Ctrl->W.set_color) {	/* To be used in polygon outline */
							gmt_M_rgb_copy (current_pen.rgb, rgb);
							gmt_setpen (GMT, &current_pen);
						}
						if (Ctrl->G.set_color) {	/* To be used in polygon fill */
							gmt_M_rgb_copy (current_fill.rgb, rgb);
							gmt_setfill (GMT, &current_fill, outline_setting);
						}
					}
					if (t_for_cpt != NULL) {	/* Look up transparency via separate t-column */
						double transp[2];
						if (Ctrl->W.set_color) {	/* To modulate polygon or symbol outline */
							transp[GMT_PEN_TRANSP] = 0.01 * t_for_cpt[seg];
							transp[GMT_FILL_TRANSP] = 0.0;
							PSL_settransparencies (PSL, transp);
						}
						if (Ctrl->G.set_color) {	/* To modulate polygon or symbol fill */
							transp[GMT_FILL_TRANSP] = 0.01 * t_for_cpt[seg];
							transp[GMT_PEN_TRANSP] = 0.0;
							PSL_settransparencies (PSL, transp);
						}
					}
				}
				else {
					change = gmt_parse_segment_header (GMT, L->header, P, &fill_active, &current_fill, &default_fill, &outline_active, &current_pen, &default_pen, default_outline, SH->ogr);
					outline_setting = outline_active ? 1 : 0;
				}

				if (P && PH->skip) {
					if (duplicate)	/* Free duplicate segment */
						gmt_free_segment (GMT, &L);
					continue;	/* Chosen CPT indicates skip for this z */
				}

				if (L->header && L->header[0]) {
					PSL_comment (PSL, "Segment header: %s\n", L->header);
					if (gmt_parse_segment_item (GMT, L->header, "-S", s_args)) {	/* Found -S */
						if ((S.symbol == GMT_SYMBOL_QUOTED_LINE && s_args[0] == 'q') || (S.symbol == GMT_SYMBOL_FRONT && s_args[0] == 'f')) { /* Update parameters */
							if ((error = gmt_parse_symbol_option (GMT, s_args, &S, 0, false))) {
								Return (error);
							}
							if (change & 1) change -= 1;	/* Don't want polygon to be true later for these symbols */
						}
						else if (S.symbol == GMT_SYMBOL_QUOTED_LINE || S.symbol == GMT_SYMBOL_FRONT)
							GMT_Report (API, GMT_MSG_ERROR, "Segment header tries to switch from -S%c to another symbol (%s) - ignored\n", S.symbol, s_args);
						else	/* Probably just junk -S in header */
							GMT_Report (API, GMT_MSG_INFORMATION, "Segment header contained -S%s - ignored\n", s_args);
					}
				}
				if (current_pen.mode == PSL_BEZIER && (S.symbol == GMT_SYMBOL_DECORATED_LINE || S.symbol == GMT_SYMBOL_QUOTED_LINE || S.symbol == GMT_SYMBOL_FRONT)) {
					GMT_Report (API, GMT_MSG_WARNING, "Bezier spline mode (modifier +s) is not supported for fronts, quoted, or decorated lines - mode ignored\n");
					current_pen.mode = PSL_LINEAR;
				}
				if (S.fq_parse) { /* Did not supply -Sf or -Sq in the segment header */
					if (S.symbol == GMT_SYMBOL_QUOTED_LINE) /* Did not supply -Sf in the segment header */
						GMT_Report (API, GMT_MSG_ERROR, "Segment header did not supply enough parameters for -Sf; skipping this segment\n");
					else
						GMT_Report (API, GMT_MSG_ERROR, "Segment header did not supply enough parameters for -Sq; skipping this segment\n");
					if (duplicate)	/* Free duplicate segment */
						gmt_free_segment (GMT, &L);
					continue;
				}

				if (Ctrl->I.active) {
					gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
					gmt_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
				}

				if (Ctrl->W.cpt_effect) {
					if (Ctrl->W.pen.cptmode & 1) {	/* Change current pen color via CPT */
						gmt_M_rgb_copy (current_pen.rgb, current_fill.rgb);
						gmt_setpen (GMT, &current_pen);
					}
					if ((Ctrl->W.pen.cptmode & 2) == 0 && !Ctrl->G.active)	/* Turn off CPT fill */
						gmt_M_rgb_copy (current_fill.rgb, GMT->session.no_rgb);
					else if (Ctrl->G.active)
						current_fill = Ctrl->G.fill;
				}
				else if (z_for_cpt == NULL) {
					if (change & 1 && Ctrl->L.anchor == 0) polygon = true;
					if (change & 2 && !Ctrl->L.polygon) {
						polygon = false;
						PSL_setcolor (PSL, current_fill.rgb, PSL_IS_STROKE);
					}
					if (change & 4 && penset_OK) gmt_setpen (GMT, &current_pen);
				}

				if (S.G.label_type == GMT_LABEL_IS_HEADER)	/* Get potential label from segment header */
					gmt_extract_label (GMT, L->header, S.G.label, SH->ogr);

				if (GMT->current.map.path_mode == GMT_RESAMPLE_PATH && psxyz_no_z_variation (GMT, L)) {	/* Resample if spacing is too coarse and no z-variation */
					uint64_t n_new;
					double z_level = L->data[GMT_Z][0];	/* The constant z-level for this line */
					if (gmt_M_is_geographic (GMT, GMT_IN))
						n_new = gmt_fix_up_path (GMT, &L->data[GMT_X], &L->data[GMT_Y], L->n_rows, Ctrl->A.step, Ctrl->A.mode);
					else
						n_new = gmt_resample_path (GMT, &L->data[GMT_X], &L->data[GMT_Y], L->n_rows, 0.5 * hypot (L->data[GMT_X][1]-L->data[GMT_X][0], L->data[GMT_Y][1]-L->data[GMT_Y][0]), GMT_TRACK_FILL);
					if (n_new == 0) {
						Return (GMT_RUNTIME_ERROR);
					}
					L->n_rows = n_new;
					L->data[GMT_Z] = gmt_M_memory (GMT, L->data[GMT_Z], L->n_rows, double);	/* Must resize this array too */
					for (k = 0; k < L->n_rows; k++) L->data[GMT_Z][k] = z_level;
					gmt_set_seg_minmax (GMT, D->geometry, 2, L);	/* Update min/max of x/y only */
				}

				n = (int)L->n_rows;				/* Number of points in this segment */
				xp = gmt_M_memory (GMT, NULL, n, double);
				yp = gmt_M_memory (GMT, NULL, n, double);

				if (polygon && !no_line_clip) {
					gmt_plane_perspective (GMT, -1, 0.0);
					for (i = 0; i < n; i++) gmt_geoz_to_xy (GMT, L->data[GMT_X][i], L->data[GMT_Y][i], L->data[GMT_Z][i], &xp[i], &yp[i]);
					gmt_setfill (GMT, &current_fill, outline_setting);
					PSL_plotpolygon (PSL, xp, yp, (int)n);
				}
				else if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {	/* Labeled lines are dealt with by the contour machinery */
					bool closed;
					/* Note that this always be plotted in the XY-plane */
					gmt_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
					if ((GMT->current.plot.n = gmt_geo_to_xy_line (GMT, L->data[GMT_X], L->data[GMT_Y], L->n_rows)) == 0) continue;
					S.G.line_pen = current_pen;
					closed = (GMT->current.plot.n > 2 && !(gmt_polygon_is_open (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n)));
					gmt_hold_contour (GMT, &GMT->current.plot.x, &GMT->current.plot.y, GMT->current.plot.n, 0.0, "N/A", 'A', S.G.label_angle, closed, false, &S.G);
					GMT->current.plot.n_alloc = GMT->current.plot.n;	/* Since gmt_hold_contour reallocates to fit the array */
				}
				else {	/* Plot line */
					uint64_t end;
					bool draw_line = true;
					gmt_plane_perspective (GMT, -1, 0.0);
					if (Ctrl->L.anchor) {	/* Build a polygon in one of several ways */
						if (Ctrl->L.anchor == PSXYZ_POL_SYMM_DEV || Ctrl->L.anchor == PSXYZ_POL_ASYMM_DEV) {	/* Build envelope around y(x) from delta y values in 1 or 2 extra columns */
							uint64_t k, m, col = (Ctrl->L.anchor == PSXYZ_POL_ASYMM_DEV) ? 4 : 3;
							if (col >= L->n_columns) {
								GMT_Report (API, GMT_MSG_ERROR, "Option -L: Data have %" PRIu64 " columns but %" PRIu64 " are needed\n", L->n_columns, col+1);
								Return (GMT_RUNTIME_ERROR);
							}
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
						else if (Ctrl->L.anchor == PSXYZ_POL_ASYMM_ENV) {	/* Build envelope around y(x) from low and high 2 extra columns */
							uint64_t k, m;
							if (L->n_columns < 5) {
								GMT_Report (API, GMT_MSG_ERROR, "Option -L: Data have %" PRIu64 " columns but 5 are needed\n", L->n_columns);
								Return (GMT_RUNTIME_ERROR);
							}
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
								case XHI:	off = 1;	/* Intentionally fall through - to select the x max entry */
								case XLO:
								case ZLO:
									value = (Ctrl->L.mode == ZLO) ? Ctrl->L.value : GMT->common.R.wesn[XLO+off];
									GMT->hidden.mem_coord[GMT_X][end] = GMT->hidden.mem_coord[GMT_X][end+1] = value;
									GMT->hidden.mem_coord[GMT_Z][end] = GMT->hidden.mem_coord[GMT_Z][end+1] = L->data[GMT_Z][0];
									GMT->hidden.mem_coord[GMT_Y][end] = L->data[GMT_Y][end-1];
									GMT->hidden.mem_coord[GMT_Y][end+1] = L->data[GMT_Y][0];
									break;
								case YHI:	off = 1;	/* Intentionally fall through - to select the y max entry */
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
						for (i = 0; i < n; i++)
							gmt_geoz_to_xy (GMT, L->data[GMT_X][i], L->data[GMT_Y][i], L->data[GMT_Z][i], &xp[i], &yp[i]);
					}
					if (no_line_clip) {	/* Draw line or polygon without border clipping at all */
						if ((GMT->current.plot.n = gmt_cart_to_xy_line (GMT, xp, yp, n)) == 0) continue;
						if (outline_active) gmt_setpen (GMT, &current_pen);	/* Select separate pen for polygon outline */
						if (Ctrl->G.active) {	/* Specify the fill, possibly set outline */
							gmt_setfill (GMT, &current_fill, outline_active);
							PSL_plotpolygon (PSL, xp, yp, (int)n);
						}
						else {	/* No fill, just outline but may still be polygon */
							gmt_setfill (GMT, NULL, outline_active);
							if (polygon)
								PSL_plotpolygon (PSL, xp, yp, (int)n);
							else
								PSL_plotline (PSL, xp, yp, (int)n, PSL_MOVE|PSL_STROKE);
						}
					}
					else if (draw_line && (S.symbol != GMT_SYMBOL_FRONT || !S.f.invisible)) {
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
				if (duplicate)	/* Free duplicate segment */
					gmt_free_segment (GMT, &L);

				gmt_M_free (GMT, xp);
				gmt_M_free (GMT, yp);
			}
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
		if (z_for_cpt) gmt_M_free (GMT, z_for_cpt);
		if (t_for_cpt) gmt_M_free (GMT, t_for_cpt);
	}
	PSL_command (GMT->PSL, "U\n");	/* Undo the gsave for all symbols or lines */

	if (S.u_set) GMT->current.setting.proj_length_unit = save_u;	/* Reset unit */

	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		if (S.G.save_labels) {	/* Want to save the Line label locations (lon, lat, angle, label) */
			if ((error = gmt_contlabel_save_begin (GMT, &S.G)) != 0) Return (error);
			if ((error = gmt_contlabel_save_end (GMT, &S.G)) != 0) Return (error);
		}
		gmt_contlabel_plot (GMT, &S.G);
	}

	if (clip_set && !S.G.delay) gmt_map_clip_off (GMT);	/* We delay map clip off if text clipping was chosen via -Sq<args:+e */

	gmt_plane_perspective (GMT, GMT_Z + GMT_ZW, GMT->current.proj.z_level);
	gmt_map_basemap (GMT);	/* Plot basemap last if not 3-D */
	if (GMT->current.proj.three_D)
		gmt_vertical_axis (GMT, 2);	/* Draw foreground axis */
		
	gmt_plane_perspective (GMT, -1, 0.0);

	if (Ctrl->D.active) PSL_setorigin (PSL, -DX, -DY, 0.0, PSL_FWD);	/* Shift plot a bit */

	PSL_setdash (PSL, NULL, 0);
	if (geovector) PSL->current.linewidth = 0.0;	/* Since we changed things under clip; this will force it to be set next */
	GMT->current.map.is_world = old_is_world;

	gmt_symbol_free (GMT, &S);

	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}

int GMT_plot3d (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: plot3d\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psxyz (V_API, mode, args);
}
