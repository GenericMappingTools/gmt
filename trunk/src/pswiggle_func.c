/*--------------------------------------------------------------------
 *	$Id: pswiggle_func.c,v 1.4 2011-04-12 13:06:43 remko Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
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
 
#include "pslib.h"
#include "gmt.h"

struct PSWIGGLE_CTRL {
	struct A {	/* -A<azimuth> */
		GMT_LONG active;
		double value;
	} A;
	struct C {	/* -C<center> */
		GMT_LONG active;
		double value;
	} C;
	struct G {	/* -G<fill> */
		GMT_LONG active;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I<azimuth> */
		GMT_LONG active;
		double value;
	} I;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct S {	/* -S[x]<lon0>/<lat0>/<length>/<units> */
		GMT_LONG active;
		GMT_LONG cartesian;
		double lon, lat, length;
		char *label;
	} S;
	struct T {	/* -T<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} T;
	struct W {	/* -W<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z<scale> */
		GMT_LONG active;
		double scale;
		char unit;
	} Z;
};

void plot_wiggle (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double *x, double *y, double *z, GMT_LONG np, double zscale, double start_az, double stop_az, GMT_LONG fixed, double fix_az, struct GMT_FILL *fill, struct GMT_PEN *pen_o, struct GMT_PEN *pen_t, GMT_LONG paint_wiggle, GMT_LONG negative, GMT_LONG outline, GMT_LONG track)
{
	GMT_LONG n = 0, i;
	double dx, dy, len, az = 0.0, s = 0.0, c = 0.0, x_inc, y_inc;
	static char *name[2] = {"Positive", "Negative"};

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
		PSL_comment (PSL, "%s wiggle\n", name[negative]);
		GMT_setfill (GMT, PSL, fill, FALSE);
		PSL_plotpolygon (PSL, GMT->current.plot.x, GMT->current.plot.y, n);
	}

	if (outline) { /* Then draw wiggle outline */
		PSL_comment (PSL, "Wiggle line\n");
		GMT_setpen (GMT, PSL, pen_o);
		PSL_plotline (PSL, &GMT->current.plot.x[1], &GMT->current.plot.y[1], np, PSL_MOVE + PSL_STROKE);
	}

	if (track) {	/* Finally draw track line */
		PSL_comment (PSL, "Track line\n");
		GMT_setpen (GMT, PSL, pen_t);
		PSL_plotline (PSL, x, y, np, PSL_MOVE + PSL_STROKE);
	}
}

void GMT_draw_z_scale (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double y0, double length, double zscale, GMT_LONG gave_xy, char *units)
{	/* Draws a basic vertical scale bar at (x0,y0) and labels it as specified */
	GMT_LONG form;
	double dy, off, xx[4], yy[4];
	char txt[GMT_LONG_TEXT];

	GMT_setpen (GMT, PSL, &GMT->current.setting.map_tick_pen);

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
	PSL_plotline (PSL, xx, yy, (GMT_LONG)4, PSL_MOVE + PSL_STROKE);
	off = ((GMT->current.setting.map_scale_height > 0.0) ? GMT->current.setting.map_tick_length : 0.0) + GMT->current.setting.map_annot_offset[0];
	form = GMT_setfont (GMT, PSL, &GMT->current.setting.font_annot[0]);
	PSL_plottext (PSL, x0 + off, y0, GMT->current.setting.font_annot[0].size, txt, 0.0, 5, form);
}

void *New_pswiggle_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSWIGGLE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct PSWIGGLE_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->T.pen = C->W.pen = GMT->current.setting.map_default_pen;
	GMT_init_fill (GMT, &C->G.fill, GMT->current.setting.map_frame_pen.rgb[0], GMT->current.setting.map_frame_pen.rgb[1], GMT->current.setting.map_frame_pen.rgb[2]);
		
	return ((void *)C);
}

