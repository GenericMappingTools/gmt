C     This is part of the netCDF package.
C     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This is an example which reads some 4D pressure and
C     temperatures. The data file read by this program is produced by
C     the companion program pres_temp_4D_wr.f. It is intended to
C     illustrate the use of the netCDF Fortran 77 API.

C     This program is part of the netCDF tutorial:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

C     Full documentation of the netCDF Fortran 77 API can be found at:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f77

C     $Id$

      program pres_temp_4D_rd
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will read.
      character*(*) FILE_NAME
      parameter (FILE_NAME='pres_temp_4D.nc')
      integer ncid

C     We are reading 4D data, a 12 x 6 x 2 lon-lat-lvl grid, with 2
C     timesteps of data.
      integer NDIMS, NRECS
      parameter (NDIMS = 4, NRECS = 2)
      integer NLVLS, NLATS, NLONS
      parameter (NLVLS = 2, NLATS = 6, NLONS = 12)
      character*(*) LVL_NAME, LAT_NAME, LON_NAME, REC_NAME
      parameter (LVL_NAME = 'level')
      parameter (LAT_NAME = 'latitude', LON_NAME = 'longitude')
      parameter (REC_NAME = 'time')
      integer lvl_dimid, lon_dimid, lat_dimid, rec_dimid

C     The start and count arrays will tell the netCDF library where to
C     read our data.
      integer start(NDIMS), count(NDIMS)

C     In addition to the latitude and longitude dimensions, we will also
C     create latitude and longitude variables which will hold the actual
C     latitudes and longitudes. Since they hold data about the
C     coordinate system, the netCDF term for these is: "coordinate
C     variables."
      real lats(NLATS), lons(NLONS)
      integer lon_varid, lat_varid

C     We will read surface temperature and pressure fields. In netCDF
C     terminology these are called "variables."
      character*(*) PRES_NAME, TEMP_NAME
      parameter (PRES_NAME='pressure')
      parameter (TEMP_NAME='temperature')
      integer pres_varid, temp_varid
      integer dimids(NDIMS)

C     We recommend that each variable carry a "units" attribute.
      character*(*) UNITS
      parameter (UNITS = 'units')
      character*(*) PRES_UNITS, TEMP_UNITS, LAT_UNITS, LON_UNITS
      parameter (PRES_UNITS = 'hPa', TEMP_UNITS = 'celsius')
      parameter (LAT_UNITS = 'degrees_north')
      parameter (LON_UNITS = 'degrees_east')

C     Program variables to hold the data we will read in. We will only
C     need enough space to hold one timestep of data; one record.
      real pres_in(NLONS, NLATS, NLVLS)
      real temp_in(NLONS, NLATS, NLVLS)
      real SAMPLE_PRESSURE
      parameter (SAMPLE_PRESSURE = 900.0)
      real SAMPLE_TEMP
      parameter (SAMPLE_TEMP = 9.0)

C     Use these to calculate the values we expect to find.
      integer START_LAT, START_LON
      parameter (START_LAT = 25.0, START_LON = -125.0)

C     Loop indices.
      integer lvl, lat, lon, rec, i

C     Error handling.
      integer retval

C     Open the file. 
      retval = nf_open(FILE_NAME, nf_nowrite, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Get the varids of the latitude and longitude coordinate variables.
      retval = nf_inq_varid(ncid, LAT_NAME, lat_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_varid(ncid, LON_NAME, lon_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Read the latitude and longitude data.
      retval = nf_get_var_real(ncid, lat_varid, lats)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_var_real(ncid, lon_varid, lons)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Check to make sure we got what we expected.
      do lat = 1, NLATS
         if (lats(lat) .ne. START_LAT + (lat - 1) * 5.0) stop 2
      end do
      do lon = 1, NLONS
         if (lons(lon) .ne. START_LON + (lon - 1) * 5.0) stop 2
      end do

C     Get the varids of the pressure and temperature netCDF variables.
      retval = nf_inq_varid(ncid, PRES_NAME, pres_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_varid(ncid, TEMP_NAME, temp_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Read 1 record of NLONS*NLATS*NLVLS values, starting at the
C     beginning of the record (the (1, 1, 1, rec) element in the netCDF
C     file).
      count(1) = NLONS
      count(2) = NLATS
      count(3) = NLVLS
      count(4) = 1
      start(1) = 1
      start(2) = 1
      start(3) = 1

C     Read the surface pressure and temperature data from the file, one
C     record at a time.
      do rec = 1, NRECS
         start(4) = rec
         retval = nf_get_vara_real(ncid, pres_varid, start, count,
     $        pres_in)
         if (retval .ne. nf_noerr) call handle_err(retval)
         retval = nf_get_vara_real(ncid, temp_varid, start, count,
     $        temp_in)
         if (retval .ne. nf_noerr) call handle_err(retval)

         i = 0
         do lvl = 1, NLVLS
            do lat = 1, NLATS
               do lon = 1, NLONS
                  if (pres_in(lon, lat, lvl) .ne. SAMPLE_PRESSURE + i) 
     $                 stop 2
                  if (temp_in(lon, lat, lvl) .ne. SAMPLE_TEMP + i)
     $                 stop 2
                  i = i + 1
               end do
            end do
         end do
C     next record
      end do
         
C     Close the file. This frees up any internal netCDF resources
C     associated with the file.
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     If we got this far, everything worked as expected. Yipee!
      print *,'*** SUCCESS reading example file pres_temp_4D.nc!'
      end

      subroutine handle_err(errcode)
      implicit none
      include 'netcdf.inc'
      integer errcode

      print *, 'Error: ', nf_strerror(errcode)
      stop 2
      end
