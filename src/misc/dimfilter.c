/*
 * $Id$
 *
 * dimfilter.c  reads a grdfile and creates filtered grd file
 *
 * user selects primary filter radius, number of sectors, and the secondary filter.
 * The Primary filter determines how we want to filter the raw data. However, instead
 * of filtering all data inside the filter radius at once, we split the filter circle
 * into several sectors and apply the filter on the data within each sector.  The
 * Secondary filter is then used to consolidate all the sector results into one output
 * value.  As an option for robust filters, we can detrend the input data prior to
 * applying the primary filter using a LS plane fit.
 *
 * Author: 	Paul Wessel with help from Caleb Fassett & Seung-Sep Kim
 * Date: 	25-MAR-2008
 * Version:	GMT 4
 *
 * For details, see Kim, S.-S., and Wessel, P. 2008, "Directional Median Filtering
 * for Regional-Residual Separation of Bathymetry, Geochem. Geophys. Geosyst.,
 * 9(Q03005), doi:10.1029/2007GC001850.
 */

#define THIS_MODULE_NAME	"dimfilter"
#define THIS_MODULE_LIB		"misc"
#define THIS_MODULE_PURPOSE	"Directional filtering of grids in the space domain"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:RVfh"

struct DIMFILTER_INFO {
	int nx;		/* The max number of filter weights in x-direction */
	int ny;		/* The max number of filter weights in y-direction */
	int x_half_width;	/* Number of filter nodes to either side needed at this latitude */
	int y_half_width;	/* Number of filter nodes above/below this point (ny_f/2) */
	int d_flag;
	int f_flag;
	double x_fix, y_fix;
	double dx, dy;		/* Grid spacing in original units */
	double width;
	double deg2km;
	double *weight;
};

struct DIMFILTER_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct C {	/* -C */
		bool active;
	} C;
	struct D {	/* -D<distflag> */
		bool active;
		unsigned int mode;
	} D;
	struct E {	/* -E */
		bool active;
	} E;
	struct F {	/* <type><filter_width>*/
		bool active;
		unsigned int filter;	/* Id for the filter */
		double width;
	} F;
	struct G {	/* -G<file> */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct N {	/* -N */
		bool active;
		unsigned int n_sectors;
		unsigned int filter;	/* Id for the filter */
	} N;
	struct Q {	/* -Q */
		bool active;
		unsigned int err_cols;
	} Q;
	struct S {	/* -S<file> */
		bool active;
		char *file;
	} S;
	struct T {	/* -T */
		bool active;
	} T;
};

void *New_dimfilter_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct DIMFILTER_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct DIMFILTER_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->F.filter = C->N.filter = C->D.mode = -1;
	C->N.n_sectors = 1;
	return (C);
}

void Free_dimfilter_Ctrl (struct GMT_CTRL *GMT, struct DIMFILTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);
	if (C->G.file) free (C->G.file);
	if (C->S.file) free (C->S.file);
	GMT_free (GMT, C);
}


int GMT_dimfilter_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: dimfilter <ingrid> -D<distance_flag> -F<type><filter_width> -G<outgrid> -N<type><n_sectors>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-Q<cols>]\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-T] [%s] [%s]\n\t[%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_ho_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is grid to be filtered.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tDistance flag determines how grid (x,y) maps into distance units of filter width as follows:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -D0 grid x,y same units as <filter_width>, cartesian Distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -D1 grid x,y in degrees, <filter_width> in km, cartesian Distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -D2 grid x,y in degrees, <filter_width> in km, x_scaled by cos(middle y), cartesian Distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   These first three options are faster; they allow weight matrix to be computed only once.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Next two options are slower; weights must be recomputed for each scan line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -D3 grid x,y in degrees, <filter_width> in km, x_scale varies as cos(y), cartesian Distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -D4 grid x,y in degrees, <filter_width> in km, spherical Distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F sets the primary filter type and full (6 sigma) filter-width  Choose between\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (b)oxcar, (c)osine arch, (g)aussian, (m)edian filters\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or p(maximum liklihood Probability estimator -- a mode estimator)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G sets output name for filtered grdfile\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N sets the secondary filter type and the number of sectors.  Choose between\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (l)ower, (u)pper, (a)verage, (m)edian, and (p) the mode estimator)\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
#ifdef OBSOLETE
	GMT_Message (API, GMT_TIME_NONE, "\t-E Remove local planar trend from data, apply filter, then add back trend at filtered value.\n");
#endif
	GMT_Message (API, GMT_TIME_NONE, "\t-I for new Increment of output grid; enter xinc, optionally xinc/yinc.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is yinc = xinc.  Append an m [or c] to xinc or yinc to indicate minutes [or seconds];\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The new xinc and yinc should be divisible by the old ones (new lattice is subset of old).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q is for the error analysis mode and requires the total number of depth columns in the input file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   See documentation for how to prepare for using this option.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-R for new Range of output grid; enter <WESN> (xmin, xmax, ymin, ymax) separated by slashes.\n");
#ifdef OBSOLETE
	GMT_Message (API, GMT_TIME_NONE, "\t-S sets output name for standard error grdfile and implies that we will compute a 2nd grid with\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a statistical measure of deviations from the average value.  For the convolution filters this\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   yields the standard deviation while for the median/mode filters we use MAD\n");
