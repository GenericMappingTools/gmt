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

#ifndef GRDSELECT_INC_H
#define GRDSELECT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "area",
	          "i,u",               "intersection,union",
	          "i",                 "increment|inc",
		  GMT_TP_STANDARD },
	{ 0, 'C', "pointfile",         "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'D', "increment|inc",     "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'E', "tabs",
	          "b",                 "polygon",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'F', "polygonfile",
	          "",                  "",
	          "i,o",               "in,out",
		  GMT_TP_STANDARD },
	{ 0, 'G', "force_remote",      "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "invert|reverse",
	          "C,D,F,L,N,R,W,Z,r", "points,increment,polygons,lines,nans,region,range,zrange,registration",
	          "",                  "",
		  GMT_TP_MULTIDIR },
	{ 0, 'L', "linefile",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'M', "margins",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'N', "nans",
	          "l,h",               "lower,higher",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'W', "range",             "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Z', "zrange",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRDSELECT_INC_H */
