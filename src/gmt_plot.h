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

/*!
 * \file gmt_plot.h
 * \brief 
 */

#ifndef GMT_PLOT_H
#define GMT_PLOT_H

/*! Identifier for gmt_plane_perspective. The others come from GMT_io.h */

#define GMT_ZW	3

/* GMT symbol identifiers. In addition to those in PSL we have
   extensions for custom symbols in psxy and psxyz and 3-D symbols
   as well as special line symbols (fronts, decorated or quoted lines) */

#define GMT_SYMBOL_BARX			((int)'B')
#define GMT_SYMBOL_BARY			((int)'b')
#define GMT_SYMBOL_FRONT		((int)'f')
#define GMT_SYMBOL_CUSTOM		((int)'k')
#define GMT_SYMBOL_TEXT			((int)'l')
#define GMT_SYMBOL_COLUMN		((int)'o')
#define GMT_SYMBOL_QUOTED_LINE		((int)'q')
#define GMT_SYMBOL_CUBE			((int)'u')
#define GMT_SYMBOL_ZDASH		((int)'z')
#define GMT_SYMBOL_DECORATED_LINE	((int)'~')
#define GMT_SYMBOL_VECTOR_V4		999

#define GMT_SYMBOL_MOVE		((int)'M')
#define GMT_SYMBOL_DRAW		((int)'D')
#define GMT_SYMBOL_STROKE	((int)'S')
#define GMT_SYMBOL_ARC		((int)'A')
#define GMT_SYMBOL_ROTATE	((int)'R')
#define GMT_SYMBOL_VARROTATE	((int)'V')
#define GMT_SYMBOL_AZIMROTATE	((int)'Z')
#define GMT_SYMBOL_TEXTURE	((int)'T')
#define GMT_SYMBOL_GEOVECTOR	((int)'=')
#define GMT_SYMBOL_VARTEXT	((int)'L')

#define GMT_SYMBOL_LINE		0
#define GMT_SYMBOL_NONE		((int)' ')
#define GMT_SYMBOL_NOT_SET	((int)'*')

#define GMT_DOT_SIZE 0.005	/* Size of a "dot" on a GMT PS map [in inches] */

/*! FRONT symbols */

enum GMT_enum_front {GMT_FRONT_FAULT = 0,
	GMT_FRONT_TRIANGLE,
	GMT_FRONT_SLIP,
	GMT_FRONT_SLIPC,
	GMT_FRONT_CIRCLE,
	GMT_FRONT_BOX};

/*! Direction of FRONT symbols: */

enum GMT_enum_frontdir {GMT_FRONT_RIGHT = -1,
	GMT_FRONT_CENTERED,
	GMT_FRONT_LEFT};

/*! Type of wedge symbols: */

enum GMT_enum_wedgetype {GMT_WEDGE_NORMAL = 0,
	GMT_WEDGE_ARCS = 1,
	GMT_WEDGE_RADII = 2,
	GMT_WEDGE_SPIDER = 3};

/*! A sub-symbol for symbols along a front */
struct GMT_FRONTLINE {
	double f_gap;		/* Gap between front symbols in inches */
	double f_len;		/* Length of front symbols in inches */
	double f_off;		/* Offset of first symbol from start of front in inches */
	double f_angle;		/* Angle of the slip vector hook [30] */
	int f_sense;		/* Draw symbols to left (+1), centered (0), or right (-1) of line */
	int f_symbol;		/* Which symbol to draw along the front line */
	int f_pen;		/* -1 for no outline (+p), 0 for default outline [-1], +1 if +p<pen> was used to set separate pen for outline */
	struct GMT_PEN pen;	/* Pen for outline of front symbol [-W] */
};

/* Vector symbols */

