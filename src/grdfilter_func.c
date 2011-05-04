/*--------------------------------------------------------------------
 *	$Id: grdfilter_func.c,v 1.11 2011-05-04 19:33:17 guru Exp $
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

struct GRDFILTER_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct D {	/* -D<distflag> */
		GMT_LONG active;
		GMT_LONG mode;
	} D;
	struct F {	/* <type>[-]<filter_width>[<mode>] */
		GMT_LONG active;
		GMT_LONG highpass;
		char filter;	/* Character codes for the filter */
		double width;
		GMT_LONG mode;
	} F;
	struct G {	/* -G<file> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct N {	/* -Np|i|r */
		GMT_LONG active;
		GMT_LONG mode;	/* 0 is default (i), 1 is replace (r), 2 is preserve (p) */
	} N;
	struct T {	/* -T */
		GMT_LONG active;
	} T;
};

#define IMG2LAT(img) (2.0*atand(exp((img)*D2R))-90.0)
#define LAT2IMG(lat) (R2D*log(tand(0.5*((lat)+90.0))))

#define GRDFILTER_WIDTH		0
#define GRDFILTER_HALF_WIDTH	1
#define GRDFILTER_X_SCALE	2
#define GRDFILTER_Y_SCALE	3
#define GRDFILTER_INV_R_SCALE	4

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
	double dx, dy;		/* Grid spacing in original units */
	double *x, *y;		/* Distances in original units along x and y to distance nodes */
	double par[5];		/* [0] is filter width, [1] is 0.5*filter_width, [2] is xscale, [3] is yscale, [4] is 1/r_half for filter */
	double x_off, y_off;	/* Offsets relative to original grid */
	PFD weight_func, radius_func;
};

void *New_grdfilter_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDFILTER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDFILTER_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->D.mode = -1;	
	return ((void *)C);
}

void Free_grdfilter_Ctrl (struct GMT_CTRL *GMT, struct GRDFILTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}

EXTERN_MSC double GMT_great_circle_dist_meter (struct GMT_CTRL *C, double x0, double y0, double x1, double y1);

void set_weight_matrix (struct GMT_CTRL *GMT, struct FILTER_INFO *F, double *weight, double output_lat, double par[], double x_off, double y_off)
{
	/* x_off and y_off give offset between output node and 'origin' input node for this window (0,0 for integral grids).
	 * fast is TRUE when input/output grids are offset by integer values in dx/dy.
	 * Here, par[0] = filter_width, par[1] = filter_width / 2, par[2] = x_scale, part[3] = y_scale, and
	 * par[4] is the normalization distance needed for the Cosine-bell or Gaussian weight function.
	 */

	GMT_LONG i, j, ij;
	double x, y, yc, y0, x_jump = 0.0, normalize = 1.0, r, dy = 1.0, dy_half = 0.5, dy_shrink = 1.0;

	yc = y0 = output_lat - y_off;
	if (F->d_flag == 5) {
		yc = IMG2LAT (yc);	/* Recover actual latitudes */
		dy = F->y[1] - F->y[0];
		dy_half = 0.5 * dy;
	}
	for (j = -F->y_half_width; j <= F->y_half_width; j++) {
		y = y0 + ((j < 0) ? -F->y[-j] : F->y[j]);
		if (F->d_flag) {	/* Must normalize based on shrinking area representation and check for going over the pole */
			if (F->d_flag == 5) {
				dy_shrink = (IMG2LAT (y + dy_half) - IMG2LAT (y - dy_half)) / dy;		/* Recover actual latitudes */
				y = IMG2LAT (y);	/* Recover actual latitudes */
			}
			if (fabs (y) > 90.0) {	/* Must find the point across the pole */
				y = copysign (180.0 - fabs (y), y);
				x_jump = 180.0;
			}
			else
				x_jump = 0.0;
			normalize = dy_shrink * cosd (y);
		}
		for (i = -F->x_half_width; i <= F->x_half_width; i++) {
			x = (i < 0) ? -F->x[-i] : F->x[i];
			ij = (j + F->y_half_width) * F->nx + i + F->x_half_width;
			r = F->radius_func (GMT, x_off, yc, x + x_jump, y, par);
			weight[ij] = (r > par[GRDFILTER_HALF_WIDTH]) ? -1.0 : F->weight_func (r, par);
			if (F->d_flag) weight[ij] *= normalize;	/* Adjust for variation in area with latitude */
		}
	}
}

