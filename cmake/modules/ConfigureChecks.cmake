#
# $Id$
#

if(NOT DEFINED _INCLUDED_CHECK_MACROS_)
	set(_INCLUDED_CHECK_MACROS_ "DEFINED")

	include (CheckCCompilerFlag)
	include (CheckCSourceCompiles)
	include (CheckCSourceRuns)
	include (CheckFunctionExists)
	include (CheckIncludeFile)
	include (CheckIncludeFiles)
	include (CheckLibraryExists)
	include (CheckPrototypeExists)
	include (CheckSymbolExists)
	include (CheckTypeExists)
	include (CheckTypeSize)
	include (TestBigEndian)

endif(NOT DEFINED _INCLUDED_CHECK_MACROS_)

#
# Check if compiler supports -traditional-cpp
#

if (MSVC)
	if (NOT HAVE_TRADITIONAL_CPP)
		# Microsoft compiler
		message (STATUS "Performing Test HAVE_TRADITIONAL_CPP")
		execute_process (COMMAND ${CMAKE_C_COMPILER} /EP
			${GMT_SOURCE_DIR}/config.h.cmake # can be any header file
			RESULT_VARIABLE _mscl_ep
			OUTPUT_QUIET ERROR_QUIET)
		if (_mscl_ep EQUAL 0)
			set (HAVE_TRADITIONAL_CPP TRUE CACHE INTERNAL "Test HAVE_TRADITIONAL_CPP")
			message (STATUS "Performing Test HAVE_TRADITIONAL_CPP - Success")
		else (_mscl_ep EQUAL 0)
			set (HAVE_TRADITIONAL_CPP "" CACHE INTERNAL "Test HAVE_TRADITIONAL_CPP")
			message (STATUS "Performing Test HAVE_TRADITIONAL_CPP - Failed")
		endif (_mscl_ep EQUAL 0)
	endif (NOT HAVE_TRADITIONAL_CPP)
elseif (CMAKE_COMPILER_IS_GNUCC OR __COMPILER_GNU)
	# GCC or Clang
	set (CMAKE_REQUIRED_FLAGS "-E -w -P -nostdinc -traditional-cpp")
	check_c_source_compiles ("#define TEST" HAVE_TRADITIONAL_CPP)
	set (CMAKE_REQUIRED_FLAGS)
endif (MSVC)

#
# Check for windows header
#

check_include_file (io.h                HAVE_IO_H_)
check_include_file (direct.h            HAVE_DIRECT_H_)
check_include_file (process.h           HAVE_PROCESS_H_)

#
# Check for C90, C99 and POSIX conformity
#

check_include_file (assert.h            HAVE_ASSERT_H_)
check_include_file (dirent.h            HAVE_DIRENT_H_)
check_include_file (errno.h             HAVE_ERRNO_H_)
check_include_file (fcntl.h             HAVE_FCNTL_H_)
check_include_file (sys/dir.h           HAVE_SYS_DIR_H_)
check_include_file (sys/stat.h          HAVE_STAT_H_)
check_include_file (unistd.h            HAVE_UNISTD_H_)

check_function_exists (fopen64          HAVE_FOPEN64)
check_function_exists (fseeko           HAVE_FSEEKO)
check_function_exists (fseeko64         HAVE_FSEEKO64)
check_function_exists (_fseeki64        HAVE__FSEEKI64)
check_function_exists (ftello           HAVE_FTELLO)
check_function_exists (ftello64         HAVE_FTELLO64)
check_function_exists (_ftelli64        HAVE__FTELLI64)
check_function_exists (getopt           HAVE_GETOPT)
check_function_exists (getpwuid         HAVE_GETPWUID)
check_function_exists (qsort_r          HAVE_QSORT_R)
check_function_exists (qsort_s          HAVE_QSORT_S)
check_function_exists (stricmp          HAVE_STRICMP)
check_function_exists (strdup           HAVE_STRDUP)
check_function_exists (strtod           HAVE_STRTOD)
check_function_exists (strtok_r         HAVE_STRTOK_R)
check_function_exists (strtok_s         HAVE_STRTOK_S)

if (HAVE_UNISTD_H_)
	check_symbol_exists (access  unistd.h HAVE_ACCESS)
