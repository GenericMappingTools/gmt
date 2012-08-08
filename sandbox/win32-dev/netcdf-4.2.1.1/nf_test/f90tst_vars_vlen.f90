! This is part of the netCDF package.
! Copyright 2009 University Corporation for Atmospheric Research/Unidata.
! See COPYRIGHT file for conditions of use.

! This program tests netCDF-4 vlen variable functions from fortran 90.

! $Id$

program f90tst_vars_vlen
  use typeSizes
  use netcdf
  implicit none
!  include 'netcdf.inc'

!   ! This is the name of the data file we will create.
!   character*(*) FILE_NAME
!   parameter (FILE_NAME='f90tst_vars_vlen.nc')

!   ! NetCDF IDs.
!   integer ncid, vlen_typeid

!   integer max_types
!   parameter (max_types = 1)

!   ! Need these to read type information.
!   integer num_types, typeids(max_types)
!   integer base_type, base_size
!   character*80 type_name
!   integer type_size, nfields, class

!   ! Information for the vlen type we will define.
!   character*(*) vlen_type_name
!   parameter (vlen_type_name = 'vlen_type')

!   ! Some data about and for the vlen.
!   integer vlen_len, vlen_len_in
!   parameter (vlen_len = 5)
!   integer data1(vlen_len), data1_in(vlen_len)

!   ! These must be big enough to hold the stuct nc_vlen in netcdf.h.
!   integer vlen(10), vlen_in(10)

!   ! Loop indexes, and error handling.
!   integer x, retval

!   print *, ''
!   print *,'*** Testing VLEN types.'

!   do x = 1, vlen_len
!      data1(x) = x
!   end do

!   ! Create the netCDF file.
!   retval = nf_create(FILE_NAME, NF_NETCDF4, ncid)
!   if (retval .ne. nf_noerr) call handle_err(retval)

!   ! Create the vlen type.
!   retval = nf_def_vlen(ncid, vlen_type_name, nf_int, vlen_typeid)
!   if (retval .ne. nf_noerr) call handle_err(retval)

!   ! Set up the vlen with this helper function, since F77 can't deal
!   ! with pointers.
!   retval = nf_put_vlen_element(ncid, vlen_typeid, vlen, vlen_len, data1)
!   if (retval .ne. nf_noerr) call handle_err(retval)

!   ! Write the vlen attribute.
!   retval = nf_put_att(ncid, NF_GLOBAL, 'att1', vlen_typeid, 1, vlen)
!   if (retval .ne. nf_noerr) call handle_err(retval)

!   ! Close the file. 
!   retval = nf_close(ncid)
!   if (retval .ne. nf_noerr) call handle_err(retval)

!   ! Reopen the file.
!   retval = nf_open(FILE_NAME, NF_NOWRITE, ncid)
!   if (retval .ne. nf_noerr) call handle_err(retval)

!   ! Get the typeids of all user defined types.
!   retval = nf_inq_typeids(ncid, num_types, typeids)
!   if (retval .ne. nf_noerr) call handle_err(retval)
!   if (num_types .ne. max_types) stop 2

!   ! Use nf_inq_user_type to confirm this is an vlen type, with vlen
!   ! size 16, base type NF_INT.
!   retval = nf_inq_user_type(ncid, typeids(1), type_name, type_size, &
!        base_type, nfields, class)
!   if (retval .ne. nf_noerr) call handle_err(retval)
!   if (type_name(1:len(vlen_type_name)) .ne. vlen_type_name .or. &
!         type_size .ne. 16 .or. base_type .ne. nf_int .or. &
!         nfields .ne. 0 .or. class .ne. nf_vlen) stop 3

!   ! Use nf_inq_vlen and make sure we get the same answers as we did
!   ! with nf_inq_user_type.
!   retval = nf_inq_vlen(ncid, typeids(1), type_name, base_size, base_type)
!   if (retval .ne. nf_noerr) call handle_err(retval)
!   if (type_name(1:len(vlen_type_name)) .ne. vlen_type_name .or. &
!        base_type .ne. nf_int .or. base_size .ne. 16) stop 4

!   ! Read the vlen attribute.
!   retval = nf_get_att(ncid, NF_GLOBAL, 'att1', vlen_in)
!   if (retval .ne. nf_noerr) call handle_err(retval)

!   ! Get the data from the vlen we just read.
!   retval = nf_get_vlen_element(ncid, vlen_typeid, vlen_in, &
!        vlen_len_in, data1_in)
!   if (retval .ne. nf_noerr) call handle_err(retval)
!   if (vlen_len_in .ne. vlen_len) stop 5

!   ! Check the data
!   do x = 1, vlen_len
!      if (data1(x) .ne. data1_in(x)) stop 6
!   end do

!   ! Close the file. 
!   retval = nf_close(ncid)
!   if (retval .ne. nf_noerr) call handle_err(retval)

!   print *,'*** SUCCESS!'
! contains
! !     This subroutine handles errors by printing an error message and
! !     exiting with a non-zero status.
!   subroutine handle_err(errcode)
!     use netcdf
!     implicit none
!     integer, intent(in) :: errcode
    
!     if(errcode /= nf90_noerr) then
!        print *, 'Error: ', trim(nf90_strerror(errcode))
!        stop 1
!     endif
!   end subroutine handle_err
end program f90tst_vars_vlen
