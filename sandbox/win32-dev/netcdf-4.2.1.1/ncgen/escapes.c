/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/escapes.c,v 1.5 2010/04/04 19:39:44 dmh Exp $
 *********************************************************************/

#include "includes.h"
#include "ConvertUTF.h"

/* Forward*/
static void initdecodify(void);

/*
 * "Expands" valid escape sequences in yystring (read by lex) into the
 * apropriate characters in termstring.  For example, the two character
 * sequence "\t" in yystring would be converted into a single tab character
 * in termstring.  On return, termstring is properly terminated.
 */

void
expand_escapes(
     Bytebuffer *s, /* fill with contents of yytext, with escapes expanded */
     char *yytext,
     int yyleng)
{
    char *t, *endp;
    yytext[yyleng-1]='\0';	/* don't copy quotes */
    /* expand "\" escapes, e.g. "\t" to tab character  */
    t = yytext+1;
    while(*t) {
	if (*t == '\\') {
	    t++;
	    switch (*t) {
	      case 'a':
		bbAppend(s,'\007'); t++; /* will use '\a' when STDC */
		break;
	      case 'b':
		bbAppend(s,'\b'); t++;
		break;
	      case 'f':
		bbAppend(s,'\f'); t++;
		break;
	      case 'n':
		bbAppend(s,'\n'); t++;
		break;
	      case 'r':
		bbAppend(s,'\r'); t++;
		break;
	      case 't':
		bbAppend(s,'\t'); t++;
		break;
	      case 'v':
		bbAppend(s,'\v'); t++;
		break;
	      case '\\':
		bbAppend(s,'\\'); t++;
		break;
	      case '?':
		bbAppend(s,'\177'); t++;
		break;
	      case '\'':
		bbAppend(s,'\''); t++;
		break;
	      case '\"':
		bbAppend(s,'\"'); t++;
		break;
	      case 'x':
		t++; /* now t points to one or more hex digits */
		bbAppend(s,(char) strtol(t, &endp, 16));
		t = endp;
		break;
	      case '0':
	      case '1':
	      case '2':
	      case '3':
	      case '4':
	      case '5':
	      case '6':
	      case '7':
		/* t now points to octal digits */
		bbAppend(s,(char) strtol(t, &endp, 8));
		t = endp;
		break;
	      default:
		bbAppend(s,*t); t++;
		break;
	    }
	} else {
	    bbAppend(s,*t); t++;
	}
    }
    bbNull(s);
    bbSetlength(s,strlen(bbContents(s)));
    return;
}

/*
 * Replace escaped chars in CDL representation of name such as
 * 'abc\:def\ gh\\i' with unescaped version, such as 'abc:def gh\i'.
 */
/* ?? This seems redundant over expand_escapes*/
void
deescapify(char* name)
{
    const char *cp = name;
    char *sp;
    size_t len = strlen(name);
    char *newname;

    if(strchr(name, '\\') == NULL)
	return;

    newname = (char *) emalloc(len + 1);
    cp = name;
    sp = newname;
    while(*cp != '\0') { /* delete '\' chars, except change '\\' to '\' */
	switch (*cp) {
	case '\\':
	    if(*(cp+1) == '\\') {
		*sp++ = '\\';
		cp++;
	    }
	    break;
	default:
	    *sp++ = *cp;
	    break;
	}
	cp++;
    }
    *sp = '\0';
    /* ASSERT(strlen(newname) <= strlen(name)); */
    strncpy(name, newname, len+1); /* watch out for trailing null*/
    efree(newname);
    return;
}

/*
Given a character c, fill s with the character suitably escaped.
E.g. c = '\t' => s="\t"
Caller must ensure enough space
Currently does not handle unicode
Returns s as it result.
*/

char*
escapifychar(unsigned int c, char* s0, int quote)
{
    char* s = s0;
    if(c == '\\') {
	*s++ = '\\'; *s++='\\';
    } else if(c == quote) {
	*s++ = '\\'; *s++=(char)quote;
    } else if(c >= ' ' && c != '\177') {
	*s++ = (char)c;
    } else {
        switch (c) {
	case '\b': strcpy(s,"\\b"); s+=2; break;
	case '\f': strcpy(s,"\\f"); s+=2; break;
	case '\n': strcpy(s,"\\n"); s+=2; break;
	case '\r': strcpy(s,"\\r"); s+=2; break;
	case '\t': strcpy(s,"\\t"); s+=2; break;
	case '\v': strcpy(s,"\\v"); s+=2; break;
	default: {
	    unsigned int oct1 = (c & 007);
	    unsigned int oct2 = ((c >> 3) & 007);
	    unsigned int oct3 = ((c >> 6) & 003);
	    *s++ = '\\';
	    *s++ = oct3 + '0';
	    *s++ = oct2 + '0';
	    *s++ = oct1 + '0';
	} break;
	}
    }
    *s = '\0';
    return s0;
}

