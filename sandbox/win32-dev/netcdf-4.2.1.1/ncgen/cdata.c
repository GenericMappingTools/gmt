/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/cdata.c,v 1.5 2010/05/24 19:59:56 dmh Exp $ */

#include "includes.h"

#ifdef ENABLE_C

#include	<math.h>


/**************************************************/
/* Code for generating C language data lists*/
/**************************************************/
/* For datalist constant rules: see the rules on the man page */

/* Forward*/
static void cdata_primdata(Symbol*, Datasrc*, Bytebuffer*, Datalist*);
static void cdata_fieldarray(Symbol*, Datasrc*, Odometer*, int, Bytebuffer*, Datalist* fillsrc);

/* Specialty wrappers for cdata_data */
void
cdata_attrdata(Symbol* asym, Bytebuffer* codebuf)
{
    Datasrc* src;
    int typecode = asym->typ.basetype->typ.typecode;

    if(asym->data == NULL) return;
    if(typecode == NC_CHAR) {
	gen_charattr(asym,codebuf);
    } else {
        src = datalist2src(asym->data);
        while(srcmore(src)) {
	    bbAppend(codebuf,' ');
            cdata_basetype(asym->typ.basetype,src,codebuf,NULL);
	}
    }
}

void
cdata_array(Symbol* vsym,
		  Bytebuffer* codebuf,
		  Datasrc* src,
		  Odometer* odom,
	          int index,
		  Datalist* fillsrc)
{
    int i;
    int rank = odom->rank;
    int pushed = 0;
    size_t count;
    Symbol* basetype = vsym->typ.basetype;
    int lastdim = (index == (rank - 1)); /* last dimension*/
    int firstdim = (index == 0); /* first dimension*/
    int isunlimited = (odom->declsize[index] == 0);

    ASSERT(index >= 0 && index < rank);

    count = odom->count[index];

    if(!firstdim && isunlimited && issublist(src)) {
	srcpush(src);
	pushed = 1;
    }
   
    if(lastdim) {
        for(i=0;i<count;i++) {
            cdata_basetype(basetype,src,codebuf,fillsrc);
	}
    } else {
        /* now walk count elements and generate recursively */
        for(i=0;i<count;i++) {
	   cdata_array(vsym,codebuf,src,odom,index+1,fillsrc);
	}
    }

    if(isunlimited && pushed) srcpop(src);

    return;
}

/* Generate an instance of the basetype using datasrc */
void
cdata_basetype(Symbol* tsym, Datasrc* datasrc, Bytebuffer* codebuf, Datalist* fillsrc)
{
    int usecmpd;

    switch (tsym->subclass) {

    case NC_ENUM:
    case NC_OPAQUE:
    case NC_PRIM:
	if(issublist(datasrc)) {
	    semerror(srcline(datasrc),"Expected primitive found {..}");
	}
	bbAppend(codebuf,' ');
	cdata_primdata(tsym,datasrc,codebuf,fillsrc);
	break;

    case NC_COMPOUND: {
	int i;
        Constant* con;
	if(!isfillvalue(datasrc) && !issublist(datasrc)) {/* fail on no compound*/
	    semerror(srcline(datasrc),"Compound data must be enclosed in {..}");
        }
        con = srcnext(datasrc);
	if(con == NULL || con->nctype == NC_FILLVALUE) {
	    Datalist* filler = getfiller(tsym,fillsrc);
	    ASSERT(filler->length == 1);
	    con = &filler->data[0];
	    if(con->nctype != NC_COMPOUND) {
	        semerror(con->lineno,"Compound data fill value is not enclosed in {..}");
	    }
	}
        srcpushlist(datasrc,con->value.compoundv); /* enter the sublist*/
        bbAppend(codebuf,'{');
        for(i=0;i<listlength(tsym->subnodes);i++) {
            Symbol* field = (Symbol*)listget(tsym->subnodes,i);
            bbAppend(codebuf,' ');
            cdata_basetype(field,datasrc,codebuf,NULL);
	}
        bbAppend(codebuf,'}');
        srcpop(datasrc);
	} break;

    case NC_VLEN: {
        Constant* con;
	if(!isfillvalue(datasrc) && !issublist(datasrc)) {/* fail on no compound*/
	    semerror(srcline(datasrc),"Vlen data must be enclosed in {..}");
        }
        con = srcnext(datasrc);
	if(con == NULL || con->nctype == NC_FILLVALUE) {
	    Datalist* filler = getfiller(tsym,fillsrc);
	    ASSERT(filler->length == 1);
	    con = &filler->data[0];
	    if(con->nctype != NC_COMPOUND) {
	        semerror(con->lineno,"Vlen data fill value is not enclosed in {..}");
	    }
	}
        /* generate the nc_vlen_t instance*/
        bbprintf0(stmt,"{%u (void*)vlen_%u}",
	         con->value.compoundv->vlen.count,
    		 con->value.compoundv->vlen.uid);
        bbCatbuf(codebuf,stmt);
        } break;

    case NC_FIELD:
	/* enclose in braces if and only if field is an array */
	usecmpd = (issublist(datasrc) && tsym->typ.dimset.ndims > 0);
	if(usecmpd) srcpush(datasrc);
	if(tsym->typ.dimset.ndims > 0) {
	    Odometer* fullodom = newodometer(&tsym->typ.dimset,NULL,NULL);
            cdata_fieldarray(tsym->typ.basetype,datasrc,fullodom,0,codebuf,fillsrc);
	    odometerfree(fullodom);
	} else {
	    cdata_basetype(tsym->typ.basetype,datasrc,codebuf,NULL);
	}
	if(usecmpd) srcpop(datasrc);
	break;

    default: PANIC1("cdata_basetype: unexpected subclass %d",tsym->subclass);
    }
}

