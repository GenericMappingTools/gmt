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

C     This is the name of the data file we will create.
      character*(*) FILE_NAME
      parameter (FILE_NAME='simple_xy.nc')

C     We are writing 2D data, a 12 x 6 grid. 
      integer NDIMS
      parameter (NDIMS=2)
      integer NX, NY
      parameter (NX = 6, NY = 12)

C     When we create netCDF files, variables and dimensions, we get back
C     an ID for each one.
      integer ncid, varid, dimids(NDIMS)
      integer x_dimid, y_dimid

C     This is the data array we will write. It will just be filled with
C     a progression of integers for this example.
      integer data_out(NY, NX)

C     Loop indexes, and error handling.
      integer x, y, retval

C     Create some pretend data. If this wasn't an example program, we
C     would have some real data to write, for example, model output.
      do x = 1, NX
         do y = 1, NY
            data_out(y, x) = (x - 1) * NY + (y - 1)
         end do
      end do

C     Always check the return code of every netCDF function call. In
C     this example program, any retval which is not equal to nf_noerr
C     (0) will call handle_err, which prints a netCDF error message, and
C     then exits with a non-zero return code.

C     Create the netCDF file. The nf_clobber parameter tells netCDF to
C     overwrite this file, if it already exists.
      retval = nf_create(FILE_NAME, NF_CLOBBER, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define the dimensions. NetCDF will hand back an ID for each. 
      retval = nf_def_dim(ncid, "x", NX, x_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_dim(ncid, "y", NY, y_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     The dimids array is used to pass the IDs of the dimensions of
C     the variables. Note that in fortran arrays are stored in
C     column-major format.
      dimids(2) = x_dimid
      dimids(1) = y_dimid

C     Define the variable. The type of the variable in this case is
C     NF_INT (4-byte integer).
      retval = nf_def_var(ncid, "data", NF_INT, NDIMS, dimids, varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     End define mode. This tells netCDF we are done defining metadata.
      retval = nf_enddef(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Write the pretend data to the file. Although netCDF supports
C     reading and writing subsets of data, in this case we write all the
C     data in one operation.
      retval = nf_put_var_int(ncid, varid, data_out)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Close the file. This frees up any internal netCDF resources
C     associated with the file, and flushes any buffers.
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      print *,'*** SUCCESS writing example file simple_xy.nc!'
      end

      subroutine handle_err(errcode)
      implicit none
      include 'netcdf.inc'
      integer errcode

      print *, 'Error: ', nf_strerror(errcode)
      stop 2
      end
