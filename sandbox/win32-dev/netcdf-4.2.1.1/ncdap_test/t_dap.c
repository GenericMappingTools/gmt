#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "netcdf.h"


#undef GENERATE

#undef DEBUG

/* Test (some) internal/external type conversions
   using following DAP dataset (test.02).
	Dataset {
	    Byte b[DIMSIZE];
	    Int32 i32[DIMSIZE];
	    UInt32 ui32[DIMSIZE];
	    Int16 i16[DIMSIZE];
	    UInt16 ui16[DIMSIZE];
	    Float32 f32[DIMSIZE];
	    Float64 f64[DIMSIZE];
	    String s[DIMSIZE];
	    Url u[DIMSIZE];
	} OneDimensionalSimpleArrays;
*/

#define NDIMS 1
#define DIMSIZE 25
#define STRLEN 64

#ifndef USE_NETCDF4
#define	NC_UBYTE 	7	/* unsigned 1 byte int */
#define	NC_USHORT 	8	/* unsigned 2-byte int */
#define	NC_UINT 	9	/* unsigned 4-byte int */
#define	NC_INT64 	10	/* signed 8-byte int */
#define	NC_UINT64 	11	/* unsigned 8-byte int */
#define	NC_STRING 	12	/* string */
#endif


#define CHECK(expr) check(expr,__FILE__,__LINE__);

#define COMMA (i==0?"":",")

#define COMPARE(t1,t2,v1,v2) compare(t1,t2,(void*)v1,(void*)v2,#v2,__FILE__,__LINE__)

static int failure = 0;

static void compare(nc_type,nc_type,void*,void*,char*,char*,int);

static void
report(const int i, const char* var, const int line)
{
    fprintf(stdout,"%s mismatch: [%d] file: %s line: %d\n",var,i,__FILE__,line);
    failure = 1;
}

static void
check(int ncstat, char* file, int line)
{
    if(ncstat == NC_NOERR) return;
    fprintf(stderr,"*** FAIL: %d (%s) at %s:%d\n",
	    ncstat,nc_strerror(ncstat),file,line);
    exit(1);
}

/* return 1 if |f1-f2| > 0.05 */
static int
fdiff(double f1, double f2)
{
    double delta = (f1 - f2);
    if(delta < 0) delta = - delta;
    if(delta > 0.05) {
	fprintf(stdout,"fdiff: %1.3f %1.3f delta=%1.3f\n",f1,f2,delta);
    }
    return (delta > 0.05?1:0);
}

static char ch_data[DIMSIZE];
static signed char int8_data[DIMSIZE];
static unsigned char uint8_data[DIMSIZE];
static int int8toint32_data[DIMSIZE];
static float int82float32_data[DIMSIZE];
static short int16_data[DIMSIZE];
static int int16toint32_data[DIMSIZE];
static float int162float32_data[DIMSIZE];
static int int32_data[DIMSIZE];
static float int32tofloat32_data[DIMSIZE];
static long int32toilong_data[DIMSIZE];
static float float32_data[DIMSIZE];
static double float64_data[DIMSIZE];
#ifndef USE_NETCDF4
static char string3_data[DIMSIZE][STRLEN];
#endif

static char ch[DIMSIZE];
static signed char int8v[DIMSIZE];
static unsigned char uint8v[DIMSIZE];
static short int16v[DIMSIZE];
static int int32v[DIMSIZE];
static float float32v[DIMSIZE];
static double float64v[DIMSIZE];
static long  ilong[DIMSIZE];
#ifndef USE_NETCDF4
static char string3[DIMSIZE][STRLEN];
#endif

