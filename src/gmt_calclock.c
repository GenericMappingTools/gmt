/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#include "gmt_dev.h"
#include "gmt_internals.h"

/* Private functions to this file: 
   int gmt_kday_on_or_before (int date, int kday);
   int gmt_kday_after (int date, int kday);
   int gmt_kday_before (int date, int kday);
   int gmt_nth_kday (int n, int kday, int date);
   int gmt_cal_imod (int x, int y);
   int gmt_gyear_from_rd (int date);
   void GMT_gcal_from_dt (struct GMT_CTRL *GMT, double t, struct GMT_gcal *cal);	Break internal time into calendar and clock struct info.
   int GMT_gmonth_length (int year,  int month);	Get the number of days in a month by Gregorian leap rule
   void gmt_small_moment_interval (struct GMT_CTRL *GMT, struct GMT_MOMENT_INTERVAL *p, int step_secs, int init);  Aux to GMT_moment_interval
*/

/* Functions to assemble/disassemble a continuous
variable (double) and a calendar day (int)
and time of day (double secs) 
*/

double GMT_rdc2dt (struct GMT_CTRL *GMT, int64_t rd, double secs) {
/*	Given rata die rd and double seconds, return 
	time in TIME_UNIT relative to chosen TIME_EPOCH  */
	double f_days;
	f_days = (rd - GMT->current.setting.time_system.rata_die - GMT->current.setting.time_system.epoch_t0);
	return ((f_days * GMT_DAY2SEC_F  + secs) * GMT->current.setting.time_system.i_scale);
}

int64_t GMT_splitinteger (double value, int epsilon, double *doublepart) {
	/* Split value into integer and and floating part for date usage.
	   While the integer can be negative, doublepart is always >= 0
	   When doublepart is close to 0 or close to epsilon, doublepart is set
	   to zero and in the latter case, GMT_splitinteger is raised by one.
	   This makes value "snap" to multiples of epsilon.
	*/
	double x = floor (value / epsilon);
	int64_t i = lrint (x);
	*doublepart = value - x * epsilon;
	if ((*doublepart) < GMT_CONV4_LIMIT)
		*doublepart = 0.0;	/* Snap to the lower integer */
	else if ((double)epsilon - (*doublepart) < GMT_CONV4_LIMIT) {
		i++;			/* Snap to the higher integer */
		*doublepart = 0.0;
	}
	return i;
}

void GMT_dt2rdc (struct GMT_CTRL *GMT, double t, int64_t *rd, double *s) {
/*	Given time in TIME_UNIT relative to TIME_EPOCH, load rata die of this day
	in rd and the seconds since the start of that day in s.  */
	double t_sec;
	t_sec = (t * GMT->current.setting.time_system.scale + GMT->current.setting.time_system.epoch_t0 * GMT_DAY2SEC_F);
	*rd = GMT_splitinteger (t_sec, GMT_DAY2SEC_I, s) + GMT->current.setting.time_system.rata_die;
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

int gmt_cal_imod (int64_t x, int y) {
	assert (y != 0);
	return ((int)(x - y * lrint (floor ((double)x / (double)y))));
}

/* kday functions:
   Let kday be the day of the week, defined with 0 = Sun,
   1 = Mon, etc. through 6 = Sat.  (Note that ISO day of
   the week is the same for all of these except ISO Sunday
   is 7.)  Since rata die 1 is a Monday, we have kday from
   rd is simply gmt_cal_imod(rd, 7).  The various functions
   below take an rd and find another rd related to the first
   through the fact that the related day falls on a given
   kday of the week.  */

int64_t gmt_kday_on_or_before (int64_t date, int kday) {
	/* Given date and kday, return the date of the nearest kday
	   on or before the given date. */
	return (date - gmt_cal_imod (date-kday, 7));
}

int64_t gmt_kday_after (int64_t date, int kday) {
	/* Given date and kday, return the date of the nearest kday
	   after the given date. */
	return (gmt_kday_on_or_before (date+7, kday));
}

int64_t gmt_kday_before (int64_t date, int kday) {
	/* Given date and kday, return the date of the nearest kday
	   before the given date. */
	return (gmt_kday_on_or_before (date-1, kday));
}

int64_t gmt_nth_kday (int n, int kday, int64_t date) {
	/* Given date, kday, and n, return the date of the n'th
	   kday before or after the given date, according to the
	   sign of n. */
	if (n > 0)
		return (7*n + gmt_kday_before (date, kday));
	else
		return (7*n + gmt_kday_after (date, kday));
}

int GMT_gmonth_length (int year, int month) {
	/* Return the number of days in a month,
	   using the gregorian leap year rule.
	   Months are numbered from 1 to 12.  */
	
	int k;
	
	if (month < 1 || month > 12) return 0;
	
	if (month != 2) {
		k = month % 2;
		return ((month < 8) ? 30 + k : 31 - k);
	}
	
	k = (GMT_is_gleap (year)) ? 29 : 28;
	return (k);
}

/* Proleptic Gregorian Calendar operations  */

bool GMT_is_gleap (int gyear) {
	/* Given integer proleptic gregorian calendar year,
	   return true if it is a Gregorian leap year; 
	   else return false.  */
	
	int y400;
	
	if (gmt_cal_imod (gyear, 4) != 0) return (false);
	
	y400 = gmt_cal_imod (gyear, 400);
	if (y400 == 0) return (true);
	if (gmt_cal_imod (y400, 100) == 0) return (false);
	
	return (true);
}

int64_t GMT_rd_from_gymd (struct GMT_CTRL *GMT, int gy, int gm, int gd) {
	/* Given gregorian calendar year, month, day of month, 
	   return the rata die integer day number.  */
	
	double s;
	int64_t rd;
	int day_offset, yearm1;
	
	if (gm < 1 || gm > 12 || gd < 1 || gd > 31) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: GMT_rd_from_gymd given bad month or day.\n");
	}
	
	if (gm <= 2)
		day_offset = 0;
	else
		day_offset = (GMT_is_gleap (gy)) ? -1 : -2;
	
	yearm1 = gy - 1;
	rd = day_offset + gd + 365 * yearm1;
	s = floor (yearm1/4.0) - floor (yearm1/100.0) + floor (yearm1/400.0);
	s += floor ((367 * gm - 362)/12.0);
	rd += lrint (s);
	return (rd);
}

