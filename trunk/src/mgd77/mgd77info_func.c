/*--------------------------------------------------------------------
 *	$Id: mgd77info_func.c,v 1.12 2011-06-22 08:10:24 jluis Exp $
 *
 *    Copyright (c) 2004-2011 by P. Wessel
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mgd77info reads one or more MGD77 or MGD77+ files and report on the
 * extent of the file, number of data points etc.  Alternatively, it
 * can echo out the entire MGD77 header section or list columns that
 * are present.
 *
 * Author:	Paul Wessel
 * Date:	26-AUG-2004
 * Version:	1.0 Ideal based on the old gmtinfo.c
 *		2005-SEP-05: Added -P [PW]
 *		2005-OCT-07: Added -C,-I [PW]
 *		2006-MAR-31: Changed -H (header info) to -M (metadata)
 *		2007-JUN-14: Added -Me|h also
 *
 *
 */
 
#include "gmt_mgd77.h"
#include "mgd77.h"
#include "mgd77_codes.h"

#define FORMATTED_HEADER	1
#define RAW_HEADER		2
#define E77_HEADER		3
#define HIST_HEADER		4

struct MGD77INFO_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct C {	/* -C */
		GMT_LONG active;
		GMT_LONG mode;
	} C;
	struct E {	/* -E */
		GMT_LONG active;
		GMT_LONG mode;
	} E;
	struct I {	/* -I */
		GMT_LONG active;
		GMT_LONG n;
		char code[3];
	} I;
	struct L {	/* -L */
		GMT_LONG active;
		GMT_LONG mode;
	} L;
	struct M {	/* -M */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG flag;
	} M;
};

void *New_mgd77info_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77INFO_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct MGD77INFO_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	C->C.mode = 3;	
	C->L.mode = 1;
	return ((void *)C);
}

void Free_mgd77info_Ctrl (struct GMT_CTRL *GMT, struct MGD77INFO_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);	
}

GMT_LONG GMT_mgd77info_usage (struct GMTAPI_CTRL *C, GMT_LONG level, struct MGD77INFO_CTRL *Ctrl)
{
	struct GMT_CTRL *GMT = C->GMT;
	struct MGD77_CONTROL M;

	GMT_message (GMT, "mgd77info %s [API]Return ( - Extract information about MGD77 files\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: mgd77info <cruise(s)> [-C[m|e]] [-E[m|e]] [-I<code>] [-Mf[<item>]|r|e|h] [-L[v]] [-V]\n\n");
        
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);
             
	MGD77_Init (GMT, &M);		/* Initialize MGD77 Machinery */
	MGD77_Cruise_Explain (GMT);
	GMT_message (GMT, "\tOPTIONS:\n\n");
	GMT_message (GMT, "\t-C List abbreviations of all columns present for each cruise\n");
	GMT_message (GMT, "\t   Append m for listing just the MGD77 columns present\n");
	GMT_message (GMT, "\t   Append e for listing just any extra columns present\n");
	GMT_message (GMT, "\t-E Give the information summary of each cruise's geographical/temporal extent\n");
	GMT_message (GMT, "\t   Append m for counting just the number of non-NaN values for each MGD77 field\n");
	GMT_message (GMT, "\t   Append e for counting just the of non-NaN values for each extra field\n");
	GMT_message (GMT, "\t-M Print header items (and MGD77+ history).  Append type of presentation:\n");
	GMT_message (GMT, "\t     f: Print header items individually, one per line.  Append name of a particular\n");
	GMT_message (GMT, "\t        item (e.g. Port_of_Departure), all [Default], or - to see a list of items.\n");
	GMT_message (GMT, "\t        You can also use the number of the item.\n");
	GMT_message (GMT, "\t     r: Display raw original MGD77 header records.\n");
	GMT_message (GMT, "\t     e: Display the MGD77+ file's E77 status.\n");
	GMT_message (GMT, "\t     h: Display the MGD77+ file's history.\n");
	GMT_message (GMT, "\t-I Ignore certain data file formats from consideration. Append combination of act to ignore\n");
	GMT_message (GMT, "\t   (a) MGD77 ASCII, (c) MGD77+ netCDF, or (t) plain table files. [Default ignores none]\n");
	GMT_message (GMT, "\t-L Just list all the institutions and their 2-character GEODAS codes.  Append v to also\n");
	GMT_message (GMT, "\t   display the vessels and their 4-character codes for each institution\n");
	GMT_explain_options (GMT, "V");
	
	MGD77_end (GMT, &M);	/* Close machinery */

	return (EXIT_FAILURE);
}

