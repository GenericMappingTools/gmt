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

#ifndef GMTSPATIAL_INC_H
#define GMTSPATIAL_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "nn|nearest_neighbor",
	          "a",                 "min_dist",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'C', "clip",              "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'D', "duplicates",
	          "",                  "",
	          "a,c,C,d,f,p,s",     "amax,cmax,cmin,dmax,file,perpendicular,factor",
		  GMT_TP_STANDARD },
	{ 0, 'E', "handedness",
	          "",                  "",
	          "p,n",               "positive|counterclockwise,negative|clockwise",
		  GMT_TP_STANDARD },
	{ 0, 'F', "force_polygons",
	          "l",                 "lines",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'I', "intersections",
	          "e,i",               "external,internal",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'L', "no_tile_lines",     "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'N', "in_polygons",
	          "",                  "",
	          "a,i,p,r,z",         "all,individual,start,report,addcolumn",
		  GMT_TP_STANDARD },
	{ 0, 'Q', "centroid|area|length",
	          "",                  "",
	          "c,h,l,p,s",         "range|limits,header,lines,close,sort",
		  GMT_TP_STANDARD },
	{ 0, 'S', "spatial",
	          "b,h,i,j,s,u",       "buffer,hole,intersection,join,split,union",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'T', "truncate",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'W', "extend",
	          "",                  "",
	          "f,l",               "first,last",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GMTSPATIAL_INC_H */
