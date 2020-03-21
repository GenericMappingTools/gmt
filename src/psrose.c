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
 * Brief synopsis: psrose reads a file [or standard input] with azimuth and length information
 * and draws a sector or rose diagram.  Several options for plot layout are available.
 * 2 diagrams are possible: Full circle (360) or half circle (180).  In the
 * latter case azimuths > 180 are reversed (-= 180).
 *
 * To be compatible with GMT, I assume radial distance to be "x"
 * and azimuth to be "y".  Hence, west = 0.0 and east = max_radius
 * south/north is -90,90 for halfcircle and 0,360 for full circle
 *
 * Update Nov 2017 PW: psrose map machinery is ancient and did not support -J.  For modern
 * mode we do need -J support.  To remain backwards compatible we now take these steps:
 *  a) Before parsing of arguments we check if -J was given.  If not we find the radius
 *     from -S (if given, else default to 3i) and create a -JX option that we add. If
 *     normalization was set via -S then we retain a plain -S option that now only means
 *     do normalization.
 *  b) psrose then processes -JX and optionally -S. Because -JX is then parsed we must be
 *     on the look-out for any -: as these are inert.  We reactivate that swap if needed.
 *  c) We then extract the radius from the -JX string, decode it, and reestablish the old
 *     -Jx1 scaling using the +/- radius as w/e/s/n.
 *  d) The old syntax is undocumented but support by backwards compatibility through GMT 6.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psrose"
#define THIS_MODULE_MODERN_NAME	"rose"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot a polar histogram (rose, sector, windrose diagrams)"
#define THIS_MODULE_KEYS	"<D{,CC(,ED(,>X},>D),>DI@<D{,ID),CC("
#define THIS_MODULE_NEEDS	"JR"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYbdehipqstxy" GMT_OPT("c")

struct PSROSE_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -A<sector_angle>[+r] */
		bool active;
		bool rose;
		double inc;
	} A;
	struct C {	/* -C<cpt> */
		bool active;
		char *file;
	} C;
	struct D {	/* -D */
		bool active;
	} D;
	struct E {	/* -Em|[+w]<modefile> */
		bool active;
		bool mode;
		bool mean;
		char *file;
	} E;
	struct F {	/* -F */
		bool active;
	} F;
	struct G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I */
		bool active;
	} I;
	struct L {	/* -L */
		bool active;
		char *w, *e, *s, *n;
	} L;
	struct M {	/* -M[<size>][<modifiers>] */
		bool active;
		struct GMT_SYMBOL S;
	} M;
	struct N {	/* -N */
		bool active;
	} N;
	struct Q {	/* -Q<alpha> */
		bool active;
		double value;
	} Q;
	struct S {	/* -S */
		bool active;
		bool normalize;
		double scale;	/* Get this via -JX */
	} S;
	struct T {	/* -T */
		bool active;
	} T;
	struct W {	/* -W[v]<pen> */
		bool active[2];
		struct GMT_PEN pen[2];
	} W;
	struct Z {	/* -Zu|<scale> */
		bool active;
		unsigned int mode;
		double scale;
	} Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSROSE_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct PSROSE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	gmt_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);
	C->M.S.symbol = PSL_VECTOR;
	C->W.pen[0] = C->W.pen[1] = GMT->current.setting.map_default_pen;
	C->Q.value = 0.05;
	C->Z.scale = 1.0;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSROSE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->E.file);
	gmt_M_str_free (C->L.w);
	gmt_M_str_free (C->L.e);
	gmt_M_str_free (C->L.s);
	gmt_M_str_free (C->L.n);
	gmt_M_free (GMT, C);
}

