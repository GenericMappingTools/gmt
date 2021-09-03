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
 * Author:	Paul Wessel
 * Date:	8-NOV-2017
 * Version:	6 API
 *
 * Brief synopsis: grd2kml reads a single grid and makes a Google Earth
 * image quadtree.  Optionally, supply an intensity grid (or auto-derive it)
 * and a CPT (or use default table), and request a contour overlay.
 * If contours are not requested we can write the PNG tiles directly without
 * going via a PostScript plot.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grd2kml"
#define THIS_MODULE_MODERN_NAME	"grd2kml"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Create KML image quadtree from single grid"
#define THIS_MODULE_KEYS	"<G{,CC(,IG(,WD("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"-Vfn"

/* Note: If -n is given here it is automatically set in any module called below, such as grdimage */

struct GRD2KML_CTRL {
	struct GRD2KM_In {
		bool active;
		char *file;
	} In;
	struct GRD2KML_A {	/* -Ag|s|a[<altitude>] [g] */
		bool active;
		int mode;
		double altitude;
	} A;
	struct GRD2KML_C {	/* -C<cpt> or -C<color1>,<color2>[,<color3>,...][+i<dz>] */
		bool active;
		double dz;
		char *file;
	} C;
	struct GRD2KML_D {	/* -D[+s][+d]  [DEBUG ONLY, NOT DOCUMENTED] */
		bool active;
		bool single;
		bool dump;
	} D;
	struct GRD2KML_E {	/* -E<url> */
		bool active;
		char *url;
	} E;
	struct GRD2KML_F {	/* -F<filter> */
		bool active;
		char filter;
	} F;
	struct GRD2KML_H {	/* -H<scale> */
		bool active;
		int factor;
	} H;
	struct GRD2KML_N {	/* -N<prefix> */
		bool active;
		char *prefix;
	} N;
	struct GRD2KML_I {	/* -I[<intensfile>|<value>|<modifiers>] */
		bool active;
		bool constant;
		bool derive;
		double value;
		char *azimuth;	/* Default azimuth(s) for shading */
		char *file;
		char *method;	/* Default scaling method */
	} I;
	struct GRD2KML_L {	/* -L<size> */
		bool active;
		unsigned int size;
	} L;
	struct GRD2KML_S {	/* -S[n] */
		bool active;
		unsigned int extra;
	} S;
	struct  GRD2KML_T {	/* -T<title> */
		bool active;
		char *title;
	} T;
	struct  GRD2KML_W {	/* -W<contour_file> */
		bool active;
		char *file;
		double scale;	/* Scaling of pen width modifier */
		double cutoff;	/* Ignore contours whose pen is < this width in points */
	} W;
};

