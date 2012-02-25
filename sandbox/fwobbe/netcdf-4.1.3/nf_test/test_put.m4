divert(-1)

dnl This is m4 source.
dnl Process using m4 to produce FORTRAN language file.

changequote([,]) dnl

undefine([index])dnl

dnl Macros

dnl Upcase(str)
dnl
define([Upcase],[dnl
translit($1, abcdefghijklmnopqrstuvwxyz, ABCDEFGHIJKLMNOPQRSTUVWXYZ)])

dnl NFT_ITYPE(type)
dnl
define([NFT_ITYPE], [NFT_[]Upcase($1)])

dnl ARITH(itype, value)
dnl
define([ARITH], [ifelse($1, text, ichar($2), $2)])

dnl  DATATYPE(funf_suffix)
dnl
define([DATATYPE], [dnl
ifelse($1, text, character,
ifelse($1, int1, NF_INT1_T,
ifelse($1, int2, NF_INT2_T,
ifelse($1, int, integer,
ifelse($1, real, real,
ifelse($1, double, doubleprecision)[]dnl
)[]dnl
)[]dnl
)[]dnl
)[]dnl
)[]dnl
])

dnl  MAKE_ARITH(funf_suffix, var)
dnl
define([MAKE_ARITH], [dnl
ifelse($1, text, ichar($2), $2)[]dnl
])

dnl  MAKE_DOUBLE(funf_suffix, var)
dnl
define([MAKE_DOUBLE], [dnl
ifelse($1, text, dble(ichar($2)), dble($2))[]dnl
])

dnl  MAKE_TYPE(funf_suffix, var)
dnl
define([MAKE_TYPE], [dnl
ifelse($1, text, char(int($2)), $2)[]dnl
])

dnl HASH(TYPE)
dnl
define([HASH],
[dnl
C
C ensure hash value within range for internal TYPE
C
        function hash_$1(type, rank, index, itype)
        implicit        none
#include "tests.inc"
        integer type
        integer rank
        integer index(1)
        integer itype
        doubleprecision minimum
        doubleprecision maximum

        minimum = internal_min(itype)
        maximum = internal_max(itype)

        hash_$1 = max(minimum, min(maximum, hash4( type, rank,
     +      index, itype)))
        end
])dnl


dnl CHECK_VARS(TYPE)
dnl
define([CHECK_VARS],dnl
[dnl
C
C check all vars in file which are (text/numeric) compatible with TYPE
C
        subroutine check_vars_$1(filename)
        implicit        none
#include "tests.inc"
        character*(*)   filename
        integer  ncid          !/* netCDF id */
        integer index(MAX_RANK)
        integer  err           !/* status */
        integer  d
        integer  i
        integer  j
        DATATYPE($1)    value
        integer datatype
        integer ndims
        integer dimids(MAX_RANK)
        integer ngatts
        doubleprecision expect
        character*(NF_MAX_NAME) name
        integer length
        logical canConvert      !/* Both text or both numeric */
        integer nok             !/* count of valid comparisons */
        doubleprecision val

        nok = 0

        err = nf_open(filename, NF_NOWRITE, ncid)
        if (err .ne. 0)
     +      call errore('nf_open: ', err)

        do 1, i = 1, NVARS
            canConvert = (var_type(i) .eq. NF_CHAR) .eqv.
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
            if (canConvert)  then
                err = nf_inq_var(ncid, i, name, datatype, ndims, dimids,
     +                           ngatts)
                if (err .ne. 0)
     +              call errore('nf_inq_var: ', err)
                if (name .ne. var_name(i))
     +              call error('Unexpected var_name')
                if (datatype .ne. var_type(i))
     +              call error('Unexpected type')
                if (ndims .ne. var_rank(i))
     +              call error('Unexpected rank')
                do 2, j = 1, ndims
                    err = nf_inq_dim(ncid, dimids(j), name, length)
                    if (err .ne. 0)
     +                  call errore('nf_inq_dim: ', err)
                    if (length .ne. var_shape(j,i))
     +                  call error('Unexpected shape')
2               continue
                do 3, j = 1, var_nels(i)
                    err = index2indexes(j, var_rank(i), var_shape(1,i), 
     +                                  index)
                    if (err .ne. 0)
     +                  call error('error in index2indexes()')
                    expect = hash4( var_type(i), var_rank(i), index, 
     +                             NFT_ITYPE($1))
                    err = nf_get_var1_$1(ncid, i, index, value)
                    if (inRange3(expect,datatype,NFT_ITYPE($1)))  then
                        if (in_internal_range(NFT_ITYPE($1), 
     +                                        expect)) then
                            if (err .ne. 0)  then
                                call errore('nf_get_var1_$1: ', err)
                            else
                                val = MAKE_ARITH($1,value)
                                if (.not.equal(
     +                              val,
     +                              expect,var_type(i),
     +                              NFT_ITYPE($1)))  then
                                    call error(
     +                          'Var value read not that expected')
                                    if (verbose)  then
                                        call error(' ')
                                        call errori('varid: %d', i)
                                        call errorc('var_name: ', 
     +                                          var_name(i))
                                        call error('index:')
                                        do 4, d = 1, var_rank(i)
                                            call errori(' ', index(d))
4                                       continue
                                        call errord('expect: ', expect)
                                        call errord('got: ',  val)
                                    end if
                                else
                                    nok = nok + 1
                                end if
                            end if
                        end if
                    end if
3               continue
            end if
1       continue
        err = nf_close (ncid)
        if (err .ne. 0)
     +      call errore('nf_close: ', err)
        call print_nok(nok)
        end
])dnl


