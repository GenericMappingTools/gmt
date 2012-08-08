/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/cdfdata.c,v 1.4 2010/05/24 19:59:56 dmh Exp $ */

#include "includes.h"
#include "offsets.h"

#ifdef ENABLE_BINARY

/**************************************************/
/* Code for generating binary data lists*/
/**************************************************/

struct Vlendata {
    char* data;
    unsigned long count;
}* vlendata;

/* Forward*/
static void genbin_data(Symbol* sym, Datasrc*, Datalist*, Bytebuffer*);
static void genbin_arraydatar(Symbol* basetype,
	      Datasrc* src,
	      Odometer* odom,
	      int index,
	      int checkpoint,
	      Bytebuffer* code);
static void genbin_primdata(Symbol*, Datasrc*, Datalist*, Bytebuffer*);
static void genbin_fieldarray(Symbol*, Datasrc*, Dimset*, int, Bytebuffer*);
static void alignbuffer(Constant* prim, Bytebuffer* buf);

/* Datalist rules: see the rules on the man page */

/* Specialty wrappers for genbin_data */
void
genbin_attrdata(Symbol* asym, Bytebuffer* memory)
{
    Datasrc* src;
    int chartype = (asym->typ.basetype->typ.typecode == NC_CHAR);

    if(asym->data == NULL) return;
    if(chartype) {gen_charattr(asym,memory); return;}
    src = datalist2src(asym->data);
    while(srcmore(src)) {
        genbin_data(asym->typ.basetype,src,NULL,memory);
    }
}

void
genbin_scalardata(Symbol* vsym, Bytebuffer* memory)
{
    Datasrc* src;

    if(vsym->data == NULL) return;
    src = datalist2src(vsym->data);
    genbin_data(vsym->typ.basetype,src,
		   vsym->var.special._Fillvalue,memory);
    if(srcmore(src)) {
        semerror(srcline(src),"Extra data at end of datalist");
    }
}

void
genbin_datalist(struct Symbol* sym, Datalist* list, Bytebuffer* memory)
{
    Datasrc* src = datalist2src(list);
    genbin_data(sym,src,NULL,memory);
}

static void
genbin_data(Symbol* tsym, Datasrc* datasrc, Datalist* fillsrc,
		Bytebuffer* memory)
{
    int usecmpd;
    Constant* con = srcpeek(datasrc);
    if(con == NULL || con->nctype == NC_FILLVALUE) {
	srcnext(datasrc);
	genbin_fillvalue(tsym,fillsrc,datasrc,memory);
	return;
    }

    switch (tsym->subclass) {

    case NC_ENUM:
    case NC_OPAQUE:
    case NC_PRIM:
	if(issublist(datasrc)) {
	    semerror(srcline(datasrc),"Expected primitive found {..}");
	}
	genbin_primdata(tsym,datasrc,fillsrc,memory);
	break;

    case NC_COMPOUND: {
	int i;
        if(!issublist(datasrc)) {
	    semerror(srcline(datasrc),"Compound data must be enclosed in {..}");
        }
	srcpush(datasrc);
        for(i=0;i<listlength(tsym->subnodes);i++) {
            Symbol* field = (Symbol*)listget(tsym->subnodes,i);
	    if(!srcmore(datasrc)) { /* generate a fill value*/
	        Datalist* fillsrc = getfiller(tsym);
	        genbin_data(field,datasrc,fillsrc,memory);
	    } else
	        genbin_data(field,datasrc,NULL,memory);
	}
        srcpop(datasrc);
	} break;

    case NC_VLEN: {
        Constant* cp;
        nc_vlen_t ptr;
        if(!issublist(datasrc)) {
	    semerror(srcline(datasrc),"Vlen data must be enclosed in {..}");
        }
        cp = srcnext(datasrc);
        /* generate the nc_vlen_t instance*/
        ptr.p = vlendata[cp->value.compoundv->vlen.uid].data;
        ptr.len = vlendata[cp->value.compoundv->vlen.uid].count;
        bbAppendn(memory,(char*)&ptr,sizeof(ptr));
        } break;

    case NC_FIELD:
	/* enclose in braces if and only if field is an array */
	usecmpd = (issublist(datasrc) && tsym->typ.dimset.ndims > 0);
	if(usecmpd) srcpush(datasrc);
	if(tsym->typ.dimset.ndims > 0) {
            genbin_fieldarray(tsym->typ.basetype,datasrc,&tsym->typ.dimset,0,memory);
	} else {
	    genbin_data(tsym->typ.basetype,datasrc,NULL,memory);
	}
	if(usecmpd) srcpop(datasrc);
	break;

    default: PANIC1("genbin_data: unexpected subclass %d",tsym->subclass);
    }
}

