/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/data.h,v 1.5 2010/05/24 19:59:56 dmh Exp $ */

#ifndef DATA_H
#define DATA_H 1

/* nmemonic*/
#define TOPLEVEL 1

/* any one possible value*/
typedef union Constvalue {
    struct Datalist* compoundv; /* NC_COMPOUND*/
    char charv;                 /* NC_CHAR*/
    signed char int8v;          /* NC_BYTE*/
    unsigned char uint8v;       /* NC_UBYTE*/
    short int16v;               /* NC_SHORT*/
    unsigned short uint16v;     /* NC_USHORT*/
    int int32v;                 /* NC_INT*/
    unsigned int uint32v;       /* NC_UINT*/
    long long int64v;           /* NC_INT64*/
    unsigned long long uint64v; /* NC_UINT64*/
    float floatv;               /* NC_FLOAT*/
    double doublev;             /* NC_DOUBLE*/
    struct Stringv {		/* NC_STRING*/
	int len;
	char* stringv;
        /*struct Datalist* charlist;*/
    } stringv;
    struct Opaquev {     /* NC_OPAQUE*/
	int len; /* length as originally written (rounded to even number)*/
	char* stringv; /*as  constant was written*/
		      /* (padded to even # chars >= 16)*/
		      /* without leading 0x*/
    } opaquev;
    struct Symbol* enumv;   /* NC_ECONST*/
} Constvalue;

typedef struct Constant {
    nc_type 	  nctype;
    int		  lineno;
    Constvalue    value;
    int           filled; /* was this originally NC_FILLVALUE? */
} Constant;

typedef struct Datalist {
    struct Datalist* next; /* chain of all known datalists*/
    int           readonly; /* data field is shared with another Datalist*/
    size_t  length; /* |data| */
    size_t  alloc;  /* track total allocated space for data field*/
    Constant*     data; /* actual list of constants constituting the datalist*/
    /* Track various values associated with the datalist*/
    /* (used to be in Constvalue.compoundv)*/
    struct Vlen {
        struct Symbol* schema; /* type/var that defines structure of this*/
        unsigned int count; /* # of vlen basetype instances*/
	unsigned int uid;       /* unique id for NC_VLEN*/
    } vlen;
} Datalist;

/* Define a structure to track
   location of current read point in the Datalist sequence
   In effect, we are parsing the data sequence.
   Push and pop of data sources is supported (see srcpush() below).*/

typedef struct Datasrc {
    Constant*    data;     /* duplicate pointer; so do not free.*/
    int index;        
    int length;
    int spliced;           /* Was this list spliced into our parent ? */
    struct Datasrc* prev; /* linked list for debugging */
} Datasrc;

/* Define a holder for passing a start/count array */
struct Vlendata {
    char* data;
    unsigned long count;
};
extern struct Vlendata* vlendata;

/* Convenience*/

#define SRCPUSH(iscmpd,src) {if(((iscmpd)=issublist(src))) {srcpush(src);}}
#define SRCPOP(iscmpd,src) {if((iscmpd)) {srcpop(src);}}

int issublist(Datasrc* src);
int isstring(Datasrc* src);
int isfillvalue(Datasrc* src);
int istype(Datasrc* src, nc_type);
int isstringable(nc_type nctype);

#ifdef ENABLE_BINARY
/* from: cdfdata.c */
void bindata_array(struct Symbol*,Bytebuffer*,Datasrc*,Odometer*,int,Datalist*);
void bindata_attrdata(struct Symbol* asym, Bytebuffer*);
void bindata_vardata(struct Symbol* vsym, Bytebuffer*);
void bindata_vlenconstants(List*);
void bindata_basetype(struct Symbol*,struct Datasrc*,Bytebuffer*,struct Datalist*);
#endif

#ifdef ENABLE_C
/* from: cdata.c */
void cdata_attrdata(struct Symbol* asym, Bytebuffer*);
void cdata_array(struct Symbol*,Bytebuffer*,Datasrc*,Odometer*,int,Datalist*);
void cdata_basetype(struct Symbol*,struct Datasrc*,Bytebuffer*,struct Datalist*);
void cdata_vlenconstants(List*,Bytebuffer*);
char* cdata_const(Constant*);
#endif

#ifdef ENABLE_F77
/* from: f77data.c */
void f77data_attrdata(struct Symbol* asym, Bytebuffer*);
void f77data_array(struct Symbol*,Bytebuffer*,Datasrc*,Odometer*,int,Datalist*);
void f77data_basetype(struct Symbol*,struct Datasrc*,Bytebuffer*,struct Datalist*);
char* f77data_const(Constant* ci);
void f77quotestring(Bytebuffer* databuf);
#endif

