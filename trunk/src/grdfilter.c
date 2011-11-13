/*--------------------------------------------------------------------
 *	$Id$
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
 * Brief synopsis: grdfilter.c reads a grid file and creates filtered grid file.
 * User selects boxcar, gaussian, cosine arch, median, mode or spatial filters,
 * and sets distance scaling as appropriate for map work, etc.
 *
 * Author:	W.H.F. Smith
 * Date: 	1-JAN-2010
 * Version:	5 API
*/

#define GMT_WITH_NO_PS
#include "gmt.h"

EXTERN_MSC double GMT_great_circle_dist_meter (struct GMT_CTRL *C, double x0, double y0, double x1, double y1);

struct GRDFILTER_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
#ifdef DEBUG
	struct A {	/* -A<a|r|w|c>row/col */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG ROW, COL;
		double x, y;
	} A;
#endif
	struct D {	/* -D<distflag> */
		GMT_LONG active;
		GMT_LONG mode;
	} D;
	struct F {	/* <type>[-]<filter_width>[/<width2>][<mode>] */
		GMT_LONG active;
		GMT_LONG highpass;
		char filter;	/* Character codes for the filter */
		double width, width2, quantile;
		GMT_LONG rect;
		GMT_LONG mode;
	} F;
	struct G {	/* -G<file> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
		char string[GMT_TEXT_LEN256];
	} I;
	struct N {	/* -Np|i|r */
		GMT_LONG active;
		GMT_LONG mode;	/* 0 is default (i), 1 is replace (r), 2 is preserve (p) */
	} N;
	struct T {	/* -T */
		GMT_LONG active;
	} T;
#ifdef DEBUG
	struct W {	/* -W */
		GMT_LONG active;
		char *file;
	} W;
#endif
};

/* Forward/Inverse Spherical Mercator coordinate transforms */
#define IMG2LAT(img) (2.0*atand(exp((img)*D2R))-90.0)
#define LAT2IMG(lat) (R2D*log(tand(0.5*((lat)+90.0))))
/* Local ij calculation for weight matrix */
#define WT_IJ(F,jj,ii) ((jj) + F.y_half_width) * F.nx + (ii) + F.x_half_width

#define GRDFILTER_N_PARS	7
#define GRDFILTER_WIDTH		0
#define GRDFILTER_HALF_WIDTH	1
#define GRDFILTER_X_SCALE	2
#define GRDFILTER_Y_SCALE	3
#define GRDFILTER_INV_R_SCALE	4
#define GRDFILTER_X_DIST	5
#define GRDFILTER_Y_DIST	6

#define GRDFILTER_N_FILTERS	9

#define NAN_IGNORE	0
#define NAN_REPLACE	1
#define NAN_PRESERVE	2

struct FILTER_INFO {
	GMT_LONG nx;		/* The max number of filter weights in x-direction */
	GMT_LONG ny;		/* The max number of filter weights in y-direction */
	GMT_LONG x_half_width;	/* Number of filter nodes to either side needed at this latitude */
	GMT_LONG y_half_width;	/* Number of filter nodes above/below this point (ny_f/2) */
	GMT_LONG d_flag;
	GMT_LONG rect;		/* For 2-D rectangular filtering */
	GMT_LONG debug;		/* Normally unused except under DEBUG */
	double dx, dy;		/* Grid spacing in original units */
	double y_min, y_max;	/* Grid limits in y(lat) */
	double *x, *y;		/* Distances in original units along x and y to distance nodes */
	double par[5];		/* [0] is filter width, [1] is 0.5*filter_width, [2] is xscale, [3] is yscale, [4] is 1/r_half for filter */
	double x_off, y_off;	/* Offsets relative to original grid */
	char *visit;		/* BOOLEAN array to keep track of which longitude nodes we have already visited once */
	PFD weight_func, radius_func;
};

void *New_grdfilter_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDFILTER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDFILTER_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->D.mode = -1;	
	C->F.quantile = 0.5;	/* Default is median */	
	return (C);
}

void Free_grdfilter_Ctrl (struct GMT_CTRL *GMT, struct GRDFILTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->G.file) free (C->G.file);	
#ifdef DEBUG
	if (C->W.file) free (C->W.file);	
#endif
	GMT_free (GMT, C);	
}

void set_weight_matrix (struct GMT_CTRL *GMT, struct FILTER_INFO *F, double *weight, double output_lat, double par[], double x_off, double y_off)
{
	/* x_off and y_off give offset between output node and 'origin' input node for this window (0,0 for integral grids).
	 * fast is TRUE when input/output grids are offset by integer values in dx/dy.
	 * Here, par[0] = filter_width, par[1] = filter_width / 2, par[2] = x_scale, part[3] = y_scale, and
	 * par[4] is the normalization distance needed for the Cosine-bell or Gaussian weight function.
	 */

	GMT_LONG i, j, ij;
	double x, y, yc, y0, r, ry = 0.0;

	yc = y0 = output_lat - y_off;		/* Input latitude of central point (i,j) = (0,0) */
	if (F->d_flag == 5) yc = IMG2LAT (yc);	/* Recover actual latitude in IMG grid at this center point */
	for (j = -F->y_half_width; j <= F->y_half_width; j++) {
		y = y0 + ((j < 0) ? F->y[-j] : -F->y[j]);	/* y or latitude at this row */
		if (F->d_flag > 2 && (y < F->y_min || y > F->y_max)) {		/* This filter row is outside input grid domain */
			for (i = -F->x_half_width, ij = (j + F->y_half_width) * F->nx; i <= F->x_half_width; i++, ij++) weight[ij] = -1.0;
			continue;	/* Done with this row */
		}
		if (F->d_flag == 5) y = IMG2LAT (y);	/* Recover actual latitudes */
		if (F->rect) ry = (double)j / (double)F->y_half_width;	/* -1 to +1 */
		for (i = -F->x_half_width; i <= F->x_half_width; i++) {
			x = (i < 0) ? -F->x[-i] : F->x[i];
			ij = (j + F->y_half_width) * F->nx + i + F->x_half_width;
			if (F->rect) {	/* 2-D rectangular filtering; radius not used as we use x/F->x_half_width and ry instead */
				weight[ij] = F->weight_func ((double)i/(double)F->x_half_width, par) * F->weight_func (ry, par);
			}
			else {
				r = F->radius_func (GMT, x_off, yc, x, y, par);
				weight[ij] = (r > par[GRDFILTER_HALF_WIDTH]) ? -1.0 : F->weight_func (r, par);
#ifdef DEBUG
				if (F->debug) weight[ij] = (r > par[GRDFILTER_HALF_WIDTH]) ? -1.0 : r;
#endif
			}
		}
	}
}

