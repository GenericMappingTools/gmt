/*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncconstraints/ncconstraints.h,v 1.40 2010/05/30 19:45:52 dmh Exp $
  *********************************************************************/
/*$Id$*/

#ifndef _CRCE_H_
#define _CRCE_H_ 1

/*
Store the relevant parameters for accessing
data for a particular variable
Break up the startp, countp, stridep into slices
to facilitate the odometer walk
*/

typedef struct CRCslice {
    size_t start;
    size_t stop; /* == first + count; aka "end"*/
    size_t stride;
    size_t count; 
    size_t length; /* count*stride */
    size_t declsize;  /* from defining dimension, if any.*/
} CRCslice;


typedef struct CRCsegment {
    char* name;
    size_t slicerank;
    CRCslice slices[NC_MAX_VAR_DIMS];        
} CRCsegment;

typedef struct CRCprojection {
    NClist* segments;
} CRCprojection;


typedef struct CRCerror {
    int lineno;
    int charno;
    char* errmsg;
} CRCerror;


extern int crce(char* src, NClist* projections, CRCerror* err);

#endif /*_CRCE_H_*/
