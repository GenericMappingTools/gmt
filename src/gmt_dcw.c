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

#include "gmt_dev.h"
#include "gmt_internals.h"

/* Misc functions to find and read DCW polygons.
 * Some of the countries have state borders too.
 * The PUBLIC functions are (5):
 *
 * gmt_DCW_option	: Present the DCW option and modifier usage
 * gmt_DCW_parse	: Parse the DCW option arguments
 * gmt_DCW_list 	: List the available polygons and exit
 * gmt_DCW_operation	: Get DCW polygons and operate on them
 * gmt_DCW_free		: Free memory allocated by gmt_DCW_parse
 *
 * Author:	Paul Wessel
 * Date:	1-MAY-2013
 * Version:	5.x
 *
 * We expect to find the file dcw-gmt.nc, dcw-countries.txt, and dcw-states.txt
 * in one of the dirs accessible to GMT or pointed to by the default DIR_DCW.
 * See separate subversion project DCW for the maintenance of the raw files that
 * are used to build the netCDF file [svn://gmtserver.soest.hawai.edu/DCW].
 */

#define DCW_SITE 			"ftp://ftp.soest.hawaii.edu/gmt"
#define GMT_DCW_N_CONTINENTS		8

#define DCW_GET_COUNTRY			1	/* Extract countries only */
#define DCW_GET_COUNTRY_AND_STATE	2	/* Extract countries and states */
#define DCW_DO_OUTLINE			1	/* Draw outline of polygons */
#define DCW_DO_FILL			2	/* Fill the polygons */

struct GMT_DCW_COUNTRY {	/* Information per country */
	char continent[4];	/* 2-char continent code (EU, NA, SA, AF, AU, AN) */
	char code[4];		/* 2-char country code ISO 3166-1 (e.g., NO, US) */
	char name[80];		/* Full name of the country */
};

struct GMT_DCW_STATE {		/* Information per state */
	char country[4];	/* 2-char country code ISO 3166-1 (e.g., BR, US) */
	char code[4];		/* 2/3-char state codes for US, Canada, China, Argentina, Australia, Brazil, Russia (e.g., TX) */
	char name[80];		/* Full name of the state */
};

struct GMT_DCW_COUNTRY_STATE {		/* Information per country with state */
	char country[4];		/* 2/3-char country code ISO 3166-1 (e.g., BR, US) for countries with states */
};

/* Compile in read-only structures and arrays with the information */

static char *GMT_DCW_continents[GMT_DCW_N_CONTINENTS] = {"Africa", "Antarctica", "Asia", "Europe", "Oceania", "North America", "South America", "Miscellaneous"};

/* Local functions only visible inside this file */

GMT_LOCAL bool dcw_get_path (struct GMT_CTRL *GMT, char *name, char *suffix, char *path) {
	bool found = false;

	/* This is the order of checking:
	 * 1. Check in GMT->session.DCWDIR, if set
	 * 2. Look via gmt_getsharepath.
	 */

	if (GMT->session.DCWDIR) {	/* 1. Check in GMT->session.DCWDIR */
		sprintf (path, "%s/%s%s", GMT->session.DCWDIR, name, suffix);
		if ( access (path, R_OK) == 0)
			found = true;
		else {
			/* remove reference to invalid GMT->session.DCWDIR but don't free
			 * the pointer. this is no leak because the reference still exists
			 * in the previous copy of the current GMT_CTRL struct. */
			GMT->session.DCWDIR = NULL;
		}
	}
	if (!found && gmt_getsharepath (GMT, "dcw", name, suffix, path, R_OK)) found = true;	/* Found it in share or user somewhere */
	if (!found) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to find or open the Digital Chart of the World for GMT\n");
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Perhaps you did not install this file in DIR_DCW, the shared dir, or the user dir?\n");
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Use your package manager to install package dcw-gmt.\n");
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Alternatively, get the latest dcw-gmt-<version>.tar.gz or dcw-gmt-<version>.zip from the %s.\n", DCW_SITE);
	}
	return (found);
}

