/*--------------------------------------------------------------------
 *	$Id$
 *
 *    Copyright (c) 2005-2011 by P. Wessel
 * mgd77manage is used to (1) remove data columns from mgd77+ files
 * or (2) add a new data column to mgd77+ files.  Data can be added
 * from data tables, created from reference field formulas, or by
 * sampling grids along track.  Data from tables may be numbers or
 * text strings.
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 *
 * Author:	Paul Wessel
 * Date:	18-OCT-2005
 * Version:	1.0
 *
 */
 
#include "gmt_mgd77.h"
#include "mgd77.h"
#include "mgd77_e77.h"	/* E77 Header Errata Codes */

#ifdef GMT_COMPAT
	EXTERN_MSC GMT_LONG backwards_SQ_parsing (struct GMT_CTRL *C, char option, char *item);
#endif

#define N_PAR		7
#define COL_SCALE	0
#define COL_OFFSET	1
#define IMG_SPACING	2
#define IMG_SCALE	3
#define IMG_MODE	4
#define IMG_LAT		5
#define COL_TYPE	6

#define ADD_IGRF	2
#define ADD_CARTER	3
#define ADD_GRAV	4
#define ADD_RMAG	5
#ifdef USE_CM4
#define ADD_CM4		6
#define ADD_RMAG4	7
#endif

#define N_E77_MODES	5
#define E77_HEADER_MODE	0
#define E77_TREND_MODE	1
#define E77_NAV_MODE	2
#define E77_VALUES_MODE	3
#define E77_SLOPES_MODE	4

#define MODE_a		1
#define MODE_c		2
#define MODE_d		3
#define MODE_e		4
#define MODE_g		5
#define MODE_i		6
#define MODE_n		7
#define MODE_t		8

#define set_bit(k) (1 << (k))

struct MGD77MANAGE_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct A {	/* -A */
		GMT_LONG active;
		GMT_LONG replace;
		GMT_LONG interpolate;
		GMT_LONG ignore_verify;
		GMT_LONG mode;
		GMT_LONG kind;
		GMT_LONG e77_skip_mode[N_E77_MODES];
		char *file;
		double parameters[N_PAR];
	} A;
	struct C {	/* -C */
		GMT_LONG active;
		GMT_LONG mode;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
		char *file;
	} D;
	struct E {	/* -E */
		GMT_LONG active;
		char value;
	} E;
	struct F {	/* -F */
		GMT_LONG active;
	} F;
	struct I {	/* -I */
		GMT_LONG active;
		char c_abbrev[GMT_TEXT_LEN64];
		char c_units[GMT_TEXT_LEN64];
		char c_name[MGD77_COL_NAME_LEN];
		char c_comment[MGD77_COL_COMMENT_LEN];
		char c_size;
	} I;
	struct N {	/* -N */
		GMT_LONG active;
		char code[2];
	} N;
};

void *New_mgd77manage_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77MANAGE_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct MGD77MANAGE_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	C->A.kind = GMT_IS_FLOAT;
	C->A.parameters[COL_SCALE] = 1.0;	/* Output column scaling */
	C->A.parameters[IMG_SCALE] = 1.0;	/* IMG data scaling */
	C->C.mode = 2;
	C->E.value = '9';
 	return (C);
}

