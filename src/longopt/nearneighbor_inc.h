/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#ifndef NEARNEIGHBOR_INC_H
#define NEARNEIGHBOR_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
	          short_directives,    long_directives,
	          short_modifiers,     long_modifiers */
	{ 0, 'E', "",
	          "",                  "",
	          "",                  "" },
	{ 0, 'G', "",
	          "",                  "",
	          "",                  "" },
	{ 0, 'I', "",
	          "",                  "",
	          "",                  "" },
	{ 0, 'N', "",
	          "",                  "",
	          "",                  "" },
	{ 0, 'S', "",
	          "",                  "",
	          "",                  "" },
	{ 0, 'W', "",
	          "",                  "",
	          "",                  "" },
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};
#endif  /* !NEARNEIGHBOR_INC_H */
