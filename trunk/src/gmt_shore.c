/*--------------------------------------------------------------------
 *	$Id: gmt_shore.c,v 1.68 2011-03-03 21:02:50 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

#include "gmt.h"

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
 * GMT_prep_polygons :		Wraps polygons if necessary and prepares them for use
 * GMT_assemble_br :		Creates lines from border or river segments
 * GMT_free_shore :		Frees up memory used by shorelines for this bin
 * GMT_free_br :		Frees up memory used by shorelines for this bin
 * GMT_shore_cleanup :		Frees up main shoreline structure memory
 * GMT_br_cleanup :		Frees up main river/border structure memory
 *
 * Author:	Paul Wessel
 * Date:	13-JUN-2000
 * Version:	4.1.x
 *
 */

#define RIVERLAKE	5	/* Fill array id for riverlakes */

#define get_level(arg) (((arg) >> 6) & 7)		/* Extract level from bit mask */

GMT_LONG GMT_copy_to_shore_path (double *lon, double *lat, struct GMT_SHORE *s, GMT_LONG id);
GMT_LONG GMT_shore_get_first_entry (struct GMT_SHORE *c, GMT_LONG dir, GMT_LONG *side);
GMT_LONG GMT_shore_get_position (GMT_LONG side, short int x, short int y);
GMT_LONG GMT_shore_get_next_entry (struct GMT_SHORE *c, GMT_LONG dir, GMT_LONG side, GMT_LONG id);
GMT_LONG GMT_copy_to_br_path (double *lon, double *lat, struct GMT_BR *s, GMT_LONG id);
void GMT_shore_to_degree (struct GMT_SHORE *c, short int dx, short int dy, double *lon, double *lat);
void GMT_shore_pau_sides (struct GMT_SHORE *c);
void GMT_shore_path_shift (double *lon, double *lat, GMT_LONG n, double edge);
void GMT_shore_path_shift2 (double *lon, double *lat, GMT_LONG n, double west, double east, GMT_LONG leftmost);
void GMT_br_to_degree (struct GMT_BR *c, short int dx, short int dy, double *lon, double *lat);
void shore_prepare_sides(struct GMT_SHORE *c, GMT_LONG dir);
int GMT_shore_asc_sort (const void *a, const void *b);
int GMT_shore_desc_sort(const void *a, const void *b);
char *GMT_shore_getpathname (char *name, char *path);
void GMT_shore_check (GMT_LONG ok[5]);
GMT_LONG GMT_res_to_int (char res);

void GMT_set_levels (char *info, struct GMT_SHORE_SELECT *I)
{	/* Decode GMT's -A option for coastline levels */
	int n;
	char *p = NULL;
	if (strstr (info, "+l"))  I->flag = GMT_NO_RIVERLAKES;
	if (strstr (info, "+r"))  I->flag = GMT_NO_LAKES;
	if ((p = strstr (info, "+p"))) {	/* Requested percentage limit on small features */
		I->fraction = irint (1e6 * 0.01 * atoi (&p[2]));	/* Convert to integer microfraction */
	}
	n = sscanf (info, "%lf/%d/%d", &I->area, &I->low, &I->high);
	if (n == 0) {
		fprintf (stderr, "%s: Error in -A: No area given\n", GMT_program);
		GMT_exit (EXIT_FAILURE);
	}
	if (n == 1) I->low = 0, I->high = GMT_MAX_GSHHS_LEVEL;
}

GMT_LONG GMT_set_resolution (char *res, char opt)
{
	/* Decodes the -D<res> option and returns the base integer value */

	GMT_LONG base;

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
			fprintf (stderr, "%s: GMT SYNTAX ERROR -%c option:  Unknown modifier %c [Defaults to -%cl]\n", GMT_program, opt, *res, opt);
			base = 3;
			*res = 'l';
			break;
	}

	return (base);
}

void GMT_shore_check (GMT_LONG ok[5])
/* Sets ok to TRUE for those resolutions available in share for
 * resolution (f, h, i, l, c) */
{
	GMT_LONG i, j, n_found;
	char stem[GMT_TEXT_LEN], path[BUFSIZ], *res = "clihf", *kind[3] = {"GSHHS", "river", "border"};
	
	for (i = 0; i < 5; i++) {	/* For each resolution... */
		ok[i] = FALSE;
		for (j = n_found = 0; j < 3; j++) {	/* For each data type... */
			sprintf (stem, "binned_%s_%c", kind[j], res[i]);
	        	if (!GMT_shore_getpathname (stem, path)) continue;	/* Failed to find file */
			n_found++;	/* Increment how many found so far for this resolution */
		}
		ok[i] = (n_found == 3);	/* Need all three sets to say this resolution is covered */
	}
}

GMT_LONG GMT_res_to_int (char res)
{	/* Turns a resolution letter into a 0-4 integer */
	GMT_LONG i, j;
	char *type = "clihf";
	
	for (i = -1, j = 0; i == -1 && j < 5; j++) if (res == type[j]) i = j;
	return (i);
}

char GMT_shore_adjust_res (char res) {	/* Returns the highest available resolution <= to specified resolution */
	GMT_LONG k, orig;
	GMT_LONG ok[5];	
	char *type = "clihf";
	(void)GMT_shore_check (ok);		/* See which resolutions we have */
	k = orig = GMT_res_to_int (res);	/* Get integer value of requested resolution */
	while (!ok[k] && k >= 0) k--;		/* Drop down one level to see if we have a lower resolution available */
	if (k >= 0 && k != orig) fprintf (stderr, "%s: Warning: Resolution %c not available, substituting resolution %c\n", GMT_program, res, type[k]);
	return ((k == -1) ? res : type[k]);	/* Return the chosen resolution */
}

