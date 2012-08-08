/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *********************************************************************/

/* yacc source for "ncgen", a netCDL parser and netCDF generator */

%{
#ifdef sccs
static char SccsId[] = "$Id$";
#endif
#include        <config.h>
#include        <string.h>
#include	<stdlib.h>
#include	<netcdf.h>
#include 	"generic.h"
#include        "ncgen.h"
#include	"genlib.h"	/* for grow_darray() et al */

typedef struct Symbol {		/* symbol table entry */
	char    	*name;
	struct Symbol   *next;
	unsigned	is_dim : 1;	/* appears as netCDF dimension */
	unsigned	is_var : 1;	/* appears as netCDF variable */
	unsigned	is_att : 1;	/* appears as netCDF attribute */
	int             dnum;	        /* handle as a dimension */
	int             vnum;	        /* handle as a variable */
	} *YYSTYPE1;

/* True if string a equals string b*/
#define	STREQ(a, b)	(*(a) == *(b) && strcmp((a), (b)) == 0)
#define NC_UNSPECIFIED ((nc_type)0)	/* unspecified (as yet) type */

#define YYSTYPE YYSTYPE1
YYSTYPE symlist;		/* symbol table: linked list */

extern int derror_count;	/* counts errors in netcdf definition */
extern int lineno;		/* line number for error messages */

static int not_a_string;	/* whether last constant read was a string */
static char termstring[MAXTRST]; /* last terminal string read */
static double double_val;	/* last double value read */
static float float_val;		/* last float value read */
static int int_val;		/* last int value read */
static short short_val;		/* last short value read */
static char char_val;		/* last char value read */
static signed char byte_val;	/* last byte value read */

static nc_type type_code;	/* holds declared type for variables */
static nc_type atype_code;	/* holds derived type for attributes */
static char *netcdfname;	/* to construct netcdf file name */
static void *att_space;		/* pointer to block for attribute values */
static nc_type valtype;		/* type code for list of attribute values  */

static char *char_valp;		/* pointers used to accumulate data values */
static signed char *byte_valp;
static short *short_valp;
static int *int_valp;
static float *float_valp;
static double *double_valp;
static void *rec_cur;		/* pointer to where next data value goes */
static void *rec_start;		/* start of space for data */

/* Forward declarations */
void defatt();
void equalatt();

#ifdef YYLEX_PARAM
int yylex(YYLEX_PARAM);
#else
int yylex();
#endif

#ifdef vms
void yyerror(char*);
#else
int yyerror(char*);
#endif
%}

/* DECLARATIONS */

%token
	NC_UNLIMITED_K /* keyword for unbounded record dimension */
	BYTE_K	    /* keyword for byte datatype */
	CHAR_K	    /* keyword for char datatype */
	SHORT_K	    /* keyword for short datatype */
	INT_K	    /* keyword for int datatype */
	FLOAT_K	    /* keyword for float datatype */
	DOUBLE_K    /* keyword for double datatype */
	IDENT	    /* name for a dimension, variable, or attribute */
	TERMSTRING  /* terminal string */
	BYTE_CONST  /* byte constant */
	CHAR_CONST  /* char constant */
	SHORT_CONST /* short constant */
	INT_CONST   /* int constant */
	FLOAT_CONST /* float constant */
	DOUBLE_CONST /* double constant */
	DIMENSIONS  /* keyword starting dimensions section, if any */
	VARIABLES   /* keyword starting variables section, if any */
	NETCDF      /* keyword declaring netcdf name */
	DATA        /* keyword starting data section, if any */
        FILLVALUE   /* fill value, from _FillValue attribute or default */

%start	ncdesc /* start symbol for grammar */

%%

/* RULES */

ncdesc:	NETCDF
		'{'
		   { init_netcdf(); }
                dimsection	/* dimension declarations */
                vasection	/* variable and attribute declarations */
		   {
		       if (derror_count == 0)
			 define_netcdf(netcdfname);
		       if (derror_count > 0)
			   exit(6);
		   }
		datasection     /* data, variables loaded as encountered */
                '}'
		   {
		       if (derror_count == 0)
			 close_netcdf();
		   }
		;
