#include "includes.h"

#ifdef ENABLE_CML

/**************************************************/
/* Code for generating CML data lists*/
/**************************************************/

typedef enum Comma {FIRST, SECOND, SUPPRESS } Comma;

#define setcomma(comma) (comma)=((comma)!=SUPPRESS?SECOND:SUPPRESS)
#define clrcomma(comma) (comma)=((comma)!=SUPPRESS?LEAD:SUPPRESS)

/* Forward*/
static void gencml_data(Symbol* sym, Datasrc*, Datalist*, Bytebuffer*);
static void gencml_primdata(Symbol*, Datasrc*, Datalist*, Bytebuffer*);
static void gencml_fieldarray(Symbol*, Datasrc*, Dimset*, int, Bytebuffer*, Comma*);
static void gencml_arraydatar(Symbol* basetype,
	      Datasrc* src,
	      Odometer* odom,
	      int index,
	      Comma*,
	      Bytebuffer* code);
static void gencml_stringarray(Symbol*, Datasrc*, Bytebuffer*);

/* Datalist rules: see the rules on the man page */

/* Specialty wrappers for gencml_data */
void
gencml_attrdata(Symbol* asym, Bytebuffer* databuf)
{
    int first = 1;
    Datasrc* src;
    nc_type typecode = asym->typ.basetype->typ.typecode;

    if(asym->data == NULL) return;
    if(typecode == NC_CHAR) {gen_charattr(asym,databuf); return;}
    src = datalist2src(asym->data);
    if(typecode == NC_STRING) {gencml_stringarray(asym,src,databuf); return;}
    while(srcmore(src)) {
	if(!first) bbCat(databuf," "); else first = 0;
        gencml_data(asym->typ.basetype,src,NULL,databuf);
    }
}

void
gencml_scalardata(Symbol* vsym, Bytebuffer* databuf)
{
    Datasrc* src;

    if(vsym->data == NULL) return;
    src = datalist2src(vsym->data);
    gencml_data(vsym->typ.basetype,src,
		   vsym->var.special._Fillvalue,databuf);
    if(srcmore(src)) {
        semerror(srcline(src),"Extra data at end of datalist");
    }
}

static void
gencml_data(Symbol* tsym, Datasrc* datasrc, Datalist* fillsrc,
		Bytebuffer* databuf)
{
    int usecmpd;
    Constant* con = srcpeek(datasrc);
    if(con == NULL || con->nctype == NC_FILLVALUE) {
	srcnext(datasrc);
	gencml_fillvalue(tsym,fillsrc,datasrc,databuf);
	return;
    }

    switch (tsym->subclass) {

    case NC_ENUM:
    case NC_OPAQUE:
    case NC_PRIM:
	if(issublist(datasrc)) {
	    semerror(srcline(datasrc),"Expected primitive found {..}");
	}
	gencml_primdata(tsym,datasrc,fillsrc,databuf);
	break;

    case NC_COMPOUND: {
	int i;
        if(!issublist(datasrc)) {
	    semerror(srcline(datasrc),"Compound data must be enclosed in {..}");
        }
	srcpush(datasrc);
        bbCat(databuf,"<compound type=\"");
        bbCat(databuf,xtypename(tsym));
        bbCat(databuf,"\">\n");
        for(i=0;i<listlength(tsym->subnodes);i++) {
            Symbol* field = (Symbol*)listget(tsym->subnodes,i);
	    if(!srcmore(datasrc)) { /* generate a fill value*/
	        Datalist* fillsrc = getfiller(tsym);
	        gencml_data(field,datasrc,fillsrc,databuf);
	    } else
	        gencml_data(field,datasrc,NULL,databuf);
	}
        bbCat(databuf,"</compound>\n");
        srcpop(datasrc);
	} break;

    case NC_VLEN:
        if(!issublist(datasrc)) {
	    semerror(srcline(datasrc),"Vlen data must be enclosed in {..}");
        }
	srcpush(datasrc);
        bbCat(databuf,"<vlen type=\"");
        bbCat(databuf,xtypename(tsym));
        bbCat(databuf,"\">\n");
	gencml_data(tsym->typ.basetype,datasrc,NULL,databuf);
        bbCat(databuf,"\n</vlen>\n");
	srcpop(datasrc);
        break;

    case NC_FIELD:
	/* enclose in braces if and only if field is an array */
	usecmpd = (issublist(datasrc) && tsym->typ.dimset.ndims > 0);
	if(usecmpd) srcpush(datasrc);
        bbCat(databuf,"<field name=");
        bbCat(databuf,xname(tsym));
        bbCat(databuf," type=\"");
        bbCat(databuf,xtypename(tsym->typ.basetype));
        bbCat(databuf,"\"");
	if(isprimplus(tsym->typ.basetype->typ.typecode)) {
            bbCat(databuf," values=\"");
	} else {
            bbCat(databuf," >\n");
	}
	if(tsym->typ.dimset.ndims > 0) {
	    Comma comma = (isprimplus(tsym->typ.basetype->typ.typecode)?SUPPRESS:SECOND);
            gencml_fieldarray(tsym->typ.basetype,datasrc,&tsym->typ.dimset,0,databuf,&comma);
	} else {
	    gencml_data(tsym->typ.basetype,datasrc,NULL,databuf);
	}
	if(isprimplus(tsym->typ.basetype->typ.typecode)) {
            bbCat(databuf,"\" >\n");
	} else {
            bbCat(databuf,"</field>\n");
	}
	if(usecmpd) srcpop(datasrc);
	break;

    default: PANIC1("gencml_data: unexpected subclass %d",tsym->subclass);
    }
}

