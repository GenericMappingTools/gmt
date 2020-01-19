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
 * API functions to support the fitcircle application.
 *
 * Author:	Walter H.F. Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: reads lon,lat pairs and finds mean position and pole
 * of best-fit circle through these points.  By default, fit great
 * circle.  If -S, fit best small circle or force given latitude.
 *
 *--------------------------------------------------------------------
 * Comments:
 *
 * fitcircle <lonlatfile> -L1|2|3 [-F[<flags>]] [-S[<lat>]]
 *
 * Read lon,lat pairs from input or file.  Find mean position and pole
 * of best-fit circle through these points.  By default, fit great
 * circle.  If -S, fit small circle.  In this case, fit great circle
 * first, and then search for minimum small circle by bisection.
 *
 * Formally, we want to minimize some norm on the distance between
 * each point and the circle, measured perpendicular to the circle.
 * For both L1 and L2 norms this is a rather intractable problem.
 * (L2 is non-linear, and in L1 it is not clear how to proceed).
 * However, some approximations exist which work well and are simple
 * to compute.  We create a list of x,y,z vectors on the unit sphere,
 * representing the original data.  To find a great circle, do this:
 * For L1:
 * 	Find the Fisher mean of these data, call it mean position.
 *	Find the (Fisher) mean of all cross-products between data and
 *		the mean position; call this the pole to the great circle.
 *	Note that the cross-products are proportional to the distance
 *		between datum and mean; hence above average gives data far
 *		from mean larger weight in determining pole.  This is
 *		analogous to fitting line in plane, where data far from
 *		average abscissa have large leverage in determining slope.
 * For L2:
 *	Create 3 x 3 matrix of sums of products of data vector elements.
 *	Find eigenvectors and eigenvalues of this matrix.
 *	Find mean as eigenvector corresponding to max eigenvalue.
 *	Find pole as eigenvector corresponding to min eigenvalue.
 *	Eigenvalue-eigenvector decomposition performed by Jacobi's iterative
 *		method of successive Givens rotations.  Trials suggest that
 *		this converges extremely rapidly (3 sweeps, 9 rotations).
 *
 * To find a small circle, first find the great circle pole and the mean
 * position.  Suppose the small circle pole to lie in the plane containing
 * the mean and great circle pole, and narrow down its location by bisection.
 * Alternatively, specify the latitude of the small circle.
 *
 */

/* TO DO: 1) Allow separate fit per line segment
	  2) New option to only output pole and ave, i.e., not text messages
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"fitcircle"
#define THIS_MODULE_MODERN_NAME	"fitcircle"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Find mean position and great [or small] circle fit to points on sphere"
#define THIS_MODULE_KEYS	"<D{,>D},>DF"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vabdefghioq" GMT_OPT("H")

struct FITCIRCLE_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct F {	/* -F[f|m|n|s|c] */
		bool active;
		unsigned int mode;	/* f = 1, m = 2, n = 4, s = 8, c = 16 [31] */
	} F;
	struct L {	/* -L[<n>] */
		bool active;
		unsigned int norm;	/* 1, 2, or 3 (both) */
	} L;
	struct S {	/* -S[<lat] */
		bool active;
		unsigned int mode;	/* 0 = find latitude, 1 = use specified latitude */
		double lat;	/* 0 for great circle */
	} S;
};

