/*--------------------------------------------------------------------
 *	$Id: gmt_calclock.h,v 1.1 2001-08-14 15:54:49 pwessel Exp $
 *
 *	Copyright (c) 1991-2001 by P. Wessel and W. H. F. Smith
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

#include "gmt.h"

typedef int GMT_cal_rd;	/* GMT calendar "rata die" day numbers  */
typedef double GMT_dtime;	/* GMT internal time representation;
	equal to seconds elapsed since Midnight on proleptic Gregorian 
	Monday Jan 01 0001  */

#define GMT_GCAL_EPOCH 1	/* rata die fixed day number when
	proleptic Gregorian calendar is at Monday, Jan 01 0001  */

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


/* Functions whose source is in gmt_calclock.c:  */

GMT_dtime GMT_rdc2dt (GMT_cal_rd rd, double secs);

void    GMT_dt2rdc (GMT_dtime t, GMT_cal_rd *rd, double *s);
 
int     GMT_atoft (char *s, double *t);

int     GMT_read_clock (char *s, double *t);

int     GMT_read_cal (char *s, GMT_cal_rd *rd);

double	GMT_cal_dmod (double x, double y);

int	GMT_cal_imod (int x, int y);

GMT_cal_rd GMT_kday_on_or_before (GMT_cal_rd date, int kday);

GMT_cal_rd GMT_kday_on_or_after (GMT_cal_rd date, int kday);

GMT_cal_rd GMT_kday_after (GMT_cal_rd date, int kday);

GMT_cal_rd GMT_kday_before (GMT_cal_rd date, int kday);

GMT_cal_rd GMT_kday_nearest (GMT_cal_rd date, int kday);

GMT_cal_rd GMT_nth_kday (int n, int kday, GMT_cal_rd date);

BOOLEAN	GMT_is_gleap (int gyear);


double GMT_cal_mod (double x, double y);

GMT_cal_rd GMT_rd_from_gymd (int gy, int gm, int gd);

int GMT_gyear_from_rd (GMT_cal_rd date);

GMT_cal_rd GMT_rd_from_iywd (int iy, int iw, int id);

void GMT_gcal_from_rd ( GMT_cal_rd date, struct GMT_gcal *gcal);
