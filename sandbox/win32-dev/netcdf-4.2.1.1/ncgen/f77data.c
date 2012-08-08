/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "includes.h"
#include "nciter.h"

#ifdef ENABLE_F77

#include <math.h> 

int f77_uid = 0;

static int
f77_charconstant(Generator* generator, Bytebuffer* codebuf, ...)
{
    /* Escapes and quoting will be handled in genc_write */
    /* Just transfer charbuf to codebuf */
    Bytebuffer* charbuf;
    va_list ap;
    vastart(ap,codebuf);
    charbuf = va_arg(ap, Bytebuffer*);
    va_end(ap);
    bbNull(charbuf);
    bbCatbuf(codebuf,charbuf);
    return 1;
}

static int
f77_constant(Generator* generator, Constant* ci, Bytebuffer* codebuf,...)
{
    char tmp[64];
    char* special = NULL;
    switch (ci->nctype) {

    case NC_CHAR:
	if(ci->value.charv == '\'') 
	    sprintf(tmp,"'\\''");
	else
	    sprintf(tmp,"'%c'",ci->value.charv);
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
	    special = bbDup(buf);
	    bbFree(buf);
	}
	break;

    default: PANIC1("f77data: bad type code: %d",ci->nctype);

    }
    if(special != NULL)
	bbCat(codebuf,special);
    else
	bbCat(codebuf,tmp);
    return 1;
}

static int
f77_listbegin(Generator* generator, ListClass lc, size_t size, Bytebuffer* codebuf, int* uidp, ...)
{
    if(uidp) *uidp = ++f77_uid;
    return 1;
}

static int
f77_list(Generator* generator, ListClass lc, int uid, size_t count, Bytebuffer* codebuf, ...)
{
    switch (lc) {
    case LISTATTR:
        if(count > 0) bbCat(codebuf,", ");
	break;
    case LISTDATA:
        bbAppend(codebuf,' ');
	break;
    case LISTVLEN:
    case LISTCOMPOUND:
    case LISTFIELDARRAY:
	break;
    }
    return 1;
}

static int
f77_listend(Generator* generator, ListClass lc, int uid, size_t count, Bytebuffer* buf, ...)
{
    return 1;
}

static int
f77_vlendecl(Generator* generator, Bytebuffer* codebuf, Symbol* tsym, int uid, size_t count, ...)
{
    return 1;
}

static int
f77_vlenstring(Generator* generator, Bytebuffer* vlenmem, int* uidp, size_t* countp,...)
{
    if(uidp) *uidp = ++f77_uid;
    return 1;
}

/* Define the single static bin data generator  */
static Generator f77_generator_singleton = {
    NULL,
    f77_charconstant,
    f77_constant,
    f77_listbegin,
    f77_list,
    f77_listend,
    f77_vlendecl,
    f77_vlenstring
};
Generator* f77_generator = &f77_generator_singleton;

#endif /*ENABLE_F77*/