/* Used only for structure field arrays*/
static void
cdata_fieldarray(Symbol* basetype, Datasrc* src, Odometer* odom, int index,
		 Bytebuffer* codebuf, Datalist* fillsrc)
{
    int i;
    int rank = odom->rank;
    unsigned int size = odom->declsize[index];
    int lastdim = (index == (rank - 1)); /* last dimension*/
    int chartype = (basetype->typ.typecode == NC_CHAR);

    if(chartype) {
	/* Collect the char field in a separate buffer */
	Bytebuffer* fieldbuf = bbNew();
        gen_charfield(src,odom,fieldbuf);	
	/* Add to the existing data buf as a single constant */
	cquotestring(fieldbuf);
	bbCat(codebuf," ");
        bbCatbuf(codebuf,fieldbuf);
	bbFree(fieldbuf);
    } else {
        ASSERT(size != 0);
        for(i=0;i<size;i++) {
            if(lastdim) {
	        bbAppend(codebuf,' ');
	        cdata_basetype(basetype,src,codebuf,NULL);
            } else { /* !lastdim*/
	        cdata_fieldarray(basetype,src,odom,index+1,codebuf,fillsrc);
	    }
	}
    }
}

static void
cdata_primdata(Symbol* basetype, Datasrc* src, Bytebuffer* codebuf, Datalist* fillsrc)
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
        cdata_primdata(basetype,src,codebuf,NULL);
	srcpop(src);
	goto done;
    }

    target.nctype = basetype->typ.typecode;

    if(target.nctype != NC_ECONST) {
	convert1(prim,&target);
    }

    switch (target.nctype) {
    case NC_ECONST:
        if(basetype->subclass != NC_ENUM) {
	    semerror(prim->lineno,"Conversion to enum not supported (yet)");
	} else {
	    Datalist* econ = builddatalist(1);
	    Symbol* enumv = prim->value.enumv;
	    srcpushlist(src,econ);
	    dlappend(econ,&enumv->typ.econst);
	    cdata_primdata(enumv->typ.basetype,src,codebuf,fillsrc);
	    srcpop(src);
	 } break;
     case NC_OPAQUE: {
	    setprimlength(&target,basetype->typ.size*2);
	} break;
    default: break;
    }
    bbCat(codebuf,cdata_const(&target));
    
done:
    return;
}

