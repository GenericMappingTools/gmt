/*--------------------------------------------------------------------
 *    $Id: block_subs.h,v 1.17 2011-03-03 21:02:50 guru Exp $
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

/* These functions are used in both blockmedian and blockmode and are
 * thus defined in an include file to avoid duplication of code.
 * They are not used anywhere else.  Prototypes are listed in both
 * main programs. [PW, 25-MAR-2006].
 * 64-bit Ready.
 */

/* Function prototypes for comparison functions used by both blockmedian
   and blockmode.
 */

#if defined(BLOCKMEAN)
#define BLOCKMEAN_CTRL BLOCK_CTRL
#elif defined(BLOCKMEDIAN)
#define BLOCKMEDIAN_CTRL BLOCK_CTRL
#else
#define BLOCKMODE_CTRL BLOCK_CTRL
#endif

struct BLOCK_CTRL {	/* All control options for this program (except common args) */
	struct C {	/* -C */
		GMT_LONG active;
	} C;
	struct E {	/* -E */
		GMT_LONG active;
		int mode;	/* Used in blockmedian to select box-and-whisker output (-Eb) */
	} E;
	struct F {	/* -F */
		GMT_LONG active;
	} F;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double xinc, yinc;
	} I;
#if !defined(BLOCKMEAN)
	struct Q {	/* -Q */
		GMT_LONG active;
	} Q;
#endif
#if defined(BLOCKMEDIAN)
	struct T {	/* -T<quantile> */
		GMT_LONG active;
		double quantile;
	} T;
#endif
	struct S {	/* -S[w|z] */
		GMT_LONG active;
		int mode;
	} S;
	struct W {	/* -W[i][o] */
		GMT_LONG active;
		GMT_LONG weighted[2];
	} W;
};

#if defined(BLOCKMEAN)
void *New_blockmean_Ctrl (), Free_blockmean_Ctrl (struct BLOCKMEAN_CTRL *C);
#elif defined(BLOCKMEDIAN)
void *New_blockmedian_Ctrl (), Free_blockmedian_Ctrl (struct BLOCKMEDIAN_CTRL *C);
#else
void *New_blockmode_Ctrl (), Free_blockmode_Ctrl (struct BLOCKMODE_CTRL *C);
#endif

#define BLK_X	0
#define BLK_Y	1
#if !defined(BLOCKMEAN)
#define BLK_Z	2
#define BLK_W	3
#endif

struct BLK_DATA {
	double	 a[4];	/* a[0] = x, a[1] = y, a[2] = z, a[3] = w  */
	GMT_LONG i;	/* Index to data value */
};

int BLK_compare_x (const void *point_1, const void *point_2);
int BLK_compare_y (const void *point_1, const void *point_2);
int BLK_compare_index_z (const void *point_1, const void *point_2);
int BLK_compare_sub (const void *point_1, const void *point_2, int item);
