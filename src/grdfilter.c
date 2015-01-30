/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
/*
 * Brief synopsis: grdfilter.c reads a grid file and creates filtered grid file.
 * User selects boxcar, gaussian, cosine arch, median, mode or spatial filters,
 * and sets distance scaling as appropriate for map work, etc.
 *
 * Author:	W.H.F. Smith
 * Date: 	1-JAN-2010
 * Version:	5 API
*/

/* 
This is a experimental multi-threaded version that uses gthreads from GLIB an hence depends on that lib
that on its turn depends on  linintl (gettext)

To compile I patched src/CMakeList.txt by adding these two lines

    set (HAVE_GLIB TRUE CACHE INTERNAL "System has GLIB")
    include_directories (${GLIB_INCLUDE_DIR})
    list (APPEND GMT_OPTIONAL_LIBRARIES ${GLIB_LIBRARY})

and added this to ConfigUserCmake

    # Set location of GLIB ...:
    set (GLIB_INCLUDE_DIR "C:/programs/compa_libs/glib-2.38.2/compileds/${VC}_${BITAGE}/include/glib-2.0")
    set (GLIB_LIBRARY "C:/programs/compa_libs/glib-2.38.2/compileds/${VC}_${BITAGE}/lib/glib-2.0.lib")

maybe you don't need the above if cmake is able to find glib in your system.

Use option -x to set the number of threads. e.g. -x2, -x4, ... or -xa to use all available
*/

#define THIS_MODULE_NAME	"grdfilter"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Filter a grid in the space (or time) domain"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-RVfx"

struct GRDFILTER_CTRL {
	struct GRDFILT_In {
		bool active;
		char *file;
	} In;
	struct GRDFILT_A {	/* -A<a|r|w|c>row/col */
		bool active;
		char mode;
		unsigned int ROW, COL;
		double x, y;
		char *file;
	} A;
	struct GRDFILT_D {	/* -D<distflag> */
		bool active;
		int mode;	/* -1 to 5 */
	} D;
	struct GRDFILT_F {	/* <type>[-]<filter_width>[/<width2>][<mode>] */
		bool active;
		bool highpass;
		bool custom;
		bool center;
		bool operator;
		char filter;	/* Character codes for the filter */
		char *file;
		double width, width2, quantile, bin, span;
		bool rect;
		int mode;	/*-1 0 +1 */
	} F;
	struct GRDFILT_G {	/* -G<file> */
		bool active;
		char *file;
	} G;
	struct GRDFILT_I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
		char string[GMT_LEN256];
	} I;
	struct GRDFILT_N {	/* -Np|i|r */
		bool active;
		unsigned int mode;	/* 0 is default (i), 1 is replace (r), 2 is preserve (p) */
	} N;
	struct GRDFILT_T {	/* -T */
		bool active;
	} T;
	struct GRDFILT_z {	/* -z */
		bool active;
		int n_threads;
	} z;
};

struct GRDFILTER_BIN_MODE_INFO {	/* Used for histogram binning */
	double width;		/* The binning width used */
	double i_offset;	/* 0.5 if we are to bin using the center the bins on multiples of width, else 0.0 */
	double o_offset;	/* 0.0 if we are to report center the bins on multiples of width, else 0.5 */
	double i_width;		/* 1/width, to avoid divisions later */
	int min, max;		/* The raw min,max bin numbers (min can be negative) */
	unsigned int n_bins;	/* Number of bins required */
	double *fcount;		/* The histogram counts (double for weighted data), to be reset before each spatial block */
	unsigned int *icount;	/* The histogram counts (ints to unweighted data), to be reset before each spatial block */
	int mode_choice;	/* For multiple modes: GRDFILTER_MODE_KIND_LOW picks lowest, GRDFILTER_MODE_KIND_AVE picks average, GRDFILTER_MODE_KIND_HIGH picks highest */
};

enum Grdfilter_mode {
	GRDFILTER_MODE_KIND_LOW  = -1,
	GRDFILTER_MODE_KIND_AVE  = 0,
	GRDFILTER_MODE_KIND_HIGH = +1
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

#define GRDFILTER_FILTERS	"bcgfhompLlUu"	/* These dont have to be in any order */
#define GRDFILTER_MODES		"012345p"

/* Distance modes: */
#define GRDFILTER_NOTSET		-2
#define GRDFILTER_XY_PIXEL		-1
#define GRDFILTER_XY_CARTESIAN		0
#define GRDFILTER_GEO_CARTESIAN		1
#define GRDFILTER_GEO_FLATEARTH1	2
#define GRDFILTER_GEO_FLATEARTH2	3
#define GRDFILTER_GEO_SPHERICAL		4
#define GRDFILTER_GEO_MERCATOR		5
/* Convolution filters: */
#define GRDFILTER_BOXCAR	0	/* Note: The order of these filters matter and must match entries in filter_code[] array */
#define GRDFILTER_COSINE	1
#define GRDFILTER_GAUSSIAN	2
#define GRDFILTER_CUSTOM	3
#define GRDFILTER_OPERATOR	4
/* Spatial non-convolution filters: */
#define GRDFILTER_MEDIAN	5
#define GRDFILTER_LMS		6
#define GRDFILTER_HIST		7
#define GRDFILTER_MIN		8
#define GRDFILTER_MINPOS	9
#define GRDFILTER_MAX		10
#define GRDFILTER_MAXNEG	11
#define GRDFILTER_MEDIAN_SPH	12
#define GRDFILTER_LMS_SPH	13
#define GRDFILTER_HIST_SPH	14
#define GRDFILTER_N_FILTERS	12	/* Not counting the three _SPH variants */
#define GRDFILTER_CART_TO_SPH	7	/* Value to add to GRDFILTER_MEDIAN to get GRDFILTER_MEDIAN_SPH */

#define NAN_IGNORE	0
#define NAN_REPLACE	1
#define NAN_PRESERVE	2


struct FILTER_INFO {
	unsigned int nx;		/* The max number of filter weights in x-direction */
	unsigned int ny;		/* The max number of filter weights in y-direction */
	int x_half_width;		/* Number of filter nodes to either side needed at this latitude */
	int y_half_width;		/* Number of filter nodes above/below this point (ny_f/2) */
	unsigned int d_flag;
	bool rect;		/* For 2-D rectangular filtering */
	bool debug;		/* Normally unused except under DEBUG */
	double dx, dy;		/* Grid spacing in original units */
	double y_min, y_max;	/* Grid limits in y(lat) */
	double *x, *y;		/* Distances in original units along x and y to distance nodes */
	double par[5];		/* [0] is filter width, [1] is 0.5*filter_width, [2] is xscale, [3] is yscale, [4] is 1/r_half for filter */
	double x_off, y_off;	/* Offsets relative to original grid */
	char *visit;		/* true/false array to keep track of which longitude nodes we have already visited once */
	double (*weight_func) (double, double *);
	double (*radius_func) (struct GMT_CTRL *, double, double, double, double, double *);
};

struct THREAD_STRUCT {
	bool   fast_way, spherical, slow, slower, get_weight_sum;
	int    *col_origin, nx_wrap;
	unsigned int row, r_start, r_stop, effort_level, filter_type, thread_num;
	double x_fix, y_fix, last_median;
	double *weight, *x_shift, *par;
	struct GMT_CTRL *GMT;
	struct GRDFILTER_CTRL *Ctrl;
	struct GMT_GRID *Gin;
	struct GMT_GRID *Gout;
	struct GMT_GRID *A, *L;
	struct FILTER_INFO F;
	struct GRDFILTER_BIN_MODE_INFO *B;
};

static void *threading_function (void *args);
void threaded_function (struct THREAD_STRUCT *t);

void *New_grdfilter_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDFILTER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDFILTER_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->D.mode = GRDFILTER_NOTSET;	
	C->F.quantile = 0.5;	/* Default is median */	
	C->F.span = 0.5;	/* Default LMS span is half the data */	
	C->F.mode = GRDFILTER_MODE_KIND_AVE;
	C->z.n_threads = 1;	/* Default number of threads (for the case of no multi-threading) */
	return (C);
}

void Free_grdfilter_Ctrl (struct GMT_CTRL *GMT, struct GRDFILTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->F.file) free (C->F.file);	
	if (C->G.file) free (C->G.file);	
	if (C->A.file) free (C->A.file);	
	GMT_free (GMT, C);	
}

/* -----------------------------------------------------------------------------------*/
static void *thread_function (void *args) {
	threaded_function ((struct THREAD_STRUCT *)args);
	return NULL;
}

struct GRDFILTER_BIN_MODE_INFO *grdfilter_bin_setup (struct GMT_CTRL *GMT, double min, double max, double width, bool center, int mode_choice, bool weighted)
{
	/* Estimate mode by finding a maximum in the histogram resulting
	 * from binning the grid data with the specified width.
	 * This function sets up quantities needed as we loop over the
	 * spatial bins */

	struct GRDFILTER_BIN_MODE_INFO *B = GMT_memory (GMT, NULL, 1, struct GRDFILTER_BIN_MODE_INFO);

