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
 * grdblend reads any number of grid files that may partly overlap and
 * creates a blend of all the files given certain criteria.  Each input
 * grid is considered to have an "outer" and "inner" region.  The outer
 * region is the extent of the grid; the inner region is provided as
 * input in the form of a -Rw/e/s/n statement.  Finally, each grid can
 * be assigned its separate weight.  This information is given to the
 * program in ASCII format, one line per grid file; each line looks like
 *
 * grdfile	-Rw/e/s/n	weight
 *
 * The blending will use a 2-D cosine taper between the inner and outer
 * regions.  The output at any node is thus a weighted average of the
 * values from any grid that occupies that grid node.  Because the out-
 * put grid can be really huge (say global grids at fine resolution),
 * all grid input/output is done row by row so memory should not be a
 * limiting factor in making large grid.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdblend"
#define THIS_MODULE_MODERN_NAME	"grdblend"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Blend several partially overlapping grids into one larger grid"
#define THIS_MODULE_KEYS	"<G{+,GG}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVfnr"

#define BLEND_UPPER	0
#define BLEND_LOWER	1
#define BLEND_FIRST	2
#define BLEND_LAST	3

struct GRDBLEND_CTRL {
	struct GRDBLEND_In {	/* Input files */
		bool active;
		char **file;
		unsigned int n;	/* If n > 1 we probably got *.grd or something */
	} In;
	struct GRDBLEND_G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct GRDBLEND_C {	/* -Cf|l|o|u[+n|p] */
		bool active;
		unsigned int mode;
		int sign;
	} C;
	struct GRDBLEND_N {	/* -N<nodata> */
		bool active;
		double nodata;
	} N;
	struct GRDBLEND_Q {	/* -Q */
		bool active;
	} Q;
	struct GRDBLEND_Z {	/* -Z<scale> */
		bool active;
		double scale;
	} Z;
	struct GRDBLEND_W {	/* -W[z] */
		bool active;
		unsigned int mode;	/* 1 if -Wz was given */
	} W;
};

struct GRDBLEND_INFO {	/* Structure with info about each input grid file */
	struct GMT_GRID *G;				/* I/O structure for grid files, including grd header */
	struct GMT_GRID_HEADER_HIDDEN *HH;
	struct GMT_GRID_ROWBYROW *RbR;
	int in_i0, in_i1, out_i0, out_i1;		/* Indices of outer and inner x-coordinates (in output grid coordinates) */
	int in_j0, in_j1, out_j0, out_j1;		/* Indices of outer and inner y-coordinates (in output grid coordinates) */
	off_t offset;					/* grid offset when the grid extends beyond north */
	off_t skip;					/* Byte offset to skip in native binary files */
	off_t pos;					/* Current byte offset for native binary files */
	bool ignore;					/* true if the grid is entirely outside desired region */
	bool outside;				/* true if the current output row is outside the range of this grid */
	bool invert;					/* true if weight was given as negative and we want to taper to zero INSIDE the grid region */
	bool open;					/* true if file is currently open */
	bool delete;					/* true if file was produced by grdsample to deal with different registration/increments */
	bool memory;					/* true if grid is a in memory array */
	char file[PATH_MAX];			/* Name of grid file */
	double weight, wt_y, wxr, wxl, wyu, wyd;	/* Various weighting factors used for cosine-taper weights */
	double wesn[4];					/* Boundaries of inner region */
	gmt_grdfloat *z;					/* Row vector holding the current row from this file */
};

#ifdef HAVE_GDAL
#define N_NOT_SUPPORTED	8
#else
#define N_NOT_SUPPORTED	7
#endif

GMT_LOCAL int found_unsupported_format (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, char *file) {
	/* Check that grid files are not among the unsupported formats that has no row-by-row io yet */
	unsigned int i, type;
#ifdef HAVE_GDAL
	static char *not_supported[N_NOT_SUPPORTED] = {"rb", "rf", "sf", "sd", "af", "ei", "ef", "gd"};
#else
	static char *not_supported[N_NOT_SUPPORTED] = {"rb", "rf", "sf", "sd", "af", "ei", "ef"};
#endif
	for (i = 0; i < N_NOT_SUPPORTED; i++) {	/* Only allow netcdf (both v3 and new) and native binary output */
		if (gmt_grd_format_decoder (GMT, not_supported[i], &type) != GMT_NOERROR) {
			/* no valid type id - which should not happen unless typo in not_supported array */
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Very odd - should not happen [format = %s]. Post a note on the gmt user forum\n", not_supported[i]);
			return (GMT_GRDIO_UNKNOWN_FORMAT);
		}
		if (h->type == type) {	/* Our file is in one of the unsupported formats that cannot do row-by-row i/o */
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Grid format type %s for file %s is not supported for row-by-row i/o.\n", not_supported[i], file);
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "We will create a temporary output file which will be converted (via grdconvert) to your chosen format.\n");
			return (1);
		}
	}
	return (GMT_NOERROR);
}

GMT_LOCAL void decode_R (struct GMT_CTRL *GMT, char *string, double wesn[]) {
	unsigned int i, pos, error = 0;
	char text[GMT_BUFSIZ];

	/* Needed to decode the inner region -Rw/e/s/n string */

	i = pos = 0;
	while (!error && (gmt_strtok (string, "/", &pos, text))) {
		error += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, i/2), gmt_scanf_arg (GMT, text, gmt_M_type (GMT, GMT_IN, i/2), true, &wesn[i]), text);
		i++;
	}
	if (error || (i != 4) || gmt_check_region (GMT, wesn)) {
		gmt_syntax (GMT, 'R');
	}
}

GMT_LOCAL bool out_of_phase (struct GMT_GRID_HEADER *g, struct GMT_GRID_HEADER *h) {
	/* Look for phase shifts in w/e/s/n between the two grids */
	unsigned int way, side;
	double a;
	for (side = 0; side < 4; side++) {
		way = side / 2;
		a = fmod (fabs (((g->wesn[side] + g->xy_off * g->inc[way]) - (h->wesn[side] + h->xy_off * h->inc[way])) / h->inc[way]), 1.0);
		if (a < GMT_CONV8_LIMIT) continue;
		a = 1.0 - a;
		if (a < GMT_CONV8_LIMIT) continue;
		return true;
	}
	return false;
}