int main()
{
    int ncid, varid;
    int ncstat = NC_NOERR;
    char* url;
    char* topsrcdir;
    size_t len;
#ifndef USE_NETCDF4
    int i,j;
#endif

    /* location of our target url: use file:// to avoid remote
	server downtime issues
     */
    
    /* Assume that TESTS_ENVIRONMENT was set */
    topsrcdir = getenv("TOPSRCDIR");
    if(topsrcdir == NULL) {
        fprintf(stderr,"*** FAIL: $abs_top_srcdir not defined: location= %s:%d\n",__FILE__,__LINE__);
        exit(1);
    }    
    len = strlen("file://") + strlen(topsrcdir) + strlen("/ncdap_test/testdata3/test.02") + 1;
#ifdef DEBUG
    len += strlen("[log][show=fetch]");
#endif
    url = (char*)malloc(len);
    url[0] = '\0';

#ifdef DEBUG
    strcat(url,"[log][show=fetch]");
#endif

    strcat(url,"file://");
    strcat(url,topsrcdir);
    strcat(url,"/ncdap_test/testdata3/test.02");

    printf("*** Test: var conversions on URL: %s\n",url);

    /* open file, get varid */
    CHECK(nc_open(url, NC_NOWRITE, &ncid));
    
    /* extract the string case for netcdf-3*/
#ifndef USE_NETCDF4
    CHECK(nc_inq_varid(ncid, "s", &varid));
    CHECK(nc_get_var_text(ncid,varid,(char*)string3));
#ifdef GENERATE
    printf("static %s string3_data[DIMSIZE][STRLEN]={","char");
    for(i=0;i<DIMSIZE;i++) {
	int j;
	/* Do simple escape */
	for(j=0;j<STRLEN;j++) {
	    if(string3[i][j] > 0
	       && string3[i][j] != '\n'
	       && string3[i][j] != '\r'
	       && string3[i][j] != '\t'
	       &&(string3[i][j] < ' ' || string3[i][j] >= '\177'))
		string3[i][j] = '?';
	}
	printf("%s\"%s\"",COMMA,string3[i]);
    }
    printf("};\n");
#else
 	fprintf(stdout,"*** testing: %s\n","string3");
	for(i=0;i<DIMSIZE;i++) {
   	    for(j=0;j<STRLEN;j++) {
	        if(string3[i][j] != string3_data[i][j]) {report(i,"string3",__LINE__); break;}
	    }
	}
#endif
#endif

    CHECK(nc_inq_varid(ncid, "b", &varid));
    CHECK(nc_get_var_text(ncid,varid,ch));
#ifdef GENERATE
    printf("static %s ch_data[DIMSIZE]={","char");
    for(i=0;i<DIMSIZE;i++) printf("%s'\\%03hho'",COMMA,ch[i]);
    printf("};\n");
#else
    COMPARE(NC_CHAR,NC_CHAR,ch,ch_data);
#endif

    CHECK(nc_inq_varid(ncid, "b", &varid));
    CHECK(nc_get_var_schar(ncid,varid,int8v));
#ifdef GENERATE
    printf("static %s int8_data[DIMSIZE]={","signed char");
    for(i=0;i<DIMSIZE;i++) printf("%s%hhd",COMMA,int8v[i]);
    printf("};\n");
#else
    COMPARE(NC_BYTE,NC_BYTE,int8v,int8_data);
#endif

    CHECK(nc_inq_varid(ncid, "b", &varid));
    CHECK(nc_get_var_uchar(ncid,varid,uint8v));
#ifdef GENERATE
    printf("static %s uint8_data[DIMSIZE]={","unsigned char");
    for(i=0;i<DIMSIZE;i++) printf("%s%hhu",COMMA,uint8v[i]);
    printf("};\n");
#else
    COMPARE(NC_UBYTE,NC_UBYTE,uint8v,uint8_data);
#endif

    CHECK(nc_inq_varid(ncid, "b", &varid));
    CHECK(nc_get_var_int(ncid,varid,int32v));
#ifdef GENERATE
    printf("static %s int8toint32_data[DIMSIZE]={","int");
    for(i=0;i<DIMSIZE;i++) printf("%s%d",COMMA,int32v[i]);
    printf("};\n");
#else
    COMPARE(NC_BYTE,NC_INT,int32v,int8toint32_data);
#endif

    CHECK(nc_inq_varid(ncid, "b", &varid));
    CHECK(nc_get_var_float(ncid,varid,float32v));
#ifdef GENERATE
    printf("static %s int82float32_data[DIMSIZE]={","float");
    for(i=0;i<DIMSIZE;i++) printf("%s%1.3f",COMMA,float32v[i]);
    printf("};\n");
#else
    COMPARE(NC_FLOAT,NC_FLOAT,float32v,int82float32_data);
#endif

    CHECK(nc_inq_varid(ncid, "i16", &varid));
    CHECK(nc_get_var_short(ncid,varid,int16v));
#ifdef GENERATE
    printf("static %s int16_data[DIMSIZE]={","short");
    for(i=0;i<DIMSIZE;i++) printf("%s%hd",COMMA,int16v[i]);
    printf("};\n");
#else
    COMPARE(NC_SHORT,NC_SHORT,int16v,int16_data);
#endif

    CHECK(nc_inq_varid(ncid, "i16", &varid));
    CHECK(nc_get_var_int(ncid,varid,int32v));
#ifdef GENERATE
    printf("static %s int16toint32_data[DIMSIZE]={","int");
    for(i=0;i<DIMSIZE;i++) printf("%s%d",COMMA,int32v[i]);
    printf("};\n");
#else
    COMPARE(NC_SHORT,NC_INT,int32v,int16toint32_data);
#endif

    CHECK(nc_inq_varid(ncid, "i16", &varid));
    CHECK(nc_get_var_float(ncid,varid,float32v));
#ifdef GENERATE
    printf("static %s int162float32_data[DIMSIZE]={","float");
    for(i=0;i<DIMSIZE;i++) printf("%s%1.3f",COMMA,float32v[i]);
    printf("};\n");
#else
    COMPARE(NC_SHORT,NC_FLOAT,float32v,int162float32_data);
#endif

    CHECK(nc_inq_varid(ncid, "i32", &varid));
    CHECK(nc_get_var_int(ncid,varid,int32v));
#ifdef GENERATE
    printf("static %s int32_data[DIMSIZE]={","int");
    for(i=0;i<DIMSIZE;i++) printf("%s%d",COMMA,int32v[i]);
    printf("};\n");
#else
    COMPARE(NC_INT,NC_INT,int32v,int32_data);
#endif

    CHECK(nc_inq_varid(ncid, "i32", &varid));
    CHECK(nc_get_var_float(ncid,varid,float32v));
#ifdef GENERATE
    printf("static %s int32tofloat32_data[DIMSIZE]={","float");
    for(i=0;i<DIMSIZE;i++) printf("%s%1.3f",COMMA,float32v[i]);
    printf("};\n");
#else
    COMPARE(NC_INT,NC_FLOAT,float32v,int32tofloat32_data);
#endif

    CHECK(nc_inq_varid(ncid, "i32", &varid));
    CHECK(nc_get_var_long(ncid,varid,ilong));
#ifdef GENERATE
    printf("static %s int32toilong_data[DIMSIZE]={","long");
    for(i=0;i<DIMSIZE;i++) printf("%s%ld",COMMA,ilong[i]);
    printf("};\n");
#else
    COMPARE(NC_INT,NC_NAT,ilong,int32toilong_data);
#endif

    CHECK(nc_inq_varid(ncid, "f32", &varid));
    CHECK(nc_get_var_float(ncid,varid,float32v));
#ifdef GENERATE
    printf("static %s float32_data[DIMSIZE]={","float");
    for(i=0;i<DIMSIZE;i++) printf("%s%1.3f",COMMA,float32v[i]);
    printf("};\n");
#else
    COMPARE(NC_FLOAT,NC_FLOAT,float32v,float32_data);
#endif

    CHECK(nc_inq_varid(ncid, "f64", &varid));
    CHECK(nc_get_var_double(ncid,varid,float64v));
#ifdef GENERATE
    printf("static %s float64_data[DIMSIZE]={","double");
    for(i=0;i<DIMSIZE;i++) printf("%s%1.3f",COMMA,float64v[i]);
    printf("};\n");
#else
    COMPARE(NC_DOUBLE,NC_DOUBLE,float64v,float64_data);
#endif

    if(failure) {
        printf("ncstat=%d %s",ncstat,nc_strerror(ncstat));
        exit(1);
    }
    return 0;
}

