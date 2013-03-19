/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

/* Misc functions to find and read DCW polygons.
 * Only US, Canada, Australia, and Brazil has state borders too.
 * The PUBLIC functions are:
 *
 * GMT_DCW_option	: Present the DCW option and modifier usage
 * GMT_DCW_parse	: Parse the DCW option arguments
 * GMT_DCW_list 	: List the available polygons and exit
 * GMT_DCW_operation	: Get DCW polygons and operate on them
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5.x
 */

struct GMT_DCW_COUNTRY {	/* Information per country */
	char continent[3];
	char code[3];
	char name[80];
};

struct GMT_DCW_STATE {		/* Information per state */
	char country[3];
	char code[3];
	char name[80];
};

static struct GMT_DCW_COUNTRY GMT_DCW_country[GMT_DCW_COUNTRIES] = {
#include "gmt_dcw_countries.h"
};
static struct GMT_DCW_STATE GMT_DCW_states[GMT_DCW_STATES] = {
#include "gmt_dcw_states.h"
};

static char *GMT_DCW_continents[GMT_DCW_N_CONTINENTS] = {"Africa", "Antarctica", "Asia", "Europe", "Oceania", "North America", "South America", "Miscellaneous"};
static char *GMT_DCW_country_with_states[GMT_DCW_N_COUNTRIES_WITH_STATES] = {"AU", "BR", "CA", "US"};

int gmt_dcw_comp_countries (const void *p1, const void *p2)
{
	struct GMT_DCW_COUNTRY *A = (struct GMT_DCW_COUNTRY *)p1;
	struct GMT_DCW_COUNTRY *B = (struct GMT_DCW_COUNTRY *)p2;
	return (strcmp (A->code, B->code));
}

