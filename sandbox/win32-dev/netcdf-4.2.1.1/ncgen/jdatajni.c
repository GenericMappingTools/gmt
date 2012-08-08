#include "includes.h"
#include "offsets.h"

#ifdef ENABLE_JAVA

extern List* vlenconstants;  /* List<Constant*>;*/

/* Forward*/
static void genjjni_primdata(Symbol*, Datasrc*, Datalist*, char*, Bytebuffer*);
static void genjjni_data(Symbol* sym, Datasrc*, Datalist*, char*, Bytebuffer*);
static void genjjni_fieldarray(Symbol*, Datasrc*, Dimset*, int, char*, Bytebuffer*);
static void genjjni_arraydatar(Symbol* basetype,
	      Datasrc* src,
	      Odometer* odom,
	      Putvar* closure,
	      int index,
	      int checkpoint,
	      Bytebuffer* code);

static void jopaquestring(Symbol* tsym, Constant* prim, Constant* target);
static int genjjni_putmemory(struct Symbol*, char*, Bytebuffer*, Bytebuffer*);
static int jalignment(Symbol* tsym);
static int jvlenalignment(void);

/**************************************************/
/* Code for generating Java language data lists*/
/**************************************************/
/* Datalist rules: see the rules on the man page */

/* Specialty wrappers for genjjni_data */
void
genjjni_attrdata(Symbol* asym, Bytebuffer* databuf)
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
            genjjni_data(asym->typ.basetype,src,NULL,memname,databuf);
	}
    }
}

void
genjjni_scalardata(Symbol* vsym, Bytebuffer* databuf)
{
    Datasrc* src;
    char* memname = "memory";

    if(vsym->data == NULL) return;
    src = datalist2src(vsym->data);
    genjjni_data(vsym->typ.basetype,src,
		   vsym->var.special._Fillvalue,memname,databuf);
    if(srcmore(src)) {
        semerror(srcline(src),"Extra data at end of datalist");
    }
}

void
genjjni_fillvalue(Symbol* tsym, Datalist* fillsrc, Datasrc* src,
               char* memname, Bytebuffer* databuf)
{
    Datalist* list = NULL;

    ASSERT(tsym->objectclass == NC_TYPE);
    list = fillsrc;
    if(list == NULL) list = getfiller(tsym);
    srcpushlist(src,list);
    genjjni_data(tsym,src,NULL,memname,databuf);
    srcpop(src);
}

void
genjjni_arraydata(Symbol* vsym, Putvar* closure, Bytebuffer* databuf)
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

    if(typecode == NC_CHAR)
        gen_chararray(vsym,closure,databuf);
    else {
        genjjni_arraydatar(vsym,src,list->dimdata,closure,0,checkpoint,databuf);
    }
    commify(databuf);
}

static void
genjjni_arraydatar(Symbol* vsym,
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
    int docheckpoint = (closure != NULL && index == checkpoint);
    int declsize = odom->dims[index].declsize;
    int isunlimited = (declsize == 0);
    Symbol* basetype = vsym->typ.basetype;
    Datalist* fillsrc = vsym->var.special._Fillvalue;
    Constant* con;
    nc_type typecode = basetype->typ.typecode;
    char* memname = "memory";

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
		if(isprimplus(typecode)) {
		    bbCat(databuf," ");
                    genjjni_primdata(basetype,src,fillsrc,memname,databuf);
		} else
                    genjjni_data(basetype,src,fillsrc,memname,databuf);
	    } else {
                genjjni_arraydatar(vsym,src,odom,closure,index+1,
                                               checkpoint,databuf);
	    }
	    odom->dims[index].index++;
	    if(docheckpoint) {
                closure->putvar(closure,odom,databuf);
	    }
	}
        odom->dims[index].datasize = odom->dims[index].index;
	if(!firstdim) srcpop(src);
    } else { /* !isunlimited*/
	for(i=0;i<declsize;i++) {
	    con = srcpeek(src);
            if(lastdim) {
		if(isprimplus(typecode)) {
		    bbCat(databuf," ");
                    genjjni_primdata(basetype,src,fillsrc,memname,databuf);
	        } else
                    genjjni_data(basetype,src,fillsrc,memname,databuf);
            } else { /* ! lastdim*/
               (void)genjjni_arraydatar(vsym,src,odom,closure,
                                        index+1,checkpoint,
					databuf);
            }
            odom->dims[index].index++;
            if(docheckpoint) {
                closure->putvar(closure,odom,databuf);
            }
	}
    }
}

