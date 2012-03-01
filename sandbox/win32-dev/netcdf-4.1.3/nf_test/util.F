!*********************************************************************
!   Copyright 1996, UCAR/Unidata
!   See netcdf/COPYRIGHT file for copying and redistribution conditions.
!   $Id$
!********************************************************************/


      SUBROUTINE PRINT_NOK(NOK)
      IMPLICIT  NONE
      INTEGER   NOK
#include "tests.inc"

      IF (VERBOSE .OR. NFAILS .GT. 0) PRINT *, ' '
      IF (VERBOSE) PRINT *, NOK, ' good comparisons.'
      END


! Is value within external type range? */
      FUNCTION INRANGE(VALUE, DATATYPE)
      IMPLICIT  NONE
      DOUBLEPRECISION   VALUE
      INTEGER           DATATYPE
#include "tests.inc"

      DOUBLEPRECISION   MIN
      DOUBLEPRECISION   MAX

      IF (DATATYPE .EQ. NF_CHAR) THEN
          MIN = X_CHAR_MIN
          MAX = X_CHAR_MAX
      ELSE IF (DATATYPE .EQ. NF_BYTE) THEN
          MIN = X_BYTE_MIN
          MAX = X_BYTE_MAX
      ELSE IF (DATATYPE .EQ. NF_SHORT) THEN
          MIN = X_SHORT_MIN
          MAX = X_SHORT_MAX
      ELSE IF (DATATYPE .EQ. NF_INT) THEN
          MIN = X_INT_MIN
          MAX = X_INT_MAX
      ELSE IF (DATATYPE .EQ. NF_FLOAT) THEN
          MIN = X_FLOAT_MIN
          MAX = X_FLOAT_MAX
      ELSE IF (DATATYPE .EQ. NF_DOUBLE) THEN
          MIN = X_DOUBLE_MIN
          MAX = X_DOUBLE_MAX
      ELSE
          CALL UDABORT
      END IF

      INRANGE = (VALUE .GE. MIN) .AND. (VALUE .LE. MAX)
      END


      FUNCTION INRANGE_UCHAR(VALUE, DATATYPE)
      IMPLICIT  NONE
      DOUBLEPRECISION   VALUE
      INTEGER           DATATYPE
#include "tests.inc"

      IF (DATATYPE .EQ. NF_BYTE) THEN
          INRANGE_UCHAR = (VALUE .GE. 0) .AND. (VALUE .LE. 255)
      ELSE
          INRANGE_UCHAR = INRANGE(VALUE, DATATYPE)
      END IF
      END


      FUNCTION INRANGE_FLOAT(VALUE, DATATYPE)
      IMPLICIT  NONE
      DOUBLEPRECISION   VALUE
      INTEGER           DATATYPE
#include "tests.inc"

      DOUBLEPRECISION   MIN
      DOUBLEPRECISION   MAX
      REAL              FVALUE

      IF (DATATYPE .EQ. NF_CHAR) THEN
          MIN = X_CHAR_MIN
          MAX = X_CHAR_MAX
      ELSE IF (DATATYPE .EQ. NF_BYTE) THEN
          MIN = X_BYTE_MIN
          MAX = X_BYTE_MAX
      ELSE IF (DATATYPE .EQ. NF_SHORT) THEN
          MIN = X_SHORT_MIN
          MAX = X_SHORT_MAX
      ELSE IF (DATATYPE .EQ. NF_INT) THEN
          MIN = X_INT_MIN
          MAX = X_INT_MAX
      ELSE IF (DATATYPE .EQ. NF_FLOAT) THEN
          IF (internal_max(NFT_REAL) .LT. X_FLOAT_MAX) THEN
              MIN = -internal_max(NFT_REAL)
              MAX = internal_max(NFT_REAL)
          ELSE
              MIN = X_FLOAT_MIN
              MAX = X_FLOAT_MAX
          END IF
      ELSE IF (DATATYPE .EQ. NF_DOUBLE) THEN
          IF (internal_max(NFT_REAL) .LT. X_DOUBLE_MAX) THEN
              MIN = -internal_max(NFT_REAL)
              MAX = internal_max(NFT_REAL)
          ELSE
              MIN = X_DOUBLE_MIN
              MAX = X_DOUBLE_MAX
          END IF
      ELSE
          CALL UDABORT
      END IF

      IF (.NOT.((VALUE .GE. MIN) .AND. (VALUE .LE. MAX))) THEN
          INRANGE_FLOAT = .FALSE.
      ELSE
          FVALUE = VALUE
          INRANGE_FLOAT = (FVALUE .GE. MIN) .AND. (FVALUE .LE. MAX)
      END IF
      END


! wrapper for inrange to handle special NF_BYTE/uchar adjustment */
      function inrange3(value, datatype, itype)
      implicit          none
      doubleprecision   value
      integer           datatype
      integer           itype
#include "tests.inc"

      if (itype .eq. NFT_REAL) then
          inrange3 = inrange_float(value, datatype)
      else
          inrange3 = inrange(value, datatype)
      end if
      end


!
!  Does x == y, where one is internal and other external (netCDF)?  
!  Use tolerant comparison based on IEEE FLT_EPSILON or DBL_EPSILON.
!
      function equal(x, y, extType, itype)
      implicit  none
      doubleprecision   x
      doubleprecision   y
      integer           extType         !!/* external data type */
      integer           itype