/* Structure used to keep track of which tile and its 4 possible underlings */
struct GMT_QUADTREE {
	unsigned int level, q, type;
	unsigned int row, col;
	char tag[16];
	char *region;
	double wesn[4];
	struct GMT_QUADTREE *next[4];
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRD2KML_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRD2KML_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->F.filter = 'g';
	C->I.method  = strdup ("t1");	/* Default normalization for shading when -I is used */
	C->L.size = 512;	/* Default tile size unless global grids [360] */
	C->W.scale = M_SQRT2;
	C->W.cutoff = 0.1;
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRD2KML_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->N.prefix);
	gmt_M_str_free (C->I.file);
	gmt_M_str_free (C->I.azimuth);
	gmt_M_str_free (C->I.method);
	gmt_M_str_free (C->T.title);
	gmt_M_str_free (C->W.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s %s -N<name> [-Aa|g|s[<altitude>]] [-C<cpt>] [-E<url>] [-F<filter>] "
		"[-H<scale>] [-I[<intensgrid>|<value>|<modifiers>]] [-L<size>] [-S[<extra>]] [-T<title>] [%s] "
		"[-W<contfile>|<pen>[+s<scl>/<limit>]] [%s] [%s] [%s]\n", name, GMT_INGRID, GMT_V_OPT, GMT_f_OPT, GMT_n_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<grid>");
	gmt_ingrid_syntax (API, 0, "Name of grid to be plotted");
	GMT_Usage (API, -2, "Note: Grid z-values are in user units and will be "
		"converted to colors via the CPT [%s].", API->GMT->current.setting.cpt);
	GMT_Usage (API, 1, "\n-N<name>");
	GMT_Usage (API, -2, "Set file name prefix for image directory and KML file. If the directory "
		"already exists we will overwrite the files.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-Aa|g|s[<altitude>]");
	GMT_Usage (API, -2, "Altitude mode of the image layer, choose among three modes:");
	GMT_Usage (API, 3, "a: Absolute altitude.");
	GMT_Usage (API, 3, "g: Altitude relative to sea surface or ground.");
	GMT_Usage (API, 3, "s: Altitude relative to sea floor or ground.");
	GMT_Usage (API, -2, "Optionally, append fixed <altitude> [g0: Clamped to sea surface or ground].");
	GMT_Usage (API, 1, "\n-C<cpt>");
	GMT_Usage (API, -2, "Color palette file to convert grid values to colors. Optionally, instead give name of a master cpt "
		"to automatically assign continuous colors over the data range [%s]; if so, "
		"optionally append +i<dz> to quantize the range [the exact grid range]. "
		"Another option is to specify -C<color1>,<color2>[,<color3>,...] to build a "
		"linear continuous cpt from those colors automatically.", API->GMT->current.setting.cpt);
	GMT_Usage (API, 1, "\n-E<url>");
	GMT_Usage (API, -2, "To store all files remotely, give leading URL [local files only].");
	GMT_Usage (API, 1, "\n-F<filter>");
	GMT_Usage (API, -2, "Specify filter type used for down-sampling (width is set automatically):");
	GMT_Usage (API, 3, "b: Boxcar - simple averaging of all points inside filter domain.");
	GMT_Usage (API, 3, "c: Cosine arch - weighted averaging with cosine arc weights.");
	GMT_Usage (API, 3, "g: Gaussian - weighted averaging with Gaussian weights [Default].");
	GMT_Usage (API, 3, "m: Median - median (50%% quantile) value of all points.");
	GMT_Usage (API, 1, "\n-H<scale>");
	GMT_Usage (API, -2, "Do sub-pixel smoothing using factor <scale> [no sub-pixel smoothing]. Ignored if -W not set.");
	GMT_Usage (API, 1, "\n-I[<intensgrid>|<value>|<modifiers>]");
	GMT_Usage (API, -2, "Apply directional illumination. Append name of intensity grid file. "
		" For a constant intensity (i.e., change the ambient light), append a single value. "
		"To derive intensities from <grid> instead, use -I+d to accept the default values (see grdgradient for details) or be specific:");
	GMT_Usage (API, 3, "+a Append <azimuth> of illumination [-45]");
	GMT_Usage (API, 3, "+n Append <method> pf intensity calculation [t1]");
	GMT_Usage (API, 1, "\n-L<size>");
	GMT_Usage (API, -2, "Set tile size as a power of 2 [512; for global grids, we instead select 360].n");
	GMT_Usage (API, 1, "\n-S[<extra>]");
	GMT_Usage (API, -2, "Add extra interpolated levels [no extra layers].");
	GMT_Usage (API, 1, "\n-T<title>");
	GMT_Usage (API, -2, "Set title (document description) for the top-level KML.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-W<contfile>|<pen>[+s<scl>/<limit>]");
	GMT_Usage (API, -2, "Give file with select contours and pens to overlay contours [no contours]. "
		"If no file is given we assume it is a <pen> and to use the contours implied by the CPT file. "
		"Pen widths apply at final tile resolution and are reduced by <scl> [1.1412] for each lower level. "
		"If a contour's scaled width is less than <limit> [0.1] it will not be drawn.");
	GMT_Option (API, "f,n,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRD2KML_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input files */
				if (n_files++ > 0) {n_errors++; continue; }
				Ctrl->In.active = true;
				if (opt->arg[0]) Ctrl->In.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file))) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Altitude mode */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				switch (opt->arg[0]) {
					case 'g': Ctrl->A.mode = 0; break;
					case 's': Ctrl->A.mode = 1; break;
					case 'a': Ctrl->A.mode = 2; break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -A: Append either a(bsolute), g(round) or s(eafloor) with optional <altitude>.\n");
						n_errors++;
						break;
				}
				if (opt->arg[1]) Ctrl->A.altitude = atof (&opt->arg[1]);
				break;
			case 'C':	/* CPT */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				gmt_cpt_interval_modifier (GMT, &(Ctrl->C.file), &(Ctrl->C.dz));
				break;
			case 'D':	/* Debug options - may fade away when happy with the performance */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				if (strstr (opt->arg, "+s")) Ctrl->D.single = true;	/* Write all files in a single directory instead of one directory per level */
				if (strstr (opt->arg, "+d")) Ctrl->D.dump = true;	/* Dump quadtree information to stdout */
				break;
			case 'E':	/* Remove URL for all contents but top driver kml */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				Ctrl->E.active = true;
				gmt_M_str_free (Ctrl->E.url);
				Ctrl->E.url = strdup (opt->arg);
				break;
			case 'F':	/* Select filter type */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				if (strchr ("bcgm", opt->arg[0]))
					Ctrl->F.filter = opt->arg[0];
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -F: Choose among b, c, g, m!\n");
					n_errors++;
				}
				break;
			case 'H':	/* RIP at a higher dpi, then downsample in gs */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->H.active);
				Ctrl->H.active = true;
				Ctrl->H.factor = atoi (opt->arg);
				break;
			case 'I':	/* Here, intensity must be a grid file since we need to filter it */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				if (!strcmp (opt->arg, "+d"))	/* Gave +d only, so derive intensities from input grid using default settings */
					Ctrl->I.derive = true;
				else if ((c = gmt_first_modifier (GMT, opt->arg, "an"))) {	/* Want to control how grdgradient is run */
					unsigned int pos = 0;
					char p[GMT_BUFSIZ] = {""};
					Ctrl->I.derive = true;
					while (gmt_getmodopt (GMT, 'I', c, "an", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'a': gmt_M_str_free (Ctrl->I.azimuth); Ctrl->I.azimuth = strdup (&p[1]); break;
							case 'n': gmt_M_str_free (Ctrl->I.method);  Ctrl->I.method  = strdup (&p[1]); break;
							default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
				}
				else if (!opt->arg[0] || strstr (opt->arg, "+"))	/* No argument or just +, so derive intensities from input grid using default settings */
					Ctrl->I.derive = true;
				else if (!gmt_access (GMT, opt->arg, R_OK))	/* Got a file */
					Ctrl->I.file = strdup (opt->arg);
				else if (gmt_M_file_is_remote (opt->arg))	/* Got a remote file */
					Ctrl->I.file = strdup (opt->arg);
				else if (opt->arg[0] && !gmt_not_numeric (GMT, opt->arg)) {	/* Looks like a constant value */
					Ctrl->I.value = atof (opt->arg);
					Ctrl->I.constant = true;
				}
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -I: Requires a valid grid file or a constant\n");
					n_errors++;
				}
				break;
			case 'L':	/* Tiles sizes */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.active = true;
				Ctrl->L.size = atoi (opt->arg);
				break;
			case 'N':	/* File name prefix */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				gmt_M_str_free (Ctrl->N.prefix);
				Ctrl->N.prefix = strdup (opt->arg);
				break;
			case 'Q':	/* Deprecated colormasking option */
				GMT_Report (API, GMT_MSG_ERROR, "Option -Q is deprecated as transparency is automatically detected\n");
				break;
			case 'S':	/* Extra levels */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.active = true;
				Ctrl->S.extra = (opt->arg[0]) ? atoi (opt->arg) : 1;
				break;
			case 'T':	/* Title */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				if (opt->arg[0]) Ctrl->T.title = strdup (opt->arg);
				break;
			case 'W':	/* Contours and pens */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				if ((c = strstr (opt->arg, "+s"))) {	/* Gave +s<scl> modifier to scale pen widths given via -C and optional cutoff pen width */
					sscanf (&c[2], "%lf/%lg", &Ctrl->W.scale, &Ctrl->W.cutoff);
					c[0] = '\0';	/* Temporarily chop off the modifier */
				}
				if (opt->arg[0]) {
					gmt_M_str_free (Ctrl->W.file);
					Ctrl->W.file = strdup (opt->arg);
				}
				if (c) c[0] = '+';	/* Restore */

				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Must specify a single grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->In.file == NULL, "Must specify a single grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.prefix == NULL, "Option -N: Must specify a prefix for naming usage.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->H.active && Ctrl->H.factor <= 1, "Option -H: Must specify an integer factor > 1.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->E.url == NULL, "Option -E: Must specify an URL.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && !Ctrl->I.constant && !Ctrl->I.file && !Ctrl->I.derive,
	                                 "Option -I: Must specify intensity file, value, or modifiers\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int grd2kml_find_quad_above (struct GMT_QUADTREE **Q, unsigned int n, unsigned int row, unsigned int col, unsigned int level) {
	/* Finds the quad entry that matches the row, col, level args */
	unsigned int k;
	for (k = 0; k < n; k++) {
		if (Q[k]->level != level) continue;
		if (Q[k]->row != row) continue;
		if (Q[k]->col != col) continue;
		return (int)k;
	}
	return -1;	/* Very bad */
}

GMT_LOCAL void grd2kml_set_dirpath (bool single, char *url, char *prefix, unsigned int level, int dir, char *string) {
	if (single) {	/* Write everything into the prefix dir */
		if (url && level == 0)	/* Set the leading URL for zero-level first kml */
			sprintf (string, "%s/%s/L%2.2d", url, prefix, level);
		else	/* Everything below is in same folder */
			sprintf (string, "L%2.2d", level);
	}
	else {	/* Write to separate level directories */
		if (url && level == 0)	/* Set the leading URL for zero-level first kml */
			sprintf (string, "%s/%s/%2.2d/", url, prefix, level);
		else if (dir == -1)	/* Need to refer to another directory at same level as this one */
			sprintf (string, "../%2.2d/", level);
		else if (dir == 0)	/* At current dir */
			string[0] = '\0';
		else	/* Down in a dir */
			sprintf (string, "%2.2d/", level);
	}
}

GMT_LOCAL void grd2kml_halve_dimensions (double *inc, double *step) {
	/* We MUST use division by two since it is a quadtree */
	*step /= 2;
	*inc  /= 2;
}

