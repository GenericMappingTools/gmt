! This is part of the netCDF-4 fortran 90 API.
! Copyright 2006, UCAR
! $Id$

  ! Extra netCDF-4 functions 

  integer, external :: nf_create_par, nf_open_par, nf_var_par_access, &
       nf_inq_ncid, nf_inq_grps, nf_inq_grpname, nf_inq_grpname_full, &
       nf_inq_grpname_len, nf_inq_grp_parent, nf_inq_grp_ncid, nf_inq_grp_full_ncid, nf_inq_varids, &
       nf_inq_dimids, nf_inq_typeids, nf_inq_typeid, nf_def_grp, nf_def_compound, &
       nf_insert_compound, nf_insert_array_compound, nf_inq_type, &
       nf_inq_compound, nf_inq_compound_name, nf_inq_compound_size, &
       nf_inq_compound_nfields, nf_inq_compound_field, &
       nf_inq_compound_fieldname, nf_inq_compound_fieldindex, &
       nf_inq_compound_fieldtype, nf_inq_compound_fieldndims, &
       nf_inq_compound_fielddim_sizes, nf_inq_compound_fieldoffset, &
       nf_def_vlen, nf_inq_vlen, nf_free_vlen, nf_inq_user_type, &
       nf_def_enum, nf_insert_enum, nf_inq_enum, nf_inq_enum_member, &
       nf_inq_enum_ident, nf_def_opaque, nf_inq_opaque, &
       nf_def_var_chunking, nf_def_var_deflate, &
       nf_def_var_fletcher32, nf_inq_var_chunking, nf_inq_var_deflate, &
       nf_inq_var_fletcher32, nf_inq_var_endian, nf_def_var_endian, &
       nf_def_var_fill, nf_inq_var_fill, nf_get_att, nf_put_att, &
       nf_put_vars, nf_get_vars, nf_put_vlen_element, &
       nf_put_var1_int64, nf_put_vara_int64, nf_put_vars_int64, &
       nf_put_varm_int64, nf_put_var_int64, nf_get_var1_int64, &
       nf_get_vara_int64, nf_get_vars_int64, nf_get_varm_int64, &
       nf_get_var_int64, nf_get_chunk_cache, nf_set_chunk_cache, &
       nf_inq_var_szip, nf_free_vlens, nf_free_string, &
       nf_set_var_chunk_cache, nf_get_var_chunk_cache

  