int gmt_gyear_from_rd (int64_t date) {
	/* Given rata die integer day number, return proleptic Gregorian year  */

	int64_t d0, d1, d2, d3;
	int year, n400, n100, n4, n1;
	
	d0 = date - 1;
	n400 = irint (floor (d0 / 146097.0));
	d1 = gmt_cal_imod (d0, 146097);
	n100 = irint (floor (d1 / 36524.0));
	d2 = gmt_cal_imod (d1, 36524);
	n4 = irint (floor (d2 / 1461.0));
	d3 = gmt_cal_imod (d2, 1461);
	n1 = irint (floor (d3 / 365.0));
	/* d4 = gmt_cal_imod (d3, 365) + 1; NOT USED (removed) */
	year = 400*n400 + 100*n100 + 4*n4 + n1;
	
	if (n100 != 4 && n1 != 4) year++;
	
	return (year);
}

/* ISO calendar routine  */

int64_t GMT_rd_from_iywd (struct GMT_CTRL *GMT, int iy, int iw, int id) {
	/* Given ISO calendar year, week, day of week, 
	   return the rata die integer day number.  */
	
	int64_t rdtemp;
	
	/* Add id to the iw'th Sunday after Dec 28 iy-1:  */
	rdtemp = GMT_rd_from_gymd (GMT, iy-1, 12, 28);
	return (id + gmt_nth_kday (iw, 0, rdtemp));
}

/* Set calendar struct data from fixed date:  */

void GMT_gcal_from_rd (struct GMT_CTRL *GMT, int64_t date, struct GMT_gcal *gcal) {
	/* Given rata die integer day number, load calendar structure
	   with proleptic Gregorian and ISO calendar values.  */
	
	int64_t prior_days, tempdate;
	int corexn, tempyear;
	
	/* Day of the week in 0 thru 6:  */
	
	gcal->day_w = gmt_cal_imod (date, 7);
	
	/* proleptic Gregorian operations:  */

	gcal->year = gmt_gyear_from_rd (date);
	prior_days = date - GMT_rd_from_gymd (GMT, gcal->year, 1, 1);
	gcal->day_y = (unsigned int)prior_days + 1;
	
	tempdate = GMT_rd_from_gymd (GMT, gcal->year, 3, 1);
	if (date < tempdate)
		corexn = 0;
	else
		corexn = (GMT_is_gleap (gcal->year)) ? 1 : 2;
	
	gcal->month = urint (floor ((12*(prior_days + corexn) + 373)/367.0));
	
	tempdate = GMT_rd_from_gymd (GMT, gcal->year, gcal->month, 1);
	
	gcal->day_m = (unsigned int)(date - tempdate + 1);
	
	/* ISO operations:  */
	
	tempyear = (prior_days >= 3) ? gcal->year : gcal->year - 1;
	tempdate = GMT_rd_from_iywd (GMT, tempyear+1, 1, 1);
	gcal->iso_y = (date >= tempdate) ? tempyear + 1 : tempyear;
	gcal->iso_w = 1U + urint (floor((date - GMT_rd_from_iywd (GMT, gcal->iso_y, 1, 1))/7.0));
	gcal->iso_d = (gcal->day_w) ? gcal->day_w : 7U;
}

