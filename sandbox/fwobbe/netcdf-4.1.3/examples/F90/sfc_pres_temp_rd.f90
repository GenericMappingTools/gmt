! This is part of the netCDF package.
! Copyright 2006 University Corporation for Atmospheric Research/Unidata.
! See COPYRIGHT file for conditions of use.

! This is an example which reads some surface pressure and
! temperatures. The data file read by this program is produced
! comapnion program sfc_pres_temp_wr.f90. It is intended to illustrate
! the use of the netCDF fortran 90 API.

! This program is part of the netCDF tutorial:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

! Full documentation of the netCDF Fortran 90 API can be found at:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f90

! $Id$

program sfc_pres_temp_rd
  use netcdf
  implicit none

  ! This is the name of the data file we will read.
  character (len = *), parameter :: FILE_NAME = "sfc_pres_temp.nc"
  integer :: ncid

  ! We are reading 2D data, a 12 x 6 lon-lat grid.
  integer, parameter :: NDIMS = 2
  integer, parameter :: NLATS = 6, NLONS = 12
  character (len = *), parameter :: LAT_NAME = "latitude"
  character (len = *), parameter :: LON_NAME = "longitude"

  ! For the lat lon coordinate netCDF variables.
  real :: lats(NLATS), lons(NLONS)
  integer :: lat_varid, lon_varid

  ! We will read surface temperature and pressure fields. 
  character (len = *), parameter :: PRES_NAME = "pressure"
  character (len = *), parameter :: TEMP_NAME = "temperature"
  integer :: pres_varid, temp_varid

  ! To check the units attributes.
  character (len = *), parameter :: UNITS = "units"
  character (len = *), parameter :: PRES_UNITS = "hPa"
  character (len = *), parameter :: TEMP_UNITS = "celsius"
  character (len = *), parameter :: LAT_UNITS = "degrees_north"
  character (len = *), parameter :: LON_UNITS = "degrees_east"
  integer, parameter :: MAX_ATT_LEN = 80
  integer :: att_len
  character*(MAX_ATT_LEN) :: pres_units_in, temp_units_in
  character*(MAX_ATT_LEN) :: lat_units_in, lon_units_in

  ! Read the data into these arrays.
  real, dimension(:,:), allocatable :: pres_in
  real, dimension(:,:), allocatable :: temp_in

  ! These are used to calculate the values we expect to find.
  real, parameter :: START_LAT = 25.0, START_LON = -125.0
  real, parameter :: SAMPLE_PRESSURE = 900.0
  real, parameter :: SAMPLE_TEMP = 9.0

  ! We will learn about the data file and store results in these
  ! program variables.
  integer :: ndims_in, nvars_in, ngatts_in, unlimdimid_in

  ! Loop indices
  integer :: lat, lon

  ! Allocate memory.
  allocate(pres_in(NLONS, NLATS))
  allocate(temp_in(NLONS, NLATS))

  ! Open the file. 
  call check( nf90_open(FILE_NAME, nf90_nowrite, ncid) )

  ! There are a number of inquiry functions in netCDF which can be
  ! used to learn about an unknown netCDF file. NF90_INQ tells how many
  ! netCDF variables, dimensions, and global attributes are in the
  ! file; also the dimension id of the unlimited dimension, if there
  ! is one.
  call check( nf90_inquire(ncid, ndims_in, nvars_in, ngatts_in, unlimdimid_in) )

  ! In this case we know that there are 2 netCDF dimensions, 4 netCDF
  ! variables, no global attributes, and no unlimited dimension.
  if (ndims_in /= 2 .or. nvars_in /= 4 .or. ngatts_in /= 0 &
       .or. unlimdimid_in /= -1) stop 2

  ! Get the varids of the latitude and longitude coordinate variables.
  call check( nf90_inq_varid(ncid, LAT_NAME, lat_varid) )
  call check( nf90_inq_varid(ncid, LON_NAME, lon_varid) )

  ! Read the latitude and longitude data.
  call check( nf90_get_var(ncid, lat_varid, lats) )
  call check( nf90_get_var(ncid, lon_varid, lons) )

  ! Check to make sure we got what we expected.
  do lat = 1, NLATS
     if (lats(lat) /= START_LAT + (lat - 1) * 5.0) stop 2
  end do
  do lon = 1, NLONS
     if (lons(lon) /= START_LON + (lon - 1) * 5.0) stop 2
  end do

  ! Get the varids of the pressure and temperature netCDF variables.
  call check( nf90_inq_varid(ncid, PRES_NAME, pres_varid) )
  call check( nf90_inq_varid(ncid, TEMP_NAME, temp_varid) )

  ! Read the surface pressure and temperature data from the file.
  ! Since we know the contents of the file we know that the data
  ! arrays in this program are the correct size to hold all the data.
  call check( nf90_get_var(ncid, pres_varid, pres_in) )
  call check( nf90_get_var(ncid, temp_varid, temp_in) )

  ! Check the data. It should be the same as the data we wrote.
  do lon = 1, NLONS
     do lat = 1, NLATS
        if (pres_in(lon, lat) /= SAMPLE_PRESSURE + &
             (lon - 1) * NLATS + (lat - 1)) stop 2
        if (temp_in(lon, lat) /= SAMPLE_TEMP + &
             .25 * ((lon - 1) * NLATS + (lat - 1))) stop 2
     end do
  end do

  ! Each of the netCDF variables has a "units" attribute. Let's read
  ! them and check them.
  call check( nf90_get_att(ncid, lat_varid, UNITS, lat_units_in) )
  call check( nf90_inquire_attribute(ncid, lat_varid, UNITS, len = att_len) )
  if (lat_units_in(1:att_len) /= LAT_UNITS) stop 2

  call check( nf90_get_att(ncid, lon_varid, UNITS, lon_units_in) )
  call check( nf90_inquire_attribute(ncid, lon_varid, UNITS, len = att_len) )
  if (lon_units_in(1:att_len) /= LON_UNITS) stop 2

  call check( nf90_get_att(ncid, pres_varid, UNITS, pres_units_in) )
  call check( nf90_inquire_attribute(ncid, pres_varid, UNITS, len = att_len) )
  if (pres_units_in(1:att_len) /= PRES_UNITS) stop 2

  call check( nf90_get_att(ncid, temp_varid, UNITS, temp_units_in) )
  call check( nf90_inquire_attribute(ncid, temp_varid, UNITS, len = att_len) )
  if (temp_units_in(1:att_len) /= TEMP_UNITS) stop 2

  ! Close the file. This frees up any internal netCDF resources
  ! associated with the file.
  call check( nf90_close(ncid) )

  ! If we got this far, everything worked as expected. Yipee! 
  print *,"*** SUCCESS reading example file sfc_pres_temp.nc!"

contains
  subroutine check(status)
    integer, intent ( in) :: status
    
    if(status /= nf90_noerr) then 
      print *, trim(nf90_strerror(status))
      stop 2
    end if
  end subroutine check  
end program sfc_pres_temp_rd

