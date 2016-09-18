/*--------------------------------------------------------------------
 *	$Id$
 *
 *   Copyright (c) 2016 by P. Wessel
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
 * Program for averaging finite rotations, resulting in mean rotations
 * optionally with a covarience matrix describing the variation.
 *
 * Author:	Paul Wessel, SOEST, Univ. of Hawaii, Honolulu, HI, USA
 * Date:	6-AUG-2016
 * Version:	GMT 5 [derived from rotsmooth_age.c in rotsuite]
 *
 * AN ASCII total reconstruction file must have the following format:
 *
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 2. Any number of finite pole records which each have the format:
 *    lon(deg)  lat(deg)  age(Ma)  ccw-angle(deg)	[<weight>]
 * 3. If no age is present the use -A to use angles as pseudo-ages
 *
 * Binary data files cannot have header records
 */

#define THIS_MODULE_NAME	"rotsmoother"
#define THIS_MODULE_LIB		"spotter"
#define THIS_MODULE_PURPOSE	"Get mean rotations and covarience matrices from set of finate rotations"
#define THIS_MODULE_KEYS	"<D{,>D}"

#include "spotter.h"

#define GMT_PROG_OPTIONS "-:>Vbdfghios" GMT_OPT("HMm")

struct ROTSMOOTHER_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -A */
		bool active;
	} A;
	struct C {	/* -C */
		bool active;
	} C;
	struct N {	/* -N */
		bool active;
	} N;
	struct S {	/* -S */
		bool active;
	} S;
	struct T {	/* -T<time>, -T<start/stop/inc> or -T<tfile.txt> */
		bool active;
		unsigned int n_times;	/* Number of reconstruction times */
		double *value;	/* Array with one or more reconstruction times */
	} T;
	struct W {	/* -W */
		bool active;
	} W;
	struct Z {	/* -Z */
		bool active;
	} Z;
};

#define K_ANGLE		0
#define K_LON		1
#define K_LAT		2
#define K_AGE		3
#define K_WEIGHT	4
#define N_ITEMS		5

