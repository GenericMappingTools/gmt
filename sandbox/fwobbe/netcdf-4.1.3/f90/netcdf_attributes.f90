  !
  ! Attribute routines:
  !
  ! -------
  function nf90_copy_att(ncid_in, varid_in, name, ncid_out, varid_out)
    integer,             intent( in) :: ncid_in,  varid_in
    character (len = *), intent( in) :: name
    integer,             intent( in) :: ncid_out, varid_out
    integer                          :: nf90_copy_att

    nf90_copy_att = nf_copy_att(ncid_in, varid_in, name, ncid_out, varid_out)
  end function nf90_copy_att
  ! -------
  function nf90_rename_att(ncid, varid, curname, newname)
    integer,             intent( in) :: ncid,  varid
    character (len = *), intent( in) :: curname, newname
    integer                          :: nf90_rename_att

    nf90_rename_att = nf_rename_att(ncid, varid, curname, newname)
  end function nf90_rename_att
  ! -------
  function nf90_del_att(ncid, varid, name)
    integer,             intent( in) :: ncid, varid
    character (len = *), intent( in) :: name
    integer                          :: nf90_del_att

    nf90_del_att = nf_del_att(ncid, varid, name)
  end function nf90_del_att
  ! -------
  ! Attribute inquiry functions
  ! -------
  function nf90_inq_attname(ncid, varid, attnum, name)
    integer,             intent( in) :: ncid, varid, attnum
    character (len = *), intent(out) :: name
    integer                          :: nf90_inq_attname

    nf90_inq_attname = nf_inq_attname(ncid, varid, attnum, name)
  end function nf90_inq_attname
  ! -------
  function nf90_inquire_attribute(ncid, varid, name, xtype, len, attnum)
    integer,             intent( in)           :: ncid, varid
    character (len = *), intent( in)           :: name
    integer,             intent(out), optional :: xtype, len, attnum
    integer                                    :: nf90_inquire_attribute

    integer                          :: local_xtype, local_len

    ! Do we need to worry about not saving the state from this call?
    if(present(attnum)) &
      nf90_inquire_attribute = nf_inq_attid(ncid, varid, name, attnum)
    nf90_inquire_attribute   = nf_inq_att  (ncid, varid, name, local_xtype, local_len)
    if(present(xtype)) xtype = local_xtype
    if(present(len  )) len   = local_len
  end function nf90_inquire_attribute
  ! -------
  ! Put and get functions; these will get overloaded
  ! -------
  ! Text
  ! -------
  function nf90_put_att_text(ncid, varid, name, values)
    integer,                          intent( in) :: ncid, varid
    character(len = *),               intent( in) :: name
    character(len = *),               intent( in) :: values
    integer                                       :: nf90_put_att_text

    nf90_put_att_text = nf_put_att_text(ncid, varid, name, len_trim(values), trim(values))
  end function nf90_put_att_text
  ! -------
  function nf90_get_att_text(ncid, varid, name, values)
    integer,                          intent( in) :: ncid, varid
    character(len = *),               intent( in) :: name
    character(len = *),               intent(out) :: values
    integer                                       :: nf90_get_att_text

    values = ' '  !! make sure result will be blank padded
    nf90_get_att_text = nf_get_att_text(ncid, varid, name, values)
  end function nf90_get_att_text
  ! -------
  ! Integer attributes
  ! -------
  function nf90_put_att_OneByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind =  OneByteInt), dimension(:), intent( in) :: values
    integer                                                 :: nf90_put_att_OneByteInt

    nf90_put_att_OneByteInt = nf_put_att_int1(ncid, varid, name, nf90_int1, size(values), values)
  end function nf90_put_att_OneByteInt
  ! -------
  function nf90_put_att_one_OneByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind =  OneByteInt),               intent( in) :: values
    integer                                                 :: nf90_put_att_one_OneByteInt

    integer (kind =  OneByteInt), dimension(1)              :: valuesA
    valuesA(1) = values
    nf90_put_att_one_OneByteInt = nf_put_att_int1(ncid, varid, name, nf90_int1, 1, valuesA)
  end function nf90_put_att_one_OneByteInt
  ! -------
  function nf90_get_att_OneByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind =  OneByteInt), dimension(:), intent(out) :: values
    integer                                                 :: nf90_get_att_OneByteInt

    nf90_get_att_OneByteInt = nf_get_att_int1(ncid, varid, name, values)
  end function nf90_get_att_OneByteInt
  ! -------
  function nf90_get_att_one_OneByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind =  OneByteInt),               intent(out) :: values
    integer                                                 :: nf90_get_att_one_OneByteInt

    integer (kind =  OneByteInt), dimension(1)              :: valuesA
    nf90_get_att_one_OneByteInt = nf_get_att_int1(ncid, varid, name, valuesA)
    values = valuesA(1)
  end function nf90_get_att_one_OneByteInt
  ! -------
  function nf90_put_att_TwoByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind =  TwoByteInt), dimension(:), intent( in) :: values
    integer                                                 :: nf90_put_att_TwoByteInt

    nf90_put_att_TwoByteInt = nf_put_att_int2(ncid, varid, name, nf90_int2, size(values), values)
  end function nf90_put_att_TwoByteInt
  ! -------
  function nf90_put_att_one_TwoByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind =  TwoByteInt),               intent( in) :: values
    integer                                                 :: nf90_put_att_one_TwoByteInt

    integer (kind =  TwoByteInt), dimension(1)              :: valuesA
    valuesA(1) = values
    nf90_put_att_one_TwoByteInt = nf_put_att_int2(ncid, varid, name, nf90_int2, 1, valuesA)
  end function nf90_put_att_one_TwoByteInt
  ! -------
  function nf90_get_att_TwoByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind =  TwoByteInt), dimension(:), intent(out) :: values
    integer                                                 :: nf90_get_att_TwoByteInt

    nf90_get_att_TwoByteInt = nf_get_att_int2(ncid, varid, name, values)
  end function nf90_get_att_TwoByteInt
  ! -------
  function nf90_get_att_one_TwoByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind =  TwoByteInt),               intent(out) :: values
    integer                                                 :: nf90_get_att_one_TwoByteInt

    integer (kind =  TwoByteInt), dimension(1)              :: valuesA
    nf90_get_att_one_TwoByteInt = nf_get_att_int2(ncid, varid, name, valuesA)
    values = valuesA(1)
  end function nf90_get_att_one_TwoByteInt
  ! -------
  function nf90_put_att_FourByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind = FourByteInt), dimension(:), intent( in) :: values
    integer                                                 :: nf90_put_att_FourByteInt

    nf90_put_att_FourByteInt = nf_put_att_int(ncid, varid, name, nf90_int, size(values), int(values))
  end function nf90_put_att_FourByteInt
  ! -------
  function nf90_put_att_one_FourByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind = FourByteInt),               intent( in) :: values
    integer                                                 :: nf90_put_att_one_FourByteInt

    integer (kind = FourByteInt), dimension(1)              :: valuesA
    valuesA(1) = int(values)
    nf90_put_att_one_FourByteInt = nf_put_att_int(ncid, varid, name, nf90_int, 1, int(valuesA))
  end function nf90_put_att_one_FourByteInt
  ! -------
  function nf90_get_att_FourByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind = FourByteInt), dimension(:), intent(out) :: values
    integer                                                 :: nf90_get_att_FourByteInt
    
    integer, dimension(size(values)) :: defaultInteger

    nf90_get_att_FourByteInt = nf_get_att_int(ncid, varid, name, defaultInteger)
    values(:) = defaultInteger(:)
  end function nf90_get_att_FourByteInt
  ! -------
  function nf90_get_att_one_FourByteInt(ncid, varid, name, values)
    integer,                                    intent( in) :: ncid, varid
    character(len = *),                         intent( in) :: name
    integer (kind = FourByteInt),               intent(out) :: values
    integer                                                 :: nf90_get_att_one_FourByteInt

    integer, dimension(1) :: defaultInteger

    nf90_get_att_one_FourByteInt = nf_get_att_int(ncid, varid, name, defaultInteger)
    values = defaultInteger(1)
  end function nf90_get_att_one_FourByteInt
  ! -------
  function nf90_put_att_EightByteInt(ncid, varid, name, values)
    integer,                                     intent( in) :: ncid, varid
    character(len = *),                          intent( in) :: name
    integer (kind = EightByteInt), dimension(:), intent( in) :: values
    integer                                                  :: nf90_put_att_EightByteInt

    nf90_put_att_EightByteInt = nf_put_att_int(ncid, varid, name, nf90_int, size(values), int(values))
  end function nf90_put_att_EightByteInt
  ! -------
  function nf90_put_att_one_EightByteInt(ncid, varid, name, values)
    integer,                                     intent( in) :: ncid, varid
    character(len = *),                          intent( in) :: name
    integer (kind = EightByteInt),               intent( in) :: values
    integer                                                  :: nf90_put_att_one_EightByteInt

    integer, dimension(1) :: valuesA
    valuesA(1) = values
    nf90_put_att_one_EightByteInt = nf_put_att_int(ncid, varid, name, nf90_int, 1, valuesA)
  end function nf90_put_att_one_EightByteInt
  ! -------
  function nf90_get_att_EightByteInt(ncid, varid, name, values)
    integer,                                     intent( in) :: ncid, varid
    character(len = *),                          intent( in) :: name
    integer (kind = EightByteInt), dimension(:), intent(out) :: values
    integer                                                 :: nf90_get_att_EightByteInt
    
    integer, dimension(size(values)) :: defaultInteger

    nf90_get_att_EightByteInt = nf_get_att_int(ncid, varid, name, defaultInteger)
    values(:) = defaultInteger(:)
  end function nf90_get_att_EightByteInt
  ! -------
  function nf90_get_att_one_EightByteInt(ncid, varid, name, values)
    integer,                                     intent( in) :: ncid, varid
    character(len = *),                          intent( in) :: name
    integer (kind = EightByteInt),               intent(out) :: values
    integer                                                 :: nf90_get_att_one_EightByteInt

    integer, dimension(1) :: defaultInteger

    nf90_get_att_one_EightByteInt = nf_get_att_int(ncid, varid, name, defaultInteger)
    values = defaultInteger(1)
  end function nf90_get_att_one_EightByteInt
  ! -------
  ! Real attributes
  ! -------
  function nf90_put_att_FourByteReal(ncid, varid, name, values)
    integer,                                   intent( in) :: ncid, varid
    character(len = *),                        intent( in) :: name
    real (kind =  FourByteReal), dimension(:), intent( in) :: values
    integer                                                :: nf90_put_att_FourByteReal

    nf90_put_att_FourByteReal = nf_put_att_real(ncid, varid, name, nf90_real4, size(values), values)
  end function nf90_put_att_FourByteReal
  ! -------
  function nf90_put_att_one_FourByteReal(ncid, varid, name, values)
    integer,                                   intent( in) :: ncid, varid
    character(len = *),                        intent( in) :: name
    real (kind =  FourByteReal),               intent( in) :: values
    integer                                                :: nf90_put_att_one_FourByteReal

    real (kind =  FourByteReal), dimension(1) :: valuesA
    valuesA(1) = values
    nf90_put_att_one_FourByteReal = nf_put_att_real(ncid, varid, name, nf90_real4, 1, valuesA)
  end function nf90_put_att_one_FourByteReal
  ! -------
  function nf90_get_att_FourByteReal(ncid, varid, name, values)
    integer,                                   intent( in) :: ncid, varid
    character(len = *),                        intent( in) :: name
    real (kind =  FourByteReal), dimension(:), intent(out) :: values
    integer                                                :: nf90_get_att_FourByteReal

    nf90_get_att_FourByteReal = nf_get_att_real(ncid, varid, name, values)
  end function nf90_get_att_FourByteReal
  ! -------
  function nf90_get_att_one_FourByteReal(ncid, varid, name, values)
    integer,                                   intent( in) :: ncid, varid
    character(len = *),                        intent( in) :: name
    real (kind =  FourByteReal),               intent(out) :: values
    integer                                                :: nf90_get_att_one_FourByteReal

    real (kind =  FourByteReal), dimension(1) :: valuesA
    nf90_get_att_one_FourByteReal = nf_get_att_real(ncid, varid, name, valuesA)
    values = valuesA(1)
  end function nf90_get_att_one_FourByteReal
  ! -------
  function nf90_put_att_EightByteReal(ncid, varid, name, values)
    integer,                                   intent( in) :: ncid, varid
    character(len = *),                        intent( in) :: name
    real (kind = EightByteReal), dimension(:), intent( in) :: values
    integer                                                :: nf90_put_att_EightByteReal

    nf90_put_att_EightByteReal = nf_put_att_double(ncid, varid, name, nf90_real8, size(values), values)
  end function nf90_put_att_EightByteReal
  ! -------
  function nf90_put_att_one_EightByteReal(ncid, varid, name, values)
    integer,                                   intent( in) :: ncid, varid
    character(len = *),                        intent( in) :: name
    real (kind = EightByteReal),               intent( in) :: values
    integer                                                :: nf90_put_att_one_EightByteReal

    real (kind = EightByteReal), dimension(1) :: valuesA
    valuesA(1) = values
    nf90_put_att_one_EightByteReal = nf_put_att_double(ncid, varid, name, nf90_real8, 1, valuesA)
  end function nf90_put_att_one_EightByteReal
  ! -------
  function nf90_get_att_EightByteReal(ncid, varid, name, values)
    integer,                                   intent( in) :: ncid, varid
    character(len = *),                        intent( in) :: name
    real (kind = EightByteReal), dimension(:), intent(out) :: values
    integer                                                :: nf90_get_att_EightByteReal

    nf90_get_att_EightByteReal = nf_get_att_double(ncid, varid, name, values)
  end function nf90_get_att_EightByteReal
  ! -------
  function nf90_get_att_one_EightByteReal(ncid, varid, name, values)
    integer,                                   intent( in) :: ncid, varid
    character(len = *),                        intent( in) :: name
    real (kind = EightByteReal),               intent(out) :: values
    integer                                                :: nf90_get_att_one_EightByteReal

    real (kind = EightByteReal), dimension(1) :: valuesA
    nf90_get_att_one_EightByteReal = nf_get_att_double(ncid, varid, name, valuesA)
    values = valuesA(1)
  end function nf90_get_att_one_EightByteReal
  ! -------
