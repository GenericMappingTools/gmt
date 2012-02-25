C Copyright 1996-2006, UCAR/Unidata
C See COPYRIGHT file for copying and redistribution conditions.

C Test driver for netCDF-3 interface.  This program performs tests against
C the netCDF-3 specification for all user-level functions in an
C implementation of the netCDF library.

C $Id$

        subroutine test(name, func)
        implicit        none
        character*(*)   name
        external        func
#include "tests.inc"

        write(*, 1) name
1       format(' *** testing ', a, ' ... ')
        nfails = 0
        call func()
        nfailsTotal = nfailsTotal + nfails
        if ( nfails .ne. 0) then
            print *, ' '
            print *, '  ### ', nfails, ' FAILURES TESTING ', name, 
     +               '! ###'
        end if
        end

        program nf_test
        implicit        none

#include "tests.inc"

        integer         status

        external        test_nf_strerror
        external        test_nf_open
        external        test_nf_close
        external        test_nf_inq
        external        test_nf_inq_dimid
        external        test_nf_inq_dim
        external        test_nf_inq_dimlen
        external        test_nf_inq_dimname
        external        test_nf_inq_varid
        external        test_nf_inq_var
        external        test_nf_inq_natts
        external        test_nf_inq_ndims
        external        test_nf_inq_nvars
        external        test_nf_inq_unlimdim
        external        test_nf_inq_format
        external        test_nf_inq_vardimid
        external        test_nf_inq_varname
        external        test_nf_inq_varnatts
        external        test_nf_inq_varndims
        external        test_nf_inq_vartype
        external        test_nf_get_var1_text
#if defined(NF_INT1_T)
        external        test_nf_get_var1_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_get_var1_int2
#endif
        external        test_nf_get_var1_int
        external        test_nf_get_var1_real
        external        test_nf_get_var1_double
        external        test_nf_get_var_text
#if defined(NF_INT1_T)
        external        test_nf_get_var_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_get_var_int2
#endif
        external        test_nf_get_var_int
        external        test_nf_get_var_real
        external        test_nf_get_var_double
        external        test_nf_get_vara_text
#if defined(NF_INT1_T)
        external        test_nf_get_vara_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_get_vara_int2
#endif
        external        test_nf_get_vara_int
        external        test_nf_get_vara_real
        external        test_nf_get_vara_double
        external        test_nf_get_vars_text
#if defined(NF_INT1_T)
        external        test_nf_get_vars_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_get_vars_int2
#endif
        external        test_nf_get_vars_int
        external        test_nf_get_vars_real
        external        test_nf_get_vars_double
        external        test_nf_get_varm_text
#if defined(NF_INT1_T)
        external        test_nf_get_varm_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_get_varm_int2
#endif
        external        test_nf_get_varm_int
        external        test_nf_get_varm_real
        external        test_nf_get_varm_double
        external        test_nf_get_att_text
#if defined(NF_INT1_T)
        external        test_nf_get_att_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_get_att_int2
#endif
        external        test_nf_get_att_int
        external        test_nf_get_att_real
        external        test_nf_get_att_double
        external        test_nf_inq_att
        external        test_nf_inq_attname
        external        test_nf_inq_attid
        external        test_nf_inq_attlen
        external        test_nf_inq_atttype
        external        test_nf_create
        external        test_nf_redef
        external        test_nf_enddef
        external        test_nf_sync
        external        test_nf_abort
        external        test_nf_def_dim
        external        test_nf_rename_dim
        external        test_nf_def_var
        external        test_nf_put_var1_text
#if defined(NF_INT1_T)
        external        test_nf_put_var1_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_put_var1_int2
#endif
        external        test_nf_put_var1_int
        external        test_nf_put_var1_real
        external        test_nf_put_var1_double
        external        test_nf_put_var_text
#if defined(NF_INT1_T)
        external        test_nf_put_var_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_put_var_int2
#endif
        external        test_nf_put_var_int
        external        test_nf_put_var_real
        external        test_nf_put_var_double
        external        test_nf_put_vara_text
#if defined(NF_INT1_T)
        external        test_nf_put_vara_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_put_vara_int2
#endif
        external        test_nf_put_vara_int
        external        test_nf_put_vara_real
        external        test_nf_put_vara_double
        external        test_nf_put_vars_text
#if defined(NF_INT1_T)
        external        test_nf_put_vars_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_put_vars_int2
#endif
        external        test_nf_put_vars_int
        external        test_nf_put_vars_real
        external        test_nf_put_vars_double
        external        test_nf_put_varm_text
