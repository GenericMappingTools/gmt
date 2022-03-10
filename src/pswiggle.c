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
 * Brief synopsis: pswiggle reads x,y,z from GMT->session.std[GMT_IN] and plots a wiggleplot using the
 * specified map projection. If the distance between 2 consecutive points
 * exceeds the value of dist_gap, a data gap is assumed.  The user may
 * select a preferred direction where s/he wants the positive anomalies
 * to be pointing towards.  Positive normal vectors to the trackline
 * will then always be within +- 90 degrees of this direction.
 * Separate colors may be specified for positive anomaly, outline of
 * anomaly, and trackline.  Plotting of the outline and track is optional.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"pswiggle"
#define THIS_MODULE_MODERN_NAME	"wiggle"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot z = f(x,y) anomalies along tracks"
#define THIS_MODULE_KEYS	"<D{,>X}"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYbdefghipqtxyw" GMT_OPT("EHMmc")

#define PSWIGGLE_POS	0
#define PSWIGGLE_NEG	1

struct PSWIGGLE_CTRL {
	struct PSWIGGLE_A {	/* -A[<azimuth>] */
		bool active;
		unsigned int mode;
		double value;
	} A;
	struct PSWIGGLE_C {	/* -C<center> */
		bool active;
		double value;
	} C;
	struct PSWIGGLE_D {	/* -D[g|j|J|n|x]<refpoint>+w<length>[+a][+j<justify>][+o<dx>[/<dy>]][+l<label>] */
		bool active;
		char *arg;
		struct GMT_MAP_SCALE scale;
	} D;
	struct PSWIGGLE_F {	/* -F[+c<clearance>][+g<fill>][+i[<off>/][<pen>]][+p[<pen>]][+r[<radius>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;	/* Panel inside GMT_MAP_SCALE in -D */
	} F;
	struct PSWIGGLE_G {	/* -G<fill>[+n][+p] */
		bool active[2];
		struct GMT_FILL fill[2];
	} G;
	struct PSWIGGLE_I {	/* -I<azimuth> */
		bool active;
		double value;
	} I;
	struct PSWIGGLE_S {	/* -S[x]<lon0>/<lat0>/<length>/<units> [Backwards compatibility with GMT 5] */
		bool active;
		bool cartesian;
		double lon, lat, length;
		char *label;
	} S;
	struct PSWIGGLE_T {	/* -T<pen> */
		bool active;
		struct GMT_PEN pen;
	} T;
	struct PSWIGGLE_W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct PSWIGGLE_Z {	/* -Z<scale> */
		bool active;
		double scale;
		char unit;
	} Z;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSWIGGLE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSWIGGLE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->A.mode = 1;	/* Default is -A0 */
	C->T.pen = C->W.pen = GMT->current.setting.map_default_pen;
	gmt_init_fill (GMT, &C->G.fill[0], GMT->current.setting.map_frame_pen.rgb[0], GMT->current.setting.map_frame_pen.rgb[1], GMT->current.setting.map_frame_pen.rgb[2]);
	C->G.fill[1] = C->G.fill[0];

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct PSWIGGLE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->D.arg);
	if (C->D.scale.refpoint) gmt_free_refpoint (GMT, &C->D.scale.refpoint);
	gmt_M_free (GMT, C->D.scale.panel);
	gmt_M_str_free (C->S.label);
	gmt_M_free (GMT, C);
}

