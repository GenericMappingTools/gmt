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

#ifndef PSCOAST_INC_H
#define PSCOAST_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
	          short_directives,    long_directives,
	          short_modifiers,     long_modifiers */
	{ 0, 'A', "area",
	          "",                  "",
	          "a,l,r,p",           "antarctica,lakes,riverlakes,percentexcl" },
	{ 0, 'C', "lakes|riverfill",
	          "",                  "",
	          "l,r",               "lakes,riverlakes" },
	{ 0, 'D', "resolution",
	          "f,h,i,l,c,a",       "full,high,intermediate,low,crude,auto",
	          "f",                 "lowfallback" },
  	/* -E not usable because of = usage within parameters? */
	{ 0, 'F', "panel",
	          "l,t",               "scale,rose",
	          "c,g,i,p,r,s",       "clearance,fill,inner,pen,radius,shade" },
	{ 0, 'G', "land",              "", "", "", "" },
	{ 0, 'I', "rivers",            "", "", "", "" },
	{ 0, 'J', "zaxis",
	          "z,Z",               "scale,width",
	          "",                  "" },
	{ 0, 'L', "mapscale",
	          "g,j,J,n,x",         "mapcoords,inside,outside,boxcoords,plotcoords",
	          "w,a,c,f,j,l,o,u,v", "length,align,loc,fancy,janchor,label,anchoroffset,units,vertical" },
	{ 0, 'M', "dump",              "", "", "", "" },
	{ 0, 'N', "borders",           "", "", "", "" },
	{ 0, 'Q', "markclipend",       "", "", "", "" },
	{ 0, 'S', "water",             "", "", "", "" },
  	/* -Td and -Tm not doable because they have inconsistent short modifiers? */
	{ 0, 'W', "shorelines",        "", "", "", "" },
  	/* -d (i) Synopsis usage, (ii) Optional Arguments usage, and
  	   (iii) general -d usage all inconsistent; also, why does
  	   -d in gmt_common_longopts.h not translate +c short modifier? */
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};

#endif  /* !PSCOAST_INC_H */
