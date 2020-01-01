/*--------------------------------------------------------------------
 *
 *    Copyright (c) 2004-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html) and Michael Chandler
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mgd77header.c reads an NGDC A77 file, determines temporal and spatial extents,
 * ten degree boxes, and data columns present. Optionally reads header items
 * from a file (-H<file).
 * Output: Header items determined from data and read from input are output in
 * H77, M77T or raw header format (-M<r|f|t>).
 *
 * Note: Program expects first row to be a header as (no header row will trigger error):
 * #rec	TZ	year	month	day	hour	min	lat		lon		ptc	twt	depth	bcc	btc	mtf1	mtf2	mag	msens	diur	msd	gobs	eot	faa	nqc	id	sln	sspn
 *
 * Author:	Michael Chandler, ported to GMT5 by P. Wessel
 * Date:	23-MAY-2012
 * Prted to GMT5 on 13-DEC-2016 by P. Wessel
 *
 */

#include "gmt_dev.h"
#include "mgd77.h"

#define THIS_MODULE_CLASSIC_NAME	"mgd77header"
#define THIS_MODULE_MODERN_NAME	"mgd77header"
#define THIS_MODULE_LIB		"mgd77"
#define THIS_MODULE_PURPOSE	"Create MGD77 headers from A77 files"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-V"

#define FORMATTED_HEADER	1
#define RAW_HEADER		2
#define M77T_HEADER		3

EXTERN_MSC int MGD77_Write_Header_Record_m77t (struct GMT_CTRL *GMT, char *file, struct MGD77_CONTROL *F, struct MGD77_HEADER *H);
EXTERN_MSC int MGD77_Get_Header_Item (struct GMT_CTRL *GMT, struct MGD77_CONTROL *F, char *item);
EXTERN_MSC int MGD77_Read_File_nohdr (struct GMT_CTRL *GMT, char *file, struct MGD77_CONTROL *F, struct MGD77_DATASET *S);

struct MGD77HEADER_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct H {	/* -H<file> */
		bool active;
		char *file;
	} H;
	struct M {	/* -Mf[<item>|r|t] */
		bool active;
		unsigned int mode;
		unsigned int flag;
	} M;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MGD77HEADER_CTRL *C = NULL;
	
	C = gmt_M_memory (GMT, NULL, 1, struct MGD77HEADER_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	C->M.mode = 1;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct MGD77HEADER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->H.file);
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	struct MGD77_CONTROL M;

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <cruise(s)>  [-H<headinfo>] [-Mf[<item>]|r|e|h] [%s] [%s]\n\n", name, GMT_V_OPT, GMT_PAR_OPT);
        
	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);
             
	MGD77_Init (API->GMT, &M);		/* Initialize MGD77 Machinery */
	MGD77_Cruise_Explain (API->GMT);
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-H Read and assign header values from a file. Each input file row gives an exact\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   header_field_name, space or tab, and header value. Values are read according to\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   NGDC's MGD77 header format specification.\n\t\te.g.,\n\t\tSource_Institution Univ. of Hawaii\n");
	GMT_Message (API, GMT_TIME_NONE, "\t\tPort_of_Arrival Honolulu, HAWAII\n\t\t...\n	   See mgd77info -Mf output for recognized header field names.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Print header items.  Append type of presentation:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f: Print header items individually, one per line.  Append name of a particular\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        item (e.g., Port_of_Departure), all [Default], or - to see a list of items.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        You can also use the number of the item.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r: Display raw original MGD77 header records [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     t: Display raw original M77T header records.\n");
	GMT_Option (API, "V,.");
	
	MGD77_end (API->GMT, &M);	/* Close machinery */

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct MGD77HEADER_CTRL *Ctrl, struct GMT_OPTION *options, struct MGD77_CONTROL *M) {
	/* This parses the options provided to mgd77info and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_opts = 0;
	int sval;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
			case '#':	/* Skip input files confused as numbers (e.g. 123456) */
				break;

			/* Processes program-specific parameters */

			case 'H':
				Ctrl->H.active = true;
				Ctrl->H.file = strdup (opt->arg);
				break;
				
			case 'M':
				Ctrl->M.active = true;
				if (opt->arg[0] == 'f') {
					Ctrl->M.mode = FORMATTED_HEADER;
					sval = MGD77_Select_Header_Item (GMT, M, &opt->arg[1]);
					if (sval < 0) n_errors++;
					Ctrl->M.flag = sval;
				}
				else if (opt->arg[0] == 'r')
					Ctrl->M.mode = RAW_HEADER;
				else if (opt->arg[0] == 't')
					Ctrl->M.mode = M77T_HEADER;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Option -M Bad modifier (%c). Use -Mf|r|t!\n", opt->arg[0]);
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	if (Ctrl->M.mode == RAW_HEADER) n_opts++;
	if (Ctrl->M.mode == M77T_HEADER) n_opts++;
	if (Ctrl->M.mode == FORMATTED_HEADER) n_opts++;

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}