else (HAVE_UNISTD_H_)
	# in MinGW:
	check_symbol_exists (access  io.h     HAVE_ACCESS)
endif (HAVE_UNISTD_H_)
check_symbol_exists (_access   io.h     HAVE__ACCESS)
check_symbol_exists (fileno    stdio.h  HAVE_FILENO)
check_symbol_exists (_fileno   stdio.h  HAVE__FILENO)
check_symbol_exists (_getcwd   direct.h HAVE__GETCWD)
if (HAVE_UNISTD_H_)
	check_symbol_exists (getpid  unistd.h  HAVE_GETPID)
elseif (HAVE_PROCESS_H_)
	check_symbol_exists (_getpid process.h HAVE__GETPID)
endif (HAVE_UNISTD_H_)
check_symbol_exists (_mkdir    direct.h HAVE__MKDIR)
check_symbol_exists (_setmode  io.h     HAVE__SETMODE)

#
# Check c types
#

check_include_file (ctype.h             HAVE_CTYPE_H_)
check_include_file (inttypes.h          HAVE_INTTYPES_H_)
check_include_file (machine/endian.h    HAVE_MACHINE_ENDIAN_H_)
#check_include_file (stddef.h            HAVE_STDDEF_H_)
#check_include_file (stdint.h            HAVE_STDINT_H_)
#check_include_file (sys/types.h         HAVE_SYS_TYPES_H_)

# set HAVE_SYS_TYPES_H, HAVE_STDINT_H, and HAVE_STDDEF_H
# and check in <sys/types.h>, <stdint.h>, and <stddef.h>:
check_type_size ("long double"          LONG_DOUBLE)
check_type_size ("long long"            LONG_LONG)
check_type_size (intmax_t               INTMAX_T)
check_type_size (mode_t                 MODE_T)
check_type_size (wchar_t                WCHAR_T)
check_type_size (wint_t                 WINT_T)

# add suffix to prevent name clash with GDAL
set (HAVE_STDDEF_H_ "${HAVE_STDDEF_H}"
	CACHE INTERNAL "Have include stddef.h")
set (HAVE_STDINT_H_ "${HAVE_STDINT_H}"
	CACHE INTERNAL "Have include stdinf.h")
set (HAVE_SYS_TYPES_H_ "${HAVE_SYS_TYPES_H}"
	CACHE INTERNAL "Have include sys/types.h")

test_big_endian (WORDS_BIGENDIAN)

#
# Check math related stuff
#

# check if -lm is needed
check_function_exists (cos HAVE_M_FUNCTIONS)
if (NOT HAVE_M_FUNCTIONS)
	check_library_exists (m cos "" HAVE_M_LIBRARY)
endif (NOT HAVE_M_FUNCTIONS)

# extra math headers

check_include_file (floatingpoint.h     HAVE_FLOATINGPOINT_H_)
check_include_file (ieeefp.h            HAVE_IEEEFP_H_)

set (_math_h math.h float.h)
if (HAVE_FLOATINGPOINT_H_)
	list (APPEND _math_h floatingpoint.h)
endif (HAVE_FLOATINGPOINT_H_)
if (HAVE_IEEEFP_H_)
	list (APPEND _math_h ieeefp.h)
endif (HAVE_IEEEFP_H_)

# sincos is a GNU extension:
set (CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)