GMT_LONG GMT_mgd77info_parse (struct GMTAPI_CTRL *C, struct MGD77INFO_CTRL *Ctrl, struct GMT_OPTION *options, struct MGD77_CONTROL *M)
{
	/* This parses the options provided to mgd77info and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
			case '#':	/* Skip input files confused as numbers (e.g. 123456) */
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Get the short list [Default] */
				Ctrl->C.active = TRUE;
				switch (opt->arg[0]) {
					case 'm':
					case 'M':
						Ctrl->C.mode = 1;
						break;
					case 'e':
					case 'E':
						Ctrl->C.mode = 2;
						break;
					default:
						Ctrl->C.mode = 3;
						break;
				}
				break;

			case 'M':
				Ctrl->M.active = TRUE;
				if (opt->arg[0] == 'f') {
					Ctrl->M.mode = FORMATTED_HEADER;
					Ctrl->M.flag = MGD77_Select_Header_Item (GMT, M, &opt->arg[1]);
					if (Ctrl->M.flag < 0) n_errors++;
				}
				else if (opt->arg[0] == 'r') {
					Ctrl->M.mode = RAW_HEADER;
				}
				else if (opt->arg[0] == 'e') {
					Ctrl->M.mode = E77_HEADER;
				}
				else if (opt->arg[0] == 'h') {
					Ctrl->M.mode = HIST_HEADER;
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Option -M Bad modifier (%c). Use -Mf|r|e|h!\n", opt->arg[0]);
					n_errors++;
				}
				break;

			case 'I':
				Ctrl->I.active = TRUE;
				if (Ctrl->I.n < 3) {
					if (strchr ("act", (int)opt->arg[0]))
						Ctrl->I.code[Ctrl->I.n++] = opt->arg[0];
					else {
						GMT_report (GMT, GMT_MSG_FATAL, "Option -I Bad modifier (%c). Use -Ia|c|t!\n", opt->arg[0]);
						n_errors++;
					}
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Option -I: Can only be applied 0-2 times\n");
					n_errors++;
				}
				break;

			case 'E':	/* Get the short list [Default] */
				switch (opt->arg[0]) {
					case 'm':
					case 'M':
						Ctrl->E.mode = 1;
						break;
					case 'e':
					case 'E':
						Ctrl->E.mode = 2;
						break;
					case '\0':
						Ctrl->E.mode = 3;
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "Option -E Bad modifier (%c). Use -E[e|m]!\n", opt->arg[0]);
						n_errors++;
				}
				Ctrl->E.active = TRUE;
				break;

			case 'L':	/* Get the list of institutions and vessels  */
				Ctrl->L.active = TRUE;
				switch (opt->arg[0]) {
					case 'a':
						Ctrl->L.mode = 1;
						break;
					case 'v':
						Ctrl->L.mode = 2;
						break;
					case '\0':
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "Option -L Bad modifier (%c). Use -L[a|v]!\n", opt->arg[0]);
						n_errors++;
				}
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !((Ctrl->M.mode == RAW_HEADER) + (Ctrl->M.mode == E77_HEADER) + (Ctrl->M.mode == HIST_HEADER) \
		+ Ctrl->E.active + Ctrl->C.active + (Ctrl->M.mode == FORMATTED_HEADER) + Ctrl->L.active ) == 1, "Syntax error: Specify one of -C, -E, -L, or -M\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_mgd77info_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); GMT_exit (code);}

