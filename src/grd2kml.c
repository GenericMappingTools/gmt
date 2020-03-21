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
 * Author:	Paul Wessel
 * Date:	8-NOV-2017
 * Version:	6 API
 *
 * Brief synopsis: grd2kml reads a single grid and makes a Google Earth
 * image quadtree.  Optionally supply an intensity grid (or auto-derive it)
 * and a CPT (or use default table), and request contours.
 *
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
	struct GRD2KML_A {	/* -A<size> */
		bool active;
		unsigned int size;
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
	struct GRD2KML_H {	/* -H<factor> */
		bool active;
		int factor;
	} H;
	struct GRD2KML_M {	/* -M[<magnify>]+i */
		bool active;
		bool interpolate;
		unsigned int magnify;
	} M;
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
	struct GRD2KML_Q {	/* -Q */
		bool active;
	} Q;
	struct  GRD2KML_W {	/* -W<cfile> */
		bool active;
		char *file;
	} W;
	struct  GRD2KML_T {	/* -T<title> */
		bool active;
		char *title;
	} T;
};

/* Structure used to keep track of which tile and its 4 possible underlings */
struct GMT_QUADTREE {
	unsigned int level, q;
	unsigned int row, col;
	char tag[16];
	char *region;
	double wesn[4];
	struct GMT_QUADTREE *next[4];
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRD2KML_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRD2KML_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->A.size = 128;
	C->F.filter = 'g';
	C->I.method  = strdup ("t1");	/* Default normalization for shading when -I is used */
	C->M.magnify = 1;
	C->L.size = 512;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRD2KML_CTRL *C) {	/* Deallocate control structure */
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

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <grid> -N<name> [-C<cpt>] [-E<url>] [-F<filter>] [-H<factor>] [-I[<intensgrid>|<value>|<modifiers>]]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "	[-L<size>] [-Q] [-T<title>] [%s] [-W<contfile>|<pen>] [%s] [%s]\n\n", GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<grid> is the data set to be plotted.  Its z-values are in user units and will be\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  converted to colors via the CPT [%s].\n", GMT_DEFAULT_CPT_NAME);
	GMT_Message (API, GMT_TIME_NONE, "\t-N Sets file name prefix for image directory and KML file. If the directory\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   already exist we will overwrite the files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Color palette file to convert z to rgb. Optionally, instead give name of a master cpt\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to automatically assign continuous colors over the data range [%s]; if so,\n", GMT_DEFAULT_CPT_NAME);
	GMT_Message (API, GMT_TIME_NONE, "\t   optionally append +i<dz> to quantize the range [the exact grid range].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Another option is to specify -C<color1>,<color2>[,<color3>,...] to build a\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   linear continuous cpt from those colors automatically.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E To store all files remotely, give leading URL [local files only].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify filter type used for downsampling.  Choose among.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     b: Boxcar      : Simple averaging of all points inside filter domain.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c: Cosine arch : Weighted averaging with cosine arc weights.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     g: Gaussian    : Weighted averaging with Gaussian weights [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     m: Median      : Median (50%% quantile) value of all points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-H Tell psconvert to do sub-pixel smoothing using factor <factor> [no sub-pixel smoothing].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Apply directional illumination. Append name of intensity grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For a constant intensity (i.e., change the ambient light), append a single value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To derive intensities from <grid> instead, append +a<azim> [-45] and +n<method> [t1]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or use -I+d to accept the default values (see grdgradient for details).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set tile size as a power of 2 [256].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Use PS Level 3 colormasking to make nodes with z = NaN transparent.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set title (document description) for the top-level KML.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Give file with select contours and pens to draw contours on the images [no contours].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no file is given we assume it is a pen and use the contours implied by the CPT file.\n");
	GMT_Option (API, "f,n,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRD2KML_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input files */
				if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID) && n_files == 0) {
					Ctrl->In.file = strdup (opt->arg);
					n_files++;
				}
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* min fade sizes  [EXPERIMENTAL, to delay fade out] */
				Ctrl->A.active = true;
				Ctrl->A.size = atoi (opt->arg);
				if (Ctrl->A.size <= 0) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -A: Must be positive!\n");
					n_errors++;
				}
				break;
			case 'C':	/* CPT */
				Ctrl->C.active = true;
				if ((c = strstr (opt->arg, "+i"))) {	/* Gave auto-interval */
					Ctrl->C.dz = atof (&c[2]);
					c[0] = '\0';	/* Temporarily chop off the modifier */
				}
				gmt_M_str_free (Ctrl->C.file);
				Ctrl->C.file = strdup (opt->arg);
				if (c) c[0] = '+';	/* Restore */
				break;
			case 'D':	/* Debug options - may fade away when happy with the performance */
				Ctrl->D.active = true;
				if (strstr (opt->arg, "+s")) Ctrl->D.single = true;	/* Write all files in a single directory instead of one directory per level */
				if (strstr (opt->arg, "+d")) Ctrl->D.dump = true;	/* Dump quadtree information to stdou */
				break;
			case 'E':	/* Remove URL for all contents but top driver kml */
				Ctrl->E.active = true;
				gmt_M_str_free (Ctrl->E.url);
				Ctrl->E.url = strdup (opt->arg);
				break;
			case 'F':	/* Select filter type */
				Ctrl->F.active = true;
				if (strchr ("bcgm", opt->arg[0]))
					Ctrl->F.filter = opt->arg[0];
				else {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -F: Choose among b, c, g, m!\n");
					n_errors++;
				}
				break;
			case 'H':	/* RIP at a higher dpi, then downsample in gs */
				Ctrl->H.active = true;
				Ctrl->H.factor = atoi (opt->arg);
				break;
			case 'I':	/* Here, intensity must be a grid file since we need to filter it */
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
				else if (gmt_M_file_is_cache (opt->arg))	/* Got a remote file */
					Ctrl->I.file = strdup (opt->arg);
				else if (opt->arg[0] && !gmt_not_numeric (GMT, opt->arg)) {	/* Looks like a constant value */
					Ctrl->I.value = atof (opt->arg);
					Ctrl->I.constant = true;
				}
				else {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -I: Requires a valid grid file or a constant\n");
					n_errors++;
				}
				break;
			case 'L':	/* Tiles sizes */
				Ctrl->L.active = true;
				Ctrl->L.size = atoi (opt->arg);
				if (Ctrl->L.size <= 0 || ((log2 ((double)Ctrl->L.size) - irint (log2 ((double)Ctrl->L.size))) > GMT_CONV8_LIMIT)) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -L: Must be radix 2!\n");
					n_errors++;
				}
				break;
			case 'M':	/* Magnify and/or interpolate [EXPERIMENTAL, to boost coarser grids] */
				Ctrl->M.active = true;
				if ((c = strstr (opt->arg, "+i"))) {
					Ctrl->M.interpolate = true;
					c[0] = '\0';	/* Chop off modifier for now */
				}
				if (opt->arg[0]) {
					Ctrl->M.magnify = atoi (opt->arg);
					if (Ctrl->M.magnify <= 0) {
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -M: Must be positive!\n");
						n_errors++;
					}
				}
				if (c) c[0] = '+';	/* Restore modifier */
				break;
			case 'N':	/* File name prefix */
				Ctrl->N.active = true;
				gmt_M_str_free (Ctrl->N.prefix);
				Ctrl->N.prefix = strdup (opt->arg);
				break;
			case 'Q':	/* Colormasking */
				Ctrl->Q.active = true;
				break;
			case 'T':	/* Title */
				Ctrl->T.active = true;
				if (opt->arg[0]) Ctrl->T.title = strdup (opt->arg);
				break;
			case 'W':	/* Contours and pens */
				Ctrl->W.active = true;
				if (opt->arg[0]) {
					gmt_M_str_free (Ctrl->W.file);
					Ctrl->W.file = strdup (opt->arg);
				}
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

