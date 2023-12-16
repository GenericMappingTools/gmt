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

#ifndef GRDINFO_INC_H
#define GRDINFO_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'C', "oneliner",
	          "n,t",               "numeric,name_at_end",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'D', "tiles",
	          "",                  "",
	          "i",                 "ignore_empty",
		  GMT_TP_STANDARD },
	{ 0, 'E', "extreme|extrema",
	          "x,y",               "x,y",
	          "l,L,u,U",           "min,minpos,max,maxneg",
		  GMT_TP_STANDARD },
	{ 0, 'F', "geographic",        "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'G', "force_remote",      "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "minmax_region",
	          "b,i,o,r",           "polygon,imgexact,oblique,wesn",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'L', "force_scan",
	          "0,1,2,p,a",         "scandata,medianL1,meanplus,modeLMS,all",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'M', "minmax_pos",
	          "c,f",               "conditional,force",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'T', "minmax_gridval",
	          "",                  "",
	          "a,s",               "alpha,symmetric",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRDINFO_INC_H */
