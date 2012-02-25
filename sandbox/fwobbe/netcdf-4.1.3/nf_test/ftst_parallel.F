!     This is part of the netCDF package.
!     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
!     See COPYRIGHT file for conditions of use.

!     This program tests netCDF-4 variable functions from fortran.

!     $Id$

      program ftst_parallel
      implicit none
      include 'netcdf.inc'
      include 'mpif.h'
      
      character*(*) FILE_NAME
      parameter (FILE_NAME = 'ftst_parallel.nc')

      integer MAX_DIMS
      parameter (MAX_DIMS = 2)
      integer NX, NY
      parameter (NX = 16)
      parameter (NY = 16)
      integer NUM_PROC
      parameter (NUM_PROC = 4)
      integer ncid, varid, dimids(MAX_DIMS)
      integer x_dimid, y_dimid
      integer data_out(NY / 2, NX / 2), data_in(NY / 2, NX / 2)
      integer mode_flag
      integer x, y, retval
      integer p, my_rank, ierr
      integer start(MAX_DIMS), count(MAX_DIMS)

      call MPI_Init(ierr)
      call MPI_Comm_rank(MPI_COMM_WORLD, my_rank, ierr)
      call MPI_Comm_size(MPI_COMM_WORLD, p, ierr)

      if (my_rank .eq. 0) then
         print *, ' '
         print *, '*** Testing netCDF-4 parallel I/O from Fortran 77.'
      endif

!     There must be 4 procs for this test.
      if (p .ne. 4) then
         print *, 'This test program must be run on 4 processors.'
         stop 2 
      endif

!     Create some pretend data.
      do x = 1, NX / 2
         do y = 1, NY / 2
            data_out(y, x) = my_rank
         end do
      end do

!     Create the netCDF file. 
      mode_flag = IOR(nf_netcdf4, nf_classic_model) 
      mode_flag = IOR(mode_flag, nf_mpiio) 
      retval = nf_create_par(FILE_NAME, mode_flag, MPI_COMM_WORLD, 
     $     MPI_INFO_NULL, ncid)
      if (retval .ne. nf_noerr) stop 3

!     Define the dimensions.
      retval = nf_def_dim(ncid, "x", NX, x_dimid)
      if (retval .ne. nf_noerr) stop 4
      retval = nf_def_dim(ncid, "y", NY, y_dimid)
      if (retval .ne. nf_noerr) stop 5
      dimids(1) = y_dimid
      dimids(2) = x_dimid

!     Define the variable. 
      retval = nf_def_var(ncid, "data", NF_INT, MAX_DIMS, dimids, varid)
      if (retval .ne. nf_noerr) stop 6

!     With classic model netCDF-4 file, enddef must be called.
      retval = nf_enddef(ncid)
      if (retval .ne. nf_noerr) stop 7

!     Determine what part of the variable will be written for this
!     processor. It's a checkerboard decomposition.
      count(1) = NX / 2
      count(2) = NY / 2
      if (my_rank .eq. 0) then
         start(1) = 1
         start(2) = 1
      else if (my_rank .eq. 1) then
         start(1) = NX / 2 + 1
         start(2) = 1
      else if (my_rank .eq. 2) then
         start(1) = 1
         start(2) = NY / 2 + 1
      else if (my_rank .eq. 3) then
         start(1) = NX / 2 + 1
         start(2) = NY / 2 + 1
      endif

!     Write this processor's data.
      retval = nf_put_vara_int(ncid, varid, start, count, data_out)
      if (retval .ne. nf_noerr) stop 8

!     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) stop 9

!     Reopen the file.
      retval = nf_open_par(FILE_NAME, nf_nowrite, 
     $     MPI_COMM_WORLD, MPI_INFO_NULL, ncid)
      if (retval .ne. nf_einval) stop 7
      retval = nf_open_par(FILE_NAME, IOR(nf_nowrite, nf_mpiio), 
     $     MPI_COMM_WORLD, MPI_INFO_NULL, ncid)
      if (retval .ne. nf_noerr) stop 10

!     Set collective access on this variable. This will cause all
!     reads/writes to happen together on every processor. Fairly
!     pointless, in this contexct, but I want to at least call this
!     function once in my testing.
      retval = nf_var_par_access(ncid, varid, nf_collective)
      if (retval .ne. nf_noerr) stop 11
      
!     Read this processor's data.
      retval = nf_get_vara_int(ncid, varid, start, count, data_in)
      if (retval .ne. nf_noerr) stop 12

!     Check the data.
      do x = 1, NX / 2
         do y = 1, NY / 2
            if (data_in(y, x) .ne. my_rank) stop 13
         end do
      end do

!     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) stop 14

      call MPI_Finalize(ierr)

      if (my_rank .eq. 0) print *,'*** SUCCESS!'

      end program ftst_parallel