struct FITCIRCLE_DATA {
	double x[3];
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct FITCIRCLE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct FITCIRCLE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct FITCIRCLE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -L[<norm>] [-F[<flags>]] [-S[<lat>]] [%s] [%s]\n", name, GMT_V_OPT, GMT_a_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
		GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_q_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-L Specify <norm> as -L1 or -L2; or use -L or -L3 to give both.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-F We normally write a mixed numerical/text report.  Use -F to return just data columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append the output columns you want as one or more of fmnsc in any order:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f: Flat Earth mean location.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     m: Fisher (L1) or Eigenvalue (L2) mean location.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     n: North hemisphere pole location.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     s: South hemisphere pole location.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c: Small-circle pole location and colatitude.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     [Default is fmnsc].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -L3 is used we repeat the output for m|n|s|c (if selected).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Attempt to fit a small circle rather than a great circle.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally append the oblique latitude <lat> of the small circle you want to fit.\n");
	GMT_Option (API, "V,a,bi,di,e,f,g,h,i,o,q,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct FITCIRCLE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to fitcircle and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	size_t k, s_length;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'F':	/* Select outputs for data */
				Ctrl->F.active = true;
				s_length = strlen (opt->arg);
				for (k = 0; k < s_length; k++) {
					switch (opt->arg[k]) {
						case 'f': Ctrl->F.mode |=  1;	break;
						case 'm': Ctrl->F.mode |=  2;	break;
						case 'n': Ctrl->F.mode |=  4;	break;
						case 's': Ctrl->F.mode |=  8;	break;
						case 'c': Ctrl->F.mode |= 16;	break;
						default:
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -F option: Bad arg %s. Select any combination from fmnsc\n", opt->arg);
							n_errors++;
							break;
					}
				}
				break;
			case 'L':	/* Select norm */
				Ctrl->L.active = true;
				Ctrl->L.norm = (opt->arg[0]) ? atoi(opt->arg) : 3;
				break;
			case 'S':	/* Fit small-circle instead [optionally fix the latitude] */
				Ctrl->S.active = true;
  				if (opt->arg[0]) {
					Ctrl->S.lat = atof (opt->arg);
					Ctrl->S.mode = 1;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	if (Ctrl->F.active && Ctrl->F.mode == 0) Ctrl->F.mode = (Ctrl->S.active) ? 31 : 15;	/* Select all */

	n_errors += gmt_M_check_condition (GMT, Ctrl->F.mode & 16 && !Ctrl->S.active, "Syntax error -F option: Cannot select c without setting -S\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.norm < 1 || Ctrl->L.norm > 3, "Syntax error -L option: Choose between 1, 2, or 3\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == 1 && fabs (Ctrl->S.lat) > 90.0, "Syntax error -S option: Fixed latitude cannot exceed +|- 90\n");
	n_errors += gmt_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL double circle_misfit (struct GMT_CTRL *GMT, struct FITCIRCLE_DATA *data, uint64_t ndata, double *pole, int norm, double *work, double *circle_distance) {
	/* Find the L(norm) misfit between a small circle through
	   center with pole pole.  Return misfit in radians.  */

	uint64_t i;
	double distance, delta_distance, misfit = 0.0;

	/* At first, I thought we could use the center to define
		circle_dist = distance between pole and center.
		Then sum over data {dist[i] - circle_dist}.
		But it turns out that if the data are tightly
		curved, so that they are on a small circle
		within a few degrees of the pole, then the
		center point is not on the small circle, and
		we cannot use it.  So, we first have to fit
		the circle_dist correctly */

	if (norm == 1) {
		for (i = 0; i < ndata; i++) work[i] = d_acos (gmt_dot3v (GMT, &data[i].x[0], pole));
		gmt_sort_array (GMT, work, ndata, GMT_DOUBLE);
		*circle_distance = (ndata%2) ?  work[ndata/2] : 0.5 * (work[(ndata/2)-1] + work[ndata/2]);
	}
	else {
		*circle_distance = 0.0;
		for (i = 0; i < ndata; i++) *circle_distance += d_acos (gmt_dot3v (GMT, &data[i].x[0], pole));
		*circle_distance /= ndata;
	}

	/* Now do each data point */

	for (i = 0; i < ndata; i++) {
		distance = d_acos (gmt_dot3v (GMT, &data[i].x[0], pole));
		delta_distance = fabs (*circle_distance - distance);
		misfit += ((norm == 1) ? delta_distance : delta_distance * delta_distance);
	}
	return (norm == 1) ? misfit : sqrt (misfit);
}

GMT_LOCAL double get_small_circle (struct GMT_CTRL *GMT, struct FITCIRCLE_DATA *data, uint64_t ndata, double *center, double *gcpole, double *scpole, int norm, double *work, int mode, double slat) {
	/* Find scpole, the pole to the best-fit small circle,
	   by L(norm) iterative search along arc between center
	   and +/- gcpole, the pole to the best fit great circle.  */

	uint64_t i, j;
	double temppole[3], a[3], b[3], oldpole[3];
	double trypos, tryneg, afit, bfit, afactor, bfactor, fit, oldfit;
	double length_ab, length_aold, length_bold, circle_distance, angle;

	/* First find out if solution is between center and gcpole, or center and -gcpole */

	gmt_add3v (GMT, center, gcpole, temppole);
	gmt_normalize3v (GMT, temppole);
	trypos = circle_misfit (GMT, data, ndata, temppole, norm, work, &circle_distance);

	gmt_sub3v (GMT, center, gcpole, temppole);
	gmt_normalize3v (GMT, temppole);
	tryneg = circle_misfit (GMT, data, ndata, temppole, norm, work, &circle_distance);

	if (tryneg < trypos) {
		gmt_M_cpy3v (a, center);
		for (i = 0; i < 3; i++) b[i] = -gcpole[i];
	}
	else {
		gmt_M_cpy3v (a, center);
		gmt_M_cpy3v (b, gcpole);
	}

	/* Now a is at center and b is at pole on correct side. */

	if (mode) {	/* Want a specified latitude */
		sincosd (slat, &afactor, &bfactor);
		for (i = 0; i < 3; i++) scpole[i] = (afactor * a[i] + bfactor * b[i]);
		gmt_normalize3v (GMT, scpole);
		(void)circle_misfit (GMT, data, ndata, scpole, norm, work, &circle_distance);
		return (90.0-slat);
	}

	/*  Try to bracket a minimum.  Move from b toward a in 1 degree steps */

	afit = circle_misfit (GMT, data, ndata, a, norm, work, &circle_distance);
	bfit = circle_misfit (GMT, data, ndata, b, norm, work, &circle_distance);
	j = 1;
	do {
		angle = (double)j;
		sincosd (angle, &afactor, &bfactor);
		for (i = 0; i < 3; i++) temppole[i] = (afactor * a[i] + bfactor * b[i]);
		gmt_normalize3v (GMT, temppole);
		fit = circle_misfit (GMT, data, ndata, temppole, norm, work, &circle_distance);
		j++;
	} while (j < 90 && fit > bfit && fit > afit);

	if (j == 90) {	/* Bad news.  There isn't a better fitting pole anywhere.  */
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Cannot find small circle fitting better than great circle.\n");
		gmt_M_cpy3v (scpole, gcpole);
		return (-1.0);
	}
	/* Get here when temppole points to a minimum bracketed by a and b.  */

	gmt_M_cpy3v (oldpole, temppole);
	oldfit = fit;

	/* Now, while not converged, take golden section of wider interval.  */
	length_aold = d_acos (gmt_dot3v (GMT, a, oldpole));
	length_bold = d_acos (gmt_dot3v (GMT, b, oldpole));
	do {
		if (length_aold > length_bold) {	/* Section a_old  */
			for (i = 0; i < 3; i++) temppole[i] = (0.38197*a[i] + 0.61803*oldpole[i]);
			gmt_normalize3v (GMT, temppole);
			fit = circle_misfit (GMT, data, ndata, temppole, norm, work, &circle_distance);
			if (fit < oldfit) {	/* Improvement.  b = oldpole, oldpole = temppole  */
				gmt_M_cpy3v (b, oldpole);
				gmt_M_cpy3v (oldpole, temppole);
				oldfit = fit;
			}
			else	/* Not improved.  a = temppole  */
				gmt_M_cpy3v (a, temppole);
		}
		else {	/* Section b_old  */
			for (i = 0; i < 3; i++) temppole[i] = (0.38197*b[i] + 0.61803*oldpole[i]);
			gmt_normalize3v (GMT, temppole);
			fit = circle_misfit (GMT, data, ndata, temppole, norm, work, &circle_distance);
			if (fit < oldfit) {	/* Improvement.  a = oldpole, oldpole = temppole  */
				gmt_M_cpy3v (a, oldpole);
				gmt_M_cpy3v (oldpole, temppole);
				oldfit = fit;
			}
			else	/* Not improved.  b = temppole  */
				gmt_M_cpy3v (b, temppole);
		}
		length_ab   = d_acos (gmt_dot3v (GMT, a, b));
		length_aold = d_acos (gmt_dot3v (GMT, a, oldpole));
		length_bold = d_acos (gmt_dot3v (GMT, b, oldpole));
	} while (length_ab > 0.0001);	/* 0.1 milliradian ~ 0.006 degree  */

	gmt_M_cpy3v (scpole, oldpole);
	return (R2D * circle_distance);
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_M_free (GMT, data); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_fitcircle (void *V_API, int mode, void *args) {
	bool greenwich = false;
	unsigned int imin, imax, nrots, j, k, n, np, n_cols = 0, col = 0;
	int error = 0;
	uint64_t i, n_data;
	size_t n_alloc;

	char format[GMT_LEN256] = {""}, record[GMT_LEN256] = {""};
	char *type[2] = {"great", "small"}, *way[4] = {"", "L1","L2","L1 and L2"};

	double lonsum, latsum, rad, out[18], *work = NULL, *in = NULL;
	double meanv[3], cross[3], cross_sum[3], gcpole[3], scpole[3];		/* Extra vectors  */

	struct GMT_OPTION *options = NULL;
	struct GMT_RECORD *In = NULL, *Out = NULL;
	struct FITCIRCLE_DATA *data = NULL;
	struct FITCIRCLE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
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

	/*---------------------------- This is the fitcircle main code ----------------------------*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");

	/* Initialize the i/o since we are doing record-by-record reading/writing */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	n_data = 0;	/* Initialize variables */
	lonsum = latsum = 0.0;
	n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	data = gmt_M_memory (GMT, NULL, n_alloc, struct FITCIRCLE_DATA);

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}
		in = In->data;	/* Only need to process numerical part here */
		if (in == NULL) {
			GMT_Report (API, GMT_MSG_VERBOSE, "No data columns found; no output can be produced");
			gmt_M_free (GMT, data);
			Return (GMT_NOERROR);
		}
		/* Data record to process */

		lonsum += in[GMT_X];	latsum += in[GMT_Y];
		gmt_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], data[n_data].x, true);

		if (++n_data == n_alloc) data = gmt_M_memory (GMT, data, n_alloc <<= 1, struct FITCIRCLE_DATA);
	} while (true);

  	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {
		Return (API->error);				/* Disables further data input */
	}