dimsection:     /* empty */
		| DIMENSIONS dimdecls
		;
dimdecls:       dimdecline ';'
		| dimdecls dimdecline ';'
		;
dimdecline:     dimdecl
                | dimdecline ',' dimdecl
                ;
dimdecl:        dimd '=' INT_CONST
		   { if (int_val <= 0)
			 derror("dimension length must be positive");
		     dims[ndims].size = int_val;
		     ndims++;
		   }
                | dimd '=' DOUBLE_CONST
                   { /* for rare case where 2^31 < dimsize < 2^32 */
		       if (double_val <= 0)
			 derror("dimension length must be positive");
		       if (double_val > 4294967295.0)
			 derror("dimension too large");
		       if (double_val - (size_t) double_val > 0)
			 derror("dimension length must be an integer");
		       dims[ndims].size = (size_t) double_val;
		       ndims++;
                   }
                | dimd '=' NC_UNLIMITED_K
		   {  if (rec_dim != -1)
			 derror("only one NC_UNLIMITED dimension allowed");
		     rec_dim = ndims; /* the unlimited (record) dimension */
		     dims[ndims].size = NC_UNLIMITED;
		     ndims++;
		   }
                ;
dimd:           dim
		   { 
		    if ($1->is_dim == 1) {
		        derror( "duplicate dimension declaration for %s",
		                $1->name);
		     }
	             $1->is_dim = 1;
		     $1->dnum = ndims;
		     /* make sure dims array will hold dimensions */
		     grow_darray(ndims,  /* must hold ndims+1 dims */
				 &dims); /* grow as needed */
		     dims[ndims].name = (char *) emalloc(strlen($1->name)+1);
		     (void) strcpy(dims[ndims].name, $1->name);
		     /* name for use in generated Fortran and C variables */
		     dims[ndims].lname = decodify($1->name);
		   }
                ;
dim:		IDENT
		;
vasection:      /* empty */
		| VARIABLES vadecls
		| gattdecls
		;
vadecls:        vadecl ';'
                | vadecls vadecl ';'
                ;
vadecl:         vardecl | attdecl | gattdecl
                ;
gattdecls:      gattdecl ';'
                | gattdecls gattdecl ';'
                ;
vardecl:        type varlist
                ;
type:             BYTE_K  { type_code = NC_BYTE; }
		| CHAR_K  { type_code = NC_CHAR; }
		| SHORT_K { type_code = NC_SHORT; }
		| INT_K   { type_code = NC_INT; }
		| FLOAT_K { type_code = NC_FLOAT; }
		| DOUBLE_K{ type_code = NC_DOUBLE; }
		;
varlist:        varspec
                | varlist ',' varspec
                ;
varspec:        var
		   {
		    static struct vars dummyvar;

		    dummyvar.name = "dummy";
		    dummyvar.type = NC_DOUBLE;
		    dummyvar.ndims = 0;
		    dummyvar.dims = 0;
		    dummyvar.fill_value.doublev = NC_FILL_DOUBLE;
		    dummyvar.has_data = 0;

		    nvdims = 0;
		    /* make sure variable not re-declared */
		    if ($1->is_var == 1) {
		       derror( "duplicate variable declaration for %s",
		               $1->name);
		    }
	            $1->is_var = 1;
		    $1->vnum = nvars;
		    /* make sure vars array will hold variables */
		    grow_varray(nvars,  /* must hold nvars+1 vars */
				&vars); /* grow as needed */
		    vars[nvars] = dummyvar; /* to make Purify happy */
		    vars[nvars].name = (char *) emalloc(strlen($1->name)+1);
		    (void) strcpy(vars[nvars].name, $1->name);
		    /* name for use in generated Fortran and C variables */
		    vars[nvars].lname = decodify($1->name);
		    vars[nvars].type = type_code;
		    /* set default fill value.  You can override this with
		     * the variable attribute "_FillValue". */
		    nc_getfill(type_code, &vars[nvars].fill_value);
		    vars[nvars].has_data = 0; /* has no data (yet) */
		   }
		dimspec
		   {
		    vars[nvars].ndims = nvdims;
		    nvars++;
		   }
		;