#endif
	GMT_Message (API, GMT_TIME_NONE, "\t-T Toggles between grid and pixel registration for output grid [Default is same as input registration]\n");
	GMT_Option (API, "V,f,h,.");

	return (EXIT_FAILURE);
}

int GMT_dimfilter_parse (struct GMT_CTRL *GMT, struct DIMFILTER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to dimfilter and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int k;
	struct GMT_OPTION *opt = NULL;
#ifdef OBSOLETE
	int slow;
#endif

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = true;
				break;
			case 'D':
				Ctrl->D.active = true;
				k = atoi (opt->arg);
				n_errors += GMT_check_condition (GMT, k < 0 || k > 4, "Syntax error -D option: Choose from the range 0-4\n");
				Ctrl->D.mode = k;
				break;
#ifdef OBSOLETE
			case 'E':
				Ctrl->E.active = true;
				break;
#endif
			case 'F':
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'b':
						Ctrl->F.filter = 0;
						break;
					case 'c':
						Ctrl->F.filter = 1;
						break;
					case 'g':
						Ctrl->F.filter = 2;
						break;
					case 'm':
						Ctrl->F.filter = 3;
						break;
					case 'p':
						Ctrl->F.filter = 4;
						break;
					default:
						n_errors++;
						break;
				}
				Ctrl->F.width = atof (&opt->arg[1]);
				break;
			case 'G':
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'N':	/* Scan: Option to set the number of sections and how to reduce the sector results to a single value */
				Ctrl->N.active = true;
				switch (opt->arg[0]) {
					case 'l':	/* Lower bound (min) */
						Ctrl->N.filter = 0;
						break;
					case 'u':	/* Upper bound (max) */
						Ctrl->N.filter = 1;
						break;
					case 'a':	/* Average (mean) */
						Ctrl->N.filter = 2;
						break;
					case 'm':	/* Median */
						Ctrl->N.filter = 3;
						break;
					case 'p':	/* Mode */
						Ctrl->N.filter = 4;
						break;
					default:
						n_errors++;
						break;
				}
				k = atoi (&opt->arg[1]);	/* Number of sections to split filter into */
				n_errors += GMT_check_condition (GMT, k <= 0, "Syntax error -N option: Correct syntax: -NX<nsectors>, with X one of luamp, nsectors is number of sectors\n");
				Ctrl->N.n_sectors = k;	/* Number of sections to split filter into */
				break;
			case 'Q':	/* entering the MAD error analysis mode */
				Ctrl->Q.active = true;
				Ctrl->Q.err_cols = atoi (opt->arg);
				break;
#ifdef OBSOLETE
			case 'S':
				Ctrl->S.active = true;
				Ctrl->S.file = strdup (opt->arg);
				break;
#endif
			case 'T':	/* Toggle registration */
				Ctrl->T.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	if (!Ctrl->Q.active) {
		GMT_check_lattice (GMT, Ctrl->I.inc, NULL, &Ctrl->I.active);
		n_errors += GMT_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0), "Syntax error -I option: Must specify positive increment(s)\n");
		n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output file\n");
		n_errors += GMT_check_condition (GMT, Ctrl->F.width <= 0.0, "Syntax error -F option: Correct syntax: -FX<width>, with X one of bcgmp, width is filter fullwidth\n");
		n_errors += GMT_check_condition (GMT, Ctrl->N.n_sectors == 0, "Syntax error -N option: Correct syntax: -NX<nsectors>, with X one of luamp, nsectors is number of sectors\n");
#ifdef OBSOLETE
		slow = (Ctrl->F.filter == 3 || Ctrl->F.filter == 4);		/* Will require sorting etc */
		n_errors += GMT_check_condition (GMT, Ctrl->E.active && !slow, "Syntax error -E option: Only valid for robust filters -Fm|p.\n");
