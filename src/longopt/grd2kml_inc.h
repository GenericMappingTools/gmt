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

#ifndef GRD2KML_INC_H
#define GRD2KML_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "mode",
	          "a,g,s",             "absolute,ground,seafloor",
	          "",                  "",
		  GMT_TP_STANDARD },
	GMT_C_CPT_KW,
	{ 0, 'E', "url",               "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'F', "filter",
	          "b,c,g,m",           "boxcar,cosarch,gaussian,median",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'H', "subpixel",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "illumination|intensity|shading",
	          "",                  "",
	          "a,d,m,n",           "azimuth,default,ambient,nargs",
		  GMT_TP_STANDARD },
	{ 0, 'L', "tilesize|tile_size", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'N', "prefix",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'S', "extra|extralayers|extra_layers", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'T', "title",             "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'W', "pen|contours",
	          "",                  "",
	          "s",                 "scale",
		  GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRD2KML_INC_H */
