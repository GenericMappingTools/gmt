/*--------------------------------------------------------------------
 *	$Id: gmt_calclock.c,v 1.10 2001-08-27 17:22:01 wsmith Exp $
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
/* Routines for Generic Mapping Tools conversions
	between [calendar, clock] and time.
	
	Calendar conversions are inspired partly by
	Dershowitz and Reingold.  We use "rata die"
	integer day numbers with day 1 set at
	proleptic Gregorian Mon Jan  1 00:00:00 0001.
	Proleptic means we assume that modern calendar
	can be extrapolated forward and backward; a
	year zero is used, and Gregory's reforms after
	Oct 1572 (?) are extrapolated backward.  Note
	that this is not historical.
	
	W H F Smith, April 2000

*/

#include "gmt.h"
#define GMT_I_3600     (1.0 / 3600.0)  /* Convert seconds to degrees */
#define GMT_I_60       (1.0 / 60.0)    /* Convert minutes to degrees */

BOOLEAN want_iso = FALSE;	/* Temporary to test compilation.  Delete this.  */

/* Functions to assemble/disassemble a continuous
variable (GMT_dtime) and a calendar day (GMT_cal_rd)
and time of day (double secs):  
*/

GMT_dtime GMT_rdc2dt (GMT_cal_rd rd, double secs) {
/*	Given rata die rd and double seconds, return 
	GMT_dtime value of time  */
	
	return ((GMT_dtime)(86400.0*(rd-GMT_GCAL_EPOCH) + secs));
}

void	GMT_dt2rdc (GMT_dtime t, GMT_cal_rd *rd, double *s) {
/*	Given GMT_dtime value t, load rata die day of that t
	in rd and the seconds since the start of that day in
	s.  */

	double	x;
	
	x = floor(t/86400.0);
	*s = t - 86400 * x;
	*rd = (GMT_cal_rd)(x+GMT_GCAL_EPOCH);
}


/* String reading functions for parsing ISO 8601 cal and clock  */

int	GMT_atoft (char *s, double *t) {
/*	Given s, which may be a string representation of
	a floating point number, or a calendar[and clock]
	value, try to decode s.  Return -1 on failure.
	Return 0 if s is a float, and store it in t.
	Return 1 if s is a calendar[and clock]value, and
	store the GMT_cctime value in t.
	
	Note that s could be a floating point representation
	of a time, such as a double precision Julian Day number.
	In that case, there is a GMT_cctime value which may be
	associated with it via TIME_UNIT and TIME_EPOCH.  But
	that association is not done in this routine.
	
	WHF Smith 20 April 2000.
*/

	double	secs = 0.0;
	int	j, k, err = -1, is_float = 0, is_time = 1;
	GMT_cal_rd	rd;
	char	*cp;
	
	if ( cp = strpbrk (s, "T") ) {
		k = strlen(s);
		j = strlen(cp);
		if (j > 1) {
			/* A clock string lies beyond the 'T' of s[]; read it.  */
			if (GMT_read_clock (&cp[1], &secs)) return (err);
		}
		/* Terminate s before the 'T' to make calendar string  */
		
		if (GMT_read_cal (s, &rd) ) return (err);
		
		*t = GMT_rdc2dt (rd, secs);
		return (is_time);
	}
	else if (strpbrk (s, "W") ) {
	
		if (GMT_read_cal (s, &rd) ) return (err);
		
		*t = GMT_rdc2dt (rd, secs);
		return (is_time);
	}
	else if ( (strpbrk (s, "E")) || (strpbrk (s, "e")) ) {
		
		if ( (sscanf (s, "%lf", t)) != 1) return (err);
		return (is_float);
	}
	
	/* Get here when string is ambiguous.  Examine last occurrence
	of a hyphen.  If this is preceded by a digit, then it is an
	internal hyphen, and the string is a time string.  */
	
	if (cp = strrchr (s, '-') ) {
		/* There is a hyphen.  If more than one, this is the last.  */
		k = strlen(s);
		j = strlen(cp);
		if (j < k) {
			/* A character precedes this hyphen.  */
			if (isdigit ((int)s[k-j-1]) ) {
				
				if (GMT_read_cal (s, &rd) ) return (err);
		
				*t = GMT_rdc2dt (rd, secs);
				return (is_time);
			}
			else if (s[k-j-1] != ' ') {
				/* It isn't space either; this is an error.  */
				return (err);
			}
		}
	}
	
	/* Get here when string has no calendar-like internal hyphens.
		Presume it is a float.  */
	
	if ( (sscanf (s, "%lf", t)) != 1) return (err);
	return (is_float);
}