static char ch_data[DIMSIZE]={'\000','\001','\002','\003','\004','\005','\006','\007','\010','\011','\012','\013','\014','\015','\016','\017','\020','\021','\022','\023','\024','\025','\026','\027','\030'};
static signed char int8_data[DIMSIZE]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
static unsigned char uint8_data[DIMSIZE]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
static int int8toint32_data[DIMSIZE]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
static float int82float32_data[DIMSIZE]={0.000,1.000,2.000,3.000,4.000,5.000,6.000,7.000,8.000,9.000,10.000,11.000,12.000,13.000,14.000,15.000,16.000,17.000,18.000,19.000,20.000,21.000,22.000,23.000,24.000};
static short int16_data[DIMSIZE]={0,256,512,768,1024,1280,1536,1792,2048,2304,2560,2816,3072,3328,3584,3840,4096,4352,4608,4864,5120,5376,5632,5888,6144};
static int int16toint32_data[DIMSIZE]={0,256,512,768,1024,1280,1536,1792,2048,2304,2560,2816,3072,3328,3584,3840,4096,4352,4608,4864,5120,5376,5632,5888,6144};
static float int162float32_data[DIMSIZE]={0.000,256.000,512.000,768.000,1024.000,1280.000,1536.000,1792.000,2048.000,2304.000,2560.000,2816.000,3072.000,3328.000,3584.000,3840.000,4096.000,4352.000,4608.000,4864.000,5120.000,5376.000,5632.000,5888.000,6144.000};
static int int32_data[DIMSIZE]={0,2048,4096,6144,8192,10240,12288,14336,16384,18432,20480,22528,24576,26624,28672,30720,32768,34816,36864,38912,40960,43008,45056,47104,49152};
static float int32tofloat32_data[DIMSIZE]={0.000,2048.000,4096.000,6144.000,8192.000,10240.000,12288.000,14336.000,16384.000,18432.000,20480.000,22528.000,24576.000,26624.000,28672.000,30720.000,32768.000,34816.000,36864.000,38912.000,40960.000,43008.000,45056.000,47104.000,49152.000};
static long int32toilong_data[DIMSIZE]={0,2048,4096,6144,8192,10240,12288,14336,16384,18432,20480,22528,24576,26624,28672,30720,32768,34816,36864,38912,40960,43008,45056,47104,49152};
static float float32_data[DIMSIZE]={0.000,0.010,0.020,0.030,0.040,0.050,0.060,0.070,0.080,0.090,0.100,0.110,0.120,0.130,0.140,0.149,0.159,0.169,0.179,0.189,0.199,0.208,0.218,0.228,0.238};
static double float64_data[DIMSIZE]={1.000,1.000,1.000,1.000,0.999,0.999,0.998,0.998,0.997,0.996,0.995,0.994,0.993,0.992,0.990,0.989,0.987,0.986,0.984,0.982,0.980,0.978,0.976,0.974,0.971};

