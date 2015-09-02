/*--------------------------------------------------------------------
 *    $Id$
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
 * API functions to support the blockmode application.
 *
 * Author:	Walter H.F. Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: reads records of x, y, data, [weight] and writes out mode
 * value per cell, where cellular region is bounded by West East South North
 * and cell dimensions are delta_x, delta_y.
 */

#define BLOCKMODE	/* Since mean, median, mode share near-similar macros we require this setting */

#define THIS_MODULE_NAME	"blockmode"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Block average (x,y,z) data tables by mode estimation"
#define THIS_MODULE_KEYS	"<DI,>DO,RG!"

#include "gmt_dev.h"
#include "block_subs.h"

#define GMT_PROG_OPTIONS "-:>RVabdfghior" GMT_OPT("FH")

struct BIN_MODE_INFO {	/* Used for histogram binning */
	double width;		/* The binning width used */
	double i_offset;	/* 0.5 if we are to bin using the center the bins on multiples of width, else 0.0 */
	double o_offset;	/* 0.0 if we are to report center the bins on multiples of width, else 0.5 */
	double i_width;		/* 1/width, to avoid divisions later */
	int min, max;		/* The raw min,max bin numbers (min can be negative) */
	unsigned int n_bins;	/* Number of bins required */
	double *count;		/* The histogram counts (double to accomodate weighted data), to be reset before each spatial block */
	int mode_choice;	/* For multiple modes: BLOCKMODE_LOW picks lowest, BLOCKMODE_AVE picks average, BLOCKMODE_HIGH picks highest */
};

int GMT_blockmode_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: blockmode [<table>] %s\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-C] [-D<width>[+c][+l|h]] [-E] [-Er|s[-]] [-Q]\n\t[%s] [-W[i][o]] [%s] [%s] [%s]\n\t[%s] [%s]\nt\t[%s] [%s] [%s] [%s]\n\n",
		GMT_Rgeo_OPT, GMT_V_OPT, GMT_a_OPT, GMT_b_OPT, GMT_d_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Option (API, "I,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Output center of block and mode z-value [Default is mode location (but see -Q)].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Compute modes via binning using <width>; append +c to center bins. If there are multiple\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   modes we return the average mode [+a]; append +l or +h to pick the low or high mode instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be combined with -E and implicitly sets -Q.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If your data are integers and <width> is not given we default to -D1+c+l\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default computes the mode as the Least Median of Squares (LMS) estimate].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Extend output with LMS scale (s), low (l), and high (h) value per block, i.e.,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   output (x,y,z,s,l,h[,w]) [Default outputs (x,y,z[,w])]; see -W regarding w.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Er to report record number of the median value per block,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or -Es to report an unsigned integer source id (sid) taken from the x,y,z[,w],sid input.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For ties, report record number (or sid) of largest value; append - for smallest.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Quicker; get mode z and mean x,y [Default gets mode x, mode y, mode z].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set Weight options.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wi reads Weighted Input (4 cols: x,y,z,w) but writes only (x,y,z[,s,l,h]) Output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wo reads unWeighted Input (3 cols: x,y,z) but reports sum (x,y,z[,s,l,h],w) Output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -W with no modifier has both weighted Input and Output; Default is no weights used.\n");
	GMT_Option (API, "a,bi");
	GMT_Message (API, GMT_TIME_NONE, "\t    Default is 3 columns (or 4 if -W is set).\n");
	GMT_Option (API, "bo,d,f,h,i,o,r,:,.");

	return (EXIT_FAILURE);
}

