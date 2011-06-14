/*--------------------------------------------------------------------
 *	$Id: originator_func.c,v 1.16 2011-06-14 22:59:53 guru Exp $
 *
 *   Copyright (c) 2000-2011 by P. Wessel
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
 * originator reads file of seamount locations and tries to match each
 * seamount with a probable hotspot by drawing flowines back in time and
 * keeping track of which hotspot is closest to each flowline.  It then
 * reports the closest hotspot, the stage of the flowline involved, the
 * implied pseudo-age of the seamount, and the minimum distance between
 * the flowline and hotspot (in km).
 *
 * Author:	Paul Wessel, SOEST, Univ. of Hawaii, Honolulu, HI, USA
 * Date:	29-DEC-1999
 * Version:	1.0
 *
 *-------------------------------------------------------------------------
 * An ASCII stage pole (Euler) file must have following format:
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
 * AN ASCII total reconstruction file must have the following format:
*
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 2. Any number of finite pole records which each have the format:
 *    lon(deg)  lat(deg)  tstop(Ma)  ccw-angle(deg)
 * 3. total reconstruction rotations must go from youngest to oldest
 *
 * Example: Duncan & Clague [1985] Pacific-Hotspot rotations:
 *
 * # Time in Ma, angles in degrees
 * #longitude	latitude	time(My)	angle(deg)
 * 285.00000	 68.00000	 42.0000	 34.0000
 * 275.66205	 53.05082	 65.0000	 43.5361
 * 276.02501	 48.34232	 74.0000	 50.0405
 * 279.86436	 46.30610	100.0000	 64.7066
 * 265.37800	 55.69932	150.0000	 82.9957
 *
 * ASCII seamount location file(s) must have the following format:
 *
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 3. For special header records, see -H
 * 4. Any number of data records which each have the format:
 *    lon lat height radius crustal_age    (or lat lon ..., see -: option).
 *    crustal_age in Ma, height and radius are not used by originator but
 *    are used by hotspotter.
 *
 * Binary files cannot have header records, and data fields must all be
 * either single or double precision (see -bi option).  Output file will
 * be ASCII since it contains a text string (hotspot ID).
 *
 * The file with a list of hotspots must have the following format:
 *
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 2. Any number of hotspot records which each have the format:
 *    lon(deg)  lat(deg)  id  name
 *    the id is a 3-character tag (e.g., HWI), the name is the
 *    full name of the hotspot (e.g., Hawaii).
 *
 * Example: 
 *
 * # Partial list (Pacific) of HotSpots from Table 1 of Yamaji, 1992
 * #Lon		Lat	Abbreviation	Hotspot_name
 * 167		3	CRL	Caroline
 * 230		46	COB	Cobb
 * 205		20	HWI	Hawaii
 * 221.9	-50.9	LSV	Louisville
 * 220		-29	MDN	MacDonald
 * 221		-11	MRQ	Marquesas
 * 231		-27	PTC	Pitcairn
 * 254		-27	SLG	Sala y Gomez
 * 192		-15	SAM	Samoa
 * 212		-18	SOC	Society
 */
 
#include "spotter.h"
#include "gmt_proj.h"

EXTERN_MSC double GMT_great_circle_dist_degree (struct GMT_CTRL *C, double x0, double y0, double x1, double y1);
EXTERN_MSC GMT_LONG GMT_great_circle_intersection (struct GMT_CTRL *T, double A[], double B[], double C[], double X[], double *CX_dist);

#define KM_PR_RAD (R2D * GMT->current.proj.DIST_KM_PR_DEG)

struct HOTSPOT_ORIGINATOR {
	struct HOTSPOT *h;	/* Pointer to regular HOTSPOT structure */
	/* Extra variables needed for this program */
	double np_dist;		/* Distance to nearest point on the current flowline */
	double np_sign;		/* "Sign" of this distance (see code) */
	double np_time;		/* Predicted time at nearest point */
	double np_lon;		/* Longitude of nearest point on the current flowline */
	double np_lat;		/* Latitude  of nearest point on the current flowline */
	int nearest;		/* Point id of current flowline node points closest to hotspot */
	int stage;		/* Stage to which seamount belongs */
};

