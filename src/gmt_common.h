/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Holds current selections for the family of common GMT options.
 *
 * Author: 	Paul Wessel
 * Date:	01-JAN-2011
 * Version:	6 API
 */

/*!
 * \file gmt_common.h
 * \brief Holds current selections for the family of common GMT options
 */

#ifndef GMT_COMMON_H
#define GMT_COMMON_H

/*! Constants related to detecting data gaps which should be treated as segment boundaries */
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
#define GMT_CRITICAL_OPT_ORDER	"V-JfrRbi"	/* If given options among these must be parsed first and in this order */

#define RSET	0	/* Index into R.active[] for -R */
#define ISET	1	/* Index into R.active[] for -I (or similar option) */
#define GSET	2	/* Index into R.active[] for -r */
#define FSET	3	/* Index into R.active[] for "got -R -I -r from a grid file" */

struct GMT_LEGEND_ITEM {	/* Information about one item in a legend */
	char label[GMT_LEN128];		/* The symbol label */
	char header[GMT_LEN128];	/* Header for the whole legend H */
	char subheader[GMT_LEN128];	/* Subheader, i.e., line label L*/
	char font[GMT_LEN32];		/* Fontsize to use for current H or L */
	char gap[GMT_LEN32];		/* Move this much down before placing symbol entry */
	char pen[2][GMT_LEN32];		/* Pens to use with +d and +v */
	int draw;			/* 0 no draw, 1 draw horizontal +d, 2 draw vertical +v */
	int just;			/* Legend placement [TR] */
	char code;			/* Label justification code (L|C|R) [L] */
	double size;			/* Fixed symbol size when otherwise cannot set it */
	double scale;			/* Scale all given sizes, including +s<length> of a line */
	double width;			/* Override auto-width with a fixed legend width */
	unsigned int ncols;		/* How many columns to use for symbols */
};

