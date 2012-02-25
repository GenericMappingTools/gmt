C     This is part of the netCDF package.
C     Copyright 2008 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This program tests netCDF-4 variable functions from fortran, even
C     more.

C     $Id$

      program ftst_vars2
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will create.
      character*(*) FILE_NAME
      parameter (FILE_NAME='ftst_vars2.nc')

C     We are writing 3D data, a 3 x 5 x 2 grid. Why do I use "x, y, z,"
C     and then do everything in order "z, y, x?" Because I am a C
C     programmer, and everything in Fortran seems backwards!
      integer NDIMS, NTYPES
      parameter (NDIMS = 3, NTYPES = 5)
      integer NX, NY, NZ
      parameter (NX = 3, NY = 5, NZ = 2)

C     These will be used to set the per-variable chunk cache.
      integer CACHE_SIZE, CACHE_NELEMS, CACHE_PREEMPTION
      parameter (CACHE_SIZE = 8, CACHE_NELEMS = 571)
      parameter (CACHE_PREEMPTION = 42)

C     These will be used to check the setting of the per-variable chunk
C     cache.
      integer cache_size_in, cache_nelems_in, cache_preemption_in

C     NetCDF IDs.
      integer ncid, varid(NTYPES), dimids(NDIMS), typeid(NTYPES)

C     Name of the variable is stored here.
      character*80 var_name

C     This is the data array we will write, and a place to store it when
C     we read it back in. Z is the fastest varying dimension.
      integer data_out(NZ, NY, NX), data_in(NZ, NY, NX)

C     Loop indexes, and error handling.
      integer i, x, y, z, retval

C     Create some pretend data.
      do x = 1, NX
         do y = 1, NY
            do z = 1, NZ
               data_out(z, y, x) = (x-1) * NY * NZ + (y-1) * NZ + (z-1)
            end do
         end do
      end do

      print *, ''
      print *,'*** Testing netCDF-4 vars from F77 with new types.'

C     Create the netCDF file.
      retval = nf_create(FILE_NAME, NF_NETCDF4, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Define the dimensions.
      retval = nf_def_dim(ncid, "z", NZ, dimids(3))
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_dim(ncid, "y", NY, dimids(2))
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_dim(ncid, "x", NX, dimids(1))
      if (retval .ne. nf_noerr) call handle_err(retval)

C     These are the types of vars that will be written.
      typeid(1) = NF_UBYTE
      typeid(2) = NF_USHORT
      typeid(3) = NF_UINT
      typeid(4) = NF_INT64
      typeid(5) = NF_UINT64

C     Define the variables. 
      do i = 1, NTYPES
         write(var_name, 1001) i
 1001    format('var', I1)
         retval = nf_def_var(ncid, var_name, typeid(i), NDIMS, 
     &        dimids, varid(i))
         if (retval .ne. nf_noerr) call handle_err(retval)

C        Set variable caches.
         retval = nf_set_var_chunk_cache(ncid, varid(i), CACHE_SIZE, 
     &        CACHE_NELEMS, CACHE_PREEMPTION)
         if (retval .ne. nf_noerr) call handle_err(retval)
      end do

C     Check the per-variable cache to make sure it is what we think it
C     is.
      do i = 1, NTYPES
         retval = nf_get_var_chunk_cache(ncid, varid(i), cache_size_in, 
     &        cache_nelems_in, cache_preemption_in)
         if (retval .ne. nf_noerr) call handle_err(retval)
         if (cache_size_in .ne. CACHE_SIZE .or. cache_nelems_in .ne. 
     &        CACHE_NELEMS .or. cache_preemption .ne. CACHE_PREEMPTION)
     &        stop 8
         
      end do

C     Write the pretend data to the file.
      do i = 1, NTYPES
         retval = nf_put_var_int(ncid, varid(i), data_out)
         if (retval .ne. nf_noerr) call handle_err(retval)
      end do

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Reopen the file and check again.
      retval = nf_open(FILE_NAME, NF_NOWRITE, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Read the data and check it.
      do i = 1, NTYPES
         retval = nf_get_var_int(ncid, varid(i), data_in)
         if (retval .ne. nf_noerr) call handle_err(retval)
         do x = 1, NX
            do y = 1, NY
               do z = 1, NZ
                  if (data_in(z, y, x) .ne. data_out(z, y, x)) stop 2
               end do
            end do
         end do
      end do

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      print *,'*** SUCCESS!'
      end