GMT_LOCAL int dcw_load_lists (struct GMT_CTRL *GMT, struct GMT_DCW_COUNTRY **C, struct GMT_DCW_STATE **S, struct GMT_DCW_COUNTRY_STATE **CS, unsigned int dim[]) {
	/* Open and read list of countries and states and return via two struct and one char arrays plus dimensions in dim */
	size_t n_alloc = 300;
	unsigned int k, n;
	char path[PATH_MAX] = {""}, line[BUFSIZ] = {""};
	FILE *fp = NULL;
	struct GMT_DCW_COUNTRY *Country = NULL;
	struct GMT_DCW_STATE *State = NULL;
	struct GMT_DCW_COUNTRY_STATE *Country_State = NULL;

	if (!dcw_get_path (GMT, "dcw-countries", ".txt", path)) return -1;

	/* Get countries first */
	if ((fp = fopen (path, "r")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to open file %s [permission trouble?]\n", path);
		return -1;
	}
	Country = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_DCW_COUNTRY);
	k = 0;
	while ( gmt_fgets (GMT, line, BUFSIZ, fp)) {
		if (line[0] == '#') continue;	/* Skip comments */
		sscanf (line, "%s %s %[^\n]", Country[k].continent, Country[k].code,  Country[k].name);
		k++;
		if (k == n_alloc) {
			n_alloc += 100;
			Country = gmt_M_memory (GMT, Country, n_alloc, struct GMT_DCW_COUNTRY);
		}
	}
	fclose (fp);
	dim[0] = k;	/* Number of countries */
	Country = gmt_M_memory (GMT, Country, k, struct GMT_DCW_COUNTRY);

	/* Get states */
	if (!dcw_get_path (GMT, "dcw-states", ".txt", path)) {
		gmt_M_free (GMT, Country);
		return -1;
	}
	if ((fp = fopen (path, "r")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to open file %s [permission trouble?]\n", path);
		gmt_M_free (GMT, Country);
		return -1;
	}
	State = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_DCW_STATE);
	k = 0;	n = 1;
	while ( gmt_fgets (GMT, line, BUFSIZ, fp)) {
		if (line[0] == '#') continue;	/* Skip comments */
		sscanf (line, "%s %s %[^\n]", State[k].country, State[k].code,  State[k].name);
		if (k && strcmp (State[k].country, State[k-1].country)) n++;	/* New country with states */
		k++;
		if (k == n_alloc) {
			n_alloc += 100;
			State = gmt_M_memory (GMT, State, n_alloc, struct GMT_DCW_STATE);
		}
	}
	fclose (fp);
	dim[1] = k;	/* Number of states */
	State = gmt_M_memory (GMT, State, k, struct GMT_DCW_STATE);

	/* Get list of countries with states */

	dim[2] = n;	/* Number of countries with states */
	if (CS) {	/* Wants list returned */
		Country_State = gmt_M_memory (GMT, NULL, n, struct GMT_DCW_COUNTRY_STATE);
		gmt_M_memcpy (Country_State[0].country, State[0].country, 4, char);
		for (k = n = 1; k < dim[1]; k++) {
			if (strcmp (State[k].country, State[k-1].country)) gmt_M_memcpy (Country_State[n++].country, State[k].country, 4, char);
		}
		*CS = Country_State;
	}

	*C = Country;
	*S = State;

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "DCW: Found %u countries, %u countries with states, and %u states\n", dim[0], dim[2], dim[1]);
	return 0;
}

GMT_LOCAL int dcw_comp_countries (const void *p1, const void *p2) {
	/* Used to sort countries alphabetically */
	struct GMT_DCW_COUNTRY *A = (struct GMT_DCW_COUNTRY *)p1;
	struct GMT_DCW_COUNTRY *B = (struct GMT_DCW_COUNTRY *)p2;
	return (strcmp (A->code, B->code));
}

GMT_LOCAL int dcw_find_country (char *code, struct GMT_DCW_COUNTRY *list, int n) {
	/* Basic binary search for country with given code and an alphabetically sorted list */
	int low = 0, high = n, mid, last = -1, way;

	while (low < high) {
		mid = (low + high) / 2;
		if (mid == last) return (-1);	/* No such code */
		way = strcmp (code, list[mid].code);
		if (way > 0) low = mid;
		else if (way < 0) high = mid;
		else return (mid);
		last = mid;
	}
	return (low);
}

GMT_LOCAL int dcw_find_state (char *scode, char *ccode, struct GMT_DCW_STATE *slist, int ns) {
	/* Return state id given country and state codes using a linear search */
	int i;

	for (i = 0; i < ns; i++) if (!strcmp (scode, slist[i].code) && !strcmp (ccode, slist[i].country)) return (i);
	return (-1);
}

GMT_LOCAL bool gmt_dcw_country_has_states (char *code, struct GMT_DCW_COUNTRY_STATE *st_country, unsigned int n) {
	/* Return true if this country has interior state boundaries */
	unsigned int i;
	for (i = 0; i < n; i++) if (!strcmp (code, st_country[i].country)) return (true);
	return (false);
}