/* Various functions that will be accessed via pointers */
double CartRadius (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1, double par[])
{	/* Plain Cartesian distance (par is not used) */
	return (hypot (x0 - x1, y0 - y1));
}

double CartScaledRadius (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1, double par[])
{	/* Plain scaled Cartesian distance (xscale = yscale) */
	return (par[GRDFILTER_X_SCALE] * hypot (x0 - x1, y0 - y1));
}

double CartScaledRect (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1, double par[])
{	/* Pass dx,dy via par[GRDFILTER_X|Y_DIST] and return a r that is either in or out */
	double r;
	par[GRDFILTER_X_DIST] = par[GRDFILTER_X_SCALE] * (x0 - x1);
	par[GRDFILTER_Y_DIST] = par[GRDFILTER_Y_SCALE] * (y0 - y1);
	r = (fabs (par[GRDFILTER_X_DIST]) > par[GRDFILTER_HALF_WIDTH] || fabs (par[GRDFILTER_Y_DIST]) > par[GRDFILTER_HALF_WIDTH]) ? 2.0 : 0.0;
	return (r);
}

double FlatEarthRadius (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1, double par[])
{	/* Cartesian radius with different scales */
	return (hypot (par[GRDFILTER_X_SCALE] * (x0 - x1), par[GRDFILTER_Y_SCALE] * (y0 - y1)));
}

double SphericalRadius (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1, double par[])
{	/* Great circle distance in km with polar wrap-around test on 2nd point */
	if (fabs (y1) > 90.0) {	/* Must find the point across the pole */
		y1 = copysign (180.0 - fabs (y1), y1);
		x1 += 180.0;
	}
	return (0.001 * GMT_great_circle_dist_meter (GMT, x0, y0, x1, y1));
}

double UnitWeight (double r, double par[])
{	/* Return unit weight since we know r is inside radius (par is not used) */
	return (1.0);
}

double CosBellWeight (double r, double par[])
{	/* Return the cosine-bell filter weight for given r.
	 * The parameter r_f_half is the 5th parameter passed via par.
	 */

	return (1.0 + cos (M_PI * r * par[GRDFILTER_INV_R_SCALE]));
}

double GaussianWeight (double r, double par[])
{	/* Return the Gaussian filter weight for given r.
	 * The parameter sig_2 is the 5th parameter passed via par.
	 */

	return (exp (r * r * par[GRDFILTER_INV_R_SCALE]));
}

GMT_LONG init_area_weights (struct GMT_CTRL *GMT, struct GMT_GRID *G, GMT_LONG mode, struct GMT_GRID *A, char *file)
{
	/* Precalculate the area weight of each node.  There are several considerations:
	 * 1. Mercator img grids (d_flag == 5) has irregular latitude spacing so we
	 *    must compute the n and s latitudes for each cell to get area.
	 * 2. Poles for grid-registered grids require a separate weight formula
	 * 3. Grid-registered grids have boundary nodes that only apply to 1/2 the area
	 *    (and the four corners (unless poles) only 1/4 the area of other cells).
	 */
	GMT_LONG row, col, ij;
	double row_weight, col_weight, dy_half = 0.0, dx, y, lat, lat_s, lat_n, s2 = 0.0;
	
	/* Based the grid on the input grid domain and increments. */
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, A, G->header->wesn, G->header->inc, G->header->registration), "");
	A->data = GMT_memory (GMT, NULL, A->header->size, float);
	
	if (mode) {	/* Geographic data */
		if (mode == 5) dy_half = 0.5 * A->header->inc[GMT_Y];	/* Half img y-spacing */
		dx = A->header->inc[GMT_X] * R2D;			/* Longitude increment in radians */
		s2 = sind (0.5 * A->header->inc[GMT_Y]);		/* Holds sin (del_y/2) */
	}
	else
		dx = A->header->inc[GMT_X];
	GMT_row_loop (GMT, A, row) {
		if (mode == 5) {		/* Adjust lat if IMG grid.  Note: these grids do not reach a pole. */
			y = GMT_grd_row_to_y (GMT, row, A->header);	/* Current input Merc y */
			lat = IMG2LAT (y);			 /* Get actual latitude */
			lat_s = IMG2LAT (y - dy_half);		/* Bottom grid cell latitude */
			lat_n = IMG2LAT (y + dy_half);		/* Top grid cell latitude */
			row_weight = sind (lat_n) - sind (lat_s);
		}
		else if (mode) {	/* Geographic data, watch for poles */
			lat = GMT_grd_row_to_y (GMT, row, A->header);	/* Current input latitude */
			if (GMT_IS_POLE (lat))	/* Poles are different */
				row_weight = 1.0 - cosd (0.5 * A->header->inc[GMT_Y]);
			else {	/* All other points */
				row_weight = 2.0 * cosd (lat) * s2;
				/* Note: No need for special weight-sharing between w/e gridline-reg grids since we explicitly only use the west node */
			}
		}
		else {	/* If not geographic then row_weight is a constant 1 except for gridline-registered grids at the ends */
			row_weight = (A->header->registration == GMT_GRIDLINE_REG && (row == 0 || row == (A->header->ny-1))) ? 0.5 : 1.0;	/* Share weight with repeat point */
			row_weight *= A->header->inc[GMT_Y];
		}

		GMT_col_loop (GMT, A, row, col, ij) {
			col_weight = dx * ((A->header->registration == GMT_GRIDLINE_REG && (col == 0 || col == (A->header->nx-1))) ? 0.5  : 1.0);
			A->data[ij] = (float)(row_weight * col_weight);
		}
	}
#ifdef DEBUG
	if (file) {	/* For debug purposes: Save the area weight grid */
		GMT_LONG error;
		if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, file, A) != GMT_OK) return (API->error);
	}
#endif
	return (GMT_NOERROR);
}

