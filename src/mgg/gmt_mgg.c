/*--------------------------------------------------------------------
 *	$Id: gmt_mgg.c,v 1.22 2010-01-05 01:15:48 guru Exp $
 *
 *    Copyright (c) 1991-2010 by P. Wessel and W. H. F. Smith
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * gmt_mgg.c is a collection of functions that are used by several of
 * the mgg GMT-SYSTEM programs. These functions perform:
 *	> Converting time from seconds to dates and back again
 *	> Find right path to a gmt-file
 *
 * Author:	Paul Wessel
 * Date:	13-JUL-1988
 * Updated:	13-JUN-1991 v2.0
 *		10-MAR-1999 v3.2
 *		05-APR-1999 Now knows about DOS delimiters, too
 *		25-JUN-1999 v3.3.1 Moved MGD77 decoding function here
 *			    as well as Carter table reading.
 *
 * List of functions:
 *
 *	gmtmgg_date		- Convert seconds to gmt date.
 *	gmtmgg_init		- Initialize gmt time functions.
 *	gmtmgg_time		- Convert gmt date to seconds.
 *	gmtmggpath_init		- Get paths to gmtfile directories.
 *	gmtmggpath_func		- Return full path to a leg.
 *	gmtmgg_decode_MGD77	- Convert a MGD77 record to GMT_MGG struct
 *	carter_setup		- Initialize Carter Table lookup
 *	carter_get_bin		- Get Carter 1x1 degree bin
 *	carter_get_zone		- Get Carter zone
 *	carter_depth_from_twt	- Convert twt to depth
 *	carter_twt_from_depth	- Convert depth to twt
 */
 
#define _GMT_MGG_LIB
#include "gmt.h"	/* System dependent files */
#include "gmt_mgg.h"	/* System dependent files */
#include "carter.h"	/* Carter Tables parameters */

char *gmtmgg_path[10];	/* Max 10 directories for now */
int n_gmtmgg_paths = 0;	/* Number of these directories */
char *MGG_SHAREDIR;	/* Copy of GMT_SHAREDIR (for DLL purposes) */

BOOLEAN MGD77_first_1900 = FALSE, MGD77_first_2000 = FALSE;

/* GMT function gmtmgg_date computes the date (hr/mi/sec/dd/mm/yy) based
 * on the total time in seconds since the beginning of first_year.
 * The pointer to the GMT structure is passed allong with the other
 * arguments. The Julian day is returned. the yymmddhhmmss is passed
 * through the argument list.
 */
 
int gmtmgg_date (int time, int *year, int *month, int *day, int *hour, int *minute, int *second, struct GMTMGG_TIME *gmt_struct)
{
	int day_time, julian_day;
	day_time = time/86400;
	*month = day_time / 31 + 1;	/* Only approximately, may be smaller */

	if ((*month) < 0 || (*month) >= GMTMGG_TIME_MAXMONTH) {
		fprintf (stderr, "GMT ERROR: in gmtmgg_date: Month outside valid range [0-%d>: %d\n", GMTMGG_TIME_MAXMONTH, *month);
		GMT_exit (EXIT_FAILURE);
	}
	while (gmt_struct->daymon[*month +1] <= day_time) {
		(*month)++;
		if ((*month) < 0 || (*month) > GMTMGG_TIME_MAXMONTH) {
			fprintf (stderr, "GMT ERROR: in gmtmgg_date: Month outside valid range [0-%d>: %d\n", GMTMGG_TIME_MAXMONTH, *month);
			GMT_exit (EXIT_FAILURE);
		}
	}
	*year = (*month  - 1) / 12 + gmt_struct->first_year;
	*day = day_time - gmt_struct->daymon[*month] + 1;
	julian_day = (*month > 12) ?
		gmt_struct->daymon[*month] - gmt_struct->daymon[(*month - (*month)%12)] + *day :
		gmt_struct->daymon[*month] + *day;
	*month  = (*month-1)%12 + 1;
	time %= 86400;
	*hour = time / 3600;
	*minute = (time%3600) / 60;
	*second = time - *hour * 3600 - *minute * 60;
	return (julian_day);
}