	B->i_offset = (center) ? 0.5 : 0.0;
	B->o_offset = (center) ? 0.0 : 0.5;
	B->width = width;
	B->i_width = 1.0 / width;
	B->min = irint (floor ((min * B->i_width) + B->i_offset));
	B->max = irint (ceil  ((max * B->i_width) + B->i_offset));
	B->n_bins = B->max - B->min + 1;
	if (weighted)
		B->fcount = GMT_memory (GMT, NULL, B->n_bins, double);
	else
		B->icount = GMT_memory (GMT, NULL, B->n_bins, unsigned int);
	B->mode_choice = mode_choice;
	
	return (B);
}

double GMT_histmode (struct GMT_CTRL * GMT_UNUSED(GMT), double *z, uint64_t n, struct GRDFILTER_BIN_MODE_INFO *B)
{
	/* Estimate mode by finding a maximum in the histogram resulting
	 * from binning unweighted data with the specified width. We check if we find more
	 * than one mode and return the chosen one as per the settings. */

	double value = 0.0;
	uint64_t i;
	unsigned int n_modes = 0, mode_count = 0, ubin;
	int bin, mode_bin = 0;
	bool done;

	GMT_memset (B->icount, B->n_bins, unsigned int);	/* Reset the counts */
	for (i = 0; i < n; i++) {	/* Loop over data points */
		bin = irint (floor ((z[i] * B->i_width) + B->i_offset)) - B->min;
		B->icount[bin]++;			/* Add up counts */
		if (B->icount[bin] > mode_count) {	/* New max count value; make a note */
			mode_count = B->icount[bin];	/* Highest count so far... */
			mode_bin = bin;			/* ...occuring for this bin */
			n_modes = 1;			/* Only one of these so far */
		}
		else if (B->icount[bin] == mode_count) n_modes++;	/* Bin has same peak as previous best mode; increase mode count */
	}
	if (n_modes == 1) {	/* Single mode; we are done */
		value = ((mode_bin + B->min) + B->o_offset) * B->width;
		return (value);
	}
	
	/* Here we found more than one mode and must choose according to settings */
	
	for (ubin = 0, done = false; !done && ubin < B->n_bins; ubin++) {	/* Loop over bin counts */
		if (B->icount[ubin] < mode_count) continue;	/* Not one of the modes */
		switch (B->mode_choice) {
			case GRDFILTER_MODE_KIND_LOW:	/* Pick lowest mode; we are done */
				value = ((ubin + B->min) + B->o_offset) * B->width;
				done = true;
				break;
			case GRDFILTER_MODE_KIND_AVE:		/* Get the average of the modes */
				value += ((ubin + B->min) + B->o_offset) * B->width;
				break;
			case GRDFILTER_MODE_KIND_HIGH:	/* Update highest mode so far, when loop exits we have the hightest mode */
			 	value = ((ubin + B->min) + B->o_offset) * B->width;
				break;
		}
	}
	if (B->mode_choice == GRDFILTER_MODE_KIND_AVE) value /= n_modes;	/* The average of the multiple modes */

	return (value);
}

double GMT_histmode_weighted (struct GMT_CTRL *GMT, struct GMT_OBSERVATION *data, uint64_t n, struct GRDFILTER_BIN_MODE_INFO *B)
{
	/* Estimate mode by finding a maximum in the histogram resulting
	 * from binning weighted data with the specified width. We check if we find more
	 * than one mode and return the chosen one as per the settings. */

	double value = 0.0, mode_count = 0.0;
	uint64_t i;
	unsigned int n_modes = 0, ubin;
	int bin, mode_bin = 0;
	bool done;

	GMT_memset (B->fcount, B->n_bins, double);	/* Reset the counts */
	for (i = 0; i < n; i++) {	/* Loop over data points */
		bin = urint (floor ((data[i].value * B->i_width) + B->i_offset)) - B->min;
		B->fcount[bin] += data[i].weight;	/* Add up weights */
		if (B->fcount[bin] > mode_count) {	/* New max weight value; make a note */
			mode_count = B->fcount[bin];	/* Highest count so far... */
			mode_bin = bin;			/* ...occuring for this bin */
			n_modes = 1;			/* Only one of these so far */
		}
		else if (doubleAlmostEqual (B->fcount[bin], mode_count)) n_modes++;	/* Bin has same peak as previous best mode; increase mode count */
	}
	if (n_modes == 1) {	/* Single mode; we are done */
		value = ((mode_bin + B->min) + B->o_offset) * B->width;
		return (value);
	}
	
	/* Here we found more than one mode and must choose according to settings */
	
	for (ubin = 0, done = false; !done && ubin < B->n_bins; ubin++) {	/* Loop over bin counts */
		if (B->fcount[ubin] < mode_count) continue;	/* Not one of the modes */
		switch (B->mode_choice) {
			case GRDFILTER_MODE_KIND_LOW:	/* Pick lowest mode; we are done */
				value = ((ubin + B->min) + B->o_offset) * B->width;
				done = true;
				break;
			case GRDFILTER_MODE_KIND_AVE:		/* Get the average of the modes */
				value += ((ubin + B->min) + B->o_offset) * B->width;
				break;
			case GRDFILTER_MODE_KIND_HIGH:	/* Update highest mode so far, when loop exits we have the hightest mode */
			 	value = ((ubin + B->min) + B->o_offset) * B->width;
				break;
		}
	}
	if (B->mode_choice == GRDFILTER_MODE_KIND_AVE) value /= n_modes;	/* The average of the multiple modes */

	return (value);
}

void set_weight_matrix (struct GMT_CTRL *GMT, struct FILTER_INFO *F, double *weight, double output_lat, double par[], double x_off, double y_off)
{
	/* x_off and y_off give offset between output node and 'origin' input node for this window (0,0 for integral grids).
	 * fast is true when input/output grids are offset by integer values in dx/dy.
	 * Here, par[0] = filter_width, par[1] = filter_width / 2, par[2] = x_scale, part[3] = y_scale, and
	 * par[4] is the normalization distance needed for the Cosine-bell or Gaussian weight function.
	 */

	int i, j;
	int64_t ij;
	double x, y, yc, y0, r, ry = 0.0, inv_x_half_width = 0.0, inv_y_half_width = 0.0;

	yc = y0 = output_lat - y_off;		/* Input latitude of central point input grid (i,j) = (0,0) */
	if (F->d_flag == GRDFILTER_GEO_MERCATOR) yc = IMG2LAT (yc);	/* Recover actual latitude in IMG grid at this center point */
	if (F->rect) {
		inv_x_half_width = 1.0 / F->x_half_width;
		inv_y_half_width = 1.0 / F->y_half_width;
	}
	for (j = -F->y_half_width; j <= F->y_half_width; j++) {
		y = y0 + ((j < 0) ? F->y[-j] : -F->y[j]);	/* y or latitude at this row in input grid */
		if (F->d_flag > GRDFILTER_GEO_FLATEARTH1 && (y < F->y_min || y > F->y_max)) {		/* This filter row is outside input grid domain */
			for (i = -F->x_half_width, ij = (j + F->y_half_width) * F->nx; i <= F->x_half_width; i++, ij++) weight[ij] = -1.0;
			continue;	/* Done with this row */
		}
		if (F->d_flag == GRDFILTER_GEO_MERCATOR) y = IMG2LAT (y);	/* Recover actual latitudes */
		if (F->rect) ry = inv_y_half_width * j;	/* -1 to +1 */
		for (i = -F->x_half_width; i <= F->x_half_width; i++) {
			x = (i < 0) ? -F->x[-i] : F->x[i];	/* Input grid x-coordinate relative to center */ 
			ij = (j + F->y_half_width) * F->nx + i + F->x_half_width;
			assert (ij >= 0);
			if (F->rect) {	/* 2-D rectangular filtering; radius not used as we use x/x_half_width and ry instead */
				weight[ij] = F->weight_func (inv_x_half_width * i, par) * F->weight_func (ry, par);
			}
			else {
				/* Use offsets x_off and y_off to account for offsets between input and output node in terms of fractional input dx/dy */
				r = F->radius_func (GMT, x_off, yc+y_off, x, y, par);
				weight[ij] = (r > par[GRDFILTER_HALF_WIDTH]) ? -1.0 : F->weight_func (r, par);
#ifdef DEBUG
				if (F->debug) weight[ij] = (r > par[GRDFILTER_HALF_WIDTH]) ? -1.0 : r;
#endif
			}
		}
	}
}

/* Various functions that will be accessed via pointers */
double CartRadius (struct GMT_CTRL *GMT_UNUSED(GMT), double x0, double y0, double x1, double y1, double GMT_UNUSED(par[]))
{	/* Plain Cartesian distance (par is not used) */
	return (hypot (x0 - x1, y0 - y1));
}

double CartScaledRadius (struct GMT_CTRL *GMT_UNUSED(GMT), double x0, double y0, double x1, double y1, double par[])
{	/* Plain scaled Cartesian distance (xscale = yscale) */
	return (par[GRDFILTER_X_SCALE] * hypot (x0 - x1, y0 - y1));
}

double CartScaledRect (struct GMT_CTRL *GMT_UNUSED(GMT), double x0, double y0, double x1, double y1, double par[])
{	/* Pass dx,dy via par[GRDFILTER_X|Y_DIST] and return a r that is either in or out */
	double r;
	par[GRDFILTER_X_DIST] = par[GRDFILTER_X_SCALE] * (x0 - x1);
	par[GRDFILTER_Y_DIST] = par[GRDFILTER_Y_SCALE] * (y0 - y1);
	r = (fabs (par[GRDFILTER_X_DIST]) > par[GRDFILTER_HALF_WIDTH] || fabs (par[GRDFILTER_Y_DIST]) > par[GRDFILTER_HALF_WIDTH]) ? 2.0 : 0.0;
	return (r);
}

