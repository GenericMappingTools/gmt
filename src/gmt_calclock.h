/*--------------------------------------------------------------------
 *	$Id: gmt_calclock.h,v 1.28 2009-01-09 04:02:32 guru Exp $
 *
 *	Copyright (c) 1991-2009 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/* Header file for Generic Mapping Tools conversions
	between [calendar, clock] and time.
	
	Calendar conversions are inspired partly by
	Dershowitz and Reingold.
	
	W H F Smith, 5 April 2000

*/

/* GMT calendar "rata die" day numbers  */
#ifdef _WIN64
typedef __int64 GMT_cal_rd;
#else
typedef long GMT_cal_rd;
#endif

#define GMT_CALSTRING_LENGTH	16	/* All strings used to format date/clock output must be this length */

struct GMT_gcal {	/* (proleptic) Gregorian calendar  */
	int	year;	/* signed; negative and 0 allowed  */
	int	month;	/* Always between 1 and 12  */
	int	day_m;	/* Day of month; always in 1 - 31  */
	int	day_y;	/* Day of year; 1 thru 366  */
	int	day_w;	/* Day of week; 0 (Sun) thru 6 (Sat)  */
	int	iso_y;	/* ISO year; not necessarily == year */
	int	iso_w;	/* ISO week of iso_y; must be in 1 -- 53  */
	int	iso_d;	/* ISO day of iso_w; uses 1 (Mon) thru 7 (Sun)  */
	int	hour;	/* 00 through 23  */
	int	min;	/* 00 through 59  */
	double	sec;	/* 00 through 59.xxxx; leap not yet handled  */
};

struct GMT_Y2K_FIX {	/* The issue that refuses to go away... */
	int y2_cutoff;	/* The 2-digit offset year.  If y2 >= y2_cuttoff, add y100 else add y200 */
	int y100;	/* The multiple of 100 to add to the 2-digit year if we are above the Y2K_offset_year */
	int y200;	/* The multiple of 100 to add to the 2-digit year if we are below the Y2K_offset_year */
};

struct GMT_MOMENT_INTERVAL {
	struct GMT_gcal	cc[2];		
	double		dt[2];		
	double		sd[2];		/* Seconds since the start of the day.  */
	GMT_cal_rd	rd[2];
	int		itype;
	int		step;
	char		unit;
};

struct GMT_TRUNCATE_TIME {		/* Used when TIME_IS_INTERVAL is not OFF */
	struct GMT_MOMENT_INTERVAL T;
	int direction;			/* 0 [+] to center on next interval, 1 [-] for previous interval */
};

/* Functions whose source is in gmt_calclock.c:  */

EXTERN_MSC double GMT_rdc2dt (GMT_cal_rd rd, double secs);
EXTERN_MSC void GMT_dt2rdc (double t, GMT_cal_rd *rd, double *s);
EXTERN_MSC int GMT_cal_imod (int x, int y);
EXTERN_MSC GMT_cal_rd GMT_kday_on_or_before (GMT_cal_rd date, int kday);
EXTERN_MSC GMT_cal_rd GMT_kday_after (GMT_cal_rd date, int kday);
EXTERN_MSC GMT_cal_rd GMT_kday_before (GMT_cal_rd date, int kday);
EXTERN_MSC GMT_cal_rd GMT_nth_kday (int n, int kday, GMT_cal_rd date);
EXTERN_MSC BOOLEAN GMT_is_gleap (int gyear);
EXTERN_MSC double GMT_cal_mod (double x, double y);
EXTERN_MSC GMT_cal_rd GMT_rd_from_gymd (int gy, int gm, int gd);
EXTERN_MSC int GMT_gyear_from_rd (GMT_cal_rd date);
EXTERN_MSC GMT_cal_rd GMT_rd_from_iywd (int iy, int iw, int id);
EXTERN_MSC void GMT_gcal_from_rd ( GMT_cal_rd date, struct GMT_gcal *gcal);
EXTERN_MSC int	GMT_y2_to_y4_yearfix (int y2);	/* Convert a 2-digit year to a 4-digit year */
EXTERN_MSC BOOLEAN GMT_iso_ywd_is_bad (int y, int w, int d);	/* Check range of week and day for ISO W calendar.  */
EXTERN_MSC BOOLEAN GMT_g_ymd_is_bad (int y, int m, int d);	/* Check range of month and day for Gregorian YMD calendar values  */
EXTERN_MSC BOOLEAN GMT_hms_is_bad (int h, int m, double s);	/* Check range of hours, min, and secs */
EXTERN_MSC void	GMT_gcal_from_dt (double t, struct GMT_gcal *cal);	/* Break internal time into calendar and clock struct info  */
EXTERN_MSC struct GMT_Y2K_FIX GMT_Y2K_fix;	/* Structure holding Y2K parameters */
EXTERN_MSC int GMT_verify_time_step (int step, char unit);	/* Check that time step and unit for time axis are OK  */
EXTERN_MSC void GMT_moment_interval (struct GMT_MOMENT_INTERVAL *p, double dt_in, BOOLEAN init); /* step a time axis by time units */
EXTERN_MSC int GMT_gmonth_length (int year,  int month);	/* Get the number of days in a month by Gregorian leap rule */
EXTERN_MSC void GMT_small_moment_interval (struct GMT_MOMENT_INTERVAL *p, int step_secs, BOOLEAN init); /* Aux to GMT_moment_interval */
EXTERN_MSC void GMT_format_calendar (char *date, char *clock, struct GMT_DATE_IO *D, struct GMT_CLOCK_IO *C, BOOLEAN upper, int kind, double dt);	/* Write formatted strings for date and clock */
EXTERN_MSC void GMT_get_time_label (char *string, struct GMT_PLOT_CALCLOCK *P, struct GMT_PLOT_AXIS_ITEM *T, double t);

#ifdef USE_UNUSED_GMT_FUNCTIONS
EXTERN_MSC int     GMT_read_clock (char *s, double *t);
EXTERN_MSC int     GMT_read_cal (char *s, GMT_cal_rd *rd);
EXTERN_MSC double	GMT_cal_dmod (double x, double y);
EXTERN_MSC GMT_cal_rd GMT_kday_on_or_after (GMT_cal_rd date, int kday);
EXTERN_MSC GMT_cal_rd GMT_kday_nearest (GMT_cal_rd date, int kday);
#endif
