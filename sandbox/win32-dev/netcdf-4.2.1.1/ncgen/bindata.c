/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "includes.h"
#include "nciter.h"

#ifdef ENABLE_BINARY

static int bin_uid = 0;

static int
bin_charconstant(Generator* generator, Bytebuffer* buf, ...)
{
    /* Just transfer charbuf to codebuf */
    Bytebuffer* charbuf;
    va_list ap;
    vastart(ap,buf);
    charbuf = va_arg(ap, Bytebuffer*);
    va_end(ap);
    bbNull(charbuf);
    bbCatbuf(buf,charbuf);
    return 1;
}

static int
bin_constant(Generator* generator, Constant* con, Bytebuffer* buf,...)
{
    if(con->nctype != NC_ECONST) {
        alignbuffer(con,buf);
    }
    switch (con->nctype) {
    case NC_OPAQUE: {
        unsigned char* bytes;
        size_t len;
	/* Assume the opaque string has been normalized */
        bytes=makebytestring(con->value.opaquev.stringv,&len);
        bbAppendn(buf,(void*)bytes,len);
        } break;
    case NC_CHAR:
        bbAppendn(buf,&con->value.charv,sizeof(con->value.charv));
        break;
    case NC_BYTE:
        bbAppendn(buf,(void*)&con->value.int8v,sizeof(con->value.int8v));
        break;
    case NC_SHORT:
        bbAppendn(buf,(void*)&con->value.int16v,sizeof(con->value.int16v));
        break;
    case NC_INT:
        bbAppendn(buf,(void*)&con->value.int32v,sizeof(con->value.int32v));
        break;
    case NC_FLOAT:
        bbAppendn(buf,(void*)&con->value.floatv,sizeof(con->value.floatv));
        break;
    case NC_DOUBLE:
        bbAppendn(buf,(void*)&con->value.doublev,sizeof(con->value.doublev));
        break;
    case NC_UBYTE:
        bbAppendn(buf,(void*)&con->value.uint8v,sizeof(con->value.uint8v));
        break;
    case NC_USHORT:
        bbAppendn(buf,(void*)&con->value.uint16v,sizeof(con->value.uint16v));
        break;
    case NC_UINT:
        bbAppendn(buf,(void*)&con->value.uint32v,sizeof(con->value.uint32v));
        break;
    case NC_INT64: {
        union SI64 { char ch[8]; long long i64;} si64;
        si64.i64 = con->value.int64v;
        bbAppendn(buf,(void*)si64.ch,sizeof(si64.ch));
        } break;
    case NC_UINT64: {
        union SU64 { char ch[8]; unsigned long long i64;} su64;
        su64.i64 = con->value.uint64v;
        bbAppendn(buf,(void*)su64.ch,sizeof(su64.ch));
        } break;
    case NC_STRING: {
        char* ptr;
        int len = (size_t)con->value.stringv.len;
        ptr = (char*)malloc(len+1);
        memcpy(ptr,con->value.stringv.stringv,len);
        ptr[len] = '\0';
        bbAppendn(buf,(void*)&ptr,sizeof(ptr));
        } break;

    default: PANIC1("bin_constant: unexpected type: %d",con->nctype);
    }
    return 1;
}

static int
bin_listbegin(Generator* generator, ListClass lc, size_t size, Bytebuffer* buf, int* uidp, ...)
{
    if(uidp) *uidp = ++bin_uid;
    return 1;
}

static int
bin_list(Generator* generator, ListClass lc, int uid, size_t count, Bytebuffer* buf, ...)
{
    return 1;
}

static int
bin_listend(Generator* generator, ListClass lc, int uid, size_t count, Bytebuffer* buf, ...)
{
    return 1;
}


static int
bin_vlendecl(Generator* generator, Bytebuffer* buf, Symbol* tsym, int uid, size_t count,...)
{
    va_list ap;
    Bytebuffer* vlenmem;
    nc_vlen_t ptr;
    vastart(ap,count);
    vlenmem = va_arg(ap, Bytebuffer*);
    va_end(ap);
    ptr.len = count;
    ptr.p = bbDup(vlenmem);
    bbAppendn(buf,(char*)&ptr,sizeof(ptr));
    return 1;
}

static int
bin_vlenstring(Generator* generator, Bytebuffer* codebuf, int* uidp, size_t* sizep,...)
{
    Bytebuffer* vlenmem;
    nc_vlen_t ptr;
    va_list ap;
    if(uidp) *uidp = ++bin_uid;
    vastart(ap,sizep);
    vlenmem = va_arg(ap, Bytebuffer*);
    va_end(ap);
    ptr.len = bbLength(vlenmem);
    ptr.p = bbDup(vlenmem);
    bbAppendn(codebuf,(char*)&ptr,sizeof(ptr));
    return 1;
}

/* Define the single static bin data generator  */
static Generator bin_generator_singleton = {
    NULL,
    bin_charconstant,
    bin_constant,
    bin_listbegin,
    bin_list,
    bin_listend,
    bin_vlendecl,
    bin_vlenstring
};
Generator* bin_generator = &bin_generator_singleton;

#endif /*ENABLE_BINARY*/