/* Used only for structure field arrays*/
static void
gencml_fieldarray(Symbol* basetype, Datasrc* src, Dimset* dimset, int index,
		Bytebuffer* databuf, Comma* commap)
{
    int i;
    Symbol* dim = dimset->dimsyms[index];
    unsigned int size = dim->dim.size;
    int lastdim = (index == (dimset->ndims - 1)); /* last dimension*/
    nc_type typecode = basetype->typ.typecode;

    if(typecode == NC_CHAR) {
	/* Collect the char field in a separate buffer */
	Bytebuffer* fieldbuf = bbNew();
        gen_charfield(src,dimset,index,fieldbuf);
	/* Add to the existing data buf as a single constant */
	xquotestring(fieldbuf);
	switch (*commap) {
	case FIRST: break;
	case SUPPRESS: bbCat(databuf," "); break;
	case SECOND: bbCat(databuf,"\n"); break;
	}
        bbAppendn(databuf,bbContents(fieldbuf),bbLength(fieldbuf));
	setcomma(*commap);
	bbFree(fieldbuf);
    } else if(typecode == NC_STRING) {
	Bytebuffer* fieldbuf = bbNew();
        gencml_stringarray(basetype,src,fieldbuf);
	switch (*commap) {
	case FIRST: break;
	case SUPPRESS: bbCat(databuf," "); break;
	case SECOND: bbCat(databuf,"\n"); break;
	}
        bbAppendn(databuf,bbContents(fieldbuf),bbLength(fieldbuf));
	setcomma(*commap);
	bbFree(fieldbuf);
    } else {
        ASSERT(size != 0);
        for(i=0;i<size;i++) {
            if(lastdim) {
		if(!isprimplus(typecode) && *commap==SECOND)
	            bbCat(databuf,"\n");
	        gencml_data(basetype,src,NULL,databuf);
	        setcomma(*commap);
            } else { /* !lastdim*/
	        gencml_fieldarray(basetype,src,dimset,index+1,databuf,commap);
    	    }
	}
    }
}

static void
gencml_primdata(Symbol* tsym, Datasrc* src, Datalist* fillsrc,
		Bytebuffer* databuf)
{
    Constant target, *prim;

    prim = srcnext(src);

    if(prim == NULL || prim->nctype == NC_FILLVALUE) {
	gencml_fillvalue(tsym,fillsrc,src,databuf);	
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
	    convert1(prim,&target);
	    setprimlength(&target,tsym->typ.size*2);
	    break;
        default:
	    convert1(prim,&target);
	    break;
    }
    bbCat(databuf,xconst(&target));
}

void
gencml_fillvalue(Symbol* tsym, Datalist* fillsrc, Datasrc* src,
               Bytebuffer* databuf)
{
    Datalist* list = NULL;

    ASSERT(tsym->objectclass == NC_TYPE);
    list = fillsrc;
    if(list == NULL) list = getfiller(tsym);
    srcpushlist(src,list);
    gencml_data(tsym,src,NULL,databuf);
    srcpop(src);
}

/* Result is a pool string or a constant => do not free*/
char*
xconst(Constant* ci)
{
    char tmp[64];
    tmp[0] = '\0';
    switch (ci->nctype) {
    case NC_CHAR: {
	char* escaped;
	char* result;
	tmp[0] = ci->value.charv;
	tmp[1] = '\0';
	escaped = xescapify(tmp,'\0',1);
	result = poolalloc(1+2+strlen(escaped));
	strcat(result,escaped);
	return result;
	} break;
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
    case NC_DOUBLE:
	sprintf(tmp,"%.16g",ci->value.doublev);
	break;
    case NC_UBYTE:
	    sprintf(tmp,"%hhu",ci->value.uint8v);
	break;
    case NC_USHORT:
	sprintf(tmp,"%hu",ci->value.uint16v);
	break;
    case NC_UINT:
	sprintf(tmp,"%uU",ci->value.uint32v);
	break;
    case NC_INT64:
	sprintf(tmp,"%lldLL",ci->value.int64v);
	break;
    case NC_UINT64:
	sprintf(tmp,"%lluLLU",ci->value.uint64v);
	break;
    case NC_ECONST:
	sprintf(tmp,"%s",cname(ci->value.enumv));
	break;
    case NC_STRING:
	{
	    char* escaped = xescapify(ci->value.stringv.stringv,
				 '\0',ci->value.stringv.len);
	    char* result = poolalloc(1+2+strlen(escaped));
	    strcat(result,escaped);
	    return result;
	}
	break;
    case NC_OPAQUE: {
	char* bstring;
	char* p;
	int bslen;
	bslen=(4*ci->value.opaquev.len);
	bstring = poolalloc(bslen+2+1);
	p = ci->value.opaquev.stringv;
	while(*p) {
	    strcat(bstring,"&#");
	    strncat(bstring,p,2);	    	    
	    strcat(bstring,";");
	    p += 2;	
	}
	return bstring;
	} break;

    default: PANIC1("ncstype: bad type code: %d",ci->nctype);
    }
    return pooldup(tmp); /*except for NC_STRING and NC_OPAQUE*/
}

