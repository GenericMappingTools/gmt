divert(-1)

dnl This is m4 source.
dnl Process using m4 to produce FORTRAN language file.

changequote([,])

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

dnl TEST_NF_GET_VAR1(TYPE)
dnl
define([TEST_NF_GET_VAR1],[dnl
        subroutine test_nf_get_var1_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer i
        integer j
        integer err
        integer nok      
        integer index(MAX_RANK)
        doubleprecision expect
        logical canConvert     
        DATATYPE($1)    value
        doubleprecision val

        nok = 0

        err = nf_open(testfile, NF_NOWRITE, ncid)
        if (err .ne. 0)
     +      call errore('nf_open: ', err)
        do 1, i = 1, NVARS
            canConvert = (var_type(i) .eq. NF_CHAR) .eqv.
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
            do 2, j = 1, var_rank(i)
                index(j) = 1
2           continue
            err = nf_get_var1_$1(BAD_ID, i, index, value)
            if (err .ne. NF_EBADID)
     +          call errore('bad ncid: ', err)
            err = nf_get_var1_$1(ncid, BAD_VARID,
     +                  index, value)
            if (err .ne. NF_ENOTVAR)
     +          call errore('bad var id: ', err)
            do 3, j = 1, var_rank(i)
                index(j) = var_shape(j,i) + 1
                err = nf_get_var1_$1(ncid, i, index, value)
                if (.not. canConvert) then
                    if (err .ne. NF_ECHAR)
     +                  call errore('conversion: ', err)
                else
                    if (err .ne. NF_EINVALCOORDS)
     +                  call errore('bad index: ', err)
                endif
                index(j) = 1
3           continue
            do 4, j = 1, var_nels(i)
                err = index2indexes(j, var_rank(i), var_shape(1,i), 
     +                              index)
                if (err .ne. 0)
     +              call error('error in index2indexes 1')
                expect = hash4( var_type(i), var_rank(i), index, 
     +                          NFT_ITYPE($1) )
                err = nf_get_var1_$1(ncid, i, index,
     +                          value)
                if (canConvert) then
                    if (inRange3(expect,var_type(i), 
     +                           NFT_ITYPE($1))) then
                        if (in_internal_range(NFT_ITYPE($1),
     +                                        expect)) then
                            if (err .ne. 0) then
                                call errore('nf_get_var: ', err)
                            else
                                val = ARITH($1, value)
                                if (.not. equal(val, expect, 
     +                                          var_type(i), 
     +                                          NFT_ITYPE($1))) then
                                    call errord('unexpected: ', val)
                                else
                                    nok = nok + 1
                                end if
                            end if
                        else
                            if (err .ne. NF_ERANGE)
     +                          call errore('Range error: ', err)
                        end if
                    else
                        if (err .ne. 0  .and. err .ne. NF_ERANGE)
     +                      call errore('OK or Range error: ', err)
                    end if
                else
                    if (err .ne. NF_ECHAR)
     +                  call errore('wrong type: ', err)
                end if
4           continue
1       continue
        err = nf_close(ncid)
        if (err .ne. 0)
     +      call errore('nf_close: ',  err)
        call print_nok(nok)
        end
])

