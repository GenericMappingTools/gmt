! This is part of the netCDF F90 API, or. Copyright 2006 UCAR. See COPYRIGHT file
! for details.

! This file contains the netcdf file functions that are shared by
! netcdf-3 and netcdf-4.

! $Id$
! -------
function nf90_inq_libvers()
  character(len = 80) :: nf90_inq_libvers

  nf90_inq_libvers = nf_inq_libvers()
end function nf90_inq_libvers
! -------
function nf90_strerror(ncerr)
  integer, intent( in) :: ncerr
  character(len = 80)  :: nf90_strerror

  nf90_strerror = nf_strerror(ncerr)
end function nf90_strerror
! -------
!
! File level control routines:
!
function nf90_inq_base_pe(ncid, pe)
  integer, intent( in) :: ncid
  integer, intent(out) :: pe
  integer              :: nf90_inq_base_pe

  nf90_inq_base_pe = nf_inq_base_pe(ncid, pe)
end function nf90_inq_base_pe
! -------
function nf90_set_base_pe(ncid, pe)
  integer, intent( in) :: ncid, pe
  integer              :: nf90_set_base_pe

  nf90_set_base_pe = nf_set_base_pe(ncid, pe)
end function nf90_set_base_pe
! -------
function nf90_create_mp(path, cmode, initalsz, basepe, chunksizehint, ncid)
  character (len = *), intent( in) :: path
  integer,             intent( in) :: cmode, initalsz, basepe, chunksizehint
  integer,             intent(out) :: ncid
  integer                          :: nf90_create_mp

  nf90_create_mp = nf__create_mp(path, cmode, initalsz, basepe, chunksizehint, ncid)
end function nf90_create_mp
! -------
function nf90_open_mp(path, mode, basepe, chunksizeint, ncid)
  character (len = *), intent( in) :: path
  integer,             intent( in) :: mode, basepe, chunksizeint
  integer,             intent(out) :: ncid
  integer                          :: nf90_open_mp

  nf90_open_mp = nf__open_mp(path, mode, basepe, chunksizeint, ncid)
end function nf90_open_mp
! -------
function nf90_set_fill(ncid, fillmode, old_mode)
  integer, intent( in) :: ncid, fillmode 
  integer, intent(out) :: old_mode
  integer              :: nf90_set_fill

  nf90_set_fill = nf_set_fill(ncid, fillmode, old_mode)
end function nf90_set_fill
! -------
function nf90_redef(ncid)
  integer, intent( in) :: ncid
  integer              :: nf90_redef

  nf90_redef = nf_redef(ncid)
end function nf90_redef
! -------
function nf90_enddef(ncid, h_minfree, v_align, v_minfree, r_align)
  integer,           intent( in) :: ncid
  integer, optional, intent( in) :: h_minfree, v_align, v_minfree, r_align
  integer                        :: nf90_enddef

  integer :: hMinFree, VAlign, VMinFree, RAlign

  if(.not. any( (/ present(h_minfree), present(v_align), &
       present(v_minfree), present(r_align) /) ) )then
     nf90_enddef = nf_enddef(ncid)
  else 
     ! Default values per the man page
     hMinFree = 0; VMinFree = 0
     VAlign   = 4; RAlign   = 4
     if(present(h_minfree)) HMinFree = h_minfree
     if(present(v_align  )) VAlign   = v_align
     if(present(v_minfree)) VMinFree = v_minfree
     if(present(r_align  )) RAlign   = r_align
     nf90_enddef = nf__enddef(ncid, hMinFree, VAlign, VMinFree, RAlign)
  end if
end function nf90_enddef
! -------
function nf90_sync(ncid)
  integer, intent( in) :: ncid
  integer              :: nf90_sync

  nf90_sync = nf_sync(ncid)
end function nf90_sync
! -------
function nf90_abort(ncid)
  integer, intent( in) :: ncid
  integer              :: nf90_abort

  nf90_abort = nf_abort(ncid)
end function nf90_abort
! -------
function nf90_close(ncid)
  integer, intent( in) :: ncid
  integer              :: nf90_close

  nf90_close = nf_close(ncid)
end function nf90_close
! -------
function nf90_delete(name)
  character(len = *), intent( in) :: name
  integer                         :: nf90_delete

  nf90_delete = nf_delete(name)
end function nf90_delete

!
! A single file level inquiry routine 
! 
function nf90_inquire(ncid, nDimensions, nVariables, nAttributes, unlimitedDimId, formatNum)
  integer,           intent( in) :: ncid
  integer, optional, intent(out) :: nDimensions, nVariables, nAttributes, unlimitedDimId, formatNum
  integer                        :: nf90_inquire

  integer :: nDims, nVars, nGAtts, unlimDimId, frmt

  nf90_inquire = nf_inq(ncid, nDims, nVars, nGAtts, unlimDimId)
  if(present(nDimensions))    nDimensions    = nDims 
  if(present(nVariables))     nVariables     = nVars
  if(present(nAttributes))    nAttributes    = nGAtts
  if(present(unlimitedDimId)) unlimitedDimId = unlimDimId
  if(present(formatNum)) then
     nf90_inquire = nf_inq_format(ncid, frmt)
     formatNum = frmt
  endif
end function nf90_inquire