GMT_LOCAL unsigned int grd2kml_max_level (struct GMT_CTRL *GMT, bool global, struct GMT_GRID_HEADER *H, unsigned int size, unsigned int extra) {
	unsigned int level = 0;
	if (global) {
		unsigned int n = 1, f = 1, go = 1;
		double inc = 1.0, step = 360, range;
		GMT_Report (GMT->parent, GMT_MSG_NOTICE, "Level = %2.2d tile size = %gd grid inc = %gm n_tiles = %d\n", level, step, 60*inc, n);
		f = 2;
		do {
			step /= f;	inc /= f;
			n *= f;
			range = n * H->inc[GMT_X] * 360;
			if (range >= (360.0-GMT_CONV6_LIMIT))
				go = 0;
			level++;
			GMT_Report (GMT->parent, GMT_MSG_NOTICE, "Level = %2.2d tile size = %gd grid inc = %gm n_tiles = %d\n", level, step, 60*inc, n);
		} while (go);
		while (extra) {	/* Add the extra levels */
			step /= f;	inc /= f;
			n *= f;
			level++;
			GMT_Report (GMT->parent, GMT_MSG_NOTICE, "Level = %2.2d tile size = %gd grid inc = %gm n_tiles = %d\n", level, step, 60*inc, n);
			extra--;
		}
	}
	else {
		unsigned int mx, my;
		mx = urint (ceil ((double)H->n_columns / (double)size)) * size;	/* Nearest image size in multiples of tile size */
		my = urint (ceil ((double)H->n_rows / (double)size)) * size;
		level = urint (ceil (log2 (MAX (mx, my) / (double)size))) + extra;	/* Number of levels in the quadtree plus optional extra levels */
	}
	return level;
}

GMT_LOCAL void grd2kml_assert_tile_size (struct GMT_CTRL *GMT, bool global, bool active, unsigned int *size) {
	/* For global grids we select tile size of 360 given the 360-degree range of the file, else we use radix-2.
	 * We only change size if -L was not given, else we warn if the size is not ideal. */

	if (global) {	/* 360 degree range in longitude and <= 180 degree range in latitude */
		if (active && *size != 360)
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Option -L: For global grids the tile size should ideally be 360; your size %d is not.\n", *size);
		else if (!active) {
			*size = 360;
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Option -L: For global grids we select a tile size of %d\n", *size);
		}
	}
	else {	/* A smaller region, so strictly radix 2 size */
		if (active && fabs (log2 ((double)(*size)) - irint (log2 ((double)(*size)))) > GMT_CONV8_LIMIT)
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Option -L: Should ideally be radix 2; your size %d is not.\n", *size);
	}
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Final tile size is %d.\n", *size);
}

