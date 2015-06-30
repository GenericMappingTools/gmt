/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#include "gmt_dev.h"
#include "gmt_internals.h"
#include "gshhg_version.h"
/*
 * These functions simplifies the access to the GMT shoreline, border, and river
 * databases.
 *
 * The PUBLIC functions are:
 *
 * GMT_init_shore :		Opens selected shoreline database and initializes structures
 * GMT_get_shore_bin :		Returns all selected shore data for this bin
 * GMT_init_br :		Opens selected border/river database and initializes structures
 * GMT_get_br_bin :		Returns all selected border/river data for this bin
 * GMT_assemble_shore :		Creates polygons or lines from shoreline segments
 * GMT_prep_shore_polygons :	Wraps polygons if necessary and prepares them for use
 * GMT_assemble_br :		Creates lines from border or river segments
 * GMT_free_shore :		Frees up memory used by shorelines for this bin
 * GMT_free_br :		Frees up memory used by shorelines for this bin
 * GMT_shore_cleanup :		Frees up main shoreline structure memory
 * GMT_br_cleanup :		Frees up main river/border structure memory
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5.x
 *
 */

#define GSHHG_SITE "ftp://ftp.soest.hawaii.edu/gshhg/"

#define RIVERLAKE	5				/* Fill array id for riverlakes */
#define get_level(arg) (((arg) >> 6) & 7)		/* Extract level from bit mask */

/* ---------- LOWER LEVEL FUNCTIONS CALLED BY THE ABOVE ------------ */

void gmt_shore_to_degree (struct GMT_SHORE *c, short int dx, short int dy, double *lon, double *lat)
{	/* Converts relative (0-65535) coordinates to actual lon, lat values */
	*lon = c->lon_sw + ((unsigned short)dx) * c->scale;
	*lat = c->lat_sw + ((unsigned short)dy) * c->scale;
}

void gmt_br_to_degree (struct GMT_BR *c, short int dx, short int dy, double *lon, double *lat)
{	/* Converts relative (0-65535) coordinates to actual lon, lat values */
	*lon = c->lon_sw + ((unsigned short)dx) * c->scale;
	*lat = c->lat_sw + ((unsigned short)dy) * c->scale;
}

int gmt_copy_to_shore_path (double *lon, double *lat, struct GMT_SHORE *s, int id)
{	/* Convert a shore segment to degrees and add to array */
	int i;
	for (i = 0; i < (int)s->seg[id].n; i++)
		gmt_shore_to_degree (s, s->seg[id].dx[i], s->seg[id].dy[i], &lon[i], &lat[i]);
	return (s->seg[id].n);
}

int gmt_copy_to_br_path (double *lon, double *lat, struct GMT_BR *s, int id)
{	/* Convert a line segment to degrees and add to array */
	int i;
	for (i = 0; i < (int)s->seg[id].n; i++)
		gmt_br_to_degree (s, s->seg[id].dx[i], s->seg[id].dy[i], &lon[i], &lat[i]);
	return (s->seg[id].n);
}

int gmt_shore_get_position (int side, short int x, short int y)
{	/* Returns the position along the given side, measured from start of side */
	return ((side%2) ? ((side == 1) ? (unsigned short)y : GSHHS_MAX_DELTA - (unsigned short)y) : ((side == 0) ? (unsigned short)x : GSHHS_MAX_DELTA - (unsigned short)x));
}

int gmt_shore_get_next_entry (struct GMT_SHORE *c, int dir, int side, int id)
{	/* Finds the next entry point on the given side that is further away
	 * in the <dir> direction than previous point.  It then removes the info
	 * regarding the new entry from the GSHHS_SIDE structure so it wont be
	 * used twice. Because we have added the 4 corners with pos = 65535 we
	 * know that if there are no segments on a side the procedure will find
	 * the corner as the last item, always. This is for CCW; when dir = -1
	 * then we have added the corners with pos = 0 and search in the other
	 * direction so we will find the corner point last.  */

	int k, pos, n;

	if (id < 0)	/* A corner, return start or end of this side */
		pos = (dir == 1) ? 0 : GSHHS_MAX_DELTA;
	else {	/* A real segment, get number of points and its starting position */
		n = c->seg[id].n - 1;
		pos = gmt_shore_get_position (side, c->seg[id].dx[n], c->seg[id].dy[n]);
	}

	if (dir == 1) {	/* CCW: find the next segment (or corner if no segments) whose entry position exceeds this pos */
		for (k = 0; k < (int)c->nside[side] && (int)c->side[side][k].pos < pos; k++);
		id = c->side[side][k].id;	/* The ID of the next segment (or corner) */
		for (k++; k < c->nside[side]; k++) c->side[side][k-1] = c->side[side][k];	/* Remove the item we found */
		c->nside[side]--;	/* Remove the item we found */
	}
	else {	/* CW: find the next segment (or corner if no segments) whose entry position is less than this pos */
		for (k = 0; k < (int)c->nside[side] && (int)c->side[side][k].pos > pos; k++);
		id = c->side[side][k].id;	/* The ID of the next segment (or corner) */
		for (k++; k < c->nside[side]; k++) c->side[side][k-1] = c->side[side][k];	/* Remove the item we found */
		c->nside[side]--;	/* Remove the item we found */
	}
	if (id >= 0) c->n_entries--;	/* Reduce number of remaining segments (not counting corners) */
	return (id);
}

int gmt_shore_get_first_entry (struct GMT_SHORE *c, int dir, int *side)
{	/* Loop over all sides and find the first available entry, starting at *side and moving around counter-clockwise.
	 * We only return IDs of segments and do not consider any corner points here - that is handled separately */
	int try = 0;	/* We have max 4 tries, i.e., all 4 sides */
	while (try < 4 && (c->nside[*side] == 0 || (c->nside[*side] == 1 && c->side[*side][0].id < 0))) {	/* No entries or only a corner left on this side */
		try++;	/* Try again */
		*side = (*side + dir + 4) % 4;	/* This is the next side going CCW */
	}
	if (try == 4) return (-5);	/* No luck finding any side with a segment */
	return (c->side[*side][0].id);	/* Return the ID of the segment; its side is returned via *side */
}

int gmt_shore_asc_sort (const void *a, const void *b)
{	/* Sort segment into ascending order based on entry positions for going CCW */
	if (((struct GSHHS_SIDE *)a)->pos < ((struct GSHHS_SIDE *)b)->pos) return (-1);
	if (((struct GSHHS_SIDE *)a)->pos > ((struct GSHHS_SIDE *)b)->pos) return (1);
	return (0);
}

int gmt_shore_desc_sort (const void *a, const void *b)
{	/* Sort segment into descending order based on entry positions for going CW */
	if (((struct GSHHS_SIDE *)a)->pos < ((struct GSHHS_SIDE *)b)->pos) return (1);
	if (((struct GSHHS_SIDE *)a)->pos > ((struct GSHHS_SIDE *)b)->pos) return (-1);
	return (0);
}

void gmt_shore_done_sides (struct GMT_CTRL *GMT, struct GMT_SHORE *c)
{	/* Free the now empty list of side structures */
	unsigned int i;
	for (i = 0; i < 4; i++) GMT_free (GMT, c->side[i]);
}

void GMT_free_shore_polygons (struct GMT_CTRL *GMT, struct GMT_GSHHS_POL *p, unsigned int n)
{	/* Free the given list of polygon coordinates */
	unsigned int k;
	for (k = 0; k < n; k++) {
		GMT_free (GMT, p[k].lon);
		GMT_free (GMT, p[k].lat);
	}
}

void gmt_shore_path_shift (double *lon, unsigned int n, double edge)
{	/* Shift all longitudes >= edige by 360 westwards */
	unsigned int i;

	for (i = 0; i < n; i++) if (lon[i] >= edge) lon[i] -= 360.0;
}

void gmt_shore_path_shift2 (double *lon, unsigned int n, double west, double east, int leftmost)
{	/* Adjust longitudes so there are no jumps with respect to current bin boundaries */
	unsigned int i;

	if (leftmost) {	/* Must check this bin differently  */
		for (i = 0; i < n; i++) if (lon[i] >= east && (lon[i]-360.0) >= west) lon[i] -= 360.0;
	}
	else {
		for (i = 0; i < n; i++) if (lon[i] > east && (lon[i]-360.0) >= west) lon[i] -= 360.0;
	}
}

