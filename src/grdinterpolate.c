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
 * Brief synopsis: grdinterpolate reads a 3D netcdf spatial data cube with
 * the 3rd dimension either depth/height or time.  It then interpolates
 * the cube at arbitrary z (or time) values and writes either a single
 * slice 2-D grid or another 3-D data cube (via ncecat).  Alternatively,
 * we can read a stack of input 2-D grids instead of the 3D cube.
 *
 * Author:	Paul Wessel
 * Date:	1-AUG-2019
 * Version:	6 API
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdinterpolate"
#define THIS_MODULE_MODERN_NAME	"grdinterpolate"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Interpolate new layers from a 3-D netCDF data cube"
#define THIS_MODULE_KEYS	"<G{+,GG}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"->RVf"

struct GRDINTERPOLATE_CTRL {
	struct In {
		bool active;
		char **file;
		unsigned int n_files;
	} In;
	struct F {	/* -Fl|a|c[1|2] */
		bool active;
		unsigned int mode;
		unsigned int type;
		char spline[GMT_LEN8];
	} F;
	struct G {	/* -G<output_grdfile>  */
		bool active;
		char *file;
	} G;
	struct S {	/* -S<x>/<y>|<pointfile>[+d<template>][+h<header>] */
		bool active;
		unsigned int mode;
		double x, y;
		char *file;
		char *header;
		char *template;
	} S;
	struct T {	/* -T<start>/<stop>/<inc> or -T<value> */
		bool active;
		struct GMT_ARRAY T;
		char *string;
	} T;
	struct Z {	/* -Z<min>/<max>/<inc>, -Z<file>, or -Z<list> */
		bool active[2];
		struct GMT_ARRAY T;
	} Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	static char type[3] = {'l', 'a', 'c'};
	struct GRDINTERPOLATE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDINTERPOLATE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	sprintf (C->F.spline, "%c", type[GMT->current.setting.interpolant]);	/* Set default interpolant */
	return (C);
}