GMT_LOCAL void pswiggle_plot_section (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double *x, double *y, double *z, uint64_t n_in, double zscale, unsigned int adjust_az, double start_az, double stop_az, int fixed, double fix_az, struct GMT_FILL *fill, struct GMT_PEN *pen_o, struct GMT_PEN *pen_t, int paint_wiggle, int negative, int outline, int track) {
	uint64_t n = 0;
	int64_t i, np = n_in;
	double dx, dy, len, az = 0.0, s = 0.0, c = 0.0, x_inc, y_inc;

	if (fixed) {
		az = fix_az;
		sincos (az, &s, &c);
	}

	if (paint_wiggle || outline) {
		if (!GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
		GMT->current.plot.x[0] = x[0];
		GMT->current.plot.y[0] = y[0];
		n = 1;
		for (i = 0; i < np; i++) {
			if (!fixed && i < np-1) {
				dx = x[i+1] - x[i];
				dy = y[i+1] - y[i];
				if (!(dx == 0.0 && dy == 0.0)) az = -d_atan2 (dy, dx) - TWO_PI;	/* Azimuth of normal to track */
				if (adjust_az) {	/* Enforce orientation in the -90/+90 window around -A */
					while (az < start_az) az += TWO_PI;
					if (az > stop_az) az -= M_PI;
				}
			}
			if (fabs (z[i]) > 0.0) {
				if (!fixed) sincos (az, &s, &c);
				len = zscale * z[i];
				x_inc = len * s;
				y_inc = len * c;
			}
			else
				x_inc = y_inc = 0.0;

			GMT->current.plot.x[n] = x[i] + x_inc;
			GMT->current.plot.y[n] = y[i] + y_inc;
			n++;
			if (n == GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
		}
		GMT->current.plot.x[n] = x[np-1];
		GMT->current.plot.y[n] = y[np-1];
		n++;

		if (paint_wiggle) {
			for (i = np - 2; i >= 0; i--, n++) {	/* Go back to 1st point along track */
				if (n == GMT->current.plot.n_alloc) gmt_get_plot_array (GMT);
				GMT->current.plot.x[n] = x[i];
				GMT->current.plot.y[n] = y[i];
			}
		}
	}


	if (paint_wiggle) { /* First shade wiggles */
		PSL_comment (PSL, "%s wiggle\n", negative ? "Negative" : "Positive");
		gmt_setfill (GMT, fill, 0);
		PSL_plotpolygon (PSL, GMT->current.plot.x, GMT->current.plot.y, (int)n);
	}

	if (outline) { /* Then draw wiggle outline */
		PSL_comment (PSL, "Wiggle line\n");
		gmt_setpen (GMT, pen_o);
		PSL_plotline (PSL, &GMT->current.plot.x[1], &GMT->current.plot.y[1], (int)np, PSL_MOVE|PSL_STROKE);
	}

	if (track) {	/* Finally draw track line */
		PSL_comment (PSL, "Track line\n");
		gmt_setpen (GMT, pen_t);
		PSL_plotline (PSL, x, y, (int)np, PSL_MOVE|PSL_STROKE);
	}
}

GMT_LOCAL void pswiggle_draw_z_scale (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, double length, double zscale, int gave_xy, char *units) {
	/* Draws a basic vertical scale bar at (x0,y0) and labels it as specified */
	int form;
	double dy, off, xx[4], yy[4];
	char txt[GMT_LEN256] = {""};

	gmt_setpen (GMT, &GMT->current.setting.map_tick_pen[GMT_PRIMARY]);

	if (!gave_xy) {	/* Project lon,lat to get position of scale */
		gmt_geo_to_xy (GMT, x0, y0, &xx[0], &yy[0]);
		x0 = xx[0];	y0 = yy[0];
	}

	if (units) /* Append data unit to the scale length */
		sprintf (txt, "%g %s", length, units);
	else
		sprintf (txt, "%g", length);
	dy = 0.5 * length * zscale;
	/* Compute the 4 coordinates on the scale line */
	gmt_xyz_to_xy (GMT, x0 + GMT->current.setting.map_scale_height, y0 - dy, 0.0, &xx[0], &yy[0]);
	gmt_xyz_to_xy (GMT, x0, y0 - dy, 0.0, &xx[1], &yy[1]);
	gmt_xyz_to_xy (GMT, x0, y0 + dy, 0.0, &xx[2], &yy[2]);
	gmt_xyz_to_xy (GMT, x0 + GMT->current.setting.map_scale_height, y0 + dy, 0.0, &xx[3], &yy[3]);
	PSL_plotline (PSL, xx, yy, 4, PSL_MOVE|PSL_STROKE);
	off = ((GMT->current.setting.map_scale_height > 0.0) ? GMT->current.setting.map_tick_length[GMT_PRIMARY] : 0.0) + GMT->current.setting.map_annot_offset[GMT_PRIMARY];
	form = gmt_setfont (GMT, &GMT->current.setting.font_annot[GMT_PRIMARY]);
	PSL_plottext (PSL, x0 + off, y0, GMT->current.setting.font_annot[GMT_PRIMARY].size, txt, 0.0, PSL_ML, form);
}

static int usage (struct GMTAPI_CTRL *API, int level) {

	/* This displays the pswiggle synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] %s %s -Z<scale>[<unit>] [-A[<azimuth>]] [%s] [-C<center>] [-D%s+w<length>[+a]%s%s[+l<label>]] "
		"[-F%s] [-G<fill>[+n][+p]] [-I<az>] [%s] %s%s%s[-T<pen>] [%s] [%s] [-W<pen>] [%s] [%s] [%s] %s[%s] [%s] [%s] "
		"[%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_J_OPT, GMT_Rgeoz_OPT, GMT_B_OPT, GMT_XYANCHOR, GMT_JUSTIFY, GMT_OFFSET, GMT_PANEL, GMT_Jz_OPT,
		API->K_OPT, API->O_OPT, API->P_OPT, GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_bi_OPT, API->c_OPT, GMT_di_OPT,
		GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_t_OPT, GMT_w_OPT,
		GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<,J-,R");
	GMT_Usage (API, 1, "\n-Z<scale>[<unit>]");
	GMT_Usage (API, -2, "Give the wiggle <scale> in data-units per %s. "
		"Alternatively, append any unit from among %s [c].", API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit], GMT_DIM_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A[<azimuth>]");
	GMT_Usage (API, -2, "Set <azimuth> for preferred positive wiggle orientation [0.0 (north)]. "
		"Perpendiculars to the track are mapped into a -90/+90 window centered on <azimuth>. "
		"If no azimuth is given then we use the azimuths as they are computed.");
	GMT_Option (API, "B-");
	GMT_Usage (API, 1, "\n-C<center>");
	GMT_Usage (API, -2, "Set center value to be removed from z before plotting [0].");
	gmt_refpoint_syntax (API->GMT, "\n-D", "Specify position and dimensions of the vertical scale bar.", GMT_ANCHOR_VSCALE, 3);
	GMT_Usage (API, 3, "+w Append <length> to set the scale length in data z-units.");
	GMT_Usage (API, 3, "+a Move label to the opposite side of vertical scale bar.");
	GMT_Usage (API, 3, "+l Set the unit label of the z-values for the scale bar label [no label].");
	gmt_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the vertical scale.", 4);
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify color/pattern for positive and/or negative areas.");
	GMT_Usage (API, -2, "You can control which areas are to be painted:");
	GMT_Usage (API, 3, "+p Fill positive areas only (Default).");
	GMT_Usage (API, 3, "+n Fill negative areas only.");
	GMT_Usage (API, -2, "Append both modifiers to fill positive and negative areas.");
	GMT_Usage (API, 1, "\n-I<az>");
	GMT_Usage (API, -2, "Set fixed projection azimuths for wiggles. "
		"Azimuths of the perpendiculars to the track are reset to <az>.");
	GMT_Option (API, "Z,K");
	GMT_Option (API, "O,P");
	GMT_Usage (API, 1, "\n-T<pen>");
	GMT_Usage (API, -2, "Specify track pen attributes [Default draws no track].");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Specify outline pen attributes [Default is no outline].", NULL, 0);
	GMT_Option (API, "X");
	GMT_Option (API, "bi3,c,di,e,f,g,h,i,p,qi,t,w,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct PSWIGGLE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pswiggle and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int j, k, n_slash, wantx, wanty, n_errors = 0, pp = 0;
	bool N_active = false, neg = false, pos = false;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, *units = NULL, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			/* Processes program-specific parameters */

			case 'A':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				if (opt->arg[0] == '\0')	/* Do not enforce a 180-degree preferred window */
					Ctrl->A.mode = 0;
				else
					Ctrl->A.value = atof (opt->arg);
				break;
			case 'C':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				Ctrl->C.value = atof (opt->arg);
				break;
			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				if ((!strchr (opt->arg, '+') || opt->arg[0] == 'x') && gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "-D option is deprecated; use -g instead.\n");
					GMT->common.g.active = true;
					if (opt->arg[0] == 'x')		/* Determine gaps using projected distances */
						sprintf (txt_a, "d%s", &opt->arg[1]);
					else if (gmt_M_is_geographic (GMT, GMT_IN))
						sprintf (txt_a, "D%sk", opt->arg);	/* Hardwired to be km */
					else
						sprintf (txt_a, "d%s", opt->arg);	/* Cartesian */
					n_errors += gmt_parse_g_option (GMT, txt_a);
				}
				else {
					if (opt->arg[0])
						Ctrl->D.arg = strdup (opt->arg);
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -D: No argument given!\n");
						n_errors++;					
					}
					Ctrl->D.active = Ctrl->D.scale.vertical = true;
				}
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				if (gmt_getpanel (GMT, opt->option, opt->arg, &(Ctrl->D.scale.panel))) {
					gmt_mappanel_syntax (GMT, 'F', "Specify a rectangular panel behind the scale", 3);
					n_errors++;
				}
				break;
			case 'G':	/* -G<fill>[+n][+p] */
				j = 0;	neg = pos = false;
				if ((c = strstr (opt->arg, "+n+p")) || (c = strstr (opt->arg, "+p+n")) || (c = strstr (opt->arg, "+n")) || (c = strstr (opt->arg, "+p"))) {	/* Gave +n and/or +p */
					pp = 0;	txt_a[0] = 0;
					while (gmt_getmodopt (GMT, 'G', c, "np", &pp, txt_a, &n_errors) && n_errors == 0) {
						switch (txt_a[0]) {
							case 'n': neg = true;	break;	/* Negative fill */
							case 'p': pos = true;	break;	/* Positive fill */
							default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';	/* Chop off all modifiers */
				}
				else if (strchr ("-+=", opt->arg[0])) {	/* Allow old syntax -G+|-|=<fill> */
					switch (opt->arg[0]) {
						case '=': j = 1, pos = neg = true; break;
						case '+': j = 1, pos = true; 	   break;
						case '-': j = 1, neg = true; 	   break;
						default : j = 0, pos = true; 	   break;
					}
				}
				if (!(pos || neg)) pos = true;	/* Default is positive fill */
				k = (pos) ? PSWIGGLE_POS : PSWIGGLE_NEG;
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active[k]);
				Ctrl->G.active[k] = true;
				if (gmt_getfill (GMT, &opt->arg[j], &Ctrl->G.fill[k])) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				if (c) c[0] = '+';	/* Restore modifiers */
				if (pos && neg) Ctrl->G.active[PSWIGGLE_NEG] = true, Ctrl->G.fill[PSWIGGLE_NEG] = Ctrl->G.fill[PSWIGGLE_POS];	/* Duplicate fill */
				break;
			case 'I':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.value = atof (opt->arg);
				Ctrl->I.active = true;
				break;
			case 'N':
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "-N option is deprecated; use -G<fill>+n instead.\n");
					N_active = true;
				}
				else
					n_errors += gmt_default_option_error (GMT, opt);
				break;
			case 'S':
				if (gmt_M_compat_check (GMT, 5)) {
					GMT_Report (API, GMT_MSG_COMPAT, "-S option is deprecated; use -D instead.\n");
					Ctrl->S.active = true;
					j = 0;
					if (opt->arg[0] == 'x') Ctrl->S.cartesian = true, j = 1;
					k = sscanf (&opt->arg[j], "%[^/]/%[^/]/%lf", txt_a, txt_b, &Ctrl->S.length);
					for (j = n_slash = 0; opt->arg[j]; j++) if (opt->arg[j] == '/') n_slash++;
					wantx = (Ctrl->S.cartesian) ? GMT_IS_FLOAT : GMT_IS_LON;
					wanty = (Ctrl->S.cartesian) ? GMT_IS_FLOAT : GMT_IS_LAT;
					n_errors += gmt_verify_expectations (GMT, wantx, gmt_scanf_arg (GMT, txt_a, wantx, false, &Ctrl->S.lon), txt_a);
					n_errors += gmt_verify_expectations (GMT, wanty, gmt_scanf_arg (GMT, txt_b, wanty, false, &Ctrl->S.lat), txt_b);
					if (n_slash == 3 && (units = strrchr (opt->arg, '/')) != NULL) {
						units++;
						Ctrl->S.label = strdup (units);
					}
					n_errors += gmt_M_check_condition (GMT, k != 3, "Option -S: Correct syntax:\n\t-S[x]<x0>/<y0>/<length>[/<units>]\n");
				}
				else
					n_errors += gmt_default_option_error (GMT, opt);
				break;
			case 'T':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				if (gmt_getpen (GMT, opt->arg, &Ctrl->T.pen)) {
					gmt_pen_syntax (GMT, 'T', NULL, " ", NULL, 0);
					n_errors++;
				}
				break;
			case 'W':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				if (gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					gmt_pen_syntax (GMT, 'W', NULL, " ", NULL, 0);
					n_errors++;
				}
				break;
			case 'Z':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
				j = (unsigned int)strlen (opt->arg) - 1;
				if (strchr (GMT_DIM_UNITS, (int)opt->arg[j])) Ctrl->Z.unit = opt->arg[j];
				Ctrl->Z.scale = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	if (N_active && gmt_M_compat_check (GMT, 4)) {
		Ctrl->G.active[PSWIGGLE_NEG] = Ctrl->G.active[PSWIGGLE_POS];
		Ctrl->G.active[PSWIGGLE_POS] = false;
		Ctrl->G.fill[PSWIGGLE_NEG] = Ctrl->G.fill[PSWIGGLE_POS];
	}

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, !(Ctrl->W.active || Ctrl->G.active[0] || Ctrl->G.active[1]), "Must specify at least one of -G, -W\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.scale == 0.0, "Option -Z: scale must be nonzero\n");
	n_errors += gmt_check_binary_io (GMT, 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LOCAL void pswiggle_alloc_space (struct GMT_CTRL *GMT, size_t *n_alloc, double **xx, double **yy, double **zz) {
	(*n_alloc) <<= 1;
	*xx = gmt_M_memory (GMT, *xx, *n_alloc, double);
	*yy = gmt_M_memory (GMT, *yy, *n_alloc, double);
	*zz = gmt_M_memory (GMT, *zz, *n_alloc, double);
}

EXTERN_MSC int GMT_pswiggle (void *V_API, int mode, void *args) {
	bool negative;
	int error = 0;

	unsigned int tbl;

	uint64_t row, seg, j;
	size_t n_alloc = GMT_CHUNK;

	double x_2, y_2, start_az, stop_az, fix_az, dz;
	double *xx = NULL, *yy = NULL, *zz = NULL, *lon = NULL, *lat = NULL, *z = NULL;

	struct GMT_DATASET *D = NULL;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATATABLE_HIDDEN *TH = NULL;

	struct PSWIGGLE_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the pswiggle main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	if (gmt_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_PROJECTION_ERROR);

	if (Ctrl->D.active && gmt_getscale (GMT, 'D', Ctrl->D.arg, &Ctrl->D.scale)) Return (GMT_PARSE_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);

	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_set_basemap_orders (GMT, GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_BEFORE, GMT_BASEMAP_ANNOT_AFTER);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	gmt_map_basemap (GMT);

	Ctrl->Z.scale = 1.0 / Ctrl->Z.scale;	/* Since we were requesting the reciprocal */

	switch (Ctrl->Z.unit) {	/* Adjust for possible unit selection */
		case 'c':
			Ctrl->Z.scale *= GMT->session.u2u[GMT_CM][GMT_INCH];
			break;
		case 'i':
			Ctrl->Z.scale *= GMT->session.u2u[GMT_INCH][GMT_INCH];
			break;
		case 'p':
			Ctrl->Z.scale *= GMT->session.u2u[GMT_PT][GMT_INCH];
			break;
		default:
			Ctrl->Z.scale *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
			break;
	}

	/* Now convert angles to radians and keep it that way */
	start_az = (Ctrl->A.value - 90.0) * D2R;
	stop_az  = (Ctrl->A.value + 90.0) * D2R;
	fix_az = Ctrl->I.value * D2R;

	gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		Return (API->error);
	}
	if ((error = GMT_Set_Columns (API, GMT_IN, 3, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_FILEBREAK, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (D->n_columns < 3) {
		GMT_Report (API, GMT_MSG_ERROR, "Input data have %d column(s) but at least 3 are needed\n", (int)D->n_columns);
		Return (GMT_DIM_TOO_SMALL);
	}

	/* Allocate memory */

	xx  = gmt_M_memory (GMT, NULL, n_alloc, double);
	yy  = gmt_M_memory (GMT, NULL, n_alloc, double);
	zz  = gmt_M_memory (GMT, NULL, n_alloc, double);

	for (tbl = 0; tbl < D->n_tables; tbl++) {
		T = D->table[tbl];
		TH = gmt_get_DT_hidden (T);

		GMT_Report (API, GMT_MSG_INFORMATION, "Working on file %s\n", TH->file[GMT_IN]);
		PSL_comment (PSL, "File %s\n", TH->file[GMT_IN]);

		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */

			/* Cannot exclude segments whose x,y are all outside since we do not yet know the coordinates of the amplitude curve */

			PSL_comment (PSL, "%s\n", T->segment[seg]->header);

			lon = T->segment[seg]->data[GMT_X];	/* lon, lat, z are just shorthands */
			lat = T->segment[seg]->data[GMT_Y];
			z = T->segment[seg]->data[GMT_Z];

			if (Ctrl->C.active) for (row = 0; row < T->segment[seg]->n_rows; row++) z[row] -= Ctrl->C.value;	/* Remove center value */

			gmt_geo_to_xy (GMT, lon[0], lat[0], &xx[0], &yy[0]);
			zz[0] = z[0];
			for (row = j = 1; row < T->segment[seg]->n_rows; row++) {	/* Convert to inches/cm and get distance increments */
				gmt_geo_to_xy (GMT, lon[row], lat[row], &x_2, &y_2);

				if (j > 0 && gmt_M_is_dnan (z[row])) {	/* Data gap, plot what we have */
					negative = zz[j-1] < 0.0;
					pswiggle_plot_section (GMT, PSL, xx, yy, zz, j, Ctrl->Z.scale, Ctrl->A.mode, start_az, stop_az, Ctrl->I.active, fix_az, &Ctrl->G.fill[negative], &Ctrl->W.pen, &Ctrl->T.pen, Ctrl->G.active[negative], negative, Ctrl->W.active, Ctrl->T.active);
					j = 0;
				}
				else if (!gmt_M_is_dnan (z[row-1]) && (z[row]*z[row-1] < 0.0 || z[row] == 0.0)) {	/* Crossed 0, add new point and plot */
					dz = z[row] - z[row-1];
					xx[j] = (dz == 0.0) ? xx[j-1] : xx[j-1] + fabs (z[row-1] / dz) * (x_2 - xx[j-1]);
					yy[j] = (dz == 0.0) ? yy[j-1] : yy[j-1] + fabs (z[row-1] / dz) * (y_2 - yy[j-1]);
					zz[j++] = 0.0;
					if (j == n_alloc) pswiggle_alloc_space (GMT, &n_alloc, &xx, &yy, &zz);
					negative = zz[j-2] < 0.0;
					pswiggle_plot_section (GMT, PSL, xx, yy, zz, j, Ctrl->Z.scale, Ctrl->A.mode, start_az, stop_az, Ctrl->I.active, fix_az, &Ctrl->G.fill[negative], &Ctrl->W.pen, &Ctrl->T.pen, Ctrl->G.active[negative], negative, Ctrl->W.active, Ctrl->T.active);
					xx[0] = xx[j-1];
					yy[0] = yy[j-1];
					zz[0] = zz[j-1];
					j = 1;
				}
				xx[j] = x_2;
				yy[j] = y_2;
				zz[j] = z[row];
				if (!gmt_M_is_dnan (z[row])) j++;
				if (j == n_alloc) pswiggle_alloc_space (GMT, &n_alloc, &xx, &yy, &zz);
			}

			if (j > 1) {
				negative = zz[j-1] < 0.0;
				pswiggle_plot_section (GMT, PSL, xx, yy, zz, j, Ctrl->Z.scale, Ctrl->A.mode, start_az, stop_az, Ctrl->I.active, fix_az, &Ctrl->G.fill[negative], &Ctrl->W.pen, &Ctrl->T.pen, Ctrl->G.active[negative], negative, Ctrl->W.active, Ctrl->T.active);
			}
		}
	}

	gmt_map_clip_off (GMT);
	gmt_map_basemap (GMT);

	if (Ctrl->D.active) {	/* New way of setting the vertical scale */
		Ctrl->D.scale.zdata = true;
		Ctrl->D.scale.z_scale = Ctrl->Z.scale;
		gmt_draw_vertical_scale (GMT, &Ctrl->D.scale);
	}
	else if (Ctrl->S.active)
		pswiggle_draw_z_scale (GMT, PSL, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.length, Ctrl->Z.scale, Ctrl->S.cartesian, Ctrl->S.label);

	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	gmt_M_free (GMT, xx);
	gmt_M_free (GMT, yy);
	gmt_M_free (GMT, zz);

	Return (GMT_NOERROR);
}

EXTERN_MSC int GMT_wiggle (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: wiggle\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_pswiggle (V_API, mode, args);
}
