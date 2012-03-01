/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/f77data.c,v 1.4 2010/05/24 19:59:57 dmh Exp $ */

#include "includes.h"
#include "offsets.h"

#ifdef ENABLE_F77

/* Forward*/
static void f77data_primdata(Symbol*, Datasrc*, Bytebuffer*, Datalist*);


/**************************************************/
/* Code for generating FORTRAN 77 language data lists*/
/**************************************************/
/* Datalist rules: see the rules on the man page */

/* Specialty wrappers for f77data_data */
void
f77data_attrdata(Symbol* asym, Bytebuffer* databuf)
{
    Datasrc* src;
    int typecode = asym->typ.basetype->typ.typecode;

    if(asym->data == NULL) return;
    if(typecode == NC_CHAR) {
        gen_charattr(asym,databuf);
    } else {
        src = datalist2src(asym->data);
        while(srcmore(src)) {
	    bbAppend(databuf,' ');
            f77data_basetype(asym->typ.basetype,src,databuf,NULL);
	}
    }
}


void
f77data_array(Symbol* vsym,
		  Bytebuffer* databuf,
		  Datasrc* src,
		  Odometer* odom,
	          int index,
		  Datalist* fillsrc)
{
    int i;
    int rank = odom->rank;
    int firstdim = (index == 0); /* last dimension*/
    int lastdim = (index == (rank - 1)); /* last dimension*/
    size_t count;
    Symbol* basetype = vsym->typ.basetype;
    int isunlimited = (odom->declsize[index] == 0);
    int pushed=0;

    ASSERT(index >= 0 && index < rank);

    count = odom->count[index];

    if(!firstdim && isunlimited && issublist(src)) {
	srcpush(src);
	pushed = 1;
    }
   
    if(lastdim) {
        for(i=0;i<count;i++) {
            f77data_basetype(basetype,src,databuf,fillsrc);
	}
    } else {
        /* now walk count elements and generate recursively */
        for(i=0;i<count;i++) {
	    f77data_array(vsym,databuf,src,odom,index+1,fillsrc);
	}
    }

    if(isunlimited && pushed) srcpop(src);

    return;
}

/* Generate an instance of the basetype using datasrc */
void
f77data_basetype(Symbol* tsym, Datasrc* datasrc, Bytebuffer* codebuf, Datalist* fillsrc)
{
    switch (tsym->subclass) {

    case NC_PRIM:
	if(issublist(datasrc)) {
	    semerror(srcline(datasrc),"Expected primitive found {..}");
	}
	bbAppend(codebuf,' ');
	f77data_primdata(tsym,datasrc,codebuf,fillsrc);
	break;

    default: PANIC1("f77data_basetype: unexpected subclass %d",tsym->subclass);
    }
}


static void
f77data_primdata(Symbol* basetype, Datasrc* src, Bytebuffer* codebuf, Datalist* fillsrc)
{
    Constant* prim;
    Constant target;

    prim = srcnext(src);
    if(prim == NULL) prim = &fillconstant;

    ASSERT(prim->nctype != NC_COMPOUND);

    if(prim->nctype == NC_FILLVALUE) {
	Datalist* filler = getfiller(basetype,fillsrc);
	ASSERT(filler->length == 1);
	srcpushlist(src,filler);
	bbAppend(codebuf,' ');
        f77data_primdata(basetype,src,codebuf,NULL);
	srcpop(src);
	goto done;
    }

    target.nctype = basetype->typ.typecode;

    convert1(prim,&target);
    bbCat(codebuf,f77data_const(&target));
    
done:
    return;
}

/* Result is a pool string or a constant => do not free*/
char*
f77data_const(Constant* ci)
{
    char tmp[64];
    char* result = NULL;

    tmp[0] = '\0';
    switch (ci->nctype) {
    case NC_CHAR:
	{
	    strcpy(tmp,"'");
	    escapifychar(ci->value.charv,tmp+1,'\'');
	    strcat(tmp,"'");
	}
	break;
    case NC_BYTE:
	sprintf(tmp,"%hhd",ci->value.int8v);
	break;
    case NC_SHORT:
	sprintf(tmp,"%hd",ci->value.int16v);
	break;
    case NC_INT:
	sprintf(tmp,"%d",ci->value.int32v);
	break;
    case NC_FLOAT:
	sprintf(tmp,"%.8g",ci->value.floatv);
	break;
    case NC_DOUBLE: { 
	char* p = tmp;
	/* FORTRAN requires e|E->D */
	sprintf(tmp,"%.16g",ci->value.doublev);
	while(*p) {if(*p == 'e' || *p == 'E') {*p = 'D';}; p++;}
	} break;
    case NC_STRING:
	{
	    Bytebuffer* buf = bbNew();
	    bbAppendn(buf,ci->value.stringv.stringv,ci->value.stringv.len);
	    f77quotestring(buf);
	    result = bbDup(buf);
	    bbFree(buf);
	    goto done;
	}
	break;

    default: PANIC1("ncstype: bad type code: %d",ci->nctype);
    }
    result = pooldup(tmp);
done:
    return result; /*except for NC_STRING and NC_OPAQUE*/
}

#endif /*ENABLE_F77*/
