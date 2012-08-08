/*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *********************************************************************/
/* $Header$ */

#ifndef CCECONSTRAINTS_H
#define CCECONSTRAINTS_H 1

#include "ceconstraints.h"

#ifndef NC_MAX_VAR_DIMS
#define NC_MAX_VAR_DIMS 1024
#endif

struct CRpath; /* Forward */

/* Provide a universal cast type containing common fields */
typedef struct CCEnode {
    CEsort sort;    
} CCEnode;

typedef struct CCEslice {
    CCEnode node;
    size_t first;
    size_t count;
    size_t length; /* count*stride */
    size_t stride;
    size_t stop; /* == first + count*/
    size_t declsize;  /* from defining dimension, if any.*/
} CCEslice;

typedef struct CCEsegment {
    CCEnode node;
    char* name; 
    int slicesdefined; /*1=>slice counts defined, except declsize*/
    int slicesdeclized; /*1=>slice declsize defined */
    size_t rank;
    CCEslice slices[NC_MAX_VAR_DIMS];    
    struct CRnode* decl;
} CCEsegment;

typedef struct CCEprojection {
    CCEnode node;
    NClist* segments;
    struct CRnode* decl; /* maps to matching stream node */
} CCEprojection;

typedef struct CCEconstraint {
    CCEnode node;
    NClist* projections;
} CCEconstraint;


extern int cdmparseconstraint(char* constraints, CCEconstraint*);
extern int cceslicemerge(CCEslice* target, size_t first, size_t length, ptrdiff_t stride);

extern char* ccebuildprojectionstring(NClist* projections);

extern CCEnode* cceclone(CCEnode* node);
extern NClist* cceclonelist(NClist* list);

extern void ccefree(CCEnode* node);
extern void ccefreelist(NClist* list);

extern char* ccetostring(CCEnode* node);
extern char* ccelisttostring(NClist* list,char*);
extern void ccetobuffer(CCEnode* node, NCbytes* buf);
extern void ccelisttobuffer(NClist* list, NCbytes* buf,char*);

extern NClist* cceallnodes(CCEnode* node, CEsort which);

extern CCEnode* ccecreate(CEsort sort);

extern void ccemakewholeslice(CCEslice* slice, size_t declsize);
extern void ccemakewholesegment(CCEsegment* seg, CRnode* node);

extern int cceiswholesegment(CCEsegment*);
extern int cceiswholeslice(CCEslice*);
extern int cceiswholeseglist(NClist*);
extern int ccesamepath(NClist* list1, NClist* list2);
extern CRpath* crsegment2path(NClist* segments);

extern int pathmatch(NClist* segments, struct CRpath* path);

extern int ccerestrictprojection(CCEprojection*,size_t,const size_t*,const size_t*,const ptrdiff_t*);

#endif /*CCECONSTRAINTS_H*/

