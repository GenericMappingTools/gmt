/*
 *	$Id$
 */
/* string_check_subs.c
 * Subroutines for testing WVS string quality.
 * Checks for simple duplicates, spikes, and crossing lines.
 *
 * Walter Smith, 4 Feb, 1994.  */

#define COASTLIB 1
#include "wvs.h"
int kill_trivial_duplicates (struct FLAGPAIR p[], int *n);
int look_for_crossings (struct FLAGPAIR p[], int n);
int intersect (double ax0, double ay0, double ax1, double ay1, double bx0, double by0, double bx1, double by1);
int look_for_spikes (struct FLAGPAIR p[],int n);

int stringcheck (struct LONGPAIR p[], struct FLAGPAIR work[], int *n)
{

	/* Returns 0 if EITHER input string passes all tests OR input
	string had fixable problems.  (If you need to tell which, save
	the value of n before the call, and compare with n after the call).
	Returns -1 if the string has unfixable problems (crossing lines).

	Operates by copying p to work.  */

	int i, spike, ntrivial, spike_total;
	int	remove_spikes();
	int	compare_xy(const void *p1, const void *p2);
	int	compare_k(const void *p1, const void *p2), compare_absk(const void *p1, const void *p2);

	for (i = 0; i < *n; i++) {
		work[i].x = p[i].x;
		work[i].y = p[i].y;
		work[i].k = i + 1;
	}

	ntrivial = 0;
	ntrivial = kill_trivial_duplicates(work, n);
	if (*n < 3) {
		if (*n < 2)
			return(-1);
		else
			return(0);
	}

	spike_total = 0;
	while (*n > 2 && (spike = remove_spikes(work, n))) {
		spike_total += spike;
		ntrivial += kill_trivial_duplicates(work, n);
	}
	if (*n < 3) {
		if (*n < 2)
			return(-1);
		else
			return(0);
	}

	if (look_for_crossings(work, *n))
		return(-1);
	else {
		/* Copy to final array  */
		for (i = 0; i < *n; i++) {
			p[i].x = work[i].x;
			p[i].y = work[i].y;
		}
		return(0);
	}
}

int look_for_crossings (struct FLAGPAIR p[], int n)
{
	/* Uses the intersect routine to see if lines cross:
		Can't do anything unless n is at least 4 */
	int	i, j, jstop;

	if (n < 4)
		return(0);

	/* Also, if string closes, this will generate an intersection,
		so skip last point if string is closed.  */
	jstop = ((p[n-1].x == p[0].x) && (p[n-1].y == p[0].y)) ? n-1 : n;
	for (i = 0; i < jstop-3; i++) {
		for (j = i + 3; j < jstop; j++) {
			if (intersect((double)p[i].x, (double)p[i].y, (double)p[i+1].x, (double)p[i+1].y,
				(double)p[j-1].x, (double)p[j-1].y, (double)p[j].x, (double)p[j].y)) 
					return(-1);
		}
	}
	return(0);
}

int compare_xy (const void *P1, const void *P2)
{
	struct FLAGPAIR *p1, *p2;

	p1 = (struct FLAGPAIR *)P1;
	p2 = (struct FLAGPAIR *)P2;
	/* Ignore k values  */
	if (p1->x > p2->x) return(1);
	if (p1->x < p2->x) return(-1);
	if (p1->y > p2->y) return(1);
	if (p1->y < p2->y) return(-1);
	return(0);
}

int compare_k (const void *P1, const void *P2)
{
	struct FLAGPAIR *p1, *p2;

	p1 = (struct FLAGPAIR *)P1;
	p2 = (struct FLAGPAIR *)P2;
	/* Here, k's magnitude and sign are both important */
	if (p1->k < p2->k) return(-1);
	if (p1->k > p2->k) return(1);
	return(0);
}

int compare_absk (const void *P1, const void *P2)
{
	struct FLAGPAIR *p1, *p2;

	p1 = (struct FLAGPAIR *)P1;
	p2 = (struct FLAGPAIR *)P2;
	/* Here, k's magnitude only, not sign, is important */
	if (abs(p1->k) < abs(p2->k)) return(-1);
	if (abs(p1->k) > abs(p2->k)) return(1);
	return(0);
}

