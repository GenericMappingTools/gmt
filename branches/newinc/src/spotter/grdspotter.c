/*--------------------------------------------------------------------
 *	$Id$
 *
 *   Copyright (c) 1999-2012 by P. Wessel
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation; version 3 or any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   Contact info: www.soest.hawaii.edu/pwessel
 *--------------------------------------------------------------------*/
/*
 * GRDSPOTTER will (1) read a grid with topo/grav/whatever of seamounts,
 * (2) read an ascii file with stage or finite rotations, and (3)
 * convolve the flowline of each node with its prism volume, and
 * (4) build a cumulative volcano amplitude (CVA) grid.  Optionally, it
 * can also calculate the Data Importance (DI) associated with each node's
 * flowline and the predicted age (PA) for each node.  Finally, errors in
 * the location of the CVA maximum can be assessed by running a bootstrap
 * estimation.  The grids are all written out in GMT format and can be
 * processed and plotted with GMT.
 * GRDSPOTTER is part of the SPOTTER supplemental GMT package and should
 * be installed accordingly, see README.spotter.
 *
 * Author:	Paul Wessel, SOEST, Univ. of Hawaii, Honolulu, HI, USA
 * Date:	20-OCT-2007
 * Version:	2.0
 *
 *-------------------------------------------------------------------------
 * The Euler file must have following format:
 *
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 2. Any number of stage pole records which each have the format:
 *    lon(deg)  lat(deg)  tstart(Ma)  tstop(Ma)  ccw-angle(deg)
 * 3. stage records must go from oldest to youngest rotation
 * 4. Note tstart is larger (older) that tstop for each record
 * 5. No gaps allowed: tstart must equal the previous records tstop
 *
 * Example: Duncan & Clague [1985] Pacific-Hotspot rotations:
 *
 * # Time in Ma, angles in degrees
 * # lon  lat	tstart	tend	ccw-angle
 * 165     85	150	100	24.0
 * 284     36	100	74	15.0
 * 265     22	74	65	7.5
 * 253     17	65	42	14.0
 * 285     68	42	0	34.0
 *
 * If total reconstruction poles are preferred,
 * give a file like DC85 poles in finite rotation format:
 *
 * #longitude	latitude	time(My)	angle(deg)
 * 285.00000	 68.00000	 42.0000	 34.0000
 * 275.66205	 53.05082	 65.0000	 43.5361
 * 276.02501	 48.34232	 74.0000	 50.0405
 * 279.86436	 46.30610	100.0000	 64.7066
 * 265.37800	 55.69932	150.0000	 82.9957
 *
 * Note that finite rotation files have one less column since each rotation
 * implicitly goes to time 0.
 *
 * Seamount data grid is a standard GMT grid file, presumably
 * representing topography or gravity of residual seamounts
 * (background residual should be removed first).
 *
 *------------------------------------------------------------------------------
 * REFERENCES:
 *
 * -> The hotspotting technique:
 *
 * Aslanian, D., L. Geli, and J.-L. Olivet, 1998, Hotspotting called into
 *    question, Nature, 396, 127.
 * Wessel, P., and L. W. Kroenke, 1997, A geometric technique for
 *    relocating hotspots and refining absolute plate motions, Nature,
 *    387, 365-369.
 * Wessel, P., and L. W. Kroenke, 1998a, Factors influencing the locations
 *    of hot spots determined by the hot-spotting technique, Geophys. Res.
 *    Lett., 25, 55-58.
 * Wessel, P., and L. W. Kroenke, 1998b, The geometric relationship between
 *    hot spots and seamounts: implications for Pacific hot spots, Earth
 *    Planet. Sci. Lett., 158, 1-18.
 * Wessel, P., and L. W. Kroenke, 1998c, Hotspotting called into question
 *    - Reply, Nature, 396, 127-128.
 *
 * -> Suitable Seamount data set:
 *
 * Sandwell, D, and W. H. F. Smith, 1995, JGR, ...
 * Smith, W. H. F., and D. Sandwell, 1997, Science, ...
 *
 * -> Plate motion models (stage poles):
 *
 * Duncan, R.A., and D. Clague, 1985, Pacific plate motion recorded by linear
 *    volcanic chains, in: A.E.M. Nairn, F. G. Stehli, S. Uyeda (eds.), The
 *    Ocean Basins and Margins, Vol. 7A, Plenum, New York, pp. 89-121.
 * Wessel and Kroenke, 1997, (see above)
 *
 */
 
#define THIS_MODULE_NAME	"grdspotter"
#define THIS_MODULE_LIB		"spotter"
#define THIS_MODULE_PURPOSE	"Create CVA image from a gravity or topography grid"

#include "spotter.h"

#define GMT_PROG_OPTIONS "-:>RVhr" GMT_OPT("F")

#define B_TO_MB	(1.0 / 1048576.0)

#define TRUNC	0	/* Indices for -T */
#define UPPER	1

struct GRDSPOTTER_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct In {
		bool active;
		char *file;
	} In;
	struct A {	/* -A<file> */
		bool active;
		char *file;
	} A;
	struct D {	/* -Di<file> */
		bool active;
		char *file;
	} D;
	struct E {	/* -E[+rotfile */
		bool active;
		bool mode;
		char *file;
	} E;
	struct G {	/* -Ggrdfile */
		bool active;	/* Pixel registration */
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct L {	/* -Lfile */
		bool active;
		char *file;
	} L;
	struct M {	/* -M */
		bool active;
	} M;
	struct N {	/* -N */
		bool active;
		double t_upper;
	} N;
	struct PA {	/* -Dp<file> */
		bool active;
		char *file;
	} PA;
	struct Q {	/* -Q */
		bool active;
		unsigned int mode;
		unsigned int id;
		char *file;
	} Q;
	struct S2 {	/* -S2 */
		bool active;
		double dist;
	} S2;
	struct S {	/* -S */
		bool active;
	} S;
	struct T {	/* -T */
		bool active[2];
		double t_fix;	/* Set fixed age*/
	} T;
	struct W {	/* -W */
		bool active;
		unsigned int n_try;
	} W;
	struct Z {	/* -Z */
		bool active;
		bool mode;
		double min, max, inc;
	} Z;
};

