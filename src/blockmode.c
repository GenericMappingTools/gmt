/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * API functions to support the blockmode application.
 *
 * Author:	Walter H.F. Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: reads records of x, y, data, [weight] and writes out mode
 * value per cell, where cellular region is bounded by West East South North
 * and cell dimensions are delta_x, delta_y.
 */

#define BLOCKMODE	/* Since mean, median, mode share near-similar macros we require this setting */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"blockmode"
#define THIS_MODULE_MODERN_NAME	"blockmode"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Block average (x,y,z) data tables by mode estimation"
#define THIS_MODULE_KEYS	"<D{,>D},GG),A->"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:>RVabdefghioqr" GMT_OPT("FH")

#include "block_subs.h"

struct BIN_MODE_INFO {	/* Used for histogram binning */
	double width;		/* The binning width used */
	double i_offset;	/* 0.5 if we are to bin using the center the bins on multiples of width, else 0.0 */
	double o_offset;	/* 0.0 if we are to report center the bins on multiples of width, else 0.5 */
	double i_width;		/* 1/width, to avoid divisions later */
	double *count;		/* The histogram counts (double to accommodate weighted data), to be reset before each spatial block */
	int min, max;		/* The raw min,max bin numbers (min can be negative) */
	int mode_choice;	/* For multiple modes: BLOCKMODE_LOW picks lowest, BLOCKMODE_AVE picks average, BLOCKMODE_HIGH picks highest */
	unsigned int n_bins;/* Number of bins required */
};

