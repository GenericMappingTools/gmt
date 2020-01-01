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
 * gmt_texture.h contains definitions of structures for pens, fills, and fonts.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	6 API
 */

/*!
 * \file gmt_texture.h
 * \brief Definitions of structures for pens, fills, and fonts.
 */

#ifndef GMT_TEXTURE_H
#define GMT_TEXTURE_H

#ifndef PATH_MAX
#	define PATH_MAX 1024
#endif

/*--------------------------------------------------------------------
 *			GMT TEXTURE STRUCTURE DEFINITIONS
 *--------------------------------------------------------------------*/

struct GMT_VECT_ATTR;	/* Forward declaration (declared in gmt_plot.h) */

/*! Holds line attributes */
struct GMT_LINE_END {
	double offset;		/* Offset the start or stop of the line by this amount before drawing */
	unsigned int type;	/* Projection type for distances */
	char unit;		/* Unit of the offset (X for Cartesian, C for plot distances in inch, else map distances) */
	/* Here we will also add vector head attributes in 5.3 */
	double length;
	struct GMT_SYMBOL *V;
};

/*! Holds pen attributes */
struct GMT_PEN {
	double width;			/* In points */
	double offset;			/* In points */
	double rgb[4];			/* RGB color of pen + Transparency 0-1 [0 = opaque] */
	char style[GMT_PEN_LEN];	/* Uses points as unit internally */
	/* For line modifications: */
	unsigned int mode;		/* Line-type: PSL_LINEAR [0; default] or PSL_BEZIER [1] */
	unsigned int cptmode;		/* How a cpt affects pens and fills: 0-none, 1=use CPT for line, 2 = use CPT for fill, 3 = both */
	struct GMT_LINE_END end[2];	/* What happens at each end of the line (see above) */
};

/*! Holds fill attributes */
struct GMT_FILL {
	double rgb[4];			/* Chosen color if no pattern + Transparency 0-1 [0 = opaque] */
	double f_rgb[4], b_rgb[4];	/* Colors applied to unset and set bits in 1-bit image */
	bool use_pattern;		/* true if pattern rather than rgb is set */
	int pattern_no;			/* Number of a predefined pattern, or -1 if not set */
	unsigned int dpi;		/* Desired dpi of image building-block if use_pattern is true */
	unsigned int dim[3];		/* width, height, depth of image */
	char pattern[PATH_MAX];		/* Full filename of user-defined raster pattern */
	unsigned char *image;	/* Pointer to image array */
	struct GMT_IMAGE *I;	/* The image container */
};

/*! Holds font attributes */
struct GMT_FONT {
	double size;			/* Font size in points */
	unsigned int id;		/* Font ID number from predefined list */
	unsigned int form;		/* Combination of binary 1 = fill, 2 = outline, 4 = pattern fill [1] */
	unsigned int set;		/* 1 if font fill set, 2 if font output pen set, 3 if both, 0 for default */
	struct GMT_FILL fill;		/* Font fill [black] */
	struct GMT_PEN pen;		/* Font outline pen [none] */
};

/*! Holds information for each predefined font [Matches PSL_FONT structure] */
struct GMT_FONTSPEC {
	char name[GMT_LEN32];	/* Name of the font */
	double height;		/* Height of letter "A" for unit fontsize */
	int encode;
	int encode_orig;
};

/*! Holds information about paper sizes in points */
struct GMT_MEDIA {
	double width;		/* Width in points */
	double height;		/* Height in points */
};

#endif  /* GMT_TEXTURE_H */
