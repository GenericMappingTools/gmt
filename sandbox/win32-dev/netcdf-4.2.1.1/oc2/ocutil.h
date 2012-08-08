/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCUTIL_H
#define OCUTIL_H 1

/* Forward */
struct OCstate;

#define ocmax(x,y) ((x) > (y) ? (x) : (y))

extern char* ocstrndup(const char* s, size_t len);
extern int ocstrncmp(const char* s1, const char* s2, size_t len);

extern size_t octypesize(OCtype etype);
extern char*  octypetostring(OCtype octype);
extern char*  octypetoddsstring(OCtype octype);
extern char* ocerrstring(int err);
extern OCerror ocsvcerrordata(struct OCstate*,char**,char**,long*);
extern OCerror octypeprint(OCtype etype, void* value, size_t bufsize, char* buf);
extern size_t xxdrsize(OCtype etype);

extern int oc_ispacked(OCnode* node);

extern size_t octotaldimsize(size_t,size_t*);

extern size_t ocarrayoffset(size_t rank, size_t*, size_t*);
extern void ocarrayindices(size_t index, int rank, size_t*, size_t*);
extern size_t ocedgeoffset(size_t rank, size_t*, size_t*);

extern int ocvalidateindices(size_t rank, size_t*, size_t*);

extern void ocmakedimlist(OClist* path, OClist* dims);

extern int ocfindbod(OCbytes* buffer, size_t*, size_t*);

/* Reclaimers*/
extern void freeOCnode(OCnode*,int);
extern void ocfreeprojectionclause(OCprojectionclause* clause);

/* Misc. */
extern void ocdataddsmsg(struct OCstate*, struct OCtree*);

extern const char* ocdtmodestring(OCDT mode,int compact);

/* Define some classifiers */
#define iscontainer(t) ((t) == OC_Dataset || (t) == OC_Structure || (t) == OC_Sequence || (t) == OC_Grid)

#define isatomic(t) ((t) == OC_Atomic)

#endif /*UTIL_H*/
