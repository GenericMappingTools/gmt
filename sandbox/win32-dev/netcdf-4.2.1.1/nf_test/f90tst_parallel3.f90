!     This is part of the netCDF package.
!     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
!     See COPYRIGHT file for conditions of use.

!     This program tests netCDF-4 parallel I/O and fill values from
!     fortran. It creates a file like this:

! netcdf f90tst_parallel3 {
! dimensions:
! 	x = 16 ;
! 	y = 16 ;
! variables:
! 	byte byte(x, y) ;
! 	short short(x, y) ;
! 	int int(x, y) ;
! 	float float(x, y) ;
! 	double double(x, y) ;
! 	ubyte ubyte(x, y) ;
! 	ushort ushort(x, y) ;
! 	uint uint(x, y) ;

!     $Id$

program f90tst_parallel3
  use typeSizes
  use netcdf
  implicit none
  include 'mpif.h'
  
  ! This is the name of the data file we will create.
  character (len = *), parameter :: FILE_NAME = "f90tst_parallel3.nc"
  integer, parameter :: MAX_DIMS = 2
  integer, parameter :: NX = 16, NY = 16
  integer, parameter :: HALF_NX = NX/2, HALF_NY = NY/2
  integer, parameter :: NUM_PROC = 4
  integer, parameter :: NUM_VARS = 8
  integer, parameter :: CACHE_SIZE = 4194304, CACHE_NELEMS = 1013
  integer, parameter :: CACHE_PREEMPTION = 79
  character (len = *), parameter :: var_name(NUM_VARS) = &
       (/ 'byte__', 'short_', 'int___', 'float_', 'double', 'ubyte_', 'ushort', 'uint__' /)
  integer :: ncid, varid(NUM_VARS), dimids(MAX_DIMS)
  integer :: var_type(NUM_VARS) = (/ nf90_byte, nf90_short, nf90_int, &
       nf90_float, nf90_double, nf90_ubyte, nf90_ushort, nf90_uint /)
  integer :: x_dimid, y_dimid
  integer :: byte_out(HALF_NY, HALF_NX), byte_in(HALF_NY, HALF_NX)
  integer :: short_out(HALF_NY, HALF_NX), short_in(HALF_NY, HALF_NX)
  integer :: int_out(HALF_NY, HALF_NX), int_in(HALF_NY, HALF_NX)
  real :: areal_out(HALF_NY, HALF_NX), areal_in(HALF_NY, HALF_NX)
  real :: double_out(HALF_NY, HALF_NX), double_in(HALF_NY, HALF_NX)
  integer :: ubyte_out(HALF_NY, HALF_NX), ubyte_in(HALF_NY, HALF_NX)
  integer :: ushort_out(HALF_NY, HALF_NX), ushort_in(HALF_NY, HALF_NX)
  integer (kind = EightByteInt) :: uint_out(HALF_NY, HALF_NX), uint_in(HALF_NY, HALF_NX)
  integer :: nvars, ngatts, ndims, unlimdimid, file_format
  integer :: x, y, v
  integer :: p, my_rank, ierr
  integer :: start(MAX_DIMS), count(MAX_DIMS)
  integer :: ret

  call MPI_Init(ierr)
  call MPI_Comm_rank(MPI_COMM_WORLD, my_rank, ierr)
  call MPI_Comm_size(MPI_COMM_WORLD, p, ierr)

  if (my_rank .eq. 0) then
     print *, ' '
     print *, '*** Testing netCDF-4 parallel I/O with fill values.'
  endif

  ! There must be 4 procs for this test.
  if (p .ne. 4) then
     print *, 'Sorry, this test program must be run on four processors.'
     stop 1
  endif

  ! Create some pretend data.
  do x = 1, HALF_NX
     do y = 1, HALF_NY
        byte_out(y, x) = my_rank * (-1)
        short_out(y, x) =  my_rank * (-2)
        int_out(y, x) = my_rank * (-4)
        areal_out(y, x) = my_rank * 2.5
        double_out(y, x) = my_rank * (-4.5)
        ubyte_out(y, x) = my_rank
        ushort_out(y, x) = my_rank * 2
        uint_out(y, x) = my_rank * 4
     end do
  end do

  ! THis should fail, because I have not set either mpiposix or mpiio.
  ret = nf90_create(FILE_NAME, nf90_netcdf4, ncid, &
       comm = MPI_COMM_WORLD, info = MPI_INFO_NULL, cache_size = CACHE_SIZE, &
       cache_nelems = CACHE_NELEMS, cache_preemption = CACHE_PREEMPTION)
  if (ret /= nf90_einval) stop 8
  
  ! Create the netCDF file. 
  call check(nf90_create(FILE_NAME, IOR(nf90_netcdf4, nf90_mpiposix), ncid, &
       comm = MPI_COMM_WORLD, info = MPI_INFO_NULL, cache_size = CACHE_SIZE, &
       cache_nelems = CACHE_NELEMS, cache_preemption = CACHE_PREEMPTION))

  ! Define the dimensions.
  call check(nf90_def_dim(ncid, "x", NX, x_dimid))
  call check(nf90_def_dim(ncid, "y", NY, y_dimid))
  dimids =  (/ y_dimid, x_dimid /)

  ! Define the variables. 
  do v = 1, NUM_VARS
     call check(nf90_def_var(ncid, var_name(v), var_type(v), dimids, varid(v)))
  end do

  ! This will be the last collective operation.
  call check(nf90_enddef(ncid))

  ! Determine what part of the variable will be written/read for this
  ! processor. It's a checkerboard decomposition.
  count = (/ HALF_NX, HALF_NY /)
  if (my_rank .eq. 0) then
     start = (/ 1, 1 /)
  else if (my_rank .eq. 1) then
     start = (/ HALF_NX + 1, 1 /)
  else if (my_rank .eq. 2) then
     start = (/ 1, HALF_NY + 1 /)
  else if (my_rank .eq. 3) then
     start = (/ HALF_NX + 1, HALF_NY + 1 /)
  endif

  ! Write this processor's data, except for processor zero.
  if (my_rank .ne. 0) then
     call check(nf90_put_var(ncid, varid(1), byte_out, start = start, count = count))
     call check(nf90_put_var(ncid, varid(2), short_out, start = start, count = count))
     call check(nf90_put_var(ncid, varid(3), int_out, start = start, count = count))
     call check(nf90_put_var(ncid, varid(4), areal_out, start = start, count = count))
     call check(nf90_put_var(ncid, varid(5), double_out, start = start, count = count))
     call check(nf90_put_var(ncid, varid(6), ubyte_out, start = start, count = count))
     call check(nf90_put_var(ncid, varid(7), ushort_out, start = start, count = count))
     call check(nf90_put_var(ncid, varid(8), uint_out, start = start, count = count))
  endif

  ! Close the file. 
  call check(nf90_close(ncid))

  ! Reopen the file.
  call check(nf90_open(FILE_NAME, IOR(nf90_nowrite, nf90_mpiio), ncid, &
       comm = MPI_COMM_WORLD, info = MPI_INFO_NULL))
  
  ! Check some stuff out.
  call check(nf90_inquire(ncid, ndims, nvars, ngatts, unlimdimid, file_format))
  if (ndims /= 2 .or. nvars /= NUM_VARS .or. ngatts /= 0 .or. unlimdimid /= -1 .or. &
       file_format /= nf90_format_netcdf4) stop 2

  ! Read this processor's data.
  call check(nf90_get_var(ncid, varid(1), byte_in, start = start, count = count))
  call check(nf90_get_var(ncid, varid(2), short_in, start = start, count = count))
  call check(nf90_get_var(ncid, varid(3), int_in, start = start, count = count))
  call check(nf90_get_var(ncid, varid(4), areal_in, start = start, count = count))
  call check(nf90_get_var(ncid, varid(5), double_in, start = start, count = count))
  call check(nf90_get_var(ncid, varid(6), ubyte_in, start = start, count = count))
  call check(nf90_get_var(ncid, varid(7), ushort_in, start = start, count = count))
  call check(nf90_get_var(ncid, varid(8), uint_in, start = start, count = count))

  ! Check the data. All the data from the processor zero are fill
  ! value.
  do x = 1, HALF_NX
     do y = 1, HALF_NY
        if (my_rank .eq. 0) then
           if (byte_in(y, x) .ne. nf90_fill_byte) stop 3
           if (short_in(y, x) .ne. nf90_fill_short) stop 4
           if (int_in(y, x) .ne. nf90_fill_int) stop 5
           if (areal_in(y, x) .ne. nf90_fill_real) stop 6
           if (double_in(y, x) .ne. nf90_fill_double) stop 7
           if (ubyte_in(y, x) .ne. nf90_fill_ubyte) stop 8
           if (ushort_in(y, x) .ne. nf90_fill_ushort) stop 9
           if (uint_in(y, x) .ne. nf90_fill_uint) stop 10
        else 
           if (byte_in(y, x) .ne. (my_rank * (-1))) stop 13
           if (short_in(y, x) .ne. (my_rank * (-2))) stop 14
           if (int_in(y, x) .ne. (my_rank * (-4))) stop 15
           if (areal_in(y, x) .ne. (my_rank * (2.5))) stop 16
           if (double_in(y, x) .ne. (my_rank * (-4.5))) stop 17
           if (ubyte_in(y, x) .ne. (my_rank * (1))) stop 18
           if (ushort_in(y, x) .ne. (my_rank * (2))) stop 19
           if (uint_in(y, x) .ne. (my_rank * (4))) stop 20
        endif
     end do
  end do

  ! Close the file. 
  call check(nf90_close(ncid))

  call MPI_Finalize(ierr)

  if (my_rank .eq. 0) print *,'*** SUCCESS!'

contains
!     This subroutine handles errors by printing an error message and
!     exiting with a non-zero status.
  subroutine check(errcode)
    use netcdf
    implicit none
    integer, intent(in) :: errcode
    
    if(errcode /= nf90_noerr) then
       print *, 'Error: ', trim(nf90_strerror(errcode))
       stop 99
    endif
  end subroutine check
end program f90tst_parallel3

