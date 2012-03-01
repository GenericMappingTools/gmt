%{
/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *********************************************************************/

/* Problems:
1. Ideally, we assume the input is true ut8.
   Unfortunately, we may actually get iso-latin-8859-1.
   This means that there will be ambiguity about the characters
   in the range 128-255 because they will look like n-byte unicode
   when they are 1-byte 8859 characters. Because of our encoding,
   8859 characters above 128 will be handles as n-byte utf8 and so
   will probably not lex correctly.
   Solution: assume utf8 and note in the documentation that
   ISO8859 is specifically unsupported.
2. The netcdf function NC_check_name in string.c must be modified to
   conform to the use of UTF8.
3. We actually have three tests for UTF8 of increasing correctness
   (in the sense that the least correct will allow some sequences that
    are technically illegal UTF8).
   The tests are derived from the table at
	    http://www.w3.org/2005/03/23-lex-U
   We include lexical definitions for all three, but use the second version.
*/

/* lex specification for tokens for ncgen */

/* Fill value used by ncdump from version 2.4 and later.  Should match
   definition of FILL_STRING in ../ncdump/vardata.h */
#define FILL_STRING "_"
#define XDR_INT32_MIN (-2147483647-1)
#define XDR_INT32_MAX 2147483647
#define XDR_INT64_MIN  (-9223372036854775807LL-1)
#define XDR_INT64_MAX  (9223372036854775807LL)

char errstr[100];		/* for short error messages */

int lineno;              /* line number for error messages */
char* lextext;           /* name or string with escapes removed */

#define YY_BREAK                /* defining as nothing eliminates unreachable
				   statement warnings from flex output, 
                                   but make sure every action ends with
                                   "return" or "break"! */

int specialconstants;		/* 1 if nan, nanf, infinity, etc is used */
double double_val;             /* last double value read */
float float_val;               /* last float value read */
long long int64_val;           /* last int64 value read */
int int32_val;                 /* last int32 value read */
short int16_val;               /* last short value read */
unsigned long long uint64_val; /* last int64 value read */
unsigned int uint32_val;       /* last int32 value read */
unsigned short uint16_val;     /* last short value read */
char char_val;                 /* last char value read */
signed char byte_val;                 /* last byte value read */
unsigned char ubyte_val;       /* last byte value read */

static Symbol* makepath(char* text);
static int lexdebug(int);
static int parseLL(char* text);

static struct Specialtoken {
    char* name;
    int   token;
} specials[] = {
{"_FillValue",_FILLVALUE},
{"_Format",_FORMAT},
{"_Storage",_STORAGE},
{"_ChunkSizes",_CHUNKSIZES},
{"_Fletcher32",_FLETCHER32},
{"_DeflateLevel",_DEFLATELEVEL},
{"_Shuffle",_SHUFFLE},
{"_Endianness",_ENDIANNESS},
{"_NoFill",_NOFILL},
{NULL,0} /* null terminate */
};

%}
%x ST_C_COMMENT
%x TEXT
%p 6000

/* The most correct (validating) version of UTF8 character set
   (Taken from: http://www.w3.org/2005/03/23-lex-U)

The lines of the expression cover the UTF8 characters as follows:
1. non-overlong 2-byte
2. excluding overlongs
3. straight 3-byte
4. excluding surrogates
5. straight 3-byte
6. planes 1-3
7. planes 4-15
8. plane 16

UTF8   ([\xC2-\xDF][\x80-\xBF])                       \
     | (\xE0[\xA0-\xBF][\x80-\xBF])                   \
     | ([\xE1-\xEC][\x80-\xBF][\x80-\xBF])            \
     | (\xED[\x80-\x9F][\x80-\xBF])                   \
     | ([\xEE-\xEF][\x80-\xBF][\x80-\xBF])            \
     | (\xF0[\x90-\xBF][\x80-\xBF][\x80-\xBF])        \
     | ([\xF1-\xF3][\x80-\xBF][\x80-\xBF][\x80-\xBF]) \
     | (\xF4[\x80-\x8F][\x80-\xBF][\x80-\xBF])        \

*/

/* Wish there was some way to ifdef lex files */

