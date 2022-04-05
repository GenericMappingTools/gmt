/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: grdcontour reads a 2-D grid file and contours it,
 * controlled by a variety of options.
 *
 * Note on KEYS: AD)=t mean -A takes an optional output Dataset as argument via the +t modifier.
 *               G?(=1 means if -Gf|x is given then we may read an input Dataset, else we set type to ! to skip it.
 *               The "1" means we must skip the single char (f or x) before finding the file name
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdcontour"
#define THIS_MODULE_MODERN_NAME	"grdcontour"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Make contour map using a grid"
#define THIS_MODULE_KEYS	"<G{,AD)=t,CC(,DDD,NC(,>X},G?(=1"
#define THIS_MODULE_NEEDS	"Jg"
#define THIS_MODULE_OPTIONS "-BJKOPRUVXYbdfhlptxy" GMT_OPT("EMmc")

/* Control structure for grdcontour */

struct GRDCONTOUR_CTRL {
	struct GRDCONTOUR_In {
		bool active;
		char *file;
	} In;
	struct GMT_CONTOUR contour;
	struct GRDCONTOUR_A {	/* -A[n|[+]<int>][labelinfo] */
		bool active;
		struct CONTOUR_ARGS info;
	} A;
	struct GRDCONTOUR_C {	/* -C<cont_int> */
		bool active;
		struct CONTOUR_ARGS info;
	} C;
	struct GRDCONTOUR_D {	/* -D<dumpfile> */
		bool active;
		char *file;
	} D;
	struct GRDCONTOUR_F {	/* -F<way> */
		bool active;
		int value;
	} F;
	struct GRDCONTOUR_G {	/* -G[d|f|n|l|L|x|X]<params> */
		bool active;
	} G;
	struct GRDCONTOUR_L {	/* -L<Low/high> */
		bool active;
		int mode;	/* -2: Negative contours, -1 also do 0, +1 : Positive contours with 0, +2: only positive, 0: given range */
		double low, high;
	} L;
	struct GRDCONTOUR_Q {	/* -Q[<cut>][+z] */
		bool active;
		bool zero;	/* True if we should skip zero-contour */
		bool project;	/* True if we need distances in plot units */
		double length;
		int mode;	/* Could be negative */
		unsigned int min;
		char unit;
	} Q;
	struct GRDCONTOUR_S {	/* -S<smooth> */
		bool active;
		unsigned int value;
	} S;
	struct GRDCONTOUR_T {	/* -T[l|h][+a][+d<gap>[c|i|p][/<length>[c|i|p]]][+lLH|"low,high"] */
		bool active;
		struct CONTOUR_CLOSED info;
	} T;
	struct GRDCONTOUR_W {	/* -W[a|c]<pen>[+c[l|f]] */
		bool active;
		bool cpt_effect;
		bool scaling;
		unsigned int cptmode;	/* Apply to both a&c */
		double scale;	/* Scaling of pen width modifier [Only used from grd2kml so far] */
		double cutoff;	/* Ignore contours whose pen is < this width in points [Only used from grd2kml so far] */
		struct GMT_PEN pen[2];
	} W;
	struct GRDCONTOUR_Z {	/* -Z[<fact>[/shift>]][p] */
		bool active;
		bool periodic;
		double scale, offset;
	} Z;
};

#define GRDCONTOUR_MIN_LENGTH 0.01	/* Contours shorter than this are skipped */
#define TICKED_SPACING	15.0		/* Spacing between ticked contour ticks (in points) */
#define TICKED_LENGTH	3.0		/* Length of ticked contour ticks (in points) */
#define PEN_CONT	0
#define PEN_ANNOT	1

enum grdcontour_contour_type {cont_is_not_closed = 0,	/* Not a closed contour of any sort */
	cont_is_closed = 1,				/* Clear case of closed contour away from boundaries */
	cont_is_closed_straddles_west = -2,		/* Part of a closed contour that exits west for periodic boundary */
	cont_is_closed_straddles_east = +2,		/* Part of a closed contour that exits east for periodic boundary */
	cont_is_closed_around_south_pole = -3,		/* Closed contour in southern hemisphere that encloses the south pole */
	cont_is_closed_around_north_pole = +3,		/* Closed contour in northern hemisphere that encloses the north pole */
	cont_is_closed_straddles_equator_south = -4,	/* Closed contour crossing equator that encloses the south pole */
	cont_is_closed_straddles_equator_north = +4};	/* Closed contour crossing equator that encloses the north pole */

struct GRDCONTOUR_SAVE {
	double *x, *y;
	double *xp, *yp;
	double cval;
	double xlabel, ylabel;
	double y_min, y_max;
	int n, np;
	struct GMT_PEN pen;
	struct GMT_FONT font;
	int do_it, high;
	enum grdcontour_contour_type kind;
	char label[GMT_LEN64];
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDCONTOUR_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDCONTOUR_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	gmt_contlabel_init (GMT, &C->contour, 1);
	C->A.info.single_cont = GMT->session.d_NaN;
	C->C.info.single_cont = GMT->session.d_NaN;
	C->L.low = -DBL_MAX;
	C->L.high = DBL_MAX;
	C->T.info.dim[GMT_X] = TICKED_SPACING * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 14p */
	C->T.info.dim[GMT_Y] = TICKED_LENGTH  * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 3p */
	C->W.pen[PEN_CONT] = C->W.pen[PEN_ANNOT] = GMT->current.setting.map_default_pen;
	C->W.pen[PEN_ANNOT].width *= 3.0;
	C->Z.scale = 1.0;

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDCONTOUR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->A.info.file);
	gmt_M_str_free (C->C.info.file);
	gmt_M_str_free (C->D.file);
	gmt_M_str_free (C->T.info.txt[0]);
	gmt_M_str_free (C->T.info.txt[1]);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	struct GMT_PEN P;

	/* This displays the grdcontour synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s %s %s [-A[n|[+]<int>|<list>][<labelinfo>]] [%s] [-C<contours>] "
		"[-D<template>] [-F[l|r]] [%s] %s[-L<low>/<high>|n|N|P|p] [-N[<cpt>]] %s%s[-Q[<n>][+z]] [%s] "
		"[-S<smooth>] [%s] [%s] [%s] [-W[a|c]<pen>[+c[l|f]]] [%s] [%s] [-Z[+o<shift>][+p][+s<fact>]] "
		"[%s] %s[%s] [%s] [%s] [%s] [%s] [%s]\n", name, GMT_INGRID, GMT_J_OPT, GMT_B_OPT, GMT_CONTG, API->K_OPT, API->O_OPT,
		API->P_OPT, GMT_Rgeoz_OPT, GMT_CONTT, GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_bo_OPT, API->c_OPT,
		GMT_do_OPT, GMT_ho_OPT, GMT_l_OPT, GMT_p_OPT, GMT_t_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Name of grid to be contoured");
	GMT_Option (API, "J-Z");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A[n|[+]<int>|<list>][<labelinfo>]");
	GMT_Usage (API, -2, "Annotation label settings [Default is no annotated contours]. "
		"Give annotation interval or comma-separated list of contours "
		"(for single contour append comma to be seen as list). "
		"Alternatively, give -An to disable all contour annotations "
		"implied by the information provided in -C.");
	GMT_Usage (API, -2, "<labelinfo> controls the specifics of the labels.  Choose from:");
	gmt_label_syntax (API->GMT, 2, 1);
	GMT_Option (API, "B-");
	GMT_Usage (API, 1, "\n-C<contours>");
	GMT_Usage (API, -2, "Contours to be drawn can be specified in one of four ways:");
	GMT_Usage (API, 3, "%s Fixed contour interval.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s Comma-separated contours (for single contour append comma to be seen as list).", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s File with contour levels, types, and optional fixed annotation angle and/or pen: "
		"<contlevel> [[<angle>] C|c|A|a [<pen>]]. "
		"Use A|a for annotated contours or C|c for plain contours. If -T is used, "
		"only inner-most contours with upper case C or A will be ticked. "
		"If file only contains <contlevel> then we default to type C for plain contours only.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s Name of a CPT. "
		"[CPT contours are set to C unless the CPT flags are set; "
		"Use -A to force all to become A].", GMT_LINE_BULLET);
	GMT_Usage (API, -2, "If neither -A nor -C are set then we auto-select the intervals.");
	GMT_Usage (API, 1, "\n-D<template>");
	GMT_Usage (API, -2, "Dump contours as data line segments; no plotting takes place. "
		"Append filename template which may contain C-format specifiers. "
		"If no filename template is given we write all lines to standard output. "
		"If filename has no specifiers then we write all lines to a single file. "
		"If a float format (e.g., %%6.2f) is found we substitute the contour z-value. "
		"If an integer format (e.g., %%06d) is found we substitute a running segment count. "
		"If an char format (%%c) is found we substitute C or O for closed and open contours. "
		"The 1-3 specifiers may be combined and appear in any order to produce the "
		"the desired number of output files (e.g., just %%c gives two files, just %%f would. "
		"separate segments into one file per contour level, and %%d would write all segments. "
		"to individual files; see module documentation for more examples.");
	GMT_Usage (API, 1, "\n-F[l|r]");
	GMT_Usage (API, -2, "Force dumped contours to be oriented so that the higher z-values "
		"are to the left (-Fl [Default]) or right (-Fr) as we move along "
		"the contour lines [Default is not oriented].");
	GMT_Usage (API, 1, "\n%s", GMT_CONTG);
	GMT_Usage (API, -2, "Control placement of labels along contours.  Choose from:");
	gmt_cont_syntax (API->GMT, 2, 0);
	GMT_Option (API, "K");
	GMT_Usage (API, 1, "\n-L<low>/<high>|n|N|P|p");
	GMT_Usage (API, -2, "Only contour inside specified range.  Alternatively, give -Ln or -Lp to just draw "
		"negative or positive contours, respectively. Upper case N or P includes zero contour.");
	GMT_Usage (API, 1, "\n-N[<cpt>]");
	GMT_Usage (API, -2, "Fill area between contour with the colors in a discrete CPT. If <cpt> is given the "
		"we use that <cpt> for the fill, else -C<cpt> must be given and used instead [no fill].");
	GMT_Option (API, "O,P");
	GMT_Usage (API, 1, "\n-Q[<n>][+z]");
	GMT_Usage (API, -2, "Do not draw closed contours with less than <n> points [Draw all contours]. "
		"Alternatively, give a minimum contour length and append a unit (%s, or c for Cartesian). "
		"Unit C means Cartesian distances after first projecting the input coordinates. "
		"Optionally, append +z to skip tracing the zero-contour.", GMT_LEN_UNITS_DISPLAY);
	GMT_Option (API, "R");
	GMT_Usage (API, -2, "[Default is extent of grid].");
	GMT_Usage (API, 1, "\n-S<smooth>");
	GMT_Usage (API, -2, "Smooth contours by interpolation at approximately <gridsize>/<smooth> intervals.");
	GMT_Usage (API, 1, "\n%s", GMT_CONTT);
	GMT_Usage (API, -2, "Embellish innermost, closed contours with ticks pointing in the downward direction. "
		"User may specify to tick only highs (-Th) or lows (-Tl) [-T implies both extrema].");
	GMT_Usage (API, 3, "+a Tick all closed contours.");
	GMT_Usage (API, 3, "+d Append spacing>[/<ticklength>] (with units) to change defaults [%gp/%gp].",
		TICKED_SPACING, TICKED_LENGTH);
	GMT_Usage (API, 3, "+l Append two characters (e.g., LH) or two comma-separated strings (e.g., \"low,high\") to place labels at the center "
		"of local lows and highs.  If no labels are given we default to - and +.");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Set pen attributes. Append a<pen> for annotated or (default) c<pen> for regular contours", NULL, 0);
	GMT_Usage (API, -2, "The default pen settings are:");
	P = API->GMT->current.setting.map_default_pen;
	GMT_Usage (API, 3, "%s Contour pen:  %s.", GMT_LINE_BULLET, gmt_putpen (API->GMT, &P));
	P.width *= 3.0;
	GMT_Usage (API, 3, "%s Annotate pen: %s.", GMT_LINE_BULLET, gmt_putpen (API->GMT, &P));
	GMT_Usage (API, 3, "+c Controls how pens and fills are affected if a CPT is specified via -C: "
		"Append l to let pen colors follow the CPT setting (requires -C). "
		"Append f to let fill/font colors follow the CPT setting. "
		"Default [+c] sets both effects.");
	GMT_Option (API, "X");
	GMT_Usage (API, 1, "\n-Z[+o<shift>][+p][+s<fact>]");
	GMT_Usage (API, -2, "Adjust grid prior to contouring, and flag periodic data:");
	GMT_Usage (API, 3, "+o Add <shift> to all grid values [0]");
	GMT_Usage (API, 3, "+p z-data are periodic in 360 (i.e., phase data)");
	GMT_Usage (API, 3, "+s Multiply all grid values by <fact> [1]");
	GMT_Usage (API, -2, "Note: Scaling by <fact> takes place prior to adding a shift");
	GMT_Option (API, "bo3,c,do,f,h,l");
	GMT_Usage (API, -2, "Normally, the annotated contour is selected; change this by specifying the label as "
		"[<annotcontlabel>][/<contlabel>] (use separator | if / is part of the label).");
	GMT_Option (API, "p,t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int grdcontour_parse_Z_opt (struct GMT_CTRL *GMT, char *txt, struct GRDCONTOUR_CTRL *Ctrl) {
	/* Parse the -Z option: -Z[+o<offset>][+p][+s<scale>] */
	unsigned int uerr = 0;
	if (!txt || txt[0] == '\0') {
		GMT_Report (GMT->parent, GMT_MSG_ERROR,
    		"Option -Z: No arguments given\n");
		return (GMT_PARSE_ERROR);
	}
	if (gmt_found_modifier (GMT, txt, "ops")) {
		char p[GMT_LEN64] = {""};
		unsigned int pos = 0;
		while (gmt_getmodopt (GMT, 'Z', txt, "sop", &pos, p, &uerr) && uerr == 0) {
			switch (p[0]) {
				case 's':	Ctrl->Z.scale = atof (&p[1]);	break;
				case 'o':	Ctrl->Z.offset = atof (&p[1]);	break;
				case 'p':	Ctrl->Z.periodic = true;		break;
				default: 	/* These are caught in gmt_getmodopt so break is just for Coverity */
					break;
			}
		}
	}
	else {	/* Old syntax */
		if (txt[0] && txt[0] != 'p') sscanf (txt, "%lf/%lf", &Ctrl->Z.scale, &Ctrl->Z.offset);
		Ctrl->Z.periodic = (txt[strlen(txt)-1] == 'p');	/* Phase data */
	}
	return (uerr ? GMT_PARSE_ERROR : GMT_NOERROR);
}

