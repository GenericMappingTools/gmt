/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "config.h"
#include "ncdispatch.h"

extern int NC3_initialize(void);

#ifdef USE_NETCDF4
extern int NC4_initialize(void);
#endif

#ifdef USE_DAP
extern int NCD3_initialize(void);
#ifdef USE_NETCDF4
extern int NCD4_initialize(void);
#endif
#endif

#ifdef USE_CDMREMOTE
extern int NCCR_initialize(void);
#endif

#ifdef BUILD_RPC
extern int NCRPC_initialize(void);
#endif

int
NC_initialize(void)
{
    int stat = NC_NOERR;

    /* Allow libdispatch to do initialization */
    if((stat = NCDISPATCH_initialize())) return stat;

    /* Initialize each active protocol */

    if((stat = NC3_initialize())) return stat;

#ifdef USE_DAP
    if((stat = NCD3_initialize())) return stat;
#endif

#ifdef USE_NETCDF4
    if((stat = NC4_initialize())) return stat;

    /* if((stat = NCD_initialize())) return stat; */

#ifdef USE_DAP
#ifdef NOTUSED
    if((stat = NCD4_initialize())) return stat;
#endif
#endif

#ifdef USE_CDMREMOTE
    if((stat = NCCR_initialize())) return stat;
#endif

#ifdef USE_RPC
    if((stat = NCRPC_initialize())) return stat;
#endif

#endif /* USE_NETCDF4 */


    return NC_NOERR;
}


