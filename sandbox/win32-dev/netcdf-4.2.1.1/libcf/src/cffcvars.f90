module libcf
  use iso_c_binding
  implicit none

  ! For some odd reason, netCDF f90 has NF90_GLOBAL=0, but NC_GLOBAL
  ! is really -1. WTF?
  integer, parameter:: NC_GLOBAL = -1

  ! These interfaces map to the C library calls.
  interface
     function nccf_def_convention(ncid) bind(c)
       use, intrinsic :: iso_c_binding
       integer(c_int), value :: ncid
       integer(c_int) :: nccf_def_convention
     end function nccf_def_convention

     function nccf_def_file(ncid, title, history) bind(c)
       use, intrinsic :: iso_c_binding
       integer(c_int), value :: ncid
       character(c_char), intent(in) :: title
       character(c_char), intent(in) :: history
       integer(c_int) :: nccf_def_file
     end function nccf_def_file

     function nccf_inq_file(ncid, title_len, title, history_len, history) bind(c)
       use, intrinsic :: iso_c_binding
       integer(c_int), value :: ncid
       integer(c_size_t), intent(out) :: title_len
       character(c_char), intent(out) :: title
       integer(c_size_t), intent(out) :: history_len
       character(c_char), intent(out) :: history
       integer(c_int) :: nccf_inq_file
     end function nccf_inq_file

     function nccf_def_notes(ncid, varid, institution, source, comment, &
          references) bind(c)
       use, intrinsic :: iso_c_binding
       integer(c_int), value :: ncid, varid
       character(c_char), intent(in) :: institution, source, comment, references
       integer(c_int) :: nccf_def_notes
     end function nccf_def_notes

     function nccf_inq_notes(ncid, varid, institution_len, institution, &
          source_len, source, comment_len, comment, references_len, &
          references) bind(c)
       use, intrinsic :: iso_c_binding
       integer(c_int), value :: ncid, varid
       integer(c_size_t), intent(out) :: institution_len
       character(c_char), intent(out) :: institution
       integer(c_size_t), intent(out) :: source_len
       character(c_char), intent(out) :: source
       integer(c_size_t), intent(out) :: comment_len
       character(c_char), intent(out) :: comment
       integer(c_size_t), intent(out) :: references_len
       character(c_char), intent(out) :: references
       integer(c_int) :: nccf_inq_notes
     end function nccf_inq_notes

     function nccf_add_history(ncid, history) bind(c)
       use, intrinsic :: iso_c_binding
       integer(c_int), value :: ncid
       character(c_char), intent(in) :: history
       integer(c_int) :: nccf_add_history
     end function nccf_add_history

  end interface

contains

  ! These functions are the new libcf F90 API.
  function cf_def_file(ncid, convention, title, history, institution, &
       source, comment, references)
    use iso_c_binding
    use netcdf
    implicit none
    integer, intent(in)::ncid
    character(len = *), optional, intent(in):: convention, title, history, &
         institution, source, comment, references
    integer:: cf_def_file
    integer(c_int):: c_ncid
    character (len = 256) c_title, c_history, c_institution, c_source
    character (len = 256) c_comment, c_references

    c_ncid = ncid
    cf_def_file = NF90_NOERR

    if (present(convention)) then
       cf_def_file = nccf_def_convention(c_ncid)
       if (cf_def_file .ne. 0) then 
          return
       endif
    endif
    if (present(title) .or. present(history)) then
       call prepare_write(title, c_title)
       call prepare_write(history, c_history)
       cf_def_file = nccf_def_file(ncid, c_title, c_history)
    endif
    if (present(institution) .or. present(source) .or. present(comment) .or. &
         present(references)) then    
       call prepare_write(institution, c_institution)
       call prepare_write(source, c_source)
       call prepare_write(comment, c_comment)
       call prepare_write(references, c_references)
       cf_def_file = nccf_def_notes(ncid, NC_GLOBAL, c_institution, c_source, &
            c_comment, c_references)
    endif

  end function cf_def_file

  subroutine prepare_write(in_str, out_str)
    use iso_c_binding
    use netcdf
    implicit none
    character(len = *), optional, intent(in):: in_str
    character(len = *), optional, intent(out):: out_str
    
    if (present(in_str)) then
       out_str = in_str
       out_str(len_trim(in_str) + 1:len_trim(in_str) + 1) = achar(0)
    else
       out_str = achar(0)
    endif
  end subroutine prepare_write

  function cf_add_history(ncid, history)
    use iso_c_binding
    implicit none
    integer, intent(in)::ncid
    character (len = *), intent(in):: history
    integer:: cf_add_history
    integer(c_int):: c_ncid
    character(len = 256):: c_history

    c_ncid = ncid
    c_history = history
    c_history(len_trim(history) + 1:len_trim(history) + 1) = achar(0)
    cf_add_history = nccf_add_history(ncid, c_history)

  end function cf_add_history

  function cf_inq_file(ncid, title_len, title, history_len, history, &
       institution_len, institution, source_len, source, comment_len, &
       comment, references_len, references)
    use iso_c_binding
    use netcdf
    implicit none
    integer, intent(in):: ncid
    integer, optional, intent(out) :: title_len, history_len, institution_len
    integer, optional, intent(out) :: source_len, comment_len, references_len
    character (len = *), optional, intent(out):: title, history, institution
    character (len = *), optional, intent(out):: source, comment, references
    integer:: cf_inq_file
    integer(c_size_t):: c_title_len, c_history_len, c_institution_len
    integer(c_size_t):: c_source_len, c_comment_len, c_references_len
