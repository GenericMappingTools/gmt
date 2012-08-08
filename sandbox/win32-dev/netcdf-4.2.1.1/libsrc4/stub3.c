#include "config.h"
#include "netcdf.h"
#include "ncdispatch.h"

extern NC_Dispatch* NC3_dispatch_table;

#ifdef USE_DAP
NC_Dispatch NC3DAP_dispatch_table = NULL;
#endif

#ifdef USE_NETCDF4

NC_Dispatch NC4_dispatch_table = NULL;

#ifdef USE_DAP
NC_Dispatch NC4DAP_dispatch_table = NULL
#endif
#endif

NC_Dispatch*
NC_getdefaultdispatch(void)
{
    return NC3_dispatch_table;
}
