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
 * Public function prototypes for GMT API option parsing.
 *
 * Author: 	Paul Wessel
 * Date:	1-JUN-2010
 * Version:	5
 *
 * The API presently consists of 52 documented functions.  For a full
 * description of the API, see the GMT_API documentation.
 * These functions have Fortran bindings as well, provided you add
 * -DFORTRAN_API to the C preprocessor flags [in ConfigUser.cmake].
 *
 * Here lie the 13 public functions used for GMT API command parsing:
 *
 * GMT_Create_Options   : Convert an array of text args to a linked option list
 * GMT_Destroy_Options  : Delete the linked option list
 * GMT_Create_Args      : Convert a struct option list back to an array of text args
 * GMT_Destroy_Args     : Delete the array of text args
 * GMT_Create_Cmd       : Convert a struct option list to a single command text.
 * GMT_Destroy_Cmd      : Delete the command string
 * GMT_Make_Option      : Create a single option structure given arguments
 * GMT_Find_Option      : Find a specified option in the linked option list
 * GMT_Update_Option    : Update the arguments of the specified option in the list
 * GMT_Append_Option    : Append the given option to the end of the structure list
 * GMT_Delete_Option    : Delete the specified option and adjust the linked list
 * GMT_Parse_Common     : Parse the common GMT options
 * GMT_Expand_Option    : Replace special marker (?) with naemd argument [external API only]

 * This part of the API helps the developer create, manipulate, modify, find, and
 * update options that will be passed to various GMT_* modules ("The GMT programs").
 * This involves converting from text arrays (e.g., argc, argv[]) to the linked list
 * of structures used by these functions, manipulate these lists, and perhaps even
 * creating text arrays from the linked list.  All these functions are pass the
 * API pointer and if that is NULL then errors will be issued.
 *
 * There are no GMT devel functions exported by this file.
 */

/*!
 * \file gmt_parse.c
 * \brief Public function prototypes for GMT API option parsing.
 */

#include "gmt_dev.h"
#include "gmt_internals.h"

static char *GMT_unique_option[GMT_N_UNIQUE] = {	/* The common GMT command-line options [ just the subset that accepts arguments (e.g., -O is not listed) ] */
#include "gmt_unique.h"
};

EXTERN_MSC int gmtapi_report_error	(void *C, int error);	/* Lives in gmt_api.c */

/* Some private macros and inline function used in this file */
#define ptr_return(API,err,ptr)   { gmtapi_report_error(API,err); return (ptr);}
#define return_null(API,err)      { gmtapi_report_error(API,err); return (NULL);}
#define return_error(API,err)     { gmtapi_report_error(API,err); return (err);}
#define return_value(API,err,val) { gmtapi_report_error(API,err); return (val);}

static inline struct GMTAPI_CTRL * parse_get_api_ptr (struct GMTAPI_CTRL *ptr) {return (ptr);}

/* Local functions */

/*! . */
GMT_LOCAL int parse_B_arg_inspector (struct GMT_CTRL *GMT, char *in) {
	/* Determines if the -B option indicates old GMT4-style switches and flags
	 * or if it follows the GMT 5 specification.  This is only called when
	 * compatibility mode has been selected; otherwise we only check GMT 5
	 * style arguments.  We return 5 for GMT5 style, 4 for GMT4 style, 9
	 * if no decision could be make and -1 if mixing of GMT4 & 5 styles are
	 * found, which is an error. */
	size_t k, j, last;
	int gmt4 = 0, gmt5 = 0, n_digits = 0, n_colons = 0, n_slashes = 0, colon_text = 0, wesn_at_end = 0;
	bool ignore = false;	/* true if inside a colon-separated string under GMT4 style assumption */
	bool ignore5 = false;	/* true if label, title, prefix, suffix */
	bool custom = false;	/* True if -B[p|s][x|y|z]c<filename> was given; then we relax checing for .c (old second) */
	char mod = 0;

	if (!in || in[0] == 0) return (9);	/* Just a safety precaution, 9 means "either" syntax but it is an empty string */
	last = strlen (in);
	for (k = 0; k < last; k++) {	/* Count digits.  If none then it is GMT5 and we can return */
		if (isdigit (in[k])) n_digits++;
	}
	if (n_digits == 0) return (5);	/* If no numbers are given then we know it is GMT5 */
	k = (in[0] == 'p' || in[0] == 's') ? 1 : 0;	/* Skip p|s in -Bp|s */
	if (strchr ("xyz", in[k])) gmt5++;		/* Definitively GMT5 */
	if (k == 0 && !isdigit (in[0]) && strchr ("WESNwesn", in[1])) gmt5++;		/* Definitively GMT5 */
	j = k;
	while (j < last && (in[j] == 'x' || in[j] == 'y' || in[j] == 'z')) j++;
	custom = (in[j] == 'c');	/* Got -B[p|s][xyz]c<customfile> */
	for (k = 0; k < last; k++) {
		if (k && in[k] == '+' && in[k-1] == '@') {	/* Found a @+ PSL sequence, just skip */
			continue;	/* Resume processing */
		}
		if (ignore5) continue;	/* Don't look inside a GMT5 title, label, suffix, or prefix */
		if (in[k] == ':') {
#ifdef _WIN32		/* Filenames under Windows may be X:\<name> which should not trigger "colon" test */
			if (!(k && isalpha (in[k-1]) && k < last && in[k+1] == '\\'))
#endif
			if (k && in[k-1] == '@') {	/* Found a @:[<font>]: sequence, scan to end */
				k++;	/* Skip past the colon */
				while (in[k] && in[k] != ':') k++;	/* Find the matching colon */
				continue;	/* Resume processing */
			}
			ignore = !ignore, n_colons++;	/* Possibly stepping into a label/title */
			if (!ignore) continue;	/* End of title or label, skip check for next character */
			if (k < last && in[k+1] == '.') colon_text++;	/* Title */
			else if (k < last && in[k+1] == '=') colon_text++;	/* Annotation prefix */
			else if (k < last && in[k+1] == ',') colon_text++;	/* Annotation suffix */
		}
		if (ignore) continue;	/* Don't look inside a title or label */
		switch (in[k]) {
			case '/': if (mod == 0) n_slashes++; break;	/* Only GMT4 uses slashes */
			case '+':	/* Plus, might be GMT5 modifier switch */
				if      (k < last && in[k+1] == 'u') {mod = 'u'; ignore5 = true;  gmt5++;}	/* unit (suffix) settings */
				else if (k < last && in[k+1] == 'b') {mod = 'b'; ignore5 = false; gmt5++;}	/* 3-D box settings */
				else if (k < last && in[k+1] == 'g') {mod = 'g'; ignore5 = false; gmt5++;}	/* fill settings */
				else if (k < last && in[k+1] == 'o') {mod = 'o'; ignore5 = false; gmt5++;}	/* oblique pole settings */
				else if (k < last && in[k+1] == 'p') {mod = 'p'; ignore5 = true;  gmt5++;}	/* prefix settings */
				else if (k < last && in[k+1] == 'l') {mod = 'l'; ignore5 = true;  gmt5++;}	/* Label */
				else if (k < last && in[k+1] == 'L') {mod = 'L'; ignore5 = true;  gmt5++;}	/* Forced horizontal Label */
				else if (k < last && in[k+1] == 's') {mod = 's'; ignore5 = true;  gmt5++;}	/* Secondary label */
				else if (k < last && in[k+1] == 'S') {mod = 'S'; ignore5 = true;  gmt5++;}	/* Forced horizontal Secondary lLabel */
				else if (k < last && in[k+1] == 't') {mod = 't'; ignore5 = true;  gmt5++;}	/* title */
				else if (k < last && in[k+1] == 'n') {mod = 'n'; ignore5 = true;  gmt5++;}	/* Turn off frames and annotations */
				else if (k && (in[k-1] == 'Z' || in[k-1] == 'z')) {ignore5 = false; gmt4++;}	/* Z-axis with 3-D box */
				break;
			case 'c':	/* If following a number this is unit c for seconds in GMT4 */
				if (!custom && k && (in[k-1] == '.' || isdigit (in[k-1]))) gmt4++;	/* Old-style second unit */
				break;
			case 'W': case 'E': case 'S': case 'N': case 'Z': case 'w': case 'e': case 'z':	/* Not checking s as confusion with seconds and n because of +n */
				if (k > 1) wesn_at_end++;	/* GMT5 has -B<WESNwesn> up front while GMT4 usually has them at the end */
				break;
			case 'n':	/* Tell this apart from +n */
				if (!(k && in[k-1] == '+') && k > 1) wesn_at_end++;	/* GMT5 has -B<WESNwesn> up front while GMT4 usually has them at the end */
				break;
		}
	}
	if (!gmt5 && wesn_at_end) gmt4++;		/* Presumably got WESNwesn stuff towards the end of the option */
	if (n_colons && (n_colons % 2) == 0) gmt4++;	/* Presumably :labels: in GMT4 style as any +mod would have kicked in above */
	if (!custom && n_slashes) gmt4++;		/* Presumably / to separate axis in GMT4 style */
	if (colon_text) gmt4++;				/* Gave title, suffix, prefix in GMT4 style */
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "parse_B_arg_inspector: GMT4 = %d vs. GMT = %d\n", gmt4, gmt5);
	if (gmt5 && !gmt4) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "parse_B_arg_inspector: Detected GMT >=5 style elements in -B option\n");
		return (5);
	}
	else if (gmt4 && !gmt5) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "parse_B_arg_inspector: Detected GMT 4 style elements in -B option\n");
		return (4);
	}
	else if (gmt4 && gmt5) {	/* Mixed case is never allowed */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "parse_B_arg_inspector: Detected both GMT 4 and >= style elements in -B option. Unable to parse.\n");
		if (n_slashes) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "parse_B_arg_inspector: Slashes no longer separate axis specifications, use -B[xyz] and repeat\n");
		if (colon_text || n_colons) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "parse_B_arg_inspector: Colons no longer used for titles, labels, prefix, and suffix; see +t, +l, +p, +s\n");
		return (-1);
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "parse_B_arg_inspector: Assume new GMT style format in -B option\n");
		return (9);
	}
}

