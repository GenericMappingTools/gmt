    /* GMT Common option long-option to short-option translation table
     *
     * The purpose of this table is to facilitate the parsing of the long-option syntax in GMT
     * without having to rewrite 150+ parsers.  The game-plan is instead to examine the given
     * options for long-option syntax and then replace them with the corresponding short-option
     * syntax that the parsers expect. For this to work correctly there needs to be a one-to-one
     * translation for each case. Thus, this aspect assumes all GMT module and common options
     * follow a simple standardized syntax:
     *
     * General short-option syntax:
     *        -<short_option>[<short_directives>][+<short_modifier1>[<argument1>]][+<short_modifier2>[<argument2>]][...]
     *
     * Such an option is then expected to correspond exactly to this long-option format:
     *
     * General long-option syntax:
     *        --<long_option>[=<long_directives>[:<arg>]][+<long_modifier1>[=<arg1>]][+<long_modifier2>[=<arg2>]][...]
     *
     * As we run into module options that DO NOT follow this template we will most likely need
     * to introduce a revised syntax and allow for a backwards compatible parsing option.
     *
     * The items below correspond to the named parameters in the GMT_KEYWORD_DICTIONARY structure:
     *
     * separator:        Indicates if option accepts a series of comma-separated items, e.g., -i0:3,4+s2,8:10,
     *                   where we give several column-ranges, each of which may have their own modifiers. The
     *                   options that behave like that have separator "," which otherwise is set to 0. Another
     *                   separator may be "/", as in -Idx/dy (see GMT_INCREMENT_KW in gmt_constants.h).
     * short_option:     The standard short GMT option letter, e.g., R for -R.
     * long_option:      The corresponding long-format option word, e.g. region for --region
     * short_directives: Comma-separated list of allowable directive letters
     * long_directives:  Comma-separated list of the corresponding words
     * short_modifiers:  Comma-separated list of all known modifier letters for this option
     * long_modifiers:   Comma-separated list of the corresponding modifier words
     *
     * Note 1: All long_directives must be lower-case to avoid conflict with --PAR=value options.
     *         Words cannot be hyphenated as in Julia that would mean a subtraction.
     * Note 2: Option -q is a special case since there is both -qi and -qo but with the same modifiers.
     * Note 3: Some options (such as -B) may be repeated but this does not affect our translation.
     *
     * This table for the common options is assumed to be complete unless proven otherwise!
     *
     * The multi-line per-entry format (line-breaks and horizontal spacing designed to hopefully
     * maximize legibility) is
     *
     * { separator, short_option, long_option,
     *                            short_directives, long_directives,
     *                            short_modifiers,  long_modifiers }
     */

    {   0, 'B', "frame",
                "",                      "",
                "b,g,i,n,o,s,t,w,x,y,z", "box,fill,interior,noframe,pole,subtitle,title,pen,yzfill,xzfill,xyfill" },
    {   0, 'B', "axis",
                "x,y,z",                 "x,y,z",
                "a,f,l,L,p,s,S,u",       "angle,fancy,label,hlabel,prefix,alt_label,alt_hlabel,unit" },
    {   0, 'J', "projection",
                "",                      "",
                "d,a,t,v,w,z,f,k,r",     "d,a,t,v,w,z,f,k,r" },
    {   0, 'R', "region",
                "",                      "",
                "r,u",                   "rectangular,unit" },
    {   0, 'U', "timestamp",
                "",                      "",
                "c,j,o",                 "command,justify,offset" },
    {   0, 'V', "verbosity",
                "q,e,w,t,i,c,d",         "quiet,error,warning,timing,info,compat,debug",
                "",                      "" },
    {   0, 'X', "xshift",
                "a,c,f,r",               "absolute,center,fixed,relative",
                "",                      "" },
    {   0, 'Y', "yshift",
                "a,c,f,r",               "absolute,center,fixed,relative",
                "",                      "" },
    {   0, 'a', "aspatial",              "", "", "", "" },
    {   0, 'b', "binary",
                "",                      "",
                "b,l",                   "big_endian,little_endian" },
    {   0, 'c', "panel",                 "", "", "", "" },
    {   0, 'd', "nodata",
                "i,o",                   "in,out",
                "c",                     "column" },
    {   0, 'e', "find",
                "",                      "",
                "f",                     "file" },
    { ',', 'f', "coltypes",
                "i,o",                   "in,out",
                "",                      "" },
    {   0, 'g', "gap",
                "",                      "",
                "a,c,n,p",               "all,column,negative,positive" },
    {   0, 'h', "header",
                "i,o",                   "in,out",
                "c,d,h,r,t",             "columns,delete,header,remark,title" },
    { ',', 'i', "incols",
                "",                      "",
                "l,d,o,s",               "log10,divide,offset,scale" },
    {   0, 'j', "distance",
                "e,f,g",                 "ellipsoidal,flatearth,spherical",
                "",                      "" },
    {   0, 'l', "legend",
                "",                      "",
                "D,G,H,L,N,S,V,f,g,j,o,p,s,w",
                                         "hline,gap,header,linetext,ncols,size,vline,font,fill,justify,offset,pen,scale,width" },
    {   0, 'n', "interpolation",
                "b,c,l,n",               "bspline,bicubic,linear,nearneighbor",
                "a,b,c,t",               "anti_alias,bc,clip,threshold" },
    { ',', 'o', "outcols",               "", "", "", "" },
    {   0, 'p', "perspective",
                "x,y,z",                 "x,y,z",
                "v,w",                   "view,world" },
    { ',', 'q', "inrows",                /* also note special -qi code in gmtinit_translate_to_short_options()!! */
                "~",                     "invert",
                "a,c,t,s",               "byset,column,bytable,bysegment" },
    { ',', 'q', "outrows",               /* also note special -qo code in gmtinit_translate_to_short_options()!! */
                "~",                     "invert",
                "a,c,t,s",               "byset,column,bytable,bysegment" },
    {   0, 'r', "registration",
                "g,p",                   "gridline,pixel",
                "",                      "" },
    {   0, 's', "skiprows",
                "",                      "",
                "a,r",                   "any,reverse" },
    {   0, 't', "transpercent",
                "",                      "",
                "f,s",                   "fill,stroke" },
    {   0, 'w', "wrap",
                "a,y,w,d,h,m,s,p",       "annual,year,week,day,hour,min,sec,period",
                "c",                     "column" },
    {   0, 'x', "cores",                 "", "", "", "" },
    {   0, ':', "swapcols",
                "i,o",                   "input,output",
                "",                      "" },
    {   0, '\0', "", "", "", "", "" }    /* End of list is marked with empty short-option code and strings */