void gmt_shore_prepare_sides (struct GMT_CTRL *GMT, struct GMT_SHORE *c, int dir)
{	/* Initializes the GSHHS_SIDE stuctures for each side, then adds corners and all entering segments */
	int s, i, n[4];

	/* Set corner coordinates */
	c->lon_corner[0] = c->lon_sw + ((dir == 1) ? c->bsize : 0.0);
	c->lon_corner[1] = c->lon_sw + c->bsize;
	c->lon_corner[2] = c->lon_sw + ((dir == 1) ? 0.0 : c->bsize);
	c->lon_corner[3] = c->lon_sw;
	c->lat_corner[0] = c->lat_sw;
	c->lat_corner[1] = c->lat_sw + ((dir == 1) ? c->bsize : 0.0);
	c->lat_corner[2] = c->lat_sw + c->bsize;
	c->lat_corner[3] = c->lat_sw + ((dir == 1) ? 0.0 : c->bsize);

	for (i = 0; i < 4; i++) c->nside[i] = n[i] = 1;	/* Each side has at least one "segment", the corner point */
	/* for (s = 0; s < c->ns; s++) if (c->seg[s].level < 3 && c->seg[s].entry < 4) c->nside[c->seg[s].entry]++; */
	for (s = 0; s < c->ns; s++) if (c->seg[s].entry < 4) c->nside[c->seg[s].entry]++;	/* Add up additional segments entering each side */

	for (i = c->n_entries = 0; i < 4; i++) {	/* Allocate memory and add corners; they are given max pos so they are the last in the sorted list per side */
		c->side[i] = GMT_memory (GMT, NULL, c->nside[i], struct GSHHS_SIDE);
		c->side[i][0].pos = (dir == 1) ? GSHHS_MAX_DELTA : 0;	/* position at end of side depends if going CCW (65535) or CW (0) */
		c->side[i][0].id = (short int)(i - 4);	/* Corners have negative IDs; add 4 to get real ID */
		c->n_entries += c->nside[i] - 1;	/* Total number of entries so far */
	}

	for (s = 0; s < c->ns; s++) {	/* Now add entry points for each segment */
		/* if (c->seg[s].level > 2 || (i = c->seg[s].entry) == 4) continue; */
		if ((i = c->seg[s].entry) == 4) continue;
		c->side[i][n[i]].pos = (unsigned short)gmt_shore_get_position (i, c->seg[s].dx[0], c->seg[s].dy[0]);
		c->side[i][n[i]].id = (short)s;
		n[i]++;
	}

	/* We then sort the array of GSHHS_SIDE stucts on their distance from the start of the side */
	for (i = 0; i < 4; i++)	{	/* sort on position */
		if (dir == 1)
			qsort (c->side[i], (size_t)c->nside[i], sizeof (struct GSHHS_SIDE), gmt_shore_asc_sort);
		else
			qsort (c->side[i], (size_t)c->nside[i], sizeof (struct GSHHS_SIDE), gmt_shore_desc_sort);
	}
}

char *gmt_shore_getpathname (struct GMT_CTRL *GMT, char *stem, char *path) {
	/* Prepends the appropriate directory to the file name
	 * and returns path if file is readable, NULL otherwise */

	FILE *fp = NULL;
	char dir[GMT_BUFSIZ];
	static struct GSHHG_VERSION version = GSHHG_MIN_REQUIRED_VERSION;
	static int warn_once = true;

	/* This is the order of checking:
	 * 1. Check in GMT->session.GSHHGDIR
	 * 2. Is there a file coastline.conf in current directory,
	 *    GMT->session.USERDIR or GMT->session.SHAREDIR[/coast]?
	 *    If so, use its information
	 * 3. Look in current directory, GMT->session.USERDIR or
	 *    GMT->session.SHAREDIR[/coast] for file "name".
	 */

	/* 1. Check in GMT->session.GSHHGDIR */

	if (GMT->session.GSHHGDIR) {
		sprintf (path, "%s/%s%s", GMT->session.GSHHGDIR, stem, ".nc");
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "1. GSHHG: GSHHGDIR set, trying %s\n", path);
		if ( access (path, F_OK) == 0) {	/* File exists here */
			if ( access (path, R_OK) == 0 && gshhg_require_min_version (path, version) ) {
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "1. GSHHG: OK, could access %s\n", path);
				return (path);
			}
			else
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Found %s but cannot read it due to wrong permissions\n", path);
		}
		else {
			/* remove reference to invalid GMT->session.GSHHGDIR but don't free
			 * the pointer. this is no leak because the reference still exists
			 * in the previous copy of the current GMT_CTRL struct. */
			GMT->session.GSHHGDIR = NULL;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "1. GSHHG: Failure, could not access %s\n", path);
		}
	}

	/* 2. First check for coastline.conf */

	if (GMT_getsharepath (GMT, "conf", "coastline", ".conf", path, F_OK) || GMT_getsharepath (GMT, "coast", "coastline", ".conf", path, F_OK)) {

		/* We get here if coastline.conf exists - search among its directories for the named file */

		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "2. GSHHG: coastline.conf found at %s\n", path);
		if ( access (path, R_OK) == 0) {	/* File can be read */
			fp = fopen (path, "r");
			while (fgets (dir, GMT_BUFSIZ, fp)) {	/* Loop over all input lines until found or done */
				if (dir[0] == '#' || dir[0] == '\n') continue;	/* Comment or blank */
				GMT_chop (dir);		/* Chop off LF or CR/LF */
				sprintf (path, "%s/%s%s", dir, stem, ".nc");
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "2. GSHHG: Trying %s\n", path);
				if ( access (path, R_OK) == 0) {	/* File can be read */
					if ( gshhg_require_min_version (path, version) ) {
						fclose (fp);
						/* update invalid GMT->session.GSHHGDIR */
						if (GMT->session.GSHHGDIR) free ((void *)GMT->session.GSHHGDIR);
						GMT->session.GSHHGDIR = strdup (dir);
						GMT_Report (GMT->parent, GMT_MSG_DEBUG, "2. GSHHG: OK, could access %s\n", path);
						return (path);
					}
					else
						GMT_Report (GMT->parent, GMT_MSG_DEBUG, "2. GSHHG: Failure, could not access %s\n", path);
				}
				else
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Found %s but cannot read it due to wrong permissions\n", path);
			}
			fclose (fp);
		}
		else
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Found %s but cannot read it due to wrong permissions\n", path);
	}

	/* 3. Then check for the named file itself */

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "3. GSHHG: Trying via sharepath\n");
	if (GMT_getsharepath (GMT, "coast", stem, ".nc", path, F_OK)) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "3. GSHHG: Trying %s\n", path);
		if ( access (path, R_OK) == 0) {	/* File can be read */
			if ( gshhg_require_min_version (path, version) ) {
				/* update invalid GMT->session.GSHHGDIR */
				sprintf (dir, "%s/%s", GMT->session.SHAREDIR, "coast");
				if (GMT->session.GSHHGDIR) free ((void *)GMT->session.GSHHGDIR);
				GMT->session.GSHHGDIR = strdup (dir);
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "3. GSHHG: OK, could access %s\n", path);
				return (path);
			}
			else
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "3. GSHHG: Failure, could not access %s\n", path);
		}
		else
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Found %s but cannot read it due to wrong permissions\n", path);
	}

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "4. GSHHG: Failure, could not access any GSHHG files\n");
	if (warn_once) {
		warn_once = false;
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GSHHG version %d.%d.%d or newer is "
								"needed to use coastlines with GMT.\n\tGet and install GSHHG from "
								GSHHG_SITE ".\n", version.major, version.minor, version.patch);
	}

	return (NULL); /* never reached */
}

