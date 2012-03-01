/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/nchashmap.h,v 1.4 2009/09/23 22:26:08 dmh Exp $
 *********************************************************************/
#ifndef NCHASHMAP_H
#define NCHASHMAP_H 1

#include "nclist.h"

/* Define the type of the elements in the hashmap*/

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__)
#define EXTERNC extern "C"
#else
#define EXTERNC extern
#endif

typedef unsigned long nchashid;

EXTERNC int nchashnull(ncelem);

typedef struct NChashmap {
  int alloc;
  int size; /* # of pairs still in table*/
  NClist** table;
} NChashmap;

EXTERNC NChashmap* nchashnew(void);
EXTERNC NChashmap* nchashnew0(int);
EXTERNC int nchashfree(NChashmap*);

/* Insert a (ncnchashid,ncelem) pair into the table*/
/* Fail if already there*/
EXTERNC int nchashinsert(NChashmap*, nchashid nchash, ncelem value);

/* Insert a (nchashid,ncelem) pair into the table*/
/* Overwrite if already there*/
EXTERNC int nchashreplace(NChashmap*, nchashid nchash, ncelem value);

/* lookup a nchashid and return found/notfound*/
EXTERNC int nchashlookup(NChashmap*, nchashid nchash, ncelem* valuep);

/* lookup a nchashid and return 0 or the value*/
EXTERNC ncelem nchashget(NChashmap*, nchashid nchash);

/* remove a nchashid*/
EXTERNC int nchashremove(NChashmap*, nchashid nchash);

/* Return the ith pair; order is completely arbitrary*/
/* Can be expensive*/
EXTERNC int nchashith(NChashmap*, int i, nchashid*, ncelem*);

EXTERNC int nchashkeys(NChashmap* hm, nchashid** keylist);

/* return the # of pairs in table*/
#define nchashsize(hm) ((hm)?(hm)->size:0)

#endif /*NCHASHMAP_H*/

