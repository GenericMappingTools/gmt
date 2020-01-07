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
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"mapproject"
#define THIS_MODULE_MODERN_NAME	"mapproject"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Forward and inverse map transformations, datum conversions and geodesy"
#define THIS_MODULE_KEYS	"<D{,LD(=,>D},W-("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>JRVbdefghijops" GMT_OPT("HMm")

enum GMT_mp_Gcodes {	/* Support for -G parsing */
	GMT_MP_VAR_POINT   = 1,	/* Compute distances from points given along a track */
	GMT_MP_FIXED_POINT = 2,	/* Compute distances from data to fixed point given by -Gx0/y0[...] */
	GMT_MP_PAIR_DIST   = 4,	/* Compute distances from data to paired point given in data columns 3&4 */
	GMT_MP_CUMUL_DIST  = 8,	/* Compute cumulative distances along track given by input data */
	GMT_MP_INCR_DIST   = 16	/* Compute incremental distances along track given by input data */
};

enum GMT_mp_Lcodes {	/* Support for -L parsing */
	GMT_MP_GIVE_CORD = 2,	/* Return the coordinates of the nearest point on the line */
	GMT_MP_GIVE_FRAC = 3	/* Return the fractional line/point number of the nearest point on the line */
};

enum GMT_mp_Wcodes {	/* Support for -W parsing */
	GMT_MP_M_WH      = 0,	/* Return width and height */
	GMT_MP_M_WIDTH   = 1,	/* Return width */
	GMT_MP_M_HEIGHT  = 2,	/* Return height */
	GMT_MP_M_POINT   = 3,	/* Return user coordinates of point given in plot coordinates */
};

enum GMT_mp_Zcodes {	/* Support for -Z parsing */
	GMT_MP_Z_DELT  = 1,	/* Return delta t */
	GMT_MP_Z_CUMT  = 2,	/* Return elapsed t */
	GMT_MP_Z_ABST  = 4,	/* Return absolute t given epoch */
	GMT_MP_Z_SPEED = 8	/* Must get speed from input column */
};

enum GMT_mp_cols {	/* Index into the extra and ecol_type arrays */
	MP_COL_AZ = 0,		/* Azimuth between to points */
	MP_COL_DS,		/* Distance between to points */
	MP_COL_CS,		/* Cumulative distance since start of segment */
	MP_COL_XN,		/* Longitude (or x) of nearest point in -L check */
	MP_COL_YN,		/* Latitude (or y) of nearest point in -L check */
	MP_COL_DT,		/* Incremental time between two points */
	MP_COL_CT,		/* Cumulative time since start of segment */
	MP_COL_AT,		/* Absolute time at present record */
	MP_COL_N		/* How many extra items there are to choose from */
};