GMT_LONG GMT_init_shore (char res, struct GMT_SHORE *c, double w, double e, double s, double n, struct GMT_SHORE_SELECT *info) {	/* res: Resolution (f, h, i, l, c */
	GMT_LONG i, nb, idiv, iw, ie, is, in, this_south, this_west, err, major, minor, release;
	short *stmp;
	int *itmp;
	size_t start[1], count[1];
	char stem[GMT_TEXT_LEN], path[BUFSIZ];
	
	sprintf (stem, "binned_GSHHS_%c", res);
	
	if (!GMT_shore_getpathname (stem, path)) return (GMT_GRDIO_FILE_NOT_FOUND);	/* Failed to find file */
	memset ((void *)c, 0, sizeof (struct GMT_SHORE));
		
	/* Open shoreline file */
	GMT_err_trap (nc_open (path, NC_NOWRITE, &c->cdfid));

	/* Get global attributes */
        if (nc_get_att_text (c->cdfid, NC_GLOBAL, "version", c->version) ||
                sscanf (c->version, "%" GMT_LL "d.%" GMT_LL "d.%" GMT_LL "d", &major, &minor, &release) < 3 ||
                major != 2 || minor < 1) {
			fprintf (stderr, "GSHHS: Version 2.1.0 or newer is needed to use coastlines with GMT %s\n", GMT_VERSION);
			fprintf (stderr, "GSHHS: CVS users must get the GSHHS%s tarballs from\n", GSHHS_VERSION);
			fprintf (stderr, "GSHHS: ftp://ftp.soest.hawaii.edu/pwessel/GSHHS%s_coast.tar.bz2\n", GSHHS_VERSION);
			fprintf (stderr, "GSHHS: ftp://ftp.soest.hawaii.edu/pwessel/GSHHS%s_high.tar.bz2\n", GSHHS_VERSION);
			fprintf (stderr, "GSHHS: ftp://ftp.soest.hawaii.edu/pwessel/GSHHS%s_full.tar.bz2\n", GSHHS_VERSION);
			fprintf (stderr, "GSHHS: or by running \"make get-gshhs-cvs\" from the top GMT directory.\n");
			GMT_exit (EXIT_FAILURE);
	}
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
        GMT_err_trap (nc_inq_varid (c->cdfid, "Ten_times_the_km_squared_area_of_polygons", &c->GSHHS_area_id));
        GMT_err_trap (nc_inq_varid (c->cdfid, "Id_of_first_point_in_a_segment", &c->seg_start_id));
        GMT_err_trap (nc_inq_varid (c->cdfid, "Relative_longitude_from_SW_corner_of_bin", &c->pt_dx_id));
        GMT_err_trap (nc_inq_varid (c->cdfid, "Relative_latitude_from_SW_corner_of_bin", &c->pt_dy_id));
        GMT_err_trap (nc_inq_varid (c->cdfid, "Micro_fraction_of_full_resolution_area", &c->GSHHS_areafrac_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_polygons_in_file", &c->n_poly_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "N_nodes_in_file", &c->n_node_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Id_of_parent_polygons", &c->GSHHS_parent_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Id_of_node_polygons", &c->GSHHS_node_id));
	GMT_err_trap (nc_inq_varid (c->cdfid, "Id_of_GSHHS_ID", &c->seg_GSHHS_ID_id));
	
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
	c->max_level = (info->low == info->high && info->high == 0) ? GMT_MAX_GSHHS_LEVEL : info->high;	/* Default to all if not set */
	c->flag = info->flag;

	c->scale = (c->bin_size / 60.0) / 65535.0;
	c->bsize = c->bin_size / 60.0;

	c->bins = (GMT_LONG *) GMT_memory (VNULL, (size_t)c->n_bin, sizeof (GMT_LONG), "GMT_init_shore");
	
	/* Round off area to nearest multiple of block-dimension */
		
	iw = (GMT_LONG)(floor (w / c->bsize) * c->bsize);
	ie =  (GMT_LONG)(ceil (e / c->bsize) * c->bsize);
	is = 90 - (GMT_LONG)(ceil ((90.0 - s) / c->bsize) * c->bsize);
	in = 90 - (GMT_LONG)(floor ((90.0 - n) / c->bsize) * c->bsize);
	idiv = irint (360.0 / c->bsize);	/* Number of blocks per latitude band */
	
	for (i = nb = 0; i < c->n_bin; i++) {	/* Find which bins are needed */
		this_south = 90 - (GMT_LONG)(c->bsize * ((i / idiv) + 1));
		if (this_south < is || this_south >= in) continue;
		this_west = (GMT_LONG)(c->bsize * (i % idiv)) - 360;
		while (this_west < iw) this_west += 360;
		if (this_west >= ie) continue;
		c->bins[nb] = i;
		nb++;
	}
	c->bins = (GMT_LONG *) GMT_memory ((void *)c->bins, (size_t)nb, sizeof (GMT_LONG), "GMT_init_shore");
	c->nb = nb;
	
	/* Get polygon variables if they are needed */

	GMT_err_trap (nc_get_var1_int (c->cdfid, c->n_poly_id, start, &c->n_poly));
	count[0] = c->n_poly;
	c->GSHHS_parent = (int *) GMT_memory (VNULL, (size_t)c->n_poly, sizeof (int), "GMT_init_shore");
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->GSHHS_parent_id, start, count, c->GSHHS_parent));
	c->GSHHS_area = (int *) GMT_memory (VNULL, (size_t)c->n_poly, sizeof (int), "GMT_init_shore");
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->GSHHS_area_id, start, count, c->GSHHS_area));
	c->GSHHS_area_fraction = (int *) GMT_memory (VNULL, (size_t)c->n_poly, sizeof (int), "GMT_init_shore");
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->GSHHS_areafrac_id, start, count, c->GSHHS_area_fraction));
	if (c->min_area > 0.0) {	/* Want to exclude small polygons so we need info about the node polygons */
	        GMT_err_trap (nc_get_var1_int (c->cdfid, c->n_node_id, start, &c->n_nodes));
		c->GSHHS_node = (int *) GMT_memory (VNULL, (size_t)c->n_nodes, sizeof (int), "GMT_init_shore");
		count[0] = c->n_nodes;
		GMT_err_trap (nc_get_vara_int (c->cdfid, c->GSHHS_node_id, start, count, c->GSHHS_node));
	}
	
	/* Get bin variables, then extract only those corresponding to the bins to use */

	/* Allocate space for arrays of bin information */
	
	c->bin_info     = (short *) GMT_memory (VNULL, (size_t)nb, sizeof (short), "GMT_init_shore");
	c->bin_nseg     = (short *) GMT_memory (VNULL, (size_t)nb, sizeof (short), "GMT_init_shore");
	c->bin_firstseg     = (int *) GMT_memory (VNULL, (size_t)nb, sizeof (int), "GMT_init_shore");
	
	count[0] = c->n_bin;
	stmp = (short *) GMT_memory (VNULL, (size_t)c->n_bin, sizeof (short), "GMT_init_shore");
	
	GMT_err_trap (nc_get_vara_short (c->cdfid, c->bin_info_id, start, count, stmp));
	for (i = 0; i < c->nb; i++) c->bin_info[i] = stmp[c->bins[i]];
	
        GMT_err_trap (nc_get_vara_short (c->cdfid, c->bin_nseg_id, start, count, stmp));
	for (i = 0; i < c->nb; i++) c->bin_nseg[i] = stmp[c->bins[i]];
	GMT_free ((void *)stmp);
	
	itmp = (int *) GMT_memory (VNULL, (size_t)c->n_bin, sizeof (int), "GMT_init_shore");
        GMT_err_trap (nc_get_vara_int (c->cdfid, c->bin_firstseg_id, start, count, itmp));
	for (i = 0; i < c->nb; i++) c->bin_firstseg[i] = itmp[c->bins[i]];
	
	GMT_free ((void *)itmp);
	
	return (GMT_NOERROR);
}