/*The most relaxed version of UTF8 (not used)
UTF8 ([\xC0-\xD6].)|([\xE0-\xEF]..)|([\xF0-\xF7]...)
*/

/*The partially relaxed version of UTF8, and the one used here */
UTF8 ([\xC0-\xD6][\x80-\xBF])|([\xE0-\xEF][\x80-\xBF][\x80-\xBF])|([\xF0-\xF7][\x80-\xBF][\x80-\xBF][\x80-\xBF])

/* The old definition of ID
ID ([A-Za-z_]|{UTF8})([A-Z.@#\[\]a-z_0-9+-]|{UTF8})*
*/

/* Don't permit control characters or '/' in names, but other special
   chars OK if escaped.  Note that to preserve backwards
   compatibility, none of the characters _.@+- should be escaped, as
   they were previously permitted in names without escaping.  */

idescaped \\[ !"#$%&'()*,:;<=>?\[\\\]^`{|}~]
numescaped \\[0-9]

/* New definition to conform to a subset of string.c */
ID ([a-zA-Z_]|{UTF8}|{numescaped})([a-zA-Z0-9_.@+-]|{UTF8}|{idescaped})*

escaped		\\.

/* Capture a datasetidentifier */
/* DATASETID ([a-zA-Z0-9!#$%&*:;<=>?/^|~_.@+-]|{UTF8})* */
DATASETID [^{][^{]*


/* Note: this definition of string will work for utf8 as well,
   although it is a very relaxed definition
*/
nonquotes	([^"\\]|{escaped})*
exp		([eE][+-]?[0-9]+)

OPAQUESTRING	(0[xX][0-9A-Fa-f][0-9A-Fa-f]*)

PATH    ([/]|([/]{ID})([/]{ID})*)

SIMPLENUMBER [+-]?[0-9][0-9]*
XUNUMBER     {OPAQUESTRING}[SsLl]
UNUMBER      [0-9][0-9]*[Uu][BbSsLl]?
NUMBER       [+-]?[0-9][0-9]*[BbSsLl]
DBLNUMBER    [+-]?[0-9]*\.[0-9]*{exp}?[LlDd]?|[+-]?[0-9]*{exp}[LlDd]?
FLTNUMBER    [+-]?[0-9]*\.[0-9]*{exp}?[Ff]|[+-]?[0-9]*{exp}[Ff]

SPECIAL "_FillValue"|"_Format"|"_Storage"|"_ChunkSizes"|"_Fletcher32"|"_DeflateLevel"|"_Shuffle"|"_Endianness"|"_NoFill"

%%
[ \r\t\f]+	{ /* whitespace */ 
		  break;        
		}

\/\/.*		        { /* comment */ 
                          break;
                        }

\"{nonquotes}\"		{
			 /* In netcdf4, this will be used in a variety
                            of places, so only remove escapes */
/*
if(yyleng > MAXTRST) {
yyerror("string too long, truncated\n");
yytext[MAXTRST-1] = '\0';
}
*/
			lextext = emalloc(1+4*yyleng); /* should be max
                                                             needed */
		        /* Assumes expand escapes also does normalization */
			expand_escapes(lextext,(char *)yytext,yyleng);
		 	return lexdebug(TERMSTRING);
		        }

{OPAQUESTRING}	{ /* drop leading 0x; pad to even number of chars */
		char* p = yytext+2;
		int len = yyleng - 2;
		int padlen = len;
	        if((padlen % 2) == 1) padlen++;
		lextext = emalloc(1+padlen);
		strncpy(lextext,p,len+1); /*include null */
		lextext[len] = '0'; /* assume padding */
		lextext[padlen] = '\0'; /* make sure null terminated */
		/* convert all chars to lower case */
		for(p=lextext;*p;p++) *p = tolower(*p);
		return lexdebug(OPAQUESTRING);
		}

[cC]ompound|[sS]truct|[sS]tructure|STRUCT|COMPOUND|STRUCTURE {return lexdebug(COMPOUND);}
enum|ENUM				{return lexdebug(ENUM);}
opaque|OPAQUE				{return lexdebug(OPAQUE);}

float|FLOAT|real|REAL	{return lexdebug(FLOAT_K);}
char|CHAR		{return lexdebug(CHAR_K);}
byte|BYTE		{return lexdebug(BYTE_K);}
ubyte|UBYTE		{return lexdebug(UBYTE_K);}
short|SHORT		{return lexdebug(SHORT_K);}
ushort|USHORT		{return lexdebug(USHORT_K);}
long|LONG|int|INT|integer|INTEGER	{return lexdebug(INT_K);}
ulong|ULONG|uint|UINT|uinteger|UINTEGER	{return lexdebug(UINT_K);}
int64|INT64		{return lexdebug(INT64_K);}
uint64|UINT64		{return lexdebug(UINT64_K);}
double|DOUBLE		{return lexdebug(DOUBLE_K);}
unlimited|UNLIMITED	{int32_val = -1;
			 return lexdebug(NC_UNLIMITED_K);}

types:			{return lexdebug(TYPES);}
dimensions:		{return lexdebug(DIMENSIONS);}
variables:		{return lexdebug(VARIABLES);}
data:			{return lexdebug(DATA);}
group:			{return lexdebug(GROUP);}

(netcdf|NETCDF|netCDF)  {BEGIN(TEXT);return lexdebug(NETCDF);}

DoubleInf|-?Infinity { /* missing value (pre-2.4 backward compatibility) */
                if (yytext[0] == '-') {
		    double_val = NEGINFINITE;
                } else {
		    double_val = INFINITE;
                }
		specialconstants = 1;
		return lexdebug(DOUBLE_CONST);
		}
NaN|nan { /* missing value (pre-2.4 backward compatibility) */
		double_val = NAN;
		specialconstants = 1;
		return lexdebug(DOUBLE_CONST);
		}

FloatInf|-?Infinityf|-?Inff {/* missing value (pre-2.4 backward compatibility)*/
                if (yytext[0] == '-') {
		    float_val = NEGINFINITEF;
                } else {
		    float_val = INFINITEF;
                }
		specialconstants = 1;
		return lexdebug(FLOAT_CONST);
		}
NaNf|nanf { /* missing value (pre-2.4 backward compatibility) */
		float_val = NANF;
		specialconstants = 1;
		return lexdebug(FLOAT_CONST);
		}

{PATH}		{
		lextext = emalloc(1+4*yyleng); /* should be max needed */
		strncpy(lextext,(char*)yytext,yyleng+1); /* include null */
		yylval.sym = makepath(lextext);
		free(lextext);
		return lexdebug(PATH);
		}


{SPECIAL}      {struct Specialtoken* st;
		lextext = emalloc(1+4*yyleng); /* should be max needed */
		strncpy(lextext,(char*)yytext,yyleng+1); /* include null */
		for(st=specials;st->name;st++) {
		    if(strcmp(lextext,st->name)==0) {return lexdebug(st->token);}
		}
		return 0;
		}

<TEXT>{DATASETID} {
		    int c;
		    char* p; char* q;
		    lextext = emalloc(1+4*yyleng); /* should be max needed */
		    /* copy the trimmed name */
		    strncpy(lextext,(char*)yytext,yyleng+1); /* include null */
		    p = lextext;
		    q = lextext;
		    while((c=*p++)) {if(c > ' ') *q++ = c;}
		    *q = '\0';
		    datasetname = lextext;
		    BEGIN(INITIAL);
		    return lexdebug(DATASETID);
		}

{ID}		{
		    lextext = emalloc(1+4*yyleng); /* should be max needed */
		    strncpy(lextext,(char*)yytext,yyleng+1); /* include null */
		    deescapify(lextext);
		    if (STREQ((char *)lextext, FILL_STRING)) return lexdebug(FILLMARKER);
		    yylval.sym = install(lextext);
		    return lexdebug(IDENT);
		}

{SIMPLENUMBER}	{
		    /* We need to try to see what size of integer ((u)int). */
		    /* Technically, the user should specify, but... */
		    /* If out of any integer range, then complain */
		    switch (parseLL(yytext)) {
		    case 0:
			return lexdebug(INT64_CONST); /* meaningless */
		    case -1:
#ifdef USE_NETCDF4
		        if(int64_val >= NC_MIN_INT) {
			    int32_val = (int)int64_val;
		            return lexdebug(INT_CONST);
			} else
		            return lexdebug(INT64_CONST);
#else
			/* Convert to int32, unless overflow then complain  */
			if(int64_val >=NC_MIN_INT) {
			    int32_val = (int)int64_val;
			} else {
			   sprintf(errstr,"32 bit integer constant out of range: %s",(char*)yytext);
			   yyerror(errstr);
			}
		        return lexdebug(INT_CONST);		    
#endif
		    case 1:
#ifdef USE_NETCDF4
 		        if(uint64_val <= NC_MAX_INT) {
			    int32_val = (int)uint64_val;
		            return lexdebug(INT_CONST);
 		        } else if(uint64_val <= NC_MAX_UINT) {
			    uint32_val = (unsigned int)uint64_val;
		            return lexdebug(UINT_CONST);
 		        } else if(uint64_val <= NC_MAX_INT64) {
			    int64_val = (long long)uint64_val;
		            return lexdebug(INT64_CONST);
			} else
		            return lexdebug(UINT64_CONST);
#else
			/* Convert to int32 but complain if overflow */
			if(uint64_val > NC_MAX_INT) {
			    sprintf(errstr,"32 bit integer constant out of range: %s",(char*)yytext);
			    yyerror(errstr);
			}
			int32_val = (int)uint64_val;
			return lexdebug(INT_CONST);
#endif
		    }
		}

{NUMBER}    { /* The number may be signed or unsigned (signed has priority) */
		int slen = strlen(yytext);
		int tag = yytext[slen-1];
		int token = 0;
		switch (tag) {
		case 'B': case 'b':
		    if (sscanf((char*)yytext, "%d", &int32_val) != 1) {
		        sprintf(errstr,"bad byte constant: %s",(char*)yytext);
		        yyerror(errstr);
		    } else if(int32_val >= NC_MIN_BYTE && int32_val <= NC_MAX_BYTE) {
		        byte_val = (signed char)int32_val;
		        token = BYTE_CONST;
		    } else if(int32_val >= 0 && int32_val <= NC_MAX_UBYTE) {
		        ubyte_val = (unsigned char)int32_val;
		        token = UBYTE_CONST;
		    } else {
		        sprintf(errstr,"bad byte constant: %s",(char*)yytext);
		        yyerror(errstr);
		    }
		    break;
		case 'S': case 's':
		    if (sscanf((char*)yytext, "%d", &int32_val) != 1) {
		        sprintf(errstr,"bad short constant: %s",(char*)yytext);
		        yyerror(errstr);
		    } else if(int32_val >= NC_MIN_SHORT && int32_val <= NC_MAX_SHORT) {
		        int16_val = (signed short)int32_val;
		        token = SHORT_CONST;
		    } else if(int32_val >= 0 && int32_val <= NC_MAX_USHORT) {
		        uint16_val = (unsigned short)int32_val;
		        token = USHORT_CONST;
		    } else {
		        sprintf(errstr,"bad short constant: %s",(char*)yytext);
		        yyerror(errstr);
		    }
		    break;
		case 'L': case 'l':
		    switch(parseLL(yytext)) {
		    case 0:
		        sprintf(errstr,"bad int64 constant: %s",(char*)yytext);
		        yyerror(errstr);
			break;
		    case -1: token = INT64_CONST; break;
		    case 1: token = UINT64_CONST; break;
		    }
		    break;
		default:/*(optionally)signed string of digits; treat like int*/
		    sprintf(errstr,"bad numeric constant: %s",(char*)yytext);
		    yyerror(errstr);
		}
		return lexdebug(token);
	    }

{UNUMBER}    {
		int slen = strlen(yytext);
		int tag = yytext[slen-1];
		int token = 0;
		switch (tag) {
		case 'B': case 'b':
		    if (sscanf((char*)yytext, "%u", &uint32_val) != 1
		        || uint32_val > NC_MAX_UBYTE) {
		        sprintf(errstr,"bad unsigned byte constant: %s",(char*)yytext);
		        yyerror(errstr);
		    }
		    ubyte_val = (unsigned char)uint32_val;
		    token = UBYTE_CONST;
		    break;
		case 'S': case 's':
		    if (sscanf((char*)yytext, "%u", &uint32_val) != 1
		        || uint32_val > NC_MAX_USHORT) {
		        sprintf(errstr,"bad unsigned short constant: %s",(char*)yytext);
		        yyerror(errstr);
		    }
		    uint16_val = (unsigned short)uint32_val;
		    token = USHORT_CONST;
		    break;
		case 'L': case 'l':
		    if (sscanf((char*)yytext, "%llu", &uint64_val) != 1) {
		        sprintf(errstr,"bad unsigned int64 constant: %s",(char*)yytext);
		        yyerror(errstr);
		    }
		    token = UINT64_CONST;
		    break;
		default: /* string of digits; treat like int */
		    if (sscanf((char*)yytext, "%u", &uint32_val) != 1) {
		        sprintf(errstr,"bad unsigned int constant: %s",(char*)yytext);
		        yyerror(errstr);
		    }
		    token = UINT_CONST;
		}
		return lexdebug(token);
	    }
{XUNUMBER}    {
		int c;
		int token = 0;
		int slen = strlen(yytext);
		int tag = yytext[slen-1];
		char* hex = yytext+2; /* point to first true hex digit */
		int xlen = (slen - 3);  /* true hex length */
		yytext[slen-1] = '\0';
	        if(xlen > 16) { /* truncate hi order digits */
		    hex += (xlen - 16);
		}
		/* convert to an unsigned long long */
		uint64_val = 0;
		while((c=*hex++)) {
		    unsigned int hexdigit = (c <= '9'?(c-'0'):(c-'a')+0xa);
		    uint64_val = ((uint64_val << 4) | hexdigit);
		}
		switch (tag) {
		case 'S': case 's':
		    uint16_val = (unsigned short)uint64_val;
		    token = USHORT_CONST;
		    break;
		case 'L': case 'l':
		    token = UINT64_CONST;
		    break;
		default: /* should never happen */
		    if (sscanf((char*)yytext, "%i", &uint32_val) != 1) {
		        sprintf(errstr,"bad unsigned int constant: %s",(char*)yytext);
		        yyerror(errstr);
		    }
		    token = UINT_CONST;
		}
		return lexdebug(token);
	    }
{DBLNUMBER}	{
		if (sscanf((char*)yytext, "%le", &double_val) != 1) {
		    sprintf(errstr,"bad long or double constant: %s",(char*)yytext);
		    yyerror(errstr);
		}
                return lexdebug(DOUBLE_CONST);
                }
{FLTNUMBER}  {
		if (sscanf((char*)yytext, "%e", &float_val) != 1) {
		    sprintf(errstr,"bad float constant: %s",(char*)yytext);
		    yyerror(errstr);
		}
                return lexdebug(FLOAT_CONST);
                }
\'[^\\]\'          {
	        (void) sscanf((char*)&yytext[1],"%c",&byte_val);
		return lexdebug(BYTE_CONST);
                }
\'\\[0-7][0-7]?[0-7]?\'  {
		byte_val = (char) strtol((char*)&yytext[2], (char **) 0, 8);
		return lexdebug(BYTE_CONST);
                }
\'\\[xX][0-9a-fA-F][0-9a-fA-F]?\'  {
		byte_val = (char) strtol((char*)&yytext[3], (char **) 0, 16);
		return lexdebug(BYTE_CONST);
                }
\'\\.\'        {
	       switch ((char)yytext[2]) {
	          case 'a': byte_val = '\007'; break; /* not everyone under-
						       * stands '\a' yet */
     	          case 'b': byte_val = '\b'; break;
		  case 'f': byte_val = '\f'; break;
		  case 'n': byte_val = '\n'; break;
		  case 'r': byte_val = '\r'; break;
		  case 't': byte_val = '\t'; break;
		  case 'v': byte_val = '\v'; break;
		  case '\\': byte_val = '\\'; break;
		  case '?': byte_val = '\177'; break;
		  case '\'': byte_val = '\''; break;
		  default: byte_val = (char)yytext[2];
	           }
		return lexdebug(BYTE_CONST);
                }