/*! Structure with all information given via the common GMT command-line options -R -J .. */
struct GMT_COMMON {
	struct synopsis {	/* \0 (zero) or ^ */
		bool active;
		bool extended;	/* + to also show non-common options */
	} synopsis;
	struct B {	/* -B<params> */
		bool active[2];	/* 0 = primary annotation, 1 = secondary annotations */
		int mode;	/* 5 = GMT 5 syntax, 4 = GMT 4 syntax, 1 = Either, -1 = mix (error), 0 = not set yet */
		char string[2][GMT_LEN256];
	} B;
	struct J {	/* -J<params> */
		bool active, zactive;
		unsigned int id;
		char string[GMT_LEN128];
		char zstring[GMT_LEN128];	/* For -Jz|Z */
		char proj4string[GMT_LEN256];
		char WKTstring[GMT_LEN1024];
		double par[6];
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
	struct R {	/* -Rw/e/s/n[/z_min/z_max][r] or -Rgridfile */
		bool active[4];	/* RSET = 0: -R, ISET = 1: inc, GSET = 2: -r, FSET = 3: read grid */
		bool oblique;	/* true when -R...r was given (oblique map, probably), else false (map borders are meridians/parallels) */
		uint32_t registration;	/* Registration mode of a grid given via -r or -Rgrid */
		int row_order;	/* Order of rows in NetCDF output: 0 (not set) or k_nc_start_north or k_nc_start_south */
		unsigned int mode;	/* For modern mode only: 0 = get exact region from data, 1 = rounded region from data */
		double wesn[6];		/* Boundaries of west, east, south, north, low-z and hi-z */
		double wesn_orig[4];	/* Original Boundaries of west, east, south, north (oblique projection may reset wesn above) */
		double inc[2];	/* For grid increments set via -Idx/dy or implicitly via -Ggrid */
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
		char mode;	/* a, c, f, or r */
	} X;
	struct Y {	/* -Y */
		bool active;
		double off;
		char mode;	/* a, c, f, or r */
	} Y;
	struct a {	/* -a<col>=<name>[:<type>][,col>=<name>[:<type>], etc][+g<geometry>] */
		bool active;
		enum GMT_enum_ogr geometry;
		unsigned int n_aspatial;
		bool clip;		/* true if we wish to clip lines/polygons at Dateline [false] */
		bool output;		/* true when we wish to build OGR output */
		int col[MAX_ASPATIAL];	/* Col id, include negative items such as GMT_IS_T (-5) */
		int ogr[MAX_ASPATIAL];	/* Column order, or -1 if not set */
		enum GMT_enum_type type[MAX_ASPATIAL];
		char *name[MAX_ASPATIAL];
		char string[GMT_LEN256];
	} a;
	struct b {	/* -b[i][o][s|S][d|D][#cols][cvar1/var2/...] */
		bool active[2];		/* true if current input/output is in native binary format */
		bool nc[2];		/* True if netcdf i/o */
		bool o_delay;		/* true if we don't know number of output columns until we have read at least one input record */
		bool bin_primary;	/* true if we need to switch back to binary after reading a secondary file in ascii */
		enum GMT_swap_direction swab[2];	/* k_swap_in or k_swap_out if current binary input/output must be byte-swapped, else k_swap_none */
		uint64_t ncol[2];		/* Number of expected columns of input/output
						   0 means it will be determined by program */
		char type[2];			/* Default column type, if set [d for double] */
		char varnames[GMT_BUFSIZ];	/* List of variable names to be input/output in netCDF mode [GMT4 COMPATIBILITY ONLY] */
		char string[GMT_LEN256];
	} b;
	struct d {	/* -d[i][o]<nan_proxy> */
		bool active[2];
		bool is_zero[2];
		double nan_proxy[2];
		char string[GMT_LEN64];
	} d;
	struct e {	/* -e[~]\"search string\"] */
		bool active;
		char string[GMT_LEN256];
		struct GMT_TEXT_SELECTION *select;
	} e;
	struct f {	/* -f[i|o]<col>|<colrange>[t|T|g],.. */
		bool active[2];	/* For GMT_IN|OUT */
		char string[GMT_LEN64];
	} f;
	struct g {	/* -g[+]x|x|y|Y|d|Y<gap>[unit]  */
		bool active;
		unsigned int n_methods;			/* How many different criteria to apply */
		uint64_t n_col;				/* Largest column-number needed to be read */
		bool match_all;				/* If true then all specified criteria must be met to be a gap [default is any of them] */
		enum GMT_enum_gaps method[GMT_N_GAP_METHODS];	/* How distances are computed for each criteria */
		int64_t col[GMT_N_GAP_METHODS];		/* Which column to use (-1 for x,y distance) */
		double gap[GMT_N_GAP_METHODS];		/* The critical distances for each criteria */
		double (*get_dist[GMT_N_GAP_METHODS]) (struct GMT_CTRL *GMT, uint64_t);	/* Pointers to functions that compute those distances */
		char string[GMT_LEN64];
	} g;
	struct h {	/* -h[i|o][<nrecs>][+d][+c][+m[<text>]][+r<remark>][+t<title>] */
		bool active;
		bool add_colnames;
		unsigned int mode;
		unsigned int n_recs;
		char *title;
		char *remark;
		char *colnames;         /* Not set by -h but maintained here */
		char *multi_segment;    /* To hold a multi-segment string */
		char string[GMT_LEN256];
	} h;
	struct i {	/* -i[<col>|<colrange>,...][t[<word>]] */
		bool active, select, orig, word;
		uint64_t n_cols, w_col;
		uint64_t n_actual_cols;
		char string[GMT_LEN64];
	} i;
	struct j {	/* -je|f|g [g] */
		bool active;
		enum GMT_enum_mdist mode;	/* Defaults to GMT_GREATCIRCLE */
		char string[GMT_LEN8];
	} j;
	struct l {	/* -l[<label>][+s<size>][+t<title>][+n<ncols>][+d<gap>/<pen>] */
		bool active;
		struct GMT_LEGEND_ITEM item;
	} l;
	struct n {	/* -n[b|c|l|n][+a][+b<BC>][+c][+t<threshold>] */
		bool active;
		bool antialias;		/* Defaults to true, if supported */
		bool truncate;		/* Defaults to false */
		unsigned int interpolant;	/* Defaults to BCR_BICUBIC */
		bool bc_set;		/* true if +b was parsed */
		bool periodic[2];	/* For periodic non-geographic grids */
		char BC[4];		/* For BC settings via +bg|n[x|y]|p[x|y] */
		double threshold;	/* Defaults to 0.5 */
		double range[2], half_range[2];	/* For periodic non-geographic grids */
		char string[GMT_LEN64];	/* Copy of argument */
	} n;
	struct o {	/* -o[<col>|<colrange>,...][t[<word>]] */
		bool active, select, orig, word;
		uint64_t n_cols, w_col;
	} o;
	struct p {	/* -p<az>[/<el>[/<z0>]]+wlon0/lat0[/z0]][+vx0[cip]/y0[cip]] */
		bool active;
		bool do_z_rotation;	/* true if rotating plot about a vertical axis */
		double z_rotation;	/* Rotation of <angle> about vertical axis */
	} p;
	struct s {	/* -s[r] */
		bool active;
		char string[GMT_LEN64];
	} s;
	struct t {	/* -t[<transparency>] */
		bool active;
		bool variable;
		double value;
	} t;
	struct x {	/* -x[[-]<n>] */
		bool active;
		int n_threads;
	} x;
	struct colon {	/* -:[i|o] */
		bool active;
		bool toggle[2];
		char string[2][GMT_LEN16];
	} colon;
};

#endif /* GMT_COMMON_H */