#if defined(NF_INT1_T)
        external        test_nf_put_varm_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_put_varm_int2
#endif
        external        test_nf_put_varm_int
        external        test_nf_put_varm_real
        external        test_nf_put_varm_double
        external        test_nf_rename_var
        external        test_nf_put_att_text
#if defined(NF_INT1_T)
        external        test_nf_put_att_int1
#endif
#if defined(NF_INT2_T)
        external        test_nf_put_att_int2
#endif
        external        test_nf_put_att_int
        external        test_nf_put_att_real
        external        test_nf_put_att_double
        external        test_nf_copy_att
        external        test_nf_rename_att
        external        test_nf_del_att
        external        test_nf_set_fill
        external        test_nf_set_default_format
        external        ignorefpe

        print *, ''
        print *, '*** Testing netCDF-3 Fortran 77 API.'
        call ignorefpe(1)

        testfile = 'test.nc'
        scratch = 'scratch.nc'

        nfailsTotal = 0
        progname = 'nf_test'
        max_nmpt = 8

        write(*, *) 'NetCDF version: ', nf_inq_libvers()

C       /* Initialize global variables defining test file */
        call init_gvars

        call write_file(testfile)
        if (nfailsTotal .gt. 0) call udexit(1)

C       /* delete any existing scratch netCDF file */
        status = nf_delete(scratch)

C       /* Test read-only functions, using pregenerated test-file */
        call test('nf_strerror', test_nf_strerror)
        call test('nf_open', test_nf_open)
        call test('nf_close', test_nf_close)
        call test('nf_inq', test_nf_inq)
        call test('nf_inq_dimid', test_nf_inq_dimid)
        call test('nf_inq_dim', test_nf_inq_dim)
        call test('nf_inq_dimlen', test_nf_inq_dimlen)
        call test('nf_inq_dimname', test_nf_inq_dimname)
        call test('nf_inq_varid', test_nf_inq_varid)
        call test('nf_inq_var', test_nf_inq_var)
        call test('nf_inq_natts', test_nf_inq_natts)
        call test('nf_inq_ndims', test_nf_inq_ndims)
        call test('nf_inq_nvars', test_nf_inq_nvars)
        call test('nf_inq_unlimdim', test_nf_inq_unlimdim)
        call test('nf_inq_format', test_nf_inq_format)
        call test('nf_inq_vardimid', test_nf_inq_vardimid)
        call test('nf_inq_varname', test_nf_inq_varname)
        call test('nf_inq_varnatts', test_nf_inq_varnatts)
        call test('nf_inq_varndims', test_nf_inq_varndims)
        call test('nf_inq_vartype', test_nf_inq_vartype)
        call test('nf_get_var1_text', test_nf_get_var1_text)
