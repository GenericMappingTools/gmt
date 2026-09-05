/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2026 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#ifndef PSSTEREONET_INC_H
#define PSSTEREONET_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "azimuth",           "", "", "", "", GMT_TP_STANDARD },
	GMT_C_CPT_KW,
	{ 0, 'G', "fill",
	          "p,P",               "bit,bitreverse",
	          "b,f,r",             "bg|background,fg|foreground,dpi",
		  GMT_TP_STANDARD },
	{ 0, 'L', "symbol_pen",        "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'M', "dump",
	          "c,p",               "traces,poles",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'S', "symbol|style",
	          "-,+,a,A,c,C,d,D,e,g,G,h,H,i,I,j,k,l,n,N,p,r,R,s,S,t,T,w,x,y",
	                               "xdash,plus,star,star_area,circle,circle_area,diamond,diamond_area,ellipse,octagon,octagon_area,hexagon,hexagon_area,invtriangle|inverted_tri,invtriangle_area,rotrectangle|rotated_rec,custom,letter,pentagon,pentagon_area,point,rectangle,roundrectangle|roundrect,square,square_area,triangle,triangle_area,wedge,cross,ydash",
	          "t,f,j,s,i,a,r,p",   "text,font,justify,corners,inner,arc,radial,pen",
		  GMT_TP_STANDARD },
	{ 0, 'T', "type",
	          "d,l,p",             "dipdir|dip_direction,line,plane",
	          "r,u",               "rake,upper",
		  GMT_TP_STANDARD },
	GMT_W_PEN_KW,
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !PSSTEREONET_INC_H */
