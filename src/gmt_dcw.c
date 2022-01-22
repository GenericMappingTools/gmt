/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * We also look for dcw-collections.txt for named groups of items (2.0.2 or later)
 * in the DCW directory and in the users .gmt directory.
 * See separate subversion project DCW for the maintenance of the raw files that
 * are used to build the netCDF file [svn://gmtserver.soest.hawai.edu/DCW].
 */

#define DCW_SITE 			"ftp://ftp.soest.hawaii.edu/gmt"
#define GMT_DCW_N_CONTINENTS		8

#define DCW_GET_COUNTRY			1	/* Extract countries only */
#define DCW_GET_COUNTRY_AND_STATE	2	/* Extract countries and states */
#define DCW_GET_COLLECTIONS	4	/* Extract Collections */
#define DCW_DO_OUTLINE			1	/* Draw outline of polygons */
#define DCW_DO_FILL				2	/* Fill the polygons */
#define DCW_NAMED_LIST			0	/* A lsit of DCW items that is given a name */
#define DCW_NAMED_WESN			1	/* A w/e/s/n area that is given a name */

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

struct GMT_DCW_COLLECTION {		/* Information of collections representing multiple selections */
	unsigned int type;			/* 0 is DCW list, 1 is named w/e/s/n */
	char region[GMT_LEN16];		/* Up to 16-character collections, e.g., SCAND for Scandinavia */
	char name[GMT_LEN64];		/* Up to 64-character optional full name, e.g., Scandinavia */
	char *wesn_string;			/* The original w/e/s/n string in the collections file */
	char *list;					/* Comma-separated list of entities making up the collection */
	double wesn[4];				/* Extent of named regions */
};

/* For version 2.0.0 we follow https://en.wikipedia.org/wiki/ISO_3166-2:CN and use 2-char instead of int codes */
#define DCW_N_CHINA_PROVINCES	34
struct GMT_DCW_CHINA_CODES {
	unsigned int id;
	char code[4];
};

/* Compile in read-only structures and arrays with the information */

static char *GMT_DCW_continents[GMT_DCW_N_CONTINENTS] = {"Africa", "Antarctica", "Asia", "Europe", "Oceania", "North America", "South America", "Miscellaneous"};

/* Local functions only visible inside this file */

GMT_LOCAL bool gmtdcw_get_path (struct GMT_CTRL *GMT, char *name, char *suffix, char *path) {
	/* This is the order of checking:
	 * 1. Check in GMT->session.DCWDIR, if set
	 * 2. Look via gmt_getsharepath.
	 * 3. Look in userdir/geography/dcw
	 * 4. Try to download from server into (3)
	 * 5. Give up.
	 */

	if (GMT->session.DCWDIR) {	/* 1. Check in GMT->session.DCWDIR */
		sprintf (path, "%s/%s%s", GMT->session.DCWDIR, name, suffix);
		if (access (path, R_OK) == 0) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "1. DCW: Read the Digital Chart of the World from %s\n", path);
			return true;
		}
		/* Failed, so remove reference to invalid GMT->session.DCWDIR but don't free
		 * the pointer. this is no leak because the reference still exists
		 * in the previous copy of the current GMT_CTRL struct. */
		GMT->session.DCWDIR = NULL;
	}
	if (gmt_getsharepath (GMT, "dcw", name, suffix, path, R_OK)) {
		if (access (path, R_OK) == 0) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "2. DCW: Read the Digital Chart of the World from %s\n", path);
			return true;	/* Found it in share or user somewhere */
		}
	}
	if (GMT->session.USERDIR) {	/* Check user dir via remote download */
		char remote_path[PATH_MAX] = {""};
		sprintf (path, "%s/geography/dcw/%s%s", GMT->session.USERDIR, name, suffix);
		if (access (path, R_OK) == 0) {	/* Previously downloaded */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "3. DCW: Read the Digital Chart of the World from %s\n", path);
			return true;	/* Found it here */
		}
		/* Must download it the first time */
		if (GMT->current.setting.auto_download == GMT_NO_DOWNLOAD) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to download the Digital Chart of the World for GMT since GMT_DATA_UPDATE_INTERVAL is off\n");
			return false;
		}
		sprintf (path, "%s/geography/dcw", GMT->session.USERDIR);	/* Local directory destination */
		if (access (path, R_OK) && gmt_mkdir (path)) {	/* Must first create the directory */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create GMT directory : %s\n", path);
			return false;
		}
		sprintf (path, "%s/geography/dcw/%s%s", GMT->session.USERDIR, name, suffix);	/* Final local path */
		snprintf (remote_path, PATH_MAX, "%s/geography/dcw/%s%s", gmt_dataserver_url (GMT->parent), name, suffix);	/* Unique remote path */
		GMT_Report (GMT->parent, GMT_MSG_NOTICE, "Downloading %s%s for the first time - be patient\n", name, suffix);
		if (gmt_download_file (GMT, name, remote_path, path, true)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to obtain remote file %s%s\n", name, suffix);
			return false;
		}
		else {	/* Successfully downloaded the first time */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "3. DCW: Read the Digital Chart of the World from %s\n", path);
			return true;
		}
	}
	return (false);
}

GMT_LOCAL void gmtdcw_freestrings (struct GMT_DCW_COLLECTION *Collection, unsigned int n) {
	unsigned int k;
	for (k = 0; k < n; k++) {
		if (Collection[k].list) gmt_M_str_free (Collection[k].list);
		if (Collection[k].wesn_string) gmt_M_str_free (Collection[k].wesn_string);
	}
}

