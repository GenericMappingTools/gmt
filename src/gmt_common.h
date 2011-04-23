/*--------------------------------------------------------------------
 *	$Id: gmt_common.h,v 1.25 2011-04-23 02:14:12 guru Exp $
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
 * Holds current selections for the family of common GMT options.
 *
 * Author: 	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	5 API
 */
 
#ifndef _GMT_COMMON_H
#define _GMT_COMMON_H

/* Macros related to detecting data gaps which should be treated as segment boundaries */
#define GMT_N_GAP_METHODS	10

#define GMT_NEGGAP_IN_COL	1
#define GMT_POSGAP_IN_COL	2
#define GMT_ABSGAP_IN_COL	3
#define GMT_NEGGAP_IN_MAP_COL	4
#define GMT_POSGAP_IN_MAP_COL	5
#define GMT_ABSGAP_IN_MAP_COL	6
#define GMT_GAP_IN_GDIST	7
#define GMT_GAP_IN_CDIST	8
#define GMT_GAP_IN_PDIST	9
#define GMT_GAP_IN_DDIST	10

#define MAX_ASPATIAL 64		/* No more than 64 aspatial options in -a */

struct GMT_COMMON {
	/* Structure with all information given via the common GMT command-line options -R -J .. */
	struct synopsis {	/* \0 (zero) or ^ */
		GMT_LONG active;
	} synopsis;
	struct B {	/* -B<params> */
		GMT_LONG active[2];	/* 0 = primary annotation, 1 = secondary annotations */
		GMT_LONG set;		/* TRUE if other than -B or -B:... or -B/... */
	} B;	
	struct J {	/* -J<params> */
		GMT_LONG active;
		GMT_LONG id;
		double par[6];
	} J;		
	struct K {	/* -K */
		GMT_LONG active;
	} K;	
	struct O {	/* -O */
		GMT_LONG active;
	} O;
	struct P {	/* -P */
		GMT_LONG active;
	} P;
	struct R {	/* -Rw/e/s/n[/z_min/z_max][r] */
		GMT_LONG active;
		GMT_LONG oblique;	/* TRUE when -R...r was given (oblique map, probably), else FALSE (map borders are meridians/parallels) */
		double wesn[6];		/* Boundaries of west, east, south, north, low-z and hi-z */
	} R;
	struct U {	/* -U */
		GMT_LONG active;
		GMT_LONG just;
		double x, y;
		char *label;		/* Content not counted by sizeof (struct) */
	} U;
	struct V {	/* -V */
		GMT_LONG active;
	} V;
	struct X {	/* -X */
		GMT_LONG active;
		double off;
		char mode;	/* r, a, or c */
	} X;
	struct Y {	/* -Y */
		GMT_LONG active;
		double off;
		char mode;	/* r, a, or c */
	} Y;
	struct a {	/* -a<col>=<name>[:<type>][,col>=<name>[:<type>], etc][+g<geometry>] */
		GMT_LONG active;
		GMT_LONG geometry;
		GMT_LONG n_aspatial;
		GMT_LONG clip;		/* TRUE if we wish to clip lines/polygons at Dateline [FALSE] */
		GMT_LONG output;	/* TRUE when we wish to build OGR output */
		GMT_LONG col[MAX_ASPATIAL];
		GMT_LONG ogr[MAX_ASPATIAL];
		GMT_LONG type[MAX_ASPATIAL];
		char *name[MAX_ASPATIAL];
	} a;
	struct b {	/* -b[i][o][s|S][d|D][#cols][cvar1/var2/...] */
		GMT_LONG active[2];		/* TRUE if current input/output is in native binary format */
		GMT_LONG netcdf[2];		/* TRUE if current input/output is in netCDF format */
		GMT_LONG single_precision[2];	/* TRUE if current binary input/output is in single precision [double] */
		GMT_LONG swab[2];		/* TRUE if current binary input/output must be byte-swapped */
		GMT_LONG ncol[2];		/* Number of expected columns of input/output
						   0 means it will be determined by program */
		char varnames[BUFSIZ];		/* List of variable names to be input/output in netCDF mode */
	} b;
	struct c {	/* -c */
		GMT_LONG active;
		GMT_LONG copies;
	} c;
	struct f {	/* -f[i|o]<col>|<colrange>[t|T|g],.. */
		GMT_LONG active[2];	/* For GMT_IN|OUT */
		char col_type[2][GMT_MAX_COLUMNS];
	} f;
	struct g {	/* -g[+]x|x|y|Y|d|Y<gap>[unit]  */
		GMT_LONG active;
		GMT_LONG n_methods;			/* How many different criteria to apply */
		GMT_LONG n_col;				/* Largest column-number needed to be read */
		GMT_LONG match_all;			/* If TRUE then all specified criteria must be met to be a gap [default is any of them] */
		GMT_LONG method[GMT_N_GAP_METHODS];	/* How distances are computed for each criteria */
		GMT_LONG col[GMT_N_GAP_METHODS];	/* Which column to use (-1 for x,y distance) */
		double gap[GMT_N_GAP_METHODS];		/* The critical distances for each criteria */
		PFD get_dist[GMT_N_GAP_METHODS];	/* Pointers to functiosn that compute those distances */
	} g;
	struct h {	/* -h[i][<nrecs>] */
		GMT_LONG active;
		GMT_LONG n_recs;
	} h;	
	struct i {	/* -i<col>|<colrange>,.. */
		GMT_LONG active;
		GMT_LONG n_cols;
	} i;
	struct o {	/* -o<col>|<colrange>,.. */
		GMT_LONG active;
		GMT_LONG n_cols;
	} o;
	struct p {	/* -p<az>/<el>[+wlon0/lat0[/z0]][+vx0[cip]/y0[cip]] */
		GMT_LONG active;
	} p;
	struct r {	/* -r */
		GMT_LONG active;
	} r;
	struct s {	/* -s[r] */
		GMT_LONG active;
	} s;
	struct t {	/* -t<transparency> */
		GMT_LONG active;
		double value;
	} t;
	struct colon {	/* -:[i|o] */
		GMT_LONG active;
		GMT_LONG toggle[2];
	} colon;
};

#endif /* _GMT_COMMON_H */