/*
 *	GMT subroutine gmtmgg_init sets up the structure GMT  which is
 *	used by other gmt routines (gmtmgg_time,gmtmgg_date) to convert
 *	times.  Daymon[month] contains the cumulative number of
 *	days from Jan 1 in first_year through the months PRIOR to the
 *	value of month.  0 <= month <= 60.  month = 0 only occurs
 *	during initializing in this routine. The user must declare
 *	a pointer to the struct GMTMGG_TIME in the main program and pass it
 *	when calling the gmt_* functions. To define the GMT structure,
 *	include the file gmt.h
 *
 *	Paul Wessel
 *	12-JUL-1987
 *
 */
 
struct GMTMGG_TIME *gmtmgg_init (int year1)
{
	struct GMTMGG_TIME *gmt_struct;
	int dm[12];	/* No of days in each month */
	int year, this_year, month, m;
	gmt_struct = (struct GMTMGG_TIME *) GMT_memory (VNULL, (size_t)1, sizeof(struct GMTMGG_TIME), "gmtmgg_init");
	gmt_struct->first_year = year1;
	/* initialize days of the month etc. */
	dm[0] = 0;
	dm[1] = 31;
	dm[2] = 28;
	dm[3] = 31;
	dm[4] = 30;
	dm[5] = 31;
	dm[6] = 30;
	dm[7] = 31;
	dm[8] = 31;
	dm[9] = 30;
	dm[10] = 31;
	dm[11] = 30;
	gmt_struct->daymon[0] = 0;
	for (year = 0, month = 0; year < 5; year++) {
		this_year = gmt_struct->first_year + year;
		if (this_year%4 == 0 && !(this_year%400 == 0)) dm[2] = 29;
		for (m = 1; m <= 12; m++) {
			month++;
			gmt_struct->daymon[month] = gmt_struct->daymon[month - 1] + dm[m - 1];
		}
   		dm[2] = 28;
   		dm[0] = 31;
   	}
	MGD77_first_1900 = MGD77_first_2000 = FALSE;	/* Possibly new file to read */

  	return (gmt_struct);
}

/* GMT function gmtmgg_time returns the number of seconds from
 * first_year calculated from (hr/mi/sc/dd/mm/yy). The pointer
 * to the GMT structure is passed along with the arguments.
 */
 /* MODIFIED 10 July, 1987 by W. Smith  --  I killed a bug in
     month calculation */
 
int gmtmgg_time (int *time, int year, int month, int day, int hour, int minute, int second, struct GMTMGG_TIME *gmt_struct)
{
	int mon, n_days, bad = 0;
	if ((mon = (year - gmt_struct->first_year)) > 4) {
		fprintf (stderr, "gmtmgg_time:  Year - first_year > 4\n");
		return(-1);
	}
	if (month < 1 || month > 12) fprintf (stderr, "GMT WARNING: in gmtmgg_time: Month out of range [1-12]: %d\n", month), bad++;
	if (day < 1 || day > 31) fprintf (stderr, "GMT WARNING: in gmtmgg_time: Day out of range [1-31]: %d\n", day), bad++;
	if (hour < 0 || hour > 24) fprintf (stderr, "GMT WARNING: in gmtmgg_time: Hour out of range [0-24]: %d\n", hour), bad++;
	if (minute < 0 || minute > 60) fprintf (stderr, "GMT WARNING: in gmtmgg_time: Minute out of range [0-60]: %d\n", minute), bad++;
	if (second < 0 || second > 60) fprintf (stderr, "GMT WARNING: in gmtmgg_time: Second out of range [0-60]: %d\n", second), bad++;
	if (bad) return (-1);	/* When we got garbage input */
	mon = mon * 12 + month;
	n_days = gmt_struct->daymon[mon] + day - 1;
	*time = n_days * 86400 + hour * 3600 + minute * 60 + second;
	return (*time);
}
	