int remove_spikes (struct FLAGPAIR p[], int *n)
{
	/* This routine calls look_for_spikes() and removes spikes if found.
		Returns the number found.  You need to keep calling it until
		it returns 0.  String version.  */

	int	i, j, spike;
	int	compare_k(const void *p1, const void *p2);
	
	for (j = 0; j < *n; j++) p[j].k = j + 1;

	spike = look_for_spikes(p, *n);

	if (spike) {
		/* Shuffle the negative k points down line, and move the data back  */
		qsort((char *)p, *n-1, sizeof(struct FLAGPAIR), compare_k);
		i = 0;
		while (i < *n-1 && p[i].k < 0) i++;
		if (i == *n-1) {
			fprintf(stderr,"string_check_subs: Error: Somehow all the points were flagged as spikes.\n");
			return(-1);
		}
		*n -= spike;
		for (j = 0; j < *n; j++, i++) {
			p[j].x = p[i].x;
			p[j].y = p[i].y;
			p[j].k = j + 1;
		}
	}

	return(spike);
}
	

int look_for_spikes (struct FLAGPAIR p[],int n)
{
	/* This routine looks over p[], setting p[].k negative at a spike.
		It returns the number of points deleted from the string.  

		STRING VERSION  */
	int	last, current, next, ndeleted, stop_point;

	last = 0;
	current = 1;
	next = 2;
	ndeleted = 0;
	stop_point = n-1;
	while (current < stop_point) {	/* Only need to check through n-2, since n-1 == 0  */
	
		if (p[last].x == p[next].x && p[last].y == p[next].y) {
			/* The current point is a spike.  Throw away two points */
			p[current].k = -p[current].k;
			p[next].k = -p[next].k;
			current += 2;
			next += 2;
			ndeleted += 2;
		}
		else {
			last = current;
			current = next;
			next++;
		}
	}
	return (ndeleted);
}

int kill_trivial_duplicates (struct FLAGPAIR p[], int *n)
{
	/* This routine looks over p[] and removes points which are equal to
		the previous point.  It decrements n each time it finds such
		a point.  This version is for strings, not polygons.  */

	int	i, j, ndup;


	/* Use sign of k as a marker for a duplicate  */
	for (i = 0; i < *n; i++) p[i].k = i + 1;
	i = 0;
	ndup = 0;
	while (i < *n - 1) {
		j = i + 1;
		while (j < *n && p[j].x == p[i].x && p[j].y == p[i].y) {
			p[j].k = -p[j].k;
			j++;
			ndup++;
		}
		i = j;
	}
	if (ndup) {
		for (i=0, j=0; i < *n; i++) {
			p[j] = p[i];
			if (p[j].k > 0)
				j++;
		}
		*n = j;
	}
	return(ndup);
}

int ccw (double x0, double y0, double x1, double y1, double x2, double y2)
{
	double dx1, dx2, dy1, dy2;
	
	dx1 = x1 - x0;	dy1 = y1 - y0;
	dx2 = x2 - x0;	dy2 = y2 - y0;
	if (dx1*dy2 > dy1*dx2) return (1);
	if (dx1*dy2 < dy1*dx2) return (-1);
	if ((dx1*dx2 < 0.0) || (dy1*dy2 < 0.0)) return (-1);
	if (hypot (dx1, dy1) < hypot (dx2, dy2)) return (1);
	return (0);
}

int intersect (double ax0, double ay0, double ax1, double ay1, double bx0, double by0, double bx1, double by1)
{
	return ((ccw (ax0, ay0, ax1, ay1, bx0, by0) * ccw (ax0, ay0, ax1, ay1, bx1, by1)) <= 0)
	    && ((ccw (bx0, by0, bx1, by1, ax0, ay0) * ccw (bx0, by0, bx1, by1, ax1, ay1)) <= 0);
}

