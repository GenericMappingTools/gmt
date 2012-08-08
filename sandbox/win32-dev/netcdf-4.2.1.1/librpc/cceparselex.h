/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef CCEPARSELEX_H
#define CCEPARSELEX_H

#include "config.h"
#include "ccetab.h"

#ifdef WIN32
#define strcasecmp stricmp
#define snprintf _snprintf
#define strtoll (long long)_strtoi64
#define strtoull (unsigned long long)_strtou64
/* Override config.h */
#undef HAVE_STRTOLL
#define HAVE_STRTOLL
#undef HAVE_STRTOULL
#define HAVE_STRTOULL
#endif

/* For consistency with Java parser */
#ifndef null
#define null NULL
#endif

typedef void* Object;

#define YYSTYPE Object

#define MAX_TOKEN_LENGTH 1024

/*! Specifies CCElexstate. */
typedef struct CCElexstate {
    char* input;
    char* next; /* next char in uri.query */
    NCbytes* yytext;
    /*! Specifies the Lasttoken. */
    int lasttoken;
    char lasttokentext[MAX_TOKEN_LENGTH+1]; /* leave room for trailing null */
    NClist* reclaim; /* reclaim SCAN_WORD instances */
} CCElexstate;

/*! Specifies CCEparsestate. */
typedef struct CCEparsestate {
    CCEconstraint* constraint;
    char errorbuf[1024];
    int errorcode;
    CCElexstate* lexstate;
} CCEparsestate;

/* Define a generic object carrier; this serves
   essentially the same role as the typical bison %union
   declaration
*/
   
/* bison parse entry point */
extern int cceparse(CCEparsestate*);

extern int cceerror(CCEparsestate*,char*);

#ifdef IGNORE
extern void ce_parse_error(CCEparsestate*,const char *fmt, ...);

extern int yyerror(CCEparsestate* state, char* msg);
extern void projections(CCEparsestate* state, Object list0);
extern Object projectionlist(CCEparsestate* state, Object list0, Object decl);
extern Object projection(CCEparsestate* state, Object segmentlist);
extern Object segmentlist(CCEparsestate* state, Object list0, Object decl);
extern Object segment(CCEparsestate* state, Object name, Object slices0);
extern Object range(CCEparsestate* state, Object, Object, Object);
extern Object indexer(CCEparsestate* state, Object name, Object indices);
extern Object indexpath(CCEparsestate* state, Object list0, Object index);
extern Object var(CCEparsestate* state, Object indexpath);
extern Object range1(CCEparsestate* state, Object rangenumber);
extern Object rangelist(CCEparsestate* state, Object list0, Object decl);

/* lexer interface */
extern int ccelex(YYSTYPE*, CCEparsestate*);
extern void ccelexinit(char* input, CCElexstate** lexstatep);
extern void ccelexcleanup(CCElexstate** lexstatep);
#endif

extern int cdmceparse(char* input, CCEconstraint*, char**);

#ifdef PARSEDEBUG
extern int ccedebug;
extern Object debugobject(Object);
#define checkobject(x) debugobject(x)
#else
#define checkobject(x) (x)
#endif


#endif /*CCEPARSELEX_H*/