/* Various functions that will be accessed via pointers */
double CartRadius (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1, double par[])
{	/* Plain Cartesian distance */
	return (hypot (x0 - x1, y0 - y1));
}

double CartScaledRadius (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1, double par[])
{	/* Plain scaled Cartesian distance (xscale = yscale) */
	return (par[GRDFILTER_X_SCALE] * hypot (x0 - x1, y0 - y1));
}

double FlatEarthRadius (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1, double par[])
{	/* Cartesian radius with different scales */
	return (hypot (par[GRDFILTER_X_SCALE] * (x0 - x1), par[GRDFILTER_Y_SCALE] * (y0 - y1)));
}

double SphericalRadius (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1, double par[])
{	/* Great circle distance with polar wrap-around test on 2nd point */
	if (fabs (y1) > 90.0) {	/* Must find the point across the pole */
		y1 = copysign (180.0 - fabs (y1), y1);
		x1 += 180.0;
	}
	return (0.001 * GMT_great_circle_dist_meter (GMT, x0, y0, x1, y1));
}

double UnitWeight (double r, double par[])
{	/* Return unit weight since we know r is inside radius */
	return (1.0);
}

double CosBellWeight (double r, double par[])
{	/* Return the cosine-bell filter weight for given r.
	 * The parameter r_f_half is the 5th parameter passed.
	 */

	return (1.0 + cos (M_PI * r * par[GRDFILTER_INV_R_SCALE]));
}

double GaussianWeight (double r, double par[])
{	/* Return the Gaussian filter weight for given r.
	 * The parameter sig_2 is the 5th parameter passed.
	 */

	return (exp (r * r * par[GRDFILTER_INV_R_SCALE]));
}

