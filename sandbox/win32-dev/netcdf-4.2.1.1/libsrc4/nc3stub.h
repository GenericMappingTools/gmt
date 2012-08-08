/*
 * Copyright 1993-1996 University Corporation for Atmospheric Research/Unidata
 * 
 */

#ifndef _NC3STUB_H
#define _NC3STUB_H

#ifndef MPI_INCLUDED
#define MPI_Comm int
#define MPI_Info int
#endif

#if defined(__cplusplus)
extern "C" {
#endif
/*
 * The Interface
 */

/* Declaration modifiers for DLL support (MSC et al) */

#if defined(DLL_NETCDF) /* define when library is a DLL */
#  if defined(DLL_EXPORT) /* define when building the library */
#   define MSC_EXTRA __declspec(dllexport)
#  else
#   define MSC_EXTRA __declspec(dllimport)
#  endif
#else
#define MSC_EXTRA
#endif	/* defined(DLL_NETCDF) */

# define EXTERNL extern MSC_EXTRA

EXTERNL int
nc3_delete_mp(const char * path, int basepe);

EXTERNL int
nc3_create(const char *path, int cmode, size_t initialsz, int basepe,
	   size_t *chunksizehintp,
	   MPI_Comm, MPI_Info,
	   NC**);

EXTERNL int
nc3_open(const char *path, int mode, int basepe, size_t *chunksizehintp, 
	 int use_parallel, MPI_Comm, MPI_Info,
	 NC**);

EXTERNL int
nc3_redef(int ncid);

EXTERNL int
nc3__enddef(int ncid, size_t h_minfree, size_t v_align,
	size_t v_minfree, size_t r_align);

EXTERNL int
nc3_sync(int ncid);

EXTERNL int
nc3_abort(int ncid);

EXTERNL int
nc3_close(int ncid);

EXTERNL int
nc3_set_fill(int ncid, int fillmode, int *old_modep);

EXTERNL int
nc3_set_base_pe(int ncid, int pe);

EXTERNL int
nc3_inq_base_pe(int ncid, int *pe);

EXTERNL int
nc3_inq_format(int ncid, int *formatp);

EXTERNL int
nc3_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimidp);

EXTERNL int
nc3_inq_type(int,nc_type,char*,size_t*);

/* Begin _dim */

EXTERNL int
nc3_def_dim(int ncid, const char *name, size_t len, int *idp);

EXTERNL int
nc3_inq_dimid(int ncid, const char *name, int *idp);

EXTERNL int
nc3_inq_dim(int ncid, int dimid, char *name, size_t *lenp);

EXTERNL int
nc3_rename_dim(int ncid, int dimid, const char *name);

/* End _dim */
/* Begin _att */

EXTERNL int
nc3_inq_att(int ncid, int varid, const char *name,
	 nc_type *xtypep, size_t *lenp);

EXTERNL int 
nc3_inq_attid(int ncid, int varid, const char *name, int *idp);

EXTERNL int
nc3_inq_attname(int ncid, int varid, int attnum, char *name);

EXTERNL int
nc3_rename_att(int ncid, int varid, const char *name, const char *newname);

EXTERNL int
nc3_del_att(int ncid, int varid, const char*);

/* End _att */
/* Begin {put,get}_att */

EXTERNL int
nc3_get_att(int ncid, int varid, const char *name, void *value, nc_type);

EXTERNL int
nc3_put_att(int ncid, int varid, const char *name, nc_type datatype,
	   size_t len, const void *value, nc_type);

/* End {put,get}_att */
/* Begin _var */

EXTERNL int
nc3_def_var(int ncid, const char *name,
	 nc_type xtype, int ndims, const int *dimidsp, int *varidp);

EXTERNL int
nc3_inq_var(int ncid, int varid, char *name,
	 nc_type *xtypep, int *ndimsp, int *dimidsp, int *nattsp);

EXTERNL int
nc3_inq_varid(int ncid, const char *name, int *varidp);

EXTERNL int
nc3_rename_var(int ncid, int varid, const char *name);