void Free_pswiggle_Ctrl (struct GMT_CTRL *GMT, struct PSWIGGLE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->S.label) free ((void *)C->S.label);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_pswiggle_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the pswiggle synopsis and optionally full usage information */

	GMT_message (GMT, "pswiggle %s [API] - Plot xyz-series along tracks\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: pswiggle <xyz-files> %s %s -Z<scale>\n", GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_message (GMT, "\t[-A<azimuth>] [%s] [-C<center>] [-G<fill>]\n", GMT_B_OPT);
	GMT_message (GMT, "\t[-I<az>] [%s] [-K] [-N] [-O] [-P] [-S[x]<lon0>/<lat0>/<length>/<units>] [-T<trackpen>]\n", GMT_Jz_OPT);
	GMT_message (GMT, "\t[%s] [%s] [-W<outlinepen>] [%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n",
		GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_bi_OPT, GMT_c_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s]\n\n", GMT_i_OPT, GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<xyz_files> is one or more files.  If none, read standard input\n");
	GMT_explain_options (GMT, "jR");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Set azimuth for preferred positive wiggle orientation [0.0 (north)]\n");
	GMT_explain_options (GMT, "b");
	GMT_message (GMT, "\t-C Sets center value to be removed from z before plotting. [0]\n");
	GMT_fill_syntax (GMT, 'G', "Specify color/pattern for positive areas.");
	GMT_message (GMT, "\t-I Set fixed projection azimuths for wiggles\n");
	GMT_explain_options (GMT, "ZK");
	GMT_message (GMT, "\t-N Fill negative wiggles instead [Default is positive]\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-S Draws a simple vertical scale centered on <lon0>/<lat0>.  Use -Sx to specify cartesian coordinates instead.\n");
	GMT_message (GMT, "\t   <length> is in z-units, append unit name for labeling\n");
	GMT_message (GMT, "\t-T Specifies track pen attributes. [Default is no track]\n");
	GMT_explain_options (GMT, "UV");
	GMT_pen_syntax (GMT, 'W', "Specifies outline pen attributes [Default is no outline].");
	GMT_explain_options (GMT, "X");
	GMT_message (GMT, "\t-Z Gives the wiggle scale in data-units per %s\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	GMT_explain_options (GMT, "C3cfghipt:.");
	
	return (EXIT_FAILURE);
}

#ifdef GMT_COMPAT
EXTERN_MSC GMT_LONG gmt_parse_g_option (struct GMT_CTRL *C, char *txt);
#endif