double FlatEarthRadius (struct GMT_CTRL *GMT_UNUSED(GMT), double x0, double y0, double x1, double y1, double par[])
{	/* Cartesian radius with different scales */
	return (hypot (par[GRDFILTER_X_SCALE] * (x0 - x1), par[GRDFILTER_Y_SCALE] * (y0 - y1)));
}

double SphericalRadius (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1, double GMT_UNUSED(par[]))
{	/* Great circle distance in km with polar wrap-around test on 2nd point */
	if (fabs (y1) > 90.0) {	/* Must find the point across the pole */
		y1 = copysign (180.0 - fabs (y1), y1);
		x1 += 180.0;
	}
	return (0.001 * GMT_great_circle_dist_meter (GMT, x0, y0, x1, y1));
}

double UnitWeight (double GMT_UNUSED(r), double GMT_UNUSED(par[]))
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

struct GMT_GRID * init_area_weights (struct GMT_CTRL *GMT, struct GMT_GRID *G, int mode, char *file)
{
	/* Precalculate the area weight of each node.  There are several considerations:
	 * 1. Mercator img grids (d_flag == GRDFILTER_GEO_MERCATOR) has irregular latitude spacing so we
	 *    must compute the north and south latitude bounds for each cell to get area.
	 * 2. Geographic poles for grid-registered grids require a separate weight formula
	 * 3. Grid-registered grids have boundary nodes that only apply to 1/2 the area
	 *    (and the four corners (unless poles) only 1/4 the area of other cells).
	 */
	unsigned int row, col;
	uint64_t ij;
	double row_weight, col_weight, dy_half = 0.0, dx, y, lat, lat_s, lat_n, s2 = 0.0;
	struct GMT_GRID *A = NULL;
	
	/* Base the area weight grid on the input grid domain and increments. */
	if ((A = GMT_Create_Data (GMT->parent, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, G->header->wesn, G->header->inc, \
		G->header->registration, GMT_NOTSET, file)) == NULL) return (NULL);
	
	if (mode > GRDFILTER_XY_CARTESIAN) {	/* Geographic data */
		if (mode == GRDFILTER_GEO_MERCATOR) dy_half = 0.5 * A->header->inc[GMT_Y];	/* Half img y-spacing */
		dx = A->header->inc[GMT_X] * R2D;			/* Longitude increment in radians */
		s2 = sind (0.5 * A->header->inc[GMT_Y]);		/* Holds sin (del_y/2) */
	}
	else	/* Cartesian */
		dx = A->header->inc[GMT_X];
	GMT_row_loop (GMT, A, row) {	/* Loop over the rows */
		if (mode == GRDFILTER_GEO_MERCATOR) {		/* Adjust lat if IMG grid.  Note: these grids can never reach a pole. */
			y = GMT_grd_row_to_y (GMT, row, A->header);	/* Current input Merc y */
			lat = IMG2LAT (y);			 /* Get actual latitude */
			lat_s = IMG2LAT (y - dy_half);		/* Bottom grid cell latitude */
			lat_n = IMG2LAT (y + dy_half);		/* Top grid cell latitude */
			row_weight = sind (lat_n) - sind (lat_s);
		}
		else if (mode > GRDFILTER_XY_CARTESIAN) {	/* Geographic data, and watch for poles */
			lat = GMT_grd_row_to_y (GMT, row, A->header);	/* Current input latitude */
			if (GMT_IS_POLE (lat))	/* Poles are different */
				row_weight = 1.0 - cosd (0.5 * A->header->inc[GMT_Y]);
			else {	/* All other points */
				row_weight = 2.0 * cosd (lat) * s2;
				/* Note: No need for special weight-sharing between w/e gridline-reg grids since we explicitly only use the west node */
			}
		}
		else {	/* If Cartesian then row_weight is a constant 1 except for gridline-registered grids at top or bottom row */
			row_weight = (A->header->registration == GMT_GRID_NODE_REG && (row == 0 || row == (A->header->ny-1))) ? 0.5 : 1.0;	/* Share weight with repeat point */
			row_weight *= A->header->inc[GMT_Y];
		}

		GMT_col_loop (GMT, A, row, col, ij) {	/* Now loop over the columns */
			col_weight = dx * ((A->header->registration == GMT_GRID_NODE_REG && (col == 0 || col == (A->header->nx-1))) ? 0.5 : 1.0);
			A->data[ij] = (float)(row_weight * col_weight);
		}
	}
	if (file) {	/* For debug purposes: Save the area weight grid */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Write area weight grid to file %s\n", file);
		if (GMT_Set_Comment (GMT->parent, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, "Area weight grid for debugging purposes", A)) return (NULL);
		if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, file, A) != GMT_OK) return (NULL);
	}
	return (A);
}

int GMT_grdfilter_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdfilter <ingrid> -D<distance_flag> -F<type>[-]<filter_width>[/<width2>][<modifiers>] -G<outgrid>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-Ni|p|r] [%s]\n\t[-T] [%s] [%s] [%s]\n", GMT_I_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_x_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is the input grid file to be filtered.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Distance flag determines how grid (x,y) maps into distance units of filter width as follows:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cartesian (x, y) Data:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Dp grid x,y with <filter_width> in pixels (must be an odd number), Cartesian distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D0 grid x,y same units as <filter_width>, Cartesian distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Geographic (lon,lat) Data:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D1 grid x,y in degrees, <filter_width> in km, Cartesian distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D2 grid x,y in degrees, <filter_width> in km, x_scaled by cos(middle y), Cartesian distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The options above are faster; they allow weight matrix to be computed only once.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Next three options are slower; weights must be recomputed for each scan line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D3 grid x,y in degrees, <filter_width> in km, x_scale varies as cos(y), Cartesian distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D4 grid x,y in degrees, <filter_width> in km, spherical distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -D5 grid x,y in Mercator units (-Jm1), <filter_width> in km, spherical distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Set the low-pass filter type and full diameter (6 sigma) filter-width.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Choose between convolution-type filters which differ in how weights are assigned\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and geospatial filters that seek to return a representative value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give a negative filter width to select highpass filtering [lowpass].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Filters are isotropic.  For rectangular filtering append /<width2> (requires -Dp|0).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Convolution filters:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     b: Boxcar : a simple averaging of all points inside filter domain.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c: Cosine arch : a weighted averaging with cosine arc weights.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     g: Gaussian : weighted averaging with Gaussian weights.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f: Custom : Give grid file with custom filter weights (requires -D0).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     o: Operator : Give grid file with custom operator weights (requires -D0).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Geospatial filters:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     h: Histogram-based mode estimator.  Append width/bin and optional modifiers:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Data inside filter domain is binned using the <bin> increment\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append +c to center the bins on multiples of <bin> [Default has bin edges as multiples]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append +l to return the lowest mode if multiple modes are found [return average].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append +u to return the uppermost mode if multiple modes are found [return average].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     l: Lower : return minimum of all points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     L: Lower+ : return minimum of all positive points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     m: Median : return the median (50%% quantile) value of all points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append +q<quantile> to select another quantile (in 0-1 range) [0.5].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     p: Maximum likelihood probability estimator : return mode of all points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        We approximate the mode using the Least Median of Squares estimator.\n");
	//GMT_Message (API, GMT_TIME_NONE, "\t        Append +c<span> to select a different LMS span, in percent [50].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append +l to return the lowest mode if multiple modes are found [return average].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append +u to return the uppermost mode if multiple modes are found [return average].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     u: Upper : return maximum of all points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     U: Upper- : return maximum of all negative points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set output filename for filtered grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
#ifdef DEBUG
	GMT_Message (API, GMT_TIME_NONE, "\t-A DEBUG: Use -A<mode><lon/<lat> to instead save filter specifics at that point. Choose:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   mode as a (area weights), c (composite weight), r (radii), or w (filter weight).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A DEBUG: Save area weigths to <file> with -As<file> [no save]\n");
#endif
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t   The new xinc and yinc should be divisible by the old ones (new lattice is subset of old).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Specify how NaNs in the input grid should be treated. There are three options:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ni skips all NaN values and returns a filtered value unless all are NaN [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Np sets filtered output to NaN is any NaNs are found inside filter circle.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Nr sets filtered output to NaN if the corresponding input node was NaN.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      (only possible if the input and output grids are coregistered).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Toggle between grid and pixel registration for output grid [Default is same as input registration].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-R For new Range of output grid; enter <WESN> (xmin, xmax, ymin, ymax) separated by slashes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses the same region as the input grid].\n");
	GMT_Option (API, "V,f");
#ifdef HAVE_GLIB_GTHREAD
	GMT_Option (API, "x");
#else
	GMT_Message (API, GMT_TIME_NONE, "\t-x Not available since this binary was not build with multi-threading support.\n");
#endif
	GMT_Option (API, ".");
	
	return (EXIT_FAILURE);
}

