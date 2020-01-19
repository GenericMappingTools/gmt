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

/* NOTE: This is a new version for dual Antarctica polygons.
 * The general idea of GSHHG 2.3.x is that there are two sets of Antarctica continents and
 * island polygons, and only one can be active at any given time. The two candidates are
 * Source ID = 2: Antarctica ice-line [this is similar to old GSHHG 2.2.x lines but more accurate].
 * Source ID = 3: Antarctica shelf ice grounding line.
 * By default we use GSHHS_ANTARCTICA_ICE but users may select -A..+ag to pick the grounding line
 * [GSHHS_ANTARCTICA_GROUND] or even -A..+as to skip Antarctica entirely (to make it easier to plot
 * custom shorelines via psxy).
 * Because the grounding line polygons are always entirely inside a ice-shelf polygon, we have
 * given the grounding-line polygons a level of 6.  Of course, when these are used their levels are
 * reset to 1 and all the ice-shelf polygons are skipped.  The node corners of the underlying grid
 * are suitable for the ice-shelf line but if grounding line is selected a different array of node
 * corner values is used.  This means the GSHHG file is backwards compatible with earlier GMT versions
 * since the grounding-line related information is stored in a separate array.
 */

#include "gmt_dev.h"
#include "gmt_internals.h"
#include "gshhg_version.h"
/*
 * These functions simplifies the access to the GMT shoreline, border, and river
 * databases.
 *
 * The PUBLIC functions are (16):
 *
 * gmt_set_levels           : Modifies what items to extract from GSHHG database
 * gmt_set_resolution       : Converts resolutions f,h,i,l,c to integers 0-4
 * gmt_init_shore           : Opens selected shoreline database and initializes structures
 * gmt_get_shore_bin        : Returns all selected shore data for this bin
 * gmt_init_br              : Opens selected border/river database and initializes structures
 * gmt_get_br_bin           : Returns all selected border/river data for this bin
 * gmt_get_gshhg_lines      : Returns a GMT_DATASET with lines
 * gmt_assemble_shore       : Creates polygons or lines from shoreline segments
 * gmt_prep_shore_polygons  : Wraps polygons if necessary and prepares them for use
 * gmt_shore_level_at_point : Return hierarchical level at specified point
 * gmt_assemble_br          : Creates lines from border or river segments
 * gmt_free_shore           : Frees up memory used by shorelines for this bin
 * gmt_free_br              : Frees up memory used by shorelines for this bin
 * gmt_free_shore_polygons  : Frees list of polygon coordinates
 * gmt_shore_cleanup        : Frees up main shoreline structure memory
 * gmt_br_cleanup           : Frees up main river/border structure memory
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5.2.x
 *
 */

#define GSHHG_SITE "ftp://ftp.soest.hawaii.edu/gshhg/"

#define RIVERLAKE		5	/* Fill array id for riverlakes */
#define ANT_LEVEL_ICE		5
#define ANT_LEVEL_GROUND	6

#define get_exit(arg) ((arg) & 7)		/* Extract exit  (0-4) from bits 1-3 */
#define get_entry(arg) (((arg) >> 3) & 7)	/* Extract entry (0-4) from bits 4-6 */
#define get_level(arg) (((arg) >> 6) & 7)	/* Extract level (0-4) from bits 7-9 */
#define get_source(arg) (((arg) >> 9) & 7)	/* Extract source (0-7) from bits 10-12 */
#define get_np(arg) ((arg) >> 9)		/* Extract number of points from bits 10-64 */

/* ---------- LOCAL FUNCTIONS CALLED BY THE PUBLIC FUNCTIONS ------------ */

GMT_LOCAL void shore_to_degree (struct GMT_SHORE *c, short int dx, short int dy, double *lon, double *lat) {
	/* Converts relative (0-65535) coordinates to actual lon, lat values */
	*lon = c->lon_sw + ((unsigned short)dx) * c->scale;
	*lat = c->lat_sw + ((unsigned short)dy) * c->scale;
}

GMT_LOCAL void shore_br_to_degree (struct GMT_BR *c, short int dx, short int dy, double *lon, double *lat) {
	/* Converts relative (0-65535) coordinates to actual lon, lat values */
	*lon = c->lon_sw + ((unsigned short)dx) * c->scale;
	*lat = c->lat_sw + ((unsigned short)dy) * c->scale;
}

GMT_LOCAL int shore_copy_to_shore_path (double *lon, double *lat, struct GMT_SHORE *s, int id) {
	/* Convert a shore segment to degrees and add to array */
	int i;
	for (i = 0; i < (int)s->seg[id].n; i++)
		shore_to_degree (s, s->seg[id].dx[i], s->seg[id].dy[i], &lon[i], &lat[i]);
	return (s->seg[id].n);
}

GMT_LOCAL int shore_copy_to_br_path (double *lon, double *lat, struct GMT_BR *s, int id) {
	/* Convert a line segment to degrees and add to array */
	int i;
	for (i = 0; i < (int)s->seg[id].n; i++)
		shore_br_to_degree (s, s->seg[id].dx[i], s->seg[id].dy[i], &lon[i], &lat[i]);
	return (s->seg[id].n);
}

GMT_LOCAL int shore_get_position (int side, short int x, short int y) {
	/* Returns the position along the given side, measured from start of side */
	return ((side%2) ? ((side == 1) ? (unsigned short)y : GSHHS_MAX_DELTA - (unsigned short)y) : ((side == 0) ? (unsigned short)x : GSHHS_MAX_DELTA - (unsigned short)x));
}

GMT_LOCAL int shore_get_next_entry (struct GMT_SHORE *c, int dir, int side, int id) {
	/* Finds the next entry point on the given side that is further away
	 * in the <dir> direction than previous point.  It then removes the info
	 * regarding the new entry from the GSHHS_SIDE structure so it won't be
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
		pos = shore_get_position (side, c->seg[id].dx[n], c->seg[id].dy[n]);
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

GMT_LOCAL int shore_get_first_entry (struct GMT_SHORE *c, int dir, int *side) {
	/* Loop over all sides and find the first available entry, starting at *side and moving around counter-clockwise.
	 * We only return IDs of segments and do not consider any corner points here - that is handled separately */
	int try = 0;	/* We have max 4 tries, i.e., all 4 sides */
	while (try < 4 && (c->nside[*side] == 0 || (c->nside[*side] == 1 && c->side[*side][0].id < 0))) {	/* No entries or only a corner left on this side */
		try++;	/* Try again */
		*side = (*side + dir + 4) % 4;	/* This is the next side going CCW */
	}
	if (try == 4) return (-5);	/* No luck finding any side with a segment */
	return (c->side[*side][0].id);	/* Return the ID of the segment; its side is returned via *side */
}

GMT_LOCAL int shore_asc_sort (const void *a, const void *b) {
	/* Sort segment into ascending order based on entry positions for going CCW */
	if (((struct GSHHS_SIDE *)a)->pos < ((struct GSHHS_SIDE *)b)->pos) return (-1);
	if (((struct GSHHS_SIDE *)a)->pos > ((struct GSHHS_SIDE *)b)->pos) return (1);
	return (0);
}

GMT_LOCAL int shore_desc_sort (const void *a, const void *b) {
	/* Sort segment into descending order based on entry positions for going CW */
	if (((struct GSHHS_SIDE *)a)->pos < ((struct GSHHS_SIDE *)b)->pos) return (1);
	if (((struct GSHHS_SIDE *)a)->pos > ((struct GSHHS_SIDE *)b)->pos) return (-1);
	return (0);
}

GMT_LOCAL void shore_done_sides (struct GMT_CTRL *GMT, struct GMT_SHORE *c) {
	/* Free the now empty list of side structures */
	unsigned int i;
	for (i = 0; i < 4; i++) gmt_M_free (GMT, c->side[i]);
}

GMT_LOCAL void shore_path_shift (double *lon, unsigned int n, double edge) {
	/* Shift all longitudes >= edge by 360 westwards */
	unsigned int i;

	for (i = 0; i < n; i++) if (lon[i] >= edge) lon[i] -= 360.0;
}

GMT_LOCAL void shore_path_shift2 (double *lon, unsigned int n, double west, double east, int leftmost) {
	/* Adjust longitudes so there are no jumps with respect to current bin boundaries */
	unsigned int i;

	if (leftmost) {	/* Must check this bin differently  */
		for (i = 0; i < n; i++) if (lon[i] >= east && (lon[i]-360.0) >= west) lon[i] -= 360.0;
	}
	else {
		for (i = 0; i < n; i++) if (lon[i] > east && (lon[i]-360.0) >= west) lon[i] -= 360.0;
	}
}

GMT_LOCAL void shore_prepare_sides (struct GMT_CTRL *GMT, struct GMT_SHORE *c, int dir) {
	/* Initializes the GSHHS_SIDE structures for each side, then adds corners and all entering segments */
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
		c->side[i] = gmt_M_memory (GMT, NULL, c->nside[i], struct GSHHS_SIDE);
		c->side[i][0].pos = (dir == 1) ? GSHHS_MAX_DELTA : 0;	/* position at end of side depends if going CCW (65535) or CW (0) */
		c->side[i][0].id = (short int)(i - 4);	/* Corners have negative IDs; add 4 to get real ID */
		c->n_entries += c->nside[i] - 1;	/* Total number of entries so far */
	}

	for (s = 0; s < c->ns; s++) {	/* Now add entry points for each segment */
		/* if (c->seg[s].level > 2 || (i = c->seg[s].entry) == 4) continue; */
		if ((i = c->seg[s].entry) == 4) continue;
		c->side[i][n[i]].pos = (unsigned short)shore_get_position (i, c->seg[s].dx[0], c->seg[s].dy[0]);
		c->side[i][n[i]].id = (short)s;
		n[i]++;
	}

	/* We then sort the array of GSHHS_SIDE structs on their distance from the start of the side */
	for (i = 0; i < 4; i++)	{	/* sort on position */
		if (dir == 1)
			qsort (c->side[i], (size_t)c->nside[i], sizeof (struct GSHHS_SIDE), shore_asc_sort);
		else
			qsort (c->side[i], (size_t)c->nside[i], sizeof (struct GSHHS_SIDE), shore_desc_sort);
	}
}