GMT_LONG GMT_grdfilter_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdfilter %s [API] %s - Filter a grid in the space (or time) domain\n\n", GMT_VERSION, GMT_MP);
	GMT_message (GMT, "usage: grdfilter <ingrid> -D<distance_flag> -F<type>[-]<filter_width>[/<width2>][<mode>] -G<outgrid>\n");
	GMT_message (GMT, "\t[%s] [-Ni|p|r] [%s]\n\t[-T] [%s] [%s]\n", GMT_I_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<ingrid> is the input grid file to be filtered.\n");
	GMT_message (GMT, "\t-D Distance flag determines how grid (x,y) maps into distance units of filter width as follows:\n");
	GMT_message (GMT, "\t   Cartesian (x, y) Data:\n");
	GMT_message (GMT, "\t     -Dp grid x,y with <filter_width> in pixels (must be an odd number), Cartesian distances.\n");
	GMT_message (GMT, "\t     -D0 grid x,y same units as <filter_width>, Cartesian distances.\n");
	GMT_message (GMT, "\t   Geographic (lon,lat) Data:\n");
	GMT_message (GMT, "\t     -D1 grid x,y in degrees, <filter_width> in km, Cartesian distances.\n");
	GMT_message (GMT, "\t     -D2 grid x,y in degrees, <filter_width> in km, x_scaled by cos(middle y), Cartesian distances.\n");
	GMT_message (GMT, "\t   These first three options are faster; they allow weight matrix to be computed only once.\n");
	GMT_message (GMT, "\t   Next three options are slower; weights must be recomputed for each scan line.\n");
	GMT_message (GMT, "\t     -D3 grid x,y in degrees, <filter_width> in km, x_scale varies as cos(y), Cartesian distances.\n");
	GMT_message (GMT, "\t     -D4 grid x,y in degrees, <filter_width> in km, spherical distances.\n");
	GMT_message (GMT, "\t     -D5 grid x,y in Mercator units (-Jm1), <filter_width> in km, spherical distances.\n");
	GMT_message (GMT, "\t-F Set the low-pass filter type and full diameter (6 sigma) filter-width.\n");
	GMT_message (GMT, "\t   Choose between convolution-type filters which differ in how weights are assigned\n");
	GMT_message (GMT, "\t   and geospatial filters that seek to return a representative value.\n");
	GMT_message (GMT, "\t   Give a negative filter width to select highpass filtering [lowpass].\n");
	GMT_message (GMT, "\t   Filters are isotropic.  For rectangular filtering append /<width2> (requires -Dp|0).\n");
	GMT_message (GMT, "\t   Convolution filters:\n");
	GMT_message (GMT, "\t     b: Boxcar : a simple averaging of all points inside filter domain.\n");
	GMT_message (GMT, "\t     c: Cosine arch : a weighted averaging with cosine arc weights.\n");
	GMT_message (GMT, "\t     g: Gaussian : weighted averaging with Gaussian weights.\n");
	GMT_message (GMT, "\t   Geospatial filters:\n");
	GMT_message (GMT, "\t     l: Lower : return minimum of all points.\n");
	GMT_message (GMT, "\t     L: Lower+ : return minimum of all +ve points.\n");
	GMT_message (GMT, "\t     m: Median : return the median (50%% quantile) value of all points.\n");
	GMT_message (GMT, "\t        Append +q<quantile> to select another quantile (in 0-1 range) [0.5].\n");
	GMT_message (GMT, "\t     p: Maximum likelihood probability estimator : return mode of all points.\n");
	GMT_message (GMT, "\t        By default, we return the average if more than one mode is found.\n");
	GMT_message (GMT, "\t        Append - or + to the width to instead return the smallest or largest mode.\n");
	GMT_message (GMT, "\t     u: Upper : return maximum of all points.\n");
	GMT_message (GMT, "\t     U: Upper- : return maximum of all -ve points.\n");
	GMT_message (GMT, "\t-G Set output filename for filtered grid.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
#ifdef DEBUG
	GMT_message (GMT, "\t-A DEBUG: Use -A<mode><lon/<lat> to instead save filter specifics at that point. Choose:\n");
	GMT_message (GMT, "\t   mode as a (area weights), c (composite weight), r (radii), or w (filter weight).\n");
	GMT_message (GMT, "\t-W DEBUG: Save area weigths to <file> [no save]\n");
#endif
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_message (GMT, "\t   The new xinc and yinc should be divisible by the old ones (new lattice is subset of old).\n");
	GMT_message (GMT, "\t-N Specify how NaNs in the input grid should be treated.  There are three options:\n");
	GMT_message (GMT, "\t   -Ni skips all NaN values and returns a filtered value unless all are NaN [Default].\n");
	GMT_message (GMT, "\t   -Np sets filtered output to NaN is any NaNs are found inside filter circle.\n");
	GMT_message (GMT, "\t   -Nr sets filtered output to NaN if the corresponding input node was NaN.\n");
	GMT_message (GMT, "\t      (only possible if the input and output grids are coregistered).\n");
	GMT_message (GMT, "\t-T Toggle between grid and pixel registration for output grid [Default is same as input registration].\n");
	GMT_message (GMT, "\t-R For new Range of output grid; enter <WESN> (xmin, xmax, ymin, ymax) separated by slashes.\n");
	GMT_message (GMT, "\t   [Default uses the same region as the input grid].\n");
	GMT_explain_options (GMT, "Vf.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdfilter_parse (struct GMTAPI_CTRL *C, struct GRDFILTER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdfilter and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	char c, a[GMT_TEXT_LEN64], b[GMT_TEXT_LEN64], txt[GMT_TEXT_LEN64], *p = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

#ifdef DEBUG
			case 'A':	/* Distance mode */
				Ctrl->A.active = TRUE;
				Ctrl->A.mode = opt->arg[0];
				sscanf (&opt->arg[1], "%[^/]/%s", a, b);
				Ctrl->A.x = atof (a);
				Ctrl->A.y = atof (b);
				break;
#endif
			case 'D':	/* Distance mode */
				Ctrl->D.active = TRUE;
				Ctrl->D.mode = (opt->arg[0] == 'p') ? -1 : atoi (opt->arg);
				break;
			case 'F':	/* Filter */
				if (strchr ("bcgmpLlUu", opt->arg[0])) {	/* OK filter code */
					Ctrl->F.active = TRUE;
					Ctrl->F.filter = opt->arg[0];
					strcpy (txt, opt->arg);	/* Work on a copy */
					if (Ctrl->F.filter == 'm') {
						if ((p = strchr (txt, 'q'))) {	/* Requested another quantile */
							*(--p) = 0;	/* Chop off the +q modifier */
							Ctrl->F.quantile = atof (p+2);
						}
					}
					if (strchr (txt, '/')) {	/* Gave xwidth/ywidth for rectangular Cartesian filtering */
						sscanf (&txt[1], "%[^/]/%s", a, b);
						Ctrl->F.width = atof (a);
						Ctrl->F.width2 = atof (b);
						Ctrl->F.rect = TRUE;
					}
					else
						Ctrl->F.width = atof (&txt[1]);
					if (Ctrl->F.width < 0.0) Ctrl->F.highpass = TRUE;
					Ctrl->F.width = fabs (Ctrl->F.width);
					if (Ctrl->F.filter == 'p') {	/* Check for some further info in case of mode filtering */
						c = opt->arg[strlen(txt)-1];
						if (c == '-') Ctrl->F.mode = -1;
						if (c == '+') Ctrl->F.mode = +1;
					}
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Expected -Fx<width>, where x is one of bcgmplLuU\n");
					n_errors++;
				}
				break;
			case 'G':	/* Output file */
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':	/* New grid spacings */
				Ctrl->I.active = TRUE;
				strcpy (Ctrl->I.string, opt->arg);	/* Verbatim copy */
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'N':	/* Treatment of NaNs */
				Ctrl->N.active = TRUE;
				switch (opt->arg[0]) {
					case 'i':
						Ctrl->N.mode = NAN_IGNORE;	/* Default */
						break;
					case 'r':
						Ctrl->N.mode = NAN_REPLACE;	/* Replace */
						break;
					case 'p':
						Ctrl->N.mode = NAN_PRESERVE;	/* Preserve */
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Expected -Ni|p|r\n");
						n_errors++;
						break;
				}
				break;
			case 'T':	/* Toggle registration */
				Ctrl->T.active = TRUE;
				break;
#ifdef DEBUG
			case 'W':	/* Area weight debug file */
				Ctrl->W.active = TRUE;
				Ctrl->W.file = strdup (opt->arg);
				break;
#endif
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	GMT_check_lattice (GMT, Ctrl->I.inc, NULL, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->D.active, "Syntax error -D option: Choose from p or 0-5\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.mode < -1 || Ctrl->D.mode > 5, "Syntax error -D option: Choose from p or 0-5\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.mode > 0 && Ctrl->F.rect, "Syntax error -F option: Rectangular Cartesian filtering requires -Dp|0\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->F.active, "Syntax error: -F option is required:\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.quantile < 0.0 || Ctrl->F.quantile > 1.0 , "Syntax error: The quantile must be in the 0-1 range.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.active && Ctrl->F.width == 0.0, "Syntax error -F option: filter fullwidth must be nonzero.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.rect && Ctrl->F.width2 <= 0.0, "Syntax error -F option: Rectangular y-width filter must be nonzero.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0), "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, GMT->common.R.active && Ctrl->I.active && Ctrl->F.highpass, "Syntax error -F option: Highpass filtering requires original -R -I\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdfilter_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_grdfilter (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG n_in_median, n_nan = 0, tid = 0, spherical = FALSE, full_360;
	GMT_LONG j_origin, col_out, row_out, nx_wrap = 0, visit_check = FALSE;
	GMT_LONG col_in, row_in, ii, jj, i, j, ij_in, ij_out, ij_wt, effort_level, go_on;
	GMT_LONG filter_type, one_or_zero = 1, GMT_n_multiples = 0, *i_origin = NULL;
	GMT_LONG error = FALSE, fast_way, slow = FALSE, slower = FALSE, same_grid = FALSE;
#ifdef DEBUG
	GMT_LONG n_conv = 0;
#endif
	double x_scale = 1.0, y_scale = 1.0, x_width, y_width, y, par[GRDFILTER_N_PARS];
	double x_out, y_out, wt_sum, value, last_median = 0.0, this_median = 0.;
	double y_shift = 0.0, x_fix = 0.0, y_fix = 0.0, max_lat, lat_out, w;
	double merc_range, *weight = NULL, *work_array = NULL, *x_shift = NULL;

	char filter_code[GRDFILTER_N_FILTERS] = {'b', 'c', 'g', 'm', 'p', 'l', 'L', 'u', 'U'};
	char *filter_name[GRDFILTER_N_FILTERS+2] = {"Boxcar", "Cosine Arch", "Gaussian", "Median", "Mode", "Lower", \
		"Lower+", "Upper", "Upper-", "Spherical Median", "Spherical Mode"};

	fpair *work_data = NULL;
	
	struct GMT_GRID *Gin = NULL, *Gout = NULL, *A = NULL, *L = NULL;
	struct FILTER_INFO F;
	struct GRDFILTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	if ((options = GMT_Prep_Options (API, mode, args)) == NULL) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_grdfilter_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_grdfilter_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdfilter", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-VRf:", "", options)) Return (API->error);
	Ctrl = New_grdfilter_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdfilter_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdfilter main code ----------------------------*/

	if ((Gin = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, Ctrl->In.file, NULL)) == NULL) {	/* Get entire grid */
		Return (API->error);
	}

	if (Ctrl->T.active)	/* Make output grid of the opposite registration */
		one_or_zero = (Gin->header->registration == GMT_PIXEL_REG) ? 1 : 0;
	else
		one_or_zero = (Gin->header->registration == GMT_PIXEL_REG) ? 0 : 1;

	full_360 = (Ctrl->D.mode && GMT_360_RANGE (Gin->header->wesn[XHI], Gin->header->wesn[XLO]));	/* Periodic geographic grid */

	if (Ctrl->D.mode == -1) {	/* Special case where widths are given in pixels */
		if (!GMT_IS_ZERO (fmod (Ctrl->F.width, 2.0) - 1.0)) {
			GMT_report (GMT, GMT_MSG_FATAL, "ERROR: -Dp requires filter width given as an odd number of pixels\n");
			Return (EXIT_FAILURE);
		}
		Ctrl->F.width *= Gin->header->inc[GMT_X];	/* Scale up to give width */
		if (Ctrl->F.rect) {
			if (!GMT_IS_ZERO (fmod (Ctrl->F.width2, 2.0) - 1.0)) {
				GMT_report (GMT, GMT_MSG_FATAL, "ERROR: -Dp requires filter width given as an odd number of pixels\n");
				Return (EXIT_FAILURE);
			}
			Ctrl->F.width2 *= Gin->header->inc[GMT_X];	/* Rectangular rather than isotropic Cartesian filtering */
		}
		Ctrl->D.mode = 0;	/* From now on they are the same */	
	}

	/* Check range of output area and set i,j offsets, etc.  */

	if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, NULL)) == NULL) Return (API->error);
	GMT_grd_init (GMT, Gout->header, options, TRUE);	/* Update command history only */
	/* Use the -R region for output if set; otherwise match grid domain */
	GMT_memcpy (Gout->header->wesn, (GMT->common.R.active ? GMT->common.R.wesn : Gin->header->wesn), 4, double);
	GMT_memcpy (Gout->header->inc, (Ctrl->I.active ? Ctrl->I.inc : Gin->header->inc), 2, double);
	if (!full_360) {
		if (Gout->header->wesn[XLO] < Gin->header->wesn[XLO]) error = TRUE;
		if (Gout->header->wesn[XHI] > Gin->header->wesn[XHI]) error = TRUE;
	}
	if (Gout->header->wesn[YLO] < Gin->header->wesn[YLO]) error = TRUE;
	if (Gout->header->wesn[YHI] > Gin->header->wesn[YHI]) error = TRUE;

	if (error) {
		GMT_report (GMT, GMT_MSG_FATAL, "New WESN incompatible with old.\n");
		Return (EXIT_FAILURE);
	}

	/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Gout, Gout->header->wesn, Gout->header->inc, !one_or_zero), Ctrl->G.file);

	/* We can save time by computing a weight matrix once [or once pr scanline] only
	   if new grid spacing is a multiple of old spacing */

	fast_way = (fabs (fmod (Gout->header->inc[GMT_X] / Gin->header->inc[GMT_X], 1.0)) < GMT_SMALL && fabs (fmod (Gout->header->inc[GMT_Y] / Gin->header->inc[GMT_Y], 1.0)) < GMT_SMALL);
	same_grid = !(GMT->common.R.active  || Ctrl->I.active || Gin->header->registration == one_or_zero);
	if (!fast_way) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Your output grid spacing is such that filter-weights must\n");
		GMT_report (GMT, GMT_MSG_NORMAL, "be recomputed for every output node, so expect this run to be slow.  Calculations\n");
		GMT_report (GMT, GMT_MSG_NORMAL, "can be speeded up significantly if output grid spacing is chosen to be a multiple\n");
		GMT_report (GMT, GMT_MSG_NORMAL, "of the input grid spacing.  If the odd output grid is necessary, consider using\n");
		GMT_report (GMT, GMT_MSG_NORMAL, "a \'fast\' grid for filtering and then resample onto your desired grid with grdsample.\n");
	}
	if (Ctrl->N.mode == NAN_REPLACE && !same_grid) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: -Nr requires co-registered input/output grids, option is ignored\n");
		Ctrl->N.mode = NAN_IGNORE;
	}
	
	Gout->data = GMT_memory (GMT, NULL, Gout->header->size, float);
	i_origin = GMT_memory (GMT, NULL, Gout->header->nx, GMT_LONG);
	if (!fast_way) x_shift = GMT_memory (GMT, NULL, Gout->header->nx, double);

	if (fast_way && Gin->header->registration == one_or_zero) {	/* multiple grid but one is pix, other is grid */
		x_fix = 0.5 * Gin->header->inc[GMT_X];
		y_fix = 0.5 * Gin->header->inc[GMT_Y];
	}
	if (Ctrl->D.mode) {	/* Data on a sphere so must check for both periodic and polar wrap-arounds */
		spherical = TRUE;
		/* Compute the wrap-around delta_nx to use [may differ from nx unless a 360 grid] */
		nx_wrap = GMT_get_n (GMT, 0.0, 360.0, Gin->header->inc[GMT_X], GMT_PIXEL_REG);	/* So we basically bypass the duplicate point at east */
	}	
	if ((A = GMT_Create_Data (API, GMT_IS_GRID, NULL)) == NULL) Return (API->error);