void *New_grdspotter_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDSPOTTER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDSPOTTER_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	C->N.t_upper = 180.0;	/* Upper age assigned to nodes on undated seafloor */
	C->Z.max = DBL_MAX;	/* Only deal with z-values inside this range [0/Inf] */
	return (C);
}

void Free_grdspotter_Ctrl (struct GMT_CTRL *GMT, struct GRDSPOTTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->A.file) free (C->A.file);	
	if (C->D.file) free (C->D.file);	
	if (C->E.file) free (C->E.file);	
	if (C->G.file) free (C->G.file);	
	if (C->L.file) free (C->L.file);	
	if (C->PA.file) free (C->PA.file);	
	GMT_free (GMT, C);	
}

int GMT_grdspotter_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdspotter <ingrid> -E[+]<rottable> -G<CVAgrid> %s\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-A<agegrid>] [-D[i|p]<grdfile>] [-L<IDgrid>] [-M]\n", GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-N<upper_age>] [-Q<IDinfo>] [-S] [-Tt|-u<age>] [%s] [-W<n_try]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Z<z_min>[/<z_max>[/<z_inc>]]] [%s] [%s]\n\n", GMT_ho_OPT, GMT_r_OPT);
	
	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);
	
	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is the grid with topo or gravity\n");
	spotter_rot_usage (API, 'E');
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output CVA convolution grid.\n");
	GMT_Option (API, "I,Rg");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Co-registered grid with upper ages to use [Default is flowlines for all ages].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set optional output grids:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Di<file> Use flowlines to estimate data importance DI grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Dp<file> Use flowlines to estimate predicted ages at node locations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Co-registered grid with chain ID for each node [Default ignores IDs].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Do flowline calculations as needed rather than storing in memory.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   You may have to use this option if -R is too large. Cannot be used with -W or -Z-slicing.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set upper age in m.y. for nodes whose plate age is NaN [180].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Either single ID to use or file with list of IDs [Default uses all IDs].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Each line would be TAG ID [w e s n] with optional zoom box.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Normalize CVA grid to percentages of the CVA maximum.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set upper ages.  Repeatable, choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  -Tt truncate all ages to max age in stage pole model [Default extrapolates].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  -Tu<age> After a node passes the -Z test, use this fixed age instead in CVA calculations.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Get <n_try> bootstrap estimates of maximum CVA location [Default is no bootstrapping].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Ignore nodes with z-value lower than z_min [0] and optionally larger than z_max [Inf].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give z_min/z_max/z_inc to make CVA grids for each z-slice {Default makes 1 CVA grid].\n");
	GMT_Option (API, "h,r,.");

	return (EXIT_FAILURE);
}

