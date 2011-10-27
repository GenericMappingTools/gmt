/*--------------------------------------------------------------------
 *	$Id$
 *
 *   Copyright (c) 1999-2011 by P. Wessel
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 or any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
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
 
#include "spotter.h"

#define B_TO_MB	(1.0 / 1048576.0)
#define OUTSIDE -1

#define TRUNC	0	/* Indices for -T */
#define UPPER	1

struct GRDSPOTTER_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct A {	/* -A<file> */
		GMT_LONG active;
		char *file;
	} A;
	struct D {	/* -Di<file> */
		GMT_LONG active;
		char *file;
	} D;
	struct E {	/* -E[+rotfile */
		GMT_LONG active;
		GMT_LONG mode;
		char *file;
	} E;
	struct G {	/* -Ggrdfile */
		GMT_LONG active;	/* Pixel registration */
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct L {	/* -Lfile */
		GMT_LONG active;
		char *file;
	} L;
	struct M {	/* -M */
		GMT_LONG active;
	} M;
	struct N {	/* -N */
		GMT_LONG active;
		double t_upper;
	} N;
	struct PA {	/* -Dp<file> */
		GMT_LONG active;
		char *file;
	} PA;
	struct Q {	/* -Q */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG id;
		char *file;
	} Q;
	struct S2 {	/* -S2 */
		GMT_LONG active;
		double dist;
	} S2;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct T {	/* -T */
		GMT_LONG active[2];
		double t_fix;	/* Set fixed age*/
	} T;
	struct W {	/* -W */
		GMT_LONG active;
		GMT_LONG n_try;
	} W;
	struct Z {	/* -Z */
		GMT_LONG active;
		GMT_LONG mode;
		double min, max, inc;
	} Z;
};

void *New_grdspotter_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDSPOTTER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDSPOTTER_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
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

