/*--------------------------------------------------------------------
 *	$Id: gmt_calclock.c,v 1.4 2001-08-17 00:31:36 wsmith Exp $
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
	char	*cj, *ck = NULL;
	
	/* A period or comma would be wrong for this format:  */
	if ( (strpbrk (s, ".") ) || (strpbrk (s, ",") ) ) return (-1);
	
	/* Look for next hyphen after possible leading
	white space and possible first hyphen  */
	
	i = 0;
	while (s[i] && s[i] == ' ') i++;
	if (s[i] == '-') i++;
	if (s[i] == NULL) return (-1);
	
	cj = strpbrk (&s[i], "-");
	if (cj != NULL) {
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
		if (ck != NULL) {
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

GMT_dtime	GMT_dt_from_usert (double x) {

	/* If we are going to allow the user to have irregular times
		(months, years) then we have to do something complicated.
		
		For now, just scale and offset ?
	*/
	
	fprintf (stderr, "GMT_dt_from_usert is a dummy routine.\n");
	return ( (GMT_dtime)x);
}

int	GMT_y2_to_y4_yearfix (int y2) {

	/* Convert 2-digit year to 4-digit year.
		This is deliberately slow to punish
		people still using this, because it is written
		in August 2001, 2 years after the Y2K bug.  */
	
	fprintf (stderr, "GMT_y2_to_y4_yearfix is a dummy routine.\n");
	return (y2);
}

	
