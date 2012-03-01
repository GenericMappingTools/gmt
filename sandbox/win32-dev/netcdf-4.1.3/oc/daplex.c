/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include "dapparselex.h"
#include <strings.h>

#define URLCVT
#define NONSTDCVT

/* Forward */
static void dumptoken(DAPlexstate* lexstate);
static int tohex(int c);
static void dapaddyytext(DAPlexstate* lex, int c);

/****************************************************/
static char* ddsworddelims =
  "{}[]:;=,";

/* Define 1 and > 1st legal characters */
static char* ddswordchars1 =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+_/%.\\*";
static char* ddswordcharsn =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+_/%.\\*#";
static char* daswordcharsn =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+_/%.\\*:#";
static char* cewordchars1 =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+_/%\\";
static char* cewordcharsn =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+_/%\\";

/* Current sets of legal characters */
/*
static char* wordchars1 = NULL;
static char* wordcharsn = NULL;
static char* worddelims = NULL;
*/

/* Hex digits */
static char hexdigits[] = "0123456789abcdefABCDEF";

static char* keywords[] = {
"alias",
"array",
"attributes",
"byte",
"dataset",
"error",
"float32",
"float64",
"grid",
"int16",
"int32",
"maps",
"sequence",
"string",
"structure",
"uint16",
"uint32",
"url",
"code",
"message",
"program_type",
"program",
NULL /* mark end of the keywords list */
};

static int keytokens[] = {
SCAN_ALIAS,
SCAN_ARRAY,
SCAN_ATTR,
SCAN_BYTE,
SCAN_DATASET,
SCAN_ERROR,
SCAN_FLOAT32,
SCAN_FLOAT64,
SCAN_GRID,
SCAN_INT16,
SCAN_INT32,
SCAN_MAPS,
SCAN_SEQUENCE,
SCAN_STRING,
SCAN_STRUCTURE,
SCAN_UINT16,
SCAN_UINT32,
SCAN_URL,
SCAN_CODE,
SCAN_MESSAGE,
SCAN_PTYPE,
SCAN_PROG
};

/**************************************************/

int
daplex(YYSTYPE* lvalp, DAPparsestate* state)
{
    DAPlexstate* lexstate = state->lexstate;
    int token;
    int c;
    unsigned int i;
    char* p=lexstate->next;
    char* tmp;

    token = 0;
    ocbytesclear(lexstate->yytext);
    /* invariant: p always points to current char */
    for(p=lexstate->next;token==0&&(c=*p);p++) {
	if(c == '\n') {
	    lexstate->lineno++;
	} else if(c <= ' ' || c == '\177') {
	    /* whitespace: ignore */
	} else if(c == '#') {
	    /* single line comment */
	    while((c=*(++p))) {if(c == '\n') break;}
	} else if(strchr(lexstate->worddelims,c) != NULL) {
	    /* don't put in lexstate->yytext to avoid memory leak */
	    token = c;
	} else if(c == '"') {
	    int more = 1;
	    /* We have a string token; will be reported as SCAN_WORD */
	    while(more && (c=*(++p))) {
#ifdef NONSTDCVT
		switch (c) {
		case '"': more=0; break;
		case '\\':
		    c=*(++p);
		    switch (c) {
		    case 'r': c = '\r'; break;
		    case 'n': c = '\n'; break;
		    case 'f': c = '\f'; break;
		    case 't': c = '\t'; break;
		    case 'x': {
			int d1,d2;
			c = '?';
			++p;
		        d1 = tohex(*p++);
			if(d1 < 0) {
			    daperror(state,"Illegal \\xDD in TOKEN_STRING");
			} else {
			    d2 = tohex(*p++);
			    if(d2 < 0) {
			        daperror(state,"Illegal \\xDD in TOKEN_STRING");
			    } else {
				c=(((unsigned int)d1)<<4) | (unsigned int)d2;
			    }
			}
		    } break;
		    default: break;
		    }
		    break;
		default: break;
		}
#else /*!NONSTDCVT*/
	        if(c == '"')
		    more = 0;
		else if(c == '\\') {
		    c=*(++p);
		    if(c == '\0') more = false;
		    if(c != '"') {c = '\\'; --p;}
		}
#endif /*!NONSTDCVT*/
		if(more) dapaddyytext(lexstate,c);
	    }
	    token=SCAN_WORD;
	} else if(strchr(lexstate->wordchars1,c) != NULL) {
	    /* we have a SCAN_WORD */
	    dapaddyytext(lexstate,c);
	    while((c=*(++p))) {
#ifdef URLCVT
		if(c == '%' && p[1] != 0 && p[2] != 0
			    && strchr(hexdigits,p[1]) != NULL
                            && strchr(hexdigits,p[2]) != NULL) {
#ifdef WRONG /* Should not unescape %xx occurrences */
		    int d1,d2;
		    d1 = tohex(p[1]);
		    d2 = tohex(p[2]);
		    if(d1 >= 0 || d2 >= 0) {
			c=(((unsigned int)d1)<<4) | (unsigned int)d2;
			p+=2;
		    }
#endif
		} else {
		    if(strchr(lexstate->wordcharsn,c) == NULL) {p--; break;}
		}
		dapaddyytext(lexstate,c);
#else
		if(strchr(lexstate->wordcharsn,c) == NULL) {p--; break;}
		dapaddyytext(lexstate,c);
#endif
	    }
	    /* Special check for Data: */
	    tmp = ocbytescontents(lexstate->yytext);
	    if(strcmp(tmp,"Data")==0 && *p == ':') {
		dapaddyytext(lexstate,*p); p++;
		token = SCAN_DATA;
	    } else {
	        /* check for keyword */
	        token=SCAN_WORD; /* assume */
	        for(i=0;;i++) {
		    if(keywords[i] == NULL) break;
		    if(strcasecmp(keywords[i],tmp)==0) {
		        token=keytokens[i];
		        break;
		    }
		}
	    }
	} else { /* illegal */
	}
    }
    lexstate->next = p;
    strncpy(lexstate->lasttokentext,ocbytescontents(lexstate->yytext),MAX_TOKEN_LENGTH);
    lexstate->lasttoken = token;
    if(ocdebug >= 2)
	dumptoken(lexstate);

    /*Put return value onto Bison stack*/

    if(ocbyteslength(lexstate->yytext) == 0)
        *lvalp = NULL;
    else {
        *lvalp = ocbytesdup(lexstate->yytext);
	oclistpush(lexstate->reclaim,(ocelem)*lvalp);
    }
    return token;      /* Return the type of the token.  */
}

