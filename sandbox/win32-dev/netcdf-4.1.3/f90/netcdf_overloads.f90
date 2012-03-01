  ! Overloaded variable functions
  interface nf90_def_var
    module procedure nf90_def_var_Scalar, nf90_def_var_oneDim, nf90_def_var_ManyDims
  end interface ! nf90_def_var
  
  ! Overloaded attribute functions
  interface nf90_put_att
    module procedure nf90_put_att_text,                                      &
                     nf90_put_att_OneByteInt,     nf90_put_att_TwoByteInt,   &
                     nf90_put_att_FourByteInt,    nf90_put_att_EightByteInt, &
                     nf90_put_att_FourByteReal,   nf90_put_att_EightByteReal
    module procedure nf90_put_att_one_OneByteInt,   nf90_put_att_one_TwoByteInt,   &
                     nf90_put_att_one_FourByteInt,  nf90_put_att_one_EightByteInt, &
                     nf90_put_att_one_FourByteReal, nf90_put_att_one_EightByteReal
  end interface !nf90_put_att
  interface nf90_get_att
    module procedure nf90_get_att_text,                                      &
                     nf90_get_att_OneByteInt,     nf90_get_att_TwoByteInt,   &
                     nf90_get_att_FourByteInt,    nf90_get_att_EightByteInt, &
                     nf90_get_att_FourByteReal,   nf90_get_att_EightByteReal
    module procedure nf90_get_att_one_OneByteInt,   nf90_get_att_one_TwoByteInt,   &
                     nf90_get_att_one_FourByteInt,  nf90_get_att_one_EightByteInt, &
                     nf90_get_att_one_FourByteReal, nf90_get_att_one_EightByteReal
  end interface ! nf90_get_att

  ! Overloaded variable functions
  interface nf90_put_var
    module procedure nf90_put_var_text,                                   &
                     nf90_put_var_OneByteInt, nf90_put_var_TwoByteInt,    &
                     nf90_put_var_FourByteInt, nf90_put_var_EightByteInt, &
                     nf90_put_var_FourByteReal, nf90_put_var_EightByteReal
    module procedure nf90_put_var_1D_text,                                      &
                     nf90_put_var_1D_OneByteInt, nf90_put_var_1D_TwoByteInt,    &
                     nf90_put_var_1D_FourByteInt, nf90_put_var_1D_EightByteInt, &
                     nf90_put_var_1D_FourByteReal, nf90_put_var_1D_EightByteReal
    module procedure nf90_put_var_2D_text,                                       &
                     nf90_put_var_2D_OneByteInt, nf90_put_var_2D_TwoByteInt,     &
                     nf90_put_var_2D_FourByteInt, nf90_put_var_2D_EightByteInt,  &
                     nf90_put_var_2D_FourByteReal, nf90_put_var_2D_EightByteReal
    module procedure nf90_put_var_3D_text,                                       &
                     nf90_put_var_3D_OneByteInt, nf90_put_var_3D_TwoByteInt,     &
                     nf90_put_var_3D_FourByteInt, nf90_put_var_3D_EightByteInt,  &
                     nf90_put_var_3D_FourByteReal, nf90_put_var_3D_EightByteReal
    module procedure nf90_put_var_4D_text,                                       &
                     nf90_put_var_4D_OneByteInt, nf90_put_var_4D_TwoByteInt,     &
                     nf90_put_var_4D_FourByteInt, nf90_put_var_4D_EightByteInt,  &
                     nf90_put_var_4D_FourByteReal, nf90_put_var_4D_EightByteReal
    module procedure nf90_put_var_5D_text,                                       &
                     nf90_put_var_5D_OneByteInt, nf90_put_var_5D_TwoByteInt,     &
                     nf90_put_var_5D_FourByteInt, nf90_put_var_5D_EightByteInt,  &
                     nf90_put_var_5D_FourByteReal, nf90_put_var_5D_EightByteReal
    module procedure nf90_put_var_6D_text,                                       &
                     nf90_put_var_6D_OneByteInt, nf90_put_var_6D_TwoByteInt,     &
                     nf90_put_var_6D_FourByteInt, nf90_put_var_6D_EightByteInt,  &
                     nf90_put_var_6D_FourByteReal, nf90_put_var_6D_EightByteReal
    module procedure nf90_put_var_7D_text,                                       &
                     nf90_put_var_7D_OneByteInt, nf90_put_var_7D_TwoByteInt,     &
                     nf90_put_var_7D_FourByteInt, nf90_put_var_7D_EightByteInt,  &
                     nf90_put_var_7D_FourByteReal, nf90_put_var_7D_EightByteReal
  end interface ! nf90_put_var

  interface nf90_get_var
    module procedure nf90_get_var_text,                                   &
                     nf90_get_var_OneByteInt, nf90_get_var_TwoByteInt,    &
                     nf90_get_var_FourByteInt, nf90_get_var_EightByteInt, &
                     nf90_get_var_FourByteReal, nf90_get_var_EightByteReal
    module procedure nf90_get_var_1D_text,                                      &
                     nf90_get_var_1D_OneByteInt, nf90_get_var_1D_TwoByteInt,    &
                     nf90_get_var_1D_FourByteInt, nf90_get_var_1D_EightByteInt, &
                     nf90_get_var_1D_FourByteReal, nf90_get_var_1D_EightByteReal
    module procedure nf90_get_var_2D_text,                                      &
                     nf90_get_var_2D_OneByteInt, nf90_get_var_2D_TwoByteInt,    &
                     nf90_get_var_2D_FourByteInt, nf90_get_var_2D_EightByteInt, &
                     nf90_get_var_2D_FourByteReal, nf90_get_var_2D_EightByteReal
    module procedure nf90_get_var_3D_text,                                      &
                     nf90_get_var_3D_OneByteInt, nf90_get_var_3D_TwoByteInt,    &
                     nf90_get_var_3D_FourByteInt, nf90_get_var_3D_EightByteInt, &
                     nf90_get_var_3D_FourByteReal, nf90_get_var_3D_EightByteReal
    module procedure nf90_get_var_4D_text,                                      &
                     nf90_get_var_4D_OneByteInt, nf90_get_var_4D_TwoByteInt,    &
                     nf90_get_var_4D_FourByteInt, nf90_get_var_4D_EightByteInt, &
                     nf90_get_var_4D_FourByteReal, nf90_get_var_4D_EightByteReal
    module procedure nf90_get_var_5D_text,                                      &
                     nf90_get_var_5D_OneByteInt, nf90_get_var_5D_TwoByteInt,    &
                     nf90_get_var_5D_FourByteInt, nf90_get_var_5D_EightByteInt, &
                     nf90_get_var_5D_FourByteReal, nf90_get_var_5D_EightByteReal
    module procedure nf90_get_var_6D_text,                                      &
                     nf90_get_var_6D_OneByteInt, nf90_get_var_6D_TwoByteInt,    &
                     nf90_get_var_6D_FourByteInt, nf90_get_var_6D_EightByteInt, &
                     nf90_get_var_6D_FourByteReal, nf90_get_var_6D_EightByteReal
    module procedure nf90_get_var_7D_text,                                      &
                     nf90_get_var_7D_OneByteInt, nf90_get_var_7D_TwoByteInt,    &
                     nf90_get_var_7D_FourByteInt, nf90_get_var_7D_EightByteInt, &
                     nf90_get_var_7D_FourByteReal, nf90_get_var_7D_EightByteReal
  end interface ! nf90_get_var