#include "tests.inc"

      doubleprecision   epsilon

      if ((extType .eq. NF_REAL) .or. (itype .eq. NFT_REAL)) then
          epsilon = 1.19209290E-07
      else
          epsilon = 2.2204460492503131E-16
      end if
      equal = abs(x-y) .le. epsilon * max( abs(x), abs(y))
      end


! Test whether two int vectors are equal. If so return 1, else 0  */
        function int_vec_eq(v1, v2, n)
        implicit        none
        integer n
        integer v1(n)
        integer v2(n)
#include "tests.inc"

        integer i

        int_vec_eq = .true.

        if (n .le. 0)
     +      return

        do 1, i=1, n
            if (v1(i) .ne. v2(i)) then
                int_vec_eq = .false.
                return
            end if
1       continue
        end


!
!  Generate random integer from 0 through n-1
!  Like throwing an n-sided dice marked 0, 1, 2, ..., n-1
!
      function roll(n)
      implicit  none
      integer   n
#include "tests.inc"

      doubleprecision   udrand
      external          udrand

1     roll = (udrand(0) * (n-1)) + 0.5
      if (roll .ge. n) goto 1
      end


!
!      Convert an origin-1 cumulative index to a netCDF index vector.
!       Grosset dimension first; finest dimension last.
!
!      Authors: Harvey Davies, Unidata/UCAR, Boulder, Colorado
!                Steve Emmerson, (same place)
!
        function index2ncindexes(index, rank, base, indexes)
        implicit        none
        integer         index           !!/* index to be converted */
        integer         rank            !/* number of dimensions */
        integer         base(rank)      !/* base(rank) ignored */
        integer         indexes(rank)   !/* returned FORTRAN indexes */
#include "tests.inc"

        integer i
        integer offset

        if (rank .gt. 0) then
            offset = index - 1
            do 1, i = rank, 1, -1
                if (base(i) .eq. 0) then
                    index2ncindexes = 1
                    return
                end if
                indexes(i) = 1 + mod(offset, base(i))
                offset = offset / base(i)
1           continue
        end if
        index2ncindexes = 0
        end


!
!      Convert an origin-1 cumulative index to a FORTRAN index vector.
!       Finest dimension first; grossest dimension last.
!
!      Authors: Harvey Davies, Unidata/UCAR, Boulder, Colorado
!                Steve Emmerson, (same place)
!
        function index2indexes(index, rank, base, indexes)
        implicit        none
        integer         index           !/* index to be converted */
        integer         rank            !/* number of dimensions */
        integer         base(rank)      !/* base(rank) ignored */
        integer         indexes(rank)   !/* returned FORTRAN indexes */
#include "tests.inc"

        integer i
        integer offset

        if (rank .gt. 0) then
            offset = index - 1
            do 1, i = 1, rank
                if (base(i) .eq. 0) then
                    index2indexes = 1
                    return
                end if
                indexes(i) = 1 + mod(offset, base(i))
                offset = offset / base(i)
1           continue
        end if
        index2indexes = 0
        end


!
!      Convert a FORTRAN index vector to an origin-1 cumulative index.
!       Finest dimension first; grossest dimension last.
!
!      Authors: Harvey Davies, Unidata/UCAR, Boulder, Colorado
!                Steve Emmerson, (same place)
!
        function indexes2index(rank, indexes, base)
        implicit        none
        integer         rank            !/* number of dimensions */
        integer         indexes(rank)   !/* FORTRAN indexes */
        integer         base(rank)      !/* base(rank) ignored */
#include "tests.inc"

        integer i

        indexes2index = 0
        if (rank .gt. 0) then
            do 1, i = rank, 1, -1
                indexes2index = (indexes2index-1) * base(i) + indexes(i)
1           continue
        end if
        end


#ifdef USE_EXTREME_NUMBERS
! Generate data values as function of type, rank (-1 for attribute), index */
      function hash(type, rank, index) 
      implicit  none
      integer   type
      integer   rank
      integer   index(*)