GMT_LOCAL bool overlap_check (struct GMT_CTRL *GMT, struct GRDBLEND_INFO *B, struct GMT_GRID_HEADER *h, unsigned int mode) {
	double w, e, shift = 720.0;
	char *type[2] = {"grid", "inner grid"};

	if (gmt_grd_is_global (GMT, h)) return false;	/* Not possible to be outside the final grids longitude range if global */
	if (gmt_grd_is_global (GMT, B->G->header)) return false;	/* Not possible to overlap with the final grid in longitude range if your are a global grid */
	/* Here the grids are not global so we must carefully check for overlap while being aware of periodicity in 360 degrees */
	w = ((mode) ? B->wesn[XLO] : B->G->header->wesn[XLO]) - shift;	e = ((mode) ? B->wesn[XHI] : B->G->header->wesn[XHI]) - shift;
	while (e < h->wesn[XLO]) { w += 360.0; e += 360.0; shift -= 360.0; }
	if (w > h->wesn[XHI]) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "File %s entirely outside longitude range of final grid region (skipped)\n", B->file);
		B->ignore = true;
		return true;
	}
	if (! (gmt_M_is_zero (shift))) {	/* Must modify region */
		if (mode) {
			B->wesn[XLO] = w;	B->wesn[XHI] = e;
		}
		else {
			B->G->header->wesn[XLO] = w;	B->G->header->wesn[XHI] = e;
		}
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "File %s %s region needed longitude adjustment to fit final grid region\n", B->file, type[mode]);
	}
	return false;
}