EXTERNL int
nc3_put_vara(int ncid, int varid,
   	     const size_t *start, const size_t *count,
             const void *value, nc_type);

EXTERNL int
nc3_get_vara(int ncid, int varid,
	     const size_t *start, const size_t *count,
             void *value, nc_type);

EXTERNL int
nc3_put_var(int ncid, int varid,  const void *op);

EXTERNL int
nc3_get_var(int ncid, int varid,  void *ip);

EXTERNL int
nc3_put_var1(int ncid, int varid,  const size_t *indexp,
	    const void *op);

EXTERNL int
nc3_get_var1(int ncid, int varid,  const size_t *indexp, void *ip);

EXTERNL int
nc3_put_vars(int ncid, int varid,  const size_t *startp, 
	    const size_t *countp, const ptrdiff_t *stridep,
	    const void *op);

EXTERNL int
nc3_get_vars(int ncid, int varid,  const size_t *startp, 
	    const size_t *countp, const ptrdiff_t *stridep,
	    void *ip);

EXTERNL int
nc3_put_varm(int ncid, int varid,  const size_t *startp, 
	    const size_t *countp, const ptrdiff_t *stridep,
	    const ptrdiff_t *imapp, const void *op);

EXTERNL int
nc3_get_varm(int ncid, int varid,  const size_t *startp, 
	    const size_t *countp, const ptrdiff_t *stridep,
	    const ptrdiff_t *imapp, void *ip);


/* End _var */

/* netCDF4 API only */
EXTERNL int
nc3_var_par_access(int,int,int);

EXTERNL int
nc3_inq_ncid(int,const char*,int*);

EXTERNL int
nc3_inq_grps(int,int*,int*);

EXTERNL int
nc3_inq_grpname(int,char*);

EXTERNL int
nc3_inq_grpname_full(int,size_t*,char*);

EXTERNL int
nc3_inq_grp_parent(int,int*);

EXTERNL int
nc3_inq_grp_full_ncid(int,const char*,int*);

EXTERNL int
nc3_inq_varids(int,int* nvars,int*);

EXTERNL int
nc3_inq_dimids(int,int* ndims,int*,int);

EXTERNL int
nc3_inq_typeids(int,int* ntypes,int*);

EXTERNL int
nc3_inq_type_equal(int,nc_type,int,nc_type,int*);

EXTERNL int
nc3_def_grp(int,const char*,int*);

EXTERNL int
nc3_inq_user_type(int,nc_type,char*,size_t*,nc_type*,size_t*,int*);


EXTERNL int
nc3_def_compound(int,size_t,const char*,nc_type*);

EXTERNL int
nc3_insert_compound(int,nc_type,const char*,size_t,nc_type);

EXTERNL int
nc3_insert_array_compound(int,nc_type,const char*,size_t,nc_type,int,const int*);

EXTERNL int
nc3_inq_typeid(int,const char*,nc_type*);

EXTERNL int
nc3_inq_compound_field(int,nc_type,int,char*,size_t*,nc_type*,int*,int*);

EXTERNL int
nc3_inq_compound_fieldindex(int,nc_type,const char*,int*);

EXTERNL int
nc3_def_vlen(int,const char*,nc_type base_typeid,nc_type*);

EXTERNL int
nc3_put_vlen_element(int,int,void*,size_t,const void*);

EXTERNL int
nc3_get_vlen_element(int,int,const void*,size_t*,void*);

EXTERNL int
nc3_def_enum(int,nc_type,const char*,nc_type*);

EXTERNL int
nc3_insert_enum(int,nc_type,const char*,const void*);

EXTERNL int
nc3_inq_enum_member(int,nc_type,int,char*,void*);

EXTERNL int
nc3_inq_enum_ident(int,nc_type,long long,char*);

EXTERNL int
nc3_def_opaque(int,size_t,const char*,nc_type*);

EXTERNL int
nc3_def_var_deflate(int,int,int,int,int);

EXTERNL int
nc3_inq_var_deflate(int,int,int*,int*,int*);

