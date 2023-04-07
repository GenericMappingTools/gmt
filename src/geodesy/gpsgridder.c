/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Authors:	Paul Wessel & David Sandwell
 * Date:	1-OCT-2016
 * Version:	6 API
 *
 * Brief synopsis: gpsgridder grids GPS vector strain data u(x,y) & v(x,y) using
 * Green's functions derived from a thin elastic sheet [e.g., Haines et al., 2015].
 * See Sandwell and Wessel, 2016, "Interpolation of 2-D Vector Data Using Constraints
 *   from Elasticity", Geophys. Res. Lett., 43, 10,703-710,709, doi:10.1002/2016GL070340,
 *   for details.
 *
 * Note on KEYS: CD)=f means -C takes an optional output Dataset as argument via the +f modifier.
 */

#include "gmt_dev.h"
#include "longopt/gpsgridder_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"gpsgridder"
#define THIS_MODULE_MODERN_NAME	"gpsgridder"
#define THIS_MODULE_LIB		"geodesy"
#define THIS_MODULE_PURPOSE	"Interpolate GPS velocities using Green's functions for elastic deformation"
#define THIS_MODULE_KEYS	"<D{,ND(,TG(,CD)=f,GG}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:>RVbdefghinoqrs" GMT_ADD_x_OPT

/* Control structure for gpsgridder */

struct GPSGRIDDER_CTRL {
	struct GPSGRIDDER_C {	/* -C[[n|r|v]<cutoff>[%]][+c][+f<file>][+i][+n] */
		bool active;
		bool dryrun;	/* Only report eigenvalues */
		unsigned int history;	
		unsigned int mode;
		double value;
		char *file;
	} C;
	struct GPSGRIDDER_E {	/* -E[<file>] */
		bool active;
		unsigned int mode;
		char *file;
	} E;
	struct GPSGRIDDER_F {	/* -F<fudgefactor> or -Fa<mindist> */
		bool active;
		unsigned int mode;
		double fudge;
	} F;
	struct GPSGRIDDER_G {	/* -G<output_grdfile_template_or_tablefile> */
		bool active;
		char *file;
	} G;
	struct GPSGRIDDER_I {	/* -I (for checking only) */
		bool active;
	} I;
	struct GPSGRIDDER_L {	/* -L */
		bool active;
	} L;
	struct GPSGRIDDER_N {	/* -N<outputnode_file> */
		bool active;
		char *file;
	} N;
	struct GPSGRIDDER_S {	/* -S<nu> */
		bool active;
		double nu;
	} S;
	struct GPSGRIDDER_T {	/* -T<mask_grdfile> */
		bool active;
		char *file;
	} T;
	struct GPSGRIDDER_W {	/* -W[+s|w] */
		bool active;
		unsigned int mode;	/* 0 = got sigmas, 1 = got weights */
	} W;
	struct GPSGRIDDER_Z {	/* -Z undocumented debugging option */
		bool active;
	} Z;
};

enum Gpsgridded_enum {	/* Indices for coeff array for normalization */
	GPSGRIDDER_MEAN_X		= 0,
	GPSGRIDDER_MEAN_Y		= 1,
	GPSGRIDDER_MEAN_U		= 2,
	GPSGRIDDER_MEAN_V		= 3,
	GPSGRIDDER_SLP_UX		= 4,
	GPSGRIDDER_SLP_UY		= 5,
	GPSGRIDDER_SLP_VX		= 6,
	GPSGRIDDER_SLP_VY		= 7,
	GPSGRIDDER_RANGE_U		= 8,
	GPSGRIDDER_RANGE_V		= 9,
	GPSGRIDDER_LENGTH		= 10,
	GPSGRIDDER_U			= 2,	/* Index into input/output rows */
	GPSGRIDDER_V			= 3,
	GPSGRIDDER_WU			= 2,	/* Index into X row vector with x,y[,du,dv] */
	GPSGRIDDER_WV			= 3,
	GPSGRIDDER_TREND		= 1,	/* Remove/Restore linear trend */
	GPSGRIDDER_NORM			= 2,	/* Normalize residual data to 0-1 range */
	GPSGRIDDER_FUNC_Q		= 0,	/* Next 3 are indices into G[] */
	GPSGRIDDER_FUNC_P		= 1,	/* Next 3 are indices into G[] */
	GPSGRIDDER_FUNC_W		= 2,	/* Next 3 are indices into G[] */
	GPSGRIDDER_FUDGE_R		= 1,	/* Modes for -F */
	GPSGRIDDER_FUDGE_F		= 2,
	GPSGRIDDER_GOT_SIG		= 0,	/* Modes for -W */
	GPSGRIDDER_GOT_W		= 1,
	GPSGRIDDER_MISFIT		= 1		/* Modes for -E */
};