# check symbols
check_symbol_exists (acosh       "${_math_h}" HAVE_ACOSH)
check_symbol_exists (alphasincos "${_math_h}" HAVE_ALPHASINCOS)
check_symbol_exists (asinh       "${_math_h}" HAVE_ASINH)
check_symbol_exists (atanh       "${_math_h}" HAVE_ATANH)
check_symbol_exists (copysign    "${_math_h}" HAVE_COPYSIGN)
check_symbol_exists (_copysign   "${_math_h}" HAVE__COPYSIGN)
check_symbol_exists (erf         "${_math_h}" HAVE_ERF)
check_symbol_exists (erfc        "${_math_h}" HAVE_ERFC)
check_symbol_exists (hypot       "${_math_h}" HAVE_HYPOT)
check_symbol_exists (irint       "${_math_h}" HAVE_IRINT)
check_symbol_exists (isnan       "${_math_h}" HAVE_ISNAN)
check_symbol_exists (isnand      "${_math_h}" HAVE_ISNAND)
check_symbol_exists (isnanf      "${_math_h}" HAVE_ISNANF)
check_symbol_exists (_isnan      "${_math_h}" HAVE__ISNAN)
check_symbol_exists (_isnanf     "${_math_h}" HAVE__ISNANF)
check_symbol_exists (j0          "${_math_h}" HAVE_J0)
check_symbol_exists (j1          "${_math_h}" HAVE_J1)
check_symbol_exists (jn          "${_math_h}" HAVE_JN)
check_symbol_exists (log1p       "${_math_h}" HAVE_LOG1P)
check_symbol_exists (log2        "${_math_h}" HAVE_LOG2)
check_symbol_exists (rint        "${_math_h}" HAVE_RINT)
check_symbol_exists (sincos      "${_math_h}" HAVE_SINCOS)
check_symbol_exists (y0          "${_math_h}" HAVE_Y0)
check_symbol_exists (y1          "${_math_h}" HAVE_Y1)
check_symbol_exists (yn          "${_math_h}" HAVE_YN)

# test if sincos is buggy
if (HAVE_SINCOS)
	check_c_source_runs (
		"
		#define _GNU_SOURCE
		include <math.h>
		int main () {
		double s = 0.1, c = 0.2;
		double s1, c1;
		s1 = s; c1 = c;
		sincos (0.5, &s, &c);
		return !(s == s1 || c == c1);} /* return TRUE if sincos works ok */
		"
		HAVE_SINCOS)
endif (HAVE_SINCOS)

set (CMAKE_REQUIRED_DEFINITIONS)

#check_symbol_exists (intptr_t "stdint.h" HAVE_STDINT_H_WITH_INTPTR)
#check_symbol_exists (intptr_t "unistdint.h" HAVE_UNISTD_H_WITH_INTPTR)

