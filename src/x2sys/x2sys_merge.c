/*-----------------------------------------------------------------
 *
 *      Copyright (c) 1999-2020 by J. Luis
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *      Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/* x2sys_merge will read two crossovers data base and output the contents
 * of the main one updated with the COEs in the second one. The second
 * file should only contain updated COEs relatively to the first one.
 * That is, it MUST NOT contain any new two tracks intersections. 
 *
 * Author:	Joaquim Luis
 * Date:	09-Jun-2009
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"x2sys_merge"
#define THIS_MODULE_MODERN_NAME	"x2sys_merge"
#define THIS_MODULE_LIB		"x2sys"
#define THIS_MODULE_PURPOSE	"Merge an updated COEs table (smaller) into the main table (bigger)"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->V"

struct X2SYS_MERGE_CTRL {
	struct A {	/* -A */
		bool active;
		char *file;
	} A;
	struct M {	/* -M */
		bool active;
		char *file;
	} M;
};

GMT_LOCAL void clear_mem (struct GMT_CTRL *GMT, char **pairs_base, char **pairs_merge, uint64_t *map_base_start, uint64_t *map_base_end,
                          uint64_t *map_merge_start, uint64_t *map_merge_end, uint64_t n_base, uint64_t n_merge);

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_MERGE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct X2SYS_MERGE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_MERGE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->A.file);
	gmt_M_str_free (C->M.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s -A<main_COEdbase> -M<new_COEdbase> [%s] [%s]\n\n", name, GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-A Give file with the main crossover error data base.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Give file with the new crossover error data base.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The new COEs will replace the old ones present in <main_COEdbase>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Result is printed to stdout.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "V,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct X2SYS_MERGE_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

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
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files > 0, "Syntax error: No command-line input files allowed\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->A.active || !Ctrl->A.file, "Syntax error: Missing Base COEs database file. -A is mandatory\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->M.active || !Ctrl->M.file, "Syntax error: Missing Updating COEs database file. -M is mandatory\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && !access (Ctrl->A.file, F_OK), "Syntax error: Unable to find crossover file %s\n", Ctrl->A.file);
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && !access (Ctrl->M.file, F_OK), "Syntax error: Unable to find crossover file %s\n", Ctrl->M.file);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}
			
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_x2sys_merge (void *V_API, int mode, void *args) {
	uint64_t  i, j, k, n_base, n_merge, merge_start, *map_merge_end = NULL;
	uint64_t *map_base_start = NULL, *map_base_end = NULL, *map_merge_start = NULL;
	int error;
	size_t n_alloc;
	char line[GMT_BUFSIZ] = {""}, **pairs_base = NULL, **pairs_merge = NULL;
	FILE *fp_base = NULL, *fp_merge = NULL;
	struct GMT_RECORD *Out = NULL;
	struct X2SYS_MERGE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the x2sys_merge main code ----------------------------*/

	if ((fp_base = fopen (Ctrl->A.file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to open crossover file %s\n", Ctrl->A.file);
		Return (GMT_ERROR_ON_FOPEN);
	}

	if ((fp_merge = fopen (Ctrl->M.file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to open crossover file %s\n", Ctrl->M.file);
		fclose (fp_base);
		Return (GMT_ERROR_ON_FOPEN);
	}

	n_alloc = GMT_CHUNK;
	map_base_start = gmt_M_memory (GMT, NULL, n_alloc, uint64_t);
	map_base_end =   gmt_M_memory (GMT, NULL, n_alloc, uint64_t);
	pairs_base =     gmt_M_memory (GMT, NULL, n_alloc, char *);

	map_merge_start = gmt_M_memory (GMT, NULL, n_alloc, uint64_t);
	map_merge_end =   gmt_M_memory (GMT, NULL, n_alloc, uint64_t);
	pairs_merge =     gmt_M_memory (GMT, NULL, n_alloc, char *);

	/* Read in the main COEs dbase and store the pair track names */
	n_base = 0;		k = 1;
	while (fgets (line, GMT_BUFSIZ, fp_base)) {	/*  */
		if (line[0] == '>') {
			map_base_start[n_base] = k;
			if (n_base) map_base_end[n_base-1] = k - 1;
			pairs_base[n_base] = gmt_M_memory (GMT, NULL, 24, char);
			strncpy(pairs_base[n_base], &line[2], 19);
			n_base++;
			if (n_base == n_alloc) {
				n_alloc <<= 1;
				map_base_start = gmt_M_memory (GMT, map_base_start, n_alloc, uint64_t);
				map_base_end =   gmt_M_memory (GMT, map_base_end, n_alloc, uint64_t);
				pairs_base =     gmt_M_memory (GMT, pairs_base, n_alloc, char *);
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
			pairs_merge[n_merge] = gmt_M_memory (GMT, NULL, 24, char);
			strncpy(pairs_merge[n_merge], &line[2], 19);
			n_merge++;
			if (n_merge == n_alloc) {
				n_alloc <<= 1;
				map_merge_start = gmt_M_memory (GMT, map_merge_start, n_alloc, uint64_t);
				map_merge_end   = gmt_M_memory (GMT, map_merge_end, n_alloc, uint64_t);
				pairs_merge     = gmt_M_memory (GMT, pairs_merge, n_alloc, char *);
			}
		}

		k++;
	}
	map_merge_end[n_merge - 1] = k - 1;	/* This one was not yet assigned */
	rewind (fp_merge);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		fclose (fp_merge);
		fclose (fp_base);
		clear_mem (GMT, pairs_base, pairs_merge, map_base_start, map_base_end, map_merge_start, map_merge_end, n_base, n_merge);
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {
		fclose (fp_merge);
		fclose (fp_base);
		clear_mem (GMT, pairs_base, pairs_merge, map_base_start, map_base_end, map_merge_start, map_merge_end, n_base, n_merge);
		Return (API->error);	/* Enables data output and sets access mode */
	}
	Out = gmt_new_record (GMT, NULL, line);	/* Only text output */
	gmt_set_tableheader (GMT, GMT_OUT, true);	/* Turn on -ho explicitly */

	/* Jump comment lines in both files and osition the file pointer into the first data line */
	k = i = 0;
	while (fgets (line, GMT_BUFSIZ, fp_merge) && line[0] == '#') k++;	/* Jump the comment lines in the to-merge file */
	rewind (fp_merge);
	while (i < k && fgets (line, GMT_BUFSIZ, fp_merge)) i++;

	k = i = 0;
	while (fgets (line, GMT_BUFSIZ, fp_base) && line[0] == '#') {
		GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, line);
		k++;
	}
	rewind (fp_base);
	while (i < k && fgets (line, GMT_BUFSIZ, fp_base)) i++;

	/* Do the merging. COEs present in file dbase2 replace their pairs in dbase1 */
	merge_start = 0;
	for (i = 0; i < n_base; i++) {
		for (j = merge_start; j < n_merge; j++) {
			if (!strcmp(pairs_base[i], pairs_merge[j])) {		 /* Update these COEs */
				for (k = map_merge_start[j]; k <= map_merge_end[j]; k++) {
					if (!fgets (line, GMT_BUFSIZ, fp_merge)) {
						GMT_Report (API, GMT_MSG_NORMAL, "Read error in merge file line\n");
						fclose (fp_merge);
						fclose (fp_base);
						clear_mem (GMT, pairs_base, pairs_merge, map_base_start, map_base_end, map_merge_start, map_merge_end, n_base, n_merge);
						Return (GMT_RUNTIME_ERROR);
					}
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
				for (k = map_base_start[i]; k <= map_base_end[i]; k++) {	/* Advance also in the base file */
					if (!fgets (line, GMT_BUFSIZ, fp_base)) {
						GMT_Report (API, GMT_MSG_NORMAL, "Read error in base file\n");
						fclose (fp_merge);
						fclose (fp_base);
						clear_mem (GMT, pairs_base, pairs_merge, map_base_start, map_base_end, map_merge_start, map_merge_end, n_base, n_merge);
						Return (GMT_RUNTIME_ERROR);
					}
				}

				merge_start = j + 1;
				break;		/* Since we found this to update, no need to continue seeking for another repetition */
			}
			else if (j == (n_merge - 1)) {	/* Not equal. So do not to update, just recopy */
				for (k = map_base_start[i]; k <= map_base_end[i]; k++) {
					if (!fgets (line, GMT_BUFSIZ, fp_base)) {
						GMT_Report (API, GMT_MSG_NORMAL, "Read error in base file\n");
						fclose (fp_merge);
						fclose (fp_base);
						clear_mem (GMT, pairs_base, pairs_merge, map_base_start, map_base_end, map_merge_start, map_merge_end, n_base, n_merge);
						Return (GMT_RUNTIME_ERROR);
					}
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
			}
		}
		if (merge_start == n_merge) {	/* Copy the rest of dbase1 file and stop */
			while (fgets (line, GMT_BUFSIZ, fp_base))
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			break;			/* Not very elegant way of stopping, but we are done */
		}
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		fclose (fp_base);	fclose (fp_merge);
		clear_mem (GMT, pairs_base, pairs_merge, map_base_start, map_base_end, map_merge_start, map_merge_end, n_base, n_merge);
		Return (API->error);
	}

	fclose (fp_base);
	fclose (fp_merge);

	gmt_M_free (GMT, Out);
	clear_mem (GMT, pairs_base, pairs_merge, map_base_start, map_base_end, map_merge_start, map_merge_end, n_base, n_merge);

	Return (GMT_NOERROR);
}

GMT_LOCAL void clear_mem (struct GMT_CTRL *GMT, char **pairs_base, char **pairs_merge, uint64_t *map_base_start, uint64_t *map_base_end,
                          uint64_t *map_merge_start, uint64_t *map_merge_end, uint64_t n_base, uint64_t n_merge) {
	uint64_t i;

	for (i = 0; i < n_base; i++)  gmt_M_free (GMT, pairs_base[i]);
	for (i = 0; i < n_merge; i++) gmt_M_free (GMT, pairs_merge[i]);
	gmt_M_free (GMT, pairs_base);
	gmt_M_free (GMT, pairs_merge);
	gmt_M_free (GMT, map_base_start);
	gmt_M_free (GMT, map_base_end);
	gmt_M_free (GMT, map_merge_start);
	gmt_M_free (GMT, map_merge_end);
}
