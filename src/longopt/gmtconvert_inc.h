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

#ifndef GMTCONVERT_INC_H
#define GMTCONVERT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
	          short_directives,    long_directives,
	          short_modifiers,     long_modifiers */
	{ 0, 'A', "horizontal|hcat",   "", "", "", "" },
	{ 0, 'C', "n_records",
	          "",                  "",
	          "l,u,i",             "minrecs,maxrecs,invert" },
	{ 0, 'D', "dump",
	          "",                  "",
	          "o",                 "orig" },
	{ 0, 'E', "first_last|extract",
	          "f,l,m,M",           "first,last,stride,stride_last",
	          "",                  "" },
	{ 0, 'F', "conn_method",       "", "", "", "" },
	{ 0, 'I', "invert|reverse",    "", "", "", "" },
	{ 0, 'L', "segment_headers|list_only", "", "", "", "" },
	{ 0, 'N', "sort",
	          "",                  "",
	          "a,d",               "ascend,descend" },
	{ 0, 'Q', "select_num",
	          "",                  "",
	          "f",                 "file" },
	{ 0, 'S', "select_hdr",
	          "",                  "",
	          "e,f",               "exact,file" },
	{ 0, 'T', "suppress|skip",     "", "", "", "" },
	{ 0, 'W', "word2num",
	          "",                  "",
	          "n",                 "nonans" },
	{ 0, 'Z', "transpose",         "", "", "", "" },
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};
#endif  /* !GMTCONVERT_INC_H */