GMT_LONG GMT_grdspotter_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdspotter %s - Create CVA image from a gravity or topography grid\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdspotter <ingrid> -E[+]<rottable> -G<CVAgrid> %s\n", GMT_I_OPT);
	GMT_message (GMT, "\t%s [-A<agegrid>] [-D[i|p]<grdfile>] [-L<IDgrid>]\n", GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[-M] [-N<upper_age>] [-Q<IDinfo>] [-S] [-Tt|-u<age>] [%s] [-W<n_try] [-Z<z_min>[/<z_max>[/<z_inc>]]] [%s]\n\n", GMT_V_OPT, GMT_r_OPT);
	
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);
	
	GMT_message (GMT, "\t<ingrid> is the grid with topo or gravity\n");
	GMT_message (GMT, "\t-E Specify the rotations table to be used (see man page for format).\n");
	GMT_message (GMT, "\t   Prepend + if you want to invert the finite rotations prior to use.\n");
	GMT_message (GMT, "\t-G Specify file name for output CVA convolution grid.\n");
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Co-registered grid with upper ages to use [Default is flowlines for all ages].\n");
	GMT_message (GMT, "\t-D Set optional output grids:\n");
	GMT_message (GMT, "\t   -Di<file> Use flowlines to estimate data importance DI grid.\n");
	GMT_message (GMT, "\t   -Dp<file> Use flowlines to estimate predicted ages at node locations.\n");
	GMT_message (GMT, "\t-L Co-registered grid with chain ID for each node [Default ignores IDs].\n");
	GMT_message (GMT, "\t-M Do flowline calculations as needed rather than storing in memory.\n");
	GMT_message (GMT, "\t   You may have to use this option if -R is too large. Cannot be used with -B or -Z-slicing.\n");
	GMT_message (GMT, "\t-N Set upper age in m.y. for nodes whose plate age is NaN [180].\n");
	GMT_message (GMT, "\t-Q Either single ID to use or file with list of IDs [Default uses all IDs].\n");
	GMT_message (GMT, "\t   Each line would be TAG ID [w e s n] with optional zoom box.\n");
	GMT_message (GMT, "\t-S Normalize CVA grid to percentages of the CVA maximum.\n");
	GMT_message (GMT, "\t-T Set upper ages.  Repeatable, choose from:\n");
	GMT_message (GMT, "\t  -Tt truncate all ages to max age in stage pole model [Default extrapolates].\n");
	GMT_message (GMT, "\t  -Tu<age> After a node passes the -Z test, use this fixed age instead in CVA calculations.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Get <n_try> bootstrap estimates of maximum CVA location [Default is no bootstrapping].\n");
	GMT_message (GMT, "\t-Z Ignore nodes with z-value lower than z_min [0] and optionally larger than z_max [Inf].\n");
	GMT_message (GMT, "\t   Give z_min/z_max/z_inc to make CVA grids for each z-slice {Default makes 1 CVA grid].\n");
	GMT_explain_options (GMT, "F");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdspotter_parse (struct GMTAPI_CTRL *C, struct GRDSPOTTER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdspotter and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG m, n_errors = 0, k, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Supplemental parameters */

			case 'A':
				Ctrl->A.active = TRUE;
				Ctrl->A.file = strdup (opt->arg);
				break;
#ifdef GMT_COMPAT
			case 'C':	/* Now done automatically in spotter_init */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -C is no longer needed as total reconstruction vs stage rotation is detected automatically.\n");
				break;
#endif
			case 'E':
				Ctrl->E.active = TRUE;	k = 0;
				if (opt->arg[0] == '+') { Ctrl->E.mode = TRUE; k = 1;}
				Ctrl->E.file  = strdup (&opt->arg[k]);
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'D':
				switch (opt->arg[0]) {
					case 'i':
						Ctrl->D.active = TRUE;
						Ctrl->D.file = strdup (&opt->arg[1]);
						break;
					case 'p':
						Ctrl->PA.active = TRUE;
						Ctrl->PA.file = strdup (&opt->arg[1]);
						break;
					default:
						n_errors++;
						break;
				}
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':
				Ctrl->L.active = TRUE;
				Ctrl->L.file = strdup (opt->arg);
				break;
			case 's':	/* Sampling interval */
				Ctrl->S2.active = TRUE;
				Ctrl->S2.dist = atof (opt->arg);
				break;
			case 'M':
				Ctrl->M.active = TRUE;
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				Ctrl->N.t_upper = atof (opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = TRUE;
				if (!access (opt->arg, R_OK)) {	/* The file exists */
					Ctrl->Q.mode = 2;
					Ctrl->Q.file = strdup (opt->arg);
				}
				else if ((Ctrl->Q.id = atoi (opt->arg)) > 0) {	/* Got OK id value */
					Ctrl->Q.mode = 1;
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Error -Q: Must give valid file or ID value\n");
					n_errors++;
				}
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				break;
			case 'T':
				if (opt->arg[0] == 't')
					Ctrl->T.active[TRUNC] = TRUE;
				else if (opt->arg[0] == 'u') {
					Ctrl->T.t_fix = atof (&opt->arg[1]);
					Ctrl->T.active[UPPER] = TRUE;
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Error -T: Either use -Tt or -Tu<age>\n");
					n_errors++;
				}
				break;
			case 'W':
				Ctrl->W.n_try = atoi (opt->arg);
				Ctrl->W.active = TRUE;
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				m = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->Z.min, &Ctrl->Z.max, &Ctrl->Z.inc);
				if (m == 1) Ctrl->Z.max = 1.0e300;	/* Max not specified */
				if (m == 3) Ctrl->Z.mode = TRUE;	/* Want several slices */
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->G.active || Ctrl->G.file), "Syntax error -G: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->In.active || Ctrl->In.file), "Syntax error -Z: Must give name of topo gridfile\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.file && !Ctrl->Q.mode, "Syntax error: Must specify both -L and -Q if one is present\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && (Ctrl->W.active || Ctrl->Z.mode), "Syntax error: Cannot use -M with -B or -Z (slicing)\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LONG get_flowline (struct GMT_CTRL *GMT, double xx, double yy, double tt, struct EULER *p, GMT_LONG n_stages, double d_km, GMT_LONG step, GMT_LONG flag, double wesn[], double **flow)
{
	GMT_LONG n_chunk, n_track, m, kx, ky, first, last, np;
	double *c = NULL, *f = NULL;

	/* Get the flowline from this point back to time tt, restricted to the given wesn box */
	n_chunk = spotter_forthtrack (GMT, &xx, &yy, &tt, (GMT_LONG)1, p, n_stages, d_km, 0.0, flag, wesn, &c);

	n_track = irint (c[0]);				/* Number of point pairs making up this flowline */

	/* Find the first point on the flowline inside the desired CVA region */

	for (m = 0, ky = 2, first = -1; m < n_track && first == -1; m++, ky += step) {	/* For each point along flowline */
		if (c[ky] < wesn[YLO] || c[ky] > wesn[YHI]) continue;	/* Latitude outside region */
		kx = ky - 1;						/* Index for the x-coordinate */
		while (c[kx] > wesn[XHI]) c[kx] -= TWO_PI;		/* Elaborate W/E test because of 360 periodicity */
		while (c[kx] < wesn[XLO]) c[kx] += TWO_PI;
		if (c[kx] > wesn[XHI]) continue;				/* Longitude outside region */
		first = kx;						/* We are inside, this terminates the for loop */
	}

	if (first == -1) { 	/* Was never inside the grid, skip the entire flowline and move on */
		GMT_free (GMT, c);	/* Free the flowline vector */
		return 0;
	}

	/* Here we know searching from the end will land inside the grid eventually so last can never exit as -1 */

	for (m = n_track - 1, ky = step * m + 2, last = -1; m >= 0 && last == -1; m--, ky -= step) {	/* For each point along flowline */
		if (c[ky] < wesn[YLO] || c[ky] > wesn[YHI]) continue;	/* Latitude outside region */
		kx = ky - 1;						/* Index for the x-coordinate */
		while (c[kx] > wesn[XHI]) c[kx] -= TWO_PI;		/* Elaborate W/E test because of 360 periodicity */
		while (c[kx] < wesn[XLO]) c[kx] += TWO_PI;
		if (c[kx] > wesn[XHI]) continue;				/* Longitude outside region */
		last = kx;						/* We are inside, this terminates the for loop */
	}

	np = (last - first) / step + 1;			/* Number of (x,y[,t]) points on this flowline inside the region */
	if (np < n_track) {	/* Just copy out the subset of points we want */
		GMT_LONG n_alloc;
		n_alloc = np * step;	/* Number of (x,y[,t]) to copy */
		f = GMT_memory (GMT, NULL, n_alloc+1, double);
		f[0] = (double)np;	/* Number of points found */
		memcpy (&f[1], &c[first], n_alloc * sizeof (double));
		GMT_free (GMT, c);	/* Free the old flowline vector */
		*flow = f;		/* Return pointer to trimmed flowline */
	}
	else
		*flow = c;		/* Return the entire flowline as is */
	return (np);
}

