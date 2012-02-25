C     This is part of the netCDF package.
C     Copyright 2008 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This program tests netCDF-4 variable functions from fortran, even
C     more, even more.

C     $Id$

      program ftst_vars5
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will create.
      character*(*) FILE_NAME
      parameter (FILE_NAME='ftst_vars5.nc')

C     NetCDF IDs.
      integer ncid, cmp_typeid

      integer max_types
      parameter (max_types = 1)

C     Need these to read type information.
      integer num_types, typeids(max_types)
      integer base_type, type_size, num_members, member_value
      character*80 type_name, member_name
      integer nfields, class

C     Information for the compound type we will define.
      character*(*) cmp_type_name, int1_name, int2_name
      parameter (cmp_type_name = 'compound_type')
      parameter (int1_name = 'int1', int2_name = 'int2')
      integer compound_len, compound_len_in
      parameter (compound_len = 2)
      integer data1(compound_len), data1_in(compound_len)
      character*(4) att_name
      parameter (att_name = 'att1')

C     Loop indexes, and error handling.
      integer x, retval, index(1)

      print *, ''
      print *,'*** Testing compound types.'

C     Prepare some data to write.
      do x = 1, compound_len
         data1(x) = x
      end do

C     Create the netCDF file.
      retval = nf_create(FILE_NAME, NF_NETCDF4, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Create the compound type.
      retval = nf_def_compound(ncid, 8, cmp_type_name, 
     &     cmp_typeid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Insert two integers.
      retval = nf_insert_compound(ncid, cmp_typeid, int1_name, 
     &     0, NF_INT)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_insert_compound(ncid, cmp_typeid, int2_name, 
     &     4, NF_INT)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Write the compound attribute.
      retval = nf_put_att(ncid, NF_GLOBAL, att_name, cmp_typeid, 
     &     1, data1)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Reopen the file.
      retval = nf_open(FILE_NAME, NF_NOWRITE, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Get the typeids of all user defined types.
      retval = nf_inq_typeids(ncid, num_types, typeids)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (num_types .ne. max_types) stop 2

C     Use nf_inq_user_type to confirm this is an compound type
      retval = nf_inq_user_type(ncid, typeids(1), type_name, type_size, 
     &     base_type, nfields, class)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (type_name(1:len(cmp_type_name)) .ne. cmp_type_name .or.
     &     type_size .ne. 8 .or. base_type .ne. 0 .or.
     &     nfields .ne. 2 .or. class .ne. nf_compound) stop 2

C     Use nf_inq_compound and make sure we get the same answers as we did
C     with nf_inq_user_type.
      retval = nf_inq_compound(ncid, typeids(1), type_name, type_size, 
     &     nfields)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (type_name(1:len(cmp_type_name)) .ne. cmp_type_name .or.
     &     base_type .ne. 0 .or. type_size .ne. 8 .or.
     &     nfields .ne. 2) stop 2

C     Read the compound attribute.
      retval = nf_get_att(ncid, NF_GLOBAL, att_name, data1_in)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Check the data
      do x = 1, compound_len
         if (data1(x) .ne. data1_in(x)) stop 2
      end do

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      print *,'*** SUCCESS!'
      end
