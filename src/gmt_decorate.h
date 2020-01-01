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
/* gmt_decorate.h - structures and variables needed for decorating lines.

   Author:	Paul Wessel
   Date:	1 OCT 2015
   Version:	6 API

   This include file defines structures and functions used.
*/

/*!
 * \file gmt_decorate.h
 * \brief Structures and variables needed for decorating lines.
 */

#ifndef GMT_DECORATE_H
#define GMT_DECORATE_H

/*! Various settings for symbol placements along lines */
enum GMT_enum_decorate {
	GMT_DECORATE_NONE = 0,	/* No contour/line crossing  */
	GMT_DECORATE_XLINE,	/* Place symbols where crossing straight lines (via key points) */
	GMT_DECORATE_XCURVE};	/* Place symbols where crossing arbitrary lines (via file) */

struct GMT_DECORATE {
	/* Control section */
	double symbol_dist_spacing;	/* Min distance between symbols */
	double symbol_dist_frac;	/* Fraction of Min distance between symbols offset for closed lines [0.25] */
	double min_dist;		/* Do not place symbols closer than this value [0] */
	double slop;			/* slop distance in being close to points */
	double *f_xy[2];		/* Array for fixed points */
	double symbol_angle;		/* For fixed-angle symbols only */
	double nudge[2];		/* Shift between calculated and desired symbols placement */
	unsigned int line_type;		/* Kind of line: contour (1) or line (0) */
	unsigned int dist_kind;		/* What kind of distance [0 = xy, 1 = map ] */
	unsigned int half_width;	/* Number of points to use in smoothing the angle [10/2] */
	unsigned int n_cont;		/* Number of symbols per segment */
	enum GMT_enum_decorate crossing;	/* 1 for crossing simple lines, 2 for file with crossing lines */
	unsigned int nx;		/* Number of crossovers at any time */
	unsigned int f_n;		/* Number of such points */
	unsigned int nudge_flag;	/* 0 if off, 1 if nudging relative to x/y axis, 2 if following local line coordinate system */
	unsigned int angle_type;	/* 0 = line-parallel, 1 = line-normal, 2 = fixed angle */
	int number_placement;		/* How the n_cont symbols are distributed [-1/0/+1]*/
	bool isolate;			/* true if we have a limit on how close symbols may appear (see below) */
	bool segmentize;		/* true if we should segmentize input lines before plotting */
	bool spacing;			/* true if we have spacing constraints to apply */
	bool number;			/* true if we have constraints on the number of symbols to apply */
	bool do_interpolate;		/* true if we must resample the crossing lines */
	bool fixed;			/* true if we chose fixed positions */
	bool debug;			/* true of we want to draw helper lines/points */
	char line_name[16];		/* Name of line: "contour" or "line" */
	char file[PATH_MAX];		/* File with crossing lines, if specified */
	char option[GMT_BUFSIZ];	/* Copy of the option string */
	char size[GMT_LEN64];		/* The symbol size */
	char fill[GMT_LEN64];		/* The symbol fill */
	char pen[GMT_LEN64];		/* The symbol outline pen */
	char symbol_code[2];		/* The symbol code only as a null-terminated string */
	char flag;			/* Char for the option key */
	struct GMT_DATASET *X;		/* Dataset with list of structures with crossing-line coordinates */
	struct GMT_XSEGMENT *ylist_XP;	/* Sorted y-segments for crossing-lines */
	struct GMT_XSEGMENT *ylist;	/* y-indices sorted in increasing order */
	struct GMT_XOVER XC;		/* Structure with resulting crossovers */
};

#endif /* GMT_DECORATE_H */
