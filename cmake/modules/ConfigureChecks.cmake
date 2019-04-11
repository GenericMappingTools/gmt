#
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
	include (CMakePushCheckState)
	include (TestBigEndian)

endif(NOT DEFINED _INCLUDED_CHECK_MACROS_)

#
# Check if compiler supports -traditional-cpp
#

if (NOT HAVE_TRADITIONAL_CPP)
	if (MSVC)
		# Visual C++
		set (_cpp_cmdline /EP)
	elseif (CMAKE_C_COMPILER_ID MATCHES "(GNU|Clang|Intel)")
		# GCC, Clang, or ICC
		set (_cpp_cmdline -E -w -P -nostdinc -traditional-cpp)
	endif (MSVC)
	message (STATUS "Performing Test HAVE_TRADITIONAL_CPP")
	execute_process (COMMAND ${CMAKE_C_COMPILER} ${_cpp_cmdline}
		${GMT_SOURCE_DIR}/config.h.in # can be any header file
		RESULT_VARIABLE _cpp_traditional_result
		OUTPUT_QUIET ERROR_QUIET)
	if (_cpp_traditional_result EQUAL 0)
		set (HAVE_TRADITIONAL_CPP TRUE CACHE INTERNAL "Test HAVE_TRADITIONAL_CPP")
		message (STATUS "Performing Test HAVE_TRADITIONAL_CPP - Success")
	else (_cpp_traditional_result EQUAL 0)
		set (HAVE_TRADITIONAL_CPP "" CACHE INTERNAL "Test HAVE_TRADITIONAL_CPP")
		message (STATUS "Performing Test HAVE_TRADITIONAL_CPP - Failed")
	endif (_cpp_traditional_result EQUAL 0)
endif (NOT HAVE_TRADITIONAL_CPP)

#
# Check if compiler supports __func__ or __FUNCTION__ identifier
#

check_c_source_compiles (
	"
	int main (){char *function_name = __func__; return 0;}
	"
	HAVE___FUNC__)
check_c_source_compiles (
	"
	int main (){char *function_name = __FUNCTION__; return 0;}
	"
	HAVE___FUNCTION__)


#
# Check if compiler supports inline functions
# This test is adapted from Jack Kelly on the CMake mailing list
#

cmake_push_check_state() # save state of CMAKE_REQUIRED_*
foreach (KEYWORD "inline" "__inline" "__inline__")
	if (NOT DEFINED HAVE_C_INLINE)
		set (CMAKE_REQUIRED_DEFINITIONS -Dinline=${KEYWORD})
		check_c_source_compiles(
			"
			typedef int foo_t;
			static inline foo_t static_foo () {return 0;}
			foo_t foo () {return 0;}
			int main (int argc, char *argv[]) {return 0;}
			"
			HAVE_C_${KEYWORD})
		if (HAVE_C_${KEYWORD})
			set (HAVE_C_INLINE TRUE)
		endif (HAVE_C_${KEYWORD})
	endif (NOT DEFINED HAVE_C_INLINE)
endforeach (KEYWORD)
cmake_pop_check_state() # restore state of CMAKE_REQUIRED_*

#
# Check for windows header
#

check_include_file (io.h                HAVE_IO_H_)
check_include_file (direct.h            HAVE_DIRECT_H_)
check_include_file (process.h           HAVE_PROCESS_H_)

#
# Check for C99 and libc extensions
#

# strdup, sincos, ... are GNU/BSD/Sun extensions:
cmake_push_check_state()
set (CMAKE_REQUIRED_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
	-D_GNU_SOURCE
	-D__EXTENSIONS__
	-D_LARGEFILE_SOURCE
	-D_LARGEFILE64_SOURCE)

check_include_file (assert.h            HAVE_ASSERT_H_)
check_include_file (dirent.h            HAVE_DIRENT_H_)
check_include_file (errno.h             HAVE_ERRNO_H_)
check_include_file (execinfo.h          HAVE_EXECINFO_H_)
check_include_file (fcntl.h             HAVE_FCNTL_H_)
check_include_file (signal.h            HAVE_SIGNAL_H_)
check_include_file (stdbool.h           HAVE_STDBOOL_H_)
check_include_file (sys/dir.h           HAVE_SYS_DIR_H_)
check_include_file (sys/resource.h      HAVE_SYS_RESOURCE_H_)
check_include_file (sys/stat.h          HAVE_SYS_STAT_H_)
check_include_file (sys/time.h          HAVE_SYS_TIME_H_)
check_include_file (sys/ucontext.h      HAVE_SYS_UCONTEXT_H_)
check_include_file (unistd.h            HAVE_UNISTD_H_)

