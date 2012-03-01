! Description:
!  
! Input Parameters:
!   None.
!
! Output Parameters:
!   Many public constants are defined in "netcdf_constants.f90". The names follow 
!     the Fortran 77 names, with nf90_ used as a prefix instead of nf_77. 
!   Functions are made accessable through PUBLIC:: statements in "netcdf_visibility.f90". 
!     Only the functions listed in this file are available in the interface. 
!
! References and Credits:
!   Written by
!    Robert Pincus
!    Cooperative Institue for Meteorological Satellite Studies
!    University of Wisconsin - Madison
!    1225 W. Dayton St. 
!    Madison, Wisconsin 53706
!    Robert.Pincus@ssec.wisc.edu
!
! Design Notes:
!   Module elements are private by default. 
!   Many functions have optional arguments. In order to keep the interface easy to use, 
!     we've reordered the arguments (relative to the F77 interface) in some functions. 
!   The external datatype of attributes is the same as the internal type. 
!   By default, data is read from or put into the lowest part of the netCDF array with stride 1. 
!   We've made heavy use of overloading, especially in the variable put and get routines. 
!     A single function exists for putting all variables; a single function exists for getting 
!     variables. 
!   Text variables must be treated somewhat differently. When a character variable is defined, the
!     fastest-varying index (first index in Fortran) must be the maxiumu length of the character 
!     string. N dimensional arrays of strings passed to the put or get functions are written/read
!     from dimensions 2:N+1. The number of values along the first dimension is determined by the
!     length of the argument character string. 
!
 module netcdf
  use typeSizes, only: OneByteInt, TwoByteInt, FourByteInt, EightByteInt, &
                       FourByteReal, EightByteReal
  implicit none
  private
  
  include "netcdf_constants.f90"
  include "netcdf_externals.f90"
  include "netcdf_overloads.f90"
  include "netcdf_visibility.f90"
contains
  include "netcdf_file.f90"
  include "netcdf3_file.f90"
  include "netcdf_dims.f90"
  include "netcdf_attributes.f90"
  include "netcdf_variables.f90"
  include "netcdf_text_variables.f90"
  include "netcdf_expanded.f90"
  include "netcdf_eightbyte.f90"
end module netcdf
