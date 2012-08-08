/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/getvara.h,v 1.11 2010/05/27 21:34:08 dmh Exp $
 *********************************************************************/
#ifndef NCCRGETVARX_H
#define NCCRGETVARX_H

typedef struct NCCRgetvarx {
    CCEprojection* projection;
    nc_type internaltype;
    nc_type externaltype;
    CRnode* target;
    Data* data;
} NCCRgetvarx;

#endif /*NCCRGETVARX_H*/
