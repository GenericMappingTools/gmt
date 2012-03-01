!     This is part of the netCDF package.
!     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
!     See COPYRIGHT file for conditions of use.

!     This program tests netCDF-4 variable functions from fortran.

!     $Id$

program f90tst_vars
  use typeSizes
  use netcdf
  implicit none
  
  ! This is the name of the data file we will create.
  character (len = *), parameter :: FILE_NAME = "f90tst_vars.nc"

  ! We are writing 2D data, a 6 x 12 grid. 
  integer, parameter :: MAX_DIMS = 2
  integer, parameter :: NX = 6, NY = 12
  integer :: data_out(NY, NX), data_in(NY, NX)

  ! We need these ids and other gunk for netcdf.
  integer :: ncid, varid, dimids(MAX_DIMS), chunksizes(MAX_DIMS), chunksizes_in(MAX_DIMS)
  integer :: x_dimid, y_dimid, contig
  integer :: mode_flag
  integer :: nvars, ngatts, ndims, unlimdimid, file_format
  integer :: x, y
  integer, parameter :: CACHE_SIZE = 1000000

  print *, ''
  print *,'*** Testing definition of netCDF-4 vars from Fortran 90.'

  ! Create some pretend data.
  do x = 1, NX
     do y = 1, NY
        data_out(y, x) = (x - 1) * NY + (y - 1)
     end do
  end do

  ! Create the netCDF file. 
  mode_flag = IOR(nf90_netcdf4, nf90_classic_model) 
  call handle_err(nf90_create(FILE_NAME, mode_flag, ncid, cache_size = CACHE_SIZE))

  ! Define the dimensions.
  call handle_err(nf90_def_dim(ncid, "x", NX, x_dimid))
  call handle_err(nf90_def_dim(ncid, "y", NY, y_dimid))
  dimids =  (/ y_dimid, x_dimid /)

  ! Define the variable. 
  call handle_err(nf90_def_var(ncid, "data", NF90_INT, dimids, varid))

  ! Set up chunking.
  chunksizes = (/ NY, NX /)
  call handle_err(nf90_def_var_chunking(ncid, varid, 0, chunksizes))

  ! With classic model netCDF-4 file, enddef must be called.
  call handle_err(nf90_enddef(ncid))

  ! Write the pretend data to the file.
  call handle_err(nf90_put_var(ncid, varid, data_out))

  ! Close the file. 
  call handle_err(nf90_close(ncid))

  ! Reopen the file.
  call handle_err(nf90_open(FILE_NAME, nf90_nowrite, ncid))
  
  ! Check some stuff out.
  call handle_err(nf90_inquire(ncid, ndims, nvars, ngatts, unlimdimid, file_format))
  if (ndims /= 2 .or. nvars /= 1 .or. ngatts /= 0 .or. unlimdimid /= -1 .or. &
       file_format /= nf90_format_netcdf4_classic) stop 2

  call handle_err(nf90_inq_var_chunking(ncid, varid, contig, chunksizes_in))
  if (chunksizes_in(1) /= chunksizes(1) .or. chunksizes_in(2) /= chunksizes(2)) &
       stop 2

  ! Check the data.
  call handle_err(nf90_get_var(ncid, varid, data_in))
  do x = 1, NX
     do y = 1, NY
        if (data_out(y, x) .ne. data_in(y, x)) stop 3
     end do
  end do

  ! Close the file. 
  call handle_err(nf90_close(ncid))

  print *,'*** SUCCESS!'

contains
!     This subroutine handles errors by printing an error message and
!     exiting with a non-zero status.
  subroutine handle_err(errcode)
    use netcdf
    implicit none
    integer, intent(in) :: errcode
    
    if(errcode /= nf90_noerr) then
       print *, 'Error: ', trim(nf90_strerror(errcode))
       stop 2
    endif
  end subroutine handle_err
end program f90tst_vars