int gmt_dcw_find_country (char *code, struct GMT_DCW_COUNTRY *list, int n)
{	/* Basic binary search for country with given code */
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

int gmt_dcw_find_state (char *scode, char *ccode, struct GMT_DCW_STATE *slist, int ns)
{	/* Return state id given country and state codes */
	int i;
	
	for (i = 0; i < ns; i++) if (!strcmp (scode, slist[i].code) && !strcmp (ccode, slist[i].country)) return (i);
	return (-1);
}

bool gmt_dcw_country_has_states (char *code, char *countries[], int n)
{	/* Return true if this country has state boundaries as well */
	int i;
	for (i = 0; i < n; i++) if (!strcmp (code, countries[i])) return (true);
	return (false);
}

struct GMT_DATASET * GMT_DCW_operation (struct GMT_CTRL *GMT, struct GMT_DCW_SELECT *F, double wesn[], unsigned int mode)
{	/* Given comma-separated names, read the corresponding netCDF files.
 	 * mode = GMT_DCW_REGION	: Return the joint w/e/s/n limits
	 * mode = GMT_DCW_PLOT		: Plot the polygons
	 * mode = GMT_DCW_DUMP		: Dump the polygons
	 * mode = GMT_DCW_EXTRACT	: Return a dataset structure
	 */
	int ks, j, retval, ncid, xvarid, yvarid, id, first, last;
	size_t np, max_np = 0U;
	uint64_t seg, n_segments;
	unsigned int k, n_items = 1, pos = 0, kk, tbl;
	unsigned short int *dx = NULL, *dy = NULL;
	bool done, want_state, outline = (F->mode & 4), fill = (F->mode & 8);
	char code[GMT_TEXT_LEN16], state[GMT_TEXT_LEN16], file[GMT_TEXT_LEN16], msg[GMT_BUFSIZ], path[GMT_BUFSIZ];
	double west, east, south, north, xscl, yscl, out[2], *lon = NULL, *lat = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *P = NULL, *S = NULL;
	
	if (!F->codes || F->codes[0] == '\0') return NULL;
	
	GMT_getsharepath (GMT, "coast", "CDW", "", path);
	if (GMT_access (GMT, path, F_OK)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "The DCW country polygons are not available.  Download the file\n");
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "http://www.soest.hawaii.edu/gmt/gmt-dcw.zip and unzip in your\n");
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT5 installation's share/coast directory.\n");
		return NULL;
	}
	
	for (k = 0; k < strlen (F->codes); k++) if (F->codes[k] == ',') n_items++;

	qsort ((void *)GMT_DCW_country, (size_t)GMT_DCW_COUNTRIES, sizeof (struct GMT_DCW_COUNTRY), gmt_dcw_comp_countries);	/* Sort on country code */
 	
	if (mode & GMT_DCW_REGION) {	/* Determine region from polygons */
		if (wesn == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Must pass wesn array if mode == 0\n");
			return NULL;
		}
		wesn[XLO] = wesn[YLO] = +9999.0;	wesn[XHI] = wesn[YHI] = -9999.0;
	}
	if (mode > GMT_DCW_REGION) {
		P = GMT_memory (GMT, NULL, 1, struct GMT_DATASEGMENT);
		GMT_alloc_segment (GMT, P, 0, 2, true);
		if (mode & GMT_DCW_PLOT) {
			if (outline) GMT_setpen (GMT, &F->pen);
			if (fill) GMT_setfill (GMT, &F->fill, outline);
		}
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Extract polygons from DCW - The Digital Chart of the World\n");
	}
	
	if (mode & GMT_DCW_EXTRACT) {
		uint64_t dim[4] = {n_items, 0, 2, 0};
		if ((D = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to create empty dataset for DCW polygons\n");
			return NULL;
		}
	}
	while (GMT_strtok (F->codes, ",", &pos, code)) {	/* Loop over countries */
		want_state = false;
		if (code[2] == '.') {	/* Requesting a state */
			GMT_memset (state, GMT_TEXT_LEN16, char);
			strcpy (state, &code[3]);
			code[2] = '\0';
			want_state = true;
		}
		ks = gmt_dcw_find_country (code, GMT_DCW_country, GMT_DCW_COUNTRIES);
		if (ks == -1) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No country code matching %s (skipped)\n", code);
			continue;
		}
		k = ks;
		if (want_state) {
			if ((j = gmt_dcw_find_state (state, code, GMT_DCW_states, GMT_DCW_STATES)) == -1) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Country %s does not have states (skipped)\n", code);
				continue;
			}
			sprintf (file, "%s/%s/%s", GMT_DCW_country[k].continent, GMT_DCW_country[k].code, GMT_DCW_states[j].code);
		}
		else
			sprintf (file, "%s/%s", GMT_DCW_country[k].continent, GMT_DCW_country[k].code);
			
		GMT_getsharepath (GMT, "coast/DCW", file, ".nc", path);

		if ((retval = nc_open (path, NC_NOWRITE, &ncid))) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot open file %s!\n", path);
			continue;
		}
		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE) || mode == 2) {
			if (want_state) 
				sprintf (msg, "Extract data for %s (%s)\n", GMT_DCW_states[j].name, GMT_DCW_country[k].name);
			else
				sprintf (msg, "Extract data for %s\n", GMT_DCW_country[k].name);
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, msg);
			k = strlen (msg) - 1;
			msg[k] = '\0';
		}
		
		/* Open and read the netCDF file */
		
		if ((retval = nc_inq_dimid (ncid, "time", &id))) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error processing %s!\n", path);
			continue;
		}
		retval = nc_inq_dimlen (ncid, id, &np);
		
		if (mode > GMT_DCW_REGION && np > max_np) {
			GMT_malloc2 (GMT, lon, lat, np, NULL, double);
			GMT_malloc2 (GMT, dx, dy, np, NULL, unsigned short int);
			max_np = np;
		}
		
	        /* Get the varid of the lon and lat variables, based on their names, and get the data */

		if ((retval = nc_inq_varid (ncid, "lon", &xvarid))) continue;
		if ((retval = nc_get_att_double (ncid, xvarid, "min", &west))) continue;
		if ((retval = nc_get_att_double (ncid, xvarid, "max", &east))) continue;
		if ((retval = nc_get_att_double (ncid, xvarid, "scale", &xscl))) continue;
		if ((retval = nc_inq_varid (ncid, "lat", &yvarid))) continue;
		if ((retval = nc_get_att_double (ncid, yvarid, "min", &south))) continue;
		if ((retval = nc_get_att_double (ncid, yvarid, "max", &north))) continue;
		if ((retval = nc_get_att_double (ncid, yvarid, "scale", &yscl))) continue;
		if (mode & GMT_DCW_REGION) {	/* Just update wesn */
			if (west < wesn[XLO])  wesn[XLO] = west;
			if (east > wesn[XHI])  wesn[XHI] = east;
			if (south < wesn[YLO]) wesn[YLO] = south;
			if (north > wesn[YHI]) wesn[YHI] = north;
		}
		if (mode > GMT_DCW_REGION) {	/* Need to read the data arrays */
			if ((retval = nc_get_var_ushort (ncid, xvarid, dx))) continue;
			if ((retval = nc_get_var_ushort (ncid, yvarid, dy))) continue;
		}
		if ((retval = nc_close (ncid))) continue;
		if (mode == GMT_DCW_REGION) continue;
		xscl = 1.0 / xscl;	yscl = 1.0 / yscl;
		for (k = n_segments = 0; k < np; k++) {	/* Unpack */
			if (dx[k] == 65535U) n_segments++;	/* Count how many segments */
			lon[k] = (dx[k] == 65535U) ? 0.0 : dx[k] * xscl + west;
			lat[k] = (dy[k] == 65535U) ? 0.0 : dy[k] * yscl + south;
		}
		if (mode & GMT_DCW_EXTRACT) {	/* ALlocate a table with the right number of segments */
			D->table[tbl] = GMT_create_table (GMT, n_segments, 2, 0, false);
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
			P->coord[GMT_X] = &lon[first];
			P->coord[GMT_Y] = &lat[first];
			if (mode & GMT_DCW_DUMP) {	/* Dump the coordinates to stdout */
				strcpy (GMT->current.io.segment_header, msg);
				GMT_Put_Record (GMT->parent, GMT_WRITE_SEGMENT_HEADER, NULL);
				for (kk = 0; kk < P->n_rows; kk++) {
					out[GMT_X] = P->coord[GMT_X][kk];
					out[GMT_Y] = P->coord[GMT_Y][kk];
					GMT_Put_Record (GMT->parent, GMT_WRITE_DOUBLE, out);
				}
			}
			else if (mode & GMT_DCW_EXTRACT) {	/* Attach to dataset */
				S = D->table[tbl]->segment[seg];
				S->n_rows = P->n_rows;
				GMT_malloc2 (GMT, S->coord[GMT_X], S->coord[GMT_Y], S->n_rows, NULL, double);
				GMT_memcpy (S->coord[GMT_X], lon, S->n_rows, double);
				GMT_memcpy (S->coord[GMT_Y], lat, S->n_rows, double);
				seg++;
			}
			else {	/* mdoe & GMT_DCW_PLOT: Plot this piece */
				if (fill)	/* Plot filled polygon, w/ or w/o outline */
					GMT_geo_polygons (GMT, P);
				else {	/* Plot outline only */
					if ((GMT->current.plot.n = GMT_geo_to_xy_line (GMT, P->coord[GMT_X], P->coord[GMT_Y], P->n_rows)) == 0) continue;
					GMT_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n);
				}
			}
		}
		tbl++;
	}
	if (mode & GMT_DCW_REGION) {
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
			/* Do basic sanity checks */
			if (wesn[YLO] < -90.0) wesn[YLO] = -90.0;
			if (wesn[YHI] > +90.0) wesn[YHI] = +90.0;
			if (fabs (wesn[XHI] - wesn[XLO]) > 360.0) {
				wesn[XLO] = 0.0;
				wesn[XHI] = 360.0;
			}
		}
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Region implied by DCW polygons is %g/%g/%g/%g\n", wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
	}
	if (mode > GMT_DCW_REGION) {
		GMT_free (GMT, dx);
		GMT_free (GMT, dy);
		GMT_free (GMT, lon);
		GMT_free (GMT, lat);
		P->coord[GMT_X] = P->coord[GMT_Y] = NULL;
		GMT_free_segment (GMT, P);
	}
	return (D);
}