dnl TEST_NF_GET_VAR(TYPE)
dnl
define([TEST_NF_GET_VAR],[dnl
        subroutine test_nf_get_var_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer i
        integer j
        integer err
        logical allInExtRange   
        logical allInIntRange   
        integer nels
        integer nok      
        integer index(MAX_RANK)
        doubleprecision expect(MAX_NELS)
        logical canConvert     
        DATATYPE($1)    value(MAX_NELS)
        doubleprecision val

        nok = 0

        err = nf_open(testfile, NF_NOWRITE, ncid)
        if (err .ne. 0)
     +      call errore('nf_open: ', err)
        do 1, i = 1, NVARS
            canConvert = (var_type(i) .eq. NF_CHAR) .eqv.
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
            err = nf_get_var_$1(BAD_ID, i, value)
            if (err .ne. NF_EBADID)
     +          call errore('bad ncid: ', err)
            err = nf_get_var_$1(ncid, BAD_VARID, value)
            if (err .ne. NF_ENOTVAR)
     +          call errore('bad var id: ', err)
            nels = 1
            do 3, j = 1, var_rank(i)
                nels = nels * var_shape(j,i)
3           continue
            allInExtRange = .true.
            allInIntRange = .true.
            do 4, j = 1, var_nels(i)
                err = index2indexes(j, var_rank(i), var_shape(1,i), 
     +                              index)
                if (err .ne. 0)
     +              call error('error in index2indexes 1')
                expect(j) = hash4( var_type(i), var_rank(i), index, 
     +                          NFT_ITYPE($1) )
                if (inRange3(expect(j),var_type(i), NFT_ITYPE($1))) then
                    allInIntRange = allInIntRange .and.
     +                  in_internal_range(NFT_ITYPE($1), expect(j))
                else
                    allInExtRange = .false.
                end if
4           continue
            err = nf_get_var_$1(ncid, i, value)
            if (canConvert) then
                if (allInExtRange) then
                    if (allInIntRange) then
                        if (err .ne. 0) 
     +                      call errore('nf_get_var: ', err)
                    else
                        if (err .ne. NF_ERANGE)
     +                      call errore('Range error: ', err)
                    endif
                else
                    if (err .ne. 0  .and. err .ne. NF_ERANGE)
     +                  call errore('Range error: ', err)
                endif
                do 5, j = 1, var_nels(i)
                    if (inRange3(expect(j),var_type(i),
     +                           NFT_ITYPE($1)) .and.
     +                  in_internal_range(NFT_ITYPE($1),
     +                                          expect(j))) then
                        val = ARITH($1, value(j))
                        if (.not. equal(val, expect(j), 
     +                                  var_type(i), 
     +                                  NFT_ITYPE($1))) then
                            call errord('unexpected: ', val)
                        else
                            nok = nok + 1
                        end if
                    endif
5               continue
            else
                if (err .ne. NF_ECHAR)
     +                  call errore('wrong type: ', err)
            end if
1       continue
        err = nf_close(ncid)
        if (err .ne. 0)
     +      call errore('nf_close: ',  err)
        call print_nok(nok)
        end
])


