/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: Based on a specified grid size, gmtbinstats reads an xy[z][w] file and
 * find all values inside a radius R centered on each output node.  Points inside the
 * circle will be ued to compute the desired output statistic.  Alternatively, we do a
 * rectangular or hexagonal tiling of the data.
 *
 * Author:	Paul Wessel
 * Date:	15-AUG-2020
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtbinstats"
#define THIS_MODULE_MODERN_NAME	"gmtbinstats"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Bin spatial data and determine statistics per bin"
#define THIS_MODULE_KEYS	"<D{,>?}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVabdefghiqrw"

enum gmtbinstats_types {
	GMTBINSTATS_LOWER = 1,
	GMTBINSTATS_LOWERP,
	GMTBINSTATS_UPPER,
	GMTBINSTATS_UPPERP,
	GMTBINSTATS_ZRANGE,
	GMTBINSTATS_SUM,
	GMTBINSTATS_COUNT,
	GMTBINSTATS_COUNTW,
	GMTBINSTATS_MEAN,
	GMTBINSTATS_MEANW,
	GMTBINSTATS_MEDIAN,
	GMTBINSTATS_MEDIANW,
	GMTBINSTATS_MODE,
	GMTBINSTATS_MODEW,
	GMTBINSTATS_QUANT,
	GMTBINSTATS_QUANTW,
	GMTBINSTATS_IRANGE,
	GMTBINSTATS_IRANGEW,
	GMTBINSTATS_STD,
	GMTBINSTATS_STDW,
	GMTBINSTATS_MAD,
	GMTBINSTATS_MADW,
	GMTBINSTATS_LMSSCL,
	GMTBINSTATS_LMSSCLW,
	GMTBINSTATS_RMS,
	GMTBINSTATS_RMSW
};

#define GMTBINSTATS_HEXAGONAL 1
#define GMTBINSTATS_RECTANGULAR 2

