/*--------------------------------------------------------------------
 *	$Id: gmt_boundcond.h,v 1.21 2011-03-03 21:02:50 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
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
 * Date:	17 April 1998
 * Version:	4.1.x
 *
 */

#ifndef _GMT_BOUNDCOND_H
#define _GMT_BOUNDCOND_H

struct GMT_EDGEINFO {
	/* Description below is the final outcome after parse and verify */
	GMT_LONG nxp;	/* if X periodic, nxp > 0 is the period in pixels  */
	GMT_LONG nyp;	/* if Y periodic, nxp > 0 is the period in pixels  */
	GMT_LONG	gn;	/* TRUE if top    edge will be set as N pole  */
	GMT_LONG	gs;	/* TRUE if bottom edge will be set as S pole  */
};


/*   GMT_boundcond_init initializes elements of struct to 0 and FALSE  */
EXTERN_MSC void GMT_boundcond_init (struct GMT_EDGEINFO *edgeinfo);

/*  GMT_boundcond_parse reads the argv[i][2] string and flags user's wishes  */
EXTERN_MSC GMT_LONG GMT_boundcond_parse (struct GMT_EDGEINFO *edgeinfo, char *edgestring);

/*  GMT_boundcond_param_prep sets edgeinfo according to wishes and grd h  */
EXTERN_MSC GMT_LONG GMT_boundcond_param_prep (struct GRD_HEADER *h, struct GMT_EDGEINFO *edgeinfo);

/*  GMT_boundcond_set sets padding values around grd to implement bond conds.  */
EXTERN_MSC GMT_LONG GMT_boundcond_set (struct GRD_HEADER *h, struct GMT_EDGEINFO *edgeinfo, GMT_LONG *pad, float *data);

/* GMT_?_out_of_bounds will shift i or j according to BC or return TRUE if outside */

EXTERN_MSC GMT_LONG GMT_y_out_of_bounds (GMT_LONG *j, struct GRD_HEADER *h, struct GMT_EDGEINFO *edgeinfo, GMT_LONG *wrap_180);
EXTERN_MSC GMT_LONG GMT_x_out_of_bounds (GMT_LONG *i, struct GRD_HEADER *h, struct GMT_EDGEINFO *edgeinfo, GMT_LONG wrap_180);

#endif /* _GMT_BOUNDCOND_H */
