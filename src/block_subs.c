/*--------------------------------------------------------------------
 *    $Id: block_subs.c,v 1.1 2006-03-23 23:49:48 pwessel Exp $
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

	if (p1->i < p2->i)
		return (-1);
	else if (p1->i > p2->i)
		return (1);
	else {
		if (p1->a[item] < p2->a[item])
			return (-1);
		else if (p1->a[item] > p2->a[item])
			return (1);
		else
			return (0);
	}
}
