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
 * API functions to support the gmtselect application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: gmtselect is a general-purpose spatial filter.  Data pass
 * or fail basedon one or more conditions.  Seven conditions may be set:
 *
 *	1. Only data inside a rectangular area may pass
 *	2. Only data within a certain distance from given points may pass
 *	3. Only data within a certain distance from given lines may pass
 *	4. Only data within given polygons may pass
 *	5. Only data within the coastline may pass
 *	6. Only data with z-values within specified range may pass
 *  7. Only data with (x,y) locations inside a mask grid may pass
 *
 * Distances are calculated in the users units using Euclidean geometry
 * unless a map projection and region (-R -J) are used.  Then, distances
 * are calculated using spherical geometry and converted to km, and any
 * distances given in options or via headers are assumed to be in km.
 *
 * Any one of these conditions may be negated for the opposite result
 * Both binary and ASCII data files are accommodated
 */
 
#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtselect"
#define THIS_MODULE_MODERN_NAME	"gmtselect"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Select data table subsets based on multiple spatial criteria"
#define THIS_MODULE_KEYS	"<D{,CD(=,FD(,LD(=,>D},GG("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>JRVabdefghios" GMT_OPT("HMm")

#define GMTSELECT_N_CLASSES	(GSHHS_MAX_LEVEL + 1)	/* Number of bands separated by the levels */

#define F_ITEM	0
#define N_ITEM	1

enum GMTSELECT {	/* Indices for the various tests */
	GMT_SELECT_R = 0,
	GMT_SELECT_C,
	GMT_SELECT_L,
	GMT_SELECT_F,
	GMT_SELECT_N,
	GMT_SELECT_Z,
	GMT_SELECT_G,
	GMT_SELECT_N_TESTS	/* Number of specific tests available */
};
	
struct GMTSELECT_DATA {	/* Used for temporary storage when sorting data on x coordinate */
	double x, y, d;
};

struct GMTSELECT_ZLIMIT {	/* Used to hold info for each -Z option given */
	unsigned int col;	/* Column to test */
	bool equal;	/* Just check if z == min within 5 ULps */
	bool invert;	/* Just check if the test should be negative != etc */
	double min;	/* Smallest z-value to pass through, for this column */
	double max;	/* Largest z-value to pass through, for this column */
};