/* Used only for structure field arrays*/
static void
genbin_fieldarray(Symbol* basetype, Datasrc* src, Dimset* dimset, int index,
		Bytebuffer* memory)
{
    int i;
    Symbol* dim = dimset->dimsyms[index];
    unsigned int size = dim->dim.size;
    int lastdim = (index == (dimset->ndims - 1)); /* last dimension*/
    int chartype = (basetype->typ.typecode == NC_CHAR);

    if(chartype) {
	/* Collect the char field in a separate buffer */
	Bytebuffer* fieldbuf = bbNew();
        gen_charfield(src,dimset,index,fieldbuf);
        bbAppendn(memory,bbContents(fieldbuf),bbLength(fieldbuf));
	bbFree(fieldbuf);
    } else {
        ASSERT(size != 0);
        for(i=0;i<size;i++) {
            if(lastdim) {
	        genbin_data(basetype,src,NULL,memory);
            } else { /* !lastdim*/
	        genbin_fieldarray(basetype,src,dimset,index+1,memory);
	    }
	}
    }
}

static void
genbin_primdata(Symbol* basetype, Datasrc* src, Datalist* fillsrc,
		Bytebuffer* memory)
{
    Constant* prim;
    Constant target;

    prim = srcnext(src);

    if(prim == NULL || prim->nctype == NC_FILLVALUE) {
	genbin_fillvalue(basetype,fillsrc,src,memory);	
	return;
    }

    target.nctype = basetype->typ.typecode;

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

    if(target.nctype != NC_ECONST) {
	convert1(prim,&target);
        alignbuffer(&target,memory);
    }

    switch (target.nctype) {
        case NC_ECONST:
	    if(basetype->subclass != NC_ENUM) {
	        semerror(prim->lineno,"Conversion to enum not supported (yet)");
	    } else {
		Datalist* econ = builddatalist(1);
		srcpushlist(src,econ);
		dlappend(econ,&prim->value.enumv->typ.econst);
	        genbin_primdata(prim->value.enumv->typ.basetype,src,
				fillsrc,memory);
		srcpop(src);
	    }
   	    break;
        case NC_OPAQUE: {
	    unsigned char* bytes;
	    size_t len;
	    setprimlength(&target,basetype->typ.size*2);
	    bytes=makebytestring(target.value.opaquev.stringv,&len);
	    bbAppendn(memory,(void*)bytes,len);
	    } break;

        case NC_CHAR:
            bbAppendn(memory,&target.value.charv,sizeof(target.value.charv));
	    break;
        case NC_BYTE:
            bbAppendn(memory,(void*)&target.value.int8v,sizeof(target.value.int8v));
	    break;
        case NC_SHORT:
            bbAppendn(memory,(void*)&target.value.int16v,sizeof(target.value.int16v));
	    break;
        case NC_INT:
            bbAppendn(memory,(void*)&target.value.int32v,sizeof(target.value.int32v));
	    break;
        case NC_FLOAT:
            bbAppendn(memory,(void*)&target.value.floatv,sizeof(target.value.floatv));
	    break;
        case NC_DOUBLE:
            bbAppendn(memory,(void*)&target.value.doublev,sizeof(target.value.doublev));
	    break;
        case NC_UBYTE:
            bbAppendn(memory,(void*)&target.value.uint8v,sizeof(target.value.uint8v));
	    break;
        case NC_USHORT:
            bbAppendn(memory,(void*)&target.value.uint16v,sizeof(target.value.uint16v));
	    break;
        case NC_UINT:
            bbAppendn(memory,(void*)&target.value.uint32v,sizeof(target.value.uint32v));
	    break;
        case NC_INT64: {
	    union SI64 { char ch[8]; long long i64;} si64;
	    si64.i64 = target.value.int64v;
            bbAppendn(memory,(void*)si64.ch,sizeof(si64.ch));
	    } break;
        case NC_UINT64: {
	    union SU64 { char ch[8]; unsigned long long i64;} su64;
	    su64.i64 = target.value.uint64v;
            bbAppendn(memory,(void*)su64.ch,sizeof(su64.ch));
	    } break;
        case NC_STRING: {
            if(usingclassic) {
                bbAppendn(memory,target.value.stringv.stringv,target.value.stringv.len);
            } else if(target.nctype == NC_CHAR) {
                bbAppendn(memory,target.value.stringv.stringv,target.value.stringv.len);
            } else {
                char* ptr;
                int len = (size_t)target.value.stringv.len;
                ptr = poolalloc(len+1); /* CAREFUL: this has short lifetime*/
                memcpy(ptr,target.value.stringv.stringv,len);
                ptr[len] = '\0';
                bbAppendn(memory,(void*)&ptr,sizeof(ptr));
            }
        } break;

        default: PANIC1("genbin_primdata: unexpected type: %d",target.nctype);
    }
}

void
genbin_fillvalue(Symbol* tsym, Datalist* fillsrc, Datasrc* src,
               Bytebuffer* memory)
{
    Datalist* list = NULL;

    ASSERT(tsym->objectclass == NC_TYPE);
    list = fillsrc;
    if(list == NULL) list = getfiller(tsym);
    srcpushlist(src,list);
    genbin_data(tsym,src,NULL,memory);
    srcpop(src);
}

