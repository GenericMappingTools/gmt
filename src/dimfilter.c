/*
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
/* dimfilter.c  reads a grdfile and creates filtered grd file
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
 * Date: 	25-OCT-2015
 * Version:	GMT 6
 *
 * For details, see Kim, S.-S., and Wessel, P. 2008, "Directional Median Filtering
 *   for Regional-Residual Separation of Bathymetry, Geochem. Geophys. Geosyst.,
 *   9(Q03005), doi:10.1029/2007GC001850.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"dimfilter"
#define THIS_MODULE_MODERN_NAME	"dimfilter"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Directional filtering of grids in the space domain"
#define THIS_MODULE_KEYS	"<G{,GG},>DQ"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS	"-:RVfh"

enum Dimfilter_mode {
	DIMFILTER_MODE_KIND_LOW  = -1,
	DIMFILTER_MODE_KIND_AVE  = 0,
	DIMFILTER_MODE_KIND_HIGH = +1
};

enum Dimfilter_filter {
	DIMFILTER_BOXCAR   = 0,
	DIMFILTER_COSINE   = 1,
	DIMFILTER_GAUSSIAN = 2,
	DIMFILTER_MEDIAN   = 3,
	DIMFILTER_MODE     = 4
};

enum Dimfilter_sector {
	DIMSECTOR_MIN    = 0,
	DIMSECTOR_MAX    = 1,
	DIMSECTOR_MEAN   = 2,
	DIMSECTOR_MEDIAN = 3,
	DIMSECTOR_MODE   = 4
};

struct DIMFILTER_INFO {
	int n_columns;		/* The max number of filter weights in x-direction */
	int n_rows;		/* The max number of filter weights in y-direction */
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
		int mode;
	} D;
	struct E {	/* -E */
		bool active;
	} E;
	struct F {	/* <type><filter_width>*/
		bool active;
		int filter;	/* Id for the filter */
		double width;
		int mode;
	} F;
	struct G {	/* -G<file> */
		bool active;
		char *file;
	} G;
	struct L {	/* -L */
		bool active;
	} L;
	struct N {	/* -N */
		bool active;
		unsigned int n_sectors;
		int filter;	/* Id for the filter */
		int mode;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
	struct S {	/* -S<file> */
		bool active;
		char *file;
	} S;
	struct T {	/* -T */
		bool active;
	} T;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct DIMFILTER_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct DIMFILTER_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->F.filter = C->N.filter = C->D.mode = -1;
	C->F.mode = DIMFILTER_MODE_KIND_AVE;
	C->N.n_sectors = 1;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct DIMFILTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->S.file);
	gmt_M_free (GMT, C);
}

