/*--------------------------------------------------------------------
 *	$Id: gmt_common.h,v 1.12 2009-01-09 04:02:32 guru Exp $
 *
 *	Copyright (c) 1991-2009 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
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
 * Holds current selections for the family of common GMT options.
 *
 * Author: Paul Wessel
 * Date:	16-APR-2006
 * Version:	4.3.0
 */
 
#ifndef _GMT_COMMON_H
#define _GMT_COMMON_H

struct GMT_COMMON {
	/* Structure with all information given via the 16 common GMT command-line options -R -J .. */
	struct synopsis {	/* [0]	\0 (zero) */
		BOOLEAN active;
	} synopsis;
	struct B {	/* [1]  -B<params> */
		BOOLEAN active;
	} B;	
	struct H {	/* [2]  -H[i][<nrecs>] */
		BOOLEAN active[2];
		int n_recs;
	} H;	
	struct J {	/* [3-4]  -J<params> */
		BOOLEAN active;
		int id;
		double par[6];
	} J;		
	struct K {	/* [5]  -K */
		BOOLEAN active;
	} K;	
	struct O {	/* [6]  -O */
		BOOLEAN active;
	} O;
	struct P {	/* [7]  -P */
		BOOLEAN active;
	} P;
	struct R {	/* [8]  -Rw/e/s/n[/z0/z1][r] */
		BOOLEAN active;
		BOOLEAN corners;
		double x_min, x_max, y_min, y_max, z_min, z_max;
	} R;
	struct U {	/* [9]  -U */
		BOOLEAN active;
		int just;
		double x, y;
		char *label;	
	} U;
	struct V {	/* [10]  -V */
		BOOLEAN active;
	} V;
	struct X {	/* [11]  -X */
		BOOLEAN active;
		double off;
		char mode;	/* r, a, or c */
	} X;
	struct Y {	/* [12] -Y */
		BOOLEAN active;
		double off;
		char mode;	/* r, a, or c */
	} Y;
	struct c {	/* [13]  -c */
		BOOLEAN active;
		int copies;
	} c;
	struct t {	/* [14]  -:[i|o] */
		BOOLEAN active;
		BOOLEAN toggle[2];
	} t;
	struct b {	/* -b[i|o][<n>][s|S|d|D] */
		BOOLEAN active;
		BOOLEAN binary[2];
		BOOLEAN sincle[2];
		BOOLEAN swab[2];
		int ncol[2];
	} b;
	struct f {	/* [15]  -f[i|o]<col>|<colrange>[t|T|g],.. */
		BOOLEAN active;
		char col_type[2][BUFSIZ];
	} f;
};

struct GMT_HIDDEN {
	/* These are internal parameters that need to be passed around between
	 * many GMT functions.  Eliminating global variables means passing a
	 * pointer to this structure instead. */
	int workinprogress;
};

struct GMT_CTRL {
	/* Master structure for a GMT invocation.  All internal settings for GMT is accessed here */
	struct GMT_COMMON *common;	/* Structure with all the common GMT command settings (-R -J ..) */
	struct GMT_DEFAULTS *gmtdefs;	/* Structure with all the GMT defaults settings (pens, colors, fonts.. ) */
	struct GMT_HIDDEN *hidden;	/* Internal global variables that should never be messed with by users */
};

#endif /* _GMT_COMMON_H */
