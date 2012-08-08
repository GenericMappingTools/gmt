/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/getfill.c,v 1.8 2010/04/14 22:04:56 dmh Exp $
 *********************************************************************/

#include "includes.h"
#include "dump.h"

/* mnemonic*/
#define TOPLEVEL 1

/*Forward*/
static void fill(Symbol* tsym, Datalist*);
static void fillarray(Symbol* tsym, Dimset* dimset, int index, Datalist*);
static void filllist(Symbol* tvsym, Datalist* dl);

/* Construct a Datalist representing a complete fill value*/
/* for a specified variable */
/* Cache if needed later*/

Datalist*
getfiller(Symbol* tvsym)
{
    Datalist* filler = NULL;
    Symbol* tsym = tvsym;
    ASSERT(tvsym->objectclass == NC_VAR || tvsym->objectclass == NC_TYPE);
    if(tvsym->objectclass == NC_VAR) {
	tsym = tvsym->typ.basetype;
        filler = tvsym->var.special._Fillvalue; /* get cached value (if any)*/
    }
    if(filler == NULL || tvsym->objectclass == NC_TYPE) {
        filler = tvsym->typ._Fillvalue; /* get cached value (if any)*/
    }
    if(filler == NULL) { /* need to compute it*/
        filler = builddatalist(0);
	fill(tsym,filler);
    }
#ifdef DEBUG2
    dumpdatalist(filler,"getfiller");
#endif
    if(tvsym->objectclass == NC_VAR) {
        tvsym->var.special._Fillvalue = filler;
    } else if(tvsym->objectclass == NC_TYPE) {
        tvsym->typ._Fillvalue = filler; /* cache value*/
    }
    return filler;        
}

static void
fill(Symbol* tsym, Datalist* filler)
{
    int i;
    Constant con;
    Datalist* sublist;
    ASSERT(tsym->objectclass == NC_TYPE);
    switch (tsym->subclass) {
    case NC_ENUM: case NC_OPAQUE: case NC_PRIM:
        con.nctype = tsym->typ.typecode;
        nc_getfill(&con);
	break;
    case NC_COMPOUND:
	sublist = builddatalist(listlength(tsym->subnodes));
        for(i=0;i<listlength(tsym->subnodes);i++) {
	    Symbol* field = (Symbol*)listget(tsym->subnodes,i);
	    if(field->typ.dimset.ndims > 0) {	
                fillarray(field->typ.basetype,&field->typ.dimset,0,filler);
	    } else
		filllist(field->typ.basetype,sublist);
        }	  
	con = builddatasublist(sublist);
	break;
    case NC_VLEN:
	sublist = builddatalist(0);
	filllist(tsym->typ.basetype,sublist); /* generate a single instance*/
	con = builddatasublist(sublist);
	break;
    default: PANIC1("fill: unexpected subclass %d",tsym->subclass);
    }
    dlappend(filler,&con);
}

static void
filllist(Symbol* tsym, Datalist* dl)
{
    int i;
    Datalist* sublist;
    Constant con;

    ASSERT(tsym->objectclass == NC_TYPE);
    switch (tsym->subclass) {
    case NC_ENUM: case NC_OPAQUE: case NC_PRIM:
        con.nctype = tsym->typ.typecode;
        nc_getfill(&con);
	dlappend(dl,&con);
	break;
    case NC_COMPOUND:
	sublist = builddatalist(listlength(tsym->subnodes));
        for(i=0;i<listlength(tsym->subnodes);i++) {
	    Symbol* field = (Symbol*)listget(tsym->subnodes,i);
	    filllist(field->typ.basetype,sublist);
        }	  
	con = builddatasublist(sublist);
	dlappend(dl,&con);
	break;
    case NC_VLEN:
	sublist = builddatalist(0);
	filllist(tsym->typ.basetype,sublist); /* generate a single instance*/
	con = builddatasublist(sublist);
	dlappend(dl,&con);
	break;
    default: PANIC1("fill: unexpected subclass %d",tsym->subclass);
    }
}

static void
fillarray(Symbol* basetype, Dimset* dimset, int index, Datalist* arraylist)
{
    int i;
    Symbol* dim = dimset->dimsyms[index];
    unsigned int size = dim->dim.declsize;
    int isunlimited = (size == 0);
    int lastdim = (index == (dimset->ndims - 1));
    int firstdim = (index == 0);
    Datalist* sublist;

    sublist = (firstdim?builddatalist(0):arraylist);
    if(isunlimited) {
	/* do a single entry to satisfy*/
        if(lastdim) {
	    filllist(basetype,sublist);
	} else {
	    fillarray(basetype->typ.basetype,dimset,index+1,sublist);
	}
    } else { /* bounded*/
        if(lastdim) {
	    for(i=0;i<size;i++) filllist(basetype,sublist);
	} else {
	    for(i=0;i<size;i++) {
	        fillarray(basetype->typ.basetype,dimset,index+1,sublist);
	    }
	}
    }
}

/*
 * Given primitive netCDF type, return a default fill_value appropriate for
 * that type.
 */
void
nc_getfill(Constant* value)
{
    switch(value->nctype) {
      case NC_CHAR: value->value.charv = NC_FILL_CHAR; break;
      case NC_BYTE: value->value.int8v = NC_FILL_BYTE; break;
      case NC_SHORT: value->value.int16v = NC_FILL_SHORT; break;
      case NC_INT: value->value.int32v = NC_FILL_INT; break;
      case NC_FLOAT: value->value.floatv = NC_FILL_FLOAT; break;
      case NC_DOUBLE: value->value.doublev = NC_FILL_DOUBLE; break;
      case NC_UBYTE: value->value.uint8v = NC_FILL_UBYTE; break;
      case NC_USHORT: value->value.uint16v = NC_FILL_USHORT; break;
      case NC_UINT: value->value.uint32v = NC_FILL_UINT; break;
      case NC_INT64: value->value.int64v = NC_FILL_INT64; break;
      case NC_UINT64: value->value.uint64v = NC_FILL_UINT64; break;
      case NC_STRING:
        value->value.stringv.stringv = nulldup(NC_FILL_STRING);
        value->value.stringv.len = strlen(NC_FILL_STRING);
	/* Exception: if string is null, then make it's length be 1 */
	if(value->value.stringv.len == 0)
	    value->value.stringv.len = 1;
	break;
      case NC_OPAQUE:
        value->value.opaquev.len = 2;
        value->value.opaquev.stringv = nulldup("00");
	break;
      default:
	derror("nc_getfill: unrecognized type: %d",value->nctype);
    }
}

char* 
nc_dfaltfillname(nc_type nctype)
{
    switch (nctype) {
    case NC_BYTE: return "NC_FILL_BYTE";
    case NC_CHAR: return "NC_FILL_CHAR";
    case NC_SHORT: return "NC_FILL_SHORT";
    case NC_INT: return "NC_FILL_INT";
    case NC_FLOAT: return "NC_FILL_FLOAT";
    case NC_DOUBLE: return "NC_FILL_DOUBLE";
    case NC_UBYTE: return "NC_FILL_UBYTE";
    case NC_USHORT: return "NC_FILL_USHORT";
    case NC_UINT: return "NC_FILL_UINT";
    case NC_INT64: return "NC_FILL_INT64";
    case NC_UINT64: return "NC_FILL_UINT64";
    case NC_STRING: return "NC_FILL_STRING";
    default: PANIC("unexpected default fill name");
    }
    return NULL;
}

