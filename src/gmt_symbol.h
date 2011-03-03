/*--------------------------------------------------------------------
 *	$Id: gmt_symbol.h,v 1.31 2011-03-03 21:02:51 guru Exp $
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
 
/*
 * Miscellaneous definitions and structures related to custom psxy[z] symbols.
 *
 * Author: Paul Wessel
 * Date:	13-SEP-2001
 * Version:	4.1.x
 */

#ifndef _GMT_SYMBOLS_H
#define _GMT_SYMBOLS_H

#define GMT_ACTION_MOVE		100
#define GMT_ACTION_DRAW		200
#define GMT_ACTION_ARC		300

#define GMT_ACTION_CROSS	2
#define GMT_ACTION_PLUS		3
#define GMT_ACTION_CIRCLE	4
#define GMT_ACTION_SQUARE	5
#define GMT_ACTION_TRIANGLE	6
#define GMT_ACTION_DIAMOND	7
#define GMT_ACTION_STAR		8
#define GMT_ACTION_HEXAGON	9
#define GMT_ACTION_ITRIANGLE	10
#define GMT_ACTION_ELLIPSE	11
#define GMT_ACTION_TEXT		14
#define GMT_ACTION_PIE		15
#define GMT_ACTION_RECT		17
#define GMT_ACTION_PENTAGON	19
#define GMT_ACTION_OCTAGON	20

struct GMT_CUSTOM_SYMBOL_ITEM {
	double x, y, p[3];
	GMT_LONG action;
	struct GMT_FILL *fill;
	struct GMT_PEN *pen;
	struct GMT_CUSTOM_SYMBOL_ITEM *next;
	char *string;
};

struct GMT_CUSTOM_SYMBOL {
	char name[GMT_TEXT_LEN];
	struct GMT_CUSTOM_SYMBOL_ITEM *first;
};

EXTERN_MSC GMT_LONG GMT_n_custom_symbols;
EXTERN_MSC struct GMT_CUSTOM_SYMBOL **GMT_custom_symbol;

EXTERN_MSC struct GMT_CUSTOM_SYMBOL * GMT_get_custom_symbol (char *name);
EXTERN_MSC void GMT_draw_custom_symbol (double x0, double y0, double z0, double size, struct GMT_CUSTOM_SYMBOL *symbol, struct GMT_PEN *pen, struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_free_custom_symbols ();

#endif	/* _GMT_SYMBOLS_H */
