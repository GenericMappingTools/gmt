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
 * we can read a stack of input 2-D grids instead of the 3D cube.  Finally,
 * we may sample time-series ratther than write gridded output.
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
#define THIS_MODULE_PURPOSE	"Interpolate 2-D grids or 1-D series from a 3-D data cube"
#define THIS_MODULE_KEYS	"<G{+,>?}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"->RVfn"

struct GRDINTERPOLATE_CTRL {
	struct In {
		bool active;
		char **file;
		unsigned int n_files;
	} In;
	struct E {	/* -E<line1>[,<line2>,...][+a<az>][+c][+g][+i<step>][+l<length>][+n<np][+o<az>][+p][+r<radius>][+x] */
		bool active;
		unsigned int mode;
		char *lines;
		double step;
		char unit;
	} E;
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
	struct S {	/* -S<x>/<y>|<pointfile>[+h<header>] */
		bool active;
		double x, y;
		char *file;
		char *header;
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
	GMT_Message (API, GMT_TIME_NONE, "\t[-E<line1>[,<line2>,...][+a<az>][+g][+i<step>][+l<length>][+n<np][+o<az>][+p][+r<radius>][+x]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-Fl|a|c|n][+1|2] [-S<x>/<y>|<table>[+h<header>]] [%s]\n", GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-Zi<levels>|o] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_n_OPT, GMT_o_OPT, GMT_q_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<3Dgrid> is the name of the input 3D netCDF data cube.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   However, with -Zi we instead expect a series of 2-D grids.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify output file name (or template; see -Zo and -S).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Interpolate the 3-D grid at given levels across the 3rd dimension\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Make evenly spaced output time steps from <min> to <max> by <inc>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n to indicate <inc> is the number of levels to produce over the range instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, give a file with output levels in the first column, or a comma-separated list.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set up a crossection based on the given <line1>[,<line2>,...]. Give start and stop coordinates for\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   each line segment.  The format of each <line> is <start>/<stop>, where <start> or <stop>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   are coordinate pairs, e.g., <lon1/lat1>/<lon2>/<lat2>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +i<inc> to set the sampling increment [Default is 0.5 x min of grid's (x_inc, y_inc)]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +d to insert an extra output column with distances following the coordinates.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Instead of <start/stop>, give <origin> and append +a|o|l|n|r as required:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +a<az> defines a profiles from <origin> in <az> direction. Add +l<length>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +g uses gridline coordinates (degree longitude or latitude) if <line> is so aligned [great circle].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +o<az> is like +a but centers profile on <origin>. Add +l<length>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +p means sample along the parallel if <line> has constant latitude.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +r<radius> defines a circle about <origin>. Add +i<inc> or +n<np>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +n<np> sets the number of output points and computes <inc> from <length>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +x follows a loxodrome (rhumbline) [great circle].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Note:  A unit is optional.  Only ONE unit type from %s can be used throughout this option,\n", GMT_LEN_UNITS2_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t     so mixing of units is not allowed [Default unit is km, if geographic].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Set the grid interpolation mode.  Choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   l Linear interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a Akima spline interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c Cubic spline interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n No interpolation (nearest point).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append +1 for 1st derivative or +2 for 2nd derivative.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is -F%c].\n", type[API->GMT->current.setting.interpolant]);
	GMT_Message (API, GMT_TIME_NONE, "\t-S Give a fixed point for across-stack sampling [Default] or interpolation [-T].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For multiple points, give a <table> of points instead (one point per record).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Output is a multi-segment table written to stdout unless -G is used to set a file name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To write each series to separate files, let -G<outfile> contain a C-format\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   integer specifier (e.g, %%d) for embedding the running point number.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append a fixed header via +h<header> [trailing text per record in <table>].\n");
	GMT_Option (API, "R,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Read or write 2-D grids that make up a virtual 3-D data cube.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To read a series of input 2-D grids, give -Zi<levels>, where <levels>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   for each grid is set via min/max/inc, <zfile>, or a comma-separated list.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To write a series of output 2-D grids, give -Zo and include a floating-point\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   C-format statement in <outfile> given via -G for embedding time in the file name.\n");
	GMT_Option (API, "a,bi2,bo,d,e,f,g,h,i,n,o,q,s,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDINTERPOLATE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, n_alloc = 0, mode = 0;
	char *c = NULL;
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

			case 'E':	/* Create an equidistant profile for slicing */
				Ctrl->E.active = true;
				Ctrl->E.lines = strdup (opt->arg);
				break;
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
				if ((c = strstr (opt->arg, "+h"))) {	/* Got a fixed header string for output segment headers */
					if (c[2])
						Ctrl->S.header = strdup (&c[2]);
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -S: Modifier +h not given a header string\n");
						n_errors++;
					}
					c[0] = '\0';	/* Chop off all modifiers */
				}
				if (strchr (opt->arg, '/')) {	/* Got a single point */
					char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""};
					if (sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b) != 2) {
						GMT_Report (API, GMT_MSG_ERROR, "Option -S: Cannot parse point coordinates from %s\n", opt->arg);
						n_errors++;
					}
					else {	/* OK to convert to numbers */
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X),
							gmt_scanf_arg (GMT, txt_a, gmt_M_type (GMT, GMT_IN, GMT_X), false, &Ctrl->S.x), txt_a);
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y),
							gmt_scanf_arg (GMT, txt_b, gmt_M_type (GMT, GMT_IN, GMT_Y), false, &Ctrl->S.y), txt_b);
					}
				}
				else if (!gmt_check_filearg (GMT, 'S', opt->arg, GMT_IN, GMT_IS_DATASET)) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -S: Cannot parse the name or find point file %s\n", opt->arg);
					n_errors++;
				}
				else	/* File exist */
					Ctrl->S.file = strdup (opt->arg);
				if (c) c[0] = '+';	/* Restore modifiers */
				break;

			case 'Z':
				switch (opt->arg[0]) {
					case 'i': mode = GMT_IN; break;
					case 'o': mode = GMT_OUT; break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -Z: Expected -Zi<levels> or -Zo\n");
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
	if (!(Ctrl->S.active || Ctrl->E.active)) {	/* Under -S, the -T and -G are optional */
		n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active, "Option -T: Must specify output levels(s)\n");
		n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Option -G: Must specify output grid file\n");
		n_errors += gmt_M_check_condition (GMT, n_files != 1, "Must specify only one output file name\n");
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->S.active, "Options -E and -S cannot be used together\n");
	if (Ctrl->E.active) {
		n_errors += gmt_M_check_condition (GMT, strstr (Ctrl->E.lines, "+d"), "Option -E: Unrecognized modifier +d\n");
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL bool equidistant_levels (struct GMT_CTRL *GMT, double *z, unsigned int nz) {
	/* Return true if spacing between layers is constant */
	unsigned int k;
	double dz;

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {	/* Report levels */
		for (k = 0; k < nz; k++)
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Level %d: %g\n", k, z[k]);
	}
	if (nz < 3) return true;	/* Special case of a single layer */
	dz = z[1] - z[0];	/* First increment */
	for (k = 2; k < nz; k++)
		if (!doubleAlmostEqual (dz, z[k]-z[k-1])) return false;
	return true;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdinterpolate (void *V_API, int mode, void *args) {
	char file[PATH_MAX] = {""}, cube_layer[GMT_LEN64] = {""}, *nc_layer = NULL;
	bool equi_levels;
	int error = 0;
	unsigned int int_mode, row, col, level_type, dtype;
	uint64_t n_layers = 0, k, node, start_k, stop_k, n_layers_used;
	double wesn[4], *level = NULL, *i_value = NULL, *o_value = NULL;
	struct GMT_GRID **G[2] = {NULL, NULL},*Grid = NULL;
	struct GMT_DATASET *In = NULL, *Out = NULL;
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

	if (Ctrl->Z.active[GMT_IN]) {	/* Create the input level array */
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
		nc_layer = strchr (Ctrl->In.file[0], '?');	/* Maybe given a specific variable? */
		if (nc_layer) {	/* Gave a specific layer. Keep variable name and remove from filename */
			strcpy (cube_layer, &nc_layer[1]);
			nc_layer[0] = '\0';	/* Chop off layer name for now */
		}
		if ((error = gmt_examine_nc_cube (GMT, Ctrl->In.file[0], &n_layers, &level))) Return (error);
	}

	if (n_layers == 1) {
		GMT_Report (API, GMT_MSG_ERROR, "Only one layer given - need at least two to interpolate\n");
		Return (GMT_RUNTIME_ERROR);
	}

	/* Create output level array, if selected */
	if (Ctrl->T.active && gmt_create_array (GMT, 'T', &(Ctrl->T.T), NULL, NULL)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to set up output level array\n");
		Return (API->error);
	}
	equi_levels = equidistant_levels (GMT, level, n_layers);	/* Are levels equidistant? */
	level_type = gmt_M_type (GMT, GMT_IN, GMT_Z);	/* Either time or floating point values like depth */

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
	n_layers_used = stop_k - start_k + 1;	/* Total number of input layers needed */
	if (n_layers_used == 1) {	/* Might have landed exactly on one of the grid levels, but GMT_intpol needs at least 2 inputs */
		if (start_k) start_k--;
		else stop_k++;	/* We know there are at least 2 input grids */
		n_layers_used = 2;
	}

	if (Ctrl->E.active) {	/* Create profiles rather than read them */
		char prof_args[GMT_LEN128] = {""};
		if (!(equi_levels || Ctrl->T.active)) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -E requires either equidistant levels or resampling via -T\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (gmt_get_dist_units (GMT, Ctrl->E.lines, &Ctrl->E.unit, &Ctrl->E.mode)) {	/* Bad mixing of units in -E specification */
			GMT_Report (API, GMT_MSG_ERROR, "Cannot mix units in -E (%s)\n", Ctrl->E.lines);
			Return (GMT_RUNTIME_ERROR);
		}
		gmt_init_distaz (GMT, Ctrl->E.unit, Ctrl->E.mode, GMT_MAP_DIST);	/* Initialize the distance unit and scaling */

		/* Need to get dx,dy from one grid */
		if (Ctrl->Z.active[GMT_IN])	/* Get the first file */
			sprintf (file, "%s", Ctrl->In.file[0]);
		else	/* Get the first layer from 3D cube possibly via a selected variable */
			sprintf (file, "%s?%s[0]", Ctrl->In.file[0], cube_layer);
		if ((Grid = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to read header from file %s.\n", file);
			Return (API->error);
		}
		/* Set default spacing to half the min grid spacing: */
		Ctrl->E.step = 0.5 * MIN (Grid->header->inc[GMT_X], Grid->header->inc[GMT_Y]);
		if (GMT_Destroy_Data (API, &Grid)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to destroy temporary grid?\n");
			Return (API->error);
		}
		if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Convert to km if geographic or degrees if arc-units */
			if (!GMT->current.map.dist[GMT_MAP_DIST].arc) Ctrl->E.step *= GMT->current.proj.DIST_M_PR_DEG;	/* Convert from degrees to meters or from min/secs to degrees */
			Ctrl->E.step *= GMT->current.map.dist[GMT_MAP_DIST].scale;	/* Scale to chosen unit */
			GMT_Report (API, GMT_MSG_INFORMATION, "Default sampling interval in -E is %g %c (may be overridden by -E modifiers).\n", Ctrl->E.step, Ctrl->E.unit);
		}
		if (strstr (Ctrl->E.lines, "+c"))
			strncpy (prof_args, Ctrl->E.lines, GMT_LEN128-1);
		else /* Make sure we request continuous if possible */
			sprintf (prof_args, "%s+c", Ctrl->E.lines);
		In = gmt_make_profiles (GMT, 'E', prof_args, true, false, true, Ctrl->E.step, 0, NULL, &dtype);
		if (In->table[0] == NULL)
			Return (GMT_RUNTIME_ERROR);
		In->n_columns = In->table[0]->n_columns;	/* Since could have changed via +d */
	}
	else if (Ctrl->S.active) {	/* Create time/depth-series and not grid output */
		/* Since we let grdtrack read the grids we do a separate branch here and the return from the module */
		uint64_t seg, row;
		uint64_t dim[4] = {1, 1, 1, 2};	/* Dataset dimension for one point */
		char header[GMT_LEN256] = {""};
		struct GMT_DATASEGMENT *Si = NULL;

		if (Ctrl->S.file) {	/* Read a list of points, the list may have trailing text which may be activated by -S...+h */
			if ((In = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->S.file, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open or read file %s.\n", Ctrl->S.file);
				Return (API->error);
			}
			if (Ctrl->S.header) {	/* Want to use this fixed text to add to the trialing text of all the points */
				for (seg = 0; seg < In->n_segments; seg++) {
					Si = In->table[0]->segment[seg];	/* Short hand to this segment */
					if (Si->text == NULL)	/* Input file did not have any trailing text so add array now */
						Si->text = gmt_M_memory (GMT, NULL, Si->n_rows, char *);
					for (row = 0; row < Si->n_rows; row++) {
						if (Si->text[row]) {	/* Already has trialing text, combine with user argument */
							sprintf (header, "%s %s", Si->text[row], Ctrl->S.header);
							gmt_M_str_free (Si->text[row]);
							Si->text[row] = strdup (header);
						}
						else	/* Just use user argument */
							Si->text[row] = strdup (Ctrl->S.header);
					}
				}
			}
		}
		else {	/* Single point, simplify logic below by making a 1-point dataset */
			if ((In = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POINT, (Ctrl->S.header) ? GMT_WITH_STRINGS : 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open or read file %s.\n", Ctrl->S.file);
				Return (API->error);
			}
			Si = In->table[0]->segment[0];	/* Short hand to the first and only segment */
			Si->data[GMT_X][0] = Ctrl->S.x;
			Si->data[GMT_Y][0] = Ctrl->S.y;
			if (Ctrl->S.header) Si->header = strdup (Ctrl->S.header);	/* Set the fixed header here */
			Si->n_rows = 1;
			gmt_set_dataset_minmax (GMT, In);
		}
	}
	if (Ctrl->E.active || Ctrl->S.active) {
		unsigned int io_mode = GMT_WRITE_NORMAL;
		uint64_t seg, row, rec, col;
		uint64_t dim[4] = {1, 1, 1, 2};	/* Dataset dimension for one point */
		char i_file[GMT_VF_LEN] = {""}, o_file[GMT_VF_LEN] = {""}, grid[GMT_LEN128] = {""}, header[GMT_LEN256] = {""}, cmd[GMT_LEN128] = {""};
		struct GMT_DATASET *D = NULL;
		struct GMT_DATASEGMENT *Si = NULL, *So = NULL;

		dim[GMT_SEG] = In->n_records;		/* One output time-series per input data location */
		dim[GMT_ROW] = n_layers_used;		/* Length of each sampled time-series per input data location */
		dim[GMT_COL] = In->n_columns + 2;	/* x, y [,col3, col4...], time|z, value */
		if ((Out = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_WITH_STRINGS, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create output dataset for time-series\n");
			Return (API->error);
		}
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, In, i_file) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual dataset for time-series\n");
			Return (API->error);
		}

		for (k = start_k; k <= stop_k; k++) {	/* For all available input levels k */
			GMT_Init_VirtualFile (API, 0, i_file);	/* Reset so it can be read again */
			if (Ctrl->Z.active[GMT_IN])	/* Get the k'th file */
				sprintf (grid, "%s", Ctrl->In.file[k]);
			else	/* Get the k'th layer from 3D cube */
				sprintf (grid, "%s?%s[%" PRIu64 "]", Ctrl->In.file[0], cube_layer, k);
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, NULL, o_file) == GMT_NOTSET) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual dataset for time-series\n");
				Return (API->error);
			}

			sprintf (cmd, "%s -G%s ->%s", i_file, grid, o_file);
			if (GMT->common.R.active[RSET]) {	/* Gave a subregion, so pass -R along */
				strcat (cmd, " -R");
				strcat (cmd, GMT->common.R.string);
			}
			GMT_Report (API, GMT_MSG_DEBUG, "Sampling the grid: %s\n", cmd);
			if (GMT_Call_Module (API, "grdtrack", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Sample this layer at the points */
				Return (API->error);
			}
			if ((D = GMT_Read_VirtualFile (API, o_file)) == NULL) {	/* Get the results from grdtrack */
				GMT_Report (API, GMT_MSG_ERROR, "Unable to read virtual dataset for time-series created by grdtrack\n");
				Return (API->error);
			}

			for (seg = rec = 0; seg < D->table[0]->n_segments; seg++) {	/* For each point we sampled at */
				Si = D->table[0]->segment[seg];	/* Short hand to this segment */


				for (row = 0; row < Si->n_rows; row++, rec++) {	/* For each selected point which matches each output segment */
					So = Out->table[0]->segment[rec];	/* Short hand to this output segment */
					if (k == start_k) {	/* Set the segment header just once */
						if (Si->text && Si->text[row])
							sprintf (header, "Location %g,%g %s", Si->data[GMT_X][row], Si->data[GMT_Y][row], Si->text[row]);
						else
							sprintf (header, "Location %g,%g", Si->data[GMT_X][row], Si->data[GMT_Y][row]);
						So->header = strdup (header);
					}
					for (col = 0; col < Si->n_columns; col++)
						So->data[col][k] = Si->data[col][row];
					So->data[col][k] = level[k];	/* Add time as the last data column */
				}
			}
			for (col = GMT_Z; col < Si->n_columns; col++)
				gmt_set_column (GMT, GMT_OUT, col, GMT_IS_FLOAT);	/* These are data columns */
			if (GMT_Close_VirtualFile (API, i_file) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to close input virtual dataset for time-series\n");
				Return (API->error);
			}
			if (GMT_Close_VirtualFile (API, o_file) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to close output irtual dataset for time-series\n");
				Return (API->error);
			}
			if (GMT_Destroy_Data (API, &D) != GMT_OK) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to delete dataset returned by grdtrack\n");
				Return (API->error);
			}
		}
		gmt_set_column (GMT, GMT_OUT, col, level_type);	/* This is the grid-level data type which on output is in this column */

		if (Ctrl->T.active) {	/* Want to interpolate through the sampled points using the specified spline */
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, Out, i_file) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual dataset for sampled time-series\n");
				Return (API->error);
			}
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT, NULL, o_file) == GMT_NOTSET) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual dataset for sampled time-series\n");
				Return (API->error);
			}
			sprintf (cmd, "%s -F%s -N%d -T%s ->%s", i_file, Ctrl->F.spline, (int)(Out->n_columns - 1), Ctrl->T.string, o_file);
			if (GMT_Call_Module (API, "sample1d", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Interpolate each profile per -T */
				Return (API->error);
			}
			if (GMT_Destroy_Data (API, &Out) != GMT_OK) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to delete virtual dataset for time-series\n");
				Return (API->error);
			}
			if ((Out = GMT_Read_VirtualFile (API, o_file)) == NULL) {	/* Get the updated results from sample1d */
				GMT_Report (API, GMT_MSG_ERROR, "Unable to read virtual dataset for time-series created by sample1d\n");
				Return (API->error);
			}
			dim[GMT_ROW] = Out->table[0]->segment[0]->n_rows;	/* Update new row dim */
		}

		if (Ctrl->S.active) {	/* Write the table(s) */
			if (Ctrl->G.file && strchr (Ctrl->G.file, '%')) {	/* Want separate files per series, so change mode and build file names per segment */
				struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
				io_mode = GMT_WRITE_SEGMENT;	/* Flag that we want individual segment files */
				for (seg = 0; seg < Out->table[0]->n_segments; seg++) {	/* Set the output file names */
					SH = gmt_get_DS_hidden (Out->table[0]->segment[seg]);
					sprintf (file, Ctrl->G.file, seg);
					SH->file[GMT_OUT] = strdup (file);
				}
			}
			/* Time to write out the results to stdout, a file, or individual segment files */
			gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Here we always want to write the headers we built */
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, io_mode, NULL, Ctrl->G.file, Out) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to write dataset for time-series\n");
				Return (API->error);
			}
		}
		else {	/* Here we return the slice as a grid */
			uint64_t ij;
			char unit[GMT_LEN64] = {""};
			double wesn[4], inc[2];

			wesn[XLO] = Out->table[0]->segment[0]->data[2][0];	wesn[XHI] = Out->table[0]->segment[dim[GMT_SEG]-1]->data[2][0];
			if (dtype == GMT_IS_LON && wesn[XHI] < wesn[XLO]) wesn[XHI] += 360.0;
			wesn[YLO] = Out->table[0]->segment[0]->data[4][0];	wesn[YHI] = Out->table[0]->segment[0]->data[4][dim[GMT_ROW]-1];
			inc[GMT_X] = gmt_M_get_inc (GMT, wesn[XLO], wesn[XHI], Out->n_segments, GMT_GRID_NODE_REG);
			inc[GMT_Y] = gmt_M_get_inc (GMT, wesn[YLO], wesn[YHI], Out->table[0]->segment[0]->n_rows, GMT_GRID_NODE_REG);
			gmt_set_column (GMT, GMT_OUT, GMT_X, dtype);	/* This is data type of the distances */
			gmt_set_column (GMT, GMT_OUT, GMT_Y, GMT_IS_FLOAT);	/* This is data type of depth or time */
			gmt_set_column (GMT, GMT_OUT, GMT_Z, GMT_IS_FLOAT);	/* This is data type of the cube values */
			if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, wesn, inc, GMT_GRID_NODE_REG, GMT_NOTSET, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create a grid for slice\n");
				Return (API->error);
			}
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, "Grid slide through 3-D data cube", Grid)) Return (API->error);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);
			if (dtype == GMT_IS_LON)
				strcpy (Grid->header->x_units, "longitude [degrees_east]");
			else if (dtype == GMT_IS_LAT)
				strcpy (Grid->header->x_units, "latitude [degrees_north]");
			else {
				sprintf (unit, "Distance (%c)", Ctrl->E.unit);
				strcpy (Grid->header->x_units, unit);
			}
			//strcpy (Grid->header->y_units, level_unit);

			for (seg = 0; seg < Out->n_segments; seg++) {	/* Each segment represents one x-coordinate */
				So = Out->table[0]->segment[seg];	/* Short hand to this output segment */
				for (row = 0; row < So->n_rows; row++) {
					ij = gmt_M_ijp (Grid->header, row, seg);
					Grid->data[ij] = (float)So->data[3][row];
				}
			}
			if (GMT_Destroy_Data (API, &Out) != GMT_OK) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to delete final dataset for time-series\n");
				Return (API->error);
			}
			GMT_Report (API, GMT_MSG_INFORMATION, "Write sliced grid to output file %s\n", Ctrl->G.file);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
				Return (API->error);
			}
		}
		if (!Ctrl->Z.active[GMT_IN])
			gmt_M_free (GMT, level);
		/* OK done with this segment of the model */
		Return (GMT_NOERROR);
	}

	int_mode = Ctrl->F.mode + 10*Ctrl->F.type;	/* What mode we pass to the interpolator */

	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */

	if ((G[GMT_IN] = gmt_M_memory (GMT, NULL, n_layers, struct GMT_GRID *)) == NULL) Return (GMT_MEMORY_ERROR);	/* Allocate one grid per input layer */

	for (k = start_k; k <= stop_k; k++) {	/* Read the required layers into individual grid structures */
		if (Ctrl->Z.active[GMT_IN])	/* Get the k'th file */
			sprintf (file, "%s", Ctrl->In.file[k]);
		else	/* Get the k'th layer from 3D cube possibly via a selected variable */
			sprintf (file, "%s?%s[%" PRIu64 "]", Ctrl->In.file[0], cube_layer, k);
		if ((G[GMT_IN][k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, wesn, file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to read layer %" PRIu64 " from file %s.\n", k, file);
			Return (API->error);
		}
	}
	if (nc_layer) nc_layer[0] = '?';	/* Restore layer name */

	GMT_Report (API, GMT_MSG_INFORMATION, "Will read %" PRIu64 " layers (%" PRIu64 " - %" PRIu64 ") for levels %g to %g.\n", n_layers_used, start_k, stop_k, level[start_k], level[stop_k]);

	if (Ctrl->T.T.n > 1 && !Ctrl->Z.active[GMT_OUT]) {
		GMT_Report (API, GMT_MSG_ERROR, "Sorry, writing 3-D output netCDF cube is not implemented yet.  Use -Zo for now.\n");
		Return (GMT_MEMORY_ERROR);
	}
	GMT_Report (API, GMT_MSG_INFORMATION, "Interpolate %" PRIu64 " new layers (%g to %g in steps of %g).\n", Ctrl->T.T.n, Ctrl->T.T.array[0], Ctrl->T.T.array[Ctrl->T.T.n-1]);

	gmt_set_column (GMT, GMT_OUT, GMT_Z, GMT_IS_FLOAT);	/* The 3rd dimension is not time in the grids, but we may have read time via -Z with -f2T */

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
		gmt_intpol (GMT, &level[start_k], &i_value[start_k], n_layers_used, Ctrl->T.T.n, Ctrl->T.T.array, o_value, int_mode);	/* Resample at requested output levels */
		for (k = 0; k < Ctrl->T.T.n; k++)	/* For all output levels */
			G[GMT_OUT][k]->data[node] = (float)o_value[k];	/* Put interpolated output values at this (x,y) across all levels */
	}

	error = GMT_NOERROR;	/* Default return code */

	if (Ctrl->T.T.n == 1 || Ctrl->Z.active[GMT_OUT]) {	/* Special case of only sampling the cube at one layer or asking for 2-D slices via -Zo */
		for (k = 0; k < Ctrl->T.T.n; k++) {	/* For all output levels */
			if (Ctrl->Z.active[GMT_OUT])	/* Create the k'th layer file */
				sprintf (file, Ctrl->G.file, Ctrl->T.T.array[k]);
			else	/* Just this one layer grid */
				sprintf (file, "%s", Ctrl->G.file);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, G[GMT_OUT][k]) != GMT_NOERROR) {
				error = API->error;
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

	Return (error);
}