/* Return a pool string that is s0 with all characters*/
/* ecaped that require it.  The resulting string is not*/
/* surrounded by quotes.*/
/* Since the string might actually contain nulls, specify the length.*/

char*
escapify(char* s0, int quote, size_t len)
{
    int i;
    char* result;
    result = poolalloc(1+4*len); /* overkill to support maximal expansion*/
    result[0] = '\0';
    for(i=0;i<len;i++) {
	char tmp[8];
	escapifychar((unsigned int)s0[i],tmp,quote);
        strcat(result,tmp);
    }
    return result;        
}

char*
escapifyname(char* s0)
{
    return escapify(s0,'"',strlen(s0));
}

void
cquotestring(Bytebuffer* databuf, char quote)
{
    char* escaped = escapify(bbContents(databuf),'"',bbLength(databuf));
    bbClear(databuf);
    bbAppend(databuf,quote);
    bbCat(databuf,escaped);
    bbAppend(databuf,quote);
}

/*
 * Replace special chars in name so it can be used in C and Fortran
 * variable names without causing syntax errors.  Here we just replace
 * each "-" in a name with "_MINUS_", each "." with "_PERIOD_", etc.
 * For bytes with high bit set, from UTF-8 encoding of Unicode, just
 * replace with "_xHH", where each H is the appropriate hex digit.
 * However, if the utf flag is set, then just pass utf characters as is.
 * If a name begins with a number N, such as "4LFTX", replace with
 * "DIGIT_N_", such as "DIGIT_4_LFTX".
 * Note that apparently, FORTRAN will not allow a leading underscore,
 * so remove if we are doing fortran.
 *
 * It is required that decodify be idempotent:
 * i.e. decodify(decodify(s)) == decodify(s)
 *
 * Returned name is pool alloc'd so is transient
 */

static int init = 0;
static char* repls[256];	/* replacement string for each char */
static int lens[256];	/* lengths of replacement strings */
static struct {
	char c;
	char *s;
} ctable[] = {
	{' ', "_SPACE_"},
	{'!', "_EXCLAMATION_"},
	{'"', "_QUOTATION_"},
	{'#', "_HASH_"},
	{'$', "_DOLLAR_"},
	{'%', "_PERCENT_"},
	{'&', "_AMPERSAND_"},
	{'\'', "_APOSTROPHE_"},
	{'(', "_LEFTPAREN_"},
	{')', "_RIGHTPAREN_"},
	{'*', "_ASTERISK_"},
	{'+', "_PLUS_"},
	{',', "_COMMA_"},
	{'-', "_MINUS_"},
	{'.', "_PERIOD_"},
	{':', "_COLON_"},
	{';', "_SEMICOLON_"},
	{'<', "_LESSTHAN_"},
	{'=', "_EQUALS_"},
	{'>', "_GREATERTHAN_"},
	{'?', "_QUESTION_"},
	{'@', "_ATSIGN_"},
	{'[', "_LEFTBRACKET_"},
	{'\\', "_BACKSLASH_"},
	{']', "_RIGHTBRACKET_"},
	{'^', "_CIRCUMFLEX_"},
	{'`', "_BACKQUOTE_"},
	{'{', "_LEFTCURLY_"},
	{'|', "_VERTICALBAR_"},
	{'}', "_RIGHTCURLY_"},
	{'~', "_TILDE_"},
 	{'/', "_SLASH_"} 		/* should not occur in names */
/* 	{'_', "_UNDERSCORE_"} */
};
static int idtlen;
static int hexlen;
static Bytebuffer* newname;