GMT_LONG GMT_grdfilter_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdfilter %s [API%s] - Filter a 2-D grid file in the space (or time) domain\n\n", GMT_VERSION, GMT_MP);
	GMT_message (GMT, "usage: grdfilter input_file -D<distance_flag> -F<type>[-]<filter_width>[<mode>]\n");
	GMT_message (GMT, "\t-G<output_file> [%s] [-Ni|p|r] [%s] [-T] [%s] [%s]\n", GMT_I_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-D Distance flag determines how grid (x,y) maps into distance units of filter width as follows:\n");
	GMT_message (GMT, "\t   Cartesian (x, y) Data:\n");
	GMT_message (GMT, "\t     -D0 grid x,y same units as <filter_width>, Cartesian distances.\n");
	GMT_message (GMT, "\t   Geographic (lon,lat) Data:\n");
	GMT_message (GMT, "\t     -D1 grid x,y in degrees, <filter_width> in km, Cartesian distances.\n");
	GMT_message (GMT, "\t     -D2 grid x,y in degrees, <filter_width> in km, x_scaled by cos(middle y), Cartesian distances.\n");
	GMT_message (GMT, "\t   These first three options are faster; they allow weight matrix to be computed only once.\n");
	GMT_message (GMT, "\t   Next three options are slower; weights must be recomputed for each scan line.\n");
	GMT_message (GMT, "\t     -D3 grid x,y in degrees, <filter_width> in km, x_scale varies as cos(y), Cartesian distances.\n");
	GMT_message (GMT, "\t     -D4 grid x,y in degrees, <filter_width> in km, spherical distances.\n");
	GMT_message (GMT, "\t     -D5 grid x,y in Mercator units (-Jm1), <filter_width> in km, spherical distances.\n");
	GMT_message (GMT, "\t-F Sets the low-pass filter type and full diameter (6 sigma) filter-widtGin->header->  Choose between\n");
	GMT_message (GMT, "\t   convolution-type filters which differ in how weights are assigned and geospatial\n");
	GMT_message (GMT, "\t   filters that seek to return a representative value.\n");
	GMT_message (GMT, "\t   Give negative filter width to select highpass filtering [lowpass].\n");
	GMT_message (GMT, "\t   Convolution filters:\n");
	GMT_message (GMT, "\t     b: Boxcar : a simple averaging of all points inside filter radius.\n");
	GMT_message (GMT, "\t     c: Cosine arch : a weighted averaging with cosine arc weights.\n");
	GMT_message (GMT, "\t     g: Gaussian : weighted averaging with Gaussian weights.\n");
	GMT_message (GMT, "\t   Geospatial filters:\n");
	GMT_message (GMT, "\t     l: Lower : return minimum of all points.\n");
	GMT_message (GMT, "\t     L: Lower+ : return minimum of all +ve points.\n");
	GMT_message (GMT, "\t     m: Median : return the median value of all points.\n");
	GMT_message (GMT, "\t     p: Maximum likelihood probability estimator : return mode of all points.\n");
	GMT_message (GMT, "\t        By default, we return the average if more than one mode is found.\n");
	GMT_message (GMT, "\t        Append - or + to the width to instead return the smallest or largest mode.\n");
	GMT_message (GMT, "\t     u: Upper : return maximum of all points.\n");
	GMT_message (GMT, "\t     U: Upper- : return maximum of all -ve points.\n");
	GMT_message (GMT, "\t-G Gets output filename for filtered grid.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_message (GMT, "\t   The new xinc and yinc should be divisible by the old ones (new lattice is subset of old).\n");
	GMT_message (GMT, "\t-N Specifies how NaNs in the input grid should be treated.  There are three options:\n");
	GMT_message (GMT, "\t   -Ni skips all NaN values and returns a filtered value unless all are NaN [Default].\n");
	GMT_message (GMT, "\t   -Np sets filtered output to NaN is any NaNs are found inside filter circle.\n");
	GMT_message (GMT, "\t   -Nr sets filtered output to NaN if the corresponding input node was NaN.\n");
	GMT_message (GMT, "\t      (only possible if the input and output grids are coregistered).\n");
	GMT_message (GMT, "\t-T Toggles between grid and pixel registration for output grid [Default is same as input registration].\n");
	GMT_message (GMT, "\t-R For new Range of output grid; enter <WESN> (xmin, xmax, ymin, ymax) separated by slashes.\n");
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
	char c;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Distance mode */
				Ctrl->D.active = TRUE;
				Ctrl->D.mode = atoi (opt->arg);
				break;
			case 'F':	/* Filter */
				if (strchr ("bcgmpLlUu", opt->arg[0])) {	/* OK filter code */
					Ctrl->F.active = TRUE;
					Ctrl->F.filter = opt->arg[0];
					Ctrl->F.width = atof (&opt->arg[1]);
					if (Ctrl->F.width < 0.0) Ctrl->F.highpass = TRUE;
					Ctrl->F.width = fabs (Ctrl->F.width);
					if (Ctrl->F.filter == 'p') {	/* Check for some futher info in case of mode filtering */
						c = opt->arg[strlen(opt->arg)-1];
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

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	GMT_check_lattice (GMT, Ctrl->I.inc, NULL, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.mode < 0 || Ctrl->D.mode > 5, "Syntax error -D option: Choose from the range 0-5\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->F.active, "Syntax error: -F option is required:\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.active && Ctrl->F.width == 0.0, "Syntax error -F option: filter fullwidth must be nonzero:\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0), "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, GMT->common.R.active && Ctrl->I.active && Ctrl->F.highpass, "Syntax error -F option: Highpass filtering requires original -R -I\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdfilter_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdfilter (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG n_in_median, n_nan = 0, i_west, i_east, j_s_pole, j_n_pole, tid = 0;
	GMT_LONG j_origin, col_out, row_out, half_nx, i_orig = 0, n_bad = 0, skip_if_NaN;
	GMT_LONG col_in, row_in, ii, jj, i, j, ij_in, ij_out, ij_wt, effort_level;
	GMT_LONG filter_type, one_or_zero = 1, GMT_n_multiples = 0, *i_origin = NULL;
	GMT_LONG error = FALSE, fast_way, slow = FALSE, same_grid = FALSE, nx_wrap = 0;
	GMT_LONG spherical = FALSE, pole_check = FALSE, sp_wrap_row = 0, np_wrap_row = 0;
	GMT_LONG full_360, duplicate_check, i_west_used, i_east_used, row_ok, new_col;

	double x_scale = 1.0, y_scale = 1.0, x_width, y_width, y, par[5], pole_weight;
	double x_out, y_out, wt_sum, value, last_median = 0.0, this_median = 0.;
	double y_shift = 0.0, x_fix = 0.0, y_fix = 0.0, max_lat, lat_out;
	double merc_range, wesn[4], inc[2], *weight = NULL, *work_array = NULL, *x_shift = NULL;

	char filter_code[GRDFILTER_N_FILTERS] = {'b', 'c', 'g', 'm', 'p', 'l', 'L', 'u', 'U'};
	char *filter_name[GRDFILTER_N_FILTERS] = {"Boxcar", "Cosine Arch", "Gaussian", "Median", "Mode", "Lower", "Lower+", "Upper", "Upper-"};

	struct GMT_GRID *Gin = NULL, *Gout = NULL;
	struct FILTER_INFO F;
	struct GRDFILTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdfilter_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdfilter_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdfilter", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRf:", "", options))) Return (error);
	Ctrl = (struct GRDFILTER_CTRL *) New_grdfilter_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdfilter_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdfilter main code ----------------------------*/

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&(Ctrl->In.file), (void **)&Gin)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (Ctrl->T.active)	/* Make output grid of the opposite registration */
		one_or_zero = (Gin->header->registration == GMT_PIXEL_REG) ? 1 : 0;
	else
		one_or_zero = (Gin->header->registration == GMT_PIXEL_REG) ? 0 : 1;

	full_360 = (Ctrl->D.mode && GMT_360_RANGE (Gin->header->wesn[XHI], Gin->header->wesn[XLO]));	/* Periodic geographic grid */

	/* Check range of output area and set i,j offsets, etc.  */

	Gout = GMT_create_grid (GMT);
	GMT_grd_init (GMT, Gout->header, options, TRUE);	/* Update command history only */
	/* Use the -R region for output if set; otherwise match grid domain */
	GMT_memcpy (wesn, (GMT->common.R.active ? GMT->common.R.wesn : Gin->header->wesn), 4, double);
	if (Ctrl->I.active)
		GMT_memcpy (inc, Ctrl->I.inc, 2, double);
	else
		GMT_memcpy (inc, Gin->header->inc, 2, double);
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
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Gout, wesn, inc, !one_or_zero), Ctrl->G.file);

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
		nx_wrap = GMT_get_n (0.0, 360.0, Gin->header->inc[GMT_X], Gin->header->registration);
		/* Determine the equivalent row numbers or the S-most row after going across the S pole and up along the other meridian */
		sp_wrap_row = 2 * GMT_grd_y_to_row (-90.0, Gin->header) - Gin->header->registration;
		/* Determine the equivalent row numbers or the N-most row after going across the N pole and down along the other meridian */
		np_wrap_row = 2 * GMT_grd_y_to_row (90.0, Gin->header) - (Gin->header->ny - 1) + Gin->header->registration;
	}	

	/* Set up the distance scalings for lon and lat, and assign pointer to distance function  */