/*----------------------------------------------------------|
 * Public functions that are part of the GMT Devel library  |
 *----------------------------------------------------------|
 */

struct GMT_DATASET * gmt_DCW_operation (struct GMT_CTRL *GMT, struct GMT_DCW_SELECT *F, double wesn[], unsigned int mode) {
	/* Given comma-separated names, read the corresponding netCDF variables.
 	 * mode = GMT_DCW_REGION	: Return the joint w/e/s/n limits
	 * mode = GMT_DCW_PLOT		: Plot the polygons
	 * mode = GMT_DCW_DUMP		: Dump the polygons
	 * mode = GMT_DCW_EXTRACT	: Return a dataset structure
	 */
	int item, ks, retval, ncid, xvarid, yvarid, id;
	int64_t first, last;
	size_t np, max_np = 0U, n_alloc;
	uint64_t k, seg, n_segments;
	unsigned int n_items = 0, r_item = 0, pos = 0, kk, tbl = 0, j = 0, *order = NULL;
	unsigned short int *dx = NULL, *dy = NULL;
	unsigned int GMT_DCW_COUNTRIES = 0, GMT_DCW_STATES = 0, n_bodies[3] = {0, 0, 0};
	bool done, new_set, want_state, outline, fill = false, is_Antarctica = false;
	char TAG[GMT_LEN16] = {""}, dim[GMT_LEN16] = {""}, xname[GMT_LEN16] = {""};
	char yname[GMT_LEN16] = {""}, code[GMT_LEN16] = {""}, state[GMT_LEN16] = {""};
	char msg[GMT_BUFSIZ] = {""}, segment[GMT_LEN32] = {""}, path[PATH_MAX] = {""}, list[GMT_BUFSIZ] = {""};
	double west, east, south, north, xscl, yscl, out[2], *lon = NULL, *lat = NULL;
	struct GMT_RANGE *Z = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *P = NULL, *S = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMT_DCW_COUNTRY *GMT_DCW_country = NULL;
	struct GMT_DCW_STATE *GMT_DCW_state = NULL;

	for (j = ks = 0; j < F->n_items; j++) {
		if (!F->item[j]->codes || F->item[j]->codes[0] == '\0') continue;
		ks++;	/* Gave some codes */
	}
	if (ks == 0) return NULL;	/* No countries requested */
	if (mode != GMT_DCW_REGION && F->region && (mode & (GMT_DCW_PLOT+GMT_DCW_DUMP+GMT_DCW_EXTRACT)) == 0) return NULL;	/* No plotting/dumping requested, just -R */

	if (mode & GMT_DCW_REGION) {	/* Wish to determine region from polygons */
		if (wesn == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Implementation error: Must pass wesn array if mode == %d\n", GMT_DCW_REGION);
			return NULL;
		}
		wesn[XLO] = wesn[XHI] = 0.0;			/* Set to zero so it can grow below */
		wesn[YLO] = +9999.0;	wesn[YHI] = -9999.0;	/* Initialize so we can shrink it below */
	}

	if (dcw_load_lists (GMT, &GMT_DCW_country, &GMT_DCW_state, NULL, n_bodies))	/* Something went wrong */
		return NULL;

	GMT_DCW_COUNTRIES = n_bodies[0];
	GMT_DCW_STATES = n_bodies[1];

	qsort ((void *)GMT_DCW_country, (size_t)GMT_DCW_COUNTRIES, sizeof (struct GMT_DCW_COUNTRY), dcw_comp_countries);	/* Sort on country code */