/**************************************************/
/* This is the general datalist processing procedure */
static void
genjjni_data(Symbol* tsym, Datasrc* datasrc, Datalist* fillsrc,
		char* memname, Bytebuffer* databuf)
{
    int usecmpd;
    Constant* con = srcpeek(datasrc);
    Bytebuffer* constdata = NULL;
    nc_type typecode = tsym->typ.typecode;

    if(con == NULL || con->nctype == NC_FILLVALUE) {
	srcnext(datasrc);
	genjjni_fillvalue(tsym,fillsrc,datasrc,memname,databuf);
	return;
    }

    switch (tsym->subclass) {

    case NC_ENUM:
    case NC_OPAQUE:
    case NC_PRIM:
	if(issublist(datasrc)) {
	    semerror(srcline(datasrc),"Expected primitive found {..}");
	}
	bbCat(databuf," ");
	genjjni_primdata(tsym,datasrc,NULL,memname,databuf);
	break;

    case NC_COMPOUND: {
	int i;
        if(!issublist(datasrc)) {
	    semerror(srcline(datasrc),
		     "Compound data must be enclosed in {..}");
        }
	srcpush(datasrc);
        for(i=0;i<listlength(tsym->subnodes);i++) {
            Symbol* field = (Symbol*)listget(tsym->subnodes,i);
	    genjjni_data(field,datasrc,fillsrc,memname,databuf);
	}
        srcpop(datasrc);
	} break;

    case NC_VLEN: {
	char stmt[C_MAX_STMT];
        Constant* cp;
	char vlenname[1024];

        if(!issublist(datasrc)) {
	    semerror(srcline(datasrc),"Vlen data must be enclosed in {..}");
        }
        cp = srcnext(datasrc);
        sprintf(vlenname,"vlen_%u",cp->value.compoundv->vlen.uid);
	/* Use special alignment */
        nprintf(stmt,sizeof(stmt),"Memory.align(%s,%d);\n",
		vlenname,jvlenalignment());
        bbCat(databuf,stmt);
        /* generate and store the nc_vlen_t instance*/
        sprintf(stmt,"Memory.put_vlen(%s,new long[]{%lld, %s_addr});\n",
		memname,
		(long long)cp->value.compoundv->vlen.count,
		vlenname);
        bbCat(databuf,stmt);
        } break;

    case NC_FIELD:
	/* enclose in braces if and only if field is an array */
	usecmpd = (issublist(datasrc) && tsym->typ.dimset.ndims > 0);
	if(usecmpd) srcpush(datasrc);
	constdata = bbNew();
	if(tsym->typ.dimset.ndims > 0) {
            genjjni_fieldarray(tsym->typ.basetype,datasrc,&tsym->typ.dimset,0,
                            memname,constdata);
	} else {
	    genjjni_data(tsym->typ.basetype,datasrc,NULL,memname,constdata);
	}
	/* If this was a primitive valued field, then generate a
           Memory.put_XXX call
        */
	if(isprimplus(typecode)) {
	    genjjni_putmemory(tsym,memname,databuf,constdata);
	} else /* It is already one or more Memory.put_XX  calls */
	    bbAppendn(databuf,bbContents(constdata),bbLength(constdata));
	if(usecmpd) srcpop(datasrc);
	bbFree(constdata);
	bbNull(databuf);
	break;

    default: PANIC1("genjjni_data: unexpected subclass %d",tsym->subclass);
    }
}

