! This is part of the netCDF package.
! Copyright 2006 University Corporation for Atmospheric Research/Unidata.
! See COPYRIGHT file for conditions of use.
      
! This is a simple example which reads a small dummy array, from a
! netCDF data file created by the companion program simple_xy_wr.f90.
      
! This is intended to illustrate the use of the netCDF fortran 90
! API. This example program is part of the netCDF tutorial, which can
! be found at:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial
      
! Full documentation of the netCDF Fortran 90 API can be found at:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f90

! $Id$

program simple_xy_rd
  use netcdf
  implicit none

  ! This is the name of the data file we will read. 
  character (len = *), parameter :: FILE_NAME = "simple_xy.nc"

  ! We are reading 2D data, a 12 x 6 grid. 
  integer, parameter :: NX = 6, NY = 12
  integer, dimension(:,:), allocatable :: data_in

  ! This will be the netCDF ID for the file and data variable.
  integer :: ncid, varid

  ! Loop indexes, and error handling.
  integer :: x, y

  ! Allocate memory for data.
  allocate(data_in(NY, NX))

  ! Open the file. NF90_NOWRITE tells netCDF we want read-only access to
  ! the file.
  call check( nf90_open(FILE_NAME, NF90_NOWRITE, ncid) )

  ! Get the varid of the data variable, based on its name.
  call check( nf90_inq_varid(ncid, "data", varid) )

  ! Read the data.
  call check( nf90_get_var(ncid, varid, data_in) )

  ! Check the data.
  do x = 1, NX
     do y = 1, NY
        if (data_in(y, x) /= (x - 1) * NY + (y - 1)) then
           print *, "data_in(", y, ", ", x, ") = ", data_in(y, x)
           stop "Stopped"
        end if
     end do
  end do

  ! Close the file, freeing all resources.
  call check( nf90_close(ncid) )

  print *,"*** SUCCESS reading example file ", FILE_NAME, "! "

contains
  subroutine check(status)
    integer, intent ( in) :: status
    
    if(status /= nf90_noerr) then 
      print *, trim(nf90_strerror(status))
      stop 2
    end if
  end subroutine check  
end program simple_xy_rd