/*! . */
GMT_LOCAL int parse_check_b_options (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {
	/* Determine how many -B options were given and if it is clear we are
	 * dealing with GMT4 or GMT5 syntax just by looking at this information.
	 * We also examine each argument for clues to version compatibility.
	 * We return 5 if it is clear this is GMT5 syntax, 4 if it is clear it
	 * is GMT 4 syntax, 9 if we cannot tell, and -1 if it is a mix of both.
	 * The latter will result in a syntax error.
	 */
	struct GMT_OPTION *opt = NULL;
	unsigned int n4_expected = 0, n_B = 0, gmt4 = 0, gmt5 = 0, k;
	int verdict;

	for (opt = options; opt; opt = opt->next) {	/* Loop over all given options */
		if (opt->option != 'B') continue;	/* Skip anything but -B options */
		n_B++;					/* Count how many (max 2 in GMT4 if -Bp|s given) */
		k = (opt->arg[0] == 'p' || opt->arg[0] == 's') ? 1 : 0;	/* Step over any p|s designation */
		if (k == 1) n4_expected++;		/* Count how many -Bp or -Bs were given */
		verdict = parse_B_arg_inspector (GMT, opt->arg);	/* Check this argument, return 5, 4, 1 (undetermined) or 0 (mixing) */
		if (verdict == 4) gmt4++;
		if (verdict == 5) gmt5++;
		if (verdict == -1) gmt4++, gmt5++;	/* This is bad and will lead to a syntax error */
	}
	if (n4_expected == 0) n4_expected = 1;	/* If there are no -Bs|p options then GMT4 expects a single -B option */
	if (n_B > n4_expected) gmt5++;		/* Gave more -B options than expected for GMT4 */
	if (gmt5 && !gmt4)  return 5;		/* Matched GMT5 syntax only */
	if (gmt4 && !gmt5)  return 4;		/* Matched GMT4 syntax only */
	if (!gmt4 && !gmt5) return 9;		/* Could be either */
	return (-1);				/* Error: Cannot be both */
}

GMT_LOCAL unsigned int count_slashes (char *txt) {
	unsigned int i, n;
	for (i = n = 0; txt[i]; i++) if (txt[i] == '/') n++;
	return (n);
}

/*! . */
GMT_LOCAL unsigned int parse_check_extended_R (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {
	/* In order to use -R[L|C|R][B|M|T]<lon0>/<lat0>/<n_columns>/<ny> we need access
	 * to grid increments dx/dy, usually given via a -I option.  Hence, we here
	 * make sure that if such a -R option is given we first process -I */

	struct GMT_OPTION *opt = NULL;
	bool got_extended_R = false;
	for (opt = options; opt; opt = opt->next) {
		if (opt->option == 'R' && count_slashes (opt->arg) == 3 && strchr ("LCRlcr", opt->arg[0]) && strchr ("TMBtmb", opt->arg[1]))
			got_extended_R = true;
	}
	if (!got_extended_R) return 0;	/* No such situation */

	/* Now look for -Idx[/dy] option */
	for (opt = options; opt; opt = opt->next) {
		if (opt->option == 'I' && opt->arg[0]) {
			if (!gmt_getinc (GMT, opt->arg, GMT->common.R.inc)) {	/* Successful parsing */
				GMT->common.R.active[ISET] = true;
			}
		}
	}
	if (GMT->common.R.active[ISET])
		return 0;
	GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-R[L|C|R][T|M|B]<x0>/<y0>/<n_columns>/<ny> requires grid spacings via -I\n");
	return 1;
}

#define Return { GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Found no history for option -%s\n", str); return (-1); }

/*! . */
GMT_LOCAL int parse_complete_options (struct GMT_CTRL *GMT, struct GMT_OPTION *options) {
	/* Go through the given arguments and look for shorthands,
	 * i.e., -B, -J, -R, -X, -x, -Y, -c, -p. given without arguments.
	 * If found, see if we have a matching command line history and then
	 * update that entry in the option list.
	 * Finally, keep the option arguments in the history list.
	 * However, when func_level > GMT_TOP_MODULE, do update the entry, but do not
	 * remember it in history. Note, there are two special cases here:
	 * -J is special since we also need to deal with the sub-species
	 *    like -JM, -JX, etc.  So these all have separate entries.
	 * -B is special because the option is repeatable for different
	 *    aspects of the basemap.  We concatenate all of them to store
	 *    in the history file and use RS = ASCII 30 as separator.
	 *    Also, a single -B in the options may expand to several
	 *    separate -B<args> so the linked options list may grow.
	 */
	int id = 0, k, n_B = 0, B_id, update_id = 0;
	unsigned int pos = 0, B_replace = 1;
	bool update, remember, check_B;
	struct GMT_OPTION *opt = NULL, *opt2 = NULL, *B_next = NULL;
	char str[3] = {""}, B_string[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""}, B_delim[2] = {30, 0};	/* Use ASCII 30 RS Record Separator between -B strings */

	remember = (GMT->hidden.func_level == GMT_TOP_MODULE);   /* Only update the history for top level function */

	for (opt = options; opt; opt = opt->next) {
		if (opt->option == 'B') {	/* Do some initial counting of how many -B options and determine if there is just one with no args */
			if (n_B > 0 || opt->arg[0]) B_replace = 0;
			n_B++;
		}
	}
	for (k = 0, B_id = GMT_NOTSET; k < GMT_N_UNIQUE && B_id == GMT_NOTSET; k++)
		if (!strcmp (GMT_unique_option[k], "B")) B_id = k;	/* B_id === 0 but just in case this changes we do this search anyway */

	check_B = (strncmp (GMT->init.module_name, "psscale", 7U) && strncmp (GMT->init.module_name, "docs", 4U));
	if (GMT->current.setting.run_mode == GMT_MODERN && n_B && check_B) {	/* Write gmt.frame file unless module is psscale, overwriting any previous file */
		char file[PATH_MAX] = {""};
		FILE *fp = NULL;
		snprintf (file, PATH_MAX, "%s/gmt%d.%s/gmt.frame", GMT->parent->session_dir, GMT_MAJOR_VERSION, GMT->parent->session_name);
		if ((fp = fopen (file, "w")) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Unable to create file %s\n", file);
			return (-1);
		}
		for (k = 0, opt = options; opt; opt = opt->next) {
			if (opt->option != 'B') continue;
			if (k) fprintf (fp, "%s", B_delim);
			fprintf (fp, "%s", opt->arg);
			k++;
		}
		fprintf (fp, "\n");
		fclose (fp);
	}
	for (opt = options; opt; opt = opt->next) {
		if (!strchr (GMT_SHORTHAND_OPTIONS, opt->option)) continue;	/* Not one of the shorthand options */
		if (GMT->current.setting.run_mode == GMT_MODERN && opt->option == 'B') continue;	/* The -B option is NOT a shorthand option under modern mode */
		update = false;
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "History: Process -%c%s\n", opt->option, opt->arg);

		str[0] = opt->option; str[1] = str[2] = '\0';
		if (opt->option == 'J') {               /* -J is special since it can be -J or -J<code> */

			/* Always look up "J" first. It comes before "J?" and tells what the last -J was */
			if ((id = gmt_get_option_id (0, str)) == GMT_NOTSET) Return;	/* No -J found at all - nothing more to do */
			if (opt->arg && opt->arg[0]) {      /* Gave -J<code>[<args>] so we either use or update history and continue */
				str[1] = opt->arg[0];
				/* Remember this last -J<code> for later use as -J, but do not remember it when -Jz|Z */
				if (remember) {
					if (str[1] == 'Z' || str[1] == 'z') {
						int z_id = gmt_get_option_id (0, "Z");
						gmt_M_str_free (GMT->init.history[z_id]);
						GMT->init.history[z_id] = strdup (&str[1]);
					}
					else {
						gmt_M_str_free (GMT->init.history[id]);
						GMT->init.history[id] = strdup (&str[1]);
					}
				}
				if (opt->arg[1]) update = true; /* Gave -J<code><args> so we want to update history and continue */
			}
			else {
				if (!GMT->init.history[id]) Return;
				str[1] = GMT->init.history[id][0];
			}
			/* Continue looking for -J<code> */
			if ((id = gmt_get_option_id (id + 1, str)) == GMT_NOTSET) Return;	/* No -J<code> found */
			update_id = id;
		}
		else if (opt->option == 'B') {          /* -B is also special since there may be many of these, or just -B */
			if (B_replace) {                    /* Only -B is given and we want to use the history */
				if (B_replace == 2) continue;   /* Already done this */
				if (!GMT->init.history[B_id]) Return;
				opt2 = opt;                     /* Since we don't want to change the opt loop avove */
				B_next = opt->next;             /* Pointer to option following the -B option */
				gmt_M_str_free (opt2->arg);/* Free previous pointer to arg */
				if (gmt_strtok (GMT->init.history[B_id], B_delim, &pos, p))	/* Get the first argument */
					opt2->arg = strdup (p);         /* Update arg */
				while (gmt_strtok (GMT->init.history[B_id], B_delim, &pos, p)) {	/* Parse any additional |<component> statements */
					opt2->next = GMT_Make_Option (GMT->parent, 'B', p);	/* Create new struct */
					opt2->next->previous = opt2;
					opt2 = opt2->next;
				}
				opt2->next = B_next;            /* Hook back onto main option list */
				B_replace = 2;                  /* Flag to let us know we are done with -B */
			}
			else {	/* One of possibly several -B<arg> options; concatenate and separate by RS */
				if (B_string[0]) strcat (B_string, B_delim);	/* Add RS separator between args */
				strncat (B_string, opt->arg, GMT_LEN256-1);
			}
		}
		else {	/* Gave -R[<args>], -V[<args>] etc., so we either use or update the history and continue */
			update_id = id = gmt_get_option_id (0, str);	/* If -R then we get id for RP */
			if (id == GMT_NOTSET) Return;	/* Error: user gave shorthand option but there is no record in the history */
			if (GMT->current.setting.run_mode == GMT_MODERN && opt->option == 'R') {	/* Must deal with both RP and RG under modern mode */
				if (GMT->current.ps.active || !strncmp (GMT->init.module_name, "subplot", 7U) || !strncmp (GMT->init.module_name, "pscoast", 7U) || !strncmp (GMT->init.module_name, "psbasemap", 9U)) {	/* Plotting module plus special options to pscoast and psbasemap: First check RP if it exists */
					if (!GMT->init.history[id]) id++;	/* No RP in the history, try RG next */
				}
				else {	/* Only try RG for non-plotting modules. RG follows RP in gmt_unique.h order [Modern mode only] */
					id++;	/* Go to RG */
					update_id = id;	/* We want to update the RG history since it is not a plotter */
				}
			}
			if (opt->arg && opt->arg[0]) update = true;	/* Gave -R<args>, -V<args> etc. so we we want to update history and continue */
		}
		if (opt->option != 'B') {               /* Do -B separately again after the loop so skip it here */
			if (id < 0) Return;                 /* Error: user gave shorthand option but there is no record in the history */
			if (update) {                       /* Gave -J<code><args>, -R<args>, -V<args> etc. so we update history and continue */
				if (remember) {
					gmt_M_str_free (GMT->init.history[update_id]);
					GMT->init.history[update_id] = strdup (opt->arg);
				}
			}
			else {	/* Gave -J<code>, -R, -J etc. so we complete the option and continue */
				if (!GMT->init.history[id]) Return;
				gmt_M_str_free (opt->arg);   /* Free previous pointer to arg */
				opt->arg = strdup (GMT->init.history[id]);
				if (id != update_id) {	/* Happens for -R when we need the RG settings as first-time RP setting via -R */
					char *c = NULL;
					if ((c = strstr (opt->arg, "+I")) || (c = strstr (opt->arg, "+G")))	/* Strip off increment/node setting when used for plot domain */
						c[0] = '\0';
					if (remember) GMT->init.history[update_id] = strdup (opt->arg);
				}
			}
		}
	}

	if (B_string[0] && B_id >= 0) {	/* Got a concatenated string with one or more individual -B args, now separated by the RS character (ASCII 30) */
		gmt_M_str_free (GMT->init.history[B_id]);
		GMT->init.history[B_id] = strdup (B_string);
	}

	return (GMT_NOERROR);
}

/*----------------------------------------------------------|
 * Public functions that are part of the GMT API library    |
 *----------------------------------------------------------|
 */
#ifdef DEBUG
/*! . */
int GMT_List_Args (void *V_API, struct GMT_OPTION *head) {
	/* This function dumps the options to stderr for debug purposes.
	 * It is not meant to be part of the API but to assist developers
	 * during the debug phase of development.
	 */

	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = parse_get_api_ptr (V_API);

	if (head == NULL) return_error (API, GMT_OPTION_LIST_NULL);

	fprintf (stderr, "Options:");
	for (opt = head; opt; opt = opt->next) {
		if (!opt->option) continue;			/* Skip all empty options */
		if (opt != head) fprintf (stderr, " ");
		if (opt->option == GMT_OPT_SYNOPSIS)		/* Produce special - command for synopsis */
			fprintf (stderr, "-");
		else if (opt->option == GMT_OPT_INFILE)		/* Option for input filename [or number] */
			fprintf (stderr, "%s", opt->arg);
		else if (opt->arg && opt->arg[0])			/* Regular -?arg commandline argument */
			fprintf (stderr, "-%c%s", opt->option, opt->arg);
		else							/* Regular -? commandline argument */
			fprintf (stderr, "-%c", opt->option);

	}
	fprintf (stderr, "\n");

	return (GMT_OK);	/* No error encountered */
}
#endif

/*! . */
GMT_LOCAL struct GMT_OPTION *fix_gdal_files (struct GMT_OPTION *opt) {
	char *pch = NULL;
	if (((pch = strstr (opt->arg, "+b")) != NULL) && !strstr(opt->arg, "=gd")) {
		/* Here we deal with the case that the filename has embedded a band request for gdalread, as in img.tif+b1
		   However, the issue is that for these cases the machinery is set only to parse the request in the
		   form of img.tif=gd+b1 so the trick is to insert the missing '=gd' in the filename and let t go.
		   JL 29-November 2014
		*/
		char t[GMT_LEN256] = {""};
		pch[0] = '\0';
		strncpy (t, opt->arg, GMT_LEN256-1);
		strcat (t, "=gd");
		pch[0] = '+';			/* Restore what we have erased 2 lines above */
		strncat(t, pch, GMT_LEN256-1);
		gmt_M_str_free (opt->arg);	/* free it so that we can extend it */
		opt->arg = strdup (t);
	}
	return opt;
}

/*! . */
struct GMT_OPTION *GMT_Create_Options (void *V_API, int n_args_in, const void *in) {
	/* This function will loop over the n_args_in supplied command line arguments (in) and
	 * then creates a linked list of GMT_OPTION structures for each program option.
	 * These will in turn will be processed by the module-specific option parsers.
	 * What actually happens is controlled by n_args_in.  There are three cases:
	 * n_args_in < 0 : This means that in is already a linked list and we just duplicate and return a copy.
	 * n_args_in == 0: in is a single text string with multiple options (e.g., "-R0/2/0/5 -Jx1 -O -K > file")
	 *		   and we must first break this command string into separate words.
	 * n_args_in > 0 : in is an array of text strings (e.g., argv[]).
	 *
	 * Note: 1. If argv[0] is the calling program name, make sure to pass argc-1, args+1 instead.
	 *	 2. in == NULL is allowed only for n_args_in <= 0 (an empty list of options).
	 *
	 * The linked list returned by GMT_Create_Options should be freed by GMT_Destroy_Options().
	 */

	int error = GMT_OK;
	unsigned int arg, first_char, append = 0, n_args;
	char option, **args = NULL, **new_args = NULL, *this_arg = NULL, buffer[GMT_LEN1024] = {""};
	struct GMT_OPTION *head = NULL, *new_opt = NULL;
	struct GMT_CTRL *G = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);		/* GMT_Create_Session has not been called */
	if (in == NULL && n_args_in > 1) return_null (V_API, GMT_ARGV_LIST_NULL);	/* Gave no argument pointer but said we had at least 1 */
	if (in == NULL) return (NULL);	/* Gave no argument pointer so a null struct is returned */
	API = parse_get_api_ptr (V_API);	/* Convert V_API to a GMTAPI_CTRL pointer */
	if (n_args_in < 0) {	/* Already was given a linked list.  Duplicate and return the copy */
		struct GMT_OPTION *head_in = (struct GMT_OPTION *)in;
		while (head_in) {
			if ((new_opt = GMT_Make_Option (API, head_in->option, head_in->arg)) == NULL)	/* Make option with the listing name flagged as option -= */
				return_null (API, API->error);	/* Create the new option structure given the args, or return the error */
			if ((head = GMT_Append_Option (API, new_opt, head)) == NULL)		/* Hook new option to the end of the list (or initiate list if new == NULL) */
				return_null (API, API->error);	/* Create the new option structure given the args, or return the error */
			head_in = head_in->next;	/* Go to next input option */
		}
		return (head);
	}

	G = API->GMT;	/* GMT control structure */
	if (n_args_in == 0) {	/* Check if a single command line, if so break into tokens */
		unsigned int pos = 0, new_n_args = 0, k;
		bool quoted;
		size_t n_alloc = GMT_SMALL_CHUNK;
		char p[GMT_LEN1024] = {""}, *txt_in = strdup (in);	/* Passed a single text string */
		new_args = gmt_M_memory (G, NULL, n_alloc, char *);
		/* txt_in can contain options that take multi-word text strings, e.g., -B+t"My title".  We avoid the problem of splitting
		 * these items by temporarily replacing spaces inside quoted strings with ASCII 31 US (Unit Separator), do the strtok on
		 * space, and then replace all ASCII 31 with space at the end (we do the same for tab using ASCII 29 GS (group separator) */
		for (k = 0, quoted = false; txt_in[k]; k++) {
			if (txt_in[k] == '\"') quoted = !quoted;	/* Initially false, becomes true at start of quote, then false when exit quote */
			else if (quoted && txt_in[k] == '\t') txt_in[k] = GMT_ASCII_GS;
			else if (quoted && txt_in[k] == ' ')  txt_in[k] = GMT_ASCII_US;
		}
		while ((gmt_strtok (txt_in, " ", &pos, p))) {	/* Break up string into separate words, and strip off double quotes */
			unsigned int i, o;
			for (k = 0; p[k]; k++)
				if (p[k] == GMT_ASCII_GS) p[k] = '\t'; else if (p[k] == GMT_ASCII_US) p[k] = ' ';	/* Replace spaces and tabs masked above */
			for (i = o = 0; p[i]; i++) if (p[i] != '\"') p[o++] = p[i];	/* Ignore double quotes */
			p[o] = '\0';
			new_args[new_n_args++] = strdup (p);
			if (new_n_args == n_alloc) {
				n_alloc += GMT_SMALL_CHUNK;
				new_args = gmt_M_memory (G, new_args, n_alloc, char *);
			}
		}
		for (k = 0; txt_in[k]; k++)	/* Restore input string to prestine condition */
			if (txt_in[k] == GMT_ASCII_GS) txt_in[k] = '\t'; else if (txt_in[k] == GMT_ASCII_US) txt_in[k] = ' ';	/* Replace spaces and tabs masked above */
		args = new_args;
		n_args = new_n_args;
		gmt_M_str_free (txt_in);
	}
	else {
		args = (char **)in;	/* Gave an argv[] argument */
		n_args = n_args_in;
	}
	if (args == NULL && n_args) return_null (API, GMT_ARGV_LIST_NULL);	/* Conflict between # of args and args being NULL */

	for (arg = 0; arg < n_args; arg++) {	/* Loop over all command arguments */

		if (!args[arg]) continue;	/* Skip any NULL arguments quietly */

		/* Note: The UNIX command line will never see redirections like >, >>, and < pass as arguments, so when we check for these
		 * below it is because command-strings passed from external APIs may contain things like '-Fap -O -K >> plot.ps' */
	
		if (args[arg][0] == '=' && args[arg][1] && !gmt_access (API->GMT, &args[arg][1], F_OK)) {	/* Gave a file list which we must expand into options */
			char **flist = NULL;
			uint64_t n_files, f;
			n_files = gmtlib_read_list (API->GMT, &args[arg][1], &flist);
			if ((new_opt = GMT_Make_Option (API, '=', &args[arg][1])) == NULL)	/* Make option with the listing name flagged as option -= */
				return_null (API, error);	/* Create the new option structure given the args, or return the error */
			head = GMT_Append_Option (API, new_opt, head);		/* Hook new option to the end of the list (or initiate list if head == NULL) */
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "GMT_Create_Options: Must expand list file %s\n", args[arg]);
			for (f = 0; f < n_files; f++) {	/* Now expand all the listed files into options */
				GMT_Report (API, GMT_MSG_DEBUG, "GMT_Create_Options: Adding input file: %s\n", flist[f]);
				if ((new_opt = GMT_Make_Option (API, GMT_OPT_INFILE, flist[f])) == NULL)
					return_null (API, error);	/* Create the new option structure given the args, or return the error */
				new_opt = fix_gdal_files (new_opt);
				head = GMT_Append_Option (API, new_opt, head);		/* Hook new option to the end of the list (or initiate list if head == NULL) */
			}
			gmtlib_free_list (API->GMT, flist, n_files);
			continue;
		}
		else if (args[arg][0] == '<' && !args[arg][1] && (arg+1) < n_args && args[arg+1][0] != '-')	/* string command with "< file" for input */
			first_char = 0, option = GMT_OPT_INFILE, arg++;
		else if (args[arg][0] == '>' && !args[arg][1] && (arg+1) < n_args && args[arg+1][0] != '-')	/* string command with "> file" for output */
			first_char = 0, option = GMT_OPT_OUTFILE, arg++;
		else if (args[arg][0] == '>' && args[arg][1] == '>' && args[arg][2] == '\0' && (arg+1) < n_args && args[arg+1][0] != '-')	/* string command with ">> file" for appended output */
			first_char = 0, option = GMT_OPT_OUTFILE, append = 1, arg++;
		else if (args[arg][0] == '+' && !args[arg][1] && n_args == 1)	/* extended synopsis + */
			first_char = 1, option = GMT_OPT_USAGE, G->common.synopsis.extended = true;
		else if (args[arg][0] == '-' && args[arg][1] == '+' && !args[arg][2] && n_args == 1)	/* extended synopsis + */
			first_char = 1, option = GMT_OPT_USAGE, G->common.synopsis.extended = true;
		else if (args[arg][0] != '-')	/* Probably a file (could also be a gmt/grdmath OPERATOR or number, to be handled later by GMT_Make_Option) */
			first_char = 0, option = GMT_OPT_INFILE;
		else if (!args[arg][1])	/* Found the special synopsis option "-" */
			first_char = 1, option = GMT_OPT_SYNOPSIS;
		else if (!strcmp(args[arg], "--help"))	/* Translate '--help' to '-?' */
			first_char = 6, option = GMT_OPT_USAGE;
		else if ((isdigit ((int)args[arg][1]) || args[arg][1] == '.') && !gmt_not_numeric (API->GMT, args[arg])) /* A negative number, most likely; convert to "file" for now */
			first_char = 0, option = GMT_OPT_INFILE;
		else {	/* Most likely found a regular option flag (e.g., -D45.0/3) */
			first_char = 2, option = args[arg][1];
			if (option == ')') option = GMT_OPT_OUTFILE, append = 1;	/* -)file is same as ">> file", i.e., append to existing file */
		}

		if (append) {	/* Must prepend ">" to the filename so we select append mode when opening the file later */
			/* First check that this file exists */
			if (access (&args[arg][first_char], F_OK)) {	/* File does not exist; revert to writing to new output file */
				GMT_Report (API, GMT_MSG_DEBUG, "GMT_Create_Options: File %s does not exist so append (>>) changed to output (>)\n", &args[arg][first_char]);
				this_arg = &args[arg][first_char];
			}
			else {
				snprintf (buffer, GMT_LEN1024, ">%s", &args[arg][first_char]);
				this_arg = buffer;
			}
			append = 0;
		}
		else
			this_arg = &args[arg][first_char];

		/* To store -JEPSG:n or even -Jn in history we must prefix it with a '+' sign */
		if (option == 'J' && (isdigit(this_arg[0]) || !strncmp(this_arg, "EPSG:", 5) || !strncmp(this_arg, "epsg:", 5))) {
			char *t = malloc (strlen(this_arg)+2);
			sprintf (t, "+%s", this_arg);
			if ((new_opt = GMT_Make_Option (API, option, t)) == NULL) {
				free (t);
				return_null (API, error);	/* Create the new option structure given the args, or return the error */
			}
			free (t);
		}
		else
			if ((new_opt = GMT_Make_Option (API, option, this_arg)) == NULL)
				return_null (API, error);	/* Create the new option structure given the args, or return the error */

		if (option == GMT_OPT_INFILE)	/* A special check on infiles to see if they refer to GDAL implicitly */
			new_opt = fix_gdal_files (new_opt);

		head = GMT_Append_Option (API, new_opt, head);		/* Hook new option to the end of the list (or initiate list if head == NULL) */
	}
	if (n_args_in == 0) {	/* Free up temporary arg list */
		for (arg = 0; arg < n_args; arg++) gmt_M_str_free (new_args[arg]);
		gmt_M_free (G, new_args);
	}

	/* Check if we are in presence of a oneliner. If yes that implies MODERN mode. */
	if (n_args_in == 0) {
		int k;
		bool code = false;
		char figure[GMT_LEN512] = {""}, *c = NULL;
		struct GMT_OPTION *opt = NULL;
		for (opt = head; opt; opt = opt->next) {	/* Loop over all options */
			if (opt->option == 'O' || opt->option == 'K') break;	/* Cannot be a one-liner if -O or -K are involved */
			snprintf (figure, GMT_LEN512, "%c%s", opt->option, opt->arg);	/* So -png, which would be parse into -p and ng, are reunited to png */
			if ((c = strchr (figure, ','))) c[0] = 0;	/* Chop of more than one format for the id test */
			if ((k = gmt_get_graphics_id (API->GMT, figure)) == GMT_NOTSET) continue;	/* Not a quicky one-liner option */
			if (opt->next && opt->next->option == GMT_OPT_INFILE) 	/* Found a -ext[,ext,ext,...] <prefix> pair */
				code = true;
		}
		if (code)
			API->GMT->current.setting.run_mode = GMT_MODERN;
	}

	return (head);		/* We return the linked list */
}

