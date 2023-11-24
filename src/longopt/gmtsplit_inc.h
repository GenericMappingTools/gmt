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

#ifndef GMTSPLIT_INC_H
#define GMTSPLIT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "azimuth_tolerance|azim_tol", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'C', "course_change",     "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'D', "min_distance|min_dist", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'F', "filter",            "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'N', "multifile|multi",   "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Q', "outputs|fields",
	          "x,y,z,d,h",         "x,y,z,distance|dist,heading|hdg",
	          "",                  "",
		  GMT_TP_MULTIDIR },
	{ 0, 'S', "extended|dist_head", "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GMTSPLIT_INC_H */
