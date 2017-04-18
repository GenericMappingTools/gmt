/*--------------------------------------------------------------------
*	$Id$
*
*	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: mapproject reads a pair of coordinates [+ optional data fields] from
 * standard input or file(s) and transforms the coordinates according to the
 * map projection selected. See the man page for projections currently supported.
 *
 * The default is to expect longitude, latitude, [and optional datavalues],
 * and return x, y, [ and optional data values], but if the -I option is used,
 * the reverse is true.  Specifying -C means that the origin of projected coordinates
 * should be set to origin of projection  [Default origin is lower left corner of "map"].
 * If your data is lat lon instead of lon lat [Default], use -: to toggle x/y -> y/x.
 * Note that only unprojected values are affected by the -: switch.  True x,y values are
 * always printed out as x,y.  Option -G allows calculation of distances along track or
 * to a fixed point, while -L calculates shortest distances to a line.
 * Finally, datum conversions can also be done, alone or in series with a
 * map projection.
 *
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"mapproject"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Forward and inverse map transformations, datum conversions and geodesy"
#define THIS_MODULE_KEYS	"<D{,LD(=,>D},W-("

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>JRVbdfghiops" GMT_OPT("HMm")

enum GMT_mp_Gcodes {	/* Support for -G parsing */
	GMT_MP_FIXED_DIST = 1,	/* Compute distances from data to fixed point given by -Gx0/y0[...] */
	GMT_MP_CUMUL_DIST = 2,	/* Compute cumulative distances along track given by input data */
	GMT_MP_INCR_DIST  = 3,	/* Compute incremental distances along track given by input data */
	GMT_MP_PAIR_DIST  = 4	/* Compute distances from data to paired point given in data columns 3&4 */
};

enum GMT_mp_Lcodes {	/* Support for -L parsing */
	GMT_MP_GIVE_CORD = 2,	/* Return the coordinates of the nearest point on the line */
	GMT_MP_GIVE_FRAC = 3	/* Return the fractional line/point number of the nearest point on the line */
};

