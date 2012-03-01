! This is part of the netCDF package.
! Copyright 2008 University Corporation for Atmospheric Research/Unidata.
! See COPYRIGHT file for conditions of use.
      
! This is a simple example which reads a small dummy array, from a
! netCDF data file created by the companion program
! simple_xy_par_wr.f90. The data are read using parallel I/O.
      
! This is intended to illustrate the use of the netCDF fortran 90
! API. This example program is part of the netCDF tutorial, which can
! be found at:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial
      
! Full documentation of the netCDF Fortran 90 API can be found at:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f90

! $Id$

program simple_xy_par_rd
  use netcdf
  implicit none
  include 'mpif.h'

  ! This is the name of the data file we will read. 
  character (len = *), parameter :: FILE_NAME = "simple_xy_par.nc"

  ! These will tell where in the data file this processor should
  ! write.
  integer, parameter :: NDIMS = 2
  integer :: start(NDIMS), count(NDIMS)
  
  ! We will read data into this array.
  integer, allocatable :: data_in(:)

  ! This will be the netCDF ID for the file and data variable.
  integer :: ncid, varid

  ! MPI stuff: number of processors, rank of this processor, and error
  ! code.
  integer :: p, my_rank, ierr

  ! Loop indexes, and error handling.
  integer :: x, y, stat

  ! Initialize MPI, learn local rank and total number of processors.
  call MPI_Init(ierr)
  call MPI_Comm_rank(MPI_COMM_WORLD, my_rank, ierr)
  call MPI_Comm_size(MPI_COMM_WORLD, p, ierr)

  ! Allocate space to read in data.
  allocate(data_in(p), stat = stat)
  if (stat .ne. 0) stop 3

  ! Open the file. NF90_NOWRITE tells netCDF we want read-only access to
  ! the file.
  call check( nf90_open(FILE_NAME, IOR(NF90_NOWRITE, NF90_MPIIO), ncid, &
       comm = MPI_COMM_WORLD, info = MPI_INFO_NULL) )

  ! Get the varid of the data variable, based on its name.
  call check( nf90_inq_varid(ncid, "data", varid) )

  ! Read the data.
  start = (/ 1, my_rank + 1/)
  count = (/ p, 1 /)
  call check( nf90_get_var(ncid, varid, data_in, &
       start = start, count = count) )

  ! Check the data.
  do x = 1, p
     if (data_in(x) .ne. my_rank) then
        print *, "data_in(", x, ") = ", data_in(x)
        stop "Stopped"
     endif
  end do

  ! Close the file, freeing all resources.
  call check( nf90_close(ncid) )

  ! Free my local memory.
  deallocate(data_in)

  ! MPI library must be shut down.
  call MPI_Finalize(ierr)

  if (my_rank .eq. 0) print *,"*** SUCCESS reading example file ", FILE_NAME, "! "

contains
  subroutine check(status)
    integer, intent ( in) :: status
    
    if(status /= nf90_noerr) then 
      print *, trim(nf90_strerror(status))
      stop 2
    end if
  end subroutine check  
end program simple_xy_par_rd
