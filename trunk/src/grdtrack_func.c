/*--------------------------------------------------------------------
 *	$Id: grdtrack_func.c,v 1.6 2011-04-17 23:53:25 guru Exp $
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
 * Brief synopsis: grdtrack reads a data table, opens the gridded file, 
 * and samples the dataset at the xy positions with a bilinear or bicubic
 * interpolant.
 *
 * Author:	Paul Wessel and Walter H F Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#include "gmt.h"

#define MAX_GRIDS BUFSIZ	/* Change and recompile if we need to sample more than BUFSIZ grids */

struct GRD_CONTAINER {	/* Keep all the grid and sample parameters together */
	struct GMT_BCR bcr;
	struct GMT_GRID *G;
	struct GMT_EDGEINFO edgeinfo;
	GMT_LONG type;
};

struct GRDTRACK_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct Out {	/* -> */
		GMT_LONG active;
		char *file;
	} Out;
	struct A {	/* -A[m|p] */
		GMT_LONG active;
		int mode;
	} A;
	struct C {	/* -C<length>/<ds>[/<spacing>] */
		GMT_LONG active;
		GMT_LONG mode;
		char unit;
		double ds, spacing, length;
	} C;
	struct D {	/* -Dresampfile */
		GMT_LONG active;
		char *file;
	} D;
	struct G {	/* -G<grdfile> */
		GMT_LONG active;
		GMT_LONG n_grids;
		char *file[MAX_GRIDS];
		double scale[MAX_GRIDS], lat[MAX_GRIDS];
		GMT_LONG mode[MAX_GRIDS];
		GMT_LONG type[MAX_GRIDS];	/*HIDDEN */
	} G;
	struct L {	/* -L<flag> */
		GMT_LONG active;
		char mode[4];
	} L;
	struct Q {	/* -Q[b|c|l|n][[/]<threshold>] */
		GMT_LONG active;
		GMT_LONG interpolant;
		double threshold;
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct Z {	/* -Z */
		GMT_LONG active;
	} Z;
};

void *New_grdtrack_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDTRACK_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDTRACK_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->Q.interpolant = BCR_BICUBIC; C->Q.threshold = 1.0;
	return ((void *)C);
}