GMT_LOCAL char *shore_getpathname (struct GMT_CTRL *GMT, char *stem, char *path, bool reset) {
	/* Prepends the appropriate directory to the file name
	 * and returns path if file is readable, NULL otherwise */

	FILE *fp = NULL;
	char dir[PATH_MAX];
	static struct GSHHG_VERSION version = GSHHG_MIN_REQUIRED_VERSION;
	static bool warn_once = true;
	bool found = false;

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
		if (access (path, F_OK) == 0) {	/* File exists here */
			if (access (path, R_OK) == 0 && gshhg_require_min_version (path, version) ) {
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "1. GSHHG: OK, could access %s\n", path);
				return (path);
			}
			else
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "1. GSHHG: Found %s but cannot read it due to wrong permissions\n", path);
		}
		else {
			/* remove reference to invalid GMT->session.GSHHGDIR but don't free
			 * the pointer. this is no leak because the reference still exists
			 * in the previous copy of the current GMT_CTRL struct. */
			if (reset) GMT->session.GSHHGDIR = NULL;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "1. GSHHG: Failure, could not access %s\n", path);
		}
	}

	/* 2. Next, check for coastline.conf */

	if (gmt_getsharepath (GMT, "conf", "coastline", ".conf", path, F_OK) || gmt_getsharepath (GMT, "coast", "coastline", ".conf", path, F_OK)) {

		/* We get here if coastline.conf exists - search among its directories for the named file */

		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "2. GSHHG: coastline.conf found at %s\n", path);
		if (access (path, R_OK) == 0) {				/* coastline.conf can be read */
			if ((fp = fopen (path, "r")) == NULL) {		/* but Coverity still complains if we don't test if it's NULL */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "2. GSHHG: Failed to open %s\n", path);
				return (NULL);
			}
			while (fgets (dir, PATH_MAX, fp)) {	/* Loop over all input lines until found or done */
				if (dir[0] == '#' || dir[0] == '\n') continue;	/* Comment or blank */
				gmt_chop (dir);		/* Chop off LF or CR/LF */
				sprintf (path, "%s/%s%s", dir, stem, ".nc");
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "2. GSHHG: Trying %s\n", path);
				found = (access (path, F_OK) == 0);	/* File was found */
				if (access (path, R_OK) == 0) {		/* File can be read */
L1:
					if (gshhg_require_min_version (path, version)) {
						fclose (fp);
						/* update invalid GMT->session.GSHHGDIR */
						gmt_M_str_free (GMT->session.GSHHGDIR);
						GMT->session.GSHHGDIR = strdup (dir);
						GMT_Report (GMT->parent, GMT_MSG_DEBUG, "2. GSHHG: OK, could access %s\n", path);
						return (path);
					}
					else
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "2. GSHHG: Failure, could not access %s\n", path);
				}
				else {
					if (found)
						GMT_Report(GMT->parent, GMT_MSG_DEBUG, "2. GSHHG: Found %s but cannot read it due to wrong permissions\n", path);
					else {	/* Before giving up, try the old .cdf file names */
						sprintf(path, "%s/%s%s", dir, stem, ".cdf");
						if (access(path, R_OK) == 0)	/* Yes, old .cdf version found */
							goto L1;
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "2. GSHHG: Did not find %s nor ithe older *.cdf version\n", path);
					}
				}
			}
			fclose (fp);
		}
		else
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "2. GSHHG: Found %s but cannot read it due to wrong permissions\n", path);
	}

	/* 3. Then check for the named file itself */

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "3. GSHHG: Trying via sharepath\n");
	if (gmt_getsharepath (GMT, "coast", stem, ".nc", path, F_OK)) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "3. GSHHG: Trying %s\n", path);
		if ( access (path, R_OK) == 0) {	/* File can be read */
			if ( gshhg_require_min_version (path, version) ) {
				/* update invalid GMT->session.GSHHGDIR */
				snprintf (dir, PATH_MAX, "%s/%s", GMT->session.SHAREDIR, "coast");
				gmt_M_str_free (GMT->session.GSHHGDIR);
				GMT->session.GSHHGDIR = strdup (dir);
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "3. GSHHG: OK, could access %s\n", path);
				return (path);
			}
			else
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "3. GSHHG: Failure, could not access %s\n", path);
		}
		else
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "3. GSHHG: Found %s but cannot read it due to wrong permissions\n", path);
	}

	/* 4. No success, just break down and cry */

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "4. GSHHG: Failure, could not access any GSHHG files\n");
	if (warn_once && reset) {
		warn_once = false;
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GSHHG version %d.%d.%d or newer is "
								"needed to use coastlines with GMT.\n\tGet and install GSHHG from "
								GSHHG_SITE ".\n", version.major, version.minor, version.patch);
	}

	return (NULL); /* never reached */
}

GMT_LOCAL void shore_check (struct GMT_CTRL *GMT, bool ok[5]) {
/* Sets ok to true for those resolutions available in share for
 * resolution (f, h, i, l, c) */

	int i, j, n_found;
	char stem[GMT_LEN64] = {""}, path[PATH_MAX] = {""}, *res = "clihf", *kind[3] = {"GSHHS", "river", "border"};

	for (i = 0; i < 5; i++) {
		/* For each resolution... */
		ok[i] = false;
		for (j = n_found = 0; j < 3; j++) {
			/* For each data type... */
			snprintf (stem, GMT_LEN64, "binned_%s_%c", kind[j], res[i]);
			if (!shore_getpathname (GMT, stem, path, false))
				/* Failed to find file */
				continue;
			n_found++; /* Increment how many found so far for this resolution */
		}
		ok[i] = (n_found == 3);	/* Need all three sets to say this resolution is covered */
	}
}

GMT_LOCAL int shore_res_to_int (char res) {
	/* Turns a resolution letter into a 0-4 integer */
	int i, j;
	char *type = "clihf";

	for (i = -1, j = 0; i == -1 && j < 5; j++) if (res == type[j]) i = j;
	return (i);
}

/* Main Public GMT shore functions */

int gmt_set_levels (struct GMT_CTRL *GMT, char *info, struct GMT_SHORE_SELECT *I) {
	/* Decode GMT's -A option for coastline levels */
	int n;
	char *p = NULL;
	if ((p = strstr (info, "+a"))) {	/* On or more modifiers under +a */
		p += 2;	/* Skip to first letter */
		while (p[0] && p[0] != '+') {	/* Processes all codes until next modifier or we are done */
			switch (p[0]) {
				case 'g': I->antarctica_mode |= GSHHS_ANTARCTICA_GROUND;	break;	/* Use Antarctica shelf ice grounding line as coastline */
				case 'i': I->antarctica_mode |= GSHHS_ANTARCTICA_ICE;		break;	/* Use Antarctica ice boundary as coastline */
				case 's': I->antarctica_mode |= GSHHS_ANTARCTICA_SKIP;		break;	/* Skip Antarctica data south of 60S */
				case 'S': I->antarctica_mode |= GSHHS_ANTARCTICA_SKIP_INV;	break;	/* Skip everything BUT Antarctica data south of 60S */
				default:
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -A modifier +a: Invalid code %c\n", p[0]);
					GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
					break;
			}
			p++;	/* Go to next code */
		}
		if ((I->antarctica_mode & GSHHS_ANTARCTICA_GROUND) && (I->antarctica_mode & GSHHS_ANTARCTICA_ICE)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -A modifier +a: Cannot select both g and i\n");
			GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
		}
		if ((I->antarctica_mode & GSHHS_ANTARCTICA_SKIP) && (I->antarctica_mode & GSHHS_ANTARCTICA_SKIP_INV)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -A modifier +a: Cannot select both s and S\n");
			GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
		}
	}
	if (strstr (info, "+l"))  I->flag = GSHHS_NO_RIVERLAKES;
	if (strstr (info, "+r"))  I->flag = GSHHS_NO_LAKES;
	if ((p = strstr (info, "+p")) != NULL) {	/* Requested percentage limit on small features */
		I->fraction = irint (1e6 * 0.01 * atoi (&p[2]));	/* Convert percent to integer microfraction */
	}
	if (info[0] == '+') return (GMT_OK);	/* No area, etc, just modifiers that we just processed */
	n = sscanf (info, "%lf/%d/%d", &I->area, &I->low, &I->high);
	if (n == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -A option: No area given\n");
		GMT_exit (GMT, GMT_PARSE_ERROR); return GMT_PARSE_ERROR;
	}
	if (n == 1) I->low = 0, I->high = GSHHS_MAX_LEVEL;
	return (GMT_OK);
}

#define GMT_CRUDE_THRESHOLD	1e8
#define GMT_LOW_THRESHOLD	5e7
#define GMT_INT_THRESHOLD	1e7
#define GMT_HIGH_THRESHOLD	5e6
#define GMT_FULL_THRESHOLD	1e6	/* Not used */

