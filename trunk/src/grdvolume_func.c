/*--------------------------------------------------------------------
 *	$Id: grdvolume_func.c,v 1.9 2011-05-16 21:23:10 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
/*
 * Brief synopsis: grdvolume reads a 2d binary gridded grid file, and calculates the volume
 * under the surface using exact integration of the bilinear interpolating
 * surface.  As an option, the user may supply a contour value; then the
 * volume is only integrated inside the chosen contour.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#include "gmt.h"

struct GRDVOLUME_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct C {	/* -C */
		GMT_LONG active;
		double low, high, inc;
	} C;
	struct L {	/* -L<base> */
		GMT_LONG active;
		double value;
	} L;
	struct S {	/* -S */
		GMT_LONG active;
		char unit;
	} S;
	struct T {	/* -T[c|z] */
		GMT_LONG active;
		GMT_LONG mode;
	} T;
	struct Z {	/* Z<fact>[/<shift>] */
		GMT_LONG active;
		double scale, offset;
	} Z;
};

/* This function returns the volume bounded by a trapezoid based on two vertical
 * lines x0 and x1 and two horizontal lines y0 = ax +b and y1 = cx + d
 */

double vol_prism_frac_x (struct GMT_GRID *G, GMT_LONG ij, double x0, double x1, double a, double b, double c, double d)
{
	double dzdx, dzdy, dzdxy, ca, db, c2a2, d2b2, cdab, v, x02, x12, x03, x04, x13, x14;

	dzdx  = (G->data[ij+1] - G->data[ij]);
	dzdy  = (G->data[ij-G->header->mx] - G->data[ij]);
	dzdxy = (G->data[ij-G->header->mx+1] + G->data[ij] - G->data[ij+1] - G->data[ij-G->header->mx]);

	ca = c - a;
	db = d - b;
	c2a2 = c * c - a * a;
	d2b2 = d * d - b * b;
	cdab = c * d - a * b;
	x02 = x0 * x0;	x03 = x02 * x0;	x04 = x02 * x02;
	x12 = x1 * x1;	x13 = x12 * x1;	x14 = x12 * x12;

	v = (3.0 * dzdxy * c2a2 * (x14 - x04) +
	     4.0 * (2.0 * dzdx * ca + dzdy * c2a2 + 2.0 * dzdxy * cdab) * (x13 - x03) +
	     6.0 * (2.0 * G->data[ij] * ca + 2.0 * dzdx * db + 2.0 * dzdy * cdab + dzdxy * d2b2) * (x12 - x02) +
	     12.0 * (2.0 * G->data[ij] * db + dzdy * d2b2) * (x1 - x0)) / 24.0;

	return (v);
}

/* This function returns the volume bounded by a trapezoid based on two horizontal
 * lines y0 and y1 and two vertical lines x0 = ay +b and x1 = cy + d
 */

double vol_prism_frac_y (struct GMT_GRID *G, GMT_LONG ij, double y0, double y1, double a, double b, double c, double d)
{
	double dzdx, dzdy, dzdxy, ca, db, c2a2, d2b2, cdab, v, y02, y03, y04, y12, y13, y14;

	dzdx = (G->data[ij+1] - G->data[ij]);
	dzdy = (G->data[ij-G->header->mx] - G->data[ij]);
	dzdxy = (G->data[ij-G->header->mx+1] + G->data[ij] - G->data[ij+1] - G->data[ij-G->header->mx]);

	ca = c - a;
	db = d - b;
	c2a2 = c * c - a * a;
	d2b2 = d * d - b * b;
	cdab = c * d - a * b;
	y02 = y0 * y0;	y03 = y02 * y0;	y04 = y02 * y02;
	y12 = y1 * y1;	y13 = y12 * y1;	y14 = y12 * y12;

	v = (3.0 * dzdxy * c2a2 * (y14 - y04) +
	     4.0 * (2.0 * dzdy * ca + dzdx * c2a2 + 2.0 * dzdxy * cdab) * (y13 - y03) +
	     6.0 * (2.0 * G->data[ij] * ca + 2.0 * dzdy * db + 2.0 * dzdx * cdab + dzdxy * d2b2) * (y12 - y02) +
	     12.0 * (2.0 * G->data[ij] * db + dzdx * d2b2) * (y1 - y0)) / 24.0;

	return (v);
}

