/*
 *	$Id$
 */
/* poly_check_subs.c
 * Subroutines for testing polygon quality.
 * Uses struct PAIR p[].x,y,k for testing.
 * Checks for simple duplicates, spikes, and non-simple repeated points.
 * Routine to look for crossings is not yet built.
 *
 * Walter Smith, 2 Feb, 1994.  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define irint(x) (int)rint(x)
struct PAIR {
	int x;
	int y;
	int k;	/* p[i].k == i+1, and k is set negative if point is bad  */
};

int P_intersect (double ax0, double ay0, double ax1, double ay1, double bx0, double by0, double bx1, double by1);
int P_kill_trivial_duplicates (struct PAIR p[], int *n);
int P_remove_spikes (struct PAIR p[], int *n);
int P_look_for_spikes (struct PAIR p[], int n);

int poly_problems (struct PAIR p[], int *n)
{

	/* Returns 0 if EITHER input polygon passes all tests OR input
	polygon had fixable problems.  (If you need to tell which, save
	the value of n before the call, and compare with n after the call).
	Returns -1 if the polygon has unfixable problems.
	If return is 0 then the k's in struct PAIR are all positive and
	p[j].k = j+1 for all j.  If return is -1 then abs(p[j].k) == j + 1,
	but p[j].k < 0 at a non-trivial duplicate.

	If the polygon is not closed when sent, it will be closed by the
	program, so make sure the malloc for p[] is sufficient.  */

	int i, j, spike, duplicate, ntrivial, spike_total, bowtie;
	int	P_compare_xy(const void *v1, const void *v2), P_compare_k(const void *v1, const void *v2), P_compare_absk(const void *v1, const void *v2);
	
	if (*n < 3) {
		fprintf(stderr,"poly_check_subs: Error called with degenerate polygon (n<3).\n");
		return(-1);
	}

	/* If the polygon is not closed we check whether the last two segments cross.  If they do,
		usual closure would make a bow tie, so we replace ends by their average.  If they
		don't, we close as usual.  If poly is already closed, we skip this.  */
	bowtie = 0;
	if (p[0].x != p[*n-1].x || p[0].y != p[*n-1].y) {
		if (*n > 3 && P_intersect(p[0].x, p[0].y, p[1].x, p[1].y, p[*n-2].x, p[*n-2].y, p[*n-1].x, p[*n-1].y)) {
			/* The first and last line segments cross.  If we simply say p[n] = p[0] we will create
				a bow tie.  So we adjust p[0] = p[n-1] = average of these two.  */
			p[0].x = irint(0.5*(p[0].x + p[*n-1].x));
			p[0].y = irint(0.5*(p[0].y + p[*n-1].y));
			p[*n-1] = p[0];
			bowtie++;
		}
		else {
			p[*n].x = p[0].x;
			p[*n].y = p[0].y;
			*n++;
		}
	}

	/* Get here when we are ready to check a polygon.  We know it is closed;
		p[n-1] == p[0]  */

	ntrivial = 0;
	ntrivial = P_kill_trivial_duplicates(p, n);

	spike_total = 0;
	while ((spike = P_remove_spikes(p, n))) {
		spike_total += spike;
		ntrivial += P_kill_trivial_duplicates(p, n);
	}
	for (j = 0; j < *n; j++) p[j].k = j + 1;


	/* At this stage, the spikes and trivial duplicates are gone.  The
		polygon is also smaller by (old_n - *n).  Now it is 
		conceivable that there are other duplicate points which are
		not nearby one another, such as a closed fjord.  To look for
		these, we sort on x, mark dups with k, and use abs(k) to resort
		into correct order.  The sort on x is done only to n-1,
		because we know that point n-1 = point 0.  However, the sort
		on abs(k) must include n, so we get all points back. */

	duplicate = 0;

	qsort(p, *n-1, sizeof(struct PAIR), P_compare_xy);
	i = 0;
	while (i < *n-1) {
		j = i + 1;
		while (j < *n-1 && p[j].x == p[i].x) {
			if (p[j].y == p[i].y) {
				duplicate++;
				p[j].k = -p[j].k;
			}
			j++;
		}
		i = j;
	}
	qsort(p, *n, sizeof(struct PAIR), P_compare_absk);

	if (duplicate)
		return(-1);
	else
		return(0);
}