check_function_exists (fcntl            HAVE_FCNTL)
check_function_exists (fopen64          HAVE_FOPEN64)
check_function_exists (fseeko           HAVE_FSEEKO)
check_function_exists (ftello           HAVE_FTELLO)
check_function_exists (getpwuid         HAVE_GETPWUID)
check_function_exists (abs              HAVE_ABS)
check_function_exists (llabs            HAVE_LLABS)
check_function_exists (pclose           HAVE_PCLOSE)
check_function_exists (popen            HAVE_POPEN)
check_function_exists (qsort_r          HAVE_QSORT_R)
if (HAVE_QSORT_R)
	# check qsort_r compatibility
	check_c_source_runs (
		"
		#include <stdlib.h>
		#include <assert.h>
		int cmp(const void *a, const void*b, void *c) {
		assert(c == NULL);
		return *(int*)a - *(int*)b;
		}
		int main() {
		int array[5] = {7,3,5,2,8};
		int i;
		qsort_r(array,5,sizeof(int),cmp,NULL);
		for (i=0;i<4;++i) {
		assert(array[i] < array[i+1]);
		}
		return 0;
		}
		"
		HAVE_QSORT_R_GLIBC)
endif (HAVE_QSORT_R)
check_function_exists (strcasecmp       HAVE_STRCASECMP)
check_function_exists (strncasecmp      HAVE_STRNCASECMP)
check_function_exists (stricmp          HAVE_STRICMP)
check_function_exists (strnicmp         HAVE_STRNICMP)
check_function_exists (strdup           HAVE_STRDUP)
check_function_exists (strndup          HAVE_STRNDUP)
check_function_exists (strsep           HAVE_STRSEP)
check_function_exists (strtod           HAVE_STRTOD)
# Note: trailing underscore = GDAL workaround
check_function_exists (strtof           HAVE_STRTOF_)
check_function_exists (strtok_r         HAVE_STRTOK_R)

if (WIN32)
	check_function_exists (_fseeki64      HAVE__FSEEKI64)
	check_function_exists (_ftelli64      HAVE__FTELLI64)
	check_function_exists (_pclose        HAVE__PCLOSE)
	check_function_exists (_popen         HAVE__POPEN)
	check_function_exists (_stat          HAVE__STAT)
	check_function_exists (_stati64       HAVE__STATI64)
	check_function_exists (_fstat         HAVE__FSTAT)
	check_function_exists (_fstati64      HAVE__FSTATI64)
	check_function_exists (strtok_s       HAVE_STRTOK_S)
endif (WIN32)

# Check if these functions are declared (might not be the case although they
# are built-in)
check_symbol_exists (strdup    string.h DECLARED_STRDUP)
check_symbol_exists (strsep    string.h DECLARED_STRSEP)

check_symbol_exists (basename  libgen.h HAVE_BASENAME)
check_symbol_exists (fileno    stdio.h  HAVE_FILENO)
check_symbol_exists (setlocale locale.h HAVE_SETLOCALE)
# Note: trailing underscore = GDAL workaround
check_symbol_exists (snprintf  stdio.h  HAVE_SNPRINTF_)
check_symbol_exists (vsnprintf stdio.h  HAVE_VSNPRINTF_)

if (HAVE_UNISTD_H_)
	check_symbol_exists (access  unistd.h HAVE_ACCESS)
	check_symbol_exists (getpid  unistd.h HAVE_GETPID)
else (HAVE_UNISTD_H_)
	# in MinGW:
	check_symbol_exists (access  io.h     HAVE_ACCESS)
	check_symbol_exists (_getpid process.h HAVE__GETPID)
endif (HAVE_UNISTD_H_)

if (WIN32)
	check_symbol_exists (_access   io.h     HAVE__ACCESS)
	check_symbol_exists (_fileno   stdio.h  HAVE__FILENO)
	check_symbol_exists (_getcwd   direct.h HAVE__GETCWD)
	check_symbol_exists (_mkdir    direct.h HAVE__MKDIR)
	check_symbol_exists (_setmode  io.h     HAVE__SETMODE)
	check_symbol_exists (_snprintf stdio.h  HAVE__SNPRINTF_)
	check_symbol_exists (_vsnprintf stdio.h HAVE__VSNPRINTF_)
