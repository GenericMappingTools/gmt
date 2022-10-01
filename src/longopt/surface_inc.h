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

#ifndef SURFACE_INC_H
#define SURFACE_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option, short_directives, long_directives, short_modifiers, long_modifiers */
	GMT_INCREMENT_KW,       /* Defined in gmt_constants.h since not a true GMT common option (but almost) */
	{ 0, 'A', "aspect",
		  "m",       "middle",
		  "",        "" },
	{ 0, 'C', "convergence",     "", "", "", "" },
	{ 0, 'D', "breakline",
		  "",        "",
		  "z",       "zvalue" },
	{ 0, 'L', "limit",
		  "l,u",     "lower,upper",
		  "",        "" },
	{ 0, 'M', "maskradius|mask", "", "", "", "" },
	{ 0, 'N', "maxiterations",   "", "", "", "" },
	{ 0, 'Q', "quicksize",
		  "r",       "region",
		  "",        "" },
	{ 0, 'S', "searchradius",    "", "", "", "" },
	{ 0, 'T', "tension",
		  "b,i",     "boundary,interior",
		  "",        "" },
	{ 0, 'W', "logfile",         "", "", "", "" },
	{ 0, 'Z', "relax",           "", "", "", "" },
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};

#endif  /* !SURFACE_INC_H */
