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

#ifndef PSROSE_INC_H
#define PSROSE_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "sector",
	          "",                  "",
	          "r",                 "rose",
		  GMT_TP_STANDARD },
	GMT_C_CPT_KW,
	{ 0, 'D', "shift",             "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'E', "vectors",
	          "m",                 "mean",
	          "w",                 "modefile",
		  GMT_TP_STANDARD },
	{ 0, 'F', "no_scale",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'G', "fill",
	          "p,P",               "bit,bitreverse",
	          "b,f,r",             "bg|background,fg|foreground,dpi",
		  GMT_TP_STANDARD },
	{ 0, 'I', "inquire",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'L', "labels",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'M', "vector_params",
	          "",                  "",
	          "a,b,c,e,g,h,j,m,n,o,p,q,s,t,v,z",
	                               "apex,begin,cpt|cmap,end,fill,shape,justify,midpoint,norm,oblique,pen,angles,xycoords,trim,polar_scale,polar_convert",
		  GMT_TP_STANDARD },
	{ 0, 'N', "distribution|vonmises",
	          "0,1,2",             "meanstddev,medianL1,LMSscale|lmsscale",
	          "p",                 "pen",
		  GMT_TP_STANDARD },
	{ 0, 'Q', "alpha",             "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'S', "norm",
	          "",                  "",
	          "a",                 "area",
		  GMT_TP_STANDARD },
	{ 0, 'T', "orientation",       "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'W', "pen",
	          "v",                 "vector",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'Z', "scale",
	          "u",                 "unity",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !PSROSE_INC_H */