/*! . */
int GMT_Destroy_Options (void *V_API, struct GMT_OPTION **head) {
	/* Delete the entire linked list of options, such as those created by GMT_Create_Options */

	struct GMT_OPTION *current = NULL, *to_delete = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	API = parse_get_api_ptr (V_API);	/* Cast to GMTAPI_CTRL pointer */

	current = *head;
	if (current && current->option < 0) {
		GMT_Report(API, GMT_MSG_NORMAL, "WARNING in GMT_Destroy_Options(): GMT_OPTION struct has junk. Returning before crash\n");
		*head = NULL;
		return (GMT_OK);	/* Should we return an error state instead? */
	}

	while (current) {	/* Start at head and loop over the list and delete the options, one by one. */
		to_delete = current;		/* The one we want to delete */
		current = current->next;	/* But first grab the pointer to the next item */
		gmt_M_str_free (to_delete->arg);	/* The option had a text argument allocated by strdup, so free it first */
		gmt_M_free (API->GMT, to_delete);		/* Then free the structure which was allocated by gmt_M_memory */
	}
	*head = NULL;		/* Reset head to NULL value since it no longer points to any allocated memory */
	return (GMT_OK);	/* No error encountered */
}

/*! . */
struct GMT_OPTION * GMT_Duplicate_Options (void *V_API, struct GMT_OPTION *head) {
	/* Create an identical copy of the linked list of options pointed to by head */
	struct GMT_OPTION *opt = NULL, *new_opt = NULL, *new_head = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (head == NULL)  return_null (V_API, GMT_OPTION_LIST_NULL);	/* No list of options was given */
	API = parse_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */

