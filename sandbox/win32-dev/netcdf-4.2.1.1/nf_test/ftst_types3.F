C     This is part of the netCDF package.
C     Copyright 2007 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This program tests netCDF-4 user defined types from fortran.

C     $Id$

      program ftst_types3
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will create.
      character*(*) FILE_NAME
      parameter (FILE_NAME='ftst_types3.nc')

C     We are writing 2D data, a 3 x 2 grid. 
      integer NDIMS
      parameter (NDIMS = 2)
      integer dim_sizes(NDIMS)
      integer NX, NY
      parameter (NX = 3, NY = 2)

C     NetCDF IDs.
      integer ncid, varid, dimids(NDIMS)
      integer cmp_typeid, typeid_in
      integer x_dimid, y_dimid
      integer typeids(1)
      integer grpid, sub_grpid

C     Info about the groups we'll create.
      character*(*) group_name, sub_group_name
      parameter (group_name = 'you_drive_me_crazy')
      parameter (sub_group_name = 'baby_Im_so_into_you')

C     Info about the type we'll create.
      integer size_in, base_type_in, nfields_in, class_in
      character*80 name_in
      character*(*) type_name, field_name
      parameter (type_name = 'I_just_want_to_have_some_fun')
      parameter (field_name = 'Ill_tell_it_to_the_world')
      integer ntypes
      integer cmp_size
      parameter (cmp_size = 4)
      integer offset_in, field_typeid_in, ndims_in, dim_sizes_in(NDIMS)

C     Loop indexes, and error handling.
      integer x, y, retval

      print *, ''
      print *,'*** Testing netCDF-4 user-defined types and groups.'

C     Create the netCDF file.
      retval = nf_create(FILE_NAME, NF_NETCDF4, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Create a group and a subgroup.
      retval = nf_def_grp(ncid, group_name, grpid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_grp(grpid, sub_group_name, sub_grpid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define a compound type in the root group.
      retval = nf_def_compound(ncid, cmp_size, type_name, 
     &     cmp_typeid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Include a float.
      retval = nf_insert_compound(ncid, cmp_typeid, field_name, 0, 
     &     NF_FLOAT)
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
     &     class_in .ne. NF_COMPOUND) stop 31

C     Check the first field of the compound type.
      retval = nf_inq_compound_field(ncid, typeids(1), 1, name_in, 
     &     offset_in, field_typeid_in, ndims_in, dim_sizes_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:len(field_name)) .ne. field_name .or. 
     &     offset_in .ne. 0 .or. field_typeid_in .ne. NF_FLOAT .or. 
     &     ndims_in .ne. 0) stop 19

C     Go to a child group and find the id of our type.
      retval = nf_inq_grp_ncid(ncid, group_name, sub_grpid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_typeid(sub_grpid, type_name, typeid_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_user_type(sub_grpid, typeid_in, name_in, size_in, 
     &     base_type_in, nfields_in, class_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:len(type_name)) .ne. type_name .or. 
     &     size_in .ne. cmp_size .or. nfields_in .ne. 1 .or. 
     &     class_in .ne. NF_COMPOUND) stop 22
      


C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      print *,'*** SUCCESS!'
      end
