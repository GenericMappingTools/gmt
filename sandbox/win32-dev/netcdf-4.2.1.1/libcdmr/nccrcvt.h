/*********************************************************************


 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#ifndef NCCRCVT_H
#define NCCRCVT_H

extern int nccrtypelen(nc_type);

extern int nccrconvert(nc_type srctype, nc_type dsttype,
                       void* value0, void* memory0,
                       size_t count, int byteswap);

#endif /*NCCRCVT_H*/
