!     This is part of the netCDF package.
!     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
!     See COPYRIGHT file for conditions of use.

!     This is a very simple example which writes a 2D array of sample
!     data. To handle this in netCDF we create two shared dimensions,
!     "x" and "y", and a netCDF variable, called "data". It uses
!     parallel I/O to write the file from all processors at the same
!     time.

!     This example demonstrates the netCDF Fortran 90 API. This is part
!     of the netCDF tutorial, which can be found at:
!     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial
      
!     Full documentation of the netCDF Fortran 90 API can be found at:
!     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f90

!     $Id$

program simple_xy_par_wr
  use netcdf
  implicit none
  include 'mpif.h'

  ! This is the name of the data file we will create.
  character (len = *), parameter :: FILE_NAME = "simple_xy_par.nc"

  ! We are writing 2D data.
  integer, parameter :: NDIMS = 2

  ! When we create netCDF files, variables and dimensions, we get back
  ! an ID for each one.
  integer :: ncid, varid, dimids(NDIMS)
  integer :: x_dimid, y_dimid

  ! These will tell where in the data file this processor should
  ! write.
  integer :: start(NDIMS), count(NDIMS)
  
  ! This is the data array we will write. It will just be filled with
  ! the rank of this processor.
  integer, allocatable :: data_out(:)

  ! MPI stuff: number of processors, rank of this processor, and error
  ! code.
  integer :: p, my_rank, ierr

  ! Loop indexes, and error handling.
  integer :: x, stat

  ! Initialize MPI, learn local rank and total number of processors.
  call MPI_Init(ierr)
  call MPI_Comm_rank(MPI_COMM_WORLD, my_rank, ierr)
  call MPI_Comm_size(MPI_COMM_WORLD, p, ierr)

  ! Create some pretend data. We just need one row.
  allocate(data_out(p), stat = stat)
  if (stat .ne. 0) stop 3
  do x = 1, p
     data_out(x) = my_rank
  end do

  ! Create the netCDF file. The NF90_NETCDF4 flag causes a
  ! HDF5/netCDF-4 file to be created. The comm and info parameters
  ! cause parallel I/O to be enabled. Use either NF90_MPIIO or
  ! NF90_MPIPOSIX to select between MPI/IO and MPI/POSIX.
  call check(nf90_create(FILE_NAME, IOR(NF90_NETCDF4, NF90_MPIIO), ncid, &
       comm = MPI_COMM_WORLD, info = MPI_INFO_NULL))

  ! Define the dimensions. NetCDF will hand back an ID for
  ! each. Metadata operations must take place on all processors.
  call check(nf90_def_dim(ncid, "x", p, x_dimid))
  call check(nf90_def_dim(ncid, "y", p, y_dimid))

  ! The dimids array is used to pass the IDs of the dimensions of
  ! the variables. Note that in fortran arrays are stored in
  ! column-major format.
  dimids = (/ y_dimid, x_dimid /)

  ! Define the variable. The type of the variable in this case is
  ! NF90_INT (4-byte integer).
  call check(nf90_def_var(ncid, "data", NF90_INT, dimids, varid))

  ! End define mode. This tells netCDF we are done defining
  ! metadata. This operation is collective and all processors will
  ! write their metadata to disk.
  call check(nf90_enddef(ncid))

  ! Write the pretend data to the file. Each processor writes one row.
  start = (/ 1, my_rank + 1/)
  count = (/ p, 1 /)
  call check(nf90_put_var(ncid, varid, data_out, start = start, &
       count = count))

  ! Close the file. This frees up any internal netCDF resources
  ! associated with the file, and flushes any buffers.
  call check( nf90_close(ncid) )

  ! Free my local memory.
  deallocate(data_out)

  ! MPI library must be shut down.
  call MPI_Finalize(ierr)

  if (my_rank .eq. 0) print *, "*** SUCCESS writing example file ", FILE_NAME, "! "

contains
  subroutine check(status)
    integer, intent ( in) :: status
    
    if(status /= nf90_noerr) then 
      print *, trim(nf90_strerror(status))
      stop 2
    end if
  end subroutine check  
end program simple_xy_par_wr
