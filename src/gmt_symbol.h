/*--------------------------------------------------------------------
 *	$Id: gmt_symbol.h,v 1.6 2001-09-22 21:12:25 pwessel Exp $
 *
 *	Copyright (c) 1991-2001 by P. Wessel and W. H. F. Smith
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
 * Miscellaneous definitions and structures related to cystom psxy[z] symbols.
 *
 * Author: Paul Wessel
 * Date:	13-SEP-2001
 * Version:	4
 */

#ifndef _GMT_SYMBOLS_H
#define _GMT_SYMBOLS_H

#define ACTION_MOVE		3
#define ACTION_DRAW		2
#define ACTION_CIRCLE		1
#define ACTION_ARC		0

struct CUSTOM_SYMBOL_ITEM {
	double x, y, r, dir1, dir2;
	int action;
	struct GMT_FILL *fill;
	struct GMT_PEN *pen;
	struct CUSTOM_SYMBOL_ITEM *next;
};

struct CUSTOM_SYMBOL {
	char name[64];
	struct CUSTOM_SYMBOL_ITEM *first;
};

EXTERN_MSC int GMT_n_custom_symbols;
EXTERN_MSC struct CUSTOM_SYMBOL **GMT_custom_symbol;

EXTERN_MSC struct CUSTOM_SYMBOL * GMT_get_custom_symbol (char *name);
EXTERN_MSC void GMT_draw_custom_symbol (double x0, double y0, double z0, double size, struct CUSTOM_SYMBOL *symbol, struct GMT_PEN *pen, struct GMT_FILL *fill, BOOLEAN outline);

#endif	/* _GMT_SYMBOLS_H */
