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

#ifndef PSCONVERT_INC_H
#define PSCONVERT_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = {
	/* separator, short_option, long_option,
		  short_directives,    long_directives,
		  short_modifiers,     long_modifiers,
		  transproc_mask */
	{ 0, 'A', "adjust|crop",
	          "",                  "",
	          "r,u",               "round,no_timestamp",
		  GMT_TP_STANDARD },
	{ 0, 'C', "gs_option",         "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'D', "outdir|out_dir",    "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'E', "dpi",               "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'F', "outfile|out_name|prefix", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'G', "gs_path|ghost_path", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'H', "scale",             "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'I', "resize",
	          "",                  "",
	          "m,s,S",             "margins,size,scale",
		  GMT_TP_STANDARD },
	{ 0, 'L', "listfile|list_file", "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'M', "pslayer",
	          "b,f",               "bg|background,fg|foreground",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'N', "bb_style",
	          "",                  "",
	          "f,g,k,p",           "fade,bg|background,fadecolor,pen",
		  GMT_TP_STANDARD },
	{ 0, 'Q', "anti_aliasing",
	          "g,p,t",             "graphics,geopdf,text",
	          "",                  "",
		  GMT_TP_STANDARD },
	{ 0, 'S', "gs_command",        "", "", "", "", GMT_TP_STANDARD },
	{ 0, 'T', "format|fmt",
	          "b,e,E,f,F,j,g,G,m,t",
		                       "bmp,eps,pageszeps,pdf,multipdf,jpeg,png,transpng,ppm,tiff",
	          "m,q",               "mono|monochrome,quality",
		  GMT_TP_STANDARD },
	{ 0, 'W', "esri|world_file",
	          "",                  "",
	          "a,c,f,g,k,l,n,o,t,u",
		                       "altitude,nocrop|no_crop,fade,gdal,kml,lod,layer,folder,doc,url",
		  GMT_TP_STANDARD },
	{ 0, 'Z', "remove_infile|del_input_ps", "", "", "", "", GMT_TP_STANDARD },
	{ 0, '\0', "", "", "", "", "", 0 }  /* End of list marked with empty option and strings */
};
#endif  /* !PSCONVERT_INC_H */