GMT_LOCAL int gmtdcw_load_lists (struct GMT_CTRL *GMT, struct GMT_DCW_COUNTRY **C, struct GMT_DCW_STATE **S, struct GMT_DCW_COUNTRY_STATE **CS, struct GMT_DCW_COLLECTION **U, unsigned int dim[]) {
	/* Open and read list of countries and states and return via two struct and one char arrays plus dimensions in dim */
	size_t n_alloc = GMT_LEN256;
	unsigned int j, k, ns, collection, n_errors;
	int nf;
	char path[PATH_MAX] = {""}, line[BUFSIZ] = {""};
	char w[GMT_LEN32] = {""}, e[GMT_LEN32] = {""}, s[GMT_LEN32] = {""}, n[GMT_LEN32] = {""};
	FILE *fp = NULL;
	struct GMT_DCW_COUNTRY *Country = NULL;
	struct GMT_DCW_STATE *State = NULL;
	struct GMT_DCW_COUNTRY_STATE *Country_State = NULL;
	struct GMT_DCW_COLLECTION *Collection = NULL;

	if (!gmtdcw_get_path (GMT, "dcw-countries", ".txt", path)) return GMT_NOTSET;

	/* Get countries first */
	if ((fp = fopen (path, "r")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to open file %s [permission trouble?]\n", path);
		return GMT_NOTSET;
	}
	Country = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_DCW_COUNTRY);
	k = 0;
	while ( gmt_fgets (GMT, line, BUFSIZ, fp)) {
		if (line[0] == '#') continue;	/* Skip comments */
		sscanf (line, "%s %s %[^\n]", Country[k].continent, Country[k].code,  Country[k].name);
		k++;
		if (k == n_alloc) {
			n_alloc += GMT_LEN128;
			Country = gmt_M_memory (GMT, Country, n_alloc, struct GMT_DCW_COUNTRY);
		}
	}
	fclose (fp);
	dim[0] = k;	/* Number of countries */
	Country = gmt_M_memory (GMT, Country, k, struct GMT_DCW_COUNTRY);

	/* Get states */
	if (!gmtdcw_get_path (GMT, "dcw-states", ".txt", path)) {
		gmt_M_free (GMT, Country);
		return GMT_NOTSET;
	}
	if ((fp = fopen (path, "r")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to open file %s [permission trouble?]\n", path);
		gmt_M_free (GMT, Country);
		return GMT_NOTSET;
	}
	State = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_DCW_STATE);
	k = 0;	ns = 1;
	while ( gmt_fgets (GMT, line, BUFSIZ, fp)) {
		if (line[0] == '#') continue;	/* Skip comments */
		sscanf (line, "%s %s %[^\n]", State[k].country, State[k].code,  State[k].name);
		if (k && strcmp (State[k].country, State[k-1].country)) ns++;	/* New country with states */
		k++;
		if (k == n_alloc) {
			n_alloc += GMT_LEN128;
			State = gmt_M_memory (GMT, State, n_alloc, struct GMT_DCW_STATE);
		}
	}
	fclose (fp);
	dim[1] = k;	/* Number of states */
	State = gmt_M_memory (GMT, State, k, struct GMT_DCW_STATE);

	/* Get list of countries with states */

	dim[2] = ns;	/* Number of countries with states */
	if (CS) {	/* Wants list returned */
		Country_State = gmt_M_memory (GMT, NULL, ns, struct GMT_DCW_COUNTRY_STATE);
		gmt_M_memcpy (Country_State[0].country, State[0].country, 4, char);
		for (k = ns = 1; k < dim[1]; k++) {
			if (strcmp (State[k].country, State[k-1].country)) gmt_M_memcpy (Country_State[ns++].country, State[k].country, 4, char);
		}
		*CS = Country_State;
	}

	/* Get collections (which may not be present, so allow for missing files */

	Collection = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_DCW_COLLECTION);
	k = 0;
	for (collection = 0; collection < 2; collection++) {	/* Read both system and user collections */
		if (collection == 0) {	/* Look for DCW conf file first so it can override anything in the system file */
			if (!gmtlib_getuserpath (GMT, "dcw.conf", path))
				continue;
		}
		else {	/* Look for the system DCW collection file second */
			if (!gmtdcw_get_path (GMT, "dcw-collections", ".txt", path))
				continue;
		}
		/* Here we got the requested path */
		dim[3+collection] = 0;	/* Number of collections in this set */
		if ((fp = fopen (path, "r")) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to open file %s [permission trouble?]\n", path);
			goto bail;
		}
		/* File format is sequences like this:
			# Scandinavia
			tag: SCA Scandinavia
			list: NO,SE,DK or region: w/e/s/n
		*/
		while (gmt_fgets (GMT, line, BUFSIZ, fp)) {
			if (line[0] == '#') continue;	/* Skip comments */
			if (gmt_is_a_blank_line (line)) continue;
			if (strncmp (line, "tag:", 4U)) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Collection file %s is not in correct format - exit\n", path);
				goto bail;
			}
			nf = sscanf (&line[5], "%s %[^\n]", Collection[k].region, Collection[k].name);
			if (nf == 2 && !isalpha (Collection[k].name[0])) {	/* Gave a name that do not start with a letter */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Collection name must start with a letter (%s)\n", (Collection[k].name));
				goto bail;
			}
			/* Get record with the list or region */
			if (gmt_fgets (GMT, line, BUFSIZ, fp) == NULL) {	/* Read error */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to read list or region for collection %s\n", (Collection[k].region));
				goto bail;
			}
			gmt_chop (line);	/* Eliminate trailing \r \n stuff */
			if (strncmp (line, "list:", 5U) == 0) {	/* Got a list of DCW entities */
				for (j = 5; line[j] == '\t' || line[j] == ' '; j++);	/* Wind past optional leading whitespace */
				Collection[k].list = strdup (&line[j]);
			}
			else if (strncmp (line, "region:", 7U) == 0) {	/* Got w/e/s/n for a named region */
				for (j = 7; line[j] == '\t' || line[j] == ' '; j++);	/* Wind past optional leading whitespace */
				Collection[k].wesn_string = strdup (&line[j]);	/* Copy of original w/e/s/n string */
				gmt_strrepc (line, '/', ' ');	/* Replace slashes with spaces to ease parsing */
				n_errors = 0;
				if (sscanf (&line[j], "%s %s %s %s", w, e, s, n) != 4) n_errors++;
				n_errors += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf_arg (GMT, w, GMT_IS_LON, false, &Collection[k].wesn[XLO]), w);
				n_errors += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf_arg (GMT, e, GMT_IS_LON, false, &Collection[k].wesn[XHI]), e);
				n_errors += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf_arg (GMT, s, GMT_IS_LAT, false, &Collection[k].wesn[YLO]), s);
				n_errors += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf_arg (GMT, n, GMT_IS_LAT, false, &Collection[k].wesn[YHI]), n);
				if (Collection[k].wesn[YHI] <= Collection[k].wesn[YLO]) n_errors++;	/* Basic sanity checks of the region values */
				if (Collection[k].wesn[XHI] <= Collection[k].wesn[XLO]) n_errors++;
				if (n_errors) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Collection file %s has incomplete, malformed or incorrect w/e/s/n region: %s\n", Collection[k].wesn_string);
					goto bail;
				}
				else	/* Set the collection type */
					Collection[k].type = DCW_NAMED_WESN;
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Collection file %s is not in correct format - exit\n", path);
				goto bail;
			}
			k++;
			dim[3+collection]++;
			if (k == n_alloc) {
				size_t old_n_alloc = n_alloc;
				n_alloc += GMT_LEN128;
				Collection = gmt_M_memory (GMT, Collection, n_alloc, struct GMT_DCW_COLLECTION);
				/* Reallocation does not initialize new memory to NULL so we must do so here manually */
				gmt_M_memset (&(Collection[old_n_alloc]), n_alloc - old_n_alloc, struct GMT_DCW_COLLECTION);	/* Set to NULL/0 */
			}
		}
		fclose (fp);
	}
	if (k == 0)	/* No collection files at all */
		gmt_M_free (GMT, Collection);
	else
		Collection = gmt_M_memory (GMT, Collection, k, struct GMT_DCW_COLLECTION);

	*C = Country;
	*S = State;
	*U = Collection;

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "DCW: Found %u countries, %u countries with states, %u states, %d DCW collections and %d user groups\n", dim[0], dim[2], dim[1], dim[3], dim[4]);
	return 0;