GMT_LONG GMT_mgd77info (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG i, id, rec, argno, length, id_col, t_col, x_col, y_col, saved_range, use;
	GMT_LONG n_paths, counter[MGD77_MAX_COLS], quad_no, n_quad, rata_die;
	GMT_LONG error = FALSE, greenwich = FALSE, first = TRUE, read_file;
	GMT_LONG quad[4] = {FALSE, FALSE, FALSE, FALSE};
	
	double this_dist, this_lon, this_lat, last_lon, last_lat, dx, dy, dlon, ds, lon_w;
	double xmin, xmax, xmin1, xmin2, xmax1, xmax2, ymin, ymax, this_time, tmin, tmax;
	double *dvalue[MGD77_MAX_COLS];
	
	char *tvalue[MGD77_MAX_COLS], **list = NULL;
		
	struct MGD77_CONTROL M, Out;
	struct MGD77_DATASET *D = NULL;
	struct MGD77INFO_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == '?') return (GMT_mgd77info_usage (API, GMTAPI_USAGE, Ctrl));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_mgd77info_usage (API, GMTAPI_SYNOPSIS, Ctrl));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_mgd77info", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-V", "", options))) Return ((int)error);
	Ctrl = (struct MGD77INFO_CTRL *) New_mgd77info_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	MGD77_Init (GMT, &M);		/* Initialize MGD77 Machinery */
	if ((error = GMT_mgd77info_parse (API, Ctrl, options, &M))) Return ((int)error);

	/*---------------------------- This is the mgd77info main code ----------------------------*/

	if (Ctrl->M.flag == 1) {
		MGD77_List_Header_Items (GMT, &M);
		MGD77_end (GMT, &M);
		Return (GMT_OK);
	}

	GMT_get_time_system (GMT, "unix", &(GMT->current.setting.time_system));						/* MGD77+ uses GMT's Unix time epoch */
	GMT_init_time_system_structure (GMT, &(GMT->current.setting.time_system));
	
	/* Initialize MGD77 output order and other parameters*/
	
	MGD77_Init (GMT, &Out);		/* Initialize output MGD77 Machinery */
	Out.fp = GMT->session.std[GMT_OUT];

	if (Ctrl->I.active) MGD77_Process_Ignore (GMT, 'I', Ctrl->I.code);

	/* Check that the options selected are mutually consistent */
	
	if (Ctrl->L.active) {	/* Just display the list and exit */
		(Ctrl->L.mode == 2) ? printf ("CODE\tINSTITUTION/VESSEL\n") : printf ("CODE\tINSTITUTION\n");
		for (id = i = 0; id < MGD77_N_AGENCIES; id++) {
			printf ("%s = %s\n", MGD77_agency[id].code, MGD77_agency[id].name);
			for (; Ctrl->L.mode == 2 && i < MGD77_N_VESSELS && MGD77_vessel[i].agent == id; i++) {
				printf ("%s\t-> %s\n", MGD77_vessel[i].code, MGD77_vessel[i].name);
			}
		}
		Return (GMT_OK);
	}

	n_paths = MGD77_Path_Expand (GMT, &M, options, &list);	/* Get list of requested IDs */
	
	if (n_paths == 0) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: No cruises given\n");
		Return (EXIT_FAILURE);
	}
	
	read_file = (Ctrl->E.active || (Ctrl->M.mode == RAW_HEADER));
	
	saved_range = GMT->current.io.geo.range;	/* We may have to reset thisso keep a copy */
	GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;	GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
	GMT->current.io.col_type[GMT_OUT][GMT_Z] = M.time_format;	
	if (Ctrl->E.active) fprintf (GMT->session.std[GMT_OUT], "#Cruise %sID      %sWest    %sEast    %sSouth   %sNorth   %sStartTime%s%sEndTime%s%s%sDist%snRec",
		GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator,
		GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator);

	use = (M.original || M.format != MGD77_FORMAT_CDF) ? MGD77_ORIG : MGD77_REVISED;
	
	for (argno = 0; argno < n_paths; argno++) {		/* Process each ID */
	
		if (MGD77_Open_File (GMT, list[argno], &M, MGD77_READ_MODE)) continue;

		GMT_report (GMT, GMT_MSG_NORMAL, "Now processing cruise %s\n", list[argno]);
		
		D = MGD77_Create_Dataset (GMT);
		
		if (read_file && MGD77_Read_File (GMT, list[argno], &M, D)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading header & data for cruise %s\n", list[argno]);
			Return (EXIT_FAILURE);
		}
		if (!read_file && MGD77_Read_Header_Record (GMT, list[argno], &M, &D->H)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading header sequence for cruise %s\n", list[argno]);
			Return (EXIT_FAILURE);
		}

		if (Ctrl->M.mode == HIST_HEADER) {	/* Dump of MGD77+ history */
			fprintf (GMT->session.std[GMT_OUT], "%s: %s", list[argno], D->H.history);
			MGD77_Close_File (GMT, &M);
			MGD77_Free (GMT, D);
			continue;
		}
		if (Ctrl->M.mode == E77_HEADER) {	/* Dump of e77 header status */
			if (D->H.E77 && strlen(D->H.E77) > 0)
				fprintf (GMT->session.std[GMT_OUT], "%s: %s\n", list[argno], D->H.E77);
			else
				fprintf (GMT->session.std[GMT_OUT], "%s: E77 not applied\n", list[argno]);
			MGD77_Close_File (GMT, &M);
			MGD77_Free (GMT, D);
			continue;
		}
		if (Ctrl->M.mode == FORMATTED_HEADER) {	/* Dump of header items, one per line */
			MGD77_Dump_Header_Params (GMT, &M, D->H.mgd77[use]);	
			MGD77_Close_File (GMT, &M);
			MGD77_Free (GMT, D);
			continue;
		}
		t_col = MGD77_Get_Column (GMT, "time", &M);
		x_col = MGD77_Get_Column (GMT, "lon", &M);
		y_col = MGD77_Get_Column (GMT, "lat", &M);
		id_col = MGD77_Get_Column (GMT, "id", &M);
		
		if (first && Ctrl->E.active) {	/* Output all column headers */
			for (i = 0; i < M.n_out_columns; i++) {
				if (i == id_col || i == t_col || i == x_col || i == y_col) continue;
				fprintf (GMT->session.std[GMT_OUT],"%s%s", GMT->current.setting.io_col_separator, D->H.info[M.order[i].set].col[M.order[i].item].abbrev);
			}
			fprintf (GMT->session.std[GMT_OUT],"\n");
		}
		
		if (Ctrl->C.active) {	/* Just list names and info for any extra columns */
			for (i = 0, first = TRUE; i < M.n_out_columns; i++) {
				if (i == id_col || i == t_col || i == x_col || i == y_col) continue;
				if (!first) fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
				if (((Ctrl->C.mode & 1) && M.order[i].set == 0) || ((Ctrl->C.mode & 2) && M.order[i].set == 1)) {
					fprintf (GMT->session.std[GMT_OUT], "%s", D->H.info[M.order[i].set].col[M.order[i].item].abbrev);
					first = FALSE;
				}
			}
			if (first) fprintf (GMT->session.std[GMT_OUT], "No columns matching selection found!");
			fprintf (GMT->session.std[GMT_OUT], "\n");
			MGD77_Close_File (GMT, &M);
			MGD77_Free (GMT, D);
			continue;
		}
			
		if (Ctrl->M.mode == RAW_HEADER) {	/* Write entire MGD77 header */
			fprintf (GMT->session.std[GMT_OUT], "-------------------------------");
			fprintf (GMT->session.std[GMT_OUT], " Cruise: %8s ", M.NGDC_id);
			fprintf (GMT->session.std[GMT_OUT], "-------------------------------\n");
			MGD77_Write_Header_Record_m77 (GMT, "", &Out, &D->H);
			fprintf (GMT->session.std[GMT_OUT], "----------------------------------------");
			fprintf (GMT->session.std[GMT_OUT], "----------------------------------------\n");
			if (M.format == MGD77_FORMAT_CDF) {
				fprintf (GMT->session.std[GMT_OUT], "%s\n", D->H.history);
				for (i = 0; i < M.n_out_columns; i++) {
					if ((M.order[i].set == MGD77_CDF_SET)) {
						fprintf (GMT->session.std[GMT_OUT], "> %s%s%s%s%s%s%s", D->H.info[MGD77_CDF_SET].col[M.order[i].item].abbrev, GMT->current.setting.io_col_separator,
						D->H.info[MGD77_CDF_SET].col[M.order[i].item].name, GMT->current.setting.io_col_separator,
						D->H.info[MGD77_CDF_SET].col[M.order[i].item].units, GMT->current.setting.io_col_separator,
						D->H.info[MGD77_CDF_SET].col[M.order[i].item].comment);
					}
				}
			}
			fprintf (GMT->session.std[GMT_OUT], "\n");
		}
		
		GMT_make_dnan (tmin);
		GMT_make_dnan (tmax);
		this_dist = this_lon = this_lat = ds = this_time = 0.0;
		xmin1 = xmin2 = 360.0;
		xmax1 = xmax2 = -360.0;
		ymin = 180.0;
		ymax = -180.0;
		GMT_memset (quad, 4, GMT_LONG);	/* Set al to FALSE */
		greenwich = FALSE;
		GMT_memset (counter, MGD77_MAX_COLS, GMT_LONG);
	
		for (i = 0; i < MGD77_MAX_COLS; i++) {
			dvalue[i] = (double *)D->values[i];
			tvalue[i] = (char *)D->values[i];
		}
		
		/* Start processing data */
	
		for (rec = 0; rec < D->H.n_records; rec++) {		/* While able to read a data record */
		
			/* Get min and max time */
			if (t_col >= 0 && !GMT_is_dnan(dvalue[t_col][rec])) {
				if (GMT_is_dnan(tmin) && GMT_is_dnan(tmax)) tmin = tmax = dvalue[t_col][rec];
				this_time = dvalue[t_col][rec];
				tmin = MIN (this_time, tmin);
				tmax = MAX (this_time, tmax);
			}

			/* Compute accumulated distance along track (Flat Earth) */
			last_lon  = this_lon;
			last_lat  = this_lat;
			this_lon  = lon_w = dvalue[x_col][rec];
			this_lat  = dvalue[y_col][rec];
			if (this_lon < 0.0) this_lon += 360.0;	/* Start off with everything in 0-360 range */
			xmin1 = MIN (this_lon, xmin1);
			xmax1 = MAX (this_lon, xmax1);
			quad_no = (GMT_LONG)floor (this_lon/90.0);	/* Yields quadrants 0-3 */
			if (quad_no == 4) quad_no = 0;		/* When this_lon == 360.0 */
			quad[quad_no] = TRUE;
			if (lon_w > 180.0) this_lon -= 360.0;	/* For -180/+180 range */
			xmin2 = MIN (this_lon, xmin2);
			xmax2 = MAX (this_lon, xmax2);
			if (rec > 0) {	/* Need a previous point to calculate distance, speed, and heading */
				dlon = this_lon - last_lon;
				if (fabs (dlon) > 180.0) {
					greenwich = TRUE;
					dlon = copysign ((360.0 - fabs (dlon)), dlon);
				}
				dx = dlon * cosd (0.5 * (this_lat + last_lat));
				dy = this_lat - last_lat;
				ds = GMT->current.proj.DIST_KM_PR_DEG * hypot (dx, dy);
				this_dist += ds;
			}
			ymin = MIN (this_lat, ymin);
			ymax = MAX (this_lat, ymax);
			
			/* Count the number of non-NaN observations */
			
			for (i = 1; i < M.n_out_columns; i++) {
				if (i == id_col || i == t_col || i == x_col || i == y_col) continue;
				if ((length = D->H.info[M.order[i].set].col[M.order[i].item].text)) {
					if (strncmp (&tvalue[i][rec*length], ALL_NINES, (size_t)length)) counter[i]++;
				}
				else
					if (!GMT_is_dnan (dvalue[i][rec])) counter[i]++;
			}
		}

		GMT->current.io.geo.range = saved_range;	/* We reset this each time */
		n_quad = quad[0] + quad[1] + quad[2] + quad[3];	/* How many quadrants had data */
		if (quad[0] && quad[3]) {	/* Longitudes on either side of Greenwich only, must use -180/+180 notation */
			xmin = xmin2;
			xmax = xmax2;
			GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;	/* Override this setting explicitly */
		}
		else if (quad[1] && quad[2]) {	/* Longitudes on either side of the date line, must user 0/360 notation */
			xmin = xmin1;
			xmax = xmax1;
			GMT->current.io.geo.range = GMT_IS_0_TO_P360_RANGE;	/* Override this setting explicitly */
		}
		else if (n_quad == 2 && ((quad[0] && quad[2]) || (quad[1] && quad[3]))) {	/* Funny quadrant gap, pick shortest longitude extent */
			if ((xmax1 - xmin1) < (xmax2 - xmin2)) {	/* 0/360 more compact */
				xmin = xmin1;
				xmax = xmax1;
				GMT->current.io.geo.range = GMT_IS_0_TO_P360_RANGE;	/* Override this setting explicitly */
			}
			else {						/* -180/+180 more compact */
				xmin = xmin2;
				xmax = xmax2;
				GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;	/* Override this setting explicitly */
			}
		}
		else {						/* Either will do, use default settings */
			xmin = xmin1;
			xmax = xmax1;
		}
		if (xmin > xmax) xmin -= 360.0;
		if (xmin < 0.0 && xmax < 0.0) xmin += 360.0, xmax += 360.0;

		if (GMT_is_dnan(tmin) || GMT_is_dnan(tmax)) {
			int yy[2], mm[2], dd[2];
			GMT_report (GMT, GMT_MSG_NORMAL, "warning: cruise %s no time records.\n", M.NGDC_id);
			yy[0] = (!D->H.mgd77[use]->Survey_Departure_Year[0] || !strncmp (D->H.mgd77[use]->Survey_Departure_Year, ALL_BLANKS, (size_t)4)) ? 0 : atoi (D->H.mgd77[use]->Survey_Departure_Year);
			yy[1] = (!D->H.mgd77[use]->Survey_Arrival_Year[0] || !strncmp (D->H.mgd77[use]->Survey_Arrival_Year, ALL_BLANKS, (size_t)4)) ? 0 : atoi (D->H.mgd77[use]->Survey_Arrival_Year);
			mm[0] = (!D->H.mgd77[use]->Survey_Departure_Month[0] || !strncmp (D->H.mgd77[use]->Survey_Departure_Month, ALL_BLANKS, (size_t)2)) ? 1 : atoi (D->H.mgd77[use]->Survey_Departure_Month);
			mm[1] = (!D->H.mgd77[use]->Survey_Arrival_Month[0] || !strncmp (D->H.mgd77[use]->Survey_Arrival_Month, ALL_BLANKS, (size_t)2)) ? 1 : atoi (D->H.mgd77[use]->Survey_Arrival_Month);
			dd[0] = (!D->H.mgd77[use]->Survey_Departure_Day[0] || !strncmp (D->H.mgd77[use]->Survey_Departure_Day, ALL_BLANKS, (size_t)2)) ? 1 : atoi (D->H.mgd77[use]->Survey_Departure_Day);
			dd[1] = (!D->H.mgd77[use]->Survey_Arrival_Day[0] || !strncmp (D->H.mgd77[use]->Survey_Arrival_Day, ALL_BLANKS, (size_t)2)) ? 1 : atoi (D->H.mgd77[use]->Survey_Arrival_Day);
			if (! (yy[0] == 0 && yy[1] == 0)) {	/* With year we can do something */
				rata_die = GMT_rd_from_gymd (GMT, yy[0], mm[0], dd[0]);
				tmin = GMT_rdc2dt (GMT, rata_die, 0.0);
				rata_die = GMT_rd_from_gymd (GMT, yy[1], mm[1], dd[1]);
				tmax = GMT_rdc2dt (GMT, rata_die, 0.0);
			}
		}			
		if (Ctrl->E.active) {
			fprintf (GMT->session.std[GMT_OUT],"%8s%s%8s%s", M.NGDC_id, GMT->current.setting.io_col_separator, D->H.mgd77[use]->Survey_Identifier, GMT->current.setting.io_col_separator);
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], xmin, GMT_X);	fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], xmax, GMT_X);	fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], ymin, GMT_Y);	fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], ymax, GMT_Y);	fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
			if (!GMT_is_dnan(tmin) && !GMT_is_dnan(tmax)) {
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], tmin, GMT_Z);	fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], tmax, GMT_Z);	fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);						
			} else {
				fprintf (GMT->session.std[GMT_OUT], "%4s-%2s-%2s%s%4s-%2s-%2s%s",
				D->H.mgd77[use]->Survey_Departure_Year, D->H.mgd77[use]->Survey_Departure_Month, D->H.mgd77[use]->Survey_Departure_Day, GMT->current.setting.io_col_separator,
				D->H.mgd77[use]->Survey_Arrival_Year, D->H.mgd77[use]->Survey_Arrival_Month, D->H.mgd77[use]->Survey_Arrival_Day, GMT->current.setting.io_col_separator);
			}
			fprintf (GMT->session.std[GMT_OUT], "%ld%s%ld", (GMT_LONG)irint (this_dist), GMT->current.setting.io_col_separator, D->H.n_records);
			for (i = 1; i < M.n_out_columns; i++) {
				if (i == id_col || i == t_col || i == x_col || i == y_col) continue;
				if (((Ctrl->E.mode & 1) && M.order[i].set == 0) || ((Ctrl->E.mode & 2) && M.order[i].set == 1))
					fprintf (GMT->session.std[GMT_OUT],"%s%ld",	GMT->current.setting.io_col_separator, counter[i]);
			}
			fprintf (GMT->session.std[GMT_OUT],"\n");
		}
		MGD77_Free (GMT, D);
	}
		
	MGD77_Path_Free (GMT, (int)n_paths, list);
	MGD77_end (GMT, &M);

	Return (GMT_OK);
}