dnl CHECK_ATTS(TYPE)         numeric only
dnl
define([CHECK_ATTS],dnl
[dnl
C/* 
C *  check all attributes in file which are (text/numeric) compatible with TYPE
C *  ignore any attributes containing values outside range of TYPE
C */
        subroutine check_atts_$1(ncid)
        implicit        none
#include "tests.inc"
        integer ncid
        integer  err           !/* status */
        integer  i
        integer  j
        integer  k
        integer ndx(1)
        DATATYPE($1)    value(MAX_NELS)
        integer datatype
        doubleprecision expect(MAX_NELS)
        integer length
        integer nInExtRange     !/* number values within external range */
        integer nInIntRange     !/* number values within internal range */
        logical canConvert      !/* Both text or both numeric */
        integer nok             !/* count of valid comparisons */
        doubleprecision val

        nok = 0

        do 1, i = 0, NVARS
            do 2, j = 1, NATTS(i)
                canConvert = (ATT_TYPE(j,i) .eq. NF_CHAR) .eqv.
     +                       (NFT_ITYPE($1) .eq. NFT_TEXT)
                if (canConvert) then
                    err = nf_inq_att(ncid, i, ATT_NAME(j,i), datatype, 
     +                               length)
                    if (err .ne. 0)
     +                  call errore('nf_inq_att: ', err)
                    if (datatype .ne. ATT_TYPE(j,i))
     +                  call error('nf_inq_att: unexpected type')
                    if (length .ne. ATT_LEN(j,i))
     +                  call error('nf_inq_att: unexpected length')
                    if (.not.(length .le. MAX_NELS))
     +                  stop 2
                    nInIntRange = 0
                    nInExtRange = 0
                    do 4, k = 1, length
                        ndx(1) = k
                        expect(k) = hash4( datatype, -1, ndx, 
     +                                    NFT_ITYPE($1))
                        if (inRange3(expect(k), datatype, 
     +                               NFT_ITYPE($1))) then
                            nInExtRange = nInExtRange + 1
                            if (in_internal_range(NFT_ITYPE($1),
     +                                            expect(k)))
     +                          nInIntRange = nInIntRange + 1
                        end if
4                   continue
                    err = nf_get_att_$1(ncid, i, 
     +                                  ATT_NAME(j,i), value)
                    if (nInExtRange .eq. length .and. 
     +                  nInIntRange .eq. length) then
                        if (err .ne. 0)
     +                      call error(nf_strerror(err))
                    else
                        if (err .ne. 0 .and. err .ne. NF_ERANGE)
     +                      call errore('OK or Range error: ', err)
                    end if
                    do 3, k = 1, length
                        if (inRange3(expect(k),datatype,NFT_ITYPE($1))
     +                          .and. 
     +                          in_internal_range(NFT_ITYPE($1), 
     +                                            expect(k))) then
                            val = MAKE_ARITH($1,value(k))
                            if (.not.equal(
     +                          val,
     +                          expect(k),datatype,
     +                          NFT_ITYPE($1))) then
                                call error(
     +                              'att. value read not that expected')
                                if (verbose) then
                                    call error(' ')
                                    call errori('varid: ', i)
                                    call errorc('att_name: ', 
     +                                  ATT_NAME(j,i))
                                    call errori('element number: ', k)
                                    call errord('expect: ', expect(k))
                                    call errord('got: ',  val)
                                end if
                            else
                                nok = nok + 1
                            end if
                        end if
3                   continue
                end if                                               
2           continue
1       continue

        call print_nok(nok)
        end
])dnl


dnl TEST_NF_PUT_VAR1(TYPE)
dnl
define([TEST_NF_PUT_VAR1],dnl
[dnl
        subroutine test_nf_put_var1_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer i
        integer j
        integer err
        integer index(MAX_RANK)
        logical canConvert      !/* Both text or both numeric */
        DATATYPE($1)    value
        doubleprecision val

        value = MAKE_TYPE($1, 5)!/* any value would do - only for error cases */

        err = nf_create(scratch, NF_CLOBBER, ncid)
        if (err .ne. 0) then
            call errore('nf_create: ', err)
            return
        end if
        call def_dims(ncid)
        call def_vars(ncid)
        err = nf_enddef(ncid)
        if (err .ne. 0)
     +      call errore('nf_enddef: ', err)

        do 1, i = 1, NVARS
            canConvert = (var_type(i) .eq. NF_CHAR) .eqv.
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
            do 2, j = 1, var_rank(i)
                index(j) = 1
2           continue
            err = nf_put_var1_$1(BAD_ID, i, index, value)
            if (err .ne. NF_EBADID) 
     +          call errore('bad ncid: ', err)
            err = nf_put_var1_$1(ncid, BAD_VARID,
     +                           index, value)
            if (err .ne. NF_ENOTVAR) 
     +          call errore('bad var id: ', err)
            do 3, j = 1, var_rank(i)
                if (var_dimid(j,i) .gt. 1) then         !/* skip record dim */
                    index(j) = var_shape(j,i) + 1
                    err = nf_put_var1_$1(ncid, i,
     +                                   index, value)
                    if (.not. canConvert) then
                        if (err .ne. NF_ECHAR)
     +                      call errore('conversion: ', err)
                        else
                            if (err .ne. NF_EINVALCOORDS)
     +                          call errore('bad index: ', err)
                        endif
                    index(j) = 0
                end if
3           continue
            do 4, j = 1, var_nels(i)
                err = index2indexes(j, var_rank(i), var_shape(1,i), 
     +                              index)
                if (err .ne. 0) 
     +              call error('error in index2indexes 1')
                value = MAKE_TYPE($1, hash_$1(var_type(i),var_rank(i),
     +                            index, NFT_ITYPE($1)))
                err = nf_put_var1_$1(ncid, i, index, value)
                if (canConvert) then
                    val = ARITH($1, value)
                    if (inRange3(val, var_type(i), NFT_ITYPE($1))) then
                        if (err .ne. 0)
     +                      call error(nf_strerror(err))
                    else
                        if (err .ne. NF_ERANGE)
     +                      call errore('Range error: ', err)
                    end if
                else
                    if (err .ne. NF_ECHAR)
     +                  call errore('wrong type: ', err)
                end if
4           continue
1       continue

        err = nf_close(ncid)
        if (err .ne. 0) 
     +      call errore('nf_close: ', err)

        call check_vars_$1(scratch)

        err = nf_delete(scratch)
        if (err .ne. 0)
     +      call errorc('delete of scratch file failed: ', 
     +                  scratch)
        end
])dnl


