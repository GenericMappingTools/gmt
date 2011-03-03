/*--------------------------------------------------------------------
 *	$Id: gmt_calclock.c,v 1.74 2011-03-03 21:02:50 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/* Routines for Generic Mapping Tools conversions
	between [calendar, clock] and time.
	
	Calendar conversions are inspired partly by
	Dershowitz and Reingold.  We use "rata die"
	integer day numbers with day 1 set at
	proleptic Gregorian Mon Jan  1 00:00:00 0001.
	Proleptic means we assume that modern calendar
	can be extrapolated forward and backward; a
	year zero is used, and Gregory's reforms after
	Oct 1582 are extrapolated backward.  Note
	that this is not historical.
	
	W H F Smith, April 2000

*/

#define GMT_WITH_NO_PS
#include "gmt.h"

GMT_LONG want_iso = FALSE;	/* Temporary to test compilation.  Delete this.  */

/* Functions to assemble/disassemble a continuous
variable (double) and a calendar day (GMT_cal_rd)
and time of day (double secs):  
*/

double GMT_rdc2dt (GMT_cal_rd rd, double secs) {
/*	Given rata die rd and double seconds, return 
	time in TIME_UNIT relative to chosen TIME_EPOCH  */
	double f_days;
	f_days = (rd - gmtdefs.time_system.rata_die - gmtdefs.time_system.epoch_t0);
	return ((f_days * GMT_DAY2SEC_F  + secs) * gmtdefs.time_system.i_scale);
}

GMT_LONG	splitinteger(double value, GMT_LONG epsilon, double *doublepart) {
	/* Split value into integer and and floating part for date usage.
	   While the integer can be negative, doublepart is always >= 0
	   When doublepart is close to 0 or close to epsilon, doublepart is set
	   to zero and in the latter case, splitinteger is raised by one.
	   This makes value "snap" to multiples of epsilon.
	*/
	GMT_LONG i;

	i = (GMT_LONG) floor(value/(double)epsilon);
	*doublepart = value - ((double)i)*((double)epsilon);
	if ((*doublepart) < GMT_SMALL)
		*doublepart = 0.0;	/* Snap to the lower integer */
	else if ((double)epsilon - (*doublepart) < GMT_SMALL) {
		i++;			/* Snap to the higher integer */
		*doublepart = 0.0;
	}
	return i;
}

void	GMT_dt2rdc (double t, GMT_cal_rd *rd, double *s) {

	GMT_LONG i;

/*	Given time in TIME_UNIT relative to TIME_EPOCH, load rata die of this day
	in rd and the seconds since the start of that day in s.  */
	double t_sec;
	t_sec = (t * gmtdefs.time_system.scale + gmtdefs.time_system.epoch_t0 * GMT_DAY2SEC_F);
	i = splitinteger(t_sec, 86400, s) + gmtdefs.time_system.rata_die;
	*rd = (GMT_cal_rd)(i);
}


/* Modulo functions.  The C operation "x%y" and the POSIX 
	fmod(x,y) will return a negative result when x < 0
	and y > 0.  The routines here below will always
	return a positive value when y > 0, a necessary
	feature in the calendar calculations.
	
	It is not clear what to do when y == 0.  Clearly,
	the result is not defined.  We should probably
	print a domain error and return something anyway.
*/

GMT_LONG	GMT_cal_imod (GMT_LONG x, GMT_LONG y) {
	if (y == 0) {
		fprintf (stderr, "GMT_cal_imod:  DOMAIN ERROR.\n");
		return (x);
	}
	return (x - y * ( (GMT_LONG)floor((double)x/(double)y) ) );
}

/* kday functions:
	Let kday be the day of the week, defined with 0 = Sun,
	1 = Mon, etc. through 6 = Sat.  (Note that ISO day of
	the week is the same for all of these except ISO Sunday
	is 7.)  Since rata die 1 is a Monday, we have kday from
	rd is simply GMT_cal_imod(rd, 7).  The various functions
	below take an rd and find another rd related to the first
	through the fact that the related day falls on a given
	kday of the week.  */

GMT_cal_rd GMT_kday_on_or_before (GMT_cal_rd date, GMT_LONG kday) {
/*	Given date and kday, return the date of the nearest kday
	on or before the given date.
*/
	return ((GMT_cal_rd)(date - GMT_cal_imod((GMT_LONG)(date-kday), 7)));
}

GMT_cal_rd GMT_kday_after (GMT_cal_rd date, GMT_LONG kday) {
/*	Given date and kday, return the date of the nearest kday
	after the given date.
*/
	return (GMT_kday_on_or_before(date+7, kday));
}

GMT_cal_rd GMT_kday_before (GMT_cal_rd date, GMT_LONG kday) {
/*	Given date and kday, return the date of the nearest kday
	before the given date.
*/
	return (GMT_kday_on_or_before(date-1, kday));
}

GMT_cal_rd GMT_nth_kday (GMT_LONG n, GMT_LONG kday, GMT_cal_rd date) {
/*	Given date, kday, and n, return the date of the n'th
	kday before or after the given date, according to the
	sign of n.
*/
	if (n > 0) {
		return ( (GMT_cal_rd) (7*n + GMT_kday_before(date, kday)));
	}
	else {
		return ( (GMT_cal_rd) (7*n + GMT_kday_after (date, kday)));
	}
}


/* Proleptic Gregorian Calendar operations  */

GMT_LONG	GMT_is_gleap (GMT_LONG gyear) {
/*	Given integer proleptic gregorian calendar year,
	return TRUE if it is a Gregorian leap year; 
	else return FALSE.  */
	
	GMT_LONG	y400;
	
	if (GMT_cal_imod (gyear, 4) != 0) return (FALSE);
	
	y400 = GMT_cal_imod (gyear, 400);
	
	if (y400 == 0) return (TRUE);
	
	if (GMT_cal_imod (y400, 100) == 0) return (FALSE);
	
	return (TRUE);
}