EXTERNL int
nc3_inq_var_szip(int,int,int*,int*);

EXTERNL int
nc3_def_var_fletcher32(int,int,int);

EXTERNL int
nc3_inq_var_fletcher32(int,int,int*);

EXTERNL int
nc3_def_var_chunking(int,int,int,const size_t*);

EXTERNL int
nc3_inq_var_chunking(int,int,int*,size_t*);

EXTERNL int
nc3_def_var_fill(int,int,int,const void*);

EXTERNL int
nc3_inq_var_fill(int,int,int*,void*);

EXTERNL int
nc3_def_var_endian(int,int,int);

EXTERNL int
nc3_inq_var_endian(int,int,int*);

EXTERNL int
nc3_set_var_chunk_cache(int,int,size_t,size_t,float);

EXTERNL int
nc3_get_var_chunk_cache(int,int,size_t*,size_t*,float*);

EXTERNL int
nc3_inq_unlimdims(int,int*,int*);

EXTERNL int 
nc3_inq_unlimdim(int ncid, int *unlimdimidp);

EXTERNL int
nc3_show_metadata(int);

EXTERNL int
nc3_put_att_text(int ncid, int varid, const char *name,
		size_t len, const char *op);

EXTERNL int
nc3_get_att_text(int ncid, int varid, const char *name, char *ip);

EXTERNL int
nc3_put_att_uchar(int ncid, int varid, const char *name, nc_type xtype,
		 size_t len, const unsigned char *op);

EXTERNL int
nc3_get_att_uchar(int ncid, int varid, const char *name, unsigned char *ip);

EXTERNL int
nc3_put_att_schar(int ncid, int varid, const char *name, nc_type xtype,
		 size_t len, const signed char *op);

EXTERNL int
nc3_get_att_schar(int ncid, int varid, const char *name, signed char *ip);

EXTERNL int
nc3_put_att_short(int ncid, int varid, const char *name, nc_type xtype,
		 size_t len, const short *op);

EXTERNL int
nc3_get_att_short(int ncid, int varid, const char *name, short *ip);

EXTERNL int
nc3_put_att_int(int ncid, int varid, const char *name, nc_type xtype,
	       size_t len, const int *op);

EXTERNL int
nc3_get_att_int(int ncid, int varid, const char *name, int *ip);

EXTERNL int
nc3_put_att_long(int ncid, int varid, const char *name, nc_type xtype,
		size_t len, const long *op);

EXTERNL int
nc3_get_att_long(int ncid, int varid, const char *name, long *ip);

EXTERNL int
nc3_put_att_float(int ncid, int varid, const char *name, nc_type xtype,
		 size_t len, const float *op);

EXTERNL int
nc3_get_att_float(int ncid, int varid, const char *name, float *ip);

EXTERNL int
nc3_put_att_double(int ncid, int varid, const char *name, nc_type xtype,
		  size_t len, const double *op);

EXTERNL int
nc3_get_att_double(int ncid, int varid, const char *name, double *ip);

EXTERNL int
nc3_put_att_ubyte(int ncid, int varid, const char *name, nc_type xtype,
		 size_t len, const unsigned char *op);

EXTERNL int
nc3_get_att_ubyte(int ncid, int varid, const char *name, 
		 unsigned char *ip);

EXTERNL int
nc3_put_att_ushort(int ncid, int varid, const char *name, nc_type xtype,
		  size_t len, const unsigned short *op);

EXTERNL int
nc3_get_att_ushort(int ncid, int varid, const char *name, unsigned short *ip);

EXTERNL int
nc3_put_att_uint(int ncid, int varid, const char *name, nc_type xtype,
		size_t len, const unsigned int *op);

EXTERNL int
nc3_get_att_uint(int ncid, int varid, const char *name, unsigned int *ip);

EXTERNL int
nc3_put_att_longlong(int ncid, int varid, const char *name, nc_type xtype,
		 size_t len, const long long *op);

EXTERNL int
nc3_get_att_longlong(int ncid, int varid, const char *name, long long *ip);