endif (WIN32)

if (UNIX)
	# Check if -ldl is needed for dladdr
	check_function_exists (dlopen HAVE_BUILTIN_DYNAMIC_LINKING_LOADER)
	if (NOT HAVE_BUILTIN_DYNAMIC_LINKING_LOADER)
		check_library_exists (dl dlopen "" HAVE_LIBDL)
	endif (NOT HAVE_BUILTIN_DYNAMIC_LINKING_LOADER)
	cmake_push_check_state() # save state of CMAKE_REQUIRED_*
	if (HAVE_LIBDL)
		set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} "-ldl")
	endif (HAVE_LIBDL)
	check_symbol_exists (dladdr dlfcn.h HAVE_DLADDR)
	cmake_pop_check_state() # restore state of CMAKE_REQUIRED_*

	check_function_exists (memalign       HAVE_MEMALIGN)
	check_function_exists (posix_memalign HAVE_POSIX_MEMALIGN)

	if (HAVE_UNISTD_H_)
		# Check if sysconf(_SC_NPROCESSORS_ONLN) can be used for CPU count
		check_c_source_compiles (
			"
			#include <unistd.h>
			int main() {
			  sysconf(_SC_NPROCESSORS_ONLN);
			}
			"
			HAVE_SC_NPROCESSORS_ONLN)
		# Check if sysconf(_SC_NPROC_ONLN) can be used for CPU count
		check_c_source_compiles (
			"
			#include <unistd.h>
			int main() {
			  sysconf(_SC_NPROC_ONLN);
			}
			"
			HAVE_SC_NPROC_ONLN)
	endif (HAVE_UNISTD_H_)

	# Check if sysctl can be used for CPU count
	check_c_source_compiles (
		"
		#include <stddef.h>
		#include <sys/sysctl.h>
		int main () {
		  int count;
		  size_t size = sizeof(count);
		  int mib[] = { CTL_HW, HW_NCPU };
		  sysctl(mib, 2, &count, &size, NULL, 0);
		}
		"
		HAVE_SYSCTL_HW_NCPU)
endif (UNIX)

#
# Check c types
#

check_include_file (ctype.h             HAVE_CTYPE_H_)
check_include_file (inttypes.h          HAVE_INTTYPES_H_)
#check_include_file (stddef.h            HAVE_STDDEF_H_)
#check_include_file (stdint.h            HAVE_STDINT_H_)
#check_include_file (sys/types.h         HAVE_SYS_TYPES_H_)

# set HAVE_SYS_TYPES_H, HAVE_STDINT_H, and HAVE_STDDEF_H
# and check in <sys/types.h>, <stdint.h>, and <stddef.h>:
check_type_size (_Bool                  SIZEOF__BOOL)
check_type_size (bool                   SIZEOF_BOOL)
check_type_size (int                    SIZEOF_INT)
set (CMAKE_EXTRA_INCLUDE_FILES sys/ucontext.h)
check_type_size (greg_t                 SIZEOF_GREG_T)
set (CMAKE_EXTRA_INCLUDE_FILES)
check_type_size (long                   SIZEOF_LONG)
check_type_size ("long long"            SIZEOF_LONG_LONG)
check_type_size ("long double"          SIZEOF_LONG_DOUBLE)
check_type_size (mode_t                 SIZEOF_MODE_T)
check_type_size (off_t                  SIZEOF_OFF_T)
check_type_size (size_t                 SIZEOF_SIZE_T)
check_type_size (wchar_t                SIZEOF_WCHAR_T)
check_type_size ("void*"                SIZEOF_VOID_P)

# add suffix to prevent name clash with GDAL
set (HAVE_STDDEF_H_ "${HAVE_STDDEF_H}"
	CACHE INTERNAL "Have include stddef.h")
set (HAVE_STDINT_H_ "${HAVE_STDINT_H}"
	CACHE INTERNAL "Have include stdinf.h")
set (HAVE_SYS_TYPES_H_ "${HAVE_SYS_TYPES_H}"
	CACHE INTERNAL "Have include sys/types.h")

test_big_endian (WORDS_BIGENDIAN)

# Byte swapping functions
check_c_source_runs (
	"
	int main(void) {
		return !__builtin_bswap16(0xabcd) == 0xcdab;
	}
	"
	HAVE___BUILTIN_BSWAP16)