bail:
	gmt_M_free (GMT, Country);
	gmt_M_free (GMT, State);
	gmt_M_free (GMT, Country_State);
	gmtdcw_freestrings (Collection, dim[3]+dim[4]);
	gmt_M_free (GMT, Collection);

	return GMT_NOTSET;
}

GMT_LOCAL int gmtdcw_comp_countries (const void *p1, const void *p2) {
	/* Used to sort countries alphabetically */
	struct GMT_DCW_COUNTRY *A = (struct GMT_DCW_COUNTRY *)p1;
	struct GMT_DCW_COUNTRY *B = (struct GMT_DCW_COUNTRY *)p2;
	return (strcmp (A->code, B->code));
}

GMT_LOCAL int gmtdcw_find_country (char *code, struct GMT_DCW_COUNTRY *list, int n) {
	/* Basic binary search for country with given code and an alphabetically sorted list */
	int low = 0, high = n, mid, last = GMT_NOTSET, way;

	while (low < high) {
		mid = (low + high) / 2;
		if (mid == last) return (GMT_NOTSET);	/* No such code */
		way = strcmp (code, list[mid].code);
		if (way > 0) low = mid;
		else if (way < 0) high = mid;
		else return (mid);
		last = mid;
	}
	return (low);
}

GMT_LOCAL bool gmtdcw_got_name (char *name, struct GMT_DCW_COUNTRY *clist, int nc, struct GMT_DCW_STATE *slist, int ns, char *ISO) {
	/* Slow linear search to find ISO that matches the given country name */
	int k;
	for (k = 0; k < nc; k++) {
		if (strcasecmp (name, clist[k].name) == 0) {
			strncpy (ISO, clist[k].code, 2); ISO[2] = '\0';	/* Terminate string */
			return (true);
		}
	}
	/* Got here because no such country.  Try states */
	for (k = 0; k < ns; k++) {
		if (strcasecmp (name, slist[k].name) == 0) {
			sprintf (ISO, "%s.%s%c", slist[k].country, slist[k].code, '\0');
			return (true);
		}
	}
	return (false);	/* Not found */
}

GMT_LOCAL int gmtdcw_find_state (struct GMT_CTRL *GMT, char *scode, char *ccode, struct GMT_DCW_STATE *slist, int ns, bool check) {
	/* Return state id given country and state codes using a linear search */
	int i;
	static struct GMT_DCW_CHINA_CODES gmtdcw_CN_codes[DCW_N_CHINA_PROVINCES] = {
		{11, "BJ"}, {12, "TJ"}, {13, "HE"}, {14, "SX"}, {15, "NM"}, {21, "LN"}, {22, "JL"}, {23, "HL"},
		{31, "SH"}, {32, "JS"}, {33, "ZJ"}, {34, "AH"}, {35, "FJ"}, {36, "JX"}, {37, "SD"}, {41, "HA"},
		{42, "HB"}, {43, "HN"}, {44, "GD"}, {45, "GX"}, {46, "HI"}, {50, "CQ"}, {51, "SC"}, {52, "GZ"},
		{53, "YN"}, {54, "XZ"}, {61, "SN"}, {62, "GS"}, {63, "QH"}, {64, "NX"}, {65, "XJ"}, {71, "TW"},
		{91, "HK"}, {92, "MO"}
	};
	if (check && !strncmp (ccode, "CN", 2U) && isdigit (scode[0])) {	/* Must switch to 2-character code */
		unsigned int k = 0, id = atoi (scode);
		while (k < DCW_N_CHINA_PROVINCES && id > gmtdcw_CN_codes[k].id) k++;
		if (k == DCW_N_CHINA_PROVINCES) return (GMT_NOTSET);	/* No such integer ID found in the list */
		if (id < gmtdcw_CN_codes[k].id) return (GMT_NOTSET);	/* No such integer ID found in the list */
		GMT_Report (GMT->parent, GMT_MSG_NOTICE, "FYI, Chinese province code %d is deprecated. Use %s instead\n", id, gmtdcw_CN_codes[k].code);
		scode = gmtdcw_CN_codes[k].code;
	}
	for (i = 0; i < ns; i++) if (!strcmp (scode, slist[i].code) && !strcmp (ccode, slist[i].country)) return (i);
	return (GMT_NOTSET);
}

GMT_LOCAL bool gmtdcw_country_has_states (char *code, struct GMT_DCW_COUNTRY_STATE *st_country, unsigned int n) {
	/* Return true if this country has interior state boundaries */
	unsigned int i;
	for (i = 0; i < n; i++) if (!strcmp (code, st_country[i].country)) return (true);
	return (false);
}

GMT_LOCAL int gmtdcw_found_collection (char *code, struct GMT_DCW_COLLECTION *GMT_DCW_collection, unsigned int *dim) {
	int k, nc = dim[3] + dim[4];	/* Total number of collections */
	if (nc == 0) return GMT_NOTSET;	/* No collections available */

	for (k = 0; k < nc; k++) {
		if (strcasecmp (code, GMT_DCW_collection[k].region) == 0 || (GMT_DCW_collection[k].name[0] && strcasecmp (code, GMT_DCW_collection[k].name) == 0))
			return (k);	/* Found a matching collection */
	}
	return (GMT_NOTSET);	/* Not found */
}

/*----------------------------------------------------------|
 * Public functions that are part of the GMT Devel library  |
 *----------------------------------------------------------|
 */

#define GMT_DCW_PLOTTING	1
#define GMT_DCW_CLIPPING	2