void Free_grdtrack_Ctrl (struct GMT_CTRL *GMT, struct GRDTRACK_CTRL *C) {	/* Deallocate control structure */
	GMT_LONG g;
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);
	if (C->D.file) free ((void *)C->D.file);
	for (g = 0; g < C->G.n_grids; g++) if (C->G.file[g]) free ((void *)C->G.file[g]);	
	if (C->Out.file) free ((void *)C->Out.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdtrack_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdtrack %s [API] - Sampling of 2-D gridded data set(s) along 1-D trackline\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdtrack <xyfile> -G<grd1> -G<grd2> ... [-A[m|p]] [-C<length>[u]/<ds>[u][/<spacing>[u]]]\n"); 
	GMT_message (GMT, "\t[-D<dfile>] [-L<flag>] [-Q[b|c|l|n][[/]<threshold>]] [%s] [-S] [%s] [-Z] [%s]\n\t[%s] [%s] [%s] [%s] [%s] [%s]\n",
		GMT_Rgeo_OPT, GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<xyfile> is an multicolumn ASCII file with (x, y) in the first two columns\n");
	GMT_message (GMT, "\t-G Sets the name of a more 2-D binary data set to sample.\n");
	GMT_message (GMT, "\t   If the file is a Sandwell/Smith Mercator grid (IMG format) instead,\n");
	GMT_message (GMT, "\t   append comma-separated scale (0.1 or 1), mode, and optionally max latitude [%g].  Modes are\n", GMT_IMG_MAXLAT_80);
	GMT_message (GMT, "\t     0 = img file w/ no constraint code, interpolate to get data at track.\n");
	GMT_message (GMT, "\t     1 = img file w/ constraints coded, interpolate to get data at track.\n");
	GMT_message (GMT, "\t     2 = img file w/ constraints coded, gets data only at constrained points, NaN elsewhere.\n");
	GMT_message (GMT, "\t     3 = img file w/ constraints coded, gets 1 at constraints, 0 elsewhere.\n");
	GMT_message (GMT, "\t   For mode 2|3 you may want to consider the -Q<threshold> setting.\n");
	GMT_message (GMT, "\t   Repeat -G for as many grids as you wish to sample.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A For spherical surface sampling we follow great circle paths.\n");
	GMT_message (GMT, "\t   Append m or p to first follow meridian then parallel, or vice versa.\n");
	GMT_message (GMT, "\t   Ignored unless -C is used.\n");
	GMT_message (GMT, "\t-C Create equidistant cross-profiles from input line segments.  Append\n");
	GMT_message (GMT, "\t   <length>: Full-length of each cross profile.  Append desired distance unit [%s]\n", GMT_LEN_UNITS);
	GMT_message (GMT, "\t   <dz>: Distance interval along the cross-profiles.\n");
	GMT_message (GMT, "\t   Optionally, append /<spacing> to set the spacing between cross-profiles [Default use input locations].\n");
	GMT_message (GMT, "\t   Output columns are x, y, dist, z1, z2, ...\n");
	GMT_message (GMT, "\t   Default samples the grid(s) at the input data points.\n");
	GMT_message (GMT, "\t-D Save [resampled] input lines to a separate file <dfile>.  Requires -C.\n");
	GMT_message (GMT, "\t   Output columns are lon, lat, dist, az, z1, z2, ...\n");
	GMT_message (GMT, "\t-L Sets boundary conditions.  <flag> can be either\n");
	GMT_message (GMT, "\t   g for geographic boundary conditions\n");
	GMT_message (GMT, "\t   or one or both of\n");
	GMT_message (GMT, "\t   x for periodic boundary conditions on x\n");
	GMT_message (GMT, "\t   y for periodic boundary conditions on y\n");
	GMT_message (GMT, "\t   [Default is natural conditions for regular grids and geographic for IMG grids]\n");
	GMT_sample_syntax (GMT, 'Q', "Determines the grid interpolation mode.");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-S Suppress output when any grid sample equals NaN\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-Z Only output z-values [Default gives all columns]\n");
	GMT_explain_options (GMT, "C2D0fghio:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdtrack_parse (struct GMTAPI_CTRL *C, struct GRDTRACK_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdsample and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j, n_errors = 0, ng = 0, n_files = 0;
	char line[BUFSIZ], ta[GMT_TEXT_LEN], tb[GMT_TEXT_LEN], tc[GMT_TEXT_LEN];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	GMT_memset (line, BUFSIZ, char);

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
						if ((j = sscanf (opt->arg, "%[^,],%lf,%" GMT_LL "d,%lf", line, &Ctrl->G.scale[ng], &Ctrl->G.mode[ng], &Ctrl->G.lat[ng])) < 3) {
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
					Ctrl->L.active = TRUE;
					strncpy (Ctrl->L.mode, opt->arg, (size_t)4);
#ifdef GMT_COMPAT
				}
				else {
					GMT->current.io.col_type[GMT_IN][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
					GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
					GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -L is obsolete (but is processed correctly).  Please use -f instead.\n");
				}
#endif
				break;
#ifdef GMT_COMPAT
			case 'N':	/* Backwards compatible */
				Ctrl->Q.interpolant = BCR_NEARNEIGHBOR;
				GMT_report (GMT, GMT_MSG_FATAL, "Warning: Option -N deprecated. Use -Qn instead.\n");
				break;
#endif
			case 'Q':	/* Interpolation mode */
				Ctrl->Q.active = TRUE;
				Ctrl->Q.interpolant = BCR_BILINEAR;
				for (j = 0; j < 3 && opt->arg[j]; j++) {
					switch (opt->arg[j]) {
						case 'n':
							Ctrl->Q.interpolant = BCR_NEARNEIGHBOR; break;
						case 'l':
							Ctrl->Q.interpolant = BCR_BILINEAR; break;
						case 'b':
							Ctrl->Q.interpolant = BCR_BSPLINE; break;
						case 'c':
							Ctrl->Q.interpolant = BCR_BICUBIC; break;
						case '/':
						default:
							Ctrl->Q.threshold = atof (&opt->arg[j]);
							if (j == 0 && Ctrl->Q.threshold < GMT_SMALL) {
								Ctrl->Q.interpolant = BCR_NEARNEIGHBOR;
								GMT_report (GMT, GMT_MSG_FATAL, "Warning: Option -Q0 deprecated. Use -Qn instead.\n");
							}
							j = 3; break;
					}
				}
				break;
			case 'S':
				Ctrl->S.active = TRUE;
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
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && !Ctrl->D.file, "Syntax error -D: Must specify file name.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.n_grids == 0, "Syntax error: Must specify -G at least once\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && (Ctrl->C.spacing < 0.0 || Ctrl->C.ds < 0.0 || Ctrl->C.length < 0.0), "Syntax error -C: Arguments must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && (Ctrl->Q.threshold < 0.0 || Ctrl->Q.threshold > 1.0), "Syntax error -Q: Threshold must be in [0,1] range\n");
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += GMT_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LONG sample_all_grids (struct GMT_CTRL *GMT, struct GRD_CONTAINER *GC, GMT_LONG n_grids, GMT_LONG img, double x_in, double y_in, double value[])
{
	GMT_LONG g, n_in, n_set;
	double x, y, x0 = 0.0, y0 = 0.0;
	
	if (img) GMT_geo_to_xy (GMT, x_in, y_in, &x0, &y0);	/* At least one Mercator IMG grid in use - get Mercator coordinates x,y */

	for (g = n_in = n_set = 0; g < n_grids; g++) {
		y = (GC[g].type == 1) ? y0 : y_in;
		value[g] = GMT->session.d_NaN;	/* In case the point is outside only some of the grids */
		/* If point is outside grd area, shift it using periodicity or skip if not periodic. */

		while ((y < GC[g].G->header->wesn[YLO]) && (GC[g].edgeinfo.nyp > 0)) y += (GC[g].G->header->inc[GMT_Y] * GC[g].edgeinfo.nyp);
		if (y < GC[g].G->header->wesn[YLO]) continue;

		while ((y > GC[g].G->header->wesn[YHI]) && (GC[g].edgeinfo.nyp > 0)) y -= (GC[g].G->header->inc[GMT_Y] * GC[g].edgeinfo.nyp);
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
		while ((x < GC[g].G->header->wesn[XLO]) && (GC[g].edgeinfo.nxp > 0)) x += (GC[g].G->header->inc[GMT_X] * GC[g].edgeinfo.nxp);
		if (x < GC[g].G->header->wesn[XLO]) continue;

		while ((x > GC[g].G->header->wesn[XHI]) && (GC[g].edgeinfo.nxp > 0)) x -= (GC[g].G->header->inc[GMT_X] * GC[g].edgeinfo.nxp);
		if (x > GC[g].G->header->wesn[XHI]) continue;

		n_in++;	/* This point is inside the current grid's domain */
		value[g] = GMT_get_bcr_z (GMT, GC[g].G, x, y, &GC[g].edgeinfo, &GC[g].bcr);

		if (!GMT_is_dnan (value[g])) n_set++;	/* Count value results */
	}
	
	return ((n_in == 0) ? -1 : n_set);
}

#define Return(code) {Free_grdtrack_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdtrack (struct GMTAPI_CTRL *API, struct GMT_OPTION *options) {
	/* High-level function that implements the grdtrack task */

	GMT_LONG status, error, n_points = 0, n_read = 0, g, k, img_conv_needed = FALSE;
	
	char line[BUFSIZ];

	double *value, wesn[4];

	struct GRDTRACK_CTRL *Ctrl = NULL;
	struct GRD_CONTAINER *GC = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdtrack_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdtrack_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdtrack", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRbf:", "ghios>" GMT_OPT("HMm"), options))) Return (error);
	Ctrl = (struct GRDTRACK_CTRL *) New_grdtrack_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdtrack_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdtrack main code ----------------------------*/

	GMT_memset (wesn, 4, double);
	if (GMT->common.R.active) GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Specified a subset */

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */

	GC = GMT_memory (GMT, NULL, Ctrl->G.n_grids, struct GRD_CONTAINER);
	value = GMT_memory (GMT, NULL, Ctrl->G.n_grids, double);
	
	for (g = 0; g < Ctrl->G.n_grids; g++) {
		GMT_boundcond_init (GMT, &GC[g].edgeinfo);
		if (Ctrl->L.active) GMT_boundcond_parse (GMT, &GC[g].edgeinfo, Ctrl->L.mode);
		if (GC[g].edgeinfo.gn) {
			GMT->current.io.col_type[GMT_IN][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
			GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
		}
		GC[g].type = Ctrl->G.type[g];
		if (Ctrl->G.type[g] == 0) {	/* Regular GMT grids */
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->G.file[g]), (void **)&GC[g].G)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
			if (GMT->common.R.active) GMT_err_fail (GMT, GMT_adjust_loose_wesn (GMT, wesn, GC[g].G->header), "");		/* Subset requested; make sure wesn matches header spacing */

			if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, GC[g].G->header->wesn, 4, double);

			if (GMT_grd_setregion (GMT, GC[g].G->header, wesn, BCR_BILINEAR)) {
				GMT_report (GMT, GMT_MSG_NORMAL, "Warning: No data within specified region\n");
				GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&GC[g].G);
				Return (GMT_OK);
			}

			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->G.file[g]), (void **)&GC[g].G)) Return (GMT_DATA_READ_ERROR);	/* Get subset */

			GMT_memcpy (GMT->common.R.wesn, wesn, 4, double);

		}
		else {	/* Sandwell/Smith Mercator grids */
			GC[g].G = GMT_create_grid (GMT);
			GMT_read_img (GMT, Ctrl->G.file[g], GC[g].G, wesn, Ctrl->G.scale[g], Ctrl->G.mode[g], Ctrl->G.lat[g], TRUE);
			if (GMT_360_RANGE (GC[g].G->header->wesn[XHI], GC[g].G->header->wesn[XLO])) GMT_boundcond_parse (GMT, &GC[g].edgeinfo, "g");
			GMT_boundcond_parse (GMT, &GC[g].edgeinfo, "g");
			img_conv_needed = TRUE;
		}
		GMT_boundcond_param_prep (GMT, GC[g].G->header, &GC[g].edgeinfo);

		/* Initialize bcr structure */

		GMT_bcr_init (GMT, GC[g].G, Ctrl->Q.interpolant, Ctrl->Q.threshold, &GC[g].bcr);

		/* Set boundary conditions  */

		GMT_boundcond_set (GMT, GC[g].G, &GC[g].edgeinfo);

	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
	
	if (Ctrl->C.active) {	/* Special case of requesting cross-profiles for given line segments */
		GMT_LONG tbl, seg, row, col;
		struct GMT_DATASET *Din = NULL, *Dout = NULL, *Dtmp = NULL;
		struct GMT_TABLE *T = NULL;
		struct GMT_LINE_SEGMENT *S = NULL;
		
		if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);				/* Enables data input and sets access mode */
		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, (void **)&Din)) Return ((error = GMT_DATA_READ_ERROR));
		if ((error = GMT_End_IO (API, GMT_IN,  0))) Return (error);	/* Disables further data input */

		GMT_init_distaz (GMT, Ctrl->C.unit, Ctrl->C.mode, GMT_MAP_DIST);
		/* Expand with dist,az columns (and posibly make space for more) and optionally resample */
		GMT_resample_data (GMT, Din, Ctrl->C.spacing, 2, (Ctrl->D.active) ? Ctrl->G.n_grids : 0, Ctrl->A.mode, &Dtmp);
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
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
			if ((error = GMT_Put_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, NULL, 0, (void **)&Ctrl->D.file, (void *)Dtmp))) Return (error);
		}
		/* Get dataset with cross-profiles, with columns for x,y,d and the n_grids samples */
		GMT_crosstracks (GMT, Dtmp, Ctrl->C.length, Ctrl->C.ds, Ctrl->G.n_grids, &Dout);
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Dtmp);
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Din);
		
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
		if ((error = GMT_Put_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, NULL, Dout->io_mode, (void **)&Ctrl->Out.file, (void *)Dout))) Return (error);
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */		
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Dout);
	}
	else {	/* Standard resampling point case */
		GMT_LONG pure_ascii = FALSE, ix, iy, n_fields;
		double *in = NULL, *out = NULL;
		
		pure_ascii = !(GMT->common.b.active[GMT_IN] || GMT->common.b.active[GMT_OUT] || GMT->common.o.active);

		if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
		if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data output */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN,  GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);	/* Enables data output and sets access mode */

		GMT_memset (line, BUFSIZ, char);
		if (Ctrl->Z.active) {
			GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_FLOAT;	/* Since we are outputting z all columns */
			GMT->common.b.ncol[GMT_OUT] = Ctrl->G.n_grids;
		}
		ix = (GMT->current.setting.io_lonlat_toggle[GMT_IN]);	iy = 1 - ix;

		while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we reach EOF */

			if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);	/* Bail on any i/o error */
			if (GMT_REC_IS_TBL_HEADER (GMT)) continue;		/* Skip any table headers */
		
			if (GMT_REC_IS_SEG_HEADER (GMT)) {			/* Echo segment headers */
				GMT_Put_Record (API, GMT_WRITE_SEGHEADER, NULL);
				continue;
			}

			if (GMT->common.b.ncol[GMT_OUT] == 0) GMT->common.b.ncol[GMT_OUT] = GMT->common.b.ncol[GMT_IN] + Ctrl->G.n_grids;	/* Set # of output cols */
			n_read++;

			status = sample_all_grids (GMT, GC, Ctrl->G.n_grids, img_conv_needed, in[GMT_X], in[GMT_Y], value);
			if (status == -1) continue;					/* Point is outside the region of all grids */
			if (Ctrl->S.active && status < Ctrl->G.n_grids) continue;	/* One or more sampled values are NaN and -S is active */

			if (Ctrl->Z.active)	/* Simply print out values */
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)value);
			else if (pure_ascii && n_fields > 2) {
				/* Special case: Ascii i/o and at least 3 columns:
				   Columns beyond first two could be text strings */

				/* First get rid of any commas that may cause grief */
				for (k = 0; GMT->current.io.current_record[k]; k++) if (GMT->current.io.current_record[k] == ',') GMT->current.io.current_record[k] = ' ';
				sscanf (GMT->current.io.current_record, "%*s %*s %[^\n]", line);
				GMT_ascii_output_one (GMT, GMT->session.std[GMT_OUT], in[ix], ix);	GMT_fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
				GMT_ascii_output_one (GMT, GMT->session.std[GMT_OUT], in[iy], iy);	GMT_fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
				GMT_fprintf (GMT->session.std[GMT_OUT], "%s", line);
				for (g = 0; g < Ctrl->G.n_grids; g++) {
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
					GMT_ascii_output_one (GMT, GMT->session.std[GMT_OUT], value[g], 2);
				}
				GMT_fprintf (GMT->session.std[GMT_OUT], "\n");
			}
			else {	/* Simply copy other columns, append value, and output */
				if (!out) out = GMT_memory (GMT, NULL, GMT->common.b.ncol[GMT_OUT], double);
				for (k = 0; k < n_fields; k++) out[k] = in[k];
				for (g = 0; g < Ctrl->G.n_grids; g++, k++) out[k] = value[g];
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)out);
			}

			n_points++;
		}
		if ((error = GMT_End_IO (API, GMT_IN,  0))) Return (error);	/* Disables further data input */
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

		if (out) GMT_free (GMT, out);
	}
	/* Clean up */
	for (g = 0; g < Ctrl->G.n_grids; g++) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Sampled %ld points from grid %s (%d x %d)\n",
			n_points, Ctrl->G.file[g], GC[g].G->header->nx, GC[g].G->header->ny);
		if (Ctrl->G.type[g] == 0)
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&GC[g].G);
		else
			GMT_free_grid (GMT, &GC[g].G, TRUE);
	}
	GMT_free (GMT, value);
	GMT_free (GMT, GC);

	Return (EXIT_SUCCESS);
}