static int parse (struct GMT_CTRL *GMT, struct GRDCONTOUR_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdcontour and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, id, reset = 0;
	int j, k, n;
	char txt_a[GMT_LEN256] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) {n_errors++; continue; }
				Ctrl->In.active = true;
				if (opt->arg[0]) Ctrl->In.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file))) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Annotation control */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				k = gmt_contour_first_pos (GMT, opt->arg);	/* Do deal with backwards compatibility */
				if ((c = gmt_first_modifier (GMT, &opt->arg[k], GMT_CONTSPEC_MODS))) {	/* Process any modifiers */
					if (gmt_contlabel_specs (GMT, c, &Ctrl->contour)) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -A: Expected\n\t-A[n|<contours>][+a<angle>|n|p[u|d]][+c<dx>[/<dy>]][+d][+e][+f<font>][+g<fill>][+i][+j<just>][+l<label>][+n|N<dx>[/<dy>]][+o][+p<pen>][+r<min_rc>][+t[<file>]][+u<unit>][+v][+w<width>][+=<prefix>]\n");
						n_errors ++;
					}
					c[0] = '\0';	/* Chop off modifiers since parsed by gmt_contlabel_specs */
				}
				n_errors += gmt_contour_A_arg_parsing (GMT, opt->arg, &Ctrl->A.info);
				if (Ctrl->A.info.mode == 0) Ctrl->contour.annot = true;
				if (c) c[0] = '+';	/* Restore modifiers */
				break;
			case 'C':	/* Contour interval/cpt */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				if (Ctrl->C.active) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -C: Given more than once\n");
					n_errors++;
					break;
				}
				Ctrl->C.active = true;
				n_errors += gmt_contour_C_arg_parsing (GMT, opt->arg, &Ctrl->C.info);
				break;
			case 'D':	/* Dump file name */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				if (opt->arg[0]) Ctrl->D.file = strdup (opt->arg);
				break;
			case 'F':	/* Orient dump contours */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case '\0': case 'l':
						Ctrl->F.value = -1;
						break;
					case 'r':
						Ctrl->F.value = +1;
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Expected -F[l|r]\n");
						break;
				}
				break;
			case 'G':	/* Contour labeling position control */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				n_errors += gmt_contlabel_info (GMT, 'G', opt->arg, &Ctrl->contour);
				break;
			case 'M': case 'm':	/* GMT4 Old-options no longer required - quietly skipped under compat */
				if (!gmt_M_compat_check (GMT, 4)) n_errors += gmt_default_option_error (GMT, opt);
				break;
			case 'L':	/* Limits on range */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.active = true;
				switch (opt->arg[0]) {
					case 'n': Ctrl->L.mode = -2; break;
					case 'N': Ctrl->L.mode = -1; break;
					case 'P': Ctrl->L.mode = +1; break;
					case 'p': Ctrl->L.mode = +2; break;
					default:
						sscanf (opt->arg, "%lf/%lf", &Ctrl->L.low, &Ctrl->L.high);
						break;
				}
				break;
			case 'Q':	/* Skip small closed contours */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				if ((c = strstr (opt->arg, "+z"))) {
					Ctrl->Q.zero = true;
					c[0] = '\0';	/* Temporarily chop off modifier */
				}
				if (opt->arg[0]) {
					size_t last = strlen (opt->arg) - 1;
					Ctrl->Q.active = true;
					if (strchr (GMT_LEN_UNITS, opt->arg[last]))	/* Gave a minimum length in data units */
						Ctrl->Q.mode = gmt_get_distance (GMT, opt->arg, &(Ctrl->Q.length), &(Ctrl->Q.unit));
					else if (opt->arg[last] == 'C') {	/* Projected units */
						Ctrl->Q.length = atof (opt->arg);
						Ctrl->Q.project = true;
						Ctrl->Q.unit = 'C';
					}
					else if (opt->arg[last] == 'c') {	/* Cartesian units */
						Ctrl->Q.length = atof (opt->arg);
						Ctrl->Q.unit = 'X';
					}
					else {	/* Just a point count cutoff */
						n = atoi (opt->arg);
						n_errors += gmt_M_check_condition (GMT, n < 0, "Option -Q: Point count must be >= 0\n");
						Ctrl->Q.min = n;
					}
				}
				if (c) c[0] = '+';	/* Restore */
				break;
			case 'S':	/* Smoothing of contours */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.active = true;
				j = atoi (opt->arg);
				n_errors += gmt_M_check_condition (GMT, j < 0, "Option -S: Smooth_factor must be > 0\n");
				Ctrl->S.value = j;
				break;
			case 'T':	/* Ticking of innermost closed contours */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				n_errors += gmt_contour_T_arg_parsing (GMT, opt->arg, &Ctrl->T.info);
				break;
			case 'W':	/* Pen settings */
				Ctrl->W.active = true;
				if ((c = strstr (opt->arg, "+s"))) {	/* Gave +s<scl> modifier to scale pen widths given via -C and optional cutoff pen width */
					sscanf (&c[2], "%lf/%lg", &Ctrl->W.scale, &Ctrl->W.cutoff);
					Ctrl->W.scaling = true;
					break;
				}
				k = reset = 0;
				if ((opt->arg[0] == '-' && opt->arg[1]) || (opt->arg[0] == '+' && opt->arg[1] != 'c')) {	/* Definitively old-style args */
					if (opt->arg[k] == '+') Ctrl->W.cptmode = 1, k++;
					if (opt->arg[k] == '-') Ctrl->W.cptmode = 3, k++;
					j = (opt->arg[k] == 'a' || opt->arg[k] == 'c') ? k+1 : k;
				}
				else {
					if ((c = strstr (opt->arg, "+c"))) {	/* Gave +c modifier - apply to both pens */
						switch (c[2]) {
							case 'l': Ctrl->W.cptmode = 1; break;
							case 'f': Ctrl->W.cptmode = 2; break;
							default:  Ctrl->W.cptmode = 3; break;
						}
						if (!strncmp (&c[2], "lf", 2U) || !strncmp (&c[2], "fl", 2U))	/* Catch any odd +clf or +cfl modifiers */
							Ctrl->W.cptmode = 3;
						c[0] = 0;	/* Temporarily chop of */
						reset = 1;
					}
					j = (opt->arg[0] == 'a' || opt->arg[0] == 'c') ? k+1 : k;
				}
				if (j == k && opt->arg[j]) {	/* Set both and gave argument */
					if (gmt_getpen (GMT, &opt->arg[j], &Ctrl->W.pen[PEN_CONT])) {
						gmt_pen_syntax (GMT, 'W', NULL, " ", NULL, 0);
						n_errors++;
					}
					else
						Ctrl->W.pen[PEN_ANNOT] = Ctrl->W.pen[PEN_CONT];
				}
				else if (opt->arg[j]) {	/* Gave a or c.  Because the user may say -Wcyan we must prevent this from being seen as -Wc and color yan! */
					/* Get the argument following a or c and up to first comma, slash (or to the end) */
					n = k+1;
					while (!(opt->arg[n] == ',' || opt->arg[n] == '/' || opt->arg[n] == '\0')) n++;
					strncpy (txt_a, &opt->arg[k], (size_t)(n-k));	txt_a[n-k] = '\0';
					if (gmt_colorname2index (GMT, txt_a) >= 0) j = k;	/* Found a colorname; wind j back by 1 */
					id = (opt->arg[k] == 'a') ? PEN_ANNOT : PEN_CONT;
					if (gmt_getpen (GMT, &opt->arg[j], &Ctrl->W.pen[id])) {
						gmt_pen_syntax (GMT, 'W', NULL, " ", NULL, 0);
						n_errors++;
					}
					if (j == k) Ctrl->W.pen[PEN_ANNOT] = Ctrl->W.pen[PEN_CONT];	/* Must copy since it was not -Wc nor -Wa after all */
				}
				if (reset) c[0] = '+';	/* Restore */
				if (Ctrl->W.cptmode) Ctrl->W.cpt_effect = true;
				break;
			case 'Z':	/* For scaling or phase data */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
				n_errors += grdcontour_parse_Z_opt (GMT, opt->arg, Ctrl);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	if (Ctrl->C.info.check && gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.info.file)))
		Ctrl->C.info.cpt = true;

	if (Ctrl->A.info.interval > 0.0 && (!Ctrl->C.info.file && Ctrl->C.info.interval == 0.0)) Ctrl->C.info.interval = Ctrl->A.info.interval;

	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Must specify a single grid file\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active && !Ctrl->D.active,
	                                 "Must specify a map projection with the -J option\n");
