C     This is part of the netCDF package.
C     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.
      
C     This is a simple example which reads a small dummy array, from a
C     netCDF data file created by the companion program simple_xy_wr.f.
      
C     This is intended to illustrate the use of the netCDF fortran 77
C     API. This example program is part of the netCDF tutorial, which can
C     be found at:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial
      
C     Full documentation of the netCDF Fortran 77 API can be found at:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f77

C     $Id$

      program simple_xy_rd
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will read. 
      character*(*) FILE_NAME
      parameter (FILE_NAME='simple_xy_nc4.nc')

C     We are reading 2D data, a 12 x 6 grid. 
      integer NX, NY
      parameter (NX = 60, NY = 120)
      integer data_in(NY, NX)

C     This will be the netCDF ID for the file and data variable.
      integer ncid, varid

C     Loop indexes, and error handling.
      integer x, y, retval

C     Open the file. NF_NOWRITE tells netCDF we want read-only access to
C     the file.
      retval = nf_open(FILE_NAME, NF_NOWRITE, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Get the varid of the data variable, based on its name.
      retval = nf_inq_varid(ncid, 'data', varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Read the data.
      retval = nf_get_var_int(ncid, varid, data_in)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Check the data.
      do x = 1, NX
         do y = 1, NY
            if (data_in(y, x) .ne. (x - 1) * NY + (y - 1)) then
               print *, 'data_in(', y, ', ', x, ') = ', data_in(y, x)
               stop 2
            end if
         end do
      end do

C     Close the file, freeing all resources.
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      print *,'*** SUCCESS reading example file ', FILE_NAME, '!'
      end

      subroutine handle_err(errcode)
      implicit none
      include 'netcdf.inc'
      integer errcode

      print *, 'Error: ', nf_strerror(errcode)
      stop 2
      end
