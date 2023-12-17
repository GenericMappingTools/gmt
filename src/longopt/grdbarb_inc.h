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

#ifndef GRDBARB_INC_H
#define GRDBARB_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "speed_theta",       "", "", "", "", GMT_TP_STANDARD },
	GMT_C_CPT_KW,
	{ 0, 'G', "fill",              "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "increment|inc",
	          "x",                 "xmult",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'N', "noclip",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Q', "barbs",
	          "",                  "",
	          "a,g,p,j,s,w",       "angle,fill,pen,justify,longspeed,width",
		  GMT_TP_STANDARD },
	{ 0, 'T', "adjust_azimuths",   "", "", "", "", GMT_TP_STANDARD },
	GMT_W_PEN_KW,
	{ 0, 'Z', "theta_azimuths",    "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRDBARB_INC_H */