GMT_LONG GMT_get_shore_bin (GMT_LONG b, struct GMT_SHORE *c)
/* b: index number into c->bins */
/* min_area: Polygons with area less than this are ignored */
/* min_level: Polygons with lower levels are ignored */
/* max_level: Polygons with higher levels are ignored */
{
	size_t start[1], count[1];
	int *seg_info, *seg_start, *seg_ID;
	GMT_LONG *seg_skip = NULL;
	GMT_LONG s, i, k, err, cut_area, level, inc[4], ll_node, node, ID;
	double w, e, dx;
		
	c->node_level[0] = (unsigned char)MIN (((unsigned short)c->bin_info[b] >> 9) & 7, c->max_level);
	c->node_level[1] = (unsigned char)MIN (((unsigned short)c->bin_info[b] >> 6) & 7, c->max_level);
	c->node_level[2] = (unsigned char)MIN (((unsigned short)c->bin_info[b] >> 3) & 7, c->max_level);
	c->node_level[3] = (unsigned char)MIN ((unsigned short)c->bin_info[b] & 7, c->max_level);
	dx = c->bin_size / 60.0;
	c->lon_sw = (c->bins[b] % c->bin_nx) * dx;
	c->lat_sw = 90.0 - ((c->bins[b] / c->bin_nx) + 1) * dx;
	c->ns = 0;

	/* Determine if this bin is one of the bins at the left side of the map */
	
	w = c->lon_sw;
	while (w > project_info.w && GMT_world_map) w -= 360.0;
	e = w + dx;
	c->leftmost_bin = ((w <= project_info.w) && (e > project_info.w));

	if (c->bin_nseg[b] == 0) return (GMT_NOERROR);
	
	cut_area = irint (10.0 * c->min_area);
	ll_node = ((c->bins[b] / c->bin_nx) + 1) * (c->bin_nx + 1) + (c->bins[b] % c->bin_nx);		/* lower-left node in current bin */
	inc[0] = 0;	inc[1] = 1;	inc[2] = 1 - (c->bin_nx + 1);	inc[3] = -(c->bin_nx + 1);	/* Relative incs to other nodes */
	
	if (cut_area) {	/* May have to revise the node_level array if the polygon that determined the level is to be skipped */
		for (k = 0; k < 4; k++) {	/* Visit all four nodes defining this bin, going counter-clockwise from lower-left bin */
			node = ll_node + inc[k];	/* Current node index */
			ID = c->GSHHS_node[node];	/* GSHHS Id of the polygon that determined the level of the current node */
			while (c->node_level[k] && c->GSHHS_area[ID] < cut_area) {	/* Polygon must be skipped and node level reset */
				ID = c->GSHHS_parent[ID];	/* Pick the parent polygon since that is the next polygon up */
				c->node_level[k]--;		/* ...and drop down one level to that of the parent polygon */
			}	/* Keep doing this until the polygon containing the node is "too big to fail" or we are in the ocean */
		}
	}
	
	/* Here the node_level has been properly set but any polygons to be skipped have not been skipped yet; this happens below */
	
	start[0] = c->bin_firstseg[b];	/* Position of first segment in this bin */
	count[0] = c->bin_nseg[b];	/* Number of segments in this bin */
	
	seg_info = (int *) GMT_memory (VNULL, (size_t)c->bin_nseg[b], sizeof (int), "GMT_get_shore_bin");
	seg_start = (int *) GMT_memory (VNULL, (size_t)c->bin_nseg[b], sizeof (int), "GMT_get_shore_bin");
	seg_ID = (int *) GMT_memory (VNULL, (size_t)c->bin_nseg[b], sizeof (int), "GMT_get_shore_bin");

        GMT_err_trap (nc_get_vara_int (c->cdfid, c->seg_info_id, start, count, seg_info));
        GMT_err_trap (nc_get_vara_int (c->cdfid, c->seg_start_id, start, count, seg_start));
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->seg_GSHHS_ID_id, start, count, seg_ID));

	/* First tally how many useful segments */

	seg_skip = (GMT_LONG *) GMT_memory (VNULL, (size_t)c->bin_nseg[b], sizeof (GMT_LONG), "GMT_get_shore_bin");
	for (i = 0; i < c->bin_nseg[b]; i++) {
		seg_skip[i] = TRUE;	/* Reset later to FALSE if we pass all the tests to follow next */
		if (c->GSHHS_area_fraction[seg_ID[i]] < c->fraction) continue;	/* Area of this feature is too small relative to its original size */
		if (abs(c->GSHHS_area[seg_ID[i]]) < cut_area) continue;		/* Too small. NOTE: Use abs() since double-lined-river lakes have negative area */
		level = get_level (seg_info[i]);
		if (level < c->min_level) continue;
		if (level > c->max_level) continue;
		if (level == 2 && c->GSHHS_area[seg_ID[i]] < 0 && c->flag == GMT_NO_RIVERLAKES) continue;
		if (level == 2 && c->GSHHS_area[seg_ID[i]] > 0 && c->flag == GMT_NO_LAKES) continue;
		seg_skip[i] = FALSE;	/* OK, so this was needed afterall */
	}
	if (c->skip_feature) {	/* Must ensure that we skip all features inside a skipped riverlake/lake */
		int j, feature;
		if (c->flag == GMT_NO_LAKES && c->node_level[0] == c->node_level[1] && c->node_level[2] == c->node_level[3] && c->node_level[0] == c->node_level[3] && c->node_level[0] == 2) {	/* Bin is entirely inside a lake */
			for (i = 0; i < c->bin_nseg[b]; i++) seg_skip[i] = TRUE;	/* Must skip all segments in the lake */
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
							seg_skip[i] = TRUE;	/* We must therefore skip feature i as well */
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
		GMT_free ((void *) seg_skip);
		GMT_free ((void *) seg_info);	
		GMT_free ((void *) seg_start);
		GMT_free ((void *) seg_ID);
		return (GMT_NOERROR);
	}

	c->seg = (struct GMT_SHORE_SEGMENT *) GMT_memory (VNULL, (size_t)c->ns, sizeof (struct GMT_SHORE_SEGMENT), "GMT_get_shore_bin");

	for (s = 0; s < c->ns; s++) {
		c->seg[s].level = get_level (seg_info[s]);
		c->seg[s].n = (seg_info[s] >> 9);
		c->seg[s].entry = (seg_info[s] >> 3) & 7;
		c->seg[s].exit = seg_info[s] & 7;
		c->seg[s].fid = (c->GSHHS_area[seg_ID[s]] < 0) ? RIVERLAKE : c->seg[s].level;
		c->seg[s].dx = (short *) GMT_memory (VNULL, (size_t)c->seg[s].n, sizeof (short), "GMT_get_shore_bin");
		c->seg[s].dy = (short *) GMT_memory (VNULL, (size_t)c->seg[s].n, sizeof (short), "GMT_get_shore_bin");
		start[0] = seg_start[s];
		count[0] = c->seg[s].n;
		GMT_err_trap (nc_get_vara_short (c->cdfid, c->pt_dx_id, start, count, c->seg[s].dx));
                GMT_err_trap (nc_get_vara_short (c->cdfid, c->pt_dy_id, start, count, c->seg[s].dy));	
	}

	GMT_free ((void *) seg_skip);
	GMT_free ((void *) seg_info);	
	GMT_free ((void *) seg_start);	
	GMT_free ((void *) seg_ID);

	return (GMT_NOERROR);
}

