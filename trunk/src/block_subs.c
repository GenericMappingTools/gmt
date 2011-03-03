/*--------------------------------------------------------------------
 *    $Id: block_subs.c,v 1.15 2011-03-03 21:02:50 guru Exp $
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
 
/* Sort on index, then x */
int BLK_compare_x (const void *point_1, const void *point_2)
{
	return (BLK_compare_sub (point_1, point_2, BLK_X));
}

/* Sort on index, then y */
int BLK_compare_y (const void *point_1, const void *point_2)
{
	return (BLK_compare_sub (point_1, point_2, BLK_Y));
}

/* Sort on index, then z */
int BLK_compare_index_z (const void *point_1, const void *point_2)
{
	return (BLK_compare_sub (point_1, point_2, BLK_Z));
}

/* Sort on index, then the specified item a[0,1,2] = x, y, z */
int BLK_compare_sub (const void *point_1, const void *point_2, int item)
{
	struct BLK_DATA *p1, *p2;

	p1 = (struct BLK_DATA *)point_1;
	p2 = (struct BLK_DATA *)point_2;

	/* First sort on bin index i */
	if (p1->i < p2->i) return (-1);
	if (p1->i > p2->i) return (+1);
	/* OK, comparing values in the same bin */
	if (p1->a[item] < p2->a[item]) return (-1);
	if (p1->a[item] > p2->a[item]) return (+1);
	/* Values are the same, return 0 */
	return (0);
}

#if defined(BLOCKMEAN)
#define NEW New_blockmean_Ctrl
#define FREE Free_blockmean_Ctrl
#elif defined(BLOCKMEDIAN)
#define NEW New_blockmedian_Ctrl
#define FREE Free_blockmedian_Ctrl
#else
#define NEW New_blockmode_Ctrl
#define FREE Free_blockmode_Ctrl
#endif

void * NEW () {	/* Allocate and initialize a new control structure */
	struct BLOCK_CTRL *C;
	
	C = (struct BLOCK_CTRL *) GMT_memory (VNULL, (size_t)1, sizeof (struct  BLOCK_CTRL), "New_block_Ctrl");
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
#if defined(BLOCKMEDIAN)
	C->T.quantile = 0.5;
#endif
	return ((void *)C);
}

void FREE (struct  BLOCK_CTRL *C) {	/* Deallocate control structure */
	GMT_free ((void *)C);	
}