void gmt_shore_check (struct GMT_CTRL *GMT, bool ok[5])
/* Sets ok to true for those resolutions available in share for
 * resolution (f, h, i, l, c) */
{
	int i, j, n_found;
	char stem[GMT_LEN64] = {""}, path[GMT_BUFSIZ] = {""}, *res = "clihf", *kind[3] = {"GSHHS", "river", "border"};

	for (i = 0; i < 5; i++) {
		/* For each resolution... */
		ok[i] = false;
		for (j = n_found = 0; j < 3; j++) {
			/* For each data type... */
			sprintf (stem, "binned_%s_%c", kind[j], res[i]);
			if (!gmt_shore_getpathname (GMT, stem, path))
				/* Failed to find file */
				continue;
			n_found++; /* Increment how many found so far for this resolution */
		}
		ok[i] = (n_found == 3);	/* Need all three sets to say this resolution is covered */
	}
}

int gmt_res_to_int (char res)
{	/* Turns a resolution letter into a 0-4 integer */
	int i, j;
	char *type = "clihf";

	for (i = -1, j = 0; i == -1 && j < 5; j++) if (res == type[j]) i = j;
	return (i);
}

/* Main Public GMT shore functions */

int GMT_set_levels (struct GMT_CTRL *GMT, char *info, struct GMT_SHORE_SELECT *I)
{	/* Decode GMT's -A option for coastline levels */
	int n;
	char *p = NULL;
	if (strstr (info, "+as"))  I->antarctica_mode = GSHHS_ANTARCTICA_SKIP;	/* Skip Antarctica data south of 60S */
	if (strstr (info, "+l"))  I->flag = GSHHS_NO_RIVERLAKES;
	if (strstr (info, "+r"))  I->flag = GSHHS_NO_LAKES;
	if ((p = strstr (info, "+p"))) {	/* Requested percentage limit on small features */
		I->fraction = irint (1e6 * 0.01 * atoi (&p[2]));	/* Convert to integer microfraction */
	}
	if (info[0] == '+') return (GMT_OK);	/* No area, etc, just modifiers that we just processed */
	n = sscanf (info, "%lf/%d/%d", &I->area, &I->low, &I->high);
	if (n == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -A option: No area given\n");
		GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
	}
	if (n == 1) I->low = 0, I->high = GSHHS_MAX_LEVEL;
	return (GMT_OK);
}

int GMT_set_resolution (struct GMT_CTRL *GMT, char *res, char opt)
{
	/* Decodes the -D<res> option and returns the base integer value */

	int base;

	switch (*res) {
		case 'f':	/* Full */
			base = 0;
			break;
		case 'h':	/* High */
			base = 1;
			break;
		case 'i':	/* Intermediate */
			base = 2;
			break;
		case 'l':	/* Low */
			base = 3;
			break;
		case 'c':	/* Crude */
			base = 4;
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -%c option: Unknown modifier %c [Defaults to -%cl]\n", opt, *res, opt);
			base = 3;
			*res = 'l';
			break;
	}

	return (base);
}

char GMT_shore_adjust_res (struct GMT_CTRL *GMT, char res) {	/* Returns the highest available resolution <= to specified resolution */
	int k, orig;
	bool ok[5];
	char *type = "clihf";
	(void)gmt_shore_check (GMT, ok);		/* See which resolutions we have */
	k = orig = gmt_res_to_int (res);	/* Get integer value of requested resolution */
	while (k >= 0 && !ok[k]) --k;		/* Drop down one level to see if we have a lower resolution available */
	if (k >= 0 && k != orig) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Resolution %c not available, substituting resolution %c\n", res, type[k]);
	return ((k == -1) ? res : type[k]);	/* Return the chosen resolution */
}

int GMT_init_shore (struct GMT_CTRL *GMT, char res, struct GMT_SHORE *c, double wesn[], struct GMT_SHORE_SELECT *info) {	/* res: Resolution (f, h, i, l, c */
	/* Opens the netcdf file and reads in all top-level attributes, IDs, and variables for all bins overlapping with wesn */
	int i, nb, idiv, iw, ie, is, in, this_south, this_west, this_north, err;
	bool int_areas = false;
	short *stmp = NULL;
	int *itmp = NULL;
	size_t start[1], count[1];
	char stem[GMT_LEN64] = {""}, path[GMT_BUFSIZ] = {""};

	sprintf (stem, "binned_GSHHS_%c", res);

	if (!gmt_shore_getpathname (GMT, stem, path))
		return (GMT_GRDIO_FILE_NOT_FOUND); /* Failed to find file */

		/* zap structure (nc_get_att_text does not null-terminate strings!) */
		GMT_memset (c, 1, struct GMT_SHORE);

	/* Open shoreline file */
	GMT_err_trap (nc_open (path, NC_NOWRITE, &c->cdfid));

	/* Get global attributes */
	GMT_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "version", c->version));
	GMT_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "title", c->title));
	GMT_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "source", c->source));

	/* Get all id tags */
	GMT_err_trap (nc_inq_varid (c->cdfid, "Bin_size_in_minutes", &c->bin_size_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_360_longitude_range", &c->bin_nx_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_180_degree_latitude_range", &c->bin_ny_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_file", &c->n_bin_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_segments_in_file", &c->n_seg_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_points_in_file", &c->n_pt_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Id_of_first_segment_in_a_bin", &c->bin_firstseg_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Embedded_node_levels_in_a_bin", &c->bin_info_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_segments_in_a_bin", &c->bin_nseg_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Embedded_npts_levels_exit_entry_for_a_segment", &c->seg_info_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Id_of_first_point_in_a_segment", &c->seg_start_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Relative_longitude_from_SW_corner_of_bin", &c->pt_dx_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Relative_latitude_from_SW_corner_of_bin", &c->pt_dy_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Micro_fraction_of_full_resolution_area", &c->GSHHS_areafrac_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_polygons_in_file", &c->n_poly_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_nodes_in_file", &c->n_node_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Id_of_parent_polygons", &c->GSHHS_parent_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Id_of_node_polygons", &c->GSHHS_node_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Id_of_GSHHS_ID", &c->seg_GSHHS_ID_id));

	if (nc_inq_varid (c->cdfid, "Ten_times_the_km_squared_area_of_polygons", &c->GSHHS_area_id) == NC_NOERR) {	/* Old file with 1/10 km^2 areas in int format*/
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "GSHHS: Areas not accurate for small lakes and islands.  Consider updating GSHHG.\n");
		int_areas = true;
	}
	else if (nc_inq_varid (c->cdfid, "The_km_squared_area_of_polygons", &c->GSHHS_area_id) != NC_NOERR) {	/* New file with km^2 areas as doubles */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GSHHS: Unable to determine how polygon areas were stored.\n");
	}

	/* Get attributes */
	GMT_err_trap (nc_get_att_text (c->cdfid, c->pt_dx_id, "units", c->units));

	/* Get global variables */

	start[0] = 0;

	GMT_err_trap (nc_get_var1_int (c->cdfid, c->bin_size_id, start, &c->bin_size));
	GMT_err_trap (nc_get_var1_int (c->cdfid, c->bin_nx_id, start, &c->bin_nx));
	GMT_err_trap (nc_get_var1_int (c->cdfid, c->bin_ny_id, start, &c->bin_ny));
	GMT_err_trap (nc_get_var1_int (c->cdfid, c->n_bin_id, start, &c->n_bin));
	GMT_err_trap (nc_get_var1_int (c->cdfid, c->n_seg_id, start, &c->n_seg));
	GMT_err_trap (nc_get_var1_int (c->cdfid, c->n_pt_id, start, &c->n_pt));

	c->fraction = info->fraction;
	c->skip_feature = info->flag;
	c->min_area = info->area;	/* Limit the features */
	c->min_level = info->low;
	c->max_level = (info->low == info->high && info->high == 0) ? GSHHS_MAX_LEVEL : info->high;	/* Default to all if not set */
	c->flag = info->flag;
	c->ant_mode = info->antarctica_mode;
	c->res = res;

	c->scale = (c->bin_size / 60.0) / 65535.0;
	c->bsize = c->bin_size / 60.0;

	c->bins = GMT_memory (GMT, NULL, c->n_bin, int);

	/* Round off area to nearest multiple of block-dimension */

	iw = irint (floor (wesn[XLO] / c->bsize) * c->bsize);
	ie = irint (ceil (wesn[XHI] / c->bsize) * c->bsize);
	is = 90 - irint (ceil ((90.0 - wesn[YLO]) / c->bsize) * c->bsize);
	in = 90 - irint (floor ((90.0 - wesn[YHI]) / c->bsize) * c->bsize);
	idiv = irint (360.0 / c->bsize);	/* Number of blocks per latitude band */

	for (i = nb = 0; i < c->n_bin; i++) {	/* Find which bins are needed */
		this_south = 90 - irint (c->bsize * ((i / idiv) + 1));
		if (this_south < is || this_south >= in) continue;
		this_north = this_south + irint (c->bsize);
		if (info->antarctica_mode == GSHHS_ANTARCTICA_SKIP && this_north <= GSHHS_ANTARCTICA_LIMIT) continue;	/* Does not want Antarctica in output */
		this_west = irint (c->bsize * (i % idiv)) - 360;
		while (this_west < iw) this_west += 360;
		if (this_west >= ie) continue;
		c->bins[nb] = i;
		nb++;
	}
	c->bins = GMT_memory (GMT, c->bins, nb, int);
	c->nb = nb;

	/* Get polygon variables if they are needed */

	GMT_err_trap (nc_get_var1_int (c->cdfid, c->n_poly_id, start, &c->n_poly));
	count[0] = c->n_poly;
	c->GSHHS_parent = GMT_memory (GMT, NULL, c->n_poly, int);
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->GSHHS_parent_id, start, count, c->GSHHS_parent));
	c->GSHHS_area = GMT_memory (GMT, NULL, c->n_poly, double);
	GMT_err_trap (nc_get_vara_double (c->cdfid, c->GSHHS_area_id, start, count, c->GSHHS_area));
	if (int_areas) for (i = 0; i < c->n_poly; i++) c->GSHHS_area[i] *= 0.1;	/* Since they were stored as 10 * km^2 using integers */
	c->GSHHS_area_fraction = GMT_memory (GMT, NULL, c->n_poly, int);
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->GSHHS_areafrac_id, start, count, c->GSHHS_area_fraction));
	if (c->min_area > 0.0) {	/* Want to exclude small polygons so we need info about the node polygons */
	        GMT_err_trap (nc_get_var1_int (c->cdfid, c->n_node_id, start, &c->n_nodes));
		c->GSHHS_node = GMT_memory (GMT, NULL, c->n_nodes, int);
		count[0] = c->n_nodes;
		GMT_err_trap (nc_get_vara_int (c->cdfid, c->GSHHS_node_id, start, count, c->GSHHS_node));
	}

	/* Get bin variables, then extract only those corresponding to the bins to use */

	/* Allocate space for arrays of bin information */

	c->bin_info      = GMT_memory (GMT, NULL, nb, short);
	c->bin_nseg      = GMT_memory (GMT, NULL, nb, short);
	c->bin_firstseg  = GMT_memory (GMT, NULL, nb, int);

	count[0] = c->n_bin;
	stmp = GMT_memory (GMT, NULL, c->n_bin, short);

	GMT_err_trap (nc_get_vara_short (c->cdfid, c->bin_info_id, start, count, stmp));
	for (i = 0; i < c->nb; i++) c->bin_info[i] = stmp[c->bins[i]];

	GMT_err_trap (nc_get_vara_short (c->cdfid, c->bin_nseg_id, start, count, stmp));
	for (i = 0; i < c->nb; i++) c->bin_nseg[i] = stmp[c->bins[i]];
	GMT_free (GMT, stmp);

	itmp = GMT_memory (GMT, NULL, c->n_bin, int);
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->bin_firstseg_id, start, count, itmp));
	for (i = 0; i < c->nb; i++) c->bin_firstseg[i] = itmp[c->bins[i]];

	GMT_free (GMT, itmp);

	return (GMT_NOERROR);
}

