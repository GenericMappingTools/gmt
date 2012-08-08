  !
  ! NetCDF-4 extra routines:
  !
  ! -----------
  function nf90_create_par(path, cmode, comm, info, ncid, cache_size, &
       cache_nelems, cache_preemption)
    character (len = *), intent(in) :: path
    integer, intent(in) :: cmode
    integer, intent(in) :: comm
    integer, intent(in) :: info
    integer, intent(out) :: ncid
    integer, optional, intent(in) :: cache_size, cache_nelems
    real, optional, intent(in) :: cache_preemption
    integer :: size_in, nelems_in, preemption_in
    integer :: size_out, nelems_out, preemption_out, ret
    integer :: nf90_create_par
    
    ! If the user specified chuck cache parameters, use them. But user
    ! may have specified one, two, or three settings. Leave the others
    ! unchanged.
    if (present(cache_size) .or. present(cache_nelems) .or. &
         present(cache_preemption)) then
       ret = nf_get_chunk_cache(size_in, nelems_in, preemption_in)
       if (ret .ne. nf90_noerr) then
          nf90_create_par = ret
          return
       end if
       if (present(cache_size)) then
          size_out = cache_size
       else
          size_out = size_in
       end if
       if (present(cache_nelems)) then
          nelems_out = cache_nelems
       else
          nelems_out = nelems_in
       end if
       if (present(cache_preemption)) then
          preemption_out = cache_preemption
       else
          preemption_out = preemption_in
       end if
       nf90_create_par = nf_set_chunk_cache(size_out, nelems_out, preemption_out)
       if (nf90_create_par .ne. nf90_noerr) return
    end if 

    nf90_create_par = nf_create_par(path, cmode, comm, info, ncid)
  end function nf90_create_par
  ! -----------
  function nf90_open_par(path, cmode, comm, info, ncid, cache_size, &
       cache_nelems, cache_preemption)
    character (len = *), intent(in) :: path
    integer, intent(in) :: cmode
    integer, intent(in) :: comm
    integer, intent(in) :: info
    integer, intent(out) :: ncid
    integer, optional, intent(in) :: cache_size, cache_nelems
    real, optional, intent(in) :: cache_preemption
    integer :: size_in, nelems_in, preemption_in
    integer :: size_out, nelems_out, preemption_out, ret
    integer :: nf90_open_par
  
    ! If the user specified chuck cache parameters, use them. But user
    ! may have specified one, two, or three settings. Leave the others
    ! unchanged.
    if (present(cache_size) .or. present(cache_nelems) .or. &
         present(cache_preemption)) then
       ret = nf_get_chunk_cache(size_in, nelems_in, preemption_in)
       if (ret .ne. nf90_noerr) then
          nf90_open_par = ret
          return
       end if
       if (present(cache_size)) then
          size_out = cache_size
       else
          size_out = size_in
       end if
       if (present(cache_nelems)) then
          nelems_out = cache_nelems
       else
          nelems_out = nelems_in
       end if
       if (present(cache_preemption)) then
          preemption_out = cache_preemption
       else
          preemption_out = preemption_in
       end if
       nf90_open_par = nf_set_chunk_cache(size_out, nelems_out, preemption_out)
       if (nf90_open_par .ne. nf90_noerr) return
    end if 

    nf90_open_par = nf_open_par(path, cmode, comm, info, ncid)
  end function nf90_open_par
  ! -----------
  function nf90_var_par_access(ncid, varid, access)
    integer, intent(in) :: ncid
    integer, intent(in) :: varid
    integer, intent(in) :: access
    integer :: nf90_var_par_access
  
    nf90_var_par_access = nf_var_par_access(ncid, varid, access)
  end function nf90_var_par_access
  ! -----------
  function nf90_inq_ncid(ncid, name, grp_ncid)
    integer, intent(in) :: ncid
    character (len = *), intent(in) :: name
    integer, intent(out) :: grp_ncid
    integer :: nf90_inq_ncid
  
    nf90_inq_ncid = nf_inq_ncid(ncid, name, grp_ncid)
  end function nf90_inq_ncid
  ! -----------
  function nf90_inq_grps(ncid, numgrps, ncids)
    integer, intent(in) :: ncid
    integer, intent(out) :: numgrps
    integer, dimension(:), intent(out) :: ncids
    integer :: nf90_inq_grps
  
    nf90_inq_grps = nf_inq_grps(ncid, numgrps, ncids)
  end function nf90_inq_grps
  ! -----------
  function nf90_inq_grpname_len(ncid, len)
    integer, intent(in) :: ncid
    integer, intent(out) :: len
    integer :: nf90_inq_grpname_len
  
    nf90_inq_grpname_len = nf_inq_grpname_len(ncid, len)
  end function nf90_inq_grpname_len
  ! -----------
  function nf90_inq_grp_ncid(ncid, name, grpid)
    integer, intent(in) :: ncid
    character (len = *), intent(in) :: name
    integer, intent(out) :: grpid
    integer :: nf90_inq_grp_ncid
  
    nf90_inq_grp_ncid = nf_inq_grp_ncid(ncid, name, grpid)
  end function nf90_inq_grp_ncid
  ! -----------
  function nf90_inq_grp_full_ncid(ncid, full_name, grpid)
    integer, intent(in) :: ncid
    character (len = *), intent(in) :: full_name
    integer, intent(out) :: grpid
    integer :: nf90_inq_grp_full_ncid
  
    nf90_inq_grp_full_ncid = nf_inq_grp_full_ncid(ncid, full_name, grpid)
  end function nf90_inq_grp_full_ncid
  ! -----------
  function nf90_inq_grp_parent(ncid, parent_ncid)
    integer, intent(in) :: ncid
    integer, intent(out) :: parent_ncid
    integer :: nf90_inq_grp_parent
  
    nf90_inq_grp_parent = nf_inq_grp_parent(ncid, parent_ncid)
  end function nf90_inq_grp_parent
  ! -----------
  function nf90_inq_grpname(ncid, name)
    integer, intent(in) :: ncid
    character (len = *), intent(out) :: name
    integer :: nf90_inq_grpname
  
    nf90_inq_grpname = nf_inq_grpname(ncid, name)
  end function nf90_inq_grpname
  ! -----------
  function nf90_inq_grpname_full(ncid, len, name)
    integer, intent(in) :: ncid
    integer, intent(out) :: len
    character (len = *), intent(out) :: name
    integer :: nf90_inq_grpname_full
  
    nf90_inq_grpname_full = nf_inq_grpname_full(ncid, len, name)
  end function nf90_inq_grpname_full
  ! -----------
  function nf90_inq_varids(ncid, nvars, varids)
    integer, intent(in) :: ncid
    integer, intent(out) :: nvars
    integer, dimension(:), intent(out) :: varids
    integer :: nf90_inq_varids
  
    nf90_inq_varids = nf_inq_varids(ncid, nvars, varids)
  end function nf90_inq_varids
  ! -----------
  function nf90_inq_dimids(ncid, ndims, dimids, include_parents)
    integer, intent(in) :: ncid
    integer, intent(out) :: ndims
    integer, dimension(:), intent(out) :: dimids
    integer, intent(out) :: include_parents
    integer :: nf90_inq_dimids
  
    nf90_inq_dimids = nf_inq_dimids(ncid, ndims, dimids, include_parents)
  end function nf90_inq_dimids
  ! -----------
  function nf90_inq_typeids(ncid, ntypes, typeids)
    integer, intent(in) :: ncid
    integer, optional, intent(out) :: ntypes
    integer, dimension(:), optional, intent(out) :: typeids
    integer :: nf90_inq_typeids
    
    nf90_inq_typeids = nf_inq_typeids(ncid, ntypes, typeids)

  end function nf90_inq_typeids
  ! -----------
  function nf90_inq_typeid(ncid, name, typeid)
    integer, intent(in) :: ncid
    character (len = *), intent(in) :: name
    integer, optional, intent(out) :: typeid
    integer :: nf90_inq_typeid
    
    nf90_inq_typeid = nf_inq_typeid(ncid, name, typeid)

  end function nf90_inq_typeid
  ! -----------
  function nf90_def_grp(parent_ncid, name, new_ncid)
    integer, intent(in) :: parent_ncid
    character (len = *), intent(in) :: name
    integer, intent(out) :: new_ncid
    integer :: nf90_def_grp
  
    nf90_def_grp = nf_def_grp(parent_ncid, name, new_ncid)
  end function nf90_def_grp
  ! -----------
  function nf90_def_compound(ncid, size, name, typeid)
    integer, intent(in) :: ncid
    integer, intent(in) :: size
    character (len = *), intent(in) :: name
    integer, intent(out) :: typeid
    integer :: nf90_def_compound
  
    nf90_def_compound = nf_def_compound(ncid, size, name, typeid)
  end function nf90_def_compound
  ! -----------
  function nf90_insert_compound(ncid, xtype, name, offset, field_typeid)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    character (len = *), intent(in) :: name
    integer, intent(in) :: offset
    integer, intent(in) :: field_typeid
    integer :: nf90_insert_compound
  
    nf90_insert_compound = nf_insert_compound(ncid, xtype, name, offset, field_typeid)
  end function nf90_insert_compound
  ! -----------
  function nf90_insert_array_compound(ncid, xtype, name, offset, field_typeid, &
       ndims, dim_sizes)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    character (len = *), intent(in) :: name
    integer, intent(in) :: offset
    integer, intent(in) :: field_typeid
    integer, intent(in) :: ndims
    integer, intent(in) :: dim_sizes
    integer :: nf90_insert_array_compound
  
    nf90_insert_array_compound = nf_insert_array_compound(ncid, xtype, name, &
         offset, field_typeid, ndims, dim_sizes)
  end function nf90_insert_array_compound
  ! -----------
  function nf90_inq_type(ncid, xtype, name, size, nfields)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    character (len = *), intent(out) :: name
    integer, intent(out) :: size
    integer, intent(out) :: nfields
    integer :: nf90_inq_type
  
    nf90_inq_type = nf_inq_type(ncid, xtype, name, size, nfields)
  end function nf90_inq_type
  ! -----------
  function nf90_inq_compound(ncid, xtype, name, size, nfields)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    character (len = *), intent(out) :: name
    integer, intent(out) :: size
    integer, intent(out) :: nfields
    integer :: nf90_inq_compound
  
    nf90_inq_compound = nf_inq_compound(ncid, xtype, name, size, nfields)
  end function nf90_inq_compound
  ! -----------
  function nf90_inq_compound_name(ncid, xtype, name)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    character (len = *), intent(out) :: name
    integer :: nf90_inq_compound_name
  
    nf90_inq_compound_name = nf_inq_compound_name(ncid, xtype, name)
  end function nf90_inq_compound_name
  ! -----------
  function nf90_inq_compound_size(ncid, xtype, size)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    integer, intent(out) :: size
    integer :: nf90_inq_compound_size
  
    nf90_inq_compound_size = nf_inq_compound_size(ncid, xtype, size)
  end function nf90_inq_compound_size
  ! -----------
  function nf90_inq_compound_nfields(ncid, xtype, nfields)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    integer, intent(out) :: nfields
    integer :: nf90_inq_compound_nfields
  
    nf90_inq_compound_nfields = nf_inq_compound_nfields(ncid, xtype, nfields)
  end function nf90_inq_compound_nfields
  ! -----------
  function nf90_inq_compound_field(ncid, xtype, fieldid, name, offset, &
       field_typeid, ndims, dim_sizes)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    integer, intent(in) :: fieldid
    character (len = *), intent(out) :: name
    integer, intent(out) :: offset
    integer, intent(out) :: field_typeid
    integer, intent(out) :: ndims
    integer, intent(out) :: dim_sizes
    integer :: nf90_inq_compound_field
  
    nf90_inq_compound_field = nf_inq_compound_field(ncid, xtype, fieldid, name, offset, &
       field_typeid, ndims, dim_sizes)
  end function nf90_inq_compound_field
  ! -----------
  function nf90_inq_compound_fieldname(ncid, xtype, fieldid, name)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    integer, intent(in) :: fieldid
    character (len = *), intent(out) :: name
    integer :: nf90_inq_compound_fieldname
  
    nf90_inq_compound_fieldname = nf_inq_compound_fieldname(ncid, xtype, fieldid, name)
  end function nf90_inq_compound_fieldname
  ! -----------
  function nf90_inq_compound_fieldindex(ncid, xtype, name, fieldid)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    character (len = *), intent(in) :: name
    integer, intent(out) :: fieldid
    integer :: nf90_inq_compound_fieldindex
  
    nf90_inq_compound_fieldindex = nf_inq_compound_fieldindex(ncid, xtype, name, fieldid)
  end function nf90_inq_compound_fieldindex
  ! -----------
  function nf90_inq_compound_fieldoffset(ncid, xtype, fieldid, offset)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    integer, intent(in) :: fieldid
    integer, intent(out) :: offset
    integer :: nf90_inq_compound_fieldoffset
  
    nf90_inq_compound_fieldoffset = nf_inq_compound_fieldoffset(ncid, xtype, fieldid, offset)
  end function nf90_inq_compound_fieldoffset
  ! -----------
  function nf90_inq_compound_fieldtype(ncid, xtype, fieldid, field_typeid)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    integer, intent(in) :: fieldid
    integer, intent(out) :: field_typeid
    integer :: nf90_inq_compound_fieldtype
  
    nf90_inq_compound_fieldtype = nf_inq_compound_fieldtype(ncid, xtype, fieldid, field_typeid)
  end function nf90_inq_compound_fieldtype
  ! -----------
  function nf90_inq_compound_fieldndims(ncid, xtype, fieldid, ndims)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    integer, intent(in) :: fieldid
    integer, intent(out) :: ndims
    integer :: nf90_inq_compound_fieldndims
  
    nf90_inq_compound_fieldndims = nf_inq_compound_fieldndims(ncid, xtype, fieldid, ndims)
  end function nf90_inq_compound_fieldndims
  ! -----------
  function nf90_inq_cmp_fielddim_sizes(ncid, xtype, fieldid, dim_sizes)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    integer, intent(in) :: fieldid
    integer, intent(out) :: dim_sizes
    integer :: nf90_inq_cmp_fielddim_sizes
  
    nf90_inq_cmp_fielddim_sizes = nf_inq_compound_fielddim_sizes(ncid, xtype, fieldid, dim_sizes)
  end function nf90_inq_cmp_fielddim_sizes
  ! -----------
  function nf90_def_vlen(ncid, name, base_typeid, xtypeid)
    integer, intent(in) :: ncid
    character (len = *), intent(in) :: name
    integer, intent(in) :: base_typeid
    integer, intent(out) :: xtypeid
    integer :: nf90_def_vlen
  
    nf90_def_vlen = nf_def_vlen(ncid, name, base_typeid, xtypeid)
  end function nf90_def_vlen
  ! -----------
  function nf90_inq_vlen(ncid, xtype, name, datum_size, base_nc_type)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    character (len = *), intent(out) :: name
    integer, intent(out) :: datum_size
    integer, intent(out) :: base_nc_type
    integer :: nf90_inq_vlen
  
    nf90_inq_vlen = nf_inq_vlen(ncid, xtype, name, datum_size, base_nc_type)
  end function nf90_inq_vlen
  ! -----------
  function nf90_free_vlen(vl)
    character (len = *), intent(in) :: vl
    integer :: nf90_free_vlen
  
    nf90_free_vlen = nf_free_vlen(vl)
  end function nf90_free_vlen