int GMT_grdfilter_parse (struct GMT_CTRL *GMT, struct GRDFILTER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdfilter and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	char c, a[GMT_LEN64] = {""}, b[GMT_LEN64] = {""}, txt[GMT_LEN256] = {""}, *p = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

#ifdef DEBUG
			case 'A':	/* Debug mode options */
				Ctrl->A.active = true;
				if (opt->arg[0] == 's') {
					Ctrl->A.file = strdup (&opt->arg[1]);
				}
				else {
					Ctrl->A.mode = opt->arg[0];
					sscanf (&opt->arg[1], "%[^/]/%s", a, b);
					Ctrl->A.x = atof (a);
					Ctrl->A.y = atof (b);
				}
				break;
#endif
			case 'D':	/* Distance mode */
				Ctrl->D.active = true;
				if (strchr (GRDFILTER_MODES, opt->arg[0])) {	/* OK filter code */
					Ctrl->D.mode = (opt->arg[0] == 'p') ? GRDFILTER_XY_PIXEL : atoi (opt->arg);
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -D<mode>, where mode is one of %s\n", GRDFILTER_MODES);
					n_errors++;
				}
				break;
			case 'F':	/* Filter */
				if (strchr (GRDFILTER_FILTERS, opt->arg[0])) {	/* OK filter code */
					Ctrl->F.active = true;
					Ctrl->F.filter = opt->arg[0];
					strncpy (txt, opt->arg, GMT_LEN256);	/* Work on a copy */
					if (Ctrl->F.filter == 'm') {
						if ((p = strchr (txt, 'q'))) {	/* Requested another quantile */
							*(--p) = 0;	/* Chop off the +q modifier */
							Ctrl->F.quantile = atof (p+2);
						}
					}
					if (Ctrl->F.filter == 'f' || Ctrl->F.filter == 'o') {
						if (GMT_check_filearg (GMT, 'F', &opt->arg[1], GMT_IN, GMT_IS_GRID))
							Ctrl->F.file = strdup (&opt->arg[1]);
						else {
								GMT_Report (API, GMT_MSG_NORMAL, "ERROR -F%c: Cannot access filter weight grid %s\n", Ctrl->F.filter, &opt->arg[1]);
								n_errors++;
							}
						Ctrl->F.width = 1.0;	/* To avoid error checking below */
						Ctrl->F.custom = true;
						Ctrl->F.operator = (Ctrl->F.filter == 'o');	/* Means weightsum is zero so no normalization, please */
					}
					else if (Ctrl->F.filter == 'h') {	/* Histogram-based mode filter */
						sscanf (&txt[1], "%[^/]/%s", a, b);	/* Get filter width and bin width */
						Ctrl->F.width = atof (a);
						Ctrl->F.bin = atof (b);
						sscanf (&txt[1], "%[^+]/%s", a, b);	/* Look for modifiers */
						if (b[0]) {	/* Gave modifiers */
							if ((p = strstr (b, "+l"))) Ctrl->F.mode = GRDFILTER_MODE_KIND_LOW;
							if ((p = strstr (b, "+u"))) Ctrl->F.mode = GRDFILTER_MODE_KIND_HIGH;
							if ((p = strstr (b, "+c"))) Ctrl->F.center = true;
						}
					}
					else if (Ctrl->F.filter == 'p') {	/* LMS-based mode filter */
						sscanf (&txt[1], "%[^+]/%s", a, b);
						Ctrl->F.width = atof (a);
						if (b[0]) {	/* Gave modifiers */
							if ((p = strstr (b, "+l"))) Ctrl->F.mode = GRDFILTER_MODE_KIND_LOW;
							if ((p = strstr (b, "+u"))) Ctrl->F.mode = GRDFILTER_MODE_KIND_HIGH;
							if ((p = strstr (b, "+s"))) {
								Ctrl->F.span = atof (&p[2]) / 100.0;	/* Got span in percent */
								if (Ctrl->F.span <= 0.0 || Ctrl->F.span > 0.5) {
									GMT_Report (API, GMT_MSG_NORMAL, "ERROR -Fp: Span must be in 0-0.5 range\n");
									n_errors++;
								}
							}
						}
					}
					else if (strchr (txt, '/')) {	/* Gave xwidth/ywidth for rectangular Cartesian filtering */
						sscanf (&txt[1], "%[^/]/%s", a, b);
						Ctrl->F.width = atof (a);
						Ctrl->F.width2 = atof (b);
						Ctrl->F.rect = true;
					}
					else
						Ctrl->F.width = atof (&txt[1]);
					if (Ctrl->F.width < 0.0) Ctrl->F.highpass = true;
					Ctrl->F.width = fabs (Ctrl->F.width);
					if (Ctrl->F.filter == 'h' || Ctrl->F.filter == 'p') {	/* Check for some further info in case of mode filtering */
						c = opt->arg[strlen(txt)-1];
						if (c == '-') Ctrl->F.mode = -1;
						if (c == '+') Ctrl->F.mode = +1;
					}
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -Fx<width>, where x is one of %s\n", GRDFILTER_FILTERS);
					n_errors++;
				}
				break;
			case 'G':	/* Output file */
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':	/* New grid spacings */
				Ctrl->I.active = true;
				strncpy (Ctrl->I.string, opt->arg, GMT_LEN256);	/* Verbatim copy */
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'N':	/* Treatment of NaNs */
				Ctrl->N.active = true;
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
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -Ni|p|r\n");
						n_errors++;
						break;
				}
				break;
			case 'T':	/* Toggle registration */
				Ctrl->T.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	GMT_check_lattice (GMT, Ctrl->I.inc, NULL, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !Ctrl->G.active, "Syntax error -G option: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->D.active, "Syntax error -D option: Choose from p or 0-5\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && (Ctrl->D.mode < GRDFILTER_XY_PIXEL || Ctrl->D.mode > GRDFILTER_GEO_MERCATOR),
				"Syntax error -D option: Choose from p or 0-5\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.mode > GRDFILTER_XY_CARTESIAN && Ctrl->F.rect,
				"Syntax error -F option: Rectangular Cartesian filtering requires -Dp|0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.mode > GRDFILTER_XY_CARTESIAN && Ctrl->F.custom,
				"Syntax error -Ff|o option: Custom Cartesian convolution requires -D0\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->F.active, "Syntax error: -F option is required:\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.quantile < 0.0 || Ctrl->F.quantile > 1.0 ,
				"Syntax error: The quantile must be in the 0-1 range.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.active && Ctrl->F.width == 0.0,
				"Syntax error -F option: filter fullwidth must be nonzero.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.rect && Ctrl->F.width2 <= 0.0,
				"Syntax error -F option: Rectangular y-width filter must be nonzero.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0),
				"Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, GMT->common.R.active && Ctrl->I.active && Ctrl->F.highpass,
				"Syntax error -F option: Highpass filtering requires original -R -I\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdfilter_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}
#ifdef DEBUG
	unsigned int n_conv_tot = 0;
#endif

int GMT_grdfilter (void *V_API, int mode, void *args)
{
	bool fast_way, slow = false, slower = false, same_grid = false;
	bool spherical = false, full_360, visit_check = false, get_weight_sum = true;
	unsigned int n_nan = 0, col_out, row_out, effort_level;
	unsigned int filter_type, one_or_zero = 1, GMT_n_multiples = 0;
	int i, tid = 0, col_in, row_in, ii, jj, *col_origin = NULL, nx_wrap = 0, error = 0;
	uint64_t ij_in, ij_out, ij_wt;
	double x_scale = 1.0, y_scale = 1.0, x_width, y_width, par[GRDFILTER_N_PARS];
	double x_out, wt_sum, last_median = 0.0;
	double x_fix = 0.0, y_fix = 0.0, max_lat;
	double merc_range, *weight = NULL, *x_shift = NULL;
	double wesn[4], inc[2];

	char filter_code[GRDFILTER_N_FILTERS] = {'b', 'c', 'g', 'f', 'o', 'm', 'p', 'h', 'l', 'L', 'u', 'U'};
	char *filter_name[GRDFILTER_N_FILTERS+3] = {"Boxcar", "Cosine Arch", "Gaussian", "Custom", "Operator", "Median", "LMS", "Histogram Mode", "Lower", \
		"Lower+", "Upper", "Upper-", "Weighted Median", "Weighted Mode", "Weighted Histogram Mode"};

	struct GMT_GRID *Gin = NULL, *Gout = NULL, *Fin = NULL, *A = NULL, *L = NULL;
	struct FILTER_INFO F;
	struct GRDFILTER_BIN_MODE_INFO *B = NULL;
	struct GRDFILTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	struct THREAD_STRUCT *threadArg;
#ifdef HAVE_GLIB_GTHREAD
	GThread **threads;
#endif

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdfilter_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdfilter_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdfilter_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	GMT->common.x.n_threads = 1;        /* Default to use only one core (we may change this to max cores) */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdfilter_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdfilter_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdfilter main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grid\n");
	if ((Gin = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get entire grid */
		Return (API->error);
	}

	if (Ctrl->T.active)	/* Make output grid of the opposite registration */
		one_or_zero = (Gin->header->registration == GMT_GRID_PIXEL_REG) ? 1 : 0;
	else
		one_or_zero = (Gin->header->registration == GMT_GRID_PIXEL_REG) ? 0 : 1;

	full_360 = (Ctrl->D.mode > GRDFILTER_XY_CARTESIAN && GMT_grd_is_global (GMT, Gin->header));	/* Periodic geographic grid */

	if (Ctrl->D.mode == GRDFILTER_XY_PIXEL) {	/* Special case where widths are given in pixels */
		if (!doubleAlmostEqual (fmod (Ctrl->F.width, 2.0), 1.0)) {
			GMT_Report (API, GMT_MSG_NORMAL, "ERROR: -Dp requires filter width given as an odd number of pixels\n");
			Return (EXIT_FAILURE);
		}
		Ctrl->F.width *= Gin->header->inc[GMT_X];	/* Scale up to give width */
		if (Ctrl->F.rect) {
			if (!doubleAlmostEqual (fmod (Ctrl->F.width2, 2.0), 1.0)) {
				GMT_Report (API, GMT_MSG_NORMAL, "ERROR: -Dp requires filter y-width given as an odd number of pixels\n");
				Return (EXIT_FAILURE);
			}
			Ctrl->F.width2 *= Gin->header->inc[GMT_X];	/* Rectangular rather than isotropic Cartesian filtering */
		}
		Ctrl->D.mode = GRDFILTER_XY_CARTESIAN;	/* From now on they are the same distances */	
	}

	/* Check range of output area and set i,j offsets, etc.  */

	/* Use the -R region for output (if set); otherwise match input grid domain */
	GMT_memcpy (wesn, (GMT->common.R.active ? GMT->common.R.wesn : Gin->header->wesn), 4, double);
	/* Use the -I increments for output (if set); otherwise match input grid increments */
	GMT_memcpy (inc, (Ctrl->I.active ? Ctrl->I.inc : Gin->header->inc), 2, double);
	if (!full_360) {	/* Sanity checks on x-domain if not geographic */
		if (wesn[XLO] < Gin->header->wesn[XLO]) error = true;
		if (wesn[XHI] > Gin->header->wesn[XHI]) error = true;
	}
	/* Sanity checks on y-domain  */
	if (wesn[YLO] < Gin->header->wesn[YLO]) error = true;
	if (wesn[YHI] > Gin->header->wesn[YHI]) error = true;

	if (error) {
		GMT_Report (API, GMT_MSG_NORMAL, "Output grid domain incompatible with input grid domain.\n");
		Return (EXIT_FAILURE);
	}

	/* BECAUSE IT'S NOT TESTED LEAVE THIS (TEMP) WARNING ON TILL WE ARE SURE IT WORKS */
	if (Ctrl->F.custom && GMT->common.x.n_threads > 1)
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: Custom filter and multi-threading is not gurantied to work.\n");

	/* Allocate space and determine the header for the new grid; croak if there are issues. */
	if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, wesn, inc, \
		!one_or_zero, GMT_NOTSET, Ctrl->G.file)) == NULL) Return (API->error);

	/* We can save time by computing a weight matrix once [or once pr scanline] only
	   if output grid spacing is a multiple of input grid spacing */

	fast_way = (fabs (fmod (Gout->header->inc[GMT_X] / Gin->header->inc[GMT_X], 1.0)) < GMT_CONV4_LIMIT && fabs (fmod (Gout->header->inc[GMT_Y] / Gin->header->inc[GMT_Y], 1.0)) < GMT_CONV4_LIMIT);
	same_grid = !(GMT->common.R.active || Ctrl->I.active || Gin->header->registration == one_or_zero);
	if (!fast_way) {	/* Not optimal... */
		if (Ctrl->F.custom) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: For -Ff or -Fo the input and output grids must be coregistered.\n");
			Return (EXIT_FAILURE);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Your output grid spacing is such that filter-weights must\n");
		GMT_Report (API, GMT_MSG_VERBOSE, "be recomputed for every output node, so expect this run to be slow. Calculations\n");
		GMT_Report (API, GMT_MSG_VERBOSE, "can be speeded up significantly if output grid spacing is chosen to be a multiple\n");
		GMT_Report (API, GMT_MSG_VERBOSE, "of the input grid spacing.  If the odd output grid is necessary, consider using\n");
		GMT_Report (API, GMT_MSG_VERBOSE, "a \'fast\' grid for filtering and then resample onto your desired grid with grdsample.\n");
	}
	if (Ctrl->N.mode == NAN_REPLACE && !same_grid) {
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: -Nr requires co-registered input/output grids, option is ignored\n");
		Ctrl->N.mode = NAN_IGNORE;
	}
	
	col_origin = GMT_memory (GMT, NULL, Gout->header->nx, int);
	if (!fast_way) x_shift = GMT_memory (GMT, NULL, Gout->header->nx, double);

	if (fast_way && Gin->header->registration == one_or_zero) {	/* multiple of grid spacings but one is pix, other is grid so adjust for 1/2 cell */
		x_fix = 0.5 * Gin->header->inc[GMT_X];
		y_fix = 0.5 * Gin->header->inc[GMT_Y];
	}
	if (Ctrl->D.mode > GRDFILTER_XY_CARTESIAN) {	/* Data on a sphere so must check for both periodic and polar wrap-arounds */
		spherical = true;
		/* Compute the wrap-around delta_nx to use [may differ from nx unless a 360 grid] */
		nx_wrap = (int)GMT_get_n (GMT, 0.0, 360.0, Gin->header->inc[GMT_X], GMT_GRID_PIXEL_REG);	/* So we basically bypass the duplicate point at east */
	}	
	if ((A = init_area_weights (GMT, Gin, Ctrl->D.mode, Ctrl->A.file)) == NULL) Return (API->error);	/* Precalculate area weights, optionally save debug grid */
	GMT_memset (&F, 1, struct FILTER_INFO);

	/* Set up the distance scalings for lon and lat, and assign pointer to distance function  */

	if (Ctrl->F.custom) {	/* Get filter-weight grid rather than compute one */
		if ((Fin = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->F.file, NULL)) == NULL) {	/* Get filter-weight grid */
			Return (API->error);
		}
		F.nx = Fin->header->nx;	F.ny = Fin->header->ny;
		if ((F.nx % 2) == 0 || (F.ny % 2) == 0) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: -Ff|o requires an odd number of rows and columns for filter|operator weight grid,\n");
			Return (API->error);
		}
		F.x_half_width = (F.nx - 1) / 2;	F.y_half_width = (F.ny - 1) / 2;
	}

	switch (Ctrl->D.mode) {
		case GRDFILTER_XY_CARTESIAN:	/* Plain, unscaled isotropic Cartesian distances */
			x_scale = y_scale = 1.0;
			F.radius_func = &CartRadius;
			break;
		case GRDFILTER_GEO_CARTESIAN:	/* Plain, scaled (degree to km) isotropic Cartesian distances */
			x_scale = y_scale = GMT->current.proj.DIST_KM_PR_DEG;
			F.radius_func = &CartScaledRadius;
			break;
		case GRDFILTER_GEO_FLATEARTH1:	/* Flat Earth Cartesian distances, xscale fixed at mid latitude */
			x_scale = GMT->current.proj.DIST_KM_PR_DEG * cosd (0.5 * (Gout->header->wesn[YHI] + Gout->header->wesn[YLO]));
			y_scale = GMT->current.proj.DIST_KM_PR_DEG;
			F.radius_func = &FlatEarthRadius;
			break;
		case GRDFILTER_GEO_FLATEARTH2:	/* Flat Earth Cartesian distances, xscale reset for each latitude (xscale here is max scale for max |lat|) */
			x_scale = GMT->current.proj.DIST_KM_PR_DEG * ((fabs (Gout->header->wesn[YLO]) > Gout->header->wesn[YHI]) ? cosd (Gout->header->wesn[YLO]) : cosd (Gout->header->wesn[YHI]));
			y_scale = GMT->current.proj.DIST_KM_PR_DEG;
			F.radius_func = &FlatEarthRadius;
			break;
		case GRDFILTER_GEO_SPHERICAL:	/* Great circle distances; set scale using input grid south/north */
			x_scale = GMT->current.proj.DIST_KM_PR_DEG * ((fabs (Gin->header->wesn[YLO]) > Gin->header->wesn[YHI]) ? cosd (Gin->header->wesn[YLO]) : cosd (Gin->header->wesn[YHI]));
			y_scale = GMT->current.proj.DIST_KM_PR_DEG;
			F.radius_func = &SphericalRadius;
			break;
		case GRDFILTER_GEO_MERCATOR:	/* Great circle distances with Mercator coordinates */
			/* Get the max |lat| extent of the grid */
			max_lat = IMG2LAT (MAX (fabs (Gin->header->wesn[YLO]), fabs (Gin->header->wesn[YHI])));
			merc_range = LAT2IMG (max_lat + (0.5 * Ctrl->F.width / GMT->current.proj.DIST_KM_PR_DEG)) - LAT2IMG (max_lat);
			x_scale = y_scale = 0.5 * Ctrl->F.width / merc_range;
			F.radius_func = &SphericalRadius;
			break;
	}
			
	/* Assign filter_type number */
	
	for (filter_type = 0; filter_type < GRDFILTER_N_FILTERS && filter_code[filter_type] != Ctrl->F.filter; filter_type++);
	
	switch (filter_type) {	/* Set special weight functions for some filters */
		case GRDFILTER_COSINE:	/*  Cosine-bell filter weights */
			par[GRDFILTER_INV_R_SCALE] = (Ctrl->F.rect) ? 1.0 : 2.0 / Ctrl->F.width;
			F.weight_func = &CosBellWeight;
			break;
		case GRDFILTER_GAUSSIAN:	/*  Gaussian filter weights */
			par[GRDFILTER_INV_R_SCALE] = (Ctrl->F.rect) ? -4.5 : -18.0 / (Ctrl->F.width * Ctrl->F.width);
			F.weight_func = &GaussianWeight;
			break;
		default:	/* Everything else uses unit weights or nothing */
			F.weight_func = &UnitWeight;
			break;
	}
	
	/* Set up miscellaneous filter parameters needed when computing the weights */
	
	par[GRDFILTER_WIDTH] = Ctrl->F.width;
	par[GRDFILTER_HALF_WIDTH] = 0.5 * Ctrl->F.width;
	par[GRDFILTER_X_SCALE] = x_scale;
	par[GRDFILTER_Y_SCALE] = (Ctrl->D.mode == GRDFILTER_GEO_MERCATOR) ? GMT->current.proj.DIST_KM_PR_DEG : y_scale;
	F.d_flag = Ctrl->D.mode;
	F.rect = Ctrl->F.rect;
	F.dx = Gin->header->inc[GMT_X];
	F.dy = Gin->header->inc[GMT_Y];
	F.y_min = Gin->header->wesn[YLO];
	F.y_max = Gin->header->wesn[YHI];
	x_width = Ctrl->F.width / (Gin->header->inc[GMT_X] * x_scale);
	y_width = ((F.rect) ? Ctrl->F.width2 : Ctrl->F.width) / (Gin->header->inc[GMT_Y] * y_scale);
	if (!Ctrl->F.custom) {	/* Parameters computed from width and other settings */
		F.x_half_width = irint (ceil (x_width / 2.0));
		F.y_half_width = irint (ceil (y_width / 2.0));
		F.nx = 2 * F.x_half_width + 1;
		F.ny = 2 * F.y_half_width + 1;
		if (GMT_IS_ZERO (x_scale) || F.x_half_width < 0 || F.nx > Gin->header->nx) {	/* Safety valve when x_scale -> 0.0 */
			F.nx = Gin->header->nx;
			F.x_half_width = (F.nx - 1) / 2;
			if ((F.nx - 2 * F.x_half_width - 1) > 0) F.x_half_width++;		/* When nx is even we may come up short by 1 */
			F.nx = 2 * F.x_half_width + 1;
		}
		if (GMT_IS_ZERO (y_scale) || F.y_half_width < 0 || F.ny > Gin->header->ny) {	/* Safety valve when y_scale -> 0.0 */
			F.ny = Gin->header->ny;
			F.y_half_width = (F.ny - 1) / 2;
		}
	}
	visit_check = ((2 * F.x_half_width + 1) >= (int)Gin->header->nx);	/* Must make sure we only visit each node once along a row */
	F.x = GMT_memory (GMT, NULL, F.x_half_width+1, double);
	F.y = GMT_memory (GMT, NULL, F.y_half_width+1, double);
	if (visit_check) F.visit = GMT_memory (GMT, NULL, Gin->header->nx, char);
	for (ii = 0; ii <= F.x_half_width; ii++) F.x[ii] = ii * F.dx;
	for (jj = 0; jj <= F.y_half_width; jj++) F.y[jj] = jj * F.dy;

	if (Ctrl->F.custom) {	/* Read convolution grid from file */
		ij_wt = 0;	wt_sum = 0.0;
		weight = GMT_memory (GMT, NULL, F.nx*F.ny, double);	/* Allocate space for convolution grid */
		GMT_grd_loop (GMT, Fin, row_in, col_in, ij_in) { /* Just copy over to weight array while skipping the padding */
			weight[ij_wt++] = Fin->data[ij_in];
			wt_sum += Fin->data[ij_in];
		}
		if (GMT_Destroy_Data (API, &Fin) != GMT_OK) {	/* Done with this grid */
			Return (API->error);
		}
		if (GMT_IS_ZERO (wt_sum)) {	/* The custom filter is an operator; should have used -Fo */
			GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Your custom filter weights sum to zero; switching to -Fo operator mode.\n");
			Ctrl->F.operator = true;
		}
		if (Ctrl->F.operator) get_weight_sum = false;	/* As weights sum to zero we don't want to add them up and divide */
	}

	if (filter_type > GRDFILTER_OPERATOR) {	/* These filters are not convolutions; they require sorting or comparisons */
		slow = true;
		last_median = 0.5 * (Gin->header->z_min + Gin->header->z_max);	/* Initial guess */
		if (Ctrl->D.mode > GRDFILTER_XY_CARTESIAN && (filter_type == GRDFILTER_MEDIAN || filter_type == GRDFILTER_LMS || filter_type == GRDFILTER_HIST)) {	/* Spherical (weighted) median/modes requires even more work */
			slower = true;
			filter_type += GRDFILTER_CART_TO_SPH;	/* Set to the weighted versions */
		}
		if (filter_type == GRDFILTER_HIST)
			B = grdfilter_bin_setup (GMT, Gin->header->z_min, Gin->header->z_max, Ctrl->F.bin, Ctrl->F.center, Ctrl->F.mode, false);
		else if (filter_type == GRDFILTER_HIST_SPH)
			B = grdfilter_bin_setup (GMT, Gin->header->z_min, Gin->header->z_max, Ctrl->F.bin, Ctrl->F.center, Ctrl->F.mode, true);
	}

	if (tid == 0) {	/* First or only thread */
		GMT_Report (API, GMT_MSG_VERBOSE, "Input nx,ny = (%d %d), output nx,ny = (%d %d), filter (max)nx,ny = (%d %d)\n", Gin->header->nx, Gin->header->ny, Gout->header->nx, Gout->header->ny);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Filter nx,ny = (%d %d) [These are maximum dimensions of weight array]\n", F.nx, F.ny);
		if (Ctrl->F.quantile != 0.5)
			GMT_Report (API, GMT_MSG_VERBOSE, "Filter type is %s [using %g%% quantile].\n",
					filter_name[filter_type], 100.0 * Ctrl->F.quantile);
		else
			GMT_Report (API, GMT_MSG_VERBOSE, "Filter type is %s.\n", filter_name[filter_type]);
#ifdef HAVE_GLIB_GTHREAD
		GMT_Report (API, GMT_MSG_VERBOSE, "Calculations will be distributed over %d threads.\n", GMT->common.x.n_threads);
#endif
	}

