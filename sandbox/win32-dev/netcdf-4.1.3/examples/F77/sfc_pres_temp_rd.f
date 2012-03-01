C     This is part of the netCDF package.
C     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This is an example which reads some surface pressure and
C     temperatures. The data file read by this program is produced
C     comapnion program sfc_pres_temp_wr.f. It is intended to illustrate
C     the use of the netCDF fortran 77 API.

C     This program is part of the netCDF tutorial:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

C     Full documentation of the netCDF Fortran 77 API can be found at:
C     http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f77

C     $Id$

      program sfc_pres_temp_rd
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will read.
      character*(*) FILE_NAME
      parameter (FILE_NAME='sfc_pres_temp.nc')
      integer ncid

C     We are reading 2D data, a 12 x 6 lon-lat grid.
      integer NDIMS
      parameter (NDIMS=2)
      integer NLATS, NLONS
      parameter (NLATS = 6, NLONS = 12)
      character*(*) LAT_NAME, LON_NAME
      parameter (LAT_NAME='latitude', LON_NAME='longitude')
      integer lat_dimid, lon_dimid

C     For the lat lon coordinate netCDF variables.
      real lats(NLATS), lons(NLONS)
      integer lat_varid, lon_varid

C     We will read surface temperature and pressure fields. 
      character*(*) PRES_NAME, TEMP_NAME
      parameter (PRES_NAME='pressure')
      parameter (TEMP_NAME='temperature')
      integer pres_varid, temp_varid
      integer dimids(NDIMS)

C     To check the units attributes.
      character*(*) UNITS
      parameter (UNITS = 'units')
      character*(*) PRES_UNITS, TEMP_UNITS, LAT_UNITS, LON_UNITS
      parameter (PRES_UNITS = 'hPa', TEMP_UNITS = 'celsius')
      parameter (LAT_UNITS = 'degrees_north')
      parameter (LON_UNITS = 'degrees_east')
      integer MAX_ATT_LEN
      parameter (MAX_ATT_LEN = 80)
      character*(MAX_ATT_LEN) pres_units_in, temp_units_in
      character*(MAX_ATT_LEN) lat_units_in, lon_units_in
      integer att_len

C     Read the data into these arrays.
      real pres_in(NLONS, NLATS), temp_in(NLONS, NLATS)

C     These are used to calculate the values we expect to find.
      real START_LAT, START_LON
      parameter (START_LAT = 25.0, START_LON = -125.0)
      real SAMPLE_PRESSURE
      parameter (SAMPLE_PRESSURE = 900.0)
      real SAMPLE_TEMP
      parameter (SAMPLE_TEMP = 9.0)

C     We will learn about the data file and store results in these
C     program variables.
      integer ndims_in, nvars_in, ngatts_in, unlimdimid_in

C     Loop indices
      integer lat, lon

C     Error handling
      integer retval

C     Open the file. 
      retval = nf_open(FILE_NAME, nf_nowrite, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     There are a number of inquiry functions in netCDF which can be
C     used to learn about an unknown netCDF file. NF_INQ tells how many
C     netCDF variables, dimensions, and global attributes are in the
C     file; also the dimension id of the unlimited dimension, if there
C     is one.
      retval = nf_inq(ncid, ndims_in, nvars_in, ngatts_in, 
     +     unlimdimid_in)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     In this case we know that there are 2 netCDF dimensions, 4 netCDF
C     variables, no global attributes, and no unlimited dimension.
      if (ndims_in .ne. 2 .or. nvars_in .ne. 4 .or. ngatts_in .ne. 0 
     +     .or. unlimdimid_in .ne. -1) stop 2

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

C     Read the surface pressure and temperature data from the file.
C     Since we know the contents of the file we know that the data
C     arrays in this program are the correct size to hold all the data.
      retval = nf_get_var_real(ncid, pres_varid, pres_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_var_real(ncid, temp_varid, temp_in)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Check the data. It should be the same as the data we wrote.
      do lon = 1, NLONS
         do lat = 1, NLATS
             if (pres_in(lon, lat) .ne. SAMPLE_PRESSURE +
     +           (lon - 1) * NLATS + (lat - 1)) stop 2
             if (temp_in(lon, lat) .ne. SAMPLE_TEMP +
     +           .25 * ((lon - 1) * NLATS + (lat - 1))) stop 2
         end do
      end do

C     Each of the netCDF variables has a "units" attribute. Let's read
C     them and check them.

      retval = nf_get_att_text(ncid, lat_varid, UNITS, lat_units_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_attlen(ncid, lat_varid, UNITS, att_len)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (lat_units_in(1:att_len) .ne. LAT_UNITS) stop 2
 
      retval = nf_get_att_text(ncid, lon_varid, UNITS, lon_units_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_attlen(ncid, lon_varid, UNITS, att_len)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (lon_units_in(1:att_len) .ne. LON_UNITS) stop 2

      retval = nf_get_att_text(ncid, pres_varid, UNITS, pres_units_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_attlen(ncid, pres_varid, UNITS, att_len)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (pres_units_in(1:att_len) .ne. PRES_UNITS) stop 2

      retval = nf_get_att_text(ncid, temp_varid, UNITS, temp_units_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_attlen(ncid, temp_varid, UNITS, att_len)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (temp_units_in(1:att_len) .ne. TEMP_UNITS) stop 2

C     Close the file. This frees up any internal netCDF resources
C     associated with the file.
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     If we got this far, everything worked as expected. Yipee!
      print *,'*** SUCCESS reading example file sfc_pres_temp.nc!'
      end

      subroutine handle_err(errcode)
      implicit none
      include 'netcdf.inc'
      integer errcode

      print *, 'Error: ', nf_strerror(errcode)
      stop 2
      end