struct ORIGINATOR_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct D {	/* -D<factor */
		GMT_LONG active;
		double value;	
	} D;
	struct E {	/* -Erotfile */
		GMT_LONG active;
		GMT_LONG mode;
		char *file;
	} E;
	struct F {	/* -Ehotspotfile */
		GMT_LONG active;
		char *file;
	} F;
	struct L {	/* -L */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG degree;	/* Report degrees */
	} L;
	struct N {	/* -N */
		GMT_LONG active;
		double t_upper;
	} N;
	struct Q {	/* -Q<tfix> */
		GMT_LONG active;
		double t_fix, r_fix;
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
		GMT_LONG n;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
	} T;
	struct W {	/* -W<max_dist> */
		GMT_LONG active;
		double dist;
	} W;
	struct Z {	/* -Z */
		GMT_LONG active;
	} Z;
};

void *New_originator_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct ORIGINATOR_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct ORIGINATOR_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	C->D.value = 5.0;
	C->N.t_upper = 180.0;
	C->S.n = 1;
	C->W.dist = 1.0e100;
	return ((void *)C);
}

void Free_originator_Ctrl (struct GMT_CTRL *GMT, struct ORIGINATOR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->E.file) free ((void *)C->E.file);	
	if (C->F.file) free ((void *)C->F.file);	
	GMT_free (GMT, C);	
}

int comp_hs (const void *p1, const void *p2)
{
	struct HOTSPOT_ORIGINATOR *a, *b;

	a = (struct HOTSPOT_ORIGINATOR *) p1;
	b = (struct HOTSPOT_ORIGINATOR *) p2;
	if (a->np_dist < b->np_dist) return (-1);
	if (a->np_dist > b->np_dist) return (+1);
	return (0);
}

