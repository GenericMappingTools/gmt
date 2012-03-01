C     This is part of the netCDF package.
C     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This is an example program which writes some 4D pressure and
C     temperatures. It is intended to illustrate the use of the netCDF
C     fortran 77 API. The companion program pres_temp_4D_rd.f shows how
C     to read the netCDF data file created by this program.

C     This program is part of the netCDF tutorial:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

C     Full documentation of the netCDF Fortran 77 API can be found at:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f77

C     $Id$

      program pres_temp_4D_wr
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will create.
      character*(*) FILE_NAME
      parameter (FILE_NAME = 'pres_temp_4D.nc')
      integer ncid

C     We are writing 4D data, a 12 x 6 x 2 lon-lat-lvl grid, with 2
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
C     write our data.
      integer start(NDIMS), count(NDIMS)

C     These program variables hold the latitudes and longitudes.
      real lats(NLATS), lons(NLONS)
      integer lon_varid, lat_varid

C     We will create two netCDF variables, one each for temperature and
C     pressure fields.
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

C     Program variables to hold the data we will write out. We will only
C     need enough space to hold one timestep of data; one record.
      real pres_out(NLONS, NLATS, NLVLS)
      real temp_out(NLONS, NLATS, NLVLS)
      real SAMPLE_PRESSURE
      parameter (SAMPLE_PRESSURE = 900.0)
      real SAMPLE_TEMP
      parameter (SAMPLE_TEMP = 9.0)

C     Use these to construct some latitude and longitude data for this
C     example.
      integer START_LAT, START_LON
      parameter (START_LAT = 25.0, START_LON = -125.0)

C     Loop indices.
      integer lvl, lat, lon, rec, i

C     Error handling.
      integer retval

C     Create pretend data. If this wasn't an example program, we would
C     have some real data to write, for example, model output.
      do lat = 1, NLATS
         lats(lat) = START_LAT + (lat - 1) * 5.0
      end do
      do lon = 1, NLONS
         lons(lon) = START_LON + (lon - 1) * 5.0
      end do
      i = 0
      do lvl = 1, NLVLS
         do lat = 1, NLATS
            do lon = 1, NLONS
               pres_out(lon, lat, lvl) = SAMPLE_PRESSURE + i
               temp_out(lon, lat, lvl) = SAMPLE_TEMP + i
               i = i + 1
            end do
         end do
      end do

C     Create the file. 
      retval = nf_create(FILE_NAME, nf_clobber, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define the dimensions. The record dimension is defined to have
C     unlimited length - it can grow as needed. In this example it is
C     the time dimension.
      retval = nf_def_dim(ncid, LVL_NAME, NLVLS, lvl_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_dim(ncid, LAT_NAME, NLATS, lat_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_dim(ncid, LON_NAME, NLONS, lon_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_dim(ncid, REC_NAME, NF_UNLIMITED, rec_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define the coordinate variables. We will only define coordinate
C     variables for lat and lon.  Ordinarily we would need to provide
C     an array of dimension IDs for each variable's dimensions, but
C     since coordinate variables only have one dimension, we can
C     simply provide the address of that dimension ID (lat_dimid) and
C     similarly for (lon_dimid).
      retval = nf_def_var(ncid, LAT_NAME, NF_REAL, 1, lat_dimid, 
     +     lat_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_var(ncid, LON_NAME, NF_REAL, 1, lon_dimid, 
     +     lon_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Assign units attributes to coordinate variables.
      retval = nf_put_att_text(ncid, lat_varid, UNITS, len(LAT_UNITS), 
     +     LAT_UNITS)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_put_att_text(ncid, lon_varid, UNITS, len(LON_UNITS), 
     +     LON_UNITS)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     The dimids array is used to pass the dimids of the dimensions of
C     the netCDF variables. Both of the netCDF variables we are creating
C     share the same four dimensions. In Fortran, the unlimited
C     dimension must come last on the list of dimids.
      dimids(1) = lon_dimid
      dimids(2) = lat_dimid
      dimids(3) = lvl_dimid
      dimids(4) = rec_dimid

C     Define the netCDF variables for the pressure and temperature data.
      retval = nf_def_var(ncid, PRES_NAME, NF_REAL, NDIMS, dimids, 
     +     pres_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_var(ncid, TEMP_NAME, NF_REAL, NDIMS, dimids, 
     +     temp_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Assign units attributes to the netCDF variables.
      retval = nf_put_att_text(ncid, pres_varid, UNITS, len(PRES_UNITS), 
     +     PRES_UNITS)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_put_att_text(ncid, temp_varid, UNITS, len(TEMP_UNITS), 
     +     TEMP_UNITS)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     End define mode.
      retval = nf_enddef(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Write the coordinate variable data. This will put the latitudes
C     and longitudes of our data grid into the netCDF file.
      retval = nf_put_var_real(ncid, lat_varid, lats)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_put_var_real(ncid, lon_varid, lons)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     These settings tell netcdf to write one timestep of data. (The
C     setting of start(4) inside the loop below tells netCDF which
C     timestep to write.)
      count(1) = NLONS
      count(2) = NLATS
      count(3) = NLVLS
      count(4) = 1
      start(1) = 1
      start(2) = 1
      start(3) = 1

C     Write the pretend data. This will write our surface pressure and
C     surface temperature data. The arrays only hold one timestep worth
C     of data. We will just rewrite the same data for each timestep. In
C     a real application, the data would change between timesteps.
      do rec = 1, NRECS
         start(4) = rec
         retval = nf_put_vara_real(ncid, pres_varid, start, count, 
     +        pres_out)
         if (retval .ne. nf_noerr) call handle_err(retval)
         retval = nf_put_vara_real(ncid, temp_varid, start, count,
     +        temp_out)
         if (retval .ne. nf_noerr) call handle_err(retval)
      end do

C     Close the file. This causes netCDF to flush all buffers and make
C     sure your data are really written to disk.
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)
   
      print *,'*** SUCCESS writing example file', FILE_NAME, '!'
      end

      subroutine handle_err(errcode)
      implicit none
      include 'netcdf.inc'
      integer errcode

      print *, 'Error: ', nf_strerror(errcode)
      stop 2
      end