/* Note: For external calls to block* we do not allow explicit -G options; these should be added by examining -A which
 * is required for external calls to make grids, even if just z is requested.  This differs from the command line where
 * -Az is the default and -G is required to set file name format.  */

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s %s\n", name, GMT_I_OPT, GMT_Rgeo_OPT);
	if (API->external)
		GMT_Message (API, GMT_TIME_NONE, "\t[-A<fields>] [-C] [-D<width>[+c][+l|h]] [-E] [-Er|s[+l|h]] [-Q] [-W[i][o][+s]]\n", GMT_V_OPT);
	else
		GMT_Message (API, GMT_TIME_NONE, "\t[-A<fields>] [-C] [-D<width>[+c][+l|h]] [-E] [-Er|s[+l|h]] [-G<grdfile>] [-Q] [-W[i][o][+s]]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_a_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_q_OPT, GMT_r_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "I,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	if (API->external)
		GMT_Message (API, GMT_TIME_NONE, "\t-A List of comma-separated fields to be written as grids. Choose from\n");
	else
		GMT_Message (API, GMT_TIME_NONE, "\t-A List of comma-separated fields to be written as grids (requires -G). Choose from\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   z, s, l, h, and w. s|l|h requires -E; w requires -W[o].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be used with -Er|s [Default is z only].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Output center of block and mode z-value [Default is mode location (but see -Q)].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Compute modes via binning using <width>; append +c to center bins. If there are multiple\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   modes we return the average mode [+a]; append +l or +h to pick the low or high mode instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be combined with -E and implicitly sets -Q.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If your data are integers and <width> is not given we default to -D1+c+l\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default computes the mode as the Least Median of Squares (LMS) estimate].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Extend output with LMS scale (s), low (l), and high (h) value per block, i.e.,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   output (x,y,z,s,l,h[,w]) [Default outputs (x,y,z[,w])]; see -W regarding w.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Er to report record number of the modal value per block,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or -Es to report an unsigned integer source id (sid) taken from the x,y,z[,w],sid input.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For ties, report record number (or sid) of highest value (+h) or append +l for lowest [highest].\n");
	if (!API->external) {
		GMT_Message (API, GMT_TIME_NONE, "\t-G Specify output grid file name; no table results will be written to stdout.\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   If more than one field is set via -A then <grdfile> must contain  %%s to format field code.\n");
	}
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Quicker; get mode z and mean x,y [Default gets mode x, mode y, mode z].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set Weight options.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Wi reads Weighted Input (4 cols: x,y,z,w) but writes only (x,y,z[,s,l,h]) Output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Wo reads unWeighted Input (3 cols: x,y,z) but reports sum (x,y,z[,s,l,h],w) Output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -W with no modifier has both weighted Input and Output; Default is no weights used.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +s read/write standard deviations instead, with w = 1/s.\n");
	GMT_Option (API, "a,bi");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   Default is 3 columns (or 4 if -W is set).\n");
	GMT_Option (API, "bo,d,e,f,h,i,o,q,r,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct BLOCKMODE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to blockmode and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos = 0;
	bool sigma;
	char arg[GMT_LEN16] = {""}, p[GMT_BUFSIZ] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

				case 'A':	/* Requires -G and selects which fields should be written as grids */
				Ctrl->A.active = true;
				pos = 0;
				while ((gmt_strtok (opt->arg, ",", &pos, p)) && Ctrl->A.n_selected < BLK_N_FIELDS) {
					switch (p[0]) {	/* z,s,l,h,w */
						case 'z':	Ctrl->A.selected[0] = true;	break;
						case 's':	Ctrl->A.selected[1] = true;	break;
						case 'l':	Ctrl->A.selected[2] = true;	break;
						case 'h':	Ctrl->A.selected[3] = true;	break;
						case 'w':	Ctrl->A.selected[4] = true;	break;
						default:
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized field argument %s in -A!\n", p);
							n_errors++;
							break;
					}
					Ctrl->A.n_selected++;
				}
				if (Ctrl->A.n_selected == 0) {	/* Let -Az be the default */
					GMT_Report (GMT->parent, GMT_MSG_DEBUG, "-A interpreted to mean -Az.\n");
					Ctrl->A.selected[0] = true;
					Ctrl->A.n_selected = 1;
				}
				break;
			case 'C':	/* Report center of block instead */
				Ctrl->C.active = true;
				break;
			case 'D':	/* Histogram mode estimate */
				Ctrl->D.active = true;
				Ctrl->D.width = atof (opt->arg);
				pos = 0;
				if ((c = strchr (opt->arg, '+')) != NULL) {	/* Found modifiers */
					while ((gmt_strtok (c, "+", &pos, p))) {
						switch (p[0]) {
							case 'c': Ctrl->D.center = true; break;	/* Center the histogram */
							case 'a': Ctrl->D.mode = BLOCKMODE_AVE;  break;	/* Pick average mode */
							case 'l': Ctrl->D.mode = BLOCKMODE_LOW;  break;	/* Pick low mode */
							case 'h': Ctrl->D.mode = BLOCKMODE_HIGH; break;	/* Pick high mode */
							default:	/* Bad modifier */
								GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized modifier +%c.\n", p[0]);
								n_errors++;
								break;
						}
					}
				}
				break;
			case 'E':
				Ctrl->E.active = true;		/* Extended report with standard deviation, min, and max in cols 4-6 */
				if (opt->arg[0] == 'r' || opt->arg[0] == 's') {	/* Report row number or sid of median */
					switch (opt->arg[1]) {	/* Look for modifiers */
						case '+':	/* New syntax with +h or +l to parse */
							if (opt->arg[2] == 'l')
								Ctrl->E.mode = BLK_DO_INDEX_LO;
							else if (opt->arg[2] == 'h' || opt->arg[2] == '\0')	/* E.g., let Er+ be thought of as -Er+h */
								Ctrl->E.mode = BLK_DO_INDEX_HI;
							else {	/* Neither +l, +h, or just + is bad */
								GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized argument -E%s!\n", opt->arg);
								n_errors++;
							}
							break;
						case '-':	/* Old syntax -Er- or -Es- for reporting index/source of lower value */
							Ctrl->E.mode = BLK_DO_INDEX_LO;
							break;
						default:	/* Default reports index/source of higher value */
							Ctrl->E.mode = BLK_DO_INDEX_HI;
							break;
					}
					if (opt->arg[0] == 's') /* report sid, add in flag */
						Ctrl->E.mode |= BLK_DO_SRC_ID;
				}
				else if (opt->arg[0] == '\0')	/* Plain -E : Report LMSscale, low, high in cols 4-6 */
					Ctrl->E.mode = BLK_DO_EXTEND3;
				else	/* WTF? */
					n_errors++;
				break;
			case 'G':	/* Write output grid(s) */
				if (!GMT->parent->external && Ctrl->G.n) {	/* Command line interface */
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "-G can only be set once!\n");
					n_errors++;
				}
				else if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file[Ctrl->G.n++] = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':	/* Get block dimensions */
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'Q':	/* Quick mode for modal z */
				Ctrl->Q.active = true;
				break;
			case 'W':	/* Use in|out weights */
				Ctrl->W.active = true;
				if (gmt_validate_modifiers (GMT, opt->arg, 'W', "s")) n_errors++;
				sigma = (gmt_get_modifier (opt->arg, 's', arg)) ? true : false;
				switch (arg[0]) {
					case '\0':
						Ctrl->W.weighted[GMT_IN] = Ctrl->W.weighted[GMT_OUT] = true;
						Ctrl->W.sigma[GMT_IN] = Ctrl->W.sigma[GMT_OUT] = sigma;
						break;
					case 'i': case 'I':
						Ctrl->W.weighted[GMT_IN] = true;
						Ctrl->W.sigma[GMT_IN] = sigma;
						break;
					case 'o': case 'O':
						Ctrl->W.weighted[GMT_OUT] = true;
						Ctrl->W.sigma[GMT_OUT] = sigma;
						break;
					default:
						n_errors++;
						break;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->G.active) {	/* Make sure -A sets valid fields, some require -E */
		if (!Ctrl->E.active && (Ctrl->A.selected[1] || Ctrl->A.selected[2] || Ctrl->A.selected[3])) {
			/* -E is required if -A specifies l or h */
			Ctrl->E.active = true;			/* Extended report with standard deviation, min, and max in cols 4-6 */
			Ctrl->E.mode = BLK_DO_EXTEND3;	/* Report LMSscale, low, high in cols 4-6 */
		}
		if (GMT->parent->external && !Ctrl->A.active) {		/* From externals let -G equals -Az */
			Ctrl->A.active = true;
			Ctrl->A.selected[0] = true;
			Ctrl->A.n_selected = 1;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && !Ctrl->G.active, "Option -A requires -G\n");
	n_errors += gmt_M_check_condition (GMT, GMT->parent->external && Ctrl->G.active && !Ctrl->A.active,
	                                   "Option -G requires -A\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && (Ctrl->E.mode == BLK_DO_INDEX_LO || Ctrl->E.mode == BLK_DO_INDEX_HI), "-Es|r are incompatible with -G\n");
	if (Ctrl->G.active) {	/* Make sure -A sets valid fields, some require -E */
		if (Ctrl->A.active && Ctrl->A.n_selected > 1 && !GMT->parent->external && !strstr (Ctrl->G.file[0], "%s")) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "-G file format must contain a %%s for field type substitution.\n");
			n_errors++;
		}
		else if (!Ctrl->A.active)	/* Set default z output grid */
			Ctrl->A.selected[0] = true, Ctrl->A.n_selected = 1;
		else {	/* Make sure -A choices are valid and that -E is set if extended fields are selected */
			if (!Ctrl->E.active && (Ctrl->A.selected[1] || Ctrl->A.selected[2] || Ctrl->A.selected[3])) {
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "-E is required if -A specifies s, l, or h.  -E was added.\n");
				Ctrl->E.active = true;
			}
			if (Ctrl->A.selected[4] && !Ctrl->W.weighted[GMT_OUT]) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "-W or -Wo is required if -A specifies w.\n");
				n_errors++;
			}
		}
	}
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0,
	                                   "Option -I: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->E.active,
	                                   "Option -D: Cannot be combined with -E\n");
	n_errors += gmt_check_binary_io (GMT, (Ctrl->W.weighted[GMT_IN]) ? 4 : 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL struct BIN_MODE_INFO *bin_setup (struct GMT_CTRL *GMT, double width, bool center, int mode_choice, bool is_integer, double z_min, double z_max) {
	/* Estimate mode by finding a maximum in the histogram resulting
	 * from binning the data with the specified width. Note that the
	 * data array is already sorted on a[k]. We check if we find more
	 * than one mode and return the chosen one as per the settings.
	 * This function sets up quantities needed as we loop over the
	 * spatial bins */

	struct BIN_MODE_INFO *B = gmt_M_memory (GMT, NULL, 1, struct BIN_MODE_INFO);
	char *mode = "lah";

	if (is_integer) {	/* Special consideration for integers */
		double d_intval;
		if (width == 0.0) {
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "For integer data and no -D<width> specified we default to <width> = 1\n");
			width = 1.0;
		}
		d_intval = (double)lrint (width);
		if (doubleAlmostEqual (d_intval, width)) {
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "For integer data and integer width we automatically select centered bins\n");
			center = true;
			if (mode_choice == BLOCKMODE_DEF) {
				GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "For integer data and integer width we automatically select lowest mode\n");
				mode_choice = BLOCKMODE_LOW;
			}
		}
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Effective mode option is -D%g+c+%c\n", width, mode[mode_choice+1]);
	}
	B->i_offset = (center) ? 0.5 : 0.0;
	B->o_offset = (center) ? 0.0 : 0.5;
	B->width = width;
	B->i_width = 1.0 / width;
	B->min = irint (floor ((z_min * B->i_width) + B->i_offset));
	B->max = irint (ceil  ((z_max * B->i_width) + B->i_offset));
	B->n_bins = B->max - B->min + 1;
	B->count = gmt_M_memory (GMT, NULL, B->n_bins, double);
	B->mode_choice = (mode_choice == BLOCKMODE_DEF) ? BLOCKMODE_AVE : mode_choice;

	return (B);
}

