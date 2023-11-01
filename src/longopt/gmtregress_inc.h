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

#ifndef GMTREGRESS_INC_H
#define GMTREGRESS_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
	          short_directives,    long_directives,
	          short_modifiers,     long_modifiers */
	{ 0, 'A', "full_range|all_slopes",
	          "",                  "",
	          "f",                 "force" },
	{ 0, 'C', "confidence|confidence_level", "", "", "", "" },
	{ 0, 'E', "regression|regression_type",
	          "x,y,o,r",           "x_on_y,y_on_x,ortho|orthogonal,reduced"
	          "",                  "" },
	{ 0, 'F', "columns|column_combination", "", "", "", "" },
	{ 0, 'N', "norm",
	          "1,2,r,w",           "mean_absolute,mean_squared,lms|LMS,rms|RMS",
	          "",                  "" },
	{ 0, 'S', "restrict_outliers|restrict",
	          "r",                 "reverse",
	          "",                  "" },
	{ 0, 'T', "range|series",
	          "",                  "",
	          "i,n",               "inverse,number" },
	{ 0, 'W', "weighted",          "", "", "", "" },
	{ 0, 'Z', "limit",             "", "", "", "" },
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};
#endif  /* !GMTREGRESS_INC_H */
