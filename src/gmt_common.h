/*--------------------------------------------------------------------
 *	$Id: gmt_common.h,v 1.2 2006-04-10 02:43:52 pwessel Exp $
 *
 *	Copyright (c) 1991-2006 by P. Wessel and W. H. F. Smith
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
 * Date:	10-APR-2006
 * Version:	4.1.2
 */
 
#ifndef _GMT_COMMON_H
#define _GMT_COMMON_H

struct GMT_COMMON {
	struct GMT_B_OPT {	/* [1]  -B<params> */
		BOOLEAN active;
		short int processed;
	} B;	
	struct GMT_H_OPT {	/* [2]  -H[i][<nrecs>] */
		BOOLEAN active[2];
		short int processed;
		int n_header_records;
	} H;	
	struct GMT_H_OPT {	/* [3-4]  -J<params> */
		BOOLEAN active;
		short int processed;
		int projection;
		double par[6];
	} J;		
	struct GMT_K_OPT {	/* [5]  -K */
		BOOLEAN active;
		short int processed;
	} K;	
	struct GMT_O_OPT {	/* [6]  -O */
		BOOLEAN active;
		short int processed;
	} K;
	struct GMT_P_OPT {	/* [7]  -P */
		BOOLEAN active;
		short int processed;
	} K;
	struct GMT_R_OPT {	/* [8]  -Rw/e/s/n[/z0/z1][r] */
		BOOLEAN active;
		BOOLEAN corners;
		short int processed;
		double x_min, x_max, y_min, y_max, z_min, z_max;
	} R;
	struct GMT_U_OPT {	/* [9]  -U */
		BOOLEAN active;
		short int processed;
		double x, y;
		char *label;	
	} U;
	struct GMT_V_OPT {	/* [10]  -V */
		BOOLEAN active;
		short int processed;
	} V;
	struct GMT_X_OPT {	/* [11]  -X */
		BOOLEAN active;
		double off;
		char mode;	/* r, a, or c */
	} X;
	struct GMT_X_OPT {	/* [12] -Y */
		BOOLEAN active;
		short int processed;
		double off;
		char mode;	/* r, a, or c */
	} Y;
	struct GMT_c_OPT {	/* [13]  -c */
		BOOLEAN active;
		short int processed;
		int copies;
	} c;
	struct GMT_t_OPT {	/* [14]  -:[i|o] */
		BOOLEAN active;
		BOOLEAN toggle[2];
		short int processed;
	} t;
	struct GMT_b_OPT {	/* -b[i|o][<n>][s|S|d|D] */
		BOOLEAN active;
		BOOLEAN binary[2];
		BOOLEAN precision[2];
		BOOLEAN swab[2];
		short int processed;
		int ncol[2];
	} b;
	struct GMT_f_OPT {	/* [15]  -f[i|o]<col>|<colrange>[t|T|g],.. */
		BOOLEAN active;
		short int processed;
		char col_type[2][BUFSIZ];
	} f;
};

#endif /* _GMT_COMMON_H */