#check_function_exists (__argz_count HAVE___ARGZ_COUNT)
#check_function_exists (__argz_next HAVE___ARGZ_NEXT)
#check_function_exists (__argz_stringify HAVE___ARGZ_STRINGIFY)
#check_function_exists (__fsetlocking HAVE___FSETLOCKING)
#check_function_exists (_close HAVE__CLOSE)
#check_function_exists (_dyld_func_lookup HAVE_DYLD)
#check_function_exists (_open HAVE__OPEN)
#check_function_exists (_pclose HAVE__PCLOSE)
#check_function_exists (_popen HAVE__POPEN)
#check_function_exists (argz_append HAVE_ARGZ_APPEND)
#check_function_exists (argz_create_sep HAVE_ARGZ_CREATE_SEP )
#check_function_exists (argz_insert HAVE_ARGZ_INSERT )
#check_function_exists (argz_next HAVE_ARGZ_NEXT)
#check_function_exists (argz_stringify HAVE_ARGZ_STRINGIFY )
#check_function_exists (chmod HAVE_CHMOD)
#check_function_exists (clock_gettime HAVE_CLOCK_GETTIME)
#check_function_exists (close HAVE_CLOSE)
#check_function_exists (dcgettext HAVE_DCGETTEXT)
#check_function_exists (dladdr HAVE_DLADDR)
#check_function_exists (dlerror HAVE_DLERROR)
#check_function_exists (fcntl HAVE_FCNTL)
#check_function_exists (getcwd HAVE_GETCWD)
#check_function_exists (getegid HAVE_GETEGID)
#check_function_exists (getgid HAVE_GETGID)
#check_function_exists (gethrtime HAVE_GETHRTIME)
#check_function_exists (getpagesize HAVE_GETPAGESIZE)
#check_function_exists (gettext HAVE_GETTEXT)
#check_function_exists (getuid HAVE_GETUID)
#check_function_exists (getwd HAVE_GETWD)
#check_function_exists (index HAVE_INDEX)
#check_function_exists (mach_absolute_time HAVE_MACH_ABSOLUTE_TIME)
#check_function_exists (mempcpy HAVE_MEMPCPY)
#check_function_exists (mkfifo HAVE_MKFIFO)
#check_function_exists (mkstemp HAVE_MKSTEMP)
#check_function_exists (mktemp HAVE_MKTEMP)
#check_function_exists (mktime HAVE_MKTIME)
#check_function_exists (mmap HAVE_MMAP)
#check_function_exists (open HAVE_OPEN)
#check_function_exists (opendir HAVE_OPENDIR)
#check_function_exists (pclose HAVE_PCLOSE)
#check_function_exists (popen HAVE_POPEN)
#check_function_exists (putenv HAVE_PUTENV)
#check_function_exists (read_real_time HAVE_READ_REAL_TIME)
#check_function_exists (readdir HAVE_READDIR)
#check_function_exists (setlocale HAVE_SETLOCALE)
#check_function_exists (shl_load HAVE_SHL_LOAD)
#check_function_exists (stpcpy HAVE_STPCPY)
#check_function_exists (strcasecmp HAVE_STRCASECMP)
#check_function_exists (strcoll HAVE_STRCOLL)
#check_function_exists (strcspn HAVE_STRCSPN)
#check_function_exists (strcspn HAVE_STRERROR)
#check_function_exists (strdup HAVE_STRDUP)
#check_function_exists (strerror HAVE_STRERROR)
#check_function_exists (strftime HAVE_STRFTIME)
#check_function_exists (strlcat HAVE_STRLCAT)
#check_function_exists (strlcpy HAVE_STRLCPY)
#check_function_exists (strspn HAVE_STRSPN)
#check_function_exists (strstr HAVE_STRSTR)
#check_function_exists (strtod HAVE_STRTOD)
#check_function_exists (strtol HAVE_STRTOL)
#check_function_exists (strtoul HAVE_STRTOUL)
#check_function_exists (time_base_to_time HAVE_TIME_BASE_TO_TIME)
#check_function_exists (tsearch HAVE_TSEARCH)
#check_function_exists (vprintf HAVE_VPRINTF )
#check_function_exists (wcslen HAVE_WCSLEN)
#check_include_file (argz.h HAVE_ARGZ_H)
#check_include_file (c_asm.h HAVE_C_ASM_H)
#check_include_file (dl.h HAVE_DL_H)
#check_include_file (dlfcn.h HAVE_DLADDR  )
#check_include_file (dlfcn.h HAVE_DLFCN_H)
#check_include_file (ffi.h HAVE_FFI_H)
#check_include_file (intrinsics.h HAVE_INTRINSICS_H)
#check_include_file (sys/time.h have_hrtime_t)
#check_include_files ( mach-o/dyld.h HAVE_MACH_O_DYLD_H  )
#check_include_files ( mach/mach_time.h  HAVE_MACH_MACH_TIME_H)
#check_include_files (argz.h HAVE_ARGZ_H)
#check_include_files (io.h HAVE_IO_H)
#check_include_files (limits.h HAVE_LIMITS_H)
#check_include_files (locale.h HAVE_LOCALE_H)
#check_include_files (memory.h HAVE_MEMORY_H)
#check_include_files (ndir.h HAVE_NDIR_H)
#check_include_files (pthread.h HAVE_PTHREAD_H)
#check_include_files (stdlib.h HAVE_STDLIB_H)
#check_include_files (string.h HAVE_STRING_H)
#check_include_files (strings.h HAVE_STRINGS_H)
#check_include_files (sys/dl.h HAVE_SYS_DL_H )
#check_include_files (sys/socket.h HAVE_SYS_SOCKET_H)
#check_include_files (sys/stat.h HAVE_SYS_STAT_H )
#check_include_files (sys/stat.h HAVE_SYS_STAT_H)
#check_include_files (sys/time.h HAVE_SYS_TIME_H )
#check_include_files (sys/time.h HAVE_SYS_TIME_H)
#check_include_files (sys/types.h HAVE_SYS_TYPES_H )
#check_include_files (sys/types.h HAVE_SYS_TYPES_H)
#check_include_files (sys/utime.h HAVE_SYS_UTIME_H)
#check_include_files (utime.h HAVE_UTIME_H)
#check_library_exists (dl dl "/lib;/usr/lib;/usr/local/lib;/usr/pkg/lib" HAVE_LIBDL)
#check_symbol_exists (LC_MESSAGES "locale.h" HAVE_LC_MESSAGES)
#check_symbol_exists (asprintf "stdio.h" HAVE_ASPRINTF)
#check_symbol_exists (intmax_t "inttypes.h" HAVE_INTTYPES_H_WITH_UINTMAX)
#check_symbol_exists (pid_t "sys/types.h" HAVE_PID_T)
#check_symbol_exists (printf "stdio.h" HAVE_POSIX_PRINTF)
#check_symbol_exists (snprintf "stdio.h" HAVE_SNPRINTF)
#check_symbol_exists (uintmax_t "stdint.h" HAVE_STDINT_H_WITH_UINTMAX)
#check_symbol_exists (wprintf "stdio.h" HAVE_WPRINTF)
#check_type_size ("long double"         SIZEOF_LONG_DOUBLE)
#check_type_size ("long long" SIZEOF_LONG_LONG)
#check_type_size ("void*" SIZEOF_VOID_P)
#check_type_size (char           SIZEOF_CHAR)
#check_type_size (double         SIZEOF_DOUBLE)
#check_type_size (float         SIZEOF_FLOAT)
#check_type_size (int            SIZEOF_INT)
#check_type_size (long           SIZEOF_LONG) 
#check_type_size (short          SIZEOF_SHORT)

