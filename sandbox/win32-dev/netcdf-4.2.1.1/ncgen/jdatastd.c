/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/jdatastd.c,v 1.2 2010/05/24 19:59:57 dmh Exp $ */

#include "includes.h"
#include "offsets.h"

extern List* vlenconstants;  /* List<Constant*>;*/

/* Forward*/
static void genjstd_primdata(Symbol*, Datasrc*, Datalist*, char*, Bytebuffer*);
static void genjstd_data(Symbol* sym, Datasrc*, Datalist*, char*, Bytebuffer*);
static void genjstd_arraydatar(Symbol* basetype,
	      Datasrc* src,
	      Odometer* odom,
	      Putvar* closure,
	      int index,
	      int checkpoint,
	      Bytebuffer* code);

/**************************************************/
/* Code for generating Java language data lists*/
/**************************************************/
/* Datalist rules: see the rules on the man page */

/* Specialty wrappers for genjstd_data */
void
genjstd_attrdata(Symbol* asym, Bytebuffer* databuf)
{
    Datasrc* src;
    int typecode = asym->typ.basetype->typ.typecode;
    char* memname = "memory";

    if(asym->data == NULL) return;
    if(typecode == NC_CHAR) {
        gen_charattr(asym,databuf);
    } else {
        src = datalist2src(asym->data);
        while(srcmore(src)) {
            genjstd_data(asym->typ.basetype,src,NULL,memname,databuf);
	}
    }
}

void
genjstd_scalardata(Symbol* vsym, Bytebuffer* databuf)
{
    Datasrc* src;
    char* memname = "memory";

    if(vsym->data == NULL) return;
    src = datalist2src(vsym->data);
    genjstd_data(vsym->typ.basetype,src,
		   vsym->var.special._Fillvalue,memname,databuf);
    if(srcmore(src)) {
        semerror(srcline(src),"Extra data at end of datalist");
    }
}

void
genjstd_fillvalue(Symbol* tsym, Datalist* fillsrc, Datasrc* src,
               char* memname, Bytebuffer* databuf)
{
    Datalist* list = NULL;

    ASSERT(tsym->objectclass == NC_TYPE);
    list = fillsrc;
    if(list == NULL) list = getfiller(tsym);
    srcpushlist(src,list);
    genjstd_data(tsym,src,NULL,memname,databuf);
    srcpop(src);
}

void
genjstd_arraydata(Symbol* vsym, Putvar* closure, Bytebuffer* databuf)
{
    Datasrc* src;
    Datalist* list;
    int i,checkpoint;
    nc_type typecode = vsym->typ.basetype->typ.typecode;

    if(vsym->data == NULL) return;

    list = vsym->data;
    ASSERT(list->dimdata != NULL);

    src = datalist2src(list);

    /* Locate checkpoint */
    checkpoint=0;
    for(i=0;i<vsym->typ.dimset.ndims;i++) {
	if(vsym->typ.dimset.dimsyms[i]->dim.size == NC_UNLIMITED) {
	    checkpoint = i;
	    break;
	}
    }

    if(typecode == NC_CHAR) {
	Odometer* odom = list->dimdata;
        gen_chararray(vsym,closure,databuf);
	/* Compute the unlimited size */
	vsym->typ.dimset.dimsyms[0]->dim.unlimitedsize
		= odom->dims[0].datasize;
	jquotestring(databuf,'"');
    } else {
        genjstd_arraydatar(vsym,src,list->dimdata,closure,0,checkpoint,databuf);
	vsym->typ.dimset.dimsyms[0]->dim.unlimitedsize
		= list->dimdata->dims[0].datasize;
    }
}

static void
genjstd_arraydatar(Symbol* vsym,
	      Datasrc* src,
	      Odometer* odom,
	      Putvar* closure,
	      int index,
	      int checkpoint,
	      Bytebuffer* databuf)
{
    int i;
    int rank = odom->rank;
    int lastdim = (index == (rank - 1)); /* last dimension*/
    int firstdim = (index == 0);
    int declsize = odom->dims[index].declsize;
    Symbol* basetype = vsym->typ.basetype;
    Datalist* fillsrc = vsym->var.special._Fillvalue;
    Constant* con;
    char* memname = "memory";
    int isunlimited = (declsize == 0);

    ASSERT(index >= 0 && index < rank);

    odom->dims[index].index = 0; /* reset*/

    if(isunlimited) {
	Constant* con;
        if(!firstdim) {
	    if(!issublist(src)) {
	        semerror(srcline(src),"Unlimited data must be enclosed in {..}");
		goto done;
	    }
	    srcpush(src); /* enter the unlimited data */
	}
	while((con=srcpeek(src))!=NULL) {
	    /* lastdim is unlimited => lastdim == firstdim */
	    if(lastdim) {
		bbCat(databuf," ");
                genjstd_primdata(basetype,src,fillsrc,memname,databuf);
	    } else {
                genjstd_arraydatar(vsym,src,odom,closure,index+1,
                                               checkpoint,databuf);
	    }
	    odom->dims[index].index++;
	}
        odom->dims[index].datasize = odom->dims[index].index;
	if(!firstdim) srcpop(src);
    } else { /* !isunlimited*/
	for(i=0;i<declsize;i++) {
	    con = srcpeek(src);
            if(lastdim) {
		bbCat(databuf," ");
                genjstd_primdata(basetype,src,fillsrc,memname,databuf);
            } else { /* ! lastdim*/
               (void)genjstd_arraydatar(vsym,src,odom,closure,
                                        index+1,checkpoint,
					databuf);
            }
            odom->dims[index].index++;
	}
    }
done:
    return;
}

/**************************************************/
/* This is the general datalist processing procedure */
static void
genjstd_data(Symbol* tsym, Datasrc* datasrc, Datalist* fillsrc,
		char* memname, Bytebuffer* databuf)
{
    int iscmpd;
    Constant* con = srcpeek(datasrc);

    if(con == NULL || con->nctype == NC_FILLVALUE) {
	srcnext(datasrc);
	genjstd_fillvalue(tsym,fillsrc,datasrc,memname,databuf);
	return;
    }

    switch (tsym->subclass) {

    case NC_PRIM:
	if((iscmpd = issublist(datasrc))) {
	    semerror(srcline(datasrc),"Expected primitive found {..}");
	    srcpush(datasrc);
	}
	bbCat(databuf," ");
	genjstd_primdata(tsym,datasrc,NULL,memname,databuf);
	if(iscmpd) srcpop(datasrc);
	break;

    default: PANIC1("genjstd_data: unexpected subclass %d",tsym->subclass);
    }
}


/**************************************************/
static void
genjstd_primdata(Symbol* tsym, Datasrc* src, Datalist* filler,
		char* memname, Bytebuffer* databuf)
{
    Constant target, *prim;

    prim = srcnext(src);

    if(prim == NULL || prim->nctype == NC_FILLVALUE) {
	genjstd_fillvalue(tsym,filler,src,memname,databuf);
	return;
    }

    target.nctype = tsym->typ.typecode;

    if(prim == NULL) {
#ifdef GENFILL
        /* generate a fill value*/
	nc_getfill(&target);
	/* fall thru*/
#else
	return;
#endif
    }

    ASSERT(prim->nctype != NC_COMPOUND);

    switch (target.nctype) {
        case NC_CHAR:
	    /* Hack to handle the char case as a string */
	    target = cloneconstant(prim);
	    break;
        default:
	    convert1(prim,&target);
	    break;
    }
    /* add hack for java bug in converting floats */
    if(target.nctype == NC_FLOAT) bbCat(databuf," (float)");
    bbCat(databuf,jconst(&target));
}