#ifdef DEBUG
	if (Ctrl->A.active) {	/* Picked a point to examine filter weights etc */
		Ctrl->A.COL = GMT_grd_x_to_col (GMT, Ctrl->A.x, Gout->header);
		Ctrl->A.ROW = GMT_grd_y_to_row (GMT, Ctrl->A.y, Gout->header);
		if (Ctrl->A.mode == 'r') F.debug = true;	/* In order to return radii instead of weights */
		if (tid == 0) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "ROW = %d COL = %d\n", Ctrl->A.ROW, Ctrl->A.COL);
	}
#endif
	/* Compute nearest xoutput i-indices and shifts once */

	for (col_out = 0; col_out < Gout->header->nx; col_out++) {
		x_out = GMT_grd_col_to_x (GMT, col_out, Gout->header);	/* Current longitude */
		col_origin[col_out] = (int)GMT_grd_x_to_col (GMT, x_out, Gin->header);
		if (!fast_way) x_shift[col_out] = x_out - GMT_grd_col_to_x (GMT, col_origin[col_out], Gin->header);
	}

	/* Determine how much effort to compute weights:
		0 = Weights read from custom weight grid
		1 = Compute weights once for entire grid
		2 = Compute weights once per scanline [slow]
		3 = Compute weights for every output point [slower]
	*/

	if (Ctrl->F.custom)
		effort_level = 0;
	else if (fast_way && Ctrl->D.mode <= GRDFILTER_GEO_FLATEARTH1)
		effort_level = 1;
	else if (fast_way && Ctrl->D.mode >= GRDFILTER_GEO_FLATEARTH2)
		effort_level = 2;
	else 
		effort_level = 3;
	
