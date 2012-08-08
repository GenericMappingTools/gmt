program tst_f90_nc4
  use typeSizes
  use netcdf
  implicit none
  integer :: fh, ierr, dimid, varid, ndim, nvar
  character (len = *), parameter :: FILE_NAME = "tst_f90_nc4.nc"

  print *, ''
  print *,'*** testing simple netCDF-4 file.'

  call check(nf90_create(FILE_NAME, NF90_NETCDF4, fh))
  call check(nf90_def_dim(fh, 'fred', 10, dimid))
  call check(nf90_def_var(fh, 'john', NF90_INT, (/dimid/), varid))
  call check(nf90_close(fh))
  
  ! Check the file.
  call check(nf90_open(FILE_NAME, NF90_WRITE, fh))
  call check(nf90_inquire(fh, nDimensions = ndim, nVariables = nvar))
  if (nvar .ne. 1 .or. ndim .ne. 1) stop 3
  call check(nf90_close(fh))
  print *,'*** OK!'

  print *,'*** Testing simple classic file.'

  call check(nf90_create(FILE_NAME, NF90_CLOBBER, fh))
  call check(nf90_def_dim(fh, 'fred', 10, dimid))
  call check(nf90_def_var(fh, 'john', NF90_INT, (/dimid/), varid))
  call check(nf90_close(fh))
  
  ! Check the file.
  call check(nf90_open(FILE_NAME, NF90_WRITE, fh))
  call check(nf90_inquire(fh, nDimensions = ndim, nVariables = nvar))
  if (nvar .ne. 1 .or. ndim .ne. 1) stop 3
  call check(nf90_close(fh))
  print *,'*** OK!'

  print *,'*** SUCCESS!'

contains
!     This subroutine handles errors by printing an error message and
!     exiting with a non-zero status.
  subroutine check(errcode)
    use netcdf
    implicit none
    integer, intent(in) :: errcode
    
    if(errcode /= nf90_noerr) then
       print *, 'Error: ', trim(nf90_strerror(errcode))
       stop 2
    endif
  end subroutine check
end program tst_f90_nc4

	
