! This is part of netCDF-4. Copyright 2006 UCAR. See COPYRIGHT file
! for details.

! This file contains the netcdf-4 file open and create functions.

! $Id$
! -------
function nf90_open(path, mode, ncid, chunksize, cache_size, cache_nelems, &
     cache_preemption, comm, info)
  implicit none
  character (len = *), intent(in) :: path
  integer, intent(in) :: mode
  integer, intent(out) :: ncid
  integer, optional, intent(inout) :: chunksize
  integer, optional, intent(in) :: cache_size, cache_nelems
  real, optional, intent(in) :: cache_preemption
  integer, optional, intent(in) :: comm, info
  integer :: size_in, nelems_in, preemption_in
  integer :: size_out, nelems_out, preemption_out, ret
  integer :: nf90_open

  ! If using parallel, both comm and info must be provided.
  if (present(comm) .and. .not. present(info)) then
     nf90_open = NF90_EINVAL;
     return
  end if

  ! If the user specified chuck cache parameters, use them. But user
  ! may have specified one, two, or three settings. Leave the others
  ! unchanged.
  if (present(cache_size) .or. present(cache_nelems) .or. &
       present(cache_preemption)) then
     ret = nf_get_chunk_cache(size_in, nelems_in, preemption_in)
     if (ret .ne. nf90_noerr) then
        nf90_open = ret
        return
     end if
     if (present(cache_size)) then
        size_out = cache_size
     else
        size_out = size_in
     end if
     if (present(cache_nelems)) then
        nelems_out = cache_nelems
     else
        nelems_out = nelems_in
     end if
     if (present(cache_preemption)) then
        preemption_out = cache_preemption
     else
        preemption_out = preemption_in
     end if
     nf90_open = nf_set_chunk_cache(size_out, nelems_out, preemption_out)
     if (nf90_open .ne. nf90_noerr) return
  end if

  ! Do the open.
  if(present(chunksize)) then
     nf90_open = nf__open(path, mode, chunksize, ncid)
  else
     if (present(comm)) then
        nf90_open = nf_open_par(path, mode, comm, info, ncid)
     else
        nf90_open = nf_open(path, mode, ncid)
     end if
  end if
  if (nf90_open .ne. nf90_noerr) return

  ! If settings were changed, reset chunk chache to original settings.
  if (present(cache_size) .or. present(cache_nelems) .or. &
       present(cache_preemption)) then
     nf90_open = nf_set_chunk_cache(size_in, nelems_in, preemption_in)
  end if

end function nf90_open
! -------
function nf90_create(path, cmode, ncid, initialsize, chunksize, cache_size, &
     cache_nelems, cache_preemption, comm, info)
  implicit none
  character (len = *), intent(in) :: path
  integer, intent(in) :: cmode
  integer, intent(out) :: ncid
  integer, optional, intent(in) :: initialsize
  integer, optional, intent(inout) :: chunksize
  integer, optional, intent(in) :: cache_size, cache_nelems
  integer, optional, intent(in) :: cache_preemption
  integer, optional, intent(in) :: comm, info
  integer :: size_in, nelems_in, preemption_in
  integer :: size_out, nelems_out, preemption_out, ret
  integer :: nf90_create
  integer :: fileSize, chunk
  integer :: x

  ! Just ignore options netCDF-3 options for netCDF-4 files, or
  ! netCDF-4 options, for netCDF-3 files, so that the same user code
  ! can work for both cases.

  ! If using parallel, but comm and info must be provided.
  if (present(comm) .and. .not. present(info)) then
     nf90_create = NF90_EINVAL;
     return
  end if

  ! If the user specified chuck cache parameters, use them. But user
  ! may have specified one, two, or three settings. Leave the others
  ! unchanged.
  if (present(cache_size) .or. present(cache_nelems) .or. &
       present(cache_preemption)) then
     nf90_create = nf_get_chunk_cache(size_in, nelems_in, preemption_in)
     if (nf90_create .ne. nf90_noerr) return
     if (present(cache_size)) then
        size_out = cache_size
     else
        size_out = size_in
     end if
     if (present(cache_nelems)) then
        nelems_out = cache_nelems
     else
        nelems_out = nelems_in
     end if
     if (present(cache_preemption)) then
        preemption_out = cache_preemption
     else
        preemption_out = preemption_in
     end if
     nf90_create = nf_set_chunk_cache(size_out, nelems_out, preemption_out)
     if (nf90_create .ne. nf90_noerr) return
  end if

  ! Do the file create.
  if(.not. (present(initialsize) .or. present(chunksize)) ) then
     if (present(comm)) then
        nf90_create = nf_create_par(path, cmode, comm, info, ncid)
     else
        nf90_create = nf_create(path, cmode, ncid)
     end if
  else
     ! Default values per man page
     filesize = 0; chunk = nf90_sizehint_default
     if(present(initialsize)) filesize = initialsize
     if(present(chunksize  )) chunk    = chunksize
     nf90_create = nf__create(path, cmode, filesize, chunk, ncid)
     ! Pass back the value actually used
     if(present(chunksize  )) chunksize = chunk
  end if
  if (nf90_create .ne. nf90_noerr) return

  ! If settings were changed, reset chunk chache to original settings.
  if (present(cache_size) .or. present(cache_nelems) .or. &
       present(cache_preemption)) then
     nf90_create = nf_set_chunk_cache(size_in, nelems_in, preemption_in)
  end if


end function nf90_create
