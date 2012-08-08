/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCINTERNAL_H
#define OCINTERNAL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _AIX
#include <netinet/in.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_RPC_TYPES_H
#include <rpc/types.h>
#endif

#ifdef HAVE_RPC_XDR_H
#include <rpc/xdr.h>
#else
#include <xdr.h>
#endif

#define CURL_DISABLE_TYPECHECK 1
#include <curl/curl.h>

#include "oclist.h"
#include "ocbytes.h"
#include "ocuri.h"

#define OCCACHEPOS
#ifdef OCCACHEPOS
extern void ocxdrstdio_create(XDR*,FILE*,enum xdr_op);
#else
#define ocxdrstdio_create(xdrs,file,op) xdrstdio_create(xdrs,file,op)
#endif

#undef OC_DISK_STORAGE

#include "oc.h"
#include "ocdatatypes.h"
#include "constraints.h"
#include "ocnode.h"
#include "ocutil.h"
#include "oclog.h"
#include "ocdata.h"

#ifndef nulldup
#define nulldup(s) (s==NULL?NULL:strdup(s))
#endif

#define nullstring(s) (s==NULL?"(null)":s)
#define PATHSEPARATOR "."

/* Default initial memory packet size */
#define DFALTPACKETSIZE 0x20000 /*approximately 100k bytes*/

/* Default maximum memory packet size */
#define DFALTMAXPACKETSIZE 0x3000000 /*approximately 50M bytes*/

/* Compile when |datadds| < 1M */
#define OCCOMPILELIMIT (0x100000)

/* Extend the OCdxd type */
#define OCVER 3

/* Define a magic number to mark externally visible oc objects */
#define OCMAGIC ((unsigned int)0x0c0c0c0c) /*clever, huh?*/

/*! Specifies the OCstate. */
typedef struct OCstate
{
    unsigned int magic; /* Mark each structure type */
    CURL* curl; /* curl handle*/
    OClist* trees; /* list<OCnode*> ; all root objects */
    OCURI* uri; /* base URI*/
    OCbytes* packet; /* shared by all trees during construction */
    /* OCContent information */
    struct OCcontent* contentlist;
    struct {/* Hold info for an error return from server */
	char* code;
	char* message;
	long  httpcode;
    } error;
    long ddslastmodified;
    long datalastmodified;
    /* Store .rc file info */
    struct OCcurlflags {
	int compress;
	int verbose;
	int timeout;
	int followlocation;
	int maxredirs;
	char* useragent;
	char* cookiejar;
	char* cookiefile;
    } curlflags;
    struct OCSSL {
	int   validate;
        char* certificate;
	char* key;
	char* keypasswd;
        char* cainfo; /* certificate authority */
	char* capath; 
    } ssl;
    struct OCproxy {
	char *host;
	int port;
    } proxy;
    struct OCcredentials {
	char *username;
	char *password;
    } creds;
} OCstate;

/*! Specifies all the info about a particular DAP tree
    i.e. DAS, DDS, or DATADDS as obtained from a fetch response
    This is associated with the root object.
*/
typedef struct OCtree
{
    OCdxd  dxdclass;
    char* constraint;
    char* text;
    struct OCnode* root; /* cross link */
    struct OCstate* state; /* cross link */
    OClist* nodes; /* all nodes in tree*/
    /* when dxdclass == OCDATADDS */
    struct {
#ifdef OC_DISK_STORAGE
        char* filename;
        FILE* file;
#else
	void* xdrdata;
#endif
        unsigned long datasize; /* size on disk or in memory */
        unsigned long bod;
        unsigned long ddslen;
        XDR* xdrs;
        struct OCmemdata* memdata; /* !NULL iff compiled */
    } data;
} OCtree;

/*
WARNING: this data structures uses the standard C trick of
casting a long memory chunk to be an instance of this
object, which means that the data part may actually be
longer than 8 chars.
*/
typedef struct OCmemdata {
    OCtype octype; /* Actually instance of OCtype, but guaranteed to be |long| */
    OCtype etype; /* Actually instance of OCtype, but guaranteed to be |long| */
    OCmode mode; /* Actually instance of OCmode, but guaranteed to be |long| */
    unsigned long count; /* count*octypesize(datatype) == |data| */
    union {
        struct OCmemdata* mdata[2];
        unsigned int* idata[2];
        char data[8]; /* Actually prefix of the data; want to start on longlong boundary */
	char* sdata;
    } data;
} OCmemdata;


/* (Almost) All shared procedure definitions are kept here
   except for: ocdebug.h ocutil.h
   The true external interfac is defined in oc.h
*/

/* Location: ocnode.c */
/*
extern OCnode* makepseudodimension(size_t size, OCnode* array, int index);
*/
extern OCnode* makeocnode(char* name, OCtype ptype, OCnode* root);
extern void collectpathtonode(OCnode* node, OClist* path);
extern void computeocfullnames(OCnode* root);
extern void computeocsemantics(OClist*);
extern void addattribute(OCattribute* attr, OCnode* parent);
extern OCattribute* makeattribute(char* name, OCtype ptype, OClist* values);
extern size_t ocsetsize(OCnode* node);
extern OCerror occorrelate(OCnode*,OCnode*);

/* Location: dapparselex.c*/
extern int dapdebug;
extern OCerror DAPparse(OCstate*, struct OCtree*, char*);
extern char* dimnameanon(char* basename, unsigned int index);

/* Location: ceparselex.c*/
extern int cedebug;
extern OClist* CEparse(OCstate*,char* input);

/* Location: occompile.c*/
extern int occompile(OCstate* state, OCnode* root);
extern void freeocmemdata(OCmemdata* md);
extern void octempclear(OCstate* state);

/* Location: ocinternal.c*/
extern OCerror ocopen(OCstate** statep, const char* url);
extern void occlose(OCstate* state);

extern OCerror ocfetch(OCstate*, const char*, OCdxd, OCnode**);

/* Location: ocinternal.c */
extern int oc_network_order;
extern int oc_invert_xdr_double;
extern int ocinternalinitialize(void);

/* Location: ocnode.c */
extern void ocfreetree(OCtree* tree);
extern void ocfreeroot(OCnode* root);
extern void ocfreenodes(OClist*);

extern void ocddsclear(struct OCstate*);
extern void ocdasclear(struct OCstate*);
extern void ocdataddsclear(struct OCstate*);
extern void* oclinearize(OCtype etype, unsigned int, char**);

/* Merge DAS with DDS or DATADDS*/
extern int ocddsdasmerge(struct OCstate*, OCnode* das, OCnode* dds);

extern OCerror ocupdatelastmodifieddata(OCstate* state);

extern int ocinternalinitialize(void);

/* Use my own ntohl and htonl */
#define ocntoh(i) (oc_network_order?(i):ocbyteswap((i)))
#define ochton(i) ocntoh(i)

/* Define an inline version of byteswap */
#define swapinline(iswap,i) \
{ \
    unsigned int b0,b1,b2,b3; \
    b0 = (i>>24) & 0x000000ff; \
    b1 = (i>>16) & 0x000000ff; \
    b2 = (i>>8) & 0x000000ff; \
    b3 = (i) & 0x000000ff; \
    iswap = (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24)); \
}

extern OCerror ocsetrcfile(char* rcfile);

/* Global stateflags */
extern int oc_network_order; /* network order is big endian */
extern int oc_invert_xdr_double;
extern int oc_curl_file_supported;
extern int oc_curl_https_supported;

#endif /*COMMON_H*/
