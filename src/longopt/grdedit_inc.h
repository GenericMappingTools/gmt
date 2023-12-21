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

#ifndef GRDEDIT_INC_H
#define GRDEDIT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "adjust_inc",        "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'C', "cmdhist|command_history",
	          "b,c,n,p",           "both,current,none,previous",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'D', "netcdf|netCDF|ncheader",
	          "",                  "",
	          "c,d,n,o,r,s,t,v,x,y,z",
		                       "cpt,dname,invalid,offset,remark,scale,title,varname,xname,yname,zname",
		  GMT_TP_STANDARD },
	{ 0, 'E', "transform",
	          "a,e,h,l,r,t,v",     "hvflip,xyswap,hflip,rot90counterclock,rot90clock,transpose,vflip",
	          "",                  "",
		  GMT_TP_STANDARD },
	GMT_G_OUTGRID_KW,
	{ 0, 'L', "lonshift_numrange",
	          "",                  "",
	          "n,p",               "negative,positive",
		  GMT_TP_STANDARD },
	{ 0, 'N', "nodes|replace",     "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'S', "lonshift_region",   "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'T', "toggle_registration|toggle", "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRDEDIT_INC_H */
