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

#ifndef GRDVIEW_INC_H
#define GRDVIEW_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	GMT_C_CPT_KW,
	{ 0, 'G', "drapegrid|drape",   "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "illumination|shading|intensity",
	          "",                  "",
	          "a,d,m,n",           "azimuth,default,ambient,args",
		  GMT_TP_STANDARD },
	{ 0, 'N', "plane",
	          "",                  "",
	          "g",                 "color",
		  GMT_TP_STANDARD },
	{ 0, 'Q', "plottype|surftype",
	          "",                  "",
	          "m",                 "monochrome|mono",
		  GMT_TP_STANDARD },
	{ 0, 'S', "smooth",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'T', "no_interp",
	          "",                  "",
	          "o,s",               "outlines,skipnans",
		  GMT_TP_STANDARD },
	{ 0, 'W', "pen",
	          "c,m,f",             "contour,mesh,facade",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRDVIEW_INC_H */