#endif
	}
	else {
		n_errors += GMT_check_condition (GMT, !Ctrl->Q.active, "Syntax error: Must use -Q to specify total # of columns in the input file.\n");
		n_errors += GMT_check_condition (GMT, Ctrl->Q.err_cols > 50, "Syntax error -Q option: Total # of columns cannot exceed 50.\n");
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

void set_weight_matrix_dim (struct DIMFILTER_INFO *F, struct GMT_GRID_HEADER *h, double y_0, int fast)
/* Last two gives offset between output node and 'origin' input node for this window (0,0 for integral grids) */
/* true when input/output grids are offset by integer values in dx/dy */

{
	int i, j, ij, i_half, j_half;
	double x_scl, y_scl, f_half, r_f_half, sigma, sig_2;
	double y1, y2, theta, x, y, r, s_y1, c_y1, s_y2, c_y2;

	/* Set Scales  */

	y_scl = (F->d_flag) ? F->deg2km : 1.0;
	if (F->d_flag < 2)
		x_scl = y_scl;
	else if (F->d_flag == 2)
		x_scl = F->deg2km * cosd (0.5 * (h->wesn[YHI] + h->wesn[YLO]));
	else
		x_scl = F->deg2km * cosd (y_0);

	/* Get radius, weight, etc.  */

	i_half = F->nx / 2;
	j_half = F->ny / 2;
	f_half = 0.5 * F->width;
	r_f_half = 1.0 / f_half;
	sigma = F->width / 6.0;
	sig_2 = -0.5 / (sigma * sigma);
	for (i = -i_half; i <= i_half; i++) {
		for (j = -j_half; j <= j_half; j++) {
			ij = (j + j_half) * F->nx + i + i_half;
			if (fast && i == 0)
				r = (j == 0) ? 0.0 : j * y_scl * F->dy;
			else if (fast && j == 0)
				r = i * x_scl * F->dx;
			else if (F->d_flag < 4) {
				x = x_scl * (i * F->dx - F->x_fix);
				y = y_scl * (j * F->dy - F->y_fix);
				r = hypot (x, y);
			}
			else {
				theta = i * F->dx - F->x_fix;
				y1 = 90.0 - y_0;
				y2 = 90.0 - (y_0 + (j * F->dy - F->y_fix));
				sincosd (y1, &s_y1, &c_y1);
				sincosd (y2, &s_y2, &c_y2);
				r = d_acos (c_y1 * c_y2 + s_y1 * s_y2 * cosd (theta)) * F->deg2km * R2D;
			}
			/* Now we know r in F->width units  */

			if (r > f_half) {
				F->weight[ij] = -1.0;
				continue;
			}
			else if (F->f_flag == 3) {
				F->weight[ij] = 1.0;
				continue;
			}
			else {
				if (F->f_flag == 0)
					F->weight[ij] = 1.0;
				else if (F->f_flag == 1)
					F->weight[ij] = 1.0 + cos (M_PI * r * r_f_half);
				else
					F->weight[ij] = exp (r * r * sig_2);
			}
		}
	}
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_dimfilter_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_dimfilter (void *V_API, int mode, void *args)
{
	unsigned short int **sector = NULL;

	unsigned int *n_in_median, wsize = 0, one_or_zero = 1, effort_level, n_sectors_2 = 0, col_in, row_in;
	unsigned int GMT_mode_selection = 0, GMT_n_multiples = 0, col_out, row_out, i, j, k, s;
	bool full_360, shift = false, slow, slow2, fast_way;
	int j_origin, *i_origin = NULL, ii, jj, scol, srow, error = 0;

	uint64_t n_nan = 0, ij_in, ij_out, ij_wt;

	double wesn[4], inc[2], x_scale, y_scale, x_width, y_width, angle, z = 0.0;
	double x_out, y_out, *wt_sum = NULL, *value = NULL, last_median, this_median;
	double last_median2 = 0.0, this_median2, d, **work_array = NULL, *x_shift = NULL;
	double z_min, z_max, z2_min = 0.0, z2_max = 0.0, wx = 0.0, *c_x = NULL, *c_y = NULL;
#ifdef DEBUG
	double x_debug[5], y_debug[5], z_debug[5];
#endif

#ifdef OBSOLETE
	bool first_time = true;
	int n = 0;
	int n_bad_planes = 0, S = 0;
	double Sx = 0.0, Sy = 0.0, Sz = 0.0, Sxx = 0.0, Syy = 0.0, Sxy = 0.0, Sxz = 0.0, Syz = 0.0;
	double denominator, scale, Sw, intercept = 0.0, slope_x = 0.0, slope_y = 0.0, inv_D;
	double *work_array2 = NULL;
	short int **xx = NULL, **yy = NULL;
	struct GMT_GRID *Sout = NULL;
#endif

	char *filter_name[5] = {"Boxcar", "Cosine Arch", "Gaussian", "Median", "Mode"};

	struct GMT_GRID *Gin = NULL, *Gout = NULL;
	struct DIMFILTER_INFO F;
	struct DIMFILTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
        struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_dimfilter_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
        options = GMT_Create_Options (API, mode, args);	if (API->error) bailout (API->error);   /* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_dimfilter_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_dimfilter_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_dimfilter_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_dimfilter_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the dimfilter main code ----------------------------*/

	GMT_memset (&F, 1, struct DIMFILTER_INFO);
	F.deg2km = GMT->current.proj.DIST_KM_PR_DEG;

	if (!Ctrl->Q.active) {

		if ((Gin = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
		GMT_grd_init (GMT, Gin->header, options, true);	/* Update command history only */

		slow  = (Ctrl->F.filter == 3 || Ctrl->F.filter == 4);	/* Will require sorting etc */
		slow2 = (Ctrl->N.filter == 3 || Ctrl->N.filter == 4);	/* SCAN: Will also require sorting etc */

		if (Ctrl->T.active)	/* Make output grid of the opposite registration */
			one_or_zero = (Gin->header->registration == GMT_GRID_PIXEL_REG) ? 1 : 0;
		else
			one_or_zero = (Gin->header->registration == GMT_GRID_PIXEL_REG) ? 0 : 1;

		/* Use the -R region for output if set; otherwise match grid domain */
		GMT_memcpy (wesn, (GMT->common.R.active ? GMT->common.R.wesn : Gin->header->wesn), 4, double);
		full_360 = (Ctrl->D.mode && GMT_grd_is_global (GMT, Gin->header));	/* Periodic geographic grid */

		if (Ctrl->I.active)
			GMT_memcpy (inc, Ctrl->I.inc, 2, double);
		else
			GMT_memcpy (inc, Gin->header->inc, 2, double);

		if (!full_360) {
			if (wesn[XLO] < Gin->header->wesn[XLO]) error = true;
			if (wesn[XHI] > Gin->header->wesn[XHI]) error = true;
		}
		if (wesn[YLO] < Gin->header->wesn[YLO]) error = true;
		if (wesn[YHI] > Gin->header->wesn[YHI]) error = true;

		if (error) {
			GMT_Report (API, GMT_MSG_NORMAL, "New WESN incompatible with old.\n");
			Return (EXIT_FAILURE);
		}

		last_median = 0.5 * (Gin->header->z_min + Gin->header->z_max);
		z_min = Gin->header->z_min;	z_max = Gin->header->z_max;

		if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, wesn, inc, \
			!one_or_zero, GMT_NOTSET, NULL)) == NULL) Return (API->error);

		/* We can save time by computing a weight matrix once [or once pr scanline] only
		   if new grid spacing is multiple of old spacing */

		fast_way = (fabs (fmod (Gout->header->inc[GMT_X] / Gin->header->inc[GMT_X], 1.0)) < GMT_SMALL && fabs (fmod (Gout->header->inc[GMT_Y] / Gin->header->inc[GMT_Y], 1.0)) < GMT_SMALL);

		if (!fast_way) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Your output grid spacing is such that filter-weights must\n");
			GMT_Report (API, GMT_MSG_VERBOSE, "be recomputed for every output node, so expect this run to be slow.  Calculations\n");
			GMT_Report (API, GMT_MSG_VERBOSE, "can be speeded up significantly if output grid spacing is chosen to be a multiple\n");
			GMT_Report (API, GMT_MSG_VERBOSE, "of the input grid spacing.  If the odd output grid is necessary, consider using\n");
			GMT_Report (API, GMT_MSG_VERBOSE, "a \'fast\' grid for filtering and then resample onto your desired grid with grdsample.\n");
		}

#ifdef OBSOLETE
		if (Ctrl->S.active) {
			if ((Sout = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, Gout)) == NULL) Return (API->error);
		}
#endif
		i_origin = GMT_memory (GMT, NULL, Gout->header->nx, int);
		if (!fast_way) x_shift = GMT_memory (GMT, NULL, Gout->header->nx, double);

		if (fast_way && Gin->header->registration == one_or_zero) {	/* multiple grid but one is pix, other is grid */
			F.x_fix = 0.5 * Gin->header->inc[GMT_X];
			F.y_fix = 0.5 * Gin->header->inc[GMT_Y];
			shift = (F.x_fix != 0.0 || F.y_fix != 0.0);
		}

		/* Set up weight matrix and i,j range to test  */

		x_scale = y_scale = 1.0;
		if (Ctrl->D.mode > 0) {
			x_scale *= F.deg2km;
			y_scale *= F.deg2km;
		}
		if (Ctrl->D.mode == 2)
			x_scale *= cosd (0.5 * (wesn[YHI] + wesn[YLO]));
		else if (Ctrl->D.mode > 2) {
			if (fabs (wesn[YLO]) > wesn[YHI])
				x_scale *= cosd (wesn[YLO]);
			else
				x_scale *= cosd (wesn[YHI]);
		}
		x_width = Ctrl->F.width / (Gin->header->inc[GMT_X] * x_scale);
		y_width = Ctrl->F.width / (Gin->header->inc[GMT_Y] * y_scale);
		F.d_flag = Ctrl->D.mode;
		F.f_flag = Ctrl->F.filter;
		F.y_half_width = irint (ceil(y_width) / 2.0);
		F.x_half_width = irint (ceil(x_width) / 2.0);
		F.dx = Gin->header->inc[GMT_X];
		F.dy = Gin->header->inc[GMT_Y];

		F.nx = 2 * F.x_half_width + 1;
		F.ny = 2 * F.y_half_width + 1;
		F.width = Ctrl->F.width;
		F.weight = GMT_memory (GMT, NULL, F.nx*F.ny, double);

		if (slow) {	/* SCAN: Now require several work_arrays, one for each sector */
			work_array = GMT_memory (GMT, NULL, Ctrl->N.n_sectors, double *);
#ifdef OBSOLETE
			if (Ctrl->S.active) work_array2 = GMT_memory (GMT, NULL, 2*F.nx*F.ny, double);
			if (Ctrl->E.active) {
				xx = GMT_memory (GMT, NULL, Ctrl->N.n_sectors, short int *);
				yy = GMT_memory (GMT, NULL, Ctrl->N.n_sectors, short int *);
			}
#endif
			wsize = 2*F.nx*F.ny/Ctrl->N.n_sectors;	/* Should be enough, watch for messages to the contrary */
			for (i = 0; i < Ctrl->N.n_sectors; i++) {
				work_array[i] = GMT_memory (GMT, NULL, wsize, double);
#ifdef OBSOLETE
				if (Ctrl->E.active) {
					xx[i] = GMT_memory (GMT, NULL, wsize, short int);
					yy[i] = GMT_memory (GMT, NULL, wsize, short int);
				}
#endif
			}
		}

		GMT_Report (API, GMT_MSG_VERBOSE, "Input nx,ny = (%d %d), output nx,ny = (%d %d), filter nx,ny = (%d %d)\n",
			Gin->header->nx, Gin->header->ny, Gout->header->nx, Gout->header->ny, F.nx, F.ny);
		GMT_Report (API, GMT_MSG_VERBOSE, "Filter type is %s.\n", filter_name[Ctrl->F.filter]);

		/* Compute nearest xoutput i-indices and shifts once */

		for (col_out = 0; col_out < Gout->header->nx; col_out++) {
			x_out = GMT_grd_col_to_x (GMT, col_out, Gout->header);	/* Current longitude */
			i_origin[col_out] = (int)GMT_grd_x_to_col (GMT, x_out, Gin->header);
			if (!fast_way) x_shift[col_out] = x_out - GMT_grd_col_to_x (GMT, i_origin[col_out], Gin->header);
		}

		/* Now we can do the filtering  */

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

		if (effort_level == 1) set_weight_matrix_dim (&F, Gout->header, 0.0, shift);	/* Only need this once */

		if (Ctrl->C.active) {	/* Use fixed-width diagonal corridors instead of bow-ties */
			n_sectors_2 = Ctrl->N.n_sectors / 2;
			c_x = GMT_memory (GMT, NULL, n_sectors_2, double);
			c_y = GMT_memory (GMT, NULL, n_sectors_2, double);
			for (i = 0; i < n_sectors_2; i++) {
				angle = (i + 0.5) * (180.0/n_sectors_2);	/* Angle of central diameter of each corridor */
				sincosd (angle, &c_y[i], &c_x[i]);	/* Unit vector of diameter */
			}
		}
		else {
		/* SCAN: Precalculate which sector each point belongs to */
			sector = GMT_memory (GMT, NULL, F.ny, unsigned short int *);
			for (jj = 0; jj < F.ny; jj++) sector[jj] = GMT_memory (GMT, NULL, F.nx, unsigned short int);
			for (jj = -F.y_half_width; jj <= F.y_half_width; jj++) {	/* This double loop visits all nodes in the square centered on an output node */
				j = F.y_half_width + jj;
				for (ii = -F.x_half_width; ii <= F.x_half_width; ii++) {	/* (ii, jj) is local coordinates relative center (0,0) */
					i = F.x_half_width + ii;
					/* We are doing "bow-ties" and not wedges here */
					angle = atan2 ((double)jj, (double)ii);				/* Returns angle in -PI,+PI range */
					if (angle < 0.0) angle += M_PI;					/* Flip to complimentary sector in 0-PI range */
					sector[j][i] = (short) rint ((Ctrl->N.n_sectors * angle) / M_PI);	/* Convert to sector id 0-<n_sectors-1> */
					if (sector[j][i] == Ctrl->N.n_sectors) sector[j][i] = 0;		/* Ensure that exact PI is set to 0 */
				}
			}
		}
		n_in_median = GMT_memory (GMT, NULL, Ctrl->N.n_sectors, unsigned int);
		value = GMT_memory (GMT, NULL, Ctrl->N.n_sectors, double);
		wt_sum = GMT_memory (GMT, NULL, Ctrl->N.n_sectors, double);

		for (row_out = 0; row_out < Gout->header->ny; row_out++) {

			GMT_Report (API, GMT_MSG_VERBOSE, "Processing output line %d\r", row_out);
			y_out = GMT_grd_row_to_y (GMT, row_out, Gout->header);
			j_origin = (int)GMT_grd_y_to_row (GMT, y_out, Gin->header);
			if (effort_level == 2) set_weight_matrix_dim (&F, Gout->header, y_out, shift);

			for (col_out = 0; col_out < Gout->header->nx; col_out++) {

				if (effort_level == 3) set_weight_matrix_dim (&F, Gout->header, y_out, shift);
				GMT_memset (n_in_median, Ctrl->N.n_sectors, unsigned int);
				GMT_memset (value, Ctrl->N.n_sectors, double);
				GMT_memset (wt_sum, Ctrl->N.n_sectors, double);
#ifdef OBSOLETE
				if (Ctrl->E.active) S = 0, Sx = Sy = Sz = Sxx = Syy = Sxy = Sxz = Syz = Sxx = Sw = 0.0;
				n = 0;
#endif

				ij_out = GMT_IJP (Gout->header, row_out, col_out);

				for (ii = -F.x_half_width; ii <= F.x_half_width; ii++) {
					scol = i_origin[col_out] + ii;
					if (scol < 0 || (col_in = scol) >= Gin->header->nx) continue;

					for (jj = -F.y_half_width; jj <= F.y_half_width; jj++) {
						srow = j_origin + jj;
						if (srow < 0 || (row_in = srow) >= Gin->header->ny) continue;

						ij_wt = (jj + F.y_half_width) * F.nx + ii + F.x_half_width;
						if (F. weight[ij_wt] < 0.0) continue;

						ij_in = GMT_IJP (Gin->header, row_in, col_in);
						if (GMT_is_fnan (Gin->data[ij_in])) continue;

						/* Get here when point is usable  */

						if (Ctrl->C.active) {	/* Point can belong to several corridors */
							for (s = 0; s < n_sectors_2; s++) {
								d = sqrt (c_y[s] * ii + c_x[s] * jj);	/* Perpendicular distance to central diameter, in nodes */
								if (d > F.y_half_width) continue;	/* Outside this corridor */
								if (slow) {
									work_array[s][n_in_median[s]] = Gin->data[ij_in];
#ifdef OBSOLETE
									if (Ctrl->S.active) work_array2[n++] = Gin->data[ij_in];
#endif
#ifdef DEBUG
									if (n_in_median[s] < 5) x_debug[n_in_median[s]] = (double)ii;
									if (n_in_median[s] < 5) y_debug[n_in_median[s]] = (double)jj;
									if (n_in_median[s] < 5) z_debug[n_in_median[s]] = Gin->data[ij_in];
#endif
#ifdef OBSOLETE
									if (Ctrl->E.active) {	/* Sum up required terms to solve for slope and intercepts of planar trend */
									xx[s][n_in_median[s]] = ii;
										yy[s][n_in_median[s]] = jj;
										Sx += ii;
										Sy += jj;
										Sz += Gin->data[ij_in];
										Sxx += ii * ii;
										Syy += jj * jj;
										Sxy += ii * jj;
										Sxz += ii * Gin->data[ij_in];
										Syz += jj * Gin->data[ij_in];
										S++;
									}
#endif
									n_in_median[s]++;
								}
								else {
									wx = Gin->data[ij_in] * F. weight[ij_wt];
									value[s] += wx;
									wt_sum[s] += F. weight[ij_wt];
#ifdef OBSOLETE
									if (Ctrl->S.active) {
										Sxx += wx * Gin->data[ij_in];
										Sw += F. weight[ij_wt];
										n++;
									}
#endif
								}
							}
						}
						else if (ii == 0 && jj == 0) {	/* Center point belongs to all sectors */
							if (slow) {	/* Must store copy in all work arrays */
								for (s = 0; s < Ctrl->N.n_sectors; s++) {
									work_array[s][n_in_median[s]] = Gin->data[ij_in];
#ifdef DEBUG
									if (n_in_median[s] < 5) x_debug[n_in_median[s]] = (double)ii;
									if (n_in_median[s] < 5) y_debug[n_in_median[s]] = (double)jj;
									if (n_in_median[s] < 5) z_debug[n_in_median[s]] = Gin->data[ij_in];
#endif
#ifdef OBSOLETE
									if (Ctrl->E.active) xx[s][n_in_median[s]] = yy[s][n_in_median[s]] = 0;	/*(0,0) at the node */
#endif
									n_in_median[s]++;
								}
#ifdef OBSOLETE
								if (Ctrl->S.active) work_array2[n++] = Gin->data[ij_in];
#endif
							}
							else {	/* Simply add to the weighted sums */
								for (s = 0; s < Ctrl->N.n_sectors; s++) {
									wx = Gin->data[ij_in] * F. weight[ij_wt];
									value[s] += wx;
									wt_sum[s] += F. weight[ij_wt];
								}
#ifdef OBSOLETE
								if (Ctrl->S.active) {
									Sxx += wx * Gin->data[ij_in];
									Sw += F. weight[ij_wt];
									n++;
								}
#endif
							}
#ifdef OBSOLOTE
							if (Ctrl->E.active) {	/* Since r is 0, only need to update Sz and S */
								Sz += Gin->data[ij_in];
								S++;
							}
#endif
						}
						else {
							s = sector[jj+F.y_half_width][ii+F.x_half_width];	/* Get the sector for this node */

							if (slow) {
								work_array[s][n_in_median[s]] = Gin->data[ij_in];
#ifdef OBSOLETE
								if (Ctrl->S.active) work_array2[n++] = Gin->data[ij_in];
#endif
#ifdef DEBUG
								if (n_in_median[s] < 5) x_debug[n_in_median[s]] = (double)ii;
								if (n_in_median[s] < 5) y_debug[n_in_median[s]] = (double)jj;
								if (n_in_median[s] < 5) z_debug[n_in_median[s]] = Gin->data[ij_in];
								(void)x_debug;
								(void)y_debug;
								(void)z_debug;
#endif
#ifdef OBSOLETE
								if (Ctrl->E.active) {	/* Sum up required terms to solve for slope and intercepts of planar trend */
									xx[s][n_in_median[s]] = ii;
									yy[s][n_in_median[s]] = jj;
									Sx += ii;
									Sy += jj;
									Sz += Gin->data[ij_in];
									Sxx += ii * ii;
									Syy += jj * jj;
									Sxy += ii * jj;
									Sxz += ii * Gin->data[ij_in];
									Syz += jj * Gin->data[ij_in];
									S++;
								}
#endif
								n_in_median[s]++;
							}
							else {
								wx = Gin->data[ij_in] * F. weight[ij_wt];
								value[s] += wx;
								wt_sum[s] += F. weight[ij_wt];
#ifdef OBSOLETE
								if (Ctrl->S.active) {
									Sxx += wx * Gin->data[ij_in];
									Sw += F. weight[ij_wt];
									n++;
								}
#endif
							}
						}
					}
				}

				/* Now we have done the sectoring and we can apply the filter on each sector  */
				/* k will be the number of sectors that had enough data to do the operation */
#ifdef OBSOLETE
				if (Ctrl->E.active) {	/* Must find trend coeeficients, remove from array, do the filter, then add in intercept (since r = 0) */
					denominator = S * Sxx * Syy + 2.0 * Sx * Sy * Sxy - S * Sxy * Sxy - Sx * Sx * Syy - Sy * Sy * Sxx;
					if (denominator == 0.0) {
						intercept = slope_x = slope_y = 0.0;
						n_bad_planes++;
					}
					else {
						inv_D = 1.0 / denominator;
						intercept = (S * Sxx * Syz + Sx * Sy * Sxz + Sz * Sx * Sxy - S * Sxy * Sxz - Sx * Sx * Syz - Sz * Sy * Sxx) * inv_D;
						slope_x = (S * Sxz * Syy + Sz * Sy * Sxy + Sy * Sx * Syz - S * Sxy * Syz - Sz * Sx * Syy - Sy * Sy * Sxz) * inv_D;
						slope_y = (S * Sxx * Syz + Sx * Sy * Sxz + Sz * Sx * Sxy - S * Sxy * Sxz - Sx * Sx * Syz - Sz * Sy * Sxx) * inv_D;
					}
				}
#endif

				if (slow) {	/* Take median or mode of each work array for each sector */
					if (slow2) {
						z2_min = DBL_MAX;
						z2_max = -DBL_MAX;
					}
					for (s = k = 0; s < Ctrl->N.n_sectors; s++) {
						if (n_in_median[s]) {
							if (n_in_median[s] >= wsize) GMT_Report (API, GMT_MSG_VERBOSE, "Exceed array size (%d > %d)!\n", n_in_median[s], wsize);
#ifdef OBSOLETE
							if (Ctrl->E.active) {
								z_min = DBL_MAX;
								z_max = -DBL_MAX;
								for (ii = 0; ii < n_in_median[s]; ii++) {
									work_array[s][ii] -= (intercept + slope_x * xx[s][ii] + slope_y * yy[s][ii]);
									if (work_array[s][ii] < z_min) z_min = work_array[s][ii];
									if (work_array[s][ii] > z_max) z_max = work_array[s][ii];
								}
								if (first_time) last_median = 0.5 * (z_min + z_max), first_time = false;
							}
#endif
							if (Ctrl->F.filter == 3) {
								GMT_median (GMT, work_array[s], n_in_median[s], z_min, z_max, last_median, &this_median);
								last_median = this_median;
							}
							else
								GMT_mode (GMT, work_array[s], n_in_median[s], n_in_median[s]/2, true, GMT_mode_selection, &GMT_n_multiples, &this_median);
							value[k] = this_median;
#ifdef OBSOLETE
							if (Ctrl->E.active) value[k] += intercept;	/* I.e., intercept + x * slope_x + y * slope_y, but x == y == 0 at node */
#endif
							if (slow2) {
								if (value[k] < z2_min) z2_min = value[k];
								if (value[k] > z2_max) z2_max = value[k];
							}
							k++;
						}
					}
				}
				else {	/* Simply divide weighted sums by the weights */
					for (s = k = 0; s < Ctrl->N.n_sectors; s++) {
						if (wt_sum[s] != 0.0) {
							value[k] = (float)(value[s] / wt_sum[s]);
							k++;
						}
					}
				}

				if (k == 0) {	/* No filtered values, set to NaN and move on to next output node */
					Gout->data[ij_out] = GMT->session.f_NaN;
#ifdef OBSOLETE
					if (Ctrl->S.active) Sout->data[ij_out] = GMT->session.f_NaN;
#endif
					n_nan++;
					continue;
				}

				if (slow2) {	/* Get median (or mode) of all the medians (or modes) */
					if (Ctrl->F.filter == 3) {
						GMT_median (GMT, value, k, z2_min, z2_max, last_median2, &this_median2);
						last_median2 = this_median2;
					}
					else
						GMT_mode (GMT, value, k, k/2, true, GMT_mode_selection, &GMT_n_multiples, &this_median2);
					z = this_median2;
				}
				else {	/* Get min, max, or mean */
					switch (Ctrl->N.filter) {	/* Initialize z, the final output value */
						case 0:	/* Lower bound */
							z = DBL_MAX;
							break;
						case 1:	/* Upper bound */
							z = -DBL_MAX;
							break;
						case 2:	/* Average (mean) */
							z = 0.0;
							break;
						default:
							break;
					}
					for (s = 0; s < k; s++) {	/* Apply the min, max, or mean update */
						switch (Ctrl->N.filter) {
							case 0:	/* Lower bound */
								if (value[s] < z) z = value[s];
								break;
							case 1:	/* Upper bound */
								if (value[s] > z) z = value[s];
								break;
							case 2:	/* Average (mean) */
								z += value[s];
								break;
							default:
								break;
						}
					}
					if (Ctrl->N.filter == 2) z /= (double)k;	/* Mean requires a final normalization */
				}
				Gout->data[ij_out] = (float)z;
#ifdef OBSOLETE
				if (Ctrl->S.active) {	/* Now assess a measure of deviation about this value */
					if (slow) {	/* Get MAD! */
						GMT_sort_array (GMT, work_array2, n, GMT_DOUBLE);
						GMT_getmad (GMT, work_array2, n, z, &scale);
					}
					else {		/* Get weighted stdev. */
						scale = sqrt ((Sxx - Sw * z * z) / (Sw * (n - 1) / n));
					}
					Sout->data[ij_out] = (float)scale;
				}
#endif
			}
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "\n");

		/* At last, that's it!  Output: */

		if (n_nan) GMT_Report (API, GMT_MSG_VERBOSE, "Unable to estimate value at %" PRIu64 " nodes, set to NaN\n", n_nan);
#ifdef OBSOLETE
		if (Ctrl->E.active && n_bad_planes) GMT_Report (API, GMT_MSG_VERBOSE, "Unable to detrend data at %" PRIu64 " nodes\n", n_bad_planes);
#endif
		if (GMT_n_multiples > 0) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: %d multiple modes found\n", GMT_n_multiples);

		GMT_Report (API, GMT_MSG_VERBOSE, "Write filtered grid\n");
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Gout)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Gout) != GMT_OK) {
			Return (API->error);
		}