var:            IDENT
                ;
dimspec:	/* empty */
		| '(' dimlist ')'
		;
dimlist:        vdim
                | dimlist ',' vdim
                ;
vdim:		dim
		   {
		    if (nvdims >= NC_MAX_VAR_DIMS) {
		       derror("%s has too many dimensions",vars[nvars].name);
		    }
		    if ($1->is_dim == 1)
		       dimnum = $1->dnum;
		    else {
		       derror( "%s is not declared as a dimension",
			       $1->name);
	               dimnum = ndims;
		    }
		    if (rec_dim != -1 && dimnum == rec_dim && nvdims != 0) {
		       derror("unlimited dimension must be first");
		    }
		    grow_iarray(nvdims, /* must hold nvdims+1 ints */
				&vars[nvars].dims); /* grow as needed */
		    vars[nvars].dims[nvdims] = dimnum;
                    nvdims++;
		   }
		;
attdecl:        att
		   {
                   defatt();
		   }
		'=' attvallist
		   {
                   equalatt();
		   }
                ;
gattdecl:       gatt
		   {
                   defatt();
		   }
		'=' attvallist
		   {
                   equalatt();
		   }
                ;

att:            avar ':' attr

gatt:           ':' attr
		   {
		    varnum = NC_GLOBAL;  /* handle of "global" attribute */
		   }
                ;

avar:           var
		   { if ($1->is_var == 1)
		       varnum = $1->vnum;
		    else {
		      derror("%s not declared as a variable, fatal error",
			     $1->name);
		      YYABORT;
		      }
		   }
		;
attr:		IDENT
		   {
		       /* make sure atts array will hold attributes */
		       grow_aarray(natts,  /* must hold natts+1 atts */
				   &atts); /* grow as needed */
		       atts[natts].name = (char *) emalloc(strlen($1->name)+1);
		       (void) strcpy(atts[natts].name,$1->name);
		       /* name for use in generated Fortran and C variables */
		       atts[natts].lname = decodify($1->name);
		   }
		;
attvallist:     aconst
                | attvallist ',' aconst
                ;
aconst:		attconst
		   {
		    if (valtype == NC_UNSPECIFIED)
		      valtype = atype_code;
		    if (valtype != atype_code)
		      derror("values for attribute must be all of same type");
		   }
		;

attconst:      CHAR_CONST
                   {
		       atype_code = NC_CHAR;
		       *char_valp++ = char_val;
		       valnum++;
		   }
	       | TERMSTRING
		   {
		       atype_code = NC_CHAR;
		       {
			   /* don't null-terminate attribute strings */
			   size_t len = strlen(termstring);
			   if (len == 0) /* need null if that's only value */
			       len = 1;
			   (void)strncpy(char_valp,termstring,len);
			   valnum += len;
			   char_valp += len;
		       }
		   }
                | BYTE_CONST
                   {
		       atype_code = NC_BYTE;
		       *byte_valp++ = byte_val;
		       valnum++;
		   }
                | SHORT_CONST
                   {
		       atype_code = NC_SHORT;
		       *short_valp++ = short_val;
		       valnum++;
		   }
                | INT_CONST
                   {
		       atype_code = NC_INT;
		       *int_valp++ = int_val;
		       valnum++;
		   }
                | FLOAT_CONST
                   {
		       atype_code = NC_FLOAT;
		       *float_valp++ = float_val;
		       valnum++;
		   }
                | DOUBLE_CONST
                   {
		       atype_code = NC_DOUBLE;
		       *double_valp++ = double_val;
		       valnum++;
		   }
                ;

datasection:    /* empty */
		| DATA datadecls
		| DATA
		;

datadecls:      datadecl ';'
                | datadecls datadecl ';'
                ;
