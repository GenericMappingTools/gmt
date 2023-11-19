/* Functions to nail down if a token is a possible longitude, latitude
 * floating point, absolute time, or text string (including NaN)
 */

GMT_LOCAL bool gmtio_is_integer (char *string) {
	unsigned int k = 0;
	if (string == NULL) return (false);	/* Safety valve */
	if (string[k] == '-' || string[k] == '+') k++;	/* Skip any leading signs */
	while (string[k]) {	/* Cannot be a floating point number */
		if (!isdigit (string[k])) return (false);	/* Not an integer */
		k++;	/* Advance to next letter */
	}
	return (true);
}

GMT_LOCAL bool gmtio_is_valid_integer (char *string, int max) {
	/* Returns true if argument is an integer and it is <= max. string has no leading sign */
	int i;
	if (!gmtio_is_integer (string)) return (false);	/* Not even an integer */
	if (max == 0) return (true);	/* No limit check on the positive integer */
	i = atoi (string);	/* Must decode the value and check limit */
	return ((i > max) ? false : true);
}

GMT_LOCAL bool gmtio_not_float (char c) {
	if (strchr ("0123456789+-.", c) == NULL)	/* Cannot be a floating point number */
		return true;
	return (false);
}

GMT_LOCAL bool gmtio_is_float (struct GMT_CTRL *GMT, char *string, bool allow_exp, double max) {
	/* Returns true if string is a floating point number: [±]<f.xxx>[e|d[±]<i>].
	 * If allow_exp is false then we only allow [±]<f.xxx>. If max > 0 then float must be <= max.
	 * Note: string has no leading sign. */

	unsigned int L = strlen (string), k = 0, n_exp = 0;
	for (k = 0; k < L; k++) {
		if (string[k] == 'e' || string[k] == 'd') {	/* Allow junk FORTRAN output using d for double precision exponential format */
			if (!allow_exp) return (false);
			n_exp++;	/* Only one allowed */
			if (string[k+1]) {	/* Determine if we have a [signed]exponential argument that is an integer */
				if (!gmtio_is_integer (&string[k+1])) return (false);	/* Not an integer */
			}
			else	/* Got e|d with nothing after, must be text */
				return (false);
		}
		else if (gmtio_not_float (string[k]))	/* Cannot be part of a floating point number */
			return (false);
	}
	if (n_exp > 1) return (false);	/* Cannot have both e and d or more than one */
	if (max > 0.0) {	/* Check if inside range */
		double d = atof (string);
		return ((d > max) ? false : true);
	}
	else	/* Unlimited float OK */
		return (true);
}

 GMT_LOCAL unsigned int gmtio_is_geographic (struct GMT_CTRL *GMT, unsigned int type, char *text) {
	/* Determine if text might be one of these forms.
	 * If type is GMT_X we look for longitudes:
	 *   1) [±]<d>[:<m>[:<s.xxx>]][W|E]
	 *   2) [±]<d>[:<m.xxx>][W|E]
	 *   3) [±]<d.xxx>[W|E]
	 *   Note: Without trailing W|E, form (3) cannot be known to be a longitude since it is periodic.
	 * If type is GMT_Y we look for latitudes:
	 *   1) [±]<d>[:<m>[:<s.xxx>]][S|N]
	 *   2) [±]<d>[:<m.xxx>][S|N]
	 *   3) [±]<d.xxx>[S|N]
	 *   Note: Without trailing S|N, form (3) cannot be known to be a latitude even if <= ±90.
	 * We return GMT_IS_LON if truly a longitude, GMT_IS_LAT if truly a valid latitude,
	  * GMT_IS_GEO if we cannot tell them apart, GMT_IS_FLOAT is just a floating point number,
	  * and GMT_IS_TEXT is something we cannot understand.
	 */
	static char *trail[2] = {"WE", "SN"};
	char string[GMT_LEN128] = {""}, *c = NULL, *p = NULL;
	int explicit = GMT_NOTSET;
	int last = strlen (text) - 1, start = 0, n_exp = 0;
	int i_limit = 0;	
	double d_limit = 0.0;

	if (type == GMT_Y) {	/* Only latitude integer has a known upper limit as longitudes are periodic */
		i_limit = 90;	d_limit = 90.0;
	}
	if (last < 0) return (GMT_IS_TEXT);
	strncpy (string, text, GMT_LEN128-1);	/* A copy we can mess with */
	if (string[start] == '-' || string[start] == '+') start++;	/* Skip any leading signs */
	if (string[last] == 'W' || string[last] == 'E') {	/* Skip trailing "signs" W, E, S, N */
		string[last] = '\0';
		last--;
		explicit = GMT_IS_LON;
	}
	if (string[last] == 'S' || string[last] == 'N') {	/* Skip trailing "signs" W, E, S, N */
		string[last] = '\0';
		last--;
		explicit = GMT_IS_LAT;
	}
	else if (strchr (GMT_DIM_UNITS, string[last])) {	/* Might be dimension */
		string[last] = '\0';
		last--;
		explicit = GMT_IS_DIMENSION;
	}
	else if (strchr (GMT_LEN_UNITS, string[last])) {	/* Might be geo-dimension */
		string[last] = '\0';
		last--;
		explicit = GMT_IS_GEODIMENSION;
	}
	else if (strchr (GMT_TIME_UNITS, string[last])) {	/* Might be time units */
		string[last] = '\0';
		last--;
		explicit = GMT_IS_DURATION;
	}
	/* Check for valid and invalid letters */
	for (int j = start; j <= last; j++) {	/* Search for bad characters over valid range of string */
		if (isalpha (string[j])) {	/* Possibly something that is not part of a floating value */
			if (string[j] == 'd' || string[j] == 'e')
				n_exp++;	/* We are OK with one e or d for an exponential */
			else
				return (GMT_IS_STRING);	/* Not something that is part of a floating value */
		}
	}
	if (n_exp > 1) return (GMT_IS_STRING);	/* Most likely text that included more than one of d and e */

	/* Now examine the potential floating points or ddd:mm:ss stuff */
	p = c = &string[start];	/* Start of candidate degree float after any leading sign */
	while (c[0] && c[0] != ':') c++;	/* Wind onward to the first colon or the end of the string */
	if (c[0]) {	/* Found a colon marking the end of integer degrees i.e., ddd:mm[.xxx|:ss.xxx] */
		int d;
		*c = '\0';	/* Hide the trailing colon */
		if (!gmtio_is_valid_integer (p, i_limit)) return (GMT_IS_STRING);	/* Not an integer degree. */
		d = atoi (p);	/* Keep the degree value */
		c++;	/* Jump over the colon after ddd */
		p = c;	/* Start of minutes, if available */
		/* Next check for minutes */
		while (c[0] && c[0] != ':') c++;	/* Wind to next colon or the end of string */
		if (c[0]) {	/* Found another colon and can isolate integer minutes, with optional floating point ss.xxx to follow */
			*c = '\0';	/* Hide colon after the mm */
			if (!gmtio_is_valid_integer (p, 60)) return (GMT_IS_STRING);	/* Not an integer <= 60 */
			c++;	/* Jump over the colon */
			p = c;	/* Start of seconds, if available */
			if (p && !gmtio_is_float (GMT, p, false, 60.0)) return (GMT_IS_STRING);
		}
		else if (!gmtio_is_float (GMT, p, false, 60.0))	/* Cannot be a floating point number of minutes */
			return (GMT_IS_STRING);
		if (d > 90) return (GMT_IS_LON);	/* Valid and exceeds 90 so a longitude */
		if (explicit == GMT_X) return (GMT_IS_LON);	/* trailing W or E means longitude */
		if (explicit == GMT_Y) return (GMT_IS_LAT);	/* trailing S or N means latitude */
		return (GMT_IS_GEO);
	}
	else if (gmtio_is_float (GMT, p, true, d_limit))	/* Rely on explicit if set */
		switch (explicit) {
			case GMT_IS_DIMENSION:
			case GMT_IS_GEODIMENSION:
			case GMT_IS_DURATION:
				return (explicit);
				break;
			default:
				return (GMT_IS_FLOAT);
				break;
		}
	else if (!gmtio_is_float (GMT, p, true, d_limit))	/* Cannot be a floating point number */
		return (GMT_IS_STRING);
	return (GMT_IS_FLOAT);	/* Floating point number so could be a longitude but cannot know that just from text */
}