#if defined(NF_INT1_T)
        call test('nf_get_var1_int1', test_nf_get_var1_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_get_var1_int2', test_nf_get_var1_int2)
#endif
        call test('nf_get_var1_int', test_nf_get_var1_int)
        call test('nf_get_var1_real', test_nf_get_var1_real)
        call test('nf_get_var1_double', test_nf_get_var1_double)
        call test('nf_get_var_text', test_nf_get_var_text)
#if defined(NF_INT1_T)
        call test('nf_get_var_int1', test_nf_get_var_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_get_var_int2', test_nf_get_var_int2)
#endif
        call test('nf_get_var_int', test_nf_get_var_int)
        call test('nf_get_var_real', test_nf_get_var_real)
        call test('nf_get_var_double', test_nf_get_var_double)
        call test('nf_get_vara_text', test_nf_get_vara_text)
#if defined(NF_INT1_T)
        call test('nf_get_vara_int1', test_nf_get_vara_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_get_vara_int2', test_nf_get_vara_int2)
#endif
        call test('nf_get_vara_int', test_nf_get_vara_int)
        call test('nf_get_vara_real', test_nf_get_vara_real)
        call test('nf_get_vara_double', test_nf_get_vara_double)
        call test('nf_get_vars_text', test_nf_get_vars_text)
#if defined(NF_INT1_T)
        call test('nf_get_vars_int1', test_nf_get_vars_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_get_vars_int2', test_nf_get_vars_int2)
#endif
        call test('nf_get_vars_int', test_nf_get_vars_int)
        call test('nf_get_vars_real', test_nf_get_vars_real)
        call test('nf_get_vars_double', test_nf_get_vars_double)
        call test('nf_get_varm_text', test_nf_get_varm_text)
#if defined(NF_INT1_T)
        call test('nf_get_varm_int1', test_nf_get_varm_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_get_varm_int2', test_nf_get_varm_int2)
#endif
        call test('nf_get_varm_int', test_nf_get_varm_int)
        call test('nf_get_varm_real', test_nf_get_varm_real)
        call test('nf_get_varm_double', test_nf_get_varm_double)
        call test('nf_get_att_text', test_nf_get_att_text)
#if defined(NF_INT1_T)
        call test('nf_get_att_int1', test_nf_get_att_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_get_att_int2', test_nf_get_att_int2)
#endif
        call test('nf_get_att_int', test_nf_get_att_int)
        call test('nf_get_att_real', test_nf_get_att_real)
        call test('nf_get_att_double', test_nf_get_att_double)
        call test('nf_inq_att', test_nf_inq_att)
        call test('nf_inq_attname', test_nf_inq_attname)
        call test('nf_inq_attid', test_nf_inq_attid)
        call test('nf_inq_attlen', test_nf_inq_attlen)
        call test('nf_inq_atttype', test_nf_inq_atttype)

C           /* Test write functions */
        call test('nf_create', test_nf_create)
        call test('nf_redef', test_nf_redef)
        call test('nf_enddef', test_nf_enddef)
        call test('nf_sync', test_nf_sync)
        call test('nf_abort', test_nf_abort)
        call test('nf_def_dim', test_nf_def_dim)
        call test('nf_rename_dim', test_nf_rename_dim)
        call test('nf_def_var', test_nf_def_var)
        call test('nf_put_var1_text', test_nf_put_var1_text)
#if defined(NF_INT1_T)
        call test('nf_put_var1_int1', test_nf_put_var1_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_put_var1_int2', test_nf_put_var1_int2)
#endif
        call test('nf_put_var1_int', test_nf_put_var1_int)
        call test('nf_put_var1_real', test_nf_put_var1_real)
        call test('nf_put_var1_double', test_nf_put_var1_double)
        call test('nf_put_var_text', test_nf_put_var_text)
#if defined(NF_INT1_T)
        call test('nf_put_var_int1', test_nf_put_var_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_put_var_int2', test_nf_put_var_int2)
#endif
        call test('nf_put_var_int', test_nf_put_var_int)
        call test('nf_put_var_real', test_nf_put_var_real)
        call test('nf_put_var_double', test_nf_put_var_double)
        call test('nf_put_vara_text', test_nf_put_vara_text)
#if defined(NF_INT1_T)
        call test('nf_put_vara_int1', test_nf_put_vara_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_put_vara_int2', test_nf_put_vara_int2)
#endif
        call test('nf_put_vara_int', test_nf_put_vara_int)
        call test('nf_put_vara_real', test_nf_put_vara_real)
        call test('nf_put_vara_double', test_nf_put_vara_double)
        call test('nf_put_vars_text', test_nf_put_vars_text)
#if defined(NF_INT1_T)
        call test('nf_put_vars_int1', test_nf_put_vars_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_put_vars_int2', test_nf_put_vars_int2)
#endif
        call test('nf_put_vars_int', test_nf_put_vars_int)
        call test('nf_put_vars_real', test_nf_put_vars_real)
        call test('nf_put_vars_double', test_nf_put_vars_double)
        call test('nf_put_varm_text', test_nf_put_varm_text)
#if defined(NF_INT1_T)
        call test('nf_put_varm_int1', test_nf_put_varm_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_put_varm_int2', test_nf_put_varm_int2)
#endif
        call test('nf_put_varm_int', test_nf_put_varm_int)
        call test('nf_put_varm_real', test_nf_put_varm_real)
        call test('nf_put_varm_double', test_nf_put_varm_double)
        call test('nf_rename_var', test_nf_rename_var)
        call test('nf_put_att_text', test_nf_put_att_text)
#if defined(NF_INT1_T)
        call test('nf_put_att_int1', test_nf_put_att_int1)
#endif
#if defined(NF_INT2_T)
        call test('nf_put_att_int2', test_nf_put_att_int2)
#endif
        call test('nf_put_att_int', test_nf_put_att_int)
        call test('nf_put_att_real', test_nf_put_att_real)
        call test('nf_put_att_double', test_nf_put_att_double)
        call test('nf_copy_att', test_nf_copy_att)
        call test('nf_rename_att', test_nf_rename_att)
        call test('nf_del_att', test_nf_del_att)
        call test('nf_set_fill', test_nf_set_fill)
        call test('nf_set_default_format', 
     +       test_nf_set_default_format)

        print *,'Total number of failures: ', nfailsTotal
        if (nfailsTotal .ne. 0) stop 2
        print *,'*** SUCCESS!'        

        end

