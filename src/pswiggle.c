/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Version:	5 API
 *
 */
 
#define THIS_MODULE_NAME	"pswiggle"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot z = f(x,y) anomalies along tracks"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>BJKOPRUVXYbcdfghipstxy" GMT_OPT("EHMm")

int gmt_parse_g_option (struct GMT_CTRL *GMT, char *txt);

struct PSWIGGLE_CTRL {
	struct A {	/* -A<azimuth> */
		bool active;
		double value;
	} A;
	struct C {	/* -C<center> */
		bool active;
		double value;
	} C;
	struct G {	/* -G[+|-|=]<fill> */
		bool active[2];
		struct GMT_FILL fill[2];
	} G;
	struct I {	/* -I<azimuth> */
		bool active;
		double value;
	} I;
	struct S {	/* -S[x]<lon0>/<lat0>/<length>/<units> */
		bool active;
		bool cartesian;
		double lon, lat, length;
		char *label;
	} S;
	struct T {	/* -T<pen> */
		bool active;
		struct GMT_PEN pen;
	} T;
	struct W {	/* -W<pen> */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z<scale> */
		bool active;
		double scale;
		char unit;
	} Z;
};

void plot_wiggle (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double *x, double *y, double *z, uint64_t n_in, double zscale, double start_az, double stop_az, int fixed, double fix_az, struct GMT_FILL *fill, struct GMT_PEN *pen_o, struct GMT_PEN *pen_t, int paint_wiggle, int negative, int outline, int track)
{
	uint64_t n = 0;
	int64_t i, np = n_in;
	double dx, dy, len, az = 0.0, s = 0.0, c = 0.0, x_inc, y_inc;

	if (fixed) {
		az = fix_az;
		sincos (az, &s, &c);
	}

	if (paint_wiggle || outline) {
		if (!GMT->current.plot.n_alloc) GMT_get_plot_array (GMT);
		GMT->current.plot.x[0] = x[0];
		GMT->current.plot.y[0] = y[0];
		n = 1;
		for (i = 0; i < np; i++) {
			if (!fixed && i < np-1) {
				dx = x[i+1] - x[i];
				dy = y[i+1] - y[i];
				if (!(dx == 0.0 && dy == 0.0)) az = -d_atan2 (dy, dx) - TWO_PI;	/* Azimuth of normal to track */
				while (az < start_az) az += TWO_PI;
				if (az > stop_az) az -= M_PI;
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
			if (n == GMT->current.plot.n_alloc) GMT_get_plot_array (GMT);
		}
		GMT->current.plot.x[n] = x[np-1];
		GMT->current.plot.y[n] = y[np-1];
		n++;

		if (paint_wiggle) {
			for (i = np - 2; i >= 0; i--, n++) {	/* Go back to 1st point along track */
				if (n == GMT->current.plot.n_alloc) GMT_get_plot_array (GMT);
				GMT->current.plot.x[n] = x[i];
				GMT->current.plot.y[n] = y[i];
			}
		}
	}


	if (paint_wiggle) { /* First shade wiggles */
		PSL_comment (PSL, "%s wiggle\n", negative ? "Negative" : "Positive");
		GMT_setfill (GMT, fill, false);
		PSL_plotpolygon (PSL, GMT->current.plot.x, GMT->current.plot.y, (int)n);
	}

	if (outline) { /* Then draw wiggle outline */
		PSL_comment (PSL, "Wiggle line\n");
		GMT_setpen (GMT, pen_o);
		PSL_plotline (PSL, &GMT->current.plot.x[1], &GMT->current.plot.y[1], (int)np, PSL_MOVE + PSL_STROKE);
	}

	if (track) {	/* Finally draw track line */
		PSL_comment (PSL, "Track line\n");
		GMT_setpen (GMT, pen_t);
		PSL_plotline (PSL, x, y, (int)np, PSL_MOVE + PSL_STROKE);
	}
}

void GMT_draw_z_scale (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, double length, double zscale, int gave_xy, char *units)
{	/* Draws a basic vertical scale bar at (x0,y0) and labels it as specified */
	int form;
	double dy, off, xx[4], yy[4];
	char txt[GMT_LEN256] = {""};

	GMT_setpen (GMT, &GMT->current.setting.map_tick_pen[0]);

	if (!gave_xy) {	/* Project lon,lat to get position of scale */
		GMT_geo_to_xy (GMT, x0, y0, &xx[0], &yy[0]);
		x0 = xx[0];	y0 = yy[0];
	}

	if (units) /* Append data unit to the scale length */
		sprintf (txt, "%g %s", length, units);
	else
		sprintf (txt, "%g", length);
	dy = 0.5 * length * zscale;
	/* Compute the 4 coordinates on the scale line */
	GMT_xyz_to_xy (GMT, x0 + GMT->current.setting.map_scale_height, y0 - dy, 0.0, &xx[0], &yy[0]);
	GMT_xyz_to_xy (GMT, x0, y0 - dy, 0.0, &xx[1], &yy[1]);
	GMT_xyz_to_xy (GMT, x0, y0 + dy, 0.0, &xx[2], &yy[2]);
	GMT_xyz_to_xy (GMT, x0 + GMT->current.setting.map_scale_height, y0 + dy, 0.0, &xx[3], &yy[3]);
	PSL_plotline (PSL, xx, yy, 4, PSL_MOVE + PSL_STROKE);
	off = ((GMT->current.setting.map_scale_height > 0.0) ? GMT->current.setting.map_tick_length[0] : 0.0) + GMT->current.setting.map_annot_offset[0];
	form = GMT_setfont (GMT, &GMT->current.setting.font_annot[0]);
	PSL_plottext (PSL, x0 + off, y0, GMT->current.setting.font_annot[0].size, txt, 0.0, 5, form);
}

void *New_pswiggle_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSWIGGLE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct PSWIGGLE_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->T.pen = C->W.pen = GMT->current.setting.map_default_pen;
	GMT_init_fill (GMT, &C->G.fill[0], GMT->current.setting.map_frame_pen.rgb[0], GMT->current.setting.map_frame_pen.rgb[1], GMT->current.setting.map_frame_pen.rgb[2]);
	C->G.fill[1] = C->G.fill[0];

	return (C);
}

