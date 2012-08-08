!     This is part of the netCDF package.
!     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
!     See COPYRIGHT file for conditions of use.

!     This program tests netCDF-4 variable functions from fortran.

!     $Id$

program f90tst_grps
  use typeSizes
  use netcdf
  implicit none
  
  ! This is the name of the data file we will create.
  character (len = *), parameter :: FILE_NAME = "f90tst_grps.nc"

  ! We are writing 2D data, a 6 x 12 grid. 
  integer, parameter :: MAX_DIMS = 2
  integer, parameter :: NX = 6, NY = 12

  ! We need these ids and other gunk for netcdf.
  integer :: ncid, varid1, varid2, varid3, varid4, dimids(MAX_DIMS)
  integer :: chunksizes(MAX_DIMS), chunksizes_in(MAX_DIMS)
  integer :: x_dimid, y_dimid
  integer :: nvars, ngatts, ndims, unlimdimid, file_format
  integer, parameter :: CACHE_NELEMS = 10000, CACHE_SIZE = 1000000
  integer, parameter :: DEFLATE_LEVEL = 4
  integer (kind = 8), parameter :: TOE_SAN_VALUE = 2147483648_8
  character (len = *), parameter :: VAR1_NAME = "Payroll"
  character (len = *), parameter :: VAR2_NAME = "Spies"
  character (len = *), parameter :: VAR3_NAME = "Propaganda"
  character (len = *), parameter :: VAR4_NAME = "Arms"
  character (len = *), parameter :: GRP1_NAME = "Irish_Republican_Brotherhood"
  character (len = *), parameter :: GRP2_NAME = "Sinn_Fein"
  character (len = *), parameter :: GRP3_NAME = "Gaelic_Athletic_Association"
  character (len = *), parameter :: GRP4_NAME = "Provisional_Government"

  character (len = NF90_MAX_NAME) :: grp1_full_name
  integer :: len

  ! Information read back from the file to check correctness.
  integer :: varid1_in, varid2_in, varid3_in, varid4_in
  integer :: grpid1, grpid2, grpid3, grpid4
  integer :: xtype_in, ndims_in, natts_in, dimids_in(MAX_DIMS)
  character (len = nf90_max_name) :: name_in
  integer :: endianness_in, deflate_level_in
  logical :: shuffle_in, fletcher32_in, contiguous_in

  print *, ''
  print *,'*** Testing netCDF-4 groups from Fortran 90.'

  ! Create the netCDF file. 
  call check(nf90_create(FILE_NAME, nf90_netcdf4, ncid))

  ! Define the dimensions.
  call check(nf90_def_dim(ncid, "x", NX, x_dimid))
  call check(nf90_def_dim(ncid, "y", NY, y_dimid))
  dimids =  (/ y_dimid, x_dimid /)

  ! Define some nested groups.
  call check(nf90_def_grp(ncid, GRP1_NAME, grpid1))
  call check(nf90_def_grp(grpid1, GRP2_NAME, grpid2))
  call check(nf90_def_grp(grpid2, GRP3_NAME, grpid3))
  call check(nf90_def_grp(grpid3, GRP4_NAME, grpid4))

  ! Define some variables. 
  chunksizes = (/ NY, NX /)
  call check(nf90_def_var(ncid, VAR1_NAME, NF90_INT, dimids, varid1, chunksizes = chunksizes, &
       shuffle = .TRUE., fletcher32 = .TRUE., endianness = nf90_endian_big, deflate_level = DEFLATE_LEVEL))
  call check(nf90_def_var(grpid1, VAR2_NAME, NF90_INT, dimids, varid2, contiguous = .TRUE.))
  call check(nf90_def_var(grpid2, VAR3_NAME, NF90_INT64, varid3))
  call check(nf90_def_var(grpid3, VAR4_NAME, NF90_INT, x_dimid, varid4, contiguous = .TRUE.))

  ! Close the file. 
  call check(nf90_close(ncid))

  ! Reopen the file.
  call check(nf90_open(FILE_NAME, nf90_nowrite, ncid))
  
  ! Check some stuff out.
  call check(nf90_inquire(ncid, ndims, nvars, ngatts, unlimdimid, file_format))
  if (ndims /= 2 .or. nvars /= 1 .or. ngatts /= 0 .or. unlimdimid /= -1 .or. &
       file_format /= nf90_format_netcdf4) stop 21

  ! Get the group ids for the newly reopened file.
  call check(nf90_inq_grp_ncid(ncid, GRP1_NAME, grpid1))
  call check(nf90_inq_grp_ncid(grpid1, GRP2_NAME, grpid2))
  call check(nf90_inq_grp_ncid(grpid2, GRP3_NAME, grpid3))
  call check(nf90_inq_grp_ncid(grpid3, GRP4_NAME, grpid4))

  ! Check for the groups with full group names. 
  write(grp1_full_name, '(A,A)') '/', GRP1_NAME
  call check(nf90_inq_grp_full_ncid(ncid, grp1_full_name, grpid1))
  call check(nf90_inq_grpname(grpid1, name_in))
  if (name_in .ne. GRP1_NAME) stop 61
  call check(nf90_inq_grpname_full(grpid1, len, name_in))
  if (name_in .ne. grp1_full_name) stop 62

  ! Get varids.
  call check(nf90_inq_varid(ncid, VAR1_NAME, varid1_in))
  call check(nf90_inq_varid(grpid1, VAR2_NAME, varid2_in))
  call check(nf90_inq_varid(grpid2, VAR3_NAME, varid3_in))
  call check(nf90_inq_varid(grpid3, VAR4_NAME, varid4_in))

  ! Check variable 1.
  call check(nf90_inquire_variable(ncid, varid1_in, name_in, xtype_in, ndims_in, dimids_in, &
       natts_in, chunksizes = chunksizes_in, endianness = endianness_in, fletcher32 = fletcher32_in, &
       deflate_level = deflate_level_in, shuffle = shuffle_in))
  if (name_in .ne. VAR1_NAME .or. xtype_in .ne. NF90_INT .or. ndims_in .ne. MAX_DIMS .or. &
       natts_in .ne. 0 .or. dimids_in(1) .ne. dimids(1) .or. dimids_in(2) .ne. dimids(2)) stop 3
  if (chunksizes_in(1) /= chunksizes(1) .or. chunksizes_in(2) /= chunksizes(2)) &
       stop 4
  if (endianness_in .ne. nf90_endian_big) stop 5

  ! Check variable 2.
  call check(nf90_inquire_variable(grpid1, varid2_in, name_in, xtype_in, ndims_in, dimids_in, &
       natts_in, contiguous = contiguous_in, endianness = endianness_in, fletcher32 = fletcher32_in, &
       deflate_level = deflate_level_in, shuffle = shuffle_in))
  if (name_in .ne. VAR2_NAME .or. xtype_in .ne. NF90_INT .or. ndims_in .ne. MAX_DIMS .or. &
       natts_in .ne. 0 .or. dimids_in(1) .ne. dimids(1) .or. dimids_in(2) .ne. dimids(2)) stop 6
  if (deflate_level_in .ne. 0 .or. .not. contiguous_in .or. fletcher32_in .or. shuffle_in) stop 7

  ! Check variable 3.
  call check(nf90_inquire_variable(grpid2, varid3_in, name_in, xtype_in, ndims_in, dimids_in, &
       natts_in, contiguous = contiguous_in, endianness = endianness_in, fletcher32 = fletcher32_in, &
       deflate_level = deflate_level_in, shuffle = shuffle_in))
  if (name_in .ne. VAR3_NAME .or. xtype_in .ne. NF90_INT64 .or. ndims_in .ne. 0 .or. &
       natts_in .ne. 0) stop 8
  if (deflate_level_in .ne. 0 .or. .not. contiguous_in .or. fletcher32_in .or. shuffle_in) stop 9
  
  ! Check variable 4.
  call check(nf90_inquire_variable(grpid3, varid4_in, name_in, xtype_in, ndims_in, dimids_in, &
       natts_in, contiguous = contiguous_in, endianness = endianness_in, fletcher32 = fletcher32_in, &
       deflate_level = deflate_level_in, shuffle = shuffle_in))
  if (name_in .ne. VAR4_NAME .or. xtype_in .ne. NF90_INT .or. ndims_in .ne. 1 .or. &
       natts_in .ne. 0 .or. dimids_in(1) .ne. x_dimid) stop 10
  if (deflate_level_in .ne. 0 .or. .not. contiguous_in .or. fletcher32_in .or. shuffle_in) stop 11

  ! Close the file. 
  call check(nf90_close(ncid))

  print *,'*** SUCCESS!'

contains
!     This subroutine handles errors by printing an error message and
!     exiting with a non-zero status.
  subroutine check(errcode)
    use netcdf
    implicit none
    integer, intent(in) :: errcode
    
    if(errcode /= nf90_noerr) then
       print *, 'Error: ', trim(nf90_strerror(errcode))
       stop 2
    endif
  end subroutine check
end program f90tst_grps

