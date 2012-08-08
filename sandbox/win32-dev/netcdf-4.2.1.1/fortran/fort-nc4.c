/*
This file is part of netcdf-4, a netCDF-like interface for HDF5, or a
HDF5 backend for netCDF, depending on your point of view.

This file provides the fortran functions for the new functions added
to the API as part of netCDF-4. This file is only compiled for
netCDF-4 builds.

Copyright 2005, University Corporation for Atmospheric Research. See
COPYRIGHT file for copying and redistribution conditions.

$Id$
*/

#include <config.h>
#include "netcdf_f.h"
#include <ncfortran.h>
#include <fort-lib.h>


FCALLSCFUN5(NF_INT, nc_create_par_fortran, NF_CREATE_PAR, nf_create_par,
	    STRING, FINT2CINT, FINT2CINT, FINT2CINT, PCINT2FINT)

FCALLSCFUN5(NF_INT, nc_open_par_fortran, NF_OPEN_PAR, nf_open_par,
	    STRING, FINT2CINT, FINT2CINT, FINT2CINT, PCINT2FINT)

FCALLSCFUN3(NF_INT, nc_var_par_access, NF_VAR_PAR_ACCESS, nf_var_par_access,
	    NCID, VARID, FINT2CINT)

FCALLSCFUN3(NF_INT, nc_inq_ncid, NF_INQ_NCID, nf_inq_ncid,
	    NCID, STRING, PCINT2FINT)

FCALLSCFUN3(NF_INT, nc_inq_grps, NF_INQ_GRPS, nf_inq_grps,
	    NCID, PCINT2FINT, INTV)

FCALLSCFUN2(NF_INT, nc_inq_grpname, NF_INQ_GRPNAME, nf_inq_grpname,
	    NCID, PSTRING)

FCALLSCFUN3(NF_INT, nc_inq_grpname_full, NF_INQ_GRPNAME_FULL, nf_inq_grpname_full,
	    NCID, PSIZET, PSTRING)

FCALLSCFUN2(NF_INT, nc_inq_grpname_len, NF_INQ_GRPNAME_LEN, nf_inq_grpname_len,
	    NCID, PSIZET)

FCALLSCFUN2(NF_INT, nc_inq_grp_parent, NF_INQ_GRP_PARENT, nf_inq_grp_parent,
	    NCID, PCINT2FINT)

FCALLSCFUN3(NF_INT, nc_inq_grp_ncid, NF_INQ_GRP_NCID, nf_inq_grp_ncid,
	    NCID, STRING, PCINT2FINT)

FCALLSCFUN3(NF_INT, nc_inq_grp_full_ncid, NF_INQ_GRP_FULL_NCID, nf_inq_grp_full_ncid,
	    NCID, STRING, PCINT2FINT)

FCALLSCFUN3(NF_INT, nc_inq_varids_f, NF_INQ_VARIDS, nf_inq_varids,
	    NCID, PCINT2FINT, INTV)

FCALLSCFUN4(NF_INT, nc_inq_dimids_f, NF_INQ_DIMIDS, nf_inq_dimids,
	    NCID, PCINT2FINT, INTV, FINT2CINT)

FCALLSCFUN3(NF_INT, nc_inq_typeids, NF_INQ_TYPEIDS, nf_inq_typeids,
	    NCID, PCINT2FINT, INTV)

FCALLSCFUN3(NF_INT, nc_inq_typeid, NF_INQ_TYPEID, nf_inq_typeid,
	    NCID, STRING, PCINT2FINT)

FCALLSCFUN3(NF_INT, nc_def_grp, NF_DEF_GRP, nf_def_grp,
	    NCID, STRING, PCINT2FINT)

FCALLSCFUN4(NF_INT, nc_def_compound, NF_DEF_COMPOUND, nf_def_compound,
	    NCID, FINT2CSIZET, STRING, PCINT2FINT)

FCALLSCFUN5(NF_INT, nc_insert_compound, NF_INSERT_COMPOUND, nf_insert_compound,
	    NCID, FINT2CINT, STRING, FINT2CSIZET, FINT2CINT)

