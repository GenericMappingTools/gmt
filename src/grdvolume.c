/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: grdvolume reads a 2d grid file, and calculates the volume
 * under the surface using exact integration of the bilinear interpolating
 * surface.  As an option, the user may supply a contour value; then the
 * volume is only integrated inside the chosen contour.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdvolume"
#define THIS_MODULE_MODERN_NAME	"grdvolume"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Calculate grid volume and area constrained by a contour"
#define THIS_MODULE_KEYS	"<G{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-RVfho"

struct GRDVOLUME_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct C {	/* -C */
		bool active;
		bool reverse, reverse_min;
		double low, high, inc;
	} C;
	struct L {	/* -L<base> */
		bool active;
		double value;
	} L;
	struct S {	/* -S */
		bool active;
		char unit;
	} S;
	struct T {	/* -T[c|z] */
		bool active;
		unsigned int mode;
	} T;
	struct Z {	/* Z<fact>[/<shift>] */
		bool active;
		double scale, offset;
	} Z;
};

/* This function returns the volume bounded by a trapezoid based on two vertical
 * lines x0 and x1 and two horizontal lines y0 = ax +b and y1 = cx + d
 */

GMT_LOCAL double vol_prism_frac_x (struct GMT_GRID *G, uint64_t ij, double x0, double x1, double a, double b, double c, double d) {
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

GMT_LOCAL double vol_prism_frac_y (struct GMT_GRID *G, uint64_t ij, double y0, double y1, double a, double b, double c, double d) {
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

GMT_LOCAL void SW_triangle (struct GMT_GRID *G, uint64_t ij, bool triangle, double *dv, double *da) {
	/* Calculates area of a SW-corner triangle */
	/* triangle = true gets triangle, false gives the complementary area */
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

GMT_LOCAL void NE_triangle (struct GMT_GRID *G, uint64_t ij, bool triangle, double *dv, double *da) {
	/* Calculates area of a NE-corner triangle */
	/* triangle = true gets triangle, false gives the complementary area */
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

GMT_LOCAL void SE_triangle (struct GMT_GRID *G, uint64_t ij, bool triangle, double *dv, double *da) {
	/* Calculates area of a SE-corner triangle */
	/* triangle = true gets triangle, false gives the complementary area */
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

GMT_LOCAL void NW_triangle (struct GMT_GRID *G, uint64_t ij, bool triangle, double *dv, double *da) {
	/* Calculates area of a NW-corner triangle */
	/* triangle = true gets triangle, false gives the complementary area */
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

GMT_LOCAL void NS_trapezoid (struct GMT_GRID *G, uint64_t ij, bool right, double *dv, double *da) {
	/* Calculates area of a NS trapezoid */
	/* right = true gets the right trapezoid, false gets the left */
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

GMT_LOCAL void EW_trapezoid (struct GMT_GRID *G, uint64_t ij, bool top, double *dv, double *da) {
	/* Calculates area of a EW trapezoid */
	/* top = true gets the top trapezoid, false gets the bottom */
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

GMT_LOCAL double median3 (double x[]) {
	/* Returns the median of the three points in x */
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

GMT_LOCAL int ors_find_kink (struct GMT_CTRL *GMT, double y[], unsigned int n, unsigned int mode) {
	/* mode: 0 = find value maximum, 1 = find curvature maximum */
	unsigned int i, im;
	double *c = NULL, *f = NULL;

	if (mode == 0) {	/* Find maximum value */
		for (i = im = 0; i < n; i++)
			if (y[i] > y[im]) im = i;
		return (im);
	}

	/* Calculate curvatures */

	c = gmt_M_memory (GMT, NULL, n, double);

	for (i = 1; i < (n-1); i++) c[i] = y[i+1] - 2.0 * y[i] + y[i-1];
	c[0] = c[1];
	if (n > 1) c[n-1] = c[n-2];

	/* Apply 3-point median filter to curvatures to mitigate noisy values */

	f = gmt_M_memory (GMT, NULL, n, double);
	for (i = 1; i < (n-1); i++) f[i] = median3 (&c[i-1]);

	/* Find maximum negative filtered curvature */

	for (i = im = 1; i < (n-1); i++) if (f[i] < f[im]) im = i;

	gmt_M_free (GMT, c);
	gmt_M_free (GMT, f);

	return (im);
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDVOLUME_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDVOLUME_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->L.value = GMT->session.d_NaN;
	C->S.unit = 'e';	/* Meter if geographic */
	C->Z.scale = 1.0;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDVOLUME_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <ingrid> [-C<cval> or -C<low>/<high>/<delta> or -Cr<low>/<high> or -Cr<cval>] [-L<base>]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-S<unit>] [-T[c|h]] [%s] [%s] [-Z<fact>[/<shift>]]\n\t[%s] [%s] [%s] [%s]\n\n",
		GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_ho_OPT, GMT_o_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is the name of the grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Find area, volume, and mean height inside the given <cval> contour,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   OR search using all contours from <low> to <high> in steps of <delta>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default returns area, volume and mean height of entire grid].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   OR append r (-Cr) to compute 'outside' area and volume between <low> and <high>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or below <cval> and grid's minimum.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Add volume from <base> up to contour [Default is from contour and up only].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S For geographic grids we convert degrees to \"Flat-Earth\" distances in meters.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append a unit from %s to select another distance unit.\n", GMT_LEN_UNITS2_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t-T (or -Th): Find the contour value that yields max average height (volume/area).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Tc to find contour that yields the max curvature of height vs contour.\n");
	GMT_Option (API, "R,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Subtract <shift> and then multiply data by <fact> before processing [1/0].\n");
	GMT_Option (API, "f,h,o,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDVOLUME_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdvolume and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int n = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)) != 0)
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = true;
				if (opt->arg[0] == 'r') {
					Ctrl->C.reverse = true;
					n = sscanf (&opt->arg[1], "%lf/%lf", &Ctrl->C.low, &Ctrl->C.high);
					if (n == 1) {		/* -Cr<cval> used */
						Ctrl->C.high = Ctrl->C.low;
						Ctrl->C.low = -DBL_MAX;
						Ctrl->C.reverse_min = true;
						n = 2;			/* To cheat the sanity test on -Cr option below */
					}
					n_errors += gmt_M_check_condition (GMT, Ctrl->C.low >= Ctrl->C.high,
					                                   "Syntax error -C option: high must exceed low\n");
					/* Now apply the trick that makes this option work. Swap and change signs of low/high */
					Ctrl->C.inc   = Ctrl->C.low;	/* Use inc as the buble sort tmp variable */
					Ctrl->C.low   = -Ctrl->C.high;
					Ctrl->C.high  = -Ctrl->C.inc;
					Ctrl->C.inc   = Ctrl->C.high - Ctrl->C.low;
					Ctrl->Z.scale = -1;
				}
				else {
					n = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->C.low, &Ctrl->C.high, &Ctrl->C.inc);
					if (n == 3) {
						n_errors += gmt_M_check_condition (GMT, Ctrl->C.low >= Ctrl->C.high || Ctrl->C.inc <= 0.0,
								"Syntax error -C option: high must exceed low and delta must be positive\n");
					}
					else
						Ctrl->C.high = Ctrl->C.low, Ctrl->C.inc = 1.0;	/* So calculation of ncontours will yield 1 */
				}
				break;
			case 'L':
				Ctrl->L.active = true;
				if (opt->arg[0]) Ctrl->L.value = atof (opt->arg);
				break;
			case 'S':
				Ctrl->S.active = true;
				Ctrl->S.unit = opt->arg[0];
				break;
			case 'T':
				Ctrl->T.active = true;
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
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Append c or h [Default].\n");
				}
				break;
			case 'Z':
				Ctrl->Z.active = true;
				n_errors += gmt_M_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->Z.scale, &Ctrl->Z.offset) < 1,
						"Syntax error option -Z: Must specify <fact> and optionally <shift>\n");
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && !Ctrl->C.reverse && !(n == 1 || n == 3),
	                                   "Syntax error option -C: Must specify 1 or 3 arguments\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.reverse && n != 2,
	                                   "Syntax error option -C: Must specify 2 arguments\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && !(strchr (GMT_LEN_UNITS2, Ctrl->S.unit)),
	                                   "Syntax error option -S: Must append one of %s\n", GMT_LEN_UNITS2_DISPLAY);
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && gmt_M_is_dnan (Ctrl->L.value),
	                                   "Syntax error option -L: Must specify base\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && !Ctrl->C.active,
	                                   "Syntax error option -T: Must also specify -Clow/high/delta\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->C.active && doubleAlmostEqualZero (Ctrl->C.high, Ctrl->C.low),
	                                   "Syntax error option -T: Must specify -Clow/high/delta\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdvolume (void *V_API, int mode, void *args) {
	bool bad, shrink, cut[4];
	int error = 0, ij_inc[5];
	unsigned int row, col, c, k, pos, neg, nc, n_contours;

	uint64_t ij;

	double take_out, dv, da, cval = 0.0, cellsize, fact, dist_pr_deg, sum, z_range, out[4];
	double *area = NULL, *vol = NULL, *height = NULL, this_base, small, wesn[4];

	struct GRDVOLUME_CTRL *Ctrl = NULL;
	struct GMT_GRID *Grid = NULL, *Work = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdvolume main code ----------------------------*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input grid\n");
	if ((Grid = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}
	shrink = gmt_M_is_geographic (GMT, GMT_IN);	/* Must deal with latitude-dependant area */

	if (Ctrl->S.active && !shrink) {	/* Not known to be geographic */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "-S requires a geographic grid.\n");
		if ((Grid->header->wesn[YLO] >= -90.0 && Grid->header->wesn[YHI] <= 90.0) &&
			(Grid->header->wesn[XLO] >= -360.0 && Grid->header->wesn[XHI] <= 720.0) &&
			(Grid->header->wesn[XHI] - Grid->header->wesn[XLO]) <= 360.0) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "However, your grid domain seems compatible with geographical grid domains.\n");
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "To make sure your Cartesian grid is recognized as geographical, use the -fg option.\n");
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Alternatively, use gmt grdedit -fg to bless it as a geographic grid first.\n");
		}
		else
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Yours grid is recognized as Cartesian.\n");
	
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->L.active && Ctrl->L.value >= Grid->header->z_min) {
		GMT_Report (API, GMT_MSG_NORMAL, "Selected base value exceeds the minimum grid z value - aborting\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if (!GMT->common.R.active[RSET]) gmt_M_memcpy (GMT->common.R.wesn, Grid->header->wesn, 4, double);	/* No -R, use grid domain */
	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);

	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, Grid) == NULL) {
		Return (API->error);
	}

	if (Ctrl->C.reverse_min) {		/* Only now we know the min value to use in the -Cr<cval> option */
		Ctrl->C.high = -Grid->header->z_min;
		Ctrl->C.inc  = Ctrl->C.high - Ctrl->C.low;
		if (Ctrl->C.high <= Ctrl->C.low) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Cr<cval> option: <cval> must exceed grid's minimum.\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	(void) gmt_set_outgrid (GMT, Ctrl->In.file, false, Grid, &Work);	/* true if input is a read-only array */
	gmt_grd_init (GMT, Work->header, options, true);

	/* Set node increments relative to the lower-left node of a 4-point box */
	gmt_grd_set_ij_inc (GMT, Work->header->mx, ij_inc);
	ij_inc[4] = ij_inc[0];	/* Repeat for convenience */
	cellsize = Work->header->inc[GMT_X] * Work->header->inc[GMT_Y];
	if (shrink) {
		gmt_init_distaz (GMT, Ctrl->S.unit, 1, GMT_MAP_DIST);	/* Flat Earth mode */
		dist_pr_deg = GMT->current.proj.DIST_M_PR_DEG;
		dist_pr_deg *= GMT->current.map.dist[GMT_MAP_DIST].scale;	/* Scales meters to desired unit */
		cellsize *= dist_pr_deg * dist_pr_deg;
	}

	n_contours = (Ctrl->C.active) ? urint ((Ctrl->C.high - Ctrl->C.low) / Ctrl->C.inc) + 1U : 1U;

	height = gmt_M_memory (GMT, NULL, n_contours, double);
	vol    = gmt_M_memory (GMT, NULL, n_contours, double);
	area   = gmt_M_memory (GMT, NULL, n_contours, double);

	if (!(Ctrl->Z.scale == 1.0 && Ctrl->Z.offset == 0.0)) {
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Subtracting %g and multiplying by %g\n", Ctrl->Z.offset, Ctrl->Z.scale);
		Work->header->z_min = (Work->header->z_min - Ctrl->Z.offset) * Ctrl->Z.scale;
		Work->header->z_max = (Work->header->z_max - Ctrl->Z.offset) * Ctrl->Z.scale;
		if (Ctrl->Z.scale < 0.0) gmt_M_double_swap (Work->header->z_min, Work->header->z_max);
		/* Since gmt_scale_and_offset_f applies z' = z * scale + offset we must adjust Z.offset first: */
		Ctrl->Z.offset *= Ctrl->Z.scale;
		gmt_scale_and_offset_f (GMT, Work->data, Work->header->size, Ctrl->Z.scale, Ctrl->Z.offset);
	}

	this_base = (Ctrl->L.active) ? Ctrl->L.value : 0.0;
	z_range = Work->header->z_max - Work->header->z_min;
	if (n_contours == 1 || Ctrl->C.inc == 0.0)
		small = z_range * 1.0e-6;	/* Our gmt_grdfloat noise threshold */
	else
		small = MIN (fabs (Ctrl->C.inc), z_range) * 1.0e-6;	/* Our gmt_grdfloat noise threshold */
	GMT_Report (API, GMT_MSG_DEBUG, "Small noise value = %g\n", small);
	for (c = 0; Ctrl->C.active && c < n_contours; c++) {	/* Trace contour, only count volumes inside contours */

		cval = Ctrl->C.low + c * Ctrl->C.inc;
		take_out = (c == 0) ? cval + small : Ctrl->C.inc;	/* Take out start contour the first time and just the increment subsequent times */

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Compute volume, area, and average height for contour = %g\n", cval);
	
		for (ij = 0; ij < Work->header->size; ij++) {
			Work->data[ij] -= (gmt_grdfloat)take_out;		/* Take out the zero value */
			if (Work->data[ij] == 0.0) Work->data[ij] = (gmt_grdfloat)small;	/* But we don't want exactly zero, just + or - */
		}
		if (Ctrl->L.active) this_base -= take_out;

		if (Ctrl->L.active && this_base >= 0.0) {
			GMT_Report (API, GMT_MSG_NORMAL, "Base exceeds the current contour value - contour is ignored.\n");
			continue;
		}

		for (row = 1; row < Work->header->n_rows; row++) {

			dv = da = 0.0;	/* Reset these for each row */

			for (col = 0, ij = gmt_M_ijp (Work->header, row, 0); col < (Work->header->n_columns-1); col++, ij++) {

				/* Find if a contour goes through this bin */

				for (k = neg = pos = 0, bad = false; !bad && k < 4; k++) {
					(Work->data[ij+ij_inc[k]] <= (gmt_grdfloat)small) ? neg++ : pos++;
					if (gmt_M_is_fnan (Work->data[ij+ij_inc[k]])) bad = true;
				}

				if (bad || neg == 4) continue;	/* Contour not crossing, go to next bin */

				if (pos == 4) {	/* Need entire prism */
					for (k = 0, sum = 0.0; k < 4; k++) sum += Work->data[ij+ij_inc[k]];
					dv += 0.25 * sum;
					da += 1.0;
				}
				else {	/* Need partial prisms */

					for (k = nc = 0; k < 4; k++) {	/* Check the 4 sides for crossings */
						cut[k] = false;
						if ((Work->data[ij+ij_inc[k]] * Work->data[ij+ij_inc[k+1]]) < 0.0) nc++, cut[k] = true;	/* Crossing this border */
					}
					if (nc < 2) continue;	/* Can happen if some nodes were 0 and then reset to small, thus passing the test */

					if (nc == 4) {	/* Saddle scenario */
						if (Work->data[ij] > 0.0) {	/* Need both SW and NE triangles */
							SW_triangle (Work, ij, true, &dv, &da);
							NE_triangle (Work, ij, true, &dv, &da);
						}
						else {			/* Need both SE and NW corners */
							SE_triangle (Work, ij, true, &dv, &da);
							NW_triangle (Work, ij, true, &dv, &da);
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
			if (shrink) fact *= cosd (Work->header->wesn[YHI] - (row+0.5) * Work->header->inc[GMT_Y]);

			vol[c]  += dv * fact;
			area[c] += da * fact;
		}

		/* Adjust for lower starting base */
		if (Ctrl->L.active) vol[c] -= area[c] * this_base;
	}
	if (!Ctrl->C.active) {	/* Since no contours we can use columns with bilinear tops to get the volume */
		for (row = 0; row < Work->header->n_rows; row++) {
			dv = da = 0.0;
			for (col = 0, ij = gmt_M_ijp (Work->header, row, 0); col < Work->header->n_columns; col++, ij++) {
				if (gmt_M_is_fnan (Work->data[ij])) continue;

				/* Half the leftmost and rightmost cell */
				if (Work->header->registration == GMT_GRID_NODE_REG && (col == 0 || col == Work->header->n_columns-1)) {
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
			if (shrink) fact *= cosd (Work->header->wesn[YHI] - row * Work->header->inc[GMT_Y]);
			/* Half the top and bottom row */
			if (Work->header->registration == GMT_GRID_NODE_REG && (row == 0 || row == Work->header->n_rows-1)) fact *= 0.5;

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

	if ((error = GMT_Set_Columns (API, GMT_OUT, 4, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		gmt_M_free (GMT, area);		gmt_M_free (GMT, vol);		gmt_M_free (GMT, height);
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
		gmt_M_free (GMT, area);		gmt_M_free (GMT, vol);		gmt_M_free (GMT, height);
		Return (API->error);
	}
	if (GMT->common.h.add_colnames) {
		char header[GMT_LEN64] = {""};
		sprintf (header, "#contour\tarea\tvolume\theight");
		if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, NULL)) {
			gmt_M_free (GMT, area);		gmt_M_free (GMT, vol);		gmt_M_free (GMT, height);
			Return (API->error);
		}
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		gmt_M_free (GMT, area);		gmt_M_free (GMT, vol);		gmt_M_free (GMT, height);
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_NONE) != GMT_NOERROR) {	/* Sets output geometry */
		gmt_M_free (GMT, area);		gmt_M_free (GMT, vol);		gmt_M_free (GMT, height);
		Return (API->error);
	}

	gmt_set_cartesian (GMT, GMT_OUT);	/* Since no coordinates are written */
	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */

	if (Ctrl->T.active) {	/* Determine the best contour value and return the corresponding information for that contour only */
		c = ors_find_kink (GMT, height, n_contours, Ctrl->T.mode);
		out[0] = Ctrl->C.low + c * Ctrl->C.inc;	out[1] = area[c];	out[2] = vol[c];	out[3] = height[c];
		GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
	}
	else {			/* Return information for all contours (possibly one if -C<val> was used) */
		if (Ctrl->C.reverse) {
			out[0] = 0;	out[1] = area[0] - area[1];	out[2] = vol[0] - vol[1];	out[3] = out[2] / out[1];
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
		}
		else {
			for (c = 0; c < n_contours; c++) {
				out[0] = Ctrl->C.low + c * Ctrl->C.inc;	out[1] = area[c];	out[2] = vol[c];	out[3] = height[c];
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
			}
		}
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		gmt_M_free (GMT, area);		gmt_M_free (GMT, vol);
		gmt_M_free (GMT, height);	gmt_M_free (GMT, Out);
		Return (API->error);
	}

	gmt_M_free (GMT, area);
	gmt_M_free (GMT, vol);
	gmt_M_free (GMT, height);
	gmt_M_free (GMT, Out);

	Return (GMT_NOERROR);
}