unsigned int GMT_DCW_list (struct GMT_CTRL *GMT, unsigned list_mode)
{	/* List available countries [and optionally states]; then make program exit */
	unsigned int i, j, k;
	if ((list_mode & 3) == 0) return 0;
	for (i = k = 0; i < GMT_DCW_COUNTRIES; i++) {
		if (i == 0 || strcmp (GMT_DCW_country[i].continent, GMT_DCW_country[i-1].continent) ) {
			GMT_Message (GMT->parent, GMT_TIME_NONE, "%s [%s]:\n", GMT_DCW_continents[k++], GMT_DCW_country[i].continent);
		}
		printf ("  %s\t%s\n", GMT_DCW_country[i].code, GMT_DCW_country[i].name);
		if ((list_mode & 2) && gmt_dcw_country_has_states (GMT_DCW_country[i].code, GMT_DCW_country_with_states, GMT_DCW_N_COUNTRIES_WITH_STATES)) {
			for (j = 0; j < GMT_DCW_STATES; j++) {
				if (!strcmp (GMT_DCW_country[i].code, GMT_DCW_states[j].country)) GMT_Message (GMT->parent, GMT_TIME_NONE, "\t\t%s.%s\t%s\n", GMT_DCW_country[i].code, GMT_DCW_states[j].code, GMT_DCW_states[j].name);
			}
		}
	}
	return ((list_mode & 3));
}

