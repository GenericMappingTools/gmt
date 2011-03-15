/*--------------------------------------------------------------------
 *	$Id: gmt_boundcond.h,v 1.22 2011-03-15 02:06:35 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * gmt_boundcond.h defines structures and functions used for setting
 * boundary conditions in processing grd file data.  
 *
 * Author:	W H F Smith
 * Date:	1 JAN 2010
 * Version:	5 API
 *
 */

#ifndef _GMT_BOUNDCOND_H
#define _GMT_BOUNDCOND_H

#define GMT_BC_IS_NOTSET	0
#define GMT_BC_IS_NATURAL	1
#define GMT_BC_IS_PERIODIC	2
#define GMT_BC_IS_POLE		3
#define GMT_BC_IS_DATA		4
 
struct GMT_EDGEINFO {
	/* Description below is the final outcome after parse and verify */
	GMT_LONG nxp;	/* if X periodic, nxp > 0 is the period in pixels  */
	GMT_LONG nyp;	/* if Y periodic, nxp > 0 is the period in pixels  */
	GMT_LONG gn;	/* TRUE if top    edge will be set as N pole  */
	GMT_LONG gs;	/* TRUE if bottom edge will be set as S pole  */
};

#endif /* _GMT_BOUNDCOND_H */