GMT_LONG set_age (struct GMT_CTRL *GMT, double *t_smt, struct GMT_GRID *A, GMT_LONG node, double upper_age, GMT_LONG truncate)
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
				GMT_report (GMT, GMT_MSG_NORMAL, "Node %ld has age (%g) > oldest stage (%g) (skipped)\n", node, *t_smt, upper_age);
				return (FALSE);
			}
		}
	}
	return (TRUE);	/* We are returning a useful age */
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdspotter_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_grdspotter (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG n_nodes;		/* Number of nodes processed */
	GMT_LONG n_stages;		/* Number of stage rotations (poles) */
	GMT_LONG node;			/* The current node index */
	GMT_LONG try;			/* Number of current bootstrap estimate */
	GMT_LONG n_alloc = 0, inc_alloc = BIG_CHUNK;
	GMT_LONG i, j, k, ij, m, row, col, k_step, np, max_ij = 0, n_flow, mem = 0, n_unique_nodes = 0;
	GMT_LONG error = FALSE;		/* TRUE when arguments are wrong */
	GMT_LONG keep_flowlines = FALSE;	/* TRUE if Ctrl->D.active, Ctrl->PA.active, or bootstrap is TRUE */
	GMT_LONG forth_flag;		/* Holds the do_time + 10 flag passed to forthtrack */
	GMT_LONG *ID = NULL;		/* Optional array with IDs for each node */
	char *processed_node = NULL;	/* Pointer to array with TRUE/FALSE values for each grid node */
	
	unsigned short pa = 0;		/* Placeholder for PA along track */

	float zz;

	double sampling_int_in_km;	/* Sampling interval along flowline (in km) */
	double *x_smt = NULL;		/* node longitude (input degrees, stored as radians) */
	double *y_smt = NULL;		/* node latitude (input degrees, stored as radians) */
	double t_smt = 0.0;		/* node upper age (up to age of seafloor) */
	double *c = NULL;		/* Array with one flowline */
	double CVA_max, wesn[4], cva_contribution, yg;
	double out[3], scale, area;
	double *lat_area = NULL;	/* Area of each dx by dy note in km as function of latitude */
	double CVA_scale;		/* Used to normalize CVAs to percent */
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
		GMT_LONG ok;		/* TRUE if we want to calculate this CVA */
		GMT_LONG check_region;	/* TRUE if w, e, s, n is more restrictive than command line -R */
	} *ID_info = NULL;
	struct GMT_OPTION *ptr = NULL;
	struct GRDSPOTTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_grdspotter_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_grdspotter_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdspotter", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRf:", "ghrs>" GMT_OPT("F"), options))) Return (error);
	if (GMT_Find_Option (API, 'f', options, &ptr)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	Ctrl = New_grdspotter_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdspotter_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdspotter main code ----------------------------*/

	/* Initialize the CVA grid and structure */

	G = GMT_Create_Data (API, GMT_IS_GRID, NULL);
	GMT_grd_init (GMT, G->header, options, FALSE);	/* Initialize grid structure */
	
	/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, G, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), Ctrl->G.file);
	G->data = GMT_memory (GMT, NULL, G->header->size, float);
	
	/* ------------------- END OF PROCESSING COMMAND LINE ARGUMENTS  --------------------------------------*/

	GMT_lat_swap_init (GMT);	/* Initialize auxiliary latitude machinery */

	/* Load in the Euler stage poles */

	n_stages = spotter_init (GMT, Ctrl->E.file, &p, TRUE, FALSE, Ctrl->E.mode, &Ctrl->N.t_upper);

	/* Assign grid-region variables in radians to avoid conversions inside convolution loop */

	G_rad = GMT_Create_Data (API, GMT_IS_GRID, NULL);
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
	GMT_report (GMT, GMT_MSG_NORMAL, "Flowline sampling interval = %.3f km\n", sampling_int_in_km);

	if (Ctrl->T.active[TRUNC]) GMT_report (GMT, GMT_MSG_NORMAL, "Ages truncated to %g\n", Ctrl->N.t_upper);

	/* Start to read input data */
	
	if ((error = GMT_Init_IO (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IN,  GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN,  GMT_BY_SET))) Return (error);				/* Enables data input and sets access mode */

	if ((Z = GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, Ctrl->In.file, NULL)) == NULL) Return (API->error);	/* Get data */
	area = 111.195 * Z->header->inc[GMT_Y] * 111.195 * Z->header->inc[GMT_X];	/* In km^2 at Equator */
	x_smt = GMT_memory (GMT, NULL, Z->header->nx, double);
	for (i = 0; i < Z->header->nx; i++) x_smt[i] = D2R * GMT_grd_col_to_x (GMT, i, Z->header);
	y_smt = GMT_memory (GMT, NULL, Z->header->ny, double);
	for (j = 0; j < Z->header->ny; j++) y_smt[j] = D2R * GMT_lat_swap (GMT, GMT_grd_row_to_y (GMT, j, Z->header), GMT_LATSWAP_G2O);	/* Convert to geocentric */
	lat_area = GMT_memory (GMT, NULL, Z->header->ny, double);

	for (j = 0; j < Z->header->ny; j++) lat_area[j] = area * cos (y_smt[j]);
	
	x_cva = GMT_memory (GMT, NULL, G->header->nx, double);
	for (i = 0; i < G->header->nx; i++) x_cva[i] = GMT_grd_col_to_x (GMT, i, G->header);
	y_cva = GMT_memory (GMT, NULL, G->header->ny, double);
	for (j = 0; j < G->header->ny; j++) y_cva[j] = GMT_grd_row_to_y (GMT, j, G->header);
	if (Ctrl->A.file) {
		if ((A = GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, Ctrl->A.file, NULL)) == NULL) Return (API->error);	/* Get header only */
		if (!(A->header->nx == Z->header->nx && A->header->ny == Z->header->ny && A->header->wesn[XLO] == Z->header->wesn[XLO] && A->header->wesn[YLO] == Z->header->wesn[YLO])) {
			GMT_report (GMT, GMT_MSG_FATAL, "Topo grid and age grid must coregister\n");
			Return (EXIT_FAILURE);
		}
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->A.file, A) == NULL) Return (API->error);	/* Get age data */
	}
	if (Ctrl->L.file) {
		if ((L = GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, Ctrl->L.file, NULL)) == NULL) Return (API->error);	/* Get header only */
		if (!(L->header->nx == Z->header->nx && L->header->ny == Z->header->ny && L->header->wesn[XLO] == Z->header->wesn[XLO] && L->header->wesn[YLO] == Z->header->wesn[YLO])) {
			GMT_report (GMT, GMT_MSG_FATAL, "Topo grid and ID grid must coregister\n");
			Return (EXIT_FAILURE);
		}
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, Ctrl->L.file, L) == NULL) Return (API->error);	/* Get ID data */
		
		/* Store IDs in a GMT_LONG array instead */
		ID = GMT_memory (GMT, NULL, L->header->size, GMT_LONG);
		for (i = 0; i < L->header->size; i++) ID[i] = (GMT_LONG) rint ((double)L->data[i]);
		GMT_free (GMT, L->data);	/* Just free the array since we use ID; Grid stuct is destroyed at end */
		
		ID_info = GMT_memory (GMT, NULL, rint (L->header->z_max) + 1, struct ID);
		if (Ctrl->Q.mode == 1) {	/* Only doing one CVA with no extra restrictions */
			ID_info[Ctrl->Q.id].ok = TRUE;	/* Every other info in struct array is NULL or 0 */
		}
		else {	/* Must read from file */
			FILE *fp = NULL;
			GMT_LONG Qid;
			double wq, eq, sq, nq;
			char line[GMT_BUFSIZ];
			
			if ((fp = fopen (Ctrl->Q.file, "r")) == NULL) {	/* Oh, oh... */
				GMT_report (GMT, GMT_MSG_FATAL, "Error: -Q info file unreadable/nonexistent\n");
				exit (EXIT_FAILURE);
			}
			while (fgets (line, GMT_BUFSIZ, fp)) {
				if (line[0] == '#' || line[0] == '\n') continue;
				k = sscanf (line, "%*s %ld %lf %lf %lf %lf", &Qid, &wq, &eq, &sq, &nq);
				ID_info[Ctrl->Q.id].ok = TRUE;
				if (k == 5) {	/* Got restricted wesn also */
					ID_info[Qid].check_region = TRUE;
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
		Ctrl->Q.mode = TRUE;
	}

	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (Ctrl->M.active) {
		keep_flowlines = FALSE;	/* Do it the hard way to save memory */
		k_step = 2;		/* Reset back to 3 if needed later */
		forth_flag = 10;	/* The 10 is used to limit the flowline calculation to only resample track within the rectangular box of interest */
	}
	else {
		keep_flowlines = (Ctrl->PA.active || Ctrl->D.active || Ctrl->W.active);
		forth_flag = Ctrl->PA.active + 10;	/* The 10 is used to limit the flowline calculation to only resample track within the rectangular box of interest */
		k_step = (Ctrl->PA.active) ? 3 : 2;
	}
	if (keep_flowlines) {
		n_alloc = inc_alloc;
		flowline = GMT_memory (GMT, NULL, n_alloc, struct FLOWLINE);
		if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
			GMT_message (GMT, "Will attempt to keep all flowlines in memory.  However, should this not be possible\n");
			GMT_message (GMT, "the program might crash.  If so consider using the -M option\n");
		}
	}
	
	n_flow = n_nodes = 0;
	GMT_grd_loop (GMT, Z, row, col, ij) {	/* Loop over all input nodes */
		/* STEP 1: Determine if z exceeds threshold and if so assign age */
		if (GMT_is_fnan (Z->data[ij]) || Z->data[ij] < Ctrl->Z.min || Z->data[ij] > Ctrl->Z.max) continue;	/* Skip node since it is NaN or outside the Ctrl->Z.min < z < Ctrl->Z.max range */
		if (Ctrl->Q.mode && !ID_info[(GMT_LONG)ID[ij]].ok) continue;			/* Skip because of wrong ID */
		if (!set_age (GMT, &t_smt, A, ij, Ctrl->N.t_upper, Ctrl->T.active[TRUNC])) continue;
		/* STEP 2: Calculate this node's flowline */
		if (Ctrl->Q.mode && ID_info[(GMT_LONG)ID[ij]].check_region) /* Set up a box-limited flowline sampling */
			GMT_memcpy (this_wesn, ID_info[(GMT_LONG)ID[ij]].wesn, 4, double);
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
			flowline[n_nodes].node = GMT_memory (GMT, NULL, np, GMT_LONG);
			if (Ctrl->PA.active) flowline[n_nodes].PA = GMT_memory (GMT, NULL, np, unsigned short);
		}
		
		/* Keep in mind that although first and last are entry/exit into the CVA grid, we must
		 * expect the flowline between those points to also exit/reentry the CVA grid; hence
		 * we must still check if points are in/out of the box.  Here, we do not skip points but
		 * set the node index to OUTSIDE */
		 
		cva_contribution = lat_area[row] * (Ctrl->T.active[UPPER] ? Ctrl->T.t_fix : Z->data[ij]);	/* This node's contribution to the convolution */

#ifdef DEBUG2
		printf ("> %ld %ld %ld %ld %g\n", n_nodes, np, row, col, cva_contribution);
#endif
		for (m = 0, k = 1; m < np; m++) {	/* Store nearest node indices only */
			i = GMT_grd_x_to_col (GMT, c[k++], G_rad->header);
			yg = GMT_lat_swap (GMT, R2D * c[k++], GMT_LATSWAP_O2G);		/* Convert back to geodetic */
			j = GMT_grd_y_to_row (GMT, yg, G->header);
			if (i < 0 || i >= G->header->nx || j < 0 || j >= G->header->ny)	/* Outside the CVA box, flag as outside */
				node = OUTSIDE;
			else								/* Inside the CVA box, assign node ij */
				node = GMT_IJP (G->header, j, i);
			if (keep_flowlines) {
				flowline[n_nodes].node[m] = node;
				if (Ctrl->PA.active) flowline[n_nodes].PA[m] = (unsigned short) irint (c[k++] * T_2_PA);
			}
			/* If we did not keep flowlines then there is no PA to skip over (hence no k++) */
			if (node != OUTSIDE) {
				if (!processed_node[node]) {	/* Have not added to the CVA at this node yet */
					G->data[node] += (float)cva_contribution;
					processed_node[node] = TRUE;		/* Now we have visited this node */
					n_unique_nodes++;
#ifdef DEBUG2
					printf ("%g\t%g\n", x_cva[i],y_cva[j]);
#endif
				}
				n_more_than_once += 1.0;
			}
		}
		if (keep_flowlines) {
			flowline[n_nodes].n = np;	/* Number of points in flowline */
			flowline[n_nodes].ij = ij;	/* Originating node in topo grid */
			mem += sizeof (struct FLOWLINE) + np * sizeof (GMT_LONG) + ((Ctrl->PA.active) ? np * sizeof (unsigned short) : 0);
		}

		GMT_free (GMT, c);	/* Free the flowline vector */

		n_nodes++;	/* Go to next node */

		if (keep_flowlines && n_nodes == n_alloc) {
			inc_alloc *= 2;
			n_alloc += inc_alloc;
			flowline = GMT_memory (GMT, flowline, n_alloc, struct FLOWLINE);
		}
		
		if (!(n_nodes%100)) GMT_report (GMT, GMT_MSG_NORMAL, "Row %5ld Processed %5ld nodes [%5ld/%.1f]\r", row, n_nodes, n_flow, mem * B_TO_MB);
	}
	GMT_report (GMT, GMT_MSG_NORMAL, "Row %5ld Processed %5ld nodes [%5ld/%.1f]\n", row, n_nodes, n_flow, mem * B_TO_MB);
	GMT_report (GMT, GMT_MSG_NORMAL, "On average, each node was visited %g times\n", n_more_than_once / n_unique_nodes);

	if (keep_flowlines && n_nodes != (GMT_LONG)n_alloc) flowline = GMT_memory (GMT, flowline, n_nodes, struct FLOWLINE);
	
	/* OK, Done processing, time to write out */

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */

	if (Ctrl->S.active) {	/* Convert CVA values to percent of CVA maximum */		
		GMT_report (GMT, GMT_MSG_NORMAL, "Normalize CVS grid to percentages of max CVA\n");
		G->header->z_min = +DBL_MAX;
		G->header->z_max = -DBL_MAX;
		GMT_grd_loop (GMT, G, row, col, node) {	/* Loop over all output nodes */
			if (GMT_is_fnan (G->data[node])) continue;
			if (G->data[node] < G->header->z_min) G->header->z_min = G->data[node];
			if (G->data[node] > G->header->z_max) G->header->z_max = G->data[node];
		}
		GMT_report (GMT, GMT_MSG_NORMAL, "CVA min/max: %g %g -> ", G->header->z_min, G->header->z_max);
		CVA_scale = 100.0 / G->header->z_max;
		for (node = 0; node < G->header->size; node++) G->data[node] *= (float)CVA_scale;
		G->header->z_min *= CVA_scale;
		G->header->z_max *= CVA_scale;
		GMT_report (GMT, GMT_MSG_NORMAL, "%g %g\n", G->header->z_min, G->header->z_max);
	}
		
	GMT_report (GMT, GMT_MSG_NORMAL, "Write CVA grid %s\n", Ctrl->G.file);

	if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, Ctrl->G.file, G)) Return (GMT_DATA_WRITE_ERROR);

	if (Ctrl->Z.mode) {	/* Do CVA calculations for each z-slice using stored flowlines */
		GMT_LONG nz;
		char file[256], format[256];
		double z0, z1;
		float *CVA_inc = NULL, *old = G->data;
		
		GMT_report (GMT, GMT_MSG_NORMAL, "Start z-slice CVA calculations\n");
		for (i = strlen (Ctrl->G.file); i >= 0 && Ctrl->G.file[i] != '.'; i--);
		if (Ctrl->G.file[i] == '.') {	/* Make a filename template from the CVA filename using the period as delimeter */
			strncpy (format, Ctrl->G.file, (size_t)i);	/* Should keep the prefix from a file called prefix.ext */
			strcat (format, "_%%ld");			/* Make filenames like prefix_#.ext */
			strcat (format, &Ctrl->G.file[i]);		/* Should add the extension from said file */
		}
		CVA_inc = GMT_memory (GMT, NULL, G->header->size, float);
		nz = irint ((Ctrl->Z.max - Ctrl->Z.min) / Ctrl->Z.inc);
		for (i = 0; i < nz; i++) {
			z0 = Ctrl->Z.min + i * Ctrl->Z.inc;
			z1 = z0 + Ctrl->Z.inc;
			GMT_report (GMT, GMT_MSG_NORMAL, "Start z-slice %g - %g\n", z0, z1);
			GMT_memset (CVA_inc, G->header->size, float);	/* Fresh start for this z-slice */
			for (m = 0; m < n_nodes; m++) {				/* Loop over all active flowlines */
				ij = flowline[m].ij;
				if (Z->data[ij] <= z0 || Z->data[ij] > z1) continue;	/* z outside current slice */
				cva_contribution = lat_area[ij/Z->header->nx] * (Ctrl->T.active[UPPER] ? Ctrl->T.t_fix : Z->data[ij]);	/* This node's contribution to the convolution */
				GMT_memset (processed_node, G->header->size, char);		/* Fresh start for this flowline convolution */
				for (k = 0; k < flowline[m].n; k++) {			/* For each point along this flowline */
					node = flowline[m].node[k];
					if (node != OUTSIDE && !processed_node[node]) {	/* Have not added to the CVA at this node yet */
						CVA_inc[node] += (float)cva_contribution;
						processed_node[node] = TRUE;		/* Now we have visited this node */
					}
				}
				if (!(m%10000)) GMT_report (GMT, GMT_MSG_VERBOSE, "Processed %5ld flowlines\r", m);
			}
			GMT_report (GMT, GMT_MSG_VERBOSE, "Processed %5ld flowlines\n", n_nodes);
			
			/* Time to write out this z-slice grid */
			if (Ctrl->S.active) {	/* Convert CVA values to percent of CVA maximum */		
				GMT_report (GMT, GMT_MSG_NORMAL, "Normalize CVS grid to percentages of max CVA\n");
				G->header->z_min = +DBL_MAX;
				G->header->z_max = -DBL_MAX;
				GMT_grd_loop (GMT, G, row, col, node) {	/* Loop over all output nodes */
					if (GMT_is_fnan (CVA_inc[node])) continue;
					if (CVA_inc[node] < G->header->z_min) G->header->z_min = CVA_inc[node];
					if (CVA_inc[node] > G->header->z_max) G->header->z_max = CVA_inc[node];
				}
				GMT_report (GMT, GMT_MSG_NORMAL, "CVA min/max: %g %g -> ", G->header->z_min, G->header->z_max);
				CVA_scale = 100.0 / G->header->z_max;
				for (node = 0; node < G->header->size; node++) CVA_inc[node] *= (float)CVA_scale;
				G->header->z_min *= CVA_scale;
				G->header->z_max *= CVA_scale;
				GMT_report (GMT, GMT_MSG_NORMAL, "%g %g\n", G->header->z_min, G->header->z_max);
			}
			sprintf (G->header->remark, "CVA for z-range %g - %g only", z0, z1);
			sprintf (file, format, i);
			G->data = CVA_inc;	/* Temporarily change the array pointer */
			GMT_report (GMT, GMT_MSG_NORMAL, "Save z-slice CVA to file %s\n", file);
			if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, file, G)) Return (GMT_DATA_WRITE_ERROR);
		}
		G->data = old;	/* Reset the array pointer */
		GMT_free (GMT, CVA_inc);
	}
			
	if (Ctrl->D.active || Ctrl->PA.active) {	/* Must determine max CVA along each flowline */
		if (Ctrl->D.active) {
			DI = GMT_Create_Data (API, GMT_IS_GRID, NULL);
			GMT_grd_init (GMT, DI->header, options, FALSE);

			/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
			GMT_err_fail (GMT, GMT_init_newgrid (GMT, DI, Z->header->wesn, Z->header->inc, Z->header->registration), Ctrl->D.file);
			DI->data = GMT_memory (GMT, NULL, PA->header->size, float);
		}
		if (Ctrl->PA.active) {
			PA = GMT_Create_Data (API, GMT_IS_GRID, NULL);
			GMT_grd_init (GMT, PA->header, options, FALSE);

			/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
			GMT_err_fail (GMT, GMT_init_newgrid (GMT, PA, Z->header->wesn, Z->header->inc, Z->header->registration), Ctrl->PA.file);
			PA->data = GMT_memory (GMT, NULL, PA->header->size, float);
		}
		GMT_report (GMT, GMT_MSG_NORMAL, "Compute DI and/or PA grids\n");

		if (keep_flowlines) {
			for (m = 0; m < n_nodes; m++) {	/* Loop over all active flowlines */
				CVA_max = 0.0;						/* Fresh start for this flowline convolution */
				for (k = 0; k < flowline[m].n; k++) {			/* For each point along this flowline */
					node = flowline[m].node[k];
					if (node != OUTSIDE && G->data[node] > CVA_max) {	/* Found a higher CVA value */
						CVA_max = G->data[node];
						if (Ctrl->PA.active) pa = flowline[m].PA[k];	/* Keep the estimate of age at highest CVA */
					}
				}
				if (Ctrl->D.active) DI->data[flowline[m].ij] = (float)CVA_max;	/* Store the maximum CVA associated with this node's flowline */
				if (Ctrl->PA.active) PA->data[flowline[m].ij] = (float) (pa * PA_2_T);
				if (!(m%10000)) GMT_report (GMT, GMT_MSG_VERBOSE, "Processed %5ld flowlines\r", m);
			}
			GMT_report (GMT, GMT_MSG_VERBOSE, "Processed %5ld flowlines\n", n_nodes);
		}
		else {	/* Must recreate flowlines */
			k_step = 3;	/* FLowlines have (x,y,t) here */
			forth_flag = Ctrl->PA.active + 10;	/* The 10 is used to limit the flowline calculation to only resample track within the rectangular box of interest */
			n_flow = n_nodes = 0;
			GMT_grd_loop (GMT, Z, row, col, ij) {	/* Loop over all input nodes */
				/* STEP 1: Determine if z exceeds threshold and if so assign age */
				if (GMT_is_fnan (Z->data[ij]) || Z->data[ij] < Ctrl->Z.min || Z->data[ij] > Ctrl->Z.max) continue;	/* Skip node since it is NaN or outside the Ctrl->Z.min < z < Ctrl->Z.max range */
				if (Ctrl->Q.mode && !ID_info[(GMT_LONG)ID[ij]].ok) continue;			/* Skip because of wrong ID */
				if (!set_age (GMT, &t_smt, A, ij, Ctrl->N.t_upper, Ctrl->T.active[TRUNC])) continue;
				/* STEP 2: Calculate this node's flowline */
				if (Ctrl->Q.mode && ID_info[(GMT_LONG)ID[ij]].check_region) /* Set up a box-limited flowline sampling */
					GMT_memcpy (this_wesn, ID_info[(GMT_LONG)ID[ij]].wesn, 4, double);
				else
					GMT_memcpy (this_wesn, wesn, 4, double);
				np = get_flowline (GMT, x_smt[col], y_smt[row], t_smt, p, n_stages, sampling_int_in_km, k_step, forth_flag, this_wesn, &c);
				if (np == 0) continue;	/* No flowline inside this wesn */
		 		n_nodes++;
				/* Fresh start for this flowline convolution */
				CVA_max = 0.0;
				this_pa = GMT->session.d_NaN;
				for (m = 0, k = 1; m < np; m++) {	/* Store nearest node indices only */
					i = GMT_grd_x_to_col (GMT, c[k++], G_rad->header);
					yg = GMT_lat_swap (GMT, R2D * c[k++], GMT_LATSWAP_O2G);		/* Convert back to geodetic */
					j = GMT_grd_y_to_row (GMT, yg, G->header);
					if (Ctrl->PA.active) pa_val = c[k++];
					if (i < 0 || i >= G->header->nx || j < 0 || j >= G->header->ny) continue;	/* Outside the CVA box, flag as outside */
					node = GMT_IJP (G->header, j, i);
					if (G->data[node] <= CVA_max) continue;	/* Already seen higher CVA values */
					CVA_max = G->data[node];
					if (Ctrl->PA.active) this_pa = pa_val;
				}
				if (Ctrl->D.active) DI->data[ij] = (float)CVA_max;	/* Store the maximum CVA associated with this node's flowline */
				if (Ctrl->PA.active) PA->data[ij] = (float) this_pa;
				GMT_free (GMT, c);
				GMT_report (GMT, GMT_MSG_VERBOSE, "Row %5ld: Processed %5ld flowlines\r", row, n_nodes);
			}
			GMT_report (GMT, GMT_MSG_VERBOSE, "Row %5ld: Processed %5ld flowlines\n", row, n_nodes);
		}
		

		if (Ctrl->D.active) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Write DI grid %s\n", Ctrl->D.file);
			sprintf (DI->header->remark, "CVA maxima along flowlines from each node");
			if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, Ctrl->D.file, DI)) Return (GMT_DATA_WRITE_ERROR);
		}
		if (Ctrl->PA.active) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Write PA grid %s\n", Ctrl->PA.file);
			sprintf (PA->header->remark, "Predicted age for each node");
			if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, Ctrl->PA.file, PA)) Return (GMT_DATA_WRITE_ERROR);
		}
	}
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */
	
	if (Ctrl->W.active) {	/* Use bootstrapping to estimate confidence region for CVA maxima */

		if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
			GMT_message (GMT, "Preprocessed %5ld flowlines\n", n_nodes);
			GMT_message (GMT, "%ld of %ld total flowlines entered CVA region\n", n_nodes, n_flow);
			GMT_message (GMT, "Flowlines consumed %ld Mb of memory\n", (GMT_LONG)irint (mem * B_TO_MB));
			GMT_message (GMT, "Estimate %ld CVA max locations using bootstrapping\n", Ctrl->W.n_try);
		}

		if ((error = GMT_set_cols (GMT, GMT_OUT, 3))) Return (error);
		if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default output destination, unless already set */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);				/* Enables data output and sets access mode */

		/* Now do bootstrap sampling of flowlines */
	
		try = 0;
		srand ((unsigned int)time(NULL));	/* Initialize random number generator */
		scale = (double)n_nodes / (double)RAND_MAX;
		for (try = 1; try <= Ctrl->W.n_try; try++) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Bootstrap try %ld\r", try);
		
			GMT_memset (G->data, G->header->size, float);	/* Start with fresh grid */
			for (m = 0; m < n_nodes; m++) {	/* Loop over all indices */
				ij = (GMT_LONG)floor (rand() * scale);		/* Get a random integer in 0 to n_nodes-1 range */
				GMT_memset (processed_node, G->header->size, char);		/* Fresh start for this flowline convolution */
				zz = Z->data[flowline[ij].ij];
				for (k = 0; k < flowline[ij].n; k++) {		/* For each point along this flowline */
					node = flowline[ij].node[k];
					if (node != OUTSIDE && !processed_node[node]) {	/* Have not added to the CVA at this node yet */
						G->data[node] += zz;
						processed_node[node] = TRUE;		/* Now we have visited this node; flag it */
					}
				}
				if (!(m%10000)) GMT_report (GMT, GMT_MSG_VERBOSE, "Processed %5ld flowlines\r", m);
			}
			GMT_report (GMT, GMT_MSG_VERBOSE, "Processed %5ld flowlines\n", n_nodes);
		
			/* Find max CVA location */
		
			CVA_max = 0.0;
			for (ij = 0; ij < G->header->size; ij++) {		/* Loop over all CVA nodes */
				if (G->data[ij] > CVA_max) {	/* Update new max location */
					CVA_max = G->data[ij];
					max_ij = ij;
				}
			}
			col = GMT_col (G->header, max_ij);
			row = GMT_row (G->header, max_ij);
			out[0] = GMT_grd_col_to_x (GMT, col, G->header);
			out[1] = GMT_grd_row_to_y (GMT, row, G->header);
			out[2] = CVA_max;
			
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
		}
		GMT_report (GMT, GMT_MSG_NORMAL, "Bootstrap try %ld\n", Ctrl->W.n_try);
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */
	}
	
	/* Clean up memory */

	GMT_free (GMT, processed_node);
	for (i = 0; keep_flowlines && i < n_nodes; i++) {
		GMT_free (GMT, flowline[i].node);
		if (Ctrl->PA.active) GMT_free (GMT, flowline[i].PA);
	}
	GMT_free (GMT, p);
	GMT_free (GMT, x_smt);	GMT_free (GMT, y_smt);
	GMT_free (GMT, x_cva);	GMT_free (GMT, y_cva);
	GMT_free (GMT, lat_area);
	if (keep_flowlines) GMT_free (GMT, flowline);
	if (Ctrl->Q.mode) GMT_free (GMT, ID_info);
	GMT_free_grid (GMT, &G_rad, FALSE);

	GMT_report (GMT, GMT_MSG_NORMAL, "Done\n");

	Return (GMT_OK);
}