EXTERNL int
nc3_put_att_ulonglong(int ncid, int varid, const char *name, nc_type xtype,
		     size_t len, const unsigned long long *op);

EXTERNL int
nc3_get_att_ulonglong(int ncid, int varid, const char *name, 
		     unsigned long long *ip);

EXTERNL int
nc3_put_att_string(int ncid, int varid, const char *name, 
		  size_t len, const char **op);

EXTERNL int
nc3_get_att_string(int ncid, int varid, const char *name, char **ip);


EXTERNL int
nc3_put_var1_text(int ncid, int varid, const size_t *indexp, const char *op);

EXTERNL int
nc3_get_var1_text(int ncid, int varid, const size_t *indexp, char *ip);

EXTERNL int
nc3_put_var1_uchar(int ncid, int varid, const size_t *indexp,
		  const unsigned char *op);

EXTERNL int
nc3_get_var1_uchar(int ncid, int varid, const size_t *indexp,
		  unsigned char *ip);

EXTERNL int
nc3_put_var1_schar(int ncid, int varid, const size_t *indexp,
		  const signed char *op);

EXTERNL int
nc3_get_var1_schar(int ncid, int varid, const size_t *indexp,
		  signed char *ip);

EXTERNL int
nc3_put_var1_short(int ncid, int varid, const size_t *indexp,
		  const short *op);

EXTERNL int
nc3_get_var1_short(int ncid, int varid, const size_t *indexp,
		  short *ip);

EXTERNL int
nc3_put_var1_int(int ncid, int varid, const size_t *indexp, const int *op);

EXTERNL int
nc3_get_var1_int(int ncid, int varid, const size_t *indexp, int *ip);

EXTERNL int
nc3_put_var1_long(int ncid, int varid, const size_t *indexp, const long *op);

EXTERNL int
nc3_get_var1_long(int ncid, int varid, const size_t *indexp, long *ip);

EXTERNL int
nc3_put_var1_float(int ncid, int varid, const size_t *indexp, const float *op);

EXTERNL int
nc3_get_var1_float(int ncid, int varid, const size_t *indexp, float *ip);

EXTERNL int
nc3_put_var1_double(int ncid, int varid, const size_t *indexp, const double *op);

EXTERNL int
nc3_get_var1_double(int ncid, int varid, const size_t *indexp, double *ip);

EXTERNL int
nc3_put_var1_ubyte(int ncid, int varid, const size_t *indexp, 
		  const unsigned char *op);

EXTERNL int
nc3_get_var1_ubyte(int ncid, int varid, const size_t *indexp, 
		  unsigned char *ip);

EXTERNL int
nc3_put_var1_ushort(int ncid, int varid, const size_t *indexp, 
		   const unsigned short *op);

EXTERNL int
nc3_get_var1_ushort(int ncid, int varid, const size_t *indexp, 
		   unsigned short *ip);

EXTERNL int
nc3_put_var1_uint(int ncid, int varid, const size_t *indexp, 
		 const unsigned int *op);

EXTERNL int
nc3_get_var1_uint(int ncid, int varid, const size_t *indexp, 
		 unsigned int *ip);

EXTERNL int
nc3_put_var1_longlong(int ncid, int varid, const size_t *indexp, 
		     const long long *op);

EXTERNL int
nc3_get_var1_longlong(int ncid, int varid, const size_t *indexp, 
		  long long *ip);

EXTERNL int
nc3_put_var1_ulonglong(int ncid, int varid, const size_t *indexp, 
		   const unsigned long long *op);

EXTERNL int
nc3_get_var1_ulonglong(int ncid, int varid, const size_t *indexp, 
		   unsigned long long *ip);

EXTERNL int
nc3_put_var1_string(int ncid, int varid, const size_t *indexp, 
		   const char **op);

EXTERNL int
nc3_get_var1_string(int ncid, int varid, const size_t *indexp, 
		   char **ip);

/* End {put,get}_var1 */
/* Begin {put,get}_vara */

EXTERNL int
nc3_put_vara_text(int ncid, int varid,
	const size_t *startp, const size_t *countp, const char *op);

