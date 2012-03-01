! This is part of Unidata's netCDF package. Copyright 2009.
! This is a test program for the nc-config utility. 

program test_prog
  use typeSizes
  use netcdf
  implicit none

  print *, 'NetCDF version: ', nf90_inq_libvers()
  print *, '*** SUCCESS!'        
end program test_prog

