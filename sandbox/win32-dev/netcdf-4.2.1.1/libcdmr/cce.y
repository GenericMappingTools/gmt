/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

/*The lines down to DO NOT DELETE ... comment are specific to the C Parser.
  They will be commennted out when building a java parser.
*/

%pure-parser
%lex-param {CCEparsestate* parsestate}
%parse-param {CCEparsestate* parsestate}
%{
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include "netcdf.h"
#include "ncbytes.h"
#include "nclist.h"
#include "cceconstraints.h"
#include "cceparselex.h"
%}

/*DO NOT DELETE THIS LINE*/

%token  SCAN_WORD
%token  SCAN_STRINGCONST
%token  SCAN_NUMBERCONST

%start constraints

%%

constraints:
	  optquestionmark projections
	| /*empty*/
	;

optquestionmark: '?' | /*empty*/ ;

projections:
	projectionlist {projections(parsestate,$1);}
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
	;

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
	  '[' number ']'
	    {$$=range(parsestate,$2,null,null);}
	| '[' number ':' number ']'
	    {$$=range(parsestate,$2,null,$4);}
	| '[' number ':' number ':' number ']'
	    {$$=range(parsestate,$2,$4,$6);}
	|  '(' number ')'
	    {$$=range(parsestate,$2,null,null);}
	| '(' number ':' number ')'
	    {$$=range(parsestate,$2,null,$4);}
	| '(' number ':' number ':' number ')'
	    {$$=range(parsestate,$2,$6,$4);}
	;


word:
	  SCAN_WORD
	    {$$ = checkobject($1);}
	;

number:  SCAN_NUMBERCONST
	    {$$ = checkobject($1);}
	;

%%