/* gmtmggpath_init reads the SHAREDIR/mgg/gmtfile_paths file and gets all
 * the gmtfile directories.
 */
 
void gmtmggpath_init (char *dir) {
	int i;
	char file[BUFSIZ], line[BUFSIZ];
	FILE *fp;

	sprintf (file, "%s%cmgg%cgmtfile_paths", dir, DIR_DELIM, DIR_DELIM);
	MGG_SHAREDIR = GMT_memory (VNULL, (size_t)1, (size_t)(strlen (dir)+1), "gmtmggpath_init");
	strcpy (MGG_SHAREDIR, dir);
	
	n_gmtmgg_paths = 0;

	if ((fp = fopen (file, "r")) == NULL) {
		fprintf (stderr, "GMT Warning: path file %s for *.gmt files not found\n", file);
		fprintf (stderr, "(Will only look in current directory for such files)\n");
		return;
	}
	
	while (fgets (line, BUFSIZ, fp)) {
		if (line[0] == '#') continue;	/* Comments */
		if (line[0] == ' ' || line[0] == '\0') continue;	/* Blank line */
		gmtmgg_path[n_gmtmgg_paths] = GMT_memory (VNULL, (size_t)1, (size_t)(strlen (line)), "gmtmggpath_init");
		line[strlen (line)-1] = 0;
#if _WIN32
		for (i = 0; line[i]; i++) if (line[i] == '/') line[i] = DIR_DELIM;
#else
		for (i = 0; line[i]; i++) if (line[i] == '\\') line[i] = DIR_DELIM;
#endif
		strcpy (gmtmgg_path[n_gmtmgg_paths], line);
		n_gmtmgg_paths++;
	}
	fclose (fp);
}

void gmtmgg_end ()
{
	int i;
	if (MGG_SHAREDIR) GMT_free ((void *)MGG_SHAREDIR);
	for (i = 0; i < n_gmtmgg_paths; i++) if (gmtmgg_path[i]) GMT_free ((void *)gmtmgg_path[i]);
}

/* gmtpath takes a legid as argument and returns the full path
 * to where this data file can be found.  gmtmggpath_init must be
 * called first
 */
 
int gmtmggpath_func (char *leg_path, char *leg)
{
	int id;
	char geo_path[BUFSIZ];
	
	/* First look in current directory */
	
	sprintf (geo_path, "%s.gmt", leg);
	if (!access(geo_path, R_OK)) {
		strcpy(leg_path, geo_path);
		return (0);
	}
	
	/* Then look elsewhere */
	
	for (id = 0; id < n_gmtmgg_paths; id++) {
		sprintf (geo_path, "%s%c%s.gmt", gmtmgg_path[id], DIR_DELIM, leg);
		if (!access (geo_path, R_OK)) {
			strcpy (leg_path, geo_path);
			return (0);
		}
	}
	return(1);
}
	