GMT_cal_rd GMT_rd_from_gymd (GMT_LONG gy, GMT_LONG gm, GMT_LONG gd) {
/*	Given gregorian calendar year, month, day of month, 
	return the rata die integer day number.  */
	
	double s;
	GMT_LONG	day_offset, yearm1, rd;
	
	
	if (gm <= 2) {
		day_offset = 0;
	}
	else {
		if ( GMT_is_gleap (gy) ) {
			day_offset = -1;
		}
		else {
			day_offset = -2;
		}
	}
	
	yearm1 = gy - 1;
	rd = day_offset + gd + 365 * yearm1;
	s = floor(yearm1/4.0) - floor(yearm1/100.0) + floor(yearm1/400.0);
	s += floor( (367 * gm - 362)/12.0);
	rd += (GMT_LONG)irint(s);
	return ((GMT_cal_rd)rd);
}

GMT_LONG	GMT_gyear_from_rd (GMT_cal_rd date) {
/*	Given rata die integer day number, return proleptic Gregorian year  */

	GMT_LONG d0, d1, d2, d3, n400, n100, n4, n1, year;
	
	d0 = (GMT_LONG)date - 1;
	n400 = (GMT_LONG) floor (d0 / 146097.0);
	d1 = GMT_cal_imod (d0, 146097);
	n100 = (GMT_LONG) floor (d1 / 36524.0);
	d2 = GMT_cal_imod (d1, 36524);
	n4 = (GMT_LONG)floor (d2 / 1461.0);
	d3 = GMT_cal_imod (d2, 1461);
	n1 = (GMT_LONG)floor (d3 / 365.0);
	/* d4 = GMT_cal_imod (d3, 365) + 1; NOT USED (removed) */
	year = 400*n400 + 100*n100 + 4*n4 + n1;
	
	if (n100 != 4 && n1 != 4) year++;
	
	return (year);
}



/* ISO calendar routine  */

GMT_cal_rd GMT_rd_from_iywd (GMT_LONG iy, GMT_LONG iw, GMT_LONG id) {
/*	Given ISO calendar year, week, day of week, 
	return the rata die integer day number.  */
	
	GMT_cal_rd	rdtemp;
	
	/* Add id to the iw'th Sunday after Dec 28 iy-1:  */
	rdtemp = GMT_rd_from_gymd (iy-1, 12, 28);
	return ((GMT_cal_rd) (id + GMT_nth_kday(iw, 0, rdtemp)));
}


/* Set calendar struct data from fixed date:  */

void GMT_gcal_from_rd (GMT_cal_rd date, struct GMT_gcal *gcal) {
/*	Given rata die integer day number, load calendar structure
	with proleptic Gregorian and ISO calendar values.  */
	
	GMT_cal_rd	prior_days, corexn, tempdate;
	GMT_LONG	tempyear;
	
	/* Day of the week in 0 thru 6:  */
	
	gcal->day_w = GMT_cal_imod ((GMT_LONG)date, 7);
	
	/* proleptic Gregorian operations:  */

	gcal->year = GMT_gyear_from_rd (date);
	
	prior_days = date - GMT_rd_from_gymd (gcal->year, 1, 1);

	gcal->day_y = (GMT_LONG)prior_days + 1;
	
	tempdate = GMT_rd_from_gymd (gcal->year, 3, 1);
	
	if (date < tempdate) {
		corexn = 0;
	}
	else {
		if ( GMT_is_gleap (gcal->year) ) {
			corexn = 1;
		}
		else {
			corexn = 2;
		}
	}
	
	gcal->month = (GMT_LONG) floor( (12*(prior_days + corexn) + 373)/367.0);
	
	tempdate = GMT_rd_from_gymd (gcal->year, gcal->month, 1);
	
	gcal->day_m = (GMT_LONG)(date - tempdate) + 1;
	
	/* ISO operations:  */
	
	tempyear = (prior_days >= 3) ? gcal->year : gcal->year - 1;
	tempdate = GMT_rd_from_iywd (tempyear+1, 1, 1);
	gcal->iso_y = (date >= tempdate) ? tempyear + 1 : tempyear;
	gcal->iso_w = 1 + (GMT_LONG)floor((date - GMT_rd_from_iywd (gcal->iso_y, 1, 1))/7.0);
	gcal->iso_d = (gcal->day_w) ? gcal->day_w : 7;
}

GMT_LONG	GMT_y2_to_y4_yearfix (GMT_LONG y2) {

	/* Convert 2-digit year to 4-digit year, using 
		gmtdefs.Y2K_offset_year.
		
		The sense of Y2K_offset_year is that it
		is the first year representable in the
		2-digit set.  For example, if the set
		runs from 1950 to 2049, then Y2K_offset_year
		should be given as 1950, and then if a
		two-digit year is from 50 to 99 it will
		return 1900 plus that amount.  If it is
		from 00 to 49, it will return 2000 plus
		that amount.
		
		Below, we do a modulo operation and some
		add/subtract to compute y100, y200, fraction
		from gmtdefs.Y2K_offset_year.  Because these
		are constants during any run of a GMT program,
		they could be computed once in gmt_init, but
		we do them over and over again here, to make
		this routine deliberately slow.  I am writing
		the time code in August 2001, and people who
		haven't fixed their Y2K bug data by now will
		be punished.
		*/

	/* The GMT_Y2K_fix structure is initialized in GMT_io_init once  */
		
	return (y2 + ((y2 >= GMT_Y2K_fix.y2_cutoff) ? GMT_Y2K_fix.y100 : GMT_Y2K_fix.y200));
}

