/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/dapcvt.c,v 1.14 2009/11/29 00:16:26 dmh Exp $
 *********************************************************************/
#include "config.h"
#include "ncdap3.h"
#include "dapodom.h"

NCerror
dapconvert3(nc_type srctype, nc_type dsttype, char* memory0, char* value0, size_t count)
{
    NCerror ncstat = NC_NOERR;
    size_t i;
    char* memory = memory0;
    char* value = value0;

    /* In order to deal with the DAP upgrade problem,
	try to preserve the bit patterns
    */

    /* Provide space and pointer casts for intermediate results */
    signed char ncbyte;
    signed char* ncbytep;
    char ncchar;
    char* nccharp;
    short ncshort;
    short* ncshortp;
    int ncint;
    int* ncintp;
    float ncfloat;
    float* ncfloatp;
    double ncdouble;
    double* ncdoublep;
    unsigned char ncubyte;
    unsigned char* ncubytep;
    unsigned short ncushort;
    unsigned short* ncushortp;
    unsigned int ncuint;
    unsigned int* ncuintp;
    long long ncint64;
    long long* ncint64p;
    unsigned long long ncuint64;
    unsigned long long* ncuint64p;


#define CASE(nc1,nc2) (nc1*256+nc2)
#define CUT8(e) ((unsigned char)((e) & 0xff))
#define CUT16(e) ((unsigned short)((e) & 0xffff))
#define CUT32(e) ((unsigned int)((e) & 0xffffffff))
#define ARM(vs,ncs,ts,vd,ncd,td) \
case CASE(ncs,ncd):\
    vs##p = (ts *)value;\
    vs = *vs##p;\
    vd##p = (td *)memory;\
    *vd##p = (td)vs;\
    break;

    for(i=0;i<count;i++) {

        switch (CASE(srctype,dsttype)) {
ARM(ncchar,NC_CHAR,char,ncchar,NC_CHAR,char)
ARM(ncchar,NC_CHAR,char,ncbyte,NC_BYTE,signed char)
ARM(ncchar,NC_CHAR,char,ncubyte,NC_UBYTE,unsigned char)
ARM(ncchar,NC_CHAR,char,ncshort,NC_SHORT,short)
ARM(ncchar,NC_CHAR,char,ncushort,NC_USHORT,unsigned short)
ARM(ncchar,NC_CHAR,char,ncint,NC_INT,int)
ARM(ncchar,NC_CHAR,char,ncuint,NC_UINT,unsigned int)
ARM(ncchar,NC_CHAR,char,ncint64,NC_INT64,long long)
ARM(ncchar,NC_CHAR,char,ncuint64,NC_UINT64,unsigned long long)
ARM(ncchar,NC_CHAR,char,ncfloat,NC_FLOAT,float)
ARM(ncchar,NC_CHAR,char,ncdouble,NC_DOUBLE,double)
ARM(ncbyte,NC_BYTE,signed char,ncchar,NC_CHAR,char)
ARM(ncbyte,NC_BYTE,signed char,ncbyte,NC_BYTE,signed char)
ARM(ncbyte,NC_BYTE,signed char,ncubyte,NC_UBYTE,unsigned char)
ARM(ncbyte,NC_BYTE,signed char,ncshort,NC_SHORT,short)
ARM(ncbyte,NC_BYTE,signed char,ncushort,NC_USHORT,unsigned short)
ARM(ncbyte,NC_BYTE,signed char,ncint,NC_INT,int)
ARM(ncbyte,NC_BYTE,signed char,ncuint,NC_UINT,unsigned int)
ARM(ncbyte,NC_BYTE,signed char,ncint64,NC_INT64,long long)
ARM(ncbyte,NC_BYTE,signed char,ncuint64,NC_UINT64,unsigned long long)
ARM(ncbyte,NC_BYTE,signed char,ncfloat,NC_FLOAT,float)
ARM(ncbyte,NC_BYTE,signed char,ncdouble,NC_DOUBLE,double)
ARM(ncubyte,NC_UBYTE,unsigned char,ncchar,NC_CHAR,char)
ARM(ncubyte,NC_UBYTE,unsigned char,ncbyte,NC_BYTE,signed char)
ARM(ncubyte,NC_UBYTE,unsigned char,ncubyte,NC_UBYTE,unsigned char)
ARM(ncubyte,NC_UBYTE,unsigned char,ncshort,NC_SHORT,short)
ARM(ncubyte,NC_UBYTE,unsigned char,ncushort,NC_USHORT,unsigned short)
ARM(ncubyte,NC_UBYTE,unsigned char,ncint,NC_INT,int)
ARM(ncubyte,NC_UBYTE,unsigned char,ncuint,NC_UINT,unsigned int)
ARM(ncubyte,NC_UBYTE,unsigned char,ncint64,NC_INT64,long long)
ARM(ncubyte,NC_UBYTE,unsigned char,ncuint64,NC_UINT64,unsigned long long)
ARM(ncubyte,NC_UBYTE,unsigned char,ncfloat,NC_FLOAT,float)
ARM(ncubyte,NC_UBYTE,unsigned char,ncdouble,NC_DOUBLE,double)
ARM(ncshort,NC_SHORT,short,ncchar,NC_CHAR,char)
ARM(ncshort,NC_SHORT,short,ncbyte,NC_BYTE,signed char)
ARM(ncshort,NC_SHORT,short,ncubyte,NC_UBYTE,unsigned char)
ARM(ncshort,NC_SHORT,short,ncshort,NC_SHORT,short)
ARM(ncshort,NC_SHORT,short,ncushort,NC_USHORT,unsigned short)
ARM(ncshort,NC_SHORT,short,ncint,NC_INT,int)
ARM(ncshort,NC_SHORT,short,ncuint,NC_UINT,unsigned int)
ARM(ncshort,NC_SHORT,short,ncint64,NC_INT64,long long)
ARM(ncshort,NC_SHORT,short,ncuint64,NC_UINT64,unsigned long long)
ARM(ncshort,NC_SHORT,short,ncfloat,NC_FLOAT,float)
ARM(ncshort,NC_SHORT,short,ncdouble,NC_DOUBLE,double)
ARM(ncushort,NC_USHORT,unsigned short,ncchar,NC_CHAR,char)
ARM(ncushort,NC_USHORT,unsigned short,ncbyte,NC_BYTE,signed char)
ARM(ncushort,NC_USHORT,unsigned short,ncubyte,NC_UBYTE,unsigned char)
ARM(ncushort,NC_USHORT,unsigned short,ncshort,NC_SHORT,short)
ARM(ncushort,NC_USHORT,unsigned short,ncushort,NC_USHORT,unsigned short)
ARM(ncushort,NC_USHORT,unsigned short,ncint,NC_INT,int)
ARM(ncushort,NC_USHORT,unsigned short,ncuint,NC_UINT,unsigned int)
ARM(ncushort,NC_USHORT,unsigned short,ncint64,NC_INT64,long long)
ARM(ncushort,NC_USHORT,unsigned short,ncuint64,NC_UINT64,unsigned long long)
ARM(ncushort,NC_USHORT,unsigned short,ncfloat,NC_FLOAT,float)
ARM(ncushort,NC_USHORT,unsigned short,ncdouble,NC_DOUBLE,double)
ARM(ncint,NC_INT,int,ncchar,NC_CHAR,char)
ARM(ncint,NC_INT,int,ncbyte,NC_BYTE,signed char)
ARM(ncint,NC_INT,int,ncubyte,NC_UBYTE,unsigned char)
ARM(ncint,NC_INT,int,ncshort,NC_SHORT,short)
ARM(ncint,NC_INT,int,ncushort,NC_USHORT,unsigned short)
ARM(ncint,NC_INT,int,ncint,NC_INT,int)
ARM(ncint,NC_INT,int,ncuint,NC_UINT,unsigned int)
ARM(ncint,NC_INT,int,ncint64,NC_INT64,long long)
ARM(ncint,NC_INT,int,ncuint64,NC_UINT64,unsigned long long)
ARM(ncint,NC_INT,int,ncfloat,NC_FLOAT,float)
ARM(ncint,NC_INT,int,ncdouble,NC_DOUBLE,double)
ARM(ncuint,NC_UINT,unsigned int,ncchar,NC_CHAR,char)
ARM(ncuint,NC_UINT,unsigned int,ncbyte,NC_BYTE,signed char)
ARM(ncuint,NC_UINT,unsigned int,ncubyte,NC_UBYTE,unsigned char)
ARM(ncuint,NC_UINT,unsigned int,ncshort,NC_SHORT,short)
ARM(ncuint,NC_UINT,unsigned int,ncushort,NC_USHORT,unsigned short)
ARM(ncuint,NC_UINT,unsigned int,ncint,NC_INT,int)
ARM(ncuint,NC_UINT,unsigned int,ncuint,NC_UINT,unsigned int)
ARM(ncuint,NC_UINT,unsigned int,ncint64,NC_INT64,long long)
ARM(ncuint,NC_UINT,unsigned int,ncuint64,NC_UINT64,unsigned long long)
ARM(ncuint,NC_UINT,unsigned int,ncfloat,NC_FLOAT,float)
ARM(ncuint,NC_UINT,unsigned int,ncdouble,NC_DOUBLE,double)
ARM(ncint64,NC_INT64,long long,ncchar,NC_CHAR,char)
ARM(ncint64,NC_INT64,long long,ncbyte,NC_BYTE,signed char)
ARM(ncint64,NC_INT64,long long,ncubyte,NC_UBYTE,unsigned char)
ARM(ncint64,NC_INT64,long long,ncshort,NC_SHORT,short)
ARM(ncint64,NC_INT64,long long,ncushort,NC_USHORT,unsigned short)
ARM(ncint64,NC_INT64,long long,ncint,NC_INT,int)
ARM(ncint64,NC_INT64,long long,ncuint,NC_UINT,unsigned int)
ARM(ncint64,NC_INT64,long long,ncint64,NC_INT64,long long)
ARM(ncint64,NC_INT64,long long,ncuint64,NC_UINT64,unsigned long long)
ARM(ncint64,NC_INT64,long long,ncfloat,NC_FLOAT,float)
ARM(ncint64,NC_INT64,long long,ncdouble,NC_DOUBLE,double)
ARM(ncuint64,NC_UINT64,unsigned long long,ncchar,NC_CHAR,char)
ARM(ncuint64,NC_UINT64,unsigned long long,ncbyte,NC_BYTE,signed char)
ARM(ncuint64,NC_UINT64,unsigned long long,ncubyte,NC_UBYTE,unsigned char)
ARM(ncuint64,NC_UINT64,unsigned long long,ncshort,NC_SHORT,short)
ARM(ncuint64,NC_UINT64,unsigned long long,ncushort,NC_USHORT,unsigned short)
ARM(ncuint64,NC_UINT64,unsigned long long,ncint,NC_INT,int)
ARM(ncuint64,NC_UINT64,unsigned long long,ncuint,NC_UINT,unsigned int)
ARM(ncuint64,NC_UINT64,unsigned long long,ncint64,NC_INT64,long long)
ARM(ncuint64,NC_UINT64,unsigned long long,ncuint64,NC_UINT64,unsigned long long)
ARM(ncuint64,NC_UINT64,unsigned long long,ncfloat,NC_FLOAT,float)
ARM(ncuint64,NC_UINT64,unsigned long long,ncdouble,NC_DOUBLE,double)
ARM(ncfloat,NC_FLOAT,float,ncchar,NC_CHAR,char)
ARM(ncfloat,NC_FLOAT,float,ncbyte,NC_BYTE,signed char)
ARM(ncfloat,NC_FLOAT,float,ncubyte,NC_UBYTE,unsigned char)
ARM(ncfloat,NC_FLOAT,float,ncshort,NC_SHORT,short)
ARM(ncfloat,NC_FLOAT,float,ncushort,NC_USHORT,unsigned short)
ARM(ncfloat,NC_FLOAT,float,ncint,NC_INT,int)
ARM(ncfloat,NC_FLOAT,float,ncuint,NC_UINT,unsigned int)
ARM(ncfloat,NC_FLOAT,float,ncint64,NC_INT64,long long)
ARM(ncfloat,NC_FLOAT,float,ncuint64,NC_UINT64,unsigned long long)
ARM(ncfloat,NC_FLOAT,float,ncfloat,NC_FLOAT,float)
ARM(ncfloat,NC_FLOAT,float,ncdouble,NC_DOUBLE,double)
ARM(ncdouble,NC_DOUBLE,double,ncchar,NC_CHAR,char)
ARM(ncdouble,NC_DOUBLE,double,ncbyte,NC_BYTE,signed char)
ARM(ncdouble,NC_DOUBLE,double,ncubyte,NC_UBYTE,unsigned char)
ARM(ncdouble,NC_DOUBLE,double,ncshort,NC_SHORT,short)
ARM(ncdouble,NC_DOUBLE,double,ncushort,NC_USHORT,unsigned short)
ARM(ncdouble,NC_DOUBLE,double,ncint,NC_INT,int)
ARM(ncdouble,NC_DOUBLE,double,ncuint,NC_UINT,unsigned int)
ARM(ncdouble,NC_DOUBLE,double,ncint64,NC_INT64,long long)
ARM(ncdouble,NC_DOUBLE,double,ncuint64,NC_UINT64,unsigned long long)
ARM(ncdouble,NC_DOUBLE,double,ncfloat,NC_FLOAT,float)
ARM(ncdouble,NC_DOUBLE,double,ncdouble,NC_DOUBLE,double)
	
        default: ncstat = NC_EINVAL; THROWCHK(ncstat); goto fail;
        }
        value += nctypesizeof(srctype);
        memory += nctypesizeof(dsttype);
    }

fail:
    return THROW(ncstat);
}

