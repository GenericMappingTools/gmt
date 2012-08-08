!/*********************************************************************
! *   Copyright 1996, UCAR/Unidata
! *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
! *   $Id$
! *********************************************************************/
!!!!
! Do not tabify this unless you like hitting the 72 char limit !!!
!!!
#ifndef UD_TESTS_INC
#define UD_TESTS_INC


!/* The following prevents non-FORTRAN code from appearing in the output. */
#if defined(__osf__)
#   undef _POSIX_SOURCE
#   define _POSIX_SOURCE
#endif

#define NO_NETCDF_2 1

#include "nfconfig.inc"

!/* Parameters of test data */

#ifdef  NF_INT1_T
#   define NF_B 1
#else
#   define NF_B 0
#endif
#ifdef  NF_INT2_T
#   define NF_S 1
#else
#   define NF_S 0
#endif
! Total number of FORTRAN types:
#define NUM_FORTRAN_TYPES (3 + NF_S + NF_B)
#undef NF_B
#undef NF_S

#define NTYPES 6
#define NDIMS 5
#define NVARS 136
#define NRECS 2
#define NGATTS NTYPES
#define RECDIM 1
#define MAX_RANK 3
#define MAX_NELS 64
#define MAX_DIM_LEN 4
#define MAX_NATTS 3


!/*
! * Limits of external types (based on those in ncx.h):
! */
#define X_CHAR_MIN 0
#define X_CHAR_MAX 127
#define X_INT1_MIN (-128)
#define X_INT1_MAX 127
#define X_INT2_MIN (-32768)
#define X_INT2_MAX 32767
#define X_INT_MIN  (-2147483647-1)
#define X_INT_MAX  2147483647
#if 0
#define X_REAL_MAX 3.4028234663852886e+38
#else
#define X_REAL_MAX 3.4028234663852886e+37
#endif
#define X_REAL_MIN (-X_FLOAT_MAX)
#if 0
#define X_DOUBLE_MAX 1.7976931348623157E+308
#else
#define X_DOUBLE_MAX 1.7976931348623157D+200
#endif
#define X_DOUBLE_MIN (-X_DOUBLE_MAX)

#define X_BYTE_MIN X_INT1_MIN
#define X_BYTE_MAX X_INT1_MAX
#define X_SHORT_MIN X_INT2_MIN
#define X_SHORT_MAX X_INT2_MAX
#define X_FLOAT_MIN X_REAL_MIN
#define X_FLOAT_MAX X_REAL_MAX


!/*
! * Examples of invalid argument values:
! */
#define BAD_ID -1
#define BAD_DIMID -1
#define BAD_VARID -2
#define BAD_ATTNUM -1
#define BAD_TYPE 0
#define BAD_FILLMODE -1
#define BAD_NAME 'a/b'


!/*
! * Internal data types:
! */
#define NFT_UNSPECIFIED 0
#define NFT_TEXT 16
#define NFT_CHAR NFT_TEXT
#define NFT_INT1 17
#define NFT_INT2 18
#define NFT_INT 20
#define NFT_REAL 36
#define NFT_DOUBLE 40


!/*
! * Define a macro for trimming trailing blanks from character variables.
! */
#define TRIM(string) string(1:len_trim(string))


!
! FORTRAN GETARG() subroutine:
!
#ifdef __hpux
#   define      getarg  getarg_
#endif


#endif /* UD_TESTS_INC */


#include "netcdf.inc"


!    /* Global variables - filenames */

      CHARACTER*80      testfile        !/* netCDF read-only test data */
      CHARACTER*80      scratch         !/* netCDF test file for writing */

!    /* Global variables - command-line arguments */

      LOGICAL   CREATE_FILE
      LOGICAL   READONLY
      LOGICAL   VERBOSE
      INTEGER   NFAILS
      INTEGER   MAX_NMPT        !/* max num messages per test */

!    /* Global variables - test data */

      CHARACTER*2               DIM_NAME(NDIMS)
      INTEGER                   DIM_LEN(NDIMS)
      CHARACTER*(2+MAX_RANK)    VAR_NAME(NVARS)
      INTEGER                   VAR_TYPE(NVARS)
      INTEGER                   VAR_RANK(NVARS)
      INTEGER                   VAR_DIMID(MAX_RANK,NVARS)
      INTEGER                   VAR_SHAPE(MAX_RANK,NVARS)
      INTEGER                   VAR_NELS(NVARS)
      INTEGER                   VAR_NATTS(NVARS)
      CHARACTER*2               ATTNAME(MAX_NATTS,NVARS)
      CHARACTER*2               GATT_NAME(NGATTS)
      INTEGER                   ATTTYPE(NGATTS,NVARS)
      INTEGER                   GATT_TYPE(NGATTS)
      INTEGER                   ATTLEN(MAX_NATTS,NVARS)
      INTEGER                   GATT_LEN(NGATTS)

!    /* Miscellaneous global variables: */
      CHARACTER*80              PROGNAME        !/* name of the program */
      INTEGER                   NFAILSTOTAL

!    /* Common blocks for global variables: */

      COMMON    /LOGCOM/        CREATE_FILE,
     *                          READONLY,       !/* don't try to change files */
     *                          VERBOSE         !/* print details of tests */

      COMMON    /TXTCOM/        TESTFILE,
     *                          SCRATCH,
     *                          DIM_NAME,
     *                          VAR_NAME,
     *                          ATTNAME,
     *                          GATT_NAME,
     *                          PROGNAME

      COMMON    /INTCOM/        NFAILS,         !/* number of failures in 
     *                                          ! * specific test */
     *                          DIM_LEN,
     *                          VAR_TYPE,
     *                          VAR_RANK,
     *                          VAR_DIMID,
     *                          VAR_SHAPE,
     *                          VAR_NELS,
     *                          VAR_NATTS,
     *                          ATTTYPE,
     *                          GATT_TYPE,
     *                          ATTLEN,
     *                          GATT_LEN,
     *                          MAX_NMPT,
     *                          NFAILSTOTAL


!    /* Functions for accessing attribute test data */
!    /* varid is -1 for NC_GLOBAL so can do global atts in same loop */

!/*      EXTERNAL       ATT_NAME */

      INTEGER           VARID
      INTEGER           NATTS
      CHARACTER*2       ATT_NAME
      INTEGER           ATT_TYPE
      INTEGER           ATT_LEN


      LOGICAL           INRANGE
      LOGICAL           INRANGE_UCHAR
      LOGICAL           INRANGE_FLOAT

      LOGICAL           INRANGE3
      LOGICAL           IN_INTERNAL_RANGE
      LOGICAL           EQUAL
      LOGICAL           INT_VEC_EQ
      INTEGER           ROLL
      INTEGER           INDEX2INDEXES
      INTEGER           INDEX2NCINDEXES
      INTEGER           INDEXES2INDEX
      INTEGER           NC2DBL
      INTEGER           DBL2NC
      INTEGER           LEN_TRIM
      DOUBLEPRECISION   HASH
      DOUBLEPRECISION   HASH4
      DOUBLEPRECISION   HASH_TEXT
      DOUBLEPRECISION   HASH_INT1
      DOUBLEPRECISION   HASH_INT2
      DOUBLEPRECISION   HASH_INT
      DOUBLEPRECISION   HASH_REAL
      DOUBLEPRECISION   HASH_DOUBLE
      DOUBLEPRECISION   INTERNAL_MIN
      DOUBLEPRECISION   INTERNAL_MAX
      DOUBLEPRECISION   EXTERNAL_MIN
      DOUBLEPRECISION   EXTERNAL_MAX
