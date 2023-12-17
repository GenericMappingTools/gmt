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

#ifndef PSPOLAR_INC_H
#define PSPOLAR_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'D', "center",            "", "", "", "",
		  GMT_TP_STANDARD },
	{ 0, 'M', "size",
	          "",                  "",
	          "m",                 "magnitude",
		  GMT_TP_STANDARD },
	{ 0, 'S', "symbol",
	          "a,c,d,h,i,p,s,t,x", "star,circle,diamond,hexagon,invtriangle,point,square,triangle,cross",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'N', "noclip",            "", "", "", "",
/* PROBLEM: +v modifier for -Q (following entry) can take arg strings which include + */
		  GMT_TP_STANDARD },
	{ 0, 'Q', "mode",
	          "e,f,g,h,s,t",       "extensive,focal,compressional,hypo71,spolarity,station",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'T', "station",
	          "",                  "",
	          "a,f,j,o",           "angle,font,justify,offset",
		  GMT_TP_STANDARD },
	{ 0, 'W', "pen",               "", "", "", "",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0}  /* End of list marked with empty option and strings */
};

#endif  /* !PSPOLAR_INC_H */
