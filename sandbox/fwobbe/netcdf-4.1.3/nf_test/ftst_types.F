C     This is part of the netCDF package.
C     Copyright 2007 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This program tests netCDF-4 user defined types from fortran.

C     $Id$

      program ftst_types
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will create.
      character*(*) FILE_NAME
      parameter (FILE_NAME='ftst_types.nc')

C     We are writing 2D data, a 6 x 12 grid. 
      integer NDIMS
      parameter (NDIMS=2)
      integer NX, NY
      parameter (NX = 6, NY = 12)

C     NetCDF IDs.
      integer ncid, varid, dimids(NDIMS)
      integer wind_typeid
      integer x_dimid, y_dimid
      integer typeids(1)

C     Info about the type we'll create.
      integer size_in, base_type_in, nfields_in, class_in
      character*80 name_in
      character*(*) type_name, u_name, v_name
      parameter (type_name = 'wind_t', u_name = 'U', v_name = 'V')
      integer ntypes
      integer WIND_T_SIZE
      parameter (WIND_T_SIZE = 8)
      integer offset_in, field_typeid_in, ndims_in, dim_sizes_in(1)

C     This is the data array we will write, and a place to store it when
C     we read it back in.
      integer data_out(NY, NX), data_in(NY, NX)

C     Loop indexes, and error handling.
      integer x, y, retval

C     Create some pretend data.
      do x = 1, NX
         do y = 1, NY
            data_out(y, x) = (x - 1) * NY + (y - 1)
         end do
      end do

      print *, ''
      print *,'*** Testing netCDF-4 compound type from F77.'

C     Create the netCDF file.
      retval = nf_create(FILE_NAME, NF_NETCDF4, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define the dimensions.
      retval = nf_def_dim(ncid, "x", NX, x_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_dim(ncid, "y", NY, y_dimid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define a compound type.
      retval = nf_def_compound(ncid, WIND_T_SIZE, type_name, 
     &     wind_typeid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_insert_compound(ncid, wind_typeid, u_name, 0, NF_INT)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_insert_compound(ncid, wind_typeid, v_name, 4, NF_INT)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Check out my new type.
      retval = nf_inq_typeids(ncid, ntypes, typeids)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (ntypes .ne. 1) stop 2
      if (typeids(1) .ne. wind_typeid) stop 2

C     Define the variable. 
      dimids(2) = x_dimid
      dimids(1) = y_dimid
      retval = nf_def_var(ncid, "data", NF_INT, NDIMS, dimids, varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Write the pretend data to the file.
      retval = nf_put_var_int(ncid, varid, data_out)
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
      if (ntypes .ne. 1 .or. typeids(1) .ne. wind_typeid) stop 2
      
C     Check the type.
      retval = nf_inq_user_type(ncid, typeids(1), name_in, size_in, 
     &     base_type_in, nfields_in, class_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:len(type_name)) .ne. type_name .or. 
     &     size_in .ne. WIND_T_SIZE .or. nfields_in .ne. 2 .or.
     &     class_in .ne. NF_COMPOUND) stop 2

C     Check it differently.
      retval = nf_inq_compound(ncid, typeids(1), name_in, size_in, 
     &     nfields_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:len(type_name)) .ne. type_name .or. 
     &     size_in .ne. WIND_T_SIZE .or. nfields_in .ne. 2) stop 2

C     Check it one piece at a time.
      retval = nf_inq_compound_nfields(ncid, typeids(1), nfields_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (nfields_in .ne. 2) stop 2
      retval = nf_inq_compound_size(ncid, typeids(1), size_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (size_in .ne. WIND_T_SIZE) stop 2
      retval = nf_inq_compound_name(ncid, typeids(1), name_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:len(type_name)) .ne. type_name) stop 2

C     Check the first field of the compound type.
      retval = nf_inq_compound_field(ncid, typeids(1), 1, name_in, 
     &     offset_in, field_typeid_in, ndims_in, dim_sizes_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:len(u_name)) .ne. u_name .or. offset_in .ne. 0 .or.
     &     field_typeid_in .ne. NF_INT .or. ndims_in .ne. 0) stop 2
      retval = nf_inq_compound_fieldname(ncid, typeids(1), 1, name_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:len(u_name)) .ne. u_name) stop 2
      retval = nf_inq_compound_fieldoffset(ncid, typeids(1), 1, 
     &     offset_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (offset_in .ne. 0) stop 2
      retval = nf_inq_compound_fieldtype(ncid, typeids(1), 1, 
     &     field_typeid_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (field_typeid_in .ne. NF_INT) stop 2
      retval = nf_inq_compound_fieldndims(ncid, typeids(1), 1, 
     &     ndims_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (ndims_in .ne. 0) stop 2
      
C     Check the second field of the compound type.
      retval = nf_inq_compound_field(ncid, typeids(1), 2, name_in, 
     &     offset_in, field_typeid_in, ndims_in, dim_sizes_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:len(v_name)) .ne. v_name .or. offset_in .ne. 4 .or.
     &     field_typeid_in .ne. NF_INT .or. ndims_in .ne. 0) stop 2
      
C     Find our variable.
      retval = nf_inq_varid(ncid, "data", varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (varid .ne. 1) stop 2

C     Read the data and check it.
      retval = nf_get_var_int(ncid, varid, data_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      do x = 1, NX
         do y = 1, NY
            if (data_in(y, x) .ne. data_out(y, x)) stop 2
         end do
      end do

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      print *,'*** SUCCESS!'
      end
