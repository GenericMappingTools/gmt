! This is part of the netCDF package.
! Copyright 2006 University Corporation for Atmospheric Research/Unidata.
! See COPYRIGHT file for conditions of use.

! This is an example program which writes some 4D pressure and
! temperatures. It is intended to illustrate the use of the netCDF
! fortran 90 API. The companion program nc4_pres_temp_4D_rd.f shows how
! to read the netCDF data file created by this program.

! This program is part of the netCDF tutorial:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

! Full documentation of the netCDF Fortran 90 API can be found at:
! http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-f90

! $Id$

program nc4_pres_temp_4D_wr
  use netcdf
  implicit none

  ! This is the name of the data file we will create.
  character (len = *), parameter :: FILE_NAME = "nc4_pres_temp_4D.nc"
  integer :: ncid

  ! We are writing 4D data, a 12 x 6 x 2 lon-lat-lvl grid, with 2
  ! timesteps of data.
  integer, parameter :: NDIMS = 4, NRECS = 2
  integer, parameter :: NLVLS = 2, NLATS = 6, NLONS = 12
  character (len = *), parameter :: LVL_NAME = "level"
  character (len = *), parameter :: LAT_NAME = "latitude"
  character (len = *), parameter :: LON_NAME = "longitude"
  character (len = *), parameter :: REC_NAME = "time"
  integer :: lvl_dimid, lon_dimid, lat_dimid, rec_dimid

  ! The start and count arrays will tell the netCDF library where to
  ! write our data.
  integer :: start(NDIMS), count(NDIMS)

  ! These program variables hold the latitudes and longitudes.
  real :: lats(NLATS), lons(NLONS)
  integer :: lon_varid, lat_varid

  ! We will create two netCDF variables, one each for temperature and
  ! pressure fields.
  character (len = *), parameter :: PRES_NAME="pressure"
  character (len = *), parameter :: TEMP_NAME="temperature"
  integer :: pres_varid, temp_varid
  integer :: dimids(NDIMS)

  ! We recommend that each variable carry a "units" attribute.
  character (len = *), parameter :: UNITS = "units"
  character (len = *), parameter :: PRES_UNITS = "hPa"
  character (len = *), parameter :: TEMP_UNITS = "celsius"
  character (len = *), parameter :: LAT_UNITS = "degrees_north"
  character (len = *), parameter :: LON_UNITS = "degrees_east"

  ! Program variables to hold the data we will write out. We will only
  ! need enough space to hold one timestep of data; one record.
  real, dimension(:,:,:), allocatable :: pres_out
  real, dimension(:,:,:), allocatable :: temp_out
  real, parameter :: SAMPLE_PRESSURE = 900.0
  real, parameter :: SAMPLE_TEMP = 9.0

  ! Use these to construct some latitude and longitude data for this
  ! example.
  real, parameter :: START_LAT = 25.0, START_LON = -125.0

  ! Loop indices
  integer :: lvl, lat, lon, rec, i

  ! Allocate memory.
  allocate(pres_out(NLONS, NLATS, NLVLS))
  allocate(temp_out(NLONS, NLATS, NLVLS))

  ! Create pretend data. If this were not an example program, we would
  ! have some real data to write, for example, model output.
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

  ! Create the file. 
  call check( nf90_create(FILE_NAME, nf90_hdf5, ncid) )
  
  ! Define the dimensions. The record dimension is defined to have
  ! unlimited length - it can grow as needed. In this example it is
  ! the time dimension.
  call check( nf90_def_dim(ncid, LVL_NAME, NLVLS, lvl_dimid) )
  call check( nf90_def_dim(ncid, LAT_NAME, NLATS, lat_dimid) )
  call check( nf90_def_dim(ncid, LON_NAME, NLONS, lon_dimid) )
  call check( nf90_def_dim(ncid, REC_NAME, NF90_UNLIMITED, rec_dimid) )

  ! Define the coordinate variables. We will only define coordinate
  ! variables for lat and lon.  Ordinarily we would need to provide
  ! an array of dimension IDs for each variable's dimensions, but
  ! since coordinate variables only have one dimension, we can
  ! simply provide the address of that dimension ID (lat_dimid) and
  ! similarly for (lon_dimid).
  call check( nf90_def_var(ncid, LAT_NAME, NF90_REAL, lat_dimid, lat_varid) )
  call check( nf90_def_var(ncid, LON_NAME, NF90_REAL, lon_dimid, lon_varid) )

  ! Assign units attributes to coordinate variables.
  call check( nf90_put_att(ncid, lat_varid, UNITS, LAT_UNITS) )
  call check( nf90_put_att(ncid, lon_varid, UNITS, LON_UNITS) )

  ! The dimids array is used to pass the dimids of the dimensions of
  ! the netCDF variables. Both of the netCDF variables we are creating
  ! share the same four dimensions. In Fortran, the unlimited
  ! dimension must come last on the list of dimids.
  dimids = (/ lon_dimid, lat_dimid, lvl_dimid, rec_dimid /)

  ! Define the netCDF variables for the pressure and temperature data.
  call check( nf90_def_var(ncid, PRES_NAME, NF90_REAL, dimids, pres_varid) )
  call check( nf90_def_var(ncid, TEMP_NAME, NF90_REAL, dimids, temp_varid) )

  ! Assign units attributes to the netCDF variables.
  call check( nf90_put_att(ncid, pres_varid, UNITS, PRES_UNITS) )
  call check( nf90_put_att(ncid, temp_varid, UNITS, TEMP_UNITS) )
  
  ! End define mode.
  call check( nf90_enddef(ncid) )
  
  ! Write the coordinate variable data. This will put the latitudes
  ! and longitudes of our data grid into the netCDF file.
  call check( nf90_put_var(ncid, lat_varid, lats) )
  call check( nf90_put_var(ncid, lon_varid, lons) )
  
  ! These settings tell netcdf to write one timestep of data. (The
  ! setting of start(4) inside the loop below tells netCDF which
  ! timestep to write.)
  count = (/ NLONS, NLATS, NLVLS, 1 /)
  start = (/ 1, 1, 1, 1 /)

  ! Write the pretend data. This will write our surface pressure and
  ! surface temperature data. The arrays only hold one timestep worth
  ! of data. We will just rewrite the same data for each timestep. In
  ! a real :: application, the data would change between timesteps.
  do rec = 1, NRECS
     start(4) = rec
     call check( nf90_put_var(ncid, pres_varid, pres_out, start = start, &
                              count = count) )
     call check( nf90_put_var(ncid, temp_varid, temp_out, start = start, &
                              count = count) )
  end do
  
  ! Close the file. This causes netCDF to flush all buffers and make
  ! sure your data are really written to disk.
  call check( nf90_close(ncid) )
  
  print *,"*** SUCCESS writing example file ", FILE_NAME, "!"

contains
  subroutine check(status)
    integer, intent ( in) :: status
    
    if(status /= nf90_noerr) then 
      print *, trim(nf90_strerror(status))
      stop 2
    end if
  end subroutine check  
end program nc4_pres_temp_4D_wr