	n_alloc = n_bodies[0] + n_bodies[1];	/* Presumably max items considered */
	order = gmt_M_memory (GMT, NULL, n_alloc, unsigned int);
	for (j = 0; j < F->n_items; j++) {
		pos = 0;
		while (gmt_strtok (F->item[j]->codes, ",", &pos, code)) {	/* Loop over items */
			if (code[0] == '=') {	/* Must expand a continent into all member countries */
				for (k = 0; k < GMT_DCW_COUNTRIES; k++) {
					if (strncmp (GMT_DCW_country[k].continent, &code[1], 2)) continue;	/* Not this one */
					if (n_items) strcat (list, ",");
					strcat (list, GMT_DCW_country[k].code);
					order[n_items] = j;	/* So we know which color/pen to apply for this item */
					n_items++;
				}
				if (n_items)
					GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Continent code expanded from %s to %s [%d countries]\n", F->item[j]->codes, list, n_items);
				else
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Continent code %s unrecognized\n", code);
			}
			else {	/* Just append this single one */
				if (n_items) strcat (list, ",");
				strcat (list, code);
				order[n_items] = j;	/* So we know which color/pen to apply for this item */
				n_items++;
			}
		}
	}
	if (n_items)
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Requested %d DCW items: %s\n", n_items, list);
	else {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "No countries selected\n");
		gmt_M_free (GMT, order);
		gmt_M_free (GMT, GMT_DCW_country);
		gmt_M_free (GMT, GMT_DCW_state);
		return NULL;
	}

	if (!dcw_get_path (GMT, "dcw-gmt", ".nc", path)) {
		gmt_M_free (GMT, order);
		return NULL;
	}

	if (mode > GMT_DCW_REGION) {	/* Wish to get actual polygons */
		P = GMT_Alloc_Segment (GMT->parent, GMT_NO_STRINGS, 0, 2, NULL, NULL);
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Extract polygons from DCW - The Digital Chart of the World\n");
	}

	if (mode & GMT_DCW_EXTRACT) {	/* Plan to return a dataset */
		uint64_t dim[4] = {n_items, 0, 0, 2};	/* n_items tables whose records (to be allocated) will have 2 columns */
		if ((D = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to create empty dataset for DCW polygons\n");
			gmt_free_segment (GMT, &P);
			gmt_M_free (GMT, order);
			return NULL;
		}
	}

	if ((retval = nc_open (path, NC_NOWRITE, &ncid))) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot open file %s!\n", path);
		gmt_free_segment (GMT, &P);
		gmt_M_free (GMT, order);
		return NULL;
	}

	/* Get global attributes */
	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		char version[GMT_LEN16] = {""}, source[GMT_LEN256] = {""}, title[GMT_LEN256] = {""};
		if ((retval = nc_get_att_text (ncid, NC_GLOBAL, "version", version))) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot obtain attribute version\n");
			gmt_free_segment (GMT, &P);
			gmt_M_free (GMT, order);
			return NULL;
		}
		if ((retval = nc_get_att_text (ncid, NC_GLOBAL, "title", title))) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot obtain attribute title\n");
			gmt_free_segment (GMT, &P);
			gmt_M_free (GMT, order);
			return NULL;
		}
		if ((retval = nc_get_att_text (ncid, NC_GLOBAL, "source", source))) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot obtain attribute source\n");
			gmt_free_segment (GMT, &P);
			gmt_M_free (GMT, order);
			return NULL;
		}
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Using country and state data from gmt-dcw\n");
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Title  : %s\n", title);
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Source : %s\n", source);
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Version: %s\n", version);
	}
	if ((mode & GMT_DCW_DUMP) || (mode & GMT_DCW_REGION)) {	/* Dump the coordinates to stdout or return -R means setting col types */
		gmt_set_geographic (GMT, GMT_OUT);
	}

	if (mode & GMT_DCW_REGION)	/* Just update wesn */
		Z = gmt_M_memory (GMT, NULL, n_items, struct GMT_RANGE);

	pos = item = 0;
	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
	while (gmt_strtok (list, ",", &pos, code)) {	/* Loop over countries */
		want_state = false;
		if (code[2] == '.') {	/* Requesting a state */
			gmt_M_memset (state, GMT_LEN16, char);
			strncpy (state, &code[3], GMT_LEN16-1);
			code[2] = '\0';
			want_state = true;
		}
		ks = dcw_find_country (code, GMT_DCW_country, GMT_DCW_COUNTRIES);
		if (ks == -1) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No country code matching %s (skipped)\n", code);
			continue;
		}
		k = ks;
		if (want_state) {
			if ((item = dcw_find_state (state, code, GMT_DCW_state, GMT_DCW_STATES)) == -1) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Country %s does not have states (skipped)\n", code);
				continue;
			}
			snprintf (TAG, GMT_LEN16, "%s%s", GMT_DCW_country[k].code, GMT_DCW_state[item].code);
			snprintf (msg, GMT_BUFSIZ, "Extract data for %s (%s)\n", GMT_DCW_state[item].name, GMT_DCW_country[k].name);
		}
		else {
			snprintf (TAG, GMT_LEN16, "%s", GMT_DCW_country[k].code);
			snprintf (msg, GMT_BUFSIZ, "Extract data for %s\n", GMT_DCW_country[k].name);
		}
		if (!strncmp (GMT_DCW_country[k].code, "AQ", 2U)) is_Antarctica = true;

		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, msg);
		k = strlen (msg) - 1;
		msg[k] = '\0';	/* Remove the newline for use as segment header */

		/* Open and read the netCDF file */

		snprintf (dim, GMT_LEN16, "%s_length", TAG);
		if ((retval = nc_inq_dimid (ncid, dim, &id))) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error getting ID for variable %s in %s!\n", dim, path);
			continue;
		}
		if ((retval = nc_inq_dimlen (ncid, id, &np))) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error getting dimension length for variable %s in %s!\n", dim, path);
			continue;
		}
		if (mode > GMT_DCW_REGION && np > max_np) {
			size_t tmp_size = max_np;
			gmt_M_malloc2 (GMT, lon, lat, np, &tmp_size, double);
			gmt_M_malloc2 (GMT, dx, dy, np, &max_np, unsigned short int);
			if (lon == NULL || lat == NULL || dx == NULL|| dy == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error allocation memory!\n");
				continue;
			}
		}

	        /* Get the varid of the lon and lat variables, based on their names, and get the data */

		snprintf (xname, GMT_LEN16, "%s_lon", TAG);	snprintf (yname, GMT_LEN16, "%s_lat", TAG);

		if ((retval = nc_inq_varid (ncid, xname, &xvarid))) continue;
		if ((retval = nc_get_att_double (ncid, xvarid, "min", &west))) continue;
		if ((retval = nc_get_att_double (ncid, xvarid, "max", &east))) continue;
		if ((retval = nc_get_att_double (ncid, xvarid, "scale", &xscl))) continue;
		if ((retval = nc_inq_varid (ncid, yname, &yvarid))) continue;
		if ((retval = nc_get_att_double (ncid, yvarid, "min", &south))) continue;
		if ((retval = nc_get_att_double (ncid, yvarid, "max", &north))) continue;
		if ((retval = nc_get_att_double (ncid, yvarid, "scale", &yscl))) continue;
		if (mode & GMT_DCW_REGION) {	/* Just update wesn */
			Z[r_item].west = west;	Z[r_item++].east = east;
			if (south < wesn[YLO]) wesn[YLO] = south;
			if (north > wesn[YHI]) wesn[YHI] = north;
		}
		if (mode > GMT_DCW_REGION) {	/* Need to read the data arrays */
			if ((retval = nc_get_var_ushort (ncid, xvarid, dx))) continue;
			if ((retval = nc_get_var_ushort (ncid, yvarid, dy))) continue;
		}
		if (mode == GMT_DCW_REGION) continue;
		xscl = 1.0 / xscl;	yscl = 1.0 / yscl;
		for (k = n_segments = 0; k < np; k++) {	/* Unpack */
			if (dx[k] == 65535U) n_segments++;	/* Count how many segments */
			lon[k] = (dx[k] == 65535U) ? 0.0 : dx[k] * xscl + west;
			lat[k] = (dy[k] == 65535U) ? 0.0 : dy[k] * yscl + south;
		}
		if (mode & GMT_DCW_EXTRACT) {	/* Allocate a table with the right number of segments */
			D->table[tbl] = gmt_create_table (GMT, n_segments, 0, 2, 0, false);
		}
		if (mode & GMT_DCW_PLOT) {	/* Time to consider fill/pen change */
			new_set = (tbl == 0 || order[tbl] != order[tbl-1]);	/* When item group change it is likely pen/fill changes too */
			outline = (F->item[order[tbl]]->mode & DCW_DO_OUTLINE);
			fill = (F->item[order[tbl]]->mode & DCW_DO_FILL);
			if (outline && new_set) gmt_setpen (GMT, &(F->item[order[tbl]]->pen));
			if (fill && new_set) gmt_setfill (GMT, &(F->item[order[tbl]]->fill), outline);
		}

	        /* Extract the pieces into separate segments */
		k = seg = 0;
		done = false;
	        while (!done) {
			first = -1;
			while (first == -1 && k < np) {	/* Look for next start of segment marker */
				if (lon[k] == 0.0 && lat[k] == 0.0) first = k + 1;	/* Start of segment */
				k++;
			}
			if (first == -1) { done = true; continue;}	/* No more segments */
			last = -1;
			while (last == -1 && k < np) {/* Look for next end of segment marker (or end of line) */
				if (lon[k] == 0.0 && lat[k] == 0.0) last = k - 1;	/* End of segment */
				k++;
			}
			if (last == -1) last = np - 1;	/* End of last segment */
			k--;	/* Back to last segment marker  which will be the next start marker */
			P->n_rows = last - first + 1;	/* Number of points in this segment */
			P->data[GMT_X] = &lon[first];
			P->data[GMT_Y] = &lat[first];
			if (mode & GMT_DCW_DUMP) {	/* Dump the coordinates to stdout */
				snprintf (segment, GMT_LEN32, " Segment %" PRIu64, seg);
				strcpy (GMT->current.io.segment_header, msg);
				strcat (GMT->current.io.segment_header, segment);
				GMT_Put_Record (GMT->parent, GMT_WRITE_SEGMENT_HEADER, NULL);
				for (kk = 0; kk < P->n_rows; kk++) {
					out[GMT_X] = P->data[GMT_X][kk];
					out[GMT_Y] = P->data[GMT_Y][kk];
					GMT_Put_Record (GMT->parent, GMT_WRITE_DATA, Out);
				}
				seg++;
			}
			else if (mode & GMT_DCW_EXTRACT) {	/* Attach to dataset */
				S = D->table[tbl]->segment[seg];
				S->n_rows = P->n_rows;
				gmt_M_malloc2 (GMT, S->data[GMT_X], S->data[GMT_Y], S->n_rows, NULL, double);
				gmt_M_memcpy (S->data[GMT_X], lon, S->n_rows, double);
				gmt_M_memcpy (S->data[GMT_Y], lat, S->n_rows, double);
				seg++;
			}
			else {	/* mode & GMT_DCW_PLOT: Plot this piece */
				if (fill) {	/* Plot filled polygon, w/ or w/o outline */
					if (!strncmp (TAG, "AQ", 2U)) gmt_set_seg_polar (GMT, P);
					gmt_geo_polygons (GMT, P);
				}
				else {	/* Plot outline only */
					if ((GMT->current.plot.n = gmt_geo_to_xy_line (GMT, P->data[GMT_X], P->data[GMT_Y], P->n_rows)) == 0) continue;
					gmt_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n, PSL_LINEAR);
				}
			}
		}
		tbl++;
	}
	nc_close (ncid);
	gmt_M_free (GMT, GMT_DCW_country);
	gmt_M_free (GMT, GMT_DCW_state);
	gmt_M_free (GMT, Out);

	if (mode & GMT_DCW_REGION) {
		gmt_find_range (GMT, Z, n_items, &wesn[XLO], &wesn[XHI]);
		gmt_M_free (GMT, Z);
		GMT->current.io.geo.range = GMT_IGNORE_RANGE;		/* Override this setting explicitly */
		if (F->adjust) {
			if (F->extend) {	/* Extend the region by increments */
				wesn[XLO] -= F->inc[XLO];
				wesn[YLO] -= F->inc[YLO];
				wesn[XHI] += F->inc[XHI];
				wesn[YHI] += F->inc[YHI];
			}
			else {	/* Make region be in multiples of increments */
				wesn[XLO] = floor (wesn[XLO] / F->inc[XLO]) * F->inc[XLO];
				wesn[YLO] = floor (wesn[YLO] / F->inc[YLO]) * F->inc[YLO];
				wesn[XHI] = ceil  (wesn[XHI] / F->inc[XHI]) * F->inc[XHI];
				wesn[YHI] = ceil  (wesn[YHI] / F->inc[YHI]) * F->inc[YHI];
			}
		}
		if (is_Antarctica) {	/* Must override to include pole and full longitude range */
			wesn[YLO] = -90.0;	/* Since it is a South polar cap */
			wesn[XLO] = 0.0;
			wesn[XHI] = 360.0;
		}
		/* Do basic sanity checks */
		if (wesn[YLO] < -90.0) wesn[YLO] = -90.0;
		if (wesn[YHI] > +90.0) wesn[YHI] = +90.0;
		if (fabs (wesn[XHI] - wesn[XLO]) > 360.0) {
			wesn[XLO] = 0.0;
			wesn[XHI] = 360.0;
		}
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Region implied by DCW polygons is %g/%g/%g/%g\n", wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
	}
	gmt_M_free (GMT, order);
	if (mode > GMT_DCW_REGION) {
		gmt_M_free (GMT, dx);
		gmt_M_free (GMT, dy);
		gmt_M_free (GMT, lon);
		gmt_M_free (GMT, lat);
		P->data[GMT_X] = P->data[GMT_Y] = NULL;
		gmt_free_segment (GMT, &P);
	}
	return (D);
}

