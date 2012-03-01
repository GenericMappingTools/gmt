/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/daputil.h,v 1.25 2010/05/05 22:15:16 dmh Exp $
 *********************************************************************/
#ifndef DAPUTIL_H
#define DAPUTIL_H 1

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

/* mnemonic */
#define WITHDATASET 1
#define WITHOUTDATASET 0
extern void collectnodepath3(struct CDFnode*, NClist* path, int dataset);
extern char* makecdfpathstring3(struct CDFnode*,const char*);
extern char* makesimplepathstring3(struct CDFnode*);
extern char* simplepathstring3(NClist*,char*);
extern void clonenodenamepath3(struct CDFnode*, NClist*, int);

extern char* cdflegalname3(char* dapname);

/* Given a param string; check for a given substring */
extern int paramcheck34(struct NCDAPCOMMON* drno, const char* param, const char* substring);

extern int nclistconcat(NClist* l1, NClist* l2);
extern int nclistminus(NClist* l1, NClist* l2);
extern int nclistdeleteall(NClist* l1, ncelem);

extern char* makeocpathstring3(OCconnection,OCobject,const char*);
extern int collectocpath(OCconnection,OCobject,NClist*);

extern char* getvaraprint(void* gv);

extern int dapinsequence(struct CDFnode* node);
extern int daptopgrid(struct CDFnode* node);
extern int daptopseq(struct CDFnode* node);
extern int daptoplevel(struct CDFnode* node);
extern int dapgridmap(struct CDFnode* node);
extern int dapgridarray(struct CDFnode* node);
extern int dapgridelement(struct CDFnode* node);

#ifdef IGNORE
/* Provide alternate path to the url parameters;
   one that does not require that an OCconnection exist */
extern NClist* dapparamdecode(char*);
extern void dapparamfree(NClist*);
extern const char* dapparamlookup(NClist*, const char*);
#endif

extern unsigned int modeldecode(int, const char*, const struct NCTMODEL*, unsigned int);
extern unsigned long getlimitnumber(const char* limit);

extern void dapexpandescapes(char *termstring);

/* Only used by libncdap4 */
extern int alignbuffer3(NCbytes*, int alignment);
extern size_t dimproduct3(NClist* dimensions);

extern int nc__testurl(const char* path, char** basename);

/* Provide a wrapper for oc_fetch so we can log what it does */
extern OCerror dap_oc_fetch(struct NCDAPCOMMON*,OCconnection,const char*,OCdxd,OCobject*);

extern int dap_badname(char* name);

#endif /*DAPUTIL_H*/
