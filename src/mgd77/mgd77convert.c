/*--------------------------------------------------------------------
 *
 *    Copyright (c) 2005-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mgd77convert allows for format conversions between three file formats:
 * a) The standard MGD-77 ASCII punchcard file format from NGDC
 * c) An enhanced "MGD77+" format based on netCDF that allows extra columns
 * t) A plain ASCII table version of the MGD-77 punch cards
 *
 * Input files are sought from both current directory and the list of data
 * directories given in the mgd77_paths.txt file in $MGD77_HOME.  Output is
 * always written to the current directory.  No file will be overwritten
 * unless this is requested.
 *
 * Author:	Paul Wessel
 * Date:	10-MAR-2006
 * Version:	1.2 for GMT 5
 *
 */

#include "gmt_dev.h"
#include "mgd77.h"

#define THIS_MODULE_CLASSIC_NAME	"mgd77convert"
#define THIS_MODULE_MODERN_NAME	"mgd77convert"
#define THIS_MODULE_LIB		"mgd77"
#define THIS_MODULE_PURPOSE	"Convert MGD77 data to other formats"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-V"

EXTERN_MSC void MGD77_select_high_resolution (struct GMT_CTRL *GMT);

struct MGD77CONVERT_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct C {	/* -C */
		bool active;
	} C;
	struct D {	/* -D */
		bool active;
	} D;
	struct L {	/* -L */
		bool active;
		unsigned int mode;
		unsigned int dest;
	} L;
	struct F {	/* -F */
		bool active;
		unsigned int mode;
		int format;
	} F;
	struct T {	/* -T */
		bool active;
		unsigned int mode;
		int format;
	} T;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77CONVERT_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct MGD77CONVERT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->F.format = C->T.format = MGD77_NOT_SET;

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct MGD77CONVERT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <cruise(s)> -Fa|c|m|t -Ta|c|m|t[+f] [-C] [-D] [-L[e][w][+l]] [%s] [%s]\n\n", name, GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	MGD77_Cruise_Explain (API->GMT);
	GMT_Message (API, GMT_TIME_NONE, "\t   [Files are read from data repositories and written to current directory]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Convert from a file that is either (a) MGD77 ASCII, (c) MGD77+ netCDF, (m) MGD77T ASCII, or (t) plain table.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -FC to recover the original MGD77 setting from the MGD77+ file [Default applies E77 corrections].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Convert to a file that is either (a) MGD77 ASCII, (c) MGD77+ netCDF, (m) MGD77T ASCII, or (t) plain table.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   By default we will refuse to overwrite existing files.  Append +f to force an override this policy.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Convert from NGDC (*.h77, *.a77) to *.mgd77 format; no other options allowed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give one or more names of h77-files, a77-files, or just cruise prefixes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Select high-resolution, 4-byte storage for mag, diur, faa, eot, and msd with precision\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of 10 fTesla, 1 nGal, 0.01 mm [Default is 2-byte with 0.1 nTesla, 0.1 mGal, m precision].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set log level and destination setting for verification reporting.  Append a combination\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of w for warnings and e for errors, and append +l to send log to stdout [Default is stderr].\n");
	GMT_Option (API, "V,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct MGD77CONVERT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to mgd77convert and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, code_pos, i;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
			case '#':	/* Skip input files confused as numbers (e.g. 123456) */
				break;

			/* Processes program-specific parameters */

			case 'L':	/* Determine level of error/warning checking and log destination */
				Ctrl->L.active = true;
				for (i = 0; opt->arg[i]; i++) {
					if (opt->arg[i] == 'e') Ctrl->L.mode |= 2;
					if (opt->arg[i] == 'w') Ctrl->L.mode |= 1;
					if (opt->arg[i] == '+' && (opt->arg[i+1] == '\0' || opt->arg[i+1] == 'l')) Ctrl->L.dest = 1;
				}
				break;
			case 'F':
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'a':		/* Standard ASCII MGD77 file */
						Ctrl->F.format = MGD77_FORMAT_M77;
						break;
					case 'C':		/* Enhanced MGD77+ netCDF file */
						Ctrl->F.mode = true;	/* Overlook revisions */
					case 'c':	/* Falling through from 'C' to 'c' on purpose */
						Ctrl->F.format = MGD77_FORMAT_CDF;
						break;
					case 'm':		/* New ASCII MGD77T file */
						Ctrl->F.format = MGD77_FORMAT_M7T;
						break;
					case 't':		/* Plain ASCII dat table */
						Ctrl->F.format = MGD77_FORMAT_TBL;
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -F Bad format (%c)!\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				code_pos = 0;
				if (opt->arg[code_pos] == '+')
					Ctrl->T.mode = true, code_pos++;	/* Force overwriting existing files */
				else if ((c = strstr (opt->arg, "+f"))) {
					Ctrl->T.mode = true;	/* Force overwriting existing files */
					c[0] = '\0';	/* Chop off modifier */
				}
				switch (opt->arg[code_pos]) {
					case 'a':		/* Standard ASCII MGD77 file */
						Ctrl->T.format = MGD77_FORMAT_M77;
						break;
					case 'c':
						Ctrl->T.format = MGD77_FORMAT_CDF;
						break;
					case 'm':		/* New ASCII MGD77T file */
						Ctrl->T.format = MGD77_FORMAT_M7T;
						break;
					case 't':		/* Plain ASCII dat table */
						Ctrl->T.format = MGD77_FORMAT_TBL;
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -T Bad format (%c)!\n", opt->arg[code_pos]);
						n_errors++;
						break;
				}
				if (c) c[0] = '\0';	/* Restore modifier */
				break;
			case '4':	/* Selected high-resolution 4-byte integer MGD77+ format for mag, diur, faa, eot [2-byte integer] */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "-4 is deprecated; use -D instead next time.\n");
					Ctrl->D.active = true;
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'C':
				Ctrl->C.active = true;
				break;
			case 'D':
				Ctrl->D.active = true;
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->C.active) {
		n_errors += gmt_M_check_condition (GMT, Ctrl->D.active || Ctrl->F.active || Ctrl->L.active || Ctrl->T.active, "Option -C: No other options allowed\n");
	}
	else {
		n_errors += gmt_M_check_condition (GMT, Ctrl->F.format == MGD77_NOT_SET, "Option -F: Must specify format of input files\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->T.format == MGD77_NOT_SET, "Option -f: Must specify format of output files\n");
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mgd77convert (void *V_API, int mode, void *args) {
	int i, argno, n_cruises = 0, n_paths, error = 0;

	char file[PATH_MAX] = {""}, **list = NULL, *fcode = "cmat";
	char *format_name[MGD77_N_FORMATS] = {"MGD77+ netCDF", "MGD77T ASCII", "MGD77 ASCII", "ASCII table"};

	struct MGD77_CONTROL M;
	struct MGD77_DATASET *D = NULL;
	struct MGD77CONVERT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the mgd77convert main code ----------------------------*/

	if (Ctrl->C.active) {	/* Just build *.mgd77 from *.h77 and *.a77 */
		char a77_file[PATH_MAX] = {""}, h77_file[PATH_MAX] = {""}, mgd77_file[PATH_MAX] = {""}, prefix[PATH_MAX] = {""};
		int pos, c, n_files = 0;
		struct GMT_OPTION *opt = NULL;
		FILE *fpa77 = NULL, *fph77 = NULL, *fpout = NULL;

		for (opt = options; opt; opt = opt->next) {	/* Loop over arguments, skip options */

			if (opt->option != '<') continue;	/* We are only processing filenames here */
			if ((pos = (int)(strlen (opt->arg) - 4)) < 0) continue;	/* Odd item, skip */
			strncpy (prefix, opt->arg, PATH_MAX);	/* Make copy of name/file */
			if (!strncmp (&prefix[pos], ".a77", 4U) || !strncmp (&prefix[pos], ".h77", 4U)) prefix[pos] = 0;	/* Truncate any extension */
			snprintf (a77_file, PATH_MAX-4, "%s.a77", prefix);
			snprintf (h77_file, PATH_MAX-4, "%s.h77", prefix);
			if (access (a77_file, R_OK)) {
				GMT_Report (API, GMT_MSG_ERROR, "A77 file %s not found - skipping conversion\n", a77_file);
				continue;
			}
			if (access (h77_file, R_OK)) {
				GMT_Report (API, GMT_MSG_ERROR, "H77 file %s not found - skipping conversion\n", h77_file);
				continue;
			}
			snprintf (mgd77_file, PATH_MAX-6, "%s.mgd77", prefix);
			if ((fpout = fopen (mgd77_file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Cannot create MGD77 file %s - skipping conversion\n", mgd77_file);
				continue;
			}
			if ((fph77 = fopen (h77_file, "r")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Cannot read H77 file %s - skipping conversion\n", h77_file);
				fclose (fpout);
				continue;
			}
			if ((fpa77 = fopen (a77_file, "r")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Cannot read A77 file %s - skipping conversion\n", a77_file);
				fclose (fpout);
				fclose (fph77);
				continue;
			}
			GMT_Report (API, GMT_MSG_INFORMATION, "Assemble %s + %s --> %s\n", h77_file, a77_file, mgd77_file);
			while ((c = fgetc (fph77)) != EOF) fputc (c, fpout);
			fclose (fph77);
			while ((c = fgetc (fpa77)) != EOF) fputc (c, fpout);
			fclose (fpa77);
			fclose (fpout);
			++n_files;
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Assembled %d H77/A77 files to MGD77 format\n", n_files);
		Return (GMT_NOERROR);
	}
	/* Initialize MGD77 output order and other parameters*/

	MGD77_Init (GMT, &M);			/* Initialize MGD77 Machinery */

	M.verbose_level = Ctrl->L.mode;
	M.verbose_dest  = Ctrl->L.dest;

	/* Check that the options selected are mutually consistent */

	n_paths = MGD77_Path_Expand (GMT, &M, options, &list);	/* Get list of requested IDs */

	if (n_paths <= 0) {
		GMT_Report (API, GMT_MSG_ERROR, "No cruises given\n");
		Return (GMT_RUNTIME_ERROR);
	}


	if (Ctrl->F.format == Ctrl->T.format) GMT_Report (API, GMT_MSG_WARNING, "The two formats chosen are the same\n");

	if (Ctrl->T.format == MGD77_FORMAT_TBL && !(strcmp (GMT->current.setting.format_float_out, "%lg") & strcmp (GMT->current.setting.format_float_out, "%g"))) {
		strcpy (GMT->current.setting.format_float_out, "%.10g");	/* To avoid losing precision upon rereading this file */
	}

	if (Ctrl->T.format == MGD77_FORMAT_CDF && Ctrl->D.active) MGD77_select_high_resolution (GMT);

	for (argno = 0; argno < n_paths; argno++) {		/* Process each ID */

		MGD77_Reset (GMT, &M);		/* Reset to start fresh for next file */

		M.format = Ctrl->F.format;	/* Set input file's format and read everything into memory */
		M.original = Ctrl->F.mode;
		if (Ctrl->F.mode) M.use_corrections[MGD77_M77_SET] = M.use_corrections[MGD77_CDF_SET] = false;	/* Turn off E77 corrections */
		MGD77_Ignore_Format (GMT, MGD77_FORMAT_ANY);	/* Reset to all formats OK, then ... */
		for (i = 0; i < MGD77_N_FORMATS; i++) if (i != M.format) MGD77_Ignore_Format (GMT, i);		/* ...only allow the specified input format */
		if (MGD77_Open_File (GMT, list[argno], &M, MGD77_READ_MODE)) continue;
		D = MGD77_Create_Dataset (GMT);	/* Get data structure w/header */
		if (MGD77_Read_Header_Record (GMT, list[argno], &M, &D->H)) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while reading header sequence for cruise %s\n", list[argno]);
			Return (GMT_DATA_READ_ERROR);
		}
		sprintf (file, "%s.%s", M.NGDC_id, MGD77_suffix[Ctrl->T.format]);
		if (Ctrl->F.format == Ctrl->T.format && !(M.path[0] == '/' || M.path[1] == ':')) {
			GMT_Report (API, GMT_MSG_WARNING, "Input and Output file have same name! Output file will have extension \".new\" appended\n");
			strcat (file, ".new");	/* To avoid overwriting original file */
		}
		if (!access (file, R_OK)) {	/* File exists */
			if (Ctrl->T.mode) {	/* Must delete the file first */
				if (gmt_remove_file (GMT, file)) {	/* Oops, removal failed */
					GMT_Report (API, GMT_MSG_ERROR, "Unable to remove existing file %s - skipping the conversion\n", file);
					MGD77_Close_File (GMT, &M);
					MGD77_Free_Dataset (GMT, &D);	/* Free memory allocated by MGD77_Read_File */
					continue;
				}
			}
			else {	/* Cowardly refuse to do this */
				GMT_Report (API, GMT_MSG_ERROR, "\nOutput file already exists.  Use -T%c+f to force overwriting\n", fcode[Ctrl->T.format]);
				MGD77_Close_File (GMT, &M);
				MGD77_Free_Dataset (GMT, &D);	/* Free memory allocated by MGD77_Read_File */
				continue;
			}
		}

		/* OK, now we can read the data set */

		if (MGD77_Read_Data (GMT, list[argno], &M, D)) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while reading data set for cruise %s\n", list[argno]);
			Return (GMT_DATA_READ_ERROR);
		}
		MGD77_Close_File (GMT, &M);

		MGD77_Verify_Prep (GMT, &M, D);	/* Get key meta-data derived form data records */

		MGD77_Verify_Header (GMT, &M, &(D->H), NULL);	/* Verify the header */

		if (Ctrl->F.format == MGD77_FORMAT_CDF && Ctrl->T.format != MGD77_FORMAT_CDF && (D->H.info[MGD77_CDF_SET].n_col || D->flags[0] || D->flags[1])) {
			GMT_Report (API, GMT_MSG_WARNING, "\nInput file contains enhanced material that the output file format cannot represent\n");
		}

		/* OK, ready to write out converted file */

		M.format = Ctrl->T.format;				/* Change the format to the desired output format and write new file in current directory */
		M.original = true;					/* Always write to original attributes */
		for (i = 0; i < MGD77_N_FORMATS; i++) MGD77_format_allowed[i] = (M.format == i) ? true : false;	/* Only allow the specified output format */
		gmt_M_free (GMT, D->H.author);	/* Make sure author is blank so it is reset below */
		D->H.author = gmt_M_memory (GMT, NULL, strlen (M.user)+1, char);	/* Allocate space for author */
		strcpy (D->H.author, M.user);									/* Pass current user login id as author */
		gmt_M_free (GMT, D->H.history);	/* Make sure history is blank so it is reset by MGD77_Write_File */
		if (MGD77_Write_File (GMT, file, &M, D)) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while writing new file for cruise %s\n", list[argno]);
			Return (GMT_DATA_WRITE_ERROR);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Converted cruise %s to %s format\n", list[argno], format_name[Ctrl->T.format]);
		if (D->H.errors[0]) GMT_Report (API, GMT_MSG_WARNING, "%s [%02d header problems (%d warnings + %d errors)]", list[argno], D->H.errors[0], D->H.errors[1], D->H.errors[2]);
		if (D->errors) GMT_Report (API, GMT_MSG_WARNING, "%s [%d data errors]", list[argno], D->errors);

		MGD77_Free_Dataset (GMT, &D);	/* Free memory allocated by MGD77_Read_File */
		n_cruises++;
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Converted %d MGD77 files\n", n_cruises);

	MGD77_Path_Free (GMT, (uint64_t)n_paths, list);
	MGD77_end (GMT, &M);

	Return (GMT_NOERROR);
}
