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
 * Brief synopsis: grdtrack reads a data table, opens the gridded file, 
 * and samples the dataset at the xy positions with a bilinear or bicubic
 * interpolant.
 *
 * Author:	Paul Wessel and Walter H F Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"grdtrack"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Sample grids at specified (x,y) locations"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>RVabfghinos" GMT_OPT("HMmQ")

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
	int type;	/* 0 = regular grid, 1 = img grid */
};

struct GMT_ZSEARCH {	/* For -T */
	double *x, *y;		/* Arrays with grid coordinates */
	double x0, y0;		/* Coordinates of central NaN node */
	double radius;		/* Smallest distance so far to non-NaN node */
	double max_radius;	/* Only consider radii less than this cutoff [or DBL_MAX for no limit] */
	uint64_t row0, col0;	/* Location of our NaN node */
	uint64_t row, col;	/* Location of nearest non-NaN node that was found */
	struct GRD_CONTAINER *C;	/* Pointer to the grid container */
};

struct GRDTRACK_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct A {	/* -A[f|m|p|r|R][+l] */
		bool active, loxo;
		enum GMT_enum_track mode;
	} A;
	struct C {	/* -C<length>/<ds>[/<spacing>][+a] */
		bool active, alternate;
		int mode;	/* May be negative */
		char unit;
		double ds, spacing, length;
	} C;
	struct D {	/* -Dresampfile */
		bool active;
		char *file;
	} D;
	struct E {	/* -E<line1>[,<line2>,...][+a<az>][+i<step>][+l<length>][+n<np][+o<az>][+r<radius>] */
		bool active;
		unsigned int mode;
		char *lines;
		double step;
		char unit;
	} E;
	struct G {	/* -G<grdfile> */
		bool active;
		unsigned int n_grids;
		char *file[MAX_GRIDS];
		double scale[MAX_GRIDS], lat[MAX_GRIDS];
		int mode[MAX_GRIDS];
		int type[MAX_GRIDS];	/*HIDDEN */
	} G;
	struct N {	/* -N */
		bool active;
	} N;
	struct S {	/* -S[<mode>][<modifiers>] */
		bool active;
		bool selected[STACK_N_OPT];	/* For +a +d +e +r +s */
		unsigned int mode;		/* Type of stack a|m|p|l|L|u|U */
		double factor;			/* Set via +c<factor> */
		char *file;			/* Output file for stack */
	} S;
	struct T {	/* -T[<radius>[unit]][+p|e] */
		bool active;
		double radius;		/* Max radius to search */
		int dmode;		/* Distance mode; could be negative */
		char unit;		/* Distance unit */
		unsigned int mode;	/* 1 if +p was given, 2 if +e was given */
		struct GMT_ZSEARCH *S;
	} T;
	struct Z {	/* -Z */
		bool active;
	} Z;
};

void *New_grdtrack_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDTRACK_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDTRACK_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->S.factor = 2.0;	/* +/- 2*sigma */
	return (C);
}

void Free_grdtrack_Ctrl (struct GMT_CTRL *GMT, struct GRDTRACK_CTRL *C) {	/* Deallocate control structure */
	unsigned int g;
	if (!C) return;
	if (C->In.file) free (C->In.file);
	if (C->D.file) free (C->D.file);
	for (g = 0; g < C->G.n_grids; g++) if (C->G.file[g]) free (C->G.file[g]);	
	if (C->Out.file) free (C->Out.file);	
	if (C->S.file) free (C->S.file);
	if (C->E.lines) free (C->E.lines);
	if (C->T.S) {
		GMT_free (GMT, C->T.S->x);
		GMT_free (GMT, C->T.S->y);
		GMT_free (GMT, C->T.S);
	}
	GMT_free (GMT, C);	
}