int GMT_blockmode_parse (struct GMT_CTRL *GMT, struct BLOCKMODE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to blockmode and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos = 0;
	struct GMT_OPTION *opt = NULL;
	char p[GMT_BUFSIZ] = {""}, *c = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Report center of block instead */
				Ctrl->C.active = true;
				break;
			case 'D':	/* Histogram mode estimate */
				Ctrl->D.active = true;
				Ctrl->D.width = atof (opt->arg);
				if ((c = strchr (opt->arg, '+'))) {	/* Found modifiers */
					while ((GMT_strtok (c, "+", &pos, p))) {
						switch (p[0]) {
							case 'c': Ctrl->D.center = true; break;	/* Center the histogram */
							case 'a': Ctrl->D.mode = BLOCKMODE_AVE; break;	/* Pick average mode */
							case 'l': Ctrl->D.mode = BLOCKMODE_LOW; break;	/* Pick low mode */
							case 'h': Ctrl->D.mode = BLOCKMODE_HIGH; break;	/* Pick high mode */
							default:	/* Bad modifier */
								GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Unrecognized modifier +%c.\n", p[0]);
								n_errors++;
								break;
						}
					}
				}
				break;
			case 'E':
				Ctrl->E.active = true;		/* Extended report with standard deviation, min, and max in cols 4-6 */
				if (opt->arg[0] == 'r' || opt->arg[0] == 's') {
					Ctrl->E.mode = (opt->arg[1] == '-') ? BLK_DO_INDEX_LO : BLK_DO_INDEX_HI;	/* Report row number or sid of median */
					if (opt->arg[0] == 's') /* report sid */
						Ctrl->E.mode |= BLK_DO_SRC_ID;
				}
				else if (opt->arg[0] == '\0')
					Ctrl->E.mode = BLK_DO_EXTEND3;		/* Report LMSscale, low, high in cols 4-6 */
				else
					n_errors++;
				break;
			case 'I':	/* Get block dimensions */
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'Q':	/* Quick mode for modal z */
				Ctrl->Q.active = true;
				break;
			case 'W':	/* Use in|out weights */
				Ctrl->W.active = true;
				switch (opt->arg[0]) {
					case '\0':
						Ctrl->W.weighted[GMT_IN] = Ctrl->W.weighted[GMT_OUT] = true; break;
					case 'i': case 'I':
						Ctrl->W.weighted[GMT_IN] = true; break;
					case 'o': case 'O':
						Ctrl->W.weighted[GMT_OUT] = true; break;
					default:
						n_errors++; break;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);	/* If -R<grdfile> was given we may get incs unless -I was used */

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && Ctrl->E.active, "Syntax error -D option: Cannot be combined with -E\n");
	n_errors += GMT_check_binary_io (GMT, (Ctrl->W.weighted[GMT_IN]) ? 4 : 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

struct BIN_MODE_INFO *bin_setup (struct GMT_CTRL *GMT, struct BLK_DATA *d, double width, bool center, int mode_choice, bool is_integer, uint64_t n, uint64_t k)
{
	/* Estimate mode by finding a maximum in the histogram resulting
	 * from binning the data with the specified width. Note that the
	 * data array is already sorted on a[k]. We check if we find more
	 * than one mode and return the chosen one as per the settings.
	 * This function sets up quantities needed as we loop over the
	 * spatial bins */

	struct BIN_MODE_INFO *B = GMT_memory (GMT, NULL, 1, struct BIN_MODE_INFO);
	char *mode = "lah";

	if (is_integer) {	/* Special consideration for integers */
		double d_intval;
		if (width == 0.0) {
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "For integer data and no -D<width> specified we default to <width> = 1\n");
			width = 1.0;
		}
		d_intval = (double)lrint (width);
		if (doubleAlmostEqual (d_intval, width)) {
			GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "For integer data and integer width we automatically select centered bins\n");
			center = true;
			if (mode_choice == BLOCKMODE_DEF) {
				GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "For integer data and integer width we automatically select lowest mode\n");
				mode_choice = BLOCKMODE_LOW;
			}
		}
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Effective mode option is -D%g+c+%c\n", width, mode[mode_choice+1]);
	}
	B->i_offset = (center) ? 0.5 : 0.0;
	B->o_offset = (center) ? 0.0 : 0.5;
	B->width = width;
	B->i_width = 1.0 / width;
	B->min = irint (floor ((d[0].a[k]   * B->i_width) + B->i_offset));
	B->max = irint (ceil  ((d[n-1].a[k] * B->i_width) + B->i_offset));
	B->n_bins = B->max - B->min + 1;
	B->count = GMT_memory (GMT, NULL, B->n_bins, double);
	B->mode_choice = (mode_choice == BLOCKMODE_DEF) ? BLOCKMODE_AVE : mode_choice;

	return (B);
}