int gmtmgg_decode_MGD77 (char *string, int tflag, struct GMTMGG_REC *record, struct GMTMGG_TIME **gmt, int anom_offset)
{
	int year, month, day, hour, min, sec, l_mag, l_top, test, bin, zone, l_twt;
	short int twt;
	double tz, fmin;
	char s_tz[5], s_yr[5], s_mo[3], s_dy[3], s_hr[3], s_mi[6], s_lat[8], s_lon[9];
	char s_faa[5], s_mag[6], s_top[7], s_top_twt[7];
	BOOLEAN version_Y2K;

	version_Y2K = (string[0] == '5');	/* New format that is Y2K compliant */

	/* First decode Time Zone */

	if (version_Y2K) {	/* TZ is in hours */
		strncpy (s_tz, &string[10], (size_t) 2);	s_tz[2] = 0;
		tz = atof (s_tz);
	}
	else {	/* Original MGD77 format in 1/100 of hour */
		strncpy (s_tz, &string[10], (size_t) 4);	s_tz[4] = 0;
		tz = atof (s_tz) * 0.01;
	}
	if (string[9] == '-') tz = -tz;
	
	/* Then decode Time */
	
	if (!tflag) {	/* Time information provided */

		if (version_Y2K) {	/* 4-digit year */
			strncpy (s_yr, &string[12], (size_t) 4);	s_yr[4] = 0;
			year = atoi (s_yr);
		}
		else {	/* 2-digit year, apply Y2K kludge */

			/* Y2K KLUDGE FIX: Assume 2-digit year xx < 39 means 20xx, else 19xx */

			strncpy (s_yr, &string[14], (size_t) 2);	s_yr[2] = 0;
			year = atoi (s_yr);
			if (year < NGDC_OLDEST_YY) {	/* Presumably 20xx */
				if (MGD77_first_2000) fprintf (stderr, "GMT WARNING: in gmtmgg_decode_MGD77: Warning: 2-digit year %d assumed to be 20%d\n", year, year);
				year += 2000;
				MGD77_first_2000 = FALSE;
			}
			else {
				if (MGD77_first_1900) fprintf (stderr, "GMT WARNING: in gmtmgg_decode_MGD77: Warning: 2-digit year %d assumed to be 19%d\n", year, year);
				year += 1900;
				MGD77_first_1900 = FALSE;
			}
		}

		strncpy (s_mo, &string[16], (size_t) 2);	s_mo[2] = 0;
		month = atoi (s_mo);
	
		strncpy (s_dy, &string[18], (size_t) 2);	s_dy[2] = 0;
		day = atoi (s_dy);
	
		strncpy (s_hr, &string[20], (size_t) 2);	s_hr[2] = 0;
		hour = atoi (s_hr);
	
		strncpy (s_mi, &string[22], (size_t) 5);	s_mi[5] = 0;
		fmin = atof (s_mi) * 0.001;
		min = (int) floor (fmin);
		sec = irint (60.0 * (fmin - min));

		if (!(*gmt)) {	/* If not set, now is the time */
			*gmt = gmtmgg_init (year);
			fprintf (stderr, "GMT ERROR: in gmtmgg_decode_MGD77:  : No start year set, using year = %d from 1st data record\n", year);
		}
		test = gmtmgg_time (&(record->time), year, month, day, hour, min, sec, *gmt);
		if (test < 0) return (1);
		record->time += irint (tz * 3600.0);
	}
	
	/* Get lat lon, return error if outside domain */
	
	strncpy (s_lat, &string[28], (size_t) 7);	s_lat[7] = 0;
	record->lat = 10 * atoi (s_lat);
	if (string[27] == '-') record->lat = -record->lat;
	if (abs(record->lat) > 90000000) return (1);
	
	strncpy (s_lon, &string[36], (size_t) 8);	s_lon[8] = 0;
	record->lon = 10 * atoi (s_lon);
	if (string[35] == '-') record->lon = -record->lon;
	if ((record->lon) < 0) record->lon += 360000000;
	if (abs (record->lon) > 360000000) return (1);
	
	/* Get gravity */
	
	strncpy (s_faa, &string[104], (size_t) 4);	s_faa[4] = 0;
	record->gmt[0] = atoi (s_faa);
	if (record->gmt[0] == 9999 || (s_faa[0] == ' ' && s_faa[1] == ' ' && s_faa[2] == ' ' && s_faa[3] == ' '))
		record->gmt[0] = GMTMGG_NODATA;
	else if (string[103] == '-')
		record->gmt[0] = -record->gmt[0];
		
	/* Get magnetics */

	if (anom_offset) {		/* Read total field and subtract 'anom_ofset' */
		strncpy (s_mag, &string[60], (size_t) 5);	s_mag[5] = 0;
		l_mag = atoi (s_mag);
		if (l_mag == 99999 || (s_mag[0] == ' ' && s_mag[1] == ' ' && s_mag[2] == ' ' && s_mag[3] == ' ' && s_mag[4] == ' '))
			l_mag = GMTMGG_NODATA;
		if (l_mag != GMTMGG_NODATA) l_mag -= anom_offset;
	}
	else {
		strncpy (s_mag, &string[73], (size_t) 5);	s_mag[5] = 0;
		l_mag = atoi (s_mag);
		if (l_mag == 99999 || (s_mag[0] == ' ' && s_mag[1] == ' ' && s_mag[2] == ' ' && s_mag[3] == ' ' && s_mag[4] == ' '))
			l_mag = GMTMGG_NODATA;
		else if (string[72] == '-')
			l_mag = -l_mag;
		if (l_mag != GMTMGG_NODATA) l_mag = irint (0.1 * l_mag);
	}
	record->gmt[1] = l_mag;
	
	/* Get Bathymetry */
	strncpy (s_top_twt, &string[45], (size_t) 6);	s_top_twt[6] = '\0';
	l_twt = atoi(s_top_twt);
	if (l_twt == 999999 || (s_top_twt[0] == ' ' && s_top_twt[1] == ' ' && s_top_twt[2] == ' ' &&
		s_top_twt[3] == ' ' && s_top_twt[4] == ' '&& s_top_twt[5] == ' ')) {
			/* No two-way time was given.  See if there is a corrected depth:  */

		strncpy (s_top, &string[51], (size_t) 6);	s_top[6] = 0;
		l_top = atoi (s_top);
		if (l_top == 999999 || (s_top[0] == ' ' && s_top[1] == ' ' && s_top[2] == ' ' &&
			s_top[3] == ' ' && s_top[4] == ' '&& s_top[5] == ' '))
			l_top = GMTMGG_NODATA;
		else
			l_top = -irint (0.1 * l_top);
		record->gmt[2] = l_top;
	}
	else {
		twt = irint (0.1 * l_twt);
		/* Convert the twt to depth  */
		if ((carter_get_bin (record->lat, record->lon, &bin)) || (carter_get_zone (bin, &zone)) || (carter_depth_from_twt (zone, twt, &(record->gmt[2]))) ) {
			fprintf (stderr, "GMT ERROR: in gmtmgg_decode_MGD77:  ERROR in Carter correction system.\n");
			record->gmt[2] = GMTMGG_NODATA;
		}
		else {
			record->gmt[2] = -record->gmt[2];
		}
	}
	return (0);
}