int GMT_grdspotter_parse (struct GMT_CTRL *GMT, struct GRDSPOTTER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdspotter and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n_files = 0;
	int m, sval;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Supplemental parameters */

			case 'A':
				if ((Ctrl->A.active = GMT_check_filearg (GMT, 'A', opt->arg, GMT_IN)))
					Ctrl->A.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'C':	/* Now done automatically in spotter_init */
				if (GMT_compat_check (GMT, 4))
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -C is no longer needed as total reconstruction vs stage rotation is detected automatically.\n");
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'E':
				Ctrl->E.active = true;	k = 0;
				if (opt->arg[0] == '+') { Ctrl->E.mode = true; k = 1;}
				if (GMT_check_filearg (GMT, 'E', &opt->arg[k], GMT_IN))
					Ctrl->E.file  = strdup (&opt->arg[k]);
				else
					n_errors++;
				break;
			case 'G':
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'D':
				switch (opt->arg[0]) {
					case 'i':
						if ((Ctrl->D.active = GMT_check_filearg (GMT, 'D', &opt->arg[1], GMT_OUT)))
							Ctrl->D.file = strdup (&opt->arg[1]);
						else
							n_errors++;
						break;
					case 'p':
						if ((Ctrl->PA.active = GMT_check_filearg (GMT, 'D', &opt->arg[1], GMT_OUT)))
							Ctrl->PA.file = strdup (&opt->arg[1]);
						else
							n_errors++;
						break;
					default:
						n_errors++;
						break;
				}
				break;
			case 'I':
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':
				if ((Ctrl->L.active = GMT_check_filearg (GMT, 'L', opt->arg, GMT_IN)))
					Ctrl->L.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 's':	/* Sampling interval */
				Ctrl->S2.active = true;
				Ctrl->S2.dist = atof (opt->arg);
				break;
			case 'M':
				Ctrl->M.active = true;
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.t_upper = atof (opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				if (!access (opt->arg, R_OK)) {	/* The file exists */
					Ctrl->Q.mode = 2;
					Ctrl->Q.file = strdup (opt->arg);
				}
				else if ((sval = atoi (opt->arg)) > 0) {	/* Got OK id value */
					Ctrl->Q.mode = 1;
					Ctrl->Q.id = sval;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Error -Q: Must give valid file or ID value\n");
					n_errors++;
				}
				break;
			case 'S':
				Ctrl->S.active = true;
				break;
			case 'T':
				if (opt->arg[0] == 't')
					Ctrl->T.active[TRUNC] = true;
				else if (opt->arg[0] == 'u') {
					Ctrl->T.t_fix = atof (&opt->arg[1]);
					Ctrl->T.active[UPPER] = true;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Error -T: Either use -Tt or -Tu<age>\n");
					n_errors++;
				}
				break;
			case 'W':
				Ctrl->W.n_try = atoi (opt->arg);
				Ctrl->W.active = true;
				break;
			case 'Z':
				Ctrl->Z.active = true;
				m = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->Z.min, &Ctrl->Z.max, &Ctrl->Z.inc);
				if (m == 1) Ctrl->Z.max = 1.0e300;	/* Max not specified */
				if (m == 3) Ctrl->Z.mode = true;	/* Want several slices */
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->G.active || Ctrl->G.file), "Syntax error -G: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->In.active || Ctrl->In.file), "Syntax error -Z: Must give name of topo gridfile\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.file && !Ctrl->Q.mode, "Syntax error: Must specify both -L and -Q if one is present\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && (Ctrl->W.active || Ctrl->Z.mode), "Syntax error: Cannot use -M with -W or -Z (slicing)\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int64_t get_flowline (struct GMT_CTRL *GMT, double xx, double yy, double tt, struct EULER *p, unsigned int n_stages, double d_km, unsigned int step, unsigned int flag, double wesn[], double **flow)
{
	int64_t n_track, m, kx, ky, np, first, last;
	double *c = NULL, *f = NULL;

	/* Get the flowline from this point back to time tt, restricted to the given wesn box */
	if (spotter_forthtrack (GMT, &xx, &yy, &tt, 1, p, n_stages, d_km, 0.0, flag, wesn, &c) <= 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Nothing returned from spotter_forthtrack - skipping\n");
		return 0LL;
	}

	n_track = lrint (c[0]);				/* Number of point pairs making up this flowline */

	/* Find the first point on the flowline inside the desired CVA region */

	for (m = 0, ky = 2, first = -1; m < n_track && first == -1; m++, ky += step) {	/* For each point along flowline */
		if (c[ky] < wesn[YLO] || c[ky] > wesn[YHI]) continue;	/* Latitude outside region */
		kx = ky - 1;						/* Index for the x-coordinate */
		while (c[kx] > wesn[XHI]) c[kx] -= TWO_PI;		/* Elaborate W/E test because of 360 periodicity */
		while (c[kx] < wesn[XLO]) c[kx] += TWO_PI;
		if (c[kx] > wesn[XHI]) continue;			/* Longitude outside region */
		first = kx;						/* We are inside, this terminates the for loop */
	}

	if (first == -1) { 	/* Was never inside the grid, skip the entire flowline and move on */
		GMT_free (GMT, c);	/* Free the flowline vector */
		return 0LL;
	}

	/* Here we know searching from the end will land inside the grid eventually so last can never exit as -1 */

	for (m = n_track - 1, ky = step * m + 2, last = -1; m >= 0 && last == -1; m--, ky -= step) {	/* For each point along flowline */
		if (c[ky] < wesn[YLO] || c[ky] > wesn[YHI]) continue;	/* Latitude outside region */
		kx = ky - 1;						/* Index for the x-coordinate */
		while (c[kx] > wesn[XHI]) c[kx] -= TWO_PI;		/* Elaborate W/E test because of 360 periodicity */
		while (c[kx] < wesn[XLO]) c[kx] += TWO_PI;
		if (c[kx] > wesn[XHI]) continue;			/* Longitude outside region */
		last = kx;						/* We are inside, this terminates the for loop */
	}

	np = (last - first) / step + 1;			/* Number of (x,y[,t]) points on this flowline inside the region */
	if (np < n_track) {	/* Just copy out the subset of points we want */
		size_t n_alloc;
		n_alloc = np * step;	/* Number of (x,y[,t]) to copy */
		f = GMT_memory (GMT, NULL, n_alloc+1, double);
		f[0] = (double)np;	/* Number of points found */
		GMT_memcpy (&f[1], &c[first], n_alloc, double);
		GMT_free (GMT, c);	/* Free the old flowline vector */
		*flow = f;		/* Return pointer to trimmed flowline */
	}
	else
		*flow = c;		/* Return the entire flowline as is */
	return (np);
}

bool set_age (struct GMT_CTRL *GMT, double *t_smt, struct GMT_GRID *A, uint64_t node, double upper_age, bool truncate)
{
	/* Returns the age of this node based on either a given seafloor age grid
	 * or the upper age, truncated if necessary */

	if (!A || GMT_is_fnan (A->data[node]))		/* Age is NaN, assign upper value */
		*t_smt = upper_age;
	else {	/* Assign given value */
		*t_smt = A->data[node];
		if (*t_smt > upper_age) {	/* Exceeding our limit */
			if (truncate)		/* Allowed to truncate to max age */
				*t_smt = upper_age;
			else {			/* Consider this an error or just skip */
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Node %" PRIu64 " has age (%g) > oldest stage (%g) (skipped)\n", node, *t_smt, upper_age);
				return (false);
			}
		}
	}
	return (true);	/* We are returning a useful age */
}