int find_quad_above (struct GMT_QUADTREE **Q, unsigned int n, unsigned int row, unsigned int col, unsigned int level) {
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

void set_dirpath (bool single, char *url, char *prefix, unsigned int level, int dir, char *string) {
	if (single) {	/* Write everything into the prefix dir */
		if (url && level == 0)	/* Set the leading URL for zero-level first kml */
			sprintf (string, "%s/%s/L%d", url, prefix, level);
#if 0
		else if (level == 0 && dir == 1)	/* For top level we must write prefix dir */
			sprintf (string, "%s/L%d", prefix, level);
#endif
		else	/* Everything below is in same folder */
			sprintf (string, "L%d", level);
	}
	else {	/* Write to separate level directories */
		if (url && level == 0)	/* Set the leading URL for zero-level first kml */
			sprintf (string, "%s/%s/%d/", url, prefix, level);
		else if (dir == -1)	/* Need to refer to another directory at same level as this one */
			sprintf (string, "../%d/", level);
		else if (dir == 0)	/* At current dir */
			string[0] = '\0';
		else	/* Down in a dir */
			sprintf (string, "%d/", level);
	}
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int gmtlib_geo_C_format (struct GMT_CTRL *GMT);

int GMT_grd2kml (void *V_API, int mode, void *args) {
	int error = 0, kk, uniq, dpi;
	bool use_tile = false, z_extend = false, i_extend = false, tmp_cpt = false;

	unsigned int level, max_level, n = 0, k, nx, ny, mx, my, row, col, n_skip, quad, n_alloc = GMT_CHUNK, n_bummer = 0;

	uint64_t node;

	double factor, dim, west, east, wesn[4], ext_wesn[4], inc[2];


	char cmd[GMT_BUFSIZ] = {""}, level_dir[PATH_MAX] = {""}, Zgrid[PATH_MAX] = {""}, Igrid[PATH_MAX] = {""};
	char W[GMT_LEN16] = {""}, E[GMT_LEN16] = {""}, S[GMT_LEN16] = {""}, N[GMT_LEN16] = {""}, file[PATH_MAX] = {""};
	char DataGrid[PATH_MAX] = {""}, IntensGrid[PATH_MAX] = {""}, path[PATH_MAX] = {""}, im_arg[16] = {""};
	char region[GMT_LEN128] = {""}, ps_cmd[GMT_LEN128] = {""}, cfile[GMT_VF_LEN] = {""}, K[4] = {""}, *cmd_args = NULL;

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

	uniq = (int)getpid();	/* Unique number for temporary files  */

	if (!Ctrl->C.active) {	/* If no cpt given then we must compute one from the grid and use throughout */
		unsigned int zmode = gmt_cpt_default (GMT, G->header);
		char cfile[PATH_MAX] = {""};
		struct GMT_PALETTE *P = NULL;
		if ((P = gmt_get_palette (GMT, Ctrl->C.file, GMT_CPT_OPTIONAL, G->header->z_min, G->header->z_max, Ctrl->C.dz, zmode)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Failed to create a CPT\n");
			Return (API->error);	/* Well, that did not go well... */
		}
		sprintf (cfile, "%s/grd2kml_%d.cpt", API->tmp_dir, uniq);
		if (GMT_Write_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, 0, NULL, cfile, P) != GMT_NOERROR) {
			Return (API->error);
		}
		Ctrl->C.active = tmp_cpt = true;
		Ctrl->C.file = strdup (cfile);
	}

	Ctrl->L.size /= Ctrl->M.magnify;
	dpi = 100 * Ctrl->M.magnify;
	/* Set specific grdimage option -Q or -Ei or neither */
	if (Ctrl->Q.active) {	/* Want NaN colormasking */
		if (Ctrl->M.magnify > 1) 	/* and magnification as well */
			sprintf (im_arg, " -Q -E100");	/* For magnifying we must turn on 100 dpi interpolation since -Q is in effect */
		else	/* Just colormasking */
			sprintf (im_arg, " -Q");
	}
	else if (Ctrl->M.interpolate)	/* Want interpolation and no colormasking in effect means we can let psconvert do it */
		sprintf (im_arg, " -Ei");
	nx = G->header->n_columns;	ny = G->header->n_rows;			/* Dimensions of original grid */
	mx = urint (ceil ((double)nx / (double)Ctrl->L.size)) * Ctrl->L.size;	/* Nearest image size in multiples of tile size */
	my = urint (ceil ((double)ny / (double)Ctrl->L.size)) * Ctrl->L.size;
	max_level = urint (ceil (log2 (MAX (mx, my) / (double)Ctrl->L.size)));	/* Number of levels in the quadtree */
	if ((60.0 * G->header->inc[GMT_X] - irint (60.0 * G->header->inc[GMT_X])) < GMT_CONV4_LIMIT) {
		/* Grid spacing is an integer multiple of 1 arc minute or higher, use ddd:mm format */
		strcpy (GMT->current.setting.format_geo_out, "ddd:mm");
	}
	else if ((3600.0 * G->header->inc[GMT_X] - irint (3600.0 * G->header->inc[GMT_X])) < GMT_CONV4_LIMIT) {
		/* Grid spacing is an integer multiple of 1 arc sec or higher, use ddd:mm:ss format */
		strcpy (GMT->current.setting.format_geo_out, "ddd:mm:ss");
	}
	else if ((36000.0 * G->header->inc[GMT_X] - irint (36000.0 * G->header->inc[GMT_X])) < GMT_CONV4_LIMIT) {
		/* Grid spacing is an integer multiple of 0.1 arc sec or higher, use ddd:mm:ss.x format */
		strcpy (GMT->current.setting.format_geo_out, "ddd:mm:ss.x");
	}
	else {	/* Cannot use 0.1 arcsecs, do full decimal resolution */
		strcpy (GMT->current.setting.format_float_out, "%.16g");
		strcpy (GMT->current.setting.format_geo_out, "D");
	}
	gmtlib_geo_C_format (GMT);	/* Update the format settings */
	dim = dpi * 0.0001 * Ctrl->L.size;	/* Constant tile map size in inches for a fixed dpi of 100 yields PNGS of the requested dimension in -L */

	GMT->current.io.geo.range = (G->header->wesn[XLO] < 0.0 && G->header->wesn[XHI] > 0.0) ? ((G->header->wesn[XHI] > 180.0) ? GMT_IS_GIVEN_RANGE : GMT_IS_M180_TO_P180_RANGE) : GMT_IS_0_TO_P360_RANGE;

	/* Create the container quadtree directory first */
	if (gmt_mkdir (Ctrl->N.prefix))
		GMT_Report (API, GMT_MSG_INFORMATION, "Directory %s already exist - will overwrite files\n", Ctrl->N.prefix);

	if (Ctrl->I.derive) {	/* Auto-create single intensity grid from data grid to ensure constant scaling */
		sprintf (file, "%s/grd2kml_intensity_tmp_%6.6d.grd", API->tmp_dir, uniq);
		Ctrl->I.file = strdup (file);
		GMT_Report (API, GMT_MSG_INFORMATION, "Derive an intensity grid from data grid\n");
		/* Prepare the grdgradient arguments using selected -A -N and the data region in effect */
		sprintf (cmd, "%s -G%s -A%s -N%s --GMT_HISTORY=false", Ctrl->In.file, Ctrl->I.file, Ctrl->I.azimuth, Ctrl->I.method);
		/* Call the grdgradient module */
		GMT_Report (API, GMT_MSG_INFORMATION, "Calling grdgradient with args %s\n", cmd);
		if (GMT_Call_Module (API, "grdgradient", GMT_MODULE_CMD, cmd))
			Return (API->error);
	}
	else if (Ctrl->I.file) {	/* Can read the intensity file and compare region etc to data grid */
		struct GMT_GRID *I = NULL;
		double x_noise = GMT_CONV4_LIMIT * G->header->inc[GMT_X], y_noise = GMT_CONV4_LIMIT * G->header->inc[GMT_Y];
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
	}

	Q = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_QUADTREE *);
	factor = pow (2.0, max_level);	/* Max width of imaged pixels in multiples of original grid spacing for this level */
	/* Determine extended region required if using the largest multiple of original grid spacing */
	inc[GMT_X] = factor * G->header->inc[GMT_X];
	inc[GMT_Y] = factor * G->header->inc[GMT_Y];
	ext_wesn[XLO] = floor (G->header->wesn[XLO] / inc[GMT_X]) * inc[GMT_X];
	ext_wesn[XHI] = MIN (ext_wesn[XLO]+360.0, ceil  (G->header->wesn[XHI] / inc[GMT_X]) * inc[GMT_X]);
	ext_wesn[YLO] = MAX (-90.0, floor (G->header->wesn[YLO] / inc[GMT_Y]) * inc[GMT_Y]);
	ext_wesn[YHI] = MIN (+90.0, ceil  (G->header->wesn[YHI] / inc[GMT_Y]) * inc[GMT_Y]);
	if (ext_wesn[XLO] < G->header->wesn[XLO] || ext_wesn[XHI] > G->header->wesn[XHI] || ext_wesn[YLO] < G->header->wesn[YLO] || ext_wesn[YHI] > G->header->wesn[YHI]) {
		/* Extend the original grid with NaNs so it is an exact multiple of largest grid stride at max level */
		sprintf (DataGrid, "%s/grd2kml_extended_data_%6.6d.grd", API->tmp_dir, uniq);
		sprintf (cmd, "%s -R%.16g/%.16g/%.16g/%.16g -N -G%s", Ctrl->In.file, ext_wesn[XLO], ext_wesn[XHI], ext_wesn[YLO], ext_wesn[YHI], DataGrid);
		GMT_Report (API, GMT_MSG_INFORMATION, "Extend original data grid to multiple of largest grid spacing\n");
		if ((error = GMT_Call_Module (API, "grdcut", GMT_MODULE_CMD, cmd)) != GMT_NOERROR) {
			gmt_M_free (GMT, Q);
			Return (GMT_RUNTIME_ERROR);
		}
		z_extend = true;	/* We made a temp file we need to zap */
		if (Ctrl->I.active) {	/* Also extend the intensity grid */
			sprintf (IntensGrid, "%s/grd2kml_extended_intens_%6.6d.grd", API->tmp_dir, uniq);
			sprintf (cmd, "%s -R%.16g/%.16g/%.16g/%.16g -N -G%s", Ctrl->I.file, ext_wesn[XLO], ext_wesn[XHI], ext_wesn[YLO], ext_wesn[YHI], IntensGrid);
			GMT_Report (API, GMT_MSG_INFORMATION, "Extend intensity grid to multiple of largest grid spacing\n");
			if ((error = GMT_Call_Module (API, "grdcut", GMT_MODULE_CMD, cmd)) != GMT_NOERROR) {
				gmt_M_free (GMT, Q);
				Return (GMT_RUNTIME_ERROR);
			}
			i_extend = true;	/* We made a temp file we need to zap */
		}
	}
	else {	/* No need to extend, use as is */
		strcpy (DataGrid, Ctrl->In.file);
		if (Ctrl->I.active)
			strcpy (IntensGrid, Ctrl->I.file);
	}

	if (Ctrl->W.active) {	/* Want to overlay contours given via file */
		uint64_t c;
		char line[GMT_LEN256] = {""};
		if (!gmt_access (GMT, Ctrl->W.file, F_OK)) {	/* Was given an actual file */
			char cval[GMT_LEN64] = {""}, pen[GMT_LEN64] = {""};
			if ((C = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_TEXT, GMT_READ_NORMAL, NULL, Ctrl->W.file, NULL)) == NULL) {
				gmt_M_free (GMT, Q);
				Return (GMT_RUNTIME_ERROR);
			}
			if (C->n_segments > 1 || C->n_records == 0 || C->table[0]->segment[0]->text == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Contour file has more than one segment, no records at all, or no text\n");
				gmt_M_free (GMT, Q);
				Return (GMT_RUNTIME_ERROR);
			}
			for (c = 0; c < C->n_records; c++) {	/* Must reformat the records to fit grdcontour requirements */
				if (C->table[0]->segment[0]->text[c] == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "No text record found\n");
					gmt_M_free (GMT, Q);
					Return (GMT_RUNTIME_ERROR);
				}
				sscanf (C->table[0]->segment[0]->text[c], "%s %s", cval, pen);
				sprintf (line, "%s C 0 %s", cval, pen);	/* Build the required record format for grdcontour */
				gmt_M_str_free (C->table[0]->segment[0]->text[c]);	/* Free previous string */
				C->table[0]->segment[0]->text[c] = strdup (line);	/* Update string */
			}
		}
		else {	/* Use contours from CPT file, with -W<pen> */
			struct GMT_PALETTE *P = NULL;
			uint64_t dim_c[4] = {1, 1, 0, 0};
			if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
				gmt_M_free (GMT, Q);
				Return (API->error);
			}
			dim_c[GMT_ROW] = P->n_colors + 1;	/* Number of contours implied by CPT */
			if ((C = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_WITH_STRINGS, dim_c, NULL, NULL, 0, 0, NULL)) == NULL) {
				gmt_M_free (GMT, Q);
				Return (GMT_RUNTIME_ERROR);
			}
			for (c = 0; c < P->n_colors; c++) {	/* Do all the low boundaries */
				sprintf (line, "%.16g C 0 %s", P->data[c].z_low, Ctrl->W.file);	/* Build the required record format for grdcontour */
				gmt_M_str_free (C->table[0]->segment[0]->text[c]);	/* Free previous string */
				C->table[0]->segment[0]->text[c] = strdup (line);	/* Update string */
			}
			sprintf (line, "%.16g C 0 %s", P->data[P->n_colors-1].z_high, Ctrl->W.file);	/* Must add the last high boundary */
			gmt_M_str_free (C->table[0]->segment[0]->text[c]);	/* Free previous string */
			C->table[0]->segment[0]->text[c] = strdup (line);	/* Update string */
		}
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_IN, C, cfile) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create virtual file for contours\n");
			gmt_M_free (GMT, Q);
			GMT_Destroy_Data (API, &C);
			Return (GMT_RUNTIME_ERROR);
		}
		strcpy (K, " -K");	/* Since now we must do a contour overlay */
	}

	if (Ctrl->H.active)	/* Do sub-pixel smoothing */
		sprintf (ps_cmd, "-TG -E100 -P -Ve -Z -H%d", Ctrl->H.factor);
	else
		sprintf (ps_cmd, "-TG -E100 -P -Ve -Z");

	/* Loop over all the levels, starting at the top level (0) */
	for (level = 0; level <= max_level; level++) {
		factor = pow (2.0, max_level - level);	/* Width of imaged pixels in multiples of original grid spacing for this level */
		inc[GMT_X] = factor * G->header->inc[GMT_X];
		inc[GMT_Y] = factor * G->header->inc[GMT_Y];
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Level %d: Factor = %g Dim = %d x %d -> %d x %d\n",
			level, factor, irint (factor * Ctrl->L.size), irint (factor * Ctrl->L.size), Ctrl->L.size, Ctrl->L.size);
		/* Create the level directory */
		if (!Ctrl->D.single) {
			sprintf (level_dir, "%s/%d", Ctrl->N.prefix, level);
			if (gmt_mkdir (level_dir))
				GMT_Report (API, GMT_MSG_INFORMATION, "Level directory %s already exist - overwriting files\n", level_dir);
		}
		if (level < max_level) {	/* Filter the data to match level resolution */
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Level %d: Filtering down the grid(s)\n", level);
			sprintf (Zgrid, "%s/grd2kml_Z_L%d_tmp_%6.6d.grd", API->tmp_dir, level, uniq);
			sprintf (cmd, "%s -D0 -F%c%.16g -I%.16g -G%s", DataGrid, Ctrl->F.filter, inc[GMT_X], inc[GMT_X], Zgrid);
			GMT_Report (API, GMT_MSG_INFORMATION, "Running grdfilter : %s\n", cmd);
			if ((error = GMT_Call_Module (API, "grdfilter", GMT_MODULE_CMD, cmd)) != GMT_NOERROR) {
				gmt_M_free (GMT, Q);
				if (Ctrl->W.active) GMT_Destroy_Data (API, &C);
				Return (GMT_RUNTIME_ERROR);
			}
			if (Ctrl->I.active) {	/* Also filter the intensity grid */
				sprintf (Igrid, "%s/grd2kml_I_L%d_tmp_%6.6d.grd", API->tmp_dir, level, uniq);
				sprintf (cmd, "%s -D0 -F%c%.16g -I%.16g -G%s", IntensGrid, Ctrl->F.filter, inc[GMT_X], inc[GMT_X], Igrid);
				GMT_Report (API, GMT_MSG_INFORMATION, "Running grdfilter : %s\n", cmd);
				if ((error = GMT_Call_Module (API, "grdfilter", GMT_MODULE_CMD, cmd)) != GMT_NOERROR) {
					gmt_M_free (GMT, Q);
					if (Ctrl->W.active) GMT_Destroy_Data (API, &C);
					Return (GMT_RUNTIME_ERROR);
				}
			}
		}
		else {	/* Use as is for the highest resolution */
			strcpy (Zgrid, Ctrl->In.file);
			if (Ctrl->I.active) strcpy (Igrid, Ctrl->I.file);
		}

		/* Loop over all rows at this level */
		row = col = n_skip = 0;
		wesn[YLO] = ext_wesn[YLO];
		gmt_ascii_format_one (GMT, S, wesn[YLO], GMT_IS_LAT);

		while (wesn[YLO] < (G->header->wesn[YHI]-G->header->inc[GMT_Y])) {	/* Small correction to avoid issues due to round-off */
			wesn[YHI] = MIN (90.0, wesn[YLO] + factor * Ctrl->L.size * G->header->inc[GMT_Y]);	/* Top row may extend beyond grid and be transparent */
			gmt_ascii_format_one (GMT, N, wesn[YHI], GMT_IS_LAT);
			/* Loop over all columns at this level */
			col = 0;
			wesn[XLO] = ext_wesn[XLO];
			while (wesn[XLO] < (G->header->wesn[XHI]-G->header->inc[GMT_X])) {
				uint64_t trow, tcol;
				wesn[XHI] = MIN (ext_wesn[XLO]+360.0, wesn[XLO] + factor * Ctrl->L.size * G->header->inc[GMT_X]);	/* So right column may extend beyond grid and be transparent */
				/* Must make sure we have a proper formatting of these longitudes so handle any east > 360 */
				west = wesn[XLO];	east = wesn[XHI];
				if (east > 360.0) west -= 360.0, east -= 360.0;	/* Keep within -180 to 180 range */
				GMT->current.io.geo.range = (west < 0.0 && east > 0.0) ? ((east > 180.0) ? GMT_IS_GIVEN_RANGE : GMT_IS_M180_TO_P180_RANGE) : GMT_IS_0_TO_P360_RANGE;
				gmt_ascii_format_one (GMT, W, west, GMT_IS_LON);
				gmt_ascii_format_one (GMT, E, east, GMT_IS_LON);
				/* Now we have the current tile region */
				if ((T = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, wesn, Zgrid, NULL)) == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to read in grid tile!\n");
					gmt_M_free (GMT, Q);
					if (Ctrl->W.active) GMT_Destroy_Data (API, &C);
					Return (API->error);
				}
				/* Determine if we have any non-NaN data points inside this grid */
				for (trow = 0, use_tile = false; !use_tile && trow < T->header->n_rows; trow++) {
					for (tcol = 0; !use_tile && tcol < T->header->n_columns; tcol++) {
						node = gmt_M_ijp (T->header, trow, tcol);
						use_tile = !gmt_M_is_fnan (T->data[node]);
					}
				}

				if (use_tile) {	/* Found data inside this tile, make plot and rasterize */
					/* Build the grdimage command to make the PostScript plot */
					char z_data[GMT_VF_LEN] = {""}, psfile[PATH_MAX] = {""};
					/* Open the grid subset as a virtual file we can pass to grdimage */
					if (GMT_Open_VirtualFile (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN, T, z_data) == GMT_NOTSET) {
						GMT_Report (API, GMT_MSG_ERROR, "Unable to open grid tile as virtual file!\n");
						gmt_M_free (GMT, Q);
						if (Ctrl->W.active) GMT_Destroy_Data (API, &C);
						Return (API->error);
					}
					/* Will pass -W so grdimage will notify us if there was no valid image data imaged */
					sprintf (psfile, "%s/grd2kml_tile_tmp_%6.6d.ps", API->tmp_dir, uniq);
					if (Ctrl->I.active)	/* Must pass two grids */
						sprintf (cmd, "%s -I%s -JX%3.2lfid -X0 -Y0 -W -R%s/%s/%s/%s%s%s -Ve --PS_MEDIA=%3.2lfix%3.2lfi ->%s", z_data, Igrid, dim, W, E, S, N, im_arg, K, dim, dim, psfile);
					else
						sprintf (cmd, "%s -JX%3.2lfid -X0 -Y0 -W -R%s/%s/%s/%s%s%s -Ve --PS_MEDIA=%3.2lfix%3.2lfi ->%s", z_data, dim, W, E, S, N, im_arg, K, dim, dim, psfile);
					if (Ctrl->C.active) {strcat (cmd, " -C"); strcat (cmd, Ctrl->C.file); }
					error = GMT_Call_Module (API, "grdimage", GMT_MODULE_CMD, cmd);
					if (error == GMT_NOERROR && Ctrl->W.active) {	/* Overlay contours */
						sprintf (cmd, "%s -JX%3.2lfid -R%s/%s/%s/%s -O -C%s -Ve ->>%s", z_data, dim, W, E, S, N, cfile, psfile);
						GMT_Init_VirtualFile (API, 0, z_data);	/* Read the same grid again */
						GMT_Init_VirtualFile (API, 0, cfile);	/* Read the same contours again */
						if ((error = GMT_Call_Module (API, "grdcontour", GMT_MODULE_CMD, cmd))) {
							GMT_Report (API, GMT_MSG_ERROR, "Unable to overlay contours!\n");
							gmt_M_free (GMT, Q);
							GMT_Destroy_Data (API, &C);
							Return (API->error);
						}
					}
					GMT_Close_VirtualFile (API, z_data);
					if (GMT_Destroy_Data (API, &T) != GMT_NOERROR) {
						GMT_Report (API, GMT_MSG_ERROR, "Unable to free memory of grid tile!\n");
						gmt_M_free (GMT, Q);
						Return (API->error);
					}
					if (error == GMT_IMAGE_NO_DATA) {	/* Must have found non-Nans in the pad since the image is all NaN? */
						GMT_Report (API, GMT_MSG_WARNING, "No image content for current tile (%d, %d, %d) - skipped\n", level, row, col);
						gmt_remove_file (GMT, psfile);
						n_bummer++;
					}
					else {	/* Made a meaningful plot, time to rip. */
						/* Create the psconvert command to convert the PS to transparent PNG */
						sprintf (region, "%s/%s/%s/%s", W, E, S, N);
						GMT_Report (GMT->parent, GMT_MSG_WARNING, "Level %d: Mapped tile %s\n", level, region);
						if (Ctrl->D.single)
							sprintf (cmd, "%s -D%s -FL%dR%dC%d.png %s", ps_cmd, Ctrl->N.prefix, level, row, col, psfile);
						else
							sprintf (cmd, "%s -D%s -FR%dC%d.png %s", ps_cmd, level_dir, row, col, psfile);
						if (GMT_Call_Module (API, "psconvert", GMT_MODULE_CMD, cmd)) {
							GMT_Report (API, GMT_MSG_ERROR, "Unable to rasterize current PNG tile!\n");
							gmt_M_free (GMT, Q);
							Return (API->error);
						}
						/* Update our list of tiles */
						Q[n] = gmt_M_memory (GMT, NULL, 1, struct GMT_QUADTREE);
						Q[n]->row = row; Q[n]->col = col;	Q[n]->level = level;
						Q[n]->wesn[XLO] = west;		Q[n]->wesn[XHI] = east;
						Q[n]->wesn[YLO] = wesn[YLO];	Q[n]->wesn[YHI] = wesn[YHI];
						sprintf (Q[n]->tag, "L%2.2dR%2.2dC%2.2d", level, row, col);
						Q[n]->region = strdup (region);
						if (++n == n_alloc) {	/* Extend the array */
							n_alloc <<= 1;
							Q = gmt_M_memory (GMT, Q, n_alloc, struct GMT_QUADTREE *);
						}
					}
				}
				else {	/* Just NaNs inside this tile */
					GMT_Report (API, GMT_MSG_INFORMATION, "Level %d: Tile %s/%s/%s/%s had no data - skipped\n", level, W, E, S, N);
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
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Summary Level %d: %d by %d = %d tiles, %d mapped, %d empty\n", level, row, col, row*col, row*col - n_skip, n_skip);
		if (level < max_level) {	/* Delete the temporary filtered grid(s) */
			gmt_remove_file (GMT, Zgrid);
			if (Ctrl->I.active) gmt_remove_file (GMT, Igrid);
		}
	}

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Found %d tiles that passed the no-NaN test but gave a blank image (?)\n", n_bummer);

	/* Clean up any temporary grids */

	if (z_extend && !access (DataGrid, F_OK))
		gmt_remove_file (GMT, DataGrid);
	if (i_extend && !access (IntensGrid, F_OK))
		gmt_remove_file (GMT, IntensGrid);
	if (Ctrl->I.derive)
		gmt_remove_file (GMT, Ctrl->I.file);

	/* Process quadtree links */

	GMT_Report (GMT->parent, GMT_MSG_WARNING, "Processes quadtree links for %d tiles.\n", n);
	Q = gmt_M_memory (GMT, Q, n, struct GMT_QUADTREE *);	/* Final size */
	for (level = max_level; level > 0; level--) {
		for (k = 0; k < n; k++) {
			if (Q[k]->level != level) continue;	/* Only deal with this level here */
			/* Determine the parent tile and the quad (0-3) we belong to */
			/* This is the parent row and col since we increase by a factor of 2 each time */
			row = Q[k]->row / 2;	col = Q[k]->col / 2;
			/* The quad is given by comparing the high and low values of row, col */
			quad = 2 * (Q[k]->row - 2 * row) + (Q[k]->col - 2 * col);
			kk = find_quad_above (Q, n, row, col, level-1);	/* kk is the parent of k */
			if (kk < 0) {	/* THis can happen when the lower-level tile grazes one above it but there really are no data involved */
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Tile %s: Unable to link tile for row = %d, col = %d at level %d to a parent (!?).  Probably empty - skipped.\n", Q[k]->tag, row, col, level);
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Tile %s: Region was %g/%g/%g/%g.\n", Q[k]->tag, Q[k]->wesn[XLO], Q[k]->wesn[XHI], Q[k]->wesn[YLO], Q[k]->wesn[YHI]);
				continue;
			}
			assert (quad < 4);	/* Sanity check */
			Q[kk]->next[quad] = Q[k];	/* Do the linking */
			Q[kk]->q++;			/* Count the links for this parent */
		}
	}

	/* Create the top-level KML file */
	sprintf (file, "%s/%s.kml", Ctrl->N.prefix, Ctrl->N.prefix);
	cmd_args = GMT_Create_Cmd (API, options);
	if ((fp = fopen (file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create file : %s\n", file);
		Return (GMT_RUNTIME_ERROR);
	}
	fprintf (fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n  <kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
	fprintf (fp, "    <Document>\n      <name>%s</name>\n", Ctrl->N.prefix);
	fprintf (fp, "    <!-- Produced by the Generic Mapping Tools [www.generic-mapping-tools.org] -->\n");
	fprintf (fp, "    <!-- cmd: gmt %s %s -->\n", GMT->init.module_name, cmd_args);
	gmt_M_free (GMT, cmd_args);
	if (Ctrl->T.active)
		fprintf (fp, "      <description>%s</description>\n\n", Ctrl->T.title);
	else
		fprintf (fp, "      <description>GMT image quadtree representation of %s</description>\n\n", Ctrl->In.file);
        fprintf (fp, "      <Style>\n");
        fprintf (fp, "        <ListStyle id=\"hideChildren\">          <listItemType>checkHideChildren</listItemType>\n        </ListStyle>\n");
        fprintf (fp, "      </Style>\n");

	set_dirpath (Ctrl->D.single, NULL, Ctrl->N.prefix, 0, 1, path);
	fprintf (fp, "      <NetworkLink>\n        <name>%sR0C0.png</name>\n", path);
	fprintf (fp, "        <Region>\n          <LatLonAltBox>\n");
	fprintf (fp, "            <north>%.14g</north>\n", G->header->wesn[YHI]);
	fprintf (fp, "            <south>%.14g</south>\n", G->header->wesn[YLO]);
	fprintf (fp, "            <east>%.14g</east>\n",   G->header->wesn[XHI]);
	fprintf (fp, "            <west>%.14g</west>\n",   G->header->wesn[XLO]);
	fprintf (fp, "          </LatLonAltBox>\n");
	fprintf (fp, "          <Lod>\n            <minLodPixels>%d</minLodPixels>\n            <maxLodPixels>-1</maxLodPixels>\n          </Lod>\n", Ctrl->A.size);
        fprintf (fp, "        </Region>\n");
	set_dirpath (Ctrl->D.single, Ctrl->E.url, Ctrl->N.prefix, 0, 1, path);
	fprintf (fp, "        <Link>\n          <href>%sR0C0.kml</href>\n", path);
	fprintf (fp, "          <viewRefreshMode>onRegion</viewRefreshMode>\n          <viewFormat/>\n");
	fprintf (fp, "        </Link>\n      </NetworkLink>\n");
	fprintf (fp, "    </Document>\n  </kml>\n");
	fclose (fp);

	/* Then create all the other KML files in the quadtree with their links down the tree */

	for (k = 0; k < n; k++) {
		if (Q[k]->q) {	/* Only examine tiles with children */
			if (Ctrl->D.dump) {
				printf ("%s [%s]:\t", Q[k]->tag, Q[k]->region);
				for (quad = 0; quad < 4; quad++)
					if (Q[k]->next[quad]) printf (" %c=%s", 'A'+quad, Q[k]->next[quad]->tag);
				printf ("\n");
			}
			if (Ctrl->D.single)
				sprintf (file, "%s/L%dR%dC%d.kml", Ctrl->N.prefix, Q[k]->level, Q[k]->row, Q[k]->col);
			else
				sprintf (file, "%s/%d/R%dC%d.kml", Ctrl->N.prefix, Q[k]->level, Q[k]->row, Q[k]->col);
			if ((fp = fopen (file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create file : %s\n", file);
				Return (GMT_RUNTIME_ERROR);
			}
			/* First this tile's kml and png */
			fprintf (fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n  <kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
			set_dirpath (Ctrl->D.single, NULL, Ctrl->N.prefix, Q[k]->level, 1, path);
			fprintf (fp, "    <Document>\n      <name>%sR%dC%d.kml</name>\n", path, Q[k]->row, Q[k]->col);
			fprintf (fp, "      <description></description>\n\n");
		        fprintf (fp, "      <Style>\n");
		        fprintf (fp, "        <ListStyle id=\"hideChildren\">          <listItemType>checkHideChildren</listItemType>\n        </ListStyle>\n");
		        fprintf (fp, "      </Style>\n");
		        fprintf (fp, "      <Region>\n        <LatLonAltBox>\n");
			fprintf (fp, "          <north>%.14g</north>\n", Q[k]->wesn[YHI]);
			fprintf (fp, "          <south>%.14g</south>\n", Q[k]->wesn[YLO]);
			fprintf (fp, "          <east>%.14g</east>\n",   Q[k]->wesn[XHI]);
			fprintf (fp, "          <west>%.14g</west>\n",   Q[k]->wesn[XLO]);
			fprintf (fp, "        </LatLonAltBox>\n");
			fprintf (fp, "        <Lod>\n          <minLodPixels>%d</minLodPixels>\n          <maxLodPixels>2048</maxLodPixels>\n        </Lod>\n", Ctrl->A.size);
		        fprintf (fp, "      </Region>\n");
			fprintf (fp, "      <GroundOverlay>\n        <drawOrder>%d</drawOrder>\n", 10+2*Q[k]->level);
			set_dirpath (Ctrl->D.single, NULL, Ctrl->N.prefix, Q[k]->level, 0, path);
			fprintf (fp, "        <Icon>\n          <href>%sR%dC%d.png</href>\n        </Icon>\n", path, Q[k]->row, Q[k]->col);
		        fprintf (fp, "        <LatLonBox>\n");
			fprintf (fp, "           <north>%.14g</north>\n", Q[k]->wesn[YHI]);
			fprintf (fp, "           <south>%.14g</south>\n", Q[k]->wesn[YLO]);
			fprintf (fp, "           <east>%.14g</east>\n",   Q[k]->wesn[XHI]);
			fprintf (fp, "           <west>%.14g</west>\n",   Q[k]->wesn[XLO]);
			fprintf (fp, "        </LatLonBox>\n      </GroundOverlay>\n");
			/* Now add up to 4 quad links */
			for (quad = 0; quad < 4; quad++) {
				if (Q[k]->next[quad] == NULL) continue;

				set_dirpath (Ctrl->D.single, NULL, Ctrl->N.prefix, Q[k]->next[quad]->level, 1, path);
				fprintf (fp, "\n      <NetworkLink>\n        <name>%sR%dC%d.png</name>\n", path, Q[k]->next[quad]->row, Q[k]->next[quad]->col);
			        fprintf (fp, "        <Region>\n          <LatLonAltBox>\n");
				fprintf (fp, "            <north>%.14g</north>\n", Q[k]->next[quad]->wesn[YHI]);
				fprintf (fp, "            <south>%.14g</south>\n", Q[k]->next[quad]->wesn[YLO]);
				fprintf (fp, "            <east>%.14g</east>\n",   Q[k]->next[quad]->wesn[XHI]);
				fprintf (fp, "            <west>%.14g</west>\n",   Q[k]->next[quad]->wesn[XLO]);
				fprintf (fp, "        </LatLonAltBox>\n");
				fprintf (fp, "        <Lod>\n          <minLodPixels>%d</minLodPixels>\n          <maxLodPixels>-1</maxLodPixels>\n        </Lod>\n", Ctrl->A.size);
			        fprintf (fp, "        </Region>\n");
				set_dirpath (Ctrl->D.single, NULL, Ctrl->N.prefix, Q[k]->next[quad]->level, -1, path);
				fprintf (fp, "        <Link>\n          <href>%sR%dC%d.kml</href>\n", path, Q[k]->next[quad]->row, Q[k]->next[quad]->col);
				fprintf (fp, "          <viewRefreshMode>onRegion</viewRefreshMode><viewFormat/>\n");
				fprintf (fp, "        </Link>\n      </NetworkLink>\n");
			}
			fprintf (fp, "    </Document>\n  </kml>\n");
			fclose (fp);

		}
		gmt_M_str_free (Q[k]->region);	/* Free this tile region */
		gmt_M_free (GMT, Q[k]);		/* Free this tile information */
	}
	gmt_M_free (GMT, Q);
	GMT_Report (API, GMT_MSG_WARNING, "Done: %d files written to directory %s\n", 2*n+1, Ctrl->N.prefix);
	if (tmp_cpt) gmt_remove_file (GMT, Ctrl->C.file);
	Return (GMT_NOERROR);
}