#ifdef DEBUG
	if (Ctrl->A.active) for (ij_in = 0; ij_in < Gin->header->size; ij_in++) Gin->data[ij_in] = 0.0f;	/* We are using Gin to store filter weights etc instead */
#endif

	GMT_tic(GMT);

#ifdef HAVE_GLIB_GTHREAD
	if (GMT->common.x.n_threads > 1)
		threads = GMT_memory (GMT, NULL, GMT->common.x.n_threads, GThread *);
#endif

	threadArg = GMT_memory (GMT, NULL, GMT->common.x.n_threads, struct THREAD_STRUCT);

	for (i = 0; i < GMT->common.x.n_threads; i++) {
		threadArg[i].GMT        = GMT;
		threadArg[i].Ctrl       = Ctrl;
		threadArg[i].Gin        = Gin;
		threadArg[i].Gout       = Gout;
		threadArg[i].A          = A;
		threadArg[i].L          = L;
   		threadArg[i].F          = F;
   		threadArg[i].B          = B;
   		threadArg[i].weight     = weight;
   		threadArg[i].x_shift    = x_shift;
   		threadArg[i].col_origin = col_origin;
   		threadArg[i].par        = par;
   		threadArg[i].x_fix      = x_fix;
   		threadArg[i].y_fix      = y_fix;
   		threadArg[i].last_median= last_median;
   		threadArg[i].fast_way   = fast_way;
   		threadArg[i].spherical  = spherical;
   		threadArg[i].slow       = slow;
   		threadArg[i].slower     = slower;
   		threadArg[i].get_weight_sum = get_weight_sum;
   		threadArg[i].nx_wrap    = nx_wrap;
   		threadArg[i].effort_level = effort_level;
   		threadArg[i].filter_type  = filter_type;
   		threadArg[i].r_start    = i * irint((Gout->header->ny) / GMT->common.x.n_threads);
   		threadArg[i].thread_num = i;

		if (GMT->common.x.n_threads == 1) {		/* Independently of WITH_THREADS, if only one don't call the threading machine */
   			threadArg[i].r_stop = Gout->header->ny;
			threaded_function (&threadArg[0]);
			break;		/* Make sure we don't go through the threads lines below */
		}
#ifndef HAVE_GLIB_GTHREAD
	}