int inside (double x0, double y0, double x[], double y[], int n)
{
	int i, count = 0, j = 0;
	x[n] = x[0];	y[n] = y[0];
	
	for (i = 1; i <= n; i++) {
		if (!intersect (x[i-1], y[i-1], x[i], y[i], x0, y0, 1.0e100, y0)) {
			if (intersect (x[i-1], y[i-1], x[j], y[j], x0, y0, 1.0e100, y0)) count++;
			j = i;
		}
	}
	return (count & 1);
}

double ds (struct LONGPAIR p1, struct LONGPAIR p2)
{
	/* Compute the distance, in meters, between two points.  */
	double	xscale;
	int	idx;

	xscale = cosd (0.5e-6 * (p1.y + p2.y));
	idx = abs(p1.x - p2.x);
	if (idx > 180000000) idx = 360000000 - idx;
	return(D2R * 6.371 * hypot(xscale * idx, (double)(p1.y - p2.y) ) );
}

int delete_small_moves (struct LONGPAIR p[], int n, double tol)
{
	/* This routine removes points which are equal to or nearby the
	   previous point.  "Nearby" is defined as having a distance
	   (in meters) less than tol.  It returns the number of points
	   in the string after the call.  */

	int	i, j;	/* i is pt being tested, j is n good so far  */
	double	s, ds();

	if (n <= 1) return(n);
	i = j = 1;
	while (i < n) {
		s = ds(p[i], p[j-1]);
		if (s >= tol) {
			p[j] = p[i];
			j++;
		}
		else if (s > 0.0) {
			/* If current pt is on a bin edge, don't throw it away */
			if (p[i].x%1000000 == 0 || p[i].y%1000000 == 0) {
				/* If previous is also on a bin edge, add current to list.  */
				if (p[j-1].x%1000000 == 0 || p[j-1].y%1000000 == 0) {
					p[j] = p[i];
					j++;
				}
				/* Otherwise, replace previous with current  */
				else {
					p[j-1] = p[i];
				}
			}
		}
		i++;
	}
	return(j);
}

int delete_nearby_spikes (struct LONGPAIR p[], int n, double tol)
{
	/* This routine removes two points every time it finds a "spike".
	   Here a spike is three points i, i+1, i+2 such that the distance
	   between i and i+2 is less than tol (in meters).  By default,
	   i is retained and i+1, i+2 thrown away.  However, if i+2 is
	   on a bin boundary, then we may throw away i and i+1.  If i is
	   also on a bin boundary, then we throw away only i+1.  */

	int	i, j;	/* i is pt being tested, j is n good so far  */
	double	s, ds();

	if (n <= 1) return(n);
	i = j = 1;
	/* j counts how many good ones so far;  j-1 is the last good one on the
	   list.  i is the current possible spike, so the points before and
	   after the spike are j-1, i+1.  */
	while (i+1 < n) {
		s = ds(p[i+1], p[j-1]);
		if (s >= tol) {
			/* Point i is not a spike  */
			p[j] = p[i];
			j++;
			i++;
		}
		else {
			/* Point i is a spike.  Check bin status of i+1, j-1 */
			if (p[i+1].x%1000000 == 0 || p[i+1].y%1000000 == 0) {
				/* Point i+1 is on a bin edge.  */
				if (p[j-1].x%1000000 == 0 || p[j-1].y%1000000 == 0) {
					/* Point j-1 is also on a bin edge.  Delete only i from list */
					p[j] = p[i+1];
					j++;
					i++;
				}
				else {
					/* Point j-1 is not on a bin edge.  Replace j-1 with i+1, delete i, and go on */
					p[j-1] = p[i+1];
					i+=2;
				}
			}
			else {
				/* Point i+1 is not on a bin edge.  Delete points i and i+1 from list */
				i += 2;
			}
		}
	}
	if (i == n-1) {
		p[j] = p[i];
		j++;
	}
	return(j);
}

