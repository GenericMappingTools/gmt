/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/libncdap3/stubdap3.c,v 1.4 2010/05/24 19:48:14 dmh Exp $ */

#include "config.h"
#include "netcdf.h"

extern int NC3_initialize(void);
extern int NCD3_initialize(void);

int
NC_initialize(void)
{
    NC3_initialize();
    NCD3_initialize();
    return NC_NOERR;
}