void GMT_DCW_option (struct GMTAPI_CTRL *API, char option, unsigned int plot)
{	/* Show the usage */
	char *action[2] = {"extract", "plot"};
	if (plot == 1)
		GMT_Message (API, GMT_TIME_NONE, "\t-%c Apply different fill or outline to specified list of countries.\n", option);
	else
		GMT_Message (API, GMT_TIME_NONE, "\t-%c Extract clipping polygons from specified list of countries.\n", option);
	GMT_Message (API, GMT_TIME_NONE, "\t   Based on closed polygons from the Digital Chart of the World (DCW).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append comma-separated list of codes for countries to plot, i.e.,\n", action[plot]);
	GMT_Message (API, GMT_TIME_NONE, "\t   <code1>,<code2>,... etc., using the 2-character country codes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To select a state of a country (if available), append .state, e.g, US.TX for Texas.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +l to just list the countries and their codes [no %sing takes place].\n", action[plot]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +L to see states/terretories for Australia, Brazil, Canada, and the US.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +r obtain -Rw/e/s/n from polygon(s). Append <inc>, <xinc>/<yinc>, or <winc>/<einc>/<sinc>/<ninc>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     for a region in these multiples [none].  Use +R to extend region by increments instead [0]\n");
	if (plot == 1) {
		GMT_Message (API, GMT_TIME_NONE, "\t   Append +p<pen> to draw outline [none] and +f<fill> to fill [none].\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   One of +p|f must be specified unless -M is in effect.\n");
	}
}

unsigned int GMT_DCW_parse (struct GMT_CTRL *GMT, char option, char *args, struct GMT_DCW_SELECT *F)
{	/* Parse the F option in pscoast */
	unsigned int n_errors = 0, pos = 0, n;
	char p[GMT_BUFSIZ], *c = NULL, *a = NULL;
		
	if ((a = strchr (args, '+'))) a[0] = '\0';	/* Temporarily chop off modifiers */
	F->codes = strdup (args);
	a[0] = '+';	/* Reset modifiers */
	
	if ((c = strchr (a, '+'))) {	/* Handle modifiers */
		while ((GMT_strtok (c, "+", &pos, p))) {
			switch (p[0]) {
				/* Listings*/
				case 'R':	/* Get region from polygon(s) BB */
					F->extend = true;
				case 'r':	/* Get region from polygon(s) BB */
					F->region = true;
					if (p[1]) {	/* Supplied increments to add or quantize region with */
						F->adjust = true;
						n = GMT_Get_Value (GMT->parent, &p[1], F->inc);
						if (n == 1)	/* Same round in all directions */
							F->inc[XHI] = F->inc[YLO] = F->inc[YHI] = F->inc[XLO];
						else if (n == 2) {	/* Separate round in x and y */
							F->inc[YLO] = F->inc[YHI] = F->inc[XHI];
							F->inc[XHI] = F->inc[XLO];
						}
						else if (n != 4){
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Bad number of increment to modifier +%c.\n", option, p[0]);
							n_errors++;
						}
					}
					break;
				case 'l':  F->mode = 1;  break;	/* Country list */
				case 'L':  F->mode = 2;  break;	/* Country and state list */
				case 'p':
					if (GMT_getpen (GMT, &p[1], &F->pen)) {	/* Error decoding pen */
						GMT_pen_syntax (GMT, 'F', " ");
						n_errors++;
					}
					F->mode |= 4;
					break;
				case 'f':
					if (GMT_getfill (GMT, &p[1], &F->fill)) {
						GMT_fill_syntax (GMT, 'F', " ");
						n_errors++;
					}
					F->mode |= 8;
					break;
				default: 
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -%c: Unrecognized modifier +%s.\n", option, p);
					n_errors++;
					break;
			}
		}
	}
	return (n_errors);
}
