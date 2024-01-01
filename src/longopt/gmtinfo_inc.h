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

#ifndef GMTINFO_INC_H
#define GMTINFO_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
	          short_directives,    long_directives,
	          short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "report",
	          "a,t,s",             "all,per_table,per_segment",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'C', "columns|numeric",   "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'D', "center",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'E', "get_record",
	          "l,L,h,H",           "min,minabs,max,maxabs",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'F', "counts",
	          "i,d,t",             "totals,segments,segments_reset",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'I', "inc|increment|spacing",
	          "b,e,f,p,s",         "box,exact,fft,override,surface",
	          "e,r,R",             "extend_box,adjust,extend_region",
		  GMT_TP_STANDARD },
	{ 0, 'L', "common_limits",     "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'T', "nearest_multiple",
	          "",                  "",
	          "c",                 "column",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GMTINFO_INC_H */
