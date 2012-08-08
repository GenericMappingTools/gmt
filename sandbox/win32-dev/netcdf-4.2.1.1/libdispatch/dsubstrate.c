/** \substrate
Define the substrate dispatch table and functions

These functions end up calling functions in one of the dispatch layers
(netCDF-4, dap server, etc) using the substrate field of struct NC.

Copyright 2010 University Corporation for Atmospheric
Research/Unidata. See COPYRIGHT file for more info.  
*/

#include "netcdf.h"
#include "ncdispatch.h"

/* forward */
static NC_Dispatch NCSUBSTRATE_dispatch_base;

/*
Note that because many of the signatures differ
from the netcdf API, we need to break
the abstraction and invoke the
substrate dispatch table directly.
*/


int
NCSUBSTRATE_initialize(void)
{
    NCSUBSTRATE_dispatch_table = &NCSUBSTRATE_dispatch_base;
    return NC_NOERR;
}

static int
NCSUB_new_nc(NC** ncp)
{
    NC* nc;
    /* Allocate memory for this info. */
    if (!(nc = calloc(1, sizeof(struct NC)))) 
       return NC_ENOMEM;
    if(ncp) *ncp = nc;
    return NC_NOERR;
}

static int
NCSUB_redef(int ncid)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate,&ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->redef(nc->substrate);
}

static int
NCSUB__enddef(int ncid, size_t a1, size_t a2, size_t a3, size_t a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->_enddef(nc->substrate,a1,a2,a3,a4);
}

static int
NCSUB_sync(int ncid)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->sync(nc->substrate);
}

static int
NCSUB_abort(int ncid)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->abort(nc->substrate);
}

static int
NCSUB_close(int ncid)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->close(nc->substrate);
}

static int
NCSUB_set_fill(int ncid, int a1, int* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->set_fill(nc->substrate,a1,a2);
}

static int
NCSUB_inq_base_pe(int ncid, int* a1)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_base_pe(nc->substrate,a1);
}

static int
NCSUB_set_base_pe(int ncid, int a1)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->set_base_pe(nc->substrate,a1);
}

static int
NCSUB_inq_format(int ncid, int* a1)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_format(nc->substrate,a1);
}

static int
NCSUB_inq(int ncid, int* a1, int* a2, int* a3, int* a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq(nc->substrate,a1,a2,a3,a4);
}

static int
NCSUB_inq_type(int ncid, nc_type a1, char* a2, size_t* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_type(nc->substrate,a1,a2,a3);
}

static int
NCSUB_def_dim(int ncid, const char* a1, size_t a2, int* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_dim(nc->substrate,a1,a2,a3);
}

static int
NCSUB_inq_dimid(int ncid, const char* a1, int* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_dimid(nc->substrate,a1,a2);
}

static int
NCSUB_inq_dim(int ncid, int a1, char* a2, size_t* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_dim(nc->substrate,a1,a2,a3);
}

static int
NCSUB_inq_unlimdim(int ncid,  int* a1)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_unlimdim(nc->substrate,a1);
}

static int
NCSUB_rename_dim(int ncid, int a1, const char* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->rename_dim(nc->substrate,a1,a2);
}

static int
NCSUB_inq_att(int ncid, int a1, const char* a2, nc_type* a3, size_t* a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_att(nc->substrate,a1,a2,a3,a4);
}

static int
NCSUB_inq_attid(int ncid, int a1, const char* a2, int* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_attid(nc->substrate,a1,a2,a3);
}

static int
NCSUB_inq_attname(int ncid, int a1, int a2, char* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_attname(nc->substrate,a1,a2,a3);
}

static int
NCSUB_rename_att(int ncid, int a1, const char* a2, const char* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->rename_att(nc->substrate,a1,a2,a3);
}

static int
NCSUB_del_att(int ncid, int a1, const char* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->del_att(nc->substrate,a1,a2);
}

static int
NCSUB_get_att(int ncid, int a1, const char* a2, void* a3, nc_type a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->get_att(nc->substrate,a1,a2,a3,a4);
}

