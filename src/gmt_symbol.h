/*--------------------------------------------------------------------
 *	$Id: gmt_symbol.h,v 1.10 2003-04-01 20:03:27 pwessel Exp $
 *
 *	Copyright (c) 1991-2002 by P. Wessel and W. H. F. Smith
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
 * Miscellaneous definitions and structures related to custom psxy[z] symbols.
 *
 * Author: Paul Wessel
 * Date:	13-SEP-2001
 * Version:	4
 */

#ifndef _GMT_SYMBOLS_H
#define _GMT_SYMBOLS_H

#define ACTION_MOVE		100
#define ACTION_DRAW		200
#define ACTION_ARC		300

#define ACTION_CROSS		2
#define ACTION_CIRCLE		4
#define ACTION_SQUARE		5
#define ACTION_TRIANGLE		6
#define ACTION_DIAMOND		7
#define ACTION_STAR		8
#define ACTION_HEXAGON		9
#define ACTION_ITRIANGLE	10
#define ACTION_ELLIPSE		11
#define ACTION_TEXT		14
#define ACTION_PIE		15
#define ACTION_RECT		17

struct CUSTOM_SYMBOL_ITEM {
	double x, y, p[3];
	int action;
	struct GMT_FILL *fill;
	struct GMT_PEN *pen;
	struct CUSTOM_SYMBOL_ITEM *next;
	char *string;
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
