  ! Library version, error string
  public :: nf90_inq_libvers, nf90_strerror
  
  ! Control routines 
  public :: nf90_create, nf90_open, nf90_set_base_pe, nf90_inq_base_pe, &
            nf90_set_fill, nf90_redef, nf90_enddef,                     &
            nf90_create_mp, nf90_open_mp,                               &
            nf90_sync, nf90_abort, nf90_close, nf90_delete
            
  ! File level inquiry
  public :: nf90_inquire
  
  ! Dimension routines
  public :: nf90_def_dim, nf90_inq_dimid, nf90_rename_dim, nf90_inquire_dimension
  
  ! attribute routines
  public :: nf90_copy_att, nf90_rename_att, nf90_del_att, nf90_inq_attname, &
            nf90_inquire_attribute
  ! overloaded functions
  public :: nf90_put_att, nf90_get_att
  
  ! Variable routines
  public :: nf90_def_var, nf90_inq_varid, nf90_rename_var, nf90_inquire_variable 
  ! overloaded functions
  public :: nf90_put_var, nf90_get_var