dnl TEST_NF_PUT_VAR(TYPE)
dnl
define([TEST_NF_PUT_VAR],dnl
[dnl
        subroutine test_nf_put_var_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer vid
        integer i
        integer j
        integer err
        integer nels
        integer index(MAX_RANK)
        logical canConvert      !/* Both text or both numeric */
        logical allInExtRange   !/* All values within external range?*/
        DATATYPE($1)    value(MAX_NELS)
        doubleprecision val

        err = nf_create(scratch, NF_CLOBBER, ncid)
        if (err .ne. 0) then
            call errore('nf_create: ', err)
            return
        end if
        call def_dims(ncid)
        call def_vars(ncid)
        err = nf_enddef(ncid)
        if (err .ne. 0)
     +      call errore('nf_enddef: ', err)

        do 1, i = 1, NVARS
            canConvert = (var_type(i) .eq. NF_CHAR) .eqv.
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
            err = nf_put_var_$1(BAD_ID, i, value)
            if (err .ne. NF_EBADID) 
     +          call errore('bad ncid: ', err)
            err = nf_put_var_$1(ncid, BAD_VARID, value)
            if (err .ne. NF_ENOTVAR) 
     +          call errore('bad var id: ', err)
            nels = 1
            do 3, j = 1, var_rank(i)
                nels = nels * var_shape(j,i)
3           continue
            allInExtRange = .true.
            do 4, j = 1, var_nels(i)
                err = index2indexes(j, var_rank(i), var_shape(1,i), 
     +                              index)
                if (err .ne. 0) 
     +              call error('error in index2indexes 1')
                value(j) = MAKE_TYPE($1, hash_$1(var_type(i), 
     +              var_rank(i),
     +              index, NFT_ITYPE($1)))
                val = ARITH($1, value(j))
                allInExtRange = allInExtRange .and.
     +              inRange3(val, var_type(i), NFT_ITYPE($1))
4           continue
            err = nf_put_var_$1(ncid, i, value)
            if (canConvert) then
                if (allInExtRange) then
                    if (err .ne. 0)
     +                  call error(nf_strerror(err))
                else
                    if (err .ne. NF_ERANGE .and.
     +                      var_dimid(var_rank(i),i) .ne. RECDIM)
     +                  call errore('Range error: ', err)
                endif
            else
                if (err .ne. NF_ECHAR)
     +              call errore('wrong type: ', err)
            endif
1       continue

C       The preceeding has written nothing for record variables, now try
C       again with more than 0 records.

C       Write record number NRECS to force writing of preceding records.
C       Assumes variable cr is char vector with UNLIMITED dimension.

        err = nf_inq_varid(ncid, "cr", vid)
        if (err .ne. 0)
     +      call errore('nf_inq_varid: ', err)
        index(1) = NRECS
        err = nf_put_var1_text(ncid, vid, index, 'x')
        if (err .ne. 0)
     +      call errore('nf_put_var1_text: ', err)

        do 5 i = 1, NVARS
C           Only test record variables here
            if (var_rank(i) .ge. 1 .and.
     +          var_dimid(var_rank(i),i) .eq. RECDIM) then
                canConvert = (var_type(i) .eq. NF_CHAR) .eqv.
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
                if (var_rank(i) .gt. MAX_RANK)
     +              stop 2
                if (var_nels(i) .gt. MAX_NELS)
     +              stop 2
                err = nf_put_var_$1(BAD_ID, i, value)

                nels = 1
                do 6 j = 1, var_rank(i)
                    nels = nels * var_shape(j,i)
6               continue
                allInExtRange = .true.
                do 7, j = 1, nels
                    err = index2indexes(j, var_rank(i), var_shape(1,i), 
     +                              index)
                    if (err .ne. 0) 
     +                  call error('error in index2indexes()')
                    value(j) = MAKE_TYPE($1, hash_$1(var_type(i), 
     +                  var_rank(i),
     +                  index, NFT_ITYPE($1)))
                    val = ARITH($1, value(j))
                    allInExtRange = allInExtRange .and.
     +                  inRange3(val, var_type(i), NFT_ITYPE($1))
7               continue
                err = nf_put_var_$1(ncid, i, value)
                if (canConvert) then
                    if (allInExtRange) then
                        if (err .ne. 0)
     +                      call error(nf_strerror(err))
                    else
                        if (err .ne. NF_ERANGE)
     +                      call errore('range error: ', err)
                    endif
                else
                    if (nels .gt. 0 .and. err .ne. NF_ECHAR)
     +                  call errore('wrong type: ', err)
                endif
            endif
5       continue

        err = nf_close(ncid)
        if (err .ne. 0) 
     +      call errore('nf_close: ', err)

        call check_vars_$1(scratch)

        err = nf_delete(scratch)
        if (err .ne. 0)
     +      call errorc('delete of scratch file failed: ', 
     +                  scratch)
        end
])dnl


