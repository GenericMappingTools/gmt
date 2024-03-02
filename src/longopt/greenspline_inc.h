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

#ifndef GREENSPLINE_INC_H
#define GREENSPLINE_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "gradient",
	          "",                  "",
	          "f",                 "format",
		  GMT_TP_STANDARD },
	{ 0, 'C', "approx_fit|approximate",
	          "n,r,v",             "largest,ratio,variance",
	          "c,f,i,n",           "cumulative,file,incremental,no_surface",
		  GMT_TP_STANDARD },
	{ 0, 'D', "header|metadata",
	          "",                  "",
	          "c,d,n,o,r,s,t,v,x,y,z",
	                               "cpt|cmap,dname,invalid,offset,remark,scale,title,varname,xname,yname,zname",
		  GMT_TP_STANDARD },
	{ 0, 'E', "misfit",
	          "",                  "",
	          "r",                 "report",
		  GMT_TP_STANDARD },
	GMT_G_OUTGRID_KW,
	{ 0, 'I', "inc|increment|spacing", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'L', "detrend|leave_trend",
	          "t,r",               "leastsquares,residuals",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'N', "nodes",             "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Q', "derivative|dir_derivative|vector", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'S', "splines",
	          "c,l,p,q,r,t",       "min_scurvature,linear,min_pcurvature,ctensionA,rtension,ctensionB",
	          "e,n",               "error,npoints",
		  GMT_TP_STANDARD },
	{ 0, 'T', "maskgrid|mask",     "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'W', "uncertainties",
	          "w",                 "weights",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'Z', "distmode|mode",     "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GREENSPLINE_INC_H */
