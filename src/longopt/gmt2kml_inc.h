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

#ifndef GMT2KML_INC_H
#define GMT2KML_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
	          short_directives,      long_directives,
	          short_modifiers,       long_modifiers */
	{ 0, 'A', "altitude_mode",
	          "a,g,s",               "absolute,relative_surface,relative_floor",
	          "",                    "" },
	{ 0, 'C', "cpt|cmap",            "", "", "", "" },
	{ 0, 'D', "descript",            "", "", "", "" },
	{ 0, 'E', "line_render",
	          "",                    "",
	          "e,s",                 "extrude,connect" },
	{ 0, 'F', "feature|feature_type",
	          "e,s,t,l,p,w",         "event,symbol,timespan,line,polygon,wiggle",
	          "",                    "" },
	{ 0, 'G', "draw_color|fill",
	          "",                    "",
	          "f,n",                 "fill,font" },
	{ 0, 'I', "icon",                "", "", "", "" },
	{ 0, 'K', "not_over",            "", "", "", "" },
	{ 0, 'L', "extended|extra_data", "", "", "", "" },
	{ 0, 'N', "name|feature_name",
	          "t",                   "trailer",
	          "",                    "" },
	{ 0, 'O', "overlay",             "", "", "", "" },
	{ 0, 'Q', "wiggle",
	          "a,i,s",               "half_circle,fixed,scale",
	          "",                    "" },
	{ 0, 'S', "scale",
	          "c,n",                 "icon,label",
	          "",                    "" },
	{ 0, 'T', "title",               "", "", "", "" },
	{ 0, 'W', "pen",
	          "",                    "",
	          "c",                   "color" },
	{ 0, 'Z', "attrib|attributes",
	          "",                    "",
	          "a,f,l,o,v",           "altitude,fade,detail,open,invisible" },
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};
#endif  /* !GMT2KML_INC_H */
