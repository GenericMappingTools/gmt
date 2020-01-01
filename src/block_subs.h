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

/**
 * \file block_subs.h
 * \brief Code included into the three blockm*_func.c files 
 *
 * This code is included into the three blockm*_func.c files which each
 * will define their names (e.g., BLOCKMEAN).  That definition controls
 * the names of the functions defined below.
 */

#if defined(BLOCKMEAN)
#define BLOCKMEAN_CTRL BLOCK_CTRL
#elif defined(BLOCKMEDIAN)
#define BLOCKMEDIAN_CTRL BLOCK_CTRL
#else
#define BLOCKMODE_CTRL BLOCK_CTRL
#endif

#define BLK_N_FIELDS	8

/*! All control options for this program (except common args) */
struct BLOCK_CTRL {
	struct A {	/* -A<fields> */
		bool active;
		unsigned int n_selected;
		bool selected[BLK_N_FIELDS];
	} A;
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
		unsigned int n;			/* Number of output grids specified via -G */
		char *file[BLK_N_FIELDS];	/* Only first is used for commandline but API may need many */
	} G;
	struct N {	/* -N<empty> */
		bool active;
		double no_data;
	} N;
#if !defined(BLOCKMEAN)		/* Only blockmedian & blockmode have a -Q option */
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
	struct W {	/* -W[i][o][+s] */
		bool active;
		bool weighted[2];
		bool sigma[2];
	} W;
};

GMT_LOCAL struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for all the block* modules */
	/* separator, short-option, long-option, short-directives, long-directives, short-modifiers, long-modifiers */
	{ 0, 'A', "fields", "", "", "", "" },
	{ 0, 'C', "center", "", "", "", "" },
#if defined(BLOCKMODE)	/* Only blockmode has a -D option */
	{ 0, 'D', "bin-width", "", "", "a,c,h,l", "average,center,high,low" },
#endif
#if defined(BLOCKMEAN)
	{ 0, 'E', "extend", "", "", "P,p", "prop-simple,prop-weighted" },
#elif defined(BLOCKMODE)
	{ 0, 'E', "extend", "r,s", "record,source", "l,h", "lower,higher" },
#else
	{ 0, 'E', "extend", "b,r,s", "box-whisker,record,source", "l,h", "lower,higher" },
#endif
	{ 0, 'G', "gridfile", "", "", "", "" },
	{ '/', 'I', "increment", "", "", "e,n", "exact,number" },
#if !defined(BLOCKMEAN)		/* Only blockmedian & blockmode have a -Q option */
	{ 0, 'Q', "quicker", "", "", "", "" },
#endif
	{ 0, 'S', "select", "m,n,s,w", "mean,count,sum,weight", "", "" },
#if defined(BLOCKMEDIAN)	/* Only blockmedian has a -T option */
	{ 0, 'T', "quantile", "", "", "", "" },
#endif
	{ 0, 'W', "weights", "i,o", "in,out", "s", "sigma" },
	{ 0, '\0', "", "", "", "", ""}	/* End of list marked with empty option and strings */
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
	"quantile",
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

/*! Used for weighted mean location */
struct BLK_PAIR {
	double a[2];	/*!< a[0] = x, a[1] = y */
};

/*! Holds std, low, high, and sigma^2 values */
struct BLK_SLHG {
	double a[4];	/*!< a[0] = w.std, a[1] = min, a[2] = max, a[3] = sigma^2 */
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
	double a[4];		/* < a[0] = x, a[1] = y, a[2] = z, a[3] = w  */
	uint64_t ij;		/* < Grid index for data value */
#if !defined(BLOCKMEAN)		/* Only blockmedian & blockmode has a -Q option */
	uint64_t src_id;	/* < Source id [Data record] on input */
#endif
};
#endif

/* Declaring the standard functions to allocate and free the program Ctrl structure */

/*! Allocate and initialize a new control structure */
GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {
	struct BLOCK_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct  BLOCK_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
#if defined(BLOCKMODE)	/* Only used by blockmode */
	C->D.mode = BLOCKMODE_LOW;
#endif
#if defined(BLOCKMEDIAN)	/* Initialize default to 0.5, i.e., the median */
	C->T.quantile = 0.5;
#endif
	return (C);
}

/*! Deallocate control structure */
GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct  BLOCK_CTRL *C) {
	unsigned int k;
	if (!C) return;
	for (k = 0; k < C->G.n; k++)
		gmt_M_str_free (C->G.file[k]);	
	gmt_M_free (GMT, C);	
}

#if !defined(BLOCKMEAN)	/* Not used by blockmean */
/* These BLK functions are used in both blockmedian and blockmode and are
 * thus defined here to avoid duplication of code.
 * They are not used anywhere else.  PW, 25-FEB-2016].
 */

/*! . */
enum GMT_enum_blockcases {BLK_CASE_X = 0,
	BLK_CASE_Y	= 1,
	BLK_CASE_Z	= 2};

/*! Sort on index, then the specified item a[0,1,2] = x, y, z */
GMT_LOCAL int BLK_compare_sub (const void *point_1, const void *point_2, int item) {
	const struct BLK_DATA *p1 = point_1, *p2 = point_2;

	/* First sort on bin index ij */
	if (p1->ij < p2->ij) return (-1);
	if (p1->ij > p2->ij) return (+1);
	/* OK, comparing values in the same bin */
	if (p1->a[item] < p2->a[item]) return (-1);
	if (p1->a[item] > p2->a[item]) return (+1);
	/* Values are the same, return 0 */
	return (0);
}

/*! Sort on index, then x */
GMT_LOCAL int BLK_compare_x (const void *point_1, const void *point_2) {
	return (BLK_compare_sub (point_1, point_2, BLK_CASE_X));
}

/* Sort on index, then y */
GMT_LOCAL int BLK_compare_y (const void *point_1, const void *point_2) {
	return (BLK_compare_sub (point_1, point_2, BLK_CASE_Y));
}

/*! Sort on index, then z */
GMT_LOCAL int BLK_compare_index_z (const void *point_1, const void *point_2) {
	return (BLK_compare_sub (point_1, point_2, BLK_CASE_Z));
}

#endif
