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

#ifndef GRDFFT_INC_H
#define GRDFFT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "azimuth|azim",      "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'C', "zcontinue|upward",  "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'D', "differentiate|dfdz", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'E', "power_spectrum",
	          "r,x,y",             "radial,x,y",
	          "w,n",               "wavelength,normalize",
		  GMT_TP_STANDARD },
	{ 0, 'F', "filter",
	          "r,x,y",             "isotropic,x,y",
	          "",                  "",
		  GMT_TP_STANDARD },
	GMT_G_OUTGRID_KW,
	{ 0, 'I', "integrate",
	          "g",                 "gravity",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'N', "dimensions|inquire",
	          "a,f,m,r,s",         "accurate,actual,low_memory,rapid,show",
	          "d,a,h,l,e,m,n,v,w,z",
			               "detrend,remove_mean,remove_mid,leave_alone,edge_point,edge_mirror,no_extend,verbose,suffix,complex",
		  GMT_TP_STANDARD },
	{ 0, 'Q', "no_wavenum_ops",    "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'S', "scale",
	          "d",                 "deflection",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRDFFT_INC_H */