GMT_LONG GMT_init_br (char which, char res, struct GMT_BR *c, double w, double e, double s, double n)
/* which: r(iver) or b(order) */
/* res: Resolution (f, h, i, l, c */
{
	GMT_LONG i, nb, idiv, iw, ie, is, in, this_south, this_west, err;
	short *stmp;
	int *itmp;
	size_t start[1], count[1];
	char stem[GMT_TEXT_LEN], path[BUFSIZ];
	
	if (which == 'r')
		sprintf (stem, "binned_river_%c", res);
	else
		sprintf (stem, "binned_border_%c", res);
	
        if (!GMT_shore_getpathname (stem, path)) return (-1);	/* Failed to find file */

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
	c->version[0] = '\0';
	(void) nc_get_att_text (c->cdfid, NC_GLOBAL, "version", c->version);

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

	c->bins = (GMT_LONG *) GMT_memory (VNULL, (size_t)c->n_bin, sizeof (GMT_LONG), "GMT_init_br");
	
	/* Round off area to nearest multiple of block-dimension */
		
	iw = (GMT_LONG)(floor (w / c->bsize) * c->bsize);
	ie =  (GMT_LONG)(ceil (e / c->bsize) * c->bsize);
	is = 90 - (GMT_LONG)(ceil ((90.0 - s) / c->bsize) * c->bsize);
	in = 90 - (GMT_LONG)(floor ((90.0 - n) / c->bsize) * c->bsize);
	idiv = irint (360.0 / c->bsize);	/* Number of blocks per latitude band */
	
	for (i = nb = 0; i < c->n_bin; i++) {	/* Find which bins are needed */
		this_south = 90 - (GMT_LONG)(c->bsize * ((i / idiv) + 1));
		if (this_south < is || this_south >= in) continue;
		this_west = (GMT_LONG)(c->bsize * (i % idiv)) - 360;
		while (this_west < iw) this_west += 360;
		if (this_west >= ie) continue;
		c->bins[nb] = i;
		nb++;
	}
	c->bins = (GMT_LONG *) GMT_memory ((void *)c->bins, (size_t)nb, sizeof (GMT_LONG), "GMT_init_br");
	c->nb = nb;
	
	/* Get bin variables, then extract only those corresponding to the bins to use */

	/* Allocate space for arrays of bin information */
	
	c->bin_nseg     = (short *) GMT_memory (VNULL, (size_t)nb, sizeof (short), "GMT_init_br");
	c->bin_firstseg     = (int *) GMT_memory (VNULL, (size_t)nb, sizeof (int), "GMT_init_br");
	
	count[0] = c->n_bin;
	stmp = (short *) GMT_memory (VNULL, (size_t)c->n_bin, sizeof (short), "GMT_init_br");
	
	GMT_err_trap (nc_get_vara_short (c->cdfid, c->bin_nseg_id, start, count, stmp));
	for (i = 0; i < c->nb; i++) c->bin_nseg[i] = stmp[c->bins[i]];
	GMT_free ((void *)stmp);
	
	itmp = (int *) GMT_memory (VNULL, (size_t)c->n_bin, sizeof (int), "GMT_init_br");
	GMT_err_trap (nc_get_vara_int (c->cdfid, c->bin_firstseg_id, start, count, itmp));
	for (i = 0; i < c->nb; i++) c->bin_firstseg[i] = itmp[c->bins[i]];
	
	GMT_free ((void *)itmp);
	
	return (0);
}

GMT_LONG GMT_get_br_bin (GMT_LONG b, struct GMT_BR *c, GMT_LONG *level, GMT_LONG n_levels)
/* b: index number into c->bins */
/* level: Levels of features to extract */
/* n_levels: # of such levels. 0 means use all levels */
{
	size_t start[1], count[1];
	int *seg_start;
	short *seg_n, *seg_level;
	GMT_LONG s, i, k, skip, err;
	
	c->lon_sw = (c->bins[b] % c->bin_nx) * c->bin_size / 60.0;
	c->lat_sw = 90.0 - ((c->bins[b] / c->bin_nx) + 1) * c->bin_size / 60.0;
	c->ns = c->bin_nseg[b];
	
	if (c->ns == 0) return (GMT_NOERROR);
	
	start[0] = c->bin_firstseg[b];
	count[0] = c->bin_nseg[b];
	
	seg_n = (short *) GMT_memory (VNULL, (size_t)c->bin_nseg[b], sizeof (short), "GMT_get_br_bin");
	seg_level = (short *) GMT_memory (VNULL, (size_t)c->bin_nseg[b], sizeof (short), "GMT_get_br_bin");
	seg_start = (int *) GMT_memory (VNULL, (size_t)c->bin_nseg[b], sizeof (int), "GMT_get_br_bin");
	
	GMT_err_trap (nc_get_vara_short (c->cdfid, c->seg_n_id, start, count, seg_n));
        GMT_err_trap (nc_get_vara_short (c->cdfid, c->seg_level_id, start, count, seg_level));
        GMT_err_trap (nc_get_vara_int (c->cdfid, c->seg_start_id, start, count, seg_start));

	c->seg = NULL;
	for (s = i = 0; i < c->ns; i++) {
		if (n_levels == 0)
			skip = FALSE;
		else {
			for (k = 0, skip = TRUE; skip && k < n_levels; k++)
				if (seg_level[i] == level[k]) skip = FALSE;
		}
		if (skip) continue;
		if (!c->seg) c->seg = (struct GMT_BR_SEGMENT *) GMT_memory (VNULL, (size_t)c->ns, sizeof (struct GMT_BR_SEGMENT), "GMT_get_br_bin");
		c->seg[s].n = seg_n[i];
		c->seg[s].level = seg_level[i];
		c->seg[s].dx = (short *) GMT_memory (VNULL, (size_t)c->seg[s].n, sizeof (short), "GMT_get_br_bin");
		c->seg[s].dy = (short *) GMT_memory (VNULL, (size_t)c->seg[s].n, sizeof (short), "GMT_get_br_bin");
		start[0] = seg_start[i];
		count[0] = c->seg[s].n;
		GMT_err_trap (nc_get_vara_short (c->cdfid, c->pt_dx_id, start, count, c->seg[s].dx));
                GMT_err_trap (nc_get_vara_short (c->cdfid, c->pt_dy_id, start, count, c->seg[s].dy));

		s++;
	}
	
	c->ns = s;

	GMT_free ((void *) seg_n);	
	GMT_free ((void *) seg_level);	
	GMT_free ((void *) seg_start);	
	
	return (GMT_NOERROR);
}

GMT_LONG GMT_assemble_shore (struct GMT_SHORE *c, GMT_LONG dir, GMT_LONG assemble, GMT_LONG shift, double west, double east, struct GMT_GSHHS_POL **pol)
                
                     
/* assemble: TRUE if polygons is needed */
/* shift: TRUE if longitudes may have to be shifted */
/* edge: Edge test for shifting */