	for (opt = head; opt; opt = opt->next) {	/* Loop over all options in the linked list */
		if ((new_opt = GMT_Make_Option (API, opt->option, opt->arg)) == NULL)
			return_null (API, API->error);	/* Create the new option structure given the args, or return the error */
		if ((new_head = GMT_Append_Option (API, new_opt, new_head)) == NULL)		/* Hook new option to the end of the list (or initiate list if new == NULL) */
			return_null (API, API->error);	/* Create the new option structure given the args, or return the error */
	}
	return (new_head);
}

/*! . */
int GMT_Free_Option (void *V_API, struct GMT_OPTION **opt) {
	struct GMTAPI_CTRL *API = NULL;
	/* Free the memory pointed to by *opt */
	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (*opt == NULL)  return_error (V_API, GMT_OPTION_LIST_NULL);	/* No list of options was given */
	API = parse_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */
	gmt_M_str_free ((*opt)->arg);	/* The option had a text argument allocated by strdup, so free it first */
	gmt_M_free (API->GMT, *opt);	/* Then free the structure which was allocated by gmt_M_memory */
	*opt = NULL;
	return (GMT_OK);	/* No error encountered */
}

/*! . */
char **GMT_Create_Args (void *V_API, int *argc, struct GMT_OPTION *head) {
	/* This function creates a character array with the command line options that
	 * correspond to the linked options provided.  It is the inverse of GMT_Create_Options.
	 * The number of array strings is returned via *argc.
	 */

	char **txt = NULL, buffer[GMT_BUFSIZ] = {""};
	int arg = 0;
	bool skip_infiles = false;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *G = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (head == NULL)  return_null (V_API, GMT_OPTION_LIST_NULL);	/* No list of options was given */
	API = parse_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */

	*argc = 0;	/* Start off with no arguments */
	for (opt = head; opt; opt = opt->next)	/* Loop over all options in the linked list */
		if (opt->option) (*argc)++;	/* Found non-empty option */
	if (*argc == 0) return NULL;		/* Found no options, so we are done */

	G = API->GMT;	/* GMT control structure */
	txt = gmt_M_memory (G, NULL, *argc, char *);	/* Allocate text arg array of given length */

	for (opt = head; opt; opt = opt->next) {	/* Loop over all options in the linked list */
		if (!opt->option) continue;			/* Skip all empty options */
		if (opt->option == GMT_OPT_SYNOPSIS)		/* Produce special - option for synopsis */
			sprintf (buffer, "-");
		else if (opt->option == '=' && opt->arg[0]) {		/* Start of long list of file args initially given via -=<list> */
			snprintf (buffer, GMT_BUFSIZ, "=%s", opt->arg);
			skip_infiles = true;
		}
		else if (opt->option == GMT_OPT_INFILE)	{	/* Option for input filename [or numbers] */
			if (skip_infiles) continue;
			snprintf (buffer, GMT_BUFSIZ, "%s", opt->arg);
		}
		else if (opt->arg && opt->arg[0])			/* Regular -?arg commandline option with argument for some ? */
			snprintf (buffer, GMT_BUFSIZ, "-%c%s", opt->option, opt->arg);
		else							/* Regular -? commandline argument without argument */
			snprintf (buffer, GMT_BUFSIZ, "-%c", opt->option);

		txt[arg] = gmt_M_memory (G, NULL, strlen (buffer)+1, char);	/* Get memory for this item */

		/* Copy over the buffer contents */
		strcpy (txt[arg], buffer);
		arg++;	/* One more option added */
	}
	/* OK, done processing all options */
	if (arg < *argc) {	/* Shrink the list to fit */
		*argc = arg;
		txt = gmt_M_memory (G, txt, *argc, char *);
	}

	return (txt);	/* Pass back the char** array to the calling module */
}