GMT_LOCAL double bin_mode (struct GMT_CTRL *GMT, struct BLK_DATA *d, uint64_t n, uint64_t k, struct BIN_MODE_INFO *B) {
	/* Estimate mode by finding a maximum in the histogram resulting
	 * from binning the data with the specified width. Note that the
	 * data array is already sorted on a[k]. We check if we find more
	 * than one mode and return the chosen one as per the settings. */

	double value = 0.0, mode_count = 0.0;
	uint64_t i;
	unsigned int n_modes = 0;
	int bin, mode_bin = 0;
	bool done;
	gmt_M_unused(GMT);

	gmt_M_memset (B->count, B->n_bins, double);	/* Reset the counts */
	for (i = 0; i < n; i++) {	/* Loop over sorted data points */
		bin = urint (floor ((d[i].a[k] * B->i_width) + B->i_offset)) - B->min;
		B->count[bin] += d[i].a[BLK_W];		/* Add up counts or weights */
		if (B->count[bin] > mode_count) {	/* New max count value; make a note */
			mode_count = B->count[bin];	/* Highest count so far... */
			mode_bin = bin;			/* ...occurring for this bin */
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
			case BLOCKMODE_HIGH:	/* Update highest mode so far, when loop exits we have the highest mode */
			 	value = ((bin + B->min) + B->o_offset) * B->width;
				break;
		}
	}
	if (B->mode_choice == BLOCKMODE_AVE && n_modes > 0) value /= n_modes;	/* The average of the multiple modes */

	return (value);
}