static char *dimtemplate =
	"#!/usr/bin/env bash\n"
	"#\n"
	"#\n"
	"# Seung-Sep Kim, Chungnam National University, Daejeon, South Korea [seungsep@cnu.kr]\n"
	"\n"
	"# This is a template script showing the steps for DiM-based\n"
	"# regional-residual separation.\n"
	"#\n"
	"# For details, see Kim, S.-S., and Wessel, P. (2008), \"Directional Median Filtering\n"
	"# for Regional-Residual Separation of Bathymetry\", Geochem. Geophys. Geosyst.,\n"
	"# 9(Q03005), doi:10.1029/2007GC001850.\n"
	"\n"
	"# 0. System defaults (Change if you know what you are doing):\n"
	"dim_dist=2		# How we compute distances on the grid [Flat Earth approximation]\n"
	"dim_sectors=8		# Number of sectors to use [8]\n"
	"dim_filter=m		# Primary filter [m for median]\n"
	"dim_quantity=l		# Secondary filter [l for lower]\n"
	"dim_smooth_type=m	# Smoothing filter type [m for median]\n"
	"dim_smooth_width=50	# Smoothing filter width, in km [50]\n"
	"\n"
	"# 1. Setting up the region:\n"
	"#    To prevent edge effects, the input grid domain must be\n"
	"#    larger than the area of interest.  Make sure the input\n"
	"#    grid exceeds the area of interest by > 1/2 max filter width.\n"
	"box=-R	# Area of interest, a subset of data domain\n"
	"\n"
	"# 2. Specify names for input and output files\n"
	"bathy=	# Input bathymetry grid file for the entire data domain\n"
	"ors=	# Intermediate Optimal Robust Separator analysis results (table)\n"
	"orsout=	# ORS output work folder\n"
	"dim=	# Final output DiM-based regional grid\n"
	"err= 	# Final output DiM-based MAD uncertainty grid\n"
	"\n"
	"gmt set ELLIPSOID Sphere\n"
	"\n"
	"# A) ORS analysis first\n"
	"# if ORS does not give you the reasonable range of filter widths,\n"
	"# use the length scale of the largest feature in your domain as the\n"
	"# standard to choose the filter widths\n"
	"\n"
	"if [ ! -f $ors ]; then\n"
	"\n"
	"	mkdir -p $orsout\n"
	"\n"
	"	gmt grdcut $bathy $box -G/tmp/$$.t.nc  # the area of interest\n"
	"\n"
	"	# A.1. Set filter parameters for an equidistant set of filters:\n"
	"	minW= 	# Minimum filter width candidate for ORS  (e.g., 60) in km\n"
	"	maxW= 	# Maximum filter width candidate for ORS  (e.g., 600) in km\n"
	"	intW= 	# Filter width step (e.g., 20) in km\n"
	"	level=  # Base contour used to compute the volume and area of the residual (e.g., 300m)\n"
	"	#------stop A.1. editing here--------------------------------------\n"
	"\n"
	"	STEP=`gmt math -T$minW/$maxW/$intW -N1/0 =`\n"
	"\n"
	"	for width in $STEP\n"
	"	do\n"
	"		echo \"W = $width km\"\n"
	"		gmt dimfilter $bathy $box -G/tmp/$$.dim.nc -F${dim_filter}${width} -D${dim_dist} -N${dim_quantity}${dim_sectors} # DiM filter\n"
	"		gmt grdfilter /tmp/$$.dim.nc -G$orsout/dim.${width}.nc -F${dim_smooth_type}${dim_smooth_width} -D${dim_dist} # smoothing\n"
	"\n"
	"		gmt grdmath /tmp/$$.t.nc $orsout/dim.${width}.nc SUB = /tmp/$$.sd.nc # residual from DiM\n"
	"		gmt grdvolume /tmp/$$.sd.nc -Sk -C$level -Vl | awk \'{print r,$2,$3,$4}\' r=${width} >> $ors  # ORS from DiM\n"
	"	done\n"
	"\n"
	"fi\n"
	"\n"
	"# B) Compute DiM-based regional\n"
	"\n"
	"if [ ! -f $dim ]; then\n"
	"\n"
	"	# B.1. Set filter parameters for an equidistant set of filters:\n"
	"	minW=  	# Minimum optimal filter width (e.g., 200) in km\n"
	"	maxW= 	# Maximum optimal filter width (e.g., 240) in km\n"
	"	intW= 	# Filter width step (e.g., 5) in km\n"
	"	alldepth= 	# for MAD analysis\n"
	"	#------stop B.1. editing here--------------------------------------\n"
	"	width=`gmt math -N1/0 -T$minW/$maxW/$intW =`\n"
	"	let n_widths=0\n"
	"	for i in $width\n"
	"	do\n"
	"		if [ ! -f $orsout/dim.${i}.nc ]; then\n"
	"			echo \"filtering W = ${i} km\"\n"
	"			gmt dimfilter $bathy $box -G/tmp/$$.dim.nc -F${dim_filter}${i} -D${dim_dist} -N${dim_quantity}${dim_sectors}	# DiM filter\n"
	"			gmt grdfilter /tmp/$$.dim.nc -G$orsout/dim.${i}.nc -F${dim_smooth_type}${dim_smooth_width} -D${dim_dist} 	# smoothing\n"
	"		fi\n"
	"\n"
	"		if [ ! -f $alldepth ]; then\n"
	"			gmt grd2xyz -Z $orsout/dim.${i}.nc > /tmp/$$.${i}.depth\n"
	"		fi\n"
	"		let n_widths=n_widths+1\n"
	"	done\n"
	"\n"
	"	if [ ! -f $alldepth ]; then\n"
	"		paste /tmp/$$.*.depth > /tmp/$$.t.depth\n"
	"		# the number of columns can be different for each case\n"
	"		awk \'{print $1,\" \",$2,\" \",$3,\" \",$4,\" \",$5,\" \",$6,\" \",$7,\" \",$8,\" \",$9}\' /tmp/$$.t.depth > $alldepth\n"
	"		awk \'{for (k = 1; k <= \'\"$n_widths\"\', k++) print $1,\" \",$2,\" \",$3,\" \",$4,\" \",$5,\" \",$6,\" \",$7,\" \",$8,\" \",$9}\' /tmp/$$.t.depth > $alldepth\n"
	"		gmt grd2xyz $bathy $box -V > $bathy.xyz\n"
	"	fi\n"
	"\n"
	"	gmt dimfilter $alldepth -Q > /tmp/$$.out\n"
	"	wc -l /tmp/$$.out $bathy.xyz\n"
	"\n"
	"	paste $bathy.xyz /tmp/$$.out | awk \'{print $1,$2,$4}\' > /tmp/$$.dim.xyz\n"
	"	paste $bathy.xyz /tmp/$$.out | awk \'{print $1,$2,$5}\' > /tmp/$$.err.xyz\n"
	"\n"
	"	gmt xyz2grd /tmp/$$.dim.xyz -G$dim -I1m $box -V -r\n"
	"	gmt xyz2grd /tmp/$$.err.xyz -G$err -I1m $box -V -r\n"
	"\n"
	"fi\n";

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <ingrid> -D<distance_flag> -F<type><filter_width>[<modifier>] -G<outgrid> -N<type><n_sectors>[<modifier>]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-L] [-Q] [%s]\n", GMT_I_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-T] [%s] [%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_f_OPT, GMT_ho_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is grid to be filtered.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tDistance flag determines how grid (x,y) maps into distance units of filter width as follows:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -D0 grid x,y same units as <filter_width>, cartesian Distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -D1 grid x,y in degrees, <filter_width> in km, cartesian Distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -D2 grid x,y in degrees, <filter_width> in km, x_scaled by cos(middle y), cartesian Distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   These first three options are faster; they allow weight matrix to be computed only once.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Next two options are slower; weights must be recomputed for each scan line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -D3 grid x,y in degrees, <filter_width> in km, x_scale varies as cos(y), cartesian Distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -D4 grid x,y in degrees, <filter_width> in km, spherical Distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Sets the primary filter type and full (6 sigma) filter-width  Choose between\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (b)oxcar, (c)osine arch, (g)aussian, (m)edian filters\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or p(maximum likelihood Probability estimator -- a mode estimator):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Append +l to return the lowest mode if multiple modes are found [return average].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Append +u to return the uppermost mode if multiple modes are found [return average].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Sets output name for filtered grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Sets the secondary filter type and the number of sectors.  Choose between\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (l)ower, (u)pper, (a)verage, (m)edian, and (p) the mode estimator). If using p:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Append +l to return the lowest mode if multiple modes are found [return average].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Append +u to return the uppermost mode if multiple modes are found [return average].\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
#ifdef OBSOLETE
	GMT_Message (API, GMT_TIME_NONE, "\t-E Remove local planar trend from data, apply filter, then add back trend at filtered value.\n");
