  !
  ! external netcdf data types:
  !
  integer, parameter, public :: &
    nf90_byte   = 1,            &
    nf90_int1   = nf90_byte,    &
    nf90_char   = 2,            &
    nf90_short  = 3,            &
    nf90_int2   = nf90_short,   &
    nf90_int    = 4,            &
    nf90_int4   = nf90_int,     &
    nf90_float  = 5,            &
    nf90_real   = nf90_float,   &
    nf90_real4  = nf90_float,   &
    nf90_double = 6,            &
    nf90_real8  = nf90_double
                        
  !
  ! default fill values:
  !
  character (len = 1),           parameter, public :: &
    nf90_fill_char  = achar(0)
  integer (kind =  OneByteInt),  parameter, public :: &
    nf90_fill_byte  = -127,                           &
    nf90_fill_int1  = nf90_fill_byte
  integer (kind =  TwoByteInt),  parameter, public :: &
    nf90_fill_short = -32767,                         &
    nf90_fill_int2  = nf90_fill_short
  integer (kind = FourByteInt),  parameter, public :: &
    nf90_fill_int   = -2147483647
  real   (kind =  FourByteReal), parameter, public :: &
    nf90_fill_float = 9.9692099683868690e+36,         &
    nf90_fill_real  = nf90_fill_float,                &
    nf90_fill_real4 = nf90_fill_float
  real   (kind = EightByteReal), parameter, public :: &
    nf90_fill_double = 9.9692099683868690e+36,        &
    nf90_fill_real8  = nf90_fill_double

  !
  ! mode flags for opening and creating a netcdf dataset:
  !
  integer, parameter, public :: &
    nf90_nowrite   = 0,         &
    nf90_write     = 1,         &
    nf90_clobber   = 0,         &
    nf90_noclobber = 4,         &
    nf90_fill      = 0,         &
    nf90_nofill    = 256,       &
    nf90_64bit_offset    = 512,       &
    nf90_lock      = 1024,      &
    nf90_share     = 2048 
  
  integer, parameter, public ::  &
    nf90_sizehint_default = 0,   & 
    nf90_align_chunk      = -1 

  !
  ! size argument for defining an unlimited dimension:
  !
  integer, parameter, public :: nf90_unlimited = 0

  !
  ! global attribute id:
  !
  integer, parameter, public :: nf90_global = 0

  !
  ! implementation limits:
  !
  integer, parameter, public :: &
    nf90_max_dims     = 1024,    &
    nf90_max_attrs    = 8192,   &
    nf90_max_vars     = 8192,   &
    nf90_max_name     = 256,    &
    nf90_max_var_dims = 1024
  
  !
  ! error codes:
  !
  integer, parameter, public :: &
    nf90_noerr        = 0,      &
    nf90_ebadid       = -33,    &
    nf90_eexist       = -35,    &
    nf90_einval       = -36,    &
    nf90_eperm        = -37,    &
    nf90_enotindefine = -38,    &
    nf90_eindefine    = -39,    &
    nf90_einvalcoords = -40,    &
    nf90_emaxdims     = -41,    &
    nf90_enameinuse   = -42,    &
    nf90_enotatt      = -43,    &
    nf90_emaxatts     = -44,    &
    nf90_ebadtype     = -45,    &
    nf90_ebaddim      = -46,    &
    nf90_eunlimpos    = -47,    &
    nf90_emaxvars     = -48,    &
    nf90_enotvar      = -49,    &
    nf90_eglobal      = -50,    &
    nf90_enotnc       = -51,    &
    nf90_ests         = -52,    &
    nf90_emaxname     = -53,    &
    nf90_eunlimit     = -54,    &
    nf90_enorecvars   = -55,    &
    nf90_echar        = -56,    &
    nf90_eedge        = -57,    &
    nf90_estride      = -58,    &
    nf90_ebadname     = -59,    &
    nf90_erange       = -60,    &
    nf90_enomem       = -61,    &
    nf90_evarsize     = -62,    &
    nf90_edimsize     = -63,    &
    nf90_etrunc       = -64

  !
  ! error handling modes:
  !
  integer, parameter, public :: &
    nf90_fatal   = 1,           &
    nf90_verbose = 2

  !
  ! format version numbers:
  !
  integer, parameter, public :: &
    nf90_format_classic = 1,    &
    nf90_format_64bit = 2,      &
    nf90_format_netcdf4 = 3,    &
    nf90_format_netcdf4_classic = 4
