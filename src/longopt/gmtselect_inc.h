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

#ifndef GMTSELECT_INC_H
#define GMTSELECT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
	          short_directives,    long_directives,
	          short_modifiers,     long_modifiers */
	{ 0, 'A', "min_area|area",
	          "",                  "",
	          "a,l,r,p",           "antarctica,lakes,river_lakes,percent" },
	{ 0, 'C', "distance|dist2pt",
	          "",                  "",
	          "d",                 "distance" },
	{ 0, 'D', "resolution",
	          "f,h,i,l,c",         "full,high,intermediate,low,crude",
	          "f",                 "lower" },
	{ 0, 'E', "boundary",          "", "", "", "" },
	{ 0, 'F', "polygon",           "", "", "", "" },
	{ 0, 'G', "grid_mask",         "", "", "", "" },
	{ 0, 'I', "reverse|invert",    "", "", "", "" },
	{ 0, 'L', "dist2line",
	          "",                  "",
	          "d,p",               "distance,project" },
	{ 0, 'N', "feature_mask",      "", "", "", "" },
	{ 0, 'Z', "z_range",
	          "",                  "",
	          "a,c,h,i",           "any,column,header,invert" },
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};
#endif  /* !GMTSELECT_INC_H */
