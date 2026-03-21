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

#ifndef TREND1D_INC_H
#define TREND1D_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'C', "condition|condition_number", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'F', "output",
	          "x,y,m,r,w,p,P,c",   "x,y,model,residual,weight,polynomial,npolynomial,chebyshev",
	          "",                  "",
		  GMT_TP_MULTIDIR },
	{ 0, 'I', "confidence|conf_level", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'N', "model",
	          "",                  "",
	          "l,o,r",             "length,origin,robust",
		  GMT_TP_STANDARD },
	{ 0, 'W', "weights",
	          "",                  "",
	          "s",                 "uncertainties",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !TREND1D_INC_H */