/* Used only for structure field arrays*/
static void
genjjni_fieldarray(Symbol* basetype, Datasrc* src, Dimset* dimset, int index,
		char* memname, Bytebuffer* databuf)
{
    int i;
    Symbol* dim = dimset->dimsyms[index];
    unsigned int size = dim->dim.size;
    int lastdim = (index == (dimset->ndims - 1)); /* last dimension*/
    int typecode = basetype->typ.typecode;

    if(typecode == NC_CHAR) {
	/* Collect the char field in a separate buffer */
	Bytebuffer* fieldbuf = bbNew();
        gen_charfield(src,dimset,index,fieldbuf);
	/* Add to the existing data buf using Memory */
	jquotestring(fieldbuf,'"');
	bbCat(databuf," ");
	bbCat(databuf,bbContents(fieldbuf));
	bbFree(fieldbuf);
    } else {
        ASSERT(size != 0);
        for(i=0;i<size;i++) {
	    bbCat(databuf," ");
            if(lastdim) {
	        if(isprimplus(typecode)) {
		    bbCat(databuf," ");
		    genjjni_primdata(basetype,src,NULL,memname,databuf);
		} else
	            genjjni_data(basetype,src,NULL,memname,databuf);
            } else { /* !lastdim*/
	        genjjni_fieldarray(basetype,src,dimset,index+1,memname,databuf);
    	    }
	}
    }
}

void
genjjni_vlendata(List* vlenconstants, Bytebuffer* databuf)
{
    int i,count;
    Datasrc* vlensrc;
    char tmp[C_MAX_STMT];
    char memname[1024];
    Bytebuffer* vlenbuf = bbNew();

    for(i=0;i<listlength(vlenconstants);i++) {
	Constant* cmpd = (Constant*)listget(vlenconstants,i);
	Symbol* tsym = cmpd->value.compoundv->vlen.schema;
        Symbol* basetype = tsym->typ.basetype;
	int typecode = basetype->typ.typecode;

        sprintf(memname,"vlen_%u",cmpd->value.compoundv->vlen.uid);

	count = 0;
	vlensrc = datalist2src(cmpd->value.compoundv);
 	bbClear(vlenbuf);
	if(typecode == NC_CHAR) {
   	    /* Collect the char vlen in a separate buffer */
            gen_charvlen(vlensrc,vlenbuf);
	    count = bbLength(vlenbuf);
	    /* Add to the existing data buf as a single constant */
   	    jquotestring(vlenbuf,'"');
	    /* Insert into the Memory buffer */
            genjjni_putmemory(basetype,memname,databuf,vlenbuf);
	} else {
  	    while(srcmore(vlensrc)) {
                genjjni_data(tsym->typ.basetype,vlensrc,NULL,memname,vlenbuf);
	        count++;
	    }
	    ASSERT(count == cmpd->value.compoundv->vlen.count);
	    /* Hack to handle user-defined vs primitive basetypes */
	    if(isprimplus(typecode)) {
		genjjni_putmemory(basetype,memname,databuf,vlenbuf);
	    } else { /* constdata should have the necessary Memory.put ops */
	        bbCat(databuf,bbContents(vlenbuf));
	    }
	    /* Now store the address */
            sprintf(tmp,"%s_addr = Memory.address(%s);\n",
			memname,memname);
	    bbCat(databuf,tmp);
        }
    }
    bbFree(vlenbuf);
}

void
genjjni_vlenconstants(List* vlenconstants, Bytebuffer* databuf)
{
    int i;
    char tmp[C_MAX_STMT];
    char memname[1024];

    for(i=0;i<listlength(vlenconstants);i++) {
	Constant* cmpd = (Constant*)listget(vlenconstants,i);

        sprintf(memname,"vlen_%u",cmpd->value.compoundv->vlen.uid);
	/* Define the memory buffer and a place to stick its address */
        sprintf(tmp,"static long %s = Memory.create();\n",memname);
	bbCat(databuf,tmp);
        sprintf(tmp,"static long %s_addr = 0L;\n",memname);
	bbCat(databuf,tmp);
    }
}