GMT_LOCAL void gpsgridder_set_filename (char *name, unsigned int k, unsigned int width, unsigned int mode, unsigned int comp, char *file) {
	/* Turn name, eigenvalue number k, precision width, mode and comp into a filename, e.g.,
	 * ("solution.grd", 33, 3, GMT_SVD_INCREMENTAL, 1, file) will give solution_v_inc_033.grd */
	unsigned int s = (unsigned int)strlen (name) - 1;
	static char *type[3] = {"", "inc", "cum"}, *uv[2] = {"u", "v"};
	char tmp[GMT_LEN256] = {""};
	while (name[s] != '.') s--;	/* Wind backwards to start of extension */
	name[s] = '\0';	/* Temporarily chop off extension */
	if (strchr (name, '%'))
		sprintf (tmp, name, uv[comp]);	/* Place the u or v in a name with a format */
	else
		sprintf (tmp, "%s_%s", name, uv[comp]);	/* Append the _u or _v to the name */
	if (width)
		sprintf (file, "%s_%s_%*.*d.%s", tmp, type[mode], width, width, k, &name[s+1]);
	else
		sprintf (file, "%s.%s", tmp, &name[s+1]);
	name[s] = '.';	/* Restore original name */
}

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GPSGRIDDER_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GPSGRIDDER_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.mode = GMT_SVD_EIGEN_RATIO_CUTOFF;
	C->S.nu = 0.5;	/* Poisson's ratio */
	C->F.fudge = 0.01;	/* Default fudge scale for r_min */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GPSGRIDDER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->N.file);
	gmt_M_str_free (C->T.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] -G<outfile> [-C[[n|r|v]<val>[%%]][+c][+f<file>][+i][+n]] [-E<misfitfile>] [-Fd|f<val>] [%s] "
		"[-L] [-N<nodefile>] [%s] [-S<nu>] [-T<maskgrid>] [%s] [-W[+s|w]] [%s] [%s] [%s] [%s] "
		"[%s] [%s] [%s] [%s] [%s] [%s] [%s]%s[%s] [%s]\n", name, GMT_I_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_d_OPT, GMT_e_OPT,
		GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_n_OPT, GMT_o_OPT, GMT_qi_OPT, GMT_r_OPT, GMT_s_OPT, GMT_x_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Usage (API, 1, "Choose one of three ways to specify where to evaluate the spline:");
	GMT_Usage (API, 2, "%s Specify a rectangular grid domain with options -R, -I [and optionally -r].", GMT_LINE_BULLET);
	GMT_Usage (API, 2, "%s Supply a mask file via -T whose values are NaN or 0.  The spline will then "
		"only be evaluated at the nodes originally set to zero.", GMT_LINE_BULLET);
	GMT_Usage (API, 2, "%s Specify a set of output locations via the -N option.", GMT_LINE_BULLET);

	GMT_Message (API, GMT_TIME_NONE, "\n  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<table>");
	GMT_Usage (API, -2, "<table> is one or more data files (in ASCII, binary, netCDF). "
		"If no files are given, standard input is read. "
		"The data must contain x y u v [weight_u weight_v] records. "
		"Specify -fg to convert longitude, latitude to Flat Earth coordinates.");
	gmt_outgrid_syntax (API, 'G', "Give name of output table (if -N) or a grid (we automatically insert _u and _v before extension.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");

	GMT_Usage (API, 1, "\n-C[[n|r|v]<val>[%%]][+c][+f<file>][+i][+n]");
	GMT_Usage (API, -2, "Solve by SVD and control how many eigenvalues to use. Optionally append a directive and value:");
	GMT_Usage (API, 3, "n: Only use the largest <val> eigenvalues [all].");
	GMT_Usage (API, 3, "r: Eliminate eigenvalues whose ratio to largest eigenvalue is less than <val> [0].");
	GMT_Usage (API, 3, "v: Include eigenvalues needed to reach a variance explained fraction of <val> [1].");
	GMT_Usage (API, -2, "Note: For r|v you may append %% to give <val> as the percentage of total instead. Various optional modifiers are available:");
	GMT_Usage (API, 3, "+c Create a series of intermediate grids for each eigenvalue holding the cumulative result. "
		"Requires -G with a valid filename and extension and we will insert _cum_### before the extension.");
	GMT_Usage (API, 3, "+f Save the eigenvalues to <filename>.");
	GMT_Usage (API, 3, "+i As +c but save incremental results, inserting _inc_### before the extension.");
	GMT_Usage (API, 3, "+n Stop execution after reporting the eigenvalues - no solution is computed.");
	GMT_Usage (API, -2, "Note: ~25%% of the total number of data constraints is a good starting point. "
		"Without -C we use Gauss-Jordan elimination to solve the linear system.");
	GMT_Usage (API, 1, "\n-E[<misfitfile>]");
	GMT_Usage (API, -2, "Evaluate spline at input locations and report statistics on the misfit. "
		"If <misfitfile> is given then we write individual location misfits to that file.");
	GMT_Usage (API, 1, "\n-Fd|f<val>");
	GMT_Usage (API, -2, "Fudging factor to avoid Green-function singularities. Choose among two directives:");
	GMT_Usage (API, 3, "d: Append <del_radius> to add to all distances between nodes and points "
		"(For geographical specify <del_radius> in km. A value of 8 km is optimal for California.).");
	GMT_Usage (API, 3, "f: Append <factor> and add <r_min>*<factor> to all distances between nodes and points, "
		"where <r_min> is the shortest inter-point distance found [Default is -Ff0.01].");
	GMT_Option (API, "I");
	GMT_Usage (API, 1, "\n-L Leave trend alone.  Do not remove least squares plane from data before spline fit "
		"[Default removes least squares plane, fits normalized residuals, and restores plane].");
	GMT_Usage (API, 1, "\n-N<nodefile>");
	GMT_Usage (API, -2, "ASCII file with desired output locations. "
		"The resulting ASCII coordinates and interpolation are written to file given in -G "
		"or standard output if no file specified (see -bo for binary output).");
	GMT_Option (API, "R");
	if (gmt_M_showusage (API)) {
		GMT_Usage (API, -2, "Requires -I for specifying equidistant increments.  A gridfile may be given; "
			"this then also sets -I (and perhaps -r); use those options to override the grid settings.");
	}
	GMT_Usage (API, 1, "\n-S<nu>");
	GMT_Usage (API, -2, "Give effective 2-D Poisson's ratio [0.5]. Note: 1.0 is incompressible in a 2-D formulation.");
	GMT_Usage (API, 1, "\n-T<maskgrid>");
	GMT_Usage (API, -2, "Mask grid file whose values are NaN or 0; its header implicitly sets -R, -I (and -r).");
	GMT_Usage (API, 1, "\n-W[+s|w]");
	GMT_Usage (API, -2, "Expects two extra input columns with either data weights or errors sigma_x, sigma_y):");
	GMT_Usage (API, 3, "+s Read sigma-errors and make weights via 1/sigma_x^2, 1/sigma_y^2 [Default].");
	GMT_Usage (API, 3, "+w Read weights directly [Default].");
	GMT_Usage (API, -2, "Note: -W will only have an effect if -C is used.");
	GMT_Option (API, "V,bi");
	if (gmt_M_showusage (API)) GMT_Usage (API, -2, "Default is 4-6 input columns (see -W); use -i to select columns from any data table.");
	GMT_Option (API, "d,e,f,h,i,n,o,qi,r,s,x,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GPSGRIDDER_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gpsgridder and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, pos = 0;
	char p[GMT_BUFSIZ] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Solve by SVD */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				if (strchr (opt->arg, '/') && strstr (opt->arg, "+f") == NULL) {	/* Old-style file deprecated specification */
					if (gmt_M_compat_check (API->GMT, 5)) {	/* OK */
						sscanf (&opt->arg[k], "%lf/%s", &Ctrl->C.value, p);
						Ctrl->C.file = strdup (p);
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -C: Expected -C[[n|r|v]<cut>[%]][+c][+f<file>][+i][+n]\n");
						n_errors++;
					}
					break;	/* No modifiers for the deprecated syntax */
				}
				switch (opt->arg[0]) {	/* First check directives (Default is no directive [0] which is the same as -Cr) */
					case 'n': Ctrl->C.mode = GMT_SVD_EIGEN_NUMBER_CUTOFF;   k = 1; break;
					case 'r': Ctrl->C.mode = GMT_SVD_EIGEN_RATIO_CUTOFF;    k = 1; break;
					case 'v': Ctrl->C.mode = GMT_SVD_EIGEN_VARIANCE_CUTOFF; k = 1; break;
					default:	/* No directive, probably part of a number of modifier, just ignore */
						k = 0; break;
				}
				if ((c = gmt_first_modifier (GMT, &opt->arg[k], "cifmMn"))) {	/* Process any modifiers */
					pos = 0;	/* Reset to start of new word */
					while (gmt_getmodopt (GMT, 'C', c, "cifmMn", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'c': Ctrl->C.history |= GMT_SVD_CUMULATIVE; break;
							case 'i': Ctrl->C.history |= GMT_SVD_INCREMENTAL; break;
							case 'f': Ctrl->C.file = strdup (&p[1]); break;
							case 'm': Ctrl->C.history = GMT_SVD_INCREMENTAL; break;
							case 'M': Ctrl->C.history = GMT_SVD_CUMULATIVE; break;
							case 'n': Ctrl->C.dryrun = true; break;
							default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';
				}				
				if (opt->arg[k]) {	/* See if we got any value argument */
					if (strchr (opt->arg, '%')) {	/* Got percentages of largest eigenvalues */
						Ctrl->C.value = 0.01 * atof (&opt->arg[k]);
						if (Ctrl->C.mode == GMT_SVD_EIGEN_NUMBER_CUTOFF) Ctrl->C.mode = GMT_SVD_EIGEN_PERCENT_CUTOFF;	/* else it is GMT_SVD_EIGEN_VARIANCE_CUTOFF */
						if (Ctrl->C.value < 0.0 || Ctrl->C.value > 1.0) {
							GMT_Report (API, GMT_MSG_ERROR, "Option -C: Percentages must be in 0-100%% range!\n");
							n_errors++;
						}
					}
					else {	/* Got regular cutoff value */
						Ctrl->C.value = atof (&opt->arg[k]);
						if (Ctrl->C.mode == GMT_SVD_EIGEN_NUMBER_CUTOFF && Ctrl->C.value >= 0.0 && Ctrl->C.value < 1.0) /* Old style fraction given instead */
							Ctrl->C.mode = GMT_SVD_EIGEN_PERCENT_CUTOFF;
					}
				}
				if (Ctrl->C.value < 0.0) Ctrl->C.dryrun = true, Ctrl->C.value = 0.0;	/* Check for deprecated syntax giving negative value */
				break;
			case 'E':	/* Evaluate misfit -E[<file>]*/
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				if (opt->arg[0]) {
					Ctrl->E.file = strdup (opt->arg);
					Ctrl->E.mode = GPSGRIDDER_MISFIT;
				}
				break;
			case 'F':	/* Fudge factor  */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				if (opt->arg[0] == 'd') {	/* Specify the delta radius in user units */
					Ctrl->F.mode = GPSGRIDDER_FUDGE_R;
					Ctrl->F.fudge = atof (&opt->arg[1]);
				}
				else if (opt->arg[0] == 'f') {	/* Specify factor used with r_min to set delta radius */
					Ctrl->F.mode = GPSGRIDDER_FUDGE_F;
					Ctrl->F.fudge = atof (&opt->arg[1]);
				}
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -F: -Fd<delta_radius> or -Ff<factor>\n");
					n_errors++;
				}
				break;
			case 'G':	/* Output file name or grid template */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file));
				break;
			case 'I':	/* Grid spacings */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'L':	/* Leave trend alone [Default removes LS plane] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'N':	/* Discrete output locations, no grid will be written */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				if (opt->arg[0]) Ctrl->N.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->N.file))) n_errors++;
				break;
			case 'S':	/* Poission's ratio */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				n_errors += gmt_get_required_double (GMT, opt->arg, opt->option, 0, &Ctrl->S.nu);
				break;
			case 'T':	/* Input mask grid */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				if (opt->arg[0]) Ctrl->T.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->T.file)))
					n_errors++;
				else  {	/* Obtain -R -I -r from file */
					struct GMT_GRID *G = NULL;
					if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->T.file, NULL)) == NULL) {	/* Get header only */
						return (API->error);
					}
					gmt_M_memcpy (GMT->common.R.wesn, G->header->wesn, 4, double);
					gmt_M_memcpy (GMT->common.R.inc, G->header->inc, 2, double);
					GMT->common.R.registration = G->header->registration;
					if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
						return (API->error);
					}
					GMT->common.R.active[RSET] = true;
				}
				break;
			case 'W':	/* Expect data weights in last two columns */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				if (opt->arg[0] == 'w')	/* Deprecated syntax -Ww */
					Ctrl->W.mode = GPSGRIDDER_GOT_W;	/* Got weights instead of sigmas */
				else if (strstr (opt->arg, "+w"))
					Ctrl->W.mode = GPSGRIDDER_GOT_W;	/* Got weights instead of sigmas */
				else if (opt->arg[0] == '\0' || strstr (opt->arg, "+s"))
					Ctrl->W.mode = GPSGRIDDER_GOT_SIG;	/* Got sigmas instead of weights [Default] */
				break;
#ifdef DEBUG
			case 'Z':	/* Dump matrices */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