#ifdef OBSOLETE
		if (Ctrl->S.active) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Write scale grid\n");
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Sout)) Return (API->error);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->S.file, Sout) != GMT_OK) {
				Return (API->error);
			}
		}
#endif
		GMT_Report (API, GMT_MSG_VERBOSE, "Done\n");

		GMT_free (GMT, F. weight);
		GMT_free (GMT, i_origin);
		for (ii = 0; ii < F.ny; ii++) GMT_free (GMT, sector[ii]);
		GMT_free (GMT, sector);
		GMT_free (GMT, value);
		GMT_free (GMT, wt_sum);
		GMT_free (GMT, n_in_median);
		if (slow) {
			for (j = 0; j < Ctrl->N.n_sectors; j++) {
				GMT_free (GMT, work_array[j]);
#ifdef OBSOLETE
				if (Ctrl->E.active) {
					GMT_free (GMT, xx[j]);
					GMT_free (GMT, yy[j]);
				}
#endif
			}
			GMT_free (GMT, work_array);
#ifdef OBSOLETE
			if (Ctrl->S.active) GMT_free (GMT, work_array2);
			if (Ctrl->E.active) {
				GMT_free (GMT, xx);
				GMT_free (GMT, yy);
			}
#endif
		}
		if (!fast_way) GMT_free (GMT, x_shift);

	}
	else {	/* Here -Q is active */
		int err_l = 1;
		double err_workarray[50], err_min, err_max, err_null_median = 0.0, err_median, err_mad, err_depth, err_sum, out[3];

		FILE *ip = NULL;

		/* check the crucial condition to run the program*/
		if ((ip = fopen (Ctrl->In.file, "r")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: Unable to open file %s\n", Ctrl->In.file);
			Return (EXIT_FAILURE);
		}

		if ((error = GMT_set_cols (GMT, GMT_OUT, 3))!= GMT_OK) Return (error);
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		GMT_set_cartesian (GMT, GMT_OUT);	/* No coordinates here */

		/* read depths from each column until EOF */
		while (fscanf (ip, "%lf", &err_depth) != EOF) {
			err_sum = 0.0;	/* start with empty sum */
			/* store data into array and find min/max */
			err_workarray[0] = err_min = err_max = err_depth;
			err_sum += err_depth;
			for (i = 1; i < Ctrl->Q.err_cols; i++) {
				if (fscanf (ip, "%lf", &err_depth) != 1) {
					GMT_Report (API, GMT_MSG_NORMAL, "Error: Unable to read depths for column %d\n", i);
					Return (EXIT_FAILURE);
				}
				err_workarray[i] = err_depth;
				err_sum += err_depth;
				if (err_depth < err_min) err_min = err_depth;
				if (err_depth > err_max) err_max = err_depth;
			}

			/* calculate MEDIAN and MAD for each row */
			GMT_median (GMT, err_workarray, Ctrl->Q.err_cols, err_min, err_max, err_null_median, &err_median);
			err_workarray[0] = fabs (err_workarray[0] - err_median);
			err_min = err_max = err_workarray[0];
			for (i = 1; i < Ctrl->Q.err_cols; i++) {
				err_workarray[i] = fabs (err_workarray[i] - err_median);
				if (err_workarray[i] < err_min) err_min=err_workarray[i];
				if (err_workarray[i] > err_max) err_max=err_workarray[i];
			}
			GMT_median (GMT, err_workarray, Ctrl->Q.err_cols, err_min, err_max, err_null_median, &err_mad);
			err_mad *= 1.482;

			/* calculate MEAN for each row */
			out[0] = err_median;
			out[1] = err_mad;
			out[2] = (Ctrl->Q.err_cols) ? err_sum / Ctrl->Q.err_cols : 0.0;

			/* print out the results */
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */

			GMT_Report (API, GMT_MSG_DEBUG, "line %d passed\n", err_l);
			err_l++;
		}
		/* close the input */
		fclose (ip);
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {
			Return (API->error);	/* Disables further data output */
		}
	}

	Return (GMT_OK);
}
