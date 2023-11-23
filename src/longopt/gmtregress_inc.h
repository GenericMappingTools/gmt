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
	          short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "angles|slopes",
	          "",                  "",
	          "f",                 "force",
		  GMT_TP_STANDARD },
	{ 0, 'C', "confidence|confidence_level", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'E', "regression|regression_type",
	          "x,y,o,r",           "x_on_y,y_on_x,ortho|orthogonal,reduced"
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'F', "columns|column_combination",
	          "x,y,m,r,c,z,w",     "x,y,model,residual,symmetrical,standardized,weight",
	          "", "",
	          GMT_TP_MULTIDIR },
	{ 0, 'N', "norm",
	          "1,2,r,w",           "mean_absolute,mean_squared,lms|LMS,rms|RMS",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'S', "skip_outliers|restrict_outliers",
	          "r",                 "reverse",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'T', "range|series",
	          "",                  "",
	          "i,n",               "inverse,number",
		  GMT_TP_STANDARD },
	{ 0, 'W', "weighted",
	          "w,x,y,r",           "weights,sigmax,sigmay,correlations",
	          "", "",
	          GMT_TP_MULTIDIR },
	{ 0, 'Z', "limit",             "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GMTREGRESS_INC_H */