#include "tests.inc"

      doubleprecision   base
      doubleprecision   result
      integer           d       !/* index of dimension */

        !/* If vector then elements 1 & 2 are min & max. Elements 3 & 4 are */
        !/* just < min & > max (except for NF_CHAR & NF_DOUBLE) */
      if (abs(rank) .eq. 1 .and. index(1) .le. 4) then
          if (index(1) .eq. 1) then
              if (type .eq. NF_CHAR) then
                  hash = X_CHAR_MIN
              else if (type .eq. NF_BYTE) then
                  hash = X_BYTE_MIN
              else if (type .eq. NF_SHORT) then
                  hash = X_SHORT_MIN
              else if (type .eq. NF_INT) then
                  hash = X_INT_MIN
              else if (type .eq. NF_FLOAT) then
                  hash = X_FLOAT_MIN
              else if (type .eq. NF_DOUBLE) then
                  hash = X_DOUBLE_MIN
              else
                  call udabort
              end if
          else if (index(1) .eq. 2) then
              if (type .eq. NF_CHAR) then
                  hash = X_CHAR_MAX
              else if (type .eq. NF_BYTE) then
                  hash = X_BYTE_MAX
              else if (type .eq. NF_SHORT) then
                  hash = X_SHORT_MAX
              else if (type .eq. NF_INT) then
                  hash = X_INT_MAX
              else if (type .eq. NF_FLOAT) then
                  hash = X_FLOAT_MAX
              else if (type .eq. NF_DOUBLE) then
                  hash = X_DOUBLE_MAX
              else
                  call udabort
              end if
          else if (index(1) .eq. 3) then
              if (type .eq. NF_CHAR) then
                  hash = ichar('A')
              else if (type .eq. NF_BYTE) then
                  hash = X_BYTE_MIN-1.0
              else if (type .eq. NF_SHORT) then
                  hash = X_SHORT_MIN-1.0
              else if (type .eq. NF_INT) then
                  hash = X_INT_MIN
              else if (type .eq. NF_FLOAT) then
                  hash = X_FLOAT_MIN
              else if (type .eq. NF_DOUBLE) then
                  hash = -1.0
              else
                  call udabort
              end if
          else if (index(1) .eq. 4) then
              if (type .eq. NF_CHAR) then
                  hash = ichar('Z')
              else if (type .eq. NF_BYTE) then
                  hash = X_BYTE_MAX+1.0
              else if (type .eq. NF_SHORT) then
                  hash = X_SHORT_MAX+1.0
              else if (type .eq. NF_INT) then
                  hash = X_INT_MAX+1.0
              else if (type .eq. NF_FLOAT) then
                  hash = X_FLOAT_MAX
              else if (type .eq. NF_DOUBLE) then
                  hash = 1.0
              else
                  call udabort
              end if
          end if
      else
          if (type .eq. NF_CHAR) then
              base = 2
          else if (type .eq. NF_BYTE) then
              base = -2
          else if (type .eq. NF_SHORT) then
              base = -5
          else if (type .eq. NF_INT) then
              base = -20
          else if (type .eq. NF_FLOAT) then
              base = -9
          else if (type .eq. NF_DOUBLE) then
              base = -10
          else
              stop 2
          end if

          if (rank .lt. 0) then
              result = base * 7
          else
              result = base * (rank + 1)
          end if

!         /*
!          * NB: Finest netCDF dimension assumed first.
!          */
          do 1, d = abs(rank), 1, -1
              result = base * (result + index(d) - 1)
1         continue
          hash = result
      end if
      end
#else /* USE_EXTREME_NUMBERS */
#define SANE_SHORT 3333
#define SANE_INT 2222
#define SANE_FLOAT 300.0
#define SANE_DOUBLE 1000.0

! Generate data values as function of type, rank (-1 for attribute), index */
      function hash(type, rank, index) 
      implicit  none
      integer   type
      integer   rank
      integer   index(*)
#include "tests.inc"

      doubleprecision   base
      doubleprecision   result
      integer           d       !/* index of dimension */

        !/* If vector then elements 1 & 2 are min & max. Elements 3 & 4 are */
        !/* just < min & > max (except for NF_CHAR & NF_DOUBLE) */
      if (abs(rank) .eq. 1 .and. index(1) .le. 4) then
          if (index(1) .eq. 1) then
              if (type .eq. NF_CHAR) then
                  hash = X_CHAR_MIN
              else if (type .eq. NF_BYTE) then
                  hash = X_BYTE_MIN
              else if (type .eq. NF_SHORT) then
                  hash = SANE_SHORT
              else if (type .eq. NF_INT) then
                  hash = SANE_INT
              else if (type .eq. NF_FLOAT) then
                  hash = SANE_FLOAT
              else if (type .eq. NF_DOUBLE) then
                  hash = SANE_DOUBLE
              else
                  call udabort
              end if
          else if (index(1) .eq. 2) then
              if (type .eq. NF_CHAR) then
                  hash = X_CHAR_MAX
              else if (type .eq. NF_BYTE) then
                  hash = X_BYTE_MAX
              else if (type .eq. NF_SHORT) then
                  hash = SANE_SHORT
              else if (type .eq. NF_INT) then
                  hash = SANE_INT
              else if (type .eq. NF_FLOAT) then
                  hash = SANE_FLOAT
              else if (type .eq. NF_DOUBLE) then
                  hash = SANE_DOUBLE
              else
                  call udabort
              end if
          else if (index(1) .eq. 3) then
              if (type .eq. NF_CHAR) then
                  hash = ichar('A')
              else if (type .eq. NF_BYTE) then
                  hash = X_BYTE_MIN-1.0
              else if (type .eq. NF_SHORT) then
                  hash = SANE_SHORT-1.0
              else if (type .eq. NF_INT) then
                  hash = SANE_INT
              else if (type .eq. NF_FLOAT) then
                  hash = SANE_FLOAT
              else if (type .eq. NF_DOUBLE) then
                  hash = -1.0
              else
                  call udabort
              end if
          else if (index(1) .eq. 4) then
              if (type .eq. NF_CHAR) then
                  hash = ichar('Z')
              else if (type .eq. NF_BYTE) then
                  hash = X_BYTE_MAX+1.0
              else if (type .eq. NF_SHORT) then
                  hash = SANE_SHORT+1.0
              else if (type .eq. NF_INT) then
                  hash = SANE_INT+1.0
              else if (type .eq. NF_FLOAT) then
                  hash = SANE_FLOAT
              else if (type .eq. NF_DOUBLE) then
                  hash = 1.0
              else
                  call udabort
              end if
          end if
      else
          if (type .eq. NF_CHAR) then
              base = 2
          else if (type .eq. NF_BYTE) then
              base = -2
          else if (type .eq. NF_SHORT) then
              base = -5
          else if (type .eq. NF_INT) then
              base = -20
          else if (type .eq. NF_FLOAT) then
              base = -9
          else if (type .eq. NF_DOUBLE) then
              base = -10
          else
              stop 2
          end if

          if (rank .lt. 0) then
              result = base * 7
          else
              result = base * (rank + 1)
          end if