void SW_triangle (struct GMT_GRID *G, GMT_LONG ij, GMT_LONG triangle, double *dv, double *da)
{	/* Calculates area of a SW-corner triangle */
	/* triangle = TRUE gets triangle, FALSE gives the complementary area */
	double x1, y0, frac;

	x1 = G->data[ij] / (G->data[ij] - G->data[ij+1]);
	y0 = G->data[ij] / (G->data[ij] - G->data[ij-G->header->mx]);
	frac = (x1 == 0.0) ? 0.0 : vol_prism_frac_x (G, ij, 0.0, x1, 0.0, 0.0, -y0 / x1, y0);
	if (triangle) {
		*dv += frac;
		*da += 0.5 * x1 * y0;
	}
	else {
		*dv += 0.25 * (G->data[ij] + G->data[ij+1] + G->data[ij-G->header->mx] + G->data[ij-G->header->mx+1]) - frac;
		*da += 1.0 - 0.5 * x1 * y0;
	}
}

void NE_triangle (struct GMT_GRID *G, GMT_LONG ij, GMT_LONG triangle, double *dv, double *da)
{	/* Calculates area of a NE-corner triangle */
	/* triangle = TRUE gets triangle, FALSE gives the complementary area */
	double x0, y1, a, x0_1, y1_1, frac = 0.0;

	x0 = G->data[ij-G->header->mx] / (G->data[ij-G->header->mx] - G->data[ij+1-G->header->mx]);
	y1 = G->data[ij+1] / (G->data[ij+1] - G->data[ij+1-G->header->mx]);
	x0_1 = 1.0 - x0;
	y1_1 = y1 - 1.0;
	if (x0_1 != 0.0) {
		a = y1_1 / x0_1;
		frac = vol_prism_frac_x (G, ij, x0, 1.0, a, 1.0 - a * x0, 0.0, 0.0);
	}
	if (triangle) {
		*dv += frac;
		*da -= 0.5 * x0_1 * y1_1;	/* -ve because we need 1 - y1 */
	}
	else {
		*dv += 0.25 * (G->data[ij] + G->data[ij+1] + G->data[ij-G->header->mx] + G->data[ij-G->header->mx+1]) - frac;
		*da += 1.0 + 0.5 * x0_1 * y1_1;	/* +ve because we need 1 - y1 */
	}
}

void SE_triangle (struct GMT_GRID *G, GMT_LONG ij, GMT_LONG triangle, double *dv, double *da)
{	/* Calculates area of a SE-corner triangle */
	/* triangle = TRUE gets triangle, FALSE gives the complementary area */
	double x0, y1, c, x0_1, frac = 0.0;

	x0 = G->data[ij] / (G->data[ij] - G->data[ij+1]);
	y1 = G->data[ij+1] / (G->data[ij+1] - G->data[ij+1-G->header->mx]);
	x0_1 = 1.0 - x0;
	if (x0_1 != 0.0) {
		c = y1 / x0_1;
		frac = vol_prism_frac_x (G, ij, x0, 1.0, 0.0, 0.0, c, -c * x0);
	}
	if (triangle) {
		*dv += frac;
		*da += 0.5 * x0_1 * y1;
	}
	else {
		*dv += 0.25 * (G->data[ij] + G->data[ij+1] + G->data[ij-G->header->mx] + G->data[ij-G->header->mx+1]) - frac;
		*da += 1.0 - 0.5 * x0_1 * y1;
	}
}

void NW_triangle (struct GMT_GRID *G, GMT_LONG ij, GMT_LONG triangle, double *dv, double *da)
{	/* Calculates area of a NW-corner triangle */
	/* triangle = TRUE gets triangle, FALSE gives the complementary area */
	double x1, y0, y0_1, frac;

	x1 = G->data[ij-G->header->mx] / (G->data[ij-G->header->mx] - G->data[ij+1-G->header->mx]);
	y0 = G->data[ij] / (G->data[ij] - G->data[ij-G->header->mx]);
	y0_1 = 1.0 - y0;
	frac = (x1 == 0.0) ? 0.0 : vol_prism_frac_x (G, ij, 0.0, x1, y0_1 / x1, y0, 0.0, 1.0);
	if (triangle) {
		*dv += frac;
		*da += 0.5 * x1 * y0_1;
	}
	else {
		*dv += 0.25 * (G->data[ij] + G->data[ij+1] + G->data[ij-G->header->mx] + G->data[ij-G->header->mx+1]) - frac;
		*da += 1.0 - 0.5 * x1 * y0_1;
	}
}

