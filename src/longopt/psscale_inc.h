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

#ifndef PSSCALE_INC_H
#define PSSCALE_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	GMT_C_CPT_KW,
	{ 0, 'D', "position",
	          "g,j,J,n,x",         "mapcoords,inside,outside,boxcoords,plotcoords",
	          "w,e,h,v,j,m,n,o,r", "size,triangles,horizontal,vertical,janchor,move_annot,nan,anchoroffset,reverse",
		  GMT_TP_STANDARD },
	{ 0, 'F', "box",
	          "",                  "",
	          "c,g,i,p,r,s",       "clearance,fill,inner,pen,radius,shade",
		  GMT_TP_STANDARD },
	{ 0, 'G', "truncate",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "illumination|shading|shade|intensity", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'L', "equalsize|equal_size",
	          "i,I",               "range,intensity|shade",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'M', "monochrome",        "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'N', "dpi",
	          "p",                 "discrete",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'Q', "log",               "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'S', "appearance",
	          "",                  "",
	          "a,c,n,r,s,x,y",     "angle,custom,numeric,minmax,nolines,barlabel,barunit",
		  GMT_TP_STANDARD },
	{ 0, 'W', "scale",             "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Z', "barwidths|zfile",   "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !PSSCALE_INC_H */
