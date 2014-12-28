/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#define GMT_SHORTHAND_OPTIONS	"BJRXYcp"	/* All of the shorthand options */
#define GMT_CRITICAL_OPT_ORDER "-VJfRb"		/* If given options among these must be parsed first and in this order */

struct GMT_COMMON {
	/* Structure with all information given via the common GMT command-line options -R -J .. */
	struct synopsis {	/* \0 (zero) or ^ */
		bool active;
		bool extended;	/* + to also show non-common options */
	} synopsis;
	struct B {	/* -B<params> */
		bool active[2];	/* 0 = primary annotation, 1 = secondary annotations */
		int mode;	/* 5 = GMT 5 syntax, 4 = GMT 4 syntax, 1 = Either, -1 = mix (error), 0 = not set yet */
		char string[2][GMT_LEN256];
	} B;	
	struct API_I {	/* -I<xinc>[/<yinc>] grids only, and for API use only */
		bool active;
		double inc[2];
	} API_I;	
	struct J {	/* -J<params> */
		bool active, zactive;
		unsigned int id;
		double par[6];
		char string[GMT_LEN256];
	} J;		
	struct K {	/* -K */
		bool active;
	} K;	
	struct O {	/* -O */
		bool active;
	} O;
	struct P {	/* -P */
		bool active;
	} P;
	struct R {	/* -Rw/e/s/n[/z_min/z_max][r] */
		bool active;
		bool oblique;	/* true when -R...r was given (oblique map, probably), else false (map borders are meridians/parallels) */
		double wesn[6];		/* Boundaries of west, east, south, north, low-z and hi-z */
		double wesn_orig[4];	/* Original Boundaries of west, east, south, north (oblique projection may reset wesn above) */
		char string[GMT_LEN256];
	} R;
	struct U {	/* -U */
		bool active;
		unsigned int just;
		double x, y;
		char *label;		/* Content not counted by sizeof (struct) */
	} U;
	struct V {	/* -V */
		bool active;
	} V;
	struct X {	/* -X */
		bool active;
		double off;
		char mode;	/* r, a, or c */
	} X;
	struct Y {	/* -Y */
		bool active;
		double off;
		char mode;	/* r, a, or c */
	} Y;
	struct a {	/* -a<col>=<name>[:<type>][,col>=<name>[:<type>], etc][+g<geometry>] */
		bool active;
		unsigned int geometry;
		unsigned int n_aspatial;
		bool clip;		/* true if we wish to clip lines/polygons at Dateline [false] */
		bool output;		/* true when we wish to build OGR output */
		int col[MAX_ASPATIAL];	/* Col id, include negative items such as GMT_IS_T (-5) */
		int ogr[MAX_ASPATIAL];	/* Column order, or -1 if not set */
		unsigned int type[MAX_ASPATIAL];
		char *name[MAX_ASPATIAL];
	} a;
	struct b {	/* -b[i][o][s|S][d|D][#cols][cvar1/var2/...] */
		bool active[2];		/* true if current input/output is in native binary format */
		bool o_delay;		/* true if we dont know number of output columns until we have read at least one input record */
		enum GMT_swap_direction swab[2];	/* k_swap_in or k_swap_out if current binary input/output must be byte-swapped, else k_swap_none */
		uint64_t ncol[2];		/* Number of expected columns of input/output
						   0 means it will be determined by program */
		char type[2];			/* Default column type, if set [d for double] */
		char varnames[GMT_BUFSIZ];	/* List of variable names to be input/output in netCDF mode [GMT4 COMPATIBILITY ONLY] */
	} b;
	struct c {	/* -c */
		bool active;
		unsigned int copies;
	} c;
	struct f {	/* -f[i|o]<col>|<colrange>[t|T|g],.. */
		bool active[2];	/* For GMT_IN|OUT */
	} f;
	struct g {	/* -g[+]x|x|y|Y|d|Y<gap>[unit]  */
		bool active;
		unsigned int n_methods;			/* How many different criteria to apply */
		uint64_t n_col;				/* Largest column-number needed to be read */
		bool match_all;			/* If true then all specified criteria must be met to be a gap [default is any of them] */
		enum GMT_enum_gaps method[GMT_N_GAP_METHODS];	/* How distances are computed for each criteria */
		uint64_t col[GMT_N_GAP_METHODS];	/* Which column to use (-1 for x,y distance) */
		double gap[GMT_N_GAP_METHODS];		/* The critical distances for each criteria */
		double (*get_dist[GMT_N_GAP_METHODS]) (struct GMT_CTRL *GMT, uint64_t);	/* Pointers to functions that compute those distances */
	} g;
	struct h {	/* -h[i|o][<nrecs>][+d][+c][+r<remark>][+t<title>] */
		bool active;
		bool add_colnames;
		unsigned int mode;
		unsigned int n_recs;
		char *title;
		char *remark;
		char *colnames;	/* Not set by -h but maintained here */
	} h;	
	struct i {	/* -i<col>|<colrange>,.. */
		bool active;
		uint64_t n_cols;
	} i;
	struct n {	/* -n[b|c|l|n][+a][+b<BC>][+c][+t<threshold>] */
		bool active;
		bool antialias;	/* Defaults to true, if supported */
		bool truncate;	/* Defaults to false */
		unsigned int interpolant;	/* Defaults to BCR_BICUBIC */
		bool bc_set;	/* true if +b was parsed */
		char BC[4];		/* For BC settings via +bg|n[x|y]|p[x|y] */
		double threshold;	/* Defaults to 0.5 */
	} n;
	struct o {	/* -o<col>|<colrange>,.. */
		bool active;
		uint64_t n_cols;
	} o;
	struct p {	/* -p<az>/<el>[+wlon0/lat0[/z0]][+vx0[cip]/y0[cip]] */
		bool active;
	} p;
	struct r {	/* -r */
		bool active;
		unsigned int registration;
	} r;
	struct s {	/* -s[r] */
		bool active;
	} s;
	struct t {	/* -t<transparency> */
		bool active;
		double value;
	} t;
	struct colon {	/* -:[i|o] */
		bool active;
		bool toggle[2];
	} colon;
};

#endif /* _GMT_COMMON_H */