#ifndef USE_NETCDF4
static char string3_data[DIMSIZE][STRLEN]={"This is a data test string (pass 0).","This is a data test string (pass 1).","This is a data test string (pass 2).","This is a data test string (pass 3).","This is a data test string (pass 4).","This is a data test string (pass 5).","This is a data test string (pass 6).","This is a data test string (pass 7).","This is a data test string (pass 8).","This is a data test string (pass 9).","This is a data test string (pass 10).","This is a data test string (pass 11).","This is a data test string (pass 12).","This is a data test string (pass 13).","This is a data test string (pass 14).","This is a data test string (pass 15).","This is a data test string (pass 16).","This is a data test string (pass 17).","This is a data test string (pass 18).","This is a data test string (pass 19).","This is a data test string (pass 20).","This is a data test string (pass 21).","This is a data test string (pass 22).","This is a data test string (pass 23).","This is a data test string (pass 24)."};
#endif

static void
compare(nc_type t1, nc_type t2, void* v0, void* vdata0, char* tag,
	char* file, int line)
{
    int i;
    fprintf(stdout,"*** testing: %s\n",tag); \

#ifdef DEBUG
#define test \
    printf("v ="); \
    for(i=0;i<DIMSIZE;i++) {printf(" %llu",(unsigned long long)v[i]);} \
    printf("\n"); \
    printf("vdata ="); \
    for(i=0;i<DIMSIZE;i++) {printf(" %llu",(unsigned long long)vdata[i]);} \
    printf("\n"); \
    for(i=0;i<DIMSIZE;i++) {\
        if(v[i] != vdata[i]) {report(i,tag,line); break;}\
    }
#define ftest \
    printf("v ="); \
    for(i=0;i<DIMSIZE;i++) {printf(" %g",v[i]);} \
    printf("\n"); \
    printf("vdata ="); \
    for(i=0;i<DIMSIZE;i++) {printf(" %g",vdata[i]);} \
    printf("\n"); \
    for(i=0;i<DIMSIZE;i++) {\
        if(fdiff((double)v[i],(double)vdata[i])) {report(i,tag,line); break;}\
    }
#else
#define test for(i=0;i<DIMSIZE;i++) {\
        if(v[i] != vdata[i]) {report(i,tag,line); break;}\
    }