int gmt_set_resolution (struct GMT_CTRL *GMT, char *res, char opt) {
	/* Decodes the -D<res> option and returns the base integer value */

	int base;
	char *choice = "fhilc";

	switch (*res) {
		case 'a':	/* Automatic selection via -J or -R, if possible */
			if (GMT->common.J.active) {	/* Use map scale xxxx as in 1:xxxx */
				double i_scale = 1.0 / (0.0254 * GMT->current.proj.scale[GMT_X]);
				if (i_scale > GMT_CRUDE_THRESHOLD)
					base = 4;	/* crude */
				else if (i_scale > GMT_LOW_THRESHOLD)
					base = 3;	/* low */
				else if (i_scale > GMT_INT_THRESHOLD)
					base = 2;	/* intermediate */
				else if (i_scale > GMT_HIGH_THRESHOLD)
					base = 1;	/* high */
				else
					base = 0;	/* full */
			}
			else if (GMT->common.R.active[RSET]) {	/* No scale, based on region only */
				double area, earth_area = 360 * 180; /* Flat Earth squared degrees */
				area = (GMT->common.R.wesn[GMT_XHI] - GMT->common.R.wesn[GMT_XLO]) * (GMT->common.R.wesn[GMT_YHI] - GMT->common.R.wesn[GMT_YLO]); /* Squared degrees */
				if (area > (pow (0.6, 2.0) * earth_area))
					base = 4;	/* crude */
				else if (area > (pow (0.6, 4.0) * earth_area))
					base = 3;	/* low */
				else if (area >  (pow (0.6, 6.0) * earth_area))
					base = 2;	/* intermediate */
				else if (area >  (pow (0.6, 8.0) * earth_area))
					base = 1;	/* high */
				else
					base = 0;	/* full */
			}
			else {	/* No basis - select low */
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-%c option: Cannot select automatic resolution without -R or -J [Default to low]\n");
				base = 3;	/* low */
			}
			*res = choice[base];
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "-%c option: Selected resolution -%c%c\n", opt, opt, *res);
			break;
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

char gmt_shore_adjust_res (struct GMT_CTRL *GMT, char res) {
	/* Returns the highest available resolution <= to specified resolution */
	int k, orig;
	bool ok[5];
	char *type = "clihf";
	(void)shore_check (GMT, ok);		/* See which resolutions we have */
	k = orig = shore_res_to_int (res);	/* Get integer value of requested resolution */
	while (k >= 0 && !ok[k]) --k;		/* Drop down one level to see if we have a lower resolution available */
	if (k >= 0 && k != orig) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Resolution %c not available, substituting resolution %c\n", res, type[k]);
	return ((k == -1) ? res : type[k]);	/* Return the chosen resolution */
}

int gmt_init_shore (struct GMT_CTRL *GMT, char res, struct GMT_SHORE *c, double wesn[], struct GMT_SHORE_SELECT *info) {
	/* res: Resolution (f, h, i, l, c */
	/* Opens the netcdf file and reads in all top-level attributes, IDs, and variables for all bins overlapping with wesn */
	int i, nb, idiv, iw, ie, is, in, i_ant, this_south, this_west, this_north, err;
	bool int_areas = false, two_Antarcticas = false;
	short *stmp = NULL;
	int *itmp = NULL;
	size_t start[1], count[1];
	char stem[GMT_LEN64] = {""}, path[PATH_MAX] = {""};

	snprintf (stem, GMT_LEN64, "binned_GSHHS_%c", res);

	if (!shore_getpathname (GMT, stem, path, true))
		return (GMT_GRDIO_FILE_NOT_FOUND); /* Failed to find file */

		/* zap structure (nc_get_att_text does not null-terminate strings!) */
		gmt_M_memset (c, 1, struct GMT_SHORE);

	/* Open shoreline file */
	gmt_M_err_trap (nc_open (path, NC_NOWRITE, &c->cdfid));

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "NetCDF Library Version: %s\n", nc_inq_libvers());

	/* Get global attributes */
	gmt_M_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "version", c->version));
	gmt_M_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "title", c->title));
	gmt_M_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "source", c->source));

	/* Get all id tags */
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Bin_size_in_minutes", &c->bin_size_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_360_longitude_range", &c->bin_nx_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_180_degree_latitude_range", &c->bin_ny_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_file", &c->n_bin_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_segments_in_file", &c->n_seg_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_points_in_file", &c->n_pt_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Id_of_first_segment_in_a_bin", &c->bin_firstseg_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Embedded_node_levels_in_a_bin", &c->bin_info_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Embedded_npts_levels_exit_entry_for_a_segment", &c->seg_info_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_segments_in_a_bin", &c->bin_nseg_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Id_of_first_point_in_a_segment", &c->seg_start_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Relative_longitude_from_SW_corner_of_bin", &c->pt_dx_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Relative_latitude_from_SW_corner_of_bin", &c->pt_dy_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Micro_fraction_of_full_resolution_area", &c->GSHHS_areafrac_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_polygons_in_file", &c->n_poly_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_nodes_in_file", &c->n_node_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Id_of_parent_polygons", &c->GSHHS_parent_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Id_of_node_polygons", &c->GSHHS_node_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Id_of_GSHHS_ID", &c->seg_GSHHS_ID_id));

	if (nc_inq_varid (c->cdfid, "Ten_times_the_km_squared_area_of_polygons", &c->GSHHS_area_id) == NC_NOERR) {	/* Old file with 1/10 km^2 areas in int format*/
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "GSHHS: Areas not accurate for small lakes and islands.  Consider updating GSHHG.\n");
		int_areas = true;
	}
	else if (nc_inq_varid (c->cdfid, "The_km_squared_area_of_polygons", &c->GSHHS_area_id) != NC_NOERR) {	/* New file with km^2 areas as doubles */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GSHHS: Unable to determine how polygon areas were stored.\n");
	}
	if (nc_inq_varid (c->cdfid, "Embedded_node_levels_in_a_bin_ANT", &c->bin_info_id_ANT) == NC_NOERR) {	/* New file with two Antarcticas */
		gmt_M_err_trap (nc_inq_varid (c->cdfid, "Embedded_ANT_flag", &c->seg_info_id_ANT));
		two_Antarcticas = true;
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GSHHG with two Antarcticas, read in extra ANT flgs.\n");
	}

	/* Get attributes */
	gmt_M_err_trap (nc_get_att_text (c->cdfid, c->pt_dx_id, "units", c->units));

	/* Get global variables */

	start[0] = 0;

	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->bin_size_id, start, &c->bin_size));
	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->bin_nx_id, start, &c->bin_nx));
	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->bin_ny_id, start, &c->bin_ny));
	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->n_bin_id, start, &c->n_bin));
	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->n_seg_id, start, &c->n_seg));
	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->n_pt_id, start, &c->n_pt));

	c->fraction = info->fraction;
	c->skip_feature = info->flag;
	c->min_area = info->area;	/* Limit the features */
	c->min_level = info->low;
	c->max_level = (info->low == info->high && info->high == 0) ? GSHHS_MAX_LEVEL : info->high;	/* Default to all if not set */
	c->flag = info->flag;
	c->two_Antarcticas = (two_Antarcticas) ? 1 : 0;
	c->ant_mode = info->antarctica_mode;
	if ((c->ant_mode & GSHHS_ANTARCTICA_GROUND) == 0)	/* Groundline not set, default to ice front */
		c->ant_mode |= GSHHS_ANTARCTICA_ICE;

	if (two_Antarcticas && gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) {	/* Report information regarding Antarctica */
		if (c->ant_mode & GSHHS_ANTARCTICA_GROUND)
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Selected ice grounding line as Antarctica coastline\n");
		else
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Selected ice front line as Antarctica coastline\n");
		if (c->ant_mode & GSHHS_ANTARCTICA_SKIP)
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Skipping Antarctica coastline entirely\n");
		else if (c->ant_mode & GSHHS_ANTARCTICA_SKIP_INV)
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Skipping all coastlines except Antarctica\n");
	}

	c->res = res;

	c->scale = (c->bin_size / 60.0) / 65535.0;
	c->bsize = c->bin_size / 60.0;
	info->bin_size = c->bsize;	/* To make bin size in degrees accessible elsewhere */

	c->bins = gmt_M_memory (GMT, NULL, c->n_bin, int);

	/* Round off area to nearest multiple of block-dimension */

	iw = irint (floor (wesn[XLO] / c->bsize) * c->bsize);
	ie = irint (ceil (wesn[XHI] / c->bsize) * c->bsize);
	is = 90 - irint (ceil ((90.0 - wesn[YLO]) / c->bsize) * c->bsize);
	in = 90 - irint (floor ((90.0 - wesn[YHI]) / c->bsize) * c->bsize);
	i_ant = 90 - irint (floor ((90.0 - GSHHS_ANTARCTICA_LIMIT) / c->bsize) * c->bsize);
	idiv = irint (360.0 / c->bsize);	/* Number of blocks per latitude band */

	for (i = nb = 0; i < c->n_bin; i++) {	/* Find which bins are needed */
		this_south = 90 - irint (c->bsize * ((i / idiv) + 1));	/* South limit of this bin */
		if (this_south < is || this_south >= in) continue;
		this_north = this_south + irint (c->bsize);
		if (c->ant_mode & GSHHS_ANTARCTICA_SKIP && this_north <= GSHHS_ANTARCTICA_LIMIT) continue;	/* Does not want Antarctica in output */
		else if (c->ant_mode & GSHHS_ANTARCTICA_SKIP_INV && this_south > i_ant) continue;	/* Does not want anything but Antarctica in output */
		this_west = irint (c->bsize * (i % idiv)) - 360;
		while (this_west < iw) this_west += 360;
		if (this_west >= ie) continue;
		c->bins[nb] = i;
		nb++;
	}
	c->bins = gmt_M_memory (GMT, c->bins, nb, int);
	c->nb = nb;

	/* Get polygon variables if they are needed */

	if ((err = nc_get_var1_int (c->cdfid, c->n_poly_id, start, &c->n_poly))) {
		gmt_shore_cleanup (GMT, c);	/* Free what we have so far and bail */
		return (err);
	}
	count[0] = c->n_poly;
	c->GSHHS_parent = gmt_M_memory (GMT, NULL, c->n_poly, int);
	if ((err = nc_get_vara_int (c->cdfid, c->GSHHS_parent_id, start, count, c->GSHHS_parent))) {
		gmt_shore_cleanup (GMT, c);	/* Free what we have so far and bail */
		return (err);
	}
	c->GSHHS_area = gmt_M_memory (GMT, NULL, c->n_poly, double);
	if ((err = nc_get_vara_double (c->cdfid, c->GSHHS_area_id, start, count, c->GSHHS_area))) {
		gmt_shore_cleanup (GMT, c);	/* Free what we have so far and bail */
		return (err);
	}
	if (int_areas) for (i = 0; i < c->n_poly; i++) c->GSHHS_area[i] *= 0.1;	/* Since they were stored as 10 * km^2 using integers */
	c->GSHHS_area_fraction = gmt_M_memory (GMT, NULL, c->n_poly, int);
	if ((err = nc_get_vara_int (c->cdfid, c->GSHHS_areafrac_id, start, count, c->GSHHS_area_fraction))) {
		gmt_shore_cleanup (GMT, c);	/* Free what we have so far and bail */
		return (err);
	}
	if (c->min_area > 0.0 || (info->flag & GSHHS_NO_RIVERLAKES) || (info->flag & GSHHS_NO_LAKES)) {	/* Want to exclude small polygons so we need info about the node polygons, or need info about lakes */
	        if ((err = nc_get_var1_int (c->cdfid, c->n_node_id, start, &c->n_nodes))) {
			gmt_shore_cleanup (GMT, c);	/* Free what we have so far and bail */
			return (err);
		}
		c->GSHHS_node = gmt_M_memory (GMT, NULL, c->n_nodes, int);
		count[0] = c->n_nodes;
		if ((err = nc_get_vara_int (c->cdfid, c->GSHHS_node_id, start, count, c->GSHHS_node))) {
			gmt_shore_cleanup (GMT, c);	/* Free what we have so far and bail */
			return (err);
		}
	}

	/* Get bin variables, then extract only those corresponding to the bins to use */

	/* Allocate space for arrays of bin information */

	c->bin_info      = gmt_M_memory (GMT, NULL, nb, short);
	c->bin_nseg      = gmt_M_memory (GMT, NULL, nb, short);
	c->bin_firstseg  = gmt_M_memory (GMT, NULL, nb, int);

	count[0] = c->n_bin;
	stmp = gmt_M_memory (GMT, NULL, c->n_bin, short);

	if (c->ant_mode & GSHHS_ANTARCTICA_ICE) {	/* Get node levels relevant for ice-shelf */
		err = nc_get_vara_short (c->cdfid, c->bin_info_id, start, count, stmp);
	}
	else {	/* Get node levels relevant for grounding line */
		err = nc_get_vara_short (c->cdfid, c->bin_info_id_ANT, start, count, stmp);
	}
	if (err) {
		gmt_shore_cleanup (GMT, c);	/* Free what we have so far and bail */
		gmt_M_free (GMT, stmp);
		return (err);
	}
	for (i = 0; i < c->nb; i++) c->bin_info[i] = stmp[c->bins[i]];

	if ((err = nc_get_vara_short (c->cdfid, c->bin_nseg_id, start, count, stmp))) {
		gmt_shore_cleanup (GMT, c);	/* Free what we have so far and bail */
		gmt_M_free (GMT, stmp);
		return (err);
	}
	for (i = 0; i < c->nb; i++) c->bin_nseg[i] = stmp[c->bins[i]];
	gmt_M_free (GMT, stmp);

	itmp = gmt_M_memory (GMT, NULL, c->n_bin, int);
	if ((err = nc_get_vara_int (c->cdfid, c->bin_firstseg_id, start, count, itmp))) {
		gmt_shore_cleanup (GMT, c);	/* Free what we have so far and bail */
		gmt_M_free (GMT, itmp);
		return (err);
	}
	for (i = 0; i < c->nb; i++) c->bin_firstseg[i] = itmp[c->bins[i]];

	gmt_M_free (GMT, itmp);

	return (GMT_NOERROR);
}