struct AGEROT {
	double wxyasn[N_ITEMS];
	double P[3];	/* For Cartesian components */
	double Q[4];	/* For quaternion components */
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct ROTSMOOTHER_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct ROTSMOOTHER_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct ROTSMOOTHER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C->T.value);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: rotsmoother [<table>] [-A] [-C] [-N] [-S] [-T<time(s)>] [%s] [-W] [-Z] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<table> (in ASCII, binary, or netCDF) has 3 or more columns.  If no file(s) is given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   First 4 columns must have lon, lat (or lat, lon, see -:), time, and angle (degrees).\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Use opening angles as time.  Input is <lon> <lat> <angle> [<weight>] and -T refers to angles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default expects <lon> <lat> <time> <angle> [<weight>] and -T refers to time].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Compute covariance matrix for each mean rotation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Ensure all poles are in northern hemisphere [Default ensures positive opening angles].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Ensure all poles are in southern hemisphere [Default ensures positive opening angles].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set the output times when a mean rotation and covariance matrix is desired.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append a single time (-T<time>), an equidistant range of times (-T<min>/<max>/<inc> or -T<min>/<max>/<npoints>+),\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or the name of a file with a list of times (-T<tfile>).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The times indicate bin-boundaries and we output the average rotation time per bin.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Expect weights in last column for a weighted mean rotation [no weights].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Report negative opening angles [positive].\n");
	GMT_Option (API, "bi3,bo,d,h,i,o,s,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct ROTSMOOTHER_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to backtracker and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_in = 3, t;
	int k;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Supplemental parameters */

			case 'A':	/* Output only an age-limited segment of the track */
				Ctrl->A.active = true;
				break;

			case 'C':
				Ctrl->C.active = true;
				break;

			case 'N':	/* Ensure all poles reported are in northern hemisphere */
				Ctrl->N.active = true;
				break;

			case 'S':	/* Ensure all poles reported are in southern hemisphere */
				Ctrl->S.active = true;
				break;

			case 'T':	/* New: -Tage, -Tmin/max/inc, -Tmin/max/n+, -Tfile */
				Ctrl->T.active = true;
				if (!gmt_access (GMT, opt->arg, R_OK)) {	/* Gave a file with times in first column */
					uint64_t seg, row;
					struct GMT_DATASET *T = NULL;
					struct GMT_DATASEGMENT *S = NULL;
					if ((T = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, opt->arg, NULL)) == NULL) {
						GMT_Report (API, GMT_MSG_NORMAL, "Error reading file %s\n", opt->arg);
						n_errors++;
						continue;
					}
					/* Single table, build t array */
					Ctrl->T.n_times = (unsigned int)T->n_records;
					Ctrl->T.value = gmt_M_memory (GMT, NULL, Ctrl->T.n_times, double);
					for (seg = t = 0; seg < T->table[0]->n_segments; seg++) {
						S = T->table[0]->segment[seg];	/* Shorthand to current segment */
						for (row = 0; row < S->n_rows; seg++, t++)
							Ctrl->T.value[t] = S->data[GMT_X][row];
					}
					if (GMT_Destroy_Data (API, &T) != GMT_NOERROR)
						n_errors++;
					break;
				}
				/* Not a file */
				k = sscanf (opt->arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
				if (k == 3) {	/* Gave -Ttstart/tstop/tinc */
					double min, max, inc;
					min = atof (txt_a);	max = atof (txt_b);	inc = atof (txt_c);
					if (opt->arg[strlen(opt->arg)-1] == '+')	/* Gave number of points instead; calculate inc */
						inc = (max - min) / (inc - 1.0);
					if (inc <= 0.0) {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Age increment must be positive\n");
						n_errors++;
					}
					else {
						Ctrl->T.n_times = lrint ((max - min) / inc) + 1;
						Ctrl->T.value = gmt_M_memory (GMT, NULL, Ctrl->T.n_times, double);
						for (t = 0; t < Ctrl->T.n_times; t++) Ctrl->T.value[t] = (t == (Ctrl->T.n_times-1)) ? max: min + t * inc;
					}
				}
				else {	/* Got a single time */
					Ctrl->T.n_times = 1;
					Ctrl->T.value = gmt_M_memory (GMT, NULL, Ctrl->T.n_times, double);
					Ctrl->T.value[0] = atof (txt_a);
				}
				break;

			case 'W':	/* Use weights */
				Ctrl->W.active = true;
				break;
			case 'Z':	/* Report negative opening angles */
				Ctrl->Z.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	if (!Ctrl->A.active) n_in++;	/* Got time in input column 3 */
	if (Ctrl->W.active) n_in++;		/* Got weights as extra column */
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = n_in;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < n_in, "Syntax error: Binary input data (-bi) must have at least %u columns\n", n_in);
	n_errors += gmt_M_check_condition (GMT, (Ctrl->N.active + Ctrl->S.active + Ctrl->W.active) > 1, "Syntax error: Only one of -N, -S, -Z can be set.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int compare_ages (const void *point_1v, const void *point_2v) {
	struct AGEROT *point_1, *point_2;

	point_1 = (struct AGEROT *)point_1v;
	point_2 = (struct AGEROT *)point_2v;
	if (point_1->wxyasn[K_AGE] < point_2->wxyasn[K_AGE]) return (-1);
	if (point_1->wxyasn[K_AGE] > point_2->wxyasn[K_AGE]) return (+1);
	return (0);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_rotsmoother (void *V_API, int mode, void *args) {
	bool stop;
	uint64_t n_read = 0, rot, p, first = 0, last, n_use = 0, n_out = 0, n_total_use = 0, n_minimum, n_alloc = GMT_CHUNK;
	int error = 0, n_fields;
	unsigned int n_in = 3, k, j, t_col, w_col, t, n_cols = 4, matrix_dim = 3, nrots;
	double *in = NULL, min_rot_angle, min_rot_age, max_rot_angle, max_rot_age;
	double sum_rot_angle, sum_rot_angle2, sum_rot_age, sum_rot_age2, sum_weights;
	double out[20], khat = 1.0, g = 1.0e-5;	/* Common scale factor for all Covariance terms */
	double sa2, ca2, qlength;
	double z, mean_rot_angle, std_rot_angle, this_weight, x_comp, y_comp, med_angle;
	double azimuth, norm, t_lo, t_hi, rot_angle_in_radians, this_rot_angle, lon_mean_pole, lat_mean_pole;
	double x_in_plane[3], y_in_plane[3], C[9], EigenValue[3], EigenVector[9], work1[3], work2[3];
	double R[3][3], DR[3][3], Ri[3][3], E[3], this_h[3], xyz_mean_pole[3], xyz_mean_quat[4], z_unit_vector[3];
	double mean_rot_age, std_rot_age, *H[3], mean_H[3], Ccopy[9], *mangle = NULL, this_lon, this_lat;
	struct AGEROT *D = NULL;
	struct ROTSMOOTHER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args); if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = gmt_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the rotsmoother main code ----------------------------*/

	gmt_set_geographic (GMT, GMT_IN);	/* In and out are lon/lat */
	gmt_set_geographic (GMT, GMT_OUT);	/* In and out are lon/lat */

	/* Read the rotation data from file or stdin */

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");

	if (!Ctrl->A.active) n_in++;	/* Got time */
	if (Ctrl->W.active) n_in++;		/* Got weights */
	if (Ctrl->C.active) n_cols = 19;	/* Want everything */

	/* Specify input and output expected columns */
	if ((error = gmt_set_cols (GMT, GMT_IN, n_in)) != GMT_NOERROR) {
		Return (error);
	}
	/* Initialize the i/o for doing record-by-record reading/writing */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET,  GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}
	t_col = (Ctrl->A.active) ? GMT_Z : 3;	/* If no time we use angle as proxy for time */
	w_col = t_col + 1;
	D = (struct AGEROT *) gmt_M_memory (GMT, NULL, n_alloc, struct AGEROT);

	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				gmt_M_free (GMT, D);
				Return (GMT_RUNTIME_ERROR);
			}
			if (gmt_M_rec_is_table_header (GMT)) {	/* Skip all table headers */
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, NULL);
				continue;
			}
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			else if (gmt_M_rec_is_new_segment (GMT)) {			/* Parse segment headers */
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
				continue;
			}
		}

		/* Convert to geocentric, load parameters  */
		D[n_read].wxyasn[K_LON]    = in[GMT_X];
		D[n_read].wxyasn[K_LAT]    = gmt_lat_swap (GMT, in[GMT_Y], GMT_LATSWAP_G2O);
		D[n_read].wxyasn[K_ANGLE]  = in[GMT_Z];
		D[n_read].wxyasn[K_AGE]    = in[t_col];
		D[n_read].wxyasn[K_WEIGHT] = (Ctrl->W.active) ? in[w_col] : 1.0;	/* Optionally use weights */
		n_read++;
		if (n_read == n_alloc) {	/* Need larger arrays */
			n_alloc <<= 1;
			D = gmt_M_memory (GMT, D, n_alloc, struct AGEROT);
		}
	} while (true);

	if (n_read < n_alloc) D = gmt_M_memory (GMT, D, n_read, struct AGEROT);

	if (GMT_End_IO (API, GMT_IN,  0) != GMT_NOERROR) {	/* Disables further data input */
		gmt_M_free (GMT, D);
		Return (API->error);
	}

	min_rot_angle = min_rot_age = DBL_MAX;	max_rot_angle = max_rot_age = -DBL_MAX;
	for (rot = 0; rot < n_read; rot++) {	/* Process rotations and make pseudo-vectors and quaternions */
		if (D[rot].wxyasn[K_ANGLE] < min_rot_angle) min_rot_angle = D[rot].wxyasn[K_ANGLE];
		if (D[rot].wxyasn[K_ANGLE] > max_rot_angle) max_rot_angle = D[rot].wxyasn[K_ANGLE];
		if (D[rot].wxyasn[K_AGE] < min_rot_age) min_rot_age = D[rot].wxyasn[K_AGE];
		if (D[rot].wxyasn[K_AGE] > max_rot_age) max_rot_age = D[rot].wxyasn[K_AGE];
		gmt_geo_to_cart (GMT, D[rot].wxyasn[K_LAT], D[rot].wxyasn[K_LON], D[rot].P, true);	/* Get unit vector on sphere */
		rot_angle_in_radians = D[rot].wxyasn[K_ANGLE] * D2R;	/* pseudo-vector length in radians */
		/* Get quarternion by scaling the unit vector by sine of half the opening angle */
		sincos (0.5 * rot_angle_in_radians, &sa2, &ca2);
		D[rot].Q[0] = ca2;	/* Store quarternion constant in first slot. */
		for (k = 0; k < 3; k++) {	/* Compute both pseudo-vector and quarternion components */
			D[rot].Q[k+1] = D[rot].P[k] * sa2;		/* Quarternion representation */
			D[rot].P[k] *= rot_angle_in_radians;	/* Pseudo-vector representation */
		}
	}

	if (!Ctrl->A.active) GMT_Report (API, GMT_MSG_VERBOSE, "Range of input ages   = %g/%g\n", min_rot_age, max_rot_age);
	GMT_Report (API, GMT_MSG_VERBOSE, "Range of input angles = %g/%g\n", min_rot_angle, max_rot_angle);

	/* Sort the entire dataset on increasing ages */
	
	qsort (D, n_read, sizeof (struct AGEROT), compare_ages);
	
	if ((error = gmt_set_cols (GMT, GMT_OUT, n_cols)) != GMT_NOERROR) {
		gmt_M_free (GMT, D);
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		gmt_M_free (GMT, D);
		Return (API->error);
	}

	/* Sort the entire dataset on increasing ages */

	qsort (D, n_read, sizeof (struct AGEROT), compare_ages);

	if (GMT->common.h.add_colnames) {	/* Create meaningful column header */
		static char *short_header = "lon\tlat\ttime\tangle";
		static char *long_header = "lon\tlat\ttime\tangle\tk_hat\ta\tb\tc\td\te\tf\tg\tdf\tstd_t\tstd_w\taz\tS1\tS2\tS3";
		char *header = (Ctrl->C.active) ? long_header : short_header;
		if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, NULL)) {
			gmt_M_free (GMT, D);
			Return (API->error);
		}
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		gmt_M_free (GMT, D);
		Return (API->error);
	}

	z_unit_vector[0] = z_unit_vector[1] = 0.0;	z_unit_vector[2] = 1.0;	/* The local z unit vector */
	n_minimum = (Ctrl->C.active) ? 2 : 1;	/* Need at least two rotations to compute covariance, at least one to report the mean */

	for (t = 1; t < Ctrl->T.n_times; t++) {	/* For each desired output time interval */
		t_lo = Ctrl->T.value[t-1];	t_hi = Ctrl->T.value[t];	/* The current interval */
		for (rot = first, stop = false; !stop && rot < n_read; rot++)	/* Determine index of first rotation inside this age window */
			if (D[rot].wxyasn[K_AGE] >= t_lo) stop = true;
		first = rot - 1;	/* Index to first rotation inside this time interval */
		for (stop = false; !stop && rot < n_read; rot++)	/* Determine index of last rotation inside this age window */
			if (D[rot].wxyasn[K_AGE] > t_hi) stop = true;
		last = rot - 1;	/* Index to first rotation outside this time interval */
		n_use = last - first;	/* Number of rotations in the interval */
		GMT_Report (API, GMT_MSG_VERBOSE, "Found %d rots for the time interval %g <= t < %g\n", n_use, t_lo, t_hi);
		if (n_use < n_minimum) continue;	/* Need at least 1 or 2 poles to do anything useful */

		/* Now extimate the average rotation */

		gmt_M_memset (xyz_mean_pole, 3, double);	/* Reset sum of mean components and weight sums */
		gmt_M_memset (xyz_mean_quat, 4, double);	/* Reset sum of mean components and weight sums */
		sum_rot_angle = sum_rot_angle2 = sum_rot_age = sum_rot_age2 = sum_weights = 0.0;
		for (rot = first; rot < last; rot++) {	/* Loop over every rotation in this time interval... */
			this_weight = D[rot].wxyasn[K_WEIGHT];	/* Either use weights or it is already set to 1 */
			for (k = 0; k < 4; k++) xyz_mean_quat[k] += (this_weight * D[rot].Q[k]);	/* Sum up the [weighted] quarternion components */
			for (k = 0; k < 3; k++) xyz_mean_pole[k] += (this_weight * D[rot].P[k]);	/* Sum up the [weighted] Cartesian components */
			z = this_weight * D[rot].wxyasn[K_ANGLE];	/* Separately sum up opening [weighted] angle stats */
			sum_rot_angle += z;
			sum_rot_angle2 += (z * D[rot].wxyasn[K_ANGLE]);
			sum_weights += this_weight;
			z = this_weight * D[rot].wxyasn[K_AGE];	/* Separately sum up opening [weighted] age stats */
			sum_rot_age += z;
			sum_rot_age2 += (z * D[rot].wxyasn[K_AGE]);
		}

		/* Get [weighted] mean age */
		mean_rot_age = sum_rot_age / sum_weights;
		std_rot_age = sqrt ((sum_weights * sum_rot_age2 - sum_rot_age * sum_rot_age) / (sum_weights * sum_weights * ((n_use - 1.0) / n_use)));
		/* Get [weighted] mean angle */
		mean_rot_angle = sum_rot_angle / sum_weights;
		std_rot_angle = sqrt ((sum_weights * sum_rot_angle2 - sum_rot_angle * sum_rot_angle) / (sum_weights * sum_weights * ((n_use - 1.0) / n_use)));

		/* Get Euclidian [weighted] mean pseudo-vector location */
		for (k = 0; k < 3; k++) xyz_mean_pole[k] /= sum_weights;	/* Components of [weighted] mean pseudovector */
		gmt_normalize3v (GMT, xyz_mean_pole);	/* Make it a unit vector ... */

		/* Get [weighted] mean quaternion */
		for (k = 0; k < 4; k++) xyz_mean_quat[k] /= sum_weights;	/* Components of mean quaternion */
		for (k = 0, qlength = 0.0; k < 4; k++) qlength += xyz_mean_quat[k] * xyz_mean_quat[k];	/* Sum squared components of total quaternion */
		qlength = sqrt (qlength);	/* Length of the quaternion */
		for (k = 0; k < 4; k++) xyz_mean_quat[k] /= qlength;		/* Components of mean unit beast */
		mean_rot_angle = 2.0 * d_acos (xyz_mean_quat[0]) * R2D;		/* Rotation (in degrees) represented by mean quaternion */
		norm = d_sqrt (1.0 - xyz_mean_quat[0] * xyz_mean_quat[0]);	/* This is sin (angle/2) */
		for (k = 1; k < 4; k++) xyz_mean_quat[k] /= norm;		/* Unit vector of mean quaternion */
		gmt_cart_to_geo (GMT, &lat_mean_pole, &lon_mean_pole, &xyz_mean_quat[1], true);	/* Convert it to lon/lat pole */
		mangle = (double *) gmt_M_memory (GMT, NULL, n_use, double);	/* For median angle calculation we need to make an array */
		/* Now compute the median of the opening angle */
		for (rot = first, p = 0; rot < last; rot++, p++)	/* For each angle of rotation in this rotation interval... */
			mangle[p] = D[rot].wxyasn[K_ANGLE];		/* Place angles in temp array for computing median angle */
		gmt_median (GMT, mangle, n_use, min_rot_angle, max_rot_angle, 0.5 * (min_rot_angle + max_rot_angle), &med_angle);
		gmt_M_free (GMT, mangle);

		if ((Ctrl->N.active && mean_rot_angle > 0.0) || (Ctrl->N.active && lat_mean_pole < 0.0) || (!Ctrl->N.active && lat_mean_pole > 0.0)) {
			/* Depending on -S, -N, -W. flip pole to right hemisphere or sign of angle */
			lat_mean_pole = -lat_mean_pole;
			lon_mean_pole += 180.0;
			mean_rot_angle = -mean_rot_angle;
			while (lon_mean_pole > 180.0) lon_mean_pole -= 360.0;
		}
		/* Prepare output array for the mean rotation */
		out[GMT_X] = lon_mean_pole;
		out[GMT_Y] = gmt_lat_swap (GMT, lat_mean_pole, GMT_LATSWAP_O2G);	/* Convert back from geocentric to geodetic */
		out[GMT_Z] = mean_rot_age;	out[3] = mean_rot_angle;
		GMT_Report (API, GMT_MSG_VERBOSE, "Mean opening angle %8.4f vs median opening angle %8.4f\n", mean_rot_angle, med_angle);	/* Report mean and median angle for testing */
		n_total_use += n_use;
		n_out++;

		if (!Ctrl->C.active) {	/* No covariance requested, print this rotation and continue */
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
			continue;
		}

		/* Now get the covariance matrix */

		/* Here we also want to get the covariance matrix.  To do so we want to parameterize all
		 * the n_use rotations as a small incremental rotation DR_i followed by the mean rotation
		 * R, i.e., R_i = R * DR_i.  We must therefore first solve for the incremental rotation
		 * DR_i (a 3x3 matrix) which becomes R^T * R_i and then convert it to a pseudo-vector h
		 * so that we can calculate covariances.  Because the incremental rotation angles are
		 * very small (|pseudo-vector| << 1 the pseudo-vector approach is warranted. */

		for (k = 0; k < 3; k++) H[k] = gmt_M_memory (GMT, NULL, n_use, double);
		gmt_M_memset (mean_H, 3, double);	/* Zero out the mean sum matrix for pseudovectors */
		gmt_make_rot_matrix (GMT, lon_mean_pole, lat_mean_pole, -mean_rot_angle, R);	/* Mean rotation R; use -ve angle since we actually need R^T */
		for (rot = first, p = 0; rot < last; rot++, p++) {	/* For each individually determined pole of rotation in this rotation interval... */
			gmt_M_memcpy (E, D[rot].P, 3, double);			/* Set up the rotation pseudo-vector E */
			this_rot_angle = gmt_mag3v (GMT, E) * R2D;		/* Magnitude of this vector -> opening in degrees */
			gmt_normalize3v (GMT, E);						/* Make rotation pole a unit vector */
			gmt_make_rot_matrix2 (GMT, E, this_rot_angle, Ri);		/* Compute this individual rotation matrix R_i */
			spotter_matrix_mult (GMT, R, Ri, DR);				/* The incremental rotation DR_i = R^T * R_i */
			spotter_matrix_to_pole (GMT, DR, &this_lon, &this_lat, &this_rot_angle);	/* Get (lon, lat, angle) of the incremental rotation */
			gmt_geo_to_cart (GMT, this_lat, this_lon, this_h, true);	/* Make unit vector this_h for incremental rotation pole */
			rot_angle_in_radians = this_rot_angle * D2R;		/* Scale to get pseudo-vector with length in radians */
			for (k = 0; k < 3; k++) {	/* For each x,y,z component of pseudo-vector this_h */
				H[k][p] = this_h[k] * rot_angle_in_radians;	/* Final pseudo-vector for the rotation increment */
				mean_H[k] += H[k][p];	/* Sum of x,y,z components of all such pseudo-vectors */
			}
		}

		gmt_M_memset (C, 9, double);		/* Zero out the covariance matrix */
		for (k = 0; k < 3; k++) mean_H[k] /= n_use;		/* The mean pseudo-vector */
		for (p = 0; p < n_use; p++) {					/* Calculate the covariance matrix */
			for (k = 0; k < 3; k++) H[k][p] -= mean_H[k];		/* Remove mean component from each vector */
			for (j = 0; j < 3; j++) for (k = 0; k < 3; k++) {
				C[3*j+k] += (H[j][p]) * (H[k][p]);		/* Products of deviations from each mean */
			}
		}
		for (k = 0; k < 9; k++) C[k] /= (n_use - 1.0);		/* Normalize to get final var-covar matrix, C */
		for (k = 0; k < 3; k++) gmt_M_free (GMT, H[k]);		/* Release temporary memory */

		/* We also want to get a 95% ellipse projected onto a tangent plane to the surface of Earth at the pole */

		gmt_M_memcpy (Ccopy, C, 9, double);	/* Duplicate since it will get trodden on */
		if (gmt_jacobi (GMT, Ccopy, matrix_dim, matrix_dim, EigenValue, EigenVector, work1, work2, &nrots)) {	/* Solve eigen-system C = EigenVector * EigenValue * EigenVector^T */
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: Eigenvalue routine gmt_jacobi failed to converge in 50 sweeps.\n");
		}

		/* In addition to reporting the covariance, we will report the azimuth of the major axis and the
		 * lengths of the major and minor axes as they appear when projected onto the local plane tangent
		 * to the surface at the mean pole.  Note that EigenVector[0], EigenVector[1], EigenVector[2] are
		 * the components of the 1st eigenvector. */

		gmt_cross3v (GMT, z_unit_vector, xyz_mean_pole, x_in_plane);	/* Local x-axis in plane normal to mean pole */
		gmt_cross3v (GMT, xyz_mean_pole, x_in_plane, y_in_plane);	/* Local y-axis in plane normal to mean pole */
		x_comp = gmt_dot3v (GMT, EigenVector, x_in_plane);		/* x-component of major axis in tangent plane */
		y_comp = gmt_dot3v (GMT, EigenVector, y_in_plane);		/* y-component of major axis in tangent plane */
		azimuth = fmod (360.0 + (90.0 - atan2 (y_comp, x_comp) * R2D), 360.0);	/* Azimuth of major axis */
		if (azimuth > 180.0) azimuth -= 180.0;
		/* Get the three axes lengths from the eigenvalues scaled from radians to km */
		EigenValue[0] = sqrt (EigenValue[0]/khat) * EQ_RAD * SQRT_CHI2;
		EigenValue[1] = sqrt (EigenValue[1]/khat) * EQ_RAD * SQRT_CHI2;
		EigenValue[2] = sqrt (EigenValue[2]/khat) * EQ_RAD * SQRT_CHI2;

		/* Report the results.  It is conventional to define the covariance matrix as follows:
		 *
		 *                        [ a b d ]
		 * C = cov (u) = k^(-1) * | b c e | * g
		 *                        [ d e f ]
		 *
		 * where k is a scale factor that relates true estimate of uncertainty in the data to the assigned
		 * estimate.  For this work we have no uncertainty estimate so k == 1.  a-f are six unique entries in the
		 * symmetric C matrix, and g is just a common scale factor to avoid too many decimal places in the
		 * tables; typically g is 1e-5 ,which we adopt here.  The output is thus written in the order k, a-g,
		 * followed by d.f., the degrees of freedom.
		 */

		for (k = 0; k < 9; k++) C[k] /= g;	/* Take out the common factor */
		 /* Place khat a b c d e f g df */
		out[4] = khat;
		out[5] = C[0];
		out[6] = C[1];
		out[7] = C[4];
		out[8] = C[2];
		out[9] = C[5];
		out[10] = C[8];
		out[11] = g;
		out[12] = (double)n_use - 1;
		out[13] = std_rot_age;	/* Add std_age and std_angle */
		out[14] = std_rot_angle;
		out[15] = azimuth;	/* Axes and orientation for surface 95% error ellipse */
		out[16] = EigenValue[0];
		out[17] = EigenValue[1];
		out[18] = EigenValue[2];
		GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	if (n_out == 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "No smoothed poles created - is filterwidth too small?\n");
		if (n_total_use != n_read) GMT_Report (API, GMT_MSG_NORMAL, "Read %ld poles, used %7ld\n", n_read, n_total_use);
	}

	/* Free up memory used for the array */

	gmt_M_free (GMT, D);

	Return (GMT_NOERROR);
}
