/*--------------------------------------------------------------------
 *    $Id: block_subs.h,v 1.2 2006-04-05 01:58:00 pwessel Exp $
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

/* These functions are used in both blockmedian and blockmode and are
 * thus defined in an include file to avoid duplication of code.
 * They are not used anywhere else.  Prototypes are listed in both
 * main programs. [PW, 25-MAR-2006].
 */

/* Function prototypes for comparison functions used by both blockmedian
   and blockmode.
 */

#define BLK_X	0
#define BLK_Y	1
#define BLK_Z	2
#define BLK_W	3

struct BLK_DATA {
	double	a[4];	/* a[0] = x, a[1] = y, a[2] = z, a[3] = w  */
       	size_t     i;	/* Index to data value */
};

int BLK_compare_x (const void *point_1, const void *point_2);
int BLK_compare_y (const void *point_1, const void *point_2);
int BLK_compare_index_z (const void *point_1, const void *point_2);
int BLK_compare_sub (const void *point_1, const void *point_2, int item);