int gmt_DCW_version (struct GMTAPI_CTRL *API, char *version) {
	/* Write DCW version into version which must have at least 8 positions */
	int cdfid, err;
	char path[PATH_MAX] = {""};
	struct GMT_CTRL *GMT = API->GMT;
	
	if (version == NULL)
		return (GMT_PTR_IS_NULL);

	if (!gmtdcw_get_path (GMT, "dcw-gmt", ".nc", path))
		return (GMT_FILE_NOT_FOUND);

	gmt_M_err_trap (gmt_nc_open (GMT, path, NC_NOWRITE, &cdfid));

	/* Get global attributes */
	gmt_M_memset (version, strlen (version), char);
	gmt_M_err_trap (nc_get_att_text (cdfid, NC_GLOBAL, "version", version));
	gmt_nc_close (GMT, cdfid);

	return (GMT_NOERROR);
}

struct GMT_DATASET * gmt_DCW_operation (struct GMT_CTRL *GMT, struct GMT_DCW_SELECT *F, double wesn[], unsigned int mode) {
	/* Given comma-separated names, read the corresponding netCDF variables.
 	 * mode = GMT_DCW_REGION	: Return the joint w/e/s/n limits
	 * mode = GMT_DCW_PLOT		: Plot the polygons [This is actually same as GMT_DCW_EXTRACT internally but plots instead of returning]
	 * mode = GMT_DCW_DUMP		: Dump the polygons
	 * mode = GMT_DCW_EXTRACT	: Return a dataset structure
	 */
	int item, ks, retval, ncid, xvarid, yvarid, id;
	int64_t first, last;
	size_t np, max_np = 0U, n_alloc = GMT_LEN128;
	uint64_t k, seg, n_segments;
	unsigned int n_items = 0, r_item = 0, pos = 0, kk, tbl = 0, j = 0, named_wesn = 0, *order = NULL;
	unsigned short int *dx = NULL, *dy = NULL;
	unsigned int GMT_DCW_COUNTRIES = 0, GMT_DCW_STATES = 0, n_bodies[5] = {0, 0, 0, 0, 0}, special = 0;
	bool done, want_state, outline, fill = false, is_Antarctica = false, hole, new_CN_codes = false;
	char TAG[GMT_LEN16] = {""}, dim[GMT_LEN16] = {""}, xname[GMT_LEN16] = {""};
	char yname[GMT_LEN16] = {""}, code[GMT_LEN16] = {""}, state[GMT_LEN16] = {""};
	char msg[GMT_BUFSIZ] = {""}, path[PATH_MAX] = {""}, list[GMT_BUFSIZ] = {""};
	char version[GMT_LEN32] = {""}, gmtversion[GMT_LEN32] = {""}, source[GMT_LEN256] = {""}, title[GMT_LEN256] = {""};
	char label[GMT_LEN256] = {""}, header[GMT_LEN256] = {""}, ISO[GMT_LEN8] = {""};
	double west, east, south, north, xscl, yscl, out[2], *lon = NULL, *lat = NULL;
	struct GMT_RANGE *Z = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *P = NULL, *S = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMT_DCW_COUNTRY *GMT_DCW_country = NULL;
	struct GMT_DCW_STATE *GMT_DCW_state = NULL;
	struct GMT_DCW_COLLECTION *GMT_DCW_collection = NULL;
	struct GMT_FILL *sfill = NULL;
	struct GMT_PEN *spen = NULL;

	for (j = ks = 0; j < F->n_items; j++) {
		if (!F->item[j]->codes || F->item[j]->codes[0] == '\0') continue;
		ks++;	/* Gave some codes */
	}
	if (ks == 0) return NULL;	/* No countries requested */
	if (mode != GMT_DCW_REGION && F->region && (mode & (GMT_DCW_PLOT+GMT_DCW_DUMP+GMT_DCW_EXTRACT)) == 0) return NULL;	/* No plotting/dumping requested, just -R */

	if (mode & GMT_DCW_REGION) {	/* Wish to determine region from polygons */
		if (wesn == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Implementation error: Must pass wesn array if mode == %d\n", GMT_DCW_REGION);
			return NULL;
		}
		wesn[XLO] = wesn[XHI] = 0.0;			/* Set to zero so it can grow below */
		wesn[YLO] = +9999.0;	wesn[YHI] = -9999.0;	/* Initialize so we can shrink it below */
	}

	if (gmtdcw_load_lists (GMT, &GMT_DCW_country, &GMT_DCW_state, NULL, &GMT_DCW_collection, n_bodies))	/* Something went wrong */
		return NULL;

	GMT_DCW_COUNTRIES = n_bodies[0];
	GMT_DCW_STATES = n_bodies[1];