static int
NCSUB_put_att(int ncid, int a1, const char* a2, nc_type a3, size_t a4, const void* a5, nc_type a6)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->put_att(nc->substrate,a1,a2,a3,a4,a5,a6);
}

static int
NCSUB_def_var(int ncid, const char* a1, nc_type a2, int a3, const int* a4, int* a5)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_var(nc->substrate,a1,a2,a3,a4,a5);
}

static int
NCSUB_inq_varid(int ncid, const char* a1, int* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_varid(nc->substrate,a1,a2);
}

static int
NCSUB_rename_var(int ncid, int a1, const char* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->rename_var(nc->substrate,a1,a2);
}

static int
NCSUB_get_vara(int ncid, int a1, const size_t* a2, const size_t* a3, void* a4, nc_type a5)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->get_vara(nc->substrate,a1,a2,a3,a4,a5);
}

static int
NCSUB_put_vara(int ncid, int a1, const size_t* a2, const size_t* a3, const void* a4, nc_type a5)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->put_vara(nc->substrate,a1,a2,a3,a4,a5);
}

/* Added to solve Ferret performance problem with Opendap */
static int
NCSUB_get_vars(int ncid, int a1, const size_t* a2, const size_t* a3, const ptrdiff_t* a4, void* a5, nc_type a6)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->get_vars(nc->substrate,a1,a2,a3,a4,a5,a6);
}

static int
NCSUB_put_vars(int ncid, int a1, const size_t* a2, const size_t* a3, const ptrdiff_t* a4, const void* a5, nc_type a6)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->put_vars(nc->substrate,a1,a2,a3,a4,a5,a6);
}

static int
NCSUB_get_varm(int ncid, int a1, const size_t* a2, const size_t* a3, const ptrdiff_t* a4, const ptrdiff_t* a5, void* a6, nc_type a7)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->get_varm(nc->substrate,a1,a2,a3,a4,a5,a6,a7);
}

static int
NCSUB_put_varm(int ncid, int a1, const size_t* a2, const size_t* a3, const ptrdiff_t* a4, const ptrdiff_t* a5, const void* a6, nc_type a7)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->put_varm(nc->substrate,a1,a2,a3,a4,a5,a6,a7);
}


static int
NCSUB_inq_var_all(int ncid, int varid, char* name, nc_type* xtypep, 
               int* ndimsp, int* dimidsp, int* nattsp, 
               int* shufflep, int* deflatep, int* deflate_levelp, 
               int* fletcher32p, int* contiguousp, size_t* chunksizesp, 
               int* no_fill, void* fill_valuep, int* endiannessp, 
	       int* options_maskp, int* pixels_per_blockp)
{
   NC *nc, *ncsub;
   int ncstat = NC_check_id(ncid, &nc);
   if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
   if(ncstat != NC_NOERR) return ncstat;
   return ncsub->dispatch->inq_var_all(nc->substrate,varid,name,xtypep,
                                     ndimsp,dimidsp,nattsp,shufflep,
                                     deflatep,deflate_levelp,fletcher32p,
                                     contiguousp,chunksizesp,
                                     no_fill,fill_valuep,
                                     endiannessp,
                                     options_maskp,pixels_per_blockp);
}

#ifdef USE_NETCDF4

static int
NCSUB_show_metadata(int ncid)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->show_metadata(nc->substrate);
}

static int
NCSUB_inq_unlimdims(int ncid, int* a1, int* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_unlimdims(nc->substrate,a1,a2);
}

static int
NCSUB_var_par_access(int ncid, int a1, int a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->var_par_access(nc->substrate,a1,a2);
}

static int
NCSUB_inq_ncid(int ncid, const char* a1, int* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_ncid(nc->substrate,a1,a2);
}

static int
NCSUB_inq_grps(int ncid, int* a1, int* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_grps(nc->substrate,a1,a2);
}

static int
NCSUB_inq_grpname(int ncid, char* a1)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_grpname(nc->substrate,a1);
}

