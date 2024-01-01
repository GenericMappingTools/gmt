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

#ifndef PSCOUPE_INC_H
#define PSCOUPE_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "crosssection",
	          "a,b,c,d",           "geopoints,geostrikelen,xypoints,xystrikelen",
	          "c,d,r,w,z,f",       "region,dip,domain,width,depth,frame",
		  GMT_TP_STANDARD },
	{ 0, 'S', "format",
	          "a,c,m,d,z,p,x,y,t", "aki,cmt,smtfull,smtdouble,smtdev,partial,axisfull,axisdouble,axisdev",
	          "a,f,j,l,m,o,s",     "angle,font,justify,moment,samesize,offset,mreference",
		  GMT_TP_STANDARD },
	{ 0, 'C', "cpt",               "", "", "", "",
		  GMT_TP_STANDARD },
	{ 0, 'F', "mode",
	          "s,a,e,g,p,r,t",     "symbol,ptaxes,taxisfill,paxisfill,paxispen,box,taxispen",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'H', "scale",             "", "", "", "",
		  GMT_TP_STANDARD },
	{ 0, 'I', "intensity",         "", "", "", "",
		  GMT_TP_STANDARD },
	{ 0, 'L', "outlinepen",        "", "", "", "",
		  GMT_TP_STANDARD },
	{ 0, 'N', "noclip",            "", "", "", "",
		  GMT_TP_STANDARD },
	{ 0, 'Q', "noinfofiles",       "", "", "", "",
		  GMT_TP_STANDARD },
	{ 0, 'T', "nodal",             "", "", "", "",
		  GMT_TP_STANDARD },
	{ 0, 'W', "pen",               "", "", "", "",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0}  /* End of list marked with empty option and strings */
};

#endif  /* !PSCOUPE_INC_H */
