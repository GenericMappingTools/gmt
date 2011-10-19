/*-----------------------------------------------------------------
 *	$Id$
 *
 *      Copyright (c) 1999-2011 by J. Luis
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/* x2sys_merge will read two crossovers data base and output the contents
 * of the main one updated with the COEs in the second one. The second
 * file should only contain updated COEs relatively to the first one.
 * That is, it MUST NOT contain any new two tracks intersections. 
 *
 * Author:	Joaquim Luis
 * Date:	09-Juin-2009
 *
 */

#include "gmt_x2sys.h"

struct X2SYS_MERGE_CTRL {
	struct A {	/* -A */
		GMT_LONG active;
		char *file;
	} A;
	struct M {	/* -M */
		GMT_LONG active;
		char *file;
	} M;
};

void *New_x2sys_merge_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_MERGE_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_MERGE_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return (C);
}

void Free_x2sys_merge_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_MERGE_CTRL *C) {	/* Deallocate control structure */
	if (C->A.file) free (C->A.file);
	if (C->M.file) free (C->M.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_x2sys_merge_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "x2sys_merge - Merge an updated COEs table (smaller) into the main table (bigger)\n\n");
	GMT_message (GMT, "usage: x2sys_merge -A<main_COEdbase> -M<new_COEdbase> [%s]\n", GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-A Give file with the main crossover error data base.\n");
	GMT_message (GMT, "\t-M Give file with the new crossover error data base.\n");
	GMT_message (GMT, "\t   The new COEs will replace the old ones present in <main_COEdbase>.\n");
	GMT_message (GMT, "\t   Result is printed to stdout.\n");
	GMT_explain_options (GMT, "V");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_x2sys_merge_parse (struct GMTAPI_CTRL *C, struct X2SYS_MERGE_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.file = strdup (opt->arg);
				break;
			case 'M':
				Ctrl->M.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files > 0, "Syntax error: No command-line input iles allowed\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->A.active || !Ctrl->A.file, "Syntax error: Missing Base COEs database file. -A is mandatory\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->M.active || !Ctrl->M.file, "Syntax error: Missing Updating COEs database file. -M is mandatory\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && !access (Ctrl->A.file, F_OK), "Syntax error: Unable to find crossover file %s\n", Ctrl->A.file);
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && !access (Ctrl->M.file, F_OK), "Syntax error: Unable to find crossover file %s\n", Ctrl->M.file);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}
			
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_x2sys_merge_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_x2sys_merge (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG  i, j, k, n_alloc, n_base, n_merge, merge_start, error;
	GMT_LONG *map_base_start = NULL, *map_base_end = NULL, *map_merge_start = NULL, *map_merge_end = NULL;
	char line[GMT_BUFSIZ], **pairs_base = NULL, **pairs_merge = NULL, *c_not_used = NULL;
	FILE *fp_base = NULL, *fp_merge = NULL;
	struct X2SYS_MERGE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_x2sys_merge_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_x2sys_merge_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_x2sys_merge", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-V", ">", options))) Return (error);
	Ctrl = New_x2sys_merge_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_merge_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the x2sys_merge main code ----------------------------*/

	if ((fp_base = fopen (Ctrl->A.file, "r")) == NULL) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: Unable to open crossover file %s\n", Ctrl->A.file);
		Return (EXIT_FAILURE);
	}

	if ((fp_merge = fopen (Ctrl->M.file, "r")) == NULL) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: Unable to open crossover file %s\n", Ctrl->M.file);
		Return (EXIT_FAILURE);
	}

	n_alloc = GMT_CHUNK;
	map_base_start = GMT_memory (GMT, NULL, n_alloc, GMT_LONG);
	map_base_end =   GMT_memory (GMT, NULL, n_alloc, GMT_LONG);
	pairs_base =     GMT_memory (GMT, NULL, n_alloc, char *);

	map_merge_start = GMT_memory (GMT, NULL, n_alloc, GMT_LONG);
	map_merge_end =   GMT_memory (GMT, NULL, n_alloc, GMT_LONG);
	pairs_merge =     GMT_memory (GMT, NULL, n_alloc, char *);

	/* Read in the main COEs dbase and store the pair track names */
	n_base = 0;		k = 1;
	while (fgets (line, GMT_BUFSIZ, fp_base)) {	/*  */
		if (line[0] == '>') {
			map_base_start[n_base] = k;
			if (n_base) map_base_end[n_base-1] = k - 1;
			pairs_base[n_base] = GMT_memory (GMT, NULL, 24, char);
			strncpy(pairs_base[n_base], &line[2], 19);
			n_base++;
			if (n_base == n_alloc) {
				n_alloc <<= 1;
				map_base_start = GMT_memory (GMT, map_base_start, n_alloc, GMT_LONG);
				map_base_end =   GMT_memory (GMT, map_base_end, n_alloc, GMT_LONG);
				pairs_base =     GMT_memory (GMT, pairs_base, n_alloc, char *);
			}
		}

		k++;
	}
	map_base_end[n_base - 1] = k - 1;	/* This one was not yet assigned */
	rewind (fp_base);

	/* Read in the updated COEs dbase and store the pair track names */
	n_alloc = GMT_CHUNK;
	n_merge = 0;		k = 1;
	while (fgets (line, GMT_BUFSIZ, fp_merge)) {	/*  */
		if (line[0] == '>') {
			map_merge_start[n_merge] = k;
			if (n_merge) map_merge_end[n_merge-1] = k - 1;
			pairs_merge[n_merge] = GMT_memory (GMT, NULL, 24, char);
			strncpy(pairs_merge[n_merge], &line[2], 19);
			n_merge++;
			if (n_merge == n_alloc) {
				n_alloc <<= 1;
				map_merge_start = GMT_memory (GMT, map_merge_start, n_alloc, GMT_LONG);
				map_merge_end   = GMT_memory (GMT, map_merge_end, n_alloc, GMT_LONG);
				pairs_merge     = GMT_memory (GMT, pairs_merge, n_alloc, char *);
			}
		}

		k++;
	}
	map_merge_end[n_merge - 1] = k - 1;	/* This one was not yet assigned */
	rewind (fp_merge);


	/* Jump comment lines in both files and osition the file poiter into the first data line */
	k = 0;
	while (fgets (line, GMT_BUFSIZ, fp_merge) && line[0] == '#') k++;	/* Jump the comment lines in the to-merge file */
	rewind (fp_merge);
	for (i = 0; i < k; i++) c_not_used = fgets (line, GMT_BUFSIZ, fp_merge);

	k = 0;
	while (fgets (line, GMT_BUFSIZ, fp_base) && line[0] == '#') {
		fprintf (stdout, "%s", line);
		k++;
	}
	rewind (fp_base);
	for (i = 0; i < k; i++) c_not_used = fgets (line, GMT_BUFSIZ, fp_base);

	/* Do the merging. COEs present in file dbase2 replace their pairs in dbase1 */
	merge_start = 0;
	for (i = 0; i < n_base; i++) {
		for (j = merge_start; j < n_merge; j++) {
			if (!strcmp(pairs_base[i], pairs_merge[j])) {		 /* Update these COEs */
				for (k = map_merge_start[j]; k <= map_merge_end[j]; k++) {
					c_not_used = fgets (line, GMT_BUFSIZ, fp_merge);
					fprintf (stdout, "%s", line);
				}
				for (k = map_base_start[i]; k <= map_base_end[i]; k++)	/* Advance also in the base file */
					c_not_used = fgets (line, GMT_BUFSIZ, fp_base);

				merge_start = j + 1;
				break;		/* Since we found this to update, no need to continue seeking for another repetition */
			}
			else if (j == (n_merge - 1)) {	/* Not equal. So do not to update, just recopy */
				for (k = map_base_start[i]; k <= map_base_end[i]; k++) {
					c_not_used = fgets (line, GMT_BUFSIZ, fp_base);
					fprintf (stdout, "%s", line);
				}
			}
		}
		if (merge_start == n_merge) {	/* Copy the rest of dbase1 file and stop */
			while (fgets (line, GMT_BUFSIZ, fp_base))
				fprintf (stdout, "%s", line);
			break;			/* Not very elegant way of stopping, but we are done */
		}
	}

	fclose (fp_base);
	fclose (fp_merge);

	for (i = 0; i < n_base; i++)  GMT_free (GMT, pairs_base[i]);
	for (i = 0; i < n_merge; i++) GMT_free (GMT, pairs_merge[i]);
	GMT_free (GMT, map_base_start);
	GMT_free (GMT, map_base_end);
	GMT_free (GMT, map_merge_start);
	GMT_free (GMT, map_merge_end);

	Return (GMT_OK);
}