/**************************************************/
static void
genjjni_primdata(Symbol* tsym, Datasrc* src, Datalist* filler,
		char* memname, Bytebuffer* databuf)
{
    Constant target, *prim;

    prim = srcnext(src);

    if(prim == NULL || prim->nctype == NC_FILLVALUE) {
	genjjni_fillvalue(tsym,filler,src,memname,databuf);
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
        case NC_ECONST:
	    if(tsym->subclass != NC_ENUM) {
	        semerror(prim->lineno,"Conversion to enum not supported (yet)");
	        nc_getfill(&target);
	    } else {
	        target.nctype = NC_ECONST;
		convert1(prim,&target);
	    }
   	    break;
        case NC_OPAQUE:
            target.nctype = NC_STRING;
	    jopaquestring(tsym,prim,&target);
	    break;
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

/* Convert an opaque to a java string */
static void
jopaquestring(Symbol* tsym, Constant* prim, Constant* target)
{
    int i;
    static char hexdigits[] = "0123456789ABCDEF";
    char* op = prim->value.opaquev.stringv;
    size_t oplen = prim->value.opaquev.len;
    unsigned char* p;
    char* opstring;

    ASSERT((oplen%2) == 0);

    opstring = (char*)emalloc(oplen*(2+4));
    opstring[0]='\0';    

    p=(unsigned char*)op;
    for(i=0;i<oplen;i++,p++) {
	char tmp[8];
	unsigned int b = *p;
	char digit1 = hexdigits[(b & 0x0f)];
	char digit2 = hexdigits[((b>>4) & 0x0f)];
	strcpy(tmp,"\\u00");
	tmp[5] = digit2;
	tmp[6] = digit1;
	tmp[7] = '\0';
	strcat(opstring,tmp);
    }
    target->value.stringv.stringv = opstring;
    target->value.stringv.len = strlen(opstring);
}

static int
jalignment(Symbol* tsym)
{
    nc_type nctype = tsym->typ.typecode;
    int alignment = 0;
    if(nctype == NC_ECONST)
        alignment = nctypealignment(tsym->typ.typecode);
    else if(usingclassic && nctype == NC_STRING)
        alignment = nctypealignment(NC_CHAR);
    else if(nctype == NC_CHAR)
        alignment = nctypealignment(NC_CHAR);
    else
        alignment = nctypealignment(nctype);
    return alignment;
}

static int
genjjni_putmemory(Symbol* tsym, char* memname, Bytebuffer* databuf, Bytebuffer* constbuf)
{
    char stmt[1024];
    nc_type typecode = tsym->typ.typecode;

    commify(constbuf);
    nprintf(stmt,sizeof(stmt),"Memory.align(%s,%d);\n",
		memname,jalignment(tsym));
    bbCat(databuf,stmt);

    if(typecode == NC_CHAR) {
        nprintf(stmt,sizeof(stmt),"Memory.put_%sarray(%s,",
		jstype(typecode),memname);
        bbCat(databuf,stmt);
        bbAppendn(databuf,bbContents(constbuf),bbLength(constbuf));
        bbCat(databuf,");\n");
    } else {
        nprintf(stmt,sizeof(stmt),"Memory.put_%sarray(%s,new %s[]{",
	    jstype(typecode),
	    memname,
	    jarraytype(typecode));
        bbCat(databuf,stmt);
        bbAppendn(databuf,bbContents(constbuf),bbLength(constbuf));
        bbCat(databuf,"});\n");
    }
    return NC_NOERR;
}

static int
jvlenalignment(void)
{
    return nctypealignment(NC_VLEN);
}

#endif /*ENABLE_JAVA*/