int grd2kml_coarsen_grid (struct GMT_CTRL *GMT, unsigned int level, char filter, unsigned int registration, double orig_inc, double inc, char *DataGrid, char *Zgrid, char *filt_report) {
	char s_int[GMT_LEN32] = {""}, fwidth[GMT_LEN32] = {""};
	char cmd[GMT_LEN256] = {""};
	int error;
	double f;
	/* We will filter the grid to make a coarser version that exactly matches the Ctrl->L.size expectations.
	 * Note: If the desired increment is smaller than the actual, we must sample the grid instead.
	 * Note: If gridline-registered we scale filter width by sqrt(2) but rounded up a bit to make sure
	 * it extends to the 4 corner nodes. */
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Level %d: Low-pass-filtering the grid(s)\n", level);
	f = (registration == GMT_GRID_NODE_REG) ? 1e-4*ceil (1e4*M_SQRT2) : 1.0;	/* Round up a bit */
	sprintf (fwidth, "%.8g", f * inc);
	sprintf (s_int, "%.16g", inc);
	if (inc < orig_inc) {	/* Resample original instead of filtering it down */
		sprintf (filt_report, " [Resampled with -I%s]", s_int);
		sprintf (cmd, "%s -I%s -rp -G%s", DataGrid, s_int, Zgrid);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Running grdsample : %s\n", cmd);
		if ((error = GMT_Call_Module (GMT->parent, "grdsample", GMT_MODULE_CMD, cmd)) != GMT_NOERROR)
			return (GMT_RUNTIME_ERROR);
	}
	else {	/* Filter the grid */
		unsigned int k = 0;
		char *kind[4] = {"Boxcar", "Cosine-taper", "Gaussian", "Median"};
		switch (filter) {
			case 'b':	k = 0; break;
			case 'c':	k = 1; break;
			case 'g':	k = 2; break;
			case 'm':	k = 3; break;
		}
		sprintf (filt_report, " [%s filtered with -F%c%s -I%s]", kind[k], filter, fwidth, s_int);
		sprintf (cmd, "%s -D0 -F%c%s -I%s -rp -G%s", DataGrid, filter, fwidth, s_int, Zgrid);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Running grdfilter : %s\n", cmd);
		if ((error = GMT_Call_Module (GMT->parent, "grdfilter", GMT_MODULE_CMD, cmd)) != GMT_NOERROR)
			return (GMT_RUNTIME_ERROR);
	}
	return (GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grd2kml (void *V_API, int mode, void *args) {
	bool use_tile = false, z_extend = false, i_extend = false, tmp_cpt = false, global_lon, adjust = false;

	int error = 0, kk, uniq, i_dir, dir, minLodPixels, maxLodPixels;

	unsigned int level, max_level, dpi = 100, n = 0, k, nx, ny, row, col, n_skip, quad, im_type;
	unsigned int n_NaN, n_NM, n_alloc = GMT_CHUNK, n_bummer = 0, n_tiles = 0, n_img[2] = {0,0};
	unsigned int registration;

	uint64_t node;

	double factor = 2.0, dim, step, wesn[4], ext_wesn[4], inc;

	char cmd[GMT_BUFSIZ] = {""}, level_dir[PATH_MAX] = {""}, Zgrid[PATH_MAX] = {""}, Igrid[PATH_MAX] = {""};
	char W[GMT_LEN16] = {""}, E[GMT_LEN16] = {""}, S[GMT_LEN16] = {""}, N[GMT_LEN16] = {""}, file[PATH_MAX] = {""};
	char DataGrid[PATH_MAX] = {""}, IntensGrid[PATH_MAX] = {""}, path[PATH_MAX] = {""}, filt_report[GMT_LEN128] = {""};
	char region[GMT_LEN128] = {""}, ps_cmd[GMT_LEN128] = {""}, contour_file[GMT_VF_LEN] = {""}, K[4] = {""};
	char box[GMT_LEN32] = {""}, grdimage[GMT_LEN256] = {""}, grdcontour[GMT_LEN256] = {""}, scalepen_arg[GMT_LEN32] = {""};

	static char *kml_xmlns = "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">";
	static char *alt_mode[3] = {"relativeToGround", "relativeToSeaFloor", "absolute"};
	static char *ext[2] = {"jpg", "png"}, img_code[2] = {'j', 'G'}, *transp = " -Q";

	FILE *fp = NULL;

	struct GMT_DATASET *C = NULL;
	struct GMT_QUADTREE **Q = NULL;
	struct GRD2KML_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL, *T = NULL;
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

	/*---------------------------- This is the grd2kml main code ----------------------------*/

	gmt_grd_set_datapadding (GMT, true);	/* Turn on gridpadding when reading a subset */

	uniq = (int)getpid();	/* Unique number for temporary files  */

	/* Read grid header only to determine dimensions and required levels for the Pyramid */
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (!gmt_M_is_geographic (GMT, GMT_IN)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid must be geographic (lon, lat)\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (!gmt_M_grd_equal_xy_inc (GMT, G)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid spacing must be the same in longitude and latitude!\n");
		Return (GMT_RUNTIME_ERROR);
	}

	global_lon = gmt_M_360_range (G->header->wesn[XLO], G->header->wesn[XHI]);

	if (!global_lon && G->header->registration == GMT_GRID_NODE_REG) {
		/* It is better to have a pixel-registered grid for tiling so that we can use the highest resolution exactly at the highest level.
		 * We will convert input to a fake pixel grid by extending its domain by half the increments. No resampling takes place here. */
		GMT_Destroy_Data (API, &G);	/* Delete previous grid struct */
		sprintf (file, "%s/grd2kml_pixeldata_tmp_%6.6d.grd", API->tmp_dir, uniq);
		sprintf (cmd, "%s -T -G%s", Ctrl->In.file, file);	/* Toggle registration */
		GMT_Report (API, GMT_MSG_INFORMATION, "Convert data grid to pixel orientation\n");
		if ((error = GMT_Call_Module (API, "grdedit", GMT_MODULE_CMD, cmd)) != GMT_NOERROR) {
			Return (GMT_RUNTIME_ERROR);
		}
		adjust = true;	/* Since may need to do the same for an intensity grid */
		gmt_M_str_free (Ctrl->In.file);
		Ctrl->In.file = strdup (file);
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);
		}
	}

	grd2kml_assert_tile_size (GMT, global_lon, Ctrl->L.active, &Ctrl->L.size);	/* Set or comment on tile size */

	max_level = grd2kml_max_level (GMT, global_lon, G->header, Ctrl->L.size, Ctrl->S.extra);

	nx = G->header->n_columns;	ny = (global_lon) ? nx : G->header->n_rows;	/* Dimensions of original grid, possibly made square for global grids */

	dim = dpi * 0.0001 * Ctrl->L.size;	/* Constant tile map size in inches for a fixed dpi of 100 yields PNGS of the requested dimension in -L */

	/* Create the container quadtree directory first */
	if (gmt_mkdir (Ctrl->N.prefix))
		GMT_Report (API, GMT_MSG_INFORMATION, "Directory %s already exist - will overwrite files\n", Ctrl->N.prefix);

	if (Ctrl->I.derive) {	/* Auto-create single intensity grid from (possibly pixel-reregistered) data grid to ensure constant scaling */
		sprintf (file, "%s/grd2kml_intensity_tmp_%6.6d.grd", API->tmp_dir, uniq);
		Ctrl->I.file = strdup (file);
		GMT_Report (API, GMT_MSG_INFORMATION, "Derive an intensity grid from data grid\n");
		/* Prepare the grdgradient arguments using selected -A -N and the data region in effect */
		sprintf (cmd, "%s -G%s -A%s -N%s --GMT_HISTORY=readonly", Ctrl->In.file, Ctrl->I.file, Ctrl->I.azimuth, Ctrl->I.method);
		/* Call the grdgradient module */
		GMT_Report (API, GMT_MSG_INFORMATION, "Calling grdgradient with args %s\n", cmd);
		if (GMT_Call_Module (API, "grdgradient", GMT_MODULE_CMD, cmd))
			Return (API->error);
	}
	else if (Ctrl->I.file) {	/* Can read the intensity file and compare region etc. to data grid */
		struct GMT_GRID *I = NULL;
		double x_noise = GMT_CONV4_LIMIT * G->header->inc[GMT_X], y_noise = GMT_CONV4_LIMIT * G->header->inc[GMT_Y];
		if (adjust) {	/* Also make intensity grid pixel-registered */
			sprintf (file, "%s/grd2kml_pixelintens_tmp_%6.6d.grd", API->tmp_dir, uniq);
			sprintf (cmd, "%s -T -G%s", Ctrl->I.file, file);
			GMT_Report (API, GMT_MSG_INFORMATION, "Convert intensity grid to pixel orientation\n");
			if ((error = GMT_Call_Module (API, "grdedit", GMT_MODULE_CMD, cmd)) != GMT_NOERROR) {
				Return (GMT_RUNTIME_ERROR);
			}
			gmt_M_str_free (Ctrl->I.file);
			Ctrl->I.file = strdup (file);
		}
		if ((I = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->I.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (!gmt_M_grd_same_shape (GMT, G, I)) {
			GMT_Report (API, GMT_MSG_ERROR, "Data grid and intensity grid are not of the same dimensions!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (fabs (G->header->wesn[XLO] - I->header->wesn[XLO]) > x_noise || fabs (G->header->wesn[XHI] - I->header->wesn[XHI]) > x_noise ||
			fabs (G->header->wesn[YLO] - I->header->wesn[YLO]) > y_noise || fabs (G->header->wesn[YHI] - I->header->wesn[YHI]) > y_noise) {
			GMT_Report (API, GMT_MSG_ERROR, "Data grid and intensity grid do not cover the same area!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Destroy_Data (API, &I);
	}

	Q = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_QUADTREE *);

	/* Determine extended region required if using the largest multiple of original grid spacing */

	registration = G->header->registration;
	if (global_lon) {	/* Make it a square 360x360 grid so the quadtree splitting works */
		ext_wesn[XLO] = G->header->wesn[XLO];
		ext_wesn[XHI] = G->header->wesn[XHI];
		ext_wesn[YLO] = -180.0;
		ext_wesn[YHI] = +180.0;
		step = 360.0;	/* Initial single tile is 360x360 degrees with increment 1 degree */
		inc = 1.0;
	}
	else {	/* Make a square Ctrl->size region centered on the input grid center */
		double tile_size = Ctrl->L.size * G->header->inc[GMT_X];	/* Dimension of highest-resolution tile in degrees */
		unsigned int width;

		width = pow (2.0, max_level);	/* Radix-2 number of the smallest tiles in both x and y */
		step = width * tile_size;		/* Square dimension of extended grid in degrees */
		col = G->header->n_columns / 2;	row = G->header->n_rows / 2;	/* Half-way point in grid */
		/* Extend the grid half-step away from the approximate center of the input grid */
		ext_wesn[XLO] = gmt_M_grd_col_to_x (GMT, col, G->header);
		if (registration == GMT_GRID_PIXEL_REG) ext_wesn[XLO] -= 0.5 * G->header->inc[GMT_X];	/* Must adjust since we don't want node location but cell boundary */
		ext_wesn[XLO] -= step / 2;
		ext_wesn[XHI] = ext_wesn[XLO] + step;
		ext_wesn[YLO] = gmt_M_grd_row_to_y (GMT, row, G->header);
		if (registration == GMT_GRID_PIXEL_REG) ext_wesn[YLO] -= 0.5 * G->header->inc[GMT_Y];	/* Must adjust since we don't want node location but cell boundary */
		ext_wesn[YLO] -= step / 2;
		ext_wesn[YHI] = ext_wesn[YLO] + step;
		inc = step / Ctrl->L.size;
		nx = ny = width * Ctrl->L.size;
	}
	if (ext_wesn[XLO] < G->header->wesn[XLO] || ext_wesn[XHI] > G->header->wesn[XHI] || ext_wesn[YLO] < G->header->wesn[YLO] || ext_wesn[YHI] > G->header->wesn[YHI]) {
		/* Extend the original grid with NaNs so it is an exact multiple of largest grid stride at max level */
		sprintf (DataGrid, "%s/grd2kml_extended_data_%6.6d.grd", API->tmp_dir, uniq);
		sprintf (cmd, "%s -R%.16g/%.16g/%.16g/%.16g -N -G%s", Ctrl->In.file, ext_wesn[XLO], ext_wesn[XHI], ext_wesn[YLO], ext_wesn[YHI], DataGrid);
		GMT_Report (API, GMT_MSG_INFORMATION, "Extend original data grid to multiple of largest grid spacing\n");
		if ((error = GMT_Call_Module (API, "grdcut", GMT_MODULE_CMD, cmd)) != GMT_NOERROR) {
			gmt_M_free (GMT, Q);
			Return (GMT_RUNTIME_ERROR);
		}
		z_extend = true;	/* We made a temp file we need to zap later */
		if (Ctrl->I.active) {	/* Also extend the intensity grid */
			sprintf (IntensGrid, "%s/grd2kml_extended_intens_%6.6d.grd", API->tmp_dir, uniq);
			sprintf (cmd, "%s -R%.16g/%.16g/%.16g/%.16g -N -G%s", Ctrl->I.file, ext_wesn[XLO], ext_wesn[XHI], ext_wesn[YLO], ext_wesn[YHI], IntensGrid);
			GMT_Report (API, GMT_MSG_INFORMATION, "Extend intensity grid to multiple of largest grid spacing\n");
			if ((error = GMT_Call_Module (API, "grdcut", GMT_MODULE_CMD, cmd)) != GMT_NOERROR) {
				gmt_M_free (GMT, Q);
				Return (GMT_RUNTIME_ERROR);
			}
			i_extend = true;	/* We made a temp file we need to zap later */
		}
		if (ext_wesn[YLO] < -90.0 || ext_wesn[YHI] > 90.0) gmt_grd_set_cartesian (GMT, G->header, 2);	/* We must treat the extended grid as Cartesian since exceeding latitude bounds */
	}
	else {	/* No need to extend, use the input files as is */
		strcpy (DataGrid, Ctrl->In.file);
		if (Ctrl->I.active)
			strcpy (IntensGrid, Ctrl->I.file);
	}

	if (!Ctrl->C.active || gmt_is_cpt_master (GMT, Ctrl->C.file)) {	/* If no cpt given or just a master then we must compute a scaled one from the full-size grid and use it throughout */
		char *cpt = gmt_cpt_default (API, Ctrl->C.file, Ctrl->In.file);
		char cptfile[PATH_MAX] = {""};
		struct GMT_PALETTE *P = NULL;
		if ((P = gmt_get_palette (GMT, cpt, GMT_CPT_OPTIONAL, G->header->z_min, G->header->z_max, Ctrl->C.dz)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Failed to create a CPT\n");
			Return (API->error);	/* Well, that did not go well... */
		}
		if (cpt) gmt_M_str_free (cpt);
		sprintf (cptfile, "%s/grd2kml_%d.cpt", API->tmp_dir, uniq);
		if (GMT_Write_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_NORMAL, NULL, cptfile, P) != GMT_NOERROR) {
			Return (API->error);
		}
		Ctrl->C.active = tmp_cpt = true;
		Ctrl->C.file = strdup (cptfile);
	}

	if (Ctrl->W.active) {	/* Want to overlay contours given via file */
	/* Contour file has records of <cvalue> [<angle>] C|A [<pen>].  With angle = 0 we want <cvalue> C <pen> records */
		uint64_t c;
		char line[GMT_LEN256] = {""};
		struct GMT_DATASEGMENT *S = NULL;

		if (!gmt_access (GMT, Ctrl->W.file, F_OK)) {	/* Was given an actual file */
			gmt_set_cartesian (GMT, GMT_IN);
			if ((error = GMT_Set_Columns (API, GMT_IN, 1, GMT_COL_FIX)) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to specify number of columns (1) to read\n");
				gmt_M_free (GMT, Q);
				Return (GMT_RUNTIME_ERROR);
			}
			if ((C = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->W.file, NULL)) == NULL) {
				gmt_M_free (GMT, Q);
				Return (GMT_RUNTIME_ERROR);
			}
			if (C->n_segments > 1 || C->n_records == 0 || C->table[0]->segment[0]->text == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Contour file has more than one segment, no records at all, or no text\n");
				gmt_M_free (GMT, Q);
				Return (GMT_RUNTIME_ERROR);
			}
			S = C->table[0]->segment[0];
			for (c = 0; c < C->n_records; c++) {	/* Must reformat the records to fit grdcontour requirements */
				if (S->text[c] == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "No text record found\n");
					gmt_M_free (GMT, Q);
					Return (GMT_RUNTIME_ERROR);
				}
				sprintf (line, "C %s", S->text[c]);	/* Build the required record format for grdcontour */
				gmt_M_str_free (S->text[c]);	/* Free previous string */
				S->text[c] = strdup (line);	/* Update string */
			}
		}
		else {	/* Use contours from CPT file, with -W<pen> */
			struct GMT_PALETTE *P = NULL;
			uint64_t dim_c[4] = {1, 1, 0, 1};
			if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
				gmt_M_free (GMT, Q);
				Return (API->error);
			}
			dim_c[GMT_ROW] = P->n_colors + 1;	/* Number of contours implied by CPT */
			if ((C = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_WITH_STRINGS, dim_c, NULL, NULL, 0, 0, NULL)) == NULL) {
				gmt_M_free (GMT, Q);
				Return (GMT_RUNTIME_ERROR);
			}
			S = C->table[0]->segment[0];
			sprintf (line, "C %s", Ctrl->W.file);	/* Build the required record format for grdcontour */
			for (c = 0; c < P->n_colors; c++) {	/* Do all the low boundaries */
				S->data[GMT_X][c] = P->data[c].z_low;
				gmt_M_str_free (S->text[c]);	/* Free previous string */
				S->text[c] = strdup (line);	/* Update string */
			}
			S->data[GMT_X][c] = P->data[P->n_colors-1].z_high;
			gmt_M_str_free (S->text[c]);	/* Free previous string */
			S->text[c] = strdup (line);	/* Update string */
		}
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN|GMT_IS_REFERENCE, C, contour_file) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual file for contours\n");
			gmt_M_free (GMT, Q);
			GMT_Destroy_Data (API, &C);
			Return (GMT_RUNTIME_ERROR);
		}
		strcpy (K, " -K");	/* Since now we must do a contour overlay */
	}

	/* Set up the constant parts of the grdimage command */
	sprintf (grdimage, "-JX%3.2lfi -X0 -Y0%s -W -Ve --PS_MEDIA=%3.2lfix%3.2lfi", dim, K, dim, dim);
	if (Ctrl->C.active) { strcat (grdimage, " -C"); strcat (grdimage, Ctrl->C.file); }
	/* Set up the constant parts of the grdcontour command */
	if (Ctrl->W.active)	/* Overlay contours */
		sprintf (grdcontour, "-JX%3.2lfi -O -C%s -Ve", dim, contour_file);

	if (Ctrl->H.active)	/* Do sub-pixel smoothing */
		sprintf (ps_cmd, "-E100 -P -Ve -Z -H%d", Ctrl->H.factor);
	else
		sprintf (ps_cmd, "-E100 -P -Ve -Z");

	GMT_Report (GMT->parent, GMT_MSG_NOTICE, "Extended grid size is by %d by %d, tile size is %d x %d\n", ny, nx, Ctrl->L.size, Ctrl->L.size);
	n_NM = Ctrl->L.size * Ctrl->L.size;	/* Number of nodes or pixels in a tile */

	/* Loop over all the levels, starting at the top level (0) */
	for (level = 0; level <= max_level; level++) {
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Level %d: Factor = %g Dim = %d x %d -> %d x %d\n",
			level, factor, irint (factor * Ctrl->L.size), irint (factor * Ctrl->L.size), Ctrl->L.size, Ctrl->L.size);
		/* Create the level directory */
		if (!Ctrl->D.single) {
			sprintf (level_dir, "%s/%2.2d", Ctrl->N.prefix, level);
			if (gmt_mkdir (level_dir))
				GMT_Report (API, GMT_MSG_INFORMATION, "Level directory %s already exist - overwriting files\n", level_dir);
		}
		if (level < max_level || registration == GMT_GRID_NODE_REG || !doubleAlmostEqual (inc, G->header->inc[GMT_X])) {	/* Filter the data to match level resolution */
			sprintf (Zgrid, "%s/grd2kml_Z_L%d_tmp_%6.6d.grd", API->tmp_dir, level, uniq);
			if (grd2kml_coarsen_grid (GMT, level, Ctrl->F.filter, registration, G->header->inc[GMT_X], inc, DataGrid, Zgrid, filt_report)) {
				gmt_M_free (GMT, Q);
				if (Ctrl->W.active) GMT_Destroy_Data (API, &C);
				Return (GMT_RUNTIME_ERROR);
			}
			if (Ctrl->I.active) {	/* Also filter the intensity grid */
				sprintf (Igrid, "%s/grd2kml_I_L%d_tmp_%6.6d.grd", API->tmp_dir, level, uniq);
				if (grd2kml_coarsen_grid (GMT, level, Ctrl->F.filter, registration, G->header->inc[GMT_X], inc, IntensGrid, Igrid, filt_report)) {
					gmt_M_free (GMT, Q);
					if (Ctrl->W.active) GMT_Destroy_Data (API, &C);
					Return (GMT_RUNTIME_ERROR);
				}
			}
		}
		else {	/* Use as is for the highest resolution */
			sprintf (filt_report, " [Original grid used]");
			strcpy (Zgrid, DataGrid);
			step = Ctrl->L.size * G->header->inc[GMT_X];
			if (Ctrl->I.active) strcpy (Igrid, IntensGrid);
		}
		/* Loop over all rows at this level */
		row = col = n_skip = 0;
		wesn[YLO] = ext_wesn[YLO];
		gmt_ascii_format_one (GMT, S, wesn[YLO], GMT_IS_FLOAT);

		if (Ctrl->W.active) {		/* Set pen width scale and overall cutoff pen size [0.1] in points */
			double p = pow (Ctrl->W.scale, -(double)(max_level - level));
			sprintf (scalepen_arg, " -W+s%g/%g", p, Ctrl->W.cutoff);
		}
		while (wesn[YLO] < (G->header->wesn[YHI]-G->header->inc[GMT_Y])) {	/* Small correction to avoid issues due to round-off */
			wesn[YHI] = wesn[YLO] + step;	/* Top row may extend beyond grid and be transparent */
			gmt_ascii_format_one (GMT, N, wesn[YHI], GMT_IS_FLOAT);	/* GMT_IS_FLOAT and not GMT_IS_LAT since we may exceed 90 */
			if (wesn[YHI] <= G->header->wesn[YLO] || wesn[YLO] >= G->header->wesn[YHI]) {	/* Tile row outside data range */
				row++;	/* Onwards to next row */
				wesn[YLO] = wesn[YHI];
				strcpy (S, N);
				n_skip++;
				continue;
			}
			/* Loop over all columns at this level */
			col = 0;
			wesn[XLO] = ext_wesn[XLO];
			gmt_ascii_format_one (GMT, W, wesn[XLO], GMT_IS_FLOAT);
			while (wesn[XLO] < (G->header->wesn[XHI]-G->header->inc[GMT_X])) {	/* Small correction to avoid issues due to round-off */
				unsigned int trow, tcol;
				wesn[XHI] = wesn[XLO] + step;	/* So right column may extend beyond grid and be transparent */
				gmt_ascii_format_one (GMT, E, wesn[XHI], GMT_IS_FLOAT);
				if (wesn[XHI] <= G->header->wesn[XLO] || wesn[XLO] >= G->header->wesn[XHI]) {	/* Tile outside x data range */
					col++;	/* Onwards to next column */
					wesn[XLO] = wesn[XHI];
					strcpy (W, E);
					n_skip++;
					continue;
				}
				/* Now we have the current tile region */
				if ((T = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, wesn, Zgrid, NULL)) == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to read in grid tile!\n");
					gmt_M_free (GMT, Q);
					if (Ctrl->W.active) GMT_Destroy_Data (API, &C);
					Return (API->error);
				}
				/* Determine if we have any non-NaN data points inside this grid */
				for (trow = n_NaN = 0; trow < T->header->n_rows; trow++) {
					for (tcol = 0; tcol < T->header->n_columns; tcol++) {
						node = gmt_M_ijp (T->header, trow, tcol);
						if (gmt_M_is_fnan (T->data[node])) n_NaN++;
					}
				}
				use_tile = (n_NaN < n_NM);
				im_type = (n_NaN) ? 1 : 0;
				if (use_tile) {	/* Found data inside this tile, make plot and rasterize (if PostScript) */
					char z_data[GMT_VF_LEN] = {""}, psfile[PATH_MAX] = {""};
					/* Open the grid subset as a virtual file we can pass to grdimage */
					if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN|GMT_IS_REFERENCE, T, z_data) == GMT_NOTSET) {
						GMT_Report (API, GMT_MSG_ERROR, "Unable to open grid tile as virtual file!\n");
						gmt_M_free (GMT, Q);
						if (Ctrl->W.active) GMT_Destroy_Data (API, &C);
						Return (API->error);
					}
					/* Will pass -W so grdimage will notify us if there was no valid image data imaged */
					if (!Ctrl->W.active) {
						/* Build the grdimage command to write an image directly via GDAL - no need to go via PostSCript */
						char imagefile[PATH_MAX] = {""};
						if (Ctrl->D.single)
							sprintf (imagefile, "%s/L%2.2dR%3.3dC%3.3d.%s", Ctrl->N.prefix, level, row, col, ext[im_type]);
						else
							sprintf (imagefile, "%s/R%3.3dC%3.3d.%s", level_dir, row, col, ext[im_type]);
						if (Ctrl->I.active)	/* Must pass two grids */
							sprintf (cmd, "%s %s -I%s -R%s/%s/%s/%s -A%s", grdimage, z_data, Igrid, W, E, S, N, imagefile);
						else
							sprintf (cmd, "%s %s -R%s/%s/%s/%s -A%s", grdimage, z_data, W, E, S, N, imagefile);
						if (im_type) strcat (cmd, transp);
						error = GMT_Call_Module (API, "grdimage", GMT_MODULE_CMD, cmd);
						if (!(error == GMT_NOERROR || error == GMT_IMAGE_NO_DATA)) {
							GMT_Report (API, GMT_MSG_ERROR, "Unable to create a direct PNG from grid\n");
							gmt_M_free (GMT, Q);
							Return (API->error);
						}
					}
					else {
						/* Build the grdimage command to make the PostScript plot */
						sprintf (psfile, "%s/grd2kml_tile_tmp_%6.6d.ps", API->tmp_dir, uniq);
						if (Ctrl->I.active)	/* Must pass two grids */
							sprintf (cmd, "%s %s -I%s -R%s/%s/%s/%s ->%s", grdimage, z_data, Igrid, W, E, S, N, psfile);
						else
							sprintf (cmd, "%s %s -R%s/%s/%s/%s ->%s", grdimage, z_data, W, E, S, N, psfile);
						if (im_type) strcat (cmd, transp);
						error = GMT_Call_Module (API, "grdimage", GMT_MODULE_CMD, cmd);
						if (error == GMT_NOERROR && Ctrl->W.active) {	/* Overlay contours */
							sprintf (cmd, "%s %s -R%s/%s/%s/%s %s ->>%s", grdcontour, z_data, W, E, S, N, scalepen_arg, psfile);
							GMT_Init_VirtualFile (API, 0, z_data);	/* Read the same grid again */
							GMT_Init_VirtualFile (API, 0, contour_file);	/* Read the same contours again */
							if ((error = GMT_Call_Module (API, "grdcontour", GMT_MODULE_CMD, cmd))) {
								GMT_Report (API, GMT_MSG_ERROR, "Unable to overlay contours!\n");
								gmt_M_free (GMT, Q);
								GMT_Destroy_Data (API, &C);
								Return (API->error);
							}
						}
					}
					GMT_Close_VirtualFile (API, z_data);
					if (GMT_Destroy_Data (API, &T) != GMT_NOERROR) {
						GMT_Report (API, GMT_MSG_ERROR, "Unable to free memory of grid tile!\n");
						gmt_M_free (GMT, Q);
						Return (API->error);
					}
					if (error == GMT_IMAGE_NO_DATA) {	/* Must have found non-NaNs in the pad since the image is all NaN? */
						GMT_Report (API, GMT_MSG_WARNING, "No image content for current tile (%d, %d, %d) - skipped\n", level, row, col);
						gmt_remove_file (GMT, psfile);
						n_bummer++;
					}
					else {	/* Made a meaningful plot, time to rip. */
						/* Create the psconvert command to convert the PS to transparent PNG */
						sprintf (region, "%s/%s/%s/%s", W, E, S, N);
						GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Level %2.2d: Mapped tile %s\n", level, region);
						if (Ctrl->W.active) {	/* Make PS, now rasterize to PNG */
							if (Ctrl->D.single)
								sprintf (cmd, "%s -T%c -D%s -FL%2.2dR%3.3dC%3.3d %s", ps_cmd, img_code[im_type], Ctrl->N.prefix, level, row, col, psfile);
							else
								sprintf (cmd, "%s -T%c -D%s -FR%3.3dC%3.3d %s", ps_cmd, img_code[im_type], level_dir, row, col, psfile);
							if (GMT_Call_Module (API, "psconvert", GMT_MODULE_CMD, cmd)) {
								GMT_Report (API, GMT_MSG_ERROR, "Unable to rasterize current PNG tile!\n");
								gmt_M_free (GMT, Q);
								Return (API->error);
							}
						}
						/* Update our list of tiles created */
						Q[n] = gmt_M_memory (GMT, NULL, 1, struct GMT_QUADTREE);
						Q[n]->row = row; Q[n]->col = col;	Q[n]->level = level;	Q[n]->type = im_type;
						Q[n]->wesn[XLO] = wesn[XLO];	Q[n]->wesn[XHI] = wesn[XHI];
						Q[n]->wesn[YLO] = wesn[YLO];	Q[n]->wesn[YHI] = wesn[YHI];
						sprintf (Q[n]->tag, "L%2.2dR%3.3dC%3.3d", level, row, col);
						Q[n]->region = strdup (region);
						if (++n == n_alloc) {	/* Extend the array */
							n_alloc <<= 1;
							Q = gmt_M_memory (GMT, Q, n_alloc, struct GMT_QUADTREE *);
						}
						n_img[im_type]++;
					}
				}
				else {	/* Just NaNs inside this tile */
					GMT_Report (API, GMT_MSG_INFORMATION, "Level %2.2d: Tile %s/%s/%s/%s had no data - skipped\n", level, W, E, S, N);
					n_skip++;
				}
				col++;	/* Onwards to next column */
				wesn[XLO] = wesn[XHI];
				strcpy (W, E);
			}
			row++;	/* Onwards to next row */
			wesn[YLO] = wesn[YHI];
			strcpy (S, N);
		}
		if (step > 1)
			sprintf (box, "%g x %g d", step, step);
		else
			sprintf (box, "%g x %g m", 60*step, 60*step);
		GMT_Report (GMT->parent, GMT_MSG_NOTICE, "Level %2.2d: Tile size: %18s Tiles: %3d by %3d = %5d %5d mapped %3d empty%s\n", level, box, row, col, row*col, row*col - n_skip, n_skip, filt_report);
		if (level < max_level) {	/* Delete the temporary filtered grid(s) */
			gmt_remove_file (GMT, Zgrid);
			if (Ctrl->I.active) gmt_remove_file (GMT, Igrid);
		}
		grd2kml_halve_dimensions (&inc, &step);
	}
	n_tiles = n;

	if (n_bummer)
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Found %d tiles that passed the no-NaN test but gave a blank image (?)\n", n_bummer);

	/* Clean up any temporary files */

	if (z_extend && !access (DataGrid, F_OK))
		gmt_remove_file (GMT, DataGrid);
	if (i_extend && !access (IntensGrid, F_OK))
		gmt_remove_file (GMT, IntensGrid);
	if (Ctrl->I.derive)
		gmt_remove_file (GMT, Ctrl->I.file);
	if (tmp_cpt)
		gmt_remove_file (GMT, Ctrl->C.file);

	/* Process quadtree links */

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Processes quadtree links for %d tiles.\n", n);
	Q = gmt_M_memory (GMT, Q, n, struct GMT_QUADTREE *);	/* Final size */
	for (level = max_level; level > 0; level--) {
		for (k = 0; k < n; k++) {
			if (Q[k]->level != level) continue;	/* Only deal with this level here */
			/* Determine the parent tile and the quad (0-3) we belong to */
			/* This is the parent row and col since we increase by a factor of 2 each time */
			row = Q[k]->row / 2;	col = Q[k]->col / 2;
			/* The quad is given by comparing the high and low values of row, col */
			quad = 2 * (Q[k]->row - 2 * row) + (Q[k]->col - 2 * col);
			kk = grd2kml_find_quad_above (Q, n, row, col, level-1);	/* kk is the parent of k */
			if (kk < 0) {	/* THis can happen when the lower-level tile grazes one above it but there really are no data involved */
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Tile %s: Unable to link tile for row = %d, col = %d at level %d to a parent (!?).  Probably empty - skipped.\n", Q[k]->tag, row, col, level);
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Tile %s: Region was %g/%g/%g/%g.\n", Q[k]->tag, Q[k]->wesn[XLO], Q[k]->wesn[XHI], Q[k]->wesn[YLO], Q[k]->wesn[YHI]);
				continue;
			}
			assert (quad < 4);	/* Sanity check */
			Q[kk]->next[quad] = Q[k];	/* Do the linking */
			Q[kk]->q++;			/* Count the links for this parent */
		}
	}

	/* Create all the KML files in the quadtree with their links down the tree */

	for (k = 0; k < n; k++) {
		if (Ctrl->D.dump) {
			printf ("%s [%s]:\t", Q[k]->tag, Q[k]->region);
			for (quad = 0; quad < 4; quad++) {
				if (Q[k]->next[quad] == NULL) continue;
				minLodPixels = (Q[k]->next[quad]->level == 0) ? 1 : Ctrl->L.size / 2;
				maxLodPixels = (Q[k]->next[quad]->level == max_level) ? -1 : 4*Ctrl->L.size;
				printf (" %c=%s [%d/%d]", 'A'+quad, Q[k]->next[quad]->tag, minLodPixels, maxLodPixels);
			}
			printf ("\n");
		}
		if (Q[k]->level == 0)
			sprintf (file, "%s/%s.kml", Ctrl->N.prefix, Ctrl->N.prefix);
		else if (Ctrl->D.single)
			sprintf (file, "%s/L%2.2dR%3.3dC%3.3d.kml", Ctrl->N.prefix, Q[k]->level, Q[k]->row, Q[k]->col);
		else
			sprintf (file, "%s/%2.2d/R%3.3dC%3.3d.kml", Ctrl->N.prefix, Q[k]->level, Q[k]->row, Q[k]->col);
		if ((fp = fopen (file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create file : %s\n", file);
			Return (GMT_RUNTIME_ERROR);
		}
		/* First this tile's kml info and image href */
		minLodPixels = (Q[k]->level == 0) ? 1 : Ctrl->L.size / 2;
		maxLodPixels = (Q[k]->level == max_level) ? -1 : 4*Ctrl->L.size;
		grd2kml_set_dirpath (Ctrl->D.single, NULL, Ctrl->N.prefix, Q[k]->level, 1, path);
		fprintf (fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n%s\n", kml_xmlns);
		if (Q[k]->level == 0) {
			char *cmd_args = GMT_Create_Cmd (API, options);
			fprintf (fp, "  <Document>\n    <name>%s</name>\n", Ctrl->N.prefix);
			fprintf (fp, "  <!-- Produced by the Generic Mapping Tools [www.generic-mapping-tools.org] -->\n");
			fprintf (fp, "  <!-- cmd: gmt %s %s -->\n", GMT->init.module_name, cmd_args);
			gmt_M_free (GMT, cmd_args);
			dir = 0;
			if (Ctrl->T.active)
				fprintf (fp, "    <description>%s</description>\n\n", Ctrl->T.title);
			else
				fprintf (fp, "    <description>GMT image quadtree representation of %s</description>\n\n", Ctrl->In.file);
		}
		else {
			fprintf (fp, "  <Document>\n    <name>%s.kml</name>\n", Q[k]->tag);
			fprintf (fp, "    <description></description>\n\n");
			dir = 1;
		}
		fprintf (fp, "    <Style>\n");
		fprintf (fp, "      <ListStyle id=\"hideChildren\">\n");
		fprintf (fp, "      <listItemType>checkHideChildren</listItemType>\n");
		fprintf (fp, "    </ListStyle>\n");
		fprintf (fp, "    </Style>\n");
		fprintf (fp, "    <Region>\n      <LatLonAltBox>\n");
		fprintf (fp, "        <north>%.14g</north>\n", Q[k]->wesn[YHI]);
		fprintf (fp, "        <south>%.14g</south>\n", Q[k]->wesn[YLO]);
		fprintf (fp, "        <east>%.14g</east>\n",   Q[k]->wesn[XHI]);
		fprintf (fp, "        <west>%.14g</west>\n",   Q[k]->wesn[XLO]);
		fprintf (fp, "      </LatLonAltBox>\n");
		fprintf (fp, "      <Lod>\n");
		fprintf (fp, "        <minLodPixels>%d</minLodPixels>\n", minLodPixels);
		fprintf (fp, "        <maxLodPixels>%d</maxLodPixels>\n", maxLodPixels);
		fprintf (fp, "      </Lod>\n");
		fprintf (fp, "    </Region>\n");
		fprintf (fp, "    <GroundOverlay>\n");
		fprintf (fp, "      <drawOrder>%d</drawOrder>\n", 10+2*Q[k]->level);
		grd2kml_set_dirpath (Ctrl->D.single, NULL, Ctrl->N.prefix, Q[k]->level, 1-dir, path);
		fprintf (fp, "      <Icon>\n        <href>%sR%3.3dC%3.3d.%s</href>\n      </Icon>\n", path, Q[k]->row, Q[k]->col, ext[Q[k]->type]);
		fprintf (fp, "      <LatLonBox>\n");
		fprintf (fp, "        <north>%.14g</north>\n", Q[k]->wesn[YHI]);
		fprintf (fp, "        <south>%.14g</south>\n", Q[k]->wesn[YLO]);
		fprintf (fp, "        <east>%.14g</east>\n",   Q[k]->wesn[XHI]);
		fprintf (fp, "        <west>%.14g</west>\n",   Q[k]->wesn[XLO]);
		fprintf (fp, "      </LatLonBox>\n");
 		fprintf (fp, "      <altitudeMode>%s</altitudeMode><altitude>%g</altitude>\n", alt_mode[Ctrl->A.mode], Ctrl->A.altitude);
		fprintf (fp, "    </GroundOverlay>\n");
		i_dir = (Q[k]->level == 0) ? 1 : -1;
		/* Now add up to 4 quad links */
		for (quad = 0; quad < 4; quad++) {
			if (Q[k]->next[quad] == NULL) continue;
			minLodPixels = (Q[k]->level == 0) ? 1 : Ctrl->L.size / 2;
			maxLodPixels = (Q[k]->level == max_level) ? -1 : 4*Ctrl->L.size;

			grd2kml_set_dirpath (Ctrl->D.single, NULL, Ctrl->N.prefix, Q[k]->next[quad]->level, dir, path);
			fprintf (fp, "\n    <NetworkLink>\n      <name>%s</name>\n", Q[k]->next[quad]->tag);
			fprintf (fp, "      <Region>\n        <LatLonAltBox>\n");
			fprintf (fp, "          <north>%.14g</north>\n", Q[k]->next[quad]->wesn[YHI]);
			fprintf (fp, "          <south>%.14g</south>\n", Q[k]->next[quad]->wesn[YLO]);
			fprintf (fp, "          <east>%.14g</east>\n",   Q[k]->next[quad]->wesn[XHI]);
			fprintf (fp, "          <west>%.14g</west>\n",   Q[k]->next[quad]->wesn[XLO]);
			fprintf (fp, "      </LatLonAltBox>\n");
			fprintf (fp, "      <Lod>\n");
			fprintf (fp, "        <minLodPixels>%d</minLodPixels>\n", minLodPixels);
			fprintf (fp, "        <maxLodPixels>%d</maxLodPixels>\n", maxLodPixels);
			fprintf (fp, "      </Lod>\n");
			fprintf (fp, "      </Region>\n");
			grd2kml_set_dirpath (Ctrl->D.single, NULL, Ctrl->N.prefix, Q[k]->next[quad]->level, i_dir, path);
			fprintf (fp, "      <Link>\n        <href>%sR%3.3dC%3.3d.kml</href>\n", path, Q[k]->next[quad]->row, Q[k]->next[quad]->col);
			fprintf (fp, "        <viewRefreshMode>onRegion</viewRefreshMode>\n");
			fprintf (fp, "      </Link>\n");
			fprintf (fp, "    </NetworkLink>\n");
		}
		fprintf (fp, "  </Document>\n</kml>\n");
		fclose (fp);

		gmt_M_str_free (Q[k]->region);	/* Free this tile region */
		gmt_M_free (GMT, Q[k]);		/* Free this tile information */
	}
	gmt_M_free (GMT, Q);
	GMT_Report (API, GMT_MSG_NOTICE, "Done: %d tiles (%d JPG and %d PNG) written to directory %s\n", n_tiles, n_img[0], n_img[1], Ctrl->N.prefix);

	Return (GMT_NOERROR);
}
