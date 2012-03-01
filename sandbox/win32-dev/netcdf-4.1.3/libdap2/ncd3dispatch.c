/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/ncd3dispatch.c,v 1.7 2010/05/27 21:34:09 dmh Exp $
 *********************************************************************/

#include "config.h"
#include <stdlib.h>
#include <string.h>

#include "nc.h"
#include "ncdap3.h"
#include "ncdispatch.h"
#include "ncd3dispatch.h"

static int
NCD3_create(const char *path, int cmode,
           size_t initialsz, int basepe, size_t *chunksizehintp,
	   int use_parallel, void* mpidata,
           NC_Dispatch*,NC** ncp);

static int NCD3_redef(int ncid);
static int NCD3__enddef(int ncid, size_t h_minfree, size_t v_align, size_t v_minfree, size_t r_align);
static int NCD3_sync(int ncid);
static int NCD3_abort(int ncid);

static int NCD3_put_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges0,
            const void *value0,
	    nc_type memtype);

static int NCD3_get_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            void *value,
	    nc_type memtype);

static int NCD3_put_vars(int ncid, int varid,
	    const size_t *start, const size_t *edges, const ptrdiff_t* stride,
            const void *value0, nc_type memtype);

static int NCD3_get_vars(int ncid, int varid,
	    const size_t *start, const size_t *edges, const ptrdiff_t* stride,
            void *value, nc_type memtype);

ptrdiff_t dapsinglestride3[NC_MAX_VAR_DIMS];
size_t dapzerostart3[NC_MAX_VAR_DIMS];
size_t dapsinglecount3[NC_MAX_VAR_DIMS];

NC_Dispatch NCD3_dispatch_base = {

NC_DISPATCH_NC3 | NC_DISPATCH_NCD,

NCD3_new_nc,

NCD3_create,
NCD3_open,

NCD3_redef,
NCD3__enddef,
NCD3_sync,
NCD3_abort,
NCD3_close,
NULL, /*set_fill*/
NULL, /*inq_base_pe*/
NULL, /*set_base_pe*/
NULL, /*inq_format*/

NULL, /*inq*/
NULL, /*inq_type*/

NULL, /*def_dim*/
NULL, /*inq_dimid*/
NULL, /*inq_dim*/
NULL, /*inq_unlimdim*/
NULL, /*rename_dim*/

NULL, /*inq_att*/
NULL, /*inq_attid*/
NULL, /*inq_attname*/
NULL, /*rename_att*/
NULL, /*del_att*/
NULL, /*get_att*/
NULL, /*put_att*/

NULL, /*def_var*/
NULL, /*inq_varid*/
NULL, /*rename_var*/
NCD3_get_vara,
NCD3_put_vara,
NCD3_get_vars,
NCD3_put_vars,
NULL, /*get_varm*/
NULL, /*put_varm*/

NULL, /*inq_var_all*/

#ifdef USE_NETCDF4
NULL, /*show_metadata*/
NULL, /*inq_unlimdims*/
NULL, /*var_par_access*/
NULL, /*inq_ncid*/
NULL, /*inq_grps*/
NULL, /*inq_grpname*/
NULL, /*inq_grpname_full*/
NULL, /*inq_grp_parent*/
NULL, /*inq_grp_full_ncid*/
NULL, /*inq_varids*/
NULL, /*inq_dimids*/
NULL, /*inq_typeids*/
NULL, /*inq_type_equal*/
NULL, /*def_grp*/
NULL, /*inq_user_type*/
NULL, /*inq_typeid*/

NULL, /*def_compound*/
NULL, /*insert_compound*/
NULL, /*insert_array_compound*/
NULL, /*inq_compound_field*/
NULL, /*inq_compound_fieldindex*/
NULL, /*def_vlen*/
NULL, /*put_vlen_element*/
NULL, /*get_vlen_element*/
NULL, /*def_enum*/
NULL, /*insert_enum*/
NULL, /*inq_enum_member*/
NULL, /*inq_enum_ident*/
NULL, /*def_opaque*/
NULL, /*def_var_deflate*/
NULL, /*def_var_fletcher32*/
NULL, /*def_var_chunking*/
NULL, /*def_var_fill*/
NULL, /*def_var_endian*/
NULL, /*set_var_chunk_cache*/
NULL, /*get_var_chunk_cache*/

#endif /*USE_NETCDF4*/

};

NC_Dispatch NCD3_dispatcher;

int
NCD3_initialize(void)
{
    int i;
    /* Create our dispatch table as the merge of NC3 table
       plus some overrides */
    NC_dispatch_overlay(&NCD3_dispatch_base, NC3_dispatch_table, &NCD3_dispatcher);    
    NCD3_dispatch_table = &NCD3_dispatcher;
    for(i=0;i<NC_MAX_VAR_DIMS;i++)
	{dapzerostart3[i] = 0; dapsinglecount3[i] = 1; dapsinglestride3[i] = 1;}
    return NC_NOERR;
}

static int
NCD3_redef(int ncid)
{
    return (NC_EPERM);
}

static int
NCD3__enddef(int ncid, size_t h_minfree, size_t v_align, size_t v_minfree, size_t r_align)
{
    return (NC_EPERM);
}

static int
NCD3_sync(int ncid)
{
    return (NC_EINVAL);
}

static int
NCD3_abort(int ncid)
{
    return (NC_NOERR);
}

static int
NCD3_create(const char *path, int cmode,
           size_t initialsz, int basepe, size_t *chunksizehintp,
	   int use_parallel, void* mpidata,
           NC_Dispatch* dispatch, NC** ncp)
{
   return NC_EPERM;
}

static int
NCD3_put_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            const void *value,
	    nc_type memtype)
{
    return NC_EPERM;
}

static int
NCD3_get_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            void *value,
	    nc_type memtype)
{
    int stat = nc3d_getvarx(ncid, varid, start, edges, dapsinglestride3,value,memtype);
    return stat;
}

static int
NCD3_put_vars(int ncid, int varid,
	    const size_t *start, const size_t *edges, const ptrdiff_t* stride,
            const void *value0, nc_type memtype)
{
    return NC_EPERM;
}

static int
NCD3_get_vars(int ncid, int varid,
	    const size_t *start, const size_t *edges, const ptrdiff_t* stride,
            void *value, nc_type memtype)
{
    int stat = nc3d_getvarx(ncid, varid, start, edges, stride, value, memtype);
    return stat;
}