datadecl:       avar
		   {
		       valtype = vars[varnum].type; /* variable type */
		       valnum = 0;	/* values accumulated for variable */
		       vars[varnum].has_data = 1;
		       /* compute dimensions product */
		       var_size = nctypesize(valtype);
		       if (vars[varnum].ndims == 0) { /* scalar */
			   var_len = 1;
		       } else if (vars[varnum].dims[0] == rec_dim) {
			   var_len = 1; /* one record for unlimited vars */
		       } else {
			   var_len = dims[vars[varnum].dims[0]].size;
		       }
		       for(dimnum = 1; dimnum < vars[varnum].ndims; dimnum++)
			 var_len = var_len*dims[vars[varnum].dims[dimnum]].size;
		       /* allocate memory for variable data */
		       if (var_len*var_size != (size_t)(var_len*var_size)) {
			   derror("variable %s too large for memory",
				  vars[varnum].name);
			   exit(9);
		       }
		       rec_len = var_len;
		       rec_start = malloc ((size_t)(rec_len*var_size));
		       if (rec_start == 0) {
			   derror ("out of memory\n");
			   exit(3);
		       }
		       rec_cur = rec_start;
		       switch (valtype) {
			 case NC_CHAR:
			   char_valp = (char *) rec_start;
			   break;
			 case NC_BYTE:
			   byte_valp = (signed char *) rec_start;
			   break;
			 case NC_SHORT:
			   short_valp = (short *) rec_start;
			   break;
			 case NC_INT:
			   int_valp = (int *) rec_start;
			   break;
			 case NC_FLOAT:
			   float_valp = (float *) rec_start;
			   break;
			 case NC_DOUBLE:
			   double_valp = (double *) rec_start;
			   break;
			 default: break;
		       }
		 }
		'=' constlist
                   {
		       if (valnum < var_len) { /* leftovers */
			   nc_fill(valtype,
				    var_len - valnum,
				    rec_cur,
				    vars[varnum].fill_value);
		       }
		       /* put out var_len values */
		       /* vars[varnum].nrecs = valnum / rec_len; */
		       vars[varnum].nrecs = var_len / rec_len;
		       if (derror_count == 0)
			   put_variable(rec_start);
		       free ((char *) rec_start);
		 }
                ;
constlist:      dconst
                | constlist ',' dconst
                ;
dconst:
                   {
		       if(valnum >= var_len) {
			   if (vars[varnum].dims[0] != rec_dim) { /* not recvar */
			       derror("too many values for this variable, %d >= %d",
				      valnum, var_len);
			       exit (4);
			   } else { /* a record variable, so grow data
				      container and increment var_len by
				      multiple of record size */
			       ptrdiff_t rec_inc = (char *)rec_cur
				   - (char *)rec_start;
			       var_len = rec_len * (1 + valnum / rec_len);
			       rec_start = erealloc(rec_start, var_len*var_size);
			       rec_cur = (char *)rec_start + rec_inc;
			       char_valp = (char *) rec_cur;
			       byte_valp = (signed char *) rec_cur;
			       short_valp = (short *) rec_cur;
			       int_valp = (int *) rec_cur;
			       float_valp = (float *) rec_cur;
			       double_valp = (double *) rec_cur;
			   }
		       }
		       not_a_string = 1;
                   }
                const
		   {
		       if (not_a_string) {
			   switch (valtype) {
			     case NC_CHAR:
			       rec_cur = (void *) char_valp;
			       break;
			     case NC_BYTE:
			       rec_cur = (void *) byte_valp;
			       break;
			     case NC_SHORT:
			       rec_cur = (void *) short_valp;
			       break;
			     case NC_INT:
			       rec_cur = (void *) int_valp;
			       break;
			     case NC_FLOAT:
			       rec_cur = (void *) float_valp;
			       break;
			     case NC_DOUBLE:
			       rec_cur = (void *) double_valp;
			       break;
			     default: break;
			   }
		       }
		   }
;