!         /*
!          * NB: Finest netCDF dimension assumed first.
!          */
          do 1, d = abs(rank), 1, -1
              result = base * (result + index(d) - 1)
1         continue
          hash = result
      end if
      end
#endif

! wrapper for hash to handle special NC_BYTE/uchar adjustment */
      function hash4(type, rank, index, itype)
      implicit  none
      integer   type
      integer   rank
      integer   index(*)
      integer   itype
#include "tests.inc"

      hash4 = hash( type, rank, index )
      if ((itype .eq. NFT_CHAR) .and. (type .eq. NF_BYTE) .and. 
     +    (hash4 .ge. -128) .and. (hash4 .lt. 0)) hash4 = hash4 + 256
      end


      integer function char2type(letter)
      implicit          none
      character*1       letter
#include "tests.inc"

      if (letter .eq. 'c') then
          char2type = NF_CHAR
      else if (letter .eq. 'b') then
          char2type = NF_BYTE
      else if (letter .eq. 's') then
          char2type = NF_SHORT
      else if (letter .eq. 'i') then
          char2type = NF_INT
      else if (letter .eq. 'f') then
          char2type = NF_FLOAT
      else if (letter .eq. 'd') then
          char2type = NF_DOUBLE
      else
        stop 2
      end if
      end


      subroutine init_dims(digit)
      implicit          none
      character*1       digit(NDIMS)
#include "tests.inc"

      integer   dimid                   !/* index of dimension */
      do 1, dimid = 1, NDIMS
          if (dimid .eq. RECDIM) then
              dim_len(dimid) = NRECS
          else
              dim_len(dimid) = dimid - 1
          endif
          dim_name(dimid) = 'D' // digit(dimid)
1     continue
      end


      subroutine init_gatts(type_letter)
      implicit          none
      character*1       type_letter(NTYPES)
#include "tests.inc"

      integer   attid
      integer   char2type

      do 1, attid = 1, NTYPES
          gatt_name(attid) = 'G' // type_letter(attid)
          gatt_len(attid) = attid
          gatt_type(attid) = char2type(type_letter(attid))
1     continue
      end


      integer function prod(nn, sp)
      implicit  none
      integer   nn
      integer   sp(MAX_RANK)
#include "tests.inc"

      integer   i

      prod = 1
      do 1, i = 1, nn
          prod = prod * sp(i)
1     continue
      end


!
!   define global variables:
!   dim_name, dim_len, 
!   var_name, var_type, var_rank, var_shape, var_natts, var_dimid, var_nels
!   att_name, gatt_name, att_type, gatt_type, att_len, gatt_len
!
        subroutine init_gvars
        implicit        none
#include "tests.inc"

        integer         max_dim_len(MAX_RANK)
        character*1     type_letter(NTYPES)
        character*1     digit(10)

        integer rank
        integer vn              !/* var number */
        integer xtype           !/* index of type */
        integer an              !/* origin-0 cumulative attribute index */
        integer nvars
        integer jj
        integer ntypes
        integer tc
        integer tmp(MAX_RANK)
        integer ac              !/* attribute index */
        integer dn              !/* dimension number */
        integer prod            !/* function */
        integer char2type       !/* function */
        integer err

        data    max_dim_len     /0, MAX_DIM_LEN, MAX_DIM_LEN/
        data    type_letter     /'c', 'b', 's', 'i', 'f', 'd'/
        data    digit           /'r', '1', '2', '3', '4', '5',
     +                           '6', '7', '8', '9'/

        max_dim_len(1) = MAX_DIM_LEN + 1

        call init_dims(digit)

        vn = 1
        xtype = 1
        an = 0

!       /* Loop over variable ranks */
        do 1, rank = 0, MAX_RANK
            nvars = prod(rank, max_dim_len)

            !/* Loop over variable shape vectors */
            do 2, jj = 1, nvars                         !/* 1, 5, 20, 80 */
                !/* number types of this shape */
                if (rank .lt. 2) then
                    ntypes = NTYPES                     !/* 6 */
                else
                    ntypes = 1
                end if

                !/* Loop over external data types */
                do 3, tc = 1, ntypes                    !/* 6, 1 */
                    var_name(vn) = type_letter(xtype)
                    var_type(vn) = char2type(type_letter(xtype))
                    var_rank(vn) = rank
                    if (rank .eq. 0) then
                        var_natts(vn) = mod(vn - 1, MAX_NATTS + 1)
                    else
                        var_natts(vn) = 0
                    end if

                    do 4, ac = 1, var_natts(vn)
                        attname(ac,vn) = 
     +                      type_letter(1+mod(an, NTYPES))
                        attlen(ac,vn) = an
                        atttype(ac,vn) =
     +                      char2type(type_letter(1+mod(an, NTYPES)))
                        an = an + 1
4                   continue

                    !/* Construct initial shape vector */
                    err = index2ncindexes(jj, rank, max_dim_len, tmp)
                    do 5, dn = 1, rank
                        var_dimid(dn,vn) = tmp(1+rank-dn)
