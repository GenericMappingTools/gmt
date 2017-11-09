/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Author:	Paul Wessel
 * Date:	8-NOV-2017
 * Version:	6 API
 *
 * Brief synopsis: grd2kml reads a single grid and makes a Google Earth
 * image quadtree.  Optionally supply an intensity grid and a CPT.
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"grd2kml"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Create KML image quadtree from single grid"
#define THIS_MODULE_KEYS	"<G{,CC(,IG("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"-Vf"

struct GRD2KML_CTRL {
	struct GRD2KM_In {
		bool active;
		char *file;
	} In;
	struct GRD2KML_C {	/* -C<cpt> */
		bool active;
		char *file;
	} C;
	struct GRD2KML_D {	/* -D */
		bool active;
	} D;
	struct GRD2KML_E {	/* -E<url> */
		bool active;
		char *url;
	} E;
	struct GRD2KML_F {	/* -F */
		bool active;
		char filter;
	} F;
	struct GRD2KML_N {	/* -N<prefix> */
		bool active;
		char *prefix;
	} N;
	struct GRD2KML_I {	/* -I<intensgrd> */
		bool active;
		char *file;
	} I;
	struct GRD2KML_L {	/* -L<size> */
		bool active;
		unsigned int size;
	} L;
};

/* Structure used to keep track of which tile and its 4 possible underlings */
struct GMT_QUADTREE {
	unsigned int level, q;
	unsigned int row, col;
	char tag[16];
	double wesn[4];
	struct GMT_QUADTREE *next[4];
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRD2KML_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRD2KML_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->F.filter = 'g';
	C->N.prefix = strdup ("GMT_Quadtree");
	C->L.size = 256;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRD2KML_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->N.prefix);
	gmt_M_str_free (C->I.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grd2kml <grid> [-C<cpt>] [-D] [-E<url>] [-F<filter>] [-I<intensgrid>] [-L<size>]\n");
	GMT_Message (API, GMT_TIME_NONE, "	[-N<name>] [%s] [%s]\n\n", GMT_V_OPT, GMT_f_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<grid> is the data set to be plotted.  Its z-values are in user units and will be\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  converted to colors via the CPT [rainbow].\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Color palette file to convert z to rgb. Optionally, instead give name of a master cpt\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to automatically assign 16 continuous colors over the data range [rainbow].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Another option is to specify -C<color1>,<color2>[,<color3>,...] to build a\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   linear continuous cpt from those colors automatically.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Write a list of quadtree associations to stdout [no listing].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E To store all files remotely, give leading URL [local files only].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify filter type used for downsampling.  Choose among.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     b: Boxcar      : a simple averaging of all points inside filter domain.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c: Cosine arch : a weighted averaging with cosine arc weights.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     g: Gaussian    : weighted averaging with Gaussian weights [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     m: Median      : return the median (50%% quantile) value of all points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Apply directional illumination. Append the name of intensity grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set tile size as a power of 2 [256].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Sets file name prefix for image directory and KML file [GMT_Quadtree].\n");
	GMT_Option (API, "V,f.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRD2KML_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
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

			case 'C':	/* CPT */
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->C.file);
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Listing */
				Ctrl->D.active = true;
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
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -F: Choose among b, c, g, m!\n");
					n_errors++;
				}
				break;
			case 'N':	/* File name prefix */
				Ctrl->N.active = true;
				gmt_M_str_free (Ctrl->N.prefix);
				Ctrl->N.prefix = strdup (opt->arg);
				break;
			case 'I':	/* Here, intensity must be a grid file since we need to filter it */
				Ctrl->I.active = true;
				gmt_M_str_free (Ctrl->I.file);
				Ctrl->I.file = strdup (opt->arg);
				break;
			case 'L':	/* Tiles sizes */
				Ctrl->L.active = true;
				Ctrl->L.size = atoi (opt->arg);
				if (Ctrl->L.size <= 0 || ((log2 ((double)Ctrl->L.size) - irint (log2 ((double)Ctrl->L.size))) > GMT_CONV8_LIMIT)) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -L: Must be radix 2!\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->In.file == NULL, "Syntax error: Must specify a single grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.prefix == NULL, "Syntax error -F: Must specify a prefix for naming usage.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->E.url == NULL, "Syntax error -E: Must specify an URL.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && Ctrl->I.file == NULL, "Syntax error -I: Must specify an intensity grid file.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && (strstr (Ctrl->I.file, "+a") || strstr (Ctrl->I.file, "+n")), "Syntax error -I: Must specify an intensity grid file.\n");

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

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int gmtlib_geo_C_format (struct GMT_CTRL *GMT);

int GMT_grd2kml (void *V_API, int mode, void *args) {
	int error = 0, kk;
	bool flat = false;	/* Experimental */
	
	unsigned int level, max_level, n = 0, k, nx, ny, mx, my, row, col, n_skip, quad, n_alloc = GMT_CHUNK;

	double lonW, lonE, latS, latN, factor, dim, wesn[4];

	char cmd[GMT_BUFSIZ] = {""}, level_dir[GMT_LEN256] = {""}, Zgrid[GMT_LEN256] = {""}, Igrid[GMT_LEN256] = {""};
	char W[GMT_LEN16] = {""}, E[GMT_LEN16] = {""}, S[GMT_LEN16] = {""}, N[GMT_LEN16] = {""}, file[GMT_LEN256] = {""};

	FILE *fp = NULL;
	struct GMT_QUADTREE **Q = NULL;
	struct GRD2KML_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grd2kml main code ----------------------------*/

	/* Read grid header only to determine dimensions and required levels for the Pyramid */
	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (!gmt_M_is_geographic (GMT, GMT_IN)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Grid must be geographic (lon, lat)\n");
		Return (API->error);
	}
	if (!doubleAlmostEqual (G->header->inc[GMT_X], G->header->inc[GMT_Y])) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Grid spacing must be the same in longitude and latitude!\n");
		Return (API->error);
	}
	
	nx = G->header->n_columns;	ny = G->header->n_rows;			/* Dimensions of grid */
	mx = urint (ceil ((double)nx / (double)Ctrl->L.size)) * Ctrl->L.size;	/* Image in multiples of tile size */
	my = urint (ceil ((double)ny / (double)Ctrl->L.size)) * Ctrl->L.size;
	max_level = urint (ceil (log2 (MAX (mx, my) / (double)Ctrl->L.size)));		/* Number of levels in the pyramid */
	if ((36000.0 * G->header->inc[GMT_X] - irint (36000.0 * G->header->inc[GMT_X])) < GMT_CONV4_LIMIT) {
		/* Grid spacing is an integer multiple of arc sec or higher, use ddd:mm:ss.x format */
		strcpy (GMT->current.setting.format_geo_out, "ddd:mm:ss.x");
	}
	else {	/* Cannot use 0.1 arcsecs */
		strcpy (GMT->current.setting.format_float_out, "%.14g");
		strcpy (GMT->current.setting.format_geo_out, "D");
	}
	gmtlib_geo_C_format (GMT);
	dim = 0.01 * Ctrl->L.size;	/* Constant tile map size in inches for a fixed dpi of 100 yields PNGS of the requested dimension */
	
	GMT->current.io.geo.range = (G->header->wesn[XLO] < 0.0 && G->header->wesn[XHI] > 0.0) ? GMT_IS_M180_TO_P180_RANGE : GMT_IS_0_TO_P360_RANGE;
	/* Create the tile directory first */
#ifndef _WIN32
	if (mkdir (Ctrl->N.prefix, (mode_t)0777))
#else
	if (mkdir (Ctrl->N.prefix))
#endif
	{
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create directory (perhaps it already exists?): %s\n", Ctrl->N.prefix);
		Return (GMT_RUNTIME_ERROR);
	}
	
	wesn[XLO] = G->header->wesn[XLO];	wesn[YLO] = G->header->wesn[YLO];
	wesn[XHI] = G->header->wesn[XHI];	wesn[YHI] = G->header->wesn[YHI];

	Q = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_QUADTREE *);

	/* Loop over all the levels, starting at the top level (0) */
	for (level = 0; level <= max_level; level++) {
		factor = pow (2.0, max_level - level);	/* Width of imaged pixels in multiples of original grid spacing for this level */
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Level %d: Factor = %g Dim = %d x %d -> %d x %d\n",
			level, factor, irint (factor * Ctrl->L.size), irint (factor * Ctrl->L.size), Ctrl->L.size, Ctrl->L.size);
		/* Create the level directory */
		if (flat && level == 0) {
			sprintf (level_dir, "%s/files", Ctrl->N.prefix);
#ifndef _WIN32
			if (mkdir (level_dir, (mode_t)0777))
#else
			if (mkdir (level_dir))
#endif
			{
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to create level directory : %s\n", level_dir);
				Return (GMT_RUNTIME_ERROR);
			}
		}
		else if (!flat) {
			sprintf (level_dir, "%s/%d", Ctrl->N.prefix, level);
#ifndef _WIN32
			if (mkdir (level_dir, (mode_t)0777))
#else
			if (mkdir (level_dir))
#endif
			{
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to create level directory : %s\n", level_dir);
				Return (GMT_RUNTIME_ERROR);
			}
		}
		if (level < max_level) {	/* Filter the data to match level resolution */
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Level %d: Filtering down the grid(s)\n", level);
			sprintf (Zgrid, "grd2kml_Z_L%d_tmp.grd", level);
			sprintf (cmd, "%s -D0 -F%c%g -I%g -G%s", Ctrl->In.file, Ctrl->F.filter, factor * G->header->inc[GMT_Y], factor * G->header->inc[GMT_Y], Zgrid);
			if ((error = GMT_Call_Module (API, "grdfilter", GMT_MODULE_CMD, cmd)) != GMT_NOERROR) {
				Return (GMT_RUNTIME_ERROR);
			}
			if (Ctrl->I.active) {	/* Also filter the intensity grid */
				sprintf (Igrid, "grd2kml_I_L%d_tmp.grd", level);
				sprintf (cmd, "%s -D0 -F%c%g -I%g -G%s", Ctrl->I.file, Ctrl->F.filter, factor * G->header->inc[GMT_Y], factor * G->header->inc[GMT_Y], Igrid);
				if ((error = GMT_Call_Module (API, "grdfilter", GMT_MODULE_CMD, cmd)) != GMT_NOERROR) {
					Return (GMT_RUNTIME_ERROR);
				}
			}
		}
		else {	/* Use as is for the highest resolution */
			strcpy (Zgrid, Ctrl->In.file);
			if (Ctrl->I.active) strcpy (Igrid, Ctrl->I.file);
		}
		
		/* Loop over all rows at this level */
		row = n_skip = 0;
		latS = wesn[YLO];
		gmt_ascii_format_one (GMT, S, latS, GMT_IS_LAT);
		 
		while (latS < (wesn[YHI]-G->header->inc[GMT_Y])) {	/* Small correction to avoid issues due to round-off */
			latN = latS + factor * Ctrl->L.size * G->header->inc[GMT_Y];	/* Top row may extend beyond grid and be transparent */
			gmt_ascii_format_one (GMT, N, latN, GMT_IS_LAT);
			/* Loop over all columns at this level */
			col = 0;
			lonW = wesn[XLO];
			gmt_ascii_format_one (GMT, W, lonW, GMT_IS_LON);
			while (lonW < (wesn[XHI]-G->header->inc[GMT_X])) {
				lonE = lonW + factor * Ctrl->L.size * G->header->inc[GMT_X];	/* So right column may extend beyond grid and be transparent */
				gmt_ascii_format_one (GMT, E, lonE, GMT_IS_LON);
				/* Now we have the current tile region */
				/* Build the grdimage command to make the PostScript plot */
				if (Ctrl->I.active)
					sprintf (cmd, "%s -I%s -JX%3.2lfid -X0 -Y0 -Qn -R%s/%s/%s/%s -Vn --PS_MEDIA=%3.2lfix%3.2lfi ->grd2kml_tile_tmp.ps", Zgrid, Igrid, dim, W, E, S, N, dim, dim);
				else
					sprintf (cmd, "%s -JX%3.2lfid -X0 -Y0 -Qn -R%s/%s/%s/%s -Vn --PS_MEDIA=%3.2lfix%3.2lfi ->grd2kml_tile_tmp.ps", Zgrid, dim, W, E, S, N, dim, dim);
				if (Ctrl->C.active) {strcat (cmd, " -C"); strcat (cmd, Ctrl->C.file); }
				error = GMT_Call_Module (API, "grdimage", GMT_MODULE_CMD, cmd);	/* Create the PS */
				/* The -Qn will return the status GMT_IMAGE_NO_DATA if all pixels were NaN.  We dont need to include such tiles */
				if (error == GMT_IMAGE_NO_DATA) {
					GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Level %d: Tile %s/%s/%s/%s had no data - skipped\n", level, W, E, S, N);
					n_skip++;
				}
				else if (error) {	/* Failed badly */
					Return (API->error);
				}
				else {	/* Made a meaningful plot, time to rip. */
					/* Create the psconvert command to convert the PS to transparent PNG */
					GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Level %d: Tile %s/%s/%s/%s\n", level, W, E, S, N);
					if (flat)
						sprintf (cmd, "-TG -E100 -P -Vn -Z -D%s/files -FL%dR%dC%d.png grd2kml_tile_tmp.ps", Ctrl->N.prefix, level, row, col);
					else
						sprintf (cmd, "-TG -E100 -P -Vn -Z -D%s -FR%dC%d.png grd2kml_tile_tmp.ps", level_dir, row, col);
					if (GMT_Call_Module (API, "psconvert", GMT_MODULE_CMD, cmd))
						Return (API->error);
					/* Update our list of tiles */
					Q[n] = gmt_M_memory (GMT, NULL, 1, struct GMT_QUADTREE);
					Q[n]->row = row; Q[n]->col = col;	Q[n]->level = level;
					Q[n]->wesn[XLO] = lonW;	Q[n]->wesn[XHI] = lonE;
					Q[n]->wesn[YLO] = latS;	Q[n]->wesn[YHI] = latN;
					sprintf (Q[n]->tag, "L%2.2dR%2.2dC%2.2d", level, row, col);
					n++;
					if (n == n_alloc) {
						n_alloc <<= 1;
						Q = gmt_M_memory (GMT, Q, n_alloc, struct GMT_QUADTREE *);
					}
				}
				col++;	/* Onwards to next column */
				lonW = lonE;
				strcpy (W, E);
			}
			row++;	/* Onwards to next row */
			latS = latN;
			strcpy (S, N);
		}
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Level %d: %d by %d = %d tiles, %d mapped, %d skipped\n", level, row, col, row*col, row*col - n_skip, n_skip);
		if (level < max_level) {	/* Delete the temporary filtered grids */
			gmt_remove_file (GMT, Zgrid);
			if (Ctrl->I.active) gmt_remove_file (GMT, Igrid);
		}
	}
	if (!access ("grd2kml_tile_tmp.ps", F_OK))
		gmt_remove_file (GMT, "grd2kml_tile_tmp.ps");

	/* Process quadtree links */
	
	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Processes quadtree links for %d images\n", n);
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
			assert (kk >= 0 && quad < 4);	/* Sanity check */
			Q[kk]->next[quad] = Q[k];	/* Do the linking */
			Q[kk]->q++;			/* Count the links for this parent */
		}
	}
	
	/* Create the top-level KML file */ 
	sprintf (file, "%s/%s.kml", Ctrl->N.prefix, Ctrl->N.prefix);
	if ((fp = fopen (file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create file : %s\n", file);
		Return (GMT_RUNTIME_ERROR);
	}
	fprintf (fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n  <kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
	fprintf (fp, "    <Document>\n      <name>%s</name>\n", Ctrl->N.prefix);
	fprintf (fp, "      <description>GMT image quadtree</description>\n      <Style>\n        <ListStyle id=\"hideChildren\">\n");
        fprintf (fp, "          <listItemType>checkHideChildren</listItemType>\n        </ListStyle>\n      </Style>\n");
	if (flat)
		fprintf (fp, "      <NetworkLink>\n        <name>files/L0R0C0.png</name>\n");
	else if (Ctrl->E.active)
		fprintf (fp, "      <NetworkLink>\n        <name>%s/0/R0C0.png</name>\n", Ctrl->E.url);
	else
		fprintf (fp, "      <NetworkLink>\n        <name>0/R0C0.png</name>\n");
        fprintf (fp, "        <Region>\n          <LatLonAltBox>\n");
	fprintf (fp, "            <north>%.14g</north>\n", wesn[YHI]);
	fprintf (fp, "            <south>%.14g</south>\n", wesn[YLO]);
	fprintf (fp, "            <east>%.14g</east>\n",   wesn[XHI]);
	fprintf (fp, "            <west>%.14g</west>\n",   wesn[XLO]);
	fprintf (fp, "          </LatLonAltBox>\n");
	fprintf (fp, "          <Lod>\n            <minLodPixels>128</minLodPixels>\n            <maxLodPixels>-1</maxLodPixels>\n          </Lod>\n");
        fprintf (fp, "        </Region>\n");
	if (flat)
		fprintf (fp, "        <Link>\n          <href>files/L0R0C0.kml</href>\n");
	else if (Ctrl->E.active)
		fprintf (fp, "        <Link>\n          <href>%s/0/R0C0.kml</href>\n", Ctrl->E.url);
	else
		fprintf (fp, "        <Link>\n          <href>0/R0C0.kml</href>\n");
	fprintf (fp, "          <viewRefreshMode>onRegion</viewRefreshMode>\n          <viewFormat/>\n");
	fprintf (fp, "        </Link>\n      </NetworkLink>\n");
	fprintf (fp, "    </Document>\n  </kml>\n");
	fclose (fp);

	/* Then create all the other KML files in the quadtree with their links down the tree */
	
	for (k = 0; k < n; k++) {
		if (Q[k]->q) {	/* Only examine tiles with children */
			if (Ctrl->L.active) {
				printf ("%s:", Q[k]->tag);
				for (quad = 0; quad < 4; quad++)
					if (Q[k]->next[quad]) printf (" %c=%s", 'A'+quad, Q[k]->next[quad]->tag);
				printf ("\n");
			}
			if (flat)
				sprintf (file, "%s/files/L%dR%dC%d.kml", Ctrl->N.prefix, Q[k]->level, Q[k]->row, Q[k]->col);
			else if (Ctrl->E.active)
				sprintf (file, "%s/%s/%d/R%dC%d.kml", Ctrl->E.url, Ctrl->N.prefix, Q[k]->level, Q[k]->row, Q[k]->col);
			else
				sprintf (file, "%s/%d/R%dC%d.kml", Ctrl->N.prefix, Q[k]->level, Q[k]->row, Q[k]->col);
			if ((fp = fopen (file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to create file : %s\n", file);
				Return (GMT_RUNTIME_ERROR);
			}
			/* First this tile's kml and png */
			fprintf (fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n  <kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
			if (flat)
				fprintf (fp, "    <Document>\n      <name>L%dR%dC%d.kml</name>\n", Q[k]->level, Q[k]->row, Q[k]->col);
			else if (Ctrl->E.active)
				fprintf (fp, "    <Document>\n      <name>%s/%d/R%dC%d.kml</name>\n", Ctrl->E.url, Q[k]->level, Q[k]->row, Q[k]->col);
			else
				fprintf (fp, "    <Document>\n      <name>%d/R%dC%d.kml</name>\n", Q[k]->level, Q[k]->row, Q[k]->col);
			fprintf (fp, "      <description></description>\n      <Style>\n        <ListStyle id=\"hideChildren\">\n");
		        fprintf (fp, "          <listItemType>checkHideChildren</listItemType>\n        </ListStyle>\n      </Style>\n");
		        fprintf (fp, "      <Region>\n        <LatLonAltBox>\n");
			fprintf (fp, "          <north>%.14g</north>\n", Q[k]->wesn[YHI]);
			fprintf (fp, "          <south>%.14g</south>\n", Q[k]->wesn[YLO]);
			fprintf (fp, "          <east>%.14g</east>\n",   Q[k]->wesn[XHI]);
			fprintf (fp, "          <west>%.14g</west>\n",   Q[k]->wesn[XLO]);
			fprintf (fp, "        </LatLonAltBox>\n");
			fprintf (fp, "        <Lod>\n          <minLodPixels>128</minLodPixels>\n          <maxLodPixels>2048</maxLodPixels>\n        </Lod>\n");
		        fprintf (fp, "      </Region>\n");
			fprintf (fp, "      <GroundOverlay>\n        <drawOrder>%d</drawOrder>\n", 10+2*Q[k]->level);
			if (flat)
				fprintf (fp, "        <Icon>\n          <href>L%dR%dC%d.png</href>\n        </Icon>\n", Q[k]->level, Q[k]->row, Q[k]->col);
			else if (Ctrl->E.active)
				fprintf (fp, "        <Icon>\n          <href>%s/%d/R%dC%d.png</href>\n        </Icon>\n", Ctrl->E.url, Q[k]->level, Q[k]->row, Q[k]->col);
			else
				fprintf (fp, "        <Icon>\n          <href>R%dC%d.png</href>\n        </Icon>\n", Q[k]->row, Q[k]->col);
		        fprintf (fp, "        <LatLonBox>\n");
			fprintf (fp, "           <north>%.14g</north>\n", Q[k]->wesn[YHI]);
			fprintf (fp, "           <south>%.14g</south>\n", Q[k]->wesn[YLO]);
			fprintf (fp, "           <east>%.14g</east>\n",   Q[k]->wesn[XHI]);
			fprintf (fp, "           <west>%.14g</west>\n",   Q[k]->wesn[XLO]);
			fprintf (fp, "        </LatLonBox>\n      </GroundOverlay>\n");
			/* Now add up to 4 quad links */
			for (quad = 0; quad < 4; quad++) {
				if (Q[k]->next[quad] == NULL) continue;
				if (flat)
					fprintf (fp, "\n      <NetworkLink>\n        <name>L%dR%dC%d.png</name>\n", Q[k]->next[quad]->level, Q[k]->next[quad]->row, Q[k]->next[quad]->col);
				else if (Ctrl->E.active)
					fprintf (fp, "\n      <NetworkLink>\n        <name>%s/%d/R%dC%d.png</name>\n", Ctrl->E.url, Q[k]->next[quad]->level, Q[k]->next[quad]->row, Q[k]->next[quad]->col);
				else
					fprintf (fp, "\n      <NetworkLink>\n        <name>%d/R%dC%d.png</name>\n", Q[k]->next[quad]->level, Q[k]->next[quad]->row, Q[k]->next[quad]->col);
			        fprintf (fp, "        <Region>\n          <LatLonAltBox>\n");
				fprintf (fp, "            <north>%.14g</north>\n", Q[k]->next[quad]->wesn[YHI]);
				fprintf (fp, "            <south>%.14g</south>\n", Q[k]->next[quad]->wesn[YLO]);
				fprintf (fp, "            <east>%.14g</east>\n",   Q[k]->next[quad]->wesn[XHI]);
				fprintf (fp, "            <west>%.14g</west>\n",   Q[k]->next[quad]->wesn[XLO]);
				fprintf (fp, "        </LatLonAltBox>\n");
				fprintf (fp, "        <Lod>\n          <minLodPixels>128</minLodPixels>\n          <maxLodPixels>-1</maxLodPixels>\n        </Lod>\n");
			        fprintf (fp, "        </Region>\n");
				if (flat)
					fprintf (fp, "        <Link>\n          <href>L%dR%dC%d.kml</href>\n", Q[k]->next[quad]->level, Q[k]->next[quad]->row, Q[k]->next[quad]->col);
				else if (Ctrl->E.active)
					fprintf (fp, "        <Link>\n          <href>%s/%d/R%dC%d.kml</href>\n", Ctrl->E.url, Q[k]->next[quad]->level, Q[k]->next[quad]->row, Q[k]->next[quad]->col);
				else
					fprintf (fp, "        <Link>\n          <href>../%d/R%dC%d.kml</href>\n", Q[k]->next[quad]->level, Q[k]->next[quad]->row, Q[k]->next[quad]->col);
				fprintf (fp, "          <viewRefreshMode>onRegion</viewRefreshMode><viewFormat/>\n");
				fprintf (fp, "        </Link>\n      </NetworkLink>\n");
			}
			fprintf (fp, "    </Document>\n  </kml>\n");
			fclose (fp);
		        
		}
		gmt_M_free (GMT, Q[k]);	/* Free this tile information */
	}
	gmt_M_free (GMT, Q);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Done!\n");
	Return (GMT_NOERROR);
}
