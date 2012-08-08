/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* What sort of HTTP client is this? */
/* #undef CNAME */

/* Should the subprocess compression for Server3 be includes? */
/* #undef COMPRESSION_FOR_SERVER3 */

#define DEFAULT_CHUNK_SIZE 4194304

/* num chunks in default per-var chunk cache. */
#define DEFAULT_CHUNKS_IN_CACHE 10

/* max size of the default per-var chunk cache. */
#define MAX_DEFAULT_CACHE_SIZE 67108864

/* default chunk cache nelems. */
#define CHUNK_CACHE_NELEMS 1000
//#define CHUNK_CACHE_NELEMS 1009

/* default chunk cache preemption policy. */
#define CHUNK_CACHE_PREEMPTION 0.75

/* default chunk cache size in bytes. */
#define CHUNK_CACHE_SIZE 32000000
//#define CHUNK_CACHE_SIZE 4194304

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* Client version number */
/* #undef CVER */

/* Define to 1 if using `alloca.c'. */
/* #undef C_ALLOCA */

/* What DAP version is supported? */
/* #undef DAP_PROTOCOL_VERSION */

/* dbyte */
/* #undef DBYTE */

/* dfloat32 */
/* #undef DFLOAT32 */

/* dfloat64 */
/* #undef DFLOAT64 */

/* dint16 */
/* #undef DINT16 */

/* int32 */
/* #undef DINT32 */

/* set this only when building a DLL under MinGW */
/* #undef DLL_NETCDF */
#define DLL_NETCDF 1

/* uint16 */
/* #undef DUINT16 */

/* uint32 */
/* #undef DUINT32 */

/* Client name and version combined */
/* #undef DVR */

/* Should all the classes run ConstraintEvaluator::eval()? */
/* #undef EVAL */

/* if true, run extra tests which may not work with HDF5 beta release */
/* #undef EXTRA_TESTS */

/* use HDF5 1.6 API */
#define H5_USE_16_API 1

/* Define to 1 if you have the `alarm' function. */
/* #undef HAVE_ALARM */

/* Define to 1 if you have `alloca', as a function or macro. */
/*#define HAVE_ALLOCA 1*/

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
/*#define HAVE_ALLOCA_H 1*/

/* Define to 1 if you have the `atexit' function. */
/* #undef HAVE_ATEXIT */

/* Define to 1 if you have the `bzero' function. */
/* #undef HAVE_BZERO */

/* Define to 1 if you have the declaration of `isfinite', and to 0 if you
   don't. */
/*#define HAVE_DECL_ISFINITE 1*/

/* Define to 1 if you have the declaration of `isinf', and to 0 if you don't.
   */
/*#define HAVE_DECL_ISINF 1*/

/* Define to 1 if you have the declaration of `isnan', and to 0 if you don't.
   */
/*#define HAVE_DECL_ISNAN 1*/

/* Define to 1 if you have the declaration of `signbit', and to 0 if you
   don't. */
#define HAVE_DECL_SIGNBIT 1

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_DIRENT_H */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the `dup2' function. */
/* #undef HAVE_DUP2 */

/* Define to 1 if you have the <fcntl.h> header file. */
/* #undef HAVE_FCNTL_H */

/* Define to 1 if you have the <float.h> header file. */
/* #undef HAVE_FLOAT_H */

/* Define to 1 if you have the `floor' function. */
/* #undef HAVE_FLOOR */

/* Define to 1 if you have the `getcwd' function. */
/* #undef HAVE_GETCWD */

/* Define to 1 if you have the `getpagesize' function. */
/* #undef HAVE_GETPAGESIZE */

/* Define to 1 if you have the <hdf5.h> header file. */
#define HAVE_HDF5_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `hdf5' library (-lhdf5). */
#define HAVE_LIBHDF5 1

/* Define to 1 if you have the `hdf5_hl' library (-lhdf5_hl). */
#define HAVE_LIBHDF5_HL 1

/* Define to 1 if you have the <limits.h> header file. */
/* #undef HAVE_LIMITS_H */

/* Define to 1 if you have the `localtime_r' function. */
/* #undef HAVE_LOCALTIME_R */

/* Define to 1 if the system has the type `long long int'. */
#define HAVE_LONG_LONG_INT 1

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef HAVE_MALLOC_H */

/* Define to 1 if you have the `memchr' function. */
/* #undef HAVE_MEMCHR */

/* Define to 1 if you have the `memmove' function. */
/* #undef HAVE_MEMMOVE */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
/* #undef HAVE_MEMSET */