unsigned int gmt_DCW_list (struct GMT_CTRL *GMT, unsigned list_mode) {
	/* List available countries [and optionally states]; then make program exit */
	unsigned int i, j, k, GMT_DCW_COUNTRIES = 0, GMT_DCW_STATES = 0, GMT_DCW_N_COUNTRIES_WITH_STATES = 0, n_bodies[3] = {0, 0, 0};
	struct GMT_DCW_COUNTRY *GMT_DCW_country = NULL;
	struct GMT_DCW_STATE *GMT_DCW_state = NULL;
	struct GMT_DCW_COUNTRY_STATE *GMT_DCW_country_with_state = NULL;
	if ((list_mode & GMT_DCW_LIST) == 0) return 0;
	if (dcw_load_lists (GMT, &GMT_DCW_country, &GMT_DCW_state, &GMT_DCW_country_with_state, n_bodies)) return 0;	/* Something went wrong */
	GMT_DCW_COUNTRIES = n_bodies[0];
	GMT_DCW_STATES = n_bodies[1];
	GMT_DCW_N_COUNTRIES_WITH_STATES = n_bodies[2];
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "List of ISO 3166-1 alpha-2 codes for DCW supported countries:\n\n");
	for (i = k = 0; i < GMT_DCW_COUNTRIES; i++) {
		if (i == 0 || strcmp (GMT_DCW_country[i].continent, GMT_DCW_country[i-1].continent) )
			printf ("%s [%s]:\n", GMT_DCW_continents[k++], GMT_DCW_country[i].continent);
		printf ("  %s\t%s\n", GMT_DCW_country[i].code, GMT_DCW_country[i].name);
		if ((list_mode & 2) && gmt_dcw_country_has_states (GMT_DCW_country[i].code, GMT_DCW_country_with_state, GMT_DCW_N_COUNTRIES_WITH_STATES)) {
			for (j = 0; j < GMT_DCW_STATES; j++) {
				if (!strcmp (GMT_DCW_country[i].code, GMT_DCW_state[j].country)) printf ("\t\t%s.%s\t%s\n", GMT_DCW_country[i].code, GMT_DCW_state[j].code, GMT_DCW_state[j].name);
			}
		}
	}
	gmt_M_free (GMT, GMT_DCW_country);
	gmt_M_free (GMT, GMT_DCW_state);
	gmt_M_free (GMT, GMT_DCW_country_with_state);
	return ((list_mode & GMT_DCW_LIST));
}