5                   continue

                    var_nels(vn) = 1
                    do 6, dn = 1, rank
                        if (dn .lt. rank) then
                            var_dimid(dn,vn) = var_dimid(dn,vn) + 1
                        end if
                        if (var_dimid(dn,vn) .gt. 9) then
                            stop 2
                        end if
                        var_name(vn)(rank+2-dn:rank+2-dn) = 
     +                      digit(var_dimid(dn,vn))
                        if (var_dimid(dn,vn) .ne. RECDIM) then
                            var_shape(dn,vn) = var_dimid(dn,vn) - 1
                        else
                            var_shape(dn,vn) = NRECS
                        end if
                        var_nels(vn) = var_nels(vn) * var_shape(dn,vn)
6                   continue

                    vn = vn + 1
                    xtype = 1 + mod(xtype, NTYPES)
3               continue
2           continue
1       continue

        call init_gatts(type_letter)
        end


! define dims defined by global variables */
        subroutine def_dims(ncid)
        implicit        none
        integer         ncid
#include "tests.inc"

        integer         err             !/* status */
        integer         i
        integer         dimid           !/* dimension id */

        do 1, i = 1, NDIMS
            if (i .eq. RECDIM) then
                err = nf_def_dim(ncid, dim_name(i), NF_UNLIMITED,
     +                           dimid)
            else
                err = nf_def_dim(ncid, dim_name(i), dim_len(i),
     +                           dimid)
            end if
            if (err .ne. 0) then
                call errore('nf_def_dim: ', err)
            end if
1       continue
        end


! define vars defined by global variables */
        subroutine def_vars(ncid)
        implicit        none
        integer         ncid
#include "tests.inc"

        integer         err             !/* status */
        integer         i
        integer         var_id

        do 1, i = 1, NVARS
            err = nf_def_var(ncid, var_name(i), var_type(i), 
     +                       var_rank(i), var_dimid(1,i), var_id)
            if (err .ne. 0) then
                call errore('nf_def_var: ', err)
            end if
1       continue
        end


! put attributes defined by global variables */
        subroutine put_atts(ncid)
        implicit        none
        integer         ncid
#include "tests.inc"

        integer                 err             !/* netCDF status */
        integer                 i               !/* variable index (0 => global 
                                                ! * attribute */
        integer                 k               !/* attribute index */
        integer                 j               !/* index of attribute */
        integer                 ndx(1)
        logical                 allInRange
        doubleprecision         att(MAX_NELS)
        character*(MAX_NELS+2)  catt

        do 1, i = 0, NVARS      !/* var 0 => NF_GLOBAL attributes */
            do 2, j = 1, NATTS(i)
                if (NF_CHAR .eq. ATT_TYPE(j,i)) then
                    catt = ' '
                    do 3, k = 1, ATT_LEN(j,i)
                        ndx(1) = k
                        catt(k:k) = char(int(hash(ATT_TYPE(j,i), -1, 
     +                                   ndx)))
3                   continue
!                   /*
!                    * The following ensures that the text buffer doesn't
!                    * start with 4 zeros (which is a CFORTRAN NULL pointer
!                    * indicator) yet contains a zero (which causes the
!                    * CFORTRAN interface to pass the address of the
!                    * actual text buffer).
!                    */
                    catt(ATT_LEN(j,i)+1:ATT_LEN(j,i)+1) = char(1)
                    catt(ATT_LEN(j,i)+2:ATT_LEN(j,i)+2) = char(0)

                    err = nf_put_att_text(ncid, varid(i), 
     +                                    ATT_NAME(j,i),
     +                                    ATT_LEN(j,i), catt)
                    if (err .ne. 0) then
                        call errore('nf_put_att_text: ', err)
                    end if
                else
                    allInRange = .true.
                    do 4, k = 1, ATT_LEN(j,i)
                        ndx(1) = k
                        att(k) = hash(ATT_TYPE(j,i), -1, ndx)
                        allInRange = allInRange .and.
     +                               inRange(att(k), ATT_TYPE(j,i))
4                   continue
                    err = nf_put_att_double(ncid, varid(i),
     +                                      ATT_NAME(j,i),
     +                                      ATT_TYPE(j,i),
     +                                      ATT_LEN(j,i), att)
                    if (allInRange) then
                        if (err .ne. 0) then
                            call errore('nf_put_att_double: ', err)
                        end if
                    else
                        if (err .ne. NF_ERANGE) then
                            call errore(
     +                  'type-conversion range error: status = ',
     +                          err)
                        end if
                    end if
                end if
2           continue
1       continue
        end


! put variables defined by global variables */
        subroutine put_vars(ncid)
        implicit        none
        integer                 ncid
#include "tests.inc"

        integer                 start(MAX_RANK)
        integer                 index(MAX_RANK)
        integer                 err             !/* netCDF status */
        integer                 i
        integer                 j
        doubleprecision         value(MAX_NELS)
        character*(MAX_NELS+2)  text
        logical                 allInRange

        do 1, j = 1, MAX_RANK
            start(j) = 1
1       continue

        do 2, i = 1, NVARS
            allInRange = .true.
            do 3, j = 1, var_nels(i)
                err = index2indexes(j, var_rank(i), var_shape(1,i), 
     +                              index)
                if (err .ne. 0) then
                    call errori(
     +                  'Error calling index2indexes() for var ', j)
                end if
                if (var_name(i)(1:1) .eq. 'c') then
                    text(j:j) = 
     +                  char(int(hash(var_type(i), var_rank(i), index)))
                else
                    value(j)  = hash(var_type(i), var_rank(i), index)
                    allInRange = allInRange .and.
     +                  inRange(value(j), var_type(i))
                end if