int GMT_get_shore_bin (struct GMT_CTRL *GMT, unsigned int b, struct GMT_SHORE *c)
/* b: index number into c->bins */
/* min_area: Polygons with area less than this are ignored */
/* min_level: Polygons with lower levels are ignored */
/* max_level: Polygons with higher levels are ignored */
{
	size_t start[1], count[1];
	int *seg_info = NULL, *seg_start = NULL, *seg_ID = NULL;
	int s, i, k, ny, err, level, inc[4], ll_node, node, ID, *seg_skip = NULL;
	double w, e, dx;

	c->node_level[0] = (unsigned char)MIN (((unsigned short)c->bin_info[b] >> 9) & 7, c->max_level);
	c->node_level[1] = (unsigned char)MIN (((unsigned short)c->bin_info[b] >> 6) & 7, c->max_level);
	c->node_level[2] = (unsigned char)MIN (((unsigned short)c->bin_info[b] >> 3) & 7, c->max_level);
	c->node_level[3] = (unsigned char)MIN ((unsigned short)c->bin_info[b] & 7, c->max_level);
	dx = c->bin_size / 60.0;
	c->lon_sw = (c->bins[b] % c->bin_nx) * dx;
	ny = (c->bins[b] / c->bin_nx) + 1;
	c->lat_sw = 90.0 - ny * dx;
	c->ns = 0;
	
	c->ant_special = (c->ant_mode && c->res == 'c' && ny == 8);	/* For crude we must split the 50-70S bin at 60S */

	/* Determine if this bin is one of the bins at the left side of the map */

	w = c->lon_sw;
	while (w > GMT->common.R.wesn[XLO] && GMT->current.map.is_world) w -= 360.0;
	e = w + dx;
	c->leftmost_bin = ((w <= GMT->common.R.wesn[XLO]) && (e > GMT->common.R.wesn[XLO]));

	if (c->bin_nseg[b] == 0) return (GMT_NOERROR);

	ll_node = ((c->bins[b] / c->bin_nx) + 1) * (c->bin_nx + 1) + (c->bins[b] % c->bin_nx);		/* lower-left node in current bin */
	inc[0] = 0;	inc[1] = 1;	inc[2] = 1 - (c->bin_nx + 1);	inc[3] = -(c->bin_nx + 1);	/* Relative incs to other nodes */

	if (c->min_area > 0.0) {	/* May have to revise the node_level array if the polygon that determined the level is to be skipped */
		for (k = 0; k < 4; k++) {	/* Visit all four nodes defining this bin, going counter-clockwise from lower-left bin */
			node = ll_node + inc[k];	/* Current node index */
			ID = c->GSHHS_node[node];	/* GSHHS Id of the polygon that determined the level of the current node */
			while (c->node_level[k] && c->GSHHS_area[ID] < c->min_area) {	/* Polygon must be skipped and node level reset */
				ID = c->GSHHS_parent[ID];	/* Pick the parent polygon since that is the next polygon up */
				c->node_level[k]--;		/* ...and drop down one level to that of the parent polygon */
			}	/* Keep doing this until the polygon containing the node is "too big to fail" or we are in the ocean */
		}
	}

	/* Here the node_level has been properly set but any polygons to be skipped have not been skipped yet; this happens below */

	start[0] = c->bin_firstseg[b];
	count[0] = c->bin_nseg[b];

	seg_info = GMT_memory (GMT, NULL, c->bin_nseg[b], int);
	seg_start = GMT_memory (GMT, NULL, c->bin_nseg[b], int);
	seg_ID = GMT_memory (GMT, NULL, c->bin_nseg[b], int);

	GMT_err_trap (nc_get_vara_int (c->cdfid, c->seg_info_id, start, count, seg_info));
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->seg_start_id, start, count, seg_start));
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->seg_GSHHS_ID_id, start, count, seg_ID));

	/* First tally how many useful segments */

	seg_skip = GMT_memory (GMT, NULL, c->bin_nseg[b], int);
	for (i = 0; i < c->bin_nseg[b]; i++) {
		seg_skip[i] = true;	/* Reset later to false if we pass all the tests to follow next */
		if (c->GSHHS_area_fraction[seg_ID[i]] < c->fraction) continue;	/* Area of this feature is too small relative to its original size */
		if (fabs (c->GSHHS_area[seg_ID[i]]) < c->min_area) continue;		/* Too small. NOTE: Use fabs() since double-lined-river lakes have negative area */
		level = get_level (seg_info[i]);
		if (level < c->min_level) continue;
		if (level > c->max_level) continue;
		if (level == 2 && c->GSHHS_area[seg_ID[i]] < 0 && c->flag == GSHHS_NO_RIVERLAKES) continue;
		if (level == 2 && c->GSHHS_area[seg_ID[i]] > 0 && c->flag == GSHHS_NO_LAKES) continue;
		seg_skip[i] = false;	/* OK, so this was needed afterall */
	}
	if (c->skip_feature) {	/* Must ensure that we skip all features inside a skipped riverlake/lake */
		int j, feature;
		if (c->flag == GSHHS_NO_LAKES && c->node_level[0] == c->node_level[1] && c->node_level[2] == c->node_level[3] && c->node_level[0] == c->node_level[3] && c->node_level[0] == 2) {	/* Bin is entirely inside a lake */
			for (i = 0; i < c->bin_nseg[b]; i++) seg_skip[i] = true;	/* Must skip all segments in the lake */
			c->node_level[0] = c->node_level[1] = c->node_level[2] = c->node_level[3] = 1;	/* Bin is now entirely inside land */
		}
		else {	/* Must find all level 3 and 4 features whose parent is level 2 and has been skipped. Then these level 3/4 features must be skipped too */
			for (feature = 3; feature <= 4; feature++) {	/* Must check twice; first for islands-in-lakes (3), then ponds in such islands (4) */
				for (i = 0; i < c->bin_nseg[b]; i++) {	/* Go through entire segment list */
					if (get_level (seg_info[i]) != feature) continue;		/* We are only looking for levels 3 or 4 here */
					/* Here segment i is a level 3 (or 4) feature */
					for (j = 0; j < c->bin_nseg[b]; j++) {	/* Go through entire segment list again */
						if (get_level (seg_info[j]) != (feature-1)) continue;	/* We are looking for the containing polygon here which is one level up */
						/* Here segment j is of level 1 higher than segment i (2 or 3) */
						if (c->GSHHS_parent[seg_ID[i]] == seg_ID[j] && seg_skip[j]) {	/* j is parent of i but j is to be skipped */
							seg_skip[i] = true;	/* We must therefore skip feature i as well */
							/* Complication: Check if this feature happened to be a node-level-determining polygon... */
							for (k = 0; k < 4; k++) {	/* Visit all four nodes defining this bin, going counter-clockwise from lower-left bin */
								node = ll_node + inc[k];	/* Current node index */
								ID = c->GSHHS_node[node];	/* GSHHS Id of the polygon that determined the level of the current node */
								if (seg_ID[i] == ID) c->node_level[k] = 1;	/* Polygon must be skipped and node level reset to land (1) */
							}
						}
					}
				}
			}
		}
	}

	/* Here, seg_skip indicates all segments that will be skipped */

	for (s = i = 0; i < c->bin_nseg[b]; i++) {
		if (seg_skip[i]) continue;	/* Marked to be skipped */
		seg_info[s] = seg_info[i];
		seg_start[s] = seg_start[i];
		s++;
	}
	c->ns = s;

	if (c->ns == 0) {	/* No useful segments in this bin */
		GMT_free (GMT, seg_skip);
		GMT_free (GMT, seg_info);
		GMT_free (GMT, seg_start);
		GMT_free (GMT, seg_ID);
		return (GMT_NOERROR);
	}

	c->seg = GMT_memory (GMT, NULL, c->ns, struct GMT_SHORE_SEGMENT);

	for (s = 0; s < c->ns; s++) {
		c->seg[s].level = get_level (seg_info[s]);
		c->seg[s].n = (short)(seg_info[s] >> 9);
		c->seg[s].entry = (seg_info[s] >> 3) & 7;
		c->seg[s].exit = seg_info[s] & 7;
		c->seg[s].fid = (c->GSHHS_area[seg_ID[s]] < 0) ? RIVERLAKE : c->seg[s].level;
		c->seg[s].dx = GMT_memory (GMT, NULL, c->seg[s].n, short);
		c->seg[s].dy = GMT_memory (GMT, NULL, c->seg[s].n, short);
		start[0] = seg_start[s];
		count[0] = c->seg[s].n;
		GMT_err_trap (nc_get_vara_short (c->cdfid, c->pt_dx_id, start, count, c->seg[s].dx));
		GMT_err_trap (nc_get_vara_short (c->cdfid, c->pt_dy_id, start, count, c->seg[s].dy));
	}

	GMT_free (GMT, seg_skip);
	GMT_free (GMT, seg_info);
	GMT_free (GMT, seg_start);
	GMT_free (GMT, seg_ID);

	return (GMT_NOERROR);
}

