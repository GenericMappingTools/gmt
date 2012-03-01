! Copyright 2007, UCAR/Unidata. See netcdf/COPYRIGHT file for copying
! and redistribution conditions.

! This program tests large files (> 4 GB) in netCDF-4. 

! $Id$
program tst_flarge
  use typeSizes
  use netcdf
  implicit none

  integer :: ncFileID, dimID, varID1, varID2 
  integer, parameter :: BIG_DIMENSION = 300000000
  character (len = *), parameter :: fileName = "tst_flarge.nc"
  character (len = *), parameter :: dimName = "really_big_dimension"
  character (len = *), parameter :: var1Name = "TweedleDum"
  character (len = *), parameter :: var2Name = "TweedleDee"
  double precision, parameter :: VAL1 = 42.5
  double precision, parameter :: VAL2 = -42.5
  double precision :: val1_in
  double precision :: val2_in

  print *,'*** Testing netCDF-4 large files from Fortran 90 API.'

  ! Create the file with 2 NF_DOUBLE vars, each with one really long dimension.
  call check(nf90_create(trim(fileName), nf90_hdf5, ncFileID))
  call check(nf90_def_dim(ncFileID, dimName, BIG_DIMENSION, dimID))
  call check(nf90_def_var(ncFileID, var1Name, nf90_double, (/ dimID /), varID1) )
  call check(nf90_def_var(ncFileID, var2Name, nf90_double, (/ dimID /), varID2) )

!   ! Write a value in each variable.
  call check(nf90_put_var(ncFileID, VarID1, (/ 42.5 /), &
       start = (/ 1 /), count = (/ 1 /)) )
  call check(nf90_put_var(ncFileID, VarID2, (/ -42.5 /), &
       start = (/ BIG_DIMENSION /), count = (/ 1 /)) )

  call check(nf90_close(ncFileID))

  ! Now open the file to read and check a few values
  call check(nf90_open(trim(fileName), NF90_NOWRITE, ncFileID))
  call check(nf90_get_var(ncFileID, VarID1, val1_in, start = (/ 1 /)) )
  call check(nf90_get_var(ncFileID, VarID2, val2_in, start = (/ BIG_DIMENSION /)) )
  if(val1_in /= VAL1 .or. val2_in /= VAL2) then
     print *, 'Variable value not what was written'
     stop 2
  end if

  call check(nf90_close(ncFileID))
  OPEN (UNIT=5, FILE=fileName, STATUS="OLD")
  CLOSE (UNIT=5, STATUS="DELETE")
  print *,'*** SUCCESS!'

contains
  ! Internal subroutine - checks error status after each netcdf, prints out text message each time
  !   an error code is returned. 
  subroutine check(status)
    integer, intent ( in) :: status

    if(status /= nf90_noerr) then 
       print *, trim(nf90_strerror(status))
       stop 2
    end if
  end subroutine check
end program tst_flarge