3           continue
            if (var_name(i)(1:1) .eq. 'c') then
!               /*
!                * The following statement ensures that the first 4
!                * characters in 'text' are not all zeros (which is
!                * a cfortran.h NULL indicator) and that the string
!                * contains a zero (which will cause the address of the
!                * actual string buffer to be passed).
!                */
                text(var_nels(i)+1:var_nels(i)+1) = char(1)
                text(var_nels(i)+2:var_nels(i)+2) = char(0)
                err = nf_put_vara_text(ncid, i, start, var_shape(1,i), 
     +                                 text)
                if (err .ne. 0) then
                    call errore('nf_put_vara_text: ', err)
                end if
            else
                err = nf_put_vara_double(ncid, i, start, var_shape(1,i),
     +                                   value)
                if (allInRange) then
                    if (err .ne. 0) then
                        call errore('nf_put_vara_double: ', err)
                    end if
                else
                    if (err .ne. NF_ERANGE) then
                        call errore(
     +                      'type-conversion range error: status = ', 
     +                      err)
                    end if
                end if
            end if
2       continue
        end


! Create & write all of specified file using global variables */
        subroutine write_file(filename) 
        implicit        none
        character*(*)   filename
#include "tests.inc"

        integer ncid            !/* netCDF id */
        integer err             !/* netCDF status */

        err = nf_create(filename, NF_CLOBBER, ncid)
        if (err .ne. 0) then
            call errore('nf_create: ', err)
        end if

        call def_dims(ncid)
        call def_vars(ncid)
        call put_atts(ncid)
        err = nf_enddef(ncid)
        if (err .ne. 0) then
            call errore('nf_enddef: ', err)
        end if
        call put_vars(ncid)

        err = nf_close(ncid)
        if (err .ne. 0) then
            call errore('nf_close: ', err)
        end if
        end


!
! check dimensions of specified file have expected name & length
!
        subroutine check_dims(ncid)
        implicit        none
        integer         ncid
#include "tests.inc"

        character*(NF_MAX_NAME) name
        integer                 length
        integer                 i
        integer                 err           !/* netCDF status */

        do 1, i = 1, NDIMS
            err = nf_inq_dim(ncid, i, name, length)
            if (err .ne. 0) then
                call errore('nf_inq_dim: ', err)
            end if
            if (name .ne. dim_name(i)) then
                call errori('Unexpected name of dimension ', i)
            end if
            if (length .ne. dim_len(i)) then
                call errori('Unexpected length of dimension ', i)
            end if
1       continue
        end


!
! check variables of specified file have expected name, type, shape & values
!
        subroutine check_vars(ncid)
        implicit        none
        integer         ncid
#include "tests.inc"

        integer                 index(MAX_RANK)
        integer                 err             !/* netCDF status */
        integer                 i
        integer                 j
        character*1             text
        doubleprecision         value
        integer                 datatype
        integer                 ndims
        integer                 natt
        integer                 dimids(MAX_RANK)
        logical                 isChar
        doubleprecision         expect
        character*(NF_MAX_NAME) name
        integer                 length
        integer                 nok             !/* count of valid comparisons */

        nok = 0

        do 1, i = 1, NVARS
            isChar = var_type(i) .eq. NF_CHAR
            err = nf_inq_var(ncid, i, name, datatype, ndims, dimids, 
     +          natt)
            if (err .ne. 0) then
                call errore('nf_inq_var: ', err)
            end if
            if (name .ne. var_name(i)) then
                call errori('Unexpected var_name for variable ', i)
            end if
            if (datatype .ne. var_type(i))  then
                call errori('Unexpected type for variable ', i)
            end if
            if (ndims .ne. var_rank(i))  then
                call errori('Unexpected rank for variable ', i)
            end if
            do 2, j = 1, ndims
                err = nf_inq_dim(ncid, dimids(j), name, length)
                if (err .ne. 0) then
                    call errore('nf_inq_dim: ', err)
                end if
                if (length .ne. var_shape(j,i))  then
                    call errori('Unexpected shape for variable ', i)
                end if
2           continue
            do 3, j = 1, var_nels(i)
                err = index2indexes(j, var_rank(i), var_shape(1,i), 
     +                  index)
                if (err .ne. 0)  then
                    call errori('error in index2indexes() 2, variable ',
     +                          i)
                end if
                expect = hash(var_type(i), var_rank(i), index )
                if (isChar) then
                    err = nf_get_var1_text(ncid, i, index, text)
                    if (err .ne. 0) then
                        call errore('nf_get_var1_text: ', err)
                    end if
                    if (ichar(text) .ne. expect) then
                        call errori(
     +              'Var value read not that expected for variable ', i)
                    else
                        nok = nok + 1
                    end if
                else
                    err = nf_get_var1_double(ncid, i, index, value)
                    if (inRange(expect,var_type(i))) then
                        if (err .ne. 0) then
                            call errore('nf_get_var1_double: ', err)
                        else
                            if (.not. equal(value,expect,var_type(i),
     +                          NFT_DOUBLE)) then
                                call errori(
     +              'Var value read not that expected for variable ', i)
                            else
                                nok = nok + 1
                            end if
                        end if
                    end if
                end if
