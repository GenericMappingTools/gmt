/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap4/ncd4dispatch.c,v 1.8 2010/05/27 21:34:10 dmh Exp $
 *********************************************************************/

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "ncdap4.h"
#include "nc.h"
#include "ncd4dispatch.h"
#include "ncdispatch.h"

static int
NCD4_create(const char *path, int cmode,
           size_t initialsz, int basepe, size_t *chunksizehintp,
	   int use_parallel, void* mpidata,
           NC_Dispatch*,NC** ncp);

static int NCD4_redef(int ncid);
static int NCD4__enddef(int ncid, size_t h_minfree, size_t v_align, size_t v_minfree, size_t r_align);
static int NCD4_put_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges0,
            const void *value0,
	    nc_type memtype);

extern ptrdiff_t dapsinglestride3[NC_MAX_VAR_DIMS];
extern size_t dapzerostart3[NC_MAX_VAR_DIMS];
extern size_t dapsinglecount3[NC_MAX_VAR_DIMS];

NC_Dispatch NCD4_dispatch_base = {

NC_DISPATCH_NC4|NC_DISPATCH_NCD,

NCD4_new_nc,

NCD4_create,
NCD4_open,

NCD4_redef,
NCD4__enddef,
NCD4_sync,
NCD4_abort,
NCD4_close,
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
NCD4_get_vara,
NCD4_put_vara,
NULL, /*get_vars*/
NULL, /*put_vars*/
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

NC_Dispatch NCD4_dispatcher;

int
NCD4_initialize(void)
{
    int i;
    /* Create our dispatch table as the merge of NC4 table
       plus some overrides */
    NC_dispatch_overlay(&NCD4_dispatch_base, NC4_dispatch_table, &NCD4_dispatcher);    
    NCD4_dispatch_table = &NCD4_dispatcher;
    for(i=0;i<NC_MAX_VAR_DIMS;i++)
	{dapzerostart3[i] = 0; dapsinglecount3[i] = 1; dapsinglestride3[i] = 1;}
    return NC_NOERR;
}

static int
NCD4_create(const char *path, int cmode,
           size_t initialsz, int basepe, size_t *chunksizehintp,
	   int use_parallel, void* mpidata,
           NC_Dispatch* dispatch, NC** ncp)
{
   return NC_EPERM;
}

static int
NCD4_put_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges0,
            const void *value0,
	    nc_type memtype)
{
    return NC_EPERM;
}

static int
NCD4_redef(int ncid)
{
    return (NC_EPERM);
}

static int
NCD4__enddef(int ncid, size_t h_minfree, size_t v_align, size_t v_minfree, size_t r_align)
{
    return (NC_EPERM);
}

int
NCD4_sync(int ncid)
{
    LOG((1, "nc_sync: ncid 0x%x", ncid));
    return NC_NOERR;
}

int
NCD4_abort(int ncid)
{
    LOG((1, "nc_abort: ncid 0x%x", ncid));
    /* Turn into close */
    return NCD4_close(ncid);
}