dnl TEST_NF_GET_VARA(TYPE)
dnl
define([TEST_NF_GET_VARA],[dnl
        subroutine test_nf_get_vara_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer d
        integer i
        integer j
        integer k
        integer err
        logical allInExtRange   
        logical allInIntRange   
        integer nels
        integer nslabs
        integer nok      
        integer start(MAX_RANK)
        integer edge(MAX_RANK)
        integer index(MAX_RANK)
        integer mid(MAX_RANK)
        logical canConvert     
        DATATYPE($1)    value(MAX_NELS)
        doubleprecision expect(MAX_NELS)
        doubleprecision val
        integer udshift

        nok = 0

        err = nf_open(testfile, NF_NOWRITE, ncid)
        if (err .ne. 0)
     +      call errore('nf_open: ', err)
        do 1, i = 1, NVARS
            canConvert = (var_type(i) .eq. NF_CHAR) .eqv. 
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
            if (.not.(var_rank(i) .le. MAX_RANK)) stop 2
            if (.not.(var_nels(i) .le. MAX_NELS)) stop 2
            do 2, j = 1, var_rank(i)
                start(j) = 1
                edge(j) = 1
2           continue
            err = nf_get_vara_$1(BAD_ID, i, start,
     +                  edge, value)
            if (err .ne. NF_EBADID)
     +          call errore('bad ncid: ', err)
            err = nf_get_vara_$1(ncid, BAD_VARID, start, 
     +                           edge, value)
            if (err .ne. NF_ENOTVAR)
     +          call errore('bad var id: ', err)
            do 3, j = 1, var_rank(i)
                start(j) = var_shape(j,i) + 1
                err = nf_get_vara_$1(ncid, i, start,
     +                               edge, value)
                if (canConvert .and. err .ne. NF_EINVALCOORDS)
     +              call errore('bad index: ', err)
                start(j) = 1
                edge(j) = var_shape(j,i) + 1
                err = nf_get_vara_$1(ncid, i, start,
     +                               edge, value)
                if (canConvert .and. err .ne. NF_EEDGE)
     +              call errore('bad edge: ', err)
                edge(j) = 1
3           continue

C           /* Check non-scalars for correct error returned even when */
C           /* there is nothing to get (edge(j).eq.0) */
            if (var_rank(i) .gt. 0) then
                do 10, j = 1, var_rank(i)
                    edge(j) = 0
10              continue
                err = nf_get_vara_$1(BAD_ID, i, start,
     +                  edge, value)
                if (err .ne. NF_EBADID) 
     +              call errore('bad ncid: ', err)
                err = nf_get_vara_$1(ncid, BAD_VARID,
     +                  start, edge, value)
                if (err .ne. NF_ENOTVAR) 
     +              call errore('bad var id: ', err)
                do 11, j = 1, var_rank(i)
                    if (var_dimid(j,i) .gt. 1) then     !/* skip record dim */
                        start(j) = var_shape(j,i) + 1
                        err = nf_get_vara_$1(ncid, i,
     +                          start, edge, value)
                        if (canConvert .and. err .ne. NF_EINVALCOORDS)
     +                      call errore('bad start: ', err)
                        start(j) = 1
                    endif
11              continue
                err = nf_get_vara_$1(ncid, i, start,
     +                          edge, value)
                if (canConvert) then
                    if (err .ne. 0) 
     +                  call error(nf_strerror(err))
                else
                    if (err .ne. NF_ECHAR)
     +                  call errore('wrong type: ', err)
                endif
                do 12, j = 1, var_rank(i)
                    edge(j) = 1
12              continue
            endif

C           Choose a random point dividing each dim into 2 parts
C           get 2^rank (nslabs) slabs so defined
            nslabs = 1
            do 4, j = 1, var_rank(i)
                mid(j) = roll( var_shape(j,i) )
                nslabs = nslabs * 2
4           continue
C           bits of k determine whether to get lower or upper part of dim 
            do 5, k = 1, nslabs
                nels = 1
                do 6, j = 1, var_rank(i)
                    if (mod(udshift((k-1), -(j-1)), 2) .eq. 1) then
                        start(j) = 1
                        edge(j) = mid(j)
                    else
                        start(j) = 1 + mid(j)
                        edge(j) = var_shape(j,i) - mid(j)
                    end if
                    nels = nels * edge(j)
6               continue
                allInIntRange = .true.
                allInExtRange = .true.
                do 7, j = 1, nels
                    err = index2indexes(j, var_rank(i), edge, index)
                    if (err .ne. 0)
     +                  call error('error in index2indexes 1')
                    do 8, d = 1, var_rank(i)
                        index(d) = index(d) + start(d) - 1
8                   continue
                    expect(j) = hash4(var_type(i), var_rank(i), index, 
     +                                NFT_ITYPE($1))
                    if (inRange3(expect(j),var_type(i), 
     +                           NFT_ITYPE($1))) then
                        allInIntRange = 
     +                      allInIntRange .and.
     +                      in_internal_range(NFT_ITYPE($1), expect(j))
                    else
                        allInExtRange = .false.
                    end if
7               continue
                err = nf_get_vara_$1(ncid, i, start,
     +                          edge, value)
                if (canConvert) then
                    if (allInExtRange) then
                        if (allInIntRange) then
                            if (err .ne. 0)
     +                          call errore('nf_get_vara_$1:', err)
                        else
                            if (err .ne. NF_ERANGE)
     +                          call errore('Range error: ', err)
                        end if
                    else
                        if (err .ne. 0 .and. err .ne. NF_ERANGE)
     +                      call errore('OK or Range error: ', err)
                    end if
                    do 9, j = 1, nels
                        if (inRange3(expect(j),var_type(i),
     +                               NFT_ITYPE($1)) .and.
     +                      in_internal_range(NFT_ITYPE($1), expect(j)))
     +                          then
                            val = ARITH($1, value(j))
                            if (.not.equal(val,expect(j),
     +                                     var_type(i),NFT_ITYPE($1))) 
     +                              then
                                call error(
     +                              'value read not that expected')
                                if (verbose) then
                                    call error(' ')
                                    call errori('varid: ', i)
                                    call errorc('var_name: ',
     +                                  var_name(i))
                                    call errori('element number: %d ', 
     +                                          j)
                                    call errord('expect: ', expect(j))
                                    call errord('got: ', val)
                                end if
                            else
                                nok = nok + 1
                            end if
                        end if
9                   continue
                else
                    if (nels .gt. 0  .and. err .ne. NF_ECHAR)
     +                  call errore('wrong type: ', err)
                end if
5           continue
1       continue
        err = nf_close(ncid)
        if (err .ne. 0)
     +      call errorc('nf_close: ', nf_strerror(err))
        call print_nok(nok)
        end
])dnl