void NS_trapezoid (struct GMT_GRID *G, GMT_LONG ij, GMT_LONG right, double *dv, double *da)
{	/* Calculates area of a NS trapezoid */
	/* right = TRUE gets the right trapezoid, FALSE gets the left */
	double x0, x1;

	x0 = G->data[ij] / (G->data[ij] - G->data[ij+1]);
	x1 = G->data[ij-G->header->mx] / (G->data[ij-G->header->mx] - G->data[ij+1-G->header->mx]);
	if (right) {	/* Need right piece */
		*dv += vol_prism_frac_y (G, ij, 0.0, 1.0, x1 - x0, x0, 0.0, 1.0);
		*da += 0.5 * (2.0 - x0 - x1);
	}
	else {
		*dv += vol_prism_frac_y (G, ij, 0.0, 1.0, 0.0, 0.0, x1 - x0, x0);
		*da += 0.5 * (x0 + x1);
	}
}

void EW_trapezoid (struct GMT_GRID *G, GMT_LONG ij, GMT_LONG top, double *dv, double *da)
{	/* Calculates area of a EW trapezoid */
	/* top = TRUE gets the top trapezoid, FALSE gets the bottom */
	double y0, y1;

	y0 = G->data[ij] / (G->data[ij] - G->data[ij-G->header->mx]);
	y1 = G->data[ij+1] / (G->data[ij+1] - G->data[ij+1-G->header->mx]);
	if (top) {	/* Need top piece */
		*dv += vol_prism_frac_x (G, ij, 0.0, 1.0, y1 - y0, y0, 0.0, 1.0);
		*da += 0.5 * (2.0 - y0 - y1);
	}
	else {
		*dv += vol_prism_frac_x (G, ij, 0.0, 1.0, 0.0, 0.0, y1 - y0, y0);
		*da += 0.5 * (y0 + y1);
	}
}

double median3 (double x[])
{	/* Returns the median of the three points in x */
	if (x[0] < x[1]) {
		if (x[2] > x[1]) return (x[1]);
		if (x[2] > x[0]) return (x[2]);
		return (x[0]);
	}
	else {
		if (x[2] > x[0]) return (x[0]);
		if (x[2] < x[1]) return (x[1]);
		return (x[2]);
	}
}

GMT_LONG ors_find_kink (struct GMT_CTRL *GMT, double y[], GMT_LONG n, GMT_LONG mode)
{	/* mode: 0 = find value maximum, 1 = find curvature maximum */
	GMT_LONG i, im;
	double *c = NULL, *f = NULL;

	if (mode == 0) {	/* Find maximum value */
		for (i = im = 0; i < n; i++) if (y[i] > y[im]) im = i;
		return (im);
	}

	/* Calculate curvatures */

	c = GMT_memory (GMT, NULL, n, double);

	for (i = 1; i < (n-1); i++) c[i] = y[i+1] - 2.0 * y[i] + y[i-1];
	c[0] = c[1];
	if (n > 1) c[n-1] = c[n-2];

	/* Apply 3-point median filter to curvatures to mitigate noisy values */

	f = GMT_memory (GMT, NULL, n, double);
	for (i = 1; i < (n-1); i++) f[i] = median3 (&c[i-1]);

	/* Find maximum negative filtered curvature */

	for (i = im = 1; i < (n-1); i++) if (f[i] < f[im]) im = i;

	GMT_free (GMT, c);
	GMT_free (GMT, f);

	return (im);
}

void *New_grdvolume_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDVOLUME_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDVOLUME_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->L.value = GMT->session.d_NaN;
	C->Z.scale = 1.0;
	return ((void *)C);
}