/*
This walk of the data lists collects
vlen sublists and constructs separate C constants
for each of them. The "id" of each list is then
recorded in the containing datalist.
*/
void
cdata_vlenconstants(List* vlenconstants, Bytebuffer* codebuf)
{
    int i,nvlen;
    Datasrc* vlensrc;
    Bytebuffer* tmp = bbNew();

    nvlen = listlength(vlenconstants);
    for(i=0;i<nvlen;i++) {
	Constant* cmpd = (Constant*)listget(vlenconstants,i);
	int chartype;
	Symbol* tsym = cmpd->value.compoundv->vlen.schema;
        ASSERT(tsym != NULL);
        chartype = (tsym->typ.basetype->typ.typecode == NC_CHAR);

        bbprintf0(tmp,"static const %s vlen_%u[] = ",
	        ctypename(tsym->typ.basetype),
                cmpd->value.compoundv->vlen.uid);
	bbCatbuf(codebuf,tmp);
	vlensrc = datalist2src(cmpd->value.compoundv);
	bbAppend(codebuf,'{');
	if(chartype) {
   	    /* Collect the char vlen in a separate buffer */
	    Bytebuffer* vlenbuf = bbNew();
            gen_charvlen(vlensrc,vlenbuf);
	    /* Add to the existing data buf as a single constant */
   	    cquotestring(vlenbuf);
            bbCatbuf(codebuf,vlenbuf);
   	    bbFree(vlenbuf);
	} else {
	    size_t count = 0;
  	    while(srcmore(vlensrc)) {
	        if(count > 0) bbCat(codebuf,", ");
                cdata_basetype(tsym->typ.basetype,vlensrc,codebuf,NULL);
		count++;
	    }
	    ASSERT(count == cmpd->value.compoundv->vlen.count);
        }
	bbCat(codebuf,"} ;\n");
    }
    bbFree(tmp);
}

/* Result is a pool string or a constant => do not free*/
char*
cdata_const(Constant* ci)
{
    Bytebuffer* codetmp = bbNew();
    char* result;

    switch (ci->nctype) {
    case NC_CHAR:
	{ 
	    char tmp[64];
	    tmp[0] = '\0';
	    escapifychar(ci->value.charv,tmp,'\'');
	    bbCat(codetmp,"'");
	    bbCat(codetmp,tmp);
	    bbCat(codetmp,"'");
	}
	break;
    case NC_BYTE:
	bbprintf(codetmp,"%hhd",ci->value.int8v);
	break;
    case NC_SHORT:
	bbprintf(codetmp,"%hd",ci->value.int16v);
	break;
    case NC_INT:
	bbprintf(codetmp,"%d",ci->value.int32v);
	break;
    case NC_FLOAT:
	/* Special case for nanf */
	if(isnan(ci->value.floatv))
	    bbprintf(codetmp,"nanf");
	else
	    bbprintf(codetmp,"%f",ci->value.floatv);
	break;
    case NC_DOUBLE:
	/* Special case for nanf */
	if(isnan(ci->value.doublev))
	    bbprintf(codetmp,"nan");
	else
	    bbprintf(codetmp,"%lf",ci->value.doublev);
	break;
    case NC_UBYTE:
	    bbprintf(codetmp,"%hhu",ci->value.uint8v);
	break;
    case NC_USHORT:
	bbprintf(codetmp,"%hu",ci->value.uint16v);
	break;
    case NC_UINT:
	bbprintf(codetmp,"%uU",ci->value.uint32v);
	break;
    case NC_INT64:
	bbprintf(codetmp,"%lldLL",ci->value.int64v);
	break;
    case NC_UINT64:
	bbprintf(codetmp,"%lluLLU",ci->value.uint64v);
	break;
    case NC_ECONST:
	bbprintf(codetmp,"%s",cname(ci->value.enumv));
	break;
    case NC_STRING:
	{ /* handle separately */
	    char* escaped = escapify(ci->value.stringv.stringv,
				 '"',ci->value.stringv.len);
	    result = poolalloc(1+2+strlen(escaped));
	    strcpy(result,"\"");
	    strcat(result,escaped);
	    strcat(result,"\"");
	    goto done;
	}
	break;
    case NC_OPAQUE: {
	char* p;
	int bslen;
	bslen=(4*ci->value.opaquev.len);
	result = poolalloc(bslen+2+1);
	strcpy(result,"\"");
	p = ci->value.opaquev.stringv;
	while(*p) {
	    strcat(result,"\\x");
	    strncat(result,p,2);	    	    
	    p += 2;	
	}
	strcat(result,"\"");
	goto done;
	} break;

    default: PANIC1("ncstype: bad type code: %d",ci->nctype);
    }
    result = pooldup(bbContents(codetmp)); /*except for NC_STRING and NC_OPAQUE*/
    bbFree(codetmp);
done:
    return result;
}

#endif /*ENABLE_C*/