{
	struct GMT_GSHHS_POL *p;
	GMT_LONG start_side, next_side, id, P = 0, more, p_alloc, wet_or_dry, use_this_level, high_seg_level = GMT_MAX_GSHHS_LEVEL;
	GMT_LONG n_alloc, cid, nid, add, first_pos, entry_pos, n, low_level, high_level, fid, nseg_at_level[GMT_MAX_GSHHS_LEVEL+1];
	GMT_LONG completely_inside;
	double *xtmp, *ytmp, plon, plat;
	
	if (!assemble) {	/* Easy, just need to scale all segments to degrees and return */
	
		p = (struct GMT_GSHHS_POL *) GMT_memory (VNULL, (size_t)c->ns, sizeof (struct GMT_GSHHS_POL), "GMT_assemble_shore");
		
		for (id = 0; id < c->ns; id++) {
			p[id].lon = (double *) GMT_memory (VNULL, (size_t)c->seg[id].n, sizeof (double), "GMT_assemble_shore");
			p[id].lat = (double *) GMT_memory (VNULL, (size_t)c->seg[id].n, sizeof (double), "GMT_assemble_shore");
			p[id].n = GMT_copy_to_shore_path (p[id].lon, p[id].lat, c, id);
			p[id].level = c->seg[id].level;
			p[id].fid = c->seg[id].fid;
			p[id].interior = FALSE;
			GMT_shore_path_shift2 (p[id].lon, p[id].lat, p[id].n, west, east, c->leftmost_bin);
		}
	
		*pol = p;
		return (c->ns);
	}
	
	/* Check the consistency of node levels in case some features are dropped */
	
	memset ((void *)nseg_at_level, 0, (size_t)((GMT_MAX_GSHHS_LEVEL + 1) * sizeof (GMT_LONG)));
	for (id = 0; id < c->ns; id++) if (c->seg[id].entry != 4) nseg_at_level[c->seg[id].level]++;	/* Only count segments that crosses the bin */
	for (n = 0; n <= GMT_MAX_GSHHS_LEVEL; n++) if (nseg_at_level[n]) high_seg_level = n;
	
	if (c->ns == 0) for (n = 0; n < 4; n++) high_seg_level = MIN (c->node_level[n], high_seg_level);	/* Initialize to lowest when there are no segments */
	for (n = high_level = 0; n < 4; n++) {
		c->node_level[n] = (unsigned char)MIN (c->node_level[n], high_seg_level);
		high_level = MAX (c->node_level[n], high_level);
	}
	
	wet_or_dry = (dir == 1) ? 1 : 0;
	use_this_level = (high_level%2 == wet_or_dry && high_level >= c->min_level);

	if (c->ns == 0 && !use_this_level) return (0);	/* No polygons for this bin */
	
	/* Here we must assemble [at least one] polygons in the correct order */
	
	for (n = 0, completely_inside = TRUE; completely_inside && n < c->ns; n++) if (c->seg[n].entry != 4) completely_inside = FALSE;
	
	shore_prepare_sides (c, dir);
	
	p_alloc = (c->ns == 0) ? 1 : GMT_SMALL_CHUNK;
	p = (struct GMT_GSHHS_POL *) GMT_memory (VNULL, (size_t)p_alloc, sizeof (struct GMT_GSHHS_POL), "GMT_assemble_shore");
	
	low_level = GMT_MAX_GSHHS_LEVEL;
	
	if (completely_inside && use_this_level) {	/* Must include path of this bin outline as first polygon */
		p[0].n = GMT_graticule_path (&p[0].lon, &p[0].lat, dir, c->lon_corner[3], c->lon_corner[1], c->lat_corner[0], c->lat_corner[2]);
		p[0].level = (c->node_level[0] == 2 && c->flag == GMT_NO_LAKES) ? 1 : c->node_level[0];	/* Any corner will do */
		p[0].fid = p[0].level;	/* Assumes no riverlake is that big to contain an entire bin */
		p[0].interior = FALSE;
		P = 1;
	}		
	
	while (c->n_entries > 0) {	/* More segments to connect */
	
		low_level = GMT_MAX_GSHHS_LEVEL;
		start_side = 0;
		id = GMT_shore_get_first_entry (c, dir, &start_side);
		next_side = c->seg[id].exit;
		
		n_alloc = c->seg[id].n;
		fid = c->seg[id].fid;
		p[P].lon = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_assemble_shore");
		p[P].lat = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_assemble_shore");
		n = GMT_copy_to_shore_path (p[P].lon, p[P].lat, c, id);
		if ((GMT_LONG)c->seg[id].level < low_level) low_level = (GMT_LONG)c->seg[id].level;
		
		more = TRUE;
		first_pos = GMT_shore_get_position (start_side, c->seg[id].dx[0], c->seg[id].dy[0]);
		while (more) {
		
			id = GMT_shore_get_next_entry (c, dir, next_side, id);
		
			if (id < 0) {	/* Corner */
				cid = id + 4;
				nid = (dir == 1) ? (cid + 1) % 4 : cid;
				if ((add = GMT_map_path (p[P].lon[n-1], p[P].lat[n-1], c->lon_corner[cid], c->lat_corner[cid], &xtmp, &ytmp))) {
					n_alloc += add;
					p[P].lon = (double *) GMT_memory ((void *)p[P].lon, (size_t)n_alloc, sizeof (double), "GMT_assemble_shore");
					p[P].lat = (double *) GMT_memory ((void *)p[P].lat, (size_t)n_alloc, sizeof (double), "GMT_assemble_shore");
					memcpy ((void *)&p[P].lon[n], (void *)xtmp, (size_t)(add * sizeof (double)));
					memcpy ((void *)&p[P].lat[n], (void *)ytmp, (size_t)(add * sizeof (double)));
					n += add;
				}
				next_side = ((id + 4) + dir + 4) % 4;
				if ((GMT_LONG)c->node_level[nid] < low_level) low_level = (GMT_LONG)c->node_level[nid];
			}
			else {
				GMT_shore_to_degree (c, c->seg[id].dx[0], c->seg[id].dy[0], &plon, &plat);
				if ((add = GMT_map_path (p[P].lon[n-1], p[P].lat[n-1], plon, plat, &xtmp, &ytmp))) {
					n_alloc += add;
					p[P].lon = (double *) GMT_memory ((void *)p[P].lon, (size_t)n_alloc, sizeof (double), "GMT_assemble_shore");
					p[P].lat = (double *) GMT_memory ((void *)p[P].lat, (size_t)n_alloc, sizeof (double), "GMT_assemble_shore");
					memcpy ((void *)&p[P].lon[n], (void *)xtmp, (size_t)(add * sizeof (double)));
					memcpy ((void *)&p[P].lat[n], (void *)ytmp, (size_t)(add * sizeof (double)));
					n += add;
				}
				entry_pos = GMT_shore_get_position (next_side, c->seg[id].dx[0], c->seg[id].dy[0]);
				if (next_side == start_side && entry_pos == first_pos)
					more = FALSE;
				else {
					n_alloc += c->seg[id].n;
					p[P].lon = (double *) GMT_memory ((void *)p[P].lon, (size_t)n_alloc, sizeof (double), "GMT_assemble_shore");
					p[P].lat = (double *) GMT_memory ((void *)p[P].lat, (size_t)n_alloc, sizeof (double), "GMT_assemble_shore");
					n += GMT_copy_to_shore_path (&p[P].lon[n], &p[P].lat[n], c, id);
					next_side = c->seg[id].exit;
					if ((GMT_LONG)c->seg[id].level < low_level) low_level = (GMT_LONG)c->seg[id].level;
				}
			}
			if (add) {
				GMT_free ((void *)xtmp);
				GMT_free ((void *)ytmp);
			}
		}
		p[P].n = n;
		p[P].interior = FALSE;
		p[P].level = (dir == 1) ? 2 * ((low_level - 1) / 2) + 1: 2 * (low_level/2);
		p[P].fid = (p[P].level == 2 && fid == RIVERLAKE) ? RIVERLAKE : p[P].level;	/* Not sure about this yet */
		P++;
		if (P == p_alloc) {
			p_alloc <<= 1;
			p = (struct GMT_GSHHS_POL *) GMT_memory ((void *)p, (size_t)p_alloc, sizeof (struct GMT_GSHHS_POL), "GMT_assemble_shore");
		}
	}
	
	/* Then add all interior polygons, if any */
	
	for (id = 0; id < c->ns; id++) {
		if (c->seg[id].entry < 4) continue;
		n_alloc = c->seg[id].n;
		p[P].lon = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_assemble_shore");
		p[P].lat = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_assemble_shore");
		p[P].n = GMT_copy_to_shore_path (p[P].lon, p[P].lat, c, id);
		p[P].interior = TRUE;
		p[P].level = c->seg[id].level;
		p[P].fid = c->seg[id].fid;
		P++;
		if (P == p_alloc) {
			p_alloc <<= 1;
			p = (struct GMT_GSHHS_POL *) GMT_memory ((void *)p, (size_t)p_alloc, sizeof (struct GMT_GSHHS_POL), "GMT_assemble_shore");
		}
	}
	
	GMT_shore_pau_sides (c);

	if (c->ns > 0) p = (struct GMT_GSHHS_POL *) GMT_memory ((void *)p, (size_t)P, sizeof (struct GMT_GSHHS_POL), "GMT_assemble_shore");
	
	for (id = 0; id < P; id++) GMT_shore_path_shift2 (p[id].lon, p[id].lat, p[id].n, west, east, c->leftmost_bin);

	*pol = p;
	return (P);
}
		
