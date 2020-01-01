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
 * Brief synopsis: psscale draws a grayscale or colorscale either vertically
 * or horizontally, optionally with illumination.
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psscale"
#define THIS_MODULE_MODERN_NAME	"colorbar"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot gray scale or color scale bar"
#define THIS_MODULE_KEYS	"CC{,>X},ZD("
#define THIS_MODULE_NEEDS	"jr"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYptxyf" GMT_OPT("c")

EXTERN_MSC void gmt_linearx_grid (struct GMT_CTRL *GMT, struct PSL_CTRL *P, double w, double e, double s, double n, double dval);
EXTERN_MSC double gmtlib_get_map_interval (struct GMT_CTRL *GMT, struct GMT_PLOT_AXIS_ITEM *T);
EXTERN_MSC void plot_timex_grid (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double w, double e, double s, double n, unsigned int item);

#define H_BORDER 16	/* 16p horizontal border space for -T [not used] */
#define V_BORDER 8	/* 8p vertical border space for -T [not used] */

#define PSSCALE_L_SCALE	0.80	/* Set scale length to 80% of map side length under auto-setting */
#define PSSCALE_W_SCALE	0.04	/* Set scale width to 4% of scale length under auto-setting */
#define PSSCALE_CYCLE_DIM 0.45	/* Cyclic symbol radius is 0.45 of width */
#define N_FAVOR_IMAGE	1
#define N_FAVOR_POLY	2

#ifdef DEBUG
static bool dump = false;
static double dump_int_val;
#endif

enum psscale_shift {
	PSSCALE_FLIP_ANNOT = 1,
	PSSCALE_FLIP_LABEL = 2,
	PSSCALE_FLIP_UNIT  = 4,
	PSSCALE_FLIP_VERT  = 8};

/* Control structure for psscale */

