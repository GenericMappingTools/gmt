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

/*!
 * \file gmt_support.h
 * \brief 
 */

#ifndef _GMT_SUPPORT_H
#define _GMT_SUPPORT_H

/*! Return codes from GMT_inonout */
enum GMT_enum_inside {
	GMT_OUTSIDE = 0,
	GMT_ONEDGE,
	GMT_INSIDE};

/*! Return codes from GMT_get_anchorpoint */
enum GMT_enum_anchor {
	GMT_ANCHOR_NOTSET = -1,		/* -D */
	GMT_ANCHOR_MAP,			/* -Dg */
	GMT_ANCHOR_JUST,		/* -Dj */
	GMT_ANCHOR_NORM,		/* -Dn */
	GMT_ANCHOR_PLOT};		/* -Dx */

/*! Definition of MATH_MACRO and some functions used by grdmath and gmtmath */
struct MATH_MACRO {
	unsigned int n_arg;	/* How many commands this macro represents */
	char *name;	/* The macro name */
	char **arg;	/* List of those commands */
};

/*! Definition of structure use for finding optimal nx.ny for surface */
struct GMT_SURFACE_SUGGESTION {	/* Used to find top ten list of faster grid dimensions  */
	unsigned int nx;
	unsigned int ny;
	double factor;	/* Speed up by a factor of factor  */
};

/*! Definition of structure used for holding information of integer items to be selected */
struct GMT_INT_SELECTION {	/* Used to hold array with items (0-n) that have been selected */
	uint64_t *item;		/* Array with item numbers given (0 is first), sorted into ascending order */
	uint64_t n;		/* Number of items */
	uint64_t current;	/* Current item in item array */
	bool invert;		/* Instead select the items NOT listed in item[] */
};

/*! Definition of structure used for holding information of text items to be selected */
struct GMT_TEXT_SELECTION {	/* Used to hold array with items (0-n) that have been selected */
	char **pattern;		/* Array with text items given, sorted into lexical order */
	int ogr_item;		/* Used if ogr_match is true */
	uint64_t n;		/* Number of items */
	bool invert;		/* Instead select the items NOT listed in item[] */
	bool *regexp;		/* Item is a regex expression */
	bool *caseless;		/* Treat as caseless */
	bool ogr_match;		/* Compare pattern to an OGR item */
};

/*! Definition of structure used for holding information about an anchor point */
struct GMT_ANCHOR {	/* Used to hold items relevant for an anchor point */
	double x;		/* X position of anchor point */
	double y;		/* Y position of anchor point */
	enum GMT_enum_anchor mode;	/* Coordinate mode */
	int justify;		/* Justification integer (1-11) for anchor (if given via -Dj) */
	char *args;		/* Text representation of any additional arguments */
};


#endif /* _GMT_SUPPORT_H */
