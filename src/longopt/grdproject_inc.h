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

#ifndef GRDPROJECT_INC_H
#define GRDPROJECT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'C', "center",            "", "", "", "", GMT_TP_STANDARD },
	{ '/', 'D', "increment|inc|spacing",
	          "",                  "",
	          "e,n",               "exact,number",
		  GMT_TP_STANDARD },
	{ 0, 'E', "dpi",               "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'F', "one2one|one2one_scale",
	          "c,i,p,e,f,k,M,n,u", "cm,inch,point,meter,foot,km,smile,nmile,ussft",
	          "",                  "",
		  GMT_TP_STANDARD },
	GMT_G_OUTGRID_KW,
	{ 0, 'I', "inverse",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'M', "unit|projected_unit",
	          "c,i,p",             "cm,inch,point",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRDPROJECT_INC_H */