GMT_LONG	GMT_g_ymd_is_bad (GMT_LONG y, GMT_LONG m, GMT_LONG d) {

	/* Check year, month, day values to see if they
		are an appropriate date in the proleptic
		Gregorian calendar.  Returns TRUE if it
		thinks the month and/or day have bad
		values.  Returns FALSE if this looks like
		a valid calendar date.  */
	
	GMT_LONG	k;
	
	
	if (m < 1 || m > 12 || d < 1) return (TRUE);
	
	k = GMT_gmonth_length (y, m);	/* Number of day in the specified month */

	if (d > k) return (TRUE);	/* More days than we've got in this month */
	
	return (FALSE);
}


GMT_LONG	GMT_iso_ywd_is_bad (GMT_LONG y, GMT_LONG w, GMT_LONG d) {

	/* Check ISO_year, ISO_week_of_year, ISO_day_of_week
		values to see if they form a probably
		appropriate date in the ISO calendar based
		on weeks.  This is only a gross error check;
		I don't verify that a particular date actually
		exists on the ISO calendar.
		Returns TRUE if it appears something is out
		of range, including negative year.
		Returns FALSE if it looks like things are OK.
	*/
	
	if (y < 0 || w < 1 || w > 53 || d < 1 || d > 7) return (TRUE);
	
	/* Later, insert something smarter here.  */
	
	return (FALSE);
}

GMT_LONG	GMT_hms_is_bad (GMT_LONG h, GMT_LONG m, double s) {

	/* Check range of hours, min, and seconds.
		Returns TRUE if it appears something is out of range.
		Returns FALSE if it looks like things are OK.
	*/
	
	if (h < 0 || h > 23 || m < 0 || m > 59 || s < 0.0 || s >= 61.0) return (TRUE);
	
	return (FALSE);
}

void	GMT_gcal_from_dt (double t, struct GMT_gcal *cal) {

	/* Given time in internal units, load cal and clock info
		in cal.
		Note: uses 0 through 23 for hours (no am/pm inside here).
		Note: does not yet deal w/ leap seconds; modulo math here.
	*/
	
	GMT_cal_rd rd;
	double	x;
	GMT_LONG i;

	GMT_dt2rdc (t, &rd, &x);
	GMT_gcal_from_rd (rd, cal);
	/* split double seconds and integer time */
	i = splitinteger(x, 60, &cal->sec);
	cal->hour = i/60;
	cal->min  = i%60;
	return;
}
	
	

GMT_LONG	GMT_verify_time_step (GMT_LONG step, char unit) {

	/* Return -1 if time step and/or unit look bad, 0 if OK.  */
	
	GMT_LONG	retval = 0;
	
	if (step < 0) {
		fprintf (stderr, "GMT SYNTAX ERROR:  time steps must be positive.\n");
		return (-1);
	}

	switch (unit) {
		case 'c':
		case 'C':
			if (step > 60) {
				fprintf (stderr, "GMT SYNTAX ERROR:  time steps in seconds must be <= 60\n");
				retval = -1;
			}
			break;
		case 'm':
		case 'M':
			if (step > 60) {
				fprintf (stderr, "GMT SYNTAX ERROR:  time steps in minutes must be <= 60\n");
				retval = -1;
			}
			break;
		case 'h':
		case 'H':
			if (step > 24) {
				fprintf (stderr, "GMT SYNTAX ERROR:  time steps in hours must be <= 24\n");
				retval = -1;
			}
			break;
		case 'R':	/* Special Gregorian days: Annotate from start of each week and not first day of month */
			/* We are leveraging the machinery for 'K' and 'k' to step along but reset to start of week */
		case 'd':
		case 'D':
			/* The letter d is used for both days of the month and days of the (gregorian) year */
			if (GMT_plot_calclock.date.day_of_year) {
				if (step > 365) {	/* This is probably an error.  */
					fprintf (stderr, "GMT SYNTAX ERROR:  time steps in year days must be <= 365\n");
					retval = -1;
				}
			}
			else {
				/* If step is longer than 31 it is probably an error. */
				if (step > 31) {
					fprintf (stderr, "GMT SYNTAX ERROR:  time steps in days of the month must be <= 31\n");
					retval = -1;
				}
			}
			break;
		case 'k':
		case 'K':
			if (step > 7) {
				fprintf (stderr, "GMT SYNTAX ERROR:  time steps in weekdays must be <= 7\n");
				retval = -1;
			}
			break;
		case 'r':	/* Gregorian week.  Special case:  since weeks aren't numbered on Gregorian
					calendar, we only allow step size = 1 here, for ticking each week start. */
			if (step != 1) {
				fprintf (stderr, "GMT SYNTAX ERROR:  time step must be 1 for Gregorian weeks\n");
				retval = -1;
			}
			break;
		case 'u':	/* ISO week */
		case 'U':
			if (step > 52) {
				fprintf (stderr, "GMT SYNTAX ERROR:  time steps in weeks must be <= 52\n");
				retval = -1;
			}
			break;
		case 'o':
		case 'O':
			if (step > 12) {
				fprintf (stderr, "GMT SYNTAX ERROR:  time steps in months must be <= 12\n");
				retval = -1;
			}
			break;
		case 'y':	/* No check on years */
		case 'Y':
			break;
		case 'l':	/* Pass-through for log10 and pow flags (they are not units) */
		case 'p':
			break;
		default:
			fprintf (stderr, "GMT SYNTAX ERROR:  Unrecognized time axis unit.\n");
			retval = -1;
			break;
	}
	return (retval);
}