check_c_source_runs (
	"
	int main(void) {
		return !__builtin_bswap32(0xdeadbeef) == 0xefbeadde;
	}
	"
	HAVE___BUILTIN_BSWAP32)
check_c_source_runs (
	"
	int main(void) {
		return !__builtin_bswap64(0x1234567890abcdef) == 0xefcdab9078563412;
	}
	"
	HAVE___BUILTIN_BSWAP64)
if (WIN32)
	check_function_exists (_byteswap_ushort HAVE__BYTESWAP_USHORT) # for uint16_t
	check_function_exists (_byteswap_ulong  HAVE__BYTESWAP_ULONG)  # for uint32_t
	check_function_exists (_byteswap_uint64 HAVE__BYTESWAP_UINT64) # for uint64_t
endif (WIN32)

#
# Check math related stuff
#

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

# Check if -lm is needed
check_function_exists (cos HAVE_M_FUNCTIONS)
if (NOT HAVE_M_FUNCTIONS)
	check_library_exists (m cos "" HAVE_M_LIBRARY)
endif (NOT HAVE_M_FUNCTIONS)

# If necessary compile with -lm
if (HAVE_M_LIBRARY)
	set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} "-lm")
endif (HAVE_M_LIBRARY)

# check symbols (double)
check_symbol_exists (acosh       "${_math_h}" HAVE_ACOSH)
check_symbol_exists (asinh       "${_math_h}" HAVE_ASINH)
check_symbol_exists (atanh       "${_math_h}" HAVE_ATANH)
check_symbol_exists (copysign    "${_math_h}" HAVE_COPYSIGN)
check_symbol_exists (erf         "${_math_h}" HAVE_ERF)
check_symbol_exists (erfc        "${_math_h}" HAVE_ERFC)
check_symbol_exists (hypot       "${_math_h}" HAVE_HYPOT)
check_symbol_exists (isfinite    "${_math_h}" HAVE_ISFINITE)
check_symbol_exists (isinf       "${_math_h}" HAVE_ISINF)
check_symbol_exists (isnan       "${_math_h}" HAVE_ISNAN)
check_symbol_exists (isnand      "${_math_h}" HAVE_ISNAND)
check_symbol_exists (isnanf      "${_math_h}" HAVE_ISNANF)
check_symbol_exists (isnormal    "${_math_h}" HAVE_ISNORMAL)
check_symbol_exists (j0          "${_math_h}" HAVE_J0)
check_symbol_exists (j1          "${_math_h}" HAVE_J1)
check_symbol_exists (jn          "${_math_h}" HAVE_JN)
check_symbol_exists (lrint       "${_math_h}" HAVE_LRINT)
check_symbol_exists (llrint      "${_math_h}" HAVE_LLRINT)
check_symbol_exists (log1p       "${_math_h}" HAVE_LOG1P)
check_symbol_exists (log2        "${_math_h}" HAVE_LOG2)
check_symbol_exists (rint        "${_math_h}" HAVE_RINT)
check_symbol_exists (sincos      "${_math_h}" HAVE_SINCOS)
check_symbol_exists (y0          "${_math_h}" HAVE_Y0)
check_symbol_exists (y1          "${_math_h}" HAVE_Y1)
check_symbol_exists (yn          "${_math_h}" HAVE_YN)
# check symbols (float)
check_symbol_exists (acosf       "${_math_h}" HAVE_ACOSF)
check_symbol_exists (acoshf      "${_math_h}" HAVE_ACOSHF)
check_symbol_exists (asinf       "${_math_h}" HAVE_ASINF)
check_symbol_exists (asinhf      "${_math_h}" HAVE_ASINHF)
check_symbol_exists (atanf       "${_math_h}" HAVE_ATANF)
check_symbol_exists (atanhf      "${_math_h}" HAVE_ATANHF)
check_symbol_exists (atan2f      "${_math_h}" HAVE_ATAN2F)
check_symbol_exists (erff        "${_math_h}" HAVE_ERFF)
check_symbol_exists (ceilf       "${_math_h}" HAVE_CEILF)
check_symbol_exists (cosf        "${_math_h}" HAVE_COSF)
check_symbol_exists (coshf       "${_math_h}" HAVE_COSHF)
check_symbol_exists (erfcf       "${_math_h}" HAVE_ERFCF)
check_symbol_exists (expf        "${_math_h}" HAVE_EXPF)
check_symbol_exists (fabsf       "${_math_h}" HAVE_FABSF)
check_symbol_exists (floorf      "${_math_h}" HAVE_FLOORF)
check_symbol_exists (fmodf       "${_math_h}" HAVE_FMODF)
check_symbol_exists (hypotf      "${_math_h}" HAVE_HYPOTF)
check_symbol_exists (logf        "${_math_h}" HAVE_LOGF)
check_symbol_exists (log2f       "${_math_h}" HAVE_LOG2F)
check_symbol_exists (log10f      "${_math_h}" HAVE_LOG10F)
check_symbol_exists (log1pf      "${_math_h}" HAVE_LOG1PF)
check_symbol_exists (lrintf      "${_math_h}" HAVE_LRINTF)
check_symbol_exists (llrintf     "${_math_h}" HAVE_LLRINTF)
check_symbol_exists (powf        "${_math_h}" HAVE_POWF)
check_symbol_exists (rintf       "${_math_h}" HAVE_RINTF)
check_symbol_exists (sinf        "${_math_h}" HAVE_SINF)
check_symbol_exists (sinhf       "${_math_h}" HAVE_SINHF)
check_symbol_exists (sqrtf       "${_math_h}" HAVE_SQRTF)
check_symbol_exists (tanf        "${_math_h}" HAVE_TANF)
check_symbol_exists (tanhf       "${_math_h}" HAVE_TANHF)

