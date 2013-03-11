/* Misc functions to find and read DCW polygons.  I made
 * netcdf versions of the DCW polygons, with lon = lat = 999
 * being the flag to indicate start of new segment.
 * Only US, Canada, Australia, and Brazil has state borders too.
 * P. Wessel, March 12, 2013.
 */

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

int GMT_get_DCW_polygon (struct GMT_CTRL *GMT, struct F *E, bool dump)
{	/* Given comma-separated names, plot or dump the corresponding polygons, or give error */
	int ks, j, kk, retval, ncid, varid, id, first, last;
	size_t np;
	unsigned int k, n_items = 1, pos = 0;
	bool done, want_state, outline = (E->mode & 4), fill = (E->mode & 8);
	char code[GMT_TEXT_LEN16], state[GMT_TEXT_LEN16], file[GMT_TEXT_LEN16], msg[GMT_BUFSIZ], path[GMT_BUFSIZ];
	double out[2];
	struct GMT_DATASEGMENT *S = NULL, *P = NULL;;
	
	if (!E->codes || E->codes[0] == '\0') return 1;
	
	for (k = 0; k < strlen (E->codes); k++) if (E->codes[k] == ',') n_items++;

	qsort ((void *)GMT_DCW_country, (size_t)GMT_DCW_COUNTRIES, sizeof (struct GMT_DCW_COUNTRY), gmt_dcw_comp_countries);	/* Sort on country code */
 	
	S = GMT_memory (GMT, NULL, 1, struct GMT_DATASEGMENT);
	GMT_alloc_segment (GMT, S, 0, 2, true);
	P = GMT_memory (GMT, NULL, 1, struct GMT_DATASEGMENT);
	GMT_alloc_segment (GMT, P, 0, 2, true);
	if (!dump) {
		if (outline) GMT_setpen (GMT, &E->pen);
		if (fill) GMT_setfill (GMT, &E->fill, outline);
	}

	while (GMT_strtok (E->codes, ",", &pos, code)) {	/* Loop over countries */
		want_state = false;
		if (code[2] == '.') {	/* Requesting a state */
			GMT_memset (state, GMT_TEXT_LEN16, char);
			strcpy (state, &code[3]);
			code[2] = '\0';
			want_state = true;
		}
		ks = gmt_dcw_find_country (code, GMT_DCW_country, GMT_DCW_COUNTRIES);
		if (ks == -1) {
			GMT_report (GMT, GMT_MSG_NORMAL, "No country code matching %s (skipped)\n", code);
			continue;
		}
		k = ks;
		if (want_state) {
			if ((j = gmt_dcw_find_state (state, code, GMT_DCW_states, GMT_DCW_STATES)) == -1) {
				GMT_report (GMT, GMT_MSG_NORMAL, "Country %s does not have states (skipped)\n", code);
				continue;
			}
			sprintf (file, "%s/%s/%s", GMT_DCW_country[k].continent, GMT_DCW_country[k].code, GMT_DCW_states[j].code);
		}
		else
			sprintf (file, "%s/%s", GMT_DCW_country[k].continent, GMT_DCW_country[k].code);
			
		GMT_getsharepath (GMT, "coast/DCW", file, ".nc", path);

		if ((retval = nc_open (path, NC_NOWRITE, &ncid))) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Cannot open file %s!\n", path);
			continue;
		}
		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE) || dump) {
			if (want_state) 
				sprintf (msg, "Extract data for %s (%s)\n", GMT_DCW_states[j].name, GMT_DCW_country[k].name);
			else
				sprintf (msg, "Extract data for %s\n", GMT_DCW_country[k].name);
			GMT_report (GMT, GMT_MSG_VERBOSE, msg);
			k = strlen (msg) - 1;
			msg[k] = '\0';
		}
		
		/* Open and read the netCDF file */
		
		if ((retval = nc_inq_dimid (ncid, "time", &id))) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Error processessing %s!\n", path);
			continue;
		}
		retval = nc_inq_dimlen (ncid, id, &np);
		
		GMT_malloc2 (GMT, S->coord[GMT_X], S->coord[GMT_Y], np, NULL, double);
		
	        /* Get the varid of the lon and lat variables, based on their names, and get the data */

		if ((retval = nc_inq_varid (ncid, "lon", &varid))) continue;
		if ((retval = nc_get_var_double (ncid, varid, S->coord[GMT_X]))) continue;
		if ((retval = nc_inq_varid (ncid, "lat", &varid))) continue;
		if ((retval = nc_get_var_double (ncid, varid, S->coord[GMT_Y]))) continue;
		if ((retval = nc_close (ncid))) continue;

	        /* Extract the pieces into separate segments */
		k = 0;
		done = false;
	        while (!done) {
			first = -1;
			while (first == -1 && k < np) {	/* Look for next start of segment marker */
				if (doubleAlmostEqual (S->coord[GMT_X][k], 999.0) && doubleAlmostEqual (S->coord[GMT_Y][k], 999.0)) first = k + 1;	/* Start of segment */
				k++;
			}
			if (first == -1) { done = true; continue;}	/* No more segments */
			last = -1;
			while (last == -1 && k < np) {/* Look for next end of segment marker (or end of line) */
				if (doubleAlmostEqual (S->coord[GMT_X][k], 999.0) && doubleAlmostEqual (S->coord[GMT_Y][k], 999.0)) last = k - 1;	/* End of segment */
				k++;
			}
			if (last == -1) last = np - 1;	/* End of last segment */
			k--;	/* Back to last segment marker  which will be the next start marker */
			P->n_rows = last - first + 1;	/* Number of points in this segment */
			P->coord[GMT_X] = &S->coord[GMT_X][first];
			P->coord[GMT_Y] = &S->coord[GMT_Y][first];
			if (dump) {	/* Dump the coordinates to stdout */
				strcpy (GMT->current.io.segment_header, msg);
				GMT_Put_Record (GMT->parent, GMT_WRITE_SEGMENT_HEADER, NULL);
				for (kk = 0; kk < P->n_rows; kk++) {
					out[GMT_X] = P->coord[GMT_X][kk];
					out[GMT_Y] = P->coord[GMT_Y][kk];
					GMT_Put_Record (GMT->parent, GMT_WRITE_DOUBLE, out);
				}
			}
			else {	/* Plot this piece */
				if (fill)	/* Plot filled polygon, w/ or w/o outline */
					GMT_geo_polygons (GMT, P);
				else {	/* Plot outline only */
					if ((GMT->current.plot.n = GMT_geo_to_xy_line (GMT, P->coord[GMT_X], P->coord[GMT_Y], P->n_rows)) == 0) continue;
					GMT_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n);
				}
			}
		}
		GMT_free (GMT, S->coord[GMT_X]);
		GMT_free (GMT, S->coord[GMT_Y]);
	}
	P->coord[GMT_X] = P->coord[GMT_Y] = NULL;
	GMT_free_segment (GMT, S);
	GMT_free_segment (GMT, P);
	return (GMT_OK);
}