dnl TEST_NF_GET_VARS(TYPE)
dnl
define([TEST_NF_GET_VARS],dnl
[dnl
        subroutine test_nf_get_vars_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer d
        integer i
        integer j
        integer k
        integer m
        integer err
        logical allInExtRange   
        logical allInIntRange   
        integer nels
        integer nslabs
        integer nstarts         
        integer nok             
        integer start(MAX_RANK)
        integer edge(MAX_RANK)
        integer index(MAX_RANK)
        integer index2(MAX_RANK)
        integer mid(MAX_RANK)
        integer count(MAX_RANK)
        integer sstride(MAX_RANK)
        integer stride(MAX_RANK)
        logical canConvert     
        DATATYPE($1)    value(MAX_NELS)
        doubleprecision expect(MAX_NELS)
        doubleprecision val
        integer udshift

        nok = 0

        err = nf_open(testfile, NF_NOWRITE, ncid)
        if (err .ne. 0)
     +      call errore('nf_open: ', err)
        do 1, i = 1, NVARS
            canConvert = (var_type(i) .eq. NF_CHAR) .eqv. 
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
            if (.not.(var_rank(i) .le. MAX_RANK)) stop 2
            if (.not.(var_nels(i) .le. MAX_NELS)) stop 2
            do 2, j = 1, var_rank(i)
                start(j) = 1
                edge(j) = 1
                stride(j) = 1
2           continue
            err = nf_get_vars_$1(BAD_ID, i, start,
     +                  edge, stride, value)
            if (err .ne. NF_EBADID)
     +          call errore('bad ncid: ', err)
            err = nf_get_vars_$1(ncid, BAD_VARID,
     +                  start, edge, stride, 
     +                           value)
            if (err .ne. NF_ENOTVAR)
     +          call errore('bad var id: ', err)
            do 3, j = 1, var_rank(i)
                start(j) = var_shape(j,i) + 1
                err = nf_get_vars_$1(ncid, i, start,
     +                               edge, stride, 
     +                               value)
                if (.not. canConvert) then
                    if (err .ne. NF_ECHAR)
     +                  call errore('conversion: ', err)
                else
                    if (err .ne. NF_EINVALCOORDS)
     +                  call errore('bad index: ', err)
                endif
                start(j) = 1
                edge(j) = var_shape(j,i) + 1
                err = nf_get_vars_$1(ncid, i, start,
     +                               edge, stride, 
     +                               value)
                if (.not. canConvert) then
                    if (err .ne. NF_ECHAR)
     +                  call errore('conversion: ', err)
                else
                    if (err .ne. NF_EEDGE)
     +                  call errore('bad edge: ', err)
                endif
                edge(j) = 1
                stride(j) = 0
                err = nf_get_vars_$1(ncid, i, start,
     +                               edge, stride, 
     +                               value)
                if (.not. canConvert) then
                    if (err .ne. NF_ECHAR)
     +                  call errore('conversion: ', err)
                else
                    if (err .ne. NF_ESTRIDE)
     +                  call errore('bad stride: ', err)
                endif
                stride(j) = 1
3           continue
C               Choose a random point dividing each dim into 2 parts
C               get 2^rank (nslabs) slabs so defined
            nslabs = 1
            do 4, j = 1, var_rank(i)
                mid(j) = roll( var_shape(j,i) )
                nslabs = nslabs * 2
4           continue
C               bits of k determine whether to get lower or upper part of dim
C               choose random stride from 1 to edge
            do 5, k = 1, nslabs
                nstarts = 1
                do 6, j = 1, var_rank(i)
                    if (mod(udshift(k-1, j-1), 2) .eq. 1) then
                        start(j) = 1
                        edge(j) = mid(j)
                    else
                        start(j) = 1 + mid(j)
                        edge(j) = var_shape(j,i) - mid(j)
                    end if
                    if (edge(j) .gt. 0) then
                        sstride(j) = 1 + roll(edge(j))
                    else
                        sstride(j) = 1
                    end if
                    nstarts = nstarts * stride(j)
6               continue
                do 7, m = 1, nstarts
                    err = index2indexes(m, var_rank(i), sstride, 
     +                                  index)
                    if (err .ne. 0)
     +                  call error('error in index2indexes')
                    nels = 1
                    do 8, j = 1, var_rank(i)
                        count(j) = 1 + (edge(j) - index(j)) / 
     +                                  stride(j)
                        nels = nels * count(j)
                        index(j) = index(j) + start(j) - 1
8                   continue
C                           Random choice of forward or backward 
C    /* TODO
C                   if ( roll(2) ) then
C                       for (j = 0 j < var_rank(i) j++) {
C                           index(j) += (count(j) - 1) * stride(j)
C                           stride(j) = -stride(j)
C                       }
C                   end if
C    */
                    allInIntRange = .true.
                    allInExtRange = .true.
                    do 9, j = 1, nels
                        err = index2indexes(j, var_rank(i), count, 
     +                                      index2)
                        if (err .ne. 0)
     +                      call error('error in index2indexes() 1')
                        do 10, d = 1, var_rank(i)
                            index2(d) = index(d) + (index2(d)-1) * 
     +                                  stride(d)
10                      continue
                        expect(j) = hash4(var_type(i), var_rank(i), 
     +                                    index2, NFT_ITYPE($1))
                        if (inRange3(expect(j),var_type(i),
     +                               NFT_ITYPE($1))) then
                            allInIntRange = 
     +                          allInIntRange .and.
     +                          in_internal_range(NFT_ITYPE($1), 
     +                                            expect(j))
                        else
                            allInExtRange = .false.
                        end if
9                   continue
                    err = nf_get_vars_$1(ncid, i, index,
     +                                   count, stride,
     +                                   value)
                    if (canConvert) then
                        if (allInExtRange) then
                            if (allInIntRange) then
                                if (err .ne. 0)
     +                              call error(nf_strerror(err))
                            else
                                if (err .ne. NF_ERANGE)
     +                              call errore('Range error: ', err)
                            end if
                        else
                            if (err .ne. 0 .and. err .ne. NF_ERANGE)
     +                          call errore('OK or Range error: ', err)
                        end if
                        do 11, j = 1, nels
                            if (inRange3(expect(j),var_type(i),
     +                          NFT_ITYPE($1)) .and.
     +                          in_internal_range(NFT_ITYPE($1), 
     +                                            expect(j))) then
                                val = ARITH($1, value(j))
                                if (.not.equal(val, expect(j),
     +                              var_type(i), NFT_ITYPE($1))) then
                                    call error(
     +                                  'value read not that expected')
                                    if (verbose) then
                                        call error(' ')
                                        call errori('varid: ', i)
                                        call errorc('var_name: ', 
     +                                              var_name(i))
                                        call errori('element number: ',
     +                                              j)
                                        call errord('expect: ', 
     +                                              expect(j))
                                        call errord('got: ', val)
                                    end if
                                else
                                    nok = nok + 1
                                end if
                            end if
11                      continue
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
        call print_nok(nok)
        end
])dnl


