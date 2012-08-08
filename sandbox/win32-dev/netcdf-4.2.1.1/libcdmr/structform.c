/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/offsets.c,v 1.1 2009/09/25 18:22:40 dmh Exp $
 *********************************************************************/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
This code is a variantion of the H5detect.c code from HDF5.
Author: D. Heimbigner 10/7/2008
*/

#ifndef OFFSETTEST
#include        "includes.h"
#else
#include        <stdlib.h>
#include        <string.h>
#include        <assert.h>
#endif


#ifdef OFFSETTEST
typedef int nc_type;
typedef struct nc_vlen_t {
    size_t len;
    void* p;
} nc_vlen_t;

#define	NC_NAT 	        0	/* NAT = 'Not A Type' (c.f. NaN) */
#define	NC_BYTE         1	/* signed 1 byte integer */
#define	NC_CHAR 	2	/* ISO/ASCII character */
#define	NC_SHORT 	3	/* signed 2 byte integer */
#define	NC_INT 	        4	/* signed 4 byte integer */
#define	NC_FLOAT 	5	/* single precision floating point number */
#define	NC_DOUBLE 	6	/* double precision floating point number */
#define	NC_UBYTE 	7	/* unsigned 1 byte int */
#define	NC_USHORT 	8	/* unsigned 2-byte int */
#define	NC_UINT 	9	/* unsigned 4-byte int */
#define	NC_INT64 	10	/* signed 8-byte int */
#define	NC_UINT64 	11	/* unsigned 8-byte int */
#define	NC_STRING 	12	/* string */
#endif

#include        "offsets.h"


/*
The heart of this is the following macro,
which computes the offset of a field x
when preceded by a char field.
The assumptions appear to be as follows:
1. the offset produced in this situation indicates
   the alignment for x relative in such a way that it
   depends only on the types that precede it in the struct.
2. the compiler does not reorder fields.
3. arrays are tightly packed.
4. nested structs are alignd according to their first member
   (this actually follows from C language requirement that
    a struct can legally be cast to an instance of its first member).
Given the alignments for the various common primitive types,
it is assumed that one can use them anywhere to construct
the layout of a struct of such types.
It seems to work for HDF5 for a wide variety of machines.
*/

#define COMP_ALIGNMENT(DST,TYPE)  {\
    struct {char f1; TYPE x;} tmp; \
    DST.typename = #TYPE ;        \
    DST.alignment = (size_t)((char*)(&(tmp.x)) - (char*)(&tmp));}


char* ctypenames[NCTYPES] = {
(char*)NULL,
"char","unsigned char",
"short","unsigned short",
"int","unsigned int",
"long","unsigned long",
"long long","unsigned long long",
"float","double",
"void*","nc_vlen_t"
};

static Typealignvec vec[NCTYPES];
static Typealignset set;

unsigned int
nctypealignment(nc_type nctype)
{
    Alignment* align = NULL;
    int index = 0;
    switch (nctype) {
      case NC_BYTE: index = UCHARINDEX; break;
      case NC_CHAR: index = CHARINDEX; break;
      case NC_SHORT: index = SHORTINDEX; break;
      case NC_INT: index = INTINDEX; break;
      case NC_FLOAT: index = FLOATINDEX; break;
      case NC_DOUBLE: index = DOUBLEINDEX; break;
      case NC_UBYTE: index = UCHARINDEX; break;
      case NC_USHORT: index = USHORTINDEX; break;
      case NC_UINT: index = UINTINDEX; break;
      case NC_INT64: index = LONGLONGINDEX; break;
      case NC_UINT64: index = ULONGLONGINDEX; break;
      case NC_STRING: index = PTRINDEX; break;
      case NC_VLEN: index = NCVLENINDEX; break;
      case NC_OPAQUE: index = UCHARINDEX; break;
      default: PANIC1("nctypealignment: bad type code: %d",nctype);
    }
    align = &vec[index];
    return align->alignment;
}


