/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "config.h"
#include "netcdf.h"

extern int NC3_initialize(void);
extern int NC4_initialize(void);

int
NC_initialize(void)
{
    NC3_initialize();
    NC4_initialize();
    return NC_NOERR;
}
