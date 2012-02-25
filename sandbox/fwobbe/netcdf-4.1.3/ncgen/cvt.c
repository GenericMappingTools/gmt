/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/cvt.c,v 1.2 2010/05/24 19:59:56 dmh Exp $ */

#include        "includes.h"
#include        "bytebuffer.h"
#include	<math.h>

static char stmp[256];

void
convert1(Constant* src, Constant* dst)
{
    Constvalue tmp;
    unsigned char* bytes = NULL;
    size_t bytelen;

    /* Need to translate all possible sources to all possible sinks.*/
    /* Rather than have a nested switch, combine the src and target into*/
    /* a single value so we can do a single n*n-way switch*/

    /* special case for src being NC_FILLVALUE*/
    if(src->nctype == NC_FILLVALUE) {
	if(dst->nctype != NC_FILLVALUE) {
	    nc_getfill(dst);
	} 
	return;
    }

    /* special case handling for src being NC_ECONST*/
    if(src->nctype == NC_ECONST) {
	if(dst->nctype == NC_ECONST) {
	    dst->value = src->value;
	} else {
	    Symbol* econst;
	    econst = src->value.enumv;
	    convert1(&econst->typ.econst,dst);
	}
	return;
    } else if(dst->nctype == NC_ECONST) {
	/* special case for dst being NC_ECONST*/
	semerror(lineno,"Conversion to enum not supported (yet)");
	return;
    }

    if(src->nctype == NC_OPAQUE) {
	ASSERT(src->value.opaquev.len >= 16);
        bytes = makebytestring(src->value.opaquev.stringv,&bytelen);
    }

#define CASE(nc1,nc2) (nc1*256+nc2)
	switch (CASE(src->nctype,dst->nctype)) {
case CASE(NC_CHAR,NC_CHAR):
    tmp.charv  = src->value.charv;
    break;
case CASE(NC_CHAR,NC_BYTE):
    tmp.int8v  = (unsigned char)src->value.charv;
    break;
case CASE(NC_CHAR,NC_UBYTE):
    tmp.uint8v	= (unsigned char)src->value.charv;
    break;
case CASE(NC_CHAR,NC_USHORT):
    tmp.uint16v = (unsigned short)src->value.charv;
    break;
case CASE(NC_CHAR,NC_UINT):
    tmp.uint32v = (unsigned int)src->value.charv;
    break;
case CASE(NC_CHAR,NC_UINT64):
    tmp.uint64v	 = (unsigned long long)src->value.charv;
    break;
case CASE(NC_CHAR,NC_SHORT):
    tmp.int16v	= (short)src->value.charv;
    break;
case CASE(NC_CHAR,NC_INT):
    tmp.int32v	= (int)src->value.charv;
    break;
case CASE(NC_CHAR,NC_INT64):
    tmp.int64v	 = (long long)src->value.charv;
    break;
case CASE(NC_CHAR,NC_FLOAT):
    tmp.floatv	= (float)src->value.charv;
    break;
case CASE(NC_CHAR,NC_DOUBLE):
    tmp.doublev = (double)src->value.charv;
    break;

case CASE(NC_BYTE,NC_CHAR):
    tmp.charv	= (char)src->value.uint8v;
    break;
case CASE(NC_BYTE,NC_BYTE):
    tmp.uint8v	= (unsigned char)src->value.uint8v;
    break;
case CASE(NC_BYTE,NC_UBYTE):
    tmp.uint8v	= (unsigned char)src->value.uint8v;
    break;
case CASE(NC_BYTE,NC_USHORT):
    tmp.uint16v = (unsigned short)src->value.uint8v;
    break;
case CASE(NC_BYTE,NC_UINT):
    tmp.uint32v = (unsigned int)src->value.uint8v;
    break;
case CASE(NC_BYTE,NC_UINT64):
    tmp.uint64v	 = (unsigned long long)src->value.uint8v;
    break;
case CASE(NC_BYTE,NC_SHORT):
    tmp.int16v	= (short)src->value.uint8v;
    break;
case CASE(NC_BYTE,NC_INT):
    tmp.int32v	= (int)src->value.uint8v;
    break;
case CASE(NC_BYTE,NC_INT64):
    tmp.int64v	 = (long long)src->value.uint8v;
    break;
case CASE(NC_BYTE,NC_FLOAT):
    tmp.floatv	= (float)src->value.uint8v;
    break;
case CASE(NC_BYTE,NC_DOUBLE):
    tmp.doublev = (double)src->value.uint8v;
    break;

case CASE(NC_UBYTE,NC_CHAR):
    tmp.charv	= (char)src->value.uint8v;
    break;
case CASE(NC_UBYTE,NC_BYTE):
    tmp.uint8v	= (unsigned char)src->value.uint8v;
    break;
case CASE(NC_UBYTE,NC_UBYTE):
    tmp.uint8v	= (unsigned char)src->value.uint8v;
    break;
case CASE(NC_UBYTE,NC_USHORT):
    tmp.uint16v = (unsigned short)src->value.uint8v;
    break;
case CASE(NC_UBYTE,NC_UINT):
    tmp.uint32v = (unsigned int)src->value.uint8v;
    break;
case CASE(NC_UBYTE,NC_UINT64):
    tmp.uint64v	 = (unsigned long long)src->value.uint8v;
    break;
case CASE(NC_UBYTE,NC_SHORT):
    tmp.int16v	= (short)src->value.uint8v;
    break;
case CASE(NC_UBYTE,NC_INT):
    tmp.int32v	= (int)src->value.uint8v;
    break;
case CASE(NC_UBYTE,NC_INT64):
    tmp.int64v	 = (long long)src->value.uint8v;
    break;
case CASE(NC_UBYTE,NC_FLOAT):
    tmp.floatv	= (float)src->value.uint8v;
    break;
case CASE(NC_UBYTE,NC_DOUBLE):
    tmp.doublev = (double)src->value.uint8v;
    break;

case CASE(NC_USHORT,NC_BYTE):
    tmp.uint8v	= (unsigned char)src->value.uint16v;
    break;
case CASE(NC_USHORT,NC_UBYTE):
    tmp.uint8v	= (unsigned char)src->value.uint16v;
    break;
case CASE(NC_USHORT,NC_USHORT):
    tmp.uint16v = (unsigned short)src->value.uint16v;
    break;
case CASE(NC_USHORT,NC_UINT):
    tmp.uint32v = (unsigned int)src->value.uint16v;
    break;
case CASE(NC_USHORT,NC_UINT64):
    tmp.uint64v	 = (unsigned long long)src->value.uint16v;
    break;
case CASE(NC_USHORT,NC_SHORT):
    tmp.int16v	= (short)src->value.uint16v;
    break;
case CASE(NC_USHORT,NC_INT):
    tmp.int32v	= (int)src->value.uint16v;
    break;
case CASE(NC_USHORT,NC_INT64):
    tmp.int64v	 = (long long)src->value.uint16v;
    break;
case CASE(NC_USHORT,NC_FLOAT):
    tmp.floatv	= (float)src->value.uint16v;
    break;
case CASE(NC_USHORT,NC_DOUBLE):
    tmp.doublev = (double)src->value.uint16v;
    break;

case CASE(NC_UINT,NC_BYTE):
    tmp.uint8v	= (unsigned char)src->value.uint32v;
    break;
case CASE(NC_UINT,NC_UBYTE):
    tmp.uint8v	= (unsigned char)src->value.uint32v;
    break;
case CASE(NC_UINT,NC_USHORT):
    tmp.uint16v = (unsigned short)src->value.uint32v;
    break;
case CASE(NC_UINT,NC_UINT):
    tmp.uint32v = (unsigned int)src->value.uint32v;
    break;
case CASE(NC_UINT,NC_UINT64):
    tmp.uint64v	 = (unsigned long long)src->value.uint32v;
    break;
case CASE(NC_UINT,NC_SHORT):
    tmp.int16v	= (short)src->value.uint32v;
    break;
case CASE(NC_UINT,NC_INT):
    tmp.int32v	= (int)src->value.uint32v;
    break;
case CASE(NC_UINT,NC_INT64):
    tmp.int64v	 = (long long)src->value.uint32v;
    break;
case CASE(NC_UINT,NC_FLOAT):
    tmp.floatv	= (float)src->value.uint32v;
    break;
case CASE(NC_UINT,NC_DOUBLE):
    tmp.doublev = (double)src->value.uint32v;
    break;

case CASE(NC_UINT64,NC_BYTE):
    tmp.uint8v	= (unsigned char)src->value.uint64v;
    break;
case CASE(NC_UINT64,NC_UBYTE):
    tmp.uint8v	= (unsigned char)src->value.uint64v;
    break;
case CASE(NC_UINT64,NC_USHORT):
    tmp.uint16v = (unsigned short)src->value.uint64v;
    break;
case CASE(NC_UINT64,NC_UINT):
    tmp.uint32v = (unsigned int)src->value.uint64v;
    break;
case CASE(NC_UINT64,NC_UINT64):
    tmp.uint64v	 = (unsigned long long)src->value.uint64v;
    break;
case CASE(NC_UINT64,NC_SHORT):
    tmp.int16v	= (short)src->value.uint64v;
    break;
case CASE(NC_UINT64,NC_INT):
    tmp.int32v	= (int)src->value.uint64v;
    break;
case CASE(NC_UINT64,NC_INT64):
    tmp.int64v	 = (long long)src->value.uint64v;
    break;
case CASE(NC_UINT64,NC_FLOAT):
    tmp.floatv	= (float)src->value.uint64v;
    break;
case CASE(NC_UINT64,NC_DOUBLE):
    tmp.doublev = (double)src->value.uint64v;
    break;

case CASE(NC_SHORT,NC_BYTE):
    tmp.uint8v	= (unsigned char)src->value.int16v;
    break;
case CASE(NC_SHORT,NC_UBYTE):
    tmp.uint8v	= (unsigned char)src->value.int16v;
    break;
case CASE(NC_SHORT,NC_USHORT):
    tmp.uint16v = (unsigned short)src->value.int16v;
    break;
case CASE(NC_SHORT,NC_UINT):
    tmp.uint32v = (unsigned int)src->value.int16v;
    break;
case CASE(NC_SHORT,NC_UINT64):
    tmp.uint64v	 = (unsigned long long)src->value.int16v;
    break;
case CASE(NC_SHORT,NC_SHORT):
    tmp.int16v	= (short)src->value.int16v;
    break;
case CASE(NC_SHORT,NC_INT):
    tmp.int32v	= (int)src->value.int16v;
    break;
case CASE(NC_SHORT,NC_INT64):
    tmp.int64v	 = (long long)src->value.int16v;
    break;
case CASE(NC_SHORT,NC_FLOAT):
    tmp.floatv	= (float)src->value.int16v;
    break;
case CASE(NC_SHORT,NC_DOUBLE):
    tmp.doublev = (double)src->value.int16v;
    break;

case CASE(NC_INT,NC_BYTE):
    tmp.uint8v	= (unsigned char)src->value.int32v;
    break;
case CASE(NC_INT,NC_UBYTE):
    tmp.uint8v	= (unsigned char)src->value.int32v;
    break;
case CASE(NC_INT,NC_USHORT):
    tmp.uint16v = (unsigned short)src->value.int32v;
    break;
case CASE(NC_INT,NC_UINT):
    tmp.uint32v = (unsigned int)src->value.int32v;
    break;
case CASE(NC_INT,NC_UINT64):
    tmp.uint64v	 = (unsigned long long)src->value.int32v;
    break;
case CASE(NC_INT,NC_SHORT):
    tmp.int16v	= (short)src->value.int32v;
    break;
case CASE(NC_INT,NC_INT):
    tmp.int32v	= (int)src->value.int32v;
    break;
case CASE(NC_INT,NC_INT64):
    tmp.int64v	 = (long long)src->value.int32v;
    break;
case CASE(NC_INT,NC_FLOAT):
    tmp.floatv	= (float)src->value.int32v;
    break;
case CASE(NC_INT,NC_DOUBLE):
    tmp.doublev = (double)src->value.int32v;
    break;

case CASE(NC_INT64,NC_BYTE):
    tmp.uint8v	= (unsigned char)src->value.int64v;
    break;
case CASE(NC_INT64,NC_UBYTE):
    tmp.uint8v	= (unsigned char)src->value.int64v;
    break;
case CASE(NC_INT64,NC_USHORT):
    tmp.uint16v = (unsigned short)src->value.int64v;
    break;
case CASE(NC_INT64,NC_UINT):
    tmp.uint32v = (unsigned int)src->value.int64v;
    break;
case CASE(NC_INT64,NC_UINT64):
    tmp.uint64v	 = (unsigned long long)src->value.int64v;
    break;
case CASE(NC_INT64,NC_SHORT):
    tmp.int16v	= (short)src->value.int64v;
    break;
case CASE(NC_INT64,NC_INT):
    tmp.int32v	= (int)src->value.int64v;
    break;
case CASE(NC_INT64,NC_INT64):
    tmp.int64v	 = (long long)src->value.int64v;
    break;
case CASE(NC_INT64,NC_FLOAT):
    tmp.floatv	= (float)src->value.int64v;
    break;
case CASE(NC_INT64,NC_DOUBLE):
    tmp.doublev = (double)src->value.int64v;
    break;

case CASE(NC_FLOAT,NC_BYTE):
    tmp.uint8v	= (unsigned char)src->value.floatv;
    break;
case CASE(NC_FLOAT,NC_UBYTE):
    tmp.uint8v	= (unsigned char)src->value.floatv;
    break;
case CASE(NC_FLOAT,NC_USHORT):
    tmp.uint16v = (unsigned short)src->value.floatv;
    break;
case CASE(NC_FLOAT,NC_UINT):
    tmp.uint32v = (unsigned int)src->value.floatv;
    break;
case CASE(NC_FLOAT,NC_UINT64):
    tmp.uint64v	 = (unsigned long long)src->value.floatv;
    break;
case CASE(NC_FLOAT,NC_SHORT):
    tmp.int16v	= (short)src->value.floatv;
    break;
case CASE(NC_FLOAT,NC_INT):
    tmp.int32v	= (int)src->value.floatv;
    break;
case CASE(NC_FLOAT,NC_INT64):
    tmp.int64v	 = (long long)src->value.floatv;
    break;
case CASE(NC_FLOAT,NC_FLOAT):
    tmp.floatv = src->value.floatv;
    break;
case CASE(NC_FLOAT,NC_DOUBLE):
    tmp.doublev = (isnan(src->value.floatv)?NAN:(double)src->value.floatv);
    break;
case CASE(NC_DOUBLE,NC_BYTE):
    tmp.uint8v	= (unsigned char)src->value.doublev;
    break;
case CASE(NC_DOUBLE,NC_UBYTE):
    tmp.uint8v	= (unsigned char)src->value.doublev;
    break;
case CASE(NC_DOUBLE,NC_USHORT):
    tmp.uint16v = (unsigned short)src->value.doublev;
    break;
case CASE(NC_DOUBLE,NC_UINT):
    tmp.uint32v = (unsigned int)src->value.doublev;
    break;
case CASE(NC_DOUBLE,NC_UINT64):
    tmp.uint64v	 = (unsigned long long)src->value.doublev;
    break;
case CASE(NC_DOUBLE,NC_SHORT):
    tmp.int16v	= (short)src->value.doublev;
    break;
case CASE(NC_DOUBLE,NC_INT):
    tmp.int32v	= (int)src->value.doublev;
    break;
case CASE(NC_DOUBLE,NC_INT64):
    tmp.int64v	 = (long long)src->value.doublev;
    break;
case CASE(NC_DOUBLE,NC_FLOAT):
    tmp.floatv = (isnan(src->value.doublev)?NANF:(float)src->value.doublev);
    break;
case CASE(NC_DOUBLE,NC_DOUBLE):
    tmp.doublev = (double)src->value.doublev;
    break;

/* Conversion of a string to e.g. an integer should be what?*/
case CASE(NC_STRING,NC_BYTE):
    sscanf(src->value.stringv.stringv,"%hhd",&tmp.int8v); break;
case CASE(NC_STRING,NC_UBYTE):
    sscanf(src->value.stringv.stringv,"%hhu",&tmp.uint8v); break;
case CASE(NC_STRING,NC_USHORT):
    sscanf(src->value.stringv.stringv,"%hu",&tmp.uint16v); break;
case CASE(NC_STRING,NC_UINT):
    sscanf(src->value.stringv.stringv,"%u",&tmp.uint32v); break;
case CASE(NC_STRING,NC_UINT64):
    sscanf(src->value.stringv.stringv,"%llu",&tmp.uint64v); break;
case CASE(NC_STRING,NC_SHORT):
    sscanf(src->value.stringv.stringv,"%hd",&tmp.int16v); break;
case CASE(NC_STRING,NC_INT):
    sscanf(src->value.stringv.stringv,"%d",&tmp.int32v); break;
case CASE(NC_STRING,NC_INT64):
    sscanf(src->value.stringv.stringv,"%lld",&tmp.int64v); break;
case CASE(NC_STRING,NC_FLOAT):
    sscanf(src->value.stringv.stringv,"%g",&tmp.floatv); break;
case CASE(NC_STRING,NC_DOUBLE):
    sscanf(src->value.stringv.stringv,"%lg",&tmp.doublev); break;
case CASE(NC_STRING,NC_CHAR):
     tmp.charv = src->value.stringv.stringv[0];
     break;
case CASE(NC_STRING,NC_STRING):
    tmp.stringv.stringv = nulldup(src->value.stringv.stringv);
    tmp.stringv.len = src->value.stringv.len;
    break;

/* What is the proper conversion for T->STRING?*/
case CASE(NC_CHAR,NC_STRING):
    sprintf(stmp,"%c",src->value.charv);
    tmp.stringv.len = nulllen(stmp);
    tmp.stringv.stringv = nulldup(stmp);
    break;
case CASE(NC_BYTE,NC_STRING):
    sprintf(stmp,"%hhd",src->value.uint8v);
    tmp.stringv.len = nulllen(stmp);
    tmp.stringv.stringv = nulldup(stmp);
    break;
case CASE(NC_UBYTE,NC_STRING):
    sprintf(stmp,"%hhu",src->value.uint8v);
    tmp.stringv.len = nulllen(stmp);
    tmp.stringv.stringv = nulldup(stmp);
    break;
case CASE(NC_USHORT,NC_STRING):
    sprintf(stmp,"%hu",src->value.uint16v);
    tmp.stringv.len = nulllen(stmp);
    tmp.stringv.stringv = nulldup(stmp);
    break;
case CASE(NC_UINT,NC_STRING):
    sprintf(stmp,"%u",src->value.uint32v);
    tmp.stringv.len = nulllen(stmp);
    tmp.stringv.stringv = nulldup(stmp);
    break;
case CASE(NC_UINT64,NC_STRING):
    sprintf(stmp,"%llu",src->value.uint64v);
    tmp.stringv.len = nulllen(stmp);
    tmp.stringv.stringv = nulldup(stmp);
    break;
case CASE(NC_SHORT,NC_STRING):
    sprintf(stmp,"%hd",src->value.int16v);
    tmp.stringv.len = nulllen(stmp);
    tmp.stringv.stringv = nulldup(stmp);
    break;
case CASE(NC_INT,NC_STRING):
    sprintf(stmp,"%d",src->value.int32v);
    tmp.stringv.len = nulllen(stmp);
    tmp.stringv.stringv = nulldup(stmp);
    break;
case CASE(NC_INT64,NC_STRING):
    sprintf(stmp,"%lld",src->value.int64v);
    tmp.stringv.len = nulllen(stmp);
    tmp.stringv.stringv = nulldup(stmp);
    break;
case CASE(NC_FLOAT,NC_STRING):
    sprintf(stmp,"%.8g",src->value.floatv);
    tmp.stringv.len = nulllen(stmp);
    tmp.stringv.stringv = nulldup(stmp);
    break;
case CASE(NC_DOUBLE,NC_STRING):
    sprintf(stmp,"%.8g",src->value.doublev);
    tmp.stringv.len = nulllen(stmp);
    tmp.stringv.stringv = nulldup(stmp);
    break;

case CASE(NC_OPAQUE,NC_CHAR):
    tmp.charv	= *(char*)bytes;
    break;
case CASE(NC_OPAQUE,NC_BYTE):
    tmp.uint8v	= *(unsigned char*)bytes;
    break;
case CASE(NC_OPAQUE,NC_UBYTE):
    tmp.uint8v	= *(unsigned char*)bytes;
    break;
case CASE(NC_OPAQUE,NC_USHORT):
    tmp.uint16v	= *(unsigned short*)bytes;
    break;
case CASE(NC_OPAQUE,NC_UINT):
    tmp.uint32v = *(unsigned int*)bytes;
    break;
case CASE(NC_OPAQUE,NC_UINT64):
    tmp.uint64v	 = *(unsigned long long*)bytes;
    break;
case CASE(NC_OPAQUE,NC_SHORT):
    tmp.int16v	= *(short*)bytes;
    break;
case CASE(NC_OPAQUE,NC_INT):
    tmp.int32v	= *(int*)bytes;
    break;
case CASE(NC_OPAQUE,NC_INT64):
    tmp.int64v	 = *(long long*)bytes;
    break;
case CASE(NC_OPAQUE,NC_FLOAT):
    tmp.floatv	= *(float*)bytes;
    break;
case CASE(NC_OPAQUE,NC_DOUBLE):
    tmp.doublev = *(double*)bytes;
    break;
case CASE(NC_OPAQUE,NC_OPAQUE):
    tmp.opaquev.stringv = nulldup(src->value.opaquev.stringv);
    tmp.opaquev.len = src->value.opaquev.len;
    break;

    /* We are missing all CASE(X,NC_ECONST) cases*/

    default:
	semerror(lineno,"transform: illegal conversion: %s/%d -> %s/%d",
		nctypename(src->nctype),src->nctype,
		nctypename(dst->nctype),dst->nctype);
	break;;
    }

    if(bytes != NULL) efree(bytes); /* cleanup*/

    /* overwrite minimum necessary parts*/
    dst->value = tmp;
}