/* CARTER TABLE ROUTINES */
int carter_setup (void)
{
	/* This routine must be called once before using carter table stuff.
	It reads the carter.d file and loads the appropriate arrays.
	It sets carter_not_initialized = FALSE upon successful completion
	and returns 0.  If failure occurs, it returns -1.  */

	FILE *fp = NULL;
	char buffer [BUFSIZ], *not_used = NULL;
	int  i;

	carter_not_initialized = TRUE;

	/* Read the correction table:  */

	sprintf (buffer, "%s%cmgg%ccarter.d", MGG_SHAREDIR, DIR_DELIM, DIR_DELIM);
	if ( (fp = fopen (buffer, "r")) == NULL) {
                fprintf (stderr,"carter_setup:  Cannot open r %s\n", buffer);
                return (-1);
        }

	for (i = 0; i < 4; i++) not_used = fgets (buffer, BUFSIZ, fp);	/* Skip 4 headers */
	not_used = fgets (buffer, BUFSIZ, fp);

	if ((i = atoi (buffer)) != N_CARTER_CORRECTIONS) {
		fprintf (stderr, "carter_setup:  Incorrect correction key (%d), should be %d\n", i, N_CARTER_CORRECTIONS);
                return(-1);
	}

        for (i = 0; i < N_CARTER_CORRECTIONS; i++) {
                if (!fgets (buffer, BUFSIZ, fp)) {
			fprintf (stderr, "carter_setup:  Could not read correction # %d\n", i);
			return (-1);
		}
                carter_correction[i] = atoi (buffer);
        }

	/* Read the offset table:  */

	not_used = fgets (buffer, BUFSIZ, fp);	/* Skip header */
	not_used = fgets (buffer, BUFSIZ, fp);

	if ((i = atoi (buffer)) != N_CARTER_OFFSETS) {
		fprintf (stderr, "carter_setup:  Incorrect offset key (%d), should be %d\n", i, N_CARTER_OFFSETS);
                return (-1);
	}

        for (i = 0; i < N_CARTER_OFFSETS; i++) {
                 if (!fgets (buffer, BUFSIZ, fp)) {
			fprintf (stderr, "carter_setup:  Could not read offset # %d\n", i);
			return (-1);
		}
                carter_offset[i] = atoi (buffer);
        }

	/* Read the zone table:  */

	not_used = fgets (buffer, BUFSIZ, fp);	/* Skip header */
	not_used = fgets (buffer, BUFSIZ, fp);

	if ((i = atoi (buffer)) != N_CARTER_BINS) {
		fprintf (stderr, "carter_setup:  Incorrect zone key (%d), should be %d\n", i, N_CARTER_BINS);
                return (-1);
	}

        for (i = 0; i < N_CARTER_BINS; i++) {
                 if (!fgets (buffer, BUFSIZ, fp)) {
			fprintf (stderr, "carter_setup:  Could not read offset # %d\n", i);
			return (-1);
		}
                carter_zone[i] = atoi (buffer);
        }
        fclose (fp);

	/* Get here when all is well.  */

	carter_not_initialized = FALSE;
	return (0);
}