GMT_LONG GMT_assemble_br (struct GMT_BR *c, GMT_LONG shift, double edge, struct GMT_GSHHS_POL **pol)
/* shift: TRUE if longitudes may have to be shifted */
/* edge: Edge test for shifting */       
{
	struct GMT_GSHHS_POL *p;
	GMT_LONG id;
	
	p = (struct GMT_GSHHS_POL *) GMT_memory (VNULL, (size_t)c->ns, sizeof (struct GMT_GSHHS_POL), "GMT_assemble_br");
	
	for (id = 0; id < c->ns; id++) {
		p[id].lon = (double *) GMT_memory (VNULL, (size_t)c->seg[id].n, sizeof (double), "GMT_assemble_br");
		p[id].lat = (double *) GMT_memory (VNULL, (size_t)c->seg[id].n, sizeof (double), "GMT_assemble_br");
		p[id].n = GMT_copy_to_br_path (p[id].lon, p[id].lat, c, id);
		p[id].level = c->seg[id].level;
		if (shift) GMT_shore_path_shift (p[id].lon, p[id].lat, p[id].n, edge);
	}
	
	*pol = p;
	return (c->ns);
}
		
void GMT_free_shore (struct GMT_SHORE *c)
{	/* Removes allocated variables for this block only */
	GMT_LONG i;
	
	for (i = 0; i < c->ns; i++) {
		GMT_free ((void *)c->seg[i].dx);
		GMT_free ((void *)c->seg[i].dy);
	}
	if (c->ns) GMT_free ((void *)c->seg);
}
		
void GMT_free_br (struct GMT_BR *c)
{	/* Removes allocated variables for this block only */
	GMT_LONG i;
	
	for (i = 0; i < c->ns; i++) {
		GMT_free ((void *)c->seg[i].dx);
		GMT_free ((void *)c->seg[i].dy);
	}
	if (c->ns) GMT_free ((void *)c->seg);
}
		
void GMT_shore_cleanup (struct GMT_SHORE *c)
{
	GMT_free ((void *)c->bins);
	GMT_free ((void *)c->bin_info);
	GMT_free ((void *)c->bin_nseg);
	GMT_free ((void *)c->bin_firstseg);
	GMT_free ((void *)c->GSHHS_parent);
	GMT_free ((void *)c->GSHHS_area);
	GMT_free ((void *)c->GSHHS_area_fraction);
	GMT_free ((void *)c->GSHHS_node);
	nc_close (c->cdfid);
}

void GMT_br_cleanup (struct GMT_BR *c)
{
	GMT_free ((void *)c->bins);
	GMT_free ((void *)c->bin_nseg);
	GMT_free ((void *)c->bin_firstseg);
        nc_close (c->cdfid);
}

