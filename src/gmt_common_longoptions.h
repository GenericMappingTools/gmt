    /* GMT Common option long-option to short-option translation table
     *
     * The purpose of this table is to facilitate the parsing of the long-option syntax in GMT
     * without having to rewrite 150+ parsers.  The game-plan is instead to examine the given
     * options for long-option syntax and then replace them with the corresponding short-option
     * syntax that the parsers expect. For this to work correctly there needs to be a one-to-one
     * translation (at least in the long-to-short direction) for each case.
     *
     * This translation process assumes all GMT module and common options follow a standardized syntax:
     *
     * General short-option syntax:
     *        -<short_option>[<short_directives>][+<short_modifier1>[<argument1>]][+<short_modifier2>[<argument2>]][...]
     *
     * Such an option is then expected to correspond exactly to this long-option format:
     *
     * General long-option syntax:
     *        --<long_option>[=<long_directives>[:<arg>]][+<long_modifier1>[:<arg1>]][+<long_modifier2>[:<arg2>]][...]
     *
     * As we run into module options that DO NOT follow this template we will most likely need
     * to introduce a revised syntax and allow for a backwards compatible parsing option.
     *
     * The items below correspond to the named parameters in the GMT_KEYWORD_DICTIONARY structure:
     *
     * separator:        Indicates if option accepts a series of comma-separated items, e.g., -i0:3,4+s2,8:10,
     *                   where we give several column-ranges, each of which may have their own modifiers. The
     *                   options that behave like that have separator "," which otherwise is set to 0. Another
     *                   separator may be "/", as in -Idx/dy (see GMT_I_INCREMENT_KW in gmt_constants.h).
     * short_option:     The standard short GMT option letter, e.g., R for -R.
     * long_option:      The corresponding long-format option word, e.g. region for --region
     * short_directives: Comma-separated list of allowable directive letters
     * long_directives:  Comma-separated list of the corresponding words
     * short_modifiers:  Comma-separated list of all known modifier letters for this option
     * long_modifiers:   Comma-separated list of the corresponding modifier words
     * transproc_mask:   Indicates via bitwise-OR of various GMT_TP_???? bitflags any unusual
     *                   translation processing used by the particular GMT_KEYWORD_DICTIONARY
     *                   entry, e.g., GMT_TP_MULTIDIR indicates multi-directive support. Specify
     *                   GMT_TP_STANDARD to indicate standard translation processing with
     *                   no unusual attributes.
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
     *                            short_modifiers,  long_modifiers,
     *                            transproc_mask }
     *
     * Aliases may be specified within the above-defined entry structure for any <long_option>,
     * <long_directives> or <long_modifierN> via the '|' character. For example, you could specify
     * an alias for the "--region" long option and a single alias for its "rectangular"
     * long-modifier via the entry
     *
     *     {   0, 'R', "region|limits",
     *                 "",                      "",
     *                 "r,u",                   "rectangular|rect,unit",
     *                 GMT_TP_STANDARD },
     *
     * Blank spaces (but not tabs or other whitespace) around the '|' character are permitted
     * for legibility as desired, e.g., "region | limits".
     *
     */

    {   0, 'B', "frame",
                "",                      "",
                "b,g,i,n,o,s,t,w,x,y,z", "box,fill,interior,noframe,pole,subtitle,title,pen,yzfill,xzfill,xyfill",
                GMT_TP_STANDARD },
    {   0, 'B', "axis",
                "x,y,z",                 "x,y,z",
                "a,f,l,L,p,s,S,u",       "angle,fancy,label,hlabel,prefix,alt_label,alt_hlabel,unit",
                GMT_TP_STANDARD },
    {   0, 'J', "projection|proj",
                "",                      "",
                "d,a,t,v,w,z,f,k,r",     "d,a,t,v,w,z,f,k,r",
                GMT_TP_STANDARD },
    {   0, 'R', "region|limits",
                "",                      "",
                "r,u",                   "rectangular|rect,unit",
                GMT_TP_STANDARD },
    {   0, 'U', "timestamp",
                "",                      "",
                "c,j,o,t",               "command,justify,offset,text",
                GMT_TP_STANDARD },
    {   0, 'V', "verbosity",
                "q,e,w,t,i,c,d",         "quiet,error,warning,timing,info,compat,debug",
                "",                      "",
                GMT_TP_STANDARD },
    {   0, 'X', "xshift",
                "a,c,f,r",               "absolute,center,fixed,relative",
                "",                      "",
                GMT_TP_STANDARD },
    {   0, 'Y', "yshift",
                "a,c,f,r",               "absolute,center,fixed,relative",
                "",                      "",
                GMT_TP_STANDARD },
    {   0, 'a', "aspatial",              "", "", "", "", GMT_TP_STANDARD },
    {   0, 'b', "binary",
                "i,o",                   "in,out",
                "b,l",                   "bigendian,littleendian",
                GMT_TP_STANDARD },
    {   0, 'c', "panel",                 "", "", "", "", GMT_TP_STANDARD },
    {   0, 'd', "nodata",
                "i,o",                   "in,out",
                "c",                     "column",
                GMT_TP_STANDARD },
    {   0, 'e', "find",
                "",                      "",
                "f",                     "file",
                GMT_TP_STANDARD },
    { ',', 'f', "coltypes",
                "i,o",                   "in,out",
                "",                      "",
                GMT_TP_STANDARD },
    {   0, 'g', "gap",
                "x,y,z,d,X,Y,D",         "x,y,z,distance,xproj,yproj,distanceproj",
                "a,c,n,p",               "all,column,negative,positive",
                GMT_TP_STANDARD },
    {   0, 'h', "header",
                "i,o",                   "in,out",
                "c,d,m,r,t",             "columns,delete,header,remark,title",
                GMT_TP_STANDARD },
    { ',', 'i', "incols",
                "",                      "",
                "l,d,o,s",               "log10,divide,offset,scale",
                GMT_TP_STANDARD },
    {   0, 'j', "metric|spherical|distcalc",
                "e,f,g",                 "ellipsoidal,flatearth,spherical",
                "",                      "",
                GMT_TP_STANDARD },
    {   0, 'l', "legend",
                "",                      "",
                "D,G,H,L,N,S,V,f,g,j,o,p,s,w",
                                         "hline,gap,header,linetext,ncols,size,vline,font,fill,justify,offset,pen,scale,width",
                GMT_TP_STANDARD },
    {   0, 'n', "interpolation",
                "b,c,l,n",               "bspline,bicubic,linear,nearneighbor",
                "a,b,c,t",               "anti_alias,bc,clip,threshold",
                GMT_TP_STANDARD },
    { ',', 'o', "outcols",
                "",                      "",
                "l,d,o,s",               "log10,divide,offset,scale",
                GMT_TP_STANDARD },
    {   0, 'p', "perspective",
                "x,y,z",                 "x,y,z",
                "v,w",                   "view,world",
                GMT_TP_STANDARD },

    /* do not add any long-options aliases to the "inrows" and "outrows" entries
       defined here or you will break the specialized code which handles them! */
    { ',', 'q', "inrows",                /* also note special -qi code in gmtinit_translate_to_short_options()!! */
                "~",                     "invert",
                "a,c,t,s",               "byset,column,bytable,bysegment",
                GMT_TP_STANDARD },
    { ',', 'q', "outrows",               /* also note special -qo code in gmtinit_translate_to_short_options()!! */
                "~",                     "invert",
                "a,c,t,s",               "byset,column,bytable,bysegment",
                GMT_TP_STANDARD },

    {   0, 'r', "registration",
                "g,p",                   "gridline,pixel",
                "",                      "",
                GMT_TP_STANDARD },
    {   0, 's', "skiprows",
                "",                      "",
                "a,r",                   "any,reverse",
                GMT_TP_STANDARD },
    {   0, 't', "transparency",
                "",                      "",
                "f,s",                   "fill,stroke",
                GMT_TP_STANDARD },
    {   0, 'w', "wrap",
                "y,a,w,d,h,m,s,c",       "year,annual,week,day,hour,min,sec,custom",
                "c",                     "column",
                GMT_TP_STANDARD },
    {   0, 'x', "cores",                 "", "", "", "", GMT_TP_STANDARD },
    {   0, ':', "swapcols",
                "i,o",                   "input,output",
                "",                      "",
                GMT_TP_STANDARD },
    {   0, '\0', "", "", "", "", "", GMT_TP_STANDARD }    /* End of list is marked with empty short-option code and strings */