#endif
	GMT_Message (API, GMT_TIME_NONE, "\t-I Sets new Increment of output grid; enter xinc, optionally xinc/yinc.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is yinc = xinc.  Append an m [or s] to xinc or yinc to indicate minutes [or seconds];\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The new xinc and yinc should be divisible by the old ones (new lattice is subset of old).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Write dim.template.sh to stdout and stop; no other options allowed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Select error analysis mode; see documentation for how to prepare for using this option.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-R Sets new Range of output grid; enter <WESN> (xmin, xmax, ymin, ymax) separated by slashes.\n");
#ifdef OBSOLETE
	GMT_Message (API, GMT_TIME_NONE, "\t-S Sets output name for standard error grdfile and implies that we will compute a 2nd grid with\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a statistical measure of deviations from the average value.  For the convolution filters this\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   yields the standard deviation while for the median/mode filters we use MAD.\n");
#endif
	GMT_Message (API, GMT_TIME_NONE, "\t-T Toggles between grid and pixel registration for output grid [Default is same as input registration].\n");
	GMT_Option (API, "V,f,h,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct DIMFILTER_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to dimfilter and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, set = 0;
	int k;
	struct GMT_OPTION *opt = NULL;
#ifdef OBSOLETE
	int slow;
#endif

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)) != 0)
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				set++;
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = true;
				set++;
				break;
			case 'D':
				Ctrl->D.active = true;
				k = atoi (opt->arg);
				n_errors += gmt_M_check_condition (GMT, k < 0 || k > 4, "Syntax error -D option: Choose from the range 0-4\n");
				Ctrl->D.mode = k;
				set++;
				break;
