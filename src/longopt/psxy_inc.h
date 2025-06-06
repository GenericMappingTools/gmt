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

#ifndef PSXY_INC_H
#define PSXY_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "steps|stairs|straight_lines",
	          "x,y",                     "x,y",
	          "",                        "",
		  GMT_TP_STANDARD },
	GMT_C_CPT_KW,
	{ 0, 'D', "offset",                  "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'E', "errorbars|error_bars|error_bar",
	          "x,y,X,Y",                 "xbar,ybar,boxwhisker,stemleaf",
	          "a,A,c,n,w,p",             "asymmetrical,lhbounds,symbolfill,notch,capwidth,pen",
		  GMT_TP_STANDARD },
	{ 0, 'F', "connection",
	          "c,n,p",                   "continuous,network,refpoint",
	          "",                        "",
		  GMT_TP_STANDARD },
	{ 0, 'H', "scale",                   "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "intensity",               "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'L', "polygon",
		  "",                        "",
	          "b,d,D,x,y,p",             "bounds,symdev,asymdev,xanchor,yanchor,pen",
		  GMT_TP_STANDARD },
	{ 0, 'N', "noclip",
	          "c,r",                     "clipnorepeat,repeatnoclip",
	          "",                        "",
		  GMT_TP_STANDARD },
	{ 0, 'T', "ignoreinfiles",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Z', "zvalue|level",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};

#endif  /* !PSXY_INC_H */