dnl TEST_NF_PUT_VARA(TYPE)
dnl
define([TEST_NF_PUT_VARA],dnl
[dnl
        subroutine test_nf_put_vara_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer i
        integer j
        integer k
        integer d
        integer err
        integer nslabs
        integer nels
        integer start(MAX_RANK)
        integer edge(MAX_RANK)
        integer mid(MAX_RANK)
        integer index(MAX_RANK)
        logical canConvert      !/* Both text or both numeric */
        logical allInExtRange   !/* all values within external range? */
        DATATYPE($1)    value(MAX_NELS)
        doubleprecision val
        integer udshift

        err = nf_create(scratch, NF_CLOBBER, ncid)
        if (err .ne. 0) then
            call errore('nf_create: ', err)
            return
        end if
        call def_dims(ncid)
        call def_vars(ncid)
        err = nf_enddef(ncid)
        if (err .ne. 0)
     +      call errore('nf_enddef: ', err)

        do 1, i = 1, NVARS
            canConvert = (var_type(i) .eq. NF_CHAR) .eqv.
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
            if (.not.(var_rank(i) .le. MAX_RANK))
     +          stop 2
            if (.not.(var_nels(i) .le. MAX_NELS))
     +          stop 2
            do 2, j = 1, var_rank(i)
                start(j) = 1
                edge(j) = 1
2           continue
            err = nf_put_vara_$1(BAD_ID, i, start,
     +                  edge, value)
            if (err .ne. NF_EBADID) 
     +          call errore('bad ncid: ', err)
            err = nf_put_vara_$1(ncid, BAD_VARID,
     +                  start, edge, value)
            if (err .ne. NF_ENOTVAR) 
     +          call errore('bad var id: ', err)
            do 3, j = 1, var_rank(i)
                if (var_dimid(j,i) .ne. RECDIM) then    !/* skip record dim */
                    start(j) = var_shape(j,i) + 1
                    err = nf_put_vara_$1(ncid, i, start, 
     +                                   edge, value)
                    if (.not. canConvert) then
                        if (err .ne. NF_ECHAR)
     +                      call errore('conversion: ', err)
                    else
                        if (err .ne. NF_EINVALCOORDS)
     +                      call errore('bad start: ', err)
                    endif
                    start(j) = 1
                    edge(j) = var_shape(j,i) + 1
                    err = nf_put_vara_$1(ncid, i, start, 
     +                                   edge, value)
                    if (.not. canConvert) then
                        if (err .ne. NF_ECHAR)
     +                      call errore('conversion: ', err)
                    else
                        if (err .ne. NF_EEDGE)
     +                      call errore('bad edge: ', err)
                    endif
                    edge(j) = 1
                end if
3           continue

C       /* Check correct error returned even when nothing to put */
        do 20, j = 1, var_rank(i)
              edge(j) = 0
20      continue
        err = nf_put_vara_$1(BAD_ID, i, start,
     +          edge, value)
        if (err .ne. NF_EBADID) 
     +      call errore('bad ncid: ', err)
        err = nf_put_vara_$1(ncid, BAD_VARID,
     +          start, edge, value)
        if (err .ne. NF_ENOTVAR) 
     +      call errore('bad var id: ', err)
        do 21, j = 1, var_rank(i)
            if (var_dimid(j,i) .gt. 1) then     ! skip record dim
                start(j) = var_shape(j,i) + 2
                err = nf_put_vara_$1(ncid, i, start,
     +                  edge, value)
                if (.not. canConvert) then
                    if (err .ne. NF_ECHAR)
     +                  call errore('conversion: ', err)
                else
                    if (err .ne. NF_EINVALCOORDS)
     +                  call errore('bad start: ', err)
                endif
                start(j) = 1
            endif
21      continue
        err = nf_put_vara_$1(ncid, i, start, edge, value)
        if (canConvert) then
            if (err .ne. 0) 
     +          call error(nf_strerror(err))
        else
            if (err .ne. NF_ECHAR)
     +          call errore('wrong type: ', err)
        endif
        do 22, j = 1, var_rank(i)
              edge(j) = 1
22      continue


                !/* Choose a random point dividing each dim into 2 parts */
                !/* Put 2^rank (nslabs) slabs so defined */
            nslabs = 1
            do 4, j = 1, var_rank(i)
                mid(j) = roll( var_shape(j,i) )
                nslabs = nslabs * 2
4           continue
                !/* bits of k determine whether to put lower or upper part of dim */
            do 5, k = 1, nslabs
                nels = 1
                do 6, j = 1, var_rank(i)
                    if (mod(udshift(k-1, -(j-1)), 2) .eq. 1) then
                        start(j) = 1
                        edge(j) = mid(j)
                    else
                        start(j) = 1 + mid(j)
                        edge(j) = var_shape(j,i) - mid(j)
                    end if
                    nels = nels * edge(j)
6               continue
                allInExtRange = .true.
                do 7, j = 1, nels
                    err = index2indexes(j, var_rank(i), edge, index)
                    if (err .ne. 0) 
     +                  call error('error in index2indexes 1')
                    do 8, d = 1, var_rank(i)
                        index(d) = index(d) + start(d) - 1
8                   continue
                    value(j)= MAKE_TYPE($1, hash_$1(var_type(i), 
     +                                  var_rank(i), index, 
     +                                  NFT_ITYPE($1)))
                    val = ARITH($1, value(j))
                    allInExtRange = allInExtRange .and.
     +                  inRange3(val, var_type(i), NFT_ITYPE($1))
7               continue
                err = nf_put_vara_$1(ncid, i, start,
     +                  edge, value)
                if (canConvert) then
                    if (allInExtRange) then
                        if (err .ne. 0) 
     +                      call error(nf_strerror(err))
                    else
                        if (err .ne. NF_ERANGE)
     +                      call errore('range error: ', err)
                    end if
                else
                    if (nels .gt. 0 .and. err .ne. NF_ECHAR)
     +                  call errore('wrong type: ', err)
                end if
5           continue
1       continue

        err = nf_close(ncid)
        if (err .ne. 0) 
     +      call errore('nf_close: ', err)

        call check_vars_$1(scratch)

        err = nf_delete(scratch)
        if (err .ne. 0)
     +      call errorc('delete of scratch file failed: ', 
     +          scratch)
        end
])dnl