\n		{
		lineno++ ;
                break;
		}

"/""*"	{/*initial*/
	    BEGIN(ST_C_COMMENT);
	    break;
	}

<ST_C_COMMENT>([^*]|"*"[^/])*	{/* continuation */
				     break;
				}

<ST_C_COMMENT>"*/"	{/* final */
			    BEGIN(INITIAL);
			    break;
			}

<ST_C_COMMENT><<EOF>>	{/* final, error */
			    fprintf(stderr,"unterminated /**/ comment");
			    BEGIN(INITIAL);
			    break;
			}

.		{/* Note: this next rule will not work for UTF8 characters */
		return lexdebug(yytext[0]) ;
		}
%%
static int
lexdebug(int token)
{
    if(debug >= 2)
    {
        fprintf(stderr,"Token=%d |%s|\n",token,yytext);
    }
    return token;
}

int
lex_init(void)
{
    lineno = 1;
    lextext = NULL;
    if(0) unput(0); /* keep -Wall quiet */
    return 0;
}

static Symbol*
makepath(char* text0)
{
    /* Convert path to a sequence of symbols */
    /* use last name as symbol name (with '/' as exception) */
    Symbol* sym = NULL;
    /* walk the path converting to a sequence of symbols */
    if(strcmp(text0,"/")==0) {
	/* special case of root reference */
	sym = rootgroup;
    } else {
        List* prefix = listnew();
	/* split the text into IDENT chunks, convert to symbols */
        Symbol* container = rootgroup;
	char *ident, *p;
        char* text = strdup(text0);
	int c,issuffix;
	ident=text+1; p=ident; /* skip leading '/' */
	do {
	    issuffix=0;
	    switch ((c=*p)) {
	    default: p++; break;
	    case '\\': p++; if(*p == '/') p++; break;
	    case '\0': /* treat null terminator like trailing '/' (mostly) */
		issuffix=1; /* this is the last ident in the path */
		/*fall thru */
	    case '/':
		*p='\0';
		deescapify(ident);
		if(!issuffix) {
		    sym = lookupingroup(NC_GRP,ident,container);
		    if(sym == NULL) {
		        sprintf(errstr,"Undefined or forward referenced group: %s",ident);
		        yyerror(errstr);
			sym = rootgroup;
		    } else {
		        listpush(prefix,(elem_t)sym);
		    }
		} else
		{
		    sym = install(ident);
		    sym->objectclass = NC_GRP;
		    sym->is_ref = 1;
		    sym->ref = NULL;
		    sym->container = container;
		    sym->subnodes = listnew();
		}
		container = sym;
	        ident=p+1; p=ident;		
	        break;
	    }
	} while(c != '\0');
	sym->objectclass = NC_NAT; /*make caller set correctly */
	sym->prefix = prefix;
	free(text);
    }
    return sym;
}

