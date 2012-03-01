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
#define XDR_INT_MIN (-2147483647-1)
#define XDR_INT_MAX 2147483647

char errstr[100];		/* for short error messages */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "genlib.h"
#include "ncgentab.h"

#define YY_BREAK                /* defining as nothing eliminates unreachable
				   statement warnings from flex output, 
                                   but make sure every action ends with
                                   "return" or "break"! */

%}

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

/* New definition to conform to a subset of string.c */
ID ([a-zA-Z_]|{UTF8}|\\[0-9])([a-zA-Z0-9_.@+-]|{UTF8}|{idescaped})*

escaped		\\.

/* Note: this definition of string will work for utf8 as well,
   although it is a very relaxed definition
*/
nonquotes	([^"\\]|{escaped})*
exp		([eE][+-]?[0-9]+)
%%
\/\/.*		        { /* comment */ 
                          break;
                        }

\"{nonquotes}\"		{
			 if(yyleng > MAXTRST) {
				yyerror("string too long, truncated\n");
			        yytext[MAXTRST-1] = '\0';
			 }
			 expand_escapes(termstring,(char *)yytext,yyleng);
		 	 return (TERMSTRING);
		        }

float|FLOAT|real|REAL	{return (FLOAT_K);}
char|CHAR		{return (CHAR_K);}
byte|BYTE		{return (BYTE_K);}
short|SHORT		{return (SHORT_K);}
long|LONG|int|INT|integer|INTEGER	{return (INT_K);}
double|DOUBLE		{return (DOUBLE_K);}
unlimited|UNLIMITED	{int_val = -1;
			 return (NC_UNLIMITED_K);}

dimensions:|DIMENSIONS:	{return (DIMENSIONS);}
variables:|VARIABLES:	{return (VARIABLES);}
data:|DATA:		{return (DATA);}
(netcdf|NETCDF|netCDF)[ \t]+[^\{]+	{
		char *s = (char*)yytext+strlen("netcdf");
		char *t = (char*)yytext+yyleng-1;
		while (isspace(*s))
			s++;
		while (isspace(*t))
			t--;
		t++;
                if (t-s+1 < 1) {
                        yyerror("netCDF name required");
                        return (DATA); /* generate syntax error */
                }
		netcdfname = (char *) emalloc(t-s+1);
		(void) strncpy(netcdfname, s, t-s);
		netcdfname[t-s] = '\0';
		deescapify(netcdfname);  /* so "\5foo" becomes "5foo", for example */
		return (NETCDF);
		}
DoubleInf|NaN|-?Infinity { /* missing value (pre-2.4 backward compatibility) */
                if (yytext[0] == '-') {
		    double_val = -NC_FILL_DOUBLE;
                } else {
		    double_val = NC_FILL_DOUBLE;
                }
		return (DOUBLE_CONST);
		}
FloatInf|-?Inff	{ /* missing value (pre-2.4 backward compatibility) */
                if (yytext[0] == '-') {
		    float_val = -NC_FILL_FLOAT;
                } else {
		    float_val = NC_FILL_FLOAT;
                }
		return (FLOAT_CONST);
		}
{ID} {
                if (STREQ((char *)yytext, FILL_STRING))
		        return (FILLVALUE);
		if ((yylval = lookup((char *)yytext)) == NULL) {
			yylval = install((char *)yytext);
			}
		return (IDENT);
		}

\n		{
		lineno++ ;
                break;
		}

[+-]?[0-9]*[0-9][Bb]  {
                int ii;
		if (sscanf((char*)yytext, "%d", &ii) != 1) {
		    sprintf(errstr,"bad byte constant: %s",(char*)yytext);
		    yyerror(errstr);
		}
                byte_val = ii;
		if (ii != (int)byte_val) {
		    sprintf(errstr,"byte constant out of range (-128,127): %s",(char*)yytext);
		    yyerror(errstr);
		}
		return (BYTE_CONST);
                }

[+-]?[0-9]*\.[0-9]*{exp}?[LlDd]?|[+-]?[0-9]*{exp}[LlDd]? {
		if (sscanf((char*)yytext, "%le", &double_val) != 1) {
		    sprintf(errstr,"bad long or double constant: %s",(char*)yytext);
		    yyerror(errstr);
		}
                return (DOUBLE_CONST);
                }
[+-]?[0-9]*\.[0-9]*{exp}?[Ff]|[+-]?[0-9]*{exp}[Ff] {
		if (sscanf((char*)yytext, "%e", &float_val) != 1) {
		    sprintf(errstr,"bad float constant: %s",(char*)yytext);
		    yyerror(errstr);
		}
                return (FLOAT_CONST);
                }
[+-]?[0-9]+[sS]|0[xX][0-9a-fA-F]+[sS] {
		if (sscanf((char*)yytext, "%hd", &short_val) != 1) {
		    sprintf(errstr,"bad short constant: %s",(char*)yytext);
		    yyerror(errstr);
		}
		return (SHORT_CONST);
	        }
[+-]?([1-9][0-9]*|0)[lL]? {
    		char *ptr;
                errno = 0;
		double_val = strtod((char*)yytext, &ptr);
		if (errno != 0 && double_val == 0.0) {
		    sprintf(errstr,"bad numerical constant: %s",(char*)yytext);
		    yyerror(errstr);
		}
                if (double_val < XDR_INT_MIN ||double_val > XDR_INT_MAX) {
                    return DOUBLE_CONST;
                } else {
                    int_val = (int) double_val;
                    return INT_CONST;
                }
	        }
0[xX]?[0-9a-fA-F]+[lL]? {
    		char *ptr;
                long long_val;
                errno = 0;
		long_val = strtol((char*)yytext, &ptr, 0);
		if (errno != 0) {
		    sprintf(errstr,"bad long constant: %s",(char*)yytext);
		    yyerror(errstr);
		}
                if (long_val < XDR_INT_MIN || long_val > XDR_INT_MAX) {
                    double_val = (double) long_val;
                    return DOUBLE_CONST;
                } else {
                    int_val = (int) long_val;
                    return INT_CONST;
                }
	        }
\'[^\\]\'          {
	        (void) sscanf((char*)&yytext[1],"%c",&byte_val);
		return (BYTE_CONST);
                }
\'\\[0-7][0-7]?[0-7]?\'  {
		byte_val = (char) strtol((char*)&yytext[2], (char **) 0, 8);
		return (BYTE_CONST);
                }
\'\\[xX][0-9a-fA-F][0-9a-fA-F]?\'  {
		byte_val = (char) strtol((char*)&yytext[3], (char **) 0, 16);
		return (BYTE_CONST);
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
		return (BYTE_CONST);
                }

[ \r\t\f]+	{ /* whitespace */ 
		  break;        
		}
.		{/* Note: this next rule will not work for UTF8 characters */
		return (yytext[0]) ;
		}

%%

/* Hack to keep compile quiet */
void
ignore()
{
    yyunput(0,NULL);

}