void
gencml_arraydata(Symbol* vsym, Bytebuffer* databuf)
{
    Datasrc* src;
    Datalist* list;
    nc_type typecode = vsym->typ.basetype->typ.typecode;
    Comma comma = (isprimplus(typecode)?SUPPRESS:FIRST);

    if(vsym->data == NULL) return;

    if(typecode == NC_CHAR) {
        gen_chararray(vsym,NULL,databuf);
	return;
    }

    list = vsym->data;
    ASSERT(list->dimdata != NULL);
    src = datalist2src(list);

    if(typecode == NC_STRING) {
        gencml_stringarray(vsym,src,databuf);
	return;
    }

    gencml_arraydatar(vsym,src,list->dimdata,0,&comma,databuf);
}

static void
gencml_arraydatar(Symbol* vsym,
	      Datasrc* src,
	      Odometer* odom,
	      int index,
	      Comma* commap,
	      Bytebuffer* databuf)
{
    int i;
    int rank = odom->rank;
    int lastdim = (index == (rank - 1)); /* last dimension*/
    int firstdim = (index == 0);
    int declsize = odom->dims[index].declsize;
    int isunlimited = (declsize == 0);
    Symbol* basetype = vsym->typ.basetype;
    nc_type typecode = basetype->typ.typecode;
    Datalist* fillsrc = vsym->var.special._Fillvalue;
    Constant* con;

    ASSERT(index >= 0 && index < rank);
    odom->dims[index].index = 0; /* reset*/

    if(isunlimited) {
	Constant* con;
	Comma savecomma = *commap;
	*commap = (isprimplus(typecode)?SUPPRESS:FIRST);
        if(!firstdim) {
	    if(!issublist(src)) {
	        semerror(srcline(src),"Unlimited data must be enclosed in {..}");
	        return;
	    }
	    srcpush(src); /* enter the unlimited data */
	}
	bbCat(databuf,"<unlimited>\n");
	while((con=srcpeek(src))!=NULL) {
	    if(lastdim) {
		switch (*commap) {
		case FIRST: break;
		case SECOND: bbCat(databuf,"\n"); break;
		case SUPPRESS: bbCat(databuf," "); break;
		}
                gencml_data(basetype,src,fillsrc,databuf);
		setcomma(*commap);
	    } else {
                gencml_arraydatar(vsym,src,odom,index+1,commap,databuf);
	    }
	    odom->dims[index].index++;
	}
        odom->dims[index].datasize = odom->dims[index].index;
	bbCat(databuf,"</unlimited>");
	if(!firstdim) srcpop(src);
	*commap = savecomma;
    } else { /* !isunlimited*/
	for(i=0;i<declsize;i++) {
	    con = srcpeek(src);
            if(lastdim) {
		switch (*commap) {
		case FIRST: break;
		case SECOND: bbCat(databuf,"\n"); break;
		case SUPPRESS: bbCat(databuf," "); break;
		}
                gencml_data(basetype,src,fillsrc,databuf);
		setcomma(*commap);
            } else { /* ! lastdim*/
               (void)gencml_arraydatar(vsym,src,odom,index+1,commap,databuf);
            }
            odom->dims[index].index++;
	}
    }
}

void
xquotestring(Bytebuffer* databuf)
{
    char* escaped = xescapify(bbContents(databuf),'"',bbLength(databuf));
    bbClear(databuf);
    bbCat(databuf,escaped);
}


/* NcML wants sequences of strings to be encoded
by using some separator character. In this
experimental prototype, we use &#00; as the
separator.
This code is a modified version of gen_charattri code in genchar.c.
*/

static void
gencml_stringarray(Symbol* sym, Datasrc* src, Bytebuffer* databuf)
{
    Constant* con;
    char* xescaped;

    while((con=srcnext(src))) {
	bbCat(databuf,"&#00;");
	switch (con->nctype) {
	case NC_STRING:
	    xescaped = xescapify(con->value.stringv.stringv,'\0',
				con->value.stringv.len);
	    bbCat(databuf,xescaped);
	    break;
	case NC_FILLVALUE: {
	    Datalist* fill = getfiller(sym);
	    Datasrc* fillsrc = datalist2src(fill);
	    gencml_stringarray(sym,fillsrc,databuf);
	    } break;	    	    
	default:
	    semerror(srcline(src),
		     "Encountered non-string constant in attribute: %s",
		     sym->name);
	    return;
	}
    }	
}

#endif /*ENABLE_CML*/