int gmt_get_shore_bin (struct GMT_CTRL *GMT, unsigned int b, struct GMT_SHORE *c) {
/* b: index number into c->bins */
/* min_area: Polygons with area less than this are ignored */
/* min_level: Polygons with lower levels are ignored */
/* max_level: Polygons with higher levels are ignored */
	size_t start[1], count[1];
	int *seg_info = NULL, *seg_start = NULL, *seg_ID = NULL;
	int s, i, k, ny, err, level, inc[4], ll_node, node, ID, *seg_skip = NULL;
	bool may_shrink = false;
	unsigned short corner[4], bitshift[4] = {9, 6, 3, 0};
	signed char *seg_info_ANT = NULL;
	double w, e, dx;

	for (k = 0; k < 4; k++) {	/* Extract node corner levels */
		corner[k] = ((unsigned short)c->bin_info[b] >> bitshift[k]) & 7;
		c->node_level[k] = (unsigned char)MIN (corner[k], c->max_level);
	}
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

	/* Trying to address issue https://github.com/GenericMappingTools/gmt/issues/1295.
	 * It only happens for very large -A values, such as -A8000.  I am trying a fix where
	 * we check if Antarctica with no polygons (i.e., just a tile) and large -A.
	 * It may have side effects to we keep that issue open for now.
	 */

	if (c->min_area > 0.0) {	/* May have to revise the node_level array if the polygon that determined the level is to be skipped */
		may_shrink = true;	/* Most likely, but check for Antarctica */
		for (k = 0; k < 4; k++) {	/* Visit all four nodes defining this bin, going counter-clockwise from lower-left bin */
			node = ll_node + inc[k];	/* Current node index */
			ID = c->GSHHS_node[node];	/* GSHHS Id of the polygon that determined the level of the current node */
			if ((ID == GSHHS_ANTARCTICA_ICE_ID || ID == GSHHS_ANTARCTICA_GROUND_ID) && c->ns == 0 && c->min_area > 5000.0) may_shrink = false;
		}
	}

	if (c->min_area > 0.0 && may_shrink) {	/* May have to revise the node_level array if the polygon that determined the level is to be skipped */
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

	seg_info = gmt_M_memory (GMT, NULL, c->bin_nseg[b], int);
	if ((err = nc_get_vara_int (c->cdfid, c->seg_info_id, start, count, seg_info))) {
		gmt_M_free (GMT, seg_info);
		return (err);
	}
	seg_start = gmt_M_memory (GMT, NULL, c->bin_nseg[b], int);
	if ((err = nc_get_vara_int (c->cdfid, c->seg_start_id, start, count, seg_start))) {
		gmt_M_free (GMT, seg_info);
		gmt_M_free (GMT, seg_start);
		return (err);
	}
	seg_ID = gmt_M_memory (GMT, NULL, c->bin_nseg[b], int);
	if ((err = nc_get_vara_int (c->cdfid, c->seg_GSHHS_ID_id, start, count, seg_ID))) {
		gmt_M_free (GMT, seg_info);
		gmt_M_free (GMT, seg_start);
		gmt_M_free (GMT, seg_ID);
		return (err);
	}

	if (c->two_Antarcticas) {	/* Read the flag that identifies Antarctica polygons */
		seg_info_ANT = gmt_M_memory (GMT, NULL, c->bin_nseg[b], signed char);
		if ((err = nc_get_vara_schar (c->cdfid, c->seg_info_id_ANT, start, count, seg_info_ANT))) {
			gmt_M_free (GMT, seg_info);
			gmt_M_free (GMT, seg_start);
			gmt_M_free (GMT, seg_ID);
			gmt_M_free (GMT, seg_info_ANT);
			return (err);
		}
	}

	/* First tally how many useful segments */

	seg_skip = gmt_M_memory (GMT, NULL, c->bin_nseg[b], int);
	for (i = 0; i < c->bin_nseg[b]; i++) {
		seg_skip[i] = true;	/* Reset later to false if we pass all the tests to follow next */
		if (c->GSHHS_area_fraction[seg_ID[i]] < c->fraction) continue;	/* Area of this feature is too small relative to its original size */
		if (may_shrink && fabs (c->GSHHS_area[seg_ID[i]]) < c->min_area) continue;	/* Too small. NOTE: Use fabs() since double-lined-river lakes have negative area */
		level = get_level (seg_info[i]);
		if (c->two_Antarcticas) {	/* Can apply any -A+ag|i check based on Antarctica source. Note if -A+as was used we may have already skipped this bin but it depends on resolution chosen */
			if (seg_info_ANT[i]) level = ANT_LEVEL_ICE;	/* Replace the 1 with 5 so Ant polygons now have levels 5 (ice) or 6 (ground) */
			if (level == ANT_LEVEL_ICE || level == ANT_LEVEL_GROUND) {	/* Need more specific checking */
				if (c->ant_mode & GSHHS_ANTARCTICA_SKIP) continue;	/* Don't want anything to do with Antarctica */
				else if (level == ANT_LEVEL_GROUND && c->ant_mode & GSHHS_ANTARCTICA_ICE) continue;	/* Don't use the Grounding line */
				else if (level == ANT_LEVEL_ICE && c->ant_mode & GSHHS_ANTARCTICA_GROUND && seg_ID[i] == GSHHS_ANTARCTICA_ICE_ID) continue;	/* Use grounding line so skip ice-shelf Antractica continent */
				level = 1;	/* Reset either shelf-ice or grounding line polygon level to land */
			}
			else if (c->ant_mode & GSHHS_ANTARCTICA_SKIP_INV) continue;	/* Wants nothing but Antarctica */
		}
		if (level < c->min_level) continue;	/* Test if level range was set */
		if (level > c->max_level) continue;
		if (level == 2 && c->GSHHS_area[seg_ID[i]] < 0 && c->flag == GSHHS_NO_RIVERLAKES) continue;
		if (level == 2 && c->GSHHS_area[seg_ID[i]] > 0 && c->flag == GSHHS_NO_LAKES) continue;
		seg_skip[i] = false;	/* OK, so this was needed after all */
	}
	if (c->skip_feature) {	/* Must ensure that we skip all features contained by a skipped riverlake/lake */
		int j, feature;
		if (c->flag == GSHHS_NO_LAKES && c->node_level[0] == c->node_level[1] && c->node_level[2] == c->node_level[3] && c->node_level[0] == c->node_level[3] && c->node_level[0] == 2) {	/* Bin is entirely inside a lake */
			for (i = 0; i < c->bin_nseg[b]; i++) seg_skip[i] = true;	/* Must skip all segments in the lake */
			c->node_level[0] = c->node_level[1] = c->node_level[2] = c->node_level[3] = 1;	/* Bin is now entirely inside land */
		}
		else {	/* Must find all level 3 and 4 features whose parent is level 2 and has been skipped. Then these level 3/4 features must be skipped too */
			for (feature = 3; feature <= 4; feature++) {	/* Must check twice; first for islands-in-lakes (3), then ponds in such islands (4) */
				for (i = 0; i < c->bin_nseg[b]; i++) {	/* Go through entire segment list */
					if (get_level (seg_info[i]) != feature) continue;	/* We are only looking for levels 3 or 4 here, so does not matter if level is 5,6 */
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
		gmt_M_free (GMT, seg_skip);
		gmt_M_free (GMT, seg_info);
		gmt_M_free (GMT, seg_start);
		gmt_M_free (GMT, seg_ID);
		if (c->two_Antarcticas) gmt_M_free (GMT, seg_info_ANT);
		return (GMT_NOERROR);
	}

	c->seg = gmt_M_memory (GMT, NULL, c->ns, struct GMT_SHORE_SEGMENT);

	for (s = 0; s < c->ns; s++) {
		c->seg[s].level = get_level (seg_info[s]);
		if (c->seg[s].level > 4) c->seg[s].level = 1;	/* Reset Antarctica segments to level 1 */
		c->seg[s].n = (short)get_np (seg_info[s]);
		c->seg[s].entry = get_entry (seg_info[s]);
		c->seg[s].exit  = get_exit (seg_info[s]);
		c->seg[s].fid = (c->GSHHS_area[seg_ID[s]] < 0) ? RIVERLAKE : c->seg[s].level;
		c->seg[s].dx = gmt_M_memory (GMT, NULL, c->seg[s].n, short);
		c->seg[s].dy = gmt_M_memory (GMT, NULL, c->seg[s].n, short);
		start[0] = seg_start[s];
		count[0] = c->seg[s].n;
		if ((err = nc_get_vara_short (c->cdfid, c->pt_dx_id, start, count, c->seg[s].dx)) ||
			(err = nc_get_vara_short (c->cdfid, c->pt_dy_id, start, count, c->seg[s].dy))) {
			gmt_free_shore (GMT, c);
			gmt_M_free (GMT, seg_skip);
			gmt_M_free (GMT, seg_info);
			gmt_M_free (GMT, seg_start);
			gmt_M_free (GMT, seg_ID);
			if (c->two_Antarcticas) gmt_M_free (GMT, seg_info_ANT);
			return (err);
		}
	}

	gmt_M_free (GMT, seg_skip);
	gmt_M_free (GMT, seg_info);
	gmt_M_free (GMT, seg_start);
	gmt_M_free (GMT, seg_ID);
	if (c->two_Antarcticas) gmt_M_free (GMT, seg_info_ANT);

	return (GMT_NOERROR);
}

int gmt_init_br (struct GMT_CTRL *GMT, char which, char res, struct GMT_BR *c, double wesn[]) {
/* which: r(iver) or b(order) */
/* res: Resolution (f, h, i, l, c */
	int i, nb, idiv, iw, ie, is, in, this_south, this_west, err;
	short *stmp = NULL;
	int *itmp = NULL;
	size_t start[1], count[1];
	char stem[GMT_LEN64] = {""}, path[PATH_MAX] = {""};

	/* zap structure (nc_get_att_text does not null-terminate strings!) */
	gmt_M_memset (c, 1, struct GMT_BR);

	if (which == 'r')
		snprintf (stem, GMT_LEN64, "binned_river_%c", res);
	else
		snprintf (stem, GMT_LEN64, "binned_border_%c", res);

	if (!shore_getpathname (GMT, stem, path, true))
		return (GMT_GRDIO_FILE_NOT_FOUND); /* Failed to find file */

	gmt_M_err_trap (nc_open (path, NC_NOWRITE, &c->cdfid));

	/* Get all id tags */
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Bin_size_in_minutes", &c->bin_size_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_360_longitude_range", &c->bin_nx_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_180_degree_latitude_range", &c->bin_ny_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_bins_in_file", &c->n_bin_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_segments_in_file", &c->n_seg_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_points_in_file", &c->n_pt_id));

	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Id_of_first_segment_in_a_bin", &c->bin_firstseg_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_segments_in_a_bin", &c->bin_nseg_id));

	gmt_M_err_trap (nc_inq_varid (c->cdfid, "N_points_for_a_segment", &c->seg_n_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Hierarchial_level_of_a_segment", &c->seg_level_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Id_of_first_point_in_a_segment", &c->seg_start_id));

	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Relative_longitude_from_SW_corner_of_bin", &c->pt_dx_id));
	gmt_M_err_trap (nc_inq_varid (c->cdfid, "Relative_latitude_from_SW_corner_of_bin", &c->pt_dy_id));

	/* Get attributes */
	gmt_M_err_trap (nc_get_att_text (c->cdfid, c->pt_dx_id, "units", c->units));
	gmt_M_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "title", c->title));
	gmt_M_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "source", c->source));
	gmt_M_err_trap (nc_get_att_text (c->cdfid, NC_GLOBAL, "version", c->version));

	/* Get global variables */

	start[0] = 0;

	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->bin_size_id, start, &c->bin_size));
	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->bin_nx_id, start, &c->bin_nx));
	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->bin_ny_id, start, &c->bin_ny));
	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->n_bin_id, start, &c->n_bin));
	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->n_seg_id, start, &c->n_seg));
	gmt_M_err_trap (nc_get_var1_int (c->cdfid, c->n_pt_id, start, &c->n_pt));


	c->scale = (c->bin_size / 60.0) / 65535.0;
	c->bsize = c->bin_size / 60.0;

	c->bins = gmt_M_memory (GMT, NULL, c->n_bin, int);

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
	c->bins = gmt_M_memory (GMT, c->bins, nb, int);
	c->nb = nb;

	/* Get bin variables, then extract only those corresponding to the bins to use */

	/* Allocate space for arrays of bin information */

	count[0] = c->n_bin;
	stmp = gmt_M_memory (GMT, NULL, c->n_bin, short);

	if ((err = nc_get_vara_short (c->cdfid, c->bin_nseg_id, start, count, stmp))) {
		gmt_M_free (GMT, stmp);
		gmt_br_cleanup (GMT, c);
		return (err);
	}
	c->bin_nseg = gmt_M_memory (GMT, NULL, nb, short);
	for (i = 0; i < c->nb; i++) c->bin_nseg[i] = stmp[c->bins[i]];
	gmt_M_free (GMT, stmp);

	itmp = gmt_M_memory (GMT, NULL, c->n_bin, int);
	if ((err = nc_get_vara_int (c->cdfid, c->bin_firstseg_id, start, count, itmp))) {
		gmt_M_free (GMT, itmp);
		gmt_br_cleanup (GMT, c);
		return (err);
	}
	c->bin_firstseg	= gmt_M_memory (GMT, NULL, nb, int);
	for (i = 0; i < c->nb; i++) c->bin_firstseg[i] = itmp[c->bins[i]];

	gmt_M_free (GMT, itmp);

	return (0);
}