dnl TEST_NF_GET_VARM(TYPE)
dnl
define([TEST_NF_GET_VARM],dnl
[dnl
        subroutine test_nf_get_varm_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer d
        integer i
        integer j
        integer k
        integer m
        integer err
        logical allInExtRange   
        logical allInIntRange   
        integer nels
        integer nslabs
        integer nstarts         
        integer nok             
        integer start(MAX_RANK)
        integer edge(MAX_RANK)
        integer index(MAX_RANK)
        integer index2(MAX_RANK)
        integer mid(MAX_RANK)
        integer count(MAX_RANK)
        integer sstride(MAX_RANK)
        integer stride(MAX_RANK)
        integer imap(MAX_RANK)
        logical canConvert     
        DATATYPE($1)    value(MAX_NELS)
        doubleprecision expect(MAX_NELS)
        doubleprecision val
        integer udshift

        nok = 0

        err = nf_open(testfile, NF_NOWRITE, ncid)
        if (err .ne. 0)
     +      call errore('nf_open: ', err)
        do 1, i = 1, NVARS
            canConvert = (var_type(i) .eq. NF_CHAR) .eqv. 
     +                   (NFT_ITYPE($1) .eq. NFT_TEXT)
            if (.not.(var_rank(i) .le. MAX_RANK)) stop 2
            if (.not.(var_nels(i) .le. MAX_NELS)) stop 2
            do 2, j = 1, var_rank(i)
                start(j) = 1
                edge(j) = 1
                stride(j) = 1
                imap(j) = 1
2           continue
            err = nf_get_varm_$1(BAD_ID, i, start, edge,
     +                           stride, imap, 
     +                           value)
            if (err .ne. NF_EBADID)
     +          call errore('bad ncid: ', err)
            err = nf_get_varm_$1(ncid, BAD_VARID, start,
     +                           edge, stride, 
     +                           imap, value)
            if (err .ne. NF_ENOTVAR)
     +          call errore('bad var id: ', err)
            do 3, j = 1, var_rank(i)
                start(j) = var_shape(j,i) + 1
                err = nf_get_varm_$1(ncid, i, start,
     +                               edge, stride, 
     +                               imap, value)
                if (.not. canConvert) then
                    if (err .ne. NF_ECHAR)
     +                  call errore('conversion: ', err)
                else
                    if (err .ne. NF_EINVALCOORDS)
     +                  call errore('bad index: ', err)
                endif
                start(j) = 1
                edge(j) = var_shape(j,i) + 1
                err = nf_get_varm_$1(ncid, i, start,
     +                               edge, stride, 
     +                               imap, value)
                if (.not. canConvert) then
                    if (err .ne. NF_ECHAR)
     +                  call errore('conversion: ', err)
                else
                    if (err .ne. NF_EEDGE)
     +                  call errore('bad edge: ', err)
                endif
                edge(j) = 1
                stride(j) = 0
                err = nf_get_varm_$1(ncid, i, start,
     +                               edge, stride, 
     +                               imap, value)
                if (.not. canConvert) then
                    if (err .ne. NF_ECHAR)
     +                  call errore('conversion: ', err)
                else
                    if (err .ne. NF_ESTRIDE)
     +                  call errore('bad stride: ', err)
                endif
                stride(j) = 1
3           continue
C               Choose a random point dividing each dim into 2 parts 
C               get 2^rank (nslabs) slabs so defined 
            nslabs = 1
            do 4, j = 1, var_rank(i)
                mid(j) = roll( var_shape(j,i) )
                nslabs = nslabs * 2
4           continue
C               /* bits of k determine whether to get lower or upper part 
C                * of dim
C                * choose random stride from 1 to edge */
            do 5, k = 1, nslabs
                nstarts = 1
                do 6, j = 1, var_rank(i)
                    if (mod(udshift((k-1), -(j-1)), 2) .ne. 0) then
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
                        count(j) = 1 + (edge(j) - index(j)) / 
     +                                  stride(j)
                        nels = nels * count(j)
                        index(j) = index(j) + start(j) - 1
8                   continue
C                       Random choice of forward or backward 
C    /* TODO
C                   if ( roll(2) ) then
C                       for (j = 0 j < var_rank(i) j++) {
C                           index(j) += (count(j) - 1) * stride(j)
C                           stride(j) = -stride(j)
C                       }
C                   end if
C     */
                    if (var_rank(i) .gt. 0) then
                        imap(1) = 1
                        do 9, j = 2, var_rank(i)
                            imap(j) = imap(j-1) * count(j-1)
9                       continue
                    end if
                    allInIntRange = .true.
                    allInExtRange = .true.
                    do 10, j = 1, nels
                        err = index2indexes(j, var_rank(i), count, 
     +                                      index2)
                        if (err .ne. 0)
     +                      call error('error in index2indexes 1')
                        do 11, d = 1, var_rank(i)
                            index2(d) = index(d) + (index2(d)-1) * 
     +                                  stride(d)
11                      continue
                        expect(j) = hash4(var_type(i), var_rank(i), 
     +                                    index2, NFT_ITYPE($1))
                        if (inRange3(expect(j),var_type(i),
     +                               NFT_ITYPE($1))) then
                            allInIntRange = 
     +                          allInIntRange .and.
     +                          in_internal_range(NFT_ITYPE($1),
     +                                            expect(j))
                        else
                            allInExtRange = .false.
                        end if
10                  continue
                    err = nf_get_varm_$1(ncid,i,index,count,
     +                                   stride,imap,
     +                                   value)
                    if (canConvert) then
                        if (allInExtRange) then
                            if (allInIntRange) then
                                if (err .ne. 0)
     +                              call error(nf_strerror(err))
                            else
                                if (err .ne. NF_ERANGE)
     +                              call errore('Range error: ', err)
                            end if
                        else
                            if (err .ne. 0 .and. err .ne. NF_ERANGE)
     +                          call errore('OK or Range error: ', err)
                        end if
                        do 12, j = 1, nels
                            if (inRange3(expect(j),var_type(i),
     +                                   NFT_ITYPE($1)) .and.
     +                          in_internal_range(NFT_ITYPE($1),
     +                                            expect(j))) then
                                val = ARITH($1, value(j))
                                if (.not.equal(val, expect(j),
     +                                         var_type(i), 
     +                                         NFT_ITYPE($1))) then
                                    call error(
     +                                  'value read not that expected')
                                    if (verbose) then
                                        call error(' ')
                                        call errori('varid: ', i)
                                        call errorc('var_name: ', 
     +                                          var_name(i))
                                        call errori('element number: ',
     +                                              j)
                                        call errord('expect: ', 
     +                                              expect(j))
                                        call errord('got: ', val)
                                    end if
                                else
                                    nok = nok + 1
                                end if
                            end if
12                      continue
                    else
                        if (nels .gt. 0 .and. err .ne. NF_ECHAR)
     +                      call errore('wrong type: ', err)
                    end if
7               continue
5           continue
1       continue
        err = nf_close(ncid)
        if (err .ne. 0)
     +      call errore('nf_close: ',  err)
        call print_nok(nok)
        end
])dnl