	qsort ((void *)GMT_DCW_country, (size_t)GMT_DCW_COUNTRIES, sizeof (struct GMT_DCW_COUNTRY), gmtdcw_comp_countries);	/* Sort on country code */

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
					if (n_items == n_alloc) {
						n_alloc += GMT_LEN128;
						order = gmt_M_memory (GMT, order, n_alloc, unsigned int);
					}
				}
				if (n_items)
					GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Continent code expanded from %s to %s [%d countries]\n", F->item[j]->codes, list, n_items);
				else
					GMT_Report (GMT->parent, GMT_MSG_WARNING, "Continent code %s unrecognized\n", code);
			}
			else if ((ks = gmtdcw_found_collection (code, GMT_DCW_collection, n_bodies)) != GMT_NOTSET) {
				if (GMT_DCW_collection[ks].type == DCW_NAMED_LIST) {
					/* Got a collection we wish to add */
					unsigned n_list = gmt_count_char (GMT, GMT_DCW_collection[ks].list, ',') + 1;	/* Number of items in collection */
					if (n_items) strcat (list, ",");
					strcat (list, GMT_DCW_collection[ks].list);
					for (k = 0; k < n_list; k++) {
						order[n_items++] = j;	/* So we know which color/pen to apply for all items in this collection */
						if (n_items == n_alloc) {
							n_alloc += GMT_LEN128;
							order = gmt_M_memory (GMT, order, n_alloc, unsigned int);
						}
					}
					GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Collection code expanded from %s to %s [%d countries]\n", F->item[j]->codes, GMT_DCW_collection[ks].list, n_list);
				}
				else	/* Named regions do not correspond to any data code so skipped here */
					named_wesn++;
			}
			else {	/* Just append this single one */
				if (gmtdcw_got_name (code, GMT_DCW_country, GMT_DCW_COUNTRIES, GMT_DCW_state, GMT_DCW_STATES, ISO))	/* Check if we got the name of a country or state, then replace with ISO code */
					strcpy (code, ISO);
				if (n_items) strcat (list, ",");
				strcat (list, code);
				order[n_items] = j;	/* So we know which color/pen to apply for this item */
				n_items++;
				if (n_items == n_alloc) {
					n_alloc += GMT_LEN128;
					order = gmt_M_memory (GMT, order, n_alloc, unsigned int);
				}
			}
		}
	}

	if (n_items || named_wesn) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Requested %d DCW items: %s\n", n_items, list);
		order = gmt_M_memory (GMT, order, n_items, unsigned int);
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "No countries selected\n");
		gmt_M_free (GMT, order);
		gmt_M_free (GMT, GMT_DCW_country);
		gmt_M_free (GMT, GMT_DCW_state);
		gmtdcw_freestrings (GMT_DCW_collection, dim[3]+dim[4]);
		gmt_M_free (GMT, GMT_DCW_collection);
		return NULL;
	}

	if (mode & GMT_DCW_PLOT) {	/* Plot via psxy instead */
		/* Because holes in polygons comes last we cannot just plot as we go. Instead, we must assemble
		 * the entire list of polygons for one item, then pass that dataset to psxy for plotting.
		 * So here, that means switch to GMT_DCW_EXTRACT but set a special flag so that we call psxy
		 * and then delete the dataset instead of returning it. */
		mode -= GMT_DCW_PLOT;
		mode += GMT_DCW_EXTRACT;
		special = GMT_DCW_PLOTTING;
	}
	else if (mode & (GMT_DCW_CLIP_IN|GMT_DCW_CLIP_OUT)) {	/* Lay down clip path via clip instead */
		/* Because holes in polygons comes last we cannot just set clip path as we go. Instead, we must assemble
		 * the entire list of polygons for one item, then pass that dataset to clip for clipping.
		 * So here, that means switch to GMT_DCW_EXTRACT but set a special flag so that we call clip
		 * and then delete the dataset instead of returning it. */
		mode -= (mode & GMT_DCW_CLIP_IN) ? GMT_DCW_CLIP_IN : GMT_DCW_CLIP_OUT;
		mode += GMT_DCW_EXTRACT;
		special = GMT_DCW_CLIPPING;
	}

	if (!gmtdcw_get_path (GMT, "dcw-gmt", ".nc", path)) {
		gmt_M_free (GMT, order);
		return NULL;
	}

	if (mode > GMT_DCW_REGION) {	/* Wish to get actual polygons */
		P = GMT_Alloc_Segment (GMT->parent, GMT_NO_STRINGS, 0, 2, NULL, NULL);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Extract polygons from DCW - The Digital Chart of the World\n");
	}

	if (mode & GMT_DCW_EXTRACT) {	/* Plan to return a dataset */
		uint64_t dim[4] = {n_items, 0, 0, 2};	/* n_items tables whose records (to be allocated) will have 2 columns */
		if ((D = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create empty dataset for DCW polygons\n");
			gmt_free_segment (GMT, &P);
			gmt_M_free (GMT, order);
			return NULL;
		}
	}

	if ((retval = gmt_nc_open (GMT, path, NC_NOWRITE, &ncid))) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot open file %s!\n", path);
		gmt_free_segment (GMT, &P);
		gmt_M_free (GMT, order);
		return NULL;
	}

	/* Get global attributes */
	if ((retval = nc_get_att_text (ncid, NC_GLOBAL, "version", version))) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot obtain DCW attribute version\n");
		gmt_free_segment (GMT, &P);
		gmt_M_free (GMT, order);
		return NULL;
	}
	if (version[0] != '1') new_CN_codes = true;	/* DCW 2.0.0 or later has new Chinese province codes */
	if ((retval = nc_get_att_text (ncid, NC_GLOBAL, "title", title))) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot obtain DCW attribute title\n");
		gmt_free_segment (GMT, &P);
		gmt_M_free (GMT, order);
		return NULL;
	}
	if ((retval = nc_get_att_text (ncid, NC_GLOBAL, "source", source))) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot obtain DCW attribute source\n");
		gmt_free_segment (GMT, &P);
		gmt_M_free (GMT, order);
		return NULL;
	}
	if ((retval = nc_get_att_text (ncid, NC_GLOBAL, "gmtversion", gmtversion)) == NC_NOERR)
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Found gmtversion string in DCW file: %s\n", gmtversion);

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Using country and state data from dcw-gmt\n");
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Title  : %s\n", title);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Source : %s\n", source);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Version: %s\n", version);
		if (gmtversion[0]) GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "DCW version %s requires GMT version %s or later.\n", version, gmtversion);
	}

	if (gmtversion[0]) {	/* The gmtversion attribute was available [starting with DCW 2.0.0] */
		int maj, min, rel;
		if (sscanf (gmtversion, "%d.%d.%d", &maj, &min, &rel) != 3) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to parse minimum GMT version information\n");
			gmt_free_segment (GMT, &P);
			gmt_M_free (GMT, order);
			return NULL;
		}
		if (maj > GMT_MAJOR_VERSION || (maj == GMT_MAJOR_VERSION && min > GMT_MINOR_VERSION) || (maj == GMT_MAJOR_VERSION && min == GMT_MINOR_VERSION && rel > GMT_RELEASE_VERSION)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "This DCW version (%s) requires at least GMT %s; you have %d.%d.%d\n", version, gmtversion, GMT_MAJOR_VERSION, GMT_MINOR_VERSION, GMT_RELEASE_VERSION);
			gmt_free_segment (GMT, &P);
			gmt_M_free (GMT, order);
			return NULL;
		}
	}

	if ((mode & GMT_DCW_DUMP) || (mode & GMT_DCW_REGION)) {	/* Dump the coordinates to stdout or return -R means setting col types */
		gmt_set_geographic (GMT, GMT_OUT);
	}

	if (mode & GMT_DCW_REGION) {	/* Just update wesn */
		Z = gmt_M_memory (GMT, NULL, n_items+named_wesn, struct GMT_RANGE);
		if (named_wesn) {	/* Must pick up the wesn of those entities */
			for (j = 0; j < F->n_items; j++) {
				pos = 0;
				while (gmt_strtok (F->item[j]->codes, ",", &pos, code)) {	/* Loop over items */
					if ((ks = gmtdcw_found_collection (code, GMT_DCW_collection, n_bodies)) != GMT_NOTSET && GMT_DCW_collection[ks].type == DCW_NAMED_WESN) {
						/* Expand current wesn by this named area */
						Z[r_item].west = GMT_DCW_collection[ks].wesn[XLO];	Z[r_item++].east = GMT_DCW_collection[ks].wesn[XHI];
						if (GMT_DCW_collection[ks].wesn[YLO] < wesn[YLO]) wesn[YLO] = GMT_DCW_collection[ks].wesn[YLO];
						if (GMT_DCW_collection[ks].wesn[YHI] > wesn[YHI]) wesn[YHI] = GMT_DCW_collection[ks].wesn[YHI];
						GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Collection code expanded from %s to %s \n", F->item[j]->codes, GMT_DCW_collection[ks].wesn_string);
					}
				}
			}
		}
	}

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
		ks = gmtdcw_find_country (code, GMT_DCW_country, GMT_DCW_COUNTRIES);
		if (ks == GMT_NOTSET) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "No country code matching %s (skipped)\n", code);
			continue;
		}
		k = ks;
		if (want_state) {
			item = gmtdcw_find_state (GMT, state, code, GMT_DCW_state, GMT_DCW_STATES, new_CN_codes);
			if (item == GMT_NOTSET) {
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "Country %s does not have a state named %s (skipped)\n", code, state);
				continue;
			}
			snprintf (TAG, GMT_LEN16, "%s%s", GMT_DCW_country[k].code, GMT_DCW_state[item].code);
			if (F->mode & GMT_DCW_ZHEADER)
				snprintf (msg, GMT_BUFSIZ, "-Z%s %s (%s)\n", TAG, GMT_DCW_state[item].name, GMT_DCW_country[k].name);
			else
				snprintf (msg, GMT_BUFSIZ, "%s (%s)\n", GMT_DCW_state[item].name, GMT_DCW_country[k].name);
		}
		else {
			snprintf (TAG, GMT_LEN16, "%s", GMT_DCW_country[k].code);
			if (F->mode & GMT_DCW_ZHEADER)
				snprintf (msg, GMT_BUFSIZ, "-Z%s %s\n", TAG, GMT_DCW_country[k].name);
			else
				snprintf (msg, GMT_BUFSIZ, "%s\n", GMT_DCW_country[k].name);
		}
		if (!strncmp (GMT_DCW_country[k].code, "AQ", 2U)) is_Antarctica = true;

		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, msg);
		k = strlen (msg) - 1;
		msg[k] = '\0';	/* Remove the newline for use as segment header */

		/* Open and read the netCDF file */

		snprintf (dim, GMT_LEN16, "%s_length", TAG);
		if ((retval = nc_inq_dimid (ncid, dim, &id))) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure while getting ID for variable %s in %s!\n", dim, path);
			continue;
		}
		if ((retval = nc_inq_dimlen (ncid, id, &np))) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure while getting dimension length for variable %s in %s!\n", dim, path);
			continue;
		}
		if (mode > GMT_DCW_REGION && np > max_np) {
			size_t tmp_size = max_np;
			gmt_M_malloc2 (GMT, lon, lat, np, &tmp_size, double);
			gmt_M_malloc2 (GMT, dx, dy, np, &max_np, unsigned short int);
			if (lon == NULL || lat == NULL || dx == NULL|| dy == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure while allocating memory!\n");
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
			if (dx[k] == 65535U) {	/* Start of new segment */
				n_segments++;	/* Count how many segments */
				lon[k] = GMT->session.d_NaN;	/* Flag a segment with lon = NaN */
				lat[k] = (dy[k] == 1) ? 1.0 : 0.0;	/* This is always 0.0 for 1.1.4 and older, which had no holes anyway */
			}
			else {
				lon[k] = dx[k] * xscl + west;
				lat[k] = dy[k] * yscl + south;
			}
		}
		if (mode & GMT_DCW_EXTRACT) {	/* Allocate a table with the right number of segments */
			gmt_free_table (GMT, D->table[tbl]);
			D->table[tbl] = gmt_create_table (GMT, n_segments, 0, 2, 0, false);
		}
		if (special == GMT_DCW_PLOTTING) {	/* Time to consider fill/pen change */
			outline = (F->item[order[tbl]]->mode & DCW_DO_OUTLINE);
			fill = (F->item[order[tbl]]->mode & DCW_DO_FILL);
			spen = (outline) ? &(F->item[order[tbl]]->pen) : NULL;
			sfill = (fill) ? &(F->item[order[tbl]]->fill) : NULL;
		}

		/* Extract the pieces into separate segments */
		k = seg = 0;
		done = false;
	        while (!done) {
			first = GMT_NOTSET;
			while (first == GMT_NOTSET && k < np) {	/* Look for next start of segment marker */
				if (gmt_M_is_dnan (lon[k])) {
					hole = (lat[k] > 0.0);
					first = k + 1;	/* Start of segment */
				}
				k++;
			}
			if (first == GMT_NOTSET) { done = true; continue;}	/* No more segments */
			last = GMT_NOTSET;
			while (last == GMT_NOTSET && k < np) {/* Look for next end of segment marker (or end of line) */
				if (gmt_M_is_dnan (lon[k])) last = k - 1;	/* End of segment */
				k++;
			}
			if (last == GMT_NOTSET) last = np - 1;	/* End of last segment */
			k--;	/* Back to last segment marker  which will be the next start marker */
			P->n_rows = last - first + 1;	/* Number of points in this segment */
			P->data[GMT_X] = &lon[first];
			P->data[GMT_Y] = &lat[first];
			sprintf (label, " %s Segment %" PRIu64, msg, seg);
			header[0] = '\0';
			if (hole)
				strcat (header, "-Ph");

			if (mode & GMT_DCW_DUMP) {	/* Dump the coordinates to stdout */
				strcat (header, label);
				strcpy (GMT->current.io.segment_header, header);
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
				SH = gmt_get_DS_hidden (S);
				if (special == GMT_DCW_PLOTTING) {
					if (sfill) {
						strcat (header, " -G"); strcat (header, gmtlib_putfill (GMT, sfill));
					}
					else
						strcat (header, " -G-");
					if (spen) {
						strcat (header, " -W"); strcat (header, gmt_putpen (GMT, spen));
					}
					else
						strcat (header, " -W-");
				}
				strcat (header, label);
				S->header = strdup (header);
				if (hole)
					SH->pol_mode = GMT_IS_HOLE;
				S->n_rows = P->n_rows;
				gmt_M_malloc2 (GMT, S->data[GMT_X], S->data[GMT_Y], S->n_rows, NULL, double);
				gmt_M_memcpy (S->data[GMT_X], P->data[GMT_X], S->n_rows, double);
				gmt_M_memcpy (S->data[GMT_Y], P->data[GMT_Y], S->n_rows, double);
				SH->alloc_mode[GMT_X] = SH->alloc_mode[GMT_Y] = GMT_ALLOC_INTERNALLY;	/* Allocated in GMT */
				seg++;
			}
		}
		tbl++;
	}

	gmt_nc_close (GMT, ncid);
	gmt_M_free (GMT, GMT_DCW_country);
	gmt_M_free (GMT, GMT_DCW_state);
	gmtdcw_freestrings (GMT_DCW_collection, n_bodies[3]+n_bodies[4]);
	gmt_M_free (GMT, GMT_DCW_collection);
	gmt_M_free (GMT, Out);

	if (mode & GMT_DCW_REGION) {
		gmt_find_range (GMT, Z, r_item, &wesn[XLO], &wesn[XHI]);
		gmt_M_free (GMT, Z);
		GMT->current.io.geo.range = GMT_IGNORE_RANGE;		/* Override this setting explicitly */
		gmt_extend_region (GMT, wesn, F->adjust, F->inc);
		if (is_Antarctica) {	/* Must override to include south pole and full longitude range */
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
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Region implied by DCW polygons is %g/%g/%g/%g\n", wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
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

	if (D) gmt_set_dataset_minmax (GMT, D);		/* Update stats */

	if (special) {	/* Plot via psxy or clip via psclip, then free dataset */
		char cmd[GMT_BUFSIZ] = {""}, in_string[GMT_VF_LEN] = {""};
		static char *module[2] = {"psxy", "psclip"};
		/* Get a virtual file for the current DCW dataset */
		if (GMT_Open_VirtualFile (GMT->parent, GMT_IS_DATASET, GMT_IS_POLY, GMT_IN|GMT_IS_REFERENCE, D, in_string) == GMT_NOTSET) {
			return (NULL);
		}
		/* All pen and fill settings are passed via segment headers, so this part is common to both psxy and psclip: */
		snprintf (cmd, GMT_BUFSIZ, "-R -J -O -K %s", in_string);
		if (F->mode & GMT_DCW_CLIP_OUT)	/* Set the outside clip flag */
			strcat (cmd, " -N");	/* Select the outside clip flag */
		strcat (cmd, " --GMT_HISTORY=readonly");	/* Ignore history on exit */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Calling %s with args %s\n", module[special-1], cmd);
		if (GMT_Call_Module (GMT->parent, module[special-1], GMT_MODULE_CMD, cmd) != GMT_OK) {
			return (NULL);
		}
		/* Close the virtual file and destroy the resource */
		GMT_Close_VirtualFile (GMT->parent, in_string);
		GMT_Destroy_Data (GMT->parent, &D);
	}

	return (D);
}

unsigned int gmt_DCW_list (struct GMT_CTRL *GMT, struct GMT_DCW_SELECT *F) {
	/* Write to stdout the available countries [and optionally states], then make calling program exit */
	unsigned int list_mode, i, j, k, kk, GMT_DCW_COUNTRIES = 0, GMT_DCW_STATES = 0, GMT_DCW_N_COUNTRIES_WITH_STATES = 0, n_bodies[5] = {0, 0, 0, 0, 0};
	bool search = false;
	char string[GMT_LEN512] = {""}, *target = NULL, *region = NULL;
	struct GMT_DCW_COUNTRY *GMT_DCW_country = NULL;
	struct GMT_DCW_STATE *GMT_DCW_state = NULL;
	struct GMT_DCW_COUNTRY_STATE *GMT_DCW_country_with_state = NULL;
	struct GMT_DCW_COLLECTION *GMT_DCW_collection = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	list_mode = F->mode;
	if ((list_mode & GMT_DCW_LIST) == 0) return 0;
	if (gmtdcw_load_lists (GMT, &GMT_DCW_country, &GMT_DCW_state, &GMT_DCW_country_with_state, &GMT_DCW_collection, n_bodies)) return GMT_NOTSET;	/* Something went wrong */
	GMT_DCW_COUNTRIES = n_bodies[0];
	GMT_DCW_STATES = n_bodies[1];
	GMT_DCW_N_COUNTRIES_WITH_STATES = n_bodies[2];
	GMT_Report (API, GMT_MSG_INFORMATION, "List of ISO 3166-1 alpha-2 codes for DCW supported countries:\n\n");
	for (k = 0; k < F->n_items; k++) {
		if (!F->item[k]->codes || F->item[k]->codes[0] == '\0') continue;
		search = true;	/* Gave some codes */
	}

	/* Initialize rec-by-rec output */
	if (GMT_Set_Columns (API, GMT_OUT, 0, GMT_COL_FIX) != GMT_NOERROR) {
		return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_OUT, GMT_ADD_DEFAULT, 0, F->options) != GMT_NOERROR) {	/* Establishes data output */
		return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_TEXT) != GMT_NOERROR) {	/* Sets output geometry */
		return (API->error);
	}

	Out = gmt_new_record (GMT, NULL, string);	/* Since we only need to worry about text in this module */

	if (list_mode & DCW_GET_COLLECTIONS) {
		unsigned int nc = n_bodies[3] + n_bodies[4];	/* Total number of collections */
		gmt_set_tableheader (GMT, GMT_OUT, true);
		sprintf (string, "TAG\tName\t\tCodes|Region");
		GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, string);
		for (i = 0; i < nc; i++) {
			if (GMT_DCW_collection[i].type == DCW_NAMED_LIST)
				target = region = GMT_DCW_collection[i].list;
			else {
				target = GMT_DCW_collection[i].name;
				region = GMT_DCW_collection[i].wesn_string;
			}
			if (search) {	/* Listed collection(s) */
				bool found = false;
				for (kk = 0; !found && kk < F->n_items; kk++) {
					if (strstr (target, F->item[kk]->codes))
						found = true;	/* This item is part of this collection */
				}
				if (!found) continue;
			}
			sprintf (string, "%s\t%s\t%s", GMT_DCW_collection[i].region, GMT_DCW_collection[i].name, region);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
	}
	else {
		for (i = k = 0; i < GMT_DCW_COUNTRIES; i++) {
			if (search) {	/* Listed continent(s) */
				bool found = false;
				for (kk = 0; kk < F->n_items; kk++) {
					if (F->item[kk]->codes[0] == '=') {
						if (strstr (F->item[kk]->codes, GMT_DCW_country[i].continent))
							found = true;
					}
					else if (strncmp (F->item[kk]->codes, GMT_DCW_country[i].code, 2U) == 0)
							found = true;
				}
				if (!found) continue;
			}
			if (F->n_items == 0 && (i == 0 || strcmp (GMT_DCW_country[i].continent, GMT_DCW_country[i-1].continent)) ) {
				sprintf (string, "%s [%s]", GMT_DCW_continents[k++], GMT_DCW_country[i].continent);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			if ((list_mode & DCW_GET_COUNTRY_AND_STATE) == 0) {
				sprintf (string, "%s\t%s", GMT_DCW_country[i].code, GMT_DCW_country[i].name);
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			}
			if ((list_mode & DCW_GET_COUNTRY_AND_STATE) && gmtdcw_country_has_states (GMT_DCW_country[i].code, GMT_DCW_country_with_state, GMT_DCW_N_COUNTRIES_WITH_STATES)) {
				for (j = 0; j < GMT_DCW_STATES; j++) {
					if (!strcmp (GMT_DCW_country[i].code, GMT_DCW_state[j].country)) {
						sprintf (string, "%s.%s\t%s", GMT_DCW_country[i].code, GMT_DCW_state[j].code, GMT_DCW_state[j].name);
						GMT_Put_Record (API, GMT_WRITE_DATA, Out);
					}
				}
			}
		}
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		return (API->error);
	}
	gmt_M_free (GMT, Out);
	gmt_M_free (GMT, GMT_DCW_country);
	gmt_M_free (GMT, GMT_DCW_state);
	gmt_M_free (GMT, GMT_DCW_country_with_state);
	gmtdcw_freestrings (GMT_DCW_collection, n_bodies[3]+n_bodies[4]);
	gmt_M_free (GMT, GMT_DCW_collection);
	return ((list_mode & GMT_DCW_LIST));
}

