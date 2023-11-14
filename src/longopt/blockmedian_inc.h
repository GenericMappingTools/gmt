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

#ifndef BLOCKMEDIAN_INC_H
#define BLOCKMEDIAN_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for all the block* modules */
    /* separator, short_option, long_option,
                                    short_directives, long_directives,
                                    short_modifiers,  long_modifiers */
    { 0, 'A', "fields",   "", "", "", "" },
    { 0, 'C', "center",   "", "", "", "" },
    { 0, 'E', "extend",
                  "b,r,s",    "boxwhisker,record,source",
                  "l,h",      "lower,higher" },
    GMT_G_OUTGRID_KW,
    GMT_I_INCREMENT_KW,
    { 0, 'Q', "quick",    "", "", "", "" },
    { 0, 'T', "quantile", "", "", "", "" },
    { 0, 'W', "weights",
                  "i,o",      "in,out",
                  "s",        "sigma" },
    { 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};

#endif  /* !BLOCKMEDIAN_INC_H */