unsigned int GMT_list_DCW_polygon (struct GMT_CTRL *GMT, unsigned list_mode)
{	/* List available countries [and optionally states]; then make program exit */
	unsigned int i, j, k;
	if ((list_mode & 3) == 0) return 0;
	for (i = k = 0; i < GMT_DCW_COUNTRIES; i++) {
		if (i == 0 || strcmp (GMT_DCW_country[i].continent, GMT_DCW_country[i-1].continent) ) {
			GMT_message (GMT, "%s [%s]:\n", GMT_DCW_continents[k++], GMT_DCW_country[i].continent);
		}
		printf ("  %s\t%s\n", GMT_DCW_country[i].code, GMT_DCW_country[i].name);
		if ((list_mode & 2) && gmt_dcw_country_has_states (GMT_DCW_country[i].code, GMT_DCW_country_with_states, GMT_DCW_N_COUNTRIES_WITH_STATES)) {
			for (j = 0; j < GMT_DCW_STATES; j++) {
				if (!strcmp (GMT_DCW_country[i].code, GMT_DCW_states[j].country)) GMT_message (GMT, "\t\t%s.%s\t%s\n", GMT_DCW_country[i].code, GMT_DCW_states[j].code, GMT_DCW_states[j].name);
			}
		}
	}
	return ((list_mode & 3));
}

void GMT_list_DCW_usage (struct GMT_CTRL *GMT, char option)
{	/* Show the usage */
	GMT_message (GMT, "\t-%c Apply different fill or outline to specified list of countries.\n", option);
	GMT_message (GMT, "\t   Based on closed polygons from the Digital Chart of the World (DCW).\n");
	GMT_message (GMT, "\t   Append comma-separated list of codes for countries to plot, i.e.,\n");
	GMT_message (GMT, "\t   <code1>,<code2>,... etc., using the 2-character country codes.\n");
	GMT_message (GMT, "\t   To select a state of a country (if available), append .state, e.g, US.TX for Texas.\n");
	GMT_message (GMT, "\t   Append +l to just list the countries and their codes [no plotting takes place].\n");
	GMT_message (GMT, "\t   Use +L to see states/terretories for Australia, Brazil, Canada, and the US.\n");
	GMT_message (GMT, "\t   Append +p<pen> to draw outline [none] and +f<fill> to fill [none].\n");
	GMT_message (GMT, "\t   One of +p|f must be specified unless -M is in effect.\n");
}

unsigned int GMT_DCW_parse (struct GMT_CTRL *GMT, char option, char *args, struct F *F)
{	/* Parse the F option in pscoast */
	unsigned int n_errors = 0, pos = 0;
	char p[GMT_BUFSIZ], *c = NULL, *a = NULL;
		
	if ((a = strchr (args, '+'))) a[0] = '\0';	/* Temporarily chop off modifiers */
	F->codes = strdup (args);
	a[0] = '+';	/* Reset modifiers */
	
	if ((c = strchr (a, '+'))) {	/* Handle modifiers */
		while ((GMT_strtok (c, "+", &pos, p))) {
			switch (p[0]) {
				/* Listings*/
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
					GMT_report (GMT, GMT_MSG_NORMAL, "Error -%c: Unrecognized modifier +%s.\n", option, p);
					n_errors++;
					break;
			}
		}
	}
	return (n_errors);
}