dnl TEST_NF_GET_ATT(TYPE)
dnl
define([TEST_NF_GET_ATT],dnl
[dnl
        subroutine test_nf_get_att_$1()
        implicit        none
#include "tests.inc"
        integer ncid
        integer i
        integer j
        integer k
        integer err
        integer ndx(1)
        logical allInExtRange
        logical allInIntRange
        logical canConvert     
        DATATYPE($1)    value(MAX_NELS)
        doubleprecision expect(MAX_NELS)
        integer nok             
        doubleprecision val

        nok = 0

        err = nf_open(testfile, NF_NOWRITE, ncid)
        if (err .ne. 0) 
     +      call errore('nf_open: ', err)

        do 1, i = 0, NVARS
            do 2, j = 1, NATTS(i)
                canConvert = (ATT_TYPE(j,i) .eq. NF_CHAR) .eqv.
     +                       (NFT_ITYPE($1) .eq. NFT_TEXT)
                err = nf_get_att_$1(BAD_ID, i,
     +                  ATT_NAME(j,i), 
     +                  value)
                if (err .ne. NF_EBADID) 
     +              call errore('bad ncid: ', err)
                err = nf_get_att_$1(ncid, BAD_VARID, 
     +                              ATT_NAME(j,i), 
     +                              value)
                if (err .ne. NF_ENOTVAR) 
     +              call errore('bad var id: ', err)
                err = nf_get_att_$1(ncid, i, 'noSuch', value)
                if (err .ne. NF_ENOTATT) 
     +              call errore('Bad attribute name: ', err)
                allInIntRange = .true.
                allInExtRange = .true.
                do 3, k = 1, ATT_LEN(j,i)
                    ndx(1) = k
                    expect(k) = hash4(ATT_TYPE(j,i), -1, ndx, 
     +                                NFT_ITYPE($1))
                    if (inRange3(expect(k),ATT_TYPE(j,i),
     +                           NFT_ITYPE($1))) then
                        allInIntRange = 
     +                      allInIntRange .and.
     +                      in_internal_range(NFT_ITYPE($1), expect(k))
                    else
                        allInExtRange = .false.
                    end if
3               continue
                err = nf_get_att_$1(ncid, i, ATT_NAME(j,i), value)
                if (canConvert .or. ATT_LEN(j,i) .eq. 0) then
                    if (allInExtRange) then
                        if (allInIntRange) then
                            if (err .ne. 0)
     +                          call errore('nf_get_att_$1: ', err)
                        else
                            if (err .ne. NF_ERANGE)
     +                          call errore('Range error: ', err)
                        end if
                    else
                        if (err .ne. 0 .and. err .ne. NF_ERANGE)
     +                      call errore('OK or Range error: ',
     +                                  err)
                    end if
                    do 4, k = 1, ATT_LEN(j,i)
                        if (inRange3(expect(k),ATT_TYPE(j,i),
     +                               NFT_ITYPE($1)) .and.
     +                      in_internal_range(NFT_ITYPE($1),
     +                                        expect(k))) then
                            val = ARITH($1, value(k))
                            if (.not.equal(val, expect(k),
     +                                     ATT_TYPE(j,i), 
     +                                     NFT_ITYPE($1)))then
                                call error(
     +                              'value read not that expected')
                                if (verbose) then
                                    call error(' ')
                                    call errori('varid: ', i)
                                    call errorc('att_name: ', 
     +                                  ATT_NAME(j,i))
                                    call errori('element number: ', k)
                                    call errord('expect: ', expect(k))
                                    call errord('got: ', val)
                                end if
                            else
                                nok = nok + 1
                            end if
                        end if
4                   continue
                else
                    if (err .ne. NF_ECHAR)
     +                  call errore('wrong type: ', err)
                end if
2           continue
1       continue

        err = nf_close(ncid)
        if (err .ne. 0)
     +      call errore('nf_close: ', err)
        call print_nok(nok)
        end
])dnl

