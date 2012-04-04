/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/* Header file for Generic Mapping Tools conversions
 *      between [calendar, clock] and time.
 *      
 *      Calendar conversions are inspired partly by
 *      Dershowitz and Reingold.
 *      
 * Author:	W H F Smith
 * Date:	1 JAN 2010
 * Version:	5 API
 *
 */

#pragma once
#ifndef _GMT_CALCLOCK_H
#define _GMT_CALCLOCK_H

#define GMT_CALSTRING_LENGTH	16	/* All strings used to format date/clock output must be this length */

struct GMT_gcal {	/* (proleptic) Gregorian calendar  */
	GMT_LONG year;	/* signed; negative and 0 allowed  */
	GMT_LONG month;	/* Always between 1 and 12  */
	GMT_LONG day_m;	/* Day of month; always in 1 - 31  */
	GMT_LONG day_y;	/* Day of year; 1 thru 366  */
	GMT_LONG day_w;	/* Day of week; 0 (Sun) thru 6 (Sat)  */
	GMT_LONG iso_y;	/* ISO year; not necessarily == year */
	GMT_LONG iso_w;	/* ISO week of iso_y; must be in 1 -- 53  */
	GMT_LONG iso_d;	/* ISO day of iso_w; uses 1 (Mon) thru 7 (Sun)  */
	GMT_LONG hour;	/* 00 through 23  */
	GMT_LONG min;	/* 00 through 59  */
	double sec;	/* 00 through 59.xxxx; leap not yet handled  */
};

struct GMT_Y2K_FIX {	/* The issue that refuses to go away... */
	GMT_LONG y2_cutoff;	/* The 2-digit offset year.  If y2 >= y2_cuttoff, add y100 else add y200 */
	GMT_LONG y100;	/* The multiple of 100 to add to the 2-digit year if we are above the time_Y2K_offset_year */
	GMT_LONG y200;	/* The multiple of 100 to add to the 2-digit year if we are below the time_Y2K_offset_year */
};

struct GMT_MOMENT_INTERVAL {
	struct GMT_gcal	cc[2];		
	double dt[2];		
	double sd[2];		/* Seconds since the start of the day.  */
	GMT_LONG rd[2];
	GMT_LONG itype;
	GMT_LONG step;
	char unit;
};

struct GMT_TRUNCATE_TIME {		/* Used when TIME_IS_INTERVAL is not OFF */
	struct GMT_MOMENT_INTERVAL T;
	GMT_LONG direction;			/* 0 [+] to center on next interval, 1 [-] for previous interval */
};

#endif /* !_GMT_CALCLOCK_H */
