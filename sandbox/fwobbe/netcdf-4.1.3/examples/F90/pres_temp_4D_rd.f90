! This is part of the netCDF package.
! Copyright 2006 University Corporation for Atmospheric Research/Unidata.
! See COPYRIGHT file for conditions of use.

! This is an example which reads some 4D pressure and
! temperatures. The data file read by this program is produced by
! the companion program pres_temp_4D_wr.f90. It is intended to
! illustrate the use of the netCDF Fortran 90 API.

! This program is part of the netCDF tutorial:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

! Full documentation of the netCDF Fortran 90 API can be found at:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f90

! $Id$

program pres_temp_4D_rd
  use netcdf
  implicit none
  
  ! This is the name of the data file we will read.
  character (len = *), parameter :: FILE_NAME = "pres_temp_4D.nc"
  integer :: ncid

  ! We are reading 4D data, a 12 x 6 x 2 lon-lat-lvl grid, with 2
  ! timesteps of data.
  integer, parameter :: NDIMS = 4, NRECS = 2
  integer, parameter :: NLVLS = 2, NLATS = 6, NLONS = 12
  character (len = *), parameter :: LVL_NAME = "level"
  character (len = *), parameter :: LAT_NAME = "latitude"
  character (len = *), parameter :: LON_NAME = "longitude"
  character (len = *), parameter :: REC_NAME = "time"

  ! The start and count arrays will tell the netCDF library where to
  ! read our data.
  integer :: start(NDIMS), count(NDIMS)

  ! In addition to the latitude and longitude dimensions, we will also
  ! create latitude and longitude variables which will hold the actual
  ! latitudes and longitudes. Since they hold data about the
  ! coordinate system, the netCDF term for these is: "coordinate
  ! variables."
  real :: lats(NLATS), lons(NLONS)
  integer :: lon_varid, lat_varid

  ! We will read surface temperature and pressure fields. In netCDF
  ! terminology these are called "variables."
  character (len = *), parameter :: PRES_NAME="pressure"
  character (len = *), parameter :: TEMP_NAME="temperature"
  integer :: pres_varid, temp_varid

  ! We recommend that each variable carry a "units" attribute.
  character (len = *), parameter :: UNITS = "units"
  character (len = *), parameter :: PRES_UNITS = "hPa", TEMP_UNITS = "celsius"
  character (len = *), parameter :: LAT_UNITS = "degrees_north"
  character (len = *), parameter :: LON_UNITS = "degrees_east"

  ! Program variables to hold the data we will read in. We will only
  ! need enough space to hold one timestep of data; one record.
  ! Allocate memory for data.
  real, dimension(:,:,:), allocatable :: pres_in
  real, dimension(:,:,:), allocatable :: temp_in
  real, parameter :: SAMPLE_PRESSURE = 900.0
  real, parameter :: SAMPLE_TEMP = 9.0

  ! Use these to calculate the values we expect to find.
  real, parameter :: START_LAT = 25.0, START_LON = -125.0

  ! Loop indices
  integer :: lvl, lat, lon, rec, i

  ! Allocate memory.
  allocate(pres_in(NLONS, NLATS, NLVLS))
  allocate(temp_in(NLONS, NLATS, NLVLS))

  ! Open the file. 
  call check( nf90_open(FILE_NAME, nf90_nowrite, ncid) )

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

  ! Read 1 record of NLONS*NLATS*NLVLS values, starting at the beginning 
  ! of the record (the (1, 1, 1, rec) element in the netCDF file).
  count = (/ NLONS, NLATS, NLVLS, 1 /)
  start = (/ 1, 1, 1, 1 /)

  ! Read the surface pressure and temperature data from the file, one
  ! record at a time.
  do rec = 1, NRECS
     start(4) = rec
     call check( nf90_get_var(ncid, pres_varid, pres_in, start = start, &
                              count = count) )
     call check( nf90_get_var(ncid, temp_varid, temp_in, start, count) )
     
     i = 0
     do lvl = 1, NLVLS
        do lat = 1, NLATS
           do lon = 1, NLONS
              if (pres_in(lon, lat, lvl) /= SAMPLE_PRESSURE + i) stop 2
              if (temp_in(lon, lat, lvl) /= SAMPLE_TEMP + i) stop 2
              i = i + 1
           end do
        end do
     end do
     ! next record
  end do
         
  ! Close the file. This frees up any internal netCDF resources
  ! associated with the file.
  call check( nf90_close(ncid) )

  ! If we got this far, everything worked as expected. Yipee! 
  print *,"*** SUCCESS reading example file ", FILE_NAME, "!"

contains
  subroutine check(status)
    integer, intent ( in) :: status
    
    if(status /= nf90_noerr) then 
      print *, trim(nf90_strerror(status))
      stop 2
    end if
  end subroutine check  
end program pres_temp_4D_rd