static int
NCSUB_inq_grpname_full(int ncid, size_t* a1, char* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_grpname_full(nc->substrate,a1,a2);
}

static int
NCSUB_inq_grp_parent(int ncid, int* a1)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_grp_parent(nc->substrate,a1);
}

static int
NCSUB_inq_grp_full_ncid(int ncid, const char* a1, int* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_grp_full_ncid(nc->substrate,a1,a2);
}

static int
NCSUB_inq_varids(int ncid, int* a1, int* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_varids(nc->substrate,a1,a2);
}

static int
NCSUB_inq_dimids(int ncid, int* a1, int* a2, int a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_dimids(nc->substrate,a1,a2,a3);
}

static int
NCSUB_inq_typeids(int ncid, int* a1, int* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_typeids(nc->substrate,a1,a2);
}

static int
NCSUB_inq_type_equal(int ncid, nc_type a1, int a2, nc_type a3, int* a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_type_equal(nc->substrate,a1,a2,a3,a4);
}

static int
NCSUB_def_grp(int ncid, const char* a1, int* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_grp(nc->substrate,a1,a2);
}

static int
NCSUB_inq_user_type(int ncid, nc_type a1, char* a2, size_t* a3, nc_type* a4, size_t* a5, int* a6)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_user_type(nc->substrate,a1,a2,a3,a4,a5,a6);
}

static int
NCSUB_inq_typeid(int ncid, const char* a1, nc_type* a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_typeid(nc->substrate,a1,a2);
}

static int
NCSUB_def_compound(int ncid, size_t a1, const char* a2, nc_type* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_compound(nc->substrate,a1,a2,a3);
}

static int
NCSUB_insert_compound(int ncid, nc_type a1, const char* a2, size_t a3, nc_type a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->insert_compound(nc->substrate,a1,a2,a3,a4);
}

static int
NCSUB_insert_array_compound(int ncid, nc_type a1, const char* a2, size_t a3, nc_type a4, int a5, const int* a6)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->insert_array_compound(nc->substrate,a1,a2,a3,a4,a5,a6);
}

static int
NCSUB_inq_compound_field(int ncid, nc_type a1, int a2, char* a3, size_t* a4, nc_type* a5, int* a6, int* a7)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_compound_field(nc->substrate,a1,a2,a3,a4,a5,a6,a7);
}

static int
NCSUB_inq_compound_fieldindex(int ncid, nc_type a1, const char* a2, int* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_compound_fieldindex(nc->substrate,a1,a2,a3);
}

static int
NCSUB_def_vlen(int ncid, const char* a1, nc_type a2, nc_type* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_vlen(nc->substrate,a1,a2,a3);
}

static int
NCSUB_put_vlen_element(int ncid, int a1, void* a2, size_t a3, const void* a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->put_vlen_element(nc->substrate,a1,a2,a3,a4);
}

static int
NCSUB_get_vlen_element(int ncid, int a1, const void* a2, size_t* a3, void* a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->get_vlen_element(nc->substrate,a1,a2,a3,a4);
}

static int
NCSUB_def_enum(int ncid, nc_type a1, const char* a2, nc_type* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_enum(nc->substrate,a1,a2,a3);
}

static int
NCSUB_insert_enum(int ncid, nc_type a1, const char* a2, const void* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->insert_enum(nc->substrate,a1,a2,a3);
}

static int
NCSUB_inq_enum_member(int ncid, nc_type a1, int a2, char* a3, void* a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_enum_member(nc->substrate,a1,a2,a3,a4);
}

static int
NCSUB_inq_enum_ident(int ncid, nc_type a1, long long a2, char* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->inq_enum_ident(nc->substrate,a1,a2,a3);
}

static int
NCSUB_def_opaque(int ncid, size_t a1, const char* a2, nc_type* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_opaque(nc->substrate,a1,a2,a3);
}

static int
NCSUB_def_var_deflate(int ncid, int a1, int a2, int a3, int a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_var_deflate(nc->substrate,a1,a2,a3,a4);
}

static int
NCSUB_def_var_fletcher32(int ncid, int a1, int a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_var_fletcher32(nc->substrate,a1,a2);
}