#else
   		threadArg[i].r_stop = (i + 1) * irint((Gout->header->ny) / GMT->common.x.n_threads);
   		if (i == GMT->common.x.n_threads - 1) threadArg[i].r_stop = Gout->header->ny;	/* Make sure last row is not left behind */
		threads[i] = g_thread_new(NULL, thread_function, (void*)&(threadArg[i]));
	}

	if (GMT->common.x.n_threads > 1) {		/* Otherwise g_thread_new was never called aand so no need to "join" */
		for (i = 0; i < GMT->common.x.n_threads; i++)
			g_thread_join(threads[i]);
	}

	if (GMT->common.x.n_threads > 1)
		GMT_free (GMT, threads);
#endif

	GMT_free (GMT, threadArg);

	GMT_toc(GMT,"");		/* Print total run time, but only if -Vt was set */

	if (weight) GMT_free (GMT, weight);
	GMT_free (GMT, F.x);
	GMT_free (GMT, F.y);
	if (visit_check) GMT_free (GMT, F.visit);
	if (B) GMT_free (GMT, B);

	GMT_free (GMT, col_origin);
	if (!fast_way) GMT_free (GMT, x_shift);
	if (GMT_Destroy_Data (API, &A) != GMT_OK) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to free A\n");
	}

	if (n_nan) GMT_Report (API, GMT_MSG_VERBOSE, "Unable to estimate value at %" PRIu64 " nodes, set to NaN\n", n_nan);
	if (GMT_n_multiples > 0) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: %d multiple modes found by the mode filter\n", GMT_n_multiples);

	if (Ctrl->F.highpass) {
		if (GMT->common.R.active || Ctrl->I.active || GMT->common.r.active) {	/* Must resample result so grids are coregistered */
			int object_ID;			/* Status code from GMT API */
			char in_string[GMT_STR16], out_string[GMT_STR16], cmd[GMT_BUFSIZ];
			/* Here we low-passed filtered onto a coarse grid but to get high-pass we must sample the low-pass result at the original resolution */
			if ((object_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REFERENCE, GMT_IS_SURFACE, GMT_IN, NULL, Gout)) == GMT_NOTSET) {
				Return (API->error);
			}
			if (GMT_Encode_ID (API, in_string, object_ID) != GMT_OK) {	/* Make filename with embedded object ID for grid Gout */
				Return (API->error);
			}
			if ((object_ID = GMT_Register_IO (API, GMT_IS_GRID, GMT_IS_REFERENCE, GMT_IS_SURFACE, GMT_OUT, NULL, NULL)) == GMT_NOTSET) {
				Return (API->error);
			}
			if (GMT_Encode_ID (GMT->parent, out_string, object_ID) != GMT_OK) {
				Return (API->error);	/* Make filename with embedded object ID for result grid L */
			}
			sprintf (cmd, "%s -G%s -R%s -V%d", in_string, out_string, Ctrl->In.file, GMT->current.setting.verbose);
			if (GMT_is_geographic (GMT, GMT_IN)) strcat (cmd, " -fg");
			GMT_Report (API, GMT_MSG_LONG_VERBOSE,
					"Highpass requires us to resample the lowpass result at original registration via grdsample %s\n", cmd);
			if (GMT_Call_Module (GMT->parent, "grdsample", GMT_MODULE_CMD, cmd) != GMT_OK) {	/* Resample the file */
				GMT_Report (API, GMT_MSG_NORMAL, "Error: Unable to resample the lowpass result - exiting\n");
				Return (API->error);
			}
			if ((L = GMT_Retrieve_Data (API, object_ID)) == NULL) {	/* Load in the resampled grid */
				Return (API->error);
			}
			if (GMT_Destroy_Data (API, &Gout) != GMT_OK) {
				Return (API->error);
			}
			GMT_grd_loop (GMT, L, row_out, col_out, ij_out) L->data[ij_out] = Gin->data[ij_out] - L->data[ij_out];
			GMT_grd_init (GMT, L->header, options, true);	/* Update command history only */
		}
		else	/* Coregistered; no need to resample */
			GMT_grd_loop (GMT, Gout, row_out, col_out, ij_out) Gout->data[ij_out] = Gin->data[ij_out] - Gout->data[ij_out];
		GMT_Report (API, GMT_MSG_VERBOSE, "Subtracting lowpass-filtered data from input grid to obtain high-pass filtered data.\n");
	}
	
	/* At last, that's it!  Output: */

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Gout)) Return (API->error);
#ifdef DEBUG
	if (Ctrl->A.active) {	/* Save the debug output instead */
		FILE *fp = fopen ("n_conv.txt", "w");
		fprintf (fp, "%d\n", n_conv_tot);
		fclose (fp);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Gin) != GMT_OK) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &Gout) != GMT_OK) {
			GMT_Report (API, GMT_MSG_NORMAL, "Failed to free Gout\n");
		}
	}
	else
#endif
	if (Ctrl->F.highpass && L) {	/* Save the highpassed-filtered grid instead */

		if (GMT_Set_Comment (GMT->parent, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, "High-pass filtered data", L)) return (GMT->parent->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, L) != GMT_OK) {
			Return (API->error);
		}
	}
	else {
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Gout) != GMT_OK) {
			Return (API->error);
		}
	}

	Return (EXIT_SUCCESS);
}

