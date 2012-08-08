/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

/*The lines down to DO NOT DELETE ... comment are specific to the C Parser.
  They will be commennted out when building a java parser.
*/

%pure-parser
%lex-param {CRCEparsestate* parsestate}
%parse-param {CRCEparsestate* parsestate}
%{
#include "config.h"
#include "CRCEparselex.h"
%}

/*DO NOT DELETE THIS LINE*/

%token  WORD
%token  STRINGCONST
%token  NUMBERCONST

%start constraints:

%%

constraints:
	projectionlist {constraints(parsestate,$1);}
	;

projectionlist:
	  projection
	    {$$=projectionlist(parsestate,(Object)null,$1);}
	| projectionlist ',' projection
	    {$$=projectionlist(parsestate,$1,$3);}
	;

projection:
	  segmentlist
	    {$$=projection(parsestate,$1);}

segmentlist:
	  segment
	    {$$=segmentlist(parsestate,null,$1);}
	| segmentlist '.' segment
	    {$$=segmentlist(parsestate,$1,$3);}
	;

segment:
	  word
	    {$$=segment(parsestate,$1,null);}
	| word rangelist
	    {$$=segment(parsestate,$1,$2);}
	;

rangelist: 
	  range
	    {$$=rangelist(parsestate,null,$1);}
        | rangelist range
	    {$$=rangelist(parsestate,$1,$2);}
	;

range:
	  '(' number ')'
	    {$$=range(parsestate,$2,null,null);}
	| '(' number ':' number ')'
	    {$$=range(parsestate,$2,$4,null);}
	| '(' number ':' number ':' number ')'
	    {$$=range(parsestate,$2,$4,$6);}
	;

ident:  word
	    {$$ = $1;}
	;

word:  WORD
	    {$$ = checkobject($1);}
	;

number:  NUMBERCONST
	    {$$ = checkobject($1);}
	;

string: STRINGCONST
	    {$$ = checkobject($1);}
	;

%%