struct GMTBINSTATS_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct GMTBINSTATS_C {	/* -Ca|d|g|i|l|L|m|n|o|p|q[<val>]|r|s|u|U|z */
		bool active;
		unsigned int mode;
		double quant;
	} C;
	struct GMTBINSTATS_E {	/* -E<empty> */
		bool active;
		double value;
	} E;
	struct GMTBINSTATS_G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct GMTBINSTATS_I {	/* -I (for checking only) */
		bool active;
	} I;
	struct GMTBINSTATS_N {	/* -N */
		bool active;
	} N;
	struct GMTBINSTATS_S {	/* -S[-|=|+]<radius>[d|e|f|k|m|M|n] */
		bool active;
		int mode;	/* May be negative */
		double radius;
		double radius_inner;	/* Inner radius for hexagon tiling */
		double area;
		char unit;
	} S;
	struct GMTBINSTATS_T {	/* -T[h|r] */
		bool active;
		unsigned int mode;
	} T;
	struct GMTBINSTATS_W {	/* -W[+s] */
		bool active;
		bool sigma;
	} W;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTBINSTATS_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1U, struct GMTBINSTATS_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.mode = GMTBINSTATS_COUNT;
	C->C.quant = 50.0;	/* Median*/
	C->E.value = GMT->session.d_NaN;
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTBINSTATS_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] -Ca|d|g|i|l|L|m|n|o|p|q[<val>]|r|s|u|U|z -G%s %s %s -S%s [-E<empty>] [-N] "
		"[-T[h|r]] [%s] [-W[+s|w]] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_OUTGRID, GMT_I_OPT, GMT_Rgeo_OPT, GMT_RADIUS_OPT, GMT_V_OPT, GMT_a_OPT, GMT_bi_OPT, GMT_di_OPT,
		GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_qi_OPT, GMT_r_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	GMT_Usage (API, 1, "\n-C Specify the statistic of data we should report per bin.  Choose from:");
	GMT_Usage (API, -3, "a: The mean (average)");
	GMT_Usage (API, -3, "d: The median absolute deviation (MAD)");
	GMT_Usage (API, -3, "g: The full data range (max-min)");
	GMT_Usage (API, -3, "i: The 25-75%% interquartile range");
	GMT_Usage (API, -3, "l: The minimum (low)");
	GMT_Usage (API, -3, "L: The minimum of all positive values");
	GMT_Usage (API, -3, "m: The median");
	GMT_Usage (API, -3, "n: The number of values [Default]");
	GMT_Usage (API, -3, "o: The LMS scale");
	GMT_Usage (API, -3, "p: The mode (maximum likelihood)");
	GMT_Usage (API, -3, "q: The selected quantile value; append quantile [50%%]");
	GMT_Usage (API, -3, "r: The r.m.s.");
	GMT_Usage (API, -3, "s: The standard deviation");
	GMT_Usage (API, -3, "u: The maximum (upper)");
	GMT_Usage (API, -3, "U: The maximum of all negative values");
	GMT_Usage (API, -3, "z: The sum");
	gmt_outgrid_syntax (API, 'G', "Set name of the output grid file");
	GMT_Option (API, "I");
	GMT_Option (API, "R");
	gmt_dist_syntax (API->GMT, "S" GMT_RADIUS_OPT, "Compute statistics using points inside this search radius.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-E Value to use for empty bins [Default is NaN].");
	GMT_Usage (API, 1, "\n-N Normalize the output by the area of the bins [no normalization].");
	GMT_Usage (API, 1, "\n-T Use area-covering tiling to set up non-overlapping bins. Choose binning scheme:");
	GMT_Usage (API, -3, "h: hexagonal binning, write non-equidistant table to standard output (or file named in -G).");
	GMT_Usage (API, -3, "r: rectangular binning, writes equidistant grid (named via -G) [Default].");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-W[+s|w]");
	GMT_Usage (API, -2, "Weighted input given, weights in 4th column; compute the weighted version "
		"of selection in -C [Default is unweighted]. Select modifier:");
	GMT_Usage (API, 3, "+s Read standard deviations and compute weights as 1/s.");
	GMT_Usage (API, 3, "+w Read weights directly [Default].");
	GMT_Option (API, "a,bi");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is 3 (or 4 if -W is set) columns.");
	GMT_Option (API, "di,e,f,h,i");
	GMT_Option (API, "qi,r,w,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GMTBINSTATS_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtbinstats and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file(s) */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			/* Processes program-specific parameters */

			case 'E':	/* NaN value */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				Ctrl->E.active = true;
				if (opt->arg[0])
					Ctrl->E.value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.d_NaN : atof (opt->arg);
				else {
					n_errors++;
					GMT_Report (API, GMT_MSG_ERROR, "Option -E: Must specify value or NaN\n");
				}
				break;
			case 'G':	/* Output file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file))) n_errors++;
				break;
			case 'I':	/* Grid spacings */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'C':	/* -Ca|d|i|l|L|m|n|o|p|q[<val>]|r|s|u|U|z */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				switch (opt->arg[0]) {
					case 'a': Ctrl->C.mode = GMTBINSTATS_MEAN;	break;
					case 'd': Ctrl->C.mode = GMTBINSTATS_MAD;	break;
					case 'g': Ctrl->C.mode = GMTBINSTATS_ZRANGE;	break;
					case 'i': Ctrl->C.mode = GMTBINSTATS_IRANGE;	break;
					case 'l': Ctrl->C.mode = GMTBINSTATS_LOWER;	break;
					case 'L': Ctrl->C.mode = GMTBINSTATS_LOWERP;	break;
					case 'm': Ctrl->C.mode = GMTBINSTATS_MEDIAN;	break;
					case 'n': Ctrl->C.mode = GMTBINSTATS_COUNT;	break;
					case 'o': Ctrl->C.mode = GMTBINSTATS_LMSSCL;	break;
					case 'p': Ctrl->C.mode = GMTBINSTATS_MODE;	break;
					case 'q': Ctrl->C.mode = GMTBINSTATS_QUANT;
						if (opt->arg[1]) Ctrl->C.quant = atof (&opt->arg[1]);
						break;
					case 'r': Ctrl->C.mode = GMTBINSTATS_RMS;	break;
					case 's': Ctrl->C.mode = GMTBINSTATS_STD;	break;
					case 'u': Ctrl->C.mode = GMTBINSTATS_UPPER;	break;
					case 'U': Ctrl->C.mode = GMTBINSTATS_UPPERP;	break;
					case 'z': Ctrl->C.mode = GMTBINSTATS_SUM;	break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -C: Method %s not recognized!\n", opt->arg);
						n_errors++;
						break;
				}
				break;
			case 'N':	/* Normalize by area */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				break;
			case 'S':	/* Search radius */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.active = true;
				Ctrl->S.mode = gmt_get_distance (GMT, opt->arg, &(Ctrl->S.radius), &(Ctrl->S.unit));
				break;
			case 'T':	/* Select hexagonal or rectangular tiling */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				switch (opt->arg[0]) {
					case 'h': Ctrl->T.mode = GMTBINSTATS_HEXAGONAL; break;
					case 'r': Ctrl->T.mode = GMTBINSTATS_RECTANGULAR; break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -T: Method %s not recognized!\n", opt->arg);
						n_errors++;
						break;
				}
				break;
			case 'W':	/* Use weights */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				if (gmt_validate_modifiers (GMT, opt->arg, 'W', "sw", GMT_MSG_ERROR)) n_errors++;
				Ctrl->W.sigma = (strstr (opt->arg, "+s")) ? true : false;
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Option -I: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->C.active, "Option -C: Must specify a method for the processing");
	if (Ctrl->T.active) {
		if (Ctrl->T.mode == GMTBINSTATS_HEXAGONAL) {
			n_errors += gmt_M_check_condition (GMT, gmt_M_is_geographic (GMT, GMT_IN), "Option -Th: Hexagonal tiling is a Cartesian operation\n");
			n_errors += gmt_M_check_condition (GMT, !doubleAlmostEqual (GMT->common.R.inc[GMT_X], GMT->common.R.inc[GMT_Y]), "Option -Th: Give a single argument reflecting desired y-increment\n");
			n_errors += gmt_M_check_condition (GMT, Ctrl->E.active, "Option -Th: The -E option is not allowed for hexagonal tiling\n");
		}
		else {
			n_errors += gmt_M_check_condition (GMT, !Ctrl->G.active, "Option -Tr: -G is a required argument when -Tr is used\n");
		}
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.active, "Option -T: No search radius -S can be set for tiling\n");
	}
	else {
		n_errors += gmt_M_check_condition (GMT, !Ctrl->S.active, "Option -S is a required argument when -T is not used\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -1, "Option -S: Unrecognized unit\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -2, "Option -S: Unable to decode radius\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -3, "Option -S: Radius is negative\n");
	}
	n_errors += gmt_check_binary_io (GMT, (Ctrl->W.active) ? 4 : 3);
	if (Ctrl->W.active) {	/* Update the mode if median or mode */
		if (Ctrl->C.mode == GMTBINSTATS_MEDIAN) Ctrl->C.mode = GMTBINSTATS_MEDIANW;
		if (Ctrl->C.mode == GMTBINSTATS_MODE) Ctrl->C.mode = GMTBINSTATS_MODEW;
		if (Ctrl->C.mode == GMTBINSTATS_MEAN) Ctrl->C.mode = GMTBINSTATS_MEANW;
		if (Ctrl->C.mode == GMTBINSTATS_MAD) Ctrl->C.mode = GMTBINSTATS_MADW;
		if (Ctrl->C.mode == GMTBINSTATS_STD) Ctrl->C.mode = GMTBINSTATS_STDW;
		if (Ctrl->C.mode == GMTBINSTATS_LMSSCL) Ctrl->C.mode = GMTBINSTATS_LMSSCLW;
		if (Ctrl->C.mode == GMTBINSTATS_COUNT) Ctrl->C.mode = GMTBINSTATS_COUNTW;
		if (Ctrl->C.mode == GMTBINSTATS_QUANT) Ctrl->C.mode = GMTBINSTATS_QUANTW;
		if (Ctrl->C.mode == GMTBINSTATS_IRANGE) Ctrl->C.mode = GMTBINSTATS_IRANGEW;
		if (Ctrl->C.mode == GMTBINSTATS_RMS) Ctrl->C.mode = GMTBINSTATS_RMSW;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void gmtbinstats_assign_node (struct GMT_CTRL *GMT, struct GMT_GRID *G, char *visited, gmt_grdfloat *w, gmt_grdfloat *s, gmt_grdfloat **zp, struct GMT_OBSERVATION **p, \
		unsigned int *np, unsigned int *n_alloc, uint64_t ij, uint64_t kk, unsigned int mode, double *in, double weight) {
	/* Assign the input value/weight to the output grid and/or helper arrays, depending on the mode */

	if (gmt_M_is_fnan (G->data[ij])) G->data[ij] = 0.0;	/* About to be assigned for the first time */
	switch (mode) {
		case GMTBINSTATS_MEAN:
			G->data[ij] += in[GMT_Z];
			w[kk] += 1.0;
			break;
		case GMTBINSTATS_MEANW:
			G->data[ij] += in[GMT_Z] * weight;
			w[kk] += weight;
			break;
		case GMTBINSTATS_STD:
			G->data[ij] += in[GMT_Z];
			s[kk] += in[GMT_Z] * in[GMT_Z];	/* Sum of z squared */
			np[kk]++;
			break;
		case GMTBINSTATS_STDW:
			G->data[ij] += in[GMT_Z] * weight;	/* Sum of weighted z */
			w[kk] += weight;	/* Sum of weights */
			s[kk] += in[GMT_Z] * in[GMT_Z] * weight;	/* Sum of weighted z^2 */
			np[kk]++;
			break;
		case GMTBINSTATS_RMS:
			G->data[ij] += in[GMT_Z] * in[GMT_Z];	/* Sum of z squared */
			np[kk]++;
			break;
		case GMTBINSTATS_RMSW:
			G->data[ij] += in[GMT_Z] * in[GMT_Z] * weight;	/* Sum of weighted z^2 */
			w[kk] += weight;	/* Sum of weights */
			break;
		case GMTBINSTATS_MEDIAN: case GMTBINSTATS_MODE: case GMTBINSTATS_QUANT: case GMTBINSTATS_MAD: case GMTBINSTATS_LMSSCL: case GMTBINSTATS_IRANGE:
			if (n_alloc[kk] >= np[kk]) {
				n_alloc[kk] += GMT_SMALL_CHUNK;
				zp[kk] = gmt_M_memory (GMT, zp[kk], n_alloc[kk], gmt_grdfloat);
			}
			zp[kk][np[kk]++] = in[GMT_Z];
			break;
		case GMTBINSTATS_MEDIANW: case GMTBINSTATS_MODEW: case GMTBINSTATS_QUANTW: case GMTBINSTATS_MADW: case GMTBINSTATS_LMSSCLW: case GMTBINSTATS_IRANGEW:
			if (n_alloc[kk] >= np[kk]) {
				n_alloc[kk] += GMT_SMALL_CHUNK;
				p[kk] = gmt_M_memory (GMT, p[kk], n_alloc[kk], struct GMT_OBSERVATION);
			}
			p[kk][np[kk]].value    = in[GMT_Z];
			p[kk][np[kk]++].weight = weight;
			break;
		case GMTBINSTATS_LOWERP:
			if (in[GMT_Z] <= 0.0) return;	/* Only consider positive values */
			/* Fall through on purpose */
		case GMTBINSTATS_LOWER:
			if (visited[kk]) {	/* Been here before, so can make a comparison */
				if (in[GMT_Z] < G->data[ij]) G->data[ij] = in[GMT_Z];
			}
			else {	/* First time, mark it */
				G->data[ij] = in[GMT_Z];
				visited[kk] = 1;
			}
			break;
		case GMTBINSTATS_UPPERP:
			if (in[GMT_Z] >= 0.0) return;	/* Only consider negative values */
			/* Fall through on purpose */
		case GMTBINSTATS_UPPER:
			if (visited[kk]) {	/* Been here before, so can make a comparison */
				if (in[GMT_Z] > G->data[ij]) G->data[ij] = in[GMT_Z];
			}
			else {	/* First time, mark it */
				G->data[ij] = in[GMT_Z];
				visited[kk] = 1;
			}
			break;
		case GMTBINSTATS_ZRANGE:
			if (visited[kk]) {	/* Been here before, so can make a comparison */
				if (in[GMT_Z] > G->data[ij]) G->data[ij] = in[GMT_Z];
				if (in[GMT_Z] < s[kk]) s[kk] = in[GMT_Z];
			}
			else {	/* First time, mark it */
				G->data[ij] = s[kk] = in[GMT_Z];
				visited[kk] = 1;
			}
			break;
		case GMTBINSTATS_SUM: case GMTBINSTATS_COUNTW:
			G->data[ij] += in[GMT_Z];
			break;
		default:	/* count */
			G->data[ij] += 1.0;
			break;
	}
}

bool outside_hexagon (struct GMT_CTRL *GMT, double x0, double y0, double x, double y, double distance, double radius_inner) {
	/* Return true if we are outside the hexagon.  Print statements left for future debugging */
	int sector;	/* 0-5 */
	bool answer;
	double d0, d, q;
#ifdef DEBUG
	static char *status[2] = {"inside", "outside"};
	char message[GMT_LEN256] = {""}, string[GMT_LEN128] = {""};
	sprintf (message, "x0 = %5.2lf y0 = %5.2lf to x = %5.2lf y = %5.2lf d = %6.3lf: ", x0, y0, x, y, distance);
#endif
	if (distance <= radius_inner) {	/* We are clearly inside the hexagon */
#ifdef DEBUG
		sprintf (string, "inside inner radius %5.3lg\n", radius_inner);
		strcat (message, string);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, message);
#endif
		return false;
	}
	d0 = 90.0 - gmt_az_backaz (GMT, x0, y0, x, y, false);	/* Get angle with horizontal of line from center to point */
	if (d0 < 0.0) d0 += 360.0;	/* Ensure we only get positive sectors */
	sector = floor (d0 / 60.0);	/* Determine which of the 6 sectors (0-5) */
	d = d0 - sector * 60.0 - 30.0;	/* Find deviation (up to +/- 30 degrees) from the normal to this hexagonal side */
	q = radius_inner / cosd (d);	/* Get distance from center to hexagon boundary along the line */
	answer = (distance > q);	/* We are either inside the cord or in area between circumscribed circle and cord */
#ifdef DEBUG
	sprintf (string, "%s cord a0 = %5.1lf sector = %d a = %4.1lf q = %6.3lf\n", status[answer], d0, sector, d, q);
	strcat (message, string);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, message);
#endif
	return (answer);
}