if (WIN32)
	check_symbol_exists (_copysign "${_math_h}" HAVE__COPYSIGN)
	check_symbol_exists (_finite   "${_math_h}" HAVE__FINITE)
	check_symbol_exists (_fpclass  "${_math_h}" HAVE__FPCLASS)
	check_symbol_exists (_isnan    "${_math_h}" HAVE__ISNAN)
endif (WIN32)

# restore state of CMAKE_REQUIRED_*
cmake_pop_check_state()

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
#check_include_file (mach-o/dyld.h HAVE_MACH_O_DYLD_H  )
#check_include_file (mach/mach_time.h  HAVE_MACH_MACH_TIME_H)
#check_include_file (machine/endian.h    HAVE_MACHINE_ENDIAN_H_)
#check_include_file (argz.h HAVE_ARGZ_H)
#check_include_file (io.h HAVE_IO_H)
#check_include_file (limits.h HAVE_LIMITS_H)
#check_include_file (locale.h HAVE_LOCALE_H)
#check_include_file (memory.h HAVE_MEMORY_H)
#check_include_file (ndir.h HAVE_NDIR_H)
#check_include_file (pthread.h HAVE_PTHREAD_H)
#check_include_file (stdlib.h HAVE_STDLIB_H)
#check_include_file (string.h HAVE_STRING_H)
#check_include_file (strings.h HAVE_STRINGS_H)
#check_include_file (sys/dl.h HAVE_SYS_DL_H )
#check_include_file (sys/socket.h HAVE_SYS_SOCKET_H)
#check_include_file (sys/stat.h HAVE_SYS_STAT_H )
#check_include_file (sys/stat.h HAVE_SYS_STAT_H)
#check_include_file (sys/time.h HAVE_SYS_TIME_H )
#check_include_file (sys/time.h HAVE_SYS_TIME_H)
#check_include_file (sys/types.h HAVE_SYS_TYPES_H )
#check_include_file (sys/types.h HAVE_SYS_TYPES_H)
#check_include_file (sys/utime.h HAVE_SYS_UTIME_H)
#check_include_file (utime.h HAVE_UTIME_H)
#check_library_exists (dl dl "/lib;/usr/lib;/usr/local/lib;/usr/pkg/lib" HAVE_LIBDL)
#check_symbol_exists (LC_MESSAGES "locale.h" HAVE_LC_MESSAGES)
#check_symbol_exists (asprintf "stdio.h" HAVE_ASPRINTF)
#check_symbol_exists (intmax_t "inttypes.h" HAVE_INTTYPES_H_WITH_UINTMAX)
#check_symbol_exists (pid_t "sys/types.h" HAVE_PID_T)
#check_symbol_exists (printf "stdio.h" HAVE_POSIX_PRINTF)
#check_symbol_exists (uintmax_t "stdint.h" HAVE_STDINT_H_WITH_UINTMAX)
#check_symbol_exists (wprintf "stdio.h" HAVE_WPRINTF)
#check_type_size ("long double"         SIZEOF_LONG_DOUBLE)
#check_type_size ("long long" SIZEOF_LONG_LONG)
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
