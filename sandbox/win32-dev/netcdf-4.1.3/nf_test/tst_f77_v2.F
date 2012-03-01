C     This is part of netCDF, Copyright 2006, UCAR

C     This test program uses the fortran 77 v2 API to create a simple
C     data file with some phoney data in it. This program is heavily
C     based on one contributed by Jeremy Kepner,
C     jvkepner@astro.Princeton.EDU.

C     This program will bail out in the event of a netcdf error.

C     $Id$

      PROGRAM tst_f77_v2
      
      IMPLICIT NONE

#include "tests.inc"

      INTEGER n_dim,x_dim,y_dim,z_dim
      PARAMETER(n_dim = 3, x_dim = 20, y_dim = 10, z_dim = 5)
      INTEGER dim_array(n_dim)
      INTEGER start(n_dim),count(n_dim)

      INTEGER ncid, errcode
      INTEGER x_id,y_id,z_id,arr_id
      REAL array(x_dim,y_dim,z_dim)
      INTEGER i,j,k

C     Put something into the array.
      DO i=1,x_dim
         DO j=1,y_dim
            DO k=1,z_dim
               array(i,j,k) = (i-1) + x_dim*(j-1) + x_dim*y_dim*(k-1)
            ENDDO
         ENDDO
      ENDDO

      print *, ''
      print *, ' *** Testing netCDF v2 api for F77.'

C     Create file.
      ncid = NCCRE('tst_f77_v2.nc', NCCLOB, errcode)

C     Create Dimensions.
      x_id = NCDDEF(ncid, 'X', x_dim, errcode)
      y_id = NCDDEF(ncid, 'Y', y_dim, errcode)
      z_id = NCDDEF(ncid, 'Z', z_dim, errcode)

C     Create a variable.
C     Assign dimensions to array.
      dim_array(1) = z_id
      dim_array(2) = y_id
      dim_array(3) = x_id
      arr_id = NCVDEF(ncid,'array',NCFLOAT,n_dim,dim_array,errcode)

C     Skip attributes.

C     Leave definitions.
      CALL NCENDF(ncid, errcode)

C     Write variable to file.
      start(1) = 1
      start(2) = 1
      start(3) = 1
      count(1) = z_dim
      count(2) = y_dim
      count(3) = x_dim
      CALL NCVPT(ncid,arr_id,start,count,array,errcode)

C     Close the file.
      CALL NCCLOS(ncid, errcode)

      print *, ' *** SUCCESS!'

      END