#ifdef OBSOLETE
			case 'E':
				Ctrl->E.active = true;
				set++;
				break;
#endif
			case 'F':
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'b':
						Ctrl->F.filter = DIMFILTER_BOXCAR;
						break;
					case 'c':
						Ctrl->F.filter = DIMFILTER_COSINE;
						break;
					case 'g':
						Ctrl->F.filter = DIMFILTER_GAUSSIAN;
						break;
					case 'm':
						Ctrl->F.filter = DIMFILTER_MEDIAN;
						break;
					case 'p':
						Ctrl->F.filter = DIMFILTER_MODE;
						if (strstr (opt->arg, "+l") || opt->arg[strlen(opt->arg)-1] == '-') Ctrl->F.mode = DIMFILTER_MODE_KIND_LOW;
						else if (strstr (opt->arg, "+u")) Ctrl->F.mode = DIMFILTER_MODE_KIND_HIGH;
						break;
					default:
						n_errors++;
						break;
				}
				Ctrl->F.width = atof (&opt->arg[1]);
				set++;
				break;
			case 'G':
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				set++;
				break;
			case 'I':
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				set++;
				break;
			case 'L':	/* Write shell template to stdout */
				Ctrl->L.active = true;
				break;
			case 'N':	/* Scan: Option to set the number of sections and how to reduce the sector results to a single value */
				Ctrl->N.active = true;
				switch (opt->arg[0]) {
					case 'l':	/* Lower bound (min) */
						Ctrl->N.filter = DIMSECTOR_MIN;
						break;
					case 'u':	/* Upper bound (max) */
						Ctrl->N.filter = DIMSECTOR_MAX;
						break;
					case 'a':	/* Average (mean) */
						Ctrl->N.filter = DIMSECTOR_MEAN;
						break;
					case 'm':	/* Median */
						Ctrl->N.filter = DIMSECTOR_MEDIAN;
						break;
					case 'p':	/* Mode */
						Ctrl->N.filter = DIMSECTOR_MODE;
						if (strstr (opt->arg, "+l") || opt->arg[strlen(opt->arg)-1] == '-') Ctrl->N.mode = DIMFILTER_MODE_KIND_LOW;
						else if (strstr (opt->arg, "+u")) Ctrl->N.mode = DIMFILTER_MODE_KIND_HIGH;
						break;
					default:
						n_errors++;
						break;
				}
				k = atoi (&opt->arg[1]);	/* Number of sections to split filter into */
				n_errors += gmt_M_check_condition (GMT, k <= 0, "Syntax error -N option: Correct syntax: -Nx<nsectors>[<modifier>], with x one of l|u|a|m|p, <nsectors> is number of sectors\n");
				Ctrl->N.n_sectors = k;	/* Number of sections to split filter into */
				set++;
				break;
			case 'Q':	/* entering the MAD error analysis mode */
				Ctrl->Q.active = true;
				set++;
				break;
#ifdef OBSOLETE
			case 'S':
				Ctrl->S.active = true;
				Ctrl->S.file = strdup (opt->arg);
				set++;
				break;
#endif
			case 'T':	/* Toggle registration */
				Ctrl->T.active = true;
				set++;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->L.active)
		n_errors += gmt_M_check_condition (GMT, set, "Syntax error: -L can only be used by itself.\n");
	else if (!Ctrl->Q.active) {
		n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
		n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[ISET] && (GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0), "Syntax error -I option: Must specify positive increment(s)\n");
		n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output file\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->F.width <= 0.0, "Syntax error -F option: Correct syntax: -FX<width>, with X one of bcgmp, width is filter fullwidth\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->N.n_sectors == 0, "Syntax error -N option: Correct syntax: -NX<nsectors>, with X one of luamp, nsectors is number of sectors\n");
#ifdef OBSOLETE
		slow = (Ctrl->F.filter == DIMFILTER_MEDIAN || Ctrl->F.filter == DIMFILTER_MODE);		/* Will require sorting etc */
		n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && !slow, "Syntax error -E option: Only valid for robust filters -Fm|p.\n");