int GMT_init_br (struct GMT_CTRL *GMT, char which, char res, struct GMT_BR *c, double wesn[])
/* which: r(iver) or b(order) */
/* res: Resolution (f, h, i, l, c */
{
	int i, nb, idiv, iw, ie, is, in, this_south, this_west, err;
	short *stmp = NULL;
	int *itmp = NULL;
	size_t start[1], count[1];
	char stem[GMT_LEN64] = {""}, path[GMT_BUFSIZ] = {""};

	/* zap structure (nc_get_att_text does not null-terminate strings!) */
	GMT_memset (c, 1, struct GMT_BR);

	if (which == 'r')
		sprintf (stem, "binned_river_%c", res);
	else
		sprintf (stem, "binned_border_%c", res);

	if (!gmt_shore_getpathname (GMT, stem, path))
		return (GMT_GRDIO_FILE_NOT_FOUND); /* Failed to find file */

	GMT_err_trap (nc_open (path, NC_NOWRITE, &c->cdfid));

	/* Get all id tags */
	GMT_err_trap (nc_inq_varid (c->cdfid, "Bin_size_in_minutes", &c->bin_size_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_360_longitude_range", &c->bin_nx_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_180_degree_latitude_range", &c->bin_ny_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_file", &c->n_bin_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_segments_in_file", &c->n_seg_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_points_in_file", &c->n_pt_id));

	GMT_err_trap (nc_inq_varid (c->cdfid, "Id_of_first_segment_in_a_bin", &c->bin_firstseg_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_segments_in_a_bin", &c->bin_nseg_id));

	GMT_err_trap (nc_inq_varid (c->cdfid, "N_points_for_a_segment", &c->seg_n_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Hierarchial_level_of_a_segment", &c->seg_level_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Id_of_first_point_in_a_segment", &c->seg_start_id));

	GMT_err_trap (nc_inq_varid (c->cdfid, "Relative_longitude_from_SW_corner_of_bin", &c->pt_dx_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Relative_latitude_from_SW_corner_of_bin", &c->pt_dy_id));

	/* Get attributes */
	GMT_err_trap (nc_get_att_text (c->cdfid, c->pt_dx_id, "units", c->units));
	GMT_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "title", c->title));
	GMT_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "source", c->source));
	GMT_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "version", c->version));

	/* Get global variables */

	start[0] = 0;

	GMT_err_trap (nc_get_var1_int (c->cdfid, c->bin_size_id, start, &c->bin_size));
	GMT_err_trap (nc_get_var1_int (c->cdfid, c->bin_nx_id, start, &c->bin_nx));
	GMT_err_trap (nc_get_var1_int (c->cdfid, c->bin_ny_id, start, &c->bin_ny));
	GMT_err_trap (nc_get_var1_int (c->cdfid, c->n_bin_id, start, &c->n_bin));
	GMT_err_trap (nc_get_var1_int (c->cdfid, c->n_seg_id, start, &c->n_seg));
	GMT_err_trap (nc_get_var1_int (c->cdfid, c->n_pt_id, start, &c->n_pt));


	c->scale = (c->bin_size / 60.0) / 65535.0;
	c->bsize = c->bin_size / 60.0;

	c->bins = GMT_memory (GMT, NULL, c->n_bin, int);

	/* Round off area to nearest multiple of block-dimension */

	iw = irint (floor (wesn[XLO] / c->bsize) * c->bsize);
	ie = irint (ceil (wesn[XHI] / c->bsize) * c->bsize);
	is = 90 - irint (ceil ((90.0 - wesn[YLO]) / c->bsize) * c->bsize);
	in = 90 - irint (floor ((90.0 - wesn[YHI]) / c->bsize) * c->bsize);
	idiv = irint (360.0 / c->bsize);	/* Number of blocks per latitude band */

	for (i = nb = 0; i < c->n_bin; i++) {	/* Find which bins are needed */
		this_south = 90 - irint (c->bsize * ((i / idiv) + 1));
		if (this_south < is || this_south >= in) continue;
		this_west = irint (c->bsize * (i % idiv)) - 360;
		while (this_west < iw) this_west += 360;
		if (this_west >= ie) continue;
		c->bins[nb] = i;
		nb++;
	}
	c->bins = GMT_memory (GMT, c->bins, nb, int);
	c->nb = nb;

	/* Get bin variables, then extract only those corresponding to the bins to use */

	/* Allocate space for arrays of bin information */

	c->bin_nseg     = GMT_memory (GMT, NULL, nb, short);
	c->bin_firstseg     = GMT_memory (GMT, NULL, nb, int);

	count[0] = c->n_bin;
	stmp = GMT_memory (GMT, NULL, c->n_bin, short);

	GMT_err_trap (nc_get_vara_short (c->cdfid, c->bin_nseg_id, start, count, stmp));
	for (i = 0; i < c->nb; i++) c->bin_nseg[i] = stmp[c->bins[i]];
	GMT_free (GMT, stmp);

	itmp = GMT_memory (GMT, NULL, c->n_bin, int);
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->bin_firstseg_id, start, count, itmp));
	for (i = 0; i < c->nb; i++) c->bin_firstseg[i] = itmp[c->bins[i]];

	GMT_free (GMT, itmp);

	return (0);
}

