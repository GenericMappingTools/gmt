/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/offsets.c,v 1.1 2009/09/25 18:22:40 dmh Exp $
 *********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netcdf.h>

#include <stdint.h>

#include "ncaux.h"
#include "ncaux.c"

static int debug = 0;

#define COMP_ALIGNMENT1(DST,TYPE1,TYPE)  {\
    struct {TYPE1 f1; TYPE x;} tmp; \
    DST.typename = #TYPE ;        \
    DST.alignment = (size_t)((char*)(&(tmp.x)) - (char*)(&tmp));}

#define COMP_ALIGNMENT2(DST,TYPE1,TYPE2,TYPE)  {\
    struct {TYPE1 f1, TYPE2 f2; TYPE x;} tmp;   \
    DST.typename = #TYPE ;                      \
    DST.alignment = (size_t)((char*)(&(tmp.x)) - (char*)(&tmp));}

#define COMP_SIZE0(DST,TYPE1,TYPE2)  {\
    struct {TYPE1 c; TYPE2 x;} tmp; \
    DST = sizeof(tmp); }

/**************************************************/

int baseline_alignment[] = {
0, 1, 1, 2, 2,
4, 4, 4, 4, 8,
8, 4, 8, 4, 4,
};

struct Baseline {
    int size;
    int alignment;
} baseline8[] = {
{0,0}, {2,1}, {2,1}, {4,2}, {4,2},
{8,4}, {8,4}, {8,4}, {8,4}, {16,8},
{16,8}, {8,4}, {16,8}, {8,4}, {12,4},
};

struct Baseline baseline16[] = {
{0,0}, {16,2}, {4,2}, {4,2}, {4,2},
{8,4}, {8,4}, {8,4}, {8,4}, {16,8},
{16,8}, {8,4}, {16,8}, {8,4}, {8,4},
};

struct Baseline baseline32[] = {
{0,0}, {8,4}, {8,4}, {8,4}, {8,4},
{8,4}, {8,4}, {8,4}, {8,4}, {16,8},
{16,8}, {8,4}, {16,8}, {8,4}, {8,4},
};

struct Baseline baseline64[] = {
{0,0}, {16,8}, {16,8}, {16,8}, {16,8},
{16,8}, {16,8}, {16,8}, {16,8}, {16,8},
{16,8}, {16,8}, {16,8}, {16,8}, {16,8},
};

static Typealignvec* vec8;
static Typealignvec* vec16;
static Typealignvec* vec32;
static Typealignvec* vec64;
static int* sizes8;
static int* sizes16;
static int* sizes32;
static int* sizes64;

/**************************************************/

static char*
padname(char* name)
{
#define MAX 20
    if(name == NULL) name = "null";
    int len = strlen(name);
    if(len > MAX) len = MAX;
    char* s = (char*)calloc(1,MAX+1);
    s[MAX+1] = '\0';
    strncpy(s,name,len);
    return s;
}