struct MAPPROJECT_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct A {	/* -Ab|B|f|Fb|B|o|O<lon0>/<lat0> */
		bool active;
		bool azims;
		bool orient;	/* true if we want orientations, not azimuths */
		bool reverse;	/* true if we want back-azimuths instead of regular azimuths */
		bool geodesic;	/* true if we want geodesic azimuths [Default is great circle azimuths] */
		double lon, lat;	/* Fixed point of reference */
	} A;
	struct C {	/* -C[<false_easting>/<false_northing>] */
		bool active;
		bool shift;
		double easting, northing;	/* Shifts */
	} C;
	struct D {	/* -D<c|i|p> */
		bool active;
		char unit;
	} D;
	struct E {	/* -E[<datum>] */
		bool active;
		struct GMT_DATUM datum;	/* Contains a, f, xyz[3] */
	} E;
	struct F {	/* -F[k|m|n|i|c|p] */
		bool active;
		char unit;
	} F;
	struct G {	/* -G<lon0>/<lat0>[d|e|f|k|m|M|n|s|c|C] */
		bool active;
		unsigned int mode;	/* 1 = distance to fixed point, 2 = cumulative distances, 3 = incremental distances, 4 = 2nd point in cols 3/4 */
		unsigned int sph;	/* 0 = Flat Earth, 1 = spherical [Default], 2 = ellipsoidal */
		double lon, lat;	/* Fixed point of reference */
		char unit;
	} G;
	struct I {	/* -I */
		bool active;
	} I;
	struct L {	/* -L<line.xy>[/<d|e|f|k|m|M|n|s|c|C>] */
		bool active;
		unsigned int mode;	/* 0 = dist to nearest point, 1 = also get the point, 2 = instead get seg#, pt# */
		unsigned int sph;	/* 0 = Flat Earth, 1 = spherical [Default], 2 = ellipsoidal */
		char *file;	/* Name of file with lines */
		char unit;
	} L;
	struct N {	/* -N */
		bool active;
		unsigned int mode;
	} N;
	struct Q {	/* -Q[e|d] */
		bool active;
		unsigned int mode;	/* 1 = print =Qe, 2 print -Qd, 3 print both */
	} Q;
	struct S {	/* -S */
		bool active;
	} S;
	struct T {	/* -T[h]<from>[/<to>] */
		bool active;
		bool heights;	/* True if we have heights */
		struct GMT_DATUM from;	/* Contains a, f, xyz[3] */
		struct GMT_DATUM to;	/* Contains a, f, xyz[3] */
	} T;
	struct W {	/* -W[w|h] */
		bool active;
		unsigned int mode;	/* 0 = print width & height, 1 print width, 2 print height */
	} W;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MAPPROJECT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct MAPPROJECT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->G.unit = GMT_MAP_DIST_UNIT;	/* Default unit is meter */
	C->G.sph = GMT_GREATCIRCLE;	/* Default is great-circle distances */
	C->L.mode = GMT_MP_GIVE_CORD;	/* Default returns coordinates of nearest point */
	C->L.unit = GMT_MAP_DIST_UNIT;	/* Default unit is meter */
	C->L.sph = GMT_GREATCIRCLE;	/* Default is great-circle distances */
	C->N.mode = GMT_LATSWAP_G2O;	/* Default is geodetic<->geocentric, if -N is used */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct MAPPROJECT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->L.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: mapproject <table> %s %s [-C[<dx></dy>]]\n", GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Ab|B|f|F|o|O[<lon0>/<lat0>]] [-D%s] [-E[<datum>]] [-F[<unit>]]\n\t[-G[-|+][<lon0>/<lat0>/][<unit>][+|-]", GMT_DIM_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, " [-I] [-L<table>[+u[+|-]<unit>][+p] [-N[a|c|g|m]]\n\t[-Q[e|d]] [-S] [-T[h]<from>[/<to>] [%s] [-W[w|h]] [%s] [%s]\n", GMT_V_OPT, GMT_b_OPT, GMT_d_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_p_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "J,R");
	GMT_Message (API, GMT_TIME_NONE, "\t   If UTM and -C are used then -R is optional (automatically set to match UTM zone)\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Calculate azimuths from previous point in the input data with -Af. If <lon0>/<lat0>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   is provided, then all azimuths are computed with respect to that point.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Ab to calculate back-azimuths from data to previous or the specified point.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Upper case B or F gives azimuths of geodesics using current ellipsoid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use o or O to get orientations (-90/90) instead of azimuths (0/360).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Return x/y relative to projection center [Default is relative to lower left corner].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally append <dx></dy> to add (or subtract if -I) (i.e., false easting & northing) [0/0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Units are plot units unless -F is set in which case the unit is meters.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Temporarily reset PROJ_LENGTH_UNIT to be c (cm), i (inch), or p (point).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be used if -F is set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Convert (lon, lat, h) to Earth Centered Earth Fixed (ECEF) coordinates [-I for inverse].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify <datum> using datum ID (see -Qd or man page) or as <ellipsoid>:<dx,dy,dz>,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   where <ellipsoid> may be ellipsoid ID (see -Qe or man page) or <semimajor>[,<inv_flattening>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <datum> = - or not given we assume WGS-84.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Force projected values to be in actual distances [Default uses the given plot scale].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify unit by appending e (meter), f (foot) k (km), M (mile), n (nautical mile), u (survey foot),\n\t   i (inch), c (cm), or p (points) [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Calculate distances to <lon0>/<lat0> OR cumulative distances along track (if point not given).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend - to the unit for (fast) flat Earth or + for (slow) geodesic calculations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is spherical great-circle calculations].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -G[<unit>]+ to obtain <lon0> <lat0> from two extra input columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -G[<unit>]- to get distance increments rather than cumulate distances along track.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give unit as arc (d)egree, m(e)ter, (f)oot, (k)m, arc (m)inute, (M)ile, (n)autical mile, s(u)rvey foot,\n\t   arc (s)econd, or (c)artesian [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Unit C means Cartesian distances after first projecting the input coordinates (-R, -J).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Inverse mode, i.e., get lon/lat from x/y input. [Default is lon/lat -> x/y].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Calculate minimum distances to specified line(s) in the file <table>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +u<unit> as arc (d)egree, m(e)ter, (f)oot, (k)m, arc (m)inute, (M)ile, (n)autical mile, s(u)rvey foot, arc (s)econd, or (c)artesian [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Unit C means Cartesian distances after first projecting the input coordinates (-R, -J).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend - to the unit for (fast) flat Earth or + for (slow) geodesic calculations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is spherical great-circle calculations].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Three columns are added on output: min dist and lon, lat of the closest point on the line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p to get line segment id and fractional point number instead of lon/lat.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Convert from geodetic to auxiliary latitudes; use -I for inverse conversion.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append a(uthalic), c(onformal), g(eocentric), or m(eridional) to select a conversion [geocentric].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q List projection parameters and stop.  For subsets [Default is all] use\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qe shows ellipsoid parameters.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qd shows datum parameters.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Suppress points outside region.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Perform coordinate transformation from datum <from> to datum <to>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend h if input data are lon, lat, height [Default sets height = 0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify datums using datum ID (see -Qd or man page) or as <ellipsoid>:<dx>,<dy>,<dz>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   where <ellipsoid> may be ellipsoid ID (see -Qe or man page) or <semimajor>[,<inv_flattening>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <from> = - means WGS-84.  If /<to> is not given we assume WGS-84.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The -T option can be used as pre- or post- (-I) processing for -J -R.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t -W prints map width and height. No input files are read. See -D for units.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append w or h to just print width or height, respectively.\n");
	GMT_Option (API, "V,bi2,bo,d,f,g,h,i,o,p,s,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int old_L_parse (struct GMTAPI_CTRL *API, char *arg, struct MAPPROJECT_CTRL *Ctrl) {
	/* [-L<table>[/[+|-]<unit>]][+] */
	int k, slash;
	gmt_M_unused(API);
	if (!gmt_M_compat_check (API->GMT, 5)) {	/* Sorry */
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L option: Expects -L<table>[+u[+|-]<unit>][+p]\n");
		return 1;
	}
	Ctrl->L.file = strdup (arg);
	k = (int)strlen (Ctrl->L.file) - 1;	/* Index of last character */
	if (Ctrl->L.file[k] == '+') {			/* Flag to get point number instead of coordinates at nearest point on line */
		Ctrl->L.mode = GMT_MP_GIVE_FRAC;
		Ctrl->L.file[k] = '\0';	/* Chop off the trailing plus sign */
		k--;	/* Now points to unit */
	}
	for (slash = k; slash && Ctrl->L.file[slash] != '/'; slash--);	/* Find location of optional slash */
	if (slash && ((k - slash) < 3)) {	/* User appended /[+|-]<unit>[+].  (k-slash) should be either 1 or 2 unless we are confused by files with subdirs */
		Ctrl->L.unit = Ctrl->L.file[k];
		k--;	/* Now points to either / or the optional -/+ mode setting */
		Ctrl->L.sph = GMT_GREATCIRCLE;	/* Great circle distances */
		if (k > 0 && (Ctrl->L.file[k] == '-' || Ctrl->L.file[k] == '+'))
			Ctrl->L.sph = (Ctrl->L.file[k] == '-') ? GMT_FLATEARTH : GMT_GEODESIC;	/* Gave [-|+]unit */
		Ctrl->L.file[slash] = '\0';
	}
	return 0;
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct MAPPROJECT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to mapproject and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_slash, k, n_errors = 0;
	int n;
	size_t last;
	bool geodetic_calc = false,  g_dist = false;
	char c, d, sign, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, from[GMT_LEN256] = {""}, to[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				n = sscanf (opt->arg, "%c%[^/]/%s", &c, txt_a, txt_b);
				if (n < 1) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -Ab|B|f|F[<lon0>/<lat0>]\n");
					n_errors++;
				}
				else {
					switch (c) {
						case 'B':
							Ctrl->A.geodesic = true;
						case 'b':
							Ctrl->A.reverse = true;
							break;
						case 'F':
							Ctrl->A.geodesic = true;
						case 'f':
							break;
						case 'O':
							Ctrl->A.geodesic = true;
						case 'o':
							Ctrl->A.orient = true;
							break;
						default:
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -Ab|B|f|F|o|O[<lon0>/<lat0>]\n");
							n_errors++;
							break;
					}
					if (n == 3) {
						n_errors += gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X],
						                                     gmt_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X],
						                                     &Ctrl->A.lon), txt_a);
						n_errors += gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y],
						                                     gmt_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y],
						                                     &Ctrl->A.lat), txt_b);
					}
					else
						Ctrl->A.azims = true;
				}
				break;
			case 'C':
				Ctrl->C.active = true;
				if (opt->arg[0]) {	/* Also gave shifts */
					n_errors += gmt_M_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->C.easting, &Ctrl->C.northing) != 2,
					                                 "Syntax error: Expected -C[<false_easting>/<false_northing>]\n");
					Ctrl->C.shift = true;
				}
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.unit = opt->arg[0];
				break;
			case 'E':
				Ctrl->E.active = true;
				if (gmt_set_datum (GMT, opt->arg, &Ctrl->E.datum) == -1) n_errors++;
				break;
			case 'F':
				Ctrl->F.active = true;
				Ctrl->F.unit = opt->arg[0];
				break;
			case 'G':	/* Syntax -G[<lon0/lat0>][/[+|-]units] */
				Ctrl->G.active = true;
				for (n_slash = k = 0; opt->arg[k]; k++) if (opt->arg[k] == '/') n_slash++;
				last = strlen (opt->arg) - 1;
				if (n_slash == 2 || n_slash == 1) {	/* Got -G<lon0/lat0>[/[+|-]unit] */
					Ctrl->G.mode = GMT_MP_FIXED_DIST;
					n = sscanf (opt->arg, "%[^/]/%[^/]/%c%c", txt_a, txt_b, &sign, &d);
					if (n_slash == 2) {	/* Got -G<lon0/lat0>/[+|-]unit */
						Ctrl->G.sph  = (sign == '-') ? GMT_FLATEARTH : ((sign == '+') ? GMT_GEODESIC : GMT_GREATCIRCLE);	/* If sign is not +|- then it was not given */
						Ctrl->G.unit = (sign == '-' || sign == '+') ? d : sign;	/* If no sign the unit is the 3rd item read by sscanf */
						n_errors += gmt_M_check_condition (GMT, !strchr (GMT_LEN_UNITS "cC", (int)Ctrl->G.unit),
						                                 "Syntax error: Expected -G<lon0>/<lat0>[/[-|+]%s|c|C]\n", GMT_LEN_UNITS_DISPLAY);
					}
					if (Ctrl->G.unit == 'c') gmt_set_cartesian (GMT, GMT_IN);	/* Cartesian */
					n_errors += gmt_M_check_condition (GMT, n < 2, "Syntax error: Expected -G<lon0>/<lat0>[/[-|+]%s|c|C]\n",
					                                   GMT_LEN_UNITS_DISPLAY);
					n_errors += gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X],
					                                     gmt_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X],
					                                     &Ctrl->G.lon), txt_a);
					n_errors += gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y],
					                                     gmt_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y],
					                                     &Ctrl->G.lat), txt_b);
				}
				else if (opt->arg[last] == '+') {				/* Got -G[[+|-]units]+ */
					Ctrl->G.mode = GMT_MP_PAIR_DIST;
					Ctrl->G.sph = (opt->arg[0] == '-') ? GMT_FLATEARTH : ((opt->arg[0] == '+') ? GMT_GEODESIC : GMT_GREATCIRCLE);
					Ctrl->G.unit = (opt->arg[0] == '-' || opt->arg[0] == '+') ? opt->arg[1] : opt->arg[0];
				}
				else if (opt->arg[last] == '-') {				/* Got -G[[+|-]units]- */
					Ctrl->G.mode = GMT_MP_INCR_DIST;
					Ctrl->G.sph = (opt->arg[0] == '-') ? GMT_FLATEARTH : ((opt->arg[0] == '+') ? GMT_GEODESIC : GMT_GREATCIRCLE);
					Ctrl->G.unit = (opt->arg[0] == '-' || opt->arg[0] == '+') ? opt->arg[1] : opt->arg[0];
				}
				else {				/* Got -G[[+|-]units] only */
					Ctrl->G.mode = GMT_MP_CUMUL_DIST;
					Ctrl->G.sph = (opt->arg[0] == '-') ? GMT_FLATEARTH : ((opt->arg[0] == '+') ? GMT_GEODESIC : GMT_GREATCIRCLE);
					k = (Ctrl->G.sph == GMT_GREATCIRCLE) ? 0 : 1;	/* k is the position of unit (which depends on any -|+), if given */
					if (opt->arg[k]) Ctrl->G.unit = opt->arg[k];
				}
				if (Ctrl->G.unit == 'c') Ctrl->G.unit = 'X';	/* Internally, this is Cartesian data and distances */
				break;
			case 'I':
				Ctrl->I.active = true;
				break;
			case 'L':	/* -L<table>[+u[+|-]<unit>][+p] */
				Ctrl->L.active = true;
				if (!(strstr (opt->arg, "+u") || strstr (opt->arg, "+p") || strchr (opt->arg, '/')))
					n_errors += old_L_parse (API, opt->arg, Ctrl);
				else {
					Ctrl->L.file = gmt_get_filename (opt->arg);
					if (gmt_get_modifier (opt->arg, 'u', txt_a)) {
						Ctrl->L.sph = GMT_GREATCIRCLE;	/* Default is great circle distances */
						k = 0;
						switch (txt_a[0]) {
							case '-': Ctrl->L.sph = GMT_FLATEARTH;	k = 1; break;
							case '+': Ctrl->L.sph = GMT_GEODESIC;	k = 1; break;
						}
						Ctrl->L.unit = txt_a[k];
					}
					if (gmt_get_modifier (opt->arg, 'p', txt_a))
						Ctrl->L.mode = GMT_MP_GIVE_FRAC;
				}
				/* Check settings */
				n_errors += gmt_M_check_condition (GMT, !strchr (GMT_LEN_UNITS "cC", (int)Ctrl->L.unit),
				            "Syntax error: Expected -L<file>[+u[-|+]%s|c|C][+p]\n", GMT_LEN_UNITS_DISPLAY);
				if (strchr (GMT_LEN_UNITS, (int)Ctrl->L.unit) && !gmt_M_is_geographic (GMT, GMT_IN))
					gmt_parse_common_options (GMT, "f", 'f', "g");	/* Implicitly set -fg since user wants spherical distances */
				if (Ctrl->L.unit == 'c') Ctrl->L.unit = 'X';		/* Internally, this is Cartesian data and distances */
				if (!gmt_check_filearg (GMT, 'L', Ctrl->L.file, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;
			case 'N':
				Ctrl->N.active = true;
				switch (opt->arg[0]) {
					case 'a': Ctrl->N.mode = GMT_LATSWAP_G2A; break;
					case 'c': Ctrl->N.mode = GMT_LATSWAP_G2C; break;
					case 'g': Ctrl->N.mode = GMT_LATSWAP_G2O; break;
					case 'm': Ctrl->N.mode = GMT_LATSWAP_G2M; break;
					case '\0': Ctrl->N.mode = GMT_LATSWAP_G2O; break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -N[a|c|g|m]\n");
						n_errors++;
				}
				break;
			case 'Q':
				Ctrl->Q.active = true;
				if (opt->arg[0] == 'e')  Ctrl->Q.mode |= 1;
				if (opt->arg[0] == 'd')  Ctrl->Q.mode |= 2;
				if (opt->arg[0] == '\0') Ctrl->Q.mode = 3;
				break;
			case 'S':
				Ctrl->S.active = true;
				break;
			case 'T':
				Ctrl->T.active = true;
				k = 0;
				if (opt->arg[k] == 'h') {	/* We will process lon, lat, height data */
					k = 1;
					Ctrl->T.heights = true;	/* If false we set height = 0 */
				}

				if (strchr (&opt->arg[k], '/')) {	/* Gave from/to */
					sscanf (&opt->arg[k], "%[^/]/%s", from, to);
				}
				else {	/* to not given, set to - which means WGS-84 */
					strcpy (to, "-");
					strncpy (from, &opt->arg[k], GMT_LEN256);
				}
				n_errors += gmt_M_check_condition (GMT, gmt_set_datum (GMT, to, &Ctrl->T.to) == -1 ||
				                                 gmt_set_datum (GMT, from, &Ctrl->T.from) == -1,
				                                 "Syntax error -T: Usage -T[h]<from>[/<to>]\n");
				break;
			case 'W':
				Ctrl->W.active = true;
				if (opt->arg[0] == 'w') Ctrl->W.mode = 1;
				else if (opt->arg[0] == 'h') Ctrl->W.mode = 2;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	geodetic_calc = (Ctrl->G.mode || Ctrl->A.active || Ctrl->L.active);

	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && (Ctrl->G.mode + Ctrl->E.active + Ctrl->L.active) > 0,
	                                 "Syntax error: -T cannot work with -E, -G or -L\n");
	/* Can only do one of -A, -G and -L */
	g_dist = (Ctrl->G.mode > 0);
	n_errors += gmt_M_check_condition (GMT, (g_dist + Ctrl->A.active + Ctrl->L.active) > 1,
	                                 "Syntax error: Can only specify one of -A, -G and -L\n");
	n_errors += gmt_M_check_condition (GMT, geodetic_calc && Ctrl->I.active, "Syntax error: -A, -G, and -L cannot work with -I\n");
	/* Can only do -p for forward projection */
	n_errors += gmt_M_check_condition (GMT, GMT->common.p.active && Ctrl->I.active, "Syntax error: -p cannot work with -I\n");
	/* Must have -J */
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active && (Ctrl->G.mode || Ctrl->L.active) && Ctrl->G.unit == 'C',
	                                 "Syntax error: Must specify -J option with selected form of -G or -L when unit is C\n");
	if (!GMT->common.R.active && GMT->current.proj.projection == GMT_UTM && Ctrl->C.active) {	/* Set default UTM region from zone info */
		if (GMT->current.proj.utm_hemisphere == 0)		/* Default to N hemisphere if nothing is known */
			GMT->current.proj.utm_hemisphere = 1;
		if (gmt_UTMzone_to_wesn (GMT, GMT->current.proj.utm_zonex, GMT->current.proj.utm_zoney,
		                         GMT->current.proj.utm_hemisphere, GMT->common.R.wesn)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Bad UTM zone\n");
			n_errors++;
		}
		else
			GMT_Report (API, GMT_MSG_VERBOSE, "UTM zone used to generate region %g/%g/%g/%g\n",
				GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);

		GMT->common.R.active = true;
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && gmt_access (GMT, Ctrl->L.file, R_OK),
	                                 "Syntax error -L: Cannot read file %s!\n", Ctrl->L.file);
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active && !(geodetic_calc || Ctrl->T.active || Ctrl->E.active ||
	                                 Ctrl->N.active || Ctrl->Q.active), "Syntax error: Must specify -R option\n");
	n_errors += gmt_check_binary_io (GMT, 2);
	n_errors += gmt_M_check_condition (GMT, (Ctrl->D.active + Ctrl->F.active) == 2, "Syntax error: Can specify only one of -D and -F\n");
	n_errors += gmt_M_check_condition (GMT, ((Ctrl->T.active && GMT->current.proj.datum.h_given) || Ctrl->E.active) &&
	                                 GMT->common.b.active[GMT_IN] && gmt_get_cols (GMT, GMT_IN) < 3,
	                                 "Syntax error: For -E or -T, binary input data (-bi) must have at least 3 columns\n");

	if (!(n_errors || GMT->common.R.active)) {
		GMT->common.R.wesn[XLO] = 0.0;	GMT->common.R.wesn[XHI] = 360.0;
		GMT->common.R.wesn[YLO] = -90.0;	GMT->common.R.wesn[YHI] = 90.0;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mapproject (void *V_API, int mode, void *args) {
	int x, y, ks, rmode, n_fields, two, way, error = 0, o_type[2] = {GMT_Z, GMT_Z};
	int fmt[2], save[2] = {0,0}, unit = 0, proj_type = 0, lat_mode = 0;
	bool line_start = true, do_geo_conv = false;
	bool geodetic_calc = false, datum_conv_only = false, double_whammy = false;

	enum GMT_enum_family family;
	enum GMT_enum_geometry geometry;

	unsigned int i = 0, pos;

	uint64_t row, n_read_in_seg, seg, n_read = 0, n = 0, k, n_output = 0;

	double x_in = 0.0, y_in = 0.0, d = 0.0, fwd_scale, inv_scale, xtmp, ytmp, *out = NULL;
	double xmin, xmax, ymin, ymax, inch_to_unit, unit_to_inch, u_scale, y_out_min;
	double x_in_min, x_in_max, y_in_min, y_in_max, x_out_min, x_out_max, y_out_max;
	double xnear = 0.0, ynear = 0.0, lon_prev = 0, lat_prev = 0, **data = NULL, *in = NULL;

	char format[GMT_BUFSIZ] = {""}, unit_name[GMT_LEN64] = {""}, scale_unit_name[GMT_LEN64] = {""};
	char line[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""}, record[GMT_BUFSIZ] = {""};

	bool (*map_fwd) (struct GMT_CTRL *, double, double, double *, double *);	/* Pointers to the selected forward mapping function */
	void (*map_inv) (struct GMT_CTRL *, double *, double *, double, double);	/* Pointers to the selected inverse mapping function */

	struct GMT_DATATABLE *xyline = NULL;
	struct GMT_DATASET *Lin = NULL;
	struct MAPPROJECT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = gmt_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);
	if (!GMT->common.J.active && !gmt_M_is_geographic (GMT, GMT_IN)) { /* -J not given and input not specified as lon/lat; check if we should add -fg */
		if ((Ctrl->G.active && Ctrl->G.unit != 'X') || (Ctrl->L.active && Ctrl->L.unit != 'X'))
			gmt_parse_common_options (GMT, "f", 'f', "g"); /* Not Cartesian unit so set -fg */
	}

	/*---------------------------- This is the mapproject main code ----------------------------*/

	if (Ctrl->Q.mode & 1) {	/* List ellipsoid parameters */
		GMT_Message (API, GMT_TIME_NONE, "GMT supports %d ellipsoids, given below (-> indicates default setting)\n", GMT_N_ELLIPSOIDS);
		GMT_Message (API, GMT_TIME_NONE, "  ID                      Date        a           1/f\n");
		GMT_Message (API, GMT_TIME_NONE, "-----------------------------------------------------------\n");
		for (i = 0; i < GMT_N_ELLIPSOIDS; i++) {
			(i == GMT->current.setting.proj_ellipsoid) ? GMT_Message (API, GMT_TIME_NONE, "->") : GMT_Message (API, GMT_TIME_NONE,  "  ");
			GMT_Message (API, GMT_TIME_NONE, "%-23s %4ld %13.3f %14.9f\n",
			             GMT->current.setting.ref_ellipsoid[i].name, GMT->current.setting.ref_ellipsoid[i].date,
			             GMT->current.setting.ref_ellipsoid[i].eq_radius, 1.0/GMT->current.setting.ref_ellipsoid[i].flattening);
		}
		GMT_Message (API, GMT_TIME_NONE, "-----------------------------------------------------------\n");
	}
	if (Ctrl->Q.mode & 2) {	/* List datum parameters */
		GMT_Message (API, GMT_TIME_NONE, "GMT supports %d datums, given below (-> indicates default setting)\n", GMT_N_DATUMS);
		GMT_Message (API, GMT_TIME_NONE, "  ID  Name                               Ellipsoid                 x     y     z   Region\n");
		GMT_Message (API, GMT_TIME_NONE, "-----------------------------------------------------------------------------------------\n");
		for (i = 0; i < GMT_N_DATUMS; i++) {
			(!strcmp (GMT->current.setting.proj_datum[i].name, "WGS 1984")) ? GMT_Message (API, GMT_TIME_NONE, "->") :
			          GMT_Message (API, GMT_TIME_NONE,  "  ");
			GMT_Message (API, GMT_TIME_NONE, "%3ld %-34s %-23s %5.0f %5.0f %5.0f %s\n",
			             i, GMT->current.setting.proj_datum[i].name, GMT->current.setting.proj_datum[i].ellipsoid,
			             GMT->current.setting.proj_datum[i].xyz[0], GMT->current.setting.proj_datum[i].xyz[1],
			             GMT->current.setting.proj_datum[i].xyz[2], GMT->current.setting.proj_datum[i].region);
		}
		GMT_Message (API, GMT_TIME_NONE, "-----------------------------------------------------------------------------------------\n");
	}
	if (Ctrl->Q.mode) Return (GMT_NOERROR);

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	if (Ctrl->D.active) gmt_M_err_fail (GMT, gmt_set_measure_unit (GMT, Ctrl->D.unit), "-D");
	if (Ctrl->T.active) gmt_datum_init (GMT, &Ctrl->T.from, &Ctrl->T.to, Ctrl->T.heights);
	if (Ctrl->A.active) {
		way = gmt_M_is_geographic (GMT, GMT_IN) ? 2 + Ctrl->A.geodesic : 0;
		proj_type = gmt_init_distaz (GMT, (way) ? 'k' : 'X', way, GMT_MAP_DIST);
	}
	if (Ctrl->G.active) {
		way = gmt_M_is_geographic (GMT, GMT_IN) ? Ctrl->G.sph : 0;
		proj_type = gmt_init_distaz (GMT, Ctrl->G.unit, way, GMT_MAP_DIST);
	}
	if (Ctrl->L.active) {
		way = gmt_M_is_geographic (GMT, GMT_IN) ? Ctrl->L.sph: 0;
		proj_type = gmt_init_distaz (GMT, Ctrl->L.unit, way, GMT_MAP_DIST);
	}
	if (Ctrl->E.active) gmt_ECEF_init (GMT, &Ctrl->E.datum);
	if (Ctrl->F.active) unit = gmt_check_scalingopt (GMT, 'F', Ctrl->F.unit, scale_unit_name);

	geodetic_calc = (Ctrl->G.mode || Ctrl->A.active || Ctrl->L.active);

	if (Ctrl->T.active && GMT->current.proj.projection != GMT_LINEAR && GMT->common.R.active) {	/* Do datum shift & project coordinates */
		double_whammy = true;
		if (Ctrl->I.active) {	/* Need to set the ellipsoid to that of the old datum */
			if (GMT->current.proj.datum.from.ellipsoid_id < 0) {
				GMT->current.setting.proj_ellipsoid = GMT_N_ELLIPSOIDS - 1;
				GMT->current.setting.ref_ellipsoid[i].eq_radius = GMT->current.proj.datum.from.a;
				GMT->current.setting.ref_ellipsoid[i].flattening = GMT->current.proj.datum.from.f;
			}
			else
				GMT->current.setting.proj_ellipsoid = GMT->current.proj.datum.from.ellipsoid_id;
		}
		else {	/* Need to set the ellipsoid to that of the new datum */
			if (GMT->current.proj.datum.to.ellipsoid_id < 0) {
				GMT->current.setting.proj_ellipsoid = GMT_N_ELLIPSOIDS - 1;
				GMT->current.setting.ref_ellipsoid[i].eq_radius = GMT->current.proj.datum.to.a;
				GMT->current.setting.ref_ellipsoid[i].flattening = GMT->current.proj.datum.to.f;
			}
			else
				GMT->current.setting.proj_ellipsoid = GMT->current.proj.datum.to.ellipsoid_id;
		}
	}
	else
		datum_conv_only = Ctrl->T.active;

	gmt_init_scales (GMT, unit, &fwd_scale, &inv_scale, &inch_to_unit, &unit_to_inch, unit_name);

	if (Ctrl->G.mode) {	/* save output format in case -J changes it */
		save[GMT_X] = (Ctrl->G.unit == 'X') ? GMT_IS_FLOAT : GMT->current.io.col_type[GMT_OUT][GMT_X];
		save[GMT_Y] = (Ctrl->G.unit == 'X') ? GMT_IS_FLOAT : GMT->current.io.col_type[GMT_OUT][GMT_Y];
	}
	u_scale = (Ctrl->I.active) ? inv_scale : fwd_scale;

	if (!GMT->common.J.active) {	/* Supply dummy linear proj */
		if (Ctrl->G.active && Ctrl->G.unit == 'X') {
			gmt_set_cartesian (GMT, GMT_IN);
			gmt_parse_common_options (GMT, "J", 'J', "x1");	/* Fake linear Cartesian projection */
		}
		else
			gmt_parse_common_options (GMT, "J", 'J', "x1d");	/* Fake linear degree projection */
		if (!GMT->common.R.active) {
			GMT->common.R.wesn[XLO] = 0.0;	GMT->common.R.wesn[XHI] = 360.0;
			GMT->common.R.wesn[YLO] = -90.0;	GMT->common.R.wesn[YHI] = 90.0;
		}
	}
	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);
	
	if (Ctrl->W.active) {	/* Print map dimensions and exit */
		double w_out[2] = {0.0, 0.0};
		switch (Ctrl->W.mode) {
			case 1:
				GMT_Report (API, GMT_MSG_VERBOSE, "Reporting map width in %s\n", unit_name);
				w_out[GMT_X] = GMT->current.proj.rect[XHI] * inch_to_unit;	n_output = 1;	break;
			case 2:
				GMT_Report (API, GMT_MSG_VERBOSE, "Reporting map height in %s\n", unit_name);
				w_out[GMT_X] = GMT->current.proj.rect[YHI] * inch_to_unit;	n_output = 1;	break;
			default:
				GMT_Report (API, GMT_MSG_VERBOSE, "Reporting map width and height in %s\n", unit_name);
				w_out[GMT_X] = GMT->current.proj.rect[XHI] * inch_to_unit;
				w_out[GMT_Y] = GMT->current.proj.rect[YHI] * inch_to_unit;
				n_output = 2;
			break;
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
			Return (API->error);
		}
		if ((error = gmt_set_cols (GMT, GMT_OUT, n_output)) != 0) Return (error);
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_NONE) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
		GMT_Put_Record (API, GMT_WRITE_DATA, w_out);	/* Write this to output */
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}

	if (Ctrl->G.unit == 'X') gmt_set_cartesian (GMT, GMT_IN);	/* Cartesian */
	if (Ctrl->L.unit == 'X') gmt_set_cartesian (GMT, GMT_IN);	/* Cartesian */

	if (Ctrl->G.mode && proj_type != GMT_GEO2CART) {	/* Ensure we use the selected output coordinates */
		GMT->current.io.col_type[GMT_OUT][GMT_X] = save[GMT_X];
		GMT->current.io.col_type[GMT_OUT][GMT_Y] = save[GMT_Y];
	}
	if (datum_conv_only) {	/* Both input and output is geographic */
		GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
		GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
	}

	if (Ctrl->C.active) {
		if (Ctrl->F.active) {
			map_fwd = &gmt_geo_to_xy_noshiftscale;
			map_inv = &gmt_xy_to_geo_noshiftscale;
		}
		else {
			map_fwd = &gmt_geo_to_xy_noshift;
			map_inv = &gmt_xy_to_geo_noshift;
		}
	}
	else {
		map_fwd = &gmt_geo_to_xy;
		map_inv = &gmt_xy_to_geo;
	}
	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE) && !(geodetic_calc || Ctrl->T.active)) {
		sprintf (format, "%s/%s/%s/%s", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		                                GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		xmin = (Ctrl->C.active) ? GMT->current.proj.rect[XLO] - GMT->current.proj.origin[GMT_X] : GMT->current.proj.rect[XLO];
		xmax = (Ctrl->C.active) ? GMT->current.proj.rect[XHI] - GMT->current.proj.origin[GMT_X] : GMT->current.proj.rect[XHI];
		ymin = (Ctrl->C.active) ? GMT->current.proj.rect[YLO] - GMT->current.proj.origin[GMT_Y] : GMT->current.proj.rect[YLO];
		ymax = (Ctrl->C.active) ? GMT->current.proj.rect[YHI] - GMT->current.proj.origin[GMT_Y] : GMT->current.proj.rect[YHI];
		if (Ctrl->F.active) {	/* Convert to meter, then to chosen unit */
			strncpy (unit_name, scale_unit_name, GMT_LEN64);
			xmin /= GMT->current.proj.scale[GMT_X];
			xmax /= GMT->current.proj.scale[GMT_X];
			ymin /= GMT->current.proj.scale[GMT_Y];
			ymax /= GMT->current.proj.scale[GMT_Y];
			if (unit) {	/* Change the 1:1 unit used */
				xmin *= fwd_scale;
				xmax *= fwd_scale;
				ymin *= fwd_scale;
				ymax *= fwd_scale;
			}
		}
		else {	/* Convert inches to chosen MEASURE */
			xmin *= inch_to_unit;
			xmax *= inch_to_unit;
			ymin *= inch_to_unit;
			ymax *= inch_to_unit;
		}
		if (Ctrl->C.shift) {
			xmin += Ctrl->C.easting;
			xmax += Ctrl->C.easting;
			ymin += Ctrl->C.northing;
			ymax += Ctrl->C.northing;
		}

		GMT_Report (API, GMT_MSG_NORMAL, " Transform ");
		if (Ctrl->N.active) {
			char *auxlat[4] = {"authalic", "conformal", "meridional", "geocentric"};
			GMT_Message (API, GMT_TIME_NONE, "geodetic");
			(Ctrl->I.active) ? GMT_Message (API, GMT_TIME_NONE, " <- ") : GMT_Message (API, GMT_TIME_NONE,  " -> ");
			GMT_Message (API, GMT_TIME_NONE, "%s coordinates [degrees]\n", auxlat[Ctrl->N.mode/2]);
		}
		else {
			GMT_Message (API, GMT_TIME_NONE, format, GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI],
			             GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);
			(Ctrl->I.active) ? GMT_Message (API, GMT_TIME_NONE, " <- ") : GMT_Message (API, GMT_TIME_NONE,  " -> ");
			GMT_Message (API, GMT_TIME_NONE, format, xmin, xmax, ymin, ymax);
			GMT_Message (API, GMT_TIME_NONE, " [%s]\n", unit_name);
		}
	}

	if (Ctrl->L.active) {
		/* Initialize the i/o for doing table reading */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {
			Return (API->error);
		}

		gmt_disable_i_opt (GMT);	/* Do not want any -i to affect the reading from -L files */
		if ((Lin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, Ctrl->L.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (Lin->n_columns < 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)Lin->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_i_opt (GMT);	/* Recover settings provided by user (if -i was used at all) */
		gmt_set_segmentheader (GMT, GMT_OUT, false);	/* Since processing of -L file might have turned it on [should be determined below] */
		xyline = Lin->table[0];			/* Can only be one table since we read a single file */
		if (proj_type == GMT_GEO2CART) {	/* Must convert the line points first */
			for (seg = 0; seg < xyline->n_segments; seg++) {
				for (row = 0; row < xyline->segment[seg]->n_rows; row++) {
					map_fwd (GMT, xyline->segment[seg]->data[GMT_X][row], xyline->segment[seg]->data[GMT_Y][row], &xtmp, &ytmp);
					xyline->segment[seg]->data[GMT_X][row] = xtmp;
					xyline->segment[seg]->data[GMT_Y][row] = ytmp;
				}
			}
		}
		else if (gmt_M_is_geographic (GMT, GMT_IN) && proj_type == GMT_GEOGRAPHIC && !gmt_M_is_spherical (GMT)) {
			do_geo_conv = true;
			/* Will need spherical trig so convert to geocentric latitudes if on an ellipsoid */
			for (seg = 0; seg < xyline->n_segments; seg++) {
				for (row = 0; row < xyline->segment[seg]->n_rows; row++) {		/* Convert to geocentric */
					xyline->segment[seg]->data[GMT_Y][row] = gmt_lat_swap (GMT, xyline->segment[seg]->data[GMT_Y][row], GMT_LATSWAP_G2O);
				}
			}
		}
	}

	/* Now we are ready to take on some input values */

	x = (GMT->current.setting.io_lonlat_toggle[GMT_OUT]) ? 1 : 0;	y = 1 - x;		/* Set up which columns have x and y for output only*/
	if ((gmt_M_is_geographic (GMT, GMT_IN) || Ctrl->E.active) && Ctrl->I.active) {
		gmt_set_geographic (GMT, GMT_OUT);	/* Inverse projection expects x,y and gives lon, lat */
		gmt_set_cartesian (GMT, GMT_IN);
	}
	else if (datum_conv_only || Ctrl->N.active) {	/* Both in and out are geographic */
		gmt_set_geographic (GMT, GMT_IN);
		gmt_set_geographic (GMT, GMT_OUT);
		o_type[GMT_X] = GMT_X;	o_type[GMT_Y] = GMT_Y;
		GMT->current.io.col_type[GMT_IN][GMT_Z] = GMT->current.io.col_type[GMT_OUT][GMT_Z] = GMT_IS_FLOAT;
	}
	else if (gmt_M_is_geographic (GMT, GMT_OUT)) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Override -fog for normal operation\n");
		gmt_set_cartesian (GMT, GMT_OUT);
	}

	/* Specify input and output expected columns */
	if ((error = gmt_set_cols (GMT, GMT_IN,  0)) != 0) Return (error);

	/* Initialize the i/o for doing record-by-record reading/writing */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}

	x_in_min = y_in_min = x_out_min = y_out_min = DBL_MAX;
	x_in_max = y_in_max = x_out_max = y_out_max = -DBL_MAX;

	two = (Ctrl->E.active || (Ctrl->T.active && GMT->current.proj.datum.h_given)) ? 3 : 2;	/* # of output points from conversion */

	if (Ctrl->C.shift && Ctrl->F.active) {	/* Use same units in -C and -F */
		Ctrl->C.easting *= u_scale;
		Ctrl->C.northing *= u_scale;
	}

	if (Ctrl->L.mode == GMT_MP_GIVE_FRAC)	/* Want fractional point locations */
		fmt[0] = fmt[1] = GMT_Z;
	else {			/* Want nearest point */
		fmt[0] = GMT_X;
		fmt[1] = GMT_Y;
	}
	if (Ctrl->N.active) lat_mode = Ctrl->N.mode + Ctrl->I.active;

	if (GMT_Begin_IO (API, GMT_IS_DATASET,  GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}
	rmode = (gmt_is_ascii_record (GMT, options) && gmt_get_cols (GMT, GMT_IN) > 2) ? GMT_READ_MIXED : GMT_READ_DATA;
	family = (rmode == GMT_READ_DATA) ? GMT_IS_DATASET : GMT_IS_TEXTSET;
	geometry = (rmode == GMT_READ_DATA) ? GMT_IS_POINT : GMT_IS_NONE;
	if (GMT_Init_IO (API, family, geometry, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, family, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, geometry) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}
	gmt_set_cols (GMT, GMT_OUT, gmt_get_cols (GMT, GMT_IN));

	n = n_read_in_seg = 0;
	out = gmt_M_memory (GMT, NULL, GMT_MAX_COLUMNS, double);
	data = (proj_type == GMT_GEO2CART) ? &out : &in;	/* Using projected or original coordinates */
	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (API, rmode, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_table_header (GMT)) {	/* Echo table headers */
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, NULL);
			}
			else if (gmt_M_rec_is_new_segment (GMT)) {			/* Echo segment headers */
				GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
				line_start = true;
				n_read_in_seg = 0;
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}
		if (gmt_M_rec_is_gap (GMT)) {	/* Gap detected.  Write a segment header but continue on since record is actually data */
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
			GMT->current.io.status = 0;	/* Done with gap */
			line_start = true;
			n_read_in_seg = 0;
		}

		/* Data record to process */

		n_read++;
		n_read_in_seg++;

		if (Ctrl->I.active) {		/* Do inverse transformation */

			if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
				x_in = in[GMT_X];
				y_in = in[GMT_Y];
			}
			if (Ctrl->C.shift) {
				in[GMT_X] -= Ctrl->C.easting;
				in[GMT_Y] -= Ctrl->C.northing;
			}
			if (Ctrl->N.active) {
				out[GMT_X] = in[GMT_X];
				out[GMT_Y] = gmt_lat_swap (GMT, in[GMT_Y], lat_mode);
			}
			else if (Ctrl->E.active) {
				gmt_ECEF_inverse (GMT, in, out);
			}
			else {
				if (Ctrl->F.active) {	/* Convert from 1:1 scale */
					if (unit) {
						in[GMT_X] *= u_scale;
						in[GMT_Y] *= u_scale;
					}
					if (!Ctrl->C.active) {
						in[GMT_X] *= GMT->current.proj.scale[GMT_X];
						in[GMT_Y] *= GMT->current.proj.scale[GMT_Y];
					}
				}
				else if (GMT->current.setting.proj_length_unit != GMT_INCH) {	/* Convert from whatever to inch */
					in[GMT_X] *= unit_to_inch;
					in[GMT_Y] *= unit_to_inch;
				}
#if 0
				if (Ctrl->C.active) {	/* Then correct so lower left corner is (0,0) */
					in[GMT_X] += GMT->current.proj.origin[GMT_X];
					in[GMT_Y] += GMT->current.proj.origin[GMT_Y];
				}
#endif
				map_inv (GMT, &out[GMT_X], &out[GMT_Y], in[GMT_X], in[GMT_Y]);
			}
			if (double_whammy) {	/* Now apply datum shift */
				in[GMT_X] = out[GMT_X];
				in[GMT_Y] = out[GMT_Y];
				gmt_conv_datum (GMT, in, out);
			}

			if (Ctrl->S.active && gmt_map_outside (GMT, out[GMT_X], out[GMT_Y])) continue;
			if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
				x_in_min = MIN (x_in_min, x_in);
				x_in_max = MAX (x_in_max, x_in);
				y_in_min = MIN (y_in_min, y_in);
				y_in_max = MAX (y_in_max, y_in);
				x_out_min = MIN (x_out_min, out[GMT_X]);
				x_out_max = MAX (x_out_max, out[GMT_X]);
				y_out_min = MIN (y_out_min, out[GMT_Y]);
				y_out_max = MAX (y_out_max, out[GMT_Y]);
			}

			if (rmode == GMT_READ_MIXED) {
				/* Special case: ASCII i/o and at least 3 input columns:
				   Columns beyond first two could be text strings */

				/* We will use gmt_strtok to step past the first 2 [or 3] columns.  The remainder
				 * will then be the user text that we want to preserve.  Since strtok places
				 * 0 to indicate start of next token we count our way to the start of the text. */

				strncpy (line, GMT->current.io.record, GMT_BUFSIZ);
				gmt_chop (line);	/* Chop of line feed */
				pos = record[0] = 0;	/* Start with blank record */
				gmt_strtok (line, GMT_TOKEN_SEPARATORS, &pos, p);	/* Returns xstring (ignored) and update pos */
				gmt_strtok (line, GMT_TOKEN_SEPARATORS, &pos, p);	/* Returns ystring (ignored) and update pos */
				gmt_add_to_record (GMT, record, out[x], GMT_X, GMT_OUT, 2);	/* Format our output x value */
				gmt_add_to_record (GMT, record, out[y], GMT_Y, GMT_OUT, 2);	/* Format our output y value */
				if (Ctrl->E.active) {
					gmt_strtok (line, GMT_TOKEN_SEPARATORS, &pos, p);		/* Returns zstring (ignore) and update pos */
					gmt_add_to_record (GMT, record, out[GMT_Z], GMT_Z, GMT_OUT, 2);	/* Format our output z value */
				}
				if (line[pos]) strcat (record, &line[pos]);	/* Append the remainder of the user text */
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write this to output */

			}
			else {	/* Simply copy other columns and output */
				if (n_output == 0) {
					gmt_set_cols (GMT, GMT_OUT, gmt_get_cols (GMT, GMT_IN));
					n_output = gmt_get_cols (GMT, GMT_OUT);
				}
				for (k = two; k < n_output; k++) out[k] = in[k];
				GMT_Put_Record (API, GMT_WRITE_DATA, out);	/* Write this to output */
			}
			n++;
			if (n%1000 == 0) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Projected %" PRIu64 " points\r", n);
		}
		else {		/* Do forward transformation */
			if (gmt_M_y_is_lat (GMT,GMT_IN) && (in[GMT_Y] < -90.0 || in[GMT_Y] > 90.0)) {
				GMT_Report (API, GMT_MSG_NORMAL, "Bad latitude outside valid range at input line %" PRIu64 " - skipped\n", n_read);
				continue;
			}

			if (Ctrl->S.active && gmt_map_outside (GMT, in[GMT_X], in[GMT_Y])) continue;

			if (Ctrl->N.active) {
				out[GMT_X] = in[GMT_X];
				out[GMT_Y] = gmt_lat_swap (GMT, in[GMT_Y], lat_mode);
			}
			else if (datum_conv_only)
				gmt_conv_datum (GMT, in, out);
			else if (Ctrl->E.active)
				gmt_ECEF_forward (GMT, in, out);
			else {
				if (double_whammy) {	/* Apply datum shift first */
					gmt_conv_datum (GMT, in, out);
					in[GMT_X] = out[GMT_X];
					in[GMT_Y] = out[GMT_Y];
				}
				map_fwd (GMT, in[GMT_X], in[GMT_Y], &out[GMT_X], &out[GMT_Y]);
#if 0
				if (Ctrl->C.active) {	/* Change origin from lower left to projection center */
					out[GMT_X] -= GMT->current.proj.origin[GMT_X];
					out[GMT_Y] -= GMT->current.proj.origin[GMT_Y];
				}
#endif
				if (Ctrl->F.active) {	/* Convert to 1:1 scale */
					if (!Ctrl->C.active) {	/* Change origin from lower left to projection center */
						out[GMT_X] /= GMT->current.proj.scale[GMT_X];
						out[GMT_Y] /= GMT->current.proj.scale[GMT_Y];
					}
					if (unit) {
						out[GMT_X] *= u_scale;
						out[GMT_Y] *= u_scale;
					}
				}
				else if (GMT->current.setting.proj_length_unit != GMT_INCH) {	/* Convert from inch to whatever */
					out[GMT_X] *= inch_to_unit;
					out[GMT_Y] *= inch_to_unit;
				}
				if (Ctrl->C.shift) {
					out[GMT_X] += Ctrl->C.easting;
					out[GMT_Y] += Ctrl->C.northing;
				}
				if (GMT->current.proj.three_D) {
					double xx = out[GMT_X], yy = out[GMT_Y];
					gmt_xyz_to_xy (GMT, xx, yy, gmt_z_to_zz (GMT, in[GMT_Z]), &out[GMT_X], &out[GMT_Y]);
				}
			}
			if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
				x_in_min = MIN (x_in_min, in[GMT_X]);
				x_in_max = MAX (x_in_max, in[GMT_X]);
				y_in_min = MIN (y_in_min, in[GMT_Y]);
				y_in_max = MAX (y_in_max, in[GMT_Y]);
			}

			if (geodetic_calc) {	/* Get either distances or azimuths */
				double s;
				if (Ctrl->G.mode) {	/* Distances of some sort */
					if (Ctrl->G.mode == GMT_MP_PAIR_DIST)	/* Segment distances from each data record using 2 points */
						s = gmt_distance (GMT, in[GMT_X], in[GMT_Y], in[2], in[3]);
					else if (Ctrl->G.mode >= GMT_MP_CUMUL_DIST && line_start)	/* Reset cumulative distances */
						s = d = 0.0;
					else						/* Incremental distance */
						s = gmt_distance (GMT, Ctrl->G.lon, Ctrl->G.lat, in[GMT_X], in[GMT_Y]);
					if (Ctrl->G.mode >= GMT_MP_CUMUL_DIST) {	/* Along-track calculation */
						line_start = false;
						d = (Ctrl->G.mode >= GMT_MP_INCR_DIST) ? s : d + s;	/* Increments or cumulative */
						Ctrl->G.lon = in[GMT_X];	/* Save last point in G */
						Ctrl->G.lat = in[GMT_Y];
					}
					else	/* Fixed-point distance */
						d = s;
				}
				else if (Ctrl->L.active) {	/* Compute closest distance to line */
					y_in = (do_geo_conv) ? gmt_lat_swap (GMT, (*data)[GMT_Y], GMT_LATSWAP_G2O) : (*data)[GMT_Y];	/* Convert to geocentric */
					(void) gmt_near_lines (GMT, (*data)[GMT_X], y_in, xyline, Ctrl->L.mode, &d, &xnear, &ynear);
					if (do_geo_conv && Ctrl->L.mode != GMT_MP_GIVE_FRAC)
						ynear = gmt_lat_swap (GMT, ynear, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
				}
				else if (Ctrl->A.azims) {	/* Azimuth from previous point */
					if (n_read_in_seg == 1)	/* First point has undefined azimuth since there is no previous point */
						d = GMT->session.d_NaN;
					else {
						d = gmt_az_backaz (GMT, lon_prev, lat_prev, in[GMT_X], in[GMT_Y], Ctrl->A.reverse);
						if (Ctrl->A.orient) {	/* Want orientations in -90/90 instead */
							d = fmod (2.0 * d, 360.0) * 0.5;	/* Get orientation */
							if (d > 90.0) d-= 180.0;
						}
					}
					lon_prev = in[GMT_X];	/* Update previous point */
					lat_prev = in[GMT_Y];
				}
				else {	/* Azimuths with respect to a fixed point */
					d = gmt_az_backaz (GMT, Ctrl->A.lon, Ctrl->A.lat, in[GMT_X], in[GMT_Y], Ctrl->A.reverse);
				}
				if (rmode == GMT_READ_MIXED) {
					/* Special case: ASCII input and at least 3 columns:
					   Columns beyond first two could be text strings */

					/* We will use gmt_strtok to step past the first 2 [or 3] columns.  The remainder
					 * will then be the user text that we want to preserve.  Since strtok places
					 * 0 to indicate start of next token we count our way to the start of the text. */

					strncpy (line, GMT->current.io.record, GMT_BUFSIZ);
					gmt_chop (line);
					pos = record[0] = 0;	/* Start with blank record */
					gmt_strtok (line, GMT_TOKEN_SEPARATORS, &pos, p);	/* Returns xstring (ignored) and update pos */
					gmt_strtok (line, GMT_TOKEN_SEPARATORS, &pos, p);	/* Returns ystring (ignored) and update pos */
					gmt_add_to_record (GMT, record, in[x], GMT_X, GMT_OUT, 2);	/* Format our output x value */
					gmt_add_to_record (GMT, record, in[y], GMT_Y, GMT_OUT, 2);	/* Format our output y value */
					if (line[pos]) {	/* Append user text */
						strcat (record, &line[pos]);
						strcat (record, GMT->current.setting.io_col_separator);
					}
					gmt_add_to_record (GMT, record, d, GMT_Z, GMT_OUT, 0);	/* Format our output z value */
					if (Ctrl->L.active) {
						gmt_add_to_record (GMT, record, xnear, fmt[0], GMT_OUT, 1);
						gmt_add_to_record (GMT, record, ynear, fmt[1], GMT_OUT, 1);
					}
					GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write this to output */
				}
				else {	/* Simply copy other columns and output */
					if (n_output == 0) {
						gmt_set_cols (GMT, GMT_OUT, gmt_get_cols (GMT, GMT_IN) + 1 + 2 * Ctrl->L.active);
						n_output = gmt_get_cols (GMT, GMT_OUT);
					}
					for (ks = 0; ks < n_fields; ks++) out[ks] = in[ks];
					out[ks++] = d;
					if (Ctrl->L.active) {
						out[ks++] = xnear;
						out[ks++] = ynear;
					}
					GMT_Put_Record (API, GMT_WRITE_DATA, out);	/* Write this to output */
				}
			}
			else {
				if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
					x_out_min = MIN (x_out_min, out[GMT_X]);
					x_out_max = MAX (x_out_max, out[GMT_X]);
					y_out_min = MIN (y_out_min, out[GMT_Y]);
					y_out_max = MAX (y_out_max, out[GMT_Y]);
				}
				if (rmode == GMT_READ_MIXED) {
					/* Special case: ASCII input and at least 3 columns:
					   Columns beyond first two could be text strings */

					/* We will use gmt_strtok to step past the first 2 [or 3] columns.  The remainder
					 * will then be the user text that we want to preserve.  Since strtok places
					 * 0 to indicate start of next token we count our way to the start of the text. */

					strncpy (line, GMT->current.io.record, GMT_BUFSIZ);
					gmt_chop (line);
					pos = record[0] = 0;	/* Start with blank record */
					gmt_strtok (line, GMT_TOKEN_SEPARATORS, &pos, p);	/* Returns xstring and update pos */
					gmt_strtok (line, GMT_TOKEN_SEPARATORS, &pos, p);	/* Returns ystring and update pos */
					gmt_add_to_record (GMT, record, out[x], o_type[GMT_X], GMT_OUT, 2);	/* Format our output x value */
					gmt_add_to_record (GMT, record, out[y], o_type[GMT_Y], GMT_OUT, 2);	/* Format our output y value */
					if (Ctrl->E.active || (Ctrl->T.active && GMT->current.proj.datum.h_given)) {
						gmt_strtok (line, GMT_TOKEN_SEPARATORS, &pos, p);		/* Returns zstring (ignored) and update pos */
						gmt_add_to_record (GMT, record, out[GMT_Z], GMT_Z, GMT_OUT, 2);	/* Format our output z value */
					}
					strcat (record, &line[pos]);
					GMT_Put_Record (API, GMT_WRITE_TEXT, record);	/* Write this to output */
				}
				else {	/* Simply copy other columns and output */
					if (n_output == 0) {
						gmt_set_cols (GMT, GMT_OUT, gmt_get_cols (GMT, GMT_IN));
						n_output = gmt_get_cols (GMT, GMT_OUT);
					}
					for (k = two; k < n_output; k++) out[k] = in[k];
					GMT_Put_Record (API, GMT_WRITE_DATA, out);	/* Write this to output */
				}
			}
			n++;
			if (n%1000 == 0) GMT_Report (API, GMT_MSG_VERBOSE, "Projected %" PRIu64 " points\r", n);
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN,  0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE) && n > 0) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Projected %" PRIu64 " points\n", n);
		sprintf (format, "Input extreme values: Xmin: %s Xmax: %s Ymin: %s Ymax %s\n",
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, format, x_in_min, x_in_max, y_in_min, y_in_max);
		if (!geodetic_calc) {
			sprintf (format, "Output extreme values: Xmin: %s Xmax: %s Ymin: %s Ymax %s\n",
			         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
			         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			GMT_Report (API, GMT_MSG_VERBOSE, format, x_out_min, x_out_max, y_out_min, y_out_max);
			if (Ctrl->I.active) {
				if (Ctrl->E.active)
					GMT_Report (API, GMT_MSG_VERBOSE, "Mapped %" PRIu64 " ECEF coordinates [m] to (lon,lat,h)\n", n);
				else if (Ctrl->N.active)
					GMT_Report (API, GMT_MSG_VERBOSE, "Converted %" PRIu64 " auxiliary (lon,lat) to geodetic coordinates [degrees]\n", n);
				else
					GMT_Report (API, GMT_MSG_VERBOSE, "Mapped %" PRIu64 " x-y pairs [%s] to lon-lat\n", n, unit_name);
			}
			else if (Ctrl->T.active && GMT->current.proj.datum.h_given)
				GMT_Report (API, GMT_MSG_VERBOSE, "Datum-converted %" PRIu64 " (lon,lat,h) triplets\n", n);
			else if (Ctrl->T.active)
				GMT_Report (API, GMT_MSG_VERBOSE, "Datum-converted %" PRIu64 " (lon,lat) pairs\n", n);
			else if (Ctrl->E.active)
				GMT_Report (API, GMT_MSG_VERBOSE, "Mapped %" PRIu64 " (lon,lat,h) triplets to ECEF coordinates [m]\n", n);
			else if (Ctrl->N.active)
				GMT_Report (API, GMT_MSG_VERBOSE, "Converted %" PRIu64 " (lon,lat) geodetic to auxiliary coordinates [degrees]\n", n);
			else if (gmt_M_is_geographic (GMT, GMT_IN))
				GMT_Report (API, GMT_MSG_VERBOSE, "Mapped %" PRIu64 " lon-lat pairs to x-y [%s]\n", n, unit_name);
			else
				GMT_Report (API, GMT_MSG_VERBOSE, "Mapped %" PRIu64 " data pairs to x-y [%s]\n", n, unit_name);
		}
		if (Ctrl->S.active && n != n_read) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " fell outside region\n", n_read - n);
	}

	gmt_M_free (GMT, out);

	Return (GMT_NOERROR);
}