void Free_mgd77manage_Ctrl (struct GMT_CTRL *GMT, struct MGD77MANAGE_CTRL *C) {	/* Deallocate control structure */
	if (C->A.file) free (C->A.file);	
	if (C->D.file) free (C->D.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_mgd77manage_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT,"mgd77manage %s [API] - Manage the content of MGD77+ files\n\n", GMT_VERSION);
	GMT_message (GMT,"usage: mgd77manage <cruise(s)> [-A[+]a|c|d|D|e|E|g|i|n|t|T<info>] [-Cf|g|e] [-D<name1>,<name2>,...]\n");
	GMT_message (GMT,"\t[-E<no_char>] [-F] [-I<abbrev>/<name>/<units>/<size>/<scale>/<offset>/\"comment\"]\n");
	GMT_message (GMT,"\t[-Ne|k|m|n[+|-]] [-V] [%s] [%s]\n\n", GMT_bi_OPT, GMT_n_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<cruises> is one or more MGD77+ legnames, e.g., 01010083.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n\n");
	GMT_message (GMT, "\t-A Append a new data column to the given files.  The code letters are:\n");
	GMT_message (GMT, "\t   +: Optional.  Will overwrite an existing column with same name with new data.\n");
	GMT_message (GMT, "\t   [Default will refuse if an existing column has the same abbreviation as the new data].\n");
	GMT_message (GMT, "\t   a: Give filename with a new column to add.  We expect a single-column file\n");
	GMT_message (GMT, "\t      with the same number of records as the MGD77 file.  Only one cruise can be set.\n");
	GMT_message (GMT, "\t      If filename is - we read from stdin.\n");
	GMT_message (GMT, "\t   c: Create a new column to be calculated from existing columns.  Add code:\n");
#ifdef USE_CM4
	GMT_message (GMT, "\t        4 = CM4 field, m = IGRF total field, c = Carter correction, g = IGF (\"normal gravity\")\n");
	GMT_message (GMT, "\t        R = recomputed magnetic anomaly rmag = mtfx - CM4 total field.\n");
	GMT_message (GMT, "\t        r = recomputed magnetic anomaly rmag = mtfx - IGRF total field.\n");
#else
	GMT_message (GMT, "\t        m = IGRF total field, c = Carter correction, g = IGF (\"normal gravity\").\n");
	GMT_message (GMT, "\t        r = recomputed magnetic anomaly rmag = mtfx - IGRF total field.\n");
#endif
	GMT_message (GMT, "\t        Append x for which mtfx field to use (1 or 2) [1].\n");
	GMT_message (GMT, "\t        For g, optionally append 1-4 to select the gravity formula to use:\n");
	GMT_message (GMT, "\t        1 = Heiskanen 1924, 2 = International 1930, 3 = IGF1967, 4 = IGF1980.\n");
	GMT_message (GMT, "\t        [Default uses formula specified in the MGD77 header, or 4 if not valid].\n");
	GMT_message (GMT, "\t   d: Give filename with (dist [see -N], data) for a new column.  We expect a two-column file\n");
	GMT_message (GMT, "\t      with distances (in km) in first column and data values in 2nd.  Only one cruise can be set.\n");
	GMT_message (GMT, "\t      If filename is - we read from stdin.  Only records with mathcing distance will have data assigned.\n");
	GMT_message (GMT, "\t   D: Same as d but we interpolate between the dist,data pairs to fill in all data records.\n");
	GMT_message (GMT, "\t   e: Ingest MGD77 error/correction information (e77) produced by mgd77sniffer.  We will look\n");
	GMT_message (GMT, "\t      for the <cruise>.e77 file in the current directory or in $MGD77_HOME/E77.\n");
	GMT_message (GMT, "\t      By default we will apply recommended header (h) and systematic fixes (f) and set all data bit flags.\n");
	GMT_message (GMT, "\t      Append a combination of these flags to change the default accordingly:\n");
	GMT_message (GMT, "\t        h = Ignore all header recommendations\n");
	GMT_message (GMT, "\t        f = Ignore all systematic fixes recommendations\n");
	GMT_message (GMT, "\t        n = Ignore data record bitflags pertaining to navigation (time, lon, lat).\n");
	GMT_message (GMT, "\t        v = Ignore data record bitflags pertaining to data values.\n");
	GMT_message (GMT, "\t        s = Ignore data record bitflags pertaining to data slopes (gradients).\n");
	GMT_message (GMT, "\t      Use -DE to ignore the verification status of the e77 file [Default requires verification to be Y].\n");
	GMT_message (GMT, "\t      Note: Previous E77 information will be removed prior to processing this E77 information.\n");
	GMT_message (GMT, "\t   g: Sample a GMT grid along track. (also see -n).\n");
	GMT_message (GMT, "\t      Append filename of the GMT grid.\n");
	GMT_message (GMT, "\t   i: Sample a Sandwell/Smith *.img Mercator grid along track (also see -n).\n");
	GMT_message (GMT, "\t      Give filename and append comma-separated scale, mode, and optionally max latitude [%g].\n", GMT_IMG_MAXLAT_80);
	GMT_message (GMT, "\t      The scale (0.1 or 1) is used to multiply after read; give mode as follows:\n");
	GMT_message (GMT, "\t        0 = img file w/ no constraint code, interpolate to get data at track.\n");
	GMT_message (GMT, "\t        1 = img file w/ constraints coded, interpolate to get data at track.\n");
	GMT_message (GMT, "\t        2 = img file w/ constraints coded, gets data only at constrained points, NaN elsewhere.\n");
	GMT_message (GMT, "\t        3 = img file w/ constraints coded, gets 1 at constraints, 0 elsewhere.\n");
	GMT_message (GMT, "\t        For mode 2|3 you may want to consider the -n+t<threshold> setting.\n");
	GMT_message (GMT, "\t   n: Give filename with (rec_no, data) for a new column.  We expect a two-column file\n");
	GMT_message (GMT, "\t      with record numbers (0 means 1st row) in first column and data values in 2nd.  Only one cruise can be set.\n");
	GMT_message (GMT, "\t      If filename is - we read from stdin.  Only records with matching record numbers will have data assigned.\n");
	GMT_message (GMT, "\t   t: Give filename with (abstime, data) for a new column.  We expect a two-column file\n");
	GMT_message (GMT, "\t      with dateTclock strings in first column and data values in 2nd.  Only one cruise can be set.\n");
	GMT_message (GMT, "\t      If filename is - we read from stdin.  Only records with matching times will have data assigned.\n");
	GMT_message (GMT, "\t   T: Same as t but we interpolate between the time, data pairs to fill in all data records.\n");
	GMT_message (GMT, "\t-C Append code for distance calculation procedure (when -Ad|D is set):\n");
	GMT_message (GMT, "\t     f Flat Earth.\n");
	GMT_message (GMT, "\t     g Great circle [Default].\n");
	GMT_message (GMT, "\t     e Ellipsoidal (geodesic) using current GMT ellipsoid.\n");
	GMT_message (GMT, "\t-D Delete the columns listed from all the cruise data files.\n");
	GMT_message (GMT, "\t   The columns are removed before any data are added.  It is not a substitute for -A+.\n");
	GMT_message (GMT, "\t   However, sometimes the shape of new data demands the old to be deleted first (you will be told).\n");
	GMT_message (GMT, "\t-E Give character used to fill empty/missing string columns [9]\n");
	GMT_message (GMT, "\t-F Force mode.  This allows you to even replace the standard MGD77 columns [only extended columns can be changed].\n");
	GMT_message (GMT, "\t-I In addition to the file information above, you must also specify column information:\n");
	GMT_message (GMT, "\t      abbrev:  Short, abbreviated word (lower case only), like satfaa (%d char max).\n", MGD77_COL_ABBREV_LEN);
	GMT_message (GMT, "\t      name:    Descriptive name, like \"Geosat/ERS-1 Free-air gravity\" (%d char max).\n", MGD77_COL_NAME_LEN);
	GMT_message (GMT, "\t      units:   Units for the column (e.g., mGal, gamma, km) (%d char max).\n", MGD77_COL_NAME_LEN);
	GMT_message (GMT, "\t      size:    Either t(ext), b(yte), s(hort), f(loat), i(nt), or d(ouble).\n");
	GMT_message (GMT, "\t      scale:   Multiply data by this scale before writing to mgd77+ file.\n");
	GMT_message (GMT, "\t      offset:  Add after scaling before writing to mgd77+ file.\n");
	GMT_message (GMT, "\t      comment: Any text (in double quotes) for information about column (%d char max).\n", MGD77_COL_COMMENT_LEN);
	GMT_message (GMT, "\t      -I is ignored by -Ae.\n");
	GMT_message (GMT, "\t   Note for text: Interpolation is not allowed, and \"not-a-string\" is created from -E.\n");
	GMT_message (GMT, "\t-N Append your choice for distance unit (if -Ad|D are set). Choose among:\n");
	GMT_message (GMT, "\t   (e) meter, (k) km, (M) miles, or (n) nautical miles [Default is -Nk].\n");
	GMT_message (GMT, "\t    See -C for selecting distance calculation procedure.\n");
	GMT_explain_options (GMT, "VC0n" GMT_OPT("Q"));
	
	return (EXIT_FAILURE);
}

/* Help functions to decode the -A and -I options */

GMT_LONG decode_A_options (GMT_LONG mode, char *line, char *file, double parameters[])
{
	GMT_LONG error = 0, n;
	
	if (mode == 1) {	/* For *.img files since we need to know data scale and grid mode */
		/* -A[+]i<filename>,<scale>/<mode>[/<lat>] */
		n = sscanf (line, "%[^,],%lf,%lf,%lf", file, &parameters[IMG_SCALE], &parameters[IMG_MODE], &parameters[IMG_LAT]);
		if (n < 3) error = 1;
	}
	else {	/* GMT grid or table: No data scale and mode to worry about */
		/* -A[+]a|c|d|D|e|g|n|t|T<filename> */
		strcpy (file, line);
	}
	
	return (error);
}

GMT_LONG decode_I_options (struct GMT_CTRL *C, char *line, char *abbrev, char *name, char *units, char *size, char *comment, double parameters[])
{	/* -I<abbrev>/<name>/<units>/<size>/<scale>/<offset>/\"comment\" */
	GMT_LONG i = 0, k, error, pos = 0;
	char p[GMT_BUFSIZ];
	
	while (i < 7 && GMT_strtok (C, line, "/", &pos, p)) {	/* Process the 7 items */
		switch (i) {
			case 0:
				strcpy (abbrev, p);
				/* Check abbrev for COARDS compliance as well as being lower case */
				for (k = error = 0; abbrev[k]; k++) {
					if (isupper ((int)abbrev[k])) error++;
					if (isalpha ((int)abbrev[k])) continue;
					if (isdigit ((int)abbrev[k]) && k > 0) continue;
					if (abbrev[k] == '_' && k > 0) continue;
					error++;
				}
				if (error) {
					fprintf (C->session.std[GMT_ERR], "%s: Abbreviation name should only contain lower case letters, digits, and underscores\n", C->init.progname);
					return (TRUE);
				}
				break;
			case 1:
				strcpy (name, p);
				break;
			case 2:
				strcpy (units, p);
				break;
			case 3:
				*size = p[0];
				break;
			case 4:
				parameters[COL_SCALE]  = atof (p);
				break;
			case 5:
				parameters[COL_OFFSET]  = atof (p);
				break;
			case 6:
				strcpy (comment, p);
				break;
		}
		i++;
	}
	
	switch (*size) {	/* Given size, set the NC type */
		case 'b':
			parameters[COL_TYPE] = NC_BYTE;
			break;
		case 'd':
			parameters[COL_TYPE] = NC_DOUBLE;
			break;
		case 'f':
			parameters[COL_TYPE] = NC_FLOAT;
			break;
		case 'i':
			parameters[COL_TYPE] = NC_INT;
			break;
		case 's':
			parameters[COL_TYPE] = NC_SHORT;
			break;
		case 't':
			parameters[COL_TYPE] = NC_CHAR;
			break;
		default:
			fprintf (C->session.std[GMT_ERR], "%s: Unknown data type flag %c\n", C->init.progname, *size);
			parameters[COL_TYPE] = MGD77_NOT_SET;
			break;
	}
	return ((GMT_LONG)(irint (parameters[COL_TYPE]) == MGD77_NOT_SET) || (i != 7));
}

GMT_LONG skip_if_missing (struct GMT_CTRL *C, char *name, char *file, struct MGD77_CONTROL *F, struct MGD77_DATASET **D)
{	/* Used when a needed column is not present and we must free memory and goto next file */
	GMT_LONG id;

	if ((id = MGD77_Get_Column (C, name, F)) == MGD77_NOT_SET) {
		fprintf (C->session.std[GMT_ERR], "%s: Cruise %s is missing column %s which is required for selected operation - skipping\n", C->init.progname, file, name);
		MGD77_Free_Dataset (C, D);	/* Free memory already allocated by MGD77_Read_File for this aborted effort */
	}
	return (id);
}

GMT_LONG got_default_answer (char *line, char *answer)
{
	GMT_LONG i, k, len;
	
	len = strlen (line) - 1;
	GMT_memset (answer, GMT_BUFSIZ, char);	/* No default answer */
	if (line[len] == ']') {	/* Got a default answer for this item */
		for (k = i = len; i && line[i] != '['; i--);
		strncpy (answer, &line[i+1], (size_t)(k - i - 1));
	}
	return (answer[0] != '\0');
}

GMT_LONG GMT_mgd77manage_parse (struct GMTAPI_CTRL *C, struct MGD77MANAGE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to mgd77manage and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, k, n_cruises = 0, got_table, got_grid, strings;
	nc_type c_nc_type;
	char file[GMT_BUFSIZ];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	GMT_memset (file, GMT_BUFSIZ, char);
	
	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
			case '#':	/* Skip input files confused as numbers (e.g. 123456) */
				n_cruises++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Adding a new column */
				Ctrl->A.active = TRUE;
				k = 0;
				if (opt->arg[k] == '+') {
					Ctrl->A.replace = TRUE;
					k++;
				}
				switch (opt->arg[k]) {
					case 'a':	/* Plain column data file of exact same # of records */
						Ctrl->A.mode = MODE_a;
						n_errors += decode_A_options (0, &opt->arg[k+1], file, Ctrl->A.parameters);
						break;
					case 'c':	/* Add reference field or correction term */
						Ctrl->A.mode = MODE_c;
						n_errors += decode_A_options (0, &opt->arg[k+1], file, Ctrl->A.parameters);
						break;
					case 'D':	/* dist,val data file - interpolate to get values at all records */
						Ctrl->A.interpolate = TRUE;
					case 'd':	/* dist,val data file - only update records with matching distances */
						Ctrl->A.mode = MODE_d;
						n_errors += decode_A_options (0, &opt->arg[k+1], file, Ctrl->A.parameters);
						break;
					case 'E':	/* Plain E77 error flag file from mgd77sniffer */
						Ctrl->A.ignore_verify = TRUE;	/* Process raw e77 files that have not been verified */
					case 'e':	/* Plain E77 error flag file from mgd77sniffer */
						Ctrl->A.mode = MODE_e;
						while (opt->arg[++k]) {
							switch (opt->arg[k]) {
								case 'h':	/* Ignore all header recommendations regardless of Y/N prefix */
									Ctrl->A.e77_skip_mode[E77_HEADER_MODE] = TRUE;
									break;
								case 'f':	/* Ignore all systematic trend recommendations regardless of Y/N prefix */
									Ctrl->A.e77_skip_mode[E77_TREND_MODE] = TRUE;
									break;
								case 'n':	/* Ignore all NAV flags */
									Ctrl->A.e77_skip_mode[E77_NAV_MODE] = TRUE;
									break;
								case 'v':	/* Ignore all VALUE flags */
									Ctrl->A.e77_skip_mode[E77_VALUES_MODE] = TRUE;
									break;
								case 's':	/* Ignore all SLOPE flags */
									Ctrl->A.e77_skip_mode[E77_SLOPES_MODE] = TRUE;
									break;
								default:
									GMT_report (GMT, GMT_MSG_FATAL, "Error: -Ae modifiers must be combination of hfnvs\n");
									n_errors++;
									break;
							}
						}
						break;
					case 'g':	/* Sample along track from this GMT grid file */
						Ctrl->A.mode = MODE_g;
						n_errors += decode_A_options (0, &opt->arg[k+1], file, Ctrl->A.parameters);
						break;
					case 'i':	/* Sample along track from this *.img grid file */
						Ctrl->A.mode = MODE_i;
						n_errors += decode_A_options (1, &opt->arg[k+1], file, Ctrl->A.parameters);
						break;
					case 'n':	/* recno,val data file - only update records with matching rec number */
						Ctrl->A.mode = MODE_n;
						n_errors += decode_A_options (0, &opt->arg[k+1], file, Ctrl->A.parameters);
						break;
					case 'T':	/* abstime,val data file - interpolate to get values at all records */
						Ctrl->A.interpolate = TRUE;
					case 't':	/* abstime,val data file - only update records with matching times */
						Ctrl->A.mode = MODE_t;
						Ctrl->A.kind = GMT_IS_ABSTIME;
						n_errors += decode_A_options (0, &opt->arg[k+1], file, Ctrl->A.parameters);
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "Error: -A modifier must be a|c|d|D|e|g|i|n|t|T\n");
						n_errors++;
						break;
				}
				if (strlen (file)) Ctrl->A.file = strdup (file);
				break;

			case 'C':	/* Distance calculation method */
				Ctrl->C.active = TRUE;
				if (opt->arg[0] == 'f') Ctrl->C.mode = 1;
				if (opt->arg[0] == 'g') Ctrl->C.mode = 2;
				if (opt->arg[0] == 'e') Ctrl->C.mode = 3;
				if (Ctrl->C.mode < 1 || Ctrl->C.mode > 3) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error -C: Flag must be f, g, or e\n");
					n_errors++;
				}
				break;
			case 'D':	/* Columns to delete */
				Ctrl->D.active = TRUE;
				Ctrl->D.file = strdup (opt->arg);
				
			case 'E':	/* character to generate no-string value */
				Ctrl->E.active = TRUE;
				Ctrl->E.value = opt->arg[0];
				break;

			case 'F':	/* Force mode */
				Ctrl->F.active = TRUE;
				break;

			case 'I':	/* Column attribute information */
				Ctrl->I.active = TRUE;
				n_errors += decode_I_options (GMT, opt->arg, Ctrl->I.c_abbrev, Ctrl->I.c_name, Ctrl->I.c_units, &Ctrl->I.c_size, Ctrl->I.c_comment, Ctrl->A.parameters);
				break;
				
			case 'N':	/* Set distance units */
				Ctrl->N.active = TRUE;
				Ctrl->N.code[0] = opt->arg[0];
#ifdef GMT_COMPAT
				if (Ctrl->N.code[0] == 'm') {
					GMT_report (GMT, GMT_MSG_COMPAT, "Warning -N: Unit m for miles is deprecated; use unit M instead\n");
					Ctrl->N.code[0] = 'M';
				}
#endif
				if (!strchr ("ekMn", (int)Ctrl->N.code[0])) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error -N: Unit must be e, k, M, or n\n");
					n_errors++;
				}
				break;
				
