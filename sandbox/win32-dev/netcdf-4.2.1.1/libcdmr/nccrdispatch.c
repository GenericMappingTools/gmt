/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap4/ncd4dispatch.c,v 1.8 2010/05/27 21:34:10 dmh Exp $
 *********************************************************************/

#include "config.h"
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "netcdf.h"
#include "nc.h"
#include "ncdispatch.h"
#include "nc4internal.h"

#include "nccr.h"
#include "nccrdispatch.h"

static int
NCCR_create(const char *path, int cmode,
           size_t initialsz, int basepe, size_t *chunksizehintp,
	   int use_parallel, void* mpidata,
           NC_Dispatch*,NC** ncp);

static int NCCR_redef(int ncid);
static int NCCR__enddef(int ncid, size_t h_minfree, size_t v_align, size_t v_minfree, size_t r_align);
static int NCCR_put_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges0,
            const void *value0,
	    nc_type memtype);

NC_Dispatch NCCR_dispatch_base = {

NC_DISPATCH_NCR,

NCCR_new_nc,

NCCR_create,
NCCR_open,

NCCR_redef,
NCCR__enddef,
NCCR_sync,
NCCR_abort,
NCCR_close,
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
NCCR_get_vara,
NCCR_put_vara,
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

NC_Dispatch NCCR_dispatcher;

int
NCCR_initialize(void)
{
    /* Create our dispatch table as the merge of NC4 table
       plus some overrides */
    NC_dispatch_overlay(&NCCR_dispatch_base, NC4_dispatch_table, &NCCR_dispatcher);    
    NCCR_dispatch_table = &NCCR_dispatcher;
    ncloginit();
    return NC_NOERR;
}

static int
NCCR_create(const char *path, int cmode,
           size_t initialsz, int basepe, size_t *chunksizehintp,
	   int use_parallel, void* mpidata,
           NC_Dispatch* dispatch, NC** ncp)
{
   return NC_EPERM;
}

static int
NCCR_put_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges0,
            const void *value0,
	    nc_type memtype)
{
    return NC_EPERM;
}

static int
NCCR_redef(int ncid)
{
    return (NC_EPERM);
}

static int
NCCR__enddef(int ncid, size_t h_minfree, size_t v_align, size_t v_minfree, size_t r_align)
{
    return (NC_EPERM);
}

int
NCCR_sync(int ncid)
{
    LOG((1, "nc_sync: ncid 0x%x", ncid));
    return NC_NOERR;
}

int
NCCR_abort(int ncid)
{
    int ncstat;
    NC* nc;

    LOG((1, "nc_abort: ncid 0x%x", ncid));

    /* Avoid repeated abort */
    ncstat = NC_check_id(ncid, (NC**)&nc); 
    if(ncstat != NC_NOERR) return ncstat;

    /* Turn into close */
    return NCCR_close(ncid);
}