int GMT_grdtrack_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdtrack <table> -G<grid1> -G<grid2> ... [-A[f|m|p|r|R][+l]]\n\t[-C<length>[u]/<ds>[/<spacing>][+a]] [-D<dfile>]\n"); 
	GMT_Message (API, GMT_TIME_NONE, "\t[-E<line1>[,<line2>,...][+a<az>][+d][+i<step>][+l<length>][+n<np][+o<az>][+r<radius>]]\n\t[-N] [%s] [-S[<method>][<modifiers>]] [-T<radius>[unit]>[+e|p]] [%s] [-Z]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n",
		GMT_Rgeo_OPT, GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_n_OPT, GMT_o_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<table> is an multicolumn ASCII file with (x, y) in the first two columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set the name of a more 2-D binary data set to sample.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   See below if the file is a Sandwell/Smith Mercator grid (IMG format).\n");
	GMT_img_syntax (API->GMT);
	GMT_Message (API, GMT_TIME_NONE, "\t   Repeat -G for as many grids as you wish to sample.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Controls how the input track in <table> is resampled when -C is selected:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f: Keep original points, but add intermediate points if needed [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m: Same, but first follow meridian (along y) then parallel (along x).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p: Same, but first follow parallel (along x) then meridian (along y).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r: Resample at equidistant locations; input points not necessarily included.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   R: Same, but adjust given spacing to fit the track length exactly.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +l to compute distances along rhumblines (loxodromes) [no].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Create equidistant cross-profiles from input line segments.  Append\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <length>: Full-length of each cross profile.  Append distance unit [%s]; it also applies to <ds>, <spacing>.\n", GMT_LEN_UNITS);
	GMT_Message (API, GMT_TIME_NONE, "\t   <dz>: Distance interval along the cross-profiles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append /<spacing> to set the spacing between cross-profiles [Default use input locations].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +a to alternate direction of cross-profiles [Default orients all the same way].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Output columns are x, y, dist, z1, z2, ...\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default samples the grid(s) at the input data points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Save [resampled] input lines to a separate file <dfile>.  Requires -C.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Output columns are lon, lat, dist, az, z1, z2, ...\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set quick paths based on <line1>[,<line2>,...]. Give start and stop coordinates for\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   each line segment.  The format of each <line> is <start>/<stop>, where <start> or <stop>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   are <lon/lat> or a 2-character XY key that uses the \"pstext\"-style justification format\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   format to specify a point on the map as [LCR][BMT].  In addition, you can use Z-, Z+ to mean\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the global minimum and maximum locations in the grid.  Note: No track file is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +i<inc>[unit] to set the sampling increment [Default is 0.5 x min of (x_inc, y_inc)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and if geographic data we choose great circle distances in km].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +d to insert an extra output column with distances following the coordinates.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Instead of <start/stop>, give <origin> and append +a|o|l|n|r as required:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +a<az> defines a profiles from <origin> in <az> direction. Add +l<length>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +o<az> is like +a but centers profile on <origin>. Add +l<length>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +r<radius> defines a circle about <origin>. Add +i<inc> or +n<np>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +n<np> sets the number of output points and computes <inc> from <length>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do NOT skip points outside the grid domain [Default only returns points inside domain].\n");
	GMT_Option (API, "R,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-S In conjunction with -C, compute a single stacked profile from all profiles across each segment.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append which method should be used when performing the stacking:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a = mean, m = median, p = mode, l = lower, L = lower of +ve values, u = upper, U = upper of -ve values [a].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The modifiers control what is being written; choose one or more among\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +a : Append stacked values to all cross-profiles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +d : Append stack deviations to all cross-profiles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +r : Append data residuals (data - stack) to all cross-profiles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +s[<file>] : Save stacked profile to <file> [stacked_profile.txt].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +c<fact> : Compute envelope as +/- <fact>*deviation [2].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: Deviations depend on mode and are L1 scale (e), st.dev (a), LMS scale (p), or half-range (u-l)/2.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T If nearest node is NaN, search outwards to find the nearest non-NaN node and return it instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +e to append 3 extra columns: lon, lat of nearest node and its distance from original node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p to instead replace input lon, lat with that of nearest node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Can only be used with a single non-IMG grid and incompatible with -A, -C, -D, -E, -S.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Only output z-values [Default gives all columns].\n");
	GMT_Option (API, "a,bi2,bo,f,g,h,i,n,o,s,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_grdtrack_parse (struct GMT_CTRL *GMT, struct GRDTRACK_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdsample and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int j, mode;
	unsigned int pos, n_errors = 0, ng = 0, n_files = 0, n_units = 0, n_modes = 0;
	char line[GMT_BUFSIZ] = {""}, ta[GMT_LEN64] = {""}, tb[GMT_LEN64] = {""};
	char tc[GMT_LEN64] = {""}, p[GMT_LEN256] = {""}, *c = NULL, X;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)) n_errors++;
				break;
			case '>':	/* Specified output file */
				if (n_files++ == 0 && GMT_check_filearg (GMT, '>', opt->arg, GMT_OUT))
					Ctrl->Out.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Change track resampling mode */
				Ctrl->A.active = true;
				switch (opt->arg[0]) {
					case 'f': Ctrl->A.mode = GMT_TRACK_FILL;   break;
					case 'm': Ctrl->A.mode = GMT_TRACK_FILL_M; break;
					case 'p': Ctrl->A.mode = GMT_TRACK_FILL_P; break;
					case 'r': Ctrl->A.mode = GMT_TRACK_SAMPLE_FIX; break;
					case 'R': Ctrl->A.mode = GMT_TRACK_SAMPLE_ADJ; break;
					default: GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -G option: Bad modifier %c\n", opt->arg[0]); n_errors++; break;
				}
				if (strstr (opt->arg, "+l")) Ctrl->A.loxo = true;
				break;
			case 'C':	/* Create cross profiles */
				Ctrl->C.active = true;
				if ((c = strstr (opt->arg, "+a"))) {	/* Gave modifiers */
					Ctrl->C.alternate = true;	/* Select alternating direction of cross-profiles */
					*c = 0;	/* Truncate option at start of modifiers */
				}
				j = sscanf (opt->arg, "%[^/]/%[^/]/%s", ta, tb, tc);
				Ctrl->C.mode = GMT_get_distance (GMT, ta, &(Ctrl->C.length), &(Ctrl->C.unit));
				mode = GMT_get_distance (GMT, tb, &(Ctrl->C.ds), &X);
				if (X != Ctrl->C.unit) n_units++;
				if (mode != Ctrl->C.mode) n_modes++;
				if (j == 3) {
					mode = GMT_get_distance (GMT, tc, &(Ctrl->C.spacing), &X);
					if (X != Ctrl->C.unit) n_units++;
					if (mode != Ctrl->C.mode) n_modes++;
				}
				if (Ctrl->C.alternate) *c = '+';	/* Undo truncation */
				if (n_units) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Only <length> takes a unit which is shared with <ds> [and <spacing>]\n");
					n_errors++;
				}
				if (n_modes) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Cannot imply different distance modes for <length> and <ds> [and/or <spacing>]\n");
					n_errors++;
				}
				break;
			case 'D':	/* Dump resampled lines */
				Ctrl->D.active = true;
				Ctrl->D.file = strdup (opt->arg);
				break;
			case 'E':	/* Create input tracks instead of reading tracks */
				Ctrl->E.active = true;
				Ctrl->E.lines = strdup (opt->arg);
				break;
			case 'G':	/* Input grid file */
				if (ng == MAX_GRIDS) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -G option: Too many grids (max = %d)\n", MAX_GRIDS);
					n_errors++;
				}
				else {
					Ctrl->G.active = true;
					Ctrl->G.scale[ng] = 1.0;
					if (strchr (opt->arg, ',') && !strchr (opt->arg, '?')) {	/* IMG grid file with required parameters */
						if ((j = sscanf (opt->arg, "%[^,],%lf,%d,%lf", line, &Ctrl->G.scale[ng], &Ctrl->G.mode[ng], &Ctrl->G.lat[ng])) < 3) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -G option: Give imgfile, scale, mode [and optionally max_lat]\n");
							n_errors++;
						}
						else if (GMT_check_filearg (GMT, '<', line, GMT_IN))
							Ctrl->G.file[ng] = strdup (line);
						else
							n_errors++;
						Ctrl->G.type[ng] = 1;
						n_errors += GMT_check_condition (GMT, Ctrl->G.mode[ng] < 0 || Ctrl->G.mode[ng] > 3, "Syntax error -G: mode must be in 0-3 range\n");
						n_errors += GMT_check_condition (GMT, Ctrl->G.lat[ng] < 0.0, "Syntax error -G: max latitude should be positive\n");
					}
					else {	/* Regular grid file */
						if (GMT_check_filearg (GMT, '<', opt->arg, GMT_IN))
							Ctrl->G.file[ng] = strdup (opt->arg);
						else
							n_errors++;
					}
					ng++;
				}
				break;
			case 'L':	/* GMT4 Sets BCs */
				if (GMT_compat_check (GMT, 4)) {
					if (opt->arg[0]) {
						GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -L<flag> is deprecated; -n+b%s was set instead, use this in the future.\n", opt->arg);
						strncpy (GMT->common.n.BC, opt->arg, 4U);
					}
					else {
						GMT_set_geographic (GMT, GMT_IN);
						GMT_set_geographic (GMT, GMT_OUT);
						GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -L is deprecated; please use -fg instead.\n");
					}
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'N':
				Ctrl->N.active = true;
				break;
			case 'S':
				if (opt->arg[0] == 0 && GMT_compat_check (GMT, 4)) {	/* Under COMPAT: Interpret -S (no args) as old-style -S option to skip output with NaNs */
					GMT_Report (API, GMT_MSG_NORMAL, "Warning: Option -S deprecated. Use -sa instead.\n");
					GMT->current.setting.io_nan_mode = GMT_IO_NAN_ONE;
					break;
				}
				Ctrl->S.active = true;
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
						GMT_Report (API, GMT_MSG_NORMAL, "Error: Bad mode (%c) given to -S.\n", (int)opt->arg[0]);
						break;
				}
				pos = 0;
				while (GMT_strtok (&opt->arg[1], "+", &pos, p)) {
					switch (p[0]) {
						case 'a': Ctrl->S.selected[STACK_ADD_VAL] = true; break;	/* Gave +a to add stacked value to all output profiles */
						case 'd': Ctrl->S.selected[STACK_ADD_DEV] = true; break;	/* Gave +d to add stacked deviations to all output profiles */
						case 'r': Ctrl->S.selected[STACK_ADD_RES] = true; break;	/* Gave +r to add residual values (data - stack) to all output profiles */
						case 'c': Ctrl->S.factor = atof (&p[1]); break;			/* Gave +c to scale deviations for use in making envelopes [2] */
						case 's': Ctrl->S.selected[STACK_ADD_TBL] = true;		/* Gave +s to write stacked profile to given table */
							Ctrl->S.file = (p[1]) ? strdup (&p[1]) : strdup ("stacked_profile.txt");
							break;
						default:
							n_errors++; 
							GMT_Report (API, GMT_MSG_NORMAL, "Error: Bad modifier (%s) given to -S.\n", p[0]);
							break;
					}
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.unit = 'X';	/* Cartesian units unless override later */
				if ((c = strstr (opt->arg, "+p"))) {	/* Gave +p modifier */
					Ctrl->T.mode = 1;	/* Report coordinates of non-NaN node instead of input coordinates */
					*c = 0;			/* Truncate option at start of modifiers */
				}
				else if ((c = strstr (opt->arg, "+e"))) {	/* Gave +e modifier */
					Ctrl->T.mode = 2;	/* Report coordinates of non-NaN node and radius as extra columns */
					*c = 0;			/* Truncate option at start of modifiers */
				}
				if (opt->arg[0]) Ctrl->T.dmode = GMT_get_distance (GMT, opt->arg, &(Ctrl->T.radius), &(Ctrl->T.unit));
				break;
			case 'Z':
				Ctrl->Z.active = true;
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
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && (Ctrl->C.spacing < 0.0 || Ctrl->C.length < 0.0), "Syntax error -C: Arguments must be positive\n");
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && !(Ctrl->G.n_grids == 1 && Ctrl->G.type[0] == 0), "Syntax error -T: Only one non-img input grid can be specified\n");
	n_errors += GMT_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int sample_all_grids (struct GMT_CTRL *GMT, struct GRD_CONTAINER *GC, unsigned int n_grids, bool img, double x_in, double y_in, double value[])
{
	unsigned int g, n_in, n_set;
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

/* The following two scanners are used below in the gmt_grdspiral_search */

unsigned int scan_grd_row (struct GMT_CTRL *GMT, int64_t row, int64_t l_col, int64_t r_col, struct GMT_ZSEARCH *S)
{
	/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is returned, 0 if all NaN */
	unsigned int ret_code = 0;
	int64_t col, node;
	double r;
	struct GMT_GRID_HEADER *h = S->C->G->header;
	if (row < 0 || row >= h->ny) return 2;	/* Outside grid */
	if (l_col < 0) l_col = 0;	/* Start inside grid */
	if (r_col >= h->nx) r_col = h->nx - 1;	/* End inside grid */
	for (col = l_col; col <= r_col; col++) {	/* Search along this row */
		node = GMT_IJP (h, row, col);
		if (GMT_is_fnan (S->C->G->data[node])) continue;	/* A NaN node */
		r = GMT_distance (GMT, S->x0, S->y0, S->x[col], S->y[row]);
		if (r > S->max_radius) continue;	/* Basically not close enough */
		if (r < S->radius) {	/* Great, this one is closer */
			S->radius = r;
			S->row = row;
			S->col = col;
			ret_code = 1;
		}
	}
	return (ret_code);
}

unsigned int scan_grd_col (struct GMT_CTRL *GMT, int64_t col, int64_t t_row, int64_t b_row, struct GMT_ZSEARCH *S)
{
/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN or beyond max radius*/
	unsigned int ret_code = 0;
	int64_t row, node;
	double r;
	struct GMT_GRID_HEADER *h = S->C->G->header;
	if (col < 0 || col >= h->nx) return 2;	/* Outside grid */
	if (t_row < 0) t_row = 0;	/* Start inside grid */
	if (b_row >= h->ny) b_row = h->ny - 1;	/* End inside grid */
	for (row = t_row; row <= b_row; row++) {	/* Search along this column */
		node = GMT_IJP (h, row, col);
		if (GMT_is_fnan (S->C->G->data[node])) continue;	/* A NaN node */
		r = GMT_distance (GMT, S->x0, S->y0, S->x[col], S->y[row]);
		if (r > S->max_radius) continue;	/* Basically not close enough */
		if (r < S->radius) {	/* Great, this one is closer */
			S->radius = r;
			S->row = row;
			S->col = col;
			ret_code = 1;
		}
	}
	return (ret_code);
}

/* gmt_grdspiral_search is used when the nearest node to the given point
 * is NaN and no interpolation is possible.  With -T we will determine
 * the nearest node that is not NaN AND within a specific radius (if not
 * given then no limiting radius is used).
 * Given a grid and the original x,y point (which is near the NaN),
 * find nearest node that is not NaN.
 * We wish to search outwards from this node and keep track of non-
 * NaN nodes and return the value and location of the nearest one.
 * For Cartesian data this is guaranteed to work.  However, for geo-
 * graphic data at higher latitudes it is possible that we will miss
 * nodes lying at the same latitude but further away then examined by
 * the spiral search.  We therefore add a special check along that line
 * to see if any node along that line exist that is actually closer and
 * has a non-NaN value.
 * The -T is experimental: Contact P. Wessel for issues.
 */

unsigned int gmt_grdspiral_search (struct GMT_CTRL *GMT, struct GMT_ZSEARCH *S, double x, double y)
{
	unsigned int T, B, L, R;
	int64_t t_row, b_row, l_col, r_col, step = 0, col0, row0;
	bool done = false, found = false;
	
	/* We know we are inside the grid when this is called */
	
	col0 = GMT_grd_x_to_col (GMT, x, S->C->G->header);		/* Closest col to x in input grid */
	row0 = GMT_grd_y_to_row (GMT, y, S->C->G->header);		/* Closest row to y in input grid */
	S->x0 = x;	S->y0 = y;	/* This is our original point */
	S->radius = DBL_MAX;		/* Initialize to mean no node found */
	do {	/* Keep spiraling (via expanding squares) until done or found something */
		step++;	/* Step search outwards from central row0,col0 node */
		t_row = row0 - step;	b_row = row0 + step;	/* Next 2 rows to scan */
		l_col = col0 - step;	r_col = col0 + step;	/* Next 2 cols to scan */
		
		/* Each search is a square frame surrounding (row0, col0).  Since the
		 * nearest non-NaN node might be found anywhere along this frame we
		 * must search all the nodes along the 4 sides and then see which was
		 * the closest one.  Two situations can arise:
		 * 1) No non-NaN nodes found. Go to next frame unless
		 *    a) max_radius in this frame > radius and we just return NaN
		 *    b) If no radius limit we continue to go until we exceed grid borders
		 * 2) Found non-NaN node and return the one closest to (row0,col0). */
		T = scan_grd_row (GMT, t_row, l_col, r_col, S);	/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		B = scan_grd_row (GMT, b_row, l_col, r_col, S);	/* Look along this row, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		L = scan_grd_col (GMT, l_col, t_row, b_row, S);	/* Look along this col, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		R = scan_grd_col (GMT, r_col, t_row, b_row, S);	/* Look along this col, return 2 if ran out of row/col, 1 if nearest non-NaN is return, 0 if all NaN */
		done = ((T + B + L + R) == 8U);	/* Now completely outside grid on all sides */
		found = (T == 1 || B == 1 || L == 1 || R == 1);	/* Found a non-NaN and its distance from node */
	} while (!done && !found);
	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Must check along the row0 line in case there are closer nodes */
		int64_t C, d_col, col = (col0 == 0) ? 1 : col0 - 1;	/* Nearest neighbor in the same row */
		double dx = GMT_distance (GMT, S->x0, S->y0, S->x[col], S->y0);	/* Distance between x-nodes at current row in chosen units */
		if (found) /* Use smallest radius so far as max distance for searching along this row */
			d_col = lrint (ceil (S->radius / dx));
		else if (S->max_radius < DBL_MAX) /* Nothing was found so use radius limit as max distance for searching along this row */
			d_col = lrint (ceil (S->max_radius / dx));
		else	/* Must search the entire row */
			d_col = S->C->G->header->nx;	/* This is obviously too large but will get truncated later */
		l_col = col0 - d_col;	/* Go to both sides, scan_grd_row will truncate to 0,nx-1 anyway */
		r_col = col0 + d_col;
		C = scan_grd_row (GMT, row0, l_col, r_col, S);	/* C is nonzero if we found a non-NaN node */
		if (C) return (1);	/* Did pick up a closer node along this row */
	}
	return (found) ? 1 : 0;
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdtrack_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdtrack (void *V_API, int mode, void *args) {
	/* High-level function that implements the grdtrack task */

	int status, error, ks;
	uint64_t n_points = 0, n_read = 0;
	unsigned int g, k;
	bool img_conv_needed = false, some_outside = false;
	
	char line[GMT_BUFSIZ] = {""}, run_cmd[BUFSIZ] = {""}, *cmd = NULL;

	double *value, wesn[4];

	struct GRDTRACK_CTRL *Ctrl = NULL;
	struct GRD_CONTAINER *GC = NULL;
	struct GMT_DATASET *Din = NULL, *Dout = NULL;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdtrack_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdtrack_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdtrack_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdtrack_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdtrack_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdtrack main code ----------------------------*/

	cmd = GMT_Create_Cmd (API, options);
	sprintf (run_cmd, "# %s %s", GMT->init.module_name, cmd);	/* Build command line argument string */
	GMT_free (GMT, cmd);

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grid(s)\n");

	GMT_memset (wesn, 4, double);
	if (GMT->common.R.active) GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Specified a subset */
	GMT_set_pad (GMT, 2U);	/* Ensure space for BCs in case an API passed pad == 0 */

	GC = GMT_memory (GMT, NULL, Ctrl->G.n_grids, struct GRD_CONTAINER);
	value = GMT_memory (GMT, NULL, Ctrl->G.n_grids, double);
	
	for (g = 0; g < Ctrl->G.n_grids; g++) {
		GC[g].type = Ctrl->G.type[g];
		if (Ctrl->G.type[g] == 0) {	/* Regular GMT grids */
			if ((GC[g].G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->G.file[g], NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			if (GMT->common.R.active) GMT_err_fail (GMT, GMT_adjust_loose_wesn (GMT, wesn, GC[g].G->header), "");		/* Subset requested; make sure wesn matches header spacing */

			if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, GC[g].G->header->wesn, 4, double);

			if (!GMT_grd_setregion (GMT, GC[g].G->header, wesn, BCR_BILINEAR)) {
				GMT_Report (API, GMT_MSG_VERBOSE, "Warning: No data within specified region\n");
				Return (GMT_OK);
			}

			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, Ctrl->G.file[g], GC[g].G) == NULL) {	/* Get subset */
				Return (API->error);
			}

			GMT_memcpy (GMT->common.R.wesn, wesn, 4, double);

		}
		else {	/* Sandwell/Smith Mercator grids */
			if ((GC[g].G = GMT_create_grid (GMT)) == NULL) Return (API->error);
			GMT_read_img (GMT, Ctrl->G.file[g], GC[g].G, wesn, Ctrl->G.scale[g], Ctrl->G.mode[g], Ctrl->G.lat[g], true);
			img_conv_needed = true;
		}
	}
	
	if (Ctrl->E.active) {	/* Create profiles rather than read them */
		double xyz[2][3];
		uint64_t dim[4] = {1, 0, 0, 3};
		
		if ((Din = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);	/* An empty dataset with 1 table */
		GMT_free_table (GMT, Din->table[0], Din->alloc_mode);	/* Since we will add our own below */
		if (Ctrl->E.unit == 0) {	/* Was not set via -E; default to Cartesian or km (great circle dist) */
			Ctrl->E.unit = (GMT_is_geographic (GMT, GMT_IN)) ? 'k' : 'X';
			Ctrl->E.mode = (GMT_is_geographic (GMT, GMT_IN)) ? 2 : 0;
			/* Set default spacing to half the min grid spacing; convert to km if geographic */
			Ctrl->E.step = 0.5 * MIN (GC[0].G->header->inc[GMT_X], GC[0].G->header->inc[GMT_Y]);
			if (GMT_is_geographic (GMT, GMT_IN)) Ctrl->E.step *= GMT->current.proj.DIST_KM_PR_DEG;
		}
		GMT_init_distaz (GMT, Ctrl->E.unit, Ctrl->E.mode, GMT_MAP_DIST);
		if (Ctrl->G.n_grids == 1) {
			GMT_grd_minmax (GMT, GC[0].G, xyz);
			Din->table[0] = GMT_make_profile (GMT, 'E', Ctrl->E.lines, true, false, false, Ctrl->E.step, GMT_TRACK_FILL, xyz);
		}
		else
			Din->table[0] = GMT_make_profile (GMT, 'E', Ctrl->E.lines, true, false, false, Ctrl->E.step, GMT_TRACK_FILL, NULL);
		Din->n_columns = Din->table[0]->n_columns;	/* Since could have changed via +d */
	}
	
	if (Ctrl->C.active) {	/* Special case of requesting cross-profiles for given line segments */
		uint64_t tbl, col, row, seg, n_cols = Ctrl->G.n_grids;
		struct GMT_DATASET *Dtmp = NULL;
		struct GMT_DATASEGMENT *S = NULL;
		
		if (!GMT_is_geographic (GMT, GMT_IN) && Ctrl->A.loxo) {
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: Loxodrome mode ignored for Cartesian data.\n");
			Ctrl->A.loxo = false;
		}
		if (!Ctrl->E.active) {
			if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
				Return (API->error);
			}
			if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
				Return (API->error);
			}
			if (Ctrl->C.mode == GMT_GEODESIC) {
				GMT_Report (API, GMT_MSG_NORMAL, "Warning: Cannot use geodesic distances as path interpolation is spherical; changed to spherical\n");
				Ctrl->C.mode = GMT_GREATCIRCLE;
			}
			if (Ctrl->A.loxo) GMT->current.map.loxodrome = true, Ctrl->C.mode = 1 + GMT_LOXODROME;
			GMT_init_distaz (GMT, Ctrl->C.unit, Ctrl->C.mode, GMT_MAP_DIST);
		}

		/* Expand with dist,az columns (mode = 2) (and posibly make space for more) and optionally resample */
		if ((Dtmp = GMT_resample_data (GMT, Din, Ctrl->C.spacing, 2, (Ctrl->D.active) ? Ctrl->G.n_grids : 0, Ctrl->A.mode)) == NULL) Return (API->error);
		if (GMT_Destroy_Data (API, &Din) != GMT_OK) {
			Return (API->error);
		}
		if (Ctrl->D.active) {	/* Also want to sample grids along the original resampled trace */
			for (tbl = 0; tbl < Dtmp->n_tables; tbl++) {
				T = Dtmp->table[tbl];
				for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment to resample */
					S = T->segment[seg];
					for (row = 0; row < S->n_rows; row++) {	/* For each row to resample */
						status = sample_all_grids (GMT, GC, Ctrl->G.n_grids, img_conv_needed, S->coord[GMT_X][row], S->coord[GMT_Y][row], value);
						if (status < 0) some_outside = true;
						for (col = 4, k = 0; k < Ctrl->G.n_grids; k++, col++) S->coord[col][row] = value[k];
					}
				}
			}
			if (some_outside) GMT_Report (API, GMT_MSG_VERBOSE, "Some points along your lines were outside the grid domain(s).\n");
			
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
		if ((Dout = GMT_crosstracks (GMT, Dtmp, Ctrl->C.length, Ctrl->C.ds, n_cols, Ctrl->C.alternate)) == NULL) Return (API->error);
#if 0
		if (Ctrl->D.active) {
			if (GMT_Destroy_Data (API, &Dtmp) != GMT_OK) {
				Return (API->error);
			}
		}
		else	/* Never written */
#endif
			GMT_free_dataset (GMT, &Dtmp);
		
		/* Sample the grids along all profiles in Dout */
		
		some_outside = false;
		for (tbl = 0; tbl < Dout->n_tables; tbl++) {
			T = Dout->table[tbl];
			for (seg = 0; seg < T->n_segments; seg++) {	/* For each segment to resample */
				S = T->segment[seg];
				for (row = 0; row < S->n_rows; row++) {	/* For each row to resample */
					status = sample_all_grids (GMT, GC, Ctrl->G.n_grids, img_conv_needed, S->coord[GMT_X][row], S->coord[GMT_Y][row], value);
					for (col = 4, k = 0; k < Ctrl->G.n_grids; k++, col++) S->coord[col][row] = value[k];
					if (status < 0)
						some_outside = true;
					else
						n_points++;
				}
			}
		}
		if (some_outside) GMT_Report (API, GMT_MSG_VERBOSE, "Some points along your profiles were outside the grid domain(s).\n");
		if (Dout->n_segments > 1) GMT_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
		
		if (Ctrl->S.active) {	/* Compute the stacked profiles */
			struct GMT_DATASET *Stack = NULL;
			struct GMT_DATASEGMENT *M = NULL;
			uint64_t dim[4], n_rows;
			uint64_t colx, col0 = 4 + Ctrl->G.n_grids;		/* First column for stacked value in cross-profiles */
			unsigned int n_step = (Ctrl->S.mode < STACK_LOWER) ? 6 : 4;	/* Number of columns per gridded data in stack file */
			unsigned int GMT_mode_selection = 0, GMT_n_multiples = 0;
			double **stack = NULL, *stacked_val = NULL, *stacked_dev = NULL, *stacked_hi = NULL, *stacked_lo = NULL, *dev = NULL;
			dim[GMT_TBL] = 1;			/* One table */
			dim[GMT_SEG] = Dout->n_tables;		/* Number of stacks */
			dim[GMT_COL] = 1 + n_step * Ctrl->G.n_grids;	/* Number of columns needed in stack file */
			dim[GMT_ROW] = n_rows = Dout->table[0]->segment[0]->n_rows;	/* Number of rows */
			if ((Stack = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);	/* An empty table for stacked results */
			
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
		
		T = Dout->table[0];
		T->n_headers = 2;
		T->header = GMT_memory (GMT, NULL, T->n_headers, char *);
		T->header[0] = strdup ("# Equidistant cross-profiles normal to each input segment");
		T->header[1] = strdup (run_cmd);
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, Dout->io_mode, NULL, Ctrl->Out.file, Dout) != GMT_OK) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &Dout) != GMT_OK) {
			Return (API->error);
		}
	}
	else if (Ctrl->E.active) {	/* Quick sampling along given lines */
		int status;
		unsigned int k;
		uint64_t col, n_cols = Din->n_columns + Ctrl->G.n_grids, row, seg;
		struct GMT_DATASEGMENT *Sin = NULL, *Sout = NULL;
		
		Din->dim[GMT_COL] = n_cols;	/* State we want a different set of columns on output */
		Dout = GMT_Duplicate_Data (API, GMT_IS_DATASET, GMT_DUPLICATE_ALLOC, Din);	/* Same table length as Din, but with up to n_cols columns (lon, lat, dist, g1, g2, ...) */
		if (Din->table[0]->n_segments > 1) GMT_set_segmentheader (GMT, GMT_OUT, true);	/* More than one segment triggers -mo */
		
		for (seg = 0; seg < Din->table[0]->n_segments; seg++) {	/* For each segment to resample */
			Sin  = Din->table[0]->segment[seg];	/* Shorthand */
			Sout = Dout->table[0]->segment[seg];	/* Shorthand */
			for (col = 0; col < Din->n_columns; col++) GMT_memcpy (Sout->coord[col], Sin->coord[col], Sin->n_rows, double);
			for (row = 0; row < Sin->n_rows; row++) {	/* For each row  */
				status = sample_all_grids (GMT, GC, Ctrl->G.n_grids, img_conv_needed, Sin->coord[GMT_X][row], Sin->coord[GMT_Y][row], value);
				if (status < 0) some_outside = true;
				for (col = Din->n_columns, k = 0; k < Ctrl->G.n_grids; k++, col++) Sout->coord[col][row] = value[k];
			}
		}
		if (some_outside) GMT_Report (API, GMT_MSG_VERBOSE, "Some points along your profiles were outside the grid domain(s).\n");
		T = Dout->table[0];
		T->n_headers = 2;
		T->header = GMT_memory (GMT, NULL, T->n_headers, char *);
		T->header[0] = strdup ("# Sampled values along specified profiles");
		T->header[1] = strdup (run_cmd);
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, Dout->io_mode, NULL, Ctrl->Out.file, Dout) != GMT_OK) {
			Return (API->error);
		}
	}
	else {	/* Standard resampling point case */
		bool pure_ascii = false;
		int ix, iy, n_fields, rmode;
		uint64_t n_out = 0;
		double *in = NULL, *out = NULL;
		char record[GMT_BUFSIZ];
		bool gmt_skip_output (struct GMT_CTRL *C, double *cols, int n_cols);
		
		pure_ascii = GMT_is_ascii_record (GMT);

		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
			Return (API->error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN,  GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}

		GMT_memset (line, GMT_BUFSIZ, char);
		if (Ctrl->Z.active) {
			GMT_set_cartesian (GMT, GMT_OUT);	/* Since we are outputting z all columns */
			n_out = Ctrl->G.n_grids;
			if ((error = GMT_set_cols (GMT, GMT_OUT, n_out))) Return (error);
		}
		ix = (GMT->current.setting.io_lonlat_toggle[GMT_IN]);	iy = 1 - ix;
		rmode = (pure_ascii && GMT_get_cols (GMT, GMT_IN) >= 2) ? GMT_READ_MIXED : GMT_READ_DOUBLE;

		if (Ctrl->T.active) {	/* Want to find nearest non-NaN if the node we find is NaN */
			Ctrl->T.S = GMT_memory (GMT, NULL, 1, struct GMT_ZSEARCH);
			Ctrl->T.S->C = &GC[0];	/* Since we know there is only one grid */
			GMT_init_distaz (GMT, Ctrl->T.unit, Ctrl->T.dmode, GMT_MAP_DIST);
			Ctrl->T.S->x = GMT_grd_coord (GMT, GC[0].G->header, GMT_X);
			Ctrl->T.S->y = GMT_grd_coord (GMT, GC[0].G->header, GMT_Y);
			Ctrl->T.S->max_radius = (Ctrl->T.radius == 0.0) ? DBL_MAX : Ctrl->T.radius;
		}

		do {	/* Keep returning records until we reach EOF */
			if ((in = GMT_Get_Record (API, rmode, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
				if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (GMT_REC_IS_TABLE_HEADER (GMT)) {	/* Echo table headers */
					GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, NULL);
					continue;
				}
				if (GMT_REC_IS_SEGMENT_HEADER (GMT)) {			/* Echo segment headers */
					GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
					continue;
				}
				if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
					break;
			}

			/* Data record to process */
			if (n_out == 0) {
				n_out = GMT_get_cols (GMT, GMT_IN) + Ctrl->G.n_grids;	/* Get new # of output cols */
				if (Ctrl->T.mode == 2) n_out += 3;
				if ((error = GMT_set_cols (GMT, GMT_OUT, n_out))) Return (error);
			}
			n_read++;

			status = sample_all_grids (GMT, GC, Ctrl->G.n_grids, img_conv_needed, in[GMT_X], in[GMT_Y], value);
			if (status == -1) {	/* Point is outside the region of all grids */
				some_outside = true;
				if (!Ctrl->N.active) continue;
			}
			else if (Ctrl->T.active && status == 0) {	/* Found a NaN; need to search for nearest non-NaN node */
				if (gmt_grdspiral_search (GMT, Ctrl->T.S, in[GMT_X], in[GMT_Y])) {	/* Did find a valid node */
					uint64_t ij = GMT_IJP (GC[0].G->header, Ctrl->T.S->row, Ctrl->T.S->col);
					value[0] = GC[0].G->data[ij];
					if (Ctrl->T.mode == 1) {	/* Replace input coordinate with node coordinate */
						in[ix] = Ctrl->T.S->x[Ctrl->T.S->col];
						in[iy] = Ctrl->T.S->y[Ctrl->T.S->row];
					}
				}
			}

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
				if (Ctrl->T.mode == 2) {	/* Add extra columns */
					GMT_add_to_record (GMT, record, Ctrl->T.S->x[Ctrl->T.S->col], GMT_X, 1);	/* Format our output x value */
					GMT_add_to_record (GMT, record, Ctrl->T.S->y[Ctrl->T.S->row], GMT_Y, 1);	/* Format our output y value */
					GMT_add_to_record (GMT, record, Ctrl->T.S->radius, GMT_Z, 1);			/* Format our radius */
				}
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write this to output */
			}
			else {	/* Simply copy other columns, append value, and output */
				if (!out) out = GMT_memory (GMT, NULL, n_out, double);
				for (ks = 0; ks < n_fields; ks++) out[ks] = in[ks];
				for (g = 0; g < Ctrl->G.n_grids; g++, ks++) out[ks] = value[g];
				if (Ctrl->T.mode == 2) {	/* Add extra columns */
					out[ks++] = Ctrl->T.S->x[Ctrl->T.S->col];	/* Add our output x value */
					out[ks++] = Ctrl->T.S->y[Ctrl->T.S->row];	/* Add our output y value */
					out[ks++] = Ctrl->T.S->radius;			/* Add our radius */
				}
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
			}

			n_points++;
		} while (true);
		
		if (GMT_End_IO (API, GMT_IN,  0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}

		if (out) GMT_free (GMT, out);
	}
	if (some_outside) GMT_Report (API, GMT_MSG_VERBOSE, "Some input points were outside the grid domain(s).\n");
	/* Clean up */
	for (g = 0; g < Ctrl->G.n_grids; g++) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Sampled %" PRIu64 " points from grid %s (%d x %d)\n",
			n_points, Ctrl->G.file[g], GC[g].G->header->nx, GC[g].G->header->ny);
		if (Ctrl->G.type[g] == 0 && GMT_Destroy_Data (API, &GC[g].G) != GMT_OK) {
			Return (API->error);
		}
		else
			GMT_free_grid (GMT, &GC[g].G, true);
	}
	GMT_free (GMT, value);
	GMT_free (GMT, GC);
	GMT_set_pad (GMT, API->pad);	/* Reset to session default pad */

	Return (EXIT_SUCCESS);
}