EXTERNL int
nc3_get_vara_text(int ncid, int varid,
	const size_t *startp, const size_t *countp, char *ip);

EXTERNL int
nc3_put_vara_uchar(int ncid, int varid,
	const size_t *startp, const size_t *countp, const unsigned char *op);

EXTERNL int
nc3_get_vara_uchar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, unsigned char *ip);

EXTERNL int
nc3_put_vara_schar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const signed char *op);

EXTERNL int
nc3_get_vara_schar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, signed char *ip);

EXTERNL int
nc3_put_vara_short(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const short *op);

EXTERNL int
nc3_get_vara_short(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, short *ip);

EXTERNL int
nc3_put_vara_int(int ncid, int varid, const size_t *startp, 
		const size_t *countp, const int *op);

EXTERNL int
nc3_get_vara_int(int ncid, int varid, const size_t *startp, 
		const size_t *countp, int *ip);

EXTERNL int
nc3_put_vara_long(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const long *op);

EXTERNL int
nc3_get_vara_long(int ncid, int varid,
	const size_t *startp, const size_t *countp, long *ip);

EXTERNL int
nc3_put_vara_float(int ncid, int varid,
	const size_t *startp, const size_t *countp, const float *op);

EXTERNL int
nc3_get_vara_float(int ncid, int varid,
	const size_t *startp, const size_t *countp, float *ip);

EXTERNL int
nc3_put_vara_double(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const double *op);

EXTERNL int
nc3_get_vara_double(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, double *ip);

EXTERNL int
nc3_put_vara_ubyte(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const unsigned char *op);

EXTERNL int
nc3_get_vara_ubyte(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, unsigned char *ip);

EXTERNL int
nc3_put_vara_ushort(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const unsigned short *op);

EXTERNL int
nc3_get_vara_ushort(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, unsigned short *ip);

EXTERNL int
nc3_put_vara_uint(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const unsigned int *op);

EXTERNL int
nc3_get_vara_uint(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, unsigned int *ip);

EXTERNL int
nc3_put_vara_longlong(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const long long *op);

EXTERNL int
nc3_get_vara_longlong(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, long long *ip);

EXTERNL int
nc3_put_vara_ulonglong(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const unsigned long long *op);

EXTERNL int
nc3_get_vara_ulonglong(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, unsigned long long *ip);

EXTERNL int
nc3_put_vara_string(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const char **op);

EXTERNL int
nc3_get_vara_string(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, char **ip);

/* End {put,get}_vara */
/* Begin {put,get}_vars */

EXTERNL int
nc3_put_vars_text(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	const char *op);

EXTERNL int
nc3_get_vars_text(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	char *ip);

EXTERNL int
nc3_put_vars_uchar(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	const unsigned char *op);

EXTERNL int
nc3_get_vars_uchar(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	unsigned char *ip);

EXTERNL int
nc3_put_vars_schar(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	const signed char *op);

EXTERNL int
nc3_get_vars_schar(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	signed char *ip);

EXTERNL int
nc3_put_vars_short(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	const short *op);

EXTERNL int
nc3_get_vars_short(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep,
		  short *ip);

EXTERNL int
nc3_put_vars_int(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	const int *op);

EXTERNL int
nc3_get_vars_int(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	int *ip);

EXTERNL int
nc3_put_vars_long(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	const long *op);

EXTERNL int
nc3_get_vars_long(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	long *ip);

EXTERNL int
nc3_put_vars_float(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	const float *op);

EXTERNL int
nc3_get_vars_float(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	float *ip);

EXTERNL int
nc3_put_vars_double(int ncid, int varid,
	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
	const double *op);

EXTERNL int
nc3_get_vars_double(int ncid, int varid,	const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep,
		   double *ip);

EXTERNL int
nc3_put_vars_ubyte(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep, 
		  const unsigned char *op);

EXTERNL int
nc3_get_vars_ubyte(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep, 
		  unsigned char *ip);

EXTERNL int
nc3_put_vars_ushort(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   const unsigned short *op);

