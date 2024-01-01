/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2024 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#ifndef PSWIGGLE_INC_H
#define PSWIGGLE_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "azimuth",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'C', "center",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'D', "scalebar",
	          "g,j,J,n,x",         "mapcoords,inside,outside,boxcoords,plotcoords",
	          "w,j,a,o,l",         "length,janchor,side,anchoroffset,label",
		  GMT_TP_STANDARD },
	{ 0, 'F', "panel",
	          "",                  "",
	          "c,g,i,p,r,s",       "clearance,fill,inner,pen,radius,shade",
		  GMT_TP_STANDARD },
	{ 0, 'G', "fill",
                  "",                  "",
                  "n,p",               "negative,positive",
		  GMT_TP_STANDARD },
	{ 0, 'I', "fixedazim",         "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'T', "trackpen|track",    "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'W', "outlinepen|pen",    "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Z', "ampscale",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};

#endif  /* !PSWIGGLE_INC_H */