#endif
	}
	else {
		n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
		n_errors += gmt_M_check_condition (GMT, !Ctrl->Q.active, "Syntax error: Must use -Q to specify total # of columns in the input file.\n");
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void set_weight_matrix_dim (struct DIMFILTER_INFO *F, struct GMT_GRID_HEADER *h, double y_0, int fast) {
/* Last two gives offset between output node and 'origin' input node for this window (0,0 for integral grids) */
/* true when input/output grids are offset by integer values in dx/dy */

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

	i_half = F->n_columns / 2;
	j_half = F->n_rows / 2;
	f_half = 0.5 * F->width;
	r_f_half = 1.0 / f_half;
	sigma = F->width / 6.0;
	sig_2 = -0.5 / (sigma * sigma);
	for (i = -i_half; i <= i_half; i++) {
		for (j = -j_half; j <= j_half; j++) {
			ij = (j + j_half) * F->n_columns + i + i_half;
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

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_dimfilter (void *V_API, int mode, void *args) {
	unsigned short int **sector = NULL;

	unsigned int *n_in_median, wsize = 0, one_or_zero = 1, effort_level, n_sectors_2 = 0, col_in, row_in;
	unsigned int GMT_n_multiples = 0, col_out, row_out, i, j, k, s;
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
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
        options = GMT_Create_Options (API, mode, args);	if (API->error) bailout (API->error);   /* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error);	/* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the dimfilter main code ----------------------------*/

	if (Ctrl->L.active) {
		printf ("%s", dimtemplate);
		Return (GMT_NOERROR);
	}
	gmt_M_memset (&F, 1, struct DIMFILTER_INFO);
	F.deg2km = GMT->current.proj.DIST_KM_PR_DEG;

	if (!Ctrl->Q.active) {

		if ((Gin = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
		gmt_grd_init (GMT, Gin->header, options, true);	/* Update command history only */

		slow  = (Ctrl->F.filter == DIMFILTER_MEDIAN || Ctrl->F.filter == DIMFILTER_MODE);	/* Will require sorting etc */
		slow2 = (Ctrl->N.filter == DIMSECTOR_MEDIAN || Ctrl->N.filter == DIMSECTOR_MODE);	/* SCAN: Will also require sorting etc */

		if (Ctrl->T.active)	/* Make output grid of the opposite registration */
			one_or_zero = (Gin->header->registration == GMT_GRID_PIXEL_REG) ? 1 : 0;
		else
			one_or_zero = (Gin->header->registration == GMT_GRID_PIXEL_REG) ? 0 : 1;

		/* Use the -R region for output if set; otherwise match grid domain */
		gmt_M_memcpy (wesn, (GMT->common.R.active[RSET] ? GMT->common.R.wesn : Gin->header->wesn), 4, double);
		full_360 = (Ctrl->D.mode && gmt_grd_is_global (GMT, Gin->header));	/* Periodic geographic grid */

		if (GMT->common.R.active[ISET])
			gmt_M_memcpy (inc, GMT->common.R.inc, 2, double);
		else
			gmt_M_memcpy (inc, Gin->header->inc, 2, double);

		if (!full_360) {
			if (wesn[XLO] < Gin->header->wesn[XLO]) error = true;
			if (wesn[XHI] > Gin->header->wesn[XHI]) error = true;
		}
		if (wesn[YLO] < Gin->header->wesn[YLO]) error = true;
		if (wesn[YHI] > Gin->header->wesn[YHI]) error = true;

		if (error) {
			GMT_Report (API, GMT_MSG_NORMAL, "New WESN incompatible with old.\n");
			Return (GMT_RUNTIME_ERROR);
		}

		last_median = 0.5 * (Gin->header->z_min + Gin->header->z_max);
		z_min = Gin->header->z_min;	z_max = Gin->header->z_max;

		if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, wesn, inc, \
			!one_or_zero, GMT_NOTSET, NULL)) == NULL) Return (API->error);

		/* We can save time by computing a weight matrix once [or once pr scanline] only
		   if new grid spacing is multiple of old spacing */

		fast_way = (fabs (fmod (Gout->header->inc[GMT_X] / Gin->header->inc[GMT_X], 1.0)) < GMT_CONV4_LIMIT && fabs (fmod (Gout->header->inc[GMT_Y] / Gin->header->inc[GMT_Y], 1.0)) < GMT_CONV4_LIMIT);

		if (!fast_way) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Your output grid spacing is such that filter-weights must\n");
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
		i_origin = gmt_M_memory (GMT, NULL, Gout->header->n_columns, int);
		if (!fast_way) x_shift = gmt_M_memory (GMT, NULL, Gout->header->n_columns, double);

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

		F.n_columns = 2 * F.x_half_width + 1;
		F.n_rows = 2 * F.y_half_width + 1;
		F.width = Ctrl->F.width;
		F.weight = gmt_M_memory (GMT, NULL, F.n_columns*F.n_rows, double);

		if (slow) {	/* SCAN: Now require several work_arrays, one for each sector */
			work_array = gmt_M_memory (GMT, NULL, Ctrl->N.n_sectors, double *);
#ifdef OBSOLETE
			if (Ctrl->S.active) work_array2 = gmt_M_memory (GMT, NULL, 2*F.n_columns*F.n_rows, double);
			if (Ctrl->E.active) {
				xx = gmt_M_memory (GMT, NULL, Ctrl->N.n_sectors, short int *);
				yy = gmt_M_memory (GMT, NULL, Ctrl->N.n_sectors, short int *);
			}
#endif
			wsize = 2*F.n_columns*F.n_rows/Ctrl->N.n_sectors;	/* Should be enough, watch for messages to the contrary */
			for (i = 0; i < Ctrl->N.n_sectors; i++) {
				work_array[i] = gmt_M_memory (GMT, NULL, wsize, double);
#ifdef OBSOLETE
				if (Ctrl->E.active) {
					xx[i] = gmt_M_memory (GMT, NULL, wsize, short int);
					yy[i] = gmt_M_memory (GMT, NULL, wsize, short int);
				}
#endif
			}
		}

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Input n_columns,n_rows = (%d %d), output n_columns,n_rows = (%d %d), filter n_columns,n_rows = (%d %d)\n",
			Gin->header->n_columns, Gin->header->n_rows, Gout->header->n_columns, Gout->header->n_rows, F.n_columns, F.n_rows);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Filter type is %s.\n", filter_name[Ctrl->F.filter]);

		/* Compute nearest xoutput i-indices and shifts once */

		for (col_out = 0; col_out < Gout->header->n_columns; col_out++) {
			x_out = gmt_M_grd_col_to_x (GMT, col_out, Gout->header);	/* Current longitude */
			i_origin[col_out] = (int)gmt_M_grd_x_to_col (GMT, x_out, Gin->header);
			if (!fast_way) x_shift[col_out] = x_out - gmt_M_grd_col_to_x (GMT, i_origin[col_out], Gin->header);
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
			c_x = gmt_M_memory (GMT, NULL, n_sectors_2, double);
			c_y = gmt_M_memory (GMT, NULL, n_sectors_2, double);
			for (i = 0; i < n_sectors_2; i++) {
				angle = (i + 0.5) * (M_PI/n_sectors_2);	/* Angle of central diameter of each corridor */
				sincos (angle, &c_y[i], &c_x[i]);	/* Unit vector of diameter */
			}
		}
		else {
		/* SCAN: Precalculate which sector each point belongs to */
			sector = gmt_M_memory (GMT, NULL, F.n_rows, unsigned short int *);
			for (jj = 0; jj < F.n_rows; jj++) sector[jj] = gmt_M_memory (GMT, NULL, F.n_columns, unsigned short int);
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
		n_in_median = gmt_M_memory (GMT, NULL, Ctrl->N.n_sectors, unsigned int);
		value = gmt_M_memory (GMT, NULL, Ctrl->N.n_sectors, double);
		wt_sum = gmt_M_memory (GMT, NULL, Ctrl->N.n_sectors, double);

		for (row_out = 0; row_out < Gout->header->n_rows; row_out++) {

			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing output line %d\r", row_out);
			y_out = gmt_M_grd_row_to_y (GMT, row_out, Gout->header);
			j_origin = (int)gmt_M_grd_y_to_row (GMT, y_out, Gin->header);
			if (effort_level == 2) set_weight_matrix_dim (&F, Gout->header, y_out, shift);

			for (col_out = 0; col_out < Gout->header->n_columns; col_out++) {

				if (effort_level == 3) set_weight_matrix_dim (&F, Gout->header, y_out, shift);
				gmt_M_memset (n_in_median, Ctrl->N.n_sectors, unsigned int);
				gmt_M_memset (value, Ctrl->N.n_sectors, double);
				gmt_M_memset (wt_sum, Ctrl->N.n_sectors, double);
#ifdef OBSOLETE
				if (Ctrl->E.active) S = 0, Sx = Sy = Sz = Sxx = Syy = Sxy = Sxz = Syz = Sxx = Sw = 0.0;
				n = 0;
#endif

				ij_out = gmt_M_ijp (Gout->header, row_out, col_out);

				for (ii = -F.x_half_width; ii <= F.x_half_width; ii++) {
					scol = i_origin[col_out] + ii;
					if (scol < 0 || (col_in = scol) >= Gin->header->n_columns) continue;

					for (jj = -F.y_half_width; jj <= F.y_half_width; jj++) {
						srow = j_origin + jj;
						if (srow < 0 || (row_in = srow) >= Gin->header->n_rows) continue;

						ij_wt = (jj + F.y_half_width) * (uint64_t)F.n_columns + ii + F.x_half_width;
						if (F. weight[ij_wt] < 0.0) continue;

						ij_in = gmt_M_ijp (Gin->header, row_in, col_in);
						if (gmt_M_is_fnan (Gin->data[ij_in])) continue;

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
							if (Ctrl->F.filter == DIMFILTER_MEDIAN) {
								gmt_median (GMT, work_array[s], n_in_median[s], z_min, z_max, last_median, &this_median);
								last_median = this_median;
							}
							else
								gmt_mode (GMT, work_array[s], n_in_median[s], n_in_median[s]/2, true, Ctrl->F.mode, &GMT_n_multiples, &this_median);
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
							value[k] = (gmt_grdfloat)(value[s] / wt_sum[s]);
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
					if (Ctrl->F.filter == DIMFILTER_MEDIAN) {
						gmt_median (GMT, value, k, z2_min, z2_max, last_median2, &this_median2);
						last_median2 = this_median2;
					}
					else
						gmt_mode (GMT, value, k, k/2, true, Ctrl->N.mode, &GMT_n_multiples, &this_median2);
					z = this_median2;
				}
				else {	/* Get min, max, or mean */
					switch (Ctrl->N.filter) {	/* Initialize z, the final output value */
						case DIMSECTOR_MIN:	/* Lower bound */
							z = DBL_MAX;
							break;
						case DIMSECTOR_MAX:	/* Upper bound */
							z = -DBL_MAX;
							break;
						case DIMSECTOR_MEAN:	/* Average (mean) */
							z = 0.0;
							break;
						default:
							break;
					}
					for (s = 0; s < k; s++) {	/* Apply the min, max, or mean update */
						switch (Ctrl->N.filter) {
							case DIMSECTOR_MIN:	/* Lower bound */
								if (value[s] < z) z = value[s];
								break;
							case DIMSECTOR_MAX:	/* Upper bound */
								if (value[s] > z) z = value[s];
								break;
							case DIMSECTOR_MEAN:	/* Average (mean) */
								z += value[s];
								break;
							default:
								break;
						}
					}
					if (Ctrl->N.filter == DIMSECTOR_MEAN) z /= (double)k;	/* Mean requires a final normalization */
				}
				Gout->data[ij_out] = (gmt_grdfloat)z;
#ifdef OBSOLETE
				if (Ctrl->S.active) {	/* Now assess a measure of deviation about this value */
					if (slow) {	/* Get MAD! */
						gmt_sort_array (GMT, work_array2, n, GMT_DOUBLE);
						gmt_getmad (GMT, work_array2, n, z, &scale);
					}
					else {		/* Get weighted stdev. */
						scale = sqrt ((Sxx - Sw * z * z) / (Sw * (n - 1) / n));
					}
					Sout->data[ij_out] = (gmt_grdfloat)scale;
				}
#endif
			}
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "\n");

		/* At last, that's it!  Output: */

		if (n_nan) GMT_Report (API, GMT_MSG_VERBOSE, "Unable to estimate value at %" PRIu64 " nodes, set to NaN\n", n_nan);
#ifdef OBSOLETE
		if (Ctrl->E.active && n_bad_planes) GMT_Report (API, GMT_MSG_VERBOSE, "Unable to detrend data at %" PRIu64 " nodes\n", n_bad_planes);
#endif
		if (GMT_n_multiples > 0) GMT_Report (API, GMT_MSG_VERBOSE, "%d multiple modes found\n", GMT_n_multiples);

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Write filtered grid\n");
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Gout)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Gout) != GMT_NOERROR) {
			Return (API->error);
		}
#ifdef OBSOLETE
		if (Ctrl->S.active) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Write scale grid\n");
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Sout)) Return (API->error);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->S.file, Sout) != GMT_NOERROR) {
				Return (API->error);
			}
		}
