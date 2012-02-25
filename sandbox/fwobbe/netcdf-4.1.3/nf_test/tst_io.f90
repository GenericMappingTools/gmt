! Copyright 2007, UCAR/Unidata. See netcdf/COPYRIGHT file for copying
! and redistribution conditions.

! This program tests io times with large files (> 4 GB) in
! netCDF-4. This is user-contributed code.

! $Id$
program tst_io
  use netcdf ! access to netcdf module
  implicit none
  integer, parameter :: prsz1 = 50, prsz2 = 50, &
       prsz3 = 50, prsz4 = 50, repct = 10
  integer :: i1, i2, i3, i4, j, k, ticksPerSec
  real :: psr
  integer :: clockRate
  integer :: start, now, wrint1, ncint1, wrint2, ncint2, &
       wrint3, ncint3, iosb, iosn, size
  real, dimension (prsz1, prsz2, prsz3, prsz4) :: x
  character(len = *), parameter :: nclFilenm1 = 'tst_io1.nc', &
       nclFilenm2 = 'tst_io2.nc', nclFilenm3 = 'tst_io3.nc', &
       nclFilenm4 = 'tst_io4.nc', nclFilenm5 = 'tst_io5.nc', &
       nclFilenm6 = 'tst_io6.nc', nclFilenm7 = 'tst_io7.nc', &
       nclFilenm8 = 'tst_io8.nc', nclFilenm9 = 'tst_io9.nc', &
       nclFilenm10 = 'tst_io10.nc', nclFilenm11 = 'tst_io11.nc'
  ! needed for netcdf
  integer :: ncid, x1id, x2id, x3id, x4id, vrid
  integer :: vrids, vridt, vridu, vridv, vridw, vridx, vridy, vridz

  psr = 1.7/real(prsz1)

  print *, "Starting data initialization."
  size = (prsz1 * prsz2 * prsz3 * prsz4 )/ 250000
  do i1 = 1, prsz1
     do i2 = 1, prsz2
        do i3 = 1, prsz3 ! Jackson Pollock it is not
           do i4 = 1, prsz4
              x(i1, i2, i3, i4) = sin(i1*psr)*(0.5 + cos(i2*psr))+(psr/i3)+ i4/(10.0*prsz4)
           enddo
        enddo
     enddo
  enddo
  call system_clock(start, ticksPerSec)
  clockRate = 1000/ticksPerSec
  print 5, size, 1000.0/real(ticksPerSec)
5 format("Array sizes =", i4, "MB. Clock resolution = ", f6.3, " ms."/)

  ! First the binary writes
  call system_clock(start, ticksPerSec)
  write(1, iostat = iosb) x
  call system_clock(now)
  wrint1 = now - start
  call check (iosb, 1)
  print 1, size, "MB","binary write = ", wrint1 * clockRate
1 format("Time for", i5, a, a26, i6, " msec. ")

  call system_clock(start)
  do i1 = 1, repct
     rewind (2, iostat = iosb)
     call check (iosb, 2)
     write(2, iostat = iosb) x
     call check (iosb, 3)
  enddo
  call system_clock(now)
  wrint2 = now - start
  call check (iosb, 4)
  close(2, iostat = iosb)
  call check (iosb, 5)
  print 2, size, "MB", repct, " binary rewind/writes = ", wrint2 * clockRate
2 format("Time for", i5, a, i3, a23, i6," msec. ", a, i6)
  close(1, iostat = iosb)

  call system_clock(start)
  write(13, iostat = iosb) x
  call check (iosb, 6)
  write(14, iostat = iosb) x
  call check (iosb, 7)
  write(15, iostat = iosb) x
  call check (iosb, 8)
  write(16, iostat = iosb) x
  call check (iosb, 9)
  write(17, iostat = iosb) x
  call check (iosb, 10)
  write(18, iostat = iosb) x
  call check (iosb, 11)
  write(19, iostat = iosb) x
  call check (iosb, 12)
  write(20, iostat = iosb) x
  call check (iosb, 13)
  call system_clock(now)
  wrint3 = now - start
  print 2, size, "MB", 8, " binary file writes = ", wrint3 * clockRate
  do i1 = 1, 8
     close(12 + i1, iostat = iosb)
     call check (iosb, 14)
  enddo

  ! Next the netCDF writes
  call setupNetCDF (nclFilenm1, ncid, vrid, x, prsz1, prsz2, prsz3, prsz4, &
       x1id, x2id, x3id, x4id, NF90_CLOBBER, 20)
  call system_clock(start)
  call check (NF90_PUT_VAR(ncid, vrid, x), 18)
  call system_clock(now)
  ncint1 = now - start
  print 3, size, "MB"," netcdf write = ", ncint1 * clockRate, &
       real(ncint1)/real (wrint1)