const:         CHAR_CONST
                   {
		       atype_code = NC_CHAR;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = char_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = char_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = char_val;
			   break;
			 case NC_INT:
			   *int_valp++ = char_val;
			   break;
			 case NC_FLOAT:
			   *float_valp++ = char_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = char_val;
			   break;
			 default: break;
		       }
		       valnum++;
		   }
	       | TERMSTRING
		   {
		       not_a_string = 0;
		       atype_code = NC_CHAR;
		       {
			   size_t len = strlen(termstring);

			   if(valnum + len > var_len) {
			       if (vars[varnum].dims[0] != rec_dim) {
				   derror("too many values for this variable, %d>%d", 
					  valnum+len, var_len);
				   exit (5);
			       } else {/* a record variable so grow it */
				   ptrdiff_t rec_inc = (char *)rec_cur
				       - (char *)rec_start;
				   var_len += rec_len * (len + valnum - var_len)/rec_len;
				   rec_start = erealloc(rec_start, var_len*var_size);
				   rec_cur = (char *)rec_start + rec_inc;
				   char_valp = (char *) rec_cur;
			       }
			   }
			   switch (valtype) {
			     case NC_CHAR:
			       {
				   int ld;
				   size_t i, sl;
				   (void)strncpy(char_valp,termstring,len);
				   ld = vars[varnum].ndims-1;
				   if (ld > 0) {/* null-fill to size of last dim */
				       sl = dims[vars[varnum].dims[ld]].size;
				       for (i =len;i<sl;i++)
					   char_valp[i] = '\0';
				       if (sl < len)
					   sl = len;
				       valnum += sl;
				       char_valp += sl;
				   } else { /* scalar or 1D strings */
				       valnum += len;
				       char_valp += len;
				   }
				   rec_cur = (void *) char_valp;
			       }
			       break;
			     case NC_BYTE:
			     case NC_SHORT:
			     case NC_INT:
			     case NC_FLOAT:
			     case NC_DOUBLE:
			       derror("string value invalid for %s variable",
				      nctype(valtype));
			       break;
			     default: break;
			   }
		       }
		   }
                | BYTE_CONST
                   {
		       atype_code = NC_BYTE;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = byte_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = byte_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = byte_val;
			   break;
			 case NC_INT:
			   *int_valp++ = byte_val;
			   break;
			 case NC_FLOAT:
			   *float_valp++ = byte_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = byte_val;
			   break;
			 default: break;
		       }
		       valnum++;
		   }
                | SHORT_CONST
                   {
		       atype_code = NC_SHORT;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = short_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = short_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = short_val;
			   break;
			 case NC_INT:
			   *int_valp++ = short_val;
			   break;
			 case NC_FLOAT:
			   *float_valp++ = short_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = short_val;
			   break;
			 default: break;			
		       }
		       valnum++;
		   }
                | INT_CONST
                   {
		       atype_code = NC_INT;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = int_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = int_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = int_val;
			   break;
			 case NC_INT:
			   *int_valp++ = int_val;
			   break;
			 case NC_FLOAT:
			   *float_valp++ = int_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = int_val;
			   break;
			 default: break;
		       }
		       valnum++;
		   }
                | FLOAT_CONST
                   {
		       atype_code = NC_FLOAT;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = float_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = float_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = float_val;
			   break;
			 case NC_INT:
			   *int_valp++ = float_val;
			   break;
			 case NC_FLOAT:
			   *float_valp++ = float_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = float_val;
			   break;
			 default: break;
		       }
		       valnum++;
		   }
                | DOUBLE_CONST
                   {
		       atype_code = NC_DOUBLE;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = double_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = double_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = double_val;
			   break;
			 case NC_INT:
			   *int_valp++ = double_val;
			   break;
			 case NC_FLOAT:
			   if (double_val == NC_FILL_DOUBLE)
			     *float_valp++ = NC_FILL_FLOAT;
			   else
			     *float_valp++ = double_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = double_val;
			   break;
			 default: break;
		       }
		       valnum++;
		   }
                | FILLVALUE
                   {
		       /* store fill_value */
		       switch (valtype) {
		       case NC_CHAR:
			   nc_fill(valtype, 1, (void *)char_valp++,
				   vars[varnum].fill_value);
			   break;
		       case NC_BYTE:
			   nc_fill(valtype, 1, (void *)byte_valp++,
				   vars[varnum].fill_value);
			   break;
		       case NC_SHORT:
			   nc_fill(valtype, 1, (void *)short_valp++,
				   vars[varnum].fill_value);
			   break;
		       case NC_INT:
			   nc_fill(valtype, 1, (void *)int_valp++,
				   vars[varnum].fill_value);
			   break;
		       case NC_FLOAT:
			   nc_fill(valtype, 1, (void *)float_valp++,
				   vars[varnum].fill_value);
			   break;
		       case NC_DOUBLE:
			   nc_fill(valtype, 1, (void *)double_valp++,
				   vars[varnum].fill_value);
			   break;
			default: break;
		       }
		       valnum++;
		   }
                ;

