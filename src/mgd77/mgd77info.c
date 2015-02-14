/*--------------------------------------------------------------------
 *	$Id$
 *
 *    Copyright (c) 2004-2015 by P. Wessel
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
 
#define THIS_MODULE_NAME	"mgd77info"
#define THIS_MODULE_LIB		"mgd77"
#define THIS_MODULE_PURPOSE	"Extract information about MGD77 files"
#define THIS_MODULE_KEYS	">TO"

#include "gmt_dev.h"
#include "mgd77.h"
#include "mgd77_codes.h"

#define GMT_PROG_OPTIONS "-V"

#define FORMATTED_HEADER	1
#define RAW_HEADER		2
#define E77_HEADER		3
#define HIST_HEADER		4

struct MGD77INFO_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct C {	/* -C */
		bool active;
		unsigned int mode;
	} C;
	struct E {	/* -E */
		bool active;
		unsigned int mode;
	} E;
	struct I {	/* -I */
		bool active;
		unsigned int n;
		char code[3];
	} I;
	struct L {	/* -L */
		bool active;
		unsigned int mode;
	} L;
	struct M {	/* -M */
		bool active;
		unsigned int mode;
		unsigned int flag;
	} M;
};

void *New_mgd77info_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77INFO_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct MGD77INFO_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	C->C.mode = 3;	
	C->L.mode = 1;
	return (C);
}

void Free_mgd77info_Ctrl (struct GMT_CTRL *GMT, struct MGD77INFO_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);	
}

int GMT_mgd77info_usage (struct GMTAPI_CTRL *API, int level)
{
	struct MGD77_CONTROL M;

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: mgd77info <cruise(s)> [-C[m|e]] [-E[m|e]] [-I<code>] [-Mf[<item>]|r|e|h] [-L[v]]\n\t[%s]\n\n", GMT_V_OPT);
        
	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);
             
	MGD77_Init (API->GMT, &M);		/* Initialize MGD77 Machinery */
	MGD77_Cruise_Explain (API->GMT);
	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C List abbreviations of all columns present for each cruise.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append m for listing just the MGD77 columns present.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append e for listing just any extra columns present.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Give the information summary of each cruise's geographical/temporal extent.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append m for counting just the number of non-NaN values for each MGD77 field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append e for counting just the of non-NaN values for each extra field.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Print header items (and MGD77+ history).  Append type of presentation:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f: Print header items individually, one per line.  Append name of a particular\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        item (e.g., Port_of_Departure), all [Default], or - to see a list of items.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        You can also use the number of the item.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r: Display raw original MGD77 header records.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     e: Display the MGD77+ file's E77 status.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     h: Display the MGD77+ file's history.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Ignore certain data file formats from consideration. Append combination of act to ignore\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (a) MGD77 ASCII, (c) MGD77+ netCDF, (m) MGD77T ASCII, or (t) plain table files [Default ignores none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L List all the institutions and their 2-character GEODAS codes only.  Append v to also\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   display the vessels and their 4-character codes for each institution.\n");
	GMT_Option (API, "V,.");
	
	MGD77_end (API->GMT, &M);	/* Close machinery */

	return (EXIT_FAILURE);
}