static void
initdecodify(void)
{
    int nctable = (sizeof(ctable))/(sizeof(ctable[0]));
    int i;
    char *rp;

    newname = bbNew();
    idtlen = strlen("DIGIT_n_"); /* initial digit template */
    hexlen = strlen("_XHH"); /* template for hex of non-ASCII bytes */
    for(i = 0; i < 128; i++) {
        rp = emalloc(2);
        rp[0] = i;
        rp[1] = '\0';
        repls[i] = rp;
    }
    for(i=0; i < nctable; i++) {
        size_t j = ctable[i].c;
        efree(repls[j]);
        repls[j] = ctable[i].s;
    }
    for(i = 128; i < 256; i++) {
        rp = emalloc(hexlen+1);
        snprintf(rp, hexlen+1, "_X%2.2X", i); /* need to include null*/
        rp[hexlen] = '\0';
        repls[i] = rp;
    }
    for(i = 0; i < 256; i++) {
        lens[i] = strlen(repls[i]);
    }
    init = 1;               /* only do this initialization once */
}

char*
decodify(const char *name0)
{
    const unsigned char *cp;
    unsigned int c;
    char* name;

    if(init == 0) initdecodify();
    bbClear(newname);
    cp = (const unsigned char*) name0;
    if('0' <= *cp && *cp <= '9') { /* handle initial digit, if any */
	char tmp[16];
	snprintf(tmp,sizeof(tmp),"DIGIT_%c_", *cp);
	bbCat(newname,tmp);
	cp++;
    }
    while((c=*cp++)) { /* copy name to newname, replacing special chars */
	ASSERT(c <= 256);
	bbCat(newname,repls[c]);
    }
    /* If FORTRAN, remove leading _, if any */
    name = bbContents(newname);
    if(bbGet(newname,0) == '_') name++;
    return pooldup(name);
}

/**************************************************/
/* CML String Escapes */
/**************************************************/

/*
Given a character c, fill s with the character suitably escaped
for use with xml.
Caller must ensure enough space
Currently does not handle unicode
Returns s as it result.
*/

static char hexdigits[] = "0123456789ABCDEF";

static char printescapable[] = "\"&<>";
static char* printescape[] = {"quot", "amp", "lt", "gt"};

static void
xescapifychar(unsigned int c, int quote, Bytebuffer* s)
{
    if(c >= ' ' && c < '\177') {
	char* p;
	char** q;
	for(p=printescapable,q=printescape;*p;p++,q++) {if(c==*p) break;}
	if(*p) {
	    bbAppend(s,'&');
	    bbCat(s,*q);
	    bbAppend(s,';');
	} else
	    bbAppend(s,(char)c);
    } else {
	/* Do hex escape */
	    unsigned int hex1 = (c & 0x0f);
	    unsigned int hex2 = ((c >> 4) & 0x0f);
	    bbCat(s,"&#");
	    bbAppend(s,hexdigits[hex2]);
	    bbAppend(s,hexdigits[hex1]);
	    bbAppend(s,';');
    }
}

/* Return a pool string that is s0 with all characters
   ecaped that require it.  The resulting string is not
   surrounded by quotes.
   Since the string might actually contain nulls, specify the length.
*/

char*
xescapify(char* s0, int quote, size_t len)
{
    int i;
    char* result;
    Bytebuffer* escaped = bbNew();
    for(i=0;i<len;i++) {
	xescapifychar((unsigned int)s0[i],quote,escaped);
    }
    result = pooldup(bbContents(escaped));
    bbFree(escaped);
    return result;        
}

/**************************************************/
/* Java String Escapes */
/**************************************************/

/*
Given a utf16 character c,
fill s with the characters needed
to suitably escape c for use with Java.
*/

static void
jescapifychar(UTF16 c, int quote, Bytebuffer* s)
{
    /* Separate out ascii from UTF16 */
    if(c <= '\177') {
	/* Separate printables from controls */
        if(c >= ' ' && c < '\177') {
	    if (c == quote) {
		bbAppend(s,'\\');
	    }
	    bbAppend(s,(char)c);
	} else switch (c) {
	    case '\t': bbCat(s,"\\t"); break;
	    case '\b': bbCat(s,"\\b"); break;
	    case '\n': bbCat(s,"\\n"); break;
	    case '\r': bbCat(s,"\\r"); break;
	    case '\f': bbCat(s,"\\f"); break;
	    default:
		{ /* Do hex escape */
		int hex1 = (c & 0x0f);
		int hex2 = ((c >> 4) & 0x0f);
		int hex3 = ((c >> 8) & 0x0f);
		int hex4 = ((c >> 12) & 0x0f);
		bbAppend(s,'\\');
		bbAppend(s,'u');
		bbAppend(s,hexdigits[hex4]);
		bbAppend(s,hexdigits[hex3]);
		bbAppend(s,hexdigits[hex2]);
		bbAppend(s,hexdigits[hex1]);
		} break;
	}
    } else { /* Do \uxxxx escapes */
	/* Do hex escape */
	int hex1 = (c & 0x0f);
	int hex2 = ((c >> 4) & 0x0f);
	int hex3 = ((c >> 8) & 0x0f);
	int hex4 = ((c >> 12) & 0x0f);
	bbAppend(s,'\\');
	bbAppend(s,'u');
	bbAppend(s,hexdigits[hex4]);
	bbAppend(s,hexdigits[hex3]);
	bbAppend(s,hexdigits[hex2]);
	bbAppend(s,hexdigits[hex1]);
    }
}