#ifdef GMT_COMPAT
			case 'Q':	/* Backwards compatible.  Grid interpolation options are now be set with -n */
				n_errors += backwards_SQ_parsing (GMT, 'Q', opt->arg);
				break;
#endif
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	got_table = (Ctrl->A.mode == MODE_a || Ctrl->A.mode == MODE_d || Ctrl->A.mode == MODE_n || Ctrl->A.mode == MODE_t);	/* Got a table to read */
	got_grid = (Ctrl->A.mode == MODE_g || Ctrl->A.mode == MODE_i);					/* Got a grid to read */
	c_nc_type = (nc_type) irint (Ctrl->A.parameters[COL_TYPE]);		/* NC data type */
	strings = (c_nc_type == NC_CHAR);				/* TRUE if our new column contains strings */
	
	n_errors += GMT_check_condition (GMT, (got_table + got_grid) > 1, "Syntax error: You must select one, and only one, of the -A options\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->A.interpolate + strings) > 1, "Syntax error: Cannot interpolate column if data are strings\n");
	n_errors += GMT_check_condition (GMT, got_table && Ctrl->A.mode == MODE_c, "Syntax error: Only one -A option can be specified\n");
	n_errors += GMT_check_condition (GMT, !got_grid && GMT->common.n.interpolant != BCR_BICUBIC, "Syntax error -n: Requires -Ag|i\n");
	if (!(Ctrl->D.active || Ctrl->A.mode == MODE_e)) {
		n_errors += GMT_check_condition (GMT, strlen (Ctrl->I.c_abbrev) > MGD77_COL_ABBREV_LEN, "Syntax error: Column abbreviation too long - %d characters is maximum!\n", MGD77_COL_ABBREV_LEN);
		n_errors += GMT_check_condition (GMT, strlen (Ctrl->I.c_name) > MGD77_COL_NAME_LEN, "Syntax error: Column name too long - %d characters is maximum!\n", MGD77_COL_NAME_LEN);
		n_errors += GMT_check_condition (GMT, strlen (Ctrl->I.c_comment) > MGD77_COL_COMMENT_LEN, "Syntax error: Column comment too long - %d characters is maximum!\n", MGD77_COL_COMMENT_LEN);
	}
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_mgd77manage_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_mgd77manage (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	int cdf_var_id, n_dims = 0, dims[2];		/* netCDF variables should be declared as int */
	size_t start[2] = {0, 0}, count[2] = {0, 0};	/* NetCDF offset variables are size_t */
	
	GMT_LONG i, j, k = 0, ii, jj, argno, n_paths = 0, column, result, c_kind = 0, interpolate = 0;
	GMT_LONG width, n_delete = 0, GF_version = MGD77_NOT_SET, n_expected_fields, n_fields = 0, ok_to_read = TRUE;
	GMT_LONG MTF_col = 1, set, n_bad, check, n_sampled = 0, n_changed = 0, n = 0, pos, n_alloc = GMT_CHUNK;
	GMT_LONG error = FALSE, transform, verified, strings = FALSE, got_grid, got_table, two_cols = FALSE, constant;
	
	time_t now;
	
	nc_type c_nc_type;
	
	char line[GMT_BUFSIZ], p[GMT_BUFSIZ], history[GMT_BUFSIZ], **list = NULL;
	char not_given[GMT_TEXT_LEN64], word[GMT_BUFSIZ], **tmp_string = NULL, *text = NULL;
	signed char LEN = 0, OLDLEN = 0;
	
	double i_dx = 0, i_dy = 0, x, y, match_value, single_val, dist_scale = 1.0;
	double *xtmp = NULL, *coldnt = NULL, *colvalue = NULL, *in = NULL, limits[2];

	FILE *fp = NULL, *fp_err = NULL;

	struct MGD77_CONTROL In;
	struct MGD77_DATASET *D = NULL;
	struct GMT_GRID *G = NULL;
	struct MGD77_CARTER Carter;
	struct MGD77MANAGE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (options && options->option == '?') return (GMT_mgd77manage_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options && options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_mgd77manage_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_mgd77manage", &GMT_cpy);		/* Save current state */
	if (GMT_Parse_Common (API, "-V", "", options)) Return (API->error);
	Ctrl = New_mgd77manage_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_mgd77manage_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the mgd77manage main code ----------------------------*/

	GMT_get_time_system (GMT, "unix", &(GMT->current.setting.time_system));						/* MGD77+ uses GMT's Unix time epoch */
	GMT_init_time_system_structure (GMT, &(GMT->current.setting.time_system));
	MGD77_Init (GMT, &In);			/* Initialize MGD77 Machinery */

	/* Default e77_skip_mode will apply header and fix corrections if prefix is Y and set all data bits */
	
	/* Check that the options selected are mutually consistent */
	
	got_table = (Ctrl->A.mode == MODE_a || Ctrl->A.mode == MODE_d || Ctrl->A.mode == MODE_n || Ctrl->A.mode == MODE_t);	/* Got a table to read */
	got_grid = (Ctrl->A.mode == MODE_g || Ctrl->A.mode == MODE_i);					/* Got a grid to read */
	c_nc_type = (nc_type) irint (Ctrl->A.parameters[COL_TYPE]);		/* NC data type */
	strings = (c_nc_type == NC_CHAR);				/* TRUE if our new column contains strings */
	
	n_paths = MGD77_Path_Expand (GMT, &In, options, &list);	/* Get list of requested IDs */

	if (n_paths == 0) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: No cruises given\n");
		Return (EXIT_FAILURE);
	}

	if (got_table && n_paths != 1) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: With -Aa|d|D|n|t|T you can only select one cruise at the time.\n");
		Return (EXIT_FAILURE);
	}
	MGD77_Set_Unit (GMT, Ctrl->N.code, &dist_scale, -1);	/* Gets scale which multiplies meters to chosen distance unit */

	memset (not_given, (int)Ctrl->E.value, (size_t)GMT_TEXT_LEN64);	/* Text representing "no text value" */
	not_given[GMT_TEXT_LEN64-1] = '\0';
	fp_err = (In.verbose_dest == 1) ? GMT->session.std[GMT_OUT] : GMT->session.std[GMT_ERR];
	
	if (Ctrl->A.mode == MODE_c) {	/* Calculate values to be stored */
		GMT_LONG version = MGD77_NOT_SET, mfield = 1;
		/* "file" is here either m, c, or g[1-4] */
		if (Ctrl->A.file[0] == 'm' && Ctrl->A.file[1] == '\0') {
			c_kind = ADD_IGRF;
		}
#ifdef USE_CM4
		else if (Ctrl->A.file[0] == '4' && Ctrl->A.file[1] == '\0') {
			c_kind = ADD_CM4;
		}
#endif
		else if (Ctrl->A.file[0] == 'c' && Ctrl->A.file[1] == '\0') {
			c_kind = ADD_CARTER;
			MGD77_carter_init (GMT, &Carter);	/* Initialize Carter machinery */
		}
		else if (Ctrl->A.file[0] == 'g' && (Ctrl->A.file[1] == '\0' || ((version = (Ctrl->A.file[1] - '0')) >= MGD77_IGF_HEISKANEN && version <= MGD77_IGF_1980)) ) {
			c_kind = ADD_GRAV;
			GF_version = version;
		}
		else if (Ctrl->A.file[0] == 'r' && (Ctrl->A.file[1] == '\0' || ((mfield = (Ctrl->A.file[1] - '0')) >= 1 && mfield <= 2)) ) {
			c_kind = ADD_RMAG;
			MTF_col = mfield;
		}
#ifdef USE_CM4
		else if (Ctrl->A.file[0] == 'R' && (Ctrl->A.file[1] == '\0' || ((mfield = (Ctrl->A.file[1] - '0')) >= 1 && mfield <= 2)) ) {
			c_kind = ADD_RMAG4;
			MTF_col = mfield;
		}
#endif
		else {
#ifdef USE_CM4
			GMT_report (GMT, GMT_MSG_FATAL, "Error: -Ac expects 4, m, c, or g[1-4]\n");
#else
			GMT_report (GMT, GMT_MSG_FATAL, "Error: -Ac expects m, c, or g[1-4]\n");
#endif
			Return (EXIT_FAILURE);
		}
	}
	else if (Ctrl->A.mode == MODE_e) {	/* Do E77 work by ignoring previous E77 settings */
		In.use_flags[MGD77_M77_SET] = In.use_flags[MGD77_CDF_SET] = FALSE;		/* Turn use of flag bits OFF */
		In.use_corrections[MGD77_M77_SET] = In.use_corrections[MGD77_CDF_SET] = FALSE;	/* Turn use of systematic corrections OFF */
	}
	else if (Ctrl->A.mode == MODE_g) {	/* Read regular GMT grid */

		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->A.file, NULL)) == NULL) {	/* Get data */
			Return (API->error);
		}
		interpolate = (GMT->common.n.threshold > 0.0);
	}
	else if (Ctrl->A.mode == MODE_i) {	/* Read Sandwell/Smith IMG file */
		if ((G = GMT_Create_Data (API, GMT_IS_GRID, NULL)) == NULL) Return (API->error);
		GMT_read_img (GMT, Ctrl->A.file, G, NULL, Ctrl->A.parameters[IMG_SCALE], (GMT_LONG)irint(Ctrl->A.parameters[IMG_MODE]), Ctrl->A.parameters[IMG_LAT], TRUE);
		interpolate = (GMT->common.n.threshold > 0.0);
	}
	else if (got_table) {	/* Got a one- or two-column table to read */
		GMT_LONG n_ave = 0;
		double last_dnt = -DBL_MAX, sum_z = 0.0;
		char *not_used = NULL;
		
		if (Ctrl->A.file[0] == '-') {   /* Just read from standard input */
			fp = GMT->session.std[GMT_IN];
#ifdef SET_IO_MODE
			GMT_setmode (GMT, GMT_IN);
#endif
		}
		else {
			if ((fp = GMT_fopen (GMT, Ctrl->A.file, GMT->current.io.r_mode)) == NULL) {
				GMT_report (GMT, GMT_MSG_FATAL, "Cannot open file %s\n", Ctrl->A.file);
				Return (EXIT_FAILURE);
			}
		}

		/* Skip any header records */
		if (GMT->current.io.io_header[GMT_IN]) for (i = 0; i < GMT->current.io.io_n_header_items; i++) not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, fp);

		two_cols = (Ctrl->A.mode == MODE_d || Ctrl->A.mode == MODE_n || Ctrl->A.mode == MODE_t);
		n = (two_cols) ? -1 : 0;
		n_alloc = GMT_CHUNK;
		n_expected_fields = (GMT->common.b.ncol[GMT_IN]) ? GMT->common.b.ncol[GMT_IN] : GMT_MAX_COLUMNS;
		colvalue = GMT_memory (GMT, NULL, n_alloc, double);
		if (two_cols) {	/* Got an abscissae column as well (dnt: d = dist, n = rec number, t = time) */
			coldnt = GMT_memory (GMT, NULL, n_alloc, double);
			GMT->current.io.col_type[GMT_IN][GMT_X] = Ctrl->A.kind;
		}
		if (strings && !two_cols) {	/* Must read strings directly from file since GMT->current.io.input would barf */
			ok_to_read = FALSE;
			tmp_string = GMT_memory (GMT, NULL, n_alloc, char *);
			while (GMT_fgets (GMT, word, GMT_BUFSIZ, fp)) {
				if (word[0] == '#') continue;
				width = strlen (word);
				tmp_string[n] = GMT_memory (GMT, NULL, width + 1, char);
				strcpy (tmp_string[n], word);
				if (width > LEN) LEN = (signed char)width;
				n++;
			}
		}
		else if (strings) {		/* Pretend to read one column and get the text string form the text record */
			tmp_string = GMT_memory (GMT, NULL, n_alloc, char *);
			n_expected_fields = 1;
		}
		
		if (ok_to_read) in = GMT->current.io.input (GMT, fp, &n_expected_fields, &n_fields);

		while (ok_to_read && !GMT_REC_IS_EOF (GMT)) {	/* Not yet EOF */

			while (GMT_REC_IS_SEG_HEADER (GMT) && !GMT_REC_IS_EOF(GMT)) {
				in = GMT->current.io.input (GMT, fp, &n_expected_fields, &n_fields);
			}
			if ((GMT->current.io.status & GMT_IO_EOF)) continue;	/* At EOF */

			if (GMT->current.io.status & GMT_IO_MISMATCH) {
				GMT_report (GMT, GMT_MSG_FATAL, "Mismatch between actual (%ld) and expected (%ld) fields near line %ld\n", n_fields, n_expected_fields, n);
				GMT_exit (EXIT_FAILURE);
			}

			if (strings) {	/* number in col1, string in col2 */
				coldnt[n]   = in[0];
				sscanf (GMT->current.io.current_record, "%*s %s", word);
				tmp_string[n] = GMT_memory (GMT, NULL, strlen(word) + 1, char);
				strcpy (tmp_string[n], word);
			}
			else if (two_cols) {
				if (in[0] > last_dnt) {	/* Start of new averaging scheme (if needed) */
					if (n_ave) {
						colvalue[n] = sum_z / n_ave;
						coldnt[n]   = last_dnt;
					}
					n_ave = 0;
					sum_z = 0.0;
					n++;
					last_dnt = in[0];
				}
				sum_z += in[1];	/* Add up possibly multiple values for same dnt */
				n_ave++;
			}
			else
				colvalue[n] = in[0];
			if (!two_cols) n++;
			if (n == n_alloc) {
				n_alloc <<= 1;
				if (strings)
					tmp_string = GMT_memory (GMT, tmp_string, n_alloc, char *);
				else
					colvalue = GMT_memory (GMT, colvalue, n_alloc, double);
				if (two_cols) coldnt = GMT_memory (GMT, coldnt, n_alloc, double);
			}

			in = GMT->current.io.input (GMT, fp, &n_expected_fields, &n_fields);
		}
		GMT_fclose (GMT, fp);
		if (two_cols && n_ave) { colvalue[n] = sum_z / n_ave; coldnt[n++] = last_dnt;}
		if (!strings) colvalue = GMT_memory (GMT, colvalue, n, double);
		if (two_cols) coldnt = GMT_memory (GMT, coldnt, n, double);
	}
	
	if (got_grid) {	/* Set inverse spacing */
		i_dx = 1.0 / G->header->inc[GMT_X];
		i_dy = 1.0 / G->header->inc[GMT_Y];
	}

	MGD77_Ignore_Format (GMT, MGD77_FORMAT_ANY);	/* Reset to all formats OK, then ... */
	MGD77_Ignore_Format (GMT, MGD77_FORMAT_M77);	/* disallow ASCII MGD77 files */
	MGD77_Ignore_Format (GMT, MGD77_FORMAT_TBL);	/* and ASCII tables */
	
	In.format = MGD77_FORMAT_CDF;	/* Only file type allowed as input */
	
	for (argno = 0; argno < n_paths; argno++) {		/* Process each ID */
	
		if (MGD77_Open_File (GMT, list[argno], &In, MGD77_READ_MODE)) continue;
				
		GMT_report (GMT, GMT_MSG_NORMAL, "Now processing cruise %s\n", list[argno]);
		
		D = MGD77_Create_Dataset (GMT);
		In.n_out_columns = 0;

		if (MGD77_Read_File (GMT, list[argno], &In, D)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading data set for cruise %s\n", list[argno]);
			GMT_exit (EXIT_FAILURE);
		}

		/* Start reading data from file */
	
		column = MGD77_Get_Column (GMT, Ctrl->I.c_abbrev, &In);
		set    = MGD77_Get_Set (GMT, Ctrl->I.c_abbrev);
		
		if (!Ctrl->A.mode == MODE_e && column != MGD77_NOT_SET) {	/* A column with same abbreviation is already present in the file */
			if (set == MGD77_M77_SET && !Ctrl->F.active) {
				GMT_report (GMT, GMT_MSG_FATAL, "Column %s is part of the standard MGD77 set and cannot be removed unless you use -F!\n", Ctrl->I.c_abbrev);
				GMT_exit (EXIT_FAILURE);
			}
			if (!Ctrl->A.replace) {
				GMT_report (GMT, GMT_MSG_FATAL, "A columned named %s is already present in %s.  use -A+ to overwrite [default is to skip]\n", Ctrl->I.c_abbrev, list[argno]);
				MGD77_Free_Dataset (GMT, &D);	/* Free memory already allocated by MGD77_Read_File for this aborted effort */
				continue;
			}
			n_dims = (D->H.info[In.order[column].set].col[In.order[column].item].constant) ? 0 : 1;
			if (D->H.info[In.order[column].set].col[In.order[column].item].text) n_dims++;
		}

		if (Ctrl->D.active) {	/* Must create a new file with everything except the fields to be deleted */
			GMT_LONG id, c;
			char oldfile[GMT_BUFSIZ];
			
			if (column != MGD77_NOT_SET) {	/* Get info about this existing column to see if it is compatible with new data */
				n_dims = (D->H.info[In.order[column].set].col[In.order[column].item].constant) ? 0 : 1;
				if (D->H.info[In.order[column].set].col[In.order[column].item].text) n_dims++;
			}
			
			pos = n_delete = 0;
			(void) time (&now);
			sprintf (history, "%s [%s] removed columns", ctime(&now), In.user);
			for (i = 0; history[i]; i++) if (history[i] == '\n') history[i] = ' ';	/* Remove the \n returned by ctime() */
			while ((GMT_strtok (GMT, Ctrl->D.file, ",", &pos, p))) {	/* For each named column */
				k = MGD77_Get_Column (GMT, p, &In);
				if (k == MGD77_NOT_SET) {
					GMT_report (GMT, GMT_MSG_FATAL, "No column named %s in %s - cannot delete it. \n", p, list[argno]);
					continue;
				}
				c = In.order[k].set;
				id = In.order[k].item;
				D->H.info[c].col[id].abbrev[0] = D->H.info[c].col[id].name[0] = D->H.info[c].col[id].units[0] = D->H.info[c].col[id].comment[0] = '\0';
				D->H.info[c].col[id].pos = D->H.info[c].col[id].var_id = MGD77_NOT_SET;
				D->H.info[c].bit_pattern = 0;
				D->H.info[c].col[id].present = FALSE;
				D->H.info[c].n_col--;
				D->H.n_fields--;
				D->n_fields--;
				In.n_out_columns--;
				for (i = k; i < In.n_out_columns; i++) {	/* Move remaining columns over */
					D->values[i] = D->values[i+1];
					strcpy (In.desired_column[i], In.desired_column[i+1]);
					In.order[i].set = In.order[i+1].set;
					In.order[i].item = In.order[i+1].item;
				}
				strcat (history, " ");
				strcat (history, p);
				n_delete++;
			}
			
			/* Rename the old file for now */
			
			sprintf (oldfile, "%s.old", In.path);
			if (rename (In.path, oldfile)) {
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to rename %s to %s\n", In.path, oldfile);
				GMT_exit (EXIT_FAILURE);
			}
			
			/* Update header history */

			k = strlen (history);
			for (i = 0; i < k; i++) if (history[i] == '\n') history[i] = ' ';	/* Remove the \n returned by ctime() */
			history[k++] = '\n';	history[k] = '\0';				/* Add LF at end of line */
			k += strlen (D->H.history);
			D->H.history = GMT_memory (GMT, D->H.history, k, char);
			strcat (D->H.history, history);		/* MGD77_Write_FILE_cdf will use this to create the history attribute, thus preserving earlier history */

			if (MGD77_Write_File (GMT, In.path, &In, D)) {	/* Create the new, slimmer file */
				GMT_report (GMT, GMT_MSG_FATAL, "Error writing slimmer version of %s\n", list[argno]);
				GMT_exit (EXIT_FAILURE);
			}

			/* Now we can safely remove the old file */
			
			if (remove (oldfile)) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error removing the old version of %s\n", list[argno]);
				GMT_exit (EXIT_FAILURE);
			}
			
			MGD77_Free_Dataset (GMT, &D);
			if (column == MGD77_NOT_SET) continue;	/* Nothing more to do for this file */
			
			/* Now reread header etc since things have changed in the file */
			
			In.n_out_columns = 0;
			D = MGD77_Create_Dataset (GMT);
			if (MGD77_Read_File (GMT, list[argno], &In, D)) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error reading data set for cruise %s\n", list[argno]);
				GMT_exit (EXIT_FAILURE);
			}
			n_changed++;
		}

		if (c_kind == ADD_IGRF) {	/* Append IGRF column */
			GMT_LONG ix, iy, it;
			double date, *xvar = NULL, *yvar = NULL, *tvar = NULL, IGRF[7];
			
			if ((ix = skip_if_missing (GMT, "lon",  list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((iy = skip_if_missing (GMT, "lat",  list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((it = skip_if_missing (GMT, "time", list[argno], &In, &D)) == MGD77_NOT_SET) continue;

			xvar = D->values[ix];
			yvar = D->values[iy];
			tvar = D->values[it];
			colvalue = GMT_memory (GMT, NULL, D->H.n_records, double);
			
			for (i = n_sampled = 0; i < D->H.n_records; i++) {
				date = MGD77_time_to_fyear (GMT, &In, tvar[i]);	/* Get date as decimal year */
				colvalue[i] = (MGD77_igrf10syn (GMT, 0, date, 1, 0.0, xvar[i], yvar[i], IGRF)) ? GMT->session.d_NaN : IGRF[MGD77_IGRF_F];
				n_sampled++;
			}
			GMT_report (GMT, GMT_MSG_NORMAL, "Estimated IGRF at %ld locations out of %ld for cruise %s\n", n_sampled, D->H.n_records, list[argno]);
		}
#ifdef USE_CM4	
		else if (c_kind == ADD_CM4) {	/* Append CM4 column */
			GMT_LONG ix, iy, it;
			double date, *xvar = NULL, *yvar = NULL, *tvar = NULL;
			struct MGD77_CM4 CM4;
			
			if ((ix = skip_if_missing (GMT, "lon",  list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((iy = skip_if_missing (GMT, "lat",  list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((it = skip_if_missing (GMT, "time", list[argno], &In, &D)) == MGD77_NOT_SET) continue;

			xvar = D->values[ix];
			yvar = D->values[iy];
			tvar = D->values[it];
			colvalue = GMT_memory (GMT, NULL, D->H.n_records, double);
			MGD77_CM4_init (&In, &CM4);
			
			for (i = n_sampled = 0; i < D->H.n_records; i++) {
				date = MGD77_time_to_fyear (GMT, &In, tvar[i]);	/* Get date as decimal year */
				colvalue[i] = MGD77_Calc_CM4 (GMT, date, xvar[i], yvar[i], FALSE, &CM4);
				n_sampled++;
			}
			MGD77_CM4_end (GMT, &CM4);
			GMT_report (GMT, GMT_MSG_NORMAL, "Estimated CM4 at %ld locations out of %ld for cruise %s\n", n_sampled, D->H.n_records, list[argno]);
		}
		else if (c_kind == ADD_RMAG4) {	/* Append recomputed residual mag column */
			GMT_LONG ix, iy, it, im;
			double date, *xvar = NULL, *yvar = NULL, *tvar = NULL, *mvar = NULL, IGRF[7];
			char field[5];
			
			if ((ix = skip_if_missing (GMT, "lon",  list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((iy = skip_if_missing (GMT, "lat",  list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((it = skip_if_missing (GMT, "time", list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			sprintf (field, "mtf%d", MTF_col);
			if ((im = skip_if_missing (GMT, field, list[argno], &In, &D)) == MGD77_NOT_SET) continue;

			xvar = D->values[ix];
			yvar = D->values[iy];
			tvar = D->values[it];
			mvar = D->values[im];
			colvalue = GMT_memory (GMT, NULL, D->H.n_records, double);
			
			for (i = n_sampled = 0; i < D->H.n_records; i++) {
				date = MGD77_time_to_fyear (GMT, &In, tvar[i]);	/* Get date as decimal year */
/* Change this--> */		check = MGD77_igrf10syn (GMT, 0, date, 1, 0.0, xvar[i], yvar[i], IGRF);
				colvalue[i] = (check) ? GMT->session.d_NaN : mvar[i] - IGRF[MGD77_IGRF_F];
				n_sampled++;
			}
			GMT_report (GMT, GMT_MSG_NORMAL, "Estimated recomputed magnetic anomaly at %ld locations out of %ld for cruise %s\n", n_sampled, D->H.n_records, list[argno]);
		}
#endif
		else if (c_kind == ADD_GRAV) {	/* Append IGF column */
			GMT_LONG ix, iy, use;
			double *xvar = NULL, *yvar = NULL;
			
			if ((ix = skip_if_missing (GMT, "lon", list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((iy = skip_if_missing (GMT, "lat", list[argno], &In, &D)) == MGD77_NOT_SET) continue;

			if (GF_version == MGD77_NOT_SET) {
				use = (In.original) ? MGD77_ORIG : MGD77_REVISED;
				GF_version = D->H.mgd77[use]->Gravity_Theoretical_Formula_Code - '0';
				if (GF_version < MGD77_IGF_HEISKANEN || GF_version > MGD77_IGF_1980) {
					GMT_report (GMT, GMT_MSG_FATAL, "Invalid Gravity Theoretical Formula Code (%c) - default to %d\n", D->H.mgd77[use]->Gravity_Theoretical_Formula_Code, MGD77_IGF_1980);
					GF_version = MGD77_IGF_1980;
				}
			}
			xvar = D->values[ix];
			yvar = D->values[iy];
			colvalue = GMT_memory (GMT, NULL, D->H.n_records, double);
			
			for (i = 0; i < D->H.n_records; i++) colvalue[i] = MGD77_Theoretical_Gravity (GMT, xvar[i], yvar[i], (int)GF_version);
			GMT_report (GMT, GMT_MSG_NORMAL, "Estimated IGRF at %ld locations out of %ld for cruise %s\n", D->H.n_records, D->H.n_records, list[argno]);
		}
		else if (c_kind == ADD_CARTER) {	/* Append Carter correction column */
			GMT_LONG ix, iy, it;
			double *xvar = NULL, *yvar = NULL, *tvar = NULL;
			
			if ((ix = skip_if_missing (GMT, "lon", list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((iy = skip_if_missing (GMT, "lat", list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((it = skip_if_missing (GMT, "twt", list[argno], &In, &D)) == MGD77_NOT_SET) continue;

			xvar = D->values[ix];
			yvar = D->values[iy];
			tvar = D->values[it];
			colvalue = GMT_memory (GMT, NULL, D->H.n_records, double);
			
			for (i = 0; i < D->H.n_records; i++) colvalue[i] = MGD77_carter_correction (GMT, xvar[i], yvar[i], 1000.0 * tvar[i], &Carter);
			GMT_report (GMT, GMT_MSG_NORMAL, "Estimated IGRF at %ld locations out of %ld for cruise %s\n", D->H.n_records, D->H.n_records, list[argno]);
		}
		else if (c_kind == ADD_RMAG) {	/* Append recomputed residual mag column */
			GMT_LONG ix, iy, it, im;
			double date, *xvar = NULL, *yvar = NULL, *tvar = NULL, *mvar = NULL, IGRF[7];
			char field[5];
			
			if ((ix = skip_if_missing (GMT, "lon",  list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((iy = skip_if_missing (GMT, "lat",  list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((it = skip_if_missing (GMT, "time", list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			sprintf (field, "mtf%ld", MTF_col);
			if ((im = skip_if_missing (GMT, field, list[argno], &In, &D)) == MGD77_NOT_SET) continue;

			xvar = D->values[ix];
			yvar = D->values[iy];
			tvar = D->values[it];
			mvar = D->values[im];
			colvalue = GMT_memory (GMT, NULL, D->H.n_records, double);
			
			for (i = n_sampled = 0; i < D->H.n_records; i++) {
				date = MGD77_time_to_fyear (GMT, &In, tvar[i]);	/* Get date as decimal year */
				check = MGD77_igrf10syn (GMT, 0, date, 1, 0.0, xvar[i], yvar[i], IGRF);
				colvalue[i] = (check) ? GMT->session.d_NaN : mvar[i] - IGRF[MGD77_IGRF_F];
				n_sampled++;
			}
			GMT_report (GMT, GMT_MSG_NORMAL, "Estimated recomputed magnetic anomaly at %ld locations out of %ld for cruise %s\n", n_sampled, D->H.n_records, list[argno]);
		}
		else if (got_grid) {	/* Sample grid along track (or Mercator-projected) track */
			GMT_LONG ix, iy;
			double *xvar = NULL, *yvar = NULL;
			
			if ((ix = skip_if_missing (GMT, "lon", list[argno], &In, &D)) == MGD77_NOT_SET) continue;
			if ((iy = skip_if_missing (GMT, "lat", list[argno], &In, &D)) == MGD77_NOT_SET) continue;

			xvar = D->values[ix];
			yvar = D->values[iy];
			colvalue = GMT_memory (GMT, NULL, D->H.n_records, double);
			
			for (i = n_sampled = 0; i < D->H.n_records; i++) {
				colvalue[i] = GMT->session.d_NaN;	/* In case we are outside grid */
	
				/* If point is outside grd area, shift it using periodicity or skip if not periodic. */

				if (Ctrl->A.mode == MODE_i)	/* Mercator IMG grid */
					GMT_geo_to_xy (GMT, xvar[i], yvar[i], &x, &y);
				else {		/* Regular geographic grd */
					x = xvar[i];
					y = yvar[i];
				}
				if (y < G->header->wesn[YLO] || y > G->header->wesn[YHI]) continue;

				while ((x < G->header->wesn[XLO]) && (G->header->nxp > 0)) x += (G->header->inc[GMT_X] * G->header->nxp);
				if (x < G->header->wesn[XLO]) continue;

				while ((x > G->header->wesn[XHI]) && (G->header->nxp > 0)) x -= (G->header->inc[GMT_X] * G->header->nxp);
				if (x > G->header->wesn[XHI]) continue;

				if (interpolate) {	/* IMG has been corrected, and GRD is good to go */
					colvalue[i] = GMT_get_bcr_z (GMT, G, x, y);
				}
				else {	/* Take IMG nearest node and do special stuff (values already set during read) */
					ii = GMT_grd_x_to_col (GMT, x, G->header);
					jj = GMT_grd_y_to_row (GMT, y, G->header);
					colvalue[i] = G->data[GMT_IJP(G->header,jj,ii)];
				}
				n_sampled++;
			}
			GMT_report (GMT, GMT_MSG_NORMAL, "Sampled grid at %ld locations out of %ld for cruise %s\n", n_sampled, D->H.n_records, list[argno]);
		}
		else if (Ctrl->A.mode == MODE_a) {	/* Just got a single column to paste in, assuming the row numbers match */
			if (n != D->H.n_records) {
				GMT_report (GMT, GMT_MSG_FATAL, "Extra column data records (%ld) do not match # of cruise records (%ld) for %s\n", n, D->H.n_records, list[argno]);
				GMT_exit (EXIT_FAILURE);
			}
			GMT_report (GMT, GMT_MSG_NORMAL, "Appended column data for all %ld records for cruise %s\n", D->H.n_records, list[argno]);
		}
		else if (Ctrl->A.mode == MODE_d || Ctrl->A.mode == MODE_n || Ctrl->A.mode == MODE_t) {	/* Got either (time,data) or (dist,data) */
			GMT_LONG ix, iy, it;
			double *x = NULL, *y = NULL, *d = NULL;
			colvalue = GMT_memory (GMT, colvalue, D->H.n_records, double);
			if (Ctrl->A.mode == MODE_d) {	/* Must create distances in user's units */
				if ((ix = skip_if_missing (GMT, "lon", list[argno], &In, &D)) == MGD77_NOT_SET) continue;
				if ((iy = skip_if_missing (GMT, "lat", list[argno], &In, &D)) == MGD77_NOT_SET) continue;
				x = D->values[ix];
				y = D->values[iy];
				if ((d = GMT_dist_array (GMT, x, y, D->H.n_records, dist_scale, Ctrl->C.mode)) == NULL) GMT_err_fail (GMT, GMT_MAP_BAD_DIST_FLAG, "");
				x = d;
			}
			else if (Ctrl->A.mode == MODE_t) {	/* Time */
				if ((it = skip_if_missing (GMT, "time", list[argno], &In, &D)) == MGD77_NOT_SET) continue;
				x = D->values[it];
			}
			if (Ctrl->A.interpolate) {	/* Using given table to interpolate the values at all mgd77 records */
				y = GMT_memory (GMT, NULL, D->H.n_records, double);
				result = GMT_intpol (GMT, coldnt, colvalue, n, D->H.n_records, x, y, GMT->current.setting.interpolant);
				if (result != 0) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error from GMT_intpol near row %ld!\n", result+1);
					GMT_exit (EXIT_FAILURE);
				}
				memcpy (colvalue, y, (size_t)(D->H.n_records * sizeof (double)));
				GMT_free (GMT, y);
			}
			else if (strings && n < D->H.n_records) {	/* Only update the exact matching records */
				text = GMT_memory (GMT, NULL, D->H.n_records * LEN, char);
				for (i = j = n_sampled = 0; i < D->H.n_records && j < n; i++) {
					match_value = (Ctrl->A.mode == MODE_n) ? i+1 : x[i];
					strncpy (&text[i*LEN], not_given, (size_t)LEN);	/* In case we have no data at this time */
					while (coldnt[j] < match_value && j < n) j++;
					if (coldnt[j] == match_value) {
						strncpy (&text[i*LEN], tmp_string[j], (size_t)LEN);
						n_sampled++;
					}
				}
				GMT_free (GMT, tmp_string);
				GMT_report (GMT, GMT_MSG_NORMAL, "Appended column data for %ld locations out of %ld for cruise %s\n", n_sampled, D->H.n_records, list[argno]);
			}
			else if (strings) {	/* One to one match */
				text = GMT_memory (GMT, NULL, D->H.n_records * LEN, char);
				for (i = 0; i < n; i++) strncpy (&text[i*LEN], tmp_string[i], (size_t)LEN);
				GMT_free (GMT, tmp_string);
				GMT_report (GMT, GMT_MSG_NORMAL, "Appended column data for %ld locations out of %ld for cruise %s\n", n_sampled, D->H.n_records, list[argno]);
			}
			else {	/* Only update the exact matching records */
				y = GMT_memory (GMT, NULL, D->H.n_records, double);
				for (i = j = n_sampled = 0; i < D->H.n_records && j < n; i++) {
					match_value = (Ctrl->A.mode == MODE_n) ? i+1 : x[i];
					y[i] = GMT->session.d_NaN;	/* In case we have no data at this time */
					while (coldnt[j] < match_value && j < n) j++;
					if (coldnt[j] == match_value) {	/* Found our guy */
						y[i] = colvalue[j];
						n_sampled++;
					}
				}
				memcpy (colvalue, y, (size_t)(D->H.n_records * sizeof (double)));
				GMT_report (GMT, GMT_MSG_NORMAL, "Appended column data for %ld locations out of %ld for cruise %s\n", n_sampled, D->H.n_records, list[argno]);
				GMT_free (GMT, y);
			}
			if (Ctrl->A.mode == MODE_d) GMT_free (GMT, d);
		}
		else if (Ctrl->A.mode == MODE_e)
		{
			/* Read any header scale/offset factors to be set.
			 * Decode error flags to give bit flag, store in colvalue
			 */
			FILE *fp_e = NULL;
			int cdf_var_id, cdf_adjust;
			char ID[16], date[16], field[GMT_TEXT_LEN64], efile[GMT_BUFSIZ], E77[256], timestamp[GMT_TEXT_LEN64], answer[GMT_BUFSIZ], code[GMT_BUFSIZ], kind, YorN;
			GMT_LONG n_recs, rec, number, type, it, id, key, n_E77_flags, from, to, day, month, year, item;
			GMT_LONG n_E77_headers, n_E77_scales, n_E77_offsets, n_E77_recalcs, n_unprocessed, e_error = 0;
			unsigned int *flags = NULL, pattern;
			size_t length;
			GMT_LONG has_time, old_flags;
			struct MGD77_HEADER_PARAMS *P = NULL;
			double rec_time, del_t, value, *tvar = NULL;
			
			if (D->H.E77 && strlen(D->H.E77) > 0 && !Ctrl->A.replace) {
				GMT_report (GMT, GMT_MSG_FATAL, "E77 corrections are already present in %s.  use -A+e to overwrite with new corrections\n", list[argno]);
				MGD77_Free_Dataset (GMT, &D);	/* Free memory allocated by MGD77_Read_File for this aborted effort */
				continue;
			}
			
			sprintf (efile, "%s.e77", list[argno]);
			if ((fp_e = GMT_fopen (GMT, efile, "r")) == NULL) {	/* Not in current directory, try MGD77_HOME/E77 */
				sprintf (efile, "%s/E77/%s.e77", In.MGD77_HOME, list[argno]);
				if ((fp_e = GMT_fopen (GMT, efile, "r")) == NULL) {	/* Not here either */
					GMT_report (GMT, GMT_MSG_FATAL, "Error: The file %s.e77 could not be found in current directory or in MGD77_HOME/E77 - skipped\n", list[argno]);
					MGD77_Free_Dataset (GMT, &D);	/* Free memory allocated by MGD77_Read_File for this aborted effort */
					continue;
				}
			
			}
			/* We will first do many checks to make sure this E77 file goes with the specified cruise and that
			 * all the verification steps has been taken to make this E77 a correction file
			 */
			 
			P = D->H.mgd77[MGD77_ORIG];	/* Because E77 is absolute and not incremental we start from original settings */
			if (!GMT_fgets (GMT, line, GMT_BUFSIZ, fp_e)) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: Could not read record #1 from %s.e77 - aborting\n", list[argno]);
				e_error++;
			}
			sscanf (&line[1], "%*s %s %*s %*s %*s %*s %*s %s %*s %ld", ID, date, &n_recs);
			if (strcmp (In.NGDC_id, ID)) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: E77 Conflict %s : ID = %s versus %s - aborting\n", efile, ID, In.NGDC_id);
				e_error++;
			}
			/* Make sure the File creation dates from the data file and the E77 match */
			day = atoi (&date[6]);
			date[6] = 0;
			month = atoi (&date[4]);
			date[4] = 0;
			year = atoi (date);
			
			if (!(year == atoi (P->File_Creation_Year) && month == atoi (P->File_Creation_Month) && day == atoi (P->File_Creation_Day))) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: E77 Conflict %s: File Creation Date: %s versus %s%s%s - aborting\n", efile, date,
					P->File_Creation_Year, P->File_Creation_Month, P->File_Creation_Day);
				e_error++;
			}
			if (n_recs != D->H.n_records) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: E77 Conflict %s: n_recs = %ld versus %ld = aborting\n", efile, n_recs, D->H.n_records);
				e_error++;
			}
			verified = FALSE;
			while (GMT_fgets (GMT, line, GMT_BUFSIZ, fp_e) && strncmp (line, "# Errata: Header", (size_t)14)) {	/* Read until we get to Header record section */
				if (line[0] == '#') continue;	/* Skip comments */
				GMT_chop (GMT, line);		/* Rid the world of CR/LF */
				if (!strncmp (line, "Y Errata table verification status", (size_t)34)) verified = TRUE;
			}
			if (!verified && !Ctrl->A.ignore_verify) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: E77 file %s not yet verified.  E77 not applied\n", efile);
				e_error++;
			}
			
			if (e_error) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: The file %s has too many errors.  E77 not applied\n", efile);
				MGD77_Free_Dataset (GMT, &D);	/* Free memory allocated by MGD77_Read_File for this aborted effort */
				continue;
			}
			
			/* OK, we got this far so the meta data for this E77 file seems OK */
			
			/* Quickly scan through file to make sure there are no unprocessed recommendations or bad records before making changes */
			
			e_error = n_unprocessed = 0;
			while (GMT_fgets (GMT, line, GMT_BUFSIZ, fp_e)) {
				if (line[0] == '#' || line[0] == '\n') continue;	/* Comments or blank lines are OK */
				if (line[1] == '-') {		/* Header record */
					if (!(line[0] == 'Y' || line[0] == 'N') && !Ctrl->A.ignore_verify) {		/* Unprocessed recommendation? */
						GMT_message (GMT, "%s: UNDECIDED: %s\n", list[argno], line);
						if (line[0] == '?') n_unprocessed++;
						e_error++;
					}
					sscanf (line, "%c-%c-%[^-]-%[^-]-%ld", &YorN, &kind, ID, field, &item);
				}
				else				/* Data record */
					sscanf (line, "%c %s %s %ld %s", &YorN, ID, timestamp, &rec, code);
				if (strcmp (In.NGDC_id, ID)) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error: E77 Conflict %s : ID = %s versus %s in header records!\n", efile, ID, In.NGDC_id);
					e_error++;
				}
			}
			
			if (e_error) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: The file %s has too many errors.  E77 not applied\n", efile);
				GMT_fclose (GMT, fp_e);
				MGD77_Free_Dataset (GMT, &D);	/* Free memory allocated by MGD77_Read_File for this aborted effort */
				continue;
			}
			if (n_unprocessed) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: The file %s has unprocessed E77 recommendations.  E77 not applied\n", efile);
				GMT_fclose (GMT, fp_e);
				MGD77_Free_Dataset (GMT, &D);	/* Free memory allocated by MGD77_Read_File for this aborted effort */
				continue;
			}
			
			/* OK, here we believe the E77 file contains the correct information for this cruise. Rewind and start from top */
			
			GMT_rewind (fp_e);
			while (GMT_fgets (GMT, line, GMT_BUFSIZ, fp_e) && strncmp (line, "# Errata: Header", (size_t)14));	/* Read until we get to Header record section */
			
			flags = GMT_memory (GMT, NULL, D->H.n_records, unsigned int);
			n_E77_flags = n_E77_headers = n_E77_scales = n_E77_offsets = n_E77_recalcs = 0;

			MGD77_nc_status (GMT, nc_open (In.path, NC_WRITE, &In.nc_id));	/* Open the file */
			MGD77_nc_status (GMT, nc_redef (In.nc_id));				/* Enter define mode */
			old_flags = MGD77_Remove_E77 (GMT, &In);				/* Remove any previously revised header parameters */
			while (GMT_fgets (GMT, line, GMT_BUFSIZ, fp_e) && strncmp (line, "# Errata: Data", (size_t)14)) {	/* Read until we get to data record section */
				if (line[0] == '#' || line[0] == '\n') continue;	/* Skip comments */
				GMT_chop (GMT, line);					/* Rid the world of CR/LF */
				/* Example of expected line 
				   Y-E-06050010-H15-01: Invalid Gravity Departure Base Station Value: (0000000) [1000009]
				*/
				sscanf (line, "%c-%c-%[^-]-%[^-]-%ld", &YorN, &kind, ID, field, &item);
				if (strcmp (In.NGDC_id, ID)) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error: E77 Conflict %s : ID = %s versus %s in header records - skipped\n", efile, ID, In.NGDC_id);
					e_error++;
					continue;
				}
				if (field[0] == 'H') {
					type = E77_HEADER_MODE;
					number = (GMT_LONG)atoi (&field[1]);
				}
				else {
					type = 1;
					number = (GMT_LONG)item;
				}
				if (Ctrl->A.e77_skip_mode[type]) continue;
				if (!Ctrl->A.e77_skip_mode[type] && YorN == 'N') continue;
				if (kind == 'W') {	/* Output the warning (if Y) and goto next line*/
					if (GMT_is_verbose (GMT, GMT_MSG_NORMAL) && (YorN == 'Y' || (Ctrl->A.ignore_verify && YorN == '?'))) GMT_message (GMT, "%s: Warning: %s\n", list[argno], line);
					continue;
				}
				if (!got_default_answer (line, answer)) continue;
				
				/* Here we must do something */
				
				if (type == E77_HEADER_MODE) {	/* Header meta data fixes */
				
					key = MGD77_Param_Key (GMT, number, (int)item);	/* Returns -ve if sequence not found or item not found, >=0 otherwise */
					
					switch (key) {
						case MGD77_BAD_HEADER_RECNO:
							GMT_message (GMT, "Warning: Sequence number %ld is outside range - skipped\n", number);
							break;
						case MGD77_BAD_HEADER_ITEM:
							GMT_message (GMT, "Warning: Sequence number %ld, Item %ld is not supported - skipped\n", number, item);
							break;
						default:	/* Update internal structure as well as netCDF file attributes */
							length = (MGD77_Header_Lookup[key].length == 1) ? 1 : strlen (answer);
							strncpy (MGD77_Header_Lookup[key].ptr[MGD77_REVISED], answer, (size_t)length);
							MGD77_Put_Param (GMT, &In, MGD77_Header_Lookup[key].name, length, MGD77_Header_Lookup[key].ptr[MGD77_ORIG], length, MGD77_Header_Lookup[key].ptr[MGD77_REVISED], 2);
							n_E77_headers++;
							break;
					}
				}
				else {			/* Systematic fixes */
					if ((id = MGD77_Get_Column (GMT, field, &In)) == MGD77_NOT_SET) {
						GMT_message (GMT, "Warning: Correction found for %s which is not in this cruise?\n", field);
					}
					else {
						k = MGD77_Info_from_Abbrev (GMT, field, &(D->H), &set, &item);
						value = atof (answer);
						switch (number) {
							case E77_HDR_PDR:	/* Must deal with undetected Precision Depth Recorder wrap-arounds - this also force recalc of depth when data is read*/
								MGD77_nc_status (GMT, nc_put_att_double (In.nc_id, NC_GLOBAL, "PDR_wrap", NC_DOUBLE, (size_t)1, &value));
								cdf_adjust = MGD77_COL_ADJ_TWT;
								MGD77_nc_status (GMT, nc_put_att_int (In.nc_id, D->H.info[set].col[id].var_id, "adjust", NC_INT, (size_t)1, &cdf_adjust));
								n_E77_recalcs++;
								if ((id = MGD77_Get_Column (GMT, "depth", &In)) == MGD77_NOT_SET) {
									GMT_message (GMT, "Warning: Correction implied for %s which is not in this cruise?\n", field);
									break;
								}
								/* no break - we want to fall through and also set depth adjustment */
							case E77_HDR_CARTER:	/* Recalculate Carter depth from twt */
								cdf_adjust = MGD77_COL_ADJ_DEPTH;
								MGD77_nc_status (GMT, nc_put_att_int (In.nc_id, D->H.info[set].col[id].var_id, "adjust", NC_INT, (size_t)1, &cdf_adjust));
								n_E77_recalcs++;
								break;
							case E77_HDR_ANOM_MAG:	/* Recalculate anomaly mag as mtf1 - igrf */
								cdf_adjust = MGD77_COL_ADJ_MAG;
								MGD77_nc_status (GMT, nc_put_att_int (In.nc_id, D->H.info[set].col[id].var_id, "adjust", NC_INT, (size_t)1, &cdf_adjust));
								n_E77_recalcs++;
								break;
							case E77_HDR_ANOM_FAA:	/* Recalculate anomaly faa as gobs - igf */
								cdf_adjust = MGD77_COL_ADJ_FAA;
								MGD77_nc_status (GMT, nc_put_att_int (In.nc_id, D->H.info[set].col[id].var_id, "adjust", NC_INT, (size_t)1, &cdf_adjust));
								n_E77_recalcs++;
								break;
							case E77_HDR_ANOM_FAA_EOT:	/* Recalculate anomaly faa as gobs - igf + eot */
								cdf_adjust = MGD77_COL_ADJ_FAA_EOT;
								MGD77_nc_status (GMT, nc_put_att_int (In.nc_id, D->H.info[set].col[id].var_id, "adjust", NC_INT, (size_t)1, &cdf_adjust));
								n_E77_recalcs++;
								break;
							case E77_HDR_SCALE:	/* Correction scale factor */
								if (D->H.info[set].col[id].corr_factor == 1.0) {	/* Must add a new attribute to the file */
									D->H.info[set].col[id].corr_factor = value;
									MGD77_nc_status (GMT, nc_put_att_double (In.nc_id, D->H.info[set].col[id].var_id, "corr_factor", NC_DOUBLE, (size_t)1, &D->H.info[set].col[id].corr_factor));
								}
								n_E77_scales++;
								break;
							case E77_HDR_DCSHIFT:	/* Correction offset */
								if (D->H.info[set].col[id].corr_offset == 0.0) {	/* Must add a new attribute to the file */
									D->H.info[set].col[id].corr_offset = value;
									MGD77_nc_status (GMT, nc_put_att_double (In.nc_id, D->H.info[set].col[id].var_id, "corr_offset", NC_DOUBLE, (size_t)1, &D->H.info[set].col[id].corr_offset));
								}
								n_E77_offsets++;
								break;
							case E77_HDR_GRID_OFFSET:	/* Range of bad values - set flags to BAD  */
							case E77_HDR_FLAGRANGE:		/* Range of bad values - set flags to BAD  */
								sscanf (answer, "%ld-%ld", &from, &to);
								if (from < 1 || from > D->H.n_records || to < 1 || to > D->H.n_records || to < from) {
									GMT_report (GMT, GMT_MSG_FATAL, "Error: Record range %s is invalid.  Correction skipped\n", answer);
									break;
								}
								pattern = set_bit (id);
 								for (rec = from-1; rec < to; rec++, n_E77_flags++) flags[rec] |= pattern;	/* -1 to get C indices */
								break;
							default:
								break;
						}
					}
				}
			}
			/* Now start on data record section */
			has_time = TRUE;
			if ((it = skip_if_missing (GMT, "time", list[argno], &In, &D)) == MGD77_NOT_SET)
				has_time = FALSE;
			else {	/* See if we really have time or if they are all NaN */
				tvar = D->values[it];
				for (rec = 0, has_time = FALSE; !has_time && rec < D->H.n_records; rec++) if (!GMT_is_dnan (tvar[rec])) has_time = TRUE;
			}
			while (GMT_fgets (GMT, line, GMT_BUFSIZ, fp_e)) {	/* Read until EOF */
				sscanf (line, "%c %s %s %ld %s", &YorN, ID, timestamp, &rec, code);
				if (strcmp (In.NGDC_id, ID)) {
					GMT_report (GMT, GMT_MSG_FATAL, "Error: E77 Conflict %s : ID = %s versus %s in data records - skipped\n", efile, ID, In.NGDC_id);
					e_error++;
					continue;
				}
				if (YorN == 'N') continue;			/* Already decided NOT to use this correction */
				if (YorN == '?' && !Ctrl->A.ignore_verify) {		/* Undecided: Output the warning and goto next line unless we ignore verification */
					GMT_message (GMT, "%s UNDECIDED: %s\n", list[argno], line);
					continue;
				}
				/* Here, YorN is 'Y' (or '?' if Ctrl->A.ignore_verify is TRUE) */
				rec--;	/* E77 starts with rec = 1 for first data record */
				if (has_time) {
					if (!strcmp(timestamp,"NaN")) {
						GMT_report (GMT, GMT_MSG_NORMAL, "Warning: %s: E77 time stamp %s, using recno\n", ID, timestamp);
					}
					else {	/* Must try to interpret the timestamp */
						if (GMT_verify_expectations (GMT, GMT_IS_ABSTIME, GMT_scanf (GMT, timestamp, GMT_IS_ABSTIME, &rec_time), timestamp)) {
							GMT_report (GMT, GMT_MSG_FATAL, "Error: %s: E77 time stamp (%s) in wrong format? - skipped\n", ID, timestamp);
							continue;
						}
						del_t = fabs (tvar[rec] - rec_time);
						if (del_t > (0.06 + GMT_CONV_LIMIT)) {	/* 0.06 is finest time step in MGD77 file so we allow that much slop */
							GMT_report (GMT, GMT_MSG_FATAL, "Error: %s: E77 time stamp and record number do not match record time (del_t = %g s) - skipped\n", ID, del_t);
							continue;
						}
					}
				}
				pos = 0;
				item = -1;	/* So we increment to 0 inside the loop */
				while (GMT_strtok (GMT, code, "-", &pos, p)) {
					item++;
					if (Ctrl->A.e77_skip_mode[item+2]) continue;	/* Ignore this sort of code */
					if (p[0] == '0') continue;
					for (k = 0; k < (int)strlen(p); k++) {	/* Loop over one or more codes */
					
						if (item == 0) {	/* NAV */
							switch (p[k]) {
								case 'A':	/* Time out of range */
									flags[rec] |= set_bit(NCPOS_TIME);
									n_E77_flags++;
									break;
								case 'B':
									GMT_report (GMT, GMT_MSG_NORMAL, "%s: Decreasing time %s - Source Institution need to sort records\n", list[argno], timestamp);
									break;
								case 'C':	/* Excessive speed - flag time, lon, lat */
									flags[rec] |= set_bit(NCPOS_TIME);
									flags[rec] |= set_bit(NCPOS_LON);
									flags[rec] |= set_bit(NCPOS_LAT);
									n_E77_flags++;
									break;
								case 'D':	/* On land */ 
									flags[rec] |= set_bit(NCPOS_LON);
									flags[rec] |= set_bit(NCPOS_LAT);
									n_E77_flags++;
									break;
								case 'E':	/* Undefined nav - flag time, lon, lat */
									flags[rec] |= set_bit(NCPOS_TIME);
									flags[rec] |= set_bit(NCPOS_LON);
									flags[rec] |= set_bit(NCPOS_LAT);
									n_E77_flags++;
									break;
								default:
									GMT_report (GMT, GMT_MSG_FATAL, "%s: Unrecognized NAV code %c - skipped\n", list[argno], p[k]);
									break;
							}
						}
						else if (p[k] < 'A' || p[k] > 'X') {
							GMT_report (GMT, GMT_MSG_FATAL, "%s: Unrecognized error field %c - skipped\n", list[argno], p[k]);
						}
						else {			/* EO, RANGE, or SLOPE */
							if (p[k] >= 'A' && p[k] <= 'X')	{ /* Valid codes */
								if (p[k] > 'G')
 									key = p[k] - 'A' - 4;	/* H (lat) = 3, J (ptc) = 5, etc (only expect J-X though) */
								else if (p[k] < 'C')
									key = p[k] - 'A' + 1;	/* A (rectype) = 1, B (TZ) = 2 */
								else
  									key = 0;	/* C-G (yyyy,mm,dd,hh,mi) all map to time 0 */
								flags[rec] |= set_bit(key);
								n_E77_flags++;
							}
							else {
								GMT_report (GMT, GMT_MSG_FATAL, "%s: Unrecognized error field %c - skipped\n", list[argno], p[k]);
							}
						}
					}		
				}
			}
			GMT_fclose (GMT, fp_e);

			/* Update E77 history */

			(void) time (&now);
			sprintf (E77, "%s [%s] E77 corrections applied to header: %ld scale: %ld offset: %ld recalc: %ld flags: %ld", ctime(&now), In.user, n_E77_headers, n_E77_scales, n_E77_offsets, n_E77_recalcs, n_E77_flags);
			for (i = 0; E77[i]; i++) if (E77[i] == '\n') E77[i] = ' ';	/* Remove the \n returned by ctime() */
			k = strlen (E77);
			D->H.E77 = GMT_memory (GMT, D->H.E77, k + 1, char);
			strcpy (D->H.E77, E77);
			MGD77_nc_status (GMT, nc_put_att_text (In.nc_id, NC_GLOBAL, "E77", (size_t)k, D->H.E77));
		
			old_flags =  (nc_inq_varid (In.nc_id, "MGD77_flags", &cdf_var_id) == NC_NOERR);	/* TRUE if flag variable exists already */
			
			if (n_E77_flags) {	/* Add flags to netCDF file */
				if (old_flags) {	/* Flag variable exists already - simply replace existing flags with the new ones */
					if (D->flags[0])	/* Was allocated and read */
						memcpy (D->flags[0], flags, (size_t)(D->H.n_records * sizeof (int)));
					else	/* Was not allcoated */
						D->flags[0] = flags;
				}
				else {	/* We need to define the flags for the first time */
					dims[0] = In.nc_recid;
					MGD77_nc_status (GMT, nc_def_var (In.nc_id, "MGD77_flags", NC_INT, 1, dims, &cdf_var_id));	/* Define an array variable */
					memset (answer, 0, (size_t)GMT_BUFSIZ);	/* No default answer */
					strcpy (answer, "MGD77 flags (ON = Bad, OFF = Good) derived from E77 errata");
					MGD77_nc_status (GMT, nc_put_att_text (In.nc_id, cdf_var_id, "comment", strlen (answer), answer));
					D->flags[0] = flags;
				}
				MGD77_nc_status (GMT, nc_enddef (In.nc_id));	/* End define mode. */
				start[0] = 0;
				count[0] = D->H.n_records;
				MGD77_nc_status (GMT, nc_put_vara_int (In.nc_id, cdf_var_id, start, count, (int *)D->flags[0]));
			}
			else if (old_flags) {	/* Had flags from before which we cannot delete */
				MGD77_nc_status (GMT, nc_enddef (In.nc_id));	/* End define mode. */
				GMT_message (GMT, "File %s contains flags from an earlier E77 but this E77 do not contain any flags.\n", list[argno]);
				GMT_message (GMT, "The flags in the file %s will all be set to zero but cannot be removed.\n", list[argno]);
				GMT_message (GMT, "If possible, recreate the MGD77+ file %s from the MGD77 original, then reapply E77.\n", list[argno]);
				start[0] = 0;
				count[0] = D->H.n_records;
				memset (D->flags[0], 0, (size_t)(D->H.n_records * sizeof (int)));	/* Reset all flags to 0 (GOOD) */
				MGD77_nc_status (GMT, nc_put_vara_int (In.nc_id, cdf_var_id, start, count, (int *)D->flags[0]));
			}
			
			MGD77_Free_Dataset (GMT, &D);
			MGD77_Close_File (GMT, &In);
			n_changed++;
			continue;	/* Nothing more to do for this file */
		}
		
		/* Specify the information for the extra column. */
		
		constant = (LEN == 0) ? MGD77_dbl_are_constant (GMT, colvalue, D->H.n_records, limits) : MGD77_txt_are_constant (GMT, text, D->H.n_records, (int)LEN);	/* Do we need to store 1 or n values? */

		if (column != MGD77_NOT_SET) {	/* Is it possible just to replace the existing column? */
			error = 0;
			if (LEN) {
				if (OLDLEN != LEN) {
					GMT_report (GMT, GMT_MSG_FATAL, "Revised text column %s differs in width (%d) from the old values (%d).\n", Ctrl->I.c_abbrev, (int)LEN, (int)OLDLEN);
					error = TRUE;
				}
				if (constant && n_dims == 2) {
					GMT_report (GMT, GMT_MSG_FATAL, "Revised text column %s is constant whereas old values were in an array\n", Ctrl->I.c_abbrev);
					error = TRUE;
				}
				if (!constant && n_dims == 1) {
					GMT_report (GMT, GMT_MSG_FATAL, "Revised text column %s is an array whereas old values is a constant\n", Ctrl->I.c_abbrev);
					error = TRUE;
				}
			}
			else {
				if (constant && n_dims == 1) {
					GMT_report (GMT, GMT_MSG_FATAL, "Revised data column %s is constant whereas old values were in an array\n", Ctrl->I.c_abbrev);
					error = TRUE;
				}
				if (!constant && n_dims == 0) {
					GMT_report (GMT, GMT_MSG_FATAL, "Revised data column %s is an array whereas old values is a constant\n", Ctrl->I.c_abbrev);
					error = TRUE;
				}
			}
			if (error) {
				GMT_report (GMT, GMT_MSG_FATAL, "You must use -D to delete the old information before adding the new information\n");
				continue;
			}
		}
		
		/* OK, here we may either replace an exiting column or add a new one */
		
		if (MGD77_Open_File (GMT, list[argno], &In, MGD77_WRITE_MODE)) return (-1);	/* Only creates the full path to the new file */
	
		MGD77_nc_status (GMT, nc_open (In.path, NC_WRITE, &In.nc_id));	/* Open the file */
		MGD77_nc_status (GMT, nc_redef (In.nc_id));				/* Enter define mode */
		
		dims[0] = In.nc_recid;	dims[1] = LEN;
		start[0] = start[1] = 0;
		count[0] = D->H.n_records;	count[1] = LEN;
		
		if (column == MGD77_NOT_SET) {	/*Adding a new column */
			if (constant) {	/* Simply store one value */
				if (LEN)	/* Text variable */
					MGD77_nc_status (GMT, nc_def_var (In.nc_id, Ctrl->I.c_abbrev, c_nc_type, 1, &dims[1], &cdf_var_id));	/* Define a single text variable */
				else
					MGD77_nc_status (GMT, nc_def_var (In.nc_id, Ctrl->I.c_abbrev, c_nc_type, 0, NULL, &cdf_var_id));		/* Define a single variable */
			}
			else {	/* Must store array */
				if (LEN)	/* Text variable */
					MGD77_nc_status (GMT, nc_def_var (In.nc_id, Ctrl->I.c_abbrev, c_nc_type, 2, dims, &cdf_var_id));		/* Define a 2-D text variable */
				else
					MGD77_nc_status (GMT, nc_def_var (In.nc_id, Ctrl->I.c_abbrev, c_nc_type, 1, dims, &cdf_var_id));		/* Define a number array variable */
			}
		}
		else	/* Reuse, get id */
			MGD77_nc_status (GMT, nc_inq_varid (In.nc_id, Ctrl->I.c_abbrev, &cdf_var_id));
		
		if (Ctrl->I.c_name[0]) MGD77_nc_status (GMT, nc_put_att_text   (In.nc_id, cdf_var_id, "long_name", strlen (Ctrl->I.c_name), Ctrl->I.c_name));
		if (Ctrl->I.c_units[0]) MGD77_nc_status (GMT, nc_put_att_text   (In.nc_id, cdf_var_id, "units", strlen (Ctrl->I.c_units), Ctrl->I.c_units));
		MGD77_nc_status (GMT, nc_put_att_double   (In.nc_id, cdf_var_id, "actual_range", NC_DOUBLE, (size_t)2, limits));
		if (Ctrl->I.c_comment[0]) MGD77_nc_status (GMT, nc_put_att_text   (In.nc_id, cdf_var_id, "comment", strlen (Ctrl->I.c_comment), Ctrl->I.c_comment));
		MGD77_nc_status (GMT, nc_put_att_double (In.nc_id, cdf_var_id, "_FillValue", c_nc_type, (size_t)1, &MGD77_NaN_val[c_nc_type]));
		MGD77_nc_status (GMT, nc_put_att_double (In.nc_id, cdf_var_id, "missing_value", c_nc_type, (size_t)1, &MGD77_NaN_val[c_nc_type]));
		if (Ctrl->A.parameters[COL_SCALE]  != 1.0) MGD77_nc_status (GMT, nc_put_att_double (In.nc_id, cdf_var_id, "scale_factor", NC_DOUBLE, (size_t)1, &Ctrl->A.parameters[COL_SCALE]));
		if (Ctrl->A.parameters[COL_OFFSET] != 0.0) MGD77_nc_status (GMT, nc_put_att_double (In.nc_id, cdf_var_id, "add_offset",   NC_DOUBLE, (size_t)1, &Ctrl->A.parameters[COL_OFFSET]));
					
		/* Update history */

		(void) time (&now);
		sprintf (history, "%s [%s] Column %s added", ctime(&now), In.user, Ctrl->I.c_abbrev);
		k = strlen (history);
		for (i = 0; i < k; i++) if (history[i] == '\n') history[i] = ' ';	/* Remove the \n returned by ctime() */
		history[k++] = '\n';	history[k] = '\0';				/* Add LF at end of line */
		k += strlen (D->H.history);
		D->H.history = GMT_memory (GMT, D->H.history, k, char);
		strcat (D->H.history, history);
		MGD77_nc_status (GMT, nc_put_att_text (In.nc_id, NC_GLOBAL, "history", strlen (D->H.history), D->H.history));
		
		MGD77_nc_status (GMT, nc_enddef (In.nc_id));	/* End define mode.  Now we can write/update data */

		transform = (! (Ctrl->A.parameters[COL_SCALE] == 1.0 && Ctrl->A.parameters[COL_OFFSET] == 0.0));	/* TRUE if we must transform before writing */
		n_bad = 0;
		if (constant) {	/* Simply store one value */
			if (LEN)
				MGD77_nc_status (GMT, nc_put_vara_schar (In.nc_id, cdf_var_id, start, &count[1], (signed char *)text));	/* Just write one text string */
			else {
				n_bad = MGD77_do_scale_offset_before_write (GMT, &single_val, colvalue, (size_t)1, Ctrl->A.parameters[COL_SCALE], Ctrl->A.parameters[COL_OFFSET], c_nc_type);
				MGD77_nc_status (GMT, nc_put_var1_double (In.nc_id, cdf_var_id, start, &single_val));
			}
		}
		else {	/* Must store array */
			if (LEN)
				MGD77_nc_status (GMT, nc_put_vara_schar (In.nc_id, cdf_var_id, start, count, (signed char *)text));
			else if (transform) {
				xtmp = GMT_memory (GMT, NULL, count[0], double);
				n_bad = MGD77_do_scale_offset_before_write (GMT, xtmp, colvalue, D->H.n_records, Ctrl->A.parameters[COL_SCALE], Ctrl->A.parameters[COL_OFFSET], c_nc_type);
				MGD77_nc_status (GMT, nc_put_vara_double (In.nc_id, cdf_var_id, start, count, xtmp));
				GMT_free (GMT, xtmp);
			}
			else 
				MGD77_nc_status (GMT, nc_put_vara_double (In.nc_id, cdf_var_id, start, count, colvalue));
		}
		if (n_bad) {	/* Report what we found */
			if (In.verbose_level | 1) fprintf (fp_err, "%s: %s [%s] had %ld values outside valid range <%g,%g> for the chosen type (set to NaN = %g)\n",
				GMT->init.progname, In.NGDC_id, Ctrl->I.c_abbrev, n_bad, MGD77_Low_val[c_nc_type], MGD77_High_val[c_nc_type], MGD77_NaN_val[c_nc_type]);
		}
		
		MGD77_Close_File (GMT, &In);
		MGD77_Free_Dataset (GMT, &D);
		n_changed++;
	}
	
	if (got_table) GMT_free (GMT, colvalue);
	if (two_cols) GMT_free (GMT, coldnt);
	
	if (Ctrl->D.active)
		GMT_report (GMT, GMT_MSG_NORMAL, "Removed %ld data columns from %ld MGD77 files\n", n_delete, n_changed);
	else if (Ctrl->A.mode == MODE_e)
		GMT_report (GMT, GMT_MSG_NORMAL, "E77 corrections applied to %ld MGD77 files\n", n_changed);
	else
		GMT_report (GMT, GMT_MSG_NORMAL, "Sampled data for %ld MGD77 files\n", n_changed);
	
	MGD77_Path_Free (GMT, n_paths, list);
	MGD77_end (GMT, &In);

	Return (GMT_OK);
}