void gmt_DCW_option (struct GMTAPI_CTRL *API, char option, unsigned int plot) {
	/* Show the usage */
	char *action[2] = {"extract", "plot"};
	char *action2[2] = {"extracting", "plotting"};
	char *usage[2] = {"Extract clipping polygons", "Apply different fill or outlines"};
	GMT_Usage (API, 1, "\n-%c%s", option, DCW_OPT);
	GMT_Usage (API, -2, "%s for specified list of countries. "
		"Based on closed polygons from the Digital Chart of the World (DCW). "
		"Append comma-separated list of ISO 3166 codes (or full names) for countries to %s, i.e., "
		"<code1>,<code2>,... etc., using the 2-character country codes. "
		"To select a state of a country (if available), append .state, e.g, US.TX for Texas, or just the state name. "
		"To select a whole continent, use =AF|AN|AS|EU|OC|NA|SA as <code> or give full name. For collections and named regions, "
		"append their codes or full name. All names are case-insensitive. Available modifiers:", usage[plot], action[plot]);
	if (plot == 1) {
		GMT_Usage (API, 3, "+c Set clip paths for the inside  area [none].");
		GMT_Usage (API, 3, "+C Set clip paths for the outside area [none].");
		GMT_Usage (API, 3, "+g Fill polygons using given <fill> to fill [none].");
	}
	GMT_Usage (API, 3, "+l Just list the countries and their codes [no %s takes place].", action2[plot]);
	GMT_Usage (API, 3, "+L List states/territories for Argentina, Australia, Brazil, Canada, China, India, Russia and the US. "
		"Select =<continent>+l|L to only list countries from that continent or <code>+L for that country(repeatable).");
	GMT_Usage (API, 3, "+n List collections and named regions, their codes and list of items (or region) [no %s takes place].", action2[plot]);
	if (plot == 1)
		GMT_Usage (API, 3, "+p Draw outline using given <pen> [none].");
	GMT_Usage (API, 3, "+z Add -Z<countrycode> to multisegment headers if extracting polygons.");
	if (plot == 1) {
		GMT_Usage (API, -2, "Note: One of +c|C|p|g must be specified to plot; if -M is in effect we just get the data. "
			"Repeat -%c to give different groups of items their own pen/fill settings.", option);
	}
}

