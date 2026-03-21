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

#ifndef PSCONTOUR_INC_H
#define PSCONTOUR_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "annotation|annot",
	          "n",                 "none",
	          "a,c,d,e,f,g,i,j,n,o,p,r,t,u,v,w,x,=",
		                       "angle,clearance,debug,delay,font,fill,nolines,justify,nudge,rounded,pen,minradius,labelfile,unit,curved,npoints,firstlast,prefix",
		  GMT_TP_STANDARD },
	{ 0, 'C', "contours|levels",   "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'D', "dump",              "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'E', "index",
	          "",                  "",
	          "b",                 "binary",
		  GMT_TP_STANDARD },
	{ 0, 'G', "labels|label_placement",
	          "d,D,f,l,L,n,N,x,X", "plotdist,mapdist,locfile,segments,circles,nlabels,linestart,segmentfile,circlefile",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'I', "colorize",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'L', "mesh|triangular_mesh_pen", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'N', "noclip|no_clip",    "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Q', "cut|minpoints|minlength",
	          "",                  "",
	          "z",                 "nonzero",
		  GMT_TP_STANDARD },
	{ 0, 'S', "skip",
	          "p,t",               "points,triangles",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'T', "ticks",
	          "h,l",               "high,low",
	          "a,d,l",             "all,gap,labels",
		  GMT_TP_STANDARD },
	{ 0, 'W', "pen",
	          "a,c",               "annot,regular",
	          "c",                 "colors",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !PSCONTOUR_INC_H */