GMT_LONG GMT_originator_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "originator %s [API] -  Associate seamounts with hotspot point sources\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: originator [<table>] -E[+]<rottable> -F<hotspottable> [-D<d_km>]\n");
	GMT_message (GMT, "\t[-H] [-L[<flag>]] [-N<upper_age>] [-Qr/t] [-S<n_hs>] [-T] [%s] [-W<maxdist>] [-Z]\n", GMT_V_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s]\n\n", GMT_bi_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-E specifies the rotation file to be used (see man page for format)\n\n");
	GMT_message (GMT, "\t   Prepend + if you want to invert the rotations prior to use\n\n");
	GMT_message (GMT, "\t-F Specify file name for hotspot locations.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t<table> (in ASCII, binary, or netCDF) has 5 or more columns.  If no file(s) is given, standard input is read.\n");
	GMT_message (GMT, "\t   Expects (x,y,z,r,t) records, with t in Ma\n");
	GMT_message (GMT, "\t-D set sampling interval in km along tracks [5].\n");
	GMT_message (GMT, "\t-L Output information for closest approach for nearest hotspot only (ignores -S).\n");
	GMT_message (GMT, "\t   -Lt gives (time, dist, z) [Default].\n");
	GMT_message (GMT, "\t   -Lw gives (omega, dist, z).\n");
	GMT_message (GMT, "\t   -Ll gives (lon, lat, time, dist, z).\n");
	GMT_message (GMT, "\t   dist is in km; use upper case T,W,L to get dist in spherical degrees.\n");
	GMT_message (GMT, "\t-N set age (in m.y.) for seafloor where age == NaN [180].\n");
	GMT_message (GMT, "\t-Q input files has (x,y,z) only. Append constant r/t to use.\n");
	GMT_message (GMT, "\t-S Report the <n_hs> closest hotSpots [1].\n");
	GMT_message (GMT, "\t-T Truncate seamount ages exceeding the upper age set with -N [no truncation] \n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Only report seamounts whose closest encounter to a hotspot is less than <maxdist> km\n");
	GMT_message (GMT, "\t   [Default reports for all seamounts] \n");
	GMT_message (GMT, "\t-Z Write hotspot ID number rather than hotspot TAG\n");
	GMT_explain_options (GMT, "C5hi:");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_originator_parse (struct GMTAPI_CTRL *C, struct ORIGINATOR_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to originator and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, k, n_input;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Supplemental parameters */
#ifdef GMT_COMPAT
			case 'C':	/* Now done automatically in spotter_init */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -C is no longer needed as total reconstruction vs stage rotation is detected automatically.\n");
				break;
#endif
			case 'D':
				Ctrl->D.active = TRUE;
				Ctrl->D.value = atof (opt->arg);
				break;
			case 'E':
				Ctrl->E.active = TRUE;	k = 0;
				if (opt->arg[0] == '+') { Ctrl->E.mode = TRUE; k = 1;}
				Ctrl->E.file  = strdup (&opt->arg[k]);
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				Ctrl->F.file = strdup (opt->arg);
				break;
			case 'L':
				Ctrl->L.active = TRUE;
				switch (opt->arg[0]) {
					case 'L':
						Ctrl->L.degree = TRUE;
					case 'l':
						Ctrl->L.mode = 3;
						break;
					case 'w':
					case 'W':
						Ctrl->L.mode = 2;
						break;
					case 'T':
						Ctrl->L.degree = TRUE;
					case 't':
						Ctrl->L.mode = 1;
						break;
					default:
						Ctrl->L.mode = 1;
						break;
				}
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				Ctrl->N.t_upper = atof (opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = TRUE;
				sscanf (opt->arg, "%lg/%lg", &Ctrl->Q.r_fix, &Ctrl->Q.t_fix);
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				Ctrl->S.n = atoi (opt->arg);
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				break;
			case 'W':
				Ctrl->W.active = TRUE;
				Ctrl->W.dist = atof (opt->arg);
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_input = (Ctrl->Q.active) ? 3 : 5;
        if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = n_input;

	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < n_input, "Syntax error: Binary input data (-bi) must have at least %ld columns\n", n_input);
	n_errors += GMT_check_condition (GMT, !Ctrl->F.file, "Syntax error -F: Must specify hotspot file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->E.file, "Syntax error -F: Must specify Euler pole file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.value <= 0.0, "Syntax error -D: Must specify a positive interval\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.dist <= 0.0, "Syntax error -W: Must specify a positive distance in km\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_originator_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_originator (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG n_max_spots, n_input, n_fields, n_expected_fields, n_out, better;
	GMT_LONG i, j, k, n, kk, ns, nh, nc, np, n_read, n_skipped = 0, error = FALSE;

	double x_smt, y_smt, z_smt, r_smt, t_smt, *c, *in = NULL, dist, dlon, out[5];
	double hx_dist, hx_dist_km, dist_NA, dist_NX, del_dist, dt = 0.0, A[3], H[3], N[3], X[3];

	char record[GMT_BUFSIZ], buffer[GMT_BUFSIZ];

	struct EULER *p = NULL;
	struct HOTSPOT *orig_hotspot = NULL;
	struct HOTSPOT_ORIGINATOR *hotspot = NULL, *hot = NULL;
	struct GMT_OPTION *ptr = NULL;
	struct ORIGINATOR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_originator_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_originator_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_originator", &GMT_cpy);				/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vbf:", "ghis>" GMT_OPT("HMm"), options))) Return (error);
	if (GMT_Find_Option (API, 'f', options, &ptr)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	Ctrl = (struct ORIGINATOR_CTRL *) New_originator_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_originator_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the originator main code ----------------------------*/

	GMT_lat_swap_init (GMT);	/* Initialize auxiliary latitude machinery */

	nh = spotter_hotspot_init (GMT, Ctrl->F.file, TRUE, &orig_hotspot);	/* Get geocentric hotspot locations */
	if (Ctrl->S.n <= 0 || Ctrl->S.n > nh) {
		GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -S option: Give value between 1 and %ld\n", nh);
		Return (EXIT_FAILURE);
	}
	n_max_spots = MIN (Ctrl->S.n, nh);

	hotspot = GMT_memory (GMT, NULL, nh, struct HOTSPOT_ORIGINATOR);
	for (i = 0; i < nh; i++) {
		hotspot[i].h = &orig_hotspot[i];	/* Point to the original hotspot structures */
		hotspot[i].np_dist = 1.0e100;
	}
	
	ns = spotter_init (GMT, Ctrl->E.file, &p, TRUE, FALSE, Ctrl->E.mode, &Ctrl->N.t_upper);

	hot = GMT_memory (GMT, NULL, nh, struct HOTSPOT_ORIGINATOR);

	n_input = (Ctrl->Q.active) ? 3 : 5;
	n_expected_fields = (GMT->common.b.ncol[GMT_IN]) ? GMT->common.b.ncol[GMT_IN] : n_input;
	n = 0;
	if (Ctrl->L.active) {
		n_out = (Ctrl->L.mode == 3) ? 5 : 3;
	}
	else
		n_out = n_expected_fields;
	if (n_out == 3) {
		GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_FLOAT;	/* NO lon/lat out */
	}
	if ((error = GMT_set_cols (GMT, GMT_IN, n_out))) Return (error);
	if ((error = GMT_set_cols (GMT, GMT_OUT, n_expected_fields))) Return (error);
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data output */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN,  GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);	/* Enables data output and sets access mode */

	n_read = 0;
	while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we reach EOF */

		if (GMT_REC_IS_ERROR (GMT) && n_fields < 2) continue;

		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Echo table headers */

		if (n_input == 3) {	/* set constant r,t values */
			in[3] = Ctrl->Q.r_fix;
			in[4] = Ctrl->Q.t_fix;
		}
		if (GMT_is_dnan (in[4]))	/* Age is NaN, assign value */
			t_smt = Ctrl->N.t_upper;
		else {			/* Assign given value, truncate if necessary */
			t_smt = in[4];
			if (t_smt > Ctrl->N.t_upper) {
				if (Ctrl->T.active)
					t_smt = Ctrl->N.t_upper;
				else {
					GMT_report (GMT, GMT_MSG_NORMAL, "Seamounts near line %ld has age (%g) > oldest stage (%g) (skipped)\n", n_read, t_smt, Ctrl->N.t_upper);
					continue;
				}
			}
		}
		if (t_smt < 0.0) {	/* Negative ages are flags for points to be skipped */
			n_skipped++;
			continue;
		}
		y_smt = D2R * GMT_lat_swap (GMT, in[GMT_Y], GMT_LATSWAP_G2O);	/* Convert to geocentric, and radians */
		x_smt = in[GMT_X] * D2R;
		z_smt = in[GMT_Z];
		r_smt = in[3];

		if (!(n % 10)) GMT_report (GMT, GMT_MSG_NORMAL, "Working on seamount # %5ld\r", n);

		nc = spotter_forthtrack (GMT, &x_smt, &y_smt, &t_smt, 1, p, ns, Ctrl->D.value, 0.0, TRUE, NULL, &c);

		np = (GMT_LONG) c[0];

		GMT_memcpy (hot, hotspot, nh, struct HOTSPOT_ORIGINATOR);

		for (kk = 0, k = 1; kk < np; kk++, k += 3) {	/* For this seamounts track */
			for (j = 0; j < nh; j++) {	/* For all hotspots */
				dist = GMT_great_circle_dist_degree (GMT, hot[j].h->lon, hot[j].h->lat, R2D * c[k], R2D * c[k+1]);
				if (!Ctrl->L.degree) dist *= GMT->current.proj.DIST_KM_PR_DEG;
				if (dist < hot[j].np_dist) {
					hot[j].np_dist = dist;
					hot[j].nearest = (int)kk;	/* Index of nearest point on the flowline */
				}
			}
		}
		for (j = 0; j < nh; j++) {

			GMT_geo_to_cart (GMT, hot[j].h->lat, hot[j].h->lon, H, TRUE);	/* 3-D Cartesian vector of this hotspot */

			/* Fine-tune the nearest point by considering intermediate points along greatcircle between knot points */

			k = 3 * hot[j].nearest + 1;			/* Corresponding index for x into the (x,y,t) array c */
			GMT_geo_to_cart (GMT, c[k+1], c[k], N, FALSE);	/* 3-D vector of nearest node to this hotspot */
			better = FALSE;
			if (hot[j].nearest > 0) {	/* There is a point along the flowline before the nearest node */
				GMT_geo_to_cart (GMT, c[k-2], c[k-3], A, FALSE);	/* 3-D vector of end of this segment */
				if (GMT_great_circle_intersection (GMT, A, N, H, X, &hx_dist) == 0) {	/* X is between A and N */
					hx_dist_km = d_acos (hx_dist) * KM_PR_RAD;
					if (hx_dist_km < hot[j].np_dist) {	/* This intermediate point is even closer */
						GMT_cart_to_geo (GMT, &hot[j].np_lat, &hot[j].np_lon, X, TRUE);
						hot[j].np_dist = hx_dist_km;
						dist_NA = d_acos (fabs (GMT_dot3v (GMT, A, N))) * KM_PR_RAD;
						dist_NX = d_acos (fabs (GMT_dot3v (GMT, X, N))) * KM_PR_RAD;
						del_dist = dist_NA - dist_NX;
						dt = (del_dist > 0.0) ? (c[k+2] - c[k-1]) * dist_NX / del_dist : 0.0;
						better = TRUE;
					}
				}
			}
			if (hot[j].nearest < (np-1) ) {	/* There is a point along the flowline after the nearest node */
				GMT_geo_to_cart (GMT, c[k+4], c[k+3], A, FALSE);	/* 3-D vector of end of this segment */
				if (GMT_great_circle_intersection (GMT, A, N, H, X, &hx_dist) == 0) {	/* X is between A and N */
					hx_dist_km = d_acos (hx_dist) * KM_PR_RAD;
					if (hx_dist_km < hot[j].np_dist) {	/* This intermediate point is even closer */
						GMT_cart_to_geo (GMT, &hot[j].np_lat, &hot[j].np_lon, X, TRUE);
						hot[j].np_dist = hx_dist_km;
						dist_NA = d_acos (fabs (GMT_dot3v (GMT, A, N))) * KM_PR_RAD;
						dist_NX = d_acos (fabs (GMT_dot3v (GMT, X, N))) * KM_PR_RAD;
						del_dist = dist_NA - dist_NX;
						dt = (del_dist > 0.0) ? (c[k+5] - c[k+2]) * dist_NX / del_dist : 0.0;
						better = TRUE;
					}
				}
			}
			if (better) {	/* Point closer to hotspot was found between nodes */
				hot[j].np_time = c[k+2] + dt;	/* Add time adjustment */
			}
			else {	/* Just use node coordinates */
				hot[j].np_lon  = c[k] * R2D;	/* Longitude of the flowline's closest approach to hotspot */
				hot[j].np_lat  = c[k+1] * R2D;	/* Latitude  of the flowline's closest approach to hotspot */
				hot[j].np_time = c[k+2];	/* Predicted time at the flowline's closest approach to hotspot */
			}

			/* Assign sign to distance: If the vector from the hotspot pointing up along the trail is positive
			 * x-axis and y-axis is normal to that, flowlines whose closest approach point's longitude is
			 * further east are said to have negative distance. */
			 
			dlon = fmod (hot[j].h->lon - hot[j].np_lon, 360.0);
			if (fabs (dlon) > 180.0) dlon = copysign (360.0 - fabs (dlon), -dlon);
			hot[j].np_sign = copysign (1.0, dlon);
			 
			/* Assign stage id for this point on the flowline */

			k = 0;
			while (k < ns && hot[j].np_time <= p[k].t_stop) k++;
			hot[j].stage = (int)(ns - k);
			if (hot[j].stage == 0) hot[j].stage++;
		}

		if (nh > 1) qsort ((void *)hot, (size_t)nh, sizeof(struct HOTSPOT_ORIGINATOR), comp_hs);

		if (hot[0].np_dist < Ctrl->W.dist) {
			if (Ctrl->L.mode == 1) {	/* Want time, dist, z output */
				out[0] = hot[0].np_time;
				out[1] = hot[0].np_dist * hot[0].np_sign;
				out[2] = z_smt;
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
			}
			else if (Ctrl->L.mode == 2) {	/* Want omega, dist, z output */
				out[0] = spotter_t2w (GMT, p, ns, hot[0].np_time);
				out[1] = hot[0].np_dist * hot[0].np_sign;
				out[2] = z_smt;
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
			}
			else if (Ctrl->L.mode == 3) {	/* Want x, y, time, dist, z output */
				out[GMT_X] = hot[0].np_lon;
				out[GMT_Y] = GMT_lat_swap (GMT, hot[0].np_lat, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
				out[2] = hot[0].np_time;
				out[3] = hot[0].np_dist * hot[0].np_sign;
				out[4] = z_smt;
				GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);
			}
			else {	/* Conventional originator output */
				if (t_smt == 180.0)
					strcpy (buffer, "NaN");
				else
					sprintf (buffer, "%g", t_smt);
				sprintf (record, "%g\t%g\t%g\t%g\t%s", in[GMT_X], in[GMT_Y], z_smt, r_smt, buffer);
				for (j = 0; j < n_max_spots; j++) {
					if (Ctrl->Z.active)
						sprintf (buffer, "\t%d\t%d\t%g\t%g", hot[j].h->id, hot[j].stage, hot[j].np_time, hot[j].np_dist);
					else
						sprintf (buffer, "\t%s\t%d\t%g\t%g", hot[j].h->abbrev, hot[j].stage, hot[j].np_time, hot[j].np_dist);
					strcat (record, buffer);
				}
				strcat (record, "\n");
				GMT_Put_Record (API, GMT_WRITE_TEXT, (void *)record);
			}
		}

		GMT_free (GMT, c);
		n++;
	}
	if ((error = GMT_End_IO (API, GMT_IN,  0))) Return (error);	/* Disables further data input */
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_report (GMT, GMT_MSG_NORMAL, "Working on seamount # %5ld\n", n);

	GMT_free (GMT, hotspot);
	GMT_free (GMT, orig_hotspot);
	GMT_free (GMT, hot);
	GMT_free (GMT, p);

	Return (GMT_OK);
}
