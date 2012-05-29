/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Brief synopsis: grdtrack reads a data table, opens the gridded file, 
 * and samples the dataset at the xy positions with a bilinear or bicubic
 * interpolant.
 *
 * Author:	Paul Wessel and Walter H F Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#include "gmt.h"

#define MAX_GRIDS GMT_BUFSIZ	/* Change and recompile if we need to sample more than GMT_BUFSIZ grids */

enum grdtrack_enum_stack {STACK_MEAN = 0,	/* Compute stack as mean value for same distance across all profiles */
	STACK_MEDIAN,				/* Use median instead */
	STACK_MODE,				/* Use mode (LMS) instead */
	STACK_LOWER,				/* Use lowest value encountered instead */
	STACK_LOWERP,				/* Use lowest value of all positive values encountered instead */
	STACK_UPPER,				/* Use highest value encountered instead */
	STACK_UPPERN};				/* Use highest value of all negative values encountered instead */

enum grdtrack_enum_opt {STACK_ADD_VAL = 0,	/* +a: Append stacked value(s) at end of each output row for all profiles */
	STACK_ADD_DEV,				/* +d: Compute deviation and add to end of each output row for all profiles */
	STACK_ADD_RES,				/* +r: Compute residual(s) (value - stacked value) and add to end of each output row for all profiles */
	STACK_ADD_TBL,				/* +s: Write stacked profile to given <file> */
	STACK_N_OPT};				/* Total number of modifiers */

struct GRD_CONTAINER {	/* Keep all the grid and sample parameters together */
	struct GMT_GRID *G;
	GMT_LONG type;	/* 0 = regular grid, 1 = img grid */
};

struct GRDTRACK_CTRL {
	struct In {
		GMT_BOOLEAN active;
		char *file;
	} In;
	struct Out {	/* -> */
		GMT_BOOLEAN active;
		char *file;
	} Out;
	struct A {	/* -A[m|p] */
		GMT_BOOLEAN active;
		int mode;
	} A;
	struct C {	/* -C<length>/<ds>[/<spacing>] */
		GMT_BOOLEAN active;
		GMT_LONG mode;	/* May be negative */
		char unit;
		double ds, spacing, length;
	} C;
	struct D {	/* -Dresampfile */
		GMT_BOOLEAN active;
		char *file;
	} D;
	struct G {	/* -G<grdfile> */
		GMT_BOOLEAN active;
		COUNTER_MEDIUM n_grids;
		char *file[MAX_GRIDS];
		double scale[MAX_GRIDS], lat[MAX_GRIDS];
		GMT_LONG mode[MAX_GRIDS];
		GMT_LONG type[MAX_GRIDS];	/*HIDDEN */
	} G;
	struct N {	/* -N */
		GMT_BOOLEAN active;
	} N;
	struct S {	/* -S[<mode>][<modifiers>] */
		GMT_BOOLEAN active;
		GMT_BOOLEAN selected[STACK_N_OPT];	/* For +a +d +e +r +s */
		COUNTER_MEDIUM mode;		/* Type of stack a|m|p|l|L|u|U */
		double factor;			/* Set via +c<factor> */
		char *file;			/* Output file for stack */
	} S;
	struct Z {	/* -Z */
		GMT_BOOLEAN active;
	} Z;
};

void *New_grdtrack_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDTRACK_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDTRACK_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->S.factor = 2.0;	/* +/- 2*sigma */
	return (C);
}

