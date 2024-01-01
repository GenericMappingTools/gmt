/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2024 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "resample",
	          "f,p,m,r,R",               "keeporig,pmfollow,mpfollow,equidistant,exactfit",
	          "l",                       "rhumb",
		  GMT_TP_STANDARD },
	{ 0, 'C', "crossprofile",
	          "",                        "",
	          "a,v,d,f,l,r",             "alternate,wesn,deviant,fixed,left,right",
		  GMT_TP_STANDARD },
	{ 0, 'D', "linefile|dfile",          "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'E', "profile",
	          "",                        "",
	          "a,c,d,g,i,l,n,o,r",       "azimuth,connect,distance,degrees,incr,length,npoints,origin,radius",
		  GMT_TP_STANDARD },
	{ 0, 'F', "critical",
	          "",                        "",
	          "b,n,r,z",                 "balance,negative,rms,zvalue",
		  GMT_TP_STANDARD },
	GMT_G_OUTGRID_KW,
	{ 0, 'N', "noskip",                  "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'S', "stack",
	          "a,m,p,l,L,u,U",           "average,median,mode,lower,lowerpos,upper,upperneg",
	          "a,d,r,s,c",               "values,deviations,residuals,save,envelope",
		  GMT_TP_STANDARD },
	{ 0, 'T', "radius",
	          "",                        "",
	          "e,p",                     "report,replace",
		  GMT_TP_STANDARD },
	{ 0, 'Z', "zonly",                   "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};

#endif  /* !GRDTRACK_INC_H */
