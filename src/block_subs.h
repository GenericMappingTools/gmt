/*--------------------------------------------------------------------
 *    $Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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

/* This code is included into the three blockm*_func.c files which each
 * will define their names (e.g., BLOCKMEAN).  That definition controls
 * the names of the functions defined below.
 */

#if defined(BLOCKMEAN)
#define BLOCKMEAN_CTRL BLOCK_CTRL
#define NEW_BLK New_blockmean_Ctrl
#define FREE_BLK Free_blockmean_Ctrl
#elif defined(BLOCKMEDIAN)
#define BLOCKMEDIAN_CTRL BLOCK_CTRL
#define NEW_BLK New_blockmedian_Ctrl
#define FREE_BLK Free_blockmedian_Ctrl
#else
#define BLOCKMODE_CTRL BLOCK_CTRL
#define NEW_BLK New_blockmode_Ctrl
#define FREE_BLK Free_blockmode_Ctrl
#endif

struct BLOCK_CTRL {	/* All control options for this program (except common args) */
	struct C {	/* -C */
		GMT_LONG active;
	} C;
	struct E {	/* -E */
		GMT_LONG active;
		int mode;	/* Used in blockmedian to select box-and-whisker output (-Eb) */
	} E;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
#if !defined(BLOCKMEAN)		/* Only blockmedian & blockmode has a -Q option */
	struct Q {	/* -Q */
		GMT_LONG active;
	} Q;
#endif
#if defined(BLOCKMEDIAN)	/* Only blockmedian has a -T option */
	struct T {	/* -T<quantile> */
		GMT_LONG active;
		double quantile;
	} T;
#endif
	struct S {	/* -S[m|w|z] */
		GMT_LONG active;
		int mode;
	} S;
	struct W {	/* -W[i][o] */
		GMT_LONG active;
		GMT_LONG weighted[2];
	} W;
};

#if defined(BLOCKMEAN)	/* Only used by blockmean */
#define BLK_Z	0
#define BLK_W	1
#define BLK_S	0
#define BLK_L	1
#define BLK_H	2
struct BLK_PAIR {	/* Used for weighted mean location */
	double a[2];	/* a[0] = x, a[1] = y */
};
struct BLK_SLH {	/* Holds std, low, and high values */
	double a[3];	/* a[0] = w.std, a[1] = min, a[2] = max */
};
#else	/* Only used by blockmedian and blockmode */
#define BLK_Z	2
#define BLK_W	3
struct BLK_DATA {
	double a[4];	/* a[0] = x, a[1] = y, a[2] = z, a[3] = w  */
	GMT_LONG i;	/* Index to data value */
};
#endif

/* Declaring the standard functions to allocate and free the program Ctrl structure */

void * NEW_BLK (struct GMT_CTRL *G) {	/* Allocate and initialize a new control structure */
	struct BLOCK_CTRL *C;
	
	C = GMT_memory (G, NULL, 1, struct  BLOCK_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
#if defined(BLOCKMEDIAN)	/* Initialize default to 0.5, i.e., the median */
	C->T.quantile = 0.5;
#endif
	return (C);
}

void FREE_BLK (struct GMT_CTRL *G, struct  BLOCK_CTRL *C) {	/* Deallocate control structure */
	GMT_free (G, C);	
}

#if !defined(BLOCKMEAN)	/* Only used by blockmean */
/* blockmedian and blockmode */
EXTERN_MSC int BLK_compare_x (const void *point_1, const void *point_2);
EXTERN_MSC int BLK_compare_y (const void *point_1, const void *point_2);
EXTERN_MSC int BLK_compare_index_z (const void *point_1, const void *point_2);
EXTERN_MSC int BLK_compare_sub (const void *point_1, const void *point_2, int item);
#endif
