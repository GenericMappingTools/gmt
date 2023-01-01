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

#ifndef PSXY_INC_H
#define PSXY_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
	          short_directives,          long_directives,
	          short_modifiers,           long_modifiers */
	{ 0, 'A', "straightlines",
	          "m,p,x,y,r,t",             "mpfollow,pmfollow,xyalong,yxalong,rtalong,tralong",
	          "",                        "" },
	{ 0, 'C', "cpt",                     "", "", "", "" },
	{ 0, 'D', "offset",                  "", "", "", "" },
	{ 0, 'E', "errorbars",
	          "x,y,X,Y",                 "xbar,ybar,boxwhisker,stemleaf",
	          "a,A,c,n,w,p",             "asymmetrical,lhbounds,symbolfill,notch,capwidth,pen" },
	{ 0, 'F', "connection",
	          "c,n,p",                   "continuous,network,refpoint",
	          "",                        "" },
	{ 0, 'H', "scale",                   "", "", "", "" },
	{ 0, 'I', "intensity",               "", "", "", "" },
	{ 0, 'L', "polygon",
		  "",                        "",
	          "b,d,D,x,y,p",             "bounds,symdev,asymdev,xanchor,yanchor,pen" },
	{ 0, 'N', "noclip",
	          "c,r",                     "clipnorepeat,repeatnoclip",
	          "",                        "" },
	{ 0, 'T', "ignoreinfiles",           "", "", "", "" },
	{ 0, 'Z', "zvalue",                  "", "", "", "" },
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};

#endif  /* !PSXY_INC_H */
