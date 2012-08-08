/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "includes.h"
#include "nciter.h"

#ifdef ENABLE_JAVA

#include <math.h> 

static int j_uid = 0;

static int
j_charconstant(Generator* generator, Bytebuffer* codebuf, ...)
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
j_constant(Generator* generator, Constant* con, Bytebuffer* buf,...)
{
    Bytebuffer* codetmp = bbNew();
    char* special = NULL;

    switch (con->nctype) {
    case NC_CHAR:
	if(con->value.charv == '\'') 
	    bbprintf(codetmp,"'\\''");
	else
	    bbprintf(codetmp,"'%c'",con->value.charv);
	break;
    case NC_BYTE:
	bbprintf(codetmp,"%hhd",con->value.int8v);
	break;
    case NC_SHORT:
	bbprintf(codetmp,"%hd",con->value.int16v);
	break;
    case NC_INT:
	bbprintf(codetmp,"%d",con->value.int32v);
	break;
    case NC_FLOAT:
	bbprintf(codetmp,"%f",con->value.floatv);
	break;
    case NC_DOUBLE:
	bbprintf(codetmp,"%lf",con->value.doublev);
	break;
    case NC_UBYTE:
        bbprintf(codetmp,"%hhu",con->value.uint8v);
	break;
    case NC_USHORT:
	bbprintf(codetmp,"%hu",con->value.uint16v);
	break;
    case NC_UINT:
	bbprintf(codetmp,"%uU",con->value.uint32v);
	break;
    case NC_INT64:
	bbprintf(codetmp,"%lldLL",con->value.int64v);
	break;
    case NC_UINT64:
	bbprintf(codetmp,"%lluLLU",con->value.uint64v);
	break;
    case NC_STRING: { /* handle separately */
	char* escaped = escapify(con->value.stringv.stringv,
				 '"',con->value.stringv.len);
	special = poolalloc(1+2+strlen(escaped));
	strcpy(special,"\"");
	strcat(special,escaped);
	strcat(special,"\"");
	} break;

    default: PANIC1("ncstype: bad type code: %d",con->nctype);

    }
    if(special == NULL)
        bbCatbuf(buf,codetmp);
    else
	bbCat(buf,special);
    bbFree(codetmp);
    return 1;
}

static int
j_listbegin(Generator* generator, ListClass lc, size_t size, Bytebuffer* codebuf, int* uidp, ...)
{
    if(uidp) *uidp = ++j_uid;
    switch (lc) {
    case LISTATTR:
    case LISTDATA:
	break;
    case LISTFIELDARRAY:
    case LISTVLEN:
    case LISTCOMPOUND:
	break;
    }
    return 1;
}

static int
j_list(Generator* generator, ListClass lc, int uid, size_t count, Bytebuffer* codebuf, ...)
{
    switch (lc) {
    case LISTATTR:
        if(count > 0) bbCat(codebuf,", ");
	break;
    case LISTDATA:
        bbCat(codebuf," ");
	break;
    case LISTVLEN:
    case LISTCOMPOUND:
    case LISTFIELDARRAY:
	break;
    }
    return 1;
}

static int
j_listend(Generator* generator, ListClass lc, int uid, size_t count, Bytebuffer* buf, ...)
{
    return 1;
}

static int
j_vlendecl(Generator* generator, Bytebuffer* codebuf, Symbol* tsym, int uid, size_t count, ...)
{
    return 1;
}

static int
j_vlenstring(Generator* generator, Bytebuffer* vlenmem, int* uidp, size_t* countp,...)
{
    if(uidp) *uidp = ++j_uid;
    if(countp) *countp = 0;
    return 1;
}


/* Define the single static bin data generator  */
static Generator j_generator_singleton = {
    NULL,
    j_charconstant,
    j_constant,
    j_listbegin,
    j_list,
    j_listend,
    j_vlendecl,
    j_vlenstring
};
Generator* j_generator = &j_generator_singleton;

#endif /*ENABLE_JAVA*/