struct GMT_VECT_ATTR {
	/* Container for common attributes for plot attributes of vectors */
	unsigned int status;	/* Bit flags for vector information (see GMT_enum_vecattr above) */
	unsigned int v_kind[2];	/* Type of vector heads */
	bool parsed_v4;		/* true if we parsed old-style <vectorwidth/headlength/headwidth> attribute */
	float v_angle;		/* Head angle */
	float v_norm;		/* shrink when lengths are smaller than this */
	float v_stem;		/* Min length in % of visible vector when head is large [10%] */
	float v_width;		/* Width of vector stem in inches */
	float v_shape;		/* Shape of vector head [MAP_VECTOR_SHAPE] */
	float h_length;		/* Length of vector head in inches */
	float h_width;		/* Width of vector head in inches */
	float pole[2];		/* Longitude and latitude of geovector pole */
	float scale;		/* Converts inches to spherical degrees */
	float comp_scale;	/* Converts hypot (dx, dy) to inches */
	float v_trim[2];	/* Offsets from begin/end point in inches */
	struct GMT_PEN pen;	/* Pen for outline of head */
	struct GMT_FILL fill;	/* Fill for head [USED IN PSROSE] */
};

#define GMT_MAX_SYMBOL_COLS	6	/* Maximum number of columns required for the most complicated symbol input */

struct GMT_SYMBOL {
	/* Voodoo: If next line is not the first member in this struct, psxy -Sl<size>/Text will have corrupt 'Text'
		   in non-debug binaries compiled with VS2010 */
	char string[GMT_LEN256];	/* Character code to plot (could be octal) */

	int symbol;	/* Symbol id */
	unsigned int n_required;	/* Number of additional columns necessary to decode chosen symbol */
	unsigned int justify;	/* Justification of text item for -Sl symbol [PSL_MC = centered] */
	unsigned int u;		/* Measure unit id (0 = cm, 1 = inch, 2 = m, 3 = point */
	unsigned int read_symbol_cmd;	/* 1 when -S indicated we must read symbol type from file, 2 with -SK is used */
	bool u_set;		/* true if u was set */
	double factor;		/* Scaling needed to unify symbol area for circle, triangles, etc. [1] */
	double size_x;		/* Current symbol size in x */
	double size_y;		/* Current symbol size in y */
	double given_size_x;	/* Symbol size read from file or command line */
	double given_size_y;	/* Symbol size read from file or command line */
	bool read_size_cmd;	/* true when -S indicated we must read symbol sizes from file */
	bool read_size;		/* true when we must read symbol size from file for the current record */
	bool shade3D;		/* true when we should simulate shading of 3D symbols cube and column */
	bool fq_parse;		/* true -Sf or -Sq were given with no args on command line and must be parsed via segment headers */
	bool accumulate;	/* true if -So takes many band z and they are increments, not total z values */
	bool diagonal;		/* true if -Sr+s is given */
	struct GMT_FONT font;	/* Font to use for the -Sl symbol */
	unsigned int convert_angles;	/* If 2, convert azimuth to angle on map, 1 special case for -JX, 0 plain case */
	unsigned int n_nondim;	/* Number of columns that has angles or km (and not dimensions with units) */
	unsigned int nondim_col[GMT_MAX_SYMBOL_COLS];	/* Which columns has angles or km for this symbol */

	/* These apply to bar|column symbols */

	double base;		/* From what level to draw the bar|column */
	bool user_unit[2];	/* If true then we must project the base via R -J to get base values, otherwise they are in c|i|p units */
	unsigned int base_set;	/* 1 if user provided a custom base, 2 if we should read it from last column [otherwise 0: default to bottom axis] */

	/* These apply to geo-wedges */
	char w_unit;		/* Radius unit */
	double w_radius;	/* In spherical degrees */
	double w_radius_i;	/* Inner radius [0] */
	double w_dr, w_da;	/* Angular and radial increments for spider web */
	unsigned int w_mode;	/* Distance mode */
	enum GMT_enum_wedgetype w_type;	/* Wedge type */
	bool w_active;
	
	/* These apply to vectors */

	struct GMT_VECT_ATTR v;	/* All attributes for vector shapes etc. [see struct above] */

	struct GMT_FRONTLINE f;	/* parameters needed for a front */
	struct GMT_CUSTOM_SYMBOL *custom;	/* pointer to a custom symbol */

	struct GMT_CONTOUR G;	/* For quoted lines */
	struct GMT_DECORATE D;	/* For decorated lines */
};

#endif /* GMT_PLOT_H */
