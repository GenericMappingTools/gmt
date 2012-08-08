  character (len = 80), external :: nf_inq_libvers, nf_strerror
  ! Control routines 
  integer,              external :: nf_open, nf__open, nf_create, nf__create,     &
                                    nf_enddef, nf__enddef, nf_set_fill, nf_redef, &
                                    nf_sync, nf_abort, nf_close,                  &
                                    ! These are used only in undocumented functions
                                    nf_set_base_pe, nf_inq_base_pe,               &
                                    nf__create_mp, nf__open_mp, nf_delete,        &
                                    nf_inq_format
  ! File level inquiry 
  integer,              external :: nf_inq
  
  ! Dimension routines   nf_inq_dim
  integer,              external :: nf_def_dim, nf_inq_dimid, nf_rename_dim, nf_inq_dim

  ! Attribute routines
  integer,              external :: nf_copy_att, nf_rename_att, nf_del_att, & 
                                    nf_inq_att, nf_inq_attid, nf_inq_attname
  integer,              external :: nf_put_att_text, nf_get_att_text,                 &
                                    nf_put_att_int1, nf_put_att_int2, nf_put_att_int, &
                                    nf_get_att_int1, nf_get_att_int2, nf_get_att_int, &
                                    nf_put_att_real,   nf_get_att_real,               &
                                    nf_put_att_double, nf_get_att_double
                                    
  ! Variable routines
  integer,              external :: nf_def_var, nf_inq_varid, nf_inq_var, nf_rename_var
  integer,              external :: nf_put_var1_text, nf_get_var1_text,                   &
                                    nf_put_var1_int1, nf_put_var1_int2, nf_put_var1_int, &
                                    nf_get_var1_int1, nf_get_var1_int2, nf_get_var1_int, &
                                    nf_put_var1_real,   nf_get_var1_real,                 &
                                    nf_put_var1_double, nf_get_var1_double
  integer,              external :: nf_put_vars_text, nf_get_vars_text,                   &
                                    nf_put_vars_int1, nf_put_vars_int2, nf_put_vars_int, &
                                    nf_get_vars_int1, nf_get_vars_int2, nf_get_vars_int, &
                                    nf_put_vars_real,   nf_get_vars_real,                 &
                                    nf_put_vars_double, nf_get_vars_double 
  integer,              external :: nf_put_vara_text, nf_get_vara_text,                   &
                                    nf_put_vara_int1, nf_put_vara_int2, nf_put_vara_int, &
                                    nf_get_vara_int1, nf_get_vara_int2, nf_get_vara_int, &
                                    nf_put_vara_real,   nf_get_vara_real,                 &
                                    nf_put_vara_double, nf_get_vara_double 
  integer,              external :: nf_put_varm_text, nf_get_varm_text,                   &
                                    nf_put_varm_int1, nf_put_varm_int2, nf_put_varm_int, &
                                    nf_get_varm_int1, nf_get_varm_int2, nf_get_varm_int, &
                                    nf_put_varm_real,   nf_get_varm_real,                 &
                                    nf_put_varm_double, nf_get_varm_double
