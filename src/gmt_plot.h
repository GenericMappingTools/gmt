/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#ifndef _GMT_PLOT_H
#define _GMT_PLOT_H

/* Identifier for GMT_plane_perspective. The others come from GMT_io.h */

#define GMT_ZW	3

/* GMT symbol identifiers. Mostly the same as PSL_<symbol> but with
   extensions for custom symbols, psxy and psxyz */

#define GMT_SYMBOL_STAR		((int)'a')
#define GMT_SYMBOL_BARX		((int)'B')
#define GMT_SYMBOL_BARY		((int)'b')
#define GMT_SYMBOL_CIRCLE	((int)'c')
#define GMT_SYMBOL_DIAMOND	((int)'d')
#define GMT_SYMBOL_ELLIPSE	((int)'e')
#define GMT_SYMBOL_FRONT	((int)'f')
#define GMT_SYMBOL_OCTAGON	((int)'g')
#define GMT_SYMBOL_HEXAGON	((int)'h')
#define GMT_SYMBOL_INVTRIANGLE	((int)'i')
#define GMT_SYMBOL_ROTRECT	((int)'j')
#define GMT_SYMBOL_CUSTOM	((int)'k')
#define GMT_SYMBOL_TEXT		((int)'l')
#define GMT_SYMBOL_MARC		((int)'m')
#define GMT_SYMBOL_PENTAGON	((int)'n')
#define GMT_SYMBOL_COLUMN	((int)'o')
#define GMT_SYMBOL_DOT		((int)'p')
#define GMT_SYMBOL_QUOTED_LINE	((int)'q')
#define GMT_SYMBOL_RECT		((int)'r')
#define GMT_SYMBOL_RNDRECT	((int)'R')
#define GMT_SYMBOL_SQUARE	((int)'s')
#define GMT_SYMBOL_TRIANGLE	((int)'t')
#define GMT_SYMBOL_CUBE		((int)'u')
#define GMT_SYMBOL_VECTOR	((int)'v')
#define GMT_SYMBOL_WEDGE	((int)'w')
#define GMT_SYMBOL_CROSS	((int)'x')
#define GMT_SYMBOL_YDASH	((int)'y')
#define GMT_SYMBOL_ZDASH	((int)'z')
#define GMT_SYMBOL_PLUS		((int)'+')
#define GMT_SYMBOL_XDASH	((int)'-')

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

/* FRONT symbols */

enum GMT_enum_front {GMT_FRONT_FAULT = 0,
	GMT_FRONT_TRIANGLE,
	GMT_FRONT_SLIP,
	GMT_FRONT_CIRCLE,
	GMT_FRONT_BOX};

/* Direction of FRONT symbols: */

enum GMT_enum_frontdir {GMT_FRONT_RIGHT = -1,
	GMT_FRONT_CENTERED,
	GMT_FRONT_LEFT};

struct GMT_FRONTLINE {		/* A sub-symbol for symbols along a front */
	double f_gap;		/* Gap between front symbols in inches */
	double f_len;		/* Length of front symbols in inches */
	double f_off;		/* Offset of first symbol from start of front in inches */
	int f_sense;	/* Draw symbols to left (+1), centered (0), or right (-1) of line */
	int f_symbol;	/* Which symbol to draw along the front line */
};

/* Note: If changes are made to GMT_enum_vecattr you must also change pslib.h: PSL_enum_vecattr */
enum GMT_enum_vecattr {GMT_VEC_LEFT = 1,	/* Only draw left half of vector head */
	GMT_VEC_RIGHT		= 2,		/* Only draw right half of vector head */
	GMT_VEC_BEGIN		= 4,		/* Place vector head at beginning of vector */
	GMT_VEC_END		= 8,		/* Place vector head at end of vector */
	GMT_VEC_HEADS		= 12,		/* Mask for either head end */
	GMT_VEC_JUST_B		= 0,		/* Align vector beginning at (x,y) */
	GMT_VEC_JUST_C		= 16,		/* Align vector center at (x,y) */
	GMT_VEC_JUST_E		= 32,		/* Align vector end at (x,y) */
	GMT_VEC_JUST_S		= 64,		/* Align vector center at (x,y) */
	GMT_VEC_ANGLES		= 128,		/* Got start/stop angles instead of az, length */
	GMT_VEC_POLE		= 256,		/* Got pole of small/great circle */
	GMT_VEC_OUTLINE		= 512,		/* Draw vector head outline using default pen */
	GMT_VEC_OUTLINE2	= 1024,		/* Draw vector head outline using supplied v_pen */
	GMT_VEC_FILL		= 2048,		/* Fill vector head using default fill */
	GMT_VEC_FILL2		= 4096,		/* Fill vector head using supplied v_fill) */
	GMT_VEC_MARC90		= 8192,		/* Matharc only: if angles subtend 90, draw straight angle symbol */
	GMT_VEC_SCALE		= 32768};	/* Not needed in pslib: If not set we determine the required inch-to-degree scale */