GMT_LOCAL char *gmtio_type_name (unsigned int kind) {
	char *t = NULL;
	switch (kind) {
		case GMT_IS_NAN: 		t = "NAN"; break;
		case GMT_IS_FLOAT: 		t = "FLOAT"; break;
		case GMT_IS_LAT: 		t = "LATITUDE"; break;
		case GMT_IS_LON: 		t = "LONGITUDE"; break;
		case GMT_IS_GEO: 		t = "GEOGRAPHIC"; break;
		case GMT_IS_RELTIME: 		t = "RELTIME"; break;
		case GMT_IS_ABSTIME: 		t = "ABSTIME"; break;
		case GMT_IS_RATIME: 		t = "RATIME"; break;
		case GMT_IS_DURATION: 		t = "DURATION"; break;
		case GMT_IS_DIMENSION:		t = "DIMENSION"; break;
		case GMT_IS_GEODIMENSION:	t = "GEODIMENSION"; break;
		case GMT_IS_STRING:		t = "STRING"; break;
		case GMT_IS_UNKNOWN:		t = "UNKNOWN"; break;
	}
	return (t);
}

GMT_LOCAL unsigned int gmtio_is_time (struct GMT_CTRL *GMT, char *text) {
	/* Detect if we are given an absolute datetime string */
	bool is_float;
	unsigned int start = 0, n_colon, n_dash, n_slash, n_items, k, i_limit = 0, Li, L = strlen (text) - 1;
	char string[GMT_LEN128] = {""}, *c = NULL, *p = NULL, C[6][GMT_LEN64] = {""};
	double d_limit = 0.0;
	strncpy (string, text, GMT_LEN128-1);	/* A copy we can mess with */

	if (L > 24U) return (GMT_IS_STRING);	/* Cannot be time related */
	if (L < 4U)  return (GMT_IS_STRING);	/* Cannot be time related */
	if (string[start] == '-' || string[start] == '+') start++;	/* Skip any leading signs */
	p = &string[start];	/* Start of arg after skipping sign */

	n_colon = gmt_count_char (GMT, p, ':');
	n_dash  = gmt_count_char (GMT, p, '-');
	n_slash = gmt_count_char (GMT, p, '/');
	for (k = start; p[k] && strchr ("-/", p[k]) == NULL; k++);	/* Find first dash or slash, if there are any */
	if (p[k])
		gmt_strrepc (string, p[k], ' ');	/* Replace date separators with space */
	if (n_colon)
		gmt_strrepc (string, ':', ' ');	/* Replace time separators with space */
	if ((n_dash >= 1 && n_slash == 0) || n_dash == 0 && n_slash >= 1) {
		/* Apart from random junk SHIT 5 6 7:X:Hello!, Possibilities are:
		 *	1.  yyyy mm dd[T][hh.xxx|:mm.xx|:ss.xx]  Full date and possibly time
		 *	2.  yyyy jjj[T][hh.xxx|:mm.xx|:ss.xx]  Julian day and possibly time
		 *	3.  yyyy mm[T][hh.xxx|:mm.xx|:ss.xx]  Full year month, implicit day = 1 and possibly time
		 *	4.  yyyy[T][hh.xxx|:mm.xx|:ss.xx]  Full year, implicit month = day = 1 and possibly time
		 *	5.  yyyyT  Same as (4) but no time: Start of year
		 *	6.  yyyy  Cannot tell if time so return float.
		 */

		/* Find the 'T' if present */
		if ((c = strstr (p, "OCT"))) {	/* Watch out for the second T instead */
	 		char *q = gmt_strrep (string, "OCT", "Oct");
	 		strncpy (string, q, GMT_LEN128);
	 		gmt_M_str_free (q);
			p = &string[start];	/* Start of arg after skipping sign */
	 	}
	 	if ((c = strchr (p, 'T'))) c[0] = ' ';	/* Remove the possible date/time separator (not so if jun) */
	 	/* Get the first 3 items */
		n_items = sscanf (p, "%s %s %s %s %s %s", &(C[0]), &(C[1]), &(C[2]), &(C[3]), &(C[4]), &(C[5]));	/* Hope to get year month day hh oo ss.xx */
		/* n_items = 6: yyyy oo dd hh mm ss.xxx
		 * n_items = 5: yyyy oo dd hh mm.xxx or yyyy oo hh mm ss.xxx or yyyy jj hh mm ss.xxx
		 *  n_items = 4: yyyy oo dd hh.xxx or yyyy oo hh mm.xxx or yyyy jj hh mm.xxx
		 *  n_items = 3: yyyy oo dd or yyyy oo hh.xxx or yyyy jj hh.xxx
		 *  n_items = 2: yyyy oo or yyyy jj
		 *  n_items = 1: yyyy
		 */
		for (k = 0; k < n_items; k++) {
			Li = strlen (C[k]);	i_limit = 0;	d_limit = 0.0;
			is_float = (strchr (C[k], '.') != NULL);
			switch (Li) {
				case 0: case 4:	break;
				case 1: case 2:	i_limit = 60;	break;
				case 3:	i_limit = 366;	break;
				default: d_limit = (is_float) ? 60.0 : 0.0;	break;
			}
			if (is_float) {
				if (!gmtio_is_float (GMT, C[k], false, d_limit)) return (GMT_IS_TEXT);
			}
			else if (!gmtio_is_valid_integer (C[k], i_limit)) {	/* Not an integer */
				if (!(strlen (C[k]) == 3 && isalpha (C[k][0])))	return (GMT_IS_STRING);	/* Allow 3-char months to pass */
			}
		}
	}
	return (GMT_IS_ABSTIME);
}