dnl TEST_NF_PUT_VARS(TYPE)
dnl
define([TEST_NF_PUT_VARS],dnl
[dnl
        subroutine test_nf_put_vars_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer d
        integer i
        integer j
        integer k
        integer m
        integer err
        integer nels
        integer nslabs
        integer nstarts        !/* number of different starts */
        integer start(MAX_RANK)
        integer edge(MAX_RANK)
        integer index(MAX_RANK)
        integer index2(MAX_RANK)
        integer mid(MAX_RANK)
        integer count(MAX_RANK)
        integer sstride(MAX_RANK)
        integer stride(MAX_RANK)
        logical canConvert      !/* Both text or both numeric */
        logical allInExtRange   !/* all values within external range? */
        DATATYPE($1)    value(MAX_NELS)
        doubleprecision val
        integer udshift

        err = nf_create(scratch, NF_CLOBBER, ncid)
        if (err .ne. 0) then
            call errore('nf_create: ', err)
            return
        end if
        call def_dims(ncid)
        call def_vars(ncid)
        err = nf_enddef(ncid)
        if (err .ne. 0)
     +      call errore('nf_enddef: ', err)

        do 1, i = 1, NVARS
            canConvert = (var_type(i) .eq. NF_CHAR) .eqv.
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
            if (.not.(var_rank(i) .le. MAX_RANK))
     +          stop 2
            if (.not.(var_nels(i) .le. MAX_NELS))
     +          stop 2
            do 2, j = 1, var_rank(i)
                start(j) = 1
                edge(j) = 1
                stride(j) = 1
2           continue
            err = nf_put_vars_$1(BAD_ID, i, start,
     +                  edge, stride, value)
            if (err .ne. NF_EBADID) 
     +          call errore('bad ncid: ', err)
            err = nf_put_vars_$1(ncid, BAD_VARID, start,
     +                           edge, stride, 
     +                           value)
            if (err .ne. NF_ENOTVAR) 
     +          call errore('bad var id: ', err)
            do 3, j = 1, var_rank(i)
                if (var_dimid(j,i) .ne. RECDIM) then    ! skip record dim
                    start(j) = var_shape(j,i) + 2
                    err = nf_put_vars_$1(ncid, i, start,
     +                                   edge, stride, 
     +                                   value)
                    if (.not. canConvert) then
                        if (err .ne. NF_ECHAR)
     +                      call errore('conversion: ', err)
                    else
                        if (err .ne. NF_EINVALCOORDS)
     +                          call errore('bad start: ', err)
                    endif
                    start(j) = 1
                    edge(j) = var_shape(j,i) + 1
                    err = nf_put_vars_$1(ncid, i, start,
     +                                   edge, stride, 
     +                                   value)
                    if (.not. canConvert) then
                        if (err .ne. NF_ECHAR)
     +                      call errore('conversion: ', err)
                    else
                        if (err .ne. NF_EEDGE)
     +                      call errore('bad edge: ', err)
                    endif
                    edge(j) = 1
                    stride(j) = 0
                    err = nf_put_vars_$1(ncid, i, start,
     +                                   edge, stride, 
     +                                   value)
                    if (.not. canConvert) then
                        if (err .ne. NF_ECHAR)
     +                      call errore('conversion: ', err)
                    else
                        if (err .ne. NF_ESTRIDE)
     +                      call errore('bad stride: ', err)
                    endif
                    stride(j) = 1
                end if
3           continue
                !/* Choose a random point dividing each dim into 2 parts */
                !/* Put 2^rank (nslabs) slabs so defined */
            nslabs = 1
            do 4, j = 1, var_rank(i)
                mid(j) = roll( var_shape(j,i) )
                nslabs = nslabs * 2
4           continue
                !/* bits of k determine whether to put lower or upper part of dim */
                !/* choose random stride from 1 to edge */
            do 5, k = 1, nslabs
                nstarts = 1
                do 6, j = 1, var_rank(i)
                    if (mod(udshift(k-1, -(j-1)), 2) .eq. 1) then
                        start(j) = 1
                        edge(j) = mid(j)
                    else
                        start(j) = 1 + mid(j)
                        edge(j) = var_shape(j,i) - mid(j)
                    end if
                    if (edge(j) .gt. 0) then
                        stride(j) = 1+roll(edge(j))
                    else
                        stride(j) = 1
                    end if
                    sstride(j) = stride(j)
                    nstarts = nstarts * stride(j)
6               continue
                do 7, m = 1, nstarts
                    err = index2indexes(m, var_rank(i), sstride, index)
                    if (err .ne. 0)
     +                  call error('error in index2indexes')
                    nels = 1
                    do 8, j = 1, var_rank(i)
                        count(j) = 1 + (edge(j) - index(j)) / stride(j)
                        nels = nels * count(j)
                        index(j) = index(j) + start(j) - 1
8                   continue
                        !/* Random choice of forward or backward */
C/* TODO
C                   if ( roll(2) ) {
C                       for (j = 1 j .lt. var_rank(i) j++) {
C                           index(j) += (count(j) - 1) * stride(j)
C                           stride(j) = -stride(j)
C                       }
C                   }
C*/
                    allInExtRange = .true.
                    do 9, j = 1, nels
                        err = index2indexes(j, var_rank(i), count, 
     +                                      index2)
                        if (err .ne. 0)
     +                      call error('error in index2indexes')
                        do 10, d = 1, var_rank(i)
                            index2(d) = index(d) + 
     +                                  (index2(d)-1) * stride(d)
10                      continue
                        value(j) = MAKE_TYPE($1, hash_$1(var_type(i), 
     +                     var_rank(i), 
     +                     index2, NFT_ITYPE($1)))
                        val = ARITH($1, value(j))
                        allInExtRange = allInExtRange .and.
     +                      inRange3(val, var_type(i), 
     +                               NFT_ITYPE($1))
9                   continue
                    err = nf_put_vars_$1(ncid, i, index,
     +                                   count, stride,
     +                                   value)
                    if (canConvert) then
                        if (allInExtRange) then
                            if (err .ne. 0) 
     +                          call error(nf_strerror(err))
                        else
                            if (err .ne. NF_ERANGE)
     +                          call errore('range error: ', err)
                        end if
                    else
                        if (nels .gt. 0 .and. err .ne. NF_ECHAR)
     +                      call errore('wrong type: ', err)
                    end if
7               continue
5           continue
1       continue

        err = nf_close(ncid)
        if (err .ne. 0) 
     +      call errore('nf_close: ', err)

        call check_vars_$1(scratch)

        err = nf_delete(scratch)
        if (err .ne. 0)
     +      call errorc('delete of scratch file failed:', 
     +          scratch)
        end
])dnl