GMT_LONG GMT_prep_polygons (struct GMT_GSHHS_POL **p_old, GMT_LONG np, GMT_LONG sample, double step, GMT_LONG anti_bin)
{
	/* This function will go through each of the polygons and determine
	 * if the polygon is clipped by the map boundary, and if so if it
	 * wraps around to the other side due to 360 degree periodicities
	 * A wrapped polygon will be returned as two new polygons so that
	 * this function may return more polygons that it receives.
	 * Upon return the polygons are in x,y inches, not degrees.
	 *
	 * *p is the array of np polygons
	 * sample is TRUE if we need to resample the polygons to reduce point spacing
	 * step is the new maximum point separation in degrees
	 * anti_bin, if >= 0, indicates a possible problem bin at the antipole using -JE only
	 * We also explicitly close all polygons if they are not so already.
	 */

	GMT_LONG k, np_new, n_use, n, start, n_alloc;
	GMT_LONG close;
	double *xtmp, *ytmp;
	struct GMT_GSHHS_POL *p;

	p = *p_old;

	np_new = np;
		
	for (k = 0; k < np; k++) {
			
		if (sample) p[k].n = GMT_fix_up_path (&p[k].lon, &p[k].lat, (GMT_LONG)p[k].n, step, 0);
		
		/* Clip polygon against map boundary if necessary and return plot x,y in inches */
				
		if ((n = GMT_clip_to_map (p[k].lon, p[k].lat, (GMT_LONG)p[k].n, &xtmp, &ytmp)) == 0) {	/* Completely outside */
			p[k].n = 0;	/* Note the memory in lon, lat not freed yet */
			continue;
		}
			
		/* Must check if polygon must be split and partially plotted at both edges of map */
				
		if ((*GMT_will_it_wrap) (xtmp, ytmp, n, &start)) {	/* Polygon does indeed wrap */
				
			/* First truncate against left border */
						
			GMT_n_plot = GMT_truncate (xtmp, ytmp, n, start, -1);
			n_use = GMT_compact_line (GMT_x_plot, GMT_y_plot, GMT_n_plot, FALSE, 0);
			if (project_info.three_D) GMT_2D_to_3D (GMT_x_plot, GMT_y_plot, project_info.z_level, GMT_n_plot);
			close = GMT_polygon_is_open (GMT_x_plot, GMT_y_plot, n_use);
			n_alloc = (close) ? n_use + 1 : n_use;
			p[k].lon = (double *) GMT_memory ((void *)p[k].lon, (size_t)n_alloc, sizeof (double), GMT_program);
			p[k].lat = (double *) GMT_memory ((void *)p[k].lat, (size_t)n_alloc, sizeof (double), GMT_program);
			memcpy ((void *)p[k].lon, (void *)GMT_x_plot, (size_t)(n_use * sizeof (double)));
			memcpy ((void *)p[k].lat, (void *)GMT_y_plot, (size_t)(n_use * sizeof (double)));
			if (close) {	/* Must explicitly close the polygon */
				p[k].lon[n_use] = p[k].lon[0];
				p[k].lat[n_use] = p[k].lat[0];
			}
			p[k].n = n_alloc;
								
			/* Then truncate against right border */
						
			GMT_n_plot = GMT_truncate (xtmp, ytmp, n, start, +1);
			n_use = GMT_compact_line (GMT_x_plot, GMT_y_plot, GMT_n_plot, FALSE, 0);
			if (project_info.three_D) GMT_2D_to_3D (GMT_x_plot, GMT_y_plot, project_info.z_level, GMT_n_plot);
			p = (struct GMT_GSHHS_POL *) GMT_memory ((void *)p, (size_t)(np_new + 1), sizeof (struct GMT_GSHHS_POL), GMT_program);
			close = GMT_polygon_is_open (GMT_x_plot, GMT_y_plot, n_use);
			n_alloc = (close) ? n_use + 1 : n_use;
			p[np_new].lon = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), GMT_program);
			p[np_new].lat = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), GMT_program);
			memcpy ((void *)p[np_new].lon, (void *)GMT_x_plot, (size_t)(n_use * sizeof (double)));
			memcpy ((void *)p[np_new].lat, (void *)GMT_y_plot, (size_t)(n_use * sizeof (double)));
			if (close) {	/* Must explicitly close the polygon */
				p[np_new].lon[n_use] = p[np_new].lon[0];
				p[np_new].lat[n_use] = p[np_new].lat[0];
			}
			p[np_new].n = n_alloc;
			p[np_new].interior = p[k].interior;
			p[np_new].level = p[k].level;
			p[np_new].fid = p[k].fid;
			np_new++;
		}
		else {
			n_use = GMT_compact_line (xtmp, ytmp, n, FALSE, 0);
			if (project_info.three_D) GMT_2D_to_3D (xtmp, ytmp, project_info.z_level, n_use);
			if (anti_bin > 0 && step == 0.0) {	/* Must warn for donut effect */
				if (gmtdefs.verbose) fprintf (stderr, "%s: GMT Warning: Antipodal bin # %ld not filled!\n", GMT_program, anti_bin);
				GMT_free ((void *)xtmp);
				GMT_free ((void *)ytmp);
				continue;
			}
					
			else {
				close = GMT_polygon_is_open (xtmp, ytmp, n_use);
				n_alloc = (close) ? n_use + 1 : n_use;
				p[k].lon = (double *) GMT_memory ((void *)p[k].lon, (size_t)n_alloc, sizeof (double), GMT_program);
				p[k].lat = (double *) GMT_memory ((void *)p[k].lat, (size_t)n_alloc, sizeof (double), GMT_program);
				memcpy ((void *)p[k].lon, (void *)xtmp, (size_t)(n_use * sizeof (double)));
				memcpy ((void *)p[k].lat, (void *)ytmp, (size_t)(n_use * sizeof (double)));
				if (close) {	/* Must explicitly close the polygon */
					p[k].lon[n_use] = p[k].lon[0];
					p[k].lat[n_use] = p[k].lat[0];
				}
				p[k].n = n_alloc;
			}
		}
				
		GMT_free ((void *)xtmp);
		GMT_free ((void *)ytmp);
	}

	*p_old = p;

	return (np_new);
}

/* ---------- LOWER LEVEL FUNCTIONS CALLED BY THE ABOVE ------------ */

GMT_LONG GMT_copy_to_shore_path (double *lon, double *lat, struct GMT_SHORE *s, GMT_LONG id)
{
	GMT_LONG i;
	for (i = 0; i < (GMT_LONG)s->seg[id].n; i++)
		GMT_shore_to_degree (s, s->seg[id].dx[i], s->seg[id].dy[i], &lon[i], &lat[i]);
	return (s->seg[id].n);
}

GMT_LONG GMT_copy_to_br_path (double *lon, double *lat, struct GMT_BR *s, GMT_LONG id)
{
	GMT_LONG i;
	for (i = 0; i < (GMT_LONG)s->seg[id].n; i++)
		GMT_br_to_degree (s, s->seg[id].dx[i], s->seg[id].dy[i], &lon[i], &lat[i]);
	return (s->seg[id].n);
}

void GMT_shore_to_degree (struct GMT_SHORE *c, short int dx, short int dy, double *lon, double *lat)
{
	*lon = c->lon_sw + ((unsigned short)dx) * c->scale;
	*lat = c->lat_sw + ((unsigned short)dy) * c->scale;
}

void GMT_br_to_degree (struct GMT_BR *c, short int dx, short int dy, double *lon, double *lat)
{
	*lon = c->lon_sw + ((unsigned short)dx) * c->scale;
	*lat = c->lat_sw + ((unsigned short)dy) * c->scale;
}

GMT_LONG GMT_shore_get_next_entry (struct GMT_SHORE *c, GMT_LONG dir, GMT_LONG side, GMT_LONG id)
{
	/* Finds the next entry point on the given side that is further away
	 * in the <dir> direction than previous point.  It removes the info
	 * regarding the new entry from the GSHHS_SIDE structure */
	 
	GMT_LONG k, pos, n;
	
	if (id < 0)
		pos = (dir == 1) ? 0 : GSHHS_MAX_DELTA;
	else {
		n = c->seg[id].n - 1;
		pos = GMT_shore_get_position (side, c->seg[id].dx[n], c->seg[id].dy[n]);
	}

	if (dir == 1) {
		for (k = 0; k < (GMT_LONG)c->nside[side] && (GMT_LONG)c->side[side][k].pos < pos; k++);
		id = c->side[side][k].id;
		for (k++; k < c->nside[side]; k++) c->side[side][k-1] = c->side[side][k];
		c->nside[side]--;
	}
	else {
		for (k = 0; k < (GMT_LONG)c->nside[side] && (GMT_LONG)c->side[side][k].pos > pos; k++);
		id = c->side[side][k].id;
		for (k++; k < c->nside[side]; k++) c->side[side][k-1] = c->side[side][k];
		c->nside[side]--;
	}
	if (id >= 0) c->n_entries--;
	return (id);
}