int GMT_get_br_bin (struct GMT_CTRL *GMT, unsigned int b, struct GMT_BR *c, unsigned int *level, unsigned int n_levels)
/* b: index number into c->bins */
/* level: Levels of features to extract */
/* n_levels: # of such levels. 0 means use all levels */
{
	size_t start[1], count[1];
	int *seg_start = NULL;
	short *seg_n = NULL, *seg_level = NULL, s_level;
	int s, i, err;
	unsigned int k;
	bool skip;
	

	c->lon_sw = (c->bins[b] % c->bin_nx) * c->bin_size / 60.0;
	c->lat_sw = 90.0 - ((c->bins[b] / c->bin_nx) + 1) * c->bin_size / 60.0;
	c->ns = c->bin_nseg[b];

	if (c->ns == 0) return (GMT_NOERROR);

	start[0] = c->bin_firstseg[b];
	count[0] = c->bin_nseg[b];

	seg_n = GMT_memory (GMT, NULL, c->bin_nseg[b], short);
	seg_level = GMT_memory (GMT, NULL, c->bin_nseg[b], short);
	seg_start = GMT_memory (GMT, NULL, c->bin_nseg[b], int);

	GMT_err_trap (nc_get_vara_short (c->cdfid, c->seg_n_id, start, count, seg_n));
	GMT_err_trap (nc_get_vara_short (c->cdfid, c->seg_level_id, start, count, seg_level));
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->seg_start_id, start, count, seg_start));

	c->seg = NULL;
	for (s = i = 0; i < c->ns; i++) {
		if (n_levels == 0)
			skip = false;
		else {
			for (k = 0, skip = true; skip && k < n_levels; k++)
				if ((s_level = level[k]) == seg_level[i]) skip = false;
		}
		if (skip) continue;
		if (!c->seg) c->seg = GMT_memory (GMT, NULL, c->ns, struct GMT_BR_SEGMENT);
		c->seg[s].n = seg_n[i];
		c->seg[s].level = seg_level[i];
		c->seg[s].dx = GMT_memory (GMT, NULL, c->seg[s].n, short);
		c->seg[s].dy = GMT_memory (GMT, NULL, c->seg[s].n, short);
		start[0] = seg_start[i];
		count[0] = c->seg[s].n;
		GMT_err_trap (nc_get_vara_short (c->cdfid, c->pt_dx_id, start, count, c->seg[s].dx));
		GMT_err_trap (nc_get_vara_short (c->cdfid, c->pt_dy_id, start, count, c->seg[s].dy));

		s++;
	}

	c->ns = s;

	GMT_free (GMT, seg_n);
	GMT_free (GMT, seg_level);
	GMT_free (GMT, seg_start);

	return (GMT_NOERROR);
}