void gmt_DCW_option (struct GMTAPI_CTRL *API, char option, unsigned int plot) {
	/* Show the usage */
	char *action[2] = {"extract", "plot"};
	char *action2[2] = {"extracting", "plotting"};
	if (plot == 1)
		GMT_Message (API, GMT_TIME_NONE, "\t-%c Apply different fill or outline to specified list of countries.\n", option);
	else
		GMT_Message (API, GMT_TIME_NONE, "\t-%c Extract clipping polygons from specified list of countries.\n", option);
	GMT_Message (API, GMT_TIME_NONE, "\t   Based on closed polygons from the Digital Chart of the World (DCW).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append comma-separated list of ISO 3166 codes for countries to %s, i.e.,\n", action[plot]);
	GMT_Message (API, GMT_TIME_NONE, "\t   <code1>,<code2>,... etc., using the 2-character country codes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To select a state of a country (if available), append .state, e.g, US.TX for Texas.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To select a whole continent, use =AF|AN|AS|EU|OC|NA|SA as <code>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +l to just list the countries and their codes [no %s takes place].\n", action2[plot]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +L to see states/territories for Australia, Brazil, Canada, and the US.\n");
	if (plot == 1) {
		GMT_Message (API, GMT_TIME_NONE, "\t   Append +p<pen> to draw outline [none] and +g<fill> to fill [none].\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   One of +p|g must be specified to plot; if -M is in effect we just get the data.\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   Repeat -%c to give different groups of items their own pen/fill settings.\n", option);
	}
}

unsigned int gmt_DCW_parse (struct GMT_CTRL *GMT, char option, char *args, struct GMT_DCW_SELECT *F) {
	/* Parse the F option in pscoast */
	unsigned int n_errors = 0, pos = 0, n;
	char p[GMT_BUFSIZ] = {""}, *c = NULL, *a = NULL, *q = NULL;
	struct GMT_DCW_ITEM *this_item = NULL;

	if ((a = strchr (args, '+'))) a[0] = '\0';	/* Temporarily chop off modifiers */
	this_item = gmt_M_memory (GMT, NULL, 1, struct GMT_DCW_ITEM);	/* New item to fill */
	this_item->codes = strdup (args);
	if (a) a[0] = '+';	/* Reset modifiers */

	/* If +g is used with patterns and +r<dpi> is appended then there is conflict with +r for the deprecated region modification.
	 * We avoid this by checking for this case and replacing +r with @r to avoid the strtok splitting off that modifier. */
	
	if (a && (c = strchr (a, '+'))) {	/* Handle modifiers */
		if ((q = strstr (c, "+g")) && strchr ("Pp", q[2]) && strstr (&q[3], "+r")) {	/* There is a +r<dpi> that follows a +g pattern modifier */
			char *t = &q[3];	/* First character of pattern name or number */
			while (t[0] != '+') t++;	/* Wind to next modifier or run out of chars */
			if (t[0] == '+' && t[1] == 'r') {	/* Found a +r<value> */
				char *r = t++;		/* Now t is at the 'r' */
				t++;	/* Now t is at first char afterwards */
				while (t[0] && isdigit (t[0])) t++;	/* Wind pass all integers */
				if (t[0] == '\0' || t[0] == '+') { /* The modifier could be +r<dpi> or +r<inc>, assume dpi */
					GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Error -%c: Ambiguous modifier +r<val>; could be dpi of the pattern or (a deprecated) region increment - choosing dpi.\n", option);
					GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "If you meant the region modifier then place it before the +g pattern specification.\n", option);
					r[0] = GMT_ASCII_US;	/* Change +r<dpi> to ASCII31<dpi> to pass strtok splitting */
				}
				/* Else it is taken to be a deprecated region increment */
			}
		}
		while ((gmt_strtok (c, "+", &pos, p))) {
			switch (p[0]) {
				/* Listings*/
				case 'R':	/* Get region from polygon(s) BB */
					F->extend = true;
					/* Intentional lack of break to fall through to next case */
				case 'r':	/* Get region from polygon(s) BB */
					F->region = true;
					if (p[1]) {	/* Supplied increments to add or quantize region with */
						F->adjust = true;
						n = GMT_Get_Values (GMT->parent, &p[1], F->inc, 4);
						if (n == 1)	/* Same round in all directions */
							F->inc[XHI] = F->inc[YLO] = F->inc[YHI] = F->inc[XLO];
						else if (n == 2) {	/* Separate round in x and y */
							F->inc[YLO] = F->inc[YHI] = F->inc[XHI];
							F->inc[XHI] = F->inc[XLO];
						}
						else if (n != 4) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Bad number of increment to modifier +%c.\n", option, p[0]);
							n_errors++;
						}
					}
					break;
				case 'l':		/* Country list */
					F->mode = DCW_GET_COUNTRY;
					F->mode |= GMT_DCW_LIST;
					break;
				case 'L': 	/* Country and state list */
					F->mode = DCW_GET_COUNTRY_AND_STATE;
					F->mode |= GMT_DCW_LIST;
					break;
				case 'p':
					if (gmt_getpen (GMT, &p[1], &(this_item->pen))) {	/* Error decoding pen */
						gmt_pen_syntax (GMT, option, NULL, " ", 0);
						n_errors++;
					}
					this_item->mode |= DCW_DO_OUTLINE;
					F->mode |= GMT_DCW_PLOT;
					break;
				case 'g':
					if ((q = strchr (p, GMT_ASCII_US))) q[0] = '+';	/* Restore +r<dpi> */
					if (gmt_getfill (GMT, &p[1], &(this_item->fill))) {
						gmt_fill_syntax (GMT, option, NULL, " ");
						n_errors++;
					}
					this_item->mode |= DCW_DO_FILL;
					F->mode |= GMT_DCW_PLOT;
					break;
				default:
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Unrecognized modifier +%s.\n", option, p);
					n_errors++;
					break;
			}
		}
	}
	if (this_item->codes[0] == '\0' && !(F->mode & (DCW_GET_COUNTRY+DCW_GET_COUNTRY_AND_STATE))) {	/* Gave +l or +L but no codes */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: No country codes given\n", option);
		n_errors++;
	}
	F->item = gmt_M_memory (GMT, F->item, F->n_items+1, struct GMT_DCW_ITEM *);	/* Add one more pointer space to the structure (NULL first time) */
	F->item[F->n_items] = this_item;
	F->n_items++;
	return (n_errors);
}

void gmt_DCW_free (struct GMT_CTRL *GMT, struct GMT_DCW_SELECT *F) {
	/* Free what we might have allocated during parsing */
	unsigned int k;
	if (F->n_items == 0) return;	/* Nothing to free */
	for (k = 0; k < F->n_items; k++) {
		gmt_M_str_free (F->item[k]->codes);
		gmt_M_free (GMT, F->item[k]);
	}
	gmt_M_free (GMT, F->item);
}
