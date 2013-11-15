/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: psrose reads a file [or standard input] with azimuth and length information
 * and draws a sector or rose diagram.  Several options for plot layout are available.
 * 2 diagrams are possible: Full circle (360) or half circle (180).  In the
 * latter case azimuths > 180 are reversed (-= 180).
 *
 * To be compatible with GMT, I assume radial distance to be "x"
 * and azimuth to be "y".  Hence, west = 0.0 and east = max_radius
 * south/north is -90,90 for halfcircle and 0,360 for full circle
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */
 
#define THIS_MODULE_NAME	"psrose"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot a polar histogram (rose, sector, windrose diagrams)"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>BKOPRUVXYbchipstxy" GMT_OPT("E")

struct PSROSE_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -A<sector_angle>[r] */
		bool active;
		bool rose;
		double inc;
	} A;
	struct C {	/* -C[<modefile>] */
		bool active;
		char *file;
	} C;
	struct D {	/* -D */
		bool active;
	} D;
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
	struct S {	/* -Sscale[n] */
		bool active;
		bool normalize;
		double scale;
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

void *New_psrose_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSROSE_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct PSROSE_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	GMT_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);
	C->W.pen[0] = C->W.pen[1] = GMT->current.setting.map_default_pen;
	C->S.scale = 3.0;
	C->Z.scale = 1.0;
	if (GMT->current.setting.proj_length_unit == GMT_CM) {
		C->S.scale = 7.5 / 2.54;
	}
	return (C);
}

void Free_psrose_Ctrl (struct GMT_CTRL *GMT, struct PSROSE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->C.file) free (C->C.file);	
	if (C->L.w) free (C->L.w);	
	if (C->L.e) free (C->L.e);	
	if (C->L.s) free (C->L.s);	
	if (C->L.n) free (C->L.n);	
	GMT_free (GMT, C);	
}