int GMT_assemble_shore (struct GMT_CTRL *GMT, struct GMT_SHORE *c, int dir, bool assemble, double west, double east, struct GMT_GSHHS_POL **pol)
/* assemble: true if polygons is needed, false if we just want to draw or dump outlines */
/* edge: Edge test for shifting of longitudes to avoid wraps */
{
	struct GMT_GSHHS_POL *p = NULL;
	int start_side, next_side, id, wet_or_dry, use_this_level, high_seg_level = GSHHS_MAX_LEVEL;
	int cid, nid, add, first_pos, entry_pos, n, low_level, high_level, fid, nseg_at_level[GSHHS_MAX_LEVEL+1];
	bool completely_inside, more, skip = false;
	unsigned int P = 0, k;
	size_t n_alloc, p_alloc;
	double *xtmp = NULL, *ytmp = NULL, plon, plat;

	if (!assemble) {	/* Easy, just need to scale all segments to degrees and return */

		p = GMT_memory (GMT, NULL, c->ns, struct GMT_GSHHS_POL);

		for (id = P = 0; id < c->ns; id++) {
			p[P].lon = GMT_memory (GMT, NULL, c->seg[id].n, double);
			p[P].lat = GMT_memory (GMT, NULL, c->seg[id].n, double);
			p[P].n = gmt_copy_to_shore_path (p[P].lon, p[P].lat, c, id);
			if (c->ant_special) {	/* Discard any pieces south of 60S */
				for (k = 0, skip = true; skip && k < p[P].n; k++) if (p[P].lat[k] > -60.0) skip = false;
			}
			if (skip) {
				GMT_free (GMT, p[P].lon);
				GMT_free (GMT, p[P].lat);
			}
			else {
				p[P].level = c->seg[id].level;
				p[P].fid = c->seg[id].fid;
				p[P].interior = false;
				gmt_shore_path_shift2 (p[P].lon, p[P].n, west, east, c->leftmost_bin);
				P++;
			}
		}
		if (P < c->ns) p = GMT_memory (GMT, p, P, struct GMT_GSHHS_POL);	/* Trim memory */
		*pol = p;
		return (P);
	}

	/* Check the consistency of node levels in case some features have been dropped */

	GMT_memset (nseg_at_level, GSHHS_MAX_LEVEL + 1, int);
	for (id = 0; id < c->ns; id++) if (c->seg[id].entry != 4) nseg_at_level[c->seg[id].level]++;	/* Only count segments that crosses the bin */
	for (n = 0; n <= GSHHS_MAX_LEVEL; n++) if (nseg_at_level[n]) high_seg_level = n;

	if (c->ns == 0) for (n = 0; n < 4; n++) high_seg_level = MIN (c->node_level[n], high_seg_level);	/* Initialize to lowest when there are no segments */
	for (n = high_level = 0; n < 4; n++) {
		c->node_level[n] = (unsigned char)MIN (c->node_level[n], high_seg_level);
		high_level = MAX (c->node_level[n], high_level);
	}

	wet_or_dry = (dir == 1) ? 1 : 0;	/* If dir == 1 we paint the dry parts */
	use_this_level = (high_level%2 == wet_or_dry && high_level >= c->min_level);

	if (c->ns == 0 && !use_this_level) return (0);	/* No polygons for this bin */

	/* Here we must assemble [at least one] polygon(s) in the correct order */

	for (n = 0, completely_inside = true; completely_inside && n < c->ns; n++) if (c->seg[n].entry != 4) completely_inside = false;

	gmt_shore_prepare_sides (GMT, c, dir);	/* Initialize the book-keeping for how segments enters each of the four sides */

	/* Allocate 1 or more polygon structures */
	p_alloc = (c->ns == 0) ? 1 : GMT_SMALL_CHUNK;
	p = GMT_memory (GMT, NULL, p_alloc, struct GMT_GSHHS_POL);

	if (completely_inside && use_this_level) {	/* Must include path of this bin's outline as our first polygon, e.g., there may be no segments here but we are in the middle of a continent (or lake) */
		p[0].n = (int)GMT_graticule_path (GMT, &p[0].lon, &p[0].lat, dir, true, c->lon_corner[3], c->lon_corner[1], c->lat_corner[0], c->lat_corner[2]);
		p[0].level = (c->node_level[0] == 2 && c->flag == GSHHS_NO_LAKES) ? 1 : c->node_level[0];	/* Any corner will do */
		p[0].fid = p[0].level;	/* Override: Assumes no riverlake is that big to contain an entire bin */
		p[0].interior = false;
		P = 1;
	}

	while (c->n_entries > 0) {	/* More segments to connect into polygons */

		low_level = GSHHS_MAX_LEVEL;	/* Start outside range and find the lowest segment involved */
		/* Because a polygon will often be composed of segments that differ in level we need to find
		 * the lowest level as that indicates what the polygon is (lake, island, etc) and hence how
		 * it should be painted.  For instance, a piece of coastline (level 1) may be added to corners
		 * in the open ocean (level 0) and the resulting polygon is a piece of ocean (level 0). */

		start_side = 0;	/* We begin looking for segments entering along the south border of the bin, but gmt_shore_get_first_entry will determine what start_side really is */
		id = gmt_shore_get_first_entry (c, dir, &start_side);	/* This is the first segment to enter (measured from the west) and we return its ID and side via start_side */
		next_side = c->seg[id].exit;	/* The segment will then exit on possibly another side or the same side */

		n_alloc = c->seg[id].n;		/* Need this much space to hold the segment */
		fid = c->seg[id].fid;		/* Fill id (same as level expect for riverlakes which is 5) */
		/* Allocate space for our new polygon */
		p[P].lon = GMT_memory (GMT, NULL, n_alloc, double);
		p[P].lat = GMT_memory (GMT, NULL, n_alloc, double);
		n = gmt_copy_to_shore_path (p[P].lon, p[P].lat, c, id);			/* Creates a lon-lat path from the segment */
		if ((int)c->seg[id].level < low_level) low_level = c->seg[id].level;	/* Update the lowest level involved */

		more = true;	/* Until we are done with all segments */
		first_pos = gmt_shore_get_position (start_side, c->seg[id].dx[0], c->seg[id].dy[0]);	/* This is the relative starting position (0-65535) on the start side for current segment */
		/* Remember, the segments have been sorted along each side according to entry position */
		while (more) {	/* Unless we run out or close the polygon we need to add more segments */

			id = gmt_shore_get_next_entry (c, dir, next_side, id);	/* Find the ID of the next segment along this side, OR the corner if no segments remain */

			if (id < 0) {	/* Found a corner */
				cid = id + 4;	/* ID of the corner */
				nid = (dir == 1) ? (cid + 1) % 4 : cid;	/* Next corner [I think] */
				if ((add = (int)GMT_map_path (GMT, p[P].lon[n-1], p[P].lat[n-1], c->lon_corner[cid], c->lat_corner[cid], &xtmp, &ytmp))) {
					/* Add the bin-border segment from last point in the growing polygon to the specified corner */
					n_alloc += add;
					p[P].lon = GMT_memory (GMT, p[P].lon, n_alloc, double);
					p[P].lat = GMT_memory (GMT, p[P].lat, n_alloc, double);
					GMT_memcpy (&p[P].lon[n], xtmp, add, double);
					GMT_memcpy (&p[P].lat[n], ytmp, add, double);
					n += add;
				}
				next_side = ((id + 4) + dir + 4) % 4;	/* This will go to the next side either CCW or CW, depending on dir */
				if ((int)c->node_level[nid] < low_level) low_level = c->node_level[nid];	/* Update lowest level involved */
			}
			else {	/* Found a segment to add to our polygon */
				gmt_shore_to_degree (c, c->seg[id].dx[0], c->seg[id].dy[0], &plon, &plat);	/* Get lon,lat of start of segment */
				if ((add = (int)GMT_map_path (GMT, p[P].lon[n-1], p[P].lat[n-1], plon, plat, &xtmp, &ytmp))) {
					/* Connect the last point in the growing polygon with the starting point of this next segment */
					n_alloc += add;
					p[P].lon = GMT_memory (GMT, p[P].lon, n_alloc, double);
					p[P].lat = GMT_memory (GMT, p[P].lat, n_alloc, double);
					GMT_memcpy (&p[P].lon[n], xtmp, add, double);
					GMT_memcpy (&p[P].lat[n], ytmp, add, double);
					n += add;
				}
				entry_pos = gmt_shore_get_position (next_side, c->seg[id].dx[0], c->seg[id].dy[0]);	/* Position on the next side */
				if (next_side == start_side && entry_pos == first_pos)	/* We have closed the polygon; done */
					more = false;
				else {	/* Add the segment to our growing polygon */
					n_alloc += c->seg[id].n;
					p[P].lon = GMT_memory (GMT, p[P].lon, n_alloc, double);
					p[P].lat = GMT_memory (GMT, p[P].lat, n_alloc, double);
					n += gmt_copy_to_shore_path (&p[P].lon[n], &p[P].lat[n], c, id);
					next_side = c->seg[id].exit;	/* Update which side we are on after adding the segment */
					if ((int)c->seg[id].level < low_level) low_level = c->seg[id].level;	/* Update lowest level involved */
				}
			}
			if (add) {	/* Free temporary variables if used */
				GMT_free (GMT, xtmp);
				GMT_free (GMT, ytmp);
			}
		}
		if (c->ant_special) {	/* Discard any pieces south of 60S */
			for (k = 0, skip = true; skip && k < n; k++) if (p[P].lat[k] > -60.0) skip = false;
		}
		if (skip) {
			GMT_free (GMT, p[P].lon);
			GMT_free (GMT, p[P].lat);
		}
		else {
			/* Update information for this closed polygon and increase polygon counter (allocate more space if needed) */
			p[P].n = n;
			p[P].interior = false;
			p[P].level = (dir == 1) ? 2 * ((low_level - 1) / 2) + 1 : 2 * (low_level/2);	/* Convoluted way of determining which level this polygon belongs to (for painting) */
			p[P].fid = (p[P].level == 2 && fid == RIVERLAKE) ? RIVERLAKE : p[P].level;	/* Not sure about this yet */
			P++;
			if (P == p_alloc) {
				size_t old_p_alloc = p_alloc;
				p_alloc <<= 1;
				p = GMT_memory (GMT, p, p_alloc, struct GMT_GSHHS_POL);
				GMT_memset (&(p[old_p_alloc]), p_alloc - old_p_alloc, struct GMT_GSHHS_POL);	/* Set to NULL/0 */
			}
		}
		/* Then we go back to top of loop and if there are more segments we start all over with a new polygon */
	}

	/* Then add all interior polygons, if any.  These just needs to be converted to lon,lat, have their level set, and added to the list of polygons */

	for (id = 0; id < c->ns; id++) {
		if (c->seg[id].entry < 4) continue;
		n_alloc = c->seg[id].n;
		p[P].lon = GMT_memory (GMT, NULL, n_alloc, double);
		p[P].lat = GMT_memory (GMT, NULL, n_alloc, double);
		p[P].n = gmt_copy_to_shore_path (p[P].lon, p[P].lat, c, id);
		if (c->ant_special) {	/* Discard any pieces south of 60S */
			for (k = 0, skip = true; skip && k < p[P].n; k++) if (p[P].lat[k] > -60.0) skip = false;
		}
		if (skip) {
			GMT_free (GMT, p[P].lon);
			GMT_free (GMT, p[P].lat);
		}
		else {
			p[P].interior = true;
			p[P].level = c->seg[id].level;
			p[P].fid = c->seg[id].fid;
			P++;
			if (P == p_alloc) {
				size_t old_p_alloc = p_alloc;
				p_alloc <<= 1;
				p = GMT_memory (GMT, p, p_alloc, struct GMT_GSHHS_POL);
				GMT_memset (&(p[old_p_alloc]), p_alloc - old_p_alloc, struct GMT_GSHHS_POL);	/* Set to NULL/0 */
			}
		}
	}

	gmt_shore_done_sides (GMT, c);	/* Free array of side structures */

	if (c->ns > 0) p = GMT_memory (GMT, p, P, struct GMT_GSHHS_POL);	/* Trim memory */

	for (k = 0; k < P; k++) gmt_shore_path_shift2 (p[k].lon, p[k].n, west, east, c->leftmost_bin);	/* Deal with possible longitude -/+360 issues */

	*pol = p;
	return (P);	/* Return list of polygons found */
}