3 format("Time for", i5, a, a25, i7, " msec. Spd ratio = ", f5.2)

  call check (NF90_CLOSE(ncid), 14)

  call system_clock(start)
  do i1 = 1, repct
     call setupNetCDF (nclFilenm1, ncid, vrid, x, prsz1, prsz2, prsz3, prsz4, &
          x1id, x2id, x3id, x4id, NF90_CLOBBER, 130)
     call check (NF90_PUT_VAR(ncid, vrid, x), 23 + i1)
     call check (NF90_CLOSE(ncid), 15)
  enddo
  call system_clock(now)
  ncint2 = now - start
  print 4, size, repct, " repeated netcdf writes = ", ncint2 * clockRate, &
       real(ncint2)/real(wrint2);
4 format("Time for", i5, "MB", i3, a22, i7, " msec. Spd ratio = ", f5.2)

!   call system_clock(start)
!   call setupNetCDF (nclFilenm3, ncid, vrids, s, prsz1, prsz2, prsz3, prsz4, &
!        x1id, x2id, x3id, x4id, NF90_CLOBBER, 20)
!   call setupNetCDF (nclFilenm4, ncid, vridt, t, prsz1, prsz2, prsz3, prsz4, &
!        x1id, x2id, x3id, x4id, NF90_CLOBBER, 30)
!   call setupNetCDF (nclFilenm5, ncid, vridu, u, prsz1, prsz2, prsz3, prsz4, &
!        x1id, x2id, x3id, x4id, NF90_CLOBBER, 40)
!   call setupNetCDF (nclFilenm6, ncid, vridv, v, prsz1, prsz2, prsz3, prsz4, &
!        x1id, x2id, x3id, x4id, NF90_CLOBBER, 50)
!   call setupNetCDF (nclFilenm7, ncid, vridw, w, prsz1, prsz2, prsz3, prsz4, &
!        x1id, x2id, x3id, x4id, NF90_CLOBBER, 60)
!   call setupNetCDF (nclFilenm8, ncid, vridx, x, prsz1, prsz2, prsz3, prsz4, &
!        x1id, x2id, x3id, x4id, NF90_CLOBBER, 70)
!   call setupNetCDF (nclFilenm9, ncid, vridy, y, prsz1, prsz2, prsz3, prsz4, &
!        x1id, x2id, x3id, x4id, NF90_CLOBBER, 80)
!   call setupNetCDF (nclFilenm10, ncid, vridz, z, prsz1, prsz2, prsz3, prsz4, &
!        x1id, x2id, x3id, x4id, NF90_CLOBBER, 90)
!   call check (NF90_PUT_VAR(ncid, vrids, s), 118)
!   call check (NF90_PUT_VAR(ncid, vridt, t), 119)
!   call check (NF90_PUT_VAR(ncid, vridu, u), 120)
!   call check (NF90_PUT_VAR(ncid, vridv, v), 121)
!   call check (NF90_PUT_VAR(ncid, vridw, w), 122)
!   call check (NF90_PUT_VAR(ncid, vridx, x), 123)
!   call check (NF90_PUT_VAR(ncid, vridy, y), 124)
!   call check (NF90_PUT_VAR(ncid, vridz, z), 125)
!   call system_clock(now)
!   ncint3 = now - start
!   call check (NF90_CLOSE(ncid), 16)
!   print 4, size, 8, " netcdf file writes = ", ncint3 * clockRate, &
!        real(ncint3)/real(wrint3);

contains
  subroutine check (st, n) ! checks the return error code
    integer, intent (in) :: st, n
    if ((n < 10.and.st /= 0).or.(n > 10.and.st /= NF90_noerr))then
       print *, "I/O error at", n, " status = ", st
       stop 2
    endif
  end subroutine check

  subroutine setupNetCDF(fn, nc, vr, vrnam, d1, d2, d3, d4, do1, do2, &
       do3, do4, stat, deb)
    integer, intent(in) :: d1, d2, d3, d4, stat, deb
    integer, intent(out) :: do1, do2, do3, do4, vr
    integer, intent(inout) :: nc
    integer, dimension(4) :: dimids (4)

    character(len = *), intent(in) :: fn
    real, dimension (d1, d2, d3, d4), intent (in) :: vrnam

    call check (NF90_CREATE (fn, stat, nc), deb + 1)
    call check (NF90_DEF_DIM(nc, "d1", d1, do1), deb + 2)
    call check (NF90_DEF_DIM(nc, "d2", d2, do2), deb + 3)
    call check (NF90_DEF_DIM(nc, "d3", d3, do3), deb + 4)
    call check (NF90_DEF_DIM(nc, "d4", d4, do4), deb + 5)

    dimids = (/ do1, do2, do3, do4 /)
    call check (NF90_DEF_VAR(nc, "data", NF90_REAL, dimids, vr), deb + 6)
    call check (NF90_ENDDEF (nc), deb + 7)

  end subroutine setupNetCDF

end program tst_io