int new_stringcheck (struct LONGPAIR p[], int *n, double x[], double y[], int id, int verbose)
{
	/* Returns 0 if EITHER input string passes all tests OR input
	string had fixable problems.  (If you need to tell which, save
	the value of n before the call, and compare with n after the call).
	Returns -1 if the string has unfixable problems (crossing lines).  */

	struct GMT_XOVER c;
	struct GMT_XSEGMENT *ylist;
	struct FLAGPAIR *pp;
	int	i, nx, delete_small_moves(), delete_nearby_spikes(), n2;
	double	tol = 4.2;	/* 4.2 meters == 1/300 inch at 1:50000 scale  */

	/* See if lon = 0 should be stored as 360 to avoid trouble in testing */
	
	if (*n > 2) {
		n2 = *n / 2;
		if ((p[n2].x/1000000) == 359) {
			for (i = 0; i < *n; i++) if (p[i].x == 0) p[i].x = 360000000;
		}
	}

	if ( (*n = delete_small_moves(p, *n, tol) ) < 3) return(0);
	if ( (*n = delete_nearby_spikes(p, *n, tol) ) < 3) return(0);

	for (i = 0; i < *n; i++) {
		x[i] = (double)p[i].x;
		y[i] = (double)p[i].y;
		if (p[i].x == 360000000) p[i].x = 0;
	}

	GMT_init_track (y, *n, &ylist);
	nx = GMT_crossover (x, y, NULL, ylist, *n, x, y, NULL, ylist, *n, TRUE, &c);

	if (nx == 1 && p[0].x == p[*n-1].x && p[0].x == irint(c.x[0]) && p[0].y == p[*n-1].y && p[0].y == irint(c.y[0]) ) {
		/* This crossing was merely a closed polygon.  */
		free((char *)c.x);
		free((char *)c.y);
		free((char *)c.xnode[0]);
		free((char *)c.xnode[1]);
		free (ylist);
		return(0);
	}

	if (nx) {
		int start, stop, cut = 0, k, j;
		
		pp = (struct FLAGPAIR *) GMT_memory (CNULL, *n, sizeof (struct FLAGPAIR), "string_check");
		for (i = 0; i < *n; i++) {	/* Copy to work array */
			pp[i].x = p[i].x;
			pp[i].y = p[i].y;
			pp[i].k = i + 1;
		}
		for (i = 0; i < nx; i++) {
			start = (int)ceil (c.xnode[0][i]);
			stop = (int) floor (c.xnode[1][i]);
			cut = abs (stop - start) + 1;
			if (cut > *n/2) {
				cut = *n - cut;
				i_swap (start, stop);
			}
				
			/* Mark all the points between start and stop as bad (negative k) */
				
			k = start - 1;
			while (k != stop) {
				k++;
				if (k == *n) k = 0;
				pp[k].k = -1;
			}
		}
		for (k=0, j=0; k < *n; k++) {	/* Remove the bad points */
			pp[j] = pp[k];
			if (pp[j].k > 0) j++;
		}
		*n = j;
		free ((char *)c.x);
		free ((char *)c.y);
		free ((char *)c.xnode[0]);
		free ((char *)c.xnode[1]);
			
		/* Make sure it worked ok */
			
		for (i = 0; i < *n; i++) {
			x[i] = pp[i].x;
			y[i] = pp[i].y;
		}
		
		if (verbose) fprintf (stderr, "\nstring_check_subs: Polygon %d had %d points removed...", id, cut);
		nx = GMT_crossover (x, y, NULL, ylist, *n, x, y, NULL, ylist, *n, TRUE, &c);
			
		if (nx) {	/* Shit... */
			if (verbose) fprintf (stderr, "FAILED\n");
			free ((char *)c.x);
			free ((char *)c.y);
			free ((char *)c.xnode[0]);
			free ((char *)c.xnode[1]);
		}
		else
			if (verbose) fprintf (stderr, "SUCCESS\n");
			
		for (i = 0; i < *n; i++) {	/* Copy back */
			p[i].x = pp[i].x;
			p[i].y = pp[i].y;
		}
		free ((char *)pp);
		free (ylist);
		
	}
	return(nx);
}