int GMT_assemble_br (struct GMT_CTRL *GMT, struct GMT_BR *c, bool shift, double edge, struct GMT_GSHHS_POL **pol)
/* shift: true if longitudes may have to be shifted */
/* edge: Edge test for shifting */
{
	struct GMT_GSHHS_POL *p = NULL;
	int id;

	p = GMT_memory (GMT, NULL, c->ns, struct GMT_GSHHS_POL);

	for (id = 0; id < c->ns; id++) {
		p[id].lon = GMT_memory (GMT, NULL, c->seg[id].n, double);
		p[id].lat = GMT_memory (GMT, NULL, c->seg[id].n, double);
		p[id].n = gmt_copy_to_br_path (p[id].lon, p[id].lat, c, id);
		p[id].level = c->seg[id].level;
		if (shift) gmt_shore_path_shift (p[id].lon, p[id].n, edge);
	}

	*pol = p;
	return (c->ns);
}

void GMT_free_shore (struct GMT_CTRL *GMT, struct GMT_SHORE *c)
{	/* Removes allocated variables for this block only */
	int i;

	for (i = 0; i < c->ns; i++) {
		GMT_free (GMT, c->seg[i].dx);
		GMT_free (GMT, c->seg[i].dy);
	}
	if (c->ns) GMT_free (GMT, c->seg);
}

void GMT_free_br (struct GMT_CTRL *GMT, struct GMT_BR *c)
{	/* Removes allocated variables for this block only */
	int i;

	for (i = 0; i < c->ns; i++) {
		GMT_free (GMT, c->seg[i].dx);
		GMT_free (GMT, c->seg[i].dy);
	}
	if (c->ns) GMT_free (GMT, c->seg);

}

void GMT_shore_cleanup (struct GMT_CTRL *GMT, struct GMT_SHORE *c)
{
	GMT_free (GMT, c->bins);
	GMT_free (GMT, c->bin_info);
	GMT_free (GMT, c->bin_nseg);
	GMT_free (GMT, c->bin_firstseg);
	GMT_free (GMT, c->GSHHS_area);
	GMT_free (GMT, c->GSHHS_area_fraction);
	if (c->min_area > 0.0) GMT_free (GMT, c->GSHHS_node);
	GMT_free (GMT, c->GSHHS_parent);
	nc_close (c->cdfid);
}

void GMT_br_cleanup (struct GMT_CTRL *GMT, struct GMT_BR *c)
{
	GMT_free (GMT, c->bins);
	GMT_free (GMT, c->bin_nseg);
	GMT_free (GMT, c->bin_firstseg);
	nc_close (c->cdfid);
}

int GMT_prep_shore_polygons (struct GMT_CTRL *GMT, struct GMT_GSHHS_POL **p_old, unsigned int np, bool sample, double step, int anti_bin)
{
	/* This function will go through each of the polygons and determine
	 * if the polygon is clipped by the map boundary, and if so if it
	 * wraps around to the other side due to 360 degree periodicities
	 * A wrapped polygon will be returned as two new polygons so that
	 * this function may return more polygons that it receives.
	 * Upon return the polygons are in x,y inches, not degrees.
	 *
	 * *p is the array of np polygons
	 * sample is true if we need to resample the polygons to reduce point spacing
	 * step is the new maximum point separation in degrees
	 * anti_bin, if >= 0, indicates a possible problem bin at the antipole using -JE only
	 * We also explicitly close all polygons if they are not so already.
	 */

	unsigned int k, np_new, n, n_use;
	uint64_t start;
	bool close;
	size_t n_alloc;
	double *xtmp = NULL, *ytmp = NULL;
	struct GMT_GSHHS_POL *p = NULL;

	p = *p_old;

	np_new = np;

	for (k = 0; k < np; k++) {

		if (sample) p[k].n = (int)GMT_fix_up_path (GMT, &p[k].lon, &p[k].lat, p[k].n, step, 0);

		/* Clip polygon against map boundary if necessary and return plot x,y in inches */

		if ((n = (unsigned int)GMT_clip_to_map (GMT, p[k].lon, p[k].lat, p[k].n, &xtmp, &ytmp)) == 0) {	/* Completely outside */
			p[k].n = 0;	/* Note the memory in lon, lat not freed yet */
			continue;
		}

		/* Must check if polygon must be split and partially plotted at both edges of map */

		if ((*GMT->current.map.will_it_wrap) (GMT, xtmp, ytmp, n, &start)) {	/* Polygon does indeed wrap */

			/* First truncate against left border */

			GMT->current.plot.n = GMT_map_truncate (GMT, xtmp, ytmp, n, start, -1);
			n_use = (unsigned int)GMT_compact_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, false, 0);
			close = GMT_polygon_is_open (GMT, GMT->current.plot.x, GMT->current.plot.y, n_use);
			n_alloc = (close) ? n_use + 1 : n_use;
			p[k].lon = GMT_memory (GMT, p[k].lon, n_alloc, double);
			p[k].lat = GMT_memory (GMT, p[k].lat, n_alloc, double);
			GMT_memcpy (p[k].lon, GMT->current.plot.x, n_use, double);
			GMT_memcpy (p[k].lat, GMT->current.plot.y, n_use, double);
			if (close) {	/* Must explicitly close the polygon */
				p[k].lon[n_use] = p[k].lon[0];
				p[k].lat[n_use] = p[k].lat[0];
			}
			p[k].n = (int)n_alloc;

			/* Then truncate against right border */

			GMT->current.plot.n = GMT_map_truncate (GMT, xtmp, ytmp, n, start, +1);
			n_use = (unsigned int)GMT_compact_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, false, 0);
			p = GMT_memory (GMT, p, np_new + 1, struct GMT_GSHHS_POL);
			close = GMT_polygon_is_open (GMT, GMT->current.plot.x, GMT->current.plot.y, n_use);
			n_alloc = (close) ? n_use + 1 : n_use;
			p[np_new].lon = GMT_memory (GMT, NULL, n_alloc, double);
			p[np_new].lat = GMT_memory (GMT, NULL, n_alloc, double);
			GMT_memcpy (p[np_new].lon, GMT->current.plot.x, n_use, double);
			GMT_memcpy (p[np_new].lat, GMT->current.plot.y, n_use, double);
			if (close) {	/* Must explicitly close the polygon */
				p[np_new].lon[n_use] = p[np_new].lon[0];
				p[np_new].lat[n_use] = p[np_new].lat[0];
			}
			p[np_new].n = (int)n_alloc;
			p[np_new].interior = p[k].interior;
			p[np_new].level = p[k].level;
			p[np_new].fid = p[k].fid;
			np_new++;
		}
		else {
			n_use = (unsigned int)GMT_compact_line (GMT, xtmp, ytmp, n, false, 0);
			if (anti_bin > 0 && step == 0.0) {	/* Must warn for donut effect */
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: Antipodal bin # %d not filled!\n", anti_bin);
				GMT_free (GMT, xtmp);
				GMT_free (GMT, ytmp);
				continue;
			}
			else {
				close = GMT_polygon_is_open (GMT, xtmp, ytmp, n_use);
				n_alloc = (close) ? n_use + 1 : n_use;
				p[k].lon = GMT_memory (GMT, p[k].lon, n_alloc, double);
				p[k].lat = GMT_memory (GMT, p[k].lat, n_alloc, double);
				GMT_memcpy (p[k].lon, xtmp, n_use, double);
				GMT_memcpy (p[k].lat, ytmp, n_use, double);
				if (close) {	/* Must explicitly close the polygon */
					p[k].lon[n_use] = p[k].lon[0];
					p[k].lat[n_use] = p[k].lat[0];
				}
				p[k].n = (int)n_alloc;
			}
		}

		GMT_free (GMT, xtmp);
		GMT_free (GMT, ytmp);
	}

	*p_old = p;

	return (np_new);
}
