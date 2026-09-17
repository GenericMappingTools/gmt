/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2026 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * rockmag.h declares code shared by the modules of the rockmag supplement
 * (rmagcurie, and future hysteresis/IRM modules). Mirrors the paleomag.h/
 * windbarbs/windbarb.h pattern: this is a shared library header, not a
 * module itself.
 */

#ifndef _GMT_ROCKMAG_H
#define _GMT_ROCKMAG_H

/* One monotonic-temperature branch (a heating or a cooling ramp) of a
 * thermomagnetic run, as indices into the caller's original arrays. */
struct ROCKMAG_BRANCH {
	uint64_t start;	/* Index of the first point of this branch (inclusive) */
	uint64_t stop;	/* Index of the last point of this branch (inclusive) */
	bool heating;	/* true if temp[start] < temp[stop] (heating ramp), false if cooling */
};

/* Split a temperature sequence temp[0..n-1] (as measured, in whatever order
 * it was recorded) into monotonic heating/cooling branches, so a run that
 * heats up and then cools back down (or several such cycles) becomes
 * separate branches that can be analyzed independently. A turning point
 * (local extremum in temperature) is the shared last/first point of the two
 * branches on either side of it, since it is physically a single measured
 * instant. Runs of exactly equal consecutive temperatures (a plateau) do
 * not by themselves count as a direction change. On return, *branch is
 * allocated via gmt_M_memory (n_branches entries; caller must gmt_M_free
 * it) and the function returns n_branches. Returns 0 (and *branch is left
 * unallocated/NULL) if n == 0. For n == 1, a single degenerate branch
 * [0,0] is returned with heating arbitrarily set to true, since a lone
 * point carries no direction information. */
EXTERN_MSC uint64_t rockmag_split_branches (struct GMT_CTRL *GMT, double *temp, uint64_t n, struct ROCKMAG_BRANCH **branch);

/* Ordinary least-squares fit of y = intercept + slope*x over the n points
 * x[0..n-1], y[0..n-1]. *r2 is the coefficient of determination (1 minus
 * the residual-to-total sum-of-squares ratio). Returns false, leaving
 * *slope/*intercept/*r2 untouched, if n < 2 or all x are identical (a
 * vertical "fit" has no finite slope). If all y are identical the fit is
 * exact (slope 0) but *r2 naturally comes out as the IEEE754 result of
 * 0.0/0.0 (NaN, since both the residual and total sum of squares vanish
 * together) rather than being special-cased: a "perfect" fit to a
 * constant carries no information about linearity either way. */
EXTERN_MSC bool rockmag_linfit (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double *slope, double *intercept, double *r2);

/* Linearly interpolate y at the first place where x crosses the level x0,
 * scanning x[0..n-1] in the order given (for a hysteresis branch or a
 * backfield curve that is the measurement order, which is what makes "first
 * crossing" meaningful). Used for the remanence at H = 0, the coercivity at
 * M = 0, and the coercivity of remanence of a backfield curve. Intervals
 * that contain a NaN, or whose two x values are identical (they cannot
 * bracket a crossing and would divide by zero), are skipped. Returns false
 * and leaves *y0 untouched if no interval brackets x0. */
EXTERN_MSC bool rockmag_crossing (double *x, double *y, uint64_t n, double x0, double *y0);

#endif /* _GMT_ROCKMAG_H */