void	GMT_moment_interval (struct GMT_MOMENT_INTERVAL *p, double dt_in, GMT_LONG init) {

	/*   
	
	Unchanged by this routine:
		p->step is a positive interval width;
		p->unit is set to a time axis unit;
		These must be in valid ranges tested by GMT_verify_time_step().
		p->init sets action to take; see below.
	
	Let a and b be points in time, both exactly on the start of intervals
		defined by p->step and p->unit (e.g. 6 hours, 3 months), and
		such that b > a and b is the start of the next interval after a.
	
	Let cc[0], dt[0], sd[0], rd[0] contain representations of time a.
	Let cc[1], dt[1], sd[1], rd[1] contain representations of time b.
	
	if (init) {
		dt_in must contain a GMT interval time;
		a and b will be found and set such that a <= dt_in < b;
	}
	else {
		dt_in is not used;
		b is copied to a;
		the next b is found;
	}
	
	Warning:  Current operation of GMT_gcal_from_rd() only sets the
	calendar components of struct GMT_gcal.  That's OK for this
	routine, as clock components are not used here.  However, before
	plotting the clock corresponding to a particular time, one should
	call a routine to break dt or sd down into a clock string, or call
	a routine to set the clock components of struct GMT_gcal.
		
	*/
	
	GMT_LONG	k, kws, kml, kyd;


	if (init) {
		/* Temporarily store a breakdown of dt_in in p->stuff[0].
			Below we will take floor of this to get time a,
			and reload stuff[0] with time a.  */
		GMT_dt2rdc (dt_in, &(p->rd[0]), &(p->sd[0]) );
		GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
		p->dt[0] = dt_in;
		p->sd[1] = p->sd[0];
		p->rd[1] = p->rd[0];
		memcpy ( (void *)&(p->cc[1]), (void *)&(p->cc[0]), sizeof (struct GMT_gcal));	/* Set to same as first calendar */
	}
	else {
		memcpy ( (void *)&(p->cc[0]), (void *)&(p->cc[1]), sizeof (struct GMT_gcal));
		p->dt[0] = p->dt[1];
		p->sd[0] = p->sd[1];
		p->rd[0] = p->rd[1];
	}
	
	
	
	switch (p->unit) {
		case 'c':
		case 'C':
			k = p->step;
			GMT_small_moment_interval (p, k, init);
			break;
		case 'm':
		case 'M':
			k = GMT_MIN2SEC_I * p->step;
			GMT_small_moment_interval (p, k, init);
			break;
		case 'h':
		case 'H':
			k = GMT_HR2SEC_I * p->step;
			GMT_small_moment_interval (p, k, init);
			break;
		case 'd':
		case 'D':
			if (p->step == 1) {
				/* Here we want every day (of the Gregorian month or year)
					so the stepping is easy.  */
				k = GMT_DAY2SEC_I;
				GMT_small_moment_interval (p, k, init);
			}
			else if (GMT_plot_calclock.date.day_of_year) {
				/* Select every n'th day of the Gregorian year  */
				if (init) {
					/* Simple mod works on positive ints  */
					/* k = (p->cc[0].day_y - 1)%(p->step); */
					k = (p->cc[0].day_y)%(p->step);		/* Want to start on a multiple of step */
					if (k) {
						p->rd[0] -= k;	/* Floor to n'th day  */
						GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
						memcpy ( (void *)&(p->cc[1]), (void *)&(p->cc[0]), sizeof (struct GMT_gcal));	/* Set to same as first calendar */
					}
					p->sd[0] = 0.0;
					p->dt[0] = GMT_rdc2dt (p->rd[0], p->sd[0]);
				}
				kyd = (GMT_is_gleap (p->cc[0].year)) ? 366 : 365;
				k = p->cc[0].day_y + p->step;
				if (k > kyd) {
					/* Go to 1st day of next year:  */
					/* p->rd[1] = GMT_rd_from_gymd (p->cc[0].year+1, 1, 1); */
					p->rd[1] = GMT_rd_from_gymd (p->cc[0].year+1, 1, 1) - 1 + p->step;	/* So jjj will be multiples of step */
				}
				else {
					p->rd[1] = p->rd[0] + p->step;
				}
				GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
				p->sd[1] = 0.0;
				p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
			}
			else {
				/* Select every n'th day of the Gregorian month  */
				if (init) {
					/* Simple mod works on positive ints  */
					k = (p->cc[0].day_m - 1)%(p->step);
					if (k) {
						p->rd[0] -= k;	/* Floor to n'th day  */
						GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
						memcpy ( (void *)&(p->cc[1]), (void *)&(p->cc[0]), sizeof (struct GMT_gcal));	/* Set to same as first calendar */
					}
					p->sd[0] = 0.0;
					p->dt[0] = GMT_rdc2dt (p->rd[0], p->sd[0]);
				}
				
				kml = GMT_gmonth_length (p->cc[0].year, p->cc[0].month);
				if (p->cc[0].day_m + p->step > kml) {
					/* Truncate to 1st of next month  */
					if (p->cc[0].month == 12) {
						p->cc[1].year = p->cc[0].year + 1;
						p->cc[1].month = 1;
					}
					else {
						p->cc[1].month = p->cc[0].month + 1;
					}
					p->rd[1] = GMT_rd_from_gymd (p->cc[1].year, p->cc[1].month, 1);
				}
				else {
					/* Adding step days will stay in current month.  */
					p->rd[1] = p->rd[0] + p->step;
				}
				p->sd[1] = 0.0;
				GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
				p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
			}
			break;
		
		case 'k':
		case 'K':
		case 'R':	/* Special Gregorian Days of the Month: Annotations start at start of week, not start of month */
			/* Here we need to know: how do you define the n'th day 
			of the week?  I answered this question (for now) numbering
			days as 0=Sun through 6=Sat, (this matches kday routines)
			assuming this numbering applies to gmtdefs.time_week_start, 
			and placing the base day at either 1=Monday for ISO calendar, 
			or time_week_start for Gregorian calendar.  User can then have 
			base, base+n, base+2n, etc. until base+kn would equal day 7 
			or more. When that happens, we truncate to start of next week.
			*/
			kws = (GMT_plot_calclock.date.iso_calendar) ? 1 : gmtdefs.time_week_start;
			if (init) {
				/* Floor to the n'th day of the week from the week start:  */
				/* a simple mod will work here since both are positive ints  */
				k = (p->cc[0].day_w - kws)%(p->step);
				if (k) {
					p->rd[0] -= k;	/* Floor to n'th day of the week.  */
					GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
					memcpy ( (void *)&(p->cc[1]), (void *)&(p->cc[0]), sizeof (struct GMT_gcal));	/* Set to same as first calendar */
				}
				p->sd[0] = 0.0;
				p->dt[0] = GMT_rdc2dt (p->rd[0], p->sd[0]);
			}
			
			k = (p->cc[0].day_w - kws + 7) % 7;		/* k will be in the 0- 6 range where 0 is the kws day (could be Wednesday or whatever) */
			k += p->step;					/* Step forward the required number of days */
			if (k > 6) {					/* Stepped into next week, must reset stride to start at week start */
				GMT_LONG n_weeks;
				n_weeks = p->step / 7;			/* Because the k % 7 would not count weeks */
				k -= p->step;
				k = (7 - k) % 7;			/* Number of days left in the week */
				if (k == 0 && n_weeks == 0) k = 1;	/* Must go at least 1 day forward */
				p->rd[1] = p->rd[0] + k + n_weeks * 7;	/* Next rd */
			}
			else {
				/* It is OK to add p->step days to rd[0] to get rd[1]  */
				p->rd[1] = p->rd[0] + p->step;
			}
			p->sd[1] = 0.0;
			GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
			p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
			break;
		
		case 'r':	/* Gregorian weeks.  Step size is 1.  */
			if (init) {
				/* Floor to the first day of the week start.  */
				k = p->cc[0].day_w - gmtdefs.time_week_start;
				if (k) {
					p->rd[0] -= GMT_cal_imod(k, 7);
 					GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
 				}
				p->sd[0] = 0.0;
 				p->dt[0] = GMT_rdc2dt (p->rd[0], p->sd[0]);
 			}
 			p->rd[1] = p->rd[0] + 7;	/* We know step size is 1; other wise, use 7 * step  */
 			p->sd[1] = 0.0;
 			GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
			p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
			break;

		case 'u':
		case 'U':
			if (init) {
				/* Floor to the n'th iso week of the year:  */
				p->sd[0] = 0.0;
				k = (p->cc[0].iso_w - 1)/p->step;
				p->cc[0].iso_w = k * p->step + 1;
				p->rd[0] = GMT_rd_from_iywd (p->cc[0].iso_y, p->cc[0].iso_w, 1);
				GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
				memcpy ( (void *)&(p->cc[1]), (void *)&(p->cc[0]), sizeof (struct GMT_gcal));	/* Set to same as first calendar */
				p->dt[0] = GMT_rdc2dt (p->rd[0], p->sd[0]);
				if (GMT_iso_ywd_is_bad (p->cc[0].iso_y, p->cc[0].iso_w, 1) ) {
					fprintf (stderr, "GMT_LOGIC_BUG:  bad ywd on floor (month) in GMT_init_moment_interval()\n");
					return;
				}
			}
			/* I'm not sure how to move <step> weeks ahead.
				I have implemented it this way:
				add 7*step days.  If this puts you in
				a new ISO year, then truncate back to
				the start of the new ISO year.  But just
				in case that somehow returns you to where
				you were (I think it can't), then go ahead
				step weeks from where you were.  */
			p->rd[1] = p->rd[0] + p->step * 7;
			GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
			if (p->cc[1].iso_y != p->cc[0].iso_y) {
				k = p->cc[1].iso_y;
				p->rd[1] = GMT_rd_from_iywd (k, 1, 1);
				if (p->rd[1] == p->rd[0]) p->rd[1] += p->step * 7; /* Just in case */
				GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
			}
			p->sd[1] = 0.0;
			p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
			break;
			
		case 'o':
		case 'O':
			/* Get the n'th month of the Gregorian year  */
			if (init) {
				/* floor to the step'th month:  */
				p->sd[0] = 0.0;
				p->cc[0].day_m = 1;
				k = (p->cc[0].month-1)/p->step;
				p->cc[0].month = k * p->step + 1;
				if (GMT_g_ymd_is_bad (p->cc[0].year, p->cc[0].month, p->cc[0].day_m) ) {
					fprintf (stderr, "GMT_LOGIC_BUG:  bad ymd on floor (month) in GMT_init_moment_interval()\n");
					return;
				}
				p->rd[0] = GMT_rd_from_gymd (p->cc[0].year, p->cc[0].month, p->cc[0].day_m);
				GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
				memcpy ( (void *)&(p->cc[1]), (void *)&(p->cc[0]), sizeof (struct GMT_gcal));	/* Set to same as first calendar */
				p->dt[0] = GMT_rdc2dt (p->rd[0], p->sd[0]);
			}
			/* Now get next n'th month  */
			p->cc[1].month = p->cc[0].month + p->step;
			if (p->cc[1].month > 12) {
				p->cc[1].month = 1;
				p->cc[1].year++;
			}
			p->rd[1] = GMT_rd_from_gymd (p->cc[1].year, p->cc[1].month, p->cc[1].day_m);
			GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
			p->sd[1] = 0.0;
			p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
			break;

		case 'y':
		case 'Y':
			if (init) {
				/* Floor to the step'th year, either ISO or Gregorian, depending on... */
				if (GMT_plot_calclock.date.iso_calendar) {
					p->sd[0] = 0.0;
					if (p->step > 1) p->cc[0].iso_y -= GMT_cal_imod (p->cc[0].iso_y, p->step);
					p->rd[0] = GMT_rd_from_iywd (p->cc[0].iso_y, 1, 1);
				}
				else {
					p->sd[0] = 0.0;
					if (p->step > 1) p->cc[0].year -= GMT_cal_imod (p->cc[0].year, p->step);
					p->rd[0] = GMT_rd_from_gymd (p->cc[0].year, 1, 1);
				}
				GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
				memcpy ( (void *)&(p->cc[1]), (void *)&(p->cc[0]), sizeof (struct GMT_gcal));	/* Set to same as first calendar */
				p->dt[0] = GMT_rdc2dt (p->rd[0], p->sd[0]);
			}
			/* Now step ahead step years, depending on calendar type:  */
			if (GMT_plot_calclock.date.iso_calendar) {
				p->cc[1].iso_y = p->cc[0].iso_y + p->step;
				p->rd[1] = GMT_rd_from_iywd (p->cc[1].iso_y, 1, 1);
			}
			else {
				p->cc[1].year = p->cc[0].year + p->step;
				p->rd[1] = GMT_rd_from_gymd (p->cc[1].year, 1, 1);
			}
			p->sd[1] = 0.0;
			p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
			GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
			break;
		default:
			/* Should never get here because unit should already have been verified.  */
			fprintf (stderr, "GMT_LOGIC_BUG:  Bad unit in GMT_init_moment_interval()\n");
			break;
	}
}		