/* Define to 1 if you have the `mktime' function. */
/* #undef HAVE_MKTIME */

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netinet/in.h> header file. */
/* #undef HAVE_NETINET_IN_H */

/* Define to 1 if you have the `pow' function. */
/* #undef HAVE_POW */

/* Define to 1 if the system has the type `ptrdiff_t'. */
#define HAVE_PTRDIFF_T 1

/* Define to 1 if you have the `putenv' function. */
/* #undef HAVE_PUTENV */

/* Define to 1 if you have the `regcomp' function. */
/* #undef HAVE_REGCOMP */

/* Define to 1 if you have the `setenv' function. */
/* #undef HAVE_SETENV */

/* Define to 1 if the system has the type `ssize_t'. */
/*#define HAVE_SSIZE_T 1*/

/* Define to 1 if you have the <sstream> header file. */
/* #undef HAVE_SSTREAM */

/* Define to 1 if stdbool.h conforms to C99. */
/*#define HAVE_STDBOOL_H 1*/

/* Define to 1 if you have the <stddef.h> header file. */
/* #undef HAVE_STDDEF_H */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strchr' function. */
/* #undef HAVE_STRCHR */

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcat' function. */
/*#define HAVE_STRLCAT 1*/

/* Define to 1 if you have the `strrchr' function. */
/* #undef HAVE_STRRCHR */

/* Define to 1 if you have the `strstr' function. */
/* #undef HAVE_STRSTR */

/* Define to 1 if you have the `strtol' function. */
/* #undef HAVE_STRTOL */

/* Define to 1 if you have the `strtoul' function. */
/* #undef HAVE_STRTOUL */

/* Define to 1 if `st_blksize' is member of `struct stat'. */
/*#define HAVE_STRUCT_STAT_ST_BLKSIZE 1*/

/* Define to 1 if your `struct stat' has `st_blksize'. Deprecated, use
   `HAVE_STRUCT_STAT_ST_BLKSIZE' instead. */
/*#define HAVE_ST_BLKSIZE 1*/

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
/* #undef HAVE_SYS_PARAM_H */

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
/* #undef HAVE_SYS_TIME_H */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
/* #undef HAVE_SYS_WAIT_H */

/* Define to 1 if you have the `timegm' function. */
/* #undef HAVE_TIMEGM */

/* Define to 1 if the system has the type `uchar'. */
/* #undef HAVE_UCHAR */

/* Define to 1 if you have the <unistd.h> header file. */
/*#define HAVE_UNISTD_H 1*/

/* Added to fool ncgen. */
#define YY_NO_UNISTD_H 1

/* Define to 1 if the system has the type `unsigned long long int'. */
#define HAVE_UNSIGNED_LONG_LONG_INT 1

/* Define to 1 if you have the `vprintf' function. */
/* #undef HAVE_VPRINTF */

/* Define to 1 if the system has the type `_Bool'. */
/*#define HAVE__BOOL 1*/

/* if true, use HDF5 data conversions */
/* #undef HDF5_CONVERT */

/* do large file tests */
/* #undef LARGE_FILE_TESTS */

/* Set to the prefix directory */
/* #undef LIBDAP_ROOT */

/* if true, turn on netCDF-4 logging */
/* #undef LOGGING */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* type definition */
#define NCBYTE_T byte

/* type definition */
#define NCSHORT_T integer*2

/* default */
#define NF_DOUBLEPRECISION_IS_C_DOUBLE 1

/* default */
#define NF_INT1_IS_C_SIGNED_CHAR 1

/* type thing */
#define NF_INT1_T byte

/* default */
#define NF_INT2_IS_C_SHORT 1

/* type thing */
#define NF_INT2_T integer*2

/* default */
#define NF_INT_IS_C_INT 1
//#define NF_INT_IS_C_LONG 1

/* default */
#define NF_REAL_IS_C_FLOAT 1

/* no IEEE float on this platform */
/* #undef NO_IEEE_FLOAT */

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* do not build the netCDF version 2 API */
/* #undef NO_NETCDF_2 */
//#define NO_NETCDF_2 1

/* no stdlib.h */
/* #undef NO_STDLIB_H */

/* no sys_types.h */
/* #undef NO_SYS_TYPES_H */

/* Name of package */
#define PACKAGE "netcdf"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "support@unidata.ucar.edu"

/* Define to the full name of this package. */
#define PACKAGE_NAME "netCDF"

