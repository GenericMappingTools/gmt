/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2026 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#ifndef PSHISTOGRAM_INC_H
#define PSHISTOGRAM_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "horizontal",        "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'C', "cpt|cmap",
	          "",                  "",
	          "b",                 "bin",
		  GMT_TP_STANDARD },
	{ 0, 'D', "annotate",
	          "",                  "",
	          "b,f,o,r",           "beneath,font,offest,rotate",
		  GMT_TP_STANDARD },
	{ 0, 'E', "barwidth|width",
	          "",                  "",
	          "o",                 "offset",
		  GMT_TP_STANDARD },
	{ 0, 'F', "center",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'G', "fill",
	          "p,P",               "bit,bitreverse",
	          "b,f,r",             "bg|background,fg|foreground,dpi",
		  GMT_TP_STANDARD },
	{ 0, 'I', "inquire",
	          "o,O",               "nonzero|no_zero,all",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'L', "extreme|out_range",
	          "b,l,h",             "both,low|first,high|last",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'N', "distribution",
	          "0,1,2",             "meanstddev,medianL1,LMSscale|lmsscale",
	          "p",                 "pen",
		  GMT_TP_STANDARD },
	{ 0, 'Q', "cumulative",
	          "r",                 "reverse",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'S', "stairs",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'T', "range|bin|series",
	          "",                  "",
	          "i,n",               "inverse,number",
		  GMT_TP_STANDARD },
	GMT_W_PEN_KW,
	{ 0, 'Z', "histtype|kind",
	          "0,1,2,3,4,5",       "counts,freq,logcount,logfreq,log10count,log10freq",
	          "w",                 "weights",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !PSHISTOGRAM_INC_H */