void	GMT_small_moment_interval (struct GMT_MOMENT_INTERVAL *p, GMT_LONG step_secs, GMT_LONG init) {

	/* Called by GMT_moment_interval ().  Get here when p->stuff[0] is initialized and
		0 < step_secs <= GMT_DAY2SEC_I.  If init, stuff[0] may need to be truncated.  */
	
	double	x;
	
	if (step_secs == GMT_DAY2SEC_I) {
		/* Special case of a 1-day step.  */
		if (p->sd[0] != 0.0) {	/* Floor it to start of day.  */
			p->dt[0] -= (p->sd[0] * gmtdefs.time_system.i_scale);
			p->sd[0] = 0.0;
		}
		/* Now we step to next day in rd first, and set dt from there.
			This will work even when leap seconds are implemented.  */
		p->rd[1] = p->rd[0] + 1;
		GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
		p->sd[1] = 0.0;
		p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
	}
	else {
		if (init) {
			x = step_secs * floor (p->sd[0] / step_secs);
			if (x != p->sd[0]) {
				p->dt[0] -= ((p->sd[0] - x) * gmtdefs.time_system.i_scale);
				x = p->sd[0];
			}
		}
		/* Step to next interval time.  If this would put GMT_DAY2SEC_I secs
		in today, go to next day at zero.  This will work even when
		leap seconds are implemented and today is a leap second say,
		unless also step_secs == 1.  That special action will have to
		be taken and will be coded later when leap seconds are put in.
		*/
		x = p->sd[0] + step_secs;
		if (x >= GMT_DAY2SEC_F) {	/* Should not be greater than  */
			p->sd[1] = 0.0;
			p->rd[1] = p->rd[0] + 1;
			GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
			p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
		}
		else {
			p->sd[1] = x;
			p->dt[1] = p->dt[0] + step_secs * gmtdefs.time_system.i_scale;
			/* No call here to reset cc[1] struct, as rd hasn't changed.
				Later, if it is desired to reset struct for clock
				changes on same day, add a call here.  */
		}
	}
}


		
GMT_LONG	GMT_gmonth_length (GMT_LONG year,  GMT_LONG month) {

	/* Return the number of days in a month,
		using the gregorian leap year rule.
	Months are numbered from 1 to 12.  */
	
	GMT_LONG	k;
	
	if (month < 1 || month > 12) return 0;
	
	if (month != 2) {
		k = month%2;
		if (month < 8) {
			return (30 + k);
		}
		else {
			return (31 - k);
		}
	}
	
	k = (GMT_is_gleap (year) ) ? 29 : 28;
	return (k);
}

