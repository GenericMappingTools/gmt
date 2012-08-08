/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/dapcvt.c,v 1.14 2009/11/29 00:16:26 dmh Exp $
 *********************************************************************/
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include "netcdf.h"
#include "nccrcvt.h"


/*
Inputs:
    1. pointer to input data (value0)
    2. pointer to output data (memory0)
    3. type of the input data (srctype)
    4. size of srctype (srcsize)
    5. size of dsttype (dstsize)
    6. type of the output data (dsttype)
    7. # of items to convert (count)
    8. flag to force byte order swapping (byteswap)
outputs:
    1. error code    
semantics:
    1. copy an instance of the srctype
       from value0 to tmp
    2. if byteswapping, then swap tmp
    3. copy tmp to an instance of srctype
    4. copy srctype instance to memory0
    5. bump the memory and value pointers
    6. repeat count times

assumptions:
    1. both types are fixed size

*/

int
nccrconvert(nc_type srctype, nc_type dsttype,
            void* value0, void* memory0,
            size_t count, int byteswap)
{
    size_t i;
    char* memory = memory0;
    char* value = value0;
    unsigned char tmp[8];
    size_t srcsize = nccrtypelen(srctype);
    size_t dstsize = nccrtypelen(dsttype);

    /* Provide space and pointer casts for intermediate results */
    signed char ncbyte;
    char ncchar;
    short ncshort;
    int ncint;
    float ncfloat;
    double ncdouble;
    unsigned char ncubyte;
    unsigned short ncushort;
    unsigned int ncuint;
    long long ncint64;
    unsigned long long ncuint64;

    signed char* ncbytep = (signed char*)tmp;
    char* nccharp = (char*)tmp;
    short* ncshortp = (short*)tmp;
    int* ncintp = (int*)tmp;
    float* ncfloatp = (float*)tmp;
    double* ncdoublep = (double*)tmp;
    unsigned char* ncubytep = (unsigned char*)tmp;
    unsigned short* ncushortp = (unsigned short*)tmp;
    unsigned int* ncuintp = (unsigned int*)tmp;
    long long* ncint64p = (long long*)tmp;
    unsigned long long* ncuint64p = (unsigned long long*)tmp;

#define CASE(nc1,nc2) (nc1*256+nc2)
#define ARM(vs,ts,vd,td) \
    vs = *vs##p;\
    vd##p = (td *)memory;\
    *vd##p = (td)vs;

    /* Do special cases when srctype == dsttype or both types have size 1*/
    if(srcsize == 1 && dstsize == 1) {
	 memcpy(memory,value,count); /* No swapping or conversion needed */
    } else {
        for(i=0;i<count;i++) {
            /* Stored src item into tmp; optionally swap */
            switch (srcsize) {
            case 2:
		if(byteswap) {
		    tmp[1] = value[0]; tmp[0] = value[1];
		} else {
		    tmp[1] = value[1]; tmp[0] = value[0];
		}
                break;
            case 4:
		if(byteswap) {
		    tmp[3] = value[0]; tmp[2] = value[1];
                    tmp[1] = value[2]; tmp[0] = value[3];
		} else {
		    tmp[0] = value[0]; tmp[1] = value[1];
                    tmp[2] = value[2]; tmp[3] = value[3];
		}
		break;
            case 8:
		if(byteswap) {
		    tmp[7] = value[0]; tmp[6] = value[1];
                    tmp[5] = value[2]; tmp[4] = value[3];
                    tmp[3] = value[4]; tmp[2] = value[5];
                    tmp[1] = value[6]; tmp[0] = value[7];
		} else {
		    tmp[0] = value[0]; tmp[1] = value[1];
                    tmp[2] = value[2]; tmp[3] = value[3];
                    tmp[4] = value[4]; tmp[5] = value[5];
                    tmp[6] = value[6]; tmp[7] = value[7];
		}
	        break;
            default: abort();
            }

	    if(srctype == dsttype) { /* Just move tmp to *memory */
                /*assert(srcsize == dstsize)*/
                switch (srcsize) {
                case 2:
                        memory[0] = tmp[0]; memory[1] = tmp[1];
                        break;
                case 4:
                        memory[0] = tmp[0]; memory[1] = tmp[1];
                        memory[2] = tmp[2]; memory[3] = tmp[3];
                        break;
                case 8:
                        memory[0] = tmp[0]; memory[1] = tmp[1];
                        memory[2] = tmp[2]; memory[3] = tmp[3];
                        memory[4] = tmp[4]; memory[5] = tmp[5];
                        memory[6] = tmp[6]; memory[7] = tmp[7];
                        break;
                default: abort();
                }
            } else {/* srctype != dsttype => We must do the conversion */
		switch (CASE(srctype,dsttype)) {

                case CASE(NC_CHAR,NC_SHORT):
	            ARM(ncchar,char,ncshort,short)
	            break;
                case CASE(NC_CHAR,NC_USHORT):
	            ARM(ncchar,char,ncushort,unsigned short)
	            break;
                case CASE(NC_CHAR,NC_INT):
	            ARM(ncchar,char,ncint,int)
	            break;
                case CASE(NC_CHAR,NC_UINT):
	            ARM(ncchar,char,ncuint,unsigned int)
	            break;
                case CASE(NC_CHAR,NC_INT64):
	            ARM(ncchar,char,ncint64,long long)
	            break;
                case CASE(NC_CHAR,NC_UINT64):
	            ARM(ncchar,char,ncuint64,unsigned long long)
	            break;
                case CASE(NC_CHAR,NC_FLOAT):
	            ARM(ncchar,char,ncfloat,float)
	            break;
                case CASE(NC_CHAR,NC_DOUBLE):
	            ARM(ncchar,char,ncdouble,double)
	            break;
                case CASE(NC_BYTE,NC_SHORT):
	            ARM(ncbyte,signed char,ncshort,short)
	            break;
                case CASE(NC_BYTE,NC_USHORT):
	            ARM(ncbyte,signed char,ncushort,unsigned short)
	            break;
                case CASE(NC_BYTE,NC_INT):
	            ARM(ncbyte,signed char,ncint,int)
	            break;
                case CASE(NC_BYTE,NC_UINT):
	            ARM(ncbyte,signed char,ncuint,unsigned int)
	            break;
                case CASE(NC_BYTE,NC_INT64):
	            ARM(ncbyte,signed char,ncint64,long long)
	            break;
                case CASE(NC_BYTE,NC_UINT64):
	            ARM(ncbyte,signed char,ncuint64,unsigned long long)
	            break;
                case CASE(NC_BYTE,NC_FLOAT):
	            ARM(ncbyte,signed char,ncfloat,float)
	            break;
                case CASE(NC_BYTE,NC_DOUBLE):
	            ARM(ncbyte,signed char,ncdouble,double)
	            break;
                case CASE(NC_UBYTE,NC_SHORT):
	            ARM(ncubyte,unsigned char,ncshort,short)
	            break;
                case CASE(NC_UBYTE,NC_USHORT):
	            ARM(ncubyte,unsigned char,ncushort,unsigned short)
	            break;
                case CASE(NC_UBYTE,NC_INT):
	            ARM(ncubyte,unsigned char,ncint,int)
	            break;
                case CASE(NC_UBYTE,NC_UINT):
	            ARM(ncubyte,unsigned char,ncuint,unsigned int)
	            break;
                case CASE(NC_UBYTE,NC_INT64):
	            ARM(ncubyte,unsigned char,ncint64,long long)
	            break;
                case CASE(NC_UBYTE,NC_UINT64):
	            ARM(ncubyte,unsigned char,ncuint64,unsigned long long)
	            break;
                case CASE(NC_UBYTE,NC_FLOAT):
	            ARM(ncubyte,unsigned char,ncfloat,float)
	            break;
                case CASE(NC_UBYTE,NC_DOUBLE):
	            ARM(ncubyte,unsigned char,ncdouble,double)
	            break;
                case CASE(NC_SHORT,NC_CHAR):
	            ARM(ncshort,short,ncchar,char)
	            break;
                case CASE(NC_SHORT,NC_BYTE):
	            ARM(ncshort,short,ncbyte,signed char)
	            break;
                case CASE(NC_SHORT,NC_UBYTE):
	            ARM(ncshort,short,ncubyte,unsigned char)
	            break;
                case CASE(NC_SHORT,NC_USHORT):
	            ARM(ncshort,short,ncushort,unsigned short)
	            break;
                case CASE(NC_SHORT,NC_INT):
	            ARM(ncshort,short,ncint,int)
	            break;
                case CASE(NC_SHORT,NC_UINT):
	            ARM(ncshort,short,ncuint,unsigned int)
	            break;
                case CASE(NC_SHORT,NC_INT64):
	            ARM(ncshort,short,ncint64,long long)
	            break;
                case CASE(NC_SHORT,NC_UINT64):
	            ARM(ncshort,short,ncuint64,unsigned long long)
	            break;
                case CASE(NC_SHORT,NC_FLOAT):
	            ARM(ncshort,short,ncfloat,float)
	            break;
                case CASE(NC_SHORT,NC_DOUBLE):
	            ARM(ncshort,short,ncdouble,double)
	            break;
                case CASE(NC_USHORT,NC_CHAR):
	            ARM(ncushort,unsigned short,ncchar,char)
	            break;
                case CASE(NC_USHORT,NC_BYTE):
	            ARM(ncushort,unsigned short,ncbyte,signed char)
	            break;
                case CASE(NC_USHORT,NC_UBYTE):
	            ARM(ncushort,unsigned short,ncubyte,unsigned char)
	            break;
                case CASE(NC_USHORT,NC_SHORT):
	            ARM(ncushort,unsigned short,ncshort,short)
	            break;
                case CASE(NC_USHORT,NC_INT):
	            ARM(ncushort,unsigned short,ncint,int)
	            break;
                case CASE(NC_USHORT,NC_UINT):
	            ARM(ncushort,unsigned short,ncuint,unsigned int)
	            break;
                case CASE(NC_USHORT,NC_INT64):
	            ARM(ncushort,unsigned short,ncint64,long long)
	            break;
                case CASE(NC_USHORT,NC_UINT64):
	            ARM(ncushort,unsigned short,ncuint64,unsigned long long)
	            break;
                case CASE(NC_USHORT,NC_FLOAT):
	            ARM(ncushort,unsigned short,ncfloat,float)
	            break;
                case CASE(NC_USHORT,NC_DOUBLE):
	            ARM(ncushort,unsigned short,ncdouble,double)
	            break;
                case CASE(NC_INT,NC_CHAR):
	            ARM(ncint,int,ncchar,char)
	            break;
                case CASE(NC_INT,NC_BYTE):
	            ARM(ncint,int,ncbyte,signed char)
	            break;
                case CASE(NC_INT,NC_UBYTE):
	            ARM(ncint,int,ncubyte,unsigned char)
	            break;
                case CASE(NC_INT,NC_SHORT):
	            ARM(ncint,int,ncshort,short)
	            break;
                case CASE(NC_INT,NC_USHORT):
	            ARM(ncint,int,ncushort,unsigned short)
	            break;
                case CASE(NC_INT,NC_UINT):
	            ARM(ncint,int,ncuint,unsigned int)
	            break;
                case CASE(NC_INT,NC_INT64):
	            ARM(ncint,int,ncint64,long long)
	            break;
                case CASE(NC_INT,NC_UINT64):
	            ARM(ncint,int,ncuint64,unsigned long long)
	            break;
                case CASE(NC_INT,NC_FLOAT):
	            ARM(ncint,int,ncfloat,float)
	            break;
                case CASE(NC_INT,NC_DOUBLE):
	            ARM(ncint,int,ncdouble,double)
	            break;
                case CASE(NC_UINT,NC_CHAR):
	            ARM(ncuint,unsigned int,ncchar,char)
	            break;
                case CASE(NC_UINT,NC_BYTE):
	            ARM(ncuint,unsigned int,ncbyte,signed char)
	            break;
                case CASE(NC_UINT,NC_UBYTE):
	            ARM(ncuint,unsigned int,ncubyte,unsigned char)
	            break;
                case CASE(NC_UINT,NC_SHORT):
	            ARM(ncuint,unsigned int,ncshort,short)
	            break;
                case CASE(NC_UINT,NC_USHORT):
	            ARM(ncuint,unsigned int,ncushort,unsigned short)
	            break;
                case CASE(NC_UINT,NC_INT):
	            ARM(ncuint,unsigned int,ncint,int)
	            break;
                case CASE(NC_UINT,NC_INT64):
	            ARM(ncuint,unsigned int,ncint64,long long)
	            break;
                case CASE(NC_UINT,NC_UINT64):
	            ARM(ncuint,unsigned int,ncuint64,unsigned long long)
	            break;
                case CASE(NC_UINT,NC_FLOAT):
	            ARM(ncuint,unsigned int,ncfloat,float)
	            break;
                case CASE(NC_UINT,NC_DOUBLE):
	            ARM(ncuint,unsigned int,ncdouble,double)
	            break;
                case CASE(NC_INT64,NC_CHAR):
	            ARM(ncint64,long long,ncchar,char)
	            break;
                case CASE(NC_INT64,NC_BYTE):
	            ARM(ncint64,long long,ncbyte,signed char)
	            break;
                case CASE(NC_INT64,NC_UBYTE):
	            ARM(ncint64,long long,ncubyte,unsigned char)
	            break;
                case CASE(NC_INT64,NC_SHORT):
	            ARM(ncint64,long long,ncshort,short)
	            break;
                case CASE(NC_INT64,NC_USHORT):
	            ARM(ncint64,long long,ncushort,unsigned short)
	            break;
                case CASE(NC_INT64,NC_INT):
	            ARM(ncint64,long long,ncint,int)
	            break;
                case CASE(NC_INT64,NC_UINT):
	            ARM(ncint64,long long,ncuint,unsigned int)
	            break;
                case CASE(NC_INT64,NC_UINT64):
	            ARM(ncint64,long long,ncuint64,unsigned long long)
	            break;
                case CASE(NC_INT64,NC_FLOAT):
	            ARM(ncint64,long long,ncfloat,float)
	            break;
                case CASE(NC_INT64,NC_DOUBLE):
	            ARM(ncint64,long long,ncdouble,double)
	            break;
                case CASE(NC_UINT64,NC_CHAR):
	            ARM(ncuint64,long long,ncchar,char)
	            break;
                case CASE(NC_UINT64,NC_BYTE):
	            ARM(ncuint64,long long,ncbyte,signed char)
	            break;
                case CASE(NC_UINT64,NC_UBYTE):
	            ARM(ncuint64,long long,ncubyte,unsigned char)
	            break;
                case CASE(NC_UINT64,NC_SHORT):
	            ARM(ncuint64,long long,ncshort,short)
	            break;
                case CASE(NC_UINT64,NC_USHORT):
	            ARM(ncuint64,long long,ncushort,unsigned short)
	            break;
                case CASE(NC_UINT64,NC_INT):
	            ARM(ncuint64,long long,ncint,int)
	            break;
                case CASE(NC_UINT64,NC_UINT):
	            ARM(ncuint64,long long,ncuint,unsigned int)
	            break;
                case CASE(NC_UINT64,NC_FLOAT):
	            ARM(ncuint64,long long,ncfloat,float)
	            break;
                case CASE(NC_UINT64,NC_DOUBLE):
	            ARM(ncuint64,long long,ncdouble,double)
	            break;
                case CASE(NC_FLOAT,NC_CHAR):
	            ARM(ncfloat,float,ncchar,char)
	            break;
                case CASE(NC_FLOAT,NC_BYTE):
	            ARM(ncfloat,float,ncbyte,signed char)
	            break;
                case CASE(NC_FLOAT,NC_UBYTE):
	            ARM(ncfloat,float,ncubyte,unsigned char)
	            break;
                case CASE(NC_FLOAT,NC_SHORT):
	            ARM(ncfloat,float,ncshort,short)
	            break;
                case CASE(NC_FLOAT,NC_USHORT):
	            ARM(ncfloat,float,ncushort,unsigned short)
	            break;
                case CASE(NC_FLOAT,NC_INT):
	            ARM(ncfloat,float,ncint,int)
	            break;
                case CASE(NC_FLOAT,NC_UINT):
	            ARM(ncfloat,float,ncuint,unsigned int)
	            break;
                case CASE(NC_FLOAT,NC_INT64):
	            ARM(ncfloat,float,ncint64,long long)
	            break;
                case CASE(NC_FLOAT,NC_UINT64):
	            ARM(ncfloat,float,ncuint64,unsigned long long)
	            break;
                case CASE(NC_FLOAT,NC_DOUBLE):
	            ARM(ncfloat,float,ncdouble,double)
	            break;
                case CASE(NC_DOUBLE,NC_CHAR):
	            ARM(ncdouble,double,ncchar,char)
	            break;
                case CASE(NC_DOUBLE,NC_BYTE):
	            ARM(ncdouble,double,ncbyte,signed char)
	            break;
                case CASE(NC_DOUBLE,NC_UBYTE):
	            ARM(ncdouble,double,ncubyte,unsigned char)
	            break;
                case CASE(NC_DOUBLE,NC_SHORT):
	            ARM(ncdouble,double,ncshort,short)
	            break;
                case CASE(NC_DOUBLE,NC_USHORT):
	            ARM(ncdouble,double,ncushort,unsigned short)
	            break;
                case CASE(NC_DOUBLE,NC_INT):
	            ARM(ncdouble,double,ncint,int)
	            break;
                case CASE(NC_DOUBLE,NC_UINT):
	            ARM(ncdouble,double,ncuint,unsigned int)
	            break;
                case CASE(NC_DOUBLE,NC_INT64):
	            ARM(ncdouble,double,ncint64,long long)
	            break;
                case CASE(NC_DOUBLE,NC_UINT64):
	            ARM(ncdouble,double,ncuint64,unsigned long long)
	            break;
                case CASE(NC_DOUBLE,NC_FLOAT):
	            ARM(ncdouble,double,ncfloat,float)
	            break;

	        default: return NC_EINVAL;
	        }
	    }
	    value += srcsize;
	    memory += dstsize;
	}
    }
    return NC_NOERR;
}

/*
 *  This is how much space is required by the user, as in
 *
 *   vals = malloc(nel * nctypelen(var.type));
 *   ncvarget(cdfid, varid, cor, edg, vals);
 */
int
nccrtypelen(nc_type type) 
{
   switch(type){
      case NC_CHAR :
	 return((int)sizeof(char));
      case NC_BYTE :
	 return((int)sizeof(signed char));
      case NC_SHORT :
	 return(int)(sizeof(short));
      case NC_INT :
	 return((int)sizeof(int));
      case NC_FLOAT :
	 return((int)sizeof(float));
      case NC_DOUBLE : 
	 return((int)sizeof(double));

	 /* These can occur in netcdf-3 code */ 
      case NC_UBYTE :
	 return((int)sizeof(unsigned char));
      case NC_USHORT :
	 return((int)(sizeof(unsigned short)));
      case NC_UINT :
	 return((int)sizeof(unsigned int));
      case NC_INT64 :
	 return((int)sizeof(signed long long));
      case NC_UINT64 :
	 return((int)sizeof(unsigned long long));
#ifdef USE_NETCDF4
      case NC_STRING :
	 return((int)sizeof(char*));
#endif /*USE_NETCDF4*/

      default:
	 return -1;
   }
}

