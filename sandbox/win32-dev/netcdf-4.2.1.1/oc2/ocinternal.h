/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCINTERNAL_H
#define OCINTERNAL_H

#include "config.h"

#ifdef _AIX
#include <netinet/in.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <stdarg.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#define CURL_DISABLE_TYPECHECK 1
#include <curl/curl.h>

#include "oclist.h"
#include "ocbytes.h"
#include "ocuri.h"

#define OCCACHEPOS

/* Forwards */
typedef struct OCstate OCstate;
typedef struct OCnode OCnode;
typedef struct OCdata OCdata;
struct OCTriplestore;

/* Define the internal node classification values */
#define OC_None  0
#define OC_State 1
#define OC_Node  2
#define OC_Data  3

/* Define a magic number to mark externally visible oc objects */
#define OCMAGIC ((unsigned int)0x0c0c0c0c) /*clever, huh*/

/* Define a struct that all oc objects must start with */
/* OCheader must be defined here to make it available in other headers */
typedef struct OCheader {
    unsigned int magic;
    unsigned int occlass;
} OCheader;

#include "oc.h"
#include "ocx.h"
#include "ocnode.h"
#include "ocdata.h"
#include "occonstraints.h"
#include "ocutil.h"
#include "oclog.h"
#include "xxdr.h"
#include "ocdatatypes.h"
#include "occompile.h"

#ifndef nulldup
#define nulldup(s) (s==NULL?NULL:strdup(s))
#endif

/*
 * Macros for dealing with flag bits.
 */
#define fset(t,f)       ((t) |= (f))
#define fclr(t,f)       ((t) &= ~(f))
#define fisset(t,f)     ((t) & (f))

#define nullstring(s) (s==NULL?"(null)":s)
#define PATHSEPARATOR "."

/* Define infinity for memory size */
#if SIZEOF_SIZE_T == 4 
#define OCINFINITY ((size_t)0xffffffff)
#else
#define OCINFINITY ((size_t)0xffffffffffffffff)
#endif

/* Extend the OCdxd type for internal use */
#define OCVER 3

/* Default initial memory packet size */
#define DFALTPACKETSIZE 0x20000 /*approximately 100k bytes*/

/* Default maximum memory packet size */
#define DFALTMAXPACKETSIZE 0x3000000 /*approximately 50M bytes*/

/* Collect global state info in one place */
extern struct OCGLOBALSTATE {
    int initialized;
    struct {
        int proto_file;
        int proto_https;
    } curl;
    struct OCTriplestore* ocdodsrc; /* the .dodsrc triple store */
} ocglobalstate;

/*! Specifies the OCstate = non-opaque version of OClink */
struct OCstate {
    OCheader header; /* class=OC_State */
    OClist* trees; /* list<OCNODE*> ; all root objects */
    OCURI* uri; /* base URI*/
    OCbytes* packet; /* shared by all trees during construction */
    struct OCerrdata {/* Hold info for an error return from server */
	char* code;
	char* message;
	long  httpcode;
	char  curlerrorbuf[CURL_ERROR_SIZE]; /* to get curl error message */
    } error;
    CURL* curl; /* curl handle*/
    char curlerror[CURL_ERROR_SIZE];
    struct OCcurlflags {
        int proto_file;
        int proto_https;
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
	int   verifypeer;
    } ssl;
    struct OCproxy {
	char *host;
	int port;
    } proxy;
    struct OCcredentials {
	char *username;
	char *password;
    } creds;
    long ddslastmodified;
    long datalastmodified;
};

/*! OCtree holds extra state info about trees */

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
	char*   memory;   /* allocated memory if OC_ONDISK is not set */
        char*   filename; /* If OC_ONDISK is set */
        FILE*   file;
        off_t   datasize; /* xdr size on disk or in memory */
        off_t   bod;      /* offset of the beginning of packet data */
        off_t   ddslen;   /* length of ddslen (assert(ddslen <= bod)) */
        XXDR*   xdrs;		/* access either memory or file */
        OCdata* data;
    } data;
} OCtree;

/* (Almost) All shared procedure definitions are kept here
   except for: ocdebug.h ocutil.h
   The true external interface is defined in oc.h
*/

#if 0
/* Location: ceparselex.c*/
extern int cedebug;
extern OClist* CEparse(OCstate*,char* input);
#endif

extern OCerror ocopen(OCstate** statep, const char* url);
extern void occlose(OCstate* state);
extern OCerror ocfetch(OCstate*, const char*, OCdxd, OCflags, OCnode**);
extern int oc_network_order;
extern int oc_invert_xdr_double;
extern int ocinternalinitialize(void);

extern OCerror ocupdatelastmodifieddata(OCstate* state);

extern int ocinternalinitialize(void);

#endif /*COMMON_H*/