GMT_LONG GMT_pswiggle_parse (struct GMTAPI_CTRL *C, struct PSWIGGLE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pswiggle and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j, k, wantx, wanty, n_errors = 0;
	char txt_a[GMT_LONG_TEXT], txt_b[GMT_LONG_TEXT], *units = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = TRUE;
				Ctrl->A.value = atof (opt->arg);
				break;
			case 'C':
				Ctrl->C.active = TRUE;
				Ctrl->C.value = atof (opt->arg);
				break;
#ifdef GMT_COMPAT
			case 'D':
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -D option is deprecated; use -g instead.\n");
				GMT->common.g.active = TRUE;
				if (opt->arg[0] == 'x')		/* Determine gaps using projected distances */
					sprintf (txt_a, "d%s", &opt->arg[1]);
				else if (GMT_is_geographic (GMT, GMT_IN))	
					sprintf (txt_a, "D%sk", opt->arg);	/* Hardwired to be km */
				else
					sprintf (txt_a, "d%s", opt->arg);	/* Cartesian */
				n_errors += gmt_parse_g_option (GMT, txt_a);
				break;
#endif
			case 'G':
				Ctrl->G.active = TRUE;
				if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'I':
				Ctrl->I.value = atof (opt->arg);
				Ctrl->I.active = TRUE;
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				j = 0;
				if (opt->arg[0] == 'x') Ctrl->S.cartesian = TRUE, j = 1;
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
				Ctrl->T.active = TRUE;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->T.pen)) {
					GMT_pen_syntax (GMT, 'T', " ");
					n_errors++;
				}
				break;
			case 'W':
				Ctrl->W.active = TRUE;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', " ");
					n_errors++;
				}
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				j = strlen (opt->arg) - 1;
				if (strchr (GMT_DIM_UNITS, (int)opt->arg[j])) Ctrl->Z.unit = opt->arg[j];
				Ctrl->Z.scale = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->W.active || Ctrl->G.active), "Syntax error: Must specify at least one of -G, -W\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Z.scale == 0.0, "Syntax error -Z option: scale must be nonzero\n");
	n_errors += GMT_check_binary_io (GMT, 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_pswiggle_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

void alloc_space (struct GMT_CTRL *GMT, GMT_LONG *n_alloc, double **xx, double **yy, double **zz)
{
	(*n_alloc) <<= 1;
	*xx = GMT_memory (GMT, *xx, *n_alloc, double);
	*yy = GMT_memory (GMT, *yy, *n_alloc, double);
	*zz = GMT_memory (GMT, *zz, *n_alloc, double);
}

GMT_LONG GMT_pswiggle (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, paint_wiggle, i, j, tbl, seg, n_alloc = GMT_CHUNK;

	double x_2, y_2, start_az, stop_az, fix_az, dz;
	double *xx = NULL, *yy = NULL, *zz = NULL, *lon = NULL, *lat = NULL, *z = NULL;

	struct GMT_DATASET *D = NULL;
	struct GMT_TABLE *T = NULL;
	struct PSWIGGLE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_pswiggle_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_pswiggle_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_pswiggle", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJRbf:", "BKOPUXxYycghipst>" GMT_OPT("EHMm"), options))) Return (error);
	Ctrl = (struct PSWIGGLE_CTRL *)New_pswiggle_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pswiggle_parse (API, Ctrl, options))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the pswiggle main code ----------------------------*/

	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	GMT_plotinit (API, PSL, options);

	GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

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

	GMT_map_clip_on (GMT, PSL, GMT->session.no_rgb, 3);

	/* Allocate memory */

	xx  = GMT_memory (GMT, NULL, n_alloc, double);
	yy  = GMT_memory (GMT, NULL, n_alloc, double);
	zz  = GMT_memory (GMT, NULL, n_alloc, double);

	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Register data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if ((error = GMT_set_cols (GMT, GMT_IN, 3))) Return (error);
	if ((error = GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, GMT_FILE_BREAK, NULL, (void **)&D))) Return (error);
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	for (tbl = 0; tbl < D->n_tables; tbl++) {
		T = D->table[tbl];
		
		GMT_report (GMT, GMT_MSG_NORMAL, "Working on file %s\n", T->file[GMT_IN]);
		PSL_comment (PSL, "File %s\n", T->file[GMT_IN]);

		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */

			PSL_comment (PSL, "%s\n", T->segment[seg]->header);

			lon = T->segment[seg]->coord[GMT_X];
			lat = T->segment[seg]->coord[GMT_Y];
			z = T->segment[seg]->coord[GMT_Z];
			
			GMT_geo_to_xy (GMT, lon[0], lat[0], &xx[0], &yy[0]);
			zz[0] = z[0] - Ctrl->C.value;
			paint_wiggle = (Ctrl->G.active && ((Ctrl->N.active && z[0] <= 0.0) || (!Ctrl->N.active && z[0] >= 0.0)));
			for (i = j = 1; i < T->segment[seg]->n_rows; i++) {	/* Convert to inches/cm and get distance increments */
				GMT_geo_to_xy (GMT, lon[i], lat[i], &x_2, &y_2);

				if (j > 0 && GMT_is_dnan (z[i])) {	/* Data gap, plot what we have */
					paint_wiggle = (Ctrl->G.active && ((Ctrl->N.active && zz[j-1] <= 0.0) || (!Ctrl->N.active && zz[j-1] >= 0.0)));
					plot_wiggle (GMT, PSL, xx, yy, zz, j, Ctrl->Z.scale, start_az, stop_az, Ctrl->I.active, fix_az, &Ctrl->G.fill, &Ctrl->W.pen, &Ctrl->T.pen, paint_wiggle, Ctrl->N.active, Ctrl->W.active, Ctrl->T.active);
					j = 0;
				}
				else if (!GMT_is_dnan (z[i-1]) && (z[i]*z[i-1] < 0.0 || z[i] == 0.0)) {	/* Crossed 0, add new point and plot */
					dz = z[i] - z[i-1];
					xx[j] = (dz == 0.0) ? xx[j-1] : xx[j-1] + fabs (z[i-1] / dz) * (x_2 - xx[j-1]);
					yy[j] = (dz == 0.0) ? yy[j-1] : yy[j-1] + fabs (z[i-1] / dz) * (y_2 - yy[j-1]);
					zz[j++] = 0.0;
					if (j == n_alloc) alloc_space (GMT, &n_alloc, &xx, &yy, &zz);
					paint_wiggle = (Ctrl->G.active && ((Ctrl->N.active && zz[j-2] <= 0.0) || (!Ctrl->N.active && zz[j-2] >= 0.0)));
					plot_wiggle (GMT, PSL, xx, yy, zz, j, Ctrl->Z.scale, start_az, stop_az, Ctrl->I.active, fix_az, &Ctrl->G.fill, &Ctrl->W.pen, &Ctrl->T.pen, paint_wiggle, Ctrl->N.active, Ctrl->W.active, Ctrl->T.active);
					xx[0] = xx[j-1];
					yy[0] = yy[j-1];
					zz[0] = zz[j-1];
					j = 1;
				}
				xx[j] = x_2;
				yy[j] = y_2;
				zz[j] = z[i] - Ctrl->C.value;
				if (!GMT_is_dnan (z[i])) j++;
				if (j == n_alloc) alloc_space (GMT, &n_alloc, &xx, &yy, &zz);
			}
	
			if (j > 1) {
				paint_wiggle = (Ctrl->G.active && ((Ctrl->N.active && zz[j-1] <= 0.0) || (!Ctrl->N.active && zz[j-1] >= 0.0)));
				plot_wiggle (GMT, PSL, xx, yy, zz, j, Ctrl->Z.scale, start_az, stop_az, Ctrl->I.active, fix_az, &Ctrl->G.fill, &Ctrl->W.pen, &Ctrl->T.pen, paint_wiggle, Ctrl->N.active, Ctrl->W.active, Ctrl->T.active);
			}
		}
	}
	
	GMT_map_clip_off (GMT, PSL);
	GMT_map_basemap (GMT, PSL);

	if (Ctrl->S.active) GMT_draw_z_scale (GMT, PSL, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.length, Ctrl->Z.scale, Ctrl->S.cartesian, Ctrl->S.label);

	GMT_plane_perspective (GMT, PSL, -1, 0.0);
	GMT_plotend (GMT, PSL);

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&D);
	GMT_free (GMT, xx);
	GMT_free (GMT, yy);
	GMT_free (GMT, zz);

	Return (GMT_OK);
}