/*
This walk of the data lists collects
vlen sublists and constructs separate C constants
for each of them. The "id" of each list is then
recorded in the containing datalist.
*/
void
genbin_vlenconstants(List* vlenconstants)
{
    int i,nvlen;
    Datasrc* vlensrc;
    Bytebuffer* memory = bbNew();

    /* Prepare a place to store vlen constants */
    nvlen = listlength(vlenconstants);
    if(nvlen == 0) return;
    vlendata = (struct Vlendata*)emalloc(sizeof(struct Vlendata)*nvlen+1);
    memset((void*)vlendata,0,sizeof(struct Vlendata)*nvlen+1);

    for(i=0;i<nvlen;i++) {
	Constant* cmpd = (Constant*)listget(vlenconstants,i);
	int chartype;
	Symbol* tsym = cmpd->value.compoundv->vlen.schema;
	unsigned long uid = cmpd->value.compoundv->vlen.uid;
	unsigned long count;
        ASSERT(tsym != NULL);
        chartype = (tsym->typ.basetype->typ.typecode == NC_CHAR);

	vlensrc = datalist2src(cmpd->value.compoundv);

	bbClear(memory);
	count = 0;
	if(chartype) {
   	    /* Collect the char vlen in a separate buffer */
            gen_charvlen(vlensrc,memory);
	    count = bbLength(memory);
	} else {
  	    while(srcmore(vlensrc)) {
                genbin_data(tsym->typ.basetype,vlensrc,NULL,memory);
		count++;
	    }
	    ASSERT(count == cmpd->value.compoundv->vlen.count);
        }
	vlendata[uid].data = bbDup(memory);
	vlendata[uid].count = count;
    }
    bbFree(memory);
}

void
genbin_arraydata(Symbol* vsym, Bytebuffer* memory)
{
    Datasrc* src;
    Datalist* list;
    int i;
    int chartype = (vsym->typ.basetype->typ.typecode == NC_CHAR);
    nciter_t iter;
    Iterodom iterodom;

    if(vsym->data == NULL) return;

    if(chartype) {
        gen_chararray(vsym,memory);
	return;
    }

    list = vsym->data;
    ASSERT(list->dimdata != NULL);
    src = datalist2src(list);

    /* Create an iterator to generate blocks of data */
    nc_get_iter(vsym,databuffersize,&iter);
    nc_next_iter(&iter,iterodom.start,iterodom.count);
    iterodom.rank = vsym->typ.dimset.ndims;
    genbin_arraydatar(vsym,memory,src,iter,&iterodom,/*index=*/0)
}

static void
genbin_arraydatar(Symbol* vsym,
		  Bytebuffer* memory,
		  nciter_t iter,
		  Iterodom* iterodom,
	          int index)
{
    int i;
    int rank = iterodom->rank;
    int lastdim = (index == (rank - 1)); /* last dimension*/
    int firstdim = (index == 0);
    int declsize = odom->dims[index].declsize;
    int isunlimited = (declsize == 0);
    Symbol* basetype = vsym->typ.basetype;
    Datalist* fillsrc = vsym->var.special._Fillvalue;
    Constant* con;

    ASSERT(index >= 0 && index < rank);
    odom->dims[index].index = 0; /* reset*/

    if(isunlimited) {
	Constant* con;
        if(!firstdim) {
	    if(!issublist(src)) {
	        semerror(srcline(src),"Unlimited data must be enclosed in {..}");
	        return;
	    }
	    srcpush(src); /* enter the unlimited data */
	}
	while((con=srcpeek(src))!=NULL) {
	    if(lastdim) {
                genbin_data(basetype,src,fillsrc,memory);
	    } else {
                genbin_arraydatar(vsym,src,odom,index+1,
                                               checkpoint,memory);
	    }
	    odom->dims[index].index++;
	    if(docheckpoint) {
                closure->putvar(closure,odom,memory);
	    }
	}
        odom->dims[index].datasize = odom->dims[index].index;
	if(!firstdim) srcpop(src);
    } else { /* !isunlimited*/
	for(i=0;i<declsize;i++) {
	    con = srcpeek(src);
            if(lastdim) {
                genbin_data(basetype,src,fillsrc,memory);
            } else { /* ! lastdim*/
               (void)genbin_arraydatar(vsym,src,odom,
                                              index+1,checkpoint,memory);
            }
            odom->dims[index].index++;
            if(docheckpoint) {
                closure->putvar(closure,odom,memory);
            }
	}
    }
}

static const char zeros[] =
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
static void
alignbuffer(Constant* prim, Bytebuffer* buf)
{
    int alignment,pad,offset;

    if(prim->nctype == NC_ECONST)
        alignment = nctypealignment(prim->value.enumv->typ.typecode);
    else if(usingclassic && prim->nctype == NC_STRING)
        alignment = nctypealignment(NC_CHAR);
    else if(prim->nctype == NC_CHAR)
        alignment = nctypealignment(NC_CHAR);
    else
        alignment = nctypealignment(prim->nctype);
    offset = bbLength(buf);
    pad = getpadding(offset,alignment);
    if(pad > 0) {
	bbAppendn(buf,(void*)zeros,pad);
    }
}
#endif /*ENABLE_BINARY*/