GMT_LOCAL unsigned int gmtio_is_special (struct GMT_CTRL *GMT, char *string) {
	/* Count special characters that are not part of coordinates */
	char *p = NULL;
	int code;
	unsigned int L = strlen (string), start = 0, n_special = 0, k;

	if (string[start] == '-' || string[start] == '+') start++;	/* Skip any leading signs */
	p = &string[start];	/* Start of arg after skipping potential signs */

	for (k = start; k < L; k++) {
		code = (int)p[k];
		if (code < '.') n_special++;
		else if (code > ';' && code < 'A') n_special++;
		else if (code > 'Z' && code < 'a') n_special++;
		else if (code > 'z' ) n_special++;
	}
	return (n_special);
}

GMT_LOCAL unsigned int gmtio_is_string (struct GMT_CTRL *GMT, char *string) {
	/* Apply basic checking for stuff that cannot be coordinates */
	char *p = NULL;
	int code;
	unsigned int L = strlen (string), start = 0, k;
	unsigned int n_text = 0, n_colons = 0, n_digits = 0, n_dashes = 0, n_slashes = 0, n_specials = 0, n_periods = 0;

	if (L > 24U) return (GMT_IS_STRING);	/* Too long to be an absolute time string */

	if (string[start] == '-' || string[start] == '+') start++;	/* Skip any leading signs */
	p = &string[start];	/* Start of arg after skipping potential signs */

	for (k = start; k < L; k++) {
		if (isalpha (string[k])) n_text++;
		if (isdigit (string[k])) n_digits++;
	}
	if (n_digits == 0) return (GMT_IS_STRING);		/* Cannot be coordinates */
	if (n_text > 4) return (GMT_IS_STRING);			/* Max letters in abs.time is 4, e.g. 2022-NOV-13T */
	n_specials = gmtio_is_special (GMT, p);			/* Non-printables, hashtags, semi-columns, parentheses, etc. */
	if (n_specials) return (GMT_IS_STRING);			/* Cannot be coordinates */
	n_periods = gmt_count_char (GMT, p, '.');		/* How many periods. No valid coordinate has more than one */
	if (n_periods > 1) return (GMT_IS_STRING);		/* Cannot be coordinates */
	n_colons = gmt_count_char (GMT, p, ':');		/* Number of colons.  Max is 2 for hh:mm:ss */
	if (n_colons > 2) return (GMT_IS_STRING);		/* Cannot be coordinates */
	n_dashes = gmt_count_char (GMT, p, '-');		/* Number of dashes.  Max is 2 for yyyy-mm-dd and  no slashes */
	n_slashes = gmt_count_char (GMT, p, '/');		/* Number of slashes.  Max is 2 for yyyy/mm/dd and no dashes*/
	if ((n_dashes + n_slashes) > 2) return (GMT_IS_STRING);	/* Cannot be coordinates */
	return (GMT_IS_FLOAT);	/* But could be any coordinate type */
}