static int
NCSUB_def_var_chunking(int ncid, int a1, int a2, const size_t* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_var_chunking(nc->substrate,a1,a2,a3);
}

static int
NCSUB_def_var_fill(int ncid, int a1, int a2, const void* a3)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_var_fill(nc->substrate,a1,a2,a3);
}

static int
NCSUB_def_var_endian(int ncid, int a1, int a2)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->def_var_endian(nc->substrate,a1,a2);
}

static int
NCSUB_set_var_chunk_cache(int ncid, int a1, size_t a2, size_t a3, float a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->set_var_chunk_cache(nc->substrate,a1,a2,a3,a4);
}

static int
NCSUB_get_var_chunk_cache(int ncid, int a1, size_t* a2, size_t* a3, float* a4)
{
    NC *nc, *ncsub;
    int ncstat = NC_check_id(ncid, &nc);
    if(ncstat != NC_NOERR) return ncstat;
    ncstat = NC_check_id(nc->substrate, &ncsub);
    if(ncstat != NC_NOERR) return ncstat;
    return ncsub->dispatch->get_var_chunk_cache(nc->substrate,a1,a2,a3,a4);
}

#endif /*USE_NETCDF4*/

/* Declare here to avoid having a bunch of static forward declarations */ 

static NC_Dispatch NCSUBSTRATE_dispatch_base =  {

0,

NCSUB_new_nc,

NULL, /*NC_create*/
NULL, /*NC_open*/

NCSUB_redef,
NCSUB__enddef,
NCSUB_sync,
NCSUB_abort,
NCSUB_close,
NCSUB_set_fill,
NCSUB_inq_base_pe,
NCSUB_set_base_pe,
NCSUB_inq_format,

NCSUB_inq,
NCSUB_inq_type,

NCSUB_def_dim,
NCSUB_inq_dimid,
NCSUB_inq_dim,
NCSUB_inq_unlimdim,
NCSUB_rename_dim,

NCSUB_inq_att,
NCSUB_inq_attid,
NCSUB_inq_attname,
NCSUB_rename_att,
NCSUB_del_att,
NCSUB_get_att,
NCSUB_put_att,

NCSUB_def_var,
NCSUB_inq_varid,
NCSUB_rename_var,

NCSUB_get_vara,
NCSUB_put_vara,
NCSUB_get_vars,
NCSUB_put_vars,
NCSUB_get_varm,
NCSUB_put_varm,

NCSUB_inq_var_all,

#ifdef USE_NETCDF4
NCSUB_show_metadata,
NCSUB_inq_unlimdims,

NCSUB_var_par_access,

NCSUB_inq_ncid,
NCSUB_inq_grps,
NCSUB_inq_grpname,
NCSUB_inq_grpname_full,
NCSUB_inq_grp_parent,
NCSUB_inq_grp_full_ncid,
NCSUB_inq_varids,
NCSUB_inq_dimids,
NCSUB_inq_typeids,
NCSUB_inq_type_equal,
NCSUB_def_grp,
NCSUB_inq_user_type,
NCSUB_inq_typeid,

NCSUB_def_compound,
NCSUB_insert_compound,
NCSUB_insert_array_compound,
NCSUB_inq_compound_field,
NCSUB_inq_compound_fieldindex,
NCSUB_def_vlen,
NCSUB_put_vlen_element,
NCSUB_get_vlen_element,
NCSUB_def_enum,
NCSUB_insert_enum,
NCSUB_inq_enum_member,
NCSUB_inq_enum_ident,
NCSUB_def_opaque,
NCSUB_def_var_deflate,
NCSUB_def_var_fletcher32,
NCSUB_def_var_chunking,
NCSUB_def_var_fill,
NCSUB_def_var_endian,
NCSUB_set_var_chunk_cache,
NCSUB_get_var_chunk_cache

#endif /*USE_NETCDF4*/

};

NC_Dispatch* NCSUBSTRATE_dispatch_table = NULL; /*Moved here from ddispatch.c*/