/* Define to the version of this package. */
#define PACKAGE_VERSION "@PACKAGE_VERSION@"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "netCDF @PACKAGE_VERSION@"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "netcdf"

/* Define as the return type of signal handlers (`int' or `void'). */
/* #undef RETSIGTYPE */

/* The size of `char', as computed by sizeof. */
/* #undef SIZEOF_CHAR */

/* The size of `double', as computed by sizeof. */
#define SIZEOF_DOUBLE 8

/* The size of `float', as computed by sizeof. */
#define SIZEOF_FLOAT 4

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `int16_t', as computed by sizeof. */
/* #undef SIZEOF_INT16_T */

/* The size of `int32_t', as computed by sizeof. */
/* #undef SIZEOF_INT32_T */

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `off_t', as computed by sizeof. */
#define SIZEOF_OFF_T 8

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* The size of `size_t', as computed by sizeof. */
#ifdef _WIN64
typedef unsigned __int64 size_t;
typedef __int64 ssize_t;
#define SIZEOF_SIZE_T 8
#else
typedef unsigned int size_t;
typedef int ssize_t;
#define SIZEOF_SIZE_T 4
#endif

/* The size of `uint16_t', as computed by sizeof. */
/* #undef SIZEOF_UINT16_T */

/* The size of `uint32_t', as computed by sizeof. */
/* #undef SIZEOF_UINT32_T */

/* The size of `uint8_t', as computed by sizeof. */
/* #undef SIZEOF_UINT8_T */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Place to put very large netCDF test files. */
#define TEMP_LARGE "."

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
/* #undef TIME_WITH_SYS_TIME */

/* if true, build DAP Client */
#define USE_DAP 1

/* if true, build DAP Client */
#define ENABLE_DAP 1

/* if true, do remote tests */
#define ENABLE_DAP_REMOTE_TESTS 1

/* set this to use extreme numbers in tests */
#define USE_EXTREME_NUMBERS 1

/* if true, build netCDF-4 */
#define USE_NETCDF4 1

#define HAVE_GETOPT_H 1

/* if true, parallel netCDF-4 is in use */
/* #undef USE_PARALLEL */

/* if true, compile in parallel netCDF-4 based on MPI/IO */
/* #undef USE_PARALLEL_MPIO */

/* if true, compile in parallel netCDF-4 based on MPI/POSIX */
/* #undef USE_PARALLEL_POSIX */

/* if true, build libsrc code with renamed API functions */
#define USE_RENAMEV3 1

/* if true, compile in zlib compression in netCDF-4 variables */
#define USE_ZLIB 1

/* Version number of package */
//#define VERSION "4.1.3"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel and VAX). */
#if defined __BIG_ENDIAN__
# define WORDS_BIGENDIAN 1
#elif ! defined __LITTLE_ENDIAN__
/* # undef WORDS_BIGENDIAN */
#endif

/* xdr float32 */
/* #undef XDR_FLOAT32 */

/* xdr float64 */
/* #undef XDR_FLOAT64 */

/* xdr int16 */
/* #undef XDR_INT16 */

/* xdr int32 */
/* #undef XDR_INT32 */

/* xdr uint16 */
/* #undef XDR_UINT16 */

/* xdr uint32 */
/* #undef XDR_UINT32 */

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Needed by HPUX with c89 compiler. */
/* #undef _HPUX_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* # undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Turned on by netCDF configure. */
/* #undef f2cFortran */

/* Turned on by netCDF configure. */
/* #undef gFortran */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Turned on by netCDF configure. */
#define pgiFortran 1

/* #define INTEL_COMPILER 1 */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
/* #undef volatile */

/* Shorthand for gcc's unused attribute feature */
#if defined(__GNUG__) || defined(__GNUC__)
#define not_used __attribute__ ((unused))
#else
#define not_used 
#endif /* __GNUG__ || __GNUC__ */

/* I added the following to this config.h file by hand, after being abducted by 
   aliens last week in Kansas. (All Hail Zorlock, Mighty Destroyer of Worlds!) */
#include <io.h>
#include <process.h>
#include "nc.h"
#define isnan _isnan
#define lseek _lseeki64
#define off_t __int64
#define _off_t __int64
#define _OFF_T_DEFINED
#define snprintf sprintf_s
#define strcasecmp _stricmp
#define strtoll _strtoi64
#define HAVE_STRDUP 1
#define HAVE_RPC_XDR_H 1
#define HAVE_RPC_TYPES_H 1