GMT_LOCAL uint64_t get_source (struct BLK_DATA *d, uint64_t i, uint64_t j, unsigned int emode) {
	/* Return the source at the center or the lo/hi of the two in the middle */
	uint64_t s = j - i + 1, mid = i + s/2, src;
	if (s%2)	/* A single central point */
		src = d[mid].src_id;
	else	/* Must go low or high */
		src = (emode & BLK_DO_INDEX_HI) ? d[mid-1].src_id : d[mid].src_id;
	return src;
}

GMT_LOCAL double weighted_mode (struct BLK_DATA *d, double wsum, unsigned int emode, uint64_t n, unsigned int k, uint64_t *index) {
	/* Looks for the "shortest 50%". This means that when the cumulative weight
	   (y) is plotted against the value (x) then the line between (xi,yi) and
	   (xj,yj) should be the steepest for any combination where (yj-yi) is 50%
	   of the total sum of weights.
	   Here, n > 2. */

	double top, bottom, p, p_max, mode, last_del = 0.0;
	uint64_t i, j, src = 0, n_modes = 0;
	bool equidistant = true;

	/* Do some initializations */
	wsum = 0.5 * wsum; /* Sets the 50% range */
	/* First check if any single point has 50% or more of the total weights; if so we are done.
	 * While at it, check if points are equally spaced, inc which case we pick mode as the center */
	for (i = 0; i < n; i++) {
		if (d[i].a[BLK_W] >= wsum) {	/* Found a mighty and weighty single point */
			if (index) *index = d[i].src_id;
			return d[i].a[k];
		}
		else if (i && equidistant) {	/* May compute delta from 2nd point onwards */
			double del = d[i].a[k] - d[i-1].a[k];
			if (i > 1 && !doubleAlmostEqualZero (del, last_del))	/* Check for identical spacing */
				equidistant = false;	/* Well, not any more */
			else
				last_del = del;
		}
	}
	/* Deal with special cases */
	if (equidistant) {	/* Equidistant data set, pick middle */
		i = n / 2;
		if (n % 2) {	/* Single middle point */
			if (index) *index = d[i].src_id;
			mode = d[i].a[k];
		}
		else {	/* Shared middle points */
			if (index) *index = (emode & BLK_DO_INDEX_HI) ? d[i-1].src_id : d[i].src_id;
			mode = 0.5 * (d[i-1].a[k] + d[i].a[k]);
		}
		return mode;
	}
	/* Must find maximum slope in the cdf */
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
		if (p > p_max) {	/* New maximum, get new best mode */
			p_max = p;
			mode = 0.5 * (d[i].a[k] + d[j].a[k]);
			n_modes = 1;
			if (index) src = get_source (d, i, j, emode);	/* Must find corresponding middle or low/high index */
		}
		else if (doubleAlmostEqual (p, p_max)) {	/* Same peak as previous best mode, get average of these modes */
			if (index) {	/* Cannot have multiple modes when we are requesting the source, go with high or low per emode setting */
				if (emode & BLK_DO_INDEX_HI) {	/* OK, we are interested in the higher mode only */
					src = get_source (d, i, j, emode);	/* Must find corresponding high index */
					mode = 0.5 * (d[i].a[k] + d[j].a[k]);
				}
			}
			else {	/* Can average modes if need be */
				mode += 0.5 * (d[i].a[k] + d[j].a[k]);
				n_modes++;
			}
		}
	}
	if (n_modes > 1) mode /= n_modes;
	if (emode && index) *index = src;
	return (mode);
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {GMT_Destroy_Data (API, &Grid); gmt_M_free (GMT, Out); gmt_M_free (GMT, data); Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_blockmode (void *V_API, int mode, void *args) {
	bool mode_xy, do_extra = false, is_integer, duplicate_col, bail = false;

	int way, error = 0;

	unsigned int row, col, emode = 0, n_input, n_output;
	unsigned int k, kk, NF = 0, fcol[BLK_N_FIELDS] = {2,3,4,5,6,7,0,0}, field[BLK_N_FIELDS];

	uint64_t node, first_in_cell, first_in_new_cell, n_lost, n_read;
	uint64_t n_cells_filled, n_in_cell, nz, n_pitched, src_id = 0;
	uint64_t w_col, i_col = 0, sid_col, *src_id_ptr = NULL;

	size_t n_alloc = 0, nz_alloc = 0;

	double out[7], wesn[4], i_n_in_cell, d_intval, weight, half_dx, *in = NULL, *z_tmp = NULL;
	double z_min = DBL_MAX, z_max = -DBL_MAX;

	char format[GMT_LEN512] = {""}, *old_format = NULL, *fcode[BLK_N_FIELDS] = {"z", "s", "l", "h", "w", "", "", ""}, *code[BLK_N_FIELDS];
	char file[PATH_MAX] = {""};

	struct GMT_OPTION *options = NULL;
	struct GMT_GRID *Grid = NULL, *G = NULL, *GridOut[BLK_N_FIELDS];
	struct GMT_RECORD *In = NULL, *Out = NULL;
	struct BIN_MODE_INFO *B = NULL;
	struct BLK_DATA *data = NULL;
	struct BLOCKMODE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);
	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the blockmode main code ----------------------------*/

	gmt_M_memset (GridOut, BLK_N_FIELDS, struct GMT_GRID *);	/* Initialize all pointers to NULL */

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");

	if (Ctrl->C.active && Ctrl->Q.active) {
		GMT_Report (API, GMT_MSG_WARNING, "-C overrides -Q\n");
		Ctrl->Q.active = false;
	}

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);

	duplicate_col = (gmt_M_360_range (Grid->header->wesn[XLO], Grid->header->wesn[XHI]) && Grid->header->registration == GMT_GRID_NODE_REG);	/* E.g., lon = 0 column should match lon = 360 column */
	half_dx = 0.5 * Grid->header->inc[GMT_X];
	mode_xy = !Ctrl->C.active;

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		snprintf (format, GMT_LEN512, "W: %s E: %s S: %s N: %s n_columns: %%d n_rows: %%d\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, format, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->n_columns, Grid->header->n_rows);
	}

	gmt_set_xy_domain (GMT, wesn, Grid->header);	/* May include some padding if gridline-registered */

	/* Specify input and output expected columns */
	n_input = 3 + Ctrl->W.weighted[GMT_IN] + ((Ctrl->E.mode & BLK_DO_SRC_ID) ? 1 : 0);	/* 3 columns on output, plus 1 extra if -W and another if -Es  */
	if ((error = GMT_Set_Columns (API, GMT_IN, n_input, GMT_COL_FIX)) != GMT_NOERROR) {
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
	if (!Ctrl->G.active && (error = GMT_Set_Columns (API, GMT_OUT, n_output, GMT_COL_FIX)) != GMT_NOERROR) {
		Return (error);
	}

	/* Register likely data sources unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (!Ctrl->G.active && GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}

	/* Initialize the i/o for doing record-by-record reading/writing */
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	sid_col = (Ctrl->W.weighted[GMT_IN]) ? 4 : 3;	/* Column with integer source id [if -Es is set] */
	n_read = n_pitched = 0;	/* Initialize counters */

	GMT->session.min_meminc = GMT_INITIAL_MEM_ROW_ALLOC;	/* Start by allocating a 32 Mb chunk */

	/* Read the input data */
	is_integer = true;	/* Until proven otherwise */

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;							/* Go back and read the next record */
		}
		in = In->data;	/* Only need to process numerical part here */

		if (gmt_M_is_dnan (in[GMT_Z])) 		/* Skip if z = NaN */
			continue;

		/* Data record to process */

		n_read++;						/* Number of records read */

		if (gmt_M_y_is_outside (GMT, in[GMT_Y], wesn[YLO], wesn[YHI])) continue;	/* Outside y-range */
		if (gmt_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;	/* Outside x-range (or longitude) */

		/* We appear to be inside: Get row and col indices of this block */

		if (gmt_row_col_out_of_bounds (GMT, in, Grid->header, &row, &col)) continue;	/* Sorry, outside after all */
		if (duplicate_col && (wesn[XHI]-in[GMT_X] < half_dx)) {	/* Only compute modal values for the west column and not the repeating east column with lon += 360 */
			in[GMT_X] -= 360.0;	/* Make this point be considered for the western block mean value */
			col = 0;
		}

		/* OK, this point is definitively inside and will be used */

		if (is_integer) {	/* Determine if we still only have integers */
			d_intval = (double) lrint (in[GMT_Z]);
			if (!doubleAlmostEqual (d_intval, in[GMT_Z])) is_integer = false;
		}

		if (Ctrl->D.active) {	/* Must find extreme values in z since sorting is per index, not on z alone */
			if (in[GMT_Z] < z_min) z_min = in[GMT_Z];
			else if (in[GMT_Z] > z_max) z_max = in[GMT_Z];
		}

		node = gmt_M_ijp (Grid->header, row, col);		/* Bin node */

		if (n_pitched == n_alloc) data = gmt_M_malloc (GMT, data, n_pitched, &n_alloc, struct BLK_DATA);
		data[n_pitched].ij = node;
		data[n_pitched].src_id = (Ctrl->E.mode & BLK_DO_SRC_ID) ? (uint64_t)lrint (in[sid_col]) : n_read;
		if (mode_xy) {	/* Need to store (x,y) so we can compute modal location later */
			data[n_pitched].a[GMT_X] = in[GMT_X];
			data[n_pitched].a[GMT_Y] = in[GMT_Y];
		}
		data[n_pitched].a[BLK_Z] = in[GMT_Z];
		data[n_pitched].a[BLK_W] = ((Ctrl->W.weighted[GMT_IN]) ? ((Ctrl->W.sigma[GMT_IN]) ? 1.0 / in[3] : in[3]) : 1.0);

		n_pitched++;
	} while (true);

	GMT->session.min_meminc = GMT_MIN_MEMINC;		/* Reset to the default value */

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	if (Ctrl->D.active && Ctrl->D.width == 0.0 && !is_integer) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -D: No bin width specified and data are not integers\n");
		Return (GMT_PARSE_ERROR);
	}

	if (n_read == 0) {	/* Blank/empty input files */
		GMT_Report (API, GMT_MSG_WARNING, "No data records found; no output produced\n");
		if (!(API->external && Ctrl->G.active))
			bail = true;
	}
	else if (n_pitched == 0) {	/* No points inside region */
		GMT_Report (API, GMT_MSG_WARNING, "No data points found inside the region; no output produced\n");
		if (!(API->external && Ctrl->G.active))
			bail = true;
	}
	if (bail) {	/* Time to quit */
		Return (GMT_NOERROR);
	}
	if (n_pitched < n_alloc) {
		n_alloc = n_pitched;
		data = gmt_M_malloc (GMT, data, 0, &n_alloc, struct BLK_DATA);
	}
	w_col = gmt_get_cols (GMT, GMT_OUT) - 1;	/* Weights always reported in last output column */
	fcol[4] = (unsigned int)w_col;				/* Since we don't know what it is until parsed */

	/* Ready to go. */

	if (Ctrl->G.active) {	/* Create the grid(s) */
		char *remarks[BLK_N_FIELDS] = {"Median value per bin", "L1 scale per bin", "Lowest value per bin", "Highest value per bin", "Weight per bin"};
		for (k = kk = 0; k < BLK_N_FIELDS; k++) {
			if (!Ctrl->A.selected[k]) continue;
			field[NF] = fcol[k];	/* Just keep record of which fields we are actually using */
			code[NF]  = fcode[k];
			if ((GridOut[NF] = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL, \
				GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_TITLE, "Grid produced by blockmode", GridOut[NF]) != GMT_NOERROR) Return (API->error);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, GridOut[NF]) != GMT_NOERROR) Return (API->error);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, remarks[k], GridOut[NF])) Return (API->error);
			if (G == NULL) G = GridOut[NF];	/* First grid header used to get node later */
			for (node = 0; node < G->header->size; node++)
				GridOut[NF]->data[node] = GMT->session.f_NaN;
			if (API->external && n_read == 0) {	/* Write the empty grids back to the external caller */
				if (strstr (Ctrl->G.file[kk], "%s"))
					sprintf (file, Ctrl->G.file[kk], code[k]);
				else
					strncpy (file, Ctrl->G.file[kk], PATH_MAX-1);
				if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, GridOut[k]) != GMT_NOERROR) {
					Return (API->error);
				}
			}
			if (Ctrl->G.n > 1) kk++;	/* Only true for APIs */
			NF++;	/* Number of actual field grids */
		}
		if (API->external && n_read == 0) {	/* Delayed return */
			Return (GMT_NOERROR);
		}
	}
	else {	/* Get ready for rec-by-rec output */
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Calculating block modes\n");

	if (emode) {					/* Index column last, with weight col just before */
		i_col = w_col--;
		old_format = GMT->current.io.o_format[i_col];		/* Need to restore this at end */
		GMT->current.io.o_format[i_col] = strdup ("%.0f");	/* Integer format for src_id */
	}

	/* Sort on node and Z value */

	qsort (data, n_pitched, sizeof (struct BLK_DATA), BLK_compare_index_z);

	if (Ctrl->D.active) {	/* Choose to compute unweighted modes by histogram binning */
		B = bin_setup (GMT, Ctrl->D.width, Ctrl->D.center, Ctrl->D.mode, is_integer, z_min, z_max);
		Ctrl->Q.active = true;	/* Cannot do modal positions */
	}

	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */

	if (emode) src_id_ptr = &src_id;

	/* Find n_in_cell and write appropriate output  */

	first_in_cell = n_cells_filled = nz = 0;
	while (first_in_cell < n_pitched) {
		weight = data[first_in_cell].a[BLK_W];
		if (do_extra) {
			if (nz == nz_alloc) z_tmp = gmt_M_malloc (GMT, z_tmp, nz, &nz_alloc, double);
			z_tmp[0] = data[first_in_cell].a[BLK_Z];
			nz = 1;
		}
		if (Ctrl->C.active) {	/* Use block center */
			row = (unsigned int)gmt_M_row (Grid->header, data[first_in_cell].ij);
			col = (unsigned int)gmt_M_col (Grid->header, data[first_in_cell].ij);
			out[GMT_X] = gmt_M_grd_col_to_x (GMT, col, Grid->header);
			out[GMT_Y] = gmt_M_grd_row_to_y (GMT, row, Grid->header);
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
				if (nz == nz_alloc) z_tmp = gmt_M_malloc (GMT, z_tmp, nz, &nz_alloc, double);
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
				out[GMT_Z] = weighted_mode (&data[first_in_cell], weight, emode, n_in_cell, GMT_Z, src_id_ptr);
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
				gmt_sort_array (GMT, z_tmp, nz, GMT_DOUBLE);
				out[3] = (nz%2) ? z_tmp[nz/2] : 0.5 * (z_tmp[(nz-1)/2] + z_tmp[nz/2]);
				out[3] *= MAD_NORMALIZE;	/* This will be LMS MAD-based scale */
			}
			else
				out[3] = GMT->session.d_NaN;
		}
		if (Ctrl->W.weighted[GMT_OUT]) out[w_col] = (Ctrl->W.sigma[GMT_OUT]) ? 1.0 / weight : weight;
		if (emode) out[i_col] = (double)src_id;

		if (Ctrl->G.active) {
			row = gmt_M_grd_y_to_row (GMT, out[GMT_Y], Grid->header);
			col = gmt_M_grd_x_to_col (GMT, out[GMT_X], Grid->header);
			node = gmt_M_ijp (Grid->header, row, col);	/* Bin node */
			for (k = 0; k < NF; k++)
				GridOut[k]->data[node] = (gmt_grdfloat)out[field[k]];
		}
		else
		GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */

		n_cells_filled++;
		first_in_cell = first_in_new_cell;
	}

	if (Ctrl->G.active) {	/* Writes the grid(s) */
		unsigned int kk;
		char file[PATH_MAX] = {""};
		for (k = kk = 0; k < NF; k++) {
			if (strstr (Ctrl->G.file[kk], "%s"))
				sprintf (file, Ctrl->G.file[kk], code[k]);
			else
				strncpy (file, Ctrl->G.file[kk], PATH_MAX-1);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, GridOut[k]) != GMT_NOERROR) {
				Return (API->error);
			}
			if (Ctrl->G.n > 1) kk++;	/* Only true for APIs */
		}
	}
	else if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		error = API->error;
	}
	else {
		n_lost = n_read - n_pitched;	/* Number of points that did not get used */
		GMT_Report (API, GMT_MSG_INFORMATION, "N read: %" PRIu64 " N used: %" PRIu64 " outside_area: %" PRIu64 " N cells filled: %" PRIu64 "\n", n_read, n_pitched, n_lost, n_cells_filled);
		error = GMT_NOERROR;
	}

	if (do_extra) gmt_M_free (GMT, z_tmp);

	if (emode) {
		gmt_M_str_free (GMT->current.io.o_format[i_col]);	/* Free the temporary integer format */
		GMT->current.io.o_format[i_col] = old_format;		/* Restore previous format */
	}
	if (Ctrl->D.active) {	/* Free histogram binning machinery */
		gmt_M_free (GMT, B->count);
		gmt_M_free (GMT, B);
	}

	Return (error);
}