int carter_get_bin (int lat, int lon, int *bin)
{
	/* Given signed long ints in the 1.0e06 times decimal degree range, 
	   -90000000 <= lat < 90000000, 0 <= lon < 360000000, set bin
	   number.  Returns 0 if OK, -1 if error.  */

	int latdeg, londeg;

	if (lat < -90000000 || lat > 90000000) {
		fprintf (stderr, "GMT ERROR: in carter_get_bin:  Latitude domain error (%g)\n", 1e-6 * lat);
		return (-1);
	}
	if (lon < 0 || lon > 360000000) {
		fprintf (stderr, "GMT ERROR: in carter_get_bin:  Longitude domain error (%g)\n", 1e-6 * lon);
		return (-1);
	}
	latdeg = (lat + 90000000)/1000000;
	if (latdeg == 180) latdeg = 179;	/* Map north pole to previous row  */

	londeg = lon/1000000;
	*bin = 360 * latdeg + londeg;

	return (0);
}

int carter_get_zone (int bin, int *zone)
{
	/* Sets value pointed to by zone to the Carter zone corresponding to
		the bin "bin".  Returns 0 if successful, -1 if bin out of
		range.  */

	if (carter_not_initialized && carter_setup() ) {
		fprintf (stderr, "GMT ERROR: in carter_get_zone:  Initialization failure.\n");
		return (-1);
	}

	if (bin < 0 || bin >= N_CARTER_BINS) {
		fprintf (stderr, "GMT ERROR: in carter_get_zone:  Input bin out of range [0-%d]: %d.\n", N_CARTER_BINS, bin);
		return (-1);
	}
	*zone = carter_zone[bin];
	return (0);
}