#define GMT_vec_justify(status) ((status>>4)&3)			/* Return justification as 0-3 */
#define GMT_vec_head(status) ((status>>2)&3)			/* Return head selection as 0-3 */
#define GMT_vec_side(status) ((status&3) ? 2*(status&3)-3 : 0)	/* Return side selection as 0,-1,+1 */
#define GMT_vec_outline(status) ((status&GMT_VEC_OUTLINE) || (status&GMT_VEC_OUTLINE2))	/* Return true if outline is currently selected */
#define GMT_vec_fill(status) ((status&GMT_VEC_FILL) || (status&GMT_VEC_FILL2))		/* Return true if fill is currently selected */

struct GMT_VECT_ATTR {
	/* Container for common attributes for plot attributes of vectors */
	unsigned int status;	/* Bit flags for vector information (see GMT_enum_vecattr above) */
	bool parsed_v4;		/* true if we parsed old-style <vectorwidth/headlength/headwidth> attribute */
	float v_angle;		/* Head angle */
	float v_norm;		/* shrink when lengths are smaller than this */
	float v_stem;		/* Min length in % of visible vector when head is large [10%] */
	float v_width;		/* Width of vector stem in inches */
	float h_length;		/* Length of vector head in inches */
	float h_width;		/* Width of vector head in inches */
	float pole[2];		/* Longitude and latitude of geovector pole */
	float scale;		/* Converts inches to spherical degrees */
	struct GMT_PEN pen;	/* Pen for outline of head */
	struct GMT_FILL fill;	/* Fill for head [USED IN PSROSE] */
};

struct GMT_SYMBOL {
	/* Voodoo: If next line is not the first member in this struct, psxy -Sl<size>/Text will have corrupt 'Text'
		   in non-debug binaries compiled with VS2010 */
	char string[GMT_LEN256];	/* Character code to plot (could be octal) */

	int symbol;	/* Symbol id */
	unsigned int n_required;	/* Number of additional columns necessary to decode chosen symbol */
	unsigned int justify;	/* Justification of text item for -Sl symbol [PSL_MC = centered] */
	unsigned int u;		/* Measure unit id (0 = cm, 1 = inch, 2 = m, 3 = point */
	bool u_set;		/* true if u was set */
	double size_x;		/* Current symbol size in x */
	double size_y;		/* Current symbol size in y */
	double given_size_x;	/* Symbol size read from file or command line */
	double given_size_y;	/* Symbol size read from file or command line */
	bool read_size_cmd;	/* true when -S indicated we must read symbol sizes from file */
	bool read_symbol_cmd;	/* true when -S indicated we must read symbol type from file */
	bool read_size;	/* true when we must read symbol size from file for the current record */
	bool shade3D;	/* true when we should simulate shading of 3D symbols cube and column */
	struct GMT_FONT font;	/* Font to use for the -Sl symbol */
	unsigned int convert_angles;	/* If 2, convert azimuth to angle on map, 1 special case for -JX, 0 plain case */
	unsigned int n_nondim;	/* Number of columns that has angles or km (and not dimensions with units) */
	unsigned int nondim_col[6];	/* Which columns has angles or km for this symbol */

	/* These apply to bar|column symbols */

	double base;		/* From what level to draw the bar|column */
	bool user_unit[2];	/* If true then we must project the base via R -J to get base values, otherwise they are in c|i|p units */
	unsigned int base_set;	/* 1 if user provided a custom base, 2 if we should read it from last column [otherwise 0: default to bottom axis] */

	/* These apply to vectors */

	struct GMT_VECT_ATTR v;	/* All attributes for vector shapes etc. [see struct above] */

	struct GMT_FRONTLINE f;	/* parameters needed for a front */
	struct GMT_CUSTOM_SYMBOL *custom;	/* pointer to a custom symbol */

	struct GMT_CONTOUR G;	/* For labelled lines */
};

#endif /* _GMT_PLOT_H */