#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mgd77header (void *V_API, int mode, void *args) {
	int i, id, argno, length, id_col, t_col, x_col, y_col, saved_range, use;
	int n_paths, counter[MGD77_MAX_COLS], quad_no, n_quad;
	int b_col, twt_col, g_col, m_col, f_col, mt1_col, mt2_col;
	int tendeg[36][18], tenx, teny, nten = 0, tquad;
	
	uint64_t rec;

	bool error = false;
	bool quad[4] = {false, false, false, false};

	double this_dist, this_lon, this_lat, last_lon, last_lat, dx, dy, dlon, ds, lon_w;
	double xmin, xmax, xmin1, xmin2, xmax1, xmax2, ymin, ymax, this_time, tmin, tmax;
	double *dvalue[MGD77_MAX_COLS];

	FILE *fp = NULL;

	time_t tt;
	struct tm *tod = NULL;

	char *tvalue[MGD77_MAX_COLS], buffer[BUFSIZ], name[BUFSIZ], value[BUFSIZ], params[BUFSIZ], line[BUFSIZ];
	char **list = NULL;

	struct MGD77_CONTROL M, Out;
	struct MGD77_DATASET *D = NULL;

	struct MGD77HEADER_CTRL *Ctrl = NULL;
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
	MGD77_Init (GMT, &M);		/* Initialize input MGD77 Machinery */
	MGD77_Init (GMT, &Out);		/* Initialize output MGD77 Machinery */
	if ((error = parse (GMT, Ctrl, options, &M)) != 0) Return (error);

	/*---------------------------- This is the mgd77info main code ----------------------------*/

	time (&tt);
	tod = localtime (&tt);
	if (Ctrl->H.active) {
		if ((fp = gmt_fopen (GMT, Ctrl->H.file, "r")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open header items input file (%s)\n", Ctrl->H.file);
			Return (GMT_NO_INPUT);
		}
	}

	Out.fp = GMT->session.std[GMT_OUT];

	if (Ctrl->M.mode != FORMATTED_HEADER && Ctrl->M.mode != M77T_HEADER) Ctrl->M.mode = RAW_HEADER;

	n_paths = MGD77_Path_Expand (GMT, &M, options, &list);	/* Get list of requested IDs */

	if (n_paths <= 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "No cruises given\n");
		if (fp) gmt_fclose (GMT, fp);
		Return (GMT_NO_INPUT);
	}

	saved_range = GMT->current.io.geo.range;	/* We may have to reset thisso keep a copy */
	gmt_set_geographic (GMT, GMT_OUT);	/* Output lon/lat */
	gmt_set_column (GMT, GMT_OUT, GMT_Z, M.time_format);

	use = (M.original || M.format != MGD77_FORMAT_CDF) ? MGD77_ORIG : MGD77_REVISED;

	for (argno = 0; argno < n_paths; argno++) {		/* Process each ID */

		if (MGD77_Open_File (GMT, list[argno], &M, MGD77_READ_MODE)) continue;

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Now processing cruise %s\n", list[argno]);

		D = MGD77_Create_Dataset (GMT);

		if (MGD77_Read_File_nohdr (GMT, list[argno], &M, D)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error reading data for cruise %s\n", list[argno]);
			Return (GMT_DATA_READ_ERROR);
		}

		t_col = MGD77_Get_Column	(GMT, "time", &M);
		x_col = MGD77_Get_Column	(GMT, "lon", &M);
		y_col = MGD77_Get_Column	(GMT, "lat", &M);
		b_col = MGD77_Get_Column	(GMT, "depth", &M);
		twt_col = MGD77_Get_Column	(GMT, "twt", &M);
		m_col = MGD77_Get_Column	(GMT, "mag", &M);
		mt1_col = MGD77_Get_Column	(GMT, "mtf1", &M);
		mt2_col = MGD77_Get_Column	(GMT, "mtf2", &M);
		f_col = MGD77_Get_Column	(GMT, "faa", &M);
		g_col = MGD77_Get_Column	(GMT, "gobs", &M);
		id_col = MGD77_Get_Column	(GMT, "id", &M);

		tmin = tmax = GMT->session.d_NaN;
		this_dist = this_lon = this_lat = ds = this_time = 0.0;
		xmin1 = xmin2 = 360.0;
		xmax1 = xmax2 = -360.0;
		ymin = 180.0;
		ymax = -180.0;
		gmt_M_memset (quad, 4, bool);	/* Set all to false */
		gmt_M_memset (counter, MGD77_MAX_COLS, int);

		for (i = 0; i < MGD77_MAX_COLS; i++) {
			dvalue[i] = (double *)D->values[i];
			tvalue[i] = (char *)D->values[i];
		}

		/* Set up tendeg identifier array */
		for (tenx = 0; tenx < 36; tenx++) {
			for (teny = 0; teny < 18; teny++)
				tendeg[tenx][teny] = 0;
		}

		/* Start processing data */
		for (rec = 0; rec < D->H.n_records; rec++) {		/* While able to read a data record */

			/* Get min and max time */
			if (t_col >= 0 && !gmt_M_is_dnan(dvalue[t_col][rec])) {
				if (gmt_M_is_dnan(tmin) && gmt_M_is_dnan(tmax)) tmin = tmax = dvalue[t_col][rec];
				this_time = dvalue[t_col][rec];
				tmin = MIN (this_time, tmin);
				tmax = MAX (this_time, tmax);
			}

			/* Compute accumulated distance along track (Flat Earth) */
			last_lon  = this_lon;
			last_lat  = this_lat;
			lon_w = dvalue[x_col][rec];
			this_lon = lon_w;
			this_lat  = dvalue[y_col][rec];
			if (this_lon < 0.0) this_lon += 360.0;	/* Start off with everything in 0-360 range */
			xmin1 = MIN (this_lon, xmin1);
			xmax1 = MAX (this_lon, xmax1);
			quad_no = (int)floor (this_lon/90.0);	/* Yields quadrants 0-3 */
			if (quad_no == 4) quad_no = 0;		/* When this_lon == 360.0 */
			quad[quad_no] = true;
			xmin2 = MIN (this_lon, xmin2);
			xmax2 = MAX (this_lon, xmax2);
			if (rec > 0) {	/* Need a previous point to calculate distance, speed, and heading */
				dlon = this_lon - last_lon;
				if (fabs (dlon) > 180.0) {
					dlon = copysign ((360.0 - fabs (dlon)), dlon);
				}
				dx = dlon * cosd (0.5 * (this_lat + last_lat));
				dy = this_lat - last_lat;
				ds = GMT->current.proj.DIST_KM_PR_DEG * hypot (dx, dy);
				this_dist += ds;
			}
			ymin = MIN (this_lat, ymin);
			ymax = MAX (this_lat, ymax);

			/* Determine 10x10 degree boxes crossed */
			teny = (int)(floor ((this_lat/10)+9));
			if (this_lon > 180) tenx = (int)(ceil (((this_lon-360)/10)+18));
			else tenx = (int)(floor (this_lon/10)+18);
			if (!tendeg[tenx][teny]) {
				tendeg[tenx][teny] = 1;
#if 0
 			if (this_lon > 180)
				fprintf (stderr,"lon: %.5f lat: %.5f\ttendeg[%d][%d] = %d\n",this_lon-360,this_lat,tenx,teny,tendeg[tenx][teny]);
			else
				fprintf (stderr,"lon: %.5f lat: %.5f\ttendeg[%d][%d] = %d\n",this_lon,this_lat,tenx,teny,tendeg[tenx][teny])
#endif
				nten++;
			}
			/* Count the number of non-NaN observations */
			for (i = 1; i < (int)M.n_out_columns; i++) {
				if (i == id_col || i == t_col || i == x_col || i == y_col) continue;
				if ((length = (int)D->H.info[M.order[i].set].col[M.order[i].item].text)) {
					if (strncmp(&tvalue[i][rec * length], ALL_NINES, (size_t)length))
						counter[i]++;
				}
				else
					if (!gmt_M_is_dnan (dvalue[i][rec])) counter[i]++;
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
		/* Avoid discontinuity at Dateline */
		if (xmin > 180) xmin -= 360.0;
	        if (xmax > 180) xmax -= 360.0;

		if (gmt_M_is_dnan(tmin) || gmt_M_is_dnan(tmax)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Abort: cruise %s no time records\n", M.NGDC_id);
			Return (GMT_DATA_READ_ERROR);
		}
		MGD77_Close_File (GMT, &M);

		/* Store data limits in header */
		sprintf (value,"%.0f",floor(ymin));
		sprintf (name,"Bottommost_Latitude");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);
		sprintf (value,"%.0f",ceil(ymax));
		sprintf (name,"Topmost_Latitude");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);
		sprintf (value,"%.0f",floor(xmin));
		sprintf (name,"Leftmost_Longitude");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);
		sprintf (value,"%.0f",ceil(xmax));
		sprintf (name,"Rightmost_Longitude");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);

		/* Add ten degree identifier string to header */
		sprintf (value,"%02d",(int)nten);
		sprintf (name,"Number_of_Ten_Degree_Identifiers");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);
		value[0] = '\0';
		i = 0;
		for (tenx = 0; tenx < 36; tenx++) {
			for (teny = 0; teny < 18; teny++) {
				if (tendeg[tenx][teny]) {
					i++;
					if (tenx >= 18) {
						tquad = 3;
						if (teny >= 9)
							tquad = 1;
					} else {
						tquad = 5;
						if (teny >= 9)
							tquad = 7;
					}
					if (teny > 8) snprintf (value, BUFSIZ, "%s%.1d%01.0f%02.0f,",value,tquad,fabs(teny-9.0),fabs(tenx-18.0));
					else snprintf (value, BUFSIZ, "%s%.1d%01.0f%02.0f,",value,tquad,fabs(teny-8.0),fabs(tenx-18.0));
				}
			}
		}
		snprintf (value, BUFSIZ, "%s9999", value); i++;
		while (i < 30) { /* MGD77 format can store up to this many 10x10 identifiers */
 			sprintf (value,"%s,   0",value);
			i++;
		}
		sprintf (name,"Ten_Degree_Identifier");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);

		/* Add survey identifier and file creation date */
		sprintf (value,"%.8s",list[argno]);
		sprintf (name,"Survey_Identifier");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);
		sprintf (value,"MGD77");
		sprintf (name,"Format_Acronym");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);
		sprintf (value,"11111");
		if (counter[b_col]) value[0] = '5';
		else if (counter[twt_col]) value[0] = '5';
		if (counter[m_col]) value[1] = '5';
		else if (counter[mt1_col]) value[1] = '5';
		else if (counter[mt2_col]) value[1] = '5';
		if (counter[f_col]) value[2] = '5';
		else if (counter[g_col]) value[2] = '5';
		strcpy(params,value);
		sprintf (name,"Parameters_Surveyed_Code");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);
		sprintf (value, "%d",1900+tod->tm_year);
		sprintf (name,"File_Creation_Year");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);
		sprintf (value, "%02d",1+tod->tm_mon);
		sprintf (name,"File_Creation_Month");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);
		sprintf (value, "%02d",tod->tm_mday);
		sprintf (name,"File_Creation_Day");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);
		gmt_ascii_format_one (GMT, value, tmin, GMT_IS_ABSTIME);
		sprintf (name,"Survey_Departure_Year");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,4);
		sprintf (name,"Survey_Departure_Month");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],&value[5],2);
		sprintf (name,"Survey_Departure_Day");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],&value[8],2);
		gmt_ascii_format_one (GMT, value, tmax, GMT_IS_ABSTIME);
		sprintf (name,"Survey_Arrival_Year");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,4);
		sprintf (name,"Survey_Arrival_Month");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],&value[5],2);
		sprintf (name,"Survey_Arrival_Day");
		id = MGD77_Get_Header_Item (GMT, &M, name);
		strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],&value[8],2);

		for (i = 0; i < MGD77_N_HEADER_ITEMS; i++) M.Want_Header_Item[i] = true;

		/* Copy header items from header input file */
		if (fp) {
			while (fgets (line,BUFSIZ,fp)) {
				sscanf (line, "%s %[^\t\n]", name, value);
				if (! strlen(value)) continue;
				if (params[0] == '1' && !strncmp(name,"Bathyme",7)) continue;
				if (params[1] == '1' && !strncmp(name,"Magneti",7)) continue;
				if (params[2] == '1' && !strncmp(name,"Gravity",7)) continue;
				id = MGD77_Get_Header_Item (GMT, &M, name);
				strncpy (MGD77_Header_Lookup[id].ptr[MGD77_M77_SET],value,MGD77_Header_Lookup[id].length);
			}
		}

		if (Ctrl->M.mode == FORMATTED_HEADER)	/* Dump of header items, one per line */
			MGD77_Dump_Header_Params (GMT, &M, D->H.mgd77[use]);
		else if (Ctrl->M.mode == RAW_HEADER || Ctrl->M.mode == M77T_HEADER) {	/* Write entire MGD77/M77T header */
			if (fp == NULL) {
				fprintf (GMT->session.std[GMT_OUT], "-------------------------------");
				fprintf (GMT->session.std[GMT_OUT], " Cruise: %8s ", M.NGDC_id);
				sprintf (buffer, " Cruise: %8s ", M.NGDC_id);	fprintf (GMT->session.std[GMT_OUT], "%s", buffer);
				fprintf (GMT->session.std[GMT_OUT], "-------------------------------\n");
			}
			if (Ctrl->M.mode == RAW_HEADER)
				MGD77_Write_Header_Record_m77 (GMT, "", &Out, &D->H);
			else
				MGD77_Write_Header_Record_m77t (GMT, "", &Out, &D->H);
			if (fp == NULL) {
				fprintf (GMT->session.std[GMT_OUT], "----------------------------------------");
				sprintf (buffer, "----------------------------------------\n");	fprintf (GMT->session.std[GMT_OUT], "%s", buffer);
			}
			if (fp == NULL) fprintf (GMT->session.std[GMT_OUT], "\n");
		}

		MGD77_Free_Dataset (GMT, &D);
	}

	MGD77_Path_Free (GMT, (uint64_t)n_paths, list);
	MGD77_end (GMT, &M);
	MGD77_end (GMT, &Out);
	if (fp) gmt_fclose (GMT, fp);

	Return (GMT_NOERROR);
}