/* END OF RULES */

%%

/* HELPER PROGRAMS */
void defatt()
{
    valnum = 0;
    valtype = NC_UNSPECIFIED;
    /* get a large block for attributes, realloc later */
    att_space = emalloc(MAX_NC_ATTSIZE);
    /* make all kinds of pointers point to it */
    char_valp = (char *) att_space;
    byte_valp = (signed char *) att_space;
    short_valp = (short *) att_space;
    int_valp = (int *) att_space;
    float_valp = (float *) att_space;
    double_valp = (double *) att_space;
}

void equalatt()
{
    /* check if duplicate attribute for this var */
    int i;
    for(i=0; i<natts; i++) { /* expensive */
        if(atts[i].var == varnum &&
           STREQ(atts[i].name,atts[natts].name)) {
            derror("duplicate attribute %s:%s",
                   vars[varnum].name,atts[natts].name);
        }
    }
    atts[natts].var = varnum ;
    atts[natts].type = valtype;
    atts[natts].len = valnum;
    /* shrink space down to what was really needed */
    att_space = erealloc(att_space, valnum*nctypesize(valtype));
    atts[natts].val = att_space;
    if (STREQ(atts[natts].name, _FillValue) &&
        atts[natts].var != NC_GLOBAL) {
        nc_putfill(atts[natts].type,atts[natts].val,
                   &vars[atts[natts].var].fill_value);
        if(atts[natts].type != vars[atts[natts].var].type) {
            derror("variable %s: %s type mismatch",
                   vars[atts[natts].var].name, _FillValue);
        }
    }
    natts++;
}
/* PROGRAMS */

#ifdef vms
void
#else
int
#endif
yyerror(	/* called for yacc syntax error */
     char *s)
{
	derror(s);
#ifndef vms
	return -1;
#endif
}

/* undefine yywrap macro, in case we are using bison instead of yacc */
#ifdef yywrap
#undef yywrap
#endif

int
ncgwrap(void)			/* returns 1 on EOF if no more input */
{
    return  1;
}


/* Symbol table operations for ncgen tool */

/* Find CDL name in symbol table (linear search).  Note, this has a
 * side-effect: it handles escape characters in the name, deleting
 * single escape characters from the CDL name, before looking it up.
 */
YYSTYPE lookup(char *sname)
{
    YYSTYPE sp;
    deescapify(sname);		/* delete escape chars from names,
				 * e.g. 'ab\:cd\ ef' becomes 
				 * 'ab:cd ef' */
    for (sp = symlist; sp != (YYSTYPE) 0; sp = sp -> next)
	if (STREQ(sp -> name, sname)) {
	    return sp;
	}
    return 0;			/* 0 ==> not found */
}

YYSTYPE install(  /* install sname in symbol table */
	const char *sname)
{
    YYSTYPE sp;

    sp = (YYSTYPE) emalloc (sizeof (struct Symbol));
    sp -> name = (char *) emalloc (strlen (sname) + 1);/* +1 for '\0' */
    (void) strcpy (sp -> name, sname);
    sp -> next = symlist;	/* put at front of list */
    sp -> is_dim = 0;
    sp -> is_var = 0;
    sp -> is_att = 0;
    symlist = sp;
    return sp;
}

void
clearout(void)	/* reset symbol table to empty */
{
    YYSTYPE sp, tp;
    for (sp = symlist; sp != (YYSTYPE) 0;) {
	tp = sp -> next;
	free (sp -> name);
	free ((char *) sp);
	sp = tp;
    }
    symlist = 0;
}

/* get lexical input routine generated by lex  */

/* Keep compile quiet */
#define YY_NO_UNPUT
#define YY_NO_INPUT

#include "ncgenyy.c"