void
compute_alignments(void)
{
    /* Compute the alignments for all the common C data types*/
    /* First for the struct*/
    /* initialize*/
    memset((void*)&set,0,sizeof(set));
    memset((void*)vec,0,sizeof(vec));

    COMP_ALIGNMENT(set.charalign,char);
    COMP_ALIGNMENT(set.ucharalign,unsigned char);
    COMP_ALIGNMENT(set.shortalign,short);
    COMP_ALIGNMENT(set.ushortalign,unsigned short);
    COMP_ALIGNMENT(set.intalign,int);
    COMP_ALIGNMENT(set.uintalign,unsigned int);
    COMP_ALIGNMENT(set.longalign,long);
    COMP_ALIGNMENT(set.ulongalign,unsigned long);
    COMP_ALIGNMENT(set.longlongalign,long long);
    COMP_ALIGNMENT(set.ulonglongalign,unsigned long long);
    COMP_ALIGNMENT(set.floatalign,float);
    COMP_ALIGNMENT(set.doublealign,double);
    COMP_ALIGNMENT(set.ptralign,void*);
    COMP_ALIGNMENT(set.ncvlenalign,nc_vlen_t);

    /* Then the vector*/
    COMP_ALIGNMENT(vec[CHARINDEX],char);
    COMP_ALIGNMENT(vec[UCHARINDEX],unsigned char); 
    COMP_ALIGNMENT(vec[SHORTINDEX],short);
    COMP_ALIGNMENT(vec[USHORTINDEX],unsigned short);
    COMP_ALIGNMENT(vec[INTINDEX],int);
    COMP_ALIGNMENT(vec[UINTINDEX],unsigned int);
    COMP_ALIGNMENT(vec[LONGINDEX],long);
    COMP_ALIGNMENT(vec[ULONGINDEX],unsigned long);
    COMP_ALIGNMENT(vec[LONGLONGINDEX],long long);
    COMP_ALIGNMENT(vec[ULONGLONGINDEX],unsigned long long);
    COMP_ALIGNMENT(vec[FLOATINDEX],float);
    COMP_ALIGNMENT(vec[DOUBLEINDEX],double);
    COMP_ALIGNMENT(vec[PTRINDEX],void*);
    COMP_ALIGNMENT(vec[NCVLENINDEX],nc_vlen_t);
}

/* Compute type sizes and compound offsets*/
void
nccr_computesize(Structure* tsym, )
{

}

typedef struct Fieldform {
    size_t offset;
    size_t alignment;
    nclist* nelems; /* product of dimensions */
} Fieldform;

typedef struct Structureform {
    Structure* structure;
    nclist* fieldforms;
} Structureform;

static Structureform
lookup(nclist formlist, Structure* sym)
{
    int i;
    for(i=0;i<nclistlength(formlist);i++) {
	Structureform form = (Structureform)nclistget(formlist,i);
	if(form->structure == sym) return form;
    }
    return NULL;
}

