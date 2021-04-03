	/* GMT Common option long-option to short-option translation table
	 *
	 * General short-option syntax:
	 *		-<short_option>[<short_directives>][+<short_modifiers>[<argument>]]
	 * General long-option syntax:
	 *		--<long_option>[=[<long_directives>:]<arg>][+<long_modifier1>[=<arg1>]][+<long_modifier2>[=<arg2>]]...
	 *
	 * The items below correspond to the named parameters in the GMT_KEYWORD_DICTIONARY structure:
	 */
	/* separator, short_option, long_option, short_directives, long_directives, short_modifiers, long_modifiers */
	{   0, 'B', "frame",         "",        "",                                         "b,g,n,o,t,s",				"box,fill,noframe,obliquepole,title,subtitle" },
	{   0, 'B', "axis",          "x,y,z",   "x,y,z",                                    "a,f,l,L,p,s,S,u",			"angle,fancy,label,Label,prefix,second_label,Second_label,unit" },
	{   0, 'J', "projection",    "",        "",                                         "",         				""},
	{   0, 'R', "region",        "",        "",                                         "r,u",        				"rectangular,unit"},
	{   0, 'U', "timestamp",     "",        "",                                         "c,j,o",    				"command,justify,offset"},
	{   0, 'V', "verbosity",     "",        "",                                         "",         				""},
	{   0, 'X', "xshift",        "a,c,f,r", "absolute,center,fixed,relative",           "",         				""},
	{   0, 'Y', "yshift",        "a,c,f,r", "absolute,center,fixed,relative",           "",         				""},
	{   0, 'a', "aspatial",      "",        "",                                         "",         				""},
	{   0, 'b', "binary",        "",        "",                                         "b,l",      				"bigendian,littleendian"},
	{   0, 'c', "panel",         "",        "",                                         "",      				    ""},
	{   0, 'd', "nodata",        "i,o",     "in,out",                                   "",         				""},
	{   0, 'e', "find",          "",        "",                                         "f",        				"file"},
	{ ',', 'f', "coltypes",      "i,o",     "in,out",                                   "",        					""},
	{   0, 'g', "gap",           "",        "",                                         "n,p",        				"negative,positive"},
	{   0, 'h', "header",        "i,o",     "in,out",                                   "c,d,r,t", 					"columns,delete,remark,title"},
	{ ',', 'i', "incols",        "",        "",                                         "l,o,s",   					"log10,offset,scale"},
	{   0, 'j', "spherical",     "e,f,g",   "ellipsoidal,flatearth,greatcircle",        "",   			    		""},
	{   0, 'l', "legend",        "",        "",                                         "D,G,H,L,N,S,V,f,g,j,o,p,s,w",   	"drawline,gap,header,linetext,ncols,size,vertline,font,fill,justify,offset,pen,scale,width"},
	{   0, 'n', "interpolation", "b,c,l,n", "bspline,bicubic,linear,nearestneighbor",   "a,b,c,t",     				"antialias,bc,clip,threshold"},
	{ ',', 'o', "outcols",       "",        "",                                         "",        					""},
	{   0, 'p', "perspective",   "x,y,z",   "x,y,z",                                    "v,w",     					"view,world"},
	{ ',', 'q', "inrows",        "~",       "invert",                                   "a,c,f,s",   				"perset,column,perfile,persegment"},	/* Actually -qi */
	{ ',', 'q', "outrows",       "~",       "invert",                                   "a,c,f,s",   				"perset,column,perfile,persegment"},	/* Actually -qo */
	{   0, 'r', "registration",  "g,p",     "gridline,pixel",                           "",        					""},
	{   0, 's', "skiprows",      "",        "",                                         "a,r",     					"any,reverse"},
	{   0, 't', "transparency",  "",        "",                                         "",        				    ""},
	{   0, 'w', "wrap",  "a,y,w,d,h,m,s,p", "annual,year,week,day,hour,min,sec,period", "c",        			    "column"},
	{   0, 'x', "cores",         "",        "",                                         "",        					""},
	{   0, '\0', "",             "",        "",                                         "",        					""}	/* End of list is marked with empty short-option code and strings */