3           continue
1       continue
        call print_nok(nok)
        end


!
! check attributes of specified file have expected name, type, length & values
!
        subroutine check_atts(ncid) 
        implicit        none
        integer         ncid
#include "tests.inc"

        integer                 err             !/* netCDF status */
        integer                 i
        integer                 j
        integer                 k
        integer                 vid             !/* "variable" ID */
        integer                 datatype
        integer                 ndx(1)
        character*(NF_MAX_NAME) name
        integer                 length
        character*(MAX_NELS)    text
        doubleprecision         value(MAX_NELS)
        doubleprecision         expect
        integer                 nok             !/* count of valid comparisons */

        nok = 0

        do 1, vid = 0, NVARS
            i = varid(vid)

            do 2, j = 1, NATTS(i)
                err = nf_inq_attname(ncid, i, j, name)
                if (err .ne. 0) then
                    call errore('nf_inq_attname: ', err)
                end if
                if (name .ne. ATT_NAME(j,i)) then
                    call errori(
     +                  'nf_inq_attname: unexpected name for var ', i)
                end if
                err = nf_inq_att(ncid, i, name, datatype, length)
                if (err .ne. 0) then
                    call errore('nf_inq_att: ', err)
                end if
                if (datatype .ne. ATT_TYPE(j,i)) then
                    call errori('nf_inq_att: unexpected type for var ',
     +                         i)
                end if
                if (length .ne. ATT_LEN(j,i)) then
                    call errori(
     +                  'nf_inq_att: unexpected length for var ', i)
                end if
                if (datatype .eq. NF_CHAR) then
                    err = nf_get_att_text(ncid, i, name, text)
                    if (err .ne. 0) then
                        call errore('nf_get_att_text: ', err)
                    end if
                    do 3, k = 1, ATT_LEN(j,i)
                        ndx(1) = k
                        if (ichar(text(k:k)) .ne. hash(datatype, -1, 
     +                                                 ndx))
     +                  then
                            call errori(
     +          'nf_get_att_text: unexpected value for var ', i)
                        else
                            nok = nok + 1
                        end if
3                   continue
                else
                    err = nf_get_att_double(ncid, i, name, value)
                    do 4, k = 1, ATT_LEN(j,i)
                        ndx(1) = k
                        expect = hash(datatype, -1, ndx)
                        if (inRange(expect,ATT_TYPE(j,i))) then
                            if (err .ne. 0) then
                                call errore('nf_get_att_double: ', err)
                            end if
                            if (.not. equal(value(k), expect,
     +                          ATT_TYPE(j,i), NFT_DOUBLE)) then
                                call errori(
     +                  'Att value read not that expected for var ', i)
                            else
                                nok = nok + 1
                            end if
                        end if
4                   continue
                end if
2           continue
1       continue
        call print_nok(nok)
        end


! Check file (dims, vars, atts) corresponds to global variables */
        subroutine check_file(filename) 
        implicit        none
        character*(*)   filename
#include "tests.inc"

        integer ncid            !/* netCDF id */
        integer err             !/* netCDF status */

        err = nf_open(filename, NF_NOWRITE, ncid)
        if (err .ne. 0) then
            call errore('nf_open: ', err)
        else
            call check_dims(ncid)
            call check_vars(ncid)
            call check_atts(ncid)
            err = nf_close (ncid)
            if (err .ne. 0) then
                call errore('nf_close: ', err)
            end if
        end if
        end


!
! Functions for accessing attribute test data.
!
! NB: 'varid' is 0 for global attributes; thus, global attributes can
! be handled in the same loop as variable attributes.
!

      FUNCTION VARID(VID)
      IMPLICIT NONE
      INTEGER VID
#include "tests.inc"
      IF (VID .LT. 1) THEN
          VARID = NF_GLOBAL
      ELSE
          VARID = VID
      ENDIF
      end


      FUNCTION NATTS(VID)
      IMPLICIT  NONE
      INTEGER VID
#include "tests.inc"
      IF (VID .LT. 1) THEN
          NATTS = NGATTS
      ELSE
          NATTS = VAR_NATTS(VID)
      ENDIF
      END


      FUNCTION ATT_NAME(J,VID)
      IMPLICIT  NONE
      INTEGER J
      INTEGER VID
#include "tests.inc"
      IF (VID .LT. 1) THEN
          ATT_NAME = GATT_NAME(J)
      ELSE
          ATT_NAME = ATTNAME(J,VID)
      ENDIF
      END


      FUNCTION ATT_TYPE(J,VID)
      IMPLICIT  NONE
      INTEGER J
      INTEGER VID
#include "tests.inc"
      IF (VID .LT. 1) THEN
          ATT_TYPE = GATT_TYPE(J)
      ELSE
          ATT_TYPE = ATTTYPE(J,VID)
      ENDIF
      END


      FUNCTION ATT_LEN(J,VID)
      IMPLICIT  NONE
      INTEGER J
      INTEGER VID
#include "tests.inc"
      IF (VID .LT. 1) THEN
          ATT_LEN = GATT_LEN(J)
      ELSE
          ATT_LEN = ATTLEN(J,VID)
      ENDIF
      END


!
! Return the minimum value of an internal type.
!
        function internal_min(type)
        implicit        none
        integer         type
	doubleprecision	min_schar
	doubleprecision	min_short
	doubleprecision	min_int
	doubleprecision	min_long
	doubleprecision	max_float
	doubleprecision	max_double