dnl TEST_NF_PUT_VARM(TYPE)
dnl
define([TEST_NF_PUT_VARM],dnl
[dnl
        subroutine test_nf_put_varm_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer d
        integer i
        integer j
        integer k
        integer m
        integer err
        integer nels
        integer nslabs
        integer nstarts        !/* number of different starts */
        integer start(MAX_RANK)
        integer edge(MAX_RANK)
        integer index(MAX_RANK)
        integer index2(MAX_RANK)
        integer mid(MAX_RANK)
        integer count(MAX_RANK)
        integer sstride(MAX_RANK)
        integer stride(MAX_RANK)
        integer imap(MAX_RANK)
        logical canConvert      !/* Both text or both numeric */
        logical allInExtRange   !/* all values within external range? */
        DATATYPE($1) value(MAX_NELS)
        doubleprecision val
        integer udshift

        err = nf_create(scratch, NF_CLOBBER, ncid)
        if (err .ne. 0) then
            call errore('nf_create: ', err)
            return
        end if
        call def_dims(ncid)
        call def_vars(ncid)
        err = nf_enddef(ncid)
        if (err .ne. 0)
     +      call errore('nf_enddef: ', err)

        do 1, i = 1, NVARS
            canConvert = (var_type(i) .eq. NF_CHAR) .eqv.
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
            if (.not.(var_rank(i) .le. MAX_RANK))
     +          stop 2
            if (.not.(var_nels(i) .le. MAX_NELS))
     +          stop 2
            do 2, j = 1, var_rank(i)
                start(j) = 1
                edge(j) = 1
                stride(j) = 1
                imap(j) = 1
2           continue
            err = nf_put_varm_$1(BAD_ID, i, start,
     +                           edge, stride, imap, 
     +                           value)
            if (err .ne. NF_EBADID) 
     +          call errore('bad ncid: ', err)
            err = nf_put_varm_$1(ncid, BAD_VARID, start,
     +                           edge, stride, 
     +                           imap, value)
            if (err .ne. NF_ENOTVAR) 
     +          call errore('bad var id: ', err)
            do 3, j = 1, var_rank(i)
                if (var_dimid(j,i) .ne. RECDIM) then    !/* skip record dim */
                    start(j) = var_shape(j,i) + 2
                    err = nf_put_varm_$1(ncid, i, start,
     +                                   edge, stride, 
     +                                   imap, value)
                    if (.not. canConvert) then
                        if (err .ne. NF_ECHAR)
     +                      call errore('conversion: ', err)
                    else
                        if (err .ne. NF_EINVALCOORDS)
     +                      call errore('bad start: ', err)
                    endif
                    start(j) = 1
                    edge(j) = var_shape(j,i) + 1
                    err = nf_put_varm_$1(ncid, i, start,
     +                                   edge, stride, 
     +                                   imap, value)
                    if (.not. canConvert) then
                        if (err .ne. NF_ECHAR)
     +                      call errore('conversion: ', err)
                    else
                        if (err .ne. NF_EEDGE)
     +                      call errore('bad edge: ', err)
                    endif
                    edge(j) = 1
                    stride(j) = 0
                    err = nf_put_varm_$1(ncid, i, start,
     +                                   edge, stride, 
     +                                   imap, value)
                    if (.not. canConvert) then
                        if (err .ne. NF_ECHAR)
     +                      call errore('conversion: ', err)
                    else
                        if (err .ne. NF_ESTRIDE)
     +                      call errore('bad stride: ', err)
                    endif
                    stride(j) = 1
                end if
3           continue
                !/* Choose a random point dividing each dim into 2 parts */
                !/* Put 2^rank (nslabs) slabs so defined */
            nslabs = 1
            do 4, j = 1, var_rank(i)
                mid(j) = roll( var_shape(j,i) )
                nslabs = nslabs * 2
4           continue
                !/* bits of k determine whether to put lower or upper part of dim */
                !/* choose random stride from 1 to edge */
            do 5, k = 1, nslabs
                nstarts = 1
                do 6, j = 1, var_rank(i)
                    if (mod(udshift(k-1, -(j-1)), 2) .eq. 1) then
                        start(j) = 1
                        edge(j) = mid(j)
                    else
                        start(j) = 1 + mid(j)
                        edge(j) = var_shape(j,i) - mid(j)
                    end if
                    if (edge(j) .gt. 0) then
                        stride(j) = 1+roll(edge(j))
                    else
                        stride(j) = 1
                    end if
                    sstride(j) = stride(j)
                    nstarts = nstarts * stride(j)
6               continue
                do 7, m = 1, nstarts
                    err = index2indexes(m, var_rank(i), sstride, index)
                    if (err .ne. 0)
     +                  call error('error in index2indexes')
                    nels = 1
                    do 8, j = 1, var_rank(i)
                        count(j) = 1 + (edge(j) - index(j)) / stride(j)
                        nels = nels * count(j)
                        index(j) = index(j) + start(j) - 1
8                   continue
                        !/* Random choice of forward or backward */
C/* TODO
C                   if ( roll(2) ) then
C                       do 9, j = 1, var_rank(i)
C                           index(j) = index(j) + 
C     +                         (count(j) - 1) * stride(j)
C                           stride(j) = -stride(j)
C9                      continue
C                   end if
C*/
                    if (var_rank(i) .gt. 0) then
                        imap(1) = 1
                        do 10, j = 2, var_rank(i)
                            imap(j) = imap(j-1) * count(j-1)
10                      continue
                    end if
                    allInExtRange = .true.
                    do 11 j = 1, nels
                        err = index2indexes(j, var_rank(i), count, 
     +                                      index2)
                        if (err .ne. 0)
     +                      call error('error in index2indexes')
                        do 12, d = 1, var_rank(i)
                            index2(d) = index(d) + 
     +                          (index2(d)-1) * stride(d)
12                      continue
                        value(j) = MAKE_TYPE($1, hash_$1(var_type(i),
     +                                       var_rank(i), 
     +                                       index2, NFT_ITYPE($1)))
                        val = ARITH($1, value(j))
                        allInExtRange = allInExtRange .and.
     +                      inRange3(val, var_type(i), 
     +                               NFT_ITYPE($1))
11                  continue
                    err = nf_put_varm_$1(ncid,i,index,count,
     +                                   stride,imap,
     +                                   value)
                    if (canConvert) then
                        if (allInExtRange) then
                            if (err .ne. 0)
     +                          call error(nf_strerror(err))
                        else
                            if (err .ne. NF_ERANGE)
     +                          call errore('range error: ', err)
                        end if
                    else
                        if (nels .gt. 0 .and. err .ne. NF_ECHAR)
     +                      call errore('wrong type: ', err)
                    end if
7               continue
5           continue
1       continue

        err = nf_close(ncid)
        if (err .ne. 0) 
     +      call errore('nf_close: ', err)

        call check_vars_$1(scratch)

        err = nf_delete(scratch)
        if (err .ne. 0)
     +      call errorc('delete of scratch file failed:', 
     +          scratch)
        end
])dnl


