!     This is part of the netCDF package.
!     Copyright 2008 University Corporation for Atmospheric Research/Unidata.
!     See COPYRIGHT file for conditions of use.

!     This program tests netCDF-4 int64 types from fortran 90.

!     $Id$

program tst_types2
  use typeSizes
  use netcdf
  implicit none
  
  ! This is the name of the data file we will create.
  character (len = *), parameter :: FILE_NAME = "tst_types2.nc"
  
  integer :: ncid, varid1, varid2, varid3, varid4, varid5, varid6, varid7
  integer :: dimid1, dimid2, dimid3, dimid4, dimid5, dimid6, dimid7
  integer :: dimids1(1), dimids2(2), dimids3(3), dimids4(4), dimids5(5), dimids6(6), dimids7(7)
  integer :: i1, i2, i3, i4, i5, i6, i7
  integer, parameter :: DLEN = 2
  integer (kind = EightByteInt) :: data1_in(DLEN), data1_out(DLEN)
  integer (kind = EightByteInt) :: data2_in(DLEN, DLEN), data2_out(DLEN, DLEN)
  integer (kind = EightByteInt) :: data3_in(DLEN, DLEN, DLEN), data3_out(DLEN, DLEN, DLEN)
  integer (kind = EightByteInt) :: data4_in(DLEN, DLEN, DLEN, DLEN), data4_out(DLEN, DLEN, DLEN, DLEN)
  integer (kind = EightByteInt) :: data5_in(DLEN, DLEN, DLEN, DLEN, DLEN), data5_out(DLEN, DLEN, DLEN, DLEN, DLEN)
  integer (kind = EightByteInt) :: data6_in(DLEN, DLEN, DLEN, DLEN, DLEN, DLEN), data6_out(DLEN, DLEN, DLEN, DLEN, DLEN, DLEN)
  integer (kind = EightByteInt) :: data7_in(DLEN, DLEN, DLEN, DLEN, DLEN, DLEN, DLEN), &
       data7_out(DLEN, DLEN, DLEN, DLEN, DLEN, DLEN, DLEN)
  integer (kind = EightByteInt), parameter :: REALLY_BIG = 9223372036854775807_EightByteInt

  print *, ''
  print *,'*** Testing netCDF-4 64-bit integer types from Fortran 90.'

  do i1 = 1, DLEN
     data1_out(i1) = REALLY_BIG
  end do
  do i2 = 1, DLEN
     do i1 = 1, DLEN
        data2_out(i1, i2) = REALLY_BIG - i1 - i2
     end do
  end do
  do i3 = 1, DLEN
     do i2 = 1, DLEN
        do i1 = 1, DLEN
           data3_out(i1, i2, i3) = REALLY_BIG - i1 - i2 - i3
        end do
     end do
  end do
  do i4 = 1, DLEN
     do i3 = 1, DLEN
        do i2 = 1, DLEN
           do i1 = 1, DLEN
              data4_out(i1, i2, i3, i4) = REALLY_BIG - i1 - i2 - i3 - i4
           end do
        end do
     end do
  end do
  do i5 = 1, DLEN
     do i4 = 1, DLEN
        do i3 = 1, DLEN
           do i2 = 1, DLEN
              do i1 = 1, DLEN
                 data5_out(i1, i2, i3, i4, i5) = REALLY_BIG - i1 - i2 - i3 - i4 - i5
              end do
           end do
        end do
     end do
  end do
  do i6 = 1, DLEN
     do i5 = 1, DLEN
        do i4 = 1, DLEN
           do i3 = 1, DLEN
              do i2 = 1, DLEN
                 do i1 = 1, DLEN
                    data6_out(i1, i2, i3, i4, i5, i6) = REALLY_BIG - i1 - i2 - i3 - i4 - i5 - i6
                 end do
              end do
           end do
        end do
     end do
  end do
  do i7 = 1, DLEN
     do i6 = 1, DLEN
        do i5 = 1, DLEN
           do i4 = 1, DLEN
              do i3 = 1, DLEN
                 do i2 = 1, DLEN
                    do i1 = 1, DLEN
                       data7_out(i1, i2, i3, i4, i5, i6, i7) = REALLY_BIG - i1 - i2 - i3 - i4 - i5 - i6 - i7
                    end do
                 end do
              end do
           end do
        end do
     end do
  end do
  
  ! Create the netCDF file. 
  call check(nf90_create(FILE_NAME, nf90_netcdf4, ncid))

  ! Define dimensions.
  call check(nf90_def_dim(ncid, "d1", DLEN, dimid1))
  call check(nf90_def_dim(ncid, "d2", DLEN, dimid2))
  call check(nf90_def_dim(ncid, "d3", DLEN, dimid3))
  call check(nf90_def_dim(ncid, "d4", DLEN, dimid4))
  call check(nf90_def_dim(ncid, "d5", DLEN, dimid5))
  call check(nf90_def_dim(ncid, "d6", DLEN, dimid6))
  call check(nf90_def_dim(ncid, "d7", DLEN, dimid7))

  ! Create some int64 variables, from 1 to 7D.
  dimids1(1) = dimid1
  call check(nf90_def_var(ncid, "v1", nf90_int64, dimids1, varid1))
  dimids2(1) = dimid1
  dimids2(2) = dimid2
  call check(nf90_def_var(ncid, "v2", nf90_int64, dimids2, varid2))
  dimids3(1) = dimid1
  dimids3(2) = dimid2
  dimids3(3) = dimid3
  call check(nf90_def_var(ncid, "v3", nf90_int64, dimids3, varid3))
  dimids4(1) = dimid1
  dimids4(2) = dimid2
  dimids4(3) = dimid3
  dimids4(4) = dimid4
  call check(nf90_def_var(ncid, "v4", nf90_int64, dimids4, varid4))
  dimids5(1) = dimid1
  dimids5(2) = dimid2
  dimids5(3) = dimid3
  dimids5(4) = dimid4
  dimids5(5) = dimid5
  call check(nf90_def_var(ncid, "v5", nf90_int64, dimids5, varid5))
  dimids6(1) = dimid1
  dimids6(2) = dimid2
  dimids6(3) = dimid3
  dimids6(4) = dimid4
  dimids6(5) = dimid5
  dimids6(6) = dimid6
  call check(nf90_def_var(ncid, "v6", nf90_int64, dimids6, varid6))
  dimids7(1) = dimid1
  dimids7(2) = dimid2
  dimids7(3) = dimid3
  dimids7(4) = dimid4
  dimids7(5) = dimid5
  dimids7(6) = dimid6
  dimids7(7) = dimid7
  call check(nf90_def_var(ncid, "v7", nf90_int64, dimids7, varid7))

  ! Write some large integers.
  call check(nf90_put_var(ncid, varid1, data1_out))
  call check(nf90_put_var(ncid, varid2, data2_out))
  call check(nf90_put_var(ncid, varid3, data3_out))
  call check(nf90_put_var(ncid, varid4, data4_out))
  call check(nf90_put_var(ncid, varid5, data5_out))
  call check(nf90_put_var(ncid, varid6, data6_out))
  call check(nf90_put_var(ncid, varid7, data7_out))

  ! Close the file. 
  call check(nf90_close(ncid))

  ! Reopen the netCDF file. 
  call check(nf90_open(FILE_NAME, 0, ncid))

  ! Read in the large numbers.
  call check(nf90_get_var(ncid, varid1, data1_in))
  call check(nf90_get_var(ncid, varid2, data2_in))
  call check(nf90_get_var(ncid, varid3, data3_in))
  call check(nf90_get_var(ncid, varid4, data4_in))
  call check(nf90_get_var(ncid, varid5, data5_in))
  call check(nf90_get_var(ncid, varid6, data6_in))
  call check(nf90_get_var(ncid, varid7, data7_in))

  ! Check the values for correctness.
  do i1 = 1, DLEN
     if (data1_in(i1) .ne. data1_out(i1)) stop 2
  end do
  do i2 = 1, DLEN
     do i1 = 1, DLEN
        if (data2_in(i1, i2) .ne. data2_out(i1, i2)) stop 2
     end do
  end do
  do i3 = 1, DLEN
     do i2 = 1, DLEN
        do i1 = 1, DLEN
           if (data3_in(i1, i2, i3) .ne. data3_out(i1, i2, i3)) stop 2
        end do
     end do
  end do
  do i4 = 1, DLEN
     do i3 = 1, DLEN
        do i2 = 1, DLEN
           do i1 = 1, DLEN
              if (data4_in(i1, i2, i3, i4) .ne. &
                   data4_out(i1, i2, i3, i4)) stop 2
           end do
        end do
     end do
  end do
  do i5 = 1, DLEN
     do i4 = 1, DLEN
        do i3 = 1, DLEN
           do i2 = 1, DLEN
              do i1 = 1, DLEN
                 if (data5_in(i1, i2, i3, i4, i5) .ne. &
                      data5_out(i1, i2, i3, i4, i5)) stop 2
              end do
           end do
        end do
     end do
  end do
  do i6 = 1, DLEN
     do i5 = 1, DLEN
        do i4 = 1, DLEN
           do i3 = 1, DLEN
              do i2 = 1, DLEN
                 do i1 = 1, DLEN
                    if (data6_in(i1, i2, i3, i4, i5, i6) .ne. &
                         data6_out(i1, i2, i3, i4, i5, i6)) stop 2
                 end do
              end do
           end do
        end do
     end do
  end do
  do i7 = 1, DLEN
     do i6 = 1, DLEN
        do i5 = 1, DLEN
           do i4 = 1, DLEN
              do i3 = 1, DLEN
                 do i2 = 1, DLEN
                    do i1 = 1, DLEN
                       if (data7_in(i1, i2, i3, i4, i5, i6, i7) .ne. &
                            data7_out(i1, i2, i3, i4, i5, i6, i7)) stop 2
                    end do
                 end do
              end do
           end do
        end do
     end do
  end do

  ! Close the file. 
  call check(nf90_close(ncid))
  
  print *,'*** SUCCESS!'

!     This subroutine handles errors by printing an error message and
!     exiting with a non-zero status.
contains
  subroutine check(status)
    integer, intent ( in) :: status
    
    if(status /= nf90_noerr) then 
      print *, trim(nf90_strerror(status))
      stop 2
    end if
  end subroutine check  

end program tst_types2

