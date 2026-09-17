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
 * rockmag.c contains code shared by the modules of the rockmag supplement
 * (rmagcurie, and future hysteresis/IRM modules). Not a module itself: no
 * THIS_MODULE_* macros, no GMT_<name> entry point (see paleomag.c /
 * windbarbs/windbarb.c for the same pattern).
 *
 * PUBLIC Functions include (3):
 *
 *      rockmag_split_branches : Split a temperature run into monotonic
 *                                heating/cooling branches
 *      rockmag_linfit         : Ordinary least-squares line fit with R^2
 *      rockmag_crossing       : Linear interpolation at a level crossing
 */
#include "gmt_dev.h"
#include "rockmag.h"

uint64_t rockmag_split_branches (struct GMT_CTRL *GMT, double *temp, uint64_t n, struct ROCKMAG_BRANCH **branch) {
	uint64_t i, start = 0, n_branches = 0, n_alloc = GMT_TINY_CHUNK;
	int dir = 0, this_dir;	/* -1 = cooling, 0 = undetermined yet, +1 = heating */
	struct ROCKMAG_BRANCH *B = NULL;

	if (n == 0) { *branch = NULL; return 0; }

	B = gmt_M_memory (GMT, NULL, n_alloc, struct ROCKMAG_BRANCH);

	if (n == 1) {	/* A single point has no direction; report it as one degenerate branch */
		B[0].start = B[0].stop = 0;	B[0].heating = true;
		*branch = B;
		return 1;
	}

	for (i = 1; i < n; i++) {
		if (temp[i] > temp[i-1]) this_dir = +1;
		else if (temp[i] < temp[i-1]) this_dir = -1;
		else continue;	/* Plateau: same temperature, no information, keep current direction */

		if (dir == 0) dir = this_dir;	/* First real step sets the initial direction */
		else if (this_dir != dir) {	/* Direction reversed: close branch at the turning point i-1 */
			if (n_branches == n_alloc) {
				n_alloc <<= 1;
				B = gmt_M_memory (GMT, B, n_alloc, struct ROCKMAG_BRANCH);
			}
			B[n_branches].start = start;	B[n_branches].stop = i-1;	B[n_branches].heating = (dir == +1);
			n_branches++;
			start = i-1;	/* The turning point starts the next branch too */
			dir = this_dir;
		}
	}
	/* Close the final branch. If dir is still 0 (temp was constant throughout), call it heating
	 * arbitrarily, same convention as the n==1 case: there is no direction to report either way. */
	if (n_branches == n_alloc) {
		n_alloc <<= 1;
		B = gmt_M_memory (GMT, B, n_alloc, struct ROCKMAG_BRANCH);
	}
	B[n_branches].start = start;	B[n_branches].stop = n-1;	B[n_branches].heating = (dir >= 0);
	n_branches++;

	*branch = B;
	return n_branches;
}

bool rockmag_linfit (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double *slope, double *intercept, double *r2) {
	uint64_t i;
	double xbar = 0.0, ybar = 0.0, Sxx = 0.0, Sxy = 0.0, SStot = 0.0, SSres = 0.0, yhat;
	gmt_M_unused (GMT);

	if (n < 2) return false;

	for (i = 0; i < n; i++) { xbar += x[i];	ybar += y[i]; }
	xbar /= (double)n;	ybar /= (double)n;

	for (i = 0; i < n; i++) {
		Sxx += (x[i] - xbar) * (x[i] - xbar);
		Sxy += (x[i] - xbar) * (y[i] - ybar);
	}
	if (Sxx <= 0.0) return false;	/* All x identical: no finite slope */

	*slope = Sxy / Sxx;
	*intercept = ybar - (*slope) * xbar;

	/* R^2 = 1 - SSres/SStot. If y is constant, both vanish together and this
	 * naturally yields NaN via IEEE754 (0.0/0.0) - no special-casing needed
	 * (same style as pmag_fisher_mean's k/a95 edge cases in paleomag.c). */
	for (i = 0; i < n; i++) {
		SStot += (y[i] - ybar) * (y[i] - ybar);
		yhat = (*intercept) + (*slope) * x[i];
		SSres += (y[i] - yhat) * (y[i] - yhat);
	}
	*r2 = 1.0 - SSres / SStot;

	return true;
}

bool rockmag_crossing (double *x, double *y, uint64_t n, double x0, double *y0) {
	uint64_t k;
	double f;

	for (k = 0; k + 1 < n; k++) {
		if (gmt_M_is_dnan (x[k]) || gmt_M_is_dnan (x[k+1])) continue;
		if (gmt_M_is_dnan (y[k]) || gmt_M_is_dnan (y[k+1])) continue;
		if (x[k] == x[k+1]) continue;				/* Cannot bracket a crossing */
		if ((x[k] - x0) * (x[k+1] - x0) > 0.0) continue;	/* Both samples on the same side of x0 */
		f = (x0 - x[k]) / (x[k+1] - x[k]);
		*y0 = y[k] + f * (y[k+1] - y[k]);
		return true;
	}
	return false;
}