GMT_LONG GMT_shore_get_first_entry (struct GMT_SHORE *c, GMT_LONG dir, GMT_LONG *side)
{
	GMT_LONG try = 0;
	while (try < 4 && (c->nside[*side] == 0 || (c->nside[*side] == 1 && c->side[*side][0].id < 0))) {	/* No entries or only a corner left on this side */
		try++;
		*side = (*side + dir + 4) % 4;
	}
	if (try == 4) return (-5);
	return (c->side[*side][0].id);
}	

int GMT_shore_asc_sort (const void *a, const void *b)
{
	if (((struct GSHHS_SIDE *)a)->pos < ((struct GSHHS_SIDE *)b)->pos) return (-1);
	if (((struct GSHHS_SIDE *)a)->pos > ((struct GSHHS_SIDE *)b)->pos) return (1);
	return (0);
}

int GMT_shore_desc_sort (const void *a, const void *b)
{
	if (((struct GSHHS_SIDE *)a)->pos < ((struct GSHHS_SIDE *)b)->pos) return (1);
	if (((struct GSHHS_SIDE *)a)->pos > ((struct GSHHS_SIDE *)b)->pos) return (-1);
	return (0);
}

void GMT_shore_pau_sides (struct GMT_SHORE *c)
{
	GMT_LONG i;
	for (i = 0; i < 4; i++) GMT_free ((void *)c->side[i]);
}

void GMT_free_polygons (struct GMT_GSHHS_POL *p, GMT_LONG n)
{
	GMT_LONG k;
	for (k = 0; k < n; k++) {
		GMT_free ((void *)p[k].lon);
		GMT_free ((void *)p[k].lat);
	}
}

void GMT_shore_path_shift (double *lon, double *lat, GMT_LONG n, double edge)
{
	GMT_LONG i;
	
	for (i = 0; i < n; i++) if (lon[i] >= edge) lon[i] -= 360.0;
}

void GMT_shore_path_shift2old (double *lon, double *lat, GMT_LONG n, double west, double east)
{
	GMT_LONG i;
	
	/* for (i = 0; i < n; i++) if (lon[i] >= east && (lon[i]-360.0) > west) lon[i] -= 360.0; */
	for (i = 0; i < n; i++) if (lon[i] > east && (lon[i]-360.0) >= west) lon[i] -= 360.0;
}

void GMT_shore_path_shift2 (double *lon, double *lat, GMT_LONG n, double west, double east, GMT_LONG leftmost)
{
	GMT_LONG i;
	
	if (leftmost) {	/* Must check this bin differently  */
		for (i = 0; i < n; i++) if (lon[i] >= east && (lon[i]-360.0) >= west) lon[i] -= 360.0;
	}
	else {
		for (i = 0; i < n; i++) if (lon[i] > east && (lon[i]-360.0) >= west) lon[i] -= 360.0;
	}
}

GMT_LONG GMT_shore_get_position (GMT_LONG side, short int x, short int y)
{
	/* Returns the position along the given side */
	
	return ((side%2) ? ((side == 1) ? (unsigned short)y : GSHHS_MAX_DELTA - (unsigned short)y) : ((side == 0) ? (unsigned short)x : GSHHS_MAX_DELTA - (unsigned short)x));
}

void shore_prepare_sides (struct GMT_SHORE *c, GMT_LONG dir)
{
	GMT_LONG s, i, n[4];
	
	c->lon_corner[0] = c->lon_sw + ((dir == 1) ? c->bsize : 0.0);
	c->lon_corner[1] = c->lon_sw + c->bsize;
	c->lon_corner[2] = c->lon_sw + ((dir == 1) ? 0.0 : c->bsize);
	c->lon_corner[3] = c->lon_sw;
	c->lat_corner[0] = c->lat_sw;
	c->lat_corner[1] = c->lat_sw + ((dir == 1) ? c->bsize : 0.0);
	c->lat_corner[2] = c->lat_sw + c->bsize;
	c->lat_corner[3] = c->lat_sw + ((dir == 1) ? 0.0 : c->bsize);

	for (i = 0; i < 4; i++) c->nside[i] = n[i] = 1;
	/* for (s = 0; s < c->ns; s++) if (c->seg[s].level < 3 && c->seg[s].entry < 4) c->nside[c->seg[s].entry]++; */
	for (s = 0; s < c->ns; s++) if (c->seg[s].entry < 4) c->nside[c->seg[s].entry]++;
	
	for (i = c->n_entries = 0; i < 4; i++) {	/* Allocate memory and add corners */
		c->side[i] = (struct GSHHS_SIDE *) GMT_memory (VNULL, (size_t)c->nside[i], sizeof (struct GSHHS_SIDE), "shore_prepare_sides");
		c->side[i][0].pos = (dir == 1) ? GSHHS_MAX_DELTA : 0;
		c->side[i][0].id = (short)(i - 4);
		c->n_entries += c->nside[i] - 1;
	}
		
	for (s = 0; s < c->ns; s++) {	/* Add entry points */
		/* if (c->seg[s].level > 2 || (i = c->seg[s].entry) == 4) continue; */
		if ((i = c->seg[s].entry) == 4) continue;
		c->side[i][n[i]].pos = (unsigned short)GMT_shore_get_position (i, c->seg[s].dx[0], c->seg[s].dy[0]);
		c->side[i][n[i]].id = (short)s;
		n[i]++;
	}
	
	for (i = 0; i < 4; i++)	{	/* sort on position */
		if (dir == 1)
			qsort ((void *)c->side[i], (size_t)c->nside[i], sizeof (struct GSHHS_SIDE), GMT_shore_asc_sort);
		else
			qsort ((void *)c->side[i], (size_t)c->nside[i], sizeof (struct GSHHS_SIDE), GMT_shore_desc_sort);
	}
}

char *GMT_shore_getpathname (char *stem, char *path) {
	/* Prepends the appropriate directory to the file name
	 * and returns path if file is readable, NULL otherwise */
	 
	FILE *fp = NULL;
	char dir[BUFSIZ];

	/* This is the order of checking:
	 * 1. Is there a file coastline.conf in current directory, GMT_USERDIR or GMT_SHAREDIR[/coast]?
	 *    If so, use its information
	 * 2. Look in current directory, GMT_USERDIR or GMT_SHAREDIR[/coast] for file "name".
	 */
	 
	/* 1. First check for coastline.conf */
	
	if (GMT_getsharepath ("conf", "coastline", ".conf", path) || GMT_getsharepath ("coast", "coastline", ".conf", path)) {

		/* We get here if coastline.conf exists - search among its directories for the named file */

		fp = fopen (path, "r");
		while (fgets (dir, BUFSIZ, fp)) {	/* Loop over all input lines until found or done */
			GMT_chop (dir);		/* Chop off LF or CR/LF */
			if (dir[0] == '#' || dir[0] == '\0') continue;	/* Comment or blank */
			sprintf (path, "%s%c%s%s", dir, DIR_DELIM, stem, ".cdf");
			if (!access (path, R_OK)) {
				fclose (fp);
				return (path);
			}
		}
		fclose (fp);
	}
	
	/* 2. Then check for the named file itself */

	if (GMT_getsharepath ("coast", stem, ".cdf", path)) return (path);

	return (NULL);
}