static void
setup(void)
{
    vec8 = (Typealignvec*)malloc(sizeof(Typealignvec)*NCTYPES);
    vec16 = (Typealignvec*)malloc(sizeof(Typealignvec)*NCTYPES);
    vec32 = (Typealignvec*)malloc(sizeof(Typealignvec)*NCTYPES);
    vec64 = (Typealignvec*)malloc(sizeof(Typealignvec)*NCTYPES);
    sizes8 = (int*)malloc(sizeof(int)*NCTYPES);
    sizes16 = (int*)malloc(sizeof(int)*NCTYPES);
    sizes32 = (int*)malloc(sizeof(int)*NCTYPES);
    sizes64 = (int*)malloc(sizeof(int)*NCTYPES);

    COMP_ALIGNMENT1(vec8[1],char,char)
    COMP_ALIGNMENT1(vec8[2],unsigned char,char)
    COMP_ALIGNMENT1(vec8[3],short,char)
    COMP_ALIGNMENT1(vec8[4],unsigned short,char)
    COMP_ALIGNMENT1(vec8[5],int,char)
    COMP_ALIGNMENT1(vec8[6],unsigned int,char)
    COMP_ALIGNMENT1(vec8[7],long,char)
    COMP_ALIGNMENT1(vec8[8],unsigned long,char)
    COMP_ALIGNMENT1(vec8[9],int64_t,char)
    COMP_ALIGNMENT1(vec8[10],uint64_t,char)
    COMP_ALIGNMENT1(vec8[11],float,char)
    COMP_ALIGNMENT1(vec8[12],double,char)
    COMP_ALIGNMENT1(vec8[13],void*,char)
    COMP_ALIGNMENT1(vec8[14],nc_vlen_t*,char)

    COMP_ALIGNMENT1(vec16[1],char,short);
    COMP_ALIGNMENT1(vec16[2],unsigned char,short);
    COMP_ALIGNMENT1(vec16[3],short,short);
    COMP_ALIGNMENT1(vec16[4],unsigned short,short);
    COMP_ALIGNMENT1(vec16[5],int,short);
    COMP_ALIGNMENT1(vec16[6],unsigned int,short);
    COMP_ALIGNMENT1(vec16[7],long,short);
    COMP_ALIGNMENT1(vec16[8],unsigned long,short);
    COMP_ALIGNMENT1(vec16[9],int64_t,short);
    COMP_ALIGNMENT1(vec16[10],uint64_t,short);
    COMP_ALIGNMENT1(vec16[11],float,short);
    COMP_ALIGNMENT1(vec16[12],double,short);
    COMP_ALIGNMENT1(vec16[13],void*,short);
    COMP_ALIGNMENT1(vec16[14],nc_vlen_t*,short);

    COMP_ALIGNMENT1(vec32[1],char,int);
    COMP_ALIGNMENT1(vec32[2],unsigned char,int);
    COMP_ALIGNMENT1(vec32[3],char,int);
    COMP_ALIGNMENT1(vec32[4],unsigned short,int);
    COMP_ALIGNMENT1(vec32[5],int,int);
    COMP_ALIGNMENT1(vec32[6],unsigned int,int);
    COMP_ALIGNMENT1(vec32[7],long,int);
    COMP_ALIGNMENT1(vec32[8],unsigned long,int);
    COMP_ALIGNMENT1(vec32[9],int64_t,int);
    COMP_ALIGNMENT1(vec32[10],uint64_t,int);
    COMP_ALIGNMENT1(vec32[11],float,int);
    COMP_ALIGNMENT1(vec32[12],double,int);
    COMP_ALIGNMENT1(vec32[13],void*,int);
    COMP_ALIGNMENT1(vec32[14],nc_vlen_t*,int);

    COMP_ALIGNMENT1(vec64[1],char,uint64_t);
    COMP_ALIGNMENT1(vec64[2],unsigned char,uint64_t);
    COMP_ALIGNMENT1(vec64[3],char,uint64_t);
    COMP_ALIGNMENT1(vec64[4],unsigned short,uint64_t);
    COMP_ALIGNMENT1(vec64[5],int,uint64_t);
    COMP_ALIGNMENT1(vec64[6],unsigned int,uint64_t);
    COMP_ALIGNMENT1(vec64[7],long,uint64_t);
    COMP_ALIGNMENT1(vec64[8],unsigned long,uint64_t);
    COMP_ALIGNMENT1(vec64[9],uint64_t,uint64_t);
    COMP_ALIGNMENT1(vec64[10],uint64_t,uint64_t);
    COMP_ALIGNMENT1(vec64[11],float,uint64_t);
    COMP_ALIGNMENT1(vec64[12],double,uint64_t);
    COMP_ALIGNMENT1(vec64[13],void*,uint64_t);
    COMP_ALIGNMENT1(vec64[14],nc_vlen_t*,uint64_t);

    COMP_SIZE0(sizes8[1],char,char);
    COMP_SIZE0(sizes8[2],unsigned char,char);
    COMP_SIZE0(sizes8[3],short,char);
    COMP_SIZE0(sizes8[4],unsigned short,char);
    COMP_SIZE0(sizes8[5],int,char);
    COMP_SIZE0(sizes8[6],unsigned int,char);
    COMP_SIZE0(sizes8[7],long,char);
    COMP_SIZE0(sizes8[8],unsigned long,char);
    COMP_SIZE0(sizes8[9],uint64_t,char);
    COMP_SIZE0(sizes8[10],uint64_t,char);
    COMP_SIZE0(sizes8[11],float,char);
    COMP_SIZE0(sizes8[12],double,char) ;
    COMP_SIZE0(sizes8[13],void*,char);
    COMP_SIZE0(sizes8[14],nc_vlen_t,char);

    COMP_SIZE0(sizes16[1],char,uint64_t);
    COMP_SIZE0(sizes16[2],unsigned char,short);
    COMP_SIZE0(sizes16[3],short,short);
    COMP_SIZE0(sizes16[4],unsigned short,short);
    COMP_SIZE0(sizes16[5],int,short);
    COMP_SIZE0(sizes16[6],unsigned int,short);
    COMP_SIZE0(sizes16[7],long,short);
    COMP_SIZE0(sizes16[8],unsigned long,short);
    COMP_SIZE0(sizes16[9],uint64_t,short);
    COMP_SIZE0(sizes16[10],uint64_t,short);
    COMP_SIZE0(sizes16[11],float,short);
    COMP_SIZE0(sizes16[12],double,short) ;
    COMP_SIZE0(sizes16[13],void*,short);
    COMP_SIZE0(sizes16[14],nc_vlen_t*,short);

    COMP_SIZE0(sizes32[1],char,int);
    COMP_SIZE0(sizes32[2],unsigned char,int);
    COMP_SIZE0(sizes32[3],short,int);
    COMP_SIZE0(sizes32[4],unsigned short,int);
    COMP_SIZE0(sizes32[5],int,int);
    COMP_SIZE0(sizes32[6],unsigned int,int);
    COMP_SIZE0(sizes32[7],long,int);
    COMP_SIZE0(sizes32[8],unsigned long,int);
    COMP_SIZE0(sizes32[9],uint64_t,int);
    COMP_SIZE0(sizes32[10],uint64_t,int);
    COMP_SIZE0(sizes32[11],float,int);
    COMP_SIZE0(sizes32[12],double,int) ;
    COMP_SIZE0(sizes32[13],void*,int);
    COMP_SIZE0(sizes32[14],nc_vlen_t*,int);

    COMP_SIZE0(sizes64[1],char,uint64_t);
    COMP_SIZE0(sizes64[2],unsigned char,uint64_t);
    COMP_SIZE0(sizes64[3],short,uint64_t);
    COMP_SIZE0(sizes64[4],unsigned short,uint64_t);
    COMP_SIZE0(sizes64[5],int,uint64_t);
    COMP_SIZE0(sizes64[6],unsigned int,uint64_t);
    COMP_SIZE0(sizes64[7],long,uint64_t);
    COMP_SIZE0(sizes64[8],unsigned long,uint64_t);
    COMP_SIZE0(sizes64[9],uint64_t,uint64_t);
    COMP_SIZE0(sizes64[10],uint64_t,uint64_t);
    COMP_SIZE0(sizes64[11],float,uint64_t);
    COMP_SIZE0(sizes64[12],double,uint64_t) ;
    COMP_SIZE0(sizes64[13],void*,uint64_t);
    COMP_SIZE0(sizes64[14],nc_vlen_t*,uint64_t);
}

