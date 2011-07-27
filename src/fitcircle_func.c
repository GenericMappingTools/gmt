/*--------------------------------------------------------------------
 *    $Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * API functions to support the fitcircle application.
 *
 * Author:	Walter H.F. Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: reads lon,lat pairs and finds mean position and pole
 * of best-fit circle through these points.  By default, fit great
 * circle.  If -S, fit best small circle or force given latitude.
 *
 *--------------------------------------------------------------------
 * Comments:
 *
 * fitcircle <lonlatfile> -L1|2|3 [-S[<lat>]]
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

#include "gmt.h"

struct FITCIRCLE_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct L {	/* -L[<n>] */
		GMT_LONG active;
		GMT_LONG norm;	/* 1, 2, or 3 (both) */
	} L;
	struct S {	/* -S[<lat] */
		GMT_LONG active;
		GMT_LONG mode;	/* 0 = find latitude, 1 = use specified latitude */
		double lat;	/* 0 for great circle */
	} S;
};

struct FITCIRCLE_DATA {
	double x[3];
};

void *New_fitcircle_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct FITCIRCLE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct FITCIRCLE_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	return ((void *)C);
}

void Free_fitcircle_Ctrl (struct GMT_CTRL *GMT, struct FITCIRCLE_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);	
}

GMT_LONG GMT_fitcircle_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "fitcircle %s [API] - Find mean position and best-fitting great- or small-circle to points on sphere\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: fitcircle [<table>] -L[<norm>] [-S[<lat>]] [%s] [%s]\n", GMT_V_OPT, GMT_bi_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-L Specify <norm> as -L1 or -L2; or use -L or -L3 to give both.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-S Attempt to fit a small circle rather than a great circle.\n");
	GMT_message (GMT, "\t   Optionally append the latitude <lat> of the small circle you want to fit.\n");
	GMT_explain_options (GMT, "VC0fghio:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_fitcircle_parse (struct GMTAPI_CTRL *C, struct FITCIRCLE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to fitcircle and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'L':	/* Select norm */
				Ctrl->L.active = TRUE;
				Ctrl->L.norm = (opt->arg[0]) ? atoi(opt->arg) : 3;
				break;
			case 'S':	/* Fit small-circle instead [optionally fix the latitude] */
				Ctrl->S.active = TRUE;
  				if (opt->arg[0]) {
					Ctrl->S.lat = atof (opt->arg);
					Ctrl->S.mode = 1;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += GMT_check_condition (GMT, Ctrl->L.norm < 1 || Ctrl->L.norm > 3, "Syntax error -L option: Choose between 1, 2, or 3\n");
	n_errors += GMT_check_binary_io (GMT, 2);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

double circle_misfit (struct GMT_CTRL *GMT, struct FITCIRCLE_DATA *data, GMT_LONG ndata, double *pole, GMT_LONG norm, double *work, double *circle_distance)
{
	/* Find the L(norm) misfit between a small circle through
	   center with pole pole.  Return misfit in radians.  */

	GMT_LONG i;
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
		for (i = 0; i < ndata; i++) work[i] = d_acos (GMT_dot3v (GMT, &data[i].x[0], pole));
		GMT_sort_array (GMT, (void *)work, ndata, GMT_DOUBLE_TYPE);
		*circle_distance = (ndata%2) ?  work[ndata/2] : 0.5 * (work[(ndata/2)-1] + work[ndata/2]);
	}
	else {
		*circle_distance = 0.0;
		for (i = 0; i < ndata; i++) *circle_distance += d_acos (GMT_dot3v (GMT, &data[i].x[0], pole));
		*circle_distance /= ndata;
	}

	/* Now do each data point */

	for (i = 0; i < ndata; i++) {
		distance = d_acos (GMT_dot3v (GMT, &data[i].x[0], pole));
		delta_distance = fabs (*circle_distance - distance);
		misfit += ((norm == 1) ? delta_distance : delta_distance * delta_distance);
	}
	return (norm == 1) ? misfit : sqrt (misfit);
}

double get_small_circle (struct GMT_CTRL *GMT, struct FITCIRCLE_DATA *data, GMT_LONG ndata, double *center, double *gcpole, double *scpole, GMT_LONG norm, double *work, GMT_LONG mode, double slat)
{
	/* Find scpole, the pole to the best-fit small circle, 
	   by L(norm) iterative search along arc between center
	   and +/- gcpole, the pole to the best fit great circle.  */

	GMT_LONG i, j;
	double temppole[3], a[3], b[3], oldpole[3];
	double trypos, tryneg, afit, bfit, afactor, bfactor, fit, oldfit;
	double length_ab, length_aold, length_bold, circle_distance;

	/* First find out if solution is between center and gcpole, or center and -gcpole */

	GMT_add3v (GMT, center, gcpole, temppole);
	GMT_normalize3v (GMT, temppole);
	trypos = circle_misfit (GMT, data, ndata, temppole, norm, work, &circle_distance);

	GMT_sub3v (GMT, center, gcpole, temppole);
	GMT_normalize3v (GMT, temppole);
	tryneg = circle_misfit (GMT, data, ndata, temppole, norm, work, &circle_distance);

	if (tryneg < trypos) {
		GMT_cpy3v (a, center);
		for (i = 0; i < 3; i++) b[i] = -gcpole[i];
	}
	else {
		GMT_cpy3v (a, center);
		GMT_cpy3v (b, gcpole);
	}

	/* Now a is at center and b is at pole on correct side. */
	
	if (mode) {	/* Want a specified latitude */
		sincosd (slat, &afactor, &bfactor);
		for (i = 0; i < 3; i++) scpole[i] = (afactor * a[i] + bfactor * b[i]);
		GMT_normalize3v (GMT, scpole);
		fit = circle_misfit (GMT, data, ndata, scpole, norm, work, &circle_distance);
		return (90.0-slat);
	}
	
	/*  Try to bracket a minimum.  Move from b toward a in 1 degree steps */

	afit = circle_misfit (GMT, data, ndata, a, norm, work, &circle_distance);
	bfit = circle_misfit (GMT, data, ndata, b, norm, work, &circle_distance);
	j = 1;
	do {
		sincosd ((double)j, &afactor, &bfactor);
		for (i = 0; i < 3; i++) temppole[i] = (afactor * a[i] + bfactor * b[i]);
		GMT_normalize3v (GMT, temppole);
		fit = circle_misfit (GMT, data, ndata, temppole, norm, work, &circle_distance);
		j++;
	} while (j < 90 && fit > bfit && fit > afit);

	if (j == 90) {	/* Bad news.  There isn't a better fitting pole anywhere.  */
		GMT_report (GMT, GMT_MSG_FATAL, "Sorry.  Cannot find small circle fitting better than great circle.\n");
		GMT_cpy3v (scpole, gcpole);
		return (-1.0);
	}
	/* Get here when temppole points to a minimum bracketed by a and b.  */

	GMT_cpy3v (oldpole, temppole);
	oldfit = fit;

	/* Now, while not converged, take golden section of wider interval.  */
	length_ab   = d_acos (GMT_dot3v (GMT, a, b));
	length_aold = d_acos (GMT_dot3v (GMT, a, oldpole));
	length_bold = d_acos (GMT_dot3v (GMT, b, oldpole));
	do {
		if (length_aold > length_bold) {	/* Section a_old  */
			for (i = 0; i < 3; i++) temppole[i] = (0.38197*a[i] + 0.61803*oldpole[i]);
			GMT_normalize3v (GMT, temppole);
			fit = circle_misfit (GMT, data, ndata, temppole, norm, work, &circle_distance);
			if (fit < oldfit) {	/* Improvement.  b = oldpole, oldpole = temppole  */
				GMT_cpy3v (b, oldpole);
				GMT_cpy3v (oldpole, temppole);
				oldfit = fit;
			}
			else	/* Not improved.  a = temppole  */
				GMT_cpy3v (a, temppole);
		}
		else {	/* Section b_old  */
			for (i = 0; i < 3; i++) temppole[i] = (0.38197*b[i] + 0.61803*oldpole[i]);
			GMT_normalize3v (GMT, temppole);
			fit = circle_misfit (GMT, data, ndata, temppole, norm, work, &circle_distance);
			if (fit < oldfit) {	/* Improvement.  a = oldpole, oldpole = temppole  */
				GMT_cpy3v (a, oldpole);
				GMT_cpy3v (oldpole, temppole);
				oldfit = fit;
			}
			else	/* Not improved.  b = temppole  */
				GMT_cpy3v (b, temppole);
		}
		length_ab   = d_acos (GMT_dot3v (GMT, a, b));
		length_aold = d_acos (GMT_dot3v (GMT, a, oldpole));
		length_bold = d_acos (GMT_dot3v (GMT, b, oldpole));
	} while (length_ab > 0.0001);	/* 0.1 milliradian ~ 0.006 degree  */

	GMT_cpy3v (scpole, oldpole);
	return (R2D * circle_distance);
}

/* Must free allocated memory before returning */
#define Return(code) {Free_fitcircle_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_fitcircle (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, greenwich = FALSE, allocate, n, np;
	GMT_LONG imin, imax, nrots, n_fields, i, j, k, n_alloc, n_data;

	char format[GMT_BUFSIZ], record[GMT_BUFSIZ], item[GMT_BUFSIZ];

	double lonsum, latsum, rad, *work = NULL, *in = NULL;
	double meanv[3], cross[3], cross_sum[3], gcpole[3], scpole[3];		/* Extra vectors  */

	struct FITCIRCLE_DATA *data = NULL;
	struct FITCIRCLE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_fitcircle_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_fitcircle_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_fitcircle", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vbf:", "ghio>" GMT_OPT("H"), options))) Return (error);
	Ctrl = (struct FITCIRCLE_CTRL *) New_fitcircle_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_fitcircle_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the fitcircle main code ----------------------------*/

	/* Initialize the i/o since we are doing record-by-record reading/writing */
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
	if ((error = GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_TEXT,  GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data output */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN,  GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */
	
	n_data = 0;	/* Initialize variables */
	lonsum = latsum = 0.0;
	n_alloc = GMT_CHUNK;
	data = GMT_memory (GMT, NULL, n_alloc, struct FITCIRCLE_DATA);
	sprintf (format, "%s\t%s", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);

	while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we reach EOF */

		if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);	/* Bail if there are any read errors */
		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;		/* Skip all table and segment headers */
		
		lonsum += in[GMT_X];	latsum += in[GMT_Y];
		GMT_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], data[n_data].x, TRUE);

		if (++n_data == n_alloc) data = GMT_memory (GMT, data, n_alloc <<= 1, struct FITCIRCLE_DATA);
        }
  	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */

 	if (n_data == 0) {	/* Blank/empty input files */
		GMT_report (GMT, GMT_MSG_NORMAL, "No data records found; no output produced");
		GMT_free (GMT, data);
		Return (EXIT_SUCCESS);
	}
     
      	if (n_data < n_alloc) data = GMT_memory (GMT, data, n_data, struct FITCIRCLE_DATA);
	allocate = (Ctrl->S.active && (Ctrl->L.norm%2));	/* Will need work array */
	if (allocate) work = GMT_memory (GMT, NULL, n_data, double);

	if ((error = GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT, GMT_BY_REC))) Return (error);	/* Enables data output and sets access mode */

	lonsum /= n_data;	latsum /= n_data;
	sprintf (record, "%ld points read, Average Position (Flat Earth): ", n_data);
	sprintf (item, format, lonsum, latsum);	strcat (record, item);
	GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)record);

	/* Get Fisher mean in any case, in order to set L2 mean correctly, if needed.  */

	GMT_memset (meanv, 3, double);
	for (i = 0; i < n_data; i++) for (j = 0; j < 3; j++) meanv[j] += data[i].x[j];
	GMT_normalize3v (GMT, meanv);
	if (lonsum > 180.0) greenwich = TRUE;

	if (Ctrl->L.norm%2) {	/* Want L1 solution */
		GMT_cart_to_geo (GMT, &latsum, &lonsum, meanv, TRUE);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		sprintf (record, format, lonsum, latsum);
		sprintf (item, "\tL1 Average Position (Fisher's Method)");	strcat (record, item);
		GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)record);
		GMT_memset (cross_sum, 3, double);
		for (i = 0; i < n_data; i++) {
			GMT_cross3v (GMT, &data[i].x[0], meanv, cross);
			if (cross[2] < 0.0)
				GMT_sub3v (GMT, cross_sum, cross, cross_sum);
			else
				GMT_add3v (GMT, cross_sum, cross, cross_sum);
		}
		GMT_normalize3v (GMT, cross_sum);
		if (Ctrl->S.active) GMT_cpy3v (gcpole, cross_sum);

		GMT_cart_to_geo (GMT, &latsum, &lonsum, cross_sum, TRUE);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		sprintf (record, format, lonsum, latsum);
		sprintf (item, "\tL1 N Hemisphere Great Circle Pole (Cross-Averaged)");	strcat (record, item);
		GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)record);
		latsum = -latsum;
		lonsum = d_atan2d (-cross_sum[GMT_Y], -cross_sum[GMT_X]);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		sprintf (record, format, lonsum, latsum);
		sprintf (item, "\tL1 S Hemisphere Great Circle Pole (Cross-Averaged)");	strcat (record, item);
		GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)record);
		if (Ctrl->S.active) {	/* Determine small circle pole */
			GMT_report (GMT, GMT_MSG_NORMAL, "Fitting small circle using L1 norm.\n");
			rad = get_small_circle (GMT, data, n_data, meanv, gcpole, scpole, (GMT_LONG)1, work, Ctrl->S.mode, Ctrl->S.lat);
			if (rad >= 0.0) {
				GMT_cart_to_geo (GMT, &latsum, &lonsum, scpole, TRUE);
				if (greenwich && lonsum < 0.0) lonsum += 360.0;
				sprintf (record, format, lonsum, latsum);
				sprintf (item, "\tL1 Small Circle Pole.  ");	strcat (record, item);
				sprintf (format, "Distance from Pole to L1 Small Circle (degrees): %s\n", GMT->current.setting.format_float_out);
				sprintf (item, format, rad);	strcat (record, item);
				GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)record);
			}
		}
	}

	sprintf (format, "%s\t%s", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	if (Ctrl->L.norm/2) {	/* Wanted the L2 solution */
		double *a = NULL, *lambda = NULL, *v = NULL, *b = NULL, *z = NULL;	/* Matrix stuff */

		n = np = 3;
		a = GMT_memory (GMT, NULL, np*np, double);
		lambda = GMT_memory (GMT, NULL, np, double);
		b = GMT_memory (GMT, NULL, np, double);
		z = GMT_memory (GMT, NULL, np, double);
		v = GMT_memory (GMT, NULL, np*np, double);

		for (i = 0; i < n_data; i++) for (j = 0; j < n; j++) for (k = 0; k < n; k++)
			a[j + k*np] += (data[i].x[j]*data[i].x[k]);

		if (GMT_jacobi (GMT, a, &n, &np, lambda, v, b, z, &nrots)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Eigenvalue routine failed to converge in 50 sweeps.\n");
			GMT_report (GMT, GMT_MSG_FATAL, "The reported L2 positions might be garbage.\n");
		}
		GMT_report (GMT, GMT_MSG_NORMAL, "Eigenvalue routine converged in %ld rotations.\n", nrots);
		imax = 0;	imin = 2;
		if (d_acos (GMT_dot3v (GMT, v, meanv)) > M_PI_2)
			for (i = 0; i < 3; i++) meanv[i] = -v[imax*np+i];
		else
			for (i = 0; i < 3; i++) meanv[i] = v[imax*np+i];
		GMT_cart_to_geo (GMT, &latsum, &lonsum, meanv, TRUE);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		sprintf (record, format, lonsum, latsum);
		sprintf (item, "\tL2 Average Position (Eigenval Method)");	strcat (record, item);
		GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)record);

		if (v[imin*np+2] < 0.0)	/* Eigvec is in S Hemisphere  */
			for (i = 0; i < 3; i++) gcpole[i] = -v[imin*np+i];
		else
			for (i = 0; i < 3; i++) gcpole[i] = v[imin*np+i];

		GMT_cart_to_geo (GMT, &latsum, &lonsum, gcpole, TRUE);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		sprintf (record, format, lonsum, latsum);
		sprintf (item, "\tL2 N Hemisphere Great Circle Pole (Eigenval Method)\n");	strcat (record, item);
		GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)record);
		latsum = -latsum;
		lonsum = d_atan2d (-gcpole[GMT_Y], -gcpole[GMT_X]);
		if (greenwich && lonsum < 0.0) lonsum += 360.0;
		sprintf (record, format, lonsum, latsum);
		sprintf (item, "\tL2 S Hemisphere Great Circle Pole (Eigenval Method)");	strcat (record, item);
		GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)record);

		GMT_free (GMT, v);
		GMT_free (GMT, z);
		GMT_free (GMT, b);
		GMT_free (GMT, lambda);
		GMT_free (GMT, a);
		if (Ctrl->S.active) {	/* Want small circle pole */
			GMT_report (GMT, GMT_MSG_NORMAL, "Fitting small circle using L2 norm.\n");
			rad = get_small_circle (GMT, data, n_data, meanv, gcpole, scpole, 2, work, Ctrl->S.mode, Ctrl->S.lat);
			if (rad >= 0.0) {
				/* True when small circle fits better than great circle */
				GMT_cart_to_geo (GMT, &latsum, &lonsum, scpole, TRUE);
				if (greenwich && lonsum < 0.0) lonsum += 360.0;
				sprintf (record, format, lonsum, latsum);
				sprintf (item, "\tL2 Small Circle Pole.  ");	strcat (record, item);
				sprintf (format, "Distance from Pole to L2 Small Circle (degrees): %s\n", GMT->current.setting.format_float_out);
				sprintf (item, format, rad);	strcat (record, item);
				GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)record);
			}
		}
	}
  	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	if (allocate) GMT_free (GMT, work);
	GMT_free (GMT, data);

	Return (GMT_OK);
}