/*! . */
int GMT_Destroy_Args (void *V_API, int argc, char **args[]) {
	/* Delete all text arguments, perhaps those created by GMT_Create_Args
	 * Note that a pointer to the args[] array is expected so that we can
	 * set it to NULL afterwards. */

	struct GMTAPI_CTRL *API = NULL;
	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);		/* GMT_Create_Session has not been called */
	if (argc == 0 || !args) return_error (V_API, GMT_ARGV_LIST_NULL);	/* We were given no args to destroy, so there! */
	if (argc < 0) return_error (V_API, GMT_COUNTER_IS_NEGATIVE);		/* We were given a negative count! */
	API = parse_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */
	/* Just deallocate the space taken by the list of arguments */
	while (argc--) gmt_M_free (API->GMT, (*args)[argc]);
	gmt_M_free (API->GMT, *args);	/* Free the array itself */
	return (GMT_OK);		/* No error encountered */
}

/*! . */
char * GMT_Create_Cmd (void *V_API, struct GMT_OPTION *head) {
	/* This function creates a single character string with the command line options that
	 * correspond to the linked options provided.
	 */

	char *txt = NULL, *c = NULL, buffer[GMT_BUFSIZ] = {""};
	bool first = true, skip_infiles = false;
	size_t length = 0, inc, n_alloc = GMT_BUFSIZ;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *G = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (head == NULL) return_null (V_API, GMT_OPTION_LIST_NULL);	/* No list of options was given */
	API = parse_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */

	G = API->GMT;	/* GMT control structure */
	txt = gmt_M_memory (G, NULL, n_alloc, char);

	for (opt = head; opt; opt = opt->next) {	/* Loop over all options in the linked list */
		if (!opt->option) continue;			/* Skip all empty options */
		if (opt->option == GMT_OPT_SYNOPSIS)		/* Produce special - option for synopsis */
			sprintf (buffer, "-");
		else if (opt->option == '=' && opt->arg[0]) {		/* Special list file, add it but not the files that follow */
			snprintf (buffer, GMT_BUFSIZ, "=%s", opt->arg);
			skip_infiles = true;
		}
		else if (opt->option == GMT_OPT_INFILE)	{	/* Option for input filename [or numbers] */
			if (skip_infiles) continue;
			if (gmtlib_file_is_srtmlist (API, opt->arg))	/* Want to replace the srtm list with the original @earth_relief_xxx name instead */
				snprintf (buffer, GMT_BUFSIZ, "@earth_relief_0%cs", opt->arg[strlen(opt->arg)-8]);
			else if (gmt_M_file_is_remotedata (opt->arg) && (c = strstr (opt->arg, ".grd"))) {
				c[0] = '\0';
				snprintf (buffer, GMT_BUFSIZ, "%s", opt->arg);
				c[0] = '.';
			}
			else
				snprintf (buffer, GMT_BUFSIZ, "%s", opt->arg);
		}
		else if (opt->arg && opt->arg[0]) {			/* Regular -?arg commandline option with argument for some ? */
			if (strchr (opt->arg, ' '))	/* Has a space in the argument, e.g., a title or similar */
				snprintf (buffer, GMT_BUFSIZ, "-%c\"%s\"", opt->option, opt->arg);
			else
				snprintf (buffer, GMT_BUFSIZ, "-%c%s", opt->option, opt->arg);
		}
		else							/* Regular -? commandline argument without argument */
			snprintf (buffer, GMT_BUFSIZ, "-%c", opt->option);

		inc = strlen (buffer);
		if (!first) inc++;	/* Count the space between args */
		if ((length + inc) >= n_alloc) {	/* Will need more memory */
			n_alloc <<= 1;
			txt = gmt_M_memory (G, txt, n_alloc, char);
		}
		if (!first) strcat (txt, " ");	/* Add space between args */
		strcat (txt, buffer);
		length += inc;
		first = false;
	}
	length++;	/* Need space for trailing \0 */
	/* OK, done processing all options */
	if (length == 1)	/* Found no options, so delete the string we allocated */
		gmt_M_free (G, txt);
	else if (length < n_alloc)	/* Trim back on the list to fit what we want */
		txt = gmt_M_memory (G, txt, length, char);

	return (txt);		/* Pass back the results to the calling module */
}