FCALLSCFUN7(NF_INT, nc_insert_array_compound_f, NF_INSERT_ARRAY_COMPOUND, 
	    nf_insert_array_compound,
	    NCID, FINT2CINT, STRING, FINT2CSIZET, 
	    FINT2CINT, FINT2CINT, INTV)

FCALLSCFUN4(NF_INT, nc_inq_type, NF_INQ_TYPE, nf_inq_type,
	    NCID, FINT2CINT, STRING, PSIZET)

FCALLSCFUN5(NF_INT, nc_inq_compound, NF_INQ_COMPOUND, nf_inq_compound,
	    NCID, FINT2CINT, STRING, PSIZET, PSIZET)

FCALLSCFUN3(NF_INT, nc_inq_compound_name, NF_INQ_COMPOUND_NAME, nf_inq_compound_name,
	    NCID, FINT2CINT, STRING)

FCALLSCFUN3(NF_INT, nc_inq_compound_size, NF_INQ_COMPOUND_SIZE, nf_inq_compound_size,
	    NCID, FINT2CINT, PSIZET)

FCALLSCFUN3(NF_INT, nc_inq_compound_nfields, NF_INQ_COMPOUND_NFIELDS, 
	    nf_inq_compound_nfields,
	    NCID, FINT2CINT, PSIZET)

FCALLSCFUN8(NF_INT, nc_inq_compound_field_f, NF_INQ_COMPOUND_FIELD, 
	    nf_inq_compound_field,
	    NCID, FINT2CINT, VARID, PSTRING, PSIZET, PCINT2FINT, 
	    PCINT2FINT, INTV)

FCALLSCFUN4(NF_INT, nc_inq_compound_fieldname, NF_INQ_COMPOUND_FIELDNAME, 
	    nf_inq_compound_fieldname,
	    NCID, FINT2CINT, FIELDIDX, PSTRING)

FCALLSCFUN4(NF_INT, nc_inq_compound_fieldindex, NF_INQ_COMPOUND_FIELDINDEX, 
	    nf_inq_compound_fieldindex,
	    NCID, FINT2CINT, STRING, PCNDX2FNDX)

FCALLSCFUN4(NF_INT, nc_inq_compound_fieldoffset, NF_INQ_COMPOUND_FIELDOFFSET, 
	    nf_inq_compound_fieldoffset,
	    NCID, FINT2CINT, FIELDIDX, PSIZET)

FCALLSCFUN4(NF_INT, nc_inq_compound_fieldtype, NF_INQ_COMPOUND_FIELDTYPE, 
	    nf_inq_compound_fieldtype,
	    NCID, FINT2CINT, FIELDIDX, PCINT2FINT)

FCALLSCFUN4(NF_INT, nc_inq_compound_fieldndims, NF_INQ_COMPOUND_FIELDNDIMS, 
	    nf_inq_compound_fieldndims,
	    NCID, FINT2CINT, FIELDIDX, PCINT2FINT)

FCALLSCFUN4(NF_INT, nc_inq_compound_fielddim_sizes, NF_INQ_COMPOUND_FIELDDIM_SIZES, 
	    nf_inq_compound_fielddim_sizes,
	    NCID, FINT2CINT, FIELDIDX, PCINT2FINT)

FCALLSCFUN4(NF_INT, nc_def_vlen, NF_DEF_VLEN, nf_def_vlen,
	    NCID, STRING, FINT2CINT, PCINT2FINT)

FCALLSCFUN5(NF_INT, nc_inq_vlen, NF_INQ_VLEN, nf_inq_vlen,
	    NCID, FINT2CINT, PSTRING, PSIZET, PCINT2FINT)

FCALLSCFUN7(NF_INT, nc_inq_user_type, NF_INQ_USER_TYPE, nf_inq_user_type,
	    NCID, FINT2CINT, PSTRING, PSIZET, PCINT2FINT, PSIZET, PCINT2FINT)