#if 0
	n_errors += gmt_M_check_condition (GMT, !Ctrl->C.info.file && Ctrl->C.info.interval <= 0.0 &&
			gmt_M_is_dnan (Ctrl->C.single_cont) && gmt_M_is_dnan (Ctrl->A.info.single_cont),
	                     "Option -C: Must specify contour interval, file name with levels, or CPT\n");
#endif
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.low > Ctrl->L.high, "Option -L: lower limit > upper!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && !Ctrl->D.active, "Option -F: Must also specify -D\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->contour.label_dist_spacing <= 0.0 || Ctrl->contour.half_width <= 0,
	                                 "Option -G: Correct syntax:\n\t-G<annot_dist>/<npoints>, both values must be > 0\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.scale == 0.0, "Option -Z: factor must be nonzero\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.cptmode && !Ctrl->C.info.cpt,
	                                 "Option -W: Modifier +c only valid if -C sets a CPT\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

/* Three sub functions used by GMT_grdcontour */

GMT_LOCAL void grdcontour_sort_and_plot_ticks (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct GRDCONTOUR_SAVE *save, size_t n, struct GMT_GRID *G, struct CONTOUR_CLOSED *I, unsigned int mode, struct GMT_DATASET *T) {
	/* Labeling and ticking of inner-most contours cannot happen until all contours are found and we can determine
	   which are the innermost ones. Here, all the n candidate contours are passed via the save array.
	   We need to do several types of testing here:
	   1) First we exclude closed contours around a single node as too small.
	   2) Next, we mark closed contours with other contours inside them as not "innermost"
	   3) We then determine if the remaining closed polygons contain highs or lows.

	   Note on mode bitflags: mode = 1 (plot only), 2 (save labels only), 3 (both)
	*/

	int np, j, k, inside, col, row, stop, n_ticks, way, form;
	uint64_t ij;
	size_t pol, pol2;
	bool done, match, found;
	char *lbl[2], *def[2] = {"-", "+"};
	double add, dx, dy, x_back, y_back, x_front, y_front, x_end, y_end, x_lbl, y_lbl;
	double xmin, xmax, ymin, ymax, inc, dist, a, this_lon, this_lat, sa, ca;
	double *s = NULL, *xp = NULL, *yp = NULL;
	double da, db, dc, dd, L, L_cut;

	double tick_gap = I->dim[GMT_X];
	double tick_length = I->dim[GMT_Y];

	lbl[0] = (I->txt[0]) ? I->txt[0] : def[0];
	lbl[1] = (I->txt[1]) ? I->txt[1] : def[1];
	/* The x/y coordinates in SAVE in original coordinates */

	for (pol = 0; pol < n; pol++) {	/* Set y min/max for polar caps */
		if (abs (save[pol].kind) < 3) continue;	/* Skip all but polar caps */
		save[pol].y_min = save[pol].y_max = save[pol].y[0];
		for (j = 1; j < save[pol].n; j++) {
			if (save[pol].y[j] < save[pol].y_min) save[pol].y_min = save[pol].y[j];
			if (save[pol].y[j] > save[pol].y_max) save[pol].y_max = save[pol].y[j];
		}
	}

	for (pol = 0; pol < n; pol++) {	/* Mark polygons that have other polygons inside them */
		np = save[pol].n;	/* Length of this polygon */
		for (pol2 = 0; save[pol].do_it && pol2 < n; pol2++) {
			if (pol == pol2) continue;		/* Cannot be inside itself */
			if (!save[pol2].do_it) continue;	/* No point checking contours that have already failed */
			if (save[pol].kind == cont_is_closed_straddles_equator_south || save[pol].kind == cont_is_closed_straddles_equator_north) {	/* These are closed "polar caps" that crosses equator (in lon/lat) */
				save[pol].do_it = false;	/* This may be improved in the future */
				continue;
			}
			if (abs (save[pol].kind) != 3) {	/* Not a polar cap so we can call gmt_non_zero_winding */
				col = save[pol2].n / 2;	/* Pick the half-point for testing */
				inside = gmt_non_zero_winding (GMT, save[pol2].x[col], save[pol2].y[col], save[pol].x, save[pol].y, np);
				if (inside == GMT_INSIDE && !I->all) save[pol].do_it = false;	/* Not innermost so mark it for exclusion */
			}
			if (save[pol2].kind == cont_is_closed_around_south_pole || save[pol2].kind == cont_is_closed_around_north_pole) {	/* Polar caps needs a different test */
				if (save[pol].kind == cont_is_closed_around_south_pole || save[pol].kind == cont_is_closed_around_north_pole) {	/* Both are caps */
					if (save[pol].kind != save[pol2].kind) continue;	/* One is S and one is N cap as far as we can tell, so we skip */
					/* Crude test to determine if one is closer to the pole than the other; if so exclude the far one */
					if ((save[pol2].kind == cont_is_closed_around_north_pole && save[pol2].y_min > save[pol].y_min) || (save[pol2].kind == cont_is_closed_around_south_pole && save[pol2].y_min < save[pol].y_min)) save[pol].do_it = false;
				}
			}
		}
	}
	for (pol = 0; pol < n; pol++) if (save[pol].kind == cont_is_closed_straddles_west || save[pol].kind == cont_is_closed_straddles_east) save[pol].n--;	/* Chop off the extra duplicate point for split periodic contours */

	/* Must make sure that for split periodic contour that if one fails to be innermost then both should fail */

	for (pol = 0; pol < n; pol++) {
		if (abs (save[pol].kind) != 2) continue;	/* Not a split polygon */
		for (pol2 = 0, found = false; !found && pol2 < n; pol2++) {
			if (pol == pol2) continue;
			if (abs (save[pol2].kind) != 2) continue;	/* Not a split polygon */
			/* The following uses 10^-7 as limit, hence the 0.1 */
			da = save[pol].y[0] - save[pol2].y[0]; db = save[pol].y[save[pol].n-1] - save[pol2].y[save[pol2].n-1];
			dc = save[pol].y[0] - save[pol2].y[save[pol2].n-1];	dd = save[pol].y[save[pol].n-1] - save[pol2].y[0];
			match = ((gmt_M_is_zero (0.1*da) && gmt_M_is_zero (0.1*db)) || (gmt_M_is_zero (0.1*dc) && gmt_M_is_zero (0.1*dd)));
			if (!match) continue;
			if ((save[pol].do_it + save[pol2].do_it) == 1) save[pol].do_it = save[pol2].do_it = false;
			found = true;
		}
		if (!found) save[pol].do_it = false;	/* Probably was not a split closed contour */
	}

	/* Here, only the polygons that are innermost (containing the local max/min, will have do_it = true */

	PSL_comment (PSL, "Start Embellishment of innermost contours\n");
	for (pol = 0; pol < n; pol++) {
		if (!save[pol].do_it) continue;
		np = save[pol].n;

		/* Need to determine if this is a local high or low */

		if (save[pol].kind == cont_is_closed_straddles_west || save[pol].kind == cont_is_closed_straddles_east) {	/* Closed contour split across a periodic boundary */
			/* Determine row, col for a point ~mid-way along the vertical periodic boundary */
			col = (save[pol].kind == cont_is_closed_straddles_west) ? 0 : G->header->n_columns - 1;
			row = (int)gmt_M_grd_y_to_row (GMT, save[pol].y[0], G->header);		/* Get start j-row */
			row += (int)gmt_M_grd_y_to_row (GMT, save[pol].y[np-1], G->header);	/* Get stop j-row */
			row /= 2;
		}
		else if (abs (save[pol].kind) >= 3) {	/* Polar cap, pick point at midpoint along top or bottom boundary */
			col = G->header->n_columns / 2;
			row = (save[pol].kind < 0) ? G->header->n_rows - 1 : 0;
		}
		else {
			/* Loop around the contour and get min/max original x,y (longitude, latitude) coordinates */

			xmin = xmax = save[pol].x[0];	ymin = ymax = save[pol].y[0];
			for (j = 1; j < np; j++) {
				xmin = MIN (xmin, save[pol].x[j]);
				xmax = MAX (xmax, save[pol].x[j]);
				ymin = MIN (ymin, save[pol].y[j]);
				ymax = MAX (ymax, save[pol].y[j]);
			}

			/* Pick the mid-latitude and march along that line from east to west */

			this_lat = 0.5 * (ymax + ymin);	/* Mid latitude, probably not exactly on a y-node */
			row = (int)MIN (G->header->n_rows - 1, gmt_M_grd_y_to_row (GMT, this_lat, G->header));	/* Get closest j-row */
			this_lat = gmt_M_grd_row_to_y (GMT, row, G->header);	/* Get its matching latitude */
			col  = (int)gmt_M_grd_x_to_col (GMT, xmin, G->header);		/* Westernmost point */
			stop = (int)gmt_M_grd_x_to_col (GMT, xmax, G->header);		/* Eastermost point */
			done = false;
			while (!done && col <= stop) {
				this_lon = gmt_M_grd_col_to_x (GMT, col, G->header);	/* Current longitude */
				inside = gmt_non_zero_winding (GMT, this_lon, this_lat, save[pol].x, save[pol].y, np);
				/* Worry about this point being ~exactly on the border but returned inside = 2, but
				 * the grid node will be exactly the contour value and then fail the assignment of high
				 * below.  We therefore check if the value is not equal to the contour value as well */
				if (inside == GMT_INSIDE) {	/* Might be inside */
					ij = gmt_M_ijp (G->header, row, col);
					if (!doubleAlmostEqual (G->data[ij], save[pol].cval))	/* OK, this point is truly inside */
						done = true;
					else	/* March along to the next point */
						col++;
				}
				else	/* March along to the next point */
					col++;
			}
			if (!done) continue;	/* Failed to find an inside point */
		}
		ij = gmt_M_ijp (G->header, row, col);
		save[pol].high = (G->data[ij] > save[pol].cval);

		if (save[pol].high && !I->high) continue;	/* Do not tick highs */
		if (!save[pol].high && !I->low) continue;	/* Do not tick lows */

		np = (int)gmt_clip_to_map (GMT, save[pol].x, save[pol].y, np, &xp, &yp);	/* Convert to inches */
		if (np == 0) continue;

		s = gmt_M_memory (GMT, NULL, np, double);	/* Compute distance along the contour */
		for (j = 1, s[0] = 0.0; j < np; j++) {
			L = hypot (xp[j]-xp[j-1], yp[j]-yp[j-1]);
			L_cut = gmt_half_map_width (GMT, 0.5 * (yp[j]+yp[j-1]));
			if (L > L_cut)	/* Eliminate horizontal jumps exceeding half mapwidth at this y-value */
				L = 0.0;
			s[j] = s[j-1] + L;
		}
		n_ticks = irint (floor (s[np-1] / tick_gap));
		if (s[np-1] < GRDCONTOUR_MIN_LENGTH || n_ticks == 0) {	/* Contour is too short to be ticked or labeled */
			save[pol].do_it = false;
			gmt_M_free (GMT, s);	gmt_M_free (GMT, xp);	gmt_M_free (GMT, yp);
			continue;
		}

		way = gmt_polygon_centroid (GMT, xp, yp, np, &save[pol].xlabel, &save[pol].ylabel);	/* -1 is CCW, +1 is CW */
		/* Compute mean location of closed contour ~hopefully a good point inside to place label. */

		if (mode & 1) {	/* Tick the innermost contour */
			x_back = xp[np-1];	/* Last point along contour */
			y_back = yp[np-1];
			dist = 0.0;
			j = 0;
			add = M_PI_2 * ((save[pol].high) ? -way : +way);	/* So that tick points in the right direction */
			inc = s[np-1] / n_ticks;
			gmt_setpen (GMT, &save[pol].pen);
			while (j < np-1) {
				x_front = xp[j+1];
				y_front = yp[j+1];
				if (s[j] >= dist) {	/* Time for tick */
					dx = x_front - x_back;
					dy = y_front - y_back;
					a = atan2 (dy, dx) + add;
					sincos (a, &sa, &ca);
					x_end = xp[j] + tick_length * ca;
					y_end = yp[j] + tick_length * sa;
					PSL_plotsegment (PSL, xp[j], yp[j], x_end, y_end);
					dist += inc;
				}
				x_back = x_front;
				y_back = y_front;
				j++;
			}
		}
		gmt_M_free (GMT, s);	gmt_M_free (GMT, xp);	gmt_M_free (GMT, yp);
	}

	/* Still not finished with labeling polar caps when the pole point plots as a line (e.g., -JN).
	 * One idea would be to add help points to include the pole and used this polygon to compute where to
	 * plot the label but then skip those points when drawing the line. */

	PSL_settextmode (PSL, PSL_TXTMODE_MINUS);	/* Replace hyphens with minus signs */

	for (pol = 0; I->label && pol < n; pol++) {	/* Finally, do labels */
		if (!save[pol].do_it) continue;
		if (abs (save[pol].kind) == 2 && gmt_M_is_azimuthal (GMT)) {	/* Only plot once at mean location */
			for (pol2 = 0, k = -1; pol2 < n && k == -1; pol2++) {	/* Finally, do labels */
				if (save[pol2].kind != -save[pol].kind) continue;
				if (save[pol2].cval != save[pol].cval) continue;
				k = (int)pol2;	/* Found its counterpart */
			}
			if (k == -1) continue;
			x_lbl = 0.5 * (save[pol].xlabel + save[k].xlabel);
			y_lbl = 0.5 * (save[pol].ylabel + save[k].ylabel);
			if (mode & 1) {
				form = gmt_setfont (GMT, &save[pol].font);
				PSL_plottext (PSL, x_lbl, y_lbl, GMT->current.setting.font_annot[GMT_PRIMARY].size, lbl[save[pol].high], 0.0, PSL_MC, form);
			}
			save[k].do_it = false;
			if (mode & 2) gmt_add_label_record (GMT, T, x_lbl, y_lbl, 0.0, lbl[save[pol].high]);
		}
		else {
			if (mode & 1) {
				form = gmt_setfont (GMT, &save[pol].font);
				PSL_plottext (PSL, save[pol].xlabel, save[pol].ylabel, GMT->current.setting.font_annot[GMT_PRIMARY].size, lbl[save[pol].high], 0.0, PSL_MC, form);
			}
			if (mode & 2) gmt_add_label_record (GMT, T, save[pol].xlabel, save[pol].ylabel, 0.0, lbl[save[pol].high]);
		}
	}

	PSL_settextmode (PSL, PSL_TXTMODE_HYPHEN);	/* Back to leave as is */
	PSL_comment (PSL, "End Embellishment of innermost contours\n");
}