/* Return a pool string that is s0 with all characters
   ecaped that require it.  The resulting string is not
   surrounded by quotes.
   Since the string might actually contain nulls, specify the length.
*/

char*
jescapify(char* s0, int quote, size_t len)
{
    int i;
    char* result;
    UTF8* s8;
    UTF16* s16; /* for storing the utf16 string */
    UTF16* tmp16; /* for storing the utf16 string */
    ConversionResult status;
    Bytebuffer* escaped = bbNew();
    size_t len16;

    s16 = emalloc((1+len)*sizeof(UTF16));
    s8 = (UTF8*)s0;
    tmp16 = s16;
    status = ConvertUTF8toUTF16((const UTF8**)&s8,s8+len,&tmp16,tmp16+len,lenientConversion);
    if(status != conversionOK) {
	derror("Cannot convert UTF8 string to UTF16: %s",s0);
	return NULL;	
    }
    /* Get the length of the utf16 string */
    len16 = (tmp16 - s16);
    for(i=0;i<len16;i++) {
	jescapifychar(s16[i],quote,escaped);
    }
    efree(s16);    
    result = pooldup(bbContents(escaped));
    bbFree(escaped);
    return result;        
}

char*
jescapifyname(char* s0)
{
    return jescapify(s0,'"',strlen(s0));
}

/*
Convert a java name that might possibly
contain utf8 characters to one that is
acceptable to the Java compiler.
Basically this means convert the printables
using decodify (above) equivalent and then escape
all the utf chars.
*/
char*
jdecodify (const char *name)
{
    return decodify(name);
}

/* FORTRAN does escapes differently than e.g. C */

char*
f77escapifychar(unsigned int c, char* s0)
{
    char* s = s0;
    s0[0] = '\0';
    if(c == '\'') {
	*s++ = '\''; *s++='\'';
    } else if(c >= ' ' && c < '\177') {
	*s++ = (char)c;
    } else {
	char tmp[32];
	nprintf(tmp,sizeof(tmp),"//char(%u)",c);	
	strcat(s,tmp);
	s += strlen(tmp);
    }
    *s = '\0';
    return s0;
}

void
f77quotestring(Bytebuffer* databuf)
{
    int i;
    int lastcharescaped;
    unsigned int slen = bbLength(databuf);
    unsigned char* s;

    /* Handle the empty string case */
    if(slen == 0) {
	bbCat(databuf,"char(0)");
	return;
    }

    s = (unsigned char*)emalloc(slen+1);
    memcpy((void*)s,bbContents(databuf),slen);
    s[slen] = '\0';
    bbClear(databuf);

    lastcharescaped = 0;    
    for(i=0;i<slen;i++) {
	char tmp[32];
	unsigned int c = s[i];
	int thischarescaped = (c < ' ' || c >= '\177');
	if(i > 0) {
            if(!lastcharescaped && thischarescaped) bbAppend(databuf,'\'');
	    else if(lastcharescaped && !thischarescaped) bbCat(databuf,"//'");
	} else if(!thischarescaped)
	    bbAppend(databuf,'\'');
	f77escapifychar(c,tmp);
	if(i == 0 && thischarescaped)
            bbCat(databuf,tmp+2);
	else
            bbCat(databuf,tmp);
	lastcharescaped = thischarescaped;
    }
    if(!lastcharescaped) bbAppend(databuf,'\'');
}

char*
f77escapifyname(char* s0)
{
    Bytebuffer* buf = bbNew();
    char* name;
    bbCat(buf,s0);
    f77quotestring(buf);
    name = bbDup(buf);
    bbFree(buf);
    return name;
}