int	GMT_read_clock (char *s, double *t) {
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

	double	tau, dsec;
	int	j, k;
	char	*cm, *cs;
	
	/* Get hours:  */
	cm = strpbrk (s, ":");
	if (cm) {
		k = strlen(s);
		j = strlen(cm);
		s[k-j] = '\0';
		cm = &cm[1];	/* Move beyond colon  */
	}
	if ( (sscanf(s, "%d", &k)) != 1) return (-1);
	if (k < 0 || k > 24) return (-1);
	tau = 3600 * k;
	
	if (!(cm)) return (0);	/* This allows colon-terminated
		strings to be treated as OK.  */
	cs = strpbrk (cm, ":");
	if (cs) {
		k = strlen(cm);
		j = strlen(cs);
		cm[k-j] = '\0';
		cs = &cs[1];
	}
	if ( (sscanf(cm, "%d", &k)) != 1) return (-1);
	if (k < 0 || k > 59) return (-1);
	tau += 60 * k;
	
	if (!(cs)) return (0);
	if ( (sscanf(cs, "%lf", &dsec)) != 1) return (-1);
	if (dsec < 0.0 || dsec >= 60.0) return (-1);
	*t = tau + dsec;
	return (0);
}

int	GMT_read_cal (char *s, GMT_cal_rd *rd) {
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

	int	i, j, k, itemp1, itemp2=1, itemp3=1, is_iso=0;
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
	
	if ( (sscanf (s, "%d", &itemp1) ) != 1) return (-1);
	
	/* A null value of cj is not an error.  We might have
		had "1985T" passed to GMT_atoft () and it might
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
		if ( (sscanf (cj, "%d", &itemp2) ) != 1) return (-1);
		if (itemp2 < 1) return (-1);
		if (is_iso) {
			if (itemp2 > 53) return (-1);
		}
		else {
			if (itemp2 > 12) return (-1);
		}
		if (ck) {
			/* Read it, skipping the leading hyphen.  */
			if ( (sscanf (&ck[1], "%d", &itemp3) ) != 1) return (-1);
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
	
	if (is_iso) {
		*rd = GMT_rd_from_iywd (itemp1, itemp2, itemp3);
	}
	else {
		*rd = GMT_rd_from_gymd (itemp1, itemp2, itemp3);
	}
	return (0);
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

double	GMT_cal_dmod (double x, double y) {
	if (y == 0.0) {
		fprintf (stderr, "GMT_cal_dmod:  DOMAIN ERROR.\n");
		return (x);
	}
	return (x - y * floor(x/y) );
}

int	GMT_cal_imod (int x, int y) {
	if (y == 0) {
		fprintf (stderr, "GMT_cal_imod:  DOMAIN ERROR.\n");
		return (x);
	}
	return (x - y * ( (int)floor((double)x/(double)y) ) );
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

GMT_cal_rd GMT_kday_on_or_before (GMT_cal_rd date, int kday) {
/*	Given date and kday, return the date of the nearest kday
	on or before the given date.
*/
	return ((GMT_cal_rd)(date - GMT_cal_imod((date-kday), 7)));
}

GMT_cal_rd GMT_kday_on_or_after (GMT_cal_rd date, int kday) {
/*	Given date and kday, return the date of the nearest kday
	on or after the given date.
*/
	return (GMT_kday_on_or_before(date+6, kday));
}

GMT_cal_rd GMT_kday_after (GMT_cal_rd date, int kday) {
/*	Given date and kday, return the date of the nearest kday
	after the given date.
*/
	return (GMT_kday_on_or_before(date+7, kday));
}

GMT_cal_rd GMT_kday_before (GMT_cal_rd date, int kday) {
/*	Given date and kday, return the date of the nearest kday
	before the given date.
*/
	return (GMT_kday_on_or_before(date-1, kday));
}

GMT_cal_rd GMT_kday_nearest (GMT_cal_rd date, int kday) {
/*	Given date and kday, return the date of the nearest kday
	to the given date.
*/
	return (GMT_kday_on_or_before(date+3, kday));
}

GMT_cal_rd GMT_nth_kday (int n, int kday, GMT_cal_rd date) {
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

BOOLEAN	GMT_is_gleap (int gyear) {
/*	Given integer proleptic gregorian calendar year,
	return TRUE if it is a Gregorian leap year; 
	else return FALSE.  */
	
	int	y400;
	
	if (GMT_cal_imod (gyear, 4) != 0) return (FALSE);
	
	y400 = GMT_cal_imod (gyear, 400);
	
	if (y400 == 0) return (TRUE);
	
	if (GMT_cal_imod (y400, 100) == 0) return (FALSE);
	
	return (TRUE);
}

GMT_cal_rd GMT_rd_from_gymd (int gy, int gm, int gd) {
/*	Given gregorian calendar year, month, day of month, 
	return the rata die integer day number.  */
	
	double s;
	int	day_offset, yearm1, rd;
	
	
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
	
	rd = GMT_GCAL_EPOCH - 1 + day_offset + gd;
	rd += 365 * yearm1;
	s = floor(yearm1/4.0) - floor(yearm1/100.0) + floor(yearm1/400.0);
	s += floor( (367 * gm - 362)/12.0);
	rd += (int)s;
	return ((GMT_cal_rd)rd);
}

int	GMT_gyear_from_rd (GMT_cal_rd date) {
/*	Given date, return proleptic Gregorian year  */

	int d0, d1, d2, d3, d4, n400, n100, n4, n1, year;
	
	d0 = date - GMT_GCAL_EPOCH;
	n400 = (int) floor (d0 / 146097.0);
	d1 = GMT_cal_imod (d0, 146097);
	n100 = (int) floor (d1 / 36524.0);
	d2 = GMT_cal_imod (d1, 36524);
	n4 = floor (d2 / 1461.0);
	d3 = GMT_cal_imod (d2, 1461);
	n1 = floor (d3 / 365.0);
	d4 = GMT_cal_imod (d4, 365) + 1;
	year = 400*n400 + 100*n100 + 4*n4 + n1;
	
	if (n100 != 4 && n1 != 4) year++;
	
	return (year);
}



/* ISO calendar routine  */

GMT_cal_rd GMT_rd_from_iywd (int iy, int iw, int id) {
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
	int	tempyear;
	
	/* Day of the week in 0 thru 6:  */
	
	gcal->day_w = GMT_cal_imod (date, 7);
	
	/* proleptic Gregorian operations:  */

	gcal->year = GMT_gyear_from_rd (date);
	
	prior_days = date - GMT_rd_from_gymd (gcal->year, 1, 1);

	gcal->day_y = prior_days + 1;
	
	tempdate = GMT_rd_from_gymd (gcal->year, 1, 1);
	
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
	
	gcal->month = (int) floor( (12*(prior_days + corexn) + 373)/367.0);
	
	tempdate = GMT_rd_from_gymd (gcal->year, gcal->month, 1);
	
	gcal->day_m = date - tempdate + 1;
	
	/* ISO operations:  */
	
	tempyear = (prior_days >= 3) ? gcal->year : gcal->year - 1;
	tempdate = GMT_rd_from_iywd (tempyear+1, 1, 1);
	gcal->iso_y = (date >= tempdate) ? tempyear + 1 : tempyear;
	gcal->iso_w = 1 + (int)floor((date - GMT_rd_from_iywd (gcal->iso_y, 1, 1))/7.0);
	gcal->iso_d = (gcal->day_w) ? gcal->day_w : 7;
}

double	GMT_usert_from_dt (GMT_dtime t) {
	double x;
	
	x = t - GMT_time_system[gmtdefs.time_system].epoch_t0;
	if (GMT_time_system[gmtdefs.time_system].i_scale != 1.0) x *= GMT_time_system[gmtdefs.time_system].i_scale;
	return (x);
}

GMT_dtime	GMT_dt_from_usert (double x) {

	/* If we are going to allow the user to have irregular times
		(months, years) then we have to do something complicated.
		
		For now, just scale and offset ?
	*/
	
	if (GMT_time_system[gmtdefs.time_system].scale == 1.0) {
		return ( (GMT_dtime) (x + GMT_time_system[gmtdefs.time_system].epoch_t0) );
	}
	else {
		return ( (GMT_dtime) (x*GMT_time_system[gmtdefs.time_system].scale 
			+ GMT_time_system[gmtdefs.time_system].epoch_t0) );
	}
}

int	GMT_y2_to_y4_yearfix (int y2) {

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

BOOLEAN	GMT_g_ymd_is_bad (int y, int m, int d) {

	/* Check year, month, day values to see if they
		are an appropriate date in the proleptic
		Gregorian calendar.  Returns TRUE if it
		thinks the month and/or day have bad
		values.  Returns FALSE if this looks like
		a valid calendar date.  */
	
	int	k;
	
	if (m < 1 || m > 12 || d < 1) return (TRUE);
	
	k = GMT_gmonth_length (y, m);
	
	if (d > k) return (TRUE);
	
	return (FALSE);
}


BOOLEAN	GMT_iso_ywd_is_bad (int y, int w, int d) {

	/* Check ISO_year, ISO_week_of_year, ISO_day_of_week
		values to see if they form a probably
		appropriate date in the ISO calendar based
		on weeks.  This is only a gross error check;
		I don't verify that a particular date actually
		exists on the ISO calendar.
		Returns TRUE if it appears something is out
		of range.
		Returns FALSE if it looks like things are OK.
	*/
	
	if (w < 1 || w > 53 || d < 1 || d > 7) return (TRUE);
	
	/* Later, insert something smarter here.  */
	
	return (FALSE);
}

void	GMT_gcal_from_dt (GMT_dtime t, struct GMT_gcal *cal) {

	/* Given time in internal units, load cal and clock info
		in cal.
		Note: uses 0 through 23 for hours (no am/pm inside here).
		Note: does not yet deal w/ leap seconds; modulo math here.
	*/
	
	GMT_cal_rd rd;
	double	x;
	
	GMT_dt2rdc (t, &rd, &x);
	GMT_gcal_from_rd (rd, cal);
	cal->hour = (int) floor (x * GMT_I_3600);
	x -= 3600.0 * cal->hour;
	cal->min  = (int) floor (x * GMT_I_60);
	cal->sec  = x - 60.0 * cal->min;
	return;
}
	
	

int	GMT_verify_time_step (int step, char unit) {

	/* Return -1 if time step and/or unit look bad, 0 if OK.  */
	
	int	retval = 0;
	
	if (step <= 0) {
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
		case 'd':
		case 'D':
			/* If step is longer than 31 it is probably an error. */
			if (step > 31) {
				fprintf (stderr, "GMT SYNTAX ERROR:  time steps in days of the month must be <= 31\n");
				retval = -1;
			}
			break;
		case 'j':
		case 'J':
			/* If step is longer than 365 it is probably an error. */
			if (step > 365) {
				fprintf (stderr, "GMT SYNTAX ERROR:  time steps in year days must be <= 365\n");
				retval = -1;
			}
			break;
		case 'k':
		case 'K':
			if (step > 7) {
				fprintf (stderr, "GMT SYNTAX ERROR:  time steps in weekdays must be <= 7\n");
				retval = -1;
			}
			break;
		case 'u':
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
		case 'y':
		case 'Y':
			break;
		default:
			fprintf (stderr, "GMT SYNTAX ERROR:  Unrecognized time axis unit.\n");
			retval = -1;
			break;
	}
	return (retval);
}


void	GMT_moment_interval (struct GMT_MOMENT_INTERVAL *p, double dt_in, BOOLEAN init) {

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
	
	int	k, kws, kml, kyd;


	if (init) {
		/* Temporarily store a breakdown of dt_in in p->stuff[0].
			Below we will take floor of this to get time a,
			and reload stuff[0] with time a.  */
		GMT_dt2rdc (dt_in, &(p->rd[0]), &(p->sd[0]) );
		GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
		p->dt[0] = dt_in;
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
			k = 60 * p->step;
			GMT_small_moment_interval (p, k, init);
			break;
		case 'h':
		case 'H':
			k = 3600 * p->step;
			GMT_small_moment_interval (p, k, init);
			break;
		case 'd':
		case 'D':
			if (p->step == 1) {
				/* Here we want every day (of the Gregorian month)
					so the stepping is easy.  */
				k = 86400;
				GMT_small_moment_interval (p, k, init);
			}
			else {
				/* Here we have code to select every n'th day of 
					the Gregorian months  */
				if (init) {
					/* Simple mod works on positive ints  */
					k = (p->cc[0].day_m - 1)%(p->step);
					if (k) {
						p->rd[0] -= k;	/* Floor to n'th day  */
						GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
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
		
		case 'j':
		case 'J':
			if (p->step == 1) {
				/* Here we want every day (of the Gregorian year)
					so the stepping is easy.  */
				k = 86400;
				GMT_small_moment_interval (p, k, init);
			}
			else {
				/* Use n'th day of the year.  */
				if (init) {
					/* Simple mod works on positive ints  */
					k = (p->cc[0].day_y - 1)%(p->step);
					if (k) {
						p->rd[0] -= k;	/* Floor to n'th day  */
						GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
					}
					p->sd[0] = 0.0;
					p->dt[0] = GMT_rdc2dt (p->rd[0], p->sd[0]);
				}
				kyd = (GMT_is_gleap (p->cc[0].year)) ? 366 : 365;
				k = p->cc[0].day_y + p->step;
				if (k > kyd) {
					/* Go to 1st day of next year:  */
					p->rd[1] = GMT_rd_from_gymd (p->cc[0].year+1, 1, 1);
				}
				else {
					p->rd[1] = p->rd[0] + p->step;
				}
				GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
				p->sd[1] = 0.0;
				p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
			}
			break;
			
		case 'k':
		case 'K':
			/* Here we need to know: how do you define the n'th day 
			of the week?  I answered this question (for now) numbering
			days as 0=Sun through 6=Sat, (this matches kday routines)
			assuming this numbering applies to gmtdefs.time_week_start, 
			and placing the base day at either 1=Monday for ISO calendar, 
			or time_week_start for Gregorian calendar.  User can then have 
			base, base+n, base+2n, etc. until base+kn would equal day 7 
			or more. When that happens, we truncate to start of next week.
			*/
			kws = (want_iso) ? 1 : gmtdefs.time_week_start;
			if (init) {
				/* Floor to the n'th day of the week from the week start:  */
				/* a simple mod will work here since both are positive ints  */
				k = (p->cc[0].day_w - kws)%(p->step);
				if (k) {
					p->rd[0] -= k;	/* Floor to n'th day of the week.  */
					GMT_gcal_from_rd (p->rd[0], &(p->cc[0]) );
				}
				p->sd[0] = 0.0;
				p->dt[0] = GMT_rdc2dt (p->rd[0], p->sd[0]);
			}
			
			k = (p->cc[0].day_w - kws) + p->step;
			if (k > 7) {
				/* Overshot start of next week; use next kday routines
					to find start of next week  */
				p->rd[1] = GMT_kday_after (p->rd[0], kws);
			}
			else {
				/* It is OK to add p->step days to rd[0] to get rd[1]  */
				p->rd[1] = p->rd[0] + p->step;
			}
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
				p->dt[0] = GMT_rdc2dt (p->rd[0], p->sd[0]);
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
				if (want_iso) {
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
				p->dt[0] = GMT_rdc2dt (p->rd[0], p->sd[0]);
			}
			/* Now step ahead step years, depending on calendar type:  */
			if (want_iso) {
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



void	GMT_small_moment_interval (struct GMT_MOMENT_INTERVAL *p, int step_secs, BOOLEAN init) {

	/* Called by GMT_moment_interval ().  Get here when p->stuff[0] is initialized and
		0 < step_secs <= 86400.  If init, stuff[0] may need to be truncated.  */
	
	double	x;
	
	if (step_secs == 86400) {
		/* Special case of a 1-day step.  */
		if (p->sd[0] != 0.0) {	/* Floor it to start of day.  */
			p->dt[0] -= p->sd[0];
			p->sd[0] = 0.0;
		}
		/* Now we step to next day in rd first, and set dt from there.
			This will work even when leap seconds are implemented.  */
		p->rd[1] = p->rd[0];
		GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
		p->sd[1] = 0.0;
		p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
	}
	else {
		if (init) {
			x = step_secs * floor (p->sd[0] / step_secs);
			if (x != p->sd[0]) {
				p->dt[0] -= (p->sd[0] - x);
				x = p->sd[0];
			}
		}
		/* Step to next interval time.  If this would put 86400 secs
		in today, go to next day at zero.  This will work even when
		leap seconds are implemented and today is a leap second say,
		unless also step_secs == 1.  That special action will have to
		be taken and will be coded later when leap seconds are put in.
		*/
		x = p->sd[0] + step_secs;
		if (x >= 86400.0) {	/* Should not be greater than  */
			p->sd[1] = 0.0;
			p->rd[1] = p->rd[0] + 1;
			GMT_gcal_from_rd (p->rd[1], &(p->cc[1]) );
			p->dt[1] = GMT_rdc2dt (p->rd[1], p->sd[1]);
		}
		else {
			p->sd[1] = x;
			p->dt[1] = p->dt[0] + step_secs;
			/* No call here to reset cc[1] struct, as rd hasn't changed.
				Later, if it is desired to reset struct for clock
				changes on same day, add a call here.  */
		}
	}
}


		
int	GMT_gmonth_length (int year,  int month) {

	/* Return the number of days in a month,
		using the gregorian leap year rule.
	Months are numbered from 1 to 12.  */
	
	int	k;
	
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
