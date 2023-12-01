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

#ifndef GRDFILTER_INC_H
#define GRDFILTER_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'D', "distance",     "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'F', "filter",
	          "b,c,g,f,o,m,p,h,l,L,u,U",
 				       "boxcar,cosarch,gaussian,custom,operator,median,mlprob,histogram,minall,minpos,maxall,maxneg",
	          "c,h,l,q,u",         "center,highpass,lower,quantile,upper",
		  GMT_TP_STANDARD },
	GMT_G_OUTGRID_KW,
	GMT_I_INCREMENT_KW,
	{ 0, 'N', "nans",
	          "i,p,r",             "ignore,coregnan,anynan",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'T', "toggle_registration|toggle",         "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRDFILTER_INC_H */
