#include "includes.h"
#include "offsets.h"

#ifdef ENABLE_JAVA

extern List* vlenconstants;  /* List<Constant*>;*/

/* Forward*/
static void jdata_primdata(Symbol*, Datasrc*, Bytebuffer*, Datalist*);

/**************************************************/
/* Code for generating Java language data lists*/
/**************************************************/
/* Datalist rules: see the rules on the man page */

void
jdata_array(Symbol* vsym,
		  Bytebuffer* databuf,
		  Datasrc* src,
		  Odometer* odom,
	          int index,
		  Datalist* fillsrc)
{
    int i;
    int rank = odom->rank;
    int firstdim = (index == 0); /* first dimension*/
    int lastdim = (index == (rank - 1)); /* last dimension*/
    size_t count;
    Symbol* basetype = vsym->typ.basetype;
    int isunlimited = (odom->declsize[index] == 0);
    int pushed = 0;

    ASSERT(index >= 0 && index < rank);

    count = odom->count[index];

    /* Unpack the nested unlimited (unless first dimension)*/
    if(!firstdim && isunlimited && issublist(src)) {
	srcpush(src);
	pushed = 1;
    }

    if(lastdim) {
        for(i=0;i<count;i++) {
            jdata_basetype(basetype,src,databuf,fillsrc);
	}
    } else {
        /* now walk count elements and generate recursively */
        for(i=0;i<count;i++) {
	    jdata_array(vsym,databuf,src,odom,index+1,fillsrc);
	}
    }

    if(isunlimited && pushed) srcpop(src);

    return;
}

/* Generate an instance of the basetype using datasrc */
void
jdata_basetype(Symbol* tsym, Datasrc* datasrc, Bytebuffer* codebuf, Datalist* fillsrc)
{
    switch (tsym->subclass) {

    case NC_PRIM:
	if(issublist(datasrc)) {
	    semerror(srcline(datasrc),"Expected primitive found {..}");
	}
	bbAppend(codebuf,' ');
	jdata_primdata(tsym,datasrc,codebuf,fillsrc);
	break;

    default: PANIC1("jdata_basetype: unexpected subclass %d",tsym->subclass);
    }
}

/**************************************************/
static void
jdata_primdata(Symbol* basetype, Datasrc* src, Bytebuffer* databuf, Datalist* fillsrc)
{
    Constant target, *prim;

    prim = srcnext(src);
    if(prim == NULL) prim = &fillconstant;

    ASSERT(prim->nctype != NC_COMPOUND);

    if(prim->nctype == NC_FILLVALUE) {
	Datalist* filler = getfiller(basetype,fillsrc);
	ASSERT(filler->length == 1);
	srcpushlist(src,filler);
	bbAppend(databuf,' ');
        jdata_primdata(basetype,src,databuf,NULL);
	srcpop(src);
	goto done;
    }

    target.nctype = basetype->typ.typecode;

    ASSERT(prim->nctype != NC_COMPOUND);

    convert1(prim,&target);
    /* add hack for java bug in converting floats */
    if(target.nctype == NC_FLOAT) bbCat(databuf," (float)");
    bbCat(databuf,jdata_const(&target));
done:
    return;
}

/* Result is a pool string or a constant => do not free*/
char*
jdata_const(Constant* ci)
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
    case NC_DOUBLE:
	sprintf(tmp,"%.16g",ci->value.doublev);
	break;

    case NC_STRING:
	{
	    char* escaped = escapify(ci->value.stringv.stringv,
				 '"',ci->value.stringv.len);
	    result = poolalloc(1+2+strlen(escaped));
	    strcpy(result,"\"");
	    strcat(result,escaped);
	    strcat(result,"\"");
	    goto done;
	}
	break;

    default: PANIC1("ncstype: bad type code: %d",ci->nctype);
    }
    result = pooldup(tmp);
done:
    return result; /*except for NC_STRING and NC_OPAQUE*/
}

#endif /*ENABLE_JAVA*/
