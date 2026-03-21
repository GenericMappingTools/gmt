/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2026 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#ifndef GRDVECTOR_INC_H
#define GRDVECTOR_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "polar",             "", "", "", "", GMT_TP_STANDARD },
	GMT_C_CPT_KW,
	{ 0, 'G', "fill",              "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "increment|inc|spacing",
	          "x",                 "multiples",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'N', "noclip",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Q', "vector",
	          "",                  "",
	          "a,b,c,e,g,h,j,m,n,o,p,q,s,t,v,z",
	                               "apex,begin,cpt|cmap,end,fill,shape,justify,midpoint,norm,oblique,pen,angles,xycoords,trim,polar_scale,polar_convert",
		  GMT_TP_STANDARD },
	{ 0, 'S', "vec_scale",
	          "i,l",               "invert,length",
	          "c,s",               "location,size",
		  GMT_TP_STANDARD },
	{ 0, 'T', "sign_scale",        "", "", "", "", GMT_TP_STANDARD },
	GMT_W_PEN_KW,
	{ 0, 'Z', "azimuth",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRDVECTOR_INC_H */
