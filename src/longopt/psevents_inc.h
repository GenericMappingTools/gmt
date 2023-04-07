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

#ifndef PSEVENTS_INC_H
#define PSEVENTS_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
	          short_directives,  long_directives,
	          short_modifiers,   long_modifiers */
	{ 0, 'T', "time",            "", "", "", "" },
	{ 0, 'A', "polylines",
	          "r,s",             "trajectories,segments",
	          "v",               "value" },
	{ 0, 'C', "cpt",             "", "", "", "" },
	{ 0, 'D', "offset",
	          "j,J",             "justify,shortdiag",
	          "v",               "line" },
	{ 0, 'E', "knots",
	          "s,t",             "symbol,text",
	          "o,O,l,r,p,d,f",   "offset,startoffset,text,rise,plateau,decay,fade" },
	{ 0, 'F', "labels",
	          "",                "",
	          "a,f,j,r,z",       "angle,font,justify,record,zvalue" },
	{ 0, 'H', "boxes",
	          "",                "",
	          "c,g,p,r,s",       "clearance,fill,pen,rounded,shade" },
	{ 0, 'L', "length",          "", "", "", "" },
	{ 0, 'M', "symbols",
	          "i,s,t,v",         "intensity,size,transparency,value",
	          "c",               "coda" },
	{ 0, 'N', "noclip",
	          "c,r",             "clipnorepeat,repeatnoclip",
	          "",                "" },
	{ 0, 'Q', "save",            "", "", "", "" },
	{ 0, 'S', "eventsymbol",     "", "", "", "" },
	{ 0, 'W', "pen",             "", "", "", "" },
	{ 0, 'Z', "symbolcommand",   "", "", "", "" },
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};

#endif  /* !PSEVENTS_INC_H */
