/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/daputil.h,v 1.25 2010/05/05 22:15:16 dmh Exp $
 *********************************************************************/
#ifndef DAPUTIL_H
#define DAPUTIL_H 1

/* Define a set of flags to control path construction */
#define PATHNC    1 /*Use ->ncname*/
#define PATHELIDE 2 /*Leave out elided nodes*/

/* mnemonic */
#define WITHDATASET 1
#define WITHOUTDATASET 0

/* sigh!, Forwards */
struct CDFnode;
struct NCTMODEL;
struct NCDAPCOMMON;

extern nc_type nctypeconvert(struct NCDAPCOMMON*, nc_type);
extern nc_type octypetonc(OCtype);
extern OCtype nctypetodap(nc_type);
extern size_t nctypesizeof(nc_type);
extern char* nctypetostring(nc_type);
extern char* maketmppath(char* path, char* prefix);

extern void collectnodepath3(struct CDFnode*, NClist* path, int dataset);
extern void collectocpath(OClink conn, OCobject node, NClist* path);

extern char* makecdfpathstring3(struct CDFnode*,const char*);
extern void clonenodenamepath3(struct CDFnode*, NClist*, int);
extern char* makepathstring3(NClist* path, const char* separator, int flags);

extern char* makeocpathstring3(OClink, OCobject, const char*);

extern char* cdflegalname3(char* dapname);

/* Given a param string; return its value or null if not found*/
extern const char* paramvalue34(struct NCDAPCOMMON* drno, const char* param);
/* Given a param string; check for a given substring */
extern int paramcheck34(struct NCDAPCOMMON* drno, const char* param, const char* substring);

extern int nclistconcat(NClist* l1, NClist* l2);
extern int nclistminus(NClist* l1, NClist* l2);
extern int nclistdeleteall(NClist* l1, ncelem);

extern char* getvaraprint(void* gv);

extern int dapinsequence(struct CDFnode* node);
extern int daptopgrid(struct CDFnode* node);
extern int daptopseq(struct CDFnode* node);
extern int daptoplevel(struct CDFnode* node);
extern int dapgridmap(struct CDFnode* node);
extern int dapgridarray(struct CDFnode* node);
extern int dapgridelement(struct CDFnode* node);

extern unsigned int modeldecode(int, const char*, const struct NCTMODEL*, unsigned int);
extern unsigned long getlimitnumber(const char* limit);

extern void dapexpandescapes(char *termstring);

/* Only used by libncdap4 */
extern int alignbuffer3(NCbytes*, int alignment);
extern size_t dimproduct3(NClist* dimensions);

#if defined(DLL_NETCDF)
# if defined(DLL_EXPORT)
#  define NCC_EXTRA __declspec(dllexport)
#else
#  define NCC_EXTRA __declspec(dllimport)
# endif
NCC_EXTRA extern int nc__testurl(const char* path, char** basename);
#else
extern int nc__testurl(const char* parth, char** basename);
#endif


/* Provide a wrapper for oc_fetch so we can log what it does */
extern OCerror dap_fetch(struct NCDAPCOMMON*,OClink,const char*,OCdxd,OCobject*);

extern int dap_badname(char* name);
extern char* dap_repairname(char* name);

#endif /*DAPUTIL_H*/