int GMT_y2_to_y4_yearfix (struct GMT_CTRL *GMT, unsigned int y2) {

	/* Convert 2-digit year to 4-digit year, using 
	   GMT->current.setting.time_Y2K_offset_year.
	   
	   The sense of time_Y2K_offset_year is that it
	   is the first year representable in the
	   2-digit set.  For example, if the set
	   runs from 1950 to 2049, then time_Y2K_offset_year
	   should be given as 1950, and then if a
	   two-digit year is from 50 to 99 it will
	   return 1900 plus that amount.  If it is
	   from 00 to 49, it will return 2000 plus
	   that amount.
	   
	   Below, we do a modulo operation and some
	   add/subtract to compute y100, y200, fraction
	   from GMT->current.setting.time_Y2K_offset_year.  Because these
	   are constants during any run of a GMT program,
	   they could be computed once in gmt_init, but
	   we do them over and over again here, to make
	   this routine deliberately slow.  I am writing
	   the time code in August 2001, and people who
	   haven't fixed their Y2K bug data by now will
	   be punished.
	*/

	/* The GMT->current.time.Y2K_fix structure is initialized in GMT->current.io.info_init once  */
		
	return (y2 + ((y2 >= GMT->current.time.Y2K_fix.y2_cutoff) ? GMT->current.time.Y2K_fix.y100 : GMT->current.time.Y2K_fix.y200));
}

bool GMT_g_ymd_is_bad (int y, int m, int d) {

	/* Check year, month, day values to see if they
	   are an appropriate date in the proleptic
	   Gregorian calendar.  Returns true if it
	   thinks the month and/or day have bad
	   values.  Returns false if this looks like
	   a valid calendar date.  */
	
	int k;
	
	if (m < 1 || m > 12 || d < 1) return (true);
	
	k = GMT_gmonth_length (y, m);	/* Number of day in the specified month */
	
	if (d > k) return (true);	/* More days than we've got in this month */
	
	return (false);
}

bool GMT_iso_ywd_is_bad (int y, int w, int d) {

	/* Check ISO_year, ISO_week_of_year, ISO_day_of_week
	   values to see if they form a probably
	   appropriate date in the ISO calendar based
	   on weeks.  This is only a gross error check;
	   I don't verify that a particular date actually
	   exists on the ISO calendar.
	   Returns true if it appears something is out
	   of range, including negative year.
	   Returns false if it looks like things are OK.
	*/
	
	if (y < 0 || w < 1 || w > 53 || d < 1 || d > 7) return (true);
	
	/* Later, insert something smarter here.  */
	
	return (false);
}

void GMT_gcal_from_dt (struct GMT_CTRL *GMT, double t, struct GMT_gcal *cal) {

	/* Given time in internal units, load cal and clock info in cal.
	   Note: uses 0 through 23 for hours (no am/pm inside here).
	   Note: does not yet deal w/ leap seconds; modulo math here.
	*/
	
	int64_t rd, i;
	double x;

	GMT_dt2rdc (GMT, t, &rd, &x);
	GMT_gcal_from_rd (GMT, rd, cal);
	/* split double seconds and integer time */
	i = GMT_splitinteger (x, 60, &cal->sec);
	cal->hour = (unsigned int)(i / GMT_MIN2SEC_I);
	cal->min  = (unsigned int)(i % GMT_MIN2SEC_I);
}

int GMT_verify_time_step (struct GMT_CTRL *GMT, int step, char unit) {

	/* Return -1 if time step and/or unit look bad, 0 if OK.  */
	
	int retval = 0;
	
	if (step < 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: time steps must be positive.\n");
		return (-1);
	}

	switch (unit) {
		case 'c':
		case 'C':
			if (GMT_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Unit c for seconds is deprecated; use s.\n");
				if (step > 60) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: time steps in seconds must be <= 60\n");
					retval = -1;
				}
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Unrecognized time axis unit.\n");
				retval = -1;
			}
			break;
		case 's':
		case 'S':
			if (step > 60) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: time steps in seconds must be <= 60\n");
				retval = -1;
			}
			break;
		case 'm':
		case 'M':
			if (step > 60) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: time steps in minutes must be <= 60\n");
				retval = -1;
			}
			break;
		case 'h':
		case 'H':
			if (step > 24) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: time steps in hours must be <= 24\n");
				retval = -1;
			}
			break;
		case 'R':	/* Special Gregorian days: Annotate from start of each week and not first day of month */
			/* We are leveraging the machinery for 'K' and 'k' to step along but reset to start of week */
		case 'd':
		case 'D':
			/* The letter d is used for both days of the month and days of the (Gregorian) year */
			if (GMT->current.plot.calclock.date.day_of_year) {
				if (step > 365) {	/* This is probably an error.  */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: time steps in year days must be <= 365\n");
					retval = -1;
				}
			}
			else {
				/* If step is longer than 31 it is probably an error. */
				if (step > 31) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: time steps in days of the month must be <= 31\n");
					retval = -1;
				}
			}
			break;
		case 'k':
		case 'K':
			if (step > 7) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: time steps in weekdays must be <= 7\n");
				retval = -1;
			}
			break;
		case 'r':	/* Gregorian week.  Special case:  since weeks aren't numbered on Gregorian
					calendar, we only allow step size = 1 here, for ticking each week start. */
			if (step != 1) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: time step must be 1 for Gregorian weeks\n");
				retval = -1;
			}
			break;
		case 'u':	/* ISO week */
		case 'U':
			if (step > 52) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: time steps in weeks must be <= 52\n");
				retval = -1;
			}
			break;
		case 'o':
		case 'O':
			if (step > 12) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: time steps in months must be <= 12\n");
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
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Unrecognized time axis unit.\n");
			retval = -1;
			break;
	}
	return (retval);
}