divert(0)dnl
dnl If you see this line, you can ignore the next one.
C Do not edit this file. It is produced from the corresponding .m4 source */

C*********************************************************************
C   Copyright 1996, UCAR/Unidata
C   See netcdf/COPYRIGHT file for copying and redistribution conditions.
C   $Id$
C*********************************************************************

TEST_NF_GET_VAR1(text)
#ifdef NF_INT1_T
TEST_NF_GET_VAR1(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_GET_VAR1(int2)
#endif
TEST_NF_GET_VAR1(int)
TEST_NF_GET_VAR1(real)
TEST_NF_GET_VAR1(double)

TEST_NF_GET_VAR(text)
#ifdef NF_INT1_T
TEST_NF_GET_VAR(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_GET_VAR(int2)
#endif
TEST_NF_GET_VAR(int)
TEST_NF_GET_VAR(real)
TEST_NF_GET_VAR(double)

TEST_NF_GET_VARA(text)
#ifdef NF_INT1_T
TEST_NF_GET_VARA(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_GET_VARA(int2)
#endif
TEST_NF_GET_VARA(int)
TEST_NF_GET_VARA(real)
TEST_NF_GET_VARA(double)

TEST_NF_GET_VARS(text)
#ifdef NF_INT1_T
TEST_NF_GET_VARS(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_GET_VARS(int2)
#endif
TEST_NF_GET_VARS(int)
TEST_NF_GET_VARS(real)
TEST_NF_GET_VARS(double)

TEST_NF_GET_VARM(text)
#ifdef NF_INT1_T
TEST_NF_GET_VARM(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_GET_VARM(int2)
#endif
TEST_NF_GET_VARM(int)
TEST_NF_GET_VARM(real)
TEST_NF_GET_VARM(double)

TEST_NF_GET_ATT(text)
#ifdef NF_INT1_T
TEST_NF_GET_ATT(int1)
#endif
#ifdef NF_INT2_T
TEST_NF_GET_ATT(int2)
#endif
TEST_NF_GET_ATT(int)
TEST_NF_GET_ATT(real)
TEST_NF_GET_ATT(double)