/*! Delete string created by GMT_Create_Cmd, pass its address */
int GMT_Destroy_Cmd (void *V_API, char **cmd) {

	struct GMTAPI_CTRL *API = NULL;
	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (*cmd == NULL) return_error (V_API, GMT_ARG_IS_NULL);	/* No command was given */
	API = parse_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */
	gmt_M_free (API->GMT, *cmd);	/* Free the command string */
	return (GMT_OK);		/* No error encountered */
}

/*! Create an option structure given the option character and the optional argument arg */
struct GMT_OPTION *GMT_Make_Option (void *V_API, char option, const char *arg) {
	struct GMT_OPTION *new_opt = NULL;
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	API = parse_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */

	/* Here we have a program-specific option or a file name.  In either case we create a new option structure */

	new_opt = gmt_M_memory (API->GMT, NULL, 1, struct GMT_OPTION);	/* Allocate one option structure */

	new_opt->option = option;		/* Assign which option character was used */
	if (!arg)				/* If arg is a NULL pointer: */
		new_opt->arg = strdup ("");	/* Copy an empty string, such that new_opt->arg[0] = '\0', which avoids */
						/* segfaults later on since few functions check for NULL pointers  */
	else {					/* If arg is set to something (may be an empty string): */
		new_opt->arg = strdup (arg);	/* Allocate space for the argument and duplicate it in the option structure */
		gmt_chop (new_opt->arg);	/* Get rid of any trailing \n \r from cross-binary use in Cygwin/Windows */
	}