#include "tests.inc"

        if (type .eq. NFT_CHAR) then
            internal_min = 0
        else if (type .eq. NFT_INT1) then
#if NF_INT1_IS_C_SIGNED_CHAR
            internal_min = min_schar()
#endif
#if NF_INT1_IS_C_SHORT
            internal_min = min_short()
#endif
#if NF_INT1_IS_C_INT
            internal_min = min_int()
#endif
#if NF_INT1_IS_C_LONG
            internal_min = min_long()
#endif
        else if (type .eq. NFT_INT2) then
#if NF_INT2_IS_C_SHORT
            internal_min = min_short()
#endif            
#if NF_INT2_IS_C_INT
            internal_min = min_int()
#endif            
#if NF_INT2_IS_C_LONG
            internal_min = min_long()
#endif
        else if (type .eq. NFT_INT) then
#if NF_INT_IS_C_INT
            internal_min = min_int()
#endif            
#if NF_INT_IS_C_LONG
            internal_min = min_long()
#endif
        else if (type .eq. NFT_REAL) then
#if NF_REAL_IS_C_FLOAT
            internal_min = -max_float()
#endif
#if NF_REAL_IS_C_DOUBLE
            internal_min = -max_double()
#endif
        else if (type .eq. NFT_DOUBLE) then
#if NF_DOUBLEPRECISION_IS_C_DOUBLE
            internal_min = -max_double()
#endif
#if NF_DOUBLEPRECISION_IS_C_FLOAT
            internal_min = -max_float()
#endif
        else
            stop 2
        end if
        end


!
! Return the maximum value of an internal type.
!
        function internal_max(type)
        implicit        none
        integer         type
	doubleprecision	max_schar
	doubleprecision	max_short
	doubleprecision	max_int
	doubleprecision	max_long
	doubleprecision	max_float
	doubleprecision	max_double
#include "tests.inc"

        if (type .eq. NFT_CHAR) then
            internal_max = 255
        else if (type .eq. NFT_INT1) then
#if NF_INT1_IS_C_SIGNED_CHAR
            internal_max = max_schar()
#endif
#if NF_INT1_IS_C_SHORT
            internal_max = max_short()
#endif
#if NF_INT1_IS_C_INT
            internal_max = max_int()
#endif
#if NF_INT1_IS_C_LONG
            internal_max = max_long()
#endif
        else if (type .eq. NFT_INT2) then
#if NF_INT2_IS_C_SHORT
            internal_max = max_short()
#endif
#if NF_INT2_IS_C_INT
            internal_max = max_int()
#endif
#if NF_INT2_IS_C_LONG
            internal_max = max_long()
#endif
        else if (type .eq. NFT_INT) then
#if NF_INT_IS_C_INT
            internal_max = max_int()
#endif
#if NF_INT_IS_C_LONG
            internal_max = max_long()
#endif
        else if (type .eq. NFT_REAL) then
#if NF_REAL_IS_C_FLOAT
            internal_max = max_float()
#endif
#if NF_REAL_IS_C_DOUBLE
            internal_max = max_double()
#endif
        else if (type .eq. NFT_DOUBLE) then
#if NF_DOUBLEPRECISION_IS_C_DOUBLE
            internal_max = max_double()
#endif            
#if NF_DOUBLEPRECISION_IS_C_FLOAT
            internal_max = max_float()
#endif
        else
            stop 2
        end if
        end


!
! Return the minimum value of an external type.
!
        function external_min(type)
        implicit        none
        integer         type
#include "tests.inc"

        if (type .eq. NF_BYTE) then
            external_min = X_BYTE_MIN
        else if (type .eq. NF_CHAR) then
            external_min = X_CHAR_MIN
        else if (type .eq. NF_SHORT) then
            external_min = X_SHORT_MIN
        else if (type .eq. NF_INT) then
            external_min = X_INT_MIN
        else if (type .eq. NF_FLOAT) then
            external_min = X_FLOAT_MIN
        else if (type .eq. NF_DOUBLE) then
            external_min = X_DOUBLE_MIN
        else
            stop 2
        end if
        end


!
! Return the maximum value of an internal type.
!
        function external_max(type)
        implicit        none
        integer         type
#include "tests.inc"

        if (type .eq. NF_BYTE) then
            external_max = X_BYTE_MAX
        else if (type .eq. NF_CHAR) then
            external_max = X_CHAR_MAX
        else if (type .eq. NF_SHORT) then
            external_max = X_SHORT_MAX
        else if (type .eq. NF_INT) then
            external_max = X_INT_MAX
        else if (type .eq. NF_FLOAT) then
            external_max = X_FLOAT_MAX
        else if (type .eq. NF_DOUBLE) then
            external_max = X_DOUBLE_MAX
        else
            stop 2
        end if
        end


!
! Indicate whether or not a value lies in the range of an internal type.
!
        function in_internal_range(itype, value)
        implicit        none
        integer         itype
        doubleprecision value
#include "tests.inc"

        in_internal_range = value .ge. internal_min(itype) .and.
     +                      value .le. internal_max(itype)
        end


!
! Return the length of a character variable minus any trailing blanks.
!
        function len_trim(string)
        implicit        none
        character*(*)   string
#include "tests.inc"

        do 1, len_trim = len(string), 1, -1
            if (string(len_trim:len_trim) .ne. ' ')
     +          goto 2
1       continue

2       return
        end