int gmt_get_br_bin (struct GMT_CTRL *GMT, unsigned int b, struct GMT_BR *c, unsigned int *level, unsigned int n_levels) {
/* b: index number into c->bins */
/* level: Levels of features to extract */
/* n_levels: # of such levels. 0 means use all levels */
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

	seg_n = gmt_M_memory (GMT, NULL, c->bin_nseg[b], short);
	seg_level = gmt_M_memory (GMT, NULL, c->bin_nseg[b], short);
	seg_start = gmt_M_memory (GMT, NULL, c->bin_nseg[b], int);

	if ((err = nc_get_vara_short (c->cdfid, c->seg_n_id, start, count, seg_n))) {
		gmt_M_free (GMT, seg_n);	gmt_M_free (GMT, seg_level);	gmt_M_free (GMT, seg_start);
		return err;
	}
	if ((err = nc_get_vara_short (c->cdfid, c->seg_level_id, start, count, seg_level))) {
		gmt_M_free (GMT, seg_n);	gmt_M_free (GMT, seg_level);	gmt_M_free (GMT, seg_start);
		return err;
	}
	if ((err = nc_get_vara_int (c->cdfid, c->seg_start_id, start, count, seg_start))) {
		gmt_M_free (GMT, seg_n);	gmt_M_free (GMT, seg_level);	gmt_M_free (GMT, seg_start);
		return err;
	}

	c->seg = NULL;
	for (s = i = 0; i < c->ns; i++) {
		if (n_levels == 0)
			skip = false;
		else {
			for (k = 0, skip = true; skip && k < n_levels; k++)
				if ((s_level = (short)level[k]) == seg_level[i]) skip = false;
		}
		if (skip) continue;
		if (!c->seg) c->seg = gmt_M_memory (GMT, NULL, c->ns, struct GMT_BR_SEGMENT);
		c->seg[s].n = seg_n[i];
		c->seg[s].level = seg_level[i];
		c->seg[s].dx = gmt_M_memory (GMT, NULL, c->seg[s].n, short);
		c->seg[s].dy = gmt_M_memory (GMT, NULL, c->seg[s].n, short);
		start[0] = seg_start[i];
		count[0] = c->seg[s].n;
		if ((err = nc_get_vara_short (c->cdfid, c->pt_dx_id, start, count, c->seg[s].dx)) ||
			(err = nc_get_vara_short (c->cdfid, c->pt_dy_id, start, count, c->seg[s].dy))) {
				gmt_free_br (GMT, c);
				gmt_M_free (GMT, seg_n);
				gmt_M_free (GMT, seg_level);
				gmt_M_free (GMT, seg_start);
				return err;
			}

		s++;
	}

	c->ns = s;

	gmt_M_free (GMT, seg_n);
	gmt_M_free (GMT, seg_level);
	gmt_M_free (GMT, seg_start);

	return (GMT_NOERROR);
}

