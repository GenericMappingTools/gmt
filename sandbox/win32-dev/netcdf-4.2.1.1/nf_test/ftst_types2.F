C     This is part of the netCDF package.
C     Copyright 2007 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This program tests netCDF-4 user defined types from fortran.

C     $Id$

      program ftst_types2
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will create.
      character*(*) FILE_NAME
      parameter (FILE_NAME='ftst_types2.nc')

C     We are writing 2D data, a 3 x 2 grid. 
      integer NDIMS
      parameter (NDIMS = 2)
      integer dim_sizes(NDIMS)
      integer NX, NY
      parameter (NX = 3, NY = 2)

C     NetCDF IDs.
      integer ncid, varid, dimids(NDIMS)
      integer cmp_typeid
      integer x_dimid, y_dimid
      integer typeids(1)

C     Info about the type we'll create.
      integer size_in, base_type_in, nfields_in, class_in
      character*80 name_in
      character*(*) type_name, ary_name
      parameter (type_name = 'cmp_w_ary', ary_name = 'A')
      integer ntypes
      integer cmp_size
      parameter (cmp_size = 24)
      integer offset_in, field_typeid_in, ndims_in, dim_sizes_in(NDIMS)

C     Loop indexes, and error handling.
      integer x, y, retval

      print *, ''
      print *,'*** Testing netCDF-4 compound types from F77 some more.'

C     Create the netCDF file.
      retval = nf_create(FILE_NAME, NF_NETCDF4, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define a compound type.
      retval = nf_def_compound(ncid, cmp_size, type_name, 
     &     cmp_typeid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Include an array.
      dim_sizes(1) = NX
      dim_sizes(2) = NY
      retval = nf_insert_array_compound(ncid, cmp_typeid, ary_name, 0, 
     &     NF_INT, NDIMS, dim_sizes)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Reopen the file and check again.
      retval = nf_open(FILE_NAME, NF_NOWRITE, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Find the type.
      retval = nf_inq_typeids(ncid, ntypes, typeids)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (ntypes .ne. 1 .or. typeids(1) .ne. cmp_typeid) stop 2
      
C     Check the type.
      retval = nf_inq_user_type(ncid, typeids(1), name_in, size_in, 
     &     base_type_in, nfields_in, class_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:len(type_name)) .ne. type_name .or. 
     &     size_in .ne. cmp_size .or. nfields_in .ne. 1 .or. 
     &     class_in .ne. NF_COMPOUND) stop 2

C     Check the first field of the compound type.
      retval = nf_inq_compound_field(ncid, typeids(1), 1, name_in, 
     &     offset_in, field_typeid_in, ndims_in, dim_sizes_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:len(ary_name)) .ne. ary_name .or. 
     &     offset_in .ne. 0 .or. field_typeid_in .ne. NF_INT .or. 
     &     ndims_in .ne. NDIMS .or. 
     &     dim_sizes_in(1) .ne. dim_sizes(1) .or. 
     &     dim_sizes_in(2) .ne. dim_sizes(2)) stop 2

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      print *,'*** SUCCESS!'
      end