double bin_mode (struct GMT_CTRL *GMT, struct BLK_DATA *d, uint64_t n, uint64_t k, struct BIN_MODE_INFO *B)
{
	/* Estimate mode by finding a maximum in the histogram resulting
	 * from binning the data with the specified width. Note that the
	 * data array is already sorted on a[k]. We check if we find more
	 * than one mode and return the chosen one as per the settings. */

	double value = 0.0, mode_count = 0.0;
	uint64_t i;
	unsigned int n_modes = 0;
	int bin, mode_bin = 0;
	bool done;
	GMT_UNUSED(GMT);

	GMT_memset (B->count, B->n_bins, double);	/* Reset the counts */
	for (i = 0; i < n; i++) {	/* Loop over sorted data points */
		bin = urint (floor ((d[i].a[k] * B->i_width) + B->i_offset)) - B->min;
		B->count[bin] += d[i].a[BLK_W];		/* Add up counts or weights */
		if (B->count[bin] > mode_count) {	/* New max count value; make a note */
			mode_count = B->count[bin];	/* Highest count so far... */
			mode_bin = bin;			/* ...occuring for this bin */
			n_modes = 1;			/* Only one of these so far */
		}
		else if (doubleAlmostEqual (B->count[bin], mode_count)) n_modes++;	/* Bin has same peak as previous best mode; increase mode count */
	}
	if (n_modes == 1) {	/* Single mode; we are done */
		value = ((mode_bin + B->min) + B->o_offset) * B->width;
		return (value);
	}

	/* Here we found more than one mode and must choose according to settings */

	for (bin = 0, done = false; !done && bin < (int)B->n_bins; bin++) {	/* Loop over bin counts */
		if (B->count[bin] < mode_count) continue;	/* Not one of the modes */
		switch (B->mode_choice) {
			case BLOCKMODE_LOW:	/* Pick lowest mode; we are done */
				value = ((bin + B->min) + B->o_offset) * B->width;
				done = true;
				break;
			case BLOCKMODE_AVE:		/* Get the average of the modes */
				value += ((bin + B->min) + B->o_offset) * B->width;
				break;
			case BLOCKMODE_HIGH:	/* Update highest mode so far, when loop exits we have the hightest mode */
			 	value = ((bin + B->min) + B->o_offset) * B->width;
				break;
		}
	}
	if (B->mode_choice == BLOCKMODE_AVE) value /= n_modes;	/* The average of the multiple modes */

	return (value);
}