static int
parseLL(char* text)
{
    int result = 0;
#if defined(HAVE_STRTOLL) && defined(HAVE_STRTOULL)
    extern int errno;
    char* endptr;
    errno = 0; endptr = NULL;
    if(text[0] == '-') {
        int64_val = strtoll(text,&endptr,10);
	result = -1; /* negative int64_val */	
    } else {
        uint64_val = strtoull(text,&endptr,10);
	result = 1; /* positive uint64_val */	
    }
    if(result == 0) {
	sprintf(errstr,"Unparseable 64 bit integer constant: %s",(char*)text);
	yyerror(errstr);
    } else if(errno == ERANGE) {
	sprintf(errstr,"64 bit integer constant out of range: %s",(char*)text);
	yyerror(errstr);
	result = 0; /* out of range */
    }			
#else /*!(defined HAVE_STRTOLL && defined HAVE_STRTOULL)*/
    if(text[0] == '-') {
        sscanf((char*)text, "%lld", &int64_val);
	result = -1; /* negative int64_val */	
    } else {
	sscanf((char*)text, "%llu", &uint64_val);
	result = 1; /* positive uint64_val */	
    }
    /* Have no useful way to detect out of range */
#endif /*!(defined HAVE_STRTOLL && defined HAVE_STRTOULL)*/
    return result;
}