int GMT_psrose_usage (struct GMTAPI_CTRL *API, int level)
{
	double r;
	char *choice[2] = {"OFF", "ON"};

	/* This displays the psrose synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: psrose [<table>] [-A[r]<sector_angle>] [%s] [-C[<modes>]] [-D] [-G<fill>] [-I] [-K]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-L[<wlab>/<elab>/<slab>/<nlab>]] [-M[<size>][<modifiers>]] [-N] [-O] [-P]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-R<r0>/<r1>/<theta0>/<theta1>] [-S[n]<scale>] [-T] [%s]\n", GMT_U_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-W[v]<pen>] [%s] [%s]\n\t[-Zu|<scale>] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n",
		GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_bi_OPT, GMT_c_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_s_OPT, GMT_t_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Sector width in degrees for sector diagram [Default is windrose];\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Ar to get rose diagram.\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t   (Remember: radial is x-direction, azimuthal is y-direction).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Plot vectors listed in the <modes> file.  If no file, use mean direction.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Will center the sectors.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Do not draw the scale length bar [Default plots scale in lower right corner].\n");
	GMT_fill_syntax (API->GMT, 'G', "Specify color for diagram [Default is no fill].");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Inquire mode; only compute statistics - no plot is created.\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Override default labels [Default is WEST/EAST/SOUTH/NORTH for full circle and 90W/90E/-/0 for half-circle].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no argument is given then labels will be disabled.  Give - to disable an individual label.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Specify arrow attributes (requires -C).\n");
	GMT_vector_syntax (API->GMT, 15);
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is %gp+gblack+p1p.\n", VECTOR_HEAD_LENGTH);
	GMT_Message (API, GMT_TIME_NONE, "\t-N Normalize rose plots for area, i.e., take sqrt(r) before plotting [false].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Only applicable if normalization has been specified with -S<radius>n.\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-R Specifies the region (<r0> = 0, <r1> = max_radius).  For azimuth:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify <theta0>/<theta1> = -90/90 or 0/180 (half-circles) or 0/360 only).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <r0> = <r1> = 0, psrose will compute a reasonable <r1> value.\n");
	r = (API->GMT->current.setting.proj_length_unit == GMT_CM) ? 7.5 : 3.0;
	GMT_Message (API, GMT_TIME_NONE, "\t-S Specify the radius of the unit circle in %s [%g]. Normalize r if -Sn is used.\n", API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit], r);
	GMT_Message (API, GMT_TIME_NONE, "\t-T Indicate that the vectors are oriented (two-headed), not directed [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Ignored if -R sets a half-circle domain.\n");
	GMT_Option (API, "U,V");
	GMT_pen_syntax (API->GMT, 'W', "Set pen attributes for outline of rose [Default is no outline].");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Wv<pen> to set a different pen for the vector (requires -C) [Same as rose outline].\n");
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Multiply the radii by <scale> before plotting or use -Zu to give each item unit weight.\n");
	GMT_Option (API, "c");
	GMT_Message (API, GMT_TIME_NONE, "\t-: Expect (azimuth,radius) input rather than (radius,azimuth) [%s].\n", choice[API->GMT->current.setting.io_lonlat_toggle[GMT_IN]]);
	GMT_Option (API, "bi2,h,i,p,s,t,.");
	
	return (EXIT_FAILURE);
}

int GMT_psrose_parse (struct GMT_CTRL *GMT, struct PSROSE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to psrose and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int n;
	unsigned int n_errors = 0, k;
	double range;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, txt_d[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Get Sector angle in degrees */
				Ctrl->A.active = true;
				if (strchr (opt->arg, 'r')) Ctrl->A.rose = true;
				k = (opt->arg[0] == 'r') ? 1 : 0;
				Ctrl->A.inc = atof (&opt->arg[k]);
				break;
			case 'C':	/* Read mode file and plot mean directions */
				Ctrl->C.active = true;
				if (Ctrl->C.file) free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Center the bins */
				Ctrl->D.active = true;
				break;
			case 'F':	/* Disable scalebar plotting */
				Ctrl->F.active = true;
				break;
			case 'G':	/* Set Gray shade */
				Ctrl->G.active = true;
				if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'I':	/* Compute statistics only - no plot */
				Ctrl->I.active = true;
				break;
			case 'L':	/* Overwride default labeling */
				Ctrl->L.active = true;
				if (opt->arg[0]) {
					n_errors += GMT_check_condition (GMT, sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d) != 4, "Syntax error -L option: Expected\n\t-L<westlabel/eastlabel/southlabel/<northlabel>>\n");
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
				if (GMT_compat_check (GMT, 4) && (strchr (opt->arg, '/') && !strchr (opt->arg, '+'))) {	/* Old-style args */
					n = sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
					if (n != 4 || GMT_getrgb (GMT, txt_d, Ctrl->M.S.v.fill.rgb)) {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -M option: Expected\n\t-M<tailwidth/headlength/headwidth/<color>>\n");
						n_errors++;
					}
					else {	/* Turn the old args into new +a<angle> and pen width */
						Ctrl->W.active[1] = true;
						Ctrl->W.pen[1].width = GMT_to_points (GMT, txt_a);
						Ctrl->M.S.v.h_length = (float)GMT_to_inch (GMT, txt_b);
						Ctrl->M.S.v.h_width = (float)GMT_to_inch (GMT, txt_c);
						Ctrl->M.S.v.v_angle = (float)atand (0.5 * Ctrl->M.S.v.h_width / Ctrl->M.S.v.h_length);
						Ctrl->M.S.v.status |= (GMT_VEC_OUTLINE + GMT_VEC_FILL);
					}
				}
				else {
					if (opt->arg[0] == '+') {	/* No size (use default), just attributes */
						n_errors += GMT_parse_vector (GMT, 'v', opt->arg, &Ctrl->M.S);
					}
					else {	/* Size, plus possible attributes */
						n = sscanf (opt->arg, "%[^+]%s", txt_a, txt_b);	/* txt_a should be symbols size with any +<modifiers> in txt_b */
						if (n == 1) txt_b[0] = 0;	/* No modifiers present, set txt_b to empty */
						Ctrl->M.S.size_x = GMT_to_inch (GMT, txt_a);	/* Length of vector */
						n_errors += GMT_parse_vector (GMT, 'v', txt_b, &Ctrl->M.S);
					}
					Ctrl->M.S.v.status |= GMT_VEC_OUTLINE;
				}
				break;
			case 'N':	/* Make sectors area be proportional to frequency instead of radius */
				Ctrl->N.active = true;
				break;
			case 'S':	/* Get radius of unit circle in inches */
				Ctrl->S.active = true;
				if (strchr (opt->arg, 'n')) Ctrl->S.normalize = true;
				n = (int)strlen (opt->arg) - 1;
				if (opt->arg[n] == 'n') opt->arg[n] = 0;	/* Temporarily remove the n */
				k = (opt->arg[0] == 'n') ? 1 : 0;		/* If we got -Sn<radius> */
				Ctrl->S.scale = GMT_to_inch (GMT, &opt->arg[k]);
				if (Ctrl->S.normalize && k == 0) opt->arg[n] = 'n';	/* Put back the n */
				break;
			case 'T':	/* Oriented instead of directed data */
				Ctrl->T.active = true;
				break;
			case 'W':	/* Get pen width for outline */
				n = (opt->arg[0] == 'v') ? 1 : 0;
				Ctrl->W.active[n] = true;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->W.pen[n])) {
					GMT_pen_syntax (GMT, 'W', " ");
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
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	GMT->common.R.wesn[XLO] = 0.0;
	range = GMT->common.R.wesn[YHI] - GMT->common.R.wesn[YLO];
	if (doubleAlmostEqual (range, 180.0) && Ctrl->T.active) {
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: -T only needed for 0-360 range data (ignored)");
		Ctrl->T.active = false;
	}
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && Ctrl->C.file && GMT_access (GMT, Ctrl->C.file, R_OK), "Syntax error -C: Cannot read file %s!\n", Ctrl->C.file);
	n_errors += GMT_check_condition (GMT, Ctrl->S.scale <= 0.0, "Syntax error -S option: radius must be nonzero\n");
	n_errors += GMT_check_condition (GMT, GMT_IS_ZERO (Ctrl->Z.scale), "Syntax error -Z option: factor must be nonzero\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.inc < 0.0, "Syntax error -A option: sector width must be positive\n");
	if (!Ctrl->I.active) {
		n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
		n_errors += GMT_check_condition (GMT, !((GMT->common.R.wesn[YLO] == -90.0 && GMT->common.R.wesn[YHI] == 90.0) \
			|| (GMT->common.R.wesn[YLO] == 0.0 && GMT->common.R.wesn[YHI] == 180.0)
			|| (GMT->common.R.wesn[YLO] == 0.0 && GMT->common.R.wesn[YHI] == 360.0)),
				"Syntax error -R option: theta0/theta1 must be either -90/90, 0/180 or 0/360\n");
	}
	n_errors += GMT_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_psrose_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_psrose (void *V_API, int mode, void *args)
{
	bool find_mean = false, do_fill = false;
	bool automatic = false, sector_plot = false, windrose = true;
	unsigned int n_bins, n_modes, form, n_in, half_only = 0, bin;
	int error = 0, k, n_annot, n_alpha, sbin;
	
	uint64_t n = 0, i;
	
	size_t n_alloc = GMT_CHUNK;

	char text[GMT_BUFSIZ] = {""}, format[GMT_BUFSIZ] = {""};

	double max = 0.0, radius, az, x_origin, y_origin, tmp, one_or_two = 1.0, s, c;
	double angle1, angle2, angle, x, y, mean_theta, mean_radius, xr = 0.0, yr = 0.0;
	double x1, x2, y1, y2, total = 0.0, total_arc, off, max_radius, az_offset, start_angle;
	double asize, lsize, this_az, half_bin_width, diameter, wesn[4], mean_vector, mean_resultant;
	double *xx = NULL, *yy = NULL, *in = NULL, *sum = NULL, *azimuth = NULL;
	double *length = NULL, *mode_direction = NULL, *mode_length = NULL, dim[7];

	struct PSROSE_CTRL *Ctrl = NULL;
	struct GMT_DATASET *Cin = NULL;
	struct GMT_DATATABLE *P = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_psrose_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_psrose_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_psrose_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_psrose_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psrose_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the psrose main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	asize = GMT->current.setting.font_annot[0].size * GMT->session.u2u[GMT_PT][GMT_INCH];
	lsize = GMT->current.setting.font_annot[0].size * GMT->session.u2u[GMT_PT][GMT_INCH];

	max_radius = GMT->common.R.wesn[XHI];
	if (doubleAlmostEqual (GMT->common.R.wesn[YLO], -90.0))
		half_only = 1;
	else if (doubleAlmostEqual (GMT->common.R.wesn[YHI], 180.0))
		half_only = 2;
	if (Ctrl->A.rose) windrose = false;
	sector_plot = (Ctrl->A.inc > 0.0);
	if (sector_plot) windrose = false;	/* Draw rose diagram instead of sector diagram */
	if (!Ctrl->S.normalize) Ctrl->N.active = false;	/* Only do this is data is normalized for length also */
	if (!Ctrl->I.active && !GMT->common.R.active) automatic = true;
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

	sum = GMT_memory (GMT, NULL, n_bins, double);
	xx = GMT_memory (GMT, NULL, n_bins+2, double);
	yy = GMT_memory (GMT, NULL, n_bins+2, double);
	azimuth = GMT_memory (GMT, NULL, n_alloc, double);
	length = GMT_memory (GMT, NULL, n_alloc, double);

	/* Read data and do some stats */

	n = 0;
	n_in = (GMT->common.i.active && GMT->common.i.n_cols == 1) ? 1 : 2;
	
	if ((error = GMT_set_cols (GMT, GMT_IN, n_in)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */

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
			azimuth = GMT_memory (GMT, azimuth, n_alloc, double);
			length = GMT_memory (GMT, length, n_alloc, double);
		}
	} while (true);

	if (Ctrl->A.inc > 0.0) {	/* Sum up sector diagram info */
		for (i = 0; i < n; i++) {
			if (Ctrl->D.active) {	/* Center bin by removing half bin width here */
				this_az = azimuth[i] - half_bin_width;
				if (!half_only && this_az < 0.0)   this_az += 360.0;
				if (half_only == 1 && this_az < -90.0) this_az += 180.0;
				if (half_only == 2 && this_az < 0.0) this_az += 180.0;
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
			if (Ctrl->T.active) {	/* Also count its other end */
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
	if (!Ctrl->S.normalize) mean_radius *= max_radius;

	if (Ctrl->A.inc > 0.0) {	/* Find max of the bins */
		for (bin = 0; bin < n_bins; bin++) if (sum[bin] > max) max = sum[bin];
		if (Ctrl->S.normalize) for (bin = 0; bin < n_bins; bin++) sum[bin] /= max;
		if (Ctrl->N.active) for (bin = 0; bin < n_bins; bin++) sum[bin] = sqrt (sum[bin]);
	}
	else {
		for (i = 0; i < n; i++) if (length[i] > max) max = length[i];
		if (Ctrl->S.normalize) {
			max = 1.0 / max;
			for (i = 0; i < n; i++) length[i] *= max;
			max = 1.0 / max;
		}
	}

	if (Ctrl->I.active || GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		char *kind[2] = {"r", "bin sum"};
		sprintf (format, "Info for data: n = %% " PRIu64 " mean az = %s mean r = %s mean resultant length = %s max %s = %s scaled mean r = %s linear length sum = %s\n",
			GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, kind[Ctrl->A.active],
			GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_NORMAL, format, n, mean_theta, mean_vector, mean_resultant, max, mean_radius, total);
		if (Ctrl->I.active) {
			GMT_free (GMT, sum);
			GMT_free (GMT, xx);
			GMT_free (GMT, yy);
			GMT_free (GMT, azimuth);
			GMT_free (GMT, length);
			Return (EXIT_SUCCESS);
		}
	}

	if (automatic) {
		if (GMT_IS_ZERO (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval)) {
			tmp = pow (10.0, floor (d_log10 (GMT, max)));
			if ((max / tmp) < 3.0) tmp *= 0.5;
		}
		else
			tmp = GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval;
		max_radius = ceil (max / tmp) * tmp;
		if (GMT_IS_ZERO (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval) || GMT_IS_ZERO (GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval)) {	/* Tickmarks not set */
			GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval = GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval = tmp;
			GMT->current.map.frame.draw = true;
		}
	}

	if (GMT->current.map.frame.draw && GMT_IS_ZERO (GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval)) GMT->current.map.frame.axis[GMT_Y].item[GMT_ANNOT_UPPER].interval = GMT->current.map.frame.axis[GMT_Y].item[GMT_GRID_UPPER].interval = 30.0;

	/* Ready to plot.  So set up GMT projections (not used by psrose), we set region to actual plot width and scale to 1 */

	GMT_parse_common_options (GMT, "J", 'J', "x1i");
	GMT->common.R.active = GMT->common.J.active = true;
	wesn[XLO] = wesn[YLO] = -Ctrl->S.scale;	wesn[XHI] = wesn[YHI] = Ctrl->S.scale;
	GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");

	if (GMT->current.map.frame.paint) {	/* Until psrose uses a polar projection we must bypass the basemap fill and do it ourself here */
		GMT->current.map.frame.paint = false;	/* Turn off so GMT_plotinit wont fill */
		do_fill = true;
	}
	PSL = GMT_plotinit (GMT, options);

	x_origin = Ctrl->S.scale;	y_origin = ((half_only) ? 0.0 : Ctrl->S.scale);
	diameter = 2.0 * Ctrl->S.scale;
	PSL_setorigin (PSL, x_origin, y_origin, 0.0, PSL_FWD);
	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	if (!Ctrl->S.normalize) Ctrl->S.scale /= max_radius;

	if (do_fill) {	/* Until psrose uses a polar projection we must bypass the basemap fill and do it ourself here */
		double dim = 2.0 * Ctrl->S.scale;
		GMT->current.map.frame.paint = true;	/* Restore original setting */
		if (half_only) {	/* Clip the circle */
			double xc[4], yc[4];
			xc[0] = xc[3] = -Ctrl->S.scale;	xc[1] = xc[2] = Ctrl->S.scale;
			yc[0] = yc[1] = 0.0;	yc[2] = yc[3] = Ctrl->S.scale;
			PSL_beginclipping (PSL, xc, yc, 4, GMT->session.no_rgb, 3);
		}
		GMT_setfill (GMT, &GMT->current.map.frame.fill, false);
		PSL_plotsymbol (PSL, 0.0, 0.0, &dim, GMT_SYMBOL_CIRCLE);
		if (half_only) PSL_endclipping (PSL, 1);		/* Reduce polygon clipping by one level */
	}
	if (GMT->common.B.active) {	/* Draw frame */
		int symbol = (half_only) ? GMT_SYMBOL_WEDGE : GMT_SYMBOL_CIRCLE;
		double dim[3];
		struct GMT_FILL no_fill;
		
		GMT_init_fill (GMT, &no_fill, -1.0, -1.0, -1.0);
		dim[0] = (half_only) ? 0.5 * diameter : diameter;
		dim[1] = 0.0;
		dim[2] = (half_only) ? 180.0 : 360.0;
		GMT_setpen (GMT, &GMT->current.setting.map_frame_pen);
		GMT_setfill (GMT, &no_fill, true);
		PSL_plotsymbol (PSL, 0.0, 0.0, dim, symbol);
	}

	GMT_setpen (GMT, &Ctrl->W.pen[0]);
	if (windrose) {
		for (i = 0; i < n; i++) {
			sincosd (start_angle - azimuth[i], &s, &c);
			radius = length[i] * Ctrl->S.scale;
			if (Ctrl->T.active)
				PSL_plotsegment (PSL, -radius * c, -radius * s, radius * c, radius * s);
			else
				PSL_plotsegment (PSL, 0.0, 0.0, radius * c, radius * s);
			
		}
	}

	if (sector_plot && !Ctrl->A.rose && Ctrl->G.fill.rgb[0] >= 0) {	/* Draw pie slices for sector plot if fill is requested */

		GMT_setfill (GMT, &(Ctrl->G.fill), false);
		for (bin = 0; bin < n_bins; bin++) {
			az = bin * Ctrl->A.inc - az_offset + half_bin_width;
			dim[1] = (start_angle - az - Ctrl->A.inc);
			dim[2] = dim[1] + Ctrl->A.inc;
			dim[0] = sum[bin] * Ctrl->S.scale;
			PSL_plotsymbol (PSL, 0.0, 0.0, dim, PSL_WEDGE);
		}
	}
	else if (Ctrl->A.rose) {	/* Draw rose diagram */

		for (i = bin = 0; bin < n_bins; bin++, i++) {
			az = (bin + 0.5) * Ctrl->A.inc - az_offset - half_bin_width;
			sincosd (start_angle - az, &s, &c);
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

	if (Ctrl->C.active) {
		unsigned int this_mode;
		if (!Ctrl->W.active[1]) Ctrl->W.pen[1] = Ctrl->W.pen[0];	/* No separate pen specified; use same as for rose outline */
		if (!Ctrl->C.file) {	/* Not given, calculate and use mean direction only */
			find_mean = true;
			n_modes = 1;
			mode_direction = GMT_memory (GMT, NULL, 1, double);
			mode_length = GMT_memory (GMT, NULL, 1, double);
			mode_direction[0] = mean_theta;
			mode_length[0] = mean_radius;
		}
		else {	/* Get mode parameters from separate file */
			if ((Cin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
				Return (API->error);
			}
			P = Cin->table[0];	/* Can only be one table since we read a single file; We also only use the first segment */
			n_modes = (unsigned int)P->n_records;
			mode_direction = P->segment[0]->coord[GMT_X];
			mode_length = P->segment[0]->coord[GMT_Y];
		}
		if (!Ctrl->M.active) {	/* Must supply defaults for the vector attributes */
			Ctrl->M.S.size_x = VECTOR_HEAD_LENGTH * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 9p */
			Ctrl->M.S.v.v_width  = (float)(VECTOR_LINE_WIDTH * GMT->session.u2u[GMT_PT][GMT_INCH]);	/* 9p */
			Ctrl->M.S.v.v_angle  = 30.0f;
			Ctrl->M.S.v.status |= (GMT_VEC_OUTLINE + GMT_VEC_OUTLINE2 + GMT_VEC_FILL + GMT_VEC_FILL2 + GMT_VEC_END);
			GMT_init_pen (GMT, &Ctrl->M.S.v.pen, VECTOR_LINE_WIDTH);
			GMT_init_fill (GMT, &Ctrl->M.S.v.fill, 0.0, 0.0, 0.0);		/* Default vector fill = black */
		}
		GMT_init_vector_param (GMT, &Ctrl->M.S, false, false, NULL, false, NULL);
		Ctrl->M.S.v.v_width = (float)(Ctrl->W.pen[1].width * GMT->session.u2u[GMT_PT][GMT_INCH]);
		dim[2] = Ctrl->M.S.v.v_width, dim[3] = Ctrl->M.S.v.h_length, dim[4] = Ctrl->M.S.v.h_width;
		dim[5] = GMT->current.setting.map_vector_shape;
		dim[6] = (double)Ctrl->M.S.v.status;
		if (Ctrl->M.S.v.status & GMT_VEC_OUTLINE2) GMT_setpen (GMT, &Ctrl->W.pen[1]);
		if (Ctrl->M.S.v.status & GMT_VEC_FILL2) GMT_setfill (GMT, &Ctrl->M.S.v.fill, true);       /* Use fill structure */
		for (this_mode = 0; this_mode < n_modes; this_mode++) {
			if (Ctrl->N.active) mode_length[this_mode] = sqrt (mode_length[this_mode]);
			if (half_only && mode_direction[this_mode] > 90.0 && mode_direction[this_mode] <= 270.0) mode_direction[this_mode] -= 180.0;
			angle = start_angle - mode_direction[this_mode];
			sincosd (angle, &s, &c);
			xr = Ctrl->S.scale * mode_length[this_mode] * c;
			yr = Ctrl->S.scale * mode_length[this_mode] * s;
			dim[0] = xr, dim[1] = yr;
			PSL_plotsymbol (PSL, 0.0, 0.0, dim, PSL_VECTOR);
		}

	}
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	if (Ctrl->L.active) {	/* Deactivate those with - */
		if (Ctrl->L.w[0] == '-' && Ctrl->L.w[1] == '\0') Ctrl->L.w[0] = '\0';
		if (Ctrl->L.e[0] == '-' && Ctrl->L.e[1] == '\0') Ctrl->L.e[0] = '\0';
		if (Ctrl->L.s[0] == '-' && Ctrl->L.s[1] == '\0') Ctrl->L.s[0] = '\0';
		if (Ctrl->L.n[0] == '-' && Ctrl->L.n[1] == '\0') Ctrl->L.n[0] = '\0';
	}
	else {	/* Use default labels */
		Ctrl->L.w = strdup ("WEST");	Ctrl->L.e = strdup ("EAST");	Ctrl->L.s = strdup ("SOUTH");	Ctrl->L.n = strdup ("NORTH");
	}
	if (GMT->current.map.frame.draw) {	/* Draw grid lines etc */
		GMT_setpen (GMT, &GMT->current.setting.map_grid_pen[0]);
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
				PSL_plotarc (PSL, 0.0, 0.0, bin * GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval * Ctrl->S.scale, 0.0, total_arc, PSL_MOVE + PSL_STROKE);
		}
		PSL_setcolor (PSL, GMT->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);
		y = lsize + 6.0 * GMT->current.setting.map_annot_offset[0];
		form = GMT_setfont (GMT, &GMT->current.setting.font_title);
		PSL_plottext (PSL, 0.0, off + y, GMT->current.setting.font_title.size, GMT->current.map.frame.header, 0.0, 2, form);

		GMT_get_format (GMT, GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval, GMT->current.map.frame.axis[GMT_X].unit, GMT->current.map.frame.axis[GMT_X].prefix, format);

		if (half_only) {
			char text[GMT_LEN64] = {""};
			if (!Ctrl->L.active) {	/* Use default labels */
				free (Ctrl->L.w);	free (Ctrl->L.e);	free (Ctrl->L.n);
				if (GMT->current.setting.map_degree_symbol == gmt_none) {
					if (half_only == 1) {
						Ctrl->L.w = strdup ("90W");
						Ctrl->L.e = strdup ("90E");
						Ctrl->L.n = strdup ("0");
					}
					else {
						Ctrl->L.w = strdup ("0N");
						Ctrl->L.e = strdup ("180S");
						Ctrl->L.n = strdup ("90E");
					}
				}
				else {
					if (half_only == 1) {
						sprintf (text, "90%cW", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);	Ctrl->L.w = strdup (text);
						sprintf (text, "90%cE", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);	Ctrl->L.e = strdup (text);
						sprintf (text, "0%c",   (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);	Ctrl->L.n = strdup (text);
					}
					else {
						sprintf (text, "0%cN", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);	Ctrl->L.w = strdup (text);
						sprintf (text, "180%cS", (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);	Ctrl->L.e = strdup (text);
						sprintf (text, "90%cE",   (int)GMT->current.setting.ps_encoding.code[GMT->current.setting.map_degree_symbol]);	Ctrl->L.n = strdup (text);
					}
				}
			}
			form = GMT_setfont (GMT, &GMT->current.setting.font_label);
			y = -(3.0 * GMT->current.setting.map_annot_offset[0] + GMT->session.font[GMT->current.setting.font_annot[0].id].height * asize);
			if (GMT->current.map.frame.axis[GMT_X].label[0]) PSL_plottext (PSL, 0.0, y, GMT->current.setting.font_label.size, GMT->current.map.frame.axis[GMT_X].label, 0.0, 10, form);
			y = -(5.0 * GMT->current.setting.map_annot_offset[0] + GMT->session.font[GMT->current.setting.font_annot[0].id].height * lsize + GMT->session.font[GMT->current.setting.font_label.id].height * lsize);
			if (GMT->current.map.frame.axis[GMT_Y].label[0]) PSL_plottext (PSL, 0.0, y, GMT->current.setting.font_label.size, GMT->current.map.frame.axis[GMT_Y].label, 0.0, 10, form);
			form = GMT_setfont (GMT, &GMT->current.setting.font_annot[0]);
			PSL_plottext (PSL, 0.0, -GMT->current.setting.map_annot_offset[0], GMT->current.setting.font_annot[0].size, "0", 0.0, 10, form);
			n_annot = (GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval > 0.0) ? irint (max_radius / GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval) : -1;
			for (k = 1; n_annot > 0 && k <= n_annot; k++) {
				x = k * GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval;
				sprintf (text, format, x);
				x *= Ctrl->S.scale;
				PSL_plottext (PSL, x, -GMT->current.setting.map_annot_offset[0], GMT->current.setting.font_annot[0].size, text, 0.0, 10, form);
				PSL_plottext (PSL, -x, -GMT->current.setting.map_annot_offset[0], GMT->current.setting.font_annot[0].size, text, 0.0, 10, form);
			}
		}
		else {
			form = GMT_setfont (GMT, &GMT->current.setting.font_label);
			PSL_plottext (PSL, 0.0, -off - 2.0 * GMT->current.setting.map_annot_offset[0], GMT->current.setting.font_label.size, Ctrl->L.s, 0.0, 10, form);
			if (!Ctrl->F.active) {	/* Draw scale bar */
				PSL_plotsegment (PSL, off, -off, (max_radius - GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, -off);
				PSL_plotsegment (PSL, off, -off, off, GMT->current.setting.map_tick_length[0] - off);
				PSL_plotsegment (PSL, (max_radius - GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, -off, (max_radius - GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, GMT->current.setting.map_tick_length[0] - off);
				if (GMT->current.map.frame.axis[GMT_X].label[0]) {
					strcat (format, " %s");
					sprintf (text, format, GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval, GMT->current.map.frame.axis[GMT_X].label);
				}
				else
					sprintf (text, format, GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval);
				form = GMT_setfont (GMT, &GMT->current.setting.font_annot[0]);
				PSL_plottext (PSL, (max_radius - 0.5 * GMT->current.map.frame.axis[GMT_X].item[GMT_GRID_UPPER].interval) * Ctrl->S.scale, -(off + GMT->current.setting.map_annot_offset[0]), GMT->current.setting.font_annot[0].size, text, 0.0, 10, form);
			}
			y = -(off + 5.0 * GMT->current.setting.map_annot_offset[0] + GMT->session.font[GMT->current.setting.font_annot[0].id].height * lsize + GMT->session.font[GMT->current.setting.font_label.id].height * lsize);
			form = GMT_setfont (GMT, &GMT->current.setting.font_label);
			if (GMT->current.map.frame.axis[GMT_Y].label[0]) PSL_plottext (PSL, 0.0, y, GMT->current.setting.font_label.size, GMT->current.map.frame.axis[GMT_Y].label, 0.0, 10, form);
		}
		form = GMT_setfont (GMT, &GMT->current.setting.font_label);
		PSL_plottext (PSL, off + 2.0 * GMT->current.setting.map_annot_offset[0], 0.0, GMT->current.setting.font_label.size, Ctrl->L.e, 0.0, 5, form);
		PSL_plottext (PSL, -off - 2.0 * GMT->current.setting.map_annot_offset[0], 0.0, GMT->current.setting.font_label.size, Ctrl->L.w, 0.0, 7, form);
		PSL_plottext (PSL, 0.0, off + 2.0 * GMT->current.setting.map_annot_offset[0], GMT->current.setting.font_label.size, Ctrl->L.n, 0.0, 2, form);
		PSL_setcolor (PSL, GMT->current.setting.map_default_pen.rgb, PSL_IS_STROKE);
	}
	
	GMT_plane_perspective (GMT, -1, 0.0);
	PSL_setorigin (PSL, -x_origin, -y_origin, 0.0, PSL_INV);
	GMT_plotend (GMT);

	GMT_free (GMT, sum);
	GMT_free (GMT, xx);
	GMT_free (GMT, yy);
	GMT_free (GMT, azimuth);
	GMT_free (GMT, length);
	if (Ctrl->C.active) {
		if (find_mean) {
			GMT_free (GMT, mode_length);
			GMT_free (GMT, mode_direction);
		}
	}

	Return (GMT_OK);
}
