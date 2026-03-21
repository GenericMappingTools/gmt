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

#ifndef SPECTRUM1D_INC_H
#define SPECTRUM1D_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'C', "outputs",
	          "a,c,g,n,o,p,x,y",   "admittance,coherent,gain,noise,sqcoherency,phase,x,y",
	          "",                  "",
		  GMT_TP_MULTIDIR },
	{ 0, 'D', "spacing|sample_dist", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'L', "leave_trend",
	          "h,m",               "remove_mid,remove_mean",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'N', "name",              "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'S', "size",              "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'T', "multifile",         "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'W', "wavelength",        "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !SPECTRUM1D_INC_H */