GMT_LOCAL void grdcontour_adjust_hill_label (struct GMT_CTRL *GMT, struct GMT_CONTOUR *G, struct GMT_GRID *Grid) {
	/* Modify orientation of contours to have top of annotation facing the local hill top */
	int col, row;
	uint64_t k, seg, ij;
	double n_columns, n_rows, x_on, y_on, x_node, y_node, x_node_p, y_node_p, dx, dy, dz, dot, angle;
	struct GMT_CONTOUR_LINE *C = NULL;

	for (seg = 0; seg < G->n_segments; seg++) {
		C = G->segment[seg];	/* Pointer to current segment */
		for (k = 0; k < C->n_labels; k++) {
			gmt_xy_to_geo (GMT, &x_on, &y_on, C->L[k].x, C->L[k].y);	/* Retrieve original coordinates */
			row = (int)gmt_M_grd_y_to_row (GMT, y_on, Grid->header);
			if (row < 0 || row >= (int)Grid->header->n_rows) continue;		/* Somehow, outside y range */
			while (gmt_M_x_is_lon (GMT, GMT_IN) && x_on < Grid->header->wesn[XLO]) x_on += 360.0;
			while (gmt_M_x_is_lon (GMT, GMT_IN) && x_on > Grid->header->wesn[XHI]) x_on -= 360.0;
			col = (int)gmt_M_grd_x_to_col (GMT, x_on, Grid->header);
			if (col < 0 || col >= (int)Grid->header->n_columns) continue;		/* Somehow, outside x range */
			angle = fmod (2.0 * C->L[k].angle, 360.0) * 0.5;	/* 0-180 range */
			if (angle > 90.0) angle -= 180.0;
			sincosd (angle + 90, &n_rows, &n_columns);	/* Coordinate of normal to label line */
			x_node = gmt_M_grd_col_to_x (GMT, col, Grid->header);
			y_node = gmt_M_grd_row_to_y (GMT, row, Grid->header);
			gmt_geo_to_xy (GMT, x_node, y_node, &x_node_p, &y_node_p);	/* Projected coordinates of nearest node point */
			dx = x_node_p - C->L[k].x;
			dy = y_node_p - C->L[k].y;
			if (gmt_M_is_zero (hypot (dx, dy))) {
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Unable to adjust hill label contour orientation (node point on contour)\n");
				continue;
			}
			ij = gmt_M_ijp (Grid->header, row, col);
			dz = Grid->data[ij] - C->z;
			if (doubleAlmostEqualZero (Grid->data[ij], C->z)) {
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Unable to adjust hill label contour orientation (node value = contour value)\n");
				continue;
			}
			dot = dx * n_columns + dy * n_rows;	/* 2-D Dot product of n and vector from contour to node. +ve if on same side of contour line */
			if (lrint (copysign (1.0, dot * dz)) != G->hill_label) C->L[k].angle += 180.0;	/* Must turn upside-down */
		}
	}
}

GMT_LOCAL enum grdcontour_contour_type gmt_is_closed (struct GMT_CTRL *GMT, struct GMT_GRID *G, double *x, double *y, int n) {
	/* Determine if this is a closed contour; returns a flag in the -4/+4 range */
	enum grdcontour_contour_type closed = cont_is_not_closed;
	int k;
	double small_x = 0.01 * G->header->inc[GMT_X], small_y = 0.01 * G->header->inc[GMT_Y];	/* Use 1% noise to find near-closed contours */
	double y_min, y_max;

	if (fabs (x[0] - x[n-1]) < small_x && fabs (y[0] - y[n-1]) < small_y) {	/* Closed interior contour */
		closed = cont_is_closed;
		x[n-1] = x[0];	y[n-1] = y[0];	/* Force exact closure */
	}
	else if (gmt_M_x_is_lon (GMT, GMT_IN) && gmt_M_360_range (G->header->wesn[XLO], G->header->wesn[XHI])) {	/* Global geographic grids are special */
		if (fabs (x[0] - G->header->wesn[XLO]) < small_x && fabs (x[n-1] - G->header->wesn[XLO]) < small_x) {	/* Split periodic boundary contour */
			closed = cont_is_closed_straddles_west;	/* Left periodic */
			x[0] = x[n-1] = G->header->wesn[XLO];	/* Force exact closure */
		}
		else if (fabs (x[0] - G->header->wesn[XHI]) < small_x && fabs (x[n-1] - G->header->wesn[XHI]) < small_x) {	/* Split periodic boundary contour */
			closed = cont_is_closed_straddles_east;	/* Right periodic */
			x[0] = x[n-1] = G->header->wesn[XHI];	/* Force exact closure */
		}
		else if (gmt_M_360_range (x[0], x[n-1])) {	/* Must be a polar cap */
			/* Determine if the polar cap stays on one side of the equator */
			y_min = y_max = y[0];
			for (k = 1; k < n; k++) {
				if (y[k] < y_min) y_min = y[k];
				if (y[k] > y_max) y_max = y[k];
			}
			if (y_min < 0.0 && y_max > 0.0)
				/* Special flags for meandering closed contours otherwise indistinguishable from polar caps */
				closed = (y_max > fabs (y_min)) ? cont_is_closed_straddles_equator_north : cont_is_closed_straddles_equator_south;
			else
				/* N or S polar cap; do not force closure though */
				closed = (y[0] > 0.0) ? cont_is_closed_around_north_pole : cont_is_closed_around_south_pole;
		}
	}
	return (closed);
}

