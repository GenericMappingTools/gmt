C     This is part of the netCDF package.
C     Copyright 2005 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This is the error handling function for some of the F77
C     tests. This error handler comes from the netcdf tutorial.

C     $Id$

C     This subroutine handles errors by printing an error message and
C     exiting with a non-zero status.
      subroutine handle_err(errcode)
      implicit none
      include 'netcdf.inc'
      integer errcode

      print *, 'Error: ', nf_strerror(errcode)
      stop 2
      end