void Free_pswiggle_Ctrl (struct GMT_CTRL *GMT, struct PSWIGGLE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->S.label) free (C->S.label);	
	GMT_free (GMT, C);	
}

int GMT_pswiggle_usage (struct GMTAPI_CTRL *API, int level)
{

	/* This displays the pswiggle synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: pswiggle [<table>] %s %s -Z<scale>\n", GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-A<azimuth>] [%s] [-C<center>] [-G[-|+|=]<fill>] [-I<az>] [%s] [-K] [-O]\n", GMT_B_OPT, GMT_Jz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-P] [-S[x]<lon0>/<lat0>/<length>/<units>] [-T<trackpen>] [%s]\n", GMT_U_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-W<outlinepen>] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\t[%s]\n\t[%s] ",
		GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_c_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT);
	GMT_Message (API, GMT_TIME_NONE, "[%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n", GMT_i_OPT, GMT_p_OPT, GMT_s_OPT, GMT_t_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Option (API, "J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Set azimuth for preferred positive wiggle orientation [0.0 (north)].\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Set center value to be removed from z before plotting [0].\n");
	GMT_fill_syntax (API->GMT, 'G', "Specify color/pattern for positive and/or negative areas.");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend + to fill positive areas (default).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend - to fill negative areas.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend = to fill positive and negative areas.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Set fixed projection azimuths for wiggles.\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Fill negative wiggles instead [Default is positive].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Draw a simple vertical scale centered on <lon0>/<lat0>.  Use -Sx to specify cartesian coordinates instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <length> is in z-units, append unit name for labeling.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify track pen attributes. [Default is no track].\n");
	GMT_Option (API, "U,V");
	GMT_pen_syntax (API->GMT, 'W', "Specify outline pen attributes [Default is no outline].");
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Give the wiggle scale in data-units per %s.\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Option (API, "bi3,c,di,f,g,h,i,p,s,t,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_pswiggle_parse (struct GMT_CTRL *GMT, struct PSWIGGLE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pswiggle and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int j, k, wantx, wanty, n_errors = 0;
	bool N_active = false;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, *units = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				Ctrl->A.value = atof (opt->arg);
				break;
			case 'C':
				Ctrl->C.active = true;
				Ctrl->C.value = atof (opt->arg);
				break;
			case 'D':
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -D option is deprecated; use -g instead.\n");
					GMT->common.g.active = true;
					if (opt->arg[0] == 'x')		/* Determine gaps using projected distances */
						sprintf (txt_a, "d%s", &opt->arg[1]);
					else if (GMT_is_geographic (GMT, GMT_IN))	
						sprintf (txt_a, "D%sk", opt->arg);	/* Hardwired to be km */
					else
						sprintf (txt_a, "d%s", opt->arg);	/* Cartesian */
					n_errors += gmt_parse_g_option (GMT, txt_a);
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'G':
				switch (opt->arg[0]) {
					case '=':
					case '+': j = 1, k = 0; break;
					case '-': j = 1, k = 1; break;
					default : j = 0, k = 0; break;
				}
				Ctrl->G.active[k] = true;
				if (GMT_getfill (GMT, &opt->arg[j], &Ctrl->G.fill[k])) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				if (opt->arg[0] == '=') Ctrl->G.fill[1] = Ctrl->G.fill[0];
				break;
			case 'I':
				Ctrl->I.value = atof (opt->arg);
				Ctrl->I.active = true;
				break;
			case 'N':
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -N option is deprecated; use -G-<fill> instead.\n");
					N_active = true;
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'S':
				Ctrl->S.active = true;
				j = 0;
				if (opt->arg[0] == 'x') Ctrl->S.cartesian = true, j = 1;
				k = sscanf (&opt->arg[j], "%[^/]/%[^/]/%lf", txt_a, txt_b, &Ctrl->S.length);
				wantx = (Ctrl->S.cartesian) ? GMT_IS_FLOAT : GMT_IS_LON;
				wanty = (Ctrl->S.cartesian) ? GMT_IS_FLOAT : GMT_IS_LAT;
				n_errors += GMT_verify_expectations (GMT, wantx, GMT_scanf_arg (GMT, txt_a, wantx, &Ctrl->S.lon), txt_a);
				n_errors += GMT_verify_expectations (GMT, wanty, GMT_scanf_arg (GMT, txt_b, wanty, &Ctrl->S.lat), txt_b);
				if ((units = strrchr (opt->arg, '/'))) {
					units++;
					Ctrl->S.label = strdup (units);
				}
				n_errors += GMT_check_condition (GMT, k != 3, "Syntax error -S option: Correct syntax:\n\t-S[x]<x0>/<y0>/<length>[/<units>]\n");
				break;
			case 'T':
				Ctrl->T.active = true;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->T.pen)) {
					GMT_pen_syntax (GMT, 'T', " ");
					n_errors++;
				}
				break;
			case 'W':
				Ctrl->W.active = true;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', " ");
					n_errors++;
				}
				break;
			case 'Z':
				Ctrl->Z.active = true;
				j = (unsigned int)strlen (opt->arg) - 1;
				if (strchr (GMT_DIM_UNITS, (int)opt->arg[j])) Ctrl->Z.unit = opt->arg[j];
				Ctrl->Z.scale = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (N_active && GMT_compat_check (GMT, 4)) {
		Ctrl->G.active[1] = Ctrl->G.active[0];
		Ctrl->G.active[0] = false;
		Ctrl->G.fill[1] = Ctrl->G.fill[0];
	}

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->W.active || Ctrl->G.active[0] || Ctrl->G.active[1]), "Syntax error: Must specify at least one of -G, -W\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Z.scale == 0.0, "Syntax error -Z option: scale must be nonzero\n");
	n_errors += GMT_check_binary_io (GMT, 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_pswiggle_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

void alloc_space (struct GMT_CTRL *GMT, size_t *n_alloc, double **xx, double **yy, double **zz)
{
	(*n_alloc) <<= 1;
	*xx = GMT_memory (GMT, *xx, *n_alloc, double);
	*yy = GMT_memory (GMT, *yy, *n_alloc, double);
	*zz = GMT_memory (GMT, *zz, *n_alloc, double);
}

int GMT_pswiggle (void *V_API, int mode, void *args)
{
	bool negative;
	int error = 0;
	
	unsigned int tbl;
	
	uint64_t row, seg, j;
	size_t n_alloc = GMT_CHUNK;

	double x_2, y_2, start_az, stop_az, fix_az, dz;
	double *xx = NULL, *yy = NULL, *zz = NULL, *lon = NULL, *lat = NULL, *z = NULL;

	struct GMT_DATASET *D = NULL;
	struct GMT_DATATABLE *T = NULL;
	struct PSWIGGLE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_pswiggle_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_pswiggle_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_pswiggle_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_pswiggle_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pswiggle_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the pswiggle main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	PSL = GMT_plotinit (GMT, options);

	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	GMT_plotcanvas (GMT);	/* Fill canvas if requested */

	Ctrl->Z.scale = 1.0 / Ctrl->Z.scale;

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

	GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);

	/* Allocate memory */

	xx  = GMT_memory (GMT, NULL, n_alloc, double);
	yy  = GMT_memory (GMT, NULL, n_alloc, double);
	zz  = GMT_memory (GMT, NULL, n_alloc, double);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Register data input */
		Return (API->error);
	}
	if ((error = GMT_set_cols (GMT, GMT_IN, 3)) != GMT_OK) {
		Return (error);
	}
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_FILE_BREAK, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}

	for (tbl = 0; tbl < D->n_tables; tbl++) {
		T = D->table[tbl];
		
		GMT_Report (API, GMT_MSG_VERBOSE, "Working on file %s\n", T->file[GMT_IN]);
		PSL_comment (PSL, "File %s\n", T->file[GMT_IN]);

		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */

			PSL_comment (PSL, "%s\n", T->segment[seg]->header);

			lon = T->segment[seg]->coord[GMT_X];	/* lon, lat, z are just shorthands */
			lat = T->segment[seg]->coord[GMT_Y];
			z = T->segment[seg]->coord[GMT_Z];
			
			GMT_geo_to_xy (GMT, lon[0], lat[0], &xx[0], &yy[0]);
			zz[0] = z[0] - Ctrl->C.value;
			for (row = j = 1; row < T->segment[seg]->n_rows; row++) {	/* Convert to inches/cm and get distance increments */
				GMT_geo_to_xy (GMT, lon[row], lat[row], &x_2, &y_2);

				if (j > 0 && GMT_is_dnan (z[row])) {	/* Data gap, plot what we have */
					negative = zz[j-1] < 0.0;
					plot_wiggle (GMT, PSL, xx, yy, zz, j, Ctrl->Z.scale, start_az, stop_az, Ctrl->I.active, fix_az, &Ctrl->G.fill[negative], &Ctrl->W.pen, &Ctrl->T.pen, Ctrl->G.active[negative], negative, Ctrl->W.active, Ctrl->T.active);
					j = 0;
				}
				else if (!GMT_is_dnan (z[row-1]) && (z[row]*z[row-1] < 0.0 || z[row] == 0.0)) {	/* Crossed 0, add new point and plot */
					dz = z[row] - z[row-1];
					xx[j] = (dz == 0.0) ? xx[j-1] : xx[j-1] + fabs (z[row-1] / dz) * (x_2 - xx[j-1]);
					yy[j] = (dz == 0.0) ? yy[j-1] : yy[j-1] + fabs (z[row-1] / dz) * (y_2 - yy[j-1]);
					zz[j++] = 0.0;
					if (j == n_alloc) alloc_space (GMT, &n_alloc, &xx, &yy, &zz);
					negative = zz[j-2] < 0.0;
					plot_wiggle (GMT, PSL, xx, yy, zz, j, Ctrl->Z.scale, start_az, stop_az, Ctrl->I.active, fix_az, &Ctrl->G.fill[negative], &Ctrl->W.pen, &Ctrl->T.pen, Ctrl->G.active[negative], negative, Ctrl->W.active, Ctrl->T.active);
					xx[0] = xx[j-1];
					yy[0] = yy[j-1];
					zz[0] = zz[j-1];
					j = 1;
				}
				xx[j] = x_2;
				yy[j] = y_2;
				zz[j] = z[row] - Ctrl->C.value;
				if (!GMT_is_dnan (z[row])) j++;
				if (j == n_alloc) alloc_space (GMT, &n_alloc, &xx, &yy, &zz);
			}
	
			if (j > 1) {
				negative = zz[j-1] < 0.0;
				plot_wiggle (GMT, PSL, xx, yy, zz, j, Ctrl->Z.scale, start_az, stop_az, Ctrl->I.active, fix_az, &Ctrl->G.fill[negative], &Ctrl->W.pen, &Ctrl->T.pen, Ctrl->G.active[negative], negative, Ctrl->W.active, Ctrl->T.active);
			}
		}
	}
	
	GMT_map_clip_off (GMT);
	GMT_map_basemap (GMT);

	if (Ctrl->S.active) GMT_draw_z_scale (GMT, PSL, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.length, Ctrl->Z.scale, Ctrl->S.cartesian, Ctrl->S.label);

	GMT_plane_perspective (GMT, -1, 0.0);
	GMT_plotend (GMT);

	GMT_free (GMT, xx);
	GMT_free (GMT, yy);
	GMT_free (GMT, zz);

	Return (GMT_OK);
}
