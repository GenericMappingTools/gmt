/*
 * $Id$
 *
 * grdseamount.c will create a grid made up from elliptical or circular
 * seamounts that can be Gaussian or Conical, with or without truncated
 * tops.
 *
 * Author: Paul Wessel
 * Date: 3-MAR-2031
 */

#define THIS_MODULE_NAME	"grdseamount"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Compute synthetic seamount (Gaussian or cone, circular or elliptical) bathymetry"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:RVbfhir" GMT_OPT("FH")

struct GRDSEAMOUNT_CTRL {
	struct A {	/* -A[<out>/<in>] */
		bool active;
		float value[2];	/* Inside and outside value for mask */
	} A;
	struct C {	/* -C */
		bool active;
	} C;
	struct E {	/* -E */
		bool active;
	} E;
	struct G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct L {	/* -L[<hcut>] */
		bool active;
		unsigned int mode;
		double value;
	} L;
	struct N {	/* -N<norm> */
		bool active;
		double value;
	} N;
	struct S {	/* -S<r_scale> */
		bool active;
		double value;
	} S;
	struct T {	/* -T[<flattening>] */
		bool active;
		unsigned int mode;
		double value;
	} T;
	struct Z {	/* -Z<base> */
		bool active;
		double value;
	} Z;
};

void *New_grdseamount_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDSEAMOUNT_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDSEAMOUNT_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->A.value[GMT_IN] = GMT->session.f_NaN;
	C->A.value[GMT_OUT] = 1.0f;
	C->S.value = 1.0;
	
	return (C);
}

void Free_grdseamount_Ctrl (struct GMT_CTRL *GMT, struct GRDSEAMOUNT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free (C->G.file);	
	GMT_free (GMT, C);	
}

int GMT_grdseamount_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdseamount [infile(s)] -G<outgrid> %s\n\t%s [-A[<out>/<in>]] [-C] [-E] [-L[<hcut>]] [-N<norm>]\n", GMT_I_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-S<r_scale>] [-T[<flat>]] [-Z<base>] [%s] [%s]\n\t[%s] [%s]\n\t[%s]\n",
		GMT_bi_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\tInput contains lon, lat, radius, height for each seamount.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   With -E we expect lon, lat, azimuth, semi-major, semi-minor, radius, height instead\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -T (no argument) is given a final column with flattening is expected.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Build a mAsk grid, append outside/inside values [1/NaN].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Here, height is ignored and -L, -N and -Z are disallowed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Cone model [Default is Gaussian]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Elliptical data format [Default is Circular].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Read lon, lat, azimuth, major, minor, height (m) for each seamount\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Sets name of output grdfile\n");
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t-L List area, volume, and mean-height for each seamount; NO grid is created.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append the noise-floor cutoff level [0]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Normalize grid so maximum gridheight equals <norm>.\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Sets scale factor for radii [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Seamounts are truncated.  Append flattening or expect it in last input column [no truncation]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Add in background depth [0].\n");
	GMT_Option (API, "V,bi");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Map units (lon, lat in degree, radius, major, minor in km).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is Cartesian - no units are implied].\n");
	GMT_Option (API, "h,i,r,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_grdseamount_parse (struct GMT_CTRL *GMT, struct GRDSEAMOUNT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdseamount and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_expected_fields;
	char T1[GMT_LEN32] = {""}, T2[GMT_LEN32] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				if (opt->arg[0]) {
					sscanf (opt->arg, "%[^/]/%s", T1, T2);
					Ctrl->A.value[GMT_OUT] = (T1[0] == 'N') ? GMT->session.f_NaN : (float)atof (T1);
					Ctrl->A.value[GMT_IN]  = (T2[0] == 'N') ? GMT->session.f_NaN : (float)atof (T2);
				}
				break;
			case 'C':
				Ctrl->C.active = true;
				break;
			case 'E':
				Ctrl->E.active = true;
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
			case 'L':
				Ctrl->L.active = true;
				if (opt->arg[0]) {
					Ctrl->L.mode = 1;
					Ctrl->L.value = atof (opt->arg);
				}
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.value = atof (opt->arg);
				break;
				break;
			case 'S':
				Ctrl->S.active = true;
				Ctrl->S.value = atof (opt->arg);
				break;
			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.mode = 1;
				if (opt->arg[0]) {
					Ctrl->T.value = atof (opt->arg);
					Ctrl->T.mode = 2;
				}
				break;
			case 'Z':
				Ctrl->Z.active = true;
				Ctrl->Z.value = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && (Ctrl->N.active || Ctrl->Z.active || Ctrl->L.active), "Syntax error -A option: Cannot use -L, -N or -Z with -A\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->G.active || Ctrl->G.file), "Syntax error option -G: Must specify output file\n");
	n_expected_fields = ((Ctrl->E.active) ? 6 : 4) + ((Ctrl->T.mode == 1) ? 1 : 0);
	n_errors += GMT_check_binary_io (GMT, n_expected_fields);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

void cone_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z)
{
	/* Compute area and volume of circular or elliptical conical seamounts */

	double e, r2;

	r2 = a * b;
	e = 1.0 - f;
	*A = M_PI * r2 * (1.0 - e * hc / h);
	*V = (M_PI / (3 * e)) * r2 * h * (pow (e, 3.0) * ((1.0 / e) - (hc / h)) - pow (f, 3.0));
	*z = (*V) / (*A);
}