void Free_grdtrack_Ctrl (struct GMT_CTRL *GMT, struct GRDTRACK_CTRL *C) {	/* Deallocate control structure */
	COUNTER_MEDIUM g;
	if (!C) return;
	if (C->In.file) free (C->In.file);
	if (C->D.file) free (C->D.file);
	for (g = 0; g < C->G.n_grids; g++) if (C->G.file[g]) free (C->G.file[g]);	
	if (C->Out.file) free (C->Out.file);	
	if (C->S.file) free (C->S.file);
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdtrack_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdtrack %s [API] - Sample grids at specified (x,y) locations\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdtrack <table> -G<grid1> -G<grid2> ... [-A[m|p]] [-C<length>[u]/<ds>[u][/<spacing>[u]]]\n"); 
	GMT_message (GMT, "\t[-D<dfile>] [-N] [%s] [-S[<method>][<modifiers>]] [%s] [-Z] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n",
		GMT_Rgeo_OPT, GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_n_OPT, GMT_o_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<table> is an multicolumn ASCII file with (x, y) in the first two columns.\n");
	GMT_message (GMT, "\t-G Set the name of a more 2-D binary data set to sample.\n");
	GMT_message (GMT, "\t   If the file is a Sandwell/Smith Mercator grid (IMG format) instead,\n");
	GMT_message (GMT, "\t   append comma-separated scale (0.1 or 1), mode, and optionally max latitude [%g].  Modes are\n", GMT_IMG_MAXLAT_80);
	GMT_message (GMT, "\t     0 = img file w/ no constraint code, interpolate to get data at track.\n");
	GMT_message (GMT, "\t     1 = img file w/ constraints coded, interpolate to get data at track.\n");
	GMT_message (GMT, "\t     2 = img file w/ constraints coded, gets data only at constrained points, NaN elsewhere.\n");
	GMT_message (GMT, "\t     3 = img file w/ constraints coded, gets 1 at constraints, 0 elsewhere.\n");
	GMT_message (GMT, "\t   For mode 2|3 you may want to consider the -n+t<threshold> setting.\n");
	GMT_message (GMT, "\t   Repeat -G for as many grids as you wish to sample.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A For spherical surface sampling we follow great circle paths.\n");
	GMT_message (GMT, "\t   Append m or p to first follow meridian then parallel, or vice versa.\n");
	GMT_message (GMT, "\t   Ignored unless -C is used.\n");
	GMT_message (GMT, "\t-C Create equidistant cross-profiles from input line segments.  Append\n");
	GMT_message (GMT, "\t   <length>: Full-length of each cross profile.  Append desired distance unit [%s].\n", GMT_LEN_UNITS);
	GMT_message (GMT, "\t   <dz>: Distance interval along the cross-profiles.\n");
	GMT_message (GMT, "\t   Optionally, append /<spacing> to set the spacing between cross-profiles [Default use input locations].\n");
	GMT_message (GMT, "\t   Output columns are x, y, dist, z1, z2, ...\n");
	GMT_message (GMT, "\t   Default samples the grid(s) at the input data points.\n");
	GMT_message (GMT, "\t-D Save [resampled] input lines to a separate file <dfile>.  Requires -C.\n");
	GMT_message (GMT, "\t   Output columns are lon, lat, dist, az, z1, z2, ...\n");
	GMT_message (GMT, "\t-N Do NOT skip points outside the grid domain [Default only returns points inside domain].\n");
	GMT_explain_options (GMT, "R");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-S In conjunction with -C, compute a single stacked profile from all profiles across each segment.\n");
	GMT_message (GMT, "\t   Append which method should be used when performing the stacking:\n");
	GMT_message (GMT, "\t   a = mean, m = median, p = mode, l = lower, L = lower of +ve values, u = upper, U = upper of -ve values [a].\n");
	GMT_message (GMT, "\t   The modifiers control what is being written; choose one or more among\n");
	GMT_message (GMT, "\t     +a : Append stacked values to all cross-profiles.\n");
	GMT_message (GMT, "\t     +d : Append stack deviations to all cross-profiles.\n");
	GMT_message (GMT, "\t     +r : Append data residuals (data - stack) to all cross-profiles.\n");
	GMT_message (GMT, "\t     +s[<file>] : Save stacked profile to <file> [stacked_profile.txt].\n");
	GMT_message (GMT, "\t     +c<fact> : Compute envelope as +/- <fact>*deviation [2].\n");
	GMT_message (GMT, "\t   Note: Deviations depend on mode and are L1 scale (e), st.dev (a), LMS scale (p), or half-range (u-l)/2.\n");
	GMT_message (GMT, "\t-Z Only output z-values [Default gives all columns].\n");
	GMT_explain_options (GMT, "C2D0fghinos:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdtrack_parse (struct GMTAPI_CTRL *C, struct GRDTRACK_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdsample and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j;
	COUNTER_MEDIUM pos, n_errors = 0, ng = 0, n_files = 0;
	char line[GMT_BUFSIZ], ta[GMT_TEXT_LEN64], tb[GMT_TEXT_LEN64], tc[GMT_TEXT_LEN64], p[GMT_TEXT_LEN256];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	GMT_memset (line, GMT_BUFSIZ, char);

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;
			case '>':	/* Specified output file */
				if (n_files++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Change spherical sampling mode */
				Ctrl->A.active = TRUE;
				switch (opt->arg[0]) {
					case 'm': Ctrl->A.mode = 1; break;
					case 'p': Ctrl->A.mode = 2; break;
				}
				break;
			case 'C':	/* Create cross profiles */
				Ctrl->C.active = TRUE;
				j = sscanf (opt->arg, "%[^/]/%[^/]/%s", ta, tb, tc);
				Ctrl->C.mode = GMT_get_distance (GMT, ta, &(Ctrl->C.length), &(Ctrl->C.unit));
				Ctrl->C.mode = GMT_get_distance (GMT, tb, &(Ctrl->C.ds), &(Ctrl->C.unit));
				if (j == 3) Ctrl->C.mode = GMT_get_distance (GMT, tc, &(Ctrl->C.spacing), &(Ctrl->C.unit));
				break;
			case 'D':	/* Dump resampled lines */
				Ctrl->D.active = TRUE;
				Ctrl->D.file = strdup (opt->arg);
				break;
			case 'G':	/* Input grid file */
				if (ng == MAX_GRIDS) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -G option: Too many grids (max = %d)\n", MAX_GRIDS);
					n_errors++;
				}
				else {
					Ctrl->G.active = TRUE;
					Ctrl->G.scale[ng] = 1.0;
					if (strchr (opt->arg, ',') && !strchr (opt->arg, '?')) {	/* IMG grid file with required parameters */
						if ((j = sscanf (opt->arg, "%[^,],%lf,%d,%lf", line, &Ctrl->G.scale[ng], &Ctrl->G.mode[ng], &Ctrl->G.lat[ng])) < 3) {
							GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -G option: Give imgfile, scale, mode [and optionally max_lat]\n");
							n_errors++;
						}
						else
							Ctrl->G.file[ng] = strdup (line);
						Ctrl->G.type[ng] = 1;
						n_errors += GMT_check_condition (GMT, Ctrl->G.mode[ng] < 0 || Ctrl->G.mode[ng] > 3, "Syntax error -G: mode must be in 0-3 range\n");
						n_errors += GMT_check_condition (GMT, Ctrl->G.lat[ng] < 0.0, "Syntax error -G: max latitude should be positive\n");
					}
					else
						Ctrl->G.file[ng] = strdup (opt->arg);
					n_errors += GMT_check_condition (GMT, !Ctrl->G.file[ng], "Syntax error -G: Must specify input file\n");
					ng++;
				}
				break;
			case 'L':	/* Sets BCs */
#ifdef GMT_COMPAT
				if (opt->arg[0]) {
#endif
					GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -L<flag> is deprecated; -n+b%s was set instead, use this in the future.\n", opt->arg);
					strncpy (GMT->common.n.BC, opt->arg, 4U);
#ifdef GMT_COMPAT
				}
				else {
					GMT->current.io.col_type[GMT_IN][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
					GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
					GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -L is deprecated; please use -f instead.\n");
				}
#endif
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				break;
			case 'S':
#ifdef GMT_COMPAT
				if (opt->arg[0] == 0) {	/* Under COMPAT: Interpret -S (no args) as old-style -S option to skip output with NaNs */
					GMT_report (GMT, GMT_MSG_FATAL, "Warning: Option -S deprecated. Use -sa instead.\n");
					GMT->current.setting.io_nan_mode = 3;
					break;
				}
#endif
				Ctrl->S.active = TRUE;
				switch (opt->arg[0]) {
					case 'a': Ctrl->S.mode = STACK_MEAN;   break;
					case 'm': Ctrl->S.mode = STACK_MEDIAN; break;
					case 'p': Ctrl->S.mode = STACK_MODE;   break;
					case 'l': Ctrl->S.mode = STACK_LOWER;  break;
					case 'L': Ctrl->S.mode = STACK_LOWERP;  break;
					case 'u': Ctrl->S.mode = STACK_UPPER;  break;
					case 'U': Ctrl->S.mode = STACK_UPPERN;  break;
					default:
						n_errors++; 
						GMT_report (GMT, GMT_MSG_FATAL, "Error: Bad mode (%c) given to -S.\n", (int)opt->arg[0]);
						break;
				}
				pos = 0;
				while (GMT_strtok (&opt->arg[1], "+", &pos, p)) {
					switch (p[0]) {
						case 'a': Ctrl->S.selected[STACK_ADD_VAL] = TRUE; break;	/* Gave +a to add stacked value to all output profiles */
						case 'd': Ctrl->S.selected[STACK_ADD_DEV] = TRUE; break;	/* Gave +d to add stacked deviations to all output profiles */
						case 'r': Ctrl->S.selected[STACK_ADD_RES] = TRUE; break;	/* Gave +r to add residual values (data - stack) to all output profiles */
						case 'c': Ctrl->S.factor = atof (&p[1]); break;			/* Gave +c to scale deviations for use in making envelopes [2] */
						case 's': Ctrl->S.selected[STACK_ADD_TBL] = TRUE;		/* Gave +s to write stacked profile to given table */
							Ctrl->S.file = (p[1]) ? strdup (&p[1]) : strdup ("stacked_profile.txt");
							break;
						default:
							n_errors++; 
							GMT_report (GMT, GMT_MSG_FATAL, "Error: Bad modifier (%s) given to -S.\n", p[0]);
							break;
					}
				}
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	Ctrl->G.n_grids = ng;
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && !Ctrl->C.active, "Syntax error -S: Requires -C.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && !(Ctrl->S.selected[STACK_ADD_VAL] || Ctrl->S.selected[STACK_ADD_DEV] || Ctrl->S.selected[STACK_ADD_RES] || Ctrl->S.selected[STACK_ADD_TBL]), "Syntax error -S: Must specify at least one modifier.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->S.factor <= 0.0, "Syntax error -S: +c<factor> must be positive.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && !Ctrl->D.file, "Syntax error -D: Must specify file name.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.n_grids == 0, "Syntax error: Must specify -G at least once\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && (Ctrl->C.spacing < 0.0 || Ctrl->C.ds < 0.0 || Ctrl->C.length < 0.0), "Syntax error -C: Arguments must be positive\n");
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += GMT_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LONG sample_all_grids (struct GMT_CTRL *GMT, struct GRD_CONTAINER *GC, COUNTER_MEDIUM n_grids, GMT_BOOLEAN img, double x_in, double y_in, double value[])
{
	COUNTER_MEDIUM g, n_in, n_set;
	double x, y, x0 = 0.0, y0 = 0.0;
	
	if (img) GMT_geo_to_xy (GMT, x_in, y_in, &x0, &y0);	/* At least one Mercator IMG grid in use - get Mercator coordinates x,y */

	for (g = n_in = n_set = 0; g < n_grids; g++) {
		y = (GC[g].type == 1) ? y0 : y_in;
		value[g] = GMT->session.d_NaN;	/* In case the point is outside only some of the grids */
		/* If point is outside grd area, shift it using periodicity or skip if not periodic. */

		while ((y < GC[g].G->header->wesn[YLO]) && (GC[g].G->header->nyp > 0)) y += (GC[g].G->header->inc[GMT_Y] * GC[g].G->header->nyp);
		if (y < GC[g].G->header->wesn[YLO]) continue;

		while ((y > GC[g].G->header->wesn[YHI]) && (GC[g].G->header->nyp > 0)) y -= (GC[g].G->header->inc[GMT_Y] * GC[g].G->header->nyp);
		if (y > GC[g].G->header->wesn[YHI]) continue;

		if (GC[g].type == 1) {	/* This grid is in Mercator x/y units - must use Mercator x0, y0 */
			x = x0;
			if (x > GC[g].G->header->wesn[XHI]) x -= 360.0;
		}
		else {	/* Regular Cartesian x,y or lon,lat */
			x = x_in;
			if (GMT_is_geographic (GMT, GMT_IN)) {	/* Must wind lonn/lat to fit current grid longitude range */
				while (x > GC[g].G->header->wesn[XHI]) x -= 360.0;
				while (x < GC[g].G->header->wesn[XLO]) x += 360.0;
			}
		}
		while ((x < GC[g].G->header->wesn[XLO]) && (GC[g].G->header->nxp > 0)) x += (GC[g].G->header->inc[GMT_X] * GC[g].G->header->nxp);
		if (x < GC[g].G->header->wesn[XLO]) continue;

		while ((x > GC[g].G->header->wesn[XHI]) && (GC[g].G->header->nxp > 0)) x -= (GC[g].G->header->inc[GMT_X] * GC[g].G->header->nxp);
		if (x > GC[g].G->header->wesn[XHI]) continue;

		n_in++;	/* This point is inside the current grid's domain */
		value[g] = GMT_get_bcr_z (GMT, GC[g].G, x, y);

		if (!GMT_is_dnan (value[g])) n_set++;	/* Count value results */
	}
	
	if (n_in == 0) return (-1);
	return (n_set);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdtrack_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_grdtrack (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args) {
	/* High-level function that implements the grdtrack task */

	GMT_LONG status, error, ks;
	COUNTER_LARGE n_points = 0, n_read = 0;
	COUNTER_MEDIUM g, k;
	GMT_BOOLEAN img_conv_needed = FALSE;
	
	char line[GMT_BUFSIZ];

	double *value, wesn[4];

	struct GRDTRACK_CTRL *Ctrl = NULL;
	struct GRD_CONTAINER *GC = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_grdtrack_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_grdtrack_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdtrack", &GMT_cpy);		/* Save current state */
	if (GMT_Parse_Common (API, "-VRbf:", "ghinos>" GMT_OPT("HMmQ"), options)) Return (API->error);
	Ctrl = New_grdtrack_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdtrack_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdtrack main code ----------------------------*/

	if (GMT_Init_IO (API, GMT_IS_DATASET, Ctrl->C.active ? GMT_IS_LINE : GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}

	GMT_memset (wesn, 4, double);
	if (GMT->common.R.active) GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Specified a subset */

	GC = GMT_memory (GMT, NULL, Ctrl->G.n_grids, struct GRD_CONTAINER);
	value = GMT_memory (GMT, NULL, Ctrl->G.n_grids, double);
	
	for (g = 0; g < Ctrl->G.n_grids; g++) {
		GC[g].type = Ctrl->G.type[g];
		if (Ctrl->G.type[g] == 0) {	/* Regular GMT grids */
			if ((GC[g].G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER, NULL, Ctrl->G.file[g], NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			if (GMT->common.R.active) GMT_err_fail (GMT, GMT_adjust_loose_wesn (GMT, wesn, GC[g].G->header), "");		/* Subset requested; make sure wesn matches header spacing */

			if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, GC[g].G->header->wesn, 4, double);

			if (!GMT_grd_setregion (GMT, GC[g].G->header, wesn, BCR_BILINEAR)) {
				GMT_report (GMT, GMT_MSG_NORMAL, "Warning: No data within specified region\n");
				Return (GMT_OK);
			}

			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA, wesn, Ctrl->G.file[g], GC[g].G) == NULL) {	/* Get subset */
				Return (API->error);
			}

			GMT_memcpy (GMT->common.R.wesn, wesn, 4, double);

		}
		else {	/* Sandwell/Smith Mercator grids */
			if ((GC[g].G = GMT_Create_Data (API, GMT_IS_GRID, NULL)) == NULL) Return (API->error);
			GMT_read_img (GMT, Ctrl->G.file[g], GC[g].G, wesn, Ctrl->G.scale[g], Ctrl->G.mode[g], Ctrl->G.lat[g], TRUE);
			img_conv_needed = TRUE;
		}
	}
	
	if (Ctrl->C.active) {	/* Special case of requesting cross-profiles for given line segments */
		COUNTER_MEDIUM tbl, col, n_cols = Ctrl->G.n_grids;
		COUNTER_LARGE row, seg;
		struct GMT_DATASET *Din = NULL, *Dout = NULL, *Dtmp = NULL;
		struct GMT_TABLE *T = NULL;
		struct GMT_LINE_SEGMENT *S = NULL;
		
		if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_ANY, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}

		GMT_init_distaz (GMT, Ctrl->C.unit, Ctrl->C.mode, GMT_MAP_DIST);
		/* Expand with dist,az columns (mode = 2) (and posibly make space for more) and optionally resample */
		if ((Dtmp = GMT_resample_data (GMT, Din, Ctrl->C.spacing, 2, (Ctrl->D.active) ? Ctrl->G.n_grids : 0, Ctrl->A.mode)) == NULL) Return (API->error);
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &Din) != GMT_OK) {
			Return (API->error);
		}
		if (Ctrl->D.active) {	/* Also want to sample grids along the original resampled trace */
			for (tbl = 0; tbl < Dtmp->n_tables; tbl++) {
				T = Dtmp->table[tbl];
				for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment to resample */
					S = T->segment[seg];
					for (row = 0; row < S->n_rows; row++) {	/* For each row to resample */
						status = sample_all_grids (GMT, GC, Ctrl->G.n_grids, img_conv_needed, S->coord[GMT_X][row], S->coord[GMT_Y][row], value);
						for (col = 4, k = 0; k < Ctrl->G.n_grids; k++, col++) S->coord[col][row] = value[k];
					}
				}
			}
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_WRITE_SET, NULL, Ctrl->D.file, Dtmp) != GMT_OK) {
				Return (API->error);
			}
		}
		/* Get dataset with cross-profiles, with columns for x,y,d and the n_grids samples */
		if (Ctrl->S.active) {	/* Want stacked profiles - determine how many extra columns to make space for */
			if (Ctrl->S.selected[STACK_ADD_VAL]) n_cols += Ctrl->G.n_grids;	/* Make space for the stacked value(s) in each profile */
			if (Ctrl->S.selected[STACK_ADD_DEV]) n_cols += Ctrl->G.n_grids;	/* Make space for the stacked deviations(s) in each profile */
			if (Ctrl->S.selected[STACK_ADD_RES]) n_cols += Ctrl->G.n_grids;	/* Make space for the stacked residuals(s) in each profile */
		}
		if ((Dout = GMT_crosstracks (GMT, Dtmp, Ctrl->C.length, Ctrl->C.ds, n_cols)) == NULL) Return (API->error);
		if (Ctrl->D.active) {
			if (GMT_Destroy_Data (API, GMT_ALLOCATED, &Dtmp) != GMT_OK) {
				Return (API->error);
			}
		}
		else	/* Never written */
			GMT_free_dataset (GMT, &Dtmp);
		
		/* Sample the grids along all profiles in Dout */
		
		for (tbl = 0; tbl < Dout->n_tables; tbl++) {
			T = Dout->table[tbl];
			for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment to resample */
				S = T->segment[seg];
				for (row = 0; row < S->n_rows; row++) {	/* For each row to resample */
					status = sample_all_grids (GMT, GC, Ctrl->G.n_grids, img_conv_needed, S->coord[GMT_X][row], S->coord[GMT_Y][row], value);
					for (col = 4, k = 0; k < Ctrl->G.n_grids; k++, col++) S->coord[col][row] = value[k];
					if (status != -1) n_points++;
				}
			}
		}
		if (Dout->n_segments > 1) GMT_set_segmentheader (GMT, GMT_OUT, TRUE);	/* Turn on segment headers on output */
		
		if (Ctrl->S.active) {	/* Compute the stacked profiles */
			struct GMT_DATASET *Stack = NULL;
			struct GMT_LINE_SEGMENT *M = NULL;
			COUNTER_LARGE dim[4], n_rows;
			COUNTER_MEDIUM n_step = (Ctrl->S.mode < STACK_LOWER) ? 6 : 4;	/* Number of columns per gridded data in stack file */
			COUNTER_MEDIUM colx, col0 = 4 + Ctrl->G.n_grids;		/* First column for stacked value in cross-profiles */
			COUNTER_MEDIUM GMT_mode_selection = 0, GMT_n_multiples = 0;
			double **stack = NULL, *stacked_val = NULL, *stacked_dev = NULL, *stacked_hi = NULL, *stacked_lo = NULL, *dev = NULL;
			dim[0] = 1;				/* One table */
			dim[1] = Dout->n_tables;		/* Number of stacks */
			dim[2] = 1 + n_step * Ctrl->G.n_grids;	/* Number of columns needed in stack file */
			dim[3] = n_rows = Dout->table[0]->segment[0]->n_rows;	/* Number of rows */
			if ((Stack = GMT_Create_Data (API, GMT_IS_DATASET, dim)) == NULL) Return (API->error);	/* An empty table for stacked results */
			
			stack = GMT_memory (GMT, NULL, Ctrl->G.n_grids, double *);
			stacked_val = GMT_memory (GMT, NULL, Ctrl->G.n_grids, double);
			stacked_dev = GMT_memory (GMT, NULL, Ctrl->G.n_grids, double);
			stacked_lo = GMT_memory (GMT, NULL, Ctrl->G.n_grids, double);
			stacked_hi = GMT_memory (GMT, NULL, Ctrl->G.n_grids, double);
			
			for (tbl = 0; tbl < Dout->n_tables; tbl++) {
				T = Dout->table[tbl];
				M = Stack->table[0]->segment[tbl];	/* Current stack */
				for (k = 0; k < Ctrl->G.n_grids; k++) {
					stack[k] = GMT_memory (GMT, NULL, T->n_segments, double);
					stacked_hi[k] = -DBL_MAX;
					stacked_lo[k] = +DBL_MAX;
				}
				if (Ctrl->S.mode == STACK_MEDIAN || Ctrl->S.mode == STACK_MODE) dev = GMT_memory (GMT, NULL, Dout->table[tbl]->n_segments, double);
				for (row = 0; row < n_rows; row++) {	/* For each row to stack across all segments */
					for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment to resample */
						for (col = 4, k = 0; k < Ctrl->G.n_grids; k++, col++) {	/* Collect sampled values across all profiles for same row into temp array */
							stack[k][seg] = T->segment[seg]->coord[col][row];
							if (GMT_is_dnan (stack[k][seg])) continue;
							if (stack[k][seg] > stacked_hi[k]) stacked_hi[k] = stack[k][seg];
							if (stack[k][seg] < stacked_lo[k]) stacked_lo[k] = stack[k][seg];
						}
					}
					switch (Ctrl->S.mode) {	/* Compute stacked value */
						case STACK_MEAN:   for (k = 0; k < Ctrl->G.n_grids; k++) stacked_val[k] = GMT_mean_and_std (GMT, stack[k], T->n_segments, &stacked_dev[k]); break;
						case STACK_MEDIAN: for (k = 0; k < Ctrl->G.n_grids; k++) GMT_median (GMT, stack[k], T->n_segments, stacked_lo[k], stacked_hi[k], 0.5*(stacked_lo[k]+stacked_hi[k]), &stacked_val[k]); break;
						case STACK_MODE:   for (k = 0; k < Ctrl->G.n_grids; k++) GMT_mode (GMT, stack[k], T->n_segments, T->n_segments/2, 0, GMT_mode_selection, &GMT_n_multiples, &stacked_val[k]); break;
						case STACK_LOWER:  for (k = 0; k < Ctrl->G.n_grids; k++) stacked_val[k] = GMT_extreme (GMT, stack[k], T->n_segments, 0.0, 0, -1); break;
						case STACK_LOWERP:  for (k = 0; k < Ctrl->G.n_grids; k++) stacked_val[k] = GMT_extreme (GMT, stack[k], T->n_segments, 0.0, +1, -1); break;
						case STACK_UPPER:  for (k = 0; k < Ctrl->G.n_grids; k++) stacked_val[k] = GMT_extreme (GMT, stack[k], T->n_segments, 0.0, 0, +1); break;
						case STACK_UPPERN:  for (k = 0; k < Ctrl->G.n_grids; k++) stacked_val[k] = GMT_extreme (GMT, stack[k], T->n_segments, 0.0, -1, +1); break;
					}
					if (Ctrl->S.mode == STACK_MEDIAN || Ctrl->S.mode == STACK_MODE) {	/* Compute deviations via stack residuals */
						for (k = 0; k < Ctrl->G.n_grids; k++) {
							for (seg = 0; seg < T->n_segments; seg++) dev[seg] = fabs (stack[k][seg] - stacked_val[k]);
							GMT_median (GMT, dev, T->n_segments, stacked_lo[k] - stacked_val[k], stacked_hi[k] - stacked_val[k], 0.5*(stacked_lo[k]+stacked_hi[k]) - stacked_val[k], &stacked_dev[k]);
							stacked_dev[k] *= 1.4826;
						}
					}
					else if (Ctrl->S.mode >= STACK_LOWER) {	/* Use half-range as deviation */
						for (k = 0; k < Ctrl->G.n_grids; k++) stacked_dev[k] = 0.5 * (stacked_lo[k] + stacked_hi[k]);
					}
					/* Here we have everything needed to populate output arrays */
					M->coord[0][row] = T->segment[0]->coord[2][row];	/* Copy over distance value */
					for (col = 4, colx = col0, k = 0; k < Ctrl->G.n_grids; k++, col++) {	/* Place stacked, deviation, low, high [and lo_env hi_env] for each grid */
						M->coord[1+k*n_step][row] = stacked_val[k];	/* The stacked value */
						M->coord[2+k*n_step][row] = stacked_dev[k];	/* The stacked deviation */
						M->coord[3+k*n_step][row] = stacked_lo[k];	/* The stacked low value */
						M->coord[4+k*n_step][row] = stacked_hi[k];	/* The stacked high value */
						if (Ctrl->S.mode >= STACK_LOWER) continue;
						M->coord[5+k*n_step][row] = stacked_val[k] - Ctrl->S.factor * stacked_dev[k];	/* The low envelope value */
						M->coord[6+k*n_step][row] = stacked_val[k] + Ctrl->S.factor * stacked_dev[k];	/* The low envelope value */
						if (Ctrl->S.selected[STACK_ADD_VAL]) T->segment[seg]->coord[colx++][row] = stacked_val[k];	/* Place stacked value at end of profile */
						if (Ctrl->S.selected[STACK_ADD_DEV]) T->segment[seg]->coord[colx++][row] = stacked_dev[k];	/* Place deviation at end of profile */
						if (Ctrl->S.selected[STACK_ADD_RES]) T->segment[seg]->coord[colx++][row] = T->segment[seg]->coord[col][row] - stacked_val[k];	/* Place residuals(s) at end of profile */
					}
				}
				for (k = 0; k < Ctrl->G.n_grids; k++) GMT_free (GMT, stack[k]);
				if (Ctrl->S.mode == STACK_MEDIAN || Ctrl->S.mode == STACK_MODE) GMT_free (GMT, dev);
			}
			GMT_free (GMT, stack);
			GMT_free (GMT, stacked_val);
			GMT_free (GMT, stacked_dev);
			GMT_free (GMT, stacked_lo);
			GMT_free (GMT, stacked_hi);
			if (Ctrl->S.selected[STACK_ADD_TBL] && GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, Stack->io_mode, NULL, Ctrl->S.file, Stack) != GMT_OK) {
				Return (API->error);
			}
		}
		
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, Dout->io_mode, NULL, Ctrl->Out.file, Dout) != GMT_OK) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &Dout) != GMT_OK) {
			Return (API->error);
		}
	}
	else {	/* Standard resampling point case */
		GMT_BOOLEAN pure_ascii = FALSE;
		GMT_LONG ix, iy, n_fields, rmode;
		double *in = NULL, *out = NULL;
		char record[GMT_BUFSIZ];
GMT_LONG gmt_skip_output (struct GMT_CTRL *C, double *cols, GMT_LONG n_cols);
		
		pure_ascii = !(GMT->common.b.active[GMT_IN] || GMT->common.b.active[GMT_OUT] || GMT->common.o.active);

		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET,  GMT_IN) != GMT_OK) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}

		GMT_memset (line, GMT_BUFSIZ, char);
		if (Ctrl->Z.active) {
			GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_FLOAT;	/* Since we are outputting z all columns */
			GMT->common.b.ncol[GMT_OUT] = Ctrl->G.n_grids;
		}
		ix = (GMT->current.setting.io_lonlat_toggle[GMT_IN]);	iy = 1 - ix;
		rmode = (pure_ascii && GMT_get_cols (GMT, GMT_IN) >= 2) ? GMT_READ_MIXED : GMT_READ_DOUBLE;

		do {	/* Keep returning records until we reach EOF */
			if ((in = GMT_Get_Record (API, rmode, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
				if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (GMT_REC_IS_TBL_HEADER (GMT)) {	/* Echo table headers */
					GMT_Put_Record (API, GMT_WRITE_TBLHEADER, NULL);
					continue;
				}
				if (GMT_REC_IS_SEG_HEADER (GMT)) {			/* Echo segment headers */
					GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
					continue;
				}
				if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
					break;
			}

			/* Data record to process */

			if (GMT->common.b.ncol[GMT_OUT] == 0) GMT->common.b.ncol[GMT_OUT] = GMT->common.b.ncol[GMT_IN] + Ctrl->G.n_grids;	/* Set # of output cols */
			n_read++;

			status = sample_all_grids (GMT, GC, Ctrl->G.n_grids, img_conv_needed, in[GMT_X], in[GMT_Y], value);
			if (status == -1 && !Ctrl->N.active) continue;		/* Point is outside the region of all grids */

			if (Ctrl->Z.active)	/* Simply print out values */
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, value);
			else if (pure_ascii && n_fields >= 2) {
				/* Special case: Ascii i/o and at least 3 columns:
				   Columns beyond first two could be text strings */
				if (gmt_skip_output (GMT, value, Ctrl->G.n_grids)) continue;	/* Suppress output due to NaNs */

				/* First get rid of any commas that may cause grief */
				for (k = 0; GMT->current.io.current_record[k]; k++) if (GMT->current.io.current_record[k] == ',') GMT->current.io.current_record[k] = ' ';
				record[0] = 0;
				sscanf (GMT->current.io.current_record, "%*s %*s %[^\n]", line);
				GMT_add_to_record (GMT, record, in[ix], ix, 2);	/* Format our output x value */
				GMT_add_to_record (GMT, record, in[iy], iy, 2);	/* Format our output y value */
				strcat (record, line);
				for (g = 0; g < Ctrl->G.n_grids; g++) {
					GMT_add_to_record (GMT, record, value[g], GMT_Z+g, 1);	/* Format our output y value */
				}
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write this to output */
			}
			else {	/* Simply copy other columns, append value, and output */
				if (!out) out = GMT_memory (GMT, NULL, GMT->common.b.ncol[GMT_OUT], double);
				for (ks = 0; ks < n_fields; ks++) out[ks] = in[ks];
				for (g = 0; g < Ctrl->G.n_grids; g++, ks++) out[ks] = value[g];
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
			}

			n_points++;
		} while (TRUE);
		
		if (GMT_End_IO (API, GMT_IN,  0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}

		if (out) GMT_free (GMT, out);
	}
	/* Clean up */
	for (g = 0; g < Ctrl->G.n_grids; g++) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Sampled %" PRIu64 " points from grid %s (%d x %d)\n",
			n_points, Ctrl->G.file[g], GC[g].G->header->nx, GC[g].G->header->ny);
		if (Ctrl->G.type[g] == 0 && GMT_Destroy_Data (API, GMT_ALLOCATED, &GC[g].G) != GMT_OK) {
			Return (API->error);
		}
		else
			GMT_free_grid (GMT, &GC[g].G, TRUE);
	}
	GMT_free (GMT, value);
	GMT_free (GMT, GC);

	Return (EXIT_SUCCESS);
}