#define skip_column(row,col) ((row)%2 != (col)%2)	/* Skip the unused pseudo-nodes where row,col are even,odd or odd,even */

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_gmtbinstats (void *V_API, int mode, void *args) {
	char *visited = NULL;

	bool wrap_180, replicate_x, replicate_y, hex_tiling, rect_tiling, geographic;

	int col_0, row_0, row, col, row_end, col_stop, col_start, ii, jj, error = 0;
	unsigned int n_cols, k, rowu, colu, d_row, y_wrap_kk, y_wrap_ij, max_d_col, x_wrap;
	unsigned int *d_col = NULL, *n_in_circle = NULL, *n_alloc = NULL;
	unsigned int gmt_mode_selection = 0, GMT_n_multiples = 0, col_inc = 1;

	uint64_t ij, kk, n = 0, n_read = 0;

	gmt_grdfloat *w = NULL, *s = NULL, **zp = NULL;

	double dx, dy, dx2, dy2, distance = 0.0, x_left, x_right, y_top, y_bottom, weight = 1.0;
	double half_y_width, y_width, half_x_width, x_width, median, zmode, mad;
	double *x0 = NULL, *y0 = NULL, *in = NULL;

	struct GMT_GRID *Grid = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_RECORD *In = NULL;
	struct GMT_OBSERVATION **zw_pair = NULL;
	struct GMTBINSTATS_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the gmtbinstats main code ----------------------------*/

	if (Ctrl->T.active) {	/* Adjust for pseudo-hexagonal grid nodes */
		if (Ctrl->T.mode == GMTBINSTATS_HEXAGONAL) {
			double xmax;
			unsigned int nx;
			Ctrl->S.radius_inner = 0.5 * GMT->common.R.inc[GMT_Y];	/* Radius of the interior of the hexagon */
			Ctrl->S.radius = Ctrl->S.radius_inner / cosd (30.0);	/* Circumscribing radius of the hexagon */
			/* First ensure w/e is compatible with s/n and dy */
			GMT->common.R.inc[GMT_X] = 3.0 * Ctrl->S.radius;
			nx = gmt_M_get_n (GMT, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.inc[GMT_X], 0.0);
			xmax = GMT->common.R.wesn[XLO] + (nx - 1) * GMT->common.R.inc[GMT_X];
			if (!doubleAlmostEqualZero (GMT->common.R.wesn[XHI], xmax)) {
				GMT_Report (API, GMT_MSG_WARNING, "Hexagonal geometry requires xmax to be adjusted from %g to %g\n", GMT->common.R.wesn[XHI], xmax);
				GMT->common.R.wesn[XHI] = xmax;
			}
			/* Divide grid spacings by two for the pseudo-grid parameters. We do this to ensure every hexagonal center
			 * is represented by a grid node, knowing that about half of the grid nodes do not correspond to a hex center.
			 * This is done to simplify coding (always fill a grid) but we check for NaNs on output for unused nodes. */
			GMT->common.R.inc[GMT_Y] /= 2.0;
			GMT->common.R.inc[GMT_X] /= 2.0;
			col_inc = 2;	/* We do every other column in the pseudo grid, and must alternate odd/even columns */
			Ctrl->S.area = 1.5 * Ctrl->S.radius * Ctrl->S.radius * sind (60.0);	/* Area of a hexagon */
		}
		else {	/* Rectangular binning */
			Ctrl->S.area = GMT->common.R.inc[GMT_X] * GMT->common.R.inc[GMT_Y];	/* Area of a rectangle */
			dx2 = 0.5 * GMT->common.R.inc[GMT_X];	/* Max |distance| of Cartesian point coordinates from a bin center */
			dy2 = 0.5 * GMT->common.R.inc[GMT_Y];
			Ctrl->S.radius = dy2;	/* Fake radius to get d_col set properly */
		}
	}
	else {	/* Evaluating on a regular grid for -S */
		if (gmt_init_distaz (GMT, Ctrl->S.unit, Ctrl->S.mode, GMT_MAP_DIST) == GMT_NOT_A_VALID_TYPE)
			Return (GMT_NOT_A_VALID_TYPE);

		Ctrl->S.area = M_PI * Ctrl->S.radius * Ctrl->S.radius;
	}

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	HH = gmt_get_H_hidden (Grid->header);

	gmt_M_grd_loop (GMT, Grid, row, col, ij)	/* Initialize grid to NaN which is our flag for an unused grid node */
		Grid->data[ij] = GMT->session.d_NaN;

	/* Initialize the input since we are doing record-by-record reading */
	n_cols = (Ctrl->C.mode == GMTBINSTATS_COUNT) ? 2 : 3;	/* Expected number of input columns */
	if ((error = GMT_Set_Columns (API, GMT_IN, n_cols + Ctrl->W.active, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Grid dimensions are n_columns = %d, n_rows = %d\n", Grid->header->n_columns, Grid->header->n_rows);

	x0 = Grid->x;	/* Short-hand */
	y0 = Grid->y;

	d_col = gmt_prep_nodesearch (GMT, Grid, Ctrl->S.radius, Ctrl->S.mode, &d_row, &max_d_col);	/* Initialize d_row/d_col etc */

	/* To allow data points falling outside -R but within the search radius we extend the data domain in all directions */

	x_left = Grid->header->wesn[XLO];	x_right = Grid->header->wesn[XHI];	/* This is what -R says */
	if (gmt_M_is_cartesian (GMT, GMT_IN) || !gmt_grd_is_global (GMT, Grid->header)) {
		x_left  -= max_d_col * Grid->header->inc[GMT_X];	/* OK to extend x-domain since not a periodic geographic grid */
		x_right += max_d_col * Grid->header->inc[GMT_X];
	}
	y_top = Grid->header->wesn[YHI] + d_row * Grid->header->inc[GMT_Y];	y_bottom = Grid->header->wesn[YLO] - d_row * Grid->header->inc[GMT_Y];
	if (gmt_M_y_is_lat (GMT, GMT_IN)) {	/* For geographic grids we must ensure the extended y-domain is physically possible */
		if (y_bottom < -90.0) y_bottom = -90.0;
		if (y_top > 90.0) y_top = 90.0;
	}
	x_width = Grid->header->wesn[XHI] - Grid->header->wesn[XLO];		y_width = Grid->header->wesn[YHI] - Grid->header->wesn[YLO];
	half_x_width = 0.5 * x_width;			half_y_width = 0.5 * y_width;
	replicate_x = (HH->nxp && Grid->header->registration == GMT_GRID_NODE_REG);	/* Gridline registration has duplicate column */
	replicate_y = (HH->nyp && Grid->header->registration == GMT_GRID_NODE_REG);	/* Gridline registration has duplicate row */
	x_wrap = Grid->header->n_columns - 1;	/* Add to node index to go to right column */
	y_wrap_kk = (Grid->header->n_rows - 1) * Grid->header->n_columns;	/* Add to node index to go to bottom row if no padding */
	y_wrap_ij = (Grid->header->n_rows - 1) * Grid->header->mx;	/* Add to node index to go to bottom row if padded */
	hex_tiling = (Ctrl->T.mode == GMTBINSTATS_HEXAGONAL);
	rect_tiling = (Ctrl->T.mode == GMTBINSTATS_RECTANGULAR);
	geographic = gmt_M_x_is_lon (GMT, GMT_IN);

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		gmt_M_free (GMT, d_col);
		Return (API->error);
	}

	/* Allocate helper arrays depending on the mode */
	switch (Ctrl->C.mode) {
		case GMTBINSTATS_MEAN: case GMTBINSTATS_MEANW: case GMTBINSTATS_RMSW:	/* Will need to keep track of n or sum of weights */
			w = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat);	/* Sum of weights */
			break;
		case GMTBINSTATS_MEDIAN: case GMTBINSTATS_MODE: case GMTBINSTATS_QUANT: case GMTBINSTATS_MAD: case GMTBINSTATS_LMSSCL: case GMTBINSTATS_IRANGE:
			n_in_circle = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Number of values inside circle per node */
			n_alloc = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Allocation size per node */
			zp = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat *);	/* Matrix with pointers to an array of grdfloats */
			break;
		case GMTBINSTATS_MEDIANW: case GMTBINSTATS_MODEW: case GMTBINSTATS_QUANTW: case GMTBINSTATS_MADW: case GMTBINSTATS_LMSSCLW: case GMTBINSTATS_IRANGEW:
			n_in_circle = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Number of values inside circle per node */
			n_alloc = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Allocation size per node */
			zw_pair = gmt_M_memory (GMT, NULL, Grid->header->nm, struct GMT_OBSERVATION *);	/* Matrix with pointers to an structure array of z,w pairs */
			break;
		case GMTBINSTATS_LOWER: case GMTBINSTATS_LOWERP: case GMTBINSTATS_UPPER: case GMTBINSTATS_UPPERP:
			visited = gmt_M_memory (GMT, NULL, Grid->header->nm, char);	/* Flag so we know if we have been here */
			break;
		case GMTBINSTATS_STD:	/* Will need to keep track of n or sum of weights, plus sum of squares */
			s = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat);	/* Sum of squares */
			n_in_circle = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Number of values inside circle per node */
			break;
		case GMTBINSTATS_STDW:	/* Will need to keep track of n or sum of weights, plus sum of squares */
			n_in_circle = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Number of values inside circle per node */
			s = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat);	/* Sum of weighted squares */
			w = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat);	/* Sum of weights */
			break;
		case GMTBINSTATS_ZRANGE:	/* Will need to keep track of max */
			s = gmt_M_memory (GMT, NULL, Grid->header->nm, gmt_grdfloat);	/* For minimum value */
			visited = gmt_M_memory (GMT, NULL, Grid->header->nm, char);	/* Flag so we know if we have been here */
			break;
		case GMTBINSTATS_RMS:	/* Will need to keep track of n */
			n_in_circle = gmt_M_memory (GMT, NULL, Grid->header->nm, unsigned int);	/* Number of values inside circle per node */
			break;
		default:	/* No extra memory needed, just Grid->data */
			break;
	}

	do {	/* Keep returning records until we reach EOF */
		n_read++;
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				gmt_M_free (GMT, d_col);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}
		if (In->data == NULL) {
			gmt_M_free (GMT, d_col);
			gmt_quit_bad_record (API, In);
			Return (API->error);
		}

		in = In->data;	/* Only need to process numerical part here */

		if (n_cols == 3 && gmt_M_is_dnan (in[GMT_Z])) continue;				/* Skip if z = NaN */
		if (gmt_M_y_is_outside (GMT, in[GMT_Y], y_bottom, y_top)) continue;	/* Outside y-range */
		if (gmt_x_is_outside (GMT, &in[GMT_X], x_left, x_right)) continue;	/* Outside x-range (or longitude) */

		/* Data record to process */

		if (Ctrl->W.active) weight = (Ctrl->W.sigma) ? 1.0 / in[3] : in[3];	/* Got sigma, make weight = 1 / sigma */

		/* Find row/col indices of the node closest to this data point.  Note: These may be negative */

		col_0 = (int)gmt_M_grd_x_to_col (GMT, in[GMT_X], Grid->header);
		row_0 = (int)gmt_M_grd_y_to_row (GMT, in[GMT_Y], Grid->header);

		/* Loop over all nodes within radius of this node */

		row_end = row_0 + d_row;
		for (row = row_0 - d_row; row <= row_end; row++) {

			jj = row;
			if (gmt_y_out_of_bounds (GMT, &jj, Grid->header, &wrap_180)) continue;	/* Outside y-range.  This call must happen BEFORE gmt_x_out_of_bounds as it sets wrap_180 */
			rowu = jj;

			if (rect_tiling) {	/* No distance calculation, just dy bounds check */
				dy = in[GMT_Y] - y0[rowu];
				if (fabs (dy) > dy2) continue;	/* Outside regardless of x-coordinate */
			}
			col_stop  = col_0 + d_col[jj];
			col_start = col_0 - d_col[jj];
			if (hex_tiling) {	/* Make sure we start and stop on valid pseudo-columns so both col,row are even or odd */
				if (skip_column (row,col_start)) col_start++;
				if (skip_column (row,col_stop))  col_stop--;
			}
			for (col = col_start; col <= col_stop; col += col_inc) {

				if (hex_tiling && skip_column (row,col)) continue;	/* Skip this node in the pseudo grid */

				ii = col;
				if (gmt_x_out_of_bounds (GMT, &ii, Grid->header, wrap_180)) continue;	/* Outside x-range,  This call must happen AFTER gmt_y_out_of_bounds which sets wrap_180 */

				/* Here, (ii,jj) [both are >= 0] is index of a node (kk) inside the grid */
				colu = ii;

				if (rect_tiling) {	/* No distance calculation, just dx bounds check */
					if (geographic) {
						gmt_M_set_delta_lon (in[GMT_X], x0[colu], dx);
					}
					else
						dx = in[GMT_X] - x0[colu];
					if (fabs (dx) > dx2) continue;
				}
				else {
					distance = gmt_distance (GMT, x0[colu], y0[rowu], in[GMT_X], in[GMT_Y]);

					if (distance > Ctrl->S.radius) continue;	/* Data constraint is too far from this node */
					if (hex_tiling && outside_hexagon (GMT, x0[colu], y0[rowu], in[GMT_X], in[GMT_Y], distance, Ctrl->S.radius_inner)) continue;
					dy = in[GMT_Y] - y0[rowu];	/* since not set yet */
				}

				ij = gmt_M_ijp (Grid->header, rowu, colu);	/* Padding used for output grid */
				kk = gmt_M_ij0 (Grid->header, rowu, colu);	/* No padding used for helper arrays */
				dx = in[GMT_X] - x0[colu];	/* Redo since the wrapping will be checked again */

				/* Check for wrap-around in x or y.  This should only occur if the
				   search radius is larger than 1/2 the grid width/height so that
				   the shortest distance is going through the periodic boundary.
				   For longitudes the dx obviously cannot exceed 180 (half_x_width)
				   since we could then go the other direction instead.
				*/
				if (HH->nxp && fabs (dx) > half_x_width) dx -= copysign (x_width, dx);
				if (HH->nyp && fabs (dy) > half_y_width) dy -= copysign (y_width, dy);

				/* OK, this point should affect this node.  */

				gmtbinstats_assign_node (GMT, Grid, visited, w, s, zp, zw_pair, n_in_circle, n_alloc, ij, kk, Ctrl->C.mode, in, weight);

				/* With periodic, gridline-registered grids there are duplicate rows and/or columns
				   so we may have to assign the point to more than one node.  The next section deals
				   with this situation.
				*/

				if (replicate_x) {	/* Must check if we have to replicate a column */
					if (colu == 0) 	/* Must replicate left to right column */
						gmtbinstats_assign_node (GMT, Grid, visited, w, s, zp, zw_pair, n_in_circle, n_alloc, ij+x_wrap, kk+x_wrap, Ctrl->C.mode, in, weight);
					else if (colu == HH->nxp)	/* Must replicate right to left column */
						gmtbinstats_assign_node (GMT, Grid, visited, w, s, zp, zw_pair, n_in_circle, n_alloc, ij-x_wrap, kk-x_wrap, Ctrl->C.mode, in, weight);
				}
				if (replicate_y) {	/* Must check if we have to replicate a row */
					if (rowu == 0)	/* Must replicate top to bottom row */
						gmtbinstats_assign_node (GMT, Grid, visited, w, s, zp, zw_pair, n_in_circle, n_alloc, ij+y_wrap_ij, kk+y_wrap_kk, Ctrl->C.mode, in, weight);
					else if (rowu == HH->nyp)	/* Must replicate bottom to top row */
						gmtbinstats_assign_node (GMT, Grid, visited, w, s, zp, zw_pair, n_in_circle, n_alloc, ij-y_wrap_ij, kk-y_wrap_kk, Ctrl->C.mode, in, weight);
				}
			}
		}
		if (!(n_read % 16384)) GMT_Report (API, GMT_MSG_INFORMATION, "Processed record %10ld\r", n_read);	/* 16384 = 2^14 */
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	GMT_Report (API, GMT_MSG_INFORMATION, "Processed record %10ld\n", n_read);

	/* Finalize the statistical calculations, as needed */
	kk = 0;
	switch (Ctrl->C.mode) {
		case GMTBINSTATS_MEAN: case GMTBINSTATS_MEANW:	/* Compute [weighted] mean */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (w[kk] > 0.0) Grid->data[ij] /= w[kk], n++;
				kk++;
			}
			break;
		case GMTBINSTATS_MEDIAN:	/* Compute plain medians */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					gmt_sort_array (GMT, zp[kk], k, GMT_FLOAT);
					Grid->data[ij] = (k%2) ? zp[kk][k/2] : 0.5 * (zp[kk][(k-1)/2] + zp[kk][row/2]);
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_MEDIANW:	/* Compute weighted medians */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					Grid->data[ij] = (gmt_grdfloat)gmt_median_weighted (GMT, zw_pair[kk], n_in_circle[kk]);
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_MODE:	/* Compute plain modes */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					gmt_mode_f (GMT, zp[kk], k, k/2, true, gmt_mode_selection, &GMT_n_multiples, &zmode);
					Grid->data[ij] = (gmt_grdfloat)zmode;
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_MODEW:	/* Compute weighted modes */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					Grid->data[ij] = (gmt_grdfloat)gmt_mode_weighted (GMT, zw_pair[kk], n_in_circle[kk]);
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_QUANT:	/* Compute plain quantile */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					Grid->data[ij] = (gmt_grdfloat) gmt_quantile_f (GMT, zp[kk], Ctrl->C.quant, k);
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_QUANTW:	/* Compute weighted quantile */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					Grid->data[ij] = (gmt_grdfloat)gmt_quantile_weighted (GMT, zw_pair[kk], n_in_circle[kk], 0.01 * Ctrl->C.quant);
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_IRANGE:	/* Compute plain inter quartile range */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					Grid->data[ij] = (gmt_grdfloat) (gmt_quantile_f (GMT, zp[kk], 75.0, k) - gmt_quantile_f (GMT, zp[kk], 25.0, k));
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_IRANGEW:	/* Compute weighted quantile */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					Grid->data[ij] = (gmt_grdfloat) (gmt_quantile_weighted (GMT, zw_pair[kk], k, 0.75) - gmt_quantile_weighted (GMT, zw_pair[kk], k, 0.25));
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_COUNT: case GMTBINSTATS_COUNTW: case GMTBINSTATS_SUM:	/* Compute count or sum */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (!gmt_M_is_fnan (Grid->data[ij]) && Grid->data[ij] > 0.0) n++;
			}
			break;
		case GMTBINSTATS_ZRANGE:	/* Compute full data range */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (visited[kk]) {
					Grid->data[ij] -= s[kk];
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_STD:	/* Compute plain standard deviation */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk] > 1) {
					Grid->data[ij] = sqrt ((n_in_circle[kk] * s[kk] - Grid->data[ij] * Grid->data[ij]) / (n_in_circle[kk] * (n_in_circle[kk] - 1)));
					n++;
				}
				else if (n_in_circle[kk])	/* Finding only one value means std is undefined */
					Grid->data[ij] = Ctrl->E.value;
				kk++;
			}
			break;
		case GMTBINSTATS_STDW:	/* Compute weighted standard deviation */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk] > 1) {
					Grid->data[ij] = sqrt ((w[kk] * s[kk] - Grid->data[ij] * Grid->data[ij]) / ((n_in_circle[kk] - 1) * w[kk] * w[kk] /n_in_circle[kk]));
					n++;
				}
				else if (n_in_circle[kk])	/* Finding only one value means std is undefined */
					Grid->data[ij] = Ctrl->E.value;
				kk++;
			}
			break;
		case GMTBINSTATS_RMS:	/* Compute plain rms */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					Grid->data[ij] = sqrt (Grid->data[ij] / n_in_circle[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_RMSW:	/* Compute weighted rms */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (w[kk] > 0.0) {
					Grid->data[ij] = sqrt (Grid->data[ij] / w[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_MAD:	/* Compute plain MAD */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					gmt_sort_array (GMT, zp[kk], k, GMT_FLOAT);
					median = (k%2) ? zp[kk][k/2] : 0.5 * (zp[kk][(k-1)/2] + zp[kk][row/2]);
					gmt_getmad_f (GMT, zp[kk], k, median, &mad);
					Grid->data[ij] = mad;
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_MADW:	/* Compute weighted MAD */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					median = gmt_median_weighted (GMT, zw_pair[kk], n_in_circle[kk]);
					/* Compute the absolute deviations from this median */
					for (k = 0; k < n_in_circle[kk]; k++) zw_pair[kk][k].value = (gmt_grdfloat)fabs (zw_pair[kk][k].value - median);
					/* Find the weighted median absolute deviation */
					Grid->data[ij] = (gmt_grdfloat)gmt_median_weighted (GMT, zw_pair[kk], n_in_circle[kk]);
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_LMSSCL:	/* Compute plain mode lmsscale */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if ((k = n_in_circle[kk])) {
					gmt_mode_f (GMT, zp[kk], k, k/2, true, gmt_mode_selection, &GMT_n_multiples, &zmode);
					gmt_getmad_f (GMT, zp[kk], k, zmode, &mad);
					Grid->data[ij] = (gmt_grdfloat)mad;
					gmt_M_free (GMT, zp[kk]);
					n++;
				}
				kk++;
			}
			break;
		case GMTBINSTATS_LMSSCLW:	/* Compute weighted modes */
			gmt_M_grd_loop (GMT, Grid, row, col, ij) {
				if (n_in_circle[kk]) {
					zmode = gmt_mode_weighted (GMT, zw_pair[kk], n_in_circle[kk]);
					for (k = 0; k < n_in_circle[kk]; k++) zw_pair[kk][k].value = (gmt_grdfloat)fabs (zw_pair[kk][k].value - zmode);
					Grid->data[ij] = (gmt_grdfloat)(MAD_NORMALIZE * gmt_median_weighted (GMT, zw_pair[kk], n_in_circle[kk]));
					gmt_M_free (GMT, zw_pair[kk]);
					n++;
				}
				kk++;
			}
			break;
		default:	/* Count the visits for the lower, upper operators */
			for (kk = 0; kk < Grid->header->nm; kk++)
				if (visited[kk]) n++;
			break;
	}

	if (Ctrl->N.active) {	/* Area normalization */
		double scale = 1.0 / Ctrl->S.area;
		gmt_M_grd_loop (GMT, Grid, row, col, ij)
			Grid->data[ij] *= scale;
	}

	gmt_M_free (GMT, visited);
	gmt_M_free (GMT, w);
	gmt_M_free (GMT, s);
	gmt_M_free (GMT, zp);
	gmt_M_free (GMT, zw_pair);
	gmt_M_free (GMT, n_in_circle);
	gmt_M_free (GMT, n_alloc);
	gmt_M_free (GMT, d_col);

	if (hex_tiling) {	/* Write the hexagonal results to stdout or file*/
		uint64_t dim[4] = {1, 1, n, 3}, k = 0;
		struct GMT_DATASET *D = NULL;
		struct GMT_DATASEGMENT *S = NULL;
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POINT, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to allocate a dataset\n");
			Return (error);
		}
		if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, D)) {
			Return (API->error);
		}
		S = D->table[0]->segment[0];	/* Only a single segment will be written */
		gmt_M_row_loop (GMT, Grid, row) {
			gmt_M_col_loop (GMT, Grid, row, col, ij) {
				if (skip_column(row,col)) continue;	/* Unused pseudo=grid node */
				if (gmt_M_is_fnan (Grid->data[ij])) continue;	/* Unvisited node */
				S->data[GMT_X][k] = x0[col];
				S->data[GMT_Y][k] = y0[row];
				S->data[GMT_Z][k] = Grid->data[ij];
				k++;
			}
		}
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_WRITE_NORMAL, NULL, Ctrl->G.file, D) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	else {	/* Write a grid */
		if (!gmt_M_is_fnan (Ctrl->E.value)) {	/* Replace NaNs with the -E value */
			gmt_M_grd_loop (GMT, Grid, row, col, ij)
				if (gmt_M_is_fnan (Grid->data[ij])) Grid->data[ij] = Ctrl->E.value;
		}
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) {
			Return (API->error);
		}

		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		char line[GMT_BUFSIZ];
		sprintf (line, "%s)\n", GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " nodes were assigned a statistical value\n", n);
		(gmt_M_is_dnan (Ctrl->E.value)) ? GMT_Message (API, GMT_TIME_NONE, "NaN)\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->E.value);
	}

	Return (GMT_NOERROR);
}
