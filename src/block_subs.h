/*--------------------------------------------------------------------
 *    $Id: block_subs.h,v 1.12 2009-09-09 23:27:01 guru Exp $
 *
 *	Copyright (c) 1991-2009 by P. Wessel and W. H. F. Smith
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
		BOOLEAN active;
	} C;
	struct E {	/* -E */
		BOOLEAN active;
		int mode;	/* Used in blockmedian to select box-and-whisker output (-Eb) */
	} E;
	struct F {	/* -F */
		BOOLEAN active;
	} F;
	struct I {	/* -Idx[/dy] */
		BOOLEAN active;
		double xinc, yinc;
	} I;
#if !defined(BLOCKMEAN)
	struct Q {	/* -Q */
		BOOLEAN active;
	} Q;
#endif
#if defined(BLOCKMEDIAN)
	struct T {	/* -T<quantile> */
		BOOLEAN active;
		double quantile;
	} T;
#endif
	struct S {	/* -S[w|z] */
		BOOLEAN active;
		int mode;
	} S;
	struct W {	/* -W[i][o] */
		BOOLEAN active;
		BOOLEAN weighted[2];
	} W;
};

#if defined(BLOCKMEAN)
void *New_Blockmean_Ctrl (), Free_Blockmean_Ctrl (struct BLOCKMEAN_CTRL *C);
#elif defined(BLOCKMEDIAN)
void *New_Blockmedian_Ctrl (), Free_Blockmedian_Ctrl (struct BLOCKMEDIAN_CTRL *C);
#else
void *New_Blockmode_Ctrl (), Free_Blockmode_Ctrl (struct BLOCKMODE_CTRL *C);
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
