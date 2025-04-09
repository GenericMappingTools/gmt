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

#ifndef PSXYZ_INC_H
#define PSXYZ_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "steps|stairs|straight_lines",
	          "x,y",       "x,y",
	          "",                  "",
		  GMT_TP_STANDARD },
	GMT_C_CPT_KW,
	{ 0, 'D', "offset",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'G', "fill",
	          "",                  "",
	          "z",                 "zvalue",
		  GMT_TP_STANDARD },
	{ 0, 'H', "scale",             "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "intensity|intens",         "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'L', "polygon|close|closed_polygon",
	          "",                  "",
	          "b,d,D,x,y,p",       "bounds,symdev,asymdev,xanchor,yanchor,pen",
		  GMT_TP_STANDARD },
	{ 0, 'N', "noclip|no_clip",
	          "c,r",               "clipnorepeat,repeatnoclip",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'Q', "nosort|no_sort",    "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'W', "pen",
	          "",                  "",
	          "c,o,s,v,z",         "color,offset,spline,vector,zvalues",
		  GMT_TP_STANDARD },
	{ 0, 'Z', "zvalue|level",
	          "",                  "",
	          "t,T",               "transparency,twocols",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !PSXYZ_INC_H */