GMT_LOCAL void free_files (struct GMT_CTRL *GMT, char ***list, unsigned int n) {
	for (unsigned int k = 0; k < n; k++)
		gmt_M_str_free ((*list)[k]);
	gmt_M_free (GMT, *list);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDINTERPOLATE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	free_files (GMT, &(C->In.file), C->In.n_files);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->S.file);
	gmt_M_str_free (C->S.header);
	gmt_M_str_free (C->S.template);
	gmt_M_str_free (C->T.string);
	gmt_free_array (GMT, &(C->T.T));
	gmt_free_array (GMT, &(C->Z.T));
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	static char type[3] = {'l', 'a', 'c'};
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <3Dgrid> | <grd1> <grd2> ... -G<outfile> -T[<min>/<max>/]<inc>[+n]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Fl|a|c|n][+1|2] [-S<x>/<y>|<ptfile>[+d<template>][+h[<header>]] [%s]\n", GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-Zi<levels>|o] [%s]\n\n", GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<3Dgrid> is the name of the input 3D netCDF data cube.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   However, with -Zi we instead expect a series of 2-D grids.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify output grid file name (or template; see -Zo).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Interpolate the 3-D grid at given levels across the 3rd dimension\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Make evenly spaced output time steps from <min> to <max> by <inc>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n to indicate <inc> is the number of knot-values to produce over the range instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, give a file with output knots in the first column, or a comma-separated list.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Set the grid interpolation mode.  Choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   l Linear interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a Akima spline interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c Cubic spline interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n No interpolation (nearest point).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append +1 for 1st derivative or +2 for 2nd derivative.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is -F%c].\n", type[API->GMT->current.setting.interpolant]);
	GMT_Message (API, GMT_TIME_NONE, "\t-S Give a fixed point for time or vertical sampling [Default] or interpolation [-T]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For multiple points, give a <ptfile> with one point per record.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Output is a multi-segment file to stdout unless -G is used to give a file name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To write each series to separate files, let <outfile> contain a.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   C-format floating integer statement (e.g, %%d) for a running point number.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Add a fixed header via +h<header> or use the trailing text of <ptfile> via +h.\n");
	GMT_Option (API, "R,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Read or write 2-D grids that make up a virtual 3-D data cube.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To read a series of input 2-D grids, give -Zi<levels>, where <levels>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   for each grid is set via min/max/inc, <zfile>, or a comma-separated list.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To write a series of output 2-D grids, give -Zo and include a floating-point\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   C-format statement in <outfile> given via -G for embedding time in the file name.\n");
	GMT_Option (API, ".");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDINTERPOLATE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, n_alloc = 0, mode = 0;
	char *c = NULL, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))) {
					if (n_alloc <= Ctrl->In.n_files) {
						n_alloc += GMT_SMALL_CHUNK;
						Ctrl->In.file = gmt_M_memory (GMT, Ctrl->In.file, n_alloc, char *);
					}
					Ctrl->In.file[Ctrl->In.n_files++] = strdup (opt->arg);
				}
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'F':
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'l':
						Ctrl->F.mode = GMT_SPLINE_LINEAR;
						break;
					case 'a':
						Ctrl->F.mode = GMT_SPLINE_AKIMA;
						break;
					case 'c':
						Ctrl->F.mode = GMT_SPLINE_CUBIC;
						break;
					case 'n':
						Ctrl->F.mode = GMT_SPLINE_NN;
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -F: Bad spline selector %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				if (opt->arg[1] == '+') Ctrl->F.type = (opt->arg[2] - '0');	/* Want first or second derivatives */
				strcpy (Ctrl->F.spline, opt->arg);	/* Keep track of what was given */
				break;
			case 'G':	/* Output file */
				if (n_files++ > 0) break;
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.string = strdup (opt->arg);
				n_errors += gmt_parse_array (GMT, 'T', opt->arg, &(Ctrl->T.T), GMT_ARRAY_TIME | GMT_ARRAY_SCALAR | GMT_ARRAY_RANGE, GMT_Z);
				break;

			case 'S':
				Ctrl->S.active = true;
				if ((c = gmt_first_modifier (GMT, opt->arg, "dh")) != NULL) {	/* Process modifiers */
					unsigned int pos = 0;
					while (gmt_getmodopt (GMT, 'S', c, "dh", &pos, txt_a, &n_errors) && n_errors == 0) {
						switch (txt_a[0]) {
							case 'd': Ctrl->S.template = strdup (&txt_a[1]); break; /* Format for multiple output files */
							case 'h':
								if (txt_a[1])
									Ctrl->S.header = strdup (&txt_a[1]);
								else	/* Read headers from the point file */
									Ctrl->S.mode = 1;
								break;
							default:
								n_errors++;
								break;
						}
					}
					c[0] = '\0';	/* Chop off all modifiers */
				}
				if (strchr (opt->arg, '/')) {	/* Got a singple point */
					if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) != 2) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -S: Cannot parse point %s\n", opt->arg);
						n_errors++;
					}
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X),
						gmt_scanf_arg (GMT, txt_a, gmt_M_type (GMT, GMT_IN, GMT_X), false, &Ctrl->S.x), txt_a);
					n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y),
						gmt_scanf_arg (GMT, txt_b, gmt_M_type (GMT, GMT_IN, GMT_Y), false, &Ctrl->S.y), txt_b);
				}
				else if (!gmt_check_filearg (GMT, 'S', opt->arg, GMT_IN, GMT_IS_DATASET)) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -S: Cannot parse name or find file %s\n", opt->arg);
					n_errors++;
				}
				else	/* File exist */
					Ctrl->S.file = strdup (opt->arg);
				if (c) c[0] = '+';	/* Restore */
				break;

			case 'Z':
				switch (opt->arg[0]) {
					case 'i': mode = GMT_IN; break;
					case 'o': mode = GMT_OUT; break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -Z: Must be -Zi<levels> or -Zo\n");
						n_errors++;
					break;
				}
				Ctrl->Z.active[mode] = true;
				if (mode == GMT_IN)
					n_errors += gmt_parse_array (GMT, 'Z', &opt->arg[1], &(Ctrl->Z.T), GMT_ARRAY_TIME | GMT_ARRAY_SCALAR | GMT_ARRAY_RANGE, GMT_Z);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->In.n_files < 1, "Error: No input grid(s) specified.\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->Z.active[GMT_IN] && Ctrl->In.n_files != 1, "Must specify one input 3D grid cube file unless -Zi is set\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.type > 2, "Option -F: Only 1st or 2nd derivatives may be requested\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active && !Ctrl->S.active, "Option -T: Must specify output knot(s)\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file && !Ctrl->S.active, "Option -G: Must specify output grid file\n");
	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Must specify only one output grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->S.mode == 1 && Ctrl->S.file == NULL, "Option -S: Modifer +h with no arguments requires a point file.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdinterpolate (void *V_API, int mode, void *args) {
	char file[PATH_MAX] = {""};
	int error = 0;
	unsigned int int_mode, row, col;
	uint64_t n_layers = 0, k, node, start_k, stop_k, n_use;
	double wesn[4], *level = NULL, *i_value = NULL, *o_value = NULL;
	struct GMT_GRID **G[2] = {NULL, NULL};
	struct GRDINTERPOLATE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdinterpolate main code ----------------------------*/

	if (Ctrl->Z.active[GMT_IN]) {
		/* Create input level array */
		if (gmt_create_array (GMT, 'Z', &(Ctrl->Z.T), NULL, NULL)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to set up input level array\n");
			Return (API->error);
		}
		if (Ctrl->In.n_files != Ctrl->Z.T.n) {
			GMT_Report (API, GMT_MSG_ERROR, "Number of input 2-D grids does not match number of levels given via -Zi\n");
			Return (API->error);
		}
		n_layers = Ctrl->Z.T.n;
		level = Ctrl->Z.T.array;
	}
	else {	/* See if we got a 3D netCDF data cube; if so return number of layers and their levels */
		if ((error = gmt_examine_nc_cube (GMT, Ctrl->In.file[0], &n_layers, &level))) Return (error);
	}

	if (n_layers == 1) {
		GMT_Report (API, GMT_MSG_ERROR, "Only one layer given - need at least two to interpolate\n");
		Return (GMT_RUNTIME_ERROR);
	}

	/* Create output level array */
	if (Ctrl->T.active && gmt_create_array (GMT, 'T', &(Ctrl->T.T), NULL, NULL)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to set up output level array\n");
		Return (API->error);
	}

	/* Determine the range of input layers needed for interpolation */

	start_k = 0; stop_k = n_layers - 1;	/* We first assume all layers are needed */
	if (Ctrl->T.active) {
		while (start_k < n_layers && Ctrl->T.T.array[0] > level[start_k])	/* Find the first that is inside the output time range */
			start_k++;
		if (start_k && Ctrl->T.T.array[0] < level[start_k]) start_k--;		/* Go back one if start time is less than first layer */
		if (start_k && (Ctrl->F.mode == GMT_SPLINE_AKIMA || Ctrl->F.mode == GMT_SPLINE_CUBIC))
			start_k--;	/* One more to define the spline coefficients */
		while (stop_k && Ctrl->T.T.array[Ctrl->T.T.n-1] < level[stop_k])	/* Find the last that is inside the output time range */
			stop_k--;
		if (stop_k < n_layers && Ctrl->T.T.array[Ctrl->T.T.n-1] > level[stop_k]) stop_k++;	/* Go forward one if stop time is larger than last layer */
		if (stop_k < (n_layers-1) && (Ctrl->F.mode == GMT_SPLINE_AKIMA || Ctrl->F.mode == GMT_SPLINE_CUBIC))
			stop_k++;	/* One more to define the spline coefficients */
	}
	n_use = stop_k - start_k + 1;	/* Total number of input layers needed */
	if (n_use == 1) {	/* Might have landed exactly on one of the grid levels, but GMT_intpol needs at least 2 inputs */
		if (start_k) start_k--;
		else stop_k++;	/* We know there are at least 2 input grids */
		n_use = 2;
	}

	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */

	int_mode = Ctrl->F.mode + 10*Ctrl->F.type;

	if (Ctrl->S.active) {	/* Create time/depth-series and not grid output */
		unsigned int io_mode = GMT_WRITE_NORMAL;
		uint64_t rec, seg, row, col;
		uint64_t dim[4] = {1, 1, 1, 2};	/* Dataset dimension for one point */
		char i_file[GMT_STR16] = {""}, o_file[GMT_STR16] = {""}, grid[GMT_LEN128] = {""}, header[GMT_LEN256] = {""}, cmd[GMT_LEN128] = {""};
		struct GMT_DATASET *In = NULL, *Out = NULL, *D = NULL;
		struct GMT_DATASEGMENT *S = NULL, *Si = NULL, *So = NULL;

		if (Ctrl->S.file) {	/* Read a list of points, the list may have trailing text which may be activated by -S...+h */
			if ((In = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->S.file, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open or read file %s.\n", Ctrl->S.file);
				Return (GMT_RUNTIME_ERROR);
			}
		}
		else {	/* Single point, simplify code by making a 1-point dataset */
			if ((In = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POINT, (Ctrl->S.header) ? GMT_WITH_STRINGS : 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open or read file %s.\n", Ctrl->S.file);
				Return (GMT_RUNTIME_ERROR);
			}
			Si = In->table[0]->segment[0];	/* Short hand to the first and only segment */
			Si->data[GMT_X][0] = Ctrl->S.x;
			Si->data[GMT_Y][0] = Ctrl->S.y;
			if (Ctrl->S.header) Si->header = strdup (Ctrl->S.header);
			Si->n_rows = 1;
			gmt_set_dataset_minmax (GMT, In);
		}
		dim[GMT_SEG] = In->n_records;		/* One output time-series per input data location */
		dim[GMT_ROW] = n_layers;			/* Length of each sampled time-series per input data location */
		dim[GMT_COL] = In->n_columns + 2;	/* x, y [,col3, col4...], time|z, value */
		if ((Out = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_WITH_STRINGS, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create output dataset for time-series\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, In, i_file) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual dataset for time-series\n");
			Return (GMT_RUNTIME_ERROR);
		}

		for (k = start_k; k <= stop_k; k++) {	/* For all available input levels k */
			GMT_Init_VirtualFile (API, 0, i_file);	/* Reset so it can be read again */
			if (Ctrl->Z.active[GMT_IN])	/* Get the k'th file */
				sprintf (grid, "%s", Ctrl->In.file[k]);
			else	/* Get the k'th layer from 3D cube */
				sprintf (grid, "%s?[%" PRIu64 "]", Ctrl->In.file[0], k);
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, NULL, o_file) == GMT_NOTSET) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual dataset for time-series\n");
				Return (GMT_RUNTIME_ERROR);
			}

			sprintf (cmd, "%s -G%s ->%s", i_file, grid, o_file);
			GMT_Report (API, GMT_MSG_DEBUG, "Sampling the grid: %s\n", cmd);
			if (GMT_Call_Module (API, "grdtrack", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Sample this layer at the points */
				Return (API->error);
			}
			D = GMT_Read_VirtualFile (API, o_file);	/* Get the results from grdtrack */

			for (seg = rec = 0; seg < D->table[0]->n_segments; seg++) {	/* For each point we sampled at */
				S = D->table[0]->segment[seg];	/* Short hand to this segment */


				for (row = 0; row < S->n_rows; row++, rec++) {	/* For each selected point which matches each output segment */
					So = Out->table[0]->segment[rec];	/* Short hand to this output segment */
					if (k == start_k) {	/* Set the segment header just once */
						if (S->text && S->text[row])
							sprintf (header, "Location %g,%g %s", S->data[GMT_X][row], S->data[GMT_Y][row], S->text[row]);
						else
							sprintf (header, "Location %g,%g", S->data[GMT_X][row], S->data[GMT_Y][row]);
						So->header = strdup (header);
					}
					for (col = 0; col < S->n_columns; col++)
						So->data[col][k] = S->data[col][row];
					So->data[col][k] = level[k];	/* Add time as the last data column */
				}
			}
			if (GMT_Close_VirtualFile (API, i_file) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to close input virtual dataset for time-series\n");
				Return (GMT_RUNTIME_ERROR);
			}
			if (GMT_Close_VirtualFile (API, o_file) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to close output irtual dataset for time-series\n");
				Return (GMT_RUNTIME_ERROR);
			}
			if (GMT_Destroy_Data (API, &D) != GMT_OK) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to delete dataset returned by grdtrack\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
		if (Ctrl->T.active) {	/* Want to interpolate through the sampled points using the specified spline */
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, Out, i_file) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual dataset for sampled time-series\n");
				Return (GMT_RUNTIME_ERROR);
			}
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, NULL, o_file) == GMT_NOTSET) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual dataset for sampled time-series\n");
				Return (GMT_RUNTIME_ERROR);
			}
			sprintf (cmd, "%s -F%s -N%d -T%s ->%s", i_file, Ctrl->F.spline, (int)(Out->n_columns - 1), Ctrl->T.string, o_file);
			if (GMT_Call_Module (API, "sample1d", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Interpolate each profile per -T */
				Return (API->error);
			}
			if (GMT_Destroy_Data (API, &Out) != GMT_OK) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to delete virtual dataset for time-series\n");
				Return (GMT_RUNTIME_ERROR);
			}
			Out = GMT_Read_VirtualFile (API, o_file);	/* Get the updated results from sample1d */
		}

		if (Ctrl->G.file && strchr (Ctrl->G.file, '%')) {	/* Want separate files per series, so change mode and build file names per segment */
			struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
			io_mode = GMT_WRITE_SEGMENT;
			for (seg = 0; seg < Out->table[0]->n_segments; seg++) {
				SH = gmt_get_DS_hidden (Out->table[0]->segment[seg]);
				sprintf (file, Ctrl->G.file, seg);
				SH->file[GMT_OUT] = strdup (file);
			}
		}
		/* Time to write out the results to stdout, a file, or individual segment files */
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, io_mode, NULL, Ctrl->G.file, Out) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to write dataset for time-series\n");
			Return (GMT_RUNTIME_ERROR);
		}

		Return (GMT_NOERROR);
	}

	if ((G[GMT_IN] = gmt_M_memory (GMT, NULL, n_layers, struct GMT_GRID *)) == NULL) Return (GMT_MEMORY_ERROR);	/* Allocate one grid per input layer */

	for (k = start_k; k <= stop_k; k++) {	/* Read the required layers into individual grid structures */
		if (Ctrl->Z.active[GMT_IN])	/* Get the k'th file */
			sprintf (file, "%s", Ctrl->In.file[k]);
		else	/* Get the k'th layer from 3D cube */
			sprintf (file, "%s?[%" PRIu64 "]", Ctrl->In.file[0], k);
		if ((G[GMT_IN][k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, wesn, file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to read layer %" PRIu64 " from file %s.\n", k, file);
			Return (API->error);
		}
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Will read %" PRIu64 " layers (%" PRIu64 " - %" PRIu64 ") for levels %g to %g.\n", n_use, start_k, stop_k, level[start_k], level[stop_k]);

	if (Ctrl->T.T.n > 1 && !Ctrl->Z.active[GMT_OUT]) {
		GMT_Report (API, GMT_MSG_ERROR, "Sorry, writing 3-D output netCDF cube is not implemented yet.  Use -Zo for now.\n");
		Return (GMT_MEMORY_ERROR);
	}
	GMT_Report (API, GMT_MSG_INFORMATION, "Interpolate %" PRIu64 " new layers (%g to %g in steps of %g).\n", Ctrl->T.T.n, Ctrl->T.T.array[0], Ctrl->T.T.array[Ctrl->T.T.n-1]);

	gmt_set_column (GMT, GMT_OUT, GMT_Z, GMT_IS_FLOAT);	/* The 3-rd dimension is not time in the grids, but we may have read time via -Z with -f2T */

	/* Create grid layers for each output level */

	if ((G[GMT_OUT] = gmt_M_memory (GMT, NULL, Ctrl->T.T.n, struct GMT_GRID *)) == NULL) Return (GMT_MEMORY_ERROR);	/* Allocate on grid per output layer */
	for (k = 0; k < Ctrl->T.T.n; k++)	{	/* Duplicate grid headers and allocate arrays */
		if ((G[GMT_OUT][k] = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, G[GMT_IN][start_k])) == NULL)
			Return (API->error);
	}

	/* Allocate input and output arrays for the 1-D spline */
	if ((i_value = gmt_M_memory (GMT, NULL, n_layers, double)) == NULL) Return (GMT_MEMORY_ERROR);
	if ((o_value = gmt_M_memory (GMT, NULL, Ctrl->T.T.n, double)) == NULL) Return (GMT_MEMORY_ERROR);

	gmt_M_grd_loop (GMT, G[GMT_IN][start_k], row, col, node) {	/* Loop over all coregistered nodes (picking G[GMT_IN][start_k] to represent all grid layouts) */
		for (k = start_k; k <= stop_k; k++)	/* For all available input levels */
			i_value[k] = G[GMT_IN][k]->data[node];	/* Get the values at this (x,y) across all input levels */
		gmt_intpol (GMT, &level[start_k], &i_value[start_k], n_use, Ctrl->T.T.n, Ctrl->T.T.array, o_value, int_mode);	/* Resample at requested output levels */
		for (k = 0; k < Ctrl->T.T.n; k++)	/* For all output levels */
			G[GMT_OUT][k]->data[node] = (float)o_value[k];	/* Put interpolated output values at this (x,y) across all levels */
	}

	if (Ctrl->T.T.n == 1 || Ctrl->Z.active[GMT_OUT]) {	/* Special case of only sampling the cube at one layer or asking for 2-D slices via -Zo */
		for (k = 0; k < Ctrl->T.T.n; k++) {	/* For all output levels */
			if (Ctrl->Z.active[GMT_OUT])	/* Create the k'th layer file */
				sprintf (file, Ctrl->G.file, Ctrl->T.T.array[k]);
			else	/* Just this one layer grid */
				sprintf (file, "%s", Ctrl->G.file);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, G[GMT_OUT][k]) != GMT_NOERROR) {
				Return (API->error);
			}
		}
	}

	/* Here we must write an output 3-D data cube - not implemented yet - but this case is caught earlier */

	/* Done with everything; free up memory */

	if (!Ctrl->Z.active[GMT_IN])
		gmt_M_free (GMT, level);
	gmt_M_free (GMT, i_value);
	gmt_M_free (GMT, o_value);
	for (k = 0; k < n_layers; k++)
		GMT_Destroy_Data (API, &(G[GMT_IN][k]));
	gmt_M_free (GMT, G[GMT_IN]);
	for (k = 0; k < Ctrl->T.T.n; k++)
		GMT_Destroy_Data (API, &(G[GMT_OUT][k]));
	gmt_M_free (GMT, G[GMT_OUT]);

	Return (GMT_NOERROR);
}
