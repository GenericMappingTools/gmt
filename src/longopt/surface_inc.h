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

#ifndef SURFACE_INC_H
#define SURFACE_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "aspect",
		  "m",       "middle",
		  "",        "",
		  GMT_TP_STANDARD },
	{ 0, 'C', "convergence",     "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'D', "breakline",
		  "",        "",
		  "z",       "zvalue",
		  GMT_TP_STANDARD },
	GMT_I_INCREMENT_KW,
	{ 0, 'L', "limit",
		  "l,u",     "lower,upper",
		  "",        "",
		  GMT_TP_STANDARD },
	{ 0, 'M', "maskradius|mask", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'N', "maxiterations",   "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Q', "quicksize",
		  "r",       "region",
		  "",        "",
		  GMT_TP_STANDARD },
	{ 0, 'S', "searchradius",    "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'T', "tension",
		  "b,i",     "boundary,interior",
		  "",        "",
		  GMT_TP_STANDARD },
	{ 0, 'W', "logfile",         "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Z', "relax",           "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};

#endif  /* !SURFACE_INC_H */