#define ftest for(i=0;i<DIMSIZE;i++) {\
        if(fdiff((double)v[i],(double)vdata[i])) {report(i,tag,line); break;}\
    }
#endif

#define setup(T) T* v = (T*)v0; T* vdata = (T*)vdata0;

#define CASE(nc1,nc2) (nc1*256+nc2)
    switch(CASE(t1,t2)) {

    default: {
	printf("unexpected compare:  %d %d\n",(int)t1,(int)t2);
	abort();
    }    

case CASE(NC_CHAR,NC_CHAR): {
    setup(char);
    test;
} break;
case CASE(NC_BYTE,NC_BYTE): {
    setup(signed char);
    test;
} break;
case CASE(NC_SHORT,NC_SHORT): {
    setup(short);
    test;
} break;
case CASE(NC_INT,NC_INT): {
    setup(int);
    test;
} break;
case CASE(NC_FLOAT,NC_FLOAT): {
    setup(float);
    ftest;
} break;
case CASE(NC_DOUBLE,NC_DOUBLE): {
    setup(double);
    ftest;
} break;


/* Mixed comparisons */
case CASE(NC_BYTE,NC_INT): {
    setup(int);
    test;
} break;
case CASE(NC_SHORT,NC_INT): {
    setup(int);
    test;
} break;
case CASE(NC_SHORT,NC_FLOAT): {
    setup(float);
    ftest;
} break;
case CASE(NC_INT,NC_FLOAT): {
    setup(float);
    ftest;
} break;

/* This is an get_var_long case */
case CASE(NC_INT,NC_NAT): {
    setup(long);
    test;
} break;

case CASE(NC_UBYTE,NC_UBYTE): {
    setup(unsigned char);
    test;
} break;
case CASE(NC_USHORT,NC_USHORT): {
    setup(unsigned short);
    test;
} break;
case CASE(NC_UINT,NC_UINT): {
    setup(unsigned int);
    test;
} break;

/* Mixed cases */
case CASE(NC_INT,NC_INT64): {
    setup(long long);
    test;
} break;

case CASE(NC_INT,NC_UINT64): {
    setup(unsigned long long);
    test;
} break;

case CASE(NC_STRING,NC_STRING):{
    setup(char*);
    for(i=0;i<DIMSIZE;i++) {
	if(strcmp(v[i],vdata[i])!=0) {report(i,tag,line); break;}
    }
} break;

case CASE(NC_CHAR,NC_STRING):{
    setup(char*);
    if(memcmp((void*)v[0],(void*)vdata,DIMSIZE+1)!=0)
        {report(0,tag,line);}
} break;

    } /*switch*/
}