void gmt_small_moment_interval (struct GMT_CTRL *GMT, struct GMT_MOMENT_INTERVAL *p, int step_secs, bool init) {

	/* Called by GMT_moment_interval ().  Get here when p->stuff[0] is initialized and
	   0 < step_secs <= GMT_DAY2SEC_I.  If init, stuff[0] may need to be truncated.  */
	
	double x;
	
	if (step_secs == GMT_DAY2SEC_I) {
		/* Special case of a 1-day step.  */
		if (p->sd[0] != 0.0) {	/* Floor it to start of day.  */
			p->dt[0] -= (p->sd[0] * GMT->current.setting.time_system.i_scale);
			p->sd[0] = 0.0;
		}
		/* Now we step to next day in rd first, and set dt from there.
			This will work even when leap seconds are implemented.  */
		p->rd[1] = p->rd[0] + 1;
		GMT_gcal_from_rd (GMT, p->rd[1], &(p->cc[1]) );
		p->sd[1] = 0.0;
		p->dt[1] = GMT_rdc2dt (GMT, p->rd[1], p->sd[1]);
	}
	else {
		if (init) {
			x = step_secs * floor (p->sd[0] / step_secs);
			if (x != p->sd[0]) {
				p->dt[0] -= ((p->sd[0] - x) * GMT->current.setting.time_system.i_scale);
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
			GMT_gcal_from_rd (GMT, p->rd[1], &(p->cc[1]) );
			p->dt[1] = GMT_rdc2dt (GMT, p->rd[1], p->sd[1]);
		}
		else {
			p->sd[1] = x;
			p->dt[1] = p->dt[0] + step_secs * GMT->current.setting.time_system.i_scale;
			/* No call here to reset cc[1] struct, as rd hasn't changed.
				Later, if it is desired to reset struct for clock
				changes on same day, add a call here.  */
		}
	}
}

void GMT_moment_interval (struct GMT_CTRL *GMT, struct GMT_MOMENT_INTERVAL *p, double dt_in, bool init) {
	/* Unchanged by this routine:
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
	
	int k, kws, kyd;
	unsigned int kml;

	if (init) {
		/* Temporarily store a breakdown of dt_in in p->stuff[0].
			Below we will take floor of this to get time a,
			and reload stuff[0] with time a.  */
		GMT_dt2rdc (GMT, dt_in, &(p->rd[0]), &(p->sd[0]) );
		GMT_gcal_from_rd (GMT, p->rd[0], &(p->cc[0]) );
		p->dt[0] = dt_in;
		p->sd[1] = p->sd[0];
		p->rd[1] = p->rd[0];
		GMT_memcpy (&(p->cc[1]), &(p->cc[0]), 1, struct GMT_gcal);	/* Set to same as first calendar */
	}
	else {
		GMT_memcpy (&(p->cc[0]), &(p->cc[1]), 1, struct GMT_gcal);
		p->dt[0] = p->dt[1];
		p->sd[0] = p->sd[1];
		p->rd[0] = p->rd[1];
	}
	
	switch (p->unit) {
		case 'c':
		case 'C':
			if (GMT_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Unit c for seconds is deprecated; use s.\n");
				k = p->step;
				gmt_small_moment_interval (GMT, p, k, init);
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_LOGIC_BUG:  Bad unit in GMT_init_moment_interval()\n");
			}
			break;
		case 's':
		case 'S':

			k = p->step;
			gmt_small_moment_interval (GMT, p, k, init);
			break;
		case 'm':
		case 'M':
			k = GMT_MIN2SEC_I * p->step;
			gmt_small_moment_interval (GMT, p, k, init);
			break;
		case 'h':
		case 'H':
			k = GMT_HR2SEC_I * p->step;
			gmt_small_moment_interval (GMT, p, k, init);
			break;
		case 'd':
		case 'D':
			if (p->step == 1) {
				/* Here we want every day (of the Gregorian month or year)
					so the stepping is easy.  */
				k = GMT_DAY2SEC_I;
				gmt_small_moment_interval (GMT, p, k, init);
			}
			else if (GMT->current.plot.calclock.date.day_of_year) {
				/* Select every n'th day of the Gregorian year  */
				if (init) {
					/* Simple mod works on positive ints  */
					/* k = (p->cc[0].day_y - 1)%(p->step); */
					k = (p->cc[0].day_y)%(p->step);		/* Want to start on a multiple of step */
					if (k) {
						p->rd[0] -= k;	/* Floor to n'th day  */
						GMT_gcal_from_rd (GMT, p->rd[0], &(p->cc[0]) );
						GMT_memcpy (&(p->cc[1]), &(p->cc[0]), 1, struct GMT_gcal);	/* Set to same as first calendar */
					}
					p->sd[0] = 0.0;
					p->dt[0] = GMT_rdc2dt (GMT, p->rd[0], p->sd[0]);
				}
				kyd = (GMT_is_gleap (p->cc[0].year)) ? 366 : 365;
				k = p->cc[0].day_y + p->step;
				if (k > kyd) {
					/* Go to 1st day of next year:  */
					p->rd[1] = GMT_rd_from_gymd (GMT, p->cc[0].year+1, 1, 1) - 1 + p->step;	/* So jjj will be multiples of step */
				}
				else {
					p->rd[1] = p->rd[0] + p->step;
				}
				GMT_gcal_from_rd (GMT, p->rd[1], &(p->cc[1]) );
				p->sd[1] = 0.0;
				p->dt[1] = GMT_rdc2dt (GMT, p->rd[1], p->sd[1]);
			}
			else {
				/* Select every n'th day of the Gregorian month  */
				if (init) {
					/* Simple mod works on positive ints  */
					k = (p->cc[0].day_m - 1)%(p->step);
					if (k) {
						p->rd[0] -= k;	/* Floor to n'th day  */
						GMT_gcal_from_rd (GMT, p->rd[0], &(p->cc[0]) );
						GMT_memcpy (&(p->cc[1]), &(p->cc[0]), 1, struct GMT_gcal);	/* Set to same as first calendar */
					}
					p->sd[0] = 0.0;
					p->dt[0] = GMT_rdc2dt (GMT, p->rd[0], p->sd[0]);
				}
				
				kml = GMT_gmonth_length (p->cc[0].year, p->cc[0].month);
				if (p->cc[0].day_m + p->step > kml) {
					/* Truncate to 1st of next month  */
					if (p->cc[0].month == 12) {
						p->cc[1].year = p->cc[0].year + 1;
						p->cc[1].month = 1;
					}
					else
						p->cc[1].month = p->cc[0].month + 1;
					p->rd[1] = GMT_rd_from_gymd (GMT, p->cc[1].year, p->cc[1].month, 1);
				}
				else	/* Adding step days will stay in current month.  */
					p->rd[1] = p->rd[0] + p->step;
				p->sd[1] = 0.0;
				GMT_gcal_from_rd (GMT, p->rd[1], &(p->cc[1]) );
				p->dt[1] = GMT_rdc2dt (GMT, p->rd[1], p->sd[1]);
			}
			break;
		
		case 'k':
		case 'K':
		case 'R':	/* Special Gregorian Days of the Month: Annotations start at start of week, not start of month */
			/* Here we need to know: how do you define the n'th day 
			of the week?  I answered this question (for now) numbering
			days as 0=Sun through 6=Sat, (this matches kday routines)
			assuming this numbering applies to GMT->current.setting.time_week_start, 
			and placing the base day at either 1=Monday for ISO calendar, 
			or time_week_start for Gregorian calendar.  User can then have 
			base, base+n, base+2n, etc. until base+kn would equal day 7 
			or more. When that happens, we truncate to start of next week.
			*/
			kws = (GMT->current.plot.calclock.date.iso_calendar) ? 1 : GMT->current.setting.time_week_start;
			if (init) {
				/* Floor to the n'th day of the week from the week start:  */
				/* a simple mod will work here since both are positive ints  */
				k = (p->cc[0].day_w - kws)%(p->step);
				if (k) {
					p->rd[0] -= k;	/* Floor to n'th day of the week.  */
					GMT_gcal_from_rd (GMT, p->rd[0], &(p->cc[0]) );
					GMT_memcpy (&(p->cc[1]), &(p->cc[0]), 1, struct GMT_gcal);	/* Set to same as first calendar */
				}
				p->sd[0] = 0.0;
				p->dt[0] = GMT_rdc2dt (GMT, p->rd[0], p->sd[0]);
			}
			
			k = (p->cc[0].day_w - kws + 7) % 7;		/* k will be in the 0- 6 range where 0 is the kws day (could be Wednesday or whatever) */
			k += p->step;					/* Step forward the required number of days */
			if (k > 6) {					/* Stepped into next week, must reset stride to start at week start */
				int n_weeks;
				n_weeks = p->step / 7;			/* Because the k % 7 would not count weeks */
				k -= p->step;
				k = (7 - k) % 7;			/* Number of days left in the week */
				if (k == 0 && n_weeks == 0) k = 1;	/* Must go at least 1 day forward */
				p->rd[1] = p->rd[0] + k + n_weeks * 7;	/* Next rd */
			}
			else	/* It is OK to add p->step days to rd[0] to get rd[1]  */
				p->rd[1] = p->rd[0] + p->step;
			p->sd[1] = 0.0;
			GMT_gcal_from_rd (GMT, p->rd[1], &(p->cc[1]) );
			p->dt[1] = GMT_rdc2dt (GMT, p->rd[1], p->sd[1]);
			break;
		
		case 'r':	/* Gregorian weeks.  Step size is 1.  */
			if (init) {
				/* Floor to the first day of the week start.  */
				k = p->cc[0].day_w - GMT->current.setting.time_week_start;
				if (k) {
					p->rd[0] -= gmt_cal_imod(k, 7);
 					GMT_gcal_from_rd (GMT, p->rd[0], &(p->cc[0]) );
 				}
				p->sd[0] = 0.0;
 				p->dt[0] = GMT_rdc2dt (GMT, p->rd[0], p->sd[0]);
 			}
 			p->rd[1] = p->rd[0] + 7;	/* We know step size is 1; other wise, use 7 * step  */
 			p->sd[1] = 0.0;
 			GMT_gcal_from_rd (GMT, p->rd[1], &(p->cc[1]) );
			p->dt[1] = GMT_rdc2dt (GMT, p->rd[1], p->sd[1]);
			break;

		case 'u':
		case 'U':
			if (init) {
				/* Floor to the n'th iso week of the year:  */
				p->sd[0] = 0.0;
				k = (p->cc[0].iso_w - 1)/p->step;
				p->cc[0].iso_w = k * p->step + 1;
				p->rd[0] = GMT_rd_from_iywd (GMT, p->cc[0].iso_y, p->cc[0].iso_w, 1);
				GMT_gcal_from_rd (GMT, p->rd[0], &(p->cc[0]) );
				GMT_memcpy (&(p->cc[1]), &(p->cc[0]), 1, struct GMT_gcal);	/* Set to same as first calendar */
				p->dt[0] = GMT_rdc2dt (GMT, p->rd[0], p->sd[0]);
				if (GMT_iso_ywd_is_bad (p->cc[0].iso_y, p->cc[0].iso_w, 1) ) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_LOGIC_BUG:  bad ywd on floor (month) in GMT_init_moment_interval()\n");
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
			GMT_gcal_from_rd (GMT, p->rd[1], &(p->cc[1]) );
			if (p->cc[1].iso_y != p->cc[0].iso_y) {
				k = p->cc[1].iso_y;
				p->rd[1] = GMT_rd_from_iywd (GMT, k, 1, 1);
				if (p->rd[1] == p->rd[0]) p->rd[1] += p->step * 7; /* Just in case */
				GMT_gcal_from_rd (GMT, p->rd[1], &(p->cc[1]) );
			}
			p->sd[1] = 0.0;
			p->dt[1] = GMT_rdc2dt (GMT, p->rd[1], p->sd[1]);
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
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_LOGIC_BUG:  bad ymd on floor (month) in GMT_init_moment_interval()\n");
					return;
				}
				p->rd[0] = GMT_rd_from_gymd (GMT, p->cc[0].year, p->cc[0].month, p->cc[0].day_m);
				GMT_gcal_from_rd (GMT, p->rd[0], &(p->cc[0]) );
				GMT_memcpy (&(p->cc[1]), &(p->cc[0]), 1, struct GMT_gcal);	/* Set to same as first calendar */
				p->dt[0] = GMT_rdc2dt (GMT, p->rd[0], p->sd[0]);
			}
			/* Now get next n'th month  */
			p->cc[1].month = p->cc[0].month + p->step;
			if (p->cc[1].month > 12) {
				p->cc[1].month = 1;
				p->cc[1].year++;
			}
			p->rd[1] = GMT_rd_from_gymd (GMT, p->cc[1].year, p->cc[1].month, p->cc[1].day_m);
			GMT_gcal_from_rd (GMT, p->rd[1], &(p->cc[1]) );
			p->sd[1] = 0.0;
			p->dt[1] = GMT_rdc2dt (GMT, p->rd[1], p->sd[1]);
			break;

		case 'y':
		case 'Y':
			if (init) {
				/* Floor to the step'th year, either ISO or Gregorian, depending on... */
				if (GMT->current.plot.calclock.date.iso_calendar) {
					p->sd[0] = 0.0;
					if (p->step > 1) p->cc[0].iso_y -= gmt_cal_imod (p->cc[0].iso_y, p->step);
					p->rd[0] = GMT_rd_from_iywd (GMT, p->cc[0].iso_y, 1, 1);
				}
				else {
					p->sd[0] = 0.0;
					if (p->step > 1) p->cc[0].year -= gmt_cal_imod (p->cc[0].year, p->step);
					p->rd[0] = GMT_rd_from_gymd (GMT, p->cc[0].year, 1, 1);
				}
				GMT_gcal_from_rd (GMT, p->rd[0], &(p->cc[0]) );
				GMT_memcpy (&(p->cc[1]), &(p->cc[0]), 1, struct GMT_gcal);	/* Set to same as first calendar */
				p->dt[0] = GMT_rdc2dt (GMT, p->rd[0], p->sd[0]);
			}
			/* Now step ahead step years, depending on calendar type:  */
			if (GMT->current.plot.calclock.date.iso_calendar) {
				p->cc[1].iso_y = p->cc[0].iso_y + p->step;
				p->rd[1] = GMT_rd_from_iywd (GMT, p->cc[1].iso_y, 1, 1);
			}
			else {
				p->cc[1].year = p->cc[0].year + p->step;
				p->rd[1] = GMT_rd_from_gymd (GMT, p->cc[1].year, 1, 1);
			}
			p->sd[1] = 0.0;
			p->dt[1] = GMT_rdc2dt (GMT, p->rd[1], p->sd[1]);
			GMT_gcal_from_rd (GMT, p->rd[1], &(p->cc[1]) );
			break;
		default:
			/* Should never get here because unit should already have been verified.  */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_LOGIC_BUG:  Bad unit in GMT_init_moment_interval()\n");
			break;
	}
}		

