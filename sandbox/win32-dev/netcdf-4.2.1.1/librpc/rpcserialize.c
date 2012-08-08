/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/common34.c,v 1.29 2010/05/25 13:53:02 ed Exp $
 *********************************************************************/

#include "rpc_includes.h"


int
rpc_serialize(MetaNode* node, NCBytes* buf)
{
    switch(node->nodeclass) {
     case NC_VAR:
	




    case NC_FIELD:
    case NC_GROUP:
    case NC_ECONST:
    case NC_VLEN:
    case NC_OPAQUE:
    case NC_ENUM:
    case NC_COMPOUND:
    default: return 0;
    }
}
