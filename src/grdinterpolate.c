/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * the cube at arbitrary depth z (or time) values and writes either a single
 * slice 2-D grid or another multi-level 3-D data cube.  Alternatively,
 * we can read a stack of input 2-D grids instead of the 3D cube.  Finally,
 * we may sample time-series (-S) or extract a vertical slice (-E) rather
 * than write gridded horizontal output slice(s).
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
#define THIS_MODULE_PURPOSE	"Interpolate a 3-D cube, 2-D grids or 1-D series from a 3-D data cube or stack of 2-D grids"
#define THIS_MODULE_KEYS	"<G{+,G?}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"->RVfn"

struct GRDINTERPOLATE_CTRL {
	struct GRDINTERPOLATE_In {
		bool active;
		char **file;
		unsigned int n_files;
	} In;
	struct GRDEDIT_D {	/* -D[+x<xname>][+yyname>][+z<zname>][+v<zname>][+s<scale>][+o<offset>][+n<invalid>][+t<title>][+r<remark>] */
		bool active;
		char *information;
	} D;
	struct GRDINTERPOLATE_E {	/* -E<file>|<line1>[,<line2>,...][+a<az>][+c][+g][+i<step>][+l<length>][+n<np][+o<az>][+p][+r<radius>][+x] */
		bool active;
		unsigned int mode;
		char *lines;
		double step;
		char unit;
	} E;
	struct GRDINTERPOLATE_F {	/* -Fl|a|c[1|2] */
		bool active;
		unsigned int mode;
		unsigned int type;
		char spline[GMT_LEN8];
	} F;
	struct GRDINTERPOLATE_G {	/* -G<output_grdfile>  */
		bool active;
		char *file;
	} G;
	struct GRDINTERPOLATE_S {	/* -S<x>/<y>|<pointfile>[+h<header>] */
		bool active;
		double x, y;
		char *file;
		char *header;
	} S;
	struct GRDINTERPOLATE_T {	/* -T<start>/<stop>/<inc> or -T<value> */
		bool active;
		struct GMT_ARRAY T;
		char *string;
	} T;
	struct GRDINTERPOLATE_Z {	/* -Z<min>/<max>/<inc>, -Z<file>, or -Z<list> */
		bool active;
		unsigned mode;	/* 1 if need to create 0/n-1/1 array */
		struct GMT_ARRAY T;
	} Z;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	static char type[3] = {'l', 'a', 'c'};
	struct GRDINTERPOLATE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDINTERPOLATE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	sprintf (C->F.spline, "%c", type[GMT->current.setting.interpolant]);	/* Set default interpolant */
	return (C);
}

