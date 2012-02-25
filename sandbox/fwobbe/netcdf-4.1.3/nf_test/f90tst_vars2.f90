!     This is part of the netCDF package.
!     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
!     See COPYRIGHT file for conditions of use.

!     This program tests netCDF-4 variable functions from fortran.

!     $Id$

program f90tst_vars2
  use typeSizes
  use netcdf
  implicit none
  
  ! This is the name of the data file we will create.
  character (len = *), parameter :: FILE_NAME = "f90tst_vars2.nc"

  ! We are writing 2D data, a 6 x 12 grid. 
  integer, parameter :: MAX_DIMS = 2
  integer, parameter :: NX = 6, NY = 12
  integer :: data_out(NY, NX), data_in(NY, NX)
  integer :: data_out_1d(NX), data_in_1d(NX)

  ! We need these ids and other gunk for netcdf.
  integer :: ncid, varid1, varid2, varid3, varid4, varid5, dimids(MAX_DIMS)
  integer :: chunksizes(MAX_DIMS), chunksizes_in(MAX_DIMS)
  integer :: x_dimid, y_dimid
  integer :: nvars, ngatts, ndims, unlimdimid, file_format
  integer :: x, y
  integer, parameter :: CACHE_NELEMS = 10000, CACHE_SIZE = 1000000
  integer, parameter :: DEFLATE_LEVEL = 4
  integer (kind = 8), parameter :: TOE_SAN_VALUE = 2147483648_8
  character (len = *), parameter :: VAR1_NAME = "Chon-Ji"
  character (len = *), parameter :: VAR2_NAME = "Tan-Gun"
  character (len = *), parameter :: VAR3_NAME = "Toe-San"
  character (len = *), parameter :: VAR4_NAME = "Won-Hyo"
  character (len = *), parameter :: VAR5_NAME = "Yul-Guk"
  integer, parameter :: CACHE_SIZE = 8, CACHE_NELEMS = 571, CACHE_PREEMPTION = 66

  ! Information read back from the file to check correctness.
  integer :: varid1_in, varid2_in, varid3_in, varid4_in, varid5_in
  integer :: xtype_in, ndims_in, natts_in, dimids_in(MAX_DIMS)
  character (len = nf90_max_name) :: name_in
  integer :: endianness_in, deflate_level_in
  logical :: shuffle_in, fletcher32_in, contiguous_in
  integer (kind = 8) :: toe_san_in
  integer :: cache_size_in, cache_nelems_in, cache_preemption_in

  print *, ''
  print *,'*** Testing definition of netCDF-4 vars from Fortran 90.'

  ! Create some pretend data.
  do x = 1, NX
     do y = 1, NY
        data_out(y, x) = (x - 1) * NY + (y - 1)
     end do
  end do
  do x = 1, NX
        data_out_1d(x) = x
  end do

  ! Create the netCDF file. 
  call check(nf90_create(FILE_NAME, nf90_netcdf4, ncid, cache_nelems = CACHE_NELEMS, &
       cache_size = CACHE_SIZE))

  ! Define the dimensions.
  call check(nf90_def_dim(ncid, "x", NX, x_dimid))
  call check(nf90_def_dim(ncid, "y", NY, y_dimid))
  dimids =  (/ y_dimid, x_dimid /)

  ! Define some variables. 
  chunksizes = (/ NY, NX /)
  call check(nf90_def_var(ncid, VAR1_NAME, NF90_INT, dimids, varid1, chunksizes = chunksizes, &
       shuffle = .TRUE., fletcher32 = .TRUE., endianness = nf90_endian_big, deflate_level = DEFLATE_LEVEL))
  call check(nf90_def_var(ncid, VAR2_NAME, NF90_INT, dimids, varid2, contiguous = .TRUE.))
  call check(nf90_def_var(ncid, VAR3_NAME, NF90_INT64, varid3))
  call check(nf90_def_var(ncid, VAR4_NAME, NF90_INT, x_dimid, varid4, contiguous = .TRUE.))
  call check(nf90_def_var(ncid, VAR5_NAME, NF90_INT, dimids, varid5, chunksizes = chunksizes, &
       option_mask = nf90_szip_ec_option_mask, bits_per_pixel = 16))

  ! As a test, set and the check the chunk cache for one of these
  ! variables. Really there is no need for this, since the 1 MB cache
  ! that comes by default is plenty for the tiny size of our
  ! variable. But for variables with larger chunksizes, a bigger
  ! per-var chunk cache really helps performance.
  call check(nf90_set_var_chunk_cache(ncid, varid1, CACHE_SIZE, &
       CACHE_NELEMS, CACHE_PREEMPTION))
  call check(nf90_inquire_variable(ncid, varid1_in, cache_size = cache_size_in, &
       cache_nelems = cache_nelems_in, cache_preemption = cache_preemption_in))
  if (cache_size_in .ne. CACHE_SIZE .or. cache_nelems_in .ne. &
       CACHE_NELEMS .or. cache_preemption_in .ne. CACHE_PREEMPTION) stop 1

  ! Write the pretend data to the file.
  call check(nf90_put_var(ncid, varid1, data_out))
  call check(nf90_put_var(ncid, varid2, data_out))
  call check(nf90_put_var(ncid, varid3, TOE_SAN_VALUE))
  call check(nf90_put_var(ncid, varid4, data_out_1d))
  call check(nf90_put_var(ncid, varid5, data_out))

  ! Close the file. 
  call check(nf90_close(ncid))

  ! Reopen the file.
  call check(nf90_open(FILE_NAME, nf90_nowrite, ncid))
  
  ! Check some stuff out.
  call check(nf90_inquire(ncid, ndims, nvars, ngatts, unlimdimid, file_format))
  if (ndims /= 2 .or. nvars /= 5 .or. ngatts /= 0 .or. unlimdimid /= -1 .or. &
       file_format /= nf90_format_netcdf4) stop 2

  ! Get varids.
  call check(nf90_inq_varid(ncid, VAR1_NAME, varid1_in))
  call check(nf90_inq_varid(ncid, VAR2_NAME, varid2_in))
  call check(nf90_inq_varid(ncid, VAR3_NAME, varid3_in))
  call check(nf90_inq_varid(ncid, VAR4_NAME, varid4_in))
  call check(nf90_inq_varid(ncid, VAR5_NAME, varid5_in))

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
  call check(nf90_inquire_variable(ncid, varid2_in, name_in, xtype_in, ndims_in, dimids_in, &
       natts_in, contiguous = contiguous_in, endianness = endianness_in, fletcher32 = fletcher32_in, &
       deflate_level = deflate_level_in, shuffle = shuffle_in))
  if (name_in .ne. VAR2_NAME .or. xtype_in .ne. NF90_INT .or. ndims_in .ne. MAX_DIMS .or. &
       natts_in .ne. 0 .or. dimids_in(1) .ne. dimids(1) .or. dimids_in(2) .ne. dimids(2)) stop 6
  if (deflate_level_in .ne. 0 .or. .not. contiguous_in .or. fletcher32_in .or. shuffle_in) stop 7

  ! Check variable 3.
  call check(nf90_inquire_variable(ncid, varid3_in, name_in, xtype_in, ndims_in, dimids_in, &
       natts_in, contiguous = contiguous_in, endianness = endianness_in, fletcher32 = fletcher32_in, &
       deflate_level = deflate_level_in, shuffle = shuffle_in))
  if (name_in .ne. VAR3_NAME .or. xtype_in .ne. NF90_INT64 .or. ndims_in .ne. 0 .or. &
       natts_in .ne. 0) stop 8
  if (deflate_level_in .ne. 0 .or. .not. contiguous_in .or. fletcher32_in .or. shuffle_in) stop 9
  
  ! Check variable 4.
  call check(nf90_inquire_variable(ncid, varid4_in, name_in, xtype_in, ndims_in, dimids_in, &
       natts_in, contiguous = contiguous_in, endianness = endianness_in, fletcher32 = fletcher32_in, &
       deflate_level = deflate_level_in, shuffle = shuffle_in))
  if (name_in .ne. VAR4_NAME .or. xtype_in .ne. NF90_INT .or. ndims_in .ne. 1 .or. &
       natts_in .ne. 0 .or. dimids_in(1) .ne. x_dimid) stop 10
  if (deflate_level_in .ne. 0 .or. .not. contiguous_in .or. fletcher32_in .or. shuffle_in) stop 11

  ! Check the data.
  call check(nf90_get_var(ncid, varid1_in, data_in))
  do x = 1, NX
     do y = 1, NY
        if (data_out(y, x) .ne. data_in(y, x)) stop 12
     end do
  end do
  call check(nf90_get_var(ncid, varid2_in, data_in))
  do x = 1, NX
     do y = 1, NY
        if (data_out(y, x) .ne. data_in(y, x)) stop 13
     end do
  end do
  call check(nf90_get_var(ncid, varid3_in, toe_san_in))
  if (toe_san_in .ne. TOE_SAN_VALUE) stop 14
  call check(nf90_get_var(ncid, varid4_in, data_in_1d))
  do x = 1, NX
     if (data_out_1d(x) .ne. data_in_1d(x)) stop 15
  end do
  call check(nf90_get_var(ncid, varid5_in, data_in))
  do x = 1, NX
     do y = 1, NY
        if (data_out(y, x) .ne. data_in(y, x)) stop 12
     end do
  end do

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
end program f90tst_vars2