#ifdef DEBUG
	if ((error = init_area_weights (GMT, Gin, Ctrl->D.mode, A, Ctrl->W.file))) Return (error);	/* Precalculate area weights */
#else
	if ((error = init_area_weights (GMT, Gin, Ctrl->D.mode, A, NULL))) Return (error);	/* Precalculate area weights */
#endif
	GMT_memset (&F, 1, struct FILTER_INFO);

	/* Set up the distance scalings for lon and lat, and assign pointer to distance function  */
#ifdef _OPENMP
#pragma omp parallel shared(fast_way,x_shift,i_origin) private(F,par,x_width,y_width,i,j,effort_level,tid,weight,work_array,work_data) firstprivate(x_scale,y_scale,filter_type,spherical,visit_check,go_on,max_lat,merc_range,slow,slower,x_fix,y_fix) reduction(+:n_nan)
{
	tid = omp_get_thread_num ();
#endif

	switch (Ctrl->D.mode) {
		case 0:	/* Plain, unscaled isotropic Cartesian distances */
			x_scale = y_scale = 1.0;
			F.radius_func = CartRadius;
			break;
		case 1:	/* Plain, scaled (degree to km) isotropic Cartesian distances */
			x_scale = y_scale = GMT->current.proj.DIST_KM_PR_DEG;
			F.radius_func = CartScaledRadius;
			break;
		case 2:	/* Flat Earth Cartesian distances, xscale fixed at mid latitude */
			x_scale = GMT->current.proj.DIST_KM_PR_DEG * cosd (0.5 * (Gout->header->wesn[YHI] + Gout->header->wesn[YLO]));
			y_scale = GMT->current.proj.DIST_KM_PR_DEG;
			F.radius_func = FlatEarthRadius;
			break;
		case 3:	/* Flat Earth Cartesian distances, xscale reset for each latitude (xscale here is max scale for max |lat|) */
			x_scale = GMT->current.proj.DIST_KM_PR_DEG * ((fabs (Gout->header->wesn[YLO]) > Gout->header->wesn[YHI]) ? cosd (Gout->header->wesn[YLO]) : cosd (Gout->header->wesn[YHI]));
			y_scale = GMT->current.proj.DIST_KM_PR_DEG;
			F.radius_func = FlatEarthRadius;
			break;
		case 4:	/* Great circle distances */
			x_scale = 0.0;
			y_scale = GMT->current.proj.DIST_KM_PR_DEG;
			F.radius_func = SphericalRadius;
			break;
		case 5:	/* Great circle distances with Mercator coordinates */
			/* Get the max |lat| extent of the grid */
			max_lat = IMG2LAT (MAX (fabs (Gin->header->wesn[YLO]), fabs (Gin->header->wesn[YHI])));
			merc_range = LAT2IMG (max_lat + (0.5 * Ctrl->F.width / GMT->current.proj.DIST_KM_PR_DEG)) - LAT2IMG (max_lat);
			x_scale = y_scale = 0.5 * Ctrl->F.width / merc_range;
			F.radius_func = SphericalRadius;
			break;
	}
			
	/* Assign filter_type number */
	
	for (filter_type = 0; filter_type < GRDFILTER_N_FILTERS && filter_code[filter_type] != Ctrl->F.filter; filter_type++);
	
	switch (filter_type) {
		case 1:	/*  Cosine-bell filter weights */
			par[GRDFILTER_INV_R_SCALE] = (Ctrl->F.rect) ? 1.0 : 2.0 / Ctrl->F.width;
			F.weight_func = CosBellWeight;
			break;
		case 2:	/*  Gaussian filter weights */
			par[GRDFILTER_INV_R_SCALE] = (Ctrl->F.rect) ? -4.5 : -18.0 / (Ctrl->F.width * Ctrl->F.width);
			F.weight_func = GaussianWeight;
			break;
		default:	/* Everything else uses unit weights */
			F.weight_func = UnitWeight;
			break;
	}
	
	/* Set up miscellaneous filter parameters needed when computing the weights */
	
	par[GRDFILTER_WIDTH] = Ctrl->F.width;
	par[GRDFILTER_HALF_WIDTH] = 0.5 * Ctrl->F.width;
	par[GRDFILTER_X_SCALE] = x_scale;
	par[GRDFILTER_Y_SCALE] = (Ctrl->D.mode == 5) ? GMT->current.proj.DIST_KM_PR_DEG : y_scale;
	F.d_flag = Ctrl->D.mode;
	F.rect = Ctrl->F.rect;
	F.dx = Gin->header->inc[GMT_X];
	F.dy = Gin->header->inc[GMT_Y];
	F.y_min = Gin->header->wesn[YLO];
	F.y_max = Gin->header->wesn[YHI];
	x_width = Ctrl->F.width / (Gin->header->inc[GMT_X] * x_scale);
	y_width = ((F.rect) ? Ctrl->F.width2 : Ctrl->F.width) / (Gin->header->inc[GMT_Y] * y_scale);
	F.x_half_width = (GMT_LONG) (ceil(x_width) / 2.0);
	F.y_half_width = (GMT_LONG) (ceil(y_width) / 2.0);
	F.nx = 2 * F.x_half_width + 1;
	F.ny = 2 * F.y_half_width + 1;
	if (x_scale == 0.0 || F.nx < 0 || F.nx > Gin->header->nx) {	/* Safety valve when x_scale -> 0.0 */
		F.nx = Gin->header->nx;
		F.x_half_width = (F.nx - 1) / 2;
		if ((F.nx - 2 * F.x_half_width - 1) > 0) F.x_half_width++;	/* When nx is even we may come up short by 1 */
		visit_check = ((2 * F.x_half_width + 1) >= Gin->header->nx);	/* Must make sure we only visit each node once along a row */
	}
	if (F.ny < 0 || F.ny > Gin->header->ny) {	/* Safety valve when y_scale -> 0.0 */
		F.ny = Gin->header->ny;
		F.y_half_width = (F.ny - 1) / 2;
	}
	F.x = GMT_memory (GMT, NULL, F.x_half_width+1, double);
	F.y = GMT_memory (GMT, NULL, F.y_half_width+1, double);
	F.visit = GMT_memory (GMT, NULL, Gin->header->nx, char);
	for (i = 0; i <= F.x_half_width; i++) F.x[i] = i * F.dx;
	for (j = 0; j <= F.y_half_width; j++) F.y[j] = j * F.dy;
	
	weight = GMT_memory (GMT, NULL, F.nx*F.ny, double);

	if (filter_type >= 3) {	/* These filters are not convolutions; they require sorting or comparisons */
		slow = TRUE;
		last_median = 0.5 * (Gin->header->z_min + Gin->header->z_max);	/* Initial guess */
		if (Ctrl->D.mode && filter_type < 5) {	/* Spherical (weighted) median/modes requires even more work */
			slower = TRUE;
			filter_type += 6;	/* To jump to the weighted versions */
			work_data = GMT_memory (GMT, NULL, F.nx*F.ny, fpair);
		}
		else
			work_array = GMT_memory (GMT, NULL, F.nx*F.ny, double);
	}

	if (tid == 0) {	/* First or only thread */
		GMT_report (GMT, GMT_MSG_NORMAL, "Input nx,ny = (%d %d), output nx,ny = (%d %d), filter (max)nx,ny = (%ld %ld)\n", Gin->header->nx, Gin->header->ny, Gout->header->nx, Gout->header->ny, F.nx, F.ny);
		GMT_report (GMT, GMT_MSG_NORMAL, "Filter type is %s.\n", filter_name[filter_type]);
#ifdef _OPENMP
		GMT_report (GMT, GMT_MSG_NORMAL, "Calculations will be distributed over %d threads.\n", omp_get_num_threads ());
#endif
	}

#ifdef DEBUG
	if (Ctrl->A.active) {	/* Picked a point to examine filter weights etc */
		Ctrl->A.COL = GMT_grd_x_to_col (GMT, Ctrl->A.x, Gout->header);
		Ctrl->A.ROW = GMT_grd_y_to_row (GMT, Ctrl->A.y, Gout->header);
		if (Ctrl->A.mode == 'r') F.debug = 1;	/* In order to return radii instead of weights */
		if (tid == 0) GMT_report (GMT, GMT_MSG_VERBOSE, "ROW = %ld COL = %ld\n", Ctrl->A.ROW, Ctrl->A.COL);
	}
#endif
	/* Compute nearest xoutput i-indices and shifts once */

	for (col_out = 0; col_out < Gout->header->nx; col_out++) {
		x_out = GMT_grd_col_to_x (GMT, col_out, Gout->header);	/* Current longitude */
		i_origin[col_out] = GMT_grd_x_to_col (GMT, x_out, Gin->header);
		if (!fast_way) x_shift[col_out] = x_out - GMT_grd_col_to_x (GMT, i_origin[col_out], Gin->header);
	}

	/* Determine how much effort to compute weights:
		1 = Compute weights once for entire grid
		2 = Compute weights once per scanline
		3 = Compute weights for every output point [slow]
	*/

	if (fast_way && Ctrl->D.mode <= 2)
		effort_level = 1;
	else if (fast_way && Ctrl->D.mode > 2)
		effort_level = 2;
	else 
		effort_level = 3;
	
	if (effort_level == 1) set_weight_matrix (GMT, &F, weight, 0.0, par, x_fix, y_fix);
	
#ifdef DEBUG
	if (Ctrl->A.active) for (i = 0; i < Gin->header->size; i++) Gin->data[i] = 0.0;	/* We are using Gin to store filter weights etc instead */
#endif

#ifdef _OPENMP
#pragma omp for schedule(dynamic,1) private(row_out,y_out,lat_out,j_origin,y,col_out,wt_sum,value,n_in_median,ij_out,ii,col_in,jj,row_in,ij_in,ij_wt) firstprivate(y_shift) nowait
#endif
	for (row_out = 0; row_out < Gout->header->ny; row_out++) {

#ifdef _OPENMP
		GMT_report (GMT, GMT_MSG_NORMAL, " Thread %ld Processing output line %ld\r", tid, row_out);
#else
		GMT_report (GMT, GMT_MSG_NORMAL, "Processing output line %ld\r", row_out);
#endif
#ifdef DEBUG
		if (Ctrl->A.active && row_out != Ctrl->A.ROW) continue;	/* Not at our selected row for testing */
#endif
		y_out = GMT_grd_row_to_y (GMT, row_out, Gout->header);		/* Current output y [or latitude] */
		lat_out = (Ctrl->D.mode == 5) ? IMG2LAT (y_out) : y_out;	/* Adjust lat if IMG grid */
		j_origin = GMT_grd_y_to_row (GMT, y_out, Gin->header);		/* Closest row in input grid */
		if (Ctrl->D.mode == 3) par[GRDFILTER_X_SCALE] = GMT->current.proj.DIST_KM_PR_DEG * cosd (lat_out);	/* Update flat-Earth longitude scale */

		if (Ctrl->D.mode > 2) {	/* Update max filterweight nodes to deal with for this latitude */
			y = fabs (lat_out);
			if (Ctrl->D.mode == 4) y += (par[GRDFILTER_HALF_WIDTH] / par[GRDFILTER_Y_SCALE]);	/* Highest latitude within filter radius */
			F.x_half_width = (y < 90.0) ? MIN ((F.nx - 1) / 2, irint (par[GRDFILTER_HALF_WIDTH] / (F.dx * par[GRDFILTER_Y_SCALE] * cosd (y)))) : (F.nx - 1) / 2;
			if (y > 90.0 && (F.nx - 2 * F.x_half_width - 1) > 0) F.x_half_width++;	/* When nx is even we may come up short by 1 */
			visit_check = ((2 * F.x_half_width + 1) >= Gin->header->nx);	/* Must make sure we only visit each node once along a row */
		}
			
		if (effort_level == 2) set_weight_matrix (GMT, &F, weight, y_out, par, x_fix, y_fix);	/* Compute new weights for this latitude */
		if (!fast_way) y_shift = y_out - GMT_grd_row_to_y (GMT, j_origin, Gin->header);

		for (col_out = 0; col_out < Gout->header->nx; col_out++) {

#ifdef DEBUG
			if (Ctrl->A.active && col_out != Ctrl->A.COL) continue;	/* Not at our selected column for testing */
#endif
			ij_out = GMT_IJP (Gout->header, row_out, col_out);	/* Node of current output point */
			if (Ctrl->N.mode == NAN_REPLACE && GMT_is_fnan (Gin->data[ij_out])) {	/* [Here we know ij_out == ij_in]. Since output will be NaN we bypass the filter loop */
				Gout->data[ij_out] = GMT->session.f_NaN;
				n_nan++;
				continue;	/* Done with this node */
			}

			if (col_out == 0 || col_out == (Gout->header->nx-1)) {
				ii = 4;
			}
			if (effort_level == 3) set_weight_matrix (GMT, &F, weight, y_out, par, x_shift[col_out], y_shift);	/* Update weights for this location */
			wt_sum = value = 0.0;	n_in_median = 0;	go_on = TRUE;	/* Reset all counters */
#ifdef DEBUG
			n_conv = 0;	/* Just to check # of points in the convolution - not used in any calculations */
#endif
			/* Now loop over the filter domain and collect those points that should be considered by the filter operation */
			
			for (jj = -F.y_half_width; go_on && jj <= F.y_half_width; jj++) {	/* Possible -/+ rows to consider for filter input */
				row_in = j_origin + jj;		/* Current input data row number */
				if (row_in < 0 || (row_in >= Gin->header->ny)) continue;	/* Outside input y-range */
				if (visit_check) GMT_memset (F.visit, Gin->header->nx, char);	/* Reset our longitude visit counter */
				for (ii = -F.x_half_width; go_on && ii <= F.x_half_width; ii++) {	/* Possible -/+ columns to consider on both sides of input point */
					col_in = i_origin[col_out] + ii;	/* Input column to consider */
					if (spherical) {			/* Must handle wrapping around the globe */
						if (col_in < 0) col_in += nx_wrap;	/* "Left" of west means we might reappear in the east */
						else if (col_in >= nx_wrap) col_in -= nx_wrap;	/* Likewise if we are "right" of east */
					}
					if (col_in < 0 || (col_in >= Gin->header->nx)) continue;	/* Still outside range of original input grid */
					if (visit_check) {	/* Make sure we never include the same node twice along a given row */
						if (F.visit[col_in]) continue;		/* Already been used */
						F.visit[col_in] = 1;			/* Now marked as visited */
					}
					ij_wt = WT_IJ (F, jj, ii);		/* Get weight array index */
					if (weight[ij_wt] <= 0.0) continue;	/* Negative weight means we are outside the filter circle */

					ij_in = GMT_IJP (Gin->header, row_in, col_in);	/* Finally, the current input data point inside the filter */
					if (GMT_is_fnan (Gin->data[ij_in])) {		/* Oops, found a rotten apple, skip */
						if (Ctrl->N.mode == NAN_PRESERVE) go_on = FALSE;	/* -Np means no NaNs are allowed */
						continue;
					}

					/* Get here when point is inside and usable  */
					
					if (slow) {	/* Add it to the relevant temporary work array */
						if (slower) {	/* Need to store both value and weight */
							work_data[n_in_median].x[0] = Gin->data[ij_in];
							work_data[n_in_median++].x[1] = (float)(weight[ij_wt] * A->data[ij_in]);
						}
						else	/* Only need to store values */
							work_array[n_in_median++] = Gin->data[ij_in];
					}
					else {	/* Update weighted sum for our convolution */
						w = weight[ij_wt] * A->data[ij_in];
						value += Gin->data[ij_in] * w;
						wt_sum += w;
#ifdef DEBUG
						n_conv++;	/* Points used inside filter circle */
						if (Ctrl->A.active) {	/* Store selected debug info in Gin data */
							if (Ctrl->A.mode == 'a') Gin->data[ij_in] = A->data[ij_in];
							else if (Ctrl->A.mode == 'w') Gin->data[ij_in] = (float)weight[ij_wt];
							else if (Ctrl->A.mode == 'r') Gin->data[ij_in] = (float)weight[ij_wt];
							else if (Ctrl->A.mode == 'c') Gin->data[ij_in] = (float)w;
						}
#endif
					}
				}
			}

			/* Done with all points inside the filter circle; Now get the final filtered value  */

			if (!go_on) {	/* -Np in effect and there were NaNs inside filter circle */
				Gout->data[ij_out] = GMT->session.f_NaN;
				n_nan++;
			}
			else if (slow) {	/* Non-convolution filters */
				if (n_in_median) {	/* Found valid values inside filter circle */
					switch (filter_type) {
						case 3:	/* Median */
							GMT_median (GMT, work_array, n_in_median, Gin->header->z_min, Gin->header->z_max, last_median, &this_median);
							last_median = this_median;
							break;
						case 4:	/* Mode */
							GMT_mode (GMT, work_array, n_in_median, n_in_median/2, TRUE, Ctrl->F.mode, &GMT_n_multiples, &this_median);
							break;
						case 5:	/* Lowest of all values */
							this_median = GMT_extreme (GMT, work_array, n_in_median, DBL_MAX, 0, -1);
							break;
						case 6:	/* Lowest of positive values */
							this_median = GMT_extreme (GMT, work_array, n_in_median, 0.0, +1, -1);
							break;
						case 7:	/* Upper of all values */
							this_median = GMT_extreme (GMT, work_array, n_in_median, -DBL_MAX, 0, +1);
							break;
						case 8:	/* Upper of negative values */
							this_median = GMT_extreme (GMT, work_array, n_in_median, 0.0, -1, +1);
							break;
						case 9:	/* Spherical Median */
							this_median = GMT_median_weighted (GMT, work_data, n_in_median, Ctrl->F.quantile);
							break;
						case 10: /* Spherical Mode */
							this_median = GMT_mode_weighted (GMT, work_data, n_in_median);
							break;
					}
					Gout->data[ij_out] = (float)this_median;
				}
				else {	/* Nothing found, set to NaN */
					Gout->data[ij_out] = GMT->session.f_NaN;
					n_nan++;
				}
			}
			else {	/* Convolution filters */
				if (wt_sum == 0.0) {	/* Nothing found, assign value = GMT->session.f_NaN */
					Gout->data[ij_out] = GMT->session.f_NaN;
					n_nan++;
				}
				else
					Gout->data[ij_out] = (float)(value / wt_sum);
			}
		}
	}

#ifdef _OPENMP
	GMT_report (GMT, GMT_MSG_NORMAL, " Thread %ld Processing output line %ld\n", tid, row_out);
#else
	GMT_report (GMT, GMT_MSG_NORMAL, "Processing output line %ld\n", row_out);
#endif
	GMT_free (GMT, weight);
	GMT_free (GMT, F.x);
	GMT_free (GMT, F.y);
	GMT_free (GMT, F.visit);
	if (slow) {
		if (slower) GMT_free (GMT, work_data);
		else GMT_free (GMT, work_array);
	}
#ifdef _OPENMP
}  /* end of parallel region */
#endif

	GMT_free (GMT, i_origin);
	if (!fast_way) GMT_free (GMT, x_shift);
	GMT_free_grid (GMT, &A, TRUE);	/* Explicitly free the area-weight array since only used internally */

	if (n_nan) GMT_report (GMT, GMT_MSG_NORMAL, "Unable to estimate value at %ld nodes, set to NaN\n", n_nan);
	if (GMT_n_multiples > 0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: %ld multiple modes found by the mode filter\n", GMT_n_multiples);

	if (Ctrl->F.highpass) {
		if (GMT->common.R.active || Ctrl->I.active || GMT->common.r.active) {	/* Must resample result */
			GMT_LONG object_ID;			/* Status code from GMT API */
			char in_string[GMTAPI_STRLEN], out_string[GMTAPI_STRLEN], cmd[GMT_BUFSIZ];
			/* Here we low-passed filtered onto a coarse grid but to get high-pass we must sample the low-pass result at the original resolution */
			if ((object_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REF, GMT_IS_SURFACE, GMT_IN, Gout, NULL)) == GMTAPI_NOTSET) {
				Return (API->error);
			}
			if (GMT_Encode_ID (API, in_string, object_ID) != GMT_OK) {	/* Make filename with embedded object ID for grid Gout */
				Return (API->error);
			}
			if ((object_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REF, GMT_IS_SURFACE, GMT_OUT, NULL, NULL)) == GMTAPI_NOTSET) {
				Return (API->error);
			}
			if (GMT_Encode_ID (GMT->parent, out_string, object_ID) != GMT_OK) {
				Return (API->error);	/* Make filename with embedded object ID for result grid L */
			}
			sprintf (cmd, "%s -G%s -R%s -V%ld", in_string, out_string, Ctrl->In.file, GMT->current.setting.verbose);
			if (GMT_is_geographic (GMT, GMT_IN)) strcat (cmd, " -fg");
			GMT_report (GMT, GMT_MSG_VERBOSE, "Highpass requires us to resample the lowpass result via grdsample %s\n", cmd);
			if (GMT_grdsample (GMT->parent, 0, cmd) != GMT_OK) {	/* Resample the file */
				GMT_report (GMT, GMT_MSG_FATAL, "Error: Unable to resample the lowpass result - exiting\n");
				GMT_exit (API->error);
			}
			if ((L = GMT_Retrieve_Data (API, object_ID)) == NULL) {
				Return (API->error);
			}
			Gout->alloc_mode = L->alloc_mode = GMT_ALLOCATED;	/* So we may destroy it */
			if (GMT_Destroy_Data (API, GMT_ALLOCATED, &Gout) != GMT_OK) {
				Return (API->error);
			}
			GMT_grd_loop (GMT, L, row_out, col_out, ij_out) L->data[ij_out] = Gin->data[ij_out] -L->data[ij_out];
			GMT_grd_init (GMT, L->header, options, TRUE);	/* Update command history only */
		}
		else	/* No need to resample */
			GMT_grd_loop (GMT, Gout, row_out, col_out, ij_out) Gout->data[ij_out] = Gin->data[ij_out] -Gout->data[ij_out];
		GMT_report (GMT, GMT_MSG_NORMAL, "Subtracting lowpass-filtered data from grid to obtain high-pass filtered data\n");
	}
	
	/* At last, that's it!  Output: */

#ifdef DEBUG
	if (Ctrl->A.active) {	/* Save the debug output instead */
		FILE *fp = fopen ("n_conv.txt", "w");
		fprintf (fp, "%ld\n", n_conv);
		fclose (fp);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, Ctrl->G.file, Gin) != GMT_OK) {
			Return (API->error);
		}
		GMT_free_grid (GMT, &Gout, TRUE);	/* Was never used due to testing */
	}
	else if (Ctrl->F.highpass && L) {
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, Ctrl->G.file, L) != GMT_OK) {
			Return (API->error);
		}
	}
	else {
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, Ctrl->G.file, Gout) != GMT_OK) {
			Return (API->error);
		}
	}
#else
	if (Ctrl->F.highpass && L) {
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, Ctrl->G.file, L) != GMT_OK) {
			Return (API->error);
		}
	}
	else {
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, Ctrl->G.file, Gout) != GMT_OK) {
			Return (API->error);
		}
	}
#endif

	Return (EXIT_SUCCESS);
}
