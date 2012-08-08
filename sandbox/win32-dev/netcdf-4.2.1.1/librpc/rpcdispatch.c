/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "rpc_includes.h"

int
NCRPC_create(const char *path, int cmode,
           size_t initialsz, int basepe, size_t *chunksizehintp,
	   int useparallel, void* parameters,
	   NC_Dispatch* dispatcher, NC** ncidp)
{
    int ncstat = NC_NOERR;
    NCCreate* request = (NCCreate*)calloc(1,sizeof(NCCreate));
    NCCreate_Return* response = NULL;
    request->path = nulldup(path);
    request->cmode = cmode;
    request->initialsz = initialsz;
    request->basepe = basepe;
    request->use_parallel = useparallel;
    /* currently, chunksizehint is ignored */
    /* currently, parameters must always be null */
    if(parameters != NULL) {
	ncstat = NC_ENOPAR; /* not supported */
	goto fail;
    }
    ncstat = rpc_send(NCRPC_CREATE,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_CREATE,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(ncidp != NULL) *ncidp = response->ncid;    
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_open(const char *path, int cmode,
         int basepe, size_t *chunksizehintp, 
	 int use_parallel, void* parameters,
	 NC_Dispatch* dispatch, NC** ncp)
{
    int ncstat = NC_NOERR;
    NCOpen* request = (Open*)calloc(1,sizeof(NCOpen));
    NCOpen_Return* response = NULL;
    request->path = nulldup(path);
    request->cmode = cmode;
    request->initialsz = initialsz;
    request->basepe = basepe;
    request->use_parallel = useparallel;
    /* currently, parameters must always be null */
    if(parameters != NULL) {
	ncstat = NC_ENOPAR; /* not supported */
	goto fail;
    }
    ncstat = rpc_send(NCRPC_OPEN,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_OPEN,(void*)response);
    if(ncstat != NC_NOERR) goto fail;

    /* Create a local, pseudo NC */

    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(ncidp != NULL) *ncidp = response->ncid;    
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_redef(int ncid)
{
    int ncstat = NC_NOERR;
    NCRedef* request = (Redef*)calloc(1,sizeof(NCRedef));
    NCRedef_Return* response = NULL;
    request->ncid = ncid;
    ncstat = rpc_send(NCRPC_REDEF,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_REDEF,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC__enddef(int ncid, size_t h_minfree, size_t v_align,
	size_t v_minfree, size_t r_align)
{
    int ncstat = NC_NOERR;
    NCEnddef* request = (Enddef*)calloc(1,sizeof(NCEnddef));
    NCEnddef_Return* response = NULL;
    request->ncid = ncid;
    request->h_minfree = h_minfree;
    request->v_align = v_align;
    request->v_minfree = v_minfree
    request->r_align = r_align;
    ncstat = rpc_send(NCRPC_ENDDEF,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_ENDDEF,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_sync(int ncid)
{
    int ncstat = NC_NOERR;
    NCSync* request = (Sync*)calloc(1,sizeof(NCSync));
    NCSync_Return* response = NULL;
    request->ncid = ncid;
    ncstat = rpc_send(NCRPC_SYNC,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_SYNC,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_abort(int ncid)
{
    int ncstat = NC_NOERR;
    NCAbort* request = (Abort*)calloc(1,sizeof(NCAbort));
    NCAbort_Return* response = NULL;
    request->ncid = ncid;
    ncstat = rpc_send(NCRPC_ABORT,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_ABORT,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_close(int ncid)
{
    int ncstat = NC_NOERR;
    NCClose* request = (Close*)calloc(1,sizeof(NCClose));
    NCClose_Return* response = NULL;
    request->ncid = ncid;
    ncstat = rpc_send(NCRPC_CLOSE,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_CLOSE,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_set_fill(int ncid, int fillmode, int *old_modep)
{
    int ncstat = NC_NOERR;
    Set_NCFill* request = (Set_Fill*)calloc(1,sizeof(NCSet_Fill));
    NCSet_Fill_Return* response = NULL;
    request->ncid = ncid;
    request->fillmode = fillmode;
    ncstat = rpc_send(NCRPC_SET_FILL,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_SET_FILL,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(old_modep) *old_modep = response->oldmode;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_inq_base_pe(int ncid, int *pe)
{
    int ncstat = NC_NOERR;
    Inq_Base_NCPE* request = (Inq_Base_PE*)calloc(1,sizeof(NCInq_Base_PE));
    NCInq_Base_PE_Return* response = NULL;
    request->ncid = ncid;
    ncstat = rpc_send(NCRPC_INQ_BASE_PE,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_INQ_BASE_PE,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(pe != NULL) *pe = response->pe;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_set_base_pe(int ncid, int pe)
{
    int ncstat = NC_NOERR;
    Set_Base_NCPE* request = (Set_Base_PE*)calloc(1,sizeof(NCSet_Base_PE));
    NCSet_Base_PE_Return* response = NULL;
    request->ncid = ncid;
    request->pe = pe;
    ncstat = rpc_send(NCRPC_SET_BASE_PE,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_SET_BASE_PE,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_inq_format(int ncid, int *formatp)
{
    int ncstat = NC_NOERR;
    Inq_NCFormat* request = (Inq_Format*)calloc(1,sizeof(NCInq_Format));
    NCInq_Format_Return* response = NULL;
    request->ncid = ncid;
    ncstat = rpc_send(NCRPC_INQ_FORMAT,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_INQ_FORMAT,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(formatp != NULL) *formatp = response->format;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimidp)
{
    int ncstat = NC_NOERR;
    NCInq* request = (Inq*)calloc(1,sizeof(NCInq));
    NCInq_Return* response = NULL;
    request->ncid = ncid;
    ncstat = rpc_send(NCRPC_INQ,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_INQ,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(ndimsp != NULL) *ndimsp = response->ndims;
    if(nvarsp != NULL) *nvarsp = response->nvars;
    if(nattsp != NULL) *nattsp = response->natts;
    if(unlimidimidp != NULL) *unlimidimidp = response->unlimidimid;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_inq_type(int ncid, nc_type xtype, char* name, size_t* sizep)
{
    int ncstat = NC_NOERR;
    Inq_NCType* request = (Inq_Type*)calloc(1,sizeof(NCInq_Type));
    NCInq_Type_Return* response = NULL;
    request->ncid = ncid;
    request->xtype = xtype;
    ncstat = rpc_send(NCRPC_INQ_TYPE,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_INQ_TYPE,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(name != NULL) strcpy(name,response->name);
    if(sizep != NULL) *size = response->size;
    return ncstat;

fail:
    return ncstat;    
}

/* Begin _dim */

int
NCRPC_def_dim(int ncid, const char *name, size_t len, int *idp)
{
    int ncstat = NC_NOERR;
    Def_NCDim* request = (Def_Dim*)calloc(1,sizeof(NCDef_Dim));
    NCDef_Dim_Return* response = NULL;
    request->ncid = ncid;
    request->name = nulldup(name);
    request->len = len;
    ncstat = rpc_send(NCRPC_DEF_DIM,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_DEF_DIM,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(idp != NULL) *idp = response->dimid;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_inq_typedimid(int ncid, const char *name, int *idp)
{
}

int
NCRPC_inq_dim(int ncid, int dimid, char *name, size_t *lenp)
{
}

int
NCRPC_inq_unlimdim(int ncid, int *unlimdimidp)
{
}

int
NCRPC_rename_dim(int ncid, int dimid, const char *name)
{
}

/* End _dim */
/* Begin _att */

int
NCRPC_inq_att(int ncid, int varid, const char *name,
	    nc_type *xtypep, size_t *lenp)
{
}

int 
NCRPC_inq_attid(int ncid, int varid, const char *name, int *idp)
{
}

int
NCRPC_inq_attname(int ncid, int varid, int attnum, char *name)
{
}

int
NCRPC_rename_att(int ncid, int varid, const char *name, const char *newname)
{
}

int
NCRPC_del_att(int ncid, int varid, const char*)
{
}

/* End _att */
/* Begin {put,get}_att */

int
NCRPC_get_att(int ncid, int varid, const char *name, void *value, nc_type)
{
}

int
NCRPC_put_att(int ncid, int varid, const char *name, nc_type datatype,
	   size_t len, const void *value, nc_type)
{
}

/* End {put,get}_att */
/* Begin _var */

int
NCRPC_def_var(int ncid, const char *name,
	 nc_type xtype, int ndims, const int *dimidsp, int *varidp)
{
    int i;
    int ncstat = NC_NOERR;
    Def_NCVar* request = (Def_Var*)calloc(1,sizeof(NCDef_Var));
    NCDef_Var_Return* response = NULL;
    request->ncid = ncid;
    request->name = nulldup(name);
    request->xtype = xtype;
    request->ndims = ndims;
    request->dimids.count = ndims;
    request->dimids.values = (int32_t*)calloc(ndims,sizeof(int32_t));
    for(i=0;i<ndims;i++)
	request->dimids.values[i] = dimidsp[i];
    ncstat = rpc_send(NCRPC_DEF_VAR,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_DEF_VAR,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(varidp != NULL) *varidp = response->varid;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_inq_var_all(int ncid, int varid, char *name, nc_type *xtypep, 
               int *ndimsp, int *dimidsp, int *nattsp, 
               int *shufflep, int *deflatep, int *deflate_levelp,
               int *fletcher32p, int *contiguousp, size_t *chunksizesp, 
               int *no_fill, void *fill_valuep, int *endiannessp, 
	       int *options_maskp, int *pixels_per_blockp)
{
}

int
NCRPC_inq_varid(int ncid, const char *name, int *varidp)
{
}

int
NCRPC_rename_var(int ncid, int varid, const char *name)
{
}

int
NCRPC_put_vara(int ncid, int varid,
   	     const size_t *start, const size_t *count,
             const void *value, nc_type)
{
}

int
NCRPC_get_vara(int ncid, int varid,
	     const size_t *start, const size_t *count,
             void *value, nc_type)
{
}

/* End _var */

/* netCDF4 API only */
int
NCRPC_var_par_access(int, int, int)
{
}

int
NCRPC_inq_ncid(int, const char *, int *)
{
}

int
NCRPC_inq_grps(int, int *, int *)
{
}

int
NCRPC_inq_grpname(int, char *)
{
}

int
NCRPC_inq_grpname_full(int, size_t *, char *)
{
}

int
NCRPC_inq_grp_parent(int, int *)
{
}

int
NCRPC_inq_grp_full_ncid(int, const char *, int *)
{
}

int
NCRPC_inq_varids(int, int * nvars, int *)
{
}

int
NCRPC_inq_dimids(int, int * ndims, int *, int)
{
}

int
NCRPC_inq_typeids(int, int * ntypes, int *)
{
}
   
int
NCRPC_inq_type_equal(int, nc_type, int, nc_type, int *)
{
}

int
NCRPC_def_grp(int ncid, const char* name, int* grpidp)
{
    int ncstat = NC_NOERR;
    Def_NCGrp* request = (Def_Grp*)calloc(1,sizeof(NCDef_Grp));
    NCDef_Grp_Return* response = NULL;
    request->ncid = ncid;
    request->name = nulldup(name);
    ncstat = rpc_send(NCRPC_DEF_GRP,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_DEF_GRP,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(grpidp != NULL) *grpidp = response->grpncid;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_inq_user_type(int, nc_type, char *, size_t *, nc_type *, 
		  size_t *, int *)
{
}

int
NCRPC_def_compound(int ncid, size_t size, const char* name, nc_type* typeidp)
{
    int ncstat = NC_NOERR;
    Def_NCCompound* request = (Def_Compound*)calloc(1,sizeof(NCDef_Compound));
    NCDef_Compound_Return* response = NULL;
    request->ncid = ncid;
    request->size = size;
    request->name = nulldup(name);
    ncstat = rpc_send(NCRPC_DEF_COMPOUND,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_DEF_COMPOUND,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(typeidp != NULL) *typeidp = response->typeid;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_insert_compound(int, nc_type, const char *, size_t, nc_type)
{
}

int
NCRPC_insert_array_compound(int, nc_type, const char *, size_t, 
			  nc_type, int, const int *)
{
}

int
NCRPC_inq_typeid(int, const char *, nc_type *)
{
}

int
NCRPC_inq_compound_field(int, nc_type, int, char *, size_t *, 
		       nc_type *, int *, int *)
{
}

int
NCRPC_inq_compound_fieldindex(int, nc_type, const char *, int *)
{
}

int
NCRPC_def_vlen(int ncid, const char* name, nc_type base_typeid, nc_type* idp)
{
    int ncstat = NC_NOERR;
    Def_NCVlen* request = (Def_Vlen*)calloc(1,sizeof(NCDef_Vlen));
    NCDef_Vlen_Return* response = NULL;
    request->ncid = ncid;
    request->name = nulldup(name);
    request->base_typeid = base_typeid;
    ncstat = rpc_send(NCRPC_DEF_VLEN,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_DEF_VLEN,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(idp != NULL) *idp = response->typeid;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_put_vlen_element(int, int, void *, size_t, const void *)
{
}

int
NCRPC_get_vlen_element(int, int, const void *, size_t *, void *)
{
}

int
NCRPC_def_enum(int ncid, nc_type basetypeid, const char* name, nc_type* idp)
{
    int ncstat = NC_NOERR;
    Def_NCEnum* request = (Def_Enum*)calloc(1,sizeof(NCDef_Enum));
    NCDef_Enum_Return* response = NULL;
    request->ncid = ncid;
    request->name = nulldup(name);
    request->basetypeid = basetypeid;
    ncstat = rpc_send(NCRPC_DEF_ENUM,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_DEF_ENUM,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(idp != NULL) *idp = response->typeid;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_insert_enum(int, nc_type, const char *, const void *)
{
}

int
NCRPC_inq_enum_member(int, nc_type, int, char *, void *)
{
}

int
NCRPC_inq_enum_ident(int, nc_type, long long, char *)
{
}

int
NCRPC_def_opaque(int ncid, size_t size , const char* name, nc_type* idp)
{
    int ncstat = NC_NOERR;
    Def_NCOpaque* request = (Def_Opaque*)calloc(1,sizeof(NCDef_Opaque));
    NCDef_Opaque_Return* response = NULL;
    request->ncid = ncid;
    request->name = nulldup(name);
    request->size = size;
    ncstat = rpc_send(NCRPC_DEF_OPAQUE,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_DEF_OPAQUE,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    if(idp != NULL) *idp = response->typeid;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_def_var_deflate(int ncid, int varid, int shuffle, int deflate, int deflatelevel)
{
    int ncstat = NC_NOERR;
    Def_Var_NCDeflate* request = (Def_Var_Deflate*)calloc(1,sizeof(NCDef_Var_Deflate));
    NCDef_Var_Deflate_Return* response = NULL;
    request->ncid = ncid;
    request->varid  varid;
    request->size = size;
    request->shuffle = (shuffle == NC_SHUFFLE?1:0);
    request->deflate = (deflate?1:0);
    request->deflatelevel = deflatelevel;
    ncstat = rpc_send(NCRPC_DEF_VAR_DEFLATE,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_DEF_VAR_DEFLATE,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_def_var_fletcher32(int ncid, int varid, int fletcher32)
{
    int ncstat = NC_NOERR;
    Def_Var_NCFletcher32* request = (Def_Var_Fletcher32*)calloc(1,sizeof(NCDef_Var_Fletcher32));
    NCDef_Var_Fletcher32_Return* response = NULL;
    request->ncid = ncid;
    request->varid  varid;
    request->fletcher32 = (fletcher32 == NC_FLETCHER32?1:0);
    ncstat = rpc_send(NCRPC_DEF_VAR_FLETCHER32,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_DEF_VAR_FLETCHER32,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_def_var_chunking(int ncid, int varid, int contiguous, const size_t* chunksizes)
{
    int i, ndims;
    int ncstat = NC_NOERR;
    Def_Var_NCChunking* request = (Def_Var_Chunking*)calloc(1,sizeof(NCDef_Var_Chunking));
    NCDef_Var_Chunking_Return* response = NULL;
    request->ncid = ncid;
    request->varid  varid;
    request->contiguous = (contiguous == NC_CONTIGUOUS?1:0);
    /* Get the number of dimensions */
    ncstat = nc_inq_ndims(ncid,&ndims);
    if(ncstat != NC_NOERR) goto fail;
    request->chunksizes.count = ndims;
    for(i=0;i<ndims;i++)
	request->chunksizes.values[i] = chunksizes[i];
    ncstat = rpc_send(NCRPC_DEF_VAR_CHUNKING,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_DEF_VAR_CHUNKING,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_def_var_fill(int ncid, int varid, int nofill, const void* fill_value)
{
    int i, ndims;
    int ncstat = NC_NOERR;
    NCDef_Var_Fill* request = (Def_Var_Fill*)calloc(1,sizeof(NCDef_Var_Fill));
    NCDef_Var_Fill_Return* response = NULL;
    request->ncid = ncid;
    request->varid  varid;
    request->nofill = (nofill == 1?1:0);
    /* Get the size of an instance */
    ncstat = nc_inq_ndims(ncid,&ndims);
    if(ncstat != NC_NOERR) goto fail;
    request->chunksizes.count = ndims;
    for(i=0;i<ndims;i++)
	request->chunksizes.values[i] = chunksizes[i];
    ncstat = rpc_send(NCRPC_DEF_VAR_FILL,(void*)request);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = rpc_receive(NCRPC_DEF_VAR_FILL,(void*)response);
    if(ncstat != NC_NOERR) goto fail;
    ncstat = response->ncstatus;
    if(ncstat != NC_NOERR) goto fail;
    return ncstat;

fail:
    return ncstat;    
}

int
NCRPC_def_var_endian(int, int, int)
{
}

int
NCRPC_set_var_chunk_cache(int, int, size_t, size_t, float)
{
}

int
NCRPC_get_var_chunk_cache(int, int, size_t *, size_t *, float *)
{
}

int
NCRPC_inq_unlimdims(int, int *, int *)
{
}

int
NCRPC_show_metadata(int)
{
}

int 
NCRPC_initialize(void)
{
}
