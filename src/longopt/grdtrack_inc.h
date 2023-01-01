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

#ifndef GRDTRACK_INC_H
#define GRDTRACK_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
	          short_directives,          long_directives,
	          short_modifiers,           long_modifiers */
	{ 0, 'A', "resample",
	          "f,p,m,r,R",               "keeporig,pmfollow,mpfollow,equidistant,exactfit",
	          "l",                       "rhumb" },
	{ 0, 'C', "crossprofile",
	          "",                        "",
	          "a,v,d,f,l,r",             "alternate,wesn,deviant,fixed,left,right" },
	{ 0, 'D', "linefile",                "", "", "", "" },
	{ 0, 'E', "profile",
	          "",                        "",
	          "a,c,d,g,i,l,n,o,r",       "azimuth,connect,distance,degrees,incr,length,npoints,origin,radius" },
	{ 0, 'F', "critical",
	          "",                        "",
	          "b,n,r,z",                 "balance,negative,rms,zvalue" },
	{ 0, 'G', "grid",
	          "",                        "",
	          "l",                       "list" },
	{ 0, 'N', "noskip",                  "", "", "", "" },
	{ 0, 'S', "stack",
	          "a,m,p,l,L,u,U",           "average,median,mode,lower,lowerpos,upper,upperneg",
	          "a,d,r,s,c",               "values,deviations,residuals,save,envelope" },
	{ 0, 'T', "radius",
	          "",                        "",
	          "e,p",                     "report,replace" },
	{ 0, 'Z', "zonly",                   "", "", "", "" },
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};

#endif  /* !GRDTRACK_INC_H */
