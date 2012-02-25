!     This is part of the netCDF package.
!     Copyright 2007 University Corporation for Atmospheric Research/Unidata.
!     See COPYRIGHT file for conditions of use.

!     This program tests netCDF-4 new types from fortran 90.

!     $Id$

program tst_types
  use typeSizes
  use netcdf
  implicit none
  
  ! This is the name of the data file we will create.
  character (len = *), parameter :: FILE_NAME = "tst_types.nc"

  ! Information for the types we create.
  character (len = *), parameter :: OPAQUE_TYPE_NAME = "Odessyus"
  character (len = *), parameter :: var_name = "Polyphemus"
  character (len = 80) :: name_in
  character (len = 10), parameter :: opaque_data = "0123456789"
  character (len = *), parameter :: att_name = "att1"

  integer, parameter :: OPAQUE_SIZE = 10
  integer (kind = EightByteInt) BIG_NUMBER, num_in
  parameter (BIG_NUMBER = 4294967295_EightByteInt)
  integer :: ncid, opaque_typeid, varid
  integer :: size_in, base_typeid_in, nfields_in, class_in

  print *, ''
  print *,'*** Testing new netCDF-4 types from Fortran 90.'
  
  ! Create the netCDF file. 
  call check(nf90_create(FILE_NAME, nf90_netcdf4, ncid))

  ! Create an opaque type.
  call check(nf90_def_opaque(ncid, OPAQUE_SIZE, OPAQUE_TYPE_NAME, opaque_typeid))

  ! Write an (global) opaque attribute.
  call check(nf90_put_att_any(ncid, NF90_GLOBAL, att_name, opaque_typeid, 1, opaque_data))

  ! Create an int64 scalar variable.
  call check(nf90_def_var(ncid, var_name, nf90_int64, varid))

  ! Write a large integer (too large to fit in 32-bit ints).
  call check(nf90_put_var(ncid, varid, BIG_NUMBER))

  ! Close the file. 
  call check(nf90_close(ncid))

  ! Reopen the netCDF file. 
  call check(nf90_open(FILE_NAME, 0, ncid))

  ! Check the opaque type.
  call check(nf90_inq_user_type(ncid, opaque_typeid, name_in, size_in, &
       base_typeid_in, nfields_in, class_in))
  if (name_in(1:len(OPAQUE_TYPE_NAME)) .ne. OPAQUE_TYPE_NAME .or. &
       size_in .ne. OPAQUE_SIZE .or. base_typeid_in .ne. 0 .or. &
       nfields_in .ne. 0 .or. class_in .ne. NF90_OPAQUE) stop 2

  ! Check it again with the inq_opaque call.
  call check(nf90_inq_opaque(ncid, opaque_typeid, name_in, size_in))
  if (name_in(1:len(OPAQUE_TYPE_NAME)) .ne. OPAQUE_TYPE_NAME .or. &
       size_in .ne. OPAQUE_SIZE) stop 2

  ! Read in the large number.
  call check(nf90_get_var(ncid, varid, num_in))
  if (num_in .ne. BIG_NUMBER) stop 2

  ! Close the file. 
  call check(nf90_close(ncid))
  
  print *,'*** SUCCESS!'

!     This subroutine handles errors by printing an error message and
!     exiting with a non-zero status.
contains
  subroutine check(status)
    integer, intent ( in) :: status
    
    if(status /= nf90_noerr) then 
      print *, trim(nf90_strerror(status))
      stop 2
    end if
  end subroutine check  

end program tst_types