int GMT_mgd77info_parse (struct GMT_CTRL *GMT, struct MGD77INFO_CTRL *Ctrl, struct GMT_OPTION *options, struct MGD77_CONTROL *M)
{
	/* This parses the options provided to mgd77info and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	int sval;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
			case '#':	/* Skip input files confused as numbers (e.g. 123456) */
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Get the short list [Default] */
				Ctrl->C.active = true;
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
				Ctrl->M.active = true;
				if (opt->arg[0] == 'f') {
					Ctrl->M.mode = FORMATTED_HEADER;
					sval = MGD77_Select_Header_Item (GMT, M, &opt->arg[1]);
					if (sval < 0) n_errors++;
					Ctrl->M.flag = sval;
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
					GMT_Report (API, GMT_MSG_NORMAL, "Option -M Bad modifier (%c). Use -Mf|r|e|h!\n", opt->arg[0]);
					n_errors++;
				}
				break;

			case 'I':
				Ctrl->I.active = true;
				if (Ctrl->I.n < 3) {
					if (strchr ("act", (int)opt->arg[0]))
						Ctrl->I.code[Ctrl->I.n++] = opt->arg[0];
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Option -I Bad modifier (%c). Use -Ia|c|t!\n", opt->arg[0]);
						n_errors++;
					}
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Option -I: Can only be applied 0-2 times\n");
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
						GMT_Report (API, GMT_MSG_NORMAL, "Option -E Bad modifier (%c). Use -E[e|m]!\n", opt->arg[0]);
						n_errors++;
				}
				Ctrl->E.active = true;
				break;

			case 'L':	/* Get the list of institutions and vessels  */
				Ctrl->L.active = true;
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
						GMT_Report (API, GMT_MSG_NORMAL, "Option -L Bad modifier (%c). Use -L[a|v]!\n", opt->arg[0]);
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

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_mgd77info_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mgd77info (void *V_API, int mode, void *args)
{
	int i, id, id_col, t_col, x_col, y_col, error = 0;
	
	int64_t rata_die;
	size_t length;
	
	uint64_t rec, argno, n_paths, counter[MGD77_MAX_COLS];
	unsigned int saved_range, quad_no, n_quad, use, k;
	bool first = true, read_file, quad[4] = {false, false, false, false};
	
	double this_dist, this_lon, this_lat, last_lon, last_lat, dx, dy, dlon, ds, lon_w;
	double xmin, xmax, xmin1, xmin2, xmax1, xmax2, ymin, ymax, this_time, tmin, tmax;
	double *dvalue[MGD77_MAX_COLS];
	
	char *tvalue[MGD77_MAX_COLS], **list = NULL;
		
	struct MGD77_CONTROL M, Out;
	struct MGD77_DATASET *D = NULL;
	struct MGD77INFO_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_mgd77info_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_mgd77info_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_mgd77info_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_mgd77info_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	MGD77_Init (GMT, &M);		/* Initialize MGD77 Machinery */
	if ((error = GMT_mgd77info_parse (GMT, Ctrl, options, &M))) Return (error);

	/*---------------------------- This is the mgd77info main code ----------------------------*/

	if (Ctrl->M.flag == 1) {
		MGD77_List_Header_Items (GMT, &M);
		MGD77_end (GMT, &M);
		Return (GMT_OK);
	}

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
		GMT_Report (API, GMT_MSG_NORMAL, "Error: No cruises given\n");
		Return (EXIT_FAILURE);
	}
	
	read_file = (Ctrl->E.active || (Ctrl->M.mode == RAW_HEADER));
	
	saved_range = GMT->current.io.geo.range;	/* We may have to reset thisso keep a copy */
	GMT_set_geographic (GMT, GMT_OUT);	/* Output lon/lat */
	GMT->current.io.col_type[GMT_OUT][GMT_Z] = M.time_format;	
	if (Ctrl->E.active) fprintf (GMT->session.std[GMT_OUT], "#Cruise %sID      %sWest    %sEast    %sSouth   %sNorth   %sStartTime%s%sEndTime%s%s%sDist%snRec",
		GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator,
		GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator, GMT->current.setting.io_col_separator);

	use = (M.original || M.format != MGD77_FORMAT_CDF) ? MGD77_ORIG : MGD77_REVISED;
	
	for (argno = 0; argno < n_paths; argno++) {		/* Process each ID */
	
		if (MGD77_Open_File (GMT, list[argno], &M, MGD77_READ_MODE)) continue;

		GMT_Report (API, GMT_MSG_VERBOSE, "Now processing cruise %s\n", list[argno]);
		
		D = MGD77_Create_Dataset (GMT);
		
		if (read_file && MGD77_Read_File (GMT, list[argno], &M, D)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error reading header & data for cruise %s\n", list[argno]);
			Return (EXIT_FAILURE);
		}
		if (!read_file && MGD77_Read_Header_Record (GMT, list[argno], &M, &D->H)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error reading header sequence for cruise %s\n", list[argno]);
			Return (EXIT_FAILURE);
		}

		if (Ctrl->M.mode == HIST_HEADER) {	/* Dump of MGD77+ history */
			fprintf (GMT->session.std[GMT_OUT], "%s: %s", list[argno], D->H.history);
			MGD77_Close_File (GMT, &M);
			MGD77_Free_Dataset (GMT, &D);
			continue;
		}
		if (Ctrl->M.mode == E77_HEADER) {	/* Dump of e77 header status */
			if (D->H.E77 && strlen(D->H.E77) > 0)
				fprintf (GMT->session.std[GMT_OUT], "%s: %s\n", list[argno], D->H.E77);
			else
				fprintf (GMT->session.std[GMT_OUT], "%s: E77 not applied\n", list[argno]);
			MGD77_Close_File (GMT, &M);
			MGD77_Free_Dataset (GMT, &D);
			continue;
		}
		if (Ctrl->M.mode == FORMATTED_HEADER) {	/* Dump of header items, one per line */
			MGD77_Dump_Header_Params (GMT, &M, D->H.mgd77[use]);	
			MGD77_Close_File (GMT, &M);
			MGD77_Free_Dataset (GMT, &D);
			continue;
		}
		t_col = MGD77_Get_Column (GMT, "time", &M);
		x_col = MGD77_Get_Column (GMT, "lon", &M);
		y_col = MGD77_Get_Column (GMT, "lat", &M);
		id_col = MGD77_Get_Column (GMT, "id", &M);
		
		if (first && Ctrl->E.active) {	/* Output all column headers */
			for (i = k = 0; k < M.n_out_columns; i++, k++) {
				if (i == id_col || i == t_col || i == x_col || i == y_col) continue;
				fprintf (GMT->session.std[GMT_OUT],"%s%s", GMT->current.setting.io_col_separator, D->H.info[M.order[k].set].col[M.order[k].item].abbrev);
			}
			fprintf (GMT->session.std[GMT_OUT],"\n");
		}
		
		if (Ctrl->C.active) {	/* Just list names and info for any extra columns */
			for (i = k = 0, first = true; k < M.n_out_columns; i++, k++) {
				if (i == id_col || i == t_col || i == x_col || i == y_col) continue;
				if (!first) fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
				if (((Ctrl->C.mode & 1) && M.order[k].set == 0) || ((Ctrl->C.mode & 2) && M.order[k].set == 1)) {
					fprintf (GMT->session.std[GMT_OUT], "%s", D->H.info[M.order[k].set].col[M.order[k].item].abbrev);
					first = false;
				}
			}
			if (first) fprintf (GMT->session.std[GMT_OUT], "No columns matching selection found!");
			fprintf (GMT->session.std[GMT_OUT], "\n");
			MGD77_Close_File (GMT, &M);
			MGD77_Free_Dataset (GMT, &D);
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
				for (i = k = 0; k < M.n_out_columns; i++, k++) {
					if (M.order[k].set == MGD77_CDF_SET) {
						fprintf (GMT->session.std[GMT_OUT], "> %s%s%s%s%s%s%s", D->H.info[MGD77_CDF_SET].col[M.order[k].item].abbrev, GMT->current.setting.io_col_separator,
						D->H.info[MGD77_CDF_SET].col[M.order[k].item].name, GMT->current.setting.io_col_separator,
						D->H.info[MGD77_CDF_SET].col[M.order[k].item].units, GMT->current.setting.io_col_separator,
						D->H.info[MGD77_CDF_SET].col[M.order[k].item].comment);
					}
				}
			}
			fprintf (GMT->session.std[GMT_OUT], "\n");
		}
		
		tmin = tmax = GMT->session.d_NaN;
		this_dist = this_lon = this_lat = ds = this_time = 0.0;
		xmin1 = xmin2 = 360.0;
		xmax1 = xmax2 = -360.0;
		ymin = 180.0;
		ymax = -180.0;
		GMT_memset (quad, 4, bool);	/* Set all to false */
		GMT_memset (counter, MGD77_MAX_COLS, uint64_t);
	
		for (i = 0; i < MGD77_MAX_COLS; i++) {
			dvalue[i] = D->values[i];
			tvalue[i] = D->values[i];
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
			quad_no = urint (floor (this_lon/90.0));	/* Yields quadrants 0-3 */
			if (quad_no == 4) quad_no = 0;		/* When this_lon == 360.0 */
			quad[quad_no] = true;
			if (lon_w > 180.0) this_lon -= 360.0;	/* For -180/+180 range */
			xmin2 = MIN (this_lon, xmin2);
			xmax2 = MAX (this_lon, xmax2);
			if (rec > 0) {	/* Need a previous point to calculate distance, speed, and heading */
				GMT_set_delta_lon (last_lon, this_lon, dlon);
				dx = dlon * cosd (0.5 * (this_lat + last_lat));
				dy = this_lat - last_lat;
				ds = GMT->current.proj.DIST_KM_PR_DEG * hypot (dx, dy);
				this_dist += ds;
			}
			ymin = MIN (this_lat, ymin);
			ymax = MAX (this_lat, ymax);
			
			/* Count the number of non-NaN observations */
			
			for (i = k = 1; k < M.n_out_columns; i++, k++) {
				if (i == id_col || i == t_col || i == x_col || i == y_col) continue;
				if ((length = D->H.info[M.order[k].set].col[M.order[k].item].text)) {
					if (strncmp (&tvalue[k][rec*length], ALL_NINES, length)) counter[k]++;
				}
				else
					if (!GMT_is_dnan (dvalue[k][rec])) counter[k]++;
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
			GMT_Report (API, GMT_MSG_VERBOSE, "warning: cruise %s no time records.\n", M.NGDC_id);
			yy[0] = (!D->H.mgd77[use]->Survey_Departure_Year[0] || !strncmp (D->H.mgd77[use]->Survey_Departure_Year, ALL_BLANKS, 4U)) ? 0 : atoi (D->H.mgd77[use]->Survey_Departure_Year);
			yy[1] = (!D->H.mgd77[use]->Survey_Arrival_Year[0] || !strncmp (D->H.mgd77[use]->Survey_Arrival_Year, ALL_BLANKS, 4U)) ? 0 : atoi (D->H.mgd77[use]->Survey_Arrival_Year);
			mm[0] = (!D->H.mgd77[use]->Survey_Departure_Month[0] || !strncmp (D->H.mgd77[use]->Survey_Departure_Month, ALL_BLANKS, 2U)) ? 1 : atoi (D->H.mgd77[use]->Survey_Departure_Month);
			mm[1] = (!D->H.mgd77[use]->Survey_Arrival_Month[0] || !strncmp (D->H.mgd77[use]->Survey_Arrival_Month, ALL_BLANKS, 2U)) ? 1 : atoi (D->H.mgd77[use]->Survey_Arrival_Month);
			dd[0] = (!D->H.mgd77[use]->Survey_Departure_Day[0] || !strncmp (D->H.mgd77[use]->Survey_Departure_Day, ALL_BLANKS, 2U)) ? 1 : atoi (D->H.mgd77[use]->Survey_Departure_Day);
			dd[1] = (!D->H.mgd77[use]->Survey_Arrival_Day[0] || !strncmp (D->H.mgd77[use]->Survey_Arrival_Day, ALL_BLANKS, 2U)) ? 1 : atoi (D->H.mgd77[use]->Survey_Arrival_Day);
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
			fprintf (GMT->session.std[GMT_OUT], "%ld%s%" PRIu64, lrint (this_dist), GMT->current.setting.io_col_separator, D->H.n_records);
			for (i = k = 1; k < M.n_out_columns; i++, k++) {
				if (i == id_col || i == t_col || i == x_col || i == y_col) continue;
				if (((Ctrl->E.mode & 1) && M.order[k].set == 0) || ((Ctrl->E.mode & 2) && M.order[k].set == 1))
					fprintf (GMT->session.std[GMT_OUT],"%s%" PRIu64,	GMT->current.setting.io_col_separator, counter[k]);
			}
			fprintf (GMT->session.std[GMT_OUT],"\n");
		}
		MGD77_Free_Dataset (GMT, &D);
	}
		
	MGD77_Path_Free (GMT, n_paths, list);
	MGD77_end (GMT, &M);
	MGD77_end (GMT, &Out);

	Return (GMT_OK);
}