void normalize_grid (struct GMT_CTRL *GMT, struct GMT_GRID *G, float *data)
{	/* Determines the grid's min/max z-values and normalizes the grid
	 * so that zmax is 100% */
	unsigned int row, col;
	uint64_t node;
	double CVA_scale;	/* Used to normalize CVAs to percent */
	
	G->header->z_min = +DBL_MAX;
	G->header->z_max = -DBL_MAX;
	GMT_grd_loop (GMT, G, row, col, node) {	/* Loop over all output nodes */
		if (GMT_is_fnan (data[node])) continue;
		if (data[node] < G->header->z_min) G->header->z_min = data[node];
		if (data[node] > G->header->z_max) G->header->z_max = data[node];
	}
	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "CVA min/max: %g %g -> ", G->header->z_min, G->header->z_max);
	CVA_scale = 100.0 / G->header->z_max;
	for (node = 0; node < G->header->size; node++) data[node] *= (float)CVA_scale;
	G->header->z_min *= CVA_scale;
	G->header->z_max *= CVA_scale;
	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "%g %g\n", G->header->z_min, G->header->z_max);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdspotter_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdspotter (void *V_API, int mode, void *args)
{
	unsigned int n_stages;	/* Number of stage rotations (poles) */
	unsigned int try;		/* Number of current bootstrap estimate */
	unsigned int row, row2, col, col2, k_step;
	unsigned int forth_flag;	/* Holds the do_time + 10 flag passed to forthtrack */
	bool keep_flowlines = false;	/* true if Ctrl->D.active, Ctrl->PA.active, or bootstrap is true */
	int error = 0;			/* nonzero when arguments are wrong */
	int i, j;			/* Signed row,col variables */
	int *ID = NULL;		/* Optional array with IDs for each node */
	
	uint64_t ij, k, node, m, np, max_ij = 0, n_flow, n_unique_nodes = 0;
	uint64_t n_nodes;		/* Number of nodes processed */

	size_t n_alloc = 0, inc_alloc = BIG_CHUNK, mem = 0;

	char *processed_node = NULL;	/* Pointer to array with true/false values for each grid node */
	
	unsigned short pa = 0;		/* Placeholder for PA along track */

	float zz;

	double sampling_int_in_km;	/* Sampling interval along flowline (in km) */
	double *x_smt = NULL;		/* node longitude (input degrees, stored as radians) */
	double *y_smt = NULL;		/* node latitude (input degrees, stored as radians) */
	double t_smt = 0.0;		/* node upper age (up to age of seafloor) */
	double *c = NULL;		/* Array with one flowline */
	double CVA_max, wesn[4], cva_contribution, yg;
	double out[3], x_scale, y_scale, area;
	double *lat_area = NULL;	/* Area of each dx by dy note in km as function of latitude */
	double this_wesn[4];
	double this_pa, pa_val = 0.0, n_more_than_once = 0.0;
	double *x_cva = NULL, *y_cva = NULL;		/* Coordinates on the CVA grid */
	
	struct GMT_GRID *G = NULL;	/* Grid structure for output CVA grid */
	struct GMT_GRID *G_rad = NULL;	/* Same but has radians in header (no grid) */
	struct GMT_GRID *Z = NULL;	/* Grid structure for input topo/grav grid */
	struct GMT_GRID *A = NULL;	/* Grid structure for input age grid */
	struct GMT_GRID *L = NULL;	/* Grid structure for input ID grid */
	struct GMT_GRID *PA = NULL;	/* Grid structure for output PA grid */
	struct GMT_GRID *DI = NULL;	/* Grid structure for output DI grid */
	
	struct EULER *p = NULL;		/* Array of structures with Euler stage rotations */

	struct FLOWLINE *flowline = NULL;	/* Array with flowline structures */
	
	struct ID {			/* Information regarding one chain ID */
		double wesn[4];		/* Do not calculate flowlines outside this box */
		bool ok;		/* true if we want to calculate this CVA */
		bool check_region;	/* true if w, e, s, n is more restrictive than command line -R */
	} *ID_info = NULL;
	struct GMT_OPTION *ptr = NULL;
	struct GRDSPOTTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdspotter_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdspotter_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdspotter_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if ((ptr = GMT_Find_Option (API, 'f', options)) == NULL) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdspotter_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdspotter_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdspotter main code ----------------------------*/

	/* Initialize the CVA grid and structure */

	if ((G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, NULL, Ctrl->I.inc, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, Ctrl->G.file)) == NULL) Return (API->error);
	
	/* ------------------- END OF PROCESSING COMMAND LINE ARGUMENTS  --------------------------------------*/

	/* Load in the Euler stage poles */

	n_stages = spotter_init (GMT, Ctrl->E.file, &p, true, false, Ctrl->E.mode, &Ctrl->N.t_upper);

	/* Assign grid-region variables in radians to avoid conversions inside convolution loop */

	if ((G_rad = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, G)) == NULL) Return (API->error);
	G_rad->header->inc[GMT_X] = G->header->inc[GMT_X] * D2R;
	G_rad->header->inc[GMT_Y] = G->header->inc[GMT_Y] * D2R;
	G_rad->header->wesn[XLO]  = G->header->wesn[XLO] * D2R;
	G_rad->header->wesn[XHI]  = G->header->wesn[XHI] * D2R;
	G_rad->header->wesn[YLO]  = G->header->wesn[YLO] * D2R;
	G_rad->header->wesn[YHI]  = G->header->wesn[YHI] * D2R;
	
	/* Get geocentric wesn region in radians */
	GMT_memcpy (wesn, G_rad->header->wesn, 4, double);
	wesn[YLO] = D2R * GMT_lat_swap (GMT, R2D * wesn[YLO], GMT_LATSWAP_G2O);
	wesn[YHI] = D2R * GMT_lat_swap (GMT, R2D * wesn[YHI], GMT_LATSWAP_G2O);

	/* Allocate T/F array */

	processed_node = GMT_memory (GMT, NULL, G->header->size, char);

	/* Set flowline sampling interval to 1/2 of the shortest distance between x-nodes */

	/* sampling_int_in_km = 0.5 * G_rad->header->inc[GMT_X] * EQ_RAD * ((fabs (G_rad->header->wesn[YHI]) > fabs (G_rad->header->wesn[YLO])) ? cos (G_rad->header->wesn[YHI]) : cos (G_rad->header->wesn[YLO])); */
	sampling_int_in_km = G_rad->header->inc[GMT_X] * EQ_RAD * ((fabs (G_rad->header->wesn[YHI]) > fabs (G_rad->header->wesn[YLO])) ? cos (G_rad->header->wesn[YHI]) : cos (G_rad->header->wesn[YLO]));
	if (Ctrl->S2.dist != 0.0) sampling_int_in_km = Ctrl->S2.dist;
	GMT_Report (API, GMT_MSG_VERBOSE, "Flowline sampling interval = %.3f km\n", sampling_int_in_km);

	if (Ctrl->T.active[TRUNC]) GMT_Report (API, GMT_MSG_VERBOSE, "Ages truncated to %g\n", Ctrl->N.t_upper);

	/* Start to read input data */
	
	if (GMT_Init_IO (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}

	if ((Z = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get data */
		Return (API->error);
	}
	area = 111.195 * Z->header->inc[GMT_Y] * 111.195 * Z->header->inc[GMT_X];	/* In km^2 at Equator */
	x_smt = GMT_memory (GMT, NULL, Z->header->nx, double);
	for (col = 0; col < Z->header->nx; col++) x_smt[col] = D2R * GMT_grd_col_to_x (GMT, col, Z->header);
	y_smt = GMT_memory (GMT, NULL, Z->header->ny, double);
	for (row = 0; row < Z->header->ny; row++) y_smt[row] = D2R * GMT_lat_swap (GMT, GMT_grd_row_to_y (GMT, row, Z->header), GMT_LATSWAP_G2O);	/* Convert to geocentric */
	lat_area = GMT_memory (GMT, NULL, Z->header->ny, double);

	for (row = 0; row < Z->header->ny; row++) lat_area[row] = area * cos (y_smt[row]);
	
	x_cva = GMT_grd_coord (GMT, G->header, GMT_X);
	y_cva = GMT_grd_coord (GMT, G->header, GMT_Y);

	if (Ctrl->A.file) {
		if ((A = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->A.file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
		if (!(A->header->nx == Z->header->nx && A->header->ny == Z->header->ny && A->header->wesn[XLO] == Z->header->wesn[XLO] && A->header->wesn[YLO] == Z->header->wesn[YLO])) {
			GMT_Report (API, GMT_MSG_NORMAL, "Topo grid and age grid must coregister\n");
			Return (EXIT_FAILURE);
		}
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->A.file, A) == NULL) {	/* Get age data */
			Return (API->error);
		}
	}
	if (Ctrl->L.file) {
		if ((L = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->L.file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
		if (!(L->header->nx == Z->header->nx && L->header->ny == Z->header->ny && L->header->wesn[XLO] == Z->header->wesn[XLO] && L->header->wesn[YLO] == Z->header->wesn[YLO])) {
			GMT_Report (API, GMT_MSG_NORMAL, "Topo grid and ID grid must coregister\n");
			Return (EXIT_FAILURE);
		}
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->L.file, L) == NULL) {	/* Get ID data */
			Return (API->error);
		}
		
		/* Store IDs in a int array instead */
		ID = GMT_memory (GMT, NULL, L->header->size, int);
		for (ij = 0; ij < L->header->size; ij++) ID[ij] = irint ((double)L->data[ij]);
		GMT_free_aligned (GMT, L->data);	/* Just free the array since we use ID; Grid stuct is destroyed at end */
		
		ID_info = GMT_memory (GMT, NULL, lrint (L->header->z_max) + 1, struct ID);
		if (Ctrl->Q.mode == 1) {	/* Only doing one CVA with no extra restrictions */
			ID_info[Ctrl->Q.id].ok = true;	/* Every other info in struct array is NULL or 0 */
		}
		else {	/* Must read from file */
			FILE *fp = NULL;
			int Qid;
			double wq, eq, sq, nq;
			char line[GMT_BUFSIZ] = {""};
			
			if ((fp = fopen (Ctrl->Q.file, "r")) == NULL) {	/* Oh, oh... */
				GMT_Report (API, GMT_MSG_NORMAL, "Error: -Q info file unreadable/nonexistent\n");
				exit (EXIT_FAILURE);
			}
			while (fgets (line, GMT_BUFSIZ, fp)) {
				if (line[0] == '#' || line[0] == '\n') continue;
				k = sscanf (line, "%*s %d %lf %lf %lf %lf", &Qid, &wq, &eq, &sq, &nq);
				ID_info[Ctrl->Q.id].ok = true;
				if (k == 5) {	/* Got restricted wesn also */
					ID_info[Qid].check_region = true;
 					sq = GMT_lat_swap (GMT, sq, GMT_LATSWAP_G2O);	/* Go geocentric */
 					nq = GMT_lat_swap (GMT, nq, GMT_LATSWAP_G2O);	/* Go geocentric */
					ID_info[Qid].wesn[XLO] = wq * D2R;
					ID_info[Qid].wesn[XHI] = eq * D2R;
					ID_info[Qid].wesn[YLO] = sq * D2R;
					ID_info[Qid].wesn[YHI] = nq * D2R;
				}
			}
			fclose (fp);
		}
		Ctrl->Q.mode = true;
	}

	if (Ctrl->M.active) {
		keep_flowlines = false;	/* Do it the hard way to save memory */
		k_step = 2;		/* Reset back to 3 if needed later */
		forth_flag = 10;	/* The 10 is used to limit the flowline calculation to only resample track within the rectangular box of interest */
	}
	else {
		keep_flowlines = (Ctrl->PA.active || Ctrl->D.active || Ctrl->W.active);
		forth_flag = (Ctrl->PA.active) ? 11 : 10;	/* The 10 is used to limit the flowline calculation to only resample track within the rectangular box of interest */
		k_step = (Ctrl->PA.active) ? 3 : 2;
	}
	if (keep_flowlines) {
		n_alloc = inc_alloc;
		flowline = GMT_memory (GMT, NULL, n_alloc, struct FLOWLINE);
		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
			GMT_Message (API, GMT_TIME_NONE, "Will attempt to keep all flowlines in memory.  However, should this not be possible\n");
			GMT_Message (API, GMT_TIME_NONE, "the program might crash.  If so consider using the -M option\n");
		}
	}
	
	n_flow = n_nodes = 0;
	GMT_grd_loop (GMT, Z, row, col, ij) {	/* Loop over all input nodes */
		/* STEP 1: Determine if z exceeds threshold and if so assign age */
		if (GMT_is_fnan (Z->data[ij]) || Z->data[ij] < Ctrl->Z.min || Z->data[ij] > Ctrl->Z.max) continue;	/* Skip node since it is NaN or outside the Ctrl->Z.min < z < Ctrl->Z.max range */
		if (Ctrl->Q.mode && !ID_info[ID[ij]].ok) continue;				/* Skip because of wrong ID */
		if (!set_age (GMT, &t_smt, A, ij, Ctrl->N.t_upper, Ctrl->T.active[TRUNC])) continue;	/* Skip if age is given and we are outside range */
		/* STEP 2: Calculate this node's flowline */
		if (Ctrl->Q.mode && ID_info[ID[ij]].check_region) /* Set up a box-limited flowline sampling */
			GMT_memcpy (this_wesn, ID_info[ID[ij]].wesn, 4, double);
		else
			GMT_memcpy (this_wesn, wesn, 4, double);
		
		np = get_flowline (GMT, x_smt[col], y_smt[row], t_smt, p, n_stages, sampling_int_in_km, k_step, forth_flag, this_wesn, &c);
		if (np == 0) continue;	/* No flowline inside this wesn */

		/* STEP 3: Convolve this flowline with node shape and add to CVA grid */

		/* Our convolution is approximate:  We sample the flowline frequently and use
		 * one of the points on the flowline that are closest to the node.  Ideally we
		 * want the nearest distance from each node to the flowline. Later versions may
		 * improve on this situation */

		GMT_memset (processed_node, G->header->size, char);	/* Fresh start for this flowline convolution */
		
		if (keep_flowlines) {
			flowline[n_nodes].node = GMT_memory (GMT, NULL, np, uint64_t);
			if (Ctrl->PA.active) flowline[n_nodes].PA = GMT_memory (GMT, NULL, np, unsigned short);
		}
		
		/* Keep in mind that although first and last are entry/exit into the CVA grid, we must
		 * expect the flowline between those points to also exit/reentry the CVA grid; hence
		 * we must still check if points are in/out of the box.  Here, we do not skip points but
		 * set the node index to UINTMAX_MAX */
		 
		cva_contribution = lat_area[row] * (Ctrl->T.active[UPPER] ? Ctrl->T.t_fix : Z->data[ij]);	/* This node's contribution to the convolution */

#ifdef DEBUG2
		printf ("> %" PRIu64 " %" PRIu64 " %d %d %g\n", n_nodes, np, row, col, cva_contribution);
#endif
		for (m = 0, k = 1; m < np; m++) {	/* Store nearest node indices only */
			i = (int)GMT_grd_x_to_col (GMT, c[k++], G_rad->header);
			yg = GMT_lat_swap (GMT, R2D * c[k++], GMT_LATSWAP_O2G);		/* Convert back to geodetic */
			j = (int)GMT_grd_y_to_row (GMT, yg, G->header);
			if (i < 0 || (col2 = i) >= G->header->nx || j < 0 || (row2 = j) >= G->header->ny)	/* Outside the CVA box, flag as outside */
				node = UINTMAX_MAX;
			else								/* Inside the CVA box, assign node ij */
				node = GMT_IJP (G->header, row2, col2);
			if (keep_flowlines) {
				flowline[n_nodes].node[m] = node;
				if (Ctrl->PA.active) flowline[n_nodes].PA[m] = (unsigned short) lrint (c[k++] * T_2_PA);
			}
			/* If we did not keep flowlines then there is no PA to skip over (hence no k++) */
			if (node != UINTMAX_MAX) {
				if (!processed_node[node]) {	/* Have not added to the CVA at this node yet */
					G->data[node] += (float)cva_contribution;
					processed_node[node] = true;		/* Now we have visited this node */
					n_unique_nodes++;
#ifdef DEBUG2
					printf ("%g\t%g\n", x_cva[col2], y_cva[row2]);
#endif
				}
				n_more_than_once += 1.0;
			}
		}
		if (keep_flowlines) {
			flowline[n_nodes].n = np;	/* Number of points in flowline */
			flowline[n_nodes].ij = ij;	/* Originating node in topo grid */
			mem += sizeof (struct FLOWLINE) + np * sizeof (uint64_t) + ((Ctrl->PA.active) ? np * sizeof (unsigned short) : 0);
		}

		GMT_free (GMT, c);	/* Free the flowline vector */

		n_nodes++;	/* Go to next node */

		if (keep_flowlines && n_nodes == n_alloc) {
			size_t old_n_alloc = n_alloc;
			inc_alloc *= 2;
			n_alloc += inc_alloc;
			flowline = GMT_memory (GMT, flowline, n_alloc, struct FLOWLINE);
			GMT_memset (&(flowline[old_n_alloc]), n_alloc - old_n_alloc, struct FLOWLINE);	/* Set to NULL/0 */
		}
		
		if (!(n_nodes%100)) GMT_Report (API, GMT_MSG_VERBOSE, "Row %5ld Processed %5" PRIu64" nodes [%5ld/%.1f]\r", row, n_nodes, n_flow, mem * B_TO_MB);
	}
	GMT_Report (API, GMT_MSG_VERBOSE, "Row %5ld Processed %5" PRIu64 " nodes [%5ld/%.1f]\n", row, n_nodes, n_flow, mem * B_TO_MB);
	GMT_Report (API, GMT_MSG_VERBOSE, "On average, each node was visited %g times\n", n_more_than_once / n_unique_nodes);

	if (keep_flowlines && n_nodes != n_alloc) flowline = GMT_memory (GMT, flowline, n_nodes, struct FLOWLINE);
	
	/* OK, Done processing, time to write out */

	if (Ctrl->S.active) {	/* Convert CVA values to percent of CVA maximum */		
		GMT_Report (API, GMT_MSG_VERBOSE, "Normalize CVS grid to percentages of max CVA\n");
		normalize_grid (GMT, G, G->data);
	}
		
	GMT_Report (API, GMT_MSG_VERBOSE, "Write CVA grid %s\n", Ctrl->G.file);

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, G) != GMT_OK) {
		Return (API->error);
	}

	if (Ctrl->Z.mode) {	/* Do CVA calculations for each z-slice using stored flowlines */
		unsigned int layer, nz;
		size_t len;
		char file[GMT_BUFSIZ] = {""}, format[GMT_BUFSIZ] = {""};
		double z0, z1;
		float *CVA_inc = NULL, *old = G->data;
		
		GMT_Report (API, GMT_MSG_VERBOSE, "Start z-slice CVA calculations\n");
		for (len = strlen (Ctrl->G.file); len > 0 && Ctrl->G.file[len] != '.'; len--);
		if (Ctrl->G.file[len] == '.') {	/* Make a filename template from the CVA filename using the period as delimeter */
			strncpy (format, Ctrl->G.file, len);	/* Should keep the prefix from a file called prefix.ext */
			strcat (format, "_%%d");		/* Make filenames like prefix_#.ext */
			strcat (format, &Ctrl->G.file[len]);	/* Should add the extension from said file */
		}
		CVA_inc = GMT_memory (GMT, NULL, G->header->size, float);
		nz = urint ((Ctrl->Z.max - Ctrl->Z.min) / Ctrl->Z.inc);
		for (layer = 0; layer < nz; layer++) {
			z0 = Ctrl->Z.min + layer * Ctrl->Z.inc;
			z1 = z0 + Ctrl->Z.inc;
			GMT_Report (API, GMT_MSG_VERBOSE, "Start z-slice %g - %g\n", z0, z1);
			GMT_memset (CVA_inc, G->header->size, float);	/* Fresh start for this z-slice */
			for (m = 0; m < n_nodes; m++) {				/* Loop over all active flowlines */
				ij = flowline[m].ij;
				if (Z->data[ij] <= z0 || Z->data[ij] > z1) continue;	/* z outside current slice */
				row = (unsigned int)GMT_row (Z->header, ij);
				cva_contribution = lat_area[row] * (Ctrl->T.active[UPPER] ? Ctrl->T.t_fix : Z->data[ij]);	/* This node's contribution to the convolution */
				GMT_memset (processed_node, G->header->size, char);		/* Fresh start for this flowline convolution */
				for (k = 0; k < flowline[m].n; k++) {			/* For each point along this flowline */
					node = flowline[m].node[k];
					if (node != UINTMAX_MAX && !processed_node[node]) {	/* Have not added to the CVA at this node yet */
						CVA_inc[node] += (float)cva_contribution;
						processed_node[node] = true;		/* Now we have visited this node */
					}
				}
				if (!(m%10000)) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processed %5ld flowlines\r", m);
			}
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processed %5" PRIu64 " flowlines\n", n_nodes);
			
			/* Time to write out this z-slice grid */
			if (Ctrl->S.active) {	/* Convert CVA values to percent of CVA maximum */		
				GMT_Report (API, GMT_MSG_VERBOSE, "Normalize CVS grid to percentages of max CVA\n");
				normalize_grid (GMT, G, CVA_inc);
			}
			sprintf (G->header->remark, "CVA for z-range %g - %g only", z0, z1);
			sprintf (file, format, layer);
			G->data = CVA_inc;	/* Temporarily change the array pointer */
			GMT_Report (API, GMT_MSG_VERBOSE, "Save z-slice CVA to file %s\n", file);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) Return (API->error);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, file, G) != GMT_OK) {
				Return (API->error);
			}
		}
		G->data = old;	/* Reset the array pointer */
		GMT_free (GMT, CVA_inc);
	}
			
	if (Ctrl->D.active || Ctrl->PA.active) {	/* Must determine max CVA along each flowline */
		if (Ctrl->D.active) {
			if ((DI = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, Z)) == NULL) Return (API->error);
		}
		if (Ctrl->PA.active) {
			if ((PA = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, Z)) == NULL) Return (API->error);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Compute DI and/or PA grids\n");

		if (keep_flowlines) {
			for (m = 0; m < n_nodes; m++) {	/* Loop over all active flowlines */
				CVA_max = 0.0;						/* Fresh start for this flowline convolution */
				for (k = 0; k < flowline[m].n; k++) {			/* For each point along this flowline */
					node = flowline[m].node[k];
					if (node != UINTMAX_MAX && G->data[node] > CVA_max) {	/* Found a higher CVA value */
						CVA_max = G->data[node];
						if (Ctrl->PA.active) pa = flowline[m].PA[k];	/* Keep the estimate of age at highest CVA */
					}
				}
				if (Ctrl->D.active) DI->data[flowline[m].ij] = (float)CVA_max;	/* Store the maximum CVA associated with this node's flowline */
				if (Ctrl->PA.active) PA->data[flowline[m].ij] = (float) (pa * PA_2_T);
				if (!(m%10000)) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processed %5ld flowlines\r", m);
			}
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processed %5" PRIu64 " flowlines\n", n_nodes);
		}
		else {	/* Must recreate flowlines */
			k_step = 3;	/* FLowlines have (x,y,t) here */
			forth_flag = (Ctrl->PA.active) ? 11 : 10;	/* The 10 is used to limit the flowline calculation to only resample track within the rectangular box of interest */
			n_flow = n_nodes = 0;
			GMT_grd_loop (GMT, Z, row, col, ij) {	/* Loop over all input nodes */
				/* STEP 1: Determine if z exceeds threshold and if so assign age */
				if (GMT_is_fnan (Z->data[ij]) || Z->data[ij] < Ctrl->Z.min || Z->data[ij] > Ctrl->Z.max) continue;	/* Skip node since it is NaN or outside the Ctrl->Z.min < z < Ctrl->Z.max range */
				if (Ctrl->Q.mode && !ID_info[ID[ij]].ok) continue;				/* Skip because of wrong ID */
				if (!set_age (GMT, &t_smt, A, ij, Ctrl->N.t_upper, Ctrl->T.active[TRUNC])) continue;	/* Skip as age is outside our range */
				/* STEP 2: Calculate this node's flowline */
				if (Ctrl->Q.mode && ID_info[ID[ij]].check_region) /* Set up a box-limited flowline sampling */
					GMT_memcpy (this_wesn, ID_info[ID[ij]].wesn, 4, double);
				else
					GMT_memcpy (this_wesn, wesn, 4, double);
				np = get_flowline (GMT, x_smt[col], y_smt[row], t_smt, p, n_stages, sampling_int_in_km, k_step, forth_flag, this_wesn, &c);
				if (np == 0) continue;	/* No flowline inside this wesn */
		 		n_nodes++;
				/* Fresh start for this flowline convolution */
				CVA_max = 0.0;
				this_pa = GMT->session.d_NaN;
				for (m = 0, k = 1; m < np; m++) {	/* Store nearest node indices only */
					i = (int)GMT_grd_x_to_col (GMT, c[k++], G_rad->header);
					if (i < 0 || (col = i) >= G->header->nx) { k += 2; continue;}	/* Outside the CVA box, flag as outside */
					yg = GMT_lat_swap (GMT, R2D * c[k++], GMT_LATSWAP_O2G);		/* Convert back to geodetic */
					j = (int)GMT_grd_y_to_row (GMT, yg, G->header);
					if (j < 0 || (row = j) >= G->header->ny) { k++; continue;}	/* Outside the CVA box, flag as outside */
					if (Ctrl->PA.active) pa_val = c[k++];
					node = GMT_IJP (G->header, row, col);
					if (G->data[node] <= CVA_max) continue;	/* Already seen higher CVA values */
					CVA_max = G->data[node];
					if (Ctrl->PA.active) this_pa = pa_val;
				}
				if (Ctrl->D.active) DI->data[ij] = (float)CVA_max;	/* Store the maximum CVA associated with this node's flowline */
				if (Ctrl->PA.active) PA->data[ij] = (float) this_pa;
				GMT_free (GMT, c);
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Row %5ld: Processed %5" PRIu64 " flowlines\r", row, n_nodes);
			}
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Row %5ld: Processed %5" PRIu64 " flowlines\n", row, n_nodes);
		}
		

		if (Ctrl->D.active) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Write DI grid %s\n", Ctrl->D.file);
			sprintf (DI->header->remark, "CVA maxima along flowlines from each node");
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, DI)) Return (API->error);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->D.file, DI) != GMT_OK) {
				Return (API->error);
			}
		}
		if (Ctrl->PA.active) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Write PA grid %s\n", Ctrl->PA.file);
			sprintf (PA->header->remark, "Predicted age for each node");
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, PA)) Return (API->error);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->PA.file, PA) != GMT_OK) {
				Return (API->error);
			}
		}
	}
	
	if (Ctrl->W.active) {	/* Use bootstrapping to estimate confidence region for CVA maxima */

		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
			GMT_Message (API, GMT_TIME_NONE, "Preprocessed %5" PRIu64 " flowlines\n", n_nodes);
			GMT_Message (API, GMT_TIME_NONE, "%" PRIu64 " of %" PRIu64 " total flowlines entered CVA region\n", n_nodes, n_flow);
			GMT_Message (API, GMT_TIME_NONE, "Flowlines consumed %d Mb of memory\n", lrint (mem * B_TO_MB));
			GMT_Message (API, GMT_TIME_NONE, "Estimate %d CVA max locations using bootstrapping\n", Ctrl->W.n_try);
		}

		if ((error = GMT_set_cols (GMT, GMT_OUT, 3)) != GMT_OK) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}

		/* Now do bootstrap sampling of flowlines */
	
		try = 0;
		srand ((unsigned int)time(NULL));	/* Initialize random number generator */
		x_scale = (double)G->header->nx / (double)RAND_MAX;
		y_scale = (double)G->header->ny / (double)RAND_MAX;
		for (try = 1; try <= Ctrl->W.n_try; try++) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Bootstrap try %d\r", try);
		
			GMT_memset (G->data, G->header->size, float);	/* Start with fresh grid */
			for (m = 0; m < n_nodes; m++) {	/* Loop over all indices */
				row = urint (floor (rand() * y_scale));		/* Get a random integer in 0 to ny-1 range */
				col = urint (floor (rand() * x_scale));		/* Get a random integer in 0 to nx-1 range */
				ij = GMT_IJP (G->header, row, col);		/* Get the node index */
				GMT_memset (processed_node, G->header->size, char);		/* Fresh start for this flowline convolution */
				zz = Z->data[flowline[ij].ij];
				for (k = 0; k < flowline[ij].n; k++) {		/* For each point along this flowline */
					node = flowline[ij].node[k];
					if (node != UINTMAX_MAX && !processed_node[node]) {	/* Have not added to the CVA at this node yet */
						G->data[node] += zz;
						processed_node[node] = true;		/* Now we have visited this node; flag it */
					}
				}
				if (!(m%10000)) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processed %5ld flowlines\r", m);
			}
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processed %5" PRIu64 " flowlines\n", n_nodes);
		
			/* Find max CVA location */
		
			CVA_max = 0.0;
			for (ij = 0; ij < G->header->size; ij++) {		/* Loop over all CVA nodes */
				if (G->data[ij] > CVA_max) {	/* Update new max location */
					CVA_max = G->data[ij];
					max_ij = ij;
				}
			}
			col = (unsigned int)GMT_col (G->header, max_ij);
			row = (unsigned int)GMT_row (G->header, max_ij);
			out[0] = GMT_grd_col_to_x (GMT, col, G->header);
			out[1] = GMT_grd_row_to_y (GMT, row, G->header);
			out[2] = CVA_max;
			
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Bootstrap try %d\n", Ctrl->W.n_try);
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
			Return (API->error);
		}
	}
	
	/* Clean up memory */

	GMT_free (GMT, processed_node);
	for (m = 0; keep_flowlines && m < n_nodes; m++) {
		GMT_free (GMT, flowline[m].node);
		if (Ctrl->PA.active) GMT_free (GMT, flowline[m].PA);
	}
	GMT_free (GMT, p);
	GMT_free (GMT, x_smt);	GMT_free (GMT, y_smt);
	GMT_free (GMT, x_cva);	GMT_free (GMT, y_cva);
	GMT_free (GMT, lat_area);
	if (keep_flowlines) GMT_free (GMT, flowline);
	if (Ctrl->Q.mode) GMT_free (GMT, ID_info);
	if (GMT_Destroy_Data (API, &G_rad) != GMT_OK) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to free G_rad\n");
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "Done\n");

	Return (GMT_OK);
}
