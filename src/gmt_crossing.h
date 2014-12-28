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
 * gmt_crossing.h contains definition of the structure for map crossings.
 *
 * Author:	Paul Wessel
 * Date:	01-OCT-2009
 * Version:	5 API
 */

#ifndef _GMT_CROSSING_H
#define _GMT_CROSSING_H

/*--------------------------------------------------------------------
 *			GMT XINGS STRUCTURE DEFINITION
 *--------------------------------------------------------------------*/

struct GMT_XINGS {
        double xx[2], yy[2];    /* Cartesian coordinates of intersection with map boundary */
        double angle[2];        /* Angles of intersection */
        unsigned int sides[2];	/* Side id of intersection */
        unsigned int nx;	/* Number of intersections (1 or 2) */
};

#endif  /* _GMT_CROSSING_H */