void GMT_format_calendar (char *date, char *clock, struct GMT_DATE_IO *D, struct GMT_CLOCK_IO *C, GMT_LONG upper, GMT_LONG kind, double dt)
{	/* Given the internal time representation dt and the formatting information
	 * in the D and C structure, write the calendar representation to strings date and clock,
	 * but skip either string if it is a NULL pointer */
	 
	struct GMT_gcal calendar;
	GMT_LONG i_sec, m_sec, ap, ival[3];
	char text[GMT_CALSTRING_LENGTH];
	double step;

	step = 0.5 / C->f_sec_to_int / gmtdefs.time_system.scale;	/* Precision desired in time units */

	GMT_gcal_from_dt (dt + step, &calendar);			/* Convert dt to a complete calendar structure */
	
	if (date) {	/* Not NULL, want to format this string */
		/* Now undo Y2K fix to make a 2-digit year here if necessary */
	
		if (D->day_of_year) {		/* Using the year and day-of-year as date entries */
			if (D->item_pos[0] != -1) ival[D->item_pos[0]] = (D->Y2K_year) ? GMT_abs (calendar.year) % 100 : calendar.year;
			if (D->item_pos[3] != -1) ival[D->item_pos[3]] = calendar.day_y;
		}
		else if (D->iso_calendar) {	/* Using ISO year, week and day-of-week entries. Order is fixed to be y-m-d */
			ival[0] = (D->Y2K_year) ? GMT_abs (calendar.iso_y) % 100 : calendar.iso_y;
			ival[1] = calendar.iso_w;
			ival[2] = calendar.iso_d;
		}
		else {				/* Gregorian calendar entries */
			if (D->item_pos[0] != -1) ival[D->item_pos[0]] = (D->Y2K_year) ? GMT_abs (calendar.year) % 100 : calendar.year;
			if (D->item_pos[1] != -1) ival[D->item_pos[1]] = calendar.month;
			if (D->item_pos[2] != -1) ival[D->item_pos[2]] = calendar.day_m;
		}
		memset ((void *)date, 0, (size_t)GMT_CALSTRING_LENGTH);			/* To set all to zero */
		if (D->mw_text)	{						/* Must write month or week name */
			if (D->iso_calendar)
				strcpy (text, GMT_time_language.week_name[kind]);
			else
				strcpy (text, GMT_time_language.month_name[kind][ival[D->item_pos[1]]-1]);
			if (upper) GMT_str_toupper (text);
			if (D->item_pos[1] == 0)		/* Month/week first */
				sprintf (date, D->format, text, ival[1], ival[2]);
			else if (D->item_pos[1] == 1)	/* Month/week second */
				sprintf (date, D->format, ival[0], text, ival[2]);
			else 				/* Month/week third */
				sprintf (date, D->format, ival[0], ival[1], text);
		}
		else								/* Plain numerical filler-upper */
			sprintf (date, D->format, ival[0], ival[1], ival[2]);	/* Write date in correct order for this format */
	}

	if (!clock) return;	/* Do not want a formatted clock string - return here */
	
	memset ((void *)clock, 0,  (size_t)GMT_CALSTRING_LENGTH);			/* To set all to zero */
	i_sec = (GMT_LONG) floor (calendar.sec);
	m_sec = (GMT_LONG) floor (C->f_sec_to_int * (calendar.sec - i_sec));
	
	if (C->twelve_hr_clock) {		/* Must deal with am/pm formatting */
		if (calendar.hour < 12)
			ap = 0;
		else {
			ap = 1;
			calendar.hour -= 12;
		}
		if (calendar.hour == 0) calendar.hour = 12;
		if (C->n_sec_decimals) {	/* Have fractional seconds has smallest item */
			sprintf (clock, C->format, calendar.hour, calendar.min, i_sec, m_sec, C->ampm_suffix[ap]);
		}
		else if (C->order[2] > 0) {	/* Have integer seconds as smallest item */
			sprintf (clock, C->format, calendar.hour, calendar.min, i_sec, C->ampm_suffix[ap]);
		}
		else if (C->order[1] > 0) {	/* Have integer minutes as smallest item */
			sprintf (clock, C->format, calendar.hour, calendar.min, C->ampm_suffix[ap]);
		}
		else {	/* Have integer hours as smallest item */
			sprintf (clock, C->format, calendar.hour, C->ampm_suffix[ap]);
		}
	}
	else {					/* 24-hour clock formatting */
		sprintf (clock, C->format, calendar.hour, calendar.min, i_sec, m_sec);
	}
	return;
}