	return (new_opt);			/* Pass back the pointer to the allocated option structure */
}

/*! Search the list for the selected option and return the pointer to the item.  Only the first occurrence will be found. */
struct GMT_OPTION * GMT_Find_Option (void *V_API, char option, struct GMT_OPTION *head) {

	struct GMT_OPTION *current = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */

	for (current = head; current && current->option != option; current = current->next);	/* Linearly search for the specified option */
	return (current);	/* NULL if not found */
}

/*! Replaces the argument of this option with new arg. */
int GMT_Update_Option (void *V_API, struct GMT_OPTION *opt, const char *arg) {

	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (opt == NULL) return_error (V_API, GMT_OPTION_IS_NULL);	/* We passed NULL as the option */
	if (arg == NULL) return_error (V_API, GMT_ARG_IS_NULL);		/* We passed NULL as the argument */
	gmt_M_str_free (opt->arg);
	opt->arg = strdup (arg);

	return (GMT_OK);	/* No error encountered */
}

/*! Replaces a marker character (?) in the option arg with the replacement argument in arg, except when in quotes. */
int GMT_Expand_Option (void *V_API, struct GMT_OPTION *opt, const char *arg) {
	char buffer[BUFSIZ] = {""};
	size_t in = 0, out = 0;
	size_t s_length;
	bool quote = false;	/* We are outside any quoted text */

	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	 /* GMT_Create_Session has not been called */
	if (opt == NULL) return_error (V_API, GMT_OPTION_IS_NULL);	 /* We passed NULL as the option */
	if (arg == NULL) return_error (V_API, GMT_ARG_IS_NULL);		 /* We passed NULL as the argument */
	if (opt->arg == NULL) return_error (V_API, GMT_ARG_IS_NULL); /* We passed NULL as the option argument */
	s_length = strlen (arg);
	if ((s_length + strlen (opt->arg)) > BUFSIZ) return_error (V_API, GMT_DIM_TOO_LARGE);		/* Don't have room */

	while (opt->arg[in]) {
		if (opt->arg[in] == '\"') quote = !quote;
		if (opt->arg[in] == '?' && !quote) {	/* Found an unquoted questionmark */
			strcat (&buffer[out], arg);	/* Insert the given arg instead */
			out += s_length;		/* Adjust next output location */
		}
		else	/* Regular text, copy one-by-one */
			buffer[out++] = opt->arg[in];
		in++;
	}
	gmt_M_str_free (opt->arg);
	opt->arg = strdup (buffer);
	return (GMT_NOERROR);
}

