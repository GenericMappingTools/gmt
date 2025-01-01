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

#ifndef PSTEXT_INC_H
#define PSTEXT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "azimuth",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'C', "clearance",
	          "",                  "",
	          "t",                 "textbox",
		  GMT_TP_STANDARD },
	{ 0, 'D', "offset",
	          "j,J",               "away,corners",
	          "v",                 "line",
		  GMT_TP_STANDARD },
	{ 0, 'F', "attributes|attrib",
	          "",                  "",
	          "a,A,c,f,h,j,l,r,t,z",
	                               "angle,zerocenter|Angle,rjustify|region_justify,font,header,justify,label,record|rec_number,text,zformat|zvalues",
		  GMT_TP_STANDARD },
	{ 0, 'G', "fill",
	          "",                  "",
	          "n",                 "no_text|C",
		  GMT_TP_STANDARD },
	{ 0, 'L', "listfonts|list",    "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'M', "paragraph",         "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'N', "noclip|no_clip",    "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Q', "case|change_case",
	          "l,u",               "lower,upper",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'S', "shade",             "", "", "", "", GMT_TP_STANDARD },
	GMT_W_PEN_KW,
	{ 0, 'Z', "zvalues|threeD",    "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !PSTEXT_INC_H */
