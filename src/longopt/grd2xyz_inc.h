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

#ifndef GRD2XYZ_INC_H
#define GRD2XYZ_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'C', "rowcol|row_col",
	          "",                  "",
	          "f,i",               "one|fortran,indexz",
		  GMT_TP_STANDARD },
	{ 0, 'L', "single",
	          "c,r,x,y",           "col|column,row,x,y",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'T', "stl",
	          "a,b",               "ascii|ASCII,binary",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'W', "weight",
	          "a",                 "area",
	          "u",                 "unit",
		  GMT_TP_STANDARD },
	{ 0, 'Z', "onecol|one_col|ordering",
	          "T,B,L,R,a,c,u,h,H,i,I,l,L,f,d,x,y,w",
				       "top,bottom,left,right,ascii|ASCII,int8|char,uint8|uchar,int16|short,uint16|ushort,int32|int,uint32|uint,int64|long,uint64|ulong,float32|float,float64|double,noxmax,noymax,swap|byteswap",
	          "",                  "",
		  GMT_TP_MULTIDIR },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !GRD2XYZ_INC_H */