GMT_LOCAL void grdinterpolate_free_files (struct GMT_CTRL *GMT, char ***list, unsigned int n) {
	for (unsigned int k = 0; k < n; k++)
		gmt_M_str_free ((*list)[k]);
	gmt_M_free (GMT, *list);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDINTERPOLATE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	grdinterpolate_free_files (GMT, &(C->In.file), C->In.n_files);
	gmt_M_str_free (C->D.information);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->S.file);
	gmt_M_str_free (C->S.header);
	gmt_M_str_free (C->T.string);
	gmt_free_array (GMT, &(C->T.T));
	gmt_free_array (GMT, &(C->Z.T));
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	static char type[3] = {'l', 'a', 'c'};
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s <cube> | <grd1> <grd2> <grd3> ... -G<outfile> [%s] "
		"[-E<file>|<line1>[,<line2>,...][+a<az>][+g][+i<step>][+l<length>][+n<np][+o<az>][+p][+r<radius>][+x]] "
		"[-Fl|a|c|n[+1|2]] [%s] [-S<x>/<y>|<table>[+h<header>]] [-T[<min>/<max>/]<inc>[+i|n]] [%s] "
		"[-Z[<levels>]] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
			name,  GMT_GRDEDIT3D, GMT_Rgeo_OPT, GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT,
			GMT_h_OPT, GMT_i_OPT, GMT_n_OPT, GMT_o_OPT, GMT_q_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");

	GMT_Usage (API, 1, "\n<cube> is the name of the input 3D netCDF data cube. However, with -Z we instead expect a "
		"series of 2-D grids.");
	GMT_Usage (API, 1, "\n-G<outfile>");
	GMT_Usage (API, -2, "Specify a single output file name (or a filename format template; also see -S) To write a "
		"series of 2-D grids instead of a cube, include a floating-point C-format statement in <outfile> set via "
		"-G for embedding the level in the file name.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	gmt_cube_info_syntax (API->GMT, 'D');
	GMT_Usage (API, 1, "\n-E<file>|<line1>[,<line2>,...][+a<az>][+g][+i<step>][+l<length>][+n<np][+o<az>][+p][+r<radius>][+x]");
	GMT_Usage (API, -2, "Set up a single cross-section based on <file> or on the given <line1>[,<line2>,...]. Give "
		"start and stop coordinates for each line segment.  The format of each <line> is <start>/<stop>, where "
		"<start> or <stop> are coordinate pairs, e.g., <lon1/lat1>/<lon2>/<lat2>. Append +i<inc> to set the "
		"sampling increment [Default is 0.5 x min of grid's (x_inc, y_inc)] Instead of <start/stop>, give <origin> "
		"and append +a|o|l|n|r as required:");
	GMT_Usage (API, -3, "+a Define a profiles from <origin> in <az> direction. Add +l<length>.");
	GMT_Usage (API, -3, "+g Use gridline coordinates (degree longitude or latitude) if <line> is so aligned [great circle].");
	GMT_Usage (API, -3, "+o Define a profile centered on <origin> in <az> direction. Add +l<length>.");
	GMT_Usage (API, -3, "+p Sample along the parallel if <line> has constant latitude.");
	GMT_Usage (API, -3, "+r Define a circle about <origin> with given <radius>. Add +i<inc> or +n<np>.");
	GMT_Usage (API, -3, "+n Set the number of output points as <np> and computes <inc> from <length>.");
	GMT_Usage (API, -3, "+x Follow a loxodrome (rhumbline) [great circle].");
	GMT_Usage (API, -2, "Note:  A unit is optional.  Only ONE unit type from %s can be used throughout this option, so "
		"mixing of units is not allowed [Default unit is km, if grid is geographic].");
	GMT_Usage (API, 1, "\n-Fl|a|c|n][+1|2]");
	GMT_Usage (API, -2, "Set the grid interpolation mode.  Choose from:");
	GMT_Usage (API, -3, "l: Linear interpolation.");
	GMT_Usage (API, -3, "a: Akima spline interpolation.");
	GMT_Usage (API, -3, "c: Cubic spline interpolation.");
	GMT_Usage (API, -3, "n: No interpolation (nearest point).");
	GMT_Usage (API, -2, "Optionally, append +1 for 1st derivative or +2 for 2nd derivative. [Default is -F%c].",
		type[API->GMT->current.setting.interpolant]);
	GMT_Option (API, "R");
	GMT_Usage (API, 1, "\n-S<x>/<y>|<table>[+h<header>]");
	GMT_Usage (API, -2, "Give a fixed point for across-stack sampling [Default] or interpolation [with -T]. For "
		"multiple points, give a <table> of points instead (one point per record). Output is a multi-segment table "
		"written to standard output unless -G is used to set a file name. To write each series to separate files, let "
		"-G<outfile> contain a C-format integer specifier (e.g, %%d) for embedding the running point number. "
		"Append a fixed header via +h<header> [trailing text per record in <table>].");
	GMT_Usage (API, 1, "\n-T[<file>|<list>|<min>/<max>/<inc>[+b|i|l|n]]");
		GMT_Usage (API, -2, "Interpolate the 3-D grid at given levels across the 3rd dimension. Make evenly spaced output "
		"level steps from <min> to <max> by <inc>. Control setup via modifiers:");
	GMT_Usage (API, 3, "+b Select log2 spacing in <inc>");
	GMT_Usage (API, 3, "+i Indicate <inc> is the reciprocal of desired <inc> (e.g., 3 for 0.3333.....).");
	GMT_Usage (API, 3, "+l Select log10 spacing via <inc> = 1,2,3.");
	GMT_Usage (API, 3, "+n Let <inc> mean the number of points instead. of increment");
	GMT_Usage (API, -2, "Alternatively, give a <file> with output times in the first column, or a comma-separated <list>.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-Z[<levels>]");
	GMT_Usage (API, -2, "Read or write 2-D grids that make up a virtual 3-D data cube. To read a series of 2-D grids, "
		"give -Z<levels>, where <levels> for each grid is set via <min>/<max>/<inc>, <zfile>, or a comma-separated "
		"list. No argument means let levels be 0, 1, 2, ...");
	GMT_Usage (API, -2, "Note: If -Z and no -T, -E, -S then we simply write the stack as a 3-D data cube.");
	GMT_Option (API, "a,bi2,bo,d,e,f,g,h,i,n,o,q,s,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDINTERPOLATE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, n_alloc = 0, k = 0;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_alloc <= Ctrl->In.n_files) {
					n_alloc += GMT_SMALL_CHUNK;
					Ctrl->In.file = gmt_M_memory (GMT, Ctrl->In.file, n_alloc, char *);
				}
				Ctrl->In.file[Ctrl->In.n_files] = strdup (opt->arg);
				if (GMT_Get_FilePath (GMT->parent, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file[Ctrl->In.n_files]))) n_errors++;
				Ctrl->In.n_files++;
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Give grid information */
				Ctrl->D.active = true;
				Ctrl->D.information = strdup (opt->arg);
				break;
			case 'E':	/* Create or read an equidistant profile for slicing */
				Ctrl->E.active = true;
				Ctrl->E.lines = strdup (opt->arg);
				break;
			case 'F':	/* Set the spline type */
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'l':	Ctrl->F.mode = GMT_SPLINE_LINEAR;	break;
					case 'a':	Ctrl->F.mode = GMT_SPLINE_AKIMA;	break;
					case 'c':	Ctrl->F.mode = GMT_SPLINE_CUBIC;	break;
					case 'n':	Ctrl->F.mode = GMT_SPLINE_NN;		break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -F: Bad spline selector %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				if (opt->arg[1] == '+') Ctrl->F.type = (opt->arg[2] - '0');	/* Want first or second derivative */
				strncpy (Ctrl->F.spline, opt->arg, GMT_LEN8-1);	/* Keep track of what was given since it may need to be passed verbatim to other modules */
				break;
			case 'G':	/* Output file or name template */
				if (n_files++ > 0) { n_errors++; continue; }
				Ctrl->G.file = strdup (opt->arg);
				if (strchr (Ctrl->G.file, '%') == NULL) {	/* Gave a fixed output file, can check */
					if (GMT_Get_FilePath (GMT->parent, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file))) n_errors++;
				}
				break;
			case 'T':	/* Set level sampling spacing */
				Ctrl->T.active = true;
				Ctrl->T.string = strdup (opt->arg);
				n_errors += gmt_parse_array (GMT, 'T', opt->arg, &(Ctrl->T.T), GMT_ARRAY_TIME | GMT_ARRAY_SCALAR | GMT_ARRAY_RANGE | GMT_ARRAY_UNIQUE, GMT_Z);
				break;

			case 'S':	/* Sample vertically across the grid stack at one or more points */
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
				if (access (opt->arg, F_OK) && strchr (opt->arg, '/')) {	/* Got a single point and not a valid path */
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
				else {
					Ctrl->S.file = strdup (opt->arg);
					if (GMT_Get_FilePath (GMT->parent, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->S.file))) n_errors++;
				}
				if (c) c[0] = '+';	/* Restore modifiers */
				break;

			case 'Z':	/* Control input/output grid management */
				Ctrl->Z.active = true;
				if (opt->arg[0] == 'i') k = 1;	/* Skip the now deprecated 'i' in -Zi */
				if (opt->arg[k])
					n_errors += gmt_parse_array (GMT, 'Z', &opt->arg[k], &(Ctrl->Z.T), GMT_ARRAY_TIME | GMT_ARRAY_SCALAR | GMT_ARRAY_RANGE | GMT_ARRAY_UNIQUE, GMT_Z);
				else
					Ctrl->Z.mode = 1;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	if (Ctrl->In.n_files) Ctrl->In.file = gmt_M_memory (GMT, Ctrl->In.file, Ctrl->In.n_files + 1, char *);	/* One extra so we have a NULL-terminated array */

	n_errors += gmt_M_check_condition (GMT, Ctrl->In.n_files < 1, "Error: No input grid(s) specified.\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->Z.active && Ctrl->In.n_files != 1, "Must specify a single input 3D grid cube file unless -Z is set\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.type > 2, "Option -F: Only 1st or 2nd derivatives may be requested\n");
	if (!(Ctrl->S.active || Ctrl->E.active)) {	/* Under -S and -E, the -T and -G are optional */
		n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Option -G: Must specify output grid file\n");
		n_errors += gmt_M_check_condition (GMT, n_files != 1, "Must specify only one output file name\n");
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->S.active, "Options -E and -S cannot be used together\n");
	if (Ctrl->E.active) {
		n_errors += gmt_M_check_condition (GMT, strstr (Ctrl->E.lines, "+d"), "Option -E: Unrecognized modifier +d\n");
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && !(Ctrl->T.active || Ctrl->E.active || Ctrl->S.active) && strchr (Ctrl->G.file, '%'), "Options -Z: If -T not given the we must write a single data cube\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && API->external && strchr (Ctrl->G.file, '%'), "Option -G: Cannot contain format-specifiers when not used on the command line\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL bool grdinterpolate_equidistant_levels (struct GMT_CTRL *GMT, double *z, unsigned int nz) {
	/* Return true if spacing between layers is constant */
	unsigned int k;
	double dz;

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {	/* Report levels */
		for (k = 0; k < nz; k++)
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Level %d: %g\n", k, z[k]);
	}
	if (nz < 3) return true;	/* Special case of a single layer */
	dz = z[1] - z[0];	/* Get first increment, then compare to the rest */
	for (k = 2; k < nz; k++)
		if (!doubleAlmostEqual (dz, z[k]-z[k-1])) return false;
	return true;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grdinterpolate (void *V_API, int mode, void *args) {
	char file[PATH_MAX] = {""}, cube_layer[GMT_LEN64] = {""}, *nc_z_named = NULL;
	bool equi_levels, convert_to_cube = false, z_is_abstime = false, got_cube = false;
	int error = 0;
	unsigned int int_mode, row, col, level_type, dtype = 0;
	uint64_t n_layers = 0, k, node, start_k, stop_k, n_layers_used, *this_dim = NULL, dims[3] = {0, 0, 0};
	double wesn[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, inc[3] = {0.0, 0.0, 0.0}, w_range[2] = {0.0, 0.0};
	double *level = NULL, *i_value = NULL, *o_value = NULL;
	struct GMT_GRID *Grid = NULL;
	struct GMT_CUBE *C[2] = {NULL, NULL};     /* Structures to hold input/output cubes */

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

	gmt_grd_set_datapadding (GMT, true);	/* Turn on gridpadding when reading a subset */

	if (Ctrl->Z.active) {	/* Create the input level array */
		if (Ctrl->Z.mode == 1)	/* No levels given, auto-make a list from 0, 1, ... */
			n_layers = gmt_make_equidistant_array (GMT, 0.0, Ctrl->In.n_files-1.0, 1.0, &level);
		else {	/* Gave information to build a level array */
			if (gmt_create_array (GMT, 'Z', &(Ctrl->Z.T), NULL, NULL)) {
				GMT_Report (API, GMT_MSG_ERROR, "Option -Z: Unable to set up input level array\n");
				Return (API->error);
			}
			if (Ctrl->In.n_files != Ctrl->Z.T.n) {
				GMT_Report (API, GMT_MSG_ERROR, "Option -Z: Number of input 2-D grids does not match number of levels given via -Z\n");
				Return (API->error);
			}
			n_layers = Ctrl->Z.T.n;		/* Set number of layers anticipated */
			level = Ctrl->Z.T.array;	/* Pointer to allocated array with the level values */
			z_is_abstime = Ctrl->Z.T.temporal;	/* In case we parsed abs time for levels */
		}
		if (!(Ctrl->T.active || Ctrl->E.active || Ctrl->S.active)) convert_to_cube = true;	/* Just want to build cube from input stack */
	}
	else if (gmt_M_file_is_memory (Ctrl->In.file[0])) {	/* Got a memory reference */
		if (Ctrl->In.n_files == 1) {	/* Got one memory reference so it must be a cube */
			if (Ctrl->In.file[0][GMTAPI_OBJECT_FAMILY_START] != 'U') {
				GMT_Report (API, GMT_MSG_ERROR, "Input memory reference is not a cube!\n");
				Return (API->error);
			}
			C[GMT_IN] = GMT_Read_VirtualFile (API, Ctrl->In.file[0]);
			n_layers = C[GMT_IN]->header->n_bands;
			level = C[GMT_IN]->z;
			got_cube = true;
		}
		else {	/* If many references given then clearly we are missing -Z */
			GMT_Report (API, GMT_MSG_ERROR, "Multiple grid memory references requires -Z!\n");
			Return (API->error);
		}
	}
	else {	/* See if we got a 3D netCDF data cube; if so return number of layers and and the levels array */
		nc_z_named = strchr (Ctrl->In.file[0], '?');	/* Maybe given a specific variable? */
		if (nc_z_named) {	/* Gave a specific variable. Keep variable name and remove from filename */
			strcpy (cube_layer, &nc_z_named[1]);
			nc_z_named[0] = '\0';	/* Chop off layer name for now */
		}
		if ((error = gmt_nc_read_cube_info (GMT, Ctrl->In.file[0], w_range, &n_layers, &level, NULL))) {
			Return (error);
		}
		got_cube = true;
	}

	if (n_layers == 1) {
		GMT_Report (API, GMT_MSG_ERROR, "Only one layer given - need at least two to interpolate across levels\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if (got_cube && !(Ctrl->E.active || Ctrl->S.active || Ctrl->T.active)) {	/* If given a cube we requires other options */
		GMT_Report (API, GMT_MSG_ERROR, "Data cube read but none of -E, -S -T were given\n");
		Return (GMT_RUNTIME_ERROR);
	}
	/* Create output level array, if selected */
	if (Ctrl->T.active && gmt_create_array (GMT, 'T', &(Ctrl->T.T), NULL, NULL)) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -T: Unable to set up output level array\n");
		Return (API->error);
	}
	equi_levels = grdinterpolate_equidistant_levels (GMT, level, n_layers);	/* Are levels equidistant? */
	level_type = gmt_M_type (GMT, GMT_IN, GMT_Z);	/* Either time or floating point values like depth */

	/* Determine the range of input layers needed for interpolation */

	start_k = 0; stop_k = n_layers - 1;	/* We first assume all layers are needed */
	if (Ctrl->T.active) {
		if (Ctrl->T.T.array[0] > level[stop_k] || Ctrl->T.T.array[Ctrl->T.T.n-1] < level[start_k]) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -T: Specified range outside that of the data cube\n");
			Return (GMT_RUNTIME_ERROR);
		}
		while (start_k < n_layers && Ctrl->T.T.array[0] > level[start_k])	/* Find the first layer that is inside the output time range */
			start_k++;
		if (start_k && Ctrl->T.T.array[0] < level[start_k]) start_k--;		/* Go back one if start time is less than first layer */
		if (start_k && (Ctrl->F.mode == GMT_SPLINE_AKIMA || Ctrl->F.mode == GMT_SPLINE_CUBIC))
			start_k--;	/* One more to define the spline coefficients */
		while (stop_k && Ctrl->T.T.array[Ctrl->T.T.n-1] < level[stop_k])	/* Find the last layer that is inside the output time range */
			stop_k--;
		if (stop_k < n_layers && Ctrl->T.T.array[Ctrl->T.T.n-1] > level[stop_k]) stop_k++;	/* Go forward one if stop time is larger than last layer */
		if (stop_k < (n_layers-1) && (Ctrl->F.mode == GMT_SPLINE_AKIMA || Ctrl->F.mode == GMT_SPLINE_CUBIC))
			stop_k++;	/* One more to define the spline coefficients */
	}
	n_layers_used = stop_k - start_k + 1;	/* Total number of input layers needed */
	if (n_layers_used == 1) {	/* Might have landed exactly on one of the grid levels, but gmt_intpol needs at least 2 inputs */
		if (start_k) start_k--;
		else stop_k++;	/* We know there are at least 2 input grids at this point in the code */
		n_layers_used = 2;
	}

	if (Ctrl->E.active) {	/* Create or read profile */
		if (!gmt_access (GMT, Ctrl->E.lines, F_OK)) {	/* Gave a file with a pre-computed crossection */
			struct GMT_DATASEGMENT *Si = NULL;
			if ((In = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, Ctrl->E.lines, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open or read file %s.\n", Ctrl->E.lines);
				Return (API->error);
			}
			if (In->n_segments > 1) {
				GMT_Report (API, GMT_MSG_ERROR, "File %s contains more than one segment\n", Ctrl->E.lines);
				Return (API->error);
			}
			if (In->n_columns < 2) {
				GMT_Report (API, GMT_MSG_ERROR, "File %s does not contain coordinate pairs\n", Ctrl->E.lines);
				Return (API->error);
			}
			Si = In->table[0]->segment[0];	/* Short-hand to only segment */
			if (In->n_columns < 3) {	/* Need to create distance column array */
				gmt_adjust_dataset (GMT, In, 3);	/* Add one more output column */
				gmt_M_free (GMT, Si->data[GMT_Z]);	/* Free it so we can add it next */
				Si->data[GMT_Z] = gmt_dist_array (GMT, Si->data[GMT_X], Si->data[GMT_Y], Si->n_rows, true);
			}
			if (!grdinterpolate_equidistant_levels (GMT, Si->data[GMT_Z], Si->n_rows)) {
				GMT_Report (API, GMT_MSG_ERROR, "File %s does not contain equidistant coordinates\n", Ctrl->E.lines);
				Return (API->error);
			}
		}
		else {	/* Create profile */
			char prof_args[GMT_LEN128] = {""};
			if (!(equi_levels || Ctrl->T.active)) {
				GMT_Report (API, GMT_MSG_ERROR, "Option -E requires either equidistant levels or resampling via -T\n");
				Return (GMT_RUNTIME_ERROR);
			}
			if (gmt_get_dist_units (GMT, Ctrl->E.lines, &Ctrl->E.unit, &Ctrl->E.mode)) {	/* Bad mixing of units in -E specification */
				GMT_Report (API, GMT_MSG_ERROR, "Cannot mix units in -E (%s)\n", Ctrl->E.lines);
				Return (GMT_RUNTIME_ERROR);
			}
			if (gmt_init_distaz (GMT, Ctrl->E.unit, Ctrl->E.mode, GMT_MAP_DIST) == GMT_NOT_A_VALID_TYPE)	/* Initialize the distance unit and scaling */
				Return (GMT_NOT_A_VALID_TYPE);

			/* Need to get dx,dy from one grid */
			if (Ctrl->Z.active)	/* Get the first file */
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
		}
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
			if (Ctrl->S.header) {	/* Want to use this fixed text to add to the trailing text of all the points */
				for (seg = 0; seg < In->n_segments; seg++) {
					Si = In->table[0]->segment[seg];	/* Short hand to this segment */
					if (Si->text == NULL)	/* Input file did not have any trailing text so add array now */
						Si->text = gmt_M_memory (GMT, NULL, Si->n_rows, char *);
					for (row = 0; row < Si->n_rows; row++) {
						if (Si->text[row]) {	/* Already has trailing text, combine with user argument */
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

	if (Ctrl->E.active || Ctrl->S.active) {	/* Vertical profiles or slice */
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
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN|GMT_IS_REFERENCE, In, i_file) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual dataset for time-series\n");
			Return (API->error);
		}

		for (k = start_k; k <= stop_k; k++) {	/* For all selected input levels k */
			GMT_Init_VirtualFile (API, 0, i_file);	/* Reset so it can be read again */
			if (Ctrl->Z.active)	/* Get the k'th file */
				sprintf (grid, "%s", Ctrl->In.file[k]);
			else	/* Get the k'th layer from 3D cube */
				sprintf (grid, "%s?%s[%" PRIu64 "]", Ctrl->In.file[0], cube_layer, k);
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT|GMT_IS_REFERENCE, NULL, o_file) == GMT_NOTSET) {
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
			if (D->n_tables == 0 || D->table[0]->n_segments == 0) {
				GMT_Report (API, GMT_MSG_ERROR, "No data time-series created by grdtrack\n");
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
					for (col = 0; col < Si->n_columns; col++)	/* Copy over the various columns */
						So->data[col][k] = Si->data[col][row];
					So->data[col][k] = level[k];	/* Add level as the last data column */
				}
			}
			for (col = GMT_Z; col < Si->n_columns; col++)
				gmt_set_column_type (GMT, GMT_OUT, col, GMT_IS_FLOAT);	/* These are data columns */
			if (GMT_Close_VirtualFile (API, i_file) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to close input virtual dataset for time-series\n");
				Return (API->error);
			}
			if (GMT_Close_VirtualFile (API, o_file) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to close output virtual dataset for time-series\n");
				Return (API->error);
			}
			if (GMT_Destroy_Data (API, &D) != GMT_OK) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to delete dataset returned by grdtrack\n");
				Return (API->error);
			}
		}
		gmt_set_column_type (GMT, GMT_OUT, col, level_type);	/* This is the grid-level data type which on output is in this column */

		if (Ctrl->T.active) {	/* Want to interpolate through the sampled points using the specified spline */
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN|GMT_IS_REFERENCE, Out, i_file) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual dataset for sampled time-series\n");
				Return (API->error);
			}
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT|GMT_IS_REFERENCE, NULL, o_file) == GMT_NOTSET) {
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
			if (dtype == GMT_IS_LON && wesn[XHI] < wesn[XLO]) wesn[XHI] += 360.0;	/* Watch out for 360 wraps */
			wesn[YLO] = Out->table[0]->segment[0]->data[4][0];	wesn[YHI] = Out->table[0]->segment[0]->data[4][dim[GMT_ROW]-1];
			inc[GMT_X] = gmt_M_get_inc (GMT, wesn[XLO], wesn[XHI], Out->n_segments, GMT_GRID_NODE_REG);
			inc[GMT_Y] = gmt_M_get_inc (GMT, wesn[YLO], wesn[YHI], Out->table[0]->segment[0]->n_rows, GMT_GRID_NODE_REG);
			gmt_set_column_type (GMT, GMT_OUT, GMT_X, dtype);	/* This is data type of the distances */
			gmt_set_column_type (GMT, GMT_OUT, GMT_Y, GMT_IS_FLOAT);	/* This is data type of depth or time */
			gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_FLOAT);	/* This is data type of the cube values */
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

			for (seg = 0; seg < Out->n_segments; seg++) {	/* Each segment represents one x-coordinate */
				So = Out->table[0]->segment[seg];	/* Short hand to this output segment */
				for (row = 0; row < So->n_rows; row++) {
					ij = gmt_M_ijp (Grid->header, So->n_rows-row-1, seg);	/* Must flip order since rows in grid goes down */
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
		if (!Ctrl->Z.active)
			gmt_M_free (GMT, level);
		Return (GMT_NOERROR);
	}

	/* Get here if neither -E nor -S were selected: We want to interpolate for one or more horizontal slices in the cube and need to read/write cubes */

	int_mode = gmt_set_interpolate_mode (GMT, Ctrl->F.mode, Ctrl->F.type);	/* What mode we pass to the interpolation */

	if (GMT->common.R.active[RSET])	/* Use current -R setting for subsets, if given */
		gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);
	wesn[ZLO] = level[start_k];	wesn[ZHI] = level[stop_k];	/* Then add the zmin/zmax limitation */

	GMT_Report (API, GMT_MSG_INFORMATION, "Will read %" PRIu64 " layers (%" PRIu64 " - %" PRIu64 ") for levels %g to %g.\n", n_layers_used, start_k, stop_k, level[start_k], level[stop_k]);

	/* Read the selected subset of the cube into C[GMT_IN] */

	if (Ctrl->Z.active) {	/* Need to read in individual grids and convert to cube first */
		size_t here = 0;
		unsigned int N = n_layers;
		struct GMT_GRID **G = NULL;

		if ((G = GMT_Read_Group (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, wesn, Ctrl->In.file, &N, NULL)) == NULL)
			Return (EXIT_FAILURE);
		if (!GMT->common.R.active[RSET])	/* Use current -R setting for subsets, if given */
			gmt_M_memcpy (wesn, G[0]->header->wesn, 4, double);
		dims[GMT_Z] = n_layers;	/* Number of input levels */
		gmt_M_memcpy (inc, G[0]->header->inc, 2U, double);
		if ((C[GMT_IN] = GMT_Create_Data (API, GMT_IS_CUBE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, dims, wesn, inc, G[0]->header->registration, GMT_NOTSET, NULL)) == NULL)
			Return (GMT_MEMORY_ERROR);
		for (k = 0; k < n_layers; k++, here += G[0]->header->size)
			gmt_M_memcpy (&(C[GMT_IN]->data[here]), G[k]->data, G[0]->header->size, gmt_grdfloat);
		if (GMT_Destroy_Group (API, &G, n_layers))
			Return (API->error);
		if (C[GMT_IN]->z == NULL && GMT_Put_Levels (API, C[GMT_IN], level, n_layers))
			Return (API->error);
		C[GMT_IN]->mode = GMT_CUBE_IS_STACK;	/* Flag that the source was a stack of grids and not a cube */
		if (gmt_M_is_geographic (GMT, GMT_IN))
			gmt_set_geographic (GMT, GMT_OUT);
		else
			gmt_set_cartesian (GMT, GMT_OUT);
		if (z_is_abstime) gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ABSTIME);
	}
	else if (C[GMT_IN] == NULL) {	/* Read the cube */
		gmt_M_free (GMT, level);	/* Free this one now since it will be re-read by GMT_Read_Data */
		if ((C[GMT_IN] = GMT_Read_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, wesn, Ctrl->In.file[0], NULL)) == NULL)
			Return (GMT_DATA_READ_ERROR);
	}

	if (convert_to_cube) {	/* Just want to build cube from input stack */
		if (Ctrl->D.active && gmt_decode_cube_h_info (GMT, Ctrl->D.information, C[GMT_IN])) {
			GMT_Destroy_Data (API, &C[GMT_IN]);
			Return (GMT_PARSE_ERROR);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Convert %" PRIu64 " grid layers to a single data cube %s.\n", n_layers, Ctrl->G.file);
		if (GMT_Set_Comment (API, GMT_IS_CUBE, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, C[GMT_IN]))
			Return (EXIT_FAILURE);
		if (GMT_Write_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, C[GMT_IN]))
			Return (EXIT_FAILURE);
		Return (GMT_NOERROR);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Interpolate %" PRIu64 " new layers (%g to %g in steps of %g).\n", Ctrl->T.T.n, Ctrl->T.T.array[0], Ctrl->T.T.array[Ctrl->T.T.n-1]);

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_FLOAT);	/* The 3rd dimension is not time in the grids, but we may have read time via -Z with -f2T */

	/* Create cube with layers for each output level */

	gmt_M_memcpy (wesn, C[GMT_IN]->header->wesn, 4, double);	/* This is the output common x/y region now */
	inc[GMT_X] = C[GMT_IN]->header->inc[GMT_X];	inc[GMT_Y] = C[GMT_IN]->header->inc[GMT_Y];	/* And common x/y increments */
	if (Ctrl->T.T.var_inc || Ctrl->T.T.n == 1) {	/* Not equidistant output levels selected via -T so must pass the number of output levels instead of increment */
		dims[GMT_Z] = Ctrl->T.T.n;	/* Number of output levels */
		this_dim = dims;	/* Pointer to the dims instead of NULL */
		inc[GMT_Z] = 0.0;
	}
	else {	/* Normal equidistant output levels lets us pass z-inc */
		inc[GMT_Z] = Ctrl->T.T.inc;
		wesn[ZLO] = Ctrl->T.T.min;
		wesn[ZHI] = Ctrl->T.T.max;
	}

	if ((C[GMT_OUT] = GMT_Create_Data (API, GMT_IS_CUBE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, this_dim, wesn, inc, C[GMT_IN]->header->registration, GMT_NOTSET, NULL)) == NULL)
		Return (GMT_MEMORY_ERROR);

	if (Ctrl->D.active && gmt_decode_cube_h_info (GMT, Ctrl->D.information, C[GMT_OUT])) {
		GMT_Destroy_Data (API, &C[GMT_OUT]);
		Return (GMT_PARSE_ERROR);
	}

	/* If not equidistant we must add in the level array manually */
	if (C[GMT_OUT]->z == NULL && GMT_Put_Levels (API, C[GMT_OUT], Ctrl->T.T.array, Ctrl->T.T.n))
		Return (API->error);

	/* Allocate input and output arrays for the 1-D spline */
	if ((i_value = gmt_M_memory (GMT, NULL, C[GMT_IN]->header->n_bands, double)) == NULL) Return (GMT_MEMORY_ERROR);
	if ((o_value = gmt_M_memory (GMT, NULL, Ctrl->T.T.n, double)) == NULL) Return (GMT_MEMORY_ERROR);

	/* Loop over all coregistered x/y nodes then drill through the cube to resample at new layer values */
	for (row = 0; row < C[GMT_IN]->header->n_rows; row++) {
		node = gmt_M_ijp (C[GMT_IN]->header, row, 0);	/* Relative node numbers in the input and output layers */
		for (col = 0; col < C[GMT_IN]->header->n_columns; col++, node++) {
			for (k = 0; k < C[GMT_IN]->header->n_bands; k++)	/* For all available input levels, extract data[x,y,z(k)] */
				i_value[k] = C[GMT_IN]->data[node+k*C[GMT_IN]->header->size];
			gmt_intpol (GMT, C[GMT_IN]->z, i_value, NULL, C[GMT_IN]->header->n_bands, Ctrl->T.T.n, Ctrl->T.T.array, o_value, 0.0, int_mode);	/* Resample at requested output levels */
			for (k = 0; k < Ctrl->T.T.n; k++)	/* For all output levels, place the interpolated values at this (x,y) across all levels */
				C[GMT_OUT]->data[node+k*C[GMT_OUT]->header->size] = (float)o_value[k];
		}
	}
	GMT_Destroy_Data (API, &C[GMT_IN]);	/* Done with the input cube */

	if (GMT_Set_Comment (API, GMT_IS_CUBE, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, C[GMT_OUT]))
		Return (EXIT_FAILURE);
	if (GMT_Write_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, C[GMT_OUT]))
		Return (EXIT_FAILURE);

	GMT_Destroy_Data (API, &C[GMT_OUT]);	/* Done with the output cube */

	/* Done with everything; free up remaining memory */

	gmt_M_free (GMT, i_value);
	gmt_M_free (GMT, o_value);

	Return (GMT_NOERROR);
}
