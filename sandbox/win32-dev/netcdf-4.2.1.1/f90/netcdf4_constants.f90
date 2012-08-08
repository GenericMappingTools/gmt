! This is part of netCDF-4. Copyright 2006 UCAR. See COPYRIGHT file
! for details.

! This file contains the extra F90 constants needed for netCDF-4.

! $Id$

! extra data types:
integer, parameter, public :: &
     nf90_ubyte = 7, &
     nf90_ushort = 8, &
     nf90_uint = 9, &
     nf90_int64 = 10, &
     nf90_uint64 = 11, &
     nf90_string = 12, &
     nf90_vlen = 13, &
     nf90_opaque = 14, &
     nf90_enum = 15, &
     nf90_compound = 16

                        
! extra default fill values:
integer (kind =  TwoByteInt),  parameter, public :: &
     nf90_fill_ubyte  = 255,                        &
     nf90_fill_uint1  = nf90_fill_ubyte
integer (kind =  FourByteInt),  parameter, public :: &
     nf90_fill_ushort = 65535,                      &
     nf90_fill_uint2  = nf90_fill_ushort
integer (kind = EightByteInt),  parameter, public :: &
     nf90_fill_uint   = 4294967295_8

! Extra file create mode flags.
integer, parameter, public :: &
     nf90_netcdf4 = 4096, &
     nf90_hdf5 = 4096, & ! deprecated
     nf90_classic_model = 256

! Flags for parallel access.
integer, parameter, public :: nf90_independent = 0, nf90_collective = 1

! Flags for parallel I/O.
integer, parameter, public :: nf90_mpiio = 8192, nf90_mpiposix = 16384, &
     nf90_pnetcdf = 32768
  
! Extra variable flags.
integer, parameter, public :: &
     nf90_chunk_seq = 0, &
     nf90_chunk_sub = 1, &
     nf90_chunk_sizes = 2, &
     nf90_endian_native = 0, &
     nf90_endian_little = 1, &
     nf90_endian_big = 2, &
     nf90_chunked = 0, &
     nf90_notcontiguous = 0, &
     nf90_contiguous = 1, &
     nf90_nochecksum = 0, &
     nf90_fletcher32 = 1, &
     nf90_noshuffle = 0, &
     nf90_shuffle = 1, &
     nf90_szip_ec_option_mask = 4, &
     nf90_szip_nn_option_mask = 32

! Extra error codes.
integer, parameter, public :: &
     nf90_ehdferr = -101, & ! Error at HDF5 layer. 
     nf90_ecantread = -102, & ! Can't read. 
     nf90_ecantwrite = -103, & ! Can't write. 
     nf90_ecantcreate = -104, & ! Can't create. 
     nf90_efilemeta = -105, & ! Problem with file metadata. 
     nf90_edimmeta = -106, & ! Problem with dimension metadata. 
     nf90_eattmeta = -107, & ! Problem with attribute metadata. 
     nf90_evarmeta = -108, & ! Problem with variable metadata. 
     nf90_enocompound = -109, & ! Not a compound type. 
     nf90_eattexists = -110, & ! Attribute already exists. 
     nf90_enotnc4 = -111, & ! Attempting netcdf-4 operation on netcdf-3 file.   
     nf90_estrictnc3 = -112, & ! Attempting netcdf-4 operation on strict nc3 netcdf-4 file.   
     nf90_enotnc3 = -113, & ! Attempting netcdf-3 operation on netcdf-4 file.   
     nf90_enopar = -114, & ! Parallel operation on file opened for non-parallel access.   
     nf90_eparinit = -115, & ! Error initializing for parallel access.   
     nf90_ebadgrpid = -116, & ! Bad group ID.   
     nf90_ebadtypid = -117, & ! Bad type ID.   
     nf90_etypdefined = -118, & ! Type has already been defined and may not be edited. 
     nf90_ebadfield = -119, & ! Bad field ID.   
     nf90_ebadclass = -120, & ! Bad class.   
     nf90_emaptype = -121, & ! Mapped access for atomic types only.   
     nf90_elatefill = -122, & ! Attempt to define fill value when data already exists. 
     nf90_elatedef = -122 ! Attempt to define var properties, like deflate, after enddef. 

! This is the position of NC_NETCDF4 in cmode, counting from the
! right, starting (uncharacteristically for fortran) at 0. It's needed
! for the BTEST function calls.
integer, parameter, private :: NETCDF4_BIT = 12
