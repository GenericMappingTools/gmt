C     This is part of the netCDF package.
C     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This example writes some surface pressure and temperatures. It is
C     intended to illustrate the use of the netCDF fortran 77 API. The
C     companion program sfc_pres_temp_rd.f shows how to read the netCDF
C     data file created by this program.

C     This program is part of the netCDF tutorial:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

C     Full documentation of the netCDF Fortran 77 API can be found at:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f77

C     $Id$

      program sfc_pres_temp_wr
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will create.
      character*(*) FILE_NAME
      parameter (FILE_NAME='sfc_pres_temp.nc')
      integer ncid

C     We are writing 2D data, a 12 x 6 lon-lat grid. We will need two
C     netCDF dimensions.
      integer NDIMS
      parameter (NDIMS=2)
      integer NLATS, NLONS
      parameter (NLATS = 6, NLONS = 12)
      character*(*) LAT_NAME, LON_NAME
      parameter (LAT_NAME='latitude', LON_NAME='longitude')
      integer lon_dimid, lat_dimid

C     In addition to the latitude and longitude dimensions, we will also
C     create latitude and longitude netCDF variables which will hold the
C     actual latitudes and longitudes. Since they hold data about the
C     coordinate system, the netCDF term for these is: "coordinate
C     variables."
      real lats(NLATS), lons(NLONS)
      integer lat_varid, lon_varid
      real START_LAT, START_LON
      parameter (START_LAT = 25.0, START_LON = -125.0)

C     We will write surface temperature and pressure fields. 
      character*(*) PRES_NAME, TEMP_NAME
      parameter (PRES_NAME='pressure')
      parameter (TEMP_NAME='temperature')
      integer pres_varid, temp_varid
      integer dimids(NDIMS)

C     It's good practice for each variable to carry a "units" attribute.
      character*(*) UNITS
      parameter (UNITS = 'units')
      character*(*) PRES_UNITS, TEMP_UNITS, LAT_UNITS, LON_UNITS
      parameter (PRES_UNITS = 'hPa', TEMP_UNITS = 'celsius')
      parameter (LAT_UNITS = 'degrees_north')
      parameter (LON_UNITS = 'degrees_east')

C     We will create some pressure and temperature data to write out.
      real pres_out(NLONS, NLATS), temp_out(NLONS, NLATS)
      real SAMPLE_PRESSURE
      parameter (SAMPLE_PRESSURE = 900.0)
      real SAMPLE_TEMP
      parameter (SAMPLE_TEMP = 9.0)

C     Loop indices.
      integer lat, lon

C     Error handling.
      integer retval

C     Create pretend data. If this were not an example program, we would
C     have some real data to write, for example, model output.
      do lat = 1, NLATS
         lats(lat) = START_LAT + (lat - 1) * 5.0
      end do
      do lon = 1, NLONS
         lons(lon) = START_LON + (lon - 1) * 5.0
      end do
      do lon = 1, NLONS
         do lat = 1, NLATS
            pres_out(lon, lat) = SAMPLE_PRESSURE + 
     +           (lon - 1) * NLATS + (lat - 1)
            temp_out(lon, lat) = SAMPLE_TEMP + 
     +           .25 * ((lon - 1) * NLATS + (lat - 1))
         end do
      end do

C     Create the file. 
      retval = nf_create(FILE_NAME, nf_clobber, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define the dimensions.
      retval = nf_def_dim(ncid, LAT_NAME, NLATS, lat_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_dim(ncid, LON_NAME, NLONS, lon_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define the coordinate variables. They will hold the coordinate
C     information, that is, the latitudes and longitudes. A varid is
C     returned for each.
      retval = nf_def_var(ncid, LAT_NAME, NF_REAL, 1, lat_dimid, 
     +     lat_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_var(ncid, LON_NAME, NF_REAL, 1, lon_dimid, 
     +     lon_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Assign units attributes to coordinate var data. This attaches a
C     text attribute to each of the coordinate variables, containing the
C     units.
      retval = nf_put_att_text(ncid, lat_varid, UNITS, len(LAT_UNITS), 
     +     LAT_UNITS)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_put_att_text(ncid, lon_varid, UNITS, len(LON_UNITS), 
     +     LON_UNITS)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define the netCDF variables. The dimids array is used to pass the
C     dimids of the dimensions of the netCDF variables.
      dimids(1) = lon_dimid
      dimids(2) = lat_dimid
      retval = nf_def_var(ncid, PRES_NAME, NF_REAL, NDIMS, dimids, 
     +     pres_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_var(ncid, TEMP_NAME, NF_REAL, NDIMS, dimids, 
     +     temp_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Assign units attributes to the pressure and temperature netCDF
C     variables.
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

C     Write the pretend data. This will write our surface pressure and
C     surface temperature data. The arrays of data are the same size as
C     the netCDF variables we have defined.
      retval = nf_put_var_real(ncid, pres_varid, pres_out)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_put_var_real(ncid, temp_varid, temp_out)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Close the file.
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)
   
C     If we got this far, everything worked as expected. Yipee!
      print *,'*** SUCCESS writing example file sfc_pres_temp.nc!'
      end

      subroutine handle_err(errcode)
      implicit none
      include 'netcdf.inc'
      integer errcode

      print *, 'Error: ', nf_strerror(errcode)
      stop 2
      end