struct GMTSELECT_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct GMTSELECT_A {	/* -A<min_area>[/<min_level>/<max_level>] */
		bool active;
		struct GMT_SHORE_SELECT info;
	} A;
	struct GMTSELECT_C {	/* [-C[-|=|+]<dist>[unit]/<ptfile>] */
		bool active;
		int mode;	/* Form of distance calculation (can be negative) */
		double dist;	/* Radius of influence for each point */
		char unit;	/* Unit name */
		char *file;	/* Name of file with points */
	} C;
	struct GMTSELECT_D {	/* -D<resolution>[+f] */
		bool active;
		bool force;	/* if true, select next highest level if current set is not available */
		char set;	/* One of f, h, i, l, c, or a for auto */
	} D;
	struct GMTSELECT_E {	/* -E<operators> , <op> = combination or f,n */
		bool active;
		unsigned int inside[2];	/* if 2, then a point exactly on a polygon boundary is considered OUTSIDE, else 1 */
	} E;
	struct GMTSELECT_L {	/* -L[p][-|=|+]<dist>[unit]/<lfile> */
		bool active;
		unsigned int end_mode;	/* Controls what happens beyond segment endpoints */
		int mode;	/* Form of distance calculation (can be negative) */
		double dist;	/* Distance of influence for each line */
		char unit;	/* Unit name */
		char *file;	/* Name of file with lines */
	} L;
	struct GMTSELECT_F {	/* -F<polygon> */
		bool active;
		char *file;	/* Name of file with polygons */
	} F;
	struct GMTSELECT_G {	/* -G<grid_mask> */
		bool active;
		char *file;
	} G;
	struct GMTSELECT_I {	/* -Icflrszg */
		bool active;
		bool pass[GMT_SELECT_N_TESTS];	/* One flag for each setting */
	} I;
	struct GMTSELECT_N {	/* -N<maskvalues> */
		bool active;
		unsigned int mode;	/* 1 if dry/wet only, 0 if 5 mask levels */
		bool mask[GMTSELECT_N_CLASSES];	/* Mask for each level */
	} N;
	struct GMTSELECT_Z {	/* -Z<min>/<max>[+c<col>][+i][+a] */
		bool active;
		bool or;		/* Logical or.  If any test passes then we pass the record [all tests must pass] */
		unsigned int n_tests;	/* How many of these tests did we get */
		unsigned int max_col;	/* The largest column we encountered */
		struct GMTSELECT_ZLIMIT *limit;
	} Z;
	struct GMTSELECT_dbg {	/* -+step */
		bool active;
		double step;
	} dbg;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	int i;
	struct GMTSELECT_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct GMTSELECT_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	C->A.info.high = GSHHS_MAX_LEVEL;		/* Include all GSHHS levels */
	C->D.set = 'l';							/* Low-resolution coastline data */
	C->E.inside[F_ITEM] = C->E.inside[N_ITEM] = GMT_ONEDGE;         /* Default is that points on a boundary are inside */
	for (i = 0; i < GMT_SELECT_N_TESTS; i++) C->I.pass[i] = true;    /* Default is to pass if we are inside */
	gmt_M_memset (C->N.mask, GMTSELECT_N_CLASSES, bool);            /* Default for "wet" areas = false (outside) */
	C->N.mask[1] = C->N.mask[3] = true;				/* Default for "dry" areas = true (inside) */
	C->Z.max_col = 1;						/* Minimum number of columns to expect is 1 unless "x,y" data are implied [2] */
	
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTSELECT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);	
	gmt_M_str_free (C->F.file);	
	gmt_M_str_free (C->L.file);	
	if (C->Z.n_tests) gmt_M_free (GMT, C->Z.limit);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int compare_x (const void *point_1, const void *point_2) {
	const struct GMTSELECT_DATA *p1 = point_1, *p2 = point_2;

	if (p1->x < p2->x) return (-1);
	if (p1->x > p2->x) return (1);
	return (0);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] [%s]\n", name, GMT_A_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<ptfile>+d%s] [-D<resolution>][+f] [-E[f][n]] [-F<polygon>] [-G<gridmask>] [%s]\n",
	             GMT_DIST_OPT, GMT_J_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-I[cfglrsz] [-L<lfile>+d%s[+p]] [-N<info>] [%s]\n\t[%s] [-Z<min>[/<max>][+c<col>][+a][+i]] [%s] "
	             "[%s]\n\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
	             GMT_DIST_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_a_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT,
				 GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Option (API, "<");
	gmt_GSHHG_syntax (API->GMT, 'A');
	GMT_Message (API, GMT_TIME_NONE, "\t   (ignored  unless -N is set).\n");
	gmt_dist_syntax (API->GMT, 'C', "Pass locations that are within <dist> of any point in the ASCII <ptfile>.");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give distance as 0 if 3rd column of <ptfile> has individual distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -R -J to compute mapped Cartesian distances in cm, inch, m, or points [%s].\n",
	             API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t-D Choose one of the following resolutions: (Ignored unless -N is set).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f - full resolution (may be very slow for large regions).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     h - high resolution (may be slow for large regions).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     i - intermediate resolution.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     l - low resolution [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c - crude resolution, for tasks that need crude continent outlines only.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +f to use a lower resolution should the chosen one not be available [abort].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Indicate if points exactly on a polygon boundary are inside or outside.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append f and/or n to modify the -F option or -N option, respectively,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to consider such points to be outside the feature [inside].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Pass locations that are inside the polygons in the ASCII <polygon> file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Pass locations that are inside the non-zero, non-NaN nodes of the grid <gridmask>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Reverse the tests, i.e., pass locations outside the region.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Supply a combination of cflrz where each flag means:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c will pass locations beyond the minimum distance to the points in -C.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f will pass locations outside the polygons in -F.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     g will pass locations outside the grid mask in -G.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     l will pass locations beyond the minimum distance to the lines in -L.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     r will pass locations outside the region given in -R [and -J].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     s will pass locations that otherwise would be skipped in -N.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     z will pass locations outside the range given in -Z (single -Z required).\n");
	GMT_Option (API, "J");
	gmt_dist_syntax (API->GMT, 'L', "Pass locations that are within <dist> of any line in ASCII <linefile>.");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give distance as 0 if 2nd column of segment headers have individual distances.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -R -J to compute mapped Cartesian distances in cm, inch, or points [%s].\n",
	             API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, Append +p to exclude points projecting beyond a line's endpoints.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set if a point outside or inside a geographic feature should be s(kipped) or k(ept).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append o to let feature boundary be considered outside [Default is inside].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify this information with s or k using 1 of 2 formats:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -N<wet>/<dry>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -N<ocean>/<land>/<lake>/<island>/<pond>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   k means keep and s means skip [Default is s/k/s/k/s (i.e., s/k)].\n");
	GMT_Option (API, "R,V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Assume the 3rd data column contains z-values and we want to keep records with\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <min> <= z <= <max>.  Use - for <min> or <max> if there is no lower/upper limit.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c<col> to select another column than the third [2].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <max> is not given we pass records whose z equal <min> within 5 ulps.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The -Z option is repeatable.  If given more than once then these modifiers may be useful:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +a: Pass the record if any of the tests are true [all tests must be true in order to pass].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +i: Reverse an individual test since -Iz only applies to a single test.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If +c is not given then it is incremented automatically, starting at 2.\n");
	GMT_Option (API, "a,bi0");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   Default is 2 input columns (3 if -Z is used).\n");
	GMT_Option (API, "bo,d,e,f,g,h,i");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   Does not apply to files given via -C, -F, or -L.\n");
	GMT_Option (API, "o,s,:,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int old_C_parse (struct GMTAPI_CTRL *API, char *arg, struct GMTSELECT_CTRL *Ctrl) {
	int j;
	bool fix = false;
	/* Parse older versions of the -C syntax */
	if (!gmt_M_compat_check (API->GMT, 5)) {	/* Sorry */
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Expects -C<file>+d%s\n", GMT_DIST_OPT);
		return 1;
	}
	if (arg[0] == 'f') {
		if (gmt_M_compat_check (API->GMT, 4)) {	/* Allow old-style quick-mode specification */
			GMT_Report (API, GMT_MSG_COMPAT, "Option -Cf is deprecated; use -C<file>+d-<distance> instead\n");
			arg[0] = '-';
			fix = true;
		}
		else {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Expects -C<file>+d%s\n", GMT_DIST_OPT);
			return 1;
		}
	}
	for (j = 0; arg[j] && arg[j] != '/'; j++);
	if (arg[j]) {	/* Found a file name */
		Ctrl->C.file = strdup (&arg[j+1]);
		arg[j] = '\0';	/* Chop off the /filename part */
		Ctrl->C.mode = gmt_get_distance (API->GMT, arg, &(Ctrl->C.dist), &(Ctrl->C.unit));
		arg[j] = '/';	/* Restore the /filename part */
	}
	else {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Expects -C<file>+d%s\n", GMT_DIST_OPT);
		return 1;
	}
	if (fix) arg[0] = 'f';	/* Just to leave the original args unaltered */
	return 0;
}

GMT_LOCAL int old_L_parse (struct GMTAPI_CTRL *API, char *arg, struct GMTSELECT_CTRL *Ctrl) {
	int j, k = 0;
	if (!gmt_M_compat_check (API->GMT, 5)) {	/* Sorry */
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L option: Expects -L[p]%s/<file>\n", GMT_DIST_OPT);
		return 1;
	}
	if (arg[k] == 'p') {	/* Disallow points beyond endpoints */
		Ctrl->L.end_mode = 10;
		k++;
	}
	for (j = k; arg[j] && arg[j] != '/'; j++);	/* Find the first slash */
	if (!arg[j]) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L option: Expects -L[p]%s/<file>\n", GMT_DIST_OPT);
		return 1;
	}
	else {
		if (gmt_check_filearg (API->GMT, 'L', &arg[j+1], GMT_IN, GMT_IS_DATASET))
			Ctrl->L.file = strdup (&arg[j+1]);
		else {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L option: No file given\n");
			return 1;
		}
		arg[j] = '\0';	/* Chop off the /filename part */
		Ctrl->L.mode = gmt_get_distance (API->GMT, &arg[k], &(Ctrl->L.dist), &(Ctrl->L.unit));
		arg[j] = '/';	/* Restore the /filename part */
	}
	return 0;
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTSELECT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtselect and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos, j, col, n_z_alloc = 0, z_col = GMT_Z;
	bool invert = false;
	char buffer[GMT_BUFSIZ] = {""}, za[GMT_LEN64] = {""}, zb[GMT_LEN64] = {""}, p[GMT_LEN256] = {""}, *c = NULL;
	enum gmt_col_enum ctype;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Limit GSHHS features */
				Ctrl->A.active = true;
				gmt_set_levels (GMT, opt->arg, &Ctrl->A.info);
				break;
			case 'C':	/* Near a point test  Syntax -C<pfile>+d<distance>  */
				Ctrl->C.active = true;
				if ((c = strstr (opt->arg, "+d")) == NULL) {	/* Must be old syntax or error */
					n_errors += old_C_parse (API, opt->arg, Ctrl);
					break;
				}
				/* Here we perform new syntax parsing */
				if (opt->arg[0] == 0) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: No file given\n");
					n_errors++;
				}
				else
					Ctrl->C.file = gmt_get_filename (opt->arg);
				Ctrl->C.mode = gmt_get_distance (GMT, &c[2], &(Ctrl->C.dist), &(Ctrl->C.unit));
				break;
			case 'D':	/* Set GSHHS resolution */
				Ctrl->D.active = true;
				Ctrl->D.set = opt->arg[0];
				Ctrl->D.force = (opt->arg[1] == '+' && (opt->arg[2] == 'f' || opt->arg[2] == '\0'));
				break;
			case 'E':	/* On-boundary selection */
				Ctrl->E.active = true;
				for (j = 0; opt->arg[j]; j++) {
					switch (opt->arg[j]) {
						case 'f': Ctrl->E.inside[F_ITEM] = GMT_INSIDE; break;
						case 'n': Ctrl->E.inside[N_ITEM] = GMT_INSIDE; break;
						default:
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -E option: Expects -Ef, -En, or -Efn\n");
							n_errors++;
							break;
					}
				}
				break;
			case 'F':	/* Inside/outside polygon test */
				if ((Ctrl->F.active = gmt_check_filearg (GMT, 'F', opt->arg, GMT_IN, GMT_IS_DATASET)))
					Ctrl->F.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'G':	/* In-grid selection */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':	/* Invert these tests */
				Ctrl->I.active = true;
				for (j = 0; opt->arg[j]; j++) {
					switch (opt->arg[j]) {
						case 'r': Ctrl->I.pass[GMT_SELECT_R] = false; break;
						case 'c': Ctrl->I.pass[GMT_SELECT_C] = false; break;
						case 'l': Ctrl->I.pass[GMT_SELECT_L] = false; break;
						case 'f': Ctrl->I.pass[GMT_SELECT_F] = false; break;
						case 's': Ctrl->I.pass[GMT_SELECT_N] = false; break;
						case 'z': Ctrl->I.pass[GMT_SELECT_Z] = false; break;
						case 'g': Ctrl->I.pass[GMT_SELECT_G] = false; break;
						default:
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -I option: Expects -Icflrszg\n");
							n_errors++;
							break;
					}
				}
				break;
			case 'L':	/* Near a line test -L<lfile>+d%s[+p]] */
				Ctrl->L.active = true;
				if ((c = strstr (opt->arg, "+d")) == NULL) {	/* Must be old syntax or error */
					n_errors += old_L_parse (API, opt->arg, Ctrl);
					break;
				}
				/* Here we perform new syntax parsing */
				if (gmt_validate_modifiers (GMT, opt->arg, 'L', "dp")) n_errors++;
				if (opt->arg[0] == 0) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L option: No file given\n");
					n_errors++;
				}
				else
					Ctrl->L.file = gmt_get_filename (opt->arg);
				if (gmt_get_modifier (opt->arg, 'd', buffer)) {
					Ctrl->L.mode = gmt_get_distance (GMT, buffer, &(Ctrl->L.dist), &(Ctrl->L.unit));
				}
				if (gmt_get_modifier (opt->arg, 'p', buffer))
					Ctrl->L.end_mode = 10;
				break;
			case 'N':	/* Inside/outside GSHHS land */
				Ctrl->N.active = true;
				strncpy (buffer, opt->arg, GMT_BUFSIZ);
				if (buffer[strlen(buffer)-1] == 'o' && gmt_M_compat_check (GMT, 4)) { /* Edge is considered outside */
					GMT_Report (API, GMT_MSG_COMPAT, "Option -N...o is deprecated; use -E instead\n");
					Ctrl->E.active = true;
					Ctrl->E.inside[N_ITEM] = GMT_INSIDE;
					buffer[strlen(buffer)-1] = 0;
				}
				j = pos = 0;
				while (j < GMTSELECT_N_CLASSES && (gmt_strtok (buffer, "/", &pos, p))) {
					switch (p[0]) {
						case 's':	/* Skip points in this level */
							Ctrl->N.mask[j] = false;
							break;
						case 'k':
							Ctrl->N.mask[j] = true;
							break;
						default:
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: Bad modifier (use s or k)\n");
							n_errors++;
					}
					j++;
				}
				if (!(j == 2 || j == GMTSELECT_N_CLASSES)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: Specify 2 or 5 arguments\n");
					n_errors++;
				}
				Ctrl->N.mode = (j == 2);
				break;
			case 'Z':	/* Test column-ranges */
				Ctrl->Z.active = true;
				invert = false;
				col = z_col++;	/* If no +c<col> given the we march from GMT_Z outwards */
				if ((c = gmt_first_modifier (GMT, opt->arg, "aci"))) {	/* Process any modifiers */
					pos = 0;	/* Reset to start of new word */
					while (gmt_getmodopt (GMT, 'Z', c, "aci", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'a': Ctrl->Z.or = true; break;	/* Logical or: pass if any z-test is true */
							case 'c': col = atoi (&p[1]); break;	/* Set z column # */
							case 'i': invert = true; break;		/* Reverse this test */
							default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';
				}
				j = sscanf (opt->arg, "%[^/]/%s", za, zb);
				if (j < 1) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Z option: Specify z_min [and z_max]\n");
					n_errors++;
				}
				if (Ctrl->Z.n_tests == n_z_alloc) Ctrl->Z.limit = gmt_M_memory (GMT, Ctrl->Z.limit, n_z_alloc += 8, struct GMTSELECT_ZLIMIT);
				Ctrl->Z.limit[Ctrl->Z.n_tests].min = -DBL_MAX;
				Ctrl->Z.limit[Ctrl->Z.n_tests].max = +DBL_MAX;
				Ctrl->Z.limit[Ctrl->Z.n_tests].invert = invert;
				if (!(za[0] == '-' && za[1] == '\0')) {
					ctype = (strchr (za, 'T')) ? GMT_IS_ABSTIME : gmt_M_type (GMT, GMT_IN, col);
					n_errors += gmt_verify_expectations (GMT, ctype, gmt_scanf_arg (GMT, za, ctype, false, &Ctrl->Z.limit[Ctrl->Z.n_tests].min), za);
				}
				if (j == 1)
					Ctrl->Z.limit[Ctrl->Z.n_tests].equal = true;
				else {
					if (!(zb[0] == '-' && zb[1] == '\0')) {
						ctype = (strchr (zb, 'T')) ? GMT_IS_ABSTIME : gmt_M_type (GMT, GMT_IN, col);
						n_errors += gmt_verify_expectations (GMT, ctype, gmt_scanf_arg (GMT, zb, ctype, false, &Ctrl->Z.limit[Ctrl->Z.n_tests].max), zb);
					}
				}
				n_errors += gmt_M_check_condition (GMT, Ctrl->Z.limit[Ctrl->Z.n_tests].max <= Ctrl->Z.limit[Ctrl->Z.n_tests].min, "Syntax error: -Z must have zmax > zmin!\n");
				Ctrl->Z.limit[Ctrl->Z.n_tests].col = col;
				if (col > Ctrl->Z.max_col) Ctrl->Z.max_col = col;
				Ctrl->Z.n_tests++;
				if (c) c[0] = '+';
				break;
#ifdef DEBUG
			case '+':	/* Undocumented option to increase path-fix resolution */
				Ctrl->dbg.active = true;
				Ctrl->dbg.step = atof (opt->arg);
				break;
#endif
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	if (Ctrl->Z.max_col == 1 && (Ctrl->C.active || Ctrl->E.active || Ctrl->F.active || Ctrl->L.active || Ctrl->N.active || GMT->common.R.active[RSET])) Ctrl->Z.max_col = 2;
	if (Ctrl->Z.n_tests) Ctrl->Z.limit = gmt_M_memory (GMT, Ctrl->Z.limit, Ctrl->Z.n_tests, struct GMTSELECT_ZLIMIT);

	n_errors += gmt_M_check_condition (GMT, Ctrl->C.mode == -1, "Syntax error -C: Unrecognized distance unit\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.mode == -2, "Syntax error -C: Unable to decode distance\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.mode == -3, "Syntax error -C: Distance is negative\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && gmt_access (GMT, Ctrl->C.file, R_OK),
	                                   "Syntax error -C: Cannot read file %s!\n", Ctrl->C.file);
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && gmt_access (GMT, Ctrl->F.file, R_OK),
	                                   "Syntax error -F: Cannot read file %s!\n", Ctrl->F.file);
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && gmt_access (GMT, Ctrl->L.file, R_OK),
	                                   "Syntax error -L: Cannot read file %s!\n", Ctrl->L.file);
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.mode == -1, "Syntax error -L: Unrecognized distance unit\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.mode == -2, "Syntax error -L: Unable to decode distance\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.mode == -3, "Syntax error -L: Distance is negative\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->N.active && (Ctrl->A.active || Ctrl->D.active), "Syntax error: -A and -D requires -N!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && Ctrl->C.active && !(Ctrl->C.mode == Ctrl->L.mode && Ctrl->C.unit == Ctrl->L.unit),
	                                   "Syntax error: If both -C and -L are used they must use the same distance unit and calculation mode\n");
	n_errors += gmt_check_binary_io (GMT, Ctrl->Z.max_col);
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.n_tests > 1 && Ctrl->I.active && !Ctrl->I.pass[GMT_SELECT_Z],
	                                   "Syntax error: -Iz can only be used with one -Z range\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtselect (void *V_API, int mode, void *args) {
	int err;	/* Required by gmt_M_err_fail */
	unsigned int base = 3, np[2] = {0, 0};
	unsigned int side, col, id;
	int n_fields, ind, wd[2] = {0, 0}, n_minimum = 2, bin, last_bin = INT_MAX, error = 0;
	bool inside = false, need_header = false, pt_cartesian = false;
	bool output_header = false, do_project = false, no_resample = false, keep;

	uint64_t k, row, seg, n_read = 0, n_pass = 0, n_output = 0;

	double xx, yy;
	double west_border = 0.0, east_border = 0.0, xmin, xmax, ymin, ymax, lon;

	char *shore_resolution[5] = {"full", "high", "intermediate", "low", "crude"};

	struct GMT_DATATABLE *pol = NULL, *line = NULL, *point = NULL;
	struct GMT_GSHHS_POL *p[2] = {NULL, NULL};
	struct GMT_SHORE c;
	struct GMT_DATASET *Cin = NULL, *Lin = NULL, *Fin = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_RECORD *In = NULL;
	struct GMTSELECT_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the gmtselect main code ----------------------------*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");

	if (Ctrl->C.active && gmt_M_is_cartesian (GMT, GMT_IN)) pt_cartesian = true;

	n_minimum = Ctrl->Z.max_col;	/* Minimum number of columns in ASCII input */
	
	if (!GMT->common.R.active[RSET] && Ctrl->N.active) {	/* If we use coastline data or used -fg but didn't give -R we implicitly set -Rg */
		GMT->common.R.active[RSET] = true;
		GMT->common.R.wesn[XLO] = 0.0;	GMT->common.R.wesn[XHI] = 360.0;	GMT->common.R.wesn[YLO] = -90.0;	GMT->common.R.wesn[YHI] = +90.0;
		gmt_set_geographic (GMT, GMT_IN);
	}
	if (GMT->common.R.active[RSET]) {	/* -R was set directly or indirectly; hence must set -J if not supplied */
		if (!GMT->common.J.active) {	/* -J not specified, set one implicitly */
			/* Supply dummy linear proj */
			GMT->current.proj.projection = GMT->current.proj.projection_GMT = GMT->current.proj.xyz_projection[GMT_X] = GMT->current.proj.xyz_projection[GMT_Y] = GMT_LINEAR;
			GMT->current.proj.pars[0] = GMT->current.proj.pars[1] = 1.0;
			GMT->common.J.active = no_resample = true;
		}
		else
			do_project = true;	/* Only true when the USER selected -J, not when we supply a dummy -Jx1d */
		Ctrl->dbg.step = 0.01;
		if (gmt_M_is_geographic (GMT, GMT_IN)) {
			while (GMT->common.R.wesn[XLO] < 0.0 && GMT->common.R.wesn[XHI] < 0.0) {	/* Make all-negative longitude range positive instead */
				GMT->common.R.wesn[XLO] += 360.0;
				GMT->common.R.wesn[XHI] += 360.0;
			}
		}
		if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);
		if (no_resample) GMT->current.map.parallel_straight = GMT->current.map.meridian_straight = 2;	/* No resampling along bin boundaries */
	}

	if (do_project) GMT_Report (API, GMT_MSG_VERBOSE, "-J means all data will be projected before tests are applied\n");
	 
	if (Ctrl->N.active) {	/* Set up GSHHS */
		if (Ctrl->D.force) Ctrl->D.set = gmt_shore_adjust_res (GMT, Ctrl->D.set);
		if (Ctrl->D.active) base = gmt_set_resolution (GMT, &Ctrl->D.set, 'D');
		if (Ctrl->N.mode) {	/* Post-process -N choice */
			Ctrl->N.mask[3] = Ctrl->N.mask[1];
			Ctrl->N.mask[2] = Ctrl->N.mask[4] = Ctrl->N.mask[0];
		}
		if ((err = gmt_init_shore (GMT, Ctrl->D.set, &c, GMT->common.R.wesn, &Ctrl->A.info))) {
			GMT_Report (API, GMT_MSG_NORMAL, "%s [GSHHG %s resolution shorelines]\n", GMT_strerror(err), shore_resolution[base]);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "GSHHG version %s\n%s\n%s\n", c.version, c.title, c.source);
		west_border = floor (GMT->common.R.wesn[XLO] / c.bsize) * c.bsize;
		east_border = ceil (GMT->common.R.wesn[XHI] / c.bsize) * c.bsize;
		wd[0] = 1;	wd[1] = -1;
		np[0] = np[1] = 0;
	}

	/* Initiate pointer to distance calculation function */
	if (gmt_M_is_geographic (GMT, GMT_IN) && !do_project) {	/* Geographic data and no -R -J conversion */
		if (Ctrl->C.active)
			gmt_init_distaz (GMT, Ctrl->C.unit, Ctrl->C.mode, GMT_MAP_DIST);
		else if (Ctrl->L.active)
			gmt_init_distaz (GMT, Ctrl->L.unit, Ctrl->L.mode, GMT_MAP_DIST);
	}
	else if (do_project)	/* Lon/lat projected via -R -J */
		gmt_init_distaz (GMT, 'Z', 0, GMT_MAP_DIST);	/* Compute r-squared instead of r after projection to avoid hypot */
	else	/* Cartesian data */
		gmt_init_distaz (GMT, 'R', 0, GMT_MAP_DIST);	/* Compute r-squared instead of r to avoid hypot  */
	
	gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -C,-F,-L files */

	if (Ctrl->C.active) { 	/* Initialize point structure used in test for proximity to points [use Ctrl->C.dist ]*/
		if ((Cin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IO_ASCII, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (Cin->n_columns < 2) {	/* Trouble */
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: %s does not have at least 2 columns with coordinates\n", Ctrl->C.file);
			Return (GMT_RUNTIME_ERROR);
		}
		if (Ctrl->C.dist == 0.0 && Cin->n_columns <= 2) {	/* Trouble */
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: %s does not have a 3rd column with distances, yet -C0/<file> was given\n", Ctrl->C.file);
			Return (GMT_RUNTIME_ERROR);
		}
		point = Cin->table[0];	/* Can only be one table since we read a single file */
		for (seg = 0; seg < point->n_segments; seg++) {
			SH = gmt_get_DS_hidden (point->segment[seg]);
			if (Cin->n_columns == 2) SH->dist = Ctrl->C.dist;
			if (do_project) {	/* Convert all the points using the map projection */
				for (row = 0; row < point->segment[seg]->n_rows; row++) {
					gmt_geo_to_xy (GMT, point->segment[seg]->data[GMT_X][row], point->segment[seg]->data[GMT_Y][row], &xx, &yy);
					point->segment[seg]->data[GMT_X][row] = xx;
					point->segment[seg]->data[GMT_Y][row] = yy;
				}
				pt_cartesian = true;	/* Well, now it is */
			}
		}
		if (pt_cartesian) {	/* Speed up testing by sorting points on the x-coordinate first */
			struct GMTSELECT_DATA *data = NULL;	/* Used for temporary storage when sorting data on x coordinate */

			/* Copy xp into struct data, sort, and copy back */

			data = gmt_M_memory (GMT, NULL, point->n_records, struct GMTSELECT_DATA);

			for (seg = k = 0; seg < point->n_segments; seg++) {
				for (row = 0; row < point->segment[seg]->n_rows; row++, k++) {
					data[k].x = point->segment[seg]->data[GMT_X][row];
					data[k].y = point->segment[seg]->data[GMT_Y][row];
					data[k].d = (Ctrl->C.dist == 0.0) ? point->segment[seg]->data[GMT_Z][row] : Ctrl->C.dist;
				}
			}
			
			/* Sort on x to speed up inside testing */
			qsort (data, point->n_records, sizeof (struct GMTSELECT_DATA), compare_x);
			
			for (seg = k = 0; seg < point->n_segments; seg++) {	/* Put back the new order */
				for (row = 0; row < point->segment[seg]->n_rows; row++, k++) {
					point->segment[seg]->data[GMT_X][row] = data[k].x;
					point->segment[seg]->data[GMT_Y][row] = data[k].y;
					if (Ctrl->C.dist == 0.0) point->segment[seg]->data[GMT_Z][row] = data[k].d ;
				}
			}
			gmt_M_free (GMT, data);
		}
	}

	if (Ctrl->L.active) {	/* Initialize lines structure used in test for proximity to lines [use Ctrl->L.dist, ] */
		if ((Lin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_IO_ASCII, NULL, Ctrl->L.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (Lin->n_columns < 2) {	/* Trouble */
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L option: %s does not have at least 2 columns with coordinates\n", Ctrl->L.file);
			Return (GMT_RUNTIME_ERROR);
		}
		line = Lin->table[0];	/* Can only be one table since we read a single file */
		for (seg = 0; seg < line->n_segments; seg++) {
			SH = gmt_get_DS_hidden (line->segment[seg]);
			if (Ctrl->L.dist > 0.0) SH->dist = Ctrl->L.dist;	/* Only override when nonzero */
			if (do_project) {	/* Convert all the line points using the map projection */
				for (row = 0; row < line->segment[seg]->n_rows; row++) {
					gmt_geo_to_xy (GMT, line->segment[seg]->data[GMT_X][row], line->segment[seg]->data[GMT_Y][row], &xx, &yy);
					line->segment[seg]->data[GMT_X][row] = xx;
					line->segment[seg]->data[GMT_Y][row] = yy;
				}
			}
		}
	}

	if (Ctrl->F.active) {	/* Initialize polygon structure used in test for polygon in/out test */
		gmt_skip_xy_duplicates (GMT, true);	/* Avoid repeating x/y points in polygons */
		if ((Fin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_IO_ASCII, NULL, Ctrl->F.file, NULL)) == NULL) {
			Return (API->error);
		}
		gmt_skip_xy_duplicates (GMT, false);	/* Reset */
		if (Fin->n_columns < 2) {	/* Trouble */
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: %s does not have at least 2 columns with coordinates\n", Ctrl->F.file);
			Return (GMT_RUNTIME_ERROR);
		}
		pol = Fin->table[0];	/* Can only be one table since we read a single file */
		if (do_project) {	/* Convert all the polygons points using the map projection */
			for (seg = 0; seg < pol->n_segments; seg++) {
				for (row = 0; row < pol->segment[seg]->n_rows; row++) {
					gmt_geo_to_xy (GMT, pol->segment[seg]->data[GMT_X][row], pol->segment[seg]->data[GMT_Y][row], &xx, &yy);
					pol->segment[seg]->data[GMT_X][row] = xx;
					pol->segment[seg]->data[GMT_Y][row] = yy;
				}
			}
			gmt_set_inside_mode (GMT, NULL, GMT_IOO_CARTESIAN);
		}
		else
			gmt_set_inside_mode (GMT, Fin, GMT_IOO_UNKNOWN);
	}

	if (Ctrl->G.active) {	/* Grid mask */
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, NULL)) == NULL) {
			Return (API->error);
		}
	}
	
	gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */

	/* Specify input and output expected columns */
	if ((error = GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_FIX)) != GMT_NOERROR) Return (error);

	/* Gather input/output  file names (or stdin/out) and enable i/o */
	
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}
	if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}

	/* Now we are ready to take on some input values */

	GMT->common.b.ncol[GMT_OUT] = UINT_MAX;	/* Flag to have it reset to GMT->common.b.ncol[GMT_IN] when writing */
	gmt_set_segmentheader (GMT, GMT_OUT, false);	/* Since processing of -C|L|F files might have turned it on [should be determined below] */
	
	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_table_header (GMT)) {	/* Echo table headers */
				GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, NULL);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			else if (gmt_M_rec_is_segment_header (GMT)) {
				output_header = true;
				need_header = GMT->current.io.multi_segments[GMT_OUT];	/* Only need to break up segments */
			}
			continue;							/* Go back and read the next record */
		}
		
		/* Data record to process */

		if (n_output == 0) {
			GMT_Set_Columns (API, GMT_OUT, (unsigned int)gmt_get_cols (GMT, GMT_IN), (In->text) ? GMT_COL_FIX : GMT_COL_FIX_NO_TEXT);
			n_output = gmt_get_cols (GMT, GMT_OUT);
		}

		n_read++;
		if (n_read%1000 == 0) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Read %" PRIu64 " records, passed %" PRIu64 " records\n", n_read, n_pass);

		if (n_fields < n_minimum) {	/* Bad number of columns */
			if (Ctrl->Z.active)
				GMT_Report (API, GMT_MSG_NORMAL, "-Z requires a data file with at least %u columns; this file only has %d near line %" PRIu64 ". Exiting.\n", n_minimum, n_fields, n_read);
			else
				GMT_Report (API, GMT_MSG_NORMAL, "Data file must have at least 2 columns; this file only has %d near line %" PRIu64 ". Exiting.\n", n_fields, n_read);
			Return (GMT_RUNTIME_ERROR);
		}

		if (Ctrl->Z.active) {	/* Apply z-range test(s) */
			for (k = 0, keep = true; keep && k < Ctrl->Z.n_tests; k++) {
				col = Ctrl->Z.limit[k].col;			/* Shorthand notation */
				if (gmt_M_is_dnan (In->data[col]))
					keep = true;		/* Make no decision on NaNs here; see -s instead */
				else if (Ctrl->Z.limit[k].equal) {
					inside = doubleAlmostEqualZero (In->data[col], Ctrl->Z.limit[k].min);
					if (Ctrl->Z.limit[k].invert) inside = !inside;	/* Flip the result for the test below */
				}
				else {
					inside = (In->data[col] >= Ctrl->Z.limit[k].min && In->data[col] <= Ctrl->Z.limit[k].max); 
					if (Ctrl->Z.limit[k].invert) inside = !inside;	/* Flip the result for the test below */
				}
				if (inside != Ctrl->I.pass[GMT_SELECT_Z]) keep = false;
				if (Ctrl->Z.or && keep)
					break;	/* At least one test passed so we are done */
			}
			if (!keep) { output_header = need_header; continue;}
		}

		lon = In->data[GMT_X];	/* Use copy since we may have to wrap 360 */
		if (GMT->common.R.active[RSET]) {	/* Apply region test */
			inside = !gmt_map_outside (GMT, lon, In->data[GMT_Y]);
			if (inside != Ctrl->I.pass[GMT_SELECT_R]) { output_header = need_header; continue;}
		}

		if (do_project)	/* First project the input point */
			gmt_geo_to_xy (GMT, lon, In->data[GMT_Y], &xx, &yy);
		else {
			xx = lon;
			yy = In->data[GMT_Y];
		}
		
		if (Ctrl->C.active) {	/* Check for distance to points */
			inside = gmt_near_a_point (GMT, xx, yy, point, Ctrl->C.dist); 
			if (inside != Ctrl->I.pass[GMT_SELECT_C]) { output_header = need_header; continue;}
		}

		if (Ctrl->L.active) {	/* Check for distance to lines */
			inside = gmt_near_lines (GMT, xx, yy, line, Ctrl->L.end_mode, NULL, NULL, NULL);
			if (inside != Ctrl->I.pass[GMT_SELECT_L]) { output_header = need_header; continue;}
		}

		if (Ctrl->F.active) {	/* Check if we are in/out-side polygons */
			if (do_project)	/* Projected lon/lat; temporary reset input type for gmt_inonout to do Cartesian mode */
				gmt_set_cartesian (GMT, GMT_IN);
			inside = 0;
			for (seg = 0; seg < pol->n_segments && !inside; seg++) {	/* Check each polygon until we find that our point is inside */
				if (gmt_polygon_is_hole (GMT, pol->segment[seg])) continue;	/* Holes are handled within gmt_inonout */
				inside = (gmt_inonout (GMT, xx, yy, pol->segment[seg]) >= Ctrl->E.inside[F_ITEM]);
			}
			if (do_project)	/* Reset input type for gmt_inonout to do Cartesian mode */
				gmt_set_geographic (GMT, GMT_IN);
			if (inside != Ctrl->I.pass[GMT_SELECT_F]) { output_header = need_header; continue;}
		}

		if (Ctrl->G.active) {	/* Check if we are in/out-side mask cell */
			unsigned int row, col;
			if (gmt_M_y_is_outside (GMT, In->data[GMT_Y], G->header->wesn[YLO], G->header->wesn[YHI]) ||	/* Outside y-range */
				gmt_x_is_outside (GMT, &In->data[GMT_X], G->header->wesn[XLO], G->header->wesn[XHI])) {	/* Outside x-range */
				inside = false;	/* Outside both y- and x-range (or periodic longitude) for this grid */
			}
			else
				inside = !gmt_row_col_out_of_bounds (GMT, In->data, G->header, &row, &col);
			if (inside) {	/* Inside grid so check the node value */
				uint64_t node = gmt_M_ijp (G->header, row, col);
				inside = !(gmt_M_is_fnan (G->data[node]) || G->data[node] == 0);
			}
			if (inside != Ctrl->I.pass[GMT_SELECT_G]) { output_header = need_header; continue;}
		}

		if (Ctrl->N.active) {	/* Check if on land or not */
			int brow, i, this_point_level;
			xx = lon;
			while (xx < 0.0) xx += 360.0;
			brow = irint (floor ((90.0 - In->data[GMT_Y]) / c.bsize));
			if (brow >= c.bin_ny) brow = c.bin_ny - 1;	/* Presumably only kicks in for south pole */
			col = urint (floor (xx / c.bsize));
			bin = brow * c.bin_nx + col;
			if (bin != last_bin) {	/* Do this upon entering new bin */
				ind = 0;
				while (ind < c.nb && c.bins[ind] != bin) ind++;	/* Set ind to right bin */
				if (ind == c.nb) continue;			/* Bin not among the chosen ones */
				last_bin = bin;
				gmt_free_shore (GMT, &c);	/* Free previously allocated arrays */
				if ((err = gmt_get_shore_bin (GMT, ind, &c))) {
					GMT_Report (API, GMT_MSG_NORMAL, "%s [%s resolution shoreline]\n", GMT_strerror(err), shore_resolution[base]);
					Return (GMT_RUNTIME_ERROR);
				}

				/* Must use polygons.  Go in both directions to cover both land and sea */
				for (id = 0; id < 2; id++) {
					gmt_free_shore_polygons (GMT, p[id], np[id]);
					if (np[id]) gmt_M_free (GMT, p[id]);
					np[id] = gmt_assemble_shore (GMT, &c, wd[id], true, west_border, east_border, &p[id]);
					np[id] = gmt_prep_shore_polygons (GMT, &p[id], np[id], !no_resample, Ctrl->dbg.step, -1);
				}
			}

			if (c.ns == 0) {	/* No lines go through, check node level */
				this_point_level = MIN (MIN (c.node_level[0], c.node_level[1]) , MIN (c.node_level[2], c.node_level[3]));
			}
			else {
				this_point_level = 0;
				gmt_geo_to_xy (GMT, lon, In->data[GMT_Y], &xx, &yy);
				for (id = 0; id < 2; id++) {

					for (k = 0; k < np[id]; k++) {

						if (p[id][k].n == 0) continue;

						/* Find min/max of polygon */

						xmin = xmax = p[id][k].lon[0];
						ymin = ymax = p[id][k].lat[0];

						for (i = 1; i < p[id][k].n; i++) {
							if (p[id][k].lon[i] < xmin) xmin = p[id][k].lon[i];
							if (p[id][k].lon[i] > xmax) xmax = p[id][k].lon[i];
							if (p[id][k].lat[i] < ymin) ymin = p[id][k].lat[i];
							if (p[id][k].lat[i] > ymax) ymax = p[id][k].lat[i];
						}

						if (yy < ymin || yy > ymax) continue;
						if (xx < xmin || xx > xmax) continue;

						/* Must compare with polygon; holes are handled explicitly via the levels */

						if ((side = gmt_non_zero_winding (GMT, xx, yy, p[id][k].lon, p[id][k].lat, p[id][k].n)) < Ctrl->E.inside[N_ITEM]) continue;	/* Outside polygon */

						/* Here, point is inside, we must assign value */

						if (p[id][k].level > this_point_level) this_point_level = p[id][k].level;
					}
				}
			}
			inside = Ctrl->N.mask[this_point_level];
			if (inside != Ctrl->I.pass[GMT_SELECT_N]) { output_header = need_header; continue;}
		}

		/* Here, we have passed all active test and the point is to be output */

		if (output_header) {	/* First output point for this segment - write the header */
			GMT_Put_Record (API, GMT_WRITE_SEGMENT_HEADER, NULL);
			output_header = false;
		}

		GMT_Put_Record (API, GMT_WRITE_DATA, In);
		n_pass++;
	} while (true);
	
	if (GMT_End_IO (API, GMT_IN,  0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Read %" PRIu64 " records, passed %" PRIu64" records\n", n_read, n_pass);

	if (Ctrl->N.active) {
		gmt_free_shore (GMT, &c);
		gmt_shore_cleanup (GMT, &c);
		for (id = 0; id < 2; id++) {
			gmt_free_shore_polygons (GMT, p[id], np[id]);
			if (np[id]) gmt_M_free (GMT, p[id]);
		}
	}
	
	Return (GMT_NOERROR);
}