/* ----------------------------------------------------------------------------------------------------- */
void threaded_function (struct THREAD_STRUCT *t) {

	bool visit_check = false, go_on;
	unsigned int n_in_median, n_nan = 0, col_out, row_out, n_span;
	unsigned int one_or_zero = 1, GMT_n_multiples = 0;
	int tid = 0, col_in, row_in, ii, jj, row_origin;
	char *visit; 
#ifdef DEBUG
	unsigned int n_conv = 0;
#endif
	uint64_t ij_in, ij_out, ij_wt;
	double y, y_out, wt_sum, value, this_estimate = 0.0;
	double y_shift = 0.0, lat_out, w;
	double *work_array = NULL, *weight = NULL;
	struct GMT_OBSERVATION *work_data = NULL;

	/* Convenience vars */
   	bool   fast_way             = t->fast_way;
   	bool   spherical            = t->spherical;
   	bool   slow                 = t->slow;
   	bool   slower               = t->slower;
   	bool   get_weight_sum       = t->get_weight_sum;
    int    *col_origin          = t->col_origin;
    int    nx_wrap              = t->nx_wrap;
    unsigned int r_start        = t->r_start;
    unsigned int r_stop         = t->r_stop;
    unsigned int effort_level   = t->effort_level;
    unsigned int filter_type    = t->filter_type;
    double *x_shift             = t->x_shift;
    double *par                 = t->par;
    double x_fix                = t->x_fix;
    double y_fix                = t->y_fix;
    double last_median          = t->last_median;
    struct GMT_CTRL *GMT        = t->GMT;
    struct GRDFILTER_CTRL *Ctrl = t->Ctrl;
    struct GMT_GRID *Gin        = t->Gin;
    struct GMT_GRID *Gout       = t->Gout;
    struct GMT_GRID *A          = t->A;
    struct GMT_GRID *L          = t->L;
    struct FILTER_INFO F        = t->F;
    struct GRDFILTER_BIN_MODE_INFO *B = t->B;

	/* We need a local copy of these becuase they are modified in this function */
	visit = GMT_memory (GMT, NULL, Gin->header->nx, char);
	if (!weight)
		weight = GMT_memory(GMT, NULL, F.nx*F.ny, double);	/* Allocate space for convolution grid */
	else
		weight = t->weight;	/* The F.custom case, where weights were obtained in main and allows NOT multi-threading */

	if (effort_level == 1) set_weight_matrix (GMT, &F, weight, 0.0, par, x_fix, y_fix);

	if (slow) {
		if (slower)		/* Spherical (weighted) median/modes requires even more work */
			work_data = GMT_memory (GMT, NULL, F.nx*F.ny, struct GMT_OBSERVATION);
		else
			work_array = GMT_memory (GMT, NULL, F.nx*F.ny, double);
	}

	if (Ctrl->T.active)	/* Make output grid of the opposite registration */
		one_or_zero = (Gin->header->registration == GMT_GRID_PIXEL_REG) ? 1 : 0;
	else
		one_or_zero = (Gin->header->registration == GMT_GRID_PIXEL_REG) ? 0 : 1;

	for (row_out = r_start; row_out < r_stop; row_out++) {

		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Processing output line %d\r", row_out);
#ifdef DEBUG
		if (Ctrl->A.active && row_out != Ctrl->A.ROW) continue;		/* Not at our selected row for testing */
#endif
		y_out = GMT_grd_row_to_y (GMT, row_out, Gout->header);		/* Current output y [or latitude] */
		lat_out = (Ctrl->D.mode == GRDFILTER_GEO_MERCATOR) ? IMG2LAT (y_out) : y_out;	/* Adjust lat if IMG grid */
		row_origin = (int)GMT_grd_y_to_row (GMT, y_out, Gin->header);		/* Closest row in input grid */
		if (Ctrl->D.mode == GRDFILTER_GEO_FLATEARTH2)
			par[GRDFILTER_X_SCALE] = GMT->current.proj.DIST_KM_PR_DEG * cosd (lat_out);	/* Update flat-Earth longitude scale */

		if (Ctrl->D.mode > GRDFILTER_GEO_FLATEARTH1) {	/* Update max filterweight nodes to deal with for this latitude */
			unsigned int test_nx;
			y = fabs (lat_out);
			if (Ctrl->D.mode == GRDFILTER_GEO_SPHERICAL)
				y += (par[GRDFILTER_HALF_WIDTH] / par[GRDFILTER_Y_SCALE]);	/* Highest latitude within filter radius */
			test_nx = urint (par[GRDFILTER_HALF_WIDTH] / (F.dx * par[GRDFILTER_Y_SCALE] * cosd (y)));
			F.x_half_width = (y < 90.0) ? MIN ((F.nx - 1) / 2, test_nx) : (F.nx - 1) / 2;
			if (y > 90.0 && (F.nx - 2 * F.x_half_width - 1) > 0) F.x_half_width++;	/* When nx is even we may come up short by 1 */
			visit_check = ((2 * F.x_half_width + 1) >= (int)Gin->header->nx);	/* Must make sure we only visit each node once along a row */
		}

		if (effort_level == 2) set_weight_matrix (GMT, &F, weight, y_out, par, x_fix, y_fix);	/* Compute new weights for this latitude */
		if (!fast_way) y_shift = y_out - GMT_grd_row_to_y (GMT, row_origin, Gin->header);

		for (col_out = 0; col_out < Gout->header->nx; col_out++) {
#ifdef DEBUG
			if (Ctrl->A.active && col_out != Ctrl->A.COL) continue;	/* Not at our selected column for testing */
#endif
			ij_out = GMT_IJP (Gout->header, row_out, col_out);	/* Node of current output point */
			if (Ctrl->N.mode == NAN_REPLACE && GMT_is_fnan (Gin->data[ij_out])) {
				/* [Here we know ij_out == ij_in]. Since output will be NaN we bypass the filter loop */
				Gout->data[ij_out] = GMT->session.f_NaN;
				n_nan++;
				continue;	/* Done with this node */
			}

			if (effort_level == 3) /* Update weights for this location */
				set_weight_matrix (GMT, &F, weight, y_out, par, x_shift[col_out], y_shift);
			wt_sum = value = 0.0;	n_in_median = 0;	go_on = true;	/* Reset all counters */
#ifdef DEBUG
			n_conv = 0;	/* Just to check # of points in the convolution - not used in any calculations */
#endif
			/* Now loop over the filter domain and collect those points that should be considered by the filter operation */

			for (jj = -F.y_half_width; go_on && jj <= F.y_half_width; jj++) {	/* Possible -/+ rows to consider for filter input */
				row_in = row_origin + jj;		/* Current input data row number */
				if (row_in < 0 || (row_in >= (int)Gin->header->ny)) continue;	/* Outside input y-range */
				if (visit_check) GMT_memset (visit, Gin->header->nx, char);	/* Reset our longitude visit counter */
				for (ii = -F.x_half_width; go_on && ii <= F.x_half_width; ii++) {	/* Possible -/+ columns to consider on both sides of input point */
					col_in = col_origin[col_out] + ii;	/* Input column to consider */
					if (spherical) {			/* Must handle wrapping around the globe */
						if (col_in < 0) col_in += nx_wrap;	/* "Left" of west means we might reappear in the east */
						else if (col_in >= nx_wrap) col_in -= nx_wrap;	/* Likewise if we are "right" of east */
					}
					if (col_in < 0 || (col_in >= (int)Gin->header->nx)) continue;	/* Still outside range of original input grid */
					if (visit_check) {	/* Make sure we never include the same node twice along a given row */
						if (visit[col_in]) continue;		/* Already been used */
						visit[col_in] = 1;			/* Now marked as visited */
					}
					ij_wt = WT_IJ (F, jj, ii);		/* Get weight array index */
					if (weight[ij_wt] <= 0.0 && get_weight_sum) continue;	/* Negative weight [and not operator] means we are outside the filter circle */

					ij_in = GMT_IJP (Gin->header, row_in, col_in);	/* Finally, the current input data point inside the filter */
					if (GMT_is_fnan (Gin->data[ij_in])) {		/* Oops, found a rotten apple, skip */
						if (Ctrl->N.mode == NAN_PRESERVE) go_on = false;	/* -Np means no NaNs are allowed */
						continue;
					}

					/* Get here when point is inside and usable  */

					if (slow) {	/* Add it to the relevant temporary work array */
						if (slower) {	/* Need to store both value and weight */
							work_data[n_in_median].value = Gin->data[ij_in];
							work_data[n_in_median++].weight = (float)(weight[ij_wt] * A->data[ij_in]);
						}
						else	/* Only need to store values */
							work_array[n_in_median++] = Gin->data[ij_in];
					}
					else {	/* Update weighted sum for our convolution */
						w = weight[ij_wt] * A->data[ij_in];
						value += Gin->data[ij_in] * w;
						if (get_weight_sum) wt_sum += w;
#ifdef DEBUG
						n_conv++;	/* Points used inside filter circle */
						if (Ctrl->A.active) {	/* Store selected debug info in Gin data */
							if (Ctrl->A.mode == 'a') Gin->data[ij_in] = A->data[ij_in];
							else if (Ctrl->A.mode == 'w') Gin->data[ij_in] = (float)weight[ij_wt];
							else if (Ctrl->A.mode == 'r') Gin->data[ij_in] = (float)weight[ij_wt];	/* holds r */
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
						case GRDFILTER_MEDIAN:	/* Median */
							GMT_median (GMT, work_array, n_in_median, Gin->header->z_min, Gin->header->z_max, last_median, &this_estimate);
							last_median = this_estimate;
							break;
						case GRDFILTER_LMS:	/* Mode */
							n_span = urint (Ctrl->F.span * n_in_median);
							GMT_mode (GMT, work_array, n_in_median, n_span, true, Ctrl->F.mode, &GMT_n_multiples, &this_estimate);
							break;
						case GRDFILTER_HIST:	/* Histogram peak */
							this_estimate = GMT_histmode (GMT, work_array, n_in_median, B);
							break;
						case GRDFILTER_MIN:	/* Lowest of all values */
							this_estimate = GMT_extreme (GMT, work_array, n_in_median, DBL_MAX, 0, -1);
							break;
						case GRDFILTER_MINPOS:	/* Lowest of positive values */
							this_estimate = GMT_extreme (GMT, work_array, n_in_median, 0.0, +1, -1);
							break;
						case GRDFILTER_MAX:	/* Upper of all values */
							this_estimate = GMT_extreme (GMT, work_array, n_in_median, -DBL_MAX, 0, +1);
							break;
						case GRDFILTER_MAXNEG:	/* Upper of negative values */
							this_estimate = GMT_extreme (GMT, work_array, n_in_median, 0.0, -1, +1);
							break;
						case GRDFILTER_MEDIAN_SPH:	/* Weighted Median */
							this_estimate = GMT_median_weighted (GMT, work_data, n_in_median, Ctrl->F.quantile);
							break;
						case GRDFILTER_LMS_SPH: /* Weighted Mode */
							// n_span = urint (Ctrl->F.span * n_in_median);
							this_estimate = GMT_mode_weighted (GMT, work_data, n_in_median);
							break;
						case GRDFILTER_HIST_SPH: /* Weighted histogram Mode */
							this_estimate = GMT_histmode_weighted (GMT, work_data, n_in_median, B);
							break;
					}
					Gout->data[ij_out] = (float)this_estimate;	/* Truncate to float */
				}
				else {	/* Nothing found inside circle, set output node to NaN */
					Gout->data[ij_out] = GMT->session.f_NaN;
					n_nan++;
				}
			}
			else if (get_weight_sum) {	/* Convolution filters */
				if (wt_sum == 0.0) {	/* Nothing found inside circle or rectangle, assign value = GMT->session.f_NaN */
					Gout->data[ij_out] = GMT->session.f_NaN;
					n_nan++;
				}
				else	/* Safe to compute weighted average */
					Gout->data[ij_out] = (float)(value / wt_sum);
			}
			else	/* Operator; no weight normalization needed */
				Gout->data[ij_out] = (float)value;
		}
	}

	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Processing output line %d\n", row_out);

	if (slow) {
		if (slower) GMT_free (GMT, work_data);
		else GMT_free (GMT, work_array);
	}
#ifdef DEBUG
	n_conv_tot += n_conv;
#endif
	GMT_free(GMT, visit);
	GMT_free(GMT, weight);
}