 	if (n_data == 0) {	/* Blank/empty input files */
		GMT_Report (API, GMT_MSG_VERBOSE, "No data records found; no output produced");
		gmt_M_free (GMT, data);
		Return (GMT_NOERROR);
	}

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}
	if (Ctrl->F.active) {	/* Must determine number of output columns for this single record */
		if (Ctrl->F.mode & 2) n_cols += 2;	/* Requested mean location */
		if (Ctrl->F.mode & 4) n_cols += 2;	/* Requested N pole location */
		if (Ctrl->F.mode & 8) n_cols += 2;	/* Requested S pole location */
		if (Ctrl->F.mode & 16) n_cols += 3;	/* Requested small circle pole location and colatitude */
		if (Ctrl->L.norm == 3) n_cols *= 2;	/* Selected all this for both norms, so double output size */
		if (Ctrl->F.mode & 1) n_cols += 2;	/* Requested flat earth mean as well */
		if ((error = GMT_Set_Columns (GMT->parent, GMT_OUT, n_cols, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
	}
	else if ((error = GMT_Set_Columns (GMT->parent, GMT_OUT, 2, GMT_COL_FIX)) != GMT_NOERROR) {
		Return (error);
	}

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {
		Return (API->error);	/* Enables data output and sets access mode */
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}

	if (n_data < n_alloc) data = gmt_M_memory (GMT, data, n_data, struct FITCIRCLE_DATA);
	if (Ctrl->S.active && Ctrl->L.norm%2) work = gmt_M_memory (GMT, NULL, n_data, double);

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Fitting %s circle using %s norm.\n", type[Ctrl->S.active], way[Ctrl->L.norm]);

	lonsum /= n_data;	latsum /= n_data;

	if (Ctrl->F.active) {	/* Return data coordinates */
		Out = gmt_new_record (GMT, out, NULL);
		if (Ctrl->F.mode & 1) {out[col++] = lonsum; out[col++] = latsum; }
	}
	else {	/* ASCII report */
		Out = gmt_new_record (GMT, out, record);	/* Place coordinates in data and message in text */
		out[GMT_X] = lonsum; out[GMT_Y] = latsum;
		snprintf (record, GMT_LEN256, "Points read: %" PRIu64 " Average Position (Flat Earth)", n_data);
		GMT_Put_Record (API, GMT_WRITE_DATA, Out);
	}

	/* Get Fisher mean in any case, in order to set L2 mean correctly, if needed.  */

	gmt_M_memset (meanv, 3, double);
	for (i = 0; i < n_data; i++) for (j = 0; j < 3; j++) meanv[j] += data[i].x[j];
	gmt_normalize3v (GMT, meanv);
	if (lonsum > 180.0) greenwich = true;

	if (Ctrl->L.norm%2) {	/* Want L1 solution */
		gmt_cart_to_geo (GMT, &latsum, &lonsum, meanv, true);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		if (Ctrl->F.active) {	/* Return data coordinates */
			if (Ctrl->F.mode & 2) {out[col++] = lonsum; out[col++] = latsum; }
		}
		else {
			out[GMT_X] = lonsum; out[GMT_Y] = latsum;
			snprintf (record, GMT_LEN256, "L1 Average Position (Fisher's Method)");
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		gmt_M_memset (cross_sum, 3, double);
		for (i = 0; i < n_data; i++) {
			gmt_cross3v (GMT, &data[i].x[0], meanv, cross);
			if (cross[2] < 0.0)
				gmt_sub3v (GMT, cross_sum, cross, cross_sum);
			else
				gmt_add3v (GMT, cross_sum, cross, cross_sum);
		}
		gmt_normalize3v (GMT, cross_sum);
		if (Ctrl->S.active) gmt_M_cpy3v (gcpole, cross_sum);

		gmt_cart_to_geo (GMT, &latsum, &lonsum, cross_sum, true);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		if (Ctrl->F.active) {	/* Return data coordinates */
			if (Ctrl->F.mode & 4) {out[col++] = lonsum; out[col++] = latsum; }
		}
		else {
			out[GMT_X] = lonsum; out[GMT_Y] = latsum;
			snprintf (record, GMT_LEN256, "L1 N Hemisphere Great Circle Pole (Cross-Averaged)");
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		latsum = -latsum;
		lonsum = d_atan2d (-cross_sum[GMT_Y], -cross_sum[GMT_X]);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		if (Ctrl->F.active) {	/* Return data coordinates */
			if (Ctrl->F.mode & 8) {out[col++] = lonsum; out[col++] = latsum; }
		}
		else {
			out[GMT_X] = lonsum; out[GMT_Y] = latsum;
			snprintf (record, GMT_LEN256, "L1 S Hemisphere Great Circle Pole (Cross-Averaged)");
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		if (Ctrl->S.active) {	/* Determine small circle pole */
			rad = get_small_circle (GMT, data, n_data, meanv, gcpole, scpole, 1, work, Ctrl->S.mode, Ctrl->S.lat);
			if (rad >= 0.0) {
				gmt_cart_to_geo (GMT, &latsum, &lonsum, scpole, true);
				if (greenwich && lonsum < 0.0) lonsum += 360.0;
				if (Ctrl->F.active) {	/* Return data coordinates */
					if (Ctrl->F.mode & 16) {out[col++] = lonsum; out[col++] = latsum; out[col++] = rad; }
				}
				else {
					out[GMT_X] = lonsum; out[GMT_Y] = latsum;
					sprintf (format, "L1 Small Circle Pole. Distance from Pole to L1 Small Circle (degrees): %s", GMT->current.setting.format_float_out);
					snprintf (record, GMT_LEN256, format, rad);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
			}
			else if (Ctrl->F.active) {	/* Must now return NaNs to indicate it failed */
				if (Ctrl->F.mode & 16) {out[col++] = GMT->session.d_NaN; out[col++] = GMT->session.d_NaN; out[col++] = GMT->session.d_NaN; }
			}
		}
	}

	if (Ctrl->L.norm/2) {	/* Wanted the L2 solution */
		double *a = NULL, *lambda = NULL, *v = NULL, *b = NULL, *z = NULL;	/* Matrix stuff */

		n = np = 3;
		a = gmt_M_memory (GMT, NULL, np*np, double);
		lambda = gmt_M_memory (GMT, NULL, np, double);
		b = gmt_M_memory (GMT, NULL, np, double);
		z = gmt_M_memory (GMT, NULL, np, double);
		v = gmt_M_memory (GMT, NULL, np*np, double);

		for (i = 0; i < n_data; i++) for (j = 0; j < n; j++) for (k = 0; k < n; k++)
			a[j + k*np] += (data[i].x[j]*data[i].x[k]);

		if (gmt_jacobi (GMT, a, n, np, lambda, v, b, z, &nrots)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Eigenvalue routine failed to converge in 50 sweeps.\n");
			GMT_Report (API, GMT_MSG_NORMAL, "The reported L2 positions might be garbage.\n");
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Eigenvalue routine converged in %d rotations.\n", nrots);
		imax = 0;	imin = 2;
		if (d_acos (gmt_dot3v (GMT, v, meanv)) > M_PI_2)
			for (i = 0; i < 3; i++) meanv[i] = -v[imax*np+i];
		else
			for (i = 0; i < 3; i++) meanv[i] = v[imax*np+i];
		gmt_cart_to_geo (GMT, &latsum, &lonsum, meanv, true);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		if (Ctrl->F.active) {	/* Return data coordinates */
			if (Ctrl->F.mode & 2) {out[col++] = lonsum; out[col++] = latsum; }
		}
		else {
			out[GMT_X] = lonsum; out[GMT_Y] = latsum;
			snprintf (record, GMT_LEN256, "L2 Average Position (Eigenval Method)");
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}

		if (v[imin*np+2] < 0.0)	/* Eigvec is in S Hemisphere  */
			for (i = 0; i < 3; i++) gcpole[i] = -v[imin*np+i];
		else
			for (i = 0; i < 3; i++) gcpole[i] = v[imin*np+i];

		gmt_cart_to_geo (GMT, &latsum, &lonsum, gcpole, true);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		if (Ctrl->F.active) {	/* Return data coordinates */
			if (Ctrl->F.mode & 4) {out[col++] = lonsum; out[col++] = latsum; }
		}
		else {
			out[GMT_X] = lonsum; out[GMT_Y] = latsum;
			snprintf (record, GMT_LEN256, "L2 N Hemisphere Great Circle Pole (Eigenval Method)");
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		latsum = -latsum;
		lonsum = d_atan2d (-gcpole[GMT_Y], -gcpole[GMT_X]);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		if (Ctrl->F.active) {	/* Return data coordinates */
			if (Ctrl->F.mode & 8) {out[col++] = lonsum; out[col++] = latsum; }
		}
		else {
			out[GMT_X] = lonsum; out[GMT_Y] = latsum;
			snprintf (record, GMT_LEN256, "L2 S Hemisphere Great Circle Pole (Eigenval Method)");
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}

		gmt_M_free (GMT, v);
		gmt_M_free (GMT, z);
		gmt_M_free (GMT, b);
		gmt_M_free (GMT, lambda);
		gmt_M_free (GMT, a);
		if (Ctrl->S.active) {	/* Want small circle pole */
			rad = get_small_circle (GMT, data, n_data, meanv, gcpole, scpole, 2, work, Ctrl->S.mode, Ctrl->S.lat);
			if (rad >= 0.0) {
				/* True when small circle fits better than great circle */
				gmt_cart_to_geo (GMT, &latsum, &lonsum, scpole, true);
				if (greenwich && lonsum < 0.0) lonsum += 360.0;
				if (Ctrl->F.active) {	/* Return data coordinates */
					if (Ctrl->F.mode & 16) {out[col++] = lonsum; out[col++] = latsum; out[col++] = rad; }
				}
				else {
					out[GMT_X] = lonsum; out[GMT_Y] = latsum;
					snprintf (format, GMT_LEN256, "L2 Small Circle Pole.  Distance from Pole to L2 Small Circle (degrees): %s", GMT->current.setting.format_float_out);
					snprintf (record, GMT_LEN256, format, rad);
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);
				}
			}
			else if (Ctrl->F.active) {	/* Must now return NaNs to indicate it failed */
				if (Ctrl->F.mode & 16) {out[col++] = GMT->session.d_NaN; out[col++] = GMT->session.d_NaN; out[col++] = GMT->session.d_NaN; }
			}
		}
	}
	if (Ctrl->F.active)
		GMT_Put_Record (API, GMT_WRITE_DATA, Out);

	gmt_M_free (GMT, work);
	gmt_M_free (GMT, data);
	gmt_M_free (GMT, Out);

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	Return (GMT_NOERROR);
}
