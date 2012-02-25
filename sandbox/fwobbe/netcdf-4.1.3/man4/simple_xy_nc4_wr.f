C     This is part of the netCDF package.
C     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This is a very simple example which writes a 2D array of
C     sample data. To handle this in netCDF we create two shared
C     dimensions, "x" and "y", and a netCDF variable, called "data".

C     This example demonstrates the netCDF Fortran 77 API. This is part
C     of the netCDF tutorial, which can be found at:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

C     Full documentation of the netCDF Fortran 77 API can be found at:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f77

C     $Id$

      program simple_xy_wr
      implicit none
      include 'netcdf.inc'

      character*(*) FILE_NAME
      parameter (FILE_NAME='simple_xy_nc4.nc')
      integer NDIMS
      parameter (NDIMS = 2)
      integer NX, NY
      parameter (NX = 60, NY = 120)
      integer ncid, varid, dimids(NDIMS)
      integer x_dimid, y_dimid
      integer data_out(NY, NX)
      integer x, y, retval
      integer chunks(2)
      integer contiguous, shuffle, deflate, deflate_level

C     Create some pretend data. If this wasn't an example program, we
C     would have some real data to write, for example, model output.
      do x = 1, NX
         do y = 1, NY
            data_out(y, x) = (x - 1) * NY + (y - 1)
         end do
      end do

C     Create the netCDF file. The nf_netcdf4  tells netCDF to
C     create a netCDF-4/HDF5 file.
      retval = nf_create(FILE_NAME, NF_NETCDF4, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define the dimensions.
      retval = nf_def_dim(ncid, "x", NX, x_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_dim(ncid, "y", NY, y_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     The dimids array is used to pass the IDs of the dimensions of
C     the variables. Note that in fortran arrays are stored in
C     column-major format.
      dimids(2) = x_dimid
      dimids(1) = y_dimid

C     Define the variable. 
      retval = nf_def_var(ncid, "data", NF_INT, NDIMS, dimids, varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Set up chunking and compression.
      contiguous = 0
      chunks(1) = NY
      chunks(2) = NX
      shuffle = 1
      deflate = 1
      deflate_level = 4

      retval = nf_def_var_chunking(ncid, varid, contiguous, chunks)
      if (retval .ne. nf_noerr) call handle_err(retval)

      retval = nf_def_var_deflate(ncid, varid, shuffle, deflate,
     &     deflate_level)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Write the pretend data to the file.
      retval = nf_put_var_int(ncid, varid, data_out)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Close the file.
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      print *,'*** SUCCESS writing example file simple_xy_nc4.nc!'
      end

      subroutine handle_err(errcode)
      implicit none
      include 'netcdf.inc'
      integer errcode

      print *, 'Error: ', nf_strerror(errcode)
      stop 2
      end