unsigned int gmt_DCW_parse (struct GMT_CTRL *GMT, char option, char *args, struct GMT_DCW_SELECT *F) {
	/* Parse the F option in pscoast */
	unsigned int n_errors = 0, pos = 0;
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
					GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Option -%c: Ambiguous modifier +r<val>; could be dpi of the pattern or (a deprecated) region increment - choosing dpi.\n", option);
					GMT_Report (GMT->parent, GMT_MSG_DEBUG, "If you meant the region modifier then place it before the +g pattern specification.\n", option);
					r[0] = GMT_ASCII_US;	/* Change +r<dpi> to ASCII31<dpi> to pass strtok splitting */
				}
				/* Else it is taken to be a deprecated region increment */
			}
		}
		while ((gmt_strtok (c, "+", &pos, p))) {
			switch (p[0]) {
				/* Listings*/
				case 'R':	case 'e': case 'r': /* Get region from polygon(s) BB */
					F->region = true;
					n_errors += gmt_parse_region_extender (GMT, option, p, &(F->adjust), F->inc);	/* Possibly extend the final region before reporting */
					break;
				case 'l':		/* Country list */
					F->mode = DCW_GET_COUNTRY;
					F->mode |= GMT_DCW_LIST;
					break;
				case 'L': 	/* Country and state list */
					F->mode = DCW_GET_COUNTRY_AND_STATE;
					F->mode |= GMT_DCW_LIST;
					break;
				case 'n': 	/* Collections list */
					F->mode = DCW_GET_COLLECTIONS;
					F->mode |= GMT_DCW_LIST;
					break;
				case 'c':		/* Set up a clip path around the selections instead */
					F->mode |= GMT_DCW_CLIP_IN;
					break;
				case 'C':		/* Set up a clip path outside the selections instead */
					F->mode |= GMT_DCW_CLIP_OUT;
					break;
				case 'p':
					if (gmt_getpen (GMT, &p[1], &(this_item->pen))) {	/* Error decoding pen */
						gmt_pen_syntax (GMT, option, NULL, " ", NULL, 0);
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
				case 'z':		/* Add country code to -Z<code> in segment header */
					F->mode |= GMT_DCW_ZHEADER;
					break;
				default:
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -%c: Unrecognized modifier +%s.\n", option, p);
					n_errors++;
					break;
			}
		}
	}
	if ((F->mode & (GMT_DCW_PLOT|DCW_DO_FILL)) && (F->mode & (GMT_DCW_CLIP_IN|GMT_DCW_CLIP_OUT))) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -%c: Cannot mix clipping and plotting!\n", option);
		n_errors++;
	}
	if ((F->mode & (GMT_DCW_CLIP_IN|GMT_DCW_CLIP_OUT)) && (F->mode & GMT_DCW_ZHEADER)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -%c: Cannot mix clipping and setting header codes!\n", option);
		n_errors++;
	}
	if (this_item->codes[0] == '\0' && !(F->mode & (DCW_GET_COUNTRY+DCW_GET_COUNTRY_AND_STATE+DCW_GET_COLLECTIONS))) {	/* Did not give +l, +L, or +n and no codes */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -%c: No country codes given\n", option);
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