GMT_LOCAL unsigned int gmtio_is_what (struct GMT_CTRL *GMT, unsigned int type, char *text) {
	unsigned int code = gmtio_is_string (GMT, text);
	if (code == GMT_IS_STRING) return (code);	/* Detect junk first */
	if (type >= GMT_X && type <= GMT_Z)
		return (gmtio_is_geographic (GMT, type, text));
	else
		return (gmtio_is_time (GMT, text));
}

void gmtio_testscanner (struct GMT_CTRL *GMT, char *file)
{	/* Add this near top of gmt convert for debug */
	int k, c;
	unsigned int kind;
	FILE *fp = fopen (file, "r");
	char line[GMT_LEN256] = {""};
	unsigned int type = GMT_X;
	while (gmt_fgets (GMT, line, GMT_LEN256, fp)) {
		if (line[0] == '#') {
			if (strstr (line, "LONGITUDE"))
				type = GMT_X;
			else if (strstr (line, "LATITUDE"))
				type = GMT_Y;
			else if (strstr (line, "DIMENSION"))
				type = GMT_Z;
			else if (strstr (line, "ABSTIME"))
				type = 3;
			fprintf (stderr, "%s", line);
			continue;
		}
		gmt_chop (line);
		k = strlen (line) - 1;
		while (line[k] != '|') k--;
		c = k + 1;	k--;
		while (line[k] == ' ' || line[k] == '\t') k--;
		k++;	line[k] = '\0';
		fprintf (stderr, "%30s", line);
		fprintf (stderr, "%14s\t", &line[c]);
		kind = gmtio_is_what (GMT, type, line);
		fprintf (stderr, "%14s\n", gmtio_type_name(kind));
	}
	fclose (fp);
}
