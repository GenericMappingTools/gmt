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

#ifndef XYZ2GRD_INC_H
#define XYZ2GRD_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "duplicate|multiple_nodes",
	          "d,f,l,m,n,r,s,S,u,z",
	                               "difference,first,low,mean,number,rms,last,stddev,upper,sum",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'D', "netcdf|netCDF|ncheader",
	          "",                      "",
	          "x,y,z,c,d,s,o,n,t,r,v", "xname,yname,zname,cpt|cmap,dname,scale,offset,invalid,title,remark,varname",
		  GMT_TP_STANDARD },
	GMT_G_OUTGRID_KW,
	GMT_I_INCREMENT_KW,
	{ 0, 'S', "swap|byteswap",     "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'Z', "onecol|one_col|convention|flags",
	          "T,B,L,R,a,A,c,u,h,H,i,I,l,L,f,d,x,y,w",
	                               "top,bottom,left,right,ascii,ascii_float,int8|char,uint8|uchar,int16|short,uint16|ushort,int32|int,uint32|uint,int64|long,uint64|ulong,float32|float,float64|double,noxmax,noymax,swap|byteswap",
	          "",                  "",
		  GMT_TP_MULTIDIR },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !XYZ2GRD_INC_H */
