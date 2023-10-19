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

#ifndef MOVIE_INC_H
#define MOVIE_INC_H

/* Translation table from long to short module options, directives and modifiers */

static struct GMT_KEYWORD_DICTIONARY module_kw[] = { /* Local options for this module */
	/* separator, short_option, long_option,
	          short_directives,    long_directives,
	          short_modifiers,     long_modifiers */
	{ 0, 'C', "canvas",                "", "", "", "" },
	{ 0, 'N', "name",                  "", "", "", "" },
	{ 0, 'T', "frames",
	          "",                      "",
	          "n,p,s,w,W",             "nframes,tagwidth,first,wordsepall,wordseptab" },
	{ 0, 'A', "audio",
                  "",                      "",
                  "e",                     "exact" },
	{ 0, 'D', "displayrate",           "", "", "", "" },
	{ 0, 'E', "title",
	          "",                      "",
	          "d,f,g",                 "duration,fadetime,fadecolor" },
	{ 0, 'F', "format",
	          "",                      "",
	          "l,o,s,t,v",              "loop,encode,stride,transparent,view" },
	{ 0, 'G', "fill",
	          "",                      "",
	          "p",                     "pen" },
	{ 0, 'H', "subpixel",              "", "", "", "" },
	{ 0, 'I', "include",               "", "", "", "" },
	{ 0, 'K', "fade",
	          "",                      "",
	          "f,g,p",                 "length,color,preserve" },
	{ 0, 'L', "label",
	          "e,s,f,p,c,t",           "time,string,frame,percent,col,textcol",
	          "s,c,f,g,h,j,o,p,r,t",   "scale,clearance,font,fill,shade,refpoint,offset,pen,rounded,format" },
	{ 0, 'M', "master",
	          "f,m,l",                 "first,middle,last",
	          "r,v",                   "dotsperunit|dpu,view" },
	{ 0, 'P', "progress",
	          "a,b,c,d,e,f",           "pie,wheel,arrow,line,gauge,axis",
	          "a,f,g,G,j,o,p,P,s,t,w", "annotate,font,mfill,sfill,justify,offset,mpen,spen,scale,format,width" },
	{ 0, 'Q', "debug",
	          "s",                     "scripts",
	          "",                      "" },
	{ 0, 'S', "static",
	          "b,f",                   "bg,fg",
	          "",                      "" },
	{ 0, 'W', "workdir",               "", "", "", "" },
	{ 0, 'Z', "delete",
	          "s",                     "scripts",
	          "",                      "" },
	{ 0, '\0', "", "", "", "", ""}  /* End of list marked with empty option and strings */
};

#endif  /* !MOVIE_INC_H */