int gmt_assemble_shore (struct GMT_CTRL *GMT, struct GMT_SHORE *c, int dir, bool assemble, double west, double east, struct GMT_GSHHS_POL **pol) {
/* assemble: true if polygons is needed, false if we just want to draw or dump outlines.  Note: if true then the resulting
 * polygons are only contained within each tile for fill purposes; we are not reconstructing Australia, etc.
 * edge: Edge test for shifting of longitudes to avoid wraps.
 */
	struct GMT_GSHHS_POL *p = NULL;
	int start_side, next_side, id, wet_or_dry, use_this_level, high_seg_level = GSHHS_MAX_LEVEL, k;
	int cid, nid, add, first_pos, entry_pos, n, low_level, high_level, fid, nseg_at_level[GSHHS_MAX_LEVEL+1];
	bool completely_inside, more, skip = false;
	unsigned int P = 0, ku;
	size_t n_alloc, p_alloc;
	double *xtmp = NULL, *ytmp = NULL, plon, plat;

	if (!assemble) {	/* Easy, just need to scale all segments to degrees and return */

		p = gmt_M_memory (GMT, NULL, c->ns, struct GMT_GSHHS_POL);

		for (id = P = 0; id < c->ns; id++) {
			p[P].lon = gmt_M_memory (GMT, NULL, c->seg[id].n, double);
			p[P].lat = gmt_M_memory (GMT, NULL, c->seg[id].n, double);
			p[P].n = shore_copy_to_shore_path (p[P].lon, p[P].lat, c, id);
			if (c->ant_special) {	/* Discard any pieces south of 60S */
				if (c->ant_mode & GSHHS_ANTARCTICA_SKIP) {
					for (k = 0, skip = true; skip && k < p[P].n; k++) if (p[P].lat[k] > -60.0) skip = false;
				}
				else if (c->ant_mode & GSHHS_ANTARCTICA_SKIP_INV) {
					for (k = 0, skip = true; skip && k < p[P].n; k++) if (p[P].lat[k] < -60.0) skip = false;
				}
			}
			if (skip) {
				gmt_M_free (GMT, p[P].lon);
				gmt_M_free (GMT, p[P].lat);
			}
			else {
				p[P].level = c->seg[id].level;
				p[P].fid = c->seg[id].fid;
				p[P].interior = false;
				shore_path_shift2 (p[P].lon, p[P].n, west, east, c->leftmost_bin);
				P++;
			}
		}
		if (P < (unsigned int)c->ns) p = gmt_M_memory (GMT, p, P, struct GMT_GSHHS_POL);	/* Trim memory */
		*pol = p;
		return (P);
	}

	/* Check the consistency of node levels in case some features have been dropped */

	gmt_M_memset (nseg_at_level, GSHHS_MAX_LEVEL + 1, int);
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

	shore_prepare_sides (GMT, c, dir);	/* Initialize the book-keeping for how segments enters each of the four sides */

	/* Allocate 1 or more polygon structures */
	p_alloc = (c->ns == 0) ? 1 : GMT_SMALL_CHUNK;
	p = gmt_M_memory (GMT, NULL, p_alloc, struct GMT_GSHHS_POL);

	if (completely_inside && use_this_level) {	/* Must include path of this bin's outline as our first polygon, e.g., there may be no segments here but we are in the middle of a continent (or lake) */
		p[0].n = (int)gmt_graticule_path (GMT, &p[0].lon, &p[0].lat, dir, true, c->lon_corner[3], c->lon_corner[1], c->lat_corner[0], c->lat_corner[2]);
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

		start_side = 0;	/* We begin looking for segments entering along the south border of the bin, but shore_get_first_entry will determine what start_side really is */
		id = shore_get_first_entry (c, dir, &start_side);	/* This is the first segment to enter (measured from the west) and we return its ID and side via start_side */
		next_side = c->seg[id].exit;	/* The segment will then exit on possibly another side or the same side */

		n_alloc = c->seg[id].n;		/* Need this much space to hold the segment */
		fid = c->seg[id].fid;		/* Fill id (same as level expect for riverlakes which is 5) */
		/* Allocate space for our new polygon */
		p[P].lon = gmt_M_memory (GMT, NULL, n_alloc, double);
		p[P].lat = gmt_M_memory (GMT, NULL, n_alloc, double);
		n = shore_copy_to_shore_path (p[P].lon, p[P].lat, c, id);			/* Creates a lon-lat path from the segment */
		if ((int)c->seg[id].level < low_level) low_level = c->seg[id].level;	/* Update the lowest level involved */
		more = true;	/* Until we are done with all segments */
		first_pos = shore_get_position (start_side, c->seg[id].dx[0], c->seg[id].dy[0]);	/* This is the relative starting position (0-65535) on the start side for current segment */
		/* Remember, the segments have been sorted along each side according to entry position */
		while (more) {	/* Unless we run out or close the polygon we need to add more segments */

			id = shore_get_next_entry (c, dir, next_side, id);	/* Find the ID of the next segment along this side, OR the corner if no segments remain */

			if (id < 0) {	/* Found a corner */
				cid = id + 4;	/* ID of the corner */
				nid = (dir == 1) ? (cid + 1) % 4 : cid;	/* Next corner [I think] */
				if ((add = (int)gmtlib_map_path (GMT, p[P].lon[n-1], p[P].lat[n-1], c->lon_corner[cid], c->lat_corner[cid], &xtmp, &ytmp)) != 0) {
					/* Add the bin-border segment from last point in the growing polygon to the specified corner */
					n_alloc += add;
					p[P].lon = gmt_M_memory (GMT, p[P].lon, n_alloc, double);
					p[P].lat = gmt_M_memory (GMT, p[P].lat, n_alloc, double);
					gmt_M_memcpy (&p[P].lon[n], xtmp, add, double);
					gmt_M_memcpy (&p[P].lat[n], ytmp, add, double);
					n += add;
				}
				next_side = ((id + 4) + dir + 4) % 4;	/* This will go to the next side either CCW or CW, depending on dir */
				if ((int)c->node_level[nid] < low_level) low_level = c->node_level[nid];	/* Update lowest level involved */
			}
			else {	/* Found a segment to add to our polygon */
				shore_to_degree (c, c->seg[id].dx[0], c->seg[id].dy[0], &plon, &plat);	/* Get lon,lat of start of segment */
				if ((add = (int)gmtlib_map_path (GMT, p[P].lon[n-1], p[P].lat[n-1], plon, plat, &xtmp, &ytmp)) != 0) {
					/* Connect the last point in the growing polygon with the starting point of this next segment */
					n_alloc += add;
					p[P].lon = gmt_M_memory (GMT, p[P].lon, n_alloc, double);
					p[P].lat = gmt_M_memory (GMT, p[P].lat, n_alloc, double);
					gmt_M_memcpy (&p[P].lon[n], xtmp, add, double);
					gmt_M_memcpy (&p[P].lat[n], ytmp, add, double);
					n += add;
				}
				entry_pos = shore_get_position (next_side, c->seg[id].dx[0], c->seg[id].dy[0]);	/* Position on the next side */
				if (next_side == start_side && entry_pos == first_pos)	/* We have closed the polygon; done */
					more = false;
				else {	/* Add the segment to our growing polygon */
					n_alloc += c->seg[id].n;
					p[P].lon = gmt_M_memory (GMT, p[P].lon, n_alloc, double);
					p[P].lat = gmt_M_memory (GMT, p[P].lat, n_alloc, double);
					n += shore_copy_to_shore_path (&p[P].lon[n], &p[P].lat[n], c, id);
					next_side = c->seg[id].exit;	/* Update which side we are on after adding the segment */
					if ((int)c->seg[id].level < low_level) low_level = c->seg[id].level;	/* Update lowest level involved */
				}
			}
			if (add) {	/* Free temporary variables if used */
				gmt_M_free (GMT, xtmp);
				gmt_M_free (GMT, ytmp);
			}
		}
		if (c->ant_special) {	/* Discard any pieces south of 60S */
			if (c->ant_mode & GSHHS_ANTARCTICA_SKIP) {
				for (k = 0, skip = true; skip && k < n; k++) if (p[P].lat[k] > -60.0) skip = false;
			}
			else if (c->ant_mode & GSHHS_ANTARCTICA_SKIP_INV) {
				for (k = 0, skip = true; skip && k < n; k++) if (p[P].lat[k] < -60.0) skip = false;
			}
		}
		if (skip) {
			gmt_M_free (GMT, p[P].lon);
			gmt_M_free (GMT, p[P].lat);
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
				p = gmt_M_memory (GMT, p, p_alloc, struct GMT_GSHHS_POL);
				gmt_M_memset (&(p[old_p_alloc]), p_alloc - old_p_alloc, struct GMT_GSHHS_POL);	/* Set to NULL/0 */
			}
		}
		/* Then we go back to top of loop and if there are more segments we start all over with a new polygon */
	}

	/* Then add all interior polygons, if any.  These just needs to be converted to lon,lat, have their level set, and added to the list of polygons */

	for (id = 0; id < c->ns; id++) {
		if (c->seg[id].entry < 4) continue;
		n_alloc = c->seg[id].n;
		p[P].lon = gmt_M_memory (GMT, NULL, n_alloc, double);
		p[P].lat = gmt_M_memory (GMT, NULL, n_alloc, double);
		p[P].n = shore_copy_to_shore_path (p[P].lon, p[P].lat, c, id);
		if (c->ant_special) {	/* Discard any pieces south of 60S */
			if (c->ant_mode & GSHHS_ANTARCTICA_SKIP) {
				for (k = 0, skip = true; skip && k < p[P].n; k++) if (p[P].lat[k] > -60.0) skip = false;
			}
			else if (c->ant_mode & GSHHS_ANTARCTICA_SKIP_INV) {
				for (k = 0, skip = true; skip && k < p[P].n; k++) if (p[P].lat[k] < -60.0) skip = false;
			}
		}
		if (skip) {
			gmt_M_free (GMT, p[P].lon);
			gmt_M_free (GMT, p[P].lat);
		}
		else {
			p[P].interior = true;
			p[P].level = c->seg[id].level;
			p[P].fid = c->seg[id].fid;
			P++;
			if (P == p_alloc) {
				size_t old_p_alloc = p_alloc;
				p_alloc <<= 1;
				p = gmt_M_memory (GMT, p, p_alloc, struct GMT_GSHHS_POL);
				gmt_M_memset (&(p[old_p_alloc]), p_alloc - old_p_alloc, struct GMT_GSHHS_POL);	/* Set to NULL/0 */
			}
		}
	}

	shore_done_sides (GMT, c);	/* Free array of side structures */

	if (c->ns > 0) p = gmt_M_memory (GMT, p, P, struct GMT_GSHHS_POL);	/* Trim memory */

	for (ku = 0; ku < P; ku++) shore_path_shift2 (p[ku].lon, p[ku].n, west, east, c->leftmost_bin);	/* Deal with possible longitude -/+360 issues */

	*pol = p;
	return (P);	/* Return list of polygons found */
}

