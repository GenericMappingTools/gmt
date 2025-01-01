/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2025 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#ifndef SUBPLOT_INC_H
#define SUBPLOT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "autolabel",
	          "",                  "",
	          "c,g,j,J,o,p,r,R,s,v",
	                               "clearance,fill,justify|anchor,mirror,offset,pen,roman,Roman,shaded,vtag|vertical",
		  GMT_TP_STANDARD },
	{ 0, 'C', "clearance",
	          "w,e,s,n,x,y",       "w|west,e|east,s|south,n|north,x,y",
	          "",                  "",
	          GMT_TP_STANDARD },
	{ 0, 'D', "defaults",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'F', "dimensions|dims",
	          "f,s",               "overall|figsize,subplot|subsize|panels",
	          "a,c,f,g,p,w",       "scale,expand,fractions|frac,fill,perimeter|outline,dividers|divlines",
		  GMT_TP_STANDARD },
	{ 0, 'M', "margins",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'S', "share",
	          "r,c",               "rows|y,columns|x",
	          "l,s,p,t",           "label,label2,parallel,row_title",
		  GMT_TP_STANDARD },
	{ 0, 'T', "title",             "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !SUBPLOT_INC_H */
