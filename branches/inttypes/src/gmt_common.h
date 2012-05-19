/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Holds current selections for the family of common GMT options.
 *
 * Author: 	Paul Wessel
 * Date:	01-JAN-2011
 * Version:	5 API
 */
 
#ifndef _GMT_COMMON_H
#define _GMT_COMMON_H

/* Constants related to detecting data gaps which should be treated as segment boundaries */
enum GMT_enum_gaps {GMT_NEGGAP_IN_COL = 0,	/* Check if previous minus current column value exceeds <gap> */
	GMT_POSGAP_IN_COL,			/* Check if current minus previous column value exceeds <gap> */
	GMT_ABSGAP_IN_COL,			/* Check if |current minus previous column value| exceeds <gap> */
	GMT_NEGGAP_IN_MAP_COL,			/* Check if previous minus current column value exceeds <gap> after map projection */
	GMT_POSGAP_IN_MAP_COL,			/* Check if current minus previous column value exceeds <gap> after map projection */
	GMT_ABSGAP_IN_MAP_COL,			/* Check if |current minus previous column value| exceeds <gap> after map projection */
	GMT_GAP_IN_GDIST,			/* Check if great-circle distance between successive points exceeds <gap> (in km,m,nm, etc)*/
	GMT_GAP_IN_CDIST,			/* Check if Cartesian distance between successive points exceeds <gap> */
	GMT_GAP_IN_PDIST,			/* Check if Cartesian distance between successive points exceeds <gap> after map projection */
	GMT_GAP_IN_DDIST,			/* Check if great-circle distance between successive points exceeds <gap> (in arc degrees,min,sec) */
	GMT_N_GAP_METHODS};

#define MAX_ASPATIAL 64		/* No more than 64 aspatial options in -a */

#define GMT_SHORTHAND_OPTIONS	"BJRXxYycp"		/* All of the shorthand options */

