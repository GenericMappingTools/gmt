#ifndef NCGEN_UTIL_H
#define NCGEN_UTIL_H
/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/util.h,v 1.3 2010/04/04 19:39:57 dmh Exp $
 *********************************************************************/

#define MAX(x,y) ((x)>(y)?(x):(y))


extern void expe2d(char*);
extern int pow2(int);
extern void tztrim(char*);
extern unsigned int chartohex(char c);

extern void reclaimvardata(List*);
extern void reclaimattptrs(void*, long);
extern void cleanup(void);
extern char* fullname(Symbol*);

extern int isunlimited0(Dimset*);
extern int hasunlimited(Dimset* dimset);
extern int classicunlimited(Dimset* dimset);
extern int isbounded(Dimset* dimset);
extern char* nctypename(nc_type);
extern char* ncclassname(nc_class);
extern int ncsize(nc_type);

/* We have several versions of primitive testing*/
extern int isclassicprim(nc_type); /* a classic primitive type*/
extern int isclassicprimplus(nc_type); /* classic + String*/
extern int isprim(nc_type); /* a netcdf4 primitive type*/
extern int isprimplus(nc_type); /* a netcdf4 primitive type + OPAQUE + ENUM*/

extern void collectpath(Symbol* grp, List* grpstack);
extern List* prefixdup(List*);
extern int prefixeq(List*,List*);
#define prefixlen(sequence) (listlength(sequence))

extern char* poolalloc(size_t);
extern char* pooldup(char*);
extern char* poolcat(const char* s1, const char* s2);

/* compute the total n-dimensional size as 1 long array;
   if stop == 0, then stop = dimset->ndims.
*/
extern size_t crossproduct(Dimset* dimset, int start, int stop);
extern int findunlimited(Dimset* dimset, int start);
extern int findlastunlimited(Dimset* dimset);

extern unsigned char* makebytestring(char* s, size_t* lenp);
extern int getpadding(int offset, int alignment);

extern Datalist* builddatalist(int initialize);
extern void dlappend(Datalist*, Constant*);
extern Constant builddatasublist(Datalist* dl);
extern void dlextend(Datalist* dl);
extern void dlsetalloc(Datalist* dl, size_t newalloc);
extern void check_err(const int stat, const int line, const char* file);

#endif /*NCGEN_UTIL_H*/