GMT_LOCAL int init_blend_job (struct GMT_CTRL *GMT, char **files, unsigned int n_files, struct GMT_GRID_HEADER **h_ptr, struct GRDBLEND_INFO **blend, unsigned int *zmode) {
	int type, status, not_supported = 0;
	unsigned int one_or_zero, n = 0, nr, do_sample, n_download = 0, down = 0, srtm_res = 0;
	bool srtm_job = false, common_inc = true, common_reg = true;
	struct GRDBLEND_INFO *B = NULL;
	struct GMT_GRID_HEADER *h = *h_ptr;	/* Input header may be NULL or preset */
	struct GMT_GRID_HIDDEN *GH = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	char *sense[2] = {"normal", "inverse"}, buffer[GMT_BUFSIZ] = {""};
	static char *V_level = "qntcvld";
	char Iargs[GMT_LEN256] = {""}, Rargs[GMT_LEN256] = {""}, cmd[GMT_LEN256] = {""};
	double wesn[4], sub = 0.0;
	struct BLEND_LIST {
		char *file;
		char *region;
		double weight;
		bool download;
	} *L = NULL;

	*zmode = GMT_DEFAULT_CPT;	/* This is the default CPT for any data type */
	
	if (n_files > 1) {	/* Got a bunch of grid files */
		L = gmt_M_memory (GMT, NULL, n_files, struct BLEND_LIST);
		for (n = 0; n < n_files; n++) {
			L[n].file = strdup (files[n]);
			L[n].region = strdup ("-");	/* inner == outer region */
			L[n].weight = 1.0;		/* Default weight */
		}
	}
	else {	/* Must read blend file */
		size_t n_alloc = 0;
		unsigned int res;
		struct GMT_RECORD *In = NULL;
		char r_in[GMT_LEN256] = {""}, file[PATH_MAX] = {""};
		double weight;
		gmt_set_meminc (GMT, GMT_SMALL_CHUNK);
		
		do {	/* Keep returning records until we reach EOF */
			if ((In = GMT_Get_Record (GMT->parent, GMT_READ_TEXT, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
					return (GMT_RUNTIME_ERROR);
				else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
					break;
				continue;							/* Go back and read the next record */
			}
			/* Data record to process */
			
			/* Data record to process.  We permint this kind of records:
			 * file [-Rinner_region ] [weight]
			 * i.e., file is required but region [grid extent] and/or weight [1] are optional
			 */

			nr = sscanf (In->text, "%s %s %lf", file, r_in, &weight);
			if (nr < 1) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Read error for blending parameters near row %d\n", n);
				gmt_M_free (GMT, L);
				return (GMT_DATA_READ_ERROR);
			}
			if (n == n_alloc) L = gmt_M_malloc (GMT, L, n, &n_alloc, struct BLEND_LIST);
			L[n].file = strdup (file);
			L[n].region = (nr > 1 && r_in[0] == '-' && r_in[1] == 'R') ? strdup (r_in) : strdup ("-");
			if (n == 2 && !(r_in[0] == '-' && (r_in[1] == '\0' || r_in[1] == 'R'))) weight = atof (r_in);	/* Got "file weight" record */
			L[n].weight = (nr == 1 || (n == 2 && r_in[0] == '-')) ? 1.0 : weight;	/* Default weight is 1 if none were given */
			if (gmt_file_is_srtmtile (GMT->parent, L[n].file, &res)) {
				srtm_res = res;
				srtm_job = true;
				if (gmt_access (GMT, &L[n].file[1], F_OK)) {	/* Tile must be downloaded */
					L[n].download = true;
					n_download++;
				}
			}
			n++;
		} while (true);
		gmt_reset_meminc (GMT);
		n_files = n;
	}
	if (srtm_job) {	/* Signal default CPT for earth or srtm relief final grid */
		*zmode = (!strcmp (L[n_files-1].file, "@earth_relief_15s")) ? 1 : 2;
		*zmode += 10 * srtm_res;
		if (h) {
			if (h->wesn[XHI] > 180.0) {
				sub = 360.0;
				h->wesn[XLO] -= sub;
				h->wesn[XHI] -= sub;
			}
			else if (h->wesn[XLO] < -180.0) {
				sub = -360.0;
				h->wesn[XLO] -= sub;
				h->wesn[XHI] -= sub;
			}
		}
	}
	
	B = gmt_M_memory (GMT, NULL, n_files, struct GRDBLEND_INFO);
	wesn[XLO] = wesn[YLO] = DBL_MAX;	wesn[XHI] = wesn[YHI] = -DBL_MAX;
	
	for (n = 0; n < n_files; n++) {	/* Process each input grid */

		if (L[n].download) {	/* Report that we will be downloading this SRTM tile */
			char tile[8] = {""};
			strncpy (tile, &L[n].file[1], 7U);
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Downloading SRTM%d tile %d of %d [%s]\n", srtm_res, ++down, n_download, tile);
		}
			
		strncpy (B[n].file, L[n].file, PATH_MAX-1);
		B[n].memory = gmt_M_file_is_memory (B[n].file);	/* If grid in memory then we only read once and have everything at once */
		if ((B[n].G = GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY|GMT_GRID_ROW_BY_ROW, NULL, B[n].file, NULL)) == NULL) {
			/* Failure somehow, free all grids read so far and bail */
			for (n = 0; n < n_files; n++) {
				gmt_M_str_free (L[n].file);	gmt_M_str_free (L[n].region);
			}
			gmt_M_free (GMT, L);	gmt_M_free (GMT, B);
			return (-1);
		}
		
		if ((not_supported = found_unsupported_format (GMT, B[n].G->header, B[n].file)) == GMT_GRDIO_UNKNOWN_FORMAT) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal snafu - please report the problem on the GMT issues page\n");
			return (-1);
		}
		B[n].HH = gmt_get_H_hidden (B[n].G->header);
		B[n].weight = L[n].weight;
		if (!strcmp (L[n].region, "-"))
			gmt_M_memcpy (B[n].wesn, B[n].G->header->wesn, 4, double);	/* Set inner = outer region */
		else
			decode_R (GMT, &L[n].region[2], B[n].wesn);			/* Must decode the -R string */
		if (h == NULL) {	/* Was not given -R, determine it from the input grids */
			if (B[n].G->header->wesn[YLO] < wesn[YLO]) wesn[YLO] = B[n].G->header->wesn[YLO];
			if (B[n].G->header->wesn[YHI] > wesn[YHI]) wesn[YHI] = B[n].G->header->wesn[YHI];
			if (B[n].G->header->wesn[XLO] < wesn[XLO]) wesn[XLO] = B[n].G->header->wesn[XLO];
			if (B[n].G->header->wesn[XHI] > wesn[XHI]) wesn[XHI] = B[n].G->header->wesn[XHI];
			if (n > 0) {
				if (fabs((B[n].G->header->inc[GMT_X] - B[0].G->header->inc[GMT_X]) / B[0].G->header->inc[GMT_X]) > 0.002 ||
					fabs((B[n].G->header->inc[GMT_Y] - B[0].G->header->inc[GMT_Y]) / B[0].G->header->inc[GMT_Y]) > 0.002)
						common_inc = false;
				if (B[n].G->header->registration != B[0].G->header->registration)
					common_reg = false;
			}
		}
		gmt_M_str_free (L[n].file);	/* Done with these now */
		gmt_M_str_free (L[n].region);
	}
	gmt_M_free (GMT, L);	/* Done with this now */
	
	if (h == NULL) {	/* Must use the common region from the tiles */
		if (!common_inc) {
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE,
			            "Must specify -I if input grids have different increments\n");
			return (-1);
		}
		/* Create the h structure and initialize it */
		h = gmt_get_header (GMT);
		gmt_M_memcpy (h->wesn, wesn, 4, double);
		gmt_M_memcpy (h->inc, B[0].G->header->inc, 2, double);
		h->registration = B[0].G->header->registration;
		gmt_M_grd_setpad (GMT, h, GMT->current.io.pad); /* Assign default pad */
		gmt_set_grddim (GMT, h);	/* Update dimensions */
		*h_ptr = h;			/* Pass out the updated settings */
	}
	
	HH = gmt_get_H_hidden (h);
	one_or_zero = !h->registration;
			
	for (n = 0; n < n_files; n++) {	/* Process each input grid */
		/* Skip the file if its outer region does not lie within the final grid region */
		if (h->wesn[YLO] > B[n].wesn[YHI] || h->wesn[YHI] < B[n].wesn[YLO]) {
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE,
			            "File %s entirely outside y-range of final grid region (skipped)\n", B[n].file);
			B[n].ignore = true;
			continue;
		}
		if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must carefully check the longitude overlap */
			if (overlap_check (GMT, &B[n], h, 0)) continue;	/* Check header for -+360 issues and overlap */
			if (overlap_check (GMT, &B[n], h, 1)) continue;	/* Check inner region for -+360 issues and overlap */
		}
		else if (h->wesn[XLO] > B[n].wesn[XHI] || h->wesn[XHI] < B[n].wesn[XLO] || h->wesn[YLO] > B[n].wesn[YHI] || h->wesn[YHI] < B[n].wesn[YLO]) {
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE,
			            "File %s entirely outside x-range of final grid region (skipped)\n", B[n].file);
			B[n].ignore = true;
			continue;
		}

		/* If input grids have different spacing or registration we must resample */

		Iargs[0] = Rargs[0] = '\0';
		do_sample = 0;
		if (not_supported) {
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE,
			            "File %s not supported via row-by-row read - must reformat first\n", B[n].file);
			do_sample |= 2;
		}
		if (fabs((B[n].G->header->inc[GMT_X] - h->inc[GMT_X]) / h->inc[GMT_X]) > 0.002 ||
			fabs((B[n].G->header->inc[GMT_Y] - h->inc[GMT_Y]) / h->inc[GMT_Y]) > 0.002) {
			sprintf (Iargs, "-I%.12g/%.12g", h->inc[GMT_X], h->inc[GMT_Y]);
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "File %s has different increments (%.12g/%.12g) than the output grid (%.12g/%.12g) - must resample\n",
				B[n].file, B[n].G->header->inc[GMT_X], B[n].G->header->inc[GMT_Y], h->inc[GMT_X], h->inc[GMT_Y]);
			do_sample |= 1;
		}
		if (out_of_phase (B[n].G->header, h)) {	/* Set explicit -R for resampling that is multiple of desired increments AND inside both original grid and desired grid */
			double wesn[4];	/* Make sure wesn is equal to or larger than B[n].G->header->wesn so all points are included */
			unsigned int k;
			k = (unsigned int)rint ((MAX (h->wesn[XLO], B[n].G->header->wesn[XLO]) - h->wesn[XLO]) / h->inc[GMT_X] - h->xy_off);
			wesn[XLO] = gmt_M_grd_col_to_x (GMT, k, h);
			k = (unsigned int)rint  ((MIN (h->wesn[XHI], B[n].G->header->wesn[XHI]) - h->wesn[XLO]) / h->inc[GMT_X] - h->xy_off);
			wesn[XHI] = gmt_M_grd_col_to_x (GMT, k, h);
			k = h->n_rows - 1 - (unsigned int)rint ((MAX (h->wesn[YLO], B[n].G->header->wesn[YLO]) - h->wesn[YLO]) / h->inc[GMT_Y] - h->xy_off);
			wesn[YLO] = gmt_M_grd_row_to_y (GMT, k, h);
			k = h->n_rows - 1 - (unsigned int)rint  ((MIN (h->wesn[YHI], B[n].G->header->wesn[YHI]) - h->wesn[YLO]) / h->inc[GMT_Y] - h->xy_off);
			wesn[YHI] = gmt_M_grd_row_to_y (GMT, k, h);
			sprintf (Rargs, "-R%.12g/%.12g/%.12g/%.12g", wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "File %s coordinates are phase-shifted w.r.t. the output grid - must resample\n", B[n].file);
			do_sample |= 1;
		}
		else if (do_sample) {	/* Set explicit -R to handle possible subsetting */
			double wesn[4];
			gmt_M_memcpy (wesn, h->wesn, 4, double);
			if (wesn[XLO] < B[n].G->header->wesn[XLO]) wesn[XLO] = B[n].G->header->wesn[XLO];
			if (wesn[XHI] > B[n].G->header->wesn[XHI]) wesn[XHI] = B[n].G->header->wesn[XHI];
			if (wesn[YLO] < B[n].G->header->wesn[YLO]) wesn[YLO] = B[n].G->header->wesn[YLO];
			if (wesn[YHI] > B[n].G->header->wesn[YHI]) wesn[YHI] = B[n].G->header->wesn[YHI];
			sprintf (Rargs, "-R%.12g/%.12g/%.12g/%.12g", wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "File %s is sampled using region %s\n", B[n].file, Rargs);
		}
		if (do_sample) {	/* One or more reasons to call upon grdsample before using this grid */
			if (do_sample & 1) {	/* Resampling of the grid into a netcdf grid */
				if (GMT->parent->tmp_dir)	/* Use the established temp directory */
					sprintf (buffer, "%s/grdblend_resampled_%d_%d.nc", GMT->parent->tmp_dir, (int)getpid(), n);
				else	/* Must dump it in current directory */
					sprintf (buffer, "grdblend_resampled_%d_%d.nc", (int)getpid(), n);
				snprintf (cmd, GMT_LEN256, "%s %s %s %s -G%s -V%c", B[n].file, h->registration ? "-r" : "",
				         Iargs, Rargs, buffer, V_level[GMT->current.setting.verbose]);
				if (gmt_M_is_geographic (GMT, GMT_IN)) strcat (cmd, " -fg");
				strcat (cmd, " --GMT_HISTORY=false");
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Resample %s via grdsample %s\n", B[n].file, cmd);
				if ((status = GMT_Call_Module (GMT->parent, "grdsample", GMT_MODULE_CMD, cmd))) {	/* Resample the file */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to resample file %s - exiting\n", B[n].file);
					GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
				}
			}
			else {	/* Just reformat to netCDF so this grid may be used as well */
				if (srtm_job)
					GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE,
					            "Convert SRTM%d tile from JPEG2000 to netCDF grid [%s]\n", srtm_res, B[n].file);
				if (GMT->parent->tmp_dir)	/* Use the established temp directory */
					sprintf (buffer, "%s/grdblend_reformatted_%d_%d.nc", GMT->parent->tmp_dir, (int)getpid(), n);
				else	/* Must dump it in current directory */
					sprintf (buffer, "grdblend_reformatted_%d_%d.nc", (int)getpid(), n);
				snprintf (cmd, GMT_LEN256, "%s %s %s -V%c", B[n].file, Rargs, buffer, V_level[GMT->current.setting.verbose]);
				if (gmt_M_is_geographic (GMT, GMT_IN)) strcat (cmd, " -fg");
				strcat (cmd, " --GMT_HISTORY=false");
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Reformat %s via grdconvert %s\n", B[n].file, cmd);
				if ((status = GMT_Call_Module (GMT->parent, "grdconvert", GMT_MODULE_CMD, cmd))) {	/* Resample the file */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to resample file %s - exiting\n", B[n].file);
					GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
				}
			}
			strncpy (B[n].file, buffer, PATH_MAX-1);	/* Use the temporary file instead */
			B[n].delete = true;		/* Flag to delete this temporary file when done */
			if (GMT_Destroy_Data (GMT->parent, &B[n].G))
				return (-1);
			if ((B[n].G = GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY|GMT_GRID_ROW_BY_ROW, NULL, B[n].file, NULL)) == NULL) {
				return (-1);
			}
			if (overlap_check (GMT, &B[n], h, 0)) continue;	/* In case grdconvert changed the region */
		}
		if (B[n].weight < 0.0) {	/* Negative weight means invert sense of taper */
			B[n].weight = fabs (B[n].weight);
			B[n].invert = true;
		}
		B[n].RbR = gmt_M_memory (GMT, NULL, 1, struct GMT_GRID_ROWBYROW);		/* Allocate structure */
		GH = gmt_get_G_hidden (B[n].G);
		if (GH->extra != NULL)	/* Only memory grids will fail this test */
			gmt_M_memcpy (B[n].RbR, GH->extra, 1, struct GMT_GRID_ROWBYROW);	/* Duplicate, since GMT_Destroy_Data will free the header->extra */

		/* Here, i0, j0 is the very first col, row to read, while i1, j1 is the very last col, row to read .
		 * Weights at the outside i,j should be 0, and reach 1 at the edge of the inside block */

		/* The following works for both pixel and grid-registered grids since we are here using the i,j to measure the width of the
		 * taper zone in units of dx, dy. */
		 
		B[n].out_i0 = irint ((B[n].G->header->wesn[XLO] - h->wesn[XLO]) * HH->r_inc[GMT_X]);
		B[n].in_i0  = irint ((B[n].wesn[XLO] - h->wesn[XLO]) * HH->r_inc[GMT_X]) - 1;
		B[n].in_i1  = irint ((B[n].wesn[XHI] - h->wesn[XLO]) * HH->r_inc[GMT_X]) + one_or_zero;
		B[n].out_i1 = irint ((B[n].G->header->wesn[XHI] - h->wesn[XLO]) * HH->r_inc[GMT_X]) - B[n].G->header->registration;
		B[n].out_j0 = irint ((h->wesn[YHI] - B[n].G->header->wesn[YHI]) * HH->r_inc[GMT_Y]);
		B[n].in_j0  = irint ((h->wesn[YHI] - B[n].wesn[YHI]) * HH->r_inc[GMT_Y]) - 1;
		B[n].in_j1  = irint ((h->wesn[YHI] - B[n].wesn[YLO]) * HH->r_inc[GMT_Y]) + one_or_zero;
		B[n].out_j1 = irint ((h->wesn[YHI] - B[n].G->header->wesn[YLO]) * HH->r_inc[GMT_Y]) - B[n].G->header->registration;

		B[n].wxl = M_PI * h->inc[GMT_X] / (B[n].wesn[XLO] - B[n].G->header->wesn[XLO]);
		B[n].wxr = M_PI * h->inc[GMT_X] / (B[n].G->header->wesn[XHI] - B[n].wesn[XHI]);
		B[n].wyu = M_PI * h->inc[GMT_Y] / (B[n].G->header->wesn[YHI] - B[n].wesn[YHI]);
		B[n].wyd = M_PI * h->inc[GMT_Y] / (B[n].wesn[YLO] - B[n].G->header->wesn[YLO]);

		if (B[n].out_j0 < 0) {	/* Must skip to first row inside the present -R */
			type = GMT->session.grdformat[B[n].G->header->type][0];
			if (type == 'c')	/* Old-style, 1-D netcdf grid */
				B[n].offset = B[n].G->header->n_columns * abs (B[n].out_j0);
			else if (type == 'n')	/* New, 2-D netcdf grid */
				B[n].offset = B[n].out_j0;
			else
				B[n].skip = (off_t)(B[n].RbR->n_byte * abs (B[n].out_j0));	/* do the fseek when we are ready to read first row */
		}
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Grid %s: out: %d/%d/%d/%d in: %d/%d/%d/%d skip: %d offset: %d\n",
			B[n].file, B[n].out_i0, B[n].out_i1, B[n].out_j1, B[n].out_j0, B[n].in_i0, B[n].in_i1, B[n].in_j1, B[n].in_j0, (int)B[n].skip, (int)B[n].offset);

		/* Allocate space for one entire row for this grid */

		B[n].z = gmt_M_memory (GMT, NULL, B[n].G->header->n_columns, gmt_grdfloat);
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Blend file %s in %g/%g/%g/%g with %s weight %g [%d-%d]\n",
			B[n].HH->name, B[n].wesn[XLO], B[n].wesn[XHI], B[n].wesn[YLO], B[n].wesn[YHI], sense[B[n].invert], B[n].weight, B[n].out_j0, B[n].out_j1);

		if (!B[n].memory && GMT_Destroy_Data (GMT->parent, &B[n].G)) return (-1);	/* Free grid unless it is a memory grid */
	}

	if (fabs (sub) > 0.0) {	/* Must undo shift earlier */
		h->wesn[XLO] += sub;
		h->wesn[XHI] += sub;
	}
	*blend = B;	/* Pass back array of structures with grid information */

	return (n_files);
}