struct GMT_DATASET * gmt_get_gshhg_lines (struct GMT_CTRL *GMT, double wesn[], char res, struct GMT_SHORE_SELECT *A) {
	/* Return a dataset with GSHHS lines (not polygons) */

	char *shore_resolution[5] = {"full", "high", "intermediate", "low", "crude"};
	unsigned int base = gmt_set_resolution (GMT, &res, 'D');
	int ind, err, np = 0, k;
	size_t n_alloc = GMT_CHUNK;
	uint64_t tbl = 0, seg, n_seg = 0;
	double west_border, east_border;
	struct GMT_SHORE c;
	struct GMT_GSHHS_POL *p = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;

	if (gmt_init_shore (GMT, res, &c, wesn, A)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "%s resolution shoreline data base not installed\n", shore_resolution[base]);
		return (NULL);
	}
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Extract data from GSHHG version %s\n%s\n%s\n", c.version, c.title, c.source);
	west_border = floor (wesn[XLO] / c.bsize) * c.bsize;
	east_border =  ceil (wesn[XHI] / c.bsize) * c.bsize;

	D = gmtlib_create_dataset (GMT, 0U, 0U, 0U, 2U, GMT_IS_LINE, 0, true);	/* 2 cols but no tables yet */
	D->table = gmt_M_memory (GMT, NULL, c.nb, struct GMT_DATATABLE *);

	for (ind = 0; ind < c.nb; ind++) {	/* Loop over necessary bins only */

		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Reading GSHHS segments from bin # %5ld\r", c.bins[ind]);
		if ((err = gmt_get_shore_bin (GMT, ind, &c)) != 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "%s [%s resolution shoreline]\n", GMT_strerror(err), shore_resolution[base]);
			return (NULL);
		}
		n_seg = 0;
		if (c.ns > 0) {	/* Assemble one or more encoded segments into lon/lat lines */
			if ((np = gmt_assemble_shore (GMT, &c, 1, false, west_border, east_border, &p)) == 0) continue;
			for (k = 0; k < np; k++) if (p[k].n) n_seg++;	/* Count number of segments needed */
		}
		if (n_seg) {	/* We have a known number of line segments in this bin; this constitutes this table */
			if ((D->table[tbl] = gmt_create_table (GMT, 0U, 0U, 2U, 0U, false)) == NULL) return (NULL);
			D->table[tbl]->segment = gmt_M_memory (GMT, NULL, n_seg, struct GMT_DATASEGMENT *);
			D->table[tbl]->n_segments = n_seg;
			for (seg = k = 0; k < np; k++) {	/* For each line segment from GSHHS */
				if (p[k].n == 0) continue;	/* One of the ones to skip anyway */
				/* Allocate segment to hold this line segment and memcpy over the coordinates */
				S = GMT_Alloc_Segment (GMT->parent, GMT_NO_STRINGS, p[k].n, 2U, NULL, NULL);
				gmt_M_memcpy (S->data[GMT_X], p[k].lon, S->n_rows, double);
				gmt_M_memcpy (S->data[GMT_Y], p[k].lat, S->n_rows, double);
				D->table[tbl]->segment[seg++] = S;	/* Hook onto dataset structure */
				D->table[tbl]->n_records += S->n_rows;	/* Add up records in this table */
			}
			gmt_free_shore_polygons (GMT, p, np);
			gmt_M_free (GMT, p);
			D->n_segments += D->table[tbl]->n_segments;	/* Sum up total number of segments across the data set */
			D->n_records  += D->table[tbl]->n_records;	/* Sum up total number of records across the data set */
			gmt_set_column (GMT, GMT_IN, GMT_X, GMT_IS_FLOAT);	/* Avoid longitude adjustments by next function: longitudes are guaranteed to be correct; rounding errors only messes things up */
			gmt_set_tbl_minmax (GMT, GMT_IS_LINE, D->table[tbl++]);	/* Determine min/max extent for all segments and the table */
			gmt_set_column (GMT, GMT_IN, GMT_X, GMT_IS_LON);	/* Reset X column to be longitudes */
		}
		gmt_free_shore (GMT, &c);	/* Done with this GSHHS bin */
	}
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Reading GSHHS segments from bin # %5ld\n", c.bins[c.nb-1]);
	if (tbl < n_alloc) D->table = gmt_M_memory (GMT, D->table, tbl, struct GMT_DATATABLE *);
	D->n_tables = tbl;

	gmt_shore_cleanup (GMT, &c);	/* Done with the GSHHS database */

	return (D);
}

int gmt_assemble_br (struct GMT_CTRL *GMT, struct GMT_BR *c, bool shift, double edge, struct GMT_GSHHS_POL **pol) {
/* shift: true if longitudes may have to be shifted */
/* edge: Edge test for shifting */
	struct GMT_GSHHS_POL *p = NULL;
	int id;

	p = gmt_M_memory (GMT, NULL, c->ns, struct GMT_GSHHS_POL);

	for (id = 0; id < c->ns; id++) {
		p[id].lon = gmt_M_memory (GMT, NULL, c->seg[id].n, double);
		p[id].lat = gmt_M_memory (GMT, NULL, c->seg[id].n, double);
		p[id].n = shore_copy_to_br_path (p[id].lon, p[id].lat, c, id);
		p[id].level = c->seg[id].level;
		if (shift) shore_path_shift (p[id].lon, p[id].n, edge);
	}

	*pol = p;
	return (c->ns);
}

void gmt_free_shore (struct GMT_CTRL *GMT, struct GMT_SHORE *c) {
	/* Removes allocated variables for this block only */
	int i;

	for (i = 0; i < c->ns; i++) {
		gmt_M_free (GMT, c->seg[i].dx);
		gmt_M_free (GMT, c->seg[i].dy);
	}
	if (c->ns) gmt_M_free (GMT, c->seg);
}

void gmt_free_br (struct GMT_CTRL *GMT, struct GMT_BR *c) {
	/* Removes allocated variables for this block only */
	int i;

	for (i = 0; i < c->ns; i++) {
		gmt_M_free (GMT, c->seg[i].dx);
		gmt_M_free (GMT, c->seg[i].dy);
	}
	if (c->ns) gmt_M_free (GMT, c->seg);

}

void gmt_shore_cleanup (struct GMT_CTRL *GMT, struct GMT_SHORE *c) {
	gmt_M_free (GMT, c->bins);
	gmt_M_free (GMT, c->bin_info);
	gmt_M_free (GMT, c->bin_nseg);
	gmt_M_free (GMT, c->bin_firstseg);
	gmt_M_free (GMT, c->GSHHS_area);
	gmt_M_free (GMT, c->GSHHS_area_fraction);
	if (c->min_area > 0.0) gmt_M_free (GMT, c->GSHHS_node);
	gmt_M_free (GMT, c->GSHHS_parent);
	nc_close (c->cdfid);
}

void gmt_br_cleanup (struct GMT_CTRL *GMT, struct GMT_BR *c) {
	gmt_M_free (GMT, c->bins);
	gmt_M_free (GMT, c->bin_nseg);
	gmt_M_free (GMT, c->bin_firstseg);
	nc_close (c->cdfid);
}

