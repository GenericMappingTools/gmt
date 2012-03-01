/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCUTIL_H
#define OCUTIL_H 1

/* Forward */
struct OCstate;

extern char* ocstrndup(const char* s, size_t len);

extern size_t octypesize(OCtype etype);
extern char*  octypetostring(OCtype octype);
extern char*  octypetoddsstring(OCtype octype);
extern char* ocerrstring(int err);
extern OCerror ocsvcerrordata(struct OCstate*,char**,char**,long*);
extern OCerror octypeprint(OCtype etype, char* buf, size_t bufsize, void* value);
extern size_t ocxdrsize(OCtype etype);

extern size_t totaldimsize(OCnode*);

extern void makedimlist(OClist* path, OClist* dims);

extern int findbod(OCbytes* buffer, size_t*, size_t*);

extern int xdr_skip(XDR* xdrs, unsigned int len);
extern int xdr_skip_strings(XDR* xdrs, unsigned int n);
extern unsigned int xdr_roundup(unsigned int n);

extern unsigned int ocbyteswap(unsigned int i);

/* Reclaimers*/
extern void freeOCnode(OCnode*,int);
extern void ocfreeprojectionclause(OCprojectionclause* clause);

/* Misc. */
extern void ocdataddsmsg(struct OCstate*, struct OCtree*);

#endif /*UTIL_H*/