#endif

		gmt_M_free (GMT, F. weight);
		gmt_M_free (GMT, i_origin);
		for (ii = 0; ii < F.n_rows; ii++) gmt_M_free (GMT, sector[ii]);
		gmt_M_free (GMT, sector);
		gmt_M_free (GMT, value);
		gmt_M_free (GMT, wt_sum);
		gmt_M_free (GMT, n_in_median);
		if (slow) {
			for (j = 0; j < Ctrl->N.n_sectors; j++) {
				gmt_M_free (GMT, work_array[j]);
#ifdef OBSOLETE
				if (Ctrl->E.active) {
					gmt_M_free (GMT, xx[j]);
					gmt_M_free (GMT, yy[j]);
				}
#endif
			}
			gmt_M_free (GMT, work_array);
#ifdef OBSOLETE
			if (Ctrl->S.active) gmt_M_free (GMT, work_array2);
			if (Ctrl->E.active) {
				gmt_M_free (GMT, xx);
				gmt_M_free (GMT, yy);
			}
#endif
		}
		if (!fast_way) gmt_M_free (GMT, x_shift);

	}
	else {	/* Here -Q is active */
		double *err_workarray = NULL, err_min, err_max, err_null_median = 0.0, err_median, err_mad, err_sum, out[3];
		uint64_t row, col;
		struct GMT_RECORD *Out = NULL;
		struct GMT_DATASET *D = NULL;
		struct GMT_DATASEGMENT *S = NULL;

		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);
		}
	
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
	
		if ((error = GMT_Set_Columns (API, GMT_OUT, 3, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) Return (error);
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_NONE) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
		gmt_set_cartesian (GMT, GMT_OUT);	/* No coordinates here */
		Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
		err_workarray = gmt_M_memory (GMT, NULL, S->n_columns, double);

		S = D->table[0]->segment[0];	/* A Single-segment data file */
		for (row = 0; row < S->n_rows; row++) {
			/* Store data into array and find sum/min/max, starting with col 0 */
			err_sum = err_workarray[0] = err_min = err_max = S->data[0][row];
			for (col = 1; col < S->n_columns; col++) {
				err_workarray[col] = S->data[col][row];
				err_sum += S->data[col][row];
				if (S->data[col][row] < err_min) err_min = S->data[col][row];
				if (S->data[col][row] > err_max) err_max = S->data[col][row];
			}
		
			/* calculate MEDIAN and MAD for each row */
			gmt_median (GMT, err_workarray, S->n_columns, err_min, err_max, err_null_median, &err_median);
			err_workarray[0] = err_min = err_max = fabs (err_workarray[0] - err_median);
			for (col = 1; col < S->n_columns; col++) {
				err_workarray[col] = fabs (err_workarray[col] - err_median);
				if (err_workarray[col] < err_min) err_min=err_workarray[col];
				if (err_workarray[col] > err_max) err_max=err_workarray[col];
			}
			gmt_median (GMT, err_workarray, S->n_columns, err_min, err_max, err_null_median, &err_mad);
			err_mad *= MAD_NORMALIZE;

			/* calculate MEAN for each row */
			out[0] = err_median;
			out[1] = err_mad;
			out[2] = (S->n_columns) ? err_sum / S->n_columns : 0.0;

			/* print out the results */
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */

			GMT_Report (API, GMT_MSG_DEBUG, "line %d passed\n", row);
		}

		gmt_M_free (GMT, Out);
		gmt_M_free (GMT, err_workarray);
	
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {
			Return (API->error);	/* Disables further data output */
		}
	}

	Return (GMT_NOERROR);
}