!   ! -----------
  function nf90_def_enum(ncid, base_typeid, name, typeid)
    integer, intent(in) :: ncid
    integer, intent(in) :: base_typeid
    character (len = *), intent(in) :: name
    integer, intent(out) :: typeid
    integer :: nf90_def_enum
  
    nf90_def_enum = nf_def_enum(ncid, base_typeid, name, typeid)
  end function nf90_def_enum
!   ! -----------
  function nf90_inq_user_type(ncid, xtype, name, size, base_typeid, nfields, class)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    character (len = *), intent(out) :: name
    integer, intent(out) :: size
    integer, intent(out) :: base_typeid
    integer, intent(out) :: nfields
    integer, intent(out) :: class
    integer :: nf90_inq_user_type
  
    nf90_inq_user_type = nf_inq_user_type(ncid, xtype, name, size, base_typeid, nfields, class)
  end function nf90_inq_user_type
  ! -----------
  function nf90_insert_enum(ncid, xtype, name, value)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    character (len = *), intent(in) :: name
    integer, intent(in) :: value
    integer :: nf90_insert_enum
  
    nf90_insert_enum = nf_insert_enum(ncid, xtype, name, value)
  end function nf90_insert_enum
  ! -----------
  function nf90_inq_enum(ncid, xtype, name, base_nc_type, base_size, num_members)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    character (len = *), intent(out) :: name
    integer, intent(out) :: base_nc_type
    integer, intent(out) :: base_size
    integer, intent(out) :: num_members
    integer :: nf90_inq_enum
  
    nf90_inq_enum = nf_inq_enum(ncid, xtype, name, base_nc_type, base_size, num_members)
  end function nf90_inq_enum
  ! -----------
  function nf90_inq_enum_member(ncid, xtype, idx, name, value)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    integer, intent(in) :: idx
    character (len = *), intent(out) :: name
    integer, intent(in) :: value
    integer :: nf90_inq_enum_member
  
    nf90_inq_enum_member = nf_inq_enum_member(ncid, xtype, idx, name, value)
  end function nf90_inq_enum_member
  ! -----------
  function nf90_inq_enum_ident(ncid, xtype, value, idx)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    integer, intent(in) :: value
    integer, intent(out) :: idx
    integer :: nf90_inq_enum_ident
  
    nf90_inq_enum_ident = nf_inq_enum_ident(ncid, xtype, value, idx)
  end function nf90_inq_enum_ident
  ! -----------
  function nf90_def_opaque(ncid, size, name, xtype)
    integer, intent(in) :: ncid
    integer, intent(in) :: size
    character (len = *), intent(in) :: name
    integer, intent(out) :: xtype
    integer :: nf90_def_opaque
  
    nf90_def_opaque = nf_def_opaque(ncid, size, name, xtype)
  end function nf90_def_opaque
  ! -----------
  function nf90_inq_opaque(ncid, xtype, name, size)
    integer, intent(in) :: ncid
    integer, intent(in) :: xtype
    character (len = *), intent(out) :: name
    integer, intent(out) :: size
    integer :: nf90_inq_opaque
  
    nf90_inq_opaque = nf_inq_opaque(ncid, xtype, name, size)
  end function nf90_inq_opaque
  ! -----------
  function nf90_def_var_chunking(ncid, varid, contiguous, chunksizes)
    integer, intent(in) :: ncid
    integer, intent(in) :: varid
    integer, intent(in) :: contiguous
    integer, dimension(:), intent(in) :: chunksizes
    integer :: nf90_def_var_chunking
  
    nf90_def_var_chunking = nf_def_var_chunking(ncid, varid, contiguous, chunksizes)
  end function nf90_def_var_chunking
  ! -----------
  function nf90_def_var_deflate(ncid, varid, shuffle, deflate, deflate_level)
    integer, intent(in) :: ncid
    integer, intent(in) :: varid
    integer, intent(in) :: shuffle
    integer, intent(in) :: deflate
    integer, intent(in) :: deflate_level
    integer :: nf90_def_var_deflate
  
    nf90_def_var_deflate = nf_def_var_deflate(ncid, varid, shuffle, deflate, deflate_level)
  end function nf90_def_var_deflate
  ! -----------
  function nf90_def_var_fletcher32(ncid, varid, fletcher32)
    integer, intent(in) :: ncid
    integer, intent(in) :: varid
    integer, intent(in) :: fletcher32
    integer :: nf90_def_var_fletcher32
  
    nf90_def_var_fletcher32 = nf_def_var_fletcher32(ncid, varid, fletcher32)
  end function nf90_def_var_fletcher32
  ! -----------
  function nf90_inq_var_chunking(ncid, varid, contiguous, chunksizes)
    integer, intent(in) :: ncid
    integer, intent(in) :: varid
    integer, intent(out) :: contiguous
    integer, dimension(:), intent(out) :: chunksizes
    integer :: nf90_inq_var_chunking
  
    nf90_inq_var_chunking = nf_inq_var_chunking(ncid, varid, contiguous, chunksizes)
  end function nf90_inq_var_chunking
  ! -----------
  function nf90_inq_var_deflate(ncid, varid, shuffle, deflate, deflate_level)
    integer, intent(in) :: ncid
    integer, intent(in) :: varid
    integer, intent(out) :: shuffle
    integer, intent(out) :: deflate
    integer, intent(out) :: deflate_level
    integer :: nf90_inq_var_deflate
  
    nf90_inq_var_deflate = nf_inq_var_deflate(ncid, varid, shuffle, deflate, deflate_level)
  end function nf90_inq_var_deflate
  ! -----------
  function nf90_inq_var_fletcher32(ncid, varid, fletcher32)
    integer, intent(in) :: ncid
    integer, intent(in) :: varid
    integer, intent(out) :: fletcher32
    integer :: nf90_inq_var_fletcher32
  
    nf90_inq_var_fletcher32 = nf_inq_var_fletcher32(ncid, varid, fletcher32)
  end function nf90_inq_var_fletcher32
  ! -----------
  function nf90_def_var_endian(ncid, varid, endian)
    integer, intent(in) :: ncid
    integer, intent(in) :: varid
    integer, intent(in) :: endian
    integer :: nf90_def_var_endian
  
    nf90_def_var_endian = nf_def_var_endian(ncid, varid, endian)
  end function nf90_def_var_endian
  ! -----------
  function nf90_inq_var_endian(ncid, varid, endian)
    integer, intent(in) :: ncid
    integer, intent(in) :: varid
    integer, intent(out) :: endian
    integer :: nf90_inq_var_endian
  
    nf90_inq_var_endian = nf_inq_var_endian(ncid, varid, endian)
  end function nf90_inq_var_endian
  ! -----------
  function nf90_def_var_fill(ncid, varid, no_fill, fill)
    integer, intent(in) :: ncid
    integer, intent(in) :: varid
    integer, intent(in) :: no_fill
    integer, intent(in) :: fill
    integer :: nf90_def_var_fill
  
    nf90_def_var_fill = nf_def_var_fill(ncid, varid, no_fill, fill)
  end function nf90_def_var_fill
  ! -----------
  function nf90_inq_var_fill(ncid, varid, no_fill, fill)
    integer, intent(in) :: ncid
    integer, intent(in) :: varid
    integer, intent(out) :: no_fill
    integer, intent(out) :: fill
    integer :: nf90_inq_var_fill
  
    nf90_inq_var_fill = nf_inq_var_fill(ncid, varid, no_fill, fill)
  end function nf90_inq_var_fill
  ! -----------
  function nf90_put_att_any(ncid, varid, name, typeid, length, values)
    integer,                          intent( in) :: ncid, varid
    character(len = *),               intent( in) :: name
    integer,                          intent( in) :: typeid, length
    character(len = *),               intent( in) :: values
    integer                                       :: nf90_put_att_any

    nf90_put_att_any = nf_put_att(ncid, varid, name, typeid, length, values)
  end function nf90_put_att_any
  ! -----------
  function nf90_get_att_any(ncid, varid, name, length, values)
    integer,                          intent( in) :: ncid, varid
    character(len = *),               intent( in) :: name
    integer,                          intent( in) :: length
    character(len = *),               intent( in) :: values
    integer                                       :: nf90_get_att_any

    nf90_get_att_any = nf_get_att(ncid, varid, name, values)
  end function nf90_get_att_any
  ! -----------
   function nf90_put_var_any(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *),             intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_any

     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride
 
     ! Set local arguments to default values
     localStart (:)  = 1
     localCount (1)  = len_trim(values); localCount (2:) = 1
     localStride(:)  = 1
          
     if(present(start))  localStart (:size(start) ) = start(:)
     if(present(count))  localCount (:size(count) ) = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)

     nf90_put_var_any = nf_put_vars(ncid, varid, localStart, localCount, localStride, values)
   end function nf90_put_var_any
  ! -----------
   function nf90_get_var_any(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *),             intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_any
 
     integer, dimension(nf90_max_var_dims) :: textDimIDs
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride 
     integer                               :: stringLength
 
     ! Set local arguments to default values
     localStart (:)  = 1
     localCount (1)  = len(values); localCount (2:) = 1
     localStride(:)  = 1
     
     if(present(start))  localStart (:size(start) ) = start(:)
     if(present(count))  localCount (:size(count) ) = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)

     nf90_get_var_any = nf_get_vars(ncid, varid, localStart, localCount, localStride, values)
   end function nf90_get_var_any
