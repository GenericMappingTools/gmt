program tst_f03_file
 use libcf
 use netcdf
 implicit none
  integer:: ncid
  integer, parameter :: MAX_LEN = 256
  character (len = *), parameter :: FILE_NAME = "tst_f03_file.nc"
  character (len = *), parameter :: TITLE = 'Viscount of Lafayette'
  character (len = *), parameter :: HISTORY = 'It all started one dark and stormy night...'
  character (len = *), parameter :: INSTITUTION = 'Happy Home for Histronic Hacks'
  character (len = *), parameter :: COMMENT = 'I like beans'
  character (len = *), parameter :: SOURCE = 'Fevered imagination'
  character (len = *), parameter :: REFERENCES = 'works of Emily Dickenson, Edgar Allen Poe'
  character (len = MAX_LEN) :: title_in, history_in, institution_in, comment_in, &
       source_in, references_in
 integer:: title_len, history_len, institution_len, comment_len, source_len, references_len
  
 print *,'hi'
  print *,'*** Testing f90 file API. All hail the ', TITLE, '!'
  call check(nf90_create(FILE_NAME, 0, ncid))
  call check(cf_def_file(ncid, convention = '1.0', title = TITLE, history = HISTORY, institution = INSTITUTION))
  call check(cf_def_file(ncid, convention = '1.0', title = TITLE, history = HISTORY, &
       institution = INSTITUTION, comment = COMMENT, source = SOURCE, references = REFERENCES))
  call check(cf_add_history(ncid, 'by the pale and sickly light of a bloated, over-described, harvest moon.'))
  call check(nf90_close(ncid))

  call check(nf90_open(FILE_NAME, NF90_NOWRITE, ncid))
  call check(cf_inq_file(ncid, title_len = title_len, title = title_in, history_len = history_len, &
       history = history_in, institution_len = institution_len, institution = institution_in, &
       comment_len = comment_len, comment = comment_in, source_len = source_len, source = source_in, &
       references_len = references_len, references = references_in))
  if (title_in .ne. TITLE .or. title_len .ne. len(title)) stop 3
  if (institution_len .ne. len(INSTITUTION) .or. institution_in .ne. INSTITUTION) stop 4
  if (comment_len .ne. len(COMMENT) .or. comment_in .ne. COMMENT) stop 5
  if (source_len .ne. len(SOURCE) .or. source_in .ne. SOURCE) stop 6
  if (references_len .ne. len(REFERENCES) .or. references_in .ne. REFERENCES) stop 7
  call check(nf90_close(ncid))
  print *,'*** SUCCESS!!!'
  
contains
  subroutine check(status)
    integer, intent ( in) :: status
    
    if(status /= nf90_noerr) then
       print *, trim(nf90_strerror(status))
       stop 2
    end if
  end subroutine check
end program tst_f03_file
