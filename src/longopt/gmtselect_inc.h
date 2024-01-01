/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#ifndef GMTSELECT_INC_H
#define GMTSELECT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,

	          short_directives,    long_directives,
	          short_modifiers,     long_modifiers,

		  transproc_mask */
	{ 0, 'A', "min_area|area",
	          "",                  "",
            "a,l,r,p",           "antarctica,lakes,river_lakes,percent",
            GMT_TP_STANDARD },
	{ 0, 'C', "distance|dist2pt",
	          "",                  "",
	          "d",                 "distance",
            GMT_TP_STANDARD },
	{ 0, 'D', "resolution",
	          "f,h,i,l,c",         "full,high,intermediate,low,crude",
	          "f",                 "lower",
            GMT_TP_STANDARD },
	{ 0, 'E', "boundary",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'F', "polygon",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'G', "gridmask",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "invert|reverse",
	          "c,f,g,l,r,s,z",     "circle,polygons,zero,line,rectangle,gridmask,zrange",
	          "",                  "",
            GMT_TP_MULTIDIR },
	{ 0, 'L', "dist2line",
	          "",                  "",
	          "d,p",               "distance,project",
            GMT_TP_STANDARD },
	{ 0, 'N', "mask|maskvalues",   "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Z', "in_range",
	          "",                  "",
	          "a,c,h,i",           "any,column,header,invert",
            GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GMTSELECT_INC_H */