struct GMT_COMMON {
	/* Structure with all information given via the common GMT command-line options -R -J .. */
	struct synopsis {	/* \0 (zero) or ^ */
		GMT_BOOLEAN active;
	} synopsis;
	struct B {	/* -B<params> */
		GMT_BOOLEAN active[2];	/* 0 = primary annotation, 1 = secondary annotations */
	} B;	
	struct J {	/* -J<params> */
		GMT_BOOLEAN active, zactive;
		COUNTER_MEDIUM id;
		double par[6];
	} J;		
	struct K {	/* -K */
		GMT_BOOLEAN active;
	} K;	
	struct O {	/* -O */
		GMT_BOOLEAN active;
	} O;
	struct P {	/* -P */
		GMT_BOOLEAN active;
	} P;
	struct R {	/* -Rw/e/s/n[/z_min/z_max][r] */
		GMT_BOOLEAN active;
		GMT_BOOLEAN oblique;	/* TRUE when -R...r was given (oblique map, probably), else FALSE (map borders are meridians/parallels) */
		double wesn[6];		/* Boundaries of west, east, south, north, low-z and hi-z */
		char string[GMT_TEXT_LEN256];
	} R;
	struct U {	/* -U */
		GMT_BOOLEAN active;
		COUNTER_MEDIUM just;
		double x, y;
		char *label;		/* Content not counted by sizeof (struct) */
	} U;
	struct V {	/* -V */
		GMT_BOOLEAN active;
	} V;
	struct X {	/* -X */
		GMT_BOOLEAN active;
		double off;
		char mode;	/* r, a, or c */
	} X;
	struct Y {	/* -Y */
		GMT_BOOLEAN active;
		double off;
		char mode;	/* r, a, or c */
	} Y;
	struct a {	/* -a<col>=<name>[:<type>][,col>=<name>[:<type>], etc][+g<geometry>] */
		GMT_BOOLEAN active;
		COUNTER_MEDIUM geometry;
		COUNTER_MEDIUM n_aspatial;
		GMT_BOOLEAN clip;		/* TRUE if we wish to clip lines/polygons at Dateline [FALSE] */
		GMT_BOOLEAN output;		/* TRUE when we wish to build OGR output */
		GMT_LONG col[MAX_ASPATIAL];
		GMT_LONG ogr[MAX_ASPATIAL];
		COUNTER_MEDIUM type[MAX_ASPATIAL];
		char *name[MAX_ASPATIAL];
	} a;
	struct b {	/* -b[i][o][s|S][d|D][#cols][cvar1/var2/...] */
		GMT_BOOLEAN active[2];		/* TRUE if current input/output is in native binary format */
		GMT_BOOLEAN swab[2];		/* TRUE if current binary input/output must be byte-swapped */
		COUNTER_MEDIUM ncol[2];		/* Number of expected columns of input/output
						   0 means it will be determined by program */
		char type[2];			/* Default column type, if set [d for double] */
#ifdef GMT_COMPAT
		char varnames[GMT_BUFSIZ];	/* List of variable names to be input/output in netCDF mode */
#endif
	} b;
	struct c {	/* -c */
		GMT_BOOLEAN active;
		COUNTER_MEDIUM copies;
	} c;
	struct f {	/* -f[i|o]<col>|<colrange>[t|T|g],.. */
		GMT_BOOLEAN active[2];	/* For GMT_IN|OUT */
		char col_type[2][GMT_MAX_COLUMNS];
	} f;
	struct g {	/* -g[+]x|x|y|Y|d|Y<gap>[unit]  */
		GMT_BOOLEAN active;
		COUNTER_MEDIUM n_methods;			/* How many different criteria to apply */
		COUNTER_MEDIUM n_col;				/* Largest column-number needed to be read */
		GMT_BOOLEAN match_all;			/* If TRUE then all specified criteria must be met to be a gap [default is any of them] */
		enum GMT_enum_gaps method[GMT_N_GAP_METHODS];	/* How distances are computed for each criteria */
		GMT_LONG col[GMT_N_GAP_METHODS];	/* Which column to use (-1 for x,y distance) */
		double gap[GMT_N_GAP_METHODS];		/* The critical distances for each criteria */
		p_func_d get_dist[GMT_N_GAP_METHODS];	/* Pointers to functiosn that compute those distances */
	} g;
	struct h {	/* -h[i][<nrecs>] */
		GMT_BOOLEAN active;
		COUNTER_MEDIUM n_recs;
	} h;	
	struct i {	/* -i<col>|<colrange>,.. */
		GMT_BOOLEAN active;
		COUNTER_MEDIUM n_cols;
	} i;
	struct n {	/* -n[b|c|l|n][+a][+b<BC>][+t<threshold>] */
		GMT_BOOLEAN active;
		GMT_BOOLEAN antialias;	/* Defaults to TRUE, if supported */
		COUNTER_MEDIUM interpolant;	/* Defaults to BCR_BICUBIC */
		GMT_BOOLEAN bc_set;	/* TRUE if +b was parsed */
		char BC[4];		/* For BC settings via +bg|n[x|y]|p[x|y] */
		double threshold;	/* Defaults to 0.5 */
	} n;
	struct o {	/* -o<col>|<colrange>,.. */
		GMT_BOOLEAN active;
		COUNTER_MEDIUM n_cols;
	} o;
	struct p {	/* -p<az>/<el>[+wlon0/lat0[/z0]][+vx0[cip]/y0[cip]] */
		GMT_BOOLEAN active;
	} p;
	struct r {	/* -r */
		GMT_BOOLEAN active;
	} r;
	struct s {	/* -s[r] */
		GMT_BOOLEAN active;
	} s;
	struct t {	/* -t<transparency> */
		GMT_BOOLEAN active;
		double value;
	} t;
	struct colon {	/* -:[i|o] */
		GMT_BOOLEAN active;
		GMT_BOOLEAN toggle[2];
	} colon;
};

#endif /* _GMT_COMMON_H */