FCALLSCFUN4(NF_INT, nc_def_enum, NF_DEF_ENUM, nf_def_enum,
	    NCID, FINT2CINT, STRING, PCINT2FINT)

FCALLSCFUN4(NF_INT, nc_insert_enum, NF_INSERT_ENUM, nf_insert_enum,
	    NCID, FINT2CINT, STRING, PVOID)

FCALLSCFUN6(NF_INT, nc_inq_enum, NF_INQ_ENUM, nf_inq_enum,
	    NCID, FINT2CINT, STRING, PCINT2FINT, PSIZET, PSIZET)

FCALLSCFUN5(NF_INT, nc_inq_enum_member, NF_INQ_ENUM_MEMBER, nf_inq_enum_member,
	    NCID, FINT2CINT, FNDX2CNDX, PSTRING, PVOID)

FCALLSCFUN4(NF_INT, nc_inq_enum_ident, NF_INQ_ENUM_IDENT, nf_inq_enum_ident,
	    NCID, FINT2CINT, FINT2CINT, PSTRING)

FCALLSCFUN4(NF_INT, nc_def_opaque, NF_DEF_OPAQUE, nf_def_opaque,
	    NCID, FINT2CSIZET, STRING, PCINT2FINT)

FCALLSCFUN4(NF_INT, nc_inq_opaque, NF_INQ_OPAQUE, nf_inq_opaque,
	    NCID, FINT2CSIZET, STRING, PSIZET)

FCALLSCFUN4(NF_INT, nc_def_var_chunking_ints, NF_DEF_VAR_CHUNKING, nf_def_var_chunking,
	    NCID, VARID, FINT2CINT, CHUNKSIZES)

FCALLSCFUN5(NF_INT, nc_def_var_deflate, NF_DEF_VAR_DEFLATE, nf_def_var_deflate,
	    NCID, VARID, FINT2CINT, FINT2CINT, FINT2CINT)

FCALLSCFUN4(NF_INT, nc_def_var_fill, NF_DEF_VAR_FILL, nf_def_var_fill,
	    NCID, VARID, FINT2CINT, PVOID)

FCALLSCFUN4(NF_INT, nc_inq_var_fill, NF_INQ_VAR_FILL, nf_inq_var_fill,
	    NCID, VARID, PCINT2FINT, PVOID)

FCALLSCFUN3(NF_INT, nc_def_var_fletcher32, NF_DEF_VAR_FLETCHER32, nf_def_var_fletcher32,
	    NCID, VARID, FINT2CINT)

FCALLSCFUN4(NF_INT, nc_inq_var_chunking_ints, NF_INQ_VAR_CHUNKING, nf_inq_var_chunking,
	    NCID, VARID, PCINT2FINT, PCHUNKSIZES)

FCALLSCFUN5(NF_INT, nc_inq_var_deflate, NF_INQ_VAR_DEFLATE, nf_inq_var_deflate,
	    NCID, VARID, PCINT2FINT, PCINT2FINT, PCINT2FINT)

FCALLSCFUN4(NF_INT, nc_inq_var_szip, NF_INQ_VAR_SZIP, nf_inq_var_szip,
	    NCID, VARID, PCINT2FINT, PCINT2FINT)

FCALLSCFUN3(NF_INT, nc_inq_var_fletcher32, NF_INQ_VAR_FLETCHER32, nf_inq_var_fletcher32,
	    NCID, VARID, PCINT2FINT)

FCALLSCFUN3(NF_INT, nc_def_var_endian, NF_DEF_VAR_ENDIAN, nf_def_var_endian,
	    NCID, VARID, FINT2CINT)

FCALLSCFUN3(NF_INT, nc_inq_var_endian, NF_INQ_VAR_ENDIAN, nf_inq_var_endian,
	    NCID, VARID, PCINT2FINT)

FCALLSCFUN6(NF_INT, nc_put_att, NF_PUT_ATT, nf_put_att,
	    NCID, VARID, STRING, FINT2CINT, FINT2CSIZET, PVOID)