dnl TEST_NF_PUT_ATT(TYPE)         numeric only
dnl
define([TEST_NF_PUT_ATT],dnl
[dnl
        subroutine test_nf_put_att_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer i
        integer j
        integer k
        integer ndx(1)
        integer err
        DATATYPE($1) value(MAX_NELS)
        logical allInExtRange  !/* all values within external range? */
        doubleprecision val

        err = nf_create(scratch, NF_NOCLOBBER, ncid)
        if (err .ne. 0) then
            call errore('nf_create: ', err)
            return
        end if
        call def_dims(ncid)
        call def_vars(ncid)

        do 1, i = 0, NVARS
            do 2, j = 1, NATTS(i)
                if (.not.(ATT_TYPE(j,i) .eq. NF_CHAR)) then
                    if (.not.((ATT_LEN(j,i) .le. MAX_NELS)))
     +                  stop 2
                    err = nf_put_att_$1(BAD_ID, i,
     +                                  ATT_NAME(j,i), 
     +                                  ATT_TYPE(j,i), 
     +                                  ATT_LEN(j,i), value)
                    if (err .ne. NF_EBADID)
     +                  call errore('bad ncid: ', err)
                    err = nf_put_att_$1(ncid, BAD_VARID,
     +                  ATT_NAME(j,i), 
     +                  ATT_TYPE(j,i), ATT_LEN(j,i), value)
                    if (err .ne. NF_ENOTVAR)
     +                  call errore('bad var id: ', err)
                    err = nf_put_att_$1(ncid, i,
     +                  ATT_NAME(j,i), BAD_TYPE, 
     +                  ATT_LEN(j,i), value)
                    if (err .ne. NF_EBADTYPE)
     +                  call errore('bad type: ', err)
                    allInExtRange = .true.
                    do 3, k = 1, ATT_LEN(j,i)
                        ndx(1) = k
                        value(k) = hash_$1(ATT_TYPE(j,i), -1, ndx, 
     +                                     NFT_ITYPE($1))
                        val = ARITH($1, value(k))
                        allInExtRange = allInExtRange .and.
     +                      inRange3(val, ATT_TYPE(j,i), 
     +                               NFT_ITYPE($1))
3                   continue
                    err = nf_put_att_$1(ncid, i, ATT_NAME(j,i), 
     +                                  ATT_TYPE(j,i), ATT_LEN(j,i), 
     +                                  value)
                    if (allInExtRange) then
                        if (err .ne. 0)
     +                      call error(nf_strerror(err))
                    else
                        if (err .ne. NF_ERANGE)
     +                      call errore('range error: ', err)
                    end if
                end if
2           continue
1       continue

        call check_atts_$1(ncid)
        err = nf_close(ncid)
        if (err .ne. 0)
     +      call errore('nf_close: ', err)

        err = nf_delete(scratch)
        if (err .ne. 0)
     +      call errorc('delete of scratch file failed:', 
     +          scratch)
        end
])dnl