NCerror
dapcvtattrval3(nc_type etype, void* dst, NClist* src)
{
    int i,ok;
    NCerror  ncstat = NC_NOERR;
    unsigned int memsize = nctypesizeof(etype);
    unsigned int nvalues = nclistlength(src);
    char* dstmem = (char*)dst;

    for(i=0;i<nvalues;i++) {
	char* s = (char*)nclistget(src,i);
	ok = 0;
	switch (etype) {
	case NC_BYTE: {
	    unsigned char* p = (unsigned char*)dstmem;
	    ok = sscanf(s,"%hhu",p);
	    } break;
	case NC_CHAR: {
	    signed char* p = (signed char*)dstmem;
	    ok = sscanf(s,"%c",p);
	    } break;
	case NC_SHORT: {
	    short* p = (short*)dstmem;
	    ok = sscanf(s,"%hd",p);
	    } break;
	case NC_INT: {
	    int* p = (int*)dstmem;
	    ok = sscanf(s,"%d",p);
	    } break;
	case NC_FLOAT: {
	    float* p = (float*)dstmem;
	    ok = sscanf(s,"%g",p);
	    } break;
	case NC_DOUBLE: {
	    double* p = (double*)dstmem;
	    ok = sscanf(s,"%lg",p);
	    } break;
	case NC_UBYTE: {
	    unsigned char* p = (unsigned char*)dstmem;
	    ok = sscanf(s,"%hhu",p);
	    } break;
	case NC_USHORT: {
	    unsigned short* p = (unsigned short*)dstmem;
	    ok = sscanf(s,"%hu",p);
	    } break;
	case NC_UINT: {
	    unsigned int* p = (unsigned int*)dstmem;
	    ok = sscanf(s,"%u",p);
	    } break;
	case NC_INT64: {
	    long long* p = (long long*)dstmem;
	    ok = sscanf(s,"%lld",p);
	    } break;
	case NC_UINT64: {
	    unsigned long long* p = (unsigned long long*)dstmem;
	    ok = sscanf(s,"%llu",p);
	    } break;
	case NC_STRING: case NC_URL: {
	    char** p = (char**)dstmem;
	    *p = nulldup(s);
	    } break;
	default:
   	    PANIC1("unexpected nc_type: %d",(int)etype);
	}
	if(ok != 1) {ncstat = NC_EINVAL; goto done;}
	dstmem += memsize;
    }
done:
    return THROW(ncstat);
}