static void
report(void)
{
    int i;

    printf("Type alignments:\n");
    for(i=0;i<NCTYPES;i++) {
	printf("%s: alignment=%2d\n",
		padname(vec[i].typename),vec[i].alignment);
    }

    printf("\n");
    printf("Paired struct sizes and alignment:\n");

    printf("\n");
    printf("int8 vs other types\n");
    for(i=0;i<NCTYPES;i++) {
	printf("char vs %s: size=%2d  alignment=%2d\n",
		padname(vec[i].typename),sizes8[i],vec8[i].alignment);
    }
    printf("\n");
    printf("int16 vs other types\n");
    for(i=0;i<NCTYPES;i++) {
	printf("short vs %s: size=%2d  alignment=%2d\n",
		padname(vec[i].typename),sizes16[i],vec16[i].alignment);
    }
    printf("\n");
    printf("int32 vs other types\n");
    for(i=0;i<NCTYPES;i++) {
	printf("int vs %s: size=%2d  alignment=%2d\n",
		padname(vec[i].typename),sizes32[i],vec32[i].alignment);
    }
    printf("\n");
    printf("int64 vs other types\n");
    for(i=0;i<NCTYPES;i++) {
	printf("long long vs %s: size=%2d  alignment=%2d\n",
		padname(vec[i].typename),sizes64[i],vec64[i].alignment);
    }

}

static int
verify(void)
{
    int i;
    for(i=0;i<NCTYPES;i++) {
	if(vec[i].alignment != baseline_alignment[i]) {
	    fprintf(stderr,"*** FAIL: verify alignment: %s\n",vec[i].typename);
	    goto fail;
	}
    }

    for(i=0;i<NCTYPES;i++) {
	if(sizes8[i] != baseline8[i].size) {
	    fprintf(stderr,"*** FAIL: verify size: uint8 vs %s\n",vec8[i].typename);
	    goto fail;
	}
    }

    for(i=0;i<NCTYPES;i++) {
	if(sizes16[i] != baseline16[i].size) {
	    fprintf(stderr,"*** FAIL: verify size: uint16 vs %s\n",vec16[i].typename);
	    goto fail;
	}
    }

    for(i=0;i<NCTYPES;i++) {
	if(sizes32[i] != baseline32[i].size) {
	    fprintf(stderr,"*** FAIL: verify size: uint32 vs %s\n",vec32[i].typename);
	    goto fail;
	}
    }

    for(i=0;i<NCTYPES;i++) {
	if(sizes64[i] != baseline64[i].size) {
	    fprintf(stderr,"*** FAIL: verify size: uint64 vs %s\n",vec64[i].typename);
	    goto fail;
	}
    }

    return 1;

fail:
    return 0;

}

/* Create a struct using the ncaux functions and check the alignment */
static int
test1(void)
{
    int status = NC_NOERR;
    void* tag;
    int tid, ncid;

    status = nc_create("./tmp.nc",NC_CLOBBER|NC_NETCDF4,&ncid);
    if(status) goto fail;
    status = ncaux_begin_compound(ncid,"test",NCAUX_ALIGN_C,&tag);
    if(status) goto fail;
    status = ncaux_add_field(tag,"f1", NC_INT,0,NULL);
    if(status) goto fail;
    status = ncaux_end_compound(tag,&tid);
    if(status) goto fail;

    return status;

fail:
    fprintf(stderr,"***FAIL: test1: (%d) %s\n",status,nc_strerror(status));
    return status;
}


int
main(int argc, char** argv)
{
    compute_alignments();

    setup();
    if(debug) report();
    if(!verify()) goto fail;

    fprintf(stderr,"***PASS: testaux\n");

    exit(0);

fail:
    exit(1);
}