int gmt_prep_shore_polygons (struct GMT_CTRL *GMT, struct GMT_GSHHS_POL **p_old, unsigned int np, bool sample, double step, int anti_bin) {
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

		if (sample) p[k].n = (int)gmt_fix_up_path (GMT, &p[k].lon, &p[k].lat, p[k].n, step, 0);

		/* Clip polygon against map boundary if necessary and return plot x,y in inches */

		if ((n = (unsigned int)gmt_clip_to_map (GMT, p[k].lon, p[k].lat, p[k].n, &xtmp, &ytmp)) == 0) {	/* Completely outside */
			p[k].n = 0;	/* Note the memory in lon, lat not freed yet */
			continue;
		}

		/* Must check if polygon must be split and partially plotted at both edges of map */

		if ((*GMT->current.map.will_it_wrap) (GMT, xtmp, ytmp, n, &start)) {	/* Polygon does indeed wrap */

			/* First truncate against left border */

			GMT->current.plot.n = gmt_map_truncate (GMT, xtmp, ytmp, n, start, -1);
			n_use = (unsigned int)gmt_compact_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, false, 0);
			close = gmt_polygon_is_open (GMT, GMT->current.plot.x, GMT->current.plot.y, n_use);
			n_alloc = (close) ? n_use + 1 : n_use;
			p[k].lon = gmt_M_memory (GMT, p[k].lon, n_alloc, double);
			p[k].lat = gmt_M_memory (GMT, p[k].lat, n_alloc, double);
			gmt_M_memcpy (p[k].lon, GMT->current.plot.x, n_use, double);
			gmt_M_memcpy (p[k].lat, GMT->current.plot.y, n_use, double);
			if (close) {	/* Must explicitly close the polygon */
				p[k].lon[n_use] = p[k].lon[0];
				p[k].lat[n_use] = p[k].lat[0];
			}
			p[k].n = (int)n_alloc;

			/* Then truncate against right border */

			GMT->current.plot.n = gmt_map_truncate (GMT, xtmp, ytmp, n, start, +1);
			n_use = (unsigned int)gmt_compact_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, false, 0);
			p = gmt_M_memory (GMT, p, np_new + 1, struct GMT_GSHHS_POL);
			close = gmt_polygon_is_open (GMT, GMT->current.plot.x, GMT->current.plot.y, n_use);
			n_alloc = (close) ? n_use + 1 : n_use;
			p[np_new].lon = gmt_M_memory (GMT, NULL, n_alloc, double);
			p[np_new].lat = gmt_M_memory (GMT, NULL, n_alloc, double);
			gmt_M_memcpy (p[np_new].lon, GMT->current.plot.x, n_use, double);
			gmt_M_memcpy (p[np_new].lat, GMT->current.plot.y, n_use, double);
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
			n_use = (unsigned int)gmt_compact_line (GMT, xtmp, ytmp, n, false, 0);
			if (anti_bin > 0 && step == 0.0) {	/* Must warn for donut effect */
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Antipodal bin # %d not filled!\n", anti_bin);
				gmt_M_free (GMT, xtmp);
				gmt_M_free (GMT, ytmp);
				continue;
			}
			else {
				close = gmt_polygon_is_open (GMT, xtmp, ytmp, n_use);
				n_alloc = (close) ? n_use + 1 : n_use;
				p[k].lon = gmt_M_memory (GMT, p[k].lon, n_alloc, double);
				p[k].lat = gmt_M_memory (GMT, p[k].lat, n_alloc, double);
				gmt_M_memcpy (p[k].lon, xtmp, n_use, double);
				gmt_M_memcpy (p[k].lat, ytmp, n_use, double);
				if (close) {	/* Must explicitly close the polygon */
					p[k].lon[n_use] = p[k].lon[0];
					p[k].lat[n_use] = p[k].lat[0];
				}
				p[k].n = (int)n_alloc;
			}
		}

		gmt_M_free (GMT, xtmp);
		gmt_M_free (GMT, ytmp);
	}

	*p_old = p;

	return (np_new);
}

void gmt_free_shore_polygons (struct GMT_CTRL *GMT, struct GMT_GSHHS_POL *p, unsigned int n) {
	/* Free the given list of polygon coordinates */
	unsigned int k;
	for (k = 0; k < n; k++) {
		gmt_M_free (GMT, p[k].lon);
		gmt_M_free (GMT, p[k].lat);
	}
}

int gmt_shore_level_at_point (struct GMT_CTRL *GMT, struct GMT_SHORE *c, int inside, double lon, double lat) {
	/* Determine the highest hierarchical level of the GSHHG polygon enclosing the given (lon,lat) point.
	 * I.e., if the point is in the ocean we return 0, 1 on land, 2 in lake, 3 in island-in-lake, and 4 if in that pond.
	 * Pass inside = GMT_ONEDGE (1) or GMT_INSIDE (2) to determine if "inside a polygon" should include landing
	 * exactly on the edge or not.
	 * It is assumed that -Jx1d is in effect.
	 * We return -1 if the shore-machinery domain is exceeded.
	 * Call with inside == -1 to free internal memory [no searching for level is done].
	 * We assume gmt_init_shore (GMT, Ctrl->D.set, &c, GMT->common.R.wesn, &Ctrl->A.info) has been
	 * already called so we may simply select bins and do our thing.  It is up to the calling program
	 * to clean up the shore stuff with gmt_free_shore etc. */
	static int last_bin = INT_MAX;	/* Initially not set */
	static int np[2] = {0, 0};	/* Initially not set */
	static struct GMT_GSHHS_POL *p[2] = {NULL, NULL};	/* Initially not set */
	int this_point_level, brow, bin, ind, err, wd[2] = {1, -1};
	unsigned int col, id, side, uinside = 0U;
	uint64_t k, i;
	double xx = lon, yy, xmin, xmax, ymin, ymax, west_border, east_border;

	if (inside < 0) {	/* Final call to clean memory */
		for (id = 0; id < 2; id++) {
			gmt_free_shore_polygons (GMT, p[id], np[id]);
			if (np[id]) gmt_M_free (GMT, p[id]);
		}
		gmt_free_shore (GMT, c);	/* Free previously allocated arrays */
		last_bin = INT_MAX;		/* In case process lives on in API */
		return GMT_OK;
	}
	else
		uinside = (unsigned int)inside;
	west_border = floor (GMT->common.R.wesn[XLO] / c->bsize) * c->bsize;
	east_border = ceil (GMT->common.R.wesn[XHI]  / c->bsize) * c->bsize;
	while (xx < 0.0) xx += 360.0;
	brow = irint (floor ((90.0 - lat) / c->bsize));
	if (brow >= c->bin_ny) brow = c->bin_ny - 1;	/* Presumably only kicks in for south pole */
	col = urint (floor (xx / c->bsize));
	bin = brow * c->bin_nx + col;
	if (bin != last_bin) {	/* Do this upon entering new bin */
		ind = 0;
		while (ind < c->nb && c->bins[ind] != bin) ind++;	/* Set ind to right bin */
		if (ind == c->nb) return -1;			/* Bin not among the chosen ones */
		last_bin = bin;
		gmt_free_shore (GMT, c);	/* Free previously allocated arrays */
		if ((err = gmt_get_shore_bin (GMT, ind, c))) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "%s [gmt_shore_level_at_point]\n", GMT_strerror(err));
			return (GMT_RUNTIME_ERROR);
		}

		/* Must use polygons.  Go in both directions to cover both land and sea */
		for (id = 0; id < 2; id++) {
			gmt_free_shore_polygons (GMT, p[id], np[id]);
			if (np[id]) gmt_M_free (GMT, p[id]);
			np[id] = gmt_assemble_shore (GMT, c, wd[id], true, west_border, east_border, &p[id]);
			np[id] = gmt_prep_shore_polygons (GMT, &p[id], np[id], false, 0.01, -1);
		}
	}

	if (c->ns == 0) {	/* No segment lines go through this bin, check node levels */
		this_point_level = MIN (MIN (c->node_level[0], c->node_level[1]) , MIN (c->node_level[2], c->node_level[3]));
	}
	else {	/* Must scan the polygons */
		this_point_level = 0;
		gmt_geo_to_xy (GMT, lon, lat, &xx, &yy);
		for (id = 0; id < 2; id++) {	/* For both directions */

			for (k = 0; k < (uint64_t)np[id]; k++) {	/* For all closed polygons */

				if (p[id][k].n == 0) continue;

				/* Find min/max of polygon [Note: longitudes are jump free for this bin] */

				xmin = xmax = p[id][k].lon[0];
				ymin = ymax = p[id][k].lat[0];

				for (i = 1; i < (uint64_t)p[id][k].n; i++) {
					if (p[id][k].lon[i] < xmin) xmin = p[id][k].lon[i];
					if (p[id][k].lon[i] > xmax) xmax = p[id][k].lon[i];
					if (p[id][k].lat[i] < ymin) ymin = p[id][k].lat[i];
					if (p[id][k].lat[i] > ymax) ymax = p[id][k].lat[i];
				}
				/* Check if we are outside this polygon */
				if (yy < ymin || yy > ymax) continue;
				if (xx < xmin || xx > xmax) continue;

				/* Must compare with polygon; holes are handled explicitly via the levels */

				if ((side = gmt_non_zero_winding (GMT, xx, yy, p[id][k].lon, p[id][k].lat, p[id][k].n)) < uinside) continue;	/* Outside polygon */

				/* Here, point is inside, we must assign value */

				if (p[id][k].level > this_point_level) this_point_level = p[id][k].level;
			}
		}
	}

	return this_point_level;
}