EXTERNL int
nc3_get_vars_ushort(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   unsigned short *ip);

EXTERNL int
nc3_put_vars_uint(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const ptrdiff_t *stridep, 
		 const unsigned int *op);

EXTERNL int
nc3_get_vars_uint(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const ptrdiff_t *stridep, 
		 unsigned int *ip);

EXTERNL int
nc3_put_vars_longlong(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep, 
		  const long long *op);

EXTERNL int
nc3_get_vars_longlong(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep, 
		  long long *ip);

EXTERNL int
nc3_put_vars_ulonglong(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   const unsigned long long *op);

EXTERNL int
nc3_get_vars_ulonglong(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   unsigned long long *ip);

EXTERNL int
nc3_put_vars_string(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   const char **op);

EXTERNL int
nc3_get_vars_string(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   char **ip);

/* End {put,get}_vars */
/* Begin {put,get}_varm */

EXTERNL int
nc3_put_varm_text(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const ptrdiff_t *stridep,
		 const ptrdiff_t *imapp, const char *op);

EXTERNL int
nc3_get_varm_text(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const ptrdiff_t *stridep,
		 const ptrdiff_t *imapp, char *ip);

EXTERNL int
nc3_put_varm_uchar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep,
		  const ptrdiff_t *imapp, const unsigned char *op);

EXTERNL int
nc3_get_varm_uchar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep,
		  const ptrdiff_t *imapp, unsigned char *ip);

EXTERNL int
nc3_put_varm_schar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep,
		  const ptrdiff_t *imapp, const signed char *op);

EXTERNL int
nc3_get_varm_schar(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep,
		  const ptrdiff_t *imapp, signed char *ip);

EXTERNL int
nc3_put_varm_short(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep,
		  const ptrdiff_t *imapp, const short *op);

EXTERNL int
nc3_get_varm_short(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep,
		  const ptrdiff_t *imapp, short *ip);

EXTERNL int
nc3_put_varm_int(int ncid, int varid, const size_t *startp, 
		const size_t *countp, const ptrdiff_t *stridep,
		const ptrdiff_t *imapp, const int *op);

EXTERNL int
nc3_get_varm_int(int ncid, int varid, const size_t *startp, 
		const size_t *countp, const ptrdiff_t *stridep,
		const ptrdiff_t *imapp, int *ip);

EXTERNL int
nc3_put_varm_long(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const ptrdiff_t *stridep,
		 const ptrdiff_t *imapp, const long *op);

EXTERNL int
nc3_get_varm_long(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const ptrdiff_t *stridep,
		 const ptrdiff_t *imapp, long *ip);

EXTERNL int
nc3_put_varm_float(int ncid, int varid,const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep,
		  const ptrdiff_t *imapp, const float *op);

EXTERNL int
nc3_get_varm_float(int ncid, int varid,const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep,
		  const ptrdiff_t *imapp, float *ip);

EXTERNL int
nc3_put_varm_double(int ncid, int varid,	const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep,
		   const ptrdiff_t *imapp, const double *op);

EXTERNL int
nc3_get_varm_double(int ncid, int varid,	const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep,
		   const ptrdiff_t * imapp, double *ip);

EXTERNL int
nc3_put_varm_ubyte(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep, 
		  const ptrdiff_t * imapp, const unsigned char *op);

EXTERNL int
nc3_get_varm_ubyte(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep, 
		  const ptrdiff_t * imapp, unsigned char *ip);

EXTERNL int
nc3_put_varm_ushort(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   const ptrdiff_t * imapp, const unsigned short *op);

EXTERNL int
nc3_get_varm_ushort(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   const ptrdiff_t * imapp, unsigned short *ip);

EXTERNL int
nc3_put_varm_uint(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const ptrdiff_t *stridep, 
		 const ptrdiff_t * imapp, const unsigned int *op);

EXTERNL int
nc3_get_varm_uint(int ncid, int varid, const size_t *startp, 
		 const size_t *countp, const ptrdiff_t *stridep, 
		 const ptrdiff_t * imapp, unsigned int *ip);