#if 0
double weighted_mode (struct BLK_DATA *d, double wsum, unsigned int emode, uint64_t n, unsigned int k, uint64_t *index)
{
	/* Estimate mode by finding a maximum in the estimated
	   pdf of weighted data.  Estimate the pdf as the finite
	   difference of the cumulative frequency distribution
	   over points from i to j.  This has the form top/bottom,
	   where top is the sum of the weights from i to j, and
	   bottom is (data[j] - data[i]).  Strategy is to start
	   with i=0, j=n-1, and then move i or j toward middle
	   while j-i > n/2 and bottom > 0.  At end while, midpoint
	   of range from i to j is the mode estimate.  Choose
	   to move either i or j depending on which one will
	   cause greatest increase in pdf estimate.  If a tie,
	   move both.

	   Strictly, the pdf estimated this way would need to be
	   scaled by (1/wsum), but this is constant so we don't
	   use it here, as we are seeking a relative minimum.

	   I assumed n > 2 when I wrote this. [WHFS] */

	double top, topj, topi, bottomj, bottomi, pj, pi;
	int64_t i = 0, j = n - 1, nh = n / 2;
	int way;

	top = wsum;

	while ((j-i) > nh) {
		topi = top - d[i].a[BLK_W];
		topj = top - d[j].a[BLK_W];
		bottomi = d[j].a[k] - d[i+1].a[k];
		bottomj = d[j-1].a[k] - d[i].a[k];

		if (bottomj == 0.0) {
			if (index) *index = d[j-1].src_id;
			return (d[j-1].a[k]);
		}
		if (bottomi == 0.0) {
			if (index) *index = d[i+1].src_id;
			return (d[i+1].a[k]);
		}
		pi = topi / bottomi;
		pj = topj / bottomj;
		if (pi > pj) {
			i++;
			top = topi;
		}
		else if (pi < pj) {
			j--;
			top = topj;
		}
		else {
			top -= (d[i].a[BLK_W] + d[j].a[BLK_W]);
			i++;
			j--;
		}
	}
	if (emode && index) {	/* Also return best src_id for this mode */
		way = (d[j].a[k] >= d[i].a[k]) ? +1 : -1;
		if (emode & BLK_DO_INDEX_HI) *index = (way == +1) ? d[j].src_id : d[i].src_id;
		else *index = (way == +1) ? d[i].src_id : d[j].src_id;
	}
	return (0.5 * (d[j].a[k] + d[i].a[k]));
}
#endif

double weighted_mode (struct BLK_DATA *d, double wsum, unsigned int emode, uint64_t n, unsigned int k, uint64_t *index)
{
	/* Looks for the "shortest 50%". This means that when the cumulative weight
	   (y) is plotted against the value (x) then the line between (xi,yi) and
	   (xj,yj) should be the steepest for any combination where (yj-yi) is 50%
	   of the total sum of weights */

	double top, bottom, p, p_max, mode;
	uint64_t i, j, src = 0;


	/* Do some initializations */
	wsum = 0.5 * wsum; /* Sets the 50% range */
	/* First check if any single point has 50% or more of the total weights; if so we are done */
	for (i = 0; i < n; i++) if (d[i].a[BLK_W] >= wsum) {
		if (index) *index = d[i].src_id;
		return d[i].a[k];
	}
	top = p_max = 0.0;
	mode = 0.5 * (d[0].a[k] + d[n-1].a[k]);
	if (index) src = (emode & BLK_DO_INDEX_HI) ? d[n-1].src_id : d[0].src_id;

	for (i = j = 0; j < n; j++) {
		top += d[j].a[BLK_W];
		if (top < wsum) continue;
		while (top > wsum && i < j) top -= d[i++].a[BLK_W];
		bottom = d[j].a[k] - d[i].a[k];

		/* If all is comprised in one point or if a lot of values are the same,
		   then we have a spike. Maybe another logic is needed to handle
		   multiple spikes in the data */
		if (bottom == 0.0) {
			if (index) *index = d[i].src_id;
			return (d[i].a[k]);
		}

		p = top / bottom;
		if (p > p_max) {
			p_max = p;
			mode = 0.5 * (d[i].a[k] + d[j].a[k]);
			//n_modes = 1;
			if (index) src = (emode & BLK_DO_INDEX_HI) ? d[j].src_id : d[i].src_id;
		}
#if 0
		else if (doubleAlmostEqual (p, p_max)) {	/* Same peak as previous best mode */
			double new_mode = 0.5 * (d[i].a[k] + d[j].a[k]);
			uint64_t n_modes = 1;
			switch (mode_type) {
				case BLOCKMODE_LOW:	/* Pick lowest mode */
					if (new_mode < mode) {
						mode = new_mode;
						if (index) src = (emode & BLK_DO_INDEX_HI) ? d[j].src_id : d[i].src_id;
					}
					break;
				case BLOCKMODE_AVE:		/* Get the average of the modes, leave src to be first mode */
					mode += new_mode;
					n_modes++;
					break;
				case BLOCKMODE_HIGH:	/* Pick highest mode */
					if (new_mode > mode) {
						mode = new_mode;
						if (index) src = (emode & BLK_DO_INDEX_HI) ? d[j].src_id : d[i].src_id;
					}
					break;
			}
		}
	}
	if (n_modes > 1) mode /= n_modes;
#else
	}