void GMT_get_time_label (char *string, struct GMT_PLOT_CALCLOCK *P, struct GMT_PLOT_AXIS_ITEM *T, double t)
{	/* Assemble the annotation label given the formatting options presented */
	struct GMT_gcal calendar;

	GMT_gcal_from_dt (t, &calendar);			/* Convert t to a complete calendar structure */

	switch (T->unit) {
		case 'Y':	/* 4-digit integer year */
			(P->date.compact) ? sprintf (string, "%ld", calendar.year) : sprintf (string, "%4.4ld", calendar.year);
			break;
		case 'y':	/* 2-digit integer year */
			/* (P->date.compact) ? sprintf (string, "%ld", calendar.year % 100) : sprintf (string, "%2.2ld", calendar.year % 100); */
			sprintf (string, "%2.2ld", calendar.year % 100);
			break;
		case 'O':	/* Plot via date format */
			GMT_format_calendar (string, CNULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'o':	/* 2-digit month */
			(P->date.compact) ? sprintf (string, "%ld", calendar.month) : sprintf (string, "%2.2ld", calendar.month);
			break;
		case 'U':	/* ISO year, week, day via date format */
			GMT_format_calendar (string, CNULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'u':	/* 2-digit ISO week */		
			(P->date.compact) ? sprintf (string, "%ld", calendar.iso_w) : sprintf (string, "%2.2ld", calendar.iso_w);
			break;
		case 'K':	/*  ISO Weekday name */
			if (T->upper_case) GMT_str_toupper (GMT_time_language.day_name[T->flavor][calendar.iso_d%7]);
			sprintf (string, "%s", GMT_time_language.day_name[T->flavor][calendar.iso_d%7]);
			break;
		case 'k':	/* Day of the week 1-7 */
			sprintf (string, "%ld", (calendar.day_w - gmtdefs.time_week_start + 7) % 7 + 1);
			break;
		case 'D':	/* Day, via date format */
			GMT_format_calendar (string, CNULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'd':	/* 2-digit day or 3-digit day of year */
		case 'R':	/* Gregorian month-days are the same thing - only they start at beginning of weeks and not months */
			if (P->date.day_of_year)
				(P->date.compact) ? sprintf (string, "%ld", calendar.day_y) : sprintf (string, "%3.3ld", calendar.day_y);
			else
				(P->date.compact) ? sprintf (string, "%ld", calendar.day_m) : sprintf (string, "%2.2ld", calendar.day_m);
			break;
		case 'H':	/* Hours via clock format */
			GMT_format_calendar (CNULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'h':	/* 2-digit hour */
			(P->date.compact) ? sprintf (string, "%ld", calendar.hour) : sprintf (string, "%2.2ld", calendar.hour);
			break;
		case 'M':	/* Minutes via clock format */
			GMT_format_calendar (CNULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'm':	/* 2-digit minutes */
			(P->date.compact) ? sprintf (string, "%ld", calendar.min) : sprintf (string, "%2.2ld", calendar.min);
			break;
		case 'C':	/* Seconds via clock format */
			GMT_format_calendar (CNULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'c':	/* 2-digit seconds */
			(P->date.compact) ? sprintf (string, "%ld", (GMT_LONG)irint(calendar.sec)) : sprintf (string, "%2.2ld", (GMT_LONG)irint(calendar.sec));
			break;
		default:
			fprintf (stderr, "%s: INTERNAL ERROR: wrong unit passed to GMT_get_time_label\n", GMT_program);
			sprintf (string, "NaN");
			break;
	}
}

/* Here lies functions not in use by GMT.  We will probably delete these */
#ifdef USE_UNUSED_GMT_FUNCTIONS
/* String reading functions for parsing ISO 8601 cal and clock  */

GMT_LONG	GMT_read_clock (char *s, double *t) {
/* 	Given a clock string s, try to read the form
	hh[:mm[:ss[.xxx]]] with 0 <= hh <= 24 (yes, 24; 
	ISO 8601 allows this), 0 <= mm <= 59, and 0 <= 
	ss[.xxx] < 60 (floating point fractions of a 
	second are allowed, but leap seconds are not yet
	handled).
	Return -1 on failure.
	On success, store total seconds since 00:00:00 
	in t and return 0.
	Currently, a null string is a failure condition.
	W H F Smith, 20 april 2000
*/

	double	dsec;
	GMT_LONG	j, k;
	char	*cm, *cs;
	
	/* Get hours:  */
	cm = strpbrk (s, ":");
	if (cm) {
		k = strlen(s);
		j = strlen(cm);
		s[k-j] = '\0';
		cm = &cm[1];	/* Move beyond colon  */
	}
	if ( (sscanf(s, "%" GMT_LL "d", &k)) != 1) return (-1);
	if (k < 0 || k > 24) return (-1);
	*t = GMT_HR2SEC_I * k;		/* t now has hours (in secs) */
	
	if (!(cm)) return (0);	/* This allows colon-terminated
		strings to be treated as OK.  */
	cs = strpbrk (cm, ":");
	if (cs) {
		k = strlen(cm);
		j = strlen(cs);
		cm[k-j] = '\0';
		cs = &cs[1];
	}
	if ( (sscanf(cm, "%" GMT_LL "d", &k)) != 1) return (-1);
	if (k < 0 || k > 59) return (-1);
	(*t) += GMT_MIN2SEC_I * k;	/* Now add in minutes (in secs) */
	
	if (!(cs)) return (0);
	if ( (sscanf(cs, "%lf", &dsec)) != 1) return (-1);
	if (dsec < 0.0 || dsec >= 60.0) return (-1);
	*t += dsec;			/* Finally add in seconds */
	return (0);
}

GMT_LONG	GMT_read_cal (char *s, GMT_cal_rd *rd) {
/*	Given a string thought to be a calendar string,
	find the rata die date of the string.  The string
	could be of the form 
	[<space>][+/-]<year>[-[W]<j>[-<k>]]
	where the leading hyphen, if present, negates
	the year, and subsequent hyphens are delimiters
	of fields for <j> and <k>.  The 'W', if present, 
	indicates that <year>, <j>, <k> are ISO year, 
	ISO week, and ISO day of week; else if 'W' not
	present then <year>, <j>, <k> are (proleptic
	gregorian) year, month, day of month.  */

	GMT_LONG	i, j, k, itemp1, itemp2=1, itemp3=1, is_iso=0;
	char	*cj, *ck = CNULL;
	
	/* A period or comma would be wrong for this format:  */
	if ( (strpbrk (s, ".") ) || (strpbrk (s, ",") ) ) return (-1);
	
	/* Look for next hyphen after possible leading
	white space and possible first hyphen  */
	
	i = 0;
	while (s[i] && s[i] == ' ') i++;
	if (s[i] == '-') i++;
	if (s[i] == '\0') return (-1);
	
	cj = strpbrk (&s[i], "-");
	if (cj != CNULL) {
		j = strlen (cj);
		k = strlen (s);
		s[k-(i+j)] = '\0';
	}
	
	if ( (sscanf (s, "%" GMT_LL "d", &itemp1) ) != 1) return (-1);
	
	/* A null value of cj is not an error.  We might have
		had "1985T" passed to a function and it might
		have stripped off the 'T' and passed the remaining
		string to this routine, in which case cj is null,
		and we should interpret this as gregorian, with
		month = 1 and day of month = 1.  */
	
	if (cj) {
		if (cj[1] == 'W') {
			is_iso = 1;
			cj = &cj[2];
		}
		else {
			cj = &cj[1];
		}
		ck = strpbrk (cj, "-");
		if (ck != CNULL) {
			j = strlen (ck);
			k = strlen (cj);
			cj[k-j] = '\0';
		}
		if ( (sscanf (cj, "%" GMT_LL "d", &itemp2) ) != 1) return (-1);
		if (itemp2 < 1) return (-1);
		if (is_iso) {
			if (itemp2 > 53) return (-1);
		}
		else {
			if (itemp2 > 12) return (-1);
		}
		if (ck) {
			/* Read it, skipping the leading hyphen.  */
			if ( (sscanf (&ck[1], "%" GMT_LL "d", &itemp3) ) != 1) return (-1);
			if (itemp3 < 1) return (-1);
			if (is_iso) {
				if (itemp3 > 7) return (-1);
			}
			else {
				if (itemp3 > 31) return (-1);
			}
		}
	}
	
	/* Get here when ready to make a rata die out of 3 itemp values,
		based on value of is_iso:  */
	
	if (is_iso)
		*rd = GMT_rd_from_iywd (itemp1, itemp2, itemp3);
	else
		*rd = GMT_rd_from_gymd (itemp1, itemp2, itemp3);
	return (0);
}

double	GMT_cal_dmod (double x, double y) {
	if (y == 0.0) {
		fprintf (stderr, "GMT_cal_dmod:  DOMAIN ERROR.\n");
		return (x);
	}
	return (x - y * floor(x/y) );
}

GMT_cal_rd GMT_kday_on_or_after (GMT_cal_rd date, GMT_LONG kday) {
/*	Given date and kday, return the date of the nearest kday
	on or after the given date.
*/
	return (GMT_kday_on_or_before(date+6, kday));
}

GMT_cal_rd GMT_kday_nearest (GMT_cal_rd date, GMT_LONG kday) {
/*	Given date and kday, return the date of the nearest kday
	to the given date.
*/
	return (GMT_kday_on_or_before(date+3, kday));
}

#endif
