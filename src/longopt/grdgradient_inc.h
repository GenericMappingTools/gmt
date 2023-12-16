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

#ifndef GRDGRADIENT_INC_H
#define GRDGRADIENT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "azimuth|azim",      "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'D', "direction|find_dir",
	          "a,c,n,o",           "aspect,conventional,add90,report",
	          "",                  "",
		  GMT_TP_MULTIDIR },
	{ 0, 'E', "radiance|lambert",
	          "m,s,p",             "m,simple,peucker",
	          "a,d,p,s",           "ambient,diffuse,specular,shine",
		  GMT_TP_STANDARD },
	GMT_G_OUTGRID_KW,
	{ 0, 'N', "normalize|norm",
	          "e,t",               "laplace,cauchy",
	          "a,s,o",             "ambient,sigma,offset",
		  GMT_TP_STANDARD },
	{ 0, 'Q', "tiles|save_stats",
	          "c,r,R",             "save,use,usedelete",
	          "f",                 "file",
		  GMT_TP_STANDARD },
	{ 0, 'S', "slope_file|slopegrid", "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRDGRADIENT_INC_H */
