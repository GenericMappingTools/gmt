C     This is part of the netCDF package.
C     Copyright 2008 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This program tests netCDF-4 variable functions from fortran, even
C     more, even more.

C     $Id$

      program ftst_vars3
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will create.
      character*(*) FILE_NAME
      parameter (FILE_NAME='ftst_vars3.nc')

C     We are writing an attribute, of length 3.
      integer NDIMS
      parameter (NDIMS = 1)
      integer NX
      parameter (NX = 3)

C     NetCDF IDs.
      integer ncid, varid, dimids(1)
      integer enum_typeid, opaque_typeid

C     This is the data array we will write as an enum attribute, and a
C     place to store it when we read it back in. Z is the fastest
C     varying dimension.
      integer data_out(NX), data_in(NX)

      integer max_types
      parameter (max_types = 2)

C     Need these to read type information.
      integer num_types, typeids(max_types)
      integer base_type, base_size, num_members, member_value
      character*80 type_name, member_name
      integer type_size, nfields, class

C     Information for the enum type we will define.
      character*(*) enum_type_name, one_name, zero_name
      parameter (enum_type_name = 'enum_type')
      parameter (zero_name = 'zero', one_name = 'one')
      integer one, zero

C     Information for the opaque type we will define.
      character*(*) opaque_type_name
      parameter (opaque_type_name = 'opaque_type')
      integer opaque_size
      parameter (opaque_size = 16)
      character*(opaque_size) opaque_data, opaque_data_in
      parameter (opaque_data = '0123456789012345')

C     Loop indexes, and error handling.
      integer x, retval, index(1)

C     Create some pretend data.
      do x = 1, NX
         data_out(x) = 0
      end do
      data_out(1) = 1

      print *, ''
      print *,'*** Testing enum and opaque types.'

C     Create the netCDF file.
      retval = nf_create(FILE_NAME, NF_NETCDF4, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Create the enum type.
      retval = nf_def_enum(ncid, NF_INT, enum_type_name, enum_typeid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      one = 1
      zero = 0
      retval = nf_insert_enum(ncid, enum_typeid, zero_name, zero)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_insert_enum(ncid, enum_typeid, one_name, one)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Create the opaque type.
      retval = nf_def_opaque(ncid, opaque_size, opaque_type_name, 
     &     opaque_typeid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Create a dimension.
      retval = nf_def_dim(ncid, 'dim', 1, dimids(1))
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Create an opaque variable.
      retval = nf_def_var(ncid, 'var', opaque_typeid, 1, dimids, varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Write the opaque scalar var. (Could also use nf_put_var).
      index(1) = 1
      retval = nf_put_var1(ncid, varid, index, opaque_data)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Attach an enum attribute to the variable.
      retval = nf_put_att(ncid, varid, 'att', enum_typeid, NX, 
     &     data_out)
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

C     Use nf_inq_user_type to confirm this is an enum type.
      retval = nf_inq_user_type(ncid, typeids(1), type_name, type_size, 
     &     base_type, nfields, class)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (type_name(1:len(enum_type_name)) .ne. enum_type_name .or.
     &     type_size .ne. 4 .or. base_type .ne. NF_INT .or. 
     &     nfields .ne. 2 .or. class .ne. nf_enum) stop 2

C     Use nf_inq_enum and make sure we get the same answers as we did
C     with nf_inq_user_type.
      retval = nf_inq_enum(ncid, typeids(1), type_name, base_type, 
     &     base_size, num_members)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (type_name(1:len(enum_type_name)) .ne. enum_type_name .or.
     &     base_type .ne. NF_INT .or. num_members .ne. 2) stop 2

C     Check the members of the enum type.
      retval = nf_inq_enum_member(ncid, typeids(1), 1, member_name, 
     &     member_value)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (member_name(1:len(zero_name)) .ne. zero_name .or.
     &     member_value .ne. 0) stop 2
      retval = nf_inq_enum_member(ncid, typeids(1), 2, member_name, 
     &     member_value)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (member_name(1:len(one_name)) .ne. one_name .or.
     &     member_value .ne. 1) stop 2
      retval = nf_inq_enum_ident(ncid, typeids(1), 0, member_name)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (member_name(1:len(zero_name)) .ne. zero_name) stop 2
      retval = nf_inq_enum_ident(ncid, typeids(1), 1, member_name)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (member_name(1:len(one_name)) .ne. one_name) stop 2

C     Use nf_inq_user_type to confirm that the second typeid is an
C     opaque type.
      retval = nf_inq_user_type(ncid, typeids(2), type_name, type_size, 
     &     base_type, nfields, class)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (type_name(1:len(opaque_type_name)) .ne. opaque_type_name .or.
     &     type_size .ne. opaque_size .or. base_type .ne. 0 .or. 
     &     nfields .ne. 0 .or. class .ne. nf_opaque) stop 2

C     Use nf_inq_opaque and make sure we get the same answers as we did
C     with nf_inq_user_type.
      retval = nf_inq_opaque(ncid, typeids(2), type_name, base_size)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (base_size .ne. opaque_size .or. 
     &     type_name(1:len(opaque_type_name)) .ne. opaque_type_name) 
     &     stop 2

C     Read the variable.
      index(1) = 1
      retval = nf_get_var1(ncid, varid, index, opaque_data_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (opaque_data_in .ne. opaque_data) stop 2

C     Read the attribute.
      retval = nf_get_att(ncid, varid, 'att', data_in)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Check the values.
      do x = 1, NX
         if (data_in(x) .ne. data_out(x)) stop 2
      end do

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      print *,'*** SUCCESS!'
      end