int carter_depth_from_twt (int zone, short int twt_in_msec, short int *depth_in_corr_m)
{
	/* Given two-way travel time of echosounder in milliseconds, and
		Carter Zone number, finds depth in Carter corrected meters.
		Returns (0) if OK, -1 if error condition.  */

	int	i, nominal_z1500, low_hundred, part_in_100;

	if (carter_not_initialized && carter_setup() ) {
		fprintf (stderr,"GMT ERROR: in carter_depth_from_twt:  Initialization failure.\n");
		return (-1);
	}
	if (zone < 1 || zone > N_CARTER_ZONES) {
		fprintf (stderr,"GMT ERROR: in carter_depth_from_twt:  Zone out of range [1-%d]: %d\n", N_CARTER_ZONES, zone);
		return (-1);
	}
	if (twt_in_msec < 0) {
		fprintf (stderr,"GMT ERROR: in carter_depth_from_twt:  Negative twt: %d msec\n", (int)twt_in_msec);
		return (-1);
	}

	nominal_z1500 = irint (0.75 * twt_in_msec);

	if (nominal_z1500 <= 100) {	/* There is no correction in water this shallow.  */
		*depth_in_corr_m = nominal_z1500;
		return (0);
	}

	low_hundred = nominal_z1500 / 100;
	i = carter_offset[zone-1] + low_hundred - 1;	/* -1 'cause .f indices */
	
	if (i >= (carter_offset[zone] - 1) ) {
		fprintf (stderr, "GMT ERROR: in carter_depth_from_twt:  twt too big: %d msec\n", (int)twt_in_msec);
		return (-1);
	}

	part_in_100 = nominal_z1500%100;

	if (part_in_100) {	/* We have to interpolate the table  */

		if ( i == (carter_offset[zone] - 2) ) {
			fprintf (stderr, "GMT ERROR: in carter_depth_from_twt:  twt too big: %d msec\n", (int)twt_in_msec);
			return (-1);
		}

		*depth_in_corr_m = (short) irint (carter_correction[i] +
			0.01 * part_in_100 * (carter_correction[i+1]
				- carter_correction[i]) );
		return (0);
	}
	else {
		*depth_in_corr_m = carter_correction[i];
		return (0);
	}
}


int carter_twt_from_depth (int zone, short int depth_in_corr_m, short int *twt_in_msec)
{
	/*  Given Carter zone and depth in Carter corrected meters,
	finds the two-way travel time of the echosounder in milliseconds.
	Returns -1 upon error, 0 upon success.  */

	int	min, max, guess;
	double	fraction;

	if (carter_not_initialized && carter_setup() ) {
		fprintf(stderr,"GMT ERROR: in carter_twt_from_depth:  Initialization failure.\n");
		return (-1);
	}
	if (zone < 1 || zone > N_CARTER_ZONES) {
		fprintf (stderr,"GMT ERROR: in carter_twt_from_depth:  Zone out of range [1-%d]: %d\n", N_CARTER_ZONES, zone);
		return (-1);
	}
	if (depth_in_corr_m < 0) {
		fprintf(stderr,"GMT ERROR: in carter_twt_from_depth:  Negative depth: %d m\n", (int)depth_in_corr_m);
		return(-1);
	}

	if (depth_in_corr_m <= 100) {	/* No correction applies.  */
		*twt_in_msec = (short int)irint (1.33333 * depth_in_corr_m);
		return (0);
	}

	max = carter_offset[zone] - 2;
	min = carter_offset[zone-1] - 1;

	if (depth_in_corr_m > carter_correction[max]) {
		fprintf (stderr, "GMT ERROR: in carter_twt_from_depth:  Depth too big: %d m.\n", (int)depth_in_corr_m);
		return (-1);
	}

	if (depth_in_corr_m == carter_correction[max]) {
		/* Hit last entry in table exactly  */
		*twt_in_msec = (short int)irint (133.333 * (max - min) );
		return (0);
	}


	guess = (depth_in_corr_m / 100) + min;
	if (guess > max) guess = max;
	while (guess < max && carter_correction[guess] < depth_in_corr_m) guess++;
	while (guess > min && carter_correction[guess] > depth_in_corr_m) guess--;

	if (depth_in_corr_m == carter_correction[guess]) {
		/* Hit a table value exactly  */
		*twt_in_msec = (short int)irint (133.333 * (guess - min) );
		return (0);
	}
	fraction = ((double)(depth_in_corr_m - carter_correction[guess])
			/ (double)(carter_correction[guess+1] - carter_correction[guess]));
	*twt_in_msec = (short int)irint (133.333 * (guess - min + fraction) );
	return (0);
}