GMT_LOCAL double Critical_Resultant (double alpha, int n) {
	/* Return critial resultant for given alpha and sample size.
	 * Based on Rayleigh test for uniformity as approximated by Zaar [1999]
	 * and reported by Berens [2009] in CircStat (MATLAB).  Valid for
	 * n >= 10 and for first 3 decimals (gets better with n). */
	double Rn;
	Rn = 0.5 * sqrt ((1.0 + 4.0 * n * (1.0 + n) - pow (log (alpha) + 1.0 + 2.0 * n, 2.0))) / n;
	return (Rn);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	char *choice[2] = {"OFF", "ON"};

	/* This displays the psrose synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] [-A<sector_angle>[+r]] [%s] [-C<cpt>] [-D] [-E[m|[+w]<modefile>]] [-F] [-G<fill>] [-I]\n", name, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-JX<diameter>] %s[-L[<wlab>,<elab>,<slab>,<nlab>]] [-M[<size>][<modifiers>]] [-N] %s%s[-Q<alpha>]\n", API->K_OPT, API->O_OPT, API->P_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-R<r0>/<r1>/<theta0>/<theta1>] [-S] [-T] [%s]\n", GMT_U_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-W[v]<pen>] [%s] [%s]\n\t[-Zu|<scale>] [%s] %s[%s] [%s]\n\t[%s] [%s]\n\t[%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_bi_OPT, API->c_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_s_OPT, GMT_t_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Sector width in degrees for sector diagram [Default is windrose];\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +r to get rose diagram.\n");
	GMT_Option (API, "B-");
	if (gmt_M_showusage (API)) {
		GMT_Message (API, GMT_TIME_NONE, "\t   The scale bar length is set to the radial gridline spacing.\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   (Remember: radial is x-direction, azimuthal is y-direction).\n");
	}
	GMT_Message (API, GMT_TIME_NONE, "\t-C Use CPT to assign fill to sectors based on the r-value. Requires -A (sector diagram)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Will center the sectors.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Plot vectors listed in the <modefile> file. For calculated mean direction, choose -Em.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To write the calculated mean direction etc. to file, append +w<modfile>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Do not draw the scale length bar [Default plots scale in lower right corner].\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify color for diagram [Default is no fill].");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Inquire mode; only compute and report statistics - no plot is created.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-J Use -JX<diameter> to set the plot diameter [7.5c].\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Override default labels [West,East,South,North (depending on GMT_LANGUAGE)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   for full circle and 90W,90E,-,0 for half-circle].  If no argument \n");
	GMT_Message (API, GMT_TIME_NONE, "\t   is given then labels will be disabled.  Give - to disable an individual label.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Specify arrow attributes.  If -E is used then the attributes apply to the -E vector(s).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Otherwise, if windrose mode is selected we apply vector attributes to individual directions.\n");
	gmt_vector_syntax (API->GMT, 15);
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is %gp+gblack+p1p.\n", VECTOR_HEAD_LENGTH);
	GMT_Message (API, GMT_TIME_NONE, "\t-N Normalize rose plots for area, i.e., take sqrt(r) before plotting [no normalization].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Only applicable if normalization has been specified with -Sn<radius>.\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Set confidence level for Rayleigh test for uniformity [0.05].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-R Specifies the region (<r0> = 0, <r1> = max_radius).  For azimuth:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify <theta0>/<theta1> = -90/90 or 0/180 (half-circles) or 0/360 only).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <r0> = <r1> = 0, psrose will compute a reasonable <r1> value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Normalize data, i.e., divide all radii (or bin counts) by the maximum radius (or count).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Indicate that the vectors are oriented (two-headed), not directed [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This implies both <azimuth> and <azimuth> + 180 will be counted as inputs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Ignored if -R sets a half-circle domain.\n");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Set pen attributes for outline of rose [Default is no outline].", 0);
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Wv<pen> to set a different pen for the vector (requires -E) [Same as rose outline].\n");
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Multiply the radii by <scale> before plotting; use -Zu to set input radii to 1.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-: Expect (azimuth,radius) input rather than (radius,azimuth) [%s].\n", choice[API->GMT->current.setting.io_lonlat_toggle[GMT_IN]]);
	GMT_Option (API, "bi2,c,di,e,h,i,p,qi,s,t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSROSE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psrose and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int n;
	unsigned int n_errors = 0, k;
	double range;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, txt_d[GMT_LEN256] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Get Sector angle in degrees -A<inc>[+r]*/
				Ctrl->A.active = true;
				k = 0;
				if ((c = strstr (opt->arg, "+r"))) {
					Ctrl->A.rose = true;
					c[0] = '\0';	/* Chop off modifier */
				}
				else if (strchr (opt->arg, 'r')) {	/* Old syntax -A[r]<inc> */
					Ctrl->A.rose = true;
					if (opt->arg[0] == 'r') k = 1;
				}
				Ctrl->A.inc = atof (&opt->arg[k]);
				if (c) c[0] = '+';	/* Restore modifier */
				break;
			case 'C':
				if (gmt_M_compat_check (GMT, 5)) {	/* Need to check for deprecated -Cm|[+w]<modefile> option */
					if (((c = strstr (opt->arg, "+w")) || (opt->arg[0] == 'm' && opt->arg[1] == '\0') || opt->arg[0] == '\0') && strstr (opt->arg, ".cpt") == NULL) {
						GMT_Report (API, GMT_MSG_COMPAT, "Option -C for mode-vector(s) is deprecated; use -E instead.\n");
						Ctrl->E.active = true;
						if ((c = strstr (opt->arg, "+w"))) {	/* Wants to write out mean direction */
							gmt_M_str_free (Ctrl->E.file);
							if (c[2]) Ctrl->E.file = strdup (&c[2]);
							Ctrl->E.mode = GMT_OUT;
							c[0] = '\0';	/* Chop off temporarily */
						}
						if ((opt->arg[0] == 'm' && opt->arg[1] == '\0') || (API->external == 0 && opt->arg[0] == '\0'))
							Ctrl->E.mean = true;
						else if (Ctrl->E.mode == GMT_IN) {
							gmt_M_str_free (Ctrl->E.file);
							if (opt->arg[0]) Ctrl->E.file = strdup (opt->arg);
						}
						break;
					}
				}
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Center the bins */
				Ctrl->D.active = true;
				break;
			case 'E':	/* Read mode file and plot mean directions */
				Ctrl->E.active = true;
				if ((c = strstr (opt->arg, "+w"))) {	/* Wants to write out mean direction */
					gmt_M_str_free (Ctrl->E.file);
					if (c[2]) Ctrl->E.file = strdup (&c[2]);
					Ctrl->E.mode = GMT_OUT;
					c[0] = '\0';	/* Chop off temporarily */
				}
				if ((opt->arg[0] == 'm' && opt->arg[1] == '\0') || (API->external == 0 && opt->arg[0] == '\0'))
					Ctrl->E.mean = true;
				else if (Ctrl->E.mode == GMT_IN) {
					gmt_M_str_free (Ctrl->E.file);
					if (opt->arg[0]) Ctrl->E.file = strdup (opt->arg);
				}
				break;
			case 'F':	/* Disable scalebar plotting */
				Ctrl->F.active = true;
				break;
			case 'G':	/* Set Gray shade */
				Ctrl->G.active = true;
				if (gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'I':	/* Compute statistics only - no plot */
				Ctrl->I.active = true;
				break;
			case 'L':	/* Overwride default labeling */
				Ctrl->L.active = true;
				if (opt->arg[0]) {
					unsigned int n_comma = 0;
					for (k = 0; k < strlen (opt->arg); k++) if (opt->arg[k] == ',') n_comma++;
					if (n_comma == 3)	/* New, comma-separated labels */
						n_errors += gmt_M_check_condition (GMT, sscanf (opt->arg, "%[^,],%[^,],%[^,],%s", txt_a, txt_b, txt_c, txt_d) != 4, "Option -L: Expected\n\t-L<westlabel>,<eastlabel>,<southlabel>,<northlabel>\n");
					else	/* Old slash-separated labels */
						n_errors += gmt_M_check_condition (GMT, sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d) != 4, "Option -L: Expected\n\t-L<westlabel>/<eastlabel>/<southlabel>/<northlabel>\n");
					Ctrl->L.w = strdup (txt_a);	Ctrl->L.e = strdup (txt_b);
					Ctrl->L.s = strdup (txt_c);	Ctrl->L.n = strdup (txt_d);
				}
				else {	/* Turn off all 4 labels */
					Ctrl->L.w = strdup ("-");	Ctrl->L.e = strdup ("-");
					Ctrl->L.s = strdup ("-");	Ctrl->L.n = strdup ("-");
				}
				break;
			case 'M':	/* Get arrow parameters */
				Ctrl->M.active = true;
				if (gmt_M_compat_check (GMT, 4) && (strchr (opt->arg, '/') && !strchr (opt->arg, '+'))) {	/* Old-style args */
					n = sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
					if (n != 4 || gmt_getrgb (GMT, txt_d, Ctrl->M.S.v.fill.rgb)) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -M: Expected\n\t-M<tailwidth/headlength/headwidth/<color>>\n");
						n_errors++;
					}
					else {	/* Turn the old args into new +a<angle> and pen width */
						Ctrl->M.S.v.status = PSL_VEC_END + PSL_VEC_FILL + PSL_VEC_OUTLINE;
						Ctrl->M.S.size_x = VECTOR_HEAD_LENGTH * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 9p */
						Ctrl->M.S.v.h_length = (float)Ctrl->M.S.size_x;	/* 9p */
						Ctrl->M.S.v.v_angle = 60.0f;
						Ctrl->M.S.v.pen = GMT->current.setting.map_default_pen;
						Ctrl->W.active[1] = true;
						//Ctrl->W.pen[1].width = gmt_M_to_points (GMT, txt_a);
						Ctrl->M.S.v.v_width = (float)gmt_M_to_inch (GMT, txt_a);
						Ctrl->M.S.v.h_length = (float)gmt_M_to_inch (GMT, txt_b);
						Ctrl->M.S.v.h_width = (float)gmt_M_to_inch (GMT, txt_c);
						Ctrl->M.S.v.v_angle = (float)atand (0.5 * Ctrl->M.S.v.h_width / Ctrl->M.S.v.h_length);
						Ctrl->M.S.v.status |= (PSL_VEC_OUTLINE + PSL_VEC_FILL);
						Ctrl->M.S.v.status |= PSL_VEC_FILL2;
					}
					Ctrl->M.S.symbol = GMT_SYMBOL_VECTOR_V4;
				}
				else {
					if (opt->arg[0] == '+' || opt->arg[0] == '\0') {	/* No size argument (use default), just attributes */
						n_errors += gmt_parse_vector (GMT, 'v', opt->arg, &Ctrl->M.S);
					}
					else {	/* Size, plus possible attributes */
						n = sscanf (opt->arg, "%[^+]%s", txt_a, txt_b);	/* txt_a should be symbols size with any +<modifiers> in txt_b */
						if (n == 1) txt_b[0] = 0;	/* No modifiers present, set txt_b to empty */
						Ctrl->M.S.size_x = gmt_M_to_inch (GMT, txt_a);	/* Length of vector */
						n_errors += gmt_parse_vector (GMT, 'v', txt_b, &Ctrl->M.S);
					}
					Ctrl->M.S.v.status |= PSL_VEC_OUTLINE;
				}
				break;
			case 'N':	/* Make sectors area be proportional to frequency instead of radius */
				Ctrl->N.active = true;
				break;
			case 'Q':	/* Set critical value [0.05] */
				Ctrl->Q.active = true;
				if (opt->arg[0]) Ctrl->Q.value = atof (opt->arg);
				break;
			case 'S':	/* Normalization */
				Ctrl->S.active = true;
				Ctrl->S.normalize = true;
				break;
			case 'T':	/* Oriented instead of directed data */
				Ctrl->T.active = true;
				break;
			case 'W':	/* Get pen width for outline */
				n = (opt->arg[0] == 'v') ? 1 : 0;
				Ctrl->W.active[n] = true;
				if (gmt_getpen (GMT, &opt->arg[n], &Ctrl->W.pen[n])) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", 0);
					n_errors++;
				}
				break;
			case 'Z':	/* Scale radii before using data */
				Ctrl->Z.active = true;
				if (opt->arg[0] == 'u')
					Ctrl->Z.mode = 1;
				else
					Ctrl->Z.scale = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	/* Check that the options selected are mutually consistent */

	GMT->common.R.wesn[XLO] = 0.0;
	range = GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO];
	if (doubleAlmostEqual (range, 180.0) && Ctrl->T.active) {
		GMT_Report (API, GMT_MSG_WARNING, "-T only needed for 0-360 range data (ignored)");
		Ctrl->T.active = false;
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->E.file && Ctrl->E.mode == GMT_IN && gmt_access (GMT, Ctrl->E.file, R_OK),
	                                 "Option -E: Cannot read file %s!\n", Ctrl->E.file);
	n_errors += gmt_M_check_condition (GMT, gmt_M_is_zero (Ctrl->Z.scale), "Option -Z: factor must be nonzero\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.inc < 0.0, "Option -A: sector width must be positive\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.value <= 0.0 || Ctrl->Q.value >= 1.0, "Option -Q: confidence level must be in 0-1 range\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->G.active, "Option -C: Cannot give both -C and -G\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->A.rose, "Option -C: Cannot be used with -A+r\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && !Ctrl->A.active, "Option -C: Requires -A\n");
	if (GMT->common.J.active) {	/* Impose our conditions on -JX */
		n_errors += gmt_M_check_condition (GMT, GMT->common.J.string[0] != 'X', "Option -J: Must specify -JX<diameter>\n");
		n_errors += gmt_M_check_condition (GMT, strchr (GMT->common.J.string, '/'), "Option -J: Must specify -JX<diameter>\n");
	}
	if (!Ctrl->I.active) {
		n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Must specify -JX option\n");
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
		n_errors += gmt_M_check_condition (GMT, !((GMT->common.R.wesn[YLO] == -90.0 && GMT->common.R.wesn[YHI] == 90.0) \
			|| (GMT->common.R.wesn[YLO] == 0.0 && GMT->common.R.wesn[YHI] == 180.0)
			|| (GMT->common.R.wesn[YLO] == 0.0 && GMT->common.R.wesn[YHI] == 360.0)),
				"Option -R: theta0/theta1 must be either -90/90, 0/180 or 0/360\n");
	}
	n_errors += gmt_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_rose (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: rose\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psrose (V_API, mode, args);
}

int GMT_psrose (void *V_API, int mode, void *args) {
	bool do_fill = false, automatic = false, sector_plot = false, windrose = true;
	unsigned int n_bins, n_modes = 0, form, n_in, half_only = 0, bin;
	int error = 0, k, n_annot, n_alpha, sbin, significant, index;

	uint64_t n = 0, i;

	size_t n_alloc = GMT_CHUNK;

	char text[GMT_BUFSIZ] = {""}, format[GMT_BUFSIZ] = {""};

	double max = 0.0, radius, az, x_origin, y_origin, tmp, one_or_two = 1.0, s, c, f;
	double angle1, angle2, angle, x, y, mean_theta, mean_radius, xr = 0.0, yr = 0.0;
	double x1, x2, y1, y2, total = 0.0, total_arc, off, max_radius, az_offset, start_angle;
	double asize, lsize, this_az, half_bin_width, diameter, wesn[4], mean_vector, mean_resultant;
	double *xx = NULL, *yy = NULL, *in = NULL, *sum = NULL, *azimuth = NULL, critical_resultant;
	double *length = NULL, *mode_direction = NULL, *mode_length = NULL, dim[PSL_MAX_DIMS], rgb[4];

	struct PSROSE_CTRL *Ctrl = NULL;
	struct GMT_FILL *F = NULL;
	struct GMT_DATASET *Cin = NULL;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_RECORD *In = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psrose main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	asize = GMT->current.setting.font_annot[GMT_PRIMARY].size * GMT->session.u2u[GMT_PT][GMT_INCH];
	lsize = GMT->current.setting.font_annot[GMT_PRIMARY].size * GMT->session.u2u[GMT_PT][GMT_INCH];
	gmt_M_memset (dim, PSL_MAX_DIMS, double);

	max_radius = GMT->common.R.wesn[XHI];
	if (doubleAlmostEqual (GMT->common.R.wesn[YLO], -90.0))
		half_only = 1;
	else if (doubleAlmostEqual (GMT->common.R.wesn[YHI], 180.0))
		half_only = 2;
	if (Ctrl->A.rose) windrose = false;
	sector_plot = (Ctrl->A.inc > 0.0);
	if (sector_plot) windrose = false;	/* Draw rose diagram instead of sector diagram */
	if (!Ctrl->S.normalize) Ctrl->N.active = false;	/* Only do this if data is normalized for length also */
	if (!Ctrl->I.active && !GMT->common.R.active[RSET]) automatic = true;
	if (Ctrl->T.active) one_or_two = 2.0;
	half_bin_width = Ctrl->D.active * Ctrl->A.inc * 0.5;
	if (half_only == 1) {
		total_arc = 180.0;
		az_offset = 90.0;
		start_angle = 90.0;
	}
	else if (half_only == 2) {
		total_arc = 180.0;
		az_offset = 0.0;
		start_angle = 180.0;
	}
	else {
		total_arc = 360.0;
		az_offset = 0.0;
		start_angle = 90.0;
	}
	n_bins = (Ctrl->A.inc <= 0.0) ? 1U : urint (total_arc / Ctrl->A.inc);

	/* Read data and do some stats */

	if (Ctrl->C.active && (P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
		Return (GMT_DATA_READ_ERROR);
	}

	n = 0;
	n_in = (GMT->common.i.select && GMT->common.i.n_cols == 1) ? 1 : 2;

	if ((error = GMT_Set_Columns (API, GMT_IN, n_in, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	/* Allocate arrays */

	sum = gmt_M_memory (GMT, NULL, n_bins, double);
	xx = gmt_M_memory (GMT, NULL, n_bins+2, double);
	yy = gmt_M_memory (GMT, NULL, n_bins+2, double);
	azimuth = gmt_M_memory (GMT, NULL, n_alloc, double);
	length = gmt_M_memory (GMT, NULL, n_alloc, double);

	/* Because of -JX being parsed already, any -: will have no effect.  For backwards compatibility we
	 * check if -: was given and turn that on again here before reading */
	if (GMT->common.colon.active)
		GMT->current.setting.io_lonlat_toggle[GMT_IN] = true;

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) { 	/* Bail if there are any read errors */
				gmt_M_free (GMT, length);		gmt_M_free (GMT, xx);	gmt_M_free (GMT, sum);
				gmt_M_free (GMT, azimuth);		gmt_M_free (GMT, yy);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}

		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */

		if (n_in == 2) {	/* Read azimuth and length */
			length[n]  = in[GMT_X];
			azimuth[n] = in[GMT_Y];
			if (Ctrl->Z.active) {
				if (Ctrl->Z.mode) length[n] = 1.0;
				else if (Ctrl->Z.scale != 1.0) length[n] *= Ctrl->Z.scale;
			}
		}
		else {	/* Only read azimuth; set length = weight = 1 */
			length[n]  = 1.0;
			azimuth[n] = in[GMT_X];
		}

		/* Make sure azimuth is in 0 <= az < 360 range */

		while (azimuth[n] < 0.0)    azimuth[n] += 360.0;
		while (azimuth[n] >= 360.0) azimuth[n] -= 360.0;

		if (half_only == 1) {	/* Flip azimuths about E-W line i.e. -90 < az <= 90 */
			if (azimuth[n] > 90.0 && azimuth[n] <= 270.0) azimuth[n] -= 180.0;
			if (azimuth[n] > 270.0) azimuth[n] -= 360.0;
		}
		else if (half_only == 2) {	/* Flip azimuths about N-S line i.e. 0 < az <= 180 */
			if (azimuth[n] > 180.0) azimuth[n] -= 180.0;
		}
		else if (Ctrl->T.active) {
			azimuth[n] = 0.5 * fmod (2.0 * azimuth[n], 360.0);
		}

		/* Double angle to find mean azimuth */

		sincosd (one_or_two * azimuth[n], &s, &c);
		xr += length[n] * c;
		yr += length[n] * s;

		total += length[n];

		n++;
		if (n == n_alloc) {	/* Get more memory */
			n_alloc <<= 1;
			azimuth = gmt_M_memory (GMT, azimuth, n_alloc, double);
			length = gmt_M_memory (GMT, length, n_alloc, double);
		}
	} while (true);

	if (Ctrl->A.inc > 0.0) {	/* Sum up sector diagram info */
		for (i = 0; i < n; i++) {
			if (Ctrl->D.active) {	/* Center bin by removing half bin width here */
				this_az = azimuth[i] - half_bin_width;
				if (!half_only && this_az < 0.0) this_az += 360.0;
				if (half_only == 1 && this_az < -90.0) this_az += 180.0;
				if (half_only == 2 && this_az <   0.0) this_az += 180.0;
			}
			else
				this_az = azimuth[i];
			sbin = irint (floor ((this_az + az_offset) / Ctrl->A.inc));
			assert (sbin >= 0);
			bin = sbin;
			if (bin == n_bins) {
				bin = 0;
			}
			assert (bin < n_bins);
			sum[bin] += length[i];
			if (Ctrl->T.active) {	/* Also count the other end of the orientation */
				this_az += 180.0;	if (this_az >= 360.0) this_az -= 360.0;
				bin = irint (floor ((this_az + az_offset) / Ctrl->A.inc));
				sum[bin] += length[i];
			}
		}
	}

	mean_theta = d_atan2d (yr, xr) / one_or_two;
	if (mean_theta < 0.0) mean_theta += 360.0;
	mean_vector = hypot (xr, yr) / n;
	mean_resultant = mean_radius = hypot (xr, yr) / total;
	critical_resultant = Critical_Resultant (Ctrl->Q.value, (int)n);
	significant = (mean_resultant > critical_resultant);
	if (!Ctrl->S.normalize) mean_radius *= max_radius;

	if (Ctrl->A.inc > 0.0) {	/* Find max of the bins */
		for (bin = 0; bin < n_bins; bin++) if (sum[bin] > max) max = sum[bin];
		if (Ctrl->S.normalize) for (bin = 0; bin < n_bins; bin++) sum[bin] /= max;
		if (Ctrl->N.active) for (bin = 0; bin < n_bins; bin++) sum[bin] = sqrt (sum[bin]);
	}
	else {	/* Find max length of individual vectors */
		for (i = 0; i < n; i++) if (length[i] > max) max = length[i];
		if (Ctrl->S.normalize)
			for (i = 0; i < n; i++) length[i] /= max;
	}

	if (Ctrl->I.active || gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		char *kind[2] = {"r", "bin sum"};
		sprintf (format, "Info for data: n = %% " PRIu64 " mean az = %s mean r = %s mean resultant length = %s max %s = %s scaled mean r = %s linear length sum = %s sign@%%.2f = %%d\n",
			GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, kind[Ctrl->A.active],
			GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, format, n, mean_theta, mean_vector, mean_resultant, max, mean_radius, total, Ctrl->Q.value, significant);
		if (Ctrl->I.active) {	/* That was all we needed to do, wrap up */
			double out[7];
			unsigned int col_type[2];
			struct GMT_RECORD *Rec = gmt_new_record (GMT, out, NULL);
			gmt_M_memcpy (col_type, GMT->current.io.col_type[GMT_OUT], 2U, unsigned int);	/* Save first 2 current output col types */
			gmt_set_column (GMT, GMT_OUT, GMT_X, GMT_IS_FLOAT);
			gmt_set_column (GMT, GMT_OUT, GMT_Y, GMT_IS_FLOAT);
			if ((error = GMT_Set_Columns (API, GMT_OUT, 7U, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
				gmt_M_free (GMT, sum);
				gmt_M_free (GMT, xx);
				gmt_M_free (GMT, yy);
				gmt_M_free (GMT, azimuth);
				gmt_M_free (GMT, length);
				Return (error);
			}
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
				gmt_M_free (GMT, sum);
				gmt_M_free (GMT, xx);
				gmt_M_free (GMT, yy);
				gmt_M_free (GMT, azimuth);
				gmt_M_free (GMT, length);
				Return (API->error);
			}
			if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {
				gmt_M_free (GMT, sum);
				gmt_M_free (GMT, xx);
				gmt_M_free (GMT, yy);
				gmt_M_free (GMT, azimuth);
				gmt_M_free (GMT, length);
				Return (API->error);	/* Enables data output and sets access mode */
			}
			if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_NONE) != GMT_NOERROR) {	/* Sets output geometry */
				gmt_M_free (GMT, sum);
				gmt_M_free (GMT, xx);
				gmt_M_free (GMT, yy);
				gmt_M_free (GMT, azimuth);
				gmt_M_free (GMT, length);
				Return (API->error);
			}
			sprintf (format, "n\tmean_az\tmean_r\tmean_resultant_length\tmax\tscaled_mean_r\tlinear_length_sum");
			out[0] = (double)n; out[1] = mean_theta;	out[2] = mean_vector;	out[3] = mean_resultant;
			out[4] = max;	out[5] = mean_radius;	out[6] = total;
			GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, format);	/* Write this to output if -ho */
			GMT_Put_Record (API, GMT_WRITE_DATA, Rec);
			gmt_M_free (GMT, Rec);
			gmt_M_free (GMT, sum);
			gmt_M_free (GMT, xx);
			gmt_M_free (GMT, yy);
			gmt_M_free (GMT, azimuth);
			gmt_M_free (GMT, length);
			if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
				Return (API->error);
			}
			gmt_M_memcpy (GMT->current.io.col_type[GMT_OUT], col_type, 2U, unsigned int);	/* Restore 2 current output col types */
			Return (GMT_NOERROR);
		}
	}

	if (automatic) {
		if (gmt_M_is_zero (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval)) {
			tmp = pow (10.0, floor (d_log10 (GMT, max)));
			if ((max / tmp) < 3.0) tmp *= 0.5;
		}
		else
			tmp = GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval;
		max_radius = ceil (max / tmp) * tmp;
		if (gmt_M_is_zero (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval) || gmt_M_is_zero (GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval)) {	/* Tickmarks not set */
			GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval = GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval = tmp;
			GMT->current.map.frame.draw = true;
		}
	}

	if (GMT->current.map.frame.draw && gmt_M_is_zero (GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval)) GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval = GMT->current.map.frame.axis[GMT_Y].item[GMT_GRID_UPPER].interval = 30.0;

	/* Ready to plot.  So set up GMT projections (not used by psrose), we set region to actual plot width and scale to 1 */

	Ctrl->S.scale = 0.5 * gmt_M_to_inch (GMT, &GMT->common.J.string[1]);	/* Get radius from full width */
	GMT->common.J.active = false;	/* Reset projection machinery */
	gmt_parse_common_options (GMT, "J", 'J', "x1i");
	GMT->common.R.active[RSET] = GMT->common.J.active = true;
	wesn[XLO] = wesn[YLO] = -Ctrl->S.scale;	wesn[XHI] = wesn[YHI] = Ctrl->S.scale;
	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, wesn), "")) {
		gmt_M_free (GMT, length);		gmt_M_free (GMT, xx);	gmt_M_free (GMT, sum);
		gmt_M_free (GMT, azimuth);		gmt_M_free (GMT, yy);
		Return (GMT_PROJECTION_ERROR);
	}

	if (GMT->current.map.frame.paint) {	/* Until psrose uses a polar projection we must bypass the basemap fill and do it ourself here */
		GMT->current.map.frame.paint = false;	/* Turn off so gmt_plotinit won't fill */
		do_fill = true;
	}
	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);

	x_origin = Ctrl->S.scale;	y_origin = ((half_only) ? 0.0 : Ctrl->S.scale);
	diameter = 2.0 * Ctrl->S.scale;
	PSL_setorigin (PSL, x_origin, y_origin, 0.0, PSL_FWD);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */

	if (!Ctrl->S.normalize) Ctrl->S.scale /= max_radius;

	if (do_fill) {	/* Until psrose uses a polar projection we must bypass the basemap fill and do it ourself here */
		double dim = 2.0 * Ctrl->S.scale;
		GMT->current.map.frame.paint = true;	/* Restore original setting */
		if (half_only) {	/* Clip the bottom half of the circle */
			double xc[4], yc[4];
			xc[0] = xc[3] = -Ctrl->S.scale;	xc[1] = xc[2] = Ctrl->S.scale;
			yc[0] = yc[1] = 0.0;	yc[2] = yc[3] = Ctrl->S.scale;
			PSL_beginclipping (PSL, xc, yc, 4, GMT->session.no_rgb, 3);
		}
		gmt_setfill (GMT, &GMT->current.map.frame.fill, 0);
		PSL_plotsymbol (PSL, 0.0, 0.0, &dim, PSL_CIRCLE);
		if (half_only) PSL_endclipping (PSL, 1);		/* Reduce polygon clipping by one level */
	}
	if (GMT->common.B.active[0]) {	/* Draw frame */
		int symbol = (half_only) ? PSL_WEDGE : PSL_CIRCLE;
		double dim[PSL_MAX_DIMS];
		struct GMT_FILL no_fill;

		gmt_init_fill (GMT, &no_fill, -1.0, -1.0, -1.0);
		gmt_M_memset (dim, PSL_MAX_DIMS, double);
		dim[0] = (half_only) ? 0.5 * diameter : diameter;
		dim[1] = 0.0;
		dim[2] = (half_only) ? 180.0 : 360.0;
		dim[7] = 2;	/* Do draw line */
		gmt_setpen (GMT, &GMT->current.setting.map_frame_pen);
		gmt_setfill (GMT, &no_fill, 1);
		PSL_plotsymbol (PSL, 0.0, 0.0, dim, symbol);
	}

	gmt_setpen (GMT, &Ctrl->W.pen[0]);
	if (Ctrl->M.S.v.status & PSL_VEC_OUTLINE2) {	/* Gave specific head outline pen */
		PSL_defpen (GMT->PSL, "PSL_vecheadpen", Ctrl->M.S.v.pen.width, Ctrl->M.S.v.pen.style, Ctrl->M.S.v.pen.offset, Ctrl->M.S.v.pen.rgb);
		dim[11] = Ctrl->M.S.v.pen.width;
	}
	else if (Ctrl->M.active) {
		PSL_defpen (GMT->PSL, "PSL_vecheadpen", 0.5 * Ctrl->W.pen[1].width, Ctrl->W.pen[1].style, Ctrl->W.pen[1].offset, Ctrl->W.pen[1].rgb);
		dim[11] = 0.5 * Ctrl->W.pen[1].width;
	}
	if (windrose) {	/* Here we draw individual vectors */
		if (Ctrl->M.active) { /* Initialize vector head settings */
			gmt_init_vector_param (GMT, &Ctrl->M.S, false, false, NULL, false, NULL);
			if (Ctrl->M.S.symbol == PSL_VECTOR) Ctrl->M.S.v.v_width = (float)(Ctrl->W.pen[1].width * GMT->session.u2u[GMT_PT][GMT_INCH]);
			dim[5] = Ctrl->M.S.v.v_shape;
			dim[6] = (double)Ctrl->M.S.v.status;
			dim[7] = (double)Ctrl->M.S.v.v_kind[0];	dim[8] = (double)Ctrl->M.S.v.v_kind[1];
			if (Ctrl->M.S.v.status & PSL_VEC_OUTLINE) gmt_setpen (GMT, &Ctrl->W.pen[1]);
			if (Ctrl->M.S.v.status & PSL_VEC_FILL2) gmt_setfill (GMT, &Ctrl->M.S.v.fill, 1);       /* Use fill structure */
		}
		for (i = 0; i < n; i++) {
			sincosd (start_angle - azimuth[i], &s, &c);
			radius = length[i] * Ctrl->S.scale;
			if (Ctrl->M.active) {	/* Set end point of vector */
				dim[0] = radius * c, dim[1] = radius * s;
				f = (radius < Ctrl->M.S.v.v_norm) ? radius / Ctrl->M.S.v.v_norm : 1.0;
				dim[2] = f * Ctrl->M.S.v.v_width, dim[3] = f * Ctrl->M.S.v.h_length, dim[4] = f * Ctrl->M.S.v.h_width;
			}
			if (Ctrl->T.active) {
				if (Ctrl->M.active)	{	/* Draw two-headed vectors */
					if (Ctrl->M.S.symbol == GMT_SYMBOL_VECTOR_V4) {
						int v4_outline = Ctrl->W.active[1];
						double *this_rgb = NULL;
						if (Ctrl->M.S.v.status & PSL_VEC_FILL2)
							this_rgb = Ctrl->M.S.v.fill.rgb;
						else
							this_rgb = GMT->session.no_rgb;
						if (v4_outline) gmt_setpen (GMT, &Ctrl->W.pen[1]);
						v4_outline += 8;	/* Double-headed */
						dim[5] = GMT->current.setting.map_vector_shape;
						psl_vector_v4 (PSL, -radius * c, -radius * s, dim, this_rgb, v4_outline);
					}
					else
						PSL_plotsymbol (PSL,  -radius * c, -radius * s, dim, PSL_VECTOR);
				}
				else
					PSL_plotsegment (PSL, -radius * c, -radius * s, radius * c, radius * s);
			}
			else {
				if (Ctrl->M.active) {	/* Draw one-headed vectors */
					if (Ctrl->M.S.symbol == GMT_SYMBOL_VECTOR_V4) {
						int v4_outline = Ctrl->W.active[1];
						double *this_rgb = NULL;
						if (Ctrl->M.S.v.status & PSL_VEC_FILL2)
							this_rgb = Ctrl->M.S.v.fill.rgb;
						else
							this_rgb = GMT->session.no_rgb;
						if (v4_outline) gmt_setpen (GMT, &Ctrl->W.pen[1]);
						dim[5] = GMT->current.setting.map_vector_shape;
						psl_vector_v4 (PSL, 0.0, 0.0, dim, this_rgb, v4_outline);
					}
					else
						PSL_plotsymbol (PSL, 0.0, 0.0, dim, PSL_VECTOR);
				}
				else
					PSL_plotsegment (PSL, 0.0, 0.0, radius * c, radius * s);
			}
		}
	}

	if (sector_plot && !Ctrl->A.rose && (Ctrl->C.active || Ctrl->G.active)) {	/* Draw pie slices for sector plot if fill is requested */

		if (Ctrl->G.active)
			gmt_setfill (GMT, &(Ctrl->G.fill), 0);
		dim[7] = 0;
		if (Ctrl->G.active) dim[7] = 1;
		if (Ctrl->W.active[0]) dim[7] += 2;
		for (bin = 0; bin < n_bins; bin++) {
			if (Ctrl->C.active) {
				index = gmt_get_rgb_from_z (GMT, P, sum[bin] * Ctrl->S.scale, rgb);
				F = gmt_M_get_cptslice_pattern (P, index);
				if (F)	/* Pattern */
					gmt_setfill (GMT, F, 0);
				else
					PSL_setfill (PSL, rgb, 0);
			}
			az = bin * Ctrl->A.inc - az_offset + half_bin_width;
			dim[1] = (start_angle - az - Ctrl->A.inc);
			dim[2] = dim[1] + Ctrl->A.inc;
			dim[0] = sum[bin] * Ctrl->S.scale;
			PSL_plotsymbol (PSL, 0.0, 0.0, dim, PSL_WEDGE);
		}
	}
	else if (Ctrl->A.rose) {	/* Draw rose diagram */

		for (i = bin = 0; bin < n_bins; bin++, i++) {
			az = (bin - 0.5) * Ctrl->A.inc - az_offset + half_bin_width;
			sincosd (start_angle - az - Ctrl->A.inc, &s, &c);
			xx[i] = Ctrl->S.scale * sum[bin] * c;
			yy[i] = Ctrl->S.scale * sum[bin] * s;
		}
		if (half_only) {
			xx[i] = Ctrl->S.scale * 0.5 * (sum[0] + sum[n_bins-1]);
			yy[i++] = 0.0;
			xx[i] = -xx[i-1];
			yy[i++] = 0.0;
		}
		PSL_setfill (PSL, Ctrl->G.fill.rgb, Ctrl->W.active[0]);
		PSL_plotpolygon (PSL, xx, yy, (int)i);
	}

	if (sector_plot && Ctrl->W.active[0] && !Ctrl->A.rose) {	/* Draw a line outlining the pie slices */
		angle1 = ((half_only) ? 180.0 : 90.0) - half_bin_width;
		angle2 = ((half_only) ?   0.0 : 90.0) - half_bin_width;
		sincosd (angle1, &s, &c);
		x1 = (sum[0] * Ctrl->S.scale) * c;
		y1 = (sum[0] * Ctrl->S.scale) * s;
		sincosd (angle2, &s, &c);
		x2 = (sum[n_bins-1] * Ctrl->S.scale) * c;
		y2 = (sum[n_bins-1] * Ctrl->S.scale) * s;
		PSL_plotpoint (PSL, x1, y1, PSL_MOVE);
		PSL_plotpoint (PSL, x2, y2, PSL_DRAW);
		for (bin = n_bins; bin > 0; bin--) {
			k = bin - 1;
			az = k * Ctrl->A.inc - az_offset + half_bin_width;
			angle1 = 90.0 - az - Ctrl->A.inc;
			angle2 = angle1 + Ctrl->A.inc;
			PSL_plotarc (PSL, 0.0, 0.0, sum[k] * Ctrl->S.scale, angle1, angle2, (k == 0) ? PSL_STROKE : PSL_DRAW);
		}
	}

	if (Ctrl->E.active) {
		unsigned int this_mode;
		int v4_outline = Ctrl->W.active[1];
		double *this_rgb = NULL;
		if (Ctrl->E.mode == GMT_OUT) {	/* Write mean vector and statistics to file */
			uint64_t dim[GMT_DIM_SIZE] = {1, 1, 1, 8};	/* One record with 8 columns */
			struct GMT_DATASET *V = NULL;
			struct GMT_DATASEGMENT *S = NULL;
			if ((V = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				Return (API->error);
			}
			S = V->table[0]->segment[0];	/* The only segment */
			sprintf (format, "mean_az\tmean_r\tmean_resultant\tmax\tscaled_mean_r\tlength_sum\tn\tsign@%.2f", Ctrl->Q.value);
			S->data[GMT_X][0] = mean_theta;
			S->data[GMT_Y][0] = mean_radius;
			S->data[GMT_Z][0] = mean_resultant;
			S->data[3][0] = max;
			S->data[4][0] = mean_radius;
			S->data[5][0] = total;
			S->data[6][0] = (double)n;
			S->data[7][0] = significant;
			S->n_rows = 1;
			if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, format, V)) {
				Return (API->error);
			}
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_WRITE_SET, NULL, Ctrl->E.file, V) != GMT_NOERROR) {
				Return (API->error);
			}
		}
		if (!Ctrl->W.active[1]) Ctrl->W.pen[1] = Ctrl->W.pen[0];	/* No separate pen specified; use same as for rose outline */
		if (Ctrl->E.mean) {	/* Not given, calculate and use mean direction only */
			n_modes = 1;
			mode_direction = gmt_M_memory (GMT, NULL, 1, double);
			mode_length = gmt_M_memory (GMT, NULL, 1, double);
			mode_direction[0] = mean_theta;
			mode_length[0] = mean_radius;
		}
		else if (Ctrl->E.mode == GMT_IN) {	/* Get mode parameters from separate file */
			if ((Cin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->E.file, NULL)) == NULL) {
				Return (API->error);
			}
			if (Cin->n_columns < 2) {
				GMT_Report (API, GMT_MSG_ERROR, "Input file %s has %d column(s) but at least 2 are needed\n", Ctrl->E.file, (int)Cin->n_columns);
				Return (GMT_DIM_TOO_SMALL);
			}
			T = Cin->table[0];	/* Can only be one table since we read a single file; We also only use the first segment */
			n_modes = (unsigned int)T->n_records;
			mode_direction = T->segment[0]->data[GMT_X];
			mode_length = T->segment[0]->data[GMT_Y];
		}
		if (!Ctrl->M.active) {	/* Must supply defaults for the vector attributes */
			Ctrl->M.S.size_x = VECTOR_HEAD_LENGTH * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 9p */
			Ctrl->M.S.v.v_width  = (float)(VECTOR_LINE_WIDTH * GMT->session.u2u[GMT_PT][GMT_INCH]);	/* 9p */
			Ctrl->M.S.v.v_angle  = 30.0f;
			Ctrl->M.S.v.status |= (PSL_VEC_OUTLINE + PSL_VEC_OUTLINE2 + PSL_VEC_FILL + PSL_VEC_FILL2 + PSL_VEC_END);
			gmt_init_pen (GMT, &Ctrl->M.S.v.pen, VECTOR_LINE_WIDTH);
			gmt_init_fill (GMT, &Ctrl->M.S.v.fill, 0.0, 0.0, 0.0);		/* Default vector fill = black */
		}
		gmt_init_vector_param (GMT, &Ctrl->M.S, false, false, NULL, false, NULL);
		if (Ctrl->M.S.symbol == PSL_VECTOR) Ctrl->M.S.v.v_width = (float)(Ctrl->W.pen[1].width * GMT->session.u2u[GMT_PT][GMT_INCH]);
		dim[2] = Ctrl->M.S.v.v_width, dim[3] = Ctrl->M.S.v.h_length, dim[4] = Ctrl->M.S.v.h_width;
		dim[5] = Ctrl->M.S.v.v_shape;
		dim[6] = (double)Ctrl->M.S.v.status;
		dim[7] = (double)Ctrl->M.S.v.v_kind[0];	dim[8] = (double)Ctrl->M.S.v.v_kind[1];
		if (Ctrl->M.S.v.status & PSL_VEC_OUTLINE) gmt_setpen (GMT, &Ctrl->W.pen[1]);
		if (Ctrl->M.S.v.status & PSL_VEC_FILL2) gmt_setfill (GMT, &Ctrl->M.S.v.fill, 1);       /* Use fill structure */
		if (Ctrl->M.S.symbol == GMT_SYMBOL_VECTOR_V4) {
			dim[5] = GMT->current.setting.map_vector_shape;
			if (Ctrl->M.S.v.status & PSL_VEC_FILL2)
				this_rgb = Ctrl->M.S.v.fill.rgb;
			else
				this_rgb = GMT->session.no_rgb;
			if (v4_outline) gmt_setpen (GMT, &Ctrl->W.pen[1]);
		}
		else {
			if (Ctrl->M.S.v.status & PSL_VEC_OUTLINE2) {	/* Gave specific head outline pen */
				PSL_defpen (GMT->PSL, "PSL_vecheadpen", Ctrl->M.S.v.pen.width, Ctrl->M.S.v.pen.style, Ctrl->M.S.v.pen.offset, Ctrl->M.S.v.pen.rgb);
				dim[11] = Ctrl->M.S.v.pen.width;
			}
			else if (Ctrl->M.active) {
				PSL_defpen (GMT->PSL, "PSL_vecheadpen", 0.5 * Ctrl->W.pen[1].width, Ctrl->W.pen[1].style, Ctrl->W.pen[1].offset, Ctrl->W.pen[1].rgb);
				dim[11] = 0.5 * Ctrl->W.pen[1].width;
			}
		}
		for (this_mode = 0; this_mode < n_modes; this_mode++) {
			if (Ctrl->N.active) mode_length[this_mode] = sqrt (mode_length[this_mode]);
			if (half_only && mode_direction[this_mode] > 90.0 && mode_direction[this_mode] <= 270.0) mode_direction[this_mode] -= 180.0;
			angle = start_angle - mode_direction[this_mode];
			sincosd (angle, &s, &c);
			xr = Ctrl->S.scale * mode_length[this_mode] * c;
			yr = Ctrl->S.scale * mode_length[this_mode] * s;
			dim[0] = xr, dim[1] = yr;
			if (Ctrl->M.S.symbol == GMT_SYMBOL_VECTOR_V4)
				psl_vector_v4 (PSL, 0.0, 0.0, dim, this_rgb, v4_outline);
			else
				PSL_plotsymbol (PSL, 0.0, 0.0, dim, PSL_VECTOR);
		}

	}
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	if (Ctrl->L.active) {	/* Deactivate those with - */
		if (Ctrl->L.w[0] == '-' && Ctrl->L.w[1] == '\0') Ctrl->L.w[0] = '\0';
		if (Ctrl->L.e[0] == '-' && Ctrl->L.e[1] == '\0') Ctrl->L.e[0] = '\0';
		if (Ctrl->L.s[0] == '-' && Ctrl->L.s[1] == '\0') Ctrl->L.s[0] = '\0';
		if (Ctrl->L.n[0] == '-' && Ctrl->L.n[1] == '\0') Ctrl->L.n[0] = '\0';
	}
	else {	/* Use default labels */
		Ctrl->L.w = strdup (GMT->current.language.cardinal_name[0][0]);
		Ctrl->L.e = strdup (GMT->current.language.cardinal_name[0][1]);
		Ctrl->L.s = strdup (GMT->current.language.cardinal_name[0][2]);
		Ctrl->L.n = strdup (GMT->current.language.cardinal_name[0][3]);
	}
	if (GMT->current.map.frame.draw) {	/* Draw grid lines etc */
		gmt_setpen (GMT, &GMT->current.setting.map_grid_pen[GMT_PRIMARY]);
		off = max_radius * Ctrl->S.scale;
		n_alpha = (GMT->current.map.frame.axis[GMT_Y].item[GMT_GRID_UPPER].interval > 0.0) ? irint (total_arc / GMT->current.map.frame.axis[GMT_Y].item[GMT_GRID_UPPER].interval) : -1;
		for (k = 0; k <= n_alpha; k++) {
			angle = k * GMT->current.map.frame.axis[GMT_Y].item[GMT_GRID_UPPER].interval;
			sincosd (angle, &s, &c);
			x = max_radius * Ctrl->S.scale * c;
			y = max_radius * Ctrl->S.scale * s;
			PSL_plotsegment (PSL, 0.0, 0.0, x, y);
		}

		if (GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval > 0.0) {
			n_bins = urint (max_radius / GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval);
			for (bin = 1; bin <= n_bins; bin++)
				PSL_plotarc (PSL, 0.0, 0.0, bin * GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval * Ctrl->S.scale, 0.0, total_arc, PSL_MOVE|PSL_STROKE);
		}
		PSL_setcolor (PSL, GMT->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);
		y = lsize + 6.0 * GMT->current.setting.map_annot_offset[GMT_PRIMARY];
		form = gmt_setfont (GMT, &GMT->current.setting.font_title);
		PSL_plottext (PSL, 0.0, off + y, GMT->current.setting.font_title.size, GMT->current.map.frame.header, 0.0, PSL_BC, form);

		gmt_get_format (GMT, GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval, GMT->current.map.frame.axis[GMT_X].unit, GMT->current.map.frame.axis[GMT_X].prefix, format);

		if (half_only) {
			char text[GMT_LEN64] = {""};
			if (!Ctrl->L.active) {	/* Use default labels */
				gmt_M_str_free (Ctrl->L.w);
				gmt_M_str_free (Ctrl->L.e);
				gmt_M_str_free (Ctrl->L.n);
				if (GMT->current.setting.map_degree_symbol == gmt_none) {
					if (half_only == 1) {
						sprintf (Ctrl->L.w, "90%s", GMT->current.language.cardinal_name[2][0]);
						sprintf (Ctrl->L.e, "90%s", GMT->current.language.cardinal_name[2][1]);
						sprintf (Ctrl->L.n, "0");
					}
					else {
						sprintf (Ctrl->L.w, "0%s",   GMT->current.language.cardinal_name[2][3]);
						sprintf (Ctrl->L.e, "180%s", GMT->current.language.cardinal_name[2][2]);
						sprintf (Ctrl->L.n, "90%s",  GMT->current.language.cardinal_name[2][1]);
					}
				}
				else {
					if (half_only == 1) {
						sprintf (Ctrl->L.w, "90%c%s", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol], GMT->current.language.cardinal_name[2][0]);
						sprintf (Ctrl->L.e, "90%c%s", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol], GMT->current.language.cardinal_name[2][1]);
						sprintf (Ctrl->L.n, "0%c",    (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);
					}
					else {
						sprintf (Ctrl->L.w, "0%c%s",   (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol], GMT->current.language.cardinal_name[2][3]);
						sprintf (Ctrl->L.e, "180%c%s", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol], GMT->current.language.cardinal_name[2][2]);
						sprintf (Ctrl->L.n, "90%c%s",  (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol], GMT->current.language.cardinal_name[2][1]);
					}
				}
			}
			form = gmt_setfont (GMT, &GMT->current.setting.font_label);
			y = -(3.0 * GMT->current.setting.map_annot_offset[GMT_PRIMARY] + GMT->session.font[GMT->current.setting.font_annot[GMT_PRIMARY].id].height * asize);
			if (GMT->current.map.frame.axis[GMT_X].label[0]) PSL_plottext (PSL, 0.0, y, GMT->current.setting.font_label.size, GMT->current.map.frame.axis[GMT_X].label, 0.0, PSL_TC, form);
			y = -(5.0 * GMT->current.setting.map_annot_offset[GMT_PRIMARY] + GMT->session.font[GMT->current.setting.font_annot[GMT_PRIMARY].id].height * lsize + GMT->session.font[GMT->current.setting.font_label.id].height * lsize);
			if (GMT->current.map.frame.axis[GMT_Y].label[0]) PSL_plottext (PSL, 0.0, y, GMT->current.setting.font_label.size, GMT->current.map.frame.axis[GMT_Y].label, 0.0, PSL_TC, form);
			form = gmt_setfont (GMT, &GMT->current.setting.font_annot[GMT_PRIMARY]);
			PSL_plottext (PSL, 0.0, -GMT->current.setting.map_annot_offset[GMT_PRIMARY], GMT->current.setting.font_annot[GMT_PRIMARY].size, "0", 0.0, PSL_TC, form);
			n_annot = (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval > 0.0) ? irint (max_radius / GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval) : -1;
			for (k = 1; n_annot > 0 && k <= n_annot; k++) {
				x = k * GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval;
				sprintf (text, format, x);
				x *= Ctrl->S.scale;
				PSL_plottext (PSL, x, -GMT->current.setting.map_annot_offset[GMT_PRIMARY], GMT->current.setting.font_annot[GMT_PRIMARY].size, text, 0.0, PSL_TC, form);
				PSL_plottext (PSL, -x, -GMT->current.setting.map_annot_offset[GMT_PRIMARY], GMT->current.setting.font_annot[GMT_PRIMARY].size, text, 0.0, PSL_TC, form);
			}
		}
		else {
			form = gmt_setfont (GMT, &GMT->current.setting.font_label);
			PSL_plottext (PSL, 0.0, -off - 2.0 * GMT->current.setting.map_annot_offset[GMT_PRIMARY], GMT->current.setting.font_label.size, Ctrl->L.s, 0.0, PSL_TC, form);
			if (!Ctrl->F.active && GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval > 0.0) {	/* Draw scale bar but only if x-grid interval is set */
				PSL_plotsegment (PSL, off, -off, (max_radius - GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, -off);
				PSL_plotsegment (PSL, off, -off, off, GMT->current.setting.map_tick_length[0] - off);
				PSL_plotsegment (PSL, (max_radius - GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, -off, (max_radius - GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, GMT->current.setting.map_tick_length[0] - off);
				if (GMT->current.map.frame.axis[GMT_X].label[0]) {
					strcat (format, " %s");
					sprintf (text, format, GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval, GMT->current.map.frame.axis[GMT_X].label);
				}
				else
					sprintf (text, format, GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval);
				form = gmt_setfont (GMT, &GMT->current.setting.font_annot[GMT_PRIMARY]);
				PSL_plottext (PSL, (max_radius - 0.5 * GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, -(off + GMT->current.setting.map_annot_offset[GMT_PRIMARY]), GMT->current.setting.font_annot[GMT_PRIMARY].size, text, 0.0, PSL_TC, form);
			}
			y = -(off + 5.0 * GMT->current.setting.map_annot_offset[GMT_PRIMARY] + GMT->session.font[GMT->current.setting.font_annot[GMT_PRIMARY].id].height * lsize + GMT->session.font[GMT->current.setting.font_label.id].height * lsize);
			form = gmt_setfont (GMT, &GMT->current.setting.font_label);
			if (GMT->current.map.frame.axis[GMT_Y].label[0]) PSL_plottext (PSL, 0.0, y, GMT->current.setting.font_label.size, GMT->current.map.frame.axis[GMT_Y].label, 0.0, PSL_TC, form);
		}
		form = gmt_setfont (GMT, &GMT->current.setting.font_label);
		PSL_plottext (PSL, off + 2.0 * GMT->current.setting.map_annot_offset[GMT_PRIMARY], 0.0, GMT->current.setting.font_label.size, Ctrl->L.e, 0.0, PSL_ML, form);
		PSL_plottext (PSL, -off - 2.0 * GMT->current.setting.map_annot_offset[GMT_PRIMARY], 0.0, GMT->current.setting.font_label.size, Ctrl->L.w, 0.0, PSL_MR, form);
		PSL_plottext (PSL, 0.0, off + 2.0 * GMT->current.setting.map_annot_offset[GMT_PRIMARY], GMT->current.setting.font_label.size, Ctrl->L.n, 0.0, PSL_BC, form);
		PSL_setcolor (PSL, GMT->current.setting.map_default_pen.rgb, PSL_IS_STROKE);
	}

	gmt_plane_perspective (GMT, -1, 0.0);
	PSL_setorigin (PSL, -x_origin, -y_origin, 0.0, PSL_INV);
	gmt_plotend (GMT);

	gmt_M_free (GMT, sum);
	gmt_M_free (GMT, xx);
	gmt_M_free (GMT, yy);
	gmt_M_free (GMT, azimuth);
	gmt_M_free (GMT, length);
	if (Ctrl->E.active) {
		if (Ctrl->E.mean) {
			gmt_M_free (GMT, mode_length);
			gmt_M_free (GMT, mode_direction);
		}
	}

	Return (GMT_NOERROR);
}
