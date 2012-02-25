! This is part of the netCDF package.
! Copyright 2006 University Corporation for Atmospheric Research/Unidata.
! See COPYRIGHT file for conditions of use.

! This example writes some surface pressure and temperatures. It is
! intended to illustrate the use of the netCDF fortran 90 API. The
! companion program nc4_sfc_pres_temp_rd.f90 shows how to read the netCDF
! data file created by this program.

! This program is part of the netCDF tutorial:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

! Full documentation of the netCDF Fortran 90 API can be found at:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f90

! $Id$

program nc4_sfc_pres_temp_wr
  use netcdf
  implicit none

  ! This is the name of the data file we will create.
  character (len = *), parameter :: FILE_NAME = "nc4_sfc_pres_temp.nc"
  integer :: ncid

  ! We are writing 2D data, a 12 x 6 lon-lat grid. We will need two
  ! netCDF dimensions.
  integer, parameter :: NDIMS = 2
  integer, parameter :: NLATS = 6, NLONS = 12
  character (len = *), parameter :: LAT_NAME = "latitude"
  character (len = *), parameter :: LON_NAME = "longitude"
  integer :: lat_dimid, lon_dimid

  ! In addition to the latitude and longitude dimensions, we will also
  ! create latitude and longitude netCDF variables which will hold the
  ! actual latitudes and longitudes. Since they hold data about the
  ! coordinate system, the netCDF term for these is: "coordinate
  ! variables."
  real :: lats(NLATS), lons(NLONS)
  integer :: lat_varid, lon_varid
  real, parameter :: START_LAT = 25.0, START_LON = -125.0

  ! We will write surface temperature and pressure fields. 
  character (len = *), parameter :: PRES_NAME="pressure"
  character (len = *), parameter :: TEMP_NAME="temperature"
  integer :: pres_varid, temp_varid
  integer :: dimids(NDIMS)

  ! It's good practice for each variable to carry a "units" attribute.
  character (len = *), parameter :: UNITS = "units"
  character (len = *), parameter :: PRES_UNITS = "hPa"
  character (len = *), parameter :: TEMP_UNITS = "celsius"
  character (len = *), parameter :: LAT_UNITS = "degrees_north"
  character (len = *), parameter :: LON_UNITS = "degrees_east"

  ! We will create some pressure and temperature data to write out.
  real, dimension(:,:), allocatable :: temp_out
  real, dimension(:,:), allocatable :: pres_out
  real, parameter :: SAMPLE_PRESSURE = 900.0
  real, parameter :: SAMPLE_TEMP = 9.0

  ! Loop indices
  integer :: lat, lon

  ! Allocate memory.
  allocate(pres_out(NLONS, NLATS))
  allocate(temp_out(NLONS, NLATS))

  ! Create pretend data. If this were not an example program, we would
  ! have some real data to write, for example, model output.
  do lat = 1, NLATS
     lats(lat) = START_LAT + (lat - 1) * 5.0
  end do
  do lon = 1, NLONS
     lons(lon) = START_LON + (lon - 1) * 5.0
  end do
  do lon = 1, NLONS
     do lat = 1, NLATS
        pres_out(lon, lat) = SAMPLE_PRESSURE + (lon - 1) * NLATS + (lat - 1)
        temp_out(lon, lat) = SAMPLE_TEMP + .25 * ((lon - 1) * NLATS + (lat - 1))
     end do
  end do

  ! Create the file. 
  call check( nf90_create(FILE_NAME, nf90_hdf5, ncid) )

  ! Define the dimensions.
  call check( nf90_def_dim(ncid, LAT_NAME, NLATS, lat_dimid) )
  call check( nf90_def_dim(ncid, LON_NAME, NLONS, lon_dimid) )

  ! Define the coordinate variables. They will hold the coordinate
  ! information, that is, the latitudes and longitudes. A varid is
  ! returned for each.
  call check( nf90_def_var(ncid, LAT_NAME, NF90_REAL, lat_dimid, lat_varid) )
  call check( nf90_def_var(ncid, LON_NAME, NF90_REAL, lon_dimid, lon_varid) )

  ! Assign units attributes to coordinate var data. This attaches a
  ! text attribute to each of the coordinate variables, containing the
  ! units.
  call check( nf90_put_att(ncid, lat_varid, UNITS, LAT_UNITS) )
  call check( nf90_put_att(ncid, lon_varid, UNITS, LON_UNITS) )

  ! Define the netCDF variables. The dimids array is used to pass the
  ! dimids of the dimensions of the netCDF variables.
  dimids = (/ lon_dimid, lat_dimid /)
  call check( nf90_def_var(ncid, PRES_NAME, NF90_REAL, dimids, pres_varid) )
  call check( nf90_def_var(ncid, TEMP_NAME, NF90_REAL, dimids, temp_varid) )

  ! Assign units attributes to the pressure and temperature netCDF
  ! variables.
  call check( nf90_put_att(ncid, pres_varid, UNITS, PRES_UNITS) )
  call check( nf90_put_att(ncid, temp_varid, UNITS, TEMP_UNITS) )

  ! End define mode.
  call check( nf90_enddef(ncid) )

  ! Write the coordinate variable data. This will put the latitudes
  ! and longitudes of our data grid into the netCDF file.
  call check( nf90_put_var(ncid, lat_varid, lats) )
  call check( nf90_put_var(ncid, lon_varid, lons) )

  ! Write the pretend data. This will write our surface pressure and
  ! surface temperature data. The arrays of data are the same size as
  ! the netCDF variables we have defined.
  call check( nf90_put_var(ncid, pres_varid, pres_out) )
  call check( nf90_put_var(ncid, temp_varid, temp_out) )

  ! Close the file.
  call check( nf90_close(ncid) )
   
  ! If we got this far, everything worked as expected. Yipee! 
  print *,"*** SUCCESS writing example file nc4_sfc_pres_temp.nc!"

contains
  subroutine check(status)
    integer, intent ( in) :: status
    
    if(status /= nf90_noerr) then 
      print *, trim(nf90_strerror(status))
      stop 2
    end if
  end subroutine check  
end program nc4_sfc_pres_temp_wr

