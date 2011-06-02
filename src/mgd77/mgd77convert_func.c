/*--------------------------------------------------------------------
 *	$Id: mgd77convert_func.c,v 1.8 2011-06-02 20:18:33 guru Exp $
 *
 *    Copyright (c) 2005-2011 by P. Wessel
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
 
#include "gmt_mgd77.h"
#include "mgd77.h"

struct MGD77CONVERT_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct D {	/* -D */
		GMT_LONG active;
	} D;
	struct L {	/* -L */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG dest;
	} L;
	struct F {	/* -F */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG format;
	} F;
	struct T {	/* -T */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG format;
	} T;
};

void *New_mgd77convert_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77CONVERT_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct MGD77CONVERT_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	C->F.format = C->T.format = MGD77_NOT_SET;
	
	return ((void *)C);
}

void Free_mgd77convert_Ctrl (struct GMT_CTRL *GMT, struct MGD77CONVERT_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);	
}

GMT_LONG GMT_mgd77convert_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT,"mgd77convert %s - Convert MGD77 data to other file formats\n\n", MGD77_VERSION);
	GMT_message (GMT, "usage: mgd77convert <cruise(s)> -Fa|c|t -T[+]a|c|t [-D] [-L[e][w][+]] [-V]\n\n");
        
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);
             
	MGD77_Cruise_Explain (GMT);
	GMT_message (GMT, "\t[Files are read from data repositories and written to current directory]\n");
	GMT_message (GMT, "\t-F Convert from a file that is either (a) MGD77 ASCII, (c) MGD77+ netCDF, or (t) plain table\n");
	GMT_message (GMT, "\t   Use -FC to recover the original MGD77 setting from the MGD77+ file [Default applies E77 corrections]\n");
	GMT_message (GMT, "\t-T Convert to a file that is either (a) MGD77 ASCII, (c) MGD77+ netCDF, or (t) plain table\n");
	GMT_message (GMT, "\t   By default we will refuse to overwrite existing files.  Prepend + to override this policy.\n");
	GMT_message (GMT, "\tOPTIONS:\n\n");
	GMT_message (GMT, "\t-D Selects high-resolution, 4-byte storage for mag, diur, faa, eot, and msd with precision\n");
	GMT_message (GMT, "\t   of 10 fTesla, 1 nGal, 0.01 mm [Default is 2-byte with 0.1 nTesla, 0.1 mGal, m precision]\n");
	GMT_message (GMT, "\t-L Log level and destination setting for verification reporting.  Append a combination\n");
	GMT_message (GMT, "\t   of w for warnings, e for errors, and + to send log to stdout [Default is GMT->session.std[GMT_ERR]])\n");
	GMT_explain_options (GMT, "V");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_mgd77convert_parse (struct GMTAPI_CTRL *C, struct MGD77CONVERT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to mgd77convert and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, code_pos, i;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
			case '#':	/* Skip input files confused as numbers (e.g. 123456) */
				break;

			/* Processes program-specific parameters */

			case 'L':	/* Determine level of error/warning checking and log destination */
				Ctrl->L.active = TRUE;
				for (i = 0; opt->arg[i]; i++) {
					if (opt->arg[i] == 'e') Ctrl->L.mode |= 2;
					if (opt->arg[i] == 'w') Ctrl->L.mode |= 1;
					if (opt->arg[i] == '+') Ctrl->L.dest = 1;
				}
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				switch (opt->arg[0]) {									
					case 'a':		/* Standard ascii MGD77 file */
						Ctrl->F.format = MGD77_FORMAT_M77;
						break;
					case 'C':		/* Enhanced MGD77+ netCDF file */
						Ctrl->F.mode = TRUE;	/* Overlook revisions */
					case 'c':
						Ctrl->F.format = MGD77_FORMAT_CDF;
						break;
					case 't':		/* Plain ascii dat table */
						Ctrl->F.format = MGD77_FORMAT_TBL;
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "Option -F Bad format (%c)!\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				code_pos = 0;
				if (opt->arg[code_pos] == '+') Ctrl->T.mode = TRUE, code_pos++;	/* Force overwriting existing files */
				switch (opt->arg[0]) {									
					case 'a':		/* Standard ascii MGD77 file */
						Ctrl->T.format = MGD77_FORMAT_M77;
						break;
					case 'c':
						Ctrl->T.format = MGD77_FORMAT_CDF;
						break;
					case 't':		/* Plain ascii dat table */
						Ctrl->T.format = MGD77_FORMAT_TBL;
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "Option -T Bad format (%c)!\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
#ifdef GMT_COMPAT
			case '4':	/* Selected high-resolution 4-byte integer MGD77+ format for mag, diur, faa, eot [2-byte integer] */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -4 is deprecated; use -D instead.\n");
#endif
			case 'D':
				Ctrl->D.active = TRUE;
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->F.format == MGD77_NOT_SET, "Syntax error: Must specify format of input files\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.format == MGD77_NOT_SET, "Syntax error: Must specify format of output files\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_mgd77convert_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); GMT_exit (code);}

GMT_LONG GMT_mgd77convert (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG i, argno, n_cruises = 0, n_paths, error = FALSE;
	
	char file[GMT_BUFSIZ], **list = NULL, *fcode = "act";
	char *format_name[MGD77_N_FORMATS] = {"MGD77 ASCII", "MGD77+ netCDF", "ASCII table"};

	struct MGD77_CONTROL M;
	struct MGD77_DATASET *D = NULL;
	struct MGD77CONVERT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (options && options->option == '?') return (GMT_mgd77convert_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options && options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_mgd77convert_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_mgd77convert", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-V", "", options))) Return ((int)error);
	Ctrl = (struct MGD77CONVERT_CTRL *) New_mgd77convert_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_mgd77convert_parse (API, Ctrl, options))) Return ((int)error);
	
	/*---------------------------- This is the mgd77convert main code ----------------------------*/

	/* Initialize MGD77 output order and other parameters*/
	
	MGD77_Init (GMT, &M);			/* Initialize MGD77 Machinery */
	
	M.verbose_level = (int)Ctrl->L.mode;
	M.verbose_dest  = (int)Ctrl->L.dest;

	/* Check that the options selected are mutually consistent */
	
	n_paths = MGD77_Path_Expand (GMT, &M, options, &list);	/* Get list of requested IDs */

	if (n_paths == 0) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: No cruises given\n");
		Return (EXIT_FAILURE);
	}

	
	if (Ctrl->F.format == Ctrl->T.format) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: The two formats chosen are the same\n");
	
	if (Ctrl->T.format == MGD77_FORMAT_TBL && !(strcmp (GMT->current.setting.format_float_out, "%lg") & strcmp (GMT->current.setting.format_float_out, "%g"))) {
		strcpy (GMT->current.setting.format_float_out, "%.10g");	/* To avoid loosing precision upon rereading this file */
	}
	
	if (Ctrl->T.format == MGD77_FORMAT_CDF && Ctrl->D.active) MGD77_select_high_resolution (GMT);
	
	for (argno = 0; argno < n_paths; argno++) {		/* Process each ID */
	
		D = MGD77_Create_Dataset (GMT);	/* Get data structure w/header */
		MGD77_Reset (GMT, &M);		/* Reset to start fresh for next file */

		M.format = (int)Ctrl->F.format;	/* Set input file's format and read everything into memory */
		M.original = Ctrl->F.mode;
		if (Ctrl->F.mode) M.use_corrections[MGD77_M77_SET] = M.use_corrections[MGD77_CDF_SET] = FALSE;	/* Turn off E77 corrections */
		MGD77_Ignore_Format (GMT, MGD77_FORMAT_ANY);	/* Reset to all formats OK, then ... */
		for (i = 0; i < MGD77_N_FORMATS; i++) if (i != M.format) MGD77_Ignore_Format (GMT, i);		/* ...only allow the specified input format */
		if (MGD77_Open_File (GMT, list[argno], &M, MGD77_READ_MODE)) continue;
		if (MGD77_Read_Header_Record (GMT, list[argno], &M, &D->H)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading header sequence for cruise %s\n", list[argno]);
			Return (EXIT_FAILURE);
		}
		sprintf (file, "%s.%s", M.NGDC_id, MGD77_suffix[Ctrl->T.format]);
		if (Ctrl->F.format == Ctrl->T.format && !(M.path[0] == '/' || M.path[1] == ':')) {
			GMT_report (GMT, GMT_MSG_FATAL, "Input and Output file have same name! Output file will have extension \".new\" appended\n");
			strcat (file, ".new");	/* To avoid overwriting original file */
		}
		if (!access (file, R_OK)) {	/* File exists */
			if (Ctrl->T.mode) {	/* Must delete the file first */
				if (remove (file)) {	/* Oops, removal failed */
					GMT_report (GMT, GMT_MSG_FATAL, "Unable to remove existing file %s - skipping the conversion\n", file);
					MGD77_Close_File (GMT, &M);
					MGD77_Free (GMT, D);	/* Free memory allocated by MGD77_Read_File */
					continue;
				}
			}
			else {	/* Cowardly refuse to do this */
				GMT_report (GMT, GMT_MSG_FATAL, "\nOutput file already exists.  Use -T+%c to force overwriting\n", fcode[Ctrl->T.format]);
				MGD77_Close_File (GMT, &M);
				MGD77_Free (GMT, D);	/* Free memory allocated by MGD77_Read_File */
				continue;
			}
		}
		
		/* OK, now we can read the data set */
		
		if (MGD77_Read_Data (GMT, list[argno], &M, D)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading data set for cruise %s\n", list[argno]);
			Return (EXIT_FAILURE);
		}
		MGD77_Close_File (GMT, &M);
		
		MGD77_Verify_Prep (GMT, &M, D);	/* Get key meta-data derived form data records */

		MGD77_Verify_Header (GMT, &M, &(D->H), NULL);	/* Verify the header */
	
		if (Ctrl->F.format == MGD77_FORMAT_CDF && Ctrl->T.format != MGD77_FORMAT_CDF && (D->H.info[MGD77_CDF_SET].n_col || D->flags[0] || D->flags[1])) {
			GMT_report (GMT, GMT_MSG_FATAL, "\nWarning: Input file contains enhanced material that the output file format cannot represent\n");
		}

		/* OK, ready to write out converted file */
		
		M.format = (int)Ctrl->T.format;				/* Change the format to the desired output format and write new file in current directory */
		M.original = TRUE;					/* Always write to original attributes */
		for (i = 0; i < MGD77_N_FORMATS; i++) MGD77_format_allowed[i] = (M.format == i) ? TRUE : FALSE;	/* Only allow the specified output format */
		D->H.author = GMT_memory (GMT, NULL, strlen (M.user)+1, char);	/* Allocate space for author */
		strcpy (D->H.author, M.user);									/* Pass current user login id as author */
		if (D->H.history) GMT_free (GMT, D->H.history);	/* Make sure history is blank so it is reset by MGD77_Write_File */
		if (MGD77_Write_File (GMT, file, &M, D)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error writing new file for cruise %s\n", list[argno]);
			Return (EXIT_FAILURE);
		}
		GMT_report (GMT, GMT_MSG_NORMAL, "Converted cruise %s to %s format", list[argno], format_name[Ctrl->T.format]);
		if (D->H.errors[0]) GMT_report (GMT, GMT_MSG_NORMAL, " [%2.2d header problems (%d warnings + %d errors)]", D->H.errors[0], D->H.errors[1], D->H.errors[2]);
		if (D->errors) GMT_report (GMT, GMT_MSG_NORMAL, " [%d data errors]", D->errors);
		GMT_report (GMT, GMT_MSG_NORMAL, "\n");

		MGD77_Free (GMT, D);	/* Free memory allocated by MGD77_Read_File */
		n_cruises++;
	}
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Converted %ld MGD77 files\n", n_cruises);
	
	MGD77_Path_Free (GMT, (int)n_paths, list);
	MGD77_end (GMT, &M);

	Return (GMT_OK);
}
