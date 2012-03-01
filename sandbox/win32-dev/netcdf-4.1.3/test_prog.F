C This is part of Unidata's netCDF package. Copyright 2009.
C This is a test program for the nc-config utility. 

      program test_prog
      implicit none
#include "netcdf.inc"
      print *, 'NetCDF version: ', nf_inq_libvers()
      print *, '*** SUCCESS!'        
      end