void Free_grdvolume_Ctrl (struct GMT_CTRL *GMT, struct GRDVOLUME_CTRL *C) {	/* Deallocate control structure */
	if (C->In.file) free ((void *)C->In.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdvolume_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdvolume %s [API] - Calculating volume under a surface within a contour\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdvolume <grdfile> [-C<cval> or -C<low/high/delta>] [-L<base>] [-S<unit>] [-T[c|h]]\n\t[%s] [%s] [-Z<fact>[/<shift>]] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<grdfile> is the name of the 2-D binary data set.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-C Find area and volume inside the given <cval> contour,\n");
	GMT_message (GMT, "\t   OR search using all contours from low to high in steps of delta.\n");
	GMT_message (GMT, "\t   [Default returns entire area and volume of grid].\n");
	GMT_message (GMT, "\t-L Add volume from <base> up to contour [Default is from contour and up only].\n");
	GMT_message (GMT, "\t-S Convert degrees to distances, append a unit from %s [Default is Cartesian].\n", GMT_LEN_UNITS2_DISPLAY);
	GMT_message (GMT, "\t-T (Or -Th): Find the contour value that yields max average height (volume/area).\n");
	GMT_message (GMT, "\t   Use -Tc to find contour that yields the max curvature of height vs contour.\n");
	GMT_explain_options (GMT, "RV");
	GMT_message (GMT, "\t-Z Subtract <shift> and then multiply data by <fact> before processing [1/0].\n");
	GMT_explain_options (GMT, "f.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdvolume_parse (struct GMTAPI_CTRL *C, struct GRDVOLUME_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdvolume and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0, n = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = TRUE;
				n = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->C.low, &Ctrl->C.high, &Ctrl->C.inc);
				if (n == 3) {
					n_errors += GMT_check_condition (GMT, Ctrl->C.low >= Ctrl->C.high || Ctrl->C.inc <= 0.0, "Syntax error -C option: high must exceed low and delta must be positive\n");
				}
				else
					Ctrl->C.high = Ctrl->C.low, Ctrl->C.inc = 1.0;	/* So calculation of ncontours will yield 1 */
				break;
			case 'L':
				Ctrl->L.active = TRUE;
				if (opt->arg[0]) Ctrl->L.value = atof (opt->arg);
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				Ctrl->S.unit = opt->arg[0];
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				switch (opt->arg[0]) {
					case 'c':
						Ctrl->T.mode = 1;	/* Find maximum in height curvatures */
						break;
					case '\0':
					case 'h':
						Ctrl->T.mode = 0;	/* Find maximum in height values */
						break;
					default:
						n_errors++;
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -T option: Append c or h [Default].\n");
				}
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				n_errors += GMT_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->Z.scale, &Ctrl->Z.offset) < 1, "Syntax error option -Z: Must specify <fact> and optionally <shift>\n");
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input grid file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.active && !(n == 1 || n == 3), "Syntax error option -C: Must specify 1 or 3 arguments\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && !(strchr (GMT_LEN_UNITS2, Ctrl->S.unit)), "Syntax error option -S: Must append one of %s\n", GMT_LEN_UNITS2_DISPLAY);
	n_errors += GMT_check_condition (GMT, Ctrl->L.active && GMT_is_dnan (Ctrl->L.value), "Syntax error option -L: Must specify base\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && !Ctrl->C.active, "Syntax error option -T: Must also specify -Clow/high/delta\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->C.active && GMT_IS_ZERO (Ctrl->C.high - Ctrl->C.low), "Syntax error option -T: Must specify -Clow/high/delta\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdvolume_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdvolume (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, bad, cut[4], ij, ij_inc[5];
	GMT_LONG row, col, c, k, pos, neg, nc, n_contours, new_grid = FALSE;

	double take_out, dv, da, cval = 0.0, cellsize, fact, dist_pr_deg, sum, out[4];
	double *area = NULL, *vol = NULL, *height = NULL, this_base, small, wesn[4];

	struct GRDVOLUME_CTRL *Ctrl = NULL;
	struct GMT_GRID *Grid = NULL, *Work = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdvolume_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdvolume_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdvolume", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRf", "", options))) Return (error);
	Ctrl = (struct GRDVOLUME_CTRL *) New_grdvolume_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdvolume_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdvolume main code ----------------------------*/

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
	if (Ctrl->L.active && Ctrl->L.value >= Grid->header->z_min) {
		GMT_report (GMT, GMT_MSG_FATAL, "Selected base value exceeds the minimum grid z value - aborting\n");
		Return (EXIT_FAILURE);
	}
	
	if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, Grid->header->wesn, 4, double);	/* No -R, use grid domain */
	GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);

	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->In.file), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	new_grid = GMT_set_outgrid (GMT, Grid, &Work);	/* TRUE if input is a read-only array */
	GMT_grd_init (GMT, Work->header, options, TRUE);

	/* Set node increments relative to the lower-left node of a 4-point box */
	ij_inc[0] = ij_inc[4] = 0;	ij_inc[1] = 1;	ij_inc[2] = 1 - Work->header->mx;	ij_inc[3] = -Work->header->mx;
	cellsize = Work->header->inc[GMT_X] * Work->header->inc[GMT_Y];
	if (Ctrl->S.active) {
		GMT_init_distaz (GMT, Ctrl->S.unit, 1, GMT_MAP_DIST);	/* Flat Earth mode */
		dist_pr_deg = GMT->current.proj.DIST_M_PR_DEG;
		dist_pr_deg *= GMT->current.map.dist[GMT_MAP_DIST].scale;	/* Scales meters to desired unit */
		cellsize *= dist_pr_deg * dist_pr_deg;
	}

	n_contours = (Ctrl->C.active) ? irint ((Ctrl->C.high - Ctrl->C.low) / Ctrl->C.inc) + 1 : 1;

	height = GMT_memory (GMT, NULL, n_contours, double);
	vol    = GMT_memory (GMT, NULL, n_contours, double);
	area   = GMT_memory (GMT, NULL, n_contours, double);

	if (!(Ctrl->Z.scale == 1.0 && Ctrl->Z.offset == 0.0)) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Subtracting %g and multiplying by %g\n", Ctrl->Z.offset, Ctrl->Z.scale);
		GMT_grd_do_scaling (GMT, Work->data, Work->header->size, Ctrl->Z.scale, Ctrl->Z.offset);
		Work->header->z_min = (Work->header->z_min - Ctrl->Z.offset) * Ctrl->Z.scale;
		Work->header->z_max = (Work->header->z_max - Ctrl->Z.offset) * Ctrl->Z.scale;
		if (Ctrl->Z.scale < 0.0) d_swap (Work->header->z_min, Work->header->z_max);
	}

	this_base = (Ctrl->L.active) ? Ctrl->L.value : 0.0;
	small = Ctrl->C.inc * 1.0e-6;

	for (c = 0; Ctrl->C.active && c < n_contours; c++) {	/* Trace contour, only count volumes inside contours */

		cval = Ctrl->C.low + c * Ctrl->C.inc;
		take_out = (c == 0) ? cval : Ctrl->C.inc;	/* Take out start contour the first time and just the increment subsequent times */

		GMT_report (GMT, GMT_MSG_NORMAL, "Compute volume, area, and average height for contour = %g\n", cval);
		
		for (ij = 0; ij < Work->header->size; ij++) {
			Work->data[ij] -= (float)take_out;		/* Take out the zero value */
			if (Work->data[ij] == 0.0) Work->data[ij] = (float)small;	/* But we dont want exactly zero, just + or - */
		}
		if (Ctrl->L.active) this_base -= take_out;

		if (Ctrl->L.active && this_base >= 0.0) {
			GMT_report (GMT, GMT_MSG_FATAL, "Base exceeds the current contour value - contour is ignored.\n");
			continue;
		}

		for (row = 1; row < Work->header->ny; row++) {

			dv = da = 0.0;	/* Reset these for each row */

			for (col = 0, ij = GMT_IJP (Work->header, row, 0); col < (Work->header->nx-1); col++, ij++) {

				/* Find if a contour goes through this bin */

				for (k = neg = pos = 0, bad = FALSE; !bad && k < 4; k++) {
					(Work->data[ij+ij_inc[k]] <= (float)small) ? neg++ : pos++;
					if (GMT_is_fnan (Work->data[ij+ij_inc[k]])) bad = TRUE;
				}

                                if (bad || neg == 4) continue;	/* Contour not crossing, go to next bin */

				if (pos == 4) {	/* Need entire prism */
					for (k = 0, sum = 0.0; k < 4; k++) sum += Work->data[ij+ij_inc[k]];
					dv += 0.25 * sum;
					da += 1.0;
				}
				else {	/* Need partial prisms */

					for (k = nc = 0; k < 4; k++) {	/* Check the 4 sides for crossings */
						cut[k] = FALSE;
						if ((Work->data[ij+ij_inc[k]] * Work->data[ij+ij_inc[k+1]]) < 0.0) nc++, cut[k] = TRUE;	/* Crossing this border */
					}
					if (nc < 2) continue;	/* Can happen if some nodes were 0 and then reset to small, thus passing the test */

					if (nc == 4) {	/* Saddle scenario */
						if (Work->data[ij] > 0.0) {	/* Need both SW and NE triangles */
							SW_triangle (Work, ij, TRUE, &dv, &da);
							NE_triangle (Work, ij, TRUE, &dv, &da);
						}
						else {			/* Need both SE and NW corners */
							SE_triangle (Work, ij, TRUE, &dv, &da);
							NW_triangle (Work, ij, TRUE, &dv, &da);
						}

					}
					else if (cut[0]) {	/* Contour enters at S border ... */
						if (cut[1])	/* and exits at E border */
							SE_triangle (Work, ij, (Work->data[ij+ij_inc[1]] > 0.0), &dv, &da);
						else if (cut[2])	/* or exits at N border */
							NS_trapezoid (Work, ij, Work->data[ij] < 0.0, &dv, &da);
						else			/* or exits at W border */
							SW_triangle (Work, ij, (Work->data[ij] > 0.0), &dv, &da);
					}
					else if (cut[1]) {	/* Contour enters at E border */
						if (cut[2])	/* exits at N border */
							NE_triangle (Work, ij, (Work->data[ij+ij_inc[2]] > 0.0), &dv, &da);
						else			/* or exits at W border */
							EW_trapezoid (Work, ij, Work->data[ij] < 0.0, &dv, &da);
					}
					else			/* Contours enters at N border and exits at W */
						NW_triangle (Work, ij, (Work->data[ij+ij_inc[3]] > 0.0), &dv, &da);
				}
			}
			ij++;

			fact = cellsize;
			/* Allow for shrinking of longitudes with latitude */
			if (Ctrl->S.active) fact *= cosd (Work->header->wesn[YHI] - (row+0.5) * Work->header->inc[GMT_Y]);

			vol[c]  += dv * fact;
			area[c] += da * fact;
		}

		/* Adjust for lower starting base */
		if (Ctrl->L.active) vol[c] -= area[c] * this_base;
	}
	if (!Ctrl->C.active) {	/* Since no contours we can use columns with bilinear tops to get the volume */
		for (row = 0; row < Work->header->ny; row++) {
			dv = da = 0.0;
			for (col = 0, ij = GMT_IJP (Work->header, row, 0); col < Work->header->nx; col++, ij++) {
				if (GMT_is_fnan (Work->data[ij])) continue;

				/* Half the leftmost and rightmost cell */
				if (Work->header->registration == GMT_GRIDLINE_REG && (col == 0 || col == Work->header->nx-1)) {
					dv += 0.5 * Work->data[ij];
					da += 0.5;
				}
				else {
					dv += Work->data[ij];
					da += 1.0;
				}
			}

			fact = cellsize;
			/* Allow for shrinking of longitudes with latitude */
			if (Ctrl->S.active) fact *= cosd (Work->header->wesn[YHI] - row * Work->header->inc[GMT_Y]);
			/* Half the top and bottom row */
			if (Work->header->registration == GMT_GRIDLINE_REG && (row == 0 || row == Work->header->ny-1)) fact *= 0.5;

			vol[0]  += dv * fact;
			area[0] += da * fact;
		}

		/* Adjust for lower starting base */
		if (Ctrl->L.active) vol[0] -= area[0] * this_base;
	}

	/* Compute average heights */

	for (c = 0; c < n_contours; c++) height[c] = (area[c] > 0.0) ? vol[c] / area[c] : GMT->session.d_NaN;

	/* Find the best contour that gives largest height */

	/* Print out final estimates */

 	if ((error = GMT_set_cols (GMT, GMT_OUT, 4))) Return (error);
 	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default output destination, unless already set */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);	/* Enables data output and sets access mode */

	if (Ctrl->T.active) {	/* Determine the best contour value and return the corresponding information for that contour only */
		c = ors_find_kink (GMT, height, n_contours, Ctrl->T.mode);
		out[0] = Ctrl->C.low + c * Ctrl->C.inc;	out[1] = area[c];	out[2] = vol[c];	out[3] = height[c];
		GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)out);	/* Write this to output */
	}
	else {			/* Return information for all contours (possibly one if -C<val> was used) */
		for (c = 0; c < n_contours; c++) {
			out[0] = Ctrl->C.low + c * Ctrl->C.inc;	out[1] = area[c];	out[2] = vol[c];	out[3] = height[c];
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, (void *)out);	/* Write this to output */
		}
	}
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_free (GMT, area);
	GMT_free (GMT, vol);
	GMT_free (GMT, height);

	Return (EXIT_SUCCESS);
}