/* Compute type sizes and compound offsets*/
int
nccr_computeform(Structure* tsym, nclist* formlist)
{
    int i;
    int offset = 0;
    unsigned long totaldimsize;

    if(lookup(formlist,tsym) != NULL) return; /* already processed */

    Structureform* form = (Structureform*)calloc(1,sizeof(Structureform));
    if(form == NULL) return NC_ENOMEM;
    form->structure = tsym;
    form->fieldforms = nclistnew();
    if(form->fieldforms == NULL) return NC_ENOMEM;


    /* Compute simple fields info */
    for(i=0;i<tsym->vars.count;i++) {
	Variable* v = tsym->vars.values[i];
	Fieldform* ff = (Fieldform*)calloc(1,sizeof(Fieldform));
        if(form == NULL) return NC_ENOMEM;
        ff->size = crsize(cvttypecode(v->dataType);
        ff->alignment = crtypealignment(cvttypecode(v->dataType));
	ff->offset = 0; /* compute later */
	ff->nelems = crshapeproduct(v->shape.count,v->shape.values);
	nclistpush(form->fieldforms,(nclem)ff);
    }

    /* Compute structured fields info */
    for(i=0;i<tsym->structs.count;i++) {
	Structure* struc = tsym->structs.values[i];
	Fieldform* ff = NULL;
	Structureform sf = NULL;
        if(form == NULL) return NC_ENOMEM;
	ncstatus = nccr_computeform(struc,formlist) /*recurse*/
	if(ncstatus != NC_NOERR) goto done;

	sf = lookup(formlist,struc);

	ff = (Fieldform*)calloc(1,sizeof(Fieldform));
        ff->size = crsize(cvttypecode(v->dataType);
        ff->alignment = crtypealignment(cvttypecode(v->dataType));
	ff->offset = 0; /* compute later */
	ff->nelems = crshapeproduct(v->shape.count,v->shape.values);
	nclistpush(form->fieldforms,(nclem)ff);
    }





    switch (tsym->subclass) {
        case NC_VLEN: /* actually two sizes for vlen*/
	    computesize(tsym->typ.basetype); /* first size*/
	    tsym->typ.size = ncsize(tsym->typ.typecode);
	    tsym->typ.alignment = nctypealignment(tsym->typ.typecode);
	    tsym->typ.nelems = 1; /* always a single compound datalist */
	    break;
	case NC_PRIM:
	    tsym->typ.size = ncsize(tsym->typ.typecode);
	    tsym->typ.alignment = nctypealignment(tsym->typ.typecode);
	    tsym->typ.nelems = 1;
	    break;
	case NC_OPAQUE:
	    /* size and alignment already assigned*/
	    tsym->typ.nelems = 1;
	    break;
	case NC_ENUM:
	    computesize(tsym->typ.basetype); /* first size*/
	    tsym->typ.size = tsym->typ.basetype->typ.size;
	    tsym->typ.alignment = tsym->typ.basetype->typ.alignment;
	    tsym->typ.nelems = 1;
	    break;
	case NC_COMPOUND: /* keep if all fields are primitive*/
	    /* First, compute recursively, the size and alignment of fields*/
	    for(i=0;i<listlength(tsym->subnodes);i++) {
		Symbol* field = (Symbol*)listget(tsym->subnodes,i);
		ASSERT(field->subclass == NC_FIELD);
		computesize(field);
		/* alignment of struct is same as alignment of first field*/
		if(i==0) tsym->typ.alignment = field->typ.alignment;
	    }	  
	    /* now compute the size of the compound based on*/
	    /* what user specified*/
	    offset = 0;
	    for(i=0;i<listlength(tsym->subnodes);i++) {
		Symbol* field = (Symbol*)listget(tsym->subnodes,i);
		/* only support 'c' alignment for now*/
		int alignment = field->typ.alignment;
		offset += getpadding(offset,alignment);
		field->typ.offset = offset;
		offset += field->typ.size;
	    }
	    tsym->typ.size = offset;
	    break;
        case NC_FIELD: /* Compute size of all non-unlimited dimensions*/
	    if(tsym->typ.dimset.ndims > 0) {
	        computesize(tsym->typ.basetype);
	        totaldimsize = arraylength(&tsym->typ.dimset);
	        tsym->typ.size = tsym->typ.basetype->typ.size * totaldimsize;
	        tsym->typ.alignment = tsym->typ.basetype->typ.alignment;
	        tsym->typ.nelems = 1;
	    } else {
	        tsym->typ.size = tsym->typ.basetype->typ.size;
	        tsym->typ.alignment = tsym->typ.basetype->typ.alignment;
	        tsym->typ.nelems = tsym->typ.basetype->typ.nelems;
	    }
	    break;
	default:
	    PANIC1("computesize: unexpected type class: %d",tsym->subclass);
	    break;
    }
}
