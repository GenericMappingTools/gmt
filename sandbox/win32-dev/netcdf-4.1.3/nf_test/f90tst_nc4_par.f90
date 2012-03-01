! This parallel test was contributed by Jim Edwards at UCAR. Thanks Jim!
program f90tst_nc4_par
  use netcdf
  use mpi
  implicit none

  character (len = *), parameter :: FILE_NAME = "f90tst_nc4_par.nc"
  integer :: nmode, ierr, fh, my_task, nprocs, i, varid
  integer :: dimid(3), start(3), count(3)
  real :: f(3)


  call MPI_INIT(ierr)
  call MPI_COMM_RANK(MPI_COMM_WORLD, my_task, ierr)
  call MPI_COMM_SIZE(MPI_COMM_WORLD, nprocs, ierr)

  if(nprocs/=8)then
     stop 'requires 8 tasks'
  end if


  nmode = ior(NF90_CLOBBER,NF90_NETCDF4)
  nmode = IOR(nmode, nf90_mpiio) 

  call handle_err(nf90_create(FILE_NAME, nmode, fh, &
       comm = MPI_COMM_WORLD, info = MPI_INFO_NULL))

  call handle_err(nf90_set_fill(fh, NF90_NOFILL, nmode))


  call handle_err(nf90_def_dim(fh, 'dim1', 6, dimid(1)))
  call handle_err(nf90_def_dim(fh, 'dim2', 4, dimid(2)))
  call handle_err(nf90_def_dim(fh, 'dim3', 1, dimid(3)))


  call handle_err(nf90_def_var(fh, 'var1', NF90_DOUBLE, dimid, varid))
  call handle_err(nf90_enddef(fh))


  do i=1,3
     f(i) = my_task*3+i
  end do

  count = (/3,1,1/)
  start(1) = mod(my_task,2)*3+1
  start(2) = my_task/2+1
  start(3) = 1

  print *,my_task, start, count, f

  call handle_err(nf90_put_var(fh, varid, f,start=start,count=count))

  call handle_err(nf90_close(fh))

  ! Reopen the file and check it.
  call handle_err(nf90_open(FILE_NAME, NF90_MPIIO, fh, &
       comm = MPI_COMM_WORLD, info = MPI_INFO_NULL))

  call handle_err(nf90_get_var(fh, varid, f, start=start, count=count))
  do i=1,3
     if (f(i) .ne. my_task*3+i) stop 3
  end do

  call handle_err(nf90_close(fh))
  call MPI_Finalize(ierr)

contains
  !     This subroutine handles errors by printing an error message and                                                                                                               
  !     exiting with a non-zero status.                                                                                                                                               
  subroutine handle_err(errcode)
    use netcdf
    implicit none
    integer, intent(in) :: errcode

    if(errcode /= nf90_noerr) then
       print *, 'Error: ', trim(nf90_strerror(errcode))
       stop 2
    endif
  end subroutine handle_err

end program f90tst_nc4_par

