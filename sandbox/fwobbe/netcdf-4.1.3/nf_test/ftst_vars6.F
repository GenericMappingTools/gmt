C     This is part of the netCDF package.
C     Copyright 2008 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This program tests netCDF-4 variable functions from fortran, even
C     more, even more.

C     $Id$

      program ftst_vars6
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will create.
      character*(*) FILE_NAME
      parameter (FILE_NAME='ftst_vars6.nc')
      integer NDIMS
      parameter (NDIMS = 1)
      integer DIM_LEN
      parameter (DIM_LEN = 22)
      integer NVARS
      parameter (NVARS = 3)
      integer DATA_LEN
      parameter (DATA_LEN = 2)
      integer check_file

      integer ncid, varid(NVARS), dimids(NDIMS)
      integer data_len_in, offset
      parameter (offset = 20)
      integer data1(data_len), data1_in(data_len)
      character*(4) var_name(NVARS)
      character*(4) dim_name
      parameter (dim_name = 'dim1')
      integer NO_FILL, MY_FILL_VALUE
      parameter (NO_FILL = 1)
      parameter (MY_FILL_VALUE = 42)

C     Loop index and error handling.
      integer x, retval

      print *, ''
      print *,'*** Testing fill values.'

C     Prepare some data to write.
      do x = 1, data_len
         data1(x) = x
      end do

C     Set up var names.
      var_name(1) = 'var1'
      var_name(2) = 'var2'
      var_name(3) = 'var3'

C     Create the netCDF file.
      retval = nf_create(FILE_NAME, NF_NETCDF4, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Create a dimension.
      retval = nf_def_dim(ncid, dim_name, DIM_LEN, dimids(1))
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Create a few integer variables.
      do x = 1, NVARS      
         retval = nf_def_var(ncid, var_name(x), NF_INT, NDIMS, dimids,
     $        varid(x))
         if (retval .ne. nf_noerr) call handle_err(retval)
      end do

C     Set no fill mode for the second variable.
      retval = nf_def_var_fill(ncid, varid(2), NO_FILL, 88)
      if (retval .ne. 0) stop 2

C     Set an alternative fill value for the third variable.
      retval = nf_def_var_fill(ncid, varid(3), 0, MY_FILL_VALUE)
      if (retval .ne. 0) stop 3

C     Check it out. 
c      retval = check_file(ncid, var_name, dim_name)
c      if (retval .ne. 0) stop 2

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Reopen the file.
      retval = nf_open(FILE_NAME, NF_NOWRITE, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Check it out. 
      retval = check_file(ncid, var_name, dim_name)
      if (retval .ne. 0) stop 4

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      print *,'*** SUCCESS!'
      end

C     This function check the file to make sure everything is OK.
      integer function check_file(ncid, var_name, dim_name)
      implicit none
      include 'netcdf.inc'

C     I need these in both here and the main program.
      integer NDIMS
      parameter (NDIMS = 1)
      integer DIM_LEN
      parameter (DIM_LEN = 22)
      integer NVARS
      parameter (NVARS = 3)
      integer DATA_LEN
      parameter (DATA_LEN = 2)
      integer MY_FILL_VALUE
      parameter (MY_FILL_VALUE = 42)

C     Parameters
      integer ncid
      character*(4) var_name(NVARS)
      character*(4) dim_name

C     Values that are read in, to check the file.
      integer ndims_in, nvars_in, ngatts_in, unlimdimid_in
      integer xtype_in, dimids_in(NDIMS), natts_in
      integer varid_in(NVARS), dimid_in, no_fill_in, fill_value_in
      character*(4) var_name_in
      integer int_data_in(DIM_LEN)

      integer x, retval

C     Check it out.
      retval = nf_inq(ncid, ndims_in, nvars_in, ngatts_in,
     $     unlimdimid_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (ndims_in .ne. 1 .or. nvars_in .ne. NVARS .or. ngatts_in .ne. 0
     $     .or. unlimdimid_in .ne. -1) stop 5

C     Get the varids and the dimid.
      do x = 1, NVARS      
         retval = nf_inq_varid(ncid, var_name(x), varid_in(x))
         if (retval .ne. nf_noerr) call handle_err(retval)
         if (varid_in(x) .ne. x) stop 6
      end do
      retval = nf_inq_dimid(ncid, dim_name, dimid_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (dimid_in .ne. 1) stop 7

C     These things are the same for all three variables, except
C     natts_in..
      do x = 1, NVARS      
         retval = nf_inq_var(ncid, varid_in(x), var_name_in, xtype_in,
     $        ndims_in, dimids_in, natts_in)
         if (retval .ne. nf_noerr) call handle_err(retval)
         if (ndims_in .ne. 1 .or. xtype_in .ne. NF_INT .or. dimids_in(1)
     $        .ne. dimid_in) stop 8
         if (x .eq. 3 .and. natts_in .ne. 1) stop 9
         if (x .lt. 3 .and. natts_in .ne. 0) stop 10
      end do

C     Check the fill value for the first var. Nothing was set, so
C     no_fill should be off, and fill_value should be the default fill
C     value for this type.
      retval = nf_inq_var_fill(ncid, varid_in(1), no_fill_in,
     $     fill_value_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (no_fill_in .ne. 0 .or. fill_value_in .ne. nf_fill_int) stop 11

C     Check that no_fill mode is on for the second variable.
      retval = nf_inq_var_fill(ncid, varid_in(2), no_fill_in,
     $     fill_value_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (no_fill_in .ne. 1 .or. fill_value_in .ne. nf_fill_int) stop 12

C     Check that a non-default fill value is in use for the third variable.
      retval = nf_inq_var_fill(ncid, varid_in(3), no_fill_in,
     $     fill_value_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (no_fill_in .ne. 0 .or. fill_value_in .ne. MY_FILL_VALUE) stop
     $     2

C     Get the data in var1. It will be all the default fill value.
      retval = nf_get_var_int(ncid, varid_in(1), int_data_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      do x = 1, DIM_LEN
         if (int_data_in(x) .ne. nf_fill_int) stop 13
      end do

C     Get the data in var2. What will it be?
      retval = nf_get_var_int(ncid, varid_in(2), int_data_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
C$$$      do x = 1, DIM_LEN
C$$$         print *, int_data_in(x)
C$$$      end do

C     Get the data in var3. It will be all the default fill value.
      retval = nf_get_var_int(ncid, varid_in(3), int_data_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      do x = 1, DIM_LEN
C         print *, int_data_in(x)
         if (int_data_in(x) .ne. MY_FILL_VALUE) stop 14
      end do

      check_file = 0
      end 
