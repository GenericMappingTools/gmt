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

#ifndef MAPPROJECT_INC_H
#define MAPPROJECT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "azimuth",
	          "b,B,f,F,o,O",             "back,backgeodetic,forward,forwardgeodetic,orient,orientgeodetic",
	          "v",                       "variable",
		  GMT_TP_STANDARD },
	{ 0, 'C', "center",
	          "",                        "",
	          "m",                       "merclat",
		  GMT_TP_STANDARD },
	{ 0, 'D', "lengthunit",
	          "c,i,p",                   "cm,inch,point",
	          "",                        "",
		  GMT_TP_STANDARD },
	{ 0, 'E', "ecef",                    "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'F', "projunit",
	          "d,m,s,e,f,k,M,n,u,c,i,p", "deg,min,sec,meter,foot,km,smile,nmile,ussft,cm,inch,point",
	          "",                        "",
		  GMT_TP_STANDARD },
	{ 0, 'G', "stride",
	          "",                        "",
	          "a,i,u,v",                 "accumulated,incremental,unit,variable",
		  GMT_TP_STANDARD },
	{ 0, 'I', "inverse",                 "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'L', "proximity",
	          "",                        "",
	          "p,u",                     "segmentpoint,unit",
		  GMT_TP_STANDARD },
	{ 0, 'N', "latconvert",
	          "a,c,g,m",                 "authalic,conformal,geocentric,meridional",
	          "",                        "",
		  GMT_TP_STANDARD },
	{ 0, 'Q', "projinfo",
	          "d,e",                     "datums,ellipsoids",
	          "",                        "",
		  GMT_TP_STANDARD },
	{ 0, 'S', "suppress",                "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'T', "changedatum",
	          "h",                       "height",
	          "",                        "",
		  GMT_TP_STANDARD },
	{ 0, 'W', "mapinfo|mapsize",
	          "e,E,g,h,j,n,o,O,r,R,w,x", "encompass,encompasstext,plotcoords,height,justify,normalize,cornercoords,regiontext,width,xy",
	          "n",                       "npoints",
		  GMT_TP_STANDARD },
	{ 0, 'Z', "traveltime",
	          "",                        "",
	          "a,i,f,t",                 "accumulated,incremental,isoformat,epochtime",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};

#endif  /* !MAPPROJECT_INC_H */