/*! Append this entry to the end of the linked list */
struct GMT_OPTION *GMT_Append_Option (void *V_API, struct GMT_OPTION *new_opt, struct GMT_OPTION *head) {
	struct GMT_OPTION *current = NULL;

	if (V_API == NULL) return_null (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (!new_opt) return_null (V_API, GMT_OPTION_IS_NULL);		/* No option was passed */
	if (!new_opt->arg) return_null (V_API, GMT_ARG_IS_NULL);	/* Option argument must not be null pointer */

	if (head == NULL) return (new_opt);				/* No list yet, let new_opt become the start of the list */

	/* Here the list already existed with head != NULL */

	if (new_opt->option == GMT_OPT_OUTFILE) {	/* Only allow one output file on command line */
		/* Search for existing output option */
		for (current = head; current->next && current->option != GMT_OPT_OUTFILE; current = current->next);
		if (current->option == GMT_OPT_OUTFILE) return_null (V_API, GMT_ONLY_ONE_ALLOWED);	/* Cannot have > 1 output file */
		/* Here current is at end so no need to loop again */
	}
	else {	/* Not an output file name so just go to end of list */
		for (current = head; current->next; current = current->next);			/* Go to end of list */
	}
	/* Append new_opt to end the list */
	current->next = new_opt;
	new_opt->previous = current;
	new_opt->next = NULL;	/* Since at end of the list and may have had junk */

	return (head);		/* Return head of list */
}

/*! . */
int GMT_Delete_Option (void *V_API, struct GMT_OPTION *current, struct GMT_OPTION **head) {
	/* Remove the specified entry from the linked list.  It is assumed that current
	 * points to the correct option in the linked list. */
	struct GMTAPI_CTRL *API = NULL;

	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */
	if (!current) return_error (V_API, GMT_OPTION_IS_NULL);		/* No option was passed */
	API = parse_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */

	/* Remove the current option and bypass via the next/prev pointers in the linked list */
	if (current->next) current->next->previous = current->previous;
	if (current->previous)	/* Reset pointer from previous entry to current's next entry */
		current->previous->next = current->next;
	else	/* current == *head so there is no previous; must update head */
		*head = current->next;
	gmt_M_str_free (current->arg);	/* Option arguments were created by strdup, so we must use free */
	gmt_M_free (API->GMT, current);		/* Option structure was created by gmt_M_memory, hence gmt_M_free */

	return (GMT_OK);	/* No error encountered */
}

/*! . */
int GMT_Parse_Common (void *V_API, const char *given_options, struct GMT_OPTION *options) {
	/* GMT_Parse_Common parses the option list for a program and detects the GMT common options.
	 * These are processed in the order required by GMT regardless of order given.
	 * The settings will override values set previously by other commands.
	 * It ignores filenames and only return errors if GMT common options are misused.
	 * Note: GMT_CRITICAL_OPT_ORDER is defined in gmt_common.h
	 */

	struct GMT_OPTION *opt = NULL;
	char list[2] = {0, 0}, critical_opt_order[] = GMT_CRITICAL_OPT_ORDER;
	unsigned int i, n_errors;
	struct GMTAPI_CTRL *API = NULL;
	const unsigned int s_length = (unsigned int)strlen(critical_opt_order);

	if (V_API == NULL) return_error (V_API, GMT_NOT_A_SESSION);	/* GMT_Create_Session has not been called */

	/* Check if there are short-hand commands present (e.g., -J with no arguments); if so complete these to full options
	 * by consulting the current GMT history machinery.  If not possible then we have an error to report */

	API = parse_get_api_ptr (V_API);	/* Cast void pointer to a GMTAPI_CTRL pointer */
	if (parse_complete_options (API->GMT, options)) return_error (API, GMT_OPTION_HISTORY_ERROR);	/* Replace shorthand failed */

	if (API->GMT->common.B.mode == 0) API->GMT->common.B.mode = parse_check_b_options (API->GMT, options);	/* Determine the syntax of the -B option(s) */

	n_errors = parse_check_extended_R (API->GMT, options);	/* Possibly parse -I if required by -R */

	/* First parse the common options in the order they appear in GMT_CRITICAL_OPT_ORDER */
	for (i = 0; i < s_length; i++) {	/* These are the GMT options that must be parsed in this particular order, if present */
		if (strchr (given_options, critical_opt_order[i]) == NULL) continue;	/* Not a selected option */
		list[0] = critical_opt_order[i];	/* Just look for this particular option in the linked opt list */
		for (opt = options; opt; opt = opt->next) {
			if (opt->option != list[0]) continue;
			n_errors += gmt_parse_common_options (API->GMT, list, opt->option, opt->arg);
		}
	}

	/* Now any critical option given has been parsed.  Next we parse anything NOT a critical GMT option */
	for (i = 0; given_options[i]; i++) {	/* Loop over all options given */
		if (strchr (critical_opt_order, given_options[i])) continue;	/* Skip critical option */
		list[0] = given_options[i];	/* Just look for this particular option */
		for (opt = options; opt; opt = opt->next) {
			n_errors += gmt_parse_common_options (API->GMT, list, opt->option, opt->arg);
			if (opt->option != list[0]) continue;
		}
	}

	/* Update [read-only] pointer to the current option list */
	API->GMT->current.options = options;
	if (n_errors) return_error (API, GMT_PARSE_ERROR);	/* One or more options failed to parse */
	if (gmt_M_is_geographic (API->GMT, GMT_IN)) API->GMT->current.io.warn_geo_as_cartesion = false;	/* Don't need this warning */

	return (GMT_OK);
}
