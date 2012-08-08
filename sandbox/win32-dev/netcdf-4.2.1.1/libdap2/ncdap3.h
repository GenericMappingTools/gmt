/*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncdap3/ncdap3.h,v 1.40 2010/05/30 19:45:52 dmh Exp $
  *********************************************************************/
#ifndef NCDAP3_H
#define NCDAP3_H 1

#include "config.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#include "ncbytes.h"
#include "nclist.h"
#include "nchashmap.h"
#include "nclog.h"
#include "dceconstraints.h"

#include "oc.h"
#include "ocuri.h"

#include "nc.h"
#include "netcdf.h"
#include "ncdispatch.h"
 /* netcdf overrides*/
#include "dapnc.h"

#include "dapdebug.h"
#include "daputil.h"

/**************************************************/
/* sigh, do the forwards */
struct NCprojection;
struct NCselection;
struct Getvara;
struct NCcachenode;
struct NCcache;
struct NCslice;
struct NCsegment;

/**************************************************/

#include "nccommon.h"
#include "getvara.h"
#include "constraints3.h"

/**************************************************/

#ifndef USE_NETCDF4
#define	NC_UBYTE 	7	/* unsigned 1 byte int */
#define	NC_USHORT 	8	/* unsigned 2-byte int */
#define	NC_UINT 	9	/* unsigned 4-byte int */
#define	NC_INT64 	10	/* signed 8-byte int */
#define	NC_UINT64 	11	/* unsigned 8-byte int */
#define	NC_STRING 	12	/* string */

#define NC_MAX_BYTE 127
#define NC_MIN_BYTE (-NC_MAX_BYTE-1)
#define NC_MAX_CHAR 255
#define NC_MAX_SHORT 32767
#define NC_MIN_SHORT (-NC_MAX_SHORT - 1)
#define NC_MAX_INT 2147483647
#define NC_MIN_INT (-NC_MAX_INT - 1)
#define NC_MAX_FLOAT 3.402823466e+38f
#define NC_MIN_FLOAT (-NC_MAX_FLOAT)
#define NC_MAX_DOUBLE 1.7976931348623157e+308 
#define NC_MIN_DOUBLE (-NC_MAX_DOUBLE)
#define NC_MAX_UBYTE NC_MAX_CHAR
#define NC_MAX_USHORT 65535U
#define NC_MAX_UINT 4294967295U
#define NC_MAX_INT64 (9223372036854775807LL)
#define NC_MIN_INT64 (-9223372036854775807LL-1)
#define NC_MAX_UINT64 (18446744073709551615ULL)
#define X_INT64_MAX     (9223372036854775807LL)
#define X_INT64_MIN     (-X_INT64_MAX - 1)
#define X_UINT64_MAX    (18446744073709551615ULL)
#endif /*USE_NETCDF4*/


/**************************************************/
/* The NCDAP3 structure is an extension of the NC structure (libsrc/nc.h) */

typedef struct NCDAP3 {
    NC nc; /* Used to store meta-data */
    NCDAPCOMMON dap;
} NCDAP3;

/**************************************************/

extern struct NCTMODEL nctmodels[];

/**************************************************/
/* Import some internal procedures from libsrc*/

#ifdef IGNORE
extern void drno_add_to_NCList(struct NC *ncp);
extern void drno_del_from_NCList(struct NC *ncp);
extern void drno_free_NC(struct NC *ncp);
extern struct NC* drno_new_NC(const size_t *chunkp);
extern void drno_set_numrecs(NC* ncp, size_t size);
extern size_t drno_get_numrecs(NC* ncp);
extern int drno_ncio_open(NC* ncp, const char* path, int mode);
#endif

 /* Internal, but non-static procedures */
extern NCerror computecdfvarnames3(NCDAPCOMMON*,CDFnode*,NClist*);
extern NCerror computecdfnodesets3(NCDAPCOMMON* drno);
extern NCerror computevarnodes3(NCDAPCOMMON*, NClist*, NClist*);
extern NCerror collectvardefdims(NCDAPCOMMON* drno, CDFnode* var, NClist* dimset);
extern NCerror fixgrids3(NCDAPCOMMON* drno);
extern NCerror dapmerge3(NCDAPCOMMON* drno, CDFnode* node, OCobject dasroot);
extern NCerror sequencecheck3(NCDAPCOMMON* drno);
extern NCerror computecdfdimnames3(NCDAPCOMMON*);
extern NCerror attachdatadds3(struct NCDAPCOMMON*);
extern NCerror detachdatadds3(struct NCDAPCOMMON*);
extern void dapdispatch3init(void);

/*
extern void dereference3(NCconstraint* constraint);
extern NCerror rereference3(NCconstraint*, NClist*);
*/

extern NCerror buildvaraprojection3(struct Getvara*,
		     const size_t* startp, const size_t* countp, const ptrdiff_t* stridep,
		     struct DCEprojection** projectionlist);

extern NCerror nc3d_getvarx(int ncid, int varid,
	    const size_t *startp,
	    const size_t *countp,
	    const ptrdiff_t *stridep,
	    void *data,
	    nc_type dsttype0);

/**************************************************/

/* From: ncdap3.c*/
extern NCerror nc3d_open(const char* path, int mode, int* ncidp);
extern int nc3d_close(int ncid);
extern int nc3dinitialize(void);
extern NCerror regrid3(CDFnode* ddsroot, CDFnode* template, NClist*);
extern NCerror imprint3(CDFnode* dstroot, CDFnode* srcroot);
extern void unimprint3(CDFnode* root);
extern NCerror imprintself3(CDFnode* root);
extern void setvisible(CDFnode* root, int visible);

/* From: ncdap3a.c*/
extern NCerror fetchtemplatemetadata3(NCDAPCOMMON* nccomm);
extern NCerror fetchconstrainedmetadata3(NCDAPCOMMON* nccomm);
extern void applyclientparamcontrols3(NCDAPCOMMON*);
extern NCerror suppressunusablevars3(NCDAPCOMMON*);
extern NCerror addstringdims(NCDAP3* drno);
extern NCerror defseqdims(NCDAP3* drno);
extern NCerror fixzerodims3(NCDAPCOMMON*);
extern void estimatevarsizes3(NCDAPCOMMON*);
extern NCerror defrecorddim3(NCDAP3*);
extern NClist* getalldims3(NClist* vars, int visibleonly);
extern NCerror showprojection3(NCDAP3*, CDFnode* var);


/* From: dapcvt.c*/
extern NCerror dapconvert3(nc_type, nc_type, char*, char*, size_t);
extern int dapcvtattrval3(nc_type, void*, NClist*);

#ifdef IGNORE
/* allow access url parse and params without exposing ocuri.h */
extern int NCDAP_urlparse(const char* s, void** dapurl);
extern void NCDAP_urlfree(void* dapurl);
extern const char* NCDAP_urllookup(void* dapurl, const char* param);
#endif

extern size_t dapzerostart3[NC_MAX_VAR_DIMS];
extern size_t dapsinglecount3[NC_MAX_VAR_DIMS];
extern ptrdiff_t dapsinglestride3[NC_MAX_VAR_DIMS];

#endif /*NCDAP3_H*/
