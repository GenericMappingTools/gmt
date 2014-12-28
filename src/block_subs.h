/*--------------------------------------------------------------------
 *    $Id$
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
		bool active;
	} C;
#if defined(BLOCKMODE)	/* Only blockmode has a -D option */
	struct D {	/* -D<binwidth>[+c][+l][+h] */
		bool active;
		bool center;
		int mode;
		double width;
	} D;
#endif
	struct E {	/* -E */
		bool active;
		unsigned int mode;
	} E;
	struct G {	/* -G<outfile> */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct N {	/* -N<empty> */
		bool active;
		double no_data;
	} N;
#if !defined(BLOCKMEAN)		/* Only blockmedian & blockmode has a -Q option */
	struct Q {	/* -Q */
		bool active;
	} Q;
#endif
#if defined(BLOCKMEDIAN)	/* Only blockmedian has a -T option */
	struct T {	/* -T<quantile> */
		bool active;
		double quantile;
	} T;
#endif
	struct S {	/* -S<item> */
		bool active;
		unsigned int mode;
	} S;
	struct W {	/* -W[i][o] */
		bool active;
		bool weighted[2];
	} W;
};

#if 0
enum GMT_grdval_blks {	/* mode for selected item for gridding */
	BLK_ITEM_MEAN = 0,
	BLK_ITEM_MEDIAN,
	BLK_ITEM_MODE,
	BLK_ITEM_LOW,
	BLK_ITEM_HIGH,
	BLK_ITEM_QUARTILE,
	BLK_ITEM_RANGE,
	BLK_ITEM_N,
	BLK_ITEM_WSUM,
	BLK_ITEM_ZSUM,
	BLK_ITEM_SOURCE,
	BLK_ITEM_RECORD,
	BLK_ITEM_STDEV,
	BLK_ITEM_L1SCL,
	BLK_ITEM_LMSSCL,
	BLK_N_ITEMS};

static char *blk_name[BLK_N_ITEMS] =
{
	"mean",
	"median",
	"mode",
	"min",
	"max",
	"quartile",
	"range",
	"n",
	"sum_z",
	"sum_w",
	"source",
	"record",
	"std",
	"L1scl",
	"LMSscl"
};
#endif

#if defined(BLOCKMODE)	/* Only used by blockmode */
enum Blockmode_mode {
	BLOCKMODE_DEF  = -2,
	BLOCKMODE_LOW  = -1,
	BLOCKMODE_AVE  = 0,
	BLOCKMODE_HIGH = +1
};
#endif

#if defined(BLOCKMEAN)	/* Only used by blockmean */
enum GMT_enum_blks {BLK_Z	= 0,
	BLK_W		= 1,
	BLK_S		= 0,
	BLK_L		= 1,
	BLK_H		= 2,
	BLK_G		= 3};

struct BLK_PAIR {	/* Used for weighted mean location */
	double a[2];	/* a[0] = x, a[1] = y */
};

struct BLK_SLHG {	/* Holds std, low, high, and sigma^2 values */
	double a[4];	/* a[0] = w.std, a[1] = min, a[2] = max, a[3] = sigma^2 */
};
#else	/* Only used by blockmedian and blockmode */
#define BLK_DO_EXTEND3	1
#define BLK_DO_EXTEND4	2
#define BLK_DO_INDEX_LO	4
#define BLK_DO_INDEX_HI	8
#define BLK_DO_SRC_ID	16

enum GMT_enum_blks {BLK_Z	= 2,
		BLK_W		= 3};
struct BLK_DATA {
	double a[4];		/* a[0] = x, a[1] = y, a[2] = z, a[3] = w  */
	uint64_t ij;	/* Grid index for data value */
#if !defined(BLOCKMEAN)		/* Only blockmedian & blockmode has a -Q option */
	uint64_t src_id;	/* Source id [Data record] on input */
#endif
};
#endif

/* Declaring the standard functions to allocate and free the program Ctrl structure */

void * NEW_BLK (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct BLOCK_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct  BLOCK_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
#if defined(BLOCKMODE)	/* Only used by blockmode */
	C->D.mode = BLOCKMODE_LOW;
#endif
#if defined(BLOCKMEDIAN)	/* Initialize default to 0.5, i.e., the median */
	C->T.quantile = 0.5;
#endif
	return (C);
}

void FREE_BLK (struct GMT_CTRL *GMT, struct  BLOCK_CTRL *C) {	/* Deallocate control structure */
	if (C->G.file) free (C->G.file);	
	GMT_free (GMT, C);	
}

#if !defined(BLOCKMEAN)	/* Only used by blockmean */
/* blockmedian and blockmode */
EXTERN_MSC int BLK_compare_x (const void *point_1, const void *point_2);
EXTERN_MSC int BLK_compare_y (const void *point_1, const void *point_2);
EXTERN_MSC int BLK_compare_index_z (const void *point_1, const void *point_2);
EXTERN_MSC int BLK_compare_sub (const void *point_1, const void *point_2, int item);
#endif