GMT_LOCAL int sync_input_rows (struct GMT_CTRL *GMT, int row, struct GRDBLEND_INFO *B, unsigned int n_blend, double half) {
	unsigned int k;
	int G_row;
	uint64_t node;

	for (k = 0; k < n_blend; k++) {	/* Get every input grid ready for the new row */
		if (B[k].ignore) continue;	/* This grid is not even inside our area */
		if (row < B[k].out_j0 || row > B[k].out_j1) {	/* Either done with grid or haven't gotten to this range yet */
			B[k].outside = true;
			if (B[k].open) {	/* If an open file then we wipe */
				if (GMT_Destroy_Data (GMT->parent, &B[k].G)) return GMT_NOERROR;
				B[k].open = false;
				gmt_M_free (GMT, B[k].z);
				gmt_M_free (GMT, B[k].RbR);
				if (B[k].delete)	/* Delete the temporary resampled file */
					if (gmt_remove_file (GMT, B[k].file))	/* Oops, removal failed */
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to delete file %s\n", B[k].file); 
			}
			continue;
		}
		B[k].outside = false;		/* Here we know the row is inside this grid */
		if (row <= B[k].in_j0)		/* Top cosine taper weight */
			B[k].wt_y = 0.5 * (1.0 - cos ((row - B[k].out_j0 + half) * B[k].wyu));
		else if (row >= B[k].in_j1)	/* Bottom cosine taper weight */
			B[k].wt_y = 0.5 * (1.0 - cos ((B[k].out_j1 - row + half) * B[k].wyd));
		else				/* We are inside the inner region; y-weight = 1 */
			B[k].wt_y = 1.0;
		B[k].wt_y *= B[k].weight;

		if (B[k].memory) {	/* Grid already in memory, just copy the relevant row - no reading needed */
			G_row = row - B[k].out_j0;	/* The corresponding row number in the k'th grid */
			node = gmt_M_ijp (B[k].G->header, G_row, 0);	/* Start of our row at col = 0 */
			gmt_M_memcpy (B[k].z, &B[k].G->data[node], B[k].G->header->n_columns, gmt_grdfloat);	/* Copy that row */
		}
		else {	/* Deal with files that may need to be opened the first time we access it */
			if (!B[k].open) {
				struct GMT_GRID_HIDDEN *GH = NULL;
				if ((B[k].G = GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY|GMT_GRID_ROW_BY_ROW, NULL, B[k].file, NULL)) == NULL) {
					GMT_exit (GMT, GMT_GRID_READ_ERROR); return GMT_GRID_READ_ERROR;
				}
				GH = gmt_get_G_hidden (B[k].G);
				gmt_M_memcpy (B[k].RbR, GH->extra, 1, struct GMT_GRID_ROWBYROW);	/* Duplicate, since GMT_Destroy_Data will free the header->extra */
				if (B[k].skip) {	/* Position for native binary files */
					if (fseek (B[k].RbR->fp, B[k].skip, SEEK_CUR)) {    /* Position for native binary files */
						GMT_exit (GMT, GMT_GRDIO_SEEK_FAILED); return GMT_GRDIO_SEEK_FAILED;
					}
#ifdef DEBUG
					B[k].RbR->pos = ftell (B[k].RbR->fp);
#endif
				}
				else {	/* Set offsets for netCDF files */
					B[k].RbR->start[0] += B[k].offset;					/* Start position for netCDF files */
					gmt_M_memcpy (GH->extra, B[k].RbR, 1, struct GMT_GRID_ROWBYROW);	/* Synchronize these two again */
				}
				B[k].open = true;
			}
			GMT_Get_Row (GMT->parent, 0, B[k].G, B[k].z);	/* Get one row from this file */
		}
	}
	return GMT_NOERROR;
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDBLEND_CTRL *C = NULL;
	
	C = gmt_M_memory (GMT, NULL, 1, struct GRDBLEND_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	C->N.nodata = GMT->session.d_NaN;
	C->Z.scale = 1.0;
	
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDBLEND_CTRL *C) {	/* Deallocate control structure */
	unsigned int k;
	if (!C) return;
	for (k = 0; k < C->In.n; k++) gmt_M_str_free (C->In.file[k]);
	gmt_M_free (GMT, C->In.file);
	gmt_M_str_free (C->G.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<blendfile> | <grid1> <grid2> ...] -G<outgrid>\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t%s %s [-Cf|l|o|u[+n|p]]\n\t[-N<nodata>] [-Q] [%s] [-W[z]] [-Z<scale>] [%s] [%s] [%s] [%s]\n\n",
		GMT_I_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_n_OPT, GMT_r_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<blendfile> is an ASCII file (or stdin) with blending parameters for each input grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Each record has 1-3 items: filename [-R<inner_reg>] [<weight>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Relative weights are <weight> [1] inside the given -R [grid domain] and cosine taper to 0\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   at actual grid -R. Skip <inner_reg> if inner region should equal the actual region.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give a negative weight to invert the sense of the taper (i.e., |<weight>| outside given R.)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <weight> is not given we default to 1.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Grids not in netCDF or native binary format will be converted first.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Grids not co-registered with the output -R -I will be resampled first.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tAlternatively, if all grids have the same weight (1) and inner region == outer region,\n");
	GMT_Message (API, GMT_TIME_NONE, "\tthen you may instead list all the grid files on the command line (e.g., patches_*.nc).\n");
	GMT_Message (API, GMT_TIME_NONE, "\tNote: You must have at least 2 input grids for this mechanism to work.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G <outgrid> is the name of the final 2-D grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Only netCDF and native binary grid formats are directly supported;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   other grid formats will be converted via grdconvert when blending is complete.\n");
	GMT_Option (API, "I,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Clobber modes; no blending takes places as output node value is determined by the mode:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f: The first input grid determines the final value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     l: The lowest input grid value determines the final value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     o: The last input grid overrides any previous value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     u: The highest input grid value determines the final value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append +n (only consider clobbering if grid value is <= 0) or\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +p (only consider clobbering if grid value is >= 0.0) [consider any value].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set value for nodes without constraints [Default is NaN].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Raster output without a leading grid header [Default writes GMT grid file].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Output grid must be in one of the native binary formats.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Write out weight-sum only [make blend grid].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append z to write weight-sum w times z instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Multiply z-values by this scale before writing to file [1].\n");
	GMT_Option (API, "f,n");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   (-n is passed to grdsample if grids are not co-registered).\n");
	GMT_Option (API, "r,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDBLEND_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdblend and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

 	unsigned int n_errors = 0;
	size_t n_alloc = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Collect input files */
				Ctrl->In.active = true;
				if (n_alloc <= Ctrl->In.n)
					Ctrl->In.file = gmt_M_memory (GMT, Ctrl->In.file, n_alloc += GMT_SMALL_CHUNK, char *);
				if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					Ctrl->In.file[Ctrl->In.n++] = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Clobber mode */
				Ctrl->C.active = true;
				switch (opt->arg[0]) {
					case 'f': Ctrl->C.mode = BLEND_FIRST; break;
					case 'l': Ctrl->C.mode = BLEND_LOWER; break;
					case 'o': Ctrl->C.mode = BLEND_LAST;  break;
					case 'u': Ctrl->C.mode = BLEND_UPPER; break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Modifiers are f|l|o|u only\n");
						n_errors++;
						break;
				}
				if (strstr (opt->arg, "+p"))		/* Only use nodes >= 0 in the updates */
					Ctrl->C.sign = +1;
				else if (strstr (opt->arg, "+n"))	/* Only use nodes <= 0 in the updates */
					Ctrl->C.sign = -1;
				else {	/* May be nothing of old-style trailing - or + */
					switch (opt->arg[1]) {	/* Any restriction due to sign */
						case '-':  Ctrl->C.sign = -1; break;
						case '+':  Ctrl->C.sign = +1; break;
						case '\0': Ctrl->C.sign =  0; break;
						default:
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C%c option: Sign modifiers are +n|p\n", opt->arg[0]);
							n_errors++;
							break;
					}
				}
				break;
			case 'G':	/* Output filename */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':	/* Grid spacings */
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'N':	/* NaN-value */
				Ctrl->N.active = true;
				if (opt->arg[0])
					Ctrl->N.nodata = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.d_NaN : atof (opt->arg);
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: Must specify value or NaN\n");
					n_errors++;
				}
				break;
			case 'Q':	/* No header on output */
				Ctrl->Q.active = true;
				break;
			case 'W':	/* Write weights instead */
				Ctrl->W.active = true;
				if (opt->arg[0] == 'z') Ctrl->W.mode = 1;
				break;
			case 'Z':	/* z-multiplier */
				Ctrl->Z.active = true;
				Ctrl->Z.scale = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

#if 0
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error -R option: Must specify region\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0,
	                                   "Syntax error -I option: Must specify positive dx, dy\n");
#endif
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdblend (void *V_API, int mode, void *args) {
	unsigned int col, row, nx_360 = 0, k, kk, m, n_blend, nx_final, ny_final, out_case, zmode;
	int status, pcol, err, error;
	bool reformat, wrap_x, write_all_at_once = false, first_grid, delayed = true;
	
	uint64_t ij, n_fill, n_tot;
	double wt_x, w, wt;
	gmt_grdfloat *z = NULL, no_data_f;
	
	char type;
	char *outfile = NULL, outtemp[PATH_MAX];
	
	struct GRDBLEND_INFO *blend = NULL;
	struct GMT_GRID *Grid = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_GRID_HEADER *h_region = NULL;
	struct GRDBLEND_CTRL *Ctrl = NULL;
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
	
	/*---------------------------- This is the grdblend main code ----------------------------*/

	if (Ctrl->In.n <= 1) {	/* Got a blend file (or stdin) */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
			Return (API->error);
		}
	}

	if (GMT->common.R.active[RSET] && GMT->common.R.active[ISET]) {	/* Set output grid via -R -I [-r] */
		if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, NULL, NULL,
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL)
				Return (API->error);
		h_region = Grid->header;
		delayed = false;	/* Was able to create the grid from command line options */
	}

	status = init_blend_job (GMT, Ctrl->In.file, Ctrl->In.n, &h_region, &blend, &zmode);

	if (Ctrl->In.n <= 1 && GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	if (status < 0) Return (GMT_RUNTIME_ERROR);	/* Something went wrong in init_blend_job */
	n_blend = status;
	if (!Ctrl->W.active && n_blend == 1) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Only 1 grid found; no blending will take place\n");
	}
	
	if (delayed) {	/* Now we can create the output grid */
		if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, h_region->wesn, h_region->inc,
			h_region->registration, GMT_NOTSET, NULL)) == NULL)
				Return (API->error);
		gmt_free_header (API->GMT, &h_region);
	}
	
	if ((err = gmt_grd_get_format (GMT, Ctrl->G.file, Grid->header, false)) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: %s [%s]\n", GMT_strerror(err), Ctrl->G.file);
		Return (GMT_RUNTIME_ERROR);
	}
	HH = gmt_get_H_hidden (Grid->header);
	
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input grids\n");

	/* Formats other than netcdf (both v3 and new) and native binary must be reformatted at the end */
	reformat = found_unsupported_format (GMT, Grid->header, Ctrl->G.file);
	type = GMT->session.grdformat[Grid->header->type][0];
	if (Ctrl->Q.active && (reformat || (type == 'c' || type == 'n'))) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option: Not supported for grid format %s\n", GMT->session.grdformat[Grid->header->type]);
		Return (GMT_RUNTIME_ERROR);
	}
	
	n_fill = n_tot = 0;

	/* Process blend parameters and populate blend structure and open input files and seek to first row inside the output grid */

	no_data_f = (gmt_grdfloat)Ctrl->N.nodata;

	/* Initialize header structure for output blend grid */

	n_tot = gmt_M_get_nm (GMT, Grid->header->n_columns, Grid->header->n_rows);

	z = gmt_M_memory (GMT, NULL, Grid->header->n_columns, gmt_grdfloat);	/* Memory for one output row */

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) {
		gmt_M_free (GMT, z);
		Return (API->error);
	}

	if (gmt_M_file_is_memory (Ctrl->G.file)) {	/* GMT_grdblend is called by another module; must return as GMT_GRID */
		/* Allocate space for the entire output grid */
		if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_GRID, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Grid) == NULL) {
			gmt_M_free (GMT, z);
			Return (API->error);
		}
		write_all_at_once = true;
	}
	else {
		unsigned int w_mode;
		if (reformat) {	/* Must use a temporary netCDF file then reformat it at the end */
			if (API->tmp_dir)	/* Use the established temp directory */
				sprintf (outtemp, "%s/grdblend_temp_%d.nc", API->tmp_dir, (int)getpid());	/* Get temporary file name */
			else	/* Must dump it in current directory */
				sprintf (outtemp, "grdblend_temp_%d.nc", (int)getpid());	/* Get temporary file name */
			outfile = outtemp;
		}
		else
			outfile = Ctrl->G.file;
		/* Write the grid header unless -Q */
		w_mode = GMT_CONTAINER_ONLY | GMT_GRID_ROW_BY_ROW;
		if (Ctrl->Q.active) w_mode |= GMT_GRID_NO_HEADER;
		if ((error = GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, w_mode, NULL, outfile, Grid))) {
			gmt_M_free (GMT, z);
			Return (error);
		}
	}
	
	if (Ctrl->Z.active) GMT_Report (API, GMT_MSG_VERBOSE, "Output data will be scaled by %g\n", Ctrl->Z.scale);

	out_case = (Ctrl->W.active) ? 1 + Ctrl->W.mode : 0;
	
	Grid->header->z_min = DBL_MAX;	Grid->header->z_max = -DBL_MAX;	/* These will be updated in the loop below */
	wrap_x = (gmt_M_is_geographic (GMT, GMT_OUT));	/* Periodic geographic grid */
	if (wrap_x) nx_360 = urint (360.0 * HH->r_inc[GMT_X]);

	for (row = 0; row < Grid->header->n_rows; row++) {	/* For every output row */

		gmt_M_memset (z, Grid->header->n_columns, gmt_grdfloat);	/* Start from scratch */

		sync_input_rows (GMT, row, blend, n_blend, Grid->header->xy_off);	/* Wind each input file to current record and read each of the overlapping rows */

		for (col = 0; col < Grid->header->n_columns; col++) {	/* For each output node on the current row */

			w = 0.0;	/* Reset weight */
			first_grid = true;	/* Since some grids do not contain this (row,col) we want to know when we are processing the first grid inside */ 
			for (k = m = 0; k < n_blend; k++) {	/* Loop over every input grid; m will be the number of contributing grids to this node  */
				if (blend[k].ignore) continue;					/* This grid is entirely outside the s/n range */
				if (blend[k].outside) continue;					/* This grid is currently outside the s/n range */
				if (wrap_x) {	/* Special testing for periodic x coordinates */
					pcol = col + nx_360;
					while (pcol > blend[k].out_i1) pcol -= nx_360;
					if (pcol < blend[k].out_i0) continue;	/* This grid is currently outside the w/e range */
				}
				else {	/* Not periodic */
					pcol = col;
					if (pcol < blend[k].out_i0 || pcol > blend[k].out_i1) continue;	/* This grid is currently outside the xmin/xmax range */
				}
				kk = pcol - blend[k].out_i0;					/* kk is the local column variable for this grid */
				if (gmt_M_is_fnan (blend[k].z[kk])) continue;			/* NaNs do not contribute */
				if (Ctrl->C.active) {	/* Clobber; update z[col] according to selected mode */
					switch (Ctrl->C.mode) {
						case BLEND_FIRST: if (m) continue; break;	/* Already set */
						case BLEND_UPPER: if (m && blend[k].z[kk] <= z[col]) continue; break;	/* Already has a higher value; else set below */
						case BLEND_LOWER: if (m && blend[k].z[kk] >= z[col]) continue; break;	/* Already has a lower value; else set below */
						/* Last case BLEND_LAST is always true in that we always update z[col] */
					}
					switch (Ctrl->C.sign) {	/* Check if sign of input grid should be considered in decision */
						case -1: if (first_grid) {z[col] = blend[k].z[kk]; first_grid = false; continue; break;}	/* Must initialize with first grid in case nothing passes */
							 else if (blend[k].z[kk] > 0.0) continue;	/* Only pick grids value if negative or zero */
							 break;
						case +1: if (first_grid) { z[col] = blend[k].z[kk]; first_grid = false; continue; break;}	/* Must initialize with first grid in case nothing passes */
							 else if (blend[k].z[kk] < 0.0) continue;	/* Only pick grids value if positive or zero */
							 break;
						default: break;						/* Always use the grid value */

					}
					z[col] = blend[k].z[kk];					/* Just pick this grid's value */
					w = 1.0;							/* Set weights to 1 */
					m = 1;								/* Pretend only one grid came here */
				}
				else {	/* Do the weighted blending */ 
					if (pcol <= blend[k].in_i0)					/* Left cosine-taper weight */
						wt_x = 0.5 * (1.0 - cos ((pcol - blend[k].out_i0 + Grid->header->xy_off) * blend[k].wxl));
					else if (pcol >= blend[k].in_i1)					/* Right cosine-taper weight */
						wt_x = 0.5 * (1.0 - cos ((blend[k].out_i1 - pcol + Grid->header->xy_off) * blend[k].wxr));
					else								/* Inside inner region, weight = 1 */
						wt_x = 1.0;
					wt = wt_x * blend[k].wt_y;					/* Actual weight is 2-D cosine taper */
					if (blend[k].invert) wt = blend[k].weight - wt;			/* Invert the sense of the tapering */
					z[col] += (gmt_grdfloat)(wt * blend[k].z[kk]);				/* Add up weighted z*w sum */
					w += wt;							/* Add up the weight sum */
					m++;								/* Add up the number of contributing grids */
				}
			}

			if (Ctrl->C.sign && m == 0) m = 1, w = 1.0;	/* Since we started off with the first grid and never set m,w at that time */
			
			if (m) {	/* OK, at least one grid contributed to an output value */
				switch (out_case) {
					case 0: /* Blended average */
						z[col] = (gmt_grdfloat)((w == 0.0) ? 0.0 : z[col] / w);	/* Get weighted average z */
						if (Ctrl->Z.active) z[col] *= (gmt_grdfloat)Ctrl->Z.scale;	/* Apply the global scale here */
						break;
					case 1:	/* Just weights */
						z[col] = (gmt_grdfloat)w;				/* Only interested in the weights */
						break;
					case 2:	/* w*z = sum of z */
						z[col] = (gmt_grdfloat)(z[col] * w);
						break;
				}
				n_fill++;						/* One more cell filled */
				if (z[col] < Grid->header->z_min) Grid->header->z_min = z[col];	/* Update the extrema for output grid */
				if (z[col] > Grid->header->z_max) Grid->header->z_max = z[col];
			}
			else			/* No grids covered this node, defaults to the no_data value */
				z[col] = no_data_f;
		}
		if (write_all_at_once) {	/* Must copy entire row to grid */
			ij = gmt_M_ijp (Grid->header, row, 0);
			gmt_M_memcpy (&(Grid->data[ij]), z, Grid->header->n_columns, gmt_grdfloat);
		}
		else
			GMT_Put_Row (API, row, Grid, z);

		if (row%10 == 0)  GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processed row %7ld of %d\r", row, Grid->header->n_rows);

	}
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processed row %7ld\n", row);
	nx_final = Grid->header->n_columns;	ny_final = Grid->header->n_rows;
	if (zmode) {	/* Pass the information up via the header */
		char line[GMT_GRID_REMARK_LEN160] = {""};
		unsigned int srtm_res = zmode / 10;	/* Extract resolutin as 1 or 3 */
		zmode -= srtm_res * 10;			/* Extract zmode as 1 or 2 */
		if (zmode == 1) sprintf (line, "@earth_relief_0%ds blend", srtm_res);
		else if (zmode == 2) sprintf (line, "@srtm_relief_0%ds blend", srtm_res);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, line, Grid)) Return (API->error);
	}

	if (write_all_at_once) {	/* Must write entire grid */
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
			gmt_M_free (GMT, z);
			Return (API->error);
		}
	}
	else {	/* Finish the line-by-line writing */
		mode = GMT_CONTAINER_ONLY | GMT_GRID_ROW_BY_ROW;
		if (Ctrl->Q.active) mode |= GMT_GRID_NO_HEADER;
		if (!Ctrl->Q.active && (error = GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, mode, NULL, outfile, Grid))) {
			Return (error);
		}
		if ((error = GMT_Destroy_Data (API, &Grid)) != GMT_NOERROR) Return (error);
	}
	gmt_M_free (GMT, z);

	/* Free up the list with grid information, closing files as necessary */
	
	for (k = 0; k < n_blend; k++) {
		if (blend[k].open || blend[k].memory) {
			gmt_M_free (GMT, blend[k].z);
			gmt_M_free (GMT, blend[k].RbR);
		}
		if (blend[k].open) {
			if (blend[k].delete && gmt_remove_file (GMT, blend[k].file))	/* Delete the temporary resampled file */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to delete file %s\n", blend[k].file); 
			if ((error = GMT_Destroy_Data (API, &blend[k].G)) != GMT_NOERROR) Return (error);
		}
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) {
		char empty[GMT_LEN64] = {""};
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Blended grid size of %s is %d x %d\n", Ctrl->G.file, nx_final, ny_final);
		if (n_fill == n_tot)
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "All nodes assigned values\n");
		else {
			if (gmt_M_is_fnan (no_data_f))
				strcpy (empty, "NaN");
			else
				sprintf (empty, "%g", no_data_f);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " nodes assigned values, %" PRIu64 " set to %s\n", n_fill, n_tot - n_fill, empty);
		}
	}

	gmt_M_free (GMT, blend);

	if (reformat) {	/* Must reformat the output grid to the non-supported format */
		int status;
		char cmd[GMT_LEN256] = {""}, *V_level = "qncvld";
		sprintf (cmd, "%s %s -V%c --GMT_HISTORY=false", outfile, Ctrl->G.file, V_level[GMT->current.setting.verbose]);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reformat %s via grdconvert %s\n", outfile, cmd);
		if ((status = GMT_Call_Module (GMT->parent, "grdconvert", GMT_MODULE_CMD, cmd))) {	/* Resample the file */
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to resample file %s.\n", outfile);
		}
		if (gmt_remove_file (GMT, outfile))	/* Try half-heartedly to remove the temporary file */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to delete file %s\n", outfile); 
	}

	Return (GMT_NOERROR);
}