FCALLSCFUN4(NF_INT, nc_get_att, NF_GET_ATT, nf_get_att,
	    NCID, VARID, STRING, PVOID)

FCALLSCFUN5(NF_INT, nc_put_vlen_element, NF_PUT_VLEN_ELEMENT, nf_put_vlen_element,
	    NCID, FINT2CINT, PVOID, FINT2CSIZET, PVOID)

FCALLSCFUN5(NF_INT, nc_get_vlen_element, NF_GET_VLEN_ELEMENT, nf_get_vlen_element,
	    NCID, FINT2CINT, PVOID, PSIZET, PVOID)

FCALLSCFUN1(NF_INT, nc_free_vlen, NF_FREE_VLEN, nf_free_vlen, PVOID)
FCALLSCFUN2(NF_INT, nc_free_vlens, NF_FREE_VLENS, nf_free_vlens, FINT2CSIZET, PVOID)
FCALLSCFUN2(NF_INT, nc_free_string, NF_FREE_STRING, nf_free_string, FINT2CSIZET, PVOID)

FCALLSCFUN4(NF_INT, nc_put_var1_longlong, NF_PUT_VAR1_INT64, nf_put_var1_int64,
	    NCID, VARID, COORDS, INTVAR)
FCALLSCFUN5(NF_INT, nc_put_vara_longlong, NF_PUT_VARA_INT64, nf_put_vara_int64,
	    NCID, VARID, COORDS, COUNTS, INTVARV)
FCALLSCFUN6(NF_INT, nc_put_vars_longlong, NF_PUT_VARS_INT64, nf_put_vars_int64,
	    NCID, VARID, COORDS, COUNTS, STRIDES, INTVARV)
FCALLSCFUN7(NF_INT, nc_put_varm_longlong, NF_PUT_VARM_INT64, nf_put_varm_int64,
            NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PINTVARV)
FCALLSCFUN3(NF_INT, nc_put_var_longlong, NF_PUT_VAR_INT64, nf_put_var_int64,
	    NCID, VARID, PINTVAR)

FCALLSCFUN4(NF_INT, nc_get_var1_longlong, NF_GET_VAR1_INT64, nf_get_var1_int64,
	    NCID, VARID, COORDS, INTVAR)
FCALLSCFUN5(NF_INT, nc_get_vara_longlong, NF_GET_VARA_INT64, nf_get_vara_int64,
	    NCID, VARID, COORDS, COUNTS, INTVARV)
FCALLSCFUN6(NF_INT, nc_get_vars_longlong, NF_GET_VARS_INT64, nf_get_vars_int64,
	    NCID, VARID, COORDS, COUNTS, STRIDES, INTVARV)
FCALLSCFUN7(NF_INT, nc_get_varm_longlong, NF_GET_VARM_INT64, nf_get_varm_int64,
            NCID, VARID, COORDS, COUNTS, STRIDES, MAPS, PINTVARV)
FCALLSCFUN3(NF_INT, nc_get_var_longlong, NF_GET_VAR_INT64, nf_get_var_int64,
	    NCID, VARID, PINTVAR)

FCALLSCFUN3(NF_INT, nc_set_chunk_cache_ints, NF_SET_CHUNK_CACHE, nf_set_chunk_cache,
	    INT, INT, INT)
FCALLSCFUN3(NF_INT, nc_get_chunk_cache_ints, NF_GET_CHUNK_CACHE, nf_get_chunk_cache,
	    PINT, PINT, PINT)

FCALLSCFUN5(NF_INT, nc_set_var_chunk_cache_ints, NF_SET_VAR_CHUNK_CACHE, nf_set_var_chunk_cache,
	    NCID, VARID, INT, INT, INT)
FCALLSCFUN5(NF_INT, nc_get_var_chunk_cache_ints, NF_GET_VAR_CHUNK_CACHE, nf_get_var_chunk_cache,
	    NCID, VARID, PINT, PINT, PINT)