#endif
	if (emode && index) *index = src;
	return (mode);
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {GMT_Destroy_Data (API, &Grid); Free_blockmode_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_blockmode (void *V_API, int mode, void *args)
{
	bool mode_xy, do_extra = false, is_integer, duplicate_col;

	int way, error = 0;

	unsigned int row, col, emode = 0, n_input, n_output;

	uint64_t node, first_in_cell, first_in_new_cell, n_lost, n_read;
	uint64_t n_cells_filled, n_in_cell, nz, n_pitched, src_id;
	uint64_t w_col, i_col = 0, sid_col;

	size_t n_alloc = 0, nz_alloc = 0;

	double out[7], wesn[4], i_n_in_cell, d_intval, weight, half_dx, *in = NULL, *z_tmp = NULL;

	char format[GMT_BUFSIZ] = {""}, *old_format = NULL;

	struct GMT_OPTION *options = NULL;
	struct GMT_GRID *Grid = NULL;
	struct BIN_MODE_INFO *B = NULL;
	struct BLK_DATA *data = NULL;
	struct BLOCKMODE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_blockmode_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);
	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_blockmode_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_blockmode_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_blockmode_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_blockmode_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the blockmode main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");

	if (Ctrl->C.active && Ctrl->Q.active) {
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: -C overrides -Q\n");
		Ctrl->Q.active = false;
	}

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, NULL, Ctrl->I.inc, \
		GMT_GRID_DEFAULT_REG, 0, NULL)) == NULL) Return (API->error);	/* Note: 0 for pad since no BC work needed */

	duplicate_col = (GMT_360_RANGE (Grid->header->wesn[XLO], Grid->header->wesn[XHI]) && Grid->header->registration == GMT_GRID_NODE_REG);	/* E.g., lon = 0 column should match lon = 360 column */
	half_dx = 0.5 * Grid->header->inc[GMT_X];
	mode_xy = !Ctrl->C.active;

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		sprintf (format, "W: %s E: %s S: %s N: %s nx: %%d ny: %%d\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, format, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->nx, Grid->header->ny);
	}

	GMT_set_xy_domain (GMT, wesn, Grid->header);	/* May include some padding if gridline-registered */

	/* Specify input and output expected columns */
	n_input = 3 + Ctrl->W.weighted[GMT_IN] + ((Ctrl->E.mode & BLK_DO_SRC_ID) ? 1 : 0);	/* 3 columns on output, plus 1 extra if -W and another if -Es  */
	if ((error = GMT_set_cols (GMT, GMT_IN, n_input)) != GMT_OK) {
		Return (error);
	}
	n_output = (Ctrl->W.weighted[GMT_OUT]) ? 4 : 3;
	if (Ctrl->E.mode & BLK_DO_EXTEND3) {
		n_output += 3;
		do_extra = true;
	}
	if (Ctrl->E.mode & BLK_DO_INDEX_LO || Ctrl->E.mode & BLK_DO_INDEX_HI) {	/* Add index */
		n_output++;
		emode = Ctrl->E.mode & (BLK_DO_INDEX_LO + BLK_DO_INDEX_HI);
	}
	if ((error = GMT_set_cols (GMT, GMT_OUT, n_output)) != GMT_OK) {
		Return (error);
	}

	/* Register likely data sources unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}

	/* Initialize the i/o for doing record-by-record reading/writing */
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	sid_col = (Ctrl->W.weighted[GMT_IN]) ? 4 : 3;	/* Column with integer source id [if -Es is set] */
	n_read = n_pitched = 0;	/* Initialize counters */

	GMT->session.min_meminc = GMT_INITIAL_MEM_ROW_ALLOC;	/* Start by allocating a 32 Mb chunk */

	/* Read the input data */
	is_integer = true;	/* Until proven otherwise */

	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		if (GMT_is_dnan (in[GMT_Z])) 		/* Skip if z = NaN */
			continue;

		/* Data record to process */

		n_read++;						/* Number of records read */

		if (GMT_y_is_outside (GMT, in[GMT_Y], wesn[YLO], wesn[YHI])) continue;	/* Outside y-range */
		if (GMT_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;	/* Outside x-range (or longitude) */

		/* We appear to be inside: Get row and col indices of this block */

		if (GMT_row_col_out_of_bounds (GMT, in, Grid->header, &row, &col)) continue;	/* Sorry, outside after all */
		if (duplicate_col && (wesn[XHI]-in[GMT_X] < half_dx)) {	/* Only compute modal values for the west column and not the repeating east column with lon += 360 */
			in[GMT_X] -= 360.0;	/* Make this point be considered for the western block mean value */
			col = 0;
		}

		/* OK, this point is definitively inside and will be used */

		if (is_integer) {	/* Determine if we still only have integers */
			d_intval = (double) lrint (in[GMT_Z]);
			if (!doubleAlmostEqual (d_intval, in[GMT_Z])) is_integer = false;
		}

		node = GMT_IJP (Grid->header, row, col);		/* Bin node */

		if (n_pitched == n_alloc) data = GMT_malloc (GMT, data, n_pitched, &n_alloc, struct BLK_DATA);
		data[n_pitched].ij = node;
		data[n_pitched].src_id = (Ctrl->E.mode & BLK_DO_SRC_ID) ? (uint64_t)lrint (in[sid_col]) : n_read;
		if (mode_xy) {	/* Need to store (x,y) so we can compute modal location later */
			data[n_pitched].a[GMT_X] = in[GMT_X];
			data[n_pitched].a[GMT_Y] = in[GMT_Y];
		}
		data[n_pitched].a[BLK_Z] = in[GMT_Z];
		data[n_pitched].a[BLK_W] = (Ctrl->W.weighted[GMT_IN]) ? in[3] : 1.0;

		n_pitched++;
	} while (true);

	GMT->session.min_meminc = GMT_MIN_MEMINC;		/* Reset to the default value */

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	if (n_read == 0) {	/* Blank/empty input files */
		GMT_Report (API, GMT_MSG_VERBOSE, "No data records found; no output produced");
		Return (EXIT_SUCCESS);
	}
	if (n_pitched == 0) {	/* No points inside region */
		GMT_Report (API, GMT_MSG_VERBOSE, "No data points found inside the region; no output produced");
		Return (EXIT_SUCCESS);
	}
	if (Ctrl->D.active && Ctrl->D.width == 0.0 && !is_integer) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error -D: No bin width specified and data are not integers\n");
		Return (EXIT_FAILURE);
	}
	if (n_pitched < n_alloc) {
		n_alloc = n_pitched;
		data = GMT_malloc (GMT, data, 0, &n_alloc, struct BLK_DATA);
	}

	/* Ready to go. */

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}

	w_col = GMT_get_cols (GMT, GMT_OUT) - 1;	/* Weights always reported in last output column */
	if (emode) {					/* Index column last, with weight col just before */
		i_col = w_col--;
		old_format = GMT->current.io.o_format[i_col];		/* Need to restore this at end */
		GMT->current.io.o_format[i_col] = strdup ("%.0f");	/* Integer format for src_id */
	}

	/* Sort on node and Z value */

	qsort (data, n_pitched, sizeof (struct BLK_DATA), BLK_compare_index_z);

	if (Ctrl->D.active) {	/* Choose to compute unweighted modes by histogram binning */
		B = bin_setup (GMT, data, Ctrl->D.width, Ctrl->D.center, Ctrl->D.mode, is_integer, n_pitched, GMT_Z);
		Ctrl->Q.active = true;	/* Cannot do modal positions */
	}

	/* Find n_in_cell and write appropriate output  */

	first_in_cell = n_cells_filled = nz = 0;
	while (first_in_cell < n_pitched) {
		weight = data[first_in_cell].a[BLK_W];
		if (do_extra) {
			if (nz == nz_alloc) z_tmp = GMT_malloc (GMT, z_tmp, nz, &nz_alloc, double);
			z_tmp[0] = data[first_in_cell].a[BLK_Z];
			nz = 1;
		}
		if (Ctrl->C.active) {	/* Use block center */
			row = (unsigned int)GMT_row (Grid->header, data[first_in_cell].ij);
			col = (unsigned int)GMT_col (Grid->header, data[first_in_cell].ij);
			out[GMT_X] = GMT_grd_col_to_x (GMT, col, Grid->header);
			out[GMT_Y] = GMT_grd_row_to_y (GMT, row, Grid->header);
		}
		else {
			out[GMT_X] = data[first_in_cell].a[GMT_X];
			out[GMT_Y] = data[first_in_cell].a[GMT_Y];
		}
		first_in_new_cell = first_in_cell + 1;
		while ((first_in_new_cell < n_pitched) && (data[first_in_new_cell].ij == data[first_in_cell].ij)) {
			weight += data[first_in_new_cell].a[BLK_W];	/* Summing up weights */
			if (mode_xy) {
				out[GMT_X] += data[first_in_new_cell].a[GMT_X];
				out[GMT_Y] += data[first_in_new_cell].a[GMT_Y];
			}
			if (do_extra) {	/* Must get a temporary copy of the sorted z array */
				if (nz == nz_alloc) z_tmp = GMT_malloc (GMT, z_tmp, nz, &nz_alloc, double);
				z_tmp[nz] = data[first_in_new_cell].a[BLK_Z];
				nz++;
			}
			first_in_new_cell++;
		}
		n_in_cell = first_in_new_cell - first_in_cell;
		if (n_in_cell > 2) {	/* data are already sorted on z; get z mode  */
			if (Ctrl->D.active)
				out[GMT_Z] = bin_mode (GMT, &data[first_in_cell], n_in_cell, GMT_Z, B);
			else
				out[GMT_Z] = weighted_mode (&data[first_in_cell], weight, emode, n_in_cell, GMT_Z, &src_id);
			if (Ctrl->Q.active) {
				i_n_in_cell = 1.0 / n_in_cell;
				out[GMT_X] *= i_n_in_cell;
				out[GMT_Y] *= i_n_in_cell;
			}
			else if (mode_xy) {
				qsort (&data[first_in_cell], n_in_cell, sizeof (struct BLK_DATA), BLK_compare_x);
				out[GMT_X] = weighted_mode (&data[first_in_cell], weight, emode, n_in_cell, GMT_X, NULL);

				qsort (&data[first_in_cell], n_in_cell, sizeof (struct BLK_DATA), BLK_compare_y);
				out[GMT_Y] = weighted_mode (&data[first_in_cell], weight, emode, n_in_cell, GMT_Y, NULL);
			}
		}
		else if (n_in_cell == 2) {
			if (Ctrl->D.active) {
				out[GMT_Z] = bin_mode (GMT, &data[first_in_cell], n_in_cell, GMT_Z, B);
				if (Ctrl->Q.active) {
					out[GMT_X] *= 0.5;
					out[GMT_Y] *= 0.5;
				}
				else if (mode_xy) {
					out[GMT_X] = data[first_in_cell].a[GMT_X];
					out[GMT_Y] = data[first_in_cell].a[GMT_Y];
				}
			}
			else if (data[first_in_cell].a[BLK_W] > data[first_in_cell+1].a[BLK_W]) {
				out[GMT_Z] = data[first_in_cell].a[BLK_Z];
				if (Ctrl->Q.active) {
					out[GMT_X] *= 0.5;
					out[GMT_Y] *= 0.5;
				}
				else if (mode_xy) {
					out[GMT_X] = data[first_in_cell].a[GMT_X];
					out[GMT_Y] = data[first_in_cell].a[GMT_Y];
				}
				if (emode) src_id = data[first_in_cell].src_id;
			}
			else if (data[first_in_cell].a[BLK_W] < data[first_in_cell+1].a[BLK_W]) {
				out[GMT_Z] = data[first_in_cell+1].a[BLK_Z];
				if (Ctrl->Q.active) {
					out[GMT_X] *= 0.5;
					out[GMT_Y] *= 0.5;
				}
				else if (mode_xy) {
					out[GMT_X] = data[first_in_cell+1].a[GMT_X];
					out[GMT_Y] = data[first_in_cell+1].a[GMT_Y];
				}
				if (emode) src_id = data[first_in_cell+1].src_id;
			}
			else {
				if (mode_xy) {	/* Need average location */
					out[GMT_X] *= 0.5;
					out[GMT_Y] *= 0.5;
				}
				out[GMT_Z] = 0.5 * (data[first_in_cell].a[BLK_Z] + data[first_in_cell+1].a[BLK_Z]);
				if (emode) {
					way = (data[first_in_cell+1].a[BLK_Z] >= data[first_in_cell].a[BLK_Z]) ? +1 : -1;
					if (emode & BLK_DO_INDEX_HI) src_id = (way == +1) ? data[first_in_cell+1].src_id : data[first_in_cell].src_id;
					else src_id = (way == +1) ? data[first_in_cell].src_id : data[first_in_cell+1].src_id;
				}
			}
		}
		else {
			out[GMT_Z] = data[first_in_cell].a[BLK_Z];
			if (emode) src_id = data[first_in_cell].src_id;
		}

		if (Ctrl->E.mode & BLK_DO_EXTEND3) {
			out[4] = z_tmp[0];	/* Low value */
			out[5] = z_tmp[nz-1];	/* High value */
			/* Turn z_tmp into absolute deviations from the mode (out[GMT_Z]) */
			if (nz > 1) {
				for (node = 0; node < nz; node++) z_tmp[node] = fabs (z_tmp[node] - out[GMT_Z]);
				GMT_sort_array (GMT, z_tmp, nz, GMT_DOUBLE);
				out[3] = (nz%2) ? z_tmp[nz/2] : 0.5 * (z_tmp[(nz-1)/2] + z_tmp[nz/2]);
				out[3] *= 1.4826;	/* This will be LMS MAD-based scale */
			}
			else
				out[3] = GMT->session.d_NaN;
		}
		if (Ctrl->W.weighted[GMT_OUT]) out[w_col] = weight;
		if (emode) out[i_col] = (double)src_id;

		GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */

		n_cells_filled++;
		first_in_cell = first_in_new_cell;
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	n_lost = n_read - n_pitched;	/* Number of points that did not get used */
	GMT_Report (API, GMT_MSG_VERBOSE, "N read: %" PRIu64 " N used: %" PRIu64 " outside_area: %" PRIu64 " N cells filled: %" PRIu64 "\n", n_read, n_pitched, n_lost, n_cells_filled);

	GMT_free (GMT, data);
	if (do_extra) GMT_free (GMT, z_tmp);

	if (emode) {
		free (GMT->current.io.o_format[i_col]);		/* Free the temporary integer format */
		GMT->current.io.o_format[i_col] = old_format;	/* Restore previous format */
	}
	if (Ctrl->D.active) {	/* Free histogram binning machinery */
		GMT_free (GMT, B->count);
		GMT_free (GMT, B);
	}

	Return (GMT_OK);
}
