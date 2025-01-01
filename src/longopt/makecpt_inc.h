/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2025 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#ifndef MAKECPT_INC_H
#define MAKECPT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "alpha",
	          "",                  "",
	          "a",                 "all",
		  GMT_TP_STANDARD },
	GMT_C_CPT_KW,
	{ 0, 'D', "bg|background",
	          "i,o",               "in,out",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'E', "nlevels",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'F', "format|color_model",
	          "c,g,h,r,R,x",       "cmyk,gray,hsv,rgb,name,hex",
	          "c,k",               "categorical,key",
		  GMT_TP_STANDARD },
	{ 0, 'G', "truncate",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'H', "savecpt",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "reverse",
	          "c,z",               "colors,zvalues",
	          "",                  "",
		  GMT_TP_MULTIDIR },
	{ 0, 'M', "overrule|overrule_bg", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'N', "no_bg",             "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Q', "log",               "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'S', "range_mode|auto",
	          "a,m,p,q,r",         "average,median,lms|mode,quartiles,minmax",
	          "d",                 "discrete",
		  GMT_TP_STANDARD },
	{ 0, 'T', "steps|series|range",
	          "",                  "",
	          "b,l,i,n",           "log2,log10,inverse,number",
		  GMT_TP_STANDARD },
	{ 0, 'W', "categorical",
	          "w",                 "wrap",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'Z', "continuous",        "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !MAKECPT_INC_H */
