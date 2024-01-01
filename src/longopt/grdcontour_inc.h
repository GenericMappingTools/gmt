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

#ifndef GRDCONTOUR_INC_H
#define GRDCONTOUR_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "annotation|annot",
	          "n",                 "none",
	          "a,c,d,e,f,g,i,j,n,N,o,p,r,t,u,v,w,x,=",
			               "angle,clearance,debug,delay,font,opaque,nolines,justify,nudge,xynudge,round,outline,minradius,labelfile,unit,curved,npoints,add,prefix",
		  GMT_TP_STANDARD },
	{ 0, 'C', "contours|interval", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'D', "dump",              "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'F', "force",
	          "l,r",               "left,right",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'G', "labels|label_placement",
	          "d,D,f,n,l,L,x,X",   "plotdist,mapdist,matchlocs,nlabels,lines,gcircles,xlines,xgcircles",
	          "r",                 "radius",
		  GMT_TP_STANDARD },
	{ 0, 'L', "range|limit",
	          "n,N,p,P",           "negative,zeronegative,positive,zeropositive",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'N', "fill",              "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Q', "cut|minpoints|minlength",
	          "",                  "",
	          "z",                 "nozero",
		  GMT_TP_STANDARD },
	{ 0, 'S', "smooth|resample",   "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'T', "ticks",
	          "h,l",               "highs,lows",
	          "a,d,l",             "all,gap,labels",
		  GMT_TP_STANDARD },
	{ 0, 'W', "pen",
	          "a,c",               "annotated,regular",
	          "c",                 "color",
		  GMT_TP_STANDARD },
	{ 0, 'Z', "modify|scale",
	          "",                  "",
	          "o,p,s",             "offset,periodic,scale",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRDCONTOUR_INC_H */
