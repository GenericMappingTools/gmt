/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

struct GMT_gcal {	/* (proleptic) Gregorian calendar  */
	int year;		/* signed; negative and 0 allowed  */
	unsigned int month;	/* Always between 1 and 12  */
	unsigned int day_m;	/* Day of month; always in 1 - 31  */
	unsigned int day_y;	/* Day of year; 1 thru 366  */
	unsigned int day_w;	/* Day of week; 0 (Sun) thru 6 (Sat)  */
	int iso_y;		/* ISO year; not necessarily == year */
	unsigned int iso_w;	/* ISO week of iso_y; must be in 1 -- 53  */
	unsigned int iso_d;	/* ISO day of iso_w; uses 1 (Mon) thru 7 (Sun)  */
	unsigned int hour;	/* 00 through 23  */
	unsigned int min;	/* 00 through 59  */
	double sec;		/* 00 through 59.xxxx; leap not yet handled  */
};

struct GMT_Y2K_FIX {	/* The issue that refuses to go away... */
	unsigned int y2_cutoff;	/* The 2-digit offset year.  If y2 >= y2_cuttoff, add y100 else add y200 */
	int y100;	/* The multiple of 100 to add to the 2-digit year if we are above the time_Y2K_offset_year */
	int y200;	/* The multiple of 100 to add to the 2-digit year if we are below the time_Y2K_offset_year */
};

struct GMT_MOMENT_INTERVAL {
	struct GMT_gcal	cc[2];		
	double dt[2];		
	double sd[2];		/* Seconds since the start of the day.  */
	int64_t rd[2];
	unsigned int step;
	char unit;
};

struct GMT_TRUNCATE_TIME {		/* Used when TIME_IS_INTERVAL is not OFF */
	struct GMT_MOMENT_INTERVAL T;
	unsigned int direction;		/* 0 [+] to center on next interval, 1 [-] for previous interval */
};

#endif /* !_GMT_CALCLOCK_H */