#endif
			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !(GMT->common.R.active[RSET] || Ctrl->N.active || Ctrl->T.active), "No output locations specified (use either [-R -I], -N, or -T)\n");
	n_errors += gmt_check_binary_io (GMT, 4 + 2*Ctrl->W.active);
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->C.dryrun && !Ctrl->C.file, "Option -C: Must specify file name for eigenvalues if cut < 0\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && !Ctrl->T.file, "Option -T: Must specify mask grid file name\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && !Ctrl->N.file, "Option -N: Must specify node file name\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && Ctrl->N.file && gmt_access (GMT, Ctrl->N.file, R_OK), "Option -N: Cannot read file %s!\n", Ctrl->N.file);
	n_errors += gmt_M_check_condition (GMT, (GMT->common.R.active[ISET] + GMT->common.R.active[RSET]) == 1, "Must specify -R, -I, [-r], -G for gridding\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.nu < -1.0 || Ctrl->S.nu > 1.0, "Option -S: Poisson\'s ratio must be in the -1 <= nu <= +1 range\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

/* GENERAL NUMERICAL FUNCTIONS */

/* Normalization parameters are stored in the coeff array which holds up to GPSGRIDDER_LENGTH terms:
 * coeff[GPSGRIDDER_MEAN_X]:	The mean x coordinate
 * coeff[GPSGRIDDER_MEAN_Y]:	The mean y coordinate
 * coeff[GPSGRIDDER_MEAN_U]:	The mean u observation
 * coeff[GPSGRIDDER_MEAN_V]:	The mean v observation
 * coeff[GPSGRIDDER_SLP_UX]:	The linear x-slope for u
 * coeff[GPSGRIDDER_SLP_UY]:	The linear y-slope for u
 * coeff[GPSGRIDDER_SLP_VX]:	The linear x-slope for v
 * coeff[GPSGRIDDER_SLP_VY]:	The linear y-slope for v
 * coeff[GPSGRIDDER_RANGE_U]:	The largest |range| of the detrended u data
 * coeff[GPSGRIDDER_RANGE_V]:	The largest |range| of the detrended v data
 */

GMT_LOCAL void gpsgridder_do_gps_normalization (struct GMTAPI_CTRL *API, double **X, double *u, double *v, uint64_t n_uv, unsigned int mode, double *coeff) {
	/* We always remove/restore the mean observation values.  mode is a combination of bitflags that affects what we do:
	 * Bit GPSGRIDDER_TREND will also remove linear trend
	 * Bit GPSGRIDDER_NORM will normalize residuals by full range
	 */

	uint64_t i;
	double d, umin = DBL_MAX, vmin = DBL_MAX, umax = -DBL_MAX, vmax = -DBL_MAX;
	char *type[4] = {"Remove mean", "Remove 2-D linear trend\n", "Remove mean and normalize data", "Remove 2-D linear trend and normalize data"};

	GMT_Report (API, GMT_MSG_INFORMATION, "Normalization mode: %s.\n", type[mode]);
	gmt_M_memset (coeff, GPSGRIDDER_LENGTH, double);
	for (i = 0; i < n_uv; i++) {	/* Find mean u and v-values */
		coeff[GPSGRIDDER_MEAN_U] += u[i];
		coeff[GPSGRIDDER_MEAN_V] += v[i];
		if ((mode & GPSGRIDDER_TREND) == 0) continue;	/* Else we also sum up x and y to get their means */
		coeff[GPSGRIDDER_MEAN_X] += X[i][GMT_X];
		coeff[GPSGRIDDER_MEAN_Y] += X[i][GMT_Y];
	}
	coeff[GPSGRIDDER_MEAN_U] /= n_uv;	/* Average u value to remove/restore */
	coeff[GPSGRIDDER_MEAN_V] /= n_uv;	/* Average v value to remove/restore */

	if (mode & GPSGRIDDER_TREND) {	/* Solve for LS plane using deviations from mean x,y,u,v */
		double xx, yy, uu, vv, sxx, sxy, sxu, sxv, syy, syu, syv;
		sxx = sxy = sxu = sxv = syy = syu = syv = 0.0;
		coeff[GPSGRIDDER_MEAN_X] /= n_uv;	/* Mean x */
		coeff[GPSGRIDDER_MEAN_Y] /= n_uv;	/* Mean y */
		for (i = 0; i < n_uv; i++) {
			xx = X[i][GMT_X] - coeff[GPSGRIDDER_MEAN_X];
			yy = X[i][GMT_Y] - coeff[GPSGRIDDER_MEAN_Y];
			uu = u[i] - coeff[GPSGRIDDER_MEAN_U];
			vv = v[i] - coeff[GPSGRIDDER_MEAN_V];
			/* xx,yy,uu,vv are residuals relative to their mean values */
			sxx += (xx * xx);
			sxu += (xx * uu);
			sxv += (xx * vv);
			sxy += (xx * yy);
			syy += (yy * yy);
			syu += (yy * uu);
			syv += (yy * vv);
		}

		d = sxx*syy - sxy*sxy;
		if (d != 0.0) {
			d = 1.0 / d;	/* Se we can multiply below */
			coeff[GPSGRIDDER_SLP_UX] = (sxu*syy - sxy*syu) * d;
			coeff[GPSGRIDDER_SLP_UY] = (sxx*syu - sxy*sxu) * d;
			coeff[GPSGRIDDER_SLP_VX] = (sxv*syy - sxy*syv) * d;
			coeff[GPSGRIDDER_SLP_VY] = (sxx*syv - sxy*sxv) * d;
		}
	}

	/* Remove planes (or just the means) */

	for (i = 0; i < n_uv; i++) {	/* Also find min/max or residuals in the process */
		u[i] -= coeff[GPSGRIDDER_MEAN_U];	/* Always remove mean u value */
		v[i] -= coeff[GPSGRIDDER_MEAN_V];	/* Always remove mean v value */
		if (mode & GPSGRIDDER_TREND) {	/* Also remove planar trends */
			u[i] -= (coeff[GPSGRIDDER_SLP_UX] * (X[i][GMT_X] - coeff[GPSGRIDDER_MEAN_X]) + coeff[GPSGRIDDER_SLP_UY] * (X[i][GMT_Y] - coeff[GPSGRIDDER_MEAN_Y]));
			v[i] -= (coeff[GPSGRIDDER_SLP_VX] * (X[i][GMT_X] - coeff[GPSGRIDDER_MEAN_X]) + coeff[GPSGRIDDER_SLP_VY] * (X[i][GMT_Y] - coeff[GPSGRIDDER_MEAN_Y]));
		}
		/* Find adjusted min/max for u and v */
		if (u[i] < umin) umin = u[i];
		if (u[i] > umax) umax = u[i];
		if (v[i] < vmin) vmin = v[i];
		if (v[i] > vmax) vmax = v[i];
	}
	if (mode & GPSGRIDDER_NORM) {	/* Normalize by u,v ranges */
		double du, dv;
		coeff[GPSGRIDDER_RANGE_U] = MAX (fabs(umin), fabs(umax));	/* Determine u range */
		coeff[GPSGRIDDER_RANGE_V] = MAX (fabs(vmin), fabs(vmax));	/* Determine v range */
		/* Select the maximum range of the two ranges */
        coeff[GPSGRIDDER_RANGE_U] = MAX (coeff[GPSGRIDDER_RANGE_U],coeff[GPSGRIDDER_RANGE_V]);
		coeff[GPSGRIDDER_RANGE_V] = coeff[GPSGRIDDER_RANGE_U];
		if (coeff[GPSGRIDDER_RANGE_U] == 0.0) coeff[GPSGRIDDER_RANGE_U] = 1.0;	/* do no harm */
		if (coeff[GPSGRIDDER_RANGE_V] == 0.0) coeff[GPSGRIDDER_RANGE_V] = 1.0;	/* do no harm */
		du = 1.0 / coeff[GPSGRIDDER_RANGE_U];
		dv = 1.0 / coeff[GPSGRIDDER_RANGE_V];
		for (i = 0; i < n_uv; i++) {	/* Normalize 0-1 */
			u[i] *= du;
			v[i] *= dv;
		}
	}

	/* Recover u(x,y) = u[i] * coeff[GPSGRIDDER_RANGE_U] + coeff[GPSGRIDDER_MEAN_U] + coeff[GPSGRIDDER_SLP_UX]*(x-coeff[GPSGRIDDER_MEAN_X]) + coeff[GPSGRIDDER_SLP_UY]*(y-coeff[GPSGRIDDER_MEAN_Y]) */
	GMT_Report (API, GMT_MSG_INFORMATION, "2-D Normalization coefficients: uoff = %g uxslope = %g xmean = %g uyslope = %g ymean = %g urange = %g\n",
		coeff[GPSGRIDDER_MEAN_U], coeff[GPSGRIDDER_SLP_UX], coeff[GPSGRIDDER_MEAN_X], coeff[GPSGRIDDER_SLP_UY], coeff[GPSGRIDDER_MEAN_Y], coeff[GPSGRIDDER_RANGE_U]);
	/* Recover v(x,y) = v[i] * coeff[GPSGRIDDER_RANGE_V] + coeff[GPSGRIDDER_MEAN_V] + coeff[GPSGRIDDER_SLP_VX]*(x-coeff[GPSGRIDDER_MEAN_X]) + coeff[GPSGRIDDER_SLP_VY]*(y-coeff[GPSGRIDDER_MEAN_Y]) */
	GMT_Report (API, GMT_MSG_INFORMATION, "2-D Normalization coefficients: voff = %g vxslope = %g xmean = %g vyslope = %g ymean = %g vrange = %g\n",
		coeff[GPSGRIDDER_MEAN_V], coeff[GPSGRIDDER_SLP_VX], coeff[GPSGRIDDER_MEAN_X], coeff[GPSGRIDDER_SLP_VY], coeff[GPSGRIDDER_MEAN_Y], coeff[GPSGRIDDER_RANGE_V]);
}

GMT_LOCAL void gpsgridder_undo_gps_normalization (double *X, unsigned int mode, double *coeff) {
 	/* Here, X holds x,y,u,v */
	if (mode & GPSGRIDDER_NORM) {	/* Scale back up by residual data range (if we normalized by range) */
		X[GPSGRIDDER_U] *= coeff[GPSGRIDDER_RANGE_U];
		X[GPSGRIDDER_V] *= coeff[GPSGRIDDER_RANGE_V];
	}
	/* Add in mean data values */
	X[GPSGRIDDER_U] += coeff[GPSGRIDDER_MEAN_U];
	X[GPSGRIDDER_V] += coeff[GPSGRIDDER_MEAN_V];
	if (mode & GPSGRIDDER_TREND) {					/* Restore residual trends */
		X[GPSGRIDDER_U] += coeff[GPSGRIDDER_SLP_UX] * (X[GMT_X] - coeff[GPSGRIDDER_MEAN_X]) + coeff[GPSGRIDDER_SLP_UY] * (X[GMT_Y] - coeff[GPSGRIDDER_MEAN_Y]);
		X[GPSGRIDDER_V] += coeff[GPSGRIDDER_SLP_VX] * (X[GMT_X] - coeff[GPSGRIDDER_MEAN_X]) + coeff[GPSGRIDDER_SLP_VY] * (X[GMT_Y] - coeff[GPSGRIDDER_MEAN_Y]);
	}
}

GMT_LOCAL double gpsgridder_get_gps_radius (struct GMT_CTRL *GMT, double *X0, double *X1) {
	double r;
	/* Get distance between the two points: */
	/* 2-D Cartesian or Flat Earth approximation in km */
	r = gmt_distance (GMT, X0[GMT_X], X0[GMT_Y], X1[GMT_X], X1[GMT_Y]);
	return (r);
}

GMT_LOCAL void gpsgridder_get_gps_dxdy (struct GMT_CTRL *GMT, double *X0, double *X1, double *dx, double *dy, bool geo) {
	/* Get increments dx,dy between point 1 and 0, as measured from point 1.
	 * Point X1 is the fixed point where we are evaluating the solution for all other points X0 */
	if (geo) {	/* Do flat Earth approximation in km */
		double dlon;
		gmt_M_set_delta_lon (X1[GMT_X], X0[GMT_X], dlon);	/* X0[GMT_X] - X1[GMT_X] but checks for 360 wrap */
		*dx = dlon * cosd (0.5 * (X0[GMT_Y] + X1[GMT_Y])) * GMT->current.proj.DIST_KM_PR_DEG;
		*dy = (X0[GMT_Y] - X1[GMT_Y]) * GMT->current.proj.DIST_KM_PR_DEG;
	}
	else {	/* Cartesian data */
		*dx = X0[GMT_X] - X1[GMT_X];
		*dy = X0[GMT_Y] - X1[GMT_Y];
	}
}

GMT_LOCAL void gpsgridder_evaluate_greensfunctions (struct GMT_CTRL *GMT, double *X0, double *X1, double par[], bool geo, double G[]) {
	/* Evaluate the Green's functions q(x), p(x), and w(x), here placed in
	 * G[GPSGRIDDER_FUNC_Q], G[GPSGRIDDER_FUNC_P], and G[GPSGRIDDER_FUNC_W].
	 * Here, par[0] holds Poisson's ratio, par[1] holds delta_r^2 (to prevent singularity) */
	double dx, dy, dx2, dy2, dr2, c1, c2, c2_dr2, dr2_fudge, dx2_fudge, dy2_fudge, dxdy_fudge;

	gpsgridder_get_gps_dxdy (GMT, X0, X1, &dx, &dy, geo);

	dx2 = dx * dx, dy2 = dy * dy;	/* Original squared offsets */
	dr2 = dx2 + dy2;				/* Original radius squared */
	dr2_fudge = dr2 + par[1];	/* Fudged radius squared (par[1] holds delta_r^2) */
	if (dr2 == 0.0)	/* Since r will be fudged away from origin we decide dr2_fudge should fall along N45E trend */
		dx2_fudge = dy2_fudge = dxdy_fudge = 0.5 * par[1];	/* Squared quantities */
	else {	/* Not at singular origin so stretch dx2,dy2 by same amount as dr2 was stretched */
		double stretch2 = dr2_fudge / dr2;	/* How much to lengthen dx2, dy2, dxy */
		dx2_fudge = dx2 * stretch2;	dy2_fudge = dy2 * stretch2;	/* Modified offsets */
		dxdy_fudge = dx * dy * stretch2;
	}

	c1 = (3.0 - par[0]) / 2.0;	/* The half is here since we will take log of r^2, not r */
	c2 = (1.0 + par[0]);

	G[GPSGRIDDER_FUNC_Q] = G[GPSGRIDDER_FUNC_P] = c1 * log (dr2_fudge);
	dr2_fudge = 1.0 / dr2_fudge;	/* Get inverse squared radius */
	c2_dr2 = c2 * dr2_fudge;		/* Do this multiplication once */
	G[GPSGRIDDER_FUNC_Q] +=  c2_dr2 * dy2_fudge;
	G[GPSGRIDDER_FUNC_P] +=  c2_dr2 * dx2_fudge;
	G[GPSGRIDDER_FUNC_W]  = -c2_dr2 * dxdy_fudge;
}

GMT_LOCAL void gpsgridder_dump_system (double *A, double *obs, uint64_t n_params, char *string) {
	/* Dump an A | b system to stderr for debugging */
	uint64_t row, col, ij;
	fprintf (stderr, "\n%s\n", string);
	for (row = 0; row < n_params; row++) {
		ij = row * n_params;
		fprintf (stderr, "%12.6f", A[ij++]);
		for (col = 1; col < n_params; col++, ij++) fprintf (stderr, "\t%12.6f", A[ij]);
		fprintf (stderr, "\t|\t%12.6f\n", obs[row]);
	}
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gpsgridder (void *V_API, int mode, void *args) {
	openmp_int col, row;
	uint64_t n_read, p, k, i, j, seg, n_uv, n_params, n_ok = 0, ij;
	uint64_t Gu_ij, Gv_ij, Guv_ij, Gvu_ij, off, n_duplicates = 0, n_skip = 0;
	unsigned int normalize, n_cols;
	size_t n_alloc;
	int n_use, error, out_ID;
	bool geo, skip;

	char *comp[2] = {"u(x,y)", "v(x,y)"};

	double **X = NULL, *A = NULL, *u = NULL, *v = NULL, *obs = NULL;
	double *f_x = NULL, *f_y = NULL, *in = NULL, *orig_u = NULL, *orig_v = NULL;
	double mem, r, par[2], norm[GPSGRIDDER_LENGTH], var_sum = 0.0;
	double err_sum = 0.0, err_sum_u = 0.0, err_sum_v = 0.0, r_min, r_max, G[3], *A_orig = NULL;
	double *V = NULL, *s = NULL, *ssave = NULL, *b = NULL;

#ifdef DUMPING
	FILE *fp = NULL;
#endif
	struct GMT_GRID *Grid = NULL, *Out[2] = {NULL, NULL};
	struct GMT_RECORD *In = NULL;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *Nin = NULL;
	struct GMT_GRID_INFO info;
	struct GPSGRIDDER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gpsgridder main code ----------------------------*/

	gmt_enable_threads (GMT);	/* Set number of active threads, if supported */
	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	gmt_M_memset (norm, GPSGRIDDER_LENGTH, double);
	gmt_M_memset (&info, 1, struct GMT_GRID_INFO);

	geo = gmt_M_is_geographic (GMT, GMT_IN);
	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Set pointers to 2-D distance functions */
		GMT_Report (API, GMT_MSG_INFORMATION, "Convert lon/lat to geographic distances in km using Flat Earth approximation\n");
		gmt_set_geographic (GMT, GMT_IN);
		gmt_set_geographic (GMT, GMT_OUT);
		gmt_init_distaz (GMT, 'k', GMT_FLATEARTH, GMT_MAP_DIST);
	}
	else {
		GMT_Report (API, GMT_MSG_INFORMATION, "Using Cartesian user distances\n");
		gmt_init_distaz (GMT, 'X', 0, GMT_MAP_DIST);
	}

	normalize = (Ctrl->L.active) ? GPSGRIDDER_NORM : GPSGRIDDER_TREND + GPSGRIDDER_NORM;	/* Do not de-plane if -L, always remove mean and normalize */

	/* Now we are ready to take on some input values */

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	n_cols = (Ctrl->W.active) ? 4 : 2;	/* So X[k][0,1,2,3] will have the x,y weights, if -W is active */
	if ((error = GMT_Set_Columns (API, GMT_IN, n_cols+2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
		Return (error);
	n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	X = gmt_M_memory (GMT, NULL, n_alloc, double *);
	u = gmt_M_memory (GMT, NULL, n_alloc, double);
	v = gmt_M_memory (GMT, NULL, n_alloc, double);

	GMT_Report (API, GMT_MSG_INFORMATION, "Read input data and check for data constraint duplicates\n");
	n_uv = n_read = 0;
	r_min = DBL_MAX;	r_max = -DBL_MAX;
	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				for (p = 0; p < n_uv; p++) gmt_M_free (GMT, X[p]);
				gmt_M_free (GMT, X);	gmt_M_free (GMT, u);	gmt_M_free (GMT, v);
				Return (GMT_RUNTIME_ERROR);
			}
			if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
		}
		if (In->data == NULL) {
			gmt_quit_bad_record (API, In);
			gmt_M_free (GMT, X);	gmt_M_free (GMT, u);	gmt_M_free (GMT, v);
			Return (API->error);
		}

		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */

		if (geo) {	/* Ensure geographic longitudes fit the range since the normalization function expects it */
			if (in[GMT_X] < GMT->common.R.wesn[XLO] && (in[GMT_X] + 360.0) < GMT->common.R.wesn[XHI]) in[GMT_X] += 360.0;
			else if (in[GMT_X] > GMT->common.R.wesn[XHI] && (in[GMT_X] - 360.0) > GMT->common.R.wesn[XLO]) in[GMT_X] -= 360.0;
		}

		X[n_uv] = gmt_M_memory (GMT, NULL, n_cols, double);	/* Allocate space for this constraint */
		X[n_uv][GMT_X] = in[GMT_X];	/* Save x,y  */
		X[n_uv][GMT_Y] = in[GMT_Y];
		/* Check for data duplicates by comparing this point to all previous points */
		skip = false;
		for (i = 0; !skip && i < n_uv; i++) {
			r = gpsgridder_get_gps_radius (GMT, X[i], X[n_uv]);
			if (gmt_M_is_zero (r)) {	/* Duplicates will produce a zero point separation */
				if (doubleAlmostEqualZero (in[GPSGRIDDER_U], u[i]) && doubleAlmostEqualZero (in[GPSGRIDDER_V], v[i])) {
					GMT_Report (API, GMT_MSG_ERROR, "Data constraint %" PRIu64 " is identical to %" PRIu64 " and will be skipped\n", n_read, i);
					skip = true;
					n_skip++;
				}
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Data constraint %" PRIu64 " and %" PRIu64 " occupy the same location but differ in observation (%.12g/%.12g vs %.12g/%.12g)\n", n_read, i, in[GPSGRIDDER_U], u[i], in[GPSGRIDDER_V], v[i]);
					n_duplicates++;
				}
			}
			else {	/* Keep track of min and max point separations */
				if (r < r_min) r_min = r;
				if (r > r_max) r_max = r;
			}
		}
		n_read++;
		if (skip) continue;	/* Current point was an exact duplicate of a previous point */
		u[n_uv] = in[GPSGRIDDER_U];	v[n_uv] = in[GPSGRIDDER_V];	/* Save current u,v data pair */
		if (Ctrl->W.active) {	/* Got sigmas (or weights) in cols 4 and 5 */
			X[n_uv][GPSGRIDDER_WU] = in[4];	/* First just copy over what we got */
			X[n_uv][GPSGRIDDER_WV] = in[5];
			if (Ctrl->W.mode == GPSGRIDDER_GOT_SIG) {	/* Got sigmas, so create weights from them */
				err_sum_u += X[n_uv][GPSGRIDDER_WU]*X[n_uv][GPSGRIDDER_WU];	/* Update u variance */
				err_sum_v += X[n_uv][GPSGRIDDER_WV]*X[n_uv][GPSGRIDDER_WV];	/* Update v variance */
				err_sum += X[n_uv][GPSGRIDDER_WU]*X[n_uv][GPSGRIDDER_WU] + X[n_uv][GPSGRIDDER_WV]*X[n_uv][GPSGRIDDER_WV];	/* Update combined data variance */
				X[n_uv][GPSGRIDDER_WU] = 1.0 / X[n_uv][GPSGRIDDER_WU];	/* We will square these weights later */
				X[n_uv][GPSGRIDDER_WV] = 1.0 / X[n_uv][GPSGRIDDER_WV];	/* We will square these weights later */
			}
		}
		var_sum += u[n_uv] * u[n_uv] + v[n_uv] * v[n_uv];
		n_uv++;			/* Added a new data constraint */
		if (n_uv == n_alloc) {	/* Get more memory */
			n_alloc <<= 1;
			X = gmt_M_memory (GMT, X, n_alloc, double *);
			u = gmt_M_memory (GMT, u, n_alloc, double);
			v = gmt_M_memory (GMT, v, n_alloc, double);
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Normally just disables further data input, but here we got screwed */
		for (p = 0; p < n_uv; p++) gmt_M_free (GMT, X[p]);
		gmt_M_free (GMT, X);	gmt_M_free (GMT, u);	gmt_M_free (GMT, v);
		Return (API->error);
	}

	n_params = 2 * n_uv;	/* Dimension of array is twice the size since using both u & v as separate observations */
	X = gmt_M_memory (GMT, X, n_uv, double *);	/* Realloc to exact size */
	u = gmt_M_memory (GMT, u, n_params, double);	/* We will append v to the end of u later so we need the extra space */
	v = gmt_M_memory (GMT, v, n_uv, double);
	GMT_Report (API, GMT_MSG_INFORMATION, "Found %" PRIu64 " unique data constraints\n", n_uv);
	if (n_skip) GMT_Report (API, GMT_MSG_WARNING, "Skipped %" PRIu64 " data constraints as duplicates\n", n_skip);

	if (Ctrl->W.active && Ctrl->W.mode == GPSGRIDDER_GOT_SIG) {	/* Able to report mean uncertainties */
		err_sum_u = sqrt (err_sum_u / n_uv);
		err_sum_v = sqrt (err_sum_v / n_uv);
		err_sum = sqrt (0.5 * err_sum / n_uv);
		GMT_Report (API, GMT_MSG_INFORMATION, "Mean u-component uncertainty: %g\n", err_sum_u);
		GMT_Report (API, GMT_MSG_INFORMATION, "Mean v-component uncertainty: %g\n", err_sum_v);
		GMT_Report (API, GMT_MSG_INFORMATION, "Combined (u,v) uncertainty  : %g\n", err_sum);
	}

	/* Check for duplicates which would result in a singular matrix system; also update min/max radius */

	GMT_Report (API, GMT_MSG_INFORMATION, "Distance between the closest data constraints:  %.12g\n", r_min);
	GMT_Report (API, GMT_MSG_INFORMATION, "Distance between most distant data constraints: %.12g\n", r_max);

	if (n_duplicates) {	/* These differ in observation value so need to be averaged, medianed, or whatever first */
		GMT_Report (API, GMT_MSG_WARNING, "Found %" PRIu64 " data constraint duplicates with different observation values\n", n_duplicates);
		if (!Ctrl->C.active || gmt_M_is_zero (Ctrl->C.value)) {
			GMT_Report (API, GMT_MSG_WARNING, "You must reconcile duplicates before running gpsgridder since they will result in a singular matrix\n");
			for (p = 0; p < n_uv; p++) gmt_M_free (GMT, X[p]);
			gmt_M_free (GMT, X);
			gmt_M_free (GMT, u);	gmt_M_free (GMT, v);
			Return (GMT_DATA_READ_ERROR);
		}
		else
			GMT_Report (API, GMT_MSG_WARNING, "Expect some eigenvalues to be identically zero\n");
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Found %" PRIu64 " (u,v) pairs, yielding a %" PRIu64 " by %" PRIu64 " set of linear equations\n", n_uv, n_params, n_params);

	if (Ctrl->T.file) {	/* Existing grid that will have zeros and NaNs, only */
		if ((Grid = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL,
		                           Ctrl->T.file, NULL)) == NULL) {	/* Get header only */
			for (p = 0; p < n_uv; p++) gmt_M_free (GMT, X[p]);
			gmt_M_free (GMT, X);	gmt_M_free (GMT, u);	gmt_M_free (GMT, v);
			Return (API->error);
		}
		if (!(Grid->header->wesn[XLO] == GMT->common.R.wesn[XLO] &&
		      Grid->header->wesn[XHI] == GMT->common.R.wesn[XHI] &&
			  Grid->header->wesn[YLO] == GMT->common.R.wesn[YLO] &&
			  Grid->header->wesn[YHI] == GMT->common.R.wesn[YHI])) {
			GMT_Report (API, GMT_MSG_ERROR, "The mask grid does not match your specified region\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (! (Grid->header->inc[GMT_X] == GMT->common.R.inc[GMT_X] && Grid->header->inc[GMT_Y] == GMT->common.R.inc[GMT_Y])) {
			GMT_Report (API, GMT_MSG_ERROR, "The mask grid resolution does not match your specified grid spacing\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (! (Grid->header->registration == GMT->common.R.registration)) {
			GMT_Report (API, GMT_MSG_ERROR, "The mask grid registration does not match your specified grid registration\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->T.file, Grid) == NULL) {	/* Get data */
			Return (API->error);
		}
		(void)gmt_set_outgrid (GMT, Ctrl->T.file, false, 0, Grid, &Out[GMT_X]);	/* true if input is a read-only array; otherwise Out[GMT_X] is just a pointer to Grid */
		n_ok = Out[GMT_X]->header->nm;
		gmt_M_grd_loop (GMT, Out[GMT_X], row, col, k) if (gmt_M_is_fnan (Out[GMT_X]->data[k])) n_ok--;
		/* Duplicate u grid to get another grid for v predictions */
		if ((Out[GMT_Y] = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA, Out[GMT_X])) == NULL) {
			Return (API->error);
		}
	}
	else if (Ctrl->N.active) {	/* Read output locations from file */
		gmt_disable_bghio_opts (GMT);	/* Do not want any -b -g -h -i -o to affect the reading from -C,-F,-L files */
		if ((error = GMT_Set_Columns (API, GMT_IN, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
			Return (error);
		if ((Nin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL,
		                          Ctrl->N.file, NULL)) == NULL) {
			for (p = 0; p < n_uv; p++) gmt_M_free (GMT, X[p]);
			gmt_M_free (GMT, X);	gmt_M_free (GMT, u);	gmt_M_free (GMT, v);
			Return (API->error);
		}
		if (Nin->n_columns < 2) {
			GMT_Report (API, GMT_MSG_ERROR, "Input file %s has %d column(s) but at least 2 are needed\n", Ctrl->N.file, (int)Nin->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_bghio_opts (GMT);	/* Recover settings provided by user (if -b -g -h -i were used at all) */
		T = Nin->table[0];
	}
	else {	/* Fill in an equidistant output table or grid */
		/* Need a full-fledged Grid creation since we are writing it to who knows where */
		for (k = GMT_X; k <= GMT_Y; k++) {
			if ((Out[k] = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL,
			                               NULL, GMT->common.R.registration, GMT_NOTSET, NULL)) == NULL) {
					for (p = 0; p < n_uv; p++) gmt_M_free (GMT, X[p]);
					gmt_M_free (GMT, X);	gmt_M_free (GMT, u);	gmt_M_free (GMT, v);
					Return (API->error);
			}
		}
		n_ok = Out[GMT_X]->header->nm;
	}

	if (Ctrl->E.active) {	/* Need to duplicate the data since SVD will destroy it */
		orig_u = gmt_M_memory (GMT, NULL, n_uv, double);
		orig_v = gmt_M_memory (GMT, NULL, n_uv, double);
		gmt_M_memcpy (orig_u, u, n_uv, double);
		gmt_M_memcpy (orig_v, v, n_uv, double);
	}

	/* Initialize the Green's function machinery */

	par[0] = Ctrl->S.nu;	/* Poisson's ratio */
	if (Ctrl->F.mode == GPSGRIDDER_FUDGE_R)
		par[1] = Ctrl->F.fudge;		/* Small fudge radius to avoid singularity for r = 0 */
	else
		par[1] = Ctrl->F.fudge * r_min;		/* Small fudge factor*r_min to avoid singularity for r = 0 */
	par[1] *= par[1];		/* Compute the square here instead of inside the loop */

	/* Remove mean (or LS plane) from data (we will add these back later) */

	gpsgridder_do_gps_normalization (API, X, u, v, n_uv, normalize, norm);

	/* Set up linear system Ax = b to solve for f_x, f_y body forces */

	mem = (double)n_params * (double)n_params * (double)sizeof (double);	/* In bytes */
	GMT_Report (API, GMT_MSG_INFORMATION, "Square matrix A (size %d x %d) requires %s\n", (int)n_params, (int)n_params, gmt_memory_use ((size_t)mem, 1));
	A = gmt_M_memory (GMT, NULL, n_params * n_params, double);

	if (Ctrl->W.active)
		GMT_Report (API, GMT_MSG_INFORMATION, "Build weighted linear system WAx = Wb\n");
	else
		GMT_Report (API, GMT_MSG_INFORMATION, "Build linear system Ax = b\n");

	off = n_uv * n_params;	/* Separation in 1-D index between rows evaluating u and v for same column */
	for (row = 0; row < (openmp_int)n_uv; row++) {	/* For each data constraint pair (u,v)_row */
		for (col = 0; col < (openmp_int)n_uv; col++) {	/* For each body force pair (f_x,f_y)_col  */
			Gu_ij  = row * n_params + col;	/* Index for Gu term in equation for u */
			Guv_ij = Gu_ij + n_uv;		/* Index for Guv term in equation for u */
			Gvu_ij = Gu_ij + off;		/* Index for Gvu term in equation for v */
			Gv_ij  = Guv_ij + off;		/* Index for Gv term in equation for v */
			gpsgridder_evaluate_greensfunctions (GMT, X[col], X[row], par, geo, G);
			A[Gu_ij]  = G[GPSGRIDDER_FUNC_Q];
			A[Gv_ij]  = G[GPSGRIDDER_FUNC_P];
			A[Guv_ij] = A[Gvu_ij] = G[GPSGRIDDER_FUNC_W];
		}
	}

	gmt_M_memcpy (&u[n_uv], v, n_uv, double);	/* Place v array at end of u array */
	obs = u;					/* Hereafter, use obs to refer to the combined (u,v) array */
	if (Ctrl->Z.active) gpsgridder_dump_system (A, obs, n_params, "A Matrix row || u|v");	/* Dump the A | b system under debug */

	if (Ctrl->E.active && Ctrl->C.history == GMT_SVD_NO_HISTORY) {	/* Needed A to evaluate misfit later as predict = A_orig * x */
		A_orig = gmt_M_memory (GMT, NULL, n_params * n_params, double);
		gmt_M_memcpy (A_orig, A, n_params * n_params, double);
	}
	if (Ctrl->W.active) {	/* Compute mean data rms from (u,v) uncertainties */
		/* Here we have requested an approximate fit instead of an exact interpolation.
		 * For exact interpolation the weights do not matter, but here they do.  Since
		 * we wish to solve via SVD we must convert our unweighted A*x = b linear system
		 * to the weighted W*A*x = W*b whose normal equations are [A'*S*A]*x = A'*S*b,
		 * where S = W*W, the squared weights.  This is N*x = r, where N = A'*S*A and
		 * r = A'*S*b.  Thus, we do these multiplication and store N and r in the
		 * original A and obs vectors so that the continuation of the code can work as is.
		 * Weighted solution idea credit: Leo Uieda.  Jan 14, 2018.
		 */

		double *At = NULL, *AtS = NULL, *S = NULL;	/* Need temporary work space */
		uint64_t ji;

		GMT_Report (API, GMT_MSG_INFORMATION, "Forming weighted normal equations A'SAx = A'Sb -> Nx = r\n");
		At  = gmt_M_memory (GMT, NULL, n_params * n_params, double);
		AtS = gmt_M_memory (GMT, NULL, n_params * n_params, double);
		S   = gmt_M_memory (GMT, NULL, n_params, double);
		/* 1. Transpose A and set diagonal matrix with squared weights (here a vector) S */
		GMT_Report (API, GMT_MSG_INFORMATION, "Create S = W'*W diagonal matrix, A', and compute A' * S\n");
		for (row = 0; row < (openmp_int)n_uv; row++) {	/* Set S, the diagonal squared weights (=1/sigma^2) matrix if given sigma, else use weights as they are */
			S[row]      = (Ctrl->W.mode == GPSGRIDDER_GOT_SIG) ? X[row][GPSGRIDDER_WU] * X[row][GPSGRIDDER_WU] : X[row][GPSGRIDDER_WU];
			S[row+n_uv] = (Ctrl->W.mode == GPSGRIDDER_GOT_SIG) ? X[row][GPSGRIDDER_WV] * X[row][GPSGRIDDER_WV] : X[row][GPSGRIDDER_WV];
		}
		for (row = 0; row < (openmp_int)n_params; row++) {	/* Transpose A */
			for (col = 0; col < (openmp_int)n_params; col++) {
				ij = row * n_params + col;
				ji = col * n_params + row;
				At[ji] = A[ij];
			}
		}
		/* 2. Compute AtS = At * S.  This means scaling all terms in At columns by the corresponding S entry */
		for (row = ij = 0; row < (openmp_int)n_params; row++) {
			for (col = 0; col < (openmp_int)n_params; col++, ij++)
				AtS[ij] = At[ij] * S[col];
		}
		/* 3. Compute r = AtS * obs (but we recycle S to hold r) */
		GMT_Report (API, GMT_MSG_DEBUG, "Compute r = A'*S*b\n");
		gmt_matrix_matrix_mult (GMT, AtS, obs, n_params, n_params, 1U, S);
		/* 4. Compute N = AtS * A (but we recycle At to hold N) */
		GMT_Report (API, GMT_MSG_DEBUG, "Compute N = A'*S*A\n");
		gmt_matrix_matrix_mult (GMT, AtS, A, n_params, n_params, n_params, At);
		/* Now free A, AtS and obs and let "A" be N and "obs" be r; these are the weighted normal equations */
		gmt_M_free (GMT, A);	gmt_M_free (GMT, AtS);	gmt_M_free (GMT, u);
		A = At;	obs = u = S;
		if (Ctrl->Z.active) gpsgridder_dump_system (A, obs, n_params, "Normal equation N row || r");
	}
	if (Ctrl->C.active) {		/* Solve using SVD decomposition */
		int error;

		GMT_Report (API, GMT_MSG_INFORMATION, "Solve linear equations by SVD\n");
#ifndef HAVE_LAPACK
		GMT_Report (API, GMT_MSG_WARNING, "Note: SVD solution without LAPACK will be very very slow.\n");
		GMT_Report (API, GMT_MSG_WARNING, "We strongly recommend you install LAPACK and recompile GMT.\n");
#endif
		V = gmt_M_memory (GMT, NULL, n_params * n_params, double);	/* Hold eigen-vectors */
		s = gmt_M_memory (GMT, NULL, n_params, double);			/* Hold eigen-values */
		if ((error = gmt_svdcmp (GMT, A, (unsigned int)n_params, (unsigned int)n_params, s, V)) != 0) {	/* Not good... */
			gmt_M_free (GMT, s);
			gmt_M_free (GMT, V);
			Return (error);
		}
		if (Ctrl->C.history) {	/* Keep copy of original eigenvalues */
			ssave = gmt_M_memory (GMT, NULL, n_params, double);
			gmt_M_memcpy (ssave, s, n_params, double);
		}

		if (Ctrl->C.file) {	/* Save the eigen-values for study */
			double *eig = gmt_M_memory (GMT, NULL, n_params, double);
			uint64_t e_dim[GMT_DIM_SIZE] = {1, 1, n_params, 2};
			unsigned int col_type[2];
			struct GMT_DATASET *E = NULL;
			for (i = 0; i < n_params; i++) eig[i] = fabs (s[i]);	/* Remove any signs */
			if ((E = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, e_dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create a data set for holding the eigenvalues\n");
				gmt_M_free (GMT, eig);
				Return (API->error);
			}
			E->table[0]->segment[0]->n_rows = n_params;
			gmt_M_memcpy (col_type, GMT->current.io.col_type[GMT_OUT], 2, unsigned int);	/* Save previous x/y output col types */
			gmt_set_cartesian (GMT, GMT_OUT);
			/* Sort eigenvalues into ascending order */
			gmt_sort_array (GMT, eig, n_params, GMT_DOUBLE);
			for (i = 0, j = n_params-1; i < n_params; i++, j--) {
				E->table[0]->segment[0]->data[GMT_X][i] = i;	/* Let 0 be x-value of the first eigenvalue */
				E->table[0]->segment[0]->data[GMT_Y][i] = eig[j];
			}
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, Ctrl->C.file, E) != GMT_NOERROR) {
				gmt_M_free (GMT, eig);
				Return (API->error);
			}
			gmt_M_memcpy (GMT->current.io.col_type[GMT_OUT], col_type, 2, unsigned int);	/* Restore output col types */
			GMT_Report (API, GMT_MSG_INFORMATION, "Eigen-values s(i) saved to %s\n", Ctrl->C.file);
			gmt_M_free (GMT, eig);

			if (Ctrl->C.dryrun) {	/* Only wanted eigen-values; we are done */
				for (p = 0; p < n_uv; p++) gmt_M_free (GMT, X[p]);
				gmt_M_free (GMT, X);
				gmt_M_free (GMT, s);
				gmt_M_free (GMT, V);
				gmt_M_free (GMT, A);
				gmt_M_free (GMT, u);
				gmt_M_free (GMT, v);
				for (k = GMT_X; k <= GMT_Y; k++)
					gmt_free_grid (GMT, &Out[k], true);
				Return (GMT_NOERROR);
			}
		}
		b = gmt_M_memory (GMT, NULL, n_params, double);
		gmt_M_memcpy (b, obs, n_params, double);
		n_use = gmt_solve_svd (GMT, A, (unsigned int)n_params, (unsigned int)n_params, V, s, b, 1U, obs, Ctrl->C.value, Ctrl->C.mode);
		if (n_use == -1) {	/* Something failed in SVD */
			gmt_M_free (GMT, b);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "[%d of %" PRIu64 " eigen-values used]\n", n_use, n_params);

		if (Ctrl->C.history == GMT_SVD_NO_HISTORY) {
			gmt_M_free (GMT, s);
			gmt_M_free (GMT, V);
			gmt_M_free (GMT, b);
		}
	}
	else {				/* Gauss-Jordan elimination */
		int error;
		if (gmt_M_is_zero (r_min)) {
			GMT_Report (API, GMT_MSG_ERROR, "Your matrix is singular because you have duplicate data constraints\n");
			GMT_Report (API, GMT_MSG_ERROR, "Preprocess your data with one of the blockm* modules to eliminate them\n");

		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Solve linear equations by Gauss-Jordan elimination\n");
		if ((error = gmt_gaussjordan (GMT, A, (unsigned int)n_params, obs)) != 0) {
			GMT_Report (API, GMT_MSG_ERROR, "You probably have nearly duplicate data constraints\n");
			GMT_Report (API, GMT_MSG_ERROR, "Preprocess your data with one of the blockm* modules\n");
			gmt_M_free (GMT, A);
			Return (error);
		}
	}
	if (Ctrl->C.history == GMT_SVD_NO_HISTORY) gmt_M_free (GMT, A);

	f_x = obs;			/* Just a different name for clarity since obs vector now holds all body forces f_x, f_y */
	f_y = &obs[n_uv];	/* Halfway down the array we find the start of the f_y body forces */
#ifdef DUMPING
	fp = fopen ("alpha.txt", "w");	/* Save body forces coefficients for debugging purposes */
	for (p = 0; p < n_uv; p++) fprintf (fp, "%g\t%g\n", f_x[p], f_y[p]);
	fclose (fp);
#endif

	if (Ctrl->E.active && Ctrl->C.history == GMT_SVD_NO_HISTORY) {	/* Want to estimate misfits between data and model */
		double here[4], mean = 0.0, std = 0.0, rms = 0.0, *predicted = NULL;
		double mean_u = 0.0, std_u = 0.0, rms_u = 0.0, res_u, dev_u, pvar_sum = 0.0;
		double mean_v = 0.0, std_v = 0.0, rms_v = 0.0, res_v, dev_v, chi2u = 0.0, chi2v = 0.0, chi2u_sum = 0.0, chi2v_sum = 0.0;
		uint64_t e_dim[GMT_DIM_SIZE] = {1, 1, n_uv, 8+2*Ctrl->W.active};
		unsigned int m = 0, m2 = 0;
		struct GMT_DATASET *E = NULL;
		struct GMT_DATASEGMENT *S = NULL;
		predicted = gmt_M_memory (GMT, NULL, n_params, double);	/* To hold predictions */
		gmt_matrix_matrix_mult (GMT, A_orig, obs, n_params, n_params, 1U, predicted);	/* predicted = A * alpha are normalized predictions at data points */
		if (Ctrl->E.mode == GPSGRIDDER_MISFIT) {	/* Want to write out prediction errors */
			if ((E = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, e_dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create a data set for saving misfit estimates\n");
				if (Ctrl->C.history) {
					gmt_M_free (GMT, ssave);
					gmt_M_free (GMT, A);
					gmt_M_free (GMT, s);
					gmt_M_free (GMT, V);
					gmt_M_free (GMT, b);
				}
				Return (API->error);
			}
			S = E->table[0]->segment[0];
			S->n_rows = n_uv;
		}
		for (j = 0; j < n_uv; j++) {	/* For each data constraint pair (u,v) */
			here[GMT_X] = X[j][GMT_X];
			here[GMT_Y] = X[j][GMT_Y];
			here[GPSGRIDDER_U] = predicted[j];
			here[GPSGRIDDER_V] = predicted[j+n_uv];
#if 0
			here[GPSGRIDDER_U] = here[GPSGRIDDER_V] = 0.0;	/* Initialize before we sum up */
			for (p = 0; p < n_uv; p++) {
				gpsgridder_evaluate_greensfunctions (GMT, X[p], here, par, geo, G);
				here[GPSGRIDDER_U] += (f_x[p] * G[GPSGRIDDER_FUNC_Q] + f_y[p] * G[GPSGRIDDER_FUNC_W]);
				here[GPSGRIDDER_V] += (f_x[p] * G[GPSGRIDDER_FUNC_W] + f_y[p] * G[GPSGRIDDER_FUNC_P]);
			}
#endif
			gpsgridder_undo_gps_normalization (here, normalize, norm);
			res_u = orig_u[j] - here[GPSGRIDDER_U];
			res_v = orig_v[j] - here[GPSGRIDDER_V];
			pvar_sum += here[GPSGRIDDER_U] * here[GPSGRIDDER_U] + here[GPSGRIDDER_V] * here[GPSGRIDDER_V];

			rms += pow (res_u, 2.0) + pow (res_v, 2.0);
			if (Ctrl->W.active && Ctrl->W.mode == GPSGRIDDER_GOT_SIG) {	/* If data had uncertainties we also compute the chi2 sum */
				chi2u = pow (res_u * X[j][GPSGRIDDER_WU], 2.0);
				chi2v = pow (res_v * X[j][GPSGRIDDER_WV], 2.0);
				chi2u_sum += chi2u;
				chi2v_sum += chi2v;
			}
			/* Do rms sums */
			rms_u += res_u * res_u;
			rms_v += res_v * res_v;
			rms += res_u * res_u + res_v * res_v;
			/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
			m++;
			dev_u = orig_u[j] - mean_u;
			mean_u += dev_u / m;
			std_u += dev_u * (orig_u[j] - mean_u);
			dev_v = orig_v[j] - mean_v;
			mean_v += dev_v / m;
			std_v += dev_v * (orig_v[j] - mean_v);
			m2++;
			mean += dev_u / m2;
			std += dev_u * (orig_u[j] - mean_u);
			m2++;
			mean += dev_v / m2;
			std += dev_v * (orig_v[j] - mean_v);
			if (Ctrl->E.mode == GPSGRIDDER_MISFIT) {	/* Save information in output dataset */
				for (p = 0; p < 2; p++)
					S->data[p][j] = X[j][p];
				S->data[p++][j] = orig_u[j];
				S->data[p++][j] = here[GPSGRIDDER_U];
				S->data[p++][j] = res_u;
				S->data[p++][j] = orig_v[j];
				S->data[p++][j] = here[GPSGRIDDER_V];
				S->data[p][j]   = res_v;
				if (Ctrl->W.active) {	/* Add the chi^2 terms */
					S->data[++p][j] = chi2u;
					S->data[++p][j] = chi2v;
				}
			}
		}
		rms_u = sqrt (rms_u / n_uv);
		rms_v = sqrt (rms_v / n_uv);
		rms = sqrt (rms / n_params);
		std_u = (m > 1)  ? sqrt (std_u / (m-1.0)) : GMT->session.d_NaN;
		std_v = (m > 1)  ? sqrt (std_v / (m-1.0)) : GMT->session.d_NaN;
		std   = (m2 > 1) ? sqrt (std / (m2-1.0))  : GMT->session.d_NaN;
		if (Ctrl->W.active && Ctrl->W.mode == GPSGRIDDER_GOT_SIG) {
			GMT_Report (API, GMT_MSG_INFORMATION, "Separate u Misfit: N = %u\tMean = %g\tStd.dev = %g\tRMS = %g\tChi^2 = %g\n", n_uv, mean_u, std_u, rms_u, chi2u_sum);
			GMT_Report (API, GMT_MSG_INFORMATION, "Separate v Misfit: N = %u\tMean = %g\tStd.dev = %g\tRMS = %g\tChi^2 = %g\n", n_uv, mean_v, std_v, rms_v, chi2v_sum);
			GMT_Report (API, GMT_MSG_INFORMATION, "Total u,v Misfit : N = %u\tMean = %g\tStd.dev = %g\tRMS = %g\tChi^2 = %g\n", n_params, mean, std, rms, chi2u_sum + chi2v_sum);
		}
		else {
			GMT_Report (API, GMT_MSG_INFORMATION, "Separate u Misfit: N = %u\tMean = %g\tStd.dev = %g\tRMS = %g\n", n_uv, mean_u, std_u, rms_u);
			GMT_Report (API, GMT_MSG_INFORMATION, "Separate v Misfit: N = %u\tMean = %g\tStd.dev = %g\tRMS = %g\n", n_uv, mean_v, std_v, rms_v);
			GMT_Report (API, GMT_MSG_INFORMATION, "Total u,v Misfit : N = %u\tMean = %g\tStd.dev = %g\tRMS = %g\n", n_params, mean, std, rms);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Variance evaluation: Data = %g\tModel = %g\tExplained = %5.1lf %%\n", var_sum, pvar_sum, 100.0 * pvar_sum / var_sum);
		gmt_M_free (GMT, orig_u);
		gmt_M_free (GMT, orig_v);
		gmt_M_free (GMT, predicted);
		gmt_M_free (GMT, A_orig);
		if (Ctrl->E.mode == GPSGRIDDER_MISFIT) {	/* Want to write out prediction errors */
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, Ctrl->E.file, E) != GMT_NOERROR) {
				Return (API->error);
			}
		}
	}

	if (Ctrl->N.file) {	/* Predict solution at specified discrete points only */
		unsigned int wmode = GMT_ADD_DEFAULT;
		double out[4] = {0.0, 0.0, 0.0, 0.0};
		struct GMT_RECORD *Rec = gmt_new_record (GMT, out, NULL);

		/* Must register Ctrl->G.file first since we are going to writing rec-by-rec */
		if (Ctrl->G.active) {
			if ((out_ID = GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, NULL, Ctrl->G.file)) == GMT_NOTSET)
				Return (API->error);
			wmode = GMT_ADD_EXISTING;
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, wmode, 0, options) != GMT_NOERROR) {	/* Establishes output */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, 4, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Evaluate spline at %" PRIu64 " given locations\n", T->n_records);
		/* This cannot be under OpenMP as is since the record writing would appear to be out of sync.  Must instead
		 * save to memory and THEN write the output via GMT_Write_Data */
		for (seg = 0; seg < T->n_segments; seg++) {
			for (row = 0; row < (openmp_int)T->segment[seg]->n_rows; row++) {
				out[GMT_X] = T->segment[seg]->data[GMT_X][row];
				out[GMT_Y] = T->segment[seg]->data[GMT_Y][row];
				out[GPSGRIDDER_U] = out[GPSGRIDDER_V] = 0.0;	/* Initialize before adding up terms */
				for (p = 0; p < n_uv; p++) {
					gpsgridder_evaluate_greensfunctions (GMT, X[p], out, par, geo, G);
					out[GPSGRIDDER_U] += (f_x[p] * G[GPSGRIDDER_FUNC_Q] + f_y[p] * G[GPSGRIDDER_FUNC_W]);
					out[GPSGRIDDER_V] += (f_x[p] * G[GPSGRIDDER_FUNC_W] + f_y[p] * G[GPSGRIDDER_FUNC_P]);
				}
				gpsgridder_undo_gps_normalization (out, normalize, norm);
				GMT_Put_Record (API, GMT_WRITE_DATA, Rec);
			}
		}
		gmt_M_free (GMT, Rec);
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &Nin) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	else {	/* Output on equidistant lattice */
		unsigned int width = urint (floor (log10 ((double)n_use))) + 1;	/* Width of maximum integer needed */
		int64_t col, row, p, e; /* On Windows 'for' index variables must be signed, so redefine these 3 inside this block only */
		char file[PATH_MAX] = {""};
		double *xp = NULL, *yp = NULL, V[4] = {0.0, 0.0, 0.0, 0.0};
		GMT_Report (API, GMT_MSG_INFORMATION, "Evaluate spline at %" PRIu64 " equidistant output locations\n", n_ok);
		/* Precalculate all coordinates */
		xp = Out[GMT_X]->x;
		yp = Out[GMT_X]->y;
		if (Ctrl->C.history) {	/* Write out U,V grids after adding contribution for each eigenvalue */
			static char *mkind[3] = {"", "Incremental", "Cumulative"};
			gmt_grdfloat *current[2] = {NULL, NULL}, *previous[2] = {NULL, NULL};
			double l2_sum_n = 0.0, l2_sum_e = 0.0;
			struct GMT_SINGULAR_VALUE *eigen = NULL;
			struct GMT_DATASET *E = NULL;
			struct GMT_DATASEGMENT *S = NULL;

			for (k = GMT_X; k <= GMT_Y; k++) current[k]  = gmt_M_memory_aligned (GMT, NULL, Out[GMT_X]->header->size, gmt_grdfloat);
			if (Ctrl->C.history & GMT_SVD_INCREMENTAL)
				for (k = GMT_X; k <= GMT_Y; k++) previous[k] = gmt_M_memory_aligned (GMT, NULL, Out[GMT_X]->header->size, gmt_grdfloat);

			if (Ctrl->E.active) {	/* Want to write out misfit as function of eigenvalue */
				uint64_t e_dim[GMT_DIM_SIZE] = {1, 1, n_use, 6+3*Ctrl->W.active};
 				eigen = gmt_sort_svd_values (GMT, ssave, n_params);	/* Get sorted eigenvalues */
				if ((E = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, e_dim, NULL, NULL, 0, 0, NULL)) == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to create a data set for saving misfit estimates per eigenvector\n");
					Return (API->error);
				}
				for (j = 0; j < n_params; j++)	/* Get sum of squared eigenvalues */
					l2_sum_n += eigen[j].value * eigen[j].value;
				S = E->table[0]->segment[0];
				S->n_rows = n_use;
			}

			for (e = 0; e < (int64_t)n_use; e++) {	/* For each eigenvalue selected */
				GMT_Report (API, GMT_MSG_INFORMATION, "Add contribution from eigenvalue %" PRIu64 "\n", e);
				/* Update solution for e eigenvalues only */
				gmt_M_memcpy (s, ssave, n_params, double);
				(void)gmt_solve_svd (GMT, A, (unsigned int)n_params, (unsigned int)n_params, V, s, b, 1U, obs, (double)e, GMT_SVD_EIGEN_NUMBER_CUTOFF);
				if (Ctrl->E.active) {	/* Compute the history of model misfit */
					double V[4],rms = 0.0, rms_u = 0.0, dev_u, rms_v = 0.0, dev_v, chi2u = 0.0, chi2v = 0.0, chi2_sum = 0.0, chi2u_sum = 0.0, chi2v_sum = 0.0;
					for (j = 0; j < n_uv; j++) {	/* For each data constraint pair (u,v) */
						V[GMT_X] = X[j][GMT_X];	V[GMT_Y] = X[j][GMT_Y];
						V[GPSGRIDDER_U] = V[GPSGRIDDER_V] = 0.0;
						for (p = 0; p < (int64_t)n_uv; p++) {	/* Initialize before adding up all body forces */
							gpsgridder_evaluate_greensfunctions (GMT, X[j], X[p], par, geo, G);
							V[GPSGRIDDER_U] += (f_x[p] * G[GPSGRIDDER_FUNC_Q] + f_y[p] * G[GPSGRIDDER_FUNC_W]);
							V[GPSGRIDDER_V] += (f_x[p] * G[GPSGRIDDER_FUNC_W] + f_y[p] * G[GPSGRIDDER_FUNC_P]);
						}
						gpsgridder_undo_gps_normalization (V, normalize, norm);
						dev_u = orig_u[j] - V[GPSGRIDDER_U];	dev_u *= dev_u;
						dev_v = orig_v[j] - V[GPSGRIDDER_V];	dev_v *= dev_v;
						rms_u += dev_u;	rms_v += dev_v;
						rms += dev_u + dev_v;
						if (Ctrl->W.active && Ctrl->W.mode == GPSGRIDDER_GOT_SIG) {	/* If data had uncertainties we also compute the chi2 sum */
							chi2u = dev_u * pow (X[j][GPSGRIDDER_WU], 2.0);
							chi2v = dev_v * pow (X[j][GPSGRIDDER_WV], 2.0);
							chi2u_sum += chi2u;
							chi2v_sum += chi2v;
							chi2_sum += chi2u + chi2v;
						}
					}
					rms   = sqrt (rms   / n_uv);
					rms_u = sqrt (rms_u / n_uv);
					rms_v = sqrt (rms_v / n_uv);
					l2_sum_e += eigen[e].value * eigen[e].value;
					if (Ctrl->W.active)
						GMT_Report (API, GMT_MSG_INFORMATION, "Cumulative data misfit for eigenvalue # %d: rms = %lg chi2 = %lg rms_u = %g rms_v = %lg  chi2_u = %lgchi2_v = %lg\n",
							(int)e, rms, chi2_sum, rms_u, rms_v, chi2u_sum, chi2v_sum);
					else
						GMT_Report (API, GMT_MSG_INFORMATION, "Cumulative data misfit for eigenvalue # %d: rms = %lg rms_u = %g rms_v = %lg\n",
							(int)e, rms, rms_u, rms_v);
					S->data[0][e] = e;	/* Eigenvalue number (starting at 0) */
					S->data[1][e] = eigen[e].value;	/* Eigenvalue, from largest to smallest */
					S->data[2][e] = 100.0 * l2_sum_e / l2_sum_n;	/* Percent of model variance */
					S->data[3][e] = rms;	/* RMS misfit for this solution in total */
					S->data[4][e] = rms_u;	/* RMS misfit for this solution for u-component only */
					S->data[5][e] = rms_v;	/* RMS misfit for this solution for v-component only */
					if (Ctrl->W.active) {
						S->data[6][e] = chi2_sum;	/* Chi^2 sum for this solution in total */
						S->data[7][e] = chi2u_sum;	/* Chi^2 sum for this solution for u-component only */
						S->data[8][e] = chi2v_sum;	/* Chi^2 sum for this solution for v-component only */
					}
				}
#ifdef _OPENMP
#pragma omp parallel for private(row,V,col,ij,p,G) shared(Out,yp,xp,n_uv,GMT,X,par,geo,f_x,f_y,normalize,norm)
#endif
				for (row = 0; row < (openmp_int)Out[GMT_X]->header->n_rows; row++) {
					V[GMT_Y] = yp[row];
					for (col = 0; col < (openmp_int)Out[GMT_X]->header->n_columns; col++) {
						ij = gmt_M_ijp (Out[GMT_X]->header, row, col);
						if (gmt_M_is_fnan (Out[GMT_X]->data[ij])) continue;	/* Only evaluate solution where mask is not NaN */
						V[GMT_X] = xp[col];
						/* Here, (V[GMT_X], V[GMT_Y]) are the current output coordinates */
						for (p = 0, V[GPSGRIDDER_U] = V[GPSGRIDDER_V] = 0.0; p < (int64_t)n_uv; p++) {	/* Initialize before adding up all body forces */
							gpsgridder_evaluate_greensfunctions (GMT, X[p], V, par, geo, G);
							V[GPSGRIDDER_U] += (f_x[p] * G[GPSGRIDDER_FUNC_Q] + f_y[p] * G[GPSGRIDDER_FUNC_W]);
							V[GPSGRIDDER_V] += (f_x[p] * G[GPSGRIDDER_FUNC_W] + f_y[p] * G[GPSGRIDDER_FUNC_P]);
						}
						gpsgridder_undo_gps_normalization (V, normalize, norm);
						Out[GMT_X]->data[ij] = (gmt_grdfloat)V[GPSGRIDDER_U];
						Out[GMT_Y]->data[ij] = (gmt_grdfloat)V[GPSGRIDDER_V];
					}
				}
				for (k = GMT_X; k <= GMT_Y; k++) gmt_M_memcpy (current[k], Out[k]->data, Out[k]->header->size, gmt_grdfloat);	/* Save current solutions */

				if (Ctrl->C.history & GMT_SVD_CUMULATIVE) {	/* Write out the cumulative solution first */
					for (k = GMT_X; k <= GMT_Y; k++) {	/* Write the two grids with u(x,y) and v(xy) */
						gpsgridder_set_filename (Ctrl->G.file, e, width, GMT_SVD_CUMULATIVE, k, file);
						snprintf (Out[k]->header->remark, GMT_GRID_REMARK_LEN160, "%s Strain component %s for eigenvalue # %d", mkind[GMT_SVD_INCREMENTAL], comp[k], (int)e);
						if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out[k]))
							Return (API->error);				/* Update solution for k eigenvalues only */
						if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, Out[k]) != GMT_NOERROR)
							Return (API->error);
					}
				}
				if (Ctrl->C.history & GMT_SVD_INCREMENTAL) {	/* Want to write out incremental solution due to this eigenvalue */
					for (k = GMT_X; k <= GMT_Y; k++) {	/* Incremental improvement since last time */
						gmt_M_grd_loop (GMT, Out[k], row, col, ij) Out[k]->data[ij] = current[k][ij] - previous[k][ij];
						gmt_M_memcpy (previous[k], current[k], Out[k]->header->size, gmt_grdfloat);	/* Save current solution which will be previous for next eigenvalue */
						gpsgridder_set_filename (Ctrl->G.file, e, width, GMT_SVD_INCREMENTAL, k, file);
						snprintf (Out[k]->header->remark, GMT_GRID_REMARK_LEN160, "%s Strain component %s for eigenvalue # %d", mkind[GMT_SVD_CUMULATIVE], comp[k], (int)e);
						if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out[k]))
							Return (API->error);
						if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, Out[k]) != GMT_NOERROR)
							Return (API->error);
					}
				}
			}
			if (Ctrl->E.active) {	/* Compute the history of model misfit as rms */
				char header[GMT_LEN128] = {""};
				sprintf (header, "# eigenno\teigenval\tvar_percent\trms\trms_u\trms_v");
				if (Ctrl->W.active) strcat (header, "\tchi2\tchi2_u\tchi2_v");
				if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, E)) {
					Return (API->error);
				}
				gmt_set_tableheader (API->GMT, GMT_OUT, true);	/* So header is written */
				for (k = 0; k < 9; k++) GMT->current.io.col_type[GMT_OUT][k] = GMT_IS_FLOAT;	/* Set plain float column types */
				if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, Ctrl->E.file, E) != GMT_NOERROR) {
					Return (API->error);
				}
				gmt_M_free (GMT, eigen);
				gmt_M_free (GMT, orig_u);
				gmt_M_free (GMT, orig_v);
			}

			for (k = GMT_X; k <= GMT_Y; k++) gmt_M_free_aligned (GMT, current[k]);
			if (Ctrl->C.history & GMT_SVD_INCREMENTAL)
				for (k = GMT_X; k <= GMT_Y; k++) gmt_M_free_aligned (GMT, previous[k]);
			gmt_M_free (GMT, A);
			gmt_M_free (GMT, s);
			gmt_M_free (GMT, v);
			gmt_M_free (GMT, b);
			gmt_M_free (GMT, ssave);
		}
		else {
#ifdef _OPENMP
			int64_t row, col;	/* Windows demands signed integers for loop variables */
#pragma omp parallel for private(row,V,col,ij,p,G) shared(Out,yp,xp,n_uv,GMT,X,par,geo,f_x,f_y,normalize,norm)
#endif
			for (row = 0; row < (openmp_int)Out[GMT_X]->header->n_rows; row++) {
				V[GMT_Y] = yp[row];
				for (col = 0; col < (openmp_int)Out[GMT_X]->header->n_columns; col++) {
					ij = gmt_M_ijp (Out[GMT_X]->header, row, col);
					if (gmt_M_is_fnan (Out[GMT_X]->data[ij])) continue;	/* Only evaluate solution where mask is not NaN */
					V[GMT_X] = xp[col];
					/* Here, (V[GMT_X], V[GMT_Y]) are the current output coordinates */
					for (p = 0, V[GPSGRIDDER_U] = V[GPSGRIDDER_V] = 0.0; p < (int64_t)n_uv; p++) {	/* Initialize before adding up terms */
						gpsgridder_evaluate_greensfunctions (GMT, X[p], V, par, geo, G);
						V[GPSGRIDDER_U] += (f_x[p] * G[GPSGRIDDER_FUNC_Q] + f_y[p] * G[GPSGRIDDER_FUNC_W]);
						V[GPSGRIDDER_V] += (f_x[p] * G[GPSGRIDDER_FUNC_W] + f_y[p] * G[GPSGRIDDER_FUNC_P]);
					}
					gpsgridder_undo_gps_normalization (V, normalize, norm);
					Out[GMT_X]->data[ij] = (gmt_grdfloat)V[GPSGRIDDER_U];
					Out[GMT_Y]->data[ij] = (gmt_grdfloat)V[GPSGRIDDER_V];
				}
			}
			for (k = GMT_X; k <= GMT_Y; k++) {	/* Write the two grids with u(x,y) and v(xy) */
				gmt_grd_init (GMT, Out[k]->header, options, true);
				snprintf (Out[k]->header->remark, GMT_GRID_REMARK_LEN160, "Strain component %s", comp[k]);
				gpsgridder_set_filename (Ctrl->G.file, 0, 0, 0, k, file);
				if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out[k])) {
					gmt_M_free (GMT, xp);	gmt_M_free (GMT, yp);
					Return (API->error);
				}
				if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, Out[k]) != GMT_NOERROR) {
					gmt_M_free (GMT, xp);	gmt_M_free (GMT, yp);
					Return (API->error);
				}
			}
		}
	}

	/* Clean up */

	for (p = 0; p < n_uv; p++) gmt_M_free (GMT, X[p]);
	gmt_M_free (GMT, X);
	gmt_M_free (GMT, u);
	gmt_M_free (GMT, v);
	if (Ctrl->C.active) gmt_M_free (GMT, V);

	Return (GMT_NOERROR);
}
