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

#ifndef SPHINTERPOLATE_INC_H
#define SPHINTERPOLATE_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
	          short_directives,    long_directives,
	          short_modifiers,     long_modifiers */
	{ 0, 'D', "",
	          "",                  "",
	          "",                  "" },
	{ 0, 'G', "",
	          "",                  "",
	          "",                  "" },
	{ 0, 'I', "",
	          "",                  "",
	          "",                  "" },
	{ 0, 'Q', "",
	          "",                  "",
	          "",                  "" },
	{ 0, 'T', "",
	          "",                  "",
	          "",                  "" },
	{ 0, 'Z', "",
	          "",                  "",
	          "",                  "" },
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};
#endif  /* !SPHINTERPOLATE_INC_H */