#ifdef _OPENMP
#pragma omp parallel shared(fast_way,x_shift,i_origin) private(F,par,x_width,y_width,i,j,effort_level,tid,weight,work_array,half_nx,pole_weight,duplicate_check,j_s_pole,j_n_pole,i_west,i_east) firstprivate(x_scale,y_scale,filter_type,spherical,max_lat,merc_range,slow,pole_check,x_fix,y_fix) reduction(+:n_nan)
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
		case 3:	/* Flat Earth Cartesian distances, xscale reset for each latitude */
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
			par[GRDFILTER_INV_R_SCALE] = 2.0 / Ctrl->F.width;
			F.weight_func = CosBellWeight;
			break;
		case 2:	/*  Gaussian filter weights */
			par[GRDFILTER_INV_R_SCALE] = -18.0 / (Ctrl->F.width * Ctrl->F.width);
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
	F.dx = Gin->header->inc[GMT_X];
	F.dy = Gin->header->inc[GMT_Y];
	x_width = Ctrl->F.width / (Gin->header->inc[GMT_X] * x_scale);
	y_width = Ctrl->F.width / (Gin->header->inc[GMT_Y] * y_scale);
	F.y_half_width = (GMT_LONG) (ceil(y_width) / 2.0);
	F.x_half_width = (GMT_LONG) (ceil(x_width) / 2.0);
	F.nx = 2 * F.x_half_width + 1;
	F.ny = 2 * F.y_half_width + 1;
	if (x_scale == 0.0 || F.nx < 0 || F.nx > Gin->header->nx) {	/* Safety valve when x_scale -> 0.0 */
		F.nx = Gin->header->nx + 1;
		F.x_half_width = Gin->header->nx / 2;
	}
	if (F.ny < 0 || F.ny > Gin->header->ny) {
		F.ny = Gin->header->ny;
		F.y_half_width = Gin->header->ny / 2;
	}
	F.x = GMT_memory (GMT, NULL, F.x_half_width+1, double);
	F.y = GMT_memory (GMT, NULL, F.y_half_width+1, double);
	for (i = 0; i <= F.x_half_width; i++) F.x[i] = i * F.dx;
	for (j = 0; j <= F.y_half_width; j++) F.y[j] = j * F.dy;
	
	weight = GMT_memory (GMT, NULL, F.nx*F.ny, double);

	if (filter_type >= 3) {	/* These filters are not convolutions; they require sorting or comparisons */
		slow = TRUE;
		last_median = 0.5 * (Gin->header->z_min + Gin->header->z_max);	/* Initial guess */
		work_array = GMT_memory (GMT, NULL, F.nx*F.ny, double);
	}

	if (tid == 0) {	/* First or only thread */
		GMT_report (GMT, GMT_MSG_NORMAL, "Input nx,ny = (%d %d), output nx,ny = (%d %d), filter nx,ny = (%ld %ld)\n", Gin->header->nx, Gin->header->ny, Gout->header->nx, Gout->header->ny, F.nx, F.ny);
		GMT_report (GMT, GMT_MSG_NORMAL, "Filter type is %s.\n", filter_name[filter_type]);
#ifdef _OPENMP
		GMT_report (GMT, GMT_MSG_NORMAL, "Calculations will be distributed over %d threads.\n", omp_get_num_threads ());
#endif
	}

	/* Compute nearest xoutput i-indices and shifts once */

	for (col_out = 0; col_out < Gout->header->nx; col_out++) {
		x_out = GMT_grd_col_to_x (col_out, Gout->header);	/* Current longitude */
		i_origin[col_out] = GMT_grd_x_to_col (x_out, Gin->header);
		if (!fast_way) x_shift[col_out] = x_out - GMT_grd_col_to_x (i_origin[col_out], Gin->header);
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
	half_nx = (Gin->header->registration == GMT_PIXEL_REG) ? nx_wrap / 2 : (nx_wrap - 1) / 2;
	pole_weight = 0.25 * M_PI * (Gin->header->inc[GMT_Y] / Gin->header->inc[GMT_X]);	/* This is the weight of the pole point for gridregistered grids */
	pole_check = (Ctrl->D.mode && Gin->header->registration == GMT_GRIDLINE_REG);		/* Must check to make sure we only use the N and S pole value once */
	duplicate_check = (full_360 && Gin->header->registration == GMT_GRIDLINE_REG);		/* Must avoid using the duplicated Greenwich node twice */
	j_s_pole = (Gin->header->wesn[YLO] == -90.0) ? Gin->header->ny - 1 : INT_MIN;		/* row number of S pole (if in the grid)*/
	j_n_pole = (Gin->header->wesn[YHI] == 90.0) ? 0 : INT_MIN;				/* row number of N pole (if in the grid)*/
	i_west = 0;			/* column number of western edge */
	i_east = Gin->header->nx - 1;	/* column number of eastern edge */
	
#ifdef _OPENMP
#pragma omp for schedule(dynamic,1) private(row_out,y_out,lat_out,j_origin,y,col_out,wt_sum,value,n_in_median,n_bad,ij_out,i_west_used,i_east_used,ii,col_in,i_orig,jj,row_in,row_ok,ij_in,ij_wt) firstprivate(y_shift) nowait
#endif
	for (row_out = 0; row_out < Gout->header->ny; row_out++) {

#ifdef _OPENMP
		GMT_report (GMT, GMT_MSG_NORMAL, " Thread %ld Processing output line %ld\r", tid, row_out);
#else
		GMT_report (GMT, GMT_MSG_NORMAL, "Processing output line %ld\r", row_out);
#endif
		y_out = GMT_grd_row_to_y (row_out, Gout->header);		/* Current output y [or latitude] */
		lat_out = (Ctrl->D.mode == 5) ? IMG2LAT (y_out) : y_out;	/* Adjust lat if IMG grid */
		j_origin = GMT_grd_y_to_row (y_out, Gin->header);		/* Closest row in input grid */
		if (Ctrl->D.mode == 3) par[GRDFILTER_X_SCALE] = GMT->current.proj.DIST_KM_PR_DEG * cosd (lat_out);	/* Update flat-earth longitude scale */

		if (Ctrl->D.mode > 2) {	/* Update max filterweight nodes to deal with for this latitude */
			y = fabs (lat_out);
			if (Ctrl->D.mode == 4) y += (par[GRDFILTER_HALF_WIDTH] / par[GRDFILTER_Y_SCALE]);
			F.x_half_width = (y < 90.0) ? MIN (F.nx / 2, irint (par[GRDFILTER_HALF_WIDTH] / (F.dx * par[GRDFILTER_Y_SCALE] * cosd (y)))) : F.nx / 2;
		}
			
		if (effort_level == 2) set_weight_matrix (GMT, &F, weight, y_out, par, x_fix, y_fix);	/* Compute new weights */
		if (!fast_way) y_shift = y_out - GMT_grd_row_to_y (j_origin, Gin->header);

		for (col_out = 0; col_out < Gout->header->nx; col_out++) {

			if (effort_level == 3) set_weight_matrix (GMT, &F, weight, y_out, par, x_shift[col_out], y_shift);
			wt_sum = value = 0.0;
			n_in_median = n_bad = 0;
			ij_out = GMT_IJP (Gout->header, row_out, col_out);	/* Node of current output point */
			skip_if_NaN = (Ctrl->N.mode == NAN_REPLACE && GMT_is_fnan (Gin->data[ij_out]));	/* Since output will be NaN we bypass the filter loop */

			/* Now loop over the filter domain and collect those points that should be considered by the filter operation */
			
			i_west_used = i_east_used = FALSE;
			for (ii = -F.x_half_width; !skip_if_NaN && ii <= F.x_half_width; ii++) {	/* Possible columns to consider on both sides of input point */
				col_in = i_origin[col_out] + ii;	/* Input column to consider */
				if (spherical) {	/* Must guard against wrapping around the globe */
					if (col_in < 0) col_in += nx_wrap;	/* "Left" of west means we might reappear in the east */
					else if (col_in >= Gin->header->nx) col_in -= nx_wrap;	/* Likewise if we are "right" of east */
					i_orig = col_in;
				}
				if (col_in < 0 || (col_in >= Gin->header->nx)) continue;	/* Still outside range of original inut grid */

				if (col_in == i_west) i_west_used = TRUE;
				if (col_in == i_east) i_east_used = TRUE;
				if (duplicate_check && ((col_in == i_east && i_west_used) || (col_in == i_west && i_east_used))) continue;	/* Do not use the node at 360 if the one at 0 was used, or vice versa */
				i_orig = col_in;
				
				for (jj = -F.y_half_width, row_ok = TRUE; row_ok && jj <= F.y_half_width; jj++) {	/* Possible rows to consider for filter input */
					row_in = j_origin + jj;	/* Current input row number */
					
					if (pole_check && (row_in == j_n_pole || row_in == j_s_pole)) {	/* N or S pole in a gridline-registered grid */
						row_ok = FALSE;		/* Only use one point from this row since we are at a single point (the pole) */
						ij_in = GMT_IJP (Gin->header, row_in, 0);	/* All points on pole rows are the same; pick the first one (col = 0) */
						if (GMT_is_fnan (Gin->data[ij_in])) {		/* Whaddaya know; the pole value is NaN */
							n_bad++;
							continue;
						}
						/* Get here when pole value is usable  */
						if (slow) {	/* Add it to the bag */
							work_array[n_in_median] = Gin->data[ij_in];
							n_in_median++;
						}
						else {	/* Update weighted sum */
							value += Gin->data[ij_in] * pole_weight;
							wt_sum += pole_weight;
						}
						continue;	/* Done with this special row */
					}
					else if (spherical) {	/* Periodic data may involve wrapping over the pole */
						col_in = i_orig;
						new_col = FALSE;	/* TRUE if we must jump 180 degrees as we pass over a pole */
						if (row_in < 0) {	/* Possible wrap over S pole and up the other side */
							if (row_in > sp_wrap_row) continue;	/* No, we are in a data gap over S pole [grid does not extend to -90]  */
							row_in = sp_wrap_row - row_in;		/* Row of the point on the far side */
							col_in += half_nx;			/* Column on the far side is 180 degrees off in longitude */
							new_col = TRUE;
						}
						else if (row_in >= Gin->header->ny) {	/* Possible wrap over N pole */
							if (row_in < np_wrap_row) continue;	/* Data gap over N pole [grid does not extend to +90] */
							row_in = Gin->header->ny - (row_in - np_wrap_row) - 1;	/* Row of the point on the far side */
							col_in += half_nx;			/* Column on the far side is 180 degrees off in longitude */
							new_col = TRUE;
							if (row_in < 0) continue;
						}
						if (new_col) {	/* Changed the longitude; must guard against wrapping around the globe */
							if (col_in < 0) col_in += nx_wrap;			/* "Left" of west means we might reappear in the east */
							else if (col_in >= Gin->header->nx) col_in -= nx_wrap;	/* Likewise if we are "right" of east */
						}
						if (new_col && (col_in < 0 || col_in >= Gin->header->nx)) continue;	/* Got pushed outside crossing the pole */
					}
					else if (row_in < 0 || (row_in >= Gin->header->ny)) continue;	/* Simple Cartesian test for y-range */

					ij_wt = (jj + F.y_half_width) * F.nx + ii + F.x_half_width;
					if (weight[ij_wt] <= 0.0) continue;

					ij_in = GMT_IJP (Gin->header, row_in, col_in);	/* Finally, the current input data point inside the filter */
					if (GMT_is_fnan (Gin->data[ij_in])) {
						n_bad++;
						continue;
					}

					/* Get here when point is usable  */
					if (slow) {	/* Add it to the bag */
						work_array[n_in_median] = Gin->data[ij_in];
						n_in_median++;
					}
					else {	/* Update weighted sum */
						value += Gin->data[ij_in] * weight[ij_wt];
						wt_sum += weight[ij_wt];
					}
				}
			}

			/* Now we have done the convolution and we can get the value  */

			if (skip_if_NaN) {
				Gout->data[ij_out] = GMT->session.f_NaN;
				n_nan++;
			}
			else if (Ctrl->N.mode == NAN_PRESERVE && n_bad) {	/* -Np in effect and there were NaNs inside circle */
				Gout->data[ij_out] = GMT->session.f_NaN;
				n_nan++;
			}
			else if (slow) {	/* Non-convolution filters */
				if (n_in_median) {
					switch (filter_type) {
						case 3:	/* Median */
							GMT_median (GMT, work_array, n_in_median, Gin->header->z_min, Gin->header->z_max, last_median, &this_median);
							last_median = this_median;
							break;
						case 4:	/* Mode */
							GMT_mode (GMT, work_array, n_in_median, n_in_median/2, TRUE, Ctrl->F.mode, &GMT_n_multiples, &this_median);
							break;
						case 5:	/* Lowest of all */
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
					}
					Gout->data[ij_out] = (float)this_median;
				}
				else {
					Gout->data[ij_out] = GMT->session.f_NaN;
					n_nan++;
				}
			}
			else {	/* Convolution filters */
				if (wt_sum == 0.0) {	/* Assign value = GMT->session.f_NaN */
					Gout->data[ij_out] = GMT->session.f_NaN;
					n_nan++;
				}
				else
					Gout->data[ij_out] = (float)(value / wt_sum);
			}
		}
	}

	GMT_free (GMT, weight);
	GMT_free (GMT, F.x);
	GMT_free (GMT, F.y);
	if (slow) GMT_free (GMT, work_array);
#ifdef _OPENMP
}  /* end of parallel region */
#endif

	if (Ctrl->F.highpass) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Subtracting lowpass-filtered data from grid to obtain high-pass filtered data\n");
		GMT_grd_loop (Gout, row_out, col_out, ij_out) Gout->data[ij_out] = Gin->data[ij_out] - Gout->data[ij_out];
	}
	
	/* At last, that's it!  Output: */

	if (n_nan) GMT_report (GMT, GMT_MSG_NORMAL, "Unable to estimate value at %ld nodes, set to NaN\n", n_nan);
	if (GMT_n_multiples > 0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: %ld multiple modes found\n", GMT_n_multiples);

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->G.file, (void *)Gout);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Gin);
	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Gout);

	GMT_free (GMT, i_origin);
	/*GMT_free (GMT, weight);
	GMT_free (GMT, F.x);
	GMT_free (GMT, F.y);
	if (slow) GMT_free (GMT, work_array);*/
	if (!fast_way) GMT_free (GMT, x_shift);

	Return (EXIT_SUCCESS);
}