int P_compare_xy (const void *v1, const void *v2)
{
	/* Ignore k values  */
	struct PAIR *p1, *p2;
	p1 = (struct PAIR *)v1;
	p2 = (struct PAIR *)v2;
	if (p1->x > p2->x) return(1);
	if (p1->x < p2->x) return(-1);
	if (p1->y > p2->y) return(1);
	if (p1->y < p2->y) return(-1);
	return(0);
}

int P_compare_k (const void *v1, const void *v2)
{
	/* Here, k's magnitude and sign are both important */
	struct PAIR *p1, *p2;
	p1 = (struct PAIR *)v1;
	p2 = (struct PAIR *)v2;
	if (p1->k < p2->k) return(-1);
	if (p1->k > p2->k) return(1);
	return(0);
}

int P_compare_absk (const void *v1, const void *v2)
{
	/* Here, k's magnitude only, not sign, is important */
	struct PAIR *p1, *p2;
	p1 = (struct PAIR *)v1;
	p2 = (struct PAIR *)v2;
	if (abs(p1->k) < abs(p2->k)) return(-1);
	if (abs(p1->k) > abs(p2->k)) return(1);
	return(0);
}

int P_remove_spikes (struct PAIR p[], int *n)
{
	/* This routine calls P_look_for_spikes() and removes spikes if found.
		Returns the number found.  You need to keep calling it until
		it returns 0.  */

	int	i, j, spike;
	int	P_compare_k (const void *v1, const void *v2);
	
	for (j = 0; j < *n; j++) p[j].k = j + 1;

	spike = P_look_for_spikes (p, *n);

	if (spike) {
		if (p[0].k < 0) p[*n-1].k = -*n;
		/* Shuffle the negative k points down line, and move the data back  */
		qsort(p, *n-1, sizeof(struct PAIR), P_compare_k);
		i = 0;
		while (i < *n-1 && p[i].k < 0) i++;
		if (i == *n-1) {
			fprintf(stderr,"poly_check_subs: Error: Somehow all the points were flagged as spikes.\n");
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
	

int P_look_for_spikes (struct PAIR p[], int n)
{
	/* This routine looks over p[], setting p[].k negative at a spike.
		It returns the number of points deleted from the string.  */
	int	last, current, next, ndeleted, stop_point;

	last = n-2;
	current = 0;
	next = 1;
	ndeleted = 0;
	stop_point = n-1;
	while (current < stop_point) {	/* Only need to check through n-2, since n-1 == 0  */
	
		if (p[last].x == p[next].x && p[last].y == p[next].y) {
			/* The current point is a spike.  Throw away two points */
			if (current == 0) {
				/* Special case.  Throw away 0 and n-1, so poly starts at 1 and ends at n-2 == 1  */
				p[0].k = -p[0].k;
				p[n-1].k = -p[n-1].k;
				stop_point--;
				last--;
				current++;
				next++;
			}
			else {
				/* The usual case.  Throw away current and next  */
				p[current].k = -p[current].k;
				p[next].k = -p[next].k;
				current += 2;
				next += 2;
			}
			ndeleted += 2;
		}
		else {
			last = current;
			current = next;
			next++;
		}
		if (next > stop_point) {
			next = 0;
			while (p[next].k < 0) next++;
		}
	}
	return (ndeleted);
}

int P_kill_trivial_duplicates (struct PAIR p[], int *n)
{
	/* This routine looks over p[] and removes points which are equal to
		the previous point.  It decrements n each time it finds such
		a point.  */

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

int P_ccw (double x0, double y0, double x1, double y1, double x2, double y2)
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

int P_intersect (double ax0, double ay0, double ax1, double ay1, double bx0, double by0, double bx1, double by1)
{
	return ((P_ccw (ax0, ay0, ax1, ay1, bx0, by0) * P_ccw (ax0, ay0, ax1, ay1, bx1, by1)) <= 0)
	    && ((P_ccw (bx0, by0, bx1, by1, ax0, ay0) * P_ccw (bx0, by0, bx1, by1, ax1, ay1)) <= 0);
}

int P_inside (double x0, double y0, double x[], double y[], int n)
{
	int i, count = 0, j = 0;
	x[n] = x[0];	y[n] = y[0];
	
	for (i = 1; i <= n; i++) {
		if (!P_intersect (x[i-1], y[i-1], x[i], y[i], x0, y0, 1.0e100, y0)) {
			if (P_intersect (x[i-1], y[i-1], x[j], y[j], x0, y0, 1.0e100, y0)) count++;
			j = i;
		}
	}
	return (count & 1);
}