/* Force an Opaque or string to conform to a given length*/
void
setprimlength(Constant* prim, unsigned long len)
{
    ASSERT(isprimplus(prim->nctype));
    if(prim->nctype == NC_STRING) {
        if(prim->value.stringv.len == len) {
	    /* do nothing*/
        } else if(prim->value.stringv.len > len) { /* truncate*/
	    prim->value.stringv.stringv[len] = '\0';
	    prim->value.stringv.len = len;
        } else {/* prim->value.stringv.len > srcov->len*/
	    char* s;
            s = (char*)emalloc(len+1);
	    memset(s,NC_FILL_CHAR,len);
	    s[len] = '\0';
	    memcpy(s,prim->value.stringv.stringv,prim->value.stringv.len);
	    efree(prim->value.stringv.stringv);
	    prim->value.stringv.stringv = s;
            prim->value.stringv.len = len;
	}
    } else if(prim->nctype == NC_OPAQUE) {
        if(prim->value.opaquev.len == len) { 
	    /* do nothing*/
        } else if(prim->value.opaquev.len > len) { /* truncate*/
	    if((len % 2) != 0) len--;
	    prim->value.opaquev.stringv[len] = '\0';
	    prim->value.opaquev.len = len;
        } else {/* prim->value.opaquev.len < len => expand*/
	    char* s;
	    if((len % 2) != 0) len++;
	    s = (char*)emalloc(len+1);
	    memset(s,'0',len);
	    memcpy(s,prim->value.opaquev.stringv,prim->value.opaquev.len);
	    s[len] = '\0';
	    efree(prim->value.opaquev.stringv);
	    prim->value.opaquev.stringv=s;
	    prim->value.opaquev.len = len;
        }
    }
}

Datalist*
convertstringtochars(Constant* str)
{
    int i;
    Datalist* dl;
    int slen;
    char* s;

    slen = str->value.stringv.len;
    dl = builddatalist(slen);
    s = str->value.stringv.stringv;
    for(i=0;i<slen;i++) {
	Constant con;
	con.nctype = NC_CHAR;
        con.lineno = str->lineno;
        con.value.charv = s[i];
	dlappend(dl,&con);
    }
    return dl;
}
