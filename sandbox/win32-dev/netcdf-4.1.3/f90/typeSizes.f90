! Description:
!   Provide named kind parameters for use in declarations of real and integer 
!    variables with specific byte sizes (i.e. one, two, four, and eight byte
!    integers; four and eight byte reals). The parameters can then be used
!    in (KIND = XX) modifiers in declarations.
!   A single function (byteSizesOK()) is provided to ensure that the selected 
!    kind parameters are correct.
!  
! Input Parameters:
!   None.
!
! Output Parameters:
!   Public parameters, fixed at compile time:
!     OneByteInt, TwoByteInt, FourByteInt, EightByteInt
!                                     FourByteReal, EightByteRadl
!
! References and Credits:
!   Written by
!    Robert Pincus
!    Cooperative Institue for Meteorological Satellite Studies
!    University of Wisconsin - Madison
!    1225 W. Dayton St. 
!    Madison, Wisconsin 53706
!    Robert.Pincus@ssec.wisc.edu
!
! Design Notes:
!   Fortran 90 doesn't allow one to check the number of bytes in a real variable;
!     we check only that four byte and eight byte reals have different kind parameters. 
!
module typeSizes
  implicit none
  public
  integer, parameter ::   OneByteInt = selected_int_kind(2), &
                          TwoByteInt = selected_int_kind(4), &
                         FourByteInt = selected_int_kind(9), &
                        EightByteInt = selected_int_kind(18)

  integer, parameter ::                                          &
                        FourByteReal = selected_real_kind(P =  6, R =  37), &
                       EightByteReal = selected_real_kind(P = 13, R = 307)
contains
  logical function byteSizesOK()
  ! Users may call this function once to ensure that the kind parameters 
  !   the module defines are available with the current compiler. 
  ! We can't ensure that the two REAL kinds are actually four and 
  !   eight bytes long, but we can ensure that they are distinct. 
  ! Early Fortran 90 compilers would sometimes report incorrect results for 
  !   the bit_size intrinsic, but I haven't seen this in a long time. 

    ! Local variables
    integer (kind =  OneByteInt) :: One
    integer (kind =  TwoByteInt) :: Two
    integer (kind = FourByteInt) :: Four

    if (bit_size( One) == 8  .and. bit_size( Two) == 16 .and.  &
        bit_size(Four) == 32 .and.                             &
        FourByteReal > 0 .and. EightByteReal > 0 .and. &
        FourByteReal /= EightByteReal) then
      byteSizesOK = .true.
    else
      byteSizesOK = .false.
    end if
  end function byteSizesOK
end module typeSizes