struct PSSCALE_CTRL {
	struct C {	/* -C<cpt> */
		bool active;
		char *file;
	} C;
	struct D {	/* -D[g|j|n|x]<refpoint>+w<length>/<width>[+m<move>][+h][+j<justify>][+o<dx>[/<dy>]]+e[b|f][<length>][+n[<text>]] */
		bool active;
		bool horizontal;
		bool move;
		struct GMT_REFPOINT *refpoint;
		int justify;
		unsigned int mmode;
		double dim[2];
		double off[2];
		bool extend;
		unsigned int emode;
		double elength;
		char *etext;
		char *opt;
	} D;
	struct F {	/* -F[+c<clearance>][+g<fill>][+i[<off>/][<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;
		struct GMT_MAP_PANEL *panel;
	} F;
	struct G {	/* -Glow/high for input CPT truncation */
		bool active;
		double z_low, z_high;
	} G;
	struct I {	/* -I[<intens>|<min_i>/<max_i>] */
		bool active;
		double min, max;
	} I;
	struct M {	/* -M */
		bool active;
	} M;
	struct N {	/* -N<dpi>|p */
		bool active;
		unsigned int mode;
		double dpi;
	} N;
	struct L {	/* -L[i][<gap>] */
		bool active;
		bool interval;
		double spacing;
	} L;
	struct Q {	/* -Q */
		bool active;
	} Q;
	struct S {	/* -S */
		bool active;
	} S;
	struct W {	/* -W<scale> */
		bool active;
		double scale;
	} W;
	struct Z {	/* -Z<zfile> */
		bool active;
		char *file;
	} Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSSCALE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSSCALE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->D.opt = strdup ("JBC");
	C->G.z_low = C->G.z_high = GMT->session.d_NaN;	/* No truncation */
	C->N.dpi = 600.0;
	C->I.min = -1.0;
	C->I.max = +1.0;
	C->L.spacing = -1.0;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSSCALE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);
	gmt_free_refpoint (GMT, &C->D.refpoint);
	gmt_M_str_free (C->D.etext);
	gmt_M_str_free (C->D.opt);
	gmt_M_free (GMT, C->F.panel);
	gmt_M_str_free (C->Z.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psscale synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [%s] [-C<cpt>]\n", name, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-D%s[+w<length>[/<width>]][+e[b|f][<length>]][+h|v][+j<justify>][+ma|c|l|u][+n[<txt>]]%s]\n", GMT_XYANCHOR, GMT_OFFSET);
	GMT_Message (API, GMT_TIME_NONE, "\t[-F%s]\n", GMT_PANEL);
	GMT_Message (API, GMT_TIME_NONE, "\t[-G<zlo>/<zhi>] [-I[<max_intens>|<low_i>/<high_i>] [%s] %s[-L[i][<gap>[<unit>]]] [-M] [-N[p|<dpi>]]\n", GMT_J_OPT, API->K_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s%s[-Q] [%s] [-S] [%s] [%s] [-W<scale>]\n", API->O_OPT, API->P_OPT, GMT_Rgeoz_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-Z<zfile>]\n\t%s[%s] [%s] [%s]\n\n", GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_p_OPT, GMT_t_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-B Set scale annotation interval and label. Use y-label to set unit label.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no annotation interval is set it is taken from the CPT.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Color palette file. If not set, stdin is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   By default all color changes are annotated (but see -B).  To use a subset,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   add an extra column to the CPT with a L, U, or B\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to annotate Lower, Upper, or Both color segment boundaries.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If a categorical CPT is given the -Li is set automatically.\n");
	gmt_refpoint_syntax (API->GMT, "D", "Specify position and dimensions of the scale bar [JBC].", GMT_ANCHOR_COLORBAR, 1);
	GMT_Message (API, GMT_TIME_NONE, "\t   For -DJ|j w/TC|BC|ML|MR the values for +w (85%% of map width),+h|v,+j,+o,+m have defaults.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   You can override any of these settings with these explicit modifiers:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +w<length>[/<width>] for the scale dimensions [Default width is 4%% of <length>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give negative <length> to reverse the positive direction along the scale bar.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +h for a horizontal scale [Default is vertical (+v)].\n");
	gmt_refpoint_syntax (API->GMT, "D", NULL, GMT_ANCHOR_COLORBAR, 2);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +e to add sidebar triangles for back- and foreground colors.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Specify b(ackground) or f(oreground) to get one only [Default is both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Optionally, append triangle height [Default is half the barwidth].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +m<code> to move selected annotations/labels/units on opposite side of colorscale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append any of a, l, or u to move the annotations, labels, or unit, respectively.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append c to plot vertical labels as column text.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n to draw rectangle with NaN color and label with <txt> [NaN].\n");
	gmt_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the scale.", 3);
	GMT_Message (API, GMT_TIME_NONE, "\t-G Truncate incoming CPT to be limited to the z-range <zlo>/<zhi>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To accept one of the incoming limits, set that limit to NaN.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Add illumination for +-<max_intens> or <low_i> to <high_i> [-1.0/1.0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, specify <lower>/<upper> intensity values.\n");
	GMT_Option (API, "J-Z,K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L For equal-sized color rectangles. -B interval cannot be used.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append i to annotate the interval range instead of lower/upper.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <gap> is appended, we separate each rectangle by <gap> units and center each\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   lower (z0) annotation on the rectangle.  Ignored if not a discrete CPT.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -I is used then each rectangle will have the illuminated constant color.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Force monochrome colorbar using gmt_M_yiq transformation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Control how color-scale is represented by PostScript.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append p to indicate a preference for using polygons, if possible;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   otherwise it indicates a preference for using colorimage, if possible.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default uses the method that produces the simplest PostScript code.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append preferred dots-per-inch for rasterization when colorimage is used [600].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Plot colorbar using logarithmic scale and annotate powers of 10 [Default is linear].\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Skip drawing color boundary lines on color scale [Default draws lines].\n");
	GMT_Option (API, "U,V,X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Give file with colorbar-width (in %s) per color entry.\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   By default, width of entry is scaled to color range,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i.e., z = 0-100 gives twice the width as z = 100-150.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Multiply all z-values in the CPT by <scale> [no change].\n");
	GMT_Option (API, "c,p");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   (Requires -R and -J for proper functioning).\n");
	GMT_Option (API, "t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSSCALE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psscale and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int n, j;
	bool auto_m = false;
	char string[GMT_LEN256] = {""}, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, *c = NULL;
	char txt_c[GMT_LEN256] = {""}, txt_d[GMT_LEN256] = {""}, txt_e[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files (should be 0) */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				GMT_Report (API, GMT_MSG_COMPAT, "-A option is deprecated; use -D modifier +m instead.\n");
				Ctrl->D.move = true;
				if (!opt->arg[0]) Ctrl->D.mmode = (PSSCALE_FLIP_ANNOT+PSSCALE_FLIP_LABEL);	/* Default is +mal */
				for (j = 0; opt->arg[j]; j++) {
					switch (opt->arg[j]) {
						case 'a': Ctrl->D.mmode |= PSSCALE_FLIP_ANNOT; break;
						case 'l': Ctrl->D.mmode |= PSSCALE_FLIP_LABEL; break;
						case 'c': Ctrl->D.mmode |= PSSCALE_FLIP_VERT;  break;
						case 'u': Ctrl->D.mmode |= PSSCALE_FLIP_UNIT;  break;
					}
				}
				break;
			case 'C':
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':
				Ctrl->D.active = true;
				gmt_M_str_free (Ctrl->D.opt);
				if (opt->arg[0]) Ctrl->D.opt = strdup (opt->arg);
				break;
			case 'E':
				GMT_Report (API, GMT_MSG_COMPAT, "The -E option is deprecated but is accepted.\n");
				GMT_Report (API, GMT_MSG_COMPAT, "For the current -D syntax you should use -D modifier +e instead.\n");
				GMT_Report (API, GMT_MSG_COMPAT, "Note you cannot mix new-style modifiers (+e) with the old-style -D option.\n");
				Ctrl->D.extend = true;
				if ((c = strchr (opt->arg, 'n')) != NULL) {	/* Got +n[<text>] */
					c--;	*c = 0; c += 2;
					Ctrl->D.etext = (*c) ? strdup (c) : strdup ("NaN");
					Ctrl->D.emode = 4;
					c -= 2;
				}
				j = 0;
				if (opt->arg[0] == 'b') {
					Ctrl->D.emode |= 1;
					j = 1;
				}
				else if (opt->arg[j] == 'f') {
					Ctrl->D.emode |= 2;
					j = 1;
				}
				if (opt->arg[j] == 'b') {
					Ctrl->D.emode |= 1;
					j++;
				}
				else if (opt->arg[j] == 'f') {
					Ctrl->D.emode |= 2;
					j++;
				}
				if (j == 0) Ctrl->D.emode |= 3;	/* No b|f added */
				if (opt->arg[j]) Ctrl->D.elength = gmt_M_to_inch (GMT, &opt->arg[j]);
				if (c) *c = '+';	/* Put back the + sign */
				break;
			case 'F':
				Ctrl->F.active = true;
				if (gmt_getpanel (GMT, opt->option, opt->arg, &(Ctrl->F.panel))) {
					gmt_mappanel_syntax (GMT, 'F', "Specify a rectangular panel behind the scale", 3);
					n_errors++;
				}
				break;
			case 'G':	/* truncate incoming CPT */
				Ctrl->G.active = true;
				j = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b);
				n_errors += gmt_M_check_condition (GMT, j < 2, "Syntax error -G option: Must specify z_low/z_high\n");
				if (!(txt_a[0] == 'N' || txt_a[0] == 'n') || !strcmp (txt_a, "-")) Ctrl->G.z_low = atof (txt_a);
				if (!(txt_b[0] == 'N' || txt_b[0] == 'n') || !strcmp (txt_b, "-")) Ctrl->G.z_high = atof (txt_b);
				n_errors += gmt_M_check_condition (GMT, gmt_M_is_dnan (Ctrl->G.z_low) && gmt_M_is_dnan (Ctrl->G.z_high), "Syntax error -G option: Both of z_low/z_high cannot be NaN\n");
				break;
			case 'I':
				Ctrl->I.active = true;
				if (opt->arg[0]) {
					j = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b);
					if (j == 1) {
						Ctrl->I.max = atof (txt_a);
						Ctrl->I.min = -Ctrl->I.max;
					}
					else {
						Ctrl->I.min = atof (txt_a);
						Ctrl->I.max = atof (txt_b);
					}
				}
				break;
			case 'L':
				Ctrl->L.active = true;
				j = 0;
				if (opt->arg[0] == 'i') Ctrl->L.interval = true, j = 1;
				if (opt->arg[j]) Ctrl->L.spacing = gmt_M_to_inch (GMT, &opt->arg[j]);
				break;
			case 'M':
				Ctrl->M.active = true;
				break;
			case 'N':
				Ctrl->N.active = true;
				/* Default will use image@600 dpi or polygons according to what is simplest */
				if (opt->arg[0] == 'p')	/* Preferentially use polygon fill if at all possible */
					Ctrl->N.mode = N_FAVOR_POLY;
				else {			/* Preferentially use image if at all possible */
					Ctrl->N.mode = N_FAVOR_IMAGE;
					if (opt->arg[0]) Ctrl->N.dpi = atof (opt->arg);
				}
				break;
			case 'Q':
				Ctrl->Q.active = true;
				break;
			case 'S':
				Ctrl->S.active = true;
				break;
			case 'T':
				if (gmt_M_compat_check (GMT, 5)) { /* Warn but process old -T */
					unsigned int pos = 0, ns = 0;
					double off[4] = {0.0, 0.0, 0.0, 0.0};
					char extra[GMT_LEN256] = {""}, p[GMT_LEN256] = {""};
					GMT_Report (API, GMT_MSG_COMPAT, "-T option is deprecated; use -F instead\n");
					/* We will build a -F compatible string and parse it */
					while (gmt_strtok (opt->arg, "+", &pos, p)) {
						switch (p[0]) {
							case 'l': off[XLO] = gmt_M_to_inch (GMT, &p[1]); ns++; break; /* Left nudge */
							case 'r': off[XHI] = gmt_M_to_inch (GMT, &p[1]); ns++; break; /* Right nudge */
							case 'b': off[YLO] = gmt_M_to_inch (GMT, &p[1]); ns++; break; /* Bottom nudge */
							case 't': off[YHI] = gmt_M_to_inch (GMT, &p[1]); ns++; break; /* Top nudge */
							case 'g': strcat (extra, "+"); strcat (extra, p); break; /* Fill */
							case 'p': strcat (extra, "+"); strcat (extra, p); break; /* Pen */
							default: n_errors++;	break;
						}
					}
					if (ns) {	/* Did specify specific nudging */
						sprintf (p, "+c%gi/%gi/%gi/%gi", off[XLO], off[XHI], off[YLO], off[YHI]);
						strcat (extra, p);
					}
					Ctrl->F.active = true;
					if (gmt_getpanel (GMT, opt->option, extra, &(Ctrl->F.panel))) {
						gmt_mappanel_syntax (GMT, 'F', "Specify a rectangular panel behind the scale", 3);
						n_errors++;
					}
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'Z':
				Ctrl->Z.active = true;
				Ctrl->Z.file = strdup (opt->arg);
				break;
			case 'W':	/* Dump out interpolated colors for debugging */
				Ctrl->W.active = true;
				Ctrl->W.scale = atof (opt->arg);
				break;
#ifdef DEBUG
			case 'd':	/* Dump out interpolated colors for debugging */
				dump = true;
				dump_int_val = (opt->arg[0]) ? atof (opt->arg) : 1.0;
				break;
#endif

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if ((Ctrl->D.refpoint = gmt_get_refpoint (GMT, Ctrl->D.opt, 'D')) == NULL)
		n_errors++;	/* Failed basic parsing */
	else if (Ctrl->D.refpoint->mode != GMT_REFPOINT_PLOT || strstr (Ctrl->D.refpoint->args, "+w")) {	/* New syntax: */
		/* Args are +w<length>[/<width>][+e[b|f][<length>]][+h|v][+j<justify>][+ma|c|l|u][+n[<txt>]][+o<dx>[/<dy>]] */
		double bar_offset[2];	/* Will be initialized in gmt_auto_offsets_for_colorbar */
		gmt_auto_offsets_for_colorbar (GMT, bar_offset, Ctrl->D.refpoint->justify);
		if (gmt_get_modifier (Ctrl->D.refpoint->args, 'j', string))
			Ctrl->D.justify = gmt_just_decode (GMT, string, PSL_NO_DEF);
		else {	/* With -Dj or -DJ, set default to reference (mirrored) justify point, else BL */
			Ctrl->D.justify = gmt_M_just_default (GMT, Ctrl->D.refpoint, PSL_BL);
			auto_m = true;
			switch (Ctrl->D.refpoint->justify) {	/* Autoset +h|v, +m, +o when placed centered on a side: Note: +h|v, +m, +o may overrule this later */
				case PSL_TC:
					Ctrl->D.mmode = (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST_FLIP) ? (PSSCALE_FLIP_ANNOT+PSSCALE_FLIP_LABEL) : 0;	/* +mal if outside */
					Ctrl->D.horizontal = true;
					Ctrl->D.off[GMT_Y] = (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST_FLIP) ? bar_offset[GMT_OUT] : bar_offset[GMT_IN];
					break;
				case PSL_BC:
					Ctrl->D.mmode = (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST) ? (PSSCALE_FLIP_ANNOT+PSSCALE_FLIP_LABEL) : 0;	/* +mal if inside */
					Ctrl->D.horizontal = true;
					Ctrl->D.off[GMT_Y] = (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST_FLIP) ? bar_offset[GMT_OUT] : bar_offset[GMT_IN];
					break;
				case PSL_ML:
					Ctrl->D.mmode = (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST_FLIP) ? (PSSCALE_FLIP_ANNOT+PSSCALE_FLIP_LABEL) : 0;	/* +mal if outside */
					Ctrl->D.horizontal = false;
					Ctrl->D.off[GMT_X] = (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST_FLIP) ? bar_offset[GMT_OUT] : bar_offset[GMT_IN];
					break;
				case PSL_MR:
					Ctrl->D.mmode = (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST) ? (PSSCALE_FLIP_ANNOT+PSSCALE_FLIP_LABEL) : 0;	/* +mal if inside */
					Ctrl->D.horizontal = false;
					Ctrl->D.off[GMT_X] = (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST_FLIP) ? bar_offset[GMT_OUT] : bar_offset[GMT_IN];
					break;
				default:
					auto_m = false;	/* None of the 4 above */
					break;
			}
		}
		if (gmt_validate_modifiers (GMT, Ctrl->D.refpoint->args, 'D', "ehjmnovw")) n_errors++;
		/* Required modifier +w */
		if (gmt_get_modifier (Ctrl->D.refpoint->args, 'w', string)) {
			if (string[(j = (int)strlen(string)-1)] == 'h') {	/* Be kind to those who forgot +h */
				string[j] = '\0';
				Ctrl->D.horizontal = true;
			}
			if ((n = gmt_get_pair (GMT, string, GMT_PAIR_DIM_NODUP, Ctrl->D.dim)) == 0)
				n_errors++;
			else if (n == 1)	/* Set height as 4% of width */
				Ctrl->D.dim[GMT_Y] = PSSCALE_W_SCALE * fabs (Ctrl->D.dim[GMT_X]);
		}
		/* Optional modifiers +e, +h, +j, +m, +n, +o, +v */
		if (gmt_get_modifier (Ctrl->D.refpoint->args, 'e', string)) {
			Ctrl->D.extend = true;
			if (strchr (string, 'b')) Ctrl->D.emode |= 1;
			if (strchr (string, 'f')) Ctrl->D.emode |= 2;
			if ((Ctrl->D.emode&3) == 0) Ctrl->D.emode |= 3;	/* No b|f added */
			j = 0; while (string[j] == 'b' || string[j] == 'f') j++;
			if (string[j]) Ctrl->D.elength = gmt_M_to_inch (GMT, &string[j]);
		}
		if (gmt_get_modifier (Ctrl->D.refpoint->args, 'h', string))
			Ctrl->D.horizontal = true;
		else if (gmt_get_modifier (Ctrl->D.refpoint->args, 'v', string))
			Ctrl->D.horizontal = false;
		if (gmt_get_modifier (Ctrl->D.refpoint->args, 'm', string)) {
			Ctrl->D.move = true;
			if (auto_m && !string[0]) {	/* Means flip might have been set and no arg, so invert it */
				switch (Ctrl->D.refpoint->justify) {	/* Autoset +h|v, +m, +o when placed centered on a side: Note: +h|v, +m, +o may overrule this later */
					case PSL_TC:
						Ctrl->D.mmode = (Ctrl->D.refpoint->mode != GMT_REFPOINT_JUST_FLIP) ? (PSSCALE_FLIP_ANNOT+PSSCALE_FLIP_LABEL) : 0;	/* +mal if outside */
						break;
					case PSL_BC:
						Ctrl->D.mmode = (Ctrl->D.refpoint->mode != GMT_REFPOINT_JUST) ? (PSSCALE_FLIP_ANNOT+PSSCALE_FLIP_LABEL) : 0;	/* +mal if inside */
						break;
					case PSL_ML:
						Ctrl->D.mmode = (Ctrl->D.refpoint->mode != GMT_REFPOINT_JUST_FLIP) ? (PSSCALE_FLIP_ANNOT+PSSCALE_FLIP_LABEL) : 0;	/* +mal if outside */
						break;
					case PSL_MR:
						Ctrl->D.mmode = (Ctrl->D.refpoint->mode != GMT_REFPOINT_JUST) ? (PSSCALE_FLIP_ANNOT+PSSCALE_FLIP_LABEL) : 0;	/* +mal if inside */
						break;
					default:
						break;
				}
			}
			else {
				if (!string[0]) Ctrl->D.mmode = (PSSCALE_FLIP_ANNOT+PSSCALE_FLIP_LABEL);	/* Default is +mal */
				for (j = 0; string[j]; j++) {
					switch (string[j]) {
						case 'a': Ctrl->D.mmode |= PSSCALE_FLIP_ANNOT; break;
						case 'l': Ctrl->D.mmode |= PSSCALE_FLIP_LABEL; break;
						case 'c': Ctrl->D.mmode |= PSSCALE_FLIP_VERT;  break;
						case 'u': Ctrl->D.mmode |= PSSCALE_FLIP_UNIT;  break;
					}
				}
			}
		}
		if (gmt_get_modifier (Ctrl->D.refpoint->args, 'n', string)) {
			Ctrl->D.etext = (string[0]) ? strdup (string) : strdup ("NaN");
			Ctrl->D.emode |= 4;
		}
		if (gmt_get_modifier (Ctrl->D.refpoint->args, 'o', string)) {
			if ((n = gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->D.off)) < 0) n_errors++;
		}
	}
	else {	/* Old-style option: args are <length>/<width>[h][/<justify>][/<dx>/<dy>]] */
		n = sscanf (Ctrl->D.refpoint->args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
		/* First deal with bar dimensions and horizontal vs vertical */
		j = (unsigned int)strlen (txt_b) - 1;
		if (txt_b[j] == 'h' || txt_b[j] == 'H') {	/* Want horizontal color bar */
			Ctrl->D.horizontal = true;
			txt_b[j] = 0;	/* Remove this to avoid unit confusion */
		}
		Ctrl->D.dim[GMT_X] = gmt_M_to_inch (GMT, txt_a);
		Ctrl->D.dim[GMT_Y]  = gmt_M_to_inch (GMT, txt_b);
		if (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST)	/* With -Dj, set default as the mirror to reference justify point */
			Ctrl->D.justify = gmt_flip_justify (GMT, Ctrl->D.refpoint->justify);
		else
			Ctrl->D.justify = (Ctrl->D.horizontal) ? PSL_TC : PSL_ML;	/* Old default justifications for non-Dj settings */
		/* Now deal with optional arguments, if any */
		switch (n) {
			case 3: Ctrl->D.justify = gmt_just_decode (GMT, txt_c, PSL_TC);	break;	/* Just got justification */
			case 4: Ctrl->D.off[GMT_X] = gmt_M_to_inch (GMT, txt_c); 	Ctrl->D.off[GMT_Y] = gmt_M_to_inch (GMT, txt_d); break;	/* Just got offsets */
			case 5: Ctrl->D.justify = gmt_just_decode (GMT, txt_c, PSL_TC);	Ctrl->D.off[GMT_X] = gmt_M_to_inch (GMT, txt_d); 	Ctrl->D.off[GMT_Y] = gmt_M_to_inch (GMT, txt_e); break;	/* Got both */
		}
	}
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Bar settings: justify = %d, dx = %g dy = %g\n", Ctrl->D.justify, Ctrl->D.off[GMT_X], Ctrl->D.off[GMT_Y]);

	/* Check that the options selected are mutually consistent */

	if (Ctrl->D.refpoint) {
		if (!(Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST_FLIP || Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST)) {	/* Only -DJ|j takes auto-width */
			n_errors += gmt_M_check_condition (GMT, fabs (Ctrl->D.dim[GMT_X]) < GMT_CONV4_LIMIT , "Syntax error -D option: scale length must be nonzero\n");
			n_errors += gmt_M_check_condition (GMT, Ctrl->D.dim[GMT_Y] <= 0.0, "Syntax error -D option: scale width must be positive\n");
		}
		if (Ctrl->D.refpoint->mode != GMT_REFPOINT_PLOT) {	/* Anything other than -Dx need -R -J; other cases don't */
			static char *kind = "gjJnx";	/* The five types of refpoint specifications */
			n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: -D%c requires the -R option\n", kind[Ctrl->D.refpoint->mode]);
			n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Syntax error: -D%c requires the -J option\n", kind[Ctrl->D.refpoint->mode]);
		}
	}
	n_errors += gmt_M_check_condition (GMT, n_files > 0, "Syntax error: No input files are allowed\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && GMT->current.map.frame.set, "Syntax error -L option: Cannot be used if -B option sets increments.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && Ctrl->N.dpi <= 0.0, "Syntax error -N option: The dpi must be > 0.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && !Ctrl->Z.file, "Syntax error -Z option: No file given\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->Z.file && gmt_access (GMT, Ctrl->Z.file, R_OK), "Syntax error -Z option: Cannot access file %s\n", Ctrl->Z.file);
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.scale == 0.0, "Syntax error -W option: Scale cannot be zero\n");

	if (!Ctrl->C.active && (c = gmt_get_current_cpt (API->GMT))) {
		Ctrl->C.active = true;	/* Select current CPT */
		gmt_M_str_free (c);
	}

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL double get_z (struct GMT_PALETTE *P, double x, double *width, unsigned int n) {
	unsigned int i = 0;
	double tmp;

	tmp = width[0];
	while (i < n && x > tmp) tmp += width[++i];
	if (i == n) return (P->data[P->n_colors-1].z_high);
	return (P->data[i].z_low + (x - tmp + width[i]) * (P->data[i].z_high - P->data[i].z_low) / width[i]);
}

GMT_LOCAL void fix_format (char *unit, char *format) {
	unsigned int i, j;
	char text[GMT_LEN64] = {""}, new_format[GMT_BUFSIZ] = {""};

	/* Check if annotation units should be added */

	if (unit && unit[0]) {	/* Must append the unit string */
		if (!strchr (unit, '%'))	/* No percent signs */
			strncpy (text, unit, GMT_LEN64-1);
		else {
			for (i = j = 0; i < strlen (unit); i++) {
				text[j++] = unit[i];
				if (unit[i] == '%') text[j++] = unit[i];
			}
			text[j] = 0;
		}
		if (text[0] == '-')	/* No space between annotation and unit */
			sprintf (new_format, "%s%s", format, &text[1]);
		else		/* 1 space between annotation and unit */
			sprintf (new_format, "%s %s", format, text);
		strcpy (format, new_format);
	}
}

void plot_cycle (struct GMT_CTRL *GMT, double x, double y, double width) {
	double vdim[PSL_MAX_DIMS], s = width / 0.1, p_width;
	struct GMT_SYMBOL S;
	struct GMT_FILL black;
	struct GMT_PEN pen;
	gmt_init_pen (GMT, &pen, 0.5);
	gmt_init_fill (GMT, &black, 0.0, 0.0, 0.0);	/* Default fill for points, if needed */
	gmt_init_vector_param (GMT, &S, false, false, NULL, false, NULL);	/* Update vector head parameters */
	gmt_M_memset (vdim, PSL_MAX_DIMS, double);
	p_width = (float)(s * pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
	pen.width = p_width * GMT->session.u2u[GMT_INCH][GMT_PT];
	vdim[0] = 0.9 * width;
	vdim[1] = 50.0;	vdim[2] = 210.0;
	vdim[3] = s * width, vdim[4] = s * width * tand (20.0), vdim[5] = p_width;
	vdim[6] = 0.75;
	vdim[7] = (double)(PSL_VEC_END|PSL_VEC_FILL);
	vdim[9] = (double)PSL_VEC_ARROW;
	gmt_setfill (GMT, &black, false);
	gmt_setpen (GMT, &pen);
	PSL_defpen (GMT->PSL, "PSL_vecheadpen", 0.0, "", 0, black.rgb);
	PSL_plotsymbol (GMT->PSL, x, y, vdim, PSL_MARC);
	vdim[1] = 230.0;	vdim[2] = 390.0;
	PSL_plotsymbol (GMT->PSL, x, y, vdim, PSL_MARC);
}

#define FONT_HEIGHT_PRIMARY (GMT->session.font[GMT->current.setting.font_annot[GMT_PRIMARY].id].height)

GMT_LOCAL bool letter_hangs_down (char *text) {
	/* Returns true if text contains one of the 4 lower-case letters that hangs down below the baseline */
	size_t k;
	for (k = 0; k < strlen (text); k++) if (strchr ("jpqy", text[k])) return true;
	return false;
}

EXTERN_MSC void gmt_xy_axis2 (struct GMT_CTRL *GMT, double x0, double y0, double length, double val0, double val1, struct GMT_PLOT_AXIS *A, bool below, bool annotate, unsigned side);

GMT_LOCAL void gmt_draw_colorbar (struct GMT_CTRL *GMT, struct PSSCALE_CTRL *Ctrl, struct GMT_PALETTE *P, double *z_width) {
	unsigned int i, ii, id, j, nb, ndec = 0, dec, depth, flip = Ctrl->D.mmode, l_justify, n_use_labels = 0;
	unsigned int Label_justify, form, cap, join, n_xpos, nx = 0, ny = 0, nm, barmem, k, justify;
	int this_just, p_val;
	bool reverse, all = true, use_image, center = false, const_width = true, do_annot, use_labels, cpt_auto_fmt = true;
	bool B_set = GMT->current.map.frame.draw, skip_lines = Ctrl->S.active, need_image;
	char format[GMT_LEN256] = {""}, text[GMT_LEN256] = {""}, test[GMT_LEN256] = {""}, unit[GMT_LEN256] = {""}, label[GMT_LEN256] = {""}, endash;
	static char *method[2] = {"polygons", "colorimage"};
	unsigned char *bar = NULL, *tmp = NULL;
	double hor_annot_width, annot_off, label_off = 0.0, len, len2, size, x0, x1, dx, xx, dir, y_base, y_annot, y_label, xd = 0.0, yd = 0.0, xt = 0.0;
	double z = 0.0, xleft, xright, inc_i, inc_j, start_val, stop_val, nan_off = 0.0, rgb[4], rrggbb[4], prev_del_z, this_del_z = 0.0;
	double length = Ctrl->D.dim[GMT_X], width = Ctrl->D.dim[GMT_Y], gap = Ctrl->L.spacing, t_len, max_intens[2], xp[4], yp[4];
	double *xpos = NULL, elength[2] = {0.0, 0.0};
	struct GMT_FILL *f = NULL;
	struct GMT_PLOT_AXIS *A = NULL;
	struct GMT_MAP_PANEL *panel = Ctrl->F.panel;	/* Shorthand */
	struct PSL_CTRL *PSL = GMT->PSL;
#ifdef DEBUG
	unsigned int dump_k_val = 0;
#endif

	max_intens[0] = Ctrl->I.min;
	max_intens[1] = Ctrl->I.max;

	GMT->current.setting.map_annot_offset[GMT_PRIMARY] = fabs (GMT->current.setting.map_annot_offset[GMT_PRIMARY]);	/* No 'inside' annotations allowed in colorbar */
	cap  = PSL->internal.line_cap;
	join = PSL->internal.line_join;

	/* Temporarily change to miter join so boxes and end triangles have near corners */
	PSL_setlinejoin (PSL, PSL_MITER_JOIN);

#ifdef DEBUG
	if (!Ctrl->I.active) dump = false;	/* Must be run with -I */
	if (dump) {
		ny = urint (width * Ctrl->N.dpi);
		inc_j = (ny > 1) ? (max_intens[1] - max_intens[0]) / (ny - 1) : 0.0;
		dump_k_val = ny - urint ((dump_int_val - max_intens[0])/inc_j) - 1;
		fprintf (stderr, "#Produced with dump_int_val = %g, dump_k_val = %u of %u\n", dump_int_val, dump_k_val, ny);
		fprintf (stderr, "#z\tred\tgreen\tblue\tr_int\tg_int\tb_int\tps_r\tps_g\tps_b\n");
	}
#endif

	/* Find max decimals needed */

	start_val = P->data[0].z_low;
	stop_val  = P->data[P->n_colors-1].z_high;
	if (B_set) {	/* Let the general -B machinery do the annotation labeling */
		A = &GMT->current.map.frame.axis[GMT_X];
		if (A->item[GMT_ANNOT_UPPER].generated) A->item[GMT_ANNOT_UPPER].interval = 0.0;	/* Reset annot so we can redo via automagic */
		if (A->item[GMT_TICK_UPPER].generated)  A->item[GMT_TICK_UPPER].interval  = 0.0;	/* Reset frame so we can redo via automagic */
		if (A->item[GMT_GRID_UPPER].generated)  A->item[GMT_GRID_UPPER].interval  = 0.0;	/* Reset grid  so we can redo via automagic */
		gmt_auto_frame_interval (GMT, GMT_X, GMT_ANNOT_UPPER);
		if (Ctrl->Q.active) {	/* Must set original start/stop values for axis */
			start_val = pow (10.0, P->data[0].z_low);
			stop_val  = pow (10.0, P->data[P->n_colors-1].z_high);
		}
		ndec = gmt_get_format (GMT, GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval, GMT->current.map.frame.axis[GMT_X].unit, GMT->current.map.frame.axis[GMT_X].prefix, format);
	}
	else {	/* Do annotations explicitly, one by one.  Try automatic guessing of decimals if equidistant cpt */
		bool const_interval = true, exp_notation = false;
		for (i = 0; i < P->n_colors; i++) {
			if (P->data[i].label) n_use_labels++;
			if (P->data[i].annot & 1) {
				z = P->data[i].z_low;
				if ((dec = gmt_get_format (GMT, z, NULL, NULL, text)) > ndec) {
					strncpy (format, text, GMT_LEN256-1);
					ndec = dec;
				}
			}
			if (P->data[i].annot & 2) {
				z = P->data[i].z_high;
				if ((dec = gmt_get_format (GMT, z, NULL, NULL, text)) > ndec) {
					strncpy (format, text, GMT_LEN256-1);
					ndec = dec;
				}
			}
			if (format[0] && !exp_notation) {
				sprintf (text, format, z);
				if (strchr (text, 'e')) exp_notation = true;	/* Got exponential notation in at least one place */
			}
			prev_del_z = this_del_z;
			this_del_z = P->data[i].z_high - P->data[i].z_low;
			if (i && !doubleAlmostEqualZero (this_del_z, prev_del_z)) const_interval = false;
			if (P->data[i].annot) all = false;
		}
		if (!const_interval) {	/* May have to abandon automatic format guessing */
			if (exp_notation && !strchr (GMT->current.setting.format_float_map, 'e')) {	/* Unwanted exponential notation: Abandon automatic format detection and use FORMAT_FLOAT_MAP */
				cpt_auto_fmt = false;
				ndec = 0;
			}
		}
	}
	if (Ctrl->L.active && n_use_labels == P->n_colors)
		all = use_labels = true;	/* Only use optional text labels for equal length scales */
	else
		use_labels = false;

	/* Check if steps in color map have constant width */
	for (i = 1; i < P->n_colors && const_width; i++)
		if (fabs (z_width[i] - z_width[0]) > GMT_CONV4_LIMIT) const_width = false;

	if (ndec == 0) {	/* Not -B and no decimals are needed */
		strncpy (format, GMT->current.setting.format_float_map, GMT_LEN256-1);
		fix_format (GMT->current.map.frame.axis[GMT_X].unit, format);	/* Add units if needed */
	}

	len = GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER];	/* +ve means draw on the outside of bar */
	len2 = GMT->current.setting.map_tick_length[GMT_TICK_UPPER];
	xleft = x0 = 0.0;

	reverse = (length < 0.0);
	length = fabs (length);
	xright = length;
	if (Ctrl->N.mode == N_FAVOR_IMAGE)	/* favor image if possible */
		use_image = (!P->has_pattern && gap <= 0.0);
	else if (Ctrl->N.mode == N_FAVOR_POLY)	/* favor polygon if possible */
		use_image = P->is_continuous;
	else	/* Auto mode */
		use_image = (!P->has_pattern && gap <= 0.0 && (Ctrl->L.active || const_width || P->is_continuous));
	/* So if CPT has pattern AND continuous color sections then use_image is false but we still need to do an image later */
	need_image = (P->has_pattern && P->is_continuous);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Color bar will be plotted using %s\n", method[use_image]);
	
	if (Ctrl->D.emode & (reverse+1)) elength[XLO] =  Ctrl->D.elength;
	if (Ctrl->D.emode & (2-reverse)) elength[XHI] =  Ctrl->D.elength;
	if (!strncmp (PSL->init.encoding, "Standard", 8U))
		endash = 0261;	/* endash code in Standard[+] charset */
	else if (!strncmp (PSL->init.encoding, "ISOLatin1+", 10U))
		endash = 0035;	/* endash code in ISOLatin1 charset */
	else
		endash = '-';	/* Use hyphen */

	if ((gap >= 0.0 || Ctrl->L.interval) && !P->is_continuous) {	/* Want to center annotations for discrete colortable, using lower z0 value */
		center = (Ctrl->L.interval || gap >= 0.0);
		if (gap > 0.0) skip_lines = true;
		gap *= 0.5;
		if (Ctrl->L.interval) {
			sprintf (text, "%s%c%s", format, endash, format);
			strncpy (format, text, GMT_LEN256-1);
		}
	}
	if (gap < 0.0) gap = 0.0;

	if (use_image || Ctrl->I.active || need_image) {	/* Make bitimage for colorbar using Ctrl->N.dpi */
		bool resample, skip;
		int index;
		if (Ctrl->N.mode == N_FAVOR_IMAGE) {	/* Honor the given image dpi */
			nx = (!use_image && gap > 0.0) ? P->n_colors : urint (length * Ctrl->N.dpi);
			ny = urint (width * Ctrl->N.dpi);
			resample = true;
		}
		else {	/* Do the smallest possible image */
			resample = P->is_continuous;
			nx = (P->is_continuous) ? urint (length * Ctrl->N.dpi) : P->n_colors;
			ny = (Ctrl->I.active) ? urint (width * Ctrl->N.dpi) : 1;
		}
		nm = nx * ny;
		inc_i = (nx) ? length / nx : 0.0;
		inc_j = (ny > 1) ? (max_intens[1] - max_intens[0]) / (ny - 1) : 0.0;
		barmem = (Ctrl->M.active || P->is_gray) ? nm : 3 * nm;
		bar = gmt_M_memory (GMT, NULL, barmem, unsigned char);

		/* Load bar image */

		for (i = 0; i < nx; i++) {
			z = (resample) ? get_z (P, (i+0.5) * inc_i, z_width, P->n_colors) : P->data[i].z_low;
			index = gmt_get_rgb_from_z (GMT, P, z, rrggbb);
			skip = gmt_M_skip_cptslice (P, index);	/* Don't what intensity shading if slice is to be skipped */
			ii = (reverse) ? nx - i - 1 : i;
			for (j = 0; j < ny; j++) {
				gmt_M_rgb_copy (rgb, rrggbb);
				k = j * nx + ii;
				if (Ctrl->I.active && !skip) gmt_illuminate (GMT, max_intens[1] - j * inc_j, rgb);
				if (P->is_gray)	/* All gray, pick red */
					bar[k] = gmt_M_u255 (rgb[0]);
				else if (Ctrl->M.active)	/* Convert to gray using the gmt_M_yiq transformation */
					bar[k] = gmt_M_u255 (gmt_M_yiq (rgb));
				else {
					k *= 3;
					bar[k++] = gmt_M_u255 (rgb[0]);
					bar[k++] = gmt_M_u255 (rgb[1]);
					bar[k++] = gmt_M_u255 (rgb[2]);
#ifdef DEBUG
					if (dump && j == dump_k_val) fprintf (stderr, "%g\t%g\t%g\t%g\t%g\t%g\t%g\t%d\t%d\t%d\n",
						z, rrggbb[0], rrggbb[1], rrggbb[2], rgb[0], rgb[1], rgb[2], bar[k-3], bar[k-2], bar[k-1]);
#endif
				}
			}
		}
	}

	/* Defeat the auto-repeat of axis info */
	if (!strcmp (GMT->current.map.frame.axis[GMT_X].label, GMT->current.map.frame.axis[GMT_Y].label)) GMT->current.map.frame.axis[GMT_Y].label[0] = 0;
	if (Ctrl->F.active) {	/* Place rectangle behind the color bar */
		double x_center, y_center, bar_tick_len, u_off = 0.0, v_off = 0.0, h_off = 0.0, dim[4] = {0.0, 0.0, 0.0, 0.0};

		/* Must guesstimate the width of the largest horizontal annotation */
		sprintf (text, "%ld", lrint (floor (P->data[0].z_low)));
		sprintf (test, "%ld", lrint (ceil (center ? P->data[P->n_colors-1].z_low : P->data[P->n_colors-1].z_high)));
		hor_annot_width = ((MAX ((int)strlen (text), (int)strlen (test)) + ndec) * GMT_DEC_WIDTH +
			((ndec > 0) ? GMT_PER_WIDTH : 0.0))
			* GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
		bar_tick_len = fabs (GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER]);	/* Length of tickmarks */
		if (Ctrl->D.horizontal) {	/* Determine center and dimensions of horizontal background rectangle */
			/* Determine dimensions */
			annot_off = MAX (0.0, GMT->current.setting.map_annot_offset[GMT_PRIMARY]);	/* Allow for space between bar and annotations */
			/* Extend y clearance by tick length */
			annot_off += bar_tick_len;
			/* Extend y clearance by annotation height */
			annot_off += GMT_LET_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
			/* If a label then add space for offset and label height */
			if (GMT->current.map.frame.axis[GMT_X].label[0]) {
				label_off = 1.0;	/* 1-letter height */
				if (!(flip & PSSCALE_FLIP_LABEL) && letter_hangs_down (GMT->current.map.frame.axis[GMT_X].label))
					label_off += 0.3;	/* Add 30% hang below baseline */
				label_off *= GMT_LET_HEIGHT * GMT->current.setting.font_label.size / PSL_POINTS_PER_INCH;	/* Scale to inches */
				label_off += MAX (0.0, GMT->current.setting.map_label_offset);	/* Add offset */
			}
			/* If a unit then add space on the right to deal with the unit */
			if (GMT->current.map.frame.axis[GMT_Y].label[0]) {
				/* u_off is width of the label placed on the right side , while v_off, if > 0, is the extra height of the label beyond the width of the bar */
				size_t ylen = strlen (GMT->current.map.frame.axis[GMT_Y].label);
				if (strchr (GMT->current.map.frame.axis[GMT_Y].label, '\\')) ylen = (ylen > 3) ? ylen - 3 : 0;
				u_off = MAX (0.0, GMT->current.setting.map_annot_offset[GMT_PRIMARY]) + (0.5+ylen) * GMT_LET_WIDTH * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
				v_off = 0.5 * (GMT_LET_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH - width);
			}
			/* Adjust clearances for the panel */
			if (flip & PSSCALE_FLIP_ANNOT) dim[YHI] += annot_off; else dim[YLO] += annot_off;
			if (flip & PSSCALE_FLIP_LABEL) dim[YHI] += label_off; else dim[YLO] += label_off;
			dim[YHI] += 0.5 * GMT->current.setting.map_frame_pen.width / PSL_POINTS_PER_INCH;
			if (v_off > dim[YHI]) dim[YHI] = v_off;	/* Y-label is higher than top of bar */
			if (flip & PSSCALE_FLIP_UNIT) {	/* The y-label is on the left */
				dim[XLO] += MAX (u_off, 0.5*hor_annot_width);	/* Half the x-label may extend outside */
				dim[XHI] += 0.5*hor_annot_width;
			}
			else {	/* The y-label is on the right */
				dim[XLO] += 0.5*hor_annot_width;
				dim[XHI] += MAX (u_off, 0.5*hor_annot_width);	/* Half the x-label may extend outside */
			}
			/* Adjust width if there are +e extensions */
			dim[XLO] += elength[XLO];
			dim[XHI] += elength[XHI];
			/* Adjust if there is +n NaNmarker */
			if (Ctrl->D.emode & 4)	/* Add NaN rectangle on left side */
				dim[XLO] += 2.0 * Ctrl->D.elength + gap + fabs (GMT->current.setting.map_annot_offset[GMT_PRIMARY]) + 3.25 * GMT_LET_WIDTH * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
			
			x_center = 0.5 * length + 0.5 * (dim[XHI] - dim[XLO]); y_center = 0.5 * width + 0.5 * (dim[YHI] - dim[YLO]);
			panel->width = length + dim[XHI] + dim[XLO];	panel->height = width + dim[YHI] + dim[YLO];
		}
		else {	/* Determine center and dimensions of vertical background rectangle */
			if (GMT->current.setting.map_annot_ortho[0] == '\0') {	/* Annotations are parallel to bar, reset width */
				h_off = 0.5 * hor_annot_width;
				hor_annot_width = GMT_LET_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
			}
			else
				h_off = 0.5 * GMT_LET_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
			/* Determine dimensions */
			annot_off = MAX (0.0, GMT->current.setting.map_annot_offset[GMT_PRIMARY]);	/* Allow for space between bar and annotations */
			/* Extend x clearance by tick length */
			annot_off += bar_tick_len;
			/* Extend x clearance by annotation width */
			annot_off += hor_annot_width;
			/* Increase width if there is a label */
			if (GMT->current.map.frame.axis[GMT_X].label[0])
				label_off = MAX (0.0, GMT->current.setting.map_label_offset) + GMT->current.setting.font_label.size / PSL_POINTS_PER_INCH;
			/* If a unit then consider if its width exceeds the bar width; then use half the excess to adjust center and width of box, and its height to adjust the height of box */
			if (GMT->current.map.frame.axis[GMT_Y].label[0]) {
				/* u_off is ~half-width of the label placed on top of the vertical bar, while v_off is the extra height needed to accommodate the label */
				size_t ylen = strlen (GMT->current.map.frame.axis[GMT_Y].label);
				if (strchr (GMT->current.map.frame.axis[GMT_Y].label, '\\')) ylen = (ylen > 3) ? ylen - 3 : 0;
				u_off = 0.5 * MAX (0.0, 1.3*ylen * GMT_LET_WIDTH * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH - width);
				v_off = MAX (0.0, GMT->current.setting.map_annot_offset[GMT_PRIMARY]) + GMT->current.setting.font_annot[GMT_PRIMARY].size * GMT_LET_HEIGHT / PSL_POINTS_PER_INCH;
			}
			/* Adjust clearances for the panel */
			if (flip & PSSCALE_FLIP_LABEL) dim[XLO] += label_off; else dim[XHI] += label_off;
			if (flip & PSSCALE_FLIP_ANNOT) dim[XLO] += annot_off; else dim[XHI] += annot_off;
			if (u_off > dim[XLO]) dim[XLO] = u_off;
			if (u_off > dim[XHI]) dim[XHI] = u_off;
			if (flip & PSSCALE_FLIP_UNIT) {	/* The y-label is on the bottom */
				dim[YHI] += h_off;
				dim[YLO] += MAX (v_off, h_off);
			}
			else {	/* The y-label is on the top */
				dim[YLO] += h_off;
				dim[YHI] += MAX (v_off, h_off);
			}
			/* Adjust width if there are +e extensions */
			dim[YLO] += elength[XLO];
			dim[YHI] += elength[XHI];
			/* Adjust if there is +n NaNmarker */
			if (Ctrl->D.emode & 4)	/* Add NaN rectangle on left side */
				dim[YLO] += 2.0 * Ctrl->D.elength + gap + fabs (GMT->current.setting.map_annot_offset[GMT_PRIMARY]) + 1.75 * GMT_LET_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
			y_center = 0.5 * length + 0.5 * (dim[YHI] - dim[YLO]); x_center = 0.5 * width + 0.5 * (dim[XHI] - dim[XLO]);
			panel->width = width + dim[XHI] + dim[XLO];	panel->height = length + dim[YHI] + dim[YLO];
		}
		gmt_draw_map_panel (GMT, x_center, y_center, 3U, panel);
	}
	gmt_setpen (GMT, &GMT->current.setting.map_frame_pen);

	unit[0] = label[0] = 0;
	/* Save label and unit, because we are going to switch them off in GMT->current.map.frame and do it ourselves */
	strncpy (label, GMT->current.map.frame.axis[GMT_X].label, GMT_LEN256-1);
	strncpy (unit, GMT->current.map.frame.axis[GMT_Y].label, GMT_LEN256-1);
	GMT->current.map.frame.axis[GMT_X].label[0] = GMT->current.map.frame.axis[GMT_Y].label[1] = 0;

	if (flip & PSSCALE_FLIP_ANNOT) {	/* Place annotations on the opposite side */
		justify = l_justify = (Ctrl->D.horizontal) ? PSL_BC : PSL_MR;
		y_base = width;
		dir = 1.0;
	}
	else {	/* Leave them on the default side */
		justify = (Ctrl->D.horizontal) ? PSL_TC : PSL_MR;
		l_justify = (Ctrl->D.horizontal) ? PSL_TC : PSL_ML;
		y_base = 0.0;
		dir = -1.0;
	}
	if (flip & PSSCALE_FLIP_LABEL) {	/* Place label on the opposite side */
		Label_justify = PSL_BC;
		y_label = width + GMT->current.setting.map_label_offset;
	}
	else {	/* Leave label on the default side */
		Label_justify = PSL_TC;
		y_label = -GMT->current.setting.map_label_offset;
	}

	if (Ctrl->D.emode) {	/* Adjustment to triangle base coordinates so pen abuts perfectly with bar */
		double theta, s, c, w = GMT->current.setting.map_frame_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH];
		theta = atan (2.0 * Ctrl->D.elength / width);	/* triangle base angle in radians */
		sincos (theta, &s, &c);
		xd = 0.5 * w * (c - 1.0);
		yd = 0.5 * w * (s - 1.0);
		xt = yd * 2.0 * Ctrl->D.elength / width;	/* Shift of triangle peak x-pos to maintain original angle theta */
	}
	/* Set up array with x-coordinates for the CPT z_lows + final z_high */
	n_xpos = P->n_colors + 1;
	xpos = gmt_M_memory (GMT, NULL, n_xpos, double);
	for (i = 0, x1 = xleft; i < P->n_colors; i++) {		/* For all z_low coordinates */
		xpos[i] = (reverse) ? xright - x1 : x1;	/* x-coordinate of z_low boundary */
		x1 += z_width[i];	/* Step to next boundary */
	}
	xpos[P->n_colors] = (reverse) ? xleft : xright;

	/* Current point (0,0) is now at lower left location of bar */

	depth = (Ctrl->M.active || P->is_gray) ? 8 : 24;
	if (Ctrl->D.horizontal) {
		if (use_image)	/* Must plot entire bar as image */
			PSL_plotcolorimage (PSL, 0.0, 0.0, length, width, PSL_BL, bar, nx, ny, depth);
		else if (need_image) {	/* Some parts require image, we overwrite where patterns are needed */
			PSL_plotcolorimage (PSL, 0.0, 0.0, length, width, PSL_BL, bar, nx, ny, depth);
			x0 = x1 = 0.0;
			for (i = 0; i < P->n_colors; i++) {
				ii = (reverse) ? P->n_colors - i - 1 : i;
				x1 += z_width[ii];
				if (P->data[ii].skip || (f = P->data[ii].fill) == NULL) {	/* Do not paint this slice at all */
					x0 = x1;
					continue;
				}
				gmt_setfill (GMT, f, center);	/* Set fill and paint box */
				PSL_plotbox (PSL, x0+gap, 0.0, x1-gap, width);
				x0 = x1;
			}
		}
		else {			/* Plot all slices as rectangles */
			x0 = x1 = 0.0;
			for (i = 0; i < P->n_colors; i++) {
				ii = (reverse) ? P->n_colors - i - 1 : i;
				x1 += z_width[ii];
				if (P->data[ii].skip) {	/* Do not paint this slice at all */
					x0 = x1;
					continue;
				}
				if ((f = P->data[ii].fill) != NULL)	/* Using pattern fills */
					gmt_setfill (GMT, f, center);
				else if (Ctrl->I.active) {
					nb = (P->is_gray || Ctrl->M.active) ? 1 : 3;
					tmp = gmt_M_memory (GMT, NULL, ny*nb, unsigned char);
					for (j = 0, k = i*nb; j < ny*nb; k+=(nx-1)*nb) {
						tmp[j++] = bar[k++];
						tmp[j++] = bar[k++];
						tmp[j++] = bar[k++];
					}
					PSL_plotcolorimage (PSL, x0 + gap, 0.0, z_width[ii] - 2*gap, width, PSL_BL, tmp, 1, ny, depth);
					gmt_M_free (GMT, tmp);
					PSL_setfill (PSL, GMT->session.no_rgb, center);
				}
				else {
					gmt_M_rgb_copy (rgb, P->data[ii].rgb_low);
					if (Ctrl->I.active) gmt_illuminate (GMT, max_intens[1], rgb);
					if (Ctrl->M.active) rgb[0] = rgb[1] = rgb[2] = gmt_M_yiq (rgb);
					PSL_setfill (PSL, rgb, center);
				}
				PSL_plotbox (PSL, x0+gap, 0.0, x1-gap, width);
				x0 = x1;
			}
		}

		annot_off = ((len > 0.0 && !center) ? len : 0.0) + GMT->current.setting.map_annot_offset[GMT_PRIMARY];
		label_off = annot_off + GMT_LET_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size * GMT->session.u2u[GMT_PT][GMT_INCH] + GMT->current.setting.map_label_offset;
		y_annot = y_base + dir * annot_off;
		if ((flip & PSSCALE_FLIP_ANNOT) == (flip & PSSCALE_FLIP_LABEL) / 2) y_label = y_base + dir * label_off;

		PSL_setlinecap (PSL, PSL_BUTT_CAP);	/* Butt cap required for outline of triangle */

		if (Ctrl->D.emode & (reverse + 1)) {	/* Add color triangle on left side */
			xp[0] = xp[2] = xleft - gap;	xp[1] = xleft - gap - Ctrl->D.elength;
			yp[0] = width - yd;	yp[2] = yd;	yp[1] = 0.5 * width;
			for (i = 0; i < 3; i++) xp[i] += xd;
			xp[1] += xt;
			id = (reverse) ? GMT_FGD : GMT_BGD;
			if ((f = P->bfn[id].fill) != NULL)
				gmt_setfill (GMT, f, false);
			else {
				gmt_M_rgb_copy (rgb, P->bfn[id].rgb);
				if (Ctrl->M.active) rgb[0] = rgb[1] = rgb[2] = gmt_M_yiq (rgb);
				PSL_setfill (PSL, rgb, false);
			}
			PSL_plotpolygon (PSL, xp, yp, 3);
			PSL_plotline (PSL, xp, yp, 3, PSL_MOVE|PSL_STROKE);
			nan_off = Ctrl->D.elength - xd;	/* Must make space for the triangle */
		}
		if (Ctrl->D.emode & 4) {	/* Add NaN rectangle on left side */
			nan_off += Ctrl->D.elength;
			xp[0] = xp[1] = xleft - gap;	xp[2] = xp[3] = xp[0] - MAX (width, Ctrl->D.elength);
			for (i = 0; i < 4; i++) xp[i] -= nan_off;
			yp[0] = yp[3] = width;	yp[1] = yp[2] = 0.0;
			if ((f = P->bfn[GMT_NAN].fill) != NULL)
				gmt_setfill (GMT, f, true);
			else {
				gmt_M_rgb_copy (rgb, P->bfn[GMT_NAN].rgb);
				if (Ctrl->M.active) rgb[0] = rgb[1] = rgb[2] = gmt_M_yiq (rgb);
				PSL_setfill (PSL, rgb, true);
			}
			PSL_plotpolygon (PSL, xp, yp, 4);
			if (Ctrl->D.etext) PSL_plottext (PSL, xp[2] - fabs (GMT->current.setting.map_annot_offset[GMT_PRIMARY]), 0.5 * width, GMT->current.setting.font_annot[GMT_PRIMARY].size, Ctrl->D.etext, 0.0, PSL_MR, 0);
		}
		if (Ctrl->D.emode & (2 - reverse)) {	/* Add color triangle on right side */
			xp[0] = xp[2] = xright + gap;	xp[1] = xp[0] + Ctrl->D.elength;
			yp[0] = width - yd;	yp[2] = yd;	yp[1] = 0.5 * width;
			for (i = 0; i < 3; i++) xp[i] -= xd;
			xp[1] -= xt;
			id = (reverse) ? GMT_BGD : GMT_FGD;
			if ((f = P->bfn[id].fill) != NULL)
				gmt_setfill (GMT, f, false);
			else {
				gmt_M_rgb_copy (rgb, P->bfn[id].rgb);
				if (Ctrl->M.active) rgb[0] = rgb[1] = rgb[2] = gmt_M_yiq (rgb);
				PSL_setfill (PSL, rgb, false);
			}
			PSL_plotpolygon (PSL, xp, yp, 3);
			PSL_plotline (PSL, xp, yp, 3, PSL_MOVE|PSL_STROKE);
		}

		PSL_setlinecap (PSL, PSL_SQUARE_CAP);	/* Square cap required for box of scale bar */

		if (gap == 0.0) {
			if ((flip & PSSCALE_FLIP_ANNOT) || !B_set) PSL_plotsegment (PSL, xleft, 0.0, xleft + length, 0.0);
			if (!(flip & PSSCALE_FLIP_ANNOT) || !B_set) PSL_plotsegment (PSL, xleft, width, xleft + length, width);
			PSL_plotsegment (PSL, xleft, 0.0, xleft, width);
			PSL_plotsegment (PSL, xright, 0.0, xright, width);
		}

		if (B_set) {	/* Used -B */
			gmt_xy_axis2 (GMT, xleft, y_base, length, start_val, stop_val, A, !(flip & PSSCALE_FLIP_ANNOT), GMT->current.map.frame.side[flip & PSSCALE_FLIP_ANNOT ? N_SIDE : S_SIDE] & PSSCALE_FLIP_LABEL, GMT->current.map.frame.side[flip & PSSCALE_FLIP_ANNOT ? N_SIDE : S_SIDE]);
			if (A->item[GMT_GRID_UPPER].active) {
				dx = gmtlib_get_map_interval (GMT, &A->item[GMT_GRID_UPPER]);
				gmt_setpen (GMT, &GMT->current.setting.map_grid_pen[GMT_PRIMARY]);
				if (A->type == GMT_TIME)
					plot_timex_grid (GMT, PSL, P->data[0].z_low, P->data[P->n_colors-1].z_high, 0.0, width, GMT_GRID_UPPER);
				else
					gmt_linearx_grid (GMT, PSL, P->data[0].z_low, P->data[P->n_colors-1].z_high, 0.0, width, dx);
			}
		}
		else {	/* When no -B we annotate every CPT bound which may be non-equidistant, hence this code (i.e., we cannot fake a call to -B) */

			if (!skip_lines) {	/* First draw gridlines, unless skip_lines is true */
				gmt_setpen (GMT, &GMT->current.setting.map_grid_pen[GMT_PRIMARY]);
				for (i = 0; i < n_xpos; i++)
					PSL_plotsegment (PSL, xpos[i], 0.0, xpos[i], width);
			}

			/* Then draw annotation and frame tickmarks */

			gmt_setpen (GMT, &GMT->current.setting.map_tick_pen[GMT_PRIMARY]);

			if (!center) {
				for (i = 0; i < P->n_colors; i++) {		/* For all z_low coordinates */
					t_len = (all || (P->data[i].annot & 1)) ? dir * len : dir * len2;	/* Annot or frame length */
					PSL_plotsegment (PSL, xpos[i], y_base, xpos[i], y_base+t_len);
				}
				if (!use_labels) {	/* Finally do last slice z_high boundary */
					t_len = (all || (P->data[P->n_colors-1].annot & 2)) ? dir * len : dir * len2;	/* Annot or frame length */
					PSL_plotsegment (PSL, xpos[P->n_colors], y_base, xpos[P->n_colors], y_base+t_len);
				}
			}

			/* Now we place annotations */

			PSL_settextmode (PSL, PSL_TXTMODE_MINUS);	/* Replace hyphens with minus signs */
			form = gmt_setfont (GMT, &GMT->current.setting.font_annot[GMT_PRIMARY]);
			x1 = xleft;
			if (center) x1 += 0.5 * z_width[0];	/* Can use z_width[0] since -L forces all widths to be equal */

			for (i = 0; i < P->n_colors; i++) {
				xx = (reverse) ? xright - x1 : x1;
				if (all || (P->data[i].annot & 1)) {	/* Annotate this */
					this_just = justify;
					do_annot = true;
					if (use_labels && P->data[i].label) {
						strncpy (text, P->data[i].label, GMT_LEN256-1);
						this_just = l_justify;
					}
					else if (center && Ctrl->L.interval)
						sprintf (text, format, P->data[i].z_low, P->data[i].z_high);
					else if (Ctrl->Q.active) {
						p_val = irint (P->data[i].z_low);
						if (doubleAlmostEqualZero (P->data[i].z_low, (double)p_val))
							sprintf (text, "10@+%d@+", p_val);
						else
							do_annot = false;
					}
					else
						gmt_sprintf_float (GMT, text, format, P->data[i].z_low);
					if (do_annot) PSL_plottext (PSL, xx, y_annot, GMT->current.setting.font_annot[GMT_PRIMARY].size, text, 0.0, -this_just, form);
				}
				x1 += z_width[i];
			}
			if (!center && !use_labels) {
				i = P->n_colors-1;
				if (all || (P->data[i].annot & 2)) {
					this_just = justify;
					do_annot = true;
					if (Ctrl->Q.active) {
						p_val = irint (P->data[i].z_high);
						if (doubleAlmostEqualZero (P->data[i].z_high, (double)p_val))
							sprintf (text, "10@+%d@+", p_val);
						else
							do_annot = false;
					}
					else
						gmt_sprintf_float (GMT, text, format, P->data[i].z_high);
					if (do_annot) PSL_plottext (PSL, xpos[P->n_colors], y_annot, GMT->current.setting.font_annot[GMT_PRIMARY].size, text, 0.0, -this_just, form);
				}
			}
			PSL_settextmode (PSL, PSL_TXTMODE_HYPHEN);	/* Back to leave as is */
		}

		if (label[0]) {	/* Add label */
			form = gmt_setfont (GMT, &GMT->current.setting.font_label);
			PSL_plottext (PSL, xleft + 0.5 * length, y_label, GMT->current.setting.font_label.size, label, 0.0, Label_justify, form);
		}
		if (unit[0]) {	/* Add unit label */
			form = gmt_setfont (GMT, &GMT->current.setting.font_annot[GMT_PRIMARY]);
			if (flip & PSSCALE_FLIP_UNIT)	/* The y-label is on the left */
				PSL_plottext (PSL, xleft  - elength[XLO] - GMT->current.setting.map_annot_offset[GMT_PRIMARY], 0.5 * width, GMT->current.setting.font_annot[GMT_PRIMARY].size, unit, 0.0, PSL_MR, form);
			else
				PSL_plottext (PSL, xright + elength[XHI] + GMT->current.setting.map_annot_offset[GMT_PRIMARY], 0.5 * width, GMT->current.setting.font_annot[GMT_PRIMARY].size, unit, 0.0, PSL_ML, form);
		}
		if (P->is_wrapping) {	/* Add cyclic glyph */
			if ((flip & PSSCALE_FLIP_UNIT) || unit[0] == 0)	/* The y-label is on the left or not used so place cyclic glyph on right */
				x0 = xright + GMT->current.setting.map_annot_offset[GMT_PRIMARY] + 0.45 * width;
			else if ((Ctrl->D.emode & 4) == 0)	/* TNo nan so place on left */
				x0 = xleft - GMT->current.setting.map_annot_offset[GMT_PRIMARY] - 0.45 * width;
			else	/* Give up and place in center */
				x0 = 0.5 * (xleft + xright);
			plot_cycle (GMT, x0, 0.5 * width, PSSCALE_CYCLE_DIM * width);
		}
	}
	else {	/* Vertical scale */
		PSL_setorigin (PSL, width, 0.0, 90.0, PSL_FWD);
		if (use_image)	/* Must plot with image */
			PSL_plotcolorimage (PSL, 0.0, 0.0, length, width, PSL_BL, bar, nx, ny, depth);
		else if (need_image) {	/* Sme parts require image, overwrite the rest */
			PSL_plotcolorimage (PSL, 0.0, 0.0, length, width, PSL_BL, bar, nx, ny, depth);
			x0 = x1 = 0.0;
			for (i = 0; i < P->n_colors; i++) {
				ii = (reverse) ? P->n_colors - i - 1 : i;
				x1 += z_width[ii];
				if (P->data[ii].skip || (f = P->data[ii].fill) == NULL) {	/* Do not paint this slice at all */
					x0 = x1;
					continue;
				}
				gmt_setfill (GMT, f, center);	/* Set fill and paint box */
				/* Must undo rotation so patterns remain aligned with original setup */
				PSL_setorigin (PSL, x0 + gap, 0.0, -90.0, PSL_FWD);
				PSL_plotbox (PSL, -width, 0.0, 0.0, x1 - x0 - 2.0 * gap);
				PSL_setorigin (PSL, -(x0 + gap), 0.0, 90.0, PSL_INV);
				x0 = x1;
			}
		}
		else {			/* Plot all slices as rectangles */
			x0 = x1 = 0.0;
			for (i = 0; i < P->n_colors; i++) {
				ii = (reverse) ? P->n_colors - i - 1 : i;
				x1 += z_width[ii];
				if (P->data[ii].skip) {	/* Do not paint this slice at all */
					x0 = x1;
					continue;
				}
				if ((f = P->data[ii].fill) != NULL)	/* Using pattern fills */
					gmt_setfill (GMT, f, center);
				else if (Ctrl->I.active) {
					nb = (P->is_gray || Ctrl->M.active) ? 1 : 3;
					tmp = gmt_M_memory (GMT, NULL, ny*nb, unsigned char);
					for (j = 0, k = i*nb; j < ny*nb; k+=(nx-1)*nb) {
						tmp[j++] = bar[k++];
						tmp[j++] = bar[k++];
						tmp[j++] = bar[k++];
					}
					PSL_plotcolorimage (PSL, x0 + gap, 0.0, z_width[ii] - 2*gap, width, PSL_BL, tmp, 1, ny, depth);
					gmt_M_free (GMT, tmp);
					PSL_setfill (PSL, GMT->session.no_rgb, center);
				}
				else {
					gmt_M_rgb_copy (rgb, P->data[ii].rgb_low);
					if (Ctrl->I.active) gmt_illuminate (GMT, max_intens[1], rgb);
					if (Ctrl->M.active) rgb[0] = rgb[1] = rgb[2] = gmt_M_yiq (rgb);
					PSL_setfill (PSL, rgb, center);
				}
				if (P->data[ii].fill) {	/* Must undo rotation so patterns remain aligned with original setup */
					PSL_setorigin (PSL, x0 + gap, 0.0, -90.0, PSL_FWD);
					PSL_plotbox (PSL, -width, 0.0, 0.0, x1 - x0 - 2.0 * gap);
					PSL_setorigin (PSL, -(x0 + gap), 0.0, 90.0, PSL_INV);
				}
				else
					PSL_plotbox (PSL, x0 + gap, 0.0, x1 - gap, width);
				x0 = x1;
			}
		}

		if (center && Ctrl->L.interval) {
			sprintf (text, "%ld - %ld", lrint (floor (P->data[0].z_low)), lrint (ceil (P->data[0].z_high)));
			sprintf (test, "%ld - %ld", lrint (floor (P->data[P->n_colors-1].z_low)), lrint (ceil (P->data[P->n_colors-1].z_high)));
			hor_annot_width = ((MAX ((int)strlen (text), (int)strlen (test)) + 2*ndec) * GMT_DEC_WIDTH - 0.4 +
				((ndec > 0) ? 2*GMT_PER_WIDTH : 0.0))
				* GMT->current.setting.font_annot[GMT_PRIMARY].size * GMT->session.u2u[GMT_PT][GMT_INCH];
		}
		else {
			sprintf (text, "%ld", lrint (floor (P->data[0].z_low)));
			sprintf (test, "%ld", lrint (ceil (center ? P->data[P->n_colors-1].z_low : P->data[P->n_colors-1].z_high)));
			hor_annot_width = ((MAX ((int)strlen (text), (int)strlen (test)) + ndec) * GMT_DEC_WIDTH +
				((ndec > 0) ? GMT_PER_WIDTH : 0.0))
				* GMT->current.setting.font_annot[GMT_PRIMARY].size * GMT->session.u2u[GMT_PT][GMT_INCH];
		}
		if (GMT->current.setting.map_annot_ortho[0] == '\0')	/* Reset width to height since rotated */
			hor_annot_width = GMT->current.setting.font_annot[GMT_PRIMARY].size * GMT->session.u2u[GMT_PT][GMT_INCH];	/* Annotations are orthogonal */

		annot_off = ((len > 0.0 && !center) ? len : 0.0) + GMT->current.setting.map_annot_offset[GMT_PRIMARY] + hor_annot_width;
		label_off = annot_off + GMT->current.setting.map_label_offset;
		if (use_labels || (flip & PSSCALE_FLIP_ANNOT) || Ctrl->Q.active) annot_off -= hor_annot_width;
		y_annot = y_base + dir * annot_off;
		if ((flip & PSSCALE_FLIP_ANNOT) == (flip & PSSCALE_FLIP_LABEL) / 2) y_label = y_base + dir * label_off;

		PSL_setlinecap (PSL, PSL_BUTT_CAP);	/* Butt cap required for outline of triangle */

		if (Ctrl->D.emode & (reverse + 1)) {	/* Add color triangle at bottom */
			xp[0] = xp[2] = xleft - gap;	xp[1] = xleft - gap - Ctrl->D.elength;
			yp[0] = width - yd;	yp[2] = yd;	yp[1] = 0.5 * width;
			for (i = 0; i < 3; i++) xp[i] += xd;
			xp[1] += xt;
			id = (reverse) ? GMT_FGD : GMT_BGD;
			if ((f = P->bfn[id].fill) != NULL)
				gmt_setfill (GMT, f, false);
			else {
				gmt_M_rgb_copy (rgb, P->bfn[id].rgb);
				if (Ctrl->M.active) rgb[0] = rgb[1] = rgb[2] = gmt_M_yiq (rgb);
				PSL_setfill (PSL, rgb, false);
			}
			PSL_plotpolygon (PSL, xp, yp, 3);
			PSL_plotline (PSL, xp, yp, 3, PSL_MOVE|PSL_STROKE);
			nan_off = Ctrl->D.elength - xd;	/* Must make space for the triangle */
		}
		if (Ctrl->D.emode & 4) {	/* Add NaN rectangle on left side */
			nan_off += Ctrl->D.elength;
			xp[0] = xp[1] = xleft - gap;	xp[2] = xp[3] = xp[0] - MAX (width, Ctrl->D.elength);
			for (i = 0; i < 4; i++) xp[i] -= nan_off;
			yp[0] = yp[3] = width;	yp[1] = yp[2] = 0.0;
			if ((f = P->bfn[GMT_NAN].fill) != NULL)
				gmt_setfill (GMT, f, true);
			else {
				gmt_M_rgb_copy (rgb, P->bfn[GMT_NAN].rgb);
				if (Ctrl->M.active) rgb[0] = rgb[1] = rgb[2] = gmt_M_yiq (rgb);
				PSL_setfill (PSL, rgb, true);
			}
			PSL_plotpolygon (PSL, xp, yp, 4);
			if (Ctrl->D.etext) PSL_plottext (PSL, xp[2] - fabs (GMT->current.setting.map_annot_offset[GMT_PRIMARY]), 0.5 * width, GMT->current.setting.font_annot[GMT_PRIMARY].size, Ctrl->D.etext, -90.0, PSL_TC, 0);
		}
		if (Ctrl->D.emode & (2 - reverse)) {	/* Add color triangle at top */
			xp[0] = xp[2] = xright + gap;	xp[1] = xp[0] + Ctrl->D.elength;
			yp[0] = width - yd;	yp[2] = yd;	yp[1] = 0.5 * width;
			for (i = 0; i < 3; i++) xp[i] -= xd;
			xp[1] -= xt;
			id = (reverse) ? GMT_BGD : GMT_FGD;
			if ((f = P->bfn[id].fill) != NULL)
				gmt_setfill (GMT, f, false);
			else {
				gmt_M_rgb_copy (rgb, P->bfn[id].rgb);
				if (Ctrl->M.active) rgb[0] = rgb[1] = rgb[2] = gmt_M_yiq (rgb);
				PSL_setfill (PSL, rgb, false);
			}
			PSL_plotpolygon (PSL, xp, yp, 3);
			PSL_plotline (PSL, xp, yp, 3, PSL_MOVE|PSL_STROKE);
		}

		PSL_setlinecap (PSL, PSL_SQUARE_CAP);	/* Square cap required for box of scale bar */

		if (gap == 0.0) {
			if ((flip & PSSCALE_FLIP_ANNOT) || !B_set) PSL_plotsegment (PSL, xleft, 0.0, xleft + length, 0.0);
			if (!(flip & PSSCALE_FLIP_ANNOT) || !B_set) PSL_plotsegment (PSL, xleft, width, xleft + length, width);
			PSL_plotsegment (PSL, xleft, 0.0, xleft, width);
			PSL_plotsegment (PSL, xright, 0.0, xright, width);
		}
		if (B_set) {	/* Used -B. Must kludge by copying x-axis and scaling to y since we must use gmt_xy_axis to draw a y-axis based on x parameters. */
			void (*tmp) (struct GMT_CTRL *, double, double *) = NULL;
			char *custum;

			A = &GMT->current.map.frame.axis[GMT_X];
			if (A->item[GMT_ANNOT_UPPER].generated) A->item[GMT_ANNOT_UPPER].interval = 0.0;	/* Reset annot so we can redo via automagic */
			if (A->item[GMT_TICK_UPPER].generated)  A->item[GMT_TICK_UPPER].interval  = 0.0;	/* Reset frame so we can redo via automagic */
			if (A->item[GMT_GRID_UPPER].generated)  A->item[GMT_GRID_UPPER].interval  = 0.0;	/* Reset grid  so we can redo via automagic */
			gmt_auto_frame_interval (GMT, GMT_X, GMT_ANNOT_UPPER);
			if (A->item[GMT_GRID_UPPER].active) {	/* Gridlines work fine without kludging since no annotations involved */
				dx = gmtlib_get_map_interval (GMT, &A->item[GMT_GRID_UPPER]);
				gmt_setpen (GMT, &GMT->current.setting.map_grid_pen[GMT_PRIMARY]);
				if (A->type == GMT_TIME)
					plot_timex_grid (GMT, PSL, P->data[0].z_low, P->data[P->n_colors-1].z_high, 0.0, width, GMT_GRID_UPPER);
				else
					gmt_linearx_grid (GMT, PSL, P->data[0].z_low, P->data[P->n_colors-1].z_high, 0.0, width, dx);
			}
			PSL_setorigin (PSL, 0.0, 0.0, -90.0, PSL_FWD);	/* Rotate back so we can plot y-axis */
			/* Copy x-axis annotation and scale info to y-axis.  We don't need to undo this since gmt_end_module will restore it for us */
			custum = GMT->current.map.frame.axis[GMT_Y].file_custom;	/* Need to remember what this was */
			gmt_M_memcpy (&GMT->current.map.frame.axis[GMT_Y], &GMT->current.map.frame.axis[GMT_X], 1, struct GMT_PLOT_AXIS);
			gmt_M_double_swap (GMT->current.proj.scale[GMT_X], GMT->current.proj.scale[GMT_Y]);
			gmt_M_double_swap (GMT->current.proj.origin[GMT_X], GMT->current.proj.origin[GMT_Y]);
			gmt_M_uint_swap (GMT->current.proj.xyz_projection[GMT_X], GMT->current.proj.xyz_projection[GMT_Y]);
			tmp = GMT->current.proj.fwd_x; GMT->current.proj.fwd_y = GMT->current.proj.fwd_x; GMT->current.proj.fwd_x = tmp;
			GMT->current.map.frame.axis[GMT_Y].id = GMT_Y;
			for (i = 0; i < 5; i++) GMT->current.map.frame.axis[GMT_Y].item[i].parent = GMT_Y;
			gmt_xy_axis2 (GMT, -y_base, 0.0, length, start_val, stop_val, &GMT->current.map.frame.axis[GMT_Y], flip & PSSCALE_FLIP_ANNOT, GMT->current.map.frame.side[flip & PSSCALE_FLIP_ANNOT ? W_SIDE : E_SIDE] & GMT_AXIS_ANNOT, GMT->current.map.frame.side[flip & PSSCALE_FLIP_ANNOT ? W_SIDE : E_SIDE]);
			PSL_setorigin (PSL, 0.0, 0.0, 90.0, PSL_INV);	/* Rotate back to where we started in this branch */
			GMT->current.map.frame.axis[GMT_Y].file_custom = custum;	/* Restore correct pointer */
		}
		else {	/* When no -B we annotate every CPT bound which may be non-equidistant, hence this code (i.e., we cannot fake a call to -B) */
			if (!skip_lines) {	/* First draw gridlines */
				gmt_setpen (GMT, &GMT->current.setting.map_grid_pen[GMT_PRIMARY]);
				for (i = 0; i < n_xpos; i++)
					PSL_plotsegment (PSL, xpos[i], 0.0, xpos[i], width);
			}

			/* Then draw annotation and frame tickmarks */

			if (!center) {
				gmt_setpen (GMT, &GMT->current.setting.map_tick_pen[GMT_PRIMARY]);
				for (i = 0; i < P->n_colors; i++) {
					t_len = (all || (P->data[i].annot & 1)) ? dir * len : dir * len2;	/* Annot or frame length */
					PSL_plotsegment (PSL, xpos[i], y_base, xpos[i], y_base+t_len);
				}
				if (!use_labels) {
					t_len = (all || (P->data[P->n_colors-1].annot & 2)) ? dir * len : dir * len2;	/* Annot or frame length */
					PSL_plotsegment (PSL, xpos[P->n_colors], y_base, xpos[P->n_colors], y_base+t_len);
				}
			}

			/* Finally plot annotations */

			PSL_settextmode (PSL, PSL_TXTMODE_MINUS);	/* Replace hyphens with minus signs */
			form = gmt_setfont (GMT, &GMT->current.setting.font_annot[GMT_PRIMARY]);
			x1 = xleft;
			if (center) x1 += 0.5 * z_width[0];	/* Can use z_width[0] since -L forces all widths to be equal */

			for (i = 0; i < P->n_colors; i++) {
				xx = (reverse) ? xright - x1 : x1;
				if (all || (P->data[i].annot & 1)) {
					this_just = justify;
					do_annot = true;
					if (use_labels && P->data[i].label) {
						strncpy (text, P->data[i].label, GMT_LEN256-1);
						this_just = l_justify;
					}
					else if (center && Ctrl->L.interval)
						sprintf (text, format, P->data[i].z_low, P->data[i].z_high);
					else if (Ctrl->Q.active) {
						p_val = irint (P->data[i].z_low);
						if (doubleAlmostEqualZero (P->data[i].z_low, (double)p_val))
							sprintf (text, "10@+%d@+", p_val);
						else
							do_annot = false;
						this_just = l_justify;
					}
					else
						sprintf (text, format, P->data[i].z_low);
					if (!cpt_auto_fmt) this_just = l_justify;
					if (do_annot) PSL_plottext (PSL, xx, y_annot, GMT->current.setting.font_annot[GMT_PRIMARY].size, text, -90.0, -this_just, form);
				}
				x1 += z_width[i];
			}
			if (!center && !use_labels) {
				i = P->n_colors-1;
				if (all || (P->data[i].annot & 2)) {
					this_just = justify;
					do_annot = true;
					if (Ctrl->Q.active) {
						p_val = irint (P->data[i].z_high);
						if (doubleAlmostEqualZero (P->data[i].z_high, (double)p_val))
							sprintf (text, "10@+%d@+", p_val);
						else
							do_annot = false;
						this_just = l_justify;
					}
					else
						gmt_sprintf_float (GMT, text, format, P->data[i].z_high);
					if (!cpt_auto_fmt) this_just = l_justify;
					if (do_annot) PSL_plottext (PSL, xpos[P->n_colors], y_annot, GMT->current.setting.font_annot[GMT_PRIMARY].size, text, -90.0, -this_just, form);
				}
			}
			PSL_settextmode (PSL, PSL_TXTMODE_HYPHEN);	/* Back to leave as is */
		}

		if (label[0]) {	/* Add label */
			form = gmt_setfont (GMT, &GMT->current.setting.font_label);
			if (strchr (label, '@') || strchr (label, '(') || !(flip & PSSCALE_FLIP_VERT)) { /* Must set text along-side color bar */
				PSL_plottext (PSL, xleft + 0.5 * length, y_label, GMT->current.setting.font_label.size, label, 0.0, Label_justify, form);
			}
			else {	/* Set label text as column (AARRGGHH) */
				int dir = (flip & PSSCALE_FLIP_LABEL) - 1;
				y_label += 0.5 * dir * GMT->current.setting.font_label.size * GMT->session.u2u[GMT_PT][GMT_INCH];
				size = 0.9 * GMT->current.setting.font_label.size * GMT->session.u2u[GMT_PT][GMT_INCH];
				x0 = 0.5 * (length + ((int)strlen (label) -1) * size);
				text[1] = 0;
				for (i = 0; i < strlen (label); i++) {
					x1 = x0 - i * size;
					text[0] = label[i];
					PSL_plottext (PSL, x1, y_label, GMT->current.setting.font_label.size, text, -90.0, PSL_MC, form);
				}
			}
		}
		if (unit[0]) {	/* Add unit label */
			form = gmt_setfont (GMT, &GMT->current.setting.font_annot[GMT_PRIMARY]);
			if (flip & PSSCALE_FLIP_UNIT)	/* The y-label is on the bottom */
				PSL_plottext (PSL, xleft  - GMT->current.setting.map_annot_offset[GMT_PRIMARY] - elength[XLO], 0.5 * width, GMT->current.setting.font_annot[GMT_PRIMARY].size, unit, -90.0, PSL_TC, form);
			else
				PSL_plottext (PSL, xright + GMT->current.setting.map_annot_offset[GMT_PRIMARY] + elength[XHI], 0.5 * width, GMT->current.setting.font_annot[GMT_PRIMARY].size, unit, -90.0, PSL_BC, form);
		}
		if (P->is_wrapping) {	/* Add cyclic glyph */
			if ((flip & PSSCALE_FLIP_UNIT) || unit[0] == 0)	/* The y-label is on the left or not used so place cyclic glyph on right */
				x0 = xright + GMT->current.setting.map_annot_offset[GMT_PRIMARY] + 0.45 * width;
			else if ((Ctrl->D.emode & 4) == 0)	/* TNo nan so place on left */
				x0 = xleft - GMT->current.setting.map_annot_offset[GMT_PRIMARY] - 0.45 * width;
			else	/* Give up and place at center */
				x0 = 0.5 * (xleft + xright);
			plot_cycle (GMT, x0, 0.5 * width, PSSCALE_CYCLE_DIM * width);
		}
		PSL_setorigin (PSL, -width, 0.0, -90.0, PSL_INV);
	}
	gmt_M_free (GMT, xpos);
	if (use_image || Ctrl->I.active || need_image) gmt_M_free (GMT, bar);
	/* Reset back to original line cap and join */
	PSL_setlinecap (PSL, cap);
	PSL_setlinejoin (PSL, join);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int gmtlib_parse_B_option (struct GMT_CTRL *GMT, char *in);

int GMT_colorbar (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: colorbar\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psscale (V_API, mode, args);
}

int GMT_psscale (void *V_API, int mode, void *args) {
	/* High-level function that implements the psscale task */
	int error = 0;
	unsigned int i;

	char text[GMT_LEN256] = {""};

	double dz, dim[2], start_val, stop_val, wesn[4], *z_width = NULL;

	struct PSSCALE_CTRL *Ctrl = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_DATASET *D = NULL;

	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMT_PALETTE_HIDDEN *PH = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	/* Overrule GMT settings of MAP_FRAME_AXES. Use WESN */
	GMT->current.map.frame.side[S_SIDE] = GMT->current.map.frame.side[E_SIDE] = GMT->current.map.frame.side[N_SIDE] = GMT->current.map.frame.side[W_SIDE] = GMT_AXIS_ALL;
	GMT->current.map.frame.draw = false;	/* No -B parsed explicitly yet */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psscale main code ----------------------------*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input CPT\n");
	if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (Ctrl->D.extend && P->is_wrapping) {
		GMT_Report (API, GMT_MSG_NORMAL, "Cannot use +e for cycling color bar; +e deactivated\n");
		Ctrl->D.extend = false;
		Ctrl->D.emode &= 4;	/* This removes any 1,2,3 of selected but leaves 4 for nan */
	}

	if (P->has_range)	/* Convert from normalized to default CPT z-range */
		gmt_stretch_cpt (GMT, P, 0.0, 0.0);
	
	if (Ctrl->G.active) {	/* Attempt truncation */
		struct GMT_PALETTE *Ptrunc = gmt_truncate_cpt (GMT, P, Ctrl->G.z_low, Ctrl->G.z_high);	/* Possibly truncate the CPT */
		if (Ptrunc == NULL)
			Return (EXIT_FAILURE);
		P = Ptrunc;
		//GMT_Write_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, 0, NULL, "chop.cpt", P);
	}
	if (Ctrl->W.active)	/* Scale all z values */
		gmt_scale_cpt (GMT, P, Ctrl->W.scale);

	if (P->categorical) {
		Ctrl->L.active = Ctrl->L.interval = true;
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "CPT is for categorical data.\n");
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "  CPT range from %g to %g\n", P->data[0].z_low, P->data[P->n_colors-1].z_high);

	if (Ctrl->Q.active) {	/* Take log of all z values */
		for (i = 0; i < P->n_colors; i++) {
			if (P->data[i].z_low <= 0.0 || P->data[i].z_high <= 0.0) {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option: All z-values must be positive for logarithmic scale\n");
				Return (GMT_RUNTIME_ERROR);
			}
			P->data[i].z_low  = d_log10 (GMT, P->data[i].z_low);
			P->data[i].z_high = d_log10 (GMT, P->data[i].z_high);
			P->data[i].i_dz = 1.0 / (P->data[i].z_high - P->data[i].z_low);
		}
	}

	/* Because psscale uses -D to position the scale we need to make some
	 * changes so that BoundingBox and others are set ~correctly */

	if (Ctrl->Q.active && GMT->current.map.frame.draw) {
		sprintf (text, "X%gil/%gi", Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y]);
		start_val = pow (10.0, P->data[0].z_low);
		stop_val  = pow (10.0, P->data[P->n_colors-1].z_high);
	}
	else {
		if (P->mode & GMT_CPT_TIME)	/* Need time axis */
			sprintf (text, "X%giT/%gi", Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y]);
		else
			sprintf (text, "X%gi/%gi", Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y]);
		start_val = P->data[0].z_low;
		stop_val  = P->data[P->n_colors-1].z_high;
	}

	gmt_M_memset (wesn, 4, double);
	if (!(GMT->common.R.active[RSET] && GMT->common.J.active)) {	/* When no projection specified, use fake linear projection */
		GMT->common.R.active[RSET] = true;
		GMT->common.J.active = false;
		gmt_parse_common_options (GMT, "J", 'J', text);
		wesn[XLO] = start_val;	wesn[XHI] = stop_val;	wesn[YHI] = Ctrl->D.dim[GMT_Y];
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, wesn), ""))
			Return (GMT_PROJECTION_ERROR);
		if ((PSL = gmt_plotinit (GMT, options)) == NULL)
			Return (GMT_RUNTIME_ERROR);
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	}
	else {	/* First use current projection, project, then use fake projection */
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), ""))
			Return (GMT_PROJECTION_ERROR);
		gmt_set_refpoint (GMT, Ctrl->D.refpoint);	/* Finalize reference point plot coordinates, if needed */

		if (Ctrl->D.dim[GMT_X] == 0.0) {	/* Automatically set scale width */
			Ctrl->D.dim[GMT_X] = PSSCALE_L_SCALE * (Ctrl->D.horizontal ? GMT->current.proj.rect[XHI] : GMT->current.proj.rect[YHI]);
			Ctrl->D.dim[GMT_Y] = PSSCALE_W_SCALE * Ctrl->D.dim[GMT_X];
			if (Ctrl->Q.active && GMT->current.map.frame.draw)
				sprintf (text, "X%gil/%gi", Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y]);
			else if (P->mode & GMT_CPT_TIME)	/* Need time axis */
				sprintf (text, "X%giT/%gi", Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y]);
			else
				sprintf (text, "X%gi/%gi", Ctrl->D.dim[GMT_X], Ctrl->D.dim[GMT_Y]);
			GMT_Report (API, GMT_MSG_DEBUG, "Scale mapping set to -J%s\n", text);
		}
		if ((PSL = gmt_plotinit (GMT, options)) == NULL)
			Return (GMT_RUNTIME_ERROR);
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT->common.J.active = false;
		gmt_parse_common_options (GMT, "J", 'J', text);
		wesn[XLO] = start_val;	wesn[XHI] = stop_val;	wesn[YHI] = Ctrl->D.dim[GMT_Y];
		if (GMT->current.plot.panel.active) GMT->current.plot.panel.no_scaling = 1;	/* Do not rescale dimensions */
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, wesn), ""))
			Return (GMT_PROJECTION_ERROR);
		if (GMT->current.plot.panel.active) GMT->current.plot.panel.no_scaling = 0;	/* Reset no_scaling flag */
	}

	if (!GMT->current.map.frame.draw && (PH = gmt_get_C_hidden (P)) && PH->auto_scale) {	/* No -B given yet we have raw auto-scaling */
		gmtlib_parse_B_option (GMT, "af");
	}
	else if (GMT->common.B.active[GMT_PRIMARY] || GMT->common.B.active[GMT_SECONDARY]) {	/* Must redo the -B parsing since projection has changed */
		char p[GMT_LEN256] = {""}, group_sep[2] = {" "}, *tmp = NULL;
		unsigned int pos = 0;
		GMT_Report (API, GMT_MSG_DEBUG, "Clean re reparse -B settings\n");
		group_sep[0] = GMT_ASCII_GS;
		GMT->current.map.frame.init = false;	/* To ensure we reset B parameters */
		for (i = GMT_PRIMARY; i <= GMT_SECONDARY; i++) {
			if (!GMT->common.B.active[i]) continue;
			tmp = strdup (GMT->common.B.string[i]);
			while (gmt_strtok (tmp, group_sep, &pos, p))
				gmtlib_parse_B_option (GMT, p);
			gmt_M_str_free (tmp);
		}
	}

	if (Ctrl->Z.active) {
		if (GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->Z.file, &D)) {
			Return (API->error);
		}
		z_width = D->table[0]->segment[0]->data[GMT_X];
		if (D->table[0]->segment[0]->n_rows < (uint64_t)P->n_colors) {
			GMT_Report (API, GMT_MSG_NORMAL, "-Z file %s has fewer slices than -C file %s!\n", Ctrl->Z.file, Ctrl->C.file);
			Return (GMT_RUNTIME_ERROR);
		}
	}
	else if (Ctrl->L.active) {
		z_width = gmt_M_memory (GMT, NULL, P->n_colors, double);
		dz = fabs (Ctrl->D.dim[GMT_X]) / P->n_colors;
		for (i = 0; i < P->n_colors; i++) z_width[i] = dz;
	}
	else {
		z_width = gmt_M_memory (GMT, NULL, P->n_colors, double);
		for (i = 0; i < P->n_colors; i++) z_width[i] = fabs (Ctrl->D.dim[GMT_X]) * (P->data[i].z_high - P->data[i].z_low) / (P->data[P->n_colors-1].z_high - P->data[0].z_low);
	}

	if (Ctrl->D.emode && Ctrl->D.elength == 0.0) Ctrl->D.elength = Ctrl->D.dim[GMT_Y] * 0.5;

	if (Ctrl->D.horizontal) {
		GMT->current.map.frame.side[E_SIDE] = GMT->current.map.frame.side[W_SIDE] = GMT_AXIS_NONE;
		dim[GMT_X] = fabs (Ctrl->D.dim[GMT_X]);	dim[GMT_Y] = Ctrl->D.dim[GMT_Y];
	}
	else {
		dim[GMT_Y] = fabs (Ctrl->D.dim[GMT_X]);	dim[GMT_X] = Ctrl->D.dim[GMT_Y];
		GMT->current.map.frame.side[S_SIDE] = GMT->current.map.frame.side[N_SIDE] = GMT_AXIS_NONE;
		gmt_M_double_swap (GMT->current.proj.z_project.xmin, GMT->current.proj.z_project.ymin);
		gmt_M_double_swap (GMT->current.proj.z_project.xmax, GMT->current.proj.z_project.ymax);
	}
	gmt_adjust_refpoint (GMT, Ctrl->D.refpoint, dim, Ctrl->D.off, Ctrl->D.justify, PSL_BL);	/* Adjust refpoint to BL corner */
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "After shifts, Bar reference x = %g y = %g\n", Ctrl->D.refpoint->x, Ctrl->D.refpoint->y);
	PSL_setorigin (PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->y, 0.0, PSL_FWD);

	gmt_draw_colorbar (GMT, Ctrl, P, z_width);

	PSL_setorigin (PSL, -Ctrl->D.refpoint->x, -Ctrl->D.refpoint->y, 0.0, PSL_FWD);
	gmt_plane_perspective (GMT, -1, 0.0);

	gmt_plotend (GMT);

	if (!Ctrl->Z.active) gmt_M_free (GMT, z_width);

	Return (GMT_NOERROR);
}
