/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

/*The lines down to DO NOT DELETE ... comment are specific to the C Parser.
  They will be commennted out when building a java parser.
*/

%pure-parser
%lex-param {DCEparsestate* parsestate}
%parse-param {DCEparsestate* parsestate}
%{
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include "netcdf.h"
#include "ncbytes.h"
#include "nclist.h"
#include "dceconstraints.h"
#include "dceparselex.h"
%}

/*DO NOT DELETE THIS LINE*/

%token  SCAN_WORD
%token  SCAN_STRINGCONST
%token  SCAN_NUMBERCONST

%start constraints

%%

constraints:
	  optquestionmark projections
	| optquestionmark selections
	| optquestionmark projections selections
	| /*empty*/
	;

optquestionmark: '?' | /*empty*/ ;

projections:
	projectionlist {projections(parsestate,$1);}
	;

selections:
	clauselist {selections(parsestate,$1);}
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
	| function
	    {$$=$1;}
	;

function:
	  ident '(' ')'
	    {$$=function(parsestate,$1,null);}
	| ident '(' arg_list ')'
	    {$$=function(parsestate,$1,$3);}
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
	;

range1: '[' number ']'
	    {$$ = range1(parsestate,$2);}
	;


clauselist:
	  sel_clause
	    {$$=clauselist(parsestate,null,$1);}
	| clauselist sel_clause
	    {$$=clauselist(parsestate,$1,$2);}
	;

sel_clause:
	  '&' value rel_op '{' value_list '}'
	    {$$=sel_clause(parsestate,1,$2,$3,$5);} /*1,2 distinguish cases*/
	| '&' value rel_op value
	    {$$=sel_clause(parsestate,2,$2,$3,$4);}
	| '&' boolfunction
	    {$$=$1;}
        ;

value_list:
	  value
	    {$$=value_list(parsestate,null,$1);}
	| value_list ',' value
	    {$$=value_list(parsestate,$1,$3);}
	;

value:
	  var
	    {$$=value(parsestate,$1);}
	| function
	    {$$=value(parsestate,$1);}
	| constant
	    {$$=value(parsestate,$1);}
	;

constant:
	  number
	    {$$=constant(parsestate,$1,SCAN_NUMBERCONST);}
	| string
	    {$$=constant(parsestate,$1,SCAN_STRINGCONST);}
	;

var:
	indexpath
	    {$$=var(parsestate,$1);}
	;

indexpath:
	  index
	    {$$=indexpath(parsestate,null,$1);}
	| indexpath '.' index
	    {$$=indexpath(parsestate,$1,$3);}
	;

index:
	  word
	    {$$=indexer(parsestate,$1,null);}
	| word array_indices
	    {$$=indexer(parsestate,$1,$2);}
	;

array_indices:
	  range1
	    {$$=array_indices(parsestate,null,$1);}
        | array_indices range1
	    {$$=array_indices(parsestate,$1,$2);}
	;

boolfunction:
	  ident '(' ')'
	    {$$=function(parsestate,$1,null);}
	| ident '(' arg_list ')'
	    {$$=function(parsestate,$1,$3);}
	;

arg_list:
	  value
	    {$$=arg_list(parsestate,null,$1);}
	| value_list ',' value
	    {$$=arg_list(parsestate,$1,$3);}
	;

rel_op:
	  '='     {$$=makeselectiontag(CEO_EQ);}
	| '>'     {$$=makeselectiontag(CEO_GT);}
	| '<'     {$$=makeselectiontag(CEO_LT);}
	| '!' '=' {$$=makeselectiontag(CEO_NEQ);}
	| '>' '=' {$$=makeselectiontag(CEO_GE);}
	| '<' '=' {$$=makeselectiontag(CEO_LE);}
	| '=' '~' {$$=makeselectiontag(CEO_RE);}
	;

ident:  word
	    {$$ = $1;}
	;

word:  SCAN_WORD
	    {$$ = checkobject($1);}
	;

number:  SCAN_NUMBERCONST
	    {$$ = checkobject($1);}
	;

string: SCAN_STRINGCONST
	    {$$ = checkobject($1);}
	;

%%