divert(0)dnl
dnl If you see this line, you can ignore the next one.
C Do not edit this file. It is produced from the corresponding .m4 source */

C********************************************************************
C   Copyright 1996, UCAR/Unidata
C   See netcdf/COPYRIGHT file for copying and redistribution conditions.
C   $Id$
C********************************************************************

HASH(text)
#ifdef NF_INT1_T
HASH(int1)
#endif
#ifdef NF_INT2_T
HASH(int2)
#endif
HASH(int)
HASH(real)
HASH(double)

CHECK_VARS(text)
#ifdef NF_INT1_T
CHECK_VARS(int1)
#endif
#ifdef NF_INT2_T
CHECK_VARS(int2)
#endif
CHECK_VARS(int)
CHECK_VARS(real)
CHECK_VARS(double)

CHECK_ATTS(text)
#ifdef NF_INT1_T
CHECK_ATTS(int1)
#endif
#ifdef NF_INT2_T
CHECK_ATTS(int2)
#endif
CHECK_ATTS(int)
CHECK_ATTS(real)
CHECK_ATTS(double)

TEST_NF_PUT_VAR1(text)
#ifdef NF_INT1_T
TEST_NF_PUT_VAR1(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_PUT_VAR1(int2)
#endif
TEST_NF_PUT_VAR1(int)
TEST_NF_PUT_VAR1(real)
TEST_NF_PUT_VAR1(double)

TEST_NF_PUT_VAR(text)
#ifdef NF_INT1_T
TEST_NF_PUT_VAR(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_PUT_VAR(int2)
#endif
TEST_NF_PUT_VAR(int)
TEST_NF_PUT_VAR(real)
TEST_NF_PUT_VAR(double)

TEST_NF_PUT_VARA(text)
#ifdef NF_INT1_T
TEST_NF_PUT_VARA(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_PUT_VARA(int2)
#endif
TEST_NF_PUT_VARA(int)
TEST_NF_PUT_VARA(real)
TEST_NF_PUT_VARA(double)

TEST_NF_PUT_VARS(text)
#ifdef NF_INT1_T
TEST_NF_PUT_VARS(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_PUT_VARS(int2)
#endif
TEST_NF_PUT_VARS(int)
TEST_NF_PUT_VARS(real)
TEST_NF_PUT_VARS(double)

TEST_NF_PUT_VARM(text)
#ifdef NF_INT1_T
TEST_NF_PUT_VARM(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_PUT_VARM(int2)
#endif
TEST_NF_PUT_VARM(int)
TEST_NF_PUT_VARM(real)
TEST_NF_PUT_VARM(double)

        subroutine test_nf_put_att_text()
        implicit        none
#include "tests.inc"
        integer ncid
        integer i
        integer j
        integer k
        integer err
        character       value(MAX_NELS)

        err = nf_create(scratch, NF_NOCLOBBER, ncid)
        if (err .ne. 0) then
            call errore('NF_create: ', err)
            return
        end if
        call def_dims(ncid)
        call def_vars(ncid)

        do 1, i = 0, NVARS
            do 2, j = 1, NATTS(i)
                if (ATT_TYPE(j,i) .eq. NF_CHAR) then
                    if (.not.(ATT_LEN(j,i) .le. MAX_NELS))
     +                  stop 2
                    err = nf_put_att_text(BAD_ID, i,
     +                  ATT_NAME(j,i), ATT_LEN(j,i), value)
                    if (err .ne. NF_EBADID)
     +                  call errore('bad ncid: ', err)
                    err = nf_put_att_text(ncid, BAD_VARID, 
     +                                    ATT_NAME(j,i), 
     +                                    ATT_LEN(j,i), value)
                    if (err .ne. NF_ENOTVAR)
     +                  call errore('bad var id: ', err)
                    do 3, k = 1, ATT_LEN(j,i)
                        value(k) = char(int(hash(ATT_TYPE(j,i), -1, k)))
3                   continue
                    err = nf_put_att_text(ncid, i, ATT_NAME(j,i), 
     +                  ATT_LEN(j,i), value)
                    if (err .ne. 0)
     +                  call error(NF_strerror(err))
                end if
2           continue
1       continue

        call check_atts_text(ncid)
        err = NF_close(ncid)
        if (err .ne. 0)
     +      call errore('NF_close: ', err)

        err = nf_delete(scratch)
        if (err .ne. 0)
     +      call errorc('delete of scratch file failed:', 
     +          scratch)
        end

#ifdef NF_INT1_T
TEST_NF_PUT_ATT(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_PUT_ATT(int2)
#endif
TEST_NF_PUT_ATT(int)
TEST_NF_PUT_ATT(real)
TEST_NF_PUT_ATT(double)