!    character, dimension(:), allocatable :: c_history, c_title
    character(len = 4000) :: c_history, c_title, c_institution
    character(len = 4000) :: c_source, c_comment, c_references
    character(len = 1), parameter:: NULL_TERM = achar(0)
    character, pointer :: null_ptr => NULL()

    cf_inq_file = NF90_NOERR
    if (present(title) .or. present(history) .or. present(title_len) &
         .or. present(history_len)) then
       ! Find the length of the title and history.
       cf_inq_file = nccf_inq_file(ncid, c_title_len, null_ptr, c_history_len, null_ptr)
       if (cf_inq_file .ne. NF90_NOERR) return

       ! Allocate memory for the title and the history strings.
       !    allocate(c_title(c_title_len))
       !    allocate(c_history(c_history_len))

       ! Get the title and history strings.
       cf_inq_file = nccf_inq_file(ncid, c_title_len, c_title, c_history_len, c_history)
       if (cf_inq_file .ne. NF90_NOERR) return

       ! Copy the strings.
       if (present(title)) then
          title = c_title(:len(title))
          if (index(title, NULL_TERM) .gt. 0) then
             title(index(title, NULL_TERM):len(title)) = ' '
          endif
       endif
       if (present(history)) then
          history = c_history(:len(history))
          if (index(history, NULL_TERM) .gt. 0) then
             history(index(history, NULL_TERM):len(history)) = ' '
          endif
       endif

       ! Free memory.
       !deallocate(c_title)
       !deallocate(c_history)

       ! Does the user want the lengths?
       if (present(title_len)) then
          title_len = c_title_len - 1
       endif
       if (present(history_len)) then
          history_len = c_history_len - 1
       endif
    endif ! present(title) .or. present(history)

    if (present(institution) .or. present(source) .or. present(comment) &
         .or. present(references) .or. present(institution_len) .or. &
         present(source_len) .or. present(comment_len) &
         .or. present(references_len)) then

       ! Find the length of the institution and source.
       cf_inq_file = nccf_inq_notes(ncid, NC_GLOBAL, c_institution_len, null_ptr, &
            c_source_len, null_ptr, c_comment_len, null_ptr, c_references_len, null_ptr)
       if (cf_inq_file .ne. NF90_NOERR) return

       ! Allocate memory for the institution and the source strings.
       !    allocate(c_institution(c_institution_len))
       !    allocate(c_source(c_source_len))

       ! Get the institution and source strings.
       cf_inq_file = nccf_inq_notes(ncid, NC_GLOBAL, c_institution_len, c_institution, &
            c_source_len, c_source, c_comment_len, c_comment, c_references_len, &
            c_references)
       if (cf_inq_file .ne. NF90_NOERR) return

       ! Copy the strings.
       if (present(institution)) then
          institution = c_institution(:len(institution))
          if (index(institution, NULL_TERM) .gt. 0) then
             institution(index(institution, NULL_TERM):len(institution)) = ' '
          endif
       endif
       if (present(source)) then
          source = c_source(:len(source))
          if (index(source, NULL_TERM) .gt. 0) then
             source(index(source, NULL_TERM):len(source)) = ' '
          endif
       endif
       if (present(comment)) then
          comment = c_comment(:len(comment))
          if (index(comment, NULL_TERM) .gt. 0) then
             comment(index(comment, NULL_TERM):len(comment)) = ' '
          endif
       endif
       if (present(references)) then
          references = c_references(:len(references))
          if (index(references, NULL_TERM) .gt. 0) then
             references(index(references, NULL_TERM):len(references)) = ' '
          endif
       endif

       ! Free memory.
       !deallocate(c_institution)
       !deallocate(c_source)

       ! Does the user want the lengths?
       if (present(institution_len)) then
          institution_len = c_institution_len - 1
       endif
       if (present(source_len)) then
          source_len = c_source_len - 1
       endif
       if (present(comment_len)) then
          comment_len = c_comment_len - 1
       endif
       if (present(references_len)) then
          references_len = c_references_len - 1
       endif
       
    endif ! present institution, source, comment, or references

    end function cf_inq_file
end module libcf