struct MAPPROJECT_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	bool used[8];	/* Used to keep track of which items are used by -A,G,L,Z */
	struct MAPPRJ_A {	/* -Ab|B|f|Fb|B|o|O<lon0>/<lat0> */
		bool active;
		bool azims;
		bool orient;	/* true if we want orientations, not azimuths */
		bool reverse;	/* true if we want back-azimuths instead of regular azimuths */
		bool geodesic;	/* true if we want geodesic azimuths [Default is great circle azimuths] */
		unsigned int mode;
		double lon, lat;	/* Fixed point of reference */
	} A;
	struct MAPPRJ_C {	/* -C[<false_easting>/<false_northing>] */
		bool active;
		bool shift;
		double easting, northing;	/* Shifts */
	} C;
	struct MAPPRJ_D {	/* -D<c|i|p> */
		bool active;
		char unit;
	} D;
	struct MAPPRJ_E {	/* -E[<datum>] */
		bool active;
		struct GMT_DATUM datum;	/* Contains a, f, xyz[3] */
	} E;
	struct MAPPRJ_F {	/* -F[k|m|n|i|c|p] */
		bool active;
		char unit;
	} F;
	struct MAPPRJ_G {	/* -G<lon0>/<lat0>[+a][+i][+u[+|-]d|e|f|k|m|M|n|s|c|C][+v] */
		bool active;
		unsigned int mode;	/* 1 = distance to fixed point, 2 = cumulative distances, 3 = incremental distances, 4 = 2nd point in cols 3/4 */
		unsigned int sph;	/* 0 = Flat Earth, 1 = spherical [Default], 2 = ellipsoidal */
		double lon, lat;	/* Fixed point of reference */
		char unit;
	} G;
	struct MAPPRJ_I {	/* -I */
		bool active;
	} I;
	struct MAPPRJ_L {	/* -L<line.xy>[/<d|e|f|k|m|M|n|s|c|C>] */
		bool active;
		unsigned int mode;	/* 0 = dist to nearest point, 1 = also get the point, 2 = instead get seg#, pt# */
		unsigned int sph;	/* 0 = Flat Earth, 1 = spherical [Default], 2 = ellipsoidal */
		char *file;	/* Name of file with lines */
		char unit;
	} L;
	struct MAPPRJ_N {	/* -N */
		bool active;
		unsigned int mode;
	} N;
	struct MAPPRJ_Q {	/* -Q[e|d] */
		bool active;
		unsigned int mode;	/* 1 = print =Qe, 2 print -Qd, 3 print both */
	} Q;
	struct MAPPRJ_S {	/* -S */
		bool active;
	} S;
	struct MAPPRJ_T {	/* -T[h]<from>[/<to>] */
		bool active;
		bool heights;	/* True if we have heights */
		struct GMT_DATUM from;	/* Contains a, f, xyz[3] */
		struct GMT_DATUM to;	/* Contains a, f, xyz[3] */
	} T;
	struct MAPPRJ_W {	/* -W[w|h|j<code>|n<rx/ry>|g<lon/lat>|x<x/y>] */
		bool active;
		unsigned int mode;	/* See GMT_mp_Wcodes above */
		struct GMT_REFPOINT *refpoint;
	} W;
	struct MAPPRJ_Z {	/* -Z[<speed>][+c][+f][+i][+t<epoch>] */
		bool active;
		bool formatted;		/* Format duration per ISO 8601 specification */
		unsigned int mode;	/* 1 = incremental time, 2 absolute time since epoch, 3 = both */
		double speed;	/* Fixed speed in distance units per TIME_UNIT [m/s] */
		double epoch;	/* Start absolute time for increments */
	} Z;
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
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <table> %s %s\n", name, GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Ab|B|f|F|o|O[<lon0>/<lat0>][+v]] [-C[<dx></dy>]] [-D%s] [-E[<datum>]] [-F[<unit>]]\n\t[-G[<lon0>/<lat0>][+a][+i][+u<unit>][+v]]", GMT_DIM_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, " [-I] [-L<table>[+u<unit>][+p]] [-N[a|c|g|m]]\n\t[-Q[e|d]] [-S] [-T[h]<from>[/<to>] [%s] [-W[g|h|j|n|w|x]] [-Z[<speed>][+a][+i][+f][+t<epoch>]]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
		GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_j_OPT, GMT_o_OPT, GMT_p_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

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
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +v to obtain variable <lon0> <lat0> points from input columns 3-4 instead.\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -G[+u<unit>]+a to get accumulated distances along track [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -G[+u<unit>]+i to get distance increments rather than accumulated distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -G[+u<unit>]+v to obtain variable <lon0> <lat0> points from input columns 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give unit as arc (d)egree, m(e)ter, (f)oot, (k)m, arc (m)inute, (M)ile, (n)autical mile, s(u)rvey foot,\n\t   arc (s)econd, or (c)artesian [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Unit C means Cartesian distances after first projecting the input coordinates (-R, -J).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Inverse mode, i.e., get lon/lat from x/y input. [Default is lon/lat -> x/y].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Calculate minimum distances to specified line(s) in the file <table>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +u<unit> as arc (d)egree, m(e)ter, (f)oot, (k)m, arc (m)inute, (M)ile, (n)autical mile, s(u)rvey foot, arc (s)econd, or (c)artesian [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Unit C means Cartesian distances after first projecting the input coordinates (-R, -J).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Three columns are added on output: min dist and lon, lat of the closest point on the line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p to get line segment id and fractional point number instead of lon/lat.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Convert from geodetic to auxiliary latitudes; use -I for inverse conversion.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append a(uthalic), c(onformal), g(eocentric), or m(eridional) to select a conversion [geocentric].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q List all projection parameters and stop [Default].  For subsets, use\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t -W prints map width and/or height or reference point. No input files are read. See -D for plot units.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append w or h to just print width or height, respectively [Default prints both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append g<gx/gy> to return plot coordinates of reference point in map coordinates.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append j<just>  to return map coordinates of reference point with 2-char justification code (BL, MC, etc.).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append n<rx/ry> to return map coordinates of reference point with normalized coordinates in 0-1 range.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append x<px/py> to return map coordinates of reference point in plot coordinates.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t -Z Compute travel times along track using specified speed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Requires -G for setting distance calculations.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append speed in distance units per TIME_UNIT [m/s]. If the speed is not\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      given the we use input column 3 (5 if -G...+v is set).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append +a to get accumulated (elapsed) time along track [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append +i to get time increments along track.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append +f to format the elapsed time using the ISO 8601 convention.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      The FORMAT_CLOCK_OUT setting is used to determine the ss.xxx format.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append +t with epoch to get absolute time along track.\n");
	GMT_Option (API, "V,bi2,bo,d,e,f,g,h,i,j,o,p,s,:,.");
	GMT_Message (API, GMT_TIME_NONE, "\tNote: Output order is A before G before L before Z, if used.\n");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL char set_unit_and_mode (struct GMTAPI_CTRL *API, char *arg, unsigned int *mode) {
	unsigned int k = 0;
	*mode = GMT_GREATCIRCLE;	/* Default is great circle distances */
	if (strchr ("-+", arg[0])) {
		if (gmt_M_compat_check (API->GMT, 6))
			GMT_Report (API, GMT_MSG_COMPAT, "Leading -|+ with unit to set flat Earth or ellipsoidal mode is deprecated; use -j<mode> instead\n");
		else {
			GMT_Report (API, GMT_MSG_NORMAL, "Signed unit is not allowed - ignored\n");
			return arg[1];
		}
		
	}
	
	/* Fall through here if in compatibility mode */
	
	switch (arg[0]) {
		case '-': *mode = GMT_FLATEARTH;	k = 1; break;
		case '+': *mode = GMT_GEODESIC;		k = 1; break;
		case '*': *mode = GMT_GEODESIC;		k = 1; break;
	}
	return (arg[k]);
}

GMT_LOCAL unsigned int old_L_parse (struct GMTAPI_CTRL *API, char *arg, struct MAPPROJECT_CTRL *Ctrl) {
	/* [-L<table>[/[+|-]<unit>]][+] Note that [+|-] is now deprecated in GMT 6 (use -j instead) */
	int k, slash;
	gmt_M_unused(API);
	if (!gmt_M_compat_check (API->GMT, 5)) {	/* Sorry */
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L option: Expects -L<table>[+u<unit>][+p]\n");
		return 1;
	}
	Ctrl->L.file = strdup (arg);
	k = (int)strlen (Ctrl->L.file) - 1;	/* Index of last character */
	if (Ctrl->L.file[k] == '+') {		/* Flag to get point number instead of coordinates at nearest point on line */
		Ctrl->L.mode = GMT_MP_GIVE_FRAC;
		Ctrl->L.file[k] = '\0';	/* Chop off the trailing plus sign */
		k--;	/* Now points to unit */
	}
	for (slash = k; slash && Ctrl->L.file[slash] != '/'; slash--);	/* Find location of optional slash */
	if (slash && ((k - slash) < 3)) {	/* User appended /[+|-]<unit>[+].  (k-slash) should be either 1 or 2 unless we are confused by files with subdirs */
		Ctrl->L.unit = Ctrl->L.file[k];
		k--;	/* Now points to either / or the optional -/+ mode setting */
		Ctrl->L.sph = GMT_GREATCIRCLE;	/* Great circle distances */
		if (k > 0 && (Ctrl->L.file[k] == '-' || Ctrl->L.file[k] == '+')) {
			if (gmt_M_compat_check (API->GMT, 6)) {
				GMT_Report (API, GMT_MSG_COMPAT, "Leading -|+ with unit to set flat Earth or ellipsoidal mode is deprecated; use -j<mode> instead\n");
				Ctrl->L.sph = (Ctrl->L.file[k] == '-') ? GMT_FLATEARTH : GMT_GEODESIC;	/* Gave [-|+]unit */
			}
			else {
				GMT_Report (API, GMT_MSG_NORMAL, "Signed unit is not allowed\n");
				return 1;
			}
		}
		Ctrl->L.file[slash] = '\0';
	}
	return 0;
}

GMT_LOCAL unsigned int old_G_parse (struct GMT_CTRL *GMT, char *arg, struct MAPPROJECT_CTRL *Ctrl) {
	/* The [-|=] way to select spherical distance calculation mode is now deprecated in GMT 6 */
	int n;
	unsigned int n_slash, k, n_errors = 0;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""};
	char d, sign;
	size_t last;

	for (n_slash = k = 0; arg[k]; k++) if (arg[k] == '/') n_slash++;
	last = strlen (arg) - 1;
	Ctrl->G.mode = GMT_MP_VAR_POINT;	/* May get overwritten below */
	if (n_slash == 2 || n_slash == 1) {	/* Got -G<lon0/lat0>[/[+|-]unit] */
		Ctrl->G.mode = GMT_MP_FIXED_POINT;
		n = sscanf (arg, "%[^/]/%[^/]/%c%c", txt_a, txt_b, &sign, &d);
		if (n_slash == 2) {	/* Got -G<lon0/lat0>/[+|-]unit */
			if (strchr ("-+", sign)) {
				if (gmt_M_compat_check (GMT, 6)) {
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Leading -|+ with unit to set flat Earth or ellipsoidal mode is deprecated; use -j<mode> instead\n");
					Ctrl->G.sph  = (sign == '-') ? GMT_FLATEARTH : ((sign == '+') ? GMT_GEODESIC : GMT_GREATCIRCLE);	/* If sign is not +|- then it was not given */
					Ctrl->G.unit = d;	/* With sign the unit is the 4th item read by sscanf */
				}
				else {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Signed unit is not allowed\n");
					n_errors++;
				}
			}
			else {	/* No leading sign */
				Ctrl->G.sph  = GMT_GREATCIRCLE;
				Ctrl->G.unit = sign;	/* If no sign the unit is the 3rd item read by sscanf */
			}
			n_errors += gmt_M_check_condition (GMT, !strchr (GMT_LEN_UNITS "cC", (int)Ctrl->G.unit),
			                                 "Syntax error: Deprecated syntax expected -G<lon0>/<lat0>[/[-|+]%s|c|C]\n", GMT_LEN_UNITS_DISPLAY);
		}
		if (Ctrl->G.unit == 'c') gmt_set_cartesian (GMT, GMT_IN);	/* Cartesian */
		n_errors += gmt_M_check_condition (GMT, n < 2, "Syntax error: Expected deprecated syntax -G<lon0>/<lat0>[/[-|+]%s|c|C]\n",
		                                   GMT_LEN_UNITS_DISPLAY);
		n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X),
		                                     gmt_scanf_arg (GMT, txt_a, gmt_M_type (GMT, GMT_IN, GMT_X), false,
		                                     &Ctrl->G.lon), txt_a);
		n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y),
		                                     gmt_scanf_arg (GMT, txt_b, gmt_M_type (GMT, GMT_IN, GMT_Y), false,
		                                     &Ctrl->G.lat), txt_b);
		Ctrl->G.mode |= ((arg[last] == '-') ? GMT_MP_INCR_DIST : GMT_MP_CUMUL_DIST);
	}
	else if (arg[last] == '+') {	/* Got -G[[+|-]units]+ */
		Ctrl->G.mode = GMT_MP_PAIR_DIST | GMT_MP_INCR_DIST;
		Ctrl->G.unit = set_unit_and_mode (GMT->parent, arg, &Ctrl->G.sph);	/* Unit specification */
	}
	else if (arg[last] == '-') {	/* Got -G[[+|-]units]- */
		Ctrl->G.mode |= GMT_MP_INCR_DIST;
		Ctrl->G.unit = set_unit_and_mode (GMT->parent, arg, &Ctrl->G.sph);	/* Unit specification */
	}
	else {				/* Got -G[[+|-]units] only */
		Ctrl->G.mode |= GMT_MP_CUMUL_DIST;
		Ctrl->G.unit = set_unit_and_mode (GMT->parent, arg, &Ctrl->G.sph);	/* Unit specification */
	}
	if (Ctrl->G.unit == 'c') Ctrl->G.unit = 'X';	/* Internally, this is Cartesian data and distances */
	if (Ctrl->G.mode == GMT_MP_VAR_POINT) Ctrl->G.mode |= GMT_MP_CUMUL_DIST;	/* Default */
	return (n_errors);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct MAPPROJECT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to mapproject and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_slash, k, n_errors = 0, pos;
	int n;
	bool geodetic_calc = false, will_need_RJ = false;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, from[GMT_LEN256] = {""}, to[GMT_LEN256] = {""};
	char c, *p = NULL, *q = NULL;
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
				if ((p = strstr(opt->arg, "+v"))) {	/* Expect two pairs of coordinates per line for azim calculations */
					Ctrl->A.mode = GMT_MP_PAIR_DIST;
					p[0] = '\0';	/* Chop off the +v string */
				}
				n = sscanf (opt->arg, "%c%[^/]/%s", &c, txt_a, txt_b);
				if (n < 1) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -Ab|B|f|F|o|O[<lon0>/<lat0>][+v]\n");
					n_errors++;
				}
				else {
					switch (c) {
						case 'B':
							Ctrl->A.geodesic = true;
							/* fall through on purpose to 'b' */
						case 'b':
							Ctrl->A.reverse = true;
							break;
						case 'F':
							Ctrl->A.geodesic = true;
							/* fall through on purpose to 'f' */
						case 'f':
							break;
						case 'O':
							Ctrl->A.geodesic = true;
							/* fall through on purpose to 'o' */
						case 'o':
							Ctrl->A.orient = true;
							break;
						default:
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -Ab|B|f|F|o|O[<lon0>/<lat0>][+v]\n");
							n_errors++;
							break;
					}
					if (n == 3) {
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X),
						                                     gmt_scanf_arg (GMT, txt_a, gmt_M_type (GMT, GMT_IN, GMT_X), false,
						                                     &Ctrl->A.lon), txt_a);
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y),
						                                     gmt_scanf_arg (GMT, txt_b, gmt_M_type (GMT, GMT_IN, GMT_Y), false,
						                                     &Ctrl->A.lat), txt_b);
					}
					else
						Ctrl->A.azims = true;
					Ctrl->used[MP_COL_AZ] = true;	/* This will include this in the output */
				}
				if (p) p[0] = '+';	/* Restore the +v string */
				break;
			case 'C':
				Ctrl->C.active = true;
				if (opt->arg[0]) {	/* Also gave shifts */
					n_errors += gmt_M_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->C.easting, &Ctrl->C.northing) != 2,
					                                 "Syntax error: Expected -C[<false_easting>/<false_northing>]\n");
					Ctrl->C.shift = true;
				}
				will_need_RJ = true;	/* Since -C is used with projections only */
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
				will_need_RJ = true;	/* Since -F is used with projections only */
				break;
			case 'G':	/* Syntax. Old: -G[<lon0/lat0>][/[+|-]unit][+|-]  New: -G[<lon0/lat0>][+i][+a][+u<unit>][+v] */
				Ctrl->G.active = true;
				for (n_slash = k = 0; opt->arg[k]; k++) if (opt->arg[k] == '/') n_slash++;
				if (opt->arg[0] && (n_slash == 2 || !(strstr (opt->arg, "+a") || strstr (opt->arg, "+i") || strstr (opt->arg, "+u") || strstr (opt->arg, "+v"))))
					n_errors += old_G_parse (GMT, opt->arg, Ctrl);		/* -G[<lon0/lat0>][/[+|-]unit][+|-] */
				else {	/* -G[<lon0/lat0>][+i][+a][+u[+|-]<unit>][+v] */
					/* Note [+|-] is now deprecated in GMT 6; use -je instead */
					/* Watch out for +u+<unit> where the + in front of unit indicates ellipsoidal calculations.  This unfortunate syntax
					 * is easily seen as another modifier, e.g., +e, which will fail.  We temporarily replace that + sign by the * sign
					 * to avoid parsing problems. */
					if ((q = strstr (opt->arg, "+u")) && q[2] == '+') q[2] = '*';
					if (gmt_validate_modifiers (GMT, opt->arg, 'G', "aiuv")) n_errors++;
					if ((p = gmt_first_modifier (GMT, opt->arg, "aiuv")) == NULL) {	/* If just -G given then we get here; default to cumulate distances in meters */
						Ctrl->G.mode |= GMT_MP_VAR_POINT;
						Ctrl->G.mode |= GMT_MP_CUMUL_DIST;
						Ctrl->used[MP_COL_DS] = true;	/* Output incremental distances */
						break;
					}
					pos = 0;	txt_a[0] = 0;
					while (gmt_getmodopt (GMT, 'G', p, "aiuv", &pos, txt_a, &n_errors) && n_errors == 0) {
						switch (txt_a[0]) {
							case 'a': Ctrl->G.mode |= GMT_MP_CUMUL_DIST;	break;	/* Cumulative distance */
							case 'i': Ctrl->G.mode |= GMT_MP_INCR_DIST;	break;	/* Incremental distance */
							case 'v': Ctrl->G.mode |= GMT_MP_PAIR_DIST;	break;	/* Variable coordinates */
							case 'u': Ctrl->G.unit = txt_a[1];			/* Unit specification */
								break;
							default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					if (q) q[2] = '+';
					p[0] = '\0';	/* Chop off all modifiers */
					/* Here, opt->arg is -G[<lon0/lat0>] */
					if (n_slash == 1) {	/* Got -G<lon0/lat0> so we were given a fixed point */
						Ctrl->G.mode |= GMT_MP_FIXED_POINT;
						n = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b);
						if (Ctrl->G.unit == 'c') gmt_set_cartesian (GMT, GMT_IN);	/* Cartesian input */
						n_errors += gmt_M_check_condition (GMT, n < 2, "Syntax error: Expected -G<lon0>/<lat0>[+u%s|c|C]\n",
						                                   GMT_LEN_UNITS_DISPLAY);
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X),
						                                     gmt_scanf_arg (GMT, txt_a, gmt_M_type (GMT, GMT_IN, GMT_X), false,
						                                     &Ctrl->G.lon), txt_a);
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y),
						                                     gmt_scanf_arg (GMT, txt_b, gmt_M_type (GMT, GMT_IN, GMT_Y), false,
						                                     &Ctrl->G.lat), txt_b);
						if ((Ctrl->G.mode & GMT_MP_CUMUL_DIST) == 0) Ctrl->G.mode |= GMT_MP_INCR_DIST;
					}
					else if ((Ctrl->G.mode & GMT_MP_PAIR_DIST) == 0) {	/* Got -G[+u<unit>] so variable point from input */
						Ctrl->G.mode |= GMT_MP_VAR_POINT;
						if ((Ctrl->G.mode & GMT_MP_INCR_DIST) == 0) Ctrl->G.mode |= GMT_MP_CUMUL_DIST;
					}
					else {
						if ((Ctrl->G.mode & GMT_MP_INCR_DIST) == 0) Ctrl->G.mode |= GMT_MP_CUMUL_DIST;
					}
				}
				if (Ctrl->G.unit == 'c') Ctrl->G.unit = 'X';	/* Internally, this is Cartesian data and distances */
				if (Ctrl->G.mode & GMT_MP_INCR_DIST)  Ctrl->used[MP_COL_DS] = true;	/* Output incremental distances */
				if (Ctrl->G.mode & GMT_MP_CUMUL_DIST) Ctrl->used[MP_COL_CS] = true;	/* Output cumulative distances */
				if (Ctrl->G.unit == 'C') will_need_RJ = true;	/* Since unit C is projected distances */
				break;
			case 'I':
				Ctrl->I.active = true;
				will_need_RJ = true;	/* Since -I means inverse projection */
				break;
			case 'L':	/* -L<table>[+u[+|-]<unit>][+p] */
				Ctrl->L.active = true;
				if (!(strstr (opt->arg, "+u") || strstr (opt->arg, "+p") || strchr (opt->arg, '/')))
					n_errors += old_L_parse (API, opt->arg, Ctrl);
				else {
					if (gmt_validate_modifiers (GMT, opt->arg, 'L', "up")) n_errors++;
					Ctrl->L.file = gmt_get_filename (opt->arg);
					if (gmt_get_modifier (opt->arg, 'u', txt_a))
						Ctrl->L.unit = set_unit_and_mode (API, txt_a, &Ctrl->L.sph);
					if (gmt_get_modifier (opt->arg, 'p', txt_a))
						Ctrl->L.mode = GMT_MP_GIVE_FRAC;
				}
				/* Check settings */
				n_errors += gmt_M_check_condition (GMT, !strchr (GMT_LEN_UNITS "cC", (int)Ctrl->L.unit),
				            "Syntax error: Expected -L<file>[+u%s|c|C][+p]\n", GMT_LEN_UNITS_DISPLAY);
				if (strchr (GMT_LEN_UNITS, (int)Ctrl->L.unit) && gmt_M_is_cartesian (GMT, GMT_IN))
					gmt_parse_common_options (GMT, "f", 'f', "g");	/* Implicitly set -fg since user wants spherical distances */
				if (Ctrl->L.unit == 'c') Ctrl->L.unit = 'X';		/* Internally, this is Cartesian data and distances */
				if (!gmt_check_filearg (GMT, 'L', Ctrl->L.file, GMT_IN, GMT_IS_DATASET)) n_errors++;
				Ctrl->used[MP_COL_DS] = Ctrl->used[MP_COL_XN] = Ctrl->used[MP_COL_YN] = true;	/* Output dist, xnear, ynear */
				if (Ctrl->L.unit == 'C') will_need_RJ = true;	/* Since unit C is projected distances */
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
				switch (opt->arg[0]) {
					case '\0': Ctrl->W.mode = GMT_MP_M_WH; break;
					case 'w': Ctrl->W.mode = GMT_MP_M_WIDTH; break;
					case 'h': Ctrl->W.mode = GMT_MP_M_HEIGHT; break;
					case 'j': case 'n': case 'g': case 'x':
						if ((Ctrl->W.refpoint = gmt_get_refpoint (GMT, opt->arg, 'W')) == NULL) {
							n_errors++;
						}
						Ctrl->W.mode = GMT_MP_M_POINT;
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -W[w|h|j|g|n|x]\n");
						n_errors++;
						break;
				}
				will_need_RJ = true;	/* Since knowing the dimensions means region and projection */
				break;

			case 'Z':
				Ctrl->Z.active = true;
				if ((p = gmt_first_modifier (GMT, opt->arg, "aift"))) {
					unsigned int pos = 0;
					txt_a[0] = 0;
					while (gmt_getmodopt (GMT, 'Z', p, "aift", &pos, txt_a, &n_errors) && n_errors == 0) {
						switch (txt_a[0]) {
							case 'a': Ctrl->Z.mode |= GMT_MP_Z_CUMT; break; /* Cumulative time */
							case 'i': Ctrl->Z.mode |= GMT_MP_Z_DELT; break;/* Incremental time */
							case 'f': Ctrl->Z.formatted = true; break;
							case 't': Ctrl->Z.mode |= GMT_MP_Z_ABST;
								if (txt_a[1] && gmt_verify_expectations (GMT, GMT_IS_ABSTIME, gmt_scanf (GMT, &txt_a[1], GMT_IS_ABSTIME, &Ctrl->Z.epoch), &txt_a[1])) {
									GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Z+t|T : Epoch time (%s) in wrong format\n", &txt_a[1]);
									n_errors++;
								}
								break;
							default:
								n_errors++;
								break;
						}
					}
					p[0] = '\0';	/* Chop off all modifiers so range can be determined */
				}
				if (opt->arg[0])
					Ctrl->Z.speed = atof (opt->arg);
				else 
					Ctrl->Z.mode |= GMT_MP_Z_SPEED;
				if (p) p[0] = '+';	/* Restore arg */
				if (Ctrl->Z.mode == 0) Ctrl->Z.mode = GMT_MP_Z_DELT;	/* Default is time increments */
				if (Ctrl->Z.mode & GMT_MP_Z_DELT) Ctrl->used[MP_COL_DT] = true;	/* Output incremental time */
				if (Ctrl->Z.mode & GMT_MP_Z_CUMT) Ctrl->used[MP_COL_CT] = true;		/* Output elapsed time */
				if (Ctrl->Z.mode & GMT_MP_Z_ABST) Ctrl->used[MP_COL_AT] = true;		/* Output absolute time */
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (n_errors) return GMT_PARSE_ERROR;	/* Might as well return here since otherwise we may get some false warnings from below as well */
	
	geodetic_calc = (Ctrl->G.mode || Ctrl->A.active || Ctrl->L.active);

	/* The following lousy hack allows NOT having to specify -R */
	if (GMT->current.proj.is_proj4) {
		if (!GMT->common.R.active[RSET]) {	/* Means MAYBE a 1:1 scale was used */
			GMT->common.R.wesn[XLO] = 0;	GMT->common.R.wesn[XHI] = 1;
			GMT->common.R.wesn[YLO] = 0;	GMT->common.R.wesn[YHI] = 1;
			GMT->common.R.active[RSET] = true;
		}
		/* See if scale == 1:1 and if yes set -C -F */
		if (GMT->current.proj.pars[14] == 1)
			Ctrl->C.active = Ctrl->F.active = true;
	}

	if (will_need_RJ) {
		/* Treat mapproject differently than other non-PS producing modules to allow a -R from previous plot to be used.
		 * Perhaps this needs to be more nuanced beyond PS vs no-PS if other modules end up needed the same treatment */
		GMT->current.ps.active = true;	/* Briefly pretend we are a PS-producing module */
		gmt_set_missing_options (GMT, "RJ");
		GMT->current.ps.active = false;	/* Come to our senses */
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && !Ctrl->used[MP_COL_DS], "Syntax error: -Z requires -G+i\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && (Ctrl->G.mode + Ctrl->E.active + Ctrl->L.active) > 0,
	                                   "Syntax error: -T cannot work with -E, -G or -L\n");
	n_errors += gmt_M_check_condition (GMT, geodetic_calc && Ctrl->I.active, "Syntax error: -A, -G, and -L cannot work with -I\n");
	/* Can only do -p for forward projection */
	n_errors += gmt_M_check_condition (GMT, GMT->common.p.active && Ctrl->I.active, "Syntax error: -p cannot work with -I\n");
	/* Must have -J */
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active && (Ctrl->G.mode || Ctrl->L.active) && Ctrl->G.unit == 'C',
	                                   "Syntax error: Must specify -J option with selected form of -G or -L when unit is C\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.J.active && Ctrl->G.active && Ctrl->G.unit != 'C',
	                                   "Syntax error: Cannot specify -J option with selected form of -G\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.J.active && Ctrl->L.active && Ctrl->L.unit != 'C',
	                                   "Syntax error: Cannot specify -J option with selected form of -L\n");
	if (!GMT->common.R.active[RSET] && GMT->current.proj.projection_GMT == GMT_UTM && Ctrl->C.active) {	/* Set default UTM region from zone info */
		if (GMT->current.proj.utm_hemisphere == 0)		/* Default to N hemisphere if nothing is known */
			GMT->current.proj.utm_hemisphere = 1;
		if (gmt_UTMzone_to_wesn (GMT, GMT->current.proj.utm_zonex, GMT->current.proj.utm_zoney,
		                         GMT->current.proj.utm_hemisphere, GMT->common.R.wesn)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Bad UTM zone\n");
			n_errors++;
		}
		else
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "UTM zone used to generate region %g/%g/%g/%g\n",
				GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI], GMT->common.R.wesn[YLO], GMT->common.R.wesn[YHI]);

		GMT->common.R.active[RSET] = true;
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && gmt_access (GMT, Ctrl->L.file, R_OK),
	                                   "Syntax error -L: Cannot read file %s!\n", Ctrl->L.file);
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET] && !(geodetic_calc || Ctrl->T.active || Ctrl->E.active ||
	                                   Ctrl->N.active || Ctrl->Q.active), "Syntax error: Must specify -R option\n");
	n_errors += gmt_check_binary_io (GMT, 2);
	n_errors += gmt_M_check_condition (GMT, (Ctrl->D.active + Ctrl->F.active) == 2, "Syntax error: Can specify only one of -D and -F\n");
	n_errors += gmt_M_check_condition (GMT, ((Ctrl->T.active && GMT->current.proj.datum.h_given) || Ctrl->E.active) &&
	                                   GMT->common.b.active[GMT_IN] && gmt_get_cols (GMT, GMT_IN) < 3,
	                                   "Syntax error: For -E or -T, binary input data (-bi) must have at least 3 columns\n");

	if (!(n_errors || GMT->common.R.active[RSET])) {
		GMT->common.R.wesn[XLO] = 0.0;	GMT->common.R.wesn[XHI] = 360.0;
		GMT->common.R.wesn[YLO] = -90.0;	GMT->common.R.wesn[YHI] = 90.0;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mapproject (void *V_API, int mode, void *args) {
	int ks, n_fields, two, way, error = 0, fmt[2], save[2] = {0,0}, unit = 0, proj_type = 0, lat_mode = 0;
	
	bool line_start = true, do_geo_conv = false, double_whammy = false, first = true;
	bool geodetic_calc = false, datum_conv_only = false, along_track = false;

	unsigned int i = 0, col, speed_col = 2;
	unsigned int ecol_type[MP_COL_N] = {GMT_IS_FLOAT, GMT_IS_FLOAT, GMT_IS_FLOAT, GMT_IS_FLOAT,
                                        GMT_IS_FLOAT, GMT_IS_FLOAT, GMT_IS_FLOAT, GMT_IS_ABSTIME};

	uint64_t row, n_read_in_seg, seg, n_read = 0, n = 0, k, n_output = 0;

	double x_in = 0.0, y_in = 0.0, d = 0.0, fwd_scale, inv_scale, xtmp, ytmp, *out = NULL;
	double xmin, xmax, ymin, ymax, inch_to_unit, unit_to_inch, u_scale, y_out_min;
	double x_in_min, x_in_max, y_in_min, y_in_max, x_out_min, x_out_max, y_out_max;
	double xnear = 0.0, ynear = 0.0, lon_prev = 0, lat_prev = 0, **data = NULL, *in = NULL;
	double speed = 0, last_speed = -1.0, extra[MP_COL_N];	/* Max possible extra output columns from -A -G -L -Z */

	char format[GMT_BUFSIZ] = {""}, unit_name[GMT_LEN64] = {""}, scale_unit_name[GMT_LEN64] = {""};

	bool (*map_fwd) (struct GMT_CTRL *, double, double, double *, double *);	/* Pointers to the selected forward mapping function */
	void (*map_inv) (struct GMT_CTRL *, double *, double *, double, double);	/* Pointers to the selected inverse mapping function */

	struct GMT_DATATABLE *xyline = NULL;
	struct GMT_DATASET *Lin = NULL;
	struct GMT_RECORD *In = NULL, *Out = NULL;
	struct MAPPROJECT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);
	if (!GMT->common.J.active && gmt_M_is_cartesian (GMT, GMT_IN)) { /* -J not given and input not specified as lon/lat; check if we should add -fg */
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

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");

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

	geodetic_calc = (Ctrl->G.mode || Ctrl->A.active || Ctrl->L.active || Ctrl->Z.active);
	gmt_M_memset (extra, MP_COL_N, double);
	if (Ctrl->Z.active) {	/* Initialize a few things regarding travel time calculations */
		speed = Ctrl->Z.speed;
		if (Ctrl->G.mode & GMT_MP_PAIR_DIST) speed_col = 4;
		if (Ctrl->Z.mode & GMT_MP_Z_ABST) extra[MP_COL_AT] = Ctrl->Z.epoch;	/* Need to initiate epoch time */
	}
		
	if (Ctrl->T.active && GMT->current.proj.projection_GMT != GMT_LINEAR && GMT->common.R.active[RSET]) {	/* Do datum shift & project coordinates */
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
		save[GMT_X] = (Ctrl->G.unit == 'X') ? GMT_IS_FLOAT : gmt_M_type (GMT, GMT_OUT, GMT_X);
		save[GMT_Y] = (Ctrl->G.unit == 'X') ? GMT_IS_FLOAT : gmt_M_type (GMT, GMT_OUT, GMT_Y);
	}
	u_scale = (Ctrl->I.active) ? inv_scale : fwd_scale;

	if (!GMT->common.J.active) {	/* Supply dummy linear proj */
		if (Ctrl->G.active && Ctrl->G.unit == 'X') {
			gmt_set_cartesian (GMT, GMT_IN);
			gmt_parse_common_options (GMT, "J", 'J', "x1");	/* Fake linear Cartesian projection */
		}
		else
			gmt_parse_common_options (GMT, "J", 'J', "x1d");	/* Fake linear degree projection */
		if (!GMT->common.R.active[RSET]) {
			GMT->common.R.wesn[XLO] = 0.0;	GMT->common.R.wesn[XHI] = 360.0;
			GMT->common.R.wesn[YLO] = -90.0;	GMT->common.R.wesn[YHI] = 90.0;
		}
	}

	if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	Out = gmt_new_record (GMT, NULL, NULL);
	
	if (Ctrl->W.active) {	/* Print map dimensions or reference point and exit */
		double w_out[2] = {0.0, 0.0}, x_orig, y_orig;
		unsigned int wmode = 0;
		char key[3] = {""};
		switch (Ctrl->W.mode) {
			case GMT_MP_M_WIDTH:
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reporting map width in %s\n", unit_name);
				w_out[GMT_X] = GMT->current.proj.rect[XHI] * inch_to_unit;	n_output = 1;	break;
			case GMT_MP_M_HEIGHT:
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reporting map height in %s\n", unit_name);
				w_out[GMT_X] = GMT->current.proj.rect[YHI] * inch_to_unit;	n_output = 1;	break;
			case GMT_MP_M_POINT:
				wmode = Ctrl->W.refpoint->mode;
				x_orig = Ctrl->W.refpoint->x;	y_orig = Ctrl->W.refpoint->y;
				gmt_set_refpoint (GMT, Ctrl->W.refpoint);	/* Finalize reference point plot coordinates, if needed */
				switch (wmode) {
					case GMT_REFPOINT_NORM:
						gmt_set_geographic (GMT, GMT_OUT);	/* Inverse projection expects x,y and gives lon, lat */
						GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reporting coordinates of normalized point (%g/%g)\n", x_orig, y_orig);
						gmt_xy_to_geo (GMT, &w_out[GMT_X], &w_out[GMT_Y], Ctrl->W.refpoint->x, Ctrl->W.refpoint->y);
						break;
					case GMT_REFPOINT_PLOT:
						GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reporting coordinates of plot point (%g/%g)\n", x_orig, y_orig);
						gmt_set_geographic (GMT, GMT_OUT);	/* Inverse projection expects x,y and gives lon, lat */
						gmt_xy_to_geo (GMT, &w_out[GMT_X], &w_out[GMT_Y], Ctrl->W.refpoint->x, Ctrl->W.refpoint->y);
						break;
					case GMT_REFPOINT_JUST:
					case GMT_REFPOINT_JUST_FLIP:	/* Ignored and treated like j */
						gmt_just_to_code (GMT, Ctrl->W.refpoint->justify, key);
						GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reporting coordinates of justification point (%s)\n", key);
						gmt_set_geographic (GMT, GMT_OUT);	/* Inverse projection expects x,y and gives lon, lat */
						gmt_xy_to_geo (GMT, &w_out[GMT_X], &w_out[GMT_Y], Ctrl->W.refpoint->x, Ctrl->W.refpoint->y);
						break;
					case GMT_REFPOINT_MAP:
						GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reporting plot coordinates of user point (%g/%g)\n", x_orig, y_orig);
						w_out[GMT_X] = Ctrl->W.refpoint->x * inch_to_unit; w_out[GMT_Y] = Ctrl->W.refpoint->y * inch_to_unit;
						break;
					case GMT_REFPOINT_NOTSET:
						GMT_Report (API, GMT_MSG_LONG_VERBOSE, "No reference point set!\n");
						Return (GMT_RUNTIME_ERROR);
						break;
				}
				gmt_free_refpoint (GMT, &Ctrl->W.refpoint);
				n_output = 2;	break;
			default:
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Reporting map width and height in %s\n", unit_name);
				w_out[GMT_X] = GMT->current.proj.rect[XHI] * inch_to_unit;
				w_out[GMT_Y] = GMT->current.proj.rect[YHI] * inch_to_unit;
				n_output = 2;
			break;
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)n_output, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_NONE) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
		Out->data = w_out;
		GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}
		gmt_M_free (GMT, Out);
		Return (GMT_NOERROR);
	}

	if (Ctrl->G.unit == 'X') gmt_set_cartesian (GMT, GMT_IN);	/* Cartesian */
	if (Ctrl->L.unit == 'X') gmt_set_cartesian (GMT, GMT_IN);	/* Cartesian */

	if (Ctrl->G.mode && proj_type != GMT_GEO2CART) {	/* Ensure we use the selected output coordinates */
		gmt_set_column (GMT, GMT_OUT, GMT_X, save[GMT_X]);
		gmt_set_column (GMT, GMT_OUT, GMT_Y, save[GMT_Y]);
	}
	if (datum_conv_only)	/* Both input and output is geographic */
		gmt_set_geographic (GMT, GMT_OUT);

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

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Transform ");
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

		gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -L files */
		if ((Lin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, Ctrl->L.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (Lin->n_columns < 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)Lin->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
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

	if ((gmt_M_is_geographic (GMT, GMT_IN) || Ctrl->E.active) && Ctrl->I.active) {
		gmt_set_geographic (GMT, GMT_OUT);	/* Inverse projection expects x,y and gives lon, lat */
		gmt_set_cartesian (GMT, GMT_IN);
	}
	else if (datum_conv_only || Ctrl->N.active) {	/* Both in and out are geographic */
		gmt_set_geographic (GMT, GMT_IN);
		gmt_set_geographic (GMT, GMT_OUT);
		gmt_set_column (GMT, GMT_IO, GMT_Z, GMT_IS_FLOAT);
	}
	else if (gmt_M_is_geographic (GMT, GMT_OUT)) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Override -fog for normal operation\n");
		gmt_set_cartesian (GMT, GMT_OUT);
	}

	/* Specify input and output expected columns */
	if ((error = GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_FIX)) != GMT_NOERROR) {
		Return (error);
	}

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

	if (Ctrl->L.active)	{	/* Possibly adjust output types */
		if (Ctrl->L.mode == GMT_MP_GIVE_FRAC)	/* Want fractional point locations */
			fmt[0] = fmt[1] = GMT_Z;	/* These are just regular floating points */
		else {			/* Want nearest point coordinates; set correct output column type */
			fmt[0] = GMT_X;
			fmt[1] = GMT_Y;
			/* Only select lon, lat if input data is geographic and proj_type is not GMT_GEO2CART */
			if (gmt_M_is_geographic (GMT, GMT_IN) && proj_type != GMT_GEO2CART) {
				ecol_type[MP_COL_XN] = GMT_IS_LON;
				ecol_type[MP_COL_YN] = GMT_IS_LAT;
			}
		}
	}
	if (Ctrl->Z.formatted) ecol_type[MP_COL_CT] = GMT_IS_DURATION;
	if (Ctrl->N.active) lat_mode = Ctrl->N.mode + Ctrl->I.active;

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}
	along_track = (Ctrl->G.mode && ((Ctrl->G.mode & GMT_MP_CUMUL_DIST) || (Ctrl->G.mode & GMT_MP_INCR_DIST)));
	if ((Ctrl->G.mode & GMT_MP_PAIR_DIST) && (Ctrl->G.mode & GMT_MP_INCR_DIST)) along_track = false;	/* Not along track (i.e., per record) when doing it per record */
	n = n_read_in_seg = 0;
	out = gmt_M_memory (GMT, NULL, GMT_MAX_COLUMNS, double);
	Out->data = out;
	data = (proj_type == GMT_GEO2CART) ? &out : &in;	/* Using projected or original coordinates */
	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
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
				last_speed = -1.0;
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}
		if (In == NULL) {	/* Crazy safety valve but it should never get here (to please Coverity) */
			GMT_Report (API, GMT_MSG_NORMAL, "Internal error: input pointer is NULL where it should not be, aborting\n");
			Return (GMT_PTR_IS_NULL);
		}
		if (first) {
			unsigned int n_in_cols = (unsigned int)gmt_get_cols (GMT, GMT_IN);
			if ((error = GMT_Set_Columns (API, GMT_OUT, n_in_cols, gmt_M_colmode (In->text))) != GMT_NOERROR) {
				Return (error);
			}
			first = false;
		}
		in = In->data;	/* Only need to process numerical part here */
		Out->text = In->text;
		if (gmt_M_rec_is_gap (GMT)) {	/* Gap detected.  Write a segment header but continue on since record is actually data */
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
			GMT->current.io.status = 0;	/* Done with gap */
			line_start = true;
			n_read_in_seg = 0;
			last_speed = -1.0;
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

			/* Simply copy other columns and output */
			if (n_output == 0) {
				unsigned int n_in_cols = (unsigned int)gmt_get_cols (GMT, GMT_IN);
				if ((error = GMT_Set_Columns (API, GMT_OUT, n_in_cols, gmt_M_colmode (Out->text))) != GMT_NOERROR) {
					Return (error);
				}
				n_output = gmt_get_cols (GMT, GMT_OUT);
			}
			for (k = two; k < n_output; k++) out[k] = in[k];
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
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

			if (geodetic_calc) {	/* Get either distances, azimuths, or travel times */
				if (Ctrl->G.mode) {	/* Distances of some sort */
					if (Ctrl->G.mode & GMT_MP_PAIR_DIST)	/* Segment distances from each data record using 2 extra coordinates */
						extra[MP_COL_DS] = gmt_distance (GMT, in[GMT_X], in[GMT_Y], in[2], in[3]);
					else	/* Distance from fixed point via -G OR the last track point */
						extra[MP_COL_DS] = gmt_distance (GMT, Ctrl->G.lon, Ctrl->G.lat, in[GMT_X], in[GMT_Y]);
					if (along_track) {	/* Along-track calculation */
						if (line_start && (Ctrl->G.mode & GMT_MP_VAR_POINT))
							extra[MP_COL_CS] = extra[MP_COL_DS] = 0.0;
						else
							extra[MP_COL_CS] += extra[MP_COL_DS];
						line_start = false;
						if ((Ctrl->G.mode & GMT_MP_FIXED_POINT) == 0) {	/* Save previous point in G */
							Ctrl->G.lon = in[GMT_X];
							Ctrl->G.lat = in[GMT_Y];
						}
					}
				}
				if (Ctrl->L.active) {	/* Compute closest distance to line */
					y_in = (do_geo_conv) ? gmt_lat_swap (GMT, (*data)[GMT_Y], GMT_LATSWAP_G2O) : (*data)[GMT_Y];	/* Convert to geocentric */
					(void) gmt_near_lines (GMT, (*data)[GMT_X], y_in, xyline, Ctrl->L.mode, &d, &xnear, &ynear);
					if (do_geo_conv && Ctrl->L.mode != GMT_MP_GIVE_FRAC)
						ynear = gmt_lat_swap (GMT, ynear, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
					extra[MP_COL_DS] = d;
					extra[MP_COL_XN] = xnear;
					extra[MP_COL_YN] = ynear;
				}
				if (Ctrl->A.active) {	/* Azimuth */
					if (Ctrl->A.azims) {	/* Azimuth from previous point */
						if (Ctrl->A.mode == GMT_MP_PAIR_DIST)	/* Azimuths from each data record using 2 points */
							d = gmt_az_backaz (GMT, in[GMT_X], in[GMT_Y], in[2], in[3], Ctrl->A.reverse);
						else if (n_read_in_seg == 1)	/* First point has undefined azimuth since there is no previous point */
							d = GMT->session.d_NaN;
						else {
							d = gmt_az_backaz (GMT, lon_prev, lat_prev, in[GMT_X], in[GMT_Y], Ctrl->A.reverse);
						}
						lon_prev = in[GMT_X];	/* Update previous point */
						lat_prev = in[GMT_Y];
					}
					else	/* Azimuths with respect to a fixed point */
						d = gmt_az_backaz (GMT, Ctrl->A.lon, Ctrl->A.lat, in[GMT_X], in[GMT_Y], Ctrl->A.reverse);
					if (Ctrl->A.orient) {	/* Want orientations in -90/90 instead */
						d = fmod (2.0 * d, 360.0) * 0.5;	/* Get orientation */
						if (d > 90.0) d-= 180.0;
					}
					extra[MP_COL_AZ] = d;
				}
				if (Ctrl->Z.active) {	/* Time calculations */
					if (Ctrl->Z.mode & GMT_MP_Z_SPEED) speed = in[speed_col];	/* Get variable speed from input file */
					if (last_speed == -1.0) last_speed = in[speed_col];	/* Get variable speed from input file */
					extra[MP_COL_DT] = 2.0 * extra[MP_COL_DS] / (speed + last_speed);	/* Incremental time */
					extra[MP_COL_CT] += extra[MP_COL_DT];			/* Elapsed time */
					if (Ctrl->Z.mode & GMT_MP_Z_ABST)				/* Absolute time */
						extra[MP_COL_AT] += extra[MP_COL_DT];
					last_speed = speed;
				}
				/* Simply copy other columns and output */
				if (n_output == 0) {
					unsigned int n_in_cols = (unsigned int)gmt_get_cols (GMT, GMT_IN);
					for (col = 0, k = 0; col < MP_COL_N; col++) if (Ctrl->used[col]) k++;
					if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)(n_in_cols + k), gmt_M_colmode (Out->text))) != GMT_NOERROR) {
						Return (error);
					}
					n_output = gmt_get_cols (GMT, GMT_OUT);
					if (geodetic_calc) {	/* Update the output column types to the extra items we added */
						for (col = 0, k = n_fields; col < MP_COL_N; col++) {
							if (Ctrl->used[col]) {
								gmt_set_column (GMT, GMT_OUT, (unsigned int)k, ecol_type[col]);
								k++;
							}	
						}
					}
				}
				for (ks = 0; ks < n_fields; ks++) out[ks] = in[ks];
				for (col = 0; col < MP_COL_N; col++)
					if (Ctrl->used[col]) out[ks++] = extra[col];
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
			}
			else {
				if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
					x_out_min = MIN (x_out_min, out[GMT_X]);
					x_out_max = MAX (x_out_max, out[GMT_X]);
					y_out_min = MIN (y_out_min, out[GMT_Y]);
					y_out_max = MAX (y_out_max, out[GMT_Y]);
				}
				/* Simply copy other columns and output */
				if (n_output == 0) {
					unsigned int n_in_cols = (unsigned int)gmt_get_cols (GMT, GMT_IN);
					if ((error = GMT_Set_Columns (API, GMT_OUT, n_in_cols, gmt_M_colmode (Out->text))) != GMT_NOERROR) {
						Return (error);
					}
					n_output = gmt_get_cols (GMT, GMT_OUT);
				}
				for (k = two; k < n_output; k++) out[k] = in[k];
				GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
			}
			n++;
			if (n%1000 == 0) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Projected %" PRIu64 " points\r", n);
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN,  0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE) && n > 0) {
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Projected %" PRIu64 " points\n", n);
		sprintf (format, "Input extreme values: Xmin: %s Xmax: %s Ymin: %s Ymax %s\n",
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, format, x_in_min, x_in_max, y_in_min, y_in_max);
		if (!geodetic_calc) {
			sprintf (format, "Output extreme values: Xmin: %s Xmax: %s Ymin: %s Ymax %s\n",
			         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
			         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, format, x_out_min, x_out_max, y_out_min, y_out_max);
			if (Ctrl->I.active) {
				if (Ctrl->E.active)
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Mapped %" PRIu64 " ECEF coordinates [m] to (lon,lat,h)\n", n);
				else if (Ctrl->N.active)
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Converted %" PRIu64 " auxiliary (lon,lat) to geodetic coordinates [degrees]\n", n);
				else
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Mapped %" PRIu64 " x-y pairs [%s] to lon-lat\n", n, unit_name);
			}
			else if (Ctrl->T.active && GMT->current.proj.datum.h_given)
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Datum-converted %" PRIu64 " (lon,lat,h) triplets\n", n);
			else if (Ctrl->T.active)
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Datum-converted %" PRIu64 " (lon,lat) pairs\n", n);
			else if (Ctrl->E.active)
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Mapped %" PRIu64 " (lon,lat,h) triplets to ECEF coordinates [m]\n", n);
			else if (Ctrl->N.active)
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Converted %" PRIu64 " (lon,lat) geodetic to auxiliary coordinates [degrees]\n", n);
			else if (gmt_M_is_geographic (GMT, GMT_IN))
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Mapped %" PRIu64 " lon-lat pairs to x-y [%s]\n", n, unit_name);
			else
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Mapped %" PRIu64 " data pairs to x-y [%s]\n", n, unit_name);
		}
		if (Ctrl->S.active && n != n_read) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " fell outside region\n", n_read - n);
	}

	gmt_M_free (GMT, out);
	gmt_M_free (GMT, Out);

	Return (GMT_NOERROR);
}