#ifdef ENABLE_CML
/* from: cmldata.c */
void gencml_attrdata(struct Symbol* asym, Bytebuffer*);
void gencml_scalardata(struct Symbol* vsym, Bytebuffer*);
void gencml_arraydata(struct Symbol* vsym, Bytebuffer*);
void gencml_vlenconstants(List*, Bytebuffer*);
void gencml_fillvalue(struct Symbol*, Datalist*, Datasrc*, Bytebuffer*);
void xquotestring(Bytebuffer* databuf);
char* xconst(Constant* ci);
#endif

#ifdef ENABLE_JAVA
/* from: jdata.c */
void jdata_array(struct Symbol*,Bytebuffer*,Datasrc*,Odometer*,int,Datalist*);
void jdata_basetype(struct Symbol*,struct Datasrc*,Bytebuffer*,struct Datalist*);
char* jdata_const(Constant* ci);
void jquotestring(Bytebuffer* databuf, char);
#endif

/* from: data.c */
Constant gen_string(unsigned long, Datasrc*);
int stringimplode(Constant* con);
Constant cloneconstant(Constant* con); /* shallow clone*/
Constant gen_stringall(unsigned long size, Datasrc* src, unsigned long);

Constant* emptycompoundconst(int,Constant*);
Constant* emptystringconst(int,Constant*);

Datasrc* datalist2src(Datalist* list);
Datasrc* const2src(Constant*);
Constant list2const(Datalist*);
Datalist* const2list(Constant* con);
void freedatasrc(Datasrc* src);

void srcpush(Datasrc*);
void srcpushlist(Datasrc* src, Datalist* cmpd);
void srcpop(Datasrc*);
void srcmoveto(Datasrc*,size_t);
void srcmove(Datasrc*,size_t);
void srcsetfill(Datasrc* ds, Datalist* list);

Datalist* datalistclone(Datalist* dl);
Datalist* datalistconcat(Datalist* dl1, Datalist* dl2);
Datalist* datalistappend(Datalist* dl, Constant* con);
Datalist* datalistreplace(Datalist* dl, unsigned int index, Constant* con);
int datalistline(Datalist*);

Constant* srcnext(Datasrc*);
int srclast(Datasrc*); /* are we at the last entry ? */
int srcmore(Datasrc*);
int srcline(Datasrc* ds);
void srcsplice(Datasrc* ds, Datalist* list);

void alignbuffer(struct Constant* prim, Bytebuffer* buf);

/* Code dump support procedures */
void bbindent(Bytebuffer*,const int);
void bbprintf(Bytebuffer*,const char *fmt, ...);
void bbprintf0(Bytebuffer*,const char *fmt, ...);
/* Following dump to codebuffer */
void codeprintf(const char *fmt, ...);
void codedump(Bytebuffer*);
void codepartial(const char*);
void codeline(const char*);
void codelined(int n,const char*);
void codeflush(void); /* flush codebuffer to stdout */

void commify(Bytebuffer* buf);
char* word(char* p, Bytebuffer* buf);

/* Provide buffers for language based generators */
extern Bytebuffer* codebuffer; /* buffer over the std output */
extern Bytebuffer* stmt; /* single stmt text generation */

#ifdef FASTDATASRC
#define srcpeek(ds) ((ds)==NULL || (ds)->index >= (ds)->max?NULL:(ds)->data+(ds)->index)
#else
Constant* srcpeek(Datasrc*);
#endif

/* Aliases */
#define srcincr(src) srcnext(src)
#define srcget(src) srcpeek(src)

extern Constant nullconstant;
extern Constant fillconstant;

/* From genchar.c */
void gen_charattr(struct Symbol* asym, Bytebuffer* databuf);
void gen_chararray(struct Symbol* vsym, Bytebuffer* databuf, Datalist* fillsrc);
void gen_charfield(Datasrc* src, Odometer*, Bytebuffer* databuf);
void gen_charvlen(Datasrc*, Bytebuffer*);
int collectstring(struct Constant*, size_t, Bytebuffer*, int);
int getfillchar(Datalist* fillsrc);
int buildcanonicalcharlist(Datalist*, size_t, int, Constant*);
void padstring(Constant* con, size_t desiredlength, int fillchar);

#endif /*DATA_H*/
