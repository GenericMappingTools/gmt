! This is part of the netCDF F90 API, or. Copyright 2006 UCAR. See COPYRIGHT file
! for details.

! This file contains the netcdf-3 file open and create functions.

! $Id$
! -------
function nf90_open(path, mode, ncid, chunksize)
  character (len = *), intent(in   ) :: path
  integer,             intent(in   ) :: mode
  integer,             intent(  out) :: ncid
  integer, optional,   intent(inout) :: chunksize
  integer                            :: nf90_open

  if(present(chunksize)) then
     nf90_open = nf__open(path, mode, chunksize, ncid)
  else
     nf90_open = nf_open(path, mode, ncid)
  end if
end function nf90_open
! -------
function nf90_create(path, cmode, ncid, initialsize, chunksize)
  character (len = *), intent(in   ) :: path
  integer,             intent(in   ) :: cmode
  integer,             intent(  out) :: ncid
  integer, optional,   intent(in   ) :: initialsize
  integer, optional,   intent(inout) :: chunksize
  integer                            :: nf90_create

  integer :: fileSize, chunk

  if(.not. (present(initialsize) .or. present(chunksize)) ) then
     nf90_create = nf_create(path, cmode, ncid)
  else
     ! Default values per man page
     filesize = 0; chunk = nf90_sizehint_default
     if(present(initialsize)) filesize = initialsize
     if(present(chunksize  )) chunk    = chunksize
     nf90_create = nf__create(path, cmode, filesize, chunk, ncid)
     ! Pass back the value actually used
     if(present(chunksize  )) chunksize = chunk
  end if
end function nf90_create