GMT_LOCAL void grdcontour_embed_quotes (char *orig, char *dup) {
	/* Add quotes around text strings with spaces in a -B option where the quotes have been lost.
	 * Because the original quotes are gone there is no way to detect things like ...+t"Title with+u in it"
	 * since now it is just +tTitle with+u in it and there is no way to distinguish the two possibilities
	 * +t"Title with"+u" in it" from +t"Title with +u in it".  We insist that +<code> must not have a space
	 * before the <code> in order to be a modifier.  This will catch things like +t"Title with +u in it" and
	 * treat the +u as part of the text.
	 */
	bool quote = false;
	size_t i, o;
	for (i = o = 0; i < strlen (orig); i++) {	/* Process each character in input */
		if (orig[i] == '+' && (i == 0 || orig[i-1] != ' ') && strchr ("lpstu", orig[i+1])) {	/* Beginning of modifier that takes text, possibly with spaces */
			if (quote) dup[o++] = '\"';	/* Terminate previous quoted string */
			dup[o++] = '+';
			dup[o++] = orig[++i];
			dup[o++] = '\"';	/* Start new text quoting */
			quote = true;
		}
		else	/* Regular character, just copy */
			dup[o++] = orig[i];
	}
	if (quote) dup[o++] = '\"';	/* Terminate last quoted string */
	dup[o] = '\0';	/* Terminate string */
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grdcontour (void *V_API, int mode, void *args) {
	/* High-level function that implements the grdcontour task */
	int error, c;
	bool need_proj, make_plot, two_only = false, begin, is_closed, data_is_time = false;
	bool use_contour = true, use_t_offset = false, mem_G = false;

	enum grdcontour_contour_type closed;

	unsigned int id, n_contours, n_edges, tbl_scl = 1, io_mode = 0, uc, tbl, label_mode = 0, n_cont_attempts = 0;
	unsigned int cont_counts[2] = {0, 0}, i, n, nn, *edge = NULL, n_tables = 1, fmt[3] = {0, 0, 0};

	uint64_t ij, *n_seg = NULL;
	int64_t ns;

	size_t n_save = 0, n_alloc = 0, n_save_alloc = 0, *n_seg_alloc = NULL;

	char cont_label[GMT_LEN256] = {""}, format[GMT_LEN256] = {""};

	double aval, cval, small, xyz[2][3], z_range, t_offset = 0.0, wesn[4], rgb[4], c_length = 0.0;
	double *xp = NULL, *yp = NULL, *x = NULL, *y = NULL;

	struct GRDCONTOUR_CTRL *Ctrl = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_DATATABLE_HIDDEN *TH = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	struct GMT_CLOCK_IO Clock;
	struct GMT_DATE_IO Date;
	struct GMT_CONTOUR_INFO *cont = NULL;
	struct GRDCONTOUR_SAVE *save = NULL;
	struct GMT_GRID *G = NULL, *G_orig = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL, *optN = NULL;
	struct PSL_CTRL *PSL = NULL;	/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	if ((optN = GMT_Find_Option (API, 'N', options))) {	/* Split into two module calls */
		/* If -N[<cpt>] is given then we split the call into a grdview + grdcontour sequence.
	 	 * We DO NOT parse any options here or initialize GMT, and just bail after running the two modules */

		char cmd0[GMT_LEN512] = {""}, cmd1[GMT_LEN512] = {""}, cmd2[GMT_LEN512] = {""}, string[GMT_LEN128] = {""}, cptfile[PATH_MAX] = {""}, *ptr = NULL;
		struct GMT_OPTION *opt = NULL;
		bool got_cpt = (optN->arg[0]), is_continuous, got_C_cpt = false, oneliner = false;
		size_t L;

		/* Make sure we don't pass options not compatible with -N */
		if ((opt = GMT_Find_Option (API, 'D', options))) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot use -D with -N\n");
			bailout (GMT_PARSE_ERROR);
		}
		if ((opt = GMT_Find_Option (API, 'Z', options))) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot use -Z with -N\n");
			bailout (GMT_PARSE_ERROR);
		}

		if (gmt_M_file_is_memory (optN->arg))	/* Got cpt via a memory object */
			strncpy (cptfile, optN->arg, PATH_MAX-1);
		else if ((L = strlen (optN->arg)) >= 4 && !strncmp (&optN->arg[L-4], GMT_CPT_EXTENSION, GMT_CPT_EXTENSION_LEN)) {	/* Gave a cpt argument, check that it is valid */
			if (!gmt_file_is_cache (API, optN->arg) && gmt_access (API->GMT, optN->arg, R_OK)) {
				GMT_Report (API, GMT_MSG_ERROR, "Option -N: CPT file %s not found\n", optN->arg);
				bailout (GMT_PARSE_ERROR);
			}
			strncpy (cptfile, optN->arg, PATH_MAX-1);
		}

		/* Process all the options given.  Some are needed by both grdview and grdcontour while others are grdcontour only.
		 * We must consider the situations arising form external API calls: The CPT's may be memory objects so we must
		 * check for that and if found not free the resources.  Also, if a PostScript output file is set via ->file.ps then
		 * we must make sure we append in the second module. Below cmd1 holds the grdview arguments and cmd2 holds the
		 * overlay grdcontour arguments. These are built on the fly */

		for (opt = options; opt; opt = opt->next) {
			if (opt->option == GMT_OPT_INFILE) gmt_filename_set (opt->arg);	/* Replace any spaces with ASCII 29 */
			sprintf (string, " -%c%s", opt->option, opt->arg);
			if (opt->option == GMT_OPT_INFILE) gmt_filename_get (opt->arg);	/* Undo */
			switch (opt->option) {
				case 'A' : case 'D': case 'F': case 'G': case 'K': case 'L': case 'Q': case 'T': case 'U': case 'W': case 'Z':	/* Only for grdcontour */
					strcat (cmd2, string); break;
				case 'B':	/* Must worry about spaces */
					if (strchr (opt->arg, ' ') || strchr (opt->arg, '\t')) {	/* Must place all string arguments in quotes */
						char dup_string[GMT_LEN128] = {""};
						grdcontour_embed_quotes (opt->arg, dup_string);
						sprintf (string, " -%c%s", opt->option, dup_string);
					}
					strcat (cmd2, string); break;
				case 'C':	/* grdcontour, but maybe this is a cpt to be used in grdview as well */
					got_cpt = got_C_cpt = true;
					strcat (cmd2, string);
					if (optN->arg[0])	/* Use the -N cpt instead */
						sprintf (string, " -C%s", optN->arg);
					else if (gmt_M_file_is_memory (opt->arg))	/* CPT passed in as an object */
						strcpy (cptfile, opt->arg);
					else if ((L = strlen (opt->arg)) >= 4 && !strncmp (&opt->arg[L-4], GMT_CPT_EXTENSION, GMT_CPT_EXTENSION_LEN)) {	/* Gave a -C<cpt> argument, check that it is valid */
						if (!gmt_file_is_cache (API, opt->arg) && gmt_access (API->GMT, opt->arg, R_OK)) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -C: CPT file %s not found\n", opt->arg);
							bailout (GMT_PARSE_ERROR);
						}
						strcpy (cptfile, opt->arg);
					}
					else if (L < 4) {
						GMT_Report (API, GMT_MSG_ERROR, "Options -C or -N: No CPT given\n");
						bailout (GMT_PARSE_ERROR);
					}
					strcat (cmd1, string);	break;
				case 'O': /* This would only apply to the first grdcontour call */
					strcat (cmd1, string);	break;
				case 'N':	/* Just skip since it is what got us in here */
					break;
				case 'X': case 'Y':	/* Only grdview gets these unless they are absolute positionings */
					strcat (cmd1, string);
					if (opt->arg[0] == 'a') strcat (cmd2, string);
					break;
				case GMT_OPT_OUTFILE:	/* PostScript file explicitly given in external interface in classic mode */
					strcat (cmd1, string);
					sprintf (string, " -%c%c%s", opt->option, opt->option, opt->arg);	/* Must explicitly append */
					strcat (cmd2, string);
					break;
				case 'b':
				case 'e':
				case 'j':
				case 'p':
				case 'P':
					if (strcmp (opt->arg, "df") == 0 || strcmp (opt->arg, "ng") == 0 || strcmp (opt->arg, "pm") == 0 || strcmp (opt->arg, "s") == 0 ||
						strcmp (opt->arg, "gp") == 0 || strcmp (opt->arg, "peg") == 0 || strcmp (opt->arg, "if") == 0 || strcmp (opt->arg, "iff") == 0 ||
						strcmp (opt->arg, "mp") == 0 || strcmp (opt->arg, "ps") == 0) {
						sprintf (cmd0, "%s %c%s", opt->next->arg, opt->option, opt->arg);
						oneliner = true;
						opt = opt->next;	/* Skip the file name */
					}
					else if (opt->option == 'P')
						strcat (cmd1, string);
					else {
						strcat (cmd1, string);	strcat (cmd2, string);
					}
					break;

				default:	/* These arguments go into both commands (may be -p -n, --, etc) */
					strcat (cmd1, string);	strcat (cmd2, string);
					break;
			}
		}
		if (!got_cpt) {	/* No cpt is bad news */
			GMT_Report (API, GMT_MSG_ERROR, "The -N option requires a <cpt> argument if -C<cpt> is not given\n");
			bailout (GMT_PARSE_ERROR);
		}
		/* Check if the given CPT is discrete as required */
		if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL | GMT_IO_RESET, NULL, cptfile, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to read CPT file %s\n", cptfile);
			bailout (GMT_PARSE_ERROR);
		}
		is_continuous = P->is_continuous;
		/* Free the P object unless it was an input memory object */
		ptr = cptfile;	/* To avoid warning message from gmt_M_file_is_memory */
		if (!gmt_M_file_is_memory (ptr) && GMT_Destroy_Data (API, &P) != GMT_NOERROR) {
			bailout (API->error);
		}
		if (is_continuous) {	/* More bad news */
			GMT_Report (API, GMT_MSG_ERROR, "Option -N: CPT file must be discrete, not continuous\n");
			bailout (GMT_PARSE_ERROR);
		}

		if (oneliner) {
			if ((API->error = GMT_Call_Module (API, "begin", GMT_MODULE_CMD, cmd0))) {
				GMT_Report (API, GMT_MSG_ERROR, "The call to begin failed\n");
				bailout (API->error);
			}
		}
		/* Required options for grdview to fill the grid */
		strcat (cmd1, " -Qs");
		if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !oneliner) strcat (cmd1, " -K");	/* If classic mode then we need to say we will append more PostScript later */
		if (!got_C_cpt) {	/* Must pass -N<cpt> via -C to grdview since no -C was given */
			strcat (cmd1, " -C");
			strcat (cmd1, optN->arg);
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Run: grdview %s\n", cmd1);
		if ((API->error = GMT_Call_Module (API, "grdview", GMT_MODULE_CMD, cmd1))) {
			GMT_Report (API, GMT_MSG_ERROR, "The call to grdview failed\n");
			bailout (API->error);
		}
		/* Required options for grdcontour */
		if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !oneliner) strcat (cmd2, " -O");	/* If classic mode then we need to say we this is an overlay */
		GMT_Report (API, GMT_MSG_DEBUG, "Run: grdcontour %s\n", cmd2);
		if ((API->error = GMT_Call_Module (API, "grdcontour", GMT_MODULE_CMD, cmd2))) {
			GMT_Report (API, GMT_MSG_ERROR, "The call to grdcontour failed\n");
			bailout (API->error);
		}
		if (oneliner) {
			sprintf (cmd0, "show");
			if ((API->error = GMT_Call_Module (API, "end", GMT_MODULE_CMD, cmd0))) {
				GMT_Report (API, GMT_MSG_ERROR, "The call to end failed\n");
				bailout (API->error);
			}
		}
		bailout (GMT_NOERROR);	/* And we made it to the end, so get out of here */
	}

	/* NOT -N, so parse the command-line arguments as a normal module would */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdcontour main code ----------------------------*/

	gmt_grd_set_datapadding (GMT, true);	/* Turn on gridpadding when reading a subset */

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input grid\n");
	if (Ctrl->D.active) {
		GMT_Report (API, GMT_MSG_INFORMATION, "With -D, no plotting will take place\n");
		if (!Ctrl->D.file) GMT_Report (API, GMT_MSG_INFORMATION, "Contours will be written to standard output\n");
	}

	GMT->current.map.z_periodic = Ctrl->Z.periodic;	/* Phase data */

	mem_G = gmt_M_file_is_memory (Ctrl->In.file);
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}
	if ((API->error = gmt_img_sanitycheck (GMT, G->header))) {	/* Used map projection on a Mercator (cartesian) grid */
		Return (API->error);
	}

	make_plot = !Ctrl->D.active;	/* Turn off plotting if -D was used */
	need_proj = !Ctrl->D.active || Ctrl->contour.save_labels;	/* Turn off mapping if -D was used, unless +t was set */

	/* Determine what wesn to pass to map_setup */

	if (!GMT->common.R.active[RSET])	/* -R was not set so we use the grid domain */
		gmt_set_R_from_grd (GMT, G->header);

	if (need_proj && gmt_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_PROJECTION_ERROR);

	/* Determine the wesn to be used to actually read the grid file */

	if (!gmt_grd_setregion (GMT, G->header, wesn, BCR_BILINEAR)) {
		/* No grid to plot; just do empty map and return */
		GMT_Report (API, GMT_MSG_WARNING, "No data within specified region\n");
		if (make_plot) {
			if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
			gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
			gmt_set_basemap_orders (GMT, GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_AFTER, GMT_BASEMAP_ANNOT_AFTER);
			GMT->current.map.frame.order = GMT_BASEMAP_AFTER;	/* Move to last order since only calling gmt_map_basemap once */
			gmt_plotcanvas (GMT);	/* Fill canvas if requested */
			gmt_map_basemap (GMT);
			gmt_plane_perspective (GMT, -1, 0.0);
			gmt_plotend (GMT);
		}
		if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}

	if (Ctrl->C.info.cpt) {	/* Presumably got a CPT */
		if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.info.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (P->categorical)
			GMT_Report (API, GMT_MSG_WARNING, "Categorical data (as implied by CPT) do not have contours.  Check plot.\n");
	}

	/* Read data */

	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, G) == NULL) {
		Return (API->error);
	}

	if (gmt_M_type (GMT, GMT_IN, GMT_Z) == GMT_IS_ABSTIME) {	/* Grid data is time */
		/* To properly label contours using GMT time formatting we will rely on the machinery
		 * for output data formatting.  Thus, we temporarily overwrite those settings with
		 * the selected map settings, then undo the damage before GMT_basemap is called.
		 */
		double t_epoch_unit, t_epoch_unit_even, tmp;
		data_is_time = true;
		/* Save original date and clock output formatting structures */
		gmt_M_memcpy (&Clock, &GMT->current.io.clock_output, 1, struct GMT_CLOCK_IO);
		gmt_M_memcpy (&Date, &GMT->current.io.date_output, 1, struct GMT_DATE_IO);
		/* Set date and clock output to the corresponding MAP versions */
		gmt_M_memcpy (&GMT->current.io.clock_output, &GMT->current.plot.calclock.clock, 1, struct GMT_CLOCK_IO);
		gmt_M_memcpy (&GMT->current.io.date_output,  &GMT->current.plot.calclock.date, 1, struct GMT_DATE_IO);
		/* We want time to align on integer TIME_UNIT.  If epoch is not a multiple of TIME_UNIT
		 * then we compute the offset and add to z */
		tmp = GMT->current.setting.time_system.epoch_t0; GMT->current.setting.time_system.epoch_t0 = 0.0;	/* Save */
		t_epoch_unit = gmt_rdc2dt (GMT, GMT->current.setting.time_system.rata_die, tmp * GMT_DAY2SEC_F);	/* Epoch in user units */
		GMT->current.setting.time_system.epoch_t0 = tmp;	/* Restore */
		t_epoch_unit_even = floor (t_epoch_unit / Ctrl->A.info.interval) * Ctrl->A.info.interval;	/* Offset to grid values in user units reuired to align contours on even epoch */
		t_offset = t_epoch_unit - t_epoch_unit_even;	/* Offset to grid values in user units to align on even epoch */
		use_t_offset = !doubleAlmostEqualZero (t_epoch_unit, t_epoch_unit_even);
		if (use_t_offset) {	/* Must temporarily add t_offset to grid, quietly */
			GMT_Report (API, GMT_MSG_DEBUG, "Adding %g to align grid times with TIME_UNIT steps\n", t_offset);
			gmt_scale_and_offset_f (GMT, G->data, G->header->size, 1.0, t_offset);
			G->header->z_min += t_offset;
			G->header->z_max += t_offset;
		}
	}
	if (!(Ctrl->Z.scale == 1.0 && Ctrl->Z.offset == 0.0)) {	/* Must transform z grid */
		GMT_Report (API, GMT_MSG_INFORMATION, "Subtracting %g and multiplying grid by %g\n", Ctrl->Z.offset, Ctrl->Z.scale);
		G->header->z_min = (G->header->z_min - Ctrl->Z.offset) * Ctrl->Z.scale;
		G->header->z_max = (G->header->z_max - Ctrl->Z.offset) * Ctrl->Z.scale;
		if (Ctrl->Z.scale < 0.0) gmt_M_double_swap (G->header->z_min, G->header->z_max);
		/* Since gmt_scale_and_offset_f applies z' = z * scale + offset we must adjust Z.offset first: */
		Ctrl->Z.offset *= Ctrl->Z.scale;
		gmt_scale_and_offset_f (GMT, G->data, G->header->size, Ctrl->Z.scale, -Ctrl->Z.offset);
	}
	if (Ctrl->L.low > G->header->z_min) G->header->z_min = Ctrl->L.low;	/* Possibly clip the z range */
	if (Ctrl->L.high < G->header->z_max) G->header->z_max = Ctrl->L.high;
	if (Ctrl->L.active && G->header->z_max < G->header->z_min) {	/* Specified contour range outside range of data - quit */
		GMT_Report (API, GMT_MSG_WARNING, "No contours within specified -L range\n");
		if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
			Return (API->error);
		}
		if (make_plot) {	/* Place empty plot according to -B settings */
			if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
			gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
			gmt_set_basemap_orders (GMT, GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_AFTER, GMT_BASEMAP_ANNOT_AFTER);
			GMT->current.map.frame.order = GMT_BASEMAP_AFTER;	/* Move to last order since only calling gmt_map_basemap once */
			gmt_plotcanvas (GMT);	/* Fill canvas if requested */
			gmt_map_basemap (GMT);
			gmt_plane_perspective (GMT, -1, 0.0);
			gmt_plotend (GMT);
		}
		Return (GMT_NOERROR);
	}
	if (!Ctrl->C.active && (!Ctrl->A.active || Ctrl->A.info.interval == 0.0)) {	/* Want automatic annotations */
		double x, range = G->header->z_max - G->header->z_min;
		int nx;
		x = pow (10, floor (log10 (range)) - 1.0);
		nx = irint (range / x);
		if (nx > 40)
			x *= 5;
		else if (nx > 20)
			x *= 2;
		Ctrl->C.info.interval = x;
		Ctrl->A.info.interval = 2.0 * x;
		Ctrl->C.active  = Ctrl->A.active = Ctrl->contour.annot = true;
		GMT_Report (API, GMT_MSG_INFORMATION, "Auto-determined contour interval = %g and annotation interval = %g\n", Ctrl->C.info.interval, Ctrl->A.info.interval);
	}

	if (!strcmp (Ctrl->contour.unit, "z")) strncpy (Ctrl->contour.unit, G->header->z_units, GMT_LEN64-1);
	if (Ctrl->A.info.interval == 0.0) Ctrl->A.info.interval = Ctrl->C.info.interval;

	if (Ctrl->contour.annot) {	/* Want annotated contours */
		/* Determine the first annotated contour level */
		aval = floor (G->header->z_min / Ctrl->A.info.interval) * Ctrl->A.info.interval;
		if (aval < G->header->z_min) aval += Ctrl->A.info.interval;
	}
	else	/* No annotations, set aval outside range */
		aval = G->header->z_max + 1.0;

	if (Ctrl->C.info.cpt) {	/* Got a CPT */
		/* Set up which contours to draw based on the CPT slices and their attributes */
		n_contours = P->n_colors + 1;	/* Since n_colors refer to slices */
		cont = gmt_M_memory (GMT, NULL, n_contours, struct GMT_CONTOUR_INFO);
		for (i = c = 0; i < P->n_colors; i++) {
			if (P->data[i].skip) continue;
			cont[c].val = P->data[i].z_low;
			if (Ctrl->A.info.mode)
				cont[c].type = 'C';
			else if (P->data[i].annot)
				cont[c].type = 'A';
			else
				cont[c].type = (Ctrl->contour.annot) ? 'A' : 'C';
			cont[c].angle = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
			cont[c].do_tick = (char)Ctrl->T.active;
			c++;
		}
		cont[c].val = P->data[P->n_colors-1].z_high;
		if (Ctrl->A.info.mode)
			cont[c].type = 'C';
		else if (P->data[P->n_colors-1].annot & 2)
			cont[c].type = 'A';
		else
			cont[c].type = (Ctrl->contour.annot) ? 'A' : 'C';
		cont[c].angle = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
		cont[c].do_tick = (char)Ctrl->T.active;
		n_contours = c + 1;
	}
	else if ((Ctrl->A.info.file && strchr (Ctrl->A.info.file, ',')) || (Ctrl->C.info.file && strchr (Ctrl->C.info.file, ','))) {	/* Got a comma-separated list of contours */
		uint64_t na = 0, nc = 0;
		double *za = NULL, *zc = NULL;
		if (Ctrl->A.info.file && strchr (Ctrl->A.info.file, ',') && (za = gmt_list_to_array (GMT, Ctrl->A.info.file, gmt_M_type (GMT, GMT_IN, GMT_Z), true, &na)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while parsing annotated contours from list %s\n", Ctrl->A.info.file);
			Return (GMT_RUNTIME_ERROR);
		}
		if (Ctrl->C.info.file && strchr (Ctrl->C.info.file, ',') && (zc = gmt_list_to_array (GMT, Ctrl->C.info.file, gmt_M_type (GMT, GMT_IN, GMT_Z), true, &nc)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while parsing regular contours from list %s\n", Ctrl->C.info.file);
			if (za) gmt_M_free (GMT, za);
			Return (GMT_RUNTIME_ERROR);
		}
		n_contours = na + nc;
		cont = gmt_M_memory (GMT, NULL, n_contours, struct GMT_CONTOUR_INFO);
		for (c = 0; c < (int)nc; c++) {
			cont[c].type = 'C';
			cont[c].val = zc[c];
			cont[c].do_tick = Ctrl->T.active ? 1 : 0;
			cont[c].angle = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
		}
		for (c = 0; c < (int)na; c++) {
			cont[c+nc].type = 'A';
			cont[c+nc].val = za[c];
			cont[c+nc].do_tick = Ctrl->T.active ? 1 : 0;
			cont[c+nc].angle = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
		}
		if (za) gmt_M_free (GMT, za);
		if (zc) gmt_M_free (GMT, zc);
	}
	else if (Ctrl->C.info.file) {	/* Read contour info from file with cval [angle] C|A [pen]] records */
		if ((cont = gmt_get_contours_from_table (GMT, Ctrl->C.info.file, Ctrl->T.active, &Ctrl->contour.angle_type, &n_contours)) == NULL) Return (GMT_RUNTIME_ERROR);
	}
	else if (!gmt_M_is_dnan (Ctrl->C.info.single_cont) || !gmt_M_is_dnan (Ctrl->A.info.single_cont)) {	/* Plot one or two contours only  */
		n_contours = 0;
		cont = gmt_M_memory (GMT, cont, 2, struct GMT_CONTOUR_INFO);
		if (!gmt_M_is_dnan (Ctrl->C.info.single_cont)) {
			cont[n_contours].type = 'C';
			cont[n_contours++].val = Ctrl->C.info.single_cont;
		}
		if (!gmt_M_is_dnan (Ctrl->A.info.single_cont)) {
			cont[n_contours].type = 'A';
			cont[n_contours].val = Ctrl->A.info.single_cont;
			cont[n_contours].do_tick = Ctrl->T.active ? 1 : 0;
			cont[n_contours].angle = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
			n_contours++;
		}
	}
	else {	/* Set up contour intervals automatically from Ctrl->C.info.interval and Ctrl->A.info.interval */
		double min, max, noise = GMT_CONV4_LIMIT * Ctrl->C.info.interval;
		min = floor (G->header->z_min / Ctrl->C.info.interval) * Ctrl->C.info.interval;
		if (!GMT->current.map.z_periodic && min < G->header->z_min) min += Ctrl->C.info.interval;
		max = ceil (G->header->z_max / Ctrl->C.info.interval) * Ctrl->C.info.interval;
		if (max > G->header->z_max) max -= Ctrl->C.info.interval;
		if (Ctrl->A.info.interval > Ctrl->C.info.interval && fabs ((Ctrl->A.info.interval/Ctrl->C.info.interval) - irint (Ctrl->A.info.interval/Ctrl->C.info.interval)) > GMT_CONV4_LIMIT)
			GMT_Report (API, GMT_MSG_WARNING, "Annotation interval is not a multiple of contour interval - no annotated contours will be drawn.\n");
		else if (Ctrl->contour.annot && Ctrl->A.info.interval < Ctrl->C.info.interval)
			GMT_Report (API, GMT_MSG_WARNING, "Annotation interval < contour interval - some/all annotated contours will not be drawn.\n");
		for (c = irint (min/Ctrl->C.info.interval), n_contours = 0; c <= irint (max/Ctrl->C.info.interval); c++, n_contours++) {
			if (n_contours == n_alloc) {
				n_alloc += 32;
				cont = gmt_M_memory (GMT, cont, n_alloc, struct GMT_CONTOUR_INFO);
			}
			gmt_M_memset (&cont[n_contours], 1, struct GMT_CONTOUR_INFO);	/* Cause the pen structure needs to be empty */
			cont[n_contours].val = c * Ctrl->C.info.interval;
			if (Ctrl->contour.annot && (cont[n_contours].val - aval) > noise) aval += Ctrl->A.info.interval;
			if (Ctrl->A.info.mode)	/* No labels */
				cont[n_contours].type = 'C';
			else
				cont[n_contours].type = (fabs (cont[n_contours].val - aval) < noise) ? 'A' : 'C';
			cont[n_contours].angle = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
			cont[n_contours].do_tick = (char)Ctrl->T.active;
		}
	}
#if 0
	/* PS: 4/10/2014: I commented this out as for phase grids in -180/180 range we always missed the 180 contour. */
	if (GMT->current.map.z_periodic && n_contours > 1 && fabs (cont[n_contours-1].val - cont[0].val - 360.0) < GMT_CONV4_LIMIT) {	/* Get rid of redundant contour */
		n_contours--;
	}
#endif
	if (n_contours == 0) {	/* No contours within range of data */
		GMT_Report (API, GMT_MSG_WARNING, "No contours found\n");
		if (make_plot) {	/* Place empty map depending on -B */
			if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
			gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
			gmt_set_basemap_orders (GMT, GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_AFTER, GMT_BASEMAP_ANNOT_AFTER);
			GMT->current.map.frame.order = GMT_BASEMAP_AFTER;	/* Move to last order since only calling gmt_map_basemap once */
			gmt_plotcanvas (GMT);	/* Fill canvas if requested */
			gmt_map_basemap (GMT);
			gmt_plane_perspective (GMT, -1, 0.0);
			gmt_plotend (GMT);
		}
		gmt_M_free (GMT, cont);
		if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
			Return (API->error);
		}
		if (Ctrl->C.info.cpt && GMT_Destroy_Data (API, &P) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}

	/* OK, now we know we have contouring to do */

	z_range = G->header->z_max - G->header->z_min;
	if (Ctrl->C.info.interval == 0.0)
		small = z_range * 1.0e-6;	/* Our gmt_grdfloat noise threshold */
	else
		small = MIN (Ctrl->C.info.interval, z_range) * 1.0e-6;	/* Our gmt_grdfloat noise threshold */
	cont = gmt_M_memory (GMT, cont, n_contours, struct GMT_CONTOUR_INFO);

	if (Ctrl->L.mode) {	/* Set negative or positive range only */
		switch (Ctrl->L.mode) {
			case -2: Ctrl->L.low = G->header->z_min-small; Ctrl->L.high = -small; break;
			case -1: Ctrl->L.low = G->header->z_min-small; Ctrl->L.high = +small; break;
			case +1: Ctrl->L.low = -small; Ctrl->L.high = G->header->z_max+small; break;
			case +2: Ctrl->L.low = +small; Ctrl->L.high = G->header->z_max+small; break;
		}
	}

	gmt_grd_minmax (GMT, G, xyz);
	if (gmt_contlabel_prep (GMT, &Ctrl->contour, xyz)) {	/* Prep for crossing lines, if any */
		gmt_M_free (GMT, cont);
		Return (GMT_RUNTIME_ERROR);
	}

	for (i = 0; i < 3; i++) GMT->current.io.col_type[GMT_OUT][i] = gmt_M_type (GMT, GMT_IN, i);	/* Used if -D is set */

	/* Because we are doing single-precision, we cannot subtract incrementally but must start with the
	 * original grid values and subtract the current contour value. */

	if ((G_orig = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA, G)) == NULL) {
		gmt_M_free (GMT, cont);
		Return (GMT_RUNTIME_ERROR); /* Original copy of grid used for contouring */
	}

	edge = gmt_contour_edge_init (GMT, G->header, &n_edges);

	if (Ctrl->D.active) {
		uint64_t dim[GMT_DIM_SIZE] = {0, 0, 0, 3};
		if (!Ctrl->D.file || !strchr (Ctrl->D.file, '%'))	/* No file given or filename without C-format specifiers means a single output file */
			io_mode = GMT_WRITE_SET;
		else {	/* Must determine the kind of output organization */
			i = 0;
			while (Ctrl->D.file[i]) {
				if (Ctrl->D.file[i++] == '%') {	/* Start of format */
					while (Ctrl->D.file[i] && !strchr ("cdf", Ctrl->D.file[i])) i++;	/* Scan past any format modifiers, like in %7.4f */
					if (Ctrl->D.file[i] == 'c') fmt[0] = i;
					if (Ctrl->D.file[i] == 'd') fmt[1] = i;
					if (Ctrl->D.file[i] == 'f') fmt[2] = i;
					i++;
				}
			}
			n_tables = 1;
			if (fmt[2]) {	/* Want files with the contour level in the name */
				if (fmt[1])	/* f+d[+c]: Write segment files named after contour level and running numbers, with or without C|O modifier */
					io_mode = GMT_WRITE_SEGMENT;
				else {	/* f[+c]: Write one table with all segments for each contour level, possibly one each for C|O */
					io_mode = GMT_WRITE_TABLE;
					tbl_scl = (fmt[0]) ? 2 : 1;
					n_tables = n_contours * tbl_scl;
				}
			}
			else if (fmt[1])	/* d[+c]: Want individual files with running numbers only, with or without C|O modifier  */
				io_mode = GMT_WRITE_SEGMENT;
			else if (fmt[0]) {	/* c: Want two files: one for open and one for closed contours */
				io_mode = GMT_WRITE_TABLE;
				n_tables = 2;
				two_only = true;
			}
		}
		dim[GMT_TBL] = n_tables;
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);	/* An empty dataset */
		n_seg_alloc = gmt_M_memory (GMT, NULL, n_tables, size_t);
		n_seg = gmt_M_memory (GMT, NULL, n_tables, uint64_t);
	}

	gmt_M_memset (rgb, 4, double);

	if (make_plot) {	/* Here we have contours to plot and possibly canvas/basemap plotting to do */
		if (Ctrl->contour.delay) GMT->current.ps.nclip = +2;	/* Signal that this program initiates clipping that will outlive this process */
		if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		gmt_set_basemap_orders (GMT, Ctrl->contour.delay ? GMT_BASEMAP_FRAME_BEFORE : GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_BEFORE, GMT_BASEMAP_ANNOT_BEFORE);
		gmt_plotcanvas (GMT);	/* Fill canvas if requested */
		gmt_map_basemap (GMT);
		gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);

		if (GMT->common.l.active) {	/* Add one or two contour entries to the auto-legend entry under modern mode; examine which pen & label we place */
			char *p = NULL;
			struct GMT_LEGEND_ITEM copy;
			if (strchr ("|/", GMT->common.l.item.label[0])) {	/* Gave single label for contour pen since starting with | or / */
				gmt_M_memcpy (&copy, &(GMT->common.l.item), 1, struct GMT_LEGEND_ITEM);	/* Make an identical copy */
				gmt_strlshift (copy.label, 1U);	/* Remove the leading divider */
				gmt_add_legend_item (API, NULL, false, NULL, true, &(Ctrl->W.pen[PEN_CONT]), &copy);
			}
			else if ((p = strchr (GMT->common.l.item.label, '|')) || (p = strchr (GMT->common.l.item.label, '/'))) {	/* Got two titles */
				char q = p[0];	/* Get the divider character */
				gmt_M_memcpy (&copy, &(GMT->common.l.item), 1, struct GMT_LEGEND_ITEM);	/* Make an identical copy */
				p[0] = '\0';	/* Truncate the second contour label */
				gmt_add_legend_item (API, NULL, false, NULL, true, &(Ctrl->W.pen[PEN_ANNOT]), &(GMT->common.l.item));	/* Place the first annotated contour entry */
				p[0] = q;	/* Restore the label in the original -l setting */
				if (copy.draw & GMT_LEGEND_DRAW_D) copy.draw -= GMT_LEGEND_DRAW_D;	/* Only want to draw one horizontal line (if set), so remove for the 2nd entry */
				gmt_strlshift (copy.label, (size_t)(p - GMT->common.l.item.label)+1);	/* Remove the leading annotated contour label first */
				gmt_add_legend_item (API, NULL, false, NULL, true, &(Ctrl->W.pen[PEN_CONT]), &copy);	/* Place the second regular contour entry */
			}
			else	/* Got a single entry for annotated contours */
				gmt_add_legend_item (API, NULL, false, NULL, true, &(Ctrl->W.pen[PEN_ANNOT]), &(GMT->common.l.item));
		}
	}

	if (Ctrl->Q.active && Ctrl->Q.unit && (strchr (GMT_LEN_UNITS, Ctrl->Q.unit) || Ctrl->Q.unit == 'X')) {	/* Need to compute distances in map units */
		if (gmt_init_distaz (GMT, Ctrl->Q.unit, Ctrl->Q.mode, GMT_MAP_DIST) == GMT_NOT_A_VALID_TYPE)
			Return (GMT_NOT_A_VALID_TYPE);
	}

	for (c = uc = 0; uc < n_contours; c++, uc++) {	/* For each contour value cval */

		if (Ctrl->L.active && (cont[c].val < Ctrl->L.low || cont[c].val > Ctrl->L.high)) continue;	/* Outside desired range */
		if (Ctrl->Q.zero && gmt_M_is_zero (cont[c].val)) continue;	/* Skip zero-contour */
		/* Reset markers and set up new zero-contour */

		cval = cont[c].val;
		id = (cont[c].type == 'A' || cont[c].type == 'a') ? PEN_ANNOT : PEN_CONT;

		if (cont[c].penset) {	/* Per-contour pen specification given */
			Ctrl->contour.line_pen = cont[c].pen;	/* Load contour-specific pen into contour structure */
			if (Ctrl->W.scaling) {	/* Apply global pen-width scaling */
				Ctrl->contour.line_pen.width *= Ctrl->W.scale;
				if (Ctrl->contour.line_pen.width > 0.0 && Ctrl->contour.line_pen.width < Ctrl->W.cutoff) {
					GMT_Report (API, GMT_MSG_INFORMATION, "Skipping the %g contour as pen width (%g) is less than threshold of %g points\n", cval, Ctrl->contour.line_pen.width, Ctrl->W.cutoff);
					continue;	/* The tests for nonzero allows -Wfaint (i.e., -W0) to pass */
				}
			}
		}
		else
			Ctrl->contour.line_pen = Ctrl->W.pen[id];	/* Load current pen into contour structure */

		GMT_Report (API, GMT_MSG_INFORMATION, "Tracing the %g contour\n", cval);

		/* New approach to avoid round-off */

		for (ij = 0; ij < G->header->size; ij++) {
			G->data[ij] = G_orig->data[ij] - (gmt_grdfloat)cval;		/* If there are NaNs they will remain NaNs */
			if (G->data[ij] == 0.0) G->data[ij] += (gmt_grdfloat)small;	  /* There will be no actual zero-values, just -ve and +ve values */
		}

		if (Ctrl->W.cpt_effect) {
			gmt_get_rgb_from_z (GMT, P, cval, rgb);
			if (Ctrl->W.cptmode & 1)	/* Override pen color according to CPT */
				gmt_M_rgb_copy (&Ctrl->contour.line_pen.rgb, rgb);
			if (Ctrl->W.cptmode & 2)	/* Override label color according to CPT */
				gmt_M_rgb_copy (&Ctrl->contour.font_label.fill.rgb, rgb);
		}
		else if ((Ctrl->contour.font_label.set & 1) == 0) /* Did not specify a font color; fault to pen color */
			gmt_M_rgb_copy (&Ctrl->contour.font_label.fill.rgb, Ctrl->contour.line_pen.rgb);
		n_alloc = 0;
		begin = true;

		while ((ns = gmt_contours (GMT, G, Ctrl->S.value, GMT->current.setting.interpolant, Ctrl->F.value, edge, &begin, &x, &y)) > 0) {
			n = (uint64_t)ns;
			closed = gmt_is_closed (GMT, G, x, y, n);	/* Closed interior/periodic boundary contour? */
			is_closed = (closed != cont_is_not_closed);

			if (Ctrl->Q.active) {	/* Avoid plotting short contours based on map length or point count */
				if (Ctrl->Q.unit) {	/* Need length of contour */
					c_length = gmt_line_length (GMT, x, y, n, Ctrl->Q.project);
					use_contour = (c_length >= Ctrl->Q.length);
				}
				else	/* Just based on the point count */
					use_contour = (n >= Ctrl->Q.min);
				if (!use_contour) {	/* Don't want this guy */
					gmt_M_free (GMT, x);
					gmt_M_free (GMT, y);
					GMT_Report (API, GMT_MSG_DEBUG, "Skipping a %g contour due to count/length limit (%g, %d)\n", cval, c_length, n);
					continue;
				}
			}

			if (closed == cont_is_not_closed || use_contour) {	/* Passed our minimum point criteria for closed contours */
				if (Ctrl->D.active && n > 2) {	/* Save the contour as output data */
					S = gmt_prepare_contour (GMT, x, y, n, cval);
					/* Select which table this segment should be added to */
					tbl = (io_mode == GMT_WRITE_TABLE) ? ((two_only) ? is_closed : tbl_scl * c) : 0;
					if (n_seg[tbl] == n_seg_alloc[tbl]) {
						size_t old_n_alloc = n_seg_alloc[tbl];
						D->table[tbl]->segment = gmt_M_memory (GMT, D->table[tbl]->segment, (n_seg_alloc[tbl] += GMT_SMALL_CHUNK), struct GMT_DATASEGMENT *);
						gmt_M_memset (&(D->table[tbl]->segment[old_n_alloc]), n_seg_alloc[tbl] - old_n_alloc, struct GMT_DATASEGMENT *);	/* Set to NULL */
					}
					D->table[tbl]->segment[n_seg[tbl]++] = S;
					D->table[tbl]->n_segments++;	D->n_segments++;
					D->table[tbl]->n_records += n;	D->n_records += n;
					/* Generate a file name and increment cont_counts, if relevant */
					TH = gmt_get_DT_hidden (D->table[tbl]);
					if (io_mode == GMT_WRITE_TABLE && !TH->file[GMT_OUT])
						TH->file[GMT_OUT] = gmt_make_filename (GMT, Ctrl->D.file, fmt, cval, is_closed, cont_counts);
					else if (io_mode == GMT_WRITE_SEGMENT) {
						SH = gmt_get_DS_hidden (S);
						SH->file[GMT_OUT] = gmt_make_filename (GMT, Ctrl->D.file, fmt, cval, is_closed, cont_counts);
					}
				}
				if (make_plot && cont[c].do_tick && is_closed) {	/* Must store the entire contour for later processing */
					/* These are original coordinates that have not yet been projected */
					int extra;
					if (n_save == n_save_alloc) save = gmt_M_malloc (GMT, save, n_save, &n_save_alloc, struct GRDCONTOUR_SAVE);
					extra = (abs (closed) == 2);	/* Need extra slot to temporarily close half-polygons */
					n_alloc = 0;
					gmt_M_memset (&save[n_save], 1, struct GRDCONTOUR_SAVE);
					gmt_M_malloc2 (GMT, save[n_save].x, save[n_save].y, n + extra, &n_alloc, double);
					gmt_M_memcpy (save[n_save].x, x, n, double);
					gmt_M_memcpy (save[n_save].y, y, n, double);
					gmt_M_memcpy (&save[n_save].pen,  &Ctrl->contour.line_pen,   1, struct GMT_PEN);
					gmt_M_memcpy (&save[n_save].font, &Ctrl->contour.font_label, 1, struct GMT_FONT);
					save[n_save].do_it = true;
					save[n_save].cval = cval;
					save[n_save].kind = closed;
					if (extra) { save[n_save].x[n] = save[n_save].x[0];	save[n_save].y[n] = save[n_save].y[0];}
					save[n_save].n = n + extra;
					n_save++;
				}
				if (need_proj && (nn = (unsigned int)gmt_clip_to_map (GMT, x, y, n, &xp, &yp)) != 0) {	/* Lines inside the region */
					/* From here on, xp/yp are map inches */
					if (cont[c].type == 'A' || cont[c].type == 'a') {	/* Annotated contours */
						if (data_is_time) {
							double tval = (use_t_offset) ? cval - t_offset : cval;
							char *c = NULL;
							gmt_ascii_format_col (GMT, cont_label, tval, GMT_OUT, GMT_Z);
							if ((c = strchr (cont_label, 'T')) != NULL) c[0] = ' ';	/* Replace ISO T with space for plots */
						}
						else {
							gmt_get_format (GMT, cval, Ctrl->contour.unit, NULL, format);
							sprintf (cont_label, format, cval);
						}
					}
					else
						cont_label[0] = '\0';
					gmt_hold_contour (GMT, &xp, &yp, nn, cval, cont_label, cont[c].type, cont[c].angle, closed == cont_is_closed, true, &Ctrl->contour);
					gmt_M_free (GMT, xp);
					gmt_M_free (GMT, yp);
					n_cont_attempts++;
				}
			}
			gmt_M_free (GMT, x);
			gmt_M_free (GMT, y);
		}
		if (ns < 0) Return (-ns);
	}
	gmt_M_free (GMT, edge);

	if (make_plot && n_cont_attempts == 0) GMT_Report (API, GMT_MSG_INFORMATION, "No contours drawn, check your -A, -C, -L settings?\n");

	if (Ctrl->D.active) {	/* Write the contour line output file(s) */
		gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
		if ((error = GMT_Set_Columns (API, GMT_OUT, 3, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			gmt_M_free (GMT, n_seg_alloc);
			gmt_M_free (GMT, n_seg);
			Return (error);
		}
		for (tbl = 0; tbl < D->n_tables; tbl++) D->table[tbl]->segment = gmt_M_memory (GMT, D->table[tbl]->segment, n_seg[tbl], struct GMT_DATASEGMENT *);
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, io_mode, NULL, Ctrl->D.file, D) != GMT_NOERROR) {
			gmt_M_free (GMT, n_seg_alloc);
			gmt_M_free (GMT, n_seg);
			Return (API->error);
		}
		gmt_M_free (GMT, n_seg_alloc);
		gmt_M_free (GMT, n_seg);
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	if (data_is_time) {	/* Undo earlier swapparoo */
		/* Restore original date and clock output formatting structures */
		gmt_M_memcpy (&GMT->current.io.clock_output, &Clock, 1, struct GMT_CLOCK_IO);
		gmt_M_memcpy (&GMT->current.io.date_output, &Date, 1, struct GMT_DATE_IO);
	}

	if (make_plot) label_mode |= 1;		/* Would want to plot ticks and labels if -T is set */
	if (Ctrl->contour.save_labels) {	/* Want to save the contour label locations (lon, lat, angle, label) if -T is set */
		label_mode |= 2;
		if ((error = gmt_contlabel_save_begin (GMT, &Ctrl->contour)) != 0) Return (error);
	}

	if (make_plot) PSL_setdash (PSL, NULL, 0.0);

	if (Ctrl->T.active && n_save) {	/* Finally sort and plot ticked innermost contours and plot/save L|H labels */
		save = gmt_M_memory (GMT, save, n_save, struct GRDCONTOUR_SAVE);

		grdcontour_sort_and_plot_ticks (GMT, PSL, save, n_save, G_orig, &Ctrl->T.info, label_mode, Ctrl->contour.Out);
		for (i = 0; i < n_save; i++) {
			gmt_M_free (GMT, save[i].x);
			gmt_M_free (GMT, save[i].y);
		}
		gmt_M_free (GMT, save);
	}

	if (make_plot) {
		/* Must possibly adjust label angles so that label is readable when following contours */
		if (Ctrl->contour.hill_label) grdcontour_adjust_hill_label (GMT, &Ctrl->contour, G);

		gmt_contlabel_plot (GMT, &Ctrl->contour);

		if (!Ctrl->contour.delay)
			gmt_map_clip_off (GMT);
		gmt_map_basemap (GMT);

		gmt_plane_perspective (GMT, -1, 0.0);

		gmt_plotend (GMT);
	}

	if (Ctrl->contour.save_labels) {	/* Close file with the contour label locations (lon, lat, angle, label) */
		if ((error = gmt_contlabel_save_end (GMT, &Ctrl->contour)) != 0) {
			gmt_M_free (GMT, cont);
			Return (error);
		}
	}

	if (make_plot || Ctrl->contour.save_labels) gmt_contlabel_free (GMT, &Ctrl->contour);

	if (mem_G) gmt_M_memcpy (G->data, G_orig->data, G->header->size, gmt_grdfloat);		/* To avoid messing up an input memory grid */

	if (GMT_Destroy_Data (GMT->parent, &G_orig) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to free G_orig\n");
	}
	gmt_M_free (GMT, cont);

	Return (GMT_NOERROR);
}
