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

/* Provide a universal cast type containing common fields */

struct CDFnode; /* Forward */

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
    struct CDFnode* cdfnode;
} CCEsegment;

typedef struct CCEfcn {
    CCEnode node;
    char* name; 
    NClist* args;
} CCEfcn;

typedef struct CCEvar {
    CCEnode node;
    NClist* segments;
    struct CDFnode* cdfnode;
    struct CDFnode* cdfleaf;
} CCEvar;

typedef struct CCEconstant {
    CCEnode node;
    CEsort discrim;
    char* text;
    long long intvalue;
    double floatvalue;
} CCEconstant;

typedef struct CCEvalue {
    CCEnode node;
    CEsort discrim;
    /* Do not bother with a union */
    CCEconstant* constant;
    CCEvar* var;
    CCEfcn* fcn;
} CCEvalue;

typedef struct CCEselection {
    CCEnode node;
    CEsort operator;
    CCEvalue* lhs;
    NClist* rhs;
} CCEselection;

typedef struct CCEprojection {
    CCEnode node;
    CEsort discrim;
    /* Do not bother with a union */
    CCEvar* var;
    CCEfcn* fcn;
} CCEprojection;

typedef struct CCEconstraint {
    CCEnode node;
    NClist* projections;
    NClist* selections;
} CCEconstraint;


extern int cceparseconstraints(char* constraints, CCEconstraint* CCEonstraint);
extern int cceslicemerge(CCEslice* dst, CCEslice* src);
extern int ccemergeprojections(NClist* dst, NClist* src);

extern char* ccebuildprojectionstring(NClist* projections);
extern char* ccebuildselectionstring(NClist* selections);
extern char* ccebuildconstraintstring(CCEconstraint* constraints);

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

extern int cceiswholesegment(CCEsegment*);
extern int cceiswholeslice(CCEslice*);
extern int cceiswholeseglist(NClist*);
extern int ccesamepath(NClist* list1, NClist* list2);

#endif /*CCECONSTRAINTS_H*/