EXTERNL int
nc3_put_varm_longlong(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep, 
		  const ptrdiff_t * imapp, const long long *op);

EXTERNL int
nc3_get_varm_longlong(int ncid, int varid, const size_t *startp, 
		  const size_t *countp, const ptrdiff_t *stridep, 
		  const ptrdiff_t * imapp, long long *ip);

EXTERNL int
nc3_put_varm_ulonglong(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   const ptrdiff_t * imapp, const unsigned long long *op);

EXTERNL int
nc3_get_varm_ulonglong(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   const ptrdiff_t * imapp, unsigned long long *ip);

EXTERNL int
nc3_put_varm_string(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   const ptrdiff_t * imapp, const char **op);

EXTERNL int
nc3_get_varm_string(int ncid, int varid, const size_t *startp, 
		   const size_t *countp, const ptrdiff_t *stridep, 
		   const ptrdiff_t * imapp, char **ip);

/* End {put,get}_varm */
/* Begin {put,get}_var */

EXTERNL int
nc3_put_var_text(int ncid, int varid, const char *op);

EXTERNL int
nc3_get_var_text(int ncid, int varid, char *ip);

EXTERNL int
nc3_put_var_uchar(int ncid, int varid, const unsigned char *op);

EXTERNL int
nc3_get_var_uchar(int ncid, int varid, unsigned char *ip);

EXTERNL int
nc3_put_var_schar(int ncid, int varid, const signed char *op);

EXTERNL int
nc3_get_var_schar(int ncid, int varid, signed char *ip);

EXTERNL int
nc3_put_var_short(int ncid, int varid, const short *op);

EXTERNL int
nc3_get_var_short(int ncid, int varid, short *ip);

EXTERNL int
nc3_put_var_int(int ncid, int varid, const int *op);

EXTERNL int
nc3_get_var_int(int ncid, int varid, int *ip);

EXTERNL int
nc3_put_var_long(int ncid, int varid, const long *op);

EXTERNL int
nc3_get_var_long(int ncid, int varid, long *ip);

EXTERNL int
nc3_put_var_float(int ncid, int varid, const float *op);

EXTERNL int
nc3_get_var_float(int ncid, int varid, float *ip);

EXTERNL int
nc3_put_var_double(int ncid, int varid, const double *op);

EXTERNL int
nc3_get_var_double(int ncid, int varid, double *ip);

EXTERNL int
nc3_put_var_ubyte(int ncid, int varid, const unsigned char *op);

EXTERNL int
nc3_get_var_ubyte(int ncid, int varid, unsigned char *ip);

EXTERNL int
nc3_put_var_ushort(int ncid, int varid, const unsigned short *op);

EXTERNL int
nc3_get_var_ushort(int ncid, int varid, unsigned short *ip);

EXTERNL int
nc3_put_var_uint(int ncid, int varid, const unsigned int *op);

EXTERNL int
nc3_get_var_uint(int ncid, int varid, unsigned int *ip);

EXTERNL int
nc3_put_var_longlong(int ncid, int varid, const long long *op);

EXTERNL int
nc3_get_var_longlong(int ncid, int varid, long long *ip);

EXTERNL int
nc3_put_var_ulonglong(int ncid, int varid, const unsigned long long *op);

EXTERNL int
nc3_get_var_ulonglong(int ncid, int varid, unsigned long long *ip);

EXTERNL int
nc3_put_var_string(int ncid, int varid, const char **op);

EXTERNL int
nc3_get_var_string(int ncid, int varid, char **ip);

EXTERNL int
nc3__create_mp(const char *path, int cmode, size_t initialsz, int basepe,
	 size_t *chunksizehintp, int *ncidp);

EXTERNL int
nc3__open_mp(const char *path, int mode, int basepe,
	size_t *chunksizehintp, int *ncidp);

EXTERNL int
nc3_enddef(int ncid);

#if defined(__cplusplus)
}
#endif

#endif /*_NC3STUB_H*/