void gaussian_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z)
{
	/* Compute area and volume of circular or elliptical Gaussian seamounts */

	bool circular = doubleAlmostEqual (a, b);
	double r, t, c, d, logt;

	if (circular) {
		r = a;
		if (fabs (hc) < GMT_CONV_LIMIT) {	/* Exact, no noise floor */
			*A = M_PI * r * r;
			*V = (2.0 / 9.0) * M_PI * r * r * h * (1.0 + (9.0 / 2.0) * f * f);
		}
		else {			/* Noise floor at hc */
			t = hc / h;
			c = 1.0 + (9.0 / 2.0) * f * f;
			*A = (2.0 / 9.0) * M_PI * r * r * ((9.0 / 2.0) * f * f - log (t));
			*V = (2.0 / 9.0) * M_PI * r * r * h * (c - t * (c - log (t)));
		}
	}
	else {		/* Elliptical cases */
		c = (9.0 / 2.0) * f * f;
		d = 3 * M_SQRT2 * f / 2.0;
		t = hc / h;
		logt = log (t);
		if (fabs (hc) < GMT_CONV_LIMIT) {	/* Exact, no noise floor */
			*A = M_PI * a * b;
			*V = (2.0 / 9.0) * M_PI * a * b * h * (pow (erfc (d), 2.0) * exp (c) + c);
		}
		else {			/* Noise floor at hc */
			*A = (2.0 / 9.0) * M_PI * a * b * (c - logt);
			*V = (2.0 / 9.0) * M_PI * a * b * h * (pow (erf (sqrt (c - logt)) - erf (d), 2.0) * exp (c) + c - t * (c - logt));
		}
	}
	*z = (*V) / (*A);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdseamount_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdseamount (void *V_API, int mode, void *args)
{
	int error, scol, srow, scol_0, srow_0;
	unsigned int n_expected_fields, n_out, nx1, d_mode = 0, row, col, row_0, col_0, max_d_col, d_row, *d_col = NULL;
	uint64_t n_read = 0, n_smts = 0, ij;
	bool map = false, periodic = false, replicate, first;
	char unit = 'X';
	double x, y, r, c, *in, this_r, A = 0.0, B = 0.0, C = 0.0, e, e2, ca, sa, ca2, sa2, r_in, dx, dy;
	double add, f, max = -DBL_MAX, r_km, amplitude, h_scale, z_assign, h_scl, noise;
	double wesn[4], rr, out[9], a, b, area, volume, height, DEG_PR_KM;
	void (*shape_func) (double a, double b, double h, double hc, double f, double *A, double *V, double *z);
	
	struct GMT_GRID *Grid = NULL;
	struct GRDSEAMOUNT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdseamount_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdseamount_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdseamount_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdseamount_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdseamount_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdseamount main code ----------------------------*/
	
	/* Specify inputexpected columns */
	n_expected_fields = ((Ctrl->E.active) ? 6 : 4) + ((Ctrl->T.mode == 1) ? 1 : 0);
	if ((error = GMT_set_cols (GMT, GMT_IN, n_expected_fields)) != GMT_OK) {
		Return (error);
	}

	/* Register likely data sources unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	shape_func = NULL;
	if (Ctrl->L.active) {	/* Just list area, volume, etc. for each seamount; no grid needed */
		n_out = n_expected_fields + 3;
		if ((error = GMT_set_cols (GMT, GMT_OUT, n_out)) != GMT_OK) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		shape_func = (Ctrl->C.active) ? cone_area_volume_height : gaussian_area_volume_height;
	}
	else {	/* Set up and allocate output grid */
		if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, NULL, Ctrl->I.inc, \
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, Ctrl->G.file)) == NULL) Return (API->error);
	}

	map = GMT_is_geographic (GMT, GMT_IN);
	GMT_set_xy_domain (GMT, wesn, Grid->header);	/* May include some padding if gridline-registered */
	nx1 = Grid->header->nx + Grid->header->registration - 1;
	if (map && GMT_360_RANGE (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) periodic = true;
	replicate = (periodic && Grid->header->registration == GMT_GRID_NODE_REG);
	if (Ctrl->A.active) for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] = Ctrl->A.value[GMT_OUT];
	DEG_PR_KM = 1.0 / GMT->current.proj.DIST_KM_PR_DEG;
	noise = exp (-4.5);		/* Normalized height of a unit Gaussian at basal radius; we must subtract this to truly get 0 at r = rbase */
	h_scl = 1.0 / (1.0 - noise);	/* Compensation scale to make the peak amplitude = 1 given our adjustment for noise above */

	if (map) d_mode = 2, unit = 'k';	/* Select km and great-circle distances */
	GMT_init_distaz (GMT, unit, d_mode, GMT_MAP_DIST);

	/* Initialize the i/o for doing record-by-record reading/writing */
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	do {	/* Keep returning records until we reach EOF */
		n_read++;
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */
	
		n_smts++;

		if (GMT_y_is_outside (GMT, in[GMT_Y],  wesn[YLO], wesn[YHI])) continue;	/* Outside y-range */
		if (GMT_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;	/* Outside x-range */

		/* Ok, we are inside the region - process data */
			
		scol_0 = (int)GMT_grd_x_to_col (GMT, in[GMT_X], Grid->header);
		if (scol_0 < 0) continue;	/* Still outside x-range */
		if ((col_0 = scol_0) >= Grid->header->nx) continue;	/* Still outside x-range */
		srow_0 = (int)GMT_grd_y_to_row (GMT, in[GMT_Y], Grid->header);
		if (srow_0 < 0) continue;	/* Still outside y-range */
		if ((row_0 = srow_0) >= Grid->header->ny) continue;	/* Still outside y-range */
		if (Ctrl->E.active) {	/* Elliptical seamount parameters */
			sincos ((90.0 - in[GMT_Z]) * D2R, &sa, &ca);	/* in[GMT_Z] is azimuth in degrees */
			a = in[3];			/* Semi-major axis */
			b = in[4];			/* Semi-minor axis */
			e = in[4] / in[3];		/* Eccentricity */
			e2 = e * e;
			ca2 = ca * ca;
			sa2 = sa * sa;
			r_km = in[4] * Ctrl->S.value;	/* Scaled semi-minor axis in user units (Cartesian or km) */
			r = r_km;
			if (map) r *= DEG_PR_KM;	/* Was in km so now it is in degrees, same units as grid coordinates */
			f = -4.5 / (r_km * r_km);	/* So we can take exp (f * radius_in_km^2) */
			A = f * (e2 * ca2 + sa2);	/* Elliptical components needed to evalute radius(az) */
			B = -f * (sa * ca * (1.0 - e2));
			C = f * (e2 * sa2 + ca2);
			r_in = in[3];			/* Semi-major axis in user units (Cartesian or km)*/
			r_km = r_in * Ctrl->S.value;	/* Scaled semi-major axis in user units (Cartesian or km) */
			r = r_km;			/* Copy of r_km */
			if (map) r *= DEG_PR_KM;	/* Was in km so now it is in degrees, same units as grid coordinates */
			amplitude = in[5];		/* Seamount max height from base */
			if (Ctrl->T.mode == 1) Ctrl->T.value = in[6];	/* Flattening given by input file */
		}
		else {	/* Circular features */
			r_in = a = b = in[GMT_Z];	/* Radius in user units */
			r_km = r_in * Ctrl->S.value;	/* Scaled up by user scale */
			r = r_km;			/* Copy of r_km */
			if (map) r *= DEG_PR_KM;	/* Was in km so now it is in degrees, same units as grid coordinates */
			f = (Ctrl->C.active) ? 1.0 / r_km : -4.5 / (r_km * r_km);	/* So we can take exp (f * radius_in_km^2) */
			amplitude = in[3];		/* Seamount max height from base */
			if (Ctrl->T.mode == 1) Ctrl->T.value = in[4];	/* Flattening given by input file */
		}
		c = (map) ? cosd (in[GMT_Y]) : 1.0;
		if (Ctrl->L.active) {	/* Only want to add back out area, volume */
			shape_func (a, b, amplitude, Ctrl->L.value, Ctrl->T.value, &area, &volume, &height);
			if (map) {
				area   *= GMT->current.proj.DIST_KM_PR_DEG * GMT->current.proj.DIST_KM_PR_DEG * c;
				volume *= GMT->current.proj.DIST_KM_PR_DEG * GMT->current.proj.DIST_KM_PR_DEG * c;
				volume *= 1.0e-3;	/* Use km^3 as unit */
			}
			for (col = 0; col < n_expected_fields; col++) out[col] = in[col];
			out[col++] = area;
			out[col++] = volume;
			out[col++] = height;
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
			continue;	/* Skip the grid part */
		}
		
		h_scale = 1.0 / ((Ctrl->C.active) ? (1.0 - Ctrl->T.value) : exp (-4.5 * Ctrl->T.value * Ctrl->T.value));	/* So h is 1 at r_top */
		if (!Ctrl->C.active) h_scale *= h_scl;

		/* Initialize local search machinery */
		if (d_col) GMT_free (GMT, d_col);
		d_col = GMT_prep_nodesearch (GMT, Grid, r_km, d_mode, &d_row, &max_d_col);
		
		for (srow = srow_0 - (int)d_row; srow <= (srow_0 + (int)d_row); srow++) {
			if (srow < 0 ) continue;
			if ((row = srow) >= Grid->header->ny) continue;
			y = GMT_grd_row_to_y (GMT, row, Grid->header);
			first = replicate;	/* Used to help us deal with duplicate columns for grid-line registered global grids */
			for (scol = scol_0 - (int)d_col[row]; scol <= (scol_0 + (int)d_col[row]); scol++) {
				if (!periodic) {
					if (scol < 0) continue;
					if ((col = scol) >= Grid->header->nx) continue;
				}
				if (scol < 0)	/* Periodic grid: Break on through to other side! */
					col = scol + nx1;
				else if ((col = scol) >= Grid->header->nx) 	/* Periodic grid: Wrap around to other side */
					col -= nx1;
				/* "silent" else we are inside w/e */
				x = GMT_grd_col_to_x (GMT, col, Grid->header);
				this_r = GMT_distance (GMT, in[GMT_X], in[GMT_Y], x, y);	/* In Cartesian units or km (if map is true) */
				if (this_r > r_km) continue;	/* Beyond the base of the seamount */
				if (Ctrl->E.active) {	/* Here we must deal with direction etc */
					dx = (map) ? (x - in[GMT_X]) * GMT->current.proj.DIST_KM_PR_DEG * c : (x - in[GMT_X]);
					dy = (map) ? (y - in[GMT_Y]) * GMT->current.proj.DIST_KM_PR_DEG : (y - in[GMT_Y]);
					this_r = A * dx * dx + 2.0 * B * dx * dy + C * dy * dy;
					/* this_r is now r^2 in the 0 to -4.5 range expected for the Gaussian case */
					rr = sqrt (-this_r/4.5);	/* Convert this r^2 to a normalized radius 0-1 inside cone */
					if (Ctrl->A.active && rr > 1.0) continue;	/* Beyond the seamount base so nothing to do for a mask */
					if (Ctrl->C.active) {	/* Elliptical cone case */
						if (rr < 1.0)	/* Since in minor direction rr may exceed 1 and is outside ellipse */
							add = (rr < Ctrl->T.value) ? 1.0 : (1.0 - rr) * h_scale;
						else
							add = 0.0;
					}
					else	/* Elliptical Gaussian case */
						add = (rr < Ctrl->T.value) ? 1.0 : exp (this_r) * h_scale - noise;
				}
				else {	/* Circular features */
					rr = this_r / r_km;	/* Now in 0-1 range */
					if (Ctrl->C.active)	/* Circular cone case */
						add = (rr < Ctrl->T.value) ? 1.0 : (1.0 - rr) * h_scale;
					else	/* Circular Gaussian case */
						add = (rr < Ctrl->T.value) ? 1.0 : exp (f * this_r * this_r) * h_scale - noise;
				}
				if (add < 0.0) continue;
				ij = GMT_IJP (Grid->header, row, col);	/* Current node location */
				z_assign = amplitude * add;		/* height to be added */
				if (Ctrl->A.active)	/* Just set inside value for mask */
					Grid->data[ij] = Ctrl->A.value[GMT_IN];
				else {	/* Add in contribution and keep track of max height */
					Grid->data[ij] += (float)z_assign;
					if (Grid->data[ij] > max) max = Grid->data[ij];
				}
				if (first) {	/* May have to copy to repeated column in global gridline-registered grids */
					if (col == 0) {	/* Must copy from x_min to repeated column at x_max */
						if (Ctrl->A.active) Grid->data[ij+nx1] = Ctrl->A.value[GMT_IN]; else Grid->data[ij+nx1] += (float)z_assign;
						first = false;
					}
					else if (col == nx1) {	/* Must copy from x_max to repeated column at x_min */
						if (Ctrl->A.active) Grid->data[ij-nx1] = Ctrl->A.value[GMT_IN]; else Grid->data[ij-nx1] += (float)z_assign;
						first = false;
					}
				}
			}
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Evaluated seamount # %6d\r", n_smts);
	} while (true);

	GMT_Report (API, GMT_MSG_VERBOSE, "Evaluated seamount # %6d\n", n_smts);
	
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}
	if (Ctrl->L.active) {
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}
	}
	else {
		if (Ctrl->N.active) {	/* Normalize so max height == N.value */
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Normalize seamount amplitude so max height is %g\r", Ctrl->N.value);
			Ctrl->N.value /= max;
			for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] *= (float)Ctrl->N.value;
		}
		if (Ctrl->Z.active) {	/* Add in the background depth */
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Add in a background level of %g\r", Ctrl->Z.value);
			for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] += (float)Ctrl->Z.value;
		}
	
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Grid) != GMT_OK) {
			Return (API->error);
		}
	}
	GMT_free (GMT, d_col);
	
	Return (GMT_OK);
}
