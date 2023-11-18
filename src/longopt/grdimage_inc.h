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

#ifndef GRDIMAGE_INC_H
#define GRDIMAGE_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
/* ?? -A not possible because of = usage, e.g., -Aout_img=driver ? */
	GMT_C_CPT_KW,
	{ 0, 'D', "inimage",
                  "r",                 "region",
                  "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'E', "dpi",
                  "i",                 "psdeviceres",
                  "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'G', "bitcolor",
                  "",                  "",
                  "b,f",               "background,foreground",
		  GMT_TP_STANDARD },
	{ 0, 'I', "intensity",
                  "",                  "",
                  "a,d,m,n",           "azimuth,default,ambient,intensity",
		  GMT_TP_STANDARD },
	{ 0, 'M', "monochrome",        "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'N', "noclip",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Q', "alphacolor|alpha_color|nan_alpha",
                  "",                  "",
                  "z",                 "gridvalue",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};

#endif  /* !GRDIMAGE_INC_H */