static void
dapaddyytext(DAPlexstate* lex, int c)
{
    ocbytesappend(lex->yytext,(char)c);
}

static int
tohex(int c)
{
    if(c >= 'a' && c <= 'f') return (c - 'a') + 0xa;
    if(c >= 'A' && c <= 'F') return (c - 'A') + 0xa;
    if(c >= '0' && c <= '9') return (c - '0');
    return -1;
}

static void
dumptoken(DAPlexstate* lexstate)
{
    fprintf(stderr,"TOKEN = |%s|\n",ocbytescontents(lexstate->yytext));
}

/*
Simple lexer
*/

void
dapsetwordchars(DAPlexstate* lexstate, int kind)
{
    switch (kind) {
    case 0:
	lexstate->worddelims = ddsworddelims;
	lexstate->wordchars1 = ddswordchars1;
	lexstate->wordcharsn = ddswordcharsn;
	break;
    case 1:
	lexstate->worddelims = ddsworddelims;
	lexstate->wordchars1 = ddswordchars1;
	lexstate->wordcharsn = daswordcharsn;
	break;
    case 2:
	lexstate->worddelims = ddsworddelims;
	lexstate->wordchars1 = cewordchars1;
	lexstate->wordcharsn = cewordcharsn;
	break;
    default: break;
    }
}

void
daplexinit(char* input, DAPlexstate** lexstatep)
{
    DAPlexstate* lexstate = (DAPlexstate*)malloc(sizeof(DAPlexstate));
    if(lexstatep) *lexstatep = lexstate;
    if(lexstate == NULL) return;
    memset((void*)lexstate,0,sizeof(DAPlexstate));
    lexstate->input = strdup(input);
    lexstate->next = lexstate->input;
    lexstate->yytext = ocbytesnew();
    lexstate->reclaim = oclistnew();
    dapsetwordchars(lexstate,0); /* Assume DDS */
}

void
daplexcleanup(DAPlexstate** lexstatep)
{
    DAPlexstate* lexstate = *lexstatep;
    if(lexstate == NULL) return;
    if(lexstate->input != NULL) ocfree(lexstate->input);
    if(lexstate->reclaim != NULL) {
	while(oclistlength(lexstate->reclaim) > 0) {
	    char* word = (char*)oclistpop(lexstate->reclaim);
	    if(word) free(word);
	}
	oclistfree(lexstate->reclaim);
    }
    ocbytesfree(lexstate->yytext);
    free(lexstate);
    *lexstatep = NULL;
}