void GMT_format_calendar (struct GMT_CTRL *GMT, char *date, char *clock, struct GMT_DATE_IO *D, struct GMT_CLOCK_IO *W, bool upper, unsigned int kind, double dt)
{	/* Given the internal time representation dt and the formatting information
	 * in the D and C structure, write the calendar representation to strings date and clock,
	 * but skip either string if it is a NULL pointer */
	 
	int i_sec, m_sec, ap, ival[3];
	char text[GMT_LEN16] = {""};
	double step;
	struct GMT_gcal calendar;

	step = 0.5 / W->f_sec_to_int / GMT->current.setting.time_system.scale;	/* Precision desired in time units */

	GMT_gcal_from_dt (GMT, dt + step, &calendar);			/* Convert dt to a complete calendar structure */
	
	if (date) date[0] = 0;
	if (date && !D->skip) {	/* Not NULL, want to format this string */
		/* Now undo Y2K fix to make a 2-digit year here if necessary */
	
		if (D->day_of_year) {		/* Using the year and day-of-year as date entries */
			if (D->item_pos[0] != -1) ival[D->item_pos[0]] = (D->Y2K_year) ? abs (calendar.year) % 100 : calendar.year;
			if (D->item_pos[3] != -1) ival[D->item_pos[3]] = calendar.day_y;
		}
		else if (D->iso_calendar) {	/* Using ISO year, week and day-of-week entries. Order is fixed to be y-m-d */
			ival[0] = (D->Y2K_year) ? abs (calendar.iso_y) % 100 : calendar.iso_y;
			ival[1] = calendar.iso_w;
			ival[2] = calendar.iso_d;
		}
		else {				/* Gregorian calendar entries */
			if (D->item_pos[0] != -1) ival[D->item_pos[0]] = (D->Y2K_year) ? abs (calendar.year) % 100 : calendar.year;
			if (D->item_pos[1] != -1) ival[D->item_pos[1]] = calendar.month;
			if (D->item_pos[2] != -1) ival[D->item_pos[2]] = calendar.day_m;
		}
		GMT_memset (date, GMT_LEN16, char);			/* To set all to zero */
		if (D->mw_text)	{						/* Must write month or week name */
			if (D->iso_calendar)
				strncpy (text, GMT->current.language.week_name[kind], GMT_LEN16);
			else
				strncpy (text, GMT->current.language.month_name[kind][ival[D->item_pos[1]]-1], GMT_LEN16);
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

	if (clock) clock[0] = 0;
	if (!clock || W->skip) return;	/* Do not want a formatted clock string - return here */
	
	GMT_memset (clock, GMT_LEN16, char);			/* To set all to zero */
	i_sec = irint (floor (calendar.sec));
	m_sec = irint (floor (W->f_sec_to_int * (calendar.sec - i_sec)));
	
	if (W->twelve_hr_clock) {		/* Must deal with am/pm formatting */
		if (calendar.hour < 12)
			ap = 0;
		else {
			ap = 1;
			calendar.hour -= 12;
		}
		if (calendar.hour == 0) calendar.hour = 12;
		if (W->n_sec_decimals) {	/* Have fractional seconds has smallest item */
			sprintf (clock, W->format, calendar.hour, calendar.min, i_sec, m_sec, W->ampm_suffix[ap]);
		}
		else if (W->order[2] > 0) {	/* Have integer seconds as smallest item */
			sprintf (clock, W->format, calendar.hour, calendar.min, i_sec, W->ampm_suffix[ap]);
		}
		else if (W->order[1] > 0) {	/* Have integer minutes as smallest item */
			sprintf (clock, W->format, calendar.hour, calendar.min, W->ampm_suffix[ap]);
		}
		else {	/* Have integer hours as smallest item */
			sprintf (clock, W->format, calendar.hour, W->ampm_suffix[ap]);
		}
	}
	else					/* 24-hour clock formatting */
		sprintf (clock, W->format, calendar.hour, calendar.min, i_sec, m_sec);
}

void GMT_get_time_label (struct GMT_CTRL *GMT, char *string, struct GMT_PLOT_CALCLOCK *P, struct GMT_PLOT_AXIS_ITEM *T, double t)
{	/* Assemble the annotation label given the formatting options presented */
	struct GMT_gcal calendar;

	GMT_gcal_from_dt (GMT, t, &calendar);			/* Convert t to a complete calendar structure */

	switch (T->unit) {
		case 'Y':	/* 4-digit integer year */
			(P->date.compact) ? sprintf (string, "%d", calendar.year) : sprintf (string, "%04d", calendar.year);
			break;
		case 'y':	/* 2-digit integer year */
			/* (P->date.compact) ? sprintf (string, "%d", calendar.year % 100) : sprintf (string, "%02d", calendar.year % 100); */
			sprintf (string, "%02d", calendar.year % 100);
			break;
		case 'O':	/* Plot via date format */
			GMT_format_calendar (GMT, string, NULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'o':	/* 2-digit month */
			(P->date.compact) ? sprintf (string, "%d", calendar.month) : sprintf (string, "%02d", calendar.month);
			break;
		case 'U':	/* ISO year, week, day via date format */
			GMT_format_calendar (GMT, string, NULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'u':	/* 2-digit ISO week */		
			(P->date.compact) ? sprintf (string, "%d", calendar.iso_w) : sprintf (string, "%02d", calendar.iso_w);
			break;
		case 'K':	/*  ISO Weekday name */
			if (T->upper_case) GMT_str_toupper (GMT->current.language.day_name[T->flavor][calendar.iso_d%7]);
			sprintf (string, "%s", GMT->current.language.day_name[T->flavor][calendar.iso_d%7]);
			break;
		case 'k':	/* Day of the week 1-7 */
			sprintf (string, "%d", (calendar.day_w - GMT->current.setting.time_week_start + 7) % 7 + 1);
			break;
		case 'D':	/* Day, via date format */
			GMT_format_calendar (GMT, string, NULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'd':	/* 2-digit day or 3-digit day of year */
		case 'R':	/* Gregorian month-days are the same thing - only they start at beginning of weeks and not months */
			if (P->date.day_of_year)
				(P->date.compact) ? sprintf (string, "%d", calendar.day_y) : sprintf (string, "%03d", calendar.day_y);
			else
				(P->date.compact) ? sprintf (string, "%d", calendar.day_m) : sprintf (string, "%02d", calendar.day_m);
			break;
		case 'H':	/* Hours via clock format */
			GMT_format_calendar (GMT, NULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'h':	/* 2-digit hour */
			(P->date.compact) ? sprintf (string, "%d", calendar.hour) : sprintf (string, "%02d", calendar.hour);
			break;
		case 'M':	/* Minutes via clock format */
			GMT_format_calendar (GMT, NULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'm':	/* 2-digit minutes */
			(P->date.compact) ? sprintf (string, "%d", calendar.min) : sprintf (string, "%02d", calendar.min);
			break;
		case 'C':
			if (GMT_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Unit C for seconds is deprecated; use S.\n");
				GMT_format_calendar (GMT, NULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: wrong unit passed to GMT_get_time_label\n");
				sprintf (string, "NaN");
			}
			break;
		case 'S':	/* Seconds via clock format */
			GMT_format_calendar (GMT, NULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'c':
			if (GMT_compat_check (GMT, 4)) {
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Unit c for seconds is deprecated; use s.\n");
				(P->date.compact) ? sprintf (string, "%d", irint(calendar.sec)) : sprintf (string, "%02d", irint(calendar.sec));
			}
			else {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: wrong unit passed to GMT_get_time_label\n");
				sprintf (string, "NaN");
			}
			break;
		case 's':	/* 2-digit seconds */
			(P->date.compact) ? sprintf (string, "%d", irint(calendar.sec)) : sprintf (string, "%02d", irint(calendar.sec));
			break;
		default:
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: wrong unit passed to GMT_get_time_label\n");
			sprintf (string, "NaN");
			break;
	}
}