if (NOT DEFINED STDC_HEADERS)
	message (STATUS "Checking whether system has ANSI C header files")
	check_include_files ("stdlib.h;stdarg.h;string.h;float.h" StandardHeadersExist)
	if (StandardHeadersExist)
		check_prototype_exists (memchr string.h memchrExists)
		if (memchrExists)
			check_prototype_exists (free stdlib.h freeExists)
			if (freeExists)
				#include (TestForHighBitCharacters)
				#if (CMAKE_HIGH_BIT_CHARACTERS)
				message (STATUS "ANSI C header files - found")
				set (STDC_HEADERS 1 CACHE INTERNAL "System has ANSI C header files")
				#endif (CMAKE_HIGH_BIT_CHARACTERS)
			endif (freeExists)
		endif (memchrExists)
	endif (StandardHeadersExist)
	if (NOT STDC_HEADERS)
		message (STATUS "ANSI C header files - not found")
		set (STDC_HEADERS 0 CACHE INTERNAL "System has ANSI C header files")
	endif (NOT STDC_HEADERS)
endif (NOT DEFINED STDC_HEADERS)

# Define to 1 if you can safely include both <sys/time.h> and <time.h>
#if (HAVE_SYS_TIME_H)
#   check_include_files ("sys/time.h;time.h" TIME_WITH_SYS_TIME)
#else (HAVE_SYS_TIME_H)
#   set (TIME_WITH_SYS_TIME 0)
#endif (HAVE_SYS_TIME_H)

# Define to 1 if your <sys/time.h> declares `struct tm'. */
#check_type_exists ("struct tm" sys/time.h TM_IN_SYS_TIME)

#check_cxx_source_compiles (
#   "
#   #include <algorithm>
#   using std::count;
#   int countChar(char * b, char * e, char const c)
#   {
#       return count (b, e, c);
#   }
#   int main (){return 0;}
#   "
#HAVE_STD_COUNT)

#check_cxx_source_compiles (
#   "
#   #include <cctype>
#   using std::tolower;
#   int main (){return 0;}
#   "
#CXX_GLOBAL_CSTD)

#   check_c_source_compiles (
#     "
#     #include <iconv.h>
#     // this declaration will fail when there already exists a non const char** version which returns size_t
#     double iconv(iconv_t cd,  char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);
#     int main () { return 0; }
#     "
#   HAVE_ICONV_CONST)
#
#   check_c_source_compiles (
#     "
#     #include <stdlib.h>
#     int i[ ( sizeof(wchar_t)==2 ? 1 : -1 ) ];
#     int main (){return 0;}
#     "
#   SIZEOF_WCHAR_T_IS_2)
#
#   check_c_source_compiles (
#     "
#     #include <stdlib.h>
#     int i[ ( sizeof(wchar_t)==4 ? 1 : -1 ) ];
#     int main (){return 0;}
#     "
#   SIZEOF_WCHAR_T_IS_4)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
